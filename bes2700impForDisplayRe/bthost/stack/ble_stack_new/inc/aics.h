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
#ifndef __ARC_AICS__
#define __ARC_AICS__
#include "bluetooth.h"

#include "gaf_cfg.h"
#include "generic_audio.h"
#include "gaf_log.h"

#include "aic_common.h"

#define AICS_CONNECTION_MAX             (GAF_CONNECTION_MAX)

/// Callback for aics bond data
typedef void (*aics_cb_bond_data)(uint8_t con_lid, uint8_t input_lid, uint8_t char_type, uint8_t cli_cfg_bf);
/// Callback for aics audio input state
typedef void (*aics_cb_input_state)(uint8_t input_lid, aics_state_t *p_input_state);
/// Callback for aics description write req
typedef void (*aics_cb_description_wr_req)(uint8_t con_lid, uint8_t input_lid, uint8_t desc_val_len, const uint8_t *p_desc_val);

/*Structure*/
typedef struct aics_evt_cb
{
    /// Callback function called when client configuration for AICS has been updated
    aics_cb_bond_data cb_bond_data;
    /// Callback function called when Audio input state is changed
    aics_cb_input_state cb_state;
    /// Callback for aics description write req
    aics_cb_description_wr_req cb_desc_req;
} aics_evt_cb_t;

typedef struct aics_inst_cfg
{
    /// Preferred MTU size
    uint16_t pref_mtu;
    /// Maximum length of Audio Input Description
    uint8_t desc_max_len;
    /// Gain
    int8_t gain;
    /// Mute
    uint8_t mute;
    /// Gain Mode
    uint8_t gain_mode;
    /// Gain Units
    uint8_t gain_units;
    /// Gain Minimum
    int8_t gain_min;
    /// Gain Maximum
    int8_t gain_max;
    /// Input Type
    uint8_t input_type;
} aics_inst_cfg_t;

typedef struct aics_init_cfg
{
    /// Max supported AICS inst
    uint8_t aics_inst_supp_max;
} aics_init_cfg_t;

/*FUNCTIONS DECLARATION*/
/**
 * @brief Audio Input Contrl Server Initilization
 *
 * @param  aics_init_cfg
 *                     AICS initilization configuration
 * @param  aics_evt_cb AICS event callbacks
 *
 * @return int         status
 */
int aics_init(const aics_init_cfg_t *aics_init_cfg, const aics_evt_cb_t *aics_evt_cb);

/**
 * @brief Audio Input Contrl Server Deinitilization
 *
 * @return int         status
 */
int aics_deinit(void);

/**
 * @brief Audio Input Contrl Server Add service instant
 *
 * @param  p_aics_inst_cfg
 *                     AICS instant configuration
 * @param  p_input_lid_ret
 *                     AICS instant input local index return
 *
 * @return int         status
 */
int aics_add_aics_instant(const aics_inst_cfg_t *p_aics_inst_cfg, uint8_t *p_input_lid_ret);

#if (GAF_USE_CACHE_GATT_CCCD)
/**
 * @brief Audio Input Contrl Server Restore CCCD value
 *
 * @param  con_lid     Connection local index
 * @param  input_lid   Audio input local index
 * @param  cli_cfg_bf  Client Configuration Bitfield
 *
 * @return int         status
 */
int aics_restore_cli_cfg_cache(uint8_t con_lid, uint8_t input_lid, uint8_t cli_cfg_bf);
#endif /// (GAF_USE_CACHE_GATT_CCCD)

/**
 * @brief Audio Input Contrl Server Set character value
 *
 * @param  input_lid   Audio input local index
 * @param  set_type    Value set type @see enum aics_set_type
 * @param  p_val       Value to set
 * @param  val_len     Value length to set
 *
 * @return int         status
 */
int aics_set_value(uint8_t input_lid, uint8_t set_type, const uint8_t *p_val, uint8_t val_len);

/**
 * @brief Audio Input Contrl Server Confirm set Audio Input Description Value request
 *
 * @param  cfm_status  Confirm status
 * @param  con_lid     Connection local index
 * @param  input_lid   Audio input local index
 * @param  desc_val_len
 *                     Description value length to be confirmed
 * @param  p_desc_val  Description value to be confirmed
 *
 * @return int         status
 */
int aics_set_description_cfm(uint16_t cfm_status, uint8_t con_lid, uint8_t input_lid, uint8_t desc_val_len, const uint8_t *p_desc_val);
#endif /// __ARC_AICS__