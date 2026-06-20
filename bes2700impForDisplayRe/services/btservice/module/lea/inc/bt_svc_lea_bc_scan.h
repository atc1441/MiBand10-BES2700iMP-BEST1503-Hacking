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
#ifndef __BT_SVC_LEA_BC_SCAN__
#define __BT_SVC_LEA_BC_SCAN__

/*INCLUDE*/
#include "bluetooth.h"

/*DEFINITIONS*/
#define BT_SVC_SCAN_BCAST_ID_LEN       (3)
#define BT_SVC_SCAN_INVALID_SRC_LID    (0xFF)

/*ENUMERATIONS*/
enum bt_svc_lea_bc_scan_sync_state
{
    /// SYNC success
    BT_SVC_SCAN_SYNC_SUCCESS = 0,
    /// SYNC ing
    BT_SVC_SCAN_SYNC_RUNNING,
    /// SYNC failed
    BT_SVC_SCAN_SYNC_FAILED,
};

enum bt_svc_lea_bc_scan_cmd
{
    BT_SVC_SCAN_SCAN_START = 0,
    BT_SVC_SCAN_SCAN_STOP = 1,
};

enum bt_svc_lea_bc_scan_state
{
    BT_SVC_SCAN_STATE_IDLE = 0,
    BT_SVC_SCAN_STATE_RECV = 1,
    BT_SVC_SCAN_STATE_BUSY = 2,
};

enum bt_svc_lea_bc_scan_code
{
    BT_SVC_SCAN_ASCLL = 0,
    BT_SVC_SCAN_UTF8 = 1,
    BT_SVC_SCAN_UNICODE = 2,
};

enum bt_svc_lea_bc_scan_opcode
{
    // Opcode min
    BT_SVC_SCAN_OP_MIN = 0,
    /// Scan state get
    BT_SVC_SCAN_OP_GET_SCAN_STATE = 0x39,
    /// Scan result get
    BT_SVC_SCAN_OP_GET_SELECT_SRC = 0x3A,
    /// Scan Command
    BT_SVC_SCAN_OP_SCAN_COMMAND   = 0x79,
    /// Source select command
    BT_SVC_SCAN_OP_SRC_SELECT_CMD = 0x7A,
    /// Source term sync
    BT_SVC_SCAN_OP_STOP_SYNC      = 0x7B,
    /// Source broadcast code set
    BT_SVC_SCAN_OP_BCAST_CODE     = 0x7C,
    /// Scan Result notify
    BT_SVC_SCAN_OP_SCAN_RESULT_NTF = 0x8C,
    /// Select src notify
    BT_SVC_SCAN_OP_SELECT_SRC_NTF = 0x8D,
    /// Broadcast code request
    BT_SVC_SCAN_OP_BCAST_CODE_REQ_NTF = 0x8E,
    /// Opcode max
    BT_SVC_SCAN_OP_MAX,
};

/*STRUCTURES*/
struct bt_svc_lea_bc_scan_result
{
    // Src adv addr
    ble_bdaddr_t src_addr;
    // Src adv sid
    uint8_t adv_sid;
    // Broadcast ID
    uint8_t broadcast_id[BT_SVC_SCAN_BCAST_ID_LEN];
    // Broadcast Encryption
    bool encrypted;
    // Broadcast Name length  (If PBA is not preset and this will be 0)
    uint8_t broadcast_name_len;
    // Broadcast Name (If PBA is not preset and this will be NONE)
    uint8_t broadcast_name[0];
};

/*Structure*/
typedef struct
{
    /// Callback function called when scan state updated
    void (*cb_scan_state)(bool started);
    /// Callback function called when scan result generated
    void (*cb_scan_result)(bool src_lid, const struct app_bis_src_scan_result *p_result);
    /// Callback function called when sync state changed
    void (*cb_sync_state)(const ble_bdaddr_t *p_src_addr, uint8_t adv_sid, enum selfscan_sync_state sync_state, uint16_t error);
    /// Callback function called when broadcast code required
    void (*cb_bcast_code_req)(bool src_lid);
} bt_svc_lea_bc_scan_evt_cb_t;

typedef struct
{
    /// Maximum scan result stored
    uint8_t max_result_present;
    /// Scan timeout in second, 0 means forever
    uint16_t scan_timeout_s;
    /// Sync PA/BIS timeout in second, 0 means forever
    uint16_t sync_timeout_s;
    const bt_svc_lea_bc_scan_evt_cb_t* callback;
} bt_svc_lea_bc_scan_init_cfg_t;

/*FUNCTIONS DECALRATION*/
int bt_svc_lea_bc_scan_init(const bt_svc_lea_bc_scan_init_cfg_t *p_init_cfg);

#endif /* __BT_SVC_LEA_SCAN__ */