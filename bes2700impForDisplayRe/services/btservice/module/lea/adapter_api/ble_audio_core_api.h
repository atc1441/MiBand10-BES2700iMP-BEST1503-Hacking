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

#ifndef __BLE_AUDIO_CORE_API_H__
#define __BLE_AUDIO_CORE_API_H__

#include "bluetooth_ble_api.h"
#include "ble_audio_core_evt.h"

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

typedef enum
{
    LEA_REQ_ASE_START,
    LEA_REQ_ASE_CODEC_CONFIGURE,
    LEA_REQ_ASE_QOS_CONFIGURE,
    LEA_REQ_ASE_ENABLE,
    LEA_REQ_ASE_DISABLE,
    LEA_REQ_ASE_RELEASE, /// 5

    LEA_EVT_ASE_CODEC_CONFIGURED,
    LEA_EVT_ASE_UC_GRP_CREATED,
    LEA_EVT_ASE_QOS_CONFIGURED,
    LEA_EVT_ASE_ENABLED,
    LEA_EVT_ASE_DISABLED, /// 10
    LEA_EVT_ASE_RELEASED,
    LEA_EVT_ASE_IDLE,
    LEA_EVT_ASE_UC_GRP_REMOVED,

    LEA_EVT_ASE_TIMEOUT,
    LEA_ASE_EVENT_MAX,
} LEA_ASE_STM_EVENT_E;

typedef enum
{
    LEA_ASE_STATE_IDLE,
    LEA_ASE_STATE_CODEC_CONFIGUED,
    LEA_ASE_STATE_QOS_CONFIGUED,
    LEA_ASE_STATE_ENABLING,
    LEA_ASE_STATE_STREAMING,
    LEA_ASE_STATE_DISABLING, /// 5
    LEA_ASE_STATE_RELEASING,

    LEA_ASE_STATE_UNKNOW,
} LEA_ASE_STM_STATE_E;

typedef enum {
    LEA_CI_MODE_SVC_DISC_START      = 0,
    LEA_CI_MODE_SVC_DISC_CMP        = 1,
    LEA_CI_MODE_ISO_DATA_ACT        = 2,
    LEA_CI_MODE_ISO_DATA_STOP       = 3,
    LEA_CI_MODE_DEFAULT             = 4,
} lea_ci_mode_t;

typedef struct
{
    uint32_t    mobile_reconnect_timeout;
    bool        is_ble_audio_central_self_pairing_feature_enable;
    uint32_t    ble_audio_central_start_alternate_scan_time_ms;
    uint32_t    ble_audio_central_start_alternate_reconn_time_ms;

}ble_audio_core_config;

#ifdef AOB_MOBILE_ENABLED
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
} ble_audio_mobile_core_config;
#endif

typedef struct
{
    uint16_t                     sample_rate;
    uint16_t                     frame_octet;
    bes_gaf_direction_t            direction;
    const bes_gaf_codec_id_t       *codec_id;
    uint16_t                    context_type;
} LEA_ASE_STM_CFG_INFO_T;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ble audio core initialize
 * @date 2021.07.20
 */
void ble_audio_core_init(uint8_t aud_cfg);

/**
 * @brief add these addresses to white list and start connectable adv
 *
 * @param addr
 * @param num_of_addr
 * @date 2021.07.20
 */
void ble_audio_start_mobile_connecting(ble_bdaddr_t *addr, uint8_t num_of_addr);

/**
 * @brief make a new leaudio sm
 *
 * @param conidx
 * @param peer_bdaddr
 * @date 2023.04.11
 */
bool ble_audio_make_new_le_core_sm(uint8_t conidx, uint8_t *peer_bdaddr);

/**
 * @brief disconnect the mobile device
 *
 * @param addr
 * @date 2021.07.20
 */
void ble_audio_mobile_req_disconnect(ble_bdaddr_t *addr);

/**
 * @brief return the ble link connect state
 *
 * @param addr
 * @date 2021.07.20
 */
bool ble_audio_is_mobile_link_connected(ble_bdaddr_t *addr);

/**
 * @brief disconnect the mobile device by connection id.
 *
 * @param connection id.
 * @date 2021.07.20
 */
void ble_audio_mobile_disconnect_device(uint8_t conidx);

/**
 * @brief disconnect all the leaudio mobile device
 *
 * @param addr
 * @date 2021.07.20
 */
void ble_audio_disconnect_all_connection(void);

/**
 * @brief determine if any ble audio mobile connected
 *
 * @return true
 * @return false
 * @date 2021.07.20
 */
bool ble_audio_is_any_mobile_connnected(void);


/**
 * @brief get ble-tws link conidx
 *
 * @return uint8_t
 * @date 2021.07.20
 */
uint8_t ble_audio_get_tws_link_conidx(void);


/**
 * @brief determine if ble-tws connected
 *
 * @return true
 * @return false
 * @date 2021.07.20
 */
bool ble_audio_tws_link_is_connected(void);


/**
 * @brief start tws connection, one earbud set white list, start adv and the other start connecting.
 *
 * @return true
 * @return false
 * @date 2021.07.20
 */
bool ble_audio_tws_connect_request(void);


/**
 * @brief cancel tws connecting request
 *
 * @return true
 * @return false
 * @date 2021.07.20
 */
bool ble_audio_tws_cancel_connecting_request(void);


/**
 * @brief disconnect tws link request
 *
 * @return true
 * @return false
 * @date 2021.07.20
 */
bool ble_audio_tws_disconnect_request(void);


/**
 * @brief dump ble audio core info, for debug
 *
 * @date 2021.07.20
 */
void ble_audio_dump_core_status();

/**
 * @brief get the local id of the connected mobile device
 *
 * @date 2021.08.05
 */
uint8_t ble_audio_get_mobile_connected_dev_lids(uint8_t con_lid[]);

/**
 * @brief get the remote device address by connection index
 *
 * @return connected device count
 * @date 2021.08.16
 */
ble_bdaddr_t *ble_audio_get_mobile_address_by_lid(uint8_t con_lid);

/**
 * @brief get the con_id by public address
 *
 * @return con_lid
 * @date 2021.08.16
 */
uint8_t ble_audio_get_mobile_lid_by_pub_address(uint8_t *pub_addr);

uint8_t ble_audio_query_ble_mobile_by_address(ble_bdaddr_t *addr);

/**
 * @brief Check if the device connection at this address is a le audio conn
 *
 * @return con_lid
 * @date 2021.08.16
 */
ble_audio_core_config* ble_audio_get_core_config();

void ble_audio_sync_info(void);

void ble_audio_connection_interval_mgr(uint8_t con_lid, lea_ci_mode_t mode);

const BLE_AUD_CORE_EVT_CB_T* ble_audio_get_evt_cb(void);

void ble_audio_core_evt_handle(ble_event_t *event);

uint8_t ble_audio_get_mobile_sm_index_by_addr(ble_bdaddr_t *addr);

void lea_ase_stm_send_msg_by_grp_lid(uint8_t grp_lid, LEA_ASE_STM_EVENT_E event, uint16_t status);

void ble_ase_sm_start_auto_play_by_con_lid(uint8_t conn_lid);

uint8_t lea_ase_stream_start(uint8_t con_lid, uint8_t cis_id, uint8_t ase_lid,
            uint8_t biDirection, LEA_ASE_STM_CFG_INFO_T* pInfo_local);

void lea_ase_stm_send_msg(uint8_t ase_lid, LEA_ASE_STM_EVENT_E evt, uint32_t para0, uint32_t para1);

uint8_t lea_ase_have_sm_by_ase_lid(uint8_t ase_lid);

bool ble_audio_mobile_conn_next_paired_dev(ble_bdaddr_t* bdaddr);

void ble_ase_stm_set_ase_stm_num_to_use(uint8_t con_lid, uint8_t ase_num);

void ble_ase_stm_increase_ase_stm_num_to_use(uint8_t con_lid, uint8_t step);

bool ble_audio_is_ux_mobile(void);

#ifdef AOB_MOBILE_ENABLED
void ble_mobile_start_connect(void);

void ble_mobile_connect_failed(bool is_failed);

bool ble_mobile_is_connect_failed(void);

uint8_t* ble_audio_mobile_conn_get_connecting_dev(void);

uint8_t ble_audio_discovery_modify_interval(uint8_t con_lid, DISCOVER_OPERATE operate, DISCOVER_SERVICE service);

bool ble_audio_mobile_conn_reconnect_dev(ble_bdaddr_t* bdaddr);

void ble_audio_mobile_conn_sm_send_event_by_conidx(uint8_t conidx, BLE_MOBILE_CONNECT_EVENT event, uint32_t param0, uint32_t param1);

void ble_audio_mobile_conn_sm_send_event_by_addr(ble_bdaddr_t* bdaddr, BLE_MOBILE_CONNECT_EVENT event, uint32_t param0, uint32_t param1);

ble_audio_mobile_core_config* ble_audio_mobile_get_core_config();

#endif

#ifdef __cplusplus
}
#endif

#endif
