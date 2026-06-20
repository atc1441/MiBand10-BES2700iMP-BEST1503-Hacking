/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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


#ifndef __SPEECH_1MIC_NS_H__
#define __SPEECH_1MIC_NS_H__


#include <stdint.h>

#include "custom_allocator.h"

typedef struct {
    int32_t         bypass;
    int32_t         nn_gain_db;
    int32_t         wdrc_enable;
    int32_t         pf_denoise_db;

} Speech1MicNSConfig;

struct Speech1MicNSState_;

typedef struct Speech1MicNSState_ Speech1MicNSState;

#ifdef __cplusplus
extern "C" {
#endif

Speech1MicNSState *speech_1mic_ns_create(Speech1MicNSConfig *cfg, custom_allocator *allocator);

int32_t speech_1mic_ns_destroy(Speech1MicNSState *st);

int32_t speech_1mic_ns_process(Speech1MicNSState *st, int16_t *pcm_buf, int16_t *ref_buf, int32_t pcm_len);

#ifdef __cplusplus
}
#endif

#endif
