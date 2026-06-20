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
#ifndef APP_AUDIO_ACTIVE_DEVICE_MANAGER_H__
#define APP_AUDIO_ACTIVE_DEVICE_MANAGER_H__

#include "app_audio_bt_device.h"

#define STATE_CONNECTED                    0x01
#define STATE_DISCONNECTED                 0x02
#define STRAEMING_START                    0x03
#define STRAEMING_STOP                     0x04
#define RINGTONE_START                     0x05
#define RINGTONE_STOP                      0x06
#define IBRT_CONNECTED                     0x07
#define HFP_BUSY                           0x08
#define AOB_CALL_BUSY                      0x09
#define AVRCP_PLAYING                      0x0A
#define AVRCP_PAUSED                         0x0B

enum DEVICE_CHANGED_ACTION
{
    FORCE_SET_ACT_DEVICE               = 0x00,
    
    A2DP_STREAMING_CHANGED             = 0x01,   /* Streaming changed */
    HFP_STREAMING_CHANGED              = 0x02,

    LE_MUSIC_STREAMING_CHANGED         = 0x03,
    LE_CALL_STREAMING_CHANGED          = 0x04,
    
    RINGTONE_CHANGED                   = 0x05,
    MEDIA_STREAMING_CHANGED            = 0x06,
    DEVICE_CONN_STATUS_CHANGED         = 0x07,
};

#ifdef __cplusplus
extern "C" {
#endif

void app_audio_adm_init();

BT_AUDIO_DEVICE_T* app_audio_adm_get_music_active_device();

BT_AUDIO_DEVICE_T* app_audio_adm_get_active_device();

uint8_t app_audio_adm_get_bt_active_device();

uint8_t app_audio_adm_get_le_audio_active_device();

void app_audio_adm_ibrt_handle_action(uint8_t device_id,enum DEVICE_CHANGED_ACTION action,uint8_t state);

void app_audio_adm_le_audio_handle_action(uint8_t device_id,enum DEVICE_CHANGED_ACTION action,uint8_t state);

bool app_audio_adm_curr_a2dp_data_need_receive(uint8_t device_id);

bool app_audio_adm_curr_le_music_data_need_receive();

void app_audio_adm_active_device_dump();

bool app_audio_adm_media_stream_is_active(void);

BT_AUDIO_DEVICE_T* app_audio_adm_get_music_stream_device();

BT_AUDIO_DEVICE_T* app_audio_adm_get_call_stream_device();

uint8_t  app_audio_adm_get_le_audio_music_stream_device();

uint8_t  app_audio_adm_get_le_audio_call_stream_device();

void app_audio_adm_slave_set_active_device_handler(uint8_t device_id, uint8_t device_type);

bool app_audio_adm_is_any_device_is_streaming();

void app_adm_force_set_active_device(BT_AUDIO_DEVICE_T *device);

bool app_audio_adm_music_stream_is_active(uint8_t device_id, uint8_t device_type);

bool app_audio_adm_call_stream_is_active(uint8_t device_id);

#if BLE_AUDIO_ENABLED
bool app_audio_adm_is_this_device_cis_setup(bt_bdaddr_t *remote_addr);
#endif

#ifdef __cplusplus
    }
#endif

#endif /*APP_AUDIO_ACTIVE_DEVICE_MANAGER_H__ */

