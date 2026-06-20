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
#ifndef __ARC_VCS_INT__
#define __ARC_VCS_INT__

#include "bluetooth.h"

#include "gaf_cfg.h"
#include "gaf_log.h"

#include "vc_common.h"

#define VCS_CONNECTION_MAX          (GAF_CONNECTION_MAX)

/// Callback for vcs bond data
typedef void (*vcs_cb_bond_data_evt)(uint8_t con_lid, uint8_t char_type, uint8_t cli_cfg_bf);
/// Callback for vcs volume change
typedef void (*vcs_cb_volume_state_change_evt)(uint8_t volume, uint8_t mute, uint8_t change_cnt,
                                               bool is_local, uint8_t con_lid);
/// Callback for vcs flags change
typedef void (*vcs_cb_volume_flags_change_evt)(uint8_t con_lid, uint8_t flags);

typedef struct vcs_event_callback
{
    /// Callback for vcs bond data
    vcs_cb_bond_data_evt cb_bond_data;
    /// Callback for vcs volume change
    vcs_cb_volume_state_change_evt cb_vol_state_change;
    /// Callback for vcs flags change
    vcs_cb_volume_flags_change_evt cb_vol_flags_change;
} vcs_evt_cb_t;

typedef struct vcs_init_cfg
{
    /// Preferred MTU size
    uint16_t pref_mtu;
    /// Volume Step
    uint8_t vol_step;
#if (ARC_AICS_ENABLE)
    /// AICS(s) include by VCS
    uint8_t include_aics_num;
    /// AICS input lid list
    const uint8_t *p_input_lid_list;
#endif /// (ARC_AICS_ENABLE)
#if (ARC_VOCS_ENABLE)
    /// VOCS(s) include by VCS
    uint8_t include_vocs_num;
    /// VOCS output lid list
    const uint8_t *p_output_lid_list;
#endif /// (ARC_VOCS_ENABLE)
} vcs_cfg_t;

typedef struct vcs_volume_cfg
{
    /// Mute
    bool mute;
    /// Volume Setting
    uint8_t vol_setting;
    /// Volume Setting Persisted
    bool vol_setting_persisted;
} vcs_vol_cfg_t;

/*FUCNTIONS DECLARATION*/
/**
 * @brief Volume Control server initilization
 *
 * @param  vcs_init_cfg
 *                     Initilization configuration
 * @param  vcs_evt_cb  Event callbacks
 *
 * @return int         status
 */
int vcs_init(const vcs_cfg_t *vcs_init_cfg, const vcs_evt_cb_t *vcs_evt_cb);

/**
 * @brief Volume Control server deinitilization
 *
 * @return int         status
 */
int vcs_deinit(void);

#if (GAF_USE_CACHE_GATT_CCCD)
/**
 * @brief Volume Control server restore client configuration bitfield
 *
 * @param  con_lid     Connection local index
 * @param  cli_cfg_bf  Client configuration bitfield
 *
 * @return int         status
 */
int vcs_restore_cli_cfg_cache(uint8_t con_lid, uint8_t cli_cfg_bf);
#endif /// (GAF_USE_CACHE_GATT_CCCD)

/**
 * @brief Volume Control server restore Volume info
 *
 * @param  con_lid     Connection local index
 * @param  p_vol_cfg   Voume Configuraton
 *
 * @return int         status
 */
int vcs_restore_volume_info(uint8_t con_lid, const vcs_vol_cfg_t *p_vol_cfg);

/**
 * @brief Volume Control server control local volume
 *
 * @param  con_lid     Connection local index
 * @param  op_code     Operation code @see vcs_opcode
 * @param  volume      Volume setting
 * @param  need_cb_upper
 *                     Is need to inform upper action complete
 * @param  need_ntf    Is need to inform peer about action complete
 *
 * @return int         status
 */
int vcs_control(uint8_t con_lid, uint8_t op_code, uint8_t volume, bool need_cb_upper, bool need_ntf);

#endif /// __ARC_VCS_INT__