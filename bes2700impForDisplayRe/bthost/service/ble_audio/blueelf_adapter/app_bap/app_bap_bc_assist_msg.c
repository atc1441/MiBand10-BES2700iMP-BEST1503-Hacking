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
#ifdef APP_BLE_BIS_ASSIST_ENABLE
#include "ble_app_dbg.h"

#include "app_gaf.h"
#include "app_gaf_custom_api.h"

#include "app_bap_bc_assist_msg.h"

#include "bap_broadcast_assist.h"
#include "bap_broadcast_scan.h"

#define APP_BAP_BC_ASSIST_RX_STATE_SUPP_MAX    (1)
#define APP_BAP_BC_ASSIST_WRITE_CP_RELIABLE    (1)
#define APP_BAP_BC_ASSIST_PA_SYNC_TIMEOUT_MS   (1000)

/// Broadcast Sources information
typedef struct app_bc_srcs_info
{
    /// Connection local index
    uint8_t con_lid;
    /// Broadcast source number of specific delegator
    uint8_t nb_srcs;
    /// Src lid
    uint8_t src_lid[APP_BAP_BC_ASSIST_RX_STATE_SUPP_MAX];
    /// true: Assistant is scanning for delegator, else NOT
    bool is_deleg_scan;
} app_bc_srcs_info_t;

/// Content of Broadcast Assistant environment
typedef struct app_bap_bc_assist_env
{
    /// Preferred Mtu
    uint8_t preferred_mtu;
    /// Broadcast Sources Information
    app_bc_srcs_info_t bc_srcs[BLE_CONNECTION_MAX];
} app_bap_bc_assist_env_t;

app_bap_bc_assist_env_t *p_app_bc_assist_env = NULL;

/*Functions*/
bool app_bap_bc_assist_is_deleg_scan(void)
{
    for (uint8_t con_idx = 0; con_idx < BLE_CONNECTION_MAX; con_idx++)
    {
        if (true == p_app_bc_assist_env->bc_srcs[con_idx].is_deleg_scan)
        {
            return true;
        }
    }

    return false;
}

int app_bap_bc_set_assist_is_deleg_scan(uint8_t con_lid, bool is_deleg_scan)
{
    p_app_bc_assist_env->bc_srcs[con_lid].is_deleg_scan = is_deleg_scan;
    return 0;
}

/// Callback for bass discovery cmp
static void app_bap_bc_assist_cb_discovery_cmp(uint8_t con_lid, uint16_t err_code)
{
    LOG_I("%s con_lid = %d, status = %d", __func__, con_lid, err_code);
}

/// Callback for bass bond data
static void app_bap_bc_assist_cb_bond_data(uint8_t con_lid, const bap_bc_assist_prf_svc_t *p_svc_info)
{
    /* note: nb_rx_state is nb_srcs */
    app_gaf_bap_bc_assist_bond_data_ind_t found_ind =
    {
        .con_lid = con_lid,
        .bass_info.svc_info.shdl = p_svc_info->svc_range.shdl,
        .bass_info.svc_info.ehdl = p_svc_info->svc_range.ehdl,
    };

    app_gaf_evt_report(APP_GAF_ASSIST_BOND_DATA_IND, (void *)&found_ind, sizeof(found_ind));
}

/// Callback for basc gatt set cfg cmp evt
static void app_bap_bc_assist_cb_set_cfg_cmp_evt(uint8_t con_lid, uint8_t char_type, uint16_t err_code)
{
    LOG_I("%s con_lid = %d, char_type = %d, err_code = %d", __func__, con_lid, char_type, err_code);
}

/// Callback for bass wr scan cp err occure
static void app_bap_bc_assist_cb_scan_cp_err_ind(uint8_t con_lid, uint8_t src_lid, uint8_t src_id, uint8_t op_code, uint16_t err_code)
{
    LOG_I("%s con_lid = %d, src_lid = %d, op_code = %d, err_code = %d", __func__, con_lid, src_lid, op_code, err_code);
}

/// Callback for bass rx state empty
static void app_bap_bc_assist_cb_empty_rx_state_ind(uint8_t con_lid, uint8_t src_lid)
{
    LOG_I("%s con_lid = %d, src_lid = %d", __func__, con_lid, src_lid);

    p_app_bc_assist_env->bc_srcs[con_lid].con_lid = con_lid;

    if (p_app_bc_assist_env->bc_srcs[con_lid].src_lid[src_lid] == APP_BAP_BC_ASSIST_RX_STATE_SUPP_MAX)
    {
        p_app_bc_assist_env->bc_srcs[con_lid].src_lid[src_lid] = src_lid;
        p_app_bc_assist_env->bc_srcs[con_lid].nb_srcs++;
    }
}

/// Callback for bass rx src ind
static void app_bap_bc_assist_cb_rx_src_info_ind(uint8_t con_lid, uint8_t src_lid, uint8_t src_id, const bap_bc_bass_src_info_ind_ptr_t *p_src_info)
{
    LOG_I("%s con_lid = %d, src_lid = %d", __func__, con_lid, src_lid);
    LOG_I("pa sync state = %d, big encryption = %d, subgrp number = %d",
          p_src_info->pa_sync_state, p_src_info->big_encryption, p_src_info->sub_grp_num);

    if (p_src_info->p_subgrp_list != NULL)
    {
        LOG_I("[0]bis sync state: %d", p_src_info->p_subgrp_list[0].bis_sync_req_bf);
    }
}

/// Callback for bass rx src broadcast code get
static void app_bap_bc_assist_cb_broadcast_code_req(uint8_t con_lid, uint8_t src_lid, uint8_t src_id)
{
    app_gaf_bap_bc_assist_bcast_code_req_ind_t bcast_code_ri =
    {
        .con_lid = con_lid,
        .src_lid = src_lid,
    };

    app_gaf_evt_report(APP_GAF_ASSIST_BCAST_CODE_RI, (void *)&bcast_code_ri, sizeof(bcast_code_ri));
}

static void app_bap_bc_assist_cb_prf_status_event(uint8_t con_lid, bool is_central, gatt_prf_status_event_e event)
{
    if (event == GATT_PRF_STATUS_EVENT_OPENED && is_central == true)
    {
        //bap_bc_assist_bass_discovery(con_lid);
    }
}

static const bap_bc_assist_evt_cb_t app_bap_bc_assist_cb =
{
    .cb_bond_data = app_bap_bc_assist_cb_bond_data,
    .cb_discovery_cmp = app_bap_bc_assist_cb_discovery_cmp,
    .cb_set_cfg_cmp = app_bap_bc_assist_cb_set_cfg_cmp_evt,
    .cb_rx_state_empty_ind = app_bap_bc_assist_cb_empty_rx_state_ind,
    .cb_scan_cp_err_ind = app_bap_bc_assist_cb_scan_cp_err_ind,
    .cb_rx_src_info_ind = app_bap_bc_assist_cb_rx_src_info_ind,
    .cb_broadcast_code_req = app_bap_bc_assist_cb_broadcast_code_req,
    .cb_prf_status_event = app_bap_bc_assist_cb_prf_status_event,
};

int app_bap_bc_assist_init(void)
{
    LOG_I("%s", __func__);

    bap_bc_assist_init_cfg_t init_cfg =
    {
        .cp_write_reliable = APP_BAP_BC_ASSIST_WRITE_CP_RELIABLE,
        .max_supp_rx_state = APP_BAP_BC_ASSIST_RX_STATE_SUPP_MAX,
        .pa_sync_timeout_10ms = (APP_BAP_BC_ASSIST_PA_SYNC_TIMEOUT_MS / 10),
        .scan_pa_use_filter_list = false,
        .scan_for_pa_to_10ms = 0,
    };

    return bap_bc_assist_init(&init_cfg, &app_bap_bc_assist_cb);
}

int app_bap_bc_assist_deinit(void)
{
    LOG_I("%s", __func__);

    return bap_bc_assist_deinit();
}

int app_bap_bc_assist_info_init(void)
{
    LOG_I("%s", __func__);

    if (p_app_bc_assist_env == NULL)
    {
        p_app_bc_assist_env = (app_bap_bc_assist_env_t *)app_gaf_malloc_buff(sizeof(app_bap_bc_assist_env_t));

        if (p_app_bc_assist_env == NULL)
        {
            LOG_E("%s no resources", __func__);
            return BT_STS_NO_RESOURCES;
        }

        memset(p_app_bc_assist_env, 0, sizeof(app_bap_bc_assist_env_t));

        p_app_bc_assist_env->preferred_mtu = APP_GAF_DFT_PREF_MTU;

        for (uint8_t con_idx = 0; con_idx < BLE_CONNECTION_MAX; con_idx++)
        {
            p_app_bc_assist_env->bc_srcs[con_idx].con_lid = APP_GAF_INVALID_CON_LID;
            p_app_bc_assist_env->bc_srcs[con_idx].is_deleg_scan = false;
            memset(p_app_bc_assist_env->bc_srcs[con_idx].src_lid,
                   APP_BAP_BC_ASSIST_RX_STATE_SUPP_MAX,
                   APP_BAP_BC_ASSIST_RX_STATE_SUPP_MAX);
        }
    }

    return BT_STS_SUCCESS;
}

int app_bap_bc_assist_info_deinit(void)
{
    LOG_I("%s", __func__);

    if (p_app_bc_assist_env == NULL)
    {
        return BT_STS_SUCCESS;
    }

    app_gaf_free_buff(p_app_bc_assist_env);

    p_app_bc_assist_env = NULL;

    return BT_STS_SUCCESS;
}

int app_bap_bc_assist_start(uint8_t con_lid)
{
    return bap_bc_assist_bass_discovery(con_lid);
}

int app_bap_bc_assist_update_scan(uint8_t scan_state)
{
    uint8_t started = false;

    uint16_t status = BT_STS_SUCCESS;

    if (APP_GAF_BAP_BC_SCAN_STATE_SCANNING == scan_state)
    {
        started = true;
    }

    for (uint8_t con_idx = 0; con_idx < BLE_CONNECTION_MAX; con_idx++)
    {
        if (true == p_app_bc_assist_env->bc_srcs[con_idx].is_deleg_scan)
        {
            status = bap_bc_assist_send_remote_scan_state(p_app_bc_assist_env->bc_srcs[con_idx].con_lid, started);

            app_gaf_bap_bc_assist_cmp_evt_t cmp_evt =
            {
                .cmd_code = APP_GAF_BAP_BC_ASSIST_UPDATE_SCAN,
                .con_lid = con_idx,
                .src_lid = 0,
                .status = status,
            };

            app_gaf_evt_report(APP_GAF_ASSIST_SOURCE_STATE_IND, (void *)&cmp_evt, sizeof(cmp_evt));

            if (status != BT_STS_SUCCESS)
            {
                LOG_E("%s status = %d", __func__, status);
                break;
            }

            p_app_bc_assist_env->bc_srcs[con_idx].is_deleg_scan = started;

            break;
        }
    }

    return status;
}

int app_bap_bc_assist_source_add(app_gaf_bap_bc_assist_add_src_t *p_src_add)
{
    uint16_t status = BT_STS_SUCCESS;

    uint8_t con_lid = 0;

    for (con_lid = 0; con_lid < BLE_CONNECTION_MAX; con_lid++)
    {
        if (true == p_app_bc_assist_env->bc_srcs[con_lid].is_deleg_scan)
        {
            if (((p_src_add->bis_sync_bf != 0) &&
                    (p_src_add->pa_sync == APP_GAF_BAP_BC_PA_SYNC_NO_SYNC)) ||
                    (p_src_add->nb_subgroups == 0))
            {
                LOG_W("app_bap bc assist source add params invalid");
                continue;
            }

            bap_bc_sub_grp_t *p_sub_grp = (bap_bc_sub_grp_t *)app_gaf_malloc_buff(sizeof(bap_bc_sub_grp_t) * p_src_add->nb_subgroups);

            if (p_sub_grp == NULL)
            {
                LOG_E("%s no resources", __func__);
                return BT_STS_NO_RESOURCES;
            }

            memset(p_sub_grp, 0, sizeof(bap_bc_sub_grp_t) * p_src_add->nb_subgroups);

            /// TODO: subgrp is not assisgn like this
            uint8_t subgrp_id = 0;

            for (subgrp_id = 0; subgrp_id < p_src_add->nb_subgroups; subgrp_id++)
            {
                p_sub_grp[subgrp_id].bis_sync_req_bf = p_src_add->bis_sync_bf;
                p_sub_grp[subgrp_id].p_metadata = NULL;
            }

            bap_bc_pa_addr_t pa_addr =
            {
                .adv_sid = p_src_add->adv_report.adv_id.adv_sid,
                .adv_addr.addr_type = p_src_add->adv_report.adv_id.addr_type
            };

            memcpy(pa_addr.adv_addr.addr, p_src_add->adv_report.adv_id.addr, sizeof(pa_addr.adv_addr.addr));

            bap_bc_bass_src_info_req_ptr_t src_info =
            {
                .p_broadcast_id = p_src_add->bcast_id.id,
                .pa_sync_req = p_src_add->pa_sync,
                .pa_interval = p_src_add->pa_intv_frames,
                .sub_grp_num = p_src_add->nb_subgroups,
                .p_subgrp_list = p_sub_grp,
                .p_src_addr = &pa_addr,
            };

            status = bap_bc_assist_add_source(con_lid, 0, &src_info);

            app_gaf_free_buff(p_sub_grp);

            app_gaf_bap_bc_assist_cmp_evt_t cmp_evt =
            {
                .cmd_code = APP_GAF_BAP_BC_ASSIST_ADD_SOURCE,
                .con_lid = con_lid,
                .src_lid = 0,
                .status = status,
            };

            app_gaf_evt_report(APP_GAF_ASSIST_SOURCE_STATE_IND, (void *)&cmp_evt, sizeof(cmp_evt));

            if (status != BT_STS_SUCCESS)
            {
                LOG_E("%s status = %d", __func__, status);
                return status;
            }
        }
    }

    return BT_STS_SUCCESS;
}

int app_bap_bc_assist_source_remove(uint8_t con_lid, uint8_t src_lid)
{
    uint16_t status = bap_bc_assist_remove_source(con_lid, src_lid);

    app_gaf_bap_bc_assist_cmp_evt_t cmp_evt =
    {
        .cmd_code = APP_GAF_BAP_BC_ASSIST_REMOVE_SOURCE,
        .con_lid = con_lid,
        .src_lid = 0,
        .status = status,
    };

    app_gaf_evt_report(APP_GAF_ASSIST_SOURCE_STATE_IND, (void *)&cmp_evt, sizeof(cmp_evt));

    return status;
}

int app_bap_bc_assist_get_state(uint8_t con_lid, uint8_t src_lid)
{
    uint16_t status = bap_bc_assist_bass_read_rx_state(con_lid, src_lid);

    app_gaf_bap_bc_assist_cmp_evt_t cmp_evt =
    {
        .cmd_code = APP_GAF_BAP_BC_ASSIST_GET_STATE,
        .con_lid = con_lid,
        .src_lid = 0,
        .status = status,
    };

    app_gaf_evt_report(APP_GAF_ASSIST_SOURCE_STATE_IND, (void *)&cmp_evt, sizeof(cmp_evt));

    return status;
}

int app_bap_bc_assist_start_scan(uint16_t timeout_s)
{
    uint16_t status = bap_bc_scan_start_scan(timeout_s * 1000 / 10, false);

    app_gaf_bap_bc_assist_cmp_evt_t cmp_evt =
    {
        .cmd_code = APP_GAF_BAP_BC_ASSIST_START_SCAN,
        .con_lid = 0,
        .src_lid = 0,
        .status = status,
    };

    app_gaf_evt_report(APP_GAF_ASSIST_SOURCE_STATE_IND, (void *)&cmp_evt, sizeof(cmp_evt));

    return status;
}

int app_bap_bc_assist_stop_scan(void)
{
    uint16_t status = bap_bc_scan_stop_scan();

    app_gaf_bap_bc_assist_cmp_evt_t cmp_evt =
    {
        .cmd_code = APP_GAF_BAP_BC_ASSIST_STOP_SCAN,
        .con_lid = 0,
        .src_lid = 0,
        .status = status,
    };

    app_gaf_evt_report(APP_GAF_ASSIST_SOURCE_STATE_IND, (void *)&cmp_evt, sizeof(cmp_evt));

    return status;
}

int app_bap_bc_assist_send_bcast_code(uint8_t con_lid, uint8_t src_lid, uint8_t *bcast_code)
{
    return bap_bc_assist_broadcast_code_req_upper_cfm(con_lid, src_lid, bcast_code);
}

#endif
#endif

/// @} APP

