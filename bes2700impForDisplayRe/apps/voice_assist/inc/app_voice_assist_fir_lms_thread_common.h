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
#ifndef __ANC_FIR_LMS_THREAD_COMMON_H__
#define __ANC_FIR_LMS_THREAD_COMMON_H__

// #include "stdbool.h"
#include "kfifo.h"
#include "event_detection.h"
#include "anc_assist_mic.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    struct kfifo ff_mic_fifo[MAX_FF_CHANNEL_NUM];
    uint8_t *ff_mic_fifo_mem[MAX_FF_CHANNEL_NUM];
    struct kfifo fb_mic_fifo[MAX_FB_CHANNEL_NUM];
    uint8_t *fb_mic_fifo_mem[MAX_FB_CHANNEL_NUM];
    struct kfifo talk_mic_fifo[MAX_TALK_CHANNEL_NUM];
    uint8_t *talk_mic_fifo_mem[MAX_TALK_CHANNEL_NUM];
    struct kfifo ref_fifo[MAX_REF_CHANNEL_NUM];
    uint8_t *ref_fifo_mem[MAX_REF_CHANNEL_NUM];
#ifdef ANC_ASSIST_VPU
    struct kfifo vpu_fifo;
    uint8_t *vpu_fifo_mem;
#endif

    struct kfifo event_control_fifo;
    uint8_t *event_control_fifo_mem;
} fir_lms_fifo_t;

#define FIR_LMS_FREAM_LEN            (240)
#define FIR_LMS_QUEUE_FRAME_NUM      (2)
#define FIR_LMS_QUEUE_BUF_LEN        (256 * FIR_LMS_QUEUE_FRAME_NUM)// 2^N, a channle data 240 
#define FIR_LMS_EVC_QUEUE_BUF_LEN    (32 * FIR_LMS_QUEUE_FRAME_NUM) //2^N, sizeof(EventController) = 20;

void fir_lms_fifo_init(fir_lms_fifo_t *fifo, float **mic_fifo_buf, uint8_t *evc_fifo_buf);

bool fir_lms_fifo_data_is_ready(fir_lms_fifo_t *fifo);

uint32_t fir_lms_fifo_put_data(fir_lms_fifo_t *fifo,process_frame_data_t *data_buf, uint32_t mic_frame_len, void *evc_buf, uint32_t evc_buf_size);

void fir_lms_fifo_get_data(fir_lms_fifo_t *fifo, process_frame_data_t *data_buf, uint32_t mic_frame_len, void *evc_buf, uint32_t evc_buf_size);

void fir_lms_fifo_data_clear(fir_lms_fifo_t *fifo, process_frame_data_t *data_buf, uint32_t mic_frame_len, void *evc_buf, uint32_t evc_buf_size);

void fir_lms_fifo_process_data_init(process_frame_data_t *process_data, void *work_buf, int32_t frame_len);
#ifdef __cplusplus
    }
#endif

#endif//__ANC_FIR_LMS_THREAD_COMMON_H__
