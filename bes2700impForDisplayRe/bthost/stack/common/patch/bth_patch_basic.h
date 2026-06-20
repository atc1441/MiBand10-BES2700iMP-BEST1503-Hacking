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
#ifndef __BTH_PATCH_BASIC_H__
#define __BTH_PATCH_BASIC_H__
#ifdef BUILD_BTH_ROM
#include "gap_service.h"
#include "gatt_service.h"
#include "gap_i.h"
#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t g_patch_tbl_basic[];
uint32_t *bth_get_basic_patch_table(void);

typedef enum
{
    FUNC_ID_basic_gatt_conn_ready_handler,
    FUNC_ID_basic_gap_impl_get_peer_resolved_addr,
    FUNC_ID_basic_gap_get_peer_resolved_address,
    FUNC_ID_basic_gap_report_connection_event,
    FUNC_ID_basic_gap_recv_smp_encrypted,
    FUNC_ID_basic_gap_address_reso_check_activity,
    FUNC_ID_basic_gap_resolving_list_check_activity,
    FUNC_ID_basic_gap_enable_address_resolution,
    FUNC_ID_basic_gap_filter_list_check_activity,
    FUNC_ID_basic_gap_adv_conn_complete_handle,
    FUNC_ID_basic_gap_impl_set_adv_parameters,
    FUNC_ID_basic_gap_parse_curr_adv_parameters,
    FUNC_ID_basic_gap_adv_has_same_parameters,
    FUNC_ID_basic_gap_create_advertising,
    FUNC_ID_basic_gap_select_adv_timing,
    FUNC_ID_basic_gap_select_scan_timing,
    FUNC_ID_basic_gap_impl_set_scan_params,
    FUNC_ID_basic_gap_select_init_timing,
    FUNC_ID_basic_gap_init_conn_complete_handle,
    FUNC_ID_basic_gap_impl_le_create_conn,
    FUNC_ID_basic_gap_impl_start_initiating,
    FUNC_ID_basic_gap_le_conn_is_established,
    FUNC_ID_basic_smp_get_local_address,
    FUNC_ID_basic_smp_get_peer_address,
    FUNC_ID_basic_smp_security_requirements_check,
    FUNC_ID_basic_smp_prepare_for_encryption,
    FUNC_ID_basic_smp_receive_peer_ltk_req,
    FUNC_ID_basic_smp_reply_ltk_req,
    FUNC_ID_basic_smp_start_pairing_request,
    FUNC_ID_basic_smp_start_security_request,
    FUNC_ID_basic_smp_select_pairing_method,
    FUNC_ID_basic_smp_distribute_keys,
    FUNC_ID_basic_smp_wait_remote_key_distribute,
    FUNC_ID_basic_smp_start_phase_3_key_distribute,
    FUNC_ID_basic_smp_phase_transfer,
    FUNC_ID_basic_smp_init_sc_gen_ltk_and_encrypt,
    FUNC_ID_basic_smp_resp_sc_gen_ltk_and_encrypt,
    FUNC_ID_basic_smp_init_sc_numeric_compare,
    FUNC_ID_basic_smp_resp_sc_numeric_compare,
    FUNC_ID_basic_smp_init_sc_passkey_entry,
    FUNC_ID_basic_smp_resp_sc_passkey_entry,
    FUNC_ID_basic_smp_init_sc_oob_method,
    FUNC_ID_basic_smp_resp_sc_oob_method,
    FUNC_ID_basic_smp_init_secure_rx_handle,
    FUNC_ID_basic_smp_resp_secure_rx_handle,
    FUNC_ID_basic_smp_init_rx_handle,
    FUNC_ID_basic_smp_resp_rx_handle,
    FUNC_ID_basic_smp_receive_packet,
    FUNC_ID_basic_gatt_report_mtu_changed,
    FUNC_ID_basic_gatt_cache_save,
    FUNC_ID_basic_att_bearer_add_new,
    FUNC_ID_basic_att_bearer_close,
    FUNC_ID_basic_att_recv_request,
    FUNC_ID_basic_att_recv_response,
    FUNC_ID_basic_att_bearer_recv_req_packet,
    FUNC_ID_basic_att_receive_packet,
    FUNC_ID_basic_att_l2cap_callback,
    FUNC_ID_basic_att_bearer_send_cmd_or_ntf,
    FUNC_ID_basic_att_bearer_process_req_ind_q,
    FUNC_ID_basic_gatt_server_conn_handle,
    FUNC_ID_basic_gatt_conn_event_handler,
    FUNC_ID_basic_gatt_add_svc_to_conn,
    FUNC_ID_basic_gatt_send_notify_for_each_conn,
    FUNC_ID_basic_gatt_server_recv_read_req,
    FUNC_ID_basic_gatt_server_recv_write_req,
    FUNC_ID_basic_gatt_for_each_attr_callback,
    FUNC_ID_basic_gatt_server_check_notify_permission,
} bt_patch_basic_func_enum_t;

bt_status_t gatt_conn_ready_handler(gap_conn_item_t *item);
ble_bdaddr_t gap_impl_get_peer_resolved_addr(gap_conn_item_t *conn);
ble_bdaddr_t gap_get_peer_resolved_address(uint16_t connhdl);
void gap_report_connection_event(gap_conn_item_t *conn, gap_conn_event_t event, uint8_t error_code, void *extra);
void gap_recv_smp_encrypted(gap_conn_item_t *conn, bool new_pair, uint8_t error_code);
bool gap_address_reso_check_activity(void);
void gap_resolving_list_check_activity(void);
bt_status_t gap_enable_address_resolution(bool enable);
void gap_filter_list_check_activity(void);
void gap_adv_conn_complete_handle(uint8_t status, gap_conn_item_t *conn, struct hci_ev_le_connection_cmpl *p);
bt_status_t gap_impl_set_adv_parameters(gap_advertising_t *adv);
bt_status_t gap_parse_curr_adv_parameters(uint8_t adv_handle, const gap_adv_param_t *param, gap_adv_callback_t cb, gap_advertising_t *adv);
bool gap_adv_has_same_parameters(const gap_advertising_t *adv, const gap_advertising_t *new_adv);
uint8_t gap_create_advertising(const gap_adv_param_t *param, gap_adv_callback_t cb, bool refresh_adv, uint8_t adv_handle);
void gap_select_adv_timing(gap_advertising_t *adv);
void gap_select_scan_timing(gap_scanning_t *scan);
bt_status_t gap_impl_set_scan_params(gap_scanning_t *scan);
void gap_select_init_timing(gap_initiating_t *init);
void gap_init_conn_complete_handle(uint8_t status, gap_conn_item_t *conn, struct hci_ev_le_connection_cmpl *p);
bt_status_t gap_impl_le_create_conn(gap_initiating_t *init);
bt_status_t gap_impl_start_initiating(uint32_t adv_handle_subevent_bleaudio, const gap_init_param_t *param, gap_init_callback_t cb);
void gap_le_conn_is_established(gap_conn_item_t *conn, bt_addr_type_t own_addr_type, const bt_bdaddr_t *own_addr, gap_conn_callback_t callback, uint8_t from_adv_handle, bool directed);
const uint8_t *smp_get_local_address(gap_conn_item_t *conn, uint8_t *addr_type);
const uint8_t *smp_get_peer_address(gap_conn_item_t *conn, uint8_t *addr_type);
bt_status_t smp_security_requirements_check(gap_conn_item_t *conn, smp_requirements_t *r, bool check_mitm);
bt_status_t smp_prepare_for_encryption(gap_conn_item_t *conn, bool is_central);
void smp_receive_peer_ltk_req(gap_conn_item_t *conn, struct hci_ev_le_ltk_request *p);
bool smp_reply_ltk_req(smp_conn_item_t *smp);
bt_status_t smp_start_pairing_request(gap_conn_item_t *conn, const smp_auth_requirements_t *peer_auth_req);
bt_status_t smp_start_security_request(gap_conn_item_t *conn);
smp_error_code_t smp_select_pairing_method(smp_conn_item_t *smp);
void smp_distribute_keys(smp_conn_item_t *smp, uint8_t key_dist, uint8_t peer_kdist);
bool smp_wait_remote_key_distribute(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
void smp_start_phase_3_key_distribute(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv, void (*key_distribute_func)(smp_conn_item_t *, smp_event_t, const smp_receive_t *));
smp_error_code_t smp_phase_transfer(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv, const smp_transfer_t *trans, int count);
void smp_init_sc_gen_ltk_and_encrypt(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
void smp_resp_sc_gen_ltk_and_encrypt(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
void smp_init_sc_numeric_compare(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
void smp_resp_sc_numeric_compare(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
void smp_init_sc_passkey_entry(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
void smp_resp_sc_passkey_entry(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
void smp_init_sc_oob_method(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
void smp_resp_sc_oob_method(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
void smp_init_secure_rx_handle(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
void smp_resp_secure_rx_handle(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
void smp_init_rx_handle(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
void smp_resp_rx_handle(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
void smp_receive_packet(gap_conn_item_t *conn, smp_opcode_t opcode, const uint8_t *pdu_data, uint16_t pdu_len);
void gatt_report_mtu_changed(att_conn_item_t *conn);
void gatt_cache_save(att_conn_item_t *conn);
att_bearer_t *att_bearer_add_new(att_conn_item_t *conn, bt_l2cap_param_t *l2cap, uint8_t bearer_id);
void att_bearer_close(att_bearer_t *att, uint8_t reason);
uint16_t att_recv_request(att_bearer_t *att, const uint8_t *pdu_data, uint16_t pdu_len);
void att_recv_response(att_bearer_t *att, gatt_proc_node_t *proc, att_response_t *rsp);
void att_bearer_recv_req_packet(att_bearer_t *att);
void att_receive_packet(att_bearer_t *att, struct pp_buff *ppb);
int att_l2cap_callback(uintptr_t connhdl, bt_l2cap_event_t event, bt_l2cap_callback_param_t param);
void att_bearer_send_cmd_or_ntf(att_bearer_t *att);
void att_bearer_process_req_ind_q(att_bearer_t *att);
void gatt_server_conn_handle(att_conn_item_t *conn, gatt_server_event_t event, gatt_server_callback_param_t param);
int gatt_conn_event_handler(uintptr_t connhdl, gap_conn_event_t event, gap_conn_callback_param_t param);
gatt_svc_node_t *gatt_add_svc_to_conn(att_conn_item_t *conn, gatt_service_t *s, bool only_add_svc, bool thread_report);
bool gatt_send_notify_for_each_conn(att_conn_item_t *conn, void *param);
bool gatt_server_recv_read_req(struct attr_context_t *ctx, uint16_t value_offset);
bool gatt_server_recv_write_req(struct attr_context_t *ctx, uint16_t value_offset, const uint8_t *value_ptr, uint16_t value_len, bool prepare_check);
bool gatt_for_each_attr_callback(struct attr_context_t *ctx);
void gatt_server_check_notify_permission(att_conn_item_t *conn, const gatt_multi_notify_item_t *char_item, uint16_t count, bool need_confirm);

typedef bt_status_t (*gatt_conn_ready_handler_func_t)(gap_conn_item_t *conn);
typedef ble_bdaddr_t (*gap_impl_get_peer_resolved_addr_func_t)(gap_conn_item_t *conn);
typedef ble_bdaddr_t (*gap_get_peer_resolved_address_func_t)(uint16_t connhdl);
typedef void (*gap_report_connection_event_func_t)(gap_conn_item_t *conn, gap_conn_event_t event, uint8_t error_code, void *extra);
typedef void (*gap_recv_smp_encrypted_func_t)(gap_conn_item_t *conn, bool new_pair, uint8_t error_code);
typedef bool (*gap_address_reso_check_activity_func_t)(void);
typedef void (*gap_resolving_list_check_activity_func_t)(void);
typedef bt_status_t (*gap_enable_address_resolution_func_t)(bool enable);
typedef void (*gap_filter_list_check_activity_func_t)(void);
typedef void (*gap_adv_conn_complete_handle_func_t)(uint8_t status, gap_conn_item_t *conn, struct hci_ev_le_connection_cmpl *p);
typedef bt_status_t (*gap_impl_set_adv_parameters_func_t)(gap_advertising_t *adv);
typedef bt_status_t (*gap_parse_curr_adv_parameters_func_t)(uint8_t adv_handle, const gap_adv_param_t *param, gap_adv_callback_t cb, gap_advertising_t *adv);
typedef bool (*gap_adv_has_same_parameters_func_t)(const gap_advertising_t *adv, const gap_advertising_t *new_adv);
typedef uint8_t (*gap_create_advertising_func_t)(const gap_adv_param_t *param, gap_adv_callback_t cb, bool refresh_adv, uint8_t adv_handle);
typedef void (*gap_select_adv_timing_func_t)(gap_advertising_t *adv);
typedef void (*gap_select_scan_timing_func_t)(gap_scanning_t *scan);
typedef bt_status_t (*gap_impl_set_scan_params_func_t)(gap_scanning_t *scan);
typedef void (*gap_select_init_timing_func_t)(gap_initiating_t *init);
typedef void (*gap_init_conn_complete_handle_func_t)(uint8_t status, gap_conn_item_t *conn, struct hci_ev_le_connection_cmpl *p);
typedef bt_status_t (*gap_impl_le_create_conn_func_t)(gap_initiating_t *init);
typedef bt_status_t (*gap_impl_start_initiating_func_t)(uint32_t adv_handle_subevent_bleaudio, const gap_init_param_t *param, gap_init_callback_t cb);
typedef void (*gap_le_conn_is_established_func_t)(gap_conn_item_t *conn, bt_addr_type_t own_addr_type, const bt_bdaddr_t *own_addr, gap_conn_callback_t callback, uint8_t from_adv_handle, bool directed);
typedef const uint8_t *(*smp_get_local_address_func_t)(gap_conn_item_t *conn, uint8_t *addr_type);
typedef const uint8_t *(*smp_get_peer_address_func_t)(gap_conn_item_t *conn, uint8_t *addr_type);
typedef bt_status_t (*smp_security_requirements_check_func_t)(gap_conn_item_t *conn, smp_requirements_t *r, bool check_mitm);
typedef bt_status_t (*smp_prepare_for_encryption_func_t)(gap_conn_item_t *conn, bool is_central);
typedef void (*smp_receive_peer_ltk_req_func_t)(gap_conn_item_t *conn, struct hci_ev_le_ltk_request *p);
typedef bool (*smp_reply_ltk_req_func_t)(smp_conn_item_t *smp);
typedef bt_status_t (*smp_start_pairing_request_func_t)(gap_conn_item_t *conn, const smp_auth_requirements_t *peer_auth_req);
typedef bt_status_t (*smp_start_security_request_func_t)(gap_conn_item_t *conn);
typedef smp_error_code_t (*smp_select_pairing_method_func_t)(smp_conn_item_t *smp);
typedef void (*smp_distribute_keys_func_t)(smp_conn_item_t *smp, uint8_t key_dist, uint8_t peer_kdist);
typedef bool (*smp_wait_remote_key_distribute_func_t)(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
typedef void (*smp_start_phase_3_key_distribute_func_t)(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv, void (*key_distribute_func)(smp_conn_item_t *, smp_event_t, const smp_receive_t *));
typedef smp_error_code_t (*smp_phase_transfer_func_t)(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv, const smp_transfer_t *trans, int count);
typedef void (*smp_init_sc_gen_ltk_and_encrypt_func_t)(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
typedef void (*smp_resp_sc_gen_ltk_and_encrypt_func_t)(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
typedef void (*smp_init_sc_numeric_compare_func_t)(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
typedef void (*smp_resp_sc_numeric_compare_func_t)(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
typedef void (*smp_init_sc_passkey_entry_func_t)(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
typedef void (*smp_resp_sc_passkey_entry_func_t)(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
typedef void (*smp_init_sc_oob_method_func_t)(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
typedef void (*smp_resp_sc_oob_method_func_t)(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
typedef void (*smp_init_secure_rx_handle_func_t)(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
typedef void (*smp_resp_secure_rx_handle_func_t)(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
typedef void (*smp_init_rx_handle_func_t)(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
typedef void (*smp_resp_rx_handle_func_t)(smp_conn_item_t *smp, smp_event_t event, const smp_receive_t *recv);
typedef void (*smp_receive_packet_func_t)(gap_conn_item_t *conn, smp_opcode_t opcode, const uint8_t *pdu_data, uint16_t pdu_len);
typedef void (*gatt_report_mtu_changed_func_t)(att_conn_item_t *conn);
typedef void (*gatt_cache_save_func_t)(att_conn_item_t *conn);
typedef att_bearer_t *(*att_bearer_add_new_func_t)(att_conn_item_t *conn, bt_l2cap_param_t *l2cap, uint8_t bearer_id);
typedef void (*att_bearer_close_func_t)(att_bearer_t *att, uint8_t reason);
typedef uint16_t (*att_recv_request_func_t)(att_bearer_t *att, const uint8_t *pdu_data, uint16_t pdu_len);
typedef void (*att_recv_response_func_t)(att_bearer_t *att, gatt_proc_node_t *proc, att_response_t *rsp);
typedef void (*att_bearer_recv_req_packet_func_t)(att_bearer_t *att);
typedef void (*att_receive_packet_func_t)(att_bearer_t *att, struct pp_buff *ppb);
typedef int (*att_l2cap_callback_func_t)(uintptr_t connhdl, bt_l2cap_event_t event, bt_l2cap_callback_param_t param);
typedef void (*att_bearer_send_cmd_or_ntf_func_t)(att_bearer_t *att);
typedef void (*att_bearer_process_req_ind_q_func_t)(att_bearer_t *att);
typedef void (*gatt_server_conn_handle_func_t)(att_conn_item_t *conn, gatt_server_event_t event, gatt_server_callback_param_t param);
typedef int (*gatt_conn_event_handler_func_t)(uintptr_t connhdl, gap_conn_event_t event, gap_conn_callback_param_t param);
typedef gatt_svc_node_t *(*gatt_add_svc_to_conn_func_t)(att_conn_item_t *conn, gatt_service_t *s, bool only_add_svc, bool thread_report);
typedef bool (*gatt_send_notify_for_each_conn_func_t)(att_conn_item_t *conn, void *param);
typedef bool (*gatt_server_recv_read_req_func_t)(struct attr_context_t *ctx, uint16_t value_offset);
typedef bool (*gatt_server_recv_write_req_func_t)(struct attr_context_t *ctx, uint16_t value_offset, const uint8_t *value_ptr, uint16_t value_len, bool prepare_check);
typedef bool (*gatt_for_each_attr_callback_func_t)(struct attr_context_t *ctx);
typedef void (*gatt_server_check_notify_permission_func_t)(att_conn_item_t *conn, const gatt_multi_notify_item_t *char_item, uint16_t count, bool need_confirm);

#ifdef __cplusplus
}
#endif
#endif /* BUILD_BTH_ROM */
#endif /* __BTH_PATCH_BASIC_H__ */