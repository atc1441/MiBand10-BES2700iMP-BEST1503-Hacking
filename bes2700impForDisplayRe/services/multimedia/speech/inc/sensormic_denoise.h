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
#ifndef __SENSORMIC_DENOISE_H__
#define __SENSORMIC_DENOISE_H__

#include <stdint.h>
#include "speech_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int32_t     bypass;
    int32_t     wnr_enable;
    float       wind_thd;
    int32_t     wnd_pwr_thd;
    float       wind_gamma;
    int32_t     state_trans_frame_thd;

    int32_t     af_enable;
    float       filter_gamma;

    int32_t     vad_bin_start;
    int32_t     vad_bin_end;
    float       coef1_thd;
    float       coef2_thd;

    int32_t     low_ram_enable;
    int32_t     echo_supp_enable;
    int32_t     ref_delay;

    int32_t     post_supp_enable;
    int32_t     denoise_db;
    // crossover
    int32_t     blend_en;
    int32_t     comp_num;
    int32_t     comp_freq[MAX_COMP_NUM];
    int32_t     comp_gaindB[MAX_COMP_NUM];
} SensorMicDenoiseConfig;

struct SensorMicDenoiseState_;

typedef struct SensorMicDenoiseState_ SensorMicDenoiseState;

SensorMicDenoiseState *sensormic_denoise_create(int32_t sample_rate, int32_t frame_size, const SensorMicDenoiseConfig *cfg);

int32_t sensormic_denoise_destroy(SensorMicDenoiseState *st);

int32_t sensormic_denoise_set_config(SensorMicDenoiseState *st, const SensorMicDenoiseConfig *cfg);

void sensormic_denoise_preprocess(SensorMicDenoiseState* st, int16_t* pcm_buf, int16_t *ref_buf, int32_t pcm_len, int16_t* inner_buf);

int32_t sensormic_denoise_process(SensorMicDenoiseState *st, int16_t* pcm_buf, int16_t *ref_buf, int32_t pcm_len, int16_t* out_buf);

float sensormic_denoise_get_required_mips(SensorMicDenoiseState *st);

void sensormic_denoise_set_anc_status(SensorMicDenoiseState *st, bool anc_enable);

enum WIND_STATE_E sensormic_denoise_get_wind_state(SensorMicDenoiseState *st);



#ifdef __cplusplus
}
#endif

#endif
