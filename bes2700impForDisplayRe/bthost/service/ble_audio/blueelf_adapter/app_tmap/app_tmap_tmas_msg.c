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
#include "app_tmap.h"
#include "ble_app_dbg.h"
#include "app_gaf_custom_api.h"

#include "tmas.h"

int app_tmap_tmas_init(uint16_t role_bf)
{
    LOG_I("app_tmap tmas init, role_bf = 0x%x", role_bf);

    return tmas_init(role_bf, GAF_PREFERRED_MTU);
}

int app_tmap_tmas_deinit(void)
{
    LOG_I("app_tmap tmas init");

    return tmas_deinit();
}

#endif
/// @} APP
