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
#ifndef __GAF_GMAS_INC__
#define __GAF_GMAS_INC__

#include "gaf_cfg.h"
#include "gaf_log.h"

#include "gma_common.h"

/// GMAP Character value Feature bitfiled struct
typedef struct gmap_role_feat_bitfield
{
    /// GMAP UGG features bitfiled
    uint8_t gmap_ugg_feat_bf;
    /// GMAP UGT features bitfiled
    uint8_t gmap_ugt_feat_bf;
    /// GMAP BGS features bitfiled
    uint8_t gmap_bgs_feat_bf;
    /// GMAP BGR features bitfiled
    uint8_t gmap_bgr_feat_bf;
} gmap_role_feat_bf_t;


/**
 * @brief Gaming Audio server initilization
 *
 * @param  role_bf     Role bitfiled @see enum gmap_role_bf_t
 * @param  role_feat   Features structure @see enum gmap_role_feat_bf_t
 * @param  pref_mtu_size
 *                     Preferred MTU size
 *
 * @return int         status
 */
int gmas_init(uint8_t role_bf, gmap_role_feat_bf_t role_feat, uint16_t pref_mtu_size);

/**
 * @brief Gaming Audio server deinitilization
 *
 * @return int         status
 */
int gmas_deinit(void);

#endif /// __GAF_GMAS_INC__
