/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#ifndef __BLE_DEMO_APP_H__
#define __BLE_DEMO_APP_H__

#include "bluetooth.h"

#ifdef __cplusplus
extern "C" {
#endif

int ble_demo_app_init(uint8_t adv_hdl_shared, const bt_bdaddr_t *p_app_ia_shared, const uint8_t *p_irk_shared);
int ble_demo_app_deinit(void);
void ble_demo_app_connection_established_handler(uint8_t conidx);
uint8_t ble_demo_app_add_resolving_list_item(void);
void ble_demo_app_tws_sync_info(void);
void ble_demo_app_tws_sync_info_receive_handler(uint8_t *p_info, uint16_t len);

void ble_demo_app_tws_role_switch_start(uint8_t curr_ui_role);
void ble_demo_app_tws_role_switch_cmp(uint8_t curr_ui_role);
void ble_demo_app_tws_role_update(uint8_t curr_ui_role);

bool ble_demo_app_is_connection_support_le_rs(uint16_t connhdl);

#ifdef __cplusplus
}
#endif

#endif /* __BLE_DEMO_APP_H__ */

