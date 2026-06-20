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
#ifndef FEEDBACK_CANCELLER_H
#define FEEDBACK_CANCELLER_H

#include <stdint.h>
#include "ae_macros.h"
#include <stdbool.h>

#include "custom_allocator.h"

typedef struct
{
    uint32_t delay;
    uint32_t taps;
} AfcConfig;

struct AfcState_;

typedef struct AfcState_ AfcState;

typedef int32_t (*afc_callback)(float *coeffs, uint32_t num);

AfcState *afc_create(int sample_rate, int frame_size, const AfcConfig *config, custom_allocator *allocator);

void afc_destroy(AfcState *st);

void afc_process(AfcState* st, float *in, float *ref, float *out, uint32_t len, afc_callback cb);

#endif
