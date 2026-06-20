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
 * @addtogroup APP_GMAP_GMAC
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#if BLE_AUDIO_ENABLED
#include "app_gaf.h"
#include "app_gmap.h"
#include "ble_app_dbg.h"
#include "app_gaf_custom_api.h"

#include "gmac.h"

/*FUNCTIONS*/

/// Callback for gmac bond data
static void app_gmap_gmac_cb_bond_data_evt(uint8_t con_lid, const gmac_prf_svc_t *param)
{
    LOG_D("app_gmap_gmac bond_data shdl = %04x, ehdl = %04x",
          param->svc_range.shdl, param->svc_range.ehdl);
}

/// Callback for gmac discovery done
static void app_gmap_gmac_cb_discovery_cmp_evt(uint8_t con_lid, uint16_t err_code)
{
    LOG_I("app_gmap gmac discovery cmp, con_lid = %d, err_code = %d",
          con_lid, err_code);
}

/// Callback for gmac cccd cfg value received or read
static void app_gmap_gmac_cb_char_value_evt(uint8_t con_lid, uint8_t char_type, uint8_t value, uint16_t err_code)
{
    LOG_I("app_gmap_gmac_value_ind con_lid = %d, err_code = %d, char_type = %d, value = 0x%x",
          con_lid, err_code, char_type, value);
}

static void app_gmap_gmac_cb_prf_status_event(uint8_t con_lid, bool is_central, gatt_prf_status_event_e event)
{
    // GMAP gmas may init one both side
    if (event == GATT_PRF_STATUS_EVENT_OPENED)
    {
        //gmac_service_discovery(con_lid);
    }
}

static const gmac_evt_cb_t app_gmap_gmac_callbacks =
{
    .cb_bond_data = app_gmap_gmac_cb_bond_data_evt,
    .cb_discovery_cmp = app_gmap_gmac_cb_discovery_cmp_evt,
    .cb_char_value = app_gmap_gmac_cb_char_value_evt,
    .cb_prf_status_event = app_gmap_gmac_cb_prf_status_event,
};

int app_gmap_gmac_init(void)
{
    LOG_I("%s", __func__);

    const gmac_init_cfg_t init_cfg =
    {

    };

    return gmac_init(&init_cfg, &app_gmap_gmac_callbacks);
}

int app_gmap_gmac_deinit(void)
{
    LOG_I("%s", __func__);

    return gmac_deinit();
}

int app_gmap_gmac_start(uint8_t con_lid)
{
    return gmac_service_discovery(con_lid);
}

#endif
