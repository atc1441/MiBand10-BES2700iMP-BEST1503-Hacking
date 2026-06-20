/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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

#ifndef __CODEC_CALIB_H__
#define __CODEC_CALIB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"

/*
 * The function is used to auto load DAC DC offset and digital
 * gain value from nvrecord;
 * If these calibration data is not find in NV, the programe will
 * auto calibrate DAC's DC and Gain.
 *
 * There are two arguments:
 *    - open_af    if true, it will try to open audioflinger before starts calibration;
 *    - reboot     if true, it will try to reboot system after calibration accomplished;
 *    - init_nv    if true, it will try to initialize nvrecord before starts calibration;
 *
 * return: 0(success), !0(failed)
 */

int codec_dac_dc_auto_load(bool open_af, bool reboot, bool init_nv);

#ifdef AUDIO_ADC_DC_AUTO_CALIB
int codec_adc_dc_auto_load(bool open_af, bool reboot, bool init_nv);
#endif

/* The function is used for production line to test DAC DC */
int codec_dac_dc_prod_test(int cmd);

#ifdef __cplusplus
}
#endif

#endif
