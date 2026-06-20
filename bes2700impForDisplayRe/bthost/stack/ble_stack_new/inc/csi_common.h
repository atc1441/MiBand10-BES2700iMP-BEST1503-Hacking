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
#ifndef __CSI_COMMON__
#define __CSI_COMMON__
#include "bluetooth.h"

#include "gatt_service.h"
#include "gaf_cfg.h"

#define CSIS_INVALID_SET_LID                (GAF_INVALID_ANY_LID)
#define CSIS_SIRK_VAL_LEN                   (16)
#define CSIS_SIRK_TOTAL_LEN                 (CSIS_SIRK_VAL_LEN + 1)
#define CSIS_SIRK_ENC_KEY_LEN               (16)
#define CSIS_RSI_VAL_LEN                    (6)

#define CSIS_LOCK_LOCK                      (0x02)
#define CSIS_LOCK_UNLOCK                    (0x01)

#define CSIS_SET_MEMBER_INST_NUMER_MAX      (8)

/// Characteristic type values for Coordinated Set Identification Service
enum csis_character_type
{
    /// Set Identity Resolving Key characteristic
    CSIS_CHARACTER_TYPE_SIRK = 0,
    /// Coordinated Set Size characteristic
    CSIS_CHARACTER_TYPE_SIZE,
    /// Set Member Lock characteristic
    CSIS_CHARACTER_TYPE_LOCK,

    CSIS_CHARACTER_TYPE_NTF_MAX,

    /// Set Member Rank characteristic
    CSIS_CHARACTER_TYPE_RANK = CSIS_CHARACTER_TYPE_NTF_MAX,

    CSIS_CHARACTER_TYPE_MAX,
};

/// Descriptor type values for Coordinated Set Identification Service
enum csis_descriptor_type
{
    /// Client Characteristic Configuration descriptor for Set Identity Resolving Key characteristic
    CSIS_DESCRIPTOR_TYPE_CCC_SIRK = 0,
    /// Client Characteristic Configuration descriptor for Coordinated Set Size characteristic
    CSIS_DESCRIPTOR_TYPE_CCC_SIZE,
    /// Client Characteristic Configuration descriptor for Set Member Lock characteristic
    CSIS_DESCRIPTOR_TYPE_CCC_LOCK,

    CSIS_DESCRIPTOR_TYPE_MAX,
};

/// Specific error codes for Coordinated Set Identification service
enum csis_app_error
{
    /// Lock is denied
    CSIS_APP_ERR_LOCK_DENIED = 0x80,
    /// Lock release is not allowed
    CSIS_APP_ERR_LOCK_RELEASE_DISALLOWED,
    /// Invalid Lock value
    CSIS_APP_ERR_INVALID_LOCK_VALUE,
    /// Support OOB SIRK only
    CSIS_APP_ERR_OOB_SIRK_ONLY,
    /// Lock is already granted
    CSIS_APP_ERR_LOCK_ALREADY_GRANTED,

    CSIS_APP_ERR_UNEXPECTED,
};

/// SIRK type values
enum csis_sirk_value_type
{
    /// Encrypted SIRK
    CSIS_SIRK_VAL_TYPE_ENCRYPTED = 0,
    /// Plain text SIRK
    CSIS_SIRK_VAL_TYPE_PLAIN = 1,
};
#endif