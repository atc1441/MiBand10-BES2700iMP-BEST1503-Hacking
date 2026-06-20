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
#ifndef __BLE_AUDIO_STM_EVT_TYPE_H__
#define __BLE_AUDIO_STM_EVT_TYPE_H__

/*****************************header include********************************/
#include "bes_aob_api.h"

/******************************macro defination*****************************/
// 12:15 -- event  prefix
// 0:11 -- event
#define AOB_STM_TWS_EVENT_PREFIX         0x1000
#define AOB_STM_MOBILE_EVENT_PREFIX      0x2000
#define AOB_STM_MUSIC_EVENT_PREFIX       0x3000
#define AOB_STM_CALL_EVENT_PREFIX        0x4000
#define AOB_STM_FUNC_CALL_EVENT_PREFIX   0x5000
#define AOB_STM_VOLUME_CONTROL_PREFIX    0x6000
#define AOB_GAF_EVENT_PREFIX             0x7000
#define AOB_MOBILE_GAF_EVENT_PREFIX      0x8000
#define AOB_STM_EVENT_PREFIX_MASK        0xF000
#define AOB_STM_EVENT_MASK               0x0FFF

/******************************type defination******************************/
typedef enum{
    AOB_TWS_CONNECTED_EVENT                 = 0,
    AOB_TWS_CONNECTING_STARTED_EVENT        = 1,
    AOB_TWS_CONNECTING_STOPPED_EVENT        = 2,
    AOB_TWS_CONNECTING_FAILED_EVENT         = 3,
    AOB_TWS_DISCONNECTED_EVENT              = 4,
    AOB_TWS_CONN_PARAM_UPDATE_REQ_EVENT     = 5,
    AOB_TWS_CONN_PARAM_UPDATE_FAILED_EVENT  = 6,
    AOB_TWS_CONN_PARAM_UPDATE_SUCCESSFUL_EVENT  = 7,

    AOB_TWS_EVENT_NUM_MAX,
} aob_tws_evnet_type_e;

typedef enum{
    AOB_CONNECTING_HANDSHAKED_FAILED        = 0,
    AOB_CONNECTING_CANCELED                 = 1,
    AOB_CONNECTING_TIMEOUT                  = 2,

    AOB_CONNECTING_FAILED_NUM_MAX,
} aob_connecting_failed_reason_e;

typedef struct {
    uint8_t conidx;
    uint8_t peer_bdaddr[BTIF_BD_ADDR_SIZE];
} tws_connected_event_t;

typedef struct {
    aob_connecting_failed_reason_e reason;
} tws_connecting_failed_event_t;

typedef struct {
    uint8_t conidx;
    uint8_t error_code;
} tws_disconnected_event_t;

typedef union {
    tws_connected_event_t connected;
    tws_disconnected_event_t disconnected;
} tws_event_t;

typedef enum{
    AOB_MOBILE_CONNECTED_EVENT              = 0,
    AOB_MOBILE_CONNECT_BOND_EVENT           = 1,    //pairing success
    AOB_MOBILE_CONNECT_ENCRYPT_EVENT        = 2,    //encrypt complete
    AOB_MOBILE_CONNECTING_STARTED_EVENT     = 3,
    AOB_MOBILE_CONNECTING_STOPPED_EVENT     = 4,
    AOB_MOBILE_CONNECTING_FAILED_EVENT      = 5,
    AOB_MOBILE_DISCONNECTED_EVENT           = 6,
    AOB_MOBILE_CONN_PARAM_UPDATE_REQ_EVENT  = 7,
    AOB_MOBILE_CONN_PARAM_UPDATE_FAILED_EVENT       = 8,
    AOB_MOBILE_CONN_PARAM_UPDATE_SUCCESSFUL_EVENT   = 9,

    AOB_MOBILE_EVENT_NUM_MAX,
} aob_mobile_evnet_type_e;

typedef struct {
    uint8_t conidx;
    ble_bdaddr_t peer_bdaddr;
} mobile_connected_event_t;

typedef struct {
    uint8_t conidx;
} mobile_connect_bond_event_t;

typedef struct {
    uint8_t conidx;
    uint8_t pairing_lvl;
} mobile_connect_encrypt_event_t;

typedef struct {
    uint8_t peer_bdaddr[BTIF_BD_ADDR_SIZE];
} mobile_connecting_stopped_event_t;

typedef struct {
    uint8_t conidx;
    uint8_t error_code;
} mobile_disconnected_event_t;

typedef union {
    mobile_connected_event_t connected;
    mobile_connect_bond_event_t connect_bonded;
    mobile_connect_encrypt_event_t connect_encrypted;
    mobile_connecting_stopped_event_t connecting_stopped;
    mobile_disconnected_event_t disconnected;
} mobile_event_t;

/****************************function declearation**************************/

#endif /* __BLE_AUDIO_STM_EVT_TYPE_H__ */

