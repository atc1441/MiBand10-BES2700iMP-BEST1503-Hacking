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
#include "app_cap_cac_msg.h"

#include "cac.h"

/// Callback for cac bond data
static void app_cac_cb_bond_data_evt(uint8_t con_lid, const cac_prf_svc_t *param)
{
    LOG_D("app_capc bond_data shdl = %04x, ehdl = %04x, uuid = %04x",
          param->svc_range.shdl, param->svc_range.ehdl, param->uuid);
}

/// Callback for cac discovery done
static void app_cac_cb_discovery_cmp_evt(uint8_t con_lid, uint16_t err_code)
{
    LOG_I("app_capc discovery cmp, con_lid = %d, status = %d", con_lid, err_code);
}

/// Callback for cac include service discovery done
static void app_cac_cb_inc_service(uint8_t con_lid, const gatt_prf_svc_range_t *p_srvc_rang, uint16_t uuid)
{
    LOG_I("app_capc include service shdl = %04x, ehdl = %04x, uuid = %04x",
          p_srvc_rang->shdl, p_srvc_rang->ehdl, uuid);
}

static void app_cac_cb_prf_status_event(uint8_t con_lid, bool is_central, gatt_prf_status_event_e event)
{
    if (event == GATT_PRF_STATUS_EVENT_OPENED && is_central == true)
    {
        //cac_service_discovery(con_lid);
    }
}
static const cac_evt_cb_t cac_evt_cb =
{
    .cb_bond_data = app_cac_cb_bond_data_evt,
    .cb_inc_svc_discovery_cmp = app_cac_cb_inc_service,
    .cb_svc_discovery_cmp = app_cac_cb_discovery_cmp_evt,
    .cb_prf_status_event = app_cac_cb_prf_status_event,
};

int app_cap_cac_init(void)
{
    LOG_I("app_cap cac init");
    return cac_init(&cac_evt_cb);
}

int app_cap_cac_deinit(void)
{
    LOG_I("app_cap cac deinit");
    return cac_deinit();
}

int app_cap_cac_start(uint8_t con_lid)
{
    LOG_I("app_capc start, con_lid = %d", con_lid);
    return cac_service_discovery(con_lid);
}

#endif
#endif

/// @} APP

