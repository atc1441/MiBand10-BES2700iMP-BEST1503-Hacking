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
#ifndef __APP_BLE_H__
#define __APP_BLE_H__
#include "gatt_service.h"
#include "ble_core_common.h"
#include "ble_ai_voice.h"
#include "app_ble_test.h"
#include "app_custom.h"
#include "nvrecord_extension.h"
#include "ble_datapath_server.h"
#include "ble_datapath_client.h"
#include "ble_tws.h"
#ifdef __cplusplus
extern "C" {
#endif

#define BLE_BASIC_ADV_HANDLE        0x00 // for legacy ble, ota, ai, dp, tile etc. advertising
#define BLE_AUDIO_ADV_HANDLE        0x01 // for ble audio adertising
#define BLE_GFPS_ADV_HANDLE         0x02
#define BLE_SPOT_ADV_HANDLE         0x03
#define BLE_SWIFT_ADV_HANDLE        0x04
#define BLE_FINDMY_ADV_HANDLE       0x05
#define BLE_CUSTOMER0_ADV_HANDLE    0x06
#define BLE_CUSTOMER1_ADV_HANDLE    0x07
#define BLE_CUSTOMER2_ADV_HANDLE    0x08
#define BLE_CUSTOMER3_ADV_HANDLE    0x09
#define BLE_MAX_FIXED_ADV_HANDLE    0x0a
#define BLE_MAX_CORE_EVT_CB         5

#if defined(BISTO_ENABLED)||defined(__AI_VOICE_BLE_ENABLE__)|| \
    defined(CTKD_ENABLE)||defined(GFPS_ENABLED)||(BLE_AUDIO_ENABLED)
#ifndef BLE_USB_AUDIO_USE_LE_LAGACY_NO_SECURITY_CON
#ifndef CFG_SEC_CON
#define CFG_SEC_CON
#endif
#endif
#endif

#define FAST_PAIR_REV_2_0   1
#define BLE_APP_GFPS_VER    FAST_PAIR_REV_2_0
#undef CFG_APP_GFPS

#ifdef GFPS_ENABLED
#if BLE_APP_GFPS_VER==FAST_PAIR_REV_2_0
#define CFG_APP_GFPS
#endif
#endif

typedef struct ble_adv_activity_t
{
    uint8_t adv_handle;
    bool adv_is_started;
    BLE_ADV_USER_E user;
    uint32_t custom_adv_interval_ms;
    gap_adv_param_t adv_param;
    bool (*adv_activity_func)(struct ble_adv_activity_t *adv);
    bt_bdaddr_t local_rpa;
#ifdef BLE_WATCH_ADAPTER
    bool adv_from_adapter;
#endif
} ble_adv_activity_t;

typedef struct
{
    ble_callback_evnet_type_e evt_type;
    union
    {
        struct
        {
            uint8_t newRole;
        } rs_complete_handled;
        struct
        {
            uint8_t newRole;
        } role_update_handled;
        struct
        {
            uint8_t event;
        } ibrt_event_entry_handled;
    } p;
} ble_callback_event_t;

typedef struct
{
    uint8_t io_cap;
    uint8_t has_oob_data;
    uint8_t auth_req;
    uint8_t max_enc_key_size;
    uint8_t init_key_dist;
    uint8_t resp_key_dist;
} ble_smp_require_t;

typedef struct
{
    uint16_t conn_interval_1_25ms; // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4000ms
    uint16_t peripheral_latency; // num of subrated conn events, 0x00 to 0x1F3 (499)
    uint16_t superv_timeout_ms; // 0x0A to 0x0C80 * 10ms, 100ms to 32s
    uint16_t central_clock_accuracy; // only valid on Peripheral, on Central shall be set to 0x00
    uint16_t subrate_factor; // 0x01 to 0x01F4, subrate factor applied to the specified underlying conn interval
    uint16_t conn_continuation_number; // 0x00 to 0x01F3, num of underlying conn events to remain active after a packet contain a LL PDU with non-zero length is sent or received
} ble_conn_timing_t;

typedef void (*APP_BLE_CORE_EVENT_CALLBACK)(ble_event_t *);
typedef void (*app_ble_mtu_exch_cb_t)(uint8_t con_lid, uint32_t mtu_size);

typedef struct
{
    ble_adv_activity_t adv[BLE_ADV_USER_NUM];
    BLE_DATA_FILL_FUNC_T data_fill_func[BLE_ADV_USER_NUM];
    bool data_fill_enable[BLE_ADV_USER_NUM];
    uint32_t adv_force_disabled;
    APP_BLE_CORE_EVENT_CALLBACK ble_core_evt_cb;
    APP_BLE_CORE_GLOBAL_HANDLER_FUNC ble_global_handler[BLE_MAX_CORE_EVT_CB];
    set_rsp_dist_lk_bit_field_func dist_lk_set_cb;
    smp_identify_addr_exch_complete ble_smp_info_derived_from_bredr_complete;
    void (*ble_global_handle)(ble_event_t *event, void *output);
    bool ble_stub_adv_enable;
    uint8_t default_tx_pref_phy_bits;
    uint8_t default_rx_pref_phy_bits;
    ble_smp_require_t *p_smp_req[BLE_CONNECTION_MAX];
    uint16_t curr_mtu_size[BLE_CONNECTION_MAX];
    const char *ble_dev_name;
    const char *peer_dev_name[BLE_CONNECTION_MAX];
    // GATT and gap features
    uint8_t gattc_supp_eatt_bearer : 1;
    uint8_t gattc_supp_robust_caching : 1;
    uint8_t gattc_supp_recv_multi_notify : 1;
    uint8_t gatts_supp_eatt_bearer : 1;
    uint8_t gatts_supp_database_hash : 1;
    uint8_t gap_only_use_rpa_after_bonding : 1;
    uint8_t gattc_profile_id;
    uint8_t local_database_hash[GAP_KEY_LEN];
    uint8_t (*ble_resolving_list_fill_cb)(void);
    void (*ble_smp_require_modify)(uint16_t connhdl, ble_smp_require_t *p_requirements);
    void (*ble_add_record_modify)(uint16_t connhdl, BleDevicePairingInfo *p_record_add);
    void (*ble_get_specific_irk_ia)(uint16_t connhdl, uint8_t **irk, bt_bdaddr_t **ia);
    bool (*ble_get_specific_record)(uint16_t connhdl, const ble_bdaddr_t *p_addr, BleDevicePairingInfo *p_info);
    void (*ble_get_specific_hash)(uint16_t connhdl, uint8_t **hash);
} ble_global_t;

#if defined(BLE_DIP_ENABLE)
/**
 * dip server
 */
typedef enum
{
    BLE_DIP_MANUFACTURER_NAME       = 0x00,
    BLE_DIP_MODEL_NUMBER_STRING     = 0x01,
    BLE_DIP_SERIAL_NUMBER_STRING    = 0x02,
    BLE_DIP_HW_REVISION_STRING      = 0x03,
    BLE_DIP_FW_REVISION_STRING      = 0x04,
    BLE_DIP_SW_REVISION_STRING      = 0x05,
    BLE_DIP_SYSTEM_ID               = 0x06,
    BLE_DIP_IEEE_CDL                = 0x07,
    BLE_DIP_PNP_INFO                = 0x08,
    BLE_DIP_MEDICAL_UDI             = 0x09,
    BLE_DIP_INVALID_TYPE            = 0xFF,
} app_ble_dip_info_type_t;
typedef struct
{
    uint8_t vendor_id_source; // 0x01 bluetooth sig
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t product_version;
} __attribute__((packed)) app_ble_dip_pnp_info_t;
void ble_dip_init(void);
/**
 * dip client
 */
typedef struct
{
    app_ble_dip_info_type_t type;
    uint8_t con_idx;
    uint16_t connhdl;
    app_ble_dip_pnp_info_t pnp_info; // for BLE_DIP_PNP_INFO
    const char *value; // for other info type
    uint16_t value_len;
} app_ble_dip_client_callback_param_t;
typedef void (*app_ble_dip_client_callback_t)(const app_ble_dip_client_callback_param_t *param);
bt_status_t app_ble_discover_dip_service(uint16_t connhdl);
bt_status_t app_ble_dip_read_peer_info(uint16_t connhdl, app_ble_dip_info_type_t type);
bt_status_t app_ble_dip_register_callback(app_ble_dip_client_callback_t callback);
#endif

#if defined(BLE_BATT_ENABLE)
/**
 * battery server
 */
#define BLE_LOCAL_DEVICE_BATTERY 0x00
#define BLE_PEER_TWS_BUD_BATTERY 0x01
void ble_batt_init(void);
bt_status_t app_ble_report_battery_level(uint8_t instance, uint8_t battery_level);
/**
 * battery client
 */
typedef struct
{
    uint8_t instance;
    uint8_t con_idx;
    uint16_t connhdl;
    uint8_t battery_level;
    uint8_t name_space;
    uint16_t description;
} app_ble_battery_param_t;
typedef void (*app_ble_recv_battery_callback_t)(const app_ble_battery_param_t *param);
bt_status_t app_ble_discover_batt_service(uint16_t connhdl);
bt_status_t app_ble_read_peer_battery_level(uint16_t connhdl, uint8_t instance);
bt_status_t app_ble_register_battery_callback(app_ble_recv_battery_callback_t callback);
#endif

#if defined(BLE_HID_ENABLE)
void ble_hid_device_init(void);
#endif

#if defined(BLE_HID_HOST)
void ble_hid_host_init(void);
bt_status_t ble_hid_host_start_discover(uint16_t connhdl);
#endif

#ifdef CFG_APP_AHP_SERVER
void ble_ahp_init(void);
#endif

#ifdef ANCS_ENABLED
void ble_ancs_init(void);
#endif

#if defined(ANCC_ENABLED)
void ble_ancc_init(void);
bt_status_t ble_ancc_start_discover(uint16_t connhdl);
#endif

#if defined (BLE_IAS_ENABLED)
int ble_ias_init(void);
int ble_ias_deinit(void);

int ble_iac_init(void);
int ble_iac_deinit(void);
int ble_iac_start_discover(uint8_t con_lid);
int iac_write_alert_level(uint8_t con_lid, uint8_t alert_level);
#endif

#if defined(CFG_APP_SAS_SERVER)
void ble_sass_init(void);
#endif

#if defined(BES_MOBILE_SAS)
void ble_sasc_init(void);
bt_status_t ble_sasc_start_discover(uint16_t connhdl);
#endif

typedef bool (*app_ble_adv_activity_func)(ble_adv_activity_t *adv);

#ifdef GFPS_ENABLED
void ble_app_gfps_init(app_ble_adv_activity_func func);
uint8_t ble_app_gfps_l2cap_send(uint8_t conidx, uint8_t *ptrData, uint32_t length);
void ble_app_gfps_l2cap_disconnect(uint8_t conidx);
void ble_app_gfps_send_keybase_pairing(uint8_t conidx, const uint8_t *data, uint32_t length);
#ifdef SPOT_ENABLED
void ble_app_gfps_send_beacon_data(uint8_t conidx, const uint8_t *data, uint32_t length);
void ble_app_spot_init(app_ble_adv_activity_func func);
#endif
#endif

#ifdef SWIFT_ENABLED
void app_swift_init(void);
void app_swift_enter_pairing_mode(void);
void app_swift_exit_pairing_mode(void);
#endif

#if (defined(BES_OTA) || defined(BES_OTA_BASIC))&& !defined(OTA_OVER_TOTA_ENABLED)
typedef enum
{
    APP_OTA_CCC_CHANGED = 0,
    APP_OTA_DIS_CONN,
    APP_OTA_RECEVICE_DATA,
    APP_OTA_MTU_UPDATE,
    APP_OTA_SEND_DONE,
} APP_OTA_EVENT_TYPE_E;
typedef struct
{
    uint8_t event_type;
    uint8_t conidx;
    uint16_t connhdl;
    union
    {
        uint8_t ntf_en; // APP_OTA_CCC_CHANGED
        uint16_t mtu; // APP_OTA_MTU_UPDATE
        uint8_t status; // APP_OTA_SEND_DONE
        struct   // APP_OTA_RECEVICE_DATA
        {
            uint16_t data_len;
            uint8_t *data;
        } receive_data;
    } param;
} app_ota_event_param_t;
typedef void(*app_ota_event_callback)(app_ota_event_param_t *param);
void ble_ota_init(void);
void app_ota_event_reg(app_ota_event_callback cb);
void app_ota_event_unreg(void);
void app_ota_send_rx_cfm(uint8_t conidx);
bool app_ota_send_notification(uint8_t conidx, uint8_t *data, uint32_t len);
bool app_ota_send_indication(uint8_t conidx, uint8_t *data, uint32_t len);
#endif

#ifdef BLE_TOTA_ENABLED
typedef enum
{
    APP_TOTA_CCC_CHANGED = 0,
    APP_TOTA_DIS_CONN_EVENT,
    APP_TOTA_RECEVICE_DATA,
    APP_TOTA_MTU_UPDATE,
    APP_TOTA_SEND_DONE,
} APP_TOTA_EVENT_TYPE_E;

typedef struct
{
    uint8_t event_type;
    uint8_t conidx;
    uint16_t connhdl;
    union
    {
        uint8_t ntf_en; // APP_TOTA_CCC_CHANGED
        uint16_t mtu; // APP_TOTA_MTU_UPDATE
        uint8_t status; // APP_TOTA_SEND_DONE
        struct   // APP_TOTA_RECEVICE_DATA
        {
            uint16_t data_len;
            uint8_t *data;
        } receive_data;
    } param;
} app_tota_event_param_t;

typedef void(*app_tota_event_callback)(app_tota_event_param_t *param);
void ble_tota_init(void);
void app_tota_event_reg(app_tota_event_callback cb);
void app_tota_event_unreg(void);
bool app_tota_send_notification(uint8_t conidx, uint8_t *data, uint32_t len);
bool app_tota_send_indication(uint8_t conidx, uint8_t *data, uint32_t len);
#endif

#ifdef BLE_MESH_ENABLE
void app_ble_mesh_init();
#endif

typedef struct
{
    const uint8_t *adv_data;
    const uint8_t *scan_rsp_data;
    uint16_t adv_data_len;
    uint16_t scan_rsp_len;
} app_ble_adv_data_param_t;

typedef gap_scan_callback_t app_ble_scan_callback_t;

typedef struct
{
    uint8_t ownAddrType;
    uint8_t scanFolicyType;
    uint8_t phys;
    uint8_t scanType;
    uint16_t scanIntervalMs;
    uint16_t scanWindowMs;
    uint8_t scanType_coded;
    uint16_t scanIntervalMs_coded;
    uint16_t scanWindowMs_coded;
    bool filter_duplicates;
    bool legacy;
    uint16_t scanDurationMs;
    uint32_t period;
    app_ble_scan_callback_t scan_callback;
} BLE_SCAN_PARAM_T;

typedef bool (*app_ble_adv_activity_func)(ble_adv_activity_t *adv);

void app_ble_init(void);
ble_global_t *ble_get_global(void);
void app_ble_ready_and_init_done();
void app_ble_core_evt_cb_register(APP_BLE_CORE_EVENT_CALLBACK cb);
void app_ble_core_evt_cb_unregister(APP_BLE_CORE_EVENT_CALLBACK cb);
void app_ble_core_register_global_handler_ind(APP_BLE_CORE_GLOBAL_HANDLER_FUNC handler);
void app_ble_core_unregister_global_handler_ind(APP_BLE_CORE_GLOBAL_HANDLER_FUNC handler);
void app_ble_mtu_exec_ind_callback_register(app_ble_mtu_exch_cb_t cb);
void app_ble_mtu_exec_ind_callback_deregister(void);
void app_ble_resolving_list_fill_callback_register(uint8_t (*resolving_list_fill_cb)(void));
void app_ble_smp_require_modify_callback_register(void (*ble_smp_require_modify)(uint16_t, ble_smp_require_t *));
void app_ble_smp_get_specifc_irk_ia_callback_register(void (*ble_get_specific_irk_ia)(uint16_t, uint8_t **,
                                                                                      bt_bdaddr_t **));
void app_ble_gap_add_record_modify_callback_register(void (*ble_add_record_modify)(uint16_t, BleDevicePairingInfo *));
void app_ble_gap_get_specifc_record_callback_register(bool (*ble_get_specific_record)(uint16_t, const ble_bdaddr_t *,
                                                                                      BleDevicePairingInfo *));
void app_ble_gatt_get_specifc_hash_callback_register(void (*ble_get_specific_hash)(uint16_t, uint8_t **));
void app_sec_reg_dist_lk_bit_set_callback(set_rsp_dist_lk_bit_field_func callback);
void *app_sec_reg_dist_lk_bit_get_callback(void);
bt_status_t app_ble_start_connect(const ble_bdaddr_t *peer_addr, uint8_t ia_rpa_npa);
bt_status_t app_ble_connect_ble_audio_device(const ble_bdaddr_t *peer_addr, uint8_t ia_rpa_npa, uint32_t connect_time);
bt_status_t app_ble_start_auto_connect(const ble_bdaddr_t *addr_list, uint16_t list_size, uint8_t ia_rpa_npa,
                                       uint32_t connect_time);
void app_ble_cancel_connecting(void);
void app_ble_disconnect(uint16_t connhdl);
void app_ble_disconnect_all(void);
void app_ble_set_phy_mode(uint8_t conidx, uint8_t tx_phy_bits, uint8_t rx_phy_bits, uint8_t phy_opt);
void app_ble_start_disconnect(uint8_t conidx);
bool app_ble_get_peer_solved_addr(uint8_t conidx, ble_bdaddr_t *p_addr);
const char *app_ble_get_peer_device_name(uint8_t conidx);
ble_bdaddr_t app_ble_get_local_identity_addr(uint8_t conidx);
const uint8_t *app_ble_get_local_rpa_addr(uint8_t conidx);
const uint8_t *app_ble_get_local_rpa_by_adv_hdl(uint8_t adv_hdl);
void app_ble_read_local_rpa_by_adv_hdl(uint8_t adv_hdl);
bool app_ble_is_remote_dev_connected(const ble_bdaddr_t *p_addr);
uint8_t app_ble_connection_count(void);
bool app_is_arrive_at_max_ble_connections(void);
bool app_ble_is_any_connection_exist(void);
bool app_ble_is_connection_on(uint8_t index);
uint16_t app_ble_get_conhdl_from_conidx(uint8_t conidx);
uint8_t app_ble_get_conidx_from_conhdl(uint16_t connhdl);
void app_ble_update_conn_param_mode(BLE_CONN_PARAM_MODE_E mode, bool enable);
void app_ble_update_conn_param_mode_of_specific_connection(uint8_t con_idx, BLE_CONN_PARAM_MODE_E mode, bool enable);
void app_ble_register_ia_exchanged_callback(smp_identify_addr_exch_complete callback);
void app_ble_set_white_list(BLE_WHITE_LIST_USER_E user, ble_bdaddr_t *bdaddr, uint8_t size);
void app_ble_clear_white_list(BLE_WHITE_LIST_USER_E user);
void app_ble_clear_all_white_list(void);
void app_ble_add_dev_to_rpa_list_in_controller(const ble_bdaddr_t *ble_addr, const uint8_t *irk);
void app_ble_add_devices_info_to_resolving(void);
void app_ble_send_security_req(uint8_t conidx);
bt_status_t app_ble_start_authentication(uint8_t conidx, const ble_smp_require_t *p_smp_req);
bt_status_t app_ble_enable_link_encryption(uint8_t conidx, const uint8_t *ediv,
                                           const uint8_t *rand, const uint8_t *ltk);
bt_status_t app_ble_send_smp_pairing_response(uint8_t conidx, const ble_smp_require_t *p_smp_req);
void app_ble_set_local_irk(const uint8_t *p_irk);
void app_ble_get_local_irk(uint8_t *p_irk);
void app_ble_start_scan(BLE_SCAN_PARAM_T *param);
void app_ble_stop_scan(void);
void app_ble_data_fill_enable(BLE_ADV_USER_E user, bool enable);
void app_ble_register_data_fill_handle(BLE_ADV_USER_E user, BLE_DATA_FILL_FUNC_T func, bool enable);
void app_ble_dt_set_flags(gap_adv_param_t *adv_param, bool simu_bredr_support);
void app_ble_dt_set_local_name(gap_adv_param_t *adv_param, const char *cust_le_name);
void app_ble_parse_out_adv_data_service_uuid(BLE_ADV_PARAM_T *data, gap_dt_buf_t *out_uuid_16,
                                             gap_dt_buf_t *out_uuid_128);
void app_ble_parse_out_scan_rsp_service_uuid(BLE_ADV_PARAM_T *data, gap_dt_buf_t *out_uuid_16,
                                             gap_dt_buf_t *out_uuid_128);
BLE_ADV_ACTIVITY_USER_E app_ble_param_get_actv_user_from_adv_user(BLE_ADV_USER_E user);
void app_ble_param_set_adv_interval(BLE_ADV_INTERVALREQ_USER_E adv_intv_user, BLE_ADV_USER_E adv_user,
                                    uint32_t interval_ms);
void app_ble_force_switch_adv(enum BLE_ADV_SWITCH_USER_E user, bool enable_adv);
bool app_ble_is_connection_on_by_addr(uint8_t *addr);
int app_ble_server_callback_api(uintptr_t connhdl, gap_adv_event_t event, gap_adv_callback_param_t param);
bool app_ble_is_in_advertising_state(void);
bool ble_adv_is_allowed(void);
void ble_core_enable_stub_adv(void);
void ble_core_disable_stub_adv(void);
void app_ble_stub_user_init(void);
void app_ble_start_adv_generic(void);
void app_ble_stop_adv_generic(void);
void app_ble_refresh_adv_state_generic(void);
void app_ble_enable_advertising(uint8_t adv_handle);
void app_ble_disable_advertising(uint8_t adv_handle);
void app_ble_sync_ble_info(void);
void app_ble_mode_tws_sync_init(void);
void ble_roleswitch_start(void);
void ble_roleswitch_complete(uint8_t newRole);
void ble_role_update(uint8_t newRole);
void ble_ibrt_event_entry(uint8_t ibrt_evt_type);
int8_t app_ble_get_rssi(uint8_t conidx);
uint16_t app_ble_get_connection_interval_1_25_ms(uint8_t conidx);
ble_conn_timing_t app_ble_get_connection_curr_timing(uint8_t conidx);
uint16_t app_ble_get_connection_current_mtu_size(uint8_t conidx);
uint8_t *app_ble_get_dev_name(void);
uint8_t app_ble_own_addr_type(void);
ble_bdaddr_t app_get_current_ble_addr(void);
ble_adv_activity_t *app_ble_register_advertising(uint8_t adv_handle, app_ble_adv_activity_func adv_activity_func);
ble_adv_activity_t *app_ble_get_advertising_by_user(BLE_ADV_USER_E user);
void app_ble_set_tx_rx_pref_phy(uint32_t tx_pref_phy, uint32_t rx_pref_phy);
void app_ble_set_adv_txpwr_by_adv_user(BLE_ADV_USER_E user, int8_t txpwr_dbm);
void app_ble_set_adv_tx_power_dbm(ble_adv_activity_t *adv, int8_t tx_power_dbm);
void app_ble_set_adv_tx_power_level(ble_adv_activity_t *adv, BLE_ADV_TX_POWER_LEVEL_E tx_power_level);
void app_ble_dt_add_adv_data(ble_adv_activity_t *adv, BLE_ADV_PARAM_T *a, const app_ble_adv_data_param_t *b);
bool app_ble_get_user_adv_data(ble_adv_activity_t *adv, BLE_ADV_PARAM_T *param, int user_group);
bt_status_t app_ble_gatt_update_enc_data_key_material(const gap_key_material_t *key_material);
bt_status_t app_ble_gatt_read_peer_character_value(uint16_t connhdl, uint16_t uuid);
void app_ble_gap_update_local_database_hash(void);
bt_status_t app_ble_gatt_server_send_service_change(uint32_t con_bfs);
bt_status_t app_ble_clr_gatt_cli_cache_by_connhdl(uint16_t connhdl);
bt_status_t app_ble_gap_set_local_ecdh_key_pair(const uint8_t *p_sec_key_256, const uint8_t *p_pub_key_256);
bt_status_t app_ble_set_public_address(const bt_bdaddr_t *public_addr);
bt_status_t app_ble_set_le_tx_pwr(uint16_t connhdl, int8_t tx_pwr);
#if defined(IBRT)
bool app_ble_check_ibrt_allow_adv(BLE_ADV_USER_E user);
#endif
int app_ble_server_callback(uintptr_t connhdl, gap_adv_event_t event, gap_adv_callback_param_t param);
uint32_t app_ble_save_ctx(uint8_t conidx, uint8_t *buf, uint16_t buf_len);
uint32_t app_ble_restore_ctx(uint8_t conidx, uint8_t *buf, uint16_t buf_len);

void app_ble_adv_set_param(BLE_ADV_PARAM_T *param, uint8_t user);
ble_adv_activity_t *app_ble_adv_get_param(BLE_ADV_ACTIVITY_USER_E actv_user);

#if (mHDT_LE_SUPPORT)
void app_ble_mhdt_hci_le_rd_local_proprietary_feat_cmd();
void app_ble_mhdt_hci_le_rd_remote_proprietary_feat_cmd(uint8_t conidx);
#endif
#ifdef __cplusplus
}
#endif
#endif /* __APP_BLE_H__ */
