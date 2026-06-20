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

#include "app_arc_mics_msg.h"

#include "mics.h"

/*FUNCTIONS*/

/// Callback for mics bond data
void app_arc_mics_cb_cb_bond_data(uint8_t con_lid, uint8_t cli_cfg_bf)
{
    LOG_D("app_arc mics bond_data_ind con_lid = %d, cli_cfg_bf = %02x", con_lid, cli_cfg_bf);
}

/// Callback for mics mute
void app_arc_mics_cb_cb_mute(uint8_t con_lid, uint8_t mute)
{
    LOG_D("app_arc mics mute con_lid = %d, mute val = %d", con_lid, mute);

    app_gaf_arc_mics_mute_ind_t mute_ind =
    {
        .mute = mute,
    };

    app_gaf_evt_report(APP_GAF_MICS_MUTE_IND, (void *)&mute_ind, sizeof(mute_ind));
}

static const mics_evt_cb_t app_arc_mics_evt_cb =
{
    .cb_bond_data = app_arc_mics_cb_cb_bond_data,
    .cb_mute = app_arc_mics_cb_cb_mute,
};

int app_arc_mics_init(void)
{
    LOG_I("%s", __func__);

    mics_init_cfg_t mics_init_cfg =
    {
        .pref_mtu = GAF_PREFERRED_MTU,
        .mute = MICS_MUTE_VALUE_NOT_MUTED,
        .aics_inst_num_inc = 0,
    };

    return mics_init(&mics_init_cfg, &app_arc_mics_evt_cb);
}

int app_arc_mics_deinit(void)
{
    LOG_I("%s", __func__);

    return mics_deinit();
}

int app_arc_mics_set_mute(uint8_t mute)
{
    return mics_set_mute(mute);
}

int app_arc_mics_rsp_handler(void const *param)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_arc_mics_ind_handler(void const *param)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

#endif

/// @} APP

