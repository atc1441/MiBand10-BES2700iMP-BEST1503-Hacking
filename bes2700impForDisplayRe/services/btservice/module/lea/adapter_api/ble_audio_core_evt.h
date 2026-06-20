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

#ifndef __BLE_AUDIO_CORE_EVT_H__
#define __BLE_AUDIO_CORE_EVT_H__

#include "bluetooth_bt_api.h"
#include "bes_aob_api.h"

typedef enum
{
    BLE_AUDIO_EVT_TYPE_FRIST,
    BLE_AUDIO_TWS_LINNK_EVT = BLE_AUDIO_EVT_TYPE_FRIST,
    BLE_ADUIO_MOBILE_LINK_EVT,


    BLE_AUDIO_EVT_TYPE_LAST,
}BLE_AUDIO_EVT_TYPE_E;

typedef void (*CallbackFunction)(void);

#ifdef __cplusplus
extern "C" {
#endif

void ble_audio_core_register_event_cb(const BLE_AUD_CORE_EVT_CB_T *cb);

#ifdef AOB_MOBILE_ENABLED
void ble_audio_mobile_core_register_event_cb(const BLE_AUD_MOB_CORE_EVT_CB_T *cb);
#endif

const char* ble_audio_event_to_string(int event);

#ifdef __cplusplus
}
#endif

#endif
