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
 * @addtogroup APP_BAP
 * @{
 ****************************************************************************************
 */

#ifndef APP_VCC_H_
#define APP_VCC_H_
#if BLE_AUDIO_ENABLED
#include "bluetooth.h"

#define APP_ARC_VCS_DFT_STEP_SIZE           15
#define APP_ARC_VCS_DFT_VOLUME              225
#define APP_ARC_VOCS_DESC_MAX_LEN           50
#define APP_ARC_VOCS_DFT_NB_OUTPUTS         1
#define APP_ARC_AICS_DFT_NB_INPUTS          2
#define APP_ARC_AICS_DESC_MAX_LEN           50
#define APP_ARC_AICS_DFT_GAIN_UNITS         10
#define APP_ARC_AICS_DFT_GAIN_MAX           127
#define APP_ARC_AICS_DFT_GAIN_MIN           (-128)
#define APP_ARC_DESC_STR                    "mdb"

#ifdef __cplusplus
extern "C" {
#endif

int app_arc_start(uint8_t con_lid);

#ifdef __cplusplus
}
#endif

#endif
#endif // APP_VCC_H_

/// @} APP_VCC
