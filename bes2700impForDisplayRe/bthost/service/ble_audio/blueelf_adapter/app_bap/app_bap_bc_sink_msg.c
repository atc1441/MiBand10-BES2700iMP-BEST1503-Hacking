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
#ifdef APP_BLE_BIS_SINK_ENABLE
#include "ble_app_dbg.h"
#include "app_bap.h"

#include "app_gaf.h"
#include "app_gaf_custom_api.h"

#include "app_bap_data_path_itf.h"
#include "app_bap_bc_sink_msg.h"
#include "app_bap_bc_scan_msg.h"

#include "bap_broadcast_sink.h"

static app_bap_bc_sink_env_t sink_env = {0};

/*EXTERN FUNCTIONS*/
extern app_gaf_bap_bc_scan_pa_report_info_t *app_bap_bc_scan_get_exist_pa_info_by_hdl(uint16_t pa_sync_hdl);

/*LOCAL FUNCTIONS*/
int app_bap_bc_sink_rx_stream_start(uint16_t bis_hdl, uint16_t max_sdu, uint8_t grp_lid)
{
    uint8_t status = BT_STS_SUCCESS;
    const struct data_path_itf *_rx_dp_itf = NULL;
    LOG_I("app_bap bc sink start rx stream, bis_hdl = %d", bis_hdl);

    _rx_dp_itf = data_path_itf_get(ISO_DP_ISOOHCI, ISO_SEL_RX);

    if (_rx_dp_itf == NULL)
    {
        LOG_W("app_bap bc sink get rx data path interface failed");
        _rx_dp_itf = data_path_itf_get(ISO_DP_DISABLE, ISO_SEL_RX);
    }

    app_bap_set_rx_dp_itf((struct data_path_itf *)_rx_dp_itf);

    if (NULL != _rx_dp_itf->cb_start)
    {
        status = _rx_dp_itf->cb_start(bis_hdl, APP_GAF_BC_SDU_INTERVAL_US, 2 * APP_GAF_BC_SDU_INTERVAL_US, max_sdu);

        if (status != BT_STS_SUCCESS)
        {
            LOG_W("app_bap bc sink start rx stream failed, status = %d", status);
        }
    }

    return status;
}

int app_bap_bc_sink_rx_stream_stop(uint16_t bis_hdl, uint8_t grp_lid)
{
    struct data_path_itf *_rx_dp_itf = app_bap_get_rx_dp_itf();
    LOG_I("app_bap bc sink stop rx stream, bis_hdl = %d _rx_dp_itf %p", bis_hdl, _rx_dp_itf);

    if (_rx_dp_itf && NULL != _rx_dp_itf->cb_stop)
    {
        _rx_dp_itf->cb_stop(bis_hdl, 0);

        return BT_STS_SUCCESS;
    }

    return BT_STS_NOT_READY;
}

bool app_bap_bc_sink_get_iso_data(uint16_t conhdl, uint8_t *buf, uint32_t *len,
                                  dp_itf_iso_buffer_t *iso_buffer)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_bap_bc_sink_get_stream_lid(uint32_t stream_pos_bf)
{
    uint8_t stream_lid;

    for (stream_lid = 0; stream_lid < 31; stream_lid++)
    {
        if (stream_pos_bf & (1 << stream_lid))
        {
            break;
        }
    }

    return stream_lid;
}

int app_bap_bc_sink_get_stream_pos(uint32_t stream_pos_bf)
{
    uint8_t stream_pos;

    for (stream_pos = 0; stream_pos < 31; stream_pos++)
    {
        if (stream_pos_bf & (1 << stream_pos))
        {
            /// clear stream pos bit
            stream_pos_bf &= ~(1 << stream_pos);
            ASSERT(stream_pos_bf == 0, "%s input stream_pos_bf contains more than one stream!", __func__);
            break;
        }
    }

    return stream_pos + 1;
}

static void app_bap_bc_sink_cb_big_sync_state(uint16_t pa_sync_hdl, uint8_t big_id, uint8_t bis_cnt, const bap_bc_sink_stream_t *p_bis_stream_info, uint16_t err_code, const bap_bis_timing_t *p_bis_timing)
{
    if (app_bap_bc_scan_get_exist_pa_info_by_hdl(pa_sync_hdl) == NULL)
    {
        LOG_W("%s no corresponding pa", __func__);
    }

    LOG_I("app_bap bc err_code = %d, pa_sync_hdl = 0x%x, big_id = %d, bis_cnt = %d", err_code, pa_sync_hdl, big_id, bis_cnt);

    app_gaf_bc_sink_status_ind_t *p_bis_stream_sink_ind = (app_gaf_bc_sink_status_ind_t *)app_gaf_malloc_buff(sizeof(app_gaf_bc_sink_status_ind_t) + sizeof(uint16_t) * bis_cnt);

    if (p_bis_stream_sink_ind == NULL)
    {
        LOG_E("%s malloc failed", __func__);
        return;
    }

    if (err_code == BT_LL_ERR_NO_ERROR)
    {
        app_gaf_evt_report(APP_GAF_SINK_BIS_SINK_ENABLED_IND, (void *)&big_id, sizeof(uint16_t));

        sink_env.sink_state = APP_GAF_BAP_BC_SINK_STATE_ENABLED;
        sink_env.stream_pos_bf = 0;

        uint8_t bis_idx = 0;

        for (bis_idx = 0; bis_idx < bis_cnt; bis_idx++)
        {
            sink_env.stream_pos_bf |= (1 << p_bis_stream_info[bis_idx].bis_idx);
            // Assign bis hdl
            p_bis_stream_sink_ind->conhdl[bis_idx] = p_bis_stream_info[bis_idx].bis_hdl;
        }
        // Assign stream info
        p_bis_stream_sink_ind->stream_pos_bf = sink_env.stream_pos_bf;
        p_bis_stream_sink_ind->bg_cfg.bn = p_bis_timing->bn;
        p_bis_stream_sink_ind->bg_cfg.irc = p_bis_timing->irc;
        p_bis_stream_sink_ind->bg_cfg.iso_interval_frames = p_bis_timing->iso_interval_1_25ms;
        p_bis_stream_sink_ind->bg_cfg.max_pdu = p_bis_timing->max_pdu_size;
        p_bis_stream_sink_ind->bg_cfg.nse = p_bis_timing->nse;
        p_bis_stream_sink_ind->bg_cfg.pto = p_bis_timing->pto;
        p_bis_stream_sink_ind->bg_cfg.tlatency_us = p_bis_timing->transport_latency_big_us;
        p_bis_stream_sink_ind->grp_lid = big_id;
        p_bis_stream_sink_ind->nb_bis = bis_cnt;
        p_bis_stream_sink_ind->state = APP_GAF_BAP_BC_SINK_ESTABLISHED;
    }
    else if (err_code == BT_LL_ERR_TERMINATED_MIC_FAILURE)
    {
        p_bis_stream_sink_ind->state = APP_GAF_BAP_BC_SINK_MIC_FAILURE;
    }
    else if (err_code == BT_LL_ERR_OP_CANCELED_BY_HOST)
    {
        p_bis_stream_sink_ind->state = APP_GAF_BAP_BC_SINK_CANCELLED;
    }
    else
    {
        p_bis_stream_sink_ind->state = APP_GAF_BAP_BC_SINK_FAILED;
    }

    app_gaf_evt_report(APP_GAF_SINK_BIS_STATUS_IND, (void *)p_bis_stream_sink_ind, sizeof(app_gaf_bc_sink_status_ind_t) + sizeof(uint16_t) * bis_cnt);

    app_gaf_free_buff(p_bis_stream_sink_ind);
}

static void app_bap_bc_sink_cb_big_sync_term(uint8_t big_id, uint16_t err_code)
{
    LOG_I("%s err_code = %d, big_id = %d", __func__, err_code, big_id);

    sink_env.sink_state = APP_GAF_BAP_BC_SINK_STATE_IDLE;

    app_gaf_bc_sink_status_ind_t bis_stream_sink_ind =
    {
        .grp_lid = big_id,
    };

    if (err_code == BT_LL_ERR_REMOTE_USER_TERM_CON)
    {
        bis_stream_sink_ind.state = APP_GAF_BAP_BC_SINK_PEER_TERMINATE;
    }
    else if (err_code == BT_LL_ERR_CON_TIMEOUT)
    {
        bis_stream_sink_ind.state = APP_GAF_BAP_BC_SINK_LOST;
    }
    // BIS start sync without data can not check mic,
    // when data is recv, err bcast code lead to mic failure
    else if (err_code == BT_LL_ERR_TERMINATED_MIC_FAILURE)
    {
        bis_stream_sink_ind.state = APP_GAF_BAP_BC_SINK_MIC_FAILURE;
    }
    else
    {
        bis_stream_sink_ind.state = APP_GAF_BAP_BC_SINK_UPPER_TERMINATE;
    }

    app_gaf_evt_report(APP_GAF_SINK_BIS_STATUS_IND, (void *)&bis_stream_sink_ind, sizeof(bis_stream_sink_ind));

    if (bis_stream_sink_ind.state == APP_GAF_BAP_BC_SINK_UPPER_TERMINATE)
    {
        app_gaf_evt_report(APP_GAF_SINK_BIS_SINK_DISABLED_IND, (void *)&big_id, sizeof(uint16_t));
    }
}

static void app_bap_bc_sink_cb_iso_dp_state(bool is_create, uint8_t big_id, uint8_t bis_idx, uint16_t err_code)
{
    LOG_I("%s err_code = %d, is_create = %d, big_id = %d, bis_idx = %d", __func__, err_code, is_create, big_id, bis_idx);

    if (is_create == true)
    {
        sink_env.sink_state = APP_GAF_BAP_BC_SINK_STATE_STREAMING;
        app_gaf_evt_report(APP_GAF_SINK_BIS_STREAM_STARTED_IND, (void *)&bis_idx, sizeof(uint8_t));
    }
    else
    {
        sink_env.sink_state = APP_GAF_BAP_BC_SINK_STATE_IDLE;
        app_gaf_evt_report(APP_GAF_SINK_BIS_STREAM_STOPPED_IND, (void *)&bis_idx, sizeof(uint8_t));
    }
}

static const bap_bc_sink_callback_t app_bap_bc_sink_app_cb =
{
    .cb_big_sync_state = app_bap_bc_sink_cb_big_sync_state,
    .cb_big_sync_term = app_bap_bc_sink_cb_big_sync_term,
    .cb_bis_iso_dp_state = app_bap_bc_sink_cb_iso_dp_state,
};

int app_bap_bc_sink_init(void)
{
    LOG_I("%s", __func__);

    uint16_t status = bap_bc_sink_init();

    if (status == BT_STS_SUCCESS)
    {
        status = bap_bc_sink_callback_register(BAP_BC_COMMON_USER_APPLICATION_IMPL, &app_bap_bc_sink_app_cb);
    }

    return status;
}

int app_bap_bc_sink_deinit(void)
{
    LOG_I("%s", __func__);

    uint16_t status = bap_bc_sink_deinit();

    if (status == BT_STS_SUCCESS)
    {
        status = bap_bc_sink_callback_unregister(BAP_BC_COMMON_USER_APPLICATION_IMPL);
    }

    return status;
}

int app_bap_bc_sink_enable(app_gaf_bap_bc_sink_enable_t *p_sink_enable)
{
    if (p_sink_enable == NULL)
    {
        return BT_STS_INVALID_PARM;
    }

    app_gaf_bap_bc_scan_pa_report_info_t *pa_info = app_bap_bc_scan_get_exist_pa_info_by_pa_lid(p_sink_enable->pa_lid);

    if (pa_info == NULL)
    {
        LOG_E("%s pa_lid = %d not found", __func__, p_sink_enable->pa_lid);
        return BT_STS_NOT_FOUND;
    }

    const uint8_t *p_broadcast_code = p_sink_enable->encrypted ? p_sink_enable->bcast_code.bcast_code : NULL;

    uint8_t bis_idx_vector[BAP_MAX_BIS_PER_BIG] = {0};

    uint8_t bis_cnt = 0;

    uint8_t bis_idx_lid = 0;

    uint32_t bis_sync_req_bf = p_sink_enable->stream_pos_bf;

    for (bis_idx_lid = 0; bis_sync_req_bf != 0 && bis_cnt < pa_info->group_info.nb_streams; bis_idx_lid++)
    {
        if (bis_sync_req_bf & (1 << (bis_idx_lid)))
        {
            bis_sync_req_bf &= ~(1 << bis_idx_lid);

            bis_idx_vector[bis_cnt++] = bis_idx_lid + BAP_BC_BIS_INDEX_MIN;
        }
    }

    return bap_bc_sink_sync_big(pa_info->pa_sync_hdl, p_sink_enable->timeout_10ms, p_sink_enable->mse,
                                bis_cnt, bis_idx_vector, p_broadcast_code, 0);
}

int app_bap_bc_sink_disable(uint8_t grp_lid)
{
    return bap_bc_sink_termiante_big_sync(grp_lid);
}

int app_bap_bc_sink_streaming_config(uint8_t grp_lid, uint32_t stream_pos_bf, uint8_t codec_type,
                                     app_gaf_bap_bc_scan_subgrp_info_t *subgroup_info,
                                     app_gaf_bap_bc_scan_stream_info_t *select_stream_info,
                                     app_gaf_bap_bc_sink_audio_streaming_t *sink_stream_info)
{
    uint8_t stream_pos = app_bap_bc_sink_get_stream_pos(stream_pos_bf);
    app_gaf_bap_cfg_param_t *subgrp_para = NULL;
    sink_stream_info->grp_lid = grp_lid;
    sink_stream_info->stream_pos = stream_pos;
    memcpy(sink_stream_info->codec_id.codec_id, select_stream_info->codec_id.codec_id, APP_GAF_CODEC_ID_LEN);

    switch (codec_type)
    {
        case APP_GAF_CODEC_TYPE_LC3:
        {
            sink_stream_info->cfg_len = sizeof(app_gaf_bap_cfg_t);
            app_gaf_bap_cfg_t *p_cfg = (app_gaf_bap_cfg_t *)sink_stream_info->cfg;

            if (select_stream_info)
            {
                memcpy(&p_cfg->param, &select_stream_info->param, sizeof(app_gaf_bap_cfg_param_t));
            }

            /// Arguments that are not in stream_info are obtained in subgroup_info
            if (subgroup_info)
            {
                subgrp_para = &subgroup_info->param;

                if ((!p_cfg->param.location_bf) && (subgrp_para->location_bf))
                {
                    p_cfg->param.location_bf = subgrp_para->location_bf;
                }
                if ((!p_cfg->param.frame_octet) && (subgrp_para->frame_octet))
                {
                    p_cfg->param.frame_octet = subgrp_para->frame_octet;
                }
                if (((!p_cfg->param.sampling_freq) || (p_cfg->param.sampling_freq >= APP_GAF_BAP_SAMPLING_FREQ_MAX))
                        && (subgrp_para->sampling_freq))
                {
                    p_cfg->param.sampling_freq = subgrp_para->sampling_freq;
                }
                if ((p_cfg->param.frame_dur >= APP_GAF_BAP_FRAME_DUR_MAX) && (subgrp_para->frame_dur))
                {
                    p_cfg->param.frame_dur = subgrp_para->frame_dur;
                }
                if ((!p_cfg->param.frames_sdu) && (subgrp_para->frames_sdu))
                {
                    p_cfg->param.frames_sdu = subgrp_para->frames_sdu;
                }
            }

            p_cfg->add_cfg.len = 0;

            app_bap_cfg_print(p_cfg);
        }
        break;
        default:
            ASSERT(0, "Unknown codec type %d", codec_type);
            break;
    }

    // Avoid no codec info
    sink_stream_info->codec_id.codec_id[0] = codec_type;

    return BT_STS_SUCCESS;
}

int app_bap_bc_sink_start_streaming(app_gaf_bap_bc_sink_audio_streaming_t *p_bis_stream)
{
    if (p_bis_stream == NULL || p_bis_stream->cfg_len == 0)
    {
        LOG_W("%s invalid parameters", __func__);
        return BT_STS_INVALID_PARM;
    }

    /// TODO: where is add cc? or we need this?
    app_gaf_bap_cfg_t *p_cfg = (app_gaf_bap_cfg_t *)p_bis_stream->cfg;

    gen_aud_cc_t cc =
    {
        .basic_cc_param.audio_chan_allocation_bf = p_cfg->param.location_bf,
        .basic_cc_param.sampling_freq = p_cfg->param.sampling_freq,
        .basic_cc_param.frame_blocks_per_sdu = p_cfg->param.frames_sdu,
        .basic_cc_param.frame_octets = p_cfg->param.frame_octet,
        .basic_cc_param.frame_dur = p_cfg->param.frame_dur,
        .add_cc_param.len = 0,
    };

    LOG_I("stream pos %d to start stream", p_bis_stream->stream_pos);

    uint16_t status = bap_bc_sink_setup_iso_data_path(p_bis_stream->grp_lid, p_bis_stream->stream_pos, p_bis_stream->codec_id.codec_id, &cc);

    if (status != BT_STS_SUCCESS)
    {
        LOG_E("%s status = %d", __func__, status);
    }

    return status;
}

int app_bap_bc_sink_stop_streaming(uint8_t grp_lid, uint8_t stream_pos)
{
    return bap_bc_sink_remove_iso_data_path(grp_lid, stream_pos);
}

app_bap_bc_sink_env_t *app_bap_bc_sink_get_env()
{
    return &sink_env;
}

#endif
#endif
/// @} APP
