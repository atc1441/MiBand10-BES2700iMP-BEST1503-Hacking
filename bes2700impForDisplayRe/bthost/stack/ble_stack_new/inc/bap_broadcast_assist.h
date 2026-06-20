/***************************************************************************
 *
 * Copyright (c) 2015-2023 BES Technic
 *
 * Authored by BES CD team (Blueelf Prj).
 *
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a databrx src or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifndef __BAP_BC_ASSIST_INC__
#define __BAP_BC_ASSIST_INC__
#include "bluetooth.h"

#include "gaf_cfg.h"

#include "gaf_prf.h"
#include "generic_audio.h"
#include "bap_broadcast_common.h"

/*DEFINE*/
#define BAP_BC_ASSIST_CONNECTION_MAX                (GAF_CONNECTION_MAX)

#define BAP_BC_ASSIST_MAX_SUPP_RX_SRC_FOUND         (0b1111)

/*TYPEDEF*/
typedef struct bap_bc_assist_prf_svc_info
{
    /// Service handler range
    gatt_prf_svc_range_t svc_range;
    /// Service UUID
    uint16_t uuid;
} bap_bc_assist_prf_svc_t;

/// Callback for bass discovery cmp
typedef void (*bap_bc_assist_cb_discovery_cmp)(uint8_t con_lid, uint16_t err_code);
/// Callback for bass bond data
typedef void (*bap_bc_assist_cb_bond_data)(uint8_t con_lid, const bap_bc_assist_prf_svc_t *p_svc_info);
/// Callback for basc gatt set cfg cmp evt
typedef void (*bap_bc_assist_cb_set_cfg_cmp_evt)(uint8_t con_lid, uint8_t char_type, uint16_t err_code);
/// Callback for bass wr scan cp err occure
typedef void (*bap_bc_assist_cb_scan_cp_err_ind)(uint8_t con_lid, uint8_t src_lid, uint8_t src_id, uint8_t op_code, uint16_t err_code);
/// Callback for bass rx state empty
typedef void (*bap_bc_assist_cb_empty_rx_state_ind)(uint8_t con_lid, uint8_t src_lid);
/// Callback for bass rx src ind
typedef void (*bap_bc_assist_cb_rx_src_info_ind)(uint8_t con_lid, uint8_t src_lid, uint8_t src_id, const bap_bc_bass_src_info_ind_ptr_t *p_src_info);
/// Callback for bass rx src broadcast code get
typedef void (*bap_bc_assist_cb_broadcast_code_req)(uint8_t con_lid, uint8_t src_lid, uint8_t src_id);
/// Callback function called when prf status event generated
typedef gatt_prf_status_event_callback bap_bc_assist_cb_prf_status_evt;

typedef struct bap_bc_assist_evt_callback
{
    /*BASC GATT callback*/
    bap_bc_assist_cb_discovery_cmp cb_discovery_cmp;
    bap_bc_assist_cb_bond_data cb_bond_data;
    bap_bc_assist_cb_set_cfg_cmp_evt cb_set_cfg_cmp;
    /*SCAN CP err callback*/
    bap_bc_assist_cb_scan_cp_err_ind cb_scan_cp_err_ind;
    /*RX state empty callback*/
    bap_bc_assist_cb_empty_rx_state_ind cb_rx_state_empty_ind;
    /*RX src ind*/
    bap_bc_assist_cb_rx_src_info_ind cb_rx_src_info_ind;
    /*RX src broadcast code required*/
    bap_bc_assist_cb_broadcast_code_req cb_broadcast_code_req;
    /// Callback function called when prf status event generated
    bap_bc_assist_cb_prf_status_evt cb_prf_status_event;
} bap_bc_assist_evt_cb_t;

typedef struct bap_bc_assist_init_cfg
{
    /// Max support RX state found
    uint8_t max_supp_rx_state;
    /// Write cmd to control point
    bool cp_write_reliable;
    /// PA sync timeout ms
    uint16_t pa_sync_timeout_10ms;
    /// PA sync estb timeout ms
    uint16_t pa_sync_estb_to_10ms;
    /// Scan for PA timeout
    uint16_t scan_for_pa_to_10ms;
    /// Scan for PA use filter list
    bool scan_pa_use_filter_list;
    /// Preferred MTU size
    uint16_t pref_mtu;
} bap_bc_assist_init_cfg_t;

/*FUNCTIONS*/
/**
 * @brief BAP broadcast assistant initilization
 *
 * @param  p_init_cfg  Initilization configuration
 * @param  p_cb        Event callback
 *
 * @return int         status
 */
int bap_bc_assist_init(const bap_bc_assist_init_cfg_t *p_init_cfg, const bap_bc_assist_evt_cb_t *p_cb);

/**
 * @brief BAP broadcast assistant deinitilization
 *
 * @return int         status
 */
int bap_bc_assist_deinit(void);

/**
 * @brief BAP broadcast assistant bass service discovery
 *
 * @param  con_lid     Connection local index
 *
 * @return int         status
 */
int bap_bc_assist_bass_discovery(uint8_t con_lid);

/**
 * @brief BAP broadcast assistant read character Receive state
 *
 * @param  con_lid     Connection local index
 * @param  src_lid     Source local index
 *
 * @return int         status
 */
int bap_bc_assist_bass_read_rx_state(uint8_t con_lid, uint8_t src_lid);

/**
 * @brief BAP broadcast assistant send remote scan start/stop
 *
 * @param  con_lid     Connection local index
 * @param  is_started  Start or Stop scan
 *
 * @return int         status
 */
int bap_bc_assist_send_remote_scan_state(uint8_t con_lid, bool is_started);

/**
 * @brief BAP broadcast assistant Add source
 *
 * @param  con_lid     Connection local index
 * @param  src_lid     Source local index
 * @param  p_src_info  Source info reuest
 *
 * @return int         status
 */
int bap_bc_assist_add_source(uint8_t con_lid, uint8_t src_lid, const bap_bc_bass_src_info_req_ptr_t *p_src_info);

/**
 * @brief BAP broadcast assistant Modify source
 *
 * @param  con_lid     Connection local index
 * @param  src_lid     Source local index
 * @param  p_src_info  Source info reuest
 *
 * @return int         status
 */
int bap_bc_assist_modify_source(uint8_t con_lid, uint8_t src_lid, const bap_bc_bass_src_info_req_ptr_t *p_src_info);

/**
 * @brief BAP broadcast assistant Confirm Broadcast Code request
 *
 * @param  con_lid     Connection local index
 * @param  src_lid     Source local index
 * @param  p_broadcast_code
 *                     Broadcast Code value pointer
 *
 * @return int         status
 */
int bap_bc_assist_broadcast_code_req_upper_cfm(uint8_t con_lid, uint8_t src_lid, const uint8_t *p_broadcast_code);

/**
 * @brief BAP broadcast assistant Remove source
 *
 * @param  con_lid     Connection local index
 * @param  src_lid     Source local index
 *
 * @return int         status
 */
int bap_bc_assist_remove_source(uint8_t con_lid, uint8_t src_lid);

#endif /// __BAP_BC_ASSIST_INC__