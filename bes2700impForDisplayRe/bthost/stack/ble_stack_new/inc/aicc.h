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
#ifndef __ARC_AICC__
#define __ARC_AICC__
#include "bluetooth.h"

#include "gaf_prf.h"
#include "gaf_cfg.h"
#include "gaf_log.h"

#include "aic_common.h"

#define AICC_CONNECTION_MAX                     (GAF_CONNECTION_MAX)

typedef struct aicc_prf_svc_info aicc_prf_svc_t;

/// Callback function called when Audio Input Control Service instance has been discovered in a peer server
/// database
typedef void (*aicc_cb_bond_data_evt)(uint8_t con_lid, uint8_t input_lid, const aicc_prf_svc_t *param);
/// Callback for aicc discovery done
typedef void (*aicc_cb_discovery_cmp_evt)(uint8_t con_lid, uint16_t err_code);
/// Callback for aicc gatt set cfg cmp evt
typedef void (*aicc_cb_set_cfg_cmp_evt)(uint8_t con_lid, uint8_t char_type, uint8_t input_lid, uint16_t err_code);
/// Callback for vocc op set desc val complete
typedef void (*aicc_cb_set_desc_cmp_evt)(uint8_t con_lid, uint8_t output_lid, uint16_t err_code);
/// Callback function called when value for Audio Input State characteristic has been received from a peer server
/// device
typedef void (*aicc_cb_input_state)(uint8_t con_lid, uint8_t input_lid, int8_t gain, uint8_t mute, uint8_t mode, uint16_t err_code);
/// Callback function called when value for Gain Setting Properties characteristic has been received from a peer
/// server device
typedef void (*aicc_cb_gain_prop)(uint8_t con_lid, uint8_t input_lid, uint8_t units, int8_t min, int8_t max, uint16_t err_code);
/// Callback function called when value for Audio Input Description characteristic has been received from a peer
/// server device
typedef void (*aicc_cb_description)(uint8_t con_lid, uint8_t input_lid, uint16_t desc_val_len, const uint8_t *p_desc_val, uint16_t err_code);
/// Callback function called when value for Audio Input Type or Audio Audio Input Status characteristic has been
/// received from a peer server device
typedef void (*aicc_cb_value)(uint8_t con_lid, uint8_t input_lid, uint8_t char_type, uint8_t val, uint16_t err_code);
/// Callback for aicc cp write cmp evt
typedef void (*aicc_cb_ctrl_op_cmp_evt)(uint8_t con_lid, uint8_t input_lid, uint8_t opcode, uint16_t err_code);
// Callback function called when prf status event generated
typedef gatt_prf_status_event_callback aicc_cb_prf_status_evt;

/*Structure*/
typedef struct aicc_init_cfg
{
    /// Max support number to found aics
    uint8_t aics_found_num_supp_max;
    /// Preferred MTU size
    uint16_t pref_mtu;
} aicc_init_cfg_t;

typedef struct aicc_evt_cb
{
    /// Callback function called when client configuration for aics has been updated
    aicc_cb_bond_data_evt cb_bond_data;
    /// Callback function called when aics is discovered or failed
    aicc_cb_discovery_cmp_evt cb_discovery_cmp;
    /// Callback for aicc gatt cmd complete
    aicc_cb_set_cfg_cmp_evt cb_set_cfg_cmp;
    /// Callback for vocc op set desc val complete
    aicc_cb_set_desc_cmp_evt cb_set_desc_cmp;
    /// Callback function called when value for Audio Input State characteristic has been received from a peer server
    /// device
    aicc_cb_input_state cb_input_state;
    /// Callback function called when value for Gain Setting Properties characteristic has been received from a peer
    /// server device
    aicc_cb_gain_prop cb_gain_prop;
    /// Callback function called when value for Audio Input Description characteristic has been received from a peer
    /// server device
    aicc_cb_description cb_description;
    /// Callback function called when value for Audio Input Type or Audio Audio Input Status characteristic has been
    /// received from a peer server device
    aicc_cb_value cb_value;
    /// Callback for aicc cp write cmp evt
    aicc_cb_ctrl_op_cmp_evt cb_ctrl_op_cmp;
    // Callback function called when prf status event generated
    aicc_cb_prf_status_evt cb_prf_status_event;
} aicc_evt_cb_t;

struct aicc_prf_svc_info
{
    /// Service handler range
    gatt_prf_svc_range_t svc_range;
    /// Service UUID
    uint16_t uuid;
};

/*FUCNTIONS DECLARATION*/
/**
 * @brief Audio Input Control Client initilization
 *
 * @param  p_init_cfg  Init configuration
 * @param  p_cb        Event callbacks
 *
 * @return int         status
 */
int aicc_init(const aicc_init_cfg_t *p_init_cfg, const aicc_evt_cb_t *p_cb);

/**
 * @brief Audio Input Control Client deinitilization
 *
 * @return int         status
 */
int aicc_deinit(void);

/**
 * @brief Audio Input Control Client Character Value Read
 *
 * @param  con_lid     Connection local index
 * @param  input_lid   Audio input local index
 * @param  char_type   Character type @see enum aics_char_type
 *
 * @return int         status
 */
int aicc_character_value_read(uint8_t con_lid, uint8_t input_lid, uint8_t char_type);

/**
 * @brief Audio Input Control Client Character CCCD write
 *
 * @param  con_lid     Connection local index
 * @param  input_lid   Audio input local index
 * @param  char_type   Character type @see enum aics_char_type
 * @param  enable_ntf  Enable or disable notify
 *
 * @return int         status
 */
int aicc_character_cccd_write(uint8_t con_lid, uint8_t input_lid, uint8_t char_type, bool enable_ntf);

/**
 * @brief Audio Input Control Client VCP oeration
 *
 * @param  con_lid     Connection local index
 * @param  input_lid   Audio input local index
 * @param  opcode      Operation code  @see enum aics_opcode
 * @param  gain        Audio input gain setting
 *
 * @return int         status
 */
int aicc_control_operation(uint8_t con_lid, uint8_t input_lid, uint8_t opcode, int8_t gain);

/**
 * @brief Audio Input Control Client Set Character Audio Input Description Value
 *
 * @param  con_lid     Connection local index
 * @param  input_lid   Audio input local index
 * @param  desc_val_len
 *                     Description value length
 * @param  p_desc_val  Description value pointer
 *
 * @return int         status
 */
int aicc_set_description(uint8_t con_lid, uint8_t input_lid, uint16_t desc_val_len, const uint8_t *p_desc_val);

#endif /// __ARC_AICC__