/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifndef __BES_TILE_API_H__
#define __BES_TILE_API_H__

#ifdef BLE_HOST_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif

#ifdef TILE_DATAPATH

typedef enum {
    BES_BLE_TILE_CONN_EVENT,
    BES_BLE_TILE_DISCONN_EVENT,
    BES_BLE_TILE_CONN_UPDATE_EVENT,
    BES_BLE_TILE_CH_CONN_EVENT,
    BES_BLE_TILE_CH_DISCONN_EVENT,
    BES_BLE_TILE_TX_DATA_DONE_EVENT,
    BES_BLE_TILE_RECEIVE_EVENT,
} BES_BLE_TILE_EVENT_TYPE_E;

typedef struct bes_ble_tile_event_param
{
    /// connection index
    uint8_t  conidx;
    /// gatt event type, see@BES_BLE_TILE_EVENT_TYPE_E
    uint8_t  event_type;
    union {
         /// BES_BLE_TILE_TX_DATA_DONE_EVENT;
         uint8_t result;
         /// BES_BLE_TILE_CONN_EVENT
         /// BES_BLE_TILE_CONN_UPDATE_EVENT,
         struct {
             uint16_t interval;
             uint16_t latency;;
             uint16_t conn_sup_timeout;
         } ble_conn;
         /// BES_BLE_TILE_CH_CONN_EVENT
         /// BES_BLE_TILE_CH_DISCONN_EVENT
        struct {
            uint8_t hdl;
            uint8_t status;
        } channel_conn;
        /// BES_BLE_TILE_RECEIVE_EVENT
        struct {
            uint8_t data_len;
            uint8_t *data;
        } receive;
    }data;
} bes_ble_tile_event_param_t;

typedef void(* bes_ble_tile_event_cb)(bes_ble_tile_event_param_t *param);

void bes_ble_tile_event_cb_reg(bes_ble_tile_event_cb cb);

void bes_ble_tile_send_via_notification(uint8_t conidx, uint8_t* ptrData, uint32_t length);

#endif

#ifdef __cplusplus
}
#endif
#endif /* BLE_HOST_SUPPORT */
#endif /* __BES_TILE_API_H__ */

