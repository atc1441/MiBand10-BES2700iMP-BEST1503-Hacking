/***************************************************************************
 *
 * Copyright 2015-2025 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/

/*--------------------------------------------------------------------
                           GENERAL INCLUDES
--------------------------------------------------------------------*/
#include "plat_types.h"
#include <stdarg.h>
#include "co_math.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "hal_timer.h"
#include "bt_drv_interface.h"
#include "app_bt_sync.h"
#ifdef IBRT
#include "app_tws_ibrt_cmd_handler.h"
#include "app_ibrt_internal.h"
#include "earbud_ux_api.h"
#include "app_tws_ctrl_thread.h"
#endif
#ifdef USE_BASIC_THREADS
#include "app_thread.h"
#endif

#if defined(__BT_SYNC__) && defined(IBRT)
/*--------------------------------------------------------------------
                               MACROS
--------------------------------------------------------------------*/
// #define BT_SYNC_DEBUG_LEVEL
#define BT_SYNC_INFO_LEVEL
#ifdef BT_SYNC_DEBUG_LEVEL
#define BT_SYNC_DBG                     TRACE
#else
#define BT_SYNC_DBG(...)                do{} while(0)
#endif

#ifdef BT_SYNC_INFO_LEVEL
#define BT_SYNC_INFO                    TRACE
#else
#define BT_SYNC_INFO(...)               do{} while(0)
#endif
#define BT_SYNC_WARN                    TRACE
#define BT_SYNC_ERROR                   TRACE

/*--------------------------------------------------------------------
                         INTERNAL VARIABLES
--------------------------------------------------------------------*/
static bool bt_sync_initilized = false;
static BT_SYNC_ENV_T bt_sync_env;

#ifndef USE_BASIC_THREADS
static osThreadId bt_sync_thread_tid = NULL;
static void app_bt_sync_thread(void const *argument);
osThreadDef(app_bt_sync_thread, osPriorityRealtime, 1, APP_BT_SYNC_THREAD_STACK_SIZE, "BT_SYNC");
osMailQDef(bt_sync_mailbox, 5, BT_SYNC_MSG_BLOCK_T);
static osMailQId bt_sync_mailbox = NULL;
#endif // endif USE_BASIC_THREADS

static APP_BT_SYNC_INFO_REPORT_T bt_sync_share_info_report_func = NULL;

static void app_bt_sync_supervision_timeout_handler(void const *param);
osTimerDef(BT_SYNC_SUPERVISION_TIMER, app_bt_sync_supervision_timeout_handler);
static osTimerId bt_sync_supervision_timer_id = NULL;
osMutexDef(bt_sync_mutex);
static osMutexId bt_sync_mutex_id = NULL;

/*--------------------------------------------------------------------
                             INTERNAL PROCEDURES
--------------------------------------------------------------------*/
static BT_SYNC_JOB_LIST_T *app_bt_sync_seek_job_handler(uint32_t opCode)
{
    uint32_t total_job_cnt = ((uint32_t)__app_bt_sync_command_handler_table_end-
        (uint32_t)__app_bt_sync_command_handler_table_start) /
        sizeof(BT_SYNC_JOB_LIST_T);

    for (uint32_t i = 0; i < total_job_cnt; i++)
    {
        BT_SYNC_JOB_LIST_T *p_job = APP_BT_SYNC_COMMAND_PTR_FROM_ENTRY_INDEX(i);
        if (p_job->opCode == opCode)
        {
            return p_job;
        }
    }

    return NULL;
}

static BT_SYNC_INSTANCE_T *app_bt_sync_seek_work_instance(uint32_t opCode)
{
    osMutexWait(bt_sync_mutex_id, osWaitForever);

    BT_SYNC_INSTANCE_T *p_work = bt_sync_env.workInstance;

    for (uint32_t i = 0; i < APP_BT_SYNC_CHANNEL_MAX; i++) {
        if ((p_work->syncType != BT_SYNC_TYPE_NONE) && (opCode == p_work->triInfo.opCode)) {
            osMutexRelease(bt_sync_mutex_id);
            return p_work;
        }
        p_work++;
    }
    osMutexRelease(bt_sync_mutex_id);

    return NULL;
}

// supervision list
#ifdef BT_SYNC_DEBUG_LEVEL
static void app_bt_sync_dump_supervison(void)
{
    BT_SYNC_INSTANCE_T *p_work= bt_sync_env.workInstance;

    for (uint32_t i = 0; i < APP_BT_SYNC_CHANNEL_MAX; i++) {
        BT_SYNC_DBG(1, "(d%d)bt_sync_dump, opCode:0x%x, chl:%d, msRemain:%u type:%d",
            i, p_work->triInfo.opCode, p_work->triInfo.triChl, p_work->msTillTimeout, p_work->syncType);
        p_work++;
    }
}
#endif

static void app_bt_sync_refresh_supervison_env(void)
{
    uint32_t nearest_idx = 0;
    uint32_t nearest_time = APP_BT_SYNC_SUPERVISON_TIMEOUT;
    uint32_t totalCnt = CO_BIT_CNT(bt_sync_env.supervisorMask);
    // do nothing if no supervisor was added
    if (totalCnt > 0)
    {
        uint32_t currentTicks = GET_CURRENT_TICKS();
        uint32_t passedTicks = 0;

        if (currentTicks >= bt_sync_env.lastSysTicks)
        {
            passedTicks = (currentTicks - bt_sync_env.lastSysTicks);
        }
        else
        {
            passedTicks = (hal_sys_timer_get_max() - bt_sync_env.lastSysTicks + 1) + currentTicks;
        }

        uint32_t passedMs = TICKS_TO_MS(passedTicks);
        if (passedMs > APP_BT_SYNC_SUPERVISON_TIMEOUT) {
            BT_SYNC_WARN(1, "bt_sync_refresh_supervisor, passedMs:%d, cnt:%d", passedMs, totalCnt);
            passedMs = 0;
        }

        BT_SYNC_INSTANCE_T *p_work = bt_sync_env.workInstance;
        for (uint32_t i = 0; i < APP_BT_SYNC_CHANNEL_MAX; i++)
        {
            if (p_work->syncType == BT_SYNC_TYPE_COMMON) {
                // TODO: there should be a margin
                ASSERT(p_work->msTillTimeout >= passedMs,
                    "(d%d)Supervisor timer missed,%d ms passed but the timeout is %d",
                    i, passedMs, p_work->msTillTimeout);

                BT_SYNC_INFO(2, "(d%d)bt_sync_refresh_supervisor:%u-%u, idx:%d, cnt:%d", i,
                    p_work->msTillTimeout, passedMs, nearest_idx, totalCnt);

                p_work->msTillTimeout -= passedMs;
                if (nearest_time > p_work->msTillTimeout) {
                    nearest_idx = i;
                    nearest_time = p_work->msTillTimeout;
                }
            }

            p_work++;
        }

    }

    osTimerStart(bt_sync_supervision_timer_id, nearest_time);
    bt_sync_env.nearest_idx = nearest_idx;
    bt_sync_env.lastSysTicks = GET_CURRENT_TICKS();
}

static void app_bt_sync_remove_out_supervision(uint8_t index)
{
    ASSERT(bt_sync_env.supervisorMask > 0, "bt_sync_remove_supervision, list empty");
    BT_SYNC_INFO(1, "bt_sync_remove_supervision, bf:%d", bt_sync_env.supervisorMask);

    osMutexWait(bt_sync_mutex_id, osWaitForever);
    bt_sync_env.supervisorMask &= ~(1 << index);

    if (bt_sync_env.supervisorMask > 0)
    {
        // refresh supervisor environment firstly
        app_bt_sync_refresh_supervison_env();
    }
    else
    {
        // no supervisor
        bt_sync_env.lastSysTicks = 0;
        osTimerStop(bt_sync_supervision_timer_id);
    }

#ifdef BT_SYNC_DEBUG_LEVEL
    app_bt_sync_dump_supervison();
#endif
    osMutexRelease(bt_sync_mutex_id);
}

static void app_bt_sync_add_into_supervision_list(uint32_t opCode, uint8_t index)
{
    ASSERT(index < APP_BT_SYNC_CHANNEL_MAX, "(d%d)%s param wrong:%d", index, __func__, opCode);

    osMutexWait(bt_sync_mutex_id, osWaitForever);

    bt_sync_env.workInstance[index].msTillTimeout = APP_BT_SYNC_SUPERVISON_TIMEOUT;
    bt_sync_env.supervisorMask |= (1 << index);

    BT_SYNC_INFO(1, "bt_sync_add_into_supervision_list, bf:%d", bt_sync_env.supervisorMask);

    // refresh supervisor environment firstly
    app_bt_sync_refresh_supervison_env();

#ifdef BT_SYNC_DEBUG_LEVEL
    app_bt_sync_dump_supervison();
#endif
    osMutexRelease(bt_sync_mutex_id);}

#ifdef USE_BASIC_THREADS
static int app_bt_sync_handle(APP_MESSAGE_BODY *msg_body)
{
    uint32_t index = msg_body->message_Param1;
    bool triResult = !!msg_body->message_Param0;
    uint32_t opCode = 0xFFFF;
    BT_SYNC_JOB_LIST_T *p_job_handler = NULL;
    BT_SYNC_INSTANCE_T *p_work = NULL;
    bool bShareStatus = false;

    p_work = &(bt_sync_env.workInstance[index]);
    opCode = p_work->triInfo.opCode;
    p_job_handler = app_bt_sync_seek_job_handler(opCode);

    BT_SYNC_INFO(4, "(d%d)bt_sync_handle, job:%p-%p Code:0x%x, result:%d",
        index, p_work, p_job_handler, opCode, triResult);

    if (p_work) {
        if ((p_work->syncType == BT_SYNC_TYPE_NONE) || p_work->isAbandoned) {
            // Process already
            // IRQ happen -> BT SYNC thread blocked by highest priority thread
            // -> Time out happen during block
           return 0;
        }

        app_bt_sync_release_trigger_channel(p_work->triInfo.triChl);
        app_bt_sync_remove_out_supervision(index);
        bShareStatus = !!p_work->twsShareDone;
        if (p_job_handler && p_job_handler->statusNotify)
        {
            p_job_handler->statusNotify(opCode, triResult, bShareStatus);
        }
    }

    if ((triResult) && (p_job_handler) &&
        (p_job_handler->syncCallback))
    {
        p_job_handler->syncCallback();
    }

    return 0;
}

static void app_bt_sync_supervision_timeout_handler(void const *param)
{
    uint32_t opCode = bt_sync_env.workInstance[bt_sync_env.nearest_idx].triInfo.opCode;
    uint32_t trigger_chl = bt_sync_env.nearest_idx + APP_BT_SYNC_CHANNEL_OFFSET;

    BT_SYNC_WARN(2, "(d%d)bt_sync_timeout, opCode:%d", bt_sync_env.nearest_idx, opCode);

    hal_cmu_bt_trigger_set_handler((enum HAL_CMU_BT_TRIGGER_SRC_T)trigger_chl, NULL);
    APP_MESSAGE_BLOCK msg;
    msg.mod_id = APP_MODULE_BTSYNC;
    msg.mod_level = APP_MOD_LEVEL_0;
    msg.msg_body.message_Param0 = APP_BT_SYNC_RESULT_FAILED;
    msg.msg_body.message_Param1 = bt_sync_env.nearest_idx;
    app_mailbox_put(&msg);
}

static void app_bt_sync_irq_handler(enum HAL_CMU_BT_TRIGGER_SRC_T src)
{
    uint32_t index = src - APP_BT_SYNC_CHANNEL_OFFSET;
    // sanity check
    if (index > APP_BT_SYNC_CHANNEL_MAX) {
        BT_SYNC_ERROR(1, "(d%d)bt_sync_irq_handler, error", index);
        return;
    }
    BT_SYNC_INSTANCE_T *p_work = &(bt_sync_env.workInstance[index]);

    BT_SYNC_INFO(3, "(d%d)bt_sync_irq_handler, src:%d opCode:0x%x",
        index, src, p_work->triInfo.opCode);

    btdrv_syn_clr_trigger((uint8_t)src);
    hal_cmu_bt_trigger_disable(src);

    APP_MESSAGE_BLOCK msg;
    msg.mod_id = APP_MODULE_BTSYNC;
    msg.mod_level = APP_MOD_LEVEL_0;
    msg.msg_body.message_Param0 = APP_BT_SYNC_RESULT_PASS;
    msg.msg_body.message_Param1 = index;
    app_mailbox_put(&msg);
}

static int app_bt_sync_module_init(void)
{
    int32_t lock = int_lock();
    if (bt_sync_initilized) {
        int_unlock(lock);
        return 0;
    }
    bt_sync_initilized = true;
    int_unlock(lock);

    app_set_threadhandle(APP_MODULE_BTSYNC, app_bt_sync_handle);

    if (!bt_sync_supervision_timer_id)
    {
        bt_sync_supervision_timer_id = osTimerCreate(osTimer(BT_SYNC_SUPERVISION_TIMER), osTimerOnce, NULL);
    }

    if (!bt_sync_mutex_id)
    {
        bt_sync_mutex_id = osMutexCreate(osMutex(bt_sync_mutex));
    }

    if ((!bt_sync_supervision_timer_id) || (!bt_sync_mutex_id))
    {
        return -1;
    }

    memset((uint8_t *)&bt_sync_env, 0x00, sizeof(bt_sync_env));

    return 0;
}

#else // USE_BASIC_THREADS
static int app_bt_sync_mailbox_free(BT_SYNC_MSG_BLOCK_T *msg_p)
{
    osStatus status;

    status = osMailFree(bt_sync_mailbox, msg_p);
    if (osOK != status) {
        BT_SYNC_ERROR(1, "%s failed to free", __func__);
    }
    return (int)status;
}

static int app_bt_sync_mailbox_get(BT_SYNC_MSG_BLOCK_T **msg_p)
{
    osEvent evt;
    evt = osMailGet(bt_sync_mailbox, osWaitForever);
    if (evt.status == osEventMail) {
        *msg_p = (BT_SYNC_MSG_BLOCK_T *)evt.value.p;
        return 0;
    }
    return -1;
}

static int app_bt_sync_mailbox_put(BT_SYNC_MSG_BLOCK_T *msg_src)
{
    osStatus status;
    BT_SYNC_MSG_BLOCK_T *msg_p = NULL;

    msg_p = (BT_SYNC_MSG_BLOCK_T *)osMailAlloc(bt_sync_mailbox, 0);

    if (!msg_p) {
        BT_SYNC_ERROR(1, "%s failed to alloc", __func__);
        return -1;
    }

    *msg_p = *msg_src;
    status = osMailPut(bt_sync_mailbox, msg_p);
    if (osOK != status) {
        BT_SYNC_ERROR(1, "%s failed(%d) to put", __func__, status);
    }

    return (int)status;
}

static void app_bt_sync_thread(void const *argument)
{
    while(1)
    {
        BT_SYNC_MSG_BLOCK_T *msg_p = NULL;
        uint32_t index = APP_BT_SYNC_INVALID_CHANNEL;
        BT_SYNC_JOB_LIST_T *p_job_handler = NULL;
        BT_SYNC_INSTANCE_T *p_work = NULL;
        bool bShareStatus = false;
        bool triResult = false;
        uint32_t opCode = 0xFFFF;

        if (!app_bt_sync_mailbox_get(&msg_p)) {
            index = msg_p->workIdx;
            triResult = !!msg_p->triResult;

            app_bt_sync_mailbox_free(msg_p);

            p_work = &(bt_sync_env.workInstance[index]);
            opCode = p_work->triInfo.opCode;
            p_job_handler = app_bt_sync_seek_job_handler(opCode);

            BT_SYNC_INFO(4, "(d%d)bt_sync_thread, job:%p-%p code:0x%x",
                index, p_work, p_job_handler, opCode);

            if (p_work) {
                if ((p_work->syncType == BT_SYNC_TYPE_NONE) || p_work->isAbandoned) {
                    // Process already or has been abandoned
                    // IRQ happen -> BT SYNC thread blocked by highest priority thread
                    // -> Time out happen during block
                    continue;
                }

                app_bt_sync_release_trigger_channel(p_work->triInfo.triChl);
                app_bt_sync_remove_out_supervision(index);
                bShareStatus = !!p_work->twsShareDone;
                if (p_job_handler && p_job_handler->statusNotify)
                {
                    p_job_handler->statusNotify(opCode, triResult, bShareStatus);
                }
            }

            if ((triResult) && (p_job_handler) &&
                (p_job_handler->syncCallback))
            {
                p_job_handler->syncCallback();
            }
        }
    }
}

static void app_bt_sync_supervision_timeout_handler(void const *param)
{
    uint32_t opCode = bt_sync_env.workInstance[bt_sync_env.nearest_idx].triInfo.opCode;
    uint32_t trigger_chl = bt_sync_env.nearest_idx + APP_BT_SYNC_CHANNEL_OFFSET;

    BT_SYNC_WARN(2, "(d%d)bt_sync_timeout, opCode:%d", bt_sync_env.nearest_idx, opCode);

    hal_cmu_bt_trigger_set_handler((enum HAL_CMU_BT_TRIGGER_SRC_T)trigger_chl, NULL);
    BT_SYNC_MSG_BLOCK_T msg;
    msg.triResult = APP_BT_SYNC_RESULT_FAILED;
    msg.workIdx = bt_sync_env.nearest_idx;
    app_bt_sync_mailbox_put(&msg);
}

static void app_bt_sync_irq_handler(enum HAL_CMU_BT_TRIGGER_SRC_T src)
{
    uint32_t index = src - APP_BT_SYNC_CHANNEL_OFFSET;
    // sanity check
    if (index >= APP_BT_SYNC_CHANNEL_MAX) {
        BT_SYNC_ERROR(1, "(d%d)bt_sync_irq_handler, error", index);
        return;
    }

    BT_SYNC_INSTANCE_T *p_work = &(bt_sync_env.workInstance[index]);

    BT_SYNC_INFO(3, "(d%d)bt_sync_irq_handler, opCode:0x%x",
        index, p_work->triInfo.opCode);

    btdrv_syn_clr_trigger((uint8_t)src);
    hal_cmu_bt_trigger_disable(src);

    BT_SYNC_MSG_BLOCK_T msg;
    msg.triResult = APP_BT_SYNC_RESULT_PASS;
    msg.workIdx = index;
    app_bt_sync_mailbox_put(&msg);
}

static int app_bt_sync_module_init(void)
{
    int32_t lock = int_lock();
    if (bt_sync_initilized) {
        int_unlock(lock);
        return 0;
    }
    bt_sync_initilized = true;
    int_unlock(lock);

    if (!bt_sync_mailbox) {
        bt_sync_mailbox = osMailCreate(osMailQ(bt_sync_mailbox), NULL);
    }

    if (!bt_sync_thread_tid) {
        bt_sync_thread_tid = osThreadCreate(osThread(app_bt_sync_thread), NULL);
    }

    if (!bt_sync_supervision_timer_id) {
        bt_sync_supervision_timer_id = osTimerCreate(osTimer(BT_SYNC_SUPERVISION_TIMER), osTimerOnce, NULL);
    }

    if (!bt_sync_mutex_id) {
        bt_sync_mutex_id = osMutexCreate(osMutex(bt_sync_mutex));
    }

    if (!bt_sync_mutex_id || !bt_sync_mailbox ||
        !bt_sync_thread_tid || !bt_sync_supervision_timer_id) {
        BT_SYNC_ERROR(4, "bt_sync_init, %p-%p-%p-%p", bt_sync_mutex_id,
            bt_sync_mailbox, bt_sync_thread_tid, bt_sync_supervision_timer_id);
        return -1;
    }

    memset((uint8_t *)&bt_sync_env, 0x00, sizeof(bt_sync_env));
    return 0;
}
#endif // USE_BASIC_THREADS

/*--------------------------------------------------------------------
                         EXTERNAL PROCEDURES
--------------------------------------------------------------------*/
void app_bt_sync_get_master_time_from_slave_time(uint32_t SlaveBtTicksUs,
    uint32_t* p_master_clk_cnt, uint16_t* p_master_bit_cnt)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    int32_t offset;
    uint8_t link_id = btdrv_conhdl_to_linkid(p_ibrt_ctrl->tws_conhandle);
    uint32_t slaveClockCnt;
    uint16_t finecnt;
    btdrv_reg_op_bts_to_bt_time(SlaveBtTicksUs, &slaveClockCnt, &finecnt);

    if (btdrv_is_link_index_valid(link_id))
    {
        offset = bt_drv_reg_op_get_clkoffset(link_id);
    }
    else
    {
        offset = 0;
    }

    *p_master_clk_cnt = (slaveClockCnt + offset) & 0x0fffffff;
    *p_master_bit_cnt = finecnt;
}

uint32_t app_bt_sync_get_slave_time_from_master_time(uint32_t master_clk_cnt, uint16_t master_bit_cnt)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    int32_t offset;
    uint8_t link_id = btdrv_conhdl_to_linkid(p_ibrt_ctrl->tws_conhandle);

    if (btdrv_is_link_index_valid(link_id))
    {
        offset = bt_drv_reg_op_get_clkoffset(link_id);
    }
    else
    {
        offset = 0;
    }
    uint32_t slaveClockCnt = (master_clk_cnt - offset) & 0x0fffffff;

    uint32_t SlaveBtTicksUs = btdrv_reg_op_bt_time_to_bts(slaveClockCnt, master_bit_cnt);
    return SlaveBtTicksUs;
}

uint8_t app_bt_sync_get_available_trigger_channel(uint32_t opcode = APP_BT_SYNC_OP_MAX,
    uint8_t policy = APP_BT_SYNC_POLICY_DEFAULT)
{
    // Need to initilize first.
    // Otherwise the variable locTriggerInfo will be cleared by sync module
    int ret = app_bt_sync_module_init();
    if (ret) {
        return APP_BT_SYNC_INVALID_CHANNEL;
    }

    osMutexWait(bt_sync_mutex_id, osWaitForever);
    uint32_t index = 0;
    uint32_t chl = APP_BT_SYNC_INVALID_CHANNEL;
    BT_SYNC_INSTANCE_T *p_work = &bt_sync_env.workInstance[0];

    for (; index < APP_BT_SYNC_CHANNEL_MAX; index++)
    {
        // The same opcode is already in the queue.
        if ((p_work->syncType != BT_SYNC_TYPE_NONE) && (opcode == p_work->triInfo.opCode))
        {
            BT_SYNC_INFO(1,"bt_sync_get_ava_chl,find same job, chl:%d, policy:%d",
                p_work->triInfo.triChl, policy);
            if (APP_BT_SYNC_POLICY_UPDATE == policy) {
                btdrv_syn_clr_trigger(p_work->triInfo.triChl);
                hal_cmu_bt_trigger_disable((HAL_CMU_BT_TRIGGER_SRC_T)p_work->triInfo.triChl);
                break;
            } else if (APP_BT_SYNC_POLICY_MULTIPLEX == policy) {
                goto exit;
            }
        }

        if (p_work->syncType == BT_SYNC_TYPE_NONE)
        {
            break;
        }
        p_work++;
    }

    if (index < APP_BT_SYNC_CHANNEL_MAX)
    {
        chl = index + APP_BT_SYNC_CHANNEL_OFFSET;
        p_work = &bt_sync_env.workInstance[index];
        p_work->syncType = opcode == APP_BT_SYNC_OP_MAX ? BT_SYNC_TYPE_AUDIO : BT_SYNC_TYPE_COMMON;
        p_work->triInfo.opCode = opcode;
        // channel0 is reserved for A2DP/HFP, channel1 is reserved for prompt
        p_work->triInfo.triChl = chl;
    }

exit:
    osMutexRelease(bt_sync_mutex_id);
    BT_SYNC_INFO(2, "bt_sync_get_ava_chl, chl:%d, opcode:%d", chl, opcode);
    return chl;
}

void app_bt_sync_register_report_info_callback(APP_BT_SYNC_INFO_REPORT_T cb)
{
    bt_sync_share_info_report_func = cb;
}

void app_bt_sync_send_tws_cmd_done(uint8_t *ptrParam, uint16_t paramLen)
{
    if (!ptrParam || (paramLen < sizeof(BT_SYNC_TRIGGER_INFO_T))) {
        BT_SYNC_ERROR(1, "%s, len error:%d", __func__, paramLen);
        return;
    }

    BT_SYNC_TRIGGER_INFO_T triInfo;
    memcpy((uint8_t*)&triInfo, ptrParam, sizeof(BT_SYNC_TRIGGER_INFO_T));

    BT_SYNC_INSTANCE_T *p_work = NULL;

    p_work = app_bt_sync_seek_work_instance(triInfo.opCode);
    if (p_work) {
        p_work->twsShareDone = 1;
    }
}

void app_bt_sync_tws_cmd_handler(uint8_t *p_buff, uint16_t length)
{
    uint32_t len_tri_info = sizeof(BT_SYNC_TRIGGER_INFO_T);
    if (!p_buff || (length < len_tri_info)) {
        BT_SYNC_ERROR(1, "%s, len error:%d", __func__, length);
        return;
    }

    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    uint32_t curr_ticks = 0;
    int ret = 0;
    uint32_t chnl = APP_BT_SYNC_INVALID_CHANNEL;
    uint32_t index = 0;
    uint8_t btRole = BTIF_BCR_MASTER;
    BT_SYNC_SHARE_INFO_T *p_shareInfo = (BT_SYNC_SHARE_INFO_T *)p_buff;

    // TODO: report function add into function table
    if (bt_sync_share_info_report_func && (length > len_tri_info)) {
        bt_sync_share_info_report_func(p_shareInfo->trigger_info.opCode, p_shareInfo->extra_info, length - len_tri_info);
    }

    btRole = btif_me_get_remote_device_role(p_ibrt_ctrl->p_tws_remote_dev);
    BT_SYNC_INFO(4, "bt_sync_tws_cmd_handler, opCode:%d role:%d tick:%u, chl:%d", p_shareInfo->trigger_info.opCode,
        btRole, p_shareInfo->trigger_info.triTick, p_shareInfo->trigger_info.triChl);

    if (BTIF_BCR_SLAVE == btRole)
    {
        do {
            /**
            * Step1: Do sanity check here
            * 1. Module init failed
            * 2. TWS not connected
            * 3. The maximum limit has been reached.
            * 4. Check trigger ticks
            *
            */
            ret = app_bt_sync_module_init();
            if (ret) {
                BT_SYNC_WARN(2, "bt_sync_tws_cmd_handler, init failed:%d", ret);
                ret = -1;
                break;
            }

            if (BT_INVALID_CONN_HANDLE == p_ibrt_ctrl->tws_conhandle) {
                ret = -2;
                break;
            }

            curr_ticks = bt_syn_get_curr_ticks(p_ibrt_ctrl->tws_conhandle);
            BT_SYNC_WARN(2, "bt_sync_tws_cmd_handler, curr tick:%u-%u", curr_ticks, p_shareInfo->trigger_info.triTick);
            if (curr_ticks >= p_shareInfo->trigger_info.triTick){
                ret = -3;
                break;
            }

            chnl = app_bt_sync_get_available_trigger_channel(p_shareInfo->trigger_info.opCode, APP_BT_SYNC_POLICY_UPDATE);
            if (chnl < APP_BT_SYNC_CHANNEL_TOTAL) {
                /// get the index of trigger management module(
                /// minus 2 for reserved channel0 for A2DP/HFP, channel 1 for prompt)
                index = chnl - APP_BT_SYNC_CHANNEL_OFFSET;
            } else {
                // do nothing, find ava channel failed or policy is APP_BT_SYNC_POLICY_MULTIPLEX
                ret = -4;
                break;
            }

            bt_syn_set_tg_ticks(p_shareInfo->trigger_info.triTick, p_ibrt_ctrl->tws_conhandle,
                BT_TRIG_SLAVE_ROLE, chnl, false);
            hal_cmu_bt_trigger_set_handler((enum HAL_CMU_BT_TRIGGER_SRC_T)chnl,
                app_bt_sync_irq_handler);
            hal_cmu_bt_trigger_enable((enum HAL_CMU_BT_TRIGGER_SRC_T)chnl);
            app_bt_sync_add_into_supervision_list(p_shareInfo->trigger_info.opCode, index);
        } while(0);
    }
    else if (BTIF_BCR_MASTER == btRole)
    {
        ret = app_bt_sync_enable(p_shareInfo->trigger_info.opCode, 0, NULL, p_shareInfo->trigger_info.policy);
    }
    else
    {
        ret = -5;
    }

    BT_SYNC_INFO(1, "bt_sync_tws_cmd_handler, ret:%d", ret);
    return;
}

void app_bt_sync_abandon_work_instance(uint32_t opCode)
{
    osMutexWait(bt_sync_mutex_id, osWaitForever);
    BT_SYNC_INSTANCE_T *p_work = app_bt_sync_seek_work_instance(opCode);
    if (p_work) {
        p_work->isAbandoned = true;
    }
    osMutexRelease(bt_sync_mutex_id);
}

void app_bt_sync_release_trigger_channel(uint8_t chnl)
{
    BT_SYNC_INSTANCE_T *p_work = NULL;
    uint32_t index = APP_BT_SYNC_INVALID_CHANNEL;

    osMutexWait(bt_sync_mutex_id, osWaitForever);
    index = chnl - APP_BT_SYNC_CHANNEL_OFFSET;
    // sanity check
    if (index >= APP_BT_SYNC_CHANNEL_MAX) {
        osMutexRelease(bt_sync_mutex_id);
        BT_SYNC_ERROR(1, "bt_sync_release_chl, chl(%d) error", chnl);
        return;
    }

    p_work = &bt_sync_env.workInstance[index];
    BT_SYNC_INFO(1, "(d%d)bt_sync_release_chl, chl:%d type:%d, opcode:%d",
        index, chnl, p_work->syncType, p_work->triInfo.opCode);
    memset(p_work, 0x0, sizeof(BT_SYNC_INSTANCE_T));
    osMutexRelease(bt_sync_mutex_id);
}

bool app_bt_sync_enable(uint32_t opCode, uint8_t extra_len,
    uint8_t *p_extra_info, uint8_t policy = APP_BT_SYNC_POLICY_DEFAULT)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    BT_SYNC_SHARE_INFO_T share_info = {0};
    uint32_t tg_tick = 0;
    uint32_t curr_ticks = 0;
    uint8_t index = 0xff;
    uint8_t btRole = BTIF_BCR_MASTER;
    uint32_t delayUs = 0;
    struct BT_DEVICE_T *p_device = NULL;
    ibrt_mobile_info_t *p_mobile_info = NULL;
    bt_bdaddr_t *p_mobile_addr = NULL;
    bool mobileInSniff = false;
    int ret = 0;
    rx_agc_t link_agc_info = {0};
    btif_cmgr_handler_t *cmgrHandler = NULL;
    uint32_t chnl = APP_BT_SYNC_INVALID_CHANNEL;

    /**
    * Step1: Initialize the module
    *
    */
    ret = app_bt_sync_module_init();
    if (ret) {
        goto exit;
    }

    do {
        /**
        * Step2: Do sanity check here
        * 1. TWS not connected or manager handler is null
        * 2. (Support share info to peer bud) The length of share info exceeds the max allowed length
        *    Max size: ARRAY_SIZE(shareBuf) - sizeof(BT_SYNC_TRIGGER_INFO_T)
        * 3. (Support share info to peer bud) Length is not zero, but the pointer is empty.
        * 4. Check to see if there is the same. If yes. then check policy
        *
        */
        if (BT_INVALID_CONN_HANDLE == p_ibrt_ctrl->tws_conhandle) {
            ret = -2;
            break;
        }

        cmgrHandler = btif_lock_free_cmgr_get_acl_handler(p_ibrt_ctrl->p_tws_remote_dev);
        if (!cmgrHandler) {
            ret = -3;
            break;
        }

        if ((extra_len > APP_BT_SYNC_EXTRA_INFO_LEN) ||
            ((extra_len > 0) && !p_extra_info)) {
            ret = -4;
            break;
        }

        /**
        * Step3: Calculate trigger time and get available channel
        *
        */
        chnl = app_bt_sync_get_available_trigger_channel(opCode, policy);
        if (chnl < APP_BT_SYNC_CHANNEL_TOTAL) {
            /// get the index of trigger management module(
            /// minus 2 for reserved channel0 for A2DP/HFP, channel 1 for prompt)
            index = chnl - APP_BT_SYNC_CHANNEL_OFFSET;
        } else {
            // do nothing, find ava channel failed or policy is APP_BT_SYNC_POLICY_MULTIPLEX
            if (APP_BT_SYNC_POLICY_MULTIPLEX == policy) {
                break;
            }
            ret = -5;
            break;
        }

        btRole = btif_me_get_remote_device_role(p_ibrt_ctrl->p_tws_remote_dev);
        if (BTIF_BCR_SLAVE == btRole) {
            goto cal_done;
        }

        for (uint32_t i = 0; i < BT_DEVICE_NUM; ++i)
        {
            p_device = app_bt_get_device(i);
            p_mobile_addr = &p_device->remote;
            p_mobile_info = app_ibrt_conn_get_mobile_info_by_addr(p_mobile_addr);

            if ((NULL != p_mobile_info) && (IBRT_SNIFF_MODE == p_mobile_info->mobile_mode)) {
                mobileInSniff = true;
                app_tws_ibrt_exit_sniff_with_mobile(p_mobile_addr);
                break;
            }
        }

        if (mobileInSniff || (p_ibrt_ctrl->tws_mode == IBRT_SNIFF_MODE)) {
            app_tws_ibrt_exit_sniff_with_tws();
            uint16_t twsSnifferInterval = btif_cmgr_get_cmgrhandler_sniff_interval(cmgrHandler);
            BT_SYNC_INFO(2, "bt_sync_enable, in sniff mode,itv:%d-%d", twsSnifferInterval, mobileInSniff);
            delayUs = twsSnifferInterval * 1000 * 2;
        } else {
            if (p_ibrt_ctrl->acl_interval < IBRT_UI_DEFAULT_POLL_INTERVAL) {
                delayUs = IBRT_UI_DEFAULT_POLL_INTERVAL * 625 * 4;
            } else {
                delayUs = p_ibrt_ctrl->acl_interval * 625 * 4;
            }
        }

        if (bt_drv_reg_op_read_rssi_in_dbm(btif_me_get_remote_device_hci_handle(p_ibrt_ctrl->p_tws_remote_dev),
            &link_agc_info)) {
            BT_SYNC_DBG(1, "bt_sync_enable, TWS-RSSI:%d", link_agc_info.rssi);
            if (link_agc_info.rssi < -90) {
                delayUs *= 4;
            } else if (link_agc_info.rssi < -85) {
                delayUs *= 2;
            }
        }

        curr_ticks = bt_syn_get_curr_ticks(p_ibrt_ctrl->tws_conhandle);
        tg_tick = curr_ticks + US_TO_BTCLKS(delayUs);
        share_info.trigger_info.triTick = tg_tick;

cal_done:
        /**
        * Step4: Share with peer bud if it is BT central
        *
        */
        share_info.trigger_info.opCode = opCode;
        share_info.trigger_info.policy = policy;
        if (p_extra_info)
        {
            memcpy(share_info.extra_info , p_extra_info, extra_len);
        }

        int result = tws_ctrl_send_cmd(APP_TWS_CMD_SET_SYNC_TIME,
                    (uint8_t *)&share_info, sizeof(BT_SYNC_TRIGGER_INFO_T) + extra_len);

        if (!result) {
            if (BTIF_BCR_MASTER == btRole)
            {
                BT_SYNC_INSTANCE_T *p_work = &bt_sync_env.workInstance[index];
                p_work->triInfo.triTick = tg_tick;
                BT_SYNC_INFO(4, "bt_sync_enable, triSrc:%d, curr tick:%u+delayUs:%d > tg_tick:%u, policy:%d",
                    p_work->triInfo.triChl, curr_ticks, delayUs, tg_tick, policy);

                /**
                * Step5: enable BT STAMP
                *
                */
                bt_syn_set_tg_ticks(tg_tick, p_ibrt_ctrl->tws_conhandle,
                    BT_TRIG_MASTER_ROLE, chnl, false);
                hal_cmu_bt_trigger_set_handler((enum HAL_CMU_BT_TRIGGER_SRC_T)chnl,
                    app_bt_sync_irq_handler);
                hal_cmu_bt_trigger_enable((enum HAL_CMU_BT_TRIGGER_SRC_T)chnl);
            }
            app_bt_sync_add_into_supervision_list(opCode, index);
        } else {
            ret = -6;
        }

    } while(0);

exit:
    BT_SYNC_INFO(1, "bt_sync_enable, ret:%d chl:%d", ret, chnl);
    if (ret) {
        app_bt_sync_release_trigger_channel(chnl);
        return false;
    }
    return true;
}
#endif
