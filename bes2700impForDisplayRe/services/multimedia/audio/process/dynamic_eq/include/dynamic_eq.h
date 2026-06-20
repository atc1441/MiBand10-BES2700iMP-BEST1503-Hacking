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

#ifndef __DYNAMIC_EQ_H__
#define __DYNAMIC_EQ_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "hal_aud.h"
#include "iirfilt.h"

#define MAX_DYEQ_NUM  (6)

#define DYNAMIC_EQ_DEFAULT_LEVEL       (10.0)

#define FRAME_INTERVAL      (15)
#define SMOOTH_COEFF        (0.32)
#define SET_EQ_SMOOTH       (0.445)

typedef struct {
	IIR_TYPE_T	type;
	float	gain;
	float	fc;
	float	Q;
} EQ_CFG_T;

typedef struct {
	float	CT;
	float	CR;
	float	WT;
	float	WR;
	float 	ET;
	float	ER;
	float	attack_time;
	float	release_time;
} DRC_CFG_T;

typedef struct {
	EQ_CFG_T 	dyeq_eq_cfg;
	DRC_CFG_T	dyeq_drc_cfg;
} DYEQ_PARAM_T;

typedef struct
{
	int32_t		switch_on;
	int32_t		debug;
	int32_t 	eq_num;
	DYEQ_PARAM_T	dyeq_param_cfg[MAX_DYEQ_NUM];
} DynamicEqConfig;

typedef struct DynamicEqState_ DynamicEqState;

DynamicEqState *dynamic_eq_create(int32_t sample_rate, int32_t frame_size, int32_t sample_bits,  int32_t ch_num, const DynamicEqConfig *cfg);

int32_t dynamic_eq_process(DynamicEqState *st, uint8_t *input_pcm, int32_t pcm_len, int vol_level);

int32_t dynamic_eq_set_config(DynamicEqState *st, const DynamicEqConfig *config);

int32_t dynamic_eq_set_new_eq_compensation(DynamicEqState *st,const IIR_CFG_T *customer_iir_cfg);

int32_t dynamic_eq_destroy(DynamicEqState *st);

IIR_CFG_T *dynamic_eq_get_iir_eq_running_cfg(DynamicEqState *st);

int32_t dynamic_eq_get_eq_update_flag(DynamicEqState *st);

void dynamic_eq_set_eq_update_flag(DynamicEqState *st, int32_t val);

void dynamic_eq_set_dynamic_level(DynamicEqState *st, float *level);

int32_t dynamic_eq_set_customer_volume_table_type(DynamicEqState *st, int32_t volume_table_type);

void dynamic_eq_switch(DynamicEqState *st, int32_t onoff);

#ifdef __cplusplus
}
#endif
#endif
