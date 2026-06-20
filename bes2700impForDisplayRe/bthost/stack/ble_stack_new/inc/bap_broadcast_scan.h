/***************************************************************************
 *
 * Copyright (c) 2015-2023 BES Technic
 *
 * Authored by BES CD team (Blueelf Prj).
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
 ****************************************************************************/
#ifndef __BAP_BC_SCAN_INC__
#define __BAP_BC_SCAN_INC__
#include "bluetooth.h"

#include "bap_broadcast_common.h"
#include "public_broadcast_audio.h"

#include "gap_service.h"

typedef void (*bap_bc_scan_adv_scan_started)(uint16_t err_code);

typedef void (*bap_bc_scan_adv_scan_stopped)(uint16_t err_code);

typedef void (*bap_bc_scan_extended_adv_recv)(const uint8_t *broadcast_id, const pbp_broadcast_name_ptr_t *p_bc_name,
                                              const pbp_pba_info_t *p_pba_info, const gap_adv_report_t *p_ext_adv_raw);

typedef void (*bap_bc_scan_pa_sync_state)(const bap_bc_pa_addr_t *p_pa_addr, uint16_t pa_sync_hdl, uint16_t err_code);

typedef void (*bap_bc_scan_pa_data_recv)(uint16_t pa_sync_hdl, const bap_bc_base_grp_info_t *p_base_info, const gap_adv_report_t *p_periodic_adv_raw);

typedef void (*bap_bc_scan_pa_big_info_recv)(uint16_t pa_sync_hdl, const bap_bc_big_info_t *p_big_info);

typedef void (*bap_bc_scan_pa_sync_term)(uint16_t pa_sync_hdl, uint16_t err_code);

typedef struct bap_bc_scan_callbacks
{
    bap_bc_scan_adv_scan_started cb_scan_started;

    bap_bc_scan_adv_scan_stopped cb_scan_stopped;

    bap_bc_scan_extended_adv_recv cb_ext_adv_recv;

    bap_bc_scan_pa_sync_state cb_pa_sync_state;

    bap_bc_scan_pa_data_recv cb_per_adv_data_recv;

    bap_bc_scan_pa_sync_term cb_pa_sync_term;

    bap_bc_scan_pa_big_info_recv cb_big_info_recv;

} bap_bc_scan_callback_t;

typedef struct bap_bc_scan_init_cfg
{
    const gap_scan_timing_t *p_scan_timing;

} bap_bc_scan_init_cfg_t;

#if (BAP_BROADCAST_SCAN)
/*FUNCTIONS DECLARATION*/
/**
 * @brief BAP broadcast Scan Initilization
 *
 * @param  p_init_cfg  Initilization configuration
 *
 * @return int         status
 */
int bap_bc_scan_init(const bap_bc_scan_init_cfg_t *p_init_cfg);

/**
 * @brief BAP broadcast Scan deinitilization
 *
 * @return int         status
 */
int bap_bc_scan_deinit(void);

/**
 * @brief BAP broadcast Scan Callbacks register
 *
 * @param  user        Internal USER @see bap_bc_common_user_e
 * @param  p_evt_cb    Event callbacks
 *
 * @return int         status
 */
int bap_bc_scan_callback_register(bap_bc_common_user_e user, const bap_bc_scan_callback_t *p_evt_cb);

/**
 * @brief BAP broadcast Scan Callbacks deregister
 *
 * @param  user        Internal USER @see bap_bc_common_user_e
 *
 * @return int         status
 */
int bap_bc_scan_callback_unregister(bap_bc_common_user_e user);

/**
 * @brief BAP broadcast Scan Start scan
 *
 * @param  timeout_10ms
 *                     Timeout 10ms
 * @param  filter_policy
 *                     scanning filter policy
 *
 * @return int         status
 */
int bap_bc_scan_start_scan(uint16_t timeout_10ms, uint8_t filter_policy);

/**
 * @brief BAP broadcast Scan Stop scan
 *
 * @return int         status
 */
int bap_bc_scan_stop_scan(void);

/**
 * @brief BAP broadcast Scan Sync PA
 *
 * @param  p_pa_addr   Source info PA address
 * @param  skip        Skip event
 * @param  timeout_10ms
 *                     Sync timeput 10ms
 * @param  wait_sync_estb_to_10ms
 *                     Wait PA Sync estb timeput 10ms, 0 means no wait
 *
 * @return int         status
 */
int bap_bc_scan_sync_pa(const bap_bc_pa_addr_t *p_pa_addr, uint16_t skip, uint16_t timeout_10ms, uint16_t wait_sync_estb_to_10ms);

/**
 * @brief BAP broadcast Cancel Scan Sync PA
 *
 * @return void         If create sync pa is running, will
 *                      generate estb event with err 0x44
 */
void bap_bc_scan_cancel_sync_pa(void);

/**
 * @brief BAP broadcast Control PA report
 *
 * @param  pa_sync_hdl PA sync handle
 * @param  enable      Enable or not enable report
 * @param  filter_duplicated
 *                     Filter duplicated packet according to ADI if support
 *
 * @return int         status
 */
int bap_bc_scan_pa_receive_enable(uint16_t pa_sync_hdl, bool enable, bool filter_duplicated);

/**
 * @brief BAP broadcast Scan PA sync terminate
 *
 * @param  pa_sync_hdl PA sync handle
 *
 * @return int         status
 */
int bap_bc_scan_term_pa_sync(uint16_t pa_sync_hdl);
#endif /// (BAP_BROADCAST_SCAN)
#endif /// __BAP_BC_SCAN_INC__