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
#ifndef __HAP_COMMON__
#define __HAP_COMMON__

#include "bluetooth.h"

#include "gaf_cfg.h"

/// Minimal length of Preset name value
#define HAS_PRESET_NAME_LEN_MIN         (1)
/// Maximal length of Preset name value
#define HAS_PRESET_NAME_LEN_MAX         (40)
/// Minimal Preset Index value
#define HAS_PRESET_IDX_MIN              (1)

/// Characteristic type values for Hearing Access Service
enum has_char_type
{
    /// Preset Control Point characteristic
    HAS_CHAR_TYPE_PRESET_CP = 0,
    /// Active Preset Index characteristic
    HAS_CHAR_TYPE_ACTIVE_PRESET_INDEX,
    /// Hearing Aid Features characteristic
    HAS_CHAR_TYPE_FEATURES,
    /// Maximum value
    HAS_CHAR_TYPE_MAX,
};

/// Error code values for Preset Control Point characteristic
enum has_cp_error
{
    /// Invalid Opcode
    HAS_CP_ERROR_INVALID_OPCODE = 0x80,
    /// Write Name Not Allowed
    HAS_CP_ERROR_WRITE_NAME_NOT_ALLOWED,
    /// Preset Synchronization Not Supported
    HAS_CP_ERROR_PRESET_SYNC_NOT_SUPPORTED,
    /// Preset Operation Not Possible
    HAS_CP_ERROR_PRESET_OP_NOT_POSSIBLE,
    /// Invalid Parameters Length with valid opcode
    HAS_CP_ERROR_PRESET_OP_INVALID_PARAM,
};

/// Operation codes for Preset Control Point characteristic
enum has_cp_opcode
{
    /// Reserve for future use
    HAS_CP_OPCODE_RFU = 0,
    /// Read Preset by Index
    HAS_CP_OPCODE_READ_PRESET,
    /// Read Preset Response
    HAS_CP_OPCODE_READ_PRESET_RSP,
    /// Preset Changed
    HAS_CP_OPCODE_PRESET_CHANGED,
    /// Write Preset Name
    HAS_CP_OPCODE_WRITE_PRESET_NAME,
    /// Set Active Preset
    HAS_CP_OPCODE_SET_ACTIVE_PRESET,
    /// Set Next Preset
    HAS_CP_OPCODE_SET_NEXT_PRESET,
    /// Set Previous Preset
    HAS_CP_OPCODE_SET_PREV_PRESET,
    /// Set Preset - Synchronized Locally
    HAS_CP_OPCODE_SET_PRESET_SYNC,
    /// Set Next Preset - Synchronized Locally
    HAS_CP_OPCODE_SET_NEXT_PRESET_SYNC,
    /// Set Previous Preset - Synchronized Locally
    HAS_CP_OPCODE_SET_PREV_PRESET_SYNC,

    HAS_CP_OPCODE_MAX,
};

/// Hearing Aid Type values
enum has_features_type
{
    /// Binaural Hearing Aid
    HAS_FEATURES_TYPE_BINAURAL = 0,
    /// Monaural Hearing Aid
    HAS_FEATURES_TYPE_MONAURAL,
    /// Banded Hearing Aid
    HAS_FEATURES_TYPE_BANDED,
};

/// Hearing Aid Properties values
enum has_preset_prop_bf
{
    HAS_PRESET_PROP_WRITABLE_BIT    = CO_BIT(0),

    HAS_PRESET_PROP_AVAILABLE_BIT   = CO_BIT(1),
};

/// Hearing Aid Features characteristic value bit field meaning
enum has_features_bf
{
    /// Indicate the Hearing Aid type of the server (see #has_features_type enumeration) - Position
    HAS_FEATURES_TYPE_LSB = 0,
    /// Indicate the Hearing Aid type of the server (see #has_features_type enumeration) - Mask
    HAS_FEATURES_TYPE_MASK = 0x03,
    /// Indicate if Preset Synchronization is supported (= 1) or not - Position
    HAS_FEATURES_PRESET_SYNC_SUPPORT_POS = 2,
    /// Indicate if Preset Synchronization is supported (= 1) or not - Bit
    HAS_FEATURES_PRESET_SYNC_SUPPORT_BIT = CO_BIT(HAS_FEATURES_PRESET_SYNC_SUPPORT_POS),
    /// Indicate if list of preset records on this server may be different (= 1)
    /// from the list of preset records in the other server of the coordinated set or not - Position
    HAS_FEATURES_INDEPENDENT_PRESETS_POS = 3,
    /// Indicate if list of preset records on this server may be different (= 1)
    /// from the list of preset records in the other server of the coordinated set or not - Bit
    HAS_FEATURES_INDEPENDENT_PRESETS_BIT = CO_BIT(HAS_FEATURES_INDEPENDENT_PRESETS_POS),
    /// Indicate if the list of preset records may change (= 1) or not - Position
    HAS_FEATURES_DYNAMIC_PRESETS_POS = 4,
    /// Indicate if the list of preset records may change (= 1) or not - Bit
    HAS_FEATURES_DYNAMIC_PRESETS_BIT = CO_BIT(HAS_FEATURES_DYNAMIC_PRESETS_POS),
    /// Indicate if the writeable preset records are supported (= 1) or not - Position
    HAS_FEATURES_WRITABLE_PRESETS_POS = 5,
    /// Indicate if the writeable preset records are supported (= 1) or not - Bit
    HAS_FEATURES_WRITABLE_PRESETS_BIT = CO_BIT(HAS_FEATURES_WRITABLE_PRESETS_POS),

    /// Mask providing RFU bits
    HAS_FEATURES_RFU_MASK = 0xC0,
};

/// Change ID values for Preset Changed operation
enum has_change_id
{
    /// Generic Update
    HAS_CHANGE_ID_GENERIC_UPDATE = 0,
    /// Preset Record Deleted
    HAS_CHANGE_ID_PRESET_RECORD_DELETED,
    /// Preset Record Available
    HAS_CHANGE_ID_PRESET_RECORD_AVAILABLE,
    /// Preset Record Unavailable
    HAS_CHANGE_ID_PRESET_RECORD_UNAVAILABLE,

    /// Maximum value
    HAS_CHANGE_ID_MAX,
};

/// Preset Record formatting
enum has_preset_fmt
{
    /// Index field
    HAS_PRESET_INDEX_POS = 0,
    /// Properties field
    HAS_PRESET_PROPERTIES_POS,
    /// Name
    HAS_PRESET_NAME_POS,

    /// Minimal length of a Preset Record
    HAS_PRESET_LEN_MIN = HAS_PRESET_NAME_POS,
};

/// Preset Control Point characteristic Read Preset Response value formatting
enum has_preset_cp_rsp_fmt
{
    /// Opcode fields
    HAS_PRESET_CP_RSP_OPCODE_POS = 0,
    /// Is Last
    HAS_PRESET_CP_RSP_LAST_POS,
    /// Preset record
    HAS_PRESET_CP_RSP_PRESET_POS,

    /// Minimum length of Read Preset Response value
    HAS_PRESET_CP_RSP_LEN_MIN = HAS_PRESET_CP_RSP_PRESET_POS,
};

/// Preset Control Point characteristic Preset Changed operation value formatting
enum has_preset_cp_changed_fmt
{
    /// Opcode fields
    HAS_PRESET_CP_CHANGED_OPCODE_POS = 0,
    /// Change ID
    HAS_PRESET_CP_CHANGED_ID_POS,
    /// Is Last
    HAS_PRESET_CP_CHANGED_LAST_POS,

    /// Previous Preset Index (for Generic Update ChangeID)
    HAS_PRESET_CP_CHANGED_PREV_INDEX_POS,
    /// Preset Index (for Deleted, Available and Unavailable ChangeID)
    HAS_PRESET_CP_CHANGED_INDEX_POS = HAS_PRESET_CP_CHANGED_PREV_INDEX_POS,

    /// Preset record
    HAS_PRESET_CP_CHANGED_PRESET_POS,

    /// Minimal length of a Preset Changed information
    HAS_PRESET_CP_CHANGED_LEN_MIN = HAS_PRESET_CP_CHANGED_PRESET_POS,
};

/// Preset Control Point characteristic value formatting
enum has_preset_cp_fmt
{
    /// Opcode fields
    HAS_PRESET_CP_OPCODE_POS = 0,
    /// Preset Index
    HAS_PRESET_CP_INDEX_POS,
    /// Name
    HAS_PRESET_CP_NAME_POS,
    /// Number of Presets
    HAS_PRESET_CP_NUM_PRESETS_POS = HAS_PRESET_CP_NAME_POS,

    /// Minimal length of Preset Control Point characteristic value
    HAS_PRESET_CP_LEN_MIN = HAS_PRESET_CP_INDEX_POS,
    /// Maximal length of Preset Control Point characteristic value
    HAS_PRESET_CP_LEN_MAX = HAS_PRESET_CP_NAME_POS + HAS_PRESET_NAME_LEN_MAX,
    /// Minimal length of Preset Control Point characteristic value when index is provided
    HAS_PRESET_CP_LEN_MIN_INDEX = HAS_PRESET_CP_NAME_POS,
};

#endif /// __HAP_COMMON__