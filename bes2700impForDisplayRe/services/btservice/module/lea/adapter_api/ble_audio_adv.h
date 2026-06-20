/***************************************************************************
 *
 * Copyright 2024-2034 BES.
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
#ifndef __BLE_AUDIO_ADV_H__
#define __BLE_AUDIO_ADV_H__

// #include "app_tws_ibrt.h"
// #include "app_ui_evt.h"
// #include "app_tws_ibrt_core_type.h"
// #include "app_ui_ipscan_mgr.h"

#ifdef __cplusplus
extern "C" {
#endif
#define BLE_RECONNECT_ADV_DURATION       (7500)
#define BLE_AUDIO_ADV_DURATION           (60000)
#define BLE_AUDIO_ADV_FOREVER            (0)
#define BLE_AUDIO_ADV_MAX_ADDR_NUM       (6)

typedef enum {
    BLE_AUDIO_ADV_EVT_NONE = 0,
    /// ADV duration timeout
    BLE_AUDIO_ADV_EVT_DURATION_TIMEOUT,
    /// ADV force stratage
    BLE_AUDIO_ADV_EVT_FORCE_STOP,
    BLE_AUDIO_ADV_EVT_FORCE_START,
    /// PAIRING ENTER
    BLE_AUDIO_ADV_EVT_ENTER_PAIRING_MODE,
    /// TWS CONNECT
    BLE_AUDIO_ADV_EVT_TWS_CONNECT_SIG,
    /// CONNECTED
    BLE_AUDIO_ADV_EVT_BLE_ACL_CONNECTED,
    /// DISCONNECT
    BLE_AUDIO_ADV_EVT_IBRT_DISCONNECTED,
    BLE_AUDIO_ADV_EVT_BT_ACL_DISCONNECTED,
    BLE_AUDIO_ADV_EVT_LE_ACL_DISCONNECTED,
    /// SMP over BREDR IA exechanged
    BLE_AUDIO_ADV_EVT_SMP_IA_EXECHANGED,
    /// SMP over BREDR IA received from peer
    BLE_AUDIO_ADV_EVT_SMP_IA_RECEIVED,
    /// LEA mobile connected received from peer
    BLE_AUDIO_ADV_EVT_MOB_CONNECTED_RECEIVED,
    /// reconnect mobile
    BLE_AUDIO_ADV_EVT_RECONNECT_MOBILE,
} ble_audio_adv_mgr_evt;

typedef enum {
    LEA_ADV_EVT_NONE = 0,
    LEA_ADV_EVT_RELOAD_COMPLETE,
    LEA_ADV_EVT_RECONNECT_FIRST,
    LEA_ADV_EVT_RECONNECT_SECOND,
    LEA_ADV_EVT_MOBILE_CONNECTED,
    LEA_ADV_EVT_MOBILE_CONNECTING_FAIL,
    LEA_ADV_EVT_MOBILE_DISCONNECTED,
    LEA_ADV_EVT_ENTER_PAIRING_MODE,
    LEA_ADV_EVT_CLOSE,
    LEA_ADV_EVT_REQUEST_START,
    LEA_ADV_EVT_MAX,
} ble_audio_adv_ui_evt;

/**
 ****************************************************************************************
 * @brief BLE audio adv mgr init
 *
 * @param[in] enable_adv            bool, whether allow lea adv
 *
 ****************************************************************************************
 */
void ble_audio_adv_init(bool enable_adv);

/**
 ****************************************************************************************
 * @brief enable/disable lea adv
 *
 * @param[in] adv_enable            bool, whether allow lea adv
 * @return none
 ****************************************************************************************
 */
void ble_audio_set_adv_enable_flag(bool adv_enable);

/**
 ****************************************************************************************
 * @brief User start lea adv, if sdk_lea_adv_enable config set false
 *
 * @param[in] duration              Lea adv duration
 * @param[in] remote                Null for general adv, otherwise, directed adv
 * @param[in] notify_peer           Notify peer to start lea adv
 * @return true                     Start adv success
 * @return false                    Start adv failed
 ****************************************************************************************
 */
bool ble_audio_user_start_lea_adv(uint32_t duration, ble_bdaddr_t *remote, bool notify_peer);

/**
 ****************************************************************************************
 * @brief User stop lea adv, if sdk_lea_adv_enable config set false
 *
 ****************************************************************************************
 */
void ble_audio_user_stop_lea_adv(bool notify_peer);

/**
 ****************************************************************************************
 * @brief Return true if any ble audio device connected
 *
 * @param[in] duration              Lea adv duration
 * @param[in] bt_address            The LE adv addr
 * @return true                     Start adv success
 * @return false                    Start adv failed
 ****************************************************************************************
 */
bool ble_audio_start_ble_connecteable_adv(uint32_t adv_duration, ble_bdaddr_t *bt_address);

/**
 ****************************************************************************************
 * @brief Return true if any ble audio device connected
 *
 ****************************************************************************************
 */
void ble_audio_stop_ble_connecteable_adv(void);

/**
 ****************************************************************************************
 * @brief Return true if any ble audio device connected
 *
 * @param[in] start                  Adv start flag
 * @param[in] timeout                Adv timeout
 * @param[in] remote                 Null for general adv, otherwise, directed adv
 ****************************************************************************************
 */
void ble_audio_adv_evt_handler(bool start, uint32_t timeout, ble_bdaddr_t *remote);

void *ble_audio_adv_add_new_addr(ble_bdaddr_t *addr, bool initiate);

void ble_audio_lea_adv_event_mgr(ble_audio_adv_ui_evt event, ble_bdaddr_t *address, uint32_t param);

#ifdef __cplusplus
}
#endif

#endif /* __BLE_AUDIO_ADV_H__ */
