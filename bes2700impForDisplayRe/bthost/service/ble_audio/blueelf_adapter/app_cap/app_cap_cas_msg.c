/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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

#include "cas.h"
#include "ble_app_dbg.h"

int app_cap_cas_init(void)
{
    LOG_I("app_cap cas init");
    return cas_init();
}

int app_cap_cas_deinit(void)
{
    LOG_I("app_cap cas deinit");
    return cas_deinit();
}

#endif
/// @} APP
