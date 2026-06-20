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
#ifdef APP_BLE_BIS_SRC_ENABLE
#include "app_bap_bc_src_msg.h"
#include "ble_app_dbg.h"
#include "app_bap.h"
#include "app_bap_data_path_itf.h"

#include "app_gaf.h"
#include "app_gaf_custom_api.h"

#include "bap_broadcast_source.h"

/*INTERNAL VARIBALE*/
app_bap_bc_src_env_t *p_app_bc_src_env = NULL;
static app_bap_src_cfg_info_cb_func app_bap_srv_cfg_info_cb = NULL;

/*FUCNTIONS*/

int app_bap_bc_src_find_big_idx(uint8_t grp_lid)
{
    uint8_t big_idx = 0;

    for (big_idx = 0; big_idx < APP_BAP_DFT_BC_SRC_MAX_BIG_NUM; big_idx++)
    {
        if (p_app_bc_src_env->p_grp[big_idx].grp_lid == grp_lid)
        {
            return big_idx;
        }
    }

    return APP_GAF_INVALID_ANY_LID;
}

app_bap_bc_src_grp_info_t *app_bap_bc_src_get_big_info_by_big_idx(uint8_t big_idx)
{
    if (big_idx >= APP_BAP_DFT_BC_SRC_MAX_BIG_NUM)
    {
        return NULL;
    }

    return (&p_app_bc_src_env->p_grp[big_idx]);
}

app_bap_bc_src_grp_info_t *app_bap_bc_src_get_big_info_by_bis_hdl(uint16_t bis_hdl)
{
    uint8_t big_idx = 0;

    for (big_idx = 0; big_idx < APP_BAP_DFT_BC_SRC_MAX_BIG_NUM; big_idx++)
    {
        app_bap_bc_src_grp_info_t *p_grp = &p_app_bc_src_env->p_grp[big_idx];

        uint8_t stream_lid = 0;

        for (stream_lid = 0; stream_lid < p_grp->big_param.nb_streams; stream_lid++)
        {
            app_bap_bc_src_stream_info_t *stream = &p_grp->stream_info[stream_lid];

            if (stream->bis_hdl == bis_hdl)
            {
                return p_grp;
            }
        }
    }

    return NULL;
}

int app_bap_bc_src_find_stream_lid(uint8_t bis_hdl)
{
    uint8_t big_idx = 0;

    for (big_idx = 0; big_idx < APP_BAP_DFT_BC_SRC_MAX_BIG_NUM; big_idx++)
    {
        uint8_t stream_lid = 0;
        app_bap_bc_src_grp_info_t *p_grp = &p_app_bc_src_env->p_grp[big_idx];

        for (stream_lid = 0; stream_lid < p_grp->big_param.nb_streams; stream_lid++)
        {
            app_bap_bc_src_stream_info_t *stream = &p_grp->stream_info[stream_lid];
            if (stream->bis_hdl == bis_hdl)
            {
                return stream_lid;
            }
        }
    }

    return 0;
}

static int app_bap_bc_src_configure_grp(app_bap_bc_src_grp_info_t *p_grp)
{
    uint16_t status = BT_STS_SUCCESS;

    uint8_t subgrp_lid = 0;

    uint8_t stream_lid = 0;

    for (subgrp_lid = 0; subgrp_lid < p_grp->big_param.nb_subgroups; subgrp_lid++)
    {
        gen_aud_metadata_t *p_md = (gen_aud_metadata_t *)app_gaf_malloc_buff(
                                       sizeof(gen_aud_metadata_t) + p_grp->subgrp_info[subgrp_lid].p_metadata->add_metadata.len);

        if (p_md == NULL)
        {
            LOG_E("%s no resources!", __func__);
            return BT_STS_NO_RESOURCES;
        }

        gen_aud_init_metadata(p_md);

        p_md->basic_metadata.streaming_audio_context = p_grp->subgrp_info[subgrp_lid].p_metadata->param.context_bf;
        p_md->add_metadata.len = p_grp->subgrp_info[subgrp_lid].p_metadata->add_metadata.len;
        memcpy(p_md->add_metadata.data, p_grp->subgrp_info[subgrp_lid].p_metadata->add_metadata.data,
               p_grp->subgrp_info[subgrp_lid].p_metadata->add_metadata.len);

        status = bap_bc_src_set_subgrp_parameter(p_grp->grp_lid, subgrp_lid,
                                                 p_grp->subgrp_info[subgrp_lid].codec_id.codec_id,
                                                 (gen_aud_cc_t *)p_grp->subgrp_info[subgrp_lid].p_cfg, p_md);

        if (status != BT_STS_SUCCESS)
        {
            LOG_E("%s set subgrp param status = %d", __func__, status);
            app_gaf_free_buff(p_md);
            return status;
        }

        app_gaf_free_buff(p_md);
    }

    for (stream_lid = 0; stream_lid < p_grp->big_param.nb_streams; stream_lid++)
    {
        uint8_t subgrp_lid_belong_to = p_grp->stream_info[stream_lid].sgrp_lid;

        if (subgrp_lid_belong_to >= p_grp->big_param.nb_subgroups)
        {
            LOG_W("Invalid subgrp_lid = %d in stream info", subgrp_lid_belong_to);
            continue;
        }

        status = bap_bc_src_set_stream_parameter(p_grp->grp_lid, stream_lid,
                                                 subgrp_lid_belong_to,
                                                 (gen_aud_cc_t *)p_grp->stream_info[stream_lid].p_cfg);

        if (status != BT_STS_SUCCESS)
        {
            LOG_E("%s set stream param status = %d", __func__, status);
            return status;
        }
    }

    bap_bc_src_pa_param_t *p_pa_adv_param = (bap_bc_src_pa_param_t *)
                                            app_gaf_malloc_buff(sizeof(bap_bc_src_pa_param_t) + p_grp->big_param.per_adv_data_len);

    if (p_pa_adv_param == NULL)
    {
        LOG_E("%s no resources!!", __func__);
        return BT_STS_NO_RESOURCES;
    }

    p_pa_adv_param->pa_interval = p_grp->big_param.per_adv_param.adv_intv_min_frame;
    p_pa_adv_param->include_tx_pwr = true;
    p_pa_adv_param->add_pa_data.len = p_grp->big_param.per_adv_data_len;
    memcpy(p_pa_adv_param->add_pa_data.data, p_grp->big_param.per_adv_data, p_grp->big_param.per_adv_data_len);

    bap_big_param_t *p_big_param = (bap_big_param_t *)app_gaf_malloc_buff(sizeof(bap_big_param_t));

    if (p_big_param == NULL)
    {
        LOG_E("%s no resources!!!", __func__);
        return BT_STS_NO_RESOURCES;
    }

    memset(p_big_param, 0, sizeof(bap_big_param_t));

    p_big_param->bis_count = p_grp->big_param.nb_streams;
    p_big_param->encryption = p_grp->big_param.encrypted;
    p_big_param->framing = p_grp->big_param.grp_param.framing;
    p_big_param->max_sdu_size = p_grp->big_param.grp_param.max_sdu;
    p_big_param->max_transport_latency_us = p_grp->big_param.grp_param.max_tlatency_ms;
    p_big_param->packing = p_grp->big_param.grp_param.packing;
    p_big_param->rtn = p_grp->big_param.grp_param.rtn;
    p_big_param->phy_bits = p_grp->big_param.grp_param.phy_bf;
    p_big_param->sdu_interval_us = p_grp->big_param.grp_param.sdu_intv_us;
    p_big_param->test_bn = p_grp->big_param.grp_param.bn;
    p_big_param->test_irc = p_grp->big_param.grp_param.irc;
    p_big_param->test_iso_interval_1_25ms = p_grp->big_param.grp_param.iso_intv_frame;
    p_big_param->test_max_pdu_size = p_grp->big_param.grp_param.max_pdu;
    p_big_param->test_nse = p_grp->big_param.grp_param.nse;
    p_big_param->test_pto = p_grp->big_param.grp_param.pto;

    if (p_grp->big_param.encrypted == true)
    {
        memcpy(p_big_param->broadcast_code,
               p_grp->big_param.bcast_code.bcast_code,
               sizeof(p_grp->big_param.bcast_code));
    }

    status = bap_bc_src_configure_group(p_grp->grp_lid,
                                        p_grp->big_param.pres_delay_us,
                                        p_pa_adv_param,
                                        p_big_param);

    app_gaf_free_buff(p_pa_adv_param);
    app_gaf_free_buff(p_big_param);

    return status;
}

/*CALLBACKS*/
static void app_bap_bc_src_cb_grp_state(uint8_t grp_lid, uint8_t state)
{
    uint8_t big_idx = app_bap_bc_src_find_big_idx(grp_lid);

    if (big_idx == APP_GAF_INVALID_ANY_LID)
    {
        LOG_E("app_bap bc_src can not found, grp_lid = %d", grp_lid);
        return;
    }

    app_bap_bc_src_grp_info_t *p_grp = app_bap_bc_src_get_big_info_by_big_idx(big_idx);

    if (p_grp == NULL)
    {
        LOG_E("app_bap bc_src can not found, big_idx = %d", big_idx);
        return;
    }

    switch (state)
    {
        case APP_GAF_BAP_BC_SRC_STATE_IDLE:
        {
            if (p_grp->big_state != APP_GAF_BAP_BC_SRC_STATE_IDLE)
            {
                LOG_I("app_bap bc release cmp, auto start grp delete");
                p_grp->big_state = APP_GAF_BAP_BC_SRC_STATE_IDLE;

                bap_bc_src_delete_group(grp_lid);
            }
            else
            {
                LOG_I("app_bap bc_src create grp success, big_idx = %d", big_idx);

                // Configure Source Group
                uint16_t status = app_bap_bc_src_configure_grp(p_grp);

                LOG_I("app_bap bc_src configure grp, big_idx = %d, status = %d", big_idx, status);
            }
        }
        break;
        case APP_GAF_BAP_BC_SRC_STATE_CONFIGURED:
        {
            uint8_t old_state = p_grp->big_state;

            p_grp->big_state = APP_GAF_BAP_BC_SRC_STATE_CONFIGURED;

            if (old_state == APP_GAF_BAP_BC_SRC_STATE_IDLE)
            {
                LOG_I("app_bap bc src enable grp cmp");
                app_gaf_evt_report(APP_GAF_SRC_BIS_SRC_ENABLED_IND, (void *)p_grp, sizeof(app_bap_bc_src_grp_info_t));
            }
            else if (old_state == APP_GAF_BAP_BC_SRC_STATE_STREAMING)
            {
                LOG_I("app_bap bc src disable grp cmp");
                app_gaf_evt_report(APP_GAF_SRC_BIS_SRC_DISABLED_IND, (void *)p_grp, sizeof(app_bap_bc_src_grp_info_t));
            }
            else
            {
                LOG_I("app_bap bc src reconfigure cmp");
            }
        }
        break;
        case APP_GAF_BAP_BC_SRC_STATE_STREAMING:
        {
            LOG_I("app_bap bc src start streaming cmp");
            p_grp->big_state = APP_GAF_BAP_BC_SRC_STATE_STREAMING;
        }
        break;
        default:
            break;
    }
}

static void app_bap_bc_src_cb_big_create_state(uint8_t grp_lid, const bap_big_opened_t *p_big_info)
{
    uint8_t big_idx = app_bap_bc_src_find_big_idx(grp_lid);

    if (big_idx == APP_GAF_INVALID_ANY_LID)
    {
        LOG_E("%s can not found, grp_lid = %d", __func__, grp_lid);
        return;
    }

    app_bap_bc_src_grp_info_t *p_grp = app_bap_bc_src_get_big_info_by_big_idx(big_idx);
    if (p_big_info->err_code == BT_STS_SUCCESS)
    {
        p_grp->grp_cmp.nb_bis        = p_big_info->bis_count;
        p_grp->grp_cmp.sync_delay_us = p_big_info->timing->big_sync_delay_us;
        p_grp->grp_cmp.tlatency_us   = p_big_info->timing->transport_latency_big_us;
        p_grp->grp_cmp.iso_interval_frames   = p_big_info->timing->iso_interval_1_25ms;
        p_grp->grp_cmp.nse   = p_big_info->timing->nse;
        p_grp->grp_cmp.bn   = p_big_info->timing->bn;
        p_grp->grp_cmp.pto   = p_big_info->timing->pto;
        p_grp->grp_cmp.irc   = p_big_info->timing->irc;
        p_grp->grp_cmp.max_pdu   = p_big_info->timing->max_pdu_size;
        for (int i = 0; i < p_big_info->bis_count; ++i)
        {
            p_grp->grp_cmp.conhdl[i] = p_big_info->stream[i]->connhdl;
        }
    }

    if (p_grp == NULL)
    {
        LOG_E("%s can not found, big_idx = %d", __func__, big_idx);
        return;
    }

    app_gaf_evt_report(APP_GAF_SRC_BIS_PA_ENABLED_IND, (void *)p_grp, sizeof(app_bap_bc_src_grp_info_t));
}

static void app_bap_bc_src_cb_big_term(uint8_t grp_lid, uint8_t big_id, uint16_t err_code)
{
    LOG_I("%s big terminated, reason = %d, grp_lid = %d, big_id = %d",
          __func__, err_code, grp_lid, big_id);

    uint8_t big_idx = app_bap_bc_src_find_big_idx(grp_lid);

    if (big_idx == APP_GAF_INVALID_ANY_LID)
    {
        LOG_E("%s can not found, grp_lid = %d", __func__, grp_lid);
        return;
    }

    app_bap_bc_src_grp_info_t *p_grp = app_bap_bc_src_get_big_info_by_big_idx(big_idx);

    if (p_grp == NULL)
    {
        LOG_E("%s can not found, big_idx = %d", __func__, big_idx);
        return;
    }

    app_gaf_evt_report(APP_GAF_SRC_BIS_PA_DISABLED_IND, (void *)p_grp, sizeof(app_bap_bc_src_grp_info_t));
}

static void app_bap_bc_src_cb_iso_dp_state(bool is_create, uint8_t grp_lid, uint8_t big_id, uint8_t bis_idx, uint16_t iso_hdl, uint16_t err_code)
{
    uint8_t big_idx = app_bap_bc_src_find_big_idx(grp_lid);

    if (big_idx == APP_GAF_INVALID_ANY_LID)
    {
        LOG_E("%s can not found, grp_lid = %d", __func__, grp_lid);
        return;
    }

    app_bap_bc_src_grp_info_t *p_grp = app_bap_bc_src_get_big_info_by_big_idx(big_idx);

    if (p_grp == NULL)
    {
        LOG_E("%s can not found, big_idx = %d", __func__, big_idx);
        return;
    }

    if (bis_idx > p_grp->big_param.nb_streams)
    {
        LOG_E("%s invalid bis index, bis_idx = %d", __func__, bis_idx);
        return;
    }

    app_bap_bc_src_stream_msg_t stream_msg;

    app_bap_bc_src_stream_info_t *p_stream = &p_grp->stream_info[bis_idx - 1];

    stream_msg.stream_lid = bis_idx - 1;

    if (is_create == true)
    {
        p_stream->bis_hdl = iso_hdl;
        stream_msg.bis_hdl = p_stream->bis_hdl;
        app_gaf_evt_report(APP_GAF_SRC_BIS_STREAM_STARTED_IND, (void *)&stream_msg, sizeof(app_bap_bc_src_stream_msg_t));
    }
    else
    {
        stream_msg.bis_hdl = p_stream->bis_hdl;
        app_gaf_evt_report(APP_GAF_SRC_BIS_STREAM_STOPPED_IND, (void *)&stream_msg, sizeof(app_bap_bc_src_stream_msg_t));
    }
}

static const bap_bc_src_evt_cb_t app_bap_bc_src_callback =
{
    .cb_src_grp_state = app_bap_bc_src_cb_grp_state,
    .cb_big_create_state = app_bap_bc_src_cb_big_create_state,
    .cb_bis_iso_dp_state = app_bap_bc_src_cb_iso_dp_state,
    .cb_big_term = app_bap_bc_src_cb_big_term,
};

int app_bap_bc_src_init(void)
{
    LOG_I("%s", __func__);

    bap_bc_src_init_cfg_t init_cfg =
    {

    };

    return bap_bc_src_init(&init_cfg, &app_bap_bc_src_callback);
}

static bool app_bap_bc_src_is_any_state_used(void)
{
    uint8_t big_idx = 0;

    app_bap_bc_src_grp_info_t *p_grp = NULL;

    while (big_idx < ARRAY_SIZE(p_app_bc_src_env->p_grp))
    {
        p_grp = &p_app_bc_src_env->p_grp[big_idx];

        if (p_grp->big_state != BAP_BC_SRC_STATE_IDLE)
        {
            LOG_E("%s big_idx %d state is not idle", __func__, big_idx);
            return true;
        }

        big_idx++;
    }

    return false;
}

int app_bap_bc_src_deinit(void)
{
    LOG_I("%s", __func__);

    if (p_app_bc_src_env == NULL)
    {
        return BT_STS_SUCCESS;
    }

    return app_bap_bc_src_is_any_state_used() ? BT_STS_FAILED : bap_bc_src_deinit();
}

int app_bap_bc_src_tx_stream_start(uint16_t bis_hdl)
{
    uint8_t status = BT_STS_SUCCESS;
    const struct data_path_itf *_tx_dp_itf = NULL;

    _tx_dp_itf = data_path_itf_get(ISO_DP_ISOOHCI, ISO_SEL_TX);
    if (!_tx_dp_itf)
    {
        LOG_W("app_bap bc src get tx data path interface failed");
        _tx_dp_itf = data_path_itf_get(ISO_DP_DISABLE, ISO_SEL_TX);
    }
    app_bap_set_tx_dp_itf((struct data_path_itf *)_tx_dp_itf);

    if (_tx_dp_itf && (NULL != _tx_dp_itf->cb_start))
    {
        app_bap_bc_src_grp_info_t *p_grp = app_bap_bc_src_get_big_info_by_bis_hdl(bis_hdl);
        if (NULL == p_grp)
        {
            LOG_W("app_bap bc src cannot get big_info by bis_hdl");
            return status;
        }

        status = _tx_dp_itf->cb_start(bis_hdl, p_grp->big_param.grp_param.sdu_intv_us,
                                      2 * p_grp->big_param.grp_param.max_tlatency_ms, p_grp->big_param.grp_param.max_sdu);
    }

    return status;
}

int app_bap_bc_src_tx_stream_stop(uint16_t bis_hdl)
{
    if (BLE_IS_BISHDL(bis_hdl))
    {
        struct data_path_itf *_tx_dp_itf = app_bap_get_tx_dp_itf();
        LOG_I("app_bap bc src stop tx stream, bis_hdl = %d", bis_hdl);
        if (_tx_dp_itf && (NULL != _tx_dp_itf->cb_stop))
        {
            _tx_dp_itf->cb_stop(bis_hdl, 0);
        }
    }

    return BT_STS_SUCCESS;
}

static void app_bap_bc_src_send_iso_data(uint16_t bis_hdl, uint16_t seq_num, uint8_t *payload, uint16_t payload_len, uint32_t bt_ref_time)
{
    app_bap_dp_itf_send_data_directly(bis_hdl, seq_num, payload, payload_len, bt_ref_time);
}

int app_bap_bc_src_iso_send_data_to_all_channel(uint8_t **payload, uint16_t payload_len, uint32_t ref_time)
{
    uint8_t big_idx = 0;

    for (big_idx = 0; big_idx < APP_BAP_DFT_BC_SRC_MAX_BIG_NUM; big_idx++)
    {
        app_bap_bc_src_grp_info_t *p_grp = &p_app_bc_src_env->p_grp[big_idx];
        uint8_t stream_lid = 0;

        for (stream_lid = 0; stream_lid < p_grp->big_param.nb_streams; stream_lid++)
        {
            app_bap_bc_src_stream_info_t *p_stream = &p_grp->stream_info[stream_lid];

            if (APP_GAF_INVALID_CON_HDL != p_stream->bis_hdl)
            {
                app_bap_bc_src_send_iso_data(p_stream->bis_hdl, 0, payload[stream_lid], payload_len, ref_time);
            }
        }
    }

    return 0;
}

int app_bap_bc_src_register_cfg_info_cb(app_bap_src_cfg_info_cb_func cb)
{
    app_bap_srv_cfg_info_cb = cb;
    return 0;
}

static uint8_t app_bap_bc_src_codec_cfg_init(app_gaf_codec_id_t *codec_id, app_gaf_bap_cfg_t **pp_cfg)
{
    uint8_t cfg_len = sizeof(app_gaf_bap_cfg_t) + APP_BAP_DFT_BC_SRC_ADD_CODEC_LEN;
    app_gaf_bap_cfg_t *p_cfg = NULL;
    *pp_cfg = (app_gaf_bap_cfg_t *)app_gaf_malloc_buff(cfg_len);
    p_cfg = *pp_cfg;
    if (NULL == p_cfg)
    {
        LOG_W("app_bap bc src codec cfg init malloc failed");
        return 0;
    }
    memset(p_cfg, 0, cfg_len);

    if (true == app_bap_codec_is_lc3(codec_id))
    {
        p_cfg->param.location_bf = APP_BAP_DFT_BC_SRC_LOCATION_BF;
        p_cfg->param.frame_octet = APP_BAP_DFT_BC_SRC_FRAME_OCTET;
        p_cfg->param.sampling_freq = APP_BAP_DFT_BC_SRC_SAMPLING_FREQ;
        p_cfg->param.frame_dur = APP_BAP_DFT_BC_SRC_FRAME_DURATION;
        p_cfg->param.frames_sdu = APP_BAP_DFT_BC_SRC_NB_LC3_STREAM;
        p_cfg->add_cfg.len = APP_BAP_DFT_BC_SRC_ADD_CODEC_LEN;
    }

    if (p_cfg->add_cfg.len != 0)
    {
        /// TODO:Do not use
        app_bap_add_data_set(&p_cfg->add_cfg.data[0], p_cfg->add_cfg.len);
    }

    return cfg_len;
}

static uint8_t app_bap_bc_src_codec_cfg_set(app_gaf_bap_cfg_t **pp_cfg_out, app_gaf_bap_cfg_t *p_cfg_in)
{
    if ((NULL == pp_cfg_out) || (NULL == p_cfg_in))
    {
        return 0;
    }

    if (NULL != (*pp_cfg_out))
    {
        app_gaf_free_buff(*pp_cfg_out);
    }

    uint8_t cfg_len = sizeof(app_gaf_bap_cfg_t) + p_cfg_in->add_cfg.len;
    app_gaf_bap_cfg_t *p_cfg_out = NULL;
    *pp_cfg_out = (app_gaf_bap_cfg_t *)app_gaf_malloc_buff(cfg_len);
    p_cfg_out = *pp_cfg_out;
    if (NULL == p_cfg_out)
    {
        LOG_W("app_bap bc src codec cfg set malloc failed");
        return 0;
    }
    memset(p_cfg_out, 0, cfg_len);
    memcpy(p_cfg_out, p_cfg_in, cfg_len);
    return cfg_len;
}

static uint8_t app_bap_bc_src_metadata_cfg_init(app_gaf_bap_cfg_metadata_t **pp_metadata)
{
    uint8_t metadata_len = sizeof(app_gaf_bap_cfg_metadata_t) + APP_BAP_DFT_BC_SRC_METADATA_LEN;
    app_gaf_bap_cfg_metadata_t *p_metadata = NULL;
    *pp_metadata = (app_gaf_bap_cfg_metadata_t *)app_gaf_malloc_buff(metadata_len);
    p_metadata = *pp_metadata;
    if (NULL == p_metadata)
    {
        LOG_W("app_bap bc src metadata cfg init malloc failed");
        return 0;
    }

    memset(p_metadata, 0, metadata_len);
    p_metadata->param.context_bf = APP_BAP_DFT_BC_SRC_CONTEXT_BF;
    p_metadata->add_metadata.len = APP_BAP_DFT_BC_SRC_METADATA_LEN;
    app_bap_add_data_set(&p_metadata->add_metadata.data[0], p_metadata->add_metadata.len);
    return metadata_len;
}

static uint16_t app_bap_bc_src_base_info_init(app_bap_bc_src_grp_info_t *p_grp, app_bap_bc_src_cfg_info_t *p_init_cfg)
{
    p_grp->big_param.nb_streams = p_init_cfg->nb_streams;
    p_grp->big_param.nb_subgroups = p_init_cfg->nb_subgroups;

    // Init grp_params
    p_grp->big_param.grp_param.sdu_intv_us = p_init_cfg->sdu_intv_us;
    p_grp->big_param.grp_param.max_sdu = p_init_cfg->max_sdu;
    p_grp->big_param.grp_param.max_tlatency_ms = p_init_cfg->max_tlatency;

    // Init adv_params
    p_grp->big_param.adv_param.adv_intv_min_slot = p_init_cfg->adv_intv_min_slot;
    p_grp->big_param.adv_param.adv_intv_max_slot = p_init_cfg->adv_intv_max_slot;

    // Init adv_params
    p_grp->big_param.per_adv_param.adv_intv_min_frame = p_init_cfg->adv_intv_min_frame;
    p_grp->big_param.per_adv_param.adv_intv_max_frame = p_init_cfg->adv_intv_max_frame;

    p_grp->grp_lid = APP_GAF_INVALID_ANY_LID;
    p_grp->big_state = BAP_BC_SRC_STATE_IDLE;

    // Init grp_params
    p_grp->big_param.grp_param.packing = APP_BAP_DFT_BC_SRC_PACKING_TYPE;
    p_grp->big_param.grp_param.framing = APP_BAP_DFT_BC_SRC_FRAMING_TYPE;
    p_grp->big_param.grp_param.phy_bf = APP_BAP_DFT_BC_SRC_PHY_BF;
    p_grp->big_param.grp_param.test   = APP_BAP_DFT_BC_SRC_TEST;
    p_grp->big_param.grp_param.rtn = APP_BAP_DFT_BC_SRC_RTN;

    // Init adv_params
    p_grp->big_param.adv_param.chnl_map = ADV_ALL_CHNLS_EN;
    p_grp->big_param.adv_param.phy_prim = APP_PHY_1MBPS_BIT;
    p_grp->big_param.adv_param.phy_second = APP_PHY_1MBPS_BIT;
    p_grp->big_param.adv_param.adv_sid = 1;

    p_grp->big_param.pres_delay_us = APP_BAP_DFT_BC_SRC_PRES_DELAY_US;

    memcpy(&p_grp->big_param.bcast_id.id, &p_init_cfg->bcast_id.id, sizeof(p_init_cfg->bcast_id));

    // Init BIS Encrypt
    p_grp->big_param.encrypted = p_init_cfg->encrypted;

    if (0 != p_grp->big_param.encrypted)
    {
        memcpy(&p_grp->big_param.bcast_code, &p_init_cfg->bcast_code, sizeof(p_init_cfg->bcast_code));
    }

    // Init subgroups buffer
    uint16_t size = p_grp->big_param.nb_subgroups * sizeof(app_bap_bc_src_subgrp_info_t);

    if (p_grp->subgrp_info != NULL)
    {
        app_gaf_free_buff(p_grp->subgrp_info);
    }

    p_grp->subgrp_info = (app_bap_bc_src_subgrp_info_t *)app_gaf_malloc_buff(size);

    if (NULL == p_grp->subgrp_info)
    {
        LOG_W("app_bap bc src sgrp init malloc failed");
        return BT_STS_NO_RESOURCES;
    }

    memset(p_grp->subgrp_info, 0, size);

    // Init subgroups
    for (uint8_t sgrp_lid = 0; sgrp_lid < p_grp->big_param.nb_subgroups; sgrp_lid++)
    {
        app_bap_bc_src_subgrp_info_t *subgrp = &p_grp->subgrp_info[sgrp_lid];
        subgrp->sgrp_lid = sgrp_lid;
        subgrp->codec_id.codec_id[0] = APP_BAP_DFT_BC_SRC_CODEC_ID;
        // Init Subgrp Codec
        subgrp->cfg_len = app_bap_bc_src_codec_cfg_init(&subgrp->codec_id, &subgrp->p_cfg);

        if (0 == subgrp->cfg_len)
        {
            LOG_W("app_bap bc src subgrp codec cfg init malloc failed");
            return BT_STS_NO_RESOURCES;
        }

        // Init Subgrp Metadata
        subgrp->metadata_len = app_bap_bc_src_metadata_cfg_init(&subgrp->p_metadata);
        if (0 == subgrp->metadata_len)
        {
            LOG_W("app_bap bc src subgrp metadata cfg init malloc failed");
            return BT_STS_NO_RESOURCES;
        }
    }

    // Init Stream buffer
    size = p_grp->big_param.nb_streams * sizeof(app_bap_bc_src_stream_info_t);

    if (p_grp->stream_info != NULL)
    {
        app_gaf_free_buff(p_grp->stream_info);
    }

    p_grp->stream_info = (app_bap_bc_src_stream_info_t *)app_gaf_malloc_buff(size);

    if (NULL == p_grp->stream_info)
    {
        LOG_W("app_bap bc src streams init malloc failed");
        return BT_STS_NO_RESOURCES;
    }

    memset(p_grp->stream_info, 0, size);

    for (uint8_t stream_lid = 0; stream_lid < p_grp->big_param.nb_streams; stream_lid++)
    {
        app_bap_bc_src_stream_info_t *stream = &p_grp->stream_info[stream_lid];
        stream->stream_lid = stream_lid;

        //Divide stream to subgroup
        stream->sgrp_lid = stream_lid;
        stream->dp_cfg.dp_id = APP_BAP_DFT_BC_SRC_DP_ID;
        stream->dp_cfg.ctl_delay_us = APP_BAP_DFT_BC_SRC_CTL_DELAY_US;
        //Init stream Codec
        app_gaf_codec_id_t codec_id = {0};
        codec_id.codec_id[0] = APP_BAP_DFT_BC_SRC_CODEC_ID;
        stream->cfg_len = app_bap_bc_src_codec_cfg_init(&codec_id, &stream->p_cfg);
        if (0 == stream->cfg_len)
        {
            LOG_W("app_bap bc src stream codec init malloc failed");
            return BT_STS_NO_RESOURCES;
        }

        stream->bis_hdl = GATT_INVALID_HDL;
    }

    return BT_STS_SUCCESS;
}

int app_bap_bc_src_info_init(void)
{
    LOG_I("app_bap bc src info init");

    if (p_app_bc_src_env != NULL)
    {
        return BT_STS_SUCCESS;
    }

    uint16_t status = BT_STS_SUCCESS;

    app_bap_bc_src_cfg_info_t grp_cfg_info = {0};
    bool cfg_by_app = false;

    uint16_t size = sizeof(app_bap_bc_src_env_t);

    p_app_bc_src_env = (app_bap_bc_src_env_t *)app_gaf_malloc_buff(size);

    if (NULL == p_app_bc_src_env)
    {
        LOG_W("app_bap bc src env init malloc failed");
        return BT_STS_NO_RESOURCES;
    }

    memset(p_app_bc_src_env, 0, size);

    if (app_bap_srv_cfg_info_cb)
    {
        cfg_by_app = app_bap_srv_cfg_info_cb(&grp_cfg_info);
    }

    uint8_t big_idx = 0;

    for (big_idx = 0; big_idx < APP_BAP_DFT_BC_SRC_MAX_BIG_NUM; big_idx++)
    {
        app_bap_bc_src_grp_info_t *p_grp = &p_app_bc_src_env->p_grp[big_idx];

        if (cfg_by_app == false)
        {
            grp_cfg_info.nb_streams = APP_BAP_DFT_BC_SRC_NB_STREAMS;
            grp_cfg_info.nb_subgroups = APP_BAP_DFT_BC_SRC_NB_SUBGRPS;

            grp_cfg_info.sdu_intv_us = APP_BAP_DFT_BC_SRC_SDU_INTV_US;
            grp_cfg_info.max_sdu = APP_BAP_DFT_BC_SRC_MAX_SDU_SIZE;
            grp_cfg_info.max_tlatency = APP_BAP_DFT_BC_SRC_MAX_TRANS_LATENCY_MS;

            grp_cfg_info.adv_intv_min_slot = APP_BAP_DFT_BC_SRC_ADV_INTERVAL;
            grp_cfg_info.adv_intv_max_slot = APP_BAP_DFT_BC_SRC_ADV_INTERVAL;

            grp_cfg_info.adv_intv_min_frame = APP_BAP_DFT_BC_SRC_PERIODIC_INTERVAL;
            grp_cfg_info.adv_intv_max_frame = APP_BAP_DFT_BC_SRC_PERIODIC_INTERVAL;

            grp_cfg_info.encrypted = APP_BAP_DFT_BC_SRC_IS_ENCRYPTED;

            if (0 != APP_BAP_DFT_BC_SRC_IS_ENCRYPTED)
            {
                app_bap_add_data_set(grp_cfg_info.bcast_code.bcast_code, APP_GAP_KEY_LEN);
            }
        }

        status = app_bap_bc_src_base_info_init(p_grp, &grp_cfg_info);

        if (status != BT_STS_SUCCESS)
        {
            break;
        }
    }

    return status;
}

int app_bap_bc_src_info_deinit(void)
{
    LOG_I("app_bap bc src info deinit");

    if (p_app_bc_src_env == NULL)
    {
        return BT_STS_SUCCESS;
    }

    if (app_bap_bc_src_is_any_state_used())
    {
        return BT_STS_FAILED;
    }

    uint8_t big_idx = 0;

    app_bap_bc_src_grp_info_t *p_grp = NULL;

    while (big_idx < ARRAY_SIZE(p_app_bc_src_env->p_grp))
    {
        p_grp = &p_app_bc_src_env->p_grp[big_idx];

        if (p_grp->big_param.adv_data != NULL)
        {
            app_gaf_free_buff(p_grp->big_param.adv_data);
        }

        if (p_grp->big_param.per_adv_data != NULL)
        {
            app_gaf_free_buff(p_grp->big_param.per_adv_data);
        }

        if (p_grp->stream_info != NULL)
        {
            uint8_t stream_lid = 0;

            for (stream_lid = 0; stream_lid < p_grp->big_param.nb_streams; stream_lid++)
            {
                if (p_grp->stream_info[stream_lid].p_cfg != NULL)
                {
                    app_gaf_free_buff(p_grp->stream_info[stream_lid].p_cfg);
                }
            }

            app_gaf_free_buff(p_grp->stream_info);
        }

        if (p_grp->subgrp_info != NULL)
        {
            uint8_t subgrp_lid = 0;

            for (subgrp_lid = 0; subgrp_lid < p_grp->big_param.nb_subgroups; subgrp_lid++)
            {
                if (p_grp->subgrp_info[subgrp_lid].p_cfg != NULL)
                {
                    app_gaf_free_buff(p_grp->subgrp_info[subgrp_lid].p_cfg);
                }

                if (p_grp->subgrp_info[subgrp_lid].p_metadata != NULL)
                {
                    app_gaf_free_buff(p_grp->subgrp_info[subgrp_lid].p_metadata);
                }
            }

            app_gaf_free_buff(p_grp->subgrp_info);
        }

        big_idx++;
    }

    app_gaf_free_buff(p_app_bc_src_env);

    p_app_bc_src_env = NULL;

    return BT_STS_SUCCESS;
}

int app_bap_bc_src_update_grp_info(uint8_t big_idx, uint8_t nb_subgrp, uint8_t nb_stream, uint32_t sdu_itvl, uint16_t max_sdu)
{
    app_bap_bc_src_cfg_info_t init_cfg =
    {
        .nb_streams = nb_stream,
        .nb_subgroups = nb_subgrp,

        .sdu_intv_us = sdu_itvl,
        .max_sdu = max_sdu,
        .max_tlatency = APP_BAP_DFT_BC_SRC_MAX_TRANS_LATENCY_MS,

        .adv_intv_min_slot = APP_BAP_DFT_BC_SRC_ADV_INTERVAL,
        .adv_intv_max_slot = APP_BAP_DFT_BC_SRC_ADV_INTERVAL,

        .adv_intv_min_frame = APP_BAP_DFT_BC_SRC_PERIODIC_INTERVAL,
        .adv_intv_max_frame = APP_BAP_DFT_BC_SRC_PERIODIC_INTERVAL,
    };

    app_bap_bc_src_grp_info_t *p_grp = &p_app_bc_src_env->p_grp[big_idx];

    if (p_grp == NULL)
    {
        return BT_STS_NOT_FOUND;
    }

    return app_bap_bc_src_base_info_init(p_grp, &init_cfg);
}

static bool app_bap_bc_src_get_ad_ltv(const uint8_t *p_data_ltv, uint8_t total_len, uint8_t ad_type, const uint8_t **pp_ltv_get)
{
    const uint8_t *p_end = p_data_ltv + total_len;

    while (p_end > p_data_ltv)
    {
        if (p_data_ltv[1] == ad_type)
        {
            if (pp_ltv_get != NULL)
            {
                *pp_ltv_get = p_data_ltv;
            }

            return true;
        }

        p_data_ltv += p_data_ltv[0] + 1;
    }

    return false;
}

int app_bap_bc_src_start(uint8_t big_idx)
{
    if (big_idx >= APP_BAP_DFT_BC_SRC_MAX_BIG_NUM)
    {
        LOG_W("app_bap bc src big_idx error %d", big_idx);
        return BT_STS_INVALID_PARM;
    }

    app_bap_bc_src_grp_info_t *p_grp = &p_app_bc_src_env->p_grp[big_idx];

    if (p_grp->big_state != APP_GAF_BAP_BC_SRC_STATE_IDLE)
    {
        LOG_W("app_bap bc src big has been used, start error");
        return BT_STS_NOT_ALLOW;
    }

    if ((p_grp->big_param.nb_streams > APP_BAP_DFT_BC_SRC_MAX_STREAM_NUM)
            || (p_grp->big_param.nb_subgroups > APP_BAP_DFT_BC_SRC_MAX_SUBGRP_NUM))
    {
        LOG_D("app_bap bc src add big failed, exceeds the maximum");
        return BT_STS_INVALID_PARM;
    }

    pbp_pba_info_t *p_pba_info = (pbp_pba_info_t *)app_gaf_malloc_buff(sizeof(pbp_pba_info_t));

    gen_aud_var_info_t *p_ext_add_adv_data =
        (gen_aud_var_info_t *)app_gaf_malloc_buff(sizeof(gen_aud_var_info_t) + p_grp->big_param.adv_data_len);

    gap_adv_param_t *p_ext_adv_param = (gap_adv_param_t *)app_gaf_malloc_buff(sizeof(gap_adv_param_t));

    // PBA broadcaster name
    pbp_broadcast_name_ptr_t pba_name = {0};

    if (p_pba_info == NULL || p_ext_add_adv_data == NULL || p_ext_adv_param == NULL)
    {
        LOG_E("app_bap bc src no resources");
        return BT_STS_NO_RESOURCES;
    }

    // PBA broadcaster name
    if (!app_bap_bc_src_get_ad_ltv(p_grp->big_param.adv_data, p_grp->big_param.adv_data_len, GAP_DT_BROADCAST_NAME, NULL))
    {
        pba_name.name_length = sizeof(APP_BAP_DFT_BC_SRC_ADV_DATA);
        pba_name.p_name = (uint8_t *)APP_BAP_DFT_BC_SRC_ADV_DATA;
    }

    /// TODO:
    // PBA info
    p_pba_info->pba_features = PBA_FEAT_STAND_QUAL_AUD_CFG_PRESENT_BIT | PBA_FEAT_HIGH_QUAL_AUD_CFG_PRESENT_BIT;

    if (p_grp->big_param.encrypted == true)
    {
        p_pba_info->pba_features |= PBA_FEAT_BC_STREAMS_ENCRYPTED_BIT;
    }

    /// TODO: now we do not have metadata
    gen_aud_init_metadata(&p_pba_info->metadata);

    // Extend adv param
    memset(p_ext_adv_param, 0, sizeof(gap_adv_param_t));

    p_ext_adv_param->adv_channel_map = p_grp->big_param.adv_param.chnl_map;
    p_ext_adv_param->has_custom_adv_timing = true;
    p_ext_adv_param->start_bg_advertising = true;
    p_ext_adv_param->adv_timing.min_adv_bg_interval_ms = p_grp->big_param.adv_param.adv_intv_min_slot;
    p_ext_adv_param->adv_timing.max_adv_bg_interval_ms = p_grp->big_param.adv_param.adv_intv_max_slot;
    p_ext_adv_param->primary_adv_phy = p_grp->big_param.adv_param.phy_prim;
    p_ext_adv_param->secondary_adv_phy = p_grp->big_param.adv_param.phy_second;
    // Extend adv data
    p_ext_add_adv_data->len = p_grp->big_param.adv_data_len;
    memcpy(p_ext_add_adv_data->data, p_grp->big_param.adv_data, p_grp->big_param.adv_data_len);
    // Lock call
    uint16_t status = bap_bc_src_create_group(p_grp->big_param.bcast_id.id,
                                              p_grp->big_param.nb_subgroups,
                                              p_grp->big_param.nb_streams,
                                              &pba_name,
                                              p_pba_info,
                                              p_ext_adv_param,
                                              p_ext_add_adv_data,
                                              &p_grp->grp_lid);

    app_gaf_free_buff(p_ext_add_adv_data);
    app_gaf_free_buff(p_ext_adv_param);
    app_gaf_free_buff(p_pba_info);

    LOG_I("%s status = %d", __func__, status);

    return status;
}

int app_bap_bc_src_stop(uint8_t big_idx)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

app_bap_bc_src_grp_info_t *app_bap_bc_src_get_big_info(uint8_t big_idx)
{
    if (big_idx >= APP_BAP_DFT_BC_SRC_MAX_BIG_NUM)
    {
        LOG_D("app_bap bc src invalid big_idx");
        return NULL;
    }

    return  &p_app_bc_src_env->p_grp[big_idx];
}

app_bap_bc_src_stream_info_t *app_bap_bc_src_get_bis_stream_info(uint8_t big_idx, uint8_t bis_index)
{
    if (big_idx >= APP_BAP_DFT_BC_SRC_MAX_BIG_NUM)
    {
        LOG_D("app_bap bc src invalid big_idx");
        return NULL;
    }

    app_bap_bc_src_grp_info_t *p_grp = &p_app_bc_src_env->p_grp[big_idx];

    if (bis_index > p_grp->big_param.nb_streams)
    {
        LOG_D("app_bap bc src invalid bis_idx = %d", bis_index);
        return NULL;
    }

    return &p_grp->stream_info[bis_index];
}

int app_bap_bc_src_set_big_params(uint8_t big_idx, app_bap_bc_src_grp_param_t *grp_param)
{
    app_bap_bc_src_grp_info_t *p_grp = &p_app_bc_src_env->p_grp[big_idx];

    if (big_idx >= APP_BAP_DFT_BC_SRC_MAX_BIG_NUM)
    {
        LOG_D("app_bap bc src invalid big_idx");
        return BT_STS_INVALID_PARM;
    }

    if (p_grp->big_param.adv_data)
    {
        app_gaf_free_buff(p_grp->big_param.adv_data);
    }

    if (p_grp->big_param.per_adv_data)
    {
        app_gaf_free_buff(p_grp->big_param.per_adv_data);
    }

    if (p_grp->subgrp_info)
    {
        for (int i = 0; i < p_grp->big_param.nb_subgroups; ++i)
        {
            app_gaf_free_buff(p_grp->subgrp_info[i].p_cfg);
            app_gaf_free_buff(p_grp->subgrp_info[i].p_metadata);
        }

        app_gaf_free_buff(p_grp->subgrp_info);
    }

    if (p_grp->stream_info)
    {
        for (int i = 0; i < p_grp->big_param.nb_subgroups; ++i)
        {
            app_gaf_free_buff(p_grp->stream_info[i].p_cfg);
        }

        app_gaf_free_buff(p_grp->stream_info);
    }

    memcpy(&p_grp->big_param, grp_param, sizeof(app_bap_bc_src_grp_param_t));

    p_grp->big_param.adv_data = NULL;
    p_grp->big_param.adv_data_len = 0;
    p_grp->big_param.per_adv_data = NULL;
    p_grp->big_param.per_adv_data_len = 0;

    if (grp_param->adv_data_len != 0 && grp_param->adv_data != NULL)
    {
        app_bap_bc_src_set_adv_data(big_idx, grp_param->adv_data_len, grp_param->adv_data);
    }

    if (grp_param->per_adv_data_len != 0 && grp_param->per_adv_data != NULL)
    {
        app_bap_bc_src_set_per_adv_data(big_idx, grp_param->per_adv_data_len, grp_param->per_adv_data);
    }

    p_grp->subgrp_info = (app_bap_bc_src_subgrp_info_t *)app_gaf_malloc_buff(sizeof(app_bap_bc_src_subgrp_info_t) * grp_param->nb_subgroups);

    if (!p_grp->subgrp_info)
    {
        LOG_E("[%s][%d]:malloc fail!", __FUNCTION__, __LINE__);
        return BT_STS_NO_RESOURCES;
    }

    memset(p_grp->subgrp_info, 0, sizeof(app_bap_bc_src_subgrp_info_t) * grp_param->nb_subgroups);

    p_grp->stream_info = (app_bap_bc_src_stream_info_t *)app_gaf_malloc_buff(sizeof(app_bap_bc_src_stream_info_t) * grp_param->nb_streams);

    if (p_grp->stream_info == NULL)
    {
        LOG_E("[%s][%d]:malloc fail!", __FUNCTION__, __LINE__);
        app_gaf_free_buff(p_grp->subgrp_info);
        return BT_STS_NO_RESOURCES;
    }

    memset(p_grp->stream_info, 0, sizeof(app_bap_bc_src_stream_info_t) * grp_param->nb_streams);

    return BT_STS_SUCCESS;
}

int app_bap_bc_src_set_subgrp_params(uint8_t big_idx, app_bap_bc_src_subgrp_info_t *subgrp_param)
{
    uint8_t subgrp_lid = subgrp_param->sgrp_lid;
    app_bap_bc_src_grp_info_t *p_grp = &p_app_bc_src_env->p_grp[big_idx];

    if ((big_idx >= APP_BAP_DFT_BC_SRC_MAX_BIG_NUM) || (subgrp_lid >= p_grp->big_param.nb_subgroups))
    {
        LOG_E("app_bap bc src invalid big_idx:(%d, %d), subgrp_idx:(%d, %d)",
              APP_BAP_DFT_BC_SRC_MAX_BIG_NUM, big_idx,
              p_grp->big_param.nb_subgroups, subgrp_lid);

        return BT_STS_INVALID_PARM;
    }

    p_grp->subgrp_info[subgrp_lid].sgrp_lid = subgrp_lid;
    p_grp->subgrp_info[subgrp_lid].codec_id = subgrp_param->codec_id;

    if (subgrp_param->cfg_len && subgrp_param->p_cfg)
    {
        if (p_grp->subgrp_info[subgrp_lid].p_cfg)
        {
            app_gaf_free_buff(p_grp->subgrp_info[subgrp_lid].p_cfg);
        }

        p_grp->subgrp_info[subgrp_lid].p_cfg = (app_gaf_bap_cfg_t *)app_gaf_malloc_buff(subgrp_param->cfg_len);

        if (!p_grp->subgrp_info[subgrp_lid].p_cfg)
        {
            LOG_E("[%s][%d]:malloc fail!", __FUNCTION__, __LINE__);
            return BT_STS_NO_RESOURCES;
        }

        memcpy(p_grp->subgrp_info[subgrp_lid].p_cfg, subgrp_param->p_cfg, subgrp_param->cfg_len);
        p_grp->subgrp_info[subgrp_lid].cfg_len  = subgrp_param->cfg_len;
    }

    if (subgrp_param->p_metadata && subgrp_param->metadata_len)
    {
        if (p_grp->subgrp_info[subgrp_lid].p_metadata)
        {
            app_gaf_free_buff(p_grp->subgrp_info[subgrp_lid].p_metadata);
        }

        p_grp->subgrp_info[subgrp_lid].p_metadata = (app_gaf_bap_cfg_metadata_t *)app_gaf_malloc_buff(subgrp_param->metadata_len);

        if (!p_grp->subgrp_info[subgrp_lid].p_metadata)
        {
            LOG_E("[%s][%d]:malloc fail!", __FUNCTION__, __LINE__);
            return BT_STS_NO_RESOURCES;
        }

        memcpy(p_grp->subgrp_info[subgrp_lid].p_metadata, subgrp_param->p_metadata, subgrp_param->metadata_len);
        p_grp->subgrp_info[subgrp_lid].metadata_len  = subgrp_param->metadata_len;
    }

    return BT_STS_SUCCESS;
}

int app_bap_bc_src_set_stream_params(uint8_t big_idx, app_bap_bc_src_stream_info_t *stream_param)
{
    uint8_t stream_lid = stream_param->stream_lid;
    app_bap_bc_src_grp_info_t *p_grp = &p_app_bc_src_env->p_grp[big_idx];

    if ((big_idx >= APP_BAP_DFT_BC_SRC_MAX_BIG_NUM) || (stream_lid >= p_grp->big_param.nb_streams))
    {
        LOG_E("app_bap bc src invalid big_idx:(%d, %d), subgrp_idx:(%d, %d)",
              APP_BAP_DFT_BC_SRC_MAX_BIG_NUM, big_idx,
              p_grp->big_param.nb_subgroups, stream_lid);

        return BT_STS_INVALID_PARM;
    }

    p_grp->stream_info[stream_lid].stream_lid = stream_param->stream_lid;
    p_grp->stream_info[stream_lid].sgrp_lid   = stream_param->sgrp_lid;
    p_grp->stream_info[stream_lid].dp_cfg     = stream_param->dp_cfg;

    if (stream_param->p_cfg && stream_param->cfg_len)
    {
        if (p_grp->stream_info[stream_lid].p_cfg)
        {
            app_gaf_free_buff(p_grp->stream_info[stream_lid].p_cfg);
        }

        p_grp->stream_info[stream_lid].p_cfg = (app_gaf_bap_cfg_t *)app_gaf_malloc_buff(stream_param->cfg_len);

        if (!p_grp->stream_info[stream_lid].p_cfg)
        {
            LOG_E("[%s][%d]:malloc fail!", __FUNCTION__, __LINE__);
            return BT_STS_NO_RESOURCES;
        }

        memcpy(p_grp->stream_info[stream_lid].p_cfg, stream_param->p_cfg, stream_param->cfg_len);
        p_grp->stream_info[stream_lid].cfg_len = stream_param->cfg_len;
    }

    return BT_STS_SUCCESS;
}

int app_bap_bc_src_set_subgrp_codec_cfg(uint8_t big_idx, uint8_t sgrp_lid,
                                        app_gaf_codec_id_t *codec_id,  uint16_t frame_octet, uint8_t sampling_freq)
{
    if (big_idx >= APP_BAP_DFT_BC_SRC_MAX_BIG_NUM)
    {
        LOG_D("app_bap bc src invalid big_idx");
        return BT_STS_INVALID_PARM;
    }

    if (NULL == codec_id)
    {
        LOG_D("app_bap bc src invalid codec configuration");
        return BT_STS_INVALID_PARM;
    }

    uint8_t cfg_len = sizeof(app_gaf_bap_cfg_t) + APP_BAP_DFT_BC_SRC_ADD_CODEC_LEN;
    app_gaf_bap_cfg_t *p_cfg = (app_gaf_bap_cfg_t *)app_gaf_malloc_buff(cfg_len);

    if (NULL == p_cfg)
    {
        LOG_W("app_bap bc src set stream codec cfg malloc failed");
        return BT_STS_NO_RESOURCES;
    }

    memset(p_cfg, 0, cfg_len);

    p_cfg->param.location_bf = APP_BAP_DFT_BC_SRC_LOCATION_BF;
    p_cfg->param.frame_octet = frame_octet;
    p_cfg->param.sampling_freq = sampling_freq;
    p_cfg->param.frame_dur = APP_BAP_DFT_BC_SRC_FRAME_DURATION;
    p_cfg->param.frames_sdu = APP_BAP_DFT_BC_SRC_NB_LC3_STREAM;
    p_cfg->add_cfg.len = APP_BAP_DFT_BC_SRC_ADD_CODEC_LEN;

    app_bap_bc_src_grp_info_t *p_grp = &p_app_bc_src_env->p_grp[big_idx];
    app_bap_bc_src_subgrp_info_t *sgrp = &p_grp->subgrp_info[sgrp_lid];
    memcpy(&sgrp->codec_id, codec_id, sizeof(app_gaf_codec_id_t));
    sgrp->cfg_len = app_bap_bc_src_codec_cfg_set(&sgrp->p_cfg, p_cfg);

    if (sgrp->cfg_len == 0)
    {
        LOG_D("app_bap bc src subgrp codec config failed");
    }
    else
    {
        uint16_t set_sdu = p_cfg->param.frame_octet * app_bap_get_audio_location_l_r_cnt(p_cfg->param.location_bf);

        if (p_grp->big_param.grp_param.max_sdu < set_sdu)
        {
            p_grp->big_param.grp_param.max_sdu = (set_sdu > APP_GAF_BC_MAX_SDU_SIZE) ? APP_GAF_BC_MAX_SDU_SIZE : set_sdu;
        }

        LOG_I("Calculated big_id %d subgrp %d BIS SDU size = %d", big_idx, sgrp_lid, set_sdu);
    }

    app_gaf_free_buff(p_cfg);

    return BT_STS_SUCCESS;
}

int app_bap_bc_src_update_stream_codec_cfg(uint8_t big_idx, uint8_t stream_lid, app_gaf_bap_cfg_t *p_cfg)
{
    if (big_idx >= APP_BAP_DFT_BC_SRC_MAX_BIG_NUM)
    {
        LOG_D("app_bap bc src invalid big_idx");
        return BT_STS_INVALID_PARM;
    }

    if (p_cfg == NULL)
    {
        return BT_STS_INVALID_PARM;
    }

    app_bap_bc_src_grp_info_t *p_grp = &p_app_bc_src_env->p_grp[big_idx];
    app_bap_bc_src_stream_info_t *stream = &p_grp->stream_info[stream_lid];
    stream->cfg_len = app_bap_bc_src_codec_cfg_set(&stream->p_cfg, p_cfg);

    if (stream->cfg_len == 0)
    {
        LOG_D("app_bap bc src update stream codec config failed");
    }
    else
    {
        uint16_t set_sdu = p_cfg->param.frame_octet * app_bap_get_audio_location_l_r_cnt(p_cfg->param.location_bf);

        if (p_grp->big_param.grp_param.max_sdu < set_sdu)
        {
            p_grp->big_param.grp_param.max_sdu = (set_sdu > APP_GAF_BC_MAX_SDU_SIZE) ? APP_GAF_BC_MAX_SDU_SIZE : set_sdu;
        }

        LOG_I("Calculated big_id %d stream_lid %d BIS SDU size = %d", big_idx, stream_lid, set_sdu);
    }

    app_bap_cfg_print(p_cfg);

    return BT_STS_SUCCESS;
}

int app_bap_bc_src_set_stream_codec_cfg(uint8_t big_idx, uint8_t stream_lid, uint16_t frame_octet, uint8_t sampling_freq)
{
    if (big_idx >= APP_BAP_DFT_BC_SRC_MAX_BIG_NUM)
    {
        LOG_D("app_bap bc src invalid big_idx");
        return BT_STS_INVALID_PARM;
    }

    app_bap_bc_src_grp_info_t *p_grp = &p_app_bc_src_env->p_grp[big_idx];
    app_bap_bc_src_stream_info_t *stream = &p_grp->stream_info[stream_lid];

    // Reserver old value
    if (stream->p_cfg != NULL)
    {
        stream->p_cfg->param.frame_octet = frame_octet;
        stream->p_cfg->param.sampling_freq = sampling_freq;
    }
    else
    {
        app_gaf_bap_cfg_t *p_cfg = (app_gaf_bap_cfg_t *)app_gaf_malloc_buff(sizeof(app_gaf_bap_cfg_t) + APP_BAP_DFT_BC_SRC_ADD_CODEC_LEN);

        if (NULL == p_cfg)
        {
            LOG_W("app_bap bc src set stream codec cfg malloc failed");
            return BT_STS_NO_RESOURCES;
        }

        memset(p_cfg, 0, sizeof(app_gaf_bap_cfg_t) + APP_BAP_DFT_BC_SRC_ADD_CODEC_LEN);

        p_cfg->param.location_bf = APP_BAP_DFT_BC_SRC_LOCATION_BF;
        p_cfg->param.frame_octet = frame_octet;
        p_cfg->param.sampling_freq = sampling_freq;
        p_cfg->param.frame_dur = APP_BAP_DFT_BC_SRC_FRAME_DURATION;
        p_cfg->param.frames_sdu = APP_BAP_DFT_BC_SRC_NB_FRAME_BLOCKS_SDU;
        p_cfg->add_cfg.len = APP_BAP_DFT_BC_SRC_ADD_CODEC_LEN;
        // use new p_cfg
        stream->p_cfg = p_cfg;
        stream->cfg_len = sizeof(app_gaf_bap_cfg_t) + APP_BAP_DFT_BC_SRC_ADD_CODEC_LEN;
    }

    uint16_t set_sdu = stream->p_cfg->param.frame_octet * app_bap_get_audio_location_l_r_cnt(stream->p_cfg->param.location_bf);

    if (p_grp->big_param.grp_param.max_sdu < set_sdu)
    {
        p_grp->big_param.grp_param.max_sdu = (set_sdu > APP_GAF_BC_MAX_SDU_SIZE) ? APP_GAF_BC_MAX_SDU_SIZE : set_sdu;

        LOG_I("Calculated big_id %d stream_lid %d BIS SDU size = %d", big_idx, stream_lid, set_sdu);
    }

    return BT_STS_SUCCESS;
}

int app_bap_bc_src_set_bcast_id(uint8_t big_idx, uint8_t *bcast_id, uint8_t bcast_id_len)
{
    if ((big_idx >= APP_BAP_DFT_BC_SRC_MAX_BIG_NUM) || (!bcast_id) || (bcast_id_len != APP_GAF_BAP_BC_BROADCAST_ID_LEN))
    {
        LOG_E("bc_src_set_bcast_id param invalid, big_idx=%d, p_bcast_code=%p, len=%d",
              big_idx, bcast_id, bcast_id_len);
        return BT_STS_INVALID_PARM;
    }

    app_bap_bc_src_grp_info_t *p_grp = &p_app_bc_src_env->p_grp[big_idx];

    memcpy(p_grp->big_param.bcast_id.id, bcast_id, sizeof(p_grp->big_param.bcast_id));

    return BT_STS_SUCCESS;
}

int app_bap_bc_src_set_encrypt(uint8_t big_idx, uint8_t is_encrypted, app_gaf_bc_code_t *bcast_code)
{
    if (big_idx >= APP_BAP_DFT_BC_SRC_MAX_BIG_NUM)
    {
        LOG_D("app_bap bc src invalid big_idx");
        return BT_STS_INVALID_PARM;
    }

    app_bap_bc_src_grp_info_t *p_grp = &p_app_bc_src_env->p_grp[big_idx];
    p_grp->big_param.encrypted = is_encrypted;

    if (0 != p_grp->big_param.encrypted)
    {
        memcpy(&p_grp->big_param.bcast_code, bcast_code, sizeof(app_gaf_bc_code_t));
    }
    else
    {
        memset(&p_grp->big_param.bcast_code, 0, sizeof(app_gaf_bc_code_t));
    }

    return BT_STS_SUCCESS;
}

int app_bap_bc_src_set_adv_data(uint8_t big_idx, uint8_t adv_len, uint8_t *adv_data)
{
    app_bap_bc_src_grp_info_t *p_grp = &p_app_bc_src_env->p_grp[big_idx];

    /// check input param
    if ((big_idx >= APP_BAP_DFT_BC_SRC_MAX_BIG_NUM)
            || (adv_len > APP_EXT_ADV_DATA_MAX_LEN)
            || (adv_len == 0)
            || (adv_data == NULL))
    {
        LOG_W("[%s][%d]: %d, %d, %p", __FUNCTION__, __LINE__, big_idx, adv_len, adv_data);
        return BT_STS_INVALID_PARM;
    }

    /// set adv data param
    if (p_grp->big_param.adv_data)
    {
        app_gaf_free_buff(p_grp->big_param.adv_data);
    }

    p_grp->big_param.adv_data = app_gaf_malloc_buff(adv_len);

    if (!p_grp->big_param.adv_data)
    {
        LOG_E("[%s][%d]: malloc error!", __FUNCTION__, __LINE__);
        return BT_STS_NO_RESOURCES;
    }

    memcpy(p_grp->big_param.adv_data, adv_data, adv_len);
    p_grp->big_param.adv_data_len = adv_len;

    return BT_STS_SUCCESS;
}

int app_bap_bc_src_set_per_adv_data(uint8_t big_idx, uint8_t per_adv_len, uint8_t *per_adv_data)
{
    app_bap_bc_src_grp_info_t *p_grp = &p_app_bc_src_env->p_grp[big_idx];

    /// check input param
    if ((big_idx >= APP_BAP_DFT_BC_SRC_MAX_BIG_NUM)
            || (per_adv_len > APP_EXT_ADV_DATA_MAX_LEN)
            || (per_adv_len == 0)
            || (per_adv_data == NULL))
    {
        LOG_W("[%s][%d]: %d, %d, %p", __FUNCTION__, __LINE__, big_idx, per_adv_len, per_adv_data);
        return BT_STS_INVALID_PARM;
    }

    /// set per adv data
    if (p_grp->big_param.per_adv_data)
    {
        app_gaf_free_buff(p_grp->big_param.per_adv_data);
    }

    p_grp->big_param.per_adv_data = app_gaf_malloc_buff(per_adv_len);

    if (!p_grp->big_param.per_adv_data)
    {
        LOG_E("[%s][%d]: malloc error!", __FUNCTION__, __LINE__);
        return BT_STS_NO_RESOURCES;
    }

    memcpy(p_grp->big_param.per_adv_data, per_adv_data, per_adv_len);
    p_grp->big_param.per_adv_data_len = per_adv_len;

    return BT_STS_SUCCESS;
}

int app_bap_bc_src_enable_pa(app_bap_bc_src_grp_info_t *p_grp)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_bap_bc_src_enable(app_bap_bc_src_grp_info_t *p_grp)
{
    return 0;
}

int app_bap_bc_src_disable_pa(uint8_t grp_lid)
{
    return bap_bc_src_disable_group_related_pa(grp_lid);
}

int app_bap_bc_src_disable(uint8_t grp_lid)
{
    return bap_bc_src_release_group(grp_lid);
}

int app_bap_bc_src_start_streaming(uint8_t big_idx, uint8_t stream_lid_bf)
{
    app_bap_bc_src_grp_info_t *p_grp = app_bap_bc_src_get_big_info_by_big_idx(big_idx);

    if (p_grp == NULL)
    {
        return BT_STS_NOT_FOUND;
    }

    return bap_bc_src_enable_group_stream(p_grp->grp_lid, stream_lid_bf << 1);
}

int app_bap_bc_src_update_adv_data_req(uint8_t grp_lid, uint8_t adv_type,
                                       uint8_t data_len, uint8_t *data)
{
    uint16_t status = BT_STS_SUCCESS;
    gen_aud_var_info_t *p_add_data = NULL;

    if (data != NULL && data_len != 0)
    {
        p_add_data = (gen_aud_var_info_t *)app_gaf_malloc_buff(sizeof(gen_aud_var_info_t) + data_len);

        if (p_add_data == NULL)
        {
            return BT_STS_NO_RESOURCES;
        }

        memcpy(p_add_data->data, data, data_len);
        p_add_data->len = data_len;
    }

    if (adv_type == 1)
    {
        status = bap_bc_src_update_group_additional_pa_data(grp_lid, p_add_data);
    }
    else
    {
        status = bap_bc_src_update_group_additional_ea_data(grp_lid, p_add_data);
    }

    if (p_add_data != NULL)
    {
        app_gaf_free_buff(p_add_data);
    }

    return status;
}

int app_bap_bc_src_update_metadata_req(uint8_t grp_lid, uint8_t sgrp_lid,
                                       app_gaf_bap_cfg_metadata_t *metadata)
{
    if (metadata == NULL)
    {
        return BT_STS_INVALID_PARM;
    }

    gen_aud_metadata_t *p_md_update = (gen_aud_metadata_t *)
                                      app_gaf_malloc_buff(metadata->add_metadata.len + sizeof(gen_aud_metadata_t));

    if (p_md_update == NULL)
    {
        return BT_STS_NO_RESOURCES;
    }

    gen_aud_init_metadata(p_md_update);

    p_md_update->basic_metadata.streaming_audio_context = metadata->param.context_bf;
    p_md_update->add_metadata.len = metadata->add_metadata.len;
    memcpy(p_md_update->add_metadata.data,
           metadata->add_metadata.data,
           metadata->add_metadata.len);

    uint16_t status = bap_bc_src_update_group_metadata(grp_lid, sgrp_lid, p_md_update);

    app_gaf_free_buff(p_md_update);

    return status;
}

int app_bap_bc_src_stop_streaming(uint8_t big_idx, uint8_t stream_lid_bf)
{
    app_bap_bc_src_grp_info_t *p_grp = app_bap_bc_src_get_big_info_by_big_idx(big_idx);

    if (p_grp == NULL)
    {
        return BT_STS_NOT_FOUND;
    }

    return bap_bc_src_disable_group_stream(p_grp->grp_lid, stream_lid_bf << 1);
}

int app_bap_bc_src_remove_group_cmd(uint8_t grp_lid)
{
    return bap_bc_src_delete_group(grp_lid);
}

#endif
#endif

/// @} APP
