/**
 * @file aob_cis_api.cpp
 * @author BES AI team
 * @version 0.1
 * @date 2022-04-18
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

/*****************************header include********************************/
#include "hal_trace.h"
#include "ble_app_dbg.h"
#include "plat_types.h"
#include "heap_api.h"

#include "ble_audio_earphone_info.h"
#include "ble_audio_core_api.h"

#include "app_gaf_custom_api.h"
#include "app_gaf_define.h"

#include "aob_bis_api.h"
#include "aob_cis_api.h"
#include "aob_stream_handler.h"

#ifdef BLE_AUDIO_CENTRAL_SELF_PAIRING_FEATURE_ENABLED
#include "app_ble_audio_central_stream_stm.h"
#endif

/****************************for server(earbud)*****************************/

/*********************external function declaration*************************/

/************************private macro defination***************************/
#define INVALID_PDU_SIZE                  (0xFF)
#define APP_BAP_UC_ASE_ID_MIN             (1)
#define INVALID_ASE_LID                   (0xFF)
/************************private type defination****************************/

/**********************private function declaration*************************/

/************************private variable defination************************/
static const BLE_AUD_CORE_EVT_CB_T *p_cis_cb = NULL;
static aob_cis_pdu_size_info_t cis_pdu_size_info[AOB_COMMON_MOBILE_CONNECTION_MAX] = {0};

static void aob_cis_established_cb(app_gaf_uc_srv_cis_state_ind_t *ascs_cis_established)
{
    for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        if (cis_pdu_size_info[i].con_lid == APP_GAF_INVALID_CON_LID)
        {
            cis_pdu_size_info[i].con_lid = ascs_cis_established->con_lid;
            cis_pdu_size_info[i].max_pdu_m2s = ascs_cis_established->cis_config.max_pdu_m2s;
            cis_pdu_size_info[i].max_pdu_s2m = ascs_cis_established->cis_config.max_pdu_s2m;
            break;
        }
    }


    if (ascs_cis_established->ase_lid_sink != 0XFF)
    {
        app_ble_audio_sink_streaming_handle_event(ascs_cis_established->con_lid,
                                                 ascs_cis_established->ase_lid_sink,
                                                 0,
                                                 BLE_AUDIO_CIS_CONNECTED_IND);
    }
    else
    {
        app_ble_audio_sink_streaming_handle_event(ascs_cis_established->con_lid,
                                                ascs_cis_established->ase_lid_src,
                                                0,
                                                BLE_AUDIO_CIS_CONNECTED_IND);
    }

    if (p_cis_cb && NULL != p_cis_cb->ble_cis_established)
    {
        p_cis_cb->ble_cis_established((AOB_UC_SRV_CIS_INFO_T *)ascs_cis_established);
    }
}

static void aob_cis_rejected_cb(uint16_t con_hdl, uint8_t error)
{
    if (p_cis_cb && NULL != p_cis_cb->ble_cis_rejected)
    {
        p_cis_cb->ble_cis_rejected(con_hdl, error);
    }
}

static void aob_cig_terminated_cb(uint8_t cig_id, uint8_t group_lid, uint8_t stream_lid, uint8_t reason)
{
    if (p_cis_cb && NULL != p_cis_cb->ble_cig_terminated)
    {
        p_cis_cb->ble_cig_terminated(cig_id, group_lid, stream_lid, reason);
    }
}

static void aob_ase_ntf_value_cb(uint8_t opcode, uint8_t nb_ases, uint8_t ase_lid, uint8_t rsp_code, uint8_t reason)
{
    if (p_cis_cb && NULL != p_cis_cb->ble_ase_ntf_value_cb)
    {
        p_cis_cb->ble_ase_ntf_value_cb(opcode, nb_ases, ase_lid, rsp_code, reason);
    }
}

static void aob_cis_disconnected_cb(app_gaf_uc_srv_cis_state_ind_t *ascs_cis_disconnected)
{
    if (ascs_cis_disconnected->con_lid == APP_GAF_INVALID_CON_LID)
    {
        LOG_W("%s ASE [QOS -> CODEC/IDLE] cause CIS state change will not report", __func__);
        return;
    }

    for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        if (ascs_cis_disconnected->con_lid == cis_pdu_size_info[i].con_lid)
        {
            LOG_I("%s con_lid:%d ", __func__, ascs_cis_disconnected->con_lid);
            cis_pdu_size_info[i].con_lid = APP_GAF_INVALID_CON_LID;
            cis_pdu_size_info[i].max_pdu_m2s = INVALID_PDU_SIZE;
            cis_pdu_size_info[i].max_pdu_s2m = INVALID_PDU_SIZE;
            break;
        }
    }

    if (ascs_cis_disconnected->ase_lid_sink != APP_GAF_INVALID_CON_LID)
    {
        if (p_cis_cb && NULL != p_cis_cb->ble_cis_disconnected)
        {
            p_cis_cb->ble_cis_disconnected(ascs_cis_disconnected->cig_id, ascs_cis_disconnected->cis_id,
                                           ascs_cis_disconnected->status, ascs_cis_disconnected->reason);
        }
    }
}

static cis_conn_evt_handler_t cis_conn_event_cb =
{
    .cis_established_cb             = aob_cis_established_cb,
    .cis_rejected_cb                = aob_cis_rejected_cb,
    .cig_terminated_cb              = aob_cig_terminated_cb,
    .ase_ntf_value_cb               = aob_ase_ntf_value_cb,
    .cis_disconnected_cb            = aob_cis_disconnected_cb,
};

aob_cis_pdu_size_info_t *aob_cis_get_pdu_size_info(uint8_t con_lid)
{
    aob_cis_pdu_size_info_t *p_info = &cis_pdu_size_info[0];

    for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        if (con_lid == cis_pdu_size_info[i].con_lid)
        {
            LOG_I("%s con_lid:%d  max_pdu_m2s: %d max_pdu_s2m: %d", __func__, cis_pdu_size_info[i].con_lid,
                  cis_pdu_size_info[i].max_pdu_m2s, cis_pdu_size_info[i].max_pdu_s2m);
            p_info = &cis_pdu_size_info[i];
            break;
        }
    }

    return p_info;
}

void aob_cis_api_init(void)
{
    p_cis_cb = ble_audio_get_evt_cb();
    aob_mgr_cis_conn_evt_handler_t_register(&cis_conn_event_cb);

    memset(&cis_pdu_size_info, 0, sizeof(aob_cis_pdu_size_info_t) * AOB_COMMON_MOBILE_CONNECTION_MAX);

    for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        cis_pdu_size_info[i].con_lid = APP_GAF_INVALID_CON_LID;
        cis_pdu_size_info[i].max_pdu_m2s = INVALID_PDU_SIZE;
        cis_pdu_size_info[i].max_pdu_s2m = INVALID_PDU_SIZE;
    }
}

#ifdef AOB_MOBILE_ENABLED
static aob_cis_pdu_size_info_t mobile_cis_pdu_size_info[AOB_COMMON_MOBILE_CONNECTION_MAX] = {0};

static void aob_mobile_cis_estab_cb(app_gaf_uc_cli_cis_state_ind_t *ascc_cis_established)
{
    for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        if (mobile_cis_pdu_size_info[i].con_lid == APP_GAF_INVALID_CON_LID)
        {
            LOG_I("%s con_lid:%d  max_pdu_m2s: %d max_pdu_s2m: %d", __func__, mobile_cis_pdu_size_info[i].con_lid,
                  mobile_cis_pdu_size_info[i].max_pdu_m2s, mobile_cis_pdu_size_info[i].max_pdu_s2m);
            mobile_cis_pdu_size_info[i].con_lid = ascc_cis_established->con_lid;
            mobile_cis_pdu_size_info[i].max_pdu_m2s = ascc_cis_established->cis_config.max_pdu_m2s;
            mobile_cis_pdu_size_info[i].max_pdu_s2m = ascc_cis_established->cis_config.max_pdu_s2m;
            break;
        }
    }
}

aob_cis_pdu_size_info_t *aob_cis_mobile_get_pdu_size_info(uint8_t con_lid)
{
    aob_cis_pdu_size_info_t *p_info = &mobile_cis_pdu_size_info[0];

    for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        if (con_lid == mobile_cis_pdu_size_info[i].con_lid)
        {
            LOG_I("%s con_lid:%d  max_pdu_m2s: %d max_pdu_s2m: %d", __func__, mobile_cis_pdu_size_info[i].con_lid,
                  mobile_cis_pdu_size_info[i].max_pdu_m2s, mobile_cis_pdu_size_info[i].max_pdu_s2m);
            p_info = &mobile_cis_pdu_size_info[i];
            break;
        }
    }

    return p_info;
}

static void aob_mobile_cis_discon_cb(app_gaf_uc_cli_cis_state_ind_t *ascc_cis_disconnected)
{
    for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        if (ascc_cis_disconnected->con_lid == mobile_cis_pdu_size_info[i].con_lid)
        {
            LOG_I("%s con_lid:%d ", __func__, ascc_cis_disconnected->con_lid);
            mobile_cis_pdu_size_info[i].con_lid = APP_GAF_INVALID_CON_LID;
            mobile_cis_pdu_size_info[i].max_pdu_m2s = INVALID_PDU_SIZE;
            mobile_cis_pdu_size_info[i].max_pdu_s2m = INVALID_PDU_SIZE;
            break;
        }
    }

#ifdef BLE_AUDIO_CENTRAL_SELF_PAIRING_FEATURE_ENABLED
    ble_audio_central_stream_post_cis_discon_operation_check();
#endif
}

static cis_mobile_conn_evt_handler_t mobile_cis_conn_event_cb =
{
    .mobile_cis_estab_cb             = aob_mobile_cis_estab_cb,
    .mobile_cis_discon_cb            = aob_mobile_cis_discon_cb,
};

void aob_cis_mobile_api_init(void)
{
    aob_mgr_mobile_cis_conn_evt_handler_register(&mobile_cis_conn_event_cb);

    memset(&mobile_cis_pdu_size_info, 0, sizeof(aob_cis_pdu_size_info_t) * AOB_COMMON_MOBILE_CONNECTION_MAX);

    for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        mobile_cis_pdu_size_info[i].con_lid = APP_GAF_INVALID_CON_LID;
        mobile_cis_pdu_size_info[i].max_pdu_m2s = INVALID_PDU_SIZE;
        mobile_cis_pdu_size_info[i].max_pdu_s2m = INVALID_PDU_SIZE;
    }
}
#endif