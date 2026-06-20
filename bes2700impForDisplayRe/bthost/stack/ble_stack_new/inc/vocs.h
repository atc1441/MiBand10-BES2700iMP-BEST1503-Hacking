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
#ifndef __ARC_VOCS__
#define __ARC_VOCS__
#include "bluetooth.h"

#include "gaf_cfg.h"
#include "generic_audio.h"
#include "gaf_log.h"

#include "voc_common.h"

#define VOCS_CONNECTION_MAX             (GAF_CONNECTION_MAX)

/// Callback function called when Client Characteristic Configuration of a notification-capable
/// characteristic has been updated by a peer client
typedef void (*vocs_cb_bond_data)(uint8_t con_lid, uint8_t output_lid, uint8_t char_type, uint8_t cli_cfg_bf);
/// Callback function called when Offset State characteristic value has been updated
typedef void (*vocs_cb_offset)(uint8_t output_lid, int16_t offset);
/// Callback function called when value of Audio Location characteristic has been written so that it can be
/// confirmed by upper layers
typedef void (*vocs_cb_location)(uint8_t con_lid, uint8_t output_lid, uint32_t location_bf);
/// Callback function called when value of Audio Output Description characteristic has been written so that it
/// can be confirmed by upper layers
typedef void (*vocs_cb_description_req)(uint8_t con_lid, uint8_t output_lid, uint8_t desc_val_len, const uint8_t *p_desc_val);

/*Structure*/
typedef struct vocs_evt_cb
{
    /// Callback function called when client configuration for VOCS has been updated
    vocs_cb_bond_data cb_bond_data;
    /// Callback function called when Audio input state is changed
    vocs_cb_offset cb_offset;
    /// Callback for vocs location is changed
    vocs_cb_location cb_location;
    /// Callback for vocs description write req
    vocs_cb_description_req cb_desc_req;
} vocs_evt_cb_t;

typedef struct vocs_inst_cfg
{
    /// Preferred MTU size
    uint16_t pref_mtu;
    /// Maximum length of Audio Output Description
    uint8_t desc_max_len;
    /// Audio Location bitfiled
    uint32_t location_bf;
    /// Offset
    int16_t offset;
} vocs_inst_cfg_t;

typedef struct vocs_init_cfg
{
    /// Max supported VOCS inst
    uint8_t vocs_inst_supp_max;
} vocs_init_cfg_t;

/*FUNCTIONS DECLARATION*/
/**
 * @brief Volume offset control server initilization
 *
 * @param  vocs_init_cfg
 *                     Initilization configuration
 * @param  vocs_evt_cb Event callbacks
 *
 * @return int         status
 */
int vocs_init(const vocs_init_cfg_t *vocs_init_cfg, const vocs_evt_cb_t *vocs_evt_cb);

/**
 * @brief Volume offset control server deinitilization
 *
 * @return int         status
 */
int vocs_deinit(void);

/**
 * @brief Volume offset control server add vocs instant
 *
 * @param  p_vocs_inst_cfg
 *                     VOCS instant configuration
 * @param  p_out_lid_ret
 *                     Output local index return
 *
 * @return int         status
 */
int vocs_add_vocs_instant(const vocs_inst_cfg_t *p_vocs_inst_cfg, uint8_t *p_out_lid_ret);

#if (GAF_USE_CACHE_GATT_CCCD)
/**
 * @brief Volume offset control server restore client configuration bitfield
 *
 * @param  con_lid     Connection local inddex
 * @param  output_lid  Output local index
 * @param  cli_cfg_bf  client configuration bitfield
 *
 * @return int         status
 */
int vocs_restore_cli_cfg_cache(uint8_t con_lid, uint8_t output_lid, uint8_t cli_cfg_bf);
#endif /// (GAF_USE_CACHE_GATT_CCCD)

/**
 * @brief Volume offset control server set character value
 *
 * @param  output_lid  Output local index
 * @param  char_type   Character type
 * @param  p_value     Value pointer
 * @param  val_len     Value length
 *
 * @return int         status
 */
int vocs_set_value(uint8_t output_lid, uint8_t char_type, const uint8_t *p_value, uint8_t val_len);

/**
 * @brief Volume offset control server set character output description value
 *
 * @param  cfm_status  Confirm status
 * @param  con_lid     Connection local inddex
 * @param  output_lid  Output local index
 * @param  desc_val_len
 *                     Description value length
 * @param  p_desc_val  Description value pointer
 *
 * @return int         status
 */
int vocs_set_description_cfm(uint16_t cfm_status, uint8_t con_lid, uint8_t output_lid, uint8_t desc_val_len, const uint8_t *p_desc_val);
#endif /// __ARC_VOCS__