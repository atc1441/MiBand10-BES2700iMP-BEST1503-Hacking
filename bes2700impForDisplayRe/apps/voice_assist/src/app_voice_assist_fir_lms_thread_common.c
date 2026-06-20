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
#include "app_voice_assist_fir_lms_thread_common.h"
#include "sensor_hub_core.h"
#include "hal_trace.h"

void fir_lms_fifo_init(fir_lms_fifo_t *fifo, float **work_buf, uint8_t *evc_fifo_buf)
{
    for (uint32_t i = 0; i < MAX_FF_CHANNEL_NUM; i++){
        fifo->ff_mic_fifo_mem[i] = (uint8_t *)work_buf[MIC_INDEX_FF + i];
    }
    for (uint32_t i = 0; i < MAX_FB_CHANNEL_NUM; i++){
        fifo->fb_mic_fifo_mem[i] = (uint8_t *)work_buf[MIC_INDEX_FB + i];
    }
    for (uint32_t i = 0; i < MAX_TALK_CHANNEL_NUM; i++){
        fifo->talk_mic_fifo_mem[i] = (uint8_t *)work_buf[MIC_INDEX_TALK + i];
    }
    for (uint32_t i = 0; i < MAX_REF_CHANNEL_NUM; i++){
        fifo->ref_fifo_mem[i] = (uint8_t *)work_buf[MIC_INDEX_REF + i];
    }
    fifo->event_control_fifo_mem = evc_fifo_buf;
#if defined(ANC_ASSIST_VPU)
    fifo->vpu_fifo_mem = (uint8_t *)work_buf[MIC_INDEX_VPU];
#endif


    for (uint32_t i = 0; i < MAX_FF_CHANNEL_NUM; i++){
        kfifo_init(&fifo->ff_mic_fifo[i], fifo->ff_mic_fifo_mem[i], FIR_LMS_QUEUE_BUF_LEN * sizeof(float));
    }
    for (uint32_t i = 0; i < MAX_FB_CHANNEL_NUM; i++){
        kfifo_init(&fifo->fb_mic_fifo[i], fifo->fb_mic_fifo_mem[i], FIR_LMS_QUEUE_BUF_LEN * sizeof(float));
    }
    for (uint32_t i = 0; i < MAX_TALK_CHANNEL_NUM; i++){
        kfifo_init(&fifo->talk_mic_fifo[i], fifo->talk_mic_fifo_mem[i], FIR_LMS_QUEUE_BUF_LEN * sizeof(float));
    }
    for (uint32_t i = 0; i < MAX_REF_CHANNEL_NUM; i++){
        kfifo_init(&fifo->ref_fifo[i], fifo->ref_fifo_mem[i], FIR_LMS_QUEUE_BUF_LEN * sizeof(float));
    }
    kfifo_init(&fifo->event_control_fifo, fifo->event_control_fifo_mem, FIR_LMS_EVC_QUEUE_BUF_LEN);

#if defined(ANC_ASSIST_VPU)
    kfifo_init(&fifo->vpu_fifo, fifo->vpu_fifo_mem, FIR_LMS_QUEUE_BUF_LEN * sizeof(float));
#endif
}

bool fir_lms_fifo_data_is_ready(fir_lms_fifo_t *fifo)
{
    //put event_control at the end
    if (kfifo_len(&fifo->event_control_fifo) >= sizeof(EventController)) {
        return true;
    } else {
        return false;
    }
}


uint32_t fir_lms_fifo_put_data(fir_lms_fifo_t *fifo, process_frame_data_t *data_buf, uint32_t mic_frame_len, void *evc_buf, uint32_t evc_buf_size)
{
    if (fifo == NULL) {
        TRACE(1, "[%s] fifo is NULL", __func__);
        return -2;
    }

    uint32_t len = mic_frame_len * sizeof(float);
    if (kfifo_get_free_space(&fifo->event_control_fifo) >= evc_buf_size) {

        for (int32_t i = 0; i < MAX_FF_CHANNEL_NUM; i++){
            kfifo_put(&(fifo->ff_mic_fifo[i]), (uint8_t *)data_buf->ff_mic[i], len);
        }

        for (int32_t i = 0; i < MAX_FB_CHANNEL_NUM; i++){
            kfifo_put(&(fifo->fb_mic_fifo[i]), (uint8_t *)data_buf->fb_mic[i], len);
        }

        for (int32_t i = 0; i < MAX_TALK_CHANNEL_NUM; i++){
            kfifo_put(&(fifo->talk_mic_fifo[i]), (uint8_t *)data_buf->talk_mic[i], len);
        }

        for (int32_t i = 0; i < MAX_REF_CHANNEL_NUM; i++){
            kfifo_put(&(fifo->ref_fifo[i]), (uint8_t *)data_buf->ref[i], len);
        }

        kfifo_put(&(fifo->event_control_fifo), (uint8_t *)evc_buf, evc_buf_size);

#if defined(ANC_ASSIST_VPU)
        kfifo_put(&(fifo->vpu_fifo), (uint8_t *)vpu, len);
#endif
    } else {
        TRACE(0, "[%s]fifo %p Input buffer is overflow", __FUNCTION__, fifo);
        return -1;
    }

    return 0;
}

void fir_lms_fifo_get_data(fir_lms_fifo_t *fifo, process_frame_data_t *process_data, uint32_t mic_frame_len, void *evc_buf, uint32_t evc_buf_size)
{
    if(!(kfifo_len(&fifo->event_control_fifo) >= evc_buf_size))
    {
        TRACE(0,"[%s]warning!! data is not ready",__func__);
        return;
    }

    uint32_t len = mic_frame_len * sizeof(float);

    for (int32_t i = 0; i < MAX_FF_CHANNEL_NUM; i++){
        kfifo_get(&(fifo->ff_mic_fifo[i]), (uint8_t *)process_data->ff_mic[i], len);
    }

    for (int32_t i = 0; i < MAX_FB_CHANNEL_NUM; i++){
        kfifo_get(&(fifo->fb_mic_fifo[i]), (uint8_t *)process_data->fb_mic[i], len);
    }

    for (int32_t i = 0; i < MAX_TALK_CHANNEL_NUM; i++){
        kfifo_get(&(fifo->talk_mic_fifo[i]), (uint8_t *)process_data->talk_mic[i], len);
    }

    for (int32_t i = 0; i < MAX_REF_CHANNEL_NUM; i++){
        kfifo_get(&(fifo->ref_fifo[i]), (uint8_t *)process_data->ref[i], len);
    }

    kfifo_get(&(fifo->event_control_fifo), (uint8_t *)evc_buf, evc_buf_size);

#if defined(ANC_ASSIST_VPU)
    kfifo_get(&(fifo->vpu_fifo), (uint8_t *)vpu, len);
#endif
}

void fir_lms_fifo_data_clear(fir_lms_fifo_t *fifo, process_frame_data_t *process_data, uint32_t mic_frame_len, void *evc_buf, uint32_t evc_buf_size)
{
    uint32_t len = kfifo_len(&(fifo->ff_mic_fifo[0]));
    uint32_t loop_cnt = len / mic_frame_len;

    for (uint32_t i=0; i<loop_cnt+1; i++) {
        fir_lms_fifo_get_data(fifo, process_data, mic_frame_len,evc_buf, evc_buf_size);
    }
}

void fir_lms_fifo_process_data_init(process_frame_data_t *process_data, void *mic_workbuf, int32_t frame_len)
{
    float **work_buf = (float **)mic_workbuf;

    process_data->ff_ch_num = MAX_FF_CHANNEL_NUM;
    process_data->ff_mic = (float **)work_buf[0];
    process_data->fb_ch_num = MAX_FB_CHANNEL_NUM;
    process_data->fb_mic = (float **)work_buf[1];
    process_data->talk_ch_num = MAX_TALK_CHANNEL_NUM;
    process_data->talk_mic = (float **)work_buf[2];
    process_data->ref_ch_num = MAX_REF_CHANNEL_NUM;
    process_data->ref = (float **)work_buf[3];
    process_data->frame_len = frame_len;
#if defined(ANC_ASSIST_VPU)
    process_data->vpu = (float *)work_buf[4];
#endif
}