/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifndef __OTA_BASIC_H__
#define __OTA_BASIC_H__

#include "stdint.h"

#ifndef OTA_BD_ADDR_LEN
#define OTA_BD_ADDR_LEN    (6)
#endif

#ifndef NEW_IMAGE_FLASH_OFFSET
#define NEW_IMAGE_FLASH_OFFSET    (0x180000)
#endif

#define BES_OTA_UUID_128                    {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66 }

#if (BLE_OTA)
#define ota_val_char_val_uuid_128_content   {0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77 }	
#define ATT_DECL_PRIMARY_SERVICE_UUID       { 0x00, 0x28 }
#define ATT_DECL_CHARACTERISTIC_UUID        { 0x03, 0x28 }
#define ATT_DESC_CLIENT_CHAR_CFG_UUID       { 0x02, 0x29 }
#endif

// ota data path is exclusive, anytime there is only one ota datapath available
typedef enum
{
    OTA_BASIC_TWS_INVALID_ROLE   = 0,
    OTA_BASIC_TWS_FREEMAN   = 1,
    OTA_BASIC_TWS_MASTER    = 2,
    OTA_BASIC_TWS_SLAVE     = 3,
    OTA_BASIC_TWS_UNKNOWN   = 4,
} OTA_BASIC_TWS_ROLE_E;

typedef enum 
{
    NO_OTA_CONNECTION = 0,
    DATA_PATH_BLE,
    DATA_PATH_SPP,
    DATA_PATH_BTGATT,
} BES_OTA_PATH_TYPE_E;

typedef enum 
{
    BES_OTA_CONN = 0,
    BES_OTA_DISCONN,
    BES_OTA_RECEVICE_DATA,
    BES_OTA_MTU_UPDATE,
    BES_OTA_SEND_DONE,
    BES_OTA_EVENT_NUM,
} BES_OTA_EVENT_TYPE_E;

typedef struct{
    BES_OTA_PATH_TYPE_E pathType;
    BES_OTA_EVENT_TYPE_E event;
    uint8_t conidx;
    union {
       /// BES_OTA_CONN/BES_OTA_DISCONN
       uint8_t address[OTA_BD_ADDR_LEN];
       // BES_OTA_MTU_UPDATE
       uint16_t mtu;
       // BES_OTA_RECEVICE_DATA
       struct {
           uint16_t dataLen;
           uint8_t *data;
       } receive_data;
    } param;
}bes_ota_event_param_t;

typedef enum {
    BLE_RX_DATA_SELF_OTA,
    SPP_RX_DATA_SELF_OTA,
    BLE_RX_DATA_SELF_OTA_OVER_TOTA,
    SPP_RX_DATA_SELF_OTA_OVER_TOTA,
    BLE_RX_DATA_SELF_MAX,
} BES_OTA_RX_FLAG_E;


OTA_BASIC_TWS_ROLE_E ota_basic_get_tws_role(void);
uint8_t* app_ota_get_common_databuf(void);
bool ota_basic_is_remap_enabled(void);
uint32_t ota_basic_get_remap_offset(void);
void app_ota_push_rx_data(BES_OTA_RX_FLAG_E flag, bes_ota_event_param_t* ptr);
void app_ota_connected(BES_OTA_PATH_TYPE_E connType);
void app_ota_disconnected(BES_OTA_PATH_TYPE_E connType);
BES_OTA_PATH_TYPE_E app_ota_get_connected_type(void);
uint8_t app_ota_get_conidx(void);
void ota_basic_mtu_set(uint16_t otaMtu);
uint16_t ota_basic_mtu_get(void);
void ota_basic_bt_mac_addr_set(const uint8_t* macAddr);
uint8_t* ota_basic_get_mac_addr(void);
uint32_t ota_basic_get_max_image_size(void);
void app_ota_data_handle(bes_ota_event_param_t *param, BES_OTA_RX_FLAG_E flag);
void app_ota_rx_thread_init(void);
void ota_basic_env_init(void);

#endif

