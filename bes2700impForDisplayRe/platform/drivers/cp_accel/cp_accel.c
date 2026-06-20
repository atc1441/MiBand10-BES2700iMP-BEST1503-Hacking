/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifdef CP_IN_SAME_EE

#include "cp_accel.h"
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_cmu.h"
#include "hal_location.h"
#include "hal_mcu2cp.h"
#include "hal_sleep.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "mpu.h"
#include "stdarg.h"
#include "system_cp.h"
#include "hal_memsc.h"
#include "string.h"
#include "app_utils.h"
#include "hal_overlay.h"

#ifdef CP_ACCEL_DEBUG
#define CP_ACCEL_TRACE(s, ...)              TRACE(s, ##__VA_ARGS__)
#else
#define CP_ACCEL_TRACE(s, ...)
#endif

#define CP_NO_FLASH_ACCESS

#define CP_CRASH_START_TIMEOUT              MS_TO_TICKS(100)
#define CP_TRACE_FLUSH_TIMEOUT              MS_TO_TICKS(200)
#define CP_CRASH_DUMP_TIMEOUT               MS_TO_TICKS(500)
#define CP_TRACE_BUF_FULL_INTVL             MS_TO_TICKS(50)

#ifdef USE_TRACE_ID
/* invalid trace type for trace id */
#undef CP_CPU_USAGE_INTVL_MS
#undef CP_CPU_USAGE_TRACE_INTVL_MS
#define CP_CPU_USAGE_INTVL_MS               0
#define CP_CPU_USAGE_TRACE_INTVL_MS         0

#else
#ifndef CP_CPU_USAGE_INTVL_MS
#define CP_CPU_USAGE_INTVL_MS               10000
#endif
#ifndef CP_CPU_USAGE_TRACE_INTVL_MS
#define CP_CPU_USAGE_TRACE_INTVL_MS         10000
#endif
#endif


enum CP_SYS_EVENT_T {
    CP_SYS_EVENT_NONE = 0,
    CP_SYS_EVENT_CRASH_START,
    CP_SYS_EVENT_CRASH_END,
    CP_SYS_EVENT_TRACE_FLUSH,
    CP_SYS_EVENT_TRACE_BUF_FULL,
};

static bool ram_inited = false;
static bool cp_accel_inited = false;
static struct cp_task_env_tag cp_task_env;
static CP_BSS_LOC volatile struct cp_env_tag cp_env;

static CP_ACCEL_INIT_DONE_CB cp_accel_init_done_cb = NULL;

static CP_BSS_LOC volatile enum CP_SYS_EVENT_T cp_sys_evt;
static CP_BSS_LOC bool cp_in_crash;
static CP_BSS_LOC volatile uint8_t cp_in_sleep;
static CP_BSS_LOC uint32_t cp_buf_full_time;
static CP_BSS_LOC uint8_t req_event = 0, pending_event = 0;

#if (CP_CPU_USAGE_INTVL_MS > 0)
static CP_BSS_LOC uint32_t stats_interval;
static CP_BSS_LOC uint32_t stats_start_time;
static CP_BSS_LOC uint32_t sleep_time;
#if (CP_CPU_USAGE_TRACE_INTVL_MS > 0)
static CP_BSS_LOC uint32_t stats_trace_interval;
static CP_BSS_LOC uint32_t stats_trace_time;;
#endif
static uint8_t sleep_ratio;
#endif

static CP_TEXT_SRAM_LOC int send_sys_ctrl_cp2mcu(uint32_t event)
{
    return hal_mcu2cp_send_cp(HAL_MCU2CP_ID_1, (unsigned char *)event, 0);
}

static CP_TEXT_SRAM_LOC void cp_trace_crash_notify(enum HAL_TRACE_STATE_T state)
{
    uint32_t time;

    if (state == HAL_TRACE_STATE_CRASH_ASSERT_START || state == HAL_TRACE_STATE_CRASH_FAULT_START) {
        cp_in_crash = true;
        cp_sys_evt = CP_SYS_EVENT_CRASH_START;
        mpu_close();
        send_sys_ctrl_cp2mcu(0);

        time = hal_sys_timer_get();
        while (cp_sys_evt == CP_SYS_EVENT_CRASH_START &&
                hal_sys_timer_get() - time < CP_CRASH_START_TIMEOUT);
    } else {
        cp_sys_evt = CP_SYS_EVENT_CRASH_END;
    }
}

static CP_TEXT_SRAM_LOC void cp_trace_buffer_ctrl(enum HAL_TRACE_BUF_STATE_T buf_state)
{
    uint32_t time;

    if (cp_sys_evt != CP_SYS_EVENT_NONE) {
        return;
    }

    time = hal_sys_timer_get();

    if (buf_state == HAL_TRACE_BUF_STATE_FLUSH) {
        cp_sys_evt = CP_SYS_EVENT_TRACE_FLUSH;
        if (!cp_in_crash) {
            send_sys_ctrl_cp2mcu(0);
        }

        while (cp_sys_evt == CP_SYS_EVENT_TRACE_FLUSH &&
                hal_sys_timer_get() - time < CP_TRACE_FLUSH_TIMEOUT);
    } else if (buf_state == HAL_TRACE_BUF_STATE_FULL || buf_state == HAL_TRACE_BUF_STATE_NEAR_FULL) {
        if (time - cp_buf_full_time >= CP_TRACE_BUF_FULL_INTVL) {
            cp_buf_full_time = time;
            if (!cp_in_crash) {
                cp_sys_evt = CP_SYS_EVENT_TRACE_BUF_FULL;
                send_sys_ctrl_cp2mcu(0);
            }
        }
    }
}

static SRAM_TEXT_LOC unsigned int cp2mcu_sys_arrived(const unsigned char *data, unsigned int len)
{
    uint32_t time;
    uint8_t task_id = 0;

    if (cp_sys_evt == CP_SYS_EVENT_TRACE_FLUSH) {
        TRACE_FLUSH();
        cp_sys_evt = CP_SYS_EVENT_NONE;
    } else if (cp_sys_evt == CP_SYS_EVENT_TRACE_BUF_FULL) {
        TRACE(0, " ");
        cp_sys_evt = CP_SYS_EVENT_NONE;
    } else if (cp_sys_evt == CP_SYS_EVENT_CRASH_START) {
        cp_sys_evt = CP_SYS_EVENT_NONE;

        TRACE(0, " ");
        TRACE(0, "CP Crash starts ...");
        UNLOCK_CP_PROCESS(); //Forced release lock

        // Wait CP crash dump finishes in interrupt context
        time = hal_sys_timer_get();
        while (cp_sys_evt != CP_SYS_EVENT_CRASH_END &&
                hal_sys_timer_get() - time < CP_CRASH_DUMP_TIMEOUT) {
            if (cp_sys_evt == CP_SYS_EVENT_TRACE_FLUSH) {
                TRACE_FLUSH();
                cp_sys_evt = CP_SYS_EVENT_NONE;
            }
        }

        for (task_id = 0; task_id < CP_TASK_MAX; task_id++) {
            if (cp_task_env.p_desc[task_id].mcu_sys_ctrl_hdlr) {
                cp_task_env.p_desc[task_id].mcu_sys_ctrl_hdlr(CP_SYS_EVENT_CRASH_END);
            }
        }

        TRACE(0, "CP Crash ends ...");
        TRACE(0, " ");
#ifdef ASSERT_ON_CP_CRASH
        ASSERT(false, "Assert on CP crash");
#endif
    }

    return len;
}

static CP_TEXT_SRAM_LOC unsigned int mcu2cp_msg_arrived(const unsigned char *data, unsigned int len)
{
    uint8_t task_id = CP_TASK_ID_GET(*data);
    uint8_t event_type = CP_EVENT_GET(*data);

    cp_env.cp_msg[task_id][event_type] = 1;
    cp_env.cp_msg_recv = true;

    if (cp_task_env.p_desc[task_id].cp_evt_hdlr) {
        cp_task_env.p_desc[task_id].cp_evt_hdlr((uint32_t)data);
    }

    return len;
}

static CP_TEXT_SRAM_LOC void mcu2cp_msg_sent(const unsigned char *data, unsigned int len)
{
    // TRACE(1, "mcu2cp_msg_sent,pending count = %d", cp_env.mcu2cp_tx_count);

    if (cp_env.mcu2cp_tx_count > 1) {
        cp_env.mcu2cp_tx_count--;
        pending_event = cp_env.mcu2cp_tx_pending[0];
        hal_mcu2cp_send_mcu(HAL_MCU2CP_ID_0, &pending_event, 1);

        for (uint8_t index = 0;index < cp_env.mcu2cp_tx_count-1;index++) {
            cp_env.mcu2cp_tx_pending[index] = cp_env.mcu2cp_tx_pending[index+1];
        }
    } else {
        cp_env.mcu2cp_tx_count = 0;
    }
}

#if defined(__ARM_ARCH_8M_MAIN__)

#if defined(ROMX_BASE) && (ROMX_BASE != ROM_BASE)
#define CP_CODE_MAP_BASE                    ROMX_BASE
#define CP_ROM_DATA_MAP_BASE                ROM_BASE
#define CP_ROM_DATA_MAP_SIZE                ROM_TOTAL_SIZE
#elif (ROM_BASE)
#define CP_CODE_MAP_BASE                    ROM_BASE
#else
#define CP_CODE_MAP_BASE                    (0x800)
#endif
#define CP_CODE_MAP_SIZE                    (RAMX_BASE + RAM_TOTAL_SIZE - CP_CODE_MAP_BASE)

static CP_DATA_LOC const  mpu_regions_t mpu_table_cp[] = {
    {CP_CODE_MAP_BASE,      CP_CODE_MAP_SIZE,       MPU_ATTR_READ_EXEC,          MEM_ATTR_INT_SRAM},
#ifdef CP_ROM_DATA_MAP_BASE
    // ROM data is located at a different address range from ROM code
    {CP_ROM_DATA_MAP_BASE,  CP_ROM_DATA_MAP_SIZE,   MPU_ATTR_READ,          MEM_ATTR_INT_SRAM},
#endif
    {RAM_BASE,              RAM_TOTAL_SIZE,         MPU_ATTR_READ_WRITE,    MEM_ATTR_INT_SRAM},
#if ((RAMCP_BASE < RAM_BASE) || (RAMCP_BASE >= RAM_BASE + RAM_TOTAL_SIZE))
    {RAMCP_BASE,            RAMCP_SIZE,             MPU_ATTR_READ_WRITE,    MEM_ATTR_INT_SRAM},
#endif
#ifdef MEM_POOL_BASE
#if (((MEM_POOL_BASE + MEM_POOL_SIZE - 1) < RAM_BASE) || (MEM_POOL_BASE > (RAM_BASE + RAM_TOTAL_SIZE - 1)))
    {MEM_POOL_BASE,         MEM_POOL_SIZE,          MPU_ATTR_READ_WRITE,    MEM_ATTR_INT_SRAM},
#if ((MEM_POOL_BASE >= RAM_BASE) && (MEM_POOL_BASE <= RAM_BASE + RAM_TOTAL_SIZE))
    #error MEM_POOL IS A SUBSET OF RAM, MEMFault will occur!!!
#endif
#endif
#endif
#if defined(LIBC_ROM) && (defined(CHIP_BEST1306P) || defined(CHIP_BEST1502P) || defined(CHIP_BEST1503))
    {0x04020000,            0x10000,                MPU_ATTR_READ_EXEC,     MEM_ATTR_INT_SRAM},
    {0x26000000,            0x10000,                MPU_ATTR_READ,          MEM_ATTR_INT_SRAM},
#endif
    {CMU_BASE,              0x01000000,             MPU_ATTR_READ_WRITE,    MEM_ATTR_DEVICE},
};

#else
static CP_DATA_LOC const  mpu_regions_t mpu_table_cp[] = {
    {0,                 0x800,      MPU_ATTR_NO_ACCESS},
    {FLASHX_BASE,       0x4000000,  MPU_ATTR_NO_ACCESS},
    {FLASH_BASE,        0x4000000,  MPU_ATTR_NO_ACCESS},
    {FLASH_NC_BASE,     0x4000000,  MPU_ATTR_NO_ACCESS},
};
#endif

void cp_accel_register_init_done_callback(CP_ACCEL_INIT_DONE_CB callback)
{
    cp_accel_init_done_cb = callback;
}

static CP_TEXT_SRAM_LOC NOINLINE void accel_loop(void)
{
    uint32_t lock;
    uint8_t task_index = 0, event_index = 0;
    bool msg_flag = false;
    uint8_t msg[CP_TASK_MAX][CP_EVENT_MAX];
#if (CP_CPU_USAGE_INTVL_MS > 0)
    uint32_t time;
    uint32_t interval;
#endif

    mpu_setup_cp(mpu_table_cp, ARRAY_SIZE(mpu_table_cp));

    if (cp_accel_init_done_cb != NULL) {
        cp_accel_init_done_cb();
    }

    while (1) {
        lock = int_lock_global();

        msg_flag = cp_env.cp_msg_recv;
        cp_env.cp_msg_recv = false;
        memcpy(msg, (uint8_t *)cp_env.cp_msg, sizeof(cp_env.cp_msg));
        memset((uint8_t *)cp_env.cp_msg, 0, sizeof(cp_env.cp_msg));
#if !defined(SPECIFIC_FREQ_POWER_CONSUMPTION_MEASUREMENT_ENABLE)
        if (false == msg_flag) {
#if (CP_CPU_USAGE_INTVL_MS > 0)
            time = hal_sys_timer_get();
#endif
            cp_in_sleep = true;
            __WFI();
            cp_in_sleep = false;
#if (CP_CPU_USAGE_INTVL_MS > 0)
            if (stats_interval) {
                sleep_time += hal_sys_timer_get() - time;
            }
#endif
        }
#endif

#if (CP_CPU_USAGE_INTVL_MS > 0)
        time = hal_sys_timer_get();
        if (stats_interval) {
            interval = time - stats_start_time;
            if (interval >= stats_interval) {
                if (sleep_time > UINT32_MAX / 100) {
                    sleep_ratio = (uint64_t)sleep_time * 100 / interval;
                } else {
                    sleep_ratio = sleep_time * 100 / interval;
                }
                sleep_time = 0;
                stats_start_time = time;
            }
        }
#endif

        int_unlock_global(lock);

#if (CP_CPU_USAGE_INTVL_MS > 0) && (CP_CPU_USAGE_TRACE_INTVL_MS > 0)
        if (stats_interval && stats_trace_interval) {
            if (time - stats_trace_time >= stats_trace_interval) {
#ifndef USE_TRACE_ID
                static const char CP_DATA_LOC usage_str[] = "CP CPU USAGE: busy=%d sleep=%d";
                TRACE(0, usage_str, (100 - sleep_ratio), sleep_ratio);
#endif
                stats_trace_time = time;
            }
        }
#endif

        if (msg_flag) {
            for (task_index = 0; task_index < CP_TASK_MAX; task_index++) {
                for (event_index = 0; event_index < CP_EVENT_MAX; event_index++) {
                    LOCK_CP_PROCESS();
                    if ((msg[task_index][event_index]) && (cp_task_env.p_desc[task_index].cp_work_main)) {
                        cp_task_env.p_desc[task_index].cp_work_main(event_index);
                    }
                    UNLOCK_CP_PROCESS();
                }
            }
        }
    }
}

static void accel_main(void)
{
    TRACE(1, "%s", __func__);

    system_cp_init(!ram_inited);

#ifndef CORE_SLEEP_POWER_DOWN
    ram_inited = true;
#endif

    memset((uint8_t *)&cp_env, 0, sizeof(cp_env));

    hal_trace_open_cp(cp_trace_buffer_ctrl, cp_trace_crash_notify);

    hal_mcu2cp_open_cp(HAL_MCU2CP_ID_0, (HAL_MCU2CP_RX_IRQ_HANDLER)mcu2cp_msg_arrived, NULL, false);
    hal_mcu2cp_open_cp(HAL_MCU2CP_ID_1, NULL, NULL, false);

    hal_mcu2cp_start_recv_cp(HAL_MCU2CP_ID_0);
    //hal_mcu2cp_start_recv_cp(HAL_MCU2CP_ID_1);

#if (CP_CPU_USAGE_INTVL_MS > 0)
    stats_interval = MS_TO_TICKS(CP_CPU_USAGE_INTVL_MS);
    stats_start_time = hal_sys_timer_get();
    sleep_time = 0;
#if (CP_CPU_USAGE_TRACE_INTVL_MS > 0)
    stats_trace_interval = MS_TO_TICKS(CP_CPU_USAGE_TRACE_INTVL_MS);
    stats_trace_time = stats_start_time;
#endif
    sleep_ratio = 0;
#endif

    cp_accel_inited = true;

    accel_loop();
}

static SRAM_TEXT_LOC unsigned int cp2mcu_msg_arrived(const unsigned char *data, unsigned int len)
{
    uint8_t task_id = CP_TASK_ID_GET((uint32_t)data);
    // TRACE(2, "%s, task_id = %d", __func__, task_id);

    if (task_id >= CP_TASK_MAX) {
        return -1;
    }

    if (cp_task_env.p_desc[task_id].mcu_evt_hdlr) {
        cp_task_env.p_desc[task_id].mcu_evt_hdlr((uint32_t)data);
    }

    return len;
}

int cp_accel_open(enum CP_TASK_TYPE task_id, struct cp_task_desc const * p_task_desc)
{
    uint32_t lock;
    int ret = 0;

    if ((task_id >= CP_TASK_MAX) || (p_task_desc == NULL)) {
        TRACE(1, "%s task id error", __func__);
        return -1;
    }

    TRACE(1, "%s, task id = %d, cp_state = %d init %d", __func__, task_id,
                    cp_task_env.p_desc[task_id].cp_accel_state, cp_accel_inited);

    lock = int_lock();

    if (cp_task_env.p_desc[task_id].cp_accel_state != CP_ACCEL_STATE_CLOSED) {
        TRACE(1, "%s cp_accel_state error", __func__);
        ret = -2;
        goto _exit;
    }

    cp_task_env.p_desc[task_id].cp_accel_state = CP_ACCEL_STATE_OPENING;
    cp_task_env.p_desc[task_id].cp_work_main = p_task_desc->cp_work_main;
    cp_task_env.p_desc[task_id].cp_evt_hdlr = p_task_desc->cp_evt_hdlr;
    cp_task_env.p_desc[task_id].mcu_evt_hdlr = p_task_desc->mcu_evt_hdlr;
    cp_task_env.p_desc[task_id].mcu_sys_ctrl_hdlr = p_task_desc->mcu_sys_ctrl_hdlr;

    if (false == cp_accel_inited) {
#ifdef CORE_SLEEP_POWER_DOWN
        hal_chip_wake_lock(HAL_CHIP_WAKE_LOCK_USER_CP);
#endif

        hal_cmu_cp_enable(RAMCP_BASE + RAMCP_SIZE, (uint32_t)accel_main);

        hal_mcu2cp_open_mcu(HAL_MCU2CP_ID_0, (HAL_MCU2CP_RX_IRQ_HANDLER)cp2mcu_msg_arrived, (HAL_MCU2CP_TX_IRQ_HANDLER)mcu2cp_msg_sent, false);
        hal_mcu2cp_open_mcu(HAL_MCU2CP_ID_1, (HAL_MCU2CP_RX_IRQ_HANDLER)cp2mcu_sys_arrived, NULL, false);

        hal_mcu2cp_start_recv_mcu(HAL_MCU2CP_ID_0);
        hal_mcu2cp_start_recv_mcu(HAL_MCU2CP_ID_1);
    }

    cp_task_env.p_desc[task_id].cp_accel_state = CP_ACCEL_STATE_OPENED;

_exit:
    int_unlock(lock);

    return ret;
}

int cp_accel_close(enum CP_TASK_TYPE task_id)
{
    uint8_t i = 0;
    uint32_t lock;

    TRACE(4, "%s, task id = %d, cp_state = %d init %d", __func__,
                task_id, cp_task_env.p_desc[task_id].cp_accel_state, cp_accel_inited);

    lock = int_lock();
    LOCK_CP_PROCESS();

    if (cp_task_env.p_desc[task_id].cp_accel_state == CP_ACCEL_STATE_CLOSED) {
        goto _exit;
    }

    cp_task_env.p_desc[task_id].cp_accel_state = CP_ACCEL_STATE_CLOSING;
    cp_task_env.p_desc[task_id].cp_work_main = NULL;
    cp_task_env.p_desc[task_id].cp_evt_hdlr = NULL;
    cp_task_env.p_desc[task_id].mcu_evt_hdlr = NULL;

    for (i=0; i<CP_TASK_MAX; i++) {
        if (cp_task_env.p_desc[i].cp_accel_state == CP_ACCEL_STATE_OPENED || \
            cp_task_env.p_desc[i].cp_accel_state == CP_ACCEL_STATE_OPENING) {
            goto _exit;
        }
    }

    if (cp_accel_inited) {
        cp_accel_inited = false;

        hal_mcu2cp_close_mcu(HAL_MCU2CP_ID_0);
        hal_mcu2cp_close_mcu(HAL_MCU2CP_ID_1);

        hal_cmu_cp_disable();

        hal_mcu2cp_close_cp(HAL_MCU2CP_ID_0);
        hal_mcu2cp_close_cp(HAL_MCU2CP_ID_1);

        hal_trace_close_cp();

        system_cp_term();

#if (CP_CPU_USAGE_INTVL_MS > 0)
        sleep_ratio = 0;
#endif

#ifdef CORE_SLEEP_POWER_DOWN
        hal_chip_wake_unlock(HAL_CHIP_WAKE_LOCK_USER_CP);
#endif
    }

_exit:
    cp_task_env.p_desc[task_id].cp_accel_state = CP_ACCEL_STATE_CLOSED;

    UNLOCK_CP_PROCESS();
    int_unlock(lock);

    return 0;
}

void SRAM_TEXT_LOC cp_accel_init(void)
{
    cp_accel_inited = false;
}

int SRAM_TEXT_LOC cp_accel_init_done(void)
{
    return cp_accel_inited;
}

int cp_accel_send_event_mcu2cp(uint8_t event)
{
    uint32_t lock;

    if ((false == cp_accel_inited) || (cp_env.mcu2cp_tx_count > MAX_CP_MSG_NUM)) {
        TRACE(2,
              "send_evt error, cp_accel_inited = %d, event pending count = %d",
              cp_accel_inited,
              cp_env.mcu2cp_tx_count);

        TRACE(2,
              "send evt task_id = %d, event = %d",
              CP_TASK_ID_GET(event),
              CP_EVENT_GET(event));
        return -1;
    }

    // TRACE(1, "current CP tx count:%d", cp_env.mcu2cp_tx_count);

    lock = int_lock();

    if (cp_env.mcu2cp_tx_count > 0) {
        cp_env.mcu2cp_tx_pending[cp_env.mcu2cp_tx_count - 1] = event;
        cp_env.mcu2cp_tx_count++;
    } else {
        req_event = event;
        cp_env.mcu2cp_tx_count = 1;
        hal_mcu2cp_send_mcu(HAL_MCU2CP_ID_0, &req_event, 1);
    }

    int_unlock(lock);

    return 0;
}

int CP_TEXT_SRAM_LOC cp_accel_send_event_cp2mcu(uint8_t event)
{
    return hal_mcu2cp_send_cp(HAL_MCU2CP_ID_0, (unsigned char *)(uint32_t)event, 0);
}

int SRAM_TEXT_LOC cp_accel_busy(enum CP_TASK_TYPE task_id)
{
    if (cp_task_env.p_desc[task_id].cp_accel_state != CP_ACCEL_STATE_CLOSED) {
        if (cp_task_env.p_desc[task_id].cp_accel_state == CP_ACCEL_STATE_OPENED && cp_in_sleep &&
                !hal_mcu2cp_local_irq_pending_cp(HAL_MCU2CP_ID_0)) {
            return false;
        }
        return true;
    }

    return false;
}

int cp_accel_get_cpu_usage(struct CP_ACCEL_CPU_USAGE_T *usage)
{
    int ret = -1;
#if (CP_CPU_USAGE_INTVL_MS > 0)
    uint8_t sleep;

    if (!usage) {
        return 1;
    }

    if (cp_accel_inited) {
        sleep = sleep_ratio;
        usage->sleep = sleep;
        usage->busy = 100 - sleep;
        ret = 0;
    } else {
        ret = 2;
    }
#endif

    return ret;
}

#endif
