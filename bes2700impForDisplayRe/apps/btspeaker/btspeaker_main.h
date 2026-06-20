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
#ifndef __APP_BT_SPEAKER_H__
#define __APP_BT_SPEAKER_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
#ifdef APP_BT_SPEAKER

#define BTSPEAKER_BLE_ADV_INTERVAL_DISCOVERY_TWS        (30) //ms
#define BTSPEAKER_BLE_ADV_INTERVAL_GFPS_PARING          (48) //ms
#define BTSPEAKER_BLE_ADV_INTERVAL_IDLE_DEFAULT         (160) //ms

#define BTSPEAKER_BLE_DISCOVER_DEVICE_TIMEOUT            (6000) //ms
#define BTSPEAKER_BLE_CONFIRM_DEVICE_TIMEOUT             (3000) //ms
#define BTSPEAKER_BLE_TWS_CONN_PREHANDLE_TIMEOUT         (500) //ms


enum
{
    SPEAKER_BLE_ADV_OP_IDLE,
    SPEAKER_BLE_ADV_OP_SETUP_STEREO_REQ,
    SPEAKER_BLE_ADV_OP_SETUP_STEREO_CFM,
    SPEAKER_BLE_ADV_OP_EXIT_STEREO_REQ,
    SPEAKER_BLE_ADV_OP_EXIT_STEREO_CFM,
    SPEAKER_BLE_ADV_OP_MAX_NUM,
};

enum
{
    SPEAKER_ROLE_MASTER,
    SPEAKER_ROLE_SLAVE,
    SPEAKER_ROLE_UNKNOW,
};

enum
{
    SPEAKER_PRODUCT_TYPE_BESBOARD,
    SPEAKER_PRODUCT_TYPE_SONY,
    SPEAKER_PRODUCT_TYPE_HARMON,
    SPEAKER_PRODUCT_TYPE_MAX_NUM,
};

enum
{
    SPEAKER_PRODUCT_COLOR_BLACK,
    SPEAKER_PRODUCT_COLOR_READ,
    SPEAKER_PRODUCT_COLOR_GREEN,
    SPEAKER_PRODUCT_COLLOR_BLUE,
};

enum
{
    SPEAKER_CONNECTION_STATUS_DISCONNECTED,
    SPEAKER_CONNECTION_STATUS_CONNECTING,
    SPEAKER_CONNECTION_STATUS_CONNECTED,
};

enum
{
    SPEAKER_APP_ID_INVALID,
    SPEAKER_APP_ID_A2DP,
    SPEAKER_APP_ID_HFP,
};

typedef enum
{
    IBRT_UI_IDLE,
    IBRT_UI_TWS_DISCOVERY,
    IBRT_UI_TWS_FORMATION,
    IBRT_UI_TWS_PREHANDLE,
    IBRT_UI_SM_STOP,

    IBRT_UI_UNKNOWN,
} speaker_ibrt_ui_state_e;

typedef enum
{
    SPEAKER_IBRT_UI_NO_ERROR,
    SPEAKER_IBRT_UI_RSP_TIMEOUT,
} speaker_ibrt_ui_error_e;


typedef struct
{
    uint8_t tws_operation:      4;
    uint8_t speaker_role:       2;
    uint8_t reserved0:          2;
} __attribute__((packed)) SPEAKER_TWS_INFO_T;

typedef struct
{
    uint8_t product_type:       4;
    uint8_t product_color:      2;
    uint8_t reserved1:          2;
} __attribute__((packed)) SPEAKER_PRODUCT_INFO_T;

typedef struct
{
    uint8_t  mobile_connection_status:       2;
    uint8_t  wifi_connection_status:         2;
    uint8_t  app_profile_status:             4;
    uint16_t rtc_time;
} __attribute__((packed)) SPEAKER_CONNECTION_STATUS_INFO_T;

typedef struct
{
    uint8_t  bt_mac_addr[6];
} __attribute__((packed)) SPEAKER_DEVICE_INFO;

typedef struct
{
    SPEAKER_TWS_INFO_T                  tws_info;
    SPEAKER_PRODUCT_INFO_T              product_info;
    SPEAKER_CONNECTION_STATUS_INFO_T    connection_info;
    SPEAKER_DEVICE_INFO                 device_info;
    uint8_t                             crc_check;
} __attribute__((packed)) SPEAKER_BLE_ADV_DATA_TWS_DISCOVE_IND_T;

typedef struct
{
    SPEAKER_TWS_INFO_T                  tws_info;
    SPEAKER_DEVICE_INFO                 local_device_info;
    SPEAKER_CONNECTION_STATUS_INFO_T    local_connection_info;
    SPEAKER_DEVICE_INFO                 peer_device_info;
    SPEAKER_CONNECTION_STATUS_INFO_T    peer_connection_info;
    uint8_t                             crc_check;
}__attribute__((packed)) SPEAKER_BLE_ADV_DATA_TWS_CONFIRM_DEV_T;

typedef struct
{
    bool                                is_use;
    int8_t                              peer_rssi_value;
    SPEAKER_DEVICE_INFO                 peer_device_info;
    SPEAKER_CONNECTION_STATUS_INFO_T    peer_connection_status_info;
    uint16_t                            peer_scan_recv_cnt;
} __attribute__((packed)) SPEAKER_BLE_SCAN_DATA_STORE_CACHE_TABLE_T;

typedef struct
{
    speaker_ibrt_ui_state_e super_state;
    speaker_ibrt_ui_error_e error_state;

} speaker_app_ibrt_ui_t;


speaker_app_ibrt_ui_t *speaker_app_ibrt_ui_get_ctx(void);

void app_speaker_ble_adv_data_start_tws_discovery_ind(void);
void app_speaker_ble_adv_data_start_tws_discovery_cfm(void);
void app_speaker_ble_adv_data_stop(void);
void app_speaker_ble_scan_device_start(void);
void app_speaker_ble_scan_device_stop(void);
bool app_speaker_setup_bt_stereo(void);
bool app_speaker_exit_bt_stereo(void);
void app_speaker_tws_pairing_parse(void);
void app_speaker_tws_reset_parse(void);
void app_speaker_init(void);
#endif
#ifdef __cplusplus
}
#endif
#endif  //end of __APP_BT_SPEAKER_H__
