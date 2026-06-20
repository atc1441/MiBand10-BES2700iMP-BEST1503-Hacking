/***************************************************************************
 * Copyright 2022-2023 BES.
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
 ***************************************************************************/
#ifndef __CAPSENSOR_FAC_CAL_H__
#define __CAPSENSOR_FAC_CAL_H__
#include "stdint.h"

#include "capsensor_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool offset_cal;
    bool diff_cal;
}CAPSENSOR_FACTORY_CAL_FLAG;

#if defined(CAPSENSOR_WEAR)
typedef struct{
    int     wear_offset0;
    int     wear_offset1;
    int     wear_noise0;
    int     wear_noise1;
    int     wear_diff0;
    int     wear_diff1;
    int     wear_snr0;
    int     wear_snr1;
    uint8_t wear_snr_result;
    uint8_t wear_diff_result;
}capsensor_wear_cal_data;
extern capsensor_wear_cal_data wear_cal_data;

typedef struct{
    int   wear_offset_up;
    int   wear_offset_down;
    float wear_diff0_up;
    float wear_diff0_down;
    float wear_diff1_up;
    float wear_diff1_down;
    int   wear_snr0_down;
    int   wear_snr1_down;
}capsensor_wear_cal_range;
extern capsensor_wear_cal_range wear_cal_range;

void capsensor_set_wear_cal_range(capsensor_wear_cal_range* range);
void capsensor_get_wear_cal_data(capsensor_wear_cal_data* data);
#endif

#if defined(CAPSENSOR_TOUCH)
typedef struct{
#ifdef CAPSENSOR_SLIDE
    int touch_diff0;
    int touch_diff1;
    int touch_diff2;
    int touch_noise0;
    int touch_noise1;
    int touch_noise2;
    int touch_snr0;
    int touch_snr1;
    int touch_snr2;
#else
    int touch_diff0;
    int touch_noise0;
    int touch_snr0;
#endif
    uint8_t touch_snr_result;
    uint8_t touch_diff_result;
}capsensor_touch_cal_data;
extern capsensor_touch_cal_data touch_cal_data;

typedef struct{
#ifdef CAPSENSOR_SLIDE
    float touch_diff0_up;
    float touch_diff0_down;
    float touch_diff1_up;
    float touch_diff1_down;
    float touch_diff2_up;
    float touch_diff2_down;
    int   touch_snr0_down;
    int   touch_snr1_down;
    int   touch_snr2_down;
#else
    float touch_diff0_up;
    float touch_diff0_down;
    int   touch_snr0_down;
#endif
}capsensor_touch_cal_range;
extern capsensor_touch_cal_range touch_cal_range;

void capsensor_set_touch_cal_range(capsensor_touch_cal_range* range);
void capsensor_get_touch_cal_data(capsensor_touch_cal_data* data);
#endif

void capsensor_factory_calculate_flag_set(bool m_cal_offset_flag, bool m_cal_diff_flag);
void capsensor_factory_calculate(struct capsensor_sample_data * data, int len);

#ifdef __cplusplus
}
#endif

#endif /* __CAPSENSOR_DEBUG_SERVER_H__ */
