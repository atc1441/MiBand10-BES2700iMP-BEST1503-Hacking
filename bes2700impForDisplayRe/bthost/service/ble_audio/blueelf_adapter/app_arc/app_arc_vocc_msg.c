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
#include "app_arc_vocc_msg.h"
#include "app_gaf_custom_api.h"

#include "vocc.h"

/*CALLBACKS*/

/// Callback function called when Audio Input Control Service instance has been discovered in a peer server
/// database
static void app_arc_vocc_cb_bond_data_evt(uint8_t con_lid, uint8_t output_lid, const vocc_prf_svc_t *param)
{
    app_gaf_arc_vocc_bond_data_ind_t found_ind =
    {
        .con_lid = con_lid,
        .output_lid = output_lid,
    };

    LOG_D("app_arc vocc service found con_lid = %d, shdl = %04x, ehdl = %04x, output_id = %04x",
          found_ind.con_lid, param->svc_range.shdl, param->svc_range.ehdl,
          found_ind.output_lid);

#ifdef AOB_MOBILE_ENABLED
    app_gaf_mobile_evt_report(APP_GAF_VOCC_BOND_DATA_IND, (void *)&found_ind, sizeof(found_ind));
#endif
}

/// Callback for vocc discovery done
static void app_arc_vocc_cb_discovery_cmp_evt(uint8_t con_lid, uint16_t err_code)
{
    LOG_I("app_arc vocc discovery cmp, con_lid = %d, err_code = %d", con_lid, err_code);
}

/// Callback for vocc gatt set cfg cmp evt
static void app_arc_vocc_cb_set_cfg_cmp_evt(uint8_t con_lid, uint8_t char_type, uint8_t output_lid, uint16_t err_code)
{
    LOG_D("app_arc vocc set cfg cmp, con_lid = %d, output_lid = %d, char_type = %d, err_code = %d",
          con_lid, output_lid, char_type, err_code);
}

/// Callback for vocc gatt set cfg cmp evt
static void app_arc_vocc_cb_set_val_cmp_evt(uint8_t con_lid, uint8_t char_type, uint8_t output_lid, uint16_t err_code)
{
    LOG_D("app_arc vocc set val cmp, con_lid = %d, output_lid = %d, char_type = %d, err_code = %d",
          con_lid, output_lid, char_type, err_code);
}

/// Callback function called when value for Audio Input Description characteristic has been received from a peer
/// server device
static void app_arc_vocc_cb_description(uint8_t con_lid, uint8_t output_lid, uint16_t desc_val_len, const uint8_t *p_desc_val, uint16_t err_code)
{
    LOG_D("app_arc vocc_desc_ind con_lid= %d, output_lid = %d, desc_len = %d, err_code = %d",
          con_lid, output_lid, desc_val_len, err_code);
}

/// Callback function called when value for Audio Input Type or Audio Audio Input Status characteristic has been
/// received from a peer server device
static void app_arc_vocc_cb_value(uint8_t con_lid, uint8_t output_lid, uint8_t char_type, uint32_t val, uint16_t err_code)
{
    app_gaf_arc_vocc_value_ind_t value_ind =
    {
        .con_lid = con_lid,
        .output_lid = output_lid,
        .char_type = char_type,
        .u.val = val,
    };

    LOG_D("app_arc vocc value ind, output_lid = %d, char_type = %d, value = %d, err_code = %d",
          value_ind.output_lid, value_ind.char_type, value_ind.u.offset, err_code);

#ifdef AOB_MOBILE_ENABLED
    app_gaf_mobile_evt_report(APP_GAF_VOCC_VALUE_IND, (void *)&value_ind, sizeof(value_ind));
#endif
}

/// Callback for vocc cp write cmp evt
static void app_arc_vocc_cb_ctrl_op_cmp_evt(uint8_t con_lid, uint8_t opcode, uint16_t err_code)
{
    LOG_D("app_arc vocc ctrl cmp, con_lid = %d, opcode = %d, err_code = %d",
          con_lid, opcode, err_code);
}

static void app_arc_vcc_cb_prf_status_event(uint8_t con_lid, bool is_central, gatt_prf_status_event_e event)
{

}

static const vocc_evt_cb_t app_arc_vocc_cb =
{
    .cb_bond_data = app_arc_vocc_cb_bond_data_evt,
    .cb_ctrl_op_cmp = app_arc_vocc_cb_ctrl_op_cmp_evt,
    .cb_description = app_arc_vocc_cb_description,
    .cb_discovery_cmp = app_arc_vocc_cb_discovery_cmp_evt,
    .cb_set_cfg_cmp = app_arc_vocc_cb_set_cfg_cmp_evt,
    .cb_set_val_cmp = app_arc_vocc_cb_set_val_cmp_evt,
    .cb_value = app_arc_vocc_cb_value,
    .cb_prf_status_event = app_arc_vcc_cb_prf_status_event,
};

int app_arc_vocc_init(void)
{
    LOG_I("%s", __func__);

    vocc_init_cfg_t vocc_init_cfg =
    {
        .vocs_found_num_supp_max = 2,
    };

    return vocc_init(&vocc_init_cfg, &app_arc_vocc_cb);
}

int app_arc_vocc_deinit(void)
{
    LOG_I("%s", __func__);

    return vocc_deinit();
}

int app_arc_vocc_start(uint8_t con_lid)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_arc_vocc_set_desc(uint8_t con_lid, uint8_t output_lid,
                          char *desc, uint16_t desc_len)
{
    return vocc_character_val_write(con_lid, output_lid,
                                    VOCS_CHAR_TYPE_DESC, (uint8_t *)desc, desc_len);
}

int app_arc_vocc_set_cfg(uint8_t con_lid, uint8_t output_lid,
                         uint8_t char_type, uint8_t enable)
{
    return vocc_character_cccd_write(con_lid, output_lid, char_type, enable);
}

int app_arc_vocc_control(uint8_t con_lid, uint8_t output_lid,
                         uint8_t set_type, uint32_t value)
{
    uint8_t char_type = set_type;

    if (char_type == VOCS_CHAR_TYPE_OFFSET)
    {
        return vocc_offset_control_operation(con_lid, output_lid, value);
    }
    else
    {
        return vocc_character_val_write(con_lid, output_lid, VOCS_CHAR_TYPE_LOC,
                                        (uint8_t *)&value, sizeof(value));
    }
}

int app_arc_vocc_read(uint8_t con_lid, uint8_t output_lid, uint8_t char_type)
{
    return vocc_character_value_read(con_lid, output_lid, char_type);
}

#endif
#endif
/// @} APP
