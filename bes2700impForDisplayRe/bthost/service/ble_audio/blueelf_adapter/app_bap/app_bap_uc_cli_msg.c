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
#include "ble_app_dbg.h"
#include "app_bap.h"

#include "app_gaf.h"
#include "app_bap_uc_cli_msg.h"
#include "app_bap_capa_cli_msg.h"
#include "app_bap_data_path_itf.h"

#include "app_gaf_custom_api.h"

#include "bap_unicast_client.h"
/*
 * DEFINES
 ****************************************************************************************
 */
/// We use  Codec Frame duration as SDU interval
#define APP_BAP_DFT_ASCC_LC3_BLOCKS_PER_SDU     (1)

/// TODO: use another alogi
#define APP_BAP_ASCC_ASE_ID_TO_ASE_LID(con_lid, ase_id)     ((con_lid * APP_BAP_DFT_ASCC_NB_ASE_CFG + ase_id - ASCS_ASE_ID_MIN) %\
                                                                            (1 + con_lid) *APP_BAP_DFT_ASCC_NB_ASE_CFG)
/*
 * INTERNAL VALUE
 ****************************************************************************************
 */
app_bap_ascc_env_t *p_app_ascc_env = NULL;

/*
 * EXTERNAL VALUE
 ****************************************************************************************
 */
extern char *ase_state_str[ASCS_ASE_STATE_MAX + 1];

/*
 * FUNCTION
 ****************************************************************************************
 */
bool app_bap_uc_cli_is_already_bonded(uint8_t con_lid)
{
    if (p_app_ascc_env == NULL)
    {
        return false;
    }

    for (uint8_t ase_lid = 0; ase_lid < p_app_ascc_env->nb_ases_cfg; ase_lid++)
    {
        app_bap_ascc_ase_t *p_ase_info = &p_app_ascc_env->ase_info[ase_lid];

        if (p_ase_info->con_lid == con_lid)
        {
            return true;
        }
    }

    return false;
}

static app_bap_ascc_ase_t *app_bap_uc_cli_find_valid_ase_by_con_lid(uint8_t con_lid)
{
    for (uint8_t ase_lid = 0; ase_lid < p_app_ascc_env->nb_ases_cfg; ase_lid++)
    {
        app_bap_ascc_ase_t *p_ase_info = &p_app_ascc_env->ase_info[ase_lid];

        LOG_I("con_lid:%d, incoming con_lid:%d", p_ase_info->con_lid, con_lid);

        if (((p_ase_info->con_lid == con_lid) ||
                (APP_GAF_INVALID_CON_LID == p_ase_info->con_lid)) &&
                (ASCS_INVALID_ASE_INDEX == p_ase_info->ase_instance_idx))
        {
            return p_ase_info;
        }
    }

    return NULL;
}

uint8_t app_bap_uc_get_ase_state_by_ase_id(uint8_t ase_id)
{
    app_bap_ascc_ase_t *p_ase_info = &p_app_ascc_env->ase_info[ase_id];

    if (p_ase_info && (ase_id < p_app_ascc_env->nb_ases_cfg))
    {
        return p_ase_info->ase_state;
    }

    return APP_GAF_INVALID_CON_LID;
}

uint8_t app_bap_uc_cli_get_exist_ase_lid_by_ase_instance_idx(uint8_t con_lid, uint8_t instance_idx)
{
    for (uint8_t ase_lid = 0; ase_lid < p_app_ascc_env->nb_ases_cfg; ase_lid++)
    {
        app_bap_ascc_ase_t *p_ase_info = &p_app_ascc_env->ase_info[ase_lid];

        if ((p_ase_info->con_lid == con_lid) &&
                (p_ase_info->ase_instance_idx == instance_idx))
        {
            return p_ase_info->ase_lid;
        }
    }

    return APP_GAF_INVALID_CON_LID;
}

uint8_t app_bap_uc_cli_get_exist_ase_lid_list_by_cis_id(uint8_t cig_id, uint8_t cis_id,
                                                        uint8_t *ase_lid_list)
{
    if (ase_lid_list == NULL)
    {
        return 0;
    }

    uint8_t idx = 0;

    for (uint8_t ase_lid = 0; ase_lid < p_app_ascc_env->nb_ases_cfg; ase_lid++)
    {
        app_bap_ascc_ase_t *p_ase_info = &p_app_ascc_env->ase_info[ase_lid];

        if (p_ase_info->cis_id == cis_id)
        {
            ase_lid_list[idx++] = p_ase_info->ase_lid;
        }
    }

    return idx;
}

uint8_t app_bap_uc_cli_get_ase_under_enabling_by_ase_id(uint8_t con_lid, uint8_t instance_idx)
{
    for (uint8_t ase_lid = 0; ase_lid < p_app_ascc_env->nb_ases_cfg; ase_lid++)
    {
        app_bap_ascc_ase_t *p_ase_info = &p_app_ascc_env->ase_info[ase_lid];

        LOG_D("app_bap uc_cli get ase by instant con_lid:%d, aseIdx:%d, state:%d, input: %d, %d",
              p_ase_info->con_lid, p_ase_info->ase_instance_idx, p_ase_info->ase_state, con_lid, instance_idx);

        if ((p_ase_info->con_lid == con_lid) &&
                (p_ase_info->ase_instance_idx == instance_idx) &&
                (p_ase_info->ase_state <= ASCS_ASE_STATE_QOS_CONFIGURED))
        {
            return p_ase_info->ase_lid;
        }
    }

    return APP_GAF_INVALID_CON_LID;
}

uint8_t app_bap_uc_cli_get_ase_lid(uint8_t con_lid, uint8_t ase_state, uint8_t direction)
{
    if (!app_bap_uc_cli_is_already_bonded(con_lid))
    {
        LOG_W("%s the specified con[%d] has not beed bonded", __func__, con_lid);
        return APP_GAF_INVALID_CON_LID;
    }

    for (uint8_t ase_lid = 0; ase_lid < p_app_ascc_env->nb_ases_cfg; ase_lid++)
    {
        app_bap_ascc_ase_t *p_ase_info = &p_app_ascc_env->ase_info[ase_lid];

        if ((p_ase_info->con_lid == con_lid) &&
                (p_ase_info->ase_state == ase_state) &&
                (p_ase_info->direction == direction))
        {
            return p_ase_info->ase_lid;
        }
    }

    return APP_GAF_INVALID_CON_LID;
}

uint8_t app_bap_uc_cli_get_specific_state_ase_lid_list(uint8_t con_lid, uint8_t direction,
                                                       uint8_t ase_state, uint8_t *ase_lid_list)
{
    uint8_t ase_nb = 0;
    if (!ase_lid_list)
    {
        LOG_E("%s err ase_lid_list input", __func__);
        return 0;
    }

    if (!app_bap_uc_cli_is_already_bonded(con_lid))
    {
        LOG_W("app_bap uc_cli the specified con[%d] has not beed bonded", con_lid);
        return 0;
    }

    for (uint8_t ase_lid = 0; ase_lid < p_app_ascc_env->nb_ases_cfg; ase_lid++)
    {
        app_bap_ascc_ase_t *p_ase_info = &p_app_ascc_env->ase_info[ase_lid];

        if ((p_ase_info->con_lid == con_lid) &&
                (p_ase_info->ase_state == ase_state) &&
                (p_ase_info->direction == direction || direction == APP_GAF_DIRECTION_MAX))
        {
            ase_lid_list[ase_nb++] = ase_lid;
        }
    }

    return ase_nb;
}

uint8_t app_bap_uc_cli_get_streaming_ase_lid(uint8_t con_lid, enum app_gaf_direction direction)
{
    app_bap_ascc_ase_t *p_ase = NULL;

    for (uint8_t ase_lid = 0; ase_lid < p_app_ascc_env->nb_ases_cfg; ase_lid++)
    {
        p_ase = &p_app_ascc_env->ase_info[ase_lid];
        if ((p_ase->ase_state == ASCS_ASE_STATE_STREAMING) &&
                (direction == p_ase->direction) && (con_lid == p_ase->con_lid))
        {
            return ase_lid;
        }
    }

    return APP_GAF_INVALID_CON_LID;
}

uint8_t app_bap_uc_cli_tx_stream_start(app_bap_ascc_ase_t *p_ase_info)
{
    uint8_t status = BT_STS_SUCCESS;
    LOG_D("cis hdl 0x%x", p_ase_info->cis_hdl);
    const struct data_path_itf *_tx_dp_itf = NULL;
    _tx_dp_itf = data_path_itf_get(ISO_DP_ISOOHCI, ISO_SEL_TX);

    if (NULL == _tx_dp_itf)
    {
        LOG_W("app_bap uc_cli get tx data path interface failed");
        _tx_dp_itf = data_path_itf_get(ISO_DP_DISABLE, ISO_SEL_TX);
    }

    app_bap_set_tx_dp_itf((struct data_path_itf *)_tx_dp_itf);

    if (_tx_dp_itf && (NULL != _tx_dp_itf->cb_start))
    {
        uint32_t sdu_interval = p_app_ascc_env->grp_info.grp_params.sdu_intv_m2s_us;
        uint32_t trans_latency = p_app_ascc_env->grp_info.grp_params.tlatency_m2s_ms;
        uint16_t max_sdu = p_ase_info->qos_cfg.max_sdu_size;
        status = _tx_dp_itf->cb_start(p_ase_info->cis_hdl, sdu_interval, trans_latency * 1000, max_sdu);
        if (status != BT_STS_SUCCESS)
        {
            LOG_W("app_bap uc_cli start tx stream failed 0x%x cis_hdl:%d", status, p_ase_info->cis_hdl);
        }
        else
        {
            LOG_I("%s %d, %d", __func__, p_ase_info->ase_lid, p_ase_info->con_lid);

            app_gaf_ascc_cis_stream_started_t cis_stream_started;
            cis_stream_started.con_lid = p_ase_info->con_lid;
            cis_stream_started.ase_lid = p_ase_info->ase_lid;
            cis_stream_started.cis_hdl = p_ase_info->cis_hdl;
            cis_stream_started.direction = APP_GAF_DIRECTION_SINK;
            app_gaf_mobile_evt_report(APP_GAF_ASCC_CIS_STREAM_STARTED_IND, (void *)&cis_stream_started,
                                      sizeof(cis_stream_started));
        }
    }

    return status;
}

static void app_bap_uc_cli_tx_stream_stop(app_bap_ascc_ase_t *p_ase_info)
{
    struct data_path_itf *_tx_dp_itf = app_bap_get_tx_dp_itf();

    LOG_D("app_bap uc_cli stop tx stream, cis_hdl = %d _tx_dp_itf %p", p_ase_info->cis_hdl, _tx_dp_itf);
    if (_tx_dp_itf && (NULL != _tx_dp_itf->cb_stop))
    {
        app_gaf_ascc_cis_stream_stopped_t cis_stream_stopped;
        cis_stream_stopped.con_lid = p_ase_info->con_lid;
        cis_stream_stopped.ase_lid = p_ase_info->ase_lid;
        cis_stream_stopped.cis_hdl = p_ase_info->cis_hdl;
        cis_stream_stopped.direction = APP_GAF_DIRECTION_SINK;
        app_gaf_mobile_evt_report(APP_GAF_ASCC_CIS_STREAM_STOPPED_IND, (void *)&cis_stream_stopped,
                                  sizeof(app_gaf_ascc_cis_stream_stopped_t));
    }
}

static uint8_t app_bap_uc_cli_rx_stream_start(app_bap_ascc_ase_t *p_ase_info)
{
    uint8_t status = BT_STS_SUCCESS;
    const struct data_path_itf *_rx_dp_itf = NULL;

    LOG_D("app_bap uc_cli start rx stream, cis_hdl = %d", p_ase_info->cis_hdl);

    _rx_dp_itf = data_path_itf_get(ISO_DP_ISOOHCI, ISO_SEL_RX);
    if (_rx_dp_itf == NULL)
    {
        LOG_W("app_bap uc_cli get rx data path interface failed");
        _rx_dp_itf = data_path_itf_get(ISO_DP_DISABLE, ISO_SEL_RX);
    }
    app_bap_set_rx_dp_itf((struct data_path_itf *)_rx_dp_itf);

    if (NULL != _rx_dp_itf->cb_start)
    {
        uint32_t sdu_interval = p_app_ascc_env->grp_info.grp_params.sdu_intv_s2m_us;
        uint32_t trans_latency = p_app_ascc_env->grp_info.grp_params.tlatency_s2m_ms;
        uint16_t max_sdu = p_ase_info->qos_cfg.max_sdu_size;
        status = _rx_dp_itf->cb_start(p_ase_info->cis_hdl, sdu_interval, trans_latency * 1000, max_sdu);
        if (status != BT_STS_SUCCESS)
        {
            LOG_W("app_bap uc_cli start rx stream failed 0x%x cis_hdl:%d", status, p_ase_info->cis_hdl);
        }
        else
        {
            app_gaf_ascc_cis_stream_started_t cis_stream_started;
            cis_stream_started.con_lid = p_ase_info->con_lid;
            cis_stream_started.ase_lid = p_ase_info->ase_lid;
            cis_stream_started.cis_hdl = p_ase_info->cis_hdl;
            cis_stream_started.direction = APP_GAF_DIRECTION_SRC;
            app_gaf_mobile_evt_report(APP_GAF_ASCC_CIS_STREAM_STARTED_IND, (void *)&cis_stream_started,
                                      sizeof(cis_stream_started));
        }
    }

    return status;
}

static void app_bap_uc_cli_rx_stream_stop(app_bap_ascc_ase_t *p_ase_info)
{
    struct data_path_itf *_rx_dp_itf = app_bap_get_rx_dp_itf();

    LOG_D("app_bap uc_cli stop rx stream, cis_hdl = %d _rx_dp_itf %p", p_ase_info->cis_hdl, _rx_dp_itf);
    if (_rx_dp_itf && (NULL != _rx_dp_itf->cb_stop))
    {
        app_gaf_ascc_cis_stream_stopped_t cis_stream_stopped;
        cis_stream_stopped.ase_lid = p_ase_info->ase_lid;
        cis_stream_stopped.cis_hdl = p_ase_info->cis_hdl;
        cis_stream_stopped.con_lid = p_ase_info->con_lid;
        cis_stream_stopped.direction = APP_GAF_DIRECTION_SRC;
        app_gaf_mobile_evt_report(APP_GAF_ASCC_CIS_STREAM_STOPPED_IND, (void *)&cis_stream_stopped,
                                  sizeof(app_gaf_ascc_cis_stream_stopped_t));
        _rx_dp_itf->cb_stop(p_ase_info->cis_hdl, 0);
    }
}

static uint8_t app_bap_uc_cli_qos_phy_judge(uint8_t phy_bf_in, uint8_t phy_bf_capa)
{
    if ((phy_bf_in & phy_bf_capa) & APP_PHY_2MBPS_BIT)
    {
        return APP_PHY_2MBPS_VALUE;
    }
    else if ((phy_bf_in & phy_bf_capa) & APP_PHY_1MBPS_BIT)
    {
        return APP_PHY_1MBPS_VALUE;
    }
    else if ((phy_bf_in & phy_bf_capa) & APP_PHY_CODED_BIT)
    {
        return APP_PHY_CODED_VALUE;
    }
#if mHDT_LE_SUPPORT
    else if ((phy_bf_in & phy_bf_capa) & APP_PHY_4MBPS_BIT)
    {
        return APP_PHY_4MBPS_VALUE;
    }
#endif

    return APP_PHY_2MBPS_VALUE;
}

static void app_bap_uc_cli_qos_req_parse(app_bap_ascc_ase_t *p_ase_info,
                                         const bap_uc_cli_ase_qos_req_t *qos_req)
{
    p_app_ascc_env->grp_info.grp_params.tlatency_m2s_ms = qos_req->max_trans_latency_ms;
    p_app_ascc_env->grp_info.grp_params.tlatency_s2m_ms = qos_req->max_trans_latency_ms;
    p_app_ascc_env->grp_info.grp_params.packing = APP_BAP_DFT_ASCC_PACKING_TYPE;
    p_app_ascc_env->grp_info.grp_params.framing = qos_req->framing;
    p_app_ascc_env->grp_info.grp_params.sca = APP_BAP_DFT_ASCC_SCA;
#ifdef BLE_AUDIO_IS_ALWAYS_USE_TEST_MODE_CIG_BIG_CREATING
    p_app_ascc_env->grp_info.grp_params.ft_m2s_ms = app_bap_get_cis_timing_config()->m2s_ft;
    p_app_ascc_env->grp_info.grp_params.ft_s2m_ms = app_bap_get_cis_timing_config()->s2m_ft;
    p_app_ascc_env->grp_info.grp_params.iso_intv_ms = app_bap_get_cis_timing_config()->iso_interval;
#endif
    p_ase_info->qos_cfg.phy = app_bap_uc_cli_qos_phy_judge(qos_req->pref_phy, p_app_ascc_env->phy_bf);
    p_ase_info->qos_cfg.retx_nb = qos_req->pref_rtn;
    p_ase_info->qos_cfg.pres_delay_us = qos_req->pres_delay_min_us;
    /// get codec cfg set location bf
    uint8_t allocation_cnt = app_bap_get_audio_location_l_r_cnt(p_ase_info->p_cfg->param.location_bf);
    if (ISO_UNFRAMED_MODE == qos_req->framing)
    {
#ifdef BLE_AUDIO_USE_ONE_CIS_FOR_DONGLE
        // Vendor specific cfg that BES set by ourself at pac add
        app_gaf_bap_vendor_specific_cfg_t vendor_specific_cfg;
        bool is_vend_cfg_set = false;
        uint8_t vendor_channel_num = 1;

        memset(&vendor_specific_cfg, 0, sizeof(app_gaf_bap_vendor_specific_cfg_t));
        is_vend_cfg_set = app_bap_get_specifc_ltv_data(&p_ase_info->p_cfg->add_cfg, 0xFF,
                                                       &vendor_specific_cfg);

        if (is_vend_cfg_set)
        {
            LOG_I("Vendor per channel frame_octet size:%d s2m_encode_channel %d s2m_decode_channel %d",
                  p_ase_info->p_cfg->param.frame_octet,
                  vendor_specific_cfg.s2m_encode_channel,
                  vendor_specific_cfg.s2m_decode_channel);
            // Calculate vendor specific channel num
            vendor_channel_num = (p_ase_info->direction == APP_GAF_DIRECTION_SRC) ?
                                 vendor_specific_cfg.s2m_encode_channel : vendor_specific_cfg.s2m_decode_channel;
        }
        p_ase_info->qos_cfg.max_sdu_size = p_ase_info->p_cfg->param.frame_octet * vendor_channel_num;
#endif
        p_ase_info->qos_cfg.max_sdu_size = p_ase_info->p_cfg->param.frame_octet * allocation_cnt;
    }
    else
    {
        p_ase_info->qos_cfg.max_sdu_size = p_ase_info->p_cfg->param.frame_octet * allocation_cnt + 5;
    }

    LOG_I("direction: %s, max sdu size:%d",
          (p_ase_info->direction == APP_GAF_DIRECTION_SRC) ? "SRC" : "SINK",
          p_ase_info->qos_cfg.max_sdu_size);
}

int app_bap_uc_cli_enable_stream_with_ccid(uint8_t ase_lid, uint16_t context_bf,
                                           uint8_t ccid_num, const uint8_t *p_ccid_list)
{
    app_bap_ascc_ase_t *p_ase_info = app_bap_uc_cli_get_ase_info_by_ase_lid(ase_lid);

    if (NULL != p_ase_info)
    {
        if (p_ase_info->p_metadata != NULL)
        {
            app_gaf_free_buff(p_ase_info->p_metadata);
        }

        p_ase_info->p_metadata = (app_gaf_bap_cfg_metadata_t *)app_gaf_malloc_buff(sizeof(app_gaf_bap_cfg_metadata_t));

        if (p_ase_info->p_metadata == NULL)
        {
            return BT_STS_NO_RESOURCES;
        }

        p_ase_info->p_metadata->param.context_bf = context_bf;
    }
    else
    {
        return BT_STS_INVALID_PARM;
    }

    bap_uc_cli_ase_op_param_t *p_ase_enable_param = (bap_uc_cli_ase_op_param_t *)app_gaf_malloc_buff(
                                                        sizeof(bap_uc_cli_ase_op_param_t) + sizeof(gen_aud_ltv_t) + ccid_num * sizeof(uint8_t));

    if (p_ase_enable_param == NULL)
    {
        return BT_STS_NO_RESOURCES;
    }

    gen_aud_init_metadata(&p_ase_enable_param->u_param.enable.metadata);

    p_ase_enable_param->u_param.enable.metadata.basic_metadata.streaming_audio_context = context_bf;

    if (ccid_num != 0)
    {
        p_ase_enable_param->u_param.enable.metadata.add_metadata.len = sizeof(gen_aud_ltv_t) + ccid_num * sizeof(uint8_t);
        gen_aud_ltv_t *p_ccid_list_ltv = (gen_aud_ltv_t *)p_ase_enable_param->u_param.enable.metadata.add_metadata.data;
        p_ccid_list_ltv->len = ccid_num + 1;
        p_ccid_list_ltv->type = GEN_AUD_METADATA_TYPE_CCID_LIST;
        memcpy(p_ccid_list_ltv->data, p_ccid_list, ccid_num * sizeof(uint8_t));
    }
    else
    {
        p_ase_enable_param->u_param.enable.metadata.add_metadata.len = 0;
    }

    p_ase_enable_param->con_lid = p_ase_info->con_lid;
    p_ase_enable_param->ase_id = p_ase_info->ase_instance_idx;

    uint16_t status =  bap_uc_cli_ase_operation(ASCS_OPCODE_ENABLE, p_ase_enable_param);

    app_gaf_free_buff(p_ase_enable_param);

    return status;
}

int app_bap_uc_cli_enable_stream(uint8_t ase_lid, uint16_t context_bf)
{
    return app_bap_uc_cli_enable_stream_with_ccid(ase_lid, context_bf, 0, NULL);
}

int app_bap_uc_cli_link_create_group_req(uint8_t cig_id)
{
    p_app_ascc_env->grp_info.cig_grp_lid = cig_id;

    bap_cig_param_t cig_param =
    {
        .framing = APP_ISO_UNFRAMED_MODE,
        .max_transport_latency_c2p_ms = p_app_ascc_env->grp_info.grp_params.tlatency_m2s_ms,
        .max_transport_latency_p2c_ms = p_app_ascc_env->grp_info.grp_params.tlatency_s2m_ms,
        .packing = APP_BAP_DFT_ASCC_PACKING_TYPE,
        .sdu_interval_c2p_us = p_app_ascc_env->grp_info.grp_params.sdu_intv_m2s_us,
        .sdu_interval_p2c_us = p_app_ascc_env->grp_info.grp_params.sdu_intv_s2m_us,
        .worst_case_sca = APP_BAP_DFT_ASCC_SCA,
        .test_iso_interval_1_25ms = 0,
    };

    const CIS_TIMING_CONFIGURATION_T *p_cis_timing = app_bap_get_cis_timing_config();

    if (p_cis_timing != NULL && p_cis_timing->iso_interval != 0)
    {
        cig_param.test_iso_interval_1_25ms = p_cis_timing->iso_interval;
        cig_param.test_ft_c2p = p_cis_timing->m2s_ft;
        cig_param.test_ft_p2c = p_cis_timing->s2m_ft;
    }

    uint16_t status = bap_uc_cli_create_uc_stream_group(cig_id,
                                                        p_app_ascc_env->grp_info.grp_params.cis_num, &cig_param);

    app_bap_ascc_grp_state_t cig_status =
    {
        .cig_grp_lid = cig_id,
        .isCreate = true,
        .status = status,
    };

    app_gaf_mobile_evt_report(APP_GAF_ASCC_CIS_GRP_STATE_IND, (void *)&cig_status,  sizeof(cig_status));

    return status;
}

int app_bap_uc_cli_link_remove_group_cmd(uint8_t grp_lid)
{
    p_app_ascc_env->grp_info.cig_grp_lid = APP_GAF_INVALID_ANY_LID;

    return bap_uc_cli_delete_uc_stream_group(grp_lid);
}

static void app_bap_uc_cli_stream_state_update(app_bap_ascc_ase_t *p_ase_info, uint8_t new_state,
                                               uint8_t old_state)
{
    LOG_I("%s %d, %d", __func__, p_ase_info->ase_lid, p_ase_info->con_lid);

    if (ASCS_ASE_STATE_STREAMING == new_state)
    {
        if (APP_GAF_DIRECTION_SINK == p_ase_info->direction)
        {
            app_bap_uc_cli_tx_stream_start(p_ase_info);
        }
        else if (APP_GAF_DIRECTION_SRC == p_ase_info->direction)
        {
            app_bap_uc_cli_rx_stream_start(p_ase_info);
        }
    }
    else if ((old_state == ASCS_ASE_STATE_STREAMING) &&
             ((ASCS_ASE_STATE_QOS_CONFIGURED == new_state) ||
              (ASCS_ASE_STATE_RELEASING == new_state) ||
              (ASCS_ASE_STATE_IDLE == new_state) ||
              (ASCS_ASE_STATE_DISABLING == new_state)))
    {
        if (APP_GAF_DIRECTION_SINK == p_ase_info->direction)
        {
            app_bap_uc_cli_tx_stream_stop(p_ase_info);
        }
        else if (APP_GAF_DIRECTION_SRC == p_ase_info->direction)
        {
            app_bap_uc_cli_rx_stream_stop(p_ase_info);
        }
    }
}

static void app_bap_uc_cli_update_ase_state(app_bap_ascc_ase_t *p_ase_info, uint8_t new_state,
                                            uint8_t old_state)
{
    if (new_state != old_state)
    {
        char *new_state_str;
        char *old_state_str;

        if (new_state >= ASCS_ASE_STATE_MAX)
        {
            new_state_str = ase_state_str[ASCS_ASE_STATE_MAX];
        }
        else
        {
            new_state_str = ase_state_str[new_state];
        }

        if (old_state >= ASCS_ASE_STATE_MAX)
        {
            old_state_str = ase_state_str[ASCS_ASE_STATE_MAX];
        }
        else
        {
            old_state_str = ase_state_str[old_state];
        }

        p_ase_info->ase_state = new_state;

        LOG_I("(d%d)app_bap uc_cli aseIdx:%d, ase_lid:%d, direction:%d",
              p_ase_info->con_lid, p_ase_info->ase_instance_idx, p_ase_info->ase_lid, p_ase_info->direction);
        LOG_I("ase state from %s to %s", old_state_str, new_state_str);

        app_gaf_cis_stream_state_updated_ind_t state_updated_ind;
        state_updated_ind.formerState = old_state;
        state_updated_ind.currentState = new_state;
        state_updated_ind.con_lid = p_ase_info->con_lid;
        state_updated_ind.ase_instance_idx = p_ase_info->ase_instance_idx;
        state_updated_ind.ase_lid = p_ase_info->ase_lid;

        app_gaf_mobile_evt_report(APP_GAF_ASCC_CIS_STREAM_STATE_UPDATED, (void *)&state_updated_ind,
                                  sizeof(state_updated_ind));
    }

    app_bap_uc_cli_stream_state_update(p_ase_info, new_state, old_state);
}

app_bap_ascc_ase_t *app_bap_uc_cli_get_ase_info_by_ase_lid(uint8_t ase_lid)
{
    if (ase_lid >= p_app_ascc_env->nb_ases_cfg)
    {
        LOG_W("%s: error ase_lid %d", __func__, ase_lid);
        return NULL;
    }

    app_bap_ascc_ase_t *p_ase_info = &p_app_ascc_env->ase_info[ase_lid];
    return p_ase_info;
}

uint8_t app_bap_uc_cli_get_ase_lid_list(uint8_t con_lid, uint8_t *ase_lid_list)
{
    uint8_t nb_ase = 0;

    for (uint8_t ase_lid = 0; ase_lid < p_app_ascc_env->nb_ases_cfg; ase_lid++)
    {
        app_bap_ascc_ase_t *p_ase_info = &p_app_ascc_env->ase_info[ase_lid];

        if (p_ase_info->con_lid == con_lid)
        {
            if (NULL != ase_lid_list)
            {
                ase_lid_list[nb_ase] = ase_lid;
            }

            nb_ase++;
        }
    }

    return nb_ase;
}

uint8_t app_bap_uc_cli_get_nb_ases_cfg()
{
    return p_app_ascc_env->nb_ases_cfg;
}

int app_bap_uc_cli_update_metadata(uint8_t ase_lid, app_gaf_bap_cfg_metadata_t *metadata)
{
    app_bap_ascc_ase_t *p_ase_info = app_bap_uc_cli_get_ase_info_by_ase_lid(ase_lid);

    if (NULL == p_ase_info || NULL == metadata)
    {
        LOG_E("%s ase_lid %d", __func__, ase_lid);
        return BT_STS_NOT_FOUND;
    }

    bap_uc_cli_ase_op_param_t *p_ase_upd_md_param = (bap_uc_cli_ase_op_param_t *)app_gaf_malloc_buff(
                                                        sizeof(bap_uc_cli_ase_op_param_t) + metadata->add_metadata.len);

    if (p_ase_upd_md_param == NULL)
    {
        return BT_STS_NO_RESOURCES;
    }

    gen_aud_init_metadata(&p_ase_upd_md_param->u_param.update_metadata.metadata);

    p_ase_upd_md_param->u_param.update_metadata.metadata.basic_metadata.streaming_audio_context =
        metadata->param.context_bf;
    p_ase_upd_md_param->u_param.update_metadata.metadata.add_metadata.len = metadata->add_metadata.len;

    memcpy(&p_ase_upd_md_param->u_param.update_metadata.metadata.add_metadata.data[0], metadata->add_metadata.data,
           metadata->add_metadata.len);

    p_ase_upd_md_param->con_lid = p_ase_info->con_lid;
    p_ase_upd_md_param->ase_id = p_ase_info->ase_instance_idx;

    uint16_t status = bap_uc_cli_ase_operation(ASCS_OPCODE_UPDATE_METADATA, p_ase_upd_md_param);

    LOG_I("%s status = %d", __func__, status);

    app_gaf_free_buff(p_ase_upd_md_param);

    return status;
}

int app_bap_uc_cli_discovery_start(uint8_t con_lid)
{
    LOG_D("app_bap uc_cli start discovery ASCS, con_lid = %d", con_lid);

    return bap_uc_cli_ascs_discovery(con_lid);
}

int app_bap_uc_cli_stream_disable(uint8_t ase_lid)
{
    app_bap_ascc_ase_t *p_ase_info = app_bap_uc_cli_get_ase_info_by_ase_lid(ase_lid);
    LOG_I("%s ase_lid %d %p %d", __func__, ase_lid,
          p_ase_info, p_ase_info->ase_state);

    if ((p_ase_info) && ((ASCS_ASE_STATE_STREAMING == p_ase_info->ase_state) ||
                         (ASCS_ASE_STATE_ENABLING == p_ase_info->ase_state)))
    {
        bap_uc_cli_ase_op_param_t ase_disable_param =
        {
            .con_lid = p_ase_info->con_lid,
            .ase_id = p_ase_info->ase_instance_idx,
        };

        return bap_uc_cli_ase_operation(ASCS_OPCODE_DISABLE, &ase_disable_param);
    }

    return BT_STS_INVALID_PARM;
}

/*Use to release sink media stream/sink call stream/src call stream*/
int app_bap_uc_cli_stream_release(uint8_t ase_lid)
{
    app_bap_ascc_ase_t *p_ase_info = app_bap_uc_cli_get_ase_info_by_ase_lid(ase_lid);

    if (p_ase_info == NULL)
    {
        return BT_STS_INVALID_PARM;
    }

    LOG_I("%s ase_lid %d direction %d ase_state %d", __func__, ase_lid,
          p_ase_info->direction, p_ase_info->ase_state);

    bap_uc_cli_ase_op_param_t ase_release_param =
    {
        .con_lid = p_ase_info->con_lid,
        .ase_id = p_ase_info->ase_instance_idx,
    };

    return bap_uc_cli_ase_operation(ASCS_OPCODE_RELEASE, &ase_release_param);
}

int app_bap_uc_cli_configure_qos(uint8_t ase_lid, uint8_t grp_lid, uint16_t max_sdu_size)
{
    app_bap_ascc_ase_t *p_ase_info = app_bap_uc_cli_get_ase_info_by_ase_lid(ase_lid);

    if (p_ase_info == NULL)
    {
        LOG_E("app_bap cli config qos get ase err:%d ", ase_lid);
        return BT_STS_NOT_FOUND;
    }

    if (max_sdu_size != 0)
    {
        LOG_I("app_bap cli config qos use external max sdu size: %d", max_sdu_size);
        p_ase_info->qos_cfg.max_sdu_size = max_sdu_size;
    }

    LOG_I("app_bap uc cli config qos grp id %d ase id %d cis id %d",
          p_app_ascc_env->grp_info.cig_grp_lid, p_ase_info->ase_lid, p_ase_info->cis_id);

    bap_uc_cli_ase_op_param_t *p_param = (bap_uc_cli_ase_op_param_t *)
                                         app_gaf_malloc_buff(sizeof(bap_uc_cli_ase_op_param_t));

    if (p_param == NULL)
    {
        LOG_E("%s no resources", __func__);
        return BT_STS_NO_RESOURCES;
    }

    memset(p_param, 0, sizeof(bap_uc_cli_ase_op_param_t));

    p_param->con_lid = p_ase_info->con_lid;
    p_param->ase_id = p_ase_info->ase_instance_idx;
    p_param->u_param.cfg_qos.grp_lid = grp_lid;
    p_param->u_param.cfg_qos.cis_id = p_ase_info->cis_id;
    p_param->u_param.cfg_qos.qos_cfg_param.framing = p_app_ascc_env->grp_info.grp_params.framing;
    p_param->u_param.cfg_qos.qos_cfg_param.max_sdu_size = p_ase_info->qos_cfg.max_sdu_size;
    p_param->u_param.cfg_qos.qos_cfg_param.phy = p_ase_info->qos_cfg.phy;
    p_param->u_param.cfg_qos.qos_cfg_param.pres_delay = p_ase_info->qos_cfg.pres_delay_us;
    p_param->u_param.cfg_qos.qos_cfg_param.rtn = p_ase_info->qos_cfg.retx_nb;

    /// Fill frame_dur with env set and pacs record
    if (p_ase_info->direction == APP_GAF_DIRECTION_SINK)
    {
        p_param->u_param.cfg_qos.qos_cfg_param.max_trans_latency =
            p_app_ascc_env->grp_info.grp_params.tlatency_m2s_ms;
        p_param->u_param.cfg_qos.qos_cfg_param.sdu_interval =
            p_app_ascc_env->grp_info.grp_params.sdu_intv_m2s_us;
    }
    else
    {
        p_param->u_param.cfg_qos.qos_cfg_param.max_trans_latency =
            p_app_ascc_env->grp_info.grp_params.tlatency_s2m_ms;
        p_param->u_param.cfg_qos.qos_cfg_param.sdu_interval =
            p_app_ascc_env->grp_info.grp_params.sdu_intv_s2m_us;
    }

    const CIS_TIMING_CONFIGURATION_T *p_cis_timing = app_bap_get_cis_timing_config();

    if (p_cis_timing != NULL && p_cis_timing->iso_interval != 0)
    {
        uint32_t max_pdu_size = (uint32_t)(((float)(p_ase_info->qos_cfg.max_sdu_size * 1250) / \
                                            p_param->u_param.cfg_qos.qos_cfg_param.sdu_interval) *\
                                           (p_cis_timing->iso_interval) / p_cis_timing->m2s_bn + 0.5f);

        LOG_I("app_bap uc cli config qos calculated max PDU size: %d", max_pdu_size);

        p_param->u_param.cfg_qos.cig_test_param.bn = p_cis_timing->m2s_bn;
        p_param->u_param.cfg_qos.cig_test_param.max_pdu_size = max_pdu_size;
        p_param->u_param.cfg_qos.cig_test_param.nse = p_cis_timing->m2s_nse;
    }

    uint16_t status = bap_uc_cli_ase_operation(ASCS_OPCODE_CFG_QOS, p_param);

    app_gaf_free_buff(p_param);

    return status;
}

int app_bap_uc_cli_configure_codec(uint8_t ase_lid, uint8_t cis_id,
                                   const app_gaf_codec_id_t *codec_id,
                                   uint8_t sampleRate, uint16_t frame_octet)
{
    app_bap_ascc_ase_t *p_ase_info = app_bap_uc_cli_get_ase_info_by_ase_lid(ase_lid);

    if ((p_ase_info) && (ASCS_ASE_STATE_QOS_CONFIGURED >= p_ase_info->ase_state))
    {
        LOG_I("Config codec, con_lid:%d, ase_instance_idx:%d", p_ase_info->con_lid,
              p_ase_info->ase_instance_idx);

        uint32_t codec_chn_support_cnt = 0;
        uint32_t peer_location_bf = 0;

        uint8_t con_lid = p_ase_info->con_lid;

        // Assign cis id for qos cfg use
        p_ase_info->cis_id = cis_id;

        // Get specific Pac Record
        app_bap_capa_record_cfg_t *record_cfg = app_bap_capa_cli_get_pac_record(con_lid,
                                                                                p_ase_info->direction,
                                                                                codec_id, sampleRate);

        if (NULL == record_cfg)
        {
            LOG_W("app_bap uc cli pick codec cfg failed");
            return BT_STS_FAILED;
        }

        codec_chn_support_cnt = app_bap_get_max_audio_channel_supp_cnt(
                                    record_cfg->p_cfg->param.location_bf);
        peer_location_bf = app_bap_capa_get_peer_audio_location_bf(con_lid, p_ase_info->direction);
        LOG_I("config codec, this codec record max_chn_cnt: %d channel cnt bf: %04x", codec_chn_support_cnt,
              record_cfg->p_cfg->param.location_bf);

        memcpy(&p_ase_info->codec_id, &record_cfg->codec_id, sizeof(app_gaf_codec_id_t));
        /// alloc mem for ase-bap_cfg
        p_ase_info->p_cfg = (app_gaf_bap_cfg_t *)app_gaf_malloc_buff(record_cfg->cfg_len);

        if (NULL == p_ase_info->p_cfg)
        {
            LOG_W("app_bap uc cli ase codec_cfg malloc failed");
            return BT_STS_NO_RESOURCES;
        }

        memcpy(p_ase_info->p_cfg, record_cfg->p_cfg, record_cfg->cfg_len);
        /// Replace ase-bap_cfg's oct with input param set
        p_ase_info->p_cfg->param.frame_octet = frame_octet;
        p_ase_info->p_cfg->param.sampling_freq = sampleRate;
        p_ase_info->p_cfg->param.frames_sdu = APP_BAP_DFT_ASCC_LC3_BLOCKS_PER_SDU;
        /// Fill frame_dur with env set and pacs record
        if (p_ase_info->direction == APP_GAF_DIRECTION_SINK)
        {
            p_ase_info->p_cfg->param.frame_dur = app_bap_frame_dur_us_to_frame_dur_enum(
                                                     p_app_ascc_env->grp_info.grp_params.sdu_intv_m2s_us /
                                                     p_ase_info->p_cfg->param.frames_sdu);
        }
        else
        {
            p_ase_info->p_cfg->param.frame_dur = app_bap_frame_dur_us_to_frame_dur_enum(
                                                     p_app_ascc_env->grp_info.grp_params.sdu_intv_s2m_us /
                                                     p_ase_info->p_cfg->param.frames_sdu);
        }

        // Only significant bit should set to 1, other bits set to 0
        /// TODO: Now we only want to support L And R not concern about location bf value
        bool stereo_chan_supp = app_bap_capa_cli_is_peer_support_stereo_channel(con_lid,
                                                                                p_ase_info->direction);
        /// BAP 4/5
        if (stereo_chan_supp && codec_chn_support_cnt == 2)
        {
            p_ase_info->p_cfg->param.location_bf = APP_GAF_BAP_AUDIO_LOCATION_FRONT_LEFT |
                                                   APP_GAF_BAP_AUDIO_LOCATION_FRONT_RIGHT;
        }
        // else codec_chn_support_cnt = 0 or 1, it means another ASE needed
        else if (stereo_chan_supp)
        {
            // choose any one ASE to set location bf
            if (p_ase_info->ase_instance_idx % 2)
            {
                p_ase_info->p_cfg->param.location_bf = APP_GAF_BAP_AUDIO_LOCATION_FRONT_LEFT;
            }
            else
            {
                p_ase_info->p_cfg->param.location_bf = APP_GAF_BAP_AUDIO_LOCATION_FRONT_RIGHT;
            }
        }
        else if (!stereo_chan_supp)
        {
            if ((peer_location_bf & (APP_GAF_BAP_AUDIO_LOCATION_SIDE_LEFT |
                                     APP_GAF_BAP_AUDIO_LOCATION_FRONT_LEFT)) != 0)
            {
                p_ase_info->p_cfg->param.location_bf = APP_GAF_BAP_AUDIO_LOCATION_FRONT_LEFT;
            }
            else
            {
                p_ase_info->p_cfg->param.location_bf = APP_GAF_BAP_AUDIO_LOCATION_FRONT_RIGHT;
            }
        }

        /***************************** Call ASCC directly *******************************/
        bap_uc_cli_ase_op_param_t *p_param =
            (bap_uc_cli_ase_op_param_t *)app_gaf_malloc_buff(sizeof(bap_uc_cli_ase_op_param_t) + p_ase_info->p_cfg->add_cfg.len);

        if (p_param == NULL)
        {
            return BT_STS_NO_RESOURCES;
        }

        p_param->con_lid = con_lid;
        p_param->ase_id = p_ase_info->ase_instance_idx;
        p_param->u_param.cfg_codec.tgt_phy = p_app_ascc_env->phy;
#ifdef LC3PLUS_SUPPORT
        /// LC3plus Reliable support first, @see LC3plus High Resolution Spec
        p_param->u_param.cfg_codec.tgt_latency = APP_GAF_BAP_UC_TGT_LATENCY_RELIABLE;
#else
        p_param->u_param.cfg_codec.tgt_latency = APP_GAF_BAP_UC_TGT_LATENCY_LOWER;
#endif
        memcpy(p_param->u_param.cfg_codec.codec_id, p_ase_info->codec_id.codec_id, sizeof(p_ase_info->codec_id.codec_id));
        // Codec Cfg
        p_param->u_param.cfg_codec.codec_cfg_param.basic_cc_param = *(gen_aud_codec_cfg_param_t *)(&p_ase_info->p_cfg->param);
        p_param->u_param.cfg_codec.codec_cfg_param.add_cc_param.len = p_ase_info->p_cfg->add_cfg.len;
        memcpy(p_param->u_param.cfg_codec.codec_cfg_param.add_cc_param.data,
               p_ase_info->p_cfg->add_cfg.data, p_ase_info->p_cfg->add_cfg.len);

        app_bap_cfg_print(p_ase_info->p_cfg);

        uint16_t status = bap_uc_cli_ase_operation(ASCS_OPCODE_CFG_CODEC, p_param);

        app_gaf_free_buff(p_param);

        return status;
    }

    return BT_STS_INVALID_PARM;
}

int app_bap_uc_cli_configure_codec_with_cfg(uint8_t ase_lid, uint8_t cis_id,
                                            const app_gaf_codec_id_t *codec_id,
                                            const app_gaf_bap_cfg_t *p_cfg)
{
    if (codec_id == NULL || p_cfg == NULL)
    {
        return BT_STS_INVALID_PARM;
    }

    app_bap_ascc_ase_t *p_ase_info = app_bap_uc_cli_get_ase_info_by_ase_lid(ase_lid);

    if ((p_ase_info) && (ASCS_ASE_STATE_QOS_CONFIGURED >= p_ase_info->ase_state))
    {
        LOG_I("Config codec with cfg, con_lid:%d, ase_instance_idx:%d", p_ase_info->con_lid,
              p_ase_info->ase_instance_idx);

        // Assign cis id for qos cfg use
        p_ase_info->cis_id = cis_id;
        memcpy(&p_ase_info->codec_id, codec_id, sizeof(app_gaf_codec_id_t));

        if (p_ase_info->p_cfg != NULL)
        {
            app_gaf_free_buff(p_ase_info->p_cfg);
        }

        /// alloc mem for ase-bap_cfg
        p_ase_info->p_cfg = (app_gaf_bap_cfg_t *)app_gaf_malloc_buff(sizeof(app_gaf_bap_cfg_t) + p_cfg->add_cfg.len);

        if (NULL == p_ase_info->p_cfg)
        {
            LOG_W("app_bap uc cli ase codec_cfg with cfg malloc failed");
            return BT_STS_NO_RESOURCES;
        }

        memcpy(p_ase_info->p_cfg, p_cfg, sizeof(app_gaf_bap_cfg_t) + p_cfg->add_cfg.len);

        /***************************** Call ASCC directly *******************************/
        bap_uc_cli_ase_op_param_t *p_param =
            (bap_uc_cli_ase_op_param_t *)app_gaf_malloc_buff(sizeof(bap_uc_cli_ase_op_param_t) + p_ase_info->p_cfg->add_cfg.len);

        if (p_param == NULL)
        {
            return BT_STS_NO_RESOURCES;
        }

        p_param->con_lid = p_ase_info->con_lid;
        p_param->ase_id = p_ase_info->ase_instance_idx;
        p_param->u_param.cfg_codec.tgt_phy = p_app_ascc_env->phy;
#ifdef LC3PLUS_SUPPORT
        /// LC3plus Reliable support first, @see LC3plus High Resolution Spec
        p_param->u_param.cfg_codec.tgt_latency = APP_GAF_BAP_UC_TGT_LATENCY_RELIABLE;
#else
        p_param->u_param.cfg_codec.tgt_latency = APP_GAF_BAP_UC_TGT_LATENCY_LOWER;
#endif
        memcpy(p_param->u_param.cfg_codec.codec_id, p_ase_info->codec_id.codec_id, sizeof(p_ase_info->codec_id.codec_id));
        // Codec Cfg
        p_param->u_param.cfg_codec.codec_cfg_param.basic_cc_param = *(gen_aud_codec_cfg_param_t *)(&p_ase_info->p_cfg->param);
        p_param->u_param.cfg_codec.codec_cfg_param.add_cc_param.len = p_ase_info->p_cfg->add_cfg.len;
        memcpy(p_param->u_param.cfg_codec.codec_cfg_param.add_cc_param.data,
               p_ase_info->p_cfg->add_cfg.data, p_ase_info->p_cfg->add_cfg.len);

        app_bap_cfg_print(p_ase_info->p_cfg);

        uint16_t status = bap_uc_cli_ase_operation(ASCS_OPCODE_CFG_CODEC, p_param);

        app_gaf_free_buff(p_param);

        return status;
    }

    return BT_STS_INVALID_PARM;
}

int app_bap_uc_cli_set_test_mode(bool enable)
{
    p_app_ascc_env->tm_enable = enable;

    return BT_STS_SUCCESS;
}

int app_bap_uc_cli_set_sdu_intv(uint32_t sdu_intv_m2s_us, uint32_t sdu_intv_s2m_us)
{
    LOG_I("app_bap uc_cli use sdu_intv m2s = %d, s2m = %d", sdu_intv_m2s_us, sdu_intv_s2m_us);
    p_app_ascc_env->grp_info.grp_params.sdu_intv_m2s_us = sdu_intv_m2s_us;
    p_app_ascc_env->grp_info.grp_params.sdu_intv_s2m_us = sdu_intv_s2m_us;

    return BT_STS_SUCCESS;
}

int app_bap_uc_cli_set_cis_num_in_cig(uint8_t cis_num)
{
    LOG_I("app_bap uc_cli use cis num in cig = %d", cis_num);
    p_app_ascc_env->grp_info.grp_params.cis_num = cis_num;

    return BT_STS_SUCCESS;
}

int app_bap_uc_cli_increase_cis_num_in_cig(uint8_t step)
{
    if (p_app_ascc_env->grp_info.grp_params.cis_num + step > APP_BAP_DFT_ASCC_CIS_NUM)
    {
        LOG_I("app_bap uc_cli increase cis num in cig arrive APP_BAP_DFT_ASCC_CIS_NUM");
        app_bap_uc_cli_set_cis_num_in_cig(APP_BAP_DFT_ASCC_CIS_NUM);

        return BT_STS_SUCCESS;
    }

    app_bap_uc_cli_set_cis_num_in_cig(p_app_ascc_env->grp_info.grp_params.cis_num + step);

    return BT_STS_SUCCESS;
}

static void app_bap_ascc_cb_discovery_cmp(uint8_t con_lid, uint16_t err_code)
{
    LOG_I("%s con_lid = %d, status = %d", __func__, con_lid, err_code);
}

static void app_bap_ascc_cb_bond_data(uint8_t con_lid, const bap_uc_cli_ascc_prf_svc_t *p_svc_info)
{
    LOG_D("app_bap uc_cli uc bond con_lid = %d, shdl = %d, ehdl = %d", con_lid,
          p_svc_info->svc_range.shdl, p_svc_info->svc_range.ehdl);
}

static void app_bap_ascc_cb_set_cfg_cmp_evt(uint8_t con_lid, uint8_t char_type, uint16_t err_code)
{
    LOG_I("%s, %d, %d, %d", __func__, con_lid, char_type, err_code);

    if (char_type != ASCS_CHAR_TYPE_CP)
    {
        app_gaf_uc_cli_bond_data_ind_t bond_data =
        {
            .con_lid = con_lid,
            .ascs_info.nb_ases_sink = char_type == ASCS_CHAR_TYPE_SINK_ASE,
            .ascs_info.nb_ases_src = char_type == ASCS_CHAR_TYPE_SRC_ASE,
        };

        app_gaf_mobile_evt_report(APP_GAF_ASCC_ASE_FOUND_IND, (void *)&bond_data, sizeof(bond_data));
    }
}

static void app_bap_ascc_cb_ase_err_state_ind(uint8_t con_lid, uint8_t ase_id, uint8_t ase_state,
                                              uint16_t err_code)
{
    LOG_E("ase err occured!!!, release all");

    if (!app_bap_uc_cli_is_already_bonded(con_lid))
    {
        LOG_W("%s the specified con[%d] has not beed bonded", __func__, con_lid);
        return;
    }

    for (uint8_t ase_lid = 0; ase_lid < p_app_ascc_env->nb_ases_cfg; ase_lid++)
    {
        app_bap_ascc_ase_t *p_ase_info = &p_app_ascc_env->ase_info[ase_lid];

        if (p_ase_info->con_lid == con_lid)
        {
            app_bap_uc_cli_stream_release(p_ase_info->ase_lid);
        }
    }
}

static void app_bap_ascc_cb_ase_cp_op_cmp_evt(uint8_t con_lid, uint8_t op_code, uint8_t ase_id,
                                              uint16_t err_code, uint8_t cp_rsp_code, uint8_t reason)
{
    LOG_I("uc_cli_cmp_evt_handler: con_lid = %d, opcode = %d status = %d, cp_rsp = %d,reason = %d, ase_id = %d",
          con_lid, op_code, err_code, cp_rsp_code, reason, ase_id);

    /* Report cmd cmp evt here if the command failed.
    *  otherwise report when the ASE state changes
    */
    uint8_t ase_lid = app_bap_uc_cli_get_exist_ase_lid_by_ase_instance_idx(con_lid, ase_id);

    if (err_code)
    {
        app_bap_ascc_cmd_cmp_t cmd_info;
        cmd_info.op_code = op_code;
        cmd_info.status = err_code;
        cmd_info.con_lid = con_lid;
        cmd_info.ase_lid = ase_lid;

        app_gaf_mobile_evt_report(APP_GAF_ASCC_CMD_CMP_IND, (void *)&cmd_info, sizeof(cmd_info));
    }
}

static void app_bap_ascc_cb_ase_idle_state_ind(uint8_t con_lid, uint8_t ase_id, uint8_t direction)
{
    uint8_t ase_lid = app_bap_uc_cli_get_exist_ase_lid_by_ase_instance_idx(con_lid, ase_id);
    app_bap_ascc_ase_t *p_ase_info = NULL;

    if (ASCS_INVALID_ASE_INDEX == ase_lid)
    {
        p_ase_info = app_bap_uc_cli_find_valid_ase_by_con_lid(con_lid);
        ase_lid = p_ase_info != NULL ? p_ase_info->ase_lid : ASCS_INVALID_ASE_INDEX;
    }

    if (ASCS_INVALID_ASE_INDEX != ase_lid)
    {
        p_ase_info = &p_app_ascc_env->ase_info[ase_lid];
        p_ase_info->con_lid = con_lid;
        p_ase_info->ase_instance_idx = ase_id;
    }
    else
    {
        LOG_E("%s cannot find valid con_lid = %d, ase id = %d, direction = %d", __func__, con_lid, ase_id,
              direction);
        return;
    }

    p_ase_info->direction = direction;

    uint8_t old_state = p_ase_info->ase_state;

    app_bap_uc_cli_update_ase_state(p_ase_info, ASCS_ASE_STATE_IDLE, old_state);

    if (old_state == ASCS_ASE_STATE_RELEASING)
    {
        app_bap_ascc_cmd_cmp_t cmd_info;
        cmd_info.op_code = ASCS_OPCODE_RELEASE;
        cmd_info.status = 0;
        cmd_info.con_lid = con_lid;
        cmd_info.ase_lid = p_ase_info->ase_lid;
        app_gaf_mobile_evt_report(APP_GAF_ASCC_CMD_CMP_IND, (void *)&cmd_info, sizeof(cmd_info));
    }

    // Free mem
    if (p_ase_info->p_cfg != NULL)
    {
        app_gaf_free_buff(p_ase_info->p_cfg);
        p_ase_info->p_cfg = NULL;
    }

    if (p_ase_info->p_cfg != NULL)
    {
        app_gaf_free_buff(p_ase_info->p_metadata);
        p_ase_info->p_cfg = NULL;
    }

    if (p_ase_info->p_metadata != NULL)
    {
        app_gaf_free_buff(p_ase_info->p_metadata);
        p_ase_info->p_metadata = NULL;
    }
}

static void app_bap_ascc_cb_ase_codec_cfg_state_ind(uint8_t con_lid, uint8_t ase_id,
                                                    const uint8_t *codec_id,
                                                    const gen_aud_cc_ptr_t *p_codec_cfg, const bap_uc_cli_ase_qos_req_t *p_qos_req)
{
    uint8_t ase_lid = app_bap_uc_cli_get_exist_ase_lid_by_ase_instance_idx(con_lid, ase_id);

    if (ase_lid == ASCS_INVALID_ASE_INDEX)
    {
        return;
    }

    app_bap_ascc_ase_t *p_ase_info = &p_app_ascc_env->ase_info[ase_lid];

    uint8_t old_state = p_ase_info->ase_state;
    // If uc srv restore ase state to codec cfg, so local value is invalid
    // TODO: maybe uc srv changes codec cfg params, so we need update it
    // ase stm also should update, when it is idle state, ase stm is NULL
    if (p_ase_info->p_cfg == NULL)
    {
        uint8_t add_cc_len = p_codec_cfg->add_cc_param_ptr.data != NULL ? p_codec_cfg->add_cc_param_ptr.len : 0;

        /// alloc mem for ase-bap_cfg
        p_ase_info->p_cfg = (app_gaf_bap_cfg_t *)app_gaf_malloc_buff(sizeof(*p_ase_info->p_cfg) + add_cc_len);

        if (NULL == p_ase_info->p_cfg)
        {
            LOG_W("app_bap uc cli ase state ind, codec cfg malloc failed");
            return;
        }

        memcpy(&p_ase_info->p_cfg->param, &p_codec_cfg->basic_cc_param,
               sizeof(p_codec_cfg->basic_cc_param));

        if (add_cc_len != 0)
        {
            p_ase_info->p_cfg->add_cfg.len = add_cc_len;
            memcpy(p_ase_info->p_cfg->add_cfg.data, p_codec_cfg->add_cc_param_ptr.data, add_cc_len);
        }
    }

    app_bap_uc_cli_qos_req_parse(p_ase_info, p_qos_req);

    app_bap_uc_cli_update_ase_state(p_ase_info, ASCS_ASE_STATE_CODEC_CONFIGURED, old_state);

    LOG_D("app_bap_uc_cli Codec Configure OK, ase_lid = %d, state = %d", p_ase_info->ase_lid,
          p_ase_info->ase_state);

    if (old_state <= ASCS_ASE_STATE_QOS_CONFIGURED)
    {
        app_bap_ascc_cmd_cmp_t cmd_info;
        cmd_info.op_code = ASCS_OPCODE_CFG_CODEC;
        cmd_info.status = 0;
        cmd_info.con_lid = p_ase_info->con_lid;
        cmd_info.ase_lid = p_ase_info->ase_lid;
        app_gaf_mobile_evt_report(APP_GAF_ASCC_CMD_CMP_IND, (void *)&cmd_info, sizeof(cmd_info));
    }
}

static void app_bap_ascc_cb_ase_qos_cfg_state_ind(uint8_t con_lid, uint8_t ase_id, uint8_t cig_id,
                                                  uint8_t cis_id, const bap_uc_cli_ase_qos_cfg_t *p_qos_cfg)
{
    uint8_t ase_lid = app_bap_uc_cli_get_exist_ase_lid_by_ase_instance_idx(con_lid, ase_id);

    if (ase_lid == ASCS_INVALID_ASE_INDEX)
    {
        return;
    }

    app_bap_ascc_ase_t *p_ase_info = &p_app_ascc_env->ase_info[ase_lid];

    uint8_t old_state = p_ase_info->ase_state;

    app_bap_uc_cli_update_ase_state(p_ase_info, ASCS_ASE_STATE_QOS_CONFIGURED, old_state);

    LOG_D("app_bap_uc_cli QOS Configure OK, ase_lid = %d, state = %d", p_ase_info->ase_lid,
          p_ase_info->ase_state);

    uint8_t opcode = 0;

    if (old_state == ASCS_ASE_STATE_CODEC_CONFIGURED)
    {
        opcode = ASCS_OPCODE_CFG_QOS;
    }
    else if (old_state == ASCS_ASE_STATE_STREAMING || old_state == ASCS_ASE_STATE_DISABLING)
    {
        opcode = ASCS_OPCODE_DISABLE;
    }
    else
    {
        return;
    }

    p_ase_info->cis_id = cis_id;

    app_bap_ascc_cmd_cmp_t cmd_info;
    cmd_info.op_code = opcode;
    cmd_info.status = 0;
    cmd_info.con_lid = p_ase_info->con_lid;
    cmd_info.ase_lid = p_ase_info->ase_lid;
    app_gaf_mobile_evt_report(APP_GAF_ASCC_CMD_CMP_IND, (void *)&cmd_info, sizeof(cmd_info));
}

static void app_bap_ascc_cb_ase_state_metadata_ind(uint8_t con_lid, uint8_t ase_id,
                                                   uint8_t ase_state,
                                                   const gen_aud_metadata_t *p_metadata)
{
    uint8_t ase_lid = app_bap_uc_cli_get_exist_ase_lid_by_ase_instance_idx(con_lid, ase_id);

    if (ase_lid == ASCS_INVALID_ASE_INDEX)
    {
        return;
    }

    app_bap_ascc_ase_t *p_ase_info = &p_app_ascc_env->ase_info[ase_lid];

    uint8_t old_state = p_ase_info->ase_state;

    app_bap_uc_cli_update_ase_state(p_ase_info, ase_state, old_state);

    if ((old_state == ASCS_ASE_STATE_ENABLING) &&
            (ase_state == ASCS_ASE_STATE_STREAMING))
    {
        LOG_I("app_bap_uc_cli ENABLE OK");
        app_bap_ascc_cmd_cmp_t cmd_info;
        cmd_info.op_code = ASCS_OPCODE_ENABLE;
        cmd_info.status = 0;
        cmd_info.con_lid = p_ase_info->con_lid;
        cmd_info.ase_lid = p_ase_info->ase_lid;
        app_gaf_mobile_evt_report(APP_GAF_ASCC_CMD_CMP_IND, (void *)&cmd_info, sizeof(cmd_info));
    }
}

static void app_bap_ascc_cb_cig_status_evt(uint8_t cig_evt, uint8_t grp_lid, uint8_t cig_id,
                                           uint16_t err_code)
{
    if (cig_evt == BAP_UC_CLI_CIG_EVENT_REMOVED)
    {
        app_bap_uc_cli_set_cis_num_in_cig(0);
    }

    LOG_D("app_bap uc_cli cig %s OK, grp_lid = %d, cig_id = %d",
          cig_evt == BAP_UC_CLI_CIG_EVENT_CREATED ? "created" : "removed", grp_lid, cig_id);

    if (cig_evt == BAP_UC_CLI_CIG_EVENT_REMOVED)
    {
        app_bap_ascc_grp_state_t cig_status =
        {
            .cig_grp_lid = grp_lid,
            .isCreate = false,
            .status = err_code,
        };

        app_gaf_mobile_evt_report(APP_GAF_ASCC_CIS_GRP_STATE_IND, (void *)&cig_status,  sizeof(cig_status));
    }
}

static void app_bap_ascc_cb_cis_status_evt(enum bap_uc_cli_cis_event cis_evt, uint8_t cig_id,
                                           uint8_t cis_id, uint16_t cis_hdl,
                                           const bap_cis_timing_t *p_cis_timing, uint16_t err_code)
{
    LOG_I("cis_status = %d, cis_hdl = 0x%x, cig_id = %d, cis_id = %d", err_code, cis_hdl, cig_id,
          cis_id);

    uint8_t ase_lid_list[BAP_DIRECTION_MAX] = {0};

    uint8_t ase_num = app_bap_uc_cli_get_exist_ase_lid_list_by_cis_id(cig_id, cis_id, ase_lid_list);

    uint8_t idx = 0;

    app_bap_ascc_ase_t *p_ase_info = NULL;

    for (idx = 0; idx < ase_num; idx++)
    {
        p_ase_info = &p_app_ascc_env->ase_info[ase_lid_list[idx]];

        LOG_I("%s ase_lid_%s = %d", __func__,
              (p_ase_info->direction == (uint8_t)BAP_DIRECTION_SINK ? "sink" : "src"), p_ase_info->ase_lid);

        if (cis_evt == BAP_UC_CLI_CIS_EVENT_CONNECTED)
        {
            p_ase_info->cis_hdl = cis_hdl;
            p_ase_info->cig_sync_delay = p_cis_timing->cig_sync_delay_us;
            p_ase_info->cis_sync_delay = p_cis_timing->cis_sync_delay_us;
            p_ase_info->bn_m2s = p_cis_timing->bn_c2p;
            p_ase_info->bn_s2m = p_cis_timing->bn_p2c;
            p_ase_info->iso_interval_us = p_cis_timing->iso_interval_1_25ms * 1250;

            app_gaf_uc_cli_cis_state_ind_t cis_status =
            {
                .con_lid = p_ase_info->con_lid,
                .cis_id = cis_id,
                .grp_lid = cig_id,
                .conhdl = cis_hdl,
                .cig_config.iso_intv_frames = p_cis_timing->iso_interval_1_25ms,
                .cig_config.sync_delay_us = p_cis_timing->cig_sync_delay_us,
                .cig_config.tlatency_m2s_us = p_cis_timing->transport_latency_c2p_us,
                .cig_config.tlatency_s2m_us = p_cis_timing->transport_latency_p2c_us,
                .cis_config.bn_m2s = p_cis_timing->bn_c2p,
                .cis_config.bn_s2m = p_cis_timing->bn_p2c,
                .cis_config.ft_m2s = p_cis_timing->ft_c2p,
                .cis_config.ft_s2m = p_cis_timing->ft_p2c,
                .cis_config.nse = p_cis_timing->nse,
            };

            app_gaf_mobile_evt_report(APP_GAF_ASCC_CIS_ESTABLISHED_IND, (void *)&cis_status,
                                      sizeof(cis_status));
        }
        else if (cis_evt == BAP_UC_CLI_CIS_EVENT_DISCONNECTED)
        {
            p_ase_info->cis_hdl = ASCS_INVALID_CIS_HDL;

            app_gaf_uc_cli_cis_state_ind_t cis_status =
            {
                .con_lid = p_ase_info->con_lid,
                .cis_id = cis_id,
                .grp_lid = cig_id,
                .conhdl = cis_hdl,
            };

            app_gaf_mobile_evt_report(APP_GAF_ASCC_CIS_DISCONNETED_IND, (void *)&cis_status,
                                      sizeof(cis_status));
        }
        else
        {
            /// TODO: Rejected
        }
    }
}

static void app_bap_ascc_cb_iso_dp_status_evt(bool is_setup, uint8_t con_lid, uint8_t ase_id,
                                              uint16_t err_code)
{
    LOG_I("iso_dp_is_setup = %d, con_lid = %d, ase_id = %d, status = %d", is_setup, con_lid, ase_id,
          err_code);
}

static void app_bap_ascc_cleanup_ase_found(uint8_t con_lid)
{
    uint8_t ase_lid = 0;

    for (ase_lid = 0; ase_lid < p_app_ascc_env->nb_ases_cfg; ase_lid++)
    {
        app_bap_ascc_ase_t *p_ase_info = &p_app_ascc_env->ase_info[ase_lid];

        if (p_ase_info->con_lid == con_lid)
        {
            p_ase_info->ase_state = ASCS_ASE_STATE_MAX;
        }
    }
}

static void app_bap_ascc_cb_prf_status_event(uint8_t con_lid, bool is_central, gatt_prf_status_event_e event)
{
    // Cleanup ASE record
    if (event == GATT_PRF_STATUS_EVENT_CLOSED && is_central)
    {
        LOG_I("uc cli disconnected with con_lid = %d", con_lid);
        // We clear ase info after all things done
        bt_thread_call_func_1(app_bap_ascc_cleanup_ase_found, bt_fixed_param(con_lid));
    }
    else if (event == GATT_PRF_STATUS_EVENT_OPENED && is_central == true)
    {
        //bap_uc_cli_ascs_discovery(con_lid);
    }
}

static const bap_uc_cli_ascc_evt_cb_t ascc_evt_cb =
{
    .cb_discovery_cmp = app_bap_ascc_cb_discovery_cmp,
    .cb_bond_data = app_bap_ascc_cb_bond_data,
    .cb_set_cfg_cmp = app_bap_ascc_cb_set_cfg_cmp_evt,
    .cb_ase_state_err = app_bap_ascc_cb_ase_err_state_ind,
    .cb_ase_cp_op_cmp = app_bap_ascc_cb_ase_cp_op_cmp_evt,
    .cb_ase_idle = app_bap_ascc_cb_ase_idle_state_ind,
    .cb_ase_codec_cfg = app_bap_ascc_cb_ase_codec_cfg_state_ind,
    .cb_ase_qos_cfg = app_bap_ascc_cb_ase_qos_cfg_state_ind,
    .cb_ase_metadata = app_bap_ascc_cb_ase_state_metadata_ind,
    .cb_cig_status = app_bap_ascc_cb_cig_status_evt,
    .cb_cis_status = app_bap_ascc_cb_cis_status_evt,
    .cb_iso_dp_status = app_bap_ascc_cb_iso_dp_status_evt,
    .cb_prf_status_event = app_bap_ascc_cb_prf_status_event,
};

int app_bap_uc_cli_init(void)
{
    LOG_I("%s", __func__);

    bap_uc_cli_ascc_init_cfg_t ascc_init_cfg =
    {
        .cp_write_reliable = true,
        .auto_cis_disconn = true,
        .ase_num_supp = APP_BAP_DFT_ASCC_NB_ASE_CFG,
    };

    return bap_uc_cli_ascc_init(&ascc_init_cfg, &ascc_evt_cb);
}

int app_bap_uc_cli_deinit(void)
{
    LOG_I("%s", __func__);

    return bap_uc_cli_ascc_deinit();
}

int app_bap_uc_cli_info_init(void)
{
    LOG_I("%s", __func__);

    if (p_app_ascc_env != NULL)
    {
        return BT_STS_SUCCESS;
    }

    uint16_t size = sizeof(app_bap_ascc_env_t) + APP_BAP_DFT_ASCC_TOTAL_NB_ASE_CFG * sizeof(
                        app_bap_ascc_ase_t);
    p_app_ascc_env = (app_bap_ascc_env_t *)app_gaf_malloc_buff(size);
    if (NULL == p_app_ascc_env)
    {
        LOG_E("app_bap uc cli env init malloc error");
        return BT_STS_NO_RESOURCES;
    }

    memset(p_app_ascc_env, 0, size);
    p_app_ascc_env->preferred_mtu = APP_GAF_DFT_PREF_MTU;
    p_app_ascc_env->nb_ases_cfg = APP_BAP_DFT_ASCC_TOTAL_NB_ASE_CFG;
    p_app_ascc_env->phy_bf = APP_BAP_DFT_ASCC_SUPP_PHY_BF;
    p_app_ascc_env->phy = APP_BAP_DFT_ASCC_PRFE_PHY;
    p_app_ascc_env->grp_info.cig_grp_lid = APP_GAF_INVALID_CON_LID;
    p_app_ascc_env->grp_info.grp_params.sdu_intv_m2s_us = APP_BAP_DFT_ASCC_SDU_INTERVAL_US;
    p_app_ascc_env->grp_info.grp_params.sdu_intv_s2m_us = APP_BAP_DFT_ASCC_SDU_INTERVAL_US;
    p_app_ascc_env->grp_info.grp_params.cis_num = APP_BAP_DFT_ASCC_CIS_NUM;

    p_app_ascc_env->ase_info = (app_bap_ascc_ase_t *)((uint32_t)p_app_ascc_env + sizeof(
                                                          app_bap_ascc_env_t));

    for (uint8_t ase_lid = 0; ase_lid < p_app_ascc_env->nb_ases_cfg; ase_lid++)
    {
        app_bap_ascc_ase_t *p_ase = &p_app_ascc_env->ase_info[ase_lid];
        p_ase->con_lid = (ase_lid / APP_BAP_DFT_ASCC_NB_ASE_CFG);
        p_ase->ase_state = ASCS_ASE_STATE_MAX;
        p_ase->direction = APP_GAF_DIRECTION_MAX;
        p_ase->cis_id = GATT_INVALID_IDX;
        p_ase->cis_hdl = GATT_INVALID_HDL;
        p_ase->ase_instance_idx = (ase_lid % APP_BAP_DFT_ASCC_NB_ASE_CFG) + ASCS_ASE_ID_MIN;
        p_ase->ase_lid = ase_lid;
    }

    return BT_STS_SUCCESS;
}

int app_bap_uc_cli_info_deinit(void)
{
    LOG_I("%s", __func__);

    if (p_app_ascc_env == NULL)
    {
        return BT_STS_SUCCESS;
    }

    for (uint8_t ase_lid = 0; ase_lid < p_app_ascc_env->nb_ases_cfg; ase_lid++)
    {
        app_bap_ascc_ase_t *p_ase_info = &p_app_ascc_env->ase_info[ase_lid];
        // Free mem
        if (p_ase_info->p_cfg != NULL)
        {
            app_gaf_free_buff(p_ase_info->p_cfg);
        }

        if (p_ase_info->p_cfg != NULL)
        {
            app_gaf_free_buff(p_ase_info->p_metadata);
        }

        if (p_ase_info->p_metadata != NULL)
        {
            app_gaf_free_buff(p_ase_info->p_metadata);
        }
    }

    app_gaf_free_buff(p_app_ascc_env);

    p_app_ascc_env = NULL;

    return BT_STS_SUCCESS;
}

#if 0
int app_bap_uc_cli_check_disonnect_cis(uint8_t ase_lid)
{
    app_bap_ascc_ase_t *p_ase_info = &p_app_ascc_env->ase_info[ase_lid];

    if (p_ase_info == NULL)
    {
        return BT_STS_NOT_FOUND;
    }

    return bap_uc_cli_disconnect_ase_cis(p_ase_info->con_lid, p_ase_info->ase_instance_idx);
}
#endif

#endif
#endif

/// @} APP
