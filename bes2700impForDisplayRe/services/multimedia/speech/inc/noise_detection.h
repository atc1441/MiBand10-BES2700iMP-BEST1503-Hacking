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
#ifndef __NOISE_DETECTION_H__
#define __NOISE_DETECTION_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NOISE_DET_NOISE = 0,
    NOISE_DET_SILENCE,
    NOISE_DET_INVALID,
} noise_det_status;


typedef struct {
    int32_t     bypass;
    int32_t     debug_en;
    float       snr_thd;
    float       noisepower_thd0;
    float       noisepower_thd1;
    int32_t     pwr_start_bin;
    int32_t     pwr_end_bin;
    int32_t     ksi_start_bin;
    int32_t     ksi_end_bin;
    int32_t     average_period;
} NoiseDetectionConfig;

struct NoiseDetectionState_;

typedef struct NoiseDetectionState_ NoiseDetectionState;


NoiseDetectionState *NoiseDetection_create(int sample_rate, int frame_size, const NoiseDetectionConfig *cfg);

void NoiseDetection_destroy(NoiseDetectionState *st);

int32_t NoiseDetection_process(NoiseDetectionState *st, short *mic_buf, uint32_t frame_len, uint32_t chan_num);

#ifdef __cplusplus
}
#endif

#endif
