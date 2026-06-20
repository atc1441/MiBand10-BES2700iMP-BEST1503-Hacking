/**
 * @brief Bluetooth Service Low Energy Audio Unicast Client Application Programming Interface
 *
 * @copyright Copyright (c) 2015-20223 BES Technic.
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
 */
#ifndef __BT_SVC_LEA_UC_CLIENT_API_H__
#define __BT_SVC_LEA_UC_CLIENT_API_H__

/*****************************header include********************************/
#include "nvrecord_extension.h"
#include "bes_aob_api.h"

/******************************macro defination*****************************/
#ifdef AOB_MOBILE_ENABLED
/******************************type defination******************************/
typedef enum{
    DISCOVER_START    = 0,
    DISCOVER_COMPLEPE = 1,

    DISCOVER_MAX      = 3,
}DISCOVER_OPERATE;

typedef enum{
    SERVICE_ACC_MCC = 0,
    SERVICE_ACC_TBC = 1,

    SERVICE_MAX     = 2,
}DISCOVER_SERVICE;

typedef enum
{
    REQ_BLE_RECONNECT,
    REQ_BLE_DISCONNECT,

    EVT_BLE_RECONNECTING_CANCELLED,
    EVT_BLE_CONNECTED,
    EVT_BLE_RECONNECT_BOND_FAIL,
    EVT_BLE_RECONNECT_FAILED,
    EVT_BLE_RECONNECT_TIMEOUT,
    EVT_BLE_DISCONNECTED,
}BLE_MOBILE_CONNECT_EVENT;

typedef struct
{
    bool                    connected;
    uint8_t                 conidx;
    BLE_ADDR_INFO_T         peer_ble_addr;
} AOB_MOBILE_PHONE_INFO_T;

typedef struct
{
    // true: slave address will be set as dongle role
    // false: device role is determined by address of g_tws_pairing_info
    uint8_t lea_central_pairing_enable : 1;
    // true: audio output switch to PC if there is no link exist 
    // false: Even without a link, the audio doesn't switch to the PC
    uint8_t audio_routing_on_link : 1;
    uint8_t rfu : 6;
    //mobile_reconnect_timeout
    uint32_t mobile_reconnect_timeout;
}ble_audio_mobile_core_config;

/****************************function declearation**************************/
void bt_svc_lea_uc_client_info_init(void);
void bt_svc_lea_uc_client_info_connected_set(uint8_t conidx, uint8_t* earbud_ble_addr, uint8_t earbud_addr_type);
void bt_svc_lea_uc_client_info_clear(uint8_t conidx);
void bt_svc_lea_uc_client_single_call_info_clear(uint8_t conidx, uint8_t call_id);
AOB_MOBILE_PHONE_INFO_T *bt_svc_lea_uc_client_info_get(uint8_t conidx);
uint8_t bt_svc_lea_uc_client_get_active_conidx(void);
AOB_SINGLE_CALL_INFO_T *bt_svc_lea_uc_client_info_get_call_info_by_id(uint8_t conidx, uint8_t call_id);
void bt_svc_lea_uc_client_info_clear_call_info(uint8_t conidx, uint8_t call_id);

AOB_SINGLE_CALL_INFO_T *bt_svc_lea_uc_client_get_call_info_by_state(uint8_t conidx, AOB_CALL_STATE_E state);
AOB_SINGLE_CALL_INFO_T *bt_svc_lea_uc_client_get_call_info_by_bearer_lid(uint8_t conidx, uint8_t bearer_lid);
void bt_svc_lea_uc_client_info_bond_bearer_id(uint8_t conidx, uint8_t bearer_lid);
void bt_svc_lea_uc_client_info_update_call_id(uint8_t conidx, uint8_t call_id);
bool bt_svc_lea_uc_client_both_is_connected(void);
bool bt_svc_lea_uc_client_is_any_device_connected(void);
bool bt_svc_lea_uc_client_check_device_connected(uint8_t *p_addr);

void bt_svc_lea_uc_client_register_event_cb(const BLE_AUD_MOB_CORE_EVT_CB_T *cb);

const BLE_AUD_MOB_CORE_EVT_CB_T* bt_svc_lea_uc_client_get_evt_cb();

void bt_svc_lea_uc_client_start_connecting(ble_bdaddr_t *addr);

void bt_svc_lea_uc_client_conn_sm_send_event_by_conidx(uint8_t conidx, BLE_MOBILE_CONNECT_EVENT event, uint32_t param0, uint32_t param1);

void bt_svc_lea_uc_client_conn_sm_send_event_by_addr(ble_bdaddr_t* bdaddr, BLE_MOBILE_CONNECT_EVENT event, uint32_t param0, uint32_t param1);

uint8_t* bt_svc_lea_uc_client_conn_get_connecting_dev(void);

void bt_svc_lea_uc_client_connect_failed(bool is_failed);

bool bt_svc_lea_uc_client_is_connect_failed(void);

bool bt_svc_lea_uc_client_conn_next_paired_dev(ble_bdaddr_t *bdaddr);

void bt_svc_lea_uc_client_start_connect(void);

uint8_t bt_svc_lea_uc_client_discovery_modify_interval(uint8_t con_lid, DISCOVER_OPERATE operate, DISCOVER_SERVICE service);

void bt_svc_lea_uc_client_core_register_event_cb(const BLE_AUD_MOB_CORE_EVT_CB_T *cb);

ble_audio_mobile_core_config* bt_svc_lea_uc_client_get_core_config();
#endif

typedef enum
{
    // One audio channel plays synchronously locally and remotely
    BT_SVC_LEA_UC_CLI_PLAY_MONO=0,
    // Two audio channels, one channel plays locally and the other
    // channel plays synchronously on remote devices
    BT_SVC_LEA_UC_CLI_PLAY_ONE_CIS,
    // Two audio channels are placed in one stream and played
    // synchronously on both local and remote devices
    BT_SVC_LEA_UC_CLI_PLAY_TWO_CIS,
    // 
    BT_SVC_LEA_UC_CLI_PLAY_UNKONW,
} BT_SVC_LEA_UC_CLI_PLAY_MODE_E;

typedef enum
{
    BT_SVC_LEA_UC_CLI_EVENT_ACL_CONN = 0,
    BT_SVC_LEA_UC_CLI_EVENT_ACL_DISCONN,
} BT_SVC_LEA_UC_CLI_EVENT_E;

typedef struct
{
    
} bt_svc_lea_uc_cli_event_apram_t;

typedef struct
{
    uint8_t*    addr;
    const char* name;
} bt_svc_lea_uc_cli_pairing_t;

typedef struct
{
    // see@BT_SVC_AUDIO_INPUT_TYPE_E
    uint8_t input_type;
    uint8_t local_paly;
    uint8_t play_mode;
    uint8_t pairing_dev_num;
    bt_svc_lea_uc_cli_pairing_t* pairing_dev;

    void (*event_callback)(uint8_t event_type, bt_svc_lea_uc_cli_event_apram_t* param);
} bt_svc_lea_uc_cli_param_t;

uint8_t bt_svc_lea_uc_cli_open(bt_svc_lea_uc_cli_param_t* param);

uint8_t bt_svc_lea_uc_cli_close();

#endif //__BT_SVC_LEA_API_H__
