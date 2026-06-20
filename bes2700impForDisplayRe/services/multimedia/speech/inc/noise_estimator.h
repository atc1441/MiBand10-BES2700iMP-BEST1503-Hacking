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
#ifndef __NOISE_ESTIMATOR_H__
#define __NOISE_ESTIMATOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#define NOISE_ESTIMATOR_GET_BAND_RES       (0)

#define NOISE_BAND_NUM      (2)
typedef struct {
	uint32_t	smooth_enable;
    uint32_t	freq1;
	uint32_t	freq2;
	uint32_t  	freq3;
} NoiseEstimatorConfig;


struct NoiseEstimatorState_;

typedef struct NoiseEstimatorState_ NoiseEstimatorState;

NoiseEstimatorState *NoiseEstimator_create(int32_t sample_rate, int32_t frame_size, const NoiseEstimatorConfig *cfg, void *get_ext_buff(uint32_t));

int32_t NoiseEstimator_destroy(NoiseEstimatorState *st);

int32_t NoiseEstimator_process(NoiseEstimatorState *st, int16_t *pcm_buf, int32_t pcm_len);

int32_t NoiseEstimator_ctl(NoiseEstimatorState *st, int32_t ctl, void *ptr);

#ifdef __cplusplus
}
#endif

#endif
