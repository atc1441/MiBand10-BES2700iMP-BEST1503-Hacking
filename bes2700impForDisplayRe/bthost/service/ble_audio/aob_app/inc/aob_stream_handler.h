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
#ifndef __AOB_STREAMINAOBG_HANDLER_H__
#define __AOB_STREAMINAOBG_HANDLER_H__
#include "ble_aob_common.h"

#ifdef __cplusplus
extern "C" {
#endif

void app_ble_audio_sink_streaming_init();

void app_ble_audio_policy_config();

BLE_AUDIO_POLICY_CONFIG_T* app_ble_audio_get_policy_config();

int app_ble_audio_bis_stream_set_resume_callback(void (*resume_cb)(uint8_t device_id, uint32_t param));

void app_ble_audio_sink_streaming_handle_event(uint8_t con_lid, uint8_t stream_lid, uint8_t data, app_ble_audio_event_t event);

void app_ble_audio_stream_start_handler(uint8_t ase_lid);

void app_ble_audio_switch_cis_cmp_register(void (*cis_switch_cb)(uint8_t device_id));

void app_ble_audio_switch_focus(uint8_t local_tws_role);

uint8_t app_ble_audio_count_music_streaming_device();

void app_ble_audio_handle_peer_swicth_info(void *addr);

void app_ble_audio_ava_context_ctrl_conversation_except_conlid(uint8_t con_lid_except, bool enable);

uint8_t app_ble_get_curr_play_bleaudio_id(void);

void app_ble_audio_switch_cis_cmp_register(void (*cis_switch_cb)(uint8_t device_id));

uint8_t app_ble_get_curr_play_bleaudio_id(void);

void app_ble_audio_switch_focus(uint8_t local_tws_role);

void app_ble_audio_gaf_media_status_handler_cb_register(void (*cb)(uint8_t con_lid, bool paused));

#ifdef __cplusplus
}
#endif


#endif /* __AOB_STREAMINAOBG_HANDLER_H__ */

