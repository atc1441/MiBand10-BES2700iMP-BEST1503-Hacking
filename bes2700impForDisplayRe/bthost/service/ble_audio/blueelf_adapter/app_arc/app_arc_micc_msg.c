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
#include "ble_app_dbg.h"
#include "app_arc_vcc_msg.h"
#include "app_gaf_custom_api.h"

#include "micc.h"

/*FUNCTIONS*/

/// Callback for micc bond data
static void app_arc_micc_cb_cb_bond_data_evt(uint8_t con_lid, const micc_prf_svc_t *param)
{
    LOG_D("app_arc micc service found con_lid = %d, shdl = %04x, ehdl = %04x",
          con_lid, param->svc_range.shdl, param->svc_range.ehdl);
}

static void app_arc_micc_cb_inc_service_evt(uint8_t con_lid,
                                            const gatt_prf_svc_range_t *p_srvc_rang, uint16_t uuid, uint8_t input_lid)
{
    LOG_D("app_arc micc inc service found con_lid = %d, shdl = %04x, ehdl = %04x, input_lid = %d",
          con_lid, p_srvc_rang->shdl, p_srvc_rang->ehdl, input_lid);
}

/// Callback for micc discovery done
static void app_arc_micc_cb_cb_discovery_cmp_evt(uint8_t con_lid, uint16_t err_code)
{
    LOG_D("app_arc micc discover cmp, con_lid = %d, err_code = %d", con_lid, err_code);
}

/// Callback for micc gatt set cfg cmp evt
static void app_arc_micc_cb_cb_set_cfg_cmp_evt(uint8_t con_lid, uint16_t err_code)
{
    LOG_D("app_arc micc set cfg cmp, con_lid = %d, err_code = %d", con_lid, err_code);
}

/// Callback for micc mute value received or read
static void app_arc_micc_cb_cb_mute_value_evt(uint8_t con_lid, uint8_t mute, uint16_t err_code)
{
    app_gaf_arc_micc_mute_ind_t mute_ind =
    {
        .mute = mute,
        .con_lid = con_lid,
    };

    LOG_D("app_arc micc_mute_ind con_lid = %d, mute = %d",
          mute_ind.con_lid, mute_ind.mute);
#ifdef AOB_MOBILE_ENABLED
    app_gaf_mobile_evt_report(APP_GAF_MICC_MUTE_IND, (void *)&mute_ind, sizeof(mute_ind));
#endif
}

/// Callback for micc set sink/src audio location cmp evt
static void app_arc_micc_cb_cb_set_mute_cmp_evt(uint8_t con_lid, uint16_t err_code)
{
    LOG_D("app_arc micc set mute cmp, con_lid = %d, err_code = %d", con_lid, err_code);
}

static void app_arc_micc_cb_prf_status_event(uint8_t con_lid, bool is_central, gatt_prf_status_event_e event)
{
    if (event == GATT_PRF_STATUS_EVENT_OPENED && is_central == true)
    {
        //micc_service_discovery(con_lid);
    }
}

static const micc_evt_cb_t app_arc_micc_evt_cb =
{
    .cb_bond_data = app_arc_micc_cb_cb_bond_data_evt,
    .cb_inc_service = app_arc_micc_cb_inc_service_evt,
    .cb_discovery_cmp = app_arc_micc_cb_cb_discovery_cmp_evt,
    .cb_set_cfg_cmp = app_arc_micc_cb_cb_set_cfg_cmp_evt,
    .cb_mute_value = app_arc_micc_cb_cb_mute_value_evt,
    .cb_set_mute_cmp = app_arc_micc_cb_cb_set_mute_cmp_evt,
    .cb_prf_status_event = app_arc_micc_cb_prf_status_event,
};

int app_arc_micc_init(void)
{
    LOG_I("%s", __func__);
    return micc_init(&app_arc_micc_evt_cb);
}

int app_arc_micc_deinit(void)
{
    LOG_I("%s", __func__);
    return micc_deinit();
}

int app_arc_micc_start(uint8_t con_lid)
{
    LOG_I("%s", __func__);
    return micc_service_discovery(con_lid);
}

int app_arc_micc_set_cfg(uint8_t con_lid, uint8_t enable)
{
    return micc_mute_cccd_write(con_lid, enable);
}

int app_arc_micc_set_mute(uint8_t con_lid, uint8_t mute)
{
    /*micc only has set mute character*/
    return micc_mute_control(con_lid, mute == 0 ? false : true);
}

int app_arc_micc_read_mute(uint8_t con_lid)
{
    /*micc only has set mute character*/
    return micc_mute_value_read(con_lid);
}

#endif
#endif

/// @} APP

