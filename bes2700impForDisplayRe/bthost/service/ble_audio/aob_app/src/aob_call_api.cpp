/**
 * @file aob_call_api.cpp
 * @author BES AI team
 * @version 0.1
 * @date 2021-05-24
 *
 * @copyright Copyright (c) 2015-2021 BES Technic.
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
 */
/**
 ****************************************************************************************
 * @addtogroup AOB_APP
 * @{
 ****************************************************************************************
 */

/*****************************header include********************************/
#include "apps.h"
#include "app_status_ind.h"
#include "bluetooth_bt_api.h"
#include "app_media_player.h"
#include "app_gaf_dbg.h"
#include "app_gaf_custom_api.h"
#include "aob_mgr_gaf_evt.h"
#include "ble_audio_core_api.h"
#include "ble_audio_earphone_info.h"
#include "aob_stream_handler.h"
#include "app_audio_control.h"
#include "aob_media_api.h"
#include "aob_call_api.h"
#include "app_acc_tbc_msg.h"

/************************private macro defination***************************/
#define AOB_CALL_MAX_CACHE_CATION 5

/************************private type defination****************************/
static aob_call_report_ui_value_handler_t aob_call_report_ui_value_handler_cb = NULL;
const BLE_AUD_CORE_EVT_CB_T *p_call_cb = NULL;

/**
 * @brief
 */
typedef struct _action_node
{
    struct list_node    action_node;
    uint8_t             action_opcode;
    uint8_t             bearer_lid;
    uint8_t             call_id;
    /* action additional parameters */
    union
    {
        struct
        {
            uint8_t *call_ids;
            uint8_t nb_calls;
        } join;

        struct
        {
            uint8_t *uri;
            uint8_t uri_len;
        } outgoing;
    } parameters;
} action_node_t;

/****************************function defination****************************/
static void aob_call_execute_pending_action(uint8_t conidx)
{
    AOB_CALL_ENV_INFO_T *info = NULL;
    AOB_CALL_ACTION_CONTEXT_T *ctx = NULL;

    info = ble_audio_earphone_info_get_call_env_info(conidx);
    if (!info)
    {
        LOG_E("NULL pointer in %s", __func__);
        return;
    }

    ctx = &info->action_ctx;
    if (ctx->is_busy == true)
    {
        return;
    }

    struct list_node *node = colist_get_head(&ctx->action_list);
    if (node)
    {
        action_node_t *p_action = colist_structure(node, action_node_t, action_node);
        colist_delete(node);

        switch (p_action->action_opcode)
        {
            case APP_GAF_ACC_TB_OPCODE_ACCEPT:
            case APP_GAF_ACC_TB_OPCODE_TERMINATE:
            case APP_GAF_ACC_TB_OPCODE_HOLD:
            case APP_GAF_ACC_TB_OPCODE_RETRIEVE:
            {
                app_acc_tbc_call_action(conidx, p_action->bearer_lid, p_action->call_id, p_action->action_opcode);
            } break;
            case APP_GAF_ACC_TB_OPCODE_ORIGINATE:
            {
                app_acc_tbc_call_outgoing(conidx, 0, p_action->parameters.outgoing.uri,
                                                     p_action->parameters.outgoing.uri_len);
                bes_bt_buf_free(p_action->parameters.outgoing.uri);
            } break;
            case APP_GAF_ACC_TB_OPCODE_JOIN:
            {
                app_acc_tbc_call_join(conidx, p_action->bearer_lid, p_action->parameters.join.nb_calls,
                                                                    p_action->parameters.join.call_ids);
                bes_bt_buf_free(p_action->parameters.join.call_ids);
            } break;
            default:
            {
                ASSERT(0, "%s invalid opcode %d", __func__, p_action->action_opcode);
            }
        }

        bes_bt_buf_free(p_action);
        ctx->is_busy = true;
    }
}

static void aob_call_start_pending_action(uint8_t conidx, action_node_t *action)
{
    AOB_CALL_ENV_INFO_T *info = NULL;
    AOB_CALL_ACTION_CONTEXT_T *ctx = NULL;

    info = ble_audio_earphone_info_get_call_env_info(conidx);
    if (!info || !action)
    {
        LOG_E("NULL pointer in %s", __func__);
        return;
    }

    ctx = &info->action_ctx;

    if (colist_item_count(&ctx->action_list) <= AOB_CALL_MAX_CACHE_CATION)
    {
        colist_addto_tail(&action->action_node, &ctx->action_list);
    }
    else
    {
        LOG_E("%s cache action reach max", __func__);
    }

    aob_call_execute_pending_action(conidx);
}

void aob_call_if_free_pending_actions(struct list_node *action_list)
{
    struct list_node *curr = NULL;
    struct list_node *next = NULL;

    if (!action_list)
    {
        LOG_E("NULL pointer in %s", __func__);
        return;
    }

    colist_iterate_safe(curr, next, action_list)
    {
        action_node_t *p_action = colist_structure(curr, action_node_t, action_node);
        colist_delete(curr);

        switch (p_action->action_opcode)
        {
            case APP_GAF_ACC_TB_OPCODE_ORIGINATE:
            {
                bes_bt_buf_free(p_action->parameters.outgoing.uri);
            } break;
            case APP_GAF_ACC_TB_OPCODE_JOIN:
            {
                bes_bt_buf_free(p_action->parameters.join.call_ids);
            } break;
        }

        bes_bt_buf_free(p_action);
    }
}


void aob_call_tbs_discovery(uint8_t con_lid)
{
    app_acc_tbc_start(con_lid);
}

void aob_call_if_join_call(uint8_t conidx, uint8_t callNb, uint8_t *callIds)
{
    AOB_CALL_ENV_INFO_T *info = ble_audio_earphone_info_get_call_env_info(conidx);
    if (!info)
    {
        LOG_E("NULL pointer in %s", __func__);
        return;
    }
    ASSERT(callNb <= AOB_COMMON_CALL_MAX_NB_IDS, "callIds exceeds the capability %s", __func__);

    action_node_t *action = (action_node_t *)bes_bt_buf_malloc(sizeof(action_node_t));
    memset(action, 0, sizeof(action_node_t));
    action->action_opcode = APP_GAF_ACC_TB_OPCODE_JOIN;
    action->bearer_lid = info->single_call_info[0].bearer_lid;
    action->parameters.join.nb_calls = callNb;
    action->parameters.join.call_ids = (uint8_t *)bes_bt_buf_malloc(callNb);
    memcpy(action->parameters.join.call_ids, callIds, callNb);
    aob_call_start_pending_action(conidx, action);
}

void aob_call_if_outgoing_dial(uint8_t conidx, uint8_t *uri, uint8_t uriLen)
{
    action_node_t *action = (action_node_t *)bes_bt_buf_malloc(sizeof(action_node_t));
    memset(action, 0, sizeof(action_node_t));
    action->action_opcode = APP_GAF_ACC_TB_OPCODE_ORIGINATE;
    action->bearer_lid = 0;
    action->parameters.outgoing.uri_len = uriLen;
    action->parameters.outgoing.uri = (uint8_t *)bes_bt_buf_malloc(uriLen);
    memcpy(action->parameters.outgoing.uri, uri, uriLen);
    aob_call_start_pending_action(conidx, action);
}

void aob_call_if_retrieve_call(uint8_t conidx, uint8_t call_id)
{
    AOB_SINGLE_CALL_INFO_T *info = ble_audio_earphone_info_find_call_info(conidx, call_id);
    if (!info)
    {
        LOG_E("NULL pointer in %s", __func__);
        return;
    }

    action_node_t *action = (action_node_t *)bes_bt_buf_malloc(sizeof(action_node_t));
    memset(action, 0, sizeof(action_node_t));
    action->action_opcode = APP_GAF_ACC_TB_OPCODE_RETRIEVE;
    action->bearer_lid = info->bearer_lid;
    action->call_id = call_id;
    aob_call_start_pending_action(conidx, action);
}

void aob_call_if_hold_call(uint8_t conidx, uint8_t call_id)
{
    AOB_SINGLE_CALL_INFO_T *info = ble_audio_earphone_info_find_call_info(conidx, call_id);
    if (!info)
    {
        LOG_E("NULL pointer in %s", __func__);
        return;
    }

    action_node_t *action = (action_node_t *)bes_bt_buf_malloc(sizeof(action_node_t));
    memset(action, 0, sizeof(action_node_t));
    action->action_opcode = APP_GAF_ACC_TB_OPCODE_HOLD;
    action->bearer_lid = info->bearer_lid;
    action->call_id = call_id;
    aob_call_start_pending_action(conidx, action);
}

void aob_call_if_terminate_call(uint8_t conidx, uint8_t call_id)
{
    AOB_SINGLE_CALL_INFO_T *info = ble_audio_earphone_info_find_call_info(conidx, call_id);
    if (!info)
    {
        LOG_E("NULL pointer in %s", __func__);
        return;
    }

    action_node_t *action = (action_node_t *)bes_bt_buf_malloc(sizeof(action_node_t));
    memset(action, 0, sizeof(action_node_t));
    action->action_opcode = APP_GAF_ACC_TB_OPCODE_TERMINATE;
    action->bearer_lid = info->bearer_lid;
    action->call_id = call_id;
    aob_call_start_pending_action(conidx, action);
}

void aob_call_if_accept_call(uint8_t conidx, uint8_t call_id)
{
    AOB_SINGLE_CALL_INFO_T *info = ble_audio_earphone_info_find_call_info(conidx, call_id);
    if (!info)
    {
        LOG_E("NULL pointer in %s", __func__);
        return;
    }

    action_node_t *action = (action_node_t *)bes_bt_buf_malloc(sizeof(action_node_t));
    memset(action, 0, sizeof(action_node_t));
    action->action_opcode = APP_GAF_ACC_TB_OPCODE_ACCEPT;
    action->bearer_lid = info->bearer_lid;
    action->call_id = call_id;
    aob_call_start_pending_action(conidx, action);
}

void aob_call_if_action_call(uint8_t conidx, uint8_t call_id, uint8_t opcode)
{
    action_node_t *action = (action_node_t *)bes_bt_buf_malloc(sizeof(action_node_t));
    memset(action, 0, sizeof(action_node_t));
    action->action_opcode = opcode;
    action->bearer_lid = 0;
    action->call_id = call_id;
    aob_call_start_pending_action(conidx, action);
}

/// return AOB_CALL_STATE_E type
uint8_t aob_call_if_call_state_get(uint8_t conidx, uint8_t call_id)
{
    AOB_SINGLE_CALL_INFO_T *info = ble_audio_earphone_info_find_call_info(conidx, call_id);
    uint8_t call_state;
    if (!info)
    {
        call_state = AOB_CALL_STATE_IDLE;
    }
    else
    {
        call_state = info->state;
    }
    LOG_I("aob_call_if_call_state_get conidx:%d, call_id:%d, call_state:%d", conidx, call_id, call_state);
    return call_state;
}

// only one device is active.
bool aob_call_is_device_call_active(uint8_t conidx)
{
    uint8_t state = AOB_CALL_STATE_IDLE;
    AOB_MOBILE_INFO_T *info = NULL;

    info = ble_audio_earphone_info_get_mobile_info(conidx);
    if (info == NULL)
    {
        return false;
    }

    for (uint8_t i = 0; i < AOB_COMMON_CALL_MAX_NB_IDS; i++)
    {
        state = info->call_env_info.single_call_info[i].state;
        if (state == AOB_CALL_STATE_ACTIVE)
        {
            LOG_I("get call state conidx:%d,call_id:%d,call_state:%d",
                  conidx, info->call_env_info.single_call_info[i].call_id, state);
            return true;
        }
    }

    return false;
}

/// char_type is defined by AOB_CALL_CHARACTERISTIC_TYPE_E.
void aob_call_if_get_specified_char_value(uint8_t conidx, uint8_t call_id, uint8_t char_type)
{
    AOB_SINGLE_CALL_INFO_T *info = ble_audio_earphone_info_find_call_info(conidx, call_id);
    ASSERT(info, "NULL pointer in %s", __func__);
    LOG_I("aob_call_if_get_specified_char_value conidx:%d, char_type:%d", conidx, char_type);
    app_acc_tbc_get_char_value(conidx, info->bearer_lid, char_type);
}

void aob_call_if_customif_ui_handler_register(aob_call_report_ui_value_handler_t call_value_ui_handler)
{
    aob_call_report_ui_value_handler_cb = call_value_ui_handler;
}

/*****************************************************************************************************************
*
*                    call callback function of ear phone sides                                                   *
*
******************************************************************************************************************/
static void aob_call_state_change_cb(uint8_t con_lid, uint8_t call_id, void *param)
{
    AOB_SINGLE_CALL_INFO_T *p_state_ind = (AOB_SINGLE_CALL_INFO_T *)param;
    AOB_CALL_ENV_INFO_T *pCallEnvInfo = ble_audio_earphone_info_get_call_env_info(con_lid);
    AOB_CALL_STATE_E call_state_curr = p_state_ind->state;
    uint8_t other_call_id = ble_audio_earphone_info_get_another_valid_call_id(call_id, pCallEnvInfo);
    AOB_SINGLE_CALL_INFO_T *p_call_info = NULL;
    LOG_I("[%s] con_lid:%d call_id:%d state:%d", __func__, con_lid, p_state_ind->call_id, call_state_curr);

    switch (call_state_curr)
    {
        case AOB_CALL_STATE_INCOMING:
#if defined(MEDIA_PLAYER_SUPPORT)
            //inband ring disable
            if (!pCallEnvInfo->status_flags.inband_ring_enable)
            {
                LOG_D("[%s] outband ringtone", __func__);
                /*
                 * Some manufacturers keep both bt and ble connected.
                 * user can switch between bt and ble during music/call/ringtone,
                 * Eg: user switch bt to le audio when rintone is onging,le ringtone should not process.
                */
                app_ble_audio_sink_streaming_handle_event(con_lid, 0, APP_GAF_DIRECTION_MAX, BLE_AUDIO_CALL_RINGING_IND);
            }
#endif
            if (other_call_id != INVALID_CALL_ID)
            {
                p_call_info = ble_audio_earphone_info_find_call_info(con_lid, other_call_id);
                if (p_call_info->state == AOB_CALL_STATE_ACTIVE)
                {
                    app_audio_ctrl_le_update_call_state(con_lid, call_id, CALL_STATE_THREE_WAY_INCOMING);
                }
            }
            else
            {
                app_audio_ctrl_le_update_call_state(con_lid, call_id, CALL_STATE_INCOMING);
            }
            break;
        case AOB_CALL_STATE_DIALING:
            app_audio_ctrl_le_update_call_state(con_lid, call_id, CALL_STATE_DIALING);
            break;
        case AOB_CALL_STATE_ALERTING:
            app_audio_ctrl_le_update_call_state(con_lid, call_id, CALL_STATE_ALERTING);
            break;
        case AOB_CALL_STATE_ACTIVE:
            //set call singal report interval
            app_acc_tbc_set_rpt_intv(con_lid, p_state_ind->bearer_lid, CALL_SIGNAL_STRENGTH_REPORT_INTERVAL, 1);
            if (other_call_id != INVALID_CALL_ID)
            {
                other_call_id = ble_audio_earphoe_info_get_call_id_by_conidx_and_type(con_lid, AOB_CALL_STATE_LOCAL_HELD);
                if (other_call_id != INVALID_CALL_ID)
                {
                    app_audio_ctrl_le_update_call_state(con_lid, call_id, CALL_STATE_TRREE_WAY_HOLD_CALLING);
                }
            }
            else
            {
                app_audio_ctrl_le_update_call_state(con_lid, call_id, CALL_STATE_ACTIVE);
            }
            break;
        case AOB_CALL_STATE_LOCAL_HELD:
            if (other_call_id != INVALID_CALL_ID)
            {
                p_call_info = ble_audio_earphone_info_find_call_info(con_lid, other_call_id);
                if (p_call_info->state == AOB_CALL_STATE_ACTIVE)
                {
                    app_audio_ctrl_le_update_call_state(con_lid, call_id, CALL_STATE_TRREE_WAY_HOLD_CALLING);
                }
            }
            else
            {
                app_audio_ctrl_le_update_call_state(con_lid, call_id, CALL_STATE_LOCAL_HELD);
            }
            break;
        case AOB_CALL_STATE_REMOTE_HELD:
            app_audio_ctrl_le_update_call_state(con_lid, call_id, CALL_STATE_REMOTE_HELD);
            break;
        case AOB_CALL_STATE_LOCAL_AND_REMOTE_HELD:
            app_audio_ctrl_le_update_call_state(con_lid, call_id, CALL_STATE_LOCAL_AND_REMOTE_HELD);
            break;
        case AOB_CALL_STATE_IDLE:
            if (other_call_id == INVALID_CALL_ID)
            {
                app_audio_ctrl_le_update_call_state(con_lid, call_id, CALL_STATE_IDLE);
            }
            else
            {
                p_call_info = ble_audio_earphone_info_find_call_info(con_lid, other_call_id);
                if (p_call_info)
                {
                    app_audio_ctrl_le_update_call_state(con_lid, other_call_id, p_call_info->state);
                }
            }
            break;
        default:
            break;
    }

	app_ble_audio_sink_streaming_handle_event(con_lid, 0, call_state_curr, BLE_AUDIO_CALL_STATUS_CHANGED);

    if (NULL != p_call_cb && NULL != p_call_cb->ble_call_state_change_cb)
    {
        p_call_cb->ble_call_state_change_cb(con_lid, p_state_ind);
    }
}

static void aob_call_server_signal_strength_value_report_cb(uint8_t con_lid, uint8_t call_id, uint8_t value)
{
    LOG_I("[%s] con_lid:%d value:%d", __func__, con_lid, value);
    if (NULL != p_call_cb && NULL != p_call_cb->ble_call_srv_signal_strength_value_ind_cb)
    {
        p_call_cb->ble_call_srv_signal_strength_value_ind_cb(con_lid, call_id, value);
    }
}

static void aob_call_status_flag_features_report_cb(uint8_t con_lid, uint8_t call_id, bool inband_ring, bool silent_mode)
{
    LOG_I("[%s] con_lid:%d inband_ring:%d silent_mode:%d", __func__, con_lid, inband_ring, silent_mode);
    if (NULL != p_call_cb && NULL != p_call_cb->ble_call_status_flags_ind_cb)
    {
        p_call_cb->ble_call_status_flags_ind_cb(con_lid, call_id, inband_ring, silent_mode);
    }
}

static void aob_call_ccp_opt_supported_opcode_ind_cb(uint8_t con_lid, bool local_hold_op_supported, bool join_op_supported)
{
    LOG_I("[%s] con_lid:%d local_hold_op_supported:%d join_op_supported:%d", __func__, con_lid, local_hold_op_supported, join_op_supported);
    if (NULL != p_call_cb && NULL != p_call_cb->ble_call_ccp_opt_supported_opcode_ind_cb)
    {
        p_call_cb->ble_call_ccp_opt_supported_opcode_ind_cb(con_lid, local_hold_op_supported, join_op_supported);
    }
}

static void aob_call_terminated_ind_cb(uint8_t con_lid, uint8_t call_id, uint8_t reason)
{
    LOG_I("[%s] con_lid:%d reason:%d", __func__, con_lid, reason);
    switch (reason)
    {
        case APP_GAF_ACC_TB_TERM_REASON_SRV_END:
            break;
        case APP_GAF_ACC_TB_TERM_REASON_CLI_END:
            break;
        case APP_GAF_ACC_TB_TERM_REASON_NO_ANSWER:
            break;
        case APP_GAF_ACC_TB_TERM_REASON_REMOTE_END:
            break;
        case APP_GAF_ACC_TB_TERM_REASON_BUSY:
            break;
        default:
            break;
    }
    //disable call singal report interval
    AOB_SINGLE_CALL_INFO_T *info = ble_audio_earphone_info_find_call_info(con_lid, call_id);
    if (info)
    {
        app_acc_tbc_set_rpt_intv(con_lid, info->bearer_lid, 0, 1);
    }

    if (NULL != p_call_cb && NULL != p_call_cb->ble_call_action_result_ind_cb)
    {
        p_call_cb->ble_call_terminate_reason_ind_cb(con_lid, call_id, reason);
    }

    AOB_MOBILE_INFO_T *pMobInfo = ble_audio_earphone_info_get_mobile_info(con_lid);
    bool call_stop = true;
    if (pMobInfo)
    {
        for (uint8_t i = 0; i < AOB_COMMON_CALL_MAX_NB_IDS; i++)
        {
            /*if there is another call is incoming, we don't need abndon the focus and ringtone*/
            if (AOB_CALL_STATE_INCOMING == pMobInfo->call_env_info.single_call_info[i].state && \
                    call_id != pMobInfo->call_env_info.single_call_info[i].call_id)
            {
                call_stop = false;
            }
        }
        if (call_stop)
        {
            LOG_I("d(%d) %s abandon the focus and stop the ringtone!", con_lid, __func__);
            app_ble_audio_sink_streaming_handle_event(con_lid, 0, APP_GAF_DIRECTION_MAX, BLE_AUDIO_CALL_TERMINATE_IND);
        }
    }

    ble_audio_earphone_info_clear_call_info(con_lid, call_id);
}

static void aob_call_incoming_number_info_cb(uint8_t con_lid, uint8_t call_id, uint8_t url_len, uint8_t *url)
{
    LOG_I("[%s] con_lid:%d url_len:%d url:%p", __func__, con_lid, url_len, url);
    if (NULL != p_call_cb && NULL != p_call_cb->ble_call_action_result_ind_cb)
    {
        p_call_cb->ble_call_incoming_number_inf_ind_cb(con_lid, call_id, url_len, url);
    }
}

static void aob_call_svc_changed_ind_cb(uint8_t con_lid)
{
    LOG_I("[%s] con_lid:%d", __func__, con_lid);
    if (NULL != p_call_cb && NULL != p_call_cb->ble_call_action_result_ind_cb)
    {
        p_call_cb->ble_call_svc_changed_ind_cb(con_lid);
    }
}

static void aob_call_action_result_ind_cb(uint8_t con_lid, void *param)
{
    AOB_CALL_CLI_ACTION_RESULT_IND_T *call_action_result = (AOB_CALL_CLI_ACTION_RESULT_IND_T *)param;
    AOB_CALL_ENV_INFO_T *info = ble_audio_earphone_info_get_call_env_info(con_lid);
    if (!info)
    {
        LOG_E("[%s] call env info NULL", __func__);
        return;
    }

    LOG_I("[%s] con_lid:%d result:%d", __func__, con_lid, call_action_result->result);

    if (call_action_result->result == APP_GAF_ACC_TB_CP_NTF_RESULT_SUCCESS)
    {

    }
    else
    {
        // error handler
        switch (call_action_result->action_opcode)
        {
            case APP_GAF_ACC_TB_OPCODE_ACCEPT:
                break;
            case APP_GAF_ACC_TB_OPCODE_TERMINATE:
                break;
            case APP_GAF_ACC_TB_OPCODE_HOLD:
                break;
            case APP_GAF_ACC_TB_OPCODE_RETRIEVE:
                break;
            case APP_GAF_ACC_TB_OPCODE_ORIGINATE:
                break;
            case APP_GAF_ACC_TB_OPCODE_JOIN:
                break;
        }
    }

    info->action_ctx.is_busy = false;
    aob_call_execute_pending_action(con_lid);

    if (NULL != p_call_cb && NULL != p_call_cb->ble_call_action_result_ind_cb)
    {
        p_call_cb->ble_call_action_result_ind_cb(con_lid, call_action_result);
    }
}

static call_event_handler_t call_event_cb =
{
    .call_state_change_cb                   = aob_call_state_change_cb,
    .call_srv_signal_strength_value_ind_cb  = aob_call_server_signal_strength_value_report_cb,
    .call_status_flags_ind_cb               = aob_call_status_flag_features_report_cb,
    .call_ccp_opt_supported_opcode_ind_cb   = aob_call_ccp_opt_supported_opcode_ind_cb,
    .call_terminate_reason_ind_cb           = aob_call_terminated_ind_cb,
    .call_incoming_number_inf_ind_cb        = aob_call_incoming_number_info_cb,
    .call_svc_changed_ind_cb                = aob_call_svc_changed_ind_cb,
    .call_action_result_ind_cb              = aob_call_action_result_ind_cb,
};

void aob_call_if_init(void)
{
    p_call_cb = ble_audio_get_evt_cb();
    aob_mgr_call_evt_handler_register(&call_event_cb);
}

uint8_t aob_call_if_get_not_idle_call_by_conidx(uint8_t conidx, uint8_t *call_id)
{
    uint8_t state = AOB_CALL_STATE_IDLE;
    AOB_MOBILE_INFO_T *info = NULL;

    info = ble_audio_earphone_info_get_mobile_info(conidx);
    if (info == NULL)
    {
        return AOB_CALL_STATE_IDLE;
    }

    for (uint8_t i = 0; i < AOB_COMMON_CALL_MAX_NB_IDS; i++)
    {
        state = info->call_env_info.single_call_info[i].state;
        if (state != AOB_CALL_STATE_IDLE)
        {
            LOG_I("get not idle call state conidx:%d,call_id:%d,call_state:%d",
                  conidx, info->call_env_info.single_call_info[i].call_id, state);
            if (call_id != NULL)
            {
                *call_id = info->call_env_info.single_call_info[i].call_id;
            }
            return state;
        }
    }

    return AOB_CALL_STATE_IDLE;
}

/*****************************************************************************************************************
*
*                        Call Server API&Callback Implementation (mobilephone side)                              *
*
******************************************************************************************************************/
#ifdef AOB_MOBILE_ENABLED
#include "app_gaf_custom_api.h"
#include "ble_audio_mobile_info.h"
static void aob_call_mobile_if_call_end_handler(uint8_t bearer_lid, uint8_t call_id)
{
    // This function used to judge whether the ASE stream needs be released
    AOB_SINGLE_CALL_INFO_T *callInfo = ble_audio_mobile_info_get_call_info_by_id(bearer_lid, call_id);

    //ASSERT(callInfo, "%s null pointer,call_id:%d", __func__, call_id);
    if (!callInfo)
    {
        return;
    }

    if (AOB_CALL_STATE_ACTIVE == callInfo->state)
    {
        return;
    }

    for (uint8_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++)
    {
        aob_media_mobile_disable_all_stream_for_specifc_direction(
                                                i, APP_GAF_DIRECTION_SINK);
        aob_media_mobile_disable_all_stream_for_specifc_direction(
                                                i, APP_GAF_DIRECTION_SRC);
    }
}

void aob_call_mobile_if_remote_alert_start(uint8_t conidx, uint8_t call_id)
{
    AOB_SINGLE_CALL_INFO_T *info = ble_audio_mobile_info_get_call_info_by_id(conidx, call_id);
    if (info)
    {
        app_acc_tbs_call_action_req(info->bearer_lid, call_id,
                                    APP_GAF_ACC_TBS_ACTION_ALERT_START, APP_GAF_ACC_TB_TERM_REASON_UNSPECIFIED);
    }
}

void aob_call_mobile_if_remote_answer_call(uint8_t conidx, uint8_t call_id)
{
    AOB_SINGLE_CALL_INFO_T *info = ble_audio_mobile_info_get_call_info_by_id(conidx, call_id);
    if (info)
    {
        app_acc_tbs_call_action_req(info->bearer_lid, call_id,
                                    APP_GAF_ACC_TBS_ACTION_ANSWER, APP_GAF_ACC_TB_TERM_REASON_UNSPECIFIED);
    }
}

void aob_call_mobile_if_accept_call(uint8_t conidx, uint8_t call_id)
{
    AOB_SINGLE_CALL_INFO_T *info = ble_audio_mobile_info_get_call_info_by_id(conidx, call_id);
    if (info)
    {
        app_acc_tbs_call_action_req(info->bearer_lid, call_id,
                                    APP_GAF_ACC_TBS_ACTION_ACCEPT, APP_GAF_ACC_TB_TERM_REASON_UNSPECIFIED);
    }
}

void aob_call_mobile_if_terminate_call(uint8_t bearer_lid, uint8_t call_id)
{
    AOB_SINGLE_CALL_INFO_T *info = ble_audio_mobile_info_get_call_info_by_id(bearer_lid, call_id);
    if (info)
    {
        LOG_I("%s bearer_lid:%d call_id:%d", __func__, info->bearer_lid, call_id);
        app_acc_tbs_call_action_req(info->bearer_lid, call_id,
                                    APP_GAF_ACC_TBS_ACTION_TERMINATE, APP_GAF_ACC_TB_TERM_REASON_SRV_END);
    }
}

void aob_call_mobile_if_hold_local_call(uint8_t bearer_lid, uint8_t call_id)
{
    AOB_SINGLE_CALL_INFO_T *info = ble_audio_mobile_info_get_call_info_by_id(bearer_lid, call_id);
    if (info)
    {
        app_acc_tbs_call_action_req(info->bearer_lid, call_id,
                                    APP_GAF_ACC_TBS_ACTION_HOLD_LOCAL, APP_GAF_ACC_TB_TERM_REASON_BUSY);
    }
}

void aob_call_mobile_if_retrieve_local_call(uint8_t bearer_lid, uint8_t call_id)
{
    AOB_SINGLE_CALL_INFO_T *info = ble_audio_mobile_info_get_call_info_by_id(bearer_lid, call_id);
    TRACE(0, "%s %d bearer_lid %d call_id %d", __func__, __LINE__, bearer_lid, call_id);
    if (info)
    {
        TRACE(0, "%s %d", __func__, __LINE__);
        app_acc_tbs_call_action_req(info->bearer_lid, call_id,
                                    APP_GAF_ACC_TBS_ACTION_RETRIEVE_LOCAL, APP_GAF_ACC_TB_TERM_REASON_UNSPECIFIED);
    }
}

void aob_call_mobile_if_hold_remote_call(uint8_t bearer_lid, uint8_t call_id)
{
    AOB_SINGLE_CALL_INFO_T *info = ble_audio_mobile_info_get_call_info_by_id(bearer_lid, call_id);
    if (info)
    {
        app_acc_tbs_call_action_req(info->bearer_lid, call_id,
                                    APP_GAF_ACC_TBS_ACTION_HOLD_REMOTE, APP_GAF_ACC_TB_TERM_REASON_BUSY);
    }
}

void aob_call_mobile_if_outgoing_dial(uint8_t conidx, uint8_t *uri, uint8_t uriLen, uint8_t username_len)
{
    AOB_SINGLE_CALL_INFO_T *info = ble_audio_mobile_info_get_call_info_by_id(conidx, 0);
    if (info)
    {
        app_acc_tbs_call_outgoing_req(info->bearer_lid, uri, uriLen, username_len);
    }
}

void aob_call_mobile_join_req(uint8_t bearer_lid,  uint8_t nb_calls, uint8_t *call_ids)
{
    app_acc_tbs_call_join_req(bearer_lid, nb_calls, call_ids);
}

void aob_call_mobile_if_incoming_call(uint8_t bearer_lid, const char *from_uri, uint8_t from_urilen, const char *to_uri,
                                      uint8_t to_urilen, const char *call_username, uint8_t username_len)
{
    TRACE(2, "%s %d", __func__, __LINE__);

    char str_buf[64] = {0};
    uint8_t len1 = 0;
    uint8_t len2 = 0;
    uint8_t len3 = 0;

    if (from_uri)
    {
        len1 = strlen((const char *)from_uri);
        memcpy(str_buf, from_uri, len1);
    }
    if (to_uri)
    {
        len2 = strlen((const char *)to_uri);
        memcpy(str_buf + len1, to_uri, len2);
    }
    if (call_username)
    {
        len3 = strlen((const char *)call_username);
        memcpy(str_buf + len1 + len2, call_username, len3);
    }

    app_acc_tbs_call_incoming_req(bearer_lid, (uint8_t *)str_buf, len1, len2, len3);
}

void aob_call_mobile_set_char_val(uint8_t bearer_lid, uint8_t char_type, uint8_t *val, uint8_t len)
{
    app_acc_tbs_set_long_req(bearer_lid, char_type, val, len);
}

void aob_call_mobile_set_call_status(uint8_t bearer_lid, uint8_t status_type, bool enable)
{
    app_acc_tbs_set_status_req(bearer_lid, status_type, enable);
}
/*****************************************************************************************************************
*
*                    call callback function of mobile phone sides                                                *
*
******************************************************************************************************************/
static void aob_call_mobile_event_incoming_handler(uint8_t con_lid, uint8_t call_id)
{
    AOB_SINGLE_CALL_INFO_T *pFoundInfo = NULL;

    for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        pFoundInfo = ble_audio_mobile_get_call_info_by_state(i, AOB_CALL_STATE_IDLE);
        if (pFoundInfo)
        {
            pFoundInfo->call_id = call_id;
            pFoundInfo->state = AOB_CALL_STATE_INCOMING;
        }
    }
}

static void aob_call_mobile_event_accept_handler(uint8_t con_lid, uint8_t call_id)
{
    AOB_SINGLE_CALL_INFO_T *pFoundInfo = NULL;

    for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        pFoundInfo = ble_audio_mobile_get_call_info_by_state(i, AOB_CALL_STATE_INCOMING);
        if (pFoundInfo)
        {
            pFoundInfo->state = AOB_CALL_STATE_ACTIVE;
        }
    }
}

static void aob_call_mobile_event_outgoing_handler(uint8_t con_lid, uint8_t call_id, uint8_t bearer_lid)
{
    AOB_SINGLE_CALL_INFO_T *pFoundInfo = NULL;

    for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        pFoundInfo = ble_audio_mobile_get_call_info_by_bearer_lid(i, bearer_lid);
        if (pFoundInfo)
        {
            pFoundInfo->state = AOB_CALL_STATE_DIALING;
            pFoundInfo->call_id = call_id;
        }
    }
}

static void aob_call_mobile_event_remote_alerting_handler(uint8_t con_lid, uint8_t call_id)
{
    AOB_SINGLE_CALL_INFO_T *pFoundInfo = NULL;

    for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        pFoundInfo = ble_audio_mobile_get_call_info_by_state(i, AOB_CALL_STATE_DIALING);
        if (pFoundInfo)
        {
            pFoundInfo->state = AOB_CALL_STATE_ALERTING;
        }
    }
}

static void aob_call_mobile_event_remote_answer_handler(uint8_t con_lid, uint8_t call_id)
{
    AOB_SINGLE_CALL_INFO_T *pFoundInfo = NULL;

    for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        pFoundInfo = ble_audio_mobile_get_call_info_by_state(i, AOB_CALL_STATE_ALERTING);
        if (pFoundInfo)
        {
            pFoundInfo->state = AOB_CALL_STATE_ACTIVE;
        }
    }
}

static void aob_call_mobile_event_accept_ind_handler(uint8_t con_lid, uint8_t call_id)
{
    AOB_SINGLE_CALL_INFO_T *pFoundInfo = NULL;

    for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        pFoundInfo = ble_audio_mobile_get_call_info_by_state(i, AOB_CALL_STATE_INCOMING);
        if (pFoundInfo)
        {
            pFoundInfo->state = AOB_CALL_STATE_ACTIVE;
        }
    }
}

static void aob_call_mobile_event_outgoing_ind_handler(uint8_t con_lid, uint8_t call_id, uint8_t bearer_lid)
{
    AOB_SINGLE_CALL_INFO_T *pFoundInfo = NULL;

    for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        pFoundInfo = ble_audio_mobile_get_call_info_by_bearer_lid(i, bearer_lid);
        if (pFoundInfo)
        {
            pFoundInfo->state = AOB_CALL_STATE_DIALING;
            pFoundInfo->call_id = call_id;
        }
    }
}

static void aob_call_mobile_event_active_handler(uint8_t con_lid, uint8_t call_id)
{
    AOB_MEDIA_ASE_CFG_INFO_T ase_to_start[] =
    {
        {APP_GAF_BAP_SAMPLE_FREQ_16000, 40, APP_GAF_DIRECTION_SINK, AOB_CODEC_ID_LC3,  APP_GAF_BAP_CONTEXT_TYPE_CONVERSATIONAL},
        {APP_GAF_BAP_SAMPLE_FREQ_16000, 40, APP_GAF_DIRECTION_SRC, AOB_CODEC_ID_LC3,  APP_GAF_BAP_CONTEXT_TYPE_CONVERSATIONAL},
    };

    for (uint8_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++)
    {
        aob_media_mobile_start_stream(ase_to_start, i, true);
    }
}

static void aob_call_mobile_event_hold_local_handler(uint8_t con_lid, uint8_t call_id)
{
    AOB_SINGLE_CALL_INFO_T *pFoundInfo = NULL;

    for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        pFoundInfo = ble_audio_mobile_info_get_call_info_by_id(i, call_id);

        if (pFoundInfo)
        {
            AOB_CALL_STATE_E preState = pFoundInfo->state;
            if ((AOB_CALL_STATE_ACTIVE == preState)
                    || (AOB_CALL_STATE_INCOMING == preState))
            {
                pFoundInfo->state = AOB_CALL_STATE_LOCAL_HELD;
            }
            else if (AOB_CALL_STATE_REMOTE_HELD == preState)
            {
                pFoundInfo->state = AOB_CALL_STATE_LOCAL_AND_REMOTE_HELD;
            }
        }
    }
    if (pFoundInfo)
    {
        aob_call_mobile_if_call_end_handler(pFoundInfo->bearer_lid, call_id);
    }
}

static void aob_call_mobile_event_hold_remote_handler(uint8_t con_lid, uint8_t call_id)
{
    AOB_SINGLE_CALL_INFO_T *pFoundInfo = NULL;

    for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        pFoundInfo = ble_audio_mobile_info_get_call_info_by_id(i, call_id);

        if (pFoundInfo)
        {
            AOB_CALL_STATE_E preState = pFoundInfo->state;
            if (AOB_CALL_STATE_ACTIVE == preState)
            {
                pFoundInfo->state = AOB_CALL_STATE_REMOTE_HELD;
            }
            else if (AOB_CALL_STATE_LOCAL_HELD == preState)
            {
                pFoundInfo->state = AOB_CALL_STATE_LOCAL_AND_REMOTE_HELD;
            }
        }
    }
    aob_call_mobile_if_call_end_handler(pFoundInfo->bearer_lid, call_id);
}

static void aob_call_mobile_event_retrieve_local_handler(uint8_t con_lid, uint8_t call_id)
{
    AOB_SINGLE_CALL_INFO_T *pFoundInfo = NULL;

    for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        pFoundInfo = ble_audio_mobile_info_get_call_info_by_id(i, call_id);

        if (pFoundInfo)
        {
            AOB_CALL_STATE_E preState = pFoundInfo->state;
            if (AOB_CALL_STATE_LOCAL_AND_REMOTE_HELD == preState)
            {
                pFoundInfo->state = AOB_CALL_STATE_REMOTE_HELD;
            }
            else if (AOB_CALL_STATE_LOCAL_HELD == preState)
            {
                pFoundInfo->state = AOB_CALL_STATE_ACTIVE;
            }
        }
    }
}

static void aob_call_mobile_event_retrieve_remote_handler(uint8_t con_lid, uint8_t call_id)
{
    AOB_SINGLE_CALL_INFO_T *pFoundInfo = NULL;

    for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        pFoundInfo = ble_audio_mobile_info_get_call_info_by_id(i, call_id);

        if (pFoundInfo)
        {
            AOB_CALL_STATE_E preState = pFoundInfo->state;
            if (AOB_CALL_STATE_LOCAL_AND_REMOTE_HELD == preState)
            {
                pFoundInfo->state = AOB_CALL_STATE_LOCAL_HELD;
            }
            else if (AOB_CALL_STATE_REMOTE_HELD == preState)
            {
                pFoundInfo->state = AOB_CALL_STATE_ACTIVE;
            }
        }
    }
}

static void aob_call_mobile_event_terminate_handler(uint8_t con_lid, uint8_t call_id, uint8_t bearer_lid)
{
    //clear audio call manager table information.
    for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        ble_audio_mobile_info_clear_call_info(i, call_id);
    }
    aob_call_mobile_if_call_end_handler(bearer_lid, call_id);
}

static void aob_call_mobile_state_event_cb(uint8_t con_lid, uint8_t call_event, void *param, uint8_t status_err_code)
{
    AOB_SINGLE_CALL_INFO_T *p_call_info = (AOB_SINGLE_CALL_INFO_T *)param;
    uint8_t call_id = p_call_info->call_id;

    LOG_I("[D%d]%s, call_id:%d call_event:%d", con_lid, __func__, call_id, call_event);

    switch (call_event)
    {
        case AOB_CALL_EVENT_LOCAL_OUTGOING_REQ_CMP:
            aob_call_mobile_event_outgoing_handler(con_lid, call_id, p_call_info->bearer_lid);
            break;
        case AOB_CALL_EVENT_REMOTE_OUTGOING_REQ_IND:
            /// TODO: push outgoing cmp event to stm and move stm state to outgoing state, and wait the next state (alterting or idle)
            /// wait user action event(start alert/terminate) to trigger stm.
            /*
             * Step1: call external module interface to establish telephone services and start a timeout timer for max service
             * access time. if establishing successfully, then call start_alter action interface and move stm state to wait
             * alterting_state; otherwise failure or timer timeout, it should roll back to idle state.
             * Step2: wait APP_GAF_ACC_TBS_ACTION_ALERT_START req cmp event to trigger stm, if it completes, start a remote answer max timeout
             * time, and move stm state to alterting state and wait the next state(active or idle), wait user action event(remote answer).
             * Step3: if remote user answer, then call remote_answer interface, wait answer action cmp event, then move stm
             * state to active state.
            **/
            aob_call_mobile_event_outgoing_ind_handler(con_lid, call_id, p_call_info->bearer_lid);
            break;
        case AOB_CALL_EVENT_INCOMING_REQ_CMP:
            aob_call_mobile_event_incoming_handler(con_lid, call_id);
            break;
        case AOB_CALL_EVENT_LOCAL_ACTION_ACCEPT_CMP:
            aob_call_mobile_event_accept_handler(con_lid, call_id);
            aob_call_mobile_event_active_handler(con_lid, call_id);
            break;
        case AOB_CALL_EVENT_LOCAL_ACTION_TERMINATE_CMP:
            aob_call_mobile_event_terminate_handler(con_lid, call_id, p_call_info->bearer_lid);
            break;
        case AOB_CALL_EVENT_LOCAL_ACTION_HOLD_LOCAL_CMP:
            aob_call_mobile_event_hold_local_handler(con_lid, call_id);
            break;
        case AOB_CALL_EVENT_LOCAL_ACTION_RETRIEVE_LOCAL_CMP:
            aob_call_mobile_event_retrieve_local_handler(con_lid, call_id);
            aob_call_mobile_event_active_handler(con_lid, call_id);
            break;
        case AOB_CALL_EVENT_LOCAL_ACTION_HOLD_REMOTE_CMP:
            aob_call_mobile_event_hold_remote_handler(con_lid, call_id);
            break;
        case AOB_CALL_EVENT_LOCAL_ACTION_RETRIEVE_REMOTE_CMP:
            aob_call_mobile_event_retrieve_remote_handler(con_lid, call_id);
            aob_call_mobile_event_active_handler(con_lid, call_id);
            break;
        case AOB_CALL_EVENT_LOCAL_ACTION_ALTER_CMP:
            aob_call_mobile_event_remote_alerting_handler(con_lid, call_id);
            break;
        case AOB_CALL_EVENT_LOCAL_ACTION_ANSWER_CMP:
            aob_call_mobile_event_remote_answer_handler(con_lid, call_id);
            aob_call_mobile_event_active_handler(con_lid, call_id);
            break;
        case AOB_CALL_EVENT_LOCAL_JOIN_REQ_CMP:
            break;
        case AOB_CALL_EVENT_REMOTE_ACTION_ACCEPT_IND:
            aob_call_mobile_event_accept_ind_handler(con_lid, call_id);
            aob_call_mobile_event_active_handler(con_lid, call_id);
            break;
        case AOB_CALL_EVENT_REMOTE_ACTION_TERMINATE_IND:
            aob_call_mobile_event_terminate_handler(con_lid, call_id, p_call_info->bearer_lid);
            break;
        case AOB_CALL_EVENT_REMOTE_ACTION_HOLD_IND:
            aob_call_mobile_event_hold_local_handler(con_lid, call_id);
            break;
        case AOB_CALL_EVENT_REMOTE_ACTION_RETRIEVE_IND:
            aob_call_mobile_event_retrieve_local_handler(con_lid, call_id);
            aob_call_mobile_event_active_handler(con_lid, call_id);
            break;
        case AOB_CALL_EVENT_REMOTE_ACTION_ORIGINATE_IND:
            break;
        case AOB_CALL_EVENT_REMOTE_ACTION_JOIN_IND:
            break;
        default:
            break;
    }
}

static call_mobile_event_handler_t call_mobile_event_cb =
{
    .call_state_event_cb = aob_call_mobile_state_event_cb,
};

void aob_call_mobile_if_init(void)
{
    aob_mgr_gaf_mobile_call_evt_handler_register(&call_mobile_event_cb);
}
#endif

void aob_acc_tbc_set_rpt_intv(uint8_t con_lid, uint8_t bearer_lid, uint8_t interval, uint8_t reliable)
{
    app_acc_tbc_set_rpt_intv(con_lid, bearer_lid, interval, reliable);
}
/// @} AOB_APP
