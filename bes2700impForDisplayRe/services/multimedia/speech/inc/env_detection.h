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


#ifndef __ENV_DETECTION_H__
#define __ENV_DETECTION_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"
#include "stdint.h"

typedef struct {
	uint32_t    debug_en;
	float       strong_thd;
	float       strong_limit_thd;
	float       lower_thd;
	float       quiet_thd;
	float       snr_thd;
	int         snr_count;
	int         period;
	int         window_size;
    int         strong_count;
    int         normal_count;
	float       band_freq[4];
    float       band_weight[3];
} env_detection_cfg_t;

typedef enum {
	ENV_STATUS_INVALID = 0,
    ENV_STATUS_QUIET_ANC,
	ENV_STATUS_LOWER_ANC,
    ENV_STATUS_MIDDLE_ANC,
	ENV_STATUS_STRONG_ANC,
} env_detection_status_t;


typedef struct EnvDetectionState_ EnvDetectionState;

EnvDetectionState * env_detection_create(env_detection_cfg_t * cfg, uint8_t *heap_buf, uint32_t heap_size, int sample_rate, int frame_size);
int32_t env_detection_destroy(EnvDetectionState* st);
int32_t env_detection_process(EnvDetectionState* st, float *inF);
int32_t snr_detection(void);

#ifdef __cplusplus
}
#endif

#endif