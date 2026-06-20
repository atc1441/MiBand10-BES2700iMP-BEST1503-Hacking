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
#include "bluetooth.h"

#include "app_gaf.h"
#include "ble_app_dbg.h"

#include "mcs.h"

#include "app_acc_mcs_msg.h"
#include "app_gaf_custom_api.h"

enum app_acc_mcs_object
{
    APP_MCS_MEDIA_PALYER_ICON_OBJ = 0,
    APP_MCS_CURRENT_TRACK_SEG_OBJ,
    APP_MCS_CURRENT_TRACK_OBJ,
    APP_MCS_NEXT_TRACK_OBJ,
    APP_MCS_CURRENT_GROUP_OBJ,
    APP_MCS_PARENT_GROUP_OBJ,
    APP_MCS_SEARCH_RESULTS_OBJ,
    APP_MCS_OBJ_MAX,
};

static const mcs_object_id_t mcs_object_id[APP_MCS_OBJ_MAX] =
{
    {{0x00, 0x00, 0x00, 0x00, 0x01, 0x01}},
    {{0x00, 0x00, 0x00, 0x00, 0x01, 0x02}},
    {{0x00, 0x00, 0x00, 0x00, 0x01, 0x03}},
    {{0x00, 0x00, 0x00, 0x00, 0x01, 0x04}},
    {{0x00, 0x00, 0x00, 0x00, 0x01, 0x05}},
    {{0x00, 0x00, 0x00, 0x00, 0x01, 0x06}},
    {{0x00, 0x00, 0x00, 0x00, 0x01, 0x07}}
};

/// Callback for mcs bond data
static void app_mcs_cb_bond_data_evt(uint8_t media_lid, uint8_t con_lid, uint8_t char_type,
                                     uint32_t cli_cfg_bf)
{
    LOG_I("app_acc mcs bond_data_ind con_lid= %d, media_lid = %d, char_type = %d, cli_cfg_bf = %08x",
          con_lid, media_lid, char_type, cli_cfg_bf);
}

/// Callback function called when either Current Track Object ID or Next Track Object
static void app_mcs_cb_set_object_id_req(uint8_t media_lid, uint8_t con_lid, uint8_t char_type,
                                         const mcs_object_id_t *p_obj_id)
{
    uint16_t cfm_status = mcs_val_set_req_cfm(BT_STS_SUCCESS, media_lid, 0);

    LOG_I("app_acc mcs set object id con_lid= %d, media_lid = %d, char_type = %d, cfm_status = %d",
          con_lid, media_lid, char_type, cfm_status);
}

/// Callback function called when a Client device request a control on a Media
static void app_mcs_cb_ctrl_req(uint8_t media_lid, uint8_t con_lid, uint8_t opcode, int32_t val)
{
    LOG_I("app_acc mcs control req con_lid= %d, media_lid = %d, opcode = %d, val (if exists) = %d",
          con_lid, media_lid, opcode, val);

#ifdef AOB_MOBILE_ENABLED
    app_gaf_mcs_control_req_ind *p_mcs_control_req =
        (app_gaf_mcs_control_req_ind *)app_gaf_malloc_buff(sizeof(app_gaf_mcs_control_req_ind) + sizeof(int32_t));
    if (p_mcs_control_req == NULL)
    {
        mcs_control_req_cfm(BT_STS_NO_RESOURCES, media_lid, 0, 0, 0, 0);
        LOG_E("%s no resources", __func__);
        return;
    }
    p_mcs_control_req->con_lid = con_lid;
    p_mcs_control_req->media_lid = media_lid;
    p_mcs_control_req->opcode = opcode;
    *(int32_t *)((uint32_t)p_mcs_control_req + sizeof(app_gaf_mcs_control_req_ind)) = val;
    app_gaf_mobile_evt_report(APP_GAF_MCS_CONTROL_REQ_IND, (void *)p_mcs_control_req,
                              sizeof(app_gaf_mcs_control_req_ind));
    app_gaf_free_buff(p_mcs_control_req);
#endif
}

/// Callback function called when a Search request has been received from Client device
static void app_mcs_cb_search_req(uint8_t media_lid, uint8_t con_lid, uint8_t param_len,
                                  const uint8_t *p_param)
{
    uint16_t cfm_status = mcs_search_req_cfm(BT_STS_SUCCESS, media_lid, (mcs_object_id_t *)p_param);

    LOG_I("app_acc mcs search req con_lid= %d, media_lid = %d, cfm_status = %d",
          con_lid, media_lid, cfm_status);
}

/// Callback function called when a Client device request to set value for either
static void app_mcs_cb_set_req(uint8_t media_lid, uint8_t con_lid, uint8_t char_type, uint32_t val)
{
    LOG_I("app_acc mcs set val con_lid= %d, media_lid = %d, char_type = %d, set_val = %d",
          con_lid, media_lid, char_type, val);

#ifdef AOB_MOBILE_ENABLED
    app_gaf_mcs_set_req_ind_t mcs_set_req = {0};

    mcs_set_req.con_lid = con_lid;
    mcs_set_req.media_lid = media_lid;
    mcs_set_req.char_type = char_type;
    mcs_set_req.param.param = val;

    app_gaf_mobile_evt_report(APP_GAF_MCS_SET_RI, (void *)&mcs_set_req, sizeof(mcs_set_req));
#endif
}

/// Callback function called when a Client device request to set value for either
static void app_mcs_cb_get_req(uint8_t media_lid, uint8_t con_lid, uint8_t char_type, uint16_t offset)
{
    LOG_I("app_acc mcs get val con_lid= %d, media_lid = %d, char_type = %d, offset = %d",
          con_lid, media_lid, char_type, offset);

#ifdef AOB_MOBILE_ENABLED
    app_gaf_mcs_get_req_ind_t mcs_get_req = {0};

    mcs_get_req.con_lid = con_lid;
    mcs_get_req.media_lid = media_lid;
    mcs_get_req.char_type = char_type;
    mcs_get_req.offset = offset;

    app_gaf_mobile_evt_report(APP_GAF_MCS_GET_RI, (void *)&mcs_get_req, sizeof(mcs_get_req));
#endif
}

/// Callback function called to inform upper layer that a media action is complete
static void app_mcs_cb_media_action_cmp(uint8_t media_lid, uint8_t action, uint16_t status)
{
    LOG_I("app_acc mcs action cmp media_lid = %d, action = %d, status = %d",
          media_lid, action, status);
}

static const mcs_evt_cb_t mcs_evt_cb =
{
    .cb_bond_data = app_mcs_cb_bond_data_evt,
    .cb_control_req = app_mcs_cb_ctrl_req,
    .cb_search_req = app_mcs_cb_search_req,
    .cb_set_object_id_req = app_mcs_cb_set_object_id_req,
    .cb_set_req = app_mcs_cb_set_req,
    .cb_get_req = app_mcs_cb_get_req,
    .cb_action_cmp = app_mcs_cb_media_action_cmp,
};

int app_acc_mcs_init(void)
{
    LOG_I("%s", __func__);

    mcs_inst_cfg_t gmcs_inst_cfg =
    {
        .pref_mtu = GAF_PREFERRED_MTU,
        .ccid = APP_GAF_ACC_GMCS_CCID,
        .opcodes_supp_bf = MCS_MEDIA_CP_OPCODES_SUPP_MASK,
        .playing_order_supp_bf = MCS_PLAY_ORDER_SUPP_MASK,
        /// TODO: get from app_acc ots
        .transfer_lid = 0,
    };

    mcs_init_cfg_t mcs_init_cfg =
    {
        .nb_mcs_inst_supp = APP_ACC_DFT_MCS_NUM,
        .p_gmcs_inst_cfg = &gmcs_inst_cfg,
    };

    uint16_t status = mcs_init(&mcs_init_cfg, &mcs_evt_cb);

    // Add tbs instant
    if (status == BT_STS_SUCCESS)
    {
        gmcs_inst_cfg.ccid = APP_GAF_ACC_MCS_CCID;

        status = mcs_add_mcs_instant(&gmcs_inst_cfg, NULL);
    }

    return status;
}

int app_acc_mcs_deinit(void)
{
    LOG_I("%s", __func__);

    return mcs_deinit();
}

/*MCS INTERFACES*/
int app_acc_mcs_control_req_cfm(uint16_t status, uint8_t media_lid,
                                uint8_t action, int32_t track_pos, int8_t seeking_speed)
{
    return mcs_control_req_cfm(status, media_lid, MCS_MEDIA_CP_RESULT_SUCCESS,
                               action, track_pos, seeking_speed);
}

int app_acc_mcs_val_get_req_cfm(uint16_t status, uint8_t con_lid, uint8_t media_lid,
                                const uint8_t *p_buf_prepare, uint16_t buf_len)
{
    return mcs_val_get_req_cfm(status, con_lid, media_lid, p_buf_prepare, buf_len);
}

int app_acc_mcs_val_set_req_cfm(uint16_t status, uint8_t media_lid, uint32_t val)
{
    return mcs_val_set_req_cfm(status, media_lid, val);
}

int app_acc_mcs_set_req(uint8_t media_lid, uint8_t char_type, int32_t val)
{
    return mcs_set_character_val(media_lid, char_type, (uint8_t *)&val, sizeof(val));
}

int app_acc_mcs_set_obj_id_req(uint8_t media_lid, uint8_t *obj_id)
{
    return mcs_set_character_val(media_lid, MCS_CHAR_TYPE_NEXT_TRACK_OBJ_ID,
                                 obj_id, sizeof(mcs_object_id_t));
}

int app_acc_mcs_set_player_name_req(uint8_t media_lid, uint8_t *name, uint8_t name_len)
{
    return mcs_set_character_val(media_lid, MCS_CHAR_TYPE_PLAYER_NAME, name, name_len);
}

int app_acc_mcs_action_req(uint8_t media_lid, uint8_t action, int32_t track_pos,
                           int8_t seeking_speed)
{
    return mcs_media_action(media_lid, action, track_pos, seeking_speed);
}

int app_acc_mcs_track_change_req(uint8_t media_lid, int32_t track_dur, uint8_t *title,
                                 uint8_t title_len)
{
    if (title == NULL)
    {
        LOG_W("%s use default title name", __func__);
        title = (uint8_t *)APP_ACC_DFT_TITLE_NAME;
        title_len = sizeof(APP_ACC_DFT_TITLE_NAME);
    }

    return mcs_track_change_update(media_lid, track_dur,
                                   &mcs_object_id[APP_MCS_CURRENT_TRACK_SEG_OBJ],
                                   &mcs_object_id[APP_MCS_CURRENT_TRACK_OBJ],
                                   &mcs_object_id[APP_MCS_NEXT_TRACK_OBJ],
                                   &mcs_object_id[APP_MCS_CURRENT_GROUP_OBJ],
                                   &mcs_object_id[APP_MCS_PARENT_GROUP_OBJ],
                                   title_len, title);
}

#endif
#endif