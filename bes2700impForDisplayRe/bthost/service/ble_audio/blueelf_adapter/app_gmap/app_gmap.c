/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
/**
 ****************************************************************************************
 * @addtogroup APP_GMAP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#if BLE_AUDIO_ENABLED
#include "ble_app_dbg.h"
#include "app_gmap.h"
#include "app_gmap_gmac_msg.h"
#include "app_gmap_gmas_msg.h"

#include "gmas.h"

/*EXTERNAL FUNCTIONS*/
/*GMAS*/
extern int app_gmap_gmas_init(uint8_t role_bf, gmap_role_feat_bf_t role_feat);
extern int app_gmap_gmas_deinit(void);
/*GMAC*/
extern int app_gmap_gmac_init(void);
extern int app_gmap_gmac_deinit(void);
extern int app_gmap_gmac_start(uint8_t con_lid);

const gmap_role_feat_bf_t role_feat_no_ugg =
{
    .gmap_ugg_feat_bf = 0,
    .gmap_ugt_feat_bf = GMAP_UGT_FEAT_ALLSUPP_MASK
#if (BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT == 0)
    & ~(GMAP_UGT_FEAT_MULTISINK_BIT | GMAP_UGT_FEAT_MULTISOURCE_BIT)
#endif
    ,
    .gmap_bgs_feat_bf =
#if (APP_BLE_BIS_SRC_ENABLE)
    GMAP_UGT_FEAT_ALLSUPP_MASK
#else
    0
#endif
    ,
    .gmap_bgr_feat_bf = GMAP_BGR_FEAT_ALLSUPP_MASK,
};

const gmap_role_feat_bf_t role_feat_no_ugt =
{
    .gmap_ugg_feat_bf = GMAP_UGG_FEAT_ALLSUPP_MASK,
    .gmap_ugt_feat_bf = 0,
    .gmap_bgs_feat_bf =
#if (APP_BLE_BIS_SRC_ENABLE)
    GMAP_UGT_FEAT_ALLSUPP_MASK
#else
    0
#endif
    ,
    .gmap_bgr_feat_bf = GMAP_BGR_FEAT_ALLSUPP_MASK,
};

/*FUNCTIONS*/
int app_gmap_client_init(void)
{
    return app_gmap_gmac_init();
}

int app_gmap_client_deinit(void)
{
    return app_gmap_gmac_deinit();
}

int app_gmap_server_init(uint8_t role_bf)
{
    if (role_bf & GMAP_ROLE_UGT_BIT)
    {
        return app_gmap_gmas_init(role_bf, role_feat_no_ugg);
    }
    else
    {
        return app_gmap_gmas_init(role_bf, role_feat_no_ugt);
    }
}

int app_gmap_server_deinit(uint8_t role_bf)
{
    return app_gmap_gmas_deinit();
}

int app_gmap_start(uint8_t con_lid)
{
    return app_gmap_gmac_start(con_lid);
}

#endif
/// @} APP_GMAP


