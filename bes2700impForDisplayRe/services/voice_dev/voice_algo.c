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
#include <stdbool.h>
#include "hal_trace.h"
#include "app_audio.h"
#include "voice_dev.h"
#include "voice_algo.h"
#include "voice_algo_cp.h"
#include "speech_cfg.h"
#include "voice_tx_aec.h"
#include "voice_noise_est.h"
#include "voice_dev_defs.h"

static uint32_t g_frame_len = FRAME_LEN;

static uint32_t  g_algos_opened = 0;
static uint32_t  g_algos_started[VOICE_DEV_ALGO_INDEX_END];

static uint32_t g_ext_buf_used_size = 0;

void *voice_algo_get_ext_buf(int size)
{
    uint8_t *pBuf = NULL;

    if (size % 4) {
        size = size + (4 - size % 4);
    }

    app_audio_mempool_get_buff(&pBuf, size);

    return (void *)pBuf;
}

void voice_algo_ext_buf_usage_start(void)
{
    g_ext_buf_used_size = 0;
}

void voice_algo_ext_buf_usage_end(const char *name)
{
    TRACE(3, "[%s] %s RAM usage: %d bytes", __func__, name, g_ext_buf_used_size);
}

static void algo_open_impl(uint32_t index)
{
    TRACE(2, "[%s] index: %d", __func__, index);

    switch(index) {
        case VOICE_DEV_ALGO_INDEX_DC_FILTER:

            break; 

        case VOICE_DEV_ALGO_INDEX_PRE_GAIN:

            break; 

        case VOICE_DEV_ALGO_INDEX_AEC:
            voice_tx_aec_open(g_frame_len, SAMPLE_BITS);
            break; 

        case VOICE_DEV_ALGO_INDEX_NOISE_EST:
            voice_noise_est_open(g_frame_len, SAMPLE_BITS);
            break; 

        case VOICE_DEV_ALGO_INDEX_POST_GAIN:

            break; 

        default:
            TRACE(2, "[%s] index(%d) is invalid", __func__, index);
    }
}

static void algo_close_impl(uint32_t index)
{
    TRACE(2, "[%s] index: %d", __func__, index);

    switch(index) {
        case VOICE_DEV_ALGO_INDEX_DC_FILTER:

            break; 

        case VOICE_DEV_ALGO_INDEX_PRE_GAIN:

            break; 

        case VOICE_DEV_ALGO_INDEX_AEC:
            voice_tx_aec_close();
            break; 

        case VOICE_DEV_ALGO_INDEX_NOISE_EST:
            voice_noise_est_close();
            break; 

        case VOICE_DEV_ALGO_INDEX_POST_GAIN:

            break; 

        default:
            TRACE(2, "[%s] index(%d) is invalid", __func__, index);
    }
}

static void algo_process_impl(uint32_t index, uint8_t *buf_ptr[], uint32_t frame_len)
{
    // TRACE(2, "[%s] index: %d", __func__, index);

    int16_t *mic0 = (int16_t *)buf_ptr[VOICE_DEV_BUF_INDEX_MIC0];
    int16_t *ref0 = (int16_t *)buf_ptr[VOICE_DEV_BUF_INDEX_REF0];
    float *noise_est = (float *)buf_ptr[VOICE_DEV_BUF_INDEX_NOISE_EST];

    switch(index) {
        case VOICE_DEV_ALGO_INDEX_DC_FILTER:

            break; 

        case VOICE_DEV_ALGO_INDEX_PRE_GAIN:
            for (uint32_t i=0; i<frame_len; i++) {
                mic0[i] = mic0[i] * 2;
            }

            break; 

        case VOICE_DEV_ALGO_INDEX_AEC:
            voice_tx_aec_process(mic0, ref0, frame_len, mic0);
            break; 

        case VOICE_DEV_ALGO_INDEX_NOISE_EST:
            voice_noise_est_process(mic0, frame_len, noise_est);
            break; 

        case VOICE_DEV_ALGO_INDEX_POST_GAIN:

            break; 

        default:
            TRACE(2, "[%s] index(%d) is invalid", __func__, index);
    }
}

//--------------------API function--------------------
int32_t voice_algo_reset(void)
{
    g_frame_len = FRAME_LEN;
    g_algos_opened = 0;

    for (uint32_t index = 0; index < VOICE_DEV_ALGO_INDEX_END; index++) {
        g_algos_started[index] = 0;
    }

    return 0;
}

int32_t voice_algo_init(void)
{
    voice_algo_reset();

    return 0;
}

int32_t voice_algo_open(uint32_t algos)
{
    g_algos_opened = algos;

    TRACE(2, "[%s] algos = %x", __func__, algos);

#if defined(VOICE_ALGO_CP_ACCEL)
    voice_algo_cp_open(g_frame_len, 1, 1);
#endif

    for (uint32_t index = 0; index < VOICE_DEV_ALGO_INDEX_END; index++) {
        if (algos & 0x01) {
            algo_open_impl(index);
        }

        algos >>= 1;

        if (!algos) {
            break;
        } 
    }

    return 0;
}

int32_t voice_algo_close(void)
{
    uint32_t algos = g_algos_opened;

    TRACE(2, "[%s] algos = %x", __func__, algos);

    for (uint32_t index = 0; index < VOICE_DEV_ALGO_INDEX_END; index++) {
        if (algos & 0x01) {
            algo_close_impl(index);
        }

        algos >>= 1;

        if (!algos) {
            break;
        } 
    }
    
    g_algos_opened = 0;

    voice_algo_reset();

#if defined(VOICE_ALGO_CP_ACCEL)
    voice_algo_cp_close();
#endif

    return 0;
}

int32_t voice_algo_start(uint32_t algos)
{
    TRACE(2, "[%s] algos = %x", __func__, algos);

    for (uint32_t index = 0; index < VOICE_DEV_ALGO_INDEX_END; index++) {
        if (algos & 0x01) {
            g_algos_started[index]++;
        }

        algos >>= 1;

        if (!algos) {
            break;
        } 
    }

    return 0;
}

int32_t voice_algo_stop(uint32_t algos)
{
    TRACE(2, "[%s] algos = %x", __func__, algos);

    for (uint32_t index = 0; index < VOICE_DEV_ALGO_INDEX_END; index++) {
        if (algos & 0x01) {
            g_algos_started[index]--;
        }

        algos >>= 1;

        if (!algos) {
            break;
        } 
    }

    return 0;
}

int32_t voice_algo_ctl(uint32_t ctl, void *ptr)
{
    switch(ctl) {
        case VOICE_ALGO_GET_FRAME_LEN:
            (*(uint32_t*)ptr) = g_frame_len;
            break; 

        case VOICE_ALGO_SET_FRAME_LEN:
            g_frame_len = (*(uint32_t*)ptr);
            TRACE(2, "[%s] Set frame len: %d", __func__, g_frame_len);
            break; 

        default:
            TRACE(2, "[%s] ctl(%d) is not valid", __func__, ctl);
            return -1;
    }

    return 0;    
}

int32_t voice_algo_process_impl(uint8_t *buf_ptr[], uint32_t frame_len)
{
    ASSERT(frame_len == g_frame_len, "[%s] frame_len(%d) != g_frame_len", __func__, frame_len);

    // TRACE(2, "[%s] frame_len = %d", __func__, frame_len);

    for (uint32_t index = 0; index < VOICE_DEV_ALGO_INDEX_END; index++) {
        if (g_algos_started[index]) {
            algo_process_impl(index, buf_ptr, frame_len);
        }
    }

    return 0;
}

int32_t voice_algo_process(uint8_t *buf_ptr[], uint32_t frame_len)
{
#if defined(VOICE_ALGO_CP_ACCEL)
    voice_algo_cp_process(buf_ptr, frame_len);
#else
    voice_algo_process_impl(buf_ptr, frame_len);
#endif

    return 0;
}


int32_t voice_algo_dump(void)
{

    return 0;
}
