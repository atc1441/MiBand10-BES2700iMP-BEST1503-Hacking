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
#ifndef __HAP_HAS_INC__
#define __HAP_HAS_INC__
#include "bluetooth.h"

#include "gaf_cfg.h"

#include "ha_common.h"

/*DEFINE*/
#define HAS_CONNECTION_MAX      (GAF_CONNECTION_MAX)

/*ENUM*/

/*TYPEDEF*/

/// Callback for has bond data
typedef void (*has_cb_bond_data)(uint8_t con_lid, uint8_t char_type, uint8_t cli_cfg_ntf_bf, uint8_t cli_cfg_ind_bf);
/// Callback function called when a peer Client device requires to update the current Active Preset
typedef void (*has_cb_set_active_preset_req)(uint8_t con_lid, uint8_t preset_lid, bool sync);
/// Callback function called when a peer Client device has updated name of a Preset
typedef void (*has_cb_set_preset_name_req)(uint8_t con_lid, uint8_t preset_lid,
                                           uint8_t length, const uint8_t *p_name);
/// Callback function called when set active preset local cmp
typedef void (*has_cb_preset_operation_cmp)(uint8_t preset_lid, uint16_t status);

/*Structure*/
typedef struct has_evt_cb
{
    /// Callback function called when client configuration for AICS has been updated
    has_cb_bond_data cb_bond_data;
    /// Callback function called when a peer Client device requires to update the current Active Preset
    has_cb_set_active_preset_req cb_set_active_preset_req;
    /// Callback function called when a peer Client device has updated name of a Preset
    has_cb_set_preset_name_req cb_set_preset_name_req;
    /// Callback function called when set active preset local cmp
    has_cb_preset_operation_cmp cb_set_active_preset_cmp;
} has_evt_cb_t;

typedef struct has_init_cfg
{
    /// Preferred MTU size
    uint16_t pref_mtu;
    /// Max supported Presets
    uint8_t nb_presets_supp;
    /// HAS feature bitfiled
    uint8_t features_bf;
} has_init_cfg_t;

/*FUNCTIONS DECLARATION*/

/**
 * @brief Hearing Access Profile server initlization
 *
 * @param  has_init_cfg
 *                     Initlization configuration
 * @param  has_evt_cb  Event callbacks
 *
 * @return int         status
 */
int has_init(const has_init_cfg_t *has_init_cfg, const has_evt_cb_t *has_evt_cb);

/**
 * @brief Hearing Access Profile server deinitlization
 *
 * @return int         status
 */
int has_deinit(void);

#if (GAF_USE_CACHE_GATT_CCCD)
/**
 * @brief Hearing Access Profile server restore client configuration bitfield
 *
 * @param  con_lid     Connection local index
 * @param  cli_cfg_ntf_bf
 *                     Client Configuration Bitfield for notify
 * @param  cli_cfg_ind_bf
 *                     Client Configuration Bitfield for indication
 *
 * @return int         status
 */
int has_restore_cli_cfg_cache(uint8_t con_lid, uint8_t cli_cfg_ntf_bf, uint8_t cli_cfg_ind_bf);
#endif /// (GAF_USE_CACHE_GATT_CCCD)

/**
 * @brief Hearing Access Profile server add preset instant
 *
 * @param  preset_lid  Preset local index
 * @param  writable    Writeable
 * @param  available   Available
 * @param  length      Length
 * @param  p_name      Name value pointer
 *
 * @return int         status
 */
int has_add_preset(uint8_t preset_lid, bool writable, bool available, uint8_t length, const uint8_t *p_name);

/**
 * @brief Hearing Access Profile server remove preset instant
 *
 * @param  preset_lid  Preset local index
 *
 * @return int         status
 */
int has_remove_preset(uint8_t preset_lid);

/**
 * @brief Hearing Access Profile server set active preset
 *
 * @param  preset_lid  Preset local index
 *
 * @return int         status
 */
int has_set_active_preset(uint8_t preset_lid);

/**
 * @brief Hearing Access Profile server set feature
 *
 * @param  features_bf ...
 *
 * @return int         status
 */
int has_set_features(uint8_t features_bf);

/**
 * @brief Hearing Access Profile server update preset
 *
 * @param  preset_lid  Preset local index
 * @param  available   Avaliable
 * @param  length      Length
 * @param  p_name      Name value pointer
 *
 * @return int         status
 */
int has_update_preset(uint8_t preset_lid, bool available, uint8_t length, const uint8_t *p_name);

/**
 * @brief Hearing Access Profile server confirm set active preset
 *
 * @param  accept      Accept or not
 *
 * @return int         status
 */
int has_set_active_preset_cfm(bool accept);

/**
 * @brief Hearing Access Profile server confirm set preset name
 *
 * @param  accept      Accept or not
 * @param  length      Length
 * @param  p_name      Name value pointer
 *
 * @return int         status
 */
int has_set_preset_name_cfm(bool accept, uint8_t length, const uint8_t *p_name);
#endif /// __HAP_HAS_INC__