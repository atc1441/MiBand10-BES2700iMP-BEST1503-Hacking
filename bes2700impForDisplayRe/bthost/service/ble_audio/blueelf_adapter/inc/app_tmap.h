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
 * @addtogroup APP_TMAP
 * @{
 ****************************************************************************************
 */

#ifndef APP_TMAP_H_
#define APP_TMAP_H_

#if BLE_AUDIO_ENABLED
#include "prf_types.h"

/*
 * DEFINES
 ****************************************************************************************
 */

typedef struct tmap_info
{
    app_gaf_prf_svc_t tmap_svc_hdl;
} tmap_info_t;

typedef struct app_tmap_info
{
    tmap_info_t tmap_info[BLE_AUDIO_CONNECTION_CNT];
} app_tmap_info_t;

#ifdef __cplusplus
extern "C" {
#endif

/* ble audio gaf tmap (Telephony and Media AUdio profile) start */
void app_tmap_start(uint8_t con_lid);

#ifdef __cplusplus
}
#endif

#endif
#endif // APP_TMAP_H_
/// @} APP_TMAP