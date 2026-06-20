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
#ifndef __BLE_DATAPATH_CLIENT_H__
#define __BLE_DATAPATH_CLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DPC_CHAR_DATA_RX,
    DPC_CHAR_DATA_TX,
    DPC_CHAR_MAX_NUM,
} dpc_char_enum_t;

typedef enum {
    /// RX Client Characteristic Configuration
    DPC_DESC_RX_CFG,
    /// RX Characteristic User Description
    DPC_DESC_RX_CUD,
    /// TX Characteristic User Description
    DPC_DESC_TX_CUD,

    DPC_DESC_MAX_NUM,

    DPC_DESC_MASK = 0x10,
} dpc_desc_enum_t;

typedef enum {
    /// Notified Char
    DPC_NTF_CHAR_DATA               = DPC_CHAR_DATA_RX,
    /// Write cmd/req Char
    DPC_WRITE_CHAR_DATA             = DPC_CHAR_DATA_TX,

    /// Read/Write Ntf Char —— Client characteristic configuration.
    DPC_RD_WR_NTF_CCC_CFG           = (DPC_DESC_RX_CFG | DPC_DESC_MASK),
    /// Read Ntf Char —— Characteristic user description.
    DPC_RD_NTF_CUD                  = (DPC_DESC_RX_CUD | DPC_DESC_MASK),
    /// Read Write Char —— Characteristic user description.
    DPC_RD_WR_CUD                   = (DPC_DESC_TX_CUD | DPC_DESC_MASK),
} dpc_code_enum_t;

typedef struct {
    void(*dpc_connected_done_cb)(uint8_t conidx);
    void(*dpc_disconnected_done_cb)(uint8_t conidx);
    void(*dpc_mtu_exchanged_done_cb)(uint8_t conidx, uint16_t mtu);
    void(*dpc_discover_done_cb)(uint8_t conidx);
    void(*dpc_data_received_cb)(uint8_t conidx, const uint8_t *p_buff, uint16_t bufLength);
    void(*dpc_tx_done_cb)(uint8_t conidx, uint8_t err_code);
} app_datapath_client_event_cb_t;

void ble_datapath_client_init(void);
void ble_datapath_client_start_discover(uint8_t conidx);
void app_datapath_client_register_callback(app_datapath_client_event_cb_t *callback);
void app_datapath_client_control_notification(uint8_t conidx, bool isEnable);
void app_datapath_client_send_data_via_write_command(uint8_t conidx, uint8_t* ptrData, uint32_t length);
void app_datapath_client_send_data_via_write_request(uint8_t conidx, uint8_t* ptrData, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif //__BLE_DATAPATH_CLIENT_H__
