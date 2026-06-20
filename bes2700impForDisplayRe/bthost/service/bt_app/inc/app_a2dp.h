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
#ifndef __APP_A2DP_H__
#define __APP_A2DP_H__
#include "btapp.h"
#if defined(__cplusplus)
extern "C" {
#endif

typedef enum
{
    APP_A2DP_PROFILE_STATE_CONNECTING = 0x01,
    APP_A2DP_PROFILE_STATE_CONNECTED = 0x02,
    APP_A2DP_PROFILE_STATE_DISCONNECTING = 0x03,
    APP_A2DP_PROFILE_STATE_DISCONNECTED = 0x04
} app_a2dp_conn_state;

typedef enum
{
    APP_A2DP_STATE_NOT_PLAYING = 0x00,
    APP_A2DP_STATE_PLAYING = 0x01,
} app_a2dp_play_state;

typedef enum
{
    APP_A2DP_EVENT_CONNECTION_STATE = 0x00,
    APP_A2DP_EVENT_PLAY_STATUS = 0x01,
} app_a2dp_event_e;

typedef void (*media_info_report_t)(const bt_bdaddr_t *addr, const avrcp_adv_rsp_parms_t *mediaPkt);

const char* avrcp_get_track_element_name(uint32_t element_id);

void app_a2dp_register_custom_allow_receive_steam(bool (*cb)(void));

uint8_t app_bt_a2dp_adjust_volume(uint8_t device_id, bool up, bool adjust_local_vol_level);

void app_avrcp_reg_media_info_report_callback(media_info_report_t cb);

void avrcp_callback_CT(uint8_t device_id, btif_avrcp_channel_t* btif_avrcp, const avrcp_callback_parms_t* parms);

#ifdef BT_SOURCE
void a2dp_source_callback(uint8_t device_id, a2dp_stream_t *Stream, const a2dp_callback_parms_t *info);
#endif

#define MAX_A2DP_VOL   (127)
#define APP_A2DP_REJECT_SNIFF_TIMEOUT (10000)

#ifdef BT_AVRCP_SUPPORT
void avrcp_init(void);
#endif

#ifdef BT_A2DP_SUPPORT
void a2dp_init(void);

bool a2dp_is_music_ongoing(void);

void app_avrcp_get_capabilities_start(int device_id);

void app_a2dp_bt_driver_callback(uint8_t device_id, btif_a2dp_event_t event);

void app_bt_a2dp_disable_aac_codec(bool disable);

/**
 * @brief Modify the element or status of the specified codec
 *
 * @param  action @see codec_act_param_t
 *
 * @return void
 */
void app_bt_a2dp_set_codec_param_handle(codec_act_param_t * action);

void app_bt_a2dp_disable_sbc_codec(bool disable);

void app_bt_a2dp_disable_vendor_codec(bool disable);

bool app_bt_a2dp_report_current_volume(int device_id);

void btapp_a2dp_report_speak_gain(void);

void app_bt_a2dp_reconfig_to_aac(a2dp_stream_t *stream);

void app_bt_a2dp_reconfig_to_vendor_codec(a2dp_stream_t *stream, uint8_t codec_id, uint8_t a2dp_non_type);

void app_bt_a2dp_reconfig_to_sbc(a2dp_stream_t *stream);

uint8_t a2dp_convert_local_vol_to_bt_vol(uint8_t localVol);

void app_bt_a2dp_reconfig_codec(a2dp_stream_t *stream, uint8_t code_type);

#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)
bool a2dp_lhdc_get_ext_flags(uint32_t flags);
uint8_t a2dp_lhdc_config_llc_get(void);
#endif

uint8_t a2dp_get_non_type_by_device_id(uint8_t device_id);

#ifdef __TENCENT_VOICE__
uint8_t avrcp_get_media_status(void);
void avrcp_set_media_status(uint8_t status);
#endif
void app_a2dp_reject_sniff_start(uint8_t device_id, uint32_t timeout);

#ifdef IBRT
int a2dp_ibrt_session_new(uint8_t devId);
int a2dp_ibrt_session_set(uint8_t session,uint8_t devId);
uint32_t a2dp_ibrt_session_get(uint8_t devId);

typedef struct ibrt_a2dp_status_t ibrt_a2dp_status_t;
int a2dp_ibrt_sync_get_status(uint8_t device_id, ibrt_a2dp_status_t *a2dp_status);
int a2dp_ibrt_sync_set_status(uint8_t device_id, ibrt_a2dp_status_t *a2dp_status);
#endif
int a2dp_bt_stream_open_mock(uint8_t device_id, bt_bdaddr_t *remote);
#endif /* BT_A2DP_SUPPORT */

#if defined(BT_AVRCP_SUPPORT) || defined(BT_A2DP_SUPPORT)
bool app_bt_a2dp_send_volume_change(int device_id);
#endif

int a2dp_bt_stream_need_autotrigger_getandclean_flag(void);

bool app_bt_is_a2dp_disconnected(uint8_t device_id);

void app_pts_av_disc_channel(void);

void app_pts_av_close_channel(void);

#ifdef A2DP_STREAM_DETECT_NO_DECODE
typedef enum {
    MUTE_TIME,
    NORMAL_TIME,
} time_flag_t;

void app_a2dp_stream_detect_time_init(uint8_t dev_id);

// time: ms
void app_a2dp_stream_set_detect_time(uint8_t dev_id, codec_type_t codec, time_flag_t time_type, uint16_t time);

#endif // A2DP_STREAM_DETECT_NO_DECODE

#ifdef __cplusplus
}
#endif
#endif
