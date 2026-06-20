/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#if BLE_AUDIO_ENABLED
#include "bluetooth.h"
#include "ble_app_dbg.h"

#include "app_hap_has_msg.h"

#include "has.h"

/*FUNCTIONS*/

/// Callback for has bond data
static void app_hap_has_cb_bond_data(uint8_t con_lid, uint8_t char_type, uint8_t cli_cfg_ntf_bf, uint8_t cli_cfg_ind_bf)
{
    LOG_D("app_hap has bond data, con_lid = %d, char_type = %d, cli_cfg_ntf = 0x%x, cli_cfg_ind = 0x%x",
          con_lid, char_type, cli_cfg_ntf_bf, cli_cfg_ind_bf);
}

/// Callback function called when a peer Client device requires to update the current Active Preset
static void app_hap_has_cb_set_active_preset_req(uint8_t con_lid, uint8_t preset_lid, bool sync)
{
    LOG_D("app_hap has set active preset, con_lid = %d, preset_lid = %d, sync = %d", con_lid, preset_lid, sync);

    has_set_active_preset_cfm(true);
}

/// Callback function called when a peer Client device has updated name of a Preset
static void app_hap_has_cb_set_preset_name_req(uint8_t con_lid, uint8_t preset_lid, uint8_t length, const uint8_t *p_name)
{
    LOG_D("app_hap has set preset name, con_lid = %d, preset_lid = %d, length = %d, name:", con_lid, preset_lid, length);

    DUMP8("%c", p_name, length);

    has_set_preset_name_cfm(true, length, p_name);
}

/// Callback function called when set active preset local cmp
static void app_hap_has_cb_preset_operation_cmp(uint8_t preset_lid, uint16_t status)
{
    LOG_D("app_hap has set preset op cmp, preset_lid = %d, status = %d", preset_lid, status);
}

static const has_evt_cb_t app_hap_has_cb =
{
    .cb_bond_data = app_hap_has_cb_bond_data,
    .cb_set_active_preset_req = app_hap_has_cb_set_active_preset_req,
    .cb_set_preset_name_req = app_hap_has_cb_set_preset_name_req,
    .cb_set_active_preset_cmp = app_hap_has_cb_preset_operation_cmp,
};

int app_hap_has_init(void)
{
    LOG_I("%s", __func__);

    has_init_cfg_t has_init_cfg =
    {
        .pref_mtu = GAF_PREFERRED_MTU,
#if (APP_HAP_HAS_INIT_2_MORE_PRESET)
        .nb_presets_supp = 3,
#else
        .nb_presets_supp = 1,
#endif
        .features_bf = HAS_FEATURES_TYPE_BINAURAL | HAS_FEATURES_DYNAMIC_PRESETS_BIT,
    };

    uint16_t status = has_init(&has_init_cfg, &app_hap_has_cb);

    if (status == BT_STS_SUCCESS)
    {
        status = has_add_preset(0, true, true, sizeof(APP_HAP_HAS_PRESET_DFT_NAME),
                                (uint8_t *)APP_HAP_HAS_PRESET_DFT_NAME);
    }

#if (APP_HAP_HAS_INIT_2_MORE_PRESET)
    if (status == BT_STS_SUCCESS)
    {
        status = has_add_preset(1, false, true, sizeof(APP_HAP_HAS_PRESET_DFT_NAME),
                                (uint8_t *)APP_HAP_HAS_PRESET_DFT_NAME);
    }

    if (status == BT_STS_SUCCESS)
    {
        status = has_add_preset(2, true, false, sizeof(APP_HAP_HAS_PRESET_DFT_NAME),
                                (uint8_t *)APP_HAP_HAS_PRESET_DFT_NAME);
    }
#endif /// (APP_HAP_HAS_INIT_2_MORE_PRESET)

    return status;
}

int app_hap_has_deinit(void)
{
    LOG_I("%s", __func__);
    return has_deinit();
}

int app_hap_has_msg_restore_bond_data_req(uint8_t con_lid, uint8_t cli_cfg_bf,
                                          uint8_t presets_cli_cfg_bf, uint8_t evt_cfg_bf, uint8_t presets_evt_cfg_bf)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_hap_has_msg_set_preset_active_cfm(uint16_t status)
{
    return has_set_active_preset_cfm((status == BT_STS_SUCCESS));
}

int app_hap_has_msg_update_preset_req(uint8_t preset_lid, bool available, uint8_t length, char *name)
{
    return has_update_preset(preset_lid, available, length, (uint8_t *)name);
}

void app_hap_has_msg_configure_preset_req(uint8_t preset_lid, uint8_t presets_instance_idx,
                                          uint8_t read_only, uint8_t length, char *name)
{
    has_add_preset(preset_lid, (read_only == false), true, length, (uint8_t *)name);
}

int app_hap_has_msg_set_active_preset_req(uint8_t preset_lid)
{
    return has_set_active_preset(preset_lid);
}

int app_hap_has_msg_set_feature_req(uint8_t features_bf)
{
    return has_set_features(features_bf);
}

#endif /// BLE_AUDIO_ENABLED
