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
#ifndef __SPEECH_2MIC_NS2_DENOISE_H__
#define __SPEECH_2MIC_NS2_DENOISE_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int32_t     bypass;
    float       delay_taps;             // MIC L/R delay samples. 0: 适用于麦克距离为<2cm; 1: 适用于麦克距离为2cm左右; 2: 适用于麦克距离为4cm左右
    int32_t     dualmic_enable;
    int32_t     freq_smooth_enable;     // 1: 频域滤波打开; 0: 频域滤波关闭; 默认打开
    int32_t     wnr_enable;             // wind noise reduction enable
    int32_t     skip_freq_range_start;
    int32_t     skip_freq_range_end;
    float       noise_supp;
    float       betalow;
    float       betamid;
    float       betahigh;
    float       vad_threshold;
    float       vad_threshold_flux;
} Speech2MicNs2Config;

struct Speech2MicNs2State_;

typedef struct Speech2MicNs2State_ Speech2MicNs2State;

#define CONSTRUCT_FUNC_NAME_A(p, c, m)          p ## _ ## c ## _ ## m
#define CONSTRUCT_FUNC_NAME(p, c, m)            CONSTRUCT_FUNC_NAME_A(p, c, m)

#ifndef SPEECH_2MIC_NS2_DENOISE_IMPL
#define SPEECH_2MIC_NS2_DENOISE_IMPL float
#endif

#define speech_2mic_ns2_create    CONSTRUCT_FUNC_NAME(speech_2mic_ns2, SPEECH_2MIC_NS2_DENOISE_IMPL, create)
#define speech_2mic_ns2_destroy    CONSTRUCT_FUNC_NAME(speech_2mic_ns2, SPEECH_2MIC_NS2_DENOISE_IMPL, destroy)
#define speech_2mic_ns2_set_config CONSTRUCT_FUNC_NAME(speech_2mic_ns2, SPEECH_2MIC_NS2_DENOISE_IMPL, set_config)
#define speech_2mic_ns2_analysis   CONSTRUCT_FUNC_NAME(speech_2mic_ns2, SPEECH_2MIC_NS2_DENOISE_IMPL, analysis)
#define speech_2mic_ns2_process   CONSTRUCT_FUNC_NAME(speech_2mic_ns2, SPEECH_2MIC_NS2_DENOISE_IMPL, process)
#define speech_2mic_ns2_get_delay   CONSTRUCT_FUNC_NAME(speech_2mic_ns2, SPEECH_2MIC_NS2_DENOISE_IMPL, get_delay)
#define speech_2mic_ns2_get_required_mips   CONSTRUCT_FUNC_NAME(speech_2mic_ns2, SPEECH_2MIC_NS2_DENOISE_IMPL, get_required_mips)
#define speech_2mic_ns2_get_wnr_gain   CONSTRUCT_FUNC_NAME(speech_2mic_ns2, SPEECH_2MIC_NS2_DENOISE_IMPL, get_wnr_gain)
#define speech_2mic_ns2_get_vad   CONSTRUCT_FUNC_NAME(speech_2mic_ns2, SPEECH_2MIC_NS2_DENOISE_IMPL, get_vad)
#define speech_2mic_ns2_get_power_ratio CONSTRUCT_FUNC_NAME(speech_2mic_ns2, SPEECH_2MIC_NS2_DENOISE_IMPL, get_power_ratio)
#define speech_2mic_ns2_get_wind_indictor CONSTRUCT_FUNC_NAME(speech_2mic_ns2, SPEECH_2MIC_NS2_DENOISE_IMPL, get_wind_indictor)

Speech2MicNs2State *speech_2mic_ns2_create(int32_t sample_rate, int32_t frame_size, const Speech2MicNs2Config *cfg);

int32_t speech_2mic_ns2_destroy(Speech2MicNs2State *st);

int32_t speech_2mic_ns2_set_config(Speech2MicNs2State *st, const Speech2MicNs2Config *cfg);

void speech_2mic_ns2_analysis(Speech2MicNs2State *st, short *in);

int32_t speech_2mic_ns2_process(Speech2MicNs2State *st, short *pcm_buf, int32_t pcm_len, short *out_buf);

int32_t speech_2mic_ns2_get_delay(Speech2MicNs2State *st);

float speech_2mic_ns2_get_required_mips(Speech2MicNs2State *st);

/*
 * swap         - 0, apply wnr gain on main mic
 *                1, apply wnr gain on reference mic
 * wnr_gain     - pointer to wnr gain
 * wnr_gain_num - pointer to wnr gain length
 */
void speech_2mic_ns2_get_wnr_gain(Speech2MicNs2State *st, bool *swap, float **wnr_gain, int *wnr_gain_num);

bool speech_2mic_ns2_get_vad(Speech2MicNs2State *st);

float speech_2mic_ns2_get_wind_indictor(Speech2MicNs2State *st);

float speech_2mic_ns2_get_power_ratio(Speech2MicNs2State *st);

#ifdef __cplusplus
}
#endif

#endif
