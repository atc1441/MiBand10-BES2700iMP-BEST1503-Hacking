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

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#if BLE_AUDIO_ENABLED
#include "app_gaf.h"
#include "ble_app_dbg.h"
#include "app_acc_tbc_msg.h"
#include "app_gaf_custom_api.h"

#include "tbc.h"

#define APP_ACC_TBC_MAX_SUPP_TBS_NUM    (1)

const uint16_t app_acc_tbs_char_uuid_list_simplified[] =
{
    /// Bearer Provider Name characteristic
    GATT_CHAR_UUID_BEARER_PROVIDER_NAME,
    /// Bearer List Current Calls characteristic
    GATT_CHAR_UUID_BEARER_LIST_CURRENT_CALLS,
    /// Status Flags characteristic
    GATT_CHAR_UUID_STATUS_FLAGS,
    /// Call State characteristic
    GATT_CHAR_UUID_CALL_STATE,
    /// Call Control Point characteristic
    GATT_CHAR_UUID_CALL_CTRL_POINT,
    /// Termination Reason characteristic
    GATT_CHAR_UUID_TERMINATION_REASON,
    /// Incoming Call characteristic
    GATT_CHAR_UUID_INCOMING_CALL,
    /// Bearer URI Schemes Supported List characteristic
    GATT_CHAR_UUID_BEARER_URI_SCHEMES_SUPP_LIST,
    /// Bearer UCI characteristic
    GATT_CHAR_UUID_BEARER_UCI,
    /// Content Control ID characteristic
    GATT_CHAR_UUID_CONTENT_CONTROL_ID,
    /// Call Control Point Optional Opcodes characteristic
    GATT_CHAR_UUID_CALL_CTRL_POINT_OPTIONAL_OPCODES,
};

/*
 * FUNCTION DEFINATIONS
 ****************************************************************************************
 */
#if (APP_ACC_TBC_DISCOVERY_ONLY_FIND_CHAR)
static void app_tbc_continue_set_cfg(uint8_t con_lid, uint8_t bearer_lid, uint8_t char_type)
{
    uint16_t status = BT_STS_SUCCESS;

    do
    {
        // If char type is reach max
        if (char_type >= TBS_CHAR_TYPE_MAX)
        {
            return;
        }
        // SS can not handle this set cfg
        else if (char_type == TBS_CHAR_TYPE_TECHNO)
        {
            status = BT_STS_NOT_ALLOW;
        }
        else
        {
            // May return with err: not supp or not ready, do not break and continue next
            status = tbc_character_cccd_write(con_lid, bearer_lid, char_type, true);
        }
        // Increase char type if this char write cccd is failed
        char_type++;
    } while (status);
}

static void app_tbc_read_all_char_val(uint8_t con_lid, uint8_t bearer_lid)
{
    tbc_character_value_read(con_lid, bearer_lid, TBS_CHAR_TYPE_CALL_CTL_PT_OPT_OPCODES);
    tbc_character_value_read(con_lid, bearer_lid, TBS_CHAR_TYPE_STATUS_FLAGS);
    tbc_character_value_read(con_lid, bearer_lid, TBS_CHAR_TYPE_CURR_CALLS_LIST);
    tbc_character_value_read(con_lid, bearer_lid, TBS_CHAR_TYPE_INCOMING_CALL);
}
#endif /// (APP_ACC_TBC_DISCOVERY_ONLY_FIND_CHAR)

/// Callback for tbc bond data
void app_tbc_cb_bond_data_evt(uint8_t con_lid, uint8_t bearer_lid, const tbc_prf_svc_t *param)
{
    LOG_D("app_acc tbc service found bearer_lid = %d, uuid = %04x, shdl = %04x, ehdl = %04x",
          bearer_lid, param->uuid, param->svc_range.shdl, param->svc_range.ehdl);

    app_gaf_acc_tbc_bond_data_ind_t bond_data;

    bond_data.con_lid    = con_lid;
    bond_data.bearer_lid = bearer_lid;

    memcpy((uint8_t *)&bond_data.tbs_info, (uint8_t *)param, sizeof(tbc_prf_svc_t));

    app_gaf_evt_report(APP_GAF_TBC_BOND_DATA_IND, (void *)&bond_data,
                       sizeof(app_gaf_acc_tbc_bond_data_ind_t));

    // One service is found, then start set cfg and read val or stack will done with it
#if (APP_ACC_TBC_DISCOVERY_ONLY_FIND_CHAR)
    app_tbc_read_all_char_val(con_lid, bearer_lid);
    app_tbc_continue_set_cfg(con_lid, bearer_lid, TBS_CHAR_TYPE_PROV_NAME);
#endif
}

/// Callback for tbc discovery done
void app_tbc_cb_discovery_cmp_evt(uint8_t con_lid, uint16_t err_code)
{
    LOG_I("app_acc tbc discovery cmp, con_lid = %d, status = %d", con_lid, err_code);

    app_gaf_acc_tbc_cmp_evt_t cmp_evt =
    {
        .con_lid = con_lid,
        .status = err_code,
    };

    app_gaf_evt_report(APP_GAF_TBC_SVC_DISCOVERYED_IND, (void *)&cmp_evt,
                       sizeof(app_gaf_acc_tbc_cmp_evt_t));
}

/// Callback for tbc set signal stength report interval cmp evt
void app_tbc_cb_set_character_value_cmp_evt(uint8_t con_lid, uint8_t bearer_lid,
                                            uint8_t char_type, uint16_t err_code)
{
    LOG_D("app_acc tbc set charcater value cmp, con_lid = %d, bearer_lid = %d, char_type = %d, status = %d",
          char_type, con_lid, bearer_lid, err_code);
}

/// Callback for tbc call control opcode cmp evt
void app_tbc_cb_ccp_control_cmp_evt(uint8_t con_lid, uint8_t bearer_lid, uint8_t op_code,
                                    uint16_t err_code, uint8_t ctrl_result)
{
    LOG_I("app_acc tbc control cmp, con_lid = %d, bearer_lid = %d, opcode = %d, status = %d, ctrl_result = %d",
          con_lid, bearer_lid, op_code, err_code, ctrl_result);

    app_gaf_acc_tbc_cmp_evt_t cmp_evt =
    {
        .con_lid = con_lid,
        .bearer_lid = bearer_lid,
        .status = err_code,
        .result = ctrl_result,
        .u.opcode = op_code,
    };

    app_gaf_evt_report(APP_GAF_TBC_CALL_ACTION_RESULT_IND, (void *)&cmp_evt, sizeof(cmp_evt));
}

/// Callback for tbc gatt cmd complete, @see enum tbc_cmd_code
void app_tbc_cb_set_cfg_cmp_evt(uint8_t con_lid, uint8_t bearer_lid, uint8_t char_type,
                                uint16_t err_code)
{
    LOG_D("app_acc set cfg cmp con_lid= %d, bearer_lid = %d, char_type = %d, status = %d",
          con_lid, bearer_lid, char_type, err_code);

#if (APP_ACC_TBC_DISCOVERY_ONLY_FIND_CHAR)
    app_tbc_continue_set_cfg(con_lid, bearer_lid, (char_type + 1));
#endif
}

/// Callback for tbc cccd cfg value received or read
void app_tbc_cb_cfg_evt(uint8_t con_lid, uint8_t bearer_lid, uint8_t char_type, bool enabled,
                        uint16_t err_code)
{
    LOG_D("app_acc cfg val evt con_lid= %d, bearer_lid = %d, char_type = %d, status = %d, enable = %d",
          con_lid, bearer_lid, char_type, err_code, enabled);
}

/// Callback for tbc value ntf or read
void app_tbc_cb_call_state_evt(uint8_t con_lid, uint8_t bearer_lid, uint8_t call_id, uint8_t flags,
                               uint8_t state)
{
    LOG_D("app_acc tbc_call_state_ind con_lid = %d, bearer_lid = %d, call_id = %d, flags = %d, state = %d",
          con_lid, bearer_lid,  call_id, flags, state);

    app_gaf_acc_tbc_call_state_ind_t call_state =
    {
        .con_lid = con_lid,
        .bearer_lid = bearer_lid,
        .state = state,
        .flags = flags,
        .id = call_id,
    };

    app_gaf_evt_report(APP_GAF_TBC_CALL_STATE_IND, (void *)&call_state, sizeof(call_state));
}

/// Callback for tbc value ntf or read
void app_tbc_cb_call_state_long_evt(uint8_t con_lid, uint8_t bearer_lid, uint8_t call_id,
                                    uint8_t flags, uint8_t state, uint8_t uri_len, const uint8_t *p_uri)
{
    LOG_D("app_acc tbc_call_state_long_ind con_lid = %d, bearer_lid = %d, call_id = %d, flags = %d, state = %d",
          con_lid, bearer_lid, call_id, flags, state);

    DUMP8("%c", p_uri, uri_len);

    app_gaf_acc_tbc_call_state_long_ind_t *p_call_state_long = (app_gaf_acc_tbc_call_state_long_ind_t *)
                                                               app_gaf_malloc_buff(sizeof(app_gaf_acc_tbc_call_state_long_ind_t) + uri_len);

    if (p_call_state_long == NULL)
    {
        LOG_E("app_acc tbc_call_state_long_ind, mo more resources!!!");
        return;
    }

    p_call_state_long->con_lid = con_lid;
    p_call_state_long->bearer_lid = bearer_lid;
    p_call_state_long->state = state;
    p_call_state_long->flags = flags;
    p_call_state_long->id = call_id;
    p_call_state_long->uri_len = uri_len;

    memcpy(p_call_state_long->uri, p_uri, uri_len);

    uint32_t param_size = sizeof(app_gaf_acc_tbc_call_state_long_ind_t) + uri_len;

    app_gaf_evt_report(APP_GAF_TBC_CALL_STATE_LONG_IND, (void *)p_call_state_long, param_size);

    app_gaf_free_buff(p_call_state_long);
}

/// Callback for tbc value ntf or read
void app_tbc_cb_value_evt(uint8_t con_lid, uint8_t bearer_lid, uint8_t call_id, uint8_t char_type,
                          const uint8_t *data, uint16_t len, uint16_t err_code)
{
    switch (char_type)
    {
        case TBS_CHAR_TYPE_IN_TGT_CALLER_ID:
        case TBS_CHAR_TYPE_INCOMING_CALL:
        case TBS_CHAR_TYPE_CALL_FRIENDLY_NAME:
        case TBS_CHAR_TYPE_CURR_CALLS_LIST:
        {
            LOG_D("app_acc tbc_val_long_ind con_lid= %d,bearer_lid = %d,call_id = %d,char_type = %d",
                  con_lid, bearer_lid, call_id, char_type);

            DUMP8("%c", data, len);

            app_gaf_acc_tbc_value_long_ind_t *p_val_long_ind =
                (app_gaf_acc_tbc_value_long_ind_t *)app_gaf_malloc_buff(sizeof(app_gaf_acc_tbc_value_long_ind_t) + len);

            p_val_long_ind->con_lid = con_lid;
            p_val_long_ind->bearer_lid = bearer_lid;
            p_val_long_ind->call_id = call_id;
            p_val_long_ind->char_type = char_type;
            p_val_long_ind->val_len = len;

            memcpy(p_val_long_ind->val, data, len);

            uint32_t param_size = sizeof(app_gaf_acc_tbc_value_long_ind_t) + len;

            app_gaf_evt_report(APP_GAF_TBC_CALL_VALUE_LONG_IND, (void *)p_val_long_ind, param_size);

            app_gaf_free_buff(p_val_long_ind);
        }
        break;
        default:
        {
            LOG_D("app_acc tbc_val_ind con_lid= %d,bearer_lid = %d,call_id = %d,char_type = %d",
                  con_lid, bearer_lid, call_id, char_type);

            DUMP8("%c", data, len);

            app_gaf_acc_tbc_value_ind_t val_ind =
            {
                .con_lid = con_lid,
                .bearer_lid = bearer_lid,
                .call_id = call_id,
                .char_type = char_type,
                .val.val = 0,
            };

            if (len == sizeof(uint16_t))
            {
                val_ind.val.val = *(uint16_t *)data;
            }
            else if (len == sizeof(uint8_t))
            {
                val_ind.val.val = *(uint8_t *)data;
            }

            app_gaf_evt_report(APP_GAF_TBC_CALL_VALUE_IND, (void *)&val_ind, sizeof(val_ind));
        }
        break;
    }
}

static void app_acc_tbc_cb_prf_status_event(uint8_t con_lid, bool is_central, gatt_prf_status_event_e event)
{
    if (event != GATT_PRF_STATUS_EVENT_SVC_CHANGED)
    {
        return;
    }

    app_gaf_acc_tbc_svc_changed_ind_t svc_changed =
    {
        .con_lid = con_lid,
    };

    LOG_D("app_acc tbc_svc_changed_ind con_lid = %d", con_lid);

    app_gaf_evt_report(APP_GAF_TBC_SVC_CHANGED_IND, (void *)&svc_changed, sizeof(svc_changed));
}

static const tbc_evt_cb_t tbc_evt_cb =
{
    .cb_bond_data = app_tbc_cb_bond_data_evt,
    .cb_call_state = app_tbc_cb_call_state_evt,
    .cb_call_state_long = app_tbc_cb_call_state_long_evt,
    .cb_ccp_ctrl_cmp = app_tbc_cb_ccp_control_cmp_evt,
    .cb_set_val_cmp = app_tbc_cb_set_character_value_cmp_evt,
    .cb_cfg_value = app_tbc_cb_cfg_evt,
    .cb_discovery_cmp = app_tbc_cb_discovery_cmp_evt,
    .cb_set_cfg_cmp = app_tbc_cb_set_cfg_cmp_evt,
    .cb_char_value = app_tbc_cb_value_evt,
    .cb_prf_status_event = app_acc_tbc_cb_prf_status_event,
};

int app_acc_tbc_init(void)
{
    LOG_I("%s", __func__);

    tbc_init_cfg_t init_cfg =
    {
        .cp_write_reliable = true,
        .discovery_only_find_char = APP_ACC_TBC_DISCOVERY_ONLY_FIND_CHAR,
        .max_supp_tbs_inst = APP_ACC_TBC_MAX_SUPP_TBS_NUM,
    };

    return tbc_init(&init_cfg, &tbc_evt_cb);
}

int app_acc_tbc_deinit(void)
{
    LOG_I("%s", __func__);

    return tbc_deinit();
}

int app_acc_tbc_start(uint8_t con_lid)
{
    LOG_I("app_acc tbc full discovery start, con_lid = %d", con_lid);

    return tbc_service_discovery_full(con_lid);
}

int app_acc_tbc_simplified_start(uint8_t con_lid)
{
    LOG_I("app_acc tbc simplified discovery start, con_lid = %d", con_lid);

    return tbc_service_discovery_with_uuid_list(con_lid,
                                                app_acc_tbs_char_uuid_list_simplified,
                                                ARRAY_SIZE(app_acc_tbs_char_uuid_list_simplified));
}

int app_acc_tbc_restore_bond_data_req(uint8_t con_lid, uint8_t nb_bearers, void const *data)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_acc_tbc_get_char_value(uint8_t con_lid, uint8_t bearer_lid, uint8_t char_type)
{
    return tbc_character_value_read(con_lid, bearer_lid, char_type);
}

int app_acc_tbc_set_cfg(uint8_t con_lid, uint8_t bearer_lid, uint8_t char_type, uint8_t enable)
{
    return tbc_character_cccd_write(con_lid, bearer_lid, char_type, enable);
}

int app_acc_tbc_get_cfg(uint8_t con_lid, uint8_t bearer_lid, uint8_t char_type)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_acc_tbc_set_rpt_intv(uint8_t con_lid, uint8_t bearer_lid, uint8_t interval,
                             uint8_t reliable)
{
    return tbc_ccp_set_signal_report_intvl(con_lid, bearer_lid, interval);
}

int app_acc_tbc_call_outgoing(uint8_t con_lid, uint8_t bearer_lid, uint8_t *uri, uint8_t uri_len)
{
    return tbc_ccp_call_outgoing(con_lid, bearer_lid, uri, uri_len);
}

int app_acc_tbc_call_action(uint8_t con_lid, uint8_t bearer_lid, uint8_t call_id, uint8_t opcode)
{
    return tbc_ccp_call_action(con_lid, bearer_lid, opcode, call_id);
}

int app_acc_tbc_call_join(uint8_t con_lid, uint8_t bearer_lid, uint8_t nb_calls, uint8_t *call_ids)
{
    return tbc_ccp_call_join(con_lid, bearer_lid, nb_calls, call_ids);
}

#endif /// BLE_AUDIO_ENABLED