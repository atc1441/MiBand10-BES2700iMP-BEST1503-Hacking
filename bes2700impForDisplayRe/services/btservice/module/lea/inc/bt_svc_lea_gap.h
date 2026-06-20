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
#ifndef __BT_SVC_LEA_GAP_H__
#define __BT_SVC_LEA_GAP_H__

// #include "app_tws_ibrt.h"
// #include "app_ui_evt.h"
// #include "app_tws_ibrt_core_type.h"
// #include "app_ui_ipscan_mgr.h"

#define BLE_RECONNECT_ADV_DURATION       (7500)
#define BLE_AUDIO_ADV_DURATION           (60000)
#define BLE_AUDIO_ADV_FOREVER            (0)
#define BLE_AUDIO_ADV_MAX_ADDR_NUM       (6)

typedef enum {
    BT_SVC_LEA_GAP_ADV_EVT_NONE = 0,
    BT_SVC_LEA_GAP_ADV_EVT_RELOAD_COMPLETE,
    BT_SVC_LEA_GAP_ADV_EVT_RECONNECT_FIRST,
    BT_SVC_LEA_GAP_ADV_EVT_RECONNECT_SECOND,
    BT_SVC_LEA_GAP_ADV_EVT_MOBILE_CONNECTED,
    BT_SVC_LEA_GAP_ADV_EVT_MOBILE_CONNECTING_FAIL,
    BT_SVC_LEA_GAP_ADV_EVT_MOBILE_DISCONNECTED,
    BT_SVC_LEA_GAP_ADV_EVT_ENTER_PAIRING_MODE,
    BT_SVC_LEA_GAP_ADV_EVT_CLOSE,
    BT_SVC_LEA_GAP_ADV_EVT_REQUEST_START,
    BT_SVC_LEA_GAP_ADV_EVT_MAX,
} bt_svc_lea_gap_adv_ui_evt;

typedef enum {
    BT_SVC_LEA_GAP_NORMAL_STOP = 0,
    BT_SVC_LEA_GAP_USER_STOP,
    BT_SVC_LEA_GAP_CLOSE_BOX_STOP,
    BT_SVC_LEA_GAP_STOP_MAX,
} bt_svc_lea_gap_adv_stop_reason_t;

/**
 ****************************************************************************************
 * @brief BLE audio adv mgr init
 *
 * @param[in] adv_enable            bool, whether allow lea adv
 *
 ****************************************************************************************
 */
void bt_svc_lea_gap_adv_init(bool adv_enable);

/**
 ****************************************************************************************
 * @brief enable/disable lea adv
 *
 * @param[in] adv_enable            bool, whether allow lea adv
 * @return none
 ****************************************************************************************
 */
void bt_svc_lea_gap_set_adv_enable_flag(bool adv_enable);

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
bool bt_svc_lea_gap_user_start_lea_adv(uint32_t duration, ble_bdaddr_t *remote, bool notify_peer);

/**
 ****************************************************************************************
 * @brief User stop lea adv, if sdk_lea_adv_enable config set false
 *
 ****************************************************************************************
 */
void bt_svc_lea_gap_user_stop_lea_adv(bool notify_peer);

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
bool bt_svc_lea_gap_start_ble_connecteable_adv(uint32_t adv_duration, ble_bdaddr_t *bt_address);

/**
 ****************************************************************************************
 * @brief Return true if any ble audio device connected
 *
 ****************************************************************************************
 */
void bt_svc_lea_gap_stop_ble_connecteable_adv(void);

/**
 ****************************************************************************************
 * @brief Return true if any ble audio device connected
 *
 * @param[in] start                  Adv start flag
 * @param[in] timeout                Adv timeout
 * @param[in] remote                 Null for general adv, otherwise, directed adv
 ****************************************************************************************
 */
void bt_svc_lea_gap_adv_evt_handler(bool start, uint32_t timeout, ble_bdaddr_t *remote);

void *bt_svc_lea_gap_adv_add_new_addr(ble_bdaddr_t *addr, bool initiate);

void bt_svc_lea_gap_adv_event_mgr(bt_svc_lea_gap_adv_ui_evt event, ble_bdaddr_t *address, uint32_t param);

#endif /* __BLE_AUDIO_ADV_H__ */
