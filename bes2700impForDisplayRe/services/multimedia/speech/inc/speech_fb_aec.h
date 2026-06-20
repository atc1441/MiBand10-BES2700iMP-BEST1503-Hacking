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
#ifndef __SPEECH_FB_AEC_H__
#define __SPEECH_FB_AEC_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    int32_t bypass;
    int32_t nfft_len;
    int32_t af_enabled;
    int32_t nlp_enabled;
    // af config
    int32_t blocks;
    int32_t delay;
    // nlp config
    float   gamma;
    int32_t echo_band_start;
    int32_t echo_band_end;
    float   min_ovrd;
    float   target_supp;
} SpeechFbAecConfig;

struct SpeechFbAecState_;

typedef struct SpeechFbAecState_ SpeechFbAecState;

SpeechFbAecState *speech_fb_aec_create(int sample_rate, int frame_size, const SpeechFbAecConfig *cfg);

int32_t speech_fb_aec_destroy(SpeechFbAecState *st);

int32_t speech_fb_aec_set_config(SpeechFbAecState *st, const SpeechFbAecConfig *cfg);

int32_t speech_fb_aec_process(SpeechFbAecState *st, int16_t *pcm_in, int32_t mic_num, int16_t *pcm_ref, int32_t pcm_len, int16_t *pcm_out);

#ifdef __cplusplus
}
#endif

#endif

