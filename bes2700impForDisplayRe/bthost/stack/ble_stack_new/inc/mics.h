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
#ifndef __ARC_MICS__
#define __ARC_MICS__
#include "bluetooth.h"

#include "gaf_cfg.h"
#include "generic_audio.h"
#include "gaf_log.h"

#include "mic_common.h"

#define MICS_CONNECTION_MAX             (GAF_CONNECTION_MAX)

/// Callback for mics bond data
typedef void (*mics_cb_bond_data)(uint8_t con_lid, uint8_t cli_cfg_bf);
/// Callback for mics mute
typedef void (*mics_cb_mute)(uint8_t con_lid, uint8_t mute);

/*Structure*/
typedef struct mics_evt_cb
{
    /// Callback function called when client configuration for MICS has been updated
    mics_cb_bond_data cb_bond_data;
    /// Callback function called when Mute characteristic has been written by a
    /// client device
    mics_cb_mute cb_mute;
} mics_evt_cb_t;

typedef struct mics_init_cfg
{
    /// Preferred MTU size
    uint16_t pref_mtu;
    /// Mute
    uint8_t mute;
    /// AICS inst included by MICS number
    uint8_t aics_inst_num_inc;
    /// AICS inst included by MICS input lid list
    const uint8_t *p_input_lid_list;
} mics_init_cfg_t;

/*FUNCTIONS DECLARATION*/
/**
 * @brief Microphone input control server initilization
 *
 * @param  mics_init_cfg
 *                     Initilization configuration
 * @param  mics_evt_cb Event callbacks
 *
 * @return int         status
 */
int mics_init(const mics_init_cfg_t *mics_init_cfg, const mics_evt_cb_t *mics_evt_cb);

/**
 * @brief Microphone input control deinitilization
 *
 * @return int         status
 */
int mics_deinit(void);

#if (GAF_USE_CACHE_GATT_CCCD)
/**
 * @brief Microphone input control restore clinet configuration bitfiled
 *
 * @param  con_lid     Connection local index
 * @param  cli_cfg_bf  Client configuration bitfiled
 *
 * @return int         status
 */
int mics_restore_cli_cfg_cache(uint8_t con_lid, uint8_t cli_cfg_bf);
#endif /// (GAF_USE_CACHE_GATT_CCCD)

/**
 * @brief Microphone input control set mute
 *
 * @param  mute        Muted or not muted
 *
 * @return int         status
 */
int mics_set_mute(uint8_t mute);
#endif /// __ARC_MICS__