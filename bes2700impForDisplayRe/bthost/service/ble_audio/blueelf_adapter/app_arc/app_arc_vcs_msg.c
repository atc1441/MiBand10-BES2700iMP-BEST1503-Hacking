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
#include "app_arc.h"
#include "app_gaf_custom_api.h"

#include "app_arc_vcs_msg.h"

#include "vcs.h"

static void app_arc_vcs_cb_bond_data(uint8_t con_lid, uint8_t char_type, uint8_t cli_cfg_bf)
{
    LOG_D("app_arc vcs bond_data_ind con_lid = %d, cli_cfg_bf = %02x", con_lid, cli_cfg_bf);

    app_gaf_arc_vcs_bond_data_ind_t bond_data =
    {
        .char_type = char_type,
        .cli_cfg_bf = cli_cfg_bf,
        .con_lid = con_lid,
    };

    app_gaf_evt_report(APP_GAF_VCS_BOND_DATA_IND, (void *)&bond_data, sizeof(bond_data));
}

static void app_arc_vcs_cb_vol_state_changed(uint8_t volume, uint8_t mute, uint8_t change_cnt,
                                             bool is_local, uint8_t con_lid)
{
    LOG_D("app_arc vcs volume state con_lid = %d, vol = %d, mute = %02x, reason = %d, change_cnt = %d",
          con_lid, volume, mute, is_local, change_cnt);

    app_gaf_arc_vcs_volume_ind_t volume_state =
    {
        .con_lid = con_lid,
        .volume = volume,
        .mute = mute,
        .reason = is_local,
    };

    app_gaf_evt_report(APP_GAF_VCS_VOLUME_IND, (void *)&volume_state, sizeof(volume_state));
}

static void app_arc_vcs_cb_vol_flags_changed(uint8_t con_lid, uint8_t flags)
{
    LOG_D("app_arc vcs_flags_ind = %d", flags == VCS_USER_SET_VOL_SETTING);

    app_gaf_arc_vcs_flags_ind_t flags_evt =
    {
        .con_lid = con_lid,
        .flags = flags,
    };

    app_gaf_evt_report(APP_GAF_VCS_FLAGS_IND, (void *)&flags_evt, sizeof(flags_evt));
}

static const vcs_evt_cb_t vcs_evt_cb =
{
    .cb_bond_data = app_arc_vcs_cb_bond_data,
    .cb_vol_state_change = app_arc_vcs_cb_vol_state_changed,
    .cb_vol_flags_change = app_arc_vcs_cb_vol_flags_changed,
};

int app_arc_vcs_init(void)
{
    LOG_I("app_arc vcs init start");

    vcs_cfg_t vcs_init_cfg =
    {
        .pref_mtu = GAF_PREFERRED_MTU,
        .include_aics_num = 0,
        .include_vocs_num = 0,
        .vol_step = APP_ARC_VCS_DFT_STEP_SIZE,
    };

    return vcs_init(&vcs_init_cfg, &vcs_evt_cb);
}

int app_arc_vcs_deinit(void)
{
    LOG_I("app_arc vcs deinit start");

    return vcs_deinit();
}

int app_arc_vcs_control(uint8_t opcode, uint8_t volume, bool no_changed_cb)
{
    return BT_STS_NOT_SUPPORT;
}

int app_arc_vcs_control_by_con_lid(uint8_t con_lid, uint8_t opcode, uint8_t volume, bool no_changed_cb)
{
    return vcs_control(con_lid, opcode, volume, no_changed_cb == false, true);
}

int app_arc_vcs_send_ntf(uint8_t con_lid, uint8_t char_type)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_arc_vcs_update_info_req_by_con_lid(uint8_t con_lid, uint8_t bit_map, uint8_t volume, bool isMute)
{
    uint8_t opcode = 0;

    // Update mute need
    if (bit_map & APP_ARC_VC_UPDATE_MUTE_BIT)
    {
        opcode = isMute ? VCS_OPCODE_VOL_MUTE : VCS_OPCODE_VOL_UNMUTE;
    }

    if (bit_map & APP_ARC_VC_UPDATE_VOL_BIT)
    {
        opcode = VCS_OPCODE_VOL_SET_ABS;
    }

    return vcs_control(con_lid, opcode, volume, false, false);
}

int app_arc_vcs_update_info_req(uint8_t bit_map, uint8_t volume, bool isMute)
{
    return BT_STS_NOT_SUPPORT;
}

int app_arc_vcs_restore_bond_data_req(uint8_t con_lid, uint8_t cli_cfg_bf)
{
    return vcs_restore_cli_cfg_cache(con_lid, cli_cfg_bf);
}

#endif

/// @} APP

