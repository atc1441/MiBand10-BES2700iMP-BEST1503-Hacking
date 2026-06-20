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
#ifndef __GAF_TMAS_COMMON__
#define __GAF_TMAS_COMMON__

#include "gaf_cfg.h"

/// Characteristic type values for Telephony and Media Audio Service
enum tmap_char_type
{
    /// TMAP Role characteristic
    TMAP_CHAR_TYPE_ROLE = 0,
    TMAP_CHAR_TYPE_MAX,
};

/// TMAP Role characteristic bit field meaning
enum tmap_role_bf
{
    /// Indicate if Server supports Call Gateway role (= 1) or not - Bit
    TMAP_ROLE_CG_BIT = CO_BIT(0),
    /// Indicate if Server supports Call Terminal role (= 1) or not - Bit
    TMAP_ROLE_CT_BIT = CO_BIT(1),
    /// Indicate if Server supports Unicast Media Sender role (= 1) or not - Bit
    TMAP_ROLE_UMS_BIT = CO_BIT(2),
    /// Indicate if Server supports Unicast Media Receiver role (= 1) or not - Bit
    TMAP_ROLE_UMR_BIT = CO_BIT(3),
    /// Indicate if Server supports Broadcast Media Sender role (= 1) or not - Bit
    TMAP_ROLE_BMS_BIT = CO_BIT(4),
    /// Indicate if Server supports Broadcast Media Receiver role (= 1) or not - Bit
    TMAP_ROLE_BMR_BIT = CO_BIT(5),
    /// Mask indicating that all roles are supported
    TMAP_ROLE_ALLSUPP_MASK = 0x003F,
    /// Mask indicating RFU bits
    TMAP_ROLE_RFU_MASK = 0xFFC0,
};

#endif /// __GAF_TMAS_COMMON__
