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
#ifndef __GMAP_GMAC_INC__
#define __GMAP_GMAC_INC__
#include "bluetooth.h"

#include "generic_audio.h"
#include "gaf_prf.h"
#include "gaf_cfg.h"
#include "gaf_log.h"

#include "gma_common.h"

#define GMAC_CONNECTION_MAX                     (GAF_CONNECTION_MAX)

typedef struct gmac_prf_svc_info gmac_prf_svc_t;

/// Callback for gmac bond data
typedef void (*gmac_cb_bond_data_evt)(uint8_t con_lid, const gmac_prf_svc_t *param);
/// Callback for gmac discovery done
typedef void (*gmac_cb_discovery_cmp_evt)(uint8_t con_lid, uint16_t err_code);
/// Callback for gmac role or feat value received or read
typedef void (*gmac_cb_char_value_evt)(uint8_t con_lid, uint8_t char_type, uint8_t value, uint16_t err_code);
/// Callback function called when prf status event generated
typedef gatt_prf_status_event_callback gmac_cb_prf_status_evt;

/*Structure*/
typedef struct gmac_evt_cb
{
    /// Callback function called when client configuration for gmas has been updated
    gmac_cb_bond_data_evt cb_bond_data;
    /// Callback function called when gmas is discovered or failed
    gmac_cb_discovery_cmp_evt cb_discovery_cmp;
    /// Callback function called when a value of Role and feat is received
    gmac_cb_char_value_evt cb_char_value;
    /// Callback function called when prf status event generated
    gmac_cb_prf_status_evt cb_prf_status_event;
} gmac_evt_cb_t;

struct gmac_prf_svc_info
{
    /// Service handler range
    gatt_prf_svc_range_t svc_range;
    /// Service UUID
    uint16_t uuid;
};

typedef struct gmac_init_cfg
{

} gmac_init_cfg_t;

/*FUCNTIONS DECLARATION*/
/**
 * @brief Gaming Audio Client initilization
 *
 * @param  p_init_cfg  Intilization configuration
 * @param  p_cb        Event callbacks
 *
 * @return int         status
 */
int gmac_init(const gmac_init_cfg_t *p_init_cfg, const gmac_evt_cb_t *p_cb);

/**
 * @brief Gaming Audio Client deinitilization
 *
 * @return int         status
 */
int gmac_deinit(void);

/**
 * @brief Gaming Audio Client gmas service discovery
 *
 * @param  con_lid     Connection local index
 *
 * @return int         status
 */
int gmac_service_discovery(uint8_t con_lid);

/**
 * @brief Gaming Audio Client read character role value
 *
 * @param  con_lid     Connection local index
 *
 * @return int         status
 */
int gmac_character_value_read(uint8_t con_lid, uint8_t char_type);

#endif /// __GMAP_GMAC_INC__