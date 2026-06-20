/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifdef APP_CHIP_BRIDGE_MODULE
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_trace.h"
#include "app_chip_bridge.h"
#include "cqueue.h"
#include "heap_api.h"
#include "app_uart_dma_thread.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_iomux.h"

extern uint32_t __chip_bridge_task_cmd_table_start[];
extern uint32_t __chip_bridge_task_cmd_table_end[];

extern uint32_t __chip_bridge_instant_cmd_table_start[];
extern uint32_t __chip_bridge_instant_cmd_table_end[];

#define CHIP_BRIDGE_TASK_CMD_PTR_FROM_ENTRY_INDEX(entryIndex)   \
    ((app_chip_bridge_task_cmd_instance_t *)                    \
    ((uint32_t)__chip_bridge_task_cmd_table_start +             \
    (entryIndex)*sizeof(app_chip_bridge_task_cmd_instance_t)))

#define CHIP_BRIDGE_INSTANT_CMD_PTR_FROM_ENTRY_INDEX(entryIndex)    \
    ((app_chip_bridge_instant_cmd_instance_t *)                     \
    ((uint32_t)__chip_bridge_instant_cmd_table_start +              \
    (entryIndex)*sizeof(app_chip_bridge_instant_cmd_instance_t)))

typedef struct
{
    uint16_t  cmdcode;
    uint16_t  cmdseq;
    uint16_t  packetlen;
    uint8_t   content[APP_CHIP_BRIDGE_MAX_DATA_PACKET_SIZE];
} __attribute__((packed)) app_chip_bridge_data_packet_t;

typedef struct
{
    uint16_t  cmdcode;
    uint16_t  cmdseq;
    uint8_t   content[APP_CHIP_BRIDGE_MAX_DATA_PACKET_SIZE];
} __attribute__((packed)) app_chip_bridge_data_packet_handle_t;

static bool app_chip_bridge_inited = false;

static CQueue app_chip_bridge_receive_queue;
static uint8_t app_chip_bridge_rx_buff[APP_CHIP_BRIDGE_RX_BUFF_SIZE];
static uint16_t g_chip_cmdseq = 0;

osSemaphoreDef(app_chip_bridge_wait_tx_done);
osSemaphoreId app_chip_bridge_wait_tx_done_id = NULL;

osSemaphoreDef(app_chip_bridge_wait_cmd_rsp);
osSemaphoreId app_chip_bridge_wait_cmd_rsp_id = NULL;

osSemaphoreDef(app_chip_bridge_wait_peer_device_ready);
osSemaphoreId app_chip_bridge_wait_peer_device_ready_id = NULL;

static osThreadId app_chip_bridge_rx_thread_id = NULL;
static void app_chip_bridge_rx_thread(const void *arg);
osThreadDef(app_chip_bridge_rx_thread, osPriorityHigh, 1,
    (APP_CHIP_BRIDGE_RX_THREAD_STACK_SIZE), "chip_bridge_rx_thread");
#define APP_CHIP_BRIDGE_RX_THREAD_SIGNAL_DATA_RECEIVED   0x01

static osThreadId app_chip_bridge_tx_thread_id = NULL;
static void app_chip_bridge_tx_thread(const void *arg);
osThreadDef(app_chip_bridge_tx_thread, osPriorityHigh, 1,
    (APP_CHIP_BRIDGE_TX_THREAD_STACK_SIZE), "chip_bridge_tx_thread");

static app_chip_bridge_task_cmd_instance_t* app_chip_bridge_get_task_cmd_entry(uint16_t cmdcode);
static int32_t app_chip_bridge_queue_push(CQueue* ptrQueue, const void* ptrData, uint32_t length);
static uint16_t app_chip_bridge_queue_get_next_entry_length(CQueue* ptrQueue);
static void app_chip_bridge_queue_pop(CQueue* ptrQueue, uint8_t *buff, uint32_t len);
static int32_t app_chip_bridge_get_queue_length(CQueue *ptrQueue);
static app_chip_bridge_instant_cmd_instance_t* app_chip_bridge_get_instant_cmd_entry(uint16_t cmdcode);

static void app_chip_bridge_transmit_data(
    app_chip_bridge_task_cmd_instance_t* pCmdInstance,
    uint16_t cmdcode, uint8_t *p_buff, uint16_t length)
{
    if (!app_chip_bridge_inited)
    {
        TRACE(0, "[%s]: WARNING, not inited", __func__);
        return;
    }

    app_chip_bridge_data_packet_t dataPacket;
    dataPacket.cmdcode = cmdcode;
    dataPacket.cmdseq = g_chip_cmdseq++;
    dataPacket.packetlen = length;

    ASSERT(length <= APP_CHIP_BRIDGE_MAX_DATA_PACKET_SIZE,
        "core bridge tx size %d > max %d", length,
        APP_CHIP_BRIDGE_MAX_DATA_PACKET_SIZE);

    memcpy(dataPacket.content, p_buff, length);
    app_uart_send_data((uint8_t *)&dataPacket, length + APP_UART_HEADER_DATA_SIZE);

    if (pCmdInstance->app_chip_bridge_transmisson_done_handler)
    {
        pCmdInstance->app_chip_bridge_transmisson_done_handler(
            cmdcode, p_buff, length);
    }
}

void app_chip_bridge_send_instant_cmd_data(uint16_t cmdcode, uint8_t *p_buff, uint16_t length)
{
    if (!app_chip_bridge_inited)
    {
        return;
    }

    app_chip_bridge_data_packet_t dataPacket;
    dataPacket.cmdcode = cmdcode;
    dataPacket.cmdseq = g_chip_cmdseq++;
    dataPacket.packetlen = length;

    ASSERT(length <= APP_CHIP_BRIDGE_MAX_DATA_PACKET_SIZE,
        "%s: chip bridge tx size %d > max %d", __func__, length,
        APP_CHIP_BRIDGE_MAX_DATA_PACKET_SIZE);

    memcpy(dataPacket.content, p_buff, length);
    app_uart_send_data((uint8_t *)&dataPacket, length + APP_UART_HEADER_DATA_SIZE);
}

void app_chip_bridge_send_data_without_waiting_rsp(uint16_t cmdcode, uint8_t *p_buff, uint16_t length)
{
    app_chip_bridge_task_cmd_instance_t* pInstance =
            app_chip_bridge_get_task_cmd_entry(cmdcode);

    ASSERT(0 == pInstance->wait_rsp_timeout_ms, \
           "%s: cmdCode:0x%x wait rsp timeout:%d should be 0", __func__, \
           pInstance->wait_rsp_timeout_ms, \
           cmdcode);

    app_chip_bridge_transmit_data(pInstance, cmdcode, p_buff, length);
}

void app_chip_bridge_send_data_with_waiting_rsp(uint16_t cmdcode, uint8_t *p_buff, uint16_t length)
{
    app_chip_bridge_task_cmd_instance_t* pInstance =
            app_chip_bridge_get_task_cmd_entry(cmdcode);

    ASSERT(pInstance->wait_rsp_timeout_ms > 0, \
           "%s: cmdCode:0x%x wait rsp timeout:%d should be > 0", __func__, \
           pInstance->wait_rsp_timeout_ms, \
           cmdcode);

    app_chip_bridge_transmit_data(pInstance, cmdcode, p_buff, length);

    uint32_t stime = 0, etime = 0;

    stime = hal_sys_timer_get();

    int32_t returnValue = 0;
    returnValue = osSemaphoreWait(app_chip_bridge_wait_cmd_rsp_id,
        pInstance->wait_rsp_timeout_ms);

    etime = hal_sys_timer_get();

    if ((0 == returnValue) || (-1 == returnValue))
    {
        TRACE(2,"%s err = %d",__func__,returnValue);
        TRACE(2,"cchip bridge:wait rsp to cmdcode=%s timeout(%d)", \
              pInstance->log_cmd_code_str, \
              (etime-stime)/16000);

        if (pInstance->app_chip_bridge_wait_rsp_timeout_handle)
        {
            pInstance->app_chip_bridge_wait_rsp_timeout_handle(
                p_buff,length);
        }
    }
}

static void app_chip_bridge_data_received(uint8_t* data, uint32_t len)
{
    if (!app_chip_bridge_inited)
    {
        TRACE(0, "[%s]: WARNING: not inited", __func__);
        return;
    }

    app_chip_bridge_data_packet_t* pDataPacket = (app_chip_bridge_data_packet_t *)data;

    if (CHIP_BRIDGE_CMD_GROUP_INSTANT == CHIP_BRIDGE_CMD_GROUP(pDataPacket->cmdcode))
    {
        app_chip_bridge_instant_cmd_instance_t* pInstance =
            app_chip_bridge_get_instant_cmd_entry(pDataPacket->cmdcode);

        if (pInstance->cmdhandler)
        {
            pInstance->cmdhandler(pDataPacket->content, pDataPacket->packetlen);
        }
        else
        {
            TRACE(1,"chip_bridge:%s cmd not handled",__func__);
        }
    }
    else
    {
        app_chip_bridge_queue_push(&app_chip_bridge_receive_queue, data, len);
    }
}

void app_chip_bridge_data_tx_done(const void* data, unsigned int len)
{
    if (app_chip_bridge_wait_tx_done_id)
    {
        osSemaphoreRelease(app_chip_bridge_wait_tx_done_id);
    }
}

static void app_chip_bridge_rx_handler(uint8_t* p_data_buff, uint16_t length)
{
    app_chip_bridge_data_packet_t* pDataPacket = (app_chip_bridge_data_packet_t *)p_data_buff;

    if (CHIP_BRIDGE_CMD_GROUP_TASK == CHIP_BRIDGE_CMD_GROUP(pDataPacket->cmdcode))
    {
        app_chip_bridge_task_cmd_instance_t* pInstance =
            app_chip_bridge_get_task_cmd_entry(pDataPacket->cmdcode);

        if (pInstance->cmdhandler)
        {
            pInstance->cmdhandler(pDataPacket->content, pDataPacket->packetlen);
        }
        else
        {
            TRACE(1,"chip_bridge:%s cmd not handled",__func__);
        }
    }
    else
    {
        ASSERT(false, "chip_bridge instant cmd shouldn't get here.");
    }
}

static void app_chip_bridge_rx_thread(const void *arg)
{
    while(1)
    {
        osEvent evt;
        // wait any signal
        evt = osSignalWait(0x0, osWaitForever);

        // get role from signal value
        if (osEventSignal == evt.status)
        {
            if (evt.value.signals & APP_CHIP_BRIDGE_RX_THREAD_SIGNAL_DATA_RECEIVED)
            {
                while (app_chip_bridge_get_queue_length(&app_chip_bridge_receive_queue) > 0)
                {
                    static uint8_t rcv_tmp_buffer[APP_CHIP_BRIDGE_RX_THREAD_TMP_BUF_SIZE];

                    uint16_t rcv_length =
                        app_chip_bridge_queue_get_next_entry_length(
                            &app_chip_bridge_receive_queue);

                    ASSERT(rcv_length <= sizeof(rcv_tmp_buffer),
                        "[%s]: Bad rcv_length=%d, > %d", __func__, rcv_length, sizeof(rcv_tmp_buffer));

                    if (rcv_length > 0)
                    {
                        app_chip_bridge_queue_pop(
                            &app_chip_bridge_receive_queue,
                            rcv_tmp_buffer, rcv_length);
                        app_chip_bridge_rx_handler(rcv_tmp_buffer, rcv_length);
                    }
                }
            }
        }
    }
}

typedef struct
{
    uint16_t cmdCode;
    uint16_t cmd_len;
    uint8_t *cmd_buffer;
} APP_CHIP_BRIDGE_TX_MAILBOX_PARAM_T;

osMailQDef (app_chip_bridge_tx_mailbox, APP_CHIP_BRIDGE_TX_MAILBOX_MAX,
            APP_CHIP_BRIDGE_TX_MAILBOX_PARAM_T);

static osMutexId app_chip_bridge_tx_mutex_id = NULL;
osMutexDef(app_chip_bridge_tx_mutex);

static osMailQId app_chip_bridge_tx_mailbox_id = NULL;

static heap_handle_t app_chip_bridge_tx_mailbox_heap;
static uint8_t app_chip_bridge_tx_mailbox_heap_pool[APP_CHIP_BRIDGE_MAX_XFER_DATA_SIZE*(APP_CHIP_BRIDGE_TX_MAILBOX_MAX/2)];

void app_chip_bridge_tx_mailbox_heap_init(void)
{
    app_chip_bridge_tx_mailbox_heap =
        heap_register(app_chip_bridge_tx_mailbox_heap_pool,
        sizeof(app_chip_bridge_tx_mailbox_heap_pool));
}

void *app_chip_bridge_tx_mailbox_heap_malloc(uint32_t size)
{
    void *ptr = NULL;

    if (size) {
        ptr = heap_malloc(app_chip_bridge_tx_mailbox_heap, size);
        ASSERT(ptr!=NULL, "[%s]: null ptr, size:%d", __func__, size);
    }
    return ptr;
}

void app_chip_bridge_tx_mailbox_heap_free(void *rmem)
{
    if (rmem){
        heap_free(app_chip_bridge_tx_mailbox_heap, rmem);
    }
}

static int32_t app_chip_bridge_tx_mailbox_get(APP_CHIP_BRIDGE_TX_MAILBOX_PARAM_T** msg_p)
{
    osEvent evt;

    evt = osMailGet(app_chip_bridge_tx_mailbox_id, osWaitForever);
    if (evt.status == osEventMail)
    {
        *msg_p = (APP_CHIP_BRIDGE_TX_MAILBOX_PARAM_T *)evt.value.p;
        return 0;
    }
    return -1;
}

static int32_t app_chip_bridge_tx_mailbox_raw(APP_CHIP_BRIDGE_TX_MAILBOX_PARAM_T** msg_p)
{
    osEvent evt;
    evt = osMailGet(app_chip_bridge_tx_mailbox_id, 0);
    if (evt.status == osEventMail)
    {
        *msg_p = (APP_CHIP_BRIDGE_TX_MAILBOX_PARAM_T *)evt.value.p;
        return 0;
    }
    return -1;
}

static int32_t app_chip_bridge_tx_mailbox_free(APP_CHIP_BRIDGE_TX_MAILBOX_PARAM_T* msg_p)
{
    osStatus status;
    app_chip_bridge_tx_mailbox_heap_free((void *)msg_p->cmd_buffer);
    status = osMailFree(app_chip_bridge_tx_mailbox_id, msg_p);
    return (int32_t)status;
}

static int32_t app_chip_bridge_tx_mailbox_put(APP_CHIP_BRIDGE_TX_MAILBOX_PARAM_T* msg_src)
{
#define ALLOC_TRYCNT 7
    int i;
    APP_CHIP_BRIDGE_TX_MAILBOX_PARAM_T *msg_p = NULL;
    osStatus status;
    uint32_t dly_list[] = {5,10,25,50,50,50,50};

    for (i = 0; i < ALLOC_TRYCNT; i++) {
        msg_p = (APP_CHIP_BRIDGE_TX_MAILBOX_PARAM_T*)osMailAlloc(app_chip_bridge_tx_mailbox_id, 0);
        if (msg_p != NULL) {
            break;
        }
        TRACE(0, "[%s]: WARNING, alloc mbox failed, try %d", __func__, i);
        osDelay(dly_list[i]);
    }
    if (msg_p == NULL) {
        app_chip_bridge_task_cmd_instance_t *cmdEntry;

        TRACE(0, "DUMP MBOX DATA:");
        for (i=0; i < APP_CHIP_BRIDGE_TX_MAILBOX_MAX; i++){
            app_chip_bridge_tx_mailbox_raw(&msg_p);
            cmdEntry = app_chip_bridge_get_task_cmd_entry(msg_p->cmdCode);
            TRACE(0, "cltr_maillbox[%d]: cmdcode=%s",i, cmdEntry->log_cmd_code_str);
        }
        TRACE(0,"DUMP MBOX END");
        ASSERT(false, "[%s]: alloc mbox error", __func__);
    }
    msg_p->cmdCode = msg_src->cmdCode;
    msg_p->cmd_len = msg_src->cmd_len ;
    msg_p->cmd_buffer = msg_src->cmd_buffer;
    status = osMailPut(app_chip_bridge_tx_mailbox_id, msg_p);
    return (int32_t)status;
}

static int32_t app_chip_bridge_tx_mailbox_init(void)
{
    app_chip_bridge_tx_mailbox_id = osMailCreate(osMailQ(app_chip_bridge_tx_mailbox), NULL);
    if (NULL == app_chip_bridge_tx_mailbox_id)
    {
        // should increase OS_DYNAMIC_MEM_SIZE
        ASSERT(0, "Failed to Create core bridge mailbox\n");
        return -1;
    }
    return 0;
}

static void app_chip_bridge_tx_thread_init(void)
{
    if (app_chip_bridge_tx_mutex_id == NULL) {
        app_chip_bridge_tx_mutex_id = osMutexCreate(osMutex(app_chip_bridge_tx_mutex));
    }

    app_chip_bridge_tx_mailbox_heap_init();
    app_chip_bridge_tx_mailbox_init();
    if (app_chip_bridge_tx_thread_id == NULL) {
        app_chip_bridge_tx_thread_id =
            osThreadCreate(osThread(app_chip_bridge_tx_thread), NULL);
    }
}

static app_chip_bridge_task_cmd_instance_t* app_chip_bridge_get_task_cmd_entry(uint16_t cmdcode)
{
    for (uint32_t index = 0;
        index < ((uint32_t)__chip_bridge_task_cmd_table_end-
        (uint32_t)__chip_bridge_task_cmd_table_start)/sizeof(app_chip_bridge_task_cmd_instance_t);
        index++) {
        if (CHIP_BRIDGE_TASK_CMD_PTR_FROM_ENTRY_INDEX(index)->cmdcode == cmdcode) {
            return CHIP_BRIDGE_TASK_CMD_PTR_FROM_ENTRY_INDEX(index);
        }
    }

    ASSERT(0, "[%s] find cmdcode index failed, wrong_cmdcode=%x", __func__, cmdcode);
    return NULL;
}

static app_chip_bridge_instant_cmd_instance_t* app_chip_bridge_get_instant_cmd_entry(uint16_t cmdcode)
{
    for (uint32_t index = 0;
        index < ((uint32_t)__chip_bridge_instant_cmd_table_end-
        (uint32_t)__chip_bridge_instant_cmd_table_start)/sizeof(app_chip_bridge_instant_cmd_instance_t);
        index++) {
        if (CHIP_BRIDGE_INSTANT_CMD_PTR_FROM_ENTRY_INDEX(index)->cmdcode == cmdcode) {
            return CHIP_BRIDGE_INSTANT_CMD_PTR_FROM_ENTRY_INDEX(index);
        }
    }

    ASSERT(0, "[%s] find cmdcode index failed, wrong_cmdcode=%x", __func__, cmdcode);
    return NULL;
}

void app_chip_bridge_tx_thread(const void *arg)
{
    int err = 0;
    APP_CHIP_BRIDGE_TX_MAILBOX_PARAM_T *msg_p = NULL;

    while(1)
    {
        err = app_chip_bridge_tx_mailbox_get(&msg_p);
        if (err)
        {
            TRACE(0, "[%s]: WARNING: err=%d", __func__, err);
            continue;
        }
        if (CHIP_BRIDGE_CMD_GROUP_TASK == CHIP_BRIDGE_CMD_GROUP(msg_p->cmdCode))
        {
            app_chip_bridge_task_cmd_instance_t* pTaskCmdInstance =
                app_chip_bridge_get_task_cmd_entry(msg_p->cmdCode);

            pTaskCmdInstance->chip_bridge_cmd_transmit_handler(
                msg_p->cmd_buffer, msg_p->cmd_len);
        }
        else if (CHIP_BRIDGE_CMD_GROUP_INSTANT == CHIP_BRIDGE_CMD_GROUP(msg_p->cmdCode))
        {
            app_chip_bridge_instant_cmd_instance_t* pInstantCmdInstance =
                app_chip_bridge_get_instant_cmd_entry(msg_p->cmdCode);

            pInstantCmdInstance->chip_bridge_cmd_transmit_handler(
                msg_p->cmd_buffer, msg_p->cmd_len);
        }
        else
        {
            ASSERT(false, "Wrong core bridge cmd code 0x%x", msg_p->cmdCode);
        }

        app_chip_bridge_tx_mailbox_free(msg_p);
        if (msg_p != NULL)
        {
            msg_p = NULL;
        }
    }
}

int32_t app_chip_bridge_send_cmd(uint16_t cmd_code, uint8_t *p_buff, uint16_t length)
{
    int nRet = 0;
    uint8_t *cmd_buffer = NULL;
    APP_CHIP_BRIDGE_TX_MAILBOX_PARAM_T msg;

    if (!app_chip_bridge_inited)
    {
        TRACE(0, "[%s]: WARNING: not inited", __func__);
        return 0;
    }
    ASSERT(length <= APP_CHIP_BRIDGE_MAX_XFER_DATA_SIZE, "%s: Bad length=%d", __func__, length);
    if (length > 0)
    {
        cmd_buffer = (uint8_t *)app_chip_bridge_tx_mailbox_heap_malloc(length);
        ASSERT(cmd_buffer!=NULL, "[%s]: null ptr", __func__);
        memcpy(cmd_buffer, p_buff, length);
    }
    msg.cmdCode = cmd_code;
    msg.cmd_len = length;
    msg.cmd_buffer = cmd_buffer;

    osMutexWait(app_chip_bridge_tx_mutex_id, osWaitForever);
    nRet = app_chip_bridge_tx_mailbox_put(&msg);
    osMutexRelease(app_chip_bridge_tx_mutex_id);
    return nRet;
}

int32_t app_chip_bridge_send_rsp(uint16_t rsp_code, uint8_t *p_buff, uint16_t length)
{
    if (!app_chip_bridge_inited)
    {
        return 0;
    }

    APP_CHIP_BRIDGE_TX_MAILBOX_PARAM_T msg = {0,};
    app_chip_bridge_task_cmd_instance_t* cmdEntry;

    ASSERT(length + sizeof(rsp_code) <= APP_CHIP_BRIDGE_MAX_XFER_DATA_SIZE, \
           "%s p_buff overflow: %u", \
           __func__, \
           length);

    msg.cmd_len = length + sizeof(rsp_code);
    msg.cmd_buffer = (uint8_t *)app_chip_bridge_tx_mailbox_heap_malloc(msg.cmd_len);
    *(uint16_t *)msg.cmd_buffer = rsp_code;
    memcpy(msg.cmd_buffer + sizeof(rsp_code), p_buff, length);

    msg.cmdCode = CHIP_TASK_CMD_RSP;

    cmdEntry = app_chip_bridge_get_task_cmd_entry(msg.cmdCode);
    cmdEntry->chip_bridge_cmd_transmit_handler(msg.cmd_buffer, msg.cmd_len);

    app_chip_bridge_tx_mailbox_free(&msg);

    return 0;
}

static void app_chip_bridge_send_cmd_rsp_handler(uint8_t *p_buff, uint16_t length)
{
    app_chip_bridge_task_cmd_instance_t* cmdEntry;
    app_chip_bridge_data_packet_t cmddata;

    ASSERT(length <= APP_CHIP_BRIDGE_MAX_DATA_PACKET_SIZE,
        "%s: length(%d) out of range(%d)", __func__,
        length, APP_CHIP_BRIDGE_MAX_DATA_PACKET_SIZE);

    cmddata.cmdcode = CHIP_TASK_CMD_RSP;
    cmddata.cmdseq = g_chip_cmdseq++;
    cmddata.packetlen = length;
    memcpy(cmddata.content, p_buff, length);

    app_uart_send_data((uint8_t *)&cmddata, length + APP_UART_HEADER_DATA_SIZE);

    cmdEntry = app_chip_bridge_get_task_cmd_entry(*(uint16_t *)p_buff);

    TRACE(2,"chip bridge:-------TRS RSP to cmdcode=%s seq = %d------->", \
          cmdEntry->log_cmd_code_str,cmddata.cmdseq);
}

static void app_chip_bridge_cmd_rsp_handler(uint8_t *p_buff, uint16_t length)
{
    //real cmd code responsed
    uint16_t rspcode = ((p_buff[1] << 8) | p_buff[0]);
    app_chip_bridge_task_cmd_instance_t* pInstance =
                app_chip_bridge_get_task_cmd_entry(rspcode);

     TRACE(2,"chip bridge:<-----------RCV_RSP: cmdcode=%s----------->", \
          pInstance->log_cmd_code_str);

    uint32_t timeout_ms = pInstance->wait_rsp_timeout_ms;
    if (pInstance->app_chip_bridge_rsp_handle)
    {
        pInstance->app_chip_bridge_rsp_handle(&p_buff[2], length - 2);

        if (timeout_ms > 0)
        {
            osSemaphoreRelease(app_chip_bridge_wait_cmd_rsp_id);
        }
    }
    else
    {
        TRACE(0,"ibrt_ui_log:rsp no handler");
    }
}

CHIP_BRIDGE_TASK_COMMAND_TO_ADD(CHIP_TASK_CMD_RSP,
                                "chip bridge task cmd rsp",
                                app_chip_bridge_send_cmd_rsp_handler,
                                app_chip_bridge_cmd_rsp_handler,
                                0,
                                NULL,
                                NULL,
                                NULL);

#define APP_CHIP_BRIDGE_QUEUE_DATA_LEN_BYTES    2

static void app_chip_bridge_queue_init(CQueue* ptrQueue, uint8_t* ptrBuf, uint32_t bufLen)
{
    uint32_t lock = int_lock();
    memset(ptrBuf, 0, bufLen);
    InitCQueue(ptrQueue, bufLen, (CQItemType *)ptrBuf);
    int_unlock(lock);
}

static int32_t app_chip_bridge_queue_push(CQueue* ptrQueue, const void* ptrData, uint32_t length)
{
    int32_t nRet = -1;
    uint32_t lock = int_lock();
    if (length > 0)
    {
        uint16_t dataLen = (uint16_t)length;
        int queueAvailableLen = AvailableOfCQueue(ptrQueue);
        if ((dataLen+APP_CHIP_BRIDGE_QUEUE_DATA_LEN_BYTES) <= queueAvailableLen)
        {
            EnCQueue(ptrQueue, (CQItemType *)&dataLen, APP_CHIP_BRIDGE_QUEUE_DATA_LEN_BYTES);
            EnCQueue(ptrQueue, (CQItemType *)ptrData, length);
            nRet = 0;
        }
    }
    int_unlock(lock);
    osSignalSet(app_chip_bridge_rx_thread_id, APP_CHIP_BRIDGE_RX_THREAD_SIGNAL_DATA_RECEIVED);

    return nRet;
}

static uint16_t app_chip_bridge_queue_get_next_entry_length(CQueue* ptrQueue)
{
    uint8_t *e1 = NULL, *e2 = NULL;
    uint32_t len1 = 0, len2 = 0;
    uint16_t length = 0;
    uint8_t* ptr;

    uint32_t lock = int_lock();

    ptr = (uint8_t *)&length;
    // get the length of the fake message
    PeekCQueue(ptrQueue, APP_CHIP_BRIDGE_QUEUE_DATA_LEN_BYTES, &e1, &len1, &e2, &len2);

    memcpy(ptr,e1,len1);
    memcpy(ptr+len1,e2,len2);

    int_unlock(lock);

    return length;
}

static void app_chip_bridge_queue_pop(CQueue* ptrQueue, uint8_t *buff, uint32_t len)
{
    uint8_t *e1 = NULL, *e2 = NULL;
    uint32_t len1 = 0, len2 = 0;

    uint32_t lock = int_lock();
    // overcome the two bytes of msg length
    DeCQueue(ptrQueue, 0, APP_CHIP_BRIDGE_QUEUE_DATA_LEN_BYTES);

    PeekCQueue(ptrQueue, len, &e1, &len1, &e2, &len2);
    if (len==(len1+len2)){
        memcpy(buff,e1,len1);
        memcpy(buff+len1,e2,len2);
        DeCQueue(ptrQueue, 0, len);

        // reset the poped data to ZERO
        memset(e1, 0, len1);
        memset(e2, 0, len2);
    }else{
        memset(buff, 0x00, len);
    }
    int_unlock(lock);
}

static int32_t app_chip_bridge_get_queue_length(CQueue *ptrQueue)
{
    int32_t nRet = 0;

    uint32_t lock = int_lock();
    nRet = LengthOfCQueue(ptrQueue);
    int_unlock(lock);

    return nRet;
}

void app_chip_bridge_free_tx_mailbox(void)
{
    APP_CHIP_BRIDGE_TX_MAILBOX_PARAM_T *msg_p = NULL;

    while(1)
    {
        osEvent evt;

        evt = osMailGet(app_chip_bridge_tx_mailbox_id, 5);
        if (evt.status == osEventMail)
        {
            msg_p = (APP_CHIP_BRIDGE_TX_MAILBOX_PARAM_T *)evt.value.p;
            app_chip_bridge_tx_mailbox_free(msg_p);
        }
        else
        {
            break;
        }
    }
}

void app_chip_bridge_init(void)
{
    if (app_chip_bridge_inited) {
        return;
    }
    ASSERT(APP_CHIP_BRIDGE_MAX_XFER_DATA_SIZE \
        > (APP_CHIP_BRIDGE_MAX_DATA_PACKET_SIZE+4)/APP_CHIP_BRIDGE_TX_MAILBOX_MAX/2,
        "core bridge init: xfer data size %d too small, should larger than %d",
        APP_CHIP_BRIDGE_MAX_XFER_DATA_SIZE,
        (APP_CHIP_BRIDGE_MAX_DATA_PACKET_SIZE+4)/APP_CHIP_BRIDGE_TX_MAILBOX_MAX/2);

    ASSERT(APP_CHIP_BRIDGE_RX_BUFF_SIZE > (APP_CHIP_BRIDGE_MAX_DATA_PACKET_SIZE+4) * 2,
        "core bridge init: rx buff size %d too small, should large than %d",
        APP_CHIP_BRIDGE_RX_BUFF_SIZE, (APP_CHIP_BRIDGE_MAX_DATA_PACKET_SIZE+4) * 2);

    ASSERT(APP_CHIP_BRIDGE_TX_THREAD_STACK_SIZE > (APP_CHIP_BRIDGE_MAX_DATA_PACKET_SIZE+4) * 2,
        "core bridge tx thread: stack size %d too small, should larger than %d",
        APP_CHIP_BRIDGE_TX_THREAD_STACK_SIZE,(APP_CHIP_BRIDGE_MAX_DATA_PACKET_SIZE+4) * 2);

    ASSERT(APP_CHIP_BRIDGE_RX_THREAD_STACK_SIZE > (APP_CHIP_BRIDGE_MAX_DATA_PACKET_SIZE+4) * 2,
        "core bridge rx thread: stack size %d too small, should larger than %d",
        APP_CHIP_BRIDGE_RX_THREAD_STACK_SIZE,(APP_CHIP_BRIDGE_MAX_DATA_PACKET_SIZE+4) * 2);

    ASSERT(APP_CHIP_BRIDGE_RX_THREAD_STACK_SIZE > APP_CHIP_BRIDGE_RX_THREAD_TMP_BUF_SIZE * 2,
        "core bridge rx thread: stack size %d too small, should larger than tmp size %d",
        APP_CHIP_BRIDGE_RX_THREAD_STACK_SIZE, APP_CHIP_BRIDGE_RX_THREAD_TMP_BUF_SIZE * 2);

    memset(&app_chip_bridge_receive_queue, 0, sizeof(app_chip_bridge_receive_queue));

    app_chip_bridge_queue_init(&app_chip_bridge_receive_queue, \
                            app_chip_bridge_rx_buff, \
                            sizeof(app_chip_bridge_rx_buff));
    g_chip_cmdseq = 0;

    app_chip_bridge_wait_tx_done_id =
        osSemaphoreCreate(osSemaphore(app_chip_bridge_wait_tx_done), 0);

    app_chip_bridge_wait_cmd_rsp_id =
        osSemaphoreCreate(osSemaphore(app_chip_bridge_wait_cmd_rsp), 0);

    app_chip_bridge_wait_peer_device_ready_id =
        osSemaphoreCreate(osSemaphore(app_chip_bridge_wait_peer_device_ready), 0);

    app_chip_bridge_tx_thread_init();

    if (app_chip_bridge_rx_thread_id == NULL) {
        app_chip_bridge_rx_thread_id =
            osThreadCreate(osThread(app_chip_bridge_rx_thread), NULL);
    }

    app_uart_register_data_receiver(app_chip_bridge_data_received);
    app_uart_start_receiving_data();

    app_chip_bridge_inited = true;
}

/*
* Test case for UART1 communication
*
*/
static void app_set_test_mode_cmd_handler(uint8_t *p_buff, uint16_t length)
{
    TRACE(0, "[%s] len = %d", __func__, length);
    app_chip_bridge_send_data_with_waiting_rsp(SET_TEST_MODE_CMD, (uint8_t *)p_buff, length);
}

static void app_set_test_mode_cmd_receive_handler(uint8_t *p_buff, uint16_t length)
{
    TRACE(1,"%s", __func__);
    TRACE(1,"uart receive: %s, len=%d", p_buff, length);
    app_chip_bridge_send_rsp(SET_TEST_MODE_CMD, p_buff, length);

}

static void app_set_test_mode_cmd_wait_rsp_timeout(uint8_t *p_buff, uint16_t length)
{
    TRACE(1,"%s", __func__);

    static uint8_t resend_time = RESEND_TIME;
    if(resend_time > 0)
    {
        app_chip_bridge_send_cmd(SET_TEST_MODE_CMD, p_buff, length);
        resend_time--;
    }
    else
    {
        resend_time = RESEND_TIME;
    }
}

static void app_set_test_mode_cmd_send_rsp_handler(uint8_t *p_buff, uint16_t length)
{
    TRACE(1,"%s", __func__);

}

CHIP_BRIDGE_TASK_COMMAND_TO_ADD(SET_TEST_MODE_CMD,
                                "test for chip bridge",
                                app_set_test_mode_cmd_handler,
                                app_set_test_mode_cmd_receive_handler,
                                APP_CHIP_BRIDGE_DEFAULT_WAIT_RSP_TIMEOUT_MS,
                                app_set_test_mode_cmd_wait_rsp_timeout,
                                app_set_test_mode_cmd_send_rsp_handler,
                                NULL);
#endif
