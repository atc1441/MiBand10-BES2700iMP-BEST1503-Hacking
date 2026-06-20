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
#ifndef SPEECH_STEREO_SURROUND_REVERB_H
#define SPEECH_STEREO_SURROUND_REVERB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct{
    int reflection_used;
    int reflection_delay[6];
    float reflection_delay_gain[6];
} SurroundReverbConfig;


struct StereoSurroundReverbState_;

typedef struct {
    int32_t input_delay_buf_len;
    int32_t reflect_num;
    SurroundReverbConfig *local_cfg;
} StereoSurroundReverbState;

#define REFLECTION_MAX_LEN (128)

StereoSurroundReverbState *stereo_surround_reverb_init(SurroundReverbConfig *config);

int32_t stereo_surround_reverb_destroy(StereoSurroundReverbState *st);

int32_t stereo_surround_reverb_process(StereoSurroundReverbState *st,int16_t *delay_buf, float * out_buf, int32_t pcm_len, float total_gain);

int32_t stereo_surround_reverb_process_q15(StereoSurroundReverbState *st,int16_t *delay_buf, int16_t * out_buf, int32_t pcm_len, float total_gain,int stride);

int32_t stereo_surround_reverb_process_q15_not_clean(StereoSurroundReverbState *st,int16_t *delay_buf, int16_t * out_buf, int32_t pcm_len, float total_gain,int stride);


#ifdef __cplusplus
}
#endif

#endif