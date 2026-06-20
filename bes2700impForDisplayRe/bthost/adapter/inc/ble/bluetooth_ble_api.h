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
#ifndef __BLUETOOTH_BLE_API_H__
#define __BLUETOOTH_BLE_API_H__
#ifdef BLE_HOST_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif

#include "ble_device_info.h"
#include "ble_core_common.h"
#include "ble_common_define.h"
#include "ble_aob_common.h"
#include "bes_gap_api.h"
#include "bes_gatt_api.h"
#include "bes_aob_api.h"
#ifdef GFPS_ENABLED
#include "bes_gfps_api.h"
#endif
#include "bes_dp_api.h"
#ifdef __AI_VOICE_BLE_ENABLE__
#include "bes_ai_api.h"
#endif
#ifdef TILE_DATAPATH
#include "bes_tile_api.h"
#endif
#ifdef BLE_WALKIE_TALKIE
#include "bes_walkie_talkie_ble_gap_api.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /* BLE_HOST_SUPPORT */
#endif /* __BLUETOOTH_BLE_API_H__ */
