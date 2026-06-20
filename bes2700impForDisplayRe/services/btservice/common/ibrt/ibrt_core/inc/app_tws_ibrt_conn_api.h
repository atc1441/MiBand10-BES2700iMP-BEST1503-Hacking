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
#ifndef __APP_TWS_IBRT_CORE_API_H__
#define __APP_TWS_IBRT_CORE_API_H__

#if defined(IBRT_UI)
#include "app_tws_ibrt_core_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*tws_switch_disallow_cb)(void);

typedef void (*tws_share_info_func_t)(void);

/**************************************API FOR EXTRENAL CALL**************************************/
#if defined(FPGA)
void app_ibrt_conn_fpga_start_tws_pairing(ibrt_role_e role);
#endif


/**
 ****************************************************************************************
 * @brief Initiate ibrt_conn
 ****************************************************************************************
 */
void app_ibrt_conn_init();


/**
 ****************************************************************************************
 * @brief print conn info for debug (autotest)
 ****************************************************************************************
 */
void app_ibrt_conn_dump_ibrt_info();


/**
 ****************************************************************************************
 * @brief register ibrt_if callback
 *
 * @param[in] cbs       app_ibrt_if_cbs_t callbacks struct
 ****************************************************************************************
 */
void app_ibrt_conn_reg_ibrt_if_cb(const app_ibrt_if_cbs_t *cbs);


/**
 ****************************************************************************************
 * @brief register ibrt_conn_evt callback
 *
 * @param[in] cbs       app_ibrt_conn_event_cb callbacks struct
 ****************************************************************************************
 */
void app_ibrt_conn_reg_evt_cb(const app_ibrt_conn_event_cb *cbs);

/**
 ****************************************************************************************
 * @brief register app_ibrt_get_peer_rssi_cb tws  callback
 *
 * @param[in] cbs       app_ibrt_get_peer_rssi_cb callbacks struct
 ****************************************************************************************
 */
void app_ibrt_conn_reg_get_peer_rssi_cb(const app_ibrt_get_peer_rssi_cb *cb);


/**
 ****************************************************************************************
 * @brief register cmd_handler callback
 *
 * @param[in] cbs       app_ibrt_cmd_ui_handler_cb callbacks struct
 ****************************************************************************************
 */
void app_ibrt_conn_reg_cmd_handler_cbs(const app_ibrt_cmd_ui_handler_cb *cbs);

/**
 ****************************************************************************************
 * @brief share some info asap, register tws info callback, only tws BT_MASTER will
 *
 * @param[in] cb       tws_share_info_func_t cb
 ****************************************************************************************
 */
void app_ibrt_conn_reg_tws_share_info_cb(tws_share_info_func_t cb);

/**
 ****************************************************************************************
 * @brief share some info asap, send tws share info complete event
 *
 ****************************************************************************************
 */
void app_ibrt_conn_send_share_info_complete_evt(void);

/**
 ****************************************************************************************
 * @brief Not implemented
 *
 * @param[in] cbs       Mobile addr
 ****************************************************************************************
 */
void app_ibrt_conn_get_tws_buds_info(app_tws_buds_info_t* buds_info);


/**
 ****************************************************************************************
 * @brief Not implemented
 *
 * @param[in] cbs       Mobile addr
 ****************************************************************************************
 */
void app_ibrt_conn_disc_all_mobile_link();


/**
 ****************************************************************************************
 * @brief Set freeman_enable
 ****************************************************************************************
 */
void app_ibrt_conn_set_freeman_enable(void);


/**
 ****************************************************************************************
 * @brief Clear freeman_enable
 ****************************************************************************************
 */
void app_ibrt_conn_clear_freeman_enable(void);

/**
 ****************************************************************************************
 * @brief perform choice mobile connect
 *
 * @param[in] addr       Mobile address
 ****************************************************************************************
 */
void app_ibrt_conn_connect_choice_mobile(const bt_bdaddr_t *addr, uint8_t try_count);

/**
 ****************************************************************************************
 * @brief send get peer rssi req
 *
 * @param[in] buf        data buf
 * @param[in] len        data len
 ****************************************************************************************
 */
void app_ibrt_conn_send_get_peer_rssi_req(uint8_t *buf, uint16_t len);

/**
 ****************************************************************************************
 * @brief send get peer rssi req
 *
 * @param[in] rsp_seq    rsp seq
 * @param[in] buf        data buf
 * @param[in] len        data len
 ****************************************************************************************
 */
void app_ibrt_conn_send_get_peer_rssi_rsp(uint16_t rsp_seq, uint8_t *buf, uint16_t len);

/**
 ****************************************************************************************
 * @brief sync_info
 *
 * @param[in] sync_info       buff
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_sync_info(uint8_t *buff, uint16_t len);

/**
 ****************************************************************************************
 * @brief sync_info_rsp
 *
 * @param[in] sync_info       buff
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_sync_info_rsp(uint16_t rsp_seq,uint8_t *buff, uint16_t len);

/**
 ****************************************************************************************
 * @brief send destroy device req
 *
 * @param[in] buf       destroy req
 * @param[in] len       buf len
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_destroy_device(uint8_t *buf, uint16_t len);

/**
 ****************************************************************************************
 * @brief response destroy device rsp
 *
 * @param[in] addr       Mobile address
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_destroy_device_rsp(uint16_t rsp_seq, uint8_t * address, uint16_t len);

/**
 ****************************************************************************************
 * @brief send sync ui mode infomation
 *
 * @param[in] buf       ui mode
 * @param[in] len       buf len
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_notify_ui_info(uint8_t *buf, uint16_t len);

/**
 ****************************************************************************************
 * @brief common chnl send data
 *
 * @param[in] buf       data buf
 * @param[in] len       data length
 ****************************************************************************************
 */
bool app_ibrt_common_chnl_send_data(uint8_t *buf, uint16_t len);

/**
 ****************************************************************************************
 * @brief Set ui-role
 *
 * @param[in] ui_role       ui_role
 ****************************************************************************************
 */
void app_ibrt_conn_set_ui_role(TWS_UI_ROLE_E ui_role);


/**
 ****************************************************************************************
 * @brief Set ui-role
 *
 * @param[in] ui_role       ui_role
 ****************************************************************************************
 */
void app_ibrt_conn_send_user_action_v2(uint8_t *p_buff, uint16_t length);


/**
 ****************************************************************************************
 * @brief Dispatch event
 *
 * @param[in] ui_role       ui_role
 * @param[in] param0        ui_role
 * @param[in] param1        ui_role
 * @param[in] param2        ui_role
 ****************************************************************************************
 */
void app_ibrt_conn_dispatch_event(unsigned int event, unsigned int param0, unsigned int param1, unsigned int param2);

/**
 ****************************************************************************************
 * @brief Check if there exist ibrt connection
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>Exist ibrt connection
 * <tr><td>False  <td>No ibrt connection
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_conn_any_ibrt_connected(void);

/**
 ****************************************************************************************
 * @brief Check if there exist mobile connection
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>Exist mobile connection and ibrt connection
 * <tr><td>False <td>No mobile connection
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_conn_any_mobile_connected();

/**
 ****************************************************************************************
 * @brief Check mobile ibrt link connection by mobile address
 *
 * @param[in] addr       Mobile address
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>mobile ibrt is connected
 * <tr><td>False  <td>mobile ibrt not connected
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_conn_is_ibrt_connected(const bt_bdaddr_t *addr);

/**
 ****************************************************************************************
 * @brief Check mobile sm state in ibrt connected
 *
 * @param[in] addr       Mobile address
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>mobile ibrt is connected
 * <tr><td>False  <td>mobile ibrt not connected
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_conn_is_mobile_state_ibrt_connected(void* addr);

/**
 ****************************************************************************************
 * @brief Check mobile link connection by mobile address
 *
 * @param[in] addr       Mobile address
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>mobile link is connected
 * <tr><td>False  <td>mobile link not connected
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_conn_mobile_link_connected(const bt_bdaddr_t* addr);


/**
 ****************************************************************************************
 * @brief Check if in freeman mode
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>In freeman mode
 * <tr><td>False  <td>Not in freeman mode
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_conn_is_freeman_mode(void);


/**
 ****************************************************************************************
 * @brief Check if current role is ibrt-master for the mobile
 *
 * @param[in] addr       Mobile address
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>current role is ibrt-master
 * <tr><td>False  <td>current role not ibrt-master
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_conn_is_ibrt_master(const bt_bdaddr_t* addr);


/**
 ****************************************************************************************
 * @brief Check if current role is ibrt-slave for the mobile
 *
 * @param[in] addr       Mobile address
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>current role is ibrt-slave
 * <tr><td>False  <td>current role not ibrt-slave
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_conn_is_ibrt_slave(const bt_bdaddr_t* addr);


/**
 ****************************************************************************************
 * @brief Check if nv master
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>is nv master
 * <tr><td>False  <td>not nv master
 ****************************************************************************************
 */
bool app_ibrt_conn_is_nv_master(void);


/**
 ****************************************************************************************
 * @brief Check if nv slave
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>is nv slave
 * <tr><td>False  <td>not nv slave
 ****************************************************************************************
 */
bool app_ibrt_conn_is_nv_slave(void);


/**
 ****************************************************************************************
 * @brief Check if profile_exchanged for the mobile
 *
 * @param[in] addr       Mobile address
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>profile_exchanged
 * <tr><td>False  <td>profile_exchanged
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_conn_is_profile_exchanged(const bt_bdaddr_t *addr);


/**
 ****************************************************************************************
 * @brief Check if ibrt sm is idle
 *
 * @param[in] addr       Mobile address
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>ibrt sm is idle
 * <tr><td>False  <td>ibrt sm not idle
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_conn_is_ibrt_idle(const bt_bdaddr_t *addr);


/**
 ****************************************************************************************
 * @brief Check if exist link on role switch
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>exist link on role switch
 * <tr><td>False  <td>no link is on role switch
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_conn_any_role_switch_running(void);

/**
 ****************************************************************************************
 * @brief Check if all link run role switch complete
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>all link run role switch complete
 * <tr><td>False  <td>exist link run role switch event failed or current doing role switch
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_conn_all_ibrt_role_complete(ibrt_role_e role);

/**
 ****************************************************************************************
 * @brief Check if tws link is connected
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>tws link is connected
 * <tr><td>False  <td>tws link not conencted
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_conn_is_tws_connected(void);


/**
 ****************************************************************************************
 * @brief Check if in tws pairing mode
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>in tws pairing mode
 * <tr><td>False  <td>not in tws pairing mode
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_conn_is_tws_in_pairing_state(void);

/**
 ****************************************************************************************
 * @brief Set stop ibrt is ongoing
 *
 * @param[in] stop_ibrt       if stop ibrt is ongoing
 ****************************************************************************************
 */
void app_ibrt_conn_set_stop_ibrt_ongoing(bool stop_ibrt);

/**
 ****************************************************************************************
 * @brief Check if stop ibrt is ongoing
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>stop ibrt is ongoing
 * <tr><td>False  <td>stop ibrt is not ongoing
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_conn_stop_ibrt_ongoing(void);

/**
 ****************************************************************************************
 * @brief Perform ibrt-role-switch for all links
 *
 * @return
 * <table>
 *      none
 ****************************************************************************************
 */
void app_ibrt_conn_all_dev_start_tws_role_switch(void);


/**
 ****************************************************************************************
 * @brief Get the max support mobile device number
 *
 * @return The number of max support mobile device
 ****************************************************************************************
 */
uint8_t app_ibrt_conn_support_max_mobile_dev(void);

uint8_t app_ibrt_conn_get_custom_support_max_dev();

/**
 ****************************************************************************************
 * @brief Get the number of active ibrt links
 *
 * @return The number of active ibrt links
 ****************************************************************************************
 */
uint8_t app_ibrt_conn_get_ibrt_link_count(void);


/**
 ****************************************************************************************
 * @brief Get the number of local connected mobile devices
 *
 * @return The number of local connected mobile devices
 ****************************************************************************************
 */
uint8_t app_ibrt_conn_get_local_connected_mobile_count();


/**
 ****************************************************************************************
 * @brief Get the number of local snoop mobile devices
 *
 * @return The number of local snoop mobile devices
 ****************************************************************************************
 */
uint8_t app_ibrt_conn_get_local_snoop_mobile_count();


/**
 ****************************************************************************************
 * @brief Check if a mobile link is connecting
 *
 * @return  true there is a mobile link connecting
 *          false no mobile link connecting
 ****************************************************************************************
 */
bool app_ibrt_conn_any_mobile_connecting(void);

/**
 ****************************************************************************************
 * @brief Get current cvalid device count and address list.
 *
 * @param[out] addr_list          Used to return current valid device address list
 *
 * @return The number of current ibrt connected count
 ****************************************************************************************
 */
uint8_t app_ibrt_conn_get_valid_device_list(bt_bdaddr_t *addr_list);

/**
 ****************************************************************************************
 * @brief Get current connected ibrt connected device count and address list.
 *
 * @param[out] addr_list          Used to return current ibrt connected mobile device address list
 *
 * @return The number of current ibrt connected count
 ****************************************************************************************
 */
uint8_t app_ibrt_conn_get_ibrt_connected_list(bt_bdaddr_t *addr_list);

/**
 ****************************************************************************************
 * @brief Get current connected mobile device count and address list.
 *
 * @param[out] addr_list          Used to return current connected mobile device address list
 *
 * @return The number of current connected mobile
 ****************************************************************************************
 */
uint8_t app_ibrt_conn_get_connected_mobile_list(bt_bdaddr_t *addr_list);


/**
 ****************************************************************************************
 * @brief Get mobile constate
 *
 * @param[in] addr       Mobile address
 *
 * @return mobile constate
 ****************************************************************************************
 */
uint32_t app_ibrt_conn_get_mobile_constate(void* addr);


/**
 ****************************************************************************************
 * @brief Get ibrt constate
 *
 * @param[in] addr       Mobile address
 *
 * @return ibrt constate
 ****************************************************************************************
 */
uint32_t app_ibrt_conn_get_ibrt_constate(void* addr);


/**
 ****************************************************************************************
 * @brief Send prepare complete messege
 ****************************************************************************************
 */
void app_ibrt_conn_notify_prepare_complete(void);


/**
 ****************************************************************************************
 * @brief Send exchange info
 *
 * @param[in] ibrt_mgr_info      the pointer of ibrt_mgr_info
 * @param[in] len               the length of ibrt_mgr_info
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_exchange_earbuds_info(uint8_t *ibrt_mgr_info, uint16_t len);


/**
 ****************************************************************************************
 * @brief Send exchange info rsp
 *
 * @param[in] rsp_seq            the pointer of rsp sequence
 * @param[in] ibrt_mgr_info      the pointer of ibrt_mgr_info
 * @param[in] len                the length of ibrt_mgr_info
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_exchange_ibrt_mgr_info_rsp(uint16_t rsp_seq,uint8_t *ibrt_mgr_info, uint16_t len);


/**
 ****************************************************************************************
 * @brief Send notify running info
 *
 * @param[in] running_info       the pointer of running_info
 * @param[in] len               the length of running_info
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_notify_running_info(uint8_t *running_info, uint16_t len);


/**
 ****************************************************************************************
 * @brief Send notify running info rsp
 *
 * @param[in] rsp_seq            the pointer of rsp sequence
 * @param[in] running_info       the pointer of running_info
 * @param[in] len                the length of running_info
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_notify_running_info_rsp(uint16_t rsp_seq,uint8_t *running_info, uint16_t len);

/**
 ****************************************************************************************
 * @brief Send devices mgr manager info
 *
 * @param[in] devices_info       the pointer of devices_info
 * @param[in] len                the length of devices_info
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_send_dev_mgr(uint8_t *devices_info, uint16_t len);

/**
 ****************************************************************************************
 * @brief Send devices mgr manager info rsp
 *
 * @param[in] rsp_seq            the pointer of rsp sequence
 * @param[in] devices_info       the pointer of devices_info
 * @param[in] len                the length of devices_info
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_send_dev_mgr_rsp(uint16_t rsp_seq,uint8_t *devices_info, uint16_t len);

/**
 ****************************************************************************************
 * @brief Send lea addr manager info
 *
 * @param[in] addr_info          the pointer of addr_info
 * @param[in] len                the length of addr_info
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_send_lea_addr_mgr(uint8_t *addr_info, uint16_t len);

/**
 ****************************************************************************************
 * @brief Send lea addr manager info rsp
 *
 * @param[in] rsp_seq            the pointer of rsp sequence
 * @param[in] addr_info          the pointer of addr_info
 * @param[in] len                the length of addr_info
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_send_lea_addr_mgr_rsp(uint16_t rsp_seq,uint8_t *addr_info, uint16_t len);

/**
 ****************************************************************************************
 * @brief Initiate ibrt connection for mobile device
 *
 * @param[in] addr       Mobile address
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_connect_ibrt(const bt_bdaddr_t *addr);


/**
 ****************************************************************************************
 * @brief Disconnect ibrt connection for mobile device
 *
 * @param[in] addr       Mobile address
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_disconnect_ibrt(const bt_bdaddr_t *addr);


/**
 ****************************************************************************************
 * @brief Start ibrt role switch for mobile device
 *
 * @param[in] addr       Mobile address
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_tws_role_switch(const bt_bdaddr_t *addr,  bool force);


/**
 ****************************************************************************************
 * @brief Start connect profiles for mobile device
 *
 * @param[in] addr       Mobile address
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_connect_profiles(const bt_bdaddr_t *addr);


/**
 ****************************************************************************************
 * @brief check if any basic profile exits
 ****************************************************************************************
 */
bool app_ibrt_conn_any_basic_profiles_exits(const bt_bdaddr_t *addr);


/**
 ****************************************************************************************
 * @brief Start connect profiles for mobile device
 *
 * @param[in] addr              Mobile address
 * @param[in] direction         The connection is initialed by local or remote
 * @param[in] request_connect   request connect
 * @param[in] timeout           page_timeout (unit is 625us)
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_remote_dev_connect_request(const bt_bdaddr_t *addr, connection_direction_t direction, bool request_connect, uint32_t timeout);


/**
 ****************************************************************************************
 * @brief Initial tws connection
 *
 * @param[in] isInPairingMode       Tws pairing state
 * @param[in] timeout               page_timeout (unit is 625us)
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_tws_connect_request(bool isInPairingMode, uint32_t timeout);


/**
 ****************************************************************************************
 * @brief cancel tws page
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_tws_cancel_page(void);


/**
 ****************************************************************************************
 * @brief Config pagescan status for test
 *
 * @param[in] disc_enable       discover enable
 * @param[in] conn_enable       connection enable
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_set_discoverable_connectable(bool disc_enable, bool conn_enable);


/**
 ****************************************************************************************
 * @brief Cancel connection for mobile device
 *
 * @param[in] addr       Mobile address
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_remote_dev_connect_cancel_request(const bt_bdaddr_t *addr);


/**
 ****************************************************************************************
 * @brief Disconnect mobile device
 *
 * @param[in] addr           Mobile address
 * @param[in] post_func      post function
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_remote_dev_disconnect_request(const bt_bdaddr_t *addr,ibrt_post_func post_func);


/**
 ****************************************************************************************
 * @brief Disconnect tws link
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_tws_disconnect(void);


/**
 ****************************************************************************************
 * @brief Disconnect ibrt first then disconnect tws link
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_ibrt_tws_disconnect(void);


/**
 ****************************************************************************************
 * @brief Get mobile info in the begin of the map
 *
 * @return pointer of mobile info
 ****************************************************************************************
 */
ibrt_mobile_info_t *app_ibrt_conn_get_mobile_info_ext(void);


/**
 ****************************************************************************************
 * @brief Get mobile info in by addr
 *
 * @param[in] addr       Mobile address
 *
 * @return pointer of mobile info
 ****************************************************************************************
 */
ibrt_mobile_info_t *app_ibrt_conn_get_mobile_info_by_addr(const bt_bdaddr_t *mobile_addr);


/**
 ****************************************************************************************
 * @brief Convert uiRole to string
 *
 * @param[in] uiRole       Mobile address
 *
 * @return pointer of the string
 ****************************************************************************************
 */
const char* app_ibrt_conn_uirole2str(TWS_UI_ROLE_E uiRole);


/**
 ****************************************************************************************
 * @brief Get ibrt-role for the mobile device
 *
 * @param[in] addr       Mobile address
 *
 * @return ibrt-role
 ****************************************************************************************
 */
tws_role_e app_ibrt_conn_tws_role_get_request(const bt_bdaddr_t *p_bd_addr);


/**
 ****************************************************************************************
 * @brief Get ui-role
 *
 * @return ui-role
 ****************************************************************************************
 */
TWS_UI_ROLE_E app_ibrt_conn_get_ui_role(void);


/**
 ****************************************************************************************
 * @brief Get mobile link mode
 *
 * @param[in] addr       Mobile address
 *
 * @return mobile link mode
 ****************************************************************************************
 */
ibrt_link_mode_e app_ibrt_conn_get_mobile_mode(void* addr);

bool app_ibrt_conn_is_mobile_connecting(const bt_bdaddr_t* addr);

/**
 ****************************************************************************************
 * @brief Get mobile connection state
 *
 * @param[in] addr       Mobile address
 *
 * @return mobile connection state
 ****************************************************************************************
 */
ibrt_conn_acl_state app_ibrt_conn_get_mobile_conn_state(const bt_bdaddr_t* addr);

/**
 ****************************************************************************************
 * @brief Get local bt name
 *
 * @param[out] name_buf  bt name
 * @param[in] max_size   the maximum name length
 *
 * @return mobile connection state
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_get_local_name(uint8_t* name_buf, uint8_t max_size);

/**
 ****************************************************************************************
 * @brief check if both side earbuds address is null
 ****************************************************************************************
 */
void app_ibrt_conn_reg_super_handle_connect_req_cb(ibrt_ui_reject_connect_req_func cb);

/**
 ****************************************************************************************
 * @brief handle cbk when mobile ssp fail
 ****************************************************************************************
 */
void app_ibrt_conn_set_mobile_ssp_fail_cb(ibrt_ui_mobile_ssp_failed_func cb);

/**
 ****************************************************************************************
 * @brief check if both side earbuds address is null
 ****************************************************************************************
 */
bool app_ibrt_conn_peer_earbuds_addr_null();

/**
 ****************************************************************************************
 * @brief get snoop link count
 ****************************************************************************************
 */
uint8_t app_ibrt_conn_get_snoop_connected_link_num(void);

/**
 ****************************************************************************************
 * @brief snoop state changed
 *
 * @param[out] remote device address
 * @param[out] snoop status
 *
 * @return void
 ****************************************************************************************
 */
void app_ibrt_conn_on_snoop_state_changed(const bt_bdaddr_t *addr,uint8_t snoop_connected);


/**
 ****************************************************************************************
 * @brief get connected mobile device handle
 *
 * @param[int] mobile device address,if input paramter is null,return connected device handle
 *
 * @return handle
 ****************************************************************************************
 */
uint16_t app_ibrt_conn_get_mobile_handle(const bt_bdaddr_t* addr);

/**
 ****************************************************************************************
 * @brief register callback for disallow sdk do role switch
 *
 * @param[in] cb
 *
 * @return void
 ****************************************************************************************
 */
 void app_ibrt_conn_reg_disallow_role_switch_callback(tws_switch_disallow_cb cb);

 /**
  ****************************************************************************************
 * @brief app_ibrt_conn_pscan_setting
 *
 * @param[in] scan_window_slots  the window of page scan(Unit slot)
 * @param[in] scan_interval_slots  the interval of page scan(Unit slot)
 *
 * @return ibrt_status_t
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_pscan_setting(uint16_t scan_window_slots, uint16_t scan_interval_slots);

/**
 ****************************************************************************************
* @brief app_ibrt_conn_startup_mobile_sm
*
* @param[in] mobile_sm_ptr   special mobile state machine
* @param[in] direction       connection direction
* @param[in] request_connect indicate

* @return ibrt_status_t
****************************************************************************************
*/
void app_ibrt_conn_startup_mobile_sm(uint32_t mobile_sm_ptr);

/**
 ****************************************************************************************
 * @brief initiate role switch without mobile link
 *
 * @param[in] msDelay       Specify the delay of the role switch
 ****************************************************************************************
 */
void app_ibrt_conn_enhanced_rs(uint16_t msDelay);

/**
 ****************************************************************************************
 * @brief initiate role switch by btc, when ibrt connected
 *
 * @param[in] addr       Mobile addr
 ****************************************************************************************
 */

void app_ibrt_conn_btc_role_switch(const bt_bdaddr_t *addr);

 /**
  ****************************************************************************************
 * @brief Handling RS done event
 *
 * @param[in] deviceId       Device id
 ****************************************************************************************
 */
void app_ibrt_conn_switch_to_ibrt_slave_process(uint8_t deviceId);

/**
 ****************************************************************************************
 * @brief for codec error handle, switch to ibrt slave
 *
 ****************************************************************************************
 */
void app_ibrt_conn_codec_error_handler(uint32_t streamId, uint32_t streamType);

/**
 ****************************************************************************************
 * @brief report connection complete event to upper layer
 *
 ****************************************************************************************
 */
void app_ibrt_conn_register_connection_complete_callbcak(connection_complete_callback cb);

/**
 ****************************************************************************************
 * @brief report ll monitor event to upper layer
 *
 ****************************************************************************************
 */
void app_hci_vender_register_ll_monitor_handle(void (*fn)(uint16_t handle, uint8_t ser));

#if BLE_AUDIO_ENABLED
ibrt_status_t app_ibrt_conn_notify_bt_nv_changed(bt_bdaddr_t* mobile_addr);

ibrt_status_t app_ibrt_conn_sync_smp_remote_device(uint8_t* buf,uint16_t len );

#endif
/**
 ****************************************************************************************
 * @brief Check whether the passed-in value is equal to mobile connection handle
 *
 * @param[in] connhandle
 ****************************************************************************************
 */
bool app_ibrt_conn_is_mobile_connhandle(uint16_t connhandle);

/**
 ****************************************************************************************
 * @brief Check whether the passed-in value is equal to tws connection handle
 *
 * @param[in] connhandle
 ****************************************************************************************
 */
bool app_ibrt_conn_is_tws_connhandle(uint16_t connhandle);

bool app_ibrt_conn_is_ui_master();

bool app_ibrt_conn_is_ui_slave();

bool app_ibrt_conn_user_rs_task_set(uint16_t life_cycle);

void app_ibrt_conn_user_rs_task_clr(void);

void app_ibrt_conn_register_ui_profile_hook_callbcak(const app_ibrt_profiles_hook_cb *cb);

void app_ibrt_conn_ui_profile_hook_callbcak_handle(uint64_t profile, const bt_bdaddr_t *addr, void *param);

#ifdef __cplusplus
}
#endif

#endif

#endif /*__APP_TWS_IBRT_CORE_API_H__*/
