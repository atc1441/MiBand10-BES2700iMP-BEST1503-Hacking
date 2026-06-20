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
#include "app_acc_mcc_msg.h"
#include "app_gaf_custom_api.h"

#include "mcc.h"

const uint16_t app_acc_mcs_char_uuid_list_simplified[] =
{
    /// Media Player Name characteristic
    GATT_CHAR_UUID_MEDIA_PLAYER_NAME,
    /// Track Changed characteristic
    GATT_CHAR_UUID_TRACK_CHANGED,
    /// Track Title characteristic
    GATT_CHAR_UUID_TRACK_TITLE,
    /// Track Duration characteristic
    GATT_CHAR_UUID_TRACK_DURATION,
    /// Track Position characteristic
    GATT_CHAR_UUID_TRACK_POSITION,
    /// Media State characteristic
    GATT_CHAR_UUID_MEDIA_STATE,
    /// Media Control Point
    GATT_CHAR_UUID_MEDIA_CTRL_POINT,
    /// Media Control Point Opcodes Supported
    GATT_CHAR_UUID_MEDIA_CTRL_POINT_OPCODES_SUPP,
    /// Content Control ID
    GATT_CHAR_UUID_CONTENT_CONTROL_ID,
};

#if (APP_ACC_MCC_DISCOVERY_ONLY_FIND_CHAR)
static void app_mcc_continue_set_cfg(uint8_t con_lid, uint8_t media_lid, uint8_t char_type)
{
    uint16_t status = BT_STS_SUCCESS;

    do
    {
        // If char type is reach max
        if (char_type >= MCS_CHAR_TYPE_MAX)
        {
            return;
        }
        // May return with err: not supp or not ready, do not break and continue next
        status = mcc_character_cccd_write(con_lid, media_lid, char_type, true);
        // Increase char type if this char write cccd is failed
        char_type++;
    } while (status);
}

static void app_mcc_read_all_char_val(uint8_t con_lid, uint8_t media_lid)
{
    mcc_character_value_read(con_lid, media_lid, MCS_CHAR_TYPE_MEDIA_CP_OPCODES_SUPP);
    mcc_character_value_read(con_lid, media_lid, MCS_CHAR_TYPE_MEDIA_STATE);
}
#endif /// (APP_ACC_MCC_DISCOVERY_ONLY_FIND_CHAR)

/*MCC CALLBACKS*/
static void app_acc_mcc_cb_bond_data(uint8_t con_lid, uint8_t media_lid, const mcc_prf_svc_t *param)
{
    LOG_D("app_acc mcc service found media_lid = %d, uuid = %04x, shdl = %04x, ehdl = %04x",
          media_lid, param->uuid,
          param->svc_range.shdl, param->svc_range.ehdl);

    app_gaf_acc_mcc_bond_data_ind_t bond_data;

    bond_data.con_lid = con_lid;
    bond_data.media_lid = media_lid;
    memcpy((uint8_t *)&bond_data.mcs_info, (uint8_t *)param, sizeof(mcc_prf_svc_t));

    app_gaf_evt_report(APP_GAF_MCC_BOND_DATA_IND, (void *)&bond_data,
                       sizeof(app_gaf_acc_mcc_bond_data_ind_t));

#if (APP_ACC_MCC_DISCOVERY_ONLY_FIND_CHAR)
    app_mcc_read_all_char_val(con_lid, media_lid);
    app_mcc_continue_set_cfg(con_lid, media_lid, MCS_CHAR_TYPE_PLAYER_NAME);
#endif
}

static void app_acc_mcc_cb_cfg_value(uint8_t con_lid, uint8_t media_lid, uint8_t char_type,
                                     bool enabled, uint16_t err_code)
{
    LOG_D("app_acc mcc_cfg_ind con_lid= %d, media_lid = %d, char_type = %d, enabled = %d",
          con_lid, media_lid, char_type, enabled);
}

static void app_acc_mcc_object_id_ind_handler(uint8_t con_lid, uint8_t media_lid, uint8_t char_type,
                                              const uint8_t *p_obj_id)
{
    LOG_D("app_acc mcc_obj_id_ind con_lid= %d, media_lid = %d, char_type = %d, obj_id:",
          con_lid, media_lid, char_type);

    if (p_obj_id != NULL)
    {
        DUMP8("%02x ", p_obj_id, MCS_OBJ_ID_LEN);
    }
}

static void app_acc_mcc_track_changed_ind_handler(uint8_t con_lid, uint8_t media_lid)
{
    app_gaf_acc_mcc_track_changed_ind_t track_changed =
    {
        .con_lid = con_lid,
        .media_lid = media_lid,
    };

    LOG_D("app_acc mcc_track_changed_ind con_lid= %d, media_lid = %d", con_lid, media_lid);

    app_gaf_evt_report(APP_GAF_MCC_TRACK_CHANGED_IND, (void *)&track_changed, sizeof(track_changed));
}

static void app_acc_mcc_cb_char_value(uint8_t con_lid, uint8_t media_lid, uint8_t char_type,
                                      const uint8_t *data, uint16_t len, uint16_t err_code)
{
    LOG_D("app_acc mcc_val_ind con_lid = %d, media_lid = %d, char_type = %d, err_code = %d",
          con_lid, media_lid, char_type, err_code);

    if (err_code != BT_STS_SUCCESS)
    {
        return;
    }

    LOG_D("app_acc mcc_val_ind val_len= %d", len);

    if (char_type == MCS_CHAR_TYPE_PLAYER_NAME ||
            char_type == MCS_CHAR_TYPE_TRACK_TITLE)
    {
        if (data == NULL)
        {
            LOG_W("%s char_type = %d is NULL, err_code = %d", __func__, char_type, err_code);
            return;
        }

        DUMP8("%c", data, len);
    }

    app_gaf_acc_mcc_value_ind_t val_ind =
    {
        .char_type = char_type,
        .con_lid = con_lid,
        .media_lid = media_lid,
    };

    switch (char_type)
    {
        case (MCS_CHAR_TYPE_MEDIA_STATE):
        {
            val_ind.val.state = *data;
            break;
        }
        case (MCS_CHAR_TYPE_TRACK_CHANGED):
        {
            app_acc_mcc_track_changed_ind_handler(con_lid, media_lid);
            return;
        }
        case (MCS_CHAR_TYPE_PLAYER_ICON_OBJ_ID):
        case (MCS_CHAR_TYPE_CUR_TRACK_SEG_OBJ_ID):
        case (MCS_CHAR_TYPE_CUR_TRACK_OBJ_ID):
        case (MCS_CHAR_TYPE_NEXT_TRACK_OBJ_ID):
        case (MCS_CHAR_TYPE_CUR_GROUP_OBJ_ID):
        case (MCS_CHAR_TYPE_PARENT_GROUP_OBJ_ID):
        case (MCS_CHAR_TYPE_SEARCH_RESULTS_OBJ_ID):
        {
            app_acc_mcc_object_id_ind_handler(con_lid, media_lid, char_type, data);
            return;
        }
        default:
            /// TODO: Assign value here
            break;
    }

    app_gaf_evt_report(APP_GAF_MCC_MEDIA_VALUE_IND, (void *)&val_ind,
                       sizeof(app_gaf_acc_mcc_value_ind_t));
}

static void app_acc_mcc_cb_included_svc_found(uint8_t con_lid, uint8_t media_lid, uint16_t uuid, uint8_t transfer_lid)
{
    LOG_I("app_acc mcc_included_svc_ind con_lid = %d, media_lid = %d, uuid = %x, transfer_lid = %d",
          con_lid, media_lid, uuid, transfer_lid);
}

static void app_acc_mcc_cb_svc_discovery_cmp(uint8_t con_lid, uint16_t err_code)
{
    app_gaf_acc_mcc_cmp_evt_t cmp_evt =
    {
        .con_lid = con_lid,
        .status = err_code,
    };

    LOG_I("app_acc mcc discovery cmp, status = %d, con_lid = %d", err_code, con_lid);

    app_gaf_evt_report(APP_GAF_MCC_SVC_DISCOVERYED_IND, (void *)&cmp_evt,
                       sizeof(app_gaf_acc_mcc_cmp_evt_t));
}

static void app_acc_mcc_cb_mcp_ctrl_cmp(uint8_t con_lid, uint8_t media_lid, uint8_t op_code,
                                        uint16_t err_code, uint8_t ctrl_result)
{
    LOG_I("app_acc mcc ctrl cmp, status = %d, con_lid = %d, opcode = %d, result = %d", err_code,
          con_lid, op_code, ctrl_result);
}

static void app_acc_mcc_cb_set_cfg_cmp(uint8_t con_lid, uint8_t media_lid, uint8_t char_type,
                                       uint16_t err_code)
{
    LOG_D("app_acc mcc set cfg cmp, status = %d, con_lid = %d, media_lid = %d, char_type = %d",
          err_code, con_lid, media_lid, char_type);

#if (APP_ACC_MCC_DISCOVERY_ONLY_FIND_CHAR)
    app_mcc_continue_set_cfg(con_lid, media_lid, (char_type + 1));
#endif
}

static void app_acc_mcc_cb_set_char_val_cmp(uint8_t con_lid, uint8_t media_lid,
                                            uint8_t char_type, uint16_t err_code)
{
    LOG_D("app_acc mcc set cmp, status = %d, con_lid = %d, char_type = %d", err_code, con_lid,
          char_type);
}

static void app_acc_mcc_cb_mcp_search_cmp(uint8_t con_lid, uint8_t media_lid, uint16_t err_code,
                                          uint8_t search_result)
{
    LOG_I("app_acc mcc search cmp, status = %d, con_lid = %d, media_lid = %d, search_result = %d",
          err_code, con_lid, media_lid, search_result);
}

static void app_acc_mcc_cb_prf_status_event(uint8_t con_lid, bool is_central, gatt_prf_status_event_e event)
{
    if (event != GATT_PRF_STATUS_EVENT_SVC_CHANGED)
    {
        return;
    }

    LOG_I("app_acc mcc_svc_changed_ind con_lid = %d", con_lid);

    app_gaf_acc_mcc_svc_changed_ind_t svc_changed =
    {
        .con_lid = con_lid,
    };

    app_gaf_evt_report(APP_GAF_MCC_SVC_CHANGED_IND, (void *)&svc_changed,
                       sizeof(app_gaf_acc_mcc_svc_changed_ind_t));
}

static const mcc_evt_cb_t p_mcc_evt_cb =
{
    .cb_bond_data = app_acc_mcc_cb_bond_data,
    .cb_cfg_value = app_acc_mcc_cb_cfg_value,
    .cb_char_value = app_acc_mcc_cb_char_value,
    .cb_set_val_cmp = app_acc_mcc_cb_set_char_val_cmp,
    .cb_discovery_cmp = app_acc_mcc_cb_svc_discovery_cmp,
    .cb_inc_svc_found = app_acc_mcc_cb_included_svc_found,
    .cb_mcp_search_cmp = app_acc_mcc_cb_mcp_search_cmp,
    .cb_set_cfg_cmp = app_acc_mcc_cb_set_cfg_cmp,
    .cb_mcp_ctrl_cmp =  app_acc_mcc_cb_mcp_ctrl_cmp,
    .cb_prf_status_event = app_acc_mcc_cb_prf_status_event,
};

int app_acc_mcc_init(void)
{
    LOG_I("app_acc mcc init");

    mcc_init_cfg_t init_cfg =
    {
        .cp_write_reliable = true,
        .discovery_only_find_char = APP_ACC_MCC_DISCOVERY_ONLY_FIND_CHAR,
        .max_supp_mcs_inst = APP_ACC_MCC_MAX_SUPP_MCS_INST,
    };

    return mcc_init(&init_cfg, &p_mcc_evt_cb);
}

int app_acc_mcc_deinit(void)
{
    LOG_I("app_acc mcc deinit");

    return mcc_deinit();
}

/*MCC INTERFACES*/
int app_acc_mcc_start(uint8_t con_lid)
{
    LOG_I("app_acc mcc full discovery start, con_lid = %d", con_lid);

    return mcc_service_discovery_full(con_lid);
}

int app_acc_mcc_simplified_start(uint8_t con_lid)
{
    LOG_I("app_acc mcc simplified discovery start, con_lid = %d", con_lid);

    return mcc_service_discovery_with_uuid_list(con_lid,
                                                app_acc_mcs_char_uuid_list_simplified,
                                                ARRAY_SIZE(app_acc_mcs_char_uuid_list_simplified));
}

int app_acc_mcc_restore_bond_data_req(uint8_t con_lid, uint8_t nb_media, void const *data)
{
    LOG_W("app_acc mcc restore bond data dose not support");
    return 0;
}

int app_acc_mcc_get(uint8_t con_lid, uint8_t media_lid, uint8_t char_type)
{
    return mcc_character_value_read(con_lid, media_lid, char_type);
}

int app_acc_mcc_set_cfg(uint8_t con_lid, uint8_t media_lid, uint8_t char_type, uint8_t enable)
{
    return mcc_character_cccd_write(con_lid, media_lid, char_type, enable);
}

int app_acc_mcc_set(uint8_t con_lid, uint8_t media_lid, uint8_t char_type, int32_t val)
{
    return mcc_character_val_write(con_lid, media_lid, char_type, (uint8_t *)&val, sizeof(uint32_t));
}

int app_acc_mcc_set_obj_id(uint8_t con_lid, uint8_t media_lid, uint8_t char_type, uint8_t *obj_id)
{
    return mcc_character_val_write(con_lid, media_lid, char_type, obj_id, sizeof(mcs_object_id_t));
}

int app_acc_mcc_control(uint8_t con_lid, uint8_t media_lid, uint8_t opcode, uint32_t val)
{
    return mcc_mcp_control(con_lid, media_lid, opcode, val);
}

int app_acc_mcc_search(uint8_t con_lid, uint8_t media_lid, uint8_t param_len, uint8_t *param)
{
    return mcc_mcp_search_items(con_lid, media_lid, (mcc_mcp_search_op_t *)param, param_len);
}

#endif

/// @} APP
