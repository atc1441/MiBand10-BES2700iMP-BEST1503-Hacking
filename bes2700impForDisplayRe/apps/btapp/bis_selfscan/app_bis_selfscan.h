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
#ifndef __APP_BIS_SELFSCAN__
#define __APP_BIS_SELFSCAN__

#ifdef BIS_SELFSCAN_ENABLED

/*INCLUDE*/
#include "bluetooth.h"

/*DEFINITIONS*/
#define APP_BIS_SELFSCAN_BCAST_ID_LEN       (3)
#define APP_BIS_SELFSCAN_INVALID_SRC_LID    (0xFF)

/*ENUMERATIONS*/
enum selfscan_sync_state
{
    /// SYNC success
    APP_BIS_SELFSCAN_SYNC_SUCCESS = 0,
    /// SYNC ing
    APP_BIS_SELFSCAN_SYNC_RUNNING,
    /// SYNC failed
    APP_BIS_SELFSCAN_SYNC_FAILED,
};

enum selfscan_scan_cmd
{
    APP_BIS_SELFSCAN_SCAN_START = 0,
    APP_BIS_SELFSCAN_SCAN_STOP = 1,
};

enum selfscan_state
{
    APP_BIS_SELFSCAN_STATE_IDLE = 0,
    APP_BIS_SELFSCAN_STATE_RECV = 1,
    APP_BIS_SELFSCAN_STATE_BUSY = 2,
};

enum selfscan_code
{
    APP_BIS_SELFSCAN_ASCLL = 0,
    APP_BIS_SELFSCAN_UTF8 = 1,
    APP_BIS_SELFSCAN_UNICODE = 2,
};

enum selfscan_opcode
{
    // Opcode min
    APP_BIS_SELFSCAN_OP_MIN = 0,
    /// Scan state get
    APP_BIS_SELFSCAN_OP_GET_SCAN_STATE = 0x39,
    /// Scan result get
    APP_BIS_SELFSCAN_OP_GET_SELECT_SRC = 0x3A,
    /// Scan Command
    APP_BIS_SELFSCAN_OP_SCAN_COMMAND   = 0x79,
    /// Source select command
    APP_BIS_SELFSCAN_OP_SRC_SELECT_CMD = 0x7A,
    /// Source term sync
    APP_BIS_SELFSCAN_OP_STOP_SYNC      = 0x7B,
    /// Source broadcast code set
    APP_BIS_SELFSCAN_OP_BCAST_CODE     = 0x7C,
    /// Scan Result notify
    APP_BIS_SELFSCAN_OP_SCAN_RESULT_NTF = 0x8C,
    /// Select src notify
    APP_BIS_SELFSCAN_OP_SELECT_SRC_NTF = 0x8D,
    /// Broadcast code request
    APP_BIS_SELFSCAN_OP_BCAST_CODE_REQ_NTF = 0x8E,
    /// Opcode max
    APP_BIS_SELFSCAN_OP_MAX,
};

/*STRUCTURES*/
struct app_bis_src_scan_result
{
    // Src adv addr
    ble_bdaddr_t src_addr;
    // Src adv sid
    uint8_t adv_sid;
    // Broadcast ID
    uint8_t broadcast_id[APP_BIS_SELFSCAN_BCAST_ID_LEN];
    // Broadcast Encryption
    bool encrypted;
    // Broadcast Name length  (If PBA is not preset and this will be 0)
    uint8_t broadcast_name_len;
    // Broadcast Name (If PBA is not preset and this will be NONE)
    uint8_t broadcast_name[0];
};

/*CALLBACKS*/
/// Callback for app_bis_selfscan scan state
typedef void (*app_bis_selfscan_cb_scan_state)(bool started);
/// Callback for app_bis_selfscan scan state
typedef void (*app_bis_selfscan_cb_scan_result)(bool src_lid, const struct app_bis_src_scan_result *p_result);
/// Callback for app_bis_selfscan sync state
typedef void (*app_bis_selfscan_cb_sync_state)(const ble_bdaddr_t *p_src_addr, uint8_t adv_sid, enum selfscan_sync_state sync_state, uint16_t error);
/// Callback for app_bis_selfscan broadcast code required
typedef void (*app_bis_selfscan_cb_bcast_code_req)(bool src_lid);

/*Structure*/
typedef struct app_bis_selfscan_evt_cb
{
    /// Callback function called when scan state updated
    app_bis_selfscan_cb_scan_state cb_scan_state;
    /// Callback function called when scan result generated
    app_bis_selfscan_cb_scan_result cb_scan_result;
    /// Callback function called when sync state changed
    app_bis_selfscan_cb_sync_state cb_sync_state;
    /// Callback function called when broadcast code required
    app_bis_selfscan_cb_bcast_code_req cb_bcast_code_req;

} app_bis_selfscan_evt_cb_t;

typedef struct app_bis_selfscan_init_cfg
{
    /// Maximum scan result stored
    uint8_t max_result_present;
    /// Scan timeout in second, 0 means forever
    uint16_t scan_timeout_s;
    /// Sync PA/BIS timeout in second, 0 means forever
    uint16_t sync_timeout_s;

} app_bis_selfscan_init_cfg_t;

/*FUNCTIONS DECALRATION*/
int app_bis_selfscan_init(const app_bis_selfscan_init_cfg_t *p_init_cfg, const app_bis_selfscan_evt_cb_t *p_evt_cb);
int app_bis_selfscan_deinit(void);

#endif /* BIS_SELFSCAN_ENABLED */

#endif /*__APP_BIS_SELFSCAN__*/