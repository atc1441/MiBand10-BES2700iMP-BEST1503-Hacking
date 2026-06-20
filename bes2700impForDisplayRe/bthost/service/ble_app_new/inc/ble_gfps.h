/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifndef __BLE_GFPS_H__
#define __BLE_GFPS_H__
#ifdef GFPS_ENABLED
#include "gatt_service.h"
#include "ble_gfps_common.h"
#ifdef __cplusplus
extern "C" {
#endif
enum gfpsp_att_data_event
{
    GFPSP_GET_SPOT_VERSION,
    GFPSP_GEN_SPOT_NOUNCE,
    GFPSP_GET_SPOT_NOUNCE,
    GFPSP_GET_MODEL_ID,
    GFPSP_GET_ADDITIONAL_PASSKEY,
    GFPSP_GET_EVENT_STREAM,
};

void ble_app_gfps_send_keybase_pairing(uint8_t conidx, const uint8_t *data, uint32_t length);

void ble_app_gfps_send_passkey(uint8_t conidx, const uint8_t *data, uint32_t length);

void ble_app_gfps_send_additional_passkey(uint8_t conidx, const uint8_t *data, uint32_t length);

void ble_app_gfps_send_naming_packet(uint8_t conidx, const uint8_t *data, uint32_t length);

#ifdef SPOT_ENABLED
void ble_app_gfps_send_beacon_data(uint8_t conidx, const uint8_t *data, uint32_t length);
#endif

void ble_app_gfps_l2cap_disconnect(uint8_t conidx);
uint8_t ble_app_gfps_is_connected(uint8_t conidx);


#ifdef __cplusplus
    }
#endif
#endif /* GFPS_ENABLED */
#endif /* __BLE_GFPS_H__ */
