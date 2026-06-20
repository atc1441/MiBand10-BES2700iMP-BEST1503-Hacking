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
#ifndef __TMAP_TMAC_INC__
#define __TMAP_TMAC_INC__
#include "bluetooth.h"

#include "generic_audio.h"
#include "gaf_prf.h"
#include "gaf_cfg.h"
#include "gaf_log.h"

#include "tma_common.h"

#define TMAC_CONNECTION_MAX                     (GAF_CONNECTION_MAX)

typedef struct tmac_prf_svc_info tmac_prf_svc_t;

/// Callback for tmac bond data
typedef void (*tmac_cb_bond_data_evt)(uint8_t con_lid, const tmac_prf_svc_t *param);
/// Callback for tmac discovery done
typedef void (*tmac_cb_discovery_cmp_evt)(uint8_t con_lid, uint16_t err_code);
/// Callback for tmac role char value received or read
typedef void (*tmac_cb_role_value_evt)(uint8_t con_lid, uint16_t role_bf, uint16_t err_code);
/// Callback function called when prf status event generated
typedef gatt_prf_status_event_callback tmac_cb_prf_status_evt;

/*Structure*/
typedef struct tmac_evt_cb
{
    /// Callback function called when client configuration for tmas has been updated
    tmac_cb_bond_data_evt cb_bond_data;
    /// Callback function called when tmas is discovered or failed
    tmac_cb_discovery_cmp_evt cb_discovery_cmp;
    /// Callback function called when a value of Role is received
    tmac_cb_role_value_evt cb_role_value;
    /// Callback function called when prf status event generated
    tmac_cb_prf_status_evt cb_prf_status_event;
} tmac_evt_cb_t;

struct tmac_prf_svc_info
{
    /// Service handler range
    gatt_prf_svc_range_t svc_range;
    /// Service UUID
    uint16_t uuid;
};

typedef struct tmac_init_cfg
{

} tmac_init_cfg_t;

/*FUCNTIONS DECLARATION*/
/**
 * @brief Telephony and Media Audio Client initilization
 *
 * @param  p_init_cfg  Intilization configuration
 * @param  p_cb        Event callbacks
 *
 * @return int         status
 */
int tmac_init(const tmac_init_cfg_t *p_init_cfg, const tmac_evt_cb_t *p_cb);

/**
 * @brief Telephony and Media Audio Client deinitilization
 *
 * @return int         status
 */
int tmac_deinit(void);

/**
 * @brief Telephony and Media Audio Client tmas service discovery
 *
 * @param  con_lid     Connection local index
 *
 * @return int         status
 */
int tmac_service_discovery(uint8_t con_lid);

/**
 * @brief Telephony and Media Audio Client read character role value
 *
 * @param  con_lid     Connection local index
 *
 * @return int         status
 */
int tmac_role_value_read(uint8_t con_lid);

#endif /// __TMAP_TMAC_INC__