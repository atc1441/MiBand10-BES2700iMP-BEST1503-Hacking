/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_trace.h"
#include "app_sensor_hub.h"
#include "sensor_hub.h"
#include "cqueue.h"
#include "heap_api.h"

//#define SENSOR_HUB_CORE_BRIDGE_RAW_LOG_DUMP_EN

extern uint32_t __core_bridge_task_cmd_table_start[];
extern uint32_t __core_bridge_task_cmd_table_end[];

extern uint32_t __core_bridge_instant_cmd_table_start[];
extern uint32_t __core_bridge_instant_cmd_table_end[];

extern "C" uint32_t hal_sys_timer_get(void);

#define CORE_BRIDGE_TASK_CMD_PTR_FROM_ENTRY_INDEX(entryIndex)   \
    ((app_core_bridge_task_cmd_instance_t *)                    \
    ((uint32_t)__core_bridge_task_cmd_table_start +             \
    (entryIndex)*sizeof(app_core_bridge_task_cmd_instance_t)))

#define CORE_BRIDGE_INSTANT_CMD_PTR_FROM_ENTRY_INDEX(entryIndex)    \
    ((app_core_bridge_instant_cmd_instance_t *)                     \
    ((uint32_t)__core_bridge_instant_cmd_table_start +              \
    (entryIndex)*sizeof(app_core_bridge_instant_cmd_instance_t)))

typedef struct
{
    uint16_t  cmdcode;
    uint16_t cmdseq;
    uint8_t   content[APP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE];
} __attribute__((packed)) app_core_bridge_data_packet_t;

static bool app_core_bridge_inited = false;

static CQueue app_core_bridge_receive_queue;
static uint8_t app_core_bridge_rx_buff[APP_CORE_BRIDGE_RX_BUFF_SIZE];
static uint16_t g_sensor_hub_cmdseq = 0;

osSemaphoreDef(app_core_bridge_wait_tx_done);
osSemaphoreId app_core_bridge_wait_tx_done_id = NULL;

osSemaphoreDef(app_core_bridge_wait_cmd_rsp);
osSemaphoreId app_core_bridge_wait_cmd_rsp_id = NULL;


static app_core_bridge_task_cmd_instance_t* app_core_bridge_get_task_cmd_entry(uint16_t cmdcode);
static int32_t app_core_bridge_queue_push(CQueue* ptrQueue, const void* ptrData, uint32_t length);
static uint16_t app_core_bridge_queue_get_next_entry_length(CQueue* ptrQueue);
static void app_core_bridge_queue_pop(CQueue* ptrQueue, uint8_t *buff, uint32_t len);
static int32_t app_core_bridge_get_queue_length(CQueue *ptrQueue);
static app_core_bridge_instant_cmd_instance_t* app_core_bridge_get_instant_cmd_entry(uint16_t cmdcode);

static void app_core_bridge_print_cmdcode(uint16_t cmdCode, uint16_t seq,uint8_t *p_buff)
{
    app_core_bridge_task_cmd_instance_t* pInstance =
        app_core_bridge_get_task_cmd_entry(cmdCode);

    if (CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, MCU_SENSOR_HUB_TASK_CMD_RSP)
        != cmdCode)
    {
        TRACE(1,"core bridge:-------TRS: cmdcode=%s seq = %d ------->", \
              pInstance->log_cmd_code_str,seq);
    }
    else
    {
        uint16_t responsedCmd = ((p_buff[1] << 8) | p_buff[0]);
        TRACE(1,"core bridge:transmit RSP to cmdcode=0x%x seq = %d", responsedCmd,seq);
    }
}

static void app_core_bridge_transmit_data(
    app_core_bridge_task_cmd_instance_t* pCmdInstance,
    uint16_t cmdcode, uint8_t *p_buff, uint16_t length)
{
    if (!app_core_bridge_inited)
    {
        return;
    }

    app_core_bridge_data_packet_t dataPacket;
    dataPacket.cmdcode = cmdcode;
    dataPacket.cmdseq = g_sensor_hub_cmdseq++;

    ASSERT(length <= APP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE,
        "core bridge tx size %d > max %d", length,
        APP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE);

    memcpy(dataPacket.content, p_buff, length);

    sensor_hub_send((const uint8_t *)&dataPacket, length+sizeof(dataPacket.cmdcode) + sizeof(dataPacket.cmdseq));

    app_core_bridge_print_cmdcode(cmdcode, dataPacket.cmdseq,p_buff);
#ifdef SENSOR_HUB_CORE_BRIDGE_RAW_LOG_DUMP_EN
    DUMP8("%02x ", (uint8_t*)(&dataPacket), length + sizeof(dataPacket.cmdcode) + sizeof(dataPacket.cmdseq));
#endif
    //uint32_t currentMs = hal_sys_timer_get();

    osSemaphoreWait(app_core_bridge_wait_tx_done_id, osWaitForever);

    //TRACE(0, "tx cost %d", hal_sys_timer_get()-currentMs);

    if (pCmdInstance->app_core_bridge_transmisson_done_handler)
    {
        pCmdInstance->app_core_bridge_transmisson_done_handler(
            cmdcode, p_buff, length);
    }
}

void app_core_bridge_send_instant_cmd_data(uint16_t cmdcode, uint8_t *p_buff, uint16_t length)
{
    if (!app_core_bridge_inited)
    {
        return;
    }
    
    app_core_bridge_data_packet_t dataPacket;
    dataPacket.cmdcode = cmdcode;
    dataPacket.cmdseq = g_sensor_hub_cmdseq++;

    ASSERT(length <= APP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE,
        "%s: core bridge tx size %d > max %d", __func__, length,
        APP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE);

    memcpy(dataPacket.content, p_buff, length);

    sensor_hub_send((const uint8_t *)&dataPacket, length+sizeof(dataPacket.cmdcode) + sizeof(dataPacket.cmdseq));

    osSemaphoreWait(app_core_bridge_wait_tx_done_id, osWaitForever);
}

void app_core_bridge_send_data_without_waiting_rsp(uint16_t cmdcode, uint8_t *p_buff, uint16_t length)
{
    app_core_bridge_task_cmd_instance_t* pInstance =
            app_core_bridge_get_task_cmd_entry(cmdcode);

    ASSERT(0 == pInstance->wait_rsp_timeout_ms, \
           "%s: cmdCode:0x%x wait rsp timeout:%d should be 0", __func__, \
           pInstance->wait_rsp_timeout_ms, \
           cmdcode);

    app_core_bridge_transmit_data(pInstance, cmdcode, p_buff, length);
}

void app_core_bridge_send_data_with_waiting_rsp(uint16_t cmdcode, uint8_t *p_buff, uint16_t length)
{
    app_core_bridge_task_cmd_instance_t* pInstance =
            app_core_bridge_get_task_cmd_entry(cmdcode);

    ASSERT(pInstance->wait_rsp_timeout_ms > 0, \
           "%s: cmdCode:0x%x wait rsp timeout:%d should be > 0", __func__, \
           pInstance->wait_rsp_timeout_ms, \
           cmdcode);

    app_core_bridge_transmit_data(pInstance, cmdcode, p_buff, length);

    uint32_t stime = 0, etime = 0;

    stime = hal_sys_timer_get();

    int32_t returnValue = 0;
    returnValue = osSemaphoreWait(app_core_bridge_wait_cmd_rsp_id,
        pInstance->wait_rsp_timeout_ms);

    etime = hal_sys_timer_get();

    if ((0 == returnValue) || (-1 == returnValue))
    {
        TRACE(2,"%s err = %d",__func__,returnValue);
        TRACE(2,"core bridge:wait rsp to cmdcode=%s timeout(%d)", \
              pInstance->log_cmd_code_str, \
              (etime-stime)/16000);

        if (pInstance->app_core_bridge_wait_rsp_timeout_handle)
        {
            pInstance->app_core_bridge_wait_rsp_timeout_handle(
                p_buff,length);
        }
    }
}

unsigned int app_core_bridge_data_received(const void* data, unsigned int len)
{
    if (!app_core_bridge_inited)
    {
        return len;
    }

    app_core_bridge_data_packet_t* pDataPacket = (app_core_bridge_data_packet_t *)data;

    if (CORE_BRIDGE_CMD_GROUP_INSTANT == CORE_BRIDGE_CMD_GROUP(pDataPacket->cmdcode))
    {
        app_core_bridge_instant_cmd_instance_t* pInstance =
            app_core_bridge_get_instant_cmd_entry(pDataPacket->cmdcode);

        if (pInstance->cmdhandler)
        {
            pInstance->cmdhandler(pDataPacket->content, len-sizeof(pDataPacket->cmdcode) - sizeof((pDataPacket->cmdseq)));
        }
        else
        {
            TRACE(1,"core bridge:%s cmd not handled",__func__);
        }
    }
    else
    {
        app_core_bridge_queue_push(&app_core_bridge_receive_queue, data, len);
    }
    return len;
}

void app_core_bridge_data_tx_done(const void* data, unsigned int len)
{
    if (app_core_bridge_wait_tx_done_id)
    {
        osSemaphoreRelease(app_core_bridge_wait_tx_done_id);
    }
}

static void app_core_bridge_rx_handler(uint8_t* p_data_buff, uint16_t length)
{
    app_core_bridge_data_packet_t* pDataPacket = (app_core_bridge_data_packet_t *)p_data_buff;

    if (CORE_BRIDGE_CMD_GROUP_TASK == CORE_BRIDGE_CMD_GROUP(pDataPacket->cmdcode))
    {
        app_core_bridge_task_cmd_instance_t* pInstance =
            app_core_bridge_get_task_cmd_entry(pDataPacket->cmdcode);
        
        if (MCU_SENSOR_HUB_TASK_CMD_RSP != pDataPacket->cmdcode)
        {
            TRACE(0,"core bridge:<-----------RCV:cmdcode=%s seq = %d len=%d-------", \
                  pInstance->log_cmd_code_str, pDataPacket->cmdseq ,length);
        }else{
            TRACE(2,"core bridge:<-----------RCV_RSP: cmdcode=%s seq = %d len %d-------", \
                 pInstance->log_cmd_code_str,pDataPacket->cmdseq,length);
        }

#ifdef SENSOR_HUB_CORE_BRIDGE_RAW_LOG_DUMP_EN
        DUMP8("%02x ", p_data_buff, length);
#endif
        if (pInstance->cmdhandler)
        {
            pInstance->cmdhandler(pDataPacket->content, length-sizeof(pDataPacket->cmdcode) - sizeof(pDataPacket->cmdseq));
        }
        else
        {
            TRACE(1,"core bridge:%s cmd not handled",__func__);
        }
    }
    else
    {
        ASSERT(false, "core bridge instant cmd shouldn't get here.");
    }
}

static osThreadId app_core_bridge_rx_thread_id = NULL;
static void app_core_bridge_rx_thread(const void *arg);
osThreadDef(app_core_bridge_rx_thread, osPriorityHigh, 1,
    (APP_CORE_BRIDGE_RX_THREAD_STACK_SIZE), "core_bridge_rx_thread");
#define APP_CORE_BRIDGE_RX_THREAD_SIGNAL_DATA_RECEIVED  0x01

static void app_core_bridge_rx_thread(const void *arg)
{

    while(1)
    {
        osEvent evt;
        // wait any signal
        evt = osSignalWait(0x0, osWaitForever);

        // get role from signal value
        if (osEventSignal == evt.status)
        {
            if (evt.value.signals & APP_CORE_BRIDGE_RX_THREAD_SIGNAL_DATA_RECEIVED)
            {
                while (app_core_bridge_get_queue_length(&app_core_bridge_receive_queue) > 0)
                {
                    //buffer aligned 4 byte
                    uint32_t rcv_tmp_buffer[APP_CORE_BRIDGE_RX_THREAD_TMP_BUF_SIZE / sizeof(uint32_t)];

                    uint16_t rcv_length =
                        app_core_bridge_queue_get_next_entry_length(
                            &app_core_bridge_receive_queue);

                    ASSERT(rcv_length <= sizeof(rcv_tmp_buffer),
                        "received data size %d is bigger than tmp rx buf size!", rcv_length);

                    if (rcv_length > 0)
                    {
                        app_core_bridge_queue_pop(
                            &app_core_bridge_receive_queue,
                            (uint8_t*)rcv_tmp_buffer, rcv_length);
                        app_core_bridge_rx_handler((uint8_t*)rcv_tmp_buffer, rcv_length);
                    }
                }
            }
        }
    }
}

static osThreadId app_core_bridge_tx_thread_id = NULL;
static void app_core_bridge_tx_thread(const void *arg);
osThreadDef(app_core_bridge_tx_thread, osPriorityHigh, 1,
    (APP_CORE_BRIDGE_TX_THREAD_STACK_SIZE), "core_bridge_tx_thread");

typedef struct
{
    uint16_t cmdCode;
    uint16_t cmd_len;
    uint8_t *cmd_buffer;
} APP_CORE_BRIDGE_TX_MAILBOX_PARAM_T;

osMailQDef (app_core_bridge_tx_mailbox, APP_CORE_BRIDGE_TX_MAILBOX_MAX,
            APP_CORE_BRIDGE_TX_MAILBOX_PARAM_T);

static osMutexId app_core_bridge_tx_mutex_id = NULL;
osMutexDef(app_core_bridge_tx_mutex);

static osMailQId app_core_bridge_tx_mailbox_id = NULL;

static heap_handle_t app_core_bridge_tx_mailbox_heap;
static uint8_t app_core_bridge_tx_mailbox_heap_pool[APP_CORE_BRIDGE_MAX_XFER_DATA_SIZE*(APP_CORE_BRIDGE_TX_MAILBOX_MAX/2)];

void app_core_bridge_tx_mailbox_heap_init(void)
{
    app_core_bridge_tx_mailbox_heap =
        heap_register(app_core_bridge_tx_mailbox_heap_pool,
        sizeof(app_core_bridge_tx_mailbox_heap_pool));
}

void *app_core_bridge_tx_mailbox_heap_malloc(uint32_t size)
{
    void *ptr = NULL;
    if (size){
        ptr = heap_malloc(app_core_bridge_tx_mailbox_heap, size);
        ASSERT(ptr, "%s size:%d", __func__, size);
    }
    return ptr;
}

void app_core_bridge_tx_mailbox_heap_free(void *rmem)
{
    if (rmem){
        heap_free(app_core_bridge_tx_mailbox_heap, rmem);
    }
}

static int32_t app_core_bridge_tx_mailbox_get(APP_CORE_BRIDGE_TX_MAILBOX_PARAM_T** msg_p)
{
    osEvent evt;

    evt = osMailGet(app_core_bridge_tx_mailbox_id, osWaitForever);
    if (evt.status == osEventMail)
    {
        *msg_p = (APP_CORE_BRIDGE_TX_MAILBOX_PARAM_T *)evt.value.p;
        return 0;
    }
    return -1;
}

static int32_t app_core_bridge_tx_mailbox_raw(APP_CORE_BRIDGE_TX_MAILBOX_PARAM_T** msg_p)
{
    osEvent evt;
    evt = osMailGet(app_core_bridge_tx_mailbox_id, 0);
    if (evt.status == osEventMail)
    {
        *msg_p = (APP_CORE_BRIDGE_TX_MAILBOX_PARAM_T *)evt.value.p;
        return 0;
    }
    return -1;
}

static int32_t app_core_bridge_tx_mailbox_free(APP_CORE_BRIDGE_TX_MAILBOX_PARAM_T* msg_p)
{
    osStatus status;
    app_core_bridge_tx_mailbox_heap_free((void *)msg_p->cmd_buffer);
    status = osMailFree(app_core_bridge_tx_mailbox_id, msg_p);
    return (int32_t)status;
}

static int32_t app_core_bridge_tx_mailbox_put(APP_CORE_BRIDGE_TX_MAILBOX_PARAM_T* msg_src)
{
    APP_CORE_BRIDGE_TX_MAILBOX_PARAM_T *msg_p = NULL;
    osStatus status;

    msg_p = (APP_CORE_BRIDGE_TX_MAILBOX_PARAM_T*)osMailAlloc(app_core_bridge_tx_mailbox_id, 0);

    if (!msg_p){

        TRACE_IMM(0, "core bridge mailbox alloc error dump");
        for (uint8_t i=0; i<APP_CORE_BRIDGE_TX_MAILBOX_MAX; i++){
            app_core_bridge_tx_mailbox_raw(&msg_p);
            app_core_bridge_task_cmd_instance_t* cmdEntry;
            cmdEntry = app_core_bridge_get_task_cmd_entry(msg_p->cmdCode);

            TRACE(3, "ctrl_mailbox:DUMP: cmdcode=%s", \
                  cmdEntry->log_cmd_code_str);
        }
        TRACE_IMM(0,"core bridge mailbox Alloc error dump end");
        TRACE_IMM(0,"core bridge mailbox reAlloc New");
        msg_p = (APP_CORE_BRIDGE_TX_MAILBOX_PARAM_T*)osMailAlloc(app_core_bridge_tx_mailbox_id, 0);
    }

    ASSERT(msg_p, "core bridge mailbox Alloc error");

    msg_p->cmdCode = msg_src->cmdCode;
    msg_p->cmd_len = msg_src->cmd_len ;
    msg_p->cmd_buffer = msg_src->cmd_buffer;

    status = osMailPut(app_core_bridge_tx_mailbox_id, msg_p);


    return (int32_t)status;
}

static int32_t app_core_bridge_tx_mailbox_init(void)
{
    if (app_core_bridge_tx_mailbox_id == NULL){
        app_core_bridge_tx_mailbox_id = osMailCreate(osMailQ(app_core_bridge_tx_mailbox), NULL);
        if (NULL == app_core_bridge_tx_mailbox_id)
        {
            TRACE(1, "Failed to Create core bridge mailbox\n");
            return -1;
        }
    }
    return 0;
}

static void app_core_bridge_tx_thread_init(void)
{
    if (app_core_bridge_tx_mutex_id == NULL) {
        app_core_bridge_tx_mutex_id = osMutexCreate(osMutex(app_core_bridge_tx_mutex));
    }

    app_core_bridge_tx_mailbox_heap_init();
    app_core_bridge_tx_mailbox_init();
    if (app_core_bridge_tx_thread_id == NULL) {
        app_core_bridge_tx_thread_id =
            osThreadCreate(osThread(app_core_bridge_tx_thread), NULL);
    }
}

static app_core_bridge_task_cmd_instance_t* app_core_bridge_get_task_cmd_entry(uint16_t cmdcode)
{
    for (uint32_t index = 0;
        index < ((uint32_t)__core_bridge_task_cmd_table_end-
        (uint32_t)__core_bridge_task_cmd_table_start)/sizeof(app_core_bridge_task_cmd_instance_t);
        index++) {
        if (CORE_BRIDGE_TASK_CMD_PTR_FROM_ENTRY_INDEX(index)->cmdcode == cmdcode) {
            return CORE_BRIDGE_TASK_CMD_PTR_FROM_ENTRY_INDEX(index);
        }
    }

    ASSERT(0, "[%s] find cmdcode index failed, wrong_cmdcode=%x", __func__, cmdcode);
    return NULL;
}

static app_core_bridge_instant_cmd_instance_t* app_core_bridge_get_instant_cmd_entry(uint16_t cmdcode)
{
    for (uint32_t index = 0;
        index < ((uint32_t)__core_bridge_instant_cmd_table_end-
        (uint32_t)__core_bridge_instant_cmd_table_start)/sizeof(app_core_bridge_instant_cmd_instance_t);
        index++) {
        if (CORE_BRIDGE_INSTANT_CMD_PTR_FROM_ENTRY_INDEX(index)->cmdcode == cmdcode) {
            return CORE_BRIDGE_INSTANT_CMD_PTR_FROM_ENTRY_INDEX(index);
        }
    }

    ASSERT(0, "[%s] find cmdcode index failed, wrong_cmdcode=%x", __func__, cmdcode);
    return NULL;
}

void app_core_bridge_tx_thread(const void *arg)
{
    APP_CORE_BRIDGE_TX_MAILBOX_PARAM_T *msg_p = NULL;

    while(1)
    {
        app_core_bridge_tx_mailbox_get(&msg_p);

        if (CORE_BRIDGE_CMD_GROUP_TASK == CORE_BRIDGE_CMD_GROUP(msg_p->cmdCode))
        {
            app_core_bridge_task_cmd_instance_t* pTaskCmdInstance =
                app_core_bridge_get_task_cmd_entry(msg_p->cmdCode);

            pTaskCmdInstance->core_bridge_cmd_transmit_handler(
                msg_p->cmd_buffer, msg_p->cmd_len);
        }
        else if (CORE_BRIDGE_CMD_GROUP_INSTANT == CORE_BRIDGE_CMD_GROUP(msg_p->cmdCode))
        {
            app_core_bridge_instant_cmd_instance_t* pInstantCmdInstance =
                app_core_bridge_get_instant_cmd_entry(msg_p->cmdCode);

            pInstantCmdInstance->core_bridge_cmd_transmit_handler(
                msg_p->cmd_buffer, msg_p->cmd_len);
        }
        else
        {
            ASSERT(false, "Wrong core bridge cmd code 0x%x", msg_p->cmdCode);
        }

        app_core_bridge_tx_mailbox_free(msg_p);
    }
}

int32_t app_core_bridge_send_cmd(uint16_t cmd_code, uint8_t *p_buff, uint16_t length)
{
    if (!app_core_bridge_inited)
    {
        return 0;
    }

    APP_CORE_BRIDGE_TX_MAILBOX_PARAM_T msg = {0,};
    int nRet = 0;
    ASSERT(length <= APP_CORE_BRIDGE_MAX_XFER_DATA_SIZE,
        "%s p_buff overflow %d", __func__, length);

    msg.cmdCode = cmd_code;
    msg.cmd_len = length;

    if (length > 0)
    {
        msg.cmd_buffer = (uint8_t *)app_core_bridge_tx_mailbox_heap_malloc(msg.cmd_len);

        memcpy(msg.cmd_buffer, p_buff, msg.cmd_len);

    }
    else
    {
        msg.cmd_buffer = NULL;
    }

    osMutexWait(app_core_bridge_tx_mutex_id, osWaitForever);
    nRet = app_core_bridge_tx_mailbox_put(&msg);
    osMutexRelease(app_core_bridge_tx_mutex_id);

    return nRet;
}

int32_t app_core_bridge_send_rsp(uint16_t rsp_code, uint8_t *p_buff, uint16_t length)
{
    if (!app_core_bridge_inited)
    {
        return 0;
    }

    APP_CORE_BRIDGE_TX_MAILBOX_PARAM_T msg = {0,};
    app_core_bridge_task_cmd_instance_t* cmdEntry;

    ASSERT(length + sizeof(rsp_code) <= APP_CORE_BRIDGE_MAX_XFER_DATA_SIZE, \
           "%s p_buff overflow: %u", \
           __func__, \
           length);

    msg.cmd_len = length + sizeof(rsp_code);
    msg.cmd_buffer = (uint8_t *)app_core_bridge_tx_mailbox_heap_malloc(msg.cmd_len);
    *(uint16_t *)msg.cmd_buffer = rsp_code;
    memcpy(msg.cmd_buffer + sizeof(rsp_code), p_buff, length);

    msg.cmdCode = MCU_SENSOR_HUB_TASK_CMD_RSP;

    cmdEntry = app_core_bridge_get_task_cmd_entry(msg.cmdCode);
    cmdEntry->core_bridge_cmd_transmit_handler(msg.cmd_buffer, msg.cmd_len);
    app_core_bridge_tx_mailbox_heap_free(msg.cmd_buffer);

    return 0;
}

static void app_core_bridge_send_cmd_rsp_handler(uint8_t *p_buff, uint16_t length)
{
    app_core_bridge_task_cmd_instance_t* cmdEntry;
    app_core_bridge_data_packet_t cmddata;

    ASSERT(length <= APP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE,
        "%s: length(%d) out of range(%d)", __func__,
        length, APP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE);

    cmddata.cmdcode = MCU_SENSOR_HUB_TASK_CMD_RSP;
    cmddata.cmdseq = g_sensor_hub_cmdseq++;
    memcpy(cmddata.content, p_buff, length);
    sensor_hub_send((const uint8_t *)&cmddata, length+sizeof(cmddata.cmdcode) + sizeof(cmddata.cmdseq));

    cmdEntry = app_core_bridge_get_task_cmd_entry(*(uint16_t *)p_buff);

    TRACE(2,"core bridge:-------TRS RSP to cmdcode=%s seq = %d------->", \
          cmdEntry->log_cmd_code_str,cmddata.cmdseq);
#ifdef SENSOR_HUB_CORE_BRIDGE_RAW_LOG_DUMP_EN
    DUMP8("%02x ", (uint8_t*)&cmddata, length + sizeof(cmddata.cmdseq) + sizeof(cmddata.cmdcode));
#endif
    osSemaphoreWait(app_core_bridge_wait_tx_done_id, osWaitForever);
}

static void app_core_bridge_cmd_rsp_handler(uint8_t *p_buff, uint16_t length)
{
    //real cmd code responsed
    uint16_t rspcode = ((p_buff[1] << 8) | p_buff[0]);
    app_core_bridge_task_cmd_instance_t* pInstance =
                app_core_bridge_get_task_cmd_entry(rspcode);

     TRACE(2,"core bridge:<-----------RCV_RSP: cmdcode=%s-------", \
          pInstance->log_cmd_code_str);

    uint32_t timeout_ms = pInstance->wait_rsp_timeout_ms;
    if (pInstance->app_core_bridge_rsp_handle)
    {
        pInstance->app_core_bridge_rsp_handle(&p_buff[2],
            length - 2);

        if (timeout_ms > 0)
        {
            osSemaphoreRelease(app_core_bridge_wait_cmd_rsp_id);
        }
    }
    else
    {
        TRACE(0,"ibrt_ui_log:rsp no handler");
    }
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_SENSOR_HUB_TASK_CMD_RSP,
                                "core bridge task cmd rsp",
                                app_core_bridge_send_cmd_rsp_handler,
                                app_core_bridge_cmd_rsp_handler,
                                0,
                                NULL,
                                NULL,
                                NULL);


#define APP_CORE_BRIDGE_QUEUE_DATA_LEN_BYTES    2

static void app_core_bridge_queue_init(CQueue* ptrQueue, uint8_t* ptrBuf, uint32_t bufLen)
{
    uint32_t lock = int_lock();
    memset(ptrBuf, 0, bufLen);
    InitCQueue(ptrQueue, bufLen, (CQItemType *)ptrBuf);
    int_unlock(lock);
}

static int32_t app_core_bridge_queue_push(CQueue* ptrQueue, const void* ptrData, uint32_t length)
{
    int32_t nRet = -1;
    uint32_t lock = int_lock();
    if (length > 0)
    {
        uint16_t dataLen = (uint16_t)length;
        int queueAvailableLen = AvailableOfCQueue(ptrQueue);
        if ((dataLen+APP_CORE_BRIDGE_QUEUE_DATA_LEN_BYTES) <= queueAvailableLen)
        {
            EnCQueue(ptrQueue, (CQItemType *)&dataLen, APP_CORE_BRIDGE_QUEUE_DATA_LEN_BYTES);
            EnCQueue(ptrQueue, (CQItemType *)ptrData, length);
            nRet = 0;
        }
    }
    int_unlock(lock);
    osSignalSet(app_core_bridge_rx_thread_id, APP_CORE_BRIDGE_RX_THREAD_SIGNAL_DATA_RECEIVED);

    return nRet;
}

static uint16_t app_core_bridge_queue_get_next_entry_length(CQueue* ptrQueue)
{
    uint8_t *e1 = NULL, *e2 = NULL;
    uint32_t len1 = 0, len2 = 0;
    uint16_t length = 0;
    uint8_t* ptr;

    uint32_t lock = int_lock();

    ptr = (uint8_t *)&length;
    // get the length of the fake message
    PeekCQueue(ptrQueue, APP_CORE_BRIDGE_QUEUE_DATA_LEN_BYTES, &e1, &len1, &e2, &len2);

    memcpy(ptr,e1,len1);
    memcpy(ptr+len1,e2,len2);

    int_unlock(lock);

    return length;
}

static void app_core_bridge_queue_pop(CQueue* ptrQueue, uint8_t *buff, uint32_t len)
{
    uint8_t *e1 = NULL, *e2 = NULL;
    uint32_t len1 = 0, len2 = 0;

    uint32_t lock = int_lock();
    // overcome the two bytes of msg length
    DeCQueue(ptrQueue, 0, APP_CORE_BRIDGE_QUEUE_DATA_LEN_BYTES);

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

static int32_t app_core_bridge_get_queue_length(CQueue *ptrQueue)
{
    int32_t nRet = 0;

    uint32_t lock = int_lock();
    nRet = LengthOfCQueue(ptrQueue);
    int_unlock(lock);

    return nRet;
}

void app_core_bridge_free_tx_mailbox(void)
{
    APP_CORE_BRIDGE_TX_MAILBOX_PARAM_T *msg_p = NULL;

    while(1)
    {
        osEvent evt;

        evt = osMailGet(app_core_bridge_tx_mailbox_id, 5);
        if (evt.status == osEventMail)
        {
            msg_p = (APP_CORE_BRIDGE_TX_MAILBOX_PARAM_T *)evt.value.p;
            app_core_bridge_tx_mailbox_free(msg_p);
        }
        else
        {
            break;
        }
    }
}

void app_core_bridge_init(void)
{
    if (app_core_bridge_inited) {
        return;
    }
    ASSERT(APP_CORE_BRIDGE_MAX_XFER_DATA_SIZE \
        > (APP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE+4)/APP_CORE_BRIDGE_TX_MAILBOX_MAX/2,
        "core bridge init: xfer data size %d too small, should larger than %d",
        APP_CORE_BRIDGE_MAX_XFER_DATA_SIZE,
        (APP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE+4)/APP_CORE_BRIDGE_TX_MAILBOX_MAX/2);

    ASSERT(APP_CORE_BRIDGE_RX_BUFF_SIZE > (APP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE+4) * 2,
        "core bridge init: rx buff size %d too small, should large than %d",
        APP_CORE_BRIDGE_RX_BUFF_SIZE, (APP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE+4) * 2);

    ASSERT(APP_CORE_BRIDGE_TX_THREAD_STACK_SIZE > (APP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE+4) * 2,
        "core bridge tx thread: stack size %d too small, should larger than %d",
        APP_CORE_BRIDGE_TX_THREAD_STACK_SIZE,(APP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE+4) * 2);

    ASSERT(APP_CORE_BRIDGE_RX_THREAD_STACK_SIZE > (APP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE+4) * 2,
        "core bridge rx thread: stack size %d too small, should larger than %d",
        APP_CORE_BRIDGE_RX_THREAD_STACK_SIZE,(APP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE+4) * 2);

    ASSERT(APP_CORE_BRIDGE_RX_THREAD_STACK_SIZE > APP_CORE_BRIDGE_RX_THREAD_TMP_BUF_SIZE * 2,
        "core bridge rx thread: stack size %d too small, should larger than tmp size %d",
        APP_CORE_BRIDGE_RX_THREAD_STACK_SIZE, APP_CORE_BRIDGE_RX_THREAD_TMP_BUF_SIZE * 2);

    memset(&app_core_bridge_receive_queue, 0, sizeof(app_core_bridge_receive_queue));

    app_core_bridge_queue_init(&app_core_bridge_receive_queue, \
                            app_core_bridge_rx_buff, \
                            sizeof(app_core_bridge_rx_buff));
    g_sensor_hub_cmdseq = 0;

    app_core_bridge_wait_tx_done_id =
        osSemaphoreCreate(osSemaphore(app_core_bridge_wait_tx_done), 0);

    app_core_bridge_wait_cmd_rsp_id =
        osSemaphoreCreate(osSemaphore(app_core_bridge_wait_cmd_rsp), 0);

    app_core_bridge_tx_thread_init();

    if (app_core_bridge_rx_thread_id == NULL) {
        app_core_bridge_rx_thread_id =
            osThreadCreate(osThread(app_core_bridge_rx_thread), NULL);
    }

    app_core_bridge_inited = true;
}

