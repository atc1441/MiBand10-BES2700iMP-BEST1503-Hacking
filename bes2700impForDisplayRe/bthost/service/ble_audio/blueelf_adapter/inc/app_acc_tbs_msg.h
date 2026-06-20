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

#ifndef APP_ACC_TBS_MSG_H_
#define APP_ACC_TBS_MSG_H_

#include "ble_acc_common.h"

#if BLE_AUDIO_ENABLED

#define APP_ACC_DFT_TBS_CHAR_VAL_LEN        (100)
//There is a GTBS by default, so it just means the number of TBS
#define APP_ACC_DFT_TBS_NUM                 (1)
//The number of calls that maintained at the same time
#define APP_ACC_CALL_POOL_SIZE              (2)
//Such as: the length of phone number, tel:+13888888888
#define APP_ACC_MAX_URI_LEN                 (32)

#define APP_GAF_ACC_GTBS_CCID               (0)
#define APP_GAF_ACC_TBS_CCID                (1)
#define APP_GAF_ACC_UCI_VAL1                "eyebm"
#define APP_GAF_ACC_UCI_VAL2                "qq"

#ifdef AOB_MOBILE_ENABLED
#endif
#endif

#endif // APP_ACC_TBS_MSG_H_

/// @} APP_ACC_TBS_MSG_H_
