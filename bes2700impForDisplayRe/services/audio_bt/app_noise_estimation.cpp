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
#include "arm_math.h"
#include "hal_trace.h"
#include "voice_dev.h"

#define VOICE_DEV_USER_NOISE_EST            VOICE_DEV_USER2

#define FRAME_MS            (16)

// Average estimates every 3 seconds
#define NOISE_EST_AVERAGE_FRAME_COUNT        (3000 / FRAME_MS)

static int noise_est_frame_count = 0;
static float aggregate_noise_est_dB = 0;

// This value depends on record wav file
// If add hw dc filter, perhaps can reduce this value.
#define SKIP_SAMPLE_MS              (200)
#define SKIP_FRAME_CNT_THRESHOLD    (SKIP_SAMPLE_MS / FRAME_MS)
static int32_t skip_frame_cnt = 0;

static uint32_t noise_est_callback_handler(uint8_t *buf_ptr[], uint32_t frame_len);

voice_dev_cfg_t voice_dev_cfg_noise_est = {
    .adc_bits = 16,
    .frame_ms = 16,
    .non_interleave = 1,
    .algos = VOICE_DEV_ALGO_AEC | VOICE_DEV_ALGO_NOISE_EST,
    .handler = noise_est_callback_handler,
};

static int32_t g_start_status = 0;

static uint32_t noise_est_callback_handler(uint8_t *buf_ptr[], uint32_t frame_len)
{
    float *noise_est_bands_res = (float *)buf_ptr[VOICE_DEV_BUF_INDEX_NOISE_EST];

    if (skip_frame_cnt < 0) {
        // TRACE(2, "[%s] noise_est_bands_res: %d/10000", __func__, (uint32_t)(noise_est_bands_res[0] * 10000));

        aggregate_noise_est_dB += (10 * log10f(noise_est_bands_res[0]));

        if (++noise_est_frame_count >= NOISE_EST_AVERAGE_FRAME_COUNT) {
            float POSSIBLY_UNUSED noise_est_dB = (aggregate_noise_est_dB / (float)NOISE_EST_AVERAGE_FRAME_COUNT);
            // PrestoUpdateNoiseEstimate(app_bt_stream_local_volume_get(), noise_est_dB);
            noise_est_frame_count = 0;
            aggregate_noise_est_dB = 0;
        }
    } else {
        skip_frame_cnt++;
        if (skip_frame_cnt >= SKIP_FRAME_CNT_THRESHOLD) {
            skip_frame_cnt = -1;
        }
    }

    return 0;
}

int app_noise_estimation_open(void)
{
    TRACE(2, "[%s] SKIP_FRAME_CNT_THRESHOLD = %d", __func__, SKIP_FRAME_CNT_THRESHOLD);

    g_start_status = 0;
    skip_frame_cnt = 0;

    voice_dev_open(VOICE_DEV_USER_NOISE_EST, &voice_dev_cfg_noise_est);

    return 0;
}

int app_noise_estimation_close(void)
{
    TRACE(1, "[%s]  ......", __func__);

    voice_dev_close(VOICE_DEV_USER_NOISE_EST);

    g_start_status = 0;

    return 0;
}

int app_noise_estimation_start(void)
{
    TRACE(2, "[%s] g_start_status = %d", __func__, g_start_status);

    if (!g_start_status) {
        voice_dev_start(VOICE_DEV_USER_NOISE_EST);
    }

    g_start_status = 1;

    noise_est_frame_count = 0;
    aggregate_noise_est_dB = 0;

    return 0;
}

int app_noise_estimation_stop(void)
{
    TRACE(2, "[%s] g_start_status = %d", __func__, g_start_status);

    if (g_start_status) {
        voice_dev_stop(VOICE_DEV_USER_NOISE_EST);
    }

    g_start_status = 0;

    return 0;
}

int app_noise_estimation_is_runing(void)
{
    return g_start_status;
}
