/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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

#ifndef __BLE_AUDIO_MOBILE_INFO_H__
#define __BLE_AUDIO_MOBILE_INFO_H__

#include "nvrecord_extension.h"
#include "ble_audio_define.h"
#include "bes_aob_api.h"

#ifdef AOB_MOBILE_ENABLED

typedef struct
{
    bool                    connected;
    uint8_t                 conidx;
    BLE_ADDR_INFO_T         peer_ble_addr;
} AOB_MOBILE_PHONE_INFO_T;

#ifdef __cplusplus
extern "C" {
#endif

void ble_audio_mobile_info_init(void);
void ble_audio_mobile_info_connected_set(uint8_t conidx, uint8_t* earbud_ble_addr, uint8_t earbud_addr_type);
void ble_audio_mobile_info_clear(uint8_t conidx);
void ble_audio_single_call_info_clear(uint8_t conidx, uint8_t call_id);
AOB_MOBILE_PHONE_INFO_T *ble_audio_mobile_info_get(uint8_t conidx);
uint8_t ble_audio_get_active_conidx(void);
AOB_SINGLE_CALL_INFO_T *ble_audio_mobile_info_get_call_info_by_id(uint8_t conidx, uint8_t call_id);
void ble_audio_mobile_info_clear_call_info(uint8_t conidx, uint8_t call_id);

AOB_SINGLE_CALL_INFO_T *ble_audio_mobile_get_call_info_by_state(uint8_t conidx, AOB_CALL_STATE_E state);
AOB_SINGLE_CALL_INFO_T *ble_audio_mobile_get_call_info_by_bearer_lid(uint8_t conidx, uint8_t bearer_lid);
void ble_audio_mobile_info_bond_bearer_id(uint8_t conidx, uint8_t bearer_lid);
void ble_audio_mobile_info_update_call_id(uint8_t conidx, uint8_t call_id);
bool ble_audio_mobile_both_is_connected(void);

/**
 * @brief Check if there is any device connected
 *
 * @return  True: Connected
 *          False: No device connected
*/
bool ble_audio_mobile_is_any_device_connected(void);

/**
 * @brief Check whether the device corresponding to the address is connected
 *
*/
bool ble_audio_mobile_check_device_connected(uint8_t *p_addr);

#ifdef __cplusplus
}
#endif

#endif      //AOB_MOBILE_ENABLED
#endif      //__BLE_AUDIO_MOBILE_INFO_H__



