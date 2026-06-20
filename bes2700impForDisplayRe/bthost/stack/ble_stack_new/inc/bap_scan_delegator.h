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
#ifndef __BAP_BC_DELEG_INC__
#define __BAP_BC_DELEG_INC__
#include "bluetooth.h"

#include "gaf_cfg.h"

#include "bap_broadcast_common.h"

#include "bass_internal.h"

/*DEFINE*/
#define BAP_SCAN_DELEG_RX_STATE_SRC_ID_INVALID  (0xFF)

#define BAP_SCAN_DELEG_SET_PAST_PARAM_MODE      (3)

#define BAP_SCAN_DELEG_HOST_TO_CTRL_MS          (2)

#define BAP_SCAN_DELEG_RX_SRC_SUPP_NUM_MAX      (0b1111)

#define BAP_SCAN_DELEG_PA_SYNC_RETRY_NUM_MAX    (0b0111)

/*TYPEDEF*/

typedef void (*bap_scan_deleg_cb_bond_data)(uint8_t con_lid, uint8_t char_type, uint8_t cli_cfg_bf);

typedef void (*bap_scan_deleg_cb_remote_scan_started)(uint8_t con_lid);

typedef void (*bap_scan_deleg_cb_remote_scan_stopped)(uint8_t con_lid);

typedef void (*bap_scan_deleg_cb_add_source_req)(uint8_t con_lid, uint8_t src_id, const bap_bc_bass_src_info_req_ptr_t *p_src_info);

typedef void (*bap_scan_deleg_cb_modify_source_req)(uint8_t con_lid, uint8_t src_id, const bap_bc_bass_src_info_req_ptr_t *p_src_info);

typedef void (*bap_scan_deleg_cb_correct_bc_code_ind)(uint8_t con_lid, uint8_t src_id, const uint8_t *p_broadcast_code);

typedef void (*bap_scan_deleg_cb_bis_sync_prefered_req)(uint8_t con_lid, uint8_t src_id);

typedef void (*bap_scan_deleg_cb_remove_source_req)(uint8_t con_lid, uint8_t src_id);

typedef struct bap_scan_deleg_evt_callback
{
    bap_scan_deleg_cb_bond_data cb_bond_data;

    bap_scan_deleg_cb_remote_scan_started cb_remote_scan_started;

    bap_scan_deleg_cb_remote_scan_stopped cb_remote_scan_stopped;

    bap_scan_deleg_cb_add_source_req cb_add_src_req;

    bap_scan_deleg_cb_modify_source_req cb_modify_src_req;

    bap_scan_deleg_cb_correct_bc_code_ind cb_correct_bc_code_ind;

    bap_scan_deleg_cb_bis_sync_prefered_req cb_bis_sync_preferd_req;

    bap_scan_deleg_cb_remove_source_req cb_remove_src_req;
} bap_scan_deleg_evt_cb_t;

typedef struct bap_scan_deleg_init_cfg
{
    /// Preferred MTU size
    uint16_t pref_mtu;
    /// Max supported rx src number
    uint8_t num_rx_src_supp;
    /// Sync PA retry count
    uint8_t pa_sync_retry_max;
    /// Scan PA use filter list
    bool scan_pa_use_filter_list;
    /// Sync PA Skip
    uint16_t pa_sync_skip;
    /// Sync PA timeout 10ms
    uint16_t pa_sync_to_10ms;
    /// Wait PAST timeout 10ms
    uint16_t wait_past_to_10ms;
    /// Sync BIG or PA sync estb timeout 10ms
    uint16_t wait_estb_to_10ms;
    /// Scan for PA sync timeout
    uint16_t scan_for_pa_to_10ms;
    /// Sync BIG MSE
    uint16_t big_sync_mse;
    /// Sync BIG timeout 10ms
    uint16_t big_sync_to_10ms;
} bap_scan_deleg_init_cfg_t;

/*FUCNTIONS DECLARATION*/
/**
 * @brief BAP broadcast delegator initilization
 *
 * @param  bap_scan_deleg_init_cfg
 *                     Initilization configuration
 * @param  bap_scan_deleg_evt_cb
 *                     Event callbacks
 *
 * @return int         status
 */
int bap_scan_deleg_init(const bap_scan_deleg_init_cfg_t *bap_scan_deleg_init_cfg, const bap_scan_deleg_evt_cb_t *bap_scan_deleg_evt_cb);

/**
 * @brief BAP broadcast delegator deinitilization
 *
 * @return int         status
 */
int bap_scan_deleg_deinit(void);

#if (GAF_USE_CACHE_GATT_CCCD)
/**
 * @brief BAP broadcast delegator restore CCCD value
 *
 * @see bass_restore_cli_cfg_cache
 */
#define bap_scan_deleg_restore_cli_cfg_cache(con_lid, cli_cfg_bf)\
                                                    bass_restore_cli_cfg_cache((uint8_t)con_lid, (uint8_t)cli_cfg_bf)
#endif /// (GAF_USE_CACHE_GATT_CCCD)

/**
 * @brief BAP broadcast delegator confirm add source request
 *
 * @param  src_id      Source ID
 * @param  accept      Accept or not
 *
 * @return int         status
 */
int bap_scan_deleg_add_source_upper_cfm(uint8_t src_id, bool accept);

/**
 * @brief BAP broadcast delegator confirm modify source request
 *
 * @param  src_id      Source ID
 * @param  accept      Accept or not
 *
 * @return int         status
 */
int bap_scan_deleg_modify_source_upper_cfm(uint8_t src_id, bool accept);

/**
 * @brief BAP broadcast delegator confirm bis sync prefer bitfield request
 *
 * @param  src_id      Source ID
 * @param  bis_sync_pref
 *                     BIS sync bitfield preferred
 *
 * @return int         status
 */
int bap_scan_deleg_bis_sync_pref_upper_cfm(uint8_t src_id, uint32_t bis_sync_pref);

/**
 * @brief BAP broadcast delegator confirm remove source request
 *
 * @param  src_id      Source ID
 * @param  accept      Accept or not
 *
 * @return int         status
 */
int bap_scan_deleg_remove_source_upper_cfm(uint8_t src_id, bool accept);

/**
 * @brief BAP broadcast delegator add source local
 *
 * @param  pa_ddr      Source info PA address
 * @param  broadcast_id
 *                     Broadcast ID value pointer
 * @param  auto_sync_pa
 *                     Auto Sync PA
 * @param  num_sub_grp Number of subgrps
 * @param  p_sub_grp   Subgrp info list
 *
 * @return int         status
 */
int bap_scan_deleg_add_source(const bap_bc_pa_addr_t *pa_ddr, const uint8_t *broadcast_id, bool auto_sync_pa, uint8_t num_sub_grp, const bap_bc_sub_grp_t *p_sub_grp);

/**
 * @brief BAP broadcast delegator modify source local
 *
 * @param  src_id      Source ID
 * @param  is_sync_pa  Sync PA or not
 * @param  num_sub_grp Number of subgrps
 * @param  p_sub_grp   Subgrp info list
 *
 * @return int         status
 */
int bap_scan_deleg_modify_source(uint8_t src_id, bool is_sync_pa, uint8_t num_sub_grp, const bap_bc_sub_grp_t *p_sub_grp);

/**
 * @brief BAP broadcast delegator set broadcast code local
 *
 * @param  src_id      Source ID
 * @param  p_broadcast_code
 *                     Broadcast Code value pointer
 *
 * @return int         status
 */
int bap_scan_deleg_set_broadcast_code(uint8_t src_id, const uint8_t *p_broadcast_code);

/**
 * @brief BAP broadcast delegator remove source local
 *
 * @param  src_id      Source ID
 *
 * @return int         status
 */
int bap_scan_deleg_remove_source(uint8_t src_id);
#endif /// __BAP_BC_DELEG_INC__