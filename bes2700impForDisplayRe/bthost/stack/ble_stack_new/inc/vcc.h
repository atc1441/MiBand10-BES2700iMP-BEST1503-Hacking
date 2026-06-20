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
#ifndef __ARC_VCC_INT__
#define __ARC_VCC_INT__
#include "bluetooth.h"

#include "gaf_cfg.h"
#include "gaf_prf.h"
#include "gaf_log.h"

#include "vc_common.h"

#define VCC_CONNECTION_MAX                  (GAF_CONNECTION_MAX)

#define VCC_CP_NTF_RX_TIMEOUT_DUR_MS        (gaf_prf_get_rx_ntf_ind_timeout_in_ms())

typedef struct vcc_prf_svc_info vcc_prf_svc_t;

/// Callback for vcc bond data
typedef void (*vcc_cb_bond_data_evt)(uint8_t con_lid, const vcc_prf_svc_t *param);
/// Callback for vcc discovery done
typedef void (*vcc_cb_discovery_cmp_evt)(uint8_t con_lid, uint16_t err_code);
/// Callback for vcc include service found
typedef void (*vcc_cb_include_service_found_evt)(uint8_t con_lid,
                                                 const gatt_prf_svc_range_t *p_srvc_rang, uint16_t uuid, uint8_t lid);
/// Callback for vcc gatt set cfg complete
typedef void (*vcc_cb_set_cfg_cmp_evt)(uint8_t con_lid, uint8_t char_type, uint16_t err_code);
/// Callback for vcc cccd cfg value received or read
typedef void (*vcc_cb_cfg_evt)(uint8_t con_lid, uint8_t char_type, bool enabled, uint16_t err_code);
/// Callback for vcc op control complete
typedef void (*vcc_cb_vcp_control_cmp_evt)(uint8_t con_lid, uint8_t op_code, uint16_t err_code);
/// Callback for vcc vol state ntf or read
typedef void (*vcc_cb_vol_state_evt)(uint8_t con_lid, uint8_t vol_setting, bool mute,
                                     uint16_t err_code);
/// Callback for vcc vol flags or read
typedef void (*vcc_cb_vol_flags_evt)(uint8_t con_lid, bool vol_setting_persisted,
                                     uint16_t err_code);
/// Callback function called when prf status event generated
typedef gatt_prf_status_event_callback vcc_cb_prf_status_evt;

/*Structure*/
typedef struct vcc_evt_cb
{
    /// Callback function called when client configuration for (G)MCS has been updated
    vcc_cb_bond_data_evt cb_bond_data;
    /// Callback function called when include service for (G)MCS has been found
    vcc_cb_include_service_found_evt cb_inc_svc_found;
    /// Callback function called when (G)MCS is discovered or failed
    vcc_cb_discovery_cmp_evt cb_discovery_cmp;
    /// Callback for vcc gatt cmd set cfg complete
    vcc_cb_set_cfg_cmp_evt cb_set_cfg_cmp;
    /// Callback function called when a cfg of cccd is received
    vcc_cb_cfg_evt cb_cfg_value;
    /// Callback function called when a control cmd is complete with status
    vcc_cb_vcp_control_cmp_evt cb_vcp_ctrl_cmp;
    /// Callback function called when a vol state is received with status
    vcc_cb_vol_state_evt cb_vol_state;
    /// Callback function called when a vol flags is received with status
    vcc_cb_vol_flags_evt cb_vol_flags;
    /// Callback function called when prf status event generated
    vcc_cb_prf_status_evt cb_prf_status_event;
} vcc_evt_cb_t;

typedef struct vcc_cfg
{

} vcc_cfg_t;

struct vcc_prf_svc_info
{
    /// Service handler range
    gatt_prf_svc_range_t svc_range;
    /// Service UUID
    uint16_t uuid;
};

/*FUNCTION DECLARATION*/
/**
 * @brief Volume control client initilization
 *
 * @param  p_init_cfg  Initilization configuration
 * @param  p_cb        Event callbacks
 *
 * @return int         status
 */
int vcc_init(const vcc_cfg_t *p_init_cfg, const vcc_evt_cb_t *p_cb);

/**
 * @brief Volume control client deinitilization
 *
 * @return int         status
 */
int vcc_deinit(void);

/**
 * @brief Volume control client VCS service discovery
 *
 * @param  con_lid     Connection local index
 *
 * @return int         status
 */
int vcc_service_discovery(uint8_t con_lid);

/**
 * @brief Volume control client read character value
 *
 * @param  con_lid     Connection local index
 * @param  char_type   Character type
 *
 * @return int         status
 */
int vcc_character_value_read(uint8_t con_lid, uint8_t char_type);

/**
 * @brief Volume control client write character cccd
 *
 * @param  con_lid     Connection local index
 * @param  char_type   Character type
 * @param  enable_ntf  Enable or disable notify
 *
 * @return int         status
 */
int vcc_character_cccd_write(uint8_t con_lid, uint8_t char_type, bool enable_ntf);

/**
 * @brief Volume control client vcp control
 *
 * @param  con_lid     Connection local index
 * @param  op_code     Operation code @see enum vcs_opcode
 * @param  vol_setting Volume setting
 *
 * @return int         status
 */
int vcc_vcp_control(uint8_t con_lid, uint8_t op_code, uint8_t vol_setting);

#endif /// __ARC_VCC_INT__