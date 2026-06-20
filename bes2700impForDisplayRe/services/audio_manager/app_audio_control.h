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

#ifndef __APP_AUDIO_CONTROL_H__
#define __APP_AUDIO_CONTROL_H__

#ifndef BLE_CALL_INVALID_ID
#define BLE_CALL_INVALID_ID (0)
#endif
typedef enum
{
    CALL_STATE_INCOMING                     = 0,
    CALL_STATE_DIALING                      = 1,
    CALL_STATE_ALERTING                     = 2,
    CALL_STATE_ACTIVE                       = 3,
    CALL_STATE_LOCAL_HELD                   = 4,
    CALL_STATE_REMOTE_HELD                  = 5,
    CALL_STATE_LOCAL_AND_REMOTE_HELD        = 6,
    CALL_STATE_IDLE                         = 7,
    CALL_STATE_OUTGOING                     = 8,
    CALL_STATE_THREE_WAY_INCOMING           = 9,
    CALL_STATE_TRREE_WAY_HOLD_CALLING       =10,
} CALL_STATE_E;

typedef enum
{
    PLAYING,
    PAUSED,
    IDLE,
}PLAYBACK_STATE_E;

typedef struct
{
    uint8_t      device_type;
    uint8_t      device_id;
    uint8_t      call_id;
    CALL_STATE_E state;
}CALL_STATE_INFO_T;

typedef struct
{
    uint8_t device_type;
    uint8_t device_id;
    PLAYBACK_STATE_E playback_status;
}PLAYBACK_INFO_T;

#ifdef __cplusplus
extern "C"{
#endif

void app_audio_control_reset_media_call_info();

CALL_STATE_INFO_T* app_bt_get_call_state_info();

CALL_STATE_E app_bt_get_call_state();

PLAYBACK_INFO_T* app_bt_get_music_playback_info();

PLAYBACK_STATE_E app_bt_get_music_playback_status();

void app_audio_ctrl_le_update_call_state(uint8_t conidx, uint8_t call_id, uint8_t state);

void app_audio_ctrl_update_ble_audio_music_state(uint8_t conidx, uint8_t event);

void app_audio_ctrl_update_bt_music_state(uint8_t device_id, uint8_t event);

void app_audio_bt_update_call_state(uint8_t device_id,bool force_update);

void app_audio_control_media_play(void);

void app_audio_control_media_pause(void);

void app_audio_control_media_fast_forward_start(void);

void app_audio_control_media_fast_forward_stop(void);

void app_audio_control_media_rewind_start(void);

void app_audio_control_media_rewind_stop(void);

void app_audio_control_media_forward();

void app_audio_control_media_backward();

void app_audio_control_streaming_volume_up(void);

void app_audio_control_streaming_volume_down(void);

void app_audio_control_call_answer(void);

void app_audio_control_call_terminate(void);

void app_audio_control_redial_last_call();

void app_audio_control_handle_three_way_incoming_call(void);

void app_audio_control_switch_hold_active_call(void);

void app_audio_control_release_hold_call_and_3way_incoming(void);

void app_audio_control_release_active_call(void);

void app_bt_audio_control_state_checker(void);

void app_audio_control_open_voice_assistant(void);

void app_audio_control_close_voice_assistant(void);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __APP_AUDIO_CONTROL_H__ */
