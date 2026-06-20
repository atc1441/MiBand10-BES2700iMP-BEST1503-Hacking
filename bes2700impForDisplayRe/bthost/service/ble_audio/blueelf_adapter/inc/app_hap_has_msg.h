/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifndef _APP_HAP_HAS_MSG_H_
#define _APP_HAP_HAS_MSG_H_

#if BLE_AUDIO_ENABLED
#include "bluetooth.h"

#define APP_HAP_HAS_INIT_2_MORE_PRESET  (false)
#define APP_HAP_HAS_PRESET_DFT_NAME     "HA_PRESET_DFT"
#ifdef __cplusplus
extern "C" {
#endif
int app_hap_has_msg_set_feature_req(uint8_t features_bf);
int app_hap_has_msg_update_preset_req(uint8_t preset_lid, bool available, uint8_t length, char *name);
#ifdef __cplusplus
}
#endif
#endif
#endif