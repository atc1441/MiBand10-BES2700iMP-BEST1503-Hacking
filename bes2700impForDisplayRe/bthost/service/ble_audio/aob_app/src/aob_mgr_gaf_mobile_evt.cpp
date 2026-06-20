/**
 * @file aob_mgr_gaf_media_evt.h
 * @author BES AI team
 * @version 0.1
 * @date 2021-07-08
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
 */

/*****************************header include********************************/
#if BLE_AUDIO_ENABLED
#ifdef AOB_MOBILE_ENABLED
#include "hal_uart.h"
#include "hal_timer.h"

#include "gatt_service.h"

#include "app_ble.h"
#include "app_gaf.h"

#include "ble_app_dbg.h"
#include "ble_audio_mobile_info.h"
#include "ble_audio_core_api.h"

#include "gaf_media_pid.h"
#include "gaf_bis_media_stream.h"
#include "gaf_mobile_media_stream.h"

#include "aob_conn_api.h"
#include "aob_call_api.h"
#include "aob_mgr_gaf_evt.h"
#include "aob_media_api.h"
#include "aob_volume_api.h"
#include "aob_bis_api.h"
#include "aob_csip_api.h"
#include "aob_dtc_api.h"
#include "aob_cis_api.h"


/************************private macro defination***************************/

/************************private type defination****************************/

/************************extern function declaration************************/

/**********************private function declaration*************************/

/************************private variable defination************************/
static media_mobile_event_handler_t aob_mgr_media_mobile_evt_handler = {NULL,};
static vol_event_handler_t aob_mgr_vol_mobile_evt_handler = {NULL,};
static call_mobile_event_handler_t aob_mgr_call_mobile_evt_handler = {NULL};
static csip_mobile_event_handler_t aob_mgr_csip_mobile_evt_handler = {NULL,};
static dtc_coc_event_handler_t aob_mgr_dtc_coc_evt_handler = {NULL,};
static cis_mobile_conn_evt_handler_t aob_mgr_cis_mobile_conn_evt_handler = {NULL,};
/****************************function defination****************************/

/****************************ASCC****************************/

static void aob_mgr_gaf_ascc_cis_established_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_uc_cli_cis_state_ind_t *ascc_cis_established = (app_gaf_uc_cli_cis_state_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_ASCC_CIS_ESTABLISHED_IND, __func__);

    if (aob_mgr_cis_mobile_conn_evt_handler.mobile_cis_estab_cb)
    {
        aob_mgr_cis_mobile_conn_evt_handler.mobile_cis_estab_cb(ascc_cis_established);
    }
}

static void aob_mgr_gaf_ascc_cis_disconnected_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_uc_cli_cis_state_ind_t *ascc_cis_disconnected = (app_gaf_uc_cli_cis_state_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_ASCC_CIS_DISCONNETED_IND, __func__);

    if (aob_mgr_cis_mobile_conn_evt_handler.mobile_cis_discon_cb)
    {
        aob_mgr_cis_mobile_conn_evt_handler.mobile_cis_discon_cb(ascc_cis_disconnected);
    }
}

static void aob_mgr_gaf_ascc_cis_stream_started_ind(void *event)
{
    app_gaf_ascc_cis_stream_started_t *ascc_cis_stream_started = (app_gaf_ascc_cis_stream_started_t *)event;
    LOG_D("app_gaf event handle: %04x, %s dir %d", APP_GAF_ASCC_CIS_STREAM_STARTED_IND, __func__, ascc_cis_stream_started->direction);
    if (aob_mgr_media_mobile_evt_handler.media_stream_status_change_cb)
    {
        aob_mgr_media_mobile_evt_handler.media_stream_status_change_cb(ascc_cis_stream_started->con_lid, ascc_cis_stream_started->ase_lid, AOB_MGR_STREAM_STATE_STREAMING);
    }

    return;
}

static void aob_mgr_gaf_ascc_cis_stream_stopped_ind(void *event)
{
    app_gaf_ascc_cis_stream_stopped_t *ascc_cis_stream_stopped = (app_gaf_ascc_cis_stream_stopped_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_ASCC_CIS_STREAM_STOPPED_IND, __func__);

    if (aob_mgr_media_mobile_evt_handler.media_stream_status_change_cb)
    {
        aob_mgr_media_mobile_evt_handler.media_stream_status_change_cb(ascc_cis_stream_stopped->con_lid, ascc_cis_stream_stopped->ase_lid, AOB_MGR_STREAM_STATE_DISABLING);
    }

    return;
}

static void aob_mgr_gaf_ascc_cis_stream_changed_ind(void *event)
{
    LEA_ASE_STM_EVENT_E ase_event = LEA_ASE_EVENT_MAX;

    app_gaf_cis_stream_state_updated_ind_t *ascc_cis_stream_updated = (app_gaf_cis_stream_state_updated_ind_t *)event;
    if (aob_mgr_media_mobile_evt_handler.media_stream_status_change_cb)
    {
        aob_mgr_media_mobile_evt_handler.media_stream_status_change_cb(ascc_cis_stream_updated->con_lid,
                                                                       ascc_cis_stream_updated->ase_lid,
                                                                       (AOB_MGR_STREAM_STATE_E)ascc_cis_stream_updated->currentState);
    }
    LOG_I("%s former state: %d current state: %d", __func__, ascc_cis_stream_updated->formerState,
          ascc_cis_stream_updated->currentState);
    if (ascc_cis_stream_updated->formerState >= APP_GAF_CIS_STREAM_ENABLING &&
            ascc_cis_stream_updated->currentState == APP_GAF_CIS_STREAM_RELEASING)
    {
        ase_event = LEA_EVT_ASE_RELEASED;
    }
    else if ((ascc_cis_stream_updated->formerState <= APP_GAF_CIS_STREAM_QOS_CONFIGURED ||
              ascc_cis_stream_updated->formerState == APP_GAF_CIS_STREAM_RELEASING) &&
             ascc_cis_stream_updated->currentState == APP_GAF_CIS_STREAM_IDLE)
    {
        ase_event = LEA_EVT_ASE_IDLE;
    }
    // TODO: add disable ntf
    /*else if (ascc_cis_stream_updated->formerState >= APP_GAF_CIS_STREAM_ENABLING &&
             (ascc_cis_stream_updated->currentState == APP_GAF_CIS_STREAM_DISABLING ||
              ascc_cis_stream_updated->currentState == APP_GAF_CIS_STREAM_QOS_CONFIGURED))
    {
        ase_event = LEA_EVT_ASE_DISABLED;
    }*/
    else
    {
        return;
    }

    lea_ase_stm_send_msg(ascc_cis_stream_updated->ase_lid, ase_event, 0, 0);
}

static void aob_mgr_gaf_ascc_ase_found_ind(void *event)
{
    app_gaf_uc_cli_bond_data_ind_t *ase_ind = (app_gaf_uc_cli_bond_data_ind_t *)event;
    if (aob_mgr_media_mobile_evt_handler.media_ase_found_cb)
    {
        aob_mgr_media_mobile_evt_handler.media_ase_found_cb(ase_ind->con_lid,
                                                            (ase_ind->ascs_info.nb_ases_sink + ase_ind->ascs_info.nb_ases_src));
    }
}

static void aob_mgr_gaf_ascc_cis_grp_state_ind(void *event)
{
    app_bap_ascc_grp_state_t *p_grp_state = (app_bap_ascc_grp_state_t *)event;

    if (aob_mgr_media_mobile_evt_handler.media_cis_group_state_change_cb)
    {
        aob_mgr_media_mobile_evt_handler.media_cis_group_state_change_cb(p_grp_state->isCreate,
                                                                         p_grp_state->ase_lid, p_grp_state->status, p_grp_state->cig_grp_lid);
    }
}

static void aob_mgr_gaf_ascc_cmd_cmp_ind(void *event)
{
    app_bap_ascc_cmd_cmp_t *p_cmd_cmp = (app_bap_ascc_cmd_cmp_t *)event;
    LOG_I("(d%d)%s opCode %d status 0x%x ase_lid %d", p_cmd_cmp->con_lid, __func__,
        p_cmd_cmp->op_code, p_cmd_cmp->status, p_cmd_cmp->ase_lid);

    LEA_ASE_STM_EVENT_E ase_event = LEA_ASE_EVENT_MAX;

    switch (p_cmd_cmp->op_code)
    {
        case APP_BAP_UC_CLI_CONFIGURE_CODEC:
            ase_event = LEA_EVT_ASE_CODEC_CONFIGURED;
            break;
        case APP_BAP_UC_CLI_CONFIGURE_QOS:
            ase_event = LEA_EVT_ASE_QOS_CONFIGURED;
        break;
        case APP_BAP_UC_CLI_ENABLE:
            ase_event = LEA_EVT_ASE_ENABLED;
        break;
        case APP_BAP_UC_CLI_DISABLE:
            ase_event = LEA_EVT_ASE_DISABLED;
        break;
        case APP_BAP_UC_CLI_RELEASE:
            ase_event = LEA_EVT_ASE_RELEASED;
        break;
        default:
            goto exit;
            break;
    }

    lea_ase_stm_send_msg(p_cmd_cmp->ase_lid, ase_event, p_cmd_cmp->status, 0);

exit:
    return;
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_mobile_ascc_evt_cb_list[] =
{
    // BAP ASCC Callback Functions (For Mobile)
    {APP_GAF_ASCC_CIS_ESTABLISHED_IND,      aob_mgr_gaf_ascc_cis_established_ind},
    {APP_GAF_ASCC_CIS_DISCONNETED_IND,      aob_mgr_gaf_ascc_cis_disconnected_ind},
    {APP_GAF_ASCC_CIS_STREAM_STARTED_IND,   aob_mgr_gaf_ascc_cis_stream_started_ind},
    {APP_GAF_ASCC_CIS_STREAM_STOPPED_IND,   aob_mgr_gaf_ascc_cis_stream_stopped_ind},
    {APP_GAF_ASCC_CIS_STREAM_STATE_UPDATED, aob_mgr_gaf_ascc_cis_stream_changed_ind},
    {APP_GAF_ASCC_ASE_FOUND_IND,            aob_mgr_gaf_ascc_ase_found_ind},
    {APP_GAF_ASCC_CIS_GRP_STATE_IND,        aob_mgr_gaf_ascc_cis_grp_state_ind},
    {APP_GAF_ASCC_CMD_CMP_IND,              aob_mgr_gaf_ascc_cmd_cmp_ind},
};

/****************************PAC****************************/
static void aob_mgr_gaf_pacc_record_ind(void *event)
{
    app_gaf_capa_cli_record_ind_t *pac_record_ind = (app_gaf_capa_cli_record_ind_t *)event;
    if (aob_mgr_media_mobile_evt_handler.media_codec_capa_change_cb)
    {
        aob_mgr_media_mobile_evt_handler.media_codec_capa_change_cb((uint8_t)pac_record_ind->con_lid, pac_record_ind->codec_id.codec_id[0]);
    }
}

static void aob_mgr_gaf_pacc_loc_ind(void *event)
{
    app_gaf_capa_cli_location_ind_t *pac_loc_ind = (app_gaf_capa_cli_location_ind_t *)event;
    if (aob_mgr_media_mobile_evt_handler.media_location_cb)
    {
        aob_mgr_media_mobile_evt_handler.media_location_cb(pac_loc_ind->con_lid, (AOB_MGR_DIRECTION_E)pac_loc_ind->direction,
                                                           (AOB_MGR_LOCATION_BF_E)pac_loc_ind->location_bf);
    }
}

static void aob_mgr_gaf_pacc_discovery_cmp_ind(void *event)
{
    app_gaf_capa_operation_cmd_ind_t *op_cmp_ind = (app_gaf_capa_operation_cmd_ind_t *)event;

    if (aob_mgr_media_mobile_evt_handler.media_pac_found_cb)
    {
        aob_mgr_media_mobile_evt_handler.media_pac_found_cb(op_cmp_ind->con_lid);
    }
}

static void aob_mgr_gaf_pacc_context_ind(void *event)
{
    app_gaf_capa_cli_context_ind_t *context_ind = (app_gaf_capa_cli_context_ind_t *)event;

    if ((AOB_MGR_CONTEXT_TYPE_SUPP == context_ind->context_type) &&
            (aob_mgr_media_mobile_evt_handler.media_sup_context_changed_cb))
    {
        aob_mgr_media_mobile_evt_handler.media_sup_context_changed_cb(context_ind->con_lid,
                                                                      (AOB_MGR_CONTEXT_TYPE_BF_E)context_ind->context_bf_sink,
                                                                      (AOB_MGR_CONTEXT_TYPE_BF_E)context_ind->context_bf_src);
    }
    else if ((AOB_MGR_CONTEXT_TYPE_AVA == context_ind->context_type) &&
             (aob_mgr_media_mobile_evt_handler.media_ava_context_changed_cb))
    {
        aob_mgr_media_mobile_evt_handler.media_ava_context_changed_cb(context_ind->con_lid,
                                                                      (AOB_MGR_CONTEXT_TYPE_BF_E)context_ind->context_bf_sink,
                                                                      (AOB_MGR_CONTEXT_TYPE_BF_E)context_ind->context_bf_src);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_mobile_pacc_evt_cb_list[] =
{
    //BAP PACC
    {APP_GAF_PACC_PAC_RECORD_IND,           aob_mgr_gaf_pacc_record_ind},
    {APP_GAF_PACC_LOCATION_IND,             aob_mgr_gaf_pacc_loc_ind},
    {APP_GAF_PACC_DISCOVERY_CMP_IND,        aob_mgr_gaf_pacc_discovery_cmp_ind},
    {APP_GAF_PACC_CONTEXT_IND,              aob_mgr_gaf_pacc_context_ind},
};

/****************************MCS****************************/
void aob_mgr_gaf_mcs_control_req_ind(void *event)
{
    app_gaf_mcs_control_req_ind *p_mcs_control_req_ind = (app_gaf_mcs_control_req_ind *)event;

    if (aob_mgr_media_mobile_evt_handler.media_control_cb)
    {
        int32_t val = *(int32_t *)((uint32_t)p_mcs_control_req_ind + sizeof(*p_mcs_control_req_ind));

        aob_mgr_media_mobile_evt_handler.media_control_cb(p_mcs_control_req_ind->con_lid, p_mcs_control_req_ind->media_lid,
                                                          (AOB_MGR_MC_OPCODE_E)p_mcs_control_req_ind->opcode, val);
    }
}

void aob_mgr_gaf_mcs_val_get_req_ind(void *event)
{
    app_gaf_mcs_get_req_ind_t *p_mcs_val_get_req_ind = (app_gaf_mcs_get_req_ind_t *)event;

    if (aob_mgr_media_mobile_evt_handler.media_val_get_cb)
    {
        aob_mgr_media_mobile_evt_handler.media_val_get_cb(p_mcs_val_get_req_ind->con_lid, p_mcs_val_get_req_ind->media_lid,
                                                          p_mcs_val_get_req_ind->char_type, p_mcs_val_get_req_ind->offset);
    }
}

void aob_mgr_gaf_mcs_val_set_req_ind(void *event)
{
    app_gaf_mcs_set_req_ind_t *p_mcs_val_set_req_ind = (app_gaf_mcs_set_req_ind_t *)event;

    if (aob_mgr_media_mobile_evt_handler.media_val_set_cb)
    {
        aob_mgr_media_mobile_evt_handler.media_val_set_cb(p_mcs_val_set_req_ind->con_lid, p_mcs_val_set_req_ind->media_lid,
                                                          p_mcs_val_set_req_ind->char_type, p_mcs_val_set_req_ind->param.param);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_mobile_mcs_evt_cb_list[] =
{
    //MCP MCS
    {APP_GAF_MCS_CONTROL_REQ_IND,            aob_mgr_gaf_mcs_control_req_ind},
    {APP_GAF_MCS_SET_OBJ_ID_RI,              NULL},
    {APP_GAF_MCS_CONTROL_RI,                 NULL},
    {APP_GAF_MCS_SEARCH_RI,                  NULL},
    {APP_GAF_MCS_GET_RI,                     aob_mgr_gaf_mcs_val_get_req_ind},
    {APP_GAF_MCS_GET_POSITION_RI,            NULL},
    {APP_GAF_MCS_SET_RI,                     aob_mgr_gaf_mcs_val_set_req_ind},
};

static void aob_mgr_gaf_micc_mute_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_arc_micc_mute_ind_t *mute = (app_gaf_arc_micc_mute_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_MICC_MUTE_IND, __func__);

    if (aob_mgr_media_mobile_evt_handler.media_mic_state_cb)
    {
        aob_mgr_media_mobile_evt_handler.media_mic_state_cb(mute->mute);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_mobile_micc_evt_cb_list[] =
{
    //ARC Microphone Control Callback Function
    {APP_GAF_MICC_MUTE_IND,                  aob_mgr_gaf_micc_mute_ind},
};

/****************************VCC****************************/
void aob_mgr_gaf_vcc_vol_ind(void *event)
{
    app_gaf_vcc_volume_ind_t *p_vol_ind = (app_gaf_vcc_volume_ind_t *)event;
    uint8_t reason = AOB_VOL_CHANGED_REASON_UNKNOW;
    if (aob_mgr_vol_mobile_evt_handler.vol_changed_cb)
    {
        aob_mgr_vol_mobile_evt_handler.vol_changed_cb(p_vol_ind->con_lid, p_vol_ind->volume, p_vol_ind->mute, p_vol_ind->change_cnt, reason);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_vcc_evt_cb_list[] =
{
    // VCC event callback function
    {APP_GAF_VCC_VOLUME_IND,                 aob_mgr_gaf_vcc_vol_ind},
};

/****************************VOCC****************************/
static void aob_mgr_gaf_vocc_bond_data_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_arc_vocc_bond_data_ind_t *p_found_ind = (app_gaf_arc_vocc_bond_data_ind_t *)event;
    if (aob_mgr_vol_mobile_evt_handler.vocc_bond_data_changed_cb)
    {
        aob_mgr_vol_mobile_evt_handler.vocc_bond_data_changed_cb(p_found_ind->con_lid, p_found_ind->output_lid);
    }
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_VOCC_BOND_DATA_IND, __func__);
}

static void aob_mgr_gaf_vocc_value_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_arc_vocc_value_ind_t *p_cfg_ind = (app_gaf_arc_vocc_value_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_VOCC_VALUE_IND, __func__);

    if (aob_mgr_vol_mobile_evt_handler.vocc_offset_changed_cb)
    {
        aob_mgr_vol_mobile_evt_handler.vocc_offset_changed_cb(p_cfg_ind->con_lid, p_cfg_ind->u.val, p_cfg_ind->output_lid);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_vocc_evt_cb_list[] =
{
    // VOCC event callback function
    {APP_GAF_VOCC_BOND_DATA_IND,              aob_mgr_gaf_vocc_bond_data_ind},
    {APP_GAF_VOCC_VALUE_IND,                  aob_mgr_gaf_vocc_value_ind},
};

/****************************TBS****************************/
static void aob_call_signal_strength_change_simulator_timer_cb(void const *n);
osTimerDef(AOB_CALL_SIGNAL_STRENGTH_INTERVAL_TIMER, aob_call_signal_strength_change_simulator_timer_cb);
osTimerId  aob_call_signal_strength_timer = NULL;
typedef struct
{
    uint8_t bearer_id;
    uint8_t report_interval;
} AOB_CALL_SIGNAL_STRENGTH_SET_T;
AOB_CALL_SIGNAL_STRENGTH_SET_T aob_call_signal_set = {0};

void aob_call_signal_strength_change_simulator_timer_cb(void const *n)
{
    uint8_t signal_strength_value = 0;
    srand(hal_sys_timer_get());
    signal_strength_value = (uint8_t)(rand() % 100 + 1);
    LOG_I("aob_call_signal_strength_change_simulator_timer_cb value:%d", signal_strength_value);
    app_acc_tbs_set_req(aob_call_signal_set.bearer_id, APP_GAF_TB_CHAR_TYPE_SIGN_STRENGTH, signal_strength_value);
    osTimerStart(aob_call_signal_strength_timer, aob_call_signal_set.report_interval * 1000);
}

static void aob_mgr_gaf_tbs_bond_data_update_ind(void *event)
{
    app_gaf_acc_tbs_bond_data_ind_t  *p_cfg_ind = (app_gaf_acc_tbs_bond_data_ind_t *)event;
    LOG_D("%s con_lid %d bearer_lid %d", __func__, p_cfg_ind->con_lid, p_cfg_ind->bearer_lid);
}

static void aob_mgr_gaf_tbs_report_interval_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_tbs_report_intv_ind_t *p_found_ind = (app_gaf_acc_tbs_report_intv_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_TBS_REPORT_INTV_IND, __func__);
    if (p_found_ind->sign_strength_intv_s != 0)
    {
        if (aob_call_signal_strength_timer == NULL)
        {
            aob_call_signal_strength_timer = osTimerCreate(osTimer(AOB_CALL_SIGNAL_STRENGTH_INTERVAL_TIMER), osTimerOnce, NULL);
        }
        aob_call_signal_set.bearer_id = p_found_ind->bearer_lid;
        aob_call_signal_set.report_interval = p_found_ind->sign_strength_intv_s;
        osTimerStop(aob_call_signal_strength_timer);
        osTimerStart(aob_call_signal_strength_timer, p_found_ind->sign_strength_intv_s * 1000);
    }
    else
    {
        if (aob_call_signal_strength_timer)
        {
            osTimerStop(aob_call_signal_strength_timer);
        }
    }
}

static void aob_mgr_gaf_tbs_char_get_req_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_tbs_get_req_ind_t *p_cfg_ind = (app_gaf_acc_tbs_get_req_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_TBS_GET_RI, __func__);
}

static void aob_mgr_gaf_tbs_call_outgoing_req_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_tbs_call_out_req_ind_t *p_cfg_ind = (app_gaf_acc_tbs_call_out_req_ind_t *)event;
    LOG_I("%s, con_lid:%d, call_id:%d", __func__, p_cfg_ind->con_lid, p_cfg_ind->call_id);

    AOB_SINGLE_CALL_INFO_T aob_call_info = {0};
    aob_call_info.bearer_lid = p_cfg_ind->bearer_lid;
    aob_call_info.call_id = p_cfg_ind->call_id;
    aob_call_info.uri_len = p_cfg_ind->uri_len;
    aob_call_info.uri = (uint8_t *)bes_bt_buf_malloc(p_cfg_ind->uri_len);
    ASSERT(aob_call_info.uri, "(d%d)URI malloc failed call_id:%d, len:%d", p_cfg_ind->con_lid,
           p_cfg_ind->call_id, p_cfg_ind->uri_len);
    memcpy(aob_call_info.uri, p_cfg_ind->uri, p_cfg_ind->uri_len);
    aob_call_info.state = AOB_CALL_STATE_DIALING;

    if (aob_mgr_call_mobile_evt_handler.call_state_event_cb)
    {
        aob_mgr_call_mobile_evt_handler.call_state_event_cb(p_cfg_ind->con_lid,
                                                            AOB_CALL_EVENT_REMOTE_OUTGOING_REQ_IND, &aob_call_info, BT_STS_SUCCESS);
    }
    bes_bt_buf_free(aob_call_info.uri);
}

static void aob_mgr_gaf_tbs_call_action_req_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_tbs_call_action_req_ind_t *p_cfg_ind = (app_gaf_acc_tbs_call_action_req_ind_t *)event;
    LOG_I("con_lid:%d, call_id:%d, action:%x", p_cfg_ind->con_lid, p_cfg_ind->call_id, p_cfg_ind->opcode);

    if (aob_mgr_call_mobile_evt_handler.call_state_event_cb)
    {
        AOB_SINGLE_CALL_INFO_T aob_call_info = {0};
        aob_call_info.bearer_lid = p_cfg_ind->bearer_lid;
        aob_call_info.call_id = p_cfg_ind->call_id;

        uint8_t call_state_event = AOB_CALL_EVENT_MAX_NUM;
        switch (p_cfg_ind->opcode)
        {
            case APP_GAF_ACC_TB_OPCODE_ACCEPT:
                aob_call_info.state = AOB_CALL_STATE_ACTIVE;
                call_state_event = AOB_CALL_EVENT_REMOTE_ACTION_ACCEPT_IND;
                break;
            case APP_GAF_ACC_TB_OPCODE_TERMINATE:
                aob_call_info.state = AOB_CALL_STATE_IDLE;
                call_state_event = AOB_CALL_EVENT_REMOTE_ACTION_TERMINATE_IND;
                break;
            case APP_GAF_ACC_TB_OPCODE_HOLD:
                aob_call_info.state = AOB_CALL_STATE_LOCAL_HELD;
                call_state_event = AOB_CALL_EVENT_REMOTE_ACTION_HOLD_IND;
                break;
            case APP_GAF_ACC_TB_OPCODE_RETRIEVE:
                aob_call_info.state = AOB_CALL_STATE_ACTIVE;
                call_state_event = AOB_CALL_EVENT_REMOTE_ACTION_RETRIEVE_IND;
                break;
            case APP_GAF_ACC_TB_OPCODE_ORIGINATE:
                aob_call_info.state = AOB_CALL_STATE_DIALING;
                call_state_event = AOB_CALL_EVENT_REMOTE_ACTION_ORIGINATE_IND;
                break;
            case APP_GAF_ACC_TB_OPCODE_JOIN:
                aob_call_info.state = AOB_CALL_STATE_ACTIVE;
                call_state_event = AOB_CALL_EVENT_REMOTE_ACTION_JOIN_IND;
                break;
            default:
                call_state_event = AOB_CALL_EVENT_MAX_NUM;
                break;
        }

        aob_mgr_call_mobile_evt_handler.call_state_event_cb(p_cfg_ind->con_lid,
                                                            call_state_event, &aob_call_info, BT_STS_SUCCESS);
    }
}

static void aob_mgr_gaf_tbs_call_join_req_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_tbs_call_join_req_ind_t *p_cfg_ind = (app_gaf_acc_tbs_call_join_req_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_TBS_CALL_JOIN_RI, __func__);

    if (aob_mgr_call_mobile_evt_handler.call_state_event_cb)
    {
        AOB_SINGLE_CALL_INFO_T aob_call_info = {0};
        aob_call_info.bearer_lid = p_cfg_ind->bearer_lid;
        ASSERT(p_cfg_ind->nb_calls <= AOB_COMMON_CALL_MAX_NB_IDS, "call number exceeds the max capability!!!");
        aob_call_info.state = AOB_CALL_STATE_ACTIVE;

        aob_mgr_call_mobile_evt_handler.call_state_event_cb(p_cfg_ind->con_lid,
                                                            AOB_CALL_EVENT_REMOTE_ACTION_JOIN_IND, &aob_call_info, BT_STS_SUCCESS);
    }

}

static void aob_mgr_gaf_tbs_call_action_req_rsp(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_tbs_rsp_t *p_req_rsp = (app_gaf_acc_tbs_rsp_t *)event;

    //con_idx is set by user
    uint8_t con_lid = ble_audio_get_active_conidx();

    LOG_D("(%d)%s call_id %d", con_lid, __func__, p_req_rsp->u.call_id);

    AOB_SINGLE_CALL_INFO_T aob_call_info = {0};
    aob_call_info.bearer_lid = p_req_rsp->bearer_lid;
    aob_call_info.call_id = p_req_rsp->u.call_id;

    uint8_t call_state_event = AOB_CALL_EVENT_MAX_NUM;

    if (p_req_rsp->action == APP_GAF_ACC_TBS_ACTION_ACCEPT)
    {
        aob_call_info.state = AOB_CALL_STATE_ACTIVE;
        call_state_event = AOB_CALL_EVENT_LOCAL_ACTION_ACCEPT_CMP;
    }
    else if (p_req_rsp->action == APP_GAF_ACC_TBS_ACTION_TERMINATE)
    {
        aob_call_info.state = AOB_CALL_STATE_IDLE;
        call_state_event = AOB_CALL_EVENT_LOCAL_ACTION_TERMINATE_CMP;
    }
    else if (p_req_rsp->action == APP_GAF_ACC_TBS_ACTION_HOLD_LOCAL)
    {
        aob_call_info.state = AOB_CALL_STATE_LOCAL_HELD;
        call_state_event = AOB_CALL_EVENT_LOCAL_ACTION_HOLD_LOCAL_CMP;
    }
    else if (p_req_rsp->action == APP_GAF_ACC_TBS_ACTION_RETRIEVE_LOCAL)
    {
        aob_call_info.state = AOB_CALL_STATE_ACTIVE;
        //aob_call_info.state = AOB_CALL_STATE_REMOTE_HELD;
        call_state_event = AOB_CALL_EVENT_LOCAL_ACTION_RETRIEVE_LOCAL_CMP;
    }
    else if (p_req_rsp->action == APP_GAF_ACC_TBS_ACTION_HOLD_REMOTE)
    {
        aob_call_info.state = AOB_CALL_STATE_REMOTE_HELD;
        //aob_call_info.state = AOB_CALL_STATE_LOCAL_AND_REMOTE_HELD;
        call_state_event = AOB_CALL_EVENT_LOCAL_ACTION_HOLD_REMOTE_CMP;
    }
    else if (p_req_rsp->action == APP_GAF_ACC_TBS_ACTION_RETRIEVE_REMOTE)
    {
        aob_call_info.state = AOB_CALL_STATE_ACTIVE;
        //aob_call_info.state = AOB_CALL_STATE_LOCAL_HELD;
        call_state_event = AOB_CALL_EVENT_LOCAL_ACTION_RETRIEVE_REMOTE_CMP;
    }
    else if (p_req_rsp->action == APP_GAF_ACC_TBS_ACTION_ALERT_START)
    {
        aob_call_info.state = AOB_CALL_STATE_ALERTING;
        call_state_event = AOB_CALL_EVENT_LOCAL_ACTION_ALTER_CMP;
    }
    else if (p_req_rsp->action == APP_GAF_ACC_TBS_ACTION_ANSWER)
    {
        aob_call_info.state = AOB_CALL_STATE_ACTIVE;
        call_state_event = AOB_CALL_EVENT_LOCAL_ACTION_ANSWER_CMP;
    }

    if (aob_mgr_call_mobile_evt_handler.call_state_event_cb)
    {
        aob_mgr_call_mobile_evt_handler.call_state_event_cb(con_lid, call_state_event, &aob_call_info, p_req_rsp->status);
    }
}

static void aob_mgr_gaf_tbs_call_outgoing_req_rsp(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_tbs_rsp_t *p_req_rsp = (app_gaf_acc_tbs_rsp_t *)event;

    //con_idx is set by user
    uint8_t con_lid = ble_audio_get_active_conidx();

    LOG_I("(%d)%s call_id %d", con_lid, __func__, p_req_rsp->u.call_id);

    AOB_SINGLE_CALL_INFO_T aob_call_info = {0};
    aob_call_info.bearer_lid = p_req_rsp->bearer_lid;
    aob_call_info.call_id = p_req_rsp->u.call_id;

    aob_call_info.state = AOB_CALL_STATE_DIALING;
    uint8_t call_state_event = AOB_CALL_EVENT_LOCAL_OUTGOING_REQ_CMP;

    if (aob_mgr_call_mobile_evt_handler.call_state_event_cb)
    {
        aob_mgr_call_mobile_evt_handler.call_state_event_cb(con_lid, call_state_event, &aob_call_info, p_req_rsp->status);
    }
}

static void aob_mgr_gaf_tbs_call_incoming_req_rsp(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_tbs_rsp_t *p_req_rsp = (app_gaf_acc_tbs_rsp_t *)event;

    //con_idx is set by user
    uint8_t con_lid = ble_audio_get_active_conidx();

    LOG_I("(%d)%s call_id %d", con_lid, __func__, p_req_rsp->u.call_id);

    AOB_SINGLE_CALL_INFO_T aob_call_info = {0};
    aob_call_info.bearer_lid = p_req_rsp->bearer_lid;
    aob_call_info.call_id = p_req_rsp->u.call_id;

    aob_call_info.state = AOB_CALL_STATE_DIALING;
    uint8_t call_state_event = AOB_CALL_EVENT_LOCAL_OUTGOING_REQ_CMP;

    if (aob_mgr_call_mobile_evt_handler.call_state_event_cb)
    {
        aob_mgr_call_mobile_evt_handler.call_state_event_cb(con_lid, call_state_event, &aob_call_info, p_req_rsp->status);
    }
}

static void aob_mgr_gaf_tbs_call_join_req_rsp(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_tbs_rsp_t *p_req_rsp = (app_gaf_acc_tbs_rsp_t *)event;

    //con_idx is set by user
    uint8_t con_lid = ble_audio_get_active_conidx();

    LOG_D("(%d)%s call_id %d", con_lid, __func__, p_req_rsp->u.call_id);

    AOB_SINGLE_CALL_INFO_T aob_call_info = {0};
    aob_call_info.bearer_lid = p_req_rsp->bearer_lid;
    aob_call_info.call_id = p_req_rsp->u.call_id;

    aob_call_info.state = AOB_CALL_STATE_ACTIVE;
    uint8_t call_state_event = AOB_CALL_EVENT_LOCAL_JOIN_REQ_CMP;

    if (aob_mgr_call_mobile_evt_handler.call_state_event_cb)
    {
        aob_mgr_call_mobile_evt_handler.call_state_event_cb(con_lid, call_state_event, &aob_call_info, p_req_rsp->status);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_tbs_evt_cb_list[] =
{
    {APP_GAF_TBS_REPORT_INTV_IND,               aob_mgr_gaf_tbs_report_interval_ind},
    {APP_GAF_TBS_GET_RI,                        aob_mgr_gaf_tbs_char_get_req_ind},
    {APP_GAF_TBS_CALL_OUTGOING_RI,              aob_mgr_gaf_tbs_call_outgoing_req_ind},
    {APP_GAF_TBS_CALL_ACTION_RI,                aob_mgr_gaf_tbs_call_action_req_ind},
    {APP_GAF_TBS_CALL_JOIN_RI,                  aob_mgr_gaf_tbs_call_join_req_ind},
    {APP_GAF_TBS_BOND_DATA_IND,                 aob_mgr_gaf_tbs_bond_data_update_ind},
    {APP_GAF_TBS_CALL_ACTION_REQ_RSP,           aob_mgr_gaf_tbs_call_action_req_rsp},
    {APP_GAF_TBS_CALL_OUTGOING_REQ_RSP,         aob_mgr_gaf_tbs_call_outgoing_req_rsp},
    {APP_GAF_TBS_CALL_INCOMING_REQ_RSP,         aob_mgr_gaf_tbs_call_incoming_req_rsp},
    {APP_GAF_TBS_CALL_JOIN_REQ_RSP,             aob_mgr_gaf_tbs_call_join_req_rsp},
};

/****************************CSISC****************************/
static void aob_mgr_gaf_csisc_bond_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_csisc_bond_data_ind_t *p_req_rsp = (app_gaf_csisc_bond_data_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_CSISC_BOND_IND, __func__);
}

//sirk value report
static void aob_mgr_gaf_csisc_sirk_value_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_csisc_sirk_ind_t *p_sirk_ind = (app_gaf_csisc_sirk_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_CSISC_SIRK_VALUE_IND, __func__);

    if (aob_mgr_csip_mobile_evt_handler.csip_sirk_value_report_cb)
    {
        aob_mgr_csip_mobile_evt_handler.csip_sirk_value_report_cb(p_sirk_ind->con_lid, p_sirk_ind->sirk.sirk);
    }
}

//set size/memeber rank/memeber lock value report
static void aob_mgr_gaf_csisc_char_value_result_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_csisc_info_ind_t *p_char_value = (app_gaf_csisc_info_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_CSISC_CHAR_VALUE_RSULT_IND, __func__);
    LOG_I("con_lid:%d char_type:%d value:%d", p_char_value->con_lid, p_char_value->char_type, p_char_value->val.val);
    if (p_char_value->char_type == APP_GAF_CSIS_CHAR_TYPE_SIRK)
    {
    }
    else if (p_char_value->char_type == APP_GAF_CSIS_CHAR_TYPE_RANK)
    {
    }
    else if (p_char_value->char_type == APP_GAF_CSIS_CHAR_TYPE_LOCK)
    {
    }
}

static void aob_mgr_gaf_csisc_resolve_result_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_csisc_cmp_evt_t *p_char_value = (app_gaf_csisc_cmp_evt_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_CSISC_PSRI_RESOLVE_RESULT_IND, __func__);
    if (aob_mgr_csip_mobile_evt_handler.csip_resolve_result_report_cb)
    {
        aob_mgr_csip_mobile_evt_handler.csip_resolve_result_report_cb(p_char_value->lid.lid, p_char_value->status);
    }
}

static void aob_mgr_gaf_csisc_server_discover_done_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_csisc_discover_cmp_ind_t *p_char_value = (app_gaf_csisc_discover_cmp_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_CSISC_SERVER_INIT_DONE_CMP_IND, __func__);
    if (aob_mgr_csip_mobile_evt_handler.csip_discover_server_cmp_ind)
    {
        aob_mgr_csip_mobile_evt_handler.csip_discover_server_cmp_ind(p_char_value->con_lid, p_char_value->result);
    }
}

static void aob_mgr_gaf_csisc_sirk_add_result_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_csisc_rsp_t *p_rsp_result = (app_gaf_csisc_rsp_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_CSISC_SIRK_ADD_RESULT_IND, __func__);
    if (aob_mgr_csip_mobile_evt_handler.csip_sirk_add_result_report_cb)
    {
        aob_mgr_csip_mobile_evt_handler.csip_sirk_add_result_report_cb(p_rsp_result->lid.key_lid, p_rsp_result->status);
    }
}

static void aob_mgr_gaf_csisc_sirk_remove_result_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_csisc_rsp_t *p_rsp_result = (app_gaf_csisc_rsp_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_CSISC_SIRK_REMOVE_RESULT_IND, __func__);
    if (aob_mgr_csip_mobile_evt_handler.csip_sirk_remove_result_report_cb)
    {
        aob_mgr_csip_mobile_evt_handler.csip_sirk_remove_result_report_cb(p_rsp_result->lid.key_lid, p_rsp_result->status);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_csisc_evt_cb_list[] =
{
    {APP_GAF_CSISC_BOND_IND,                    aob_mgr_gaf_csisc_bond_ind},
    {APP_GAF_CSISC_SIRK_VALUE_IND,              aob_mgr_gaf_csisc_sirk_value_ind},
    {APP_GAF_CSISC_CHAR_VALUE_RSULT_IND,        aob_mgr_gaf_csisc_char_value_result_ind},
    {APP_GAF_CSISC_PSRI_RESOLVE_RESULT_IND,     aob_mgr_gaf_csisc_resolve_result_ind},
    {APP_GAF_CSISC_SERVER_INIT_DONE_CMP_IND,    aob_mgr_gaf_csisc_server_discover_done_ind},
    {APP_GAF_CSISC_SIRK_ADD_RESULT_IND,         aob_mgr_gaf_csisc_sirk_add_result_ind},
    {APP_GAF_CSISC_SIRK_REMOVE_RESULT_IND,      aob_mgr_gaf_csisc_sirk_remove_result_ind},
};

static void aob_mgr_gaf_dtc_coc_connected_ind(void *event)
{
    app_gaf_dtc_coc_connected_ind_t *p_coc_connected_ind = (app_gaf_dtc_coc_connected_ind_t *)event;

    LOG_D("app_gaf event handle: %04x, %s", APP_DTC_COC_CONNECTED_IND, __func__);

    if (aob_mgr_dtc_coc_evt_handler.dtc_coc_connected_cb)
    {
        aob_mgr_dtc_coc_evt_handler.dtc_coc_connected_cb(p_coc_connected_ind->con_lid,
                                                         p_coc_connected_ind->tx_mtu, p_coc_connected_ind->tx_mps,
                                                         p_coc_connected_ind->spsm);
    }
}

static void aob_mgr_gaf_dtc_coc_disconnected_ind(void *event)
{
    app_gaf_dtc_coc_disconnected_ind_t *p_coc_disconnected_ind = (app_gaf_dtc_coc_disconnected_ind_t *)event;

    LOG_D("app_gaf event handle: %04x, %s", APP_DTC_COC_DISCONNECTED_IND, __func__);

    if (aob_mgr_dtc_coc_evt_handler.dtc_coc_disconnected_cb)
    {
        aob_mgr_dtc_coc_evt_handler.dtc_coc_disconnected_cb(p_coc_disconnected_ind->con_lid, p_coc_disconnected_ind->reason,
                                                            p_coc_disconnected_ind->spsm);
    }
}

static void aob_mgr_gaf_dtc_coc_data_ind(void *event)
{
    app_gaf_dtc_coc_data_ind_t *p_data_coc_ind = (app_gaf_dtc_coc_data_ind_t *)event;

    LOG_D("app_gaf event handle: %04x, %s", APP_DTC_COC_DATA_IND, __func__);

    if (aob_mgr_dtc_coc_evt_handler.dtc_coc_data_cb)
    {
        aob_mgr_dtc_coc_evt_handler.dtc_coc_data_cb(p_data_coc_ind->con_lid, p_data_coc_ind->length, p_data_coc_ind->sdu,
                                                    p_data_coc_ind->spsm);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_dtc_evt_cb_list[] =
{
    //DTC event callback function
    {APP_DTC_COC_CONNECTED_IND,          aob_mgr_gaf_dtc_coc_connected_ind},
    {APP_DTC_COC_DISCONNECTED_IND,       aob_mgr_gaf_dtc_coc_disconnected_ind},
    {APP_DTC_COC_DATA_IND,               aob_mgr_gaf_dtc_coc_data_ind},
};

void aob_mgr_gaf_mobile_evt_handle(uint16_t id, void *event_id)
{
    uint8_t module_id = (uint8_t)GAF_ID_GET(id);
    uint16_t event = (uint16_t)GAF_EVENT_GET(id);
    aob_app_gaf_evt_cb_t *evt_cb = NULL;

    switch (module_id)
    {
        case APP_GAF_ASCC_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_mobile_ascc_evt_cb_list[0];
        }
        break;
        case APP_GAF_PACC_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_mobile_pacc_evt_cb_list[0];
        }
        break;
        case APP_GAF_MCS_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_mobile_mcs_evt_cb_list[0];
        }
        break;
        case APP_GAF_VOCC_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_vocc_evt_cb_list[0];
        }
        break;
        case APP_GAF_VCC_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_vcc_evt_cb_list[0];
        }
        break;
        case APP_GAF_MICC_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_mobile_micc_evt_cb_list[0];
        }
        break;
        case APP_GAF_TBS_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_tbs_evt_cb_list[0];
        }
        break;
        case APP_GAF_CSISC_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_csisc_evt_cb_list[0];
        }
        break;
        case APP_GAF_DTC_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_dtc_evt_cb_list[0];
        }
        break;
        default:
            break;
    }

    if ((evt_cb) && evt_cb[event].cb)
    {
        evt_cb[event].cb(event_id);
    }
}

void aob_mgr_gaf_mobile_media_evt_handler_register(media_mobile_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_media_mobile_evt_handler, handlerBundle, sizeof(media_mobile_event_handler_t));
}

void aob_mgr_gaf_mobile_vol_evt_handler_register(vol_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_vol_mobile_evt_handler, handlerBundle, sizeof(vol_event_handler_t));
}

void aob_mgr_gaf_mobile_call_evt_handler_register(call_mobile_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_call_mobile_evt_handler, handlerBundle, sizeof(call_mobile_event_handler_t));
}

void aob_mgr_gaf_mobile_csip_evt_handler_register(csip_mobile_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_csip_mobile_evt_handler, handlerBundle, sizeof(csip_mobile_event_handler_t));
}

void aob_mgr_dtc_coc_evt_handler_register(dtc_coc_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_dtc_coc_evt_handler, handlerBundle, sizeof(dtc_coc_event_handler_t));
}

void aob_mgr_mobile_cis_conn_evt_handler_register(cis_mobile_conn_evt_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_cis_mobile_conn_evt_handler, handlerBundle, sizeof(cis_mobile_conn_evt_handler_t));
}

void aob_mgr_gaf_mobile_evt_init(void)
{
    aob_csip_mobile_if_init();
    aob_conn_api_init();
    aob_media_mobile_api_init();
    aob_vol_mobile_api_init();
    aob_call_mobile_if_init();
    aob_dtc_api_init();
    aob_cis_mobile_api_init();
}
#endif
#endif

/// @} APP
