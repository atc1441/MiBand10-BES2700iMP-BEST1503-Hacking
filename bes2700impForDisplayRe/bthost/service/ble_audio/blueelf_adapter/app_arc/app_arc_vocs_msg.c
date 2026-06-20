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
#include "app_gaf.h"
#include "ble_app_dbg.h"
#include "app_gaf_custom_api.h"

#include "app_arc.h"
#include "app_arc_vocs_msg.h"

#include "vocs.h"

/*CALLBACKS*/

/// Callback function called when Client Characteristic Configuration of a notification-capable
/// characteristic has been updated by a peer client
void app_arc_vocs_cb_bond_data(uint8_t con_lid, uint8_t output_lid, uint8_t char_type, uint8_t cli_cfg_bf)
{
    app_gaf_arc_vocs_cfg_ind_t bond_ind =
    {
        .con_lid = con_lid,
        .output_lid = output_lid,
        .cli_cfg_bf = cli_cfg_bf,
    };

    LOG_D("app_arc vocs bond_data_ind output_lid= %d, con_lid = %d, cli_cfg_bf = %02x",
          bond_ind.output_lid, bond_ind.con_lid, bond_ind.cli_cfg_bf);

    app_gaf_evt_report(APP_GAF_VOCS_BOND_DATA_IND, (void *)&bond_ind, sizeof(bond_ind));
}

/// Callback function called when Offset State characteristic value has been updated
void app_arc_vocs_cb_offset(uint8_t output_lid, int16_t offset)
{
    app_gaf_arc_vocs_offset_ind_t offset_ind =
    {
        .offset = offset,
        .output_lid = output_lid,
    };

    LOG_D("app_arc vocs_offset_ind output_lid= %d, offset = %d", offset_ind.output_lid, offset_ind.offset);

    app_gaf_evt_report(APP_GAF_VOCS_OFFSET_IND, (void *)&offset_ind, sizeof(offset_ind));
}

/// Callback function called when value of Audio Location characteristic has been written so that it can be
/// confirmed by upper layers
void app_arc_vocs_cb_location(uint8_t con_lid, uint8_t output_lid, uint32_t location_bf)
{
    app_gaf_arc_vocs_set_location_req_ind_t set_loc_req =
    {
        .location = location_bf,
        .con_lid = con_lid,
        .output_lid = output_lid,
    };

    app_gaf_evt_report(APP_GAF_VOCS_LOCATION_SET_RI, (void *)&set_loc_req, sizeof(set_loc_req));
}

/// Callback function called when value of Audio Output Description characteristic has been written so that it
/// can be confirmed by upper layers
void app_arc_vocs_cb_description_req(uint8_t con_lid, uint8_t output_lid, uint8_t desc_val_len, const uint8_t *p_desc_val)
{
    vocs_set_description_cfm(BT_STS_SUCCESS, con_lid, output_lid, desc_val_len, p_desc_val);
}

static const vocs_evt_cb_t app_arc_vocs_cb =
{
    .cb_bond_data = app_arc_vocs_cb_bond_data,
    .cb_offset = app_arc_vocs_cb_offset,
    .cb_location = app_arc_vocs_cb_location,
    .cb_desc_req = app_arc_vocs_cb_description_req,
};

int app_arc_vocs_init(void)
{
    LOG_I("%s", __func__);

    vocs_init_cfg_t vocs_init_cfg =
    {
        .vocs_inst_supp_max = APP_ARC_VOCS_DFT_NB_OUTPUTS,
    };

    uint16_t status = vocs_init(&vocs_init_cfg, &app_arc_vocs_cb);

    if (status != BT_STS_SUCCESS)
    {
        return status;
    }

    uint8_t output_lid = 0;

    for (output_lid = 0; output_lid < APP_ARC_VOCS_DFT_NB_OUTPUTS; output_lid++)
    {
        uint8_t output_lid_ret = 0;

        vocs_inst_cfg_t vocs_instant_cfg =
        {
            .pref_mtu = GAF_PREFERRED_MTU,
            .desc_max_len = APP_ARC_VOCS_DESC_MAX_LEN,
            .location_bf = 0,
            .offset = 0,
        };

        status = vocs_add_vocs_instant(&vocs_instant_cfg, &output_lid_ret);

        if (status != BT_STS_SUCCESS)
        {
            LOG_E("arc_vocs add instant failed");
            return status;
        }

        LOG_I("arc vocs add instant lid = %d", output_lid_ret);

        vocs_set_value(output_lid_ret, VOCS_CHAR_TYPE_DESC,
                       (const uint8_t *)APP_ARC_DESC_STR, sizeof(APP_ARC_DESC_STR));
    }

    return BT_STS_SUCCESS;
}

int app_arc_vocs_deinit(void)
{
    LOG_I("%s", __func__);
    return vocs_deinit();
}

int app_arc_vocs_set(uint8_t output_lid, uint8_t set_type, uint32_t value)
{
    uint8_t char_type = set_type;

    return vocs_set_value(output_lid, char_type,
                         (const uint8_t *)&value, sizeof(uint32_t));
}

int app_arc_vocs_set_desc(uint8_t output_lid, uint8_t *desc, uint8_t desc_len)
{
    return vocs_set_value(output_lid, VOCS_CHAR_TYPE_DESC,
                          (const uint8_t *)desc, desc_len);
}

#endif

/// @} APP

