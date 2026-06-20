/**
 ****************************************************************************************
 *
 * @file earbud_ux_api.h
 *
 * @brief APIs For Customer
 *
 * Copyright 2023-2030 BES.
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
#ifndef __EARBUD_UX_DUPLICATE_API__
#define __EARBUD_UX_DUPLICATE_API__
#include "cmsis_os.h"

#ifdef __cplusplus
extern "C" {
#endif

void app_ibrt_if_set_access_mode(ibrt_if_access_mode_enum mode);

void app_ibrt_if_init_open_box_state_for_evb(void);

bool app_ibrt_if_is_maximum_mobile_dev_connected(void);

uint8_t app_ibrt_if_get_support_max_remote_link();

/**
 ****************************************************************************************
 * @brief Get the link key of mobile link
 ****************************************************************************************
 */
void app_ibrt_if_get_mobile_bt_link_key(uint8_t *linkKey1, uint8_t *linkKey2);

/**
 ****************************************************************************************
 * @brief Get the link key of TWS link
 ****************************************************************************************
 */
void app_ibrt_if_get_tws_bt_link_key(uint8_t *linkKey);

/**
 ****************************************************************************************
 * @brief Get all paired BT link key info
 ****************************************************************************************
 */
bool app_ibrt_if_get_all_paired_bt_link_key(ibrt_if_paired_bt_link_key_info *linkKey);

/**
 ****************************************************************************************
 * @brief Get all paired BLE Long Term key(LTK) info
 ****************************************************************************************
 */
bool app_ibrt_if_get_all_paired_ble_ltk(ibrt_if_paired_ble_ltk_info* leLTK);

void app_ibrt_if_write_page_timeout(uint16_t timeout);

bool app_ibrt_if_is_peer_mobile_link_exist_but_local_not_on_tws_connected(void);

// to avoid corner cases, call this API to clear the flag after playing local connected prompt on the slave side
// when app_ibrt_if_is_peer_mobile_link_exist_but_local_not_on_tws_connected returns true
void app_ibrt_if_clear_flag_peer_mobile_link_exist_but_local_not_on_tws_connected(void);

void app_ibrt_if_write_page_scan_setting(uint16_t window_slots, uint16_t interval_slots);

#ifdef IS_TWS_IBRT_DEBUG_SYSTEM_ENABLED
void app_ibrt_if_register_link_loss_universal_info_received_cb(app_tws_ibrt_analysing_info_received_cb callback);
void app_ibrt_if_register_link_loss_clock_info_received_cb(app_tws_ibrt_analysing_info_received_cb callback);
void app_ibrt_if_register_a2dp_sink_info_received_cb(app_tws_ibrt_analysing_info_received_cb callback);
tws_ibrt_link_loss_info_t* app_ibrt_if_get_wireless_signal_link_loss_total_info(void);
tws_ibrt_a2dp_sink_info_t* app_ibrt_if_get_wireless_signal_a2dp_sink_info(void);
#endif

uint8_t app_ibrt_if_get_max_support_link(void);

#ifdef __cplusplus
}
#endif

#endif
