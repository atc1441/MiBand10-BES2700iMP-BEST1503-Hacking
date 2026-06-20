/**
 * @file aob_bis_api.cpp
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
#include "cmsis.h"
#include "cmsis_os.h"
#include "aob_bis_api.h"
#include "hal_trace.h"
#include "ble_app_dbg.h"
#include "hal_aud.h"
#include "plat_types.h"
#include "heap_api.h"
#include "bt_drv_reg_op.h"
#include "app_ble.h"
#include "isoohci_int.h"
#include "ble_audio_earphone_info.h"
#include "ble_audio_core_api.h"

#include "aob_stream_handler.h"
#include "aob_media_api.h"
#include "aob_bis_api.h"

#include "gaf_bis_media_stream.h"
#include "gaf_bis_external_media_stream.h"

#include "app_bap.h"
#include "app_bap_bc_scan_msg.h"
#include "app_gaf.h"
#include "app_gaf_define.h"
#include "app_gaf_custom_api.h"
#include "app_bap_bc_src_msg.h"
#include "app_bap_bc_assist_msg.h"

#ifdef USB_BIS_AUDIO_STREAM
#include "app_usb_bis_stream.h"
#endif

/****************************for server(earbud)*****************************/

/*********************external function declaration*************************/

/************************private macro defination***************************/
#define AOB_BIS_TWS_SYNC_BCAST_ID_LEN   3
#define AOB_BIS_TWS_SYNC_BCAST_CODE_LEN 16

/************************private type defination****************************/

/**********************private function declaration*************************/

/************************private variable defination************************/
#ifdef APP_BLE_BIS_SRC_ENABLE
typedef struct
{
    uint32_t bis_stream_bf;
    uint8_t  bcast_id[APP_GAP_BCAST_ID_LEN];
    uint8_t  bcast_code[APP_GAP_KEY_LEN];
    aob_bis_src_started_info_t *bis_src_external_info;
} aob_bis_src_env_t;

aob_bis_src_env_t aob_bis_src_env = {0};
#endif

#ifdef APP_BLE_BIS_SINK_ENABLE
typedef struct
{
    uint8_t  grp_lid;
    uint32_t bis_stream_bf;
    uint32_t ble_bis_select_bf;
    uint8_t  bcast_id[APP_GAP_BCAST_ID_LEN];
    uint8_t  bcast_code[APP_GAP_KEY_LEN];
    struct
    {
        void (*bis_sink_started_callback)(uint8_t grp_lid);
        void (*bis_sink_stoped_callback)(uint8_t grp_lid, uint16_t err_code);
    } event_callback;
} aob_bis_sink_env_t;

static aob_bis_sink_env_t aob_bis_sink_env = {0};
#endif

#if defined(APP_BLE_BIS_ASSIST_ENABLE)
typedef struct
{
    uint8_t  bcast_id[APP_GAP_BCAST_ID_LEN];
    uint8_t  bcast_code[APP_GAP_KEY_LEN];
} aob_bis_assist_env_t;

static aob_bis_assist_env_t aob_bis_assist_env = {0};
#endif

#if defined(APP_BLE_BIS_ASSIST_ENABLE) || defined(APP_BLE_BIS_DELEG_ENABLE) || defined(APP_BLE_BIS_SINK_ENABLE)
typedef struct
{
    uint8_t  pa_lid;
    uint8_t  bcast_id[APP_GAP_BCAST_ID_LEN];
    uint8_t  addr[6];
    void (*bis_scan_state_cb)(bool scan_or_pa_sync, bool started, uint32_t param);
    void (*bis_scan_metadata_cb)(uint8_t subgrp_lid, uint8_t *buf, uint8_t buf_len);
    bool (*bis_scan_select_source_cb)(ble_bdaddr_t *addr, uint8_t adv_sid, uint8_t *bcast_id,
                                            uint8_t *adv_data, uint8_t adv_data_len, int8_t rssi);
} aob_bis_scan_env_t;

static aob_bis_scan_env_t aob_bis_scan_env = {0};
#endif

#ifdef CFG_BAP_BC
USED static const BLE_AUD_CORE_EVT_CB_T *p_ble_core_evt_cb  = NULL;
#endif

#ifdef APP_BLE_BIS_SRC_ENABLE
/*BAP Broadcast Source APIs*/
void aob_bis_src_set_big_param(uint8_t big_idx, aob_bis_src_big_param_t *p_big_param)
{
    app_bap_bc_src_grp_param big_param = {{0}};

    memcpy(big_param.bcast_id.id, p_big_param->bcast_id, APP_GAF_BAP_BC_BROADCAST_ID_LEN);
    big_param.nb_streams                     = p_big_param->nb_streams;
    big_param.nb_subgroups                     = p_big_param->nb_subgroups;
    big_param.grp_param.sdu_intv_us            = p_big_param->sdu_intv_us;
    big_param.grp_param.max_sdu                = p_big_param->max_sdu;
    big_param.grp_param.packing                = APP_BAP_DFT_BC_SRC_PACKING_TYPE;
    big_param.grp_param.framing                = APP_BAP_DFT_BC_SRC_FRAMING_TYPE;
    big_param.grp_param.phy_bf                 = APP_BAP_DFT_BC_SRC_PHY_BF;
    big_param.grp_param.test                   = p_big_param->test;
    if (p_big_param->test)
    {
        big_param.grp_param.iso_intv_frame     = p_big_param->test_big_param.iso_intv_frame;
        big_param.grp_param.nse                = p_big_param->test_big_param.nse;
        big_param.grp_param.max_pdu            = p_big_param->test_big_param.max_pdu;
        big_param.grp_param.bn                 = p_big_param->test_big_param.bn;
        big_param.grp_param.irc                = p_big_param->test_big_param.irc;
        big_param.grp_param.pto                = p_big_param->test_big_param.pto;
    }
    else
    {
        big_param.grp_param.max_tlatency_ms    = p_big_param->big_param.max_tlatency_ms;
        big_param.grp_param.rtn                = p_big_param->big_param.rtn;
    }
    big_param.adv_param.adv_intv_min_slot      = APP_BAP_DFT_BC_SRC_ADV_INTERVAL;
    big_param.adv_param.adv_intv_max_slot      = APP_BAP_DFT_BC_SRC_ADV_INTERVAL;
    big_param.adv_param.chnl_map               = ADV_ALL_CHNLS_EN;
    big_param.adv_param.phy_prim               = APP_PHY_1MBPS_BIT;
    big_param.adv_param.phy_second             = APP_PHY_1MBPS_BIT;
    big_param.adv_param.adv_sid                = 1;
    big_param.per_adv_param.adv_intv_min_frame = APP_BAP_DFT_BC_SRC_PERIODIC_INTERVAL;
    big_param.per_adv_param.adv_intv_max_frame = APP_BAP_DFT_BC_SRC_PERIODIC_INTERVAL;
    big_param.pres_delay_us                    = p_big_param->pres_delay_us;

    big_param.encrypted                        = p_big_param->encrypted;
    if (big_param.encrypted)
    {
        memcpy(big_param.bcast_code.bcast_code, p_big_param->bcast_code, APP_GAP_KEY_LEN);
    }
    else
    {
        memset(big_param.bcast_code.bcast_code, 0, APP_GAP_KEY_LEN);
    }

    big_param.adv_data_len                      = p_big_param->adv_data_len;
    big_param.adv_data                          = p_big_param->adv_data;
    big_param.per_adv_data_len                  = 0;
    big_param.per_adv_data                      = NULL;

    memcpy(aob_bis_src_env.bcast_id, big_param.bcast_id.id, sizeof(aob_bis_src_env.bcast_id));
    memcpy(aob_bis_src_env.bcast_code, big_param.bcast_code.bcast_code, sizeof(big_param.bcast_code.bcast_code));

    app_bap_bc_src_set_big_params(big_idx, &big_param);
}

void aob_bis_src_set_subgrp_param(uint8_t big_idx, aob_bis_src_subgrp_param_t *p_subgrp_param)
{
    app_bap_bc_src_subgrp_info_t subgrp_param = {0};
    app_gaf_bap_cfg_t                cfg_param = {0};
    app_gaf_bap_cfg_metadata_t* metadata_data = NULL;

    metadata_data = (app_gaf_bap_cfg_metadata_t *)bes_bt_buf_malloc(sizeof(app_gaf_bap_cfg_metadata_t) + p_subgrp_param->add_metadata_len);
    /// set ltv cfg param
    cfg_param.param.location_bf       = p_subgrp_param->location_bf;
    cfg_param.param.frame_octet       = p_subgrp_param->frame_octet;
    cfg_param.param.sampling_freq     = p_subgrp_param->sampling_freq;
    cfg_param.param.frame_dur         = p_subgrp_param->frame_dur;
    cfg_param.param.frames_sdu        = p_subgrp_param->frames_sdu;
    cfg_param.add_cfg.len             = 0;

    /// set ltv metadata param
    metadata_data->param.context_bf    = p_subgrp_param->context_bf;
    metadata_data->add_metadata.len    = p_subgrp_param->add_metadata_len;
    memcpy(metadata_data->add_metadata.data, p_subgrp_param->add_metadata_data, p_subgrp_param->add_metadata_len);

    /// set subdroup param
    subgrp_param.sgrp_lid             = p_subgrp_param->sgrp_lid;
    subgrp_param.codec_id.codec_id[0] = APP_GAF_CODEC_TYPE_LC3;
    subgrp_param.cfg_len              = sizeof(app_gaf_bap_cfg_t);
    subgrp_param.p_cfg                = &cfg_param;
    subgrp_param.metadata_len         = sizeof(app_gaf_bap_cfg_metadata_t) + p_subgrp_param->add_metadata_len;
    subgrp_param.p_metadata           = metadata_data;
    app_bap_bc_src_set_subgrp_params(big_idx, &subgrp_param);
    bes_bt_buf_free(metadata_data);
}

void aob_bis_src_set_stream_param(uint8_t big_idx, aob_bis_src_stream_param_t *p_stream_param)
{
    app_bap_bc_src_stream_info_t stream_param = {0};
    app_gaf_bap_cfg_t                cfg_param = {0};

    cfg_param.param.location_bf   = p_stream_param->location_bf;
    cfg_param.param.frame_octet   = p_stream_param->frame_octet;
    cfg_param.param.sampling_freq = p_stream_param->sampling_freq;
    cfg_param.param.frame_dur     = p_stream_param->frame_dur;
    cfg_param.param.frames_sdu    = p_stream_param->frames_sdu;
    cfg_param.add_cfg.len         = 0;

    /// set stream param
    stream_param.stream_lid          = p_stream_param->stream_lid;
    stream_param.sgrp_lid            = p_stream_param->sgrp_lid;
    stream_param.dp_cfg.dp_id        = APP_BAP_DFT_BC_SRC_DP_ID;
    stream_param.dp_cfg.ctl_delay_us = APP_BAP_DFT_BC_SRC_CTL_DELAY_US;
    stream_param.cfg_len             = sizeof(app_gaf_bap_cfg_t);
    stream_param.p_cfg               = &cfg_param;
    app_bap_bc_src_set_stream_params(big_idx, &stream_param);
}

void aob_bis_src_write_bis_data(uint8_t big_idx, uint8_t stream_lid, uint8_t *data, uint16_t data_len)
{
    app_bap_bc_src_grp_info_t *big_info = app_bap_bc_src_get_big_info(big_idx);

    if (big_info->big_state < APP_GAF_BAP_BC_SRC_STATE_STREAMING)
    {
        LOG_E("%s: Data cannot be sent in the current state, state=%d", __func__, big_info->big_state);
        return;
    }
    // TODO: write data to bis send queue
}

uint32_t aob_bis_src_get_anchor_time(uint8_t big_idx, uint8_t stream_idx)
{
    app_bap_bc_src_grp_info_t *grp = app_bap_bc_src_get_big_info_by_big_idx(big_idx);
    app_bap_bc_src_stream_info_t  *stream_info = NULL;

    if (!grp)
    {
        return 0;
    }

    for (int i=0; i < grp->big_param.nb_streams; ++i)
    {
        if (grp->stream_info[i].stream_lid == stream_idx)
        {
            stream_info = &grp->stream_info[i];
        }
    }

    if (!stream_info)
    {
        return 0;
    }

    return btdrv_reg_op_big_anchor_timestamp(stream_info->bis_hdl);
}

void aob_bis_src_start(uint8_t big_idx, aob_bis_src_started_info_t *start_info)
{
    app_bap_bc_src_grp_info_t *big_info = app_bap_bc_src_get_big_info(big_idx);

    if (big_info->big_state == APP_GAF_BAP_BC_SRC_STATE_IDLE)
    {
        aob_bis_src_env.bis_src_external_info = start_info;
        app_bap_bc_src_start(big_idx);
    }
    else
    {
        LOG_W("%s:bis src started", __func__);
    }
}

void aob_bis_src_stop(uint8_t big_idx)
{
    app_bap_bc_src_grp_info_t *big_info = app_bap_bc_src_get_big_info(big_idx);

    if (big_info->big_state == APP_GAF_BAP_BC_SRC_STATE_STREAMING)
    {
        app_bap_bc_src_stop_streaming(big_idx, aob_bis_src_env.bis_stream_bf);
    }
    else if (big_info->big_state == APP_GAF_BAP_BC_SRC_STATE_CONFIGURED)
    {
        app_bap_bc_src_disable(big_idx);
    }
}

void aob_bis_src_set_id_key(uint8_t big_idx, uint8_t *bcast_id, uint8_t *bcast_code)
{
    if (bcast_id)
    {
        memcpy(aob_bis_src_env.bcast_id, bcast_id, sizeof(aob_bis_src_env.bcast_id));
        app_bap_bc_src_set_bcast_id(big_idx, bcast_id, APP_GAP_BCAST_ID_LEN);
    }

    if (bcast_code)
    {
        memcpy(aob_bis_src_env.bcast_code, bcast_code, sizeof(app_gaf_bc_code_t));
        app_bap_bc_src_set_encrypt(big_idx, APP_BAP_DFT_BC_SRC_IS_ENCRYPTED, (app_gaf_bc_code_t *)bcast_code);
    }
}

void aob_bis_src_disable_encrypt(uint8_t big_idx)
{
    if (ble_audio_is_ux_mobile())
    {
        app_bap_bc_src_set_encrypt(big_idx, false, NULL);
    }
}

void aob_bis_src_set_stream_codec_cfg(uint8_t big_idx, uint8_t stream_lid, uint16_t frame_octet, uint8_t sampling_freq)
{
    app_bap_bc_src_set_stream_codec_cfg(big_idx, stream_lid, frame_octet, sampling_freq);
}

void aob_bis_src_update_stream_codec_cfg(uint8_t big_idx, uint8_t stream_lid, app_gaf_bap_cfg_t *p_cfg)
{
    app_bap_bc_src_update_stream_codec_cfg(big_idx, stream_lid, p_cfg);
}

void aob_bis_src_update_src_group_info(uint8_t big_idx, uint8_t nb_subgrp, uint8_t nb_stream, uint32_t sdu_intvl, uint16_t max_sdu_size)
{
    app_bap_bc_src_update_grp_info(big_idx, nb_subgrp, nb_stream, sdu_intvl, max_sdu_size);
}

uint16_t aob_bis_src_get_bis_hdl_by_big_idx(uint8_t big_idx)
{
    app_bap_bc_src_grp_info_t *p_grp = app_bap_bc_src_get_big_info_by_big_idx(big_idx);

    return p_grp->stream_info->bis_hdl;
}

const AOB_CODEC_ID_T *aob_bis_src_get_codec_id_by_big_idx(uint8_t big_idx, uint8_t subgrp_idx)
{
    app_bap_bc_src_grp_info_t *p_grp = app_bap_bc_src_get_big_info_by_big_idx(big_idx);

    return (AOB_CODEC_ID_T *)&p_grp->subgrp_info[subgrp_idx].codec_id;
}

const AOB_BAP_CFG_T *aob_bis_src_get_codec_cfg_by_big_idx(uint8_t big_idx)
{
    app_bap_bc_src_stream_info_t *p_stream = app_bap_bc_src_get_bis_stream_info(big_idx, 0);

    return (AOB_BAP_CFG_T *)p_stream->p_cfg;
}

uint32_t aob_bis_src_get_iso_interval_ms_by_big_idx(uint8_t big_idx)
{
    uint8_t iso_interval_frames = 0;

    app_bap_bc_src_grp_info_t *p_grp = app_bap_bc_src_get_big_info_by_big_idx(big_idx);

    iso_interval_frames = p_grp->grp_cmp.iso_interval_frames;

    return (iso_interval_frames * 10 / 8) * 1000;
}

void aob_bis_src_enable_pa(uint8_t big_idx)
{
    app_bap_bc_src_grp_info_t *p_grp = app_bap_bc_src_get_big_info_by_big_idx(big_idx);
    app_bap_bc_src_enable_pa(p_grp);
}

void aob_bis_src_disable_pa(uint8_t grp_lid)
{
    app_bap_bc_src_disable_pa(grp_lid);
}

void aob_bis_src_enable(uint8_t big_idx)
{
    app_bap_bc_src_grp_info_t *p_grp = app_bap_bc_src_get_big_info_by_big_idx(big_idx);
    app_bap_bc_src_enable(p_grp);
}

void aob_bis_src_disable(uint8_t grp_lid)
{
    app_bap_bc_src_disable(grp_lid);
}

void aob_bis_src_add_group_req(uint8_t big_idx)
{
    app_bap_bc_src_start(big_idx);
}

void aob_bis_src_start_streaming(uint8_t big_idx)
{
    app_bap_bc_src_start_streaming(big_idx, aob_bis_src_env.bis_stream_bf);
}

void aob_bis_src_update_metadata(uint8_t grp_lid, uint8_t sgrp_lid, app_gaf_bap_cfg_metadata_t *metadata)
{
    app_bap_bc_src_update_metadata_req(grp_lid, sgrp_lid, metadata);
}

void aob_bis_src_stop_streaming(uint8_t big_idx)
{
    app_bap_bc_src_stop_streaming(big_idx, aob_bis_src_env.bis_stream_bf);
}

static void aob_bis_src_enabled_cb(app_bap_bc_src_grp_info_t *p_grp)
{
    //big state changed APP_GAF_BAP_BC_SRC_STATE_CONFIGURED -> APP_GAF_BAP_BC_SRC_STATE_STREAMING
    LOG_D("%s ", __func__);
    uint32_t stream_lid_bf = 0;
    for (uint8_t stream_lid = 0; stream_lid < p_grp->big_param.nb_streams; stream_lid++)
    {
        stream_lid_bf |= (1 << stream_lid);
    }//the input should be stream_lid_bf not stream_lid
    app_bap_bc_src_start_streaming(app_bap_bc_src_find_big_idx(p_grp->grp_lid), stream_lid_bf);
}

static void aob_bis_src_disabled_cb(uint8_t grp_lid)
{
    //big state changed APP_GAF_BAP_BC_SRC_STATE_STREAMING -> APP_GAF_BAP_BC_SRC_STATE_CONFIGURED
    LOG_D("%s ", __func__);
    aob_bis_src_disable_pa(grp_lid);
}

static void aob_bis_src_pa_enabled_cb(app_bap_bc_src_grp_info_t *p_grp)
{
    //big state changed APP_GAF_BAP_BC_SRC_STATE_IDLE -> APP_GAF_BAP_BC_SRC_STATE_CONFIGURED
    LOG_D("%s ", __func__);
    app_bap_bc_src_enable(p_grp);
}

static void aob_bis_src_pa_disabled_cb(uint8_t grp_lid)
{
    //big state changed APP_GAF_BAP_BC_SRC_STATE_CONFIGURED -> APP_GAF_BAP_BC_SRC_STATE_IDLE
    LOG_D("%s ", __func__);
    app_bap_bc_src_remove_group_cmd(grp_lid);
}

static void aob_bis_src_stream_started_cb(uint8_t stream_lid, uint16_t bis_hdl)
{
    uint8_t status = BT_STS_SUCCESS;
    app_bap_bc_src_grp_info_t *app_p_grp = app_bap_bc_src_get_big_info_by_bis_hdl(bis_hdl);
    app_bap_bc_src_stream_info_t *stream_info = &app_p_grp->stream_info[stream_lid];

    if (NULL == app_p_grp)
    {
        LOG_D("Bis src can not start audio stream, find grp info err");
        return;
    }

    status = app_bap_bc_src_tx_stream_start(bis_hdl);
    if (status != BT_STS_SUCCESS)
    {
        LOG_W("aob_bap bc src start tx stream failed status 0x%02x", status);
    }
    else
    {
        app_bap_bc_src_grp_info_t *grp_info = NULL;
        grp_info = app_bap_bc_src_get_big_info(app_p_grp->grp_lid);

        if (aob_bis_src_env.bis_src_external_info)
        {
            aob_bis_src_started_param_t start_param;
            gaf_bis_external_stream_param_t ext_stream_param;

            ext_stream_param.stream_idx        = stream_lid;
            ext_stream_param.bis_hdl           = bis_hdl;
            ext_stream_param.data_size         = stream_info->p_cfg->param.frame_octet * stream_info->p_cfg->param.frames_sdu;
            ext_stream_param.send_interval_us  = grp_info->grp_cmp.iso_interval_frames * 10 / 8 * 1000;
            ext_stream_param.big_bn            = grp_info->grp_cmp.bn;
            ext_stream_param.big_sync_delay_us = grp_info->grp_cmp.sync_delay_us;
            ext_stream_param.read_data_cb      = (read_data_callback_t)aob_bis_src_env.bis_src_external_info->bis_stream_read_data_cb;
            ext_stream_param.data_buf_free_cb  = (data_buf_free_callback_t)aob_bis_src_env.bis_src_external_info->bis_stream_get_buf_free;
            gaf_bis_external_stream_start(&ext_stream_param);

            start_param.big_trans_latency = grp_info->grp_cmp.tlatency_us;
            aob_bis_src_env.bis_src_external_info->bis_stream_start_ind(stream_lid, &start_param);

        }
        else
        {

#ifdef USB_BIS_AUDIO_STREAM
            gaf_usb_bis_src_audio_stream_start_handler(app_p_grp->grp_lid);
#else
            gaf_bis_src_param_t src_para;

            src_para.grp_lid           = app_p_grp->grp_lid;
            src_para.conhdl            = bis_hdl;
            src_para.big_sync_delay    = grp_info->grp_cmp.sync_delay_us;
            src_para.big_trans_latency = grp_info->grp_cmp.tlatency_us;
            gaf_bis_src_audio_stream_start_handler(&src_para);
#endif
        }

        aob_bis_src_env.bis_stream_bf |= 1UL << stream_lid;
        LOG_D("Bis src stream was started_lid=%d", stream_lid);
    }
}

static void aob_bis_src_stream_stoped_cb(uint8_t stream_lid, uint16_t bis_hdl)
{
    app_bap_bc_src_grp_info_t *app_p_grp = app_bap_bc_src_get_big_info_by_bis_hdl(bis_hdl);

    if (NULL == app_p_grp)
    {
        LOG_D("Bis src can not stop audio stream, find grp info err");
        return;
    }
    app_bap_bc_src_tx_stream_stop(bis_hdl);
    if (aob_bis_src_env.bis_src_external_info)
    {
        gaf_bis_external_stream_stop(stream_lid);
        aob_bis_src_env.bis_src_external_info->bis_stream_stop_ind(stream_lid);
    }
    else
    {
#ifdef USB_BIS_AUDIO_STREAM
        gaf_usb_bis_src_audio_stream_stop_handler(app_p_grp->grp_lid);
#else
        gaf_bis_src_audio_stream_stop_handler(app_p_grp->grp_lid);
#endif
    }

    app_p_grp->stream_info->bis_hdl = GATT_INVALID_HDL;
    LOG_D("Bis src stream was stoped");

    aob_bis_src_env.bis_stream_bf &= ~(1UL << stream_lid);
    if (!aob_bis_src_env.bis_stream_bf)
    {
        aob_bis_src_disable(app_p_grp->grp_lid);
    }
}

static src_event_handler_t src_event_cb =
{
    .bis_src_enabled_ind       = aob_bis_src_enabled_cb,
    .bis_src_disabled_ind      = aob_bis_src_disabled_cb,
    .bis_src_pa_enabled_ind    = aob_bis_src_pa_enabled_cb,
    .bis_src_pa_disabled_ind   = aob_bis_src_pa_disabled_cb,
    .bis_src_stream_started_cb = aob_bis_src_stream_started_cb,
    .bis_src_stream_stoped_cb  = aob_bis_src_stream_stoped_cb,
};

static bool aob_bis_src_cfg_info_cb_func(app_bap_bc_src_cfg_info_t *info)
{
    TRACE(1, "aob src cfg init info");
    info->nb_streams = APP_BAP_DFT_BC_SRC_NB_STREAMS;
    info->nb_subgroups = APP_BAP_DFT_BC_SRC_NB_SUBGRPS;
    info->sdu_intv_us = APP_BAP_DFT_BC_SRC_SDU_INTV_US;
    info->max_sdu = APP_BAP_DFT_BC_SRC_MAX_SDU_SIZE;
    info->max_tlatency = APP_BAP_DFT_BC_SRC_MAX_TRANS_LATENCY_MS;
    info->adv_intv_min_slot = APP_BAP_DFT_BC_SRC_ADV_INTERVAL;
    info->adv_intv_max_slot = APP_BAP_DFT_BC_SRC_ADV_INTERVAL;
    info->adv_intv_min_frame = APP_BAP_DFT_BC_SRC_PERIODIC_INTERVAL;
    info->adv_intv_max_frame = APP_BAP_DFT_BC_SRC_PERIODIC_INTERVAL;
    info->encrypted = APP_BAP_DFT_BC_SRC_IS_ENCRYPTED;
    app_bap_add_data_set(info->bcast_id.id, APP_GAF_BAP_BC_BROADCAST_ID_LEN);//Data that can be used by scanning devices to help find broadcast Audio Streams.

    if (info->encrypted)
    {
        app_bap_add_data_set(info->bcast_code.bcast_code, APP_GAP_KEY_LEN);//BIS source bcast_code set,bcast_code will be filled by customer,set same value to test encrypted BIS
    }

    return true;
}
void aob_bis_src_api_init(void)
{
    aob_mgr_gaf_mobile_src_evt_handler_register(&src_event_cb);
    app_bap_bc_src_register_cfg_info_cb(aob_bis_src_cfg_info_cb_func);
}
#endif

#ifdef APP_BLE_BIS_ASSIST_ENABLE
/*BAP Broadcast Assist APIs*/
void aob_bis_assist_set_src_id_key(uint8_t *src_id, uint8_t *src_key)
{
    if (src_id)
    {
        memcpy(aob_bis_assist_env.bcast_id, src_id, sizeof(aob_bis_assist_env.bcast_id));
        aob_bis_scan_set_src_info(src_id, NULL);
    }

    if (src_key)
    {
        memcpy(aob_bis_assist_env.bcast_code, src_key, sizeof(aob_bis_assist_env.bcast_code));
    }
}

void aob_bis_assist_scan_bc_src(uint8_t con_lid)
{
    if (false == app_bap_bc_assist_is_deleg_scan())
    {
        app_bap_bc_scan_start(APP_GAF_BAP_BC_ASSIST_SCAN);
    }
    app_bap_bc_set_assist_is_deleg_scan(con_lid, true);
}

void aob_bis_assist_start_scan(uint16_t timeout_s)
{
    app_bap_bc_assist_start_scan(timeout_s);
}

void aob_bis_assist_stop_scan(void)
{
    app_bap_bc_assist_stop_scan();
}

void aob_bis_assist_discovery(uint8_t con_lid)
{
    app_bap_bc_assist_start(con_lid);
}

void aob_bis_assist_add_source(uint8_t con_lid, const app_gaf_bap_adv_id_t *p_adv_id, const uint8_t *p_bcast_id,
                               uint8_t pa_sync, uint8_t nb_subgrp, uint32_t bis_sync_bf)
{
    bool is_deleg_scan_prev = true;

    if (app_bap_bc_assist_is_deleg_scan() == false)
    {
        is_deleg_scan_prev = false;
        app_bap_bc_set_assist_is_deleg_scan(con_lid, true);
    }

    app_gaf_bap_bc_assist_add_src_t p_src_add;
    app_gaf_extend_adv_report_t adv_report = {*p_adv_id, 0, {0}};
    memcpy(&p_src_add.adv_report, &adv_report, sizeof(app_gaf_extend_adv_report_t));
    p_src_add.pa_sync = pa_sync;
    memcpy(&p_src_add.bcast_id.id, p_bcast_id, APP_GAF_BAP_BC_BROADCAST_ID_LEN);
    p_src_add.pa_intv_frames = APP_BAP_DFT_BC_SRC_PERIODIC_INTERVAL;
    p_src_add.nb_subgroups = nb_subgrp;
    p_src_add.bis_sync_bf = bis_sync_bf;
#ifdef APP_BLE_BIS_ASSIST_ENABLE
    app_bap_bc_assist_source_add(&p_src_add);
#endif

    if (is_deleg_scan_prev == false)
    {
        app_bap_bc_set_assist_is_deleg_scan(con_lid, false);
    }
}

void aob_bis_assist_remove_source(uint8_t con_lid, uint8_t src_lid)
{
    app_bap_bc_assist_source_remove(con_lid, src_lid);
}

static void aob_bis_assist_solicitation_cb(uint8_t addr_type, uint8_t *addr, uint16_t length, uint8_t *adv_data)
{
    LOG_D("aob_bap bc_assist_solicitation_ind addr_type = %d, len = %d", addr_type, length);
    DUMP8("%02x ", addr, GAP_BD_ADDR_LEN);
    DUMP8("%02x ", adv_data, length);
    BLE_ADV_DATA_T *particle;

    for (uint8_t offset = 0; offset < length;)
    {
        particle = (BLE_ADV_DATA_T *)(adv_data + offset);
        ble_bdaddr_t connect_addr;
        memcpy(connect_addr.addr, addr, APP_GAP_BD_ADDR_LEN);
        connect_addr.addr_type = addr_type;

        if (GAP_DT_COMPLETE_LOCAL_NAME == particle->type)
        {
            LOG_I("aob_bap bc_assist start connecting!");
            app_bap_bc_assist_stop_scan();
            app_ble_start_connect(&connect_addr, APP_GAPM_STATIC_ADDR);
            break;
        }
        else
        {
            offset += (ADV_PARTICLE_HEADER_LEN + particle->length);
        }
    }
}

static void aob_bis_assist_source_state_cb(uint16_t cmd_code, uint16_t status, uint8_t con_lid, uint8_t src_lid)
{
    switch (cmd_code)
    {
        case (APP_GAF_BAP_BC_ASSIST_START_SCAN):
        {
            LOG_D("aob_bap bc_assist start scan cmp");
        }
        break;

        case (APP_GAF_BAP_BC_ASSIST_STOP_SCAN):
        {
            LOG_D("aob_bap bc_assist stop scan cmp");
        }
        break;

        case (APP_GAF_BAP_BC_ASSIST_DISCOVER):
        {
            LOG_D("aob_bap bc_assist discovery cmp, con_lid = %d", con_lid);
            app_bap_bc_assist_set_cfg_cmd(con_lid, 0, 1);
        }
        break;

        case (APP_GAF_BAP_BC_ASSIST_GET_STATE):
        {
            LOG_D("aob_bap bc_assist get state cmp, con_lid = %d, src_lid = %d", con_lid, src_lid);
        }
        break;

        case (APP_GAF_BAP_BC_ASSIST_GET_CFG):
        {
            LOG_D("aob_bap bc_assist get cfg cmp, con_lid = %d, src_lid = %d", con_lid, src_lid);
            if (false == app_bap_bc_assist_is_deleg_scan())
            {
                app_bap_bc_scan_start(APP_GAF_BAP_BC_ASSIST_SCAN);
            }
            app_bap_bc_set_assist_is_deleg_scan(con_lid, true);
        }
        break;

        case (APP_GAF_BAP_BC_ASSIST_SET_CFG):
        {
            LOG_D("aob_bap bc_assist set cfg cmp, con_lid = %d, src_lid = %d", con_lid, src_lid);
            app_bap_bc_assist_get_cfg_cmd(con_lid, src_lid);
        }
        break;

        case (APP_GAF_BAP_BC_ASSIST_UPDATE_SCAN):
        {
            LOG_D("aob_bap bc_assist update scan cmp, con_lid = %d, src_lid = %d", con_lid, src_lid);
        }
        break;

        case (APP_GAF_BAP_BC_ASSIST_ADD_SOURCE):
        {
            LOG_D("aob_bap bc_assist source add cmp, con_lid = %d, src_lid = %d", con_lid, src_lid);
            app_bap_bc_scan_stop();
        }
        break;

        case (APP_GAF_BAP_BC_ASSIST_ADD_SOURCE_LOCAL):
        {
            LOG_D("aob_bap bc_assist source add local cmp, con_lid = %d, src_lid = %d", con_lid, src_lid);
        }
        break;

        case (APP_GAF_BAP_BC_ASSIST_REMOVE_SOURCE):
        {
            LOG_D("aob_bap bc_assist source remove cmp, con_lid = %d, src_lid = %d", con_lid, src_lid);
        }
        break;

        case (APP_GAF_BAP_BC_ASSIST_MODIFY_SOURCE):
        {
            LOG_D("aob_bap bc_assist source modify cmp, con_lid = %d, src_lid = %d", con_lid, src_lid);
        }
        break;

        case (APP_GAF_BAP_BC_ASSIST_MODIFY_SOURCE_LOCAL):
        {
            LOG_D("aob_bap bc_assist source modify local cmp, con_lid = %d, src_lid = %d", con_lid, src_lid);
        }
        break;

        default:
            break;
    }
}

static void aob_bis_assist_bond_data_cb(uint8_t con_lid, uint8_t nb_rx_state, uint16_t shdl, uint16_t ehdl)
{
    LOG_D("aob_bap bass found con_lid = %d, nb_rx_state = %d, shdl = %04x, ehdl = %04x", con_lid, nb_rx_state, shdl, ehdl);
}

static void aob_bis_assist_bcast_cdoe_ri_cb(uint8_t con_lid, uint8_t src_lid)
{
    LOG_D("aob_bap bcast code ri con_lid = %d, src_lid = %d", con_lid, src_lid);

    app_bap_bc_assist_send_bcast_code(con_lid, src_lid, aob_bis_assist_env.bcast_code);
}

static assist_event_handler_t assist_event_cb =
{
    .assist_solicitation_cb    = aob_bis_assist_solicitation_cb,
    .assist_source_state_cb    = aob_bis_assist_source_state_cb,
    .assist_bond_data_cb       = aob_bis_assist_bond_data_cb,
    .assist_bcast_code_ri_cb   = aob_bis_assist_bcast_cdoe_ri_cb,
};

void aob_bis_assist_api_init(void)
{
    aob_mgr_gaf_mobile_assist_evt_handler_register(&assist_event_cb);
}

#else

/*BAP Broadcast Assist APIs*/
void aob_bis_assist_set_src_id_key(uint8_t *src_id, uint8_t *src_key)
{
    LOG_I("%s is not supported", __func__);
}

void aob_bis_assist_scan_bc_src(uint8_t con_lid)
{
    LOG_I("%s is not supported", __func__);
}

void aob_bis_assist_start_scan(uint16_t timeout_s)
{
}

void aob_bis_assist_stop_scan(void)
{
    LOG_I("%s is not supported", __func__);
}

void aob_bis_assist_discovery(uint8_t con_lid)
{
    LOG_I("%s is not supported", __func__);
}

void aob_bis_assist_add_source(uint8_t con_lid, const app_gaf_bap_adv_id_t *p_adv_id, const uint8_t *p_bcast_id,
                               uint8_t pa_sync, uint8_t nb_subgrp, uint32_t bis_sync_bf)
{
    LOG_I("%s is not supported", __func__);
}

void aob_bis_assist_remove_source(uint8_t con_lid, uint8_t src_lid)
{
    LOG_I("%s is not supported", __func__);
}
#endif

#ifdef APP_BLE_BIS_DELEG_ENABLE
/*BAP Broadcast Deleg APIs*/
void aob_bis_deleg_start_solicite(uint16_t timeout_s, uint32_t context_bf)
{
    app_bap_bc_deleg_start_solicite(timeout_s, context_bf);
}

void aob_bis_deleg_stop_solicite(void)
{
    app_bap_bc_deleg_stop_solicite();
}

void aob_bis_deleg_cfm_bis_sync_pref(uint8_t src_lid, uint32_t bis_sync_pref)
{
    app_bap_bc_deleg_pref_bis_sync_cfm(src_lid, bis_sync_pref);
}

void aob_bis_deleg_add_source(app_gaf_bap_adv_id_t *addr, const app_gaf_bap_bcast_id_t *p_bcast_id,
                              uint8_t nb_subgroups, uint8_t *add_metadata, uint8_t add_metadata_len, uint16_t streaming_context_bf,
                              uint32_t bis_sync_bf_deprecated)
{
    app_bap_bc_deleg_source_add(addr, p_bcast_id, nb_subgroups,
                                add_metadata, add_metadata_len, streaming_context_bf, bis_sync_bf_deprecated);
}

void aob_bis_deleg_pa_sync_ri(uint8_t pa_lid, bool accept)
{
    app_bap_bc_scan_dele_pa_sync_ri(pa_lid, accept);
}

void aob_bis_deleg_pa_terminate_ri(uint8_t pa_lid, bool accept)
{
    app_bap_bc_scan_dele_pa_terminate_ri(pa_lid, accept);
}

static void aob_bis_deleg_solicite_started_cb(void)
{
    LOG_D("aob_bap bc_deleg start solicite cmp");

    if ((NULL != p_ble_core_evt_cb) && (NULL != p_ble_core_evt_cb->ble_bis_deleg_scolite_start_cb))
    {
        p_ble_core_evt_cb->ble_bis_deleg_scolite_start_cb();
    }
}

static void aob_bis_deleg_solicite_stoped_cb(void)
{
    LOG_D("aob_bap bc_deleg stop solicite cmp");

    if ((NULL != p_ble_core_evt_cb) && (NULL != p_ble_core_evt_cb->ble_bis_deleg_scolite_stoped_cb))
    {
        p_ble_core_evt_cb->ble_bis_deleg_scolite_stoped_cb();
    }
}

static void aob_bis_deleg_source_add_ri_cb(uint8_t src_lid, uint8_t *bcast_id, uint8_t con_lid, uint8_t pa_sync_req)
{
    LOG_D("aob_bap bc_deleg source add ri");
    ble_audio_earphone_info_set_bis_src_lid(src_lid);
    /// Store bcast id for later sync with PA according to EA's bcast id if no PAST
    memcpy(aob_bis_sink_env.bcast_id, bcast_id, sizeof(aob_bis_sink_env.bcast_id));

    if ((NULL != p_ble_core_evt_cb) && (NULL != p_ble_core_evt_cb->ble_bis_deleg_source_add_ri_cb))
    {
        p_ble_core_evt_cb->ble_bis_deleg_source_add_ri_cb(src_lid, con_lid, pa_sync_req);
    }
}

static void aob_bis_deleg_source_remove_ri_cb(uint8_t src_lid, uint8_t con_lid)
{
    LOG_D("aob_bap bc_deleg source remove ri");
    ble_audio_earphone_info_set_bis_src_lid(0);
    if ((NULL != p_ble_core_evt_cb) && (NULL != p_ble_core_evt_cb->ble_bis_deleg_source_rm_ri_cb))
    {
        p_ble_core_evt_cb->ble_bis_deleg_source_rm_ri_cb(src_lid, con_lid);
    }
}

static void aob_bis_deleg_source_modify_ri_cb(uint8_t src_lid, uint8_t con_lid, uint8_t pa_sync_req)
{
    LOG_D("aob_bap bc_deleg source modify ri");

    if ((NULL != p_ble_core_evt_cb) && (NULL != p_ble_core_evt_cb->ble_bis_deleg_source_upd_ri_cb))
    {
        p_ble_core_evt_cb->ble_bis_deleg_source_upd_ri_cb(src_lid, con_lid, pa_sync_req);
    }
}

static void aob_bis_deleg_pref_bis_sync_ri_cb(uint8_t src_lid, uint8_t con_lid)
{
    LOG_D("aob_bap bc_deleg prefer bis sync ri");

    AOB_BIS_GROUP_INFO_T *aob_bis_group_info = ble_audio_earphone_info_get_bis_group_info();

    if (NULL == aob_bis_group_info)
    {
        LOG_W("Bis earphone info get error");
        return;
    }

    app_gaf_bap_bc_scan_pa_report_info_t *pa_info = app_bap_bc_scan_get_exist_pa_info_by_pa_lid(aob_bis_group_info->pa_lid);

    if (pa_info == NULL)
    {
        LOG_W("Bis pa info get error");
        return;
    }

    if (pa_info->group_info.nb_streams == 0)
    {
        LOG_W("Bis stream is empty");
        return;
    }

    uint8_t idx = 0;
    uint32_t bis_sync_pref = 0;

    /// TODO: here to select bis sync pref or later choose bis sync hdl
    for (idx = 0; idx < pa_info->group_info.nb_streams; idx++)
    {
        bis_sync_pref |= (1 << (pa_info->stream_info[idx].stream_pos - 1));
    }

    LOG_I("%s bis sync pref is 0x%x", __func__, bis_sync_pref);

    aob_bis_deleg_cfm_bis_sync_pref(src_lid, bis_sync_pref);
}

static deleg_event_handler_t deleg_event_cb =
{
    .deleg_solicite_started_cb = aob_bis_deleg_solicite_started_cb,
    .deleg_solicite_stoped_cb  = aob_bis_deleg_solicite_stoped_cb,
    .deleg_remote_scan_started = NULL,
    .deleg_remote_scan_stoped  = NULL,
    .deleg_source_add_ri_cb    = aob_bis_deleg_source_add_ri_cb,
    .deleg_source_remove_ri_cb = aob_bis_deleg_source_remove_ri_cb,
    .deleg_source_update_ri_cb = aob_bis_deleg_source_modify_ri_cb,
    .deleg_pref_bis_sync_ri_cb = aob_bis_deleg_pref_bis_sync_ri_cb,
};

void aob_bis_deleg_api_init(void)
{
    p_ble_core_evt_cb = ble_audio_get_evt_cb();
    aob_mgr_gaf_deleg_evt_handler_register(&deleg_event_cb);
}
#endif

#ifdef APP_BLE_BIS_SINK_ENABLE
void aob_bis_sink_start(aob_bis_sink_start_param_t *param)
{
    uint8_t scan_state = app_bap_bc_scan_get_scan_state();

    LOG_I("Bis scan state=%d!", scan_state);

    if (scan_state == APP_GAF_BAP_BC_SCAN_STATE_IDLE)
    {
        aob_bis_scan_env.bis_scan_state_cb = param->event_callback.bis_sink_scan_state_cb;
        aob_bis_scan_env.bis_scan_metadata_cb = param->event_callback.bis_sink_metadata_callback;
        aob_bis_scan_env.bis_scan_select_source_cb = param->event_callback.bis_sink_select_source;
        aob_bis_sink_env.event_callback.bis_sink_started_callback = param->event_callback.bis_sink_started_callback;
        aob_bis_sink_env.event_callback.bis_sink_stoped_callback = param->event_callback.bis_sink_stoped_callback;
        aob_bis_sink_env.ble_bis_select_bf = param->ch_bf;
        memcpy(aob_bis_sink_env.bcast_id, param->bc_id, sizeof(aob_bis_sink_env.bcast_id));
        memcpy(aob_bis_sink_env.bcast_code, param->bc_code, sizeof(aob_bis_sink_env.bcast_code));
        aob_bis_scan_set_src_info(param->bc_id, NULL);
        app_bap_bc_scan_start(APP_GAF_BAP_BC_SINK_SCAN);
    }
    else
    {
        LOG_W("Bis scan enabled!");
    }
}

void aob_bis_sink_stop(void)
{
    uint8_t scan_state = app_bap_bc_scan_get_scan_state();
    uint8_t sync_state = app_bap_bc_scan_get_sync_state();

    app_bap_bc_sink_env_t *sink_info = app_bap_bc_sink_get_env();

    if (sink_info == NULL)
    {
        return;
    }

    LOG_I("Bis sink state=%d!", sink_info->sink_state);
    LOG_I("Bis scan state=%d, sync state=%d!", scan_state, sync_state);
    if (sink_info->sink_state == APP_GAF_BAP_BC_SINK_STATE_STREAMING
            && aob_bis_sink_env.bis_stream_bf)
    {
        for (int i = 0; i < 32; ++i)
        {
            if (aob_bis_sink_env.bis_stream_bf & (1UL << i))
            {
                app_bap_bc_sink_stop_streaming(aob_bis_sink_env.grp_lid, i);
                break;
            }
        }
    }
    else if (sink_info->sink_state == APP_GAF_BAP_BC_SINK_STATE_ENABLED)
    {
        app_bap_bc_sink_disable(aob_bis_sink_env.grp_lid);
    }
    else if (sync_state > APP_GAF_BAP_BC_SCAN_STATE_SCANNING)
    {
        app_bap_bc_scan_pa_terminate(aob_bis_scan_env.pa_lid);
    }
    else if (scan_state > APP_GAF_BAP_BC_SCAN_STATE_IDLE)
    {
        app_bap_bc_scan_stop();
    }

    aob_bis_sink_env.ble_bis_select_bf = 0;
}

void aob_bis_restart_deleg_bis_sink(void)
{
    app_bap_bc_sink_env_t *sink_info = app_bap_bc_sink_get_env();

    if (sink_info == NULL || sink_info->sink_state != APP_GAF_BAP_BC_SINK_STATE_IDLE)
    {
        LOG_E("Bis sink state:%d", sink_info != NULL ? sink_info->sink_state : 0xFF);
        return;
    }

    AOB_BIS_GROUP_INFO_T *aob_bis_group_info = ble_audio_earphone_info_get_bis_group_info();

    if (NULL == aob_bis_group_info)
    {
        LOG_E("Bis earphone info get error");
        return;
    }

    app_gaf_bap_bc_scan_pa_report_info_t *pa_info =
                    app_bap_bc_scan_get_exist_pa_info_by_pa_lid(aob_bis_group_info->pa_lid);

    if (pa_info == NULL)
    {
        LOG_E("PA info not found %d", aob_bis_group_info->pa_lid);
        return;
    }

    app_bap_bc_deleg_restart_src_big_sync(aob_bis_group_info->src_lid);
}

/*BAP Broadcast Sink APIs*/
void aob_bis_sink_enable(uint8_t pa_lid, uint8_t mse, uint8_t stream_pos_bf,
                         uint16_t timeout_10ms, uint8_t encrypted, uint8_t *bcast_code,
                         uint8_t *bcast_id)
{
    app_gaf_bap_bc_sink_enable_t p_sink_enable;
    p_sink_enable.pa_lid = pa_lid;
    p_sink_enable.mse = mse;
    p_sink_enable.stream_pos_bf = stream_pos_bf;
    p_sink_enable.timeout_10ms = timeout_10ms;
    p_sink_enable.encrypted = encrypted;
    memcpy(&p_sink_enable.bcast_id.id[0], &bcast_id[0], APP_GAF_BAP_BC_BROADCAST_ID_LEN);
    if (p_sink_enable.encrypted)
    {
        if (bcast_code)
        {
            memcpy(&p_sink_enable.bcast_code, bcast_code, sizeof(app_gaf_bc_code_t));
        }
    }

    app_bap_bc_sink_enable(&p_sink_enable);
}

void aob_bis_sink_disable(uint8_t grp_lid)
{
    app_bap_bc_sink_disable(grp_lid);
}

void aob_bis_sink_set_src_id_key(uint8_t *bcast_id, uint8_t *bcast_code)
{
    if (bcast_id)
    {
        memcpy(aob_bis_sink_env.bcast_id, bcast_id, sizeof(aob_bis_sink_env.bcast_id));
        aob_bis_scan_set_src_info(bcast_id, NULL);
    }

    if (bcast_code)
    {
        memcpy(aob_bis_sink_env.bcast_code, bcast_code, sizeof(aob_bis_sink_env.bcast_code));
    }
}

void aob_bis_sink_start_streaming(uint8_t grp_lid, uint32_t stream_pos_bf,
                                  uint8_t codec_type, AOB_BIS_MEDIA_INFO_T *media_info)
{
    AOB_BIS_GROUP_INFO_T *aob_bis_group_info = ble_audio_earphone_info_get_bis_group_info();
    app_gaf_bap_bc_scan_stream_info_t stream_info = {0};

    stream_info.sgrp_id = 0;
    stream_info.stream_pos = stream_pos_bf;
    stream_info.codec_id.codec_id[0] = APP_GAF_CODEC_TYPE_LC3;
    stream_info.param.location_bf = (APP_GAF_BAP_AUDIO_LOCATION_SIDE_LEFT | APP_GAF_BAP_AUDIO_LOCATION_SIDE_RIGHT);
    stream_info.param.frame_octet = media_info->frame_octet;
    stream_info.param.sampling_freq = media_info->sampling_freq;
    stream_info.param.frame_dur     = APP_GAF_BAP_FRAME_DURATION_10MS;
    stream_info.param.frames_sdu    = 1;
    stream_info.ltv_len         = 0;
    aob_bis_group_info->play_stream_num = 1;
    aob_bis_group_info->play_ch_num     = 1;
    aob_bis_group_info->play_stream_info[0].stream_lid = \
                                                         ble_audio_earphone_info_bis_stream_pos_2_stream_lid(stream_pos_bf);
    aob_bis_group_info->play_stream_info[0].select_ch_bf = 0x01;
    app_bap_bc_sink_streaming_config(grp_lid, stream_pos_bf, codec_type, NULL, &stream_info, \
                                     (app_gaf_bap_bc_sink_audio_streaming_t *)&aob_bis_group_info->play_stream_info[0].stream_info);
    app_bap_bc_sink_start_streaming((app_gaf_bap_bc_sink_audio_streaming_t *)&aob_bis_group_info->play_stream_info[0].stream_info);
}

void aob_bis_sink_stop_streaming(uint8_t grp_lid, uint8_t stream_pos)
{
    app_bap_bc_sink_stop_streaming(grp_lid, stream_pos);
}

void aob_bis_sink_set_player_channel(uint32_t channel_bf)
{
    aob_bis_sink_env.ble_bis_select_bf = channel_bf;
}

const app_gaf_bap_adv_id_t *aob_bis_sink_get_pa_addr_info(uint8_t pa_lid)
{
    app_gaf_bap_bc_scan_pa_report_info_t *pa_info = app_bap_bc_scan_get_exist_pa_info_by_pa_lid(pa_lid);

    if (pa_info != NULL &&
        pa_info->pa_sync_hdl != APP_GAF_INVALID_CON_HDL &&
        pa_info->pa_lid != APP_GAF_INVALID_LID)
    {
        return &pa_info->pa_addr;
    }

    return NULL;
}

uint16_t aob_bis_sink_get_pa_sync_hdl_by_pa_lid(uint8_t pa_lid)
{
    app_gaf_bap_bc_scan_pa_report_info_t *pa_info = app_bap_bc_scan_get_exist_pa_info_by_pa_lid(pa_lid);

    if (pa_info != NULL &&
        pa_info->pa_lid != APP_GAF_INVALID_LID)
    {
        return pa_info->pa_sync_hdl;
    }

    return APP_GAF_INVALID_CON_HDL;
}

static uint8_t aob_bis_get_bf_num(uint32_t data_bf)
{
    uint8_t cnt = 0;
    uint32_t stream_bf = data_bf;

    while (stream_bf)
    {
        cnt++;
        stream_bf &= (stream_bf - 1);
    }

    LOG_I("%s location_bf: 0x%x, cnt: %d", __func__, data_bf, cnt);
    return cnt;
}

static void aob_bis_sink_select_play_channel(uint8_t stream_id, uint32_t select_channel_bf,
                                             app_gaf_bap_bc_scan_stream_info_t *stream_info,
                                             AOB_BIS_GROUP_INFO_T *aob_bis_group_info)
{
    uint8_t ch_num = 0;
    uint32_t stream_location_bf = stream_info->param.location_bf;

    if (aob_bis_group_info->play_ch_num >= 2)
    {
        return;
    }

    if (select_channel_bf == 0)
    {
        // When no channel is selected, the channel of stream0 will be played by default,
        // with a maximum of two channels
        ch_num = aob_bis_get_bf_num(stream_location_bf);
        if (ch_num > 1)
        {
            aob_bis_group_info->play_ch_num = 2;
            aob_bis_group_info->play_stream_info[0].select_ch_bf = 3;
        }
        else
        {
            aob_bis_group_info->play_ch_num = 1;
            aob_bis_group_info->play_stream_info[0].select_ch_bf = 1;
        }
    }
    else
    {
        // Map the selected channel to the bit bit of the data stream
        for (int i = 0; i < 32; ++i)
        {
            if (stream_location_bf & (1UL << i))
            {
                if (select_channel_bf & (1UL << i))
                {
                    aob_bis_group_info->play_ch_num++;
                    aob_bis_group_info->play_stream_info[stream_id].select_ch_bf |= \
                                                                                    (1UL << i);
                    select_channel_bf &= ~(1UL << i);
                }
            }
            stream_location_bf &= ~(1UL << i);

            if ((!select_channel_bf) || (!stream_location_bf) || (aob_bis_group_info->play_ch_num >= 2))
            {
                break;
            }
        }
    }
}

static void aob_bis_sink_select_play_stream(uint8_t grp_lid, uint32_t stream_pos_bf, AOB_BIS_GROUP_INFO_T *aob_bis_group_info)
{
    /// Get streaming info from PA's BASE
    uint8_t stream_lid = 0;
    uint8_t ch_num;
    uint32_t select_ch_bf = aob_bis_sink_env.ble_bis_select_bf;
    app_gaf_bap_bc_scan_pa_report_info_t *pa_info = app_bap_bc_scan_get_exist_pa_info_by_pa_lid(aob_bis_group_info->pa_lid);
    app_gaf_bap_bc_scan_stream_info_t *stream_info_p[2] = {NULL};
    app_gaf_bap_bc_scan_subgrp_info_t *subgroup_info_p[2] = {NULL};

    LOG_I("select_ch_bf = %u, sink_stream_bf = 0x%x", select_ch_bf, stream_pos_bf);

    /// select player stream, max two stream
    if (select_ch_bf)
    {
        for (int num = 0; num < pa_info->group_info.nb_streams; ++num)
        {
            if (pa_info->stream_info[num].param.location_bf & select_ch_bf)
            {
                if (!stream_info_p[0])
                {
                    LOG_I("0-finded pa_info->stream_info[%d].param.location_bf = %u", num, pa_info->stream_info[num].param.location_bf);
                    LOG_I("0-finded stream pos = %d", pa_info->stream_info[num].stream_pos);
                    stream_info_p[0] = &pa_info->stream_info[num];
                }
                else if (!stream_info_p[1])
                {
                    LOG_I("1-finded pa_info->stream_info[%d].param.location_bf = %u", num, pa_info->stream_info[num].param.location_bf);
                    LOG_I("1-finded stream pos = %d", pa_info->stream_info[num].stream_pos);
                    stream_info_p[1] = &pa_info->stream_info[num];
                    break;
                }
            }
        }
    }

    /// if not find stream, use stream that sink to
    if (!stream_info_p[0])
    {
        /// select player stream, max two stream
        for (int num = 0; num < pa_info->group_info.nb_streams; ++num)
        {
            uint32_t stream_pos_bf_get = (1 << pa_info->stream_info[num].stream_pos);

            if (stream_pos_bf_get & stream_pos_bf)
            {
                if (!stream_info_p[0])
                {
                    LOG_I("0-finded pa_info->stream_info[%d].param.location_bf = %u", num, pa_info->stream_info[num].param.location_bf);
                    LOG_I("0-finded stream pos = %d", pa_info->stream_info[num].stream_pos);
                    stream_info_p[0] = &pa_info->stream_info[num];
                }
                else if (!stream_info_p[1])
                {
                    LOG_I("1-finded pa_info->stream_info[%d].param.location_bf = %u", num, pa_info->stream_info[num].param.location_bf);
                    LOG_I("1-finded stream pos = %d", pa_info->stream_info[num].stream_pos);
                    stream_info_p[1] = &pa_info->stream_info[num];
                    break;
                }
            }
        }
    }

    if (stream_info_p[0] == NULL)
    {
        LOG_E("Can not found correspond stream");
        return;
    }

    /// find stream in subgrep
    for (int num = 0; num < pa_info->group_info.nb_subgroups; ++num)
    {
        if (pa_info->subgroup_info[num].sgrp_id == stream_info_p[0]->sgrp_id)
        {
            LOG_I("0-finded pa_info->subgroup_info[%d].sgrp_id = %d", num, pa_info->subgroup_info[num].sgrp_id);
            subgroup_info_p[0] = &pa_info->subgroup_info[num];
        }
        if ((stream_info_p[1]) && pa_info->subgroup_info[num].sgrp_id == stream_info_p[1]->sgrp_id)
        {
            LOG_I("1-finded pa_info->subgroup_info[%d].sgrp_id = %d", num, pa_info->subgroup_info[num].sgrp_id);
            subgroup_info_p[1] = &pa_info->subgroup_info[num];
        }
    }

    /// set sink stream cfg
    if (stream_info_p[0])
    {
        app_bap_bc_sink_streaming_config(grp_lid, stream_info_p[0]->stream_pos, APP_GAF_CODEC_TYPE_LC3, \
                                         subgroup_info_p[0], stream_info_p[0],
                                         (app_gaf_bap_bc_sink_audio_streaming_t *)&aob_bis_group_info->play_stream_info[0].stream_info);

        if (stream_info_p[0]->param.location_bf & select_ch_bf)
        {
            aob_bis_sink_select_play_channel(0, aob_bis_sink_env.ble_bis_select_bf, stream_info_p[0], \
                                             aob_bis_group_info);
        }
        else
        {
            aob_bis_sink_select_play_channel(0, 0, stream_info_p[0], aob_bis_group_info);
        }
        aob_bis_group_info->play_stream_num = 1;
        stream_lid = ble_audio_earphone_info_bis_stream_pos_2_stream_lid(stream_info_p[0]->stream_pos);
        aob_bis_group_info->play_stream_info[0].stream_lid = stream_lid;
        aob_bis_group_info->play_stream_info[0].bis_hdl    = aob_bis_group_info->bis_hdl[stream_lid];
        ch_num = aob_bis_get_bf_num(stream_info_p[0]->param.location_bf);
        aob_bis_group_info->play_stream_info[0].blocks_size =
            aob_bis_group_info->play_stream_info[0].stream_info.cfg_param.param.frame_octet * ch_num;
    }

    if (stream_info_p[1])
    {
        app_bap_bc_sink_streaming_config(grp_lid, stream_info_p[1]->stream_pos, APP_GAF_CODEC_TYPE_LC3, \
                                         subgroup_info_p[1], stream_info_p[1],
                                         (app_gaf_bap_bc_sink_audio_streaming_t *)&aob_bis_group_info->play_stream_info[1].stream_info);
        aob_bis_sink_select_play_channel(1, aob_bis_sink_env.ble_bis_select_bf, stream_info_p[1], \
                                         aob_bis_group_info);
        aob_bis_group_info->play_stream_num++;
        stream_lid = ble_audio_earphone_info_bis_stream_pos_2_stream_lid(stream_info_p[1]->stream_pos);
        aob_bis_group_info->play_stream_info[1].stream_lid = stream_lid;
        aob_bis_group_info->play_stream_info[1].bis_hdl    = aob_bis_group_info->bis_hdl[stream_lid];
        ch_num = aob_bis_get_bf_num(stream_info_p[1]->param.location_bf);
        aob_bis_group_info->play_stream_info[1].blocks_size =
            aob_bis_group_info->play_stream_info[1].stream_info.cfg_param.param.frame_octet * ch_num;
    }
}

static void aob_bis_sink_state_cb(uint8_t grp_lid, uint8_t state, uint32_t stream_pos_bf)
{
    LOG_D("Bis sink state is %d", state);
    uint16_t err_code = BT_LL_ERR_NO_ERROR;
    AOB_BIS_GROUP_INFO_T *aob_bis_group_info = ble_audio_earphone_info_get_bis_group_info();

    if (NULL == aob_bis_group_info)
    {
        LOG_I("Bis earphone info get error");
        return;
    }

    switch (state)
    {
        case APP_GAF_BAP_BC_SINK_ESTABLISHED:
        {
            aob_bis_sink_select_play_stream(grp_lid, stream_pos_bf, aob_bis_group_info);
            LOG_I("Play_stream_num = %d, stream_pos_bf = %u", aob_bis_group_info->play_stream_num, stream_pos_bf);
            if (aob_bis_group_info->play_stream_num == 1)
            {
                app_bap_bc_sink_start_streaming((app_gaf_bap_bc_sink_audio_streaming_t *)&aob_bis_group_info->play_stream_info[0].stream_info);
                app_ble_audio_sink_streaming_handle_event(AOB_COMMON_MOBILE_CONNECTION_MAX - 1, aob_bis_group_info->play_stream_info[0].stream_lid, APP_GAF_DIRECTION_SINK, BLE_AUDIO_BIS_STREAM_START_IND);
            }
            if (aob_bis_group_info->play_stream_num >= 2)
            {
                app_bap_bc_sink_start_streaming((app_gaf_bap_bc_sink_audio_streaming_t *)&aob_bis_group_info->play_stream_info[0].stream_info);
                app_bap_bc_sink_start_streaming((app_gaf_bap_bc_sink_audio_streaming_t *)&aob_bis_group_info->play_stream_info[1].stream_info);
                app_ble_audio_sink_streaming_handle_event(AOB_COMMON_MOBILE_CONNECTION_MAX - 1, aob_bis_group_info->play_stream_info[0].stream_lid, APP_GAF_DIRECTION_SINK, BLE_AUDIO_BIS_STREAM_START_IND);
                app_ble_audio_sink_streaming_handle_event(AOB_COMMON_MOBILE_CONNECTION_MAX - 1, aob_bis_group_info->play_stream_info[1].stream_lid, APP_GAF_DIRECTION_SINK, BLE_AUDIO_BIS_STREAM_START_IND);

            }
            if (aob_bis_sink_env.event_callback.bis_sink_started_callback)
            {
                aob_bis_sink_env.event_callback.bis_sink_started_callback(grp_lid);
            }
        }
        break;
        case APP_GAF_BAP_BC_SINK_LOST:
        case APP_GAF_BAP_BC_SINK_PEER_TERMINATE:
        case APP_GAF_BAP_BC_SINK_UPPER_TERMINATE:
        case APP_GAF_BAP_BC_SINK_MIC_FAILURE:
        case APP_GAF_BAP_BC_SINK_FAILED:
        case APP_GAF_BAP_BC_SINK_CANCELLED:
        {
            app_bap_bc_scan_env_t *aob_bap_scan_env = app_bap_bc_scan_get_scan_env();

            if (APP_GAF_BAP_BC_DELEG_TRIGGER != aob_bap_scan_env->scan_trigger_method)
            {
                aob_bis_sink_scan_pa_terminate(aob_bis_group_info->pa_lid);
            }

            if (aob_bis_group_info->play_stream_num == 0)
            {
                break;
            }

            app_bap_bc_sink_rx_stream_stop(aob_bis_group_info->play_stream_info[0].bis_hdl, aob_bis_group_info->grp_lid);
            app_ble_audio_sink_streaming_handle_event(AOB_COMMON_MOBILE_CONNECTION_MAX - 1, aob_bis_group_info->play_stream_info[0].stream_lid, APP_GAF_DIRECTION_SINK, BLE_AUDIO_BIS_STREAM_STOP_IND);
            // More than one stream is playing
            if (aob_bis_group_info->play_stream_num > 1)
            {
                app_bap_bc_sink_rx_stream_stop(aob_bis_group_info->play_stream_info[1].bis_hdl, aob_bis_group_info->grp_lid);
                app_ble_audio_sink_streaming_handle_event(AOB_COMMON_MOBILE_CONNECTION_MAX - 1, aob_bis_group_info->play_stream_info[0].stream_lid, APP_GAF_DIRECTION_SINK, BLE_AUDIO_BIS_STREAM_STOP_IND);
            }

            ble_audio_earphone_info_clear_bis_group_info();
            aob_bis_sink_env.bis_stream_bf = 0;
        }
        break;
        default:
            LOG_D("unkonw sink status %d", state);
            break;
    }

    switch (state)
    {
        case APP_GAF_BAP_BC_SINK_LOST:
            err_code = BT_LL_ERR_CON_TIMEOUT;
            break;
        case APP_GAF_BAP_BC_SINK_PEER_TERMINATE:
            err_code = BT_LL_ERR_REMOTE_USER_TERM_CON;
            break;
        case APP_GAF_BAP_BC_SINK_UPPER_TERMINATE:
            err_code = BT_LL_ERR_CON_TERM_BY_LOCAL_HOST;
            break;
        case APP_GAF_BAP_BC_SINK_FAILED:
            err_code = BT_LL_ERR_CON_FAILED_TO_BE_EST;
            break;
        case APP_GAF_BAP_BC_SINK_CANCELLED:
            err_code = BT_LL_ERR_OP_CANCELED_BY_HOST;
            break;
        case APP_GAF_BAP_BC_SINK_MIC_FAILURE:
            err_code = BT_LL_ERR_TERMINATED_MIC_FAILURE;
            break;
        default:
            err_code = BT_LL_ERR_NO_ERROR;
            break;
    }

    if (err_code != BT_LL_ERR_NO_ERROR &&
            aob_bis_sink_env.event_callback.bis_sink_stoped_callback)
    {
        aob_bis_sink_env.event_callback.bis_sink_stoped_callback(grp_lid, err_code);
    }

    if ((NULL != p_ble_core_evt_cb) && (NULL != p_ble_core_evt_cb->ble_bis_sink_status_cb))
    {
        p_ble_core_evt_cb->ble_bis_sink_status_cb(grp_lid, state, stream_pos_bf);
    }
}

static void aob_bis_sink_enabled_cb(uint8_t grp_lid)
{
    LOG_I("Bis sink %d was enabled", grp_lid);
    ble_audio_earphone_info_set_bis_grp_lid(grp_lid);

    uint8_t nb_ase = 0;
    uint8_t ase_lid_list[APP_BAP_DFT_ASCS_NB_ASE_CHAR] = {0};

    aob_bis_sink_env.grp_lid = grp_lid;
    for (int con_lid = 0; con_lid < AOB_COMMON_MOBILE_CONNECTION_MAX; con_lid++)
    {
        nb_ase = aob_media_get_curr_streaming_ase_lid_list(con_lid, ase_lid_list);
        for (int idx = 0; idx < nb_ase; idx++)
        {
            TRACE(2, "stop cis before bis start, ase_lid = %d", ase_lid_list[idx]);
            aob_media_disable_stream(ase_lid_list[idx]);
        }
    }

    if ((NULL != p_ble_core_evt_cb) && (NULL != p_ble_core_evt_cb->ble_bis_sink_enabled_cb))
    {
        p_ble_core_evt_cb->ble_bis_sink_enabled_cb(grp_lid);
    }
}

static void aob_bis_sink_disabled_cb(uint8_t grp_lid)
{
    LOG_I("Bis sink %d was disabled", grp_lid);
    aob_bis_scan_env.bis_scan_metadata_cb = NULL;
    aob_bis_scan_env.bis_scan_select_source_cb = NULL;
    memset(&aob_bis_sink_env.event_callback, 0, sizeof(aob_bis_sink_env.event_callback));
    memset(aob_bis_sink_env.bcast_id, 0, sizeof(aob_bis_sink_env.bcast_id));
    memset(aob_bis_sink_env.bcast_code, 0, sizeof(aob_bis_sink_env.bcast_code));
    ble_audio_earphone_info_set_bis_grp_lid(0);

    if ((NULL != p_ble_core_evt_cb) && (NULL != p_ble_core_evt_cb->ble_bis_sink_disabled_cb))
    {
        p_ble_core_evt_cb->ble_bis_sink_disabled_cb(grp_lid);
    }
}

static void aob_bis_sink_stream_started_cb(uint8_t stream_pos)
{
    uint8_t status;
    uint8_t stream_lid = ble_audio_earphone_info_bis_stream_pos_2_stream_lid(stream_pos);
    AOB_BIS_GROUP_INFO_T *aob_bis_group_info = ble_audio_earphone_info_get_bis_group_info();
    app_gaf_bap_bc_scan_pa_report_info_t *pa_info =
        app_bap_bc_scan_get_exist_pa_info_by_pa_lid(aob_bis_group_info->pa_lid);

    LOG_I("%s stream_pos = %d", __func__, stream_pos);
    aob_bis_group_info->pres_delay = pa_info->group_info.pres_delay_us;
    status = app_bap_bc_sink_rx_stream_start(aob_bis_group_info->bis_hdl[stream_lid], \
                                                     aob_bis_group_info->group_info.max_sdu, \
                                                     aob_bis_group_info->grp_lid);
    if ((!status) && (!aob_bis_sink_env.bis_stream_bf))
    {
        LOG_I("Bis sink pos %d stream was started,", stream_pos);
    }
    else
    {
        LOG_I("Bis sink pos %d stream was started failed, status = %d", stream_pos, status);
    }

    aob_bis_sink_env.bis_stream_bf |= 1UL << stream_pos;

    if ((NULL != p_ble_core_evt_cb) && (NULL != p_ble_core_evt_cb->ble_bis_sink_stream_start_cb))
    {
        p_ble_core_evt_cb->ble_bis_sink_stream_start_cb(stream_lid);
    }
}

static void aob_bis_sink_stream_stoped_cb(uint8_t stream_pos)
{
    AOB_BIS_GROUP_INFO_T *aob_bis_group_info = ble_audio_earphone_info_get_bis_group_info();
    uint8_t stream_lid = ble_audio_earphone_info_bis_stream_pos_2_stream_lid(stream_pos);

    app_bap_bc_sink_rx_stream_stop(aob_bis_group_info->bis_hdl[stream_lid], 0);

    aob_bis_sink_env.bis_stream_bf &= ~(1 << stream_pos);
    if (aob_bis_sink_env.bis_stream_bf)
    {
        for (int i = 0; i < 32; ++i)
        {
            if (aob_bis_sink_env.bis_stream_bf & (1UL << i))
            {
                app_bap_bc_sink_stop_streaming(aob_bis_group_info->grp_lid, i);
                break;
            }
        }
    }
    else
    {
        app_bap_bc_sink_disable(aob_bis_group_info->grp_lid);
    }

    LOG_I("Bis sink pos %d stream was stoped", stream_pos);
    if ((NULL != p_ble_core_evt_cb) && (NULL != p_ble_core_evt_cb->ble_bis_sink_stream_stoped_cb))
    {
        p_ble_core_evt_cb->ble_bis_sink_stream_stoped_cb(stream_lid);
    }
}

static sink_event_handler_t sink_event_cb =
{
    .bis_sink_state_cb          = aob_bis_sink_state_cb,
    .bis_sink_enabled_cb        = aob_bis_sink_enabled_cb,
    .bis_sink_disabled_cb       = aob_bis_sink_disabled_cb,
    .bis_sink_stream_started_cb = aob_bis_sink_stream_started_cb,
    .bis_sink_stream_stoped_cb  = aob_bis_sink_stream_stoped_cb,
};

void aob_bis_sink_api_init(void)
{
    p_ble_core_evt_cb = ble_audio_get_evt_cb();
    aob_mgr_gaf_sink_evt_handler_register(&sink_event_cb);
}
#endif

#if defined(APP_BLE_BIS_ASSIST_ENABLE) || defined(APP_BLE_BIS_DELEG_ENABLE) || defined(APP_BLE_BIS_SINK_ENABLE)
/*BAP Broadcast Scan APIs*/

void aob_bis_start_scan(void)
{
    app_bap_bc_scan_start(APP_GAF_BAP_BC_SINK_SCAN);
}

void aob_bis_stop_scan(void)
{
    app_bap_bc_scan_stop();
}

void aob_bis_scan_set_src_info(uint8_t *bcast_id, uint8_t *addr)
{
    if (bcast_id)
    {
        memcpy(aob_bis_scan_env.bcast_id, bcast_id, APP_GAP_BCAST_ID_LEN);
    }

    if (addr)
    {
        memcpy(aob_bis_scan_env.addr, addr, 6);
    }
}

void aob_bis_scan_pa_sync_with_to(uint8_t *addr, uint8_t addr_type, uint8_t adv_sid, uint16_t sync_to_s)
{
    app_gaf_bap_adv_id_t addr_info;
    memcpy(&addr_info.addr, addr, APP_GAP_BD_ADDR_LEN);
    addr_info.addr_type = addr_type;
    addr_info.adv_sid = adv_sid;

    app_bap_bc_scan_pa_sync_with_to(&addr_info, sync_to_s * 100);
}

void aob_bis_scan_pa_sync(uint8_t *addr, uint8_t addr_type, uint8_t adv_sid)
{
    aob_bis_scan_pa_sync_with_to(addr, addr_type, adv_sid, 0);
}

void aob_bis_scan_pa_sync_cancel(void)
{
    app_bap_bc_scan_pa_sync_cancel();
}

void aob_bis_scan_pa_sync_stop(void)
{
    uint8_t scan_state = app_bap_bc_scan_get_scan_state();
    uint8_t sync_state = app_bap_bc_scan_get_sync_state();

    if (sync_state > APP_GAF_BAP_BC_SCAN_STATE_SCANNING)
    {
        app_bap_bc_scan_pa_terminate(aob_bis_scan_env.pa_lid);
    }
    else if (scan_state > APP_GAF_BAP_BC_SCAN_STATE_IDLE)
    {
        app_bap_bc_scan_stop();
    }
}

void aob_bis_scan_pa_report_ctrl(uint8_t pa_lid, bool enable)
{
    app_bap_bc_scan_pa_report_ctrl(pa_lid, enable);
}

void aob_bis_sink_scan_pa_terminate(uint8_t pa_lid)
{
    app_bap_bc_scan_pa_terminate(pa_lid);
}

void aob_bis_scan_set_scan_param(uint16_t scan_timeout_s, uint16_t intv_1m_slot,
                                 uint16_t intv_coded_slot, uint16_t wd_1m_slot, uint16_t wd_coded_slot)
{
    app_gaf_bap_bc_scan_param_t scan_param;
    scan_param.intv_1m_slot = intv_1m_slot;
    scan_param.intv_coded_slot = intv_coded_slot;
    scan_param.wd_1m_slot = wd_1m_slot;
    scan_param.wd_coded_slot = wd_coded_slot;
    app_bap_bc_scan_set_scan_param(scan_timeout_s, &scan_param);
}

static void aob_bis_scan_state_idle_cb(void)
{
    app_bap_bc_scan_env_t *aob_bap_scan_env = app_bap_bc_scan_get_scan_env();
    aob_bap_scan_env->scan_state = APP_GAF_BAP_BC_SCAN_STATE_IDLE;
    LOG_D("%s scan_state_idle", __func__);

    if (aob_bis_scan_env.bis_scan_state_cb != NULL)
    {
        aob_bis_scan_env.bis_scan_state_cb(true, false, 0);
    }
}

static void aob_bis_scan_state_scanning_cb(void)
{
    LOG_D("%s scan_state_scanning", __func__);

    if (aob_bis_scan_env.bis_scan_state_cb != NULL)
    {
        aob_bis_scan_env.bis_scan_state_cb(true, true, 0);
    }
}

static void aob_bis_scan_state_synchronizing_cb(uint16_t status)
{
    LOG_I("%s scan_state_synchronizing status =%d", __func__, status);
}

static void aob_bis_scan_state_synchronized_cb(void)
{
    LOG_D("%s scan_state_synchronized", __func__);
}

static void aob_bis_scan_state_streaming_cb(app_gaf_bc_scan_state_stream_t *p_scan_state)
{
    AOB_BIS_GROUP_INFO_T *aob_bis_group_info = ble_audio_earphone_info_get_bis_group_info();

    if (APP_GAF_BAP_BC_SINK_TRIGGER == p_scan_state->scan_trigger_method)
    {
        app_gaf_bap_bc_sink_enable_t p_sink_enable;
        p_sink_enable.pa_lid = aob_bis_group_info->pa_lid;
        p_sink_enable.mse = 0;
        ///TODO:Maybe only need to sink with one bis, not all in stream_pos_bf
        p_sink_enable.stream_pos_bf = aob_bis_group_info->stream_pos_bf >> 1;
        p_sink_enable.timeout_10ms = 100;
        app_bap_bc_scan_env_t *aob_app_bc_scan = (app_bap_bc_scan_env_t *)app_bap_bc_scan_get_scan_env;
        memcpy(&p_sink_enable.bcast_id.id, &aob_app_bc_scan->bcast_id.id, APP_GAF_BAP_BC_BROADCAST_ID_LEN); //get bcast_id

        p_sink_enable.encrypted = p_scan_state->encrypted;//BIS should get encrypted from BIGInfo_Advertising_Report
        if (p_sink_enable.encrypted)
        {
            memcpy(p_sink_enable.bcast_code.bcast_code, aob_bis_sink_env.bcast_code, APP_GAP_KEY_LEN);
            LOG_D("%s slave&master generate bcast_code %d", __func__, p_sink_enable.stream_pos_bf);
        }
#ifdef APP_BLE_BIS_SINK_ENABLE
        app_bap_bc_sink_enable((app_gaf_bap_bc_sink_enable_t *)&p_sink_enable);
#endif
    }

    if (APP_GAF_BAP_BC_DELEG_TRIGGER != p_scan_state->scan_trigger_method)
    {
        app_bap_bc_scan_stop();
    }
}

static void aob_bis_scan_pa_sync_req_cb(uint8_t pa_lid)
{
    LOG_D("%s pa_lid %d", __func__, pa_lid);
#ifdef APP_BLE_BIS_DELEG_ENABLE
    aob_bis_deleg_pa_sync_ri(pa_lid, true);
#endif
}

static void aob_bis_scan_pa_terminate_req_cb(uint8_t pa_lid)
{
    LOG_D("%s pa_lid %d", __func__, pa_lid);
#ifdef APP_BLE_BIS_DELEG_ENABLE
    aob_bis_deleg_pa_terminate_ri(pa_lid, true);
#endif
}

static void aob_bis_scan_pa_established_cb(uint8_t pa_lid, uint8_t *addr, uint8_t addr_type,
                                           uint8_t adv_sid, uint16_t serv_data)
{
    LOG_D("aob_bap bc scan pa established pa_lid = %d, serv_data = 0x%04x, addr:", pa_lid, serv_data);
    DUMP8("%02x ", addr, GAP_BD_ADDR_LEN);
    aob_bis_scan_env.pa_lid = pa_lid;
    ble_audio_earphone_info_set_bis_pa_lid(pa_lid);

    if (aob_bis_scan_env.bis_scan_state_cb != NULL)
    {
        uint32_t param = (pa_lid << 16);
        aob_bis_scan_env.bis_scan_state_cb(false, true, param);
    }
}

static void aob_bis_scan_pa_terminated_cb(uint8_t pa_lid, uint16_t reason)
{
    LOG_D("aob_bap bc scan pa terminated pa_lid %d reason 0x%04x", pa_lid, reason);
    ble_audio_earphone_info_set_bis_pa_lid(BLE_INVALID_LINK_ID);

    if (aob_bis_scan_env.bis_scan_state_cb != NULL)
    {
        uint32_t param = (pa_lid << 16) | reason;
        aob_bis_scan_env.bis_scan_state_cb(false, false, param);
    }
}

static void aob_bis_scan_subgrp_info_report_cb(app_gaf_bc_scan_subgroup_report_ind_t *subgrp_info)
{
    LOG_D("aob_bap bc scan subgrp_lid=%d", subgrp_info->sgrp_id);

    if (aob_bis_scan_env.bis_scan_metadata_cb)
    {
        aob_bis_scan_env.bis_scan_metadata_cb(subgrp_info->sgrp_id,
            (uint8_t*)subgrp_info->metadata_val, subgrp_info->metadata_len);
    }
}

static void aob_bis_scan_stream_info_report_cb(app_gaf_bc_scan_stream_report_ind_t *stream_info)
{
    LOG_D("aob_bap bc scan stream pos %d", stream_info->stream_pos);

    ble_audio_earphone_info_set_bis_stream_pos_bf(1 << (stream_info->stream_pos));
}

static void aob_bis_scan_big_info_report_cb(app_gaf_bc_scan_big_info_report_ind_t *big_info)
{
    AOB_BIS_GROUP_INFO_T *aob_bis_group_info = ble_audio_earphone_info_get_bis_group_info();
    memcpy(&aob_bis_group_info->group_info, &big_info->report, sizeof(app_gaf_big_info_t));
}

static void aob_bis_scan_report_cb(app_gaf_bap_adv_id_t *p_addr_info, uint8_t *bcast_id,
                                        uint8_t *adv_data, uint8_t adv_data_len, int8_t rssi, bool *choose_to_sync)
{
    *choose_to_sync = false;

    if (aob_bis_scan_env.bis_scan_select_source_cb)
    {
        *choose_to_sync = aob_bis_scan_env.bis_scan_select_source_cb((ble_bdaddr_t *)p_addr_info->addr,
                                                        p_addr_info->adv_sid, bcast_id, adv_data, adv_data_len, rssi);
    }
    else
    {
        DUMP8("%02x ", aob_bis_scan_env.bcast_id, 3);
        DUMP8("%02x ", bcast_id, 3);
        DUMP8("%02x ", aob_bis_scan_env.addr, 6);
        DUMP8("%02x ", p_addr_info->addr, 6);
        if ((!memcmp(aob_bis_scan_env.bcast_id, bcast_id, 3)) || (!memcmp(aob_bis_scan_env.addr, p_addr_info->addr, 6)))
        {
            *choose_to_sync = true;
        }
    }
}

static scan_event_handler_t scan_event_cb =
{
    .scan_state_idle_cb          = aob_bis_scan_state_idle_cb,
    .scan_state_scanning_cb      = aob_bis_scan_state_scanning_cb,
    .scan_state_synchronizing_cb = aob_bis_scan_state_synchronizing_cb,
    .scan_state_synchronized_cb  = aob_bis_scan_state_synchronized_cb,
    .scan_state_streaming_cb     = aob_bis_scan_state_streaming_cb,
    .scan_pa_sync_req_cb         = aob_bis_scan_pa_sync_req_cb,
    .scan_pa_terminate_req_cb    = aob_bis_scan_pa_terminate_req_cb,
    .scan_pa_established_cb      = aob_bis_scan_pa_established_cb,
    .scan_pa_terminated_cb       = aob_bis_scan_pa_terminated_cb,
    .scan_subgrp_report_cb       = aob_bis_scan_subgrp_info_report_cb,
    .scan_stream_report_cb       = aob_bis_scan_stream_info_report_cb,
    .scan_big_info_report_cb     = aob_bis_scan_big_info_report_cb,
    .scan_report_cb              = aob_bis_scan_report_cb,
};

void aob_bis_scan_api_init(void)
{
    aob_mgr_gaf_scan_evt_handler_register(&scan_event_cb);
}
#endif
