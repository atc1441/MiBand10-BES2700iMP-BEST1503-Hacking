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
#ifndef __MCU_SENSOR_HUB_APP_AI_H__
#define __MCU_SENSOR_HUB_APP_AI_H__


#ifdef __cplusplus
extern "C" {
#endif
#include "plat_types.h"

#define SENSOR_HUB_MCU_SNDP_REQUEST_FREQ (APP_SYSFREQ_78M)
#define SENSOR_HUB_MCU_SNDP_RELEASE_FREQ (APP_SYSFREQ_32K)

/*
 * MCU starts to request vad be in detecting mode on sensor-hub
*/
void app_sensor_hub_sndp_mcu_request_vad_start(void);

/*
 * MCU stop the streaming data from sensor-hub to be in detecting mode
*/
void app_sensor_hub_sndp_mcu_request_vad_data(bool onoff);

void app_sensor_hub_sndp_mcu_request_vad_stop(void);

#ifdef __cplusplus
}
#endif

#endif

