/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "app_utils.h"
#include "heap_api.h"
#include "anc_ff_fir_lms.h"
#include "sensor_hub_core.h"
#include "app_voice_assist_fir_lms_thread_common.h"
#include "app_voice_assist_event_detection_thread.h"

#if defined(VOICE_ASSIST_FF_FIR_LMS_MULTI_THREAD)

#ifndef EVENT_DETECTION_THREAD_STACK_SIZE
#define EVENT_DETECTION_THREAD_STACK_SIZE 1024 * 3
#endif

static void event_detection_thread(void const *argument);
osThreadDef(event_detection_thread, osPriorityNormal, 1, EVENT_DETECTION_THREAD_STACK_SIZE, "event_detection_thread");

osMailQDef (event_detection_mailbox, EVENT_DETECTION_MAILBOX_MAX, EVENT_DETECTION_MESSAGE_T);
static osMailQId event_detection_mailbox = NULL;
osThreadId event_detection_thread_tid = NULL;

static void event_detection_process_handler(void);
static void event_detection_close_handler(void);
extern int32_t voice_assist_event_detection_process(process_frame_data_t *buf, EventController* event_control);
extern void voice_assist_event_detection_close(void);
#ifdef MC_FIR_LMS_ENABLED
extern int32_t voice_assist_anc_mc_event_detection_process(process_frame_data_t *buf, EventController* event_control);
extern void voice_assist_mc_event_detection_close(void);
#endif

//fifo cache data
static fir_lms_fifo_t event_detection_fifo;
static float event_detection_mic_cache_buf[MIC_INDEX_QTY][FIR_LMS_QUEUE_BUF_LEN];
static uint8_t event_detection_evc_cache_buf[FIR_LMS_EVC_QUEUE_BUF_LEN];

//algo process data
static float event_detection_mic_using_buf[MIC_INDEX_QTY][FIR_BLOCK_SIZE];
static process_frame_data_t event_detection_process_data;
static EventController input_evc_buf;

static int event_detection_mailbox_init(void)
{
    event_detection_mailbox = osMailCreate(osMailQ(event_detection_mailbox), NULL);
    if (event_detection_mailbox == NULL)  {
        TRACE(0,"Failed to Create event_detection_mailbox\n");
        return -1;
    }
    return 0;
}

int event_detection_mailbox_put(EVENT_DETECTION_MESSAGE_T* msg_src)
{
    osStatus status;

    EVENT_DETECTION_MESSAGE_T *msg_p = NULL;

    msg_p = (EVENT_DETECTION_MESSAGE_T*)osMailAlloc(event_detection_mailbox, 0);

    if (!msg_p){
        osEvent evt;
        TRACE_IMM(1,"%s osMailAlloc error dump", __func__);
        for (uint8_t i=0; i<EVENT_DETECTION_MAILBOX_MAX; i++){
            evt = osMailGet(event_detection_mailbox, 0);
            if (evt.status == osEventMail) {
                TRACE_IMM(4, "cnt:%d param:%d/%p",
                      i,
                      ((EVENT_DETECTION_MESSAGE_T *)(evt.value.p))->cmd,
                      ((EVENT_DETECTION_MESSAGE_T *)(evt.value.p))->ctrl_buf);
            }else{
                TRACE_IMM(2,"cnt:%d %d", i, evt.status);
                break;
            }
        }
        TRACE_IMM(1,"%s osMailAlloc error dump end", __func__);
    }

    ASSERT(msg_p, "osMailAlloc error");
    msg_p->cmd = msg_src->cmd;
    msg_p->ctrl_buf = msg_src->ctrl_buf;
    msg_p->ctrl = msg_src->ctrl;
    msg_p->ctrl_buf_len = msg_src->ctrl_buf_len;

    status = osMailPut(event_detection_mailbox, msg_p);
    return (int)status;
}

int event_detection_mailbox_free(EVENT_DETECTION_MESSAGE_T* msg_p)
{
    osStatus status;

    status = osMailFree(event_detection_mailbox, msg_p);
    if (osOK != status) {
        TRACE(2, "[%s] WARNING: failed: %d", __func__, status);
    }

    return (int)status;
}

int event_detection_mailbox_get(EVENT_DETECTION_MESSAGE_T** msg_p)
{
    osEvent evt;
    evt = osMailGet(event_detection_mailbox, osWaitForever);
    if (evt.status == osEventMail) {
        *msg_p = (EVENT_DETECTION_MESSAGE_T *)evt.value.p;
        return 0;
    }
    return -1;
}

static uint32_t event_detection_mailbox_cnt(void)
{
    return osMailGetCount(event_detection_mailbox);
}

void event_detection_mailbox_clear(void)
{
    uint32_t count = event_detection_mailbox_cnt();
    EVENT_DETECTION_MESSAGE_T *msg_p = NULL;

    TRACE(0,"[%s]mailbox_cnt = %d",__func__, count);

    for(uint32_t i = 0; i < count; i++){
        if (!event_detection_mailbox_get(&msg_p)) {
            event_detection_mailbox_free(msg_p);
        }
    }

}

static void event_detection_thread(void const *argument)
{
    while(1){
        EVENT_DETECTION_MESSAGE_T *msg_p = NULL;

        if (!event_detection_mailbox_get(&msg_p)) {
            if (msg_p->cmd == EVENT_DETECTION_PROCESS) {
                event_detection_process_handler();
            }else if(msg_p->cmd == EVENT_DETECTION_CLOSE){
                event_detection_close_handler();
            }
            event_detection_mailbox_free(msg_p);
        }
    }
}

osThreadId event_detection_get_thread_id(void)
{
    return event_detection_thread_tid;
}

int event_detection_thread_init(void)
{

    if(event_detection_thread_tid != NULL){
        TRACE(0,"[%s] is inited",__func__);
        return 0;
    }//only init once

    if (event_detection_mailbox_init()){
        TRACE(0,"[%s] mailbox init failed",__func__);
        return -1;
    }

    event_detection_thread_tid = osThreadCreate(osThread(event_detection_thread), NULL);
    // osThreadSetPriority(event_detection_thread_tid, osPriorityLow);
    if (event_detection_thread_tid == NULL)  {
        TRACE(0,"Failed to Create event_detection_thread\n");
        return 0;
    }

    //fifo init
    float *mic_buf[MIC_INDEX_QTY];
    for (uint32_t i = 0; i < MIC_INDEX_QTY; i++) {
        mic_buf[i] = event_detection_mic_cache_buf[i];
    }
    fir_lms_fifo_init(&event_detection_fifo, mic_buf, event_detection_evc_cache_buf);
    TRACE(0,"[%s] event_detection_fifo %p",__func__, &event_detection_fifo);

    // process_data init
    static float *ff_mic_buf[MAX_FF_CHANNEL_NUM] = {NULL,};
    static float *fb_mic_buf[MAX_FB_CHANNEL_NUM] = {NULL,};
    static float *talk_mic_buf[MAX_TALK_CHANNEL_NUM] = {NULL,};
    static float *ref_mic_buf[MAX_REF_CHANNEL_NUM] = {NULL,};
    float *vpu_buf = NULL;

    uint32_t j = 0;
    for (uint32_t i = 0; i < MAX_FF_CHANNEL_NUM; i++){
        ff_mic_buf[i] = event_detection_mic_using_buf[j];
        j++;
    }
    for (uint32_t i = 0; i < MAX_FB_CHANNEL_NUM; i++){
        fb_mic_buf[i] = event_detection_mic_using_buf[j];
        j++;
    }
    for (uint32_t i = 0; i < MAX_TALK_CHANNEL_NUM; i++){
        talk_mic_buf[i] = event_detection_mic_using_buf[j];
        j++;
    }
    for (uint32_t i = 0; i < MAX_REF_CHANNEL_NUM; i++){
        ref_mic_buf[i] = event_detection_mic_using_buf[j];
        j++;
    }
#if defined(ANC_ASSIST_VPU)
    vpu_buf = event_detection_mic_using_buf[j];
#endif
    void *mic_buf2[5] = {ff_mic_buf, fb_mic_buf, talk_mic_buf, ref_mic_buf, vpu_buf};
    fir_lms_fifo_process_data_init(&event_detection_process_data, mic_buf2, FIR_BLOCK_SIZE);

    return 0;
}

void app_voice_assist_event_detection_thread_process(process_frame_data_t *data_buf, EventController* event_control)
{
    static uint32_t fifo_total_cnt = 0;
    static uint32_t fifo_overflow_cnt = 0;

    uint32_t fifo_res = fir_lms_fifo_put_data(&event_detection_fifo, data_buf, FIR_BLOCK_SIZE, event_control, sizeof(EventController));
    if (fifo_res == -1){
        fifo_overflow_cnt++;
    }
    fifo_total_cnt++;
    if(fifo_total_cnt % 1000 == 0){
        TRACE(0,"[event detection fifo] total_cnt = %d, overflow_cnt = %d", fifo_total_cnt, fifo_overflow_cnt);
    }

    if (fifo_res == 0){
        EVENT_DETECTION_MESSAGE_T msg_src;
        msg_src.cmd = EVENT_DETECTION_PROCESS;
        event_detection_mailbox_put(&msg_src);
    }
}

void app_voice_assist_event_detection_thread_close(void)
{
    EVENT_DETECTION_MESSAGE_T msg_src;
    msg_src.cmd = EVENT_DETECTION_CLOSE;
    event_detection_mailbox_put(&msg_src);
}

static void event_detection_process_handler(void)
{
    fir_lms_fifo_get_data(&event_detection_fifo, &event_detection_process_data, FIR_BLOCK_SIZE, &input_evc_buf, sizeof(EventController));

    voice_assist_event_detection_process(&event_detection_process_data, &input_evc_buf);

}

static void event_detection_close_handler(void)
{
    voice_assist_event_detection_close();

    event_detection_mailbox_clear();

    fir_lms_fifo_data_clear(&event_detection_fifo, &event_detection_process_data, FIR_BLOCK_SIZE, &input_evc_buf, sizeof(EventController));
}
#endif

#define HEAP_BUFF_SIZE HEAP_BUFF_EVENT_SIZE
#include "ext_heap.h"

int32_t ext_heap_init_event(void *buff)
{
    return ext_heap_init(buff);
}

int32_t ext_heap_deinit_event(void)
{
    return ext_heap_deinit();
}

uint32_t ext_heap_event_get_used_buff_size(void)
{
    return ext_heap_get_used_buff_size();
}

static void *ext_malloc_event(uint32_t size)
{
    return ext_malloc(size);
}

static void *ext_calloc_event(uint32_t nitems, uint32_t size)
{
    return ext_calloc(nitems, size);
}

static void ext_free_event(void *mem_ptr)
{
    ext_free(mem_ptr);
}

custom_allocator ext_allocator_event = {
    .malloc = ext_malloc_event,
    .calloc = ext_calloc_event,
    .free = ext_free_event,
};