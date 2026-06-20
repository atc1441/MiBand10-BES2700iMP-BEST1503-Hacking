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
#ifndef __APP_UI_API_H__
#define __APP_UI_API_H__
#include "app_ibrt_conn_evt.h"
#include "app_tws_ibrt.h"
#include "app_ui_evt.h"
#include "app_tws_ibrt_core_type.h"
#include "app_ui_ipscan_mgr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void (*ibrt_global_state_changed)(ibrt_global_state_change_event *state);
    void (*ibrt_a2dp_state_changed)(const bt_bdaddr_t *addr, ibrt_conn_a2dp_state_change *state);
    void (*ibrt_hfp_state_changed)(const bt_bdaddr_t *addr, ibrt_conn_hfp_state_change *state);
    void (*ibrt_avrcp_state_changed)(const bt_bdaddr_t *addr, ibrt_conn_avrcp_state_change *state);
    void (*ibrt_tws_pairing_changed)(ibrt_conn_pairing_state state, uint8_t reason_code);
    void (*ibrt_tws_acl_state_changed)(ibrt_conn_tws_conn_state_event *state, uint8_t reason_code);
    void (*ibrt_mobile_acl_state_changed)(const bt_bdaddr_t *addr, ibrt_mobile_conn_state_event *state, uint8_t reason_code);
    void (*ibrt_sco_state_changed)(const bt_bdaddr_t *addr, ibrt_sco_conn_state_event *state, uint8_t reason_code);
    void (*ibrt_tws_role_switch_status_ind)(const bt_bdaddr_t *addr, ibrt_conn_role_change_state state, ibrt_role_e role);
    void (*ibrt_ibrt_state_changed)(const bt_bdaddr_t *addr, ibrt_connection_state_event *state, ibrt_role_e role, uint8_t reason_code);
    void (*ibrt_access_mode_changed)(btif_accessible_mode_t newAccessMode);
} ibrt_link_status_changed_cb_t;

typedef struct {
    bool (*accept_incoming_conn_req_hook)(const bt_bdaddr_t *addr);
    bool (*accept_extra_incoming_conn_req_hook)(const bt_bdaddr_t *incoming_addr, bt_bdaddr_t *steal_addr);
    bool (*disallow_start_reconnect_mob_hook)(const bt_bdaddr_t *addr, const uint16_t active_event);
    bool (*disallow_start_reconnect_tws_hook)(void);
    bool (*disallow_tws_role_switch_hook)(void);
} ibrt_ext_conn_policy_cb_t;

typedef struct {
    void (*ibrt_mgr_event_run_comp_hook)(app_ui_evt_t box_evt);
    void (*ibrt_mgr_pairing_mode_entry_hook)();
    void (*ibrt_mgr_pairing_mode_exit_hook)();
    void (*ibrt_mgr_pairing_mode_timeout_pre_hook)();
    void (*ibrt_mgr_pairing_mode_timeout_post_hook)();
    void (*ibrt_mgr_tws_role_switch_comp_hook)(TWS_UI_ROLE_E current_role, uint8_t errCode);
    void (*peer_box_state_update_hook)(bud_box_state box_state);
    void (*pre_handle_box_event_hook)(app_ui_evt_t box_evt);
} ibrt_mgr_status_changed_cb_t;

typedef void (*ibrt_vender_event_handler_ind)(uint8_t, uint8_t *, uint8_t);

typedef enum {
    // dont disc any mob
    IBRT_PAIRING_DISC_NONE = 0,
    // disc all already connected mob when enter pairing
    IBRT_PAIRING_DISC_ALL_MOB,
    // support multipoint & current already exist two link, but only disc one when enter pairing
    IBRT_PAIRING_DISC_ONE_MOB,
} ibrt_pairing_with_disc_num;

typedef enum {
    // just disconnect sco and resume when pairing exit
    IBRT_PAIRING_DISC_SCO = 0,
    // hungup call
    IBRT_PAIRING_HF_HUNGUP,
} ibrt_pairing_with_disc_hf_cfg;

typedef struct
{
    uint32_t rx_seq_error_timeout;
    uint32_t rx_seq_error_threshold;
    uint32_t rx_seq_recover_wait_timeout;
    uint32_t rssi_monitor_timeout;
    uint32_t tws_conn_failed_wait_time;
    uint32_t connect_no_03_timeout;
    uint32_t disconnect_no_05_timeout;
    uint32_t tws_cmd_send_timeout;
    uint32_t tws_cmd_send_counter_threshold;

    /// freeman mode config, default should be false
    bool freeman_enable;
    /// enable sdk lea adv strategy, default should be true
    bool sdk_lea_adv_enable;
    /// pairing mode timeout value config
    uint32_t pairing_timeout_value;
    /// SDK pairing enable, the default should be true
    bool sdk_pairing_enable;
    /// passive enter pairing when no mobile record, the default should be false
    bool enter_pairing_on_empty_record;
    /// passive enter pairing when reconnect failed, the default should be false
    bool enter_pairing_on_reconnect_mobile_failed;
    /// passive enter pairing when mobile disconnect, the default should be false
    bool enter_pairing_on_mobile_disconnect;
    /// exit pairing when peer close, the default should be true
    bool exit_pairing_on_peer_close;
    /// exit pairing when a2dp or hfp streaming, the default should be true
    bool exit_pairing_on_streaming;
    /// disconnect remote devices when entering pairing mode actively
    uint8_t paring_with_disc_mob_num;
    /// disconnect remote le devices when entering pairing mode actively
    uint8_t paring_with_disc_le_mob_num;
    /// disconnect SCO or not when accepting phone connection in pairing mode
    uint8_t pairing_with_disc_hf_cfg;
    /// pause current music when entering pairing mode
    bool pairing_with_pause_music;
    /// not start ibrt in pairing mode, the default should be false
    bool pairing_without_start_ibrt;
    /// set nv master to tws master for lea connection
    bool pairing_with_set_nv_master_as_master;
    /// accept new dev only in pairing state
    bool accept_new_dev_only_in_pairing;
    /// disconnect tws immediately when single bud closed in box
    bool disc_tws_imm_on_single_bud_closed;
    ///no need to reconnect mobile when easbuds in busy mode
    bool disallow_reconnect_in_streaming_state;
    /// open(enable) without auto reconnect
    bool open_without_auto_reconnect;
    /// do tws switch when RSII value change, default should be true
    bool tws_switch_according_to_rssi_value;
    /// controller basband monitor
    bool lowlayer_monitor_enable;
    bool support_steal_connection;
    bool check_plugin_excute_closedbox_event;
    //if add ai feature
    bool ibrt_with_ai;
    bool giveup_reconn_when_peer_unpaired;

    bool tws_switch_tx_data_protect;

    bool without_reconnect_if_remote_driving_disc;
    bool without_reconnect_when_fetch_out_wear_up;
    bool disallow_rs_by_box_state;
    bool only_rs_when_sco_active;
    bool reject_same_cod_device_conn_req;
    /// do tws switch when rssi value change over threshold
    uint8_t rssi_threshold;
    /// do tws switch when RSII value change, timer threshold
    uint8_t role_switch_timer_threshold;
    uint8_t audio_sync_mismatch_resume_version;

    uint8_t  profile_concurrency_supported;

    /// close box delay disconnect tws timeout
    uint16_t close_box_delay_tws_disc_timeout;

    /// close box debounce time config
    uint16_t close_box_event_wait_response_timeout;
    /// reconnect event internal config wait timer when tws disconnect
    uint16_t reconnect_wait_ready_timeout;
    uint16_t reconnect_mobile_wait_ready_timeout;
    uint16_t reconnect_tws_wait_ready_timeout;

    /// wait time before launch reconnect event
    uint16_t reconnect_mobile_wait_response_timeout;
    uint16_t reconnect_ibrt_wait_response_timeout;
    uint16_t nv_master_reconnect_tws_wait_response_timeout;
    uint16_t nv_slave_reconnect_tws_wait_response_timeout;

    /// open box reconnect mobile times config
    uint16_t open_reconnect_mobile_max_times;
    /// open box reconnect tws times config
    uint16_t open_reconnect_tws_max_times;
    /// connection timeout reconnect mobile times config
    uint16_t reconnect_mobile_max_times;
    /// connection timeout reconnect tws times config
    uint16_t reconnect_tws_max_times;

    /// connection timeout reconnect ibrt times config
    uint16_t reconnect_ibrt_max_times;
    uint16_t mobile_page_timeout;

    /// tws connection supervision timeout
    uint16_t tws_connection_timeout;

    uint16_t radical_scan_interval_nv_slave;
    uint16_t radical_scan_interval_nv_master;

    uint16_t scan_interval_in_sco_tws_disconnected;
    uint16_t scan_window_in_sco_tws_disconnected;

    uint16_t scan_interval_in_sco_tws_connected;
    uint16_t scan_window_in_sco_tws_connected;

    uint16_t scan_interval_in_a2dp_tws_disconnected;
    uint16_t scan_window_in_a2dp_tws_disconnected;

    uint16_t scan_interval_in_a2dp_tws_connected;
    uint16_t scan_window_in_a2dp_tws_connected;

    bool support_steal_connection_in_sco;
    bool support_steal_connection_in_a2dp_steaming;
    bool steal_audio_inactive_device;
    bool allow_sniff_in_sco;
    bool always_interlaced_scan;
    uint8_t llmonitor_report_format;
    uint32_t llmonitor_report_count;

    bool is_changed_to_ui_master_on_tws_disconnected;
    /// if tws&mobile disc, reconnect tws first until tws connected then reconn mobile
    bool delay_reconn_mob_until_tws_connected;
    uint8_t delay_reconn_mob_max_times;
    // handle lea multiple connect cfg
    bool  connected_max_device_num_allow_new_connect;
    bool lea_connected_allow_new_connect;
    bool dev_idle_allow_nonsupport_stay_connected;
} app_ui_config_t;

typedef enum {
    TWS_CHNL_USER_EARBUD,
    TWS_CHNL_USER_SOUND,
    TWS_CHNL_USER_MAX,
} tws_chnl_user_t;

typedef void (*tws_chnl_recv_func_t)(uint8_t *, uint16_t);

void app_ui_init();

app_ui_config_t* app_ui_get_config();

void *app_ui_get_devices_ctx(uint8_t index);

void app_ui_custom_role_switch_cb_ind(const bt_bdaddr_t *addr, ibrt_conn_role_change_state state, ibrt_role_e role);

int app_ui_reconfig_env(app_ui_config_t *config);

void *app_ui_get_box_evt_que(void);

void app_ui_push_box_evt_into_queue(const void* user_action_evt);

bool app_ui_sync_box_state(bud_box_state box_state);

void app_ui_dump_status();

void app_ui_monitor_dump(void);

void app_ui_handle_vender_event(uint8_t evt_type, uint8_t * buffer, uint32_t length);

bud_box_state app_ui_get_local_box_state(void);

void app_ui_set_local_box_state(bud_box_state state);

bud_box_state app_ui_get_peer_box_state(void);

void app_ui_set_peer_box_state(bud_box_state box_state);

void app_ui_update_scan_type_policy(scanUpdateEvent event);

void app_ui_send_tws_reconnect_event(app_ui_evt_t reconect_event);

void app_ui_send_mobile_reconnect_event(uint8_t link_id);

void app_ui_send_mobile_reconnect_event_by_addr(const bt_bdaddr_t* mobile_addr);

void app_ui_choice_mobile_connect(const bt_bdaddr_t* mobile_addr, uint8_t try_count);

uint8_t app_ui_get_connected_remote_dev_count();

bool app_ui_any_mobile_device_connected(void);

bool app_ui_max_mobile_device_connected(void);

bool app_ui_allow_steal_connection();

void app_ui_set_accepting_device(const bt_bdaddr_t *addr);

bool app_ui_accept_conn_req_ongoing();

bool app_ui_is_accepting_device(const bt_bdaddr_t *addr);

void app_ui_clr_pending_incoming_conn_req(void);

bool app_ui_accept_pending_incoming_conn_req(bt_bdaddr_t *addr);

void app_ui_register_link_status_callback(ibrt_link_status_changed_cb_t *cb);

void app_ui_register_mgr_status_callback(ibrt_mgr_status_changed_cb_t *cb);

void app_ui_register_ext_conn_policy_callback(ibrt_ext_conn_policy_cb_t *cb);

void app_ui_register_local_stream_callback(bool(*cb)(const bt_bdaddr_t *addr, uint8_t state));

void app_ui_register_vender_event_update_ind(ibrt_vender_event_handler_ind handler);

void best_mgr_pairing_mode_entry_hook();

void best_mgr_pairing_mode_exit_hook();

void best_mgr_pairing_mode_timeout_pre_hook();

void best_mgr_pairing_mode_timeout_post_hook();

void best_mgr_tws_role_switch_comp_hook(TWS_UI_ROLE_E current_role, uint8_t errCode);

bool app_ui_is_addr_null(bt_bdaddr_t *addr);

bool app_ui_pop_from_pending_list(bt_bdaddr_t *addr);

bool app_ui_get_from_pending_list(bt_bdaddr_t *addr);

void app_ui_push_device_to_pending_list(bt_bdaddr_t *addr);

uint16_t app_ui_pending_conn_req_list_size(void);

bool app_ui_event_has_been_queued(const bt_bdaddr_t* remote_addr,app_ui_evt_t event);

app_ui_evt_t app_ui_get_active_event(const bt_bdaddr_t* remote_addr);

bool app_ui_high_priority_event_interrupt_reconnec();

bool app_ui_disallow_reconnect_mobile_by_peer_status(void);

void app_ui_send_mobile_disconnect_event(const bt_bdaddr_t* mobile_addr);

bool app_ui_notify_peer_to_destroy_device(const bt_bdaddr_t *addr, bool delete_record);

bool app_ui_destroy_device_ongoing(void);

bool app_ui_destroy_device(const bt_bdaddr_t *del_nv_addr, bool delete_record);

void app_ui_destroy_the_other_device(const bt_bdaddr_t *active_addr, bool need_delete_nv);

void app_ui_map_valid_device2nvrecord(void);

void app_ui_load_device_from_record_when_no_instance(void);

bool app_ui_has_available_mobile_resource(void);

void app_ui_keep_only_mobile_link();

/**
 ****************************************************************************************
 * @brief To disconnect all BT connections
 *
 ****************************************************************************************
 */
void app_ui_disconnect_all(void);

/**
 ****************************************************************************************
 * @brief To disconnect all connections and then shutdown the system
 *
 ****************************************************************************************
 */
void app_ui_shutdown(void);

void app_ui_set_enter_pairing_flag(void);

void app_ui_clr_enter_pairing_flag(void);

bool app_ui_is_enter_pairing_mode(void);

void app_ui_pairing_ctl_init(void);

/**
 ****************************************************************************************
 * @brief To get current pairing mode state
 *
 * @return true                     In pairing mode
 * @return false                    Not in pairing mode
 ****************************************************************************************
 */
bool app_ui_in_pairing_mode(void);

/**
 ****************************************************************************************
 * @brief Enter pairing mode
 *
 * @param timeout                   Pairing timeout value
 * @param notify_peer               If true, notify peer enter pairing
 ****************************************************************************************
 */
void app_ui_enter_pairing_mode(uint32_t timeout, bool notify_peer);

/**
 ****************************************************************************************
 * @brief Exit pairing mode
 *
 * @param notify_peer               If true, notify peer exit pairing
 ****************************************************************************************
 */
void app_ui_exit_pairing_mode(bool notify_peer);

void app_ui_pairing_evt_handler(pairing_evt_e evt, uint32_t para);

bool app_ui_need_disconnect_mob_on_pairing(const bt_bdaddr_t *addr);

bool app_ui_allow_start_ibrt_in_pairing(void);

bool app_ui_in_tws_mode(void);

bool app_ui_support_bleaudio(void);

bool app_ui_support_multipoint(void);

/**
 ****************************************************************************************
 * @brief role switch > ibrt role-switch
 *
 * @param[in] switch2master         if true will switch to master else to slave
 ****************************************************************************************
 */
bool app_ui_user_role_switch(bool switch2master);

/**
 ****************************************************************************************
 * @brief user get current ui role
 *
 ****************************************************************************************
 */
TWS_UI_ROLE_E app_ui_get_current_role(void);

/**
 ****************************************************************************************
 * @brief user get is any connected device doing role switch
 *
 ****************************************************************************************
 */
bool app_ui_role_switch_ongoing(void);

/**
 ****************************************************************************************
 * @brief trigger role switch by local and peer box state
 *
 ****************************************************************************************
 */
void app_ui_trigger_role_switch_by_box_state(void);

/* TODO: will be removed */
bool app_ui_change_mode(bool enable_bleaudio, bool enable_multipoint, const bt_bdaddr_t *addr);

/* TODO: will be removed */
bool app_ui_change_mode_ext(bool enable_leaudio, bool enable_multipoint, const bt_bdaddr_t *addr);

uint8_t app_ui_support_max_links();

void app_ui_set_multipoint_mode(multipoint_mode_t mode);

bool app_ui_change_support_max_links(multipoint_mode_t mode, const bt_bdaddr_t reserved_addrs[], int reserved_count);

bool app_ui_change_support_max_links_ext(multipoint_mode_t mode, const bt_bdaddr_t reserved_addrs[], int reserved_count);
/**
 ****************************************************************************************
 * @brief enable/disable page function
 *
 * @param enable                    If true, enable page; else, disable page
 ****************************************************************************************
 */
void app_ui_set_page_enabled(bool enable);

/**
 ****************************************************************************************
 * @brief Get current enable or disable page
 *
 * @return true                     Enable page
 * @return false                    Disable Page
 ****************************************************************************************
 */
bool app_ui_enabled_page(void);

bool app_ui_need_delay_mob_reconn();

bool app_ui_custom_disallow_reconn_dev(const bt_bdaddr_t *addr, uint16_t active_event);

bool app_ui_custom_disallow_reconn_tws(void);

/**
 ****************************************************************************************
 * @brief exit eabud mode for enter other mode
 *
 * @return None
 ****************************************************************************************
 */
void app_ui_exit_earbud_mode(void);

/**
 * @brief Get remote BT name
 *
 * @param remote_addr           - [in]  Remote device BT address
 * @param nameStr               - [out] name string pointer
 * @param nameLen               - [out] name length pointer
 * @return true                 - Success
 * @return false                - Failed
 */
bool app_ui_get_remote_name(const bt_bdaddr_t *remote_addr, char *nameStr, uint8_t *nameLen);

/**
 * @brief Cancel Page
 *
 * @param mobile                - Cancel page mobile if true
 * @param tws                   - Cancel page tws if true
 */
void app_ui_cancel_page(bool mobile, bool tws);

/**
 * @brief is disconnect event
 *
 * @param evt                - event
 */
bool app_ui_is_disconnect_evt(app_ui_evt_t evt);

/**
 * @brief is reconnect event
 *
 * @param evt                - event
 */
bool app_ui_is_reconnect_evt(app_ui_evt_t evt);

/**
 * @brief put the device at the top of NV record
 *
 * @param device address
 */
 void app_ui_put_dev_at_nv_top(const bt_bdaddr_t *addr);

/**
 ****************************************************************************************
 * @brief Except the TWS earbud, the other project without open/close box, dock/undock,
 * wear up/down, is suggested to use the following interfaces:
 * 1. enable relevant interface
 * 2. start access management
 * 3. start reconnect the latest one/two mobile by config
 ****************************************************************************************
 */
bool app_ui_bt_open();

/**
 ****************************************************************************************
 * @brief Except the TWS earbud, the other project without open/close box, dock/undock,
 * wear up/down, is suggested to use the following interfaces:
 * 1. disable access
 * 2. disconnect all bt connections
 * 3. disable relevant interface
 ****************************************************************************************
 */
bool app_ui_bt_close();

/**
 ****************************************************************************************
 * @brief Auto connect function:
 * 1. start reconnect TWS in TWS mode if TWS not connect
 * 2. start reconnect the latest one/two mobile
 ****************************************************************************************
 */
bool app_ui_bt_auto_connect();

/**
 * @brief send msg(event) to btmgr
 *
 * @param e                     - event
 */
bool best_btmgr_send_msg(app_ui_evt_t e);

bool best_btmgr_tws_chnl_register(tws_chnl_user_t user, tws_chnl_recv_func_t user_handle);

bool best_btmgr_tws_chnl_send_data(tws_chnl_user_t user, uint8_t *data_buf, uint16_t data_len);

void app_ui_cancel_page_by_addr(bt_bdaddr_t *remote);

void bt_update_pscan_para(uint16_t scan_interval, uint16_t scan_window);
void bt_update_iscan_para(uint16_t scan_interval, uint16_t scan_window);
void bt_set_pscan_enabled(bool enable);
void bt_set_iscan_enabled(bool enable);

#if BLE_AUDIO_ENABLED
#include "bluetooth_ble_api.h"

typedef struct
{
    void (*ble_audio_adv_state_changed)(AOB_ADV_STATE_T state, uint8_t err_code);
    void (*mob_acl_state_changed)(uint8_t conidx, const ble_bdaddr_t* addr, AOB_ACL_STATE_T state, uint8_t errCode);
    void (*vol_changed_cb)(uint8_t con_lid, uint8_t volume, uint8_t mute);
    void (*vocs_offset_changed_cb)(int16_t offset, uint8_t output_lid);
    void (*vocs_bond_data_changed_cb)(uint8_t output_lid, uint8_t cli_cfg_bf);
    void (*media_track_change_cb)(uint8_t con_lid);
    void (*media_stream_status_change_cb)(uint8_t con_lid, uint8_t ase_lid, AOB_MGR_STREAM_STATE_E state);
    void (*media_playback_status_change_cb)(uint8_t con_lid, AOB_MGR_PLAYBACK_STATE_E state);
    void (*media_mic_state_cb)(uint8_t mute);
    void (*media_iso_link_quality_cb)(AOB_ISO_LINK_QUALITY_INFO_T param);
    void (*media_pacs_cccd_written_cb)(uint8_t con_lid);
    void (*call_state_change_cb)(uint8_t con_lid, void *param);
    void (*call_srv_signal_strength_value_ind_cb)(uint8_t con_lid, uint8_t value);
    void (*call_status_flags_ind_cb)(uint8_t con_lid, bool inband_ring, bool silent_mode);
    void (*call_ccp_opt_supported_opcode_ind_cb)(uint8_t con_lid, bool local_hold_op_supported, bool join_op_supported);
    void (*call_terminate_reason_ind_cb)(uint8_t con_lid, uint8_t call_id, uint8_t reason);
    void (*call_incoming_number_inf_ind_cb)(uint8_t con_lid, uint8_t url_len, uint8_t *url);
    void (*call_svc_changed_ind_cb)(uint8_t con_lid);
    void (*call_action_result_ind_cb)(uint8_t con_lid, void *param);
    void (*ble_bis_sink_status_cb)(uint8_t grp_lid, uint8_t state, uint32_t stream_pos_bf);
    void (*ble_bis_sink_stream_status_cb)(uint8_t grp_lid, bool status);
    void (*ble_bis_deleg_source_add_ri_cb)(uint8_t src_lid, uint8_t con_lid, uint8_t pa_sync_req);
}app_ble_audio_event_cb_t;

/**
 ****************************************************************************************
 * @brief Register custom ble audio callback
 *
 * @param[in] cb                    Custom LEA callback
 ****************************************************************************************
 */
void app_ui_register_custom_ui_le_aud_callback(app_ble_audio_event_cb_t* cb);

/**
 ****************************************************************************************
 * @brief Return true if any ble audio device connected
 ****************************************************************************************
 */
bool app_ui_any_ble_audio_links(void);

/**
 ****************************************************************************************
 * @brief Return true if any ble audio device connected
 *
 * @param[in] conidx                The LE connection index
 ****************************************************************************************
 */
void app_ui_keep_only_the_leaudio_device(uint8_t conidx);


// TODO:: will move to app_ui.h
/**
 ****************************************************************************************
 * @brief Return true if any ble audio device connected
 *
 * @param[in] conidx                The LE connection index
 ****************************************************************************************
 */
void app_ui_notify_bt_nv_recored_changed(bt_bdaddr_t* mobile_addr);

/**
 ****************************************************************************************
 * @brief Return true if need disconnect current le device on enter pairing
 *
 * @param[in] addr                  The LE connection address
 ****************************************************************************************
 */
bool app_ui_need_disconnect_le_mob_on_pairing(const ble_bdaddr_t *addr);

#endif

#ifdef __cplusplus
}
#endif
#endif /* __APP_UI_API_H__ */
