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
#include "hal_trace.h"
#include <math.h>
#include "signal_generator.h"

// DEFINE
#define _PI               3.14159265358979f

// NOTE: Support sample rate: x.xk
#define PERIOD_CNT      (10)

// max sample rate: 96k
#define PCM_BUF_LEN     (96 * sizeof(int) * PERIOD_CNT)


// global parameters
uint32_t g_type = SG_TYPE_NUM;
uint32_t g_bits = 16;
int32_t g_bits_max = 0;
uint32_t g_channel_num = 1;

static uint32_t g_pcm_index = 0;
static uint32_t g_pcm_len = 0;
static uint8_t g_pcm_buf[PCM_BUF_LEN];

static double g_gain = 0.977237f;       // -0.2dB
// TODO: Calculate gain from dB
// static double g_gain = 1.0f;       // 0dB

void signal_generator_init(SG_TYPE_T type, uint32_t sample_rate, uint32_t bits, uint32_t channel_num)
{
    uint32_t period_samples = 0;
    uint32_t period_cnt = 1;
    int16_t *g_pcm_buf_int16 = (int16_t *)g_pcm_buf;
    int32_t *g_pcm_buf_int32 = (int32_t *)g_pcm_buf;

    TRACE(5, "[%s] type = %d, sample_rate = %d, bits = %d, channel_num = %d", __func__, type, sample_rate, bits, channel_num);
    ASSERT((bits == 16) || (bits == 24) || (bits == 32), "[%s] bits(%d) is invalid", __func__, bits);

    g_type = type;
    g_bits = bits;
    g_channel_num =  channel_num;
    
    g_pcm_index = 0;

    g_bits_max = ((0x00000001 << (g_bits-1)) - 1);

    if (type == SG_TYPE_TONE_SIN_1K) {
        if (sample_rate % 1000 == 0) {
            period_samples = sample_rate / 1000;
            period_cnt = 1;
        } else if (sample_rate % 100 == 0) {
            period_samples = sample_rate / 100;
            period_cnt = 10;
        } else {
            ASSERT(0, "[%s] sample_rate(%d) is invalid for 1k tone", __func__, sample_rate);
        }
    }else if(type == SG_TYPE_TONE_SIN_500) {
        if (sample_rate % 500 == 0) {
            period_samples = sample_rate / 500;
            period_cnt = 1;
        } else if (sample_rate % 100 == 0) {
            period_samples = sample_rate / 100;
            period_cnt = 5;
        } else {
            ASSERT(0, "[%s] sample_rate(%d) is invalid for 1k tone", __func__, sample_rate);
        }
    }else if(type == SG_TYPE_TONE_SIN_1600) {
        if (sample_rate % 1600 == 0) {
            period_samples = sample_rate / 1600;
            period_cnt = 1;
        } else if (sample_rate % 100 == 0) {
            period_samples = sample_rate / 100;
            period_cnt = 16;
        } else {
            ASSERT(0, "[%s] sample_rate(%d) is invalid for 1k tone", __func__, sample_rate);
        }
    } else if (type == SG_TYPE_TONE_SIN_100) {
        if (sample_rate % 100 == 0) {
            period_samples = sample_rate / 100;
            period_cnt = 1;
        } else if (sample_rate % 10 == 0) {
            period_samples = sample_rate / 10;
            period_cnt = 10;
        } else {
            ASSERT(0, "[%s] sample_rate(%d) is invalid for 100 tone", __func__, sample_rate);
        }
    }

    ASSERT(period_samples * (bits / 8) <= PCM_BUF_LEN, "[%s] period_samples(%d) > PCM_BUF_LEN", __func__, period_samples);

    for (uint32_t i=0; i<period_samples; i++) {
        if (g_bits == 16) {
            g_pcm_buf_int16[i] = (int16_t)(sin(i * 2 * _PI * period_cnt / period_samples) * g_gain * g_bits_max);
            // TRACE(3, "[%s] %d: %x, ", __func__, i, g_pcm_buf_int16[i]);
        } else {
            g_pcm_buf_int32[i] = (int32_t)(sin(i * 2 * _PI * period_cnt  / period_samples) * g_gain * g_bits_max);
            // TRACE(3, "[%s] %d: %x, ", __func__, i, g_pcm_buf_int32[i]);
        }

    }

    g_pcm_len = period_samples;

    TRACE(3, "[%s] period_cnt = %d, g_pcm_len = %d", __func__, period_cnt, g_pcm_len);
    
}

void signal_generator_deinit(void)
{
    ;
}

void signal_generator_get_data(void *pcm_buf, uint32_t frame_len)
{
    ;
}

void signal_generator_loop_get_data(void *pcm_buf, uint32_t frame_len)
{
    int16_t *g_pcm_buf_int16 = (int16_t *)g_pcm_buf;
    int32_t *g_pcm_buf_int32 = (int32_t *)g_pcm_buf;

    if ((g_type == SG_TYPE_TONE_SIN_1K)||(g_type == SG_TYPE_TONE_SIN_500)||(g_type == SG_TYPE_TONE_SIN_1600)||(g_type == SG_TYPE_TONE_SIN_100)) {
        for (uint32_t i = 0; i < frame_len; i++) {
            for (uint32_t ch = 0; ch < g_channel_num; ch++) {
                if (g_bits == 16) {
                    ((int16_t *)pcm_buf)[g_channel_num * i + ch] = g_pcm_buf_int16[g_pcm_index];
                } else if (g_bits == 24) {
                    ((int32_t *)pcm_buf)[g_channel_num * i + ch] = g_pcm_buf_int32[g_pcm_index];
                } else if (g_bits == 32) {
                    ((int32_t *)pcm_buf)[g_channel_num * i + ch] = g_pcm_buf_int32[g_pcm_index];
                } else {
                    ASSERT(0, "[%s] g_bits(%d) is invalid", __func__, g_bits);
                }
            }

            g_pcm_index++;
            g_pcm_index = g_pcm_index % g_pcm_len;
        }
    }
}