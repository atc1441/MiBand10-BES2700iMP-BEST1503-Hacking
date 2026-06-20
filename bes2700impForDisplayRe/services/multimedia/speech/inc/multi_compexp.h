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
#ifndef MULTI_COMPEXP_H
#define MULTI_COMPEXP_H

#include <stdint.h>
#include "compexp.h"

#define MAX_COMPEXP_BAND_NUM (2)

typedef struct
{
    int32_t     bypass;
    int32_t     num;
    int32_t     xover_freq[MAX_COMPEXP_BAND_NUM - 1];
    int32_t     order; // should be 2*N
    CompexpConfig params[MAX_COMPEXP_BAND_NUM];
} MultiCompexpConfig;

typedef struct MultiCompexpState_ MultiCompexpState;

#ifdef __cplusplus
extern "C" {
#endif

MultiCompexpState *multi_compexp_create(int32_t sample_rate, int32_t frame_size, const MultiCompexpConfig *config);

MultiCompexpState *multi_compexp_create_with_custom_allocator(int32_t sample_rate, int32_t frame_size, const MultiCompexpConfig *config, custom_allocator *allocator);

int32_t multi_compexp_destroy(MultiCompexpState *st);

int32_t multi_compexp_set_config(MultiCompexpState *st, const MultiCompexpConfig *cfg);

int32_t multi_compexp_process(MultiCompexpState *st, int16_t *pcm_buf, int32_t pcm_len);

int32_t multi_compexp_process_int24(MultiCompexpState *st, int32_t *pcm_buf, int32_t pcm_len);

float multi_compexp_get_required_mips(MultiCompexpState *st);

#ifdef __cplusplus
}
#endif

#endif