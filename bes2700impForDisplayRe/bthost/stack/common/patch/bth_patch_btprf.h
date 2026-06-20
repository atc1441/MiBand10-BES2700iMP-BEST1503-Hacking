
/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifndef __BTH_PATCH_BTPRF_H__
#define __BTH_PATCH_BTPRF_H__
#ifdef BUILD_BTH_ROM
#include "a2dp_i.h"
#include "a2dp_common_define.h"
#include "hid_i.h"
#include "hfp_i.h"
#include "l2cap_service.h"
#include "avrcp_i.h"
#include "rfcomm_i.h"
#include "sdp_i.h"
#include "sdp_service.h"
#include "btm_i.h"
#include "dip_common_define.h"
#include "dip_i.h"

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t g_patch_tbl_btprf[];

typedef enum
{
    FUNC_ID_btprf_a2dp_sdp_callback = 1,
    FUNC_ID_btprf_av_connectReq = 2,
    FUNC_ID_btprf_av_reconfig_codec = 3,
    FUNC_ID_btprf_av_reconfig_to_vendor_codec = 4,
    FUNC_ID_btprf_a2dp_select_sbc_params = 5,
    FUNC_ID_btprf_a2dp_select_sbc_capability = 6,
    FUNC_ID_btprf_a2dp_select_aac_capability = 7,
    FUNC_ID_btprf_a2dp_select_lhdcv5_capability = 8,
    FUNC_ID_btprf_a2dp_select_lhdc_capability = 9,
    FUNC_ID_btprf_a2dp_select_ldac_capability = 10,
    FUNC_ID_btprf_a2dp_select_scalable_capability = 11,
    FUNC_ID_btprf_a2dp_select_capabilities = 12,
    FUNC_ID_btprf_a2dp_find_specified_local_codec = 13,
    FUNC_ID_btprf_select_specified_force_codec = 14,
    FUNC_ID_btprf_select_lower_priority_codec_than = 15,
    FUNC_ID_btprf_a2dp_notifyCallback = 16,
    FUNC_ID_btprf_a2dp_dataRecvCallback = 17,
    FUNC_ID_btprf_a2dp_save_ctx = 18,
    FUNC_ID_btprf_a2dp_restore_ctx = 19,
    FUNC_ID_btprf_avdtp_parse_configures = 20,
    FUNC_ID_btprf_avdtp_receive_command = 21,
    FUNC_ID_btprf_avdtp_receive_response = 22,
    FUNC_ID_btprf_avdtp_send_with_context = 23,
    FUNC_ID_btprf_avdtp_discovery_complete = 24,
    FUNC_ID_btprf_avdtp_receive_response_reject_handle = 25,
    FUNC_ID_btprf_avdtp_signal_channel_created = 26,
    FUNC_ID_btprf_avdtp_l2cap_callback = 27,
    FUNC_ID_btprf_avdtp_handle_remote_request = 28,
    FUNC_ID_btprf_avdtp_open_i = 29,
    FUNC_ID_btprf_avdtp_save_ctx = 30,
    FUNC_ID_btprf_avdtp_restore_ctx = 31,
    FUNC_ID_btprf_btif_a2dp_event_callback = 32,
    FUNC_ID_btprf_a2dp_indicate_callback = 33,
    FUNC_ID_btprf_a2dp_data_callback = 34,
    FUNC_ID_btprf_btif_a2dp_profile_save_ctx = 35,
    FUNC_ID_btprf_btif_a2dp_profile_restore_ctx = 36,
    FUNC_ID_btprf_btif_a2dp_force_disconnect_a2dp_profile = 37,
    FUNC_ID_btprf_hshf_send_cmd = 38,
    FUNC_ID_btprf__hshf_send_next_cmd = 39,
    FUNC_ID_btprf_hfp_hf_send_command_do = 40,
    FUNC_ID_btprf_hfp_sdp_callback = 41,
    FUNC_ID_btprf_is_response = 42,
    FUNC_ID_btprf__hshf_process_response = 43,
    FUNC_ID_btprf_hf_call_prefix_handler = 44,
    FUNC_ID_btprf_hf_has_result_code = 45,
    FUNC_ID_btprf_hf_process_input = 46,
    FUNC_ID_btprf_hf_sco_ind_cb = 47,
    FUNC_ID_btprf_hshf_create_codec_connection = 48,
    FUNC_ID_btprf_hfp_rfcomm_callback = 49,
    FUNC_ID_btprf_slc_completed = 50,
    FUNC_ID_btprf_hfp_ag_send_result_code = 51,
    FUNC_ID_btprf_ag_process_at_command = 52,
    FUNC_ID_btprf_ag_process_input = 53,
    FUNC_ID_btprf_enable_hf_client = 54,
    FUNC_ID_btprf_hfp_save_ctx = 55,
    FUNC_ID_btprf_hfp_restore_ctx = 56,
    FUNC_ID_btprf_btif_get_hf_chan_state = 57,
    FUNC_ID_btprf__hshf_check_request_acceptable = 58,
    FUNC_ID_btprf_btif_hfp_callback = 59,
    FUNC_ID_btprf_btif_hfp_profile_save_ctx = 60,
    FUNC_ID_btprf_btif_hfp_profile_restore_ctx = 61,
    FUNC_ID_btprf_avrcp_notify_callback = 62,
    FUNC_ID_btprf_avrcp_datarecv_callback = 63,
    FUNC_ID_btprf_avrcp_send_panel_key = 64,
    FUNC_ID_btprf_avrcp_send_advanced_command = 65,
    FUNC_ID_btprf_avrcp_send_panel_response = 66,
    FUNC_ID_btprf_avrcp_send_advanced_response = 67,
    FUNC_ID_btprf_avrcp_send_unit_info_response = 68,
    FUNC_ID_btprf_avrcp_send_subunit_info_response = 69,
    FUNC_ID_btprf_avrcp_send_unit_info_cmd = 70,
    FUNC_ID_btprf_avrcp_control_sdp_callback = 71,
    FUNC_ID_btprf_avrcp_target_sdp_callback = 72,
    FUNC_ID_btprf_avrcp_start_avctp_connect = 73,
    FUNC_ID_btprf_avrcp_connectReq = 74,
    FUNC_ID_btprf_avrcp_save_ctx = 75,
    FUNC_ID_btprf_avrcp_restore_ctx = 76,
    FUNC_ID_btprf__avctp_send_frame = 77,
    FUNC_ID_btprf__avctp_send_next_frame = 78,
    FUNC_ID_btprf_avctp_send_message = 79,
    FUNC_ID_btprf_avctp_l2cap_callback = 80,
    FUNC_ID_btprf_avctp_save_ctx = 81,
    FUNC_ID_btprf_avctp_restore_ctx = 82,
    FUNC_ID_btprf__adv_rsp_parse_register_notify = 83,
    FUNC_ID_btprf__avrcp_tx_done_send_next = 84,
    FUNC_ID_btprf__avrcp_indicate_process_adv_rsp = 85,
    FUNC_ID_btprf__avrcp_indicate_process_adv_cmd = 86,
    FUNC_ID_btprf__avrcp_indicate_process_panel_cmd = 87,
    FUNC_ID_btprf__avrcp_indicate_process_command = 88,
    FUNC_ID_btprf__avrcp_indicate_process_panel_rsp = 89,
    FUNC_ID_btprf__avrcp_indicate_process_response = 90,
    FUNC_ID_btprf__avrcp_indicate = 91,
    FUNC_ID_btprf__avrcp_datarecv_callback = 92,
    FUNC_ID_btprf_btif_avrcp_profile_save_ctxs = 93,
    FUNC_ID_btprf_btif_avrcp_profile_restore_ctxs = 94,
    FUNC_ID_btprf_btif_avrcp_force_disconnect_avrcp_profile = 95,
    FUNC_ID_btprf_btif_avrcp_ibrt_slave_restore_sdp_info = 96,
    FUNC_ID_btprf_sdp_channel_is_valid = 97,
    FUNC_ID_btprf_sdp_server_chan_req_wait_handler = 98,
    FUNC_ID_btprf_sdp_start_client_request_failed = 99,
    FUNC_ID_btprf_sdp_stop_client_wait_disc_timer = 100,
    FUNC_ID_btprf_sdp_client_wait_disc_timer_timeout = 101,
    FUNC_ID_btprf_sdp_client_conn_open_wait_timer_handler = 102,
    FUNC_ID_btprf_sdp_client_tx_wait_timer_handler = 103,
    FUNC_ID_btprf_sdp_start_client_conn_tx_wait_timer = 104,
    FUNC_ID_btprf_sdp_proto_conn_open_close = 105,
    FUNC_ID_btprf_sdp_channel_free = 106,
    FUNC_ID_btprf_sdp_serv_get_free_channel = 107,
    FUNC_ID_btprf__sdp_send_request = 108,
    FUNC_ID_btprf_sdp_start_client_request = 109,
    FUNC_ID_btprf__sdp_client_service_search = 110,
    FUNC_ID_btprf_sdp_parse_received_service_records = 111,
    FUNC_ID_btprf_sdp_client_uuid_search = 112,
    FUNC_ID_btprf__sdp_service_search_attr_request_reply = 113,
    FUNC_ID_btprf__sdp_service_search_attr_response = 114,
    FUNC_ID_btprf__sdp_service_search_response = 115,
    FUNC_ID_btprf__sdp_match_service_pattern_in_record_list = 116,
    FUNC_ID_btprf__sdp_gether_attr_in_record = 117,
    FUNC_ID_btprf__sdp_service_search_request_process = 118,
    FUNC_ID_btprf__sdp_service_attr_request_process = 119,
    FUNC_ID_btprf__sdp_service_search_attr_request_process = 120,
    FUNC_ID_btprf__sdp_service_search_response_process = 121,
    FUNC_ID_btprf_sdp_l2cap_callback = 122,
    FUNC_ID_btprf_sdp_receive_l2cap_data = 123,
    FUNC_ID_btprf_sdp_connect = 124,
    FUNC_ID_btprf_sdp_close = 125,
    FUNC_ID_btprf_rfcomm_send_frame = 126,
    FUNC_ID_btprf_rfcomm_data_is_handled = 127,
    FUNC_ID_btprf_rfcomm_process_tx = 128,
    FUNC_ID_btprf_rfcomm_foreach_dlc_in_session = 129,
    FUNC_ID_btprf_rfcomm_dlc_add_new = 130,
    FUNC_ID_btprf_rfcomm_dlc_send = 131,
    FUNC_ID_btprf_rfcomm_dlc_close_impl = 132,
    FUNC_ID_btprf_rfcomm_check_conn_req_acceptable = 133,
    FUNC_ID_btprf_rfcomm_is_dlc_connecting = 134,
    FUNC_ID_btprf_rfcomm_session_close = 135,
    FUNC_ID_btprf_rfcomm_recv_mcc = 136,
    FUNC_ID_btprf_rfcomm_recv_data = 137,
    FUNC_ID_btprf_rfcomm_client_create_new_dlc = 138,
    FUNC_ID_btprf_rfcomm_open_wait_dlc_timer_callback = 139,
    FUNC_ID_btprf_rfcomm_open_request_wait_timer_callback = 140,
    FUNC_ID_btprf_rfcomm_start_open_request_wait_timer = 141,
    FUNC_ID_btprf_rfcomm_open_wait_session_timer_callback = 142,
    FUNC_ID_btprf_rfcomm_has_same_dlc_channel = 143,
    FUNC_ID_btprf_rfcomm_open_request = 144,
    FUNC_ID_btprf_rfcomm_socket_sdp_callback = 145,
    FUNC_ID_btprf_rfcomm_ibrt_slave_release_dlc = 146,
    FUNC_ID_btprf_rfcomm_start_save_credit_before_switch = 147,
    FUNC_ID_btprf_rfcomm_send_out_saved_credits = 148,
    FUNC_ID_btprf_rfcomm_save_ctx = 149,
    FUNC_ID_btprf_rfcomm_restore_ctx = 150,
    FUNC_ID_btprf_btif_rfc_is_dlci_channel_connected = 151,
    FUNC_ID_btprf_btif_spp_rfcomm_callback = 152,
    FUNC_ID_btprf_hid_stack_init = 153,
    FUNC_ID_btprf_hid_clean_control = 154,
    FUNC_ID_btprf_hid_sdp_callback = 155,
    FUNC_ID_btprf_hid_connect_req = 156,
    FUNC_ID_btprf_hid_disconnect_req = 157,
    FUNC_ID_btprf_hid_start_create_channel = 158,
    FUNC_ID_btprf_hid_send_intr_frame = 159,
    FUNC_ID_btprf_hid_send_ctrl_frame = 160,
    FUNC_ID_btprf_hid_send_data_report = 161,
    FUNC_ID_btprf_hid_send_interrupt_report = 162,
    FUNC_ID_btprf_hid_l2cap_control_callback = 163,
    FUNC_ID_btprf_hid_l2cap_interrupt_callback = 164,
    FUNC_ID_btprf_hid_l2cap_control_data_receive = 165,
    FUNC_ID_btprf_hid_l2cap_interrupt_data_receive = 166,
    FUNC_ID_btprf_hid_save_ctx = 167,
    FUNC_ID_btprf_hid_restore_ctx = 168,
    FUNC_ID_btprf_btif_hid_profile_save_ctx = 169,
    FUNC_ID_btprf_btif_hid_profile_restore_ctx = 170,
    FUNC_ID_btprf_btif_dip_get_remote_info = 171,
    FUNC_ID_btprf_btif_dip_query_for_service = 172,
    FUNC_ID_btprf__dip_callback = 173,
    FUNC_ID_btprf_btif_dip_clear = 174,
    FUNC_ID_btprf__dip_sdp_callback = 175,
    FUNC_ID_btprf_dip_register_sdp = 176,
    FUNC_ID_btprf_dip_send_sdp_request = 177,
} bt_patch_btprf_func_enum_t;

void btif_a2dp_event_callback(uint8 device_id, struct a2dp_control_t *stream, btif_a2dp_callback_parms_t *info);
uint8_t a2dp_indicate_callback(uint8 device_id, struct a2dp_control_t *stream, uint8 event, void *pData);
void a2dp_data_callback(uint8 device_id, struct a2dp_control_t *stream, struct pp_buff *ppb);
uint32_t btif_a2dp_profile_save_ctx(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);
uint32_t btif_a2dp_profile_restore_ctx(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);
void btif_a2dp_force_disconnect_a2dp_profile(uint8_t device_id,uint8_t reason);
void _adv_rsp_parse_register_notify(uint8_t device_id, struct avrcp_control_t *avrcp_ctl, void *pdata, btif_avrcp_callback_parms_t *parms);
void _avrcp_tx_done_send_next(btif_avrcp_channel_t *channel);
struct avdtp_remote_sep* select_lower_priority_codec_than(struct a2dp_control_t *a2dp_ctl, struct avdtp_remote_sep* rsep_list, int prio);
void _avrcp_indicate_process_adv_rsp(uint8_t device_id, struct avrcp_control_t *avrcp_ctl, void *pdata, btif_avrcp_callback_parms_t *parms);
void _avrcp_indicate_process_adv_cmd(uint8_t device_id, struct avrcp_control_t *avrcp_ctl, void *pdata, btif_avrcp_callback_parms_t *parms);
void _avrcp_indicate_process_panel_cmd(uint8_t device_id, struct avrcp_control_t *avrcp_ctl, uint8 trans_id, void *pdata, btif_avrcp_callback_parms_t *parms);
void _avrcp_indicate_process_command(uint8_t device_id, struct avrcp_control_t *avrcp_ctl, void *pdata, btif_avrcp_callback_parms_t *parms);
void _avrcp_indicate_process_panel_rsp(struct avrcp_control_t *avrcp_ctl, void *pdata, btif_avrcp_callback_parms_t *parms);
void _avrcp_indicate_process_response(uint8_t device_id, struct avrcp_control_t *avrcp_ctl, void *pdata, btif_avrcp_callback_parms_t *parms);
void _avrcp_indicate(uint8_t device_id, struct avrcp_control_t *avrcp_ctl, uint8 event, void *pdata);
void _avrcp_datarecv_callback(uint8_t device_id, struct avrcp_control_t *avrcp_ctl, struct pp_buff *ppb);
uint32_t btif_avrcp_profile_save_ctxs(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);
uint32_t btif_avrcp_profile_restore_ctxs(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);
bt_status_t btif_avrcp_force_disconnect_avrcp_profile(uint8_t device_id,uint8_t reason);
void btif_avrcp_ibrt_slave_restore_sdp_info(uint8_t device_id, uint16_t avctp_version, uint16_t avrcp_version, uint16_t support_feature);
void btif_dip_get_remote_info(btif_remote_device_t *bt_dev);
bt_status_t btif_dip_query_for_service(uint16_t conn_handle, btif_dip_client_t *client_t);
void _dip_callback(struct dip_control_t *ctrl, struct dip_callback_param *param);
void btif_dip_clear(const bt_bdaddr_t *remote);
int _dip_sdp_callback(const bt_bdaddr_t *remote, bt_sdp_event_t event, bt_sdp_callback_param_t param);
int8 dip_register_sdp(void);
int8 dip_send_sdp_request(struct dip_control_t *ctrl, struct bdaddr_t *remote);
bt_hfp_chan_state_t btif_get_hf_chan_state(btif_hf_channel_t* chan_h);
void btif_hfp_callback(struct hshf_control *chan, uint8_t event, void *Info);
bool is_response(const char *prefix, enum hfp_result *result, enum hfp_error *cme_err, struct hfp_response *context);
uint32_t btif_hfp_profile_save_ctx(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);
uint32_t btif_hfp_profile_restore_ctx(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);
uint32_t hid_save_ctx(struct hid_control_t *hid_ctl, uint8_t *buf, uint32_t buf_len);
uint32_t hid_restore_ctx(struct hid_ctx_input *input, uint8_t hid_intr_connected);
uint32_t btif_hid_profile_save_ctx(btif_remote_device_t *rem_dev, uint8_t *buf, uint32_t buf_len);
uint32_t btif_hid_profile_restore_ctx(bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);
bool btif_rfc_is_dlci_channel_connected(uint32_t session_handle, uint64_t app_id);
int btif_spp_rfcomm_callback(const bt_bdaddr_t *remote, bt_socket_event_t event, bt_socket_callback_param_t param);
int a2dp_sdp_callback(const bt_bdaddr_t *remote, bt_sdp_event_t event, bt_sdp_callback_param_t param);
int8 av_connectReq(struct a2dp_control_t *stream, struct bdaddr_t *remote);
int8 av_reconfig_codec(struct a2dp_control_t *stream, uint8_t codec_id);
int8 av_reconfig_to_vendor_codec(struct a2dp_control_t *stream, uint8_t codec_id, uint8_t a2dp_non_type);
uint8 a2dp_select_sbc_params(struct a2dp_control_t *a2dp_ctl, struct sbc_codec_cap *cap, struct sbc_codec_cap *supported);
uint8 a2dp_select_sbc_capability(struct a2dp_control_t *a2dp_ctl, struct avdtp_remote_sep *rsep, uint8 **caps, uint16 *capslen);
uint8 a2dp_select_aac_capability(struct a2dp_control_t *a2dp_ctl, struct avdtp_remote_sep *rsep, uint8 **caps, uint16 *capslen);
uint8 a2dp_select_lhdcv5_capability(struct a2dp_control_t *a2dp_ctl, struct avdtp_remote_sep *rsep, uint8 **caps, uint16 *capslen);
uint8 a2dp_select_lhdc_capability(struct a2dp_control_t *a2dp_ctl, struct avdtp_remote_sep *rsep, uint8 **caps, uint16 *capslen);
uint8 a2dp_select_ldac_capability(struct a2dp_control_t *a2dp_ctl, struct avdtp_remote_sep *rsep, uint8 **caps, uint16 *capslen);
uint8 a2dp_select_scalable_capability(struct a2dp_control_t *a2dp_ctl, struct avdtp_remote_sep *rsep, uint8 **caps, uint16 *capslen);
uint8 a2dp_select_capabilities(struct a2dp_control_t *a2dp_ctl, struct avdtp_remote_sep *rsep);
struct avdtp_local_sep *a2dp_find_specified_local_codec(struct a2dp_control_t *a2dp_ctl, uint8_t codec_type, uint8_t *codec_info);
struct avdtp_remote_sep* select_specified_force_codec(struct a2dp_control_t *a2dp_ctl, struct avdtp_remote_sep* rsep_list);
void a2dp_notifyCallback(uint8 device_id, struct avdtp_control_t* avdtp_ctl, uint8 event, uint32 l2cap_channel, void *pData, uint8 reason);
void a2dp_dataRecvCallback(uint8 device_id, struct avdtp_control_t* avdtp_ctl, uint32 l2cap_handle, struct pp_buff *ppb);
uint32 a2dp_save_ctx(struct a2dp_control_t *a2dp_ctl, uint8_t *buf, uint32_t buf_len);
uint32 a2dp_restore_ctx(struct a2dp_ctx_input *input);
uint8 avdtp_receive_command(uint8 device_id, struct avdtp_control_t *avdtp_ctl, struct avdtp_header *header, uint32 size);
uint8 avdtp_receive_response(uint8 device_id, struct avdtp_control_t *avdtp_ctl, struct avdtp_header *header, uint32 size);
int8 avdtp_send_with_context(uint32 avdtp_handle, uint8 *data, uint32 datalen, void* context);
void avdtp_discovery_complete(struct avdtp_control_t* avdtp_ctl, struct avdtp_remote_sep *rsep_list);
void avdtp_receive_response_reject_handle(uint8 device_id, struct avdtp_control_t *avdtp_ctl, struct avdtp_header *header, uint32 size);
void avdtp_signal_channel_created(uint8 device_id, struct avdtp_control_t *avdtp_ctl, enum avdtp_event_enum event);
int avdtp_l2cap_callback(uintptr_t connhdl, bt_l2cap_event_t event, bt_l2cap_callback_param_t param);
int avdtp_handle_remote_request(uint8_t device_id, bt_l2cap_accept_t *accept);
uint32 avdtp_open_i(struct avdtp_control_t *avdtp_ctl, struct bdaddr_t *remote);
uint32 avdtp_save_ctx(struct avdtp_control_t *avdtp_ctl, uint8_t *buf, uint32_t buf_len);
uint32 avdtp_restore_ctx(struct avdtp_ctx_input *input);
struct avdtp_config_request *avdtp_parse_configures(struct avdtp_local_sep *local_sep, uint8 *data, uint16 len, uint8 signal_id);
int avrcp_notify_callback(uint8 device_id, struct avctp_control_t *avctp_ctl, uint8 event, uint32 handle, void *pdata);
void avrcp_datarecv_callback(uint8 device_id, struct avctp_control_t *avctp_ctl, uint32 handle, struct pp_buff *ppb);
int8 avrcp_send_panel_key(struct avrcp_control_t *avrcp_ctl, uint16 op, uint8 press);
int8 avrcp_send_advanced_command(struct avrcp_control_t *avrcp_ctl, struct avrcp_advanced_cmd_pdu *pdu);
int8 avrcp_send_advanced_browsing_command(struct avrcp_control_t *avrcp_ctl, struct avrcp_advanced_cmd_pdu *pdu);
int8 avrcp_send_panel_response(struct avrcp_control_t *avrcp_ctl, uint8 trans_id, uint16 op, uint8 press, uint8 response);
int8 avrcp_send_advanced_response(struct avrcp_control_t *avrcp_ctl, struct avrcp_advanced_cmd_pdu *pdu);
int8 avrcp_send_unit_info_response(struct avrcp_control_t *avrcp_ctl, uint8 trans_id);
int8 avrcp_send_subunit_info_response(struct avrcp_control_t *avrcp_ctl, uint8 trans_id);
int8 avrcp_send_unit_info_cmd(struct avrcp_control_t *avrcp_ctl);
int avrcp_control_sdp_callback(const bt_bdaddr_t *remote, bt_sdp_event_t event, bt_sdp_callback_param_t param);
int avrcp_target_sdp_callback(const bt_bdaddr_t *remote, bt_sdp_event_t event, bt_sdp_callback_param_t param);
void avrcp_start_avctp_connect(struct avrcp_control_t *avrcp_ctl);
int8 avrcp_connectReq(struct avrcp_control_t *avrcp_ctl, struct bdaddr_t *peer);
uint32 avrcp_save_ctx(struct avrcp_control_t *avrcp_ctl, uint8_t *buf, uint32_t buf_len);
uint32 avrcp_restore_ctx(struct avrcp_ctx_input *input, struct avrcp_ctx_output *output);
int8 _avctp_send_frame(struct avctp_control_t *avctp_ctl, struct avctp_frame_t *frame);
int8 _avctp_send_next_frame(struct avctp_control_t *avctp_ctl);
int8 avctp_send_message(struct avctp_control_t *avctp_ctl, struct avctp_frame_t *frame);
int avctp_l2cap_callback(uintptr_t connhdl, bt_l2cap_event_t event, bt_l2cap_callback_param_t param);
uint32 avctp_save_ctx(struct avctp_control_t *avctp_ctl, uint8_t *buf, uint32_t buf_len);
uint32 avctp_restore_ctx(struct avctp_ctx_input *input, struct avctp_ctx_output *output);
int8 hshf_send_cmd(struct hshf_control *chan, const char *data, uint32 len);
void _hshf_send_next_cmd(struct hshf_control *hfp);
int hfp_sdp_callback(const bt_bdaddr_t *remote, bt_sdp_event_t event, bt_sdp_callback_param_t param);
void _hshf_process_response(struct hshf_control *hfp, enum hfp_result result, enum hfp_error cme_err, bool is_timeout);
void hf_call_prefix_handler(struct hshf_control *hfp, const char *data, int total_length);
struct hf_result_code_parse_t hf_has_result_code(const uint8_t *data, uint16_t len);
void hf_process_input(struct hshf_control *hfp, const uint8_t *data, uint16_t len);
void hf_set_sco_path_status(struct bdaddr_t *remote, enum sco_event_enum event, void *data, void *link_host);
int8 hshf_create_codec_connection(struct bdaddr_t *bdaddr, struct hshf_control *chan);
int hfp_rfcomm_callback(const bt_bdaddr_t *remote, bt_socket_event_t event, bt_socket_callback_param_t param);
void slc_completed(struct hshf_control *chan);
bool hfp_ag_send_result_code(struct hshf_control *hfp, const char *data, int len);
struct hshf_control* _hshf_check_request_acceptable(uint8_t device_id, const bt_bdaddr_t* addr, uint8_t server_channel);
void ag_process_at_command(struct hshf_control *hfp, const char *at, size_t len);
void ag_process_input(struct hshf_control *hfp, const char *data, size_t len);
bool enable_hf_client(struct hshf_control *hfp);
bool hfp_hf_send_command_do(struct hshf_control *hfp, hfp_response_func_t resp_cb, const char *data, uint16_t len, bool is_cust_cmd, uint8 param);
uint32 hfp_save_ctx(struct hshf_control *hfp_ctl, uint8_t *buf, uint32_t buf_len);
uint32 hfp_restore_ctx(struct hfp_ctx_input *input, struct hfp_ctx_output *output);
int hid_stack_init(struct hid_stack_interface_t *stack_if, uint8_t is_hid_device_role);
void hid_clean_control(struct hid_control_t *hid_ctl);
int hid_sdp_callback(const bt_bdaddr_t *remote, bt_sdp_event_t event, bt_sdp_callback_param_t param);
int hid_connect_req(uint8_t device_id, struct hid_control_t *hid_ctl);
void hid_disconnect_req(struct hid_control_t *hid_ctl);
int hid_start_create_channel(uint8_t device_id, struct hid_control_t *hid_ctl);
int hid_send_intr_frame(struct hid_control_t *hid_ctl, struct hid_frame_t *frame);
int hid_send_ctrl_frame(struct hid_control_t *hid_ctl, struct hid_frame_t *frame);
int hid_send_data_report(struct hid_control_t *hid_ctl, hid_report_type_enum_t report_type, struct hid_report_data_t *report, bool send_on_interrupt_channel);
int hid_send_interrupt_report(struct hid_control_t * hid_ctl, hid_report_type_enum_t report_type, struct hid_report_data_t * report_data);
int hid_l2cap_control_callback(uintptr_t connhdl, bt_l2cap_event_t event, bt_l2cap_callback_param_t param);
int hid_l2cap_interrupt_callback(uintptr_t connhdl, bt_l2cap_event_t event, bt_l2cap_callback_param_t param);
void hid_l2cap_control_data_receive(uint8_t device_id, uint32 l2cap_handle, struct pp_buff *ppb);
void hid_l2cap_interrupt_data_receive(uint8_t device_id, uint32 l2cap_handle, struct pp_buff *ppb);
int8 rfcomm_send_frame(struct rfcomm_session *s, struct pp_buff *ppb);
void rfcomm_data_is_handled(struct rfcomm_dlc *dlc, uint8_t handled_data_count);
void rfcomm_process_tx(struct rfcomm_dlc *dlc);
bool rfcomm_foreach_dlc_in_session(bt_proto_conn_t *conn, void *priv);
int8 rfcomm_dlc_send(struct rfcomm_dlc *d, struct pp_buff *ppb);
bt_status_t rfcomm_dlc_close_impl(struct rfcomm_dlc *d, uint8_t reason, bool force_close);
bool rfcomm_is_dlc_connecting(uint32_t session_handle, uint8_t local_server_channel);
void rfcomm_session_close(struct rfcomm_session *s, uint8 reason);
void rfcomm_recv_mcc(struct rfcomm_session *s, struct pp_buff *ppb);
void rfcomm_open_wait_dlc_timer_callback(void *arg);
void rfcomm_open_request_wait_timer_callback(void *arg);
bool rfcomm_start_open_request_wait_timer(struct rfcomm_open_request_t *req);
void rfcomm_open_wait_session_timer_callback(void *arg);
bool rfcomm_has_same_dlc_channel(struct rfcomm_session *s, bt_service_port_t *port, uint8_t remote_server_channel, struct rfcomm_same_dlc_result_t *result);
bt_status_t rfcomm_open_request(struct rfcomm_open_request_t *req);
int rfcomm_socket_sdp_callback(const bt_bdaddr_t *remote, bt_sdp_event_t event, bt_sdp_callback_param_t param);
void rfcomm_ibrt_slave_release_dlc(uint8_t device_id, uint8_t dlci);
void rfcomm_start_save_credit_before_switch(const bt_bdaddr_t *remote);
void rfcomm_send_out_saved_credits(const bt_bdaddr_t *remote);
uint32 rfcomm_save_ctx(uint32 rfcomm_handle, uint8_t *buf, uint32_t buf_len);
uint32 rfcomm_restore_ctx(struct rfcomm_ctx_input *input, struct rfcomm_ctx_output *output, void *sock_priv);
void sdp_server_chan_req_wait_handler(void *arg);
void sdp_start_client_request_failed(struct sdp_control_t *sdp_chan);
void sdp_stop_client_wait_disc_timer(struct sdp_control_t *sdp_chan);
void sdp_client_wait_disc_timer_timeout(void *arg);
void sdp_client_conn_open_wait_timer_handler(void *arg);
void sdp_client_tx_wait_timer_handler(void *arg);
void sdp_start_client_conn_tx_wait_timer(struct sdp_control_t *sdp_chan, void (*timer_handler)(void *arg));
void sdp_proto_conn_open_close(struct l2cap_conn *l2cap_conn, bt_proto_conn_t *proto_conn, bool open);
void sdp_channel_free(struct sdp_control_t *sdp_chan, uint8_t reason);
bt_status_t _sdp_send_request(struct sdp_control_t *sdp_chan, struct sdp_request *request);
bt_status_t sdp_start_client_request(struct sdp_control_t *sdp_chan);
bt_status_t sdp_client_uuid_search(const bt_bdaddr_t *remote, bt_sdp_uuid_search_t *request, bool only_queue_request);
int32 _sdp_service_search_attr_request_reply(struct sdp_control_t* sdp_chan, enum sdp_pdu_id pdu_id, bool is_cont_req);
int32 _sdp_service_search_attr_response(struct sdp_control_t* sdp_chan, enum sdp_pdu_id pdu_id);
int32 _sdp_service_search_response(struct sdp_control_t* sdp_chan);
bt_status_t _sdp_service_search_request_process(uint8_t device_id, struct sdp_control_t *sdp_chan, uint16 trans_id, uint8 *param, uint16 param_len);
bt_status_t _sdp_service_attr_request_process(uint8_t device_id, struct sdp_control_t *sdp_chan, uint16 trans_id, uint8 *param, uint16 param_len);
bt_status_t _sdp_service_search_attr_request_process(uint8_t device_id, struct sdp_control_t *sdp_chan, uint16 trans_id, uint8 *param, uint16 param_len);
bt_status_t _sdp_service_search_response_process(uint8 device_id, struct sdp_control_t *sdp_chan, uint16 trans_id, uint8 *param, uint16 param_len);
int sdp_l2cap_callback(uintptr_t connhdl, bt_l2cap_event_t event, bt_l2cap_callback_param_t param);
void sdp_receive_l2cap_data(uint8 device_id, uint32 l2cap_handle, struct pp_buff *ppb);
int32 sdp_connect(struct sdp_control_t *sdp_chan, struct bdaddr_t *remote);
bt_status_t sdp_close(struct sdp_control_t *sdp_chan);
struct sdp_channel_item_t *sdp_channel_is_valid(struct sdp_control_t *sdp_chan);
struct sdp_control_t *sdp_serv_get_free_channel(struct bdaddr_t *remote);
bt_status_t _sdp_client_service_search(struct sdp_control_t *sdp_chan, const bt_sdp_service_search_t *param, bt_sdp_service_uuid_t request_uuid, bool only_queue_request);
bt_sdp_remote_record_t *sdp_parse_received_service_records(struct sdp_control_t *sdp_chan, struct sdp_request *request, enum sdp_pdu_id rsp_pdu_id, bool *free_record_list);
bt_status_t _sdp_match_service_pattern_in_record_list(struct sdp_control_t *sdp_chan, const uint8_t *search_pattern_uuid_list, uint16_t uuid_list_length, uint16_t max_service_record_count);
bt_status_t _sdp_gether_attr_in_record(struct sdp_control_t *sdp_chan, struct sdp_record_entry_t *record, const uint8_t *attr_id_list_ptr, uint16_t attr_id_list_bytes, bool store_record_handle);
struct rfcomm_dlc *rfcomm_dlc_add_new(struct rfcomm_session *s, uint8 dlci, struct bt_service_port_t *port);
int8 rfcomm_dlc_connect_ind(struct rfcomm_session *s, uint8 dlci, struct rfcomm_dlc **d);
void rfcomm_recv_data(struct rfcomm_session *s, uint8 dlci, int pf, struct pp_buff *ppb);
struct rfcomm_dlc* rfcomm_client_create_new_dlc(struct rfcomm_session *s, struct rfcomm_open_request_t *req);

typedef void (*rfcomm_send_out_saved_credits_func_t)(const bt_bdaddr_t *remote);
typedef struct rfcomm_dlc* (*rfcomm_client_create_new_dlc_func_t)(struct rfcomm_session *s, struct rfcomm_open_request_t *req);
typedef void (*rfcomm_recv_data_func_t)(struct rfcomm_session *s, uint8 dlci, int pf, struct pp_buff *ppb);
typedef struct rfcomm_registered_server_item_t *(*rfcomm_check_conn_req_acceptable_func_t)(struct rfcomm_session *s, bt_socket_accept_t *accept);
typedef struct rfcomm_dlc *(*rfcomm_dlc_add_new_func_t)(struct rfcomm_session *s, uint8 dlci, struct bt_service_port_t *port);
typedef bt_status_t (*_sdp_gether_attr_in_record_func_t)(struct sdp_control_t *sdp_chan, struct sdp_record_entry_t *record, const uint8_t *attr_id_list_ptr, uint16_t attr_id_list_bytes, bool store_record_handle);
typedef bt_status_t (*_sdp_match_service_pattern_in_record_list_func_t)(struct sdp_control_t *sdp_chan, const uint8_t *search_pattern_uuid_list, uint16_t uuid_list_length, uint16_t max_service_record_count);
typedef bt_sdp_remote_record_t* (*sdp_parse_received_service_records_func_t)(struct sdp_control_t *sdp_chan, struct sdp_request *request, enum sdp_pdu_id rsp_pdu_id, bool *free_record_list);
typedef bt_status_t (*_sdp_client_service_search_func_t)(struct sdp_control_t *sdp_chan, const bt_sdp_service_search_t *param, bt_sdp_service_uuid_t request_uuid, bool only_queue_request);
typedef struct sdp_control_t* (*sdp_serv_get_free_channel_func_t)(struct bdaddr_t *remote);
typedef void (*btif_a2dp_event_callback_func_t)(uint8 device_id, struct a2dp_control_t *stream, btif_a2dp_callback_parms_t *info);
typedef uint8_t (*a2dp_indicate_callback_func_t)(uint8 device_id, struct a2dp_control_t *stream, uint8 event, void *pData);
typedef void (*a2dp_data_callback_func_t)(uint8 device_id, struct a2dp_control_t *stream, struct pp_buff *ppb);
typedef uint32_t (*btif_a2dp_profile_save_ctx_func_t)(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);
typedef uint32_t (*btif_a2dp_profile_restore_ctx_func_t)(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);
typedef void (*btif_a2dp_force_disconnect_a2dp_profile_func_t)(uint8_t device_id,uint8_t reason);
typedef void (*_adv_rsp_parse_register_notify_func_t)(uint8_t device_id, struct avrcp_control_t *avrcp_ctl, void *pdata, btif_avrcp_callback_parms_t *parms);
typedef void (*_avrcp_tx_done_send_next_func_t)(btif_avrcp_channel_t *channel);
typedef void (*a2dp_notifyCallback_func_t)(uint8 device_id, struct avdtp_control_t* avdtp_ctl, uint8 event, uint32 l2cap_channel, void *pData, uint8 reason);
typedef struct avdtp_remote_sep* (*select_lower_priority_codec_than_func_t)(struct a2dp_control_t *a2dp_ctl, struct avdtp_remote_sep* rsep_list, int prio);
typedef void (*_avrcp_indicate_process_adv_rsp_func_t)(uint8_t device_id, struct avrcp_control_t *avrcp_ctl, void *pdata, btif_avrcp_callback_parms_t *parms);
typedef void (*_avrcp_indicate_process_adv_cmd_func_t)(uint8_t device_id, struct avrcp_control_t *avrcp_ctl, void *pdata, btif_avrcp_callback_parms_t *parms);
typedef void (*_avrcp_indicate_process_panel_cmd_func_t)(uint8_t device_id, struct avrcp_control_t *avrcp_ctl, uint8 trans_id, void *pdata, btif_avrcp_callback_parms_t *parms);
typedef void (*_avrcp_indicate_process_command_func_t)(uint8_t device_id, struct avrcp_control_t *avrcp_ctl, void *pdata, btif_avrcp_callback_parms_t *parms);
typedef struct avdtp_remote_sep* (*select_specified_force_codec_func_t)(struct a2dp_control_t *a2dp_ctl, struct avdtp_remote_sep* rsep_list);
typedef void (*_avrcp_indicate_process_panel_rsp_func_t)(struct avrcp_control_t *avrcp_ctl, void *pdata, btif_avrcp_callback_parms_t *parms);
typedef void (*btif_avrcp_ibrt_slave_restore_sdp_info_func_t)(uint8_t device_id, uint16_t avctp_version, uint16_t avrcp_version, uint16_t support_feature);
typedef void (*_avrcp_indicate_process_response_func_t)(uint8_t device_id, struct avrcp_control_t *avrcp_ctl, void *pdata, btif_avrcp_callback_parms_t *parms);
typedef void (*_avrcp_indicate_func_t)(uint8_t device_id, struct avrcp_control_t *avrcp_ctl, uint8 event, void *pdata);
typedef struct sdp_channel_item_t* (*sdp_channel_is_valid_func_t)(struct sdp_control_t *sdp_chan);
typedef void (*_avrcp_datarecv_callback_func_t)(uint8_t device_id, struct avrcp_control_t *avrcp_ctl, struct pp_buff *ppb);
typedef uint32_t (*btif_avrcp_profile_save_ctxs_func_t)(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);
typedef uint32_t (*btif_avrcp_profile_restore_ctxs_func_t)(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);
typedef bt_status_t (*btif_avrcp_force_disconnect_avrcp_profile_func_t)(uint8_t device_id,uint8_t reason);
typedef void (*btif_dip_get_remote_info_func_t)(btif_remote_device_t *bt_dev);
typedef bt_status_t (*btif_dip_query_for_service_func_t)(uint16_t conn_handle, btif_dip_client_t *client_t);
typedef void (*_dip_callback_func_t)(struct dip_control_t *ctrl, struct dip_callback_param *param);
typedef void (*btif_dip_clear_func_t)(const bt_bdaddr_t *remote);
typedef int (*_dip_sdp_callback_func_t)(const bt_bdaddr_t *remote, bt_sdp_event_t event, bt_sdp_callback_param_t param);
typedef int8 (*dip_register_sdp_func_t)(void);
typedef int8 (*dip_send_sdp_request_func_t)(struct dip_control_t *ctrl, struct bdaddr_t *remote);
typedef bt_hfp_chan_state_t (*btif_get_hf_chan_state_func_t)(btif_hf_channel_t* chan_h);
typedef void (*btif_hfp_callback_func_t)(struct hshf_control *chan, uint8_t event, void *Info);
typedef bool (*is_response_func_t)(const char *prefix, enum hfp_result *result, enum hfp_error *cme_err, struct hfp_response *context);
typedef uint32_t (*btif_hfp_profile_save_ctx_func_t)(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);
typedef uint32_t (*btif_hfp_profile_restore_ctx_func_t)(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);
typedef uint32_t (*hid_save_ctx_func_t)(struct hid_control_t *hid_ctl, uint8_t *buf, uint32_t buf_len);
typedef uint32_t (*hid_restore_ctx_func_t)(struct hid_ctx_input *input, uint8_t hid_intr_connected);
typedef uint32_t (*btif_hid_profile_save_ctx_func_t)(btif_remote_device_t *rem_dev, uint8_t *buf, uint32_t buf_len);
typedef uint32_t (*btif_hid_profile_restore_ctx_func_t)(bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);
typedef bool (*btif_rfc_is_dlci_channel_connected_func_t)(uint32_t session_handle, uint64_t app_id);
typedef int (*btif_spp_rfcomm_callback_func_t)(const bt_bdaddr_t *remote, bt_socket_event_t event, bt_socket_callback_param_t param);
typedef int (*a2dp_sdp_callback_func_t)(const bt_bdaddr_t *remote, bt_sdp_event_t event, bt_sdp_callback_param_t param);
typedef int8 (*av_connectReq_func_t)(struct a2dp_control_t *stream, struct bdaddr_t *remote);
typedef int8 (*av_reconfig_codec_func_t)(struct a2dp_control_t *stream, uint8_t codec_id);
typedef int8 (*av_reconfig_to_vendor_codec_func_t)(struct a2dp_control_t *stream, uint8_t codec_id, uint8_t a2dp_non_type);
typedef uint8 (*a2dp_select_sbc_params_func_t)(struct a2dp_control_t *a2dp_ctl, struct sbc_codec_cap *cap, struct sbc_codec_cap *supported);
typedef uint8 (*a2dp_select_sbc_capability_func_t)(struct a2dp_control_t *a2dp_ctl, struct avdtp_remote_sep *rsep, uint8 **caps, uint16 *capslen);
typedef uint8 (*a2dp_select_aac_capability_func_t)(struct a2dp_control_t *a2dp_ctl, struct avdtp_remote_sep *rsep, uint8 **caps, uint16 *capslen);
typedef uint8 (*a2dp_select_lhdcv5_capability_func_t)(struct a2dp_control_t *a2dp_ctl, struct avdtp_remote_sep *rsep, uint8 **caps, uint16 *capslen);
typedef uint8 (*a2dp_select_lhdc_capability_func_t)(struct a2dp_control_t *a2dp_ctl, struct avdtp_remote_sep *rsep, uint8 **caps, uint16 *capslen);
typedef uint8 (*a2dp_select_ldac_capability_func_t)(struct a2dp_control_t *a2dp_ctl, struct avdtp_remote_sep *rsep, uint8 **caps, uint16 *capslen);
typedef uint8 (*a2dp_select_scalable_capability_func_t)(struct a2dp_control_t *a2dp_ctl, struct avdtp_remote_sep *rsep, uint8 **caps, uint16 *capslen);
typedef uint8 (*a2dp_select_capabilities_func_t)(struct a2dp_control_t *a2dp_ctl, struct avdtp_remote_sep *rsep);
typedef void (*a2dp_notifyCallback_func_t)(uint8 device_id, struct avdtp_control_t* avdtp_ctl, uint8 event, uint32 l2cap_channel, void *pData, uint8 reason);
typedef void (*a2dp_dataRecvCallback_func_t)(uint8 device_id, struct avdtp_control_t* avdtp_ctl, uint32 l2cap_handle, struct pp_buff *ppb);
typedef uint32 (*a2dp_save_ctx_func_t)(struct a2dp_control_t *a2dp_ctl, uint8_t *buf, uint32_t buf_len);
typedef uint32 (*a2dp_restore_ctx_func_t)(struct a2dp_ctx_input *input);
typedef uint8 (*avdtp_receive_command_func_t)(uint8 device_id, struct avdtp_control_t *avdtp_ctl, struct avdtp_header *header, uint32 size);
typedef uint8 (*avdtp_receive_response_func_t)(uint8 device_id, struct avdtp_control_t *avdtp_ctl, struct avdtp_header *header, uint32 size);
typedef int8 (*avdtp_send_with_context_func_t)(uint32 avdtp_handle, uint8 *data, uint32 datalen, void* context);
typedef void (*avdtp_discovery_complete_func_t)(struct avdtp_control_t* avdtp_ctl, struct avdtp_remote_sep *rsep_list);
typedef void (*avdtp_receive_response_reject_handle_func_t)(uint8 device_id, struct avdtp_control_t *avdtp_ctl, struct avdtp_header *header, uint32 size);
typedef void (*avdtp_signal_channel_created_func_t)(uint8 device_id, struct avdtp_control_t *avdtp_ctl, enum avdtp_event_enum event);
typedef int (*avdtp_l2cap_callback_func_t)(uintptr_t connhdl, bt_l2cap_event_t event, bt_l2cap_callback_param_t param);
typedef int (*avdtp_handle_remote_request_func_t)(uint8_t device_id, bt_l2cap_accept_t *accept);
typedef uint32 (*avdtp_open_i_func_t)(struct avdtp_control_t *avdtp_ctl, struct bdaddr_t *remote);
typedef uint32 (*avdtp_save_ctx_func_t)(struct avdtp_control_t *avdtp_ctl, uint8_t *buf, uint32_t buf_len);
typedef uint32 (*avdtp_restore_ctx_func_t)(struct avdtp_ctx_input *input);
typedef struct avdtp_config_request* (*avdtp_parse_configures_func_t)(struct avdtp_local_sep *local_sep, uint8 *data, uint16 len, uint8 signal_id);
typedef int (*avrcp_notify_callback_func_t)(uint8 device_id, struct avctp_control_t *avctp_ctl, uint8 event, uint32 handle, void *pdata);
typedef void (*avrcp_datarecv_callback_func_t)(uint8 device_id, struct avctp_control_t *avctp_ctl, uint32 handle, struct pp_buff *ppb);
typedef int8 (*avrcp_send_panel_key_func_t)(struct avrcp_control_t *avrcp_ctl, uint16 op, uint8 press);
typedef int8 (*avrcp_send_advanced_command_func_t)(struct avrcp_control_t *avrcp_ctl, struct avrcp_advanced_cmd_pdu *pdu);
typedef int8 (*avrcp_send_panel_response_func_t)(struct avrcp_control_t *avrcp_ctl, uint8 trans_id, uint16 op, uint8 press, uint8 response);
typedef int8 (*avrcp_send_advanced_response_func_t)(struct avrcp_control_t *avrcp_ctl, struct avrcp_advanced_cmd_pdu *pdu);
typedef int8 (*avrcp_send_unit_info_response_func_t)(struct avrcp_control_t *avrcp_ctl, uint8 trans_id);
typedef int8 (*avrcp_send_subunit_info_response_func_t)(struct avrcp_control_t *avrcp_ctl, uint8 trans_id);
typedef int8 (*avrcp_send_unit_info_cmd_func_t)(struct avrcp_control_t *avrcp_ctl);
typedef int (*avrcp_control_sdp_callback_func_t)(const bt_bdaddr_t *remote, bt_sdp_event_t event, bt_sdp_callback_param_t param);
typedef int (*avrcp_target_sdp_callback_func_t)(const bt_bdaddr_t *remote, bt_sdp_event_t event, bt_sdp_callback_param_t param);
typedef void (*avrcp_start_avctp_connect_func_t)(struct avrcp_control_t *avrcp_ctl);
typedef int8 (*avrcp_connectReq_func_t)(struct avrcp_control_t *avrcp_ctl, struct bdaddr_t *peer);
typedef uint32 (*avrcp_save_ctx_func_t)(struct avrcp_control_t *avrcp_ctl, uint8_t *buf, uint32_t buf_len);
typedef uint32 (*avrcp_restore_ctx_func_t)(struct avrcp_ctx_input *input, struct avrcp_ctx_output *output);
typedef int8 (*_avctp_send_frame_func_t)(struct avctp_control_t *avctp_ctl, struct avctp_frame_t *frame);
typedef int8 (*_avctp_send_next_frame_func_t)(struct avctp_control_t *avctp_ctl);
typedef int8 (*avctp_send_message_func_t)(struct avctp_control_t *avctp_ctl, struct avctp_frame_t *frame);
typedef int (*avctp_l2cap_callback_func_t)(uintptr_t connhdl, bt_l2cap_event_t event, bt_l2cap_callback_param_t param);
typedef uint32 (*avctp_save_ctx_func_t)(struct avctp_control_t *avctp_ctl, uint8_t *buf, uint32_t buf_len);
typedef uint32 (*avctp_restore_ctx_func_t)(struct avctp_ctx_input *input, struct avctp_ctx_output *output);
typedef int8 (*hshf_send_cmd_func_t)(struct hshf_control *chan, const char *data, uint32 len);
typedef void (*_hshf_send_next_cmd_func_t)(struct hshf_control *hfp);
typedef struct hshf_control* (*_hshf_check_request_acceptable_func_t)(uint8_t device_id, const bt_bdaddr_t* addr, uint8_t server_channel);
typedef bool (*hfp_hf_send_command_do_func_t)(struct hshf_control *hfp, hfp_response_func_t resp_cb,const char *data, uint16_t len, bool is_cust_cmd, uint8 param);
typedef int (*hfp_sdp_callback_func_t)(const bt_bdaddr_t *remote, bt_sdp_event_t event, bt_sdp_callback_param_t param);
typedef void (*_hshf_process_response_func_t)(struct hshf_control *hfp, enum hfp_result result, enum hfp_error cme_err, bool is_timeout);
typedef void (*hf_call_prefix_handler_func_t)(struct hshf_control *hfp, const char *data, int total_length);
typedef struct hf_result_code_parse_t (*hf_has_result_code_func_t)(const uint8_t *data, uint16_t len);
typedef void (*hf_process_input_func_t)(struct hshf_control *hfp, const uint8_t *data, uint16_t len);
typedef void (*hf_sco_ind_cb_func_t)(struct bdaddr_t *remote, enum sco_event_enum event, void *data, void *link_host);
typedef int8 (*hshf_create_codec_connection_func_t)(struct bdaddr_t *bdaddr, struct hshf_control *chan);
typedef int (*hfp_rfcomm_callback_func_t)(const bt_bdaddr_t *remote, bt_socket_event_t event, bt_socket_callback_param_t param);
typedef void (*slc_completed_func_t)(struct hshf_control *chan);
typedef bool (*hfp_ag_send_result_code_func_t)(struct hshf_control *hfp, const char *data, int len);
typedef void (*ag_process_at_command_func_t)(struct hshf_control *hfp, const char *at, size_t len);
typedef void (*ag_process_input_func_t)(struct hshf_control *hfp, const char *data, size_t len);
typedef bool (*enable_hf_client_func_t)(struct hshf_control *hfp);
typedef uint32 (*hfp_save_ctx_func_t)(struct hshf_control *hfp_ctl, uint8_t *buf, uint32_t buf_len);
typedef uint32 (*hfp_restore_ctx_func_t)(struct hfp_ctx_input *input, struct hfp_ctx_output *output);
typedef int (*hid_stack_init_func_t)(struct hid_stack_interface_t *stack_if, uint8_t is_hid_device_role);
typedef void (*hid_clean_control_func_t)(struct hid_control_t *hid_ctl);
typedef int (*hid_sdp_callback_func_t)(const bt_bdaddr_t *remote, bt_sdp_event_t event, bt_sdp_callback_param_t param);
typedef int (*hid_connect_req_func_t)(uint8_t device_id, struct hid_control_t *hid_ctl);
typedef void (*hid_disconnect_req_func_t)(struct hid_control_t *hid_ctl);
typedef int (*hid_start_create_channel_func_t)(uint8_t device_id, struct hid_control_t *hid_ctl);
typedef int (*hid_send_intr_frame_func_t)(struct hid_control_t *hid_ctl, struct hid_frame_t *frame);
typedef int (*hid_send_ctrl_frame_func_t)(struct hid_control_t *hid_ctl, struct hid_frame_t *frame);
typedef int (*hid_send_data_report_func_t)(struct hid_control_t *hid_ctl, hid_report_type_enum_t report_type, struct hid_report_data_t *report, bool send_on_interrupt_channel);
typedef int (*hid_send_interrupt_report_func_t)(struct hid_control_t * hid_ctl, hid_report_type_enum_t report_type, struct hid_report_data_t * report_data);
typedef int (*hid_l2cap_control_callback_func_t)(uintptr_t connhdl, bt_l2cap_event_t event, bt_l2cap_callback_param_t param);
typedef int (*hid_l2cap_interrupt_callback_func_t)(uintptr_t connhdl, bt_l2cap_event_t event, bt_l2cap_callback_param_t param);
typedef void (*hid_l2cap_control_data_receive_func_t)(uint8_t device_id, uint32 l2cap_handle, struct pp_buff *ppb);
typedef void (*hid_l2cap_interrupt_data_receive_func_t)(uint8_t device_id, uint32 l2cap_handle, struct pp_buff *ppb);
typedef int8 (*rfcomm_send_frame_func_t)(struct rfcomm_session *s, struct pp_buff *ppb);
typedef void (*rfcomm_data_is_handled_func_t)(struct rfcomm_dlc *dlc, uint8_t handled_data_count);
typedef void (*rfcomm_process_tx_func_t)(struct rfcomm_dlc *dlc);
typedef bool (*rfcomm_foreach_dlc_in_session_func_t)(bt_proto_conn_t *conn, void *priv);
typedef int8 (*rfcomm_dlc_send_func_t)(struct rfcomm_dlc *d, struct pp_buff *ppb);
typedef bt_status_t (*rfcomm_dlc_close_impl_func_t)(struct rfcomm_dlc *d, uint8_t reason, bool force_close);
typedef bool (*rfcomm_is_dlc_connecting_func_t)(uint32_t session_handle, uint8_t local_server_channel);
typedef void (*rfcomm_session_close_func_t)(struct rfcomm_session *s, uint8 reason);
typedef void (*rfcomm_recv_mcc_func_t)(struct rfcomm_session *s, struct pp_buff *ppb);
typedef void (*rfcomm_open_wait_dlc_timer_callback_func_t)(void *arg);
typedef void (*rfcomm_open_request_wait_timer_callback_func_t)(void *arg);
typedef bool (*rfcomm_start_open_request_wait_timer_func_t)(struct rfcomm_open_request_t *req);
typedef void (*rfcomm_open_wait_session_timer_callback_func_t)(void *arg);
typedef bool (*rfcomm_has_same_dlc_channel_func_t)(struct rfcomm_session *s, bt_service_port_t *port, uint8_t remote_server_channel, struct rfcomm_same_dlc_result_t *result);
typedef bt_status_t (*rfcomm_open_request_func_t)(struct rfcomm_open_request_t *req);
typedef int (*rfcomm_socket_sdp_callback_func_t)(const bt_bdaddr_t *remote, bt_sdp_event_t event, bt_sdp_callback_param_t param);
typedef void (*rfcomm_ibrt_slave_release_dlc_func_t)(uint8_t device_id, uint8_t dlci);
typedef void (*rfcomm_start_save_credit_before_switch_func_t)(const bt_bdaddr_t *remote);
typedef void (*rfcomm_send_out_saved_credits_func_t)(const bt_bdaddr_t *remote);
typedef uint32 (*rfcomm_save_ctx_func_t)(uint32 rfcomm_handle, uint8_t *buf, uint32_t buf_len);
typedef uint32 (*rfcomm_restore_ctx_func_t)(struct rfcomm_ctx_input *input, struct rfcomm_ctx_output *output, void *sock_priv);
typedef void (*sdp_server_chan_req_wait_handler_func_t)(void *arg);
typedef struct avdtp_local_sep*(*a2dp_find_specified_local_codec_func_t)(struct a2dp_control_t *a2dp_ctl, uint8_t codec_type, uint8_t *codec_info);
typedef void (*sdp_start_client_request_failed_func_t)(struct sdp_control_t *sdp_chan);
typedef void (*sdp_stop_client_wait_disc_timer_func_t)(struct sdp_control_t *sdp_chan);
typedef void (*sdp_client_wait_disc_timer_timeout_func_t)(void *arg);
typedef void (*sdp_client_conn_open_wait_timer_handler_func_t)(void *arg);
typedef void (*sdp_client_tx_wait_timer_handler_func_t)(void *arg);
typedef struct sdp_channel_item_t *(*sdp_channel_is_valid_func_t)(struct sdp_control_t *sdp_chan);
typedef void (*sdp_start_client_conn_tx_wait_timer_func_t)(struct sdp_control_t *sdp_chan, void (*timer_handler)(void *arg));
typedef void (*sdp_proto_conn_open_close_func_t)(struct l2cap_conn *l2cap_conn, bt_proto_conn_t *proto_conn, bool open);
typedef void (*sdp_channel_free_func_t)(struct sdp_control_t *sdp_chan, uint8_t reason);
typedef bt_status_t (*_sdp_send_request_func_t)(struct sdp_control_t *sdp_chan, struct sdp_request *request);
typedef bt_status_t (*sdp_start_client_request_func_t)(struct sdp_control_t *sdp_chan);
typedef bt_status_t (*sdp_client_uuid_search_func_t)(const bt_bdaddr_t *remote, bt_sdp_uuid_search_t *request, bool only_queue_request);
typedef int32 (*_sdp_service_search_attr_request_reply_func_t)(struct sdp_control_t* sdp_chan, enum sdp_pdu_id pdu_id, bool is_cont_req);
typedef int32 (*_sdp_service_search_attr_response_func_t)(struct sdp_control_t* sdp_chan, enum sdp_pdu_id pdu_id);
typedef int32 (*_sdp_service_search_response_func_t)(struct sdp_control_t* sdp_chan);
typedef bt_status_t (*_sdp_service_search_request_process_func_t)(uint8_t device_id, struct sdp_control_t *sdp_chan, uint16 trans_id, uint8 *param, uint16 param_len);
typedef bt_status_t (*_sdp_service_attr_request_process_func_t)(uint8_t device_id, struct sdp_control_t *sdp_chan, uint16 trans_id, uint8 *param, uint16 param_len);
typedef bt_status_t (*_sdp_service_search_attr_request_process_func_t)(uint8_t device_id, struct sdp_control_t *sdp_chan, uint16 trans_id, uint8 *param, uint16 param_len);
typedef bt_status_t (*_sdp_service_search_response_process_func_t)(uint8 device_id, struct sdp_control_t *sdp_chan, uint16 trans_id, uint8 *param, uint16 param_len);
typedef int (*sdp_l2cap_callback_func_t)(uintptr_t connhdl, bt_l2cap_event_t event, bt_l2cap_callback_param_t param);
typedef void (*sdp_receive_l2cap_data_func_t)(uint8 device_id, uint32 l2cap_handle, struct pp_buff *ppb);
typedef int32 (*sdp_connect_func_t)(struct sdp_control_t *sdp_chan, struct bdaddr_t *remote);
typedef bt_status_t (*sdp_close_func_t)(struct sdp_control_t *sdp_chan);

#ifdef __cplusplus
}
#endif
#endif /* BUILD_BTH_ROM */
#endif /* __BTH_PATCH_BASIC_H__ */
