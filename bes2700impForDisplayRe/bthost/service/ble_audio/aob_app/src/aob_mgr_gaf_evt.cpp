/**
* @file aob_mgr_gaf_evt.cpp
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
#include "ble_app_dbg.h"
#include "app_gaf_custom_api.h"
#include "ble_audio_earphone_info.h"
#include "app_acc.h"
#include "aob_mgr_gaf_evt.h"
#include "bluetooth_bt_api.h"
#include "app_bt_stream.h"
#include "app_bt_func.h"
#include "aob_media_api.h"
#include "aob_call_api.h"
#include "aob_bis_api.h"
#include "aob_volume_api.h"
#include "aob_conn_api.h"
#include "ble_audio_core_api.h"
#include "ble_audio_earphone_info.h"
#include "gaf_media_pid.h"
#include "gaf_media_stream.h"

#include "aob_csip_api.h"
#include "aob_dts_api.h"
#include "aob_gatt_cache.h"
#include "aob_service_sync.h"
#include "besaud_api.h"
#include "aob_cis_api.h"
#include "aob_pacs_api.h"
#include "aob_stream_handler.h"

#include "tbs.h"
#include "bap_broadcast_assist.h"
#include "gatt_service.h"
#include "app_ble.h"
#include "app_bap_bc_assist_msg.h"

#if (BLE_AHP_SERVER_SUPPORT)
#include "gaf_non_codec_stream.h"
#endif

#ifdef AOB_UC_TEST
#include "app_ble_cmd_handler.h"
#include "cmsis_os.h"

#define TEST_READ_ISO_QUALITY_INTERVAL 6*1000*10

void aob_uc_test_get_iso_quality_report(void const *param)
{
    uint8_t ase_lid = aob_media_get_cur_streaming_ase_lid(0, AOB_MGR_DIRECTION_SINK);
    TRACE(2, "%s %04X", __func__, GET_CURRENT_MS());
    aob_media_read_iso_link_quality(ase_lid);
}

osTimerDef(AOB_UC_TEST_BUDS_ISO_QUALITY_REPORT, aob_uc_test_get_iso_quality_report);
static osTimerId aob_uc_test_buds_iso_quality_report_timer_id = NULL;
#endif

/************************private macro defination***************************/

/************************private type defination****************************/

/************************extern function declaration************************/

/**********************private function declaration*************************/

/************************private variable defination************************/
static call_event_handler_t aob_mgr_call_evt_handler = {NULL,};
static media_event_handler_t aob_mgr_media_evt_handler = {NULL,};
static vol_event_handler_t aob_mgr_vol_evt_handler = {NULL,};
static src_event_handler_t aob_mgr_src_evt_handler = {NULL,};
static assist_event_handler_t aob_mgr_assist_evt_handler = {NULL,};
static sink_event_handler_t aob_mgr_sink_evt_handler = {NULL,};
static scan_event_handler_t aob_mgr_scan_evt_handler = {NULL,};
static deleg_event_handler_t aob_mgr_deleg_evt_handler = {NULL,};
static csip_event_handler_t aob_mgr_csip_evt_handler = {NULL,};
static dts_coc_event_handler_t aob_mgr_dts_coc_evt_handler = {NULL,};
static cis_conn_evt_handler_t aob_mgr_cis_conn_evt_handler = {NULL,};
static pacs_event_handler_t aob_mgr_pacs_event_handler = {NULL,};

/****************************function defination****************************/
static void aob_mgr_ble_audio_connected_report(uint8_t con_lid)
{
    if (!app_ble_is_connection_on(con_lid))
    {
        return;
    }

    ble_bdaddr_t remote_addr = {{0}};
    const BLE_AUD_CORE_EVT_CB_T *p_cbs = ble_audio_get_evt_cb();

    LOG_I("%s with con_lid: %x", __func__, con_lid);

    bool ret = app_ble_get_peer_solved_addr(con_lid, &remote_addr);

    // Before alloc sm for ble audio, check whether sm is already exist
    if (ret == true)
    {
        // Only first connected event can trigger service restore
        if (ble_audio_get_mobile_sm_index_by_addr(&remote_addr) == 0xFF)
        {
            app_ble_audio_sink_streaming_handle_event(con_lid, 0, APP_GAF_DIRECTION_MAX, BLE_AUDIO_LE_LINK_CONNECTED_IND);
        }
    }

    if (ret && ble_audio_make_new_le_core_sm(con_lid, remote_addr.addr))
    {
        if (p_cbs && p_cbs->ble_audio_connected_cb)
        {
            p_cbs->ble_audio_connected_cb(con_lid, remote_addr.addr);
        }
    }
}

static const char *aob_call_state_to_str(uint8_t state)
{
    switch (state)
    {
        case APP_GAF_TB_CALL_STATE_INCOMING:
            return "Incoming_call";
        case APP_GAF_TB_CALL_STATE_DIALING:
            return "Dialing";
        case APP_GAF_TB_CALL_STATE_ALERTING:
            return "Alerting";
        case APP_GAF_TB_CALL_STATE_ACTIVE:
            return "Call_active";
        case APP_GAF_TB_CALL_STATE_LOC_HELD:
            return "Local_held";
        case APP_GAF_TB_CALL_STATE_REMOTE_HELD:
            return "Remote_held";
        case APP_GAF_TB_CALL_STATE_LOC_REMOTE_HELD:
            return "RL_held";
    }
    return "Unknown";
}

static void aob_mgr_gaf_ascs_cis_established_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_uc_srv_cis_state_ind_t *ascs_cis_established = (app_gaf_uc_srv_cis_state_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_ASCS_CIS_ESTABLISHED_IND, __func__);

    /// Update iso handle for gaf media
    if (APP_GAF_INVALID_ANY_LID != ascs_cis_established->ase_lid_sink)
    {
        gaf_audio_update_stream_iso_hdl(ascs_cis_established->ase_lid_sink);
    }

    if (APP_GAF_INVALID_ANY_LID != ascs_cis_established->ase_lid_src)
    {
        gaf_audio_update_stream_iso_hdl(ascs_cis_established->ase_lid_src);
    }

    if (aob_mgr_cis_conn_evt_handler.cis_established_cb)
    {
        aob_mgr_cis_conn_evt_handler.cis_established_cb(ascs_cis_established);
    }

#ifdef AOB_UC_TEST
    if (NULL == aob_uc_test_buds_iso_quality_report_timer_id)
    {
        aob_uc_test_buds_iso_quality_report_timer_id = osTimerCreate(osTimer(AOB_UC_TEST_BUDS_ISO_QUALITY_REPORT), osTimerPeriodic, NULL);
    }
    osTimerStart(aob_uc_test_buds_iso_quality_report_timer_id, TEST_READ_ISO_QUALITY_INTERVAL);
#endif
}

static void aob_mgr_gaf_ascs_cis_disconnected_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_uc_srv_cis_state_ind_t *ascs_cis_disconnected = (app_gaf_uc_srv_cis_state_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_ASCS_CIS_DISCONNETED_IND, __func__);

    if (aob_mgr_cis_conn_evt_handler.cis_disconnected_cb)
    {
        aob_mgr_cis_conn_evt_handler.cis_disconnected_cb(ascs_cis_disconnected);
    }
#ifdef AOB_UC_TEST
    if (NULL != aob_uc_test_buds_iso_quality_report_timer_id)
    {
        osTimerStop(aob_uc_test_buds_iso_quality_report_timer_id);
    }
#endif
}

static void aob_mgr_gaf_ascs_cis_stream_started_ind(void *event)
{
    app_gaf_ascs_cis_stream_started_t *ascs_cis_stream_started = (app_gaf_ascs_cis_stream_started_t *)event;

    LOG_I("[%d]%s ase_lid:%d direction:%d", ascs_cis_stream_started->con_lid, __func__,
          ascs_cis_stream_started->ase_lid, ascs_cis_stream_started->direction);

#if (BLE_AHP_SERVER_SUPPORT)
    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info(ascs_cis_stream_started->ase_lid);
    // Fetch intv and fmt ltv
    app_gaf_bap_ht_cfg_intv_ltv_t *p_ht_intv =
            (app_gaf_bap_ht_cfg_intv_ltv_t *)app_bap_get_ltv_value_by_type(&p_ase_info->p_cfg->add_cfg,
                                                                           APP_GAF_BAP_CFG_TYPE_HT_FRAME_INTV);
    app_gaf_bap_ht_cfg_fmt_ltv_t *p_ht_fmt =
            (app_gaf_bap_ht_cfg_fmt_ltv_t *)app_bap_get_ltv_value_by_type(&p_ase_info->p_cfg->add_cfg,
                                                                          APP_GAF_BAP_CFG_TYPE_HT_FRAME_FMT);

    if (APP_GAF_DIRECTION_SRC == p_ase_info->direction && p_ht_intv != NULL && p_ht_fmt != NULL)
    {
        LOG_I("HT src ase_lid %d to start gaf, intv %x, supp_fmt %x, rel_flags %x",
                                                             ascs_cis_stream_started->ase_lid,
                                                             p_ht_intv->intv, p_ht_fmt->data_format,
                                                             p_ht_fmt->related_flags);

        gaf_simulated_sensor_data_update_upstream_start(p_ase_info, p_ht_intv->intv,
                                                        ((p_ht_fmt->data_format << 8) | p_ht_fmt->related_flags));
        return;
    }
#endif

    gaf_audio_stream_update_and_start_handler(ascs_cis_stream_started->ase_lid);
    app_ble_audio_stream_start_handler(ascs_cis_stream_started->ase_lid);
}

void aob_mgr_gaf_stream_stop_handler(uint8_t ase_lid)
{
    LOG_I("%s start", __func__);
    app_ble_audio_event_t evt = BLE_AUDIO_MAX_IND;
    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info(ase_lid);
    gaf_stream_context_state_t updatedContextStreamState =
        gaf_audio_update_stream_state_info_from_ase(GAF_AUDIO_UPDATE_STREAM_INFO_TO_STOP, ase_lid);

    switch (updatedContextStreamState)
    {
        case APP_GAF_CONTEXT_ALL_STREAMS_STOPPED:
        {
            if (p_ase_info->init_context_bf & AOB_AUDIO_CONTEXT_TYPE_CONVERSATIONAL)
            {
                evt = BLE_AUDIO_CALL_ALL_STREAMS_STOP_IND;
            }
            else if (p_ase_info->init_context_bf & AOB_AUDIO_CONTEXT_TYPE_MEDIA)
            {
                evt = BLE_AUDIO_MUSIC_STREAM_STOP_IND;
            }
            else
            {
                evt = BLE_AUDIO_FLEXIBLE_ALL_STREAMS_STOP_IND;
            }
        }
        break;
        case APP_GAF_CONTEXT_CAPTURE_STREAMS_STOPPED:
        case APP_GAF_CONTEXT_PLAYBACK_STREAMS_STOPPED:
            if (p_ase_info->init_context_bf & AOB_AUDIO_CONTEXT_TYPE_CONVERSATIONAL)
            {
                if (APP_GAF_CONTEXT_CAPTURE_STREAMS_STOPPED == updatedContextStreamState)
                {
                    evt = BLE_AUDIO_CALL_CAPTURE_STREAM_STOP_IND;
                }
                else
                {
                    evt = BLE_AUDIO_CALL_PLAYBACK_STREAM_STOP_IND;
                }
            }
            else if (p_ase_info->init_context_bf & AOB_AUDIO_CONTEXT_TYPE_MEDIA)
            {
                LOG_E("%s music do not need single stream stop!!!", __func__);
                return;
            }
            else
            {
                if (APP_GAF_CONTEXT_CAPTURE_STREAMS_STOPPED == updatedContextStreamState)
                {
                    evt = BLE_AUDIO_FLEXIBLE_CAPTURE_STREAM_STOP_IND;
                }
                else
                {
                    evt = BLE_AUDIO_FLEXIBLE_PLAYBACK_STREAM_STOP_IND;
                }
            }
            break;
        default:
            return;
    }

    app_ble_audio_sink_streaming_handle_event(p_ase_info->con_lid, ase_lid, APP_GAF_DIRECTION_MAX, evt);
    LOG_D("%s end", __func__);
}

static void aob_mgr_gaf_ascs_cis_stream_stopped_ind(void *event)
{
    app_gaf_ascs_cis_stream_stopped_t *ascs_cis_stream_stopped = (app_gaf_ascs_cis_stream_stopped_t *)event;
    LOG_I("%s ase lid %d direction %d", __func__,
          ascs_cis_stream_stopped->ase_lid, ascs_cis_stream_stopped->direction);

#if (BLE_AHP_SERVER_SUPPORT)
    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info(ascs_cis_stream_stopped->ase_lid);

    // Fetch intv and fmt ltv
    app_gaf_bap_ht_cfg_intv_ltv_t *p_ht_intv =
            (app_gaf_bap_ht_cfg_intv_ltv_t *)app_bap_get_ltv_value_by_type(&p_ase_info->p_cfg->add_cfg,
                                                                           APP_GAF_BAP_CFG_TYPE_HT_FRAME_INTV);
    app_gaf_bap_ht_cfg_fmt_ltv_t *p_ht_fmt =
            (app_gaf_bap_ht_cfg_fmt_ltv_t *)app_bap_get_ltv_value_by_type(&p_ase_info->p_cfg->add_cfg,
                                                                          APP_GAF_BAP_CFG_TYPE_HT_FRAME_FMT);

    if (APP_GAF_DIRECTION_SRC == p_ase_info->direction && p_ht_intv != NULL && p_ht_fmt != NULL)
    {
        LOG_I("%s HT src ase_lid %d to start gaf", __func__, ascs_cis_stream_stopped->ase_lid);
        gaf_non_codec_upstream_stop(ascs_cis_stream_stopped->ase_lid);
        return;
    }
#endif
#ifdef HID_ULL_ENABLE
    app_bap_ascs_ase_t *p_bap_ase_ull_stream_stop_info = app_bap_uc_srv_get_ase_info(ascs_cis_stream_stopped->ase_lid);
    if (APP_GAF_DIRECTION_SRC == p_bap_ase_ull_stream_stop_info->direction
            && APP_GAF_CODEC_TYPE_ULL == p_bap_ase_ull_stream_stop_info->codec_id.codec_id[0]
            && 0x00 == p_bap_ase_ull_stream_stop_info->codec_id.codec_id[3]
            && 0x00 == p_bap_ase_ull_stream_stop_info->codec_id.codec_id[4])
    {
        gaf_ull_hid_upstream_stop(ascs_cis_stream_stopped->con_lid);
        return;
    }
#endif

    aob_mgr_gaf_stream_stop_handler(ascs_cis_stream_stopped->ase_lid);
}

static void aob_mgr_gaf_ascs_configure_codec_ri(void *event)
{
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_ASCS_CONFIGURE_CODEC_RI, __func__);
    app_gaf_uc_srv_configure_codec_req_ind_t *p_cfg_codec_ri = (app_gaf_uc_srv_configure_codec_req_ind_t *)event;
    if (aob_mgr_media_evt_handler.ase_codec_cfg_req_handler_cb)
    {
        app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info(p_cfg_codec_ri->ase_lid);
        aob_mgr_media_evt_handler.ase_codec_cfg_req_handler_cb(p_cfg_codec_ri->con_lid, p_ase_info,
                                                               p_cfg_codec_ri->tgt_latency, &p_cfg_codec_ri->codec_id,
                                                               &p_cfg_codec_ri->cfg);
    }
}

static void aob_mgr_gaf_ascs_iso_link_quality_ind(void *event)
{
    if (NULL != aob_mgr_media_evt_handler.media_iso_link_quality_cb)
    {
        aob_mgr_media_evt_handler.media_iso_link_quality_cb(event);
    }
#ifdef AOB_UC_TEST
    app_bap_uc_srv_quality_rpt_evt_t *event_p = (app_bap_uc_srv_quality_rpt_evt_t *)event;
    LOG_I("tx_unack = %04x", event_p->tx_unacked_packets);
    LOG_I("tx_flush = %04x", event_p->tx_flushed_packets);
    LOG_I("tx_last_sub = %04x", event_p->tx_last_subevent_packets);
    LOG_I("tx_rtn = %04x", event_p->retx_packets);
    LOG_I("crc_err = %04x", event_p->crc_error_packets);
    LOG_I("rx_unrx = %04x", event_p->rx_unrx_packets);
    LOG_I("rx_dulp = %04x", event_p->duplicate_packets);
#endif
}

static void aob_mgr_gaf_ascs_enable_ri(void *event)
{
    app_gaf_uc_srv_enable_req_ind_t *p_enable_ri = (app_gaf_uc_srv_enable_req_ind_t *)event;
    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info(p_enable_ri->ase_lid);
    app_ble_audio_event_t evt = BLE_AUDIO_MAX_IND;

    if (NULL == p_ase_info)
    {
        LOG_W("WARNING: %s ase lid %d context %d", __func__,
              p_enable_ri->ase_lid, p_enable_ri->metadata.param.context_bf);
        return;
    }

    LOG_I("[%d]%s ase_lid: %d context 0x%x", p_ase_info->con_lid, __func__, p_enable_ri->ase_lid, p_enable_ri->metadata.param.context_bf);

    uint16_t context_bf_ava_sink = 0;
    uint16_t context_bf_ava_src = 0;

    app_bap_capa_srv_get_ava_context_bf(p_ase_info->con_lid, &context_bf_ava_sink, &context_bf_ava_src);

    if ((p_ase_info->direction == APP_GAF_DIRECTION_SINK &&
            ((~context_bf_ava_sink) & p_enable_ri->metadata.param.context_bf) != 0) ||
            (p_ase_info->direction == APP_GAF_DIRECTION_SRC &&
             ((~context_bf_ava_src) & p_enable_ri->metadata.param.context_bf) != 0))
    {
        LOG_E("Invalid enable stream context, can not support");
        aob_media_send_enable_reject_stream_ctx_rsp(p_enable_ri->ase_lid);
    }
    else
    {
        /// Record enable req bring what kind of context during streaming
        p_ase_info->init_context_bf = p_enable_ri->metadata.param.context_bf;

        if (p_ase_info->init_context_bf & AOB_AUDIO_CONTEXT_TYPE_CONVERSATIONAL)
        {
            evt = BLE_AUDIO_CALL_ENABLE_REQ;
        }
        else if (p_ase_info->init_context_bf & AOB_AUDIO_CONTEXT_TYPE_MEDIA)
        {
            evt = BLE_AUDIO_MUSIC_ENABLE_REQ;
        }
        else if(p_ase_info->init_context_bf & AOB_AUDIO_CONTEXT_TYPE_SOUND_EFFECT)
        {
            evt = BLE_AUDIO_PROMPT_SOUND_ENABLE_REQ;
        }
        else
        {
            evt = BLE_AUDIO_FLEXIBLE_ENABLE_REQ;
        }

        app_ble_audio_sink_streaming_handle_event(p_ase_info->con_lid, p_enable_ri->ase_lid, p_ase_info->direction, evt);
    }

    if (aob_mgr_media_evt_handler.ase_enable_req_handler_cb)
    {
        aob_mgr_media_evt_handler.ase_enable_req_handler_cb(p_ase_info->con_lid, p_enable_ri->ase_lid,
                                                            &p_enable_ri->metadata);
    }
}

static void aob_mgr_gaf_ascs_update_metadata_ri(void *event)
{
    uint8_t nb_ase = 0;
    uint8_t ase_lid_list[APP_BAP_DFT_ASCS_NB_ASE_CHAR] = {0};
    uint16_t all_streaming_context = 0;
    POSSIBLY_UNUSED app_gaf_uc_srv_update_metadata_req_ind_t *p_metadata_ri = (app_gaf_uc_srv_update_metadata_req_ind_t *)event;
    app_ble_audio_event_t evt = BLE_AUDIO_MAX_IND;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_ASCS_UPDATE_METADATA_RI, __func__);

    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info(p_metadata_ri->ase_lid);

    if (NULL == p_ase_info)
    {
        LOG_W("WARNING: %s ase lid %d context %d", __func__,
              p_metadata_ri->ase_lid, p_metadata_ri->metadata.param.context_bf);
        return;
    }

    LOG_I("[%d]%s ase_lid: %d context 0x%x", p_ase_info->con_lid, __func__, p_metadata_ri->ase_lid, p_metadata_ri->metadata.param.context_bf);

    uint16_t context_bf_ava_sink = 0;
    uint16_t context_bf_ava_src = 0;

    app_bap_capa_srv_get_ava_context_bf(p_ase_info->con_lid, &context_bf_ava_sink, &context_bf_ava_src);

    if ((p_ase_info->direction == APP_GAF_DIRECTION_SINK &&
            ((~context_bf_ava_sink) & p_metadata_ri->metadata.param.context_bf) != 0) ||
            (p_ase_info->direction == APP_GAF_DIRECTION_SRC &&
             ((~context_bf_ava_src) & p_metadata_ri->metadata.param.context_bf) != 0))
    {
        LOG_E("Invalid upd md stream context, can not support");
        aob_media_send_upd_metadata_rsp(p_metadata_ri->ase_lid, false,
                                        APP_GAF_BAP_METADATA_TYPE_STREAM_CONTEXTS);
    }
    else
    {
        aob_media_send_upd_metadata_rsp(p_metadata_ri->ase_lid, true, 0);
    }

    p_ase_info->init_context_bf = p_metadata_ri->metadata.param.context_bf;

    // Get all streaming or enabling context
    nb_ase = aob_media_get_ready_for_stream_ase_lid_list(p_ase_info->con_lid, ase_lid_list);

    for (int idx = 0; idx < nb_ase; idx++)
    {
        p_ase_info = app_bap_uc_srv_get_ase_info(ase_lid_list[idx]);
        all_streaming_context |= p_ase_info->init_context_bf;
    }

    if (all_streaming_context & AOB_AUDIO_CONTEXT_TYPE_CONVERSATIONAL)
    {
        evt = BLE_AUDIO_UPDATE_CONTEXT_TYPE_TO_CALL;
    }
    else if (all_streaming_context & AOB_AUDIO_CONTEXT_TYPE_MEDIA)
    {
        evt = BLE_AUDIO_UPDATE_CONTEXT_TYPE_TO_MUSIC;
    }
    else
    {
        evt = BLE_AUDIO_UPDATE_CONTEXT_TYPE_TO_FLEXIBLE;
    }

    if (all_streaming_context != 0)
    {
        LOG_I("%s conid %d context 0x%x", __func__, p_ase_info->con_lid, all_streaming_context);
        app_ble_audio_sink_streaming_handle_event(p_ase_info->con_lid, 0, 0, evt);
    }

    p_ase_info = app_bap_uc_srv_get_ase_info(p_metadata_ri->ase_lid);

    if (aob_mgr_media_evt_handler.ase_update_metadata_req_handler_cb)
    {
        aob_mgr_media_evt_handler.ase_update_metadata_req_handler_cb(p_ase_info->con_lid, p_metadata_ri->ase_lid,
                                                                     &p_metadata_ri->metadata, p_ase_info->ase_state);
    }
}

static void aob_mgr_gaf_ascs_release_ri(void *event)
{
    app_gaf_uc_srv_release_req_ind_t *p_release_ri = (app_gaf_uc_srv_release_req_ind_t *)event;
    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info(p_release_ri->ase_lid);
    app_ble_audio_event_t evt = BLE_AUDIO_MAX_IND;

    if (NULL == p_ase_info)
    {
        LOG_W("WARNING: %s ase lid %d", __func__, p_release_ri->ase_lid);
        goto exit;
    }

    LOG_I("[%d]%s ase_lid: %d", p_ase_info->con_lid, __func__, p_ase_info->ase_lid);

    if (p_ase_info->ase_state == APP_GAF_CIS_STREAM_STREAMING ||
        p_ase_info->ase_state == APP_GAF_CIS_STREAM_ENABLING ||
        p_ase_info->ase_state == APP_GAF_CIS_STREAM_DISABLING)
    {
        if (p_ase_info->init_context_bf & AOB_AUDIO_CONTEXT_TYPE_CONVERSATIONAL)
        {
            evt = BLE_AUDIO_CALL_RELEASE_REQ;
        }
        else if (p_ase_info->init_context_bf & AOB_AUDIO_CONTEXT_TYPE_MEDIA)
        {
            evt = BLE_AUDIO_MUSIC_RELEASE_REQ;
        }
        else
        {
            evt = BLE_AUDIO_FLEXIBLE_RELEASE_REQ;
        }

        app_ble_audio_sink_streaming_handle_event(p_ase_info->con_lid, p_ase_info->ase_lid, APP_GAF_DIRECTION_MAX, evt);
    }

    if (aob_mgr_media_evt_handler.ase_release_req_handler_cb)
    {
        aob_mgr_media_evt_handler.ase_release_req_handler_cb(p_ase_info->con_lid, p_release_ri->ase_lid,
                                                             (app_gaf_bap_cfg_metadata_t *)p_ase_info->p_metadata);
    }

exit:
    return;
}

static void aob_mgr_gaf_ascs_stream_updated_ind(void *event)
{
    app_gaf_cis_stream_state_updated_ind_t *p_stream_updated = (app_gaf_cis_stream_state_updated_ind_t *)event;

    if (APP_GAF_CIS_STREAM_QOS_CONFIGURED == p_stream_updated->currentState)
    {
        app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info(p_stream_updated->ase_lid);
        if (NULL == p_ase_info)
        {
            LOG_W("WARNING: %s ase lid %d", __func__, p_stream_updated->ase_lid);
            return;
        }

        LOG_I("[%d]%s ase_lid: %d", p_ase_info->con_lid, __func__, p_ase_info->ase_lid);
        app_ble_audio_sink_streaming_handle_event(\
                                                  p_ase_info->con_lid, p_ase_info->ase_lid, APP_GAF_DIRECTION_MAX, BLE_AUDIO_CALL_QOS_CONFIG_IND);
        app_ble_audio_sink_streaming_handle_event(\
                                                  p_ase_info->con_lid, p_ase_info->ase_lid, APP_GAF_DIRECTION_MAX, BLE_AUDIO_MUSIC_QOS_CONFIG_IND);
    }

    aob_mgr_ble_audio_connected_report(p_stream_updated->con_lid);

    if (aob_mgr_media_evt_handler.media_stream_status_change_cb)
    {
        aob_mgr_media_evt_handler.media_stream_status_change_cb(
                            p_stream_updated->con_lid, p_stream_updated->ase_lid, (AOB_MGR_STREAM_STATE_E)p_stream_updated->currentState);
    }

}

static void aob_mgr_gaf_ascs_bond_data_ind(void *event)
{
    app_gaf_bap_uc_srv_bond_data_ind_t *ascs_bond_data = (app_gaf_bap_uc_srv_bond_data_ind_t *)event;
    const BLE_AUD_CORE_EVT_CB_T *p_cbs = ble_audio_get_evt_cb();

    ble_bdaddr_t remote_addr = {{0}};

    app_ble_get_peer_solved_addr(ascs_bond_data->con_lid, &remote_addr);
    aob_gattc_cache_save(remote_addr.addr, GATT_SVC_AUDIO_STREAM_CTRL, (void *)ascs_bond_data);

    aob_mgr_ble_audio_connected_report(ascs_bond_data->con_lid);

    if ((p_cbs && p_cbs->ble_ase_cp_cccd_written_cb) &&
            (ascs_bond_data->char_type == APP_GAF_BAP_UC_CHAR_TYPE_CP))
    {
        p_cbs->ble_ase_cp_cccd_written_cb(ascs_bond_data->con_lid);
    }
}

static void aob_mgr_gaf_ascs_cis_rejected_ind(void *event)
{
    app_gaf_bap_uc_srv_cis_rejected_ind_t *ascs_cis_rejected = (app_gaf_bap_uc_srv_cis_rejected_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_ASCS_CIS_REJECTED_IND, __func__);

    if (aob_mgr_cis_conn_evt_handler.cis_rejected_cb)
    {
        aob_mgr_cis_conn_evt_handler.cis_rejected_cb(ascs_cis_rejected->con_hdl, ascs_cis_rejected->error);
    }
}

static void aob_mgr_gaf_ascs_cig_terminated_ind(void *event)
{
    app_gaf_bap_uc_srv_cig_terminated_ind_t *ascs_cig_terminated = (app_gaf_bap_uc_srv_cig_terminated_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_ASCS_CIG_TERMINATED_IND, __func__);

    if (aob_mgr_cis_conn_evt_handler.cig_terminated_cb)
    {
        aob_mgr_cis_conn_evt_handler.cig_terminated_cb(ascs_cig_terminated->cig_id, ascs_cig_terminated->group_lid,
                                                       ascs_cig_terminated->stream_lid, ascs_cig_terminated->reason);
    }
}

static void aob_mgr_gaf_ascs_ase_ntf_value_ind(void *event)
{
    app_gaf_bap_uc_srv_ase_ntf_value_ind_t *ascs_ase_ntf_value = (app_gaf_bap_uc_srv_ase_ntf_value_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_ASCS_ASE_NTF_VALUE_IND, __func__);

    if (aob_mgr_cis_conn_evt_handler.ase_ntf_value_cb)
    {
        aob_mgr_cis_conn_evt_handler.ase_ntf_value_cb(ascs_ase_ntf_value->opcode, ascs_ase_ntf_value->nb_ases,
                                                      ascs_ase_ntf_value->ase_lid, ascs_ase_ntf_value->rsp_code, ascs_ase_ntf_value->reason);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_ascs_evt_cb_list[] =
{
    //BAP ASCS Callback Functions
    {APP_GAF_ASCS_CIS_ESTABLISHED_IND,      aob_mgr_gaf_ascs_cis_established_ind},
    {APP_GAF_ASCS_CIS_DISCONNETED_IND,      aob_mgr_gaf_ascs_cis_disconnected_ind},
    {APP_GAF_ASCS_CIS_STREAM_STARTED_IND,   aob_mgr_gaf_ascs_cis_stream_started_ind},
    {APP_GAF_ASCS_CIS_STREAM_STOPPED_IND,   aob_mgr_gaf_ascs_cis_stream_stopped_ind},
    {APP_GAF_ASCS_CONFIGURE_CODEC_RI,       aob_mgr_gaf_ascs_configure_codec_ri},
    {APP_GAF_ASCS_ENABLE_RI,                aob_mgr_gaf_ascs_enable_ri},
    {APP_GAF_ASCS_UPDATE_METADATA_RI,       aob_mgr_gaf_ascs_update_metadata_ri},
    {APP_GAF_ASCS_RELEASE_RI,               aob_mgr_gaf_ascs_release_ri},
    {APP_GAF_ASCS_CLI_STREAM_STATE_UPDATED, aob_mgr_gaf_ascs_stream_updated_ind},
    {APP_GAF_ASCS_ISO_LINK_QUALITY_EVT,     aob_mgr_gaf_ascs_iso_link_quality_ind},
    {APP_GAF_ASCS_BOND_DATA_IND,            aob_mgr_gaf_ascs_bond_data_ind},
    {APP_GAF_ASCS_CIS_REJECTED_IND,         aob_mgr_gaf_ascs_cis_rejected_ind},
    {APP_GAF_ASCS_CIG_TERMINATED_IND,       aob_mgr_gaf_ascs_cig_terminated_ind},
    {APP_GAF_ASCS_ASE_NTF_VALUE_IND,        aob_mgr_gaf_ascs_ase_ntf_value_ind},
};

static void aob_mgr_gaf_pacs_location_set_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_capa_srv_location_ind_t *location_set = (app_gaf_capa_srv_location_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_PACS_LOCATION_SET_IND, __func__);
}

static void aob_mgr_gaf_pacs_bond_data_ind(void *event)
{
    app_gaf_capa_srv_bond_data_ind_t *pacs_bond_data = (app_gaf_capa_srv_bond_data_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_PACS_BOND_DATA_IND, __func__);
    ble_bdaddr_t remote_addr = {{0}};
    app_ble_get_peer_solved_addr(pacs_bond_data->con_lid, &remote_addr);
    aob_gattc_cache_save(remote_addr.addr, GATT_SVC_PUBLISHED_AUDIO_CAPA, (void *)pacs_bond_data);

}

static void aob_mgr_gaf_pacs_cccd_written_ind(void *event)
{
    app_gaf_capa_srv_cccd_written_ind_t *p_cccd_written_ind = (app_gaf_capa_srv_cccd_written_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_PACS_CCCD_WRITTEN_IND, __func__);

    if (NULL != aob_mgr_pacs_event_handler.pacs_cccd_written_cb)
    {
        aob_mgr_pacs_event_handler.pacs_cccd_written_cb(p_cccd_written_ind->con_lid);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_pacs_evt_cb_list[] =
{
    //BAP PACS Callback Function
    {APP_GAF_PACS_LOCATION_SET_IND,         aob_mgr_gaf_pacs_location_set_ind},
    {APP_GAF_PACS_BOND_DATA_IND,            aob_mgr_gaf_pacs_bond_data_ind},
    {APP_GAF_PACS_CCCD_WRITTEN_IND,         aob_mgr_gaf_pacs_cccd_written_ind},
};

#ifdef APP_BLE_BIS_SRC_ENABLE
static void aob_mgr_gaf_src_enabled_ind(void *event)
{
    POSSIBLY_UNUSED app_bap_bc_src_grp_info_t *p_src_ind = (app_bap_bc_src_grp_info_t *)event;
    LOG_D("app_gaf event handle: %04x status %d, %s", APP_GAF_SRC_BIS_SRC_ENABLED_IND, p_src_ind->big_state, __func__);

    if (aob_mgr_src_evt_handler.bis_src_enabled_ind)
    {
        aob_mgr_src_evt_handler.bis_src_enabled_ind(p_src_ind);
    }
}

static void aob_mgr_gaf_src_disabled_ind(void *event)
{
    POSSIBLY_UNUSED app_bap_bc_src_grp_info_t *p_src_ind = (app_bap_bc_src_grp_info_t *)event;
    LOG_D("app_gaf event handle: %04x state %d, %s", APP_GAF_SRC_BIS_SRC_DISABLED_IND, p_src_ind->big_state, __func__);

    if (aob_mgr_src_evt_handler.bis_src_disabled_ind)
    {
        aob_mgr_src_evt_handler.bis_src_disabled_ind(p_src_ind->grp_lid);
    }
}

static void aob_mgr_gaf_src_pa_enabled_ind(void *event)
{
    POSSIBLY_UNUSED app_bap_bc_src_grp_info_t *p_src_ind = (app_bap_bc_src_grp_info_t *)event;
    LOG_D("app_gaf event handle: %04x status %d, %s", APP_GAF_SRC_BIS_PA_ENABLED_IND, p_src_ind->big_state, __func__);

    if (aob_mgr_src_evt_handler.bis_src_pa_enabled_ind)
    {
        aob_mgr_src_evt_handler.bis_src_pa_enabled_ind(p_src_ind);
    }
}

static void aob_mgr_gaf_src_pa_disabled_ind(void *event)
{
    POSSIBLY_UNUSED app_bap_bc_src_grp_info_t *p_src_ind = (app_bap_bc_src_grp_info_t *)event;
    LOG_D("app_gaf event handle: %04x status %d, %s", APP_GAF_SRC_BIS_PA_DISABLED_IND, p_src_ind->big_state, __func__);

    if (aob_mgr_src_evt_handler.bis_src_pa_disabled_ind)
    {
        aob_mgr_src_evt_handler.bis_src_pa_disabled_ind(p_src_ind->grp_lid);
    }
}

static void aob_mgr_gaf_src_bis_stream_started_ind(void *event)
{
    POSSIBLY_UNUSED app_bap_bc_src_stream_msg_t *stram_msg = (app_bap_bc_src_stream_msg_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SRC_BIS_STREAM_STARTED_IND, __func__);

    if (aob_mgr_src_evt_handler.bis_src_stream_started_cb)
    {
        aob_mgr_src_evt_handler.bis_src_stream_started_cb(stram_msg->stream_lid, stram_msg->bis_hdl);
    }
}

static void aob_mgr_gaf_src_bis_stream_stopped_ind(void *event)
{
    POSSIBLY_UNUSED app_bap_bc_src_stream_msg_t *stram_msg = (app_bap_bc_src_stream_msg_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SRC_BIS_STREAM_STOPPED_IND, __func__);

    if (aob_mgr_src_evt_handler.bis_src_stream_stoped_cb)
    {
        aob_mgr_src_evt_handler.bis_src_stream_stoped_cb(stram_msg->stream_lid, stram_msg->bis_hdl);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_mobile_src_bis_evt_cb_list[] =
{
    {APP_GAF_SRC_BIS_PA_ENABLED_IND,         aob_mgr_gaf_src_pa_enabled_ind},
    {APP_GAF_SRC_BIS_PA_DISABLED_IND,        aob_mgr_gaf_src_pa_disabled_ind},
    {APP_GAF_SRC_BIS_SRC_ENABLED_IND,        aob_mgr_gaf_src_enabled_ind},
    {APP_GAF_SRC_BIS_SRC_DISABLED_IND,       aob_mgr_gaf_src_disabled_ind},
    {APP_GAF_SRC_BIS_STREAM_STARTED_IND,     aob_mgr_gaf_src_bis_stream_started_ind},
    {APP_GAF_SRC_BIS_STREAM_STOPPED_IND,     aob_mgr_gaf_src_bis_stream_stopped_ind},
};
#endif

#ifdef APP_BLE_BIS_ASSIST_ENABLE
static void aob_mgr_gaf_assist_solicitation_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_bap_bc_assist_solicitation_ind_t *p_assist_solicitation_ind = (app_gaf_bap_bc_assist_solicitation_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_ASSIST_SOLICITATION_IND, __func__);

    if (aob_mgr_assist_evt_handler.assist_solicitation_cb)
    {
        aob_mgr_assist_evt_handler.assist_solicitation_cb(p_assist_solicitation_ind->addr_type, p_assist_solicitation_ind->addr,
                                                          p_assist_solicitation_ind->length, p_assist_solicitation_ind->adv_data);
    }
}

static void aob_mgr_gaf_assist_source_state_ind(void *event)
{
    app_gaf_bap_bc_assist_cmp_evt_t *passist_source_state_ind = (app_gaf_bap_bc_assist_cmp_evt_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_ASSIST_SOURCE_STATE_IND, __func__);

    if (aob_mgr_assist_evt_handler.assist_source_state_cb)
    {
        aob_mgr_assist_evt_handler.assist_source_state_cb(passist_source_state_ind->cmd_code, passist_source_state_ind->status,
                                                          passist_source_state_ind->con_lid, passist_source_state_ind->src_lid);
    }
}

static void aob_mgr_gaf_assist_bond_data_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_bap_bc_assist_bond_data_ind_t *p_bond_data_ind = (app_gaf_bap_bc_assist_bond_data_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_ASSIST_BOND_DATA_IND, __func__);

    if (aob_mgr_assist_evt_handler.assist_bond_data_cb)
    {
        aob_mgr_assist_evt_handler.assist_bond_data_cb(p_bond_data_ind->con_lid, p_bond_data_ind->bass_info.nb_rx_state,
                                                       p_bond_data_ind->bass_info.svc_info.shdl, p_bond_data_ind->bass_info.svc_info.ehdl);
    }
}

static void aob_mgr_gaf_assist_bacst_code_ri(void *event)
{
    POSSIBLY_UNUSED app_gaf_bap_bc_assist_bcast_code_req_ind_t *p_bcast_code_ri = (app_gaf_bap_bc_assist_bcast_code_req_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_ASSIST_BCAST_CODE_RI, __func__);

    if (aob_mgr_assist_evt_handler.assist_bcast_code_ri_cb)
    {
        aob_mgr_assist_evt_handler.assist_bcast_code_ri_cb(p_bcast_code_ri->con_lid, p_bcast_code_ri->src_lid);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_mobile_assist_evt_cb_list[] =
{
    {APP_GAF_ASSIST_SCAN_TIMEOUT_IND,         NULL},
    {APP_GAF_ASSIST_SOLICITATION_IND,         aob_mgr_gaf_assist_solicitation_ind},
    {APP_GAF_ASSIST_SOURCE_STATE_IND,         aob_mgr_gaf_assist_source_state_ind},
    {APP_GAF_ASSIST_BCAST_CODE_RI,            aob_mgr_gaf_assist_bacst_code_ri},
    {APP_GAF_ASSIST_BOND_DATA_IND,            aob_mgr_gaf_assist_bond_data_ind},
};
#endif

#if defined(APP_BLE_BIS_ASSIST_ENABLE) || defined(APP_BLE_BIS_DELEG_ENABLE) || defined(APP_BLE_BIS_SINK_ENABLE)
static void aob_mgr_gaf_scan_timeout_ind(void *event)
{
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SCAN_TIMEOUT_IND, __func__);
}

static void aob_mgr_gaf_scan_report_ind(void *event)
{
    app_gaf_bc_scan_adv_report_t *scan_adv_report = (app_gaf_bc_scan_adv_report_t *)event;

    LOG_D("app_gaf event handle: %04x, %s method %d", APP_GAF_SCAN_REPORT_IND, __func__, scan_adv_report->scan_trigger_method);

    bool choose_to_sync = false;

    aob_mgr_scan_evt_handler.scan_report_cb(&scan_adv_report->adv_report.adv_id, scan_adv_report->bcast_id.id,
                                            scan_adv_report->adv_report.data, scan_adv_report->adv_report.length,
                                            scan_adv_report->rssi, &choose_to_sync);

    if (choose_to_sync)
    {
        if (APP_GAF_BAP_BC_ASSIST_TRIGGER == scan_adv_report->scan_trigger_method)
        {
            app_gaf_bap_bc_assist_add_src_t p_src_add;
            memcpy(&p_src_add.adv_report, &scan_adv_report->adv_report, sizeof(app_gaf_extend_adv_report_t));
            p_src_add.pa_sync = APP_GAF_BAP_BC_PA_SYNC_SYNC_PAST;
            memcpy(&p_src_add.bcast_id.id, &scan_adv_report->bcast_id.id, APP_GAF_BAP_BC_BROADCAST_ID_LEN);
            p_src_add.pa_intv_frames = APP_BAP_DFT_BC_SRC_PERIODIC_INTERVAL;
            p_src_add.nb_subgroups = APP_BAP_DFT_BC_SRC_NB_SUBGRPS;
            p_src_add.bis_sync_bf = 0xFFFFFFFF; //0xFFFFFFFF No preference
#ifdef APP_BLE_BIS_ASSIST_ENABLE
            app_bap_bc_assist_source_add(&p_src_add);
#endif
        }
        else if (APP_GAF_BAP_BC_SINK_TRIGGER == scan_adv_report->scan_trigger_method)
        {
            app_bap_bc_scan_pa_sync(&scan_adv_report->adv_report.adv_id);
        }
    }
}

static void aob_mgr_gaf_scan_pa_report_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_scan_pa_report_ind_t *p_pa_report = (app_gaf_bc_scan_pa_report_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SCAN_PA_REPORT_IND, __func__);
}

static void aob_mgr_gaf_scan_pa_established_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_scan_pa_established_ind_t *p_pa_established = (app_gaf_bc_scan_pa_established_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SCAN_PA_ESTABLISHED_IND, __func__);

    if (aob_mgr_scan_evt_handler.scan_pa_established_cb)
    {
        aob_mgr_scan_evt_handler.scan_pa_established_cb(p_pa_established->pa_lid, p_pa_established->adv_addr.addr,
                                                        p_pa_established->adv_addr.addr_type, p_pa_established->adv_addr.adv_sid, p_pa_established->serv_data);
    }
}

static void aob_mgr_gaf_scan_pa_terminated_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_scan_pa_terminated_ind_t *p_pa_terminated = (app_gaf_bc_scan_pa_terminated_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SCAN_PA_TERMINATED_IND, __func__);

    if (aob_mgr_scan_evt_handler.scan_pa_terminated_cb)
    {
        aob_mgr_scan_evt_handler.scan_pa_terminated_cb(p_pa_terminated->pa_lid, p_pa_terminated->reason);
    }
}

static void aob_mgr_gaf_scan_group_report_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_scan_group_report_ind_t *p_group_report = (app_gaf_bc_scan_group_report_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SCAN_GROUP_REPORT_IND, __func__);
}

static void aob_mgr_gaf_scan_subgroup_report_ind(void *event)
{
    app_gaf_bc_scan_subgroup_report_ind_t *p_subgroup_report = (app_gaf_bc_scan_subgroup_report_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SCAN_SUBGROUP_REPORT_IND, __func__);

    if (aob_mgr_scan_evt_handler.scan_subgrp_report_cb)
    {
        aob_mgr_scan_evt_handler.scan_subgrp_report_cb(p_subgroup_report);
    }
}

static void aob_mgr_gaf_scan_stream_report_ind(void *event)
{
    app_gaf_bc_scan_stream_report_ind_t *p_stream_report = (app_gaf_bc_scan_stream_report_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SCAN_STREAM_REPORT_IND, __func__);

    if (aob_mgr_scan_evt_handler.scan_stream_report_cb)
    {
        aob_mgr_scan_evt_handler.scan_stream_report_cb(p_stream_report);
    }
}

static void aob_mgr_gaf_scan_biginfo_report_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_scan_big_info_report_ind_t *p_big_info_report = (app_gaf_bc_scan_big_info_report_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SCAN_BIGINFO_REPORT_IND, __func__);
    LOG_D("Big id %d info:", p_big_info_report->pa_lid);
    LOG_D("SDU interval %d us ISO interval %d us", p_big_info_report->report.sdu_interval,
          (uint32_t)(p_big_info_report->report.iso_interval * 1.25 * 1000));
    LOG_D("Includes %d bis, NSE %d BN %d", p_big_info_report->report.num_bis,
          p_big_info_report->report.nse, p_big_info_report->report.bn);
    LOG_D("PTO %d, IRC %d PHY %d", p_big_info_report->report.pto,
          p_big_info_report->report.irc, p_big_info_report->report.phy);
    LOG_D("framing %d, encrypted %d", p_big_info_report->report.framing,
          p_big_info_report->report.encrypted);

    if (aob_mgr_scan_evt_handler.scan_big_info_report_cb)
    {
        aob_mgr_scan_evt_handler.scan_big_info_report_cb(p_big_info_report);
    }
}

static void aob_mgr_gaf_scan_pa_sync_req_ind(void *event)
{
    app_gaf_bc_scan_pa_synchronize_req_ind_t *p_scan_pa_sync_req = (app_gaf_bc_scan_pa_synchronize_req_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SCAN_PA_SYNC_REQ_IND, __func__);

    if (aob_mgr_scan_evt_handler.scan_pa_sync_req_cb)
    {
        aob_mgr_scan_evt_handler.scan_pa_sync_req_cb(p_scan_pa_sync_req->pa_lid);
    }
}

static void aob_mgr_gaf_scan_pa_terminated_req_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_scan_pa_terminate_req_ind_t *p_scan_pa_terminated_req = (app_gaf_bc_scan_pa_terminate_req_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SCAN_PA_TERMINATED_REQ_IND, __func__);

    if (aob_mgr_scan_evt_handler.scan_pa_terminate_req_cb)
    {
        aob_mgr_scan_evt_handler.scan_pa_terminate_req_cb(p_scan_pa_terminated_req->pa_lid);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_bis_scan_evt_cb_list[] =
{
    //BAP BIS Scan Callback Functions
    {APP_GAF_SCAN_TIMEOUT_IND,              aob_mgr_gaf_scan_timeout_ind},
    {APP_GAF_SCAN_REPORT_IND,               aob_mgr_gaf_scan_report_ind},
    {APP_GAF_SCAN_PA_REPORT_IND,            aob_mgr_gaf_scan_pa_report_ind},
    {APP_GAF_SCAN_PA_ESTABLISHED_IND,       aob_mgr_gaf_scan_pa_established_ind},
    {APP_GAF_SCAN_PA_TERMINATED_IND,        aob_mgr_gaf_scan_pa_terminated_ind},
    {APP_GAF_SCAN_GROUP_REPORT_IND,         aob_mgr_gaf_scan_group_report_ind},
    {APP_GAF_SCAN_SUBGROUP_REPORT_IND,      aob_mgr_gaf_scan_subgroup_report_ind},
    {APP_GAF_SCAN_STREAM_REPORT_IND,        aob_mgr_gaf_scan_stream_report_ind},
    {APP_GAF_SCAN_BIGINFO_REPORT_IND,       aob_mgr_gaf_scan_biginfo_report_ind},
    {APP_GAF_SCAN_PA_SYNC_REQ_IND,          aob_mgr_gaf_scan_pa_sync_req_ind},
    {APP_GAF_SCAN_PA_TERMINATED_REQ_IND,    aob_mgr_gaf_scan_pa_terminated_req_ind},
};
#endif

#ifdef APP_BLE_BIS_SINK_ENABLE
static void aob_mgr_gaf_sink_bis_status_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_sink_status_ind_t *p_sink_ind = (app_gaf_bc_sink_status_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s, nb of bis %d", APP_GAF_SINK_BIS_STATUS_IND, __func__, p_sink_ind->nb_bis);

    AOB_BIS_GROUP_INFO_T *bis_gropu_info_p = ble_audio_earphone_info_get_bis_group_info();

    if (APP_GAF_BAP_BC_SINK_ESTABLISHED == p_sink_ind->state)
    {
        uint8_t stream_lid = 0;
        uint32_t stream_pos_bf = p_sink_ind->stream_pos_bf;
        /// Set stream pos bf that local synced
        ble_audio_earphone_info_set_bis_stream_sink_pos_bf(stream_pos_bf);
        // Pointer to first stream pos
        uint8_t stream_pos = co_ctz(stream_pos_bf);
        uint16_t *bis_hdl = &p_sink_ind->conhdl[0];

        memset(&bis_gropu_info_p->bis_hdl[0], 0xFF, sizeof(bis_gropu_info_p->bis_hdl));

        while (stream_pos_bf != 0)
        {
            stream_lid = ble_audio_earphone_info_bis_stream_pos_2_stream_lid(stream_pos);

            if (stream_lid >= sizeof(bis_gropu_info_p->bis_hdl) / sizeof(bis_gropu_info_p->bis_hdl[0]))
            {
                break;
            }

            LOG_I("app_gaf bc stream_lid = %d, conhdl = 0x%04x", stream_lid, *bis_hdl);
            bis_gropu_info_p->bis_hdl[stream_lid] = *bis_hdl;

            stream_pos_bf &= ~CO_BIT_MASK(stream_pos);
            stream_pos = co_ctz(stream_pos_bf);
            // Move to next bis handle
            bis_hdl++;
        }
    }

    if (aob_mgr_sink_evt_handler.bis_sink_state_cb)
    {
        aob_mgr_sink_evt_handler.bis_sink_state_cb(p_sink_ind->grp_lid, p_sink_ind->state, p_sink_ind->stream_pos_bf);
    }
}

static void aob_mgr_gaf_sink_bis_stream_started_ind(void *event)
{
    uint8_t bis_idx = *(uint8_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SINK_BIS_STREAM_STARTED_IND, __func__);

    /// BES BIS sink stream start callback
    if (aob_mgr_sink_evt_handler.bis_sink_stream_started_cb)
    {
        aob_mgr_sink_evt_handler.bis_sink_stream_started_cb(bis_idx);
    }
}

static void aob_mgr_gaf_sink_bis_stream_stopped_ind(void *event)
{
    uint8_t bis_idx = *(uint8_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SINK_BIS_STREAM_STOPPED_IND, __func__);

    /// BES BIS sink stream stop callback
    if (aob_mgr_sink_evt_handler.bis_sink_stream_stoped_cb)
    {
        aob_mgr_sink_evt_handler.bis_sink_stream_stoped_cb(bis_idx);
    }
}

static void aob_mgr_gaf_sink_bis_sink_enabled_ind(void *event)
{
    uint8_t grp_lid = *(uint8_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SINK_BIS_SINK_ENABLED_IND, __func__);

    /// BES BIS sink enabled callback
    if (aob_mgr_sink_evt_handler.bis_sink_enabled_cb)
    {
        aob_mgr_sink_evt_handler.bis_sink_enabled_cb(grp_lid);
    }
}

static void aob_mgr_gaf_sink_bis_sink_disabled_ind(void *event)
{
    uint8_t grp_lid = *(uint8_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SINK_BIS_SINK_DISABLED_IND, __func__);

    /// BES BIS sink disabled callback
    if (aob_mgr_sink_evt_handler.bis_sink_disabled_cb)
    {
        aob_mgr_sink_evt_handler.bis_sink_disabled_cb(grp_lid);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_bis_sink_evt_cb_list[] =
{
    //BAP BIS Sink Callback Functions
    {APP_GAF_SINK_BIS_STATUS_IND,           aob_mgr_gaf_sink_bis_status_ind},
    {APP_GAF_SINK_BIS_SINK_ENABLED_IND,     aob_mgr_gaf_sink_bis_sink_enabled_ind},
    {APP_GAF_SINK_BIS_SINK_DISABLED_IND,    aob_mgr_gaf_sink_bis_sink_disabled_ind},
    {APP_GAF_SINK_BIS_STREAM_STARTED_IND,   aob_mgr_gaf_sink_bis_stream_started_ind},
    {APP_GAF_SINK_BIS_STREAM_STOPPED_IND,   aob_mgr_gaf_sink_bis_stream_stopped_ind},
};
#endif

#ifdef APP_BLE_BIS_DELEG_ENABLE
static void aob_mgr_gaf_deleg_solicite_started_ind(void *event)
{
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_DELEG_SOLICITE_STARTED_IND, __func__);

    /// BES BIS deleg solicite started callback
    if (aob_mgr_deleg_evt_handler.deleg_solicite_started_cb)
    {
        aob_mgr_deleg_evt_handler.deleg_solicite_started_cb();
    }
}

static void aob_mgr_gaf_deleg_solicite_stopped_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_deleg_solicite_stopped_ind_t *reason = (app_gaf_bc_deleg_solicite_stopped_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_DELEG_SOLICITE_STOPPED_IND, __func__);

    /// BES BIS deleg solicite stop callback
    if (aob_mgr_deleg_evt_handler.deleg_solicite_stoped_cb)
    {
        aob_mgr_deleg_evt_handler.deleg_solicite_stoped_cb();
    }
}

static void aob_mgr_gaf_deleg_remote_scan_started_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_deleg_bond_remote_scan_ind_t *deleg_remote_scan_started = (app_gaf_bc_deleg_bond_remote_scan_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_DELEG_REMOTE_SCAN_STARTED_IND, __func__);
}

static void aob_mgr_gaf_deleg_remote_scan_stopped_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_deleg_bond_remote_scan_ind_t *deleg_remote_scan_stopped = (app_gaf_bc_deleg_bond_remote_scan_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_DELEG_REMOTE_SCAN_STOPPED_IND, __func__);
}

static void aob_mgr_gaf_deleg_source_add_ri(void *event)
{
    app_gaf_bc_deleg_source_add_req_ind_t *p_source_add_ri = (app_gaf_bc_deleg_source_add_req_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_DELEG_SOURCE_ADD_RI, __func__);

    /// BES BIS sink deleg add source
    if (aob_mgr_deleg_evt_handler.deleg_source_add_ri_cb)
    {
        aob_mgr_deleg_evt_handler.deleg_source_add_ri_cb(p_source_add_ri->src_lid, \
                                                         &p_source_add_ri->bcast_id.id[0], p_source_add_ri->con_lid,
                                                         p_source_add_ri->pa_sync_req);
    }
}

static void aob_mgr_gaf_deleg_source_remove_ri(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_deleg_source_remove_req_ind_t *p_source_remove_ri = (app_gaf_bc_deleg_source_remove_req_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_DELEG_SOURCE_REMOVE_RI, __func__);

    /// BES BIS sink deleg remove source
    if (aob_mgr_deleg_evt_handler.deleg_source_remove_ri_cb)
    {
        aob_mgr_deleg_evt_handler.deleg_source_remove_ri_cb(p_source_remove_ri->src_lid, p_source_remove_ri->con_lid);
    }
}

static void aob_mgr_gaf_deleg_source_update_ri(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_deleg_source_update_req_ind_t *p_source_update_ri = (app_gaf_bc_deleg_source_update_req_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_DELEG_SOURCE_UPDATE_RI, __func__);

    /// BES BIS deleg update source
    if (aob_mgr_deleg_evt_handler.deleg_source_update_ri_cb)
    {
        aob_mgr_deleg_evt_handler.deleg_source_update_ri_cb(p_source_update_ri->src_lid, p_source_update_ri->con_lid,
                                                            p_source_update_ri->pa_sync_req);
    }
}

static void aob_mgr_gaf_deleg_bond_data_ind(void *event)
{
    app_gaf_bc_deleg_bond_data_ind_t *p_bond_data = (app_gaf_bc_deleg_bond_data_ind_t *)event;
    ble_bdaddr_t remote_addr = {{0}};
    app_ble_get_peer_solved_addr(p_bond_data->con_lid, &remote_addr);
    aob_gattc_cache_save(remote_addr.addr, GATT_SVC_BCAST_AUDIO_SCAN, (void *) & (p_bond_data->cli_cfg_bf));
}

static void aob_mgr_gaf_deleg_upper_pref_bis_sync_ri(void *event)
{
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_DELEG_PREF_BIS_SYNC_RI, __func__);

    uint8_t src_lid = (uint8_t)((*(uint16_t *)event) >> 8);
    uint8_t con_lid = (uint8_t)((*(uint16_t *)event) & 0xFF);

    if (aob_mgr_deleg_evt_handler.deleg_pref_bis_sync_ri_cb)
    {
        aob_mgr_deleg_evt_handler.deleg_pref_bis_sync_ri_cb(src_lid, con_lid);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_deleg_evt_cb_list[] =
{
    //BAP BIS Delegator Callback Functions
    {APP_GAF_DELEG_SOLICITE_STARTED_IND,    aob_mgr_gaf_deleg_solicite_started_ind},
    {APP_GAF_DELEG_SOLICITE_STOPPED_IND,    aob_mgr_gaf_deleg_solicite_stopped_ind},
    {APP_GAF_DELEG_REMOTE_SCAN_STARTED_IND, aob_mgr_gaf_deleg_remote_scan_started_ind},
    {APP_GAF_DELEG_REMOTE_SCAN_STOPPED_IND, aob_mgr_gaf_deleg_remote_scan_stopped_ind},
    {APP_GAF_DELEG_SOURCE_ADD_RI,           aob_mgr_gaf_deleg_source_add_ri},
    {APP_GAF_DELEG_SOURCE_REMOVE_RI,        aob_mgr_gaf_deleg_source_remove_ri},
    {APP_GAF_DELEG_SOURCE_UPDATE_RI,        aob_mgr_gaf_deleg_source_update_ri},
    {APP_GAF_DELEG_BOND_DATA_IND,           aob_mgr_gaf_deleg_bond_data_ind},
    {APP_GAF_DELEG_PREF_BIS_SYNC_RI,        aob_mgr_gaf_deleg_upper_pref_bis_sync_ri},
};
#endif

static void aob_mgr_gaf_mcc_svc_discoveryed_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_mcc_cmp_evt_t *mcc_svc_discoveryed = (app_gaf_acc_mcc_cmp_evt_t *)event;
    LOG_I("app_gaf event handle: %04x, %s", APP_GAF_MCC_SVC_DISCOVERYED_IND, __func__);
#ifdef AOB_MOBILE_ENABLED
    ble_audio_discovery_modify_interval(mcc_svc_discoveryed->con_lid, DISCOVER_COMPLEPE, SERVICE_ACC_MCC);
#endif
}

static void aob_mgr_gaf_mcc_track_changed_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_mcc_track_changed_ind_t *p_track_changed_ind = (app_gaf_acc_mcc_track_changed_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_MCC_TRACK_CHANGED_IND, __func__);

    if (aob_mgr_media_evt_handler.media_track_change_cb)
    {
        aob_mgr_media_evt_handler.media_track_change_cb(p_track_changed_ind->con_lid);
    }
}

static void aob_mgr_gaf_mcc_media_value_ind(void *event)
{
    app_gaf_acc_mcc_value_ind_t *p_val_ind = (app_gaf_acc_mcc_value_ind_t *)event;

    switch (p_val_ind->char_type)
    {
        case AOB_MGR_MC_CHAR_TYPE_MEDIA_STATE:
        {
            app_ble_audio_sink_streaming_handle_event(p_val_ind->con_lid, 0, p_val_ind->val.state, BLE_AUDIO_MEDIA_PLAYSTATUS_CHANGED);
            // Media state not inactive means lea is active
            if (p_val_ind->val.state != AOB_MGR_PLAYBACK_STATE_INACTIVE)
            {
                aob_mgr_ble_audio_connected_report(p_val_ind->con_lid);
            }

            if (aob_mgr_media_evt_handler.media_playback_status_change_cb)
            {
                aob_mgr_media_evt_handler.media_playback_status_change_cb(p_val_ind->con_lid, (AOB_MGR_PLAYBACK_STATE_E)p_val_ind->val.state);
            }
        }
            break;
        default:
            LOG_D("unknown char type %d", p_val_ind->char_type);
            break;
    }

}

static void aob_mgr_gaf_mcc_media_value_long_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_mcc_value_long_ind_t *p_val_long_ind = (app_gaf_acc_mcc_value_long_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_MCC_MEDIA_VALUE_LONG_IND, __func__);
}

static void aob_mgr_gaf_mcc_svc_changed_handler(uint8_t con_lid)
{
#if APP_GAF_ACC_ENABLE
    ble_audio_earphone_info_set_acc_bond_status(con_lid, BLE_AUDIO_ACC_MCC, false);
    /// clr svc restore success state
    aob_conn_clr_gatt_cache_restore_state(con_lid, GATT_C_MCS_POS);
    /// Only start svc re discovery when conn is encrypted
    if (aob_conn_get_encrypt_state(con_lid) == true)
    {
        aob_media_mcs_discovery(con_lid);
    }
#endif
}

static void aob_mgr_gaf_mcc_svc_changed_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_mcc_svc_changed_ind_t *mcc_svc_changed = (app_gaf_acc_mcc_svc_changed_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_MCC_SVC_CHANGED_IND, __func__);

    bt_thread_call_func_1(aob_mgr_gaf_mcc_svc_changed_handler,
                          bt_fixed_param(mcc_svc_changed->con_lid));
}

static void aob_mgr_gaf_mcc_bond_data_ind(void *event)
{
    ble_bdaddr_t remote_addr = {{0}};
    app_gaf_acc_mcc_bond_data_ind_t *mcc_bond_data = (app_gaf_acc_mcc_bond_data_ind_t *)event;
    LOG_I("app_gaf event handle: %04x, %s", APP_GAF_MCC_BOND_DATA_IND, __func__);
    ble_audio_earphone_info_set_acc_bond_status(mcc_bond_data->con_lid, BLE_AUDIO_ACC_MCC, true);
    app_ble_get_peer_solved_addr(mcc_bond_data->con_lid, &remote_addr);
    aob_gattc_cache_save(remote_addr.addr, mcc_bond_data->mcs_info.uuid, (void *)mcc_bond_data);
}

static void aob_mgr_gaf_mcc_set_cfg_cmp_ind(void *event)
{
    app_gaf_acc_mcc_cmp_evt_t *mcc_set_cfg = (app_gaf_acc_mcc_cmp_evt_t *)event;
    LOG_I("app_gaf event handle: %04x, %s", APP_GAF_MCC_SET_CFG_CMP_IND, __func__);
    // Set cfg failed due to unexpected reason except "peer not support"
    if (mcc_set_cfg->status != BT_STS_SUCCESS)
    {
        // means service is disconnected, need to restart
        aob_conn_clr_gatt_cache_restore_state(mcc_set_cfg->con_lid, GATT_C_MCS_POS);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_mcc_evt_cb_list[] =
{
    //ACC Media Control Callback Functions
    {APP_GAF_MCC_SVC_DISCOVERYED_IND,       aob_mgr_gaf_mcc_svc_discoveryed_ind},
    {APP_GAF_MCC_TRACK_CHANGED_IND,         aob_mgr_gaf_mcc_track_changed_ind},
    {APP_GAF_MCC_MEDIA_VALUE_IND,           aob_mgr_gaf_mcc_media_value_ind},
    {APP_GAF_MCC_MEDIA_VALUE_LONG_IND,      aob_mgr_gaf_mcc_media_value_long_ind},
    {APP_GAF_MCC_SVC_CHANGED_IND,           aob_mgr_gaf_mcc_svc_changed_ind},
    {APP_GAF_MCC_BOND_DATA_IND,             aob_mgr_gaf_mcc_bond_data_ind},
    {APP_GAF_MCC_SET_CFG_CMP_IND,           aob_mgr_gaf_mcc_set_cfg_cmp_ind}
};

static void aob_mgr_gaf_tbc_svc_discoveryed_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_tbc_cmp_evt_t *tbc_svc_discoveryed = (app_gaf_acc_tbc_cmp_evt_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_TBC_SVC_DISCOVERYED_IND, __func__);
    LOG_D("cmd_code:%x con_lid:%x bearer_lid:%x call_id:%x", tbc_svc_discoveryed->cmd_code, tbc_svc_discoveryed->con_lid,
          tbc_svc_discoveryed->bearer_lid, tbc_svc_discoveryed->call_id);
    if (ble_audio_earphone_info_set_bearer_lid(tbc_svc_discoveryed->con_lid, tbc_svc_discoveryed->call_id, tbc_svc_discoveryed->bearer_lid))
    {
        LOG_I("Discovery tbs bearer_id complete and map device con_lid with bearer_id");
    }

#ifdef AOB_MOBILE_ENABLED
    ble_audio_discovery_modify_interval(tbc_svc_discoveryed->con_lid, DISCOVER_COMPLEPE, SERVICE_ACC_TBC);
#else
    ble_audio_connection_interval_mgr(tbc_svc_discoveryed->con_lid, LEA_CI_MODE_SVC_DISC_CMP);
#endif
}

static void aob_mgr_gaf_tbc_call_state_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_tbc_call_state_ind_t *p_call_state_ind = (app_gaf_acc_tbc_call_state_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_TBC_CALL_STATE_IND, __func__);
    LOG_D("ind_code:%x bearer_lid:%x call_id:%x state:%x", p_call_state_ind->ind_code, p_call_state_ind->bearer_lid,
          p_call_state_ind->con_lid, p_call_state_ind->state);
}

static void aob_mgr_gaf_tbc_call_state_long_ind(void *event)
{
    app_gaf_acc_tbc_call_state_long_ind_t *pCallStateInd = (app_gaf_acc_tbc_call_state_long_ind_t *)event;
    uint8_t newState = pCallStateInd->state;
    uint8_t callId = pCallStateInd->id;

    aob_mgr_ble_audio_connected_report(pCallStateInd->con_lid);

    AOB_SINGLE_CALL_INFO_T *pInfo = ble_audio_earphone_info_find_call_info(pCallStateInd->con_lid, callId);
    // Match uri second
    if (pInfo == NULL)
    {
        pInfo = ble_audio_earphone_info_find_call_info_by_uri(pCallStateInd->con_lid, pCallStateInd->uri_len, pCallStateInd->uri);
    }
    // Can not get local call info, make one
    if (pInfo == NULL)
    {
        pInfo = ble_audio_earphone_info_make_call_info(pCallStateInd->con_lid, callId, pCallStateInd->uri_len);
    }

    if (pInfo == NULL)
    {
        LOG_E("%s, Can not get call info newState:%d, callId:%d", __func__, newState, callId);
        return;
    }

    uint8_t call_state_pre = pInfo->state;
    LOG_I("%s, call_id:%d, state:%s->%s", __func__, callId, aob_call_state_to_str(call_state_pre), aob_call_state_to_str(newState));
    if (newState < APP_GAF_TB_CALL_STATE_MAX)
    {
        pInfo->bearer_lid = pCallStateInd->bearer_lid;
        pInfo->call_id = callId;
        pInfo->call_flags.outgoing_call_flag = pCallStateInd->flags & 0x01;
        pInfo->call_flags.withheld_server_flag = pCallStateInd->flags & 0x02;
        pInfo->call_flags.withheld_network_flag = pCallStateInd->flags & 0x04;
        pInfo->uri_len = pCallStateInd->uri_len;
        memcpy(pInfo->uri, pCallStateInd->uri, pCallStateInd->uri_len);
        if (newState < APP_GAF_TB_CALL_STATE_MAX)
        {
            pInfo->state = (AOB_CALL_STATE_E)(newState);
        }
        else
        {
            pInfo->state = AOB_CALL_STATE_IDLE;
        }
    }

    if (pInfo->state != call_state_pre || (pInfo->state == AOB_CALL_STATE_ACTIVE && call_state_pre == AOB_CALL_STATE_ACTIVE))
    {
        if (aob_mgr_call_evt_handler.call_state_change_cb)
        {
            aob_mgr_call_evt_handler.call_state_change_cb(pCallStateInd->con_lid, callId, pInfo);
        }
    }
    else
    {
        LOG_I("%s, call_id:%d, dumplicate call, new_state:%d", __func__, callId, newState);
    }
}

/// Callback function called when value of one of the following characteristic is
/// received:
///     - Bearer Technology characteristic
///     - Bearer Signal Strength characteristic
///     - Bearer Signal Strength Reporting Interval characteristic
///     - Content Control ID characteristic
///     - Status Flags characteristic
///     - Call Control Point Optional Opcodes characteristic
///     - Termination Reason characteristic
static void aob_mgr_gaf_tbc_call_value_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_tbc_value_ind_t *p_val_ind = (app_gaf_acc_tbc_value_ind_t *)event;
    LOG_I("%s, con_lid:%d, call_id:%d, char_type:%d", __func__, p_val_ind->con_lid, p_val_ind->call_id, p_val_ind->char_type);

    uint16_t char_value = 0;
    AOB_SINGLE_CALL_INFO_T *pInfo = ble_audio_earphone_info_find_call_info(p_val_ind->con_lid, p_val_ind->call_id);
    AOB_CALL_ENV_INFO_T *pCallEnvInfo = ble_audio_earphone_info_get_call_env_info(p_val_ind->con_lid);

    switch (p_val_ind->char_type)
    {
        case APP_GAF_TB_CHAR_TYPE_SIGN_STRENGTH:
            if (pInfo)
            {
                pInfo->signal_strength = p_val_ind->val.sign_strength;
            }
            if (aob_mgr_call_evt_handler.call_srv_signal_strength_value_ind_cb)
            {
                aob_mgr_call_evt_handler.call_srv_signal_strength_value_ind_cb(p_val_ind->con_lid,
                                                                               p_val_ind->call_id, p_val_ind->val.sign_strength);
            }
            break;
        case APP_GAF_TB_CHAR_TYPE_STATUS_FLAGS:
            char_value = p_val_ind->val.status_flags_bf;
            if (pCallEnvInfo)
            {
                pCallEnvInfo->status_flags.inband_ring_enable = char_value & 0x01;
                pCallEnvInfo->status_flags.silent_mode_enable = char_value & 0x02;
            }
            if (aob_mgr_call_evt_handler.call_status_flags_ind_cb)
            {
                aob_mgr_call_evt_handler.call_status_flags_ind_cb(p_val_ind->con_lid,
                                                                  p_val_ind->call_id, (char_value & 0x01), (char_value & 0x02));
            }
            break;
        case APP_GAF_TB_CHAR_TYPE_CALL_CTL_PT_OPT_OPCODES:
            char_value = p_val_ind->val.opt_opcodes_bf;
            if (pCallEnvInfo)
            {
                pCallEnvInfo->opt_opcode_flags.local_hold_op_supported = char_value & 0x01;
                pCallEnvInfo->opt_opcode_flags.join_op_supported = char_value & 0x02;
            }
            if (aob_mgr_call_evt_handler.call_ccp_opt_supported_opcode_ind_cb)
            {
                aob_mgr_call_evt_handler.call_ccp_opt_supported_opcode_ind_cb(p_val_ind->con_lid, (char_value & 0x01), (char_value & 0x02));
            }
            break;
        case APP_GAF_TB_CHAR_TYPE_TERM_REASON:
            AOB_CALL_SRV_TERMINATE_IND_T call_terminate_ind;
            call_terminate_ind.con_lid = p_val_ind->con_lid;
            call_terminate_ind.bearer_id = p_val_ind->bearer_lid;
            call_terminate_ind.call_id = p_val_ind->call_id;
            call_terminate_ind.terminate_reason = p_val_ind->val.term_reason;
            if (pInfo)
            {
                pInfo->state = AOB_CALL_STATE_IDLE;
                if (aob_mgr_call_evt_handler.call_state_change_cb)
                {
                    aob_mgr_call_evt_handler.call_state_change_cb(call_terminate_ind.con_lid, call_terminate_ind.call_id, pInfo);
                }
            }
            if (aob_mgr_call_evt_handler.call_terminate_reason_ind_cb)
            {
                char_value = p_val_ind->val.term_reason;
                aob_mgr_call_evt_handler.call_terminate_reason_ind_cb(p_val_ind->con_lid, p_val_ind->call_id, char_value);
            }
            break;
        case APP_GAF_TB_CHAR_TYPE_CALL_CTL_PT:
            break;
        default:
            break;
    }
}

static void aob_mgr_gaf_tbc_report_dialing_call_idle(uint8_t con_lid)
{
    uint8_t call_index = 0;
    AOB_SINGLE_CALL_INFO_T *p_call_info = NULL;
    AOB_CALL_ENV_INFO_T *p_call_env = ble_audio_earphone_info_get_call_env_info(con_lid);

    if (p_call_env == NULL)
    {
        LOG_E("%s invalid connection %d to get call info", __func__, con_lid);
        return;
    }

    p_call_info = p_call_env->single_call_info;

    for (call_index = 0; call_index < ARRAY_LEN(p_call_env->single_call_info); call_index++, p_call_info++)
    {
        if (p_call_info->state == AOB_CALL_STATE_DIALING)
        {
            p_call_info->state = AOB_CALL_STATE_IDLE;
            // Callback upper call is end
            if (aob_mgr_call_evt_handler.call_state_change_cb)
            {
                aob_mgr_call_evt_handler.call_state_change_cb(con_lid, p_call_info->call_id, p_call_info);
            }
            if (aob_mgr_call_evt_handler.call_terminate_reason_ind_cb)
            {
                aob_mgr_call_evt_handler.call_terminate_reason_ind_cb(con_lid, p_call_info->call_id, APP_GAF_ACC_TB_TERM_REASON_SRV_END);
            }
        }
    }
}

/// Callback function called when value of one of the following characteristic is
/// received:
///     - Bearer Provider Name characteristic
///     - Bearer UCI characteristic
///     - Bearer URI Schemes Supported List characteristic
///     - Incoming Call Target Bearer URI characteristic
///     - Incoming Call characteristic
///     - Call Friendly Name characteristic
static void aob_mgr_gaf_tbc_call_value_long_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_tbc_value_long_ind_t *p_val_long_ind = (app_gaf_acc_tbc_value_long_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_TBC_CALL_VALUE_LONG_IND, __func__);
    switch (p_val_long_ind->char_type)
    {
        case APP_GAF_TB_CHAR_TYPE_IN_TGT_CALLER_ID:
            break;
        case APP_GAF_TB_CHAR_TYPE_INCOMING_CALL:
            if (aob_mgr_call_evt_handler.call_incoming_number_inf_ind_cb)
            {
                aob_mgr_call_evt_handler.call_incoming_number_inf_ind_cb(p_val_long_ind->con_lid, p_val_long_ind->call_id,
                                                                         p_val_long_ind->val_len, p_val_long_ind->val);
            }
            break;
        case APP_GAF_TB_CHAR_TYPE_CALL_FRIENDLY_NAME:
            break;
        case APP_GAF_TB_CHAR_TYPE_CURR_CALLS_LIST:
            if (p_val_long_ind->val_len == 0)
            {
                aob_mgr_gaf_tbc_report_dialing_call_idle(p_val_long_ind->con_lid);
            }
            break;
        default:
            break;
    }
}

static void aob_mgr_gaf_tbc_svc_changed_handler(uint8_t con_lid)
{
#if APP_GAF_ACC_ENABLE
    ble_audio_earphone_info_set_acc_bond_status(con_lid, BLE_AUDIO_ACC_TBC, false);
    ble_bdaddr_t remote_addr = {{0}};
    app_ble_get_peer_solved_addr(con_lid, &remote_addr);
    aob_gattc_delete_nv_cache(remote_addr.addr, GATT_SVC_GENERIC_TELEPHONE_BEARER);
    /// clr svc restore success state
    aob_conn_clr_gatt_cache_restore_state(con_lid, GATT_C_TBS_POS);
    /// Only start svc re discovery when conn is encrypted
    if (aob_conn_get_encrypt_state(con_lid) == true)
    {
        aob_call_tbs_discovery(con_lid);
    }
#endif
}

static void aob_mgr_gaf_tbc_svc_changed_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_tbc_svc_changed_ind_t *tbc_svc_changed = (app_gaf_acc_tbc_svc_changed_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_TBC_SVC_CHANGED_IND, __func__);
    LOG_D("ind_code:%x", tbc_svc_changed->con_lid);
    if (aob_mgr_call_evt_handler.call_svc_changed_ind_cb)
    {
        aob_mgr_call_evt_handler.call_svc_changed_ind_cb(tbc_svc_changed->con_lid);
    }

    bt_thread_call_func_1(aob_mgr_gaf_tbc_svc_changed_handler,
                          bt_fixed_param(tbc_svc_changed->con_lid));
}

static void aob_mgr_gaf_tbc_call_action_result_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_tbc_cmp_evt_t *tbc_action_result = (app_gaf_acc_tbc_cmp_evt_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_TBC_CALL_ACTION_RESULT_IND, __func__);

    if (aob_mgr_call_evt_handler.call_action_result_ind_cb)
    {
        AOB_CALL_CLI_ACTION_RESULT_IND_T action_result;
        action_result.con_lid = tbc_action_result->con_lid;
        action_result.bearer_id = tbc_action_result->bearer_lid;
        action_result.action_opcode = tbc_action_result->u.opcode;
        action_result.result = tbc_action_result->result;
        aob_mgr_call_evt_handler.call_action_result_ind_cb(tbc_action_result->con_lid, &action_result);
    }
}

static void aob_mgr_gaf_tbc_svc_bond_data_ind(void *event)
{
    ble_bdaddr_t remote_addr = {{0}};
    app_gaf_acc_tbc_bond_data_ind_t *tbc_bond_data = (app_gaf_acc_tbc_bond_data_ind_t *)event;
    LOG_I("app_gaf event handle: %04x, %s", APP_GAF_TBC_BOND_DATA_IND, __func__);
    ble_audio_earphone_info_set_acc_bond_status(tbc_bond_data->con_lid, BLE_AUDIO_ACC_TBC, true);
    app_ble_get_peer_solved_addr(tbc_bond_data->con_lid, &remote_addr);
    aob_gattc_cache_save(remote_addr.addr, tbc_bond_data->tbs_info.uuid, (void *)tbc_bond_data);
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_tbc_evt_cb_list[] =
{
    //ACC Call Control Callback Functions
    {APP_GAF_TBC_SVC_DISCOVERYED_IND,       aob_mgr_gaf_tbc_svc_discoveryed_ind},
    {APP_GAF_TBC_CALL_STATE_IND,            aob_mgr_gaf_tbc_call_state_ind},
    {APP_GAF_TBC_CALL_STATE_LONG_IND,       aob_mgr_gaf_tbc_call_state_long_ind},
    {APP_GAF_TBC_CALL_VALUE_IND,            aob_mgr_gaf_tbc_call_value_ind},
    {APP_GAF_TBC_CALL_VALUE_LONG_IND,       aob_mgr_gaf_tbc_call_value_long_ind},
    {APP_GAF_TBC_SVC_CHANGED_IND,           aob_mgr_gaf_tbc_svc_changed_ind},
    {APP_GAF_TBC_CALL_ACTION_RESULT_IND,    aob_mgr_gaf_tbc_call_action_result_ind},
    {APP_GAF_TBC_BOND_DATA_IND,             aob_mgr_gaf_tbc_svc_bond_data_ind}
};

static void aob_mgr_gaf_aics_state_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_arc_aics_state_ind_t *p_state_ind = (app_gaf_arc_aics_state_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_AICS_STATE_IND, __func__);
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_aics_evt_cb_list[] =
{
    //ARC Audio Input Control Callback Function
    {APP_GAF_AICS_STATE_IND,                aob_mgr_gaf_aics_state_ind},
};

static void aob_mgr_gaf_mics_mute_ind(void *event)
{
    app_gaf_arc_mics_mute_ind_t *mute = (app_gaf_arc_mics_mute_ind_t *)event;

    if (aob_mgr_media_evt_handler.media_mic_state_cb)
    {
        aob_mgr_media_evt_handler.media_mic_state_cb(mute->mute);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_mics_evt_cb_list[] =
{
    //ARC Microphone Control Callback Function
    {APP_GAF_MICS_MUTE_IND,                 aob_mgr_gaf_mics_mute_ind},
};

static void aob_mgr_gaf_vcs_volume_ind(void *event)
{
    app_gaf_arc_vcs_volume_ind_t *p_volume_ind = (app_gaf_arc_vcs_volume_ind_t *)event;
    uint8_t reason = p_volume_ind->reason;
    uint8_t con_lid = p_volume_ind->con_lid;

    aob_mgr_ble_audio_connected_report(con_lid);

    LOG_D("%s volume %d mute %d reason %d", __func__, p_volume_ind->volume, p_volume_ind->mute, reason);
    if (aob_mgr_vol_evt_handler.vol_changed_cb)
    {
        aob_mgr_vol_evt_handler.vol_changed_cb(con_lid, p_volume_ind->volume,
                                                p_volume_ind->mute, p_volume_ind->change_cnt, reason);
    }
}

static void aob_mgr_gaf_vcs_flags_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_arc_vcs_flags_ind_t *p_flags_ind = (app_gaf_arc_vcs_flags_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_VCS_FLAGS_IND, __func__);
}

static void aob_mgr_gaf_vcs_bond_data_ind(void *event)
{
    app_gaf_arc_vcs_bond_data_ind_t *vcs_bond_data = (app_gaf_arc_vcs_bond_data_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_VCS_BOND_DATA_IND, __func__);
    ble_bdaddr_t remote_addr = {{0}};
    app_ble_get_peer_solved_addr(vcs_bond_data->con_lid, &remote_addr);
    aob_gattc_cache_save(remote_addr.addr, GATT_SVC_VOLUME_CONTROL, (void *)vcs_bond_data);
    if (aob_mgr_vol_evt_handler.vcs_bond_data_changed_cb)
    {
        aob_mgr_vol_evt_handler.vcs_bond_data_changed_cb(vcs_bond_data->con_lid,
                                                         vcs_bond_data->char_type, vcs_bond_data->cli_cfg_bf);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_vcs_evt_cb_list[] =
{
    // VCS Events
    {APP_GAF_VCS_VOLUME_IND,                aob_mgr_gaf_vcs_volume_ind},
    {APP_GAF_VCS_FLAGS_IND,                 aob_mgr_gaf_vcs_flags_ind},
    {APP_GAF_VCS_BOND_DATA_IND,             aob_mgr_gaf_vcs_bond_data_ind},
};

static void aob_mgr_gaf_vocs_location_set_ri(void *event)
{
    POSSIBLY_UNUSED app_gaf_arc_vocs_set_location_req_ind_t *p_vocs_loc_req_ind = (app_gaf_arc_vocs_set_location_req_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_VOCS_LOCATION_SET_RI, __func__);
}

static void aob_mgr_gaf_vocs_offset_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_arc_vocs_offset_ind_t *p_offset_ind = (app_gaf_arc_vocs_offset_ind_t *)event;
    if (aob_mgr_vol_evt_handler.vocs_offset_changed_cb)
    {
        aob_mgr_vol_evt_handler.vocs_offset_changed_cb(p_offset_ind->offset, p_offset_ind->output_lid);
    }
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_VOCS_OFFSET_IND, __func__);
}

static void aob_mgr_gaf_vocs_bond_data_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_arc_vocs_cfg_ind_t *p_cfg_ind = (app_gaf_arc_vocs_cfg_ind_t *)event;
    if (aob_mgr_vol_evt_handler.vocs_bond_data_changed_cb)
    {
        aob_mgr_vol_evt_handler.vocs_bond_data_changed_cb(p_cfg_ind->output_lid, p_cfg_ind->cli_cfg_bf);
    }
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_VOCS_BOND_DATA_IND, __func__);
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_vocs_evt_cb_list[] =
{
    //ARC Volume Offset Control Callback Functions
    {APP_GAF_VOCS_LOCATION_SET_RI,          aob_mgr_gaf_vocs_location_set_ri},
    {APP_GAF_VOCS_OFFSET_IND,               aob_mgr_gaf_vocs_offset_ind},
    {APP_GAF_VOCS_BOND_DATA_IND,            aob_mgr_gaf_vocs_bond_data_ind},
};

static void aob_mgr_gaf_csism_lock_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_atc_csism_lock_ind_t *p_lock_ind = (app_gaf_atc_csism_lock_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_CSISM_LOCK_IND, __func__);
}

static void aob_mgr_gaf_csism_ltk_ri(void *event)
{
    POSSIBLY_UNUSED app_gaf_atc_csism_ltk_req_ind_t *p_authorization_ri = (app_gaf_atc_csism_ltk_req_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_CSISM_LTK_RI, __func__);

    aob_mgr_ble_audio_connected_report(p_authorization_ri->con_lid);
}

static void aob_mgr_gaf_csism_rsi_generated_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_atc_csism_rsi_ind_t *p_rsi_ind = (app_gaf_atc_csism_rsi_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_CSISM_NEW_RSI_GENERATED_IND, __func__);
    DUMP8("%02x ", p_rsi_ind->rsi.rsi, AOB_COMMON_ATC_CSIS_RSI_LEN);

    if (aob_mgr_csip_evt_handler.csip_rsi_value_updated_cb)
    {
        aob_mgr_csip_evt_handler.csip_rsi_value_updated_cb(p_rsi_ind->rsi.rsi, AOB_COMMON_ATC_CSIS_RSI_LEN);
    }
}

static void aob_mgr_gaf_csism_ntf_sent_ind(void *event)
{
    app_gaf_atc_csism_ntf_sent_t *ntf_sent_ind = (app_gaf_atc_csism_ntf_sent_t *)event;
    if (aob_mgr_csip_evt_handler.csip_ntf_sent_cb)
    {
        aob_mgr_csip_evt_handler.csip_ntf_sent_cb(ntf_sent_ind->con_lid, ntf_sent_ind->char_type);
    }
}

static void aob_mgr_gaf_csism_read_rsp_sent_ind(void *event)
{
    app_gaf_atc_csism_read_rsp_sent_t *read_rsp_sent_ind = (app_gaf_atc_csism_read_rsp_sent_t *)event;
    LOG_I("app_gaf event handle: %s", __func__);
    if (aob_mgr_csip_evt_handler.csip_read_rsp_sent_cb)
    {
        aob_mgr_csip_evt_handler.csip_read_rsp_sent_cb(read_rsp_sent_ind->con_lid,
                                                       read_rsp_sent_ind->char_type, read_rsp_sent_ind->data, read_rsp_sent_ind->data_len);
    }
    // Read any value of csis indicate that le audio is connected
    aob_mgr_ble_audio_connected_report(read_rsp_sent_ind->con_lid);
}

static void aob_mgr_gaf_csism_bond_data_ind(void *param)
{
    app_gaf_atc_csism_bond_data_ind_t *bond_data_ind = (app_gaf_atc_csism_bond_data_ind_t *)param;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_CSISM_BOND_DATA_IND, __func__);
    ble_bdaddr_t remote_addr = {{0}};
    app_ble_get_peer_solved_addr(bond_data_ind->con_lid, &remote_addr);
    aob_gattc_cache_save(remote_addr.addr, GATT_SVC_COORD_SET_IDENTIFICATION, (void *)bond_data_ind);
    // Peer device is set cfg of CSIS
    aob_mgr_ble_audio_connected_report(bond_data_ind->con_lid);
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_csim_evt_cb_list[] =
{
    //ATC CSISM Callback Functions
    {APP_GAF_CSISM_LOCK_IND,                  aob_mgr_gaf_csism_lock_ind},
    {APP_GAF_CSISM_LTK_RI,                    aob_mgr_gaf_csism_ltk_ri},
    {APP_GAF_CSISM_NEW_RSI_GENERATED_IND,     aob_mgr_gaf_csism_rsi_generated_ind},
    {APP_GAF_CSISM_BOND_DATA_IND,             aob_mgr_gaf_csism_bond_data_ind},
    {APP_GAF_CSISM_NTF_SENT_IND,              aob_mgr_gaf_csism_ntf_sent_ind},
    {APP_GAF_CSISM_READ_RSP_SENT_IND,         aob_mgr_gaf_csism_read_rsp_sent_ind},
};

static void aob_mgr_gaf_bc_scan_state_idle_ind(void *event)
{
    if (aob_mgr_scan_evt_handler.scan_state_idle_cb)
    {
        aob_mgr_scan_evt_handler.scan_state_idle_cb();
    }
    LOG_D("app_gaf event handle: %04x, %s", APP_BAP_BC_SCAN_STATE_IDLE_IND, __func__);
}

static void aob_mgr_gaf_bc_scan_state_scanning_ind(void *event)
{
    if (aob_mgr_scan_evt_handler.scan_state_scanning_cb)
    {
        aob_mgr_scan_evt_handler.scan_state_scanning_cb();
    }
    LOG_D("app_gaf event handle: %04x, %s", APP_BAP_BC_SCAN_STATE_SCANNING_IND, __func__);
}

static void aob_mgr_gaf_bc_scan_state_synchronizing_ind(void *event)
{
    app_gaf_cmp_evt_t *p_scan_ynchronizing_ind = (app_gaf_cmp_evt_t *)event;

    if (aob_mgr_scan_evt_handler.scan_state_synchronizing_cb)
    {
        if(p_scan_ynchronizing_ind != NULL)
        {
            aob_mgr_scan_evt_handler.scan_state_synchronizing_cb(p_scan_ynchronizing_ind->status);
        }
    }
    LOG_D("app_gaf event handle: %04x, %s", APP_BAP_BC_SCAN_STATE_SYNCHRONIZING_IND, __func__);
}

static void aob_mgr_gaf_bc_scan_state_synchronized_ind(void *event)
{
    if (aob_mgr_scan_evt_handler.scan_state_synchronized_cb)
    {
        aob_mgr_scan_evt_handler.scan_state_synchronized_cb();
    }
    LOG_D("app_gaf event handle: %04x, %s", APP_BAP_BC_SCAN_STATE_SYNCHRONIZED_IND, __func__);
}

static void aob_mgr_gaf_bc_scan_state_streaming_ind(void *event)
{
    app_gaf_bc_scan_state_stream_t *p_scan_state_ind = (app_gaf_bc_scan_state_stream_t *)event;

    LOG_D("app_gaf event handle: %04x, %s", APP_BAP_BC_SCAN_STATE_STREAMING_IND, __func__);

    if (aob_mgr_scan_evt_handler.scan_state_streaming_cb)
    {
        aob_mgr_scan_evt_handler.scan_state_streaming_cb(p_scan_state_ind);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_bc_scan_state_evt_cb_list[] =
{
    //BIS scan state callback function
    {APP_BAP_BC_SCAN_STATE_IDLE_IND,          aob_mgr_gaf_bc_scan_state_idle_ind},
    {APP_BAP_BC_SCAN_STATE_SCANNING_IND,      aob_mgr_gaf_bc_scan_state_scanning_ind},
    {APP_BAP_BC_SCAN_STATE_SYNCHRONIZING_IND, aob_mgr_gaf_bc_scan_state_synchronizing_ind},
    {APP_BAP_BC_SCAN_STATE_SYNCHRONIZED_IND,  aob_mgr_gaf_bc_scan_state_synchronized_ind},
    {APP_BAP_BC_SCAN_STATE_STREAMING_IND,     aob_mgr_gaf_bc_scan_state_streaming_ind},
};

static void aob_mgr_gaf_dts_coc_registered_ind(void *event)
{
    app_gaf_dts_cmp_evt_t *p_cmp_evt_t = (app_gaf_dts_cmp_evt_t *)event;

    LOG_D("app_gaf event handle: %04x, %s", APP_DTS_COC_REGISTERED_IND, __func__);

    if (aob_mgr_dts_coc_evt_handler.dts_coc_registered_cb)
    {
        aob_mgr_dts_coc_evt_handler.dts_coc_registered_cb(p_cmp_evt_t->status,
                                                          p_cmp_evt_t->spsm);
    }
}

static void aob_mgr_gaf_dts_coc_connected_ind(void *event)
{
    app_gaf_dts_coc_connected_ind_t *p_coc_connected_ind = (app_gaf_dts_coc_connected_ind_t *)event;

    LOG_D("app_gaf event handle: %04x, %s", APP_DTS_COC_CONNECTED_IND, __func__);

    if (aob_mgr_dts_coc_evt_handler.dts_coc_connected_cb)
    {
        aob_mgr_dts_coc_evt_handler.dts_coc_connected_cb(p_coc_connected_ind->con_lid,
                                                         p_coc_connected_ind->tx_mtu, p_coc_connected_ind->tx_mps,
                                                         p_coc_connected_ind->spsm, p_coc_connected_ind->initial_credits);
    }
}

static void aob_mgr_gaf_dts_coc_disconnected_ind(void *event)
{
    app_gaf_dts_coc_disconnected_ind_t *p_coc_disconnected_ind = (app_gaf_dts_coc_disconnected_ind_t *)event;

    LOG_D("app_gaf event handle: %04x, %s", APP_DTS_COC_DISCONNECTED_IND, __func__);

    if (aob_mgr_dts_coc_evt_handler.dts_coc_disconnected_cb)
    {
        aob_mgr_dts_coc_evt_handler.dts_coc_disconnected_cb(p_coc_disconnected_ind->con_lid,
                                                            p_coc_disconnected_ind->reason, p_coc_disconnected_ind->spsm);
    }
}

static void aob_mgr_gaf_dts_coc_data_ind(void *event)
{
    app_gaf_dts_coc_data_ind_t *p_data_coc_ind = (app_gaf_dts_coc_data_ind_t *)event;

    LOG_D("app_gaf event handle: %04x, %s", APP_DTS_COC_DATA_IND, __func__);

    if (aob_mgr_dts_coc_evt_handler.dts_coc_data_cb)
    {
        aob_mgr_dts_coc_evt_handler.dts_coc_data_cb(p_data_coc_ind->con_lid, p_data_coc_ind->spsm,
                                                    p_data_coc_ind->length, p_data_coc_ind->sdu);
    }
}

static void aob_mgr_gaf_dts_coc_send_ind(void *event)
{
    app_gaf_dts_cmp_evt_t *p_cmp_evt_t = (app_gaf_dts_cmp_evt_t *)event;

    LOG_D("app_gaf event handle: %04x, %s", APP_DTS_COC_SEND_IND, __func__);

    if (aob_mgr_dts_coc_evt_handler.dts_coc_send_cb)
    {
        aob_mgr_dts_coc_evt_handler.dts_coc_send_cb(p_cmp_evt_t->con_lid, p_cmp_evt_t->spsm);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_dts_evt_cb_list[] =
{
    //DTS event callback function
    {APP_DTS_COC_REGISTERED_IND,         aob_mgr_gaf_dts_coc_registered_ind},
    {APP_DTS_COC_CONNECTED_IND,          aob_mgr_gaf_dts_coc_connected_ind},
    {APP_DTS_COC_DISCONNECTED_IND,       aob_mgr_gaf_dts_coc_disconnected_ind},
    {APP_DTS_COC_DATA_IND,               aob_mgr_gaf_dts_coc_data_ind},
    {APP_DTS_COC_SEND_IND,               aob_mgr_gaf_dts_coc_send_ind},
};

void aob_mgr_gaf_evt_handle(uint16_t id, void *event_id)
{
    uint8_t module_id = (uint8_t)GAF_ID_GET(id);
    uint16_t event = (uint16_t)GAF_EVENT_GET(id);
    aob_app_gaf_evt_cb_t *evt_cb = NULL;

    switch (module_id)
    {
        case APP_GAF_ASCS_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_ascs_evt_cb_list[0];
        }
        break;
        case APP_GAF_PACS_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_pacs_evt_cb_list[0];
        }
        break;

#ifdef APP_BLE_BIS_SRC_ENABLE
        case APP_GAF_BIS_SOURCE_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_mobile_src_bis_evt_cb_list[0];
        }
        break;
#endif

#ifdef APP_BLE_BIS_ASSIST_ENABLE
        case APP_GAF_BIS_ASSIST_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_mobile_assist_evt_cb_list[0];
        }
        break;
#endif

#if defined(APP_BLE_BIS_ASSIST_ENABLE) || defined(APP_BLE_BIS_DELEG_ENABLE) || defined(APP_BLE_BIS_SINK_ENABLE)
        case APP_GAF_BIS_SCAN_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_bis_scan_evt_cb_list[0];
        }
        break;
#endif

#ifdef APP_BLE_BIS_SINK_ENABLE
        case APP_GAF_BIS_SINK_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_bis_sink_evt_cb_list[0];
        }
        break;
#endif

#ifdef APP_BLE_BIS_DELEG_ENABLE
        case APP_GAF_BIS_DELEG_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_deleg_evt_cb_list[0];
        }
        break;
#endif

        case APP_GAF_MCC_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_mcc_evt_cb_list[0];
        }
        break;

        case APP_GAF_TBC_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_tbc_evt_cb_list[0];
        }
        break;
        case APP_GAF_AICS_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_aics_evt_cb_list[0];
        }
        break;
        case APP_GAF_MICS_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_mics_evt_cb_list[0];
        }
        break;
        case APP_GAF_VCS_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_vcs_evt_cb_list[0];
        }
        break;
        case APP_GAF_VOCS_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_vocs_evt_cb_list[0];
        }
        break;
        case APP_GAF_CSISM_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_csim_evt_cb_list[0];
        }
        break;
        case APP_GAF_BC_SCAN_STATE_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_bc_scan_state_evt_cb_list[0];
        }
        break;
        case APP_GAF_DTS_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_dts_evt_cb_list[0];
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

void aob_mgr_call_evt_handler_register(call_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_call_evt_handler, handlerBundle, sizeof(call_event_handler_t));
}

void aob_mgr_media_evt_handler_register(media_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_media_evt_handler, handlerBundle, sizeof(media_event_handler_t));
}

void aob_mgr_gaf_vol_evt_handler_register(vol_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_vol_evt_handler, handlerBundle, sizeof(vol_event_handler_t));
}

void aob_mgr_gaf_mobile_src_evt_handler_register(src_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_src_evt_handler, handlerBundle, sizeof(src_event_handler_t));
}

void aob_mgr_gaf_mobile_assist_evt_handler_register(assist_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_assist_evt_handler, handlerBundle, sizeof(assist_event_handler_t));
}

void aob_mgr_gaf_sink_evt_handler_register(sink_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_sink_evt_handler, handlerBundle, sizeof(sink_event_handler_t));
}

void aob_mgr_gaf_scan_evt_handler_register(scan_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_scan_evt_handler, handlerBundle, sizeof(scan_event_handler_t));
}

void aob_mgr_gaf_deleg_evt_handler_register(deleg_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_deleg_evt_handler, handlerBundle, sizeof(deleg_event_handler_t));
}

void aob_mgr_csip_evt_handler_register(csip_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_csip_evt_handler, handlerBundle, sizeof(csip_event_handler_t));
}

void aob_mgr_dts_coc_evt_handler_register(dts_coc_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_dts_coc_evt_handler, handlerBundle, sizeof(dts_coc_event_handler_t));
}

void aob_mgr_cis_conn_evt_handler_t_register(cis_conn_evt_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_cis_conn_evt_handler, handlerBundle, sizeof(cis_conn_evt_handler_t));
}

void aob_mgr_pacs_evt_handler_t_register(pacs_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_pacs_event_handler, handlerBundle, sizeof(pacs_event_handler_t));
}

void aob_mgr_gaf_evt_init(void)
{
    LOG_I("%s done", __FUNCTION__);
    aob_csip_if_init();
    aob_conn_api_init();
    aob_media_api_init();
    aob_call_if_init();
    aob_vol_api_init();
    aob_pacs_api_init();
    aob_cis_api_init();
}
#endif

/// @} APP
