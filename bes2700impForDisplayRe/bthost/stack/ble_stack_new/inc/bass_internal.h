
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
#ifndef __BAP_BASS__
#define __BAP_BASS__

#include "bluetooth.h"

#include "gaf_prf.h"
#include "gaf_cfg.h"
#include "generic_audio.h"
#include "gaf_log.h"

#include "bas_common.h"

#define BASS_CONNECTION_MAX         (GAF_CONNECTION_MAX)

#define BASS_MAX_SUPP_RX_STATE      (6)

/*TYPEDEF*/
/// BASS callback for check src id existence
typedef bool (*bass_cb_check_src_id_existence)(uint8_t con_lid, uint8_t rx_src_id);
/// BASS callback for read ase char
typedef void (*bass_cb_get_rx_state_value_buf)(uint8_t con_lid, uint8_t rx_src_id,
                                               const uint8_t **p_rx_state_value_buf, uint16_t *p_buf_len);
/// BASS callback for scan cp write
typedef void (*bass_cb_scan_cp_write)(gatt_server_write_ctx_t *p_wr_ctx, uint8_t con_lid, const uint8_t *char_value_buf, uint16_t buf_len);
/// BASS callback for bond data
typedef void (*bass_cb_bond_data)(uint8_t con_lid, uint8_t char_type, uint8_t cli_cfg_bf);
/// BASS callback for service disconnect
typedef void (*bass_cb_disconnect)(uint8_t con_lid);

typedef struct bass_evt_cb
{
    bass_cb_get_rx_state_value_buf cb_get_rx_state_value_buf;

    bass_cb_scan_cp_write cb_scan_cp_write;

    bass_cb_bond_data cb_bond_data;

    bass_cb_disconnect cb_disconnect;
} bass_evt_cb_t;

typedef struct bass_init_cfg
{
    /// Preferred MTU size
    uint16_t pref_mtu;
    /// Max supported RX state
    uint8_t rx_state_supp_max;
} bass_init_cfg_t;

/*FUNCTIONS*/
#if (GAF_USE_CACHE_GATT_CCCD)
/**
 * @brief BASS restore Client Configuration Bitfield
 *
 * @param  con_lid     Connection local index
 * @param  cli_cfg_bf  Client Configuration Bitfield
 *
 * @return int         status
 */
int bass_restore_cli_cfg_cache(uint8_t con_lid, uint8_t cli_cfg_bf);
#endif /// (GAF_USE_CACHE_GATT_CCCD)

#endif /// __BAP_BASS__