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
#ifndef __ARC_VOCC__
#define __ARC_VOCC__
#include "bluetooth.h"

#include "gaf_prf.h"
#include "gaf_cfg.h"
#include "gaf_log.h"

#include "voc_common.h"

#define VOCC_CONNECTION_MAX                     (GAF_CONNECTION_MAX)

typedef struct vocc_prf_svc_info vocc_prf_svc_t;

/// Callback function called when Audio Input Control Service instance has been discovered in a peer server
/// database
typedef void (*vocc_cb_bond_data_evt)(uint8_t con_lid, uint8_t output_lid, const vocc_prf_svc_t *param);
/// Callback for vocc discovery done
typedef void (*vocc_cb_discovery_cmp_evt)(uint8_t con_lid, uint16_t err_code);
/// Callback for vocc gatt set cfg cmp evt
typedef void (*vocc_cb_set_cfg_cmp_evt)(uint8_t con_lid, uint8_t char_type, uint8_t output_lid, uint16_t err_code);
/// Callback for vocc op set val complete
typedef void (*vocc_cb_set_val_cmp_evt)(uint8_t con_lid, uint8_t output_lid, uint8_t char_type, uint16_t err_code);
/// Callback function called when value for Audio Input Description characteristic has been received from a peer
/// server device
typedef void (*vocc_cb_description)(uint8_t con_lid, uint8_t output_lid, uint16_t desc_val_len, const uint8_t *p_desc_val, uint16_t err_code);
/// Callback function called when value for Audio Input Type or Audio Audio Input Status characteristic has been
/// received from a peer server device
typedef void (*vocc_cb_value)(uint8_t con_lid, uint8_t output_lid, uint8_t char_type, uint32_t val, uint16_t err_code);
/// Callback for vocc cp write cmp evt
typedef void (*vocc_cb_ctrl_op_cmp_evt)(uint8_t con_lid, uint8_t opcode, uint16_t err_code);
/// Callback function called when prf status event generated
typedef gatt_prf_status_event_callback vocc_cb_prf_status_evt;

/*Structure*/

typedef struct vocc_init_cfg
{
    /// Max support number to found aics
    uint8_t vocs_found_num_supp_max;
    /// Preferred MTU size
    uint16_t pref_mtu;
} vocc_init_cfg_t;

typedef struct vocc_evt_cb
{
    /// Callback function called when client configuration for aics has been updated
    vocc_cb_bond_data_evt cb_bond_data;
    /// Callback function called when aics is discovered or failed
    vocc_cb_discovery_cmp_evt cb_discovery_cmp;
    /// Callback for vocc gatt cmd complete
    vocc_cb_set_cfg_cmp_evt cb_set_cfg_cmp;
    /// Callback for vocc op set val complete
    vocc_cb_set_val_cmp_evt cb_set_val_cmp;
    /// Callback function called when value for Audio Input Description characteristic has been received from a peer
    /// server device
    vocc_cb_description cb_description;
    /// Callback function called when value for Audio Input Type or Audio Audio Input Status characteristic has been
    /// received from a peer server device
    vocc_cb_value cb_value;
    /// Callback for vocc cp write cmp evt
    vocc_cb_ctrl_op_cmp_evt cb_ctrl_op_cmp;
    /// Callback function called when prf status event generated
    vocc_cb_prf_status_evt cb_prf_status_event;
} vocc_evt_cb_t;

struct vocc_prf_svc_info
{
    /// Service handler range
    gatt_prf_svc_range_t svc_range;
    /// Service UUID
    uint16_t uuid;
};

/*FUCNTIONS DECLARATION*/
/**
 * @brief Volume offset control client initilization
 *
 * @param  p_init_cfg  Initilization configuration
 * @param  p_cb        Event callbacks
 *
 * @return int         status
 */
int vocc_init(const vocc_init_cfg_t *p_init_cfg, const vocc_evt_cb_t *p_cb);

/**
 * @brief Volume offset control client deinitilization
 *
 * @return int         status
 */
int vocc_deinit(void);

/**
 * @brief Volume offset control client read character value
 *
 * @param  con_lid     Connection local index
 * @param  output_lid  Output local index
 * @param  char_type   Character type
 *
 * @return int         status
 */
int vocc_character_value_read(uint8_t con_lid, uint8_t output_lid, uint8_t char_type);

/**
 * @brief Volume offset control client write character cccd
 *
 * @param  con_lid     Connection local index
 * @param  output_lid  Output local index
 * @param  char_type   Character type
 * @param  enable_ntf  Enable or disable notify
 *
 * @return int         status
 */
int vocc_character_cccd_write(uint8_t con_lid, uint8_t output_lid, uint8_t char_type, bool enable_ntf);

/**
 * @brief Volume offset control client write character cccd
 *
 * @param  con_lid     Connection local index
 * @param  output_lid  Output local index
 * @param  char_type   Character type
 * @param  p_val       Value pointer
 * @param  val_len     Value length
 *
 * @return int         status
 */
int vocc_character_val_write(uint8_t con_lid, uint8_t output_lid, uint8_t char_type, const uint8_t *p_val, uint8_t val_len);

/**
 * @brief Volume offset control client control
 *
 * @param  con_lid     Connection local index
 * @param  output_lid  Output local index
 * @param  offset      Volume offset
 *
 * @return int         status
 */
int vocc_offset_control_operation(uint8_t con_lid, uint8_t output_lid, int16_t offset);

#endif /// __ARC_VOCC__