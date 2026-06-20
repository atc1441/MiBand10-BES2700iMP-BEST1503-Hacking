#include "bluetooth_ble_api.h"
#include "co_bt_defines.h"
#include "gatt_service.h"
#include "app_ble.h"
#if ISO_BEARER_SUPPORT
#include "isoohci_int.h"
#include "ble_iso.h"
#endif
#if BLE_AUDIO_ENABLED
#include "aob_conn_api.h"
#include "aob_csip_api.h"
#include "aob_gatt_cache.h"
#include "aob_bis_api.h"
#include "aob_call_api.h"
#include "aob_volume_api.h"
#include "aob_media_api.h"
#include "aob_pacs_api.h"
#include "aob_gaf_api.h"
#include "aob_stream_handler.h"
#include "aob_service_sync.h"

#include "ble_aob_common.h"

#include "app_bap_data_path_itf.h"

#include "ble_audio_core_api.h"
#include "ble_audio_earphone_info.h"

#ifdef BLE_USB_AUDIO_SUPPORT
#include "app_ble_usb_audio.h"
#endif  //BLE_USB_AUDIO_SUPPORT
#endif  //BLE_AUDIO_ENABLED

#include "gaf_media_common.h"

#ifdef BLE_WALKIE_TALKIE
#include "walkie_talkie_ble_gapm_cmd.h"
#endif

#if defined(IBRT_UI)
#include "app_tws_ibrt_conn_api.h"
#include "app_custom_api.h"
#endif

#ifdef BLE_RATE_TEST_SERVER
#include "app_rate_tests.h"
#include "bes_rate_test_api.h"
#endif

#ifdef BLE_RATE_TEST_CLIENT
#include "app_rate_testc.h"
#include "bes_rate_test_api.h"
#endif

#if (APP_BLE_DEMO_APP_ENABLED)
#include "ble_demo_app.h"
#endif

#ifdef BLE_WIRELESS_TRANS_SRV_ENABLED
#include "ble_wireless_trans_srv.h"
#endif

#ifdef BLE_WIRELESS_TRANS_CLI_ENABLED
#include "ble_wireless_trans_cli.h"
#endif

#ifdef SPOT_ENABLED
#include "ble_dult.h"
#endif

/**
 * IBRT CALL FUNC
 *
 */
void bes_ble_roleswitch_start(uint8_t curr_ui_role)
{
    ble_roleswitch_start();
#if (APP_BLE_DEMO_APP_ENABLED)
    ble_demo_app_tws_role_switch_start(curr_ui_role);
#endif
}

void bes_ble_roleswitch_complete(uint8_t newRole)
{
    ble_roleswitch_complete(newRole);
#if (APP_BLE_DEMO_APP_ENABLED)
    ble_demo_app_tws_role_switch_cmp(newRole);
#endif
}

void bes_ble_role_update(uint8_t newRole)
{
    ble_role_update(newRole);
#if (APP_BLE_DEMO_APP_ENABLED)
    ble_demo_app_tws_role_update(newRole);
#endif
}

void bes_ble_ibrt_event_entry(uint8_t ibrt_evt_type)
{
    ble_ibrt_event_entry(ibrt_evt_type);
}

/**
 * GAP
 *
 */
void bes_ble_gap_stub_user_init(void)
{
    app_ble_stub_user_init();
}

int bes_ble_gap_start_connect(bes_ble_bdaddr_t *addr, BES_GAP_OWN_ADDR_E own_type)
{
    return app_ble_start_connect((ble_bdaddr_t *)addr, own_type);
}

int bes_ble_gap_connect_ble_audio_device(bes_ble_bdaddr_t *addr, BES_GAP_OWN_ADDR_E own_type)
{
    return app_ble_connect_ble_audio_device((ble_bdaddr_t *)addr, own_type, 0);
}

void bes_ble_gap_cancel_connecting(void)
{
    app_ble_cancel_connecting();
}

bool bes_ble_gap_is_connection_on(uint8_t index)
{
    return app_ble_is_connection_on(index);
}

void bes_ble_gap_update_conn_param_mode(BLE_CONN_PARAM_MODE_E mode, bool isEnable)
{
    app_ble_update_conn_param_mode(mode, isEnable);
}

bool bes_ble_gap_is_remote_dev_connected(const ble_bdaddr_t* p_addr)
{
    return app_ble_is_remote_dev_connected(p_addr);
}

int8_t bes_ble_gap_get_rssi(uint8_t conidx)
{
    return app_ble_get_rssi(conidx);
}

void bes_ble_gap_clear_white_list_for_mobile(void)
{
    app_ble_clear_white_list(BLE_WHITE_LIST_USER_MOBILE);
}

void bes_ble_gap_start_disconnect(uint8_t conIdx)
{
    app_ble_start_disconnect(conIdx);
}

void bes_ble_gap_disconnect_all(void)
{
    app_ble_disconnect_all();
}

uint8_t bes_ble_get_con_id_by_addr(const ble_bdaddr_t *peer_addr)
{
    uint8_t conid = 0xFF;
    ble_bdaddr_t addr_get = {0};
    bool ret = false;
    for (uint8_t conidx = 0; conidx < AOB_COMMON_MOBILE_CONNECTION_MAX; conidx++)
    {
        ret = bes_ble_gap_get_peer_solved_addr(conidx, &addr_get);

        if (ret == true && !memcmp(peer_addr, &addr_get, sizeof(addr_get)))
        {
            conid = conidx;
        }
    }
    return conid;
}

int bes_ble_gap_disconnect_by_addr(const ble_bdaddr_t *peer_addr)
{
    uint8_t conidx = bes_ble_get_con_id_by_addr(peer_addr);

    if(conidx != 0xFF)
    {
        bes_ble_gap_start_disconnect(conidx);
        return BT_STS_SUCCESS;
    }

    return BT_STS_FAILED;
}

void bes_ble_gap_refresh_irk(void)
{

}

void bes_ble_gap_force_switch_adv(enum BLE_ADV_SWITCH_USER_E user, bool isToEnableAdv)
{
    app_ble_force_switch_adv(user, isToEnableAdv);
}

void bes_ble_gap_start_connectable_adv(uint16_t advInterval)
{
    app_ble_start_adv_generic();
}

void bes_ble_gap_start_adv(void)
{
    app_ble_start_adv_generic();
}

void bes_ble_gap_stop_adv_all(void)
{
    app_ble_stop_adv_generic();
}

void bes_ble_gap_refresh_adv_state(uint16_t advInternal)
{
    app_ble_refresh_adv_state_generic();
}

void bes_ble_gap_set_white_list(BLE_WHITE_LIST_USER_E user, const ble_bdaddr_t *bdaddr, uint8_t size)
{
    app_ble_set_white_list(user, (ble_bdaddr_t *)bdaddr, size);
}

void bes_ble_gap_remove_white_list_user_item(BLE_WHITE_LIST_USER_E user)
{
    app_ble_clear_white_list(user);
}

void bes_ble_gap_clear_white_list(void)
{
    app_ble_clear_all_white_list();
}

void bes_ble_gap_set_rpa_list(const ble_bdaddr_t *ble_addr, const uint8_t *irk)
{
    app_ble_add_dev_to_rpa_list_in_controller(ble_addr, irk);
}

void bes_ble_gap_set_adv_param(BLE_ADV_PARAM_T *param, uint8_t user)
{
    app_ble_adv_set_param(param, user);
}

ble_adv_activity_t *bes_ble_gap_get_adv_param(uint8_t user)
{
    return app_ble_adv_get_param(user);
}

void bes_ble_gap_set_bonded_devs_rpa_list(void)
{
    app_ble_add_devices_info_to_resolving();
}

void bes_ble_gap_set_rpa_timeout(uint16_t rpa_timeout)
{
    gap_set_rpa_timeout(rpa_timeout);
}

void bes_ble_gap_custom_adv_start(BLE_ADV_ACTIVITY_USER_E actv_user)
{
    app_ble_custom_adv_start(actv_user);
}

BLE_ADV_ACTIVITY_USER_E bes_ble_param_get_actv_user_from_adv_user(BLE_ADV_USER_E user)
{
    return app_ble_param_get_actv_user_from_adv_user(user);
}

void bes_ble_gap_custom_adv_write_data(bes_ble_gap_cus_adv_param_t *param)
{
    app_ble_custom_adv_write_data(param->actv_user, param->is_custom_adv_flags, param->type,
                                  param->local_addr, (ble_bdaddr_t *)param->peer_addr,param->adv_interval,
                                  param->adv_type, param->adv_mode, param->tx_power_dbm,
                                  param->adv_data, param->adv_data_size, param->scan_rsp_data,
                                  param->scan_rsp_data_size);
}

void bes_ble_gap_custom_adv_stop(BLE_ADV_ACTIVITY_USER_E actv_user)
{
    app_ble_custom_adv_stop(actv_user);
}

void bes_ble_gap_param_set_adv_interval(BLE_ADV_INTERVALREQ_USER_E adv_intv_user, BLE_ADV_USER_E adv_user, uint32_t interval_ms)
{
    app_ble_param_set_adv_interval(adv_intv_user, adv_user, interval_ms);
}

void bes_ble_gap_set_all_adv_txpwr(int8_t txpwr_dbm)
{

}

bool bes_ble_is_connection_on_by_index(uint8_t conidx)
{
    return app_ble_is_connection_on(conidx);
}

bool bes_ble_gap_is_in_advertising_state(void)
{
    return app_ble_is_in_advertising_state();
}

void bes_ble_gap_start_scan(bes_ble_scan_param_t *param)
{
    // CHECK_SIZE_TYPE(bes_ble_scan_param_t, sizeof(BLE_SCAN_PARAM_T));

    BLE_SCAN_PARAM_T scan_param =
    {
        .scanType = param->scanType,
        .scanFolicyType = param->scanFolicyType,
        .scanWindowMs = param->scanWindowMs,
        .scanIntervalMs = param->scanIntervalMs,
        .scanDurationMs = param->scanDurationMs,
        .filter_duplicates = true,
        .phys = GAP_PHY_BIT_LE_1M,
    };

    app_ble_start_scan(&scan_param);
}

void bes_ble_gap_stop_scan(void)
{
    app_ble_stop_scan();
}

uint8_t bes_ble_gap_set_ecdh_key_pair(const uint8_t *p_sec_key_256, const uint8_t *p_pub_key_256)
{
    return app_ble_gap_set_local_ecdh_key_pair(p_sec_key_256, p_pub_key_256);
}

void bes_ble_gap_adv_report_callback_register(bes_ble_scan_result_user_id_e id,
                                                             bes_ble_adv_data_report_cb_t cb)
{
    app_ble_customif_adv_report_callback_register(id, (ble_adv_data_report_cb_t)cb);
}

void bes_ble_gap_adv_report_callback_deregister(bes_ble_scan_result_user_id_e id)
{
    app_ble_customif_adv_report_callback_deregister(id);
}

void bes_ble_customif_link_event_callback_register(bes_ble_link_event_report_cb_t cb)
{
    app_ble_customif_link_event_callback_register((ble_link_event_report_cb_t)cb);
}

void bes_ble_customif_link_event_callback_deregister(void)
{
    app_ble_customif_link_event_callback_deregister();
}

void bes_ble_set_tx_rx_pref_phy(uint32_t tx_pref_phy, uint32_t rx_pref_phy)
{
    app_ble_set_tx_rx_pref_phy(tx_pref_phy, rx_pref_phy);
}

void bes_ble_connect_req_callback_register(bes_ble_link_connect_cb_t req_cb, bes_ble_link_connect_cb_t done_cb)
{

}

void bes_ble_connect_req_callback_deregister(void)
{

}

void bes_ble_mtu_exec_ind_callback_register(bes_ble_link_mtu_exch_cb_t mtu_exec_cb)
{

}

void bes_ble_mtu_exec_ind_callback_deregister(void)
{

}

void bes_ble_gatt_cli_create_bearer(uint8_t conidx)
{
    gatt_create_att_bearer(gap_conn_idx_as_hdl(conidx));
}

void bes_ble_gatt_cli_disconnect_bearer(uint8_t conidx)
{
    gatt_disconnect_att_bearers(gap_conn_idx_as_hdl(conidx), false, 0);
}

void bes_ble_set_scan_coded_phy_en_and_param_before_start_scan(bool enable, bes_ble_scan_wd_t *start_scan_coded_scan_wd)
{

}

void bes_ble_set_init_conn_all_phy_param_before_start_connect(bes_ble_conn_param_t *init_param_universal,
                                                        bes_ble_scan_wd_t *init_coded_scan_wd)
{

}

void bes_ble_gap_register_data_fill_handle(BLE_ADV_USER_E user, BLE_DATA_FILL_FUNC_T func, bool enable)
{
    app_ble_register_data_fill_handle(user, func, enable);
}

void bes_ble_gap_data_fill_enable(BLE_ADV_USER_E user, bool enable)
{
    app_ble_data_fill_enable(user, enable);
}

bool bes_ble_gap_is_any_connection_exist(void)
{
    return app_ble_is_any_connection_exist();
}

uint8_t bes_ble_gap_connection_count(void)
{
    return app_ble_connection_count();
}

uint16_t bes_ble_gap_get_conhdl_from_conidx(uint8_t conidx)
{
    return app_ble_get_conhdl_from_conidx(conidx);
}

uint8_t bes_ble_gap_get_conidx_from_conhdl(uint16_t connhdl)
{
    return app_ble_get_conidx_from_conhdl(connhdl);
}

void bes_ble_gap_conn_update_param(uint8_t conidx, uint32_t min_interval_in_ms, uint32_t max_interval_in_ms,
        uint32_t supervision_timeout_in_ms, uint8_t  slaveLatency)
{
    gap_update_params_t param = {0};
    param.conn_interval_min_1_25ms = (min_interval_in_ms * 100) / 125; // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4000ms
    param.conn_interval_max_1_25ms = (max_interval_in_ms * 100) / 125; // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4000ms
    param.max_peripheral_latency = slaveLatency;
    param.superv_timeout_ms = supervision_timeout_in_ms;
    gap_update_le_conn_parameters(gap_zero_based_ble_conidx_as_hdl(conidx), &param);
}

void bes_ble_gap_l2cap_data_rec_over_bt(uint8_t condix, uint16_t conhdl, uint8_t* ptrData, uint16_t dataLen)
{

}

void bes_ble_gap_core_register_global_handler(APP_BLE_CORE_GLOBAL_HANDLER_FUNC handler)
{
    app_ble_core_register_global_handler_ind(handler);
}

void bes_ble_gap_core_unregister_global_handler(APP_BLE_CORE_GLOBAL_HANDLER_FUNC handler)
{
    app_ble_core_unregister_global_handler_ind(handler);
}

ble_bdaddr_t bes_ble_gap_get_current_ble_addr(void)
{
    return app_get_current_ble_addr();
}

void bes_ble_demo_app_tws_sync_info_recv_handler(uint8_t *p_info, uint16_t len)
{
#if (APP_BLE_DEMO_APP_ENABLED)
    ble_demo_app_tws_sync_info_receive_handler(p_info, len);
#endif
}

bool bes_ble_is_connection_support_role_switch(uint16_t connhdl)
{
#if (APP_BLE_DEMO_APP_ENABLED)
    return ble_demo_app_is_connection_support_le_rs(connhdl);
#else
    return false;
#endif
}

uint32_t bes_ble_app_save_ctx(uint8_t conidx, uint8_t *buf, uint16_t buf_len)
{
    return app_ble_save_ctx(conidx, buf, buf_len);
}

uint32_t bes_ble_app_restore_ctx(uint8_t conidx, uint8_t *buf, uint16_t buf_len)
{
    return app_ble_restore_ctx(conidx, buf, buf_len);
}

ble_bdaddr_t bes_ble_gap_get_local_identity_addr(uint8_t conidx)
{
    return app_ble_get_local_identity_addr(conidx);
}

void bes_ble_gap_read_local_rpa_by_adv_hdl(uint8_t adv_hdl)
{
    app_ble_read_local_rpa_by_adv_hdl(adv_hdl);
}

const uint8_t *bes_ble_gap_get_local_rpa_addr(uint8_t conidx)
{
    return app_ble_get_local_rpa_addr(conidx);
}

const uint8_t *bes_ble_gap_get_local_rpa_by_adv_hdl(uint8_t adv_hdl)
{
    return app_ble_get_local_rpa_by_adv_hdl(adv_hdl);
}

uint8_t bes_ble_gap_set_public_address(const bt_bdaddr_t *public_addr)
{
    return app_ble_set_public_address(public_addr);
}

uint8_t bes_ble_gap_set_le_tx_pwr(uint16_t connhdl, int8_t tx_pwr)
{
    return app_ble_set_le_tx_pwr(connhdl, tx_pwr);
}

void bes_ble_gap_sec_send_security_req(uint8_t conidx, uint8_t sec_level)
{
    app_ble_send_security_req(conidx);
}

int bes_ble_gap_start_authentication(uint8_t conidx, const bes_ble_smp_require_t *p_req)
{
    return app_ble_start_authentication(conidx, (ble_smp_require_t *)p_req);
}

int bes_ble_gap_enable_link_encryption(uint8_t conidx, const uint8_t* ediv, const uint8_t *rand, const uint8_t *ltk)
{
    return app_ble_enable_link_encryption(conidx, ediv, rand, ltk);
}

int bes_ble_gap_send_pairing_response(uint8_t conidx, const bes_ble_smp_require_t *p_req)
{
    return app_ble_send_smp_pairing_response(conidx, (ble_smp_require_t *)p_req);
}

void bes_ble_gap_set_local_irk(const uint8_t *p_irk)
{
    app_ble_set_local_irk(p_irk);
}

void bes_ble_gap_sec_reg_dist_lk_bit_set_callback(set_rsp_dist_lk_bit_field_func callback)
{
    app_sec_reg_dist_lk_bit_set_callback(callback);
}

set_rsp_dist_lk_bit_field_func bes_ble_gap_sec_reg_dist_lk_bit_get_callback()
{
    return app_sec_reg_dist_lk_bit_get_callback();
}

void bes_ble_gap_sec_reg_smp_identify_info_cmp_callback(smp_identify_addr_exch_complete callback)
{
    app_ble_register_ia_exchanged_callback(callback);
}

void bes_ble_gap_resolving_list_fill_cb_resgiter(uint8_t user,
                        void (*cb)(ble_bdaddr_t *bleAddr, uint8_t *peer_irk, uint8_t *local_irk))
{
    app_ble_customif_resol_list_fill_cb_register(user, cb);
}

int bes_ble_gap_app_server_callback(uintptr_t connhdl, gap_adv_event_t event, gap_adv_callback_param_t param)
{
    return app_ble_server_callback(connhdl, event, param);
}

bool bes_ble_gap_get_peer_solved_addr(uint8_t conidx, ble_bdaddr_t* p_addr)
{
    return app_ble_get_peer_solved_addr(conidx, p_addr);
}

bes_ble_conn_timing_t bes_ble_get_connection_curr_timing(uint8_t conidx)
{
    ble_conn_timing_t curr_timing = app_ble_get_connection_curr_timing(conidx);
    return *(bes_ble_conn_timing_t *)&curr_timing;
}

uint16_t bes_ble_get_connection_current_att_mtu(uint8_t conidx)
{
    return app_ble_get_connection_current_mtu_size(conidx);
}

const char *bes_ble_gap_get_peer_device_name(uint8_t conidx)
{
    return app_ble_get_peer_device_name(conidx);
}

void bes_ble_gap_get_tx_pwr_value(uint8_t conidx, bes_ble_tx_object_e obj, bes_ble_phy_pwr_value_e phy)
{
    if (obj == BES_BLE_TX_LOCAL)
    {
        gap_read_conn_local_tx_power(gap_zero_based_ble_conidx_as_hdl(conidx), (gap_le_detail_phy_t)(phy+1));
    }
    else
    {
        gap_read_conn_remote_tx_power(gap_zero_based_ble_conidx_as_hdl(conidx), (gap_le_detail_phy_t)(phy+1));
    }
}

void bes_ble_gap_get_dev_tx_pwr_range(void)
{
    gap_read_le_tx_power_range();
}

void bes_ble_gap_get_adv_txpower_value(void)
{
    gap_read_legacy_adv_tx_power();
}

void bes_ble_gap_tx_power_report_enable(uint8_t conidx, bool local_enable, bool remote_enable)
{
    gap_set_conn_tx_power_report(gap_zero_based_ble_conidx_as_hdl(conidx), local_enable, remote_enable);
}

void bes_ble_gap_subrate_request(uint8_t conidx, uint16_t subrate_min, uint16_t subrate_max,
        uint16_t latency_max, uint16_t cont_num, uint16_t timeout)
{
    gap_subrate_params_t param = {0};
    param.subrate_factor_min = subrate_min;
    param.subrate_factor_max = subrate_max;
    param.max_peripheral_latency = latency_max;
    param.conn_continuation_number = cont_num;
    param.superv_timeout_ms = timeout;
    if (conidx == BES_BLE_INVALID_CONNECTION_INDEX)
    {
        gap_set_default_subrate(&param);
    }
    else
    {
        gap_update_subrate_parameters(gap_zero_based_ble_conidx_as_hdl(conidx), &param);
    }
}

void bes_ble_gap_set_phy_mode(uint8_t conidx, uint8_t tx_phy_bits, uint8_t rx_phy_bits, gap_coded_phy_prefer_t phy_opt)
{
    app_ble_set_phy_mode(conidx, tx_phy_bits, rx_phy_bits, phy_opt);
}

void bes_ble_gap_get_phy_mode(uint8_t conidx)
{
    gap_read_le_conn_phy(gap_zero_based_ble_conidx_as_hdl(conidx));
}

void bes_ble_gatt_server_send_service_changed(uint8_t conidx)
{
    uint32_t con_bfs = gap_conn_bf(conidx);
    app_ble_gatt_server_send_service_change(con_bfs);
}
/**
 * WALKIE_TALKIE
 *
 */
#ifdef BLE_WALKIE_TALKIE
int bta_walkie_gap_init(bes_if_ble_walkie_gap_callback *cb)
{
    return ble_walkie_gap_init((ble_walkie_gap_callback*)cb);
}

int bta_walkie_gap_deinit()
{
    return ble_walkie_gap_deinit();
}

uint8_t bta_walkie_gap_adv_creat(bes_if_ble_walkie_gap_adv_param* param)
{
    return ble_walkie_gap_adv_creat((ble_walkie_gap_adv_param*)param);
}

int bta_walkie_gap_adv_start(uint8_t adv_handle, uint32_t duartion)
{
    return ble_walkie_gap_adv_start(adv_handle, duartion);
}

int bta_walkie_gap_adv_stop(uint8_t adv_handle)
{
    return ble_walkie_gap_adv_stop(adv_handle);
}

int bta_talkie_gap_adv_set_data(uint8_t adv_handle, uint8_t *data, uint8_t data_len)
{
    return ble_talkie_gap_adv_set_data(adv_handle, data, data_len);
}

int bta_walkie_gap_scan_creat(uint16_t interval_ms, uint16_t windows_ms, bool use_filter_list)
{
    return ble_walkie_gap_scan_creat(interval_ms, windows_ms, use_filter_list);
}

int bta_walkie_gap_scan_start(bool filter_duplicates, uint32_t duration_ms)
{
    return ble_walkie_gap_scan_start(filter_duplicates, duration_ms);
}

int bta_walkie_gap_scan_stop()
{
    return ble_walkie_gap_scan_stop();
}

int bta_walkie_gap_pa_sync_create(const uint8_t *mac_addr)
{
    return ble_walkie_gap_pa_sync_create(mac_addr);
}

int bta_walkie_gap_pa_sync_stop(uint16_t pa_sync_hdl)
{
    return ble_walkie_gap_pa_sync_stop(pa_sync_hdl);
}

int bta_walkie_gap_pa_set_data(uint8_t adv_handle, const uint8_t *pa_data, uint8_t data_len)
{
    return ble_walkie_gap_pa_set_data(adv_handle, pa_data, data_len);
}

int bta_walkie_gap_set_white_list(uint8_t *mac_addr, uint8_t count)
{
    return ble_walkie_gap_set_white_list(mac_addr, count);
}

int bta_walkie_gap_clear_white_list()
{
    return ble_walkie_gap_clear_white_list();
}

int bta_walkie_gap_set_mesh_list(uint8_t *mac_addr, uint8_t count)
{
    return ble_walkie_gap_set_mesh_list(mac_addr, count);
}

int bta_walkie_gap_clear_mesh_list()
{
    return ble_walkie_gap_clear_mesh_list();
}

#endif  //BLE_WALKIE_TALKIE

void bes_ble_iso_quality(uint16_t cisHdl, uint8_t *param)
{
#if BLE_AUDIO_ENABLED
    app_bap_uc_srv_iso_quality_ind_handler(cisHdl, param);
#endif
}

/**
 * Datapath
 *
 */

#if (defined(BES_OTA) || defined(BES_OTA_BASIC))&& !defined(OTA_OVER_TOTA_ENABLED)
void bes_ble_ota_event_reg(bes_ble_ota_event_callback cb)
{
    app_ota_event_reg((app_ota_event_callback)cb);
}

void bes_ble_ota_event_unreg(void)
{
    app_ota_event_unreg();
}

void bes_ble_ota_send_rx_cfm(uint8_t conidx)
{
    app_ota_send_rx_cfm(conidx);
}

bool bes_ble_ota_send_notification(uint8_t conidx, uint8_t* ptrData, uint32_t length)
{
    return app_ota_send_notification(conidx, ptrData, length);
}

bool bes_ble_ota_send_indication(uint8_t conidx, uint8_t* ptrData, uint32_t length)
{
    return app_ota_send_indication(conidx, ptrData, length);
}
#endif

#ifdef BLE_TOTA_ENABLED
void bes_ble_tota_event_reg(bes_ble_tota_event_callback cb)
{
    app_tota_event_reg((app_tota_event_callback)cb);
}

void bes_ble_tota_event_unreg(void)
{
    app_tota_event_unreg();
}

bool bes_ble_tota_send_notification(uint8_t conidx, uint8_t* ptrData, uint32_t length)
{
    return app_tota_send_notification(conidx, ptrData, length);
}

bool bes_ble_tota_send_indication(uint8_t conidx, uint8_t* ptrData, uint32_t length)
{
    return app_tota_send_indication(conidx, ptrData, length);
}
#endif

#ifdef __AI_VOICE_BLE_ENABLE__
void bes_ble_ai_gatt_event_reg(bes_ble_ai_event_cb cb)
{
    app_ai_event_reg((app_ble_ai_event_cb)cb);
}

void bes_ble_ai_gatt_data_send(bes_ble_ai_data_send_param_t *param)
{
    app_ai_data_send((app_ble_ai_data_send_param_t *) param);
}
#endif

#ifdef TILE_DATAPATH
void bes_ble_tile_event_cb_reg(bes_ble_tile_event_cb cb)
{
    app_tile_event_cb_reg((app_ble_tile_event_cb)cb);
}

void bes_ble_tile_send_via_notification(uint8_t conidx, uint8_t* ptrData, uint32_t length)
{
    app_tile_send_ntf(conidx, ptrData, length);
}
#endif

/**
 * DATAPATH_SERVER
 *
 */
#ifdef CFG_APP_DATAPATH_SERVER
void bes_ble_datapath_server_register_event_callback(app_datapath_event_cb cb)
{
    app_datapath_server_register_event_callback(cb);
}

void bes_ble_datapath_server_send_data_via_notification(uint8_t conidx, uint8_t* ptrData, uint32_t length)
{
    app_datapath_server_send_data_via_notification(conidx, ptrData, length);
}

void bes_ble_datapath_server_send_data_via_indication(uint8_t conidx, uint8_t* ptrData, uint32_t length)
{
    app_datapath_server_send_data_via_indication(conidx, ptrData, length);
}

void bes_ble_datapath_server_register_event_cb(app_datapath_event_cb callback)
{
    app_datapath_server_register_event_cb(callback);
}
#endif

/**
 * DATAPATH_CLIENT
 *
 */
#if (CFG_APP_DATAPATH_CLIENT)
void bes_ble_datapath_client_control_notification(uint8_t conidx, bool isEnable)
{
    app_datapath_client_control_notification(conidx, isEnable);
}

void bes_ble_datapath_client_send_data_via_write_command(uint8_t conidx, uint8_t* ptrData, uint32_t length)
{
    app_datapath_client_send_data_via_write_command(conidx, ptrData, length);
}

void bes_ble_datapath_client_send_data_via_write_request(uint8_t conidx, uint8_t* ptrData, uint32_t length)
{
    app_datapath_client_send_data_via_write_request(conidx, ptrData, length);
}

void bes_ble_datapath_client_register_callbacK(void *callback)
{
    app_datapath_client_register_callback((app_datapath_client_event_cb_t *)callback);
}
#endif

/**
 * GFPS
 *
 */

#ifdef GFPS_ENABLED
void bes_ble_gfps_init(app_ble_adv_activity_func func)
{
    ble_app_gfps_init(func);
}

#ifdef SPOT_ENABLED
void bes_ble_spot_init(app_ble_adv_activity_func func)
{
    ble_app_spot_init(func);
}

void bes_ble_dult_init(struct ble_dult_cb_t *cbs)
{
    ble_app_dult_init(cbs);
}

void bes_ble_gfps_send_beacon_data(uint8_t conidx, uint8_t *data, uint16_t length)
{
    ble_app_gfps_send_beacon_data(conidx, data, length);
}
#endif

#ifdef SWIFT_ENABLED
void bes_ble_swift_enter_pairing_mode(void)
{
    app_swift_enter_pairing_mode();
}

void bes_ble_swift_exit_pairing_mode(void)
{
    app_swift_exit_pairing_mode();
}
#endif

uint8_t bes_ble_gfps_l2cap_send(uint8_t conidx, uint8_t *ptrData, uint32_t length)
{
    return ble_app_gfps_l2cap_send(conidx, ptrData, length);
}

void bes_ble_gfps_l2cap_disconnect(uint8_t conidx)
{
    ble_app_gfps_l2cap_disconnect(conidx);
}

void bes_ble_gfps_send_keybase_pairing(uint8_t conidx, uint8_t *data, uint16_t length)
{
    ble_app_gfps_send_keybase_pairing(conidx, data, length);
}
#endif /* BLE_APP_GFPS */

#ifdef TWS_SYSTEM_ENABLED
void bes_ble_sync_ble_info(void)
{
    app_ble_sync_ble_info();
}

void bes_ble_gap_mode_tws_sync_init(void)
{
    app_ble_mode_tws_sync_init();
}
#endif

/**
 * BLE AUDIO
 *
 */

#if BLE_AUDIO_ENABLED
void bes_ble_audio_common_deinit(void)
{
    ble_audio_tws_deinit();
}

BLE_AUDIO_POLICY_CONFIG_T* bes_ble_audio_get_policy_config(void)
{
    return app_ble_audio_get_policy_config();
}

void bes_ble_bap_set_activity_type(gaf_bap_activity_type_e type)
{
    app_bap_set_activity_type(type);
}

void bes_ble_update_tws_nv_role(uint8_t role)
{
    ble_audio_update_tws_nv_role(role);
}

bool bes_ble_aob_conn_start_adv(bool br_edr_support, bool discoverable, bool init_reconnect)
{
    return aob_conn_start_adv(br_edr_support, discoverable, init_reconnect);
}

bool bes_ble_aob_conn_stop_adv(void)
{
    return aob_conn_stop_adv();
}

void bes_ble_bap_capa_srv_get_ava_context_bf(uint8_t con_lid, uint16_t *context_bf_ava_sink, uint16_t *context_bf_ava_src)
{
    app_bap_capa_srv_get_ava_context_bf(con_lid, context_bf_ava_sink, context_bf_ava_src);
}

void bes_ble_bap_capa_srv_set_ava_context_bf(uint8_t con_lid, uint16_t context_bf_ava_sink, uint16_t context_bf_ava_src)
{
    app_bap_capa_srv_set_ava_context_bf(con_lid, context_bf_ava_sink, context_bf_ava_src);
}

void bes_ble_aob_gattc_rebuild_cache(GATTC_NV_SRV_ATTR_t *record)
{
    aob_gattc_rebuild_cache(record);
}

void bes_ble_aob_service_recv_handler(uint8_t *p_buff, uint16_t len)
{
    app_ble_audio_recv_service_data(p_buff, len);
}

bool bes_ble_aob_csip_if_set_device_numbers(uint8_t dev_num)
{
    return aob_csip_if_set_device_numbers(dev_num);
}

uint8_t bes_ble_aob_csip_if_get_device_numbers(void)
{
    return aob_csip_if_get_device_numbers();
}

bool bes_ble_aob_csip_if_get_sirk(uint8_t *sirk)
{
    return aob_csip_if_get_sirk(sirk);
}

void bes_ble_aob_csip_if_use_temporary_sirk()
{
    aob_csip_if_use_temporary_sirk();
}

void bes_ble_aob_csip_if_refresh_sirk(uint8_t *sirk)
{
    aob_csip_if_refresh_sirk(sirk);
}

bool bes_ble_aob_csip_sirk_already_refreshed(void)
{
    return aob_csip_sirk_already_refreshed();
}

void bes_ble_aob_csip_if_update_sirk(uint8_t *sirk, uint8_t sirk_len)
{
    aob_csip_if_update_sirk(sirk, sirk_len);
}

bool bes_ble_aob_csip_is_use_custom_sirk(void)
{
    return aob_csip_is_use_custom_sirk();
}

bool bes_ble_aob_csip_get_rsi_value(uint8_t *p_rsi_get)
{
    return aob_csip_if_get_rsi_data(p_rsi_get);
}

void bes_ble_aob_conn_dump_state_info(void)
{
    aob_conn_dump_state_info();
}

void bes_ble_aob_bis_tws_sync_state_req(void)
{

}

void bes_ble_aob_bis_tws_sync_state_req_handler(uint8_t *buf)
{

}

ble_bdaddr_t *bes_ble_aob_conn_get_remote_address(uint8_t con_lid)
{
    return aob_conn_get_remote_address(con_lid);
}

void bes_ble_aob_media_play(uint8_t con_lid)
{
    aob_media_play(con_lid);
}

void bes_ble_aob_media_pause(uint8_t con_lid)
{
    aob_media_pause(con_lid);
}

void bes_ble_aob_media_stop(uint8_t con_lid)
{
    aob_media_stop(con_lid);
}

void bes_ble_aob_media_next(uint8_t con_lid)
{
    aob_media_next(con_lid);
}

void bes_ble_aob_media_prev(uint8_t con_lid)
{
    aob_media_prev(con_lid);
}

void bes_ble_aob_media_fast_fw(uint8_t con_lid)
{
    aob_media_fast_fw(con_lid);
}

void bes_ble_aob_media_fast_rw(uint8_t con_lid)
{
    aob_media_fast_rw(con_lid);
}

void bes_ble_aob_vol_mute(void)
{
    aob_vol_mute();
}

void bes_ble_aob_vol_unmute(void)
{
    aob_vol_unmute();
}

void bes_ble_aob_vol_up(void)
{
    aob_vol_up();
}

void bes_ble_aob_vol_down(void)
{
    aob_vol_down();
}

void bes_ble_aob_call_if_outgoing_dial(uint8_t conidx, uint8_t *uri, uint8_t uriLen)
{
    aob_call_if_outgoing_dial(conidx, uri, uriLen);
}

void bes_ble_aob_call_if_retrieve_call(uint8_t conidx, uint8_t call_id)
{
    aob_call_if_retrieve_call(conidx, call_id);
}

void bes_ble_aob_call_if_hold_call(uint8_t conidx, uint8_t call_id)
{
    aob_call_if_hold_call(conidx, call_id);
}

void bes_ble_aob_call_if_terminate_call(uint8_t conidx, uint8_t call_id)
{
    aob_call_if_terminate_call(conidx, call_id);
}

void bes_ble_aob_call_if_accept_call(uint8_t conidx, uint8_t call_id)
{
    aob_call_if_accept_call(conidx, call_id);
}

void bes_ble_aob_call_if_free_pending_actions(struct list_node *action_list)
{
    aob_call_if_free_pending_actions(action_list);
}

uint8_t bes_ble_aob_call_if_get_any_call_by_conidx(uint8_t conidx, uint8_t *p_call_id)
{
    return aob_call_if_get_not_idle_call_by_conidx(conidx, p_call_id);
}

uint8_t bes_ble_aob_convert_local_vol_to_le_vol(uint8_t bt_vol)
{
    return aob_convert_local_vol_to_le_vol(bt_vol);
}

int bes_ble_audio_bis_stream_set_resume_callback(void (*resume_cb)(uint8_t device_id, uint32_t param))
{
    return app_ble_audio_bis_stream_set_resume_callback(resume_cb);
}

void bes_ble_audio_sink_streaming_handle_event(uint8_t con_lid, uint8_t data,
                                                                bes_gaf_direction_t direction, app_ble_audio_event_t event)
{
    app_ble_audio_sink_streaming_handle_event(con_lid, data, (uint8_t)direction, event);
}

uint8_t bes_ble_audio_get_mobile_addr(uint8_t deviceId, uint8_t *addr)
{
    AOB_MOBILE_INFO_T *pLEMobileInfo = ble_audio_earphone_info_get_mobile_info(deviceId);
    if(NULL != pLEMobileInfo)
    {

        memcpy(addr, pLEMobileInfo->peer_ble_addr.addr, 6);
    }
    else
    {
        return -1;
    }
    return 0;
}

void bes_ble_audio_dump_conn_state(void)
{
    ble_adv_activity_t *p_actv_le_audio = app_ble_get_advertising_by_user(USER_BLE_AUDIO);

    if (p_actv_le_audio != NULL && p_actv_le_audio->adv_is_started)
    {
        TRACE(3, "BLE adv state: 5, adv type: %d, interval: %d, busy state: 0",
              !p_actv_le_audio->adv_param.use_legacy_pdu && p_actv_le_audio->adv_param.connectable ? 5 : 0,
              p_actv_le_audio->custom_adv_interval_ms);
    }
    else
    {
        TRACE(1, "BLE adv state: 0, adv type: 0, interval: 0, busy state: 0");
    }
    // Judge ble whether have connect handle
    if (app_ble_is_any_connection_exist())
    {
        TRACE(1, "BLE cnn state: %d", BLE_CONNECTED);
    }
    else
    {
        TRACE(1, "BLE cnn state: %d", BLE_DISCONNECTED);
    }

    for (uint8_t index = 0; index < AOB_COMMON_MOBILE_CONNECTION_MAX; index++)
    {
        // dump mobile state
        AOB_MOBILE_INFO_T *p_mobileInfo = ble_audio_earphone_info_get_mobile_info(index);

        if (NULL != p_mobileInfo && true == p_mobileInfo->connected)
        {
            BLE_ADDR_INFO_T bleAddr = p_mobileInfo->peer_ble_addr;
            TRACE(9, "[BLE Audio]: mobile: %d mute: %d, volume: %d, address: %02x:%02x:%02x:%02x:%02x:%02x",
                p_mobileInfo->conidx, p_mobileInfo->muted, p_mobileInfo->volume,
                bleAddr.addr[0], bleAddr.addr[1], bleAddr.addr[2],bleAddr.addr[3], bleAddr.addr[4], bleAddr.addr[5]);
            //dump ble cis and ase state

            app_bap_ascs_ase_t *p_ase = app_bap_uc_srv_get_ase_info(p_mobileInfo->conidx);

            if (NULL != p_ase)
            {
                TRACE(3, "[BLE Audio]: index: %d, cis state: %d, handle: %d", p_mobileInfo->conidx, p_ase->con_lid, p_ase->cis_hdl);
            }
        }
    }
}

uint8_t bes_ble_aob_get_call_id_by_conidx_and_type(uint8_t device_id, uint8_t call_state)
{
    return ble_audio_earphoe_info_get_call_id_by_conidx_and_type(device_id, call_state);
}

uint8_t bes_ble_aob_get_call_id_by_conidx(uint8_t device_id)
{
    return ble_audio_earphone_info_get_call_id_by_conidx(device_id);
}

bool bes_ble_aob_get_acc_bond_status(uint8_t conidx, uint8_t type)
{
    return ble_audio_earphone_info_get_acc_bond_status(conidx, type);
}

//// BLE bis api
// bis src
uint16_t bes_ble_bis_src_get_bis_hdl_by_big_idx(uint8_t big_idx)
{
#ifdef APP_BLE_BIS_SRC_ENABLE
    return aob_bis_src_get_bis_hdl_by_big_idx(big_idx);
#else
    return 0;
#endif
}

const AOB_CODEC_ID_T *bes_ble_bis_src_get_codec_id_by_big_idx(uint8_t big_idx, uint8_t subgrp_idx)
{
#ifdef APP_BLE_BIS_SRC_ENABLE
    return aob_bis_src_get_codec_id_by_big_idx(big_idx, subgrp_idx);
#else
    return NULL;
#endif
}

const AOB_BAP_CFG_T *bes_ble_bis_src_get_codec_cfg_by_big_idx(uint8_t big_idx)
{
#ifdef APP_BLE_BIS_SRC_ENABLE
    return aob_bis_src_get_codec_cfg_by_big_idx(big_idx);
#else
    return NULL;
#endif
}

uint32_t bes_ble_bis_src_get_iso_interval_ms_by_big_idx(uint8_t big_idx)
{
#ifdef APP_BLE_BIS_SRC_ENABLE
    return aob_bis_src_get_iso_interval_ms_by_big_idx(big_idx);
#else
    return 0;
#endif
}

uint8_t bes_ble_bis_src_send_iso_data_to_all_channel(uint8_t **payload, uint16_t payload_len, uint32_t ref_time)
{
#ifdef APP_BLE_BIS_SRC_ENABLE
    return app_bap_bc_src_iso_send_data_to_all_channel(payload, payload_len, ref_time);
#else
    return 0;
#endif
}

void bes_ble_bis_src_set_big_param(uint8_t big_idx, bes_ble_bis_src_big_param_t *p_big_param)
{
#ifdef APP_BLE_BIS_SRC_ENABLE
    aob_bis_src_set_big_param(big_idx, (aob_bis_src_big_param_t *)p_big_param);
#endif
}

void bes_ble_bis_src_set_subgrp_param(uint8_t big_idx, bes_ble_bis_src_subgrp_param_t *p_subgrp_param)
{
#ifdef APP_BLE_BIS_SRC_ENABLE
    aob_bis_src_set_subgrp_param(big_idx, (aob_bis_src_subgrp_param_t *)p_subgrp_param);
#endif
}

void bes_ble_bis_src_set_stream_param(uint8_t big_idx, bes_ble_bis_src_stream_param_t *p_stream_param)
{
#ifdef APP_BLE_BIS_SRC_ENABLE
    aob_bis_src_set_stream_param(big_idx, (aob_bis_src_stream_param_t *)p_stream_param);
#endif
}

void bes_ble_bis_src_update_metadata(uint8_t grp_lid, uint8_t sgrp_lid, bes_ble_bis_src_metadata_update_t *metadata)
{
#ifdef APP_BLE_BIS_SRC_ENABLE
    aob_bis_src_update_metadata(grp_lid, sgrp_lid, (app_gaf_bap_cfg_metadata_t *)metadata);
#endif
}

void bes_ble_bis_src_write_bis_data(uint8_t big_idx, uint8_t stream_lid, uint8_t *data, uint16_t data_len)
{
#ifdef APP_BLE_BIS_SRC_ENABLE
    aob_bis_src_write_bis_data(big_idx, stream_lid, data, data_len);
#endif
}

uint32_t bes_ble_bis_src_get_stream_anchor_time(uint8_t big_idx, uint8_t stream_lid)
{
#ifdef APP_BLE_BIS_SRC_ENABLE
    return aob_bis_src_get_anchor_time(big_idx, stream_lid);
#else
    return 0;
#endif
}

void bes_ble_bis_src_start(uint8_t big_idx, bes_ble_bis_src_start_param_t *event_info)
{
#ifdef APP_BLE_BIS_SRC_ENABLE
    aob_bis_src_start(big_idx, (aob_bis_src_started_info_t *)event_info);
#endif
}

void bes_ble_bis_src_stop(uint8_t big_idx)
{
#ifdef APP_BLE_BIS_SRC_ENABLE
    aob_bis_src_stop(big_idx);
#endif
}

// bis sink
void bes_ble_bis_start_scan(void)
{
#ifdef APP_BLE_BIS_SINK_ENABLE
    aob_bis_start_scan();
#endif
}

void bes_ble_bis_stop_scan(void)
{
#ifdef APP_BLE_BIS_SINK_ENABLE
    aob_bis_stop_scan();
#endif
}

void bes_ble_bis_sink_start(bes_ble_bis_sink_start_param_t *param)
{
#ifdef APP_BLE_BIS_SINK_ENABLE
    aob_bis_sink_start((aob_bis_sink_start_param_t *)param);
#endif
}

void bes_ble_bis_sink_set_src_id_key(uint8_t *bcast_id, uint8_t *bcast_code)
{
#ifdef APP_BLE_BIS_SINK_ENABLE
    aob_bis_sink_set_src_id_key(bcast_id, bcast_code);
#endif
}

uint16_t bes_ble_bis_sink_get_pa_sync_hdl_by_pa_lid(uint8_t pa_lid)
{
#ifdef APP_BLE_BIS_SINK_ENABLE
    return aob_bis_sink_get_pa_sync_hdl_by_pa_lid(pa_lid);
#endif
}

void bes_ble_bis_scan_pa_sync_with_to(ble_bdaddr_t *p_addr, uint8_t adv_sid, uint16_t sync_to_s)
{
#ifdef APP_BLE_BIS_SINK_ENABLE
    if (p_addr == NULL)
    {
        return;
    }

    aob_bis_scan_pa_sync_with_to(p_addr->addr, p_addr->addr_type, adv_sid, sync_to_s);
#endif
}

void bes_ble_bis_scan_pa_sync_cancel(void)
{
#ifdef APP_BLE_BIS_SINK_ENABLE
    aob_bis_scan_pa_sync_cancel();
#endif
}

void bes_ble_bis_scan_pa_sync_stop(void)
{
#ifdef APP_BLE_BIS_SINK_ENABLE
    aob_bis_scan_pa_sync_stop();
#endif
}

void bes_ble_bis_scan_pa_report_ctrl(uint8_t pa_lid, bool enable)
{
#ifdef APP_BLE_BIS_SINK_ENABLE
    aob_bis_scan_pa_report_ctrl(pa_lid, enable);
#endif
}

void bes_ble_bis_sink_stop()
{
#ifdef APP_BLE_BIS_SINK_ENABLE
    aob_bis_sink_stop();
#endif
}

void bes_ble_bis_sink_set_aud_location_bf(uint32_t aud_loc_bf)
{
    aob_bis_sink_set_player_channel(aud_loc_bf);
}

void bes_ble_bap_start_discovery(uint8_t con_lid)
{
    app_bap_start(con_lid);
}

void bes_ble_start_gaf_discovery(uint8_t con_lid)
{
    app_gaf_mobile_start_discovery(con_lid);
}

gaf_bap_activity_type_e bes_ble_bap_get_actv_type(void)
{
    return app_bap_get_activity_type();
}

#ifdef AOB_MOBILE_ENABLED
uint8_t bes_ble_bap_get_device_num_to_be_connected(void)
{
    return app_bap_get_device_num_to_be_connected();
}
#endif /// AOB_MOBILE_ENABLED

/*BAP ASCS API*/
const bes_ble_bap_ascs_ase_t *bes_ble_get_ascs_ase_info(uint8_t ase_lid)
{
    CHECK_SIZE_TYPE(bes_ble_bap_ascs_ase_t, sizeof(app_bap_ascs_ase_t));

    return (bes_ble_bap_ascs_ase_t *)app_bap_uc_srv_get_ase_info(ase_lid);
}

uint8_t bes_ble_bap_ascs_get_streaming_ase_lid_list(uint8_t con_lid, uint8_t *ase_lid_list)
{
    return aob_media_get_curr_streaming_ase_lid_list(con_lid, ase_lid_list);
}

void bes_ble_bap_ascs_send_ase_enable_rsp(uint8_t ase_lid, bool accept)
{
    aob_media_send_enable_rsp(ase_lid, accept);
}

void bes_ble_bap_ascs_disable_ase_req(uint8_t ase_lid)
{
    aob_media_disable_stream(ase_lid);
}

void bes_ble_bap_ascs_release_ase_req(uint8_t ase_lid)
{
    aob_media_release_stream(ase_lid);
}

uint32_t bes_ble_bap_capa_get_location_bf(bes_gaf_direction_t direction)
{
    return aob_pacs_get_cur_audio_location(direction);
}

void bes_ble_bap_dp_itf_data_come_callback_register(void *callback)
{
    app_bap_dp_itf_data_come_callback_register(callback);
}

void bes_ble_bap_dp_itf_data_come_callback_deregister(void)
{
    app_bap_dp_itf_data_come_callback_deregister();
}

uint8_t bes_ble_audio_get_tws_nv_role(void)
{
    return ble_audio_get_tws_nv_role();
}

uint8_t bes_ble_audio_get_location_fs_l_r_cnt(uint32_t audio_location_bf)
{
    return app_bap_get_audio_location_l_r_cnt(audio_location_bf);
}

uint8_t *bes_ble_audio_get_ltv_value_by_type(AOB_CFG_LTV_T *p_ltv_data, uint8_t ltv_type)
{
    return app_bap_get_ltv_value_by_type((app_gaf_ltv_t *)p_ltv_data, ltv_type);
}

uint8_t bes_ble_arc_get_mic_state(uint8_t con_lid)
{
    return aob_media_get_mic_state(con_lid);
}

void bes_ble_bap_iso_dp_send_data(uint16_t conhdl, uint16_t seq_num, uint8_t *payload, uint16_t payload_len, uint32_t ref_time)
{
    app_bap_dp_itf_send_data(conhdl, seq_num, payload, payload_len, ref_time);
}

uint8_t bes_ble_arc_convert_le_vol_to_local_vol(uint8_t le_vol)
{
    return aob_convert_le_vol_to_local_vol(le_vol);
}

uint8_t bes_ble_arc_vol_get_real_time_volume(uint8_t con_lid)
{
    return aob_vol_get_real_time_volume(con_lid);
}

#ifdef AOB_MOBILE_ENABLED
void bes_ble_arc_mobile_set_abs_vol(uint8_t con_lid, uint8_t local_vol)
{
    aob_mobile_vol_set_abs(con_lid, local_vol);
}
#endif

int bes_ble_bap_get_free_iso_packet_num(void)
{
    return app_bap_get_free_packet_num();
}

void *bes_ble_bap_dp_itf_get_rx_data(uint16_t iso_hdl, bes_ble_dp_itf_iso_buffer_t *p_iso_buffer)
{
    return app_bap_dp_itf_get_rx_data(iso_hdl, (dp_itf_iso_buffer_t *)p_iso_buffer);
}

void bes_ble_bap_dp_tx_iso_stop(uint16_t iso_hdl)
{
    app_bap_dp_tx_iso_stop(iso_hdl);
}

void bes_ble_bap_dp_rx_iso_stop(uint16_t iso_hdl)
{
    app_bap_dp_rx_iso_stop(iso_hdl);
}

bool bes_ble_ccp_call_is_device_call_active(uint8_t con_lid)
{
    return aob_call_is_device_call_active(con_lid);
}

void bes_ble_gaf_media_status_handler_cb_register(void (*cb)(uint8_t con_lid, bool paused))
{
    app_ble_audio_gaf_media_status_handler_cb_register(cb);
}

#ifdef AOB_MOBILE_ENABLED
void bes_lea_mobile_stream_start(uint8_t con_lid, bes_lea_ase_cfg_param_t *cfg, bool bidirectional)
{
    CHECK_SIZE_TYPE(bes_lea_ase_cfg_param_t, sizeof(AOB_MEDIA_ASE_CFG_INFO_T));

    aob_media_mobile_start_stream((AOB_MEDIA_ASE_CFG_INFO_T *)cfg, con_lid, bidirectional);
}

const bes_ble_bap_ascc_ase_t *bes_ble_bap_ascc_get_ase_info(uint8_t ase_lid)
{
    CHECK_SIZE_TYPE(bes_ble_bap_ascc_ase_t, sizeof(app_bap_ascc_ase_t));

    return (bes_ble_bap_ascc_ase_t *)app_bap_uc_cli_get_ase_info_by_ase_lid(ase_lid);
}

uint8_t bes_ble_bap_ascc_get_specific_state_ase_lid_list(uint8_t con_lid, uint8_t direction, uint8_t ase_state, uint8_t *ase_lid_list)
{
    return app_bap_uc_cli_get_specific_state_ase_lid_list(con_lid, direction, ase_state, ase_lid_list);
}

bool bes_ble_bap_pacc_is_peer_support_stereo_channel(uint8_t con_lid, uint8_t direction)
{
    return app_bap_capa_cli_is_peer_support_stereo_channel(con_lid, direction);
}

void bes_ble_bap_ascc_configure_codec_by_ase_lid(uint8_t ase_lid, uint8_t cis_id, const AOB_CODEC_ID_T *codec_id,
                                                        uint16_t sampleRate, uint16_t frame_octet)
{
    app_bap_uc_cli_configure_codec(ase_lid, cis_id, (app_gaf_codec_id_t *)codec_id, sampleRate, frame_octet);
}

void bes_ble_bap_ascc_ase_release_by_ase_lid(uint8_t ase_lid)
{
    app_bap_uc_cli_stream_release(ase_lid);
}

void bes_ble_bap_ascc_ase_disable_by_ase_lid(uint8_t ase_lid)
{
    app_bap_uc_cli_stream_disable(ase_lid);
}

void bes_ble_bap_ascc_link_create_group_req(uint8_t cig_lid)
{
    app_bap_uc_cli_link_create_group_req(cig_lid);
}

void bes_ble_bap_ascc_link_remove_group_req(uint8_t grp_lid)
{
    app_bap_uc_cli_link_remove_group_cmd(grp_lid);
}

void bes_ble_bap_ascc_ase_qos_cfg_by_ase_lid(uint8_t ase_lid, uint8_t grp_lid)
{
    app_bap_uc_cli_configure_qos(ase_lid, grp_lid, 0);
}

void bes_ble_bap_ascc_ase_enable_by_ase_lid(uint8_t ase_lid, uint16_t context_bf)
{
    app_bap_uc_cli_enable_stream(ase_lid, context_bf);
}

void bes_ble_bap_ascc_set_sdu_interval(uint32_t sdu_intv_c2p_us, uint32_t sdu_intv_p2c_us)
{
    aob_media_mobile_set_sdu_interval(sdu_intv_c2p_us, sdu_intv_p2c_us);
}

void bes_ble_bap_ascc_set_cis_count_in_cig(uint8_t cis_count)
{
    aob_media_mobile_set_cis_count_in_cig(cis_count);
}

void bes_ble_bap_ascc_prepare_cig_param(const AOB_BAP_CIG_PARAM_T *cig_param)
{
    aob_media_mobile_prepare_cig_param(cig_param);
}

void bes_ble_mcp_mcs_action_control(uint8_t media_lid, uint8_t action)
{
    aob_media_mobile_action_control(media_lid, action);
}

void bes_ble_mcp_mcs_track_changed(uint8_t media_lid, uint32_t duration_10ms, uint8_t *title, uint8_t title_len)
{
    aob_media_mobile_change_track(media_lid, duration_10ms, title, title_len);
}
#endif /// AOB_MOBILE_ENABLED
#endif /* BLE_AUDIO_ENABLED */

#ifdef BLE_ISO_ENABLED
void *bes_ble_iso_malloc_buff(uint32_t size)
{
    return app_iso_heap_malloc(size);
}

void bes_ble_iso_free_buff(void *mem_ptr)
{
    app_iso_heap_free(mem_ptr);
}

void bes_ble_iso_rx_free_buff(void *mem_ptr)
{
    app_iso_rx_free(mem_ptr);
}

int bes_ble_iso_get_free_iso_packet_num(void)
{
    return app_iso_get_free_packet_num();
}

void *bes_ble_iso_dp_itf_get_rx_data(uint16_t iso_hdl, bes_ble_dp_itf_iso_buffer_t *p_iso_buffer)
{
    return app_iso_dp_itf_get_rx_data(iso_hdl, (dp_itf_iso_buffer_t *)p_iso_buffer);
}

void bes_ble_iso_dp_tx_iso_stop(uint16_t iso_hdl)
{
    app_iso_dp_tx_iso_stop(iso_hdl);
}

void bes_ble_iso_dp_rx_iso_stop(uint16_t iso_hdl)
{
    app_iso_dp_rx_iso_stop(iso_hdl);
}

void bes_ble_iso_dp_send_data(uint16_t conhdl, uint16_t seq_num, uint8_t *payload, uint16_t payload_len, uint32_t ref_time)
{
    app_iso_dp_itf_send_data(conhdl, seq_num, payload, payload_len, ref_time);
}

void bes_ble_iso_dp_set_rx_dp_itf(void)
{
    app_iso_dp_set_rx_dp_itf();
}

void bes_ble_iso_dp_itf_data_come_callback_register(void *callback)
{
    app_iso_dp_itf_data_come_callback_register(callback);
}

void bes_ble_iso_dp_itf_data_come_callback_deregister(void)
{
    app_iso_dp_itf_data_come_callback_deregister();
}

#endif

#if (GATT_RATE_TESTS)
void bes_ble_rate_test_server_send_data_via_notification(uint8_t* ptrData, uint32_t length)
{
    app_rate_test_server_send_data_via_notification(ptrData, length);
}
void bes_ble_rate_test_server_send_data_via_intification(uint8_t* ptrData, uint32_t length)
{
    app_rate_test_server_send_data_via_indication(ptrData, length);
}
void bes_ble_rate_test_server_register_connected_done(bes_ble_rate_test_server_connected_done_t callback)
{
    app_rate_test_server_register_connected_done(callback);
}

void bes_ble_rate_test_server_register_conn_param_update_done(bes_ble_rate_test_server_conn_param_update_done_t callback)
{
    app_rate_test_server_register_conn_param_update_done(callback);
}
void bes_ble_rate_test_server_register_tx_done(bes_ble_rate_test_server_tx_done_t callback)
{
    app_rate_test_server_register_tx_done(callback);
}
#endif /* GATT_RATE_TESTS */

#if (GATT_RATE_TESTC)
void bes_ble_rate_test_client_register_connected_done(bes_ble_rate_test_client_connected_done_t callback)
{
    app_rate_test_client_register_connected_done(callback);
}

void bes_ble_rate_test_client_discover(uint8_t conidx)
{
    app_rate_test_client_discover(conidx);
}

void bes_ble_rate_test_client_register_conn_param_update_done(bes_ble_rate_test_client_conn_param_update_done_t callback)
{
    app_rate_test_client_register_conn_param_update_done(callback);
}

void bes_ble_rate_test_client_register_tx_done(bes_ble_rate_test_client_tx_done_t callback)
{
    app_rate_test_client_register_tx_done(callback);
}

void bes_ble_rate_test_client_send_data_via_write_command(uint8_t* ptrData, uint32_t length)
{
    app_rate_test_client_send_data_via_write_command(ptrData, length);
}

#endif /* GATT_RATE_TESTC */

#ifdef BLE_WIRELESS_TRANS_SRV_ENABLED
void app_ble_wireless_trans_srv_register_callback(void *callback)
{
    ble_wireless_trans_srv_register_callback((ble_wireless_trans_server_cb_t *)callback);
}

uint8_t app_ble_srv_send_attr1_data_via_notification(uint8_t *value, uint16_t len)
{
    return ble_wireless_trans_srv_send_attr1_data_via_notification(value, len);
}
#endif

#ifdef BLE_WIRELESS_TRANS_CLI_ENABLED
void app_ble_wireless_trans_cli_register_callback(void *callback)
{
    ble_wireless_trans_cli_register_callback((ble_wireless_trans_client_cb_t *)callback);
}

// uart1 rece data send to attr0
uint8_t app_ble_wireless_trans_cli_send_attr0_data_handler(uint16_t connhdl, const uint8_t *value, uint16_t len)
{
    return ble_wireless_trans_cli_send_attr0_data_handler(connhdl, value, len);
}
#endif
