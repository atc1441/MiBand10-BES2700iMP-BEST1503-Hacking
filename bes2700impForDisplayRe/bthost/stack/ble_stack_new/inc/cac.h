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
#ifndef __GAF_CAC_INC__
#define __GAF_CAC_INC__

#include "gaf_cfg.h"
#include "gaf_prf.h"
#include "gaf_log.h"

#define CAC_CONNECTION_MAX                 (GAF_CONNECTION_MAX)

typedef struct cac_prf_svc_info cac_prf_svc_t;

/// Callback for cac bond data
typedef void (*cac_cb_bond_data_evt)(uint8_t con_lid, const cac_prf_svc_t *param);
/// Callback for cac discovery done
typedef void (*cac_cb_discovery_cmp_evt)(uint8_t con_lid, uint16_t err_code);
/// Callback for cac include service discovery done
typedef void (*cac_cb_inc_service)(uint8_t con_lid, const gatt_prf_svc_range_t *p_srvc_rang,
                                   uint16_t uuid);
/// Callback function called when prf status event generated
typedef gatt_prf_status_event_callback cac_cb_prf_status_evt;

typedef struct
{
    /// Callback for cac bond data
    cac_cb_bond_data_evt cb_bond_data;
    /// Callback for cac discovery done
    cac_cb_discovery_cmp_evt cb_svc_discovery_cmp;
    /// Callback for cac include service discovery done
    cac_cb_inc_service cb_inc_svc_discovery_cmp;
    /// Callback function called when prf status event generated
    cac_cb_prf_status_evt cb_prf_status_event;
} cac_evt_cb_t;

struct cac_prf_svc_info
{
    /// Service handler range
    gatt_prf_svc_range_t svc_range;
    /// Service UUID
    uint16_t uuid;
};

/**
 * @brief CAC initilization
 *
 * @param  p_cb        Event callbacks
 *
 * @return int         status
 */
int cac_init(const cac_evt_cb_t *p_cb);

/**
 * @brief CAC deinitilization
 *
 * @return int         status
 */
int cac_deinit(void);

/**
 * @brief CAC service discovery
 *
 * @param  con_lid     Connection local index
 *
 * @return int         status
 */
int cac_service_discovery(uint8_t con_lid);

#endif /// __GAF_CAC_INC__