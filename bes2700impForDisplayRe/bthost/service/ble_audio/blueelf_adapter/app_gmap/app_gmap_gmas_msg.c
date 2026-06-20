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
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#if BLE_AUDIO_ENABLED
#include "app_gaf.h"
#include "app_gmap.h"
#include "ble_app_dbg.h"
#include "app_gaf_custom_api.h"

#include "gmas.h"

int app_gmap_gmas_init(uint8_t role_bf, gmap_role_feat_bf_t role_feat)
{
    LOG_I("app_gmap gmas init, role_bf = 0x%x", role_bf);

    return gmas_init(role_bf, role_feat, GAF_PREFERRED_MTU);
}

int app_gmap_gmas_deinit(void)
{
    LOG_I("app_gmap gmas deinit");

    return gmas_deinit();
}

#endif
/// @} APP
