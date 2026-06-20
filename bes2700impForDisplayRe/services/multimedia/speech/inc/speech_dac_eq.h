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
#ifndef SPEECH_DAC_EQ_H
#define SPEECH_DAC_EQ_H

#include "speech_eq_cfg.h"

struct SpeechDacEqState_;

typedef struct SpeechDacEqState_ SpeechDacEqState;

#ifdef __cplusplus
extern "C" {
#endif

SpeechDacEqState *speech_dac_eq_init(int32_t sample_rate, int32_t frame_size, const EqConfig *cfg);

int32_t speech_dac_eq_destroy(SpeechDacEqState *st);

int32_t speech_dac_eq_set_config(SpeechDacEqState *st, const EqConfig *cfg);

float speech_dac_eq_get_required_mips(SpeechDacEqState *st);

#ifdef __cplusplus
}
#endif

#endif