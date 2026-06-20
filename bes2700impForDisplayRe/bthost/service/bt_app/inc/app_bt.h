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
#ifndef __APP_BT_H__
#define __APP_BT_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "me_api.h"
#include "a2dp_api.h"
#include "hfp_api.h"
#include "avrcp_api.h"

typedef uint8_t second_sco_decision_t;
#define IBRT_REJECT_SECOND_SCO                   0x00
#define IBRT_ACCEPT_SECOND_SCO_DISC_FIRST_SCO    0x01
#define IBRT_ACCEPT_SECOND_SCO_KEEP_FIRST_SCO    0x02
#define IBRT_HOST_DECIDE_SECONED_SCO             0x03
#define IBRT_ACCEPT_SECOND_SCO                   0x04

typedef uint8_t second_sco_bg_action_t;
#define IBRT_ACTION_ROUTE_SCO_TO_PHONE 0x00
#define IBRT_ACTION_HOLD_ACTIVE_SCO    0x01

typedef uint8_t virtual_call_handle_t;
#define VIRTUAL_HANDLE_NORMAL_WAY      0x00
#define VIRTUAL_HANDLE_NON_PROMPT      0x01
#define VIRTUAL_HANDLE_NON_PROMPTED    0x02

enum APP_BT_GOLBAL_HANDLE_HOOK_USER_T {
    APP_BT_GOLBAL_HANDLE_HOOK_USER_0 = 0,
    APP_BT_GOLBAL_HANDLE_HOOK_USER_1,
    APP_BT_GOLBAL_HANDLE_HOOK_USER_QTY
};

#ifdef RESUME_MUSIC_AFTER_CRASH_REBOOT
typedef struct
{
    bt_bdaddr_t addr;
} app_bt_curr_palyback_device_t;
#endif

enum BT_PAIR_TYPE
{
    APP_BT_PAIR_NONE      = 0,
    APP_BT_PAIRED         = 1
};

#define GET_HCI_HANDLE(ptr)    ((unsigned short)(((unsigned short) *((ptr)+1) << 8) | ((unsigned short) *(ptr))))

//Sub code of Vendor event
#define HCI_DBG_TRACE_WARNING_EVT_CODE      0x01
#define HCI_SCO_SNIFFER_STATUS_EVT_CODE     0x02
#define HCI_ACL_SNIFFER_STATUS_EVT_CODE     0x03
#define HCI_TWS_EXCHANGE_CMP_EVT_CODE       0x04
#define HCI_NOTIFY_CURRENT_ADDR_EVT_CODE    0x05
#define HCI_START_SWITCH_EVT_CODE           0x09
#define HCI_LL_MONITOR_EVT_CODE             0x0A
#define HCI_DBG_LMP_RESERVED_EVT_SUBCODE    0x0B
#define HCI_GET_TWS_SLAVE_MOBILE_RSSI_CODE            0x0C
#define HCI_DBG_IBRT_SWITCH_COMPLETE_EVT_SUBCODE      0x0D
#define HCI_DBG_IBRT_CONNECTED_EVT_SUBCODE            0x0E
#define HCI_DBG_IBRT_DISCONNECTED_EVT_SUBCODE         0x0f
#define HCI_DBG_SNIFFER_INIT_CMP_EVT_SUBCODE          0x10
#define HCI_DBG_RX_SEQ_ERROR_EVT_SUBCODE              0x11
#define HCI_DBG_FA_TO_EVT_SUBCODE                     0x14
#define HCI_DBG_ECC_DATA_TEST_EVT_SUBCODE             0x15
#define HCI_DBG_MULTI_IBRT_ACL_SCH_CHG_EVT_SUBCODE    0x16
#define HCI_DBG_PERIODIC_ADVERT_CONFLICT_EVT_SUBCODE  0x1f
#define HCI_DBG_TX_PWR_CHG_EVT_SUBCODE                0x24

#ifdef IS_TWS_IBRT_DEBUG_SYSTEM_ENABLED
#define HCI_DBG_LINK_LOSS_INFO_EVT_SUBCODE            0x27
#define HCI_DBG_LINK_LOSS_CLOCK_INFO_EVT_SUBCODE      0x28
#define HCI_DBG_A2DP_SINK_INFO_EVT_SUBCODE            0x29
#endif

#define HCI_DBG_SCO_SWITCH_RESULT_EVT_SUBCODE         0x2a

//ext_opcode for reserved vendor event
//HCI_DBG_LMP_RESERVED_EVT_SUBCODE
#define HCI_BLOCK_CHANNEL_REPORT_EXT_OPCODE    0x1

// type: ACL_CONNECT/ACL_DISCONNECT event;
// addr: peer device address;
// para: event reason. For ACL_CONNECT event, it is acl link status reason; For ACL_DISCONNECT event, it is disconnect
// status reason.
typedef void (*bt_acl_conn_state_change_cb_t)(uint8_t type,bt_bdaddr_t* addr, uint32_t para);
typedef void (*bt_pair_state_change_cb_t)(bt_bdaddr_t* addr, uint32_t para);
typedef void (*bt_sink_event_global_cb_t)(const btif_event_t * event);
typedef void (*bt_audio_coex_event_cb_t)(bt_bdaddr_t* addr, enum app_bt_audio_event_t event, uint32_t data);

void app_bt_reconnect_a2dp_profile(const bt_bdaddr_t *mobile_addr);
void app_bt_reconnect_hfp_profile(const bt_bdaddr_t * mobile_addr);
void app_bt_reconnect_avrcp_profile(const bt_bdaddr_t *mobile_addr);

void app_bt_disconnect_a2dp_profile(a2dp_stream_t * a2dp);
void app_bt_disconnect_avrcp_profile(btif_avrcp_channel_t * avrcp);
void app_bt_disconnect_hfp_profile(btif_hf_channel_t * hfp);

void app_bt_query_hfp_sdp_when_mobile_reconnect(uint8_t device_id);
void app_bt_response_acl_conn_req(bt_bdaddr_t *remote, bool accept);
uint8_t app_bt_count_mobile_link(void);
uint8_t app_bt_get_active_cons(void);
bt_status_t app_bt_set_fix_tws_interval_param(uint16_t duration, uint16_t interval, uint16_t interval_in_sco);

const char* app_bt_a2dp_get_all_device_streaming_state(void);
const char* app_bt_a2dp_get_all_device_state(void);
const char* app_bt_hf_get_all_device_state(void);
const char *app_bt_get_device_current_roles(void);

void app_bt_disconnect_link_by_id(uint8_t device_id);
void app_bt_disconnect_link_byaddr(bt_bdaddr_t * remote);

void app_bt_switch_to_non_prompt_disc_a2dp_play_mode(void);
void app_bt_switch_to_multi_a2dp_quick_switch_play_mode(void);

struct app_bt_profile_manager;

void app_bt_reset_profile_manager(struct app_bt_profile_manager *mgr);

void app_bt_a2dp_state_checker(void);
void app_bt_hfp_state_checker(void);

void app_bt_link_state_checker(void);
bool app_bt_checker_print_link_state(const char* tag, btif_remote_device_t *btm_conn);

const char* app_bt_get_active_media_state(void);

void app_bt_a2dp_send_set_abs_volume(uint8_t device_id, uint8_t volume);

void app_bt_a2dp_send_key_request(uint8_t device_id, uint8_t a2dp_key);

bool app_bt_a2dp_service_is_connected(uint8_t device_id);
uint16_t app_bt_get_handle_by_device_id(uint8_t device_id);
void app_bt_active_mode_set(BT_ACTIVE_MODE_KEEP_USER_T user, uint32_t link_id);
void app_bt_active_mode_clear(BT_ACTIVE_MODE_KEEP_USER_T user, uint32_t link_id);

void app_disconnect_all_bt_connections(bool power_off_flag);
void app_bt_local_volume_up(void (*cb)(uint8_t device_id));
void app_bt_local_volume_down(void (*cb)(uint8_t device_id));
void app_bt_local_volume_set(uint16_t volume, void (*cb)(uint8_t device_id));

void app_bt_set_a2dp_default_abs_volume(uint8_t volume);
void app_bt_update_a2dp_default_abs_volume(int device_id, uint8_t volume);
void app_bt_set_a2dp_current_abs_volume(int device_id, uint8_t volume);
uint8_t app_bt_get_a2dp_current_abs_volume(int device_id);
void app_bt_a2dp_current_abs_volume_just_set(int device_id, uint8_t volume);
void app_bt_a2dp_abs_volume_mix_version_handled(int device_id);
a2dp_stream_t *app_bt_get_mobile_a2dp_stream(uint32_t deviceId);

void app_bt_volumeup(void);
void app_bt_volumedown(void);

bool app_bt_get_tx_power_idx(uint16_t handle, int8_t *tx_power_idx);

bool app_bt_update_tx_power_idx(uint16_t handle, int8_t tx_power_idx);

#ifdef BT_AVRCP_SUPPORT
bt_status_t app_bt_avrcp_key_operation(const bt_bdaddr_t *remote, avrcp_panel_operation_t key, bool is_press);
#endif

#ifdef IBRT
const char *app_bt_get_profile_exchanged_state(void);
void app_bt_ibrt_mobile_link_state_checker(void);
bool app_bt_ibrt_has_mobile_link_connected(void);
bool app_bt_ibrt_has_snoop_link_connected(void);
void app_bt_ibrt_reconnect_mobile_profile(const bt_bdaddr_t *mobile_addr);
#endif

typedef void (*APP_BT_REQ_CONNECT_PROFILE_FN_T)(void *, void *);
typedef bt_status_t  (*APP_BT_REQ_HF_OP_FN_T)(void *);
typedef void (*APP_BT_GOLBAL_HANDLE_HOOK_HANDLER)(const btif_event_t*Event);
typedef void (*APP_BT_LE_READ_CHANNEL_MAP_CALLBACK) (void *);

#define app_bt_accessmode_set_req(accmode) do{app_bt_send_request(APP_BT_REQ_ACCESS_MODE_SET, accmode, 0, 0,0);}while(0)

#ifdef __SOURCE_TRACE_RX__
typedef struct
{
    uint32_t index;
    bt_bdaddr_t addr;
    char name[256];
}device_info_t;
#endif

int bes_bt_cfg_vendor_codec_en(void);

int bes_bt_cfg_dont_play_mute_when_a2dp_stuck(void);

int bes_bt_cfg_mark_some_code_for_fuzz_test(void);

int bes_bt_cfg_keep_only_one_stream_close_connected_a2dp(void);

#ifdef IBRT
int bes_bt_gap_tws_acl_is_connected(void);
uint8_t *bes_bt_gap_get_tws_remote_addr(void);
#endif

int bes_bt_a2dp_get_last_paused_device(void);

int bes_bt_a2dp_set_last_paused_device(int device_id);

void app_bt_process_cmd_complete_read_le_host_chnl_map(uint8 *data);

void app_bt_host_fault_dump_trace(void);

void bt_apply_full_ecdh_key_pair(void);

void bt_apply_ecdh_key_pair(void);

void app_bt_active_mode_manager_init(void);

void app_bt_active_mode_reset(uint32_t linkIndex);

void app_bt_get_remote_device_name(const bt_bdaddr_t * bdaddr);

void app_bt_inquiry_remote_device_name(const bt_bdaddr_t * bdaddr);

void app_bt_global_handle(const btif_event_t *Event);

bool is_app_bt_pairing_running(void);

void PairingTransferToConnectable(void);

bt_status_t LinkDisconnectDirectly(bool PowerOffFlag);

void app_bt_global_handle_init(void);

void app_bt_opening_reconnect(void);

int app_bt_send_request(uint32_t message_id, uint32_t param0, uint32_t param1, uint32_t param2,uint32_t ptr);

void app_bt_init(void);

int app_bt_state_checker(void);

void app_start_fast_connectable_ble_adv(uint16_t advInterval);

void *app_bt_profile_active_store_ptr_get(uint8_t *bdAddr);

void app_bt_profile_connect_manager_opening_reconnect(void);

BOOL app_bt_profile_connect_openreconnecting(void *ptr);

int app_bt_global_handle_hook_set(enum APP_BT_GOLBAL_HANDLE_HOOK_USER_T user, APP_BT_GOLBAL_HANDLE_HOOK_HANDLER handler);

APP_BT_GOLBAL_HANDLE_HOOK_HANDLER app_bt_global_handle_hook_get(enum APP_BT_GOLBAL_HANDLE_HOOK_USER_T user);

bool app_is_hfp_service_connected(uint8_t device_id);

void bt_generate_ecdh_key_pair(void);

void bt_generate_full_ecdh_key_pair(void);

#if defined(IBRT)
void app_bt_ibrt_reconnect_mobile_profile_flag_set(bt_bdaddr_t* remote_addr);
void app_bt_ibrt_reconnect_mobile_profile_flag_clear(bt_bdaddr_t* remote_addr);
bool app_bt_ibrt_reconnect_mobile_profile_flag_get(bt_bdaddr_t* remote_addr);

void app_bt_ibrt_connect_mobile_a2dp_profile(const bt_bdaddr_t *addr);
void app_bt_ibrt_connect_mobile_hfp_profile(const bt_bdaddr_t *addr);

void app_bt_device_snoop_acl_connected(uint8_t device_id, void* remote, void* btm_conn);
void app_bt_device_snoop_acl_disconnected(uint8_t device_id, void* remote);
#endif

#if defined(GET_PEER_RSSI_ENABLE)
void app_bt_ibrt_rssi_status_checker(void);
#endif

char *app_bt_get_global_state_buffer(void);

void app_bt_set_keep_active_mode(bool set_keep, BT_ACTIVE_MODE_KEEP_USER_T user, uint32_t link_id);

void app_bt_stay_active(uint8_t deviceId);

void app_bt_allow_sniff(int device_id);

void app_bt_conn_stop_sniff(uint16_t conn_handle);

void app_bt_stop_sniff(uint8_t deviceId);

void app_bt_sniff_config(uint16_t conn_handle);

bool app_bt_has_mobile_device_in_sniff_mode(void);

bool app_bt_has_mobile_device_in_active_mode(void);

void app_bt_allow_sniff_rem_dev(uint16_t conn_handle);

void app_bt_stay_active_rem_dev(uint16_t conn_handle);

void app_check_pending_stop_sniff_op(void);

uint8_t app_bt_count_connected_device(void);

struct BT_DEVICE_T * app_bt_get_connected_sink_device(void);

struct BT_DEVICE_T *app_bt_get_connected_device_byaddr(const bt_bdaddr_t *remote);

void app_bt_pause_media_player_again(uint8_t deviceId);

bool app_bt_is_music_player_working(uint8_t deviceId);

bool app_bt_pause_music_player(uint8_t deviceId);

void app_bt_resume_music_player(uint8_t deviceId);

bool app_bt_is_device_profile_connected(uint8_t deviceId);

bool app_bt_is_acl_connected(uint8_t device_id);

bool app_bt_is_sco_connected(uint8_t device_id);

bool app_bt_is_hfp_connected(uint8_t device_id);

uint8_t app_bt_get_device_id_byaddr(const bt_bdaddr_t *remote);

uint16_t app_bt_get_handle_byaddr(const bt_bdaddr_t *remote);

uint8_t* app_bt_get_addr_byhandle(uint16_t conn_hdl);

uint8_t app_bt_get_device_id_byhandle(uint16_t conn_handle);

uint8_t app_bt_get_connected_device_id_byaddr(const bt_bdaddr_t *remote);

bool app_bt_is_a2dp_connected(uint8_t device_id);

bool app_bt_is_acl_connected_byaddr(const bt_bdaddr_t *remote);

bool app_bt_is_sco_connected_byaddr(bt_bdaddr_t *remote);

bool app_bt_is_hfp_connected_byaddr(bt_bdaddr_t *remote);

bool app_bt_is_a2dp_connected_byaddr(bt_bdaddr_t *remote);

bool app_bt_is_a2dp_streaming(uint8_t deviceId);

void app_bt_suspend_a2dp_streaming(uint8_t deviceId);

bool app_bt_get_device_bdaddr(uint8_t deviceId, uint8_t* btAddr);

void fast_pair_enter_pairing_mode_handler(void);

bool app_bt_is_in_reconnecting(void);

bool app_bt_is_profile_connected_before(uint8_t device_id);

bool btapp_hfp_is_dev_sco_connected(uint8_t devId);

bool btapp_hfp_current_is_virtual_call(uint8_t device_id);

bool app_bt_is_in_connecting_profiles_state(void);

void app_bt_clear_connecting_profiles_state(uint8_t devId);

void app_bt_set_connecting_profiles_state(uint8_t devId);

bool app_bt_is_hfp_audio_on(void);

bt_bdaddr_t* app_bt_get_remote_device_address(uint8_t device_id);

btif_remote_device_t *app_bt_get_remote_dev_by_handle(uint16_t conn_handle);

btif_remote_device_t *app_bt_get_remote_dev_by_address(const bt_bdaddr_t *remote);

btif_remote_device_t *app_bt_get_remote_dev_by_device_id(uint8_t device_id);

uint8_t app_bt_get_remote_sync_id_by_bdaddr(bt_bdaddr_t *bdaddr);

bool app_is_disconnecting_all_bt_connections(void);

void app_set_disconnecting_all_bt_connections(bool isEnable);

void app_bt_start_search(void);

void app_bt_start_search_with_callback(void (*cb)(bt_bdaddr_t *remote), void (*result_cb)(app_bt_search_result_t *result));

void app_bt_del_search_except_device(const bt_bdaddr_t *addr);

void app_bt_add_search_except_device(const bt_bdaddr_t *addr);

void app_bt_clear_search_except_device_list(void);

typedef void (*inquiryResponseCallback_t)(uint8_t *btAddr, uint8_t *deviceName, uint8_t *eir);
/**
 ****************************************************************************************
 * @brief register inquiry_response_callback to get bt addr, device name and eir.
 *
 * @param[in] inquiryResponseCallback_t callback
 ****************************************************************************************
 */
void app_bt_register_inquiry_response_callback(inquiryResponseCallback_t callback);

/**
 ****************************************************************************************
 * @brief bt start inquiry.
 *
 * @param[in] inquiryLastingTime        Inquiry lasting time, uint is 1.28s.
                                        Range: 0x01 to 0x30
                                        Time(seconds) = inquiryLastingTime * 1.28s
                                        Range: 1.28 to 61.44 s
 ****************************************************************************************
 */
void app_bt_start_inquiry(uint8_t inquiryLastingTime);

/**
 ****************************************************************************************
 * @brief bt stop inquiry.
 *
 ****************************************************************************************
 */
void app_bt_stop_inquiry(void);

/**
 ****************************************************************************************
 * @brief bt set channel classification map which related Set AFH Host Channel Classification command
 * AFH_Host_Channel_Classification: Size: 10 octets (79 bits meaningful)
 * This parameter contains 80 1-bit fields
 * The nth such field (in the range 0 to 78) contains the value for channel n:
 *    0: channel n is bad
 *    1: channel n is unknown
 *
 * The most significant bit (bit 79) is reserved for future use
 * At least (Nmin == 20) channels shall be marked as unknown.
 * default all bits value is 1
 *
 ****************************************************************************************
 */
void app_bt_set_chnl_classification(uint8_t *chnl_map);

void app_bt_read_le_chnl_map(uint16_t conn_handle);
void app_bt_set_le_host_chnl_classification(uint8_t *chnl_map);
void app_bt_registe_read_le_chnl_map_callback(APP_BT_LE_READ_CHANNEL_MAP_CALLBACK cb);

#if defined(IBRT)
uint32_t app_bt_save_bd_addr_ctx(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);
uint32_t app_bt_restore_bd_addr_ctx(bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);
#ifdef BT_A2DP_SUPPORT
uint32_t app_bt_save_a2dp_app_ctx(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);
uint32_t app_bt_restore_a2dp_app_ctx(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);
#endif
uint32_t app_bt_save_avrcp_app_ctx(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);
uint32_t app_bt_restore_avrcp_app_ctx(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);
uint32_t app_bt_save_hfp_app_ctx(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);
uint32_t app_bt_restore_hfp_app_ctx(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);
uint32_t app_bt_save_spp_app_ctx(uint64_t app_id,const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);
uint32_t app_bt_restore_spp_app_ctx(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len, uint64_t app_id);
#ifdef BT_MAP_SUPPORT
uint32_t app_bt_save_map_app_ctx(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);
uint32_t app_bt_restore_map_app_ctx(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);
#endif
#ifdef BT_HID_DEVICE
uint32_t app_bt_save_hid_app_ctx(btif_remote_device_t *rem_dev, uint8_t *buf, uint32_t buf_len);
uint32_t app_bt_restore_hid_app_ctx(bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);
#endif
bool app_bt_is_any_connection(void);
#endif

void app_stop_fast_connectable_ble_adv_timer(void);
int8_t app_bt_get_rssi(void);

#ifdef  TILE_DATAPATH
int8_t app_tile_get_ble_rssi(void);
#endif

#ifdef CTKD_ENABLE
bool app_bt_ctkd_is_connecting_mobile_pending(void);
void app_bt_ctkd_connecting_mobile_handler(void);
#endif

void app_bt_get_remote_cod(uint8_t *cod0, uint8_t *cod1);
bool app_bt_get_remote_cod_by_addr(const bt_bdaddr_t *bd_ddr, uint8_t *cod);

void app_bt_report_audio_retrigger(uint8_t retrgigerType);
void app_bt_reset_tws_acl_data_packet_check(void);
void app_bt_update_link_monitor_info(uint8_t *ptr);
void app_bt_acl_data_packet_check(uint8_t *data);
#ifdef RESUME_MUSIC_AFTER_CRASH_REBOOT
void app_bt_resume_music_after_crash_reboot(uint8_t device_id);
void app_bt_resume_music_after_crash_reboot_init(void);
void app_bt_reset_curr_playback_device(uint8_t device_id);
#endif
void app_bt_sleep_init(void);
void app_bt_reset_rssi_collector(void);
int32_t app_bt_tx_rssi_analyzer(int8_t rssi);
void app_bt_user_passkey_notify_callback(btif_remote_device_t *remdev,uint32_t passkey);
void app_bt_user_passkey_request_callback(btif_remote_device_t *remdev);

struct BT_DEVICE_T *app_bt_manager_get_device_all_start(void);

struct BT_DEVICE_T *app_bt_manager_get_device_all_end(void);

bool app_bt_need_disconnect_profiles_before_acl(btif_remote_device_t *remote_device);
void app_bt_disconnect_all_profiles_and_acl(btif_remote_device_t *remote_device);

bool app_bt_is_remote_device_support_le_audio(const bt_bdaddr_t *p_addr);

#if BLE_AUDIO_ENABLED
void app_bt_set_device_support_le_audio(const bt_bdaddr_t *p_addr);
#endif

bool app_bt_is_sco_connected_exist(void);

uint8_t app_bt_find_connected_device(void);

void app_bt_disconnect_link_by_id(uint8_t device_id);

void app_bt_disconnect_link_byaddr(bt_bdaddr_t *remote);

void app_bt_disconnect_acl_link(void);

void app_bt_disconnect_all_acl_link(void);

uint8_t app_bt_audio_count_connected_hfp(void);

void app_bt_enable_bt_pcm(bool en);

void app_bt_reset_device_info(uint8_t device_id);

uint8_t app_bt_a2dp_hfp_adjust_volume(uint8_t device_id, uint8_t stream_player, uint8_t vol);

void app_bt_pair_state_callback_register(bt_pair_state_change_cb_t cb);

void app_bt_pair_state_callback_register(bt_pair_state_change_cb_t cb);

void app_bt_pair_state_callback_deregister(void);

bt_pair_state_change_cb_t app_bt_get_pair_state_callback(void);

int bt_pairing_init(void);

#ifdef NV_RECORD_DEV_NAME
uint8_t *app_get_current_remote_device_name(void);
#endif

btif_accessible_mode_t app_bt_get_curr_access_mode(void);

void app_bt_access_mode_ctrl_init(void);

btif_accessible_mode_t app_bt_access_mode_pickup(void);

bt_status_t app_bt_set_access_mode(btif_accessible_mode_t mode);

void app_bt_access_mode_change_event_handle(const btif_event_t *event, uint8_t error_code);

uint8_t app_bt_hfp_streaming_sco_count(void);

bool app_bt_check_is_ios_device(const bt_bdaddr_t *remote_addr);

void app_bt_register_coex_sink_global_event_handle(bt_sink_event_global_cb_t func);

void app_bt_register_coex_audio_sink_event_handler(bt_audio_coex_event_cb_t func);

void app_bt_coex_sink_global_event_ind(const btif_event_t * event);

void app_bt_coex_audio_sink_event_ind(bt_bdaddr_t *addr, enum app_bt_audio_event_t event, uint32_t data);

uint32_t app_bt_get_class_of_device_headset(void);

uint32_t app_bt_get_class_of_device_watch(void);

typedef bool (*app_bt_inquiry_t)(const btif_event_t* event);
void app_bt_callback_register(app_bt_inquiry_t func);

#ifdef __cplusplus
}
#endif
#endif /* BESBT_H */
