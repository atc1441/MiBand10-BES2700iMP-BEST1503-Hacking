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
#ifndef __SENSOR_HUB_VAD_CORE_H__
#define __SENSOR_HUB_VAD_CORE_H__

#ifdef __cplusplus
extern "C" {
#endif

void app_sensor_hub_core_vad_init(void);
int app_sensor_hub_core_vad_send_evt_msg(enum SENS_VAD_EVT_ID_T evt, uint32_t param0, uint32_t param1, uint32_t param2);
int app_sensor_hub_core_vad_send_data_msg(uint32_t data_idx, uint32_t data_addr, uint32_t data_size);

#ifdef __cplusplus
}
#endif

#endif

