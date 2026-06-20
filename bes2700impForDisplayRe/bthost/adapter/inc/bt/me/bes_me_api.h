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
#ifndef __BES_BT_ME_API_H__
#define __BES_BT_ME_API_H__
#include "me_common_define.h"
#ifdef __cplusplus
extern "C" {
#endif

#define ble_buffer_malloc(size) ble_buffer_malloc_with_ca((size), (uint32_t)(uintptr_t)__builtin_return_address(0), __LINE__)
#define ble_buffer_free(buffer) ble_buffer_free_with_ca((buffer), (uint32_t)(uintptr_t)__builtin_return_address(0), __LINE__)
void *ble_buffer_malloc_with_ca(uint16_t size, uint32_t ca, uint32_t line);
void ble_buffer_free_with_ca(void *buffer, uint32_t ca, uint32_t line);

int bt_audio_event_handler(uint8_t device_id, enum app_bt_audio_event_t event, uint32_t data);
void bt_audio_set_event_handler(int (*hdlr)(uint8_t device_id, enum app_bt_audio_event_t event, uint32_t data));

#define BT_BES_CFG_VENDOR_CODEC_EN 1
#define BT_BES_CFG_DONT_PLAY_MUTE_WHEN_A2DP_STUCK 2
#define BT_BES_CFG_MARK_CODE_FOR_FUZZ_TEST 3
#define BT_BES_CFG_KEEP_ONLY_ONE_STREAM_CLOSE_CONNECTED_A2DP 4
#define BT_BES_CFG_BT_SINK_ENABLED 5
#define BT_BES_CFG_PTS_TEST_DONT_ROLE_SWITCH 6

void bes_bt_app_start_inquiry(void);

void bes_bt_app_stop_inquiry(void);

bt_status_t bes_bt_me_start_inquiry(uint32_t lap, uint8_t len, uint8_t maxResp);

bt_status_t bes_bt_me_set_ble_adv_data(uint8_t len, uint8_t * data);

bt_status_t bes_bt_me_set_ble_scan_rsp_data(U8 len, U8 * data);

bt_status_t bes_bt_me_set_ble_adv_parameters(btif_adv_para_struct_t * para);

bt_status_t bes_bt_me_set_ble_adv_en(uint8_t en);

bt_status_t bes_bt_me_set_ble_scan_parameter(btif_scan_para_struct_t * para);

bt_status_t bes_bt_me_set_ble_scan_en(uint8_t scan_en, uint8_t filter_duplicate);

bt_status_t bes_bt_me_receive_ble_adv_report(void (*cb)(const btif_ble_adv_report* report));

bt_status_t bes_bt_me_read_ble_chnle_map(uint16_t conn_handle);

bt_status_t bes_bt_me_set_ble_host_chnl_classification(uint8_t *chnl_map);

void bes_bt_sco_response(uint8_t device_id, uint8_t accept);

bt_status_t bes_bt_me_setup_acl_qos_with_tpoll_generic(uint16_t conn_handle, uint32_t tpoll_slot, uint8_t service_type);

bool bes_bt_me_is_in_active_mode(bt_bdaddr_t *remote);

bt_status_t bes_bt_me_dbg_set_iso_quality_rep_thr(uint16_t conn_handle, uint16_t qlty_rep_evt_cnt_thr,
        uint16_t tx_unack_pkts_thr, uint16_t tx_flush_pkts_thr, uint16_t tx_last_subevent_pkts_thr,
        uint16_t retrans_pkts_thr, uint16_t crc_err_pkts_thr, uint16_t rx_unreceived_pkts_thr,
        uint16_t duplicate_pkts_thr);

bt_status_t bes_bt_me_dbg_set_txpwr_link_thd(uint8_t index, uint8_t enable,uint8_t link_id, uint16_t rssi_avg_nb_pkt,
        int8_t rssi_high_thd, int8_t rssi_low_thd, int8_t rssi_below_low_thd, int8_t rssi_interf_thd);

uint8_t bes_bt_me_set_io_capabilities(uint8_t ioCap);

uint8_t bes_bt_me_set_authrequirements(uint8_t authRequirements);

int bes_bt_me_set_bt_local_name(const unsigned char *device_name, uint8_t len);

void bes_bt_me_set_extended_inquiry_response(uint8_t* eir, uint32_t len);

void bes_bt_me_send_bt_key(APP_KEY_STATUS *status);

uint32_t bes_bt_me_bes_cfg_get(int cfg);

void bes_bt_me_reset_bt_controller(void);

bt_status_t bes_bt_me_set_bt_address(const uint8_t *local_addr);

bt_status_t bes_bt_me_set_ble_address(const uint8_t *local_addr);

bt_status_t bes_bt_me_set_bt_name(char *localName, uint8_t length);

bt_status_t bes_bt_me_set_ble_name(char *localName, unsigned char length);

void bes_bt_me_set_stay_active_mode(bool keep_active_mode, BT_ACTIVE_MODE_KEEP_USER_T user, uint32_t link_id);

uint8_t bes_bt_me_get_device_id_from_address(const bt_bdaddr_t *remote);

uint8_t bt_export_count_link(BT_COUNT_LINK_ENUM_T type);

uint8_t bt_export_select_device(BT_SELECT_DEVICE_ENUM_T op);

uint8_t bt_export_select_another_device(BT_SELECT_ANOTHER_DEVICE_ENUM_T op, int curr_device_id);

bt_acl_state_t bt_export_acl_get_state_by_rdev(btif_remote_device_t *remote_dev);

bt_status_t bt_export_me_write_access_mode(btif_accessible_mode_t mode,bool disable_before_update);

uint8_t bes_bt_me_count_link(BT_COUNT_LINK_ENUM_T type);

uint8_t bes_bt_me_get_max_sco_number(void);

void bes_bt_me_write_scan_type(BT_SCAN_ENUM_T op, uint8_t scan_type);

void bes_bt_me_write_scan_activity(BT_SCAN_ENUM_T op, uint16_t scan_interval, uint16_t scan_window);

bt_status_t bes_bt_me_write_page_timeout(uint16_t page_timeout_slots);

bt_status_t bes_bt_me_write_access_mode(btif_accessible_mode_t mode,bool disable_before_update);

void bes_bt_me_fast_pair_enter_pairing_mode_handler(void);

uint8_t bes_bt_me_select_device(BT_SELECT_DEVICE_ENUM_T op);

uint8_t bes_bt_me_select_another_device(BT_SELECT_ANOTHER_DEVICE_ENUM_T op, int curr_device_id);

bool bes_bt_me_profile_is_connected_before(uint8_t device_id);

void *bes_bt_me_cmgr_get_handler(uint16_t conn_handle);

bt_bdaddr_t *bes_bt_me_cmgr_get_address(void *cmgr_handler);

btif_sniff_info_t *bes_bt_me_cmgr_get_sniff_info(void *cmgr_handler);

bt_status_t bes_bt_me_cmgr_set_sniff_info(uint16_t conn_handle, btif_sniff_info_t *sniff_info);

bt_status_t bes_bt_me_cmgr_set_sniff_timer(void *cmgr_handler, btif_sniff_info_t *sniff_info, void *timer_mgr);

void bes_bt_me_cmgr_start_sniff_timer(evm_timer_t* timer, uint32_t time);

bt_status_t bes_bt_me_cmgr_register_callback(void *cmgr_handler, btif_cmgr_callback callback);

void bes_bt_me_acl_set_remote_device_role(uint16_t conn_handle, uint8_t role);

bool bes_bt_me_is_device_profile_connected(uint8_t device_id);

bool bes_bt_me_is_device_hfp_connected(uint8_t device_id);

bool bes_bt_me_is_device_a2dp_connected(uint8_t device_id);

bt_acl_state_t bes_bt_me_acl_get_state(int device_id);

bt_acl_state_t bes_bt_me_acl_get_state_by_address(const bt_bdaddr_t *remote);

bt_acl_state_t bes_bt_me_acl_get_state_by_handle(uint16_t conn_handle);

bt_status_t bes_bt_me_acl_force_disconnect(uint16_t conn_handle, uint8_t reason, bool force_disc);

bt_status_t bes_bt_me_acl_connect_req(const bt_bdaddr_t *remote);

bt_status_t bes_bt_me_acl_disconnect_req(const bt_bdaddr_t* remote);

void bes_bt_me_acl_disc_all_bt_links(bool power_off_flag);

bt_status_t bes_bt_me_acl_set_policy(uint16_t conn_handle, btif_link_policy_t policy);

bt_status_t bes_bt_me_acl_switch_role(uint16_t conn_handle);

bt_status_t bes_bt_me_acl_start_sniff(uint16_t conn_handle, btif_sniff_info_t *info);

bt_status_t bes_bt_me_acl_stop_sniff(uint16_t conn_handle);

bt_status_t bes_bt_me_acl_accept_link_req(const bt_bdaddr_t *remote, btif_connection_role_t role);

bt_status_t bes_bt_me_acl_reject_link_req(const bt_bdaddr_t *remote, btif_error_code_t reason);

bt_status_t bes_bt_me_acl_auth_req(uint16_t conn_handle);

bt_remver_t bes_bt_me_acl_get_remote_version(uint16_t conn_handle);

bt_status_t bes_bt_me_acl_set_link_monitor(uint16_t conn_handle, uint8_t control_flag, uint8_t report_format,
        uint32_t data_format, uint8_t report_unit);

void bes_bt_me_response_acl_conn_req(bt_bdaddr_t *remote, bool accept);

uint8_t bes_bt_me_acl_get_event_type(const btif_event_t *event);

uint8_t bes_bt_me_acl_get_event_errcode(const btif_event_t *event);

uint16_t bes_bt_me_acl_get_event_handle(const btif_event_t *event);

bt_bdaddr_t *bes_bt_me_acl_get_event_address(const btif_event_t *event);

uint8_t bes_bt_me_get_event_access_mode(const btif_event_t *event);

uint8_t bes_bt_me_get_event_encrypt_mode(const btif_event_t *event);

/**
 * @brief Write automatic flush timeout on BR/EDR acl
 *
 * @param[in] connhandle     Conn handle
 * @param[in] flush_timeout  Rang 0x0001 to 0x07FF, in unit 0.625ms, 0x0000 is No Automatic Flush
 *
 * @return bt_status_t
 */
bt_status_t bes_bt_me_write_automatic_flush_timeout(uint16 connhandle, uint16 flush_timeout);

bt_bdaddr_t *bes_bt_me_get_addr_by_sco_handle(uint16_t handle);

void bes_bt_me_report_inquiry_result(uint8_t *inquiry_buff, bool rssi, bool extended_mode);

void bes_bt_me_handle_bt_key_click(void);

int bes_bt_call_func_in_app_thread(uint32_t message_id, uint32_t param0, uint32_t param1, uint32_t param2,uint32_t ptr);

int bes_bt_call_func_in_bt_thread(uint32_t param0, uint32_t param1, uint32_t param2, uint32_t param3, uint32_t funcPtr);

int bes_bt_call_custom_function_in_bt_thread(uint32_t param0, uint32_t param1, uint32_t funcPtr);

void bes_bt_app_init(void);

void bes_bt_sleep_init(void);

bool bes_bt_me_is_bt_thread(void);

void bes_bt_me_gen_addr_for_debug(void);

void bes_bt_me_transfer_pairing_to_connectable(void);

int8_t bes_bt_me_get_bt_rssi(void);

void bes_bt_me_opening_reconnect(void);

void bes_bt_me_pair_state_callback_deregister(void);

uint32_t bes_bt_get_class_of_device(void);

void bes_bt_osapi_lock_stack(void);

void bes_bt_osapi_unlock_stack(void);

void bes_bt_osapi_notify_evm(void);

bool bes_bt_me_get_remote_cod_by_addr(const bt_bdaddr_t *bd_ddr, uint8_t *cod);

void *bes_bt_me_cmgr_create_handler(void);

btif_remote_device_t* bes_bt_get_remote_dev_by_handle(uint16_t hci_handle);

uint8_t bes_bt_get_active_cons(void);

int8 bes_bt_l2cap_send_data( uint32 l2cap_handle, uint8 *data, uint32 datalen, void *context);

void bes_bt_l2cap_create_besaud_extra_channel(void* remote_addr, bt_l2cap_callback_t l2cap_callback);

void bes_bt_l2cap_reset_sigid(const bt_bdaddr_t *addr);

btif_remote_device_t* bes_bt_get_remote_dev_by_address(const bt_bdaddr_t *bdaddr);

bt_status_t bes_bt_set_fix_tws_interval_param(uint16_t duration, uint16_t interval, uint16_t interval_in_sco);

struct btm_conn_item_t *bes_bt_ibrt_create_new_snoop_link(void *remote, uint16 conn_handle);

btif_remote_device_t *bes_bt_get_remote_dev_by_device_id(uint8_t device_id);

void bes_bt_conn_ibrt_disconnected_handle(struct btm_conn_item_t *btm_conn);

void bes_bt_device_snoop_acl_disconnected(uint8_t device_id, void* addr);

bt_status_t bes_bt_me_set_accessible_mode(btif_accessible_mode_t mode,
                                        const btif_access_mode_info_t *info);

bt_status_t bes_bt_me_cancel_create_connection(const bt_bdaddr_t *remote);

bt_status_t bes_bt_get_remote_dev_addr_by_device_id(uint8_t deviceId, uint8_t* btAddr);

bt_status_t bes_bt_get_remote_device_id_by_addr(uint8_t* btAddr, uint8_t *deviceId);

uint8_t bes_bt_get_remote_device_id_by_handle(uint16_t conn_hdl);

uint16_t bes_bt_get_remote_conn_handle_by_addr(uint8_t* btAddr);

bt_status_t bes_bt_get_remote_addr_by_conn_handle( uint16_t conn_hdl, uint8_t* btAddr);

void bes_bt_me_confirmation_register_callback(btif_confirmation_req_callback_t callback);

void bes_bt_me_confirmation_resp(struct bdaddr_t *bdaddr, bool accept);

#ifdef __IAG_BLE_INCLUDE__
void bes_bt_me_stop_fast_connectable_ble_adv(void);
#endif

#ifdef __INTERACTION__
uint8_t bes_bt_me_get_findme_fadein_vol(void);

void bes_bt_me_set_findme_fadein_vol(uint8_t vol);
#endif

void *bes_bt_me_profile_active_store_ptr_get(uint8_t *bdAddr);

int Besbt_hook_handler_set(enum BESBT_HOOK_USER_T user, BESBT_HOOK_HANDLER handler);

/// BT APP memory api
unsigned char *bes_bt_me_malloc(uint16 size, uint32 ca, uint32 line);

void bes_bt_me_free(unsigned char *buf, uint32 ca, uint32 line);

#define bes_bt_buf_malloc(size) bes_bt_me_malloc((size), (uint32_t)(uintptr_t)__builtin_return_address(0), __LINE__)

#define bes_bt_buf_free(buf) bes_bt_me_free((unsigned char *)buf, (uint32_t)(uintptr_t)__builtin_return_address(0), __LINE__)

bool bes_bt_check_buffer_available(uint16_t size);

#ifdef __cplusplus
}
#endif
#endif /* __BES_BT_ME_API_H__ */
