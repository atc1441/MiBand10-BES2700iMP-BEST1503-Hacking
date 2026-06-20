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
#include "app_gaf_define.h"

#include "tb_common.h"
#include "tbs.h"

#include "app_acc_tbs_msg.h"
#include "app_gaf_custom_api.h"

/// Callback for tbs bond data
static void app_acc_tbs_cb_bond_data_evt(uint8_t bearer_lid, uint8_t con_lid, uint8_t char_type,
                                         uint16_t cli_cfg_bf)
{
    LOG_I("app_acc tbs bond_data_ind con_lid = %d, bearer_lid = %d, char_type = %d, cli_cfg_bf = %04x",
          con_lid, bearer_lid, char_type, cli_cfg_bf);
    app_gaf_acc_tbs_bond_data_ind_t bond_data =
    {
        .con_lid = con_lid,
        .bearer_lid = bearer_lid,
        .cli_cfg_bf = cli_cfg_bf,
    };

    app_gaf_mobile_evt_report(APP_GAF_TBS_BOND_DATA_IND, (void *)&bond_data, sizeof(bond_data));
}

/// Callback function called when Signal Report intvl characteristic value has been updated
static void app_acc_tbs_cb_report_intv(uint8_t bearer_lid, uint8_t con_lid, uint8_t sign_strength_intv_s)
{
    LOG_D("app_acc p_rpt_intv_ind con_lid = %d, bearer_lid = %d, rpt_intv = %d",
          con_lid, bearer_lid, sign_strength_intv_s);

    app_gaf_acc_tbs_report_intv_ind_t intvl_report =
    {
        .con_lid = con_lid,
        .bearer_lid = bearer_lid,
        .sign_strength_intv_s = sign_strength_intv_s,
    };

    app_gaf_mobile_evt_report(APP_GAF_TBS_REPORT_INTV_IND, (void *)&intvl_report, sizeof(intvl_report));
}

/// Callback function called to request from upper layer complete or piece of value for one of the following characteristics:
/*
 * - Bearer Provider Name characteristic
 * - Incoming Call Target Bearer URI characteristic
 * - Call Friendly Name characteristic
 */
static void app_acc_tbs_cb_val_get_req(uint8_t bearer_lid, uint8_t call_id, uint8_t con_lid, uint8_t char_type, uint16_t offset)
{
    LOG_I("app_acc tbs val get char_type = %d", char_type);

    uint16_t buf_len = 0;
    char *p_val = NULL;

    if (TBS_CHAR_TYPE_PROV_NAME == char_type)
    {
        buf_len = sizeof(APP_ACC_DFT_PROV_NAME);
        p_val = APP_ACC_DFT_PROV_NAME;
    }
    else if (TBS_CHAR_TYPE_URI_SCHEMES_LIST == char_type)
    {
        buf_len = sizeof(APP_ACC_DFT_URI_SCHEMES_LIST);
        p_val = APP_ACC_DFT_URI_SCHEMES_LIST;
    }
    else if (TBS_CHAR_TYPE_IN_TGT_CALLER_ID == char_type)
    {
        buf_len = sizeof(APP_ACC_DFT_CALL1_INCOMING_TGT_URI);
        p_val = APP_ACC_DFT_CALL1_INCOMING_TGT_URI;
    }
    else if (TBS_CHAR_TYPE_CALL_FRIENDLY_NAME == char_type)
    {
        buf_len = sizeof(APP_ACC_DFT_CALL1_INCOMING_FRIENDLY_NAME);
        p_val = APP_ACC_DFT_CALL1_INCOMING_FRIENDLY_NAME;
    }

    tbs_val_get_req_cfm(BT_STS_SUCCESS, bearer_lid, call_id, con_lid,
                        char_type, offset, buf_len - offset, (uint8_t *)(p_val + offset));

    app_gaf_acc_tbs_get_req_ind_t val_get =
    {
        .con_lid = con_lid,
        .bearer_lid = bearer_lid,
        .call_id = call_id,
        .char_type = char_type,
    };

    app_gaf_mobile_evt_report(APP_GAF_TBS_GET_RI, (void *)&val_get,
                              sizeof(app_gaf_acc_tbs_get_req_ind_t));
}

/// Callback function called to inform upper layer that a client device has requested creation of an outgoing call
static void app_acc_tbs_cb_call_req(uint8_t bearer_lid, uint8_t con_lid, uint8_t opcode, uint8_t call_id,
                                    uint8_t len, const uint8_t *p_val)
{
    LOG_D("app_acc tbs call req con_lid = %d, bearer_lid = %d, opcode = %d, call_id = %d", con_lid,
          bearer_lid, opcode, call_id);
    if (p_val != NULL)
    {
        LOG_D("uri_len = %d, uri = %s", len, p_val);
    }

    uint8_t *cfm_val = NULL;
    uint8_t cfm_val_len = 0;

    if (opcode == TBS_OPCODE_ORIGINATE)
    {
        cfm_val = (uint8_t *)APP_ACC_DFT_CALL_OUTGOING_FRIENDLY_NAME;
        cfm_val_len = sizeof(APP_ACC_DFT_CALL_OUTGOING_FRIENDLY_NAME);
    }

    POSSIBLY_UNUSED uint16_t status = tbs_call_req_cfm(TBS_CP_NTF_RESULT_SUCCESS, bearer_lid,
                                                       cfm_val_len, cfm_val);

    LOG_D("app_acc tbs call req - cfm status = %d", status);

    if (opcode == TBS_OPCODE_ORIGINATE)
    {
        app_gaf_acc_tbs_call_out_req_ind_t *p_outgoing_req =
            (app_gaf_acc_tbs_call_out_req_ind_t *)app_gaf_malloc_buff(sizeof(app_gaf_acc_tbs_call_out_req_ind_t) +
                                                               len);

        if (p_outgoing_req == NULL)
        {
            LOG_E("app_acc tbs call outgoing no more resources!!!");
            return;
        }

        p_outgoing_req->con_lid = con_lid,
                        p_outgoing_req->bearer_lid = bearer_lid,
                                        p_outgoing_req->call_id = call_id,
                                                        p_outgoing_req->uri_len = len;

        memcpy(p_outgoing_req->uri, p_val, len);

        app_gaf_mobile_evt_report(APP_GAF_TBS_CALL_OUTGOING_RI,
                                  (void *)p_outgoing_req, sizeof(app_gaf_acc_tbs_call_out_req_ind_t) + len);
        app_gaf_free_buff(p_outgoing_req);
    }
    else if (opcode == TBS_OPCODE_JOIN)
    {
        app_gaf_acc_tbs_call_join_req_ind_t *p_join_req =
            (app_gaf_acc_tbs_call_join_req_ind_t *)app_gaf_malloc_buff(sizeof(app_gaf_acc_tbs_call_join_req_ind_t) +
                                                                len);

        if (p_join_req == NULL)
        {
            LOG_E("app_acc tbs call outgoing no more resources!!!");
            return;
        }

        p_join_req->con_lid = con_lid,
                    p_join_req->bearer_lid = bearer_lid,
                                p_join_req->nb_calls = len;

        memcpy(p_join_req->call_ids, p_val, len);

        app_gaf_mobile_evt_report(APP_GAF_TBS_CALL_JOIN_RI,
                                  (void *)p_join_req, sizeof(app_gaf_acc_tbs_call_join_req_ind_t) + len);
        app_gaf_free_buff(p_join_req);
    }
    else
    {
        app_gaf_acc_tbs_call_action_req_ind_t action_req =
        {
            .con_lid = con_lid,
            .bearer_lid = bearer_lid,
            .call_id = call_id,
            .opcode = opcode,
        };

        app_gaf_mobile_evt_report(APP_GAF_TBS_CALL_ACTION_RI, (void *)&action_req, sizeof(action_req));
    }
}

static void app_acc_tbs_send_gaf_evt_to_upper(int gaf_evt_code, uint8_t bearer_lid, uint8_t status,
                                              uint8_t call_id, uint8_t action)
{
    if (status == BT_STS_SUCCESS)
    {
        app_gaf_acc_tbs_rsp_t tbs_rsp =
        {
            .bearer_lid = bearer_lid,
            .status = status,
            .u.call_id = call_id,
            .action = action,
        };

        app_gaf_mobile_evt_report(gaf_evt_code, (void *)&tbs_rsp, sizeof(tbs_rsp));
    }
}

/// Callback function called to inform upper layer that a call action is complete
static void app_acc_tbs_cb_call_action_cmp(uint8_t bearer_lid, uint8_t action, uint16_t status, uint8_t call_id)
{
    // Additional action for internal use
    if (action == TBS_ADD_ACTION_INCOMING)
    {
        app_acc_tbs_send_gaf_evt_to_upper(APP_GAF_TBS_CALL_INCOMING_REQ_RSP,
                                          bearer_lid, status, call_id, 0);
    }
    else if (action == TBS_ADD_ACTION_OUTGOING)
    {
        app_acc_tbs_send_gaf_evt_to_upper(APP_GAF_TBS_CALL_OUTGOING_REQ_RSP,
                                          bearer_lid, status, call_id, 0);
    }
    else if (action == TBS_ADD_ACTION_JOIN)
    {
        app_acc_tbs_send_gaf_evt_to_upper(APP_GAF_TBS_CALL_JOIN_REQ_RSP,
                                          bearer_lid, status, call_id, 0);
    }
    else // Normal action for external use
    {
        app_acc_tbs_send_gaf_evt_to_upper(APP_GAF_TBS_CALL_ACTION_REQ_RSP,
                                          bearer_lid, status, call_id, 0);
    }
}

static const tbs_evt_cb_t tbs_evt_cb =
{
    .cb_bond_data = app_acc_tbs_cb_bond_data_evt,
    .cb_call_req = app_acc_tbs_cb_call_req,
    .cb_report_intv = app_acc_tbs_cb_report_intv,
    .cb_get_req = app_acc_tbs_cb_val_get_req,
    .cb_action_cmp = app_acc_tbs_cb_call_action_cmp,
};

int app_acc_tbs_init(void)
{
    LOG_I("%s", __func__);

    tbs_inst_cfg_t tbs_inst_cfg =
    {
        .pref_mtu = GAF_PREFERRED_MTU,
        .bearer_uci_len = sizeof(APP_GAF_ACC_UCI_VAL1),
        .p_bearer_uci = (uint8_t *)APP_GAF_ACC_UCI_VAL1,
        .ccid = APP_GAF_ACC_GTBS_CCID,
        .opt_opcodes_bf = TBS_OPT_OPCODES_MASK,
    };

    tbs_init_cfg_t tbs_init_cfg =
    {
        .call_pool_size = APP_ACC_CALL_POOL_SIZE,
        .call_pool_uri_len = APP_ACC_MAX_URI_LEN,
        .uri_len_max = APP_ACC_MAX_URI_LEN,
        .nb_tbs_inst_supp = APP_ACC_DFT_TBS_NUM,
        .p_gtbs_inst_cfg = &tbs_inst_cfg,
    };

    uint16_t status = tbs_init(&tbs_init_cfg, &tbs_evt_cb);

    // Add tbs instant
    if (status == BT_STS_SUCCESS)
    {
        tbs_inst_cfg.ccid = APP_GAF_ACC_TBS_CCID;
        tbs_inst_cfg.bearer_uci_len = sizeof(APP_GAF_ACC_UCI_VAL2);
        tbs_inst_cfg.p_bearer_uci = (uint8_t *)APP_GAF_ACC_UCI_VAL2;

        status = tbs_add_tbs_instant(&tbs_inst_cfg, NULL);
    }

    return status;
}

int app_acc_tbs_deinit(void)
{
    LOG_I("%s", __func__);

    return tbs_deinit();
}

int app_acc_tbs_add_req(uint8_t ccid, uint8_t *uci, uint8_t uci_len)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_acc_tbs_set_req(uint8_t bearer_lid, uint8_t char_type, uint8_t val)
{
    return tbs_set_character_val(bearer_lid, char_type, sizeof(uint8_t), &val);
}

int app_acc_tbs_set_status_req(uint8_t bearer_lid, uint8_t status_type, uint8_t val)
{
    return tbs_set_call_status(bearer_lid, status_type, val);
}

int app_acc_tbs_set_long_req(uint8_t bearer_lid, uint8_t char_type, uint8_t *val, uint8_t len)
{
    uint8_t status = 0;
    status = tbs_set_character_val(bearer_lid, char_type, len, val);
    TRACE(0, "%s bearer_lid %d char_type %d len %d status %04x", __func__, bearer_lid, char_type, len, status);
    return status;
}

int app_acc_tbs_call_incoming_req(uint8_t bearer_lid, uint8_t *val, uint8_t uri_len,
                                  uint8_t tgt_uri_len, uint8_t friendly_name_len)
{
    return tbs_call_incoming(bearer_lid, uri_len, tgt_uri_len, friendly_name_len,
                             &val[0], &val[uri_len], &val[uri_len + tgt_uri_len]);
}

int app_acc_tbs_call_outgoing_req(uint8_t bearer_lid, uint8_t *val,
                                  uint8_t uri_len, uint8_t friendly_name_len)
{
    return tbs_call_outgoing(bearer_lid, uri_len, friendly_name_len, &val[0], &val[uri_len]);
}

int app_acc_tbs_call_join_req(uint8_t bearer_lid,  uint8_t nb_calls, uint8_t *call_ids)
{
    return tbs_call_join(bearer_lid, nb_calls, call_ids);
}

int app_acc_tbs_call_action_req(uint8_t bearer_lid, uint8_t call_id, uint8_t action, uint8_t reason)
{
    return tbs_call_action(bearer_lid, call_id, action, reason);
}

#endif
#endif

/// @} APP
