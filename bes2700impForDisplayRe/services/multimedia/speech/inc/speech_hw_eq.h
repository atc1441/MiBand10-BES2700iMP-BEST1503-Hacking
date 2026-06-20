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
#ifndef SPEECH_HW_EQ_H
#define SPEECH_HW_EQ_H

#include "speech_eq_cfg.h"

struct SpeechHwEqState_;

typedef struct SpeechHwEqState_ SpeechHwEqState;

#ifdef __cplusplus
extern "C" {
#endif

SpeechHwEqState *speech_hw_eq_init(int32_t sample_rate, int32_t sample_bits, int32_t frame_size, const EqConfig *cfg);

int32_t speech_hw_eq_destroy(SpeechHwEqState *st);

int32_t speech_hw_eq_set_config(SpeechHwEqState *st, const EqConfig *cfg);

int32_t speech_hw_eq_process(SpeechHwEqState *st, int16_t *pcm_buf, int32_t pcm_len);

int32_t speech_hw_eq_process_int24(SpeechHwEqState *st, int32_t *pcm_buf, int32_t pcm_len);

float speech_hw_eq_get_required_mips(SpeechHwEqState *st);

#ifdef __cplusplus
}
#endif

#endif