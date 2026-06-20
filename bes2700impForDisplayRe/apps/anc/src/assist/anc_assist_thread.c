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
#include "cmsis_os.h"
#include "hal_trace.h"
#include "speech_utils.h"
#include "anc_assist_mic.h"
#include "anc_assist_thread.h"

#if defined(_ASSIST_THREAD_DEBUG)
#define ASSIST_THREAD_LOG_D(str, ...)       TR_DEBUG(TR_MOD(AUD), "[ASSIST_THREAD]" str, ##__VA_ARGS__)
#else
#define ASSIST_THREAD_LOG_D(str, ...)
#endif

#if defined(ASSIST_LOW_RAM_MOD)
#define ALGO_FRAME_LEN      (60)
#else
#define ALGO_FRAME_LEN      (120)
#endif
#define ASSIST_QUEUE_FRAME_NUM      (6)
#define ASSIST_QUEUE_BUF_PCM_NUM    (ALGO_FRAME_LEN * ASSIST_QUEUE_FRAME_NUM)

static float g_in_queue_buf[MIC_INDEX_QTY][ASSIST_QUEUE_BUF_PCM_NUM];
static float g_process_buf[MIC_INDEX_QTY][ALGO_FRAME_LEN];

static float *assist_in_queue_buf[MIC_INDEX_QTY] = {NULL};
static float *assist_process_buf[MIC_INDEX_QTY] = {NULL};

static uint32_t g_in_queue_len = 0;
static uint8_t g_ff_ch_num = MAX_FF_CHANNEL_NUM;
static uint8_t g_fb_ch_num = MAX_FB_CHANNEL_NUM;

// Thread
#if defined(VOICE_ASSIST_FF_FIR_LMS)
#define ANC_ASSIST_THREAD_STACK_SIZE    (1024 * 4)
#else
#define ANC_ASSIST_THREAD_STACK_SIZE    (1024 * 2)
#endif
static osThreadId anc_assist_thread_tid;
static void anc_assist_thread(void const *argument);
osThreadDef(anc_assist_thread, osPriorityAboveNormal, 1, ANC_ASSIST_THREAD_STACK_SIZE, "anc_assist");

// Signal
#define ANC_ASSIST_SIGNAL_WRITE     (1)

// Lock
osMutexId _anc_assist_thread_mutex_id = NULL;
osMutexDef(_anc_assist_thread_mutex);
extern int32_t _process_frame(float **ff_mic, uint8_t ff_ch_num, float **fb_mic, uint8_t fb_ch_num, float *talk_mic, float *ref, uint32_t frame_len);

static void _anc_assist_thread_create_lock(void)
{
    if (_anc_assist_thread_mutex_id == NULL) {
        _anc_assist_thread_mutex_id = osMutexCreate((osMutex(_anc_assist_thread_mutex)));
    }
}

static void _anc_assist_thread_lock(void)
{
    osMutexWait(_anc_assist_thread_mutex_id, osWaitForever);
} 

static void _anc_assist_thread_unlock(void)
{
    osMutexRelease(_anc_assist_thread_mutex_id);
}

static int32_t _assist_write_input_queue(float *buf[], uint32_t frame_len)
{
#if defined(_ASSERT_QUEUE_STATUS)
    ASSERT(g_in_queue_len + frame_len <= ASSIST_QUEUE_BUF_PCM_NUM, "[%s] queue overflow!", __func__);
#else
    if (g_in_queue_len + frame_len > ASSIST_QUEUE_BUF_PCM_NUM) {
        TRACE(3, "[%s] WARNING: queue overflow! g_in_queue_len = %d, frame_len = %d", __func__, g_in_queue_len, frame_len);
        return -1;
    }
#endif

    _anc_assist_thread_lock();
    
    for (uint32_t ch = 0; ch < MIC_INDEX_QTY; ch++) {
         speech_copy_f32((assist_in_queue_buf[ch] + g_in_queue_len), buf[ch], frame_len);
    }
    g_in_queue_len += frame_len;
    _anc_assist_thread_unlock();

    ASSIST_THREAD_LOG_D("[%s] Size: %d", __func__, g_in_queue_len);

    return 0;
} 

static int32_t _assist_read_input_queue(float *buf[], uint32_t frame_len)
{
    int32_t ret = 0;

    _anc_assist_thread_lock();
    if (g_in_queue_len >= frame_len) {
        for (uint32_t ch = 0; ch < MIC_INDEX_QTY; ch++) {
            speech_copy_f32(buf[ch], assist_in_queue_buf[ch], frame_len);
            speech_copy_f32(assist_in_queue_buf[ch], assist_in_queue_buf[ch] + frame_len, g_in_queue_len - frame_len);
        }
        g_in_queue_len -= frame_len;
    } else {
        for (uint32_t ch = 0; ch < MIC_INDEX_QTY; ch++) {
            speech_set_f32(buf[ch], 0, frame_len);
        }
        ret = -1;
    }
    _anc_assist_thread_unlock();

    ASSIST_THREAD_LOG_D("[%s] Size: %d", __func__, g_in_queue_len);

    return ret;
}

static void anc_assist_thread(void const *argument)
{
    float *ff_mic_buf[MAX_FF_CHANNEL_NUM];
    float *fb_mic_buf[MAX_FB_CHANNEL_NUM];
    float *talk_buf;
    float *ref;
    osEvent evt;
    uint32_t signals = 0;
    while (1) {
        evt = osSignalWait(0x0, osWaitForever);
        signals = evt.value.signals;
        if (evt.status == osEventSignal) {
            if (signals & (1 << ANC_ASSIST_SIGNAL_WRITE)) {
                ASSIST_THREAD_LOG_D("Thread run start ...");
                while (1) {
                    if (!_assist_read_input_queue(assist_process_buf, ALGO_FRAME_LEN)) {
                        ASSIST_THREAD_LOG_D("Process run start ...");

                        for (uint8_t i = 0; i < g_ff_ch_num; i++) {
                            ff_mic_buf[i] = assist_process_buf[i];
                        }

                        for (uint8_t i = 0; i < g_fb_ch_num; i++) {
                            fb_mic_buf[i] = assist_process_buf[i + MAX_FF_CHANNEL_NUM];
                        }

                        talk_buf = assist_process_buf[MIC_INDEX_QTY - 2];
                        ref = assist_process_buf[MIC_INDEX_QTY - 1];

                        _process_frame(ff_mic_buf, g_ff_ch_num, fb_mic_buf, g_fb_ch_num, talk_buf, ref, ALGO_FRAME_LEN);

                        ASSIST_THREAD_LOG_D("Process run end ...");
                    } else {
                        break;
                    }
                }
                ASSIST_THREAD_LOG_D("Thread run end ...");
            }
        } else {
            ASSERT(0, "[%s] ERROR: evt.status = %d", __func__, evt.status);
            continue;
        }
    }
}
 
int32_t anc_assist_thread_open()
{
    g_in_queue_len = 0;

    for(uint32_t ch = 0; ch < MIC_INDEX_QTY; ch++) {
        assist_in_queue_buf[ch] = g_in_queue_buf[ch];
        assist_process_buf[ch] = g_process_buf[ch];
    }
    _anc_assist_thread_create_lock();
    anc_assist_thread_tid = osThreadCreate(osThread(anc_assist_thread), NULL);
    osSignalSet(anc_assist_thread_tid, 0x0);

    return 0;
}

int32_t anc_assist_thread_close(void)
{
    osThreadTerminate(anc_assist_thread_tid);
    return 0;
}

int32_t anc_assist_thread_capture_process(float *in_buf[], uint32_t frame_len)
{
    ASSERT(in_buf != NULL, "[%s] in_buf = NULL", __func__);
    
    _assist_write_input_queue(in_buf, frame_len);
    osSignalSet(anc_assist_thread_tid, 0x01 << ANC_ASSIST_SIGNAL_WRITE);

    return 0;
}