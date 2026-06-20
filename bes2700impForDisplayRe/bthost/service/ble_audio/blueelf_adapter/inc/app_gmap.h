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

#ifndef APP_GMAP_H_
#define APP_GMAP_H_

#if BLE_AUDIO_ENABLED
#include "bluetooth.h"

/*
 * DEFINES
 ****************************************************************************************
 */

#ifdef __cplusplus
extern "C" {
#endif

/* ble audio gaf gmap (Gaming Audio profile) start */
int app_gmap_start(uint8_t con_lid);

#ifdef __cplusplus
}
#endif

#endif
#endif // APP_GMAP_H_
/// @} APP_GMAP