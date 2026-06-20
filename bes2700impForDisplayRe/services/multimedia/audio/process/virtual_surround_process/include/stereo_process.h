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
#ifndef SPEECH_STEREO_SURROUND_H
#define SPEECH_STEREO_SURROUND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    bool        switch_on;
    //          sample
    int32_t     pre_delay_1;
    int32_t     pre_delay_2;
    int32_t     pre_delay_3;
    int32_t     allpass_len1;
    int32_t     allpass_len2;
    int32_t     allpass_len3;
    int32_t     allpass_len4;
    int32_t     allpass_len5;
    int32_t     allpass_len6;

    //          percent
    float       reverb_input_ratio_1;
    float       reverb_input_ratio_2;

    float       k1;
    float       k2;
    float       k3;
    float       k4;
    float       k5;
    float       k6;

    float       center_channel_ratio;
    float       surround_channel_ratio;
    float       output_ratio;

    float       hrtf_input_ratio;
    float       hrtf_output_ratio;
    int32_t     hrtf_select_index;
} VirtualSurroundConfig;

typedef enum {
    VIRTUAL_SURROUND_FIR_MIDDLE_LEFT = 0,
    VIRTUAL_SURROUND_FIR_MIDDLE_RIGHT,
    VIRTUAL_SURROUND_FIR_DIR_LEFT_RIGHT,
    VIRTUAL_SURROUND_FIR_DIR_RIGHT_RIGHT,
    VIRTUAL_SURROUND_FIR_DIR_LEFT_LEFT,
    VIRTUAL_SURROUND_FIR_DIR_RIGHT_LEFT,
    VIRTUAL_SURROUND_FIR_SUR_LEFT_RIGHT,
    VIRTUAL_SURROUND_FIR_SUR_RIGHT_RIGHT,
    VIRTUAL_SURROUND_FIR_SUR_LEFT_LEFT,
    VIRTUAL_SURROUND_FIR_SUR_RIGHT_LEFT,
    VIRTUAL_SURROUND_FIR_QTY
} stereo_surround_fir_index_t;

struct StereoSurroundState_;

typedef struct StereoSurroundState_ StereoSurroundState;

struct StereoSurroundStateCp_;

typedef struct StereoSurroundStateCp_ StereoSurroundStateCp;

int32_t stereo_surround_get_onoff_status(StereoSurroundState *st);

StereoSurroundState *stereo_surround_init(int32_t sample_rate, int32_t frame_size, int32_t sample_bits, int32_t nv_role, int32_t is_ap_cp, int32_t sw_ch_num, const VirtualSurroundConfig *config);

int32_t stereo_surround_set_config(StereoSurroundState *st, const VirtualSurroundConfig *config);

int32_t stereo_surround_destroy(StereoSurroundState *st);

int32_t stereo_surround_preprocess(StereoSurroundState *st,uint8_t *buf, int32_t len);

int32_t stereo_surround_process(StereoSurroundState *st,uint8_t *buf, int32_t len);

int32_t stereo_surround_onoff(StereoSurroundState *st, int32_t onoff);

int32_t stereo_surround_fir_onoff(StereoSurroundState *st, int32_t onoff);

int32_t stereo_surround_preprocess_cp(StereoSurroundState *st,uint8_t *buf, int32_t len);

int32_t stereo_surround_preprocess_ap(StereoSurroundState *st,uint8_t *buf, int32_t len);

int32_t stereo_surround_set_yaw_angle(StereoSurroundState *st, float yaw);

int32_t stereo_surround_set_pitch_angle(StereoSurroundState *st, float pitch);

#ifdef __cplusplus
}
#endif

#endif