/**
 ****************************************************************************************
 *
 * @file app_bap_bc_assist_msg.h
 *
 * @brief BLE Audio Broadcast Assistant
 *
 * Copyright 2015-2019 BES.
 *
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
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP_ACC

 * @{
 ****************************************************************************************
 */

#ifndef APP_BAP_ASSIST_MSG_H_
#define APP_BAP_ASSIST_MSG_H_
#if BLE_AUDIO_ENABLED
#include "app_bap_bc_scan_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef APP_BLE_BIS_ASSIST_ENABLE
int app_bap_bc_assist_start(uint8_t con_lid);
int app_bap_bc_assist_update_scan(uint8_t scan_state);
bool app_bap_bc_assist_is_deleg_scan(void);
int app_bap_bc_set_assist_is_deleg_scan(uint8_t con_lid, bool is_deleg_scan);
int app_bap_bc_assist_get_cfg_cmd(uint8_t con_lid, uint8_t src_lid);
int app_bap_bc_assist_set_cfg_cmd(uint8_t con_lid, uint8_t src_lid, uint8_t enable);
#endif

#ifdef __cplusplus
}
#endif

#endif
#endif // APP_BAP_ASSIST_MSG_H_

/// @} APP_BAP_ASSIST_MSG_H_
