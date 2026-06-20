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
 * @addtogroup APP_ACC
 * @{
 ****************************************************************************************
 */

#ifndef APP_ACC_H_
#define APP_ACC_H_
#if BLE_AUDIO_ENABLED

#include "bluetooth.h"

#ifdef __cplusplus
extern "C" {
#endif

int app_acc_start(uint8_t con_lid, bool is_mobile);

#ifdef __cplusplus
}
#endif

#endif
#endif // APP_ACC_H_

/// @} APP_ACC
