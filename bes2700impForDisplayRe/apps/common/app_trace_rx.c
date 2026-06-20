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
#include "app_trace_rx.h"
#include "hal_trace.h"
#include "stdio.h"
#include "string.h"
#include "cmsis_os.h"
#include "stdlib.h"

// Some target don't support heap_api module
#if defined(APP_TRACE_RX_ENABLE)
#define APP_TRACE_RX_HEAPx
#endif

#if defined(APP_TRACE_RX_HEAP)
#include "heap_api.h"
#else
#define APP_TRACE_RX_CMD_BUF_SIZE   128
#endif

#define APP_TRACE_RX_HEAD_SIZE   4
#define APP_TRACE_RX_NAME_SIZE   20
#if (defined(A2DP_SOURCE_TEST) || defined(HFP_AG_TEST))
#define APP_TRACE_RX_ROLE_NUM    30
#else
#define APP_TRACE_RX_ROLE_NUM    6
#endif

#define APP_TRACE_RX_BUF_SIZE    1024

#define APP_TRACE_RX_HEAD               '['
#define APP_TRACE_RX_END                ']'
#define APP_TRACE_RX_SEPARATOR          ','
#define APP_TRACE_RX_COLON_SEPARATOR    ':'
#define APP_TRACE_RX_USR_SEPARATOR      '='

typedef struct{
    char *name;
    uint32_t len;
    uint8_t  *buf;
}APP_TRACE_RX_CFG_T;

typedef struct{
#if defined(APP_TRACE_RX_HEAP)
    uint8_t *buf;
#else
    uint8_t buf[APP_TRACE_RX_CMD_BUF_SIZE];
#endif
    uint32_t len;
}APP_TRACE_RX_MSG;

typedef struct{
    char name[APP_TRACE_RX_NAME_SIZE];
    APP_TRACE_RX_CALLBACK_T callback;
}APP_TRACE_RX_LIST_T;

typedef struct{
    uint32_t list_num;
    APP_TRACE_RX_LIST_T list[APP_TRACE_RX_ROLE_NUM];
    uint8_t rx_buf[APP_TRACE_RX_BUF_SIZE + 1];
} APP_TRACE_RX_T;

#if defined(_AUTO_TEST_)
extern unsigned int auto_test_prase(unsigned char *cmd, unsigned int len);
#endif

static APP_TRACE_RX_T app_trace_rx;

#define APP_TRACE_RX_THREAD_STACK_SIZE (1024 * 2)
static osThreadId app_trace_rx_thread_tid;
static void app_trace_rx_thread(void const *argument);
osThreadDef(app_trace_rx_thread, osPriorityNormal, 1, APP_TRACE_RX_THREAD_STACK_SIZE, "app_trace_rx");

#define MAILBOX_SIZE (10)
osMailQDef(app_trace_rx_mailbox, MAILBOX_SIZE, APP_TRACE_RX_MSG);
static osMailQId app_trace_rx_mailbox = NULL;
static uint8_t app_trace_rx_mailbox_cnt = 0;

#if defined(APP_TRACE_RX_HEAP)
#define HEAP_BUF_SIZE 1024
static heap_handle_t app_trace_rx_heap;
uint8_t app_trace_rx_heap_buf[HEAP_BUF_SIZE]__attribute__((aligned(4)));

void *app_trace_rx_heap_malloc(uint32_t size)
{
    void *ptr = heap_malloc(app_trace_rx_heap,size);
    ASSERT(ptr, "%s size:%d", __func__, size);
    memset(ptr, 0, size);

    return ptr;
}

void app_trace_rx_heap_free(void *mem)
{
    heap_free(app_trace_rx_heap, mem);
}
#endif

static void app_trace_rx_mailbox_init(void)
{
    app_trace_rx_mailbox = osMailCreate(osMailQ(app_trace_rx_mailbox), NULL);
    ASSERT(app_trace_rx_mailbox, "Failed to create app_trace_rx mailbox");
    app_trace_rx_mailbox_cnt = 0;
}

static int32_t app_trace_rx_mailbox_put(APP_TRACE_RX_MSG *msg)
{
    osStatus status;
    APP_TRACE_RX_MSG *msg_new = NULL;
    msg_new = (APP_TRACE_RX_MSG *)osMailAlloc(app_trace_rx_mailbox, 0);

    if (msg_new == NULL){
        TRACE(1, "[%s]WARNING:mailbox overflow!", __func__);
        return -1;
    }

    memcpy(msg_new, msg, sizeof(APP_TRACE_RX_MSG));

    status = osMailPut(app_trace_rx_mailbox, msg_new);

    if (osOK == status) {
        app_trace_rx_mailbox_cnt++;
    } else {
        TRACE(0, "app_trace_rx: osMailPut fail, status: 0x%x", status);
    }

    return (int32_t)status;
}

static int32_t app_trace_rx_mailbox_get(APP_TRACE_RX_MSG **msg_p)
{
    osEvent evt;
    evt = osMailGet(app_trace_rx_mailbox, osWaitForever);
    if (evt.status == osEventMail) {
        *msg_p = (APP_TRACE_RX_MSG *)evt.value.p;
        return 0;
    }

    return -1;
}

static int32_t app_trace_rx_mailbox_free(APP_TRACE_RX_MSG *msg_p)
{
    osStatus status;

    status = osMailFree(app_trace_rx_mailbox, msg_p);
    if (osOK == status){
        app_trace_rx_mailbox_cnt--;
    }

    return (int32_t)status;
}

int app_trace_rx_dump_list(void)
{
    for(int i=0;i<APP_TRACE_RX_ROLE_NUM;i++)
    {
        TRACE(2,"%d: %s", i, app_trace_rx.list[i].name);
    }
    return 0;
}

int app_trace_rx_is_in_list(const char *name)
{
    for(int i=0;i<APP_TRACE_RX_ROLE_NUM;i++)
    {
        if(!strcmp(app_trace_rx.list[i].name, name))
        {
            return i;
        }
    }
    app_trace_rx_dump_list();
    // TRACE(1,"%s", app_trace_rx.list[0].name);
    // TRACE(1,"%s", name);
    // TRACE(1,"%d", strlen(app_trace_rx.list[0].name));
    // TRACE(1,"%d", strlen(name));
    // TRACE(1,"%d", strcmp(app_trace_rx.list[0].name, name));
    return -1;
}

int app_trace_rx_add_item_to_list(const char *name, APP_TRACE_RX_CALLBACK_T callback)
{
    for(int i=0;i<APP_TRACE_RX_ROLE_NUM;i++)
    {
        if(app_trace_rx.list[i].name[0] == 0)
        {
            if(strlen(name) > APP_TRACE_RX_NAME_SIZE)
            {
                TRACE(1,"[%s] cmd name is too long", __func__);
                return 1;
            }
            memcpy(app_trace_rx.list[i].name, name, strlen(name));
            app_trace_rx.list[i].callback = callback;
            app_trace_rx.list_num++;
            return 0;
        }
    }

    return 1;
}

int app_trace_rx_del_item_to_list(int id)
{
    memset(app_trace_rx.list[id].name, 0, sizeof(app_trace_rx.list[id].name));
    app_trace_rx.list[id].callback = NULL;
    app_trace_rx.list_num--;

    return 0;
}

int app_trace_rx_register(const char *name, APP_TRACE_RX_CALLBACK_T callback)
{
    TRACE(2,"[%s] Add %s", __func__, name);
    if(app_trace_rx_is_in_list(name) == -1)
    {
        app_trace_rx_add_item_to_list(name, callback);
        return 0;
    }
    else
    {
        return 0;
    }
}

int app_trace_rx_deregister(const char *name)
{
    int id = 0;

    id = app_trace_rx_is_in_list(name);

    if(id != -1)
    {
        app_trace_rx_del_item_to_list(id);
        return 0;
    }
    else
    {
        return 1;
    }
}

// [test,12,102.99]
static int app_trace_rx_parse(int8_t *buf, unsigned int buf_len, APP_TRACE_RX_CFG_T *cfg)
{
    // TRACE(1,"[%s] Start...", __func__);
    int pos = 0;
    int len = 0;

    for(; pos<APP_TRACE_RX_HEAD_SIZE; pos++)
    {
        if(buf[pos] == APP_TRACE_RX_HEAD)
        {
            buf[pos] = 0;
            break;
        }
    }

    if(pos == APP_TRACE_RX_HEAD_SIZE)
    {
        return 3;
    }

    pos++;

    cfg->name = (char *)(buf+pos);
    for(; pos<APP_TRACE_RX_NAME_SIZE+APP_TRACE_RX_HEAD_SIZE; pos++)
    {
        if (buf[pos] == APP_TRACE_RX_USR_SEPARATOR)
        {
            buf[pos] = 0;
            break;
        }
        if (buf[pos] == APP_TRACE_RX_SEPARATOR)
        {
            buf[pos] = 0;
            break;
        }
    }

    // TRACE(1,"Step1: %s", cfg->name);
    // TRACE(1,"%d", strlen(cfg->name));

    if(pos == APP_TRACE_RX_NAME_SIZE)
    {
        return 1;
    }

    pos++;

    len = 0;
    cfg->buf = (uint8_t*)(buf+pos);
    for(; pos<buf_len; pos++)
    {
        if(buf[pos] == APP_TRACE_RX_END)
        {
            buf[pos] = 0;
            break;
        }
        len++;
    }
    cfg->len = len;
    if(pos == buf_len)
    {
        return 2;
    }

    return 0;
}

void app_bt_cmd_perform_test(const char* ibrt_cmd, uint32_t cmd_len);

#if defined(CONFIG_BES_AUDIO_DUMP)
#include "hal_uart.h"
typedef struct {
    HAL_UART_IRQ_RXDMA_HANDLER_T rxdma;
    uint8_t *rx_buffer;
    uint32_t buff_len;
}NUTTX_RX_T;

NUTTX_RX_T nuttx_rx_process;
static unsigned int app_nuttx_callback(unsigned char *buf, uint32_t len)
{
    TRACE(2,"[%s] len = %d,buf:%s", __func__, len, buf);
    if (nuttx_rx_process.rxdma && nuttx_rx_process.rx_buffer)
    {
        union HAL_UART_IRQ_T status;
        status.reg = 0;
        if (len > nuttx_rx_process.buff_len-1)
            len = nuttx_rx_process.buff_len-1;
        memcpy(nuttx_rx_process.rx_buffer,buf,len);
        nuttx_rx_process.rx_buffer[len]='\n';
        nuttx_rx_process.rxdma(len+1, 0, status);
    }
    if (strncmp("assert",nuttx_rx_process.rx_buffer,6) ==0)
        ASSERT(0,"TEST");
    return 0;
}
void hal_trace_set_rx_hander(HAL_UART_IRQ_RXDMA_HANDLER_T rxdma,uint8_t *rx_buffer, uint32_t buff_len)
{
    nuttx_rx_process.rx_buffer = rx_buffer;
    nuttx_rx_process.buff_len = buff_len;
    nuttx_rx_process.rxdma = rxdma;
}
#endif

uint32_t app_trace_rx_process_impl(uint8_t *buf, uint32_t len);
uint32_t app_trace_rx_process(uint8_t *buf, uint32_t len)
{
    if (buf == NULL) {
        return 1;
    }

    // TRACE(2, "[%s] len: %d", __func__, len);
    // DUMP8("%x, ", buf, len);

    buf[len] = 0;

    if (len > 15 && (strncmp((char*)buf, "[Speech Tuning,", 15) == 0)) {
        app_trace_rx_process_impl(buf, len);
        return 0;
    }

    APP_TRACE_RX_MSG msg;
#if defined(APP_TRACE_RX_HEAP)
    msg.buf = (uint8_t *)app_trace_rx_heap_malloc(len);
    if (NULL == msg.buf) {
        TRACE(0,"alloc memory fail!\n");
        return 2;
    }
#else
    if (len > APP_TRACE_RX_CMD_BUF_SIZE) {
        return 3;
    }
#endif
    memcpy(msg.buf, buf, len);
    msg.len = len;
    app_trace_rx_mailbox_put(&msg);

    return 0;
}

uint32_t app_trace_rx_process_impl(uint8_t *buf, uint32_t len)
{
     if (buf == NULL) {
        return 1;
    }

    APP_TRACE_RX_CFG_T cfg;
    int id = 0;
    int res = 0;
    int separator_pos;

    // TRACE(2, "[%s] len: %d", __func__, len);
    // DUMP8("%x, ", buf, len);

    buf[len] = 0;

    if (buf[0] == APP_TRACE_RX_HEAD) {
        // [A,B]
        res = app_trace_rx_parse((int8_t *)buf, len, &cfg);
    } else {
        // A:B
        for (separator_pos = 0; separator_pos < len; separator_pos++) {
            if (buf[separator_pos] == APP_TRACE_RX_COLON_SEPARATOR) {
                break;
            }
        }
        buf[separator_pos] = 0;
        separator_pos++;
        cfg.name = (char *)buf;
        cfg.buf = (uint8_t *)(buf + separator_pos);
        cfg.len = len - separator_pos;
    }

    if(res)
    {
        TRACE(1,"ERROR: app_trace_rx_parse %d", res);
        return 2;
    }
    id = app_trace_rx_is_in_list(cfg.name);
    if(id < 0) {
        TRACE(1,"[%s]: invalid cmd: %s", __func__, cfg.name);
        return 3;
    }

    if(app_trace_rx.list[id].callback)
    {
        app_trace_rx.list[id].callback(cfg.buf, cfg.len);
    }
    else
    {
        TRACE(1,"%s has not callback", app_trace_rx.list[id].name);
        return 4;
    }

    return 0;
}

static void app_trace_rx_thread(void const *argument)
{
    while (1) {
        APP_TRACE_RX_MSG *msg = NULL;
        if (!app_trace_rx_mailbox_get(&msg)) {
            app_trace_rx_process_impl(msg->buf, msg->len);
#if defined(APP_TRACE_RX_HEAP)
            app_trace_rx_heap_free(msg->buf);
#endif
            app_trace_rx_mailbox_free(msg);
        }
    }
}

static unsigned int app_test_callback(unsigned char *buf, uint32_t len)
{
    TRACE(2,"[%s] len = %d", __func__, len);

    // Process string
    int num_int = 0;
    int num_float = 0.0;
    TRACE(2,"[%s] %s", __func__, buf);
    sscanf((char*)buf, "%d,%d", &num_int, &num_float);

    TRACE(3,"[%s] %d:%d", __func__, num_int, num_float);

    return 0;
}

void app_trace_rx_open(void)
{
    HAL_TRACE_RX_CALLBACK_T rx_proc;

#if defined(_AUTO_TEST_)
    rx_proc = auto_test_prase;
#else
    rx_proc = app_trace_rx_process;
#endif

#if defined(APP_TRACE_RX_HEAP)
    app_trace_rx_heap = heap_register(app_trace_rx_heap_buf, HEAP_BUF_SIZE);
#endif
    app_trace_rx_mailbox_init();
    app_trace_rx_thread_tid = osThreadCreate(osThread(app_trace_rx_thread), NULL);

    hal_trace_rx_open(app_trace_rx.rx_buf, APP_TRACE_RX_BUF_SIZE, rx_proc);

    app_trace_rx_register("test", app_test_callback);
#if defined(CONFIG_BES_AUDIO_DUMP)
    app_trace_rx_register("nuttx", app_nuttx_callback);
#endif
}

