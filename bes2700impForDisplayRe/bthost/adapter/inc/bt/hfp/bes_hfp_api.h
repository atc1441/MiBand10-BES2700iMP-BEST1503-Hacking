/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifndef __BES_HFP_API_H__
#define __BES_HFP_API_H__
#include "hfp_common_define.h"
#ifdef __cplusplus
extern "C" {
#endif

#define APP_BT_SCO_REJECT_REQUEST 0
#define APP_BT_SCO_ACCEPT_REQUEST 1
#define APP_BT_SCO_LET_UPPER_LAYER_HANDLE 2

int bta_hfp_battery_report(uint8_t level);
int bta_hfp_enable_siri_voice(bool en);

uint8_t bes_bt_hfp_has_service_connected(void);
bool bes_bt_hfp_has_sco_connected(void);
bool bes_bt_hfp_has_call_active(void);
uint8_t bes_bt_hfp_has_call_setup(void);
bool bes_bt_hfp_curr_voice_rec_active(void);

#ifdef BT_HFP_AG_ROLE
bt_status_t bes_bt_ag_set_microphone_gain(const bt_bdaddr_t *remote, uint8_t volume);
#endif

bt_status_t bes_bt_hfp_profile_connect(const bt_bdaddr_t *remote);
bt_status_t bes_bt_hfp_profile_disconnect(const bt_bdaddr_t *remote);
bt_status_t bes_bt_hfp_create_audio_link(int device_id);
bt_status_t bes_bt_hfp_disc_audio_link(int device_id);
bt_hfp_state_t bes_bt_hfp_get_state(int device_id);
bt_status_t bes_bt_hfp_call_action(int device_id, BT_HFP_CALL_ACTION_T action);
bt_status_t bes_bt_hfp_dial_number(int device_id, const uint8_t *number, uint16_t len);
bt_status_t bes_bt_hfp_call_hold(int device_id, btif_hf_hold_call_t action, uint8_t index);
bool bes_bt_hfp_is_connecting(const bt_bdaddr_t *remote);
bool bes_bt_hfp_is_initiator(const bt_bdaddr_t *remote);
void bes_bt_hfp_send_at_command(uint8_t device_id, const char* cmd);
void bes_bt_hfp_key_handler(uint8_t hfp_key);
void bes_bt_hfp_set_battery_level(uint8_t level);
void bes_bt_hfp_report_speak_gain(void);
int bes_bt_hfp_battery_report(uint8_t level);
void bes_bt_hfp_send_call_hold_request(uint8_t device_id, btif_hf_hold_call_t action);
void bes_bt_hfp_set_local_vol(int device_id, uint8_t local_volume);
uint8_t bes_bt_hfp_get_local_vol(int device_id);
uint8_t bes_bt_hfp_get_bt_vol(int device_id);
bool bes_bt_hfp_curr_voice_rec_active(void);
uint8_t bes_bt_get_curr_playing_sco(void);
uint32_t bes_bt_switch_hold_background_sco(uint32_t btclk);
bt_status_t bt_hf_start_voice_recognition(const bt_bdaddr_t *bd_addr);
bt_status_t bt_hf_stop_voice_recognition(const bt_bdaddr_t *bd_addr);
void bes_bt_register_hfp_hf_callback(app_hfp_hf_callback_t cb);

#ifdef SUPPORT_SIRI
int bes_bt_hfp_enable_siri_voice(bool en);
#endif

typedef void (*bes_bt_hf_event_cb_t)(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context * ctx);

typedef void (*bes_bt_hfp_aud_discon_report_cb)(uint8_t *value, uint8_t len);

void bes_bt_hfp_user_event_callback_register(bes_bt_hf_event_cb_t cb);

void bes_bt_hfp_user_event_callback_deregister(void);

bes_bt_hf_event_cb_t bes_bt_hfp_get_user_event_callback(void);

void bes_bt_hfp_aud_discon_report_callback_register(bes_bt_hfp_aud_discon_report_cb cb);

void bes_bt_hfp_aud_discon_report_callback_deregister(void);

void bes_bt_hfp_report_user_audio_play_stop_status(void);

bt_status_t bt_hf_volume_control(const bt_bdaddr_t *bd_addr, bt_hf_volume_type_t type, int volume);

#ifdef __cplusplus
}
#endif
#endif /* __BES_HFP_API_H__ */
