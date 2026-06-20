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
#include "app_arc_aicc_msg.h"
#include "app_gaf_custom_api.h"

#include "aicc.h"

/*CALLBACKS*/

/// Callback function called when Audio Input Control Service instance has been discovered in a peer server
/// database
static void app_arc_aicc_cb_bond_data_evt(uint8_t con_lid, uint8_t input_lid, const aicc_prf_svc_t *param)
{
    LOG_D("app_arc aicc found ind, con_lid = %d, input_lid = %d, shdl = %04x, ehdl = %04x, uuid = %02x",
          con_lid, input_lid, param->svc_range.shdl, param->svc_range.ehdl, param->uuid);
}

/// Callback for aicc discovery done
static void app_arc_aicc_cb_discovery_cmp_evt(uint8_t con_lid, uint16_t err_code)
{
    LOG_I("app_arc aicc discovery cmp, con_lid = %d, errr_code = %d", con_lid, err_code);
}

/// Callback for aicc gatt set cfg cmp evt
static void app_arc_aicc_cb_set_cfg_cmp_evt(uint8_t con_lid, uint8_t char_type, uint8_t input_lid, uint16_t err_code)
{
    LOG_D("app_arc aicc set cfg cmp, con_lid = %d, char_type = %d, input_lid = %d, err_code = %d",
          con_lid, char_type, input_lid, err_code);
}

/// Callback for aicc gatt set description cmp evt
static void app_arc_aicc_cb_set_desc_cmp_evt(uint8_t con_lid, uint8_t input_lid, uint16_t err_code)
{
    LOG_D("app_arc aicc set description cmp, con_lid = %d, input_lid = %d, err_code = %d",
          con_lid, input_lid, err_code);
}

/// Callback function called when value for Audio Input State characteristic has been received from a peer server
/// device
static void app_arc_aicc_cb_input_state(uint8_t con_lid, uint8_t input_lid, int8_t gain, uint8_t mute, uint8_t mode, uint16_t err_code)
{
    LOG_D("app_arc aicc gain ind con_lid = %d, input_lid = %d, gain = %d, gain_mode = %d, mute = %d, err_code = %d",
          con_lid, input_lid, gain, mode, mute, err_code);
}

/// Callback function called when value for Gain Setting Properties characteristic has been received from a peer
/// server device
static void app_arc_aicc_cb_gain_prop(uint8_t con_lid, uint8_t input_lid, uint8_t units, int8_t min, int8_t max, uint16_t err_code)
{
    LOG_D("app_arc aicc_gain_prop_ind con_lid = %d, input_lid= %d, units = %d, min = %d, max = %d, err_code = %d",
          con_lid, input_lid, units, min, max, err_code);
}

/// Callback function called when value for Audio Input Description characteristic has been received from a peer
/// server device
static void app_arc_aicc_cb_description(uint8_t con_lid, uint8_t input_lid, uint16_t desc_val_len, const uint8_t *p_desc_val, uint16_t err_code)
{
    LOG_D("app_arc aicc_desc_ind con_lid = %d, input_lid = %d, desc_len = %d",
          con_lid, input_lid, desc_val_len);
}

/// Callback function called when value for Audio Input Type or Audio Audio Input Status characteristic has been
/// received from a peer server device
static void app_arc_aicc_cb_value(uint8_t con_lid, uint8_t input_lid, uint8_t char_type, uint8_t val, uint16_t err_code)
{
    LOG_D("app_arc aicc_value_ind con_lid = %d, input_lid= %d, char_type = %d, val = %d, err_code = %d",
          con_lid, input_lid, char_type, val, err_code);
}

/// Callback for aicc cp write cmp evt
static void app_arc_aicc_cb_ctrl_op_cmp_evt(uint8_t con_lid, uint8_t input_lid, uint8_t opcode, uint16_t err_code)
{
    LOG_D("app_arc aicc set cmp, con_lid = %d, input_lid = %d, opcode = %d, err_code = %d",
          con_lid, input_lid, opcode, err_code);
}

static void app_arc_aicc_cb_prf_status_event(uint8_t con_lid, bool iscentral, gatt_prf_status_event_e event)
{

}

static const aicc_evt_cb_t app_arc_aicc_cb =
{
    .cb_bond_data = app_arc_aicc_cb_bond_data_evt,
    .cb_ctrl_op_cmp = app_arc_aicc_cb_ctrl_op_cmp_evt,
    .cb_description = app_arc_aicc_cb_description,
    .cb_discovery_cmp = app_arc_aicc_cb_discovery_cmp_evt,
    .cb_gain_prop = app_arc_aicc_cb_gain_prop,
    .cb_input_state = app_arc_aicc_cb_input_state,
    .cb_set_cfg_cmp = app_arc_aicc_cb_set_cfg_cmp_evt,
    .cb_set_desc_cmp = app_arc_aicc_cb_set_desc_cmp_evt,
    .cb_value = app_arc_aicc_cb_value,
    .cb_prf_status_event = app_arc_aicc_cb_prf_status_event,
};

int app_arc_aicc_init(void)
{
    LOG_I("%s", __func__);

    aicc_init_cfg_t aicc_init_cfg =
    {
        .aics_found_num_supp_max = 2,
    };

    return aicc_init(&aicc_init_cfg, &app_arc_aicc_cb);
}

int app_arc_aicc_deinit(void)
{
    LOG_I("%s", __func__);

    return aicc_deinit();
}

int app_arc_aicc_start(uint8_t con_lid)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_arc_aicc_set_desc(uint8_t con_lid, uint8_t input_lid,
                          char *desc, uint16_t desc_len)
{
    return aicc_set_description(con_lid, input_lid, desc_len, (uint8_t *)desc);
}

int app_arc_aicc_set_cfg(uint8_t con_lid, uint8_t input_lid,
                         uint8_t char_type, uint8_t enable)
{
    return aicc_character_cccd_write(con_lid, input_lid, char_type, enable);
}

int app_arc_aicc_control(uint8_t con_lid, uint8_t input_lid,
                         uint8_t opcode, int16_t value)
{
    return aicc_control_operation(con_lid, input_lid, opcode, value);
}

int app_arc_aicc_read(uint8_t con_lid, uint8_t input_lid, uint8_t char_type)
{
    return aicc_character_value_read(con_lid, input_lid, char_type);
}

#endif
#endif

/// @} APP

