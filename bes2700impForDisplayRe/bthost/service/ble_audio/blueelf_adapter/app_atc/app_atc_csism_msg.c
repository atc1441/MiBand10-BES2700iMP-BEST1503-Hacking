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
#include "app_gaf.h"
#include "ble_app_dbg.h"
#include "app_atc_csism_msg.h"
#include "app_gaf_custom_api.h"
#include "app_ble.h"
#include "nvrecord_ble.h"
#include "atc_csis_member.h"

static csism_info_user_config_cb app_atc_csism_user_config_cb = NULL;

APP_ATC_CSISM_ENV_T app_atc_csism_env = {0};

void app_atc_csism_cb_bond_data(uint8_t con_lid, uint8_t char_type_written, uint16_t cli_cfg_bf)
{
    LOG_D("app_atc csism bond_data_ind con_lid= %d, set_lid = %d, cli_cfg_bf = %02x",
          con_lid, app_atc_csism_env.active_set_lid, cli_cfg_bf);

    app_gaf_atc_csism_bond_data_ind_t bond_data =
    {
        .con_lid = con_lid,
        .set_lid = app_atc_csism_env.active_set_lid,
        .cli_cfg_bf = cli_cfg_bf,
    };

    app_gaf_evt_report(APP_GAF_CSISM_BOND_DATA_IND, (void *)&bond_data, sizeof(bond_data));
}

void app_atc_csism_cb_char_val_sent(uint8_t con_lid, bool is_read_rsp, uint8_t char_type, const uint8_t *p_val_rsp)
{
    LOG_D("app_atc csism val sent con_lid= %d, set_lid = %d",
          con_lid, app_atc_csism_env.active_set_lid);

    if (is_read_rsp == true)
    {
        app_gaf_atc_csism_read_rsp_sent_t read_rsp_ind =
        {
            .con_lid = con_lid,
            .char_type = char_type,
            .data_len = (char_type == CSIS_CHARACTER_TYPE_SIRK) ? APP_GAF_CSIS_SIRK_LEN_VALUE : sizeof(uint8_t),
        };
        ///TODO:enc sirk contains one octect type:enc or plain text
        memcpy(read_rsp_ind.data, p_val_rsp, read_rsp_ind.data_len);

        app_gaf_evt_report(APP_GAF_CSISM_READ_RSP_SENT_IND, (void *)&read_rsp_ind, sizeof(app_gaf_atc_csism_read_rsp_sent_t));
    }
    else
    {
        app_gaf_atc_csism_ntf_sent_t ntf_sent_ind =
        {
            .con_lid = con_lid,
            .char_type = char_type,
        };

        app_gaf_evt_report(APP_GAF_CSISM_NTF_SENT_IND, (void *)&ntf_sent_ind, sizeof(app_gaf_atc_csism_ntf_sent_t));
    }
}

POSSIBLY_UNUSED void app_atc_csism_cb_ltk_get(uint8_t con_lid, uint16_t *status, uint8_t *ltk)
{
    ble_bdaddr_t remote_addr = {{0}};
    bool ret = app_ble_get_peer_solved_addr(con_lid, &remote_addr);

    if (ret == true)
    {
        ret = nv_record_ble_record_find_ltk(remote_addr.addr, ltk, 0);
    }

    if (ret == false)
    {
        *status = BT_STS_NOT_FOUND;
    }
    else
    {
        *status = BT_STS_SUCCESS;
    }

    app_gaf_atc_csism_ltk_req_ind_t ltk_req =
    {
        .con_lid = con_lid,
        .set_lid = app_atc_csism_env.active_set_lid,
    };

    app_gaf_evt_report(APP_GAF_CSISM_LTK_RI, (void *)&ltk_req, sizeof(ltk_req));
}

void app_atc_csism_cb_rsi_generated(uint16_t err_code, uint8_t *rsi)
{
    LOG_D("app_atc csism rsi_ind set_lid:%d", app_atc_csism_env.active_set_lid);
    DUMP8("%02x", rsi, CSIS_RSI_LEN);

    app_gaf_atc_csism_rsi_ind_t rsi_evt =
    {
        .set_lid = app_atc_csism_env.active_set_lid,
    };

    memcpy(rsi_evt.rsi.rsi, rsi, CSIS_RSI_LEN);

    app_gaf_evt_report(APP_GAF_CSISM_NEW_RSI_GENERATED_IND, (void *)&rsi_evt, sizeof(rsi_evt));
}

void app_atc_csism_cb_lock_state(uint8_t con_lid, bool lock)
{
    LOG_D("app_atc csism lock_ind, con_lid = %d, set_lid = %d, lock = %d, reason = %d",
          con_lid, app_atc_csism_env.active_set_lid, lock, 0xFF);

    app_gaf_atc_csism_lock_ind_t lock_evt =
    {
        .con_lid = con_lid,
        .lock = lock,
        .set_lid = app_atc_csism_env.active_set_lid,
        /// TODO: timeout or peer write
        .reason = 0,
    };

    app_gaf_evt_report(APP_GAF_CSISM_LOCK_IND, (void *)&lock_evt, sizeof(lock_evt));
}

static const csism_atc_evt_cb_t csism_evt_cb =
{
    .cb_bond_data = app_atc_csism_cb_bond_data,
    .cb_val_sent = app_atc_csism_cb_char_val_sent,
    .cb_lock = app_atc_csism_cb_lock_state,
    .cb_ltk_get = app_atc_csism_cb_ltk_get,
    .cb_rsi = app_atc_csism_cb_rsi_generated,
};

int app_atc_csism_init(void)
{
    LOG_I("app_atc csism init");

    CSISM_SET_INFO_CONFIG_T cfg_info = {0};

    if (app_atc_csism_user_config_cb != NULL)
    {
        app_atc_csism_user_config_cb(&cfg_info);
    }

    csism_atc_init_cfg_t csism_init_cfg =
    {
        .lock_bf = 0,
        .rank = cfg_info.rank_index,
        .size = cfg_info.group_include_dev_nb,
        .sirk_encrypt_needed = true,
        .pref_mtu = GAF_PREFERRED_MTU,
    };

    memcpy(csism_init_cfg.sirk, cfg_info.sirk_value, CSIS_SIRK_VAL_LEN);

    uint8_t set_lid = 0;

    uint16_t status = csism_atc_init(&csism_init_cfg, &csism_evt_cb, &set_lid);

    if (status == BT_STS_SUCCESS)
    {
        app_atc_csism_env.active_set_lid = set_lid;
        memcpy(&app_atc_csism_env.config_info[0], &cfg_info, sizeof(CSISM_SET_INFO_CONFIG_T));
    }

    return status;
}

int app_atc_csism_deinit(void)
{
    LOG_I("app_atc csism deinit");

    return csism_atc_deinit();
}

int app_atc_csism_set_sirk(uint8_t set_lid, uint8_t *sirk)
{
    if (app_atc_csism_env.active_set_lid != set_lid)
    {
        LOG_E("%s invalid set_lid = %d", __func__, set_lid);
        return BT_STS_INVALID_PARM;
    }

    return csism_atc_set_sirk(sirk);
}

int app_atc_csism_update_rsi(uint8_t set_lid)
{
    if (app_atc_csism_env.active_set_lid != set_lid)
    {
        LOG_E("%s invalid set_lid = %d", __func__, set_lid);
        return BT_STS_INVALID_PARM;
    }

    return csism_atc_generate_rsi();
}

int app_atc_csism_set_size(uint8_t set_lid, uint8_t size)
{
    int ret;

    if (app_atc_csism_env.active_set_lid != set_lid)
    {
        LOG_E("%s invalid set_lid = %d", __func__, set_lid);
        return BT_STS_INVALID_PARM;
    }

    ret = csism_atc_set_size(size);
    if (ret == BT_STS_SUCCESS)
    {
        app_atc_csism_env.config_info[0].group_include_dev_nb = size;
    }

    return ret;
}

int app_atc_csism_set_rank(uint8_t set_lid, uint8_t rank)
{
    if (app_atc_csism_env.active_set_lid != set_lid)
    {
        LOG_E("%s invalid set_lid = %d", __func__, set_lid);
        return BT_STS_INVALID_PARM;
    }

    return csism_atc_set_rank(rank);
}

void app_atc_csism_set_lock(uint8_t con_lid, bool lock)
{
    csism_atc_set_lock(con_lid, lock);
}

int app_atc_csism_restore_bond_data_req(uint8_t con_lid, uint8_t set_lid, uint8_t is_locked,
                                        uint8_t cli_cfg_bf)
{
    return csism_atc_restore_cli_cfg_cache(con_lid, is_locked, cli_cfg_bf);
}

// app_atc_csism_register_sets_config_cb() need be called before app_atc_csism_init()
int app_atc_csism_register_sets_config_cb(csism_info_user_config_cb func_cb)
{
    LOG_I("%s old cb: [%p], new cb: [%p]", __func__, app_atc_csism_user_config_cb, func_cb);
    app_atc_csism_user_config_cb = func_cb;

    return BT_STS_SUCCESS;
}

APP_ATC_CSISM_ENV_T *app_atc_csism_get_env(void)
{
    return &app_atc_csism_env;
}

#endif

/// @} APP
