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
#ifndef DYNAMIC_BOOST_H
#define DYNAMIC_BOOST_H

#include <stdint.h>
#include "iir_process.h"

#define MAX_BOOST_EQ_NUM (4)

#define DEFAULT_LEVEL       (10)
#define DEFAULT_SWITCH_ON   (1)

#define FRAME_INTERVAL      (15)
#define SMOOTH_COEFF        (0.32)
#define SET_EQ_SMOOTH       (0.445)

typedef struct {
    IIR_TYPE_T type;
    float gain;
    float freq;
    float Q;
} dynamic_boost_eq_cfg;

typedef struct
{
    int32_t     debug;
    int32_t     xover_freq[1];
    int32_t     order; // should be 2*N
    float       CT;
    float       CS;
    float       WT;
    float       WS;
    float       ET;
    float       ES;
    float       attack_time;
    float       release_time;
    float       makeup_gain;
    int32_t     delay;
    float       tav;
    int32_t     eq_num;
    dynamic_boost_eq_cfg boost_eq[MAX_BOOST_EQ_NUM];
} DynamicBoostConfig;

typedef struct DynamicBoostState_ DynamicBoostState;

#ifdef __cplusplus
extern "C" {
#endif

DynamicBoostState *dynamic_boost_create(int32_t sample_rate, int32_t frame_size, int32_t sample_bit, int32_t ch_num, const DynamicBoostConfig *cfg);

int32_t dynamic_boost_destroy(DynamicBoostState *st);

int32_t dynamic_boost_set_config(DynamicBoostState *st, const DynamicBoostConfig *cfg);

void dynamic_boost_process(DynamicBoostState *st, uint8_t *buf, int32_t pcm_len, int vol_level);

IIR_CFG_T *dynamic_boost_get_iir_eq_running_cfg(DynamicBoostState *st);

int32_t dynamic_boost_set_customer_volume_table_type(DynamicBoostState *st, int32_t volume_table_type);

int32_t dynamic_boost_set_new_eq_compensation(DynamicBoostState *st, const IIR_CFG_T *customer_iir_cfg);

int32_t dynamic_boost_get_dynamic_level(DynamicBoostState *st);

void dynamic_boost_set_dynamic_level(DynamicBoostState *st, int32_t level);

int32_t dynamic_boost_get_eq_update_flag(DynamicBoostState *st);

void dynamic_boost_set_eq_update_flag(DynamicBoostState *st, int32_t val);

void dynamic_boost_switch(DynamicBoostState *st, int32_t onoff);

#ifdef __cplusplus
}
#endif

#endif