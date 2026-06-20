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
#if defined(APP_BLE_BIS_ASSIST_ENABLE) || defined(APP_BLE_BIS_DELEG_ENABLE) || defined(APP_BLE_BIS_SINK_ENABLE)
#include "ble_app_dbg.h"
#include "app_bap.h"
#include "app_ble.h"

#include "app_gaf.h"
#include "app_gaf_custom_api.h"

#include "app_bap_bc_scan_msg.h"

#include "bap_broadcast_scan.h"

#define APP_GAF_BAP_BC_INVALID_ID       0xFF

app_bap_bc_scan_env_t *p_app_bc_scan_env = NULL;

/*Functions*/
app_gaf_bap_bc_scan_pa_report_info_t *app_bap_bc_scan_get_exist_pa_info_by_pa_lid(uint8_t pa_lid)
{
    if (p_app_bc_scan_env == NULL || pa_lid >= APP_BAP_DFT_BC_SCAN_NB_SYNC)
    {
        return NULL;
    }

    return &p_app_bc_scan_env->pa_info[pa_lid];
}

app_gaf_bap_bc_scan_pa_report_info_t *app_bap_bc_scan_get_exist_pa_info_by_addr(const app_gaf_bap_adv_id_t *p_pa_addr)
{
    if (p_pa_addr == NULL)
    {
        return NULL;
    }

    uint8_t idx = 0;

    for (idx = 0; idx < APP_BAP_DFT_BC_SCAN_NB_SYNC; idx++)
    {
        if (!memcmp(&p_app_bc_scan_env->pa_info[idx].pa_addr, p_pa_addr, sizeof(*p_pa_addr)))
        {
            return &p_app_bc_scan_env->pa_info[idx];
        }
    }

    return NULL;
}

app_gaf_bap_bc_scan_pa_report_info_t *app_bap_bc_scan_get_exist_pa_info_by_hdl(uint16_t pa_sync_hdl)
{
    uint8_t idx = 0;

    for (idx = 0; idx < APP_BAP_DFT_BC_SCAN_NB_SYNC; idx++)
    {
        if (p_app_bc_scan_env->pa_info[idx].pa_sync_hdl == pa_sync_hdl)
        {
            return &p_app_bc_scan_env->pa_info[idx];
        }
    }

    return NULL;
}

app_gaf_bap_bc_scan_pa_report_info_t *app_bap_bc_scan_get_ava_pa_info(void)
{
    if (p_app_bc_scan_env == NULL)
    {
        return NULL;
    }

    uint8_t idx = 0;

    for (idx = 0; idx < APP_BAP_DFT_BC_SCAN_NB_SYNC; idx++)
    {
        if (p_app_bc_scan_env->pa_info[idx].pa_sync_hdl == BAP_BC_INVALID_PA_SYNC_HDL)
        {
            memset(&p_app_bc_scan_env->pa_info[idx], 0, sizeof(p_app_bc_scan_env->pa_info[idx]));

            p_app_bc_scan_env->pa_info[idx].pa_lid = idx;
            return &p_app_bc_scan_env->pa_info[idx];
        }
    }

    return NULL;
}

static void app_bap_bc_scan_set_scan_state(uint8_t scan_state)
{
    LOG_I("app_bap bc_scan scan_state = %d", scan_state);
    p_app_bc_scan_env->scan_state &= 0xF0;
    p_app_bc_scan_env->scan_state |= (scan_state & 0x0F);
}

static void app_bap_bc_scan_set_sync_state(uint8_t sync_state)
{
    LOG_I("app_bap bc_scan sync_state = %d", sync_state);

    p_app_bc_scan_env->scan_state &= 0x0F;
    p_app_bc_scan_env->scan_state |= sync_state << 4;
}

uint8_t app_bap_bc_scan_get_scan_state(void)
{
    if (p_app_bc_scan_env == NULL)
    {
        return 0;
    }

    return (p_app_bc_scan_env->scan_state & 0x0F);
}

uint8_t app_bap_bc_scan_get_sync_state(void)
{
    if (p_app_bc_scan_env == NULL)
    {
        return 0;
    }

    return ((p_app_bc_scan_env->scan_state & 0xF0) >> 4);
}

static void app_bap_bc_scan_cb_scan_started(uint16_t err_code)
{
    LOG_I("%s err_code = %d", __func__, err_code);
    app_bap_bc_scan_set_scan_state(APP_GAF_BAP_BC_SCAN_STATE_SCANNING);

    app_gaf_evt_report(APP_BAP_BC_SCAN_STATE_SCANNING_IND, NULL, 0);
}

static void app_bap_bc_scan_cb_scan_stopped(uint16_t err_code)
{
    LOG_I("%s err_code = %d", __func__, err_code);

    app_bap_bc_scan_set_scan_state(APP_GAF_BAP_BC_SCAN_STATE_IDLE);

    if (err_code == BT_STS_ADV_TIMER_TIMEOUT)
    {
        app_gaf_evt_report(APP_GAF_SCAN_TIMEOUT_IND, NULL, 0);
    }

    app_gaf_evt_report(APP_BAP_BC_SCAN_STATE_IDLE_IND, NULL, 0);
}

static void app_bap_bc_scan_cb_scan_extended_adv_recv(const uint8_t *broadcast_id, const pbp_broadcast_name_ptr_t *p_bc_name,
                                                      const pbp_pba_info_t *p_pba_info, const gap_adv_report_t *p_ext_adv_raw)
{
    if (p_bc_name != NULL && p_bc_name->name_length != 0)
    {
        LOG_I("%s broadcast_name =", __func__);
        DUMP8("%c", p_bc_name->p_name, p_bc_name->name_length);
    }

    if (p_pba_info != NULL)
    {
        LOG_I("%s PBA features = 0x%x", __func__, p_pba_info->pba_features);
    }

    if (broadcast_id == NULL || p_ext_adv_raw == NULL)
    {
        return;
    }

    LOG_I("%s rssi = %d, broadcast_id =", __func__, p_ext_adv_raw->rssi);
    DUMP8("%02x ", broadcast_id, APP_GAF_BAP_BC_BROADCAST_ID_LEN);
    LOG_I("%s addr type = %d, adv_sid = %d, address =", __func__, p_ext_adv_raw->peer_type, p_ext_adv_raw->adv_set_id);
    DUMP8("%02x ", p_ext_adv_raw->peer_addr.address, sizeof(p_ext_adv_raw->peer_addr.address));

    app_gaf_bc_scan_adv_report_t scan_adv_report;
    scan_adv_report.scan_trigger_method = app_bap_bc_scan_get_scan_env()->scan_trigger_method;
    // Adv addr info
    memcpy(scan_adv_report.adv_report.adv_id.addr, p_ext_adv_raw->peer_addr.address, APP_GAP_BD_ADDR_LEN);
    scan_adv_report.adv_report.adv_id.addr_type = p_ext_adv_raw->peer_type;
    scan_adv_report.adv_report.adv_id.adv_sid = p_ext_adv_raw->adv_set_id;
    scan_adv_report.rssi = p_ext_adv_raw->rssi;
    // Adv data info
    scan_adv_report.adv_report.length = p_ext_adv_raw->data_length;
    memcpy(scan_adv_report.adv_report.data, p_ext_adv_raw->data, p_ext_adv_raw->data_length);
    memcpy(scan_adv_report.bcast_id.id, broadcast_id, APP_GAF_BAP_BC_BROADCAST_ID_LEN);//copy bcast_id from Extended_Advertising_Report
    //Transfer ervery Recv Scan result to GAF_EVT
    app_gaf_evt_report(APP_GAF_SCAN_REPORT_IND, (void *)&scan_adv_report, sizeof(app_gaf_bc_scan_adv_report_t));
}

static void app_bap_bc_scan_cb_pa_sync_state(const bap_bc_pa_addr_t *p_pa_addr, uint16_t pa_sync_hdl, uint16_t err_code)
{
    LOG_I("%s addr type = %d, adv_sid = %d", __func__, p_pa_addr->adv_addr.addr_type, p_pa_addr->adv_sid);
    LOG_I("%s err_code = %d, pa_sync_hdl = 0x%x", __func__, err_code, pa_sync_hdl);

    if (err_code != BT_STS_SUCCESS)
    {
        LOG_W("%s pa sync failed", __func__);

        app_bap_bc_scan_set_sync_state(APP_GAF_BAP_BC_SCAN_STATE_IDLE);

        app_gaf_bc_scan_pa_terminated_ind_t pa_terminated_ind =
        {
            .pa_lid = APP_GAF_BAP_BC_INVALID_ID,
            .reason = err_code,
        };

        app_gaf_evt_report(APP_GAF_SCAN_PA_TERMINATED_IND, (void *)&pa_terminated_ind, sizeof(pa_terminated_ind));
        return;
    }

    app_gaf_bap_bc_scan_pa_report_info_t *pa_info = app_bap_bc_scan_get_exist_pa_info_by_hdl(pa_sync_hdl);

    if (pa_info == NULL)
    {
        pa_info = app_bap_bc_scan_get_ava_pa_info();
    }

    if (pa_info == NULL)
    {
        LOG_W("%s no resources for pa", __func__);
        return;
    }

    LOG_I("app_bap bc scan pa established pa_lid = %d, addr:", pa_info->pa_lid);
    DUMP8("%02x ", p_pa_addr->adv_addr.addr, sizeof(p_pa_addr->adv_addr.addr));

    pa_info->pa_addr = *(app_gaf_bap_adv_id_t *)p_pa_addr;
    pa_info->pa_sync_hdl = pa_sync_hdl;

    app_bap_bc_scan_set_sync_state(APP_GAF_BAP_BC_SCAN_STATE_SYNCHRONIZED);

    app_gaf_evt_report(APP_BAP_BC_SCAN_STATE_SYNCHRONIZED_IND, NULL, 0);
}

static void app_bap_bc_scan_cb_pa_data_recv(uint16_t pa_sync_hdl, const bap_bc_base_grp_info_t *p_base_info, const gap_adv_report_t *p_periodic_adv_raw)
{
    // Subgroup ID
    uint8_t subgrp_id;
    uint8_t num_stream = 0;
    const bap_bc_base_sub_grp_t *p_subgrp = NULL;
    app_gaf_bap_bc_scan_pa_report_info_t *pa_info = app_bap_bc_scan_get_exist_pa_info_by_hdl(pa_sync_hdl);
    app_gaf_bc_scan_subgroup_report_ind_t p_subgroup_report = {0};

    LOG_I("%s pa_sync_hdl = 0x%x", __func__, pa_sync_hdl);
    if (pa_info == NULL)
    {
        return;
    }

    app_gaf_bap_bc_scan_pa_established_ind_t scan_pa_established =
    {
        .adv_id = pa_info->pa_addr,
        .interval_frames = p_periodic_adv_raw->pa_interval,
        .pa_lid = pa_info->pa_lid,
        .phy = p_periodic_adv_raw->adv.secondary_phy,
    };

    // Only report first time
    if (pa_info->subgroup_info == NULL)
    {
        app_bap_bc_scan_set_sync_state(APP_GAF_BAP_BC_SCAN_STATE_SYNCHRONIZED);
        app_gaf_evt_report(APP_GAF_SCAN_PA_ESTABLISHED_IND, (void *)&scan_pa_established, sizeof(scan_pa_established));
    }

    if (p_base_info == NULL)
    {
        LOG_W("%s base is NULL", __func__);
        return;
    }

    // Level 1******************************
    if ((pa_info->subgroup_info) && (pa_info->group_info.nb_subgroups != p_base_info->num_sub_grp))
    {
        app_gaf_free_buff(pa_info->subgroup_info);
        pa_info->subgroup_info = NULL;
        pa_info->group_info.nb_subgroups = 0;
    }

    if (!pa_info->subgroup_info)
    {
        pa_info->subgroup_info = (app_gaf_bap_bc_scan_subgrp_info_t *)app_gaf_malloc_buff(
            sizeof(app_gaf_bap_bc_scan_subgrp_info_t) * p_base_info->num_sub_grp);
        if (pa_info->subgroup_info == NULL)
        {
            LOG_E("%s no resources", __func__);
            return;
        }
        memset(pa_info->subgroup_info, 0, sizeof(app_gaf_bap_bc_scan_subgrp_info_t) * p_base_info->num_sub_grp);
        pa_info->group_info.nb_subgroups = p_base_info->num_sub_grp;
    }
    pa_info->group_info.pres_delay_us = p_base_info->pres_delay;

    p_subgrp = &p_base_info->sub_grp[0];
    for (subgrp_id = 0; subgrp_id < p_base_info->num_sub_grp; subgrp_id++)
    {
        // Total Number of BIS in this Group
        num_stream += p_subgrp->num_bis;
        p_subgrp = (bap_bc_base_sub_grp_t *)((uint32_t)p_subgrp + p_subgrp->num_bis * sizeof(bap_bc_base_bis_info_t));
    }

    p_subgrp = &p_base_info->sub_grp[0];

    for (subgrp_id = 0; subgrp_id < p_base_info->num_sub_grp; subgrp_id++)
    {
        // Level 2**************************
        pa_info->subgroup_info[subgrp_id].sgrp_id = subgrp_id;
        // Codec ID
        pa_info->subgroup_info[subgrp_id].codec_id = *(app_gaf_codec_id_t *)p_subgrp->codec_id;

        /// TOOD: fetach add codec cfg
        if (p_subgrp->p_codec_cfg != NULL)
        {
            pa_info->subgroup_info[subgrp_id].param.frame_dur = p_subgrp->p_codec_cfg->basic_cc_param.frame_dur;
            pa_info->subgroup_info[subgrp_id].param.frames_sdu = p_subgrp->p_codec_cfg->basic_cc_param.frame_blocks_per_sdu;
            pa_info->subgroup_info[subgrp_id].param.frame_octet = p_subgrp->p_codec_cfg->basic_cc_param.frame_octets;
            pa_info->subgroup_info[subgrp_id].param.sampling_freq = p_subgrp->p_codec_cfg->basic_cc_param.sampling_freq;
            pa_info->subgroup_info[subgrp_id].param.location_bf = p_subgrp->p_codec_cfg->basic_cc_param.audio_chan_allocation_bf;
        }
        p_subgroup_report.pa_lid = pa_info->pa_lid;
        p_subgroup_report.sgrp_id = subgrp_id;
        p_subgroup_report.codec_id = *(app_gaf_codec_id_t *)p_subgrp->codec_id;
        p_subgroup_report.stream_pos_bf = 0;
        p_subgroup_report.cfg_param.location_bf = p_subgrp->p_codec_cfg->basic_cc_param.audio_chan_allocation_bf;
        p_subgroup_report.cfg_param.frame_octet = p_subgrp->p_codec_cfg->basic_cc_param.frame_octets;
        p_subgroup_report.cfg_param.sampling_freq = p_subgrp->p_codec_cfg->basic_cc_param.sampling_freq;
        p_subgroup_report.cfg_param.frame_dur = p_subgrp->p_codec_cfg->basic_cc_param.frame_dur;
        p_subgroup_report.cfg_param.frames_sdu = p_subgrp->p_codec_cfg->basic_cc_param.frame_blocks_per_sdu;

        /// TOOD: fetach add metadata
        if (p_subgrp->p_metadata != NULL)
        {
            pa_info->subgroup_info[subgrp_id].param_metadata.context_bf =
                p_subgrp->p_metadata->basic_metadata.streaming_audio_context;
            p_subgroup_report.param_metadata.context_bf = p_subgrp->p_metadata->basic_metadata.streaming_audio_context;
        }

        p_subgroup_report.cfg_len = p_subgrp->p_codec_cfg ? p_subgrp->p_codec_cfg->add_cc_param.len : 0;
        p_subgroup_report.metadata_len = p_subgrp->p_metadata ? p_subgrp->p_metadata->add_metadata.len : 0;
        p_subgroup_report.cfg_val = p_subgrp->p_codec_cfg->add_cc_param.data;
        p_subgroup_report.metadata_val = p_subgrp->p_metadata->add_metadata.data;
        app_gaf_evt_report(APP_GAF_SCAN_SUBGROUP_REPORT_IND, (uint8_t*)&p_subgroup_report,
                            sizeof(app_gaf_bc_scan_subgroup_report_ind_t));
        p_subgrp = (bap_bc_base_sub_grp_t *)((uint32_t)p_subgrp + p_subgrp->num_bis * sizeof(bap_bc_base_bis_info_t));
    }

    // Level 3*************************************
    if ((pa_info->stream_info) && (pa_info->group_info.nb_streams != num_stream))
    {
        app_gaf_free_buff(pa_info->stream_info);
        pa_info->stream_info = NULL;
        pa_info->group_info.nb_streams = 0;
    }

    if (!pa_info->stream_info)
    {
        pa_info->stream_info = (app_gaf_bap_bc_scan_stream_info_t *)app_gaf_malloc_buff(
            sizeof(app_gaf_bap_bc_scan_stream_info_t) * num_stream);
        if (pa_info->stream_info == NULL)
        {
            LOG_E("%s no resources", __func__);
            /// TODO: maybe subgrp contains add cc
            app_gaf_free_buff(pa_info->subgroup_info);
            pa_info->subgroup_info = NULL;
            memset(&pa_info->group_info, 0, sizeof(pa_info->group_info));
            return;
        }
        memset(pa_info->stream_info, 0, sizeof(app_gaf_bap_bc_scan_stream_info_t) * num_stream);
        pa_info->group_info.nb_streams = num_stream;
        LOG_I("%s num_bis = %d", __func__, num_stream);
    }

    // Jump to fist subgrp
    p_subgrp =  &p_base_info->sub_grp[0];

    uint8_t stream_lid = 0;

    app_gaf_bc_scan_stream_report_ind_t stream_report = {0};

    const bap_bc_base_bis_info_t *p_specific_bis = NULL;

    for (subgrp_id = 0; subgrp_id < p_base_info->num_sub_grp; subgrp_id++)
    {
        uint8_t specific_bis_idx = 0;

        p_specific_bis = &p_subgrp->specific_bis_info[0];

        for (specific_bis_idx = 0; specific_bis_idx < p_subgrp->num_bis; specific_bis_idx++)
        {
            pa_info->subgroup_info[subgrp_id].stream_pos_bf |= (1 << (p_specific_bis[specific_bis_idx].bis_idx));
            /// TOOD: There is no codec id in level 3
            pa_info->stream_info[stream_lid].codec_id = pa_info->subgroup_info[subgrp_id].codec_id;
            pa_info->stream_info[stream_lid].sgrp_id = subgrp_id;
            pa_info->stream_info[stream_lid].stream_pos = p_specific_bis[specific_bis_idx].bis_idx;
            // Codec Cfg
            if (p_specific_bis[specific_bis_idx].p_codec_cfg != NULL)
            {
                pa_info->stream_info[stream_lid].param.frame_dur =
                    p_specific_bis[specific_bis_idx].p_codec_cfg->basic_cc_param.frame_dur;
                pa_info->stream_info[stream_lid].param.frames_sdu =
                    p_specific_bis[specific_bis_idx].p_codec_cfg->basic_cc_param.frame_blocks_per_sdu;
                pa_info->stream_info[stream_lid].param.frame_octet =
                    p_specific_bis[specific_bis_idx].p_codec_cfg->basic_cc_param.frame_octets;
                pa_info->stream_info[stream_lid].param.sampling_freq =
                    p_specific_bis[specific_bis_idx].p_codec_cfg->basic_cc_param.sampling_freq;
                pa_info->stream_info[stream_lid].param.location_bf =
                    p_specific_bis[specific_bis_idx].p_codec_cfg->basic_cc_param.audio_chan_allocation_bf;
            }

            // Report stream of this bis to upper
            stream_report.stream_pos = pa_info->stream_info[stream_lid].stream_pos;
            stream_report.codec_id = pa_info->stream_info[stream_lid].codec_id;
            stream_report.pa_lid = pa_info->pa_lid;
            stream_report.sgrp_id = subgrp_id;
            stream_report.cfg.param = pa_info->stream_info[stream_lid].param;
            /// TODO: Add codec cfg
            stream_report.cfg.add_cfg.len = 0;

            LOG_I("app_bap bc scan stream report stream_pos = No.%d", stream_report.stream_pos);

            app_gaf_evt_report(APP_GAF_SCAN_STREAM_REPORT_IND, (void *)&stream_report, sizeof(stream_report));

            // Stream local id
            stream_lid++;
        }

        p_subgrp = (bap_bc_base_sub_grp_t *)((uint32_t)p_subgrp + p_subgrp->num_bis * sizeof(bap_bc_base_bis_info_t));
    }
}

static void app_bap_bc_scan_cb_pa_big_info_recv(uint16_t pa_sync_hdl, const bap_bc_big_info_t *p_big_info)
{
    LOG_I("%s pa_sync_hdl = 0x%x, is_big_enc = %d", __func__, pa_sync_hdl, p_big_info->encrypted);
    app_gaf_bap_bc_scan_pa_report_info_t *pa_info = app_bap_bc_scan_get_exist_pa_info_by_hdl(pa_sync_hdl);

    if (pa_info == NULL)
    {
        return;
    }

    pa_info->big_info = *(app_gaf_bap_bc_scan_big_info_t *)p_big_info;

    app_gaf_bc_scan_big_info_report_ind_t big_info_report =
    {
        .report = *(app_gaf_big_info_t *) &pa_info->big_info,
        .pa_lid = pa_info->pa_lid,
    };

    LOG_I("app_bap bc scan big_info report pa_lid = %d, scan state = %d",
        big_info_report.pa_lid, app_bap_bc_scan_get_scan_env()->scan_state);

    app_gaf_evt_report(APP_GAF_SCAN_BIGINFO_REPORT_IND, (void *)&big_info_report, sizeof(big_info_report));

    if (app_bap_bc_scan_get_sync_state() == APP_GAF_BAP_BC_SCAN_STATE_SYNCHRONIZED)
    {
        app_bap_bc_scan_set_sync_state(APP_GAF_BAP_BC_SCAN_STATE_STREAMING);

        app_gaf_bc_scan_state_stream_t scan_state_report =
        {
            .scan_trigger_method = app_bap_bc_scan_get_scan_env()->scan_trigger_method,
            .encrypted = big_info_report.report.encrypted,
        };

        app_gaf_evt_report(APP_BAP_BC_SCAN_STATE_STREAMING_IND, (void *)&scan_state_report, sizeof(scan_state_report));
    }
}

static void app_bap_bc_scan_cb_pa_sync_term(uint16_t pa_sync_hdl, uint16_t err_code)
{
    LOG_I("%s err_code = %d, pa_sync_hdl = 0x%x", __func__, err_code, pa_sync_hdl);

    app_gaf_bap_bc_scan_pa_report_info_t *pa_info = app_bap_bc_scan_get_exist_pa_info_by_hdl(pa_sync_hdl);

    if (pa_info == NULL)
    {
        return;
    }

    app_bap_bc_scan_set_sync_state(APP_GAF_BAP_BC_SCAN_STATE_IDLE);

    app_gaf_bc_scan_pa_terminated_ind_t pa_terminated_ind =
    {
        .pa_lid = pa_info->pa_lid,
        .reason = err_code,
    };

    app_gaf_evt_report(APP_GAF_SCAN_PA_TERMINATED_IND, (void *)&pa_terminated_ind, sizeof(pa_terminated_ind));

    LOG_I("Cleanup PA info");
    // Stream local id
    uint8_t stream_lid;

    for (stream_lid = 0; stream_lid < pa_info->group_info.nb_streams; stream_lid++)
    {
        if (pa_info->stream_info[stream_lid].ltv_data != NULL)
        {
            app_gaf_free_buff(pa_info->stream_info[stream_lid].ltv_data);
        }
    }

    app_gaf_free_buff(pa_info->stream_info);

    pa_info->stream_info = NULL;

    // Subgroup ID
    uint8_t subgrp_id;

    for (subgrp_id = 0; subgrp_id < pa_info->group_info.nb_subgroups; subgrp_id++)
    {
        if (pa_info->subgroup_info[subgrp_id].val != NULL)
        {
            app_gaf_free_buff(pa_info->subgroup_info[subgrp_id].val);
        }
    }

    app_gaf_free_buff(pa_info->subgroup_info);

    pa_info->subgroup_info = NULL;

    memset(&pa_info->group_info, 0, sizeof(pa_info->group_info));

    pa_info->pa_sync_hdl = BAP_BC_INVALID_PA_SYNC_HDL;
}

__STATIC const bap_bc_scan_callback_t app_bap_bc_scan_app_cb =
{
    .cb_scan_started = app_bap_bc_scan_cb_scan_started,
    .cb_scan_stopped = app_bap_bc_scan_cb_scan_stopped,
    .cb_ext_adv_recv = app_bap_bc_scan_cb_scan_extended_adv_recv,
    .cb_pa_sync_state = app_bap_bc_scan_cb_pa_sync_state,
    .cb_per_adv_data_recv = app_bap_bc_scan_cb_pa_data_recv,
    .cb_pa_sync_term = app_bap_bc_scan_cb_pa_sync_term,
    .cb_big_info_recv = app_bap_bc_scan_cb_pa_big_info_recv,
};

int app_bap_bc_scan_init(void)
{
    LOG_I("%s", __func__);

    gap_scan_timing_t scan_timing =
    {
        .fg_scan_window_ms = ((p_app_bc_scan_env->scan_param.intv_1m_slot * 5) / 8),
        .bg_scan_window_ms = ((p_app_bc_scan_env->scan_param.wd_1m_slot * 5) / 8),
        .fg_scan_interval_ms = ((p_app_bc_scan_env->scan_param.intv_1m_slot * 5) / 8),
        .bg_scan_interval_ms = ((p_app_bc_scan_env->scan_param.wd_1m_slot * 5) / 8),
        .fg_scan_interval_coded_ms = ((p_app_bc_scan_env->scan_param.intv_coded_slot * 5) / 8),
        .fg_scan_window_coded_ms = ((p_app_bc_scan_env->scan_param.wd_coded_slot * 5) / 8),
        .bg_scan_interval_coded_ms = ((p_app_bc_scan_env->scan_param.wd_coded_slot * 5) / 8),
        .bg_scan_window_coded_ms = ((p_app_bc_scan_env->scan_param.wd_coded_slot * 5) / 8),
    };

    bap_bc_scan_init_cfg_t init_cfg =
    {
        .p_scan_timing = &scan_timing,
    };

    uint16_t status = bap_bc_scan_init(&init_cfg);

    if (status == BT_STS_SUCCESS)
    {
        p_app_bc_scan_env->scan_para_set_done = true;
        status = bap_bc_scan_callback_register(BAP_BC_COMMON_USER_APPLICATION_IMPL, &app_bap_bc_scan_app_cb);
    }

    return status;
}

int app_bap_bc_scan_deinit(void)
{
    LOG_I("%s", __func__);

    uint16_t status = bap_bc_scan_deinit();

    if (status == BT_STS_SUCCESS)
    {
        status = bap_bc_scan_callback_unregister(BAP_BC_COMMON_USER_APPLICATION_IMPL);
    }

    return status;
}

int app_bap_bc_scan_info_init(void)
{
    if (p_app_bc_scan_env == NULL)
    {
        p_app_bc_scan_env = (app_bap_bc_scan_env_t *)app_gaf_malloc_buff(sizeof(*p_app_bc_scan_env));

        if (p_app_bc_scan_env == NULL)
        {
            LOG_E("%s no resources", __func__);
            return BT_STS_NO_RESOURCES;
        }

        memset(p_app_bc_scan_env, 0, sizeof(app_bap_bc_scan_env_t));

        p_app_bc_scan_env->scan_para_set_done = false;
        p_app_bc_scan_env->cache_size = APP_BAP_DFT_BC_SCAN_CACHE_SIZE;
        p_app_bc_scan_env->nb_sync = APP_BAP_DFT_BC_SCAN_NB_SYNC;
        p_app_bc_scan_env->scan_trigger_method = APP_GAF_BAP_BC_SINK_TRIGGER;
        p_app_bc_scan_env->scan_state = 0;

        //Init Scan Params
        p_app_bc_scan_env->scan_timeout_s = APP_BAP_DFT_BC_SCAN_SCAN_TIMEOUT_S;
        p_app_bc_scan_env->scan_param.intv_1m_slot = APP_BAP_DFT_BC_SCAN_INTV_1M_SLOT;
        p_app_bc_scan_env->scan_param.intv_coded_slot = APP_BAP_DFT_BC_SCAN_INTV_CODED_SLOT;
        p_app_bc_scan_env->scan_param.wd_1m_slot = APP_BAP_DFT_BC_SCAN_WD_1M_SLOT;
        p_app_bc_scan_env->scan_param.wd_coded_slot = APP_BAP_DFT_BC_SCAN_WD_CODED_SLOT;

        //Init Sync Params
        memset(&p_app_bc_scan_env->sync_param.adv_id, 0, sizeof(bap_bc_pa_addr_t));
        p_app_bc_scan_env->sync_param.skip = APP_BAP_DFT_BC_SCAN_SKIP;
        p_app_bc_scan_env->sync_param.report_filter_bf = APP_BAP_DFT_BC_SCAN_REPORT_FILTER_BF;
        p_app_bc_scan_env->sync_param.sync_to_10ms = APP_BAP_DFT_BC_SCAN_SYNC_TO_10MS;
        p_app_bc_scan_env->sync_param.timeout_s = APP_BAP_DFT_BC_SCAN_TIMEOUT_S;

        for (uint8_t pa_idx = 0; pa_idx < APP_BAP_DFT_BC_SCAN_NB_SYNC; pa_idx++)
        {
            app_gaf_bap_bc_scan_pa_report_info_t *pa_info = &p_app_bc_scan_env->pa_info[pa_idx];
            memset(pa_info, 0, sizeof(app_gaf_bap_bc_scan_pa_report_info_t));
            pa_info->pa_lid = APP_GAF_BAP_BC_INVALID_ID;
            pa_info->pa_sync_hdl = BAP_BC_INVALID_PA_SYNC_HDL;
        }
    }

    return 0;
}

int app_bap_bc_scan_info_deinit(void)
{
    if (p_app_bc_scan_env == NULL)
    {
        return BT_STS_SUCCESS;
    }

    uint8_t idx = 0;

    for (idx = 0; idx < APP_BAP_DFT_BC_SCAN_NB_SYNC; idx++)
    {
        app_gaf_bap_bc_scan_pa_report_info_t *pa_info = &p_app_bc_scan_env->pa_info[idx];

        if (pa_info->pa_sync_hdl != BAP_BC_INVALID_PA_SYNC_HDL)
        {
            // Stream local id
            uint8_t stream_lid;

            for (stream_lid = 0; stream_lid < pa_info->group_info.nb_streams; stream_lid++)
            {
                if (pa_info->stream_info[stream_lid].ltv_data != NULL)
                {
                    app_gaf_free_buff(pa_info->stream_info[stream_lid].ltv_data);
                }
            }

            app_gaf_free_buff(pa_info->stream_info);

            // Subgroup ID
            uint8_t subgrp_id;

            for (subgrp_id = 0; subgrp_id < pa_info->group_info.nb_subgroups; subgrp_id++)
            {
                if (pa_info->subgroup_info[subgrp_id].val != NULL)
                {
                    app_gaf_free_buff(pa_info->subgroup_info[subgrp_id].val);
                }
            }

            app_gaf_free_buff(pa_info->subgroup_info);
        }
    }

    app_gaf_free_buff(p_app_bc_scan_env);

    p_app_bc_scan_env = NULL;

    return BT_STS_SUCCESS;
}

int app_bap_bc_scan_start(enum app_gaf_bap_bc_scan_method scan_method)
{
    if (p_app_bc_scan_env != NULL && p_app_bc_scan_env->scan_para_set_done == true)
    {
        return bap_bc_scan_start_scan(p_app_bc_scan_env->scan_timeout_s * 1000 / 10, false);
    }

    return BT_STS_NOT_READY;
}

int app_bap_bc_scan_stop(void)
{
    if (p_app_bc_scan_env != NULL)
    {
        return bap_bc_scan_stop_scan();
    }

    return BT_STS_NOT_READY;
}

int app_bap_bc_scan_set_scan_param(uint16_t scan_timeout_s, app_gaf_bap_bc_scan_param_t *scan_param)
{
    if (p_app_bc_scan_env == NULL)
    {
        return BT_STS_NOT_READY;
    }

    p_app_bc_scan_env->scan_timeout_s = scan_timeout_s;
    p_app_bc_scan_env->scan_param = *scan_param;

    return BT_STS_SUCCESS;
}

int app_bap_bc_scan_set_sync_param(app_gaf_bap_bc_scan_sync_param_t *sync_param)
{
    if (p_app_bc_scan_env == NULL)
    {
        return BT_STS_NOT_READY;
    }

    p_app_bc_scan_env->sync_param = *sync_param;

    return BT_STS_SUCCESS;
}

int app_bap_bc_scan_pa_sync_with_to(app_gaf_bap_adv_id_t *adv_id, uint16_t sync_to_10ms)
{
    if (p_app_bc_scan_env == NULL)
    {
        return BT_STS_NOT_READY;
    }

    bap_bc_pa_addr_t pa_addr = *(bap_bc_pa_addr_t *)adv_id;

    uint16_t status = bap_bc_scan_sync_pa(&pa_addr, p_app_bc_scan_env->sync_param.skip, p_app_bc_scan_env->sync_param.sync_to_10ms, sync_to_10ms);

    if (status == BT_STS_SUCCESS)
    {
        app_bap_bc_scan_set_sync_state(APP_GAF_BAP_BC_SCAN_STATE_SYNCHRONIZING);

        app_gaf_evt_report(APP_BAP_BC_SCAN_STATE_SYNCHRONIZING_IND, NULL, 0);
    }

    return status;
}

int app_bap_bc_scan_pa_sync(app_gaf_bap_adv_id_t *adv_id)
{
    return app_bap_bc_scan_pa_sync_with_to(adv_id, 0);
}

void app_bap_bc_scan_pa_sync_cancel(void)
{
    bap_bc_scan_cancel_sync_pa();
}

int app_bap_bc_scan_pa_report_ctrl(uint8_t pa_lid, uint8_t report_filter_bf)
{
    app_gaf_bap_bc_scan_pa_report_info_t *pa_info = &p_app_bc_scan_env->pa_info[pa_lid];

    return bap_bc_scan_pa_receive_enable(pa_info->pa_sync_hdl, (report_filter_bf != 0), true);
}

int app_bap_bc_scan_pa_terminate(uint8_t pa_lid)
{
    if (p_app_bc_scan_env == NULL)
    {
        return BT_STS_NOT_READY;
    }

    app_gaf_bap_bc_scan_pa_report_info_t *pa_info = &p_app_bc_scan_env->pa_info[pa_lid];

    if (pa_info->pa_sync_hdl == BAP_BC_INVALID_PA_SYNC_HDL)
    {
        return BT_STS_FAILED;
    }

    return bap_bc_scan_term_pa_sync(pa_info->pa_sync_hdl);
}

int app_bap_bc_scan_dele_pa_sync_ri(uint8_t pa_lid, bool accept)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_bap_bc_scan_dele_pa_terminate_ri(uint8_t pa_lid, bool accept)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

app_bap_bc_scan_env_t *app_bap_bc_scan_get_scan_env(void)
{
    if (p_app_bc_scan_env)
    {
        return p_app_bc_scan_env;
    }
    else
    {
        return NULL;
    }
}

#endif
#endif

/// @} APP_BAP
