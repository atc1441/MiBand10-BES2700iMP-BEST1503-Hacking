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
#ifndef __BES_DP_API_H__
#define __BES_DP_API_H__
#include "ble_dp_common.h"
#ifdef BLE_HOST_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif

#define BTIF_BLE_APP_DATAPATH_SERVER
#define BTIF_BLE_APP_DATAPATH_CLIENT

#if (defined(BES_OTA) || defined(BES_OTA_BASIC))&& !defined(OTA_OVER_TOTA_ENABLED)
typedef enum {
    BES_BLE_OTA_CCC_CHANGED = 0,
    BES_BLE_OTA_DISCONN,
    BES_BLE_OTA_RECEVICE_DATA,
    BES_BLE_OTA_MTU_UPDATE,
    BES_BLE_OTA_SEND_DONE,
} BES_BLE_OTA_EVENT_TYPE_E;

typedef struct {
    /// tota event type, see@BES_BLE_OTA_EVENT_TYPE_E
    uint8_t event_type;
    uint8_t conidx;
    uint16_t connhdl;
    union {
       // BES_BLE_OTA_CCC_CHANGED
       uint8_t ntf_en;
       // BES_BLE_OTA_MTU_UPDATE
       uint16_t mtu;
       // BES_BLE_OTA_SEND_DONE
       uint8_t status;
       // BES_BLE_OTA_RECEVICE_DATA
       struct {
           uint16_t data_len;
           uint8_t *data;
       } receive_data;
    } param;
} bes_ble_ota_event_param_t;

typedef void(*bes_ble_ota_event_callback)(bes_ble_ota_event_param_t *param);

void bes_ble_ota_event_reg(bes_ble_ota_event_callback cb);

void bes_ble_ota_event_unreg(void);

void bes_ble_ota_send_rx_cfm(uint8_t conidx);

bool bes_ble_ota_send_notification(uint8_t conidx, uint8_t* ptrData, uint32_t length);

bool bes_ble_ota_send_indication(uint8_t conidx, uint8_t* ptrData, uint32_t length);

#endif

#ifdef BLE_TOTA_ENABLED
typedef enum {
    BES_BLE_TOTA_CCC_CHANGED = 0,
    BES_BLE_TOTA_DIS_CONN_EVENT,
    BES_BLE_TOTA_RECEVICE_DATA,
    BES_BLE_TOTA_MTU_UPDATE,
    BES_BLE_TOTA_SEND_DONE,
} BES_BLE_TOTA_EVENT_TYPE_E;

typedef struct {
    /// tota event type, see@BES_BLE_TOTA_EVENT_TYPE_E
    uint8_t event_type;
    uint8_t conidx;
    uint16_t connhdl;
    union {
       /// BES_BLE_TOTA_CCC_CHANGED
       uint8_t ntf_en;
       // BES_BLE_TOTA_MTU_UPDATE
       uint16_t mtu;
       // BES_BLE_TOTA_SEND_DONE
       uint8_t status;
       // BES_BLE_TOTA_RECEVICE_DATA
       struct {
           uint16_t data_len;
           uint8_t *data;
       } receive_data;
    } param;
} bes_ble_tota_event_param_t;

typedef void(*bes_ble_tota_event_callback)(bes_ble_tota_event_param_t *param);

void bes_ble_tota_event_reg(bes_ble_tota_event_callback cb);

void bes_ble_tota_event_unreg(void);

bool bes_ble_tota_send_notification(uint8_t conidx, uint8_t* ptrData, uint32_t length);

bool bes_ble_tota_send_indication(uint8_t conidx, uint8_t* ptrData, uint32_t length);

#endif

void bes_ble_datapath_server_register_event_callback(app_datapath_event_cb cb);
void bes_ble_datapath_server_send_data_via_notification(uint8_t conidx, uint8_t* ptrData, uint32_t length);
void bes_ble_datapath_server_send_data_via_indication(uint8_t conidx, uint8_t* ptrData, uint32_t length);
void bes_ble_add_datapathps(void);
void bes_ble_datapath_server_register_event_cb(app_datapath_event_cb callback);

void bes_ble_datapath_client_control_notification(uint8_t conidx, bool isEnable);
void bes_ble_datapath_client_send_data_via_write_command(uint8_t conidx, uint8_t* ptrData, uint32_t length);
void bes_ble_datapath_client_send_data_via_write_request(uint8_t conidx, uint8_t* ptrData, uint32_t length);
void bes_ble_datapath_client_register_callbacK(void *callback);

#ifdef BLE_WIRELESS_TRANS_SRV_ENABLED
void app_ble_wireless_trans_srv_register_callback(void *callback);
uint8_t app_ble_srv_send_attr1_data_via_notification(uint8_t *value, uint16_t len);
#endif

#ifdef BLE_WIRELESS_TRANS_CLI_ENABLED
void app_ble_wireless_trans_cli_register_callback(void *callback);
uint8_t app_ble_wireless_trans_cli_send_attr0_data_handler(uint16_t connhdl, const uint8_t *value, uint16_t len);
#endif

#ifdef __cplusplus
}
#endif
#endif
#endif /* __BES_DP_API_H__ */
