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
/*
NOISE_EST:
sample_rate: 16k
frame_size: 16ms
MIPS: 4M
malloc RAM: 10k Byte(NOTE: FFT 4k)
*/
#include "audio_dump.h"
#include "hal_trace.h"
#include "voice_dev.h"
#include "speech_cfg.h"
#include "voice_dev_defs.h"

// #define VOICE_NOISE_EST_AUDIO_DUMP

#define CHANNEL_NUM         (1)
#define SAMPLE_RATE         (16000)

NoiseEstimatorState *noise_est_st = NULL;

/**
 * smooth_enable: Enable or disable smooth bands result. 
 * band1: freq1 ~ freq2
 * band2: freq2 ~ freq3
 * NOTE: Currently, just support band1. Optimization for MIPS
 */
const NoiseEstimatorConfig noise_est_cfg = {
    .smooth_enable = 1,
    .freq1 = 500,
    .freq2 = 1000,
    .freq3 = 5000,
};

static uint32_t g_frame_len = FRAME_LEN;

extern void *voice_algo_get_ext_buf(uint32_t size);
extern void voice_algo_ext_buf_usage_start(void);
extern void voice_algo_ext_buf_usage_end(const char *name);

int32_t voice_noise_est_open(uint32_t frame_len, uint32_t bits)
{
    TRACE(3, "[%s] frame_len = %d, bits = %d", __func__, frame_len, bits);

    ASSERT((frame_len == FRAME_LEN) || (frame_len == FRAME_LEN_MAX), "[%s] frame_len(%d) is invalid", __func__, frame_len);
    ASSERT(bits == SAMPLE_BITS, "[%s] bits(%d) != SAMPLE_BITS", __func__, bits);

    g_frame_len = frame_len;

#ifdef VOICE_NOISE_EST_AUDIO_DUMP
    audio_dump_init(g_frame_len, sizeof(short), CHANNEL_NUM);
#endif

    voice_algo_ext_buf_usage_start();
    noise_est_st = NoiseEstimator_create(SAMPLE_RATE, g_frame_len, &noise_est_cfg, voice_algo_get_ext_buf);
    voice_algo_ext_buf_usage_end("Noise Estimator");

    return 0;
}

int32_t voice_noise_est_close(void)
{
    TRACE(1, "[%s]  ......", __func__);

    NoiseEstimator_destroy(noise_est_st);

    return 0;
}

int32_t voice_noise_est_process(int16_t *pcm_buf, int32_t pcm_len, float *out_buf)
{
    ASSERT(pcm_len == g_frame_len, "[%s] pcm_len(%d) != g_frame_len(%d)", __func__, pcm_len, g_frame_len);

#ifdef VOICE_NOISE_EST_AUDIO_DUMP
    audio_dump_clear_up();
    audio_dump_add_channel_data(0, pcm_buf, pcm_len);
#endif

    NoiseEstimator_process(noise_est_st, pcm_buf, pcm_len);
    NoiseEstimator_ctl(noise_est_st, NOISE_ESTIMATOR_GET_BAND_RES, out_buf);

#ifdef VOICE_NOISE_EST_AUDIO_DUMP
    audio_dump_run();
#endif

    return pcm_len;
}