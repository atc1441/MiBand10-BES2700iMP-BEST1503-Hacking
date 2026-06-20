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
#ifndef __ARC_MICC__
#define __ARC_MICC__
#include "bluetooth.h"

#include "gaf_cfg.h"
#include "generic_audio.h"
#include "gaf_log.h"

#include "gaf_prf.h"

#include "mic_common.h"

#define MICC_CONNECTION_MAX             (GAF_CONNECTION_MAX)

typedef struct micc_prf_svc_info micc_prf_svc_t;

/// Callback for micc bond data
typedef void (*micc_cb_bond_data_evt)(uint8_t con_lid, const micc_prf_svc_t *param);
/// Callback for mics include service discovery done
typedef void (*micc_cb_inc_service)(uint8_t con_lid, const gatt_prf_svc_range_t *p_srvc_rang,
                                    uint16_t uuid, uint8_t input_lid);
/// Callback for micc discovery done
typedef void (*micc_cb_discovery_cmp_evt)(uint8_t con_lid, uint16_t err_code);
/// Callback for micc gatt set cfg cmp evt
typedef void (*micc_cb_set_cfg_cmp_evt)(uint8_t con_lid, uint16_t err_code);
/// Callback for micc mute value received or read
typedef void (*micc_cb_mute_value_evt)(uint8_t con_lid, uint8_t mute, uint16_t err_code);
/// Callback for micc set sink/src audio location cmp evt
typedef void (*micc_cb_set_mute_cmp_evt)(uint8_t con_lid, uint16_t err_code);
// Callback function called when prf status event generated
typedef gatt_prf_status_event_callback micc_cb_prf_status_evt;

/*Structure*/
typedef struct micc_evt_cb
{
    /// Callback function called when client configuration for pacs has been updated
    micc_cb_bond_data_evt cb_bond_data;
    /// Callback for mics include service discovery done
    micc_cb_inc_service cb_inc_service;
    /// Callback function called when pacs is discovered or failed
    micc_cb_discovery_cmp_evt cb_discovery_cmp;
    /// Callback for micc gatt cmd complete
    micc_cb_set_cfg_cmp_evt cb_set_cfg_cmp;
    /// Callback function called when a value of mute is received
    micc_cb_mute_value_evt cb_mute_value;
    /// Callback function called when set mute cmd is cmp
    micc_cb_set_mute_cmp_evt cb_set_mute_cmp;
    /// Callback function called when prf status event generated
    micc_cb_prf_status_evt cb_prf_status_event;
} micc_evt_cb_t;

struct micc_prf_svc_info
{
    /// Service handler range
    gatt_prf_svc_range_t svc_range;
    /// Service UUID
    uint16_t uuid;
};

/*FUCNTIONS DECLARATION*/
/**
 * @brief Microphone input control client initilization
 *
 * @param  p_cb        Event callbacks
 *
 * @return int         status
 */
int micc_init(const micc_evt_cb_t *p_cb);

/**
 * @brief Microphone input control client deinitilization
 *
 * @return int         status
 */
int micc_deinit(void);

/**
 * @brief Microphone input control client MICS service discovery
 *
 * @param  con_lid     Connection local index
 *
 * @return int         status
 */
int micc_service_discovery(uint8_t con_lid);

/**
 * @brief Microphone input control client write mute Character cccd
 *
 * @param  con_lid     Connection local index
 * @param  enable_ntf  Enable or disable notify
 *
 * @return int         status
 */
int micc_mute_cccd_write(uint8_t con_lid, bool enable_ntf);

/**
 * @brief Microphone input control client read mute character value
 *
 * @param  con_lid     Connection local index
 *
 * @return int         status
 */
int micc_mute_value_read(uint8_t con_lid);

/**
 * @brief Microphone input control client control mute
 *
 * @param  con_lid     Connection local index
 * @param  mute        Muted or unmuted
 *
 * @return int         status
 */
int micc_mute_control(uint8_t con_lid, bool mute);
#endif /// __ARC_MICC__