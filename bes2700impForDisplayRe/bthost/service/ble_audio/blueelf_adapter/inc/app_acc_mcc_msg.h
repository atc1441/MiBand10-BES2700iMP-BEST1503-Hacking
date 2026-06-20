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

#ifndef APP_ACC_MCC_MSG_H_
#define APP_ACC_MCC_MSG_H_
#if BLE_AUDIO_ENABLED
#include "bluetooth.h"

#define APP_ACC_MCC_MAX_SUPP_MCS_INST   (1)
/// Stack perform discovery with only character find, do not do read val and set cfg
#define APP_ACC_MCC_DISCOVERY_ONLY_FIND_CHAR (1)

#ifdef __cplusplus
extern "C" {
#endif

/// APP_GAF_MCC_SVC_DISCOVERYED_IND
int app_acc_mcc_start(uint8_t con_lid);

int app_acc_mcc_simplified_start(uint8_t con_lid);

int app_acc_mcc_restore_bond_data_req(uint8_t con_lid, uint8_t nb_media, void const *data);

#ifdef __cplusplus
}
#endif

#endif
#endif // APP_ACC_MCC_MSG_H_

/// @} APP_ACC_MCC_MSG_H_
