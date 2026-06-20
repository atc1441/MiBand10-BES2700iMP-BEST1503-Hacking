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
#ifndef __BLE_DULT_H__
#define __BLE_DULT_H__
#ifdef SPOT_ENABLED
#include "gatt_service.h"
#include "ble_gfps_common.h"
#ifdef __cplusplus
extern "C" {
#endif

#define DULT_GET_PRODUCT_DATA                                 0x0003
#define DULT_GET_PRODUCT_DATA_RES                             0x0803
#define DULT_GET_MANUFACTURE_NAME                             0x0004
#define DULT_GET_MANUFACTURE_NAME_RSP                         0x0804
#define DULT_GET_MODEL_NAME                                   0x0005
#define DULT_GET_MODEL_NAME_RSP                               0x0805
#define DULT_GET_ACCESSORY_CATEGORY                           0x0006
#define DULT_GET_ACCESSORY_CATEGORY_RSP                       0x0806
#define DULT_GET_PROTOCOL_VERSION                             0x0007
#define DULT_GET_PROTOCOL_VERSION_RSP                         0x0807
#define DULT_GET_ACCESSORY_CAPABILITIES                       0x0008
#define DULT_GET_ACCESSORY_CAPABILITIES_RSP                   0x0808
#define DULT_GET_NETWORK_ID                                   0x0009
#define DULT_GET_NETWORK_ID_RSP                               0x0809
#define DULT_GET_FIRMWARE_VERSION                             0x000A
#define DULT_GET_FIRMWARE_VERSION_RSP                         0x080A
#define DULT_GET_BATTERY_TYPE                                 0x000B
#define DULT_GET_BATTERY_TYPE_RSP                             0x080B
#define DULT_GET_BATTERY_LEVEL                                0x000C
#define DULT_GET_BATTERY_LEVEL_RSP                            0x080C

#define DULT_SOUND_START                                      0x0300
#define DULT_SOUND_STOP                                       0x0301
#define DULT_SOUND_COMMAND_RESPONSE                           0x0302
#define DULT_SOUND_COMPLETED                                  0x0303
#define DULT_GET_IDENTIFIER                                   0x0404
#define DULT_GET_IDENTIFIER_RSP                               0x0405

#define SOUND_SUCCESS                                         0x0000
#define SOUND_INVALID_STATE                                   0x0001
#define SOUND_INVALID_CONFIGURATION                           0x0002
#define SOUND_INVALID_LENGTH                                  0x0003
#define SOUND_INVALID_PARAM                                   0x0004
#define SOUND_INVALID_COMMAND                                 0xFFFF


#define DULT_ACCESSORY_CATEGORY_VALUE                         151
#define DULT_POWERED_BATTERT                                  0x00
#define DULT_NON_RECHARGEDABLE_BATTERY                        0x01
#define DULT_RECHARGEDABLE_BATTERY                            0x02
#define DULT_BATTERT_LEVEL_FULL                               0x00
#define DULT_BATTERY_LEVEL_MEDIUM                             0x01
#define DULT_BATTERY_LEVEL_LOW                                0x02
#define DULT_BATTERY_LEVEL_CRITICALLY_LOW                     0x03

#define DULT_PRODUCT_DATA_LEN                                 (8)
#define DULT_CATEGROY_LEN                                     (8)
#define DULT_MANUFACTUR_NAME_LEN                              (64)
#define DULT_MODEL_NAME_LEN                                   (64)
typedef struct _dult_get_product_data_resp{
    uint16_t data_id;
    uint8_t data[DULT_PRODUCT_DATA_LEN];
}dult_get_product_data_resp;

typedef struct _dult_get_manufacturer_name_resp{
    uint16_t data_id;
    char str[DULT_MANUFACTUR_NAME_LEN];
}dult_get_manufacturer_name_resp;

typedef struct _dult_get_model_name_resp{
    uint16_t data_id;
    char str[DULT_MODEL_NAME_LEN];
}dult_get_model_name_resp;

typedef struct _dult_get_category_value_resp{
    uint16_t data_id;
    uint8_t data[DULT_CATEGROY_LEN];
}dult_get_category_value_resp;

typedef struct _dult_get_protocol_resp{
    uint16_t data_id;
    uint32_t prorocol;
}__attribute__ ((__packed__))dult_get_protocol_resp;

typedef struct _dult_get_accessory_capability_resp{
    uint16_t data_id;
    uint32_t accessory_value;
}__attribute__ ((__packed__))dult_get_accessory_capability_resp;

typedef struct _dult_get_network_id_resp{
    uint16_t data_id;
    uint8_t networkID;
}__attribute__ ((__packed__))dult_get_network_id_resp;

typedef struct _dult_get_firmware_version_resp{
    uint16_t data_id;
    uint32_t firmware_version;
}__attribute__ ((__packed__))dult_get_firmware_version_resp;

typedef struct _dult_get_battery_type_resp{
    uint16_t data_id;
    uint8_t battery_type;
}__attribute__ ((__packed__))dult_get_battery_type_resp;

typedef struct _dult_get_battery_level_resp{
    uint16_t data_id;
    uint8_t battery_level;
}__attribute__ ((__packed__))dult_get_battery_level_resp;

typedef struct _dult_sound_start_resp{
    uint16_t data_id;
    uint16_t cmdcode;
    uint16_t status;
}__attribute__ ((__packed__))dult_sound_start_resp;

typedef struct _dult_sound_stop_resp{
    uint16_t data_id;
    uint16_t cmdcode;
    uint16_t status;
}__attribute__ ((__packed__))dult_sound__stop_resp;

typedef struct _dult_sound_complete_resp{
    uint16_t data_id;
}__attribute__ ((__packed__))dult_sound_complete_resp;


typedef struct _dult_identifier_payload_resp{
    uint16_t data_id;
    uint8_t data[18];
}__attribute__ ((__packed__))dult_get_identifier_payload_resp;

typedef struct _dult_cmd_resp{
    uint16_t cmdcode;
    uint16_t status;
}__attribute__ ((__packed__))dult_cmd_resp;

struct ble_dult_cb_t
{
    bool (*get_spot_get_mode)(void);
    void (*start_find_ringtone)(void);
    void (*stop_find_ringtone)(void);
    uint8_t *(*get_eid)(void);
    uint32_t (*sha256_hash)(const void* in_data, int len, void* out_data);
    void (*beacon_encrpt_data)(uint8_t *accKey, uint8_t *inputData, uint32_t inputDataLen, uint8_t *output);
};

struct dult_ble_env_tag
{
    uint8_t connectionIndex;
    uint8_t isIndicationEnabled;
    struct ble_dult_cb_t *cb;
};

void ble_app_dult_init(struct ble_dult_cb_t *cbs);

#ifdef __cplusplus
    }
#endif
#endif /* SPOT_ENABLED */
#endif /* __BLE_DULT_H__ */
