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
#ifndef __ARC_VC_COMMON__
#define __ARC_VC_COMMON__

/// Minimum volume value
#define VCS_VOLUME_MIN              (0)
/// Maximum volume value
#define VCS_VOLUME_MAX              (255)

#define VCS_VOL_STATE_VALUE_LEN     (3)

#define VCS_RESET_VOL_SETTING       (0 << 0)
#define VCS_USER_SET_VOL_SETTING    CO_BIT(0)

/// List of Volume Control Service characteristics
enum vcs_char_type
{
    /// Volume State characteristic
    VCS_CHAR_TYPE_VOL_STATE = 0,
    /// Volume Flags characteristic
    VCS_CHAR_TYPE_VOL_FLAGS,

    VCS_CHAR_TYPE_NTF_MAX,

    /// Volume Control Point characteristic
    VCS_CHAR_TYPE_VOL_CP = VCS_CHAR_TYPE_NTF_MAX,

    VCS_CHAR_TYPE_MAX,
};

/// Volume Operation Code values
enum vcs_opcode
{
    /// Relative Volume Down
    VCS_OPCODE_REL_VOL_DOWN = 0,
    /// Relative Volume Up
    VCS_OPCODE_REL_VOL_UP,
    /// Unmute/Relative Volume Down
    VCS_OPCODE_REL_VOL_DOWN_UNMUTE,
    /// Unmute/Relative Volume Up
    VCS_OPCODE_REL_VOL_UP_UNMUTE,
    /// Set Absolute Volume
    VCS_OPCODE_VOL_SET_ABS,
    /// Unmute
    VCS_OPCODE_VOL_UNMUTE,
    /// Mute
    VCS_OPCODE_VOL_MUTE,

    VCS_OPCODE_MAX
};

/// Application error codes
enum vcs_err
{
    /// Invalid change counter
    VCS_ERR_INVALID_CHANGE_CNT = 0x80,
    /// Opcode not supported
    VCS_ERR_OPCODE_NOT_SUPP,
};

#endif /// __ARC_VC_COMMON__