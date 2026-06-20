/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#include "cmsis.h"
#include "cmsis_os.h"
#include "plat_types.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "string.h"
#include "cqueue.h"
#include "hal_sys2bth.h"
#include "app_rpc_api.h"
#include "rpc_queue.h"
#include "rpc_bth_dsp.h"
#include "rpc_rx_thread.h"
#if defined(DSP_HIFI4_TRC_TO_MCU) && defined(RMT_TRC_IN_MSG_CHAN)
#include "rx_dsp_hifi4_trc.h"
#endif

#define BTH_DSP_TRACE(s,...)
// TRACE(s, ##__VA_ARGS__)

#ifdef RPC_BTH_DSP_DEMO_ENABLE
#include "app_rpc_demo.h"
#endif

// #define RPC_BTH_DSP_RAW_LOG_DUMP_EN

#ifndef RPC_BTH_DSP_RX_BUFF_SIZE
#define RPC_BTH_DSP_RX_BUFF_SIZE (2048)
#endif

#ifndef RPC_BTH_DSP_TX_BUFF_SIZE
#define RPC_BTH_DSP_TX_BUFF_SIZE (2048)
#endif

#ifndef RPC_SYS2BTH_ID
#define RPC_SYS2BTH_ID HAL_SYS2BTH_ID_1
#endif

#ifndef BTH_DSP_SEMA_MAX_NUM
#define BTH_DSP_SEMA_MAX_NUM (65535u)
#endif

extern uint32_t __rpc_bth_dsp_cmd_table_start[];
extern uint32_t __rpc_bth_dsp_cmd_table_end[];

#define BTH_DSP_TASK_CMD_PTR_FROM_ENTRY_INDEX(entryIndex)   \
    ((rpc_cmd_inst_t *)                    \
    ((uint32_t)__rpc_bth_dsp_cmd_table_start +             \
    (entryIndex)*sizeof(rpc_cmd_inst_t)))

typedef struct {
    osSemaphoreId_t tx_queue_sema;
    osSemaphoreId_t cmd_tx_done_sema;
    osSemaphoreId_t cmd_wait_rsp_sema;
    CQueue tx_queue;
    CQueue rx_queue;
    uint8_t *tx_queue_buff;
    uint8_t *rx_queue_buff;
    uint16_t tx_queue_len;
    uint16_t rx_queue_len;
    uint16_t cmd_seq;
    bool core_ctx_inited;
} rpc_core_ctx_t;

rpc_core_ctx_t bth_dsp_core_ctx = {
    .core_ctx_inited= false,
};

static osThreadId bth_dsp_tx_thread_id = NULL;
static void bth_dsp_tx_thread(const void *arg);
osThreadDef(bth_dsp_tx_thread, osPriorityHigh, 1,
    (4096), "bth_dsp_tx_thread");

static volatile uint8_t bth_dsp_tx_buff[RPC_BTH_DSP_TX_BUFF_SIZE];
static volatile uint8_t bth_dsp_rx_buff[RPC_BTH_DSP_RX_BUFF_SIZE];

static bool init_done_arrived = false;
typedef void (*app_dsp_hifi_init_done_callback)(void);
static app_dsp_hifi_init_done_callback init_done_cb = NULL;

void app_dsp_hifi_register_init_done_callback(app_dsp_hifi_init_done_callback cb)
{
    init_done_cb = cb;
}

static rpc_core_ctx_t* bth_dsp_get_ctx(void)
{
    return &bth_dsp_core_ctx;
}

static int32_t tx_queue_sema_wait(void)
{
    rpc_core_ctx_t* ctx = bth_dsp_get_ctx();
    return osSemaphoreAcquire(ctx->tx_queue_sema, osWaitForever);
}

static void tx_queue_sema_set(void)
{
    rpc_core_ctx_t* ctx = bth_dsp_get_ctx();
    if (ctx->tx_queue_sema) {
        osSemaphoreRelease(ctx->tx_queue_sema);
    }
}

static int32_t cmd_tx_done_sema_wait(void)
{
    rpc_core_ctx_t* ctx = bth_dsp_get_ctx();
    return osSemaphoreAcquire(ctx->cmd_tx_done_sema, osWaitForever);
}

static void cmd_tx_done_sema_set(void)
{
    rpc_core_ctx_t* ctx = bth_dsp_get_ctx();
    if (ctx->cmd_tx_done_sema) {
        osSemaphoreRelease(ctx->cmd_tx_done_sema);
    }
}

static int32_t cmd_rsp_sema_wait(uint32_t timeout)
{
    rpc_core_ctx_t* ctx = bth_dsp_get_ctx();
    return osSemaphoreAcquire(ctx->cmd_wait_rsp_sema, timeout);
}

static void cmd_rsp_sema_set(void)
{
    rpc_core_ctx_t* ctx = bth_dsp_get_ctx();
    if (ctx->cmd_wait_rsp_sema) {
        osSemaphoreRelease(ctx->cmd_wait_rsp_sema);
    }
}

static unsigned int bth_dsp_data_received(const void* data, unsigned int len)
{
#if defined(DSP_HIFI4_TRC_TO_MCU) && defined(RMT_TRC_IN_MSG_CHAN)
    if (dsp_hifi4_trace_rx_handler(data, len)) {
        return len;
    }
#endif

#ifdef RPC_BTH_DSP_RAW_LOG_DUMP_EN
    DUMP8("%02x ", data, len);
#endif

    if ((len == sizeof("Ready")) && (strncmp((char *)data, "Ready", strlen("Ready")) == 0)) {
        // TRACE(0, "[%s] recv: %s", __func__, (char *)data);
        if (!init_done_arrived && init_done_cb) {
            init_done_cb();
            init_done_arrived = true;
        }
        return len;
    }

    rpc_core_ctx_t* ctx = bth_dsp_get_ctx();
    rpc_queue_push(&ctx->rx_queue, data, len);

    // notify rx thread
    RPC_RX_MAILBOX_PARAM_T msg;
    msg.rpc_core_id = APP_RPC_CORE_BTH_DSP;
    rpc_rx_mailbox_put(&msg);

    return len;
}

static void bth_dsp_data_tx_done(const void* data, unsigned int len)
{
    cmd_tx_done_sema_set();
}

static void bth_dsp_core_open(void)
{
    int ret;
    ret = hal_sys2bth_open(RPC_SYS2BTH_ID, bth_dsp_data_received, bth_dsp_data_tx_done, false);
    ASSERT(ret == 0, "hal_sys2bth_opened failed: %d", ret);
    ret = hal_sys2bth_start_recv(RPC_SYS2BTH_ID);
    ASSERT(ret == 0, "hal_sys2bth_start_recv failed: %d", ret);
}

static void bth_dsp_core_close(void)
{
    int ret;
    ret = hal_sys2bth_stop_recv(RPC_SYS2BTH_ID);
    ASSERT(ret == 0, "hal_sys2bth_stop_recv failed: %d", ret);
    ret = hal_sys2bth_close(RPC_SYS2BTH_ID);
    ASSERT(ret == 0, "hal_sys2bth_close failed: %d", ret);
}

static int bth_dsp_send(const void *data, uint32_t len)
{
    return hal_sys2bth_send(RPC_SYS2BTH_ID, data, len);
}

static rpc_cmd_inst_t* bth_dsp_get_task_cmd_entry(uint16_t cmdcode)
{
    for (uint32_t index = 0;
        index < ((uint32_t)__rpc_bth_dsp_cmd_table_end-
        (uint32_t)__rpc_bth_dsp_cmd_table_start)/sizeof(rpc_cmd_inst_t);
        index++) {
        if (BTH_DSP_TASK_CMD_PTR_FROM_ENTRY_INDEX(index)->cmdcode == cmdcode) {
            return BTH_DSP_TASK_CMD_PTR_FROM_ENTRY_INDEX(index);
        }
    }

    ASSERT(0, "[%s] find cmdcode index failed, wrong_cmdcode=%x", __func__, cmdcode);
    return NULL;
}

static void bth_dsp_print_cmdcode(uint16_t cmdCode, uint16_t seq,uint8_t *p_buff)
{
    POSSIBLY_UNUSED rpc_cmd_inst_t* pInstance =
        bth_dsp_get_task_cmd_entry(cmdCode);

    BTH_DSP_TRACE(1,"bth2dsp:-------TRS: cmdcode=%s seq = %d ------->", \
              pInstance->cmd_code_str,seq);
}

static int bth_dsp_transmit_data(
    rpc_cmd_inst_t* pCmdInstance,
    uint16_t cmdcode, uint8_t *p_buff, uint16_t length)
{
    int ret;
    rpc_core_ctx_t* ctx = bth_dsp_get_ctx();
    rpc_core_data_t dataPacket;
    dataPacket.cmdcode = cmdcode;
    dataPacket.cmdseq = ctx->cmd_seq++;

    ASSERT(length <= RPC_MAX_DATA_PACKET_SIZE,
        "rpc_bth2dsp tx size %d > max %d", length,
        RPC_MAX_DATA_PACKET_SIZE);

    bth_dsp_print_cmdcode(cmdcode, dataPacket.cmdseq,p_buff);

    memcpy(dataPacket.content, p_buff, length);
    ret = bth_dsp_send((const uint8_t *)&dataPacket, length+sizeof(dataPacket.cmdcode) + sizeof(dataPacket.cmdseq));

#ifdef RPC_BTH_DSP_RAW_LOG_DUMP_EN
    DUMP8("%02x ", (uint8_t*)(&dataPacket), length + sizeof(dataPacket.cmdcode) + sizeof(dataPacket.cmdseq));
#endif

    cmd_tx_done_sema_wait();

    if (pCmdInstance->cmd_transmit_done_handler) {
        pCmdInstance->cmd_transmit_done_handler(
            cmdcode, p_buff, length);
    }

    return ret;
}

static void bth_dsp_send_cmd_rsp_handler(uint8_t *p_buff, uint16_t length)
{
    rpc_core_ctx_t* ctx = bth_dsp_get_ctx();
    POSSIBLY_UNUSED rpc_cmd_inst_t* cmdEntry;
    rpc_core_data_t cmddata;

    ASSERT(length <= RPC_MAX_DATA_PACKET_SIZE,
        "%s: length(%d) out of range(%d)", __func__,
        length, RPC_MAX_DATA_PACKET_SIZE);

    cmddata.cmdcode = BTH_DSP_TASK_CMD_RSP;
    cmddata.cmdseq = ctx->cmd_seq++;
    memcpy(cmddata.content, p_buff, length);

    cmdEntry = bth_dsp_get_task_cmd_entry(*(uint16_t *)p_buff);
    BTH_DSP_TRACE(2,"bth2dsp:-------TRS RSP to cmdcode=%s seq = %d------->", \
          cmdEntry->cmd_code_str,cmddata.cmdseq);

    bth_dsp_send((const uint8_t *)&cmddata, length+sizeof(cmddata.cmdcode) + sizeof(cmddata.cmdseq));

#ifdef RPC_BTH_DSP_RAW_LOG_DUMP_EN
    DUMP8("%02x ", (uint8_t*)&cmddata, length + sizeof(cmddata.cmdseq) + sizeof(cmddata.cmdcode));
#endif

    cmd_tx_done_sema_wait();
}

static void bth_dsp_cmd_rsp_recevied_handler(uint8_t *p_buff, uint16_t length)
{
    //real cmd code responsed
    uint16_t rspcode = ((p_buff[1] << 8) | p_buff[0]);
    rpc_cmd_inst_t* pInstance =
                bth_dsp_get_task_cmd_entry(rspcode);

     BTH_DSP_TRACE(2,"bth2dsp:<-----------RCV_RSP: cmdcode=%s-------", \
          pInstance->cmd_code_str);

    uint32_t timeout_ms = pInstance->cmd_wait_rsp_timeout_ms;
    if (pInstance->cmd_rsp_received_handle) {
        pInstance->cmd_rsp_received_handle(&p_buff[2],
            length - 2);
        if (timeout_ms > 0) {
            cmd_rsp_sema_set();
        }
    } else {
        BTH_DSP_TRACE(0,"bth2dsp:rsp no handler");
    }
}

RPC_BTH_DSP_TASK_CMD_TO_ADD(BTH_DSP_TASK_CMD_RSP,
                            "bth_dsp task cmd rsp",
                            bth_dsp_send_cmd_rsp_handler,
                            bth_dsp_cmd_rsp_recevied_handler,
                            0,
                            NULL,
                            NULL,
                            NULL);

static void bth_dsp_tx_process_handler(uint8_t* p_data_buff, uint16_t length)
{
    rpc_cmd_data_t *dataPacket = (rpc_cmd_data_t*)p_data_buff;

    rpc_cmd_inst_t* pInstance =
        bth_dsp_get_task_cmd_entry(dataPacket->cmdcode);
    if (pInstance->cmd_transmit_handler) {
        pInstance->cmd_transmit_handler(&dataPacket->content[0], \
                                           length - sizeof(dataPacket->cmdcode));
    }
}

static void bth_dsp_tx_thread(const void *arg)
{
    rpc_core_ctx_t* ctx = bth_dsp_get_ctx();
    for(;;) {
        osStatus_t ret = tx_queue_sema_wait();
        if (ret == osOK) {
            if (rpc_get_queue_length(&ctx->tx_queue) > 0) {
                uint8_t send_tmp_buffer[512];
                uint16_t send_length =
                    rpc_queue_get_next_entry_length(
                        &ctx->tx_queue);

                ASSERT(send_length <= sizeof(send_tmp_buffer),
                    "send data size %d is bigger than tmp tx buf size!", send_length);

                if (send_length > 0) {
                    rpc_queue_pop(&ctx->tx_queue, send_tmp_buffer, send_length);
                    bth_dsp_tx_process_handler(send_tmp_buffer, send_length);
                }
            }
        }
    }
}

static void bth_dsp_rx_process_handler(uint8_t* p_data_buff, uint16_t length)
{
    rpc_core_data_t* pDataPacket = (rpc_core_data_t *)p_data_buff;

    //ingore the unkown cmdcode, instead of enter assert
    if (pDataPacket->cmdcode >= BTH_DSP_TASK_CMD_QTY)
        return;

    rpc_cmd_inst_t* pInstance =
        bth_dsp_get_task_cmd_entry(pDataPacket->cmdcode);
    
    if (BTH_DSP_TASK_CMD_RSP != pDataPacket->cmdcode) {
        BTH_DSP_TRACE(0,"bth2dsp:<-----------RCV:cmdcode=%s seq = %d len=%d-------", \
                pInstance->cmd_code_str, pDataPacket->cmdseq ,length);
    }else {
        BTH_DSP_TRACE(2,"bth2dsp:<-----------RCV_RSP: cmdcode=%s seq = %d len %d-------", \
                pInstance->cmd_code_str,pDataPacket->cmdseq,length);
    }

#ifdef RPC_BTH_DSP_RAW_LOG_DUMP_EN
    DUMP8("%02x ", p_data_buff, length);
#endif
    if (pInstance->cmd_received_handler) {
        pInstance->cmd_received_handler(pDataPacket->content, \
                length-sizeof(pDataPacket->cmdcode) - sizeof(pDataPacket->cmdseq));
    }
}

int32_t rpc_bth_dsp_send_cmd(uint16_t cmdcode,const uint8_t *p_buff, uint16_t length)
{
    ASSERT(length <= RPC_MAX_DATA_PACKET_SIZE,
        "core bridge tx size %d > max %d", length,
        RPC_MAX_DATA_PACKET_SIZE);

    int ret = RPC_RES_FAILD;
    rpc_core_ctx_t* ctx = bth_dsp_get_ctx();
    rpc_cmd_data_t dataPacket;
    dataPacket.cmdcode = cmdcode;
    memcpy(dataPacket.content, p_buff, length);

    ret = rpc_queue_push(&ctx->tx_queue, (const uint8_t*)&dataPacket, sizeof(cmdcode) + length);
    // notify tx thread
    tx_queue_sema_set();

    return ret;
}

int32_t rpc_bth_dsp_send_data_no_rsp(uint16_t cmdcode, uint8_t *p_buff, uint16_t length)
{
    int ret = RPC_RES_FAILD;
    rpc_cmd_inst_t* pInstance =
            bth_dsp_get_task_cmd_entry(cmdcode);

    ASSERT(0 == pInstance->cmd_wait_rsp_timeout_ms, \
           "%s: cmdCode:0x%x wait rsp timeout:%d should be 0", __func__, \
           pInstance->cmd_wait_rsp_timeout_ms, \
           cmdcode);

    ret = bth_dsp_transmit_data(pInstance, cmdcode, p_buff, length);
    return ret;
}

int32_t rpc_bth_dsp_send_data_wait_rsp(uint16_t cmdcode, uint8_t *p_buff, uint16_t length)
{
    int ret = RPC_RES_FAILD;

    rpc_cmd_inst_t* pInstance =
            bth_dsp_get_task_cmd_entry(cmdcode);
    ASSERT(pInstance->cmd_wait_rsp_timeout_ms > 0, \
           "%s: cmdCode:0x%x wait rsp timeout:%d should be > 0", __func__, \
           pInstance->cmd_wait_rsp_timeout_ms, \
           cmdcode);

    ret = bth_dsp_transmit_data(pInstance, cmdcode, p_buff, length);

    POSSIBLY_UNUSED uint32_t stime = 0, etime = 0;

    stime = hal_sys_timer_get();

    ret = cmd_rsp_sema_wait(pInstance->cmd_wait_rsp_timeout_ms);
    etime = hal_sys_timer_get();
    if (ret != osOK) {
        BTH_DSP_TRACE(2,"%s err = %d",__func__,ret);
        BTH_DSP_TRACE(2,"bth2dsp:wait rsp to cmdcode=%s timeout(%d)", \
              pInstance->cmd_code_str, \
              (etime-stime)/16000);

        if (pInstance->cmd_wait_rsp_timeout_handle)
        {
            pInstance->cmd_wait_rsp_timeout_handle(
                p_buff,length);
        }
    }
    return ret;
}

int32_t rpc_bth_dsp_send_cmd_rsp(uint16_t rsp_code, uint8_t *p_buff, uint16_t length)
{
    ASSERT(length <= RPC_MAX_DATA_PACKET_SIZE,
        "bth2dsp tx size %d > max %d", length,
        RPC_MAX_DATA_PACKET_SIZE);
    int ret = RPC_RES_SUCCESS;

    rpc_cmd_data_t dataPacket;
    dataPacket.cmdcode = rsp_code;
    memcpy(dataPacket.content, p_buff, length);

    rpc_cmd_inst_t* cmdEntry =
            bth_dsp_get_task_cmd_entry(BTH_DSP_TASK_CMD_RSP);
    cmdEntry->cmd_transmit_handler((uint8_t*)&dataPacket, length + sizeof(dataPacket.cmdcode));

    return ret;
}

void rpc_bth_dsp_rx_queue_data_process_handler(void)
{
    rpc_core_ctx_t* ctx = bth_dsp_get_ctx();
    if (rpc_get_queue_length(&ctx->rx_queue) > 0) {
        uint8_t rcv_tmp_buffer[512];

        uint16_t rcv_length =
            rpc_queue_get_next_entry_length(
                &ctx->rx_queue);

        ASSERT(rcv_length <= sizeof(rcv_tmp_buffer),
            "received data size %d is bigger than tmp rx buf size!", rcv_length);

        if (rcv_length > 0) {
            rpc_queue_pop(&ctx->rx_queue, rcv_tmp_buffer, rcv_length);

            bth_dsp_rx_process_handler(rcv_tmp_buffer, rcv_length);
        }
    }
}

void rpc_bth_dsp_ctx_init(void)
{
    init_done_arrived = false;

    bth_dsp_core_open();
    rpc_core_ctx_t* ctx = bth_dsp_get_ctx();
    if(!ctx->core_ctx_inited) {
        ctx->tx_queue_sema = osSemaphoreNew(BTH_DSP_SEMA_MAX_NUM, 0, NULL);
        if (!ctx->tx_queue_sema) {
            BTH_DSP_TRACE(0, "cannot create tx_queue_sema for rpc");
        }

        ctx->cmd_tx_done_sema = osSemaphoreNew(BTH_DSP_SEMA_MAX_NUM, 0, NULL);
        if (!ctx->cmd_tx_done_sema) {
            BTH_DSP_TRACE(0, "cannot create cmd_tx_done_sema for rpc");
        }

        ctx->cmd_wait_rsp_sema = osSemaphoreNew(BTH_DSP_SEMA_MAX_NUM, 0, NULL);
        if (!ctx->cmd_wait_rsp_sema) {
            BTH_DSP_TRACE(0, "cannot create cmd_wait_rsp_sema for rpc");
        }

        ctx->cmd_seq = 0;
        ctx->core_ctx_inited = true;
        ctx->tx_queue_buff = (uint8_t *)bth_dsp_tx_buff;
        ctx->tx_queue_len = sizeof(bth_dsp_tx_buff);
        memset(&ctx->tx_queue, 0, sizeof(ctx->tx_queue));
        rpc_queue_init(&ctx->tx_queue, ctx->tx_queue_buff, ctx->tx_queue_len);

        ctx->rx_queue_buff = (uint8_t *)bth_dsp_rx_buff;
        ctx->rx_queue_len = sizeof(bth_dsp_rx_buff);
        memset(&ctx->rx_queue, 0, sizeof(ctx->rx_queue));
        rpc_queue_init(&ctx->rx_queue, ctx->rx_queue_buff, ctx->rx_queue_len);

        if (bth_dsp_tx_thread_id == NULL) {
            bth_dsp_tx_thread_id =
                osThreadCreate(osThread(bth_dsp_tx_thread), NULL);
        }
    }
#ifdef RPC_BTH_DSP_DEMO_ENABLE
    app_rpc_test_demo();
#endif

}

void rpc_bth_dsp_ctx_deinit(void)
{
    bth_dsp_core_close();

    init_done_cb = NULL;
}