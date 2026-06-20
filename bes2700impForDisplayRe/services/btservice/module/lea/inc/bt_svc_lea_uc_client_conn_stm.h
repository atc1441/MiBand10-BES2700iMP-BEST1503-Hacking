/**
 * @brief Bluetooth Service Low Energy Audio Unicast Client Application Programming Interface
 *
 * @copyright Copyright (c) 2015-20223 BES Technic.
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
 */
#ifndef __BT_SVC_LEA_UC_CLIENT_STM_H__
#define __BT_SVC_LEA_UC_CLIENT_STM_H__

/*****************************header include********************************/
#include "hsm.h"
#include "bt_svc_lea_uc_client_api.h"

/******************************macro defination*****************************/
#ifdef AOB_MOBILE_ENABLED
/******************************type defination******************************/
typedef enum {
    BLE_MOBILE_CONN_DISCONNECTED              = 0,
    BLE_MOBILE_CONN_RECONNECTING              = 1,
    BLE_MOBILE_CONN_RECONNECTING_CANCELED     = 2,
    BLE_MOBILE_CONN_RECONNECTING_FAILURE      = 3,
    BLE_MOBILE_CONN_CONNECTED                 = 4,
    BLE_MOBILE_CONN_DISCONNECTING             = 5,
    BLE_MOBILE_CONN_UNKNOWN                   = 6,
} bt_svc_lea_uc_client_conn_state;

typedef struct
{
    Hsm     super;

    State   idle;
    State   reconnecting;
    State   connected;
    State   disconnecting;
    State   cancel_disconecting;
    State   disconnected;

    uint8_t         link_id;
    osTimerId       ble_mobile_link_moniter_timer_id;
    osTimerDefEx_t  ble_mobile_link_moniter_timer_def;
    uint32_t        link_moniter_timeout_ms;
    bool            used;
    uint8_t         conidx;
    ble_bdaddr_t    address;
    uint8_t         disconn_reson;

    bool entry_connected;
} bt_svc_lea_uc_client_conn_stm_t;

/****************************function declearation**************************/

void bt_svc_lea_uc_client_conn_sm_init(bt_svc_lea_uc_client_conn_stm_t *ble_mobile_sm);

void bt_svc_lea_uc_client_conn_sm_startup(bt_svc_lea_uc_client_conn_stm_t *ble_mobile_sm);

void bt_svc_lea_uc_client_conn_link_send_message(bt_svc_lea_uc_client_conn_stm_t *mobile_instance, BLE_MOBILE_CONNECT_EVENT event, uint32_t param0, uint32_t param1);

bt_svc_lea_uc_client_conn_state bt_svc_lea_uc_client_conn_get_connection_state(bt_svc_lea_uc_client_conn_stm_t* ble_mobile_sm);

void bt_svc_lea_uc_client_conn_sm_timer_init(bt_svc_lea_uc_client_conn_stm_t *ble_mobile_sm);

#endif

#endif

