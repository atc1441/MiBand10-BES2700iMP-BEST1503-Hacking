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
#ifndef __BES_IBRT_API_H__
#define __BES_IBRT_API_H__
#include "ibrt_common_define.h"
#ifdef IBRT
#ifdef __cplusplus
extern "C" {
#endif

void bes_bt_tws_besaud_client_create(uint16_t conn_handle);

void bes_bt_tws_besaud_server_create(void (*cb)(uint16_t besaud_event));

uint8_t bes_bt_tws_besaud_is_connected(void);

void bes_bt_tws_besaud_data_recv_register(void (*cb)(uint8_t *data, uint16_t len));

void bes_bt_tws_besaud_clear_cmd_sending(void);

bt_status_t bes_bt_tws_besaud_send_cmd_no_wait(uint8_t* cmd, uint16_t len);

void bes_bt_tws_besaud_send_cmd(uint8_t* cmd, uint16_t len);

uint8_t bes_bt_tws_besaud_is_cmd_sending(void);

bt_ibrt_state_t bes_bt_tws_ibrt_get_state(int device_id);

void bes_bt_tws_ibrt_set_field(int device_id, BT_IBRT_FIELD_ENUM_T field, uint32_t value);

void bes_bt_tws_ibrt_reconnect_mobile_profile(const bt_bdaddr_t *remote);

void bes_bt_tws_ibrt_set_remote_tws_device(const bt_bdaddr_t *remote, bool tws_link);

bool bes_bt_tws_ibrt_is_remote_tws_device(const bt_bdaddr_t *remote);

void bes_bt_tws_ibrt_clear_reconnect_mobile_profile_flag(bt_bdaddr_t* remote_addr);

void bes_bt_tws_ibrt_reset_tws_packet_check(void);

bool bes_bt_tws_ibrt_master_wait_new_master_ready(const bt_bdaddr_t* remote);

void bes_bt_tws_ibrt_slave_become_master(const bt_bdaddr_t* remote);

void bes_bt_tws_ibrt_master_become_slave(const bt_bdaddr_t* remote);

void bes_bt_tws_ibrt_master_set_switch_start(const bt_bdaddr_t* remote);

void bes_bt_tws_ibrt_slave_set_switch_start(const bt_bdaddr_t* remote);

void bes_bt_tws_ibrt_old_master_receive_ready_req(struct btif_sync_data_to_new_master_t *sync_data, const bt_bdaddr_t *remote);

void bes_bt_tws_ibrt_new_master_receive_ready_rsp(struct btif_sync_data_to_new_master_t *sync_data);

bt_status_t bes_bt_tws_start_ibrt(uint16_t tws_conhandle, uint16_t mobile_conhandle);

bt_status_t bes_bt_tws_stop_ibrt(uint16_t mobile_conhdl,uint8_t reason);

void bes_bt_tws_set_env(uint8_t sniffer_acitve, uint8_t sniffer_role, uint8_t * monitored_addr, uint8_t * sniffer_addr);

bt_status_t bes_bt_tws_ibrt_role_switch(uint16_t mobile_conhdl);

bt_status_t bes_bt_tws_ibrt_mode_init(bool enable);

bt_status_t bes_bt_tws_suspend_ibrt(void);

bt_status_t bes_bt_tws_resume_ibrt(uint8_t enable);

bt_status_t bes_bt_tws_ibrt_enable_fastack(uint16_t conhdl, uint8_t direction, uint8_t enable);

void bes_bt_tws_ibrt_create_snoop_acl_link(bt_bdaddr_t *remote, uint16 conn_handle);

void bes_bt_tws_ibrt_register_sco_link(uint8_t device_id, bt_bdaddr_t *remote);

void bes_bt_tws_ibrt_report_snoop_acl_disconnected(uint8_t device_id, bt_bdaddr_t *remote);

void bes_bt_tws_ibrt_clean_slave_state(const bt_bdaddr_t* remote);

void bes_bt_tws_ibrt_response_acl_conn_req(bt_bdaddr_t *remote, bool accept);

#ifdef BT_A2DP_SUPPORT
uint8_t bes_bt_tws_ibrt_is_critical_avdtp_cmd_handling(void);

void bes_bt_tws_ibrt_critical_avdtp_cmd_timeout(void);
#endif

void bes_bt_tws_ibrt_register_hci_acl_ecc_softbit_handler(bt_hci_acl_ecc_softbit_handler_func func);

void bes_bt_tws_ibrt_fake_mobile_disconnect(uint16_t hci_handle, uint8_t reason);

void bes_bt_tws_ibrt_fake_tws_disconnect(uint16_t hci_handle, uint8_t reason);

void bes_bt_tws_ibrt_fake_tws_connect(uint8_t status, bt_bdaddr_t *bdAddr);

void bes_bt_tws_ibrt_fake_hci_event_disallow(uint8_t opcode1, uint8_t opcode2);

void bes_bt_tws_ibrt_mock_a2dp_callback(uint8_t device_id, const bt_bdaddr_t *remote, const a2dp_callback_parms_t *info);

void bes_bt_tws_ibrt_mock_avrcp_callback(uint8_t device_id, const bt_bdaddr_t *remote, const avrcp_callback_parms_t *parms);

uint32_t bes_bt_tws_a2dp_get_ibrt_session(uint8_t device_id);

int bes_bt_tws_a2dp_set_ibrt_session(uint8_t session, uint8_t device_id);

int bes_bt_tws_ibrt_a2dp_sync_get_status(uint8_t device_id, ibrt_a2dp_status_t *a2dp_status);

int bes_bt_tws_ibrt_a2dp_sync_set_status(uint8_t device_id, ibrt_a2dp_status_t *a2dp_status);

#ifdef BT_HFP_SUPPORT
void bes_bt_tws_ibrt_register_sco_link(uint8_t device_id, bt_bdaddr_t *remote);

bt_status_t bes_bt_tws_ibrt_mock_sync_conn_disconnected(uint16_t conn_handle);

bt_status_t bes_bt_tws_ibrt_mock_sync_conn_connected(hfp_sco_codec_t sco_codec, uint16_t conhdl);

int bes_bt_tws_ibrt_hfp_service_connected_mock(uint8_t device_id);

int bes_bt_tws_ibrt_hfp_sync_get_status(uint8_t device_id,ibrt_hfp_status_t *hfp_status);

int bes_bt_tws_ibrt_hfp_sync_set_status(uint8_t device_id,ibrt_hfp_status_t *hfp_status);

int bes_bt_tws_ibrt_hfp_sco_audio_disconnected(void);

int bes_bt_tws_ibrt_hfp_sco_audio_connected(hfp_sco_codec_t codec, uint16_t sco_connhdl);

void bes_bt_tws_ibrt_hfp_sync_status_sent_callback(void);

void bes_bt_tws_ibrt_receive_peer_sco_codec_info(const void* remote, uint8_t codec);

bt_status_t bes_bt_tws_ibrt_force_disconnect_hfp_profile(uint8_t device_id,uint8_t reason);
#endif

void bes_bt_tws_ibrt_spp_slave_release_dlc_connection(uint32_t device_id, uint32_t dlci);

#ifdef BT_AVRCP_SUPPORT
void bes_bt_tws_ibrt_avrcp_slave_restore_sdp_info(uint8_t device_id, uint16_t avctp_version, uint16_t avrcp_version, uint16_t support_feature);

bt_status_t bes_bt_tws_ibrt_force_disconnect_avrcp_profile(uint8_t device_id,uint8_t reason);
#endif

#if defined(__GATT_OVER_BR_EDR__)
void bes_bt_tws_ibrt_force_disconnect_btgatt_profile(uint8_t device_id,uint8_t reason);
#endif

void bes_bt_tws_ibrt_force_disconnect_spp_profile(const bt_bdaddr_t *addr,uint64_t app_id,uint8_t reason);

void bes_bt_tws_ibrt_force_disconnect_a2dp_profile(uint8_t device_id,uint8_t reason);

void bes_bt_tws_ibrt_sync_set_avdtp_streaming_state(bt_bdaddr_t *addr);

uint64_t bes_bt_tws_ibrt_get_app_id_from_spp_flag(uint8_t spp_flag);

uint8_t bes_bt_tws_ibrt_get_spp_flag_from_app_id(uint64_t app_id);

uint32_t bes_bt_tws_ibrt_save_profile(BT_PROFILE_SYNC_ENUM_T op, bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);

uint32_t bes_bt_tws_ibrt_save_spp_profile(BT_PROFILE_SYNC_ENUM_T op, uint64_t app_id, bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);

uint32_t bes_bt_tws_ibrt_restore_profile(BT_PROFILE_SYNC_ENUM_T op, bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);

uint32_t bes_bt_tws_ibrt_restore_spp_profile(BT_PROFILE_SYNC_ENUM_T op, bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len, uint64_t app_id);

const char *bes_bt_tws_ibrt_get_device_current_roles(void);

void bes_bt_tws_audio_receive_peer_a2dp_playing_device(bool is_response, uint8_t device_id);

uint32_t bes_bt_tws_audio_start_trigger_switch_a2dp(uint32_t btclk);

bool bes_bt_tws_audio_switch_streaming_a2dp(void);

void bes_bt_tws_audio_switch_streaming_sco(void);

void bes_bt_tws_ibrt_report_audio_retrigger(uint8_t retrgigerType);

void bes_bt_tws_ibrt_a2dp_reject_sniff_start(uint8_t device_id, uint32_t timeout);

#ifdef __cplusplus
}
#endif
#endif /* IBRT */
#endif /* __BES_IBRT_API_H__ */
