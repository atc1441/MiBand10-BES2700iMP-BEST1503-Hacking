/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
 * @addtogroup APP_TMAP_TMAC
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#if BLE_AUDIO_ENABLED
#include "app_gaf.h"
#include "ble_app_dbg.h"
#include "app_gaf_custom_api.h"

#include "app_tmap_tmac_msg.h"

#include "tmac.h"

/*FUNCTIONS*/

/// Callback for tmac bond data
static void app_tmap_tmac_cb_bond_data_evt(uint8_t con_lid, const tmac_prf_svc_t *param)
{
    LOG_D("app_tmap_tmac bond_data shdl = %04x, ehdl = %04x",
          param->svc_range.shdl, param->svc_range.ehdl);
}

/// Callback for tmac discovery done
static void app_tmap_tmac_cb_discovery_cmp_evt(uint8_t con_lid, uint16_t err_code)
{
    LOG_I("app_tmap tmac discovery cmp, con_lid = %d, err_code = %d",
          con_lid, err_code);
}

/// Callback for tmac cccd cfg value received or read
static void app_tmap_tmac_cb_role_value_evt(uint8_t con_lid, uint16_t role_bf, uint16_t err_code)
{
    LOG_I("app_tmap_tmac_role_ind con_lid = %d, err_code = %d, role_bf = 0x%x",
          con_lid, err_code, role_bf);
}

static void app_tmap_tmac_cb_prf_status_event(uint8_t con_lid, bool is_central, gatt_prf_status_event_e event)
{
    // TMAP tmas may init one both side
    if (event == GATT_PRF_STATUS_EVENT_OPENED)
    {
        //tmac_service_discovery(con_lid);
    }
}

static tmac_evt_cb_t app_tmap_tmac_cb =
{
    .cb_bond_data = app_tmap_tmac_cb_bond_data_evt,
    .cb_discovery_cmp = app_tmap_tmac_cb_discovery_cmp_evt,
    .cb_role_value = app_tmap_tmac_cb_role_value_evt,
    .cb_prf_status_event = app_tmap_tmac_cb_prf_status_event,
};

int app_tmap_tmac_init(void)
{
    LOG_I("%s", __func__);
    tmac_init_cfg_t tmac_init_cfg;

    return tmac_init(&tmac_init_cfg, &app_tmap_tmac_cb);
}

int app_tmap_tmac_deinit(void)
{
    LOG_I("%s", __func__);

    return tmac_deinit();
}

int app_tmap_tmac_start(uint8_t con_lid)
{
    LOG_I("app_tmap tmac start, con_lid = %d", con_lid);
    return tmac_service_discovery(con_lid);
}

#endif
