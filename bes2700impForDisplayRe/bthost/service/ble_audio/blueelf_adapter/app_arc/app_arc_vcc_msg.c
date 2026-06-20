/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#if BLE_AUDIO_ENABLED
#ifdef AOB_MOBILE_ENABLED
#include "app_gaf.h"
#include "app_arc.h"
#include "ble_app_dbg.h"
#include "app_gaf_custom_api.h"

#include "vcc.h"

/// Callback for vcc bond data
static void app_arc_vcc_cb_bond_data_evt(uint8_t con_lid, const vcc_prf_svc_t *param)
{
    LOG_D("app_arc vcc service found con_lid = %d, shdl = %04x, ehdl = %04x",
          con_lid, param->svc_range.shdl, param->svc_range.ehdl);
}

/// Callback for vcc discovery done
static void app_arc_vcc_cb_discovery_cmp_evt(uint8_t con_lid, uint16_t err_code)
{
    LOG_I("app_arc vcc discovery cmp, con_lid = %d, status = %d", con_lid, err_code);
}

/// Callback for vcc include service found
static void app_arc_vcc_cb_include_service_found_evt(uint8_t con_lid,
                                                     const gatt_prf_svc_range_t *p_srvc_rang, uint16_t uuid, uint8_t lid)
{
    LOG_D("app_arc vcc found included service con_lid = %d, uuid = %04x, shdl = %04x, ehdl = %04x, lid = %d",
          con_lid, uuid, p_srvc_rang->shdl, p_srvc_rang->ehdl, lid);
}

/// Callback for vcc gatt set cfg complete
static void app_arc_vcc_cb_set_cfg_cmp_evt(uint8_t con_lid, uint8_t char_type, uint16_t err_code)
{
    LOG_D("app_arc vcc set cfg cmp, char_type = %d, status = %d", char_type, err_code);
}

/// Callback for vcc cccd cfg value received or read
static void app_arc_vcc_cb_cfg_evt(uint8_t con_lid, uint8_t char_type, bool enabled, uint16_t err_code)
{
    LOG_D("app_arc vcc get cfg cmp, char_type = %d, status = %d, enable_ntf = %d", char_type, err_code,
          enabled);
}

/// Callback for vcc op control complete
static void app_arc_vcc_cb_vcp_control_cmp_evt(uint8_t con_lid, uint8_t op_code, uint16_t err_code)
{
    LOG_D("app_arc vcc control cmp, con_lid = %d, op_code = %d, status = %d", con_lid, op_code,
          err_code);
}

/// Callback for vcc vol state ntf or read
static void app_arc_vcc_cb_vol_state_evt(uint8_t con_lid, uint8_t vol_setting, bool mute,
                                         uint16_t err_code)
{
    LOG_I("app_arc vcc_volume_ind con_lid = %d, volume= %d, mute = %d, status = %d", con_lid,
          vol_setting, mute, err_code);

#ifdef AOB_MOBILE_ENABLED
    app_gaf_vcc_volume_ind_t volume_ind =
    {
        .con_lid = con_lid,
        .mute = mute,
        .volume = vol_setting,
    };

    app_gaf_mobile_evt_report(APP_GAF_VCC_VOLUME_IND, (void *)&volume_ind, sizeof(volume_ind));
#endif
}

/// Callback for vcc vol flags or read
static void app_arc_vcc_cb_vol_flags_evt(uint8_t con_lid, bool vol_setting_persisted, uint16_t err_code)
{
    LOG_D("app_arc vcc_flags_ind con_lid = %d, flags = %d, status = %d", con_lid, vol_setting_persisted,
          err_code);
}

static void app_arc_vcc_cb_prf_status_event(uint8_t con_lid, bool is_central, gatt_prf_status_event_e event)
{
    if (event == GATT_PRF_STATUS_EVENT_OPENED && is_central == true)
    {
        //vcc_service_discovery(con_lid);
    }
}

static const vcc_evt_cb_t vcc_evt_cb =
{
    .cb_bond_data = app_arc_vcc_cb_bond_data_evt,
    .cb_discovery_cmp = app_arc_vcc_cb_discovery_cmp_evt,
    .cb_cfg_value = app_arc_vcc_cb_cfg_evt,
    .cb_inc_svc_found = app_arc_vcc_cb_include_service_found_evt,
    .cb_set_cfg_cmp = app_arc_vcc_cb_set_cfg_cmp_evt,
    .cb_vcp_ctrl_cmp = app_arc_vcc_cb_vcp_control_cmp_evt,
    .cb_vol_flags = app_arc_vcc_cb_vol_flags_evt,
    .cb_vol_state = app_arc_vcc_cb_vol_state_evt,
    .cb_prf_status_event = app_arc_vcc_cb_prf_status_event,
};

int app_arc_vcc_init(void)
{
    LOG_I("app_arc vcc init");

    vcc_cfg_t vcc_init_cfg;

    return vcc_init(&vcc_init_cfg, &vcc_evt_cb);
}

int app_arc_vcc_deinit(void)
{
    LOG_I("app_arc vcc deinit");

    return vcc_deinit();
}

int app_arc_vcc_start(uint8_t con_lid)
{
    LOG_I("app_arc vcc start, con_lid = %d", con_lid);
    return vcc_service_discovery(con_lid);
}

int app_arc_vcc_control(uint8_t con_lid, uint8_t opcode, uint8_t volume)
{
    return vcc_vcp_control(con_lid, opcode, volume);
}

#endif
#endif

/// @} APP

