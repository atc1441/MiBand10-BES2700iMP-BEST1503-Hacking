/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifndef __APP_BT_SOURCE_H__
#define __APP_BT_SOURCE_H__
#include "a2dp_api.h"
#include "avrcp_api.h"
#include "app_a2dp_source.h"
#include "app_avrcp_target.h"
#include "app_hfp_ag.h"
#include "btapp.h"
#include "hal_aud.h"
#include "app_bt.h"
#include "source_common_define.h"

#ifdef __cplusplus
extern "C" {
#endif

struct BT_SOURCE_DEVICE_T {
    struct BT_DEVICE_T *base_device;
    enum AUD_SAMPRATE_T aud_sample_rate;
    uint32_t aud_bit_rate;
    bool prev_packet_is_pending;
    bool a2dp_paused_by_sink;
    bool a2dp_paused_by_ag_sco;
    bool is_lossless_on;
};

struct bt_source_config {
    bool read_snk_or_src_from_nv;
    bool bt_source_enable;
    bool ag_standalone;
    bool ag_enable;
    bool av_enable;
};

struct BT_SOURCE_MANAGER_T {
    struct bt_source_config config;
    struct BT_SOURCE_DEVICE_T devices[BT_SOURCE_DEVICE_NUM];
    bool a2dp_source_input_on;
    uint8_t curr_source_a2dp_id;
    uint8_t curr_source_playing_a2dp;
    uint8_t curr_source_hfp_id;
    uint8_t curr_source_playing_sco;
    struct list_node codec_packet_list;
    struct list_node encoded_packet_list;
};

extern struct BT_SOURCE_MANAGER_T bt_source_manager;

extern struct BT_SOURCE_DEVICE_T *app_bt_source_get_device(int i);

extern struct BT_SOURCE_DEVICE_T *app_bt_source_find_device(bt_bdaddr_t *remote);

uint8_t app_bt_source_count_connected_device(void);

bool app_bt_source_has_streaming_a2dp(void);

bool app_bt_source_has_streaming_sco(void);

uint8_t app_bt_source_count_streaming_sco(void);

uint8_t app_bt_source_count_streaming_a2dp(void);

uint8_t app_bt_source_count_streaming_aac(void);

uint8_t app_bt_source_count_streaming_lhdc(void);

uint8_t app_bt_source_count_streaming_lhdcv5(void);

uint8_t app_bt_source_count_streaming_ldac(void);

uint8_t app_bt_source_count_streaming_sbc(void);

uint8_t app_bt_source_get_streaming_a2dp(void);

uint8_t app_bt_source_get_current_a2dp(void);

struct BT_SOURCE_DEVICE_T *app_bt_source_get_current_a2dp_device(void);

uint8_t app_bt_source_get_streaming_sco(void);

uint8_t app_bt_source_get_current_hfp(void);

struct BT_SOURCE_DEVICE_T *app_bt_source_get_current_hfp_device(void);

void app_bt_source_init(void);

bool app_bt_source_nv_snk_or_src_enabled(void);

void app_bt_source_set_connectable_state(bool enable);

void app_bt_source_search_device(void);

void app_bt_report_source_link_connected(btif_remote_device_t *dev, uint8_t errcode);

void app_bt_report_source_link_disconnected(const bt_bdaddr_t *remote, uint8_t errcode);

void app_bt_source_disconnect_all_connections(bool power_off);

void app_bt_source_disconnect_mobile_connections(void);

void bt_source_register_callback(bt_source_callback_t cb);

void bt_source_reconnect_hfp_profile(const bt_bdaddr_t *remote);

void bt_source_reconnect_a2dp_profile(const bt_bdaddr_t *remote);

void bt_source_reconnect_avrcp_profile(const bt_bdaddr_t *remote);

#if defined(BT_HID_DEVICE)
void bt_source_reconnect_hid_profile(const bt_bdaddr_t *remote);
#endif // BT_HID_DEVICE

void bt_source_perform_profile_reconnect(const bt_bdaddr_t *remote);

void bt_source_create_audio_link(const bt_bdaddr_t *remote);

void bt_source_disc_audio_link(const bt_bdaddr_t *remote);

#if defined(A2DP_SOURCE_TEST) || defined(HFP_AG_TEST)
void app_bt_source_set_source_addr(uint8_t *addr);
#endif

enum app_bt_source_audio_event_t {
    APP_BT_SOURCE_AUDIO_EVENT_HF_SCO_CONNECTED = 0x01,
    APP_BT_SOURCE_AUDIO_EVENT_HF_SCO_DISCONNECTED,
    APP_BT_SOURCE_AUDIO_EVENT_SNK_STREAM_START,
    APP_BT_SOURCE_AUDIO_EVENT_SNK_STREAM_SUSPEND,
    APP_BT_SOURCE_AUDIO_EVENT_AG_SERVICE_CONNECTED = 0x10,
    APP_BT_SOURCE_AUDIO_EVENT_AG_SERVICE_DISCONNECTED,
    APP_BT_SOURCE_AUDIO_EVENT_AG_SCO_CONNECTED,
    APP_BT_SOURCE_AUDIO_EVENT_AG_SCO_DISCONNECTED,
    APP_BT_SOURCE_AUDIO_EVENT_SRC_CONNECT_FAIL,
    APP_BT_SOURCE_AUDIO_EVENT_SRC_STREAM_OPEN,
    APP_BT_SOURCE_AUDIO_EVENT_SRC_STREAM_CLOSE,
    APP_BT_SOURCE_AUDIO_EVENT_SRC_STREAM_START,
    APP_BT_SOURCE_AUDIO_EVENT_SRC_STREAM_SUSPEND,
};

struct app_bt_source_audio_event_param_t {
    union {
        struct {
            bt_bdaddr_t *addr;
            int reason;
        } a2dp_source_connect_fail;
    } p;
};

void app_bt_source_audio_event_handler(uint8_t device_id, enum app_bt_source_audio_event_t event, struct app_bt_source_audio_event_param_t *param);
void app_bt_source_set_hfp_ag_pts_enable(bool enable);
void app_bt_source_set_hfp_ag_pts_esc_01_enable(bool enable);
void app_bt_source_set_hfp_ag_pts_esc_02_enable(bool enable);
void app_bt_source_set_hfp_ag_pts_ecc_enable(bool enable);
void app_bt_source_set_source_pts_get_all_cap_flag(bool enable);
void app_bt_source_set_source_pts_suspend_err_flag(bool enable);
void app_bt_source_set_source_pts_unknown_cmd_flag(bool enable);

typedef void (*bt_source_event_global_cb_t)(const btif_event_t * event);
typedef void (*bt_source_audio_coex_event_cb_t)(bt_bdaddr_t *addr,
                                enum app_bt_source_audio_event_t event,
                                struct app_bt_source_audio_event_param_t *event_param);

void app_bt_source_register_coex_global_event_handle(bt_source_event_global_cb_t func);

void app_bt_source_register_coex_audio_event_handler(bt_source_audio_coex_event_cb_t func);

void app_bt_source_coex_global_event_ind(const btif_event_t * event);

void app_bt_source_coex_audio_event_ind(bt_bdaddr_t *addr, enum app_bt_source_audio_event_t event,
                                    struct app_bt_source_audio_event_param_t *event_param);
#ifdef __cplusplus
}
#endif

#endif /* __APP_BT_SOURCE_H__ */

