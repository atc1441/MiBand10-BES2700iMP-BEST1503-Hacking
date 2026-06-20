/***************************************************************************
 *
 * Copyright (c) 2015-2023 BES Technic
 *
 * Authored by BES CD team (Blueelf Prj).
 *
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
#ifndef __BAP_ASC_COMMON__
#define __BAP_ASC_COMMON__

#include "bluetooth.h"

#include "gaf_cfg.h"
#include "generic_audio.h"

#include "bap_broadcast_common.h"

/// Maximum length supported for Metadata
#define BASS_METADATA_LEN_MAX                   (255)
/// Minimal length of Broadcast Audio Scan Control Point characteristic value
#define BASS_CP_LEN_MIN                         (1)
/// Failed to synchronized to BIG value for BIS Sync State
#define BASS_BIG_SYNC_FAILED                    (0xFFFFFFFF)
/// No preference for synchronization with BIG
#define BASS_BIG_SYNC_NO_PREF                   (0xFFFFFFFF)
/// Unknown Periodic Advertising interval
#define BASS_UNKNOWN_PA_INTV                    (0xFFFF)
/// Maximum Presentation Delay for Broadcast (in microseconds)
#define BASS_MAX_PRES_DELAY_US                  (0x0000FFFF)
/// Length of Broadcast Id
#define BASS_BROADCAST_ID_LEN                   (3)
/// Maximum number of Subgroups in a Group
#define BASS_NB_SUBGROUPS_MAX                   (31)
/// Length of header for Basic Audio Announcement (without length field)
#define BASS_BASIC_AUDIO_ANNOUNCE_HEAD_LENGTH   (3)
/// Minimal length of Broadcast Audio Announcement
#define BASS_BCAST_AUDIO_ANNOUNCEMENT_LEN       (6)

#define BASS_RX_SRC_INVLAID_ID                  (GAF_INVALID_ANY_LID)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

enum bass_char_type
{
    BASS_CHAR_TYPE_MIN = 0,
    /// Broadcast Audio BC Scan Control Point
    BASS_CHAR_TYPE_BC_SCAN_CP = BASS_CHAR_TYPE_MIN,
    /// Broadcast Receiver state
    BASS_CHAR_TYPE_BC_RX_STATE,
    /// Char Type Max
    BASS_CHAR_TYPE_MAX,
};

/// Values for PA Sync field in Broadcast Audio Scan Control Point characteristic value
enum bass_pa_sync
{
    /// Do not synchronize to PA
    BASS_PA_SYNC_NO_SYNC = 0,
    /// Synchronize to PA, PAST on client
    BASS_PA_SYNC_SYNC_PAST,
    /// Synchronize to PA, no PAST on client
    BASS_PA_SYNC_SYNC_NO_PAST,

    BASS_PA_SYNC_MAX
};

/// Source Address Type values // TODO should use the GAP define instead
enum bass_src_addr_type
{
    /// Public Device Address
    BASS_SRC_ADDR_TYPE_PUBLIC = 0,
    /// Random Device Address
    BASS_SRC_ADDR_TYPE_RANDOM,

    /// Maximum allowed value
    BASS_SRC_ADDR_TYPE_MAX,
};

/// Operation codes for Broadcast Audio Scan Control Point characteristic
enum bass_opcodes
{
    /// Remote Scan Stop
    BASS_OPCODE_REMOTE_SCAN_STOPPED = 0,
    /// Remote Scan Start
    BASS_OPCODE_REMOTE_SCAN_STARTED,
    /// Add Source
    BASS_OPCODE_ADD_SOURCE,
    /// Modify Source
    BASS_OPCODE_MODIFY_SOURCE,
    /// Set Broadcast Code
    BASS_OPCODE_SET_BROADCAST_CODE,
    /// Remove Source
    BASS_OPCODE_REMOVE_SOURCE,

    BASS_OPCODE_MAX,
};

/// Position of fields in Broadcast Receive State characteristic value
enum bass_rx_state_pos
{
    /// Source ID
    BASS_RX_STATE_SRC_ID_POS = 0,
    /// Source Address Type
    BASS_RX_STATE_SRC_ADDR_TYPE_POS,
    /// Source Address
    BASS_RX_STATE_SRC_ADDR_POS,
    /// Source Advertising SID
    BASS_RX_STATE_SRC_ADV_SID_POS = BASS_RX_STATE_SRC_ADDR_POS + 6,
    /// Broadcast ID
    BASS_RX_STATE_BCAST_ID_POS,
    /// PA Sync State
    BASS_RX_STATE_PA_SYNC_STATE_POS = BASS_RX_STATE_BCAST_ID_POS + 3,
    /// BIG Encryption
    BASS_RX_STATE_BIG_ENCRYPTION_POS,
    /// Bad Code
    BASS_RX_STATE_BAD_CODE_POS,

    /// BIS Sync State
    BASS_RX_STATE_BIS_SYNC_STATE_POS = 0,
    /// Metadata length
    BASS_RX_STATE_METADATA_LEN_POS = BASS_RX_STATE_BIS_SYNC_STATE_POS + 4,
    /// Metadata
    BASS_RX_STATE_METADATA_POS,

    /// Length of Broadcast Receive State characteristic value (+ 1 for Number of Subgroups field)
    BASS_RX_STATE_MIN_LEN = BASS_RX_STATE_BAD_CODE_POS + 1,
    /// Minimum length of Subgroup information
    BASS_RX_STATE_SGRP_MIN_LEN = BASS_RX_STATE_METADATA_POS,
};

/// Values for PA Sync State subfield in Scan State field in Broadcast Receive State characteristic value
enum bass_rx_state_pa
{
    /// Not synchronized to PA
    BASS_RX_STATE_PA_NOT_SYNCED = 0,
    /// SyncInfo Request
    BASS_RX_STATE_PA_SYNCINFO_REQ,
    /// Synchronized to PA
    BASS_RX_STATE_PA_SYNCED,
    /// Failed to synchronize with PA
    BASS_RX_STATE_PA_FAILED,
    /// No PAST
    BASS_RX_STATE_PA_NO_PAST,

    BASS_RX_STATE_PA_MAX,
};

/// Values for BIG encryption subfield in Scan State field in Broadcast Receive State characteristic value
enum bass_big_encrypt_state
{
    /// Not encrypted
    BASS_BIG_ENCRYPT_STATE_NOT_ENCRYPTED = 0,
    /// Broadcast code required
    BASS_BIG_ENCRYPT_STATE_CODE_REQUIRED,
    /// Decrypting
    BASS_BIG_ENCRYPT_STATE_DECRYPTING,
    /// Bad code
    BASS_BIG_ENCRYPT_STATE_BAD_CODE,

    BASS_BIG_ENCRYPT_STATE_MAX,
};

/// Error codes for Broadcast Audio Scan Service
enum bass_bass_app_err
{
    /// Opcode not supported
    BASS_APP_ERR_OPCODE_NOT_SUPPORTED = 0x80,
    /// Invalid Source ID
    BASS_APP_ERR_INVALID_SRC_ID,
};

/// Broadcast Audio Scan Control Point operation
typedef struct bass_bc_ctrl_operation
{
    /// BC control point opcode
    uint8_t op_code;
    /// BC control point operation param
    uint8_t param[GAF_ARRAY_EMPTY];
} __attribute__((packed)) bass_bc_op_t;

/// Subgroup info
typedef struct bass_bc_subgroup_info
{
    /// BIS_Sync
    uint32_t bis_syn_req_bf;
    /// Metadata_Length
    uint8_t metadata_len;
    /// Metadata
    uint8_t metadata[GAF_ARRAY_EMPTY];
} __attribute__((packed)) bass_bc_subgrp_t;

/// Add Source operation
typedef struct bass_bc_ctrl_operation_add_src
{
    /// Advertiser_Address_Type
    uint8_t adv_addr_type;
    /// Advertiser_Address
    uint8_t adv_address[6];
    /// Advertising_SID
    uint8_t adv_sid;
    /// Broadcast_ID
    uint8_t broadcast_id[BAP_BC_BROADCAST_ID_LEN];
    /// PA_Sync
    uint8_t pa_sync_req;
    /// PA_Interval
    uint16_t pa_interval;
    /// Num_Subgroups
    uint8_t num_sub_grp;
    /// Subgroup info param
    bass_bc_subgrp_t subgrp[GAF_ARRAY_EMPTY];
} __attribute__((packed)) bass_bc_op_add_src_t;

/// Modify Source operation
typedef struct bass_bc_ctrl_operation_modify_src
{
    /// Source_ID
    uint8_t src_id;
    /// PA_Sync
    uint8_t pa_sync_req;
    /// PA_Interval
    uint16_t pa_interval;
    /// Num_Subgroups
    uint8_t num_sub_grp;
    /// Subgroup info param
    bass_bc_subgrp_t subgrp[GAF_ARRAY_EMPTY];
} __attribute__((packed)) bass_bc_op_modify_src_t;

/// Set Broadcast_Code operation
typedef struct bass_bc_ctrl_operation_set_bc_code
{
    /// Source_ID
    uint8_t src_id;
    /// Broadcast_Code
    uint8_t broadcast_code[BAP_BC_BROADCAST_CODE_LEN];
} __attribute__((packed)) bass_bc_op_set_bc_code_t;

/// Remove Source operation
typedef struct bass_bc_ctrl_operation_remove_source
{
    /// Source_ID
    uint8_t src_id;
} __attribute__((packed)) bass_bc_op_remove_src_t;

#endif /// __BAP_ASC_COMMON__