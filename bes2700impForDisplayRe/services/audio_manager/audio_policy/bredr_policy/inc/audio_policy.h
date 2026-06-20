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
#ifndef __APP_BT_AUDIO_POLICY_H__
#define __APP_BT_AUDIO_POLICY_H__

#include "bluetooth.h"
#include "me_common_define.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    HF_FOCUS_MACHINE_CURRENT_IDLE                        = 0x00,
    HF_FOCUS_MACHINE_CURRENT_INCOMING                    = 0x01,
    HF_FOCUS_MACHINE_CURRENT_OUTGOING                    = 0x02,
    HF_FOCUS_MACHINE_CURRENT_CALLING                     = 0x03,
    HF_FOCUS_MACHINE_CURRENT_3WAY_INCOMING               = 0x04,
    HF_FOCUS_MACHINE_CURRENT_3WAY_HOLD_CALING            = 0x05,
    HF_FOCUS_MACHINE_CURRENT_IDLE_ANOTHER_IDLE           = 0x06,
    HF_FOCUS_MACHINE_CURRENT_INCOMMING_ANOTHER_INCOMMING = 0x07,
    HF_FOCUS_MACHINE_CURRENT_INCOMMING_ANOTHER_CALLING   = 0x08,
    HF_FOCUS_MACHINE_CURRENT_INCOMMING_ANOTHER_OUTGOING  = 0x09,
    HF_FOCUS_MACHINE_CURRENT_OUTGOING_ANOTHER_INCOMMING  = 0x0A,
    HF_FOCUS_MACHINE_CURRENT_OUTGOING_ANOTHER_OUTGOING   = 0x0B,
    HF_FOCUS_MACHINE_CURRENT_OUTGOING_ANOTHER_CALLING    = 0x0C,
    HF_FOCUS_MACHINE_CURRENT_CALLING_ANOTHER_INCOMING    = 0x0D,
    HF_FOCUS_MACHINE_CURRENT_CALLING_ANOTHER_OUTGOING    = 0x0E,
    HF_FOCUS_MACHINE_CURRENT_CALLING_ANOTHER_CALLING     = 0x0F,
    HF_FOCUS_MACHINE_CURRENT_CALLING_ANOTHER_HOLD        = 0x10,
    HF_FOCUS_MACHINE_CURRENT_HOLD_ANOTHER_ACTIVE         = 0x11,
    HF_FOCUS_MACHINE_CURRENT_ACTIVE_ANOTHER_HOLD         = 0x12,
}HF_CALL_FOCUS_MACHINE_T;

struct app_bt_config;

#define CHECK_A2DP_RESTREAMING_TIME    (3000)

typedef struct
{
    bool time_valid;
    uint32_t time_of_sco_preempted;
}sco_preempted_time_t;

typedef bool (*local_stream_status_changed)(const bt_bdaddr_t *addr, uint8_t state);
typedef bool (*app_bt_audio_ui_allow_resume_request)(bt_bdaddr_t *bdaddr);

void app_bt_audio_register_local_stream_callback(local_stream_status_changed cb);

void app_bt_init_config_postphase(struct app_bt_config *config);

bool app_bt_audio_curr_a2dp_data_need_receive(uint8_t device_id);

int app_bt_audio_event_handler(uint8_t device_id, enum app_bt_audio_event_t event, uint32_t data);

uint32_t app_bt_audio_create_new_prio(void);

uint8_t app_bt_audio_get_curr_a2dp_device(void);

uint8_t app_bt_audio_get_curr_hfp_device(void);

uint8_t app_bt_audio_get_curr_sco_device(void);

uint8_t app_bt_audio_get_hfp_device_for_user_action(void);

uint8_t app_bt_audio_get_low_prio_sco_device(uint8_t device_id);

uint8_t app_bt_audio_get_device_for_user_action(void);

uint8_t app_bt_audio_get_another_hfp_device_for_user_action(uint8_t curr_device_id);

uint8_t app_bt_audio_get_curr_playing_a2dp(void);

uint8_t app_bt_audio_count_connected_hfp(void);

uint8_t app_bt_audio_get_curr_playing_sco(void);

uint8_t app_bt_audio_select_connected_a2dp(void);

uint8_t app_bt_audio_select_another_streaming_a2dp(uint8_t curr_device_id);

uint8_t app_bt_audio_select_another_device_to_create_sco(uint8_t curr_device_id);

uint8_t app_bt_audio_select_another_call_active_hfp(uint8_t curr_device_id);

uint8_t app_bt_audio_select_call_active_hfp(void);

uint8_t app_bt_audio_count_streaming_a2dp(void);

uint8_t app_bt_audio_select_connected_device(void);

uint8_t app_bt_audio_count_connected_a2dp(void);

uint8_t app_bt_audio_count_connected_sco(void);

bool app_bt_get_if_sco_audio_connected(uint8_t curr_device_id);

uint8_t app_bt_audio_count_straming_mobile_links(void);

uint8_t app_bt_audio_count_connected_hfp(void);

uint8_t app_bt_audio_select_streaming_sco(void);

uint8_t app_bt_audio_select_bg_a2dp_to_resume(void);

void app_bt_audio_set_bg_a2dp_device(bool is_paused_bg_device, bt_bdaddr_t *bdaddr);

void app_bt_audio_enable_active_link(bool enable, uint8_t active_id);

void app_bt_audio_switch_streaming_sco(void);

void app_bt_audio_a2dp_resume_this_profile(uint8_t device_id);

bool app_bt_audio_switch_streaming_a2dp(void);

bool app_audio_toggle_a2dp_cis(void);

void app_bt_audio_recheck_a2dp_streaming(void);

void app_bt_audio_rechceck_kill_another_connected_a2dp(uint8_t device_id);

bool app_bt_audio_a2dp_disconnect_self_check(uint8_t device_id);

uint32_t app_bt_audio_trigger_switch_streaming_a2dp(uint32_t btclk);

uint32_t app_audio_trigger_toggle_a2dp_cis(uint32_t btclk);

void app_bt_audio_new_sco_is_rejected_by_controller(uint8_t device_id);

void app_bt_audio_receive_peer_a2dp_playing_device(bool is_response, uint8_t device_id);

void app_bt_audio_peer_sco_codec_received(uint8_t device_id);

void app_bt_audio_select_a2dp_behavior_handler(uint8_t device_id);

void app_bt_audio_init_focus_listener(void* device);

void app_bt_audio_self_ring_tone_play_timer_handler(void const *param);

void app_bt_audio_stop_self_ring_tone_play(void* device, bool stop_all);

int app_bt_audio_handle_sco_req(uint8_t device_id);

void app_bt_audio_handle_sco_connected_event(uint8_t device_id);

void app_bt_audio_handle_hfp_callsetup_event(uint8_t device_id);

void app_bt_audio_handle_hfp_call_event(uint8_t device_id);

void app_bt_audio_handle_sco_disconnect_event(uint8_t device_id);

void app_bt_start_audio_focus_timer(uint8_t device_id);

void app_bt_stop_audio_focus_timer(uint8_t device_id);

void app_bt_audio_abandon_focus_timer_handler(void const *param);

void app_bt_switch_to_prompt_a2dp_mode(void);

void app_bt_audio_stop_a2dp_playing(uint8_t device_id);

void app_bt_ibrt_audio_pause_a2dp_stream(uint8_t device_id);

void app_bt_ibrt_audio_play_a2dp_stream(uint8_t device_id);

void app_bt_audio_a2dp_switch_trigger(void);

void app_bt_start_audio_focus_timer(uint8_t device_id);

void app_bt_stop_audio_focus_timer(uint8_t device_id);

void app_bt_audio_abandon_focus_timer_handler(void const *param);

void app_bt_select_hfp_behavior_handler(uint8_t device_id);

uint32_t app_bt_audio_trigger_switch_mute_streaming_sco(uint32_t btclk);

bool app_bt_audio_sco_switch_trigger(void);

bool app_bt_audio_allow_send_profile_immediate(void *remote);

void app_audio_toggle_a2dp_cis_trigger(void);

void app_bt_audio_a2dp_switch_trigger(void);

uint8_t app_audio_hfp_machine_convert(uint8_t device_id);

HF_CALL_FOCUS_MACHINE_T app_audio_get_current_hfp_machine();

bool app_bt_audio_current_device_is_hfp_idle_state(uint8_t device_id);

void app_bt_audio_ui_allow_resume_a2dp_register(app_bt_audio_ui_allow_resume_request allow_resume_request_cb);

void app_bt_audio_strategy_init(void);

void app_bt_audio_strategy_init(void);

void app_bt_audio_hfp_get_clcc_timeout_handler(void const *param);

void app_bt_handle_profile_exchanged_done(void);

bool app_bt_audio_both_profile_exchanged(void);

bool app_bt_audio_handle_peer_focus_info(uint8_t *buff, uint16_t length);

uint8 app_bt_audio_a2dp_close_req_allow(uint8_t device_id);

void app_bt_audio_switch_a2dp_cmp_cb_init(void (*a2dp_switch_cb)(uint8_t device_id));

void app_audio_toggle_a2dp_cis_cmp_cb_init(void (*a2dp_cis_toggle_cb)(uint8_t device_id));

void app_bt_stop_check_a2dp_restreaming_timer(uint8_t device_id);

void app_bt_start_check_a2dp_restreaming_timer(uint8_t device_id, uint32_t timeout);

void app_bt_stop_delay_play_a2dp_timer(uint8_t device_id);

void app_bt_start_delay_play_a2dp_timer(uint8_t device_id, uint32_t timeout);

bool app_bt_audio_judge_is_pc_bydevid(uint8 devid);

#ifdef __cplusplus
}
#endif

#endif /* __APP_BT_AUDIO_POLICY_H__ */

