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
#ifndef __BLE_GFPS_COMMON_H__
#define __BLE_GFPS_COMMON_H__
#include "ble_common_define.h"
#ifdef BLE_HOST_SUPPORT
#ifdef GFPS_ENABLED
#ifdef __cplusplus
extern "C" {
#endif

/*
 * DEFINES
 ****************************************************************************************
 */
#define BLE_FASTPAIR_NORMAL_ADVERTISING_INTERVAL (160)
#define BLE_FASTPAIR_FAST_ADVERTISING_INTERVAL   (48)
#define BLE_FASTPAIR_SPOT_ADVERTISING_INTERVAL   (1000)

#define GFPSP_IDX_MAX               (BLE_CONNECTION_MAX)

#define L2CAP_SPSM_GFPS             (0x0081)

#define STATE_UNAVAILABLE               (2)
#define STATE_READY_CONNECT             (1)
#define STATE_NOT_READY_CONNECT         (0)

typedef enum {
    GFPS_BLE_EVT_LE_CONNECTED = 0,
    GFPS_BLE_EVT_LE_DISCONNECTED,
    GFPS_BLE_EVT_L2CAP_CONNECTED,
    GFPS_BLE_EVT_L2CAP_DISCONNECTED,
    GFPS_BLE_EVT_L2CAP_RX_REC,
    GFPS_BLE_EVT_KEY_BASE_PAIRING_CCCD,
    GFPS_BLE_EVT_KEY_BASE_PAIRING_IND,
    GFPS_BLE_EVT_PASS_KEY_CCCD,
    GFPS_BLE_EVT_PASS_KEY_IND,
    GFPS_BLE_EVT_ACCOUNT_KEY_IND,
    GFPS_BLE_EVT_WRITE_NAME_IND,
    GFPS_BLE_EVT_SPOT_WRITE_BEACON_IND,
    GFPS_BLE_EVT_SPOT_BEACON_CCCD,
    GFPS_BLE_EVT_GET_SPOT_VERSION,
    GFPS_BLE_EVT_GEN_SPOT_NOUNCE,
    GFPS_BLE_EVT_GET_SPOT_NOUNCE,
    GFPS_BLE_EVT_GET_MODEL_ID,
    GFPS_BLE_EVT_ADDITIONAL_PASS_KEY_IND,
    GFPS_BLE_EVT_ADDITIONAL_PASS_KEY_CCCD,
    GFPS_BLE_EVT_GET_ADDITIONAL_PASS_KEY,
    GFPS_BLE_EVT_GET_EVENT_STREAM,
} ble_app_gfps_event_e;

typedef struct
{
    uint8_t *pBuf;
    uint16_t len;
} ble_app_gfps_packet_t;

typedef struct
{
    bt_bdaddr_t localAddr;
    bt_bdaddr_t peerAddr;
} ble_app_gfps_connection_t;

typedef struct {
    uint8_t                 conidx;
    ble_app_gfps_event_e    event;
    uint8_t                 *outData;
    uint16_t                outLen;
    union {
        bool                        enabled;
        ble_app_gfps_connection_t   conn;
        ble_app_gfps_packet_t       packet;
    } p;
} ble_app_gfps_event_param_t;

typedef uint8_t (*ble_app_gfps_event_cb)(ble_app_gfps_event_param_t *entry);

void ble_app_gfps_register_event_callback(ble_app_gfps_event_cb func);

#ifdef __cplusplus
}
#endif
#endif /* GFPS_ENABLED */
#endif /* BLE_HOST_SUPPORT */
#endif /* __BLE_GFPS_COMMON_H__ */
