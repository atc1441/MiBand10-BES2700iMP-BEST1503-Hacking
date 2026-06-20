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
#include "sensor_hub_core_app_ai.h"

#define SENSOR_HUB_MCU_AI_REQUEST_FREQ (APP_SYSFREQ_78M)
#define SENSOR_HUB_MCU_AI_RELEASE_FREQ (APP_SYSFREQ_32K)

typedef uint32_t (* sensor_hub_ai_mcu_kws_history_pcm_data_size_update)(uint32_t);
typedef uint32_t (*sensor_hub_ai_mcu_kws_valid_raw_pcm_sample_check)(uint8_t* buf , uint32_t len);
typedef uint32_t (* sensor_hub_ai_mcu_get_valid_kws_pcm_offset)(void);
typedef void (*sensor_hub_ai_mcu_kws_word_info_handler_t)(uint8_t* buf , uint16_t len);
typedef uint32_t (*sensor_hub_ai_mcu_mic_data_come_handler_t)(uint8_t* buf , uint32_t len);


typedef struct{
    sensor_hub_ai_mcu_kws_history_pcm_data_size_update history_pcm_size_update_handler;
    sensor_hub_ai_mcu_kws_valid_raw_pcm_sample_check kws_raw_pcm_sample_check_handler;
    sensor_hub_ai_mcu_get_valid_kws_pcm_offset kws_get_valid_offset_of_raw_pcm_handler;
    sensor_hub_ai_mcu_kws_word_info_handler_t kws_info_handler;
    sensor_hub_ai_mcu_mic_data_come_handler_t mic_come_handler;
}SENSOR_HUB_AI_HANDLER_T;

typedef struct{
    uint16_t ai_user;
    uint32_t history_pcm_data_size;
    SENSOR_HUB_AI_HANDLER_T handler;
}SENSOR_HUB_AI_OPERATOR_T;

typedef struct{
    uint16_t ai_user_num;
    uint16_t ai_current_user;
    SENSOR_HUB_AI_OPERATOR_T ai_operator[SENSOR_HUB_MAXIAM_AI_USER];
}SENSOR_HUB_AI_OPERATOR_MANAGER_T;

/*
 * mcu ai initial
*/
void sensor_hub_ai_mcu_app_init(void);

/*
 * callback to be called to handle the event and data from sensor hub.
 * would be fistly registed, and then be used.
 * user, the registered application user
 * ai_operator, the operator of user
*/
void sensor_hub_ai_mcu_register_ai_user(SENSOR_HUB_AI_USER_E user,SENSOR_HUB_AI_OPERATOR_T ai_operator);

/*
 * MCU starts to request vad be in detecting mode on sensor-hub
*/
void app_sensor_hub_ai_mcu_request_vad_data_start(void);

/*
 * MCU stop the streaming data from sensor-hub to be in detecting mode
*/
void app_sensor_hub_ai_mcu_request_vad_data_stop(void);

void app_sensor_hub_ai_mcu_request_vad_data_close(void);

void app_sensor_hub_ai_mcu_env_setup(SENSOR_HUB_AI_USER_E user,uint8_t * info,uint32_t len);

void app_sensor_hub_ai_mcu_activate_ai_user(SENSOR_HUB_AI_USER_E user, bool op);

#ifdef __cplusplus
}
#endif

#endif

