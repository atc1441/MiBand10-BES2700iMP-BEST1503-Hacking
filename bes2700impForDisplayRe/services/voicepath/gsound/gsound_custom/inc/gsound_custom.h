/***************************************************************************
*
*Copyright 2015-2019 BES.
*All rights reserved. All unpublished rights reserved.
*
*No part of this work may be used or reproduced in any form or by any
*means, or stored in a database or retrieval system, without prior written
*permission of BES.
*
*Use of this work is governed by a license granted by BES.
*This work contains confidential and proprietary information of
*BES. which is protected by copyright, trade secret,
*trademark and other intellectual property rights.
*
****************************************************************************/

#ifndef __GSOND_CUSTOM_H__
#define __GSOND_CUSTOM_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****************************header include********************************/
#include "gsound_target.h"
#include "bluetooth_bt_api.h"
#include "app_dip.h"

/******************************macro defination*****************************/
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

/******************************type defination******************************/
enum {
    GSOUND_NOT_INIT  = 0,
    GSOUND_INIT_DONE = 1,
    GSOUND_INIT_STATE_NUM,
};

enum {
    CHANNEL_DISCONNECTED = 0,
    CHANNEL_CONNECTED    = 1,
    CHANNEL_CONNECT_STATE_NUM,
};

enum {
    GSOUND_MASTER  = 0,
    GSOUND_SLAVE   = 1,
    GSOUND_FREEMAN = 2,
    GSOUND_ROLE_NUM,
};

enum {
    CONNECTION_NULL = 0,
    CONNECTION_BLE  = 1,
    CONNECTION_BT   = 2,
    CONNECTION_TYPE_NUM,
};

/****************************function declearation**************************/
/*---------------------------------------------------------------------------
 *            gsound_custom_init
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    initialize all of the gsound relabted environment
 *
 * Parameters:
 *    isEnable - true if the gsound service is allowed to enable
 *               false if the gsound service is not allowed to enable
 *
 * Return:
 *    void
 */
void gsound_custom_init(bool isEnable);

/*---------------------------------------------------------------------------
 *            gsound_is_service_enabled
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    get the enable status of gsound service
 *
 * Parameters:
 *    void
 *
 * Return:
 *    true - gsound service is currently enabled
 *    false - gsound service is currently disabled
 */
bool gsound_is_service_enabled(void);

/*---------------------------------------------------------------------------
 *            gsound_set_service_enable_state
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    update the gsound service state saved in gsoundEnv
 *
 * Parameters:
 *    enable - enable state to be set in gsoundEnv
 *             true if gsound service is enabled
 *             flase if gsound service is disabled
 *
 * Return:
 *    void
 */
void gsound_set_service_enable_state(bool enable);

/*---------------------------------------------------------------------------
 *            gsound_get_connected_mobile_type
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    get the connected mobile type
 *
 * Parameters:
 *    void
 *
 * Return:
 *    MOBILE_CONNECT_IOS - earbud connected with ios device
 *    MOBILE_CONNECT_ANDROID - earbud connected with android device
 */
MOBILE_CONN_TYPE_E gsound_get_connected_mobile_type();


/*---------------------------------------------------------------------------
 *            gsound_mobile_type_get_callback
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    set the connected mobile type
 *
 * Parameters:
 *    type -  device type of connected with earbud
 *
 *
 * Return:
 *    void
 */
void gsound_mobile_type_get_callback(MOBILE_CONN_TYPE_E type);

/*---------------------------------------------------------------------------
 *            gsound_is_bt_init_done
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    get bt init done state saved in gsoundEnv
 *
 * Parameters:
 *    void
 *
 * Return:
 *    true - gsound related bt initialization is done
 */
bool gsound_is_bt_init_done(void);

/*---------------------------------------------------------------------------
 *            gsound_set_bt_init_state
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    update gsound bt init done state saved in gsoundEnv
 *
 * Parameters:
 *    state - gsound bt init state to be set in gsoundEnv
 *
 * Return:
 *    void
 */
void gsound_set_bt_init_state(bool state);

/*---------------------------------------------------------------------------
 *            gsound_get_bt_connect_state
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    get gsound related spp channel connection status
 *
 * Parameters:
 *    void
 *
 * Return:
 *    gsound related spp channel connection status
 */
uint8_t gsound_get_bt_connect_state(void);

/*---------------------------------------------------------------------------
 *            gsound_get_rfcomm_connect_state
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    get rfcomm connect state
 *
 * Parameters:
 *    chnl - gsound channel
 *
 * Return:
 *    0 - channel is not connected
 *    1 - channel is connected
 */
uint8_t gsound_get_rfcomm_connect_state(uint8_t chnl);

/*---------------------------------------------------------------------------
 *            gsound_set_rfcomm_connect_state
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    set gsound related spp channel connection status
 *
 * Parameters:
 *    chnl - channel type of which connection to be updated
 *    state - true if connected
 *            false if disconnected
 *
 * Return:
 *    void
 */
void gsound_set_rfcomm_connect_state(uint8_t chnl, bool state);

/*---------------------------------------------------------------------------
 *            gsound_is_ble_init_done
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    get gsound related ble init state
 *
 * Parameters:
 *    void
 *
 * Return:
 *    true - gsound related ble init done
 *    flase - gsound related ble init not accomplished
 */
bool gsound_is_ble_init_done(void);

/*---------------------------------------------------------------------------
 *            gsound_set_ble_init_state
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    update gsound related ble init state
 *
 * Parameters:
 *    state - true if gsound related ble init done
 *            false if gsound related ble init not accomplished
 * Return:
 *    void
 */
void gsound_set_ble_init_state(bool state);

/*---------------------------------------------------------------------------
 *            gsound_get_ble_connected_state
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    get gsound ble channel connection state
 *
 * Parameters:
 *    void
 *
 * Return:
 *    gsound ble channel connection state
 */
uint8_t gsound_get_ble_connect_state(void);

/*---------------------------------------------------------------------------
 *            gsound_get_ble_channel_connected_state
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    get gsound specific ble channel connection state
 *
 * Parameters:
 *    chnl - ble channel
 *
 * Return:
 *    0 - channel not connected
 *    1 - channel connected
 */
uint8_t gsound_get_ble_channel_connect_state(uint8_t chnl);

/*---------------------------------------------------------------------------
 *            gsound_set_ble_connect_state
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    update the gsound ble channel connection state
 *
 * Parameters:
 *    chnl - gsound related ble channel of which connection state to be updated
 *    state - true if connected
 *            false if disconnected
 *
 * Return:
 *    void
 */
void gsound_set_ble_connect_state(uint8_t chnl, bool state);

/*---------------------------------------------------------------------------
 *            gsound_custom_get_connection_path
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    get gsound connection path, see @CONNECTION_NULL to get more info
 *
 * Parameters:
 *    void
 *
 * Return:
 *    CONNECTION_NULL - no connection
 *    CONNECTION_BLE - connected with BLE
 *    CONNECTION_BT - connected with BT
 */
uint8_t gsound_custom_get_connection_path(void);

/*---------------------------------------------------------------------------
 *            gsound_is_role_switch_ongoing
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    get the gsound role switch state
 *
 * Parameters:
 *    void
 *
 * Return:
 *    true - libgsound is role switching
 *    false - libgsound is not role switching
 */
bool gsound_is_role_switch_ongoing(void);

/*---------------------------------------------------------------------------
 *            gsound_set_role_switch_state
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    update gsound role switch sate
 *
 * Parameters:
 *    state - true if libgsound is going to do role switch
 *            false if libgsound role switch is done
 *
 * Return:
 *    void
 */
void gsound_set_role_switch_state(bool state);

/*---------------------------------------------------------------------------
 *            gsound_is_bt_connected
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    get bluetooth connection state
 *
 * Parameters:
 *    void
 *
 * Return:
 *    true - bt is connected with mobile
 *    false - bt is not connected with mobile
 */
bool gsound_is_bt_connected(void);

/*---------------------------------------------------------------------------
 *            gsound_set_bt_connection_state
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    set gsound bt connection state
 *
 * Parameters:
 *    state - connection state of bluetooth
 *
 * Return:
 *    void
 */
void gsound_set_bt_connection_state(bool state);

/*---------------------------------------------------------------------------
 *            gsound_set_connected_bd_addr
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    update the connected bd addr saved in gsoundEnv
 *
 * Parameters:
 *    addr - connected bd addr
 *
 * Return:
 *    void
 */
void gsound_set_connected_bd_addr(uint8_t *addr);

/*---------------------------------------------------------------------------
 *            gsound_get_connected_bd_addr
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    get gsound connected bluetooth device address
 *
 * Parameters:
 *    void
 *
 * Return:
 *    address pointer of the connected bluetooth device
 */
uint8_t *gsound_get_connected_bd_addr(void);

void gsound_set_connected_ble_addr(uint8_t *addr);

uint8_t *gsound_get_connected_ble_addr(void);

#ifdef IBRT
/*---------------------------------------------------------------------------
 *            gsound_get_role
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    get the current role saved in gsoundEnv
 *
 * Parameters:
 *    void
 *
 * Return:
 *    current role saved in gsoundEnv
 */
uint8_t gsound_get_role(void);

/*---------------------------------------------------------------------------
 *            gsound_set_role
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    update the current role saved in gsoundEnv
 *
 * Parameters:
 *    role - role to be set in gsoundEnv, could be master/slave/freeman
 *
 * Return:
 *    void
 */
void gsound_set_role(uint8_t role);

/*---------------------------------------------------------------------------
 *            gsound_on_system_role_switch_done
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    callback function for gsound when system role switch done
 *
 * Parameters:
 *    newRole - current device role after the switch, could be master/slave/freeman
 *
 * Return:
 *    void
 */
void gsound_on_system_role_switch_done(uint8_t newRole);

/*---------------------------------------------------------------------------
 *            gsound_on_tws_role_updated
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    callback function of gsound when tws role updated
 *
 * Parameters:
 *    newRole - current role after role updated
 *
 * Return:
 *    void
 */
void gsound_on_tws_role_updated(uint8_t newRole);
#endif

/*---------------------------------------------------------------------------
 *            gsound_chnl2str
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    convert the gsound channel type into string
 *
 * Parameters:
 *    chnl - channel type
 *
 * Return:
 *    char* - the string of channel type
 */
const char *gsound_chnl2str(uint8_t chnl);

/**
 * Convert Platform Bluetooth Device Address to GSound address format
 */
/*---------------------------------------------------------------------------
 *            gsound_conv_bdaddr_to_gsound
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *
 *
 * Parameters:
 *    plat_addr - pointer of address in plateform format
 *    gsound_addr - pointer of address in libgsound format
 *
 * Return:
 *    void
 */
void gsound_convert_bdaddr_to_gsound(void const *plat_addr,
                                     GSoundBTAddr *gsound_addr);

/*---------------------------------------------------------------------------
 *            gsound_convert_bdaddr_to_plateform
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    Convert GSound Bluetooth Device Address to Platform address format
 *
 * Parameters:
 *    gsound_addr - address format used for ligsound
 *    plat_addr - address format used for plateform
 *
 * Return:
 *    void
 */
void gsound_convert_bdaddr_to_plateform(GSoundBTAddr const *gsound_addr,
                                        void *plat_addr);

void gsound_custom_connection_state_received_handler(uint8_t *buf);

/*---------------------------------------------------------------------------
 *            gsound_on_bt_link_disconnected
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    callback function of gsound when BT link is disconnected with phone
 *
 * Parameters:
 *    addr - disconnected phone's address
 *
 * Return:
 *    void
 */
void gsound_on_bt_link_disconnected(uint8_t *addr);

/*---------------------------------------------------------------------------
 *            gsound_bt_is_rx_available
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    function used to check if gsound control and data channel available
 *
 * Parameters:
 *    void
 *
 * Return:
 *    true - gsound rx thread is available
 *    false - gsound rx thread is not available
 */
bool gsound_bt_is_rx_available();

/*---------------------------------------------------------------------------
 *            gsound_bt_set_rx_available
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    function used to set gsound control and data channel available
 *
 * Parameters:
 *    true - gsound rx thread is available
 *    false - gsound rx thread is not available
 *
 * Return:
 *    void
 */
void gsound_bt_set_rx_available(bool avail);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __GSOND_CUSTOM_H__ */
