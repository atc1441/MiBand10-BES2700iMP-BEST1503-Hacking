
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
#ifndef __BLE_WIRELESS_TRANS_CLI_H__
#define __BLE_WIRELESS_TRANS_CLI_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BLE_WIRELESS_TRANS_CLI_ENABLED
#define BLE_INVALID_CONNECTION_HDL    0xFFFF

typedef enum {
    CLI_CHAR_SEND_ATTR0,
    CLI_CHAR_RECV_ATTR1,
    CLI_CHAR_RECV_ATTR2,
    CLI_CHAR_RECV_SEND_ATTR,

    CLI_CHAR_MAX_NUM,
} ble_wireless_trans_cli_char_enum_t;

typedef struct {
    gatt_prf_t head;
    uint16_t NtfIndCfgAttr[CLI_CHAR_MAX_NUM];
    gatt_peer_service_t *peer_service;
    gatt_peer_character_t *peer_char[CLI_CHAR_MAX_NUM];
} ble_wireless_trans_cli_prf_t;

typedef struct {
    void(*tx_done_cb)(uint8_t conidx, uint8_t char_index, uint8_t status);
    void(*data_received_cb)(uint8_t conidx, uint8_t char_index, uint8_t *p_buff, uint16_t bufLength);
    void(*disconnected_done_cb)(uint8_t conidx);
    void(*connected_done_cb)(uint8_t conidx, uint16_t connhdl);
    void(*mtu_exchanged_done_cb)(uint8_t conidx, uint16_t mtu);
    void(*discover_done_cb)(uint8_t conidx);
} ble_wireless_trans_client_cb_t;

void ble_wireless_trans_cli_init(void);

void ble_wireless_trans_cli_register_callback(ble_wireless_trans_client_cb_t *callback);
// uart1 rece data send to attr0
uint8_t ble_wireless_trans_cli_send_attr0_data_handler(uint16_t connhdl, const uint8_t *value, uint16_t len);

#endif // BLE_WIRELESS_TRANS_CLI_ENABLED

#ifdef __cplusplus
}
#endif

#endif /* __BLE_WIRELESS_TRANS_CLI_H__ */
