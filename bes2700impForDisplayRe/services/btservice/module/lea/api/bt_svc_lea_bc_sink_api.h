/**
 * @brief Bluetooth Service Low Energy Audio Broadcast Sink Application Programming Interface
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
#ifndef __BT_SVC_LEA_BC_SINK_API_H__
#define __BT_SVC_LEA_BC_SINK_API_H__

/*****************************header include********************************/
#include <stdint.h>

/******************************macro defination*****************************/
#define BT_SVC_LEA_BC_SINK_VOLUME_INVALID       0xFF

/******************************type defination******************************/
typedef enum 
{
    // Local volume adjustment
    BT_SVC_LEA_BC_SINK_LOCAL,
    // Remote device volume adjustment
    BT_SVC_LEA_BC_SINK_REMOTE,
} BT_SVC_LEA_BC_SINK_VOLUME_SOURCE;

typedef enum
{
    // Module closed state
    BT_SVC_LEA_BC_SINK_STATE_CLOSE = 0,
    // Module opened ed state
    BT_SVC_LEA_BC_SINK_STATE_OPENED,
    // Module stream state
    BT_SVC_LEA_BC_SINK_STATE_STREAM_STARTED,
    // Module unknow state
    BT_SVC_LEA_BC_SINK_STATE_UNKNOWN,
} BT_SVC_LEA_BC_SINK_STATE_E;

typedef enum
{
    // Stream start event, param: NULL
    BT_SVC_LEA_BC_SINK_EVENT_STREAM_START,
    // Stream stop event, param: NULL
    BT_SVC_LEA_BC_SINK_EVENT_STREAM_STOP = 3,
    // The data carried by the manufacturer in the received ADV
    BT_SVC_LEA_BC_SINK_EVENT_SINK_ADV_DATA_MANUFACTURER,
    // Unkonw event
    BT_SVC_LEA_BC_SINK_EVENT_UNKNOWN,
} BT_SVC_LEA_BC_SINK_EVENT_E;

typedef enum
{
    /// Multiple bits can be selected simultaneously
    // No strategy selected, play music from the first scanned device
    BT_SVC_LEA_BC_SINK_SYNC_POLICY_BCAST_NO   = 0,
    // Filter by advertising ID, bit 0
    BT_SVC_LEA_BC_SINK_SYNC_POLICY_BCAST_ID   = 1,
    // Filter by device name, bit 1
    BT_SVC_LEA_BC_SINK_SYNC_POLICY_BCAST_NAME = 2,
    // Filter by company ID, bit 2
    BT_SVC_LEA_BC_SINK_SYNC_POLICY_COMPANY_ID = 4,
} BT_SVC_LEA_BC_SINK_POLICY_E;

typedef struct
{
    /// bis transport module state @BT_SVC_LEA_BC_SINK_STATE_E
    uint8_t state;
} bt_svc_lea_bc_sink_get_info_t;

typedef struct
{
    // Channel selection bit
    uint32_t ch_bf;
    // Broadcast ID
    uint8_t  broadcast_id[3];
    // Broadcast encryption key
    uint8_t  encrypt_key[16];
    // Synchronize volume, 0:disenable, 1:enable
    uint8_t  sync_volume;
    // Acceptance selection criteria, see@BT_SVC_LEA_BC_SINK_POLICY_E
    uint8_t  sync_policy;
    // Company ID
    uint16_t company_id;
    // Company name
    const char* bcast_name;
    // Callback function for event notification
    void (*event_callback)(uint8_t event_type, uint8_t *param, uint8_t param_len);
} bt_svc_lea_bc_sink_param_t;
/****************************function declearation**************************/
/**
 ****************************************************************************************
 * @brief Open bis sink transport function
 * @param[in]: sink_param, bis transport sink open param
 * @param[out]: error code, see@BT_SVC_LEA_BC_SINK_ERROR_CODE_E
 ****************************************************************************************
 */
uint8_t bt_svc_lea_bc_sink_open(bt_svc_lea_bc_sink_param_t *sink_param);

/**
 ****************************************************************************************
 * @brief Close bis sink transport function
 * @param[out]: error code, see@BT_SVC_LEA_BC_SINK_ERROR_CODE_E
 ****************************************************************************************
 */
uint8_t bt_svc_lea_bc_sink_close(void);

/**
 ****************************************************************************************
 * @brief Get bis transport module info function
 * @param[in]: info, bis transport info ptr
 * @param[out]: error code, see@BT_SVC_LEA_BC_SINK_ERROR_CODE_E
 ****************************************************************************************
 */
uint8_t bt_svc_lea_bc_sink_get_info(bt_svc_lea_bc_sink_get_info_t *info);

#endif //__BT_SVC_LEA_API_H__
