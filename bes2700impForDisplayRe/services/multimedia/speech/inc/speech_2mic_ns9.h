/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifndef __SPEECH_2MIC_NS9_H__
#define __SPEECH_2MIC_NS9_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int32_t     bypass;
    float       delay_taps;
    int32_t     freq_smooth_enable;
    int32_t     wnr_enable;
} Speech2MicNs9Config;

struct Speech2MicNs9State_;

typedef struct Speech2MicNs9State_ Speech2MicNs9State;

Speech2MicNs9State *speech_2mic_ns9_create(int32_t sample_rate, int32_t frame_size, const Speech2MicNs9Config *cfg);

int32_t speech_2mic_ns9_destroy(Speech2MicNs9State *st);

int32_t speech_2mic_ns9_set_config(Speech2MicNs9State *st, const Speech2MicNs9Config *cfg);

int32_t speech_2mic_ns9_process(Speech2MicNs9State *st, short *pcm_buf, int32_t pcm_len, short *out_buf);

int32_t speech_2mic_ns9_get_delay(Speech2MicNs9State *st);

float speech_2mic_ns9_get_required_mips(Speech2MicNs9State *st);

#ifdef __cplusplus
}
#endif

#endif
