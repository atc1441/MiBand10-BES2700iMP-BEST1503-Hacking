/**
 * @file aob_media_api.cpp
 * @author BES AI team
 * @version 0.1
 * @date 2020-08-31
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
#include "bluetooth_ble_api.h"

#include "co_timer.h"
#include "hal_trace.h"
#include "hal_aud.h"
#include "app_trace_rx.h"
#include "plat_types.h"
#include "heap_api.h"
#include "me_api.h"
#include "app_bt_func.h"

#include "app_gaf.h"
#include "app_gaf_dbg.h"
#include "app_gaf_define.h"

#include "ble_audio_earphone_info.h"
#include "ble_audio_core_api.h"

#include "aob_stream_handler.h"

#include "aob_media_api.h"
#include "aob_conn_api.h"

#include "app_audio_active_device_manager.h"
#include "app_audio_control.h"

#ifdef AOB_MOBILE_ENABLED
#include "ble_audio_mobile_info.h"
#include "gaf_mobile_media_stream.h"
#include "app_acc_mcs_msg.h"
#endif

#include "app_bap_uc_srv_msg.h"
#include "app_acc_mcc_msg.h"

#ifdef HID_ULL_ENABLE
#include "gaf_ull_hid_stream.h"
#endif

#if defined(BLE_AUDIO_CENTRAL_APP_ENABLED)
#include "app_ble_audio_central_stream_stm.h"
#endif

#define MCP_MEDIA_PLAYER_PLAYING_SPEED      (1)// 1x
#define MCP_MEDIA_PLAYER_SEEKING_SPEED      (5 * MCP_MEDIA_PLAYER_PLAYING_SPEED)// 5x
#define MCP_MEDIA_DEFAULT_TRACK_DUR_10MS    (12000)// 120s

#define MCP_MEDIA_PLAYBACK_SEGMENT_10MS     (200)// 2000ms per segment
#define MCP_MEDIA_SEEKING_INTERVAL_10MS     (MCP_MEDIA_PLAYER_SEEKING_SPEED * 100)// 5000ms
#define MCP_MEDIA_SEEKING_TIMEOUT_MS        (500)// 500ms

#define MCP_MEDIA_STARTING_OF_SEGMENT(pos)  ((pos / MCP_MEDIA_PLAYBACK_SEGMENT_10MS) * MCP_MEDIA_PLAYBACK_SEGMENT_10MS)

typedef enum
{
    AOB_MEDIA_INACTIVE = 0,
    AOB_MEDIA_PLAYING,
    AOB_MEDIA_PAUSED,
    AOB_MEDIA_SEEKING,
} aob_media_media_state;
/****************************for server(earbud)*****************************/

/*********************external function declaration*************************/

/************************private macro defination***************************/

/************************private type defination****************************/

/**********************private function declaration*************************/

/************************private variable defination************************/
#ifdef AOB_MOBILE_ENABLED
static struct media_player_emul
{
    uint8_t media_state = 0;
    uint8_t seeking_timer = 0;
    int32_t track_position = 0;
} media_player_state[1 + APP_ACC_MCC_MAX_SUPP_MCS_INST];
#endif

#if (!defined(HID_ULL_ENABLE)) && defined(BLE_AUDIO_CENTRAL_APP_ENABLED)
uint8_t already_resume_usb_dongle_bf = 0;
#endif

/**
 * @brief use for fill  qos req in codec req_ind - cfm cmd
 *
 */
static get_qos_req_cfg_info_cb aob_ascs_srv_codec_req_handler_cb = NULL;
/****************************function defination****************************/
void aob_media_mcs_discovery(uint8_t con_lid)
{
    app_acc_mcc_start(con_lid);
}

void aob_mc_char_type_get(uint8_t device_id, AOB_MGR_MC_CHAR_TYPE_E char_type)
{
    uint8_t con_lid = 1;
    app_acc_mcc_get(con_lid, 0, (uint8_t)char_type);
}

void aob_mc_char_type_set(uint8_t device_id, AOB_MGR_MC_CHAR_TYPE_E char_type, int32_t val)
{
    uint8_t con_lid = 1;
    app_acc_mcc_set(con_lid, 0, (uint8_t)char_type, val);
}

void aob_mc_get_cfg(uint8_t device_id, AOB_MGR_MC_CHAR_TYPE_E char_type)
{
    uint8_t con_lid = 1;
    app_acc_mcc_get_cfg(con_lid, 0, (uint8_t)char_type);
}

void aob_mc_set_cfg(uint8_t device_id, AOB_MGR_MC_CHAR_TYPE_E char_type, uint8_t enable)
{
    uint8_t con_lid = 1;
    app_acc_mcc_set_cfg(con_lid, 0, (uint8_t)char_type, enable);
}

void aob_mc_set_obj_id(uint8_t device_id, AOB_MGR_MC_CHAR_TYPE_E char_type, uint8_t *obj_id)
{
    uint8_t con_lid = 1;
    app_acc_mcc_set_obj_id(con_lid, 0, (uint8_t)char_type, obj_id);
}

void aob_media_mcc_control(uint8_t con_lid, uint8_t media_lid, AOB_MGR_MC_OPCODE_E opcode, uint32_t val)
{
    app_acc_mcc_control(con_lid, media_lid, opcode, val);
}

void aob_media_mcc_set_val(uint8_t con_lid, uint8_t media_lid, uint8_t char_type, uint32_t val)
{
    app_acc_mcc_set(con_lid, media_lid, char_type, val);
}

void aob_media_mcc_set_obj_id(uint8_t con_lid, uint8_t media_lid, uint8_t char_type, uint8_t *obj_id)
{
    app_acc_mcc_set_obj_id(con_lid, media_lid, char_type, obj_id);
}

void aob_media_play(uint8_t con_lid)
{
    aob_media_mcc_control(con_lid, 0, AOB_MGR_MC_OP_PLAY, 0);
}

void aob_media_pause(uint8_t con_lid)
{
    aob_media_mcc_control(con_lid, 0, AOB_MGR_MC_OP_PAUSE, 0);
}

void aob_media_stop(uint8_t con_lid)
{
    aob_media_mcc_control(con_lid, 0, AOB_MGR_MC_OP_STOP, 0);
}

void aob_media_next(uint8_t con_lid)
{
    aob_media_mcc_control(con_lid, 0, AOB_MGR_MC_OP_NEXT_TRACK, 0);
}

void aob_media_prev(uint8_t con_lid)
{
    aob_media_mcc_control(con_lid, 0, AOB_MGR_MC_OP_PREV_TRACK, 0);
}

void aob_media_fast_fw(uint8_t con_lid)
{
    aob_media_mcc_control(con_lid, 0, AOB_MGR_MC_OP_FAST_FW, 0);
}

void aob_media_fast_rw(uint8_t con_lid)
{
    aob_media_mcc_control(con_lid, 0, AOB_MGR_MC_OP_FAST_RW, 0);
}

void aob_media_search_player(uint8_t con_lid, uint8_t media_lid, uint8_t param_len, uint8_t *param)
{
    app_acc_mcc_search(con_lid, media_lid, param_len, param);
}

void aob_media_search(uint8_t device_id, uint8_t param_len, uint8_t *param)
{
    uint8_t con_lid = 1;
    app_acc_mcc_search(con_lid, 0, param_len, param);
}

AOB_MGR_STREAM_STATE_E aob_media_get_cur_ase_state(uint8_t ase_lid)
{
    app_bap_ascs_ase_t *p_ascs_ase = app_bap_uc_srv_get_ase_info(ase_lid);

    return (p_ascs_ase != NULL) ? (AOB_MGR_STREAM_STATE_E)p_ascs_ase->ase_state : AOB_MGR_STREAM_STATE_MAX;
}

AOB_MGR_DIRECTION_E aob_media_get_ase_direction(uint8_t ase_lid)
{
    app_bap_ascs_ase_t *p_ascs_ase = app_bap_uc_srv_get_ase_info(ase_lid);

    return (p_ascs_ase != NULL) ? (AOB_MGR_DIRECTION_E)p_ascs_ase->direction : AOB_MGR_DIRECTION_MAX;
}

void aob_media_set_location(AOB_MGR_LOCATION_BF_E location)
{
    app_bap_capa_srv_set_supp_location_bf(APP_GAF_DIRECTION_SINK, (uint32_t)location);
    app_bap_capa_srv_set_supp_location_bf(APP_GAF_DIRECTION_SRC, (uint32_t)location);
}

uint32_t aob_media_get_cur_location(uint8_t direction)
{
    return app_bap_capa_srv_get_location_bf((enum app_gaf_direction)direction);
}

void aob_media_set_ava_audio_context(uint8_t con_lid, uint16_t context_bf_ava_sink, uint16_t context_bf_ava_src)
{
    app_bap_capa_srv_set_ava_context_bf(con_lid, context_bf_ava_sink, context_bf_ava_src);
}

void aob_media_set_supp_audio_context(uint8_t con_lid, AOB_MGR_CONTEXT_TYPE_BF_E context_bf_supp)
{
    app_bap_capa_srv_set_supp_context_bf(con_lid, APP_GAF_DIRECTION_SINK, (uint16_t)context_bf_supp);
    app_bap_capa_srv_set_supp_context_bf(con_lid, APP_GAF_DIRECTION_SRC, (uint16_t)context_bf_supp);
}

void aob_media_send_enable_rsp(uint8_t ase_lid, bool accept)
{
    app_bap_uc_srv_send_enable_rsp_with_reason(ase_lid, accept, APP_GAF_BAP_UC_CP_REASON_APP_REJECTED);
}

void aob_media_send_enable_reject_stream_ctx_rsp(uint8_t ase_lid)
{
    app_bap_uc_srv_send_enable_rsp_with_reason(ase_lid, false, APP_GAF_BAP_METADATA_TYPE_STREAM_CONTEXTS);
}

void aob_media_send_upd_metadata_rsp(uint8_t ase_lid, bool accept, uint8_t reason)
{
    app_bap_uc_srv_send_update_metadata_rsp(ase_lid, accept, reason);
}

void aob_media_release_stream(uint8_t ase_lid)
{
    app_bap_uc_srv_stream_release(ase_lid, true);
}

void aob_media_read_iso_link_quality(uint8_t ase_id)
{
    if (AOB_MGR_STREAM_STATE_STREAMING == aob_media_get_cur_ase_state(ase_id))
    {
        app_bap_uc_srv_read_iso_link_quality(ase_id);
    }
}

void aob_media_set_iso_quality_rep_thr(uint8_t ase_lid, uint16_t qlty_rep_evt_cnt_thr,
                                       uint16_t tx_unack_pkts_thr, uint16_t tx_flush_pkts_thr, uint16_t tx_last_subevent_pkts_thr, uint16_t retrans_pkts_thr,
                                       uint16_t crc_err_pkts_thr, uint16_t rx_unreceived_pkts_thr, uint16_t duplicate_pkts_thr)
{
    app_bap_ascs_ase_t *p_ase = app_bap_uc_srv_get_ase_info(ase_lid);

    if (p_ase->cis_hdl != APP_GAF_INVALID_CON_HDL)
    {
        btif_me_bt_dbg_set_iso_quality_rep_thr(p_ase->cis_hdl, qlty_rep_evt_cnt_thr,
                                               tx_unack_pkts_thr, tx_flush_pkts_thr, tx_last_subevent_pkts_thr, retrans_pkts_thr,
                                               crc_err_pkts_thr, rx_unreceived_pkts_thr, duplicate_pkts_thr);
    }
}

uint8_t aob_media_get_src_ase_streaming_device_id(void)
{
    uint8_t con_lid = 0;

    for (con_lid = 0; con_lid < BLE_AUDIO_CONNECTION_CNT; con_lid++)
    {
        uint8_t ase_lid = aob_media_get_cur_streaming_ase_lid(con_lid, AOB_MGR_DIRECTION_SRC);

        if (ase_lid != APP_GAF_INVALID_CON_LID)
        {
            return con_lid;
        }
    }

    return APP_GAF_INVALID_CON_LID;
}

uint8_t aob_media_get_sink_ase_streaming_device_id(void)
{
    uint8_t con_lid = 0;

    for (con_lid = 0; con_lid < BLE_AUDIO_CONNECTION_CNT; con_lid++)
    {
        uint8_t ase_lid = aob_media_get_cur_streaming_ase_lid(con_lid, AOB_MGR_DIRECTION_SINK);

        if (ase_lid != APP_GAF_INVALID_CON_LID)
        {
            return con_lid;
        }
    }

    return APP_GAF_INVALID_CON_LID;
}

uint8_t aob_media_get_ase_streaming_device_id(void)
{
    uint8_t con_lid = aob_media_get_sink_ase_streaming_device_id();

    if (con_lid == APP_GAF_INVALID_CON_LID)
    {
        con_lid = aob_media_get_src_ase_streaming_device_id();
    }

    return con_lid;
}

void aob_media_disable_stream(uint8_t ase_lid)
{
    app_bap_uc_srv_stream_disable(ase_lid);
}

void aob_media_update_metadata(uint8_t ase_lid, app_gaf_bap_cfg_metadata_t *meta_data)
{
    app_bap_uc_srv_update_metadata_req(ase_lid, meta_data);
}

uint8_t aob_media_get_cur_streaming_ase_lid(uint8_t con_lid, AOB_MGR_DIRECTION_E direction)
{
    return app_bap_uc_srv_get_streaming_ase_lid(con_lid, (enum app_gaf_direction)direction);
}

uint8_t aob_media_get_curr_streaming_ase_lid_list(uint8_t con_lid, uint8_t *ase_lid_list)
{
    return app_bap_uc_srv_get_streaming_ase_lid_list(con_lid, ase_lid_list);
}

uint8_t aob_media_get_ready_for_stream_ase_lid_list(uint8_t con_lid, uint8_t *ase_lid_list)
{
    uint8_t ase_cnt = 0;

    ase_cnt += app_bap_uc_srv_get_specifc_state_ase_lid_list(con_lid, AOB_MGR_STREAM_STATE_ENABLING, ase_lid_list);

    ase_cnt += aob_media_get_curr_streaming_ase_lid_list(con_lid, &ase_lid_list[ase_cnt]);

    return ase_cnt;
}

uint8_t aob_media_get_enableing_ase_lid_list(uint8_t con_lid, uint8_t *ase_lid_list)
{
    return app_bap_uc_srv_get_specifc_state_ase_lid_list(con_lid, AOB_MGR_STREAM_STATE_ENABLING, ase_lid_list);
}

bool aob_media_is_device_any_ase_in_streamimg_state(uint8_t con_lid)
{
    uint8_t ase_lid_list[APP_BAP_DFT_ASCS_NB_ASE_CHAR] = {0};

    return app_bap_uc_srv_get_streaming_ase_lid_list(con_lid, ase_lid_list) != 0;
}

uint8_t aob_media_get_src_ase_streaming_device_list(uint8_t *p_con_lid_list)
{
    uint8_t cnt = 0;
    uint8_t con_lid = 0;

    for (con_lid = 0; con_lid < BLE_AUDIO_CONNECTION_CNT; con_lid++)
    {
        uint8_t ase_lid = aob_media_get_cur_streaming_ase_lid(con_lid, AOB_MGR_DIRECTION_SRC);

        if (ase_lid != APP_GAF_INVALID_CON_LID)
        {
            cnt++;

            if (p_con_lid_list != NULL)
            {
                p_con_lid_list[cnt] = con_lid;
            }
        }
    }

    return cnt;
}

AOB_MGR_CONTEXT_TYPE_BF_E aob_media_get_cur_context_type_by_ase_lid(uint8_t ase_lid)
{
    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info(ase_lid);
    if ((NULL != p_ase_info) && (NULL != p_ase_info->p_metadata))
    {
        return (AOB_MGR_CONTEXT_TYPE_BF_E)p_ase_info->p_metadata->param.context_bf;
    }

    return AOB_AUDIO_CONTEXT_TYPE_UNSPECIFIED;
}

AOB_MGR_CONTEXT_TYPE_BF_E aob_media_get_cur_context_type(uint8_t ase_lid)
{
    AOB_MGR_CONTEXT_TYPE_BF_E ctxType = AOB_AUDIO_CONTEXT_TYPE_UNSPECIFIED;

    uint8_t con_lid = app_audio_adm_get_le_audio_active_device();

    if (INVALID_CONNECTION_INDEX != con_lid)
    {
        for (uint32_t i = 0; i < AOB_MGR_DIRECTION_MAX; i++)
        {
            uint8_t _ase_lid = app_bap_uc_srv_get_streaming_ase_lid(con_lid, (enum app_gaf_direction)i);
            if (APP_GAF_INVALID_CON_LID != _ase_lid)
            {
                ctxType = aob_media_get_cur_context_type_by_ase_lid(_ase_lid);
                break;
            }
        }
    }
    return ctxType;
}

void aob_media_set_qos_info(uint8_t ase_lid, app_gaf_bap_qos_req_t *qos_info)
{
    app_bap_uc_srv_set_ase_qos_req(ase_lid, qos_info);
}

void aob_media_mics_set_mute(uint8_t mute)
{
    AOB_MOBILE_INFO_T *p_info = NULL;
    uint8_t conidx = app_audio_adm_get_le_audio_active_device();
    /// TODO:no active device
    if (BT_DEVICE_INVALID_ID == conidx)
    {
        LOG_E("%s no active device to set vol", __func__);
        conidx = 0;
    }
    p_info = ble_audio_earphone_info_get_mobile_info(conidx);
    if (NULL != p_info)
    {
        p_info->media_info.mic_mute = mute;
    }

    app_arc_mics_set_mute(mute);
}

APP_GAF_CODEC_TYPE_T aob_media_get_codec_type(uint8_t ase_lid)
{
    app_bap_ascs_ase_t *p_ascs_info = app_bap_uc_srv_get_ase_info(ase_lid);

    if (!p_ascs_info)
    {
        TRACE(2, "%s ASCS null, ase_lid %d", __func__, ase_lid);
        return APP_GAF_CODEC_TYPE_SIG_MAX;
    }

    return (APP_GAF_CODEC_TYPE_T)p_ascs_info->codec_id.codec_id[0];
}

GAF_BAP_SAMLLING_REQ_T aob_media_get_sample_rate(uint8_t ase_lid)
{
    app_bap_ascs_ase_t *p_ascs_info = app_bap_uc_srv_get_ase_info(ase_lid);
    uint8_t ret = APP_GAF_BAP_SAMPLE_FREQ_MAX;
    APP_GAF_CODEC_TYPE_T code_type = aob_media_get_codec_type(ase_lid);

    if (!p_ascs_info)
    {
        TRACE(2, "%s ASCS null, ase_lid %d", __func__, ase_lid);
        goto exit;
    }

    if (APP_GAF_CODEC_TYPE_LC3 == code_type)
    {
        ret = p_ascs_info->p_cfg->param.sampling_freq;

    }
    else
    {
#if defined(LC3PLUS_SUPPORT) || defined(HID_ULL_ENABLE)
        ret = p_ascs_info->p_cfg->param.sampling_freq;
#endif
        //TODO: Add vendor codec
    }

exit:
    return (GAF_BAP_SAMLLING_REQ_T)ret;
}

AOB_MEDIA_METADATA_CFG_T *aob_media_get_metadata(uint8_t ase_lid)
{
    AOB_MEDIA_METADATA_CFG_T *p_metadata_cfg = NULL;

    app_bap_ascs_ase_t *p_ascs_info = app_bap_uc_srv_get_ase_info(ase_lid);
    if (!p_ascs_info)
    {
        LOG_E("%s ASCS null, ase_lid %d", __func__, ase_lid);
        goto exit;
    }

    p_metadata_cfg = (AOB_MEDIA_METADATA_CFG_T *)&p_ascs_info->p_metadata->add_metadata.len;

exit:
    return p_metadata_cfg;
}

bool aob_media_is_exist_qos_configured_ase(void)
{
    uint8_t aseNum = app_bap_uc_srv_get_nb_ases_cfg();

    for (uint8_t i = 0; i < aseNum; i++)
    {
        if (aob_media_get_cur_ase_state(i) == AOB_MGR_STREAM_STATE_QOS_CONFIGURED)
        {
            return true;
        }
    }

    return false;
}

AOB_MGR_PLAYBACK_STATE_E aob_media_get_state(uint8_t con_lid)
{
    AOB_MEDIA_INFO_T *p_media_info = ble_audio_earphone_info_get_media_info(con_lid);

    if (!p_media_info)
    {
        return AOB_MGR_PLAYBACK_STATE_MAX;
    }

    return (AOB_MGR_PLAYBACK_STATE_E)p_media_info->media_state;
}

void aob_media_ascs_srv_set_codec(uint8_t ase_lid, const app_gaf_codec_id_t *codec_id,
                                  app_gaf_bap_qos_req_t *ntf_qos_req, app_gaf_bap_cfg_t *ntf_codec_cfg)
{
    if (!ntf_qos_req || !ntf_codec_cfg)
    {
        LOG_E("%s Err Params", __func__);
        return;
    }

    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info(ase_lid);

    if (p_ase_info->ase_state != APP_GAF_BAP_UC_ASE_STATE_QOS_CONFIGURED &&
            p_ase_info->ase_state != APP_GAF_BAP_UC_ASE_STATE_CODEC_CONFIGURED &&
            p_ase_info->ase_state != APP_GAF_BAP_UC_ASE_STATE_IDLE)
    {
        LOG_W("%s ase state: %d", __func__, p_ase_info->ase_state);
        return;
    }

    app_bap_uc_srv_configure_codec_ase_local(ase_lid, codec_id, ntf_qos_req, ntf_codec_cfg);
}

static void aob_media_stream_status_cb(uint8_t con_lid, uint8_t ase_lid, AOB_MGR_STREAM_STATE_E state)
{
    const BLE_AUD_CORE_EVT_CB_T *p_media_cb = ble_audio_get_evt_cb();

    if (NULL != p_media_cb && NULL != p_media_cb->ble_media_stream_status_change_cb)
    {
        p_media_cb->ble_media_stream_status_change_cb(con_lid, ase_lid, state);
    }
}

static void aob_media_playback_status_cb(uint8_t con_lid, AOB_MGR_PLAYBACK_STATE_E state)
{
    LOG_I("%s con_lid %d state %d", __func__, con_lid, state);

    AOB_MEDIA_INFO_T *p_media_info = ble_audio_earphone_info_get_media_info(con_lid);

    if (p_media_info == NULL)
    {
        LOG_E("%s con_lid %d media info is NULL", __func__, con_lid);
        return;
    }

    p_media_info->media_state = state;

    if (state == AOB_MGR_PLAYBACK_STATE_PLAYING)
    {
        app_audio_ctrl_update_ble_audio_music_state(con_lid, STRAEMING_START);
    }
    else if (state == AOB_MGR_PLAYBACK_STATE_PAUSED ||
             state == AOB_MGR_PLAYBACK_STATE_INACTIVE)
    {
        app_audio_ctrl_update_ble_audio_music_state(con_lid, STRAEMING_STOP);
    }

    const BLE_AUD_CORE_EVT_CB_T *p_media_cb = ble_audio_get_evt_cb();

    if (NULL != p_media_cb && NULL != p_media_cb->ble_media_playback_status_change_cb)
    {
        p_media_cb->ble_media_playback_status_change_cb(con_lid, state);
    }
}

static void aob_media_track_changed_cb(uint8_t con_lid)
{
    LOG_I("(d%d)%s", con_lid, __func__);

    const BLE_AUD_CORE_EVT_CB_T *p_media_cb = ble_audio_get_evt_cb();
    if (NULL != p_media_cb && NULL != p_media_cb->ble_media_track_change_cb)
    {
        p_media_cb->ble_media_track_change_cb(con_lid);
    }
}

uint8_t aob_media_get_mic_state(uint8_t con_lid)
{
    AOB_MOBILE_INFO_T *p_info = NULL;

    p_info = ble_audio_earphone_info_get_mobile_info(con_lid);
    if (NULL != p_info)
    {
        return p_info->media_info.mic_mute;
    }
    return 0;
}

static void aob_media_mics_state_cb(uint8_t mute)
{
    uint8_t conidx = 0;
    AOB_MOBILE_INFO_T *p_info = NULL;

    conidx = app_audio_adm_get_le_audio_active_device();
    /// TODO:no active device
    if (BT_DEVICE_INVALID_ID == conidx)
    {
        LOG_E("%s no active device to set vol", __func__);
        conidx = 0;
    }

    LOG_I("(d%d)%s mute:%d", conidx, __func__, mute);

    p_info = ble_audio_earphone_info_get_mobile_info(conidx);
    if (NULL != p_info)
    {
        p_info->media_info.mic_mute = mute;
    }
    const BLE_AUD_CORE_EVT_CB_T *p_media_cb = ble_audio_get_evt_cb();
    if (NULL != p_media_cb && NULL != p_media_cb->ble_media_mic_state_cb)
    {
        p_media_cb->ble_media_mic_state_cb(mute);
    }
}

static void aob_media_iso_link_quality_cb(void *event)
{
    const BLE_AUD_CORE_EVT_CB_T *p_media_cb = ble_audio_get_evt_cb();
    if (p_media_cb && NULL != p_media_cb->ble_media_iso_link_quality_cb)
    {
        p_media_cb->ble_media_iso_link_quality_cb(event);
    }
}

static void aob_media_pacs_cccd_written_cb(uint8_t con_lid)
{
    const BLE_AUD_CORE_EVT_CB_T *p_media_cb = ble_audio_get_evt_cb();
    if (p_media_cb && NULL != p_media_cb->ble_media_pacs_cccd_written_cb)
    {
        p_media_cb->ble_media_pacs_cccd_written_cb(con_lid);
    }
}

static void aob_media_ase_codec_cfg_req_handler_cb(uint8_t con_lid, app_bap_ascs_ase_t *p_ase_info, uint8_t tgt_latency,
                                                   const app_gaf_codec_id_t *codec_id, app_gaf_bap_cfg_t *codec_cfg)
{
    //Check is there an valid ase for codec cfg
    if (NULL == p_ase_info)
    {
        LOG_W("%s ava_ase_info error!", __func__);
        return ;
    }

    app_gaf_bap_qos_req_t *p_ntf_qos_req = (app_gaf_bap_qos_req_t *)bes_bt_buf_malloc(sizeof(app_gaf_bap_qos_req_t));

    if (p_ntf_qos_req == NULL)
    {
        LOG_E("%s no resources", __func__);
    }

    bool accept = false;
    // check qos req fill callback function is registed
    if (aob_ascs_srv_codec_req_handler_cb && p_ntf_qos_req)
    {
        // Fill Qos req using callback function, can modify codec_cfg in callback
        accept = aob_ascs_srv_codec_req_handler_cb(p_ase_info->direction, codec_id, tgt_latency, (app_gaf_bap_cfg_t *)codec_cfg, p_ntf_qos_req);
        const BLE_AUD_CORE_EVT_CB_T *p_media_cb = ble_audio_get_evt_cb();
        // If accept, fill local ase info
        if (accept && p_media_cb && p_media_cb->ble_ase_codec_cfg_req_cb)
        {
            // Customer may fill params in this callback
            p_media_cb->ble_ase_codec_cfg_req_cb(p_ase_info->ase_lid, (const AOB_CODEC_ID_T *)codec_id,
                                                 tgt_latency, (AOB_BAP_CFG_T *)codec_cfg, (AOB_BAP_QOS_REQ_T *)p_ntf_qos_req);
        }
        // Send codec cfg cfm rsp
        app_bap_uc_srv_send_configure_codec_rsp(accept, p_ase_info->ase_lid, codec_id, p_ntf_qos_req, codec_cfg);
        // Free qos req
        bes_bt_buf_free(p_ntf_qos_req);
    }
    else
    {
        //Fill any param that not NULL
        app_bap_uc_srv_send_configure_codec_rsp(false, p_ase_info->ase_lid, NULL, NULL, NULL);
    }
}

static void aob_ascs_ase_enable_req_handler_cb(uint8_t con_lid, uint8_t ase_lid, app_gaf_bap_cfg_metadata_t *context)
{

}

static void aob_ascs_ase_release_req_handler_cb(uint8_t con_lid, uint8_t ase_lid, app_gaf_bap_cfg_metadata_t *context)
{

}

static void aob_ascs_ase_update_metadata_req_handler_cb(uint8_t con_lid, uint8_t ase_lid, app_gaf_bap_cfg_metadata_t *context, uint8_t ase_state)
{
    const BLE_AUD_CORE_EVT_CB_T *p_media_cb = ble_audio_get_evt_cb();

    if (NULL != p_media_cb && NULL != p_media_cb->ble_ase_update_metadata_req_cb)
    {
        p_media_cb->ble_ase_update_metadata_req_cb(con_lid, ase_lid, (void *)context, ase_state);
    }
}

static media_event_handler_t media_event_cb =
{
    .media_track_change_cb                  = aob_media_track_changed_cb,
    .media_stream_status_change_cb          = aob_media_stream_status_cb,
    .media_playback_status_change_cb        = aob_media_playback_status_cb,
    .media_mic_state_cb                     = aob_media_mics_state_cb,
    .media_iso_link_quality_cb              = aob_media_iso_link_quality_cb,
    .media_pacs_cccd_written_cb             = aob_media_pacs_cccd_written_cb,
    .ase_codec_cfg_req_handler_cb           = aob_media_ase_codec_cfg_req_handler_cb,
    .ase_enable_req_handler_cb              = aob_ascs_ase_enable_req_handler_cb,
    .ase_release_req_handler_cb             = aob_ascs_ase_release_req_handler_cb,
    .ase_update_metadata_req_handler_cb     = aob_ascs_ase_update_metadata_req_handler_cb,
};

void aob_media_ascs_register_codec_req_handler_cb(get_qos_req_cfg_info_cb cb_func)
{
    LOG_I("%s [old cb] = %p, [new cb] = %p", __func__, aob_ascs_srv_codec_req_handler_cb, cb_func);
    aob_ascs_srv_codec_req_handler_cb = cb_func;
}

void aob_media_api_init(void)
{
    aob_mgr_media_evt_handler_register(&media_event_cb);
}

#ifdef AOB_MOBILE_ENABLED

/****************************for client(mobile)*****************************/
/************************private macro defination***************************/

/************************private type defination****************************/

/**********************private function declaration*************************/
static void aob_mobile_media_djob_timer_timeout(void const *para);

/************************private variable defination************************/

/****************************function defination****************************/
osTimerDef(aob_mobile_media_djob_timer, aob_mobile_media_djob_timer_timeout);
osTimerId   aob_mobile_media_djob_timer_id = NULL;
static AOB_MOBILE_MEDIA_ENV_T aob_mobile_media_env;
static aob_mobile_ava_ctx_changed_cb mobile_avalible_ctx_changed_cb = NULL;

static void aob_mobile_media_djob_proc(uint8_t con_lid)
{
    uint8_t statusBf = 0;

    if (!aob_mobile_media_djob_timer_id)
    {
        aob_mobile_media_djob_timer_id =
            osTimerCreate(osTimer(aob_mobile_media_djob_timer), osTimerOnce, NULL);
    }

    LOG_I("%s statusBf:0x%x, %d", __func__, aob_mobile_media_env.djobStatusBf,
          aob_mobile_media_env.info[con_lid].delayCnt);

    statusBf = aob_mobile_media_env.djobStatusBf & (0x1 << con_lid);
    if ((statusBf) && (!(--aob_mobile_media_env.info[con_lid].delayCnt)))
    {
        aob_mobile_media_env.djobStatusBf &= ~(0x1 << con_lid);
        app_bt_call_func_in_bt_thread((uint32_t)&aob_mobile_media_env.info[con_lid].pCfgInfo, (uint32_t)con_lid,
                                      (uint32_t)aob_mobile_media_env.info[con_lid].biDirection, 0,
                                      (uint32_t)aob_media_mobile_start_stream);
    }

    if (!aob_mobile_media_env.djobStatusBf)
    {
        aob_mobile_media_env.djobTimerStarted = false;
        osTimerStop(aob_mobile_media_djob_timer_id);
        goto exit;
    }

    if (!aob_mobile_media_env.djobTimerStarted)
    {
        aob_mobile_media_env.djobTimerStarted = true;
        osTimerStart(aob_mobile_media_djob_timer_id, 500);
    }

exit:
    return;
}

static void aob_mobile_media_djob_timer_timeout(void const *para)
{
    aob_mobile_media_env.djobTimerStarted = false;

    for (uint8_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++)
    {
        aob_mobile_media_djob_proc(i);
    }
}

void aob_media_mobile_start_ase_disvovery(uint8_t con_lid)
{
    app_bap_uc_cli_discovery_start(con_lid);
}

void aob_media_mobile_cfg_codec(uint8_t ase_lid, uint8_t cis_id,
                                const app_gaf_codec_id_t *codec_id,
                                uint8_t sampleRate_enum, uint16_t frame_octet)
{
    app_bap_uc_cli_configure_codec(ase_lid, cis_id,
                                   codec_id,
                                   sampleRate_enum, frame_octet);
}

void aob_media_mobile_configure_codec_with_cfg(uint8_t ase_lid, uint8_t cis_id,
                                                const app_gaf_codec_id_t *codec_id,
                                                const app_gaf_bap_cfg_t *p_cfg)
{
    app_bap_uc_cli_configure_codec_with_cfg(ase_lid, cis_id, codec_id, p_cfg);
}

void aob_media_mobile_cfg_qos(uint8_t ase_lid, uint8_t cig_id, uint16_t max_sdu_size)
{
    app_bap_uc_cli_link_create_group_req(cig_id);
    app_bap_uc_cli_configure_qos(ase_lid, cig_id, max_sdu_size);
}

void aob_media_mobile_set_sdu_interval(uint32_t sdu_intv_m2s_us, uint32_t sdu_intv_s2m_us)
{
    app_bap_update_sdu_intv(sdu_intv_m2s_us, sdu_intv_s2m_us);
}

void aob_media_mobile_set_cis_count_in_cig(uint8_t cis_num)
{
    app_bap_uc_cli_set_cis_num_in_cig(cis_num);
}

void aob_media_mobile_prepare_cig_param(const AOB_BAP_CIG_PARAM_T *cig_param)
{
    if (cig_param == NULL)
    {
        return;
    }

    CIS_TIMING_CONFIGURATION_T timming =
    {
        .m2s_bn = cig_param->c2p_bn,
        .m2s_nse = cig_param->c2p_nse,
        .m2s_ft = cig_param->c2p_ft,
        .s2m_bn = cig_param->p2c_bn,
        .s2m_nse = cig_param->p2c_nse,
        .s2m_ft = cig_param->p2c_ft,
        .frame_cnt_per_sdu = 1,
        .iso_interval = cig_param->iso_interval_1_25ms,
    };

    app_bap_update_cis_timing(&timming);
}

void aob_media_mobile_enable(uint8_t ase_lid, uint16_t stream_context_bf,
                             uint8_t ccid_num, const uint8_t *p_ccid_list)
{
    if (ccid_num == 0 || p_ccid_list == NULL)
    {
        app_bap_uc_cli_enable_stream(ase_lid, stream_context_bf);
    }
    else
    {
        app_bap_uc_cli_enable_stream_with_ccid(ase_lid,
                                               stream_context_bf, ccid_num, p_ccid_list);
    }
}

void aob_media_mobile_disable(uint8_t ase_lid)
{
    app_bap_uc_cli_stream_disable(ase_lid);
}

void aob_media_mobile_release(uint8_t ase_lid)
{
    app_bap_uc_cli_stream_release(ase_lid);
}

void aob_media_mobile_update_metadata(uint8_t ase_lid, app_gaf_bap_cfg_metadata_t *meta_data)
{
    app_bap_uc_cli_update_metadata(ase_lid, meta_data);
}

void aob_media_mobile_start_stream(AOB_MEDIA_ASE_CFG_INFO_T *pInfo, uint8_t con_lid, bool biDirection)
{
    POSSIBLY_UNUSED uint32_t ase_num_to_use = 0;
    AOB_MEDIA_ASE_CFG_INFO_T *pInfo_local = pInfo;
    uint8_t ase_lid = 0;
    uint8_t direction = APP_GAF_DIRECTION_MAX;
    uint8_t ase_lid_need[APP_GAF_DIRECTION_MAX][2] = {{APP_GAF_INVALID_CON_LID, APP_GAF_INVALID_CON_LID},
        {APP_GAF_INVALID_CON_LID, APP_GAF_INVALID_CON_LID}
    };

    if ((!pInfo) || (con_lid == INVALID_CONNECTION_INDEX)) {
        LOG_W("(d%d)%s error detected", con_lid, __func__);
        return;
    }

    if (!ble_audio_mobile_info_get(con_lid)) {
        LOG_W("(d%d)%s not connect", con_lid, __func__);
        return;
    }

    if (!app_bap_uc_cli_is_already_bonded(con_lid)) {
        LOG_W("(d%d)%s ASCS not bond", con_lid, __func__);
        return;
    }

    bool is_need_two_ase[APP_GAF_DIRECTION_MAX] = {true, true};
    // Check for is two ASEs should use to assign an cis per direction
    for (uint32_t idx = 0; idx < 1 + (uint32_t)biDirection; idx++)
    {
        // Device support stereo?
        is_need_two_ase[(pInfo + idx)->direction] = is_need_two_ase[(pInfo + idx)->direction]
                                                    && app_bap_capa_cli_is_peer_support_stereo_channel(con_lid, (pInfo + idx)->direction);
        // Codec record support stereo?
        is_need_two_ase[(pInfo + idx)->direction] = is_need_two_ase[(pInfo + idx)->direction]
                                                    && !app_bap_capa_cli_is_codec_capa_support_stereo_channel(con_lid, (pInfo + idx)->direction,
                                                            (pInfo + idx)->codec_id, (pInfo + idx)->sample_rate);
    }
    // search for all ase can be used
    for (uint32_t idx = 0; idx < 1 + (uint32_t)biDirection; idx++)
    {
        uint8_t ase_search_idx = 0;

        ase_lid = 0;
        // get ase_lid by con_lid and direction
        do
        {
            if ((ase_lid > app_bap_uc_cli_get_nb_ases_cfg() - 1))
            {
                // ase search is end but valid ase can not be get
                ase_lid = APP_GAF_INVALID_CON_LID;
                break;
            }

            const app_bap_ascc_ase_t *p_ase_info = app_bap_uc_cli_get_ase_info_by_ase_lid(ase_lid);

            if (p_ase_info == NULL)
            {
                // ase search is end but valid ase can not be get
                ase_lid = APP_GAF_INVALID_CON_LID;
                break;
            }

            if (p_ase_info->con_lid == con_lid &&
                    p_ase_info->direction == pInfo_local->direction)
            {
                LOG_I("%s, get ase lid %d, direction %d", __func__, ase_lid, p_ase_info->direction);
                ase_lid_need[p_ase_info->direction][ase_search_idx++] = ase_lid;

                if (!is_need_two_ase[p_ase_info->direction])
                {
                    // break for ase search done
                    break;
                }
            }

            ase_lid++;
        } while (ase_search_idx < 2);

        if (ase_lid == APP_GAF_INVALID_CON_LID)
        {
            LOG_E("ASCC can not get all valid ASE need, direction: %s, con_lid %d",
                            (pInfo_local->direction == APP_GAF_DIRECTION_SRC) ? "SRC" : "SINK", con_lid);
            is_need_two_ase[pInfo_local->direction] = false;
        }

        LOG_I("direction: %d, now we get ase lid need below:", pInfo_local->direction);
        DUMP8("[%d] ", ase_lid_need[pInfo_local->direction], 2);
        // move to next direction info
        pInfo_local++;
    }

    uint8_t extra_cis_num = 0;

    // update truly ase num and cis num should to be estab
    if (!biDirection)
    {
        // calculate ASE number that need to config codec
        ase_num_to_use = 1 + ((APP_GAF_DIRECTION_SRC == pInfo->direction) ?
                              (is_need_two_ase[APP_GAF_DIRECTION_SRC]) :
                              (is_need_two_ase[APP_GAF_DIRECTION_SINK]));
        // calculate extra CIS should include in CIG
        extra_cis_num = ((APP_GAF_DIRECTION_SRC == pInfo->direction) ?
                         (is_need_two_ase[APP_GAF_DIRECTION_SRC]) :
                         (is_need_two_ase[APP_GAF_DIRECTION_SINK]));
    }
    else
    {
        // calculate ASE number that need to config codec
        ase_num_to_use = 2 + is_need_two_ase[APP_GAF_DIRECTION_SINK] + is_need_two_ase[APP_GAF_DIRECTION_SRC];
        // calculate extra CIS should include in CIG, such ase BAP 6(i)
        extra_cis_num = co_max(is_need_two_ase[APP_GAF_DIRECTION_SINK], is_need_two_ase[APP_GAF_DIRECTION_SRC]);
    }

    // check is this connection use BAP 6(i), dual cis in one connection
    if (extra_cis_num == 1)
    {
        LOG_I("d(%d) is willing to use two cis for stereo channel", con_lid);
#if defined(BLE_AUDIO_CENTRAL_APP_ENABLED)
        LOG_W("d(%d) usb dongle do not support two cis per connection!!!", con_lid);
        return;
#endif
    }

    // increase cis num inluded in CIG
    // now we use 2 cis in dft prepare for dev rejoin
    // app_bap_uc_cli_increase_cis_num_in_cig(APP_BAP_DFT_ASCC_CIS_NUM);
    // init direction info
    pInfo_local = pInfo;
    uint8_t ret;

    // prepare for several ASEs stm
    for (uint32_t i = 0; i < 1 + (uint32_t)biDirection; i++)
    {
        direction = pInfo_local->direction;
        /// check for all ase need to be estab
        for (uint32_t idx = 0; idx < 2; idx++)
        {
            if (ase_lid_need[direction][idx] == APP_GAF_INVALID_CON_LID)
            {
                continue;
            }
            ret = lea_ase_stream_start(con_lid, con_lid + idx,
                    ase_lid_need[direction][idx], biDirection, (LEA_ASE_STM_CFG_INFO_T*)pInfo_local);
            if (!ret)
            {
                return;
            }
        }
        // move to next direction info
        pInfo_local++;
    }

    return;
}

void aob_media_mobile_release_stream(uint8_t ase_lid)
{
    lea_ase_stm_send_msg(ase_lid, LEA_REQ_ASE_RELEASE, ase_lid, 0);
}

void aob_media_mobile_release_all_stream_for_specifc_direction(uint8_t con_lid, uint8_t direction)
{
    uint8_t nb_ase_streaming = 0;
    uint8_t nb_ase_enabling = 0;
    uint8_t ase_lid_list[APP_BAP_DFT_ASCC_NB_ASE_CFG] = {0};

    nb_ase_streaming = app_bap_uc_cli_get_specific_state_ase_lid_list(con_lid, direction,
                                                                      APP_GAF_BAP_UC_ASE_STATE_STREAMING,
                                                                      ase_lid_list);
    /// disable all ase in streaming state
    for (uint32_t ase_idx = 0; ase_idx < nb_ase_streaming; ase_idx++)
    {
        aob_media_mobile_release_stream(ase_lid_list[ase_idx]);
    }

    nb_ase_enabling = app_bap_uc_cli_get_specific_state_ase_lid_list(con_lid, direction,
                                                                     APP_GAF_BAP_UC_ASE_STATE_ENABLING,
                                                                     ase_lid_list);
    /// disable all ase in enabling state
    for (uint32_t ase_idx = 0; ase_idx < nb_ase_enabling; ase_idx++)
    {
        aob_media_mobile_release_stream(ase_lid_list[ase_idx]);
    }
}

void aob_media_mobile_disable_stream(uint8_t ase_lid)
{
    lea_ase_stm_send_msg(ase_lid, LEA_REQ_ASE_DISABLE, (uint32_t)ase_lid, 0);
}

void aob_media_mobile_disable_all_stream_for_specifc_direction(uint8_t con_lid, uint8_t direction)
{
    uint8_t nb_ase_streaming = 0;
    uint8_t nb_ase_enabling = 0;
    uint8_t ase_lid_list[APP_BAP_DFT_ASCC_NB_ASE_CFG] = {0};

    nb_ase_streaming = app_bap_uc_cli_get_specific_state_ase_lid_list(con_lid, direction,
                                                                      APP_GAF_BAP_UC_ASE_STATE_STREAMING,
                                                                      ase_lid_list);
    /// disable all ase in streaming state
    for (uint32_t ase_idx = 0; ase_idx < nb_ase_streaming; ase_idx++)
    {
        aob_media_mobile_disable_stream(ase_lid_list[ase_idx]);
    }

    nb_ase_enabling = app_bap_uc_cli_get_specific_state_ase_lid_list(con_lid, direction,
                                                                     APP_GAF_BAP_UC_ASE_STATE_ENABLING,
                                                                     ase_lid_list);
    /// disable all ase in enabling state
    for (uint32_t ase_idx = 0; ase_idx < nb_ase_enabling; ase_idx++)
    {
        aob_media_mobile_disable_stream(ase_lid_list[ase_idx]);
    }
}

void aob_media_mobile_enable_stream(uint8_t ase_lid)
{
    lea_ase_stm_send_msg(ase_lid, LEA_REQ_ASE_ENABLE, (uint32_t)ase_lid, 0);
}

void aob_media_mobile_set_player_name(uint8_t media_lid, uint8_t *name, uint8_t name_len)
{
    app_acc_mcs_set_player_name_req(media_lid, name, name_len);
}

void aob_media_mobile_change_track(uint8_t media_lid, uint32_t duration, uint8_t *title, uint8_t title_len)
{
    app_acc_mcs_track_change_req(media_lid, duration, title, title_len);
}

uint8_t aob_media_mobile_get_cur_streaming_ase_lid(uint8_t con_lid, AOB_MGR_DIRECTION_E direction)
{
    return app_bap_uc_cli_get_streaming_ase_lid(con_lid, (enum app_gaf_direction)direction);
}

void aob_media_mobile_micc_set_mute(uint8_t con_lid, uint8_t mute)
{
    app_arc_micc_set_mute(con_lid, mute);
}

void aob_media_mobile_micc_read_mute(uint8_t con_lid)
{
    app_arc_micc_read_mute(con_lid);
}

AOB_MGR_STREAM_STATE_E aob_media_mobile_get_cur_ase_state(uint8_t ase_lid)
{
    app_bap_ascc_ase_t *p_ascc_ase = app_bap_uc_cli_get_ase_info_by_ase_lid(ase_lid);

    return (p_ascc_ase != NULL) ? (AOB_MGR_STREAM_STATE_E)p_ascc_ase->ase_state : AOB_MGR_STREAM_STATE_MAX;
}

void aob_media_mobile_action_control(uint8_t media_lid, uint8_t action)
{
    app_acc_mcs_action_req(media_lid, (uint8_t)action,
                           media_player_state[media_lid].track_position, MCP_MEDIA_PLAYER_SEEKING_SPEED);
}

void aob_media_mobile_position_ctrl(uint8_t media_lid, uint8_t action, int32_t track_pos, int8_t seeking_speed)
{
    media_player_state[media_lid].track_position = track_pos;
    app_acc_mcs_action_req(media_lid, action, track_pos, seeking_speed);
}

static void aob_media_mobile_capa_changed_cb(uint8_t con_lid, uint8_t type)
{
    TRACE(3, "%s con_lid %d added codec %d", __func__, con_lid, type);
}

static void aob_media_mobile_stream_status_cb(uint8_t con_lid, uint8_t ase_lid, AOB_MGR_STREAM_STATE_E state)
{
    switch (state)
    {
        case AOB_MGR_STREAM_STATE_STREAMING:
#ifdef HID_ULL_ENABLE
        {
            app_bap_ascs_ase_t *p_ascs_ase = app_bap_uc_srv_get_ase_info(ase_lid);

            if (GAF_CODEC_TYPE_ULL == p_ascs_ase->codec_id.codec_id[0])
            {
                gaf_mobile_ull_stream_update_and_start_handler(ase_lid);
            }
        }
#else
        gaf_mobile_audio_stream_update_and_start_handler(ase_lid, con_lid);
#endif
        break;
        case AOB_MGR_STREAM_STATE_DISABLING:
#ifdef HID_ULL_ENABLE
        {
            app_bap_ascs_ase_t *p_ascs_ase = app_bap_uc_srv_get_ase_info(ase_lid);

            if (GAF_CODEC_TYPE_ULL == p_ascs_ase->codec_id.codec_id[0])
            {
                gaf_mobile_ull_stream_update_and_stop_handler(ase_lid);
            }
        }
#else
        gaf_mobile_audio_stream_update_and_stop_handler(ase_lid, con_lid);
#endif
        break;
        default:
            LOG_I("%s,not fine state=%d,con_lid=%d,ase_lid=%d", __func__, state, con_lid, ase_lid);
            break;
    }
}

#if 0
static void aob_media_mobile_seeking_timer_cb(void *param)
{
    uint8_t media_lid = (uint32_t)param;
    // Seeking timer timeout
    LOG_I("%s media_lid = %d", __func__, media_lid);
    // Update track position after seeking
    if (media_player_state[media_lid].media_state == AOB_MEDIA_PLAYING)
    {
        app_acc_mcs_action_req(media_lid, AOB_MGR_MCS_ACTION_PLAY,
                               media_player_state[media_lid].track_position, 0);
    }
    else
    {
        app_acc_mcs_action_req(media_lid, AOB_MGR_MCS_ACTION_PAUSE,
                               media_player_state[media_lid].track_position, 0);
    }
}

static void aob_media_mobile_start_seeking_timer(uint8_t media_lid)
{
    // Seeking timer start
    LOG_I("%s media_lid = %d", __func__, media_lid);

    if (media_player_state[media_lid].seeking_timer != 0)
    {
        co_timer_stop(&media_player_state[media_lid].seeking_timer);
        co_timer_del(&media_player_state[media_lid].seeking_timer);
    }

    // Start seeking timer to update track position
    co_timer_new(&media_player_state[media_lid].seeking_timer,
                 MCP_MEDIA_SEEKING_TIMEOUT_MS,
                 aob_media_mobile_seeking_timer_cb, (void *)(uint32_t)media_lid, 1);
    co_timer_start(&media_player_state[media_lid].seeking_timer);
}
#endif

static void aob_media_mobile_control_cb(uint8_t con_lid, uint8_t media_lid, AOB_MGR_MC_OPCODE_E opCode, int32_t val)
{
    /// TODO: Just a sample
    LOG_I("%s con_lid %d media_lid = %d, opcode %d", __func__, con_lid, media_lid, opCode);

    switch (opCode)
    {
        case AOB_MGR_MC_OP_PLAY:
        {
            // Confirm play
            app_acc_mcs_control_req_cfm(BT_STS_SUCCESS, media_lid, AOB_MGR_MCS_ACTION_PLAY,
                                        media_player_state[media_lid].track_position, 0);

            media_player_state[media_lid].media_state = AOB_MEDIA_PLAYING;

            for (uint8_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++)
            {
                AOB_MEDIA_ASE_CFG_INFO_T ase_to_start =
                {
                    APP_GAF_BAP_SAMPLE_FREQ_48000, 120, APP_GAF_DIRECTION_SINK, AOB_CODEC_ID_LC3, APP_GAF_BAP_CONTEXT_TYPE_MEDIA
                };

                aob_media_mobile_start_stream(&ase_to_start, i, false);
            }
        }
        break;
        case AOB_MGR_MC_OP_PAUSE:
        {
            // Confirm pause
            app_acc_mcs_control_req_cfm(BT_STS_SUCCESS, media_lid, AOB_MGR_MCS_ACTION_PAUSE,
                                        media_player_state[media_lid].track_position, 0);
            media_player_state[media_lid].media_state = AOB_MEDIA_PAUSED;

            for (uint8_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++)
            {
                aob_media_mobile_disable_all_stream_for_specifc_direction(
                    i, APP_GAF_DIRECTION_SINK);
            }
        }
        break;
        case AOB_MGR_MC_OP_STOP:
        {
            // Confirm stop
            app_acc_mcs_control_req_cfm(BT_STS_SUCCESS, media_lid, AOB_MGR_MCS_ACTION_STOP, 0, 0);
            // Update track position
            media_player_state[media_lid].track_position = 0;
            media_player_state[media_lid].media_state = AOB_MEDIA_PAUSED;

            for (uint8_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++)
            {
                aob_media_mobile_release_all_stream_for_specifc_direction(
                    i, APP_GAF_DIRECTION_SINK);
            }
        }
        break;
        case AOB_MGR_MC_OP_FAST_RW:
        {
            // Confirm seeking
            app_acc_mcs_control_req_cfm(BT_STS_SUCCESS, media_lid, AOB_MGR_MCS_ACTION_SEEK,
                                        0, (0 - MCP_MEDIA_PLAYER_SEEKING_SPEED));

            media_player_state[media_lid].media_state = AOB_MEDIA_SEEKING;

            // Update track position
            media_player_state[media_lid].track_position -= MCP_MEDIA_SEEKING_INTERVAL_10MS;

            if (media_player_state[media_lid].track_position < 0)
            {
                media_player_state[media_lid].track_position = MCP_MEDIA_STARTING_OF_SEGMENT(0);
            }

#if 0
            aob_media_mobile_start_seeking_timer(media_lid);
#endif
        }
        break;
        case AOB_MGR_MC_OP_FAST_FW:
        {
            // Confirm seeking
            app_acc_mcs_control_req_cfm(BT_STS_SUCCESS, media_lid, AOB_MGR_MCS_ACTION_SEEK,
                                        0, MCP_MEDIA_PLAYER_SEEKING_SPEED);

            media_player_state[media_lid].media_state = AOB_MEDIA_SEEKING;

            // Update track position
            media_player_state[media_lid].track_position += MCP_MEDIA_SEEKING_INTERVAL_10MS;

            if (media_player_state[media_lid].track_position > MCP_MEDIA_DEFAULT_TRACK_DUR_10MS)
            {
                media_player_state[media_lid].track_position = MCP_MEDIA_DEFAULT_TRACK_DUR_10MS;
            }

#if 0
            aob_media_mobile_start_seeking_timer(media_lid);
#endif
        }
        break;
        case AOB_MGR_MC_OP_MOVE_RELATIVE:
        {
            if (media_player_state[media_lid].track_position + val >= MCP_MEDIA_DEFAULT_TRACK_DUR_10MS)
            {
                media_player_state[media_lid].track_position = MCP_MEDIA_DEFAULT_TRACK_DUR_10MS;
            }
            else if (media_player_state[media_lid].track_position + val <= 0)
            {
                media_player_state[media_lid].track_position =  MCP_MEDIA_STARTING_OF_SEGMENT(0);
            }
            else
            {
                media_player_state[media_lid].track_position += val;
            }
            // Confirm move relative
            app_acc_mcs_control_req_cfm(BT_STS_SUCCESS, media_lid, AOB_MGR_MCS_ACTION_NO_ACTION,
                                        media_player_state[media_lid].track_position, 0);
        }
        break;
        case AOB_MGR_MC_OP_GOTO_SEG:
        {
            int32_t track_pos = 0;

            if (val != 0)
            {
                track_pos = val < 0 ?
                            MCP_MEDIA_STARTING_OF_SEGMENT(MCP_MEDIA_DEFAULT_TRACK_DUR_10MS - MCP_MEDIA_PLAYBACK_SEGMENT_10MS) +
                            (val + 1) * MCP_MEDIA_PLAYBACK_SEGMENT_10MS
                            : MCP_MEDIA_STARTING_OF_SEGMENT(0) +
                            (val - 1) * MCP_MEDIA_PLAYBACK_SEGMENT_10MS;

                if (track_pos < 0)
                {
                    track_pos = 0;
                }
                else if (track_pos > MCP_MEDIA_DEFAULT_TRACK_DUR_10MS)
                {
                    track_pos = MCP_MEDIA_DEFAULT_TRACK_DUR_10MS;
                }

                media_player_state[media_lid].track_position = track_pos;
            }
            else
            {
                track_pos = media_player_state[media_lid].track_position;
            }
            // Confirm Segement
            app_acc_mcs_control_req_cfm(BT_STS_SUCCESS, media_lid, AOB_MGR_MCS_ACTION_NO_ACTION, track_pos, 0);
        }
        break;
        case AOB_MGR_MC_OP_PREV_SEG:
        {
            if (media_player_state[media_lid].track_position - MCP_MEDIA_PLAYBACK_SEGMENT_10MS <= 0)
            {
                // Starting of first segment
                media_player_state[media_lid].track_position =  MCP_MEDIA_STARTING_OF_SEGMENT(0);
            }
            else
            {
                // Starting of previous segment
                media_player_state[media_lid].track_position = MCP_MEDIA_STARTING_OF_SEGMENT(media_player_state[media_lid].track_position)
                                                               - MCP_MEDIA_PLAYBACK_SEGMENT_10MS;
            }
            // Confirm Segement
            app_acc_mcs_control_req_cfm(BT_STS_SUCCESS, media_lid, AOB_MGR_MCS_ACTION_NO_ACTION,
                                        media_player_state[media_lid].track_position, 0);
        }
        break;
        case AOB_MGR_MC_OP_NEXT_SEG:
        {
            if (media_player_state[media_lid].track_position + MCP_MEDIA_PLAYBACK_SEGMENT_10MS >= MCP_MEDIA_DEFAULT_TRACK_DUR_10MS)
            {
                // Ending of last segment
                media_player_state[media_lid].track_position = MCP_MEDIA_DEFAULT_TRACK_DUR_10MS;
            }
            else
            {
                // Starting of next segment
                media_player_state[media_lid].track_position = MCP_MEDIA_STARTING_OF_SEGMENT(media_player_state[media_lid].track_position)
                                                               + MCP_MEDIA_PLAYBACK_SEGMENT_10MS;
            }
            // Confirm Segement
            app_acc_mcs_control_req_cfm(BT_STS_SUCCESS, media_lid, AOB_MGR_MCS_ACTION_NO_ACTION,
                                        media_player_state[media_lid].track_position, 0);
        }
        break;
        case AOB_MGR_MC_OP_FIRST_SEG:
        {
            // Starting of first segment
            media_player_state[media_lid].track_position =  MCP_MEDIA_STARTING_OF_SEGMENT(0);
            // Confirm seeking
            app_acc_mcs_control_req_cfm(BT_STS_SUCCESS, media_lid, AOB_MGR_MCS_ACTION_NO_ACTION,
                                        MCP_MEDIA_STARTING_OF_SEGMENT(media_player_state[media_lid].track_position), 0);
        }
        break;
        case AOB_MGR_MC_OP_LAST_SEG:
        {
            // Starting of last segment
            media_player_state[media_lid].track_position = MCP_MEDIA_DEFAULT_TRACK_DUR_10MS - MCP_MEDIA_PLAYBACK_SEGMENT_10MS;
            // Confirm seeking
            app_acc_mcs_control_req_cfm(BT_STS_SUCCESS, media_lid, AOB_MGR_MCS_ACTION_NO_ACTION,
                                        MCP_MEDIA_STARTING_OF_SEGMENT(media_player_state[media_lid].track_position), 0);
        }
        break;
        case AOB_MGR_MC_OP_PREV_TRACK:
        case AOB_MGR_MC_OP_NEXT_TRACK:
        case AOB_MGR_MC_OP_FIRST_TRACK:
        case AOB_MGR_MC_OP_LAST_TRACK:
        case AOB_MGR_MC_OP_GOTO_TRACK:
        case AOB_MGR_MC_OP_PREV_GROUP:
        case AOB_MGR_MC_OP_NEXT_GROUP:
        case AOB_MGR_MC_OP_FIRST_GROUP:
        case AOB_MGR_MC_OP_LAST_GROUP:
        case AOB_MGR_MC_OP_GOTO_GROUP:
        {
            media_player_state[media_lid].track_position = MCP_MEDIA_STARTING_OF_SEGMENT(0);
            // Confirm change track
            app_acc_mcs_control_req_cfm(BT_STS_SUCCESS, media_lid, AOB_MGR_MCS_ACTION_CHANGE_TRACK, 0, 0);
            // Check value
            if (((opCode == AOB_MGR_MC_OP_GOTO_SEG) ||
                    (opCode == AOB_MGR_MC_OP_GOTO_TRACK) ||
                    (opCode == AOB_MGR_MC_OP_GOTO_GROUP)) && val == 0)
            {
                LOG_W("Reset Track Position to zero but do not change track");
            }
            else
            {
                // Change track info
                aob_media_mobile_change_track(media_lid, MCP_MEDIA_DEFAULT_TRACK_DUR_10MS, NULL, 0);
            }
            // Check continue to play
            if (media_player_state[media_lid].media_state == AOB_MEDIA_PLAYING)
            {
                aob_media_mobile_action_control(media_lid, AOB_MGR_MCS_ACTION_PLAY);
            }
        }
        break;
        default:
        {
            // Confirm invalid
            app_acc_mcs_control_req_cfm(BT_STS_FAILED, media_lid, AOB_MGR_MCS_ACTION_NO_ACTION, 0, 0);
        }
        break;
    }
}

static void aob_media_mobile_val_get_cb(uint8_t con_lid, uint8_t media_lid, uint8_t char_type, uint16_t offset)
{
    POSSIBLY_UNUSED uint16_t val_len = 0;
    POSSIBLY_UNUSED const char *p_val_buf = NULL;

    if (AOB_MGR_MC_CHAR_TYPE_PLAYER_NAME == char_type)
    {
        val_len = sizeof(APP_ACC_DFT_MCS_PLAYER_NAME);
        p_val_buf = APP_ACC_DFT_MCS_PLAYER_NAME;
    }
    else if (AOB_MGR_MC_CHAR_TYPE_PLAYER_ICON_URL == char_type)
    {
        val_len = sizeof(APP_ACC_DFT_ICON_URL);
        p_val_buf = APP_ACC_DFT_ICON_URL;
    }
    else if (AOB_MGR_MC_CHAR_TYPE_TRACK_TITLE == char_type)
    {
        val_len = sizeof(APP_ACC_DFT_TITLE_NAME);
        p_val_buf = APP_ACC_DFT_TITLE_NAME;
    }
    else if (AOB_MGR_MC_CHAR_TYPE_TRACK_POSITION == char_type)
    {
        val_len = sizeof(media_player_state[media_lid].track_position);
        p_val_buf = (char *)&media_player_state[media_lid].track_position;
    }
    app_acc_mcs_val_get_req_cfm(BT_STS_SUCCESS, con_lid, media_lid,
                                (uint8_t *)(p_val_buf + offset), (val_len - offset));
}

static void aob_media_mobile_val_set_cb(uint8_t con_lid, uint8_t media_lid, uint8_t char_type, uint32_t val)
{
    if (char_type == AOB_MGR_MC_CHAR_TYPE_TRACK_POSITION)
    {
        int32_t track_pos = val;

        if (media_player_state[media_lid].media_state == AOB_MEDIA_INACTIVE)
        {
            track_pos = 0;
        }
        else
        {
            track_pos = track_pos > 0 ? media_player_state[media_lid].track_position + track_pos :
                        MCP_MEDIA_DEFAULT_TRACK_DUR_10MS + track_pos;

            if (track_pos > MCP_MEDIA_DEFAULT_TRACK_DUR_10MS)
            {
                track_pos = MCP_MEDIA_DEFAULT_TRACK_DUR_10MS;
            }
            else if (track_pos < 0)
            {
                track_pos = 0;
            }
        }

        media_player_state[media_lid].track_position = track_pos;

        val = track_pos;
    }
    app_acc_mcs_val_set_req_cfm(BT_STS_SUCCESS, media_lid, val);
}

static void aob_media_mobile_pac_found_cb(uint8_t con_lid)
{
    gaf_bap_activity_type_e type = app_bap_get_activity_type();

    if (type == GAF_BAP_ACT_TYPE_CIS_AUDIO)
    {
        aob_media_mobile_start_ase_disvovery(con_lid);
    }
}

static void aob_media_mobile_ase_found_cb(uint8_t con_lid, uint8_t nb_ases)
{
    TRACE(3, "%s con_lid %d %d ASEs found", __func__, con_lid, nb_ases);

#if (!defined(HID_ULL_ENABLE)) && defined(BLE_AUDIO_CENTRAL_APP_ENABLED)
    uint8_t sink_ase_lid = app_bap_uc_cli_get_ase_lid(con_lid,
                                                      APP_GAF_BAP_UC_ASE_STATE_IDLE, APP_GAF_DIRECTION_SINK);
    uint8_t src_ase_lid = app_bap_uc_cli_get_ase_lid(con_lid,
                                                     APP_GAF_BAP_UC_ASE_STATE_IDLE, APP_GAF_DIRECTION_SRC);

    if (sink_ase_lid != APP_GAF_INVALID_CON_LID &&
            src_ase_lid != APP_GAF_INVALID_CON_LID)
    {
        // Only NEW STACK need check resume flag because ase found event will report serveral times
        if (already_resume_usb_dongle_bf & CO_BIT_MASK(con_lid))
        {
            return;
        }

        already_resume_usb_dongle_bf |= CO_BIT_MASK(con_lid);
        ble_audio_central_stream_resume_ble_audio(con_lid);
    }
    else
    {
        already_resume_usb_dongle_bf &= ~CO_BIT_MASK(con_lid);
    }
#endif
}

static void aob_media_mobile_grp_state_change_cb(bool isCreate, uint8_t ase_lid, uint16_t status, uint8_t grp_lid)
{
    if (isCreate)
    {
        lea_ase_stm_send_msg_by_grp_lid(grp_lid, LEA_EVT_ASE_UC_GRP_CREATED, status);
    }
    else
    {
        lea_ase_stm_send_msg_by_grp_lid(grp_lid, LEA_EVT_ASE_UC_GRP_REMOVED, status);
    }
}

static void aob_media_mobile_micc_state_cb(uint8_t mute)
{
    TRACE(3, "%s micc %d", __func__, mute);
}

static void aob_media_mobile_ava_ctx_changed_cb(uint8_t con_lid,
                                                AOB_MGR_CONTEXT_TYPE_BF_E sink_ava_ctx,
                                                AOB_MGR_CONTEXT_TYPE_BF_E src_ava_ctx)
{
    LOG_I("%s [%d] ava ctx changed sink 0x%x src 0x%x", __func__, con_lid,
          sink_ava_ctx, src_ava_ctx);
    if (mobile_avalible_ctx_changed_cb)
    {
        mobile_avalible_ctx_changed_cb(con_lid, (uint16_t)sink_ava_ctx, (uint16_t)src_ava_ctx);
    }
}

static media_mobile_event_handler_t media_mobile_event_cb =
{
    .media_stream_status_change_cb = aob_media_mobile_stream_status_cb,
    .media_codec_capa_change_cb = aob_media_mobile_capa_changed_cb,
    .media_control_cb = aob_media_mobile_control_cb,
    .media_val_get_cb = aob_media_mobile_val_get_cb,
    .media_val_set_cb = aob_media_mobile_val_set_cb,
    .media_pac_found_cb = aob_media_mobile_pac_found_cb,
    .media_ava_context_changed_cb = aob_media_mobile_ava_ctx_changed_cb,
    .media_ase_found_cb = aob_media_mobile_ase_found_cb,
    .media_cis_group_state_change_cb = aob_media_mobile_grp_state_change_cb,
    .media_mic_state_cb = aob_media_mobile_micc_state_cb,
};

void aob_media_mobile_api_init(void)
{
    aob_mgr_gaf_mobile_media_evt_handler_register(&media_mobile_event_cb);
}

void aob_mobile_ava_ctx_changed_cb_init(aob_mobile_ava_ctx_changed_cb cb)
{
    mobile_avalible_ctx_changed_cb = cb;
}
#endif
