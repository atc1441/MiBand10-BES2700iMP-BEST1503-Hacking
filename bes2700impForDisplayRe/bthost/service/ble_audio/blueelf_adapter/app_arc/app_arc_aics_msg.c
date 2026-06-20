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
#include "app_arc_aics_msg.h"

#include "aics.h"

/*CALLBACKS*/

/// Callback for aics bond data
void app_arc_aics_cb_bond_data(uint8_t con_lid, uint8_t input_lid, uint8_t char_type, uint8_t cli_cfg_bf)
{
    LOG_D("app_arc aics bond_data_ind input_lid= %d, con_lid = %d, cli_cfg_bf = %02x",
          input_lid, con_lid, cli_cfg_bf);
}

/// Callback for aics audio input state
void app_arc_aics_cb_input_state(uint8_t input_lid, aics_state_t *p_input_state)
{
    app_gaf_arc_aics_state_ind_t state_ind =
    {
        .gain = p_input_state->gain,
        .gain_mode = p_input_state->gain_mode,
        .input_lid = input_lid,
        .mute = p_input_state->mute,
    };

    LOG_D("app_arc aics_state_ind input_lid= %d, gain = %d, gain_mode = %d, mute = %d",
          state_ind.input_lid, state_ind.gain, state_ind.gain_mode, state_ind.mute);

    app_gaf_evt_report(APP_GAF_AICS_STATE_IND, (void *)&state_ind, sizeof(state_ind));
}

/// Callback for aics description write req
void app_arc_aics_cb_description_wr_req(uint8_t con_lid, uint8_t input_lid, uint8_t desc_val_len, const uint8_t *p_desc_val)
{
    LOG_I("app_arc aics_description set val:");
    DUMP8("%c", p_desc_val, desc_val_len);

    aics_set_description_cfm(BT_STS_SUCCESS, con_lid, input_lid, desc_val_len, p_desc_val);
}

static const aics_evt_cb_t app_arc_aics_cb =
{
    .cb_bond_data = app_arc_aics_cb_bond_data,
    .cb_desc_req = app_arc_aics_cb_description_wr_req,
    .cb_state = app_arc_aics_cb_input_state,
};

int app_arc_aics_init(void)
{
    LOG_I("%s", __func__);

    aics_init_cfg_t aics_init_cfg =
    {
        .aics_inst_supp_max = APP_ARC_AICS_DFT_NB_INPUTS,
    };

    uint16_t status =  aics_init(&aics_init_cfg, &app_arc_aics_cb);

    if (status != BT_STS_SUCCESS)
    {
        return status;
    }

    uint8_t input_lid = 0;

    for (input_lid = 0; input_lid < APP_ARC_AICS_DFT_NB_INPUTS; input_lid++)
    {
        uint8_t input_lid_ret = 0;

        aics_inst_cfg_t aics_instant_cfg =
        {
            .pref_mtu = GAF_PREFERRED_MTU,
            .desc_max_len = APP_ARC_AICS_DESC_MAX_LEN,
            .gain = 0,
            .gain_min = APP_ARC_AICS_DFT_GAIN_MIN,
            .gain_max = APP_ARC_AICS_DFT_GAIN_MAX,
            .gain_units = APP_ARC_AICS_DFT_GAIN_UNITS,
            .gain_mode = 0,
            .input_type = 0,
            .mute = 0,
        };

        status = aics_add_aics_instant(&aics_instant_cfg, &input_lid_ret);

        if (status != BT_STS_SUCCESS)
        {
            LOG_E("arc_aics add instant failed");
            return status;
        }

        LOG_I("arc aics add instant lid = %d", input_lid_ret);

        aics_set_value(input_lid_ret, AICS_SET_TYPE_INPUT_DESC, (const uint8_t *)APP_ARC_DESC_STR, sizeof(APP_ARC_DESC_STR));
    }

    return BT_STS_SUCCESS;
}

int app_arc_aics_deinit(void)
{
    LOG_I("%s", __func__);
    return aics_deinit();
}

int app_arc_aics_set(uint8_t input_lid, uint8_t set_type, uint32_t value)
{
    return aics_set_value(input_lid, set_type, (const uint8_t *)&value, sizeof(uint32_t));
}

int app_arc_aics_set_desc(uint8_t input_lid, uint8_t *desc, uint8_t desc_len)
{
    return aics_set_value(input_lid, AICS_SET_TYPE_INPUT_DESC, desc, desc_len);
}

#endif

/// @} APP

