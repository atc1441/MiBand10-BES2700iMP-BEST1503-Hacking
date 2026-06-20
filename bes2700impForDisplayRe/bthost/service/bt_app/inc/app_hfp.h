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
#ifndef __APP_HFP_H__
#define __APP_HFP_H__
#include "bluetooth.h"
#include "btapp.h"
#include "hfp_common_define.h"
#ifdef __cplusplus
extern "C" {
#endif

void app_hfp_hf_register_callback(app_hfp_hf_callback_t cb);
void btapp_hfp_report_speak_gain(void);
bool btapp_hfp_mic_need_skip_frame(void);
uint8_t btapp_hfp_need_mute(void);
void hfp_volume_local_set(int id, uint8_t vol);
uint8_t app_bt_hfp_adjust_volume(uint8_t device_id, bool up, bool adjust_local_vol_level);
void app_pts_hfp_siri_voice_enable(void);
void app_pts_hfp_siri_voice_disable(void);
void app_pts_hf_acs_bi_13_i_set_enable(void);
void app_pts_hf_acs_bi_13_i_set_disable(void);
void app_pts_hf_acs_bv_09_i_set_enable(void);
void app_pts_hf_acs_bv_09_i_set_disable(void);
void app_pts_av_create_channel(bt_bdaddr_t *btaddr);
void app_pts_av_set_sink_delay(void);
void app_pts_hf_create_service_link(bt_bdaddr_t *btaddr);
void app_pts_hf_disc_service_link(void);
void app_pts_hf_create_audio_link(void);
void app_pts_hf_disc_audio_link(void);
void app_pts_hf_send_key_pressed(void);
void app_pts_hf_redial_call(void);
void app_pts_hf_dial_number(void);
void app_pts_hf_dial_number_memory_index(void);
void app_pts_hf_dial_number_invalid_memory_index(void);
void app_pts_hf_answer_call(void);
void app_pts_hf_hangup_call(void);
void app_pts_hf_vr_enable(void);
void app_pts_hf_vr_disable(void);
void app_pts_hf_list_current_calls(void);
void app_pts_hf_release_active_call_2(void);
void app_pts_hf_hold_active_call(void);
void app_pts_hf_hold_active_call_2(void);
void app_pts_hf_release_active_call(void);
void app_pts_hf_hold_call_transfer(void);
void app_pts_hf_send_ind_1(void);
void app_pts_hf_send_ind_2(void);
void app_pts_hf_send_ind_3(void);
void app_pts_hf_update_ind_value(void);
void app_pts_hf_report_mic_volume(void);
void app_pts_hf_attach_voice_tag(void);
void app_pts_hf_ind_activation(void);

#ifdef BT_HFP_SUPPORT

#define HF_VOICE_DISABLE  0
#define HF_VOICE_ENABLE   1

#define HF_SENDBUFF_SIZE (320)
#define HF_SENDBUFF_MEMPOOL_NUM (2)
#define APP_BT_HFP_GET_CLCC_TIMEOUT 1500
struct hf_sendbuff_control {
    struct {
        btif_bt_packet_t packet;
        uint8_t buffer[HF_SENDBUFF_SIZE];
    } mempool[HF_SENDBUFF_MEMPOOL_NUM];
    uint8_t index;
};

#ifndef _SCO_BTPCM_CHANNEL_
extern struct hf_sendbuff_control  hf_sendbuff_ctrl;
#endif

int app_hfp_battery_report(uint8_t level);

bt_status_t app_hfp_send_at_command(const char *cmd);

int app_hfp_siri_voice(bool en);

void app_hfp_send_call_hold_request(uint8_t device_id, btif_hf_hold_call_t action);

void app_hfp_report_battery_hf_indicator(uint8_t device_id, uint8_t level);

void app_hfp_report_enhanced_safety(uint8_t device_id, uint8_t value);

void app_bt_hf_send_at_command(uint8_t device_id, const char* at_str);

void app_bt_hf_create_sco_directly(uint8_t device_id);

struct btif_hf_cind_value app_bt_hf_get_cind_service_value(uint8_t device_id);

struct btif_hf_cind_value app_bt_hf_get_cind_call_value(uint8_t device_id);

struct btif_hf_cind_value app_bt_hf_get_cind_callsetup_value(uint8_t device_id);

struct btif_hf_cind_value app_bt_hf_get_cind_callheld_value(uint8_t device_id);

struct btif_hf_cind_value app_bt_hf_get_cind_signal_value(uint8_t device_id);

struct btif_hf_cind_value app_bt_hf_get_cind_roam_value(uint8_t device_id);

struct btif_hf_cind_value app_bt_hf_get_cind_battchg_value(uint8_t device_id);

uint32_t app_bt_hf_get_ag_features(uint8_t device_id);

void app_bt_hf_report_batt_level(btif_hf_channel_t* chan_h, uint8_t level);

void app_bt_hfp_enahnced_battery_report(uint8_t level);

bool app_bt_is_hfp_disconnected(uint8_t device_id);

void app_hfp_init(void);

void app_hfp_set_battery_level(uint8_t level);

int app_hfp_battery_report_reset(uint8_t bt_device_id);

void hfp_speak_volume_handler(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context *ctx);

void btapp_hfp_mic_need_skip_frame_set(int32_t skip_frame);

uint8_t btapp_hfp_incoming_calls(void);

uint8_t btapp_hfp_get_call_setup(void);

bool btapp_hfp_is_pc_call_active(void);

bool btapp_hfp_is_call_active(void);

bool btapp_hfp_is_sco_active(void);

#ifdef __INTERCONNECTION__
uint8_t ask_is_selfdefined_battery_report_AT_command_support(void);

uint8_t send_selfdefined_battery_report_AT_command(uint8_t device_id);
#endif

bool app_hfp_curr_audio_up(btif_hf_channel_t* hfp_chnl);

void app_hfp_bt_driver_callback(uint8_t device_id, hf_event_t event);

int app_hfp_siri_voice(bool en);
uint8_t app_bt_hf_get_reject_dev(void);

void app_bt_hf_set_reject_dev(uint8_t device_id);

void app_hfp_restore_master_local_volume(void);

void hfp_ibrt_sync_status_sent_callback(void);

bt_status_t app_hf_send_current_call_at_commond(btif_hf_channel_t* chan_h);

bool app_hfp_check_whether_need_to_mute_hfp(void);

void app_bt_audio_hfp_recon_timeout_handler(void const *param);
#ifdef IBRT
int hfp_ibrt_sync_get_status(uint8_t device_id,ibrt_hfp_status_t *hfp_status);
int hfp_ibrt_sync_set_status(uint8_t device_id,ibrt_hfp_status_t *hfp_status);
int hfp_ibrt_service_connected_mock(uint8_t device_id);
int hfp_ibrt_sco_audio_disconnected(void);
int hfp_ibrt_sco_audio_connected(hfp_sco_codec_t codec, uint16_t sco_connhdl);
#endif
#endif /* BT_HFP_SUPPORT */
#ifdef __cplusplus
}
#endif
#endif /*__APP_HFP_H__*/

