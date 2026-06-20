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
#if defined(VOICE_ASSIST_FF_FIR_LMS_MULTI_THREAD)

#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "app_utils.h"
#include "heap_api.h"
#include "anc_ff_fir_lms.h"
#include "sensor_hub_core.h"
#include "app_voice_assist_fir_lms_thread_common.h"
#include "app_voice_assist_fir_lms_thread.h"

#ifndef ANC_FIR_LMS_THREAD_STACK_SIZE
#define ANC_FIR_LMS_THREAD_STACK_SIZE 1024 * 2
#endif

static void fir_lms_thread(void const *argument);
osThreadDef(fir_lms_thread, osPriorityAboveNormal, 1, ANC_FIR_LMS_THREAD_STACK_SIZE, "fir_lms_thread");

osMailQDef (fir_lms_mailbox, ANC_FIR_LMS_MAILBOX_MAX, ANC_FIR_LMS_MESSAGE_T);
static osMailQId fir_lms_mailbox = NULL;
osThreadId fir_lms_thread_tid = NULL;

static void fir_lms_process_handler(void);
static void fir_lms_close_handler(void);
extern int32_t voice_assist_anc_ff_fir_lms_process(process_frame_data_t *buf, EventController* event_control);
extern void voice_assist_ff_fir_lms_close(void);
#ifdef MC_FIR_LMS_ENABLED
extern int32_t voice_assist_anc_mc_fir_lms_process(float **mic_data, EventController* event_control);
extern void voice_assist_mc_fir_lms_close(void);
#endif

//fifo cache data
static fir_lms_fifo_t fir_lms_fifo;
static float fir_lms_mic_cache_buf[MIC_INDEX_QTY][FIR_LMS_QUEUE_BUF_LEN];
static uint8_t fir_lms_evc_cache_buf[FIR_LMS_EVC_QUEUE_BUF_LEN];

//algo process data
static float fir_lms_mic_using_buf[MIC_INDEX_QTY][FIR_BLOCK_SIZE];
static process_frame_data_t fir_lms_process_data;
static EventController input_evc_buf;

static int fir_lms_mailbox_init(void)
{
    fir_lms_mailbox = osMailCreate(osMailQ(fir_lms_mailbox), NULL);
    if (fir_lms_mailbox == NULL)  {
        TRACE(0,"Failed to Create fir_lms_mailbox\n");
        return -1;
    }
    return 0;
}

int fir_lms_mailbox_put(ANC_FIR_LMS_MESSAGE_T* msg_src)
{
    osStatus status;

    ANC_FIR_LMS_MESSAGE_T *msg_p = NULL;

    msg_p = (ANC_FIR_LMS_MESSAGE_T*)osMailAlloc(fir_lms_mailbox, 0);

    if (!msg_p){
        osEvent evt;
        TRACE_IMM(1,"%s osMailAlloc error dump", __func__);
        for (uint8_t i=0; i<ANC_FIR_LMS_MAILBOX_MAX; i++){
            evt = osMailGet(fir_lms_mailbox, 0);
            if (evt.status == osEventMail) {
                TRACE_IMM(4, "cnt:%d param:%d/%p",
                      i,
                      ((ANC_FIR_LMS_MESSAGE_T *)(evt.value.p))->cmd,
                      ((ANC_FIR_LMS_MESSAGE_T *)(evt.value.p))->ctrl_buf);
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

    status = osMailPut(fir_lms_mailbox, msg_p);
    return (int)status;
}

FIR_LMS_TEXT_LOCATION
int fir_lms_mailbox_free(ANC_FIR_LMS_MESSAGE_T* msg_p)
{
    osStatus status;

    status = osMailFree(fir_lms_mailbox, msg_p);
    if (osOK != status) {
        TRACE(2, "[%s] WARNING: failed: %d", __func__, status);
    }

    return (int)status;
}

FIR_LMS_TEXT_LOCATION
int fir_lms_mailbox_get(ANC_FIR_LMS_MESSAGE_T** msg_p)
{
    osEvent evt;
    evt = osMailGet(fir_lms_mailbox, osWaitForever);
    if (evt.status == osEventMail) {
        *msg_p = (ANC_FIR_LMS_MESSAGE_T *)evt.value.p;
        return 0;
    }
    return -1;
}

static uint32_t fir_lms_mailbox_cnt(void)
{
    return osMailGetCount(fir_lms_mailbox);
}

void fir_lms_mailbox_clear(void)
{
    uint32_t count = fir_lms_mailbox_cnt();
    ANC_FIR_LMS_MESSAGE_T *msg_p = NULL;

    TRACE(0,"[%s]mailbox_cnt = %d",__func__, count);

    for(uint32_t i = 0; i < count; i++){
        if (!fir_lms_mailbox_get(&msg_p)) {
            fir_lms_mailbox_free(msg_p);
        }
    }

}

FIR_LMS_TEXT_LOCATION
static void fir_lms_thread(void const *argument)
{
    while(1){
        ANC_FIR_LMS_MESSAGE_T *msg_p = NULL;

        if (!fir_lms_mailbox_get(&msg_p)) {
            if (msg_p->cmd == ANC_FIR_LMS_PROCESS) {
                fir_lms_process_handler();
            }else if(msg_p->cmd == ANC_FIR_LMS_CLOSE){
                fir_lms_close_handler();
            }
            fir_lms_mailbox_free(msg_p);
        }
    }
}

osThreadId fir_lms_get_thread_id(void)
{
    return fir_lms_thread_tid;
}

int fir_lms_thread_init(void)
{
    if(fir_lms_thread_tid != NULL)//only init once
        return 0;

    if (fir_lms_mailbox_init())
        return -1;

    fir_lms_thread_tid = osThreadCreate(osThread(fir_lms_thread), NULL);
    // osThreadSetPriority(fir_lms_thread_tid, osPriorityLow);
    if (fir_lms_thread_tid == NULL)  {
        TRACE(0,"Failed to Create fir_lms_thread\n");
        return 0;
    }

    //fifo init
    float *mic_buf[MIC_INDEX_QTY];
    for (uint32_t i = 0; i < MIC_INDEX_QTY; i++) {
        mic_buf[i] = fir_lms_mic_cache_buf[i];
    }
    fir_lms_fifo_init(&fir_lms_fifo, mic_buf, fir_lms_evc_cache_buf);
    TRACE(0,"[%s] fir_lms_fifo %p",__func__, &fir_lms_fifo);

    // process_data init
    static float *ff_mic_buf[MAX_FF_CHANNEL_NUM] = {NULL,};
    static float *fb_mic_buf[MAX_FB_CHANNEL_NUM] = {NULL,};
    static float *talk_mic_buf[MAX_TALK_CHANNEL_NUM] = {NULL,};
    static float *ref_mic_buf[MAX_REF_CHANNEL_NUM] = {NULL,};
    float *vpu_buf = NULL;

    uint32_t j = 0;
    for (uint32_t i = 0; i < MAX_FF_CHANNEL_NUM; i++){
        ff_mic_buf[i] = fir_lms_mic_using_buf[j];
        j++;
    }
    for (uint32_t i = 0; i < MAX_FB_CHANNEL_NUM; i++){
        fb_mic_buf[i] = fir_lms_mic_using_buf[j];
        j++;
    }
    for (uint32_t i = 0; i < MAX_TALK_CHANNEL_NUM; i++){
        talk_mic_buf[i] = fir_lms_mic_using_buf[j];
        j++;
    }
    for (uint32_t i = 0; i < MAX_REF_CHANNEL_NUM; i++){
        ref_mic_buf[i] = fir_lms_mic_using_buf[j];
        j++;
    }
#if defined(ANC_ASSIST_VPU)
    vpu_buf = fir_lms_mic_using_buf[j];
#endif
    void *mic_buf2[5] = {ff_mic_buf, fb_mic_buf, talk_mic_buf, ref_mic_buf, vpu_buf};
    fir_lms_fifo_process_data_init(&fir_lms_process_data, mic_buf2, FIR_BLOCK_SIZE);

    return 0;
}

void app_voice_assist_fir_lms_thread_process(process_frame_data_t *data_buf, EventController* event_control)
{
    static uint32_t fifo_total_cnt = 0;
    static uint32_t fifo_overflow_cnt = 0;

    uint32_t fifo_res = fir_lms_fifo_put_data(&fir_lms_fifo, data_buf, FIR_BLOCK_SIZE, event_control, sizeof(EventController));
    if (fifo_res == -1){
        fifo_overflow_cnt++;
    }
    fifo_total_cnt++;
    if(fifo_total_cnt % 1000 == 0){
        TRACE(0,"[fir lms fifo] total_cnt = %d, overflow_cnt = %d", fifo_total_cnt, fifo_overflow_cnt);
    }

    if (fifo_res == 0){
        ANC_FIR_LMS_MESSAGE_T msg_src;
        msg_src.cmd = ANC_FIR_LMS_PROCESS;
        fir_lms_mailbox_put(&msg_src);
    }
}

void app_voice_assist_fir_lms_thread_close(void)
{
    ANC_FIR_LMS_MESSAGE_T msg_src;
    msg_src.cmd = ANC_FIR_LMS_CLOSE;
    fir_lms_mailbox_put(&msg_src);
}

static void fir_lms_process_handler(void)
{
    fir_lms_fifo_get_data(&fir_lms_fifo, &fir_lms_process_data, FIR_BLOCK_SIZE, &input_evc_buf, sizeof(EventController));

    POSSIBLY_UNUSED int32_t ff_res = -1;
    ff_res = voice_assist_anc_ff_fir_lms_process(&fir_lms_process_data, &input_evc_buf);
#ifdef MC_FIR_LMS_ENABLED
    if(ff_res != ANC_FF_FIR_LMS_RES_IN_CONVERGING){
        voice_assist_anc_mc_fir_lms_process(&fir_lms_process_data, &input_evc_buf);
    }
#endif
}

static void fir_lms_close_handler(void)
{
    voice_assist_ff_fir_lms_close();
#ifdef MC_FIR_LMS_ENABLED
    voice_assist_mc_fir_lms_close();
#endif
    fir_lms_mailbox_clear();

    fir_lms_fifo_data_clear(&fir_lms_fifo, &fir_lms_process_data, FIR_BLOCK_SIZE, &input_evc_buf, sizeof(EventController));
}

#endif
