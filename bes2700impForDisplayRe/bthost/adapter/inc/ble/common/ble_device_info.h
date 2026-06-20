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
#ifndef __BLE_DEVICE_INFO_H__
#define __BLE_DEVICE_INFO_H__

#include "stdint.h"

#define BTIF_BD_ADDR_SIZE 6
#define BLE_ADDR_SIZE 6
#define BLE_ENC_RANDOM_SIZE 8
#define BLE_LTK_SIZE 16
#define BLE_IRK_SIZE 16
#define BLE_CSRK_SIZE 16

typedef struct {
    uint8_t ble_addr[BTIF_BD_ADDR_SIZE];
    uint8_t ble_irk[BLE_IRK_SIZE];
    uint8_t ble_csrk[BLE_CSRK_SIZE];
} __attribute__ ((packed)) BLE_BASIC_INFO_T;

typedef struct
{
    /// BD Address of device
    uint8_t addr[BLE_ADDR_SIZE];
    /// Address type of the device 0=public/1=private random
    uint8_t addr_type;
} __attribute__ ((packed)) BLE_ADDR_INFO_T;

// Cache peer service num
#define BLE_MAX_CACHE_16_SVC_NUM    (5)
// Cache peer 128 service num
#define BLE_MAX_CACHE_128_SVC_NUM   (3)
// GATT and GAP service and tbs and mcs
#define BLE_MAX_CACHE_16_CHAR_NUM   (50)
// GATT and GAP service and tbs and mcs
#define BLE_MAX_CACHE_128_CHAR_NUM  (8)

typedef struct
{
    uint8_t server_cache_is_exist: 1;
    uint8_t service_change_unaware: 1;
    uint8_t gatt_client_supp_robust_caching: 1;
    uint8_t gatt_client_supp_eatt_bearer: 1;
    uint8_t gatt_client_supp_recv_multi_notify: 1;
    uint8_t gatt_server_supp_eatt_bearer: 1;
    uint8_t gatt_server_has_service_changed_char: 1;
    uint8_t gatt_server_has_database_hash_char: 1;
}  __attribute__ ((packed)) gattc_server_cache_t;

typedef struct
{
    //uint16_t char_handle;
    //uint16_t char_end_handle;
    uint16_t char_value_handle;
    //uint16_t cccd_handle;
    uint8_t service_seqn: 5;
    //uint8_t in_128_service: 1;
    uint8_t reliable_write: 1;
    uint8_t writable_aux: 1;
    uint8_t char_prop;
    uint16_t char_uuid;
} __attribute__ ((packed)) gattc_char_t;

typedef struct
{
    //uint16_t char_handle;
    //uint16_t char_end_handle;
    uint16_t char_value_handle;
    ///uint16_t cccd_handle;
    uint8_t service_seqn: 5;
    uint8_t reliable_write: 1;
    uint8_t writable_aux: 1;
    uint8_t char_prop;
    uint8_t uuid_128_le[16];
} __attribute__ ((packed)) gattc_128_char_t;

typedef struct
{
    uint16_t start_handle;
    uint16_t end_handle;
    uint16_t service_uuid;
} __attribute__ ((packed)) gattc_service_t;

typedef struct
{
    uint16_t start_handle;
    uint16_t end_handle;
    uint8_t uuid_128_le[16];
} __attribute__ ((packed)) gattc_128_service_t;

typedef struct
{
    uint8_t peer_database_hash[16];
    gattc_service_t peer_service[BLE_MAX_CACHE_16_SVC_NUM];
    gattc_128_service_t peer_128_service[BLE_MAX_CACHE_128_SVC_NUM];
    gattc_char_t peer_character[BLE_MAX_CACHE_16_CHAR_NUM];
    gattc_128_char_t peer_128_character[BLE_MAX_CACHE_128_CHAR_NUM];
}  __attribute__ ((packed)) gattc_client_cache_t;

typedef struct
{
    uint8_t cache_seqn;
    BLE_ADDR_INFO_T peer_addr;
    gattc_client_cache_t client_cache;
} __attribute__ ((packed)) gattc_nv_cache_t;

enum bond_info
{
    BONDED_STATUS_POS       = 0,
    BONDED_WITH_IRK_DISTRIBUTED = 1,
    BONDED_WITH_CSRK_DISTRIBUTED = 2,
    BONDED_SECURE_CONNECTION = 3,
    BONDED_WITH_NUM_COMPARE = 4,
    BONDED_WITH_PASSKEY_ENTRY = 5,
    BONDED_WITH_OOB_METHOD = 6,
    BONDED_WITH_DEMO_APP_ADV = 7,
};

typedef struct {
    BLE_ADDR_INFO_T peer_addr;
    uint8_t bond_info_bf;
    uint16_t EDIV;
    uint16_t LOCAL_EDIV;
    uint8_t RANDOM[BLE_ENC_RANDOM_SIZE];
    uint8_t LTK[BLE_LTK_SIZE];
    uint8_t LOCAL_RANDOM[BLE_ENC_RANDOM_SIZE];
    uint8_t LOCAL_LTK[BLE_LTK_SIZE];
    uint8_t IRK[BLE_IRK_SIZE];
    uint8_t CSRK[BLE_CSRK_SIZE];
    uint8_t enc_key_size;
    gattc_server_cache_t server_cache;
} __attribute__ ((packed)) BleDevicePairingInfo;

typedef struct {
    BleDevicePairingInfo pairingInfo;
    uint8_t volume;
} __attribute__ ((packed)) BleDeviceinfo;

#endif /* __BLE_DEVICE_INFO_H__ */
