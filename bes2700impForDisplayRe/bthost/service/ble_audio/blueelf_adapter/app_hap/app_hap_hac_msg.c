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
#ifdef AOB_MOBILE_ENABLED
#include "bluetooth.h"
#include "ble_app_dbg.h"

#include "app_hap_hac_msg.h"

#include "hac.h"

/*FUNCTIONS*/

/// Callback function called when Hearing Access Service has been discovered in a Service device database
static void app_hap_hac_cb_bond_data(uint8_t con_lid, const hac_prf_svc_t *param)
{
    LOG_I("app_hap bond data con_lid = %d, shdl = %d, ehdl = %d", con_lid,
          param->svc_range.shdl, param->svc_range.ehdl);
}

/// Callback function called when Hearing Access Service has been discovered cmp
static void app_hap_hac_cb_discovery_cmp(uint8_t con_lid, uint16_t err_code)
{
    LOG_I("app_hap discovery cmp con_lid = %d, err_code = %d", con_lid, err_code);
}

/// Callback for hac gatt set cfg cmp evt
static void app_hap_hac_cb_set_cfg_cmp(uint8_t con_lid, uint8_t char_type, uint16_t err_code)
{
    LOG_I("app_hap set cfg cmp con_lid = %d, char_type = %d, err_code = %d",
          con_lid, char_type, err_code);
}

/// Callback function called when a Preset Record is received
static void app_hap_hac_cb_preset(uint8_t con_lid, bool last, uint8_t preset_idx, bool writable,
                                  bool available, uint8_t length, const uint8_t *p_name)
{
    LOG_I("app_hap preset value con_lid = %d, is_last = %d, preset_idx = %d,\
                                    writeable = %d, available = %d, name_length = %d, name =",
          con_lid, last, preset_idx, writable, available, length);
    DUMP8("%c", p_name, length);
}

/// Callback function called when either Active Preset index or Hearing Aid Features
/// bit field value is received
static void app_hap_hac_cb_value(uint8_t con_lid, uint8_t char_type, uint8_t value, uint16_t err_code)
{
    LOG_I("app_hap char value con_lid = %d, char_type = %d, value = %d, err_code = %d",
          con_lid, char_type, value, err_code);
}

/// Callback function called when:
/// - A new preset record is added
/// - The name of an existing preset record has changed
/// - More than one change happened during disconnection
static void app_hap_hac_cb_generic_update(uint8_t con_lid, bool last, uint8_t prev_preset_idx, uint8_t preset_idx,
                                          bool writable, bool available, uint8_t length, const uint8_t *p_name)
{
    LOG_I("app_hap generic update con_lid = %d, is_last = %d, prev_preset_idx = %d, preset_idx = %d,\
                                    writeable = %d, available = %d, name_length = %d, name =",
          con_lid, last, prev_preset_idx, preset_idx, writable, available, length);
}

/// Callback function called when:
/// - A preset record has been deleted
/// - A preset record has become available
/// - A preset record has become unavailable
static void app_hap_hac_cb_update(uint8_t con_lid, uint8_t change_id, bool last, uint8_t preset_idx)
{
    LOG_I("app_hap update con_lid = %d, is_last = %d, change_id = %d, preset_idx = %d",
          con_lid, last, change_id, preset_idx);
}

/// Callback function called when Write CP is complete
static void app_hap_hac_cb_hap_ctrl_cmp(uint8_t con_lid, uint8_t opcode, uint16_t err_code)
{
    LOG_I("app_hap hap ctrl cmp con_lid = %d, opcode = %d, err_code = %d",
          con_lid, opcode, err_code);
}

static void app_hap_hac_cb_prf_status_event(uint8_t con_lid, bool is_central, gatt_prf_status_event_e event)
{
    if (event == GATT_PRF_STATUS_EVENT_OPENED && is_central == true)
    {
        //hac_service_discovery(con_lid);
    }
}

static const hac_evt_cb_t app_hap_hac_callbak =
{
    .cb_bond_data = app_hap_hac_cb_bond_data,
    .cb_discovery_cmp = app_hap_hac_cb_discovery_cmp,
    .cb_set_cfg_cmp = app_hap_hac_cb_set_cfg_cmp,
    .cb_value = app_hap_hac_cb_value,
    .cb_update = app_hap_hac_cb_update,
    .cb_generic_update = app_hap_hac_cb_generic_update,
    .cb_preset = app_hap_hac_cb_preset,
    .cb_hap_ctrl_cmp = app_hap_hac_cb_hap_ctrl_cmp,
    .cb_prf_status_event = app_hap_hac_cb_prf_status_event,
};

int app_hap_hac_init(void)
{
    LOG_I("%s", __func__);

    hac_init_cfg_t hac_init_cfg;

    return hac_init(&hac_init_cfg, &app_hap_hac_callbak);
}

int app_hap_hac_deinit(void)
{
    LOG_I("%s", __func__);

    return hac_deinit();
}

int app_hap_hac_start(uint8_t con_lid)
{
    return hac_service_discovery(con_lid);
}

int app_hap_hac_msg_set_preset_name(uint8_t con_lid, uint8_t preset_idx,
                                    uint8_t length, char *name)
{
    return hac_hap_ctrl_set_preset_name(con_lid, preset_idx, length, (uint8_t *)name);
}

int app_hap_hac_msg_get_preset(uint8_t con_lid, uint8_t preset_idx, bool is_read_all)
{
    return hac_hap_ctrl_get_preset(con_lid, preset_idx, is_read_all);
}

int app_hap_hac_msg_set_active_preset(uint8_t con_lid, uint8_t opcode, uint8_t preset_idx)
{
    return hac_hap_ctrl_set_active_preset(con_lid, opcode, preset_idx);
}

int app_hap_hac_restore_bond_data_req(uint8_t con_lid)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

#endif
#endif