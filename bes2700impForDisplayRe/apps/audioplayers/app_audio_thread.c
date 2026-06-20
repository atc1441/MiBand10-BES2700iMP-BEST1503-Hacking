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
#if !defined(USE_BASIC_THREADS)
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "app_utils.h"
#include "app_audio_thread.h"

#ifndef APP_AUDIO_THREAD_STACK_SIZE
#define APP_AUDIO_THREAD_STACK_SIZE 3072
#endif

static APP_AUDIO_MOD_HANDLER_T mod_handler[APP_AUDIO_MODULE_NUM];

static void app_audio_thread(void const *argument);
osThreadDef(app_audio_thread, osPriorityHigh, 1, APP_AUDIO_THREAD_STACK_SIZE, "app_audio_thread");

osMailQDef (app_audio_mailbox, APP_AUDIO_MAILBOX_MAX, APP_AUDIO_MESSAGE_BLOCK);
static osMailQId app_audio_mailbox = NULL;
osThreadId app_audio_thread_tid;

static int app_audio_mailbox_init(void)
{
    app_audio_mailbox = osMailCreate(osMailQ(app_audio_mailbox), NULL);
    if (app_audio_mailbox == NULL)  {
        TRACE(0,"Failed to Create app_audio_mailbox\n");
        return -1;
    }
    return 0;
}

int app_audio_mailbox_put(APP_AUDIO_MESSAGE_BLOCK* msg_src)
{
    osStatus status;

    APP_AUDIO_MESSAGE_BLOCK *msg_p = NULL;

    msg_p = (APP_AUDIO_MESSAGE_BLOCK*)osMailAlloc(app_audio_mailbox, 0);

    if (!msg_p){
        osEvent evt;
        TRACE_IMM(1,"%s osMailAlloc error dump", __func__);
        for (uint8_t i=0; i<APP_AUDIO_MAILBOX_MAX; i++){
            evt = osMailGet(app_audio_mailbox, 0);
            if (evt.status == osEventMail) {
                TRACE_IMM(5,"cnt:%d mod:%d src:%08x tim:%d id:%8x",
                            i,
                            ((APP_AUDIO_MESSAGE_BLOCK *)(evt.value.p))->mod_id,
                            ((APP_AUDIO_MESSAGE_BLOCK *)(evt.value.p))->src_thread,
                            ((APP_AUDIO_MESSAGE_BLOCK *)(evt.value.p))->system_time,
                            ((APP_AUDIO_MESSAGE_BLOCK *)(evt.value.p))->msg_body.message_id);
                TRACE_IMM(5,"ptr:%08x para:%08x/%08x/%08x/%08x",
                            ((APP_AUDIO_MESSAGE_BLOCK *)(evt.value.p))->msg_body.message_ptr,
                            ((APP_AUDIO_MESSAGE_BLOCK *)(evt.value.p))->msg_body.message_Param0,
                            ((APP_AUDIO_MESSAGE_BLOCK *)(evt.value.p))->msg_body.message_Param1,
                            ((APP_AUDIO_MESSAGE_BLOCK *)(evt.value.p))->msg_body.message_Param2,
                            (int32_t)(((APP_AUDIO_MESSAGE_BLOCK *)(evt.value.p))->msg_body.message_Param3 * 10000000));
            }else{
                TRACE_IMM(2,"cnt:%d %d", i, evt.status);
                break;
            }
        }
        TRACE_IMM(1,"%s osMailAlloc error dump end", __func__);
    }

    ASSERT(msg_p, "osMailAlloc error");
    msg_p->src_thread = (uint32_t)osThreadGetId();
    msg_p->dest_thread = (uint32_t)NULL;
    msg_p->system_time = hal_sys_timer_get();
    msg_p->mod_id = msg_src->mod_id;
    msg_p->msg_body.message_id = msg_src->msg_body.message_id;
    msg_p->msg_body.message_ptr = msg_src->msg_body.message_ptr;
    msg_p->msg_body.message_Param0 = msg_src->msg_body.message_Param0;
    msg_p->msg_body.message_Param1 = msg_src->msg_body.message_Param1;
    msg_p->msg_body.message_Param2 = msg_src->msg_body.message_Param2;
    msg_p->msg_body.message_Param3 = msg_src->msg_body.message_Param3;

    status = osMailPut(app_audio_mailbox, msg_p);
    return (int)status;
}

int app_audio_mailbox_free(APP_AUDIO_MESSAGE_BLOCK* msg_p)
{
    osStatus status;

    status = osMailFree(app_audio_mailbox, msg_p);

    return (int)status;
}

int app_audio_mailbox_get(APP_AUDIO_MESSAGE_BLOCK** msg_p)
{
    osEvent evt;
    evt = osMailGet(app_audio_mailbox, osWaitForever);
    if (evt.status == osEventMail) {
        *msg_p = (APP_AUDIO_MESSAGE_BLOCK *)evt.value.p;
        return 0;
    }
    return -1;
}

static void app_audio_thread(void const *argument)
{
    while(1){
        APP_AUDIO_MESSAGE_BLOCK *msg_p = NULL;

        if (!app_audio_mailbox_get(&msg_p)) {
            if (msg_p->mod_id < APP_AUDIO_MODULE_NUM) {
                if (mod_handler[msg_p->mod_id]) {
                    int ret = mod_handler[msg_p->mod_id](&(msg_p->msg_body));
                    if (ret)
                        TRACE(2,"%s, mod_handler[%d] ret=%d", __func__, msg_p->mod_id, ret);
                }
            }
            app_audio_mailbox_free(msg_p);
        }
    }
}

int app_audio_thread_init(void)
{
    if (app_audio_mailbox_init())
        return -1;

    app_audio_thread_tid = osThreadCreate(osThread(app_audio_thread), NULL);
    if (app_audio_thread_tid == NULL)  {
        TRACE(0,"Failed to Create app_audio_thread\n");
        return 0;
    }
    return 0;
}

int app_audio_set_threadhandle(enum APP_AUDIO_MODULE_ID_T mod_id, APP_AUDIO_MOD_HANDLER_T handler)
{
    if (mod_id>=APP_AUDIO_MODULE_NUM)
        return -1;

    mod_handler[mod_id] = handler;
    return 0;
}

void * app_audio_thread_tid_get(void)
{
    return (void *)app_audio_thread_tid;
}
#endif

