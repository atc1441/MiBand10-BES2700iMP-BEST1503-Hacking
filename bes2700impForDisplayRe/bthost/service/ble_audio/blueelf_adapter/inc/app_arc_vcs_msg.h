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
 * @addtogroup APP_ARC

 * @{
 ****************************************************************************************
 */

#ifndef APP_ARC_VCS_MSG_H_
#define APP_ARC_VCS_MSG_H_
#if BLE_AUDIO_ENABLED
#include "bluetooth.h"

#define APP_ARC_VC_UPDATE_VOL_BIT   (0x1)
#define APP_ARC_VC_UPDATE_MUTE_BIT  (0x2)
#define APP_ARC_VC_UPDATE_VOL_MUTE_MASK  (APP_ARC_VC_UPDATE_VOL_BIT | APP_ARC_VC_UPDATE_MUTE_BIT)

#ifdef __cplusplus
extern "C" {
#endif

int app_arc_vcs_restore_bond_data_req(uint8_t con_lid, uint8_t cli_cfg_bf);
int app_arc_vcs_control_by_con_lid(uint8_t con_lid, uint8_t opcode, uint8_t volume, bool no_changed_cb);
int app_arc_vcs_update_info_req_by_con_lid(uint8_t con_lid, uint8_t bit_map, uint8_t volume, bool isMute);

#ifdef __cplusplus
}
#endif

#endif
#endif // APP_ARC_VCS_MSG_H_

/// @} APP_ARC_VCS
