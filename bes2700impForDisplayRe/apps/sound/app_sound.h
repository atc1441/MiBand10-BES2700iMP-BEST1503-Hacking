/***************************************************************************
 *
 * @copyright 2013-2023 BES.
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
 * @author dazhaoyang
 * @date 2023.06.15         v1.0
 *
 ****************************************************************************/

#ifndef __APP_SOUND_H__
#define __APP_SOUND_H__

#include <stdint.h>
#include "cmsis_os.h"
#include "hal_timer.h"
#ifdef APP_SOUND_UI_ENABLE
typedef enum
{
    SOUND_PRODUCT_TYPE_BESBOARD,
    SOUND_PRODUCT_TYPE_SONY,
    SOUND_PRODUCT_TYPE_HARMON,
    SOUND_PRODUCT_TYPE_MAX_NUM,
} SOUND_PRODUCT_TYPE_E;

typedef enum
{
    SOUND_FREE = 0,
    SOUND_FREE2TWS,
    SOUND_TWS,
    SOUND_TWS2FREE,
    SOUND_MODE_END = SOUND_TWS2FREE,
} BT_SOUND_MODE_E;

typedef  uint8_t   SOUND_EVENT_TYPE;

typedef enum {
    SOUND_NONE_EVENT = 0,
    SOUND_POWER_KEY_CLICK,
    SOUND_POWER_KEY_DOUBLE_CLICK,
    SOUND_POWER_KEY_LONG_PRESS,
    SOUND_BT_KEY_CLICK,
    SOUND_BT_KEY_DOUBLE_CLICK,
    SOUND_BT_KEY_THREE_CLICK,
    SOUND_BT_KEY_LONG_PRESS,
    SOUND_WIFI_KEY_CLICK,
    SOUND_WIFI_KEY_DOUBLE_CLICK,
    SOUND_WIFI_KEY_LONG_PRESS,
    SOUND_CIRCLE_KEY_CLICK,
    SOUND_CIRCLE_KEY_DOUBLE_CLICK,
    SOUND_CIRCLE_UP_KEY_CLICK,
    SOUND_CIRCLE_DOWN_KEY_CLICK,
    SOUND_CIRCLE_LEFT_KEY_CLICK,
    SOUND_CIRCLE_RIGHT_KEY_CLICK,
    SOUND_BT_AND_POWER,
    SOUND_CIRCLE_AND_POWER,
    SOUND_AUX_IN_INSERT,
    SOUND_AUX_IN_EXTRACT,
} SOUND_KEY_EVENT_TYPE_E;

typedef enum
{
    SOUND_UI_IDLE = 0,
    SOUND_UI_W4_BT_EBABLE,
    SOUND_UI_W4_BT_DISABLE,
    SOUND_UI_W4_BT_DISCONNECT_ALL,
    SOUND_UI_W4_TWS_DISCONNECT,
    SOUND_UI_W4_TWS_CONNECT,
    SOUND_UI_W4_ENTER_PAIRING,
    SOUND_UI_W4_CLEAR_PAIRING,
    SOUND_UI_W4_TWS_DISCOVERY,
    SOUND_UI_W4_TWS_FORMATION,
    SOUND_UI_W4_TWS_PREHANDLE,
    SOUND_UI_W4_MOBILE_CONNECTED,
    SOUND_UI_W4_MOBILE_DISCONNECTED,
    SOUND_UI_W4_ENTER_FREEMAN,
    SOUND_UI_W4_ENTER_BIS,
    SOUND_UI_W4_ALL_DEVICE_READY,
    SOUND_UI_W4_SM_STOP,
    SOUND_UI_W4_END = SOUND_UI_W4_SM_STOP,
} SOUND_UI_STATE_E;

//Mapping to sound ui state
typedef enum
{
    SOUND_ACTION_IDLE = 0,
    SOUND_ACTION_BT_ENABLE_EVENT,
    SOUND_ACTION_BT_DISABLE_EVENT,
    SOUND_ACTION_BT_DISCONNECT_ALL,
    SOUND_ACTION_TWS_DISCONNECTED,
    SOUND_ACTION_TWS_CONNECTE_SIG,
    SOUND_ACTION_BT_ENTERPAIRING_EVENT,
    SOUND_ACTION_BT_CLEAR_PAIRING_EVENT,
    SOUND_ACTION_TWS_DISCOVERY,
    SOUND_ACTION_TWS_FORMATION,
    SOUND_ACTION_TWS_PREHANDLE,
    SOUND_ACTION_MOBILE_CONNECTED,
    SOUND_ACTION_MOBILE_DISCONNECTED,
    SOUND_ACTION_ENTER_FREEMAN,
    SOUND_ACTION_ENTER_BIS,
    SOUND_ACTION_W4_ALL_DEVICE_READY,
    SOUND_ACTION_SM_STOP,
    SOUND_ACTION_END= SOUND_ACTION_SM_STOP,
} SOUND_ACTION_E;

typedef enum {
    SOUND_UI_NONE_EVENT = 0,
    SOUND_UI_POWERON_EVENT,
    SOUND_UI_POWEROFF_EVENT,
    SOUND_UI_CLEAR_PAIRING_EVENT,
    SOUND_UI_ENTER_PAIRING_EVENT,
    SOUND_UI_FREEMAN2TWS_EVENT,
    SOUND_UI_FREEMAN2TBIS_EVENT,
    SOUND_UI_TWS2FREEMAN_EVENT,
    SOUND_UI_TWS2BIS_EVENT,
    SOUND_UI_BIS2FREEMAN_EVENT,
    SOUND_UI_BIS2TWS_EVENT,
    SOUND_UI_FACTORY_RESET_EVENT,
    SOUND_UI_EVENT_END = SOUND_UI_FACTORY_RESET_EVENT,
    SOUND_CUSTOM_BT_LONG_PRESS_EVENT,
    SOUND_CUSTOM_CIRCLE_CLICK_EVENT,
    SOUND_CUSTOM_CIRCLE_DOUBLE_CLICK_EVENT,
    SOUND_CUSTOM_VOLUME_UP_EVENT,
    SOUND_CUSTOM_VOLUME_DOWN_EVENT,
    SOUND_CUSTOM_CONTROL_MEDIA_FORWARD_EVENT,
    SOUND_CUSTOM_CONTROL_MEDIA_BACKWARD_EVENT,
    SOUND_CUSTOM_EVENT_END = SOUND_CUSTOM_CONTROL_MEDIA_BACKWARD_EVENT,
} SOUND_UI_TYPE_E;

typedef enum
{
    SOUND_UI_NO_ERROR = 0,
    SOUND_UI_RSP_TIMEOUT,
    SOUND_UI_NOT_ACCEPT,
    SOUND_UI_CONNECT_FAILED_PAGE_TIMEOUT,
    SOUND_UI_CONNECT_FAILED,
    SOUND_UI_STATUS_ERROR,
    SOUND_UI_CONNECTION_TIMEOUT,
} SOUND_UI_ERROR_E;

typedef enum {
    SOUND_BT_READY_MODE = 0,
    SOUND_BT_FREEMAN_MODE,
    SOUND_BT_TWS_MODE,
    SOUND_BT_BIS_MODE,
    SOUND_BT_FREEMAN2TWS_MODE,
    SOUND_BT_FREEMAN2BIS_MODE,
    SOUND_BT_TWS2FREEMAN_MODE,
    SOUND_BT_TWS2BIS_MODE,
    SOUND_BT_BIS2FREEMAN_MODE,
    SOUND_BT_BIS2TWS_MODE,
    SOUND_BT_MODE_END,
} SOUND_BT_MODE_TYPE_E;

typedef enum {
    SOUND_ONOFF_SHOUTED_MODE = 0,
    SOUND_ONOFF_SHOUTING,
    SOUND_ONOFF_OPENING,
    SOUND_ONOFF_OPENED_MODE,
} SOUND_ONOFF_STATUS_TYPE_E;

typedef enum {
    SOUND_ACL_DISCONNECTED = 0,
    SOUND_ACL_DISCONNECTING,
    SOUND_ACL_CONNECTING,
    SOUND_ACL_CONNECTED,
} SOUND_ACL_STATUS_TYPE_E;

typedef enum {
    SOUND_AUDIO_IDLE = 0,
    SOUND_AUDIO_BT_A2DP,
    SOUND_AUDIO_WIFI_MUSIC,
    SOUND_AUDIO_CALLING,
} SOUND_AUDIO_STATUS_TYPE_E;

typedef enum {
    SOUND_WIFI_AIRPLAY_DISCONNECTED = 0,
    SOUND_WIFI_AIRPLAY_CONNECTED,
    SOUND_WIFI_AIRPLAY_STREAM_STARTED,
    SOUND_WIFI_AIRPLAY_STREAM_CLOSE,
} SOUND_WIFI_AIRPLAY_STATUS_TYPE_E;

typedef enum {
    SOUND_BT_A2DP_DISCONNECTED = 0,
    SOUND_BT_A2DP_CONNECTED,
    SOUND_BT_A2DP_STREAM_STARTED,
    SOUND_BT_A2DP_STREAM_CLOSE,
} SOUND_BT_A2DP_STATUS_TYPE_E;

typedef enum {
    SOUND_BT_HFP_DISCONNECTED = 0,
    SOUND_BT_HFP_CONNECTED,
    SOUND_BT_HFP_INCOMING_CALL,
    SOUND_BT_HFP_OUTGOING_CALL,
    SOUND_BT_HFP_CALLNG,
} SOUND_BT_HFP_STATUS_TYPE_E;

typedef enum {
    SOUND_BT_SCO_DISCONNECTED = 0,
    SOUND_BT_SCO_CONNECTED,
} SOUND_BT_SCO_STATUS_TYPE_E;

typedef enum {
    SOUND_BLE_SCAN_STATE_STOPPED = 0,
    SOUND_BLE_SCAN_STATE_STARTED,
} SOUND_BLE_SCAN_STATE_E;

typedef enum {
    SOUND_BLE_ADV_OP_IDLE = 0,
    SOUND_BLE_ADV_OP_SETUP_STEREO_REQ,
    SOUND_BLE_ADV_OP_SETUP_STEREO_CFM,
    SOUND_BLE_ADV_OP_SETUP_STEREO_PHD,
    SOUND_BLE_ADV_OP_EXIT_STEREO_REQ,
    SOUND_BLE_ADV_OP_EXIT_STEREO_CFM,
    SOUND_BLE_ADV_OP_END = SOUND_BLE_ADV_OP_EXIT_STEREO_CFM,
} SOUND_BLE_ADV_OP_E;

typedef enum {
    SOUND_ROLE_MASTER = 0,
    SOUND_ROLE_SLAVE,
    SOUND_ROLE_UNKNOW,
} SOUND_ROLE_E;

typedef enum
{
    SOUND_TIMER_UI_EVENT                 = 0,
    SOUND_TIMER_BT_A2DP_STREAMING        = (1 << 0),
    SOUND_TIMER_BT_HFP_CALL              = (1 << 1),
    SOUND_TIMER_WIFI_STREAMING           = (1 << 8),
    SOUND_TIMER_AUX_STREAMING            = (1 << 9),
}SOUND_CHECK_TIMER_EVENT_E;

typedef enum {
    SOUND_PROMPT_POWER_ON = 0,
    SOUND_PROMPT_POWER_OFF,
    SOUND_PROMPT_VOL_CHANGED,
    SOUND_PROMPT_BT_ENTER_FREEMAN_MODE_START,
    SOUND_PROMPT_BT_ENTER_FREEMAN_MODE_END,
    SOUND_PROMPT_BT_ENTER_BIS_MODE_START,
    SOUND_PROMPT_BT_ENTER_BIS_MODE_END,
    SOUND_PROMPT_BT_ENTER_TWS_MODE_START,
    SOUND_PROMPT_BT_ENTER_TWS_MODE_END,
    SOUND_PROMPT_WIFI_SWITCH_BT,
    SOUND_PROMPT_BT_CLEAR_PAIRING_MODE,
    SOUND_PROMPT_BT_ENTER_PAIRING_MODE,
    SOUND_PROMPT_AIRPLAY_PAIRING,
    SOUND_PROMPT_CLEAR_WIFI_NV_RECORD,
    SOUND_PROMPT_SDK_VERSION,
} SOUND_PROMPT_TYPE_E;

typedef enum
{
    SOUND_DISCOVERY_TWS_ADV = 0,
    SOUND_FORMATION_TWS_ADV,
    SOUND_PERHANDLE_TWS_ADV,
} SOUND_ADV_TYPE_E;

#define  BT_DEVICE_TYPE                                            (1)
#define  WIFI_DEVICE_TYPE                                          (2)
#define  AUDIO_DEVICE_TYPE_UNKNOW                                  (0xff)
#define  BT_AND_WIFI_DEVICE_TYPE                                   (BT_DEVICE_TYPE|WIFI_DEVICE_TYPE)

#define  BT_REMOTE_DEVICE_ID0                                      (0)
#define  BT_REMOTE_DEVICE_ID1                                      (1)
#define  BT_REMOTE_DEVICE_NUM                                      (2)
#define  BT_DEVICE_ID_UNKNOWN                                      (0xff)

#define  SOUND_IS_BT_DEVICE(type)                                  (type & BT_DEVICE_TYPE)
#define  SOUND_IS_WIFI_DEVICE(type)                                (type & WIFI_DEVICE_TYPE)
#define  SOUND_IS_BT_AND_WIFI_DEVICE(type)                         (type == BT_AND_WIFI_DEVICE_TYPE)
#define  SOUND_ONLY_IS_WIFI_DEVICE(type)                           (type == WIFI_DEVICE_TYPE)

#define  SOUND_WAIT_SHUTDOWN_TIMEOUT                               (600000)//ms

#define  SOUND_EVENT_Q_LENGTH                                      (20)

#define SOUND_BLE_SCAN_WINDOWN_MS                                  (100) //ms of unit
#define SOUND_BLE_SCAN_INTERVAL_MS                                 (200) //ms of unit
#define BLE_ADV_DATA_MANUFACTURER_TYPE_ID                          (0xFF)
#define SOUND_BLE_ADV_INTERVAL_DISCOVERY_TWS                       (30) //ms
#define SOUND_BLE_ADV_TX_POWER                                     (12)
#define SOUND_BLE_SCAN_SAME_DEVICE_MAX_CNT_THRESHOLD               (5)
#define LOCAL_SOUND_PRODUCT_TYPE                                   (SOUND_PRODUCT_TYPE_BESBOARD)
#define SOUND_NAME_MAX_LEN                                         (32)
#define SOUND_ADV_DATA_MAX_LEN                                     (64)
#define BLE_SCAN_DATA_STORE_CACHE_TABLE_SIZE                       (3)
#define SOUND_ENTER_TWS_MODE_RECONNECT_TWS_MAX_TIMES               (3)

typedef struct
{
    SOUND_UI_TYPE_E  sound_event_q[SOUND_EVENT_Q_LENGTH];
    uint8_t         front;
    uint8_t         rear;
} sound_event_manager_t;

typedef struct {
    uint8_t addr[6];
} device_addr_t;

struct app_sound_tws_info_t {
    device_addr_t twsaddr;
    device_addr_t slave_addr;
    SOUND_ROLE_E role;
    SOUND_ACL_STATUS_TYPE_E connect_status;
    uint8_t tws_reconnect_cnt;
};

struct app_sound_bt_remote_info_t {
    bool is_used;
    device_addr_t remote_addr;
    SOUND_ACL_STATUS_TYPE_E bredr_acl_state;
    SOUND_BT_A2DP_STATUS_TYPE_E a2dp_state;
    uint8_t a2dp_vol;
    SOUND_BT_HFP_STATUS_TYPE_E hfp_state;
    SOUND_BT_SCO_STATUS_TYPE_E sco_state;
    uint8_t hfp_vol;
};

struct app_sound_bt_device_info_t {
    SOUND_BT_MODE_TYPE_E mode_type;
    app_sound_tws_info_t tws_info;
    uint8_t active_device_id;
    SOUND_BLE_SCAN_STATE_E ble_scan_state;
    SOUND_BLE_ADV_OP_E ble_adv_state;
    uint8_t name_len;
    uint8_t device_name[SOUND_NAME_MAX_LEN];
    app_sound_bt_remote_info_t remote_info[BT_REMOTE_DEVICE_NUM];
};

struct app_sound_wifi_device_info_t {
    uint8_t acl_state;
    uint8_t profile_state;
    uint8_t sound_state;
};

struct app_sound_factory_device_info_t {
    device_addr_t device_addr;
    uint8_t name_len;
    uint8_t device_name[SOUND_NAME_MAX_LEN];
};

typedef struct
{
    uint8_t product_type:       4;
    uint8_t reserved0:          2;
    uint8_t reserved1:          2;
} __attribute__((packed)) sound_product_info_t;

typedef struct
{
    SOUND_ACL_STATUS_TYPE_E acl_state;
    SOUND_BT_A2DP_STATUS_TYPE_E a2dp_state;
    SOUND_BT_HFP_STATUS_TYPE_E hfp_state;
    SOUND_BT_SCO_STATUS_TYPE_E sco_state;
} __attribute__((packed)) sound_share_bt_info_t;

typedef struct
{
    uint8_t acl_state;
    uint8_t profile_state;
} __attribute__((packed)) sound_share_wifi_info_t;

typedef struct
{
    uint8_t operation:      4;
    uint8_t sound_role:       2;
    uint8_t reserved0:          2;
} __attribute__((packed)) sound_tws_info_t;

typedef struct
{
    device_addr_t                       master_addr;
    device_addr_t                       slave_addr;
    uint8_t                             name_len;
    uint8_t                             device_name[SOUND_NAME_MAX_LEN];
}__attribute__((packed)) sound_tws_config_adv_info_t;

typedef struct
{
    bool                                is_use;
    int8_t                              peer_rssi_value;
    sound_product_info_t                peer_product_info;
    device_addr_t                       peer_device_addr;
    sound_share_bt_info_t               peer_bt_remote_info[BT_REMOTE_DEVICE_NUM];
    sound_share_wifi_info_t             peer_wifi_status;
    uint16_t                            peer_scan_recv_cnt;
} __attribute__((packed)) sound_ble_scan_data_store_cache_table_t;

struct app_sound_ctrl_t {
    uint8_t onoff_status;                                         //Do shutdown and standby state management
    uint8_t business_device_type;                                 //Distinguish between BT devices and WIFI device business types
    uint8_t business_mode;                                        //Record the device type of the current business
    bool    aux_in;                                               //linein or usb input

    bool sm_running;
    SOUND_UI_STATE_E super_state;
    SOUND_UI_STATE_E sub_btsuper_state;
    SOUND_UI_STATE_E sub_wifisuper_state;
    SOUND_UI_STATE_E wait_state;
    SOUND_EVENT_TYPE active_event;
    sound_event_manager_t event_q_manager;
    bool tws_connect_fail_enter_freeman;

    osTimerId_t sound_timer;
    bool sound_timer_start;
    uint16_t sound_timer_status;
    osTimerId_t sound_adv_timer;
    app_sound_factory_device_info_t factory_device_info;
    app_sound_bt_device_info_t bt_device_info;
    app_sound_wifi_device_info_t wifi_device_info;
    uint8_t sound_tws_adv_cfg_cnt;
    uint8_t adv_data_info[SOUND_ADV_DATA_MAX_LEN];
    sound_ble_scan_data_store_cache_table_t ble_scan_data_cache[BLE_SCAN_DATA_STORE_CACHE_TABLE_SIZE];
    sound_tws_config_adv_info_t tws_config_info;
};

typedef struct
{
    sound_tws_info_t                    tws_info;
    sound_product_info_t                product_info;
    device_addr_t                       device_addr;

    sound_share_bt_info_t               bt_remote_info[BT_REMOTE_DEVICE_NUM];
    sound_share_wifi_info_t             wifi_status;
    uint16_t                            rtc_time;
    uint8_t                             crc_check;
} __attribute__((packed)) sound_ble_adv_data_tws_discover_ind_t;

typedef struct
{
    SOUND_AUDIO_STATUS_TYPE_E           audio_status;
    uint8_t                             audio_device_type;
    uint8_t                             bt_acl_connected_num;
    uint8_t                             wifi_acl_status;
}__attribute__((packed)) sound_device_status_info_t;

typedef struct
{
    sound_tws_info_t                    tws_info;
    sound_device_status_info_t          device_status_info;
    sound_tws_config_adv_info_t         tws_config_info;
    uint8_t                             crc_check;
}__attribute__((packed)) sound_ble_adv_data_tws_comfirm_dev_t;

typedef struct
{
    sound_tws_info_t                    tws_info;
    sound_tws_config_adv_info_t         tws_config_info;
    uint8_t                             crc_check;
}__attribute__((packed)) sound_adv_data_tws_perhandle_t;

struct app_sound_sync_event_t
{
    uint8_t opcode;
    uint8_t data_len;
    SOUND_EVENT_TYPE event;
};

#ifdef __cplusplus
extern "C" {
#endif

struct app_sound_ctrl_t *app_sound_get_ctrl(void);

void app_sound_ui_event_handler(SOUND_ACTION_E action, SOUND_UI_ERROR_E status);

void app_sound_wifi_acl_state_handle(SOUND_ACL_STATUS_TYPE_E status);

void app_sound_airplay_state_handle(SOUND_WIFI_AIRPLAY_STATUS_TYPE_E status);

void app_sound_bredr_acl_state_handle(uint8_t device_id, device_addr_t *addr, SOUND_ACL_STATUS_TYPE_E status, uint8_t reason_code);

void app_sound_a2dp_state_handle(uint8_t device_id, device_addr_t *addr, SOUND_BT_A2DP_STATUS_TYPE_E status);

void app_sound_hfp_state_handle(uint8_t device_id, device_addr_t *addr, SOUND_BT_HFP_STATUS_TYPE_E status);

void app_sound_sco_state_handle(uint8_t device_id, device_addr_t *addr, SOUND_BT_SCO_STATUS_TYPE_E status);

void app_sound_ui_event_entry(SOUND_UI_TYPE_E event);

void app_sound_key_handle(SOUND_EVENT_TYPE sound_key);

void app_sound_init(void);

void app_sound_bt_init(void);

void app_sound_ui_test_key_init(void);

void app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_TYPE_E type, uint8_t reason_code);

#ifdef __cplusplus
}
#endif

#endif //APP_SOUND_UI_ENABLE

#endif

