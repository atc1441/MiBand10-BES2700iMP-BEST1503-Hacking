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
#ifndef __VOCS_COMMON__
#define __VOCS_COMMON__

#include "bluetooth.h"

#include "gaf_cfg.h"

/*TYPEDEF*/

/// Minimum offset value
#define VOCS_OFFSET_MIN           (-255)
/// Maximum offset value
#define VOCS_OFFSET_MAX           (255)

/*ENUMERATIONS*/

/// Volume Offset Operation Code values
enum vocs_opcode
{
    VOCS_OPCODE_MIN = 0,
    /// Set Volume Offset
    VOCS_OPCODE_SET_OFFSET = 1,

    VOCS_OPCODE_MAX,
};

/// Application error codes
enum vocs_app_err
{
    /// Invalid change counter
    VOCS_ERR_INVALID_CHANGE_CNT = 0x80,
    /// Opcode not supported
    VOCS_ERR_OPCODE_NOT_SUPP,
    /// Value out of range
    VOCS_ERR_VALUE_OUT_OF_RANGE,
};

/// List of Volume Offset Control Service characteristics
enum vocs_char_type
{
    /// Volume Offset State characteristic
    VOCS_CHAR_TYPE_OFFSET = 0,
    /// Audio Location characteristic
    VOCS_CHAR_TYPE_LOC,
    /// Audio Output Description characteristic
    VOCS_CHAR_TYPE_DESC,

    VOCS_CHAR_TYPE_NTF_MAX,

    /// Volume Offset Control Point characteristic
    VOCS_CHAR_TYPE_CP = VOCS_CHAR_TYPE_NTF_MAX,

    VOCS_CHAR_TYPE_MAX,
};

/*TYPEDEF*/
typedef struct vocs_control_operation_packed
{
    /// Control opcode
    uint8_t opcode;
    /// Change counter
    uint8_t counter;
    /// Volume offset
    int16_t offset;
} __attribute((packed)) vocs_ctrl_op_p_t;

typedef struct vocs_state_packed
{
    /// Volume offset
    int16_t offset;
    /// Change counter
    uint8_t change_counter;
} __attribute((packed)) vocs_state_p_t;
#endif /// __VOCS_COMMON__