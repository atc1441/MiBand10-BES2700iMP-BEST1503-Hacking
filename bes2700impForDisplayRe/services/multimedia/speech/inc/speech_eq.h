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
#ifndef SPEECH_EQ_H
#define SPEECH_EQ_H

#include "speech_eq_cfg.h"
#include "ae_macros.h"
#include "custom_allocator.h"

typedef struct EqState_ EqState;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * EQ_IMPL options:
 *  raw: plain c impl
 *  arm: cmsis dsp
 */
#ifndef EQ_IMPL
// use more frame_size * sizeof(float) ram
#define EQ_IMPL arm
#endif

#define eq_init    CONSTRUCT_FUNC_NAME(eq, EQ_IMPL, init)
#define eq_init_with_custom_allocator    CONSTRUCT_FUNC_NAME(eq, EQ_IMPL, init_with_custom_allocator)
#define multi_eq_init CONSTRUCT_FUNC_NAME(multi_eq, EQ_IMPL, init)
#define multi_eq_init_with_custom_allocator CONSTRUCT_FUNC_NAME(multi_eq, EQ_IMPL, init_with_custom_allocator)
#define eq_destroy    CONSTRUCT_FUNC_NAME(eq, EQ_IMPL, destroy)
#define eq_set_config    CONSTRUCT_FUNC_NAME(eq, EQ_IMPL, set_config)
#define eq_process    CONSTRUCT_FUNC_NAME(eq, EQ_IMPL, process)
#define eq_process_int24    CONSTRUCT_FUNC_NAME(eq, EQ_IMPL, process_int24)
#define eq_process_float    CONSTRUCT_FUNC_NAME(eq, EQ_IMPL, process_float)
#define eq_process2    CONSTRUCT_FUNC_NAME(eq, EQ_IMPL, process2)
#define eq_process2_int24    CONSTRUCT_FUNC_NAME(eq, EQ_IMPL, process2_int24)
#define eq_process2_float    CONSTRUCT_FUNC_NAME(eq, EQ_IMPL, process2_float)
#define eq_get_required_mips    CONSTRUCT_FUNC_NAME(eq, EQ_IMPL, get_required_mips)

EqState *eq_init(int32_t sample_rate, int32_t frame_size, const EqConfig *cfg);

EqState *eq_init_with_custom_allocator(int32_t sample_rate, int32_t frame_size, const EqConfig *cfg, custom_allocator *allocator);

EqState *multi_eq_init(int32_t sample_rate, int32_t frame_size, int32_t ch_num, const EqConfig *cfg);

EqState *multi_eq_init_with_custom_allocator(int32_t sample_rate, int32_t frame_size, int32_t ch_num, const EqConfig *cfg, custom_allocator *allocator);

int32_t eq_destroy(EqState *st);

int32_t eq_set_config(EqState *st, const EqConfig *cfg);

int32_t eq_process(EqState *st, int16_t *pcm_buf, int32_t pcm_len);

int32_t eq_process_int24(EqState *st, int32_t *pcm_buf, int32_t pcm_len);

int32_t eq_process_float(EqState *st, float *pcm_buf, int32_t pcm_len);

int32_t eq_process2(EqState *st, int16_t *pcm_buf, int32_t pcm_len, int32_t stride);

int32_t eq_process2_int24(EqState *st, int32_t *pcm_buf, int32_t pcm_len, int32_t stride);

int32_t eq_process2_float(EqState *st, float *pcm_buf, int32_t pcm_len, int32_t stride);

float eq_get_required_mips(EqState *st);

EqState *eq_hwiir_init(int32_t sample_rate, int32_t frame_size, const EqConfig *cfg);

int32_t eq_hwiir_destroy(EqState *st);

int32_t eq_hwiir_set_config(EqState *st, const EqConfig *cfg);

int32_t eq_hwiir_process(EqState *st, int16_t *pcm_buf, int32_t pcm_len);

int32_t eq_hwiir_process_int24(EqState *st, int32_t *pcm_buf, int32_t pcm_len);

float eq_hwiir_get_required_mips(EqState *st);

#ifdef __cplusplus
}
#endif

#endif