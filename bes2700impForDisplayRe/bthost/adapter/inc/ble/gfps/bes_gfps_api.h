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
#ifndef __BES_GFPS_API_H__
#define __BES_GFPS_API_H__

#ifdef BLE_HOST_SUPPORT
#ifdef GFPS_ENABLED
#ifdef __cplusplus
extern "C" {
#endif

#include "ble_gfps_common.h"
#include "app_ble.h"

#ifdef SPOT_ENABLED
#include "ble_dult.h"
#endif

void bes_ble_gfps_init(app_ble_adv_activity_func func);

void bes_ble_gfps_send_keybase_pairing(uint8_t conidx, uint8_t *data, uint16_t length);

uint8_t bes_ble_gfps_l2cap_send(uint8_t conidx, uint8_t *ptrData, uint32_t length);

void bes_ble_gfps_l2cap_disconnect(uint8_t conidx);

#ifdef SPOT_ENABLED
void bes_ble_gfps_send_beacon_data(uint8_t conidx, uint8_t *data, uint16_t length);
void bes_ble_spot_init(app_ble_adv_activity_func func);
void bes_ble_dult_init(struct ble_dult_cb_t *cb);
#endif

#ifdef __cplusplus
}
#endif
#endif
#endif
#endif /* __BES_GFPS_API_H__ */
