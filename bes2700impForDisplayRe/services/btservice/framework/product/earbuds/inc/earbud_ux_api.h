/**
 ****************************************************************************************
 *
 * @file earbud_ux_api.h
 *
 * @brief APIs For Customer
 *
 * Copyright 2023-2030 BES.
 *
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
 ****************************************************************************************
*/

#ifndef __EARBUD_UX_API__
#define __EARBUD_UX_API__
#include "cmsis_os2.h"
#include "earbud_profiles_api.h"
#include "app_ibrt_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t eir[240];
} ibrt_if_extended_inquiry_response_t;

#ifdef IBRT_UI
typedef ibrt_link_status_changed_cb_t  APP_IBRT_IF_LINK_STATUS_CHANGED_CALLBACK;
typedef ibrt_vender_event_handler_ind  APP_IBRT_IF_VENDER_EVENT_HANDLER_IND;
#endif

typedef bool (*earbud_ux_new_connection_callback_t)(void);
typedef void (*sco_disconnect_event_callback_t)(uint8_t *addr, uint8_t disonnectReason);
typedef void (*earbud_ux_post_func)(void);
typedef bool (*earbud_ux_tws_switch_disallow_cb)(void);
typedef void (*earbud_ux_remote_name_Callback_t)(const bt_bdaddr_t *current_addr, const uint8_t *devName);

void app_ibrt_if_bt_set_local_dev_name(const uint8_t *dev_name, unsigned char len);

/**
 ****************************************************************************************
 * @brief Get loal bt name
 *
 * @param[out] name_buf        Remote device name
 * @param[in]  max_size            maximum name length
 *
 * @return Execute result status
 ****************************************************************************************
 */
AppIbrtStatus app_ibrt_if_get_local_name(uint8_t* name_buf, uint8_t max_size);

#ifdef IBRT_UI
/**
 ****************************************************************************************
 * @brief Write local bt address into factory section and activate it.
 *
 * @param[in] btAddr                Pointer of the bt local address
 ****************************************************************************************
 */
void app_ibrt_if_write_bt_local_address(uint8_t* btAddr);

/**
 ****************************************************************************************
 * @brief Write local ble address into factory section and activate it.
 *
 * @param[in] btAddr                Pointer of the ble local address
 ****************************************************************************************
 */
void app_ibrt_if_write_ble_local_address(uint8_t* bleAddr);

/**
 ****************************************************************************************
 * @brief Get local bt address.
 *
 * @return Pointer of the bt local address to be returned
 ****************************************************************************************
 */
uint8_t *app_ibrt_if_get_bt_local_address(void);

/**
 ****************************************************************************************
 * @brief Get local ble address.
 *
 * @return Pointer of the ble local address to be returned
 ****************************************************************************************
 */
uint8_t *app_ibrt_if_get_ble_local_address(void);

/**
 ****************************************************************************************
 * @brief Get peer bt address.
 *
 * @return Pointer of the bt peer address to be returned
 ****************************************************************************************
 */
uint8_t *app_ibrt_if_get_bt_peer_address(void);

#endif

void app_ibrt_if_bt_stop_inqury(void);

void app_ibrt_if_bt_start_inqury(void);

void app_ibrt_if_set_extended_inquiry_response(ibrt_if_extended_inquiry_response_t *data);

/**
 ****************************************************************************************
 * @brief Disconnect tws link.
 *
 * @return Execute result status
 ****************************************************************************************
 */
AppIbrtStatus app_ibrt_if_tws_disconnect_request(void);

/**
 * @brief Disconnect all links.
 *
 * @return Execute result status
 ****************************************************************************************
 */
AppIbrtStatus app_ibrt_if_all_disconnect_request(void);

void app_ibrt_if_disconnect_all_bt_connections(void);

/**
 ****************************************************************************************
 * @brief When enable ui, call this function to disconnect the device.
 *
 * @param[in] remote_addr        Remote device address
 ****************************************************************************************
 */
void app_ibrt_if_disconnet_moblie_device(const bt_bdaddr_t* remote_addr);

/**
 ****************************************************************************************
 * @brief When enable ui, call this function to reconnect the device.
 *
 * @param[in] remote_addr        Remote device address
 ****************************************************************************************
 */
void app_ibrt_if_reconnect_moblie_device(const bt_bdaddr_t* addr);

/**
 ****************************************************************************************
 * @brief Disconnect mobile link.
 *
 * @param[in] addr              Address of the mobile to be disconnected
 * @param[in] post_func         Callback function when disconnecting is completed
 *
 * @return Execute result status
 ****************************************************************************************
 */
AppIbrtStatus app_ibrt_if_mobile_disconnect_request(const bt_bdaddr_t *addr,earbud_ux_post_func post_func);

void app_ibrt_if_choice_mobile_connect(const bt_bdaddr_t* bt_addr, uint8_t try_count);

bool app_ibrt_if_is_any_mobile_connected(void);

uint8_t app_ibrt_if_get_connected_remote_dev_count();

/**
 ****************************************************************************************
 * @brief Get current connected mobile count.
 *
 * @return The number of current connected mobile
 ****************************************************************************************
 */
uint8_t app_ibrt_if_get_connected_mobile_count(void);

/**
 ****************************************************************************************
 * @brief Get current connected mobile device count and address list.
 *
 * @param[in] addr_list          Used to return current connected mobile device address list
 *
 * @return The number of current connected mobile
 ****************************************************************************************
 */
uint8_t app_ibrt_if_get_mobile_connected_dev_list(bt_bdaddr_t *addr_list);

/**
 ****************************************************************************************
 * @brief Check if the tws link is connected.
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>Executed successfully
 * <tr><td>False  <td>Executed failed
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_if_is_tws_link_connected(void);

bool app_ibrt_if_is_mobile_connhandle(uint16_t connhandle);

bool app_ibrt_if_is_tws_connhandle(uint16_t connhandle);

void app_ibrt_if_enter_non_signalingtest_mode(void);

void app_ibrt_if_conn_tws_connect_request(bool isInPairingMode, uint32_t timeout);

void app_ibrt_if_conn_remote_dev_connect_request(const bt_bdaddr_t *addr,bool is_incommig_req,bool request_connect, uint32_t timeout);

#ifdef IBRT_UI
bool app_ibrt_if_is_earbud_in_pairing_mode(void);

/**
 ****************************************************************************************
 * @brief Check if freeman mode is enable.
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>Freeman mode is enable
 * <tr><td>False  <td>Freeman mode is disable
 * </table>
 ****************************************************************************************
 */
uint8_t app_ibrt_if_is_in_freeman_mode(void);

/**
 ****************************************************************************************
 * @brief Check if in tws pairing state.
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>In tws pairing state
 * <tr><td>False  <td>Not in tws pairing state
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_if_is_tws_in_pairing_state(void);

void app_ibrt_if_enter_pairing_after_tws_connected(void);

/**
 ****************************************************************************************
 * @brief Get remote device current running event,such as case open/close,reconnect and so on.
 *
 * @param[in] remote_addr        Remote device address
 *
 * @return current running event
 ****************************************************************************************
 */
app_ui_evt_t app_ibrt_if_get_remote_dev_active_event(const bt_bdaddr_t* remote_addr);

/**
 ****************************************************************************************
 * @brief Check current event have already been pushed into the queue
 *
 * @param[in] remote_addr        Remote device address
 * @param[in] event              The event which will be quiry
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>Push event into the queue success
 * <tr><td>False  <td>Push event  into the queue failed
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_if_event_has_been_queued(const bt_bdaddr_t* remote_addr,app_ui_evt_t event);

void app_ibrt_if_register_sco_disconnect_event_callback(sco_disconnect_event_callback_t callback);

void app_ibrt_if_sco_disconnect(uint8_t *addr, uint8_t disconnect_rseason);

void app_ibrt_if_register_link_status_changed_client_callback(APP_IBRT_IF_LINK_STATUS_CHANGED_CALLBACK* cbs);

void app_ibrt_if_register_mgr_status_changed_client_callback(ibrt_mgr_status_changed_cb_t* cbs);

void app_ibrt_if_register_ext_conn_policy_client_callback(ibrt_ext_conn_policy_cb_t* cbs);

void app_ibrt_if_register_vender_handler_ind(APP_IBRT_IF_VENDER_EVENT_HANDLER_IND handler);
#endif
// // +// void app_ibrt_if_register_pairing_mode_callback(APP_IBRT_IF_PAIRING_MODE_CHANGED_CALLBACK* cbs);
// // +
// // +// void app_ibrt_if_register_sw_ui_role_complete_callback(APP_IBRT_SW_UI_ROLE_COMPLETE_CALLBACK* cbs);

void app_ibrt_if_register_is_reject_new_mobile_connection_query_callback(earbud_ux_new_connection_callback_t callback);

void app_ibrt_if_register_is_reject_tws_connection_callback(new_connection_callback_t callback);

/**
 ****************************************************************************************
 * @brief Register user and its handling functions for specific tws information sync.
 *
 * @param[in] id                  User id
 * @param[in] user                Handling functions belonging to the user
 ****************************************************************************************
 */
void app_ibrt_if_register_sync_user(TWS_SYNC_USER_E id, TWS_SYNC_USER_T *user);

/**
 ****************************************************************************************
 * @brief Unregister user.
 *
 * @param[in] id                  User id
 ****************************************************************************************
 */
void app_ibrt_if_deregister_sync_user(TWS_SYNC_USER_E id);

/**
 ****************************************************************************************
 * @brief Get the mobile link's service state
 ****************************************************************************************
 */
bool app_ibrt_if_is_audio_active(uint8_t device_id);

/**
 ****************************************************************************************
 * @brief Get active device's addr
 ****************************************************************************************
 */
bool app_ibrt_if_get_active_device(bt_bdaddr_t* device);

void app_ibrt_if_nvrecord_config_load(void *config);

void app_ibrt_if_nvrecord_update_ibrt_mode_tws(bool status);

int app_ibrt_if_nvrecord_get_latest_mobiles_addr(bt_bdaddr_t *mobile_addr1, bt_bdaddr_t* mobile_addr2);

/**
 ****************************************************************************************
 * @brief Get address list of the history paired mobiles.
 *
 * @param[in] mobile_addr_list       Address list of the history paired mobiles
 * @param[in] count                  Address count pointer
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>Executed successfully
 * <tr><td>False  <td>Executed failed
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_if_nvrecord_get_mobile_addr(bt_bdaddr_t mobile_addr_list[],uint8_t *count);

/**
 ****************************************************************************************
 * @brief Clear all history paired mobile records in the flash.
 ****************************************************************************************
 */
void app_ibrt_if_nvrecord_delete_all_mobile_record(void);

/**
 ****************************************************************************************
 * @brief Get the history paired mobile records.
 *
 * @param[in] nv_record             Record list of the history paired mobiles
 * @param[in] count                 Record count pointer
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>Executed successfully
 * <tr><td>False  <td>Executed failed
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_if_nvrecord_get_mobile_paired_dev_list(nvrec_btdevicerecord *nv_record,uint8_t *count);

/**
 ****************************************************************************************
 * @brief Check if the tws role switch is ongoing.
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>Role switch is ongoing
 * <tr><td>False  <td>Role switch is not ongoing
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_if_is_tws_role_switch_on(void);

/**
 ****************************************************************************************
 * @brief Perform the role switch operation.
 *
 * @return
 *      none
 ****************************************************************************************
 */
void app_ibrt_if_tws_role_switch_request(void);

void app_ibrt_if_customer_role_switch();

void app_ibrt_if_reg_disallow_role_switch_callback(earbud_ux_tws_switch_disallow_cb cb);

/**
 ****************************************************************************************
 * @brief Get role of earbud.
 *
 * @return Role of earbud
 *           MASTER       0
 *           SLAVE        1
 *           UNKNOW       0xff
 ****************************************************************************************
 */
TWS_UI_ROLE_E app_ibrt_if_get_ui_role(void);

/**
 ****************************************************************************************
 * @brief Check if earbud side is left.
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>Earbud side is left
 * <tr><td>False  <td>Earbud side is not left
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_if_is_left_side(void);

/**
 ****************************************************************************************
 * @brief Check if earbud side is right.
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>Earbud side is right
 * <tr><td>False  <td>Earbud side is not right
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_if_is_right_side(void);

/**
 ****************************************************************************************
 * @brief Check if earbud side is unknown.
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>Earbud side is unknown
 * <tr><td>False  <td>Earbud side is not unknown
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_if_unknown_side(void);

/**
 ****************************************************************************************
 * @brief Check the IBRT UI role
 *
 * @return bool
 ****************************************************************************************
 */
bool app_ibrt_if_is_ui_slave(void);

/**
 ****************************************************************************************
 * @brief Check the IBRT UI role
 *
 * @return bool
 ****************************************************************************************
 */
bool app_ibrt_if_is_ui_master(void);


/**
 ****************************************************************************************
 * @brief Check the IBRT NV role
 *
 * @return bool
 ****************************************************************************************
 */
bool app_ibrt_if_is_nv_master(void);


/**
 ****************************************************************************************
 * @brief Convert role value to string.
 *
 * @param[in] uiRole                Role value of earbud
 *
 * @return Role string
 ****************************************************************************************
 */
const char* app_ibrt_if_uirole2str(TWS_UI_ROLE_E uiRole);

btif_connection_role_t app_ibrt_if_get_tws_current_bt_role(void);

#ifdef IBRT_UI

void app_ibrt_if_dump_ui_status();

void app_ibrt_if_change_ui_mode(bool enable_leaudio, bool enable_multipoint, const bt_bdaddr_t *addr);

void app_ibrt_if_config(app_ui_config_t *ui_config);

void app_ibrt_if_event_entry(app_ui_evt_t event);

/**
 ****************************************************************************************
 * @brief Convert event(hex) to string
 *
 * @param[in] type        event type
 *
 * @return string
 ****************************************************************************************
 */
const char* app_ibrt_if_ui_event_to_string(app_ui_evt_t type);

#endif

bt_status_t app_tws_if_ibrt_write_link_policy(const bt_bdaddr_t *p_addr, btif_link_policy_t policy);

/**
 ****************************************************************************************
 * @brief Set mobile link tpoll value
 ****************************************************************************************
 */
void app_ibrt_if_update_mobile_link_qos(uint8_t device_id, uint8_t tpoll_slot);

/**
 ****************************************************************************************
 * @brief Get the tws mtu size
 *
 * @return uint32_t
 ****************************************************************************************
 */
uint32_t app_ibrt_if_get_tws_mtu_size(void);

bool app_ibrt_if_start_ibrt_onprocess(const bt_bdaddr_t *addr);

int app_ibrt_if_config_keeper_clear(void);

int app_ibrt_if_config_keeper_mobile_update(bt_bdaddr_t *addr);

int app_ibrt_if_config_keeper_tws_update(bt_bdaddr_t *addr);

void app_ibrt_if_enable_bluetooth(void);

void app_ibrt_if_disable_bluetooth(void);

void app_ibrt_if_enter_freeman_pairing(void);

void app_ibrt_if_enter_pairing_after_power_on(void);

/**
 ****************************************************************************************
 * @brief Write local tws role and peer device bt address, the TWS pairing will be
 * immediately started.
 *
 * @param[in] role                  The tws role to update
 * @param[in] peerAddr              The peer device bt address to update
 ****************************************************************************************
 */
void app_ibrt_if_start_tws_pairing(ibrt_role_e role, uint8_t* peerAddr);

/**
 ****************************************************************************************
 * @brief Write local tws role and peer device bt address, the TWS pairing won��t be started.
 *
 * @param[in] role                  The tws role to update
 * @param[in] peerAddr              The peer device bt address to update
 ****************************************************************************************
 */
void app_ibrt_if_update_tws_pairing_info(ibrt_role_e role, uint8_t* peerAddr);

void app_ibrt_gma_exchange_ble_key();

// NULL will be returned if the remote device's dip info hasn't been gathered
ibrt_if_pnp_info* app_ibrt_if_get_pnp_info(bt_bdaddr_t *remote);

void app_ibrt_if_set_afh_assess_en(bool status);

/**
 ****************************************************************************************
 * @brief write tws link
 ****************************************************************************************
 */
void app_ibrt_if_write_tws_link(const uint16_t connhandle);

/**
 ****************************************************************************************
 * @brief write btpcm en
 ****************************************************************************************
 */
void app_ibrt_if_write_btpcm_en(bool en);

/**
 ****************************************************************************************
 * @brief Trigger key event.
 ****************************************************************************************
 */
void app_ibrt_if_key_event(enum APP_KEY_EVENT_T event);

/**
 ****************************************************************************************
 * @brief tota printf data to remote device
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Custom handler of the post device reboot
 *
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>Execute successful
 * <tr><td>False  <td>Execute failed
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_if_post_custom_reboot_handler(void);

/**
 ****************************************************************************************
 * @brief Check whether the address is a TWS address
 *
 * @param[in] pBdAddr        6 bytes of bt address
 *
 * @return true if it's tws address, otherwise false
 ****************************************************************************************
 */
bool app_ibrt_if_is_tws_addr(const uint8_t* pBdAddr);

/*****************************************************************************
 Prototype    : callback function interface for customer
 Description  : Register the callback function of reporting the remote device name.
                The callback function will be triggered when hfp connected and a2dp connected

 Input        : callback function designed by customer
 Output       : None
 Return Value :

 History        :
 Date         : 2022/10/24
 Author       : bestechnic
 Modification : Add an interface
*****************************************************************************/
void app_ibrt_if_register_report_remoteDevice_name_callback(earbud_ux_remote_name_Callback_t callback);

/*****************************************************************************
 Description  : Set the protection time for the user program,
                during which the earphones will not be tws switch

 Input        : None
 Output       : None
 Return Value :

 History        :
 Date         : 2024/12/13
 Author       : bestechnic
 Modification : Add an interface
*****************************************************************************/
bool app_ibrt_if_user_rs_task_set(uint16_t life_cycle);

/*****************************************************************************
 Description  : Clear the protection for the user program,
                during which the earphones will can tws switch

 Input        : None
 Output       : None
 Return Value :

 History        :
 Date         : 2024/12/13
 Author       : bestechnic
 Modification : Add an interface
*****************************************************************************/
void app_ibrt_if_user_rs_task_clr(void);

uint8_t app_ibrt_if_support_max_links();

void app_ibrt_if_set_multipoint_mode(multipoint_mode_t mode);

/*****************************************************************************
 Description  : reconfig lea links and dul mode devices num of max support

 Input        : uint8_t lea_max_num          lea links num of max support
                uint8_t dul_max_num          dul mode devices num of max support
 Output       : bool   true:config success   false:config fail
 
 Return Value :

 History        :
 Date         : 2024/12/13
 Author       : bestechnic
 Modification : Add an interface
*****************************************************************************/
bool app_ibrt_if_change_lea_max_links(uint8_t lea_max_num, uint8_t dul_max_num);

/*****************************************************************************
 Description  : config devices num of max support

 Input        : multipoint_mode_t mode              SINGLE_POINT_MODE = 1,
                                                    DOUBLE_POINT_MODE,
                                                    TRIPLE_POINT_MODE,
                const bt_bdaddr_t reserved_addrs[]  the addr of reserved device
                int reserved_count                  the num of reserved device
   
 Output       : bool   true:config success   false:config fail
 Return Value :

 History        :
 Date         : 2024/12/13
 Author       : bestechnic
 Modification : Add an interface
*****************************************************************************/
bool app_ibrt_if_change_support_max_links_ext(multipoint_mode_t mode, const bt_bdaddr_t reserved_addrs[], int reserved_count);

#ifdef __cplusplus
}
#endif

#endif
