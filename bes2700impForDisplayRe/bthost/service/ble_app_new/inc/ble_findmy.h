/***************************************************************************
 *
 * Copyright 2024-2029 BES.
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
#ifndef __BLE_FINDMY_H__
#define __BLE_FINDMY_H__
#include "gatt_service.h"
#include "app_ble.h"
#ifdef __cplusplus
extern "C" {
#endif

#define FINDMY_PREF_MTU             (512)
#define FINDMY_SERVICE_UUID         0xFD44

typedef enum {
    BLE_FINDMY_SRV_PAIRING_CTRL = 0,
    BLE_FINDMY_SRV_CONFIG_CTRL,
    BLE_FINDMY_SRV_NONOWNER_CTRL,
    BLE_FINDMY_SRV_OWNER_CTRL,
    BLE_FINDMY_SRV_DEBUG_CTRL,
} ble_findmy_services_e;

typedef enum {
    BLE_FINDMY_EVT_LE_CONNECTED = 0,
    BLE_FINDMY_EVT_LE_DISCONNECTED,
    BLE_FINDMY_EVT_LE_ENCRYPTED,
    BLE_FINDMY_EVT_LE_MTU_EXC_DONE,
    BLE_FINDMY_EVT_LE_PAIR_CMP,
    BLE_FINDMY_EVT_PAIR_CTRL_IND_CFM, //5
    BLE_FINDMY_EVT_CONFIG_CTRL_IND_CFM,
    BLE_FINDMY_EVT_NONOWNER_CTRL_IND_CFM,
    BLE_FINDMY_EVT_OWNER_CTRL_IND_CFM,
    BLE_FINDMY_EVT_DEBUG_CTRL_IND_CFM,
    BLE_FINDMY_EVT_RX_REC_PAIR_CTRL, //10
    BLE_FINDMY_EVT_RX_REC_PAIR_CTRL_CCCD,
    BLE_FINDMY_EVT_RX_REC_CFG_CTRL,
    BLE_FINDMY_EVT_RX_REC_CFG_CTRL_CCCD,
    BLE_FINDMY_EVT_RX_REC_NON_OWNER_CTRL,
    BLE_FINDMY_EVT_RX_REC_NON_OWNER_CTRL_CCCD, // 15
    BLE_FINDMY_EVT_RX_REC_OWNER_CTRL,
    BLE_FINDMY_EVT_RX_REC_OWNER_CTRL_CCCD,
    BLE_FINDMY_EVT_RX_REC_DEBUG_CTRL,
    BLE_FINDMY_EVT_RX_REC_DEBUG_CTRL_CCCD,
    BLE_FINDMY_EVT_RX_READ_PROC_DATA, // 20
    BLE_FINDMY_EVT_RX_READ_MANU_NAME,
    BLE_FINDMY_EVT_RX_READ_MODEL_NAME,
    BLE_FINDMY_EVT_RX_READ_CATEGORY,
    BLE_FINDMY_EVT_RX_READ_CAPABILITY,
    BLE_FINDMY_EVT_RX_READ_FW_VERSION, // 25
    BLE_FINDMY_EVT_RX_READ_FD_VERSION,
    BLE_FINDMY_EVT_RX_READ_BATTERY_TYPE,
    BLE_FINDMY_EVT_RX_READ_BATTERY_LEVEL,
} ble_findmy_event_e;

typedef struct {
    uint8_t productData[8];
    char manuName[64];
    char modelName[64];
    uint8_t category[8];
    uint32_t capability;
    uint32_t fwVer;
    uint32_t findmyVer;
    uint8_t batteryType;
    uint8_t batteryLevel;
} ble_findmy_dev_info_t;

typedef struct {
    uint8_t               conidx;
    ble_findmy_event_e    event;
    uint8_t               reason;
    bt_bdaddr_t           peerAddr;
    uint8_t               ltk[16];
    uint16_t              mtu;
    uint8_t               *pBuf;
    uint32_t              len;
    uint8_t               *outBuf;
    uint32_t              outLen;
} ble_findmy_event_param_t;

typedef uint8_t (*ble_findmy_event_cb)(ble_findmy_event_param_t *entry);

void ble_findmy_register_event_callback(ble_findmy_event_cb func);

bt_status_t ble_findmy_send_pairing_data(uint8_t conidx, const uint8_t *value, uint16_t len);

bt_status_t ble_findmy_send_config_data(uint8_t conidx, const uint8_t *value, uint16_t len);

bt_status_t ble_findmy_send_nonowner_data(uint8_t conidx, const uint8_t *value, uint16_t len);

bt_status_t ble_findmy_send_paired_owner_data(uint8_t conidx, const uint8_t *value, uint16_t len);

bt_status_t ble_findmy_send_debug_data(uint8_t conidx, const uint8_t *value, uint16_t len);

void ble_findmy_init(app_ble_adv_activity_func advFunc, ble_findmy_event_cb evtCb, ble_findmy_dev_info_t *devInfo);

void ble_findmy_enter_paired_mode(uint8_t conidx);

void ble_findmy_notify_pair_complete_handler(uint8_t conidx, uint8_t *addr);

#ifdef __cplusplus
    }
#endif

#endif /* __BLE_GFPS_H__ */
