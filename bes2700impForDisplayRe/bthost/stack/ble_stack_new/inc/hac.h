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
#ifndef __HAP_HAC_INC__
#define __HAP_HAC_INC__

#include "bluetooth.h"

#include "gaf_cfg.h"

#include "gaf_prf.h"

#define HAC_CONNECTION_MAX                     (GAF_CONNECTION_MAX)

/*TYPEDEF*/
typedef struct hac_prf_svc_info hac_prf_svc_t;

/// Callback function called when Hearing Access Service has been discovered in a Service device database
typedef void (*hac_cb_bond_data)(uint8_t con_lid, const hac_prf_svc_t *param);
/// Callback function called when Hearing Access Service has been discovered cmp
typedef void (*hac_cb_discovery_cmp)(uint8_t con_lid, uint16_t err_code);
/// Callback for hac gatt set cfg cmp evt
typedef void (*hac_cb_set_cfg_cmp)(uint8_t con_lid, uint8_t char_type, uint16_t err_code);
/// Callback function called when a Preset Record is received
typedef void (*hac_cb_preset)(uint8_t con_lid, bool last, uint8_t preset_idx, bool writable,
                              bool available, uint8_t length, const uint8_t *p_name);
/// Callback function called when either Active Preset index or Hearing Aid Features
/// bit field value is received
typedef void (*hac_cb_value)(uint8_t con_lid, uint8_t char_type, uint8_t value, uint16_t err_code);
/// Callback function called when:
/// - A new preset record is added
/// - The name of an existing preset record has changed
/// - More than one change happened during disconnection
typedef void (*hac_cb_generic_update)(uint8_t con_lid, bool last, uint8_t prev_preset_idx, uint8_t preset_idx,
                                      bool writable, bool available, uint8_t length, const uint8_t *p_name);
/// Callback function called when:
/// - A preset record has been deleted
/// - A preset record has become available
/// - A preset record has become unavailable
typedef void (*hac_cb_update)(uint8_t con_lid, uint8_t change_id, bool last, uint8_t preset_idx);
/// Callback function called when Write CP is complete
typedef void (*hac_cb_hap_ctrl_cmp)(uint8_t con_lid, uint8_t opcode, uint16_t err_code);
/// Callback for mcc prf event
typedef gatt_prf_status_event_callback hac_cb_prf_status_evt;

/*CALLBACK*/

/// Set of callback functions for Hearing Access Service Client module communication with upper layer
typedef struct hac_cb
{
    /// Callback function called when Hearing Access Service has been discovered in a Service device database
    hac_cb_bond_data cb_bond_data;
    /// Callback function called when Hearing Access Service has been discovered cmp
    hac_cb_discovery_cmp cb_discovery_cmp;
    /// Callback for hac gatt set cfg cmp evt
    hac_cb_set_cfg_cmp cb_set_cfg_cmp;
    /// Callback function called when a Preset Record is received
    hac_cb_preset cb_preset;
    /// Callback function called when either Active Preset index or Hearing Aid Features bit field value is received
    hac_cb_value cb_value;
    /// Callback function called when a generic update of Preset record(s) has been received
    hac_cb_generic_update cb_generic_update;
    /// Callback function called when an update of Preset record has been received
    hac_cb_update cb_update;
    /// Callback function called when Write CP is complete
    hac_cb_hap_ctrl_cmp cb_hap_ctrl_cmp;
    /// Callback function called when prf status event generated
    hac_cb_prf_status_evt cb_prf_status_event;
} hac_evt_cb_t;

typedef struct hac_init_cfg
{
    /// Preferred MTU size
    uint16_t pref_mtu;
} hac_init_cfg_t;

struct hac_prf_svc_info
{
    /// Service handler range
    gatt_prf_svc_range_t svc_range;
    /// Service UUID
    uint16_t uuid;
};

/**
 * @brief Hearing Access Profile Client initilization
 *
 * @param  p_init_cfg  Initilization configuration
 * @param  p_evt_cb    Event callbacks
 *
 * @return int         status
 */
int hac_init(const hac_init_cfg_t *p_init_cfg, const hac_evt_cb_t *p_evt_cb);

/**
 * @brief Hearing Access Profile Client deinitlization
 *
 * @return int         status
 */
int hac_deinit(void);

/**
 * @brief Hearing Access Profile Client has service discovery
 *
 * @param  con_lid     Connection local index
 *
 * @return int         status
 */
int hac_service_discovery(uint8_t con_lid);

/**
 * @brief Hearing Access Profile Client read Character value
 *
 * @param  con_lid     Connection local index
 * @param  char_type   Character type
 *
 * @return int         status
 */
int hac_character_value_read(uint8_t con_lid, uint8_t char_type);

/**
 * @brief Hearing Access Profile Client write Character cccd
 *
 * @param  con_lid     Connection local index
 * @param  char_type   Character type
 * @param  enable_ntf  Enable or disable notify
 *
 * @return int         status
 */
int hac_character_cccd_write(uint8_t con_lid, uint8_t char_type, bool enable_ntf);

/**
 * @brief Hearing Access Profile Client set preset name
 *
 * @param  con_lid     Connection local index
 * @param  preset_idx  Preset index
 * @param  length      Name to write length
 * @param  p_name      Name value to write pointer
 *
 * @return int         status
 */
int hac_hap_ctrl_set_preset_name(uint8_t con_lid, uint8_t preset_idx, uint8_t length, const uint8_t *p_name);

/**
 * @brief Hearing Access Profile Client set active preset index
 *
 * @param  con_lid     Connection local index
 * @param  opcode      Operation code @see enum has_cp_opcode
 * @param  preset_idx  Preset index
 *
 * @return int         status
 */
int hac_hap_ctrl_set_active_preset(uint8_t con_lid, uint8_t opcode, uint8_t preset_idx);

/**
 * @brief Hearing Access Profile Client get preset informations
 *
 * @param  con_lid     Connection local index
 * @param  start_idx   Preset Read start index
 * @param  num_presets Read Presets num
 *
 * @return int         status
 */
int hac_hap_ctrl_get_preset(uint8_t con_lid, uint8_t start_idx, uint8_t num_presets);
#endif /// __HAP_HAC_INC__