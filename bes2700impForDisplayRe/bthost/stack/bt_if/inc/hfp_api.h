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
#ifndef __HFP_API_H__
#define __HFP_API_H__
#include "bluetooth.h"
#include "conmgr_api.h"
#include "hfp_common_define.h"
#include "hci_api.h"
#include "me_api.h"
#ifdef BT_HFP_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif

#define HF_CHANNEL_NUM (BT_DEVICE_NUM+BT_SOURCE_DEVICE_NUM)

typedef uint16_t hf_gateway_version;

/* Unable to determine the Hands Free Profile version that is supported */
#define BTIF_HF_GW_VERSION_UNKNOWN 0x0000

/* Supports Version 0.96 of the Hands Free Profile */
#define BTIF_HF_GW_VERSION_0_96    0x0100

/* Supports Version 1.0 of the Hands Free Profile */
#define BTIF_HF_GW_VERSION_1_0     0x0101

/* Supports Version 1.5 of the Hands Free Profile */
#define BTIF_HF_GW_VERSION_1_5     0x0105

#define BTIF_HF_GW_VERSION_1_6     0x0106

#define BTIF_HF_GW_VERSION_1_7     0x0107

/* End of hf_gateway_version */

struct hfp_vendor_info {
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t version_id;
    uint16_t feature_id;  //default is 3, if you do not known, set it 0
};

typedef struct {
    uint8_t device_id;
    struct bdaddr_t *remote;
    // 0: connect_fail or disconnected, 1:connected
    uint8_t connected;
    // state = BTM_CONN_SCO_OPENED, set this param
    uint8_t* connected_codec;
} hfp_set_sco_state_param_t;

typedef void (*hf_event_cb_t) (uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context * ctx);

struct btif_hf_cind_value {
    uint8_t index;
    uint8_t min;
    uint8_t max;
    uint8_t value;
    bool initial_support;
};

struct hshf_control;
struct _hshf_channel;

struct btif_hf_cind_value btif_hf_get_cind_service_value(btif_hf_channel_t* chan_h);
struct btif_hf_cind_value btif_hf_get_cind_call_value(btif_hf_channel_t* chan_h);
struct btif_hf_cind_value btif_hf_get_cind_callsetup_value(btif_hf_channel_t* chan_h);
struct btif_hf_cind_value btif_hf_get_cind_callheld_value(btif_hf_channel_t* chan_h);
struct btif_hf_cind_value btif_hf_get_cind_signal_value(btif_hf_channel_t* chan_h);
struct btif_hf_cind_value btif_hf_get_cind_roam_value(btif_hf_channel_t* chan_h);
struct btif_hf_cind_value btif_hf_get_cind_battchg_value(btif_hf_channel_t* chan_h);
uint32_t btif_hf_get_ag_features(btif_hf_channel_t *chan_h);
void app_ibrt_register_sco_link(uint8_t device_id, bt_bdaddr_t *remote);

void btif_register_tws_current_ibrt_slave_role_callback(bool (*cb)(void* addr));
struct _hshf_channel *btif_hfp_get_channel(int *count);
void btif_hf_set_sco_path_status(hfp_set_sco_state_param_t *param);

int btif_hfp_initialize(void);
int btif_hf_register_callback(hf_event_cb_t callback);
int btif_ag_register_callback(hf_event_cb_t callback);
hf_gateway_version btif_hf_get_version(btif_hf_channel_t* chan_h);
bt_status_t btif_hf_update_indicators_batt_level(btif_hf_channel_t* chan_h, uint32_t level);
bt_status_t btif_hf_batt_report(btif_hf_channel_t* chan_h, uint8_t level);
bt_status_t btif_hf_report_enhanced_safety(btif_hf_channel_t* chan_h, uint8_t value);
bt_status_t btif_hf_enable_voice_recognition(btif_hf_channel_t* chan_h, bool en);
bt_status_t btif_hf_batt_report(btif_hf_channel_t* chan_h, uint8_t level);
bool btif_hf_is_voice_rec_active(btif_hf_channel_t* chan_h);
bt_status_t btif_hf_disable_nrec(btif_hf_channel_t* chan_h);
bt_status_t btif_hf_report_speaker_volume(btif_hf_channel_t* chan_h, uint8_t gain);
bt_status_t btif_hf_report_mic_volume(btif_hf_channel_t* chan_h, uint8_t gain);
bt_status_t btif_hf_attach_voice_tag(btif_hf_channel_t* chan_h);
bt_status_t btif_hf_ind_activation(btif_hf_channel_t* chan_h);
bt_status_t btif_hf_send_at_cmd(btif_hf_channel_t* chan_h, const char *at_str);
bt_status_t btif_hf_list_current_calls(btif_hf_channel_t* chan_h);
bool btif_hf_is_hf_indicators_support(btif_hf_channel_t* chan_h);
bool btif_hf_is_batt_report_support(btif_hf_channel_t* chan_h);
bool btif_hf_is_sco_wait_codec_sync(btif_hf_channel_t* chan_h);
void btif_hf_set_negotiated_codec(btif_hf_channel_t* chan_h, hfp_sco_codec_t codec);
hfp_sco_codec_t btif_hf_get_negotiated_codec(btif_hf_channel_t* chan_h);
bt_status_t btif_hf_answer_call(btif_hf_channel_t* chan_h);
bt_status_t btif_hf_hang_up_call(btif_hf_channel_t* chan_h);
bt_status_t btif_hf_redial_call(btif_hf_channel_t* chan_h);
bt_status_t btif_hf_dial_number(btif_hf_channel_t* chan_h, uint8_t *number, uint16_t len);
bt_status_t btif_hf_dial_memory(btif_hf_channel_t* chan_h, int location);
bt_status_t btif_hf_call_action(int device_id, BT_HFP_CALL_ACTION_T action);
bt_status_t btif_hf_disc_audio_link(btif_hf_channel_t* chan_h);
bt_status_t btif_hf_create_audio_link(btif_hf_channel_t* chan_h);
bt_status_t btif_hf_create_sco_directly(btif_hf_channel_t* chan_h);
bt_status_t btif_hf_call_hold(btif_hf_channel_t* chan_h, btif_hf_hold_call_t action, uint8_t index);
bt_status_t btif_hf_switch_calls(btif_hf_channel_t* hangup_chan_h,
                                 btif_hf_channel_t* answer_chan_h);
btif_hf_channel_t* btif_get_hf_chan_by_address(bt_bdaddr_t *bdaddr);
uint8_t btif_get_hf_chan_audio_up_flag(btif_hf_channel_t* chan_h);
bt_hfp_chan_state_t btif_get_hf_chan_state(btif_hf_channel_t* chan_h);
bool btif_hf_check_AudioConnect_status(btif_hf_channel_t* chan_h);
btif_hf_channel_t* btif_hf_alloc_channel(void);
btif_hf_channel_t* btif_ag_alloc_channel(void);
int btif_hf_init_channel(btif_hf_channel_t* chan_h);
bt_status_t btif_hf_disconnect_service_link(btif_hf_channel_t* chan_h);
bt_status_t btif_hf_create_service_link(bt_bdaddr_t * bt_addr);
void btif_hf_query_remote_sdp(bt_bdaddr_t *addr);
bool btif_hfp_profile_is_connecting(uint8_t device_id);
bool btif_hf_get_remote_bdaddr(btif_hf_channel_t* chan_h, bt_bdaddr_t *bdaddr_p);
void btif_hfp_register_peer_sco_codec_receive_handler(void (*cb)(uint8_t device_id,void * chan,uint8_t codec));
void btif_hf_receive_peer_sco_codec_info(const void* remote, uint8_t codec);
uint16_t btif_hf_get_sco_hcihandle(btif_hf_channel_t* chan_h);
btif_hci_handle_t btif_hf_get_remote_hci_handle(btif_hf_channel_t* chan_h);
bool btif_hf_is_acl_connected(btif_hf_channel_t* chan_h);
struct hshf_control* _hshf_get_control_from_id(uint8_t device_id);
btif_remote_device_t *btif_hf_cmgr_get_remote_device(btif_hf_channel_t* chan_h);
bool btif_hf_check_rfcomm_l2cap_channel_is_creating(bt_bdaddr_t *bdaddr);
btif_remote_device_t *btif_hf_cmgr_get_remote_device(btif_hf_channel_t* chan_h);
bt_status_t btif_hf_send_audio_data(btif_hf_channel_t* chan_h, btif_bt_packet_t *packet);
bt_status_t btif_hf_is_inbandring_enabled(btif_hf_channel_t* chan_h);
bool btif_hfp_is_profile_initiator(const bt_bdaddr_t* remote);
bool btif_hfp_profile_connecting(const bt_bdaddr_t *bdaddr_p);
bool btif_hf_is_virtual_call_enabled(btif_hf_channel_t* chan_h);
void btif_hf_set_virtual_call_enable(btif_hf_channel_t* chan_h);
void btif_hf_set_virtual_call_disable(btif_hf_channel_t* chan_h);
#ifdef IBRT
uint32_t btif_hfp_profile_save_ctx(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);
uint32_t btif_hfp_profile_restore_ctx(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);
bt_status_t btif_hf_sync_conn_audio_connected(hfp_sco_codec_t codec,uint16_t conhdl);
bt_status_t btif_hf_sync_conn_audio_disconnected(uint16_t conhdl);
bt_status_t btif_hfp_force_disconnect_hfp_profile(uint8_t device_id,uint8_t reason);
void btif_hfp_ibrt_role_switch_handle(const bt_bdaddr_t *remote);
#endif
bt_status_t btif_hf_indicators_1(btif_hf_channel_t* chan_h);
bt_status_t btif_hf_indicators_2(btif_hf_channel_t* chan_h);
bt_status_t btif_hf_indicators_3(btif_hf_channel_t* chan_h);
bt_status_t btif_hf_send_key_pressed(btif_hf_channel_t* chan_h);

typedef void(*hfp_hold_result_callback)(uint8_t device_id, uint8_t result);
void btif_hfp_hold_result_callback(hfp_hold_result_callback cb);

#ifdef BT_HFP_AG_ROLE
struct btif_ag_call_info
{
    uint8_t direction; // 0 outgoing, 1 incoming
    uint8_t state; // 0 active, 1 held, 2 outgoing dialing, 3 outgoing alerting, 4 incoming, 5 waiting, 6 held by Response and Hold
    uint8_t mode; // 0 voice, 1 data, 2 fax
    uint8_t multiparty; // 0 is not one of multiparty call parties, 1 is one of.
    const char* number; // calling number, optional
};

typedef int (*btif_ag_handler)(btif_hf_channel_t *chan);
typedef int (*btif_ag_handler_int)(btif_hf_channel_t *chan, int n);
typedef int (*btif_ag_handler_str)(btif_hf_channel_t *chan, const char* s);
typedef int (*btif_ag_iterate_call_handler)(btif_hf_channel_t *chan, struct btif_ag_call_info* out);
typedef const char* (*btif_ag_query_operator_handler)(btif_hf_channel_t *chan);

struct btif_ag_module_handler
{
    btif_ag_handler answer_call;
    btif_ag_handler hungup_call;
    btif_ag_handler dialing_last_number;
    btif_ag_handler release_held_calls;
    btif_ag_handler release_active_and_accept_calls;
    btif_ag_handler hold_active_and_accept_calls;
    btif_ag_handler add_held_call_to_conversation;
    btif_ag_handler connect_remote_two_calls;
    btif_ag_handler disable_mobile_nrec;
    btif_ag_handler_int release_specified_active_call;
    btif_ag_handler_int hold_all_calls_except_specified_one;
    btif_ag_handler_int hf_battery_change; /* battery level 0 ~ 100 */
    btif_ag_handler_int hf_spk_gain_change; /* speaker gain 0 ~ 15 */
    btif_ag_handler_int hf_mic_gain_change; /* mic gain 0 ~ 15 */
    btif_ag_handler_int transmit_dtmf_code;
    btif_ag_handler_int memory_dialing_call;
    btif_ag_handler_str dialing_call;
    btif_ag_handler_str handle_at_command;
    btif_ag_query_operator_handler query_current_operator;
    btif_ag_iterate_call_handler iterate_current_call;
};

uint32_t btif_ag_get_hf_features(btif_hf_channel_t *chan_h);
bool btif_get_hf_is_ag_role(btif_hf_channel_t* chan_h);
bool btif_ag_cmee_enabled(btif_hf_channel_t *chan_h);
bt_ag_ind_status_t btif_ag_get_ind_status(btif_hf_channel_t *chan_h);
void btif_ag_set_ind_status(btif_hf_channel_t *chan_h, const bt_ag_ind_status_t *status);
bt_status_t btif_ag_set_curr_at_upper_handle(btif_hf_channel_t* chan_h);
bt_status_t btif_ag_create_service_link(btif_hf_channel_t* chan_h, bt_bdaddr_t * bt_addr);
bt_status_t btif_ag_disconnect_service_link(btif_hf_channel_t* chan_h);
bt_status_t btif_ag_create_audio_link(btif_hf_channel_t* chan_h);
bt_status_t btif_ag_disc_audio_link(btif_hf_channel_t* chan_h);
bt_status_t btif_ag_send_service_status(btif_hf_channel_t* chan_h, bool enabled);
bt_status_t btif_ag_send_call_active_status(btif_hf_channel_t* chan_h, bool active);
bt_status_t btif_ag_send_callsetup_status(btif_hf_channel_t* chan_h, uint8_t status);
bt_status_t btif_ag_send_callheld_status(btif_hf_channel_t* chan_h, uint8_t status);
bt_status_t btif_ag_send_mobile_signal_level(btif_hf_channel_t* chan_h, uint8_t level);
bt_status_t btif_ag_send_mobile_roam_status(btif_hf_channel_t* chan_h, bool enabled);
bt_status_t btif_ag_send_mobile_battery_level(btif_hf_channel_t* chan_h, uint8_t level);
bt_status_t btif_ag_send_calling_ring(btif_hf_channel_t* chan_h, const char* number);
bt_status_t btif_ag_set_speaker_gain(btif_hf_channel_t* chan_h, uint8_t volume);
bt_status_t btif_ag_set_inband_ring_tone(btif_hf_channel_t* chan_h, bool enabled);
bt_status_t btif_ag_set_last_dial_number(btif_hf_channel_t* chan_h, bool enabled);
bt_status_t btif_ag_set_microphone_gain(btif_hf_channel_t* chan_h, uint8_t volume);
bt_status_t btif_ag_send_call_waiting_notification(btif_hf_channel_t* chan_h, const char* number);
bt_status_t btif_ag_send_result_code(btif_hf_channel_t* chan_h, const char *data, int len);
bt_status_t btif_ag_register_module_handler(btif_hf_channel_t* chan_h, struct btif_ag_module_handler* handler);
#endif /* BT_HFP_AG_ROLE */

typedef void (*hfp_aud_discon_report_cb)(uint8_t *value, uint8_t len);

void btif_hfp_user_event_callback_register(hf_event_cb_t cb);

void btif_hfp_user_event_callback_deregister(void);

hf_event_cb_t btif_hfp_get_user_event_callback(void);

void btif_hfp_aud_discon_report_callback_register(hfp_aud_discon_report_cb cb);

void btif_hfp_aud_discon_report_callback_deregister(void);

void btif_hfp_report_user_audio_play_stop_status(void);

#ifdef __cplusplus
}
#endif
#endif /* BT_HFP_SUPPORT */
#endif /*__HFP_API_H__*/
