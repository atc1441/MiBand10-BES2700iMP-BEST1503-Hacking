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
#ifndef __MCU_SENSOR_HUB_H__
#define __MCU_SENSOR_HUB_H__

#ifdef __cplusplus
extern "C" {
#endif

#define APP_VAD_DATA_PKT_SIZE  ((16000/1000)*(16/8)*1*8*4*2)

enum APP_VAD_EVENT_T {
    APP_VAD_EVT_IRQ_TRIG,  //events for VAD irq triggered
    APP_VAD_EVT_VOICE_CMD, //events for key word is recognized
};

enum APP_VAD_ADPT_ID_T {
    APP_VAD_ADPT_ID_NONE,
    APP_VAD_ADPT_ID_DEMO,
    APP_VAD_ADPT_ID_AI_KWS,
    APP_VAD_ADPT_ID_SNDP,
    APP_VAD_ADPT_ID_QTY,
};

enum APP_SENSOR_HUB_RESET_REASON {
    APP_SENSOR_HUB_WDOG_TIME_OUT,
};

enum APP_SENSOR_HUB_RESET_T {
    APP_SENSOR_HUB_INIT,
    APP_SENSOR_HUB_RESET,
};

typedef void (*app_vad_event_handler_t)(enum APP_VAD_EVENT_T event, uint8_t *param);

typedef void (*app_vad_data_handler_t)(uint32_t pkt_id, uint8_t *payload, uint32_t bytes);

void app_sensor_hub_init(void);
void app_sensor_hub_deinit(void);
bool app_sensor_hub_is_inited(void);
void app_mcu_sensor_hub_send_demo_req_with_rsp(void);
void app_mcu_sensor_hub_send_demo_req_no_rsp(void);
void app_mcu_sensor_hub_send_demo_instant_req(void);
void app_mcu_sensor_hub_watch_dog_polling_handler(void);

/* mcu core start sensor-hub vad */
int app_mcu_sensor_hub_start_vad(uint8_t vad_adaptor_id);

/* mcu core stop sensor-hub vad */
int app_mcu_sensor_hub_stop_vad(uint8_t vad_adaptor_id);

/* app_mcu_sensor_hub_setup_vad_event_handler - mcu core setup event handler
 * this subroutine should be invoked before VAD starts. The handler can handle
 * VAD events;
 */
int app_mcu_sensor_hub_setup_vad_event_handler(app_vad_event_handler_t handler);

/* app_mcu_sensor_hub_setup_vad_data_handler - mcu core setup vad data handler
 * this subroutine should be invoked before VAD starts. The handler can handle
 * VAD key word data stream or preload buffer data;
 */
int app_mcu_sensor_hub_setup_vad_data_handler(app_vad_data_handler_t handler);

/* app_mcu_sensor_hub_request_vad_data - mcu core request vad data
 * when VAD irq is triggered, the subroutine should be invoked for MCU core
 * requesting VAD data from sensor-hub core;
 * seek_size : mcu request to seek at the recording data offset of sensor-hub
 */
int app_mcu_sensor_hub_request_vad_data(bool request,uint32_t seek_size);

/*
 * mcu can directly request to seek at the recording data offset of sensor-hub
*/
int app_mcu_sensor_hub_seek_vad_data(uint32_t seek_size);

/*
 * app_mcu_sensor_hub_bypass_vad - bypass vad
 * When hot word is recognized, the VAD hardware can be bypassed, then
 * The codec capture stream will always on;
 * If bypass is true, the VAD hw will be bypassed;
 * If bypass is false, the VAD hw will be active immidiately;
 * This subroutine should be invoked after hot word is recognized;
 */
int app_mcu_sensor_hub_bypass_vad(bool bypass);

int app_mcu_sensor_hub_send_test_msg(uint32_t scmd, uint32_t arg1, uint32_t arg2);

void app_mcu_sensor_hub_enable_watch_dog(void);

void app_mcu_sensor_hub_disable_watch_dog(void);

void app_mcu_reset_sesor_hub(void);

#ifdef SENSOR_HUB_BSP_TEST
void app_sensor_hub_at_cmd_send(uint8_t *buf, uint16_t len);
#endif

#ifdef __cplusplus
}
#endif

#endif

