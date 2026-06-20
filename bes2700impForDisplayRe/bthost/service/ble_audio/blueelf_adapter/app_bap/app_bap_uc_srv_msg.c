/***************************************************************************
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
 ****************************************************************************/
#if BLE_AUDIO_ENABLED
/*****************************header include********************************/
#include "ble_app_dbg.h"
#include "app_acc.h"
#include "ble_audio_core_evt.h"
#include "app_bap.h"
#include "isoohci_int.h"

#include "app_gaf.h"
#include "app_bap_uc_srv_msg.h"
#include "app_bap_data_path_itf.h"
#include "app_gaf_custom_api.h"

#include "bap_unicast_server.h"
/*
 * DEFINES
 ****************************************************************************************
 */
#define APP_BAP_DFT_ASCS_DP_ID                    (0)//HCI
#define APP_BAP_DFT_ASCS_CTL_DELAY_US             (GAF_CONTROLLER_DELAY_US)

/// Audio Stream Control Service Server environment structure
typedef struct app_bap_ascs_env
{
    /// test mode enable flag
    bool tm_enable;
    /// ASCS ASE Information
    app_bap_ascs_ase_t *ase_info;
} app_bap_ascs_env_t;

/************************private variable defination************************/
static app_bap_ascs_env_t *p_app_ascs_env = NULL;

static app_bap_uc_iso_tx_sync_dbg_info_t app_uc_sr_iso_tx_sync_dbg_info;

/************************external declaration******************************/
extern char *ase_state_str[ASCS_ASE_STATE_MAX + 1];

/************************functions definition******************************/
static inline app_bap_ascs_ase_t *app_bap_uc_srv_get_ase_by_ase_lid(uint8_t ase_lid)
{
    if (p_app_ascs_env == NULL)
    {
        return NULL;
    }

    if (ase_lid >= APP_BAP_DFT_ASCS_NB_ASE_CFG)
    {
        LOG_W("app_bap uc srv error ase_lid %d", ase_lid);
        return NULL;
    }

    return &p_app_ascs_env->ase_info[ase_lid];
}

static inline app_bap_ascs_ase_t *app_bap_uc_srv_get_ase_info_by_ase_id(uint8_t con_lid, uint8_t ase_id)
{
    if (con_lid >= BLE_CONNECTION_MAX || ase_id > APP_BAP_DFT_ASCS_NB_ASE_CHAR || ase_id < ASCS_ASE_ID_MIN)
    {
        return NULL;
    }

    uint8_t ase_lid = con_lid * APP_BAP_DFT_ASCS_NB_ASE_CHAR + ase_id - ASCS_ASE_ID_MIN;

    return app_bap_uc_srv_get_ase_by_ase_lid(ase_lid);
}

static int app_bap_uc_srv_tx_stream_start(app_bap_ascs_ase_t *p_ase_info)
{
    uint8_t status = BT_STS_SUCCESS;
    const struct data_path_itf *_tx_dp_itf = NULL;

    LOG_I("app_bap uc_srv start tx stream, cis_hdl = %d", p_ase_info->cis_hdl);
    _tx_dp_itf = data_path_itf_get(ISO_DP_ISOOHCI, ISO_SEL_TX);

    if (_tx_dp_itf == NULL)
    {
        LOG_W("app_bap uc_srv get tx data path interface failed");
        _tx_dp_itf = data_path_itf_get(ISO_DP_DISABLE, ISO_SEL_TX);
    }

    app_bap_set_tx_dp_itf((struct data_path_itf *)_tx_dp_itf);

    if (_tx_dp_itf && (NULL != _tx_dp_itf->cb_start))
    {
        uint32_t sdu_interval = p_ase_info->qos_cfg.sdu_intv_us;
        uint32_t trans_latency = p_ase_info->qos_cfg.trans_latency_max_ms;

        uint16_t max_sdu = p_ase_info->qos_cfg.max_sdu_size;
        status = _tx_dp_itf->cb_start(p_ase_info->cis_hdl, sdu_interval, trans_latency * 1000 + 60000, max_sdu);
        if (status != BT_STS_SUCCESS)
        {
            LOG_W("app_bap uc_srv start tx stream failed");
        }
    }

    return status;
}

static int app_bap_uc_srv_rx_stream_start(app_bap_ascs_ase_t *p_ase_info)
{
    uint8_t status = BT_STS_SUCCESS;
    const struct data_path_itf *_rx_dp_itf = NULL;

    LOG_I("app_bap uc_srv start rx stream, cis_hdl = %d", p_ase_info->cis_hdl);
    _rx_dp_itf = data_path_itf_get(ISO_DP_ISOOHCI, ISO_SEL_RX);

    if (_rx_dp_itf == NULL)
    {
        LOG_W("get rx data path interface failed");
        _rx_dp_itf = data_path_itf_get(ISO_DP_DISABLE, ISO_SEL_RX);
    }

    app_bap_set_rx_dp_itf((struct data_path_itf *)_rx_dp_itf);

    if (NULL != _rx_dp_itf->cb_start)
    {
        // TODO: qos config should be saved in application layer ase info
        uint32_t sdu_interval = p_ase_info->qos_cfg.sdu_intv_us;
        uint32_t trans_latency = p_ase_info->qos_cfg.trans_latency_max_ms;
        uint16_t max_sdu = p_ase_info->qos_cfg.max_sdu_size;
        status = _rx_dp_itf->cb_start(p_ase_info->cis_hdl, sdu_interval, trans_latency * 1000, max_sdu);

        if (status != BT_STS_SUCCESS)
        {
            LOG_W("app_bap uc_srv start rx stream failed");
        }
    }

    return status;
}

static void app_bap_uc_srv_update_ase_state(app_bap_ascs_ase_t *p_ase_info, uint8_t new_state,
                                            uint8_t old_state)
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

    app_gaf_cis_stream_state_updated_ind_t state_updated_ind;
    state_updated_ind.formerState = old_state;
    state_updated_ind.currentState = new_state;
    state_updated_ind.con_lid = p_ase_info->con_lid;
    state_updated_ind.ase_lid = p_ase_info->ase_lid;
    state_updated_ind.ase_instance_idx = p_ase_info->ase_instance_idx;

    app_gaf_evt_report(APP_GAF_ASCS_CLI_STREAM_STATE_UPDATED, (void *)&state_updated_ind,
                       sizeof(state_updated_ind));

    LOG_I("%s con_lid:%d ase_lid:%d cisHdl:%d direction %d", __func__,
          p_ase_info->con_lid, p_ase_info->ase_lid, p_ase_info->cis_hdl, p_ase_info->direction);
    LOG_I("ase state from %s to %s", old_state_str, new_state_str);
    LOG_I("ase cig delay %d cis delay %d", p_ase_info->cig_sync_delay, p_ase_info->cis_sync_delay);
}

app_bap_uc_iso_tx_sync_dbg_info_t *app_bap_uc_srv_iso_tx_sync_dbg_info_get(void)
{
    return &app_uc_sr_iso_tx_sync_dbg_info;
}

POSSIBLY_UNUSED static int app_bap_uc_srv_stop_iso(app_bap_ascs_ase_t *p_ase_info, uint16_t cisHdl)
{
    if (APP_GAF_DIRECTION_SINK == p_ase_info->direction)
    {
        struct data_path_itf *_rx_dp_itf = app_bap_get_rx_dp_itf();
        if (_rx_dp_itf)
        {
            _rx_dp_itf->cb_stop(cisHdl, 0);
        }
    }
    else if (APP_GAF_DIRECTION_SRC == p_ase_info->direction)
    {
        struct data_path_itf *_tx_dp_itf = app_bap_get_tx_dp_itf();
        if (_tx_dp_itf)
        {
            _tx_dp_itf->cb_stop(cisHdl, 0);
        }
    }

    return BT_STS_SUCCESS;
}

int app_bap_uc_srv_restore_bond_data_req(uint8_t con_lid, uint8_t cli_cfg_bf, uint8_t ase_cli_cfg_bf)
{
    return bap_uc_srv_restore_ascs_cli_cfg_cache(con_lid, ((uint32_t)(cli_cfg_bf & 0b1) | (ase_cli_cfg_bf << 1)));
}

int app_bap_uc_srv_configure_codec_ase_local(uint8_t ase_lid,
                                             const app_gaf_codec_id_t *p_codec_id,
                                             const app_gaf_bap_qos_req_t *p_qos_req,
                                             const app_gaf_bap_cfg_t *ntf_codec_cfg)
{
    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info(ase_lid);

    if (p_ase_info == NULL || p_qos_req == NULL || ntf_codec_cfg == NULL || p_codec_id == NULL)
    {
        return BT_STS_INVALID_PARM;
    }

    bap_uc_srv_codec_cfg_qos_req_t *p_qos_req_param =
        (bap_uc_srv_codec_cfg_qos_req_t *)app_gaf_malloc_buff(sizeof(bap_uc_srv_codec_cfg_qos_req_t));

    if (p_qos_req_param == NULL)
    {
        LOG_E("%s no resources", __func__);
        return BT_STS_NO_RESOURCES;
    }

    p_qos_req_param->framing_type = p_qos_req->framing;
    p_qos_req_param->max_trans_latency_ms = p_qos_req->trans_latency_max_ms;
    p_qos_req_param->phy_bf = p_qos_req->phy_bf;
    p_qos_req_param->rtn = p_qos_req->retx_nb;
    p_qos_req_param->prefer_pd_max_us = p_qos_req->pref_pres_delay_max_us;
    p_qos_req_param->prefer_pd_min_us = p_qos_req->pref_pres_delay_min_us;
    p_qos_req_param->pres_delay_max_us = p_qos_req->pres_delay_max_us;
    p_qos_req_param->pres_delay_min_us = p_qos_req->pres_delay_min_us;

    uint16_t status = bap_uc_srv_configure_codec_ase_local(p_ase_info->con_lid, p_ase_info->ase_instance_idx,
                                                           p_codec_id->codec_id, (gen_aud_cc_t *)ntf_codec_cfg, p_qos_req_param);

    if (status == BT_STS_SUCCESS)
    {
        // Assign codec id
        p_ase_info->codec_id = *p_codec_id;

        if (p_ase_info->p_cfg != NULL && p_ase_info->p_cfg->add_cfg.len != ntf_codec_cfg->add_cfg.len)
        {
            app_gaf_free_buff(p_ase_info->p_cfg);
            p_ase_info->p_cfg = NULL;
        }

        if (p_ase_info->p_cfg == NULL)
        {
            p_ase_info->p_cfg = (app_gaf_bap_cfg_t *)app_gaf_malloc_buff(sizeof(app_gaf_bap_cfg_t) + ntf_codec_cfg->add_cfg.len);

            if (p_ase_info->p_cfg == NULL)
            {
                return BT_STS_NO_RESOURCES;
            }
        }

        // Assign codec cfg parameters
        memcpy(p_ase_info->p_cfg, ntf_codec_cfg, sizeof(app_gaf_bap_cfg_t) + ntf_codec_cfg->add_cfg.len);
        // Assign qos req parameters
        p_ase_info->qos_req = *p_qos_req;
    }

    return status;
}

int app_bap_uc_srv_update_metadata_req(uint8_t ase_lid, app_gaf_bap_cfg_metadata_t *metadata)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

static void app_bap_ascs_cb_bond_data(uint8_t con_lid, uint8_t char_type, uint16_t cli_cfg_bf)
{
    LOG_D("app_bap ascs bond_data_ind con_lid = %d, char_type = %d, cli_cfg_bf = %02x", con_lid,
          char_type, cli_cfg_bf);

    app_gaf_bap_uc_srv_bond_data_ind_t bond_data =
    {
        .con_lid = con_lid,
        .char_type = char_type,
        .cli_cfg_bf = cli_cfg_bf & 0b1,
        .ase_cli_cfg_bf = (cli_cfg_bf & ~0b1) >> 1,
    };

    app_gaf_evt_report(APP_GAF_ASCS_BOND_DATA_IND, (void *)&bond_data, sizeof(bond_data));
}

static void app_bap_ascs_cb_ase_codec_cfg_req(uint8_t con_lid, uint8_t ase_num,
                                              const bap_uc_srv_codec_cfg_req_t *p_codec_cfg_req)
{
    if (p_codec_cfg_req == NULL)
    {
        LOG_W("%s codec cfg req is NULL!", __func__);
        return;
    }

    uint8_t  i = 0;

    for (i = 0; i < ase_num; i++)
    {
        app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info_by_ase_id(con_lid, p_codec_cfg_req[i].ase_id);

        if (NULL == p_ase_info)
        {
            LOG_W("%s p_ase_info find error!", __func__);
            return;
        }

        uint8_t add_cc_len = p_codec_cfg_req[i].codec_cfg.add_cc_param_ptr.data == NULL ? 0 :
                             p_codec_cfg_req[i].codec_cfg.add_cc_param_ptr.len;

        app_gaf_uc_srv_configure_codec_req_ind_t *p_req = (app_gaf_uc_srv_configure_codec_req_ind_t *)
                                                          app_gaf_malloc_buff(sizeof(*p_req) + add_cc_len);

        if (p_req == NULL)
        {
            LOG_W("%s codec cfg req evt malloc failed!", __func__);
            return;
        }

        memset(p_req, 0, sizeof(*p_req) + add_cc_len);

        p_req->tgt_latency = p_codec_cfg_req[i].latency;
        p_req->tgt_phy = p_codec_cfg_req[i].phy;
        p_req->ase_instance_idx = p_codec_cfg_req[i].ase_id;
        memcpy(p_req->codec_id.codec_id, p_codec_cfg_req[i].codec_id, GEN_AUD_CODEC_ID_LEN);
        memcpy(&p_req->cfg.param, &p_codec_cfg_req[i].codec_cfg.basic_cc_param,
               sizeof(app_gaf_bap_cfg_param_t));
        if (add_cc_len != 0)
        {
            p_req->cfg.add_cfg.len = add_cc_len;
            memcpy(&p_req->cfg.add_cfg.data, p_codec_cfg_req[i].codec_cfg.add_cc_param_ptr.data, add_cc_len);
        }
        p_req->con_lid = con_lid;
        // Record ase local id
        p_req->ase_lid = p_ase_info->ase_lid;
        // Send req_ind to aob mgr, avoid stack over flow, using thread call
        bt_thread_call_func_3(app_gaf_evt_report, bt_fixed_param(APP_GAF_ASCS_CONFIGURE_CODEC_RI),
                              bt_alloc_param_size(p_req, (sizeof(*p_req) + add_cc_len)),
                              bt_fixed_param((sizeof(*p_req) + add_cc_len)));
        app_gaf_free_buff(p_req);
    }
}

static void app_bap_ascs_cb_ase_enable_req(uint8_t con_lid, uint8_t ase_num,
                                           const ascs_metadata_p_t *param)
{
    if (param == NULL)
    {
        LOG_W("%s enable req metadata is NULL!", __func__);
        return;
    }

    uint8_t ase_lid_list[APP_BAP_DFT_ASCS_NB_ASE_CHAR] = {0};

    uint8_t  i = 0;

    const ascs_metadata_p_t *param_get = param;

    for (i = 0; i < ase_num; i++)
    {
        app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info_by_ase_id(con_lid, param_get->ase_id);

        if (NULL == p_ase_info)
        {
            LOG_W("%s ase_id %d find error", __func__, param_get->ase_id);
            return;
        }

        gen_aud_metadata_ptr_t metadata;

        gen_aud_init_metadata_ptr(&metadata);

        gen_aud_codec_unpack_metadata(p_ase_info->codec_id.codec_id, &metadata,
                                      (gen_aud_var_info_t *) & (param_get->len));

        uint8_t metadata_ltv_len = gen_aud_codec_calc_metadata_ltv_list_value_len(&metadata.parsed_metadata_ptr);

        uint8_t metadata_len = sizeof(app_gaf_bap_cfg_metadata_t) + metadata_ltv_len;

        if (NULL != p_ase_info->p_metadata)
        {
            app_gaf_free_buff(p_ase_info->p_metadata);
            p_ase_info->p_metadata = NULL;
        }

        p_ase_info->p_metadata = (app_gaf_bap_cfg_metadata_t *)app_gaf_malloc_buff(metadata_len);

        if (NULL != p_ase_info->p_metadata)
        {
            p_ase_info->p_metadata->param.context_bf = metadata.basic_metadata.streaming_audio_context;

            p_ase_info->p_metadata->add_metadata.len =
                gen_aud_codec_memcpy_s_metadata_ltv_list_value(&metadata.parsed_metadata_ptr,
                                                               &p_ase_info->p_metadata->add_metadata.data[0], metadata_ltv_len);
        }
        else
        {
            LOG_E("%d:malloc failed!", __LINE__);
            break;
        }

        ase_lid_list[i] = p_ase_info->ase_lid;

        param_get = (ascs_metadata_p_t *)((uint32_t)param_get + sizeof(ascs_metadata_p_t) + param_get->len);
    }

    for (i = 0; i < ase_num; i++)
    {
        app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_by_ase_lid(ase_lid_list[i]);

        app_gaf_uc_srv_enable_req_ind_t *p_enable_req =
            (app_gaf_uc_srv_enable_req_ind_t *)app_gaf_malloc_buff(
                sizeof(app_gaf_uc_srv_enable_req_ind_t) + p_ase_info->p_metadata->add_metadata.len);

        p_enable_req->ase_lid = p_ase_info->ase_lid;

        memcpy(&p_enable_req->metadata, p_ase_info->p_metadata,
               p_ase_info->p_metadata->add_metadata.len + sizeof(app_gaf_bap_cfg_metadata_t));

        app_gaf_evt_report(APP_GAF_ASCS_ENABLE_RI, (void *)p_enable_req,
                           (sizeof(app_gaf_uc_srv_enable_req_ind_t) + p_ase_info->p_metadata->add_metadata.len));

        app_gaf_free_buff(p_enable_req);
    }
}

static void app_bap_ascs_cb_ase_update_metadata_req(uint8_t con_lid, uint8_t ase_num,
                                                    const ascs_metadata_p_t *param)
{
    if (param == NULL)
    {
        LOG_W("%s upd md req metadata is NULL!", __func__);
        return;
    }

    const ascs_metadata_p_t *param_get = param;

    uint8_t ase_id_list[APP_BAP_DFT_ASCS_NB_ASE_CHAR] = {0};

    uint8_t  i = 0;

    for (i = 0; i < ase_num; i++)
    {
        app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info_by_ase_id(con_lid, param_get->ase_id);

        if (NULL == p_ase_info)
        {
            LOG_W("%s ase_id %d find error", __func__, param_get->ase_id);
            return;
        }

        gen_aud_metadata_ptr_t metadata;

        gen_aud_init_metadata_ptr(&metadata);

        gen_aud_codec_unpack_metadata(p_ase_info->codec_id.codec_id, &metadata,
                                      (gen_aud_var_info_t *) & (param_get->len));

        uint8_t add_metadata_len = gen_aud_codec_calc_metadata_ltv_list_value_len(&metadata.parsed_metadata_ptr);

        uint8_t metadata_len = sizeof(app_gaf_bap_cfg_metadata_t) + add_metadata_len;

        if (NULL != p_ase_info->p_metadata)
        {
            app_gaf_free_buff(p_ase_info->p_metadata);
        }

        p_ase_info->p_metadata = (app_gaf_bap_cfg_metadata_t *)app_gaf_malloc_buff(metadata_len);

        if (NULL != p_ase_info->p_metadata)
        {
            p_ase_info->p_metadata->param.context_bf = metadata.basic_metadata.streaming_audio_context;
            p_ase_info->p_metadata->add_metadata.len =
                gen_aud_codec_memcpy_s_metadata_ltv_list_value(&metadata.parsed_metadata_ptr,
                                                               &p_ase_info->p_metadata->add_metadata.data[0],
                                                               add_metadata_len);
        }
        else
        {
            LOG_E("%d:malloc failed!", __LINE__);
            break;
        }

        ase_id_list[i] = param_get->ase_id;

        param_get = (ascs_metadata_p_t *)((uint32_t)param_get + sizeof(ascs_metadata_p_t) + param_get->len);
    }

    for (i = 0; i < ase_num; i++)
    {
        app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info_by_ase_id(con_lid, ase_id_list[i]);

        app_gaf_uc_srv_update_metadata_req_ind_t *p_upd_md_req = (app_gaf_uc_srv_update_metadata_req_ind_t
                                                                  *)app_gaf_malloc_buff(sizeof(app_gaf_uc_srv_update_metadata_req_ind_t) +
                                                                                        p_ase_info->p_metadata->add_metadata.len);

        p_upd_md_req->ase_lid = p_ase_info->ase_lid;

        memcpy(&p_upd_md_req->metadata, p_ase_info->p_metadata, sizeof(app_gaf_bap_cfg_metadata_t) +
               p_ase_info->p_metadata->add_metadata.len);

        app_gaf_evt_report(APP_GAF_ASCS_UPDATE_METADATA_RI, (void *)p_upd_md_req,
                           sizeof(app_gaf_uc_srv_update_metadata_req_ind_t) +
                           p_ase_info->p_metadata->add_metadata.len);

        app_gaf_free_buff(p_upd_md_req);
    }

    bap_uc_srv_update_metadata_req_upper_cfm(con_lid, true, 0, 0, ase_num, ase_id_list);
}

static void app_bap_uc_srv_report_ase_ntf_value(uint8_t ase_lid, uint8_t opcode)
{
    app_gaf_bap_uc_srv_ase_ntf_value_ind_t ase_ntf_value =
    {
        .nb_ases = 1,
        .ase_lid = ase_lid,
        .opcode = opcode,
    };

    app_gaf_evt_report(APP_GAF_ASCS_ASE_NTF_VALUE_IND, (void *)&ase_ntf_value, sizeof(ase_ntf_value));
}

static void app_bap_ascs_cb_ase_state_ind(uint8_t con_lid, uint8_t ase_id, uint8_t ase_state,
                                          const bap_uc_srv_qos_cfg_ind_t *p_qos_cfg_ind)
{
    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info_by_ase_id(con_lid, ase_id);

    if (NULL == p_ase_info)
    {
        LOG_W("%s: cannot found ase info %d", __func__, ase_id);
        return;
    }

    uint8_t new_state = ase_state;
    uint8_t old_state = p_ase_info->ase_state;

    enum app_gaf_direction direction = p_ase_info->direction;

    if (new_state != old_state)
    {
        static uint8_t state_machine_transition[ASCS_ASE_STATE_MAX][ASCS_ASE_STATE_MAX] =
        {
            /*IDLE*/    {ASCS_OPCODE_MIN,       ASCS_OPCODE_CFG_CODEC,},
            /*CODEC*/   {
                ASCS_OPCODE_RELEASE,   ASCS_OPCODE_CFG_CODEC,
                ASCS_OPCODE_CFG_QOS,   ASCS_OPCODE_MIN,
                ASCS_OPCODE_MIN,       ASCS_OPCODE_MIN,
                ASCS_OPCODE_RELEASE
            },
            /*QOS*/     {
                ASCS_OPCODE_MIN,       ASCS_OPCODE_CFG_CODEC,
                ASCS_OPCODE_CFG_QOS,   ASCS_OPCODE_ENABLE,
                ASCS_OPCODE_MIN,       ASCS_OPCODE_MIN,
                ASCS_OPCODE_RELEASE
            },
            /*ENABLING*/{
                ASCS_OPCODE_MIN,       ASCS_OPCODE_MIN,
                ASCS_OPCODE_DISABLE,   ASCS_OPCODE_MIN,
                ASCS_OPCODE_MIN,       ASCS_OPCODE_DISABLE,
                ASCS_OPCODE_RELEASE
            },
            /*STREAMING*/{
                ASCS_OPCODE_MIN,       ASCS_OPCODE_MIN,
                ASCS_OPCODE_DISABLE,   ASCS_OPCODE_MIN,
                ASCS_OPCODE_MIN,       ASCS_OPCODE_DISABLE,
                ASCS_OPCODE_RELEASE
            },
            /*DISBLING*/{
                ASCS_OPCODE_MIN,       ASCS_OPCODE_MIN,
                ASCS_OPCODE_MIN,       ASCS_OPCODE_MIN,
                ASCS_OPCODE_MIN,       ASCS_OPCODE_MIN,
                ASCS_OPCODE_RELEASE
            },
            /*RELEASING*/{ASCS_OPCODE_MIN},
        };

        uint8_t opcode = state_machine_transition[old_state][new_state];

        if (opcode != ASCS_OPCODE_MIN)
        {
            app_bap_uc_srv_report_ase_ntf_value(p_ase_info->ase_lid, opcode);
        }

        app_bap_uc_srv_update_ase_state(p_ase_info, new_state, old_state);

        app_gaf_ascs_cis_stream_started_t cis_stream_started;

        /// Send stream start ind to prepare gaf stream info before truly streaming
        if (((APP_GAF_DIRECTION_SINK == direction) && (new_state == ASCS_ASE_STATE_ENABLING)) ||
                ((APP_GAF_DIRECTION_SRC == direction) && (new_state == ASCS_ASE_STATE_STREAMING)))
        {
            cis_stream_started.ase_lid = p_ase_info->ase_lid;
            cis_stream_started.con_lid = p_ase_info->con_lid;
            cis_stream_started.cis_hdl = p_ase_info->cis_hdl;
            cis_stream_started.direction = p_ase_info->direction;
            app_gaf_evt_report(APP_GAF_ASCS_CIS_STREAM_STARTED_IND, (void *)&cis_stream_started,
                               sizeof(app_gaf_ascs_cis_stream_started_t));
        }
    }

    if (ASCS_ASE_STATE_STREAMING == new_state)
    {
        if (APP_GAF_DIRECTION_SINK == direction)
        {
            app_bap_uc_srv_rx_stream_start(p_ase_info);
        }
        else if (APP_GAF_DIRECTION_SRC == direction)
        {
            app_bap_uc_srv_tx_stream_start(p_ase_info);
        }
    }
    else if ((ASCS_ASE_STATE_STREAMING == old_state ||
              ASCS_ASE_STATE_ENABLING == old_state) &&
             /// Streaming or enabling go to any state legally
             ((ASCS_ASE_STATE_QOS_CONFIGURED == new_state) ||
              (ASCS_ASE_STATE_RELEASING == new_state) ||
              (ASCS_ASE_STATE_IDLE == new_state) ||
              (ASCS_ASE_STATE_CODEC_CONFIGURED == new_state) ||
              (ASCS_ASE_STATE_DISABLING == new_state)))
    {
        if (direction < APP_GAF_DIRECTION_MAX)
        {
            app_gaf_ascs_cis_stream_stopped_t cis_stream_stopped;
            cis_stream_stopped.con_lid = p_ase_info->con_lid;
            cis_stream_stopped.ase_lid = p_ase_info->ase_lid;
            cis_stream_stopped.cis_hdl = p_ase_info->cis_hdl;
            cis_stream_stopped.direction = direction;
            app_gaf_evt_report(APP_GAF_ASCS_CIS_STREAM_STOPPED_IND, (void *)&cis_stream_stopped,
                               sizeof(app_gaf_ascs_cis_stream_stopped_t));
        }
    }
    else if (ASCS_ASE_STATE_QOS_CONFIGURED == new_state)
    {
        p_ase_info->qos_cfg.framing = p_qos_cfg_ind->framing;
        p_ase_info->qos_cfg.max_sdu_size = p_qos_cfg_ind->max_sdu_size;
        p_ase_info->qos_cfg.phy = p_qos_cfg_ind->phy;
        p_ase_info->qos_cfg.pres_delay_us = p_qos_cfg_ind->pres_delay;
        p_ase_info->qos_cfg.retx_nb = p_qos_cfg_ind->rtn;
        p_ase_info->qos_cfg.sdu_intv_us = p_qos_cfg_ind->sdu_interval;
        p_ase_info->qos_cfg.trans_latency_max_ms = p_qos_cfg_ind->max_trans_latency;
        p_ase_info->cig_id = p_qos_cfg_ind->cig_id;
        p_ase_info->cis_id = p_qos_cfg_ind->cis_id;
    }
    else if (ASCS_ASE_STATE_QOS_CONFIGURED > new_state)
    {
        memset(&p_ase_info->qos_cfg, 0, sizeof(p_ase_info->qos_cfg));
        p_ase_info->cig_id = ASCS_INVALID_GRP_LID;
        p_ase_info->cis_id = ASCS_INVALID_STREAM_LID;
    }
}

static void app_bap_ascs_cb_ase_op_local_cmp(uint8_t con_lid, uint8_t ase_id,
                                             uint8_t op_code, uint16_t status)
{
    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info_by_ase_id(con_lid, ase_id);

    if (NULL == p_ase_info)
    {
        LOG_W("%s p_ase_info find error!", __func__);
        return;
    }

    LOG_I("%s ase_lid = %d, op_code = %d, status = %d", __func__, p_ase_info->ase_lid, op_code, status);
}

static uint8_t app_bap_uc_srv_get_ase_by_cis_id(uint8_t con_lid, uint8_t cig_id, uint8_t cis_id,
                                                app_bap_ascs_ase_t **ase_list)
{
    if (ase_list == NULL)
    {
        return 0;
    }

    uint8_t ase_cnt = 0;

    app_bap_ascs_ase_t *p_ase_info = NULL;

    uint8_t ase_lid = 0;

    for (ase_lid = 0; ase_lid < APP_BAP_DFT_ASCS_NB_ASE_CFG; ase_lid++)
    {
        p_ase_info = app_bap_uc_srv_get_ase_by_ase_lid(ase_lid);

        if (p_ase_info->con_lid == con_lid &&
                p_ase_info->cig_id == cig_id &&
                p_ase_info->cis_id == cis_id)
        {
            ase_list[ase_cnt++] = p_ase_info;

            if (ase_cnt == BAP_DIRECTION_MAX)
            {
                break;
            }
        }
    }

    return ase_cnt;
}

static void app_bap_ascs_cb_cis_status_ind(enum bap_uc_srv_cis_evt cis_evt, uint8_t con_lid,
                                           uint16_t err_code, const bap_uc_srv_cis_info_t *p_cis_info)
{
    uint8_t cig_id = p_cis_info->cig_id;
    uint8_t cis_id = p_cis_info->cis_id;
    uint16_t cis_hdl = p_cis_info->cis_hdl;
    const bap_cis_timing_t *p_cis_timing = p_cis_info->cis_timing;

    app_bap_ascs_ase_t *p_ase_info_list[BAP_DIRECTION_MAX] = {NULL};

    uint8_t cnt = app_bap_uc_srv_get_ase_by_cis_id(con_lid, cig_id, cis_id, p_ase_info_list);

    LOG_I("cis_hdl 0x%x cig_id %d cis_id %d bind_ase_num %d", cis_hdl, cig_id, cis_id, cnt);

    if (cnt == 0)
    {
        LOG_W("app_bap cis can not found ase by cig cis id");
        return;
    }

    app_gaf_uc_srv_cis_state_ind_t cis_state =
    {
        .cig_id = cig_id,
        .cis_id = cis_id,
        .con_lid = con_lid,
        .conhdl = cis_hdl,
        .reason = 0,
        .ase_lid_sink = APP_GAF_INVALID_ANY_LID,
        .ase_lid_src = APP_GAF_INVALID_ANY_LID,
    };

    if (p_cis_timing != NULL)
    {
        cis_state.cig_config.iso_intv_frames = p_cis_timing->iso_interval_1_25ms;
        cis_state.cig_config.sync_delay_us = p_cis_timing->cig_sync_delay_us;
        cis_state.cig_config.tlatency_m2s_us = p_cis_timing->bn_c2p;
        cis_state.cig_config.tlatency_s2m_us = p_cis_timing->bn_p2c;
        cis_state.cis_config.bn_m2s = p_cis_timing->bn_c2p;
        cis_state.cis_config.bn_s2m = p_cis_timing->bn_p2c;
        cis_state.cis_config.ft_m2s = p_cis_timing->ft_c2p;
        cis_state.cis_config.ft_s2m = p_cis_timing->ft_p2c;
        cis_state.cis_config.sync_delay_us = p_cis_timing->cis_sync_delay_us;
    }

    if (cis_evt == BAP_UC_SRV_CIS_EVENT_CONNECTED) // CIS created
    {
        uint8_t idx = 0;

        for (idx = 0; idx < cnt; idx++)
        {
            p_ase_info_list[idx]->cis_hdl = cis_hdl;
            p_ase_info_list[idx]->cig_sync_delay = p_cis_timing->cig_sync_delay_us;
            p_ase_info_list[idx]->cis_sync_delay = p_cis_timing->cis_sync_delay_us;
            p_ase_info_list[idx]->bn_m2s = p_cis_timing->bn_c2p;
            p_ase_info_list[idx]->bn_s2m = p_cis_timing->bn_p2c;
            p_ase_info_list[idx]->iso_interval_us = p_cis_timing->iso_interval_1_25ms * 1250;

            // Only report ASE(s) that used to streaming
            if (p_ase_info_list[idx]->ase_state < ASCS_ASE_STATE_ENABLING)
            {
                continue;
            }

            if (p_ase_info_list[idx]->direction == (enum app_gaf_direction)BAP_DIRECTION_SINK)
            {
                cis_state.ase_lid_sink = p_ase_info_list[idx]->ase_lid;
            }
            else
            {
                cis_state.ase_lid_src = p_ase_info_list[idx]->ase_lid;
            }
        }

        app_gaf_evt_report(APP_GAF_ASCS_CIS_ESTABLISHED_IND, (void *)&cis_state, sizeof(cis_state));
    }
    else if (cis_evt == BAP_UC_SRV_CIS_EVENT_DISCONNECTED)
    {
        uint8_t idx = 0;
        for (idx = 0; idx < cnt; idx++)
        {
            p_ase_info_list[idx]->cis_hdl = ASCS_INVALID_CIS_HDL;
            p_ase_info_list[idx]->cig_sync_delay = 0;
            p_ase_info_list[idx]->cis_sync_delay = 0;
            p_ase_info_list[idx]->bn_m2s = 0;
            p_ase_info_list[idx]->bn_s2m = 0;
            p_ase_info_list[idx]->iso_interval_us = 0;

            if (p_ase_info_list[idx]->direction == (enum app_gaf_direction)BAP_DIRECTION_SINK)
            {
                cis_state.ase_lid_sink = p_ase_info_list[idx]->ase_lid;
            }
            else
            {
                cis_state.ase_lid_src = p_ase_info_list[idx]->ase_lid;
            }
        }

        app_gaf_evt_report(APP_GAF_ASCS_CIS_DISCONNETED_IND, (void *)&cis_state,
                           sizeof(app_gaf_uc_srv_cis_state_ind_t));
    }
    else
    {
        app_gaf_bap_uc_srv_cis_rejected_ind_t cis_rej =
        {
            .con_hdl = cis_hdl,
            .error = 0,
        };

        LOG_D("app_bap ascs cis_rejected_ind con_hdl= %02x", cis_hdl);

        app_gaf_evt_report(APP_GAF_ASCS_CIS_REJECTED_IND, (void *)&cis_rej, sizeof(cis_rej));
    }
}

static void app_bap_ascs_cb_cig_termianted_ind(uint8_t con_lid, uint8_t cig_id)
{
    LOG_I("app_bap ascs cig_terminated_ind con_lid = %d, cig_id = %02x", con_lid, cig_id);

    app_gaf_bap_uc_srv_cig_terminated_ind_t cig_term =
    {
        .cig_id = cig_id,
        .reason = 0,
    };

    app_gaf_evt_report(APP_GAF_ASCS_CIG_TERMINATED_IND, (void *)&cig_term, sizeof(cig_term));
}

static void app_bap_ascs_cb_iso_dp_state_ind(bool is_setup, uint8_t con_lid, uint8_t ase_id, uint8_t err_code)
{
    LOG_D("app_bap ascs iso dp state is_setup = %d, con_lid = %d, ase_id = %d, err_code = %d",
          is_setup, con_lid, ase_id, err_code);
}

const bap_uc_srv_ascs_evt_cb_t ascs_evt_cb =
{
    .cb_bond_data = app_bap_ascs_cb_bond_data,
    .cb_ase_codec_cfg_req = app_bap_ascs_cb_ase_codec_cfg_req,
    .cb_ase_enable_req = app_bap_ascs_cb_ase_enable_req,
    .cb_ase_metadata_upd_req = app_bap_ascs_cb_ase_update_metadata_req,
    .cb_ase_state_ind = app_bap_ascs_cb_ase_state_ind,
    .cb_ase_op_local_cmp = app_bap_ascs_cb_ase_op_local_cmp,
    .cb_cis_status_ind = app_bap_ascs_cb_cis_status_ind,
    .cb_cig_term_ind = app_bap_ascs_cb_cig_termianted_ind,
    .cb_iso_dp_status_ind = app_bap_ascs_cb_iso_dp_state_ind,
};

int app_bap_uc_srv_init(void)
{
    LOG_I("%s", __func__);

    bap_uc_srv_init_cfg_t ascs_init_cfg =
    {
        .pref_mtu = GAF_PREFERRED_MTU,
        .num_sink_ase_supp = APP_BAP_DFT_ASCS_NB_ASE_CHAR_SINK,
        .num_src_ase_supp = APP_BAP_DFT_ASCS_NB_ASE_CHAR_SRC,
        .auto_cis_disconn = false,
        .accpet_qos_cis_req = true,
    };

    return bap_uc_srv_ascs_init(&ascs_init_cfg, &ascs_evt_cb);
}

int app_bap_uc_srv_deinit(void)
{
    LOG_I("%s", __func__);
    return bap_uc_srv_ascs_deinit();
}

int app_bap_uc_srv_info_init(void)
{
    if (p_app_ascs_env != NULL)
    {
        return BT_STS_SUCCESS;
    }

    uint16_t size = sizeof(app_bap_ascs_env_t) + APP_BAP_DFT_ASCS_NB_ASE_CFG *
                    sizeof(app_bap_ascs_ase_t);

    p_app_ascs_env = (app_bap_ascs_env_t *)app_gaf_malloc_buff(size);

    if (NULL == p_app_ascs_env)
    {
        LOG_E("app_bap uc srv env init malloc error");
        return BT_STS_NO_RESOURCES;
    }

    memset(p_app_ascs_env, 0, size);

    // Pointer to reserved mem for ase
    p_app_ascs_env->ase_info = (app_bap_ascs_ase_t *)((uint32_t)p_app_ascs_env + sizeof(app_bap_ascs_env_t));

    for (uint8_t ase_lid = 0; ase_lid < APP_BAP_DFT_ASCS_NB_ASE_CFG; ase_lid++)
    {
        app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_by_ase_lid(ase_lid);

        // Please align with ascs.c [1, 2, 3, 4, ...]
        p_ase_info->ase_instance_idx = (ase_lid) % (APP_BAP_DFT_ASCS_NB_ASE_CHAR) + ASCS_ASE_ID_MIN;

        if (p_ase_info->ase_instance_idx <= APP_BAP_DFT_ASCS_NB_ASE_CHAR_SINK)
        {
            p_ase_info->direction = APP_GAF_DIRECTION_SINK;
        }
        else
        {
            p_ase_info->direction = APP_GAF_DIRECTION_SRC;
        }

        p_ase_info->ase_state = ASCS_ASE_STATE_IDLE;
        p_ase_info->ase_lid = ase_lid;
        p_ase_info->con_lid = ase_lid / APP_BAP_DFT_ASCS_NB_ASE_CHAR;
        p_ase_info->cig_id = ASCS_INVALID_GRP_LID;
        p_ase_info->cis_id = ASCS_INVALID_STREAM_LID;
        p_ase_info->cis_hdl = ASCS_INVALID_CIS_HDL;
    }

    return BT_STS_SUCCESS;
}

int app_bap_uc_srv_info_deinit(void)
{
    LOG_I("%s", __func__);
    if (p_app_ascs_env == NULL)
    {
        return BT_STS_SUCCESS;
    }

    uint8_t ase_lid = 0;

    for (ase_lid = 0; ase_lid < APP_BAP_DFT_ASCS_NB_ASE_CFG; ase_lid++)
    {
        app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_by_ase_lid(ase_lid);

        if (p_ase_info != NULL)
        {
            if (p_ase_info->p_cfg != NULL)
            {
                app_gaf_free_buff(p_ase_info->p_cfg);
            }

            if (p_ase_info->p_metadata != NULL)
            {
                app_gaf_free_buff(p_ase_info->p_metadata);
            }
        }
    }

    app_gaf_free_buff(p_app_ascs_env);

    p_app_ascs_env = NULL;

    return BT_STS_SUCCESS;
}

int app_bap_uc_srv_set_ase_qos_req(uint8_t ase_lid, app_gaf_bap_qos_req_t *qos_req)
{
    if ((ase_lid >= APP_BAP_DFT_ASCS_NB_ASE_CFG)
            || (NULL == qos_req))
    {
        LOG_W("app_bap uc srv error param");
        return BT_STS_INVALID_PARM;
    }

    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_by_ase_lid(ase_lid);
    memcpy(&p_ase_info->qos_req, qos_req, sizeof(app_gaf_bap_qos_req_t));

    return BT_STS_SUCCESS;
}

app_bap_ascs_ase_t *app_bap_uc_srv_get_ase_info(uint8_t ase_lid)
{
    if (ase_lid >= APP_BAP_DFT_ASCS_NB_ASE_CFG)
    {
        LOG_W("app_bap uc srv error ase_lid %d", ase_lid);
        return NULL;
    }

    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_by_ase_lid(ase_lid);
    return p_ase_info;
}

app_bap_ascs_ase_t *app_bap_uc_srv_get_ase_info_from_cis_hdl(enum app_gaf_direction direction,
                                                             uint16_t cis_hdl)
{
    if (cis_hdl <= APP_GAF_INVALID_CON_LID)
    {
        LOG_W("app_bap uc srv error cis_hdl %d", cis_hdl);
        return NULL;
    }

    app_bap_ascs_ase_t *p_ase_info = &p_app_ascs_env->ase_info[0];

    for (uint8_t i = 0; i < APP_BAP_DFT_ASCS_NB_ASE_CFG; i++)
    {
        if (p_ase_info[i].cis_hdl == cis_hdl && direction == p_ase_info[i].direction)
        {
            return &p_ase_info[i];
        }
    }

    return NULL;
}

int app_bap_uc_srv_get_ase_state(uint8_t ase_lid)
{
    if (ase_lid >= APP_BAP_DFT_ASCS_NB_ASE_CFG)
    {
        LOG_W("app_bap uc srv error ase_lid %d", ase_lid);
        return ASCS_ASE_STATE_MAX;
    }

    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_by_ase_lid(ase_lid);

    return p_ase_info->ase_state;
}

int app_bap_uc_srv_get_streaming_ase_lid(uint8_t con_lid, enum app_gaf_direction direction)
{
    app_bap_ascs_ase_t *p_ase_info = NULL;

    for (uint8_t ase_lid = 0; ase_lid < APP_BAP_DFT_ASCS_NB_ASE_CFG; ase_lid++)
    {
        p_ase_info = app_bap_uc_srv_get_ase_by_ase_lid(ase_lid);

        if ((p_ase_info->ase_state == ASCS_ASE_STATE_STREAMING) &&
                (direction == p_ase_info->direction) && (con_lid == p_ase_info->con_lid))
        {
            return ase_lid;
        }
    }

    return APP_GAF_INVALID_CON_LID;
}

int app_bap_uc_srv_get_specifc_state_ase_lid_list(uint8_t con_lid, uint8_t ase_state, uint8_t *ase_lid_list)
{
    if (NULL == ase_lid_list || con_lid >= BLE_CONNECTION_MAX)
    {
        LOG_E("%s err input", __func__);
        return 0;
    }

    if (!p_app_ascs_env)
    {
        return 0;
    }

    uint8_t ase_cnt = 0;
    app_bap_ascs_ase_t *p_ase_info = NULL;

    for (uint8_t ase_lid = 0; ase_lid < APP_BAP_DFT_ASCS_NB_ASE_CFG; ase_lid++)
    {
        p_ase_info = app_bap_uc_srv_get_ase_by_ase_lid(ase_lid);

        if ((p_ase_info->ase_state == ase_state) && (con_lid == p_ase_info->con_lid))
        {
            ase_lid_list[ase_cnt] = ase_lid;
            ase_cnt++;
        }
    }

    return ase_cnt;
}

int app_bap_uc_srv_get_streaming_ase_lid_list(uint8_t con_lid, uint8_t *ase_lid_list)
{
    return app_bap_uc_srv_get_specifc_state_ase_lid_list(con_lid, ASCS_ASE_STATE_STREAMING, ase_lid_list);
}

int app_bap_uc_srv_send_configure_codec_rsp(bool accept, uint8_t ase_lid,
                                            const app_gaf_codec_id_t *p_codec_id,
                                            const app_gaf_bap_qos_req_t *p_qos_req,
                                            const app_gaf_bap_cfg_t *p_codec_cfg)
{
    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_by_ase_lid(ase_lid);

    if (p_ase_info == NULL)
    {
        return BT_STS_INVALID_PARM;
    }

    if (accept == true && (p_qos_req == NULL || p_codec_cfg == NULL || p_codec_id == NULL))
    {
        return BT_STS_INVALID_PARM;
    }

    LOG_I("%s con_lid = %d, accpet = %d, ase_id = %d", __func__, p_ase_info->con_lid, accept, p_ase_info->ase_instance_idx);

    uint16_t status = BT_STS_SUCCESS;

    if (accept == true)
    {
        bap_uc_srv_codec_cfg_qos_req_t *p_qos_req_param = (bap_uc_srv_codec_cfg_qos_req_t *)
                                                          app_gaf_malloc_buff(sizeof(bap_uc_srv_codec_cfg_qos_req_t));
        if (p_qos_req_param != NULL)
        {
            p_qos_req_param->framing_type = p_qos_req->framing;
            p_qos_req_param->max_trans_latency_ms = p_qos_req->trans_latency_max_ms;
            p_qos_req_param->phy_bf = p_qos_req->phy_bf;
            p_qos_req_param->rtn = p_qos_req->retx_nb;
            p_qos_req_param->prefer_pd_max_us = p_qos_req->pref_pres_delay_max_us;
            p_qos_req_param->prefer_pd_min_us = p_qos_req->pref_pres_delay_min_us;
            p_qos_req_param->pres_delay_max_us = p_qos_req->pres_delay_max_us;
            p_qos_req_param->pres_delay_min_us = p_qos_req->pres_delay_min_us;
        }
        else
        {
            accept = false;
        }

        status = bap_uc_srv_configure_codec_req_upper_cfm(p_ase_info->con_lid,
                                                          p_ase_info->ase_instance_idx,
                                                          accept,
                                                          ASCS_CP_RSP_CODE_REJECTED_CFG_PARAM,
                                                          ASCS_CP_REASON_APP_REJECTED,
                                                          p_qos_req_param);
        app_gaf_free_buff(p_qos_req_param);
    }
    else
    {
        status = bap_uc_srv_configure_codec_req_upper_cfm(p_ase_info->con_lid,
                                                          p_ase_info->ase_instance_idx,
                                                          false,
                                                          ASCS_CP_RSP_CODE_REJECTED_CFG_PARAM,
                                                          ASCS_CP_REASON_APP_REJECTED,
                                                          NULL);
    }

    if (status == BT_STS_SUCCESS && accept)
    {
        // Assign codec id
        p_ase_info->codec_id = *p_codec_id;

        if (p_ase_info->p_cfg != NULL && p_ase_info->p_cfg->add_cfg.len != p_codec_cfg->add_cfg.len)
        {
            app_gaf_free_buff(p_ase_info->p_cfg);
            p_ase_info->p_cfg = NULL;
        }

        if (p_ase_info->p_cfg == NULL)
        {
            p_ase_info->p_cfg = (app_gaf_bap_cfg_t *)app_gaf_malloc_buff(sizeof(app_gaf_bap_cfg_t) + p_codec_cfg->add_cfg.len);

            if (p_ase_info->p_cfg == NULL)
            {
                return BT_STS_NO_RESOURCES;
            }
        }

        // Assign codec cfg parameters
        memcpy(p_ase_info->p_cfg, p_codec_cfg, sizeof(app_gaf_bap_cfg_t) + p_codec_cfg->add_cfg.len);
        // Assign qos req parameters
        p_ase_info->qos_req = *p_qos_req;

        app_bap_cfg_print(p_ase_info->p_cfg);
    }

    return status;
}

int app_bap_uc_srv_send_enable_rsp_with_reason(uint8_t ase_lid, bool accept, uint8_t reason)
{
    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info(ase_lid);

    if (p_ase_info == NULL)
    {
        LOG_W("%s ase_lid %d find error", __func__, ase_lid);
        return BT_STS_INVALID_PARM;
    }

    return bap_uc_srv_enable_req_upper_cfm(p_ase_info->con_lid, accept, ASCS_CP_RSP_CODE_REJECTED_METADATA,
                                           accept ? 0 : reason, 1, &p_ase_info->ase_instance_idx);
}

int app_bap_uc_srv_send_update_metadata_rsp(uint8_t ase_lid, bool accpet, uint8_t reason)
{
    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info(ase_lid);

    if (p_ase_info == NULL)
    {
        LOG_W("%s ase_lid %d find error", __func__, ase_lid);
        return BT_STS_INVALID_PARM;
    }

    return bap_uc_srv_update_metadata_req_upper_cfm(p_ase_info->con_lid, accpet, ASCS_CP_RSP_CODE_REJECTED_METADATA,
                                                    accpet ? 0 : reason, 1, &p_ase_info->ase_instance_idx);
}

int app_bap_uc_srv_stream_release(uint8_t ase_lid, uint8_t idle)
{
    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info(ase_lid);

    if (p_ase_info == NULL)
    {
        return BT_STS_INVALID_PARM;
    }

    if ((ase_lid >= APP_BAP_DFT_ASCS_NB_ASE_CFG) ||
            (ASCS_ASE_STATE_IDLE == p_ase_info->ase_state))
    {
        LOG_W("app_bap uc srv release error ase_lid:%d", ase_lid);
        return BT_STS_INVALID_PARM;
    }

    return bap_uc_srv_release_ase_local(p_ase_info->con_lid, p_ase_info->ase_instance_idx);
}

#if 0
int app_bap_uc_srv_check_disconnect_exist_cis(uint8_t ase_lid)
{
    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info(ase_lid);

    if (p_ase_info == NULL)
    {
        return BT_STS_INVALID_PARM;
    }

    if ((ase_lid >= APP_BAP_DFT_ASCS_NB_ASE_CFG) ||
            (ASCS_ASE_STATE_IDLE == p_ase_info->ase_state))
    {
        LOG_W("app_bap uc srv disconnect cis error ase_lid:%d", ase_lid);
        return BT_STS_INVALID_PARM;
    }

    return bap_uc_srv_disonnect_ase_cis(p_ase_info->con_lid, p_ase_info->ase_instance_idx);
}
#endif

int app_bap_uc_srv_read_iso_link_quality(uint8_t ase_lid)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_bap_uc_srv_stream_disable(uint8_t ase_lid)
{
    if (ase_lid >= APP_BAP_DFT_ASCS_NB_ASE_CFG)
    {
        LOG_W("app_bap uc srv error disable ase_lid");
        return BT_STS_INVALID_PARM;
    }

    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info(ase_lid);

    if (p_ase_info == NULL)
    {
        return BT_STS_INVALID_PARM;
    }

    return bap_uc_srv_disable_ase_local(p_ase_info->con_lid, p_ase_info->ase_instance_idx);
}

int app_bap_uc_srv_set_test_mode(bool enable)
{
    p_app_ascs_env->tm_enable = enable;
    return BT_STS_SUCCESS;
}

int app_bap_uc_srv_get_nb_ase_chars()
{
    return APP_BAP_DFT_ASCS_NB_ASE_CHAR;
}

int app_bap_uc_srv_get_nb_ases_cfg()
{
    return APP_BAP_DFT_ASCS_NB_ASE_CFG;
}

int app_bap_uc_srv_iso_quality_ind_handler(uint16_t cisHdl, uint8_t *param)
{
    LOG_I("%s does not support", __func__);
    return 0;
}
#endif
