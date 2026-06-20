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
#ifndef __APP_BTGATT_H__
#define __APP_BTGATT_H__
#include "bluetooth.h"

#if defined(__GATT_OVER_BR_EDR__)

#define ATT_SERVICE_UUID 0x0118
#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t btif_btgatt_event;
typedef void (*btgatt_event_callback)(uint8_t conidx,btif_btgatt_event event, uint8_t reason);

bool app_btgatt_over_br_edr_enabled(void);
bool app_btgatt_is_connected(uint8_t device_id);
bool app_btgatt_is_connected_by_conidx(uint8_t con_idx);
void app_btgatt_client_create(const bt_bdaddr_t *remote);
void app_btgatt_disconnect(uint8_t device_id);

void app_btgatt_init(void);
void app_btgatt_addsdp(uint16_t pServiceUUID, uint16_t startHandle, uint16_t endHandle);
void app_btgatt_register_callback(btgatt_event_callback cb);

#ifdef __cplusplus
}
#endif

#endif
#endif

