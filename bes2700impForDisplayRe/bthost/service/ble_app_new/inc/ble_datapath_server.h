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
#ifndef __BLE_DATAPATH_SERVER_H__
#define __BLE_DATAPATH_SERVER_H__
#include "ble_dp_common.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifdef CFG_APP_DATAPATH_SERVER
#define BLE_INVALID_CONNECTION_INDEX    0xFF

#define HIGH_SPEED_BLE_CONNECTION_INTERVAL_MIN_IN_MS        20
#define HIGH_SPEED_BLE_CONNECTION_INTERVAL_MAX_IN_MS        30
#define HIGH_SPEED_BLE_CONNECTION_SUPERVISOR_TIMEOUT_IN_MS  2000
#define HIGH_SPEED_BLE_CONNECTION_SLAVELATENCY              0

#define LOW_SPEED_BLE_CONNECTION_INTERVAL_MIN_IN_MS         400
#define LOW_SPEED_BLE_CONNECTION_INTERVAL_MAX_IN_MS         500
#define LOW_SPEED_BLE_CONNECTION_SUPERVISOR_TIMEOUT_IN_MS   2000
#define LOW_SPEED_BLE_CONNECTION_SLAVELATENCY               0

struct app_datapath_server_env_tag
{
    struct
    {
        uint8_t connectionIndex;
        uint8_t isNotificationEnabled;
        uint8_t recv_cmd_latest;
    } con_info[BLE_CONNECTION_MAX];
};

typedef void(*app_datapath_server_tx_done_t)(void);
typedef void(*app_datapath_server_data_received_callback_func_t)(uint8_t *p_buff, uint16_t bufLength);
typedef void(*app_datapath_server_disconnected_done_t)(uint8_t conidx);
typedef void(*app_datapath_server_connected_done_t)(uint8_t conidx);
typedef void(*app_datapath_server_mtuexchanged_done_t)(uint8_t conidx, uint16_t mtu);
typedef uint32_t(*app_datapath_server_role_switch_callback_t)(uint8_t conidx, bool is_restore, uint8_t *buf, uint16_t buf_len);

void ble_datapath_server_init(void);
void app_datapath_server_register_event_callback(app_datapath_event_cb cb);
void app_datapath_server_send_data_via_notification(uint8_t conidx, uint8_t* data, uint32_t len);
void app_datapath_server_send_data_via_indication(uint8_t conidx, uint8_t* data, uint32_t len);
void app_datapath_server_register_event_cb(app_datapath_event_cb callback);
void app_datapath_server_register_le_rs_callback(app_datapath_server_role_switch_callback_t callback);
void app_datapath_server_register_rx_done(app_datapath_server_data_received_callback_func_t callback);
#endif

#ifdef TILE_DATAPATH
typedef enum {
    APP_BLE_TILE_CONN_EVENT,
    APP_BLE_TILE_DISCONN_EVENT,
    APP_BLE_TILE_CONN_UPDATE_EVENT,
    APP_BLE_TILE_CH_CONN_EVENT,
    APP_BLE_TILE_CH_DISCONN_EVENT,
    APP_BLE_TILE_TX_DATA_DONE_EVENT,
    APP_BLE_TILE_RECEIVE_EVENT,
} APP_BLE_TILE_EVENT_TYPE_E;

typedef struct app_ble_tile_event_param {
    uint8_t  conidx;
    uint8_t  event_type; // APP_BLE_TILE_EVENT_TYPE_E
    union {
    uint8_t result; // APP_BLE_TILE_TX_DATA_DONE_EVENT
    struct {
        uint16_t interval;
        uint16_t latency;;
        uint16_t conn_sup_timeout;
    } ble_conn_param;
    struct { // APP_BLE_TILE_CH_CONN_EVENT APP_BLE_TILE_CH_DISCONN_EVENT
        uint8_t hdl;
        uint8_t status;
    } channel_conn;
    struct {
        uint8_t data_len;
        uint8_t *data;
    } receive;
    } data;
} app_ble_tile_event_param_t;

typedef void(* app_ble_tile_event_cb)(app_ble_tile_event_param_t *param);

void ble_tile_init(void);
void app_tile_event_cb_reg(app_ble_tile_event_cb cb);
void app_tile_send_ntf(uint8_t conidx, uint8_t* data, uint32_t len);
#endif

#ifdef __cplusplus
}
#endif
#endif /* __BLE_DATAPATH_SERVER_H__ */
