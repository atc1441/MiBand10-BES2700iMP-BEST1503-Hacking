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
#ifndef __MCU_SENSOR_HUB_APP_AI_OTA_H__
#define __MCU_SENSOR_HUB_APP_AI_OTA_H__


#ifdef __cplusplus
extern "C" {
#endif

typedef void(*sensorHubOtaFinishCbType)(uint32_t magicCode);

typedef struct{
    sensorHubOtaFinishCbType finish_cb;
}sensorHubOtaOpCbType;


/*
 * MCU calls to update the dynamic data setion registed on sensor-hub
 * magicCode , used to identify which data section
 * src , copied src addr,
 * size, copied src size
 * cb, when update finished, application do cb handler
*/
uint32_t app_sensor_hub_mcu_data_update_write_start(uint32_t magicCode,uint8_t* src,uint32_t size,sensorHubOtaOpCbType cb);

bool app_sensor_hub_mcu_data_update_write_allow(void);

void app_sensor_hub_mcu_data_update_init(void);

uint32_t app_sensor_hub_mcu_updating_section_data_process(uint8_t * buf, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif

