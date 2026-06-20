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
#ifndef __BLE_WIRELESS_TRANS_SRV_H__
#define __BLE_WIRELESS_TRANS_SRV_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BLE_WIRELESS_TRANS_SRV_ENABLED
#define BLE_INVALID_CONNECTION_INDEX    0xFF

typedef enum {
    SRV_CHAR_RECV_ATTR0,
    SRV_CHAR_SEND_ATTR1,
    SRV_CHAR_SEND_ATTR2,
    SRV_CHAR_RECV_SEND_ATTR,

    SRV_CHAR_MAX_NUM,
} ble_wireless_trans_srv_char_enum_t;

struct ble_wireless_trans_server_env_tag
{
    uint8_t connectionIndex;
    uint16_t NtfIndCfgAttr[SRV_CHAR_MAX_NUM];
};

typedef struct {
    void(*tx_done_cb)(uint8_t conidx, uint8_t char_index);
    void(*data_received_cb)(uint8_t conidx, uint8_t char_index, uint8_t *p_buff, uint16_t bufLength);
    void(*disconnected_done_cb)(uint8_t condix, uint8_t char_index);
    void(*connected_done_cb)(uint8_t condix, uint8_t char_index, uint16_t value);
    void(*mtu_exchanged_done_cb)(uint8_t conidx, uint16_t mtu);
} ble_wireless_trans_server_cb_t;

void ble_wireless_trans_srv_init(void);
void ble_wireless_trans_srv_register_callback(ble_wireless_trans_server_cb_t *callback);

uint8_t ble_wireless_trans_srv_send_attr1_data_via_notification(uint8_t* data, uint32_t len);
uint8_t ble_wireless_trans_srv_send_attr2_data_via_notification(uint8_t* data, uint32_t len);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __BLE_WIRELESS_TRANS_SRV_H__ */
