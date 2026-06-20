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
#ifndef __AICS_COMMON__
#define __AICS_COMMON__

#include "bluetooth.h"

#include "gaf_cfg.h"

/*ENUMERATIONS*/

/// Set type values
enum aics_set_type
{
    /// Set Input Status
    AICS_SET_TYPE_INPUT_STATUS = 0,
    /// Set Gain Setting
    AICS_SET_TYPE_GAIN,
    /// Set Mute
    AICS_SET_TYPE_MUTE,
    /// Set Gain Mode
    AICS_SET_TYPE_GAIN_MODE,
    /// Set input description
    AICS_SET_TYPE_INPUT_DESC,

    AICS_SET_TYPE_MAX,
};

/// Operation Code values for Audio Input Control Point characteristic
enum aics_opcode
{
    /// Set Gain Setting
    AICS_OPCODE_SET_GAIN = 1,
    /// Unmute
    AICS_OPCODE_UNMUTE,
    /// Mute
    AICS_OPCODE_MUTE,
    /// Set Manual Gain Mode
    AICS_OPCODE_SET_MANUAL_MODE,
    /// Set Automatic Gain Mode
    AICS_OPCODE_SET_AUTO_MODE,
};

/// Application error codes
enum aics_app_err
{
    /// Invalid Change Counter
    AICS_ERR_INVALID_CHANGE_CNT = 0x80,
    /// Opcode Not Supported
    AICS_ERR_OPCODE_NOT_SUPP,
    /// Mute Disabled
    AICS_ERR_MUTE_DISABLED,
    /// Value Out of Range
    AICS_ERR_OUT_OF_RANGE,
    /// Gain Mode Change Not Allowed
    AICS_ERR_GAIN_MODE_CHANGE_NOT_ALLOWED,
};

/// List of Audio Input Control Service characteristics
enum aics_char_type
{
    /// Audio Input State characteristic
    AICS_CHAR_TYPE_STATE = 0,
    /// Audio Input Status characteristic
    AICS_CHAR_TYPE_STATUS,
    /// Audio Input Description characteristic
    AICS_CHAR_TYPE_DESC,

    AICS_CHAR_TYPE_NTF_MAX,

    /// Gain Setting Properties characteristic
    AICS_CHAR_TYPE_GAIN_PROP = AICS_CHAR_TYPE_NTF_MAX,
    /// Audio Input Type characteristic
    AICS_CHAR_TYPE_TYPE,
    /// Audio Input Control Point characteristic
    AICS_CHAR_TYPE_CP,

    AICS_CHAR_TYPE_MAX,
};

/// Mute values
enum aics_mute
{
    /// Not muted
    AICS_MUTE_NOT_MUTED = 0,
    /// Muted
    AICS_MUTE_MUTED,
    /// Disabled
    AICS_MUTE_DISABLED,

    AICS_MUTE_MAX,
};

/// Gain Mode values
enum aics_gain_mode
{
    /// Manual only
    AICS_GAIN_MODE_MANUAL_ONLY = 0,
    /// Automatic only
    AICS_GAIN_MODE_AUTO_ONLY,
    /// Manual
    AICS_GAIN_MODE_MANUAL,
    /// Automatic
    AICS_GAIN_MODE_AUTO,

    AICS_GAIN_MODE_MAX
};

/// Input Status values
enum aics_input_status
{
    /// Inactive
    AICS_INPUT_STATUS_INACTIVE = 0,
    /// Active
    AICS_INPUT_STATUS_ACTIVE,

    AICS_INPUT_STATUS_MAX,
};

/// Input Type values
enum aics_input_type
{
    /// Unspecified Input
    AICS_INPUT_TYPE_UNSPEC = 0,
    /// Bluetooth Audio Stream
    AICS_INPUT_TYPE_BT,
    /// Microphone
    AICS_INPUT_TYPE_MICROPHONE,
    /// Analog Interface
    AICS_INPUT_TYPE_ANALOG,
    /// Digital
    AICS_INPUT_TYPE_DIGITAL,
    /// Radio (AM/FM/XM/...)
    AICS_INPUT_TYPE_RADIO,
    /// Streaming Audio Source
    AICS_INPUT_TYPE_STREAMING,

    AICS_INPUT_TYPE_MAX,
};

/*TYPEDEF*/

/// Control Operation structure
typedef struct aics_control_operation_packed
{
    /// OPCODE
    uint8_t opcode;
    /// Counter
    uint8_t counter;
} __attribute__((packed)) aics_ctrl_op_p_t;

/// Gain Control structure
typedef struct aics_gain_control_operation_packed
{
    /// Operation
    aics_ctrl_op_p_t op;
    /// Gain setting
    int8_t gain_setting;
} __attribute__((packed)) aics_gain_ctrl_op_p_t;

/// Audio Input State structure
typedef struct aics_state_packed
{
    /// Gain
    int8_t gain;
    /// Mute
    uint8_t mute;
    /// Gain Mode
    uint8_t gain_mode;
    /// Change counter
    uint8_t change_counter;
} __attribute__((packed)) aics_state_p_t;

/// Gain Setting Properties structure
typedef struct aics_gain_settings_properties_packed
{
    /// Gain Units
    uint8_t units;
    /// Gain Minimum
    int8_t minimum;
    /// Gain Maximum
    int8_t maximum;
} __attribute__((packed)) aics_gain_setting_prop_p_t;

/// Audio Input State structure
typedef struct aics_state
{
    /// Gain
    int8_t gain;
    /// Mute
    uint8_t mute;
    /// Gain Mode
    uint8_t gain_mode;
    /// Change counter
    uint8_t change_counter;
} aics_state_t;

/// Gain Setting Properties structure
typedef struct aics_gain_settings_properties
{
    /// Gain Units
    uint8_t units;
    /// Gain Minimum
    int8_t minimum;
    /// Gain Maximum
    int8_t maximum;
} aics_gain_setting_prop_t;

#endif /// __AICS_COMMON__