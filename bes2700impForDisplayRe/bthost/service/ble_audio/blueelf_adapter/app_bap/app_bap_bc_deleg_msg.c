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
#ifdef APP_BLE_BIS_DELEG_ENABLE
#include "app_bap.h"
#include "app_gaf.h"
#include "app_gaf_define.h"
#include "ble_app_dbg.h"
#include "app_bap_bc_deleg_msg.h"
#include "app_bap_bc_scan_msg.h"
#include "app_gaf_custom_api.h"

#include "bap_scan_delegator.h"
#include "bap_broadcast_common.h"

/// Content of Broadcast Delegator environment
typedef struct app_bap_bc_deleg_env
{
    /// Broadcast Code record
    uint8_t broadcast_code[APP_BAP_DFT_BC_DELEG_NB_SRCS][APP_GAP_KEY_LEN];
} app_bap_bc_deleg_env_t;

app_bap_bc_deleg_env_t *p_app_bc_deleg_env = NULL;

static const uint8_t invalid_bcast_code[APP_GAP_KEY_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/*FUNCTIONS*/
static void app_bap_scan_deleg_cb_bond_data_hadler(uint8_t con_lid, uint8_t char_type, uint8_t cli_cfg_bf)
{
    LOG_I("%s con_lid = %d, char_type = %d, cli_cfg_bf = %x", __func__, con_lid, char_type, cli_cfg_bf);

    app_gaf_bc_deleg_bond_data_ind_t bond_data_ind =
    {
        .con_lid = con_lid,
        .cli_cfg_bf = cli_cfg_bf,
    };

    app_gaf_evt_report(APP_GAF_DELEG_BOND_DATA_IND, (void *)&bond_data_ind, sizeof(bond_data_ind));
}

static void app_bap_scan_deleg_cb_remote_scan_started_hadler(uint8_t con_lid)
{
    LOG_I("%s con_lid = %d", __func__, con_lid);

    app_gaf_bc_deleg_bond_remote_scan_ind_t remote_scan_started_ind =
    {
        .con_lid = con_lid,
        .state = 1,
    };

    app_gaf_evt_report(APP_GAF_DELEG_REMOTE_SCAN_STARTED_IND, (void *)&remote_scan_started_ind, sizeof(remote_scan_started_ind));
}

static void app_bap_scan_deleg_cb_remote_scan_stopped_hadler(uint8_t con_lid)
{
    LOG_I("%s con_lid = %d", __func__, con_lid);

    app_gaf_bc_deleg_bond_remote_scan_ind_t remote_scan_stopped_ind =
    {
        .con_lid = con_lid,
        .state = 0,
    };

    app_gaf_evt_report(APP_GAF_DELEG_REMOTE_SCAN_STOPPED_IND, (void *)&remote_scan_stopped_ind, sizeof(remote_scan_stopped_ind));
}

static void app_bap_scan_deleg_cb_add_source_req_hadler(uint8_t con_lid, uint8_t src_id, const bap_bc_bass_src_info_req_ptr_t *p_src_info)
{
    uint16_t status = bap_scan_deleg_add_source_upper_cfm(src_id, true);

    LOG_I("%s, cfm status = %d", __func__, status);

    app_gaf_bc_deleg_source_add_req_ind_t add_source_req =
    {
        .adv_id = *(app_gaf_bap_adv_id_t *)p_src_info->p_src_addr,
        .bcast_id = *(app_gaf_bap_bcast_id_t *)p_src_info->p_broadcast_id,
        .con_lid = con_lid,
        .nb_subgroups = p_src_info->sub_grp_num,
        .pa_intv_frames = p_src_info->pa_interval,
        .pa_sync_req = p_src_info->pa_sync_req,
        .src_lid = src_id,
    };//pa sync req ind from deleg

    // Broadcast code need reset to zero when add source
    memset(p_app_bc_deleg_env->broadcast_code[src_id % APP_BAP_DFT_BC_DELEG_NB_SRCS], 0, APP_GAP_KEY_LEN);

    // Mark scan method delegator
    app_bap_bc_scan_env_t *p_app_bap_bc_scan_env = app_bap_bc_scan_get_scan_env();

    if (p_app_bap_bc_scan_env != NULL)
    {
        p_app_bap_bc_scan_env->scan_trigger_method = APP_GAF_BAP_BC_DELEG_TRIGGER;
    }

    // No need to transfer subgrp info to GAF_EVT
    app_gaf_evt_report(APP_GAF_DELEG_SOURCE_ADD_RI, (void *)&add_source_req, sizeof(add_source_req));
}

static void app_bap_scan_deleg_cb_get_bis_sync_pref_hadler(uint8_t con_lid, uint8_t src_id)
{
    uint16_t dummy = (src_id << 8) | con_lid;
    app_gaf_evt_report(APP_GAF_DELEG_PREF_BIS_SYNC_RI, (void *)&dummy, sizeof(dummy));
}

static void app_bap_scan_deleg_cb_modify_source_req_hadler(uint8_t con_lid, uint8_t src_id, const bap_bc_bass_src_info_req_ptr_t *p_src_info)
{
    uint16_t status = bap_scan_deleg_modify_source_upper_cfm(src_id, true);
    LOG_I("%s, cfm status = %d", __func__, status);

    app_gaf_bc_deleg_source_update_req_ind_t modify_source_req =
    {
        .con_lid = con_lid,
        .src_lid = src_id,
        .pa_sync_req = p_src_info->pa_sync_req,
        .metadata.len = 0,
    };

    app_gaf_evt_report(APP_GAF_DELEG_SOURCE_UPDATE_RI, (void *)&modify_source_req, sizeof(modify_source_req));
}

static void app_bap_scan_deleg_cb_remove_source_req_hadler(uint8_t con_lid, uint8_t src_id)
{
    uint16_t status = bap_scan_deleg_remove_source_upper_cfm(src_id, true);

    LOG_I("%s, cfm status = %d", __func__, status);

    memset(p_app_bc_deleg_env->broadcast_code[src_id % APP_BAP_DFT_BC_DELEG_NB_SRCS], 0xFF, APP_GAP_KEY_LEN);

    app_gaf_bc_deleg_source_remove_req_ind_t remove_source_req =
    {
        .con_lid = con_lid,
        .src_lid = src_id,
    };

    app_gaf_evt_report(APP_GAF_DELEG_SOURCE_REMOVE_RI, (void *)&remove_source_req, sizeof(remove_source_req));
}

static void app_bap_scan_deleg_cb_correct_bc_coded_ind_hadler(uint8_t con_lid, uint8_t src_id, const uint8_t *p_broadcast_code)
{
    LOG_I("%s con_lid = %d, src_id = %d", __func__, con_lid, src_id);
    DUMP8("%02x ", p_broadcast_code, APP_GAP_KEY_LEN);
    // Record bcast code to restart bis sink if upper want to
    memcpy(p_app_bc_deleg_env->broadcast_code[src_id % APP_GAP_KEY_LEN], p_broadcast_code, APP_GAP_KEY_LEN);
}

static const bap_scan_deleg_evt_cb_t app_bap_scan_deleg_cb =
{
    .cb_bond_data = app_bap_scan_deleg_cb_bond_data_hadler,
    .cb_remote_scan_started = app_bap_scan_deleg_cb_remote_scan_started_hadler,
    .cb_remote_scan_stopped = app_bap_scan_deleg_cb_remote_scan_stopped_hadler,
    .cb_add_src_req = app_bap_scan_deleg_cb_add_source_req_hadler,
    .cb_modify_src_req = app_bap_scan_deleg_cb_modify_source_req_hadler,
    .cb_correct_bc_code_ind = app_bap_scan_deleg_cb_correct_bc_coded_ind_hadler,
    .cb_bis_sync_preferd_req = app_bap_scan_deleg_cb_get_bis_sync_pref_hadler,
    .cb_remove_src_req = app_bap_scan_deleg_cb_remove_source_req_hadler,
};

/*Functions*/
int app_bap_bc_deleg_init(void)
{
    LOG_I("%s", __func__);

    bap_scan_deleg_init_cfg_t bap_scan_deleg_init_cfg =
    {
        .pref_mtu = GAF_PREFERRED_MTU,
        .num_rx_src_supp = APP_BAP_DFT_BC_DELEG_NB_SRCS,
        .big_sync_mse = APP_BAP_DFT_BC_DELEG_SYNC_BIG_MSE,
        .big_sync_to_10ms = APP_BAP_DFT_BC_DELEG_SYNC_BIG_TIMEOUT_10MS,
        .pa_sync_skip = APP_BAP_DFT_BC_DELEG_SKIP,
        .pa_sync_to_10ms = APP_BAP_DFT_BC_DELEG_SYNC_TO_10MS,
        .pa_sync_retry_max = APP_BAP_DFT_BC_DELEG_SYNC_PA_RETRY_CNT,
        .scan_for_pa_to_10ms = APP_BAP_DFT_BC_DELEG_TIMEOUT_S,
        .wait_past_to_10ms = APP_BAP_DFT_BC_DELEG_WAIT_PAST_TO_10MS,
    };

    uint16_t status = bap_scan_deleg_init(&bap_scan_deleg_init_cfg, &app_bap_scan_deleg_cb);

    return status;
}

int app_bap_bc_deleg_deinit(void)
{
    LOG_I("%s", __func__);

    return bap_scan_deleg_deinit();
}

int app_bap_bc_deleg_info_init(void)
{
    if (p_app_bc_deleg_env != NULL)
    {
        return BT_STS_ALREADY_EXIST;
    }

    p_app_bc_deleg_env = (app_bap_bc_deleg_env_t *)app_gaf_malloc_buff(sizeof(app_bap_bc_deleg_env_t));

    if (p_app_bc_deleg_env == NULL)
    {
        return BT_STS_NO_RESOURCES;
    }

    memset(p_app_bc_deleg_env, 0, sizeof(app_bap_bc_deleg_env_t));
    memset(p_app_bc_deleg_env->broadcast_code, 0xFF, sizeof(p_app_bc_deleg_env->broadcast_code));
    return BT_STS_SUCCESS;
}

int app_bap_bc_deleg_info_deinit(void)
{
    if (p_app_bc_deleg_env == NULL)
    {
        return BT_STS_SUCCESS;
    }

    app_gaf_free_buff(p_app_bc_deleg_env);
    p_app_bc_deleg_env = NULL;
    return BT_STS_SUCCESS;
}

int app_bap_bc_deleg_source_add(app_gaf_bap_adv_id_t *addr, const app_gaf_bap_bcast_id_t *p_bcast_id,
                                uint8_t nb_subgroups, uint8_t *add_metadata, uint8_t add_metadata_len, uint16_t streaming_context_bf,
                                uint32_t bis_sync_bf_deprecated)
{
    uint16_t status = BT_STS_SUCCESS;

    bap_bc_pa_addr_t pa_addr = *(bap_bc_pa_addr_t *)addr;

    bap_bc_sub_grp_t *p_sub_grp = (bap_bc_sub_grp_t *)app_gaf_malloc_buff(sizeof(bap_bc_sub_grp_t) * nb_subgroups);

    if (p_sub_grp == NULL)
    {
        LOG_E("%s no resources", __func__);
        return BT_STS_NO_RESOURCES;
    }

    memset(p_sub_grp, 0, sizeof(bap_bc_sub_grp_t) * nb_subgroups);

    uint8_t i = 0;

    for (i = 0; i < nb_subgroups; i++)
    {
        p_sub_grp[i].p_metadata = (gen_aud_metadata_t *)app_gaf_malloc_buff(sizeof(gen_aud_metadata_t) + add_metadata_len);

        if (p_sub_grp[i].p_metadata == NULL)
        {
            status = BT_STS_NO_RESOURCES;
            break;
        }

        memset(p_sub_grp[i].p_metadata, 0, sizeof(gen_aud_metadata_t) + add_metadata_len);

        gen_aud_init_metadata(p_sub_grp[i].p_metadata);

        // Assign value
        p_sub_grp[i].p_metadata->basic_metadata.streaming_audio_context = streaming_context_bf;

        if (add_metadata != NULL)
        {
            p_sub_grp[i].p_metadata->add_metadata.len = add_metadata_len;
            memcpy(p_sub_grp[i].p_metadata->add_metadata.data, add_metadata, add_metadata_len);
        }

        p_sub_grp[i].bis_sync_req_bf = bis_sync_bf_deprecated;
    }

    if (status == BT_STS_SUCCESS)
    {
        status = bap_scan_deleg_add_source(&pa_addr, p_bcast_id->id, true, nb_subgroups, p_sub_grp);
    }

    for (i = 0; i < nb_subgroups; i++)
    {
        if (p_sub_grp[i].p_metadata != NULL)
        {
            app_gaf_free_buff(p_sub_grp[i].p_metadata);
        }
    }

    app_gaf_free_buff(p_sub_grp);

    return status;
}

int app_bap_bc_deleg_source_modify(uint8_t src_lid, uint8_t nb_subgroups, uint8_t *add_metadata,
                                   uint8_t add_metadata_len, uint16_t streaming_context_bf, uint32_t bis_sync_bf_deprecated)
{
    uint16_t status = BT_STS_SUCCESS;

    bap_bc_sub_grp_t *p_sub_grp = (bap_bc_sub_grp_t *)app_gaf_malloc_buff(sizeof(bap_bc_sub_grp_t) * nb_subgroups);

    if (p_sub_grp == NULL)
    {
        LOG_E("%s no resources", __func__);
        return BT_STS_NO_RESOURCES;
    }

    memset(p_sub_grp, 0, sizeof(bap_bc_sub_grp_t) * nb_subgroups);

    uint8_t i = 0;

    for (i = 0; i < nb_subgroups; i++)
    {
        p_sub_grp[i].p_metadata = (gen_aud_metadata_t *)app_gaf_malloc_buff(sizeof(gen_aud_metadata_t) + add_metadata_len);

        if (p_sub_grp[i].p_metadata == NULL)
        {
            status = BT_STS_NO_RESOURCES;
            break;
        }

        memset(p_sub_grp[i].p_metadata, 0, sizeof(gen_aud_metadata_t) + add_metadata_len);

        gen_aud_init_metadata(p_sub_grp[i].p_metadata);

        // Assign value
        p_sub_grp[i].p_metadata->basic_metadata.streaming_audio_context = streaming_context_bf;
        p_sub_grp[i].p_metadata->add_metadata.len = add_metadata_len;
        memcpy(p_sub_grp[i].p_metadata->add_metadata.data, add_metadata, add_metadata_len);
    }

    if (status == BT_STS_SUCCESS)
    {
        status = bap_scan_deleg_modify_source(src_lid, true, nb_subgroups, p_sub_grp);
    }

    for (i = 0; i < nb_subgroups; i++)
    {
        if (p_sub_grp[i].p_metadata != NULL)
        {
            app_gaf_free_buff(p_sub_grp[i].p_metadata);
        }
    }

    app_gaf_free_buff(p_sub_grp);

    return status;
}

int app_bap_bc_deleg_source_remove(uint8_t src_lid)
{
    return bap_scan_deleg_remove_source(src_lid);
}

int app_bap_bc_deleg_pref_bis_sync_cfm(uint8_t src_lid, uint32_t upper_pref_bis_sync)
{
    return bap_scan_deleg_bis_sync_pref_upper_cfm(src_lid, upper_pref_bis_sync);
}

int app_bap_bc_deleg_restart_src_big_sync(uint8_t src_lid)
{
    if (p_app_bc_deleg_env == NULL)
    {
        return BT_STS_NOT_READY;
    }

    const uint8_t *p_bcast_code = p_app_bc_deleg_env->broadcast_code[src_lid % APP_BAP_DFT_BC_DELEG_NB_SRCS];

    if (memcmp(p_bcast_code, invalid_bcast_code, APP_GAP_KEY_LEN) == 0)
    {
        return BT_STS_NOT_ALLOW;
    }

    return bap_scan_deleg_set_broadcast_code(src_lid, p_bcast_code);
}

int app_bap_bc_deleg_set_solicite_adv_data(char *adv_data, uint8_t adv_data_len)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_bap_bc_deleg_set_adv_params(app_gaf_bap_bc_adv_param_t *adv_param)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_bap_bc_deleg_set_sync_params(app_gaf_bap_bc_deleg_pa_sync_t *sync_param)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_bap_bc_deleg_start_solicite(uint16_t timeout_s, uint32_t context_bf)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_bap_bc_deleg_stop_solicite(void)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_bap_bc_assist_get_cfg_cmd(uint8_t con_lid, uint8_t src_lid)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_bap_bc_assist_set_cfg_cmd(uint8_t con_lid, uint8_t src_lid, uint8_t enable)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_bap_bc_deleg_restore_bond_data_req(uint8_t con_lid, uint8_t cfg_bf)
{
    return bap_scan_deleg_restore_cli_cfg_cache(con_lid, cfg_bf);
}

#endif
#endif

/// @} APP

