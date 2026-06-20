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
#ifndef __APP_BT_MEDIA_MANAGER_H__
#define __APP_BT_MEDIA_MANAGER_H__

#include "bluetooth_bt_api.h"
#include "audio_player_adapter.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "btapp.h"

#define  BT_STREAM_DIRECTION_OUT    0
#define  BT_STREAM_DIRECTION_IN     1

// direction is out
#define  BT_STREAM_MUSIC          0x1
#define  BT_STREAM_MEDIA        0x2
#define  BT_STREAM_VOICE        0x4
#ifdef AUDIO_LINEIN
#define  BT_STREAM_LINEIN       0x8
#endif
#ifdef RB_CODEC
#define  BT_STREAM_RBCODEC      0x10    //from rockbox decoder
#endif
#ifdef AUDIO_PCM_PLAYER
#define  BT_STREAM_PCM_PLAYER   0x20
#endif

// direction is in
#ifdef __AI_VOICE__
#define BT_STREAM_AI_VOICE      0x200
#endif

#define BT_STREAM_TYPE_MASK   (BT_STREAM_MUSIC | BT_STREAM_MEDIA | BT_STREAM_VOICE)


enum APP_BT_MEDIA_MANAGER_ID_T {
    APP_BT_STREAM_MANAGER_START = 0,
    APP_BT_STREAM_MANAGER_STOP,
    APP_BT_STREAM_MANAGER_STOP_MEDIA,
    APP_BT_STREAM_MANAGER_UPDATE_MEDIA,
    APP_BT_STREAM_MANAGER_SWAP_SCO,
    APP_BT_STREAM_MANAGER_CTRL_VOLUME,
    APP_BT_STREAM_MANAGER_TUNE_SAMPLERATE_RATIO,
    APP_BT_STREAM_MANAGER_SET_OUTPUT_ROUTE,
    APP_BT_STREAM_MANAGER_NUM,
};

enum APP_BT_BLE_MEDIA_MANAGER_ID_T {
    APP_BT_BLE_STREAM_MANAGER_START = 10,
    APP_BT_BLE_STREAM_MANAGER_STOP,
    APP_BT_BLE_STREAM_MANAGER_STOP_SINGLE_STREAM,
};

#ifdef BLE_WALKIE_TALKIE
enum APP_BT_WT_MEDIA_MANAGER_ID_T {
    APP_BT_WT_STREAM_MANAGER_START = 20,
    APP_BT_WT_STREAM_MANAGER_STOP,
};
#endif

enum APP_BT_BIS_TRAN_MEDIA_MANAGER_ID_T {
    APP_BT_BIS_TRAN_STREAM_MANAGER_START = 40,
    APP_BT_BIS_TRAN_STREAM_MANAGER_STOP,
};

typedef struct {
    uint8_t id;
    uint16_t stream_type;
    uint8_t device_id;
    uint16_t aud_id;

}APP_AUDIO_MANAGER_MSG_STRUCT;

enum APP_AUDIO_MANAGER_VOLUME_CTRL_T {
    APP_AUDIO_MANAGER_VOLUME_CTRL_SET = 0,
    APP_AUDIO_MANAGER_VOLUME_CTRL_UP,
    APP_AUDIO_MANAGER_VOLUME_CTRL_DOWN,
    APP_AUDIO_MANAGER_VOLUME_CTRL_NUM,
};

typedef void (*APP_AUDIO_MANAGER_CALLBACK_T)(uint8_t device_id, uint32_t status, uint32_t param);
#ifdef BLE_WALKIE_TALKIE
typedef void (*APP_WT_AUDIO_MANAGER_CALLBACK_T)(uint8_t device_id,uint32_t context_type);
#endif

typedef void (*APP_BIS_TRAN_AUDIO_MANAGER_CALLBACK_T)(uint8_t device_id,uint32_t context_type);

#define APP_AUDIO_MANAGER_SET_MESSAGE(appevt, id, stream_type) (appevt = (((uint32_t)id&0xffff)<<16)|(stream_type&0xffff))
#define APP_AUDIO_MANAGER_SET_MESSAGE0(appmsg, device_id,aud_id) (appmsg = (((uint32_t)device_id&0xffff)<<16)|(aud_id&0xffff))
#define APP_AUDIO_MANAGER_GET_ID(appevt, id) (id = (appevt>>16)&0xffff)
#define APP_AUDIO_MANAGER_GET_STREAM_TYPE(appevt, stream_type) (stream_type = appevt&0xffff)
#define APP_AUDIO_MANAGER_GET_DEVICE_ID(appmsg, device_id) (device_id = (appmsg>>16)&0xffff)
#define APP_AUDIO_MANAGER_GET_AUD_ID(appmsg, aud_id) (aud_id = appmsg&0xffff)
#define APP_AUDIO_MANAGER_GET_CALLBACK(appmsg, callback) (callback = (APP_AUDIO_MANAGER_CALLBACK_T)appmsg)
#define APP_AUDIO_MANAGER_GET_CALLBACK_PARAM(appmsg, param) (param = appmsg)

int app_audio_manager_sendrequest(uint8_t massage_id,uint16_t stream_type, uint8_t device_id, uint16_t aud_id);
int app_audio_manager_sendrequest_need_callback(
                           uint8_t massage_id, uint16_t stream_type, uint8_t device_id, uint16_t aud_id, uint32_t cb, uint32_t cb_param);
void app_audio_manager_open(void);

void  bt_media_start(uint16_t stream_type,int device_id,uint16_t media_id);
void bt_media_stop(uint16_t stream_type,int device_id,uint16_t media_id);
uint8_t bt_media_is_media_active_by_type(uint16_t media_type);
void bt_media_volume_ptr_update_by_mediatype(uint16_t stream_type);
int app_audio_manager_set_active_sco_num(int id);
int app_audio_manager_get_active_sco_num(void);
btif_hf_channel_t* app_audio_manager_get_active_sco_chnl(void);
int app_audio_manager_swap_sco(int id);
uint8_t bt_media_is_media_active_by_device(uint16_t media_type,int device_id);
uint16_t bt_media_get_current_media(void);
void app_bt_media_set_current_media(uint16_t media_type);
int app_audio_manager_sco_status_checker(void);
int app_audio_manager_switch_a2dp(int id);
bool app_audio_manager_a2dp_is_active(int id);
bool app_audio_manager_capture_is_active(void);
bool app_audio_manager_media_is_active(void);
bool app_audio_manager_hfp_is_active(int id);
hfp_sco_codec_t app_audio_manager_get_scocodecid(void);
bool bt_media_is_media_idle(void);
uint8_t bt_media_device_enumerate_media_type_by_prior();
void bt_media_clean_up(void);
bool bt_media_cur_is_bt_stream_music(void);
#ifdef AUDIO_LINEIN
bool bt_media_cur_is_linein_stream(void);
#endif
bool bt_media_is_media_active_by_music(void);
bool bt_media_is_media_active_by_call(void);
#ifdef RB_CODEC
bool app_rbplay_is_localplayer_mode(void);
bool app_rbplay_mode_switch(void);
void app_rbplay_set_player_mode(bool isInPlayerMode);
void app_rbcodec_ctr_play_onoff(bool on );
bool app_rbcodec_get_play_status(void);
void app_rbcodec_toggle_play_stop(void);
#endif

void app_stop_a2dp_media_stream(uint8_t devId);
void app_stop_sco_media_stream(uint8_t devId);
int app_audio_manager_ctrl_volume(enum APP_AUDIO_MANAGER_VOLUME_CTRL_T volume_ctrl, uint16_t volume_level);
int app_audio_manager_ctrl_volume_with_callback(enum APP_AUDIO_MANAGER_VOLUME_CTRL_T volume_ctrl, uint16_t volume_level, void (*cb)(uint8_t device_id));
int app_audio_manager_tune_samplerate_ratio(enum AUD_STREAM_T stream, float ratio);
void app_audio_manager_set_output_route(Audio_output_route_t *route_info);
bool bt_media_cur_is_bt_stream_media(void);
bool bt_media_is_music_media_active(void);
void bt_media_current_music_set(uint8_t id);
void app_ibrt_sync_mix_prompt_req_handler(uint8_t* ptrParam, uint16_t paramLen);
void app_audio_decode_err_force_trigger(void);
uint8_t app_audio_manager_get_a2dp_codec_type(uint8_t device_id);
uint8_t app_audio_manager_get_a2dp_non_type(uint8_t device_id);
void bt_media_clear_current_media(uint16_t media_type);
void audio_manager_stream_ctrl_start_ble_audio(uint8_t conlid, uint16_t stream_type, uint8_t stream_lid);
void audio_manager_stream_ctrl_stop_ble_audio(uint8_t conlid, uint16_t stream_type, uint8_t stream_lid);
void audio_manager_stream_ctrl_stop_single_ble_audio_stream(uint8_t conlid, uint16_t stream_type, uint8_t stream_lid);
void app_bt_audio_state_checker(void);
#ifdef BLE_WALKIE_TALKIE
void app_audio_manager_register_wt_start_callback(APP_WT_AUDIO_MANAGER_CALLBACK_T cb);
void app_audio_manager_register_wt_stop_callback(APP_WT_AUDIO_MANAGER_CALLBACK_T cb);
void audio_manager_stream_ctrl_start_wt_audio(uint8_t device_id,uint32_t context_type);
void audio_manager_stream_ctrl_stop_wt_audio(uint8_t device_id,uint32_t context_type);
#endif

void app_audio_manager_register_bis_tran_callback(APP_BIS_TRAN_AUDIO_MANAGER_CALLBACK_T start_cb,
                                                  APP_BIS_TRAN_AUDIO_MANAGER_CALLBACK_T stop_cb);

#ifdef __cplusplus
    }
#endif

#endif
