#include "bluetooth_bt_api.h"
#include "cmsis.h"
#include "app_bt.h"
#include "app_bt_cmd.h"
#include "me_api.h"
#include "bt_if.h"
#include "cobuf.h"
#include "btapp.h"
#include "besbt.h"
#include "besbt_cfg.h"
#include "conmgr_api.h"
#include "cobuf.h"
#include "co_ppbuff.h"
#include "app_bt_func.h"
#include "app_a2dp.h"
#include "audio_policy.h"
#include "a2dp_api.h"
#include "avrcp_api.h"
#include "hfp_api.h"
#include "app_hfp.h"
#include "hfp_i.h"
#include "hci_api.h"
#include "hci_i.h"
#include "l2cap_api.h"
#include "l2cap_i.h"
#include "app_btgatt.h"
#include "sdp_api.h"
#include "spp_api.h"
#include "app_dip.h"
#include "dip_api.h"
#include "sco_api.h"
#include "besaud_api.h"
#include "btm_i.h"
#include "app_a2dp_source.h"
#include "bt_source.h"
#include "app_bt_hid.h"
#include "app_map.h"
#include "map_api.h"

static int (*g_bt_audio_event_handler)(uint8_t device_id, enum app_bt_audio_event_t event, uint32_t data) = NULL;

int bt_audio_event_handler(uint8_t device_id, enum app_bt_audio_event_t event, uint32_t data)
{
    return g_bt_audio_event_handler ? g_bt_audio_event_handler(device_id, event, data) : 0;
}

void bt_audio_set_event_handler(int (*hdlr)(uint8_t device_id, enum app_bt_audio_event_t event, uint32_t data))
{
    g_bt_audio_event_handler = hdlr;
}

POSSIBLY_UNUSED static void bt_export_profile_connect(BT_PROFILE_ID_ENUM_T profile, const bt_bdaddr_t *remote)
{
    switch (profile)
    {
        case BT_PROFILE_HFP:
            app_bt_reconnect_hfp_profile((bt_bdaddr_t *)remote);
            break;
        case BT_PROFILE_A2DP:
            app_bt_reconnect_a2dp_profile((bt_bdaddr_t *)remote);
            break;
        case BT_PROFILE_AVRCP:
            app_bt_reconnect_avrcp_profile((bt_bdaddr_t *)remote);
            break;
        default:
            break;
    }
}

POSSIBLY_UNUSED static void bt_export_profile_disconnect(BT_PROFILE_ID_ENUM_T profile, int device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (curr_device == NULL)
    {
        return;
    }

    switch (profile)
    {
        case BT_PROFILE_HFP:
            app_bt_disconnect_hfp_profile(curr_device->hf_channel);
            break;
        case BT_PROFILE_A2DP:
            app_bt_disconnect_a2dp_profile(curr_device->a2dp_connected_stream);
            break;
        case BT_PROFILE_AVRCP:
            app_bt_disconnect_avrcp_profile(curr_device->avrcp_channel);
            break;
        default:
            break;
    }
}

uint8_t bt_export_count_link(BT_COUNT_LINK_ENUM_T type)
{
    uint8_t count = 0;

    switch (type)
    {
        case BT_COUNT_MOBILE_LINK:
            count = app_bt_count_connected_device();
            break;
        case BT_COUNT_MOBILE_TWS_LINK:
            count = app_bt_get_active_cons();
            break;
        case BT_COUNT_SOURCE_LINK:
            count = btif_me_get_source_activeCons();
            break;
        case BT_COUNT_TOTAL_ACL_LINK:
            count = app_bt_get_active_cons() + btif_me_get_source_activeCons();
            break;
#if defined(BT_HFP_SUPPORT)
        case BT_COUNT_CONNECTED_SCO:
            count = app_bt_audio_count_connected_sco();
            break;
#endif // BT_HFP_SUPPORT
        case BT_COUNT_STREAMING_A2DP:
            count = app_bt_audio_count_streaming_a2dp();
            break;
        case BT_COUNT_STREAMING_LINK: // a2dp+sco
            count = app_bt_audio_count_straming_mobile_links();
            break;
        default:
            break;
    }

    return count;
}

bt_status_t bt_export_me_write_access_mode(btif_accessible_mode_t mode,bool disable_before_update)
{
#if defined(IBRT)
    if (disable_before_update)
    {
        return BT_STS_PENDING;
    }
#endif
    app_bt_set_access_mode(mode);
    return BT_STS_PENDING;
}

static void bt_export_write_scan_type(BT_SCAN_ENUM_T op, uint8_t scan_type)
{
    if (op == BT_PAGE_SCAN)
    {
        app_bt_start_custom_function_in_bt_thread(scan_type, 0, (uint32_t)(uintptr_t)btif_me_write_bt_page_scan_type);
    }
    else
    {
        app_bt_start_custom_function_in_bt_thread(scan_type, 0, (uint32_t)(uintptr_t)btif_me_write_bt_inquiry_scan_type);
    }
}

static void bt_export_write_scan_activity(BT_SCAN_ENUM_T op, uint16_t scan_interval, uint16_t scan_window)
{
    if (op == BT_PAGE_SCAN)
    {
        app_bt_call_func_in_bt_thread(BTIF_HCC_WRITE_PAGE_SCAN_ACTIVITY, scan_interval, scan_window,
            0, (uint32_t)(uintptr_t)btif_me_write_scan_activity_specific);
    }
    else
    {
        app_bt_call_func_in_bt_thread(BTIF_HCC_WRITE_INQ_SCAN_ACTIVITY, scan_interval, scan_window,
            0, (uint32_t)(uintptr_t)btif_me_write_scan_activity_specific);
    }
}

uint8_t bt_export_select_device(BT_SELECT_DEVICE_ENUM_T op)
{
    uint8_t device_id = BT_DEVICE_INVALID_ID;

    switch (op)
    {
        case BT_SELECT_CURR_A2DP_DEVICE:
            device_id = app_bt_audio_get_curr_a2dp_device();
            break;
#ifdef BT_HFP_SUPPORT
        case BT_SELECT_CALL_ACTIVE_HFP_DEVICE:
            device_id = app_bt_audio_select_call_active_hfp();
            break;
        case BT_SELECT_CURR_HFP_DEVICE:
            device_id = app_bt_audio_get_curr_hfp_device();
            break;
        case BT_SELECT_CURR_PLAYING_SCO:
            device_id = app_bt_audio_get_curr_playing_sco();
            break;
#endif
        case BT_SELECT_CURR_PLAYING_A2DP:
            device_id = app_bt_audio_get_curr_playing_a2dp();
            break;
        case BT_SELECT_CONNECTED_DEVICE:
            device_id = app_bt_audio_select_connected_device();
            break;
        case BT_SELECT_USER_ACTION_DEVICE:
            device_id = app_bt_audio_get_device_for_user_action();
            break;
        default:
            break;
    }

    return device_id;
}

uint8_t bt_export_select_another_device(BT_SELECT_ANOTHER_DEVICE_ENUM_T op, int curr_device_id)
{
    uint8_t device_id = BT_DEVICE_INVALID_ID;

    switch (op)
    {
#ifdef BT_HFP_SUPPORT
        case BT_SELECT_ANOTHER_CREATE_SCO_DEVICE:
            device_id = app_bt_audio_select_another_device_to_create_sco(curr_device_id);
            break;
#endif
        case BT_SELECT_ANOTHER_STREAMING_A2DP_DEVICE:
            device_id = app_bt_audio_select_another_streaming_a2dp(curr_device_id);
            break;
        default:
            break;
    }

    return device_id;
}

static bt_bdaddr_t *bt_export_cmgr_get_address(void *cmgr_handler)
{
    btif_remote_device_t *remDev = btif_cmgr_get_cmgrhandler_remdev(cmgr_handler);
    return btif_me_get_remote_device_bdaddr(remDev);
}

void bes_bt_me_pair_state_callback_deregister(void)
{
    app_bt_pair_state_callback_deregister();
}

bt_acl_state_t bt_export_acl_get_state_by_rdev(btif_remote_device_t *remote_dev)
{
    bt_acl_state_t acl_state = {{{0}}, 0};
    acl_state.acl_conn_hdl = BT_INVALID_CONN_HANDLE;
    acl_state.device_id = BT_DEVICE_INVALID_ID;
    if (remote_dev)
    {
        struct btm_conn_item_t *btm_conn = (struct btm_conn_item_t *)remote_dev;
        acl_state.remote = *((bt_bdaddr_t *)&btm_conn->remote);
        acl_state.acl_conn_hdl = btm_conn->conn_handle;
        acl_state.acl_is_connected = true;
        acl_state.acl_link_mode = btm_conn->mode;
        acl_state.acl_bt_role = btm_conn->btm_bt_role;
        acl_state.device_id = btm_conn->device_id;
        acl_state.sniff_interval = btm_conn->sniff_interval;
    }
    return acl_state;
}

static bt_acl_state_t bt_export_acl_get_state_by_address(const bt_bdaddr_t *remote)
{
    return bt_export_acl_get_state_by_rdev(app_bt_get_remote_dev_by_address(remote));
}

static bt_acl_state_t bt_export_acl_get_state_by_handle(uint16_t conn_handle)
{
    return bt_export_acl_get_state_by_rdev(app_bt_get_remote_dev_by_handle(conn_handle));
}

static bt_remver_t bt_export_acl_get_remote_version(uint16_t conn_handle)
{
    btif_remote_device_t *remote_dev = app_bt_get_remote_dev_by_handle(conn_handle);
    bt_remver_t remver = {0};
    if (remote_dev)
    {
        uint8_t * p_remver = btif_me_get_remote_device_version(remote_dev);
        remver = *((bt_remver_t *)p_remver);
    }
    return remver;
}

void bes_bt_app_start_inquiry(void)
{
    app_bt_start_search();
}

void bes_bt_app_stop_inquiry(void)
{
    app_bt_stop_inquiry();
}

bt_status_t bes_bt_me_start_inquiry(uint32_t lap, uint8_t len, uint8_t maxResp)
{
    return btif_me_inquiry(lap, len, maxResp);
}

void bes_bt_sco_response(uint8_t device_id, uint8_t accept)
{
    btif_sco_connect_resp(device_id, accept);
}

bt_status_t bes_bt_me_setup_acl_qos_with_tpoll_generic(uint16_t conn_handle, uint32_t tpoll_slot, uint8_t service_type)
{
    return btif_me_qos_setup_with_tpoll_generic(conn_handle, tpoll_slot, service_type);
}

bool bes_bt_me_is_in_active_mode(bt_bdaddr_t *remote)
{
    return btif_me_is_in_active_mode(remote);
}

uint8_t bes_bt_me_set_io_capabilities(uint8_t ioCap)
{
    return btif_sec_set_io_capabilities(ioCap);
}

uint8_t bes_bt_me_set_authrequirements(uint8_t authRequirements)
{
    return btif_sec_set_authrequirements(authRequirements);
}

void bes_bt_me_set_extended_inquiry_response(uint8_t* eir, uint32_t len)
{
    btif_set_extended_inquiry_response(eir, len);
}

uint8_t bes_bt_me_get_device_id_from_address(const bt_bdaddr_t *remote)
{
    return btif_me_get_device_id_from_addr(remote);
}

void bes_bt_me_write_scan_type(BT_SCAN_ENUM_T op, uint8_t scan_type)
{
    bt_export_write_scan_type(op, scan_type);
}

void bes_bt_me_write_scan_activity(BT_SCAN_ENUM_T op, uint16_t scan_interval, uint16_t scan_window)
{
    bt_export_write_scan_activity(op, scan_interval, scan_window);
}

bt_status_t bes_bt_me_write_page_timeout(uint16_t page_timeout_slots)
{
    return btif_me_write_page_timeout(page_timeout_slots);
}

bool bes_bt_me_profile_is_connected_before(uint8_t device_id)
{
    return app_bt_is_profile_connected_before(device_id);
}

void *bes_bt_me_cmgr_get_handler(uint16_t conn_handle)
{
    return btif_cmgr_get_acl_handler(conn_handle);
}

bt_bdaddr_t *bes_bt_me_cmgr_get_address(void *cmgr_handler)
{
    return bt_export_cmgr_get_address(cmgr_handler);
}

btif_sniff_info_t *bes_bt_me_cmgr_get_sniff_info(void *cmgr_handler)
{
    return btif_cmgr_get_cmgrhandler_sniff_info(cmgr_handler);
}

bt_status_t bes_bt_me_cmgr_set_sniff_info(uint16_t conn_handle, btif_sniff_info_t *sniff_info)
{
    return btif_cmgr_set_sniff_info_by_handle(conn_handle, sniff_info);
}

bt_status_t bes_bt_me_cmgr_set_sniff_timer(void *cmgr_handler, btif_sniff_info_t *sniff_info, void *timer_mgr)
{
    return btif_cmgr_set_sniff_timer(cmgr_handler, sniff_info, timer_mgr);
}

void bes_bt_me_cmgr_start_sniff_timer(evm_timer_t* timer, uint32_t time)
{
    btif_evm_start_timer(timer, time);
}

bt_status_t bes_bt_me_cmgr_register_callback(void *cmgr_handler, btif_cmgr_callback callback)
{
    return btif_cmgr_register_handler(cmgr_handler, callback);
}

void bes_bt_me_acl_set_remote_device_role(uint16_t conn_handle, uint8_t role)
{
    btif_me_set_remote_device_role(conn_handle, role);
}

bool bes_bt_me_is_device_profile_connected(uint8_t device_id)
{
    return app_bt_is_device_profile_connected(device_id);
}

bool bes_bt_me_is_device_hfp_connected(uint8_t device_id)
{
    return app_is_hfp_service_connected(device_id);
}

bool bes_bt_me_is_device_a2dp_connected(uint8_t device_id)
{
    return app_bt_a2dp_service_is_connected(device_id);
}

bt_acl_state_t bes_bt_me_acl_get_state_by_address(const bt_bdaddr_t *remote)
{
    return bt_export_acl_get_state_by_address(remote);
}

bt_acl_state_t bes_bt_me_acl_get_state_by_handle(uint16_t conn_handle)
{
    return bt_export_acl_get_state_by_handle(conn_handle);
}

bt_status_t bes_bt_me_acl_force_disconnect(uint16_t conn_handle, uint8_t reason, bool force_disc)
{
    return btif_me_force_disconnect_link_with_reason(conn_handle, reason, force_disc);
}

bt_status_t bt_adapter_connect_acl(const bt_bdaddr_t *remote)
{
    return btif_create_acl_to_slave(remote);
}

bt_status_t bes_bt_me_acl_disconnect_req(const bt_bdaddr_t* remote)
{
    return btif_cmgr_remove_data_link(remote);
}

bt_status_t bes_bt_me_acl_set_policy(uint16_t conn_handle, btif_link_policy_t policy)
{
    btif_remote_device_t *p_remote_dev = app_bt_get_remote_dev_by_handle(conn_handle);
    return btif_me_set_link_policy(p_remote_dev, policy);
}

bt_status_t bes_bt_me_acl_switch_role(uint16_t conn_handle)
{
    return btif_me_switch_role(conn_handle);
}

bt_status_t bes_bt_me_acl_start_sniff(uint16_t conn_handle, btif_sniff_info_t *info)
{
    return btif_me_start_sniff(conn_handle, info);
}

bt_status_t bes_bt_me_acl_stop_sniff(uint16_t conn_handle)
{
    return btif_me_stop_sniff(conn_handle);
}

void bes_bt_me_response_acl_conn_req(bt_bdaddr_t *remote, bool accept)
{
    btif_me_response_acl_conn_req(remote, accept, BTIF_BEC_LIMITED_RESOURCE);
}

bt_status_t bes_bt_me_acl_accept_link_req(const bt_bdaddr_t *remote, btif_connection_role_t role)
{
    return btif_me_accept_incoming_link(remote, role);
}

bt_status_t bes_bt_me_acl_reject_link_req(const bt_bdaddr_t *remote, btif_error_code_t reason)
{
    return btif_me_reject_incoming_link(remote, reason);
}

bt_status_t bes_bt_me_acl_auth_req(uint16_t conn_handle)
{
    return btif_me_auth_req(conn_handle);
}

bt_remver_t bes_bt_me_acl_get_remote_version(uint16_t conn_handle)
{
    return bt_export_acl_get_remote_version(conn_handle);
}

uint8_t bes_bt_me_acl_get_event_type(const btif_event_t *event)
{
    return btif_me_get_callback_event_type(event);
}

uint8_t bes_bt_me_acl_get_event_errcode(const btif_event_t *event)
{
    return btif_me_get_callback_event_err_code(event);
}

uint16_t bes_bt_me_acl_get_event_handle(const btif_event_t *event)
{
    return btif_me_get_callback_event_handle(event);
}

bt_bdaddr_t *bes_bt_me_acl_get_event_address(const btif_event_t *event)
{
    return btif_me_get_callback_event_address(event);
}

uint8_t bes_bt_me_get_event_access_mode(const btif_event_t *event)
{
    return btif_me_get_callback_event_a_mode(event);
}

uint8_t bes_bt_me_get_event_encrypt_mode(const btif_event_t *event)
{
    return btif_me_get_callback_event_encrypt_mode(event);
}

bt_status_t bes_bt_me_write_automatic_flush_timeout(uint16 connhandle, uint16 flush_timeout)
{
    return btif_me_write_automatic_flush_timeout(connhandle, flush_timeout);
}

bt_bdaddr_t *bes_bt_me_get_addr_by_sco_handle(uint16_t handle)
{
#if defined(AUTO_ACCEPT_SECOND_SCO)
    return (bt_bdaddr_t *)btif_sco_get_btaddr_by_sco_handle(handle);
#endif
    return NULL;
}

void bes_bt_me_opening_reconnect(void)
{
    app_bt_profile_connect_manager_opening_reconnect();
}

void bes_bt_me_report_inquiry_result(uint8_t *inquiry_buff, bool rssi, bool extended_mode)
{
    btif_me_inquiry_result_setup(inquiry_buff, rssi, extended_mode);
}

void bes_bt_me_handle_bt_key_click(void)
{
    bt_key_handle_func_click();
}

bool bes_bt_me_get_remote_cod_by_addr(const bt_bdaddr_t *bd_ddr, uint8_t *cod)
{
    return app_bt_get_remote_cod_by_addr(bd_ddr, cod);
}

#ifdef IBRT
bool app_ibrt_remote_is_tws_device(const bt_bdaddr_t *remote);

static void bt_export_set_remote_tws_device(const bt_bdaddr_t *remote, bool tws_link)
{
    btif_me_set_conn_tws_link(btif_me_get_connhandle_by_addr(remote),tws_link);
}

static bool bt_export_is_remote_tws_device(const bt_bdaddr_t *remote)
{
    return app_ibrt_remote_is_tws_device(remote);
}
#endif

#ifdef BT_HFP_SUPPORT
bt_status_t bt_export_hfp_profile_connect(const bt_bdaddr_t *remote)
{
    app_bt_reconnect_hfp_profile((bt_bdaddr_t *)remote);
    return BT_STS_PENDING;
}

bt_status_t bt_export_hfp_profile_disconnect(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (curr_device)
    {
        app_bt_disconnect_hfp_profile(curr_device->hf_channel);
    }
    return BT_STS_PENDING;
}

bt_status_t bt_export_hfp_send_at_command(const bt_bdaddr_t *remote, const char* cmd)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (curr_device)
    {
        app_bt_hf_send_at_command(curr_device->device_id, cmd);
    }
    return BT_STS_PENDING;
}

void bes_bt_hfp_user_event_callback_register(bes_bt_hf_event_cb_t cb)
{
    btif_hfp_user_event_callback_register(cb);
}

void bes_bt_hfp_user_event_callback_deregister(void)
{
    btif_hfp_user_event_callback_deregister();
}

void bes_bt_hfp_report_user_audio_play_stop_status(void)
{
    btif_hfp_report_user_audio_play_stop_status();
}
#endif /* BT_HFP_SUPPORT */

#ifdef BT_A2DP_SUPPORT
bt_status_t bt_export_a2dp_profile_connect(const bt_bdaddr_t *remote)
{
    app_bt_reconnect_a2dp_profile((bt_bdaddr_t *)remote);
    return BT_STS_PENDING;
}

bt_status_t bt_export_a2dp_profile_disconnect(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (curr_device)
    {
        app_bt_disconnect_a2dp_profile(curr_device->a2dp_connected_stream);
    }
    return BT_STS_PENDING;
}

bt_status_t bes_bt_a2dp_open_stream(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (curr_device)
    {
        return app_bt_A2DP_OpenStream(curr_device->profile_mgr.stream, (bt_bdaddr_t *)remote->address);
    }
    return BT_STS_PENDING;
}

void bta_register_custom_allow_receive_a2dp_steam(bool (*cb)(void))
{
    app_a2dp_register_custom_allow_receive_steam(cb);
}

#endif /* BT_A2DP_SUPPORT */

#ifdef BT_AVRCP_SUPPORT
btif_avrcp_channel_t *bes_bt_get_avrcp_channel_by_addr(uint8_t* addr)
{
    return btif_get_avrcp_channel_by_addr(addr);
}

uint8_t bes_bt_avcp_get_connect_status(uint8_t* addr)
{
    btif_avrcp_channel_t *channel = btif_get_avrcp_channel_by_addr(addr);
    if (channel)
    {
        return btif_avcp_get_connect_status(channel);
    }
    return BTIF_AVRCP_STATE_DISCONNECTED;
}

bt_status_t bt_export_avrcp_profile_connect(const bt_bdaddr_t *remote)
{
    app_bt_reconnect_avrcp_profile((bt_bdaddr_t *)remote);
    return BT_STS_PENDING;
}

bt_status_t bt_export_avrcp_profile_disconnect(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (curr_device)
    {
        app_bt_disconnect_avrcp_profile(curr_device->avrcp_channel);
    }
    return BT_STS_PENDING;
}

bt_status_t bt_export_avrcp_send_play_cmd(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (curr_device)
    {
        app_bt_a2dp_send_key_request(curr_device->device_id, AVRCP_KEY_PLAY);
    }
    return BT_STS_PENDING;
}

bt_status_t bt_export_avrcp_send_pause_cmd(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (curr_device)
    {
        app_bt_a2dp_send_key_request(curr_device->device_id, AVRCP_KEY_PAUSE);
    }
    return BT_STS_PENDING;
}

bt_status_t bt_export_avrcp_send_volume_up_cmd(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (curr_device)
    {
        app_bt_a2dp_send_key_request(curr_device->device_id, AVRCP_KEY_VOLUME_UP);
    }
    return BT_STS_PENDING;
}

bt_status_t bt_export_avrcp_send_volume_down_cmd(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (curr_device)
    {
        app_bt_a2dp_send_key_request(curr_device->device_id, AVRCP_KEY_VOLUME_DOWN);
    }
    return BT_STS_PENDING;
}

bt_status_t bt_export_avrcp_send_forward_cmd(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (curr_device)
    {
        app_bt_a2dp_send_key_request(curr_device->device_id, AVRCP_KEY_FORWARD);
    }
    return BT_STS_PENDING;
}

bt_status_t bt_export_avrcp_send_backward_cmd(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (curr_device)
    {
        app_bt_a2dp_send_key_request(curr_device->device_id, AVRCP_KEY_BACKWARD);
    }
    return BT_STS_PENDING;
}

bt_status_t bt_export_avrcp_send_fast_forward_press_cmd(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (curr_device)
    {
        app_bt_a2dp_send_key_request(curr_device->device_id, AVRCP_KEY_FAST_FORWARD_START);
    }
    return BT_STS_PENDING;
}

bt_status_t bt_export_avrcp_send_fast_forward_release_cmd(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (curr_device)
    {
        app_bt_a2dp_send_key_request(curr_device->device_id, AVRCP_KEY_FAST_FORWARD_STOP);
    }
    return BT_STS_PENDING;
}

bt_status_t bt_export_avrcp_send_rewind_press_cmd(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (curr_device)
    {
        app_bt_a2dp_send_key_request(curr_device->device_id, AVRCP_KEY_REWIND_START);
    }
    return BT_STS_PENDING;
}

bt_status_t bt_export_avrcp_send_rewind_release_cmd(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (curr_device)
    {
        app_bt_a2dp_send_key_request(curr_device->device_id, AVRCP_KEY_REWIND_STOP);
    }
    return BT_STS_PENDING;
}
#endif /* BT_AVRCP_SUPPORT */

#if defined(BT_HID_DEVICE)
bt_status_t bt_export_hid_profile_connect(const bt_bdaddr_t *remote)
{
    app_bt_hid_profile_connect_v1((bt_bdaddr_t *)remote);
    return BT_STS_PENDING;
}

bt_status_t bt_export_hid_profile_disconnect(const bt_bdaddr_t *remote)
{
    app_bt_hid_profile_disconnect(remote);
    return BT_STS_PENDING;
}
#endif // BT_HID_DEVICE

bool bes_bt_export_is_sco_prompt_play_mode(void)
{
    return app_bt_manager.config.call_preempt_play_mode;
}

uint8_t bt_export_get_a2dp_codec_type(int device_id)
{
    uint8_t codec_type = BT_A2DP_CODEC_TYPE_SBC;
    struct BT_DEVICE_T * curr_device = NULL;
    curr_device = app_bt_get_device(device_id);
    if (curr_device)
    {
        codec_type = curr_device->codec_type;
    }
    return codec_type;
}

bool bt_export_ignore_ring_and_play_self(int device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    return curr_device && curr_device->ignore_ring_and_play_tone_self;
}

void bt_export_audio_set_stay_active_mode(bool keep_active)
{
    app_bt_set_keep_active_mode(keep_active, BT_ACTIVE_MODE_KEEP_USER_SYNC_VOICE_PROMPT, UPDATE_ACTIVE_MODE_FOR_ALL_LINKS);
}

static const bes_aud_bt_t g_bes_aud_bt = {
    .aud_set_stay_active_mode = bt_export_audio_set_stay_active_mode,
    .aud_get_curr_a2dp_device = app_bt_audio_get_curr_a2dp_device,
#ifdef BT_HFP_SUPPORT
    .aud_get_curr_sco_device = app_bt_audio_get_curr_sco_device,
    .aud_is_sco_prompt_play_mode = bes_bt_export_is_sco_prompt_play_mode,
#endif
    .aud_get_a2dp_codec_type = bt_export_get_a2dp_codec_type,
    .aud_switch_sco = app_bt_Me_switch_sco,
    .aud_report_hfp_speak_gain = btapp_hfp_report_speak_gain,
#if defined(BT_AVRCP_SUPPORT) || defined(BT_A2DP_SUPPORT)
    .aud_report_a2dp_speak_gain = btapp_a2dp_report_speak_gain,
#endif
    .aud_hfp_mic_need_skip_frame = btapp_hfp_mic_need_skip_frame,
    .aud_hfp_need_mute = btapp_hfp_need_mute,
    .aud_hfp_set_local_vol = hfp_volume_local_set,
    .aud_adjust_hfp_volume = app_bt_hfp_adjust_volume,
    .aud_adjust_a2dp_volume = app_bt_a2dp_adjust_volume,
    .ignore_ring_and_play_tone_self = bt_export_ignore_ring_and_play_self,
#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)
    .a2dp_lhdc_get_ext_flags = a2dp_lhdc_get_ext_flags,
    .a2dp_lhdc_config_llc_get = a2dp_lhdc_config_llc_get,
#endif
#if defined(BT_A2DP_SUPPORT)
    .a2dp_get_non_type_by_device_id = a2dp_get_non_type_by_device_id,
#endif // BT_A2DP_SUPPORT
};

const bes_aud_bt_t * const bes_aud_bt= &g_bes_aud_bt;

/**
 * A2DP
 *
 */

static void bt_export_a2dp_set_field(int device_id, BT_A2DP_FIELD_ENUM_T field, uint32_t value)
{
#ifdef BT_A2DP_SUPPORT
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (!curr_device)
    {
        return;
    }

    switch (field)
    {
        case BT_A2DP_DISC_ON_PROCESS:
            curr_device->a2dp_disc_on_process = value ? true : false;
            break;
        case BT_THIS_IS_CLOSED_BG_A2DP:
            if (value)
            {
                curr_device->this_is_closed_bg_a2dp = true;
                curr_device->close_a2dp_resume_prio = app_bt_audio_create_new_prio();
            }
            else
            {
                curr_device->this_is_closed_bg_a2dp = false;
            }
            break;
        case BT_DISC_A2DP_PROFILE_ONLY:
            curr_device->ibrt_disc_a2dp_profile_only = value ? true : false;
            break;
        case BT_A2DP_SET_CONN_FLAG:
            curr_device->a2dp_conn_flag = value ? true : false;
            break;
        case BT_A2DP_SET_STREAM_STATE:
            btif_a2dp_set_stream_state(curr_device->a2dp_connected_stream, (bt_a2dp_stream_state_t)value);
            break;
        case BT_A2DP_LAST_PAUSED_DEVICE:
            bes_bt_a2dp_set_last_paused_device(value);
            break;
        default:
            break;
    }
#endif
}

static uint32_t bt_export_a2dp_get_field(int device_id, BT_A2DP_FIELD_ENUM_T field)
{
#ifdef BT_A2DP_SUPPORT
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (!curr_device)
    {
        return 0;
    }

    switch (field)
    {
        case BT_A2DP_DISC_ON_PROCESS:
            return curr_device->a2dp_disc_on_process;
            break;
        case BT_THIS_IS_CLOSED_BG_A2DP:
            return curr_device->this_is_closed_bg_a2dp;
            break;
        case BT_DISC_A2DP_PROFILE_ONLY:
            return curr_device->ibrt_disc_a2dp_profile_only;
            break;
        case BT_A2DP_GET_CONN_FLAG:
            return curr_device->a2dp_conn_flag;
            break;
        case BT_A2DP_GET_STREAM_STATE:
            return btif_a2dp_get_stream_state(curr_device->a2dp_connected_stream);
            break;
        case BT_A2DP_LAST_PAUSED_DEVICE:
            return bes_bt_a2dp_get_last_paused_device();
            break;
        default:
            break;
    }
#endif
    return 0;
}

uint32_t bes_bt_a2dp_get_field(int device_id, BT_A2DP_FIELD_ENUM_T field)
{
    return bt_export_a2dp_get_field(device_id, field);
}

void bes_bt_a2dp_set_field(int device_id, BT_A2DP_FIELD_ENUM_T field, uint32_t value)
{
    bt_export_a2dp_set_field(device_id, field, value);
}

bt_status_t bes_bt_a2dp_set_sink_delay(int device_id, U16 delayMs)
{
#ifdef BT_A2DP_SUPPORT
    return btif_a2dp_set_sink_delay(device_id, delayMs);
#else
    return BT_STS_FAILED;
#endif
}

void bes_bt_a2dp_get_device_codec_info(uint8_t dev_num, uint8_t *codec)
{
#ifdef BT_A2DP_SUPPORT
    btif_a2dp_get_codec_info(dev_num, codec);
#endif
}

bool bes_bt_a2dp_is_initiator(const bt_bdaddr_t *remote)
{
#ifdef BT_A2DP_SUPPORT
    return btif_a2dp_is_profile_initiator(remote);
#else
    return false;
#endif
}

static const uint8_t *bt_export_a2dp_get_codec_element(int device_id)
{
#ifdef BT_A2DP_SUPPORT
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (curr_device == NULL)
    {
        return NULL;
    }
    return (uint8_t *)btif_a2dp_get_stream_codec(curr_device->btif_a2dp_stream->a2dp_stream);
#else
    return NULL;
#endif
}

const uint8_t *bes_bt_a2dp_get_codec_element(int device_id)
{
    return bt_export_a2dp_get_codec_element(device_id);
}

uint8_t bta_get_a2dp_codec_type_by_id(int device_id)
{
    return bes_aud_bt->aud_get_a2dp_codec_type(device_id);
}

uint8_t bta_get_curr_a2dp_codec_type(void)
{
    uint8_t device_id = bes_aud_bt->aud_get_curr_a2dp_device();
    return bes_aud_bt->aud_get_a2dp_codec_type(device_id);
}

uint8_t bta_get_a2dp_vender_codec_type_by_id(int device_id)
{
    return bes_aud_bt->a2dp_get_non_type_by_device_id(device_id);
}

uint8_t bta_get_curr_a2dp_vender_codec_type(void)
{
    uint8_t device_id = bes_aud_bt->aud_get_curr_a2dp_device();
    return bes_aud_bt->a2dp_get_non_type_by_device_id(device_id);
}

uint32_t bta_get_a2dp_sample_rate_by_id(int device_id)
{
    uint32_t sample_rate = A2D_SBC_IE_SAMP_FREQ_44;
    struct BT_DEVICE_T *curr_device = NULL;

    curr_device = app_bt_get_device(device_id);
    if (curr_device)
    {
        sample_rate = curr_device->sample_rate;
    }

    return sample_rate;
}

int bta_sample_rate_convert_to_sample_count(uint32_t sr)
{
    int sc = 0;
    switch (sr)
    {
        case A2D_SBC_IE_SAMP_FREQ_16:
            sc = 16000;
            break;
        case A2D_SBC_IE_SAMP_FREQ_32:
            sc = 32000;
            break;
        case A2D_SBC_IE_SAMP_FREQ_44:
            sc = 44100;
            break;
        case A2D_SBC_IE_SAMP_FREQ_48:
            sc = 48000;
            break;
        case A2D_SBC_IE_SAMP_FREQ_88D2:
            sc = 88200;
            break;
        case A2D_SBC_IE_SAMP_FREQ_96:
            sc = 96000;
            break;
        default:
            TRACE(0, "sample rate error");
    }

    return sc;
}

uint32_t bta_get_curr_a2dp_sample_rate(void)
{
    uint8_t device_id = bes_aud_bt->aud_get_curr_a2dp_device();
    return bta_get_a2dp_sample_rate_by_id(device_id);
}

uint8_t bta_get_a2dp_channel_mode_by_id(int device_id)
{
    uint32_t channel_mode = 1;
    struct BT_DEVICE_T *curr_device = NULL;

    curr_device = app_bt_get_device(device_id);
    if (curr_device)
    {
        channel_mode = curr_device->channel_mode;
    }

    return channel_mode;
}

uint8_t bta_get_curr_a2dp_channel_mode(void)
{
    uint8_t device_id = bes_aud_bt->aud_get_curr_a2dp_device();
    return bta_get_a2dp_channel_mode_by_id(device_id);
}

void bes_bt_a2dp_accept_stream_request_command(bt_bdaddr_t* remote, uint8_t transaction, uint8_t signal_id)
{
#ifdef BT_A2DP_SUPPORT
    btif_a2dp_accept_stream_request_command(remote, transaction, signal_id);
#endif
}

void bes_bt_a2dp_stream_open_mock(uint8_t device_id, bt_bdaddr_t *remote)
{
#ifdef BT_A2DP_SUPPORT
    a2dp_bt_stream_open_mock(device_id, remote);
#endif
}

#ifdef BT_A2DP_SUPPORT
static bt_a2dp_state_t bt_export_a2dp_get_state(int device_id)
{
    bt_a2dp_state_t a2dp_state = {{{0}}, 0};
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (curr_device)
    {
        a2dp_state.remote = curr_device->remote;
        a2dp_state.a2dp_is_connected = curr_device->a2dp_conn_flag;
        a2dp_state.a2dp_is_streaming = curr_device->a2dp_streamming;
        a2dp_state.a2dp_channel_num = curr_device->a2dp_channel_num;
        a2dp_state.a2dp_channel_mode = curr_device->channel_mode;
        a2dp_state.a2dp_codec_type = curr_device->codec_type;
        a2dp_state.a2dp_sample_rate = curr_device->sample_rate;
        a2dp_state.a2dp_sample_bit = curr_device->sample_bit;
        a2dp_state.a2dp_stream_state = btif_a2dp_get_stream_state(curr_device->a2dp_connected_stream);
        a2dp_state.delay_report_enabled = btif_a2dp_is_stream_device_has_delay_reporting(curr_device->a2dp_connected_stream);
    }
    return a2dp_state;
}

bt_status_t bes_bt_a2dp_profile_connect(const bt_bdaddr_t *remote)
{
    return bt_export_a2dp_profile_connect(remote);
}

bt_status_t bes_bt_a2dp_profile_disconnect(const bt_bdaddr_t *remote)
{
    return bt_export_a2dp_profile_disconnect(remote);
}

bt_a2dp_state_t bes_bt_a2dp_get_state(int device_id)
{
    return bt_export_a2dp_get_state(device_id);
}

void bes_bt_a2dp_abs_volume_mix_version_handled(int device_id)
{
    app_bt_a2dp_abs_volume_mix_version_handled(device_id);
}

bool bes_bt_a2dp_has_device_streaming(void)
{
    return a2dp_is_music_ongoing();
}

void bes_bt_a2dp_send_set_abs_volume(uint8_t device_id, uint8_t volume)
{
    app_bt_a2dp_send_set_abs_volume(device_id, volume);
}

void bes_bt_a2dp_send_key_request(uint8_t device_id, uint8_t a2dp_key)
{
    app_bt_a2dp_send_key_request(device_id, a2dp_key);
}

void bes_bt_audio_trigger_a2dp(void)
{
    app_bt_audio_a2dp_switch_trigger();
}

#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)
bool bes_bt_a2dp_lhdc_get_ext_flags(uint32_t flags)
{
    return a2dp_lhdc_get_ext_flags(flags);
}
#endif

void bes_bt_a2dp_set_current_abs_volume(int device_id, uint8_t volume)
{
    app_bt_set_a2dp_current_abs_volume(device_id, volume);
}

void bes_bt_a2dp_set_local_vol(int device_id, uint8_t local_volume)
{
    a2dp_volume_set_local_vol(device_id, local_volume);
}

void bes_bt_a2dp_set_volume(int device_id, uint8_t volume)
{
    a2dp_volume_set(device_id, volume);
}

uint8_t bes_bt_a2dp_get_local_vol(int device_id)
{
    return a2dp_volume_local_get(device_id);
}

uint8_t bes_bt_a2dp_get_bt_vol(int device_id)
{
    return a2dp_volume_get(device_id);
}

uint8_t bes_bt_a2dp_get_abs_vol(int device_id)
{
    return a2dp_abs_volume_get(device_id);
}

bt_status_t bt_a2dp_init(bt_a2dp_callback_t callback)
{
    if (callback)
    {
        bt_add_event_callback((bt_event_callback_t)callback, BT_EVENT_MASK_A2DP_SNK_GROUP);
    }
    return BT_STS_SUCCESS;
}

bt_status_t bt_a2dp_cleanup(void)
{
    return BT_STS_SUCCESS;
}

bt_status_t bt_a2dp_connect(const bt_bdaddr_t *bd_addr)
{
    return bt_export_a2dp_profile_connect(bd_addr);
}

bt_status_t bt_a2dp_disconnect(const bt_bdaddr_t *bd_addr)
{
    return bt_export_a2dp_profile_disconnect(bd_addr);
}

bt_status_t bt_a2dp_accept_custom_cmd(const bt_bdaddr_t *bd_addr, const bt_a2dp_custom_cmd_req_param_t *cmd, bool accept)
{
    bt_a2dp_signal_msg_header_t header;
    header.message_type = accept ? 2 : 3; // accept or reject
    header.packet_type = 0; // single packet
    header.transaction = cmd->trans_lable;
    header.signal_id = cmd->cmd_id;
    header.reserve = 0;
    return btif_a2dp_send_signal_message(bd_addr, &header, NULL, 0);
}

bt_status_t bt_a2dp_send_custom_cmd(const bt_bdaddr_t *bd_addr, uint8_t custom_cmd_id, const uint8_t *data, uint16_t len)
{
    bt_a2dp_signal_msg_header_t header;
    header.message_type = 0; // command
    header.packet_type = 0; // single packet
    header.transaction = 0;
    header.signal_id = custom_cmd_id;
    header.reserve = 0;
    return btif_a2dp_send_signal_message(bd_addr, &header, data, len);
}

#if defined(BT_SOURCE)

bt_status_t bt_a2dp_source_init(bt_a2dp_source_callback_t callback)
{
    if (callback)
    {
        bt_add_event_callback((bt_event_callback_t)callback, BT_EVENT_MASK_A2DP_SRC_GROUP);
    }
    return BT_STS_SUCCESS;
}

bt_status_t bt_a2dp_source_cleanup(void)
{
    return BT_STS_SUCCESS;
}

bt_status_t bt_a2dp_source_connect(const bt_bdaddr_t *bd_addr)
{
    bt_source_reconnect_a2dp_profile(bd_addr);
    return BT_STS_SUCCESS;
}

bt_status_t bt_a2dp_source_disconnect(const bt_bdaddr_t *bd_addr)
{
    return bt_export_a2dp_profile_disconnect(bd_addr);
}

bt_status_t bt_a2dp_source_start_stream(const bt_bdaddr_t *bd_addr)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    app_a2dp_source_start_stream(curr_device->device_id);
    return BT_STS_SUCCESS;
}

bt_status_t bt_a2dp_source_suspend_stream(const bt_bdaddr_t *bd_addr)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    app_a2dp_source_suspend_stream(curr_device->device_id);
    return BT_STS_SUCCESS;
}

#endif /* BT_SOURCE */
#else /* BT_A2DP_SUPPORT */

bt_a2dp_state_t bes_bt_a2dp_get_state(int device_id)
{
    bt_a2dp_state_t a2dp_state = {{{0}}, 0};
    return a2dp_state;
}

bool bes_bt_a2dp_has_device_streaming(void)
{
    return false;
}

#endif /* BT_A2DP_SUPPORT */

/**
 * AVRCP
 *
 */

#ifdef BT_AVRCP_SUPPORT

static bt_avrcp_state_t bt_export_avrcp_get_state(int device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    bt_avrcp_state_t avrcp_state = {0};
    if (curr_device)
    {
        avrcp_state.avrcp_is_connected = curr_device->avrcp_conn_flag;
        avrcp_state.avrcp_playback_status = curr_device->avrcp_playback_status;
        avrcp_state.remote_support_playback_status_change_event = curr_device->avrcp_remote_support_playback_status_change_event;
        avrcp_state.avrcp_play_pause_flag = curr_device->a2dp_play_pause_flag;
        avrcp_state.curr_abs_volume = curr_device->a2dp_current_abs_volume;
    }
    return avrcp_state;
}

bool bt_export_avrcp_channel_is_open(int device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (curr_device == NULL)
    {
        return false;
    }
    return btif_avrcp_is_control_channel_connected(curr_device->avrcp_channel);
}

bt_status_t bes_bt_avrcp_profile_connect(const bt_bdaddr_t *remote)
{
    return bt_export_avrcp_profile_connect(remote);
}

bt_status_t bes_bt_avrcp_profile_disconnect(const bt_bdaddr_t *remote)
{
    return bt_export_avrcp_profile_disconnect(remote);
}

bt_status_t bes_bt_avrcp_send_play_cmd(const bt_bdaddr_t *remote)
{
    return bt_export_avrcp_send_play_cmd(remote);
}

bt_status_t bes_bt_avrcp_send_pause_cmd(const bt_bdaddr_t *remote)
{
    return bt_export_avrcp_send_pause_cmd(remote);
}

bt_status_t bes_bt_avrcp_send_volume_up_cmd(const bt_bdaddr_t *remote)
{
    return bt_export_avrcp_send_volume_up_cmd(remote);
}

bt_status_t bes_bt_avrcp_send_volume_down_cmd(const bt_bdaddr_t *remote)
{
    return bt_export_avrcp_send_volume_down_cmd(remote);
}

bt_status_t bes_bt_avrcp_send_forward_cmd(const bt_bdaddr_t *remote)
{
    return bt_export_avrcp_send_forward_cmd(remote);
}

bt_status_t bes_bt_avrcp_send_backward_cmd(const bt_bdaddr_t *remote)
{
    return bt_export_avrcp_send_backward_cmd(remote);
}

bt_status_t bes_bt_avrcp_send_fast_forward_press_cmd(const bt_bdaddr_t *remote)
{
    return bt_export_avrcp_send_fast_forward_press_cmd(remote);
}

bt_status_t bes_bt_avrcp_send_fast_forward_release_cmd(const bt_bdaddr_t *remote)
{
    return bt_export_avrcp_send_fast_forward_release_cmd(remote);
}

bt_status_t bes_bt_avrcp_send_rewind_press_cmd(const bt_bdaddr_t *remote)
{
    return bt_export_avrcp_send_rewind_press_cmd(remote);
}

bt_status_t bes_bt_avrcp_send_rewind_release_cmd(const bt_bdaddr_t *remote)
{
    return bt_export_avrcp_send_rewind_release_cmd(remote);
}
bt_status_t bes_bt_avrcp_key_operation(const bt_bdaddr_t *remote, bes_avrcp_panel_operation_t key, bool is_press)
{
    btif_avrcp_channel_t *channel = btif_get_avrcp_channel_by_addr((uint8_t *)remote->address);

    if (!channel)
    {
        return BT_STS_FAILED;
    }

    if (key == BES_BT_AVRCP_POP_RESERVED)
    {
        return BT_STS_FAILED;
    }

    return btif_avrcp_set_panel_key(channel, key, is_press);
}

bt_status_t bes_bt_avrcp_ct_get_capabilities(const bt_bdaddr_t *remote, bes_bt_avrcp_capabilityId capabilityId)
{
    btif_avrcp_channel_t *channel = btif_get_avrcp_channel_by_addr((uint8_t *)remote->address);

    if (!channel)
    {
        return BT_STS_FAILED;
    }

    return btif_avrcp_ct_get_capabilities(channel, (btif_avrcp_capabilityId)capabilityId);
}

bt_status_t bta_avrcp_send_get_media_status(const bt_bdaddr_t *bd_addr, uint32_t attr_masks)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    return btif_avrcp_ct_get_media_Info(curr_device->avrcp_channel, attr_masks == 0 ? 0xff : attr_masks);
}

bt_status_t bta_avrcp_set_addressed_player(const bt_bdaddr_t *remote, uint16_t player_id)
{
    return btif_avrcp_ct_set_addressed_player(remote, player_id);
}

bt_status_t bta_avrcp_set_browsed_player(const bt_bdaddr_t *remote, uint16_t player_id)
{
    return btif_avrcp_ct_set_browsed_player(remote, player_id);
}

bt_status_t bta_avrcp_ct_get_folder_items(const bt_bdaddr_t *remote, void *param)
{
    return btif_avrcp_ct_get_folder_items(remote, (bt_avrcp_get_folder_items_t *)param);
}

bt_status_t bta_avrcp_ct_send_change_patch(const bt_bdaddr_t *remote, void *param)
{
    return btif_avrcp_ct_send_change_patch(remote, (bt_avrcp_change_patch_t *)param);
}

bt_status_t bta_avrcp_ct_send_get_item_attributes(const bt_bdaddr_t *remote, void *param)
{
    return btif_avrcp_ct_send_get_item_attributes(remote, (bt_avrcp_get_item_att_t *)param);
}

bt_status_t bta_avrcp_ct_send_search(const bt_bdaddr_t *remote, const char * string, uint16_t string_len)
{
    return btif_avrcp_ct_send_search(remote, string, string_len);
}

bt_status_t bta_avrcp_ct_get_total_number_of_items(const bt_bdaddr_t *remote, uint8_t scope)
{
    return btif_avrcp_ct_get_total_number_of_items(remote, scope);
}

bt_status_t bta_avrcp_send_cover_art_get_image_properties(const bt_bdaddr_t *remote, const char *image_handle)
{
    return btif_avrcp_get_image_properties(remote, image_handle);
}

bt_status_t bta_avrcp_send_cover_art_get_image(const bt_bdaddr_t *remote, const char *image_handle, const char *descriptor, uint16_t descriptor_len)
{
    return btif_avrcp_send_obex_get_image_by_art_handle(remote, image_handle, descriptor, descriptor_len);
}

bt_status_t bta_avrcp_send_cover_art_get_linked_thumbnail(const bt_bdaddr_t *remote, const char *image_handle)
{
    return btif_avrcp_send_obex_get_link_thumbnail_by_art_handle(remote, image_handle);
}

bt_status_t bes_bt_avrcp_ct_register_play_pos_notification(const bt_bdaddr_t *remote, uint32_t interval)
{
    btif_avrcp_channel_t *channel = btif_get_avrcp_channel_by_addr((uint8_t *)remote->address);

    if (!channel)
    {
        return BT_STS_FAILED;
    }
    return btif_avrcp_ct_register_play_pos_notification(channel, interval);
}

bt_status_t bes_bt_avrcp_ct_register_media_status_notification(const bt_bdaddr_t *remote, uint32_t interval)
{
    btif_avrcp_channel_t *channel = btif_get_avrcp_channel_by_addr((uint8_t *)remote->address);

    if (!channel)
    {
        return BT_STS_FAILED;
    }
    return btif_avrcp_ct_register_media_status_notification(channel, interval);
}

bt_status_t bes_bt_avrcp_ct_register_track_change_notification(const bt_bdaddr_t *remote, uint32_t interval)
{
    btif_avrcp_channel_t *channel = btif_get_avrcp_channel_by_addr((uint8_t *)remote->address);

    if (!channel)
    {
        return BT_STS_FAILED;
    }
    return btif_avrcp_ct_register_track_change_notification(channel, interval);
}

bt_status_t bes_bt_avrcp_ct_register_volume_change_notification(const bt_bdaddr_t *remote, uint32_t interval)
{
    btif_avrcp_channel_t *channel = btif_get_avrcp_channel_by_addr((uint8_t *)remote->address);

    if (!channel)
    {
        return BT_STS_FAILED;
    }
    return btif_avrcp_ct_register_volume_change_notification(channel, interval);
}

bt_avrcp_state_t bes_bt_avrcp_get_state(int device_id)
{
    return bt_export_avrcp_get_state(device_id);
}

uint8_t bes_bt_avrcp_cb_get_event(const avrcp_callback_parms_t * parms)
{
    return btif_avrcp_get_callback_event(parms);
}

bt_status_t bes_bt_avrcp_cb_get_channel_state(const avrcp_callback_parms_t * parms)
{
    return btif_get_avrcp_cb_channel_state(parms);
}

uint8_t bes_bt_avrcp_cb_get_adv_op(const avrcp_callback_parms_t * parms)
{
    return btif_get_avrcp_cb_channel_advOp(parms);
}

avrcp_adv_notify_parms_t *bes_bt_avrcp_cb_get_adv_notify(const avrcp_callback_parms_t * parms)
{
    return btif_get_avrcp_adv_notify(parms);
}

avctp_cmd_frame_t *bes_bt_avrcp_cb_get_cmd_frame(const avrcp_callback_parms_t * parms)
{
    return btif_get_avrcp_cmd_frame(parms);
}

struct avrcp_remote_sdp_info bes_bt_avrcp_get_remote_sdp_info(btif_avrcp_channel_t * channel, bool is_target)
{
    return btif_avrcp_get_remote_sdp_info(channel, is_target);
}

bool bes_bt_avrcp_is_initiator(const bt_bdaddr_t* remote)
{
    return btif_avrcp_is_profile_initiator(remote);
}

void bes_bt_avrcp_ct_register_notification_event(uint8_t device_id, uint8_t event)
{
    btif_avrcp_ct_register_notification_event(device_id, event);
}

bool bes_bt_avrcp_is_remote_ct_info_got(const bt_bdaddr_t *remote)
{
    return btif_avrcp_is_remote_ct_info_got(remote);
}

uint8_t bes_bt_avrcp_get_volume_change_trans_id(uint8_t device_id)
{
    return app_bt_avrcp_get_volume_change_trans_id(device_id);
}

void bes_bt_avrcp_set_volume_change_trans_id(uint8_t device_id, uint8_t trans_id)
{
    app_bt_avrcp_set_volume_change_trans_id(device_id, trans_id);
}

uint8_t bes_bt_avrcp_get_ctl_trans_id(uint8_t device_id)
{
    return app_bt_avrcp_get_ctl_trans_id(device_id);
}

void bes_bt_avrcp_set_ctl_trans_id(uint8_t device_id, uint8_t trans_id)
{
    app_bt_avrcp_set_ctl_trans_id(device_id, trans_id);
}

bt_bdaddr_t *bes_bt_avrcp_get_remote_device_addr(btif_avrcp_channel_t* handle)
{
    return btif_avrcp_get_remote_device_addr(handle);
}

bt_status_t bt_avrcp_init(bt_avrcp_callback_t callback)
{
    if (callback)
    {
        bt_add_event_callback((bt_event_callback_t)callback, BT_EVENT_MASK_AVRCP_GROUP);
    }
    return BT_STS_SUCCESS;
}

bt_status_t bt_avrcp_cleanup(void)
{
    return BT_STS_SUCCESS;
}

bt_status_t bt_avrcp_connect(const bt_bdaddr_t *bd_addr)
{
    return bt_export_avrcp_profile_connect(bd_addr);
}

bt_status_t bt_avrcp_disconnect(const bt_bdaddr_t *bd_addr)
{
    return bt_export_avrcp_profile_disconnect(bd_addr);
}

bt_status_t bt_avrcp_send_passthrough_cmd(const bt_bdaddr_t *bd_addr, bt_avrcp_key_code_t key)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    app_bt_a2dp_send_key_request(curr_device->device_id, key);
    return BT_STS_SUCCESS;
}

bt_status_t bt_avrcp_send_get_play_status(const bt_bdaddr_t *bd_addr)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    return btif_avrcp_ct_get_play_status(curr_device->avrcp_channel);
}

bt_status_t bt_avrcp_send_set_abs_volume(const bt_bdaddr_t *bd_addr, uint8_t volume)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    return btif_avrcp_ct_set_absolute_volume(curr_device->avrcp_channel, volume);
}

bt_status_t bt_avrcp_send_get_play_status_rsp(const bt_bdaddr_t *bd_addr, bt_avrcp_play_status_t play_status, uint32_t song_len, uint32_t song_pos)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    return btif_avrcp_send_play_status_rsp(curr_device->avrcp_channel, song_len, song_pos, play_status);
}

bt_status_t bt_avrcp_send_volume_notify_rsp(const bt_bdaddr_t *bd_addr, uint8_t volume)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device || !curr_device->avrcp_conn_flag)
    {
        return BT_STS_FAILED;
    }
    if (curr_device->volume_report == BTIF_AVCTP_RESPONSE_INTERIM)
    {
        bt_status_t status = btif_avrcp_tg_send_volume_change_actual_rsp(curr_device->avrcp_channel, volume);
        if (BT_STS_FAILED != status)
        {
            curr_device->volume_report = BTIF_AVCTP_RESPONSE_CHANGED;
        }
        return status;
    }
    return BT_STS_FAILED;
}

bt_status_t bt_avrcp_report_status_change(const bt_bdaddr_t *bd_addr, bt_avrcp_status_change_event_t event_id, uint32_t param)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    if (event_id == BT_AVRCP_VOLUME_CHANGED)
    {
        return bt_avrcp_send_volume_notify_rsp(bd_addr, (uint8_t)param);
    }
    if (event_id == BT_AVRCP_PLAY_STATUS_CHANGED)
    {
        bt_avrcp_play_status_t play_status = (bt_avrcp_play_status_t)param;
        bt_avrcp_play_status_t prev_play_status = (bt_avrcp_play_status_t)curr_device->avrcp_playback_status;
        if (curr_device->avrcp_conn_flag && play_status <= BT_AVRCP_PLAY_STATUS_PAUSED)
        {
            curr_device->avrcp_playback_status = (uint8_t)play_status;
            if (curr_device->play_status_notify_registered && play_status != prev_play_status)
            {
                return btif_avrcp_send_play_status_change_actual_rsp(curr_device->avrcp_channel, (uint8_t)play_status);
            }
        }
        return BT_STS_FAILED;
    }
    TRACE(0, "bt_avrcp_send_notify_rsp: unsupported event %d", event_id);
    return BT_STS_FAILED;
}

bt_status_t bes_bt_avrcp_send_volume_notify_rsp(const bt_bdaddr_t *bd_addr, uint8_t volume)
{
    return bt_avrcp_send_volume_notify_rsp(bd_addr, volume);
}
#endif /* BT_AVRCP_SUPPORT */


/**
 * HFP
 *
 */


int bta_hfp_battery_report(uint8_t level)
{
#ifdef BT_HFP_SUPPORT
    return app_hfp_battery_report(level);
#else
    return -1;
#endif
}

int bta_hfp_enable_siri_voice(bool en)
{
#ifdef BT_HFP_SUPPORT
    return app_hfp_siri_voice(en);
#else
    return -1;
#endif
}

#if !defined(BT_HFP_SUPPORT)

uint8_t bes_bt_hfp_has_service_connected(void)
{
    return false;
}

bool bes_bt_hfp_has_sco_connected(void)
{
    return false;
}

bool bes_bt_hfp_has_call_active(void)
{
    return false;
}

uint8_t bes_bt_hfp_has_call_setup(void)
{
    return false;
}

bool bes_bt_hfp_curr_voice_rec_active(void)
{
    return false;
}

#else /* BT_HFP_SUPPORT */

static bt_status_t bt_export_hfp_create_audio_link(int device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (curr_device == NULL)
    {
        return BT_STS_FAILED;
    }
    return btif_hf_create_audio_link(curr_device->hf_channel);
}

static bt_status_t bt_export_hfp_disc_audio_link(int device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (curr_device == NULL)
    {
        return BT_STS_FAILED;
    }
    return btif_hf_disc_audio_link(curr_device->hf_channel);
}


static bt_hfp_state_t bt_export_hfp_get_state(int device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    bt_hfp_state_t hfp_state = {{{0}}, 0};
    if (curr_device)
    {
        struct hshf_control *hf_ctl = _hshf_get_control_from_id(device_id);
        hfp_state.remote = curr_device->remote;
        hfp_state.hfp_is_connected = curr_device->hf_conn_flag;
        hfp_state.sco_is_connected = curr_device->hf_audio_state;
        hfp_state.hfp_nego_codec = hf_ctl->negotiated_codec;
        hfp_state.call_is_active = curr_device->hfchan_call;
        hfp_state.hfp_chan_state = btif_get_hf_chan_state(curr_device->hf_channel);
        hfp_state.callsetup_state = curr_device->hfchan_callSetup;
        hfp_state.callheld_state = curr_device->hf_callheld;
        hfp_state.service_status = btif_hf_get_cind_service_value(curr_device->hf_channel).value;
        hfp_state.signal_status = btif_hf_get_cind_signal_value(curr_device->hf_channel).value;
        hfp_state.roam_status = btif_hf_get_cind_roam_value(curr_device->hf_channel).value;
        hfp_state.battery_status = btif_hf_get_cind_battchg_value(curr_device->hf_channel).value;
        hfp_state.spk_volume = hf_ctl->speak_volume;
        hfp_state.mic_volume = hf_ctl->mic_gain;
        hfp_state.in_band_ring_enable = hf_ctl->bsir_enable;
        hfp_state.voice_rec_state = hf_ctl->voice_rec;
    }
    return hfp_state;
}

static bt_status_t bt_export_hfp_call_action(int device_id, BT_HFP_CALL_ACTION_T action)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    bt_status_t status = BT_STS_FAILED;
    if (curr_device == NULL)
    {
        return status;
    }
    switch (action)
    {
        case BT_HFP_ANSWER_CALL:
            status = btif_hf_answer_call(curr_device->hf_channel);
            break;
        case BT_HFP_HANGUP_CALL:
            status = btif_hf_hang_up_call(curr_device->hf_channel);
            break;
        case BT_HFP_REDIAL_CALL:
            status = btif_hf_redial_call(curr_device->hf_channel);
            break;
        default:
            break;
    }
    return status;
}

static bt_status_t bt_export_hfp_dial_number(int device_id, const uint8_t *number, uint16_t len)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (curr_device == NULL)
    {
        return BT_STS_FAILED;
    }
    return btif_hf_dial_number(curr_device->hf_channel, (uint8_t *)number, len);
}

static bt_status_t bt_export_hfp_call_hold(int device_id, btif_hf_hold_call_t action, uint8_t index)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (curr_device == NULL)
    {
        return BT_STS_FAILED;
    }
    return btif_hf_call_hold(curr_device->hf_channel, action, index);
}

static bool bt_export_hfp_curr_voice_rec_active(void)
{
    return btif_hf_is_voice_rec_active(app_bt_get_device(app_bt_audio_get_curr_hfp_device())->hf_channel);
}

bt_status_t bes_bt_hfp_profile_connect(const bt_bdaddr_t *remote)
{
    return bt_export_hfp_profile_connect(remote);
}

bt_status_t bes_bt_hfp_profile_disconnect(const bt_bdaddr_t *remote)
{
    return bt_export_hfp_profile_disconnect(remote);
}

bt_status_t bes_bt_hfp_create_audio_link(int device_id)
{
    return bt_export_hfp_create_audio_link(device_id);
}

bt_status_t bes_bt_hfp_disc_audio_link(int device_id)
{
    return bt_export_hfp_disc_audio_link(device_id);
}

bt_hfp_state_t bes_bt_hfp_get_state(int device_id)
{
    return bt_export_hfp_get_state(device_id);
}

bt_status_t bes_bt_hfp_call_action(int device_id, BT_HFP_CALL_ACTION_T action)
{
    return bt_export_hfp_call_action(device_id, action);
}

bt_status_t bes_bt_hfp_dial_number(int device_id, const uint8_t *number, uint16_t len)
{
    return bt_export_hfp_dial_number(device_id, number, len);
}

bt_status_t bes_bt_hfp_call_hold(int device_id, btif_hf_hold_call_t action, uint8_t index)
{
    return bt_export_hfp_call_hold(device_id, action, index);
}

bool bes_bt_hfp_is_connecting(const bt_bdaddr_t *remote)
{
    return btif_hfp_profile_connecting(remote);
}

bool bes_bt_hfp_is_initiator(const bt_bdaddr_t *remote)
{
    return btif_hfp_is_profile_initiator(remote);
}

uint8_t bes_bt_hfp_has_service_connected(void)
{
    return app_bt_audio_count_connected_hfp();
}

uint8_t bes_bt_get_curr_playing_sco(void)
{
    return app_bt_audio_get_curr_playing_sco();
}

bool bes_bt_hfp_has_sco_connected(void)
{
    return btapp_hfp_is_sco_active();
}

uint32_t bes_bt_switch_hold_background_sco(uint32_t btclk)
{
    return app_bt_audio_trigger_switch_mute_streaming_sco(btclk);
}

bool bes_bt_hfp_has_call_active(void)
{
    return btapp_hfp_is_call_active();
}

uint8_t bes_bt_hfp_has_call_setup(void)
{
    return btapp_hfp_get_call_setup();
}

void bes_bt_hfp_send_at_command(uint8_t device_id, const char* cmd)
{
    app_bt_hf_send_at_command(device_id, cmd);
}

void bes_bt_hfp_set_battery_level(uint8_t level)
{
    app_hfp_set_battery_level(level);
}

void bes_bt_hfp_send_call_hold_request(uint8_t device_id, btif_hf_hold_call_t action)
{
    app_hfp_send_call_hold_request(device_id, action);
}

void bes_bt_hfp_set_local_vol(int device_id, uint8_t local_volume)
{
    hfp_update_local_volume(device_id, local_volume);
}

uint8_t bes_bt_hfp_get_local_vol(int device_id)
{
    return hfp_volume_local_get(device_id);
}

uint8_t bes_bt_hfp_get_bt_vol(int device_id)
{
    return hfp_volume_get(device_id);
}

bool bes_bt_hfp_curr_voice_rec_active(void)
{
    return bt_export_hfp_curr_voice_rec_active();
}

bt_status_t bt_hf_init(bt_hf_callback_t callback)
{
    if (callback)
    {
        bt_add_event_callback((bt_event_callback_t)callback, BT_EVENT_MASK_HFP_HF_GROUP);
    }
    return BT_STS_SUCCESS;
}

bt_status_t bt_hf_cleanup(void)
{
    return BT_STS_SUCCESS;
}

bt_status_t bt_hf_connect(const bt_bdaddr_t *bd_addr)
{
    return bt_export_hfp_profile_connect(bd_addr);
}

bt_status_t bt_hf_disconnect(const bt_bdaddr_t *bd_addr)
{
    return bt_export_hfp_profile_disconnect(bd_addr);
}

bt_status_t bt_hf_connect_audio(const bt_bdaddr_t *bd_addr)
{
    uint8_t device_id = app_bt_get_connected_device_id_byaddr(bd_addr);
    return bt_export_hfp_create_audio_link(device_id);
}

bt_status_t bt_hf_disconnect_audio(const bt_bdaddr_t *bd_addr)
{
    uint8_t device_id = app_bt_get_connected_device_id_byaddr(bd_addr);
    return bt_export_hfp_disc_audio_link(device_id);
}

bt_status_t bt_hf_start_voice_recognition(const bt_bdaddr_t *bd_addr)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (curr_device)
    {
        status = btif_hf_enable_voice_recognition(curr_device->hf_channel, true);
    }
    return status;
}

bt_status_t bt_hf_stop_voice_recognition(const bt_bdaddr_t *bd_addr)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (curr_device)
    {
        status = btif_hf_enable_voice_recognition(curr_device->hf_channel, false);
    }
    return status;
}

bt_status_t bt_hf_volume_control(const bt_bdaddr_t *bd_addr, bt_hf_volume_type_t type, int volume)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (curr_device)
    {
        if (type == BT_HF_VOLUME_TYPE_SPK)
        {
            btif_hf_report_speaker_volume(curr_device->hf_channel, (uint8_t)volume);
        }
        else
        {
            btif_hf_report_mic_volume(curr_device->hf_channel, (uint8_t)volume);
        }

        status = btif_hf_enable_voice_recognition(curr_device->hf_channel, false);
    }
    return status;
}

bt_status_t bt_hf_dial(const bt_bdaddr_t *bd_addr, const char *number)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (curr_device)
    {
        if (number == NULL)
        {
            status = btif_hf_redial_call(curr_device->hf_channel);
        }
        else
        {
            status = btif_hf_dial_number(curr_device->hf_channel, (uint8_t *)number, number ? strlen(number) : 0);
        }
    }
    return status;
}

bt_status_t bt_hf_dial_memory(const bt_bdaddr_t *bd_addr, int location)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (curr_device)
    {
        status = btif_hf_dial_memory(curr_device->hf_channel, location);
    }
    return status;
}

bt_status_t bt_hf_handle_call_action(const bt_bdaddr_t *bd_addr, bt_hf_call_action_t action, int idx)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return status;
    }
    if (action == BT_HF_CALL_ACTION_ATA)
    {
        return btif_hf_answer_call(curr_device->hf_channel);
    }
    if (action == BT_HF_CALL_ACTION_CHUP)
    {
        return btif_hf_hang_up_call(curr_device->hf_channel);
    }
    if (action == BT_HF_CALL_ACTION_REDIAL)
    {
        return btif_hf_redial_call(curr_device->hf_channel);
    }
    if (action >= BT_HF_CALL_ACTION_CHLD_0 && action <= BT_HF_CALL_ACTION_CHLD_4)
    {
        return btif_hf_call_hold(curr_device->hf_channel, (btif_hf_hold_call_t)(action - BT_HF_CALL_ACTION_CHLD_0), 0);
    }
    if (action == BT_HF_CALL_ACTION_CHLD_1x || action == BT_HF_CALL_ACTION_CHLD_2x)
    {
        return btif_hf_call_hold(curr_device->hf_channel,
            action == BT_HF_CALL_ACTION_CHLD_1x ? BTIF_HF_HOLD_RELEASE_ACTIVE_CALLS : BTIF_HF_HOLD_HOLD_ACTIVE_CALLS,
            idx);
    }
    if (action >= BT_HF_CALL_ACTION_BTRH_0 && action <= BT_HF_CALL_ACTION_BTRH_2)
    {
        char at_cmd[] = {'A', 'T', '+', 'B', 'T', 'R', 'H', '=', '0' + (action - BT_HF_CALL_ACTION_BTRH_0), '\r', 0};
        return bt_hf_send_at_cmd(bd_addr, at_cmd);
    }
    return BT_STS_FAILED;
}

bt_status_t bt_hf_query_current_calls(const bt_bdaddr_t *bd_addr)
{
    return bt_hf_send_at_cmd(bd_addr, "AT+CLCC\r");
}

bt_status_t bt_hf_query_current_operator_name(const bt_bdaddr_t *bd_addr)
{
    return bt_hf_send_at_cmd(bd_addr, "AT+COPS?\r");
}

bt_status_t bt_hf_retrieve_subscriber_info(const bt_bdaddr_t *bd_addr)
{
    return bt_hf_send_at_cmd(bd_addr, "AT+CNUM\r");
}

bt_status_t bt_hf_send_dtmf(const bt_bdaddr_t *bd_addr, char code)
{
    char at_cmd[] = {'A', 'T', '+', 'V', 'T', 'S', '=', code, '\r', 0};
    return bt_hf_send_at_cmd(bd_addr, at_cmd);
}

bt_status_t bt_hf_request_last_voice_tag_number(const bt_bdaddr_t *bd_addr)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (curr_device)
    {
        status = btif_hf_attach_voice_tag(curr_device->hf_channel);
    }
    return status;
}

bt_status_t bt_hf_send_at_cmd(const bt_bdaddr_t *bd_addr, const char *at_cmd_str)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (curr_device)
    {
        status = btif_hf_send_at_cmd(curr_device->hf_channel, at_cmd_str);
        status = BT_STS_SUCCESS;
    }
    return status;
}

bt_hfp_state_t bt_hf_get_state(int device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    bt_hfp_state_t hfp_state = {{{0}}, 0};
    if (curr_device)
    {
        struct hshf_control *hf_ctl = _hshf_get_control_from_id(device_id);
        hfp_state.remote = curr_device->remote;
        hfp_state.hfp_is_connected = curr_device->hf_conn_flag;
        hfp_state.sco_is_connected = curr_device->hf_audio_state;
        hfp_state.hfp_nego_codec = hf_ctl->negotiated_codec;
        hfp_state.call_is_active = curr_device->hfchan_call;
        hfp_state.hfp_chan_state = btif_get_hf_chan_state(curr_device->hf_channel);
        hfp_state.callsetup_state = curr_device->hfchan_callSetup;
        hfp_state.callheld_state = curr_device->hf_callheld;
        hfp_state.service_status = btif_hf_get_cind_service_value(curr_device->hf_channel).value;
        hfp_state.signal_status = btif_hf_get_cind_signal_value(curr_device->hf_channel).value;
        hfp_state.roam_status = btif_hf_get_cind_roam_value(curr_device->hf_channel).value;
        hfp_state.battery_status = btif_hf_get_cind_battchg_value(curr_device->hf_channel).value;
        hfp_state.spk_volume = hf_ctl->speak_volume;
        hfp_state.mic_volume = hf_ctl->mic_gain;
        hfp_state.in_band_ring_enable = hf_ctl->bsir_enable;
        hfp_state.voice_rec_state = hf_ctl->voice_rec;
    }
    return hfp_state;
}

#if defined(BT_HFP_AG_ROLE)

bt_status_t bt_ag_init(bt_ag_callback_t callback)
{
    if (callback)
    {
        bt_add_event_callback((bt_event_callback_t)callback, BT_EVENT_MASK_HFP_AG_GROUP);
    }
    return BT_STS_SUCCESS;
}

bt_status_t bt_ag_cleanup(void)
{
    return BT_STS_SUCCESS;
}

bt_status_t bt_ag_connect(const bt_bdaddr_t *bd_addr)
{
    bt_source_reconnect_hfp_profile(bd_addr);
    return BT_STS_SUCCESS;
}

bt_status_t bt_ag_disconnect(const bt_bdaddr_t *bd_addr)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return status;
    }
    return btif_ag_disconnect_service_link(curr_device->hf_channel);
}

bt_status_t bt_ag_connect_audio(const bt_bdaddr_t *bd_addr)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return status;
    }
    return btif_ag_create_audio_link(curr_device->hf_channel);
}

bt_status_t bt_ag_disconnect_audio(const bt_bdaddr_t *bd_addr)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return status;
    }
    return btif_ag_disc_audio_link(curr_device->hf_channel);
}

bt_status_t bt_ag_start_voice_recoginition(const bt_bdaddr_t *bd_addr)
{
    return bt_ag_send_at_result(bd_addr, "\r\nBVRA: 1\r\n");
}

bt_status_t bt_ag_stop_voice_recoginition(const bt_bdaddr_t *bd_addr)
{
    return bt_ag_send_at_result(bd_addr, "\r\nBVRA: 0\r\n");
}

bt_status_t bt_ag_volume_control(const bt_bdaddr_t *bd_addr, bt_hf_volume_type_t type, int volume)
{
    char result[32] = {0};
    snprintf(result, sizeof(result), (type == BT_HF_VOLUME_TYPE_SPK) ? "\r\n+VGS: %d\r\n" : "\r\n+VGM: %d\r\n", volume);
    return bt_ag_send_at_result(bd_addr, result);
}

bt_status_t bt_ag_set_curr_at_upper_handle(const bt_bdaddr_t *bd_addr)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    return btif_ag_set_curr_at_upper_handle(curr_device->hf_channel);
}

bt_status_t bt_ag_cops_response(const bt_bdaddr_t *bd_addr, const char *operator_name)
{
    char result[32] = {0};
    if (operator_name && operator_name[0])
    {
        snprintf(result, sizeof(result), "\r\n+COPS: 1,0,\"%s\"\r\n", operator_name);
    }
    else
    {
        snprintf(result, sizeof(result), "\r\n+COPS: 0\r\n");
    }
    return bt_ag_send_at_result(bd_addr, result);
}

bt_status_t bt_ag_clcc_response(const bt_bdaddr_t *bd_addr, const bt_ag_clcc_status_t *status)
{
    char clcc[128] = {0};
    snprintf(clcc, sizeof(clcc), "\r\n+CLCC: %d,%d,%d,%d,%d,\"%s\",%d\r\n",
        status->index, status->dir, status->state, status->mode, status->mpty, status->number, status->number_type);
    return bt_ag_send_at_result(bd_addr, clcc);
}

bt_status_t bt_ag_cind_response(const bt_bdaddr_t *bd_addr, const bt_ag_cind_status_t *status)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    char result[40] = {0};
    bt_ag_ind_status_t ag_status;
    bt_hf_call_ind_t call = BT_HF_CALL_NO_CALLS_IN_PROGRESS;
    bt_hf_callsetup_ind_t callsetup = BT_HF_CALLSETUP_NONE;
    bt_hf_callheld_ind_t callheld = BT_HF_CALLHELD_NONE;
    if (status->num_active || status->num_held)
    {
        call = BT_HF_CALL_CALLS_IN_PROGRESS;
    }
    if (status->call_state == BT_HF_CALL_STATE_INCOMING || status->call_state == BT_HF_CALL_STATE_WAITING)
    {
        callsetup = BT_HF_CALLSETUP_INCOMING;
    }
    else if (status->call_state == BT_HF_CALL_STATE_DIALING)
    {
        callsetup = BT_HF_CALLSETUP_OUTGOING;
    }
    else if (status->call_state == BT_HF_CALL_STATE_ALERTING)
    {
        callsetup = BT_HF_CALLSETUP_ALERTING;
    }
    if (status->num_active && status->num_held)
    {
        callheld = BT_HF_CALLHELD_HOLD_AND_ACTIVE;
    }
    else if (status->num_held)
    {
        callheld = BT_HF_CALLHELD_HOLD;
    }
    ag_status.service = status->service;
    ag_status.call = call;
    ag_status.callsetup = callsetup;
    ag_status.callheld = callheld;
    ag_status.signal = status->signal;
    ag_status.roam = status->roam;
    ag_status.battchg = status->battery_level;
    snprintf(result, sizeof(result), "\r\n+CIND: %d,%d,%d,%d,%d,%d,%d\r\n",
        ag_status.service, ag_status.call, ag_status.callsetup, ag_status.callheld,
        ag_status.signal, ag_status.roam, ag_status.battchg);
    btif_ag_set_ind_status(curr_device->hf_channel, &ag_status);
    return bt_ag_send_at_result(bd_addr, result);
}

bt_status_t bt_ag_device_status_change(const bt_bdaddr_t *bd_addr, const bt_ag_device_status_t *status)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    bt_ag_ind_status_t ag_status = btif_ag_get_ind_status(curr_device->hf_channel);
    if (status->service != ag_status.service)
    {
        btif_ag_send_service_status(curr_device->hf_channel, status->service);
    }
    if (status->signal != ag_status.signal)
    {
        btif_ag_send_mobile_signal_level(curr_device->hf_channel, status->signal);
    }
    if (status->roam != ag_status.roam)
    {
        btif_ag_send_mobile_roam_status(curr_device->hf_channel, status->roam);
    }
    if (status->battery_level != ag_status.battchg)
    {
        btif_ag_send_mobile_battery_level(curr_device->hf_channel, status->battery_level);
    }
    return BT_STS_SUCCESS;
}

bt_status_t bt_ag_phone_status_change(const bt_bdaddr_t *bd_addr, const bt_ag_phone_status_t *status)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    bt_ag_ind_status_t ag_status = btif_ag_get_ind_status(curr_device->hf_channel);
    bt_hf_call_ind_t call = BT_HF_CALL_NO_CALLS_IN_PROGRESS;
    bt_hf_callsetup_ind_t callsetup = BT_HF_CALLSETUP_NONE;
    bt_hf_callheld_ind_t callheld = BT_HF_CALLHELD_NONE;
    if (status->num_active || status->num_held)
    {
        call = BT_HF_CALL_CALLS_IN_PROGRESS;
    }
    if (status->call_state == BT_HF_CALL_STATE_INCOMING || status->call_state == BT_HF_CALL_STATE_WAITING)
    {
        callsetup = BT_HF_CALLSETUP_INCOMING;
    }
    else if (status->call_state == BT_HF_CALL_STATE_DIALING)
    {
        callsetup = BT_HF_CALLSETUP_OUTGOING;
    }
    else if (status->call_state == BT_HF_CALL_STATE_ALERTING)
    {
        callsetup = BT_HF_CALLSETUP_ALERTING;
    }
    if (status->num_active && status->num_held)
    {
        callheld = BT_HF_CALLHELD_HOLD_AND_ACTIVE;
    }
    else if (status->num_held)
    {
        callheld = BT_HF_CALLHELD_HOLD;
    }
    if (callsetup == BT_HF_CALLSETUP_INCOMING)
    {
        if (ag_status.call)
        {
            btif_ag_send_calling_ring(curr_device->hf_channel, status->number);
        }
        else
        {
            btif_ag_send_call_waiting_notification(curr_device->hf_channel, status->number);
        }
    }
    if (call != ag_status.call)
    {
        btif_ag_send_call_active_status(curr_device->hf_channel, call);
    }
    if (callsetup != ag_status.callsetup)
    {
        btif_ag_send_callsetup_status(curr_device->hf_channel, callsetup);
    }
    if (callheld != ag_status.callheld)
    {
        btif_ag_send_callheld_status(curr_device->hf_channel, callheld);
    }
    return BT_STS_SUCCESS;
}

bt_status_t bt_ag_send_at_result(const bt_bdaddr_t *bd_addr, const char *at_result)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    return btif_ag_send_result_code(curr_device->hf_channel, at_result, at_result ? strlen(at_result) : 0);
}

bt_status_t bt_ag_send_response_code(const bt_bdaddr_t *bd_addr, bt_hf_at_response_t code, int cme_error)
{
    const char *at_result = NULL;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    if (code == BT_HF_AT_RESPONSE_ERROR_CME)
    {
        if (btif_ag_cmee_enabled(curr_device->hf_channel))
        {
            char result[32] = {0};
            snprintf(result, sizeof(result), "\r\n+CME ERROR: %d\r\n", cme_error);
            return bt_ag_send_at_result(bd_addr, result);
        }
        code = BT_HF_AT_RESPONSE_ERROR;
    }
    switch (code)
    {
        case BT_HF_AT_RESPONSE_OK: at_result = "\r\nOK\r\n"; break;
        case BT_HF_AT_RESPONSE_ERROR: at_result = "\r\nERROR\r\n"; break;
        case BT_HF_AT_RESPONSE_ERROR_NO_CARRIER: at_result = "\r\nNO CARRIER\r\n"; break;
        case BT_HF_AT_RESPONSE_ERROR_BUSY: at_result = "\r\nBUSY\r\n"; break;
        case BT_HF_AT_RESPONSE_ERROR_NO_ANSWER: at_result = "\r\nNO ANSWER\r\n"; break;
        case BT_HF_AT_RESPONSE_ERROR_DELAYED: at_result = "\r\nDELAYED\r\n"; break;
        case BT_HF_AT_RESPONSE_ERROR_BLACKLISTED: at_result = "\r\nBLACKLISTED\r\n"; break;
        default: TRACE(0, "bt_ag_send_response_code: invalid code %d", code); break;
    }
    if (at_result == NULL)
    {
        return BT_STS_FAILED;
    }
    return bt_ag_send_at_result(bd_addr, at_result);
}

bt_status_t bt_ag_set_sco_allowed(const bt_bdaddr_t *bd_addr, bool sco_enable)
{
    return BT_STS_SUCCESS;
}

bt_status_t bt_ag_send_bsir(const bt_bdaddr_t *bd_addr, bool in_band_ring_enable)
{
    char at_result[] = {'\r', '\n', '+', 'B', 'S', 'I', 'R', ':', ' ', in_band_ring_enable ? '1' : '0', '\r', '\n', 0};
    return bt_ag_send_at_result(bd_addr, at_result);
}

int bt_ag_is_noise_reduction_supported(const bt_bdaddr_t *bd_addr)
{
    uint32_t hf_features = 0;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return false;
    }
    hf_features = btif_ag_get_hf_features(curr_device->hf_channel);
    return (hf_features & BT_HF_FEAT_ECNR) != 0;
}

int bt_ag_is_voice_recognition_supported(const bt_bdaddr_t *bd_addr)
{
    uint32_t hf_features = 0;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return false;
    }
    hf_features = btif_ag_get_hf_features(curr_device->hf_channel);
    return (hf_features & BT_HF_FEAT_VR) != 0;
}

bt_status_t bes_bt_ag_set_microphone_gain(const bt_bdaddr_t *remote, uint8_t volume)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (curr_device)
    {
        btif_ag_set_microphone_gain(curr_device->hf_channel, volume);
    }
    return BT_STS_PENDING;
}
#endif /* BT_HFP_AG_ROLE */
#endif /* BT_HFP_SUPPORT */

/**
 * DIP
 *
 */

void bes_bt_dip_app_sync_init(void)
{
    app_dip_sync_init();
}

void bes_bt_dip_query_remote_info(uint8_t device_id)
{
    app_dip_get_remote_info(device_id);
}

bt_dip_pnp_info_t* bes_bt_dip_get_device_info(bt_bdaddr_t *remote)
{
    return btif_dip_get_device_info(remote);
}

bool bes_bt_dip_check_is_ios_device(uint16_t vend_id, uint16_t vend_id_source)
{
    return btif_dip_check_is_ios_by_vend_id(vend_id, vend_id_source);
}

void bes_bt_dip_set_device_info_queried(const bt_bdaddr_t *remote, bool queried)
{
    btif_me_set_remote_dip_queried(remote, queried);
}

/**
 * HID
 *
 */
#ifdef BT_HID_DEVICE
bt_status_t bt_hid_init(bt_hid_callback_t callback)
{
    if (callback)
    {
        bt_add_event_callback((bt_event_callback_t)callback, BT_EVENT_MASK_HID_GROUP);
    }

    return BT_STS_SUCCESS;
}

bt_status_t bt_hid_cleanup(void)
{
    return BT_STS_SUCCESS;
}

bt_status_t bt_hid_connect(const bt_bdaddr_t *bd_addr)
{
    return bt_export_hid_profile_connect(bd_addr);
}

bt_status_t bt_hid_disconnect(const bt_bdaddr_t *bd_addr)
{
    return bt_export_hid_profile_disconnect(bd_addr);
}

void bes_bt_hid_send_consumer_ctrl_request(uint8_t ctrl_key)
{
    bt_defer_call_func_1(app_bt_hid_send_consumer_ctrl_key, bt_fixed_param(ctrl_key));
}
#endif // BT_HID_DEVICE

/**
 * IBRT
 *
 */

#ifdef IBRT
static bt_ibrt_state_t bt_export_ibrt_get_state(int device_id)
{
    bt_ibrt_state_t ibrt_state = {0};
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (curr_device)
    {
        ibrt_state.ibrt_slave_force_disc_hfp = curr_device->ibrt_slave_force_disc_hfp;
        ibrt_state.ibrt_slave_force_disc_a2dp = curr_device->ibrt_slave_force_disc_a2dp;
        ibrt_state.ibrt_slave_force_disc_avrcp = curr_device->ibrt_slave_force_disc_avrcp;
        ibrt_state.mock_hfp_after_force_disc = curr_device->mock_hfp_after_force_disc;
        ibrt_state.mock_a2dp_after_force_disc = curr_device->mock_a2dp_after_force_disc;
        ibrt_state.mock_avrcp_after_force_disc = curr_device->mock_avrcp_after_force_disc;
    }
    return ibrt_state;
}

static void bt_export_ibrt_set_field(int device_id, BT_IBRT_FIELD_ENUM_T field, uint32_t value)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (curr_device == NULL)
    {
        return;
    }
    switch (field)
    {
        case BT_IBRT_SLAVE_FORCE_DISC_HFP:
            curr_device->ibrt_slave_force_disc_hfp = value ? true : false;
            break;
        case BT_IBRT_SLAVE_FORCE_DISC_A2DP:
            curr_device->ibrt_slave_force_disc_a2dp = value ? true : false;
            break;
        case BT_IBRT_SLAVE_FORCE_DISC_AVRCP:
            curr_device->ibrt_slave_force_disc_avrcp = value ? true : false;
            break;
        case BT_IBRT_MOCK_HFP_AFTER_FORCE_DISC:
            curr_device->mock_hfp_after_force_disc = value ? true : false;
            break;
        case BT_IBRT_MOCK_A2DP_AFTER_FORCE_DISC:
            curr_device->mock_a2dp_after_force_disc = value ? true : false;
            break;
        case BT_IBRT_MOCK_AVRCP_AFTER_FORCE_DISC:
            curr_device->mock_avrcp_after_force_disc = value ? true : false;
            break;
        default:
            break;
    }
}

static void bt_export_ibrt_mock_a2dp_callback(uint8_t device_id, const bt_bdaddr_t *remote, const a2dp_callback_parms_t *info)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (curr_device)
    {
        a2dp_callback(device_id, curr_device->a2dp_connected_stream, info);
    }
}

static void bt_export_ibrt_mock_avrcp_callback(uint8_t device_id, const bt_bdaddr_t *remote, const avrcp_callback_parms_t *parms)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (curr_device)
    {
        avrcp_callback_CT(device_id, curr_device->avrcp_channel, parms);
    }
}

static uint32_t bt_export_ibrt_save_profile(BT_PROFILE_SYNC_ENUM_T op, bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len)
{
    uint32_t len = 0;

    switch (op)
    {
        case BT_PROFILE_SYNC_BDADDR:
            len = app_bt_save_bd_addr_ctx(remote, buf, buf_len);
            break;
#ifdef BT_A2DP_SUPPORT
        case BT_PROFILE_SYNC_APP_A2DP:
            len = app_bt_save_a2dp_app_ctx(remote, buf, buf_len);
            break;
        case BT_PROFILE_SYNC_STACK_A2DP:
            len = btif_a2dp_profile_save_ctx(remote, buf, buf_len);
            break;
#endif
        case BT_PROFILE_SYNC_APP_AVRCP:
            len = app_bt_save_avrcp_app_ctx(remote, buf, buf_len);
            break;
        case BT_PROFILE_SYNC_APP_HFP:
            len = app_bt_save_hfp_app_ctx(remote, buf, buf_len);
            break;
#ifdef BT_MAP_SUPPORT
        case BT_PROFILE_SYNC_APP_MAP:
            len = app_bt_save_map_app_ctx(remote, buf, buf_len);
            break;
#endif
#ifdef BT_AVRCP_SUPPORT
        case BT_PROFILE_SYNC_STACK_AVRCP:
            len = btif_avrcp_profile_save_ctxs(remote, buf, buf_len);
            break;
#endif

#ifdef BT_HFP_SUPPORT
        case BT_PROFILE_SYNC_STACK_HFP:
            len = btif_hfp_profile_save_ctx(remote, buf, buf_len);
            break;
#endif
#ifdef BT_MAP_SUPPORT
        case BT_PROFILE_SYNC_STACK_MAP:
            len = btif_map_profile_save_ctx(remote, buf, buf_len);
            break;
#endif
        default:
            TRACE(0, "bt_export_ibrt_save_profile: invalid op %d", op);
            break;
    }

    return len;
}

static uint32_t bt_export_ibrt_save_spp_profile(BT_PROFILE_SYNC_ENUM_T op, uint64_t app_id, bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len)
{
    uint32_t len = 0;

    if (op == BT_PROFILE_SYNC_APP_SPP)
    {
        len = app_bt_save_spp_app_ctx(app_id, remote, buf, buf_len);
    }
    else if (op == BT_PROFILE_SYNC_STACK_SPP)
    {
        len = btif_spp_profile_save_ctx(app_id, remote, buf, buf_len);
    }
    else
    {
        TRACE(0, "bt_export_ibrt_restore_spp_profile: invalid op %d", op);
    }

    return len;
}

static uint32_t bt_export_ibrt_restore_profile(BT_PROFILE_SYNC_ENUM_T op, bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len)
{
    uint32_t len = 0;

    switch (op)
    {
        case BT_PROFILE_SYNC_BDADDR:
            len = app_bt_restore_bd_addr_ctx(remote, buf, buf_len);
            break;
#ifdef BT_A2DP_SUPPORT
        case BT_PROFILE_SYNC_APP_A2DP:
            len = app_bt_restore_a2dp_app_ctx(remote, buf, buf_len);
            break;
        case BT_PROFILE_SYNC_STACK_A2DP:
            len = btif_a2dp_profile_restore_ctx(remote, buf, buf_len);
            break;
#endif
        case BT_PROFILE_SYNC_APP_AVRCP:
            len = app_bt_restore_avrcp_app_ctx(remote, buf, buf_len);
            break;
        case BT_PROFILE_SYNC_APP_HFP:
            len = app_bt_restore_hfp_app_ctx(remote, buf, buf_len);
            break;
#ifdef BT_MAP_SUPPORT
        case BT_PROFILE_SYNC_APP_MAP:
            len = app_bt_restore_map_app_ctx(remote, buf, buf_len);
            break;
#endif

#ifdef BT_AVRCP_SUPPORT
        case BT_PROFILE_SYNC_STACK_AVRCP:
            len = btif_avrcp_profile_restore_ctxs(remote, buf, buf_len);
            break;
#endif

#ifdef BT_HFP_SUPPORT
        case BT_PROFILE_SYNC_STACK_HFP:
            len = btif_hfp_profile_restore_ctx(remote, buf, buf_len);
            break;
#endif
        case BT_PROFILE_SYNC_STACK_SPP:
            len = btif_spp_profile_restore_ctx(remote, buf, buf_len);
            break;
#ifdef BT_MAP_SUPPORT
        case BT_PROFILE_SYNC_STACK_MAP:
            len = btif_map_profile_restore_ctx(remote, buf, buf_len);
            break;
#endif
        default:
            TRACE(0, "bt_export_ibrt_restore_profile: invalid op %d", op);
            break;
    }

    return len;
}

static uint32_t bt_export_ibrt_restore_spp_profile(BT_PROFILE_SYNC_ENUM_T op, bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len, uint64_t app_id)
{
    uint32_t len = 0;

    if (op == BT_PROFILE_SYNC_APP_SPP)
    {
        len = app_bt_restore_spp_app_ctx(remote, buf, buf_len, app_id);;
    }
    else if (op == BT_PROFILE_SYNC_STACK_SPP)
    {
        len = btif_spp_profile_restore_ctx(remote, buf, buf_len);
    }
    else
    {
        TRACE(0, "bt_export_ibrt_restore_spp_profile: invalid op %d", op);
    }

    return len;
}

void bes_bt_tws_besaud_client_create(uint16_t conn_handle)
{
    btif_besaud_client_create(conn_handle);
}

void bes_bt_tws_besaud_server_create(void (*cb)(uint16_t besaud_event))
{
    btif_besaud_server_create(cb);
}

uint8_t bes_bt_tws_besaud_is_connected(void)
{
    return btif_besaud_is_connected();
}

void bes_bt_tws_besaud_data_recv_register(void (*cb)(uint8_t *data, uint16_t len))
{
    btif_besaud_data_received_register(cb);
}

void bes_bt_tws_besaud_clear_cmd_sending(void)
{
    btif_besaud_clear_cmd_sending();
}

bt_status_t bes_bt_tws_besaud_send_cmd_no_wait(uint8_t* cmd, uint16_t len)
{
    return btif_besaud_send_cmd_no_wait(cmd, len);
}

void bes_bt_tws_besaud_send_cmd(uint8_t* cmd, uint16_t len)
{
    btif_besaud_send_cmd(cmd, len);
}

uint8_t bes_bt_tws_besaud_is_cmd_sending(void)
{
    return btif_besaud_is_cmd_sending();
}

bt_ibrt_state_t bes_bt_tws_ibrt_get_state(int device_id)
{
    return bt_export_ibrt_get_state(device_id);
}

void bes_bt_tws_ibrt_set_field(int device_id, BT_IBRT_FIELD_ENUM_T field, uint32_t value)
{
    bt_export_ibrt_set_field(device_id, field, value);
}

void bes_bt_tws_ibrt_reconnect_mobile_profile(const bt_bdaddr_t *remote)
{
    app_bt_ibrt_reconnect_mobile_profile(remote);
}

void bes_bt_tws_ibrt_set_remote_tws_device(const bt_bdaddr_t *remote, bool tws_link)
{
    bt_export_set_remote_tws_device(remote, tws_link);
}

bool bes_bt_tws_ibrt_is_remote_tws_device(const bt_bdaddr_t *remote)
{
    return bt_export_is_remote_tws_device(remote);
}

void bes_bt_tws_ibrt_clear_reconnect_mobile_profile_flag(bt_bdaddr_t* remote_addr)
{
    app_bt_ibrt_reconnect_mobile_profile_flag_clear(remote_addr);
}

void bes_bt_tws_ibrt_reset_tws_packet_check(void)
{
    app_bt_reset_tws_acl_data_packet_check();
}

bool bes_bt_tws_ibrt_master_wait_new_master_ready(const bt_bdaddr_t* remote)
{
    return btif_ibrt_master_wait_remote_new_master_ready(remote);
}

void bes_bt_tws_ibrt_slave_become_master(const bt_bdaddr_t* remote)
{
    btif_ibrt_slave_become_master(remote);
}

void bes_bt_tws_ibrt_master_become_slave(const bt_bdaddr_t* remote)
{
    btif_ibrt_master_become_slave(remote);
}

void bes_bt_tws_ibrt_master_set_switch_start(const bt_bdaddr_t* remote)
{
    btif_ibrt_master_tws_switch_set_start(remote);
}

void bes_bt_tws_ibrt_slave_set_switch_start(const bt_bdaddr_t* remote)
{
    btif_ibrt_slave_tws_switch_set_start(remote);
}

void bes_bt_tws_ibrt_old_master_receive_ready_req(struct btif_sync_data_to_new_master_t *sync_data, const bt_bdaddr_t *remote)
{
    btif_ibrt_old_master_receive_ready_req(sync_data, remote);
}

void bes_bt_tws_ibrt_new_master_receive_ready_rsp(struct btif_sync_data_to_new_master_t *sync_data)
{
    btif_ibrt_new_master_receive_ready_rsp(sync_data);
}

bt_status_t bes_bt_tws_start_ibrt(uint16_t tws_conhandle, uint16_t mobile_conhandle)
{
    return btif_me_start_ibrt(tws_conhandle, mobile_conhandle);
}

bt_status_t bes_bt_tws_stop_ibrt(uint16_t mobile_conhdl,uint8_t reason)
{
    return btif_me_stop_ibrt(mobile_conhdl, reason);
}

void bes_bt_tws_set_env(uint8_t sniffer_acitve, uint8_t sniffer_role, uint8_t * monitored_addr, uint8_t * sniffer_addr)
{
    btif_me_set_sniffer_env(sniffer_acitve, sniffer_role, monitored_addr, sniffer_addr);
}

bt_status_t bes_bt_tws_ibrt_role_switch(uint16_t mobile_conhdl)
{
    return btif_me_ibrt_role_switch(mobile_conhdl);
}

bt_status_t bes_bt_tws_ibrt_mode_init(bool enable)
{
    return btif_me_ibrt_mode_init(enable);
}

bt_status_t bes_bt_tws_suspend_ibrt(void)
{
    return btif_me_suspend_ibrt();
}

bt_status_t bes_bt_tws_resume_ibrt(uint8_t enable)
{
    return btif_me_resume_ibrt(enable);
}

bt_status_t bes_bt_tws_ibrt_enable_fastack(uint16_t conhdl, uint8_t direction, uint8_t enable)
{
    return btif_me_enable_fastack(conhdl, direction, enable);
}

void bes_bt_tws_ibrt_clean_slave_state(const bt_bdaddr_t* remote)
{
    btif_ibrt_stack_clean_slave_status(remote);
}

void bes_bt_tws_ibrt_response_acl_conn_req(bt_bdaddr_t *remote, bool accept)
{
    btif_me_response_acl_conn_req(remote, accept, BTIF_BEC_LIMITED_RESOURCE);
}

#ifdef BT_A2DP_SUPPORT
uint8_t bes_bt_tws_ibrt_is_critical_avdtp_cmd_handling(void)
{
    return btif_a2dp_is_critical_avdtp_cmd_handling();
}

void bes_bt_tws_ibrt_critical_avdtp_cmd_timeout(void)
{
    btif_a2dp_critical_avdtp_cmd_timeout();
}
#endif

void bes_bt_tws_ibrt_register_hci_acl_ecc_softbit_handler(bt_hci_acl_ecc_softbit_handler_func func)
{
    //register_hci_acl_ecc_softbit_handler_callback(func);
}

void bes_bt_tws_ibrt_fake_mobile_disconnect(uint16_t hci_handle, uint8_t reason)
{
    btif_me_fake_mobile_disconnect(hci_handle, reason);
}

void bes_bt_tws_ibrt_fake_tws_disconnect(uint16_t hci_handle, uint8_t reason)
{
    btif_me_fake_tws_disconnect(hci_handle, reason);
}

void bes_bt_tws_ibrt_fake_tws_connect(uint8_t status, bt_bdaddr_t *bdAddr)
{
    btif_me_fake_tws_connect(status, bdAddr);
}

void bes_bt_tws_ibrt_fake_hci_event_disallow(uint8_t opcode1, uint8_t opcode2)
{
    btif_me_ibrt_simu_hci_event_disallow(opcode1, opcode2);
}

void bes_bt_tws_ibrt_mock_a2dp_callback(uint8_t device_id, const bt_bdaddr_t *remote, const a2dp_callback_parms_t *info)
{
    bt_export_ibrt_mock_a2dp_callback(device_id, remote, info);
}

void bes_bt_tws_ibrt_mock_avrcp_callback(uint8_t device_id, const bt_bdaddr_t *remote, const avrcp_callback_parms_t *parms)
{
    bt_export_ibrt_mock_avrcp_callback(device_id, remote, parms);
}

uint32_t bes_bt_tws_a2dp_get_ibrt_session(uint8_t device_id)
{
#ifdef BT_A2DP_SUPPORT
    return a2dp_ibrt_session_get(device_id);
#else
    return 0;
#endif
}

int bes_bt_tws_a2dp_set_ibrt_session(uint8_t session, uint8_t device_id)
{
#ifdef BT_A2DP_SUPPORT
    return a2dp_ibrt_session_set(session, device_id);
#else
    return 0;
#endif
}

int bes_bt_tws_ibrt_a2dp_sync_get_status(uint8_t device_id, ibrt_a2dp_status_t *a2dp_status)
{
#ifdef BT_A2DP_SUPPORT
    return a2dp_ibrt_sync_get_status(device_id, a2dp_status);
#else
    return 0;
#endif
}

int bes_bt_tws_ibrt_a2dp_sync_set_status(uint8_t device_id, ibrt_a2dp_status_t *a2dp_status)
{
#ifdef BT_A2DP_SUPPORT
    return a2dp_ibrt_sync_set_status(device_id, a2dp_status);
#else
    return 0;
#endif
}

#ifdef BT_HFP_SUPPORT
void bes_bt_tws_ibrt_register_sco_link(uint8_t device_id, bt_bdaddr_t *remote)
{
    app_ibrt_register_sco_link(device_id, remote);
}

bt_status_t bes_bt_tws_ibrt_mock_sync_conn_disconnected(uint16_t conn_handle)
{
    return btif_hf_sync_conn_audio_disconnected(conn_handle);
}

bt_status_t bes_bt_tws_ibrt_mock_sync_conn_connected(hfp_sco_codec_t sco_codec, uint16_t conhdl)
{
    return btif_hf_sync_conn_audio_connected(sco_codec, conhdl);
}

int bes_bt_tws_ibrt_hfp_service_connected_mock(uint8_t device_id)
{
    return hfp_ibrt_service_connected_mock(device_id);
}

int bes_bt_tws_ibrt_hfp_sync_get_status(uint8_t device_id,ibrt_hfp_status_t *hfp_status)
{
    return hfp_ibrt_sync_get_status(device_id, hfp_status);
}

int bes_bt_tws_ibrt_hfp_sync_set_status(uint8_t device_id,ibrt_hfp_status_t *hfp_status)
{
    return hfp_ibrt_sync_set_status(device_id, hfp_status);
}

int bes_bt_tws_ibrt_hfp_sco_audio_disconnected(void)
{
    return hfp_ibrt_sco_audio_disconnected();
}

int bes_bt_tws_ibrt_hfp_sco_audio_connected(hfp_sco_codec_t codec, uint16_t sco_connhdl)
{
    return hfp_ibrt_sco_audio_connected(codec, sco_connhdl);
}

void bes_bt_tws_ibrt_hfp_sync_status_sent_callback(void)
{
    hfp_ibrt_sync_status_sent_callback();
}

void bes_bt_tws_ibrt_receive_peer_sco_codec_info(const void* remote, uint8_t codec)
{
    btif_hf_receive_peer_sco_codec_info(remote, codec);
}

bt_status_t bes_bt_tws_ibrt_force_disconnect_hfp_profile(uint8_t device_id,uint8_t reason)
{
    return btif_hfp_force_disconnect_hfp_profile(device_id, reason);
}
#endif
void bes_bt_tws_ibrt_spp_slave_release_dlc_connection(uint32_t device_id, uint32_t dlci)
{
    btif_spp_ibrt_slave_release_dlc_connection(device_id, dlci);
}

#ifdef BT_AVRCP_SUPPORT
void bes_bt_tws_ibrt_avrcp_slave_restore_sdp_info(uint8_t device_id, uint16_t avctp_version, uint16_t avrcp_version, uint16_t support_feature)
{
    btif_avrcp_ibrt_slave_restore_sdp_info(device_id, avctp_version, avrcp_version, support_feature);
}
#endif

void bes_bt_tws_ibrt_force_disconnect_a2dp_profile(uint8_t device_id,uint8_t reason)
{
#ifdef BT_A2DP_SUPPORT
    btif_a2dp_force_disconnect_a2dp_profile(device_id, reason);
#endif
}

#ifdef BT_AVRCP_SUPPORT
bt_status_t bes_bt_tws_ibrt_force_disconnect_avrcp_profile(uint8_t device_id,uint8_t reason)
{
   return btif_avrcp_force_disconnect_avrcp_profile(device_id, reason);
}
#endif

#if defined(__GATT_OVER_BR_EDR__)
void bes_bt_tws_ibrt_force_disconnect_btgatt_profile(uint8_t device_id,uint8_t reason)
{
    app_btgatt_disconnect(device_id);
}
#endif

void bes_bt_tws_ibrt_force_disconnect_spp_profile(const bt_bdaddr_t *addr,uint64_t app_id,uint8_t reason)
{
    btif_spp_force_disconnect_spp_profile(addr, app_id, reason);
}

void bes_bt_tws_ibrt_sync_set_avdtp_streaming_state(bt_bdaddr_t *addr)
{
#ifdef BT_A2DP_SUPPORT
    btif_a2dp_sync_avdtp_streaming_state(addr);
#endif
}

uint64_t bes_bt_tws_ibrt_get_app_id_from_spp_flag(uint8_t spp_flag)
{
    return btif_app_get_app_id_from_spp_flag(spp_flag);
}

uint8_t bes_bt_tws_ibrt_get_spp_flag_from_app_id(uint64_t app_id)
{
    return btif_app_get_spp_flag_from_app_id(app_id);
}

uint32_t bes_bt_tws_ibrt_save_profile(BT_PROFILE_SYNC_ENUM_T op, bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len)
{
    return bt_export_ibrt_save_profile(op, remote, buf, buf_len);
}

uint32_t bes_bt_tws_ibrt_save_spp_profile(BT_PROFILE_SYNC_ENUM_T op, uint64_t app_id, bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len)
{
    return bt_export_ibrt_save_spp_profile(op, app_id, remote, buf, buf_len);
}

uint32_t bes_bt_tws_ibrt_restore_profile(BT_PROFILE_SYNC_ENUM_T op, bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len)
{
    return bt_export_ibrt_restore_profile(op, remote, buf, buf_len);
}

uint32_t bes_bt_tws_ibrt_restore_spp_profile(BT_PROFILE_SYNC_ENUM_T op, bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len, uint64_t app_id)
{
    return bt_export_ibrt_restore_spp_profile(op, remote, buf, buf_len, app_id);
}

const char *bes_bt_tws_ibrt_get_device_current_roles(void)
{
    return app_bt_get_device_current_roles();
}

void bes_bt_tws_audio_receive_peer_a2dp_playing_device(bool is_response, uint8_t device_id)
{
    app_bt_audio_receive_peer_a2dp_playing_device(is_response, device_id);
}

uint32_t bes_bt_tws_audio_start_trigger_switch_a2dp(uint32_t btclk)
{
    return app_bt_audio_trigger_switch_streaming_a2dp(btclk);
}

bool bes_bt_tws_audio_switch_streaming_a2dp(void)
{
    return app_bt_audio_switch_streaming_a2dp();
}

#if defined(BT_HFP_SUPPORT)
void bes_bt_tws_audio_switch_streaming_sco(void)
{
    app_bt_audio_switch_streaming_sco();
}
#endif // BT_HFP_SUPPORT

void bes_bt_tws_ibrt_report_audio_retrigger(uint8_t retrgigerType)
{
    app_bt_report_audio_retrigger(retrgigerType);
}

void bes_bt_tws_ibrt_a2dp_reject_sniff_start(uint8_t device_id, uint32_t timeout)
{
#ifdef BT_A2DP_SUPPORT
    app_a2dp_reject_sniff_start(device_id, timeout);
#endif
}

#endif /* IBRT */

/**
 * SOURCE
 *
 */

#ifdef BT_SOURCE

void bes_bt_source_start_a2dp_stream(uint8_t device_id)
{
    app_a2dp_source_start_stream(device_id);
}

void bes_bt_source_suspend_a2dp_stream(uint8_t device_id)
{
    app_a2dp_source_suspend_stream(device_id);
}

void bes_bt_source_toggle_a2dp_stream(uint8_t device_id)
{
    app_a2dp_source_toggle_stream(device_id);
}

uint32_t bes_bt_source_write_a2dp_pcm_data(uint8_t * pcm_buf, uint32_t len)
{
    return a2dp_source_write_pcm_data(pcm_buf, len);
}
uint32_t bes_bt_source_write_a2dp_encoded_data(uint8_t encoded_type, uint8_t * pcm_buf, uint32_t len, uint16_t frame_len)
{
    return a2dp_source_write_encoded_data(encoded_type, pcm_buf, len, frame_len);
}

uint32_t bes_bt_source_write_a2dp_data(bool packet_type, uint8_t encoded_type, uint8_t * pcm_buf, uint32_t len, uint16_t frame_len)
{
    uint32_t ret = 0;

    if (packet_type)
    {
        ret = bes_bt_source_write_a2dp_encoded_data(encoded_type, pcm_buf, len, frame_len);
    }
    else
    {
        ret = bes_bt_source_write_a2dp_pcm_data(pcm_buf,len);
    }
    return ret;
}


void bes_bt_source_reconnect_hfp_profile(const bt_bdaddr_t *remote)
{
    bt_source_reconnect_hfp_profile(remote);
}

void bes_bt_source_reconnect_a2dp_profile(const bt_bdaddr_t *remote)
{
    bt_source_reconnect_a2dp_profile(remote);
}

void bes_bt_source_reconnect_avrcp_profile(const bt_bdaddr_t *remote)
{
    bt_source_reconnect_avrcp_profile(remote);
}

void bes_bt_source_search_device(void)
{
    app_bt_source_search_device();
}

void bes_bt_source_register_bt_callback(bt_source_callback_t cb)
{
    bt_source_register_callback(cb);
}

void bes_bt_source_a2dp_stream_buffer_init()
{
    app_a2dp_source_audio_init();
}

void bes_bt_source_a2dp_stream_buffer_deinit()
{
    app_a2dp_source_audio_deinit();
}

#if defined(BT_HID_DEVICE)
void bes_bt_source_reconnect_hid_profile(const bt_bdaddr_t *remote)
{
    bt_source_reconnect_hid_profile(remote);
}
#endif // BT_HID_DEVIC
#endif /* BT_SOURCE */
