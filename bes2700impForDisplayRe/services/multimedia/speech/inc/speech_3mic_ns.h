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
#ifndef __SPEECH_3MIC_NS_H__
#define __SPEECH_3MIC_NS_H__

#include <stdint.h>
#include "speech_common.h"
#include "custom_allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int32_t     bypass;
    int32_t     wnr_enable;

    float       dist;
    int32_t     mode_flag;
    float       talk_pre_gain;
    //bf
    int32_t     bf_bin_start;
    int32_t     bf_bin_end;
    //vad
    int32_t     vad_bin_start;
    int32_t     vad_bin_end;
    float       energy_ratio;
    float       coef1_thd;
    float       energy1_thd; 
    float       coef2_thd;
    float       energy2_thd; 
    float       vad_speech_prob_thd;
    //af
    int32_t     outer_echo_af_enable;
    float       outer_af_error_threshold;
    float       fb_af_error_threshold;
    int32_t     outer_af_filter_len;
    int32_t     fb_af_filter_len;
    //nlp
    int32_t     echo_supp_enable;
    int32_t     ref_delay;
    int32_t     outer_echo_band_start;
    int32_t     outer_echo_band_end;
    int32_t     fb_echo_band_start;
    int32_t     fb_echo_band_end;

    int32_t     post_supp_enable;
    float       pf_denoise_db;
    int32_t     fb_treat_enable;
    //change with mode
    float       inner_denoise_db;
    const float*      fb_filter;
    float       fb_energy_diff_min;
    float       fb_energy_diff_max;
    // crossover
    int32_t     blend_en;
    int32_t     blend_mix_start;
    int32_t     blend_mix_end;
    int32_t     noise_blend_band[2];
    float       blend_noise_power_thd[2];
    float       blend_snr_thd[2];

    int32_t     comp_num;
    int32_t     comp_freq[MAX_COMP_NUM];
    int32_t     comp_gaindB[MAX_COMP_NUM];
    
    // wdrc
    int32_t     wdrc_enable;
    // gain
    float       pre_gain;
    float       post_gain;
    // vad
    float       comp_speech_prob_thd;
} Speech3MicNsConfig;

struct Speech3MicNsState_;

typedef struct Speech3MicNsState_ Speech3MicNsState;

Speech3MicNsState *speech_3mic_ns_create(int32_t sample_rate, int32_t frame_size, const Speech3MicNsConfig *cfg, custom_allocator *allocator);

int32_t speech_3mic_ns_destroy(Speech3MicNsState *st);

int32_t speech_3mic_ns_set_config(Speech3MicNsState *st, const Speech3MicNsConfig *cfg);

int32_t speech_3mic_ns_reset_config_with_mode(Speech3MicNsState *st, const Speech3MicNsConfig *cfg);

void speech_3mic_ns_preprocess(Speech3MicNsState *st, int16_t *pcm_buf, int16_t *ref_buf, int32_t pcm_len, int16_t *out_buf);

int32_t speech_3mic_ns_process(Speech3MicNsState *st, short *pcm_buf, short *ref_buf, int32_t pcm_len, short *out_buf);

float speech_3mic_get_required_mips(Speech3MicNsState *st);

void speech_3mic_ns_set_ns_state(Speech3MicNsState *st, void *ns);

void speech_3mic_ns_update_snr(Speech3MicNsState *st, float snr);

enum WIND_STATE_E speech_3mic_ns_get_wind_state(Speech3MicNsState *st);

#ifdef __cplusplus
}
#endif

#endif
