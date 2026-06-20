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
#ifndef __MICS_COMMON__
#define __MICS_COMMON__

#include "bluetooth.h"

#include "gaf_cfg.h"

/*DEFINE*/
#define MICS_APP_ERR_MUTE_DISABLED      (0x80)

/*TYPEDEF*/
typedef enum arc_mics_mute_value
{
    MICS_MUTE_VALUE_MIN = 0,

    MICS_MUTE_VALUE_NOT_MUTED = MICS_MUTE_VALUE_MIN,

    MICS_MUTE_VALUE_MUTED,

    MICS_MUTE_VALUE_DISABLED,

    MICS_MUTE_VALUE_MAX,
} mics_mute_val_e;

#endif /// __MICS_COMMON__