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
#ifndef __BES_WALKIE_TALKIE_BLE_GAP_API_H__
#define __BES_WALKIE_TALKIE_BLE_GAP_API_H__

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
   ///
   int (*cmp_evt_handler)(uint8_t operation, uint16_t status, uint8_t actv_idx);
   ///
   int (*adv_handler)(uint8_t *data , uint8_t actv_idx, uint8_t *addr, int8_t rssi, uint8_t length);
   ///
   int (*stop_ind)(uint8_t actv_idx);
   ///
   int (*per_sync_est_evt)(uint8_t actv_idx, uint8_t *addr);
}bes_if_walie_gap_cb_func_t;

typedef struct
{
    bool is_contain_pa;
    uint16_t pa_interval_1_25ms; // unit 1.25ms
    uint32_t ea_max_interval_ms; // unit ms
    uint32_t ea_min_interval_ms;
    uint8_t primary_adv_phy; // see gap_le_phy_t
    uint8_t secondary_adv_phy; //
}bes_if_ble_walkie_gap_adv_param;// see ble_walkie_gap_adv_param

typedef struct
{
    void (*adv_start)(uint8_t adv_hdl, uint8_t error_code_ea, uint8_t error_code_pa);
    void (*adv_stop)(uint8_t adv_hdl, uint8_t error_code, uint8_t error_code_pa);
    void (*scan_evt)(bool enable, uint8_t error_code);
    void (*recv_ea_data)(const uint8_t *data, uint8_t date_len, const uint8_t *mac, uint8_t rssi);
    void (*recv_pa_data)(uint16_t pa_handle, const uint8_t *data, uint8_t date_len, const uint8_t *mac, uint8_t rssi);
    void (*pa_sync_est)(uint16_t pa_handle, uint8_t *mac, uint8_t status);
    void (*pa_sync_lost)(uint16_t pa_handle, uint8_t *mac);
    void (*pa_sync_terminate)(uint16_t pa_handle, uint8_t *mac);
    void (*add_mesh_list)(uint8_t status);
    void (*clear_mesh_list)(uint8_t status);
}bes_if_ble_walkie_gap_callback;// see ble_walkie_gap_callback

int bta_walkie_gap_init(bes_if_ble_walkie_gap_callback *cb);
int bta_walkie_gap_deinit();
uint8_t bta_walkie_gap_adv_creat(bes_if_ble_walkie_gap_adv_param* param);
int bta_walkie_gap_adv_start(uint8_t adv_handle, uint32_t duartion);
int bta_walkie_gap_adv_stop(uint8_t adv_handle);
int bta_talkie_gap_adv_set_data(uint8_t adv_handle, uint8_t *data, uint8_t data_len);
int bta_walkie_gap_scan_creat(uint16_t interval_ms, uint16_t windows_ms, bool use_filter_list);
int bta_walkie_gap_scan_start(bool filter_duplicates, uint32_t duration_ms);
int bta_walkie_gap_scan_stop();
int bta_walkie_gap_pa_sync_create(const uint8_t *mac_addr);
int bta_walkie_gap_pa_sync_stop(uint16_t pa_sync_hdl);
int bta_walkie_gap_pa_set_data(uint8_t adv_handle, const uint8_t *pa_data, uint8_t data_len);
int bta_walkie_gap_set_white_list(uint8_t *mac_addr, uint8_t count);
int bta_walkie_gap_clear_white_list();
int bta_walkie_gap_set_mesh_list(uint8_t *mac_addr, uint8_t count);
int bta_walkie_gap_clear_mesh_list();

#ifdef __cplusplus
}
#endif

#endif /*_BES_WALKIE_TALKIE_BLE_GAP_API_H__*/
