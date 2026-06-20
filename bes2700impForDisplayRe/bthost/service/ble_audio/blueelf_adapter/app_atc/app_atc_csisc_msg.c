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
#include "app_gaf.h"
#include "ble_app_dbg.h"
#include "app_atc_csisc_msg.h"
#include "app_gaf_custom_api.h"
#include "nvrecord_ble.h"
#include "app_ble.h"
#include "csisc.h"
#include "atc_csis_coordinator.h"

static APP_ATC_CSISC_ENV_T app_atc_csisc_env = {0};

/// Callback for csisc bond data
static void app_atc_csisc_cb_bond_data_evt(uint8_t con_lid, uint8_t set_lid, const csisc_prf_svc_t *param)
{
    LOG_D("app_atc csisc service found con_lid = %d, set_lid = %d, shdl = %04x, ehdl = %04x",
          con_lid, set_lid, param->svc_range.shdl, param->svc_range.ehdl);

    memset(&app_atc_csisc_env.device_info[con_lid], 0, sizeof(APP_ATC_CSISC_DEV_INFO_T));
}

/// Callback for csisc discovery done
static void app_atc_csisc_cb_discovery_cmp_evt(uint8_t con_lid, uint16_t err_code)
{
    LOG_I("app_atc csisc discovery cmp, con_lid = %d, status = %d", con_lid, err_code);

    app_atc_csisc_env.device_info[con_lid].discover_cmp_done = true;

    app_gaf_csisc_discover_cmp_ind_t cmp_ind_evt;
    cmp_ind_evt.con_lid = con_lid;
    cmp_ind_evt.result = err_code;
    app_gaf_mobile_evt_report(APP_GAF_CSISC_SERVER_INIT_DONE_CMP_IND, (void *)&cmp_ind_evt, sizeof(app_gaf_csisc_discover_cmp_ind_t));
}

/// Callback for csisc op set lock complete
static void app_atc_csisc_cb_set_lock_cmp_evt(uint8_t con_lid, uint8_t set_lid, uint16_t err_code)
{
    LOG_D("app_atc csisc lock cmp, con_lid = %d, set_lid = %d, status = %d", con_lid, set_lid,
          err_code);
}

/// Callback for csisc gatt cmd complete, @see enum csisc_cmd_code
static void app_atc_csisc_cb_set_cfg_cmp_evt(uint8_t con_lid, uint8_t set_lid, uint8_t char_type,
                                             uint16_t err_code)
{
    LOG_D("app_atc csisc set cfg cmp, char_type = %d, con_lid = %d, set_lid = %d, status = %d",
          char_type, con_lid, set_lid, err_code);
}

/// Callback for csisc cccd cfg value received or read
static void app_atc_csisc_cb_cfg_evt(uint8_t con_lid, uint8_t set_lid, uint8_t char_type, bool enabled,
                                     uint16_t err_code)
{
    LOG_D("app_atc csisc cfg vaue, char_type = %d, con_lid = %d, set_lid = %d, enable_ntf = %d, status = %d",
          char_type, con_lid, set_lid, enabled, err_code);
}

/// Callback for csisc value ntf or read
static void app_atc_csisc_cb_value_evt(uint8_t con_lid, uint8_t set_lid, uint8_t char_type,
                                       const uint8_t *data, uint16_t len, uint16_t err_code)
{
    if (err_code != BT_STS_SUCCESS)
    {
        LOG_D("app_atc csisc_val status = %d, con_lid= %d, set_lid = %d, sirk:", err_code, con_lid,
              set_lid);
        return;
    }

    if (char_type == CSIS_CHARACTER_TYPE_SIRK)
    {
        LOG_D("app_atc csisc_sirk_ind con_lid= %d, set_lid = %d, sirk:", con_lid, set_lid);
        DUMP8("%02x ", data, len);

        if (len != 0)
        {
            app_gaf_csisc_sirk_ind_t sirk =
            {
                .con_lid = con_lid,
                .set_lid = set_lid,
            };

            memcpy(sirk.sirk.sirk, data, len);

            app_atc_csisc_env.device_info[con_lid].sets_info[set_lid].sirk = *(csisc_sirk_t *)&sirk.sirk;

            app_gaf_mobile_evt_report(APP_GAF_CSISC_SIRK_VALUE_IND, (void *)&sirk, sizeof(sirk));
        }

        return;
    }
    else if (char_type == CSIS_CHARACTER_TYPE_LOCK)
    {
        app_atc_csisc_env.device_info[con_lid].sets_info[set_lid].lock_status = *data;
    }
    else if (char_type == CSIS_CHARACTER_TYPE_SIZE)
    {
        app_atc_csisc_env.device_info[con_lid].sets_info[set_lid].grp_device_numbers = *data;
    }
    else if (char_type == CSIS_CHARACTER_TYPE_RANK)
    {
        app_atc_csisc_env.device_info[con_lid].sets_info[set_lid].rank_index = *data;
    }

    app_gaf_csisc_info_ind_t info =
    {
        .con_lid = con_lid,
        .set_lid = set_lid,
        .char_type = char_type,
        .val.val = *data,
    };

    LOG_D("app_atc csisc_info_ind con_lid= %d, set_lid = %d, char_type = %d, val = %d", con_lid,
          set_lid, char_type, *data);

    app_gaf_mobile_evt_report(APP_GAF_CSISC_CHAR_VALUE_RSULT_IND, (void *)&info, sizeof(info));
}

/// Callback for csisc sirk decrypt get ltk
static void app_atc_csisc_cb_get_ltk(uint8_t con_lid, uint16_t *status, uint8_t *ltk)
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
}

/// Callback for csisc rsi resolved
static void app_atc_csisc_cb_rsi_resolved(uint8_t con_lid, uint8_t set_lid, uint16_t err_code,
                                          const uint8_t *p_rsi)
{
    LOG_I("app_atc csisc resolve cmp, con_lid = %d set_lid = %d, status = %d", con_lid, set_lid,
          err_code);

    app_gaf_csisc_cmp_evt_t cmp_evt =
    {
        .set_lid = set_lid,
        .status = err_code,
        .lid.con_lid = con_lid,
    };

    app_gaf_mobile_evt_report(APP_GAF_CSISC_PSRI_RESOLVE_RESULT_IND, (void *)&cmp_evt, sizeof(cmp_evt));
}

static void app_atc_csisc_cb_prf_status_event(uint8_t con_lid, bool is_central, gatt_prf_status_event_e event)
{

}

const csisc_evt_cb_t csisc_evt_cb =
{
    .cb_bond_data = app_atc_csisc_cb_bond_data_evt,
    .cb_cfg_value = app_atc_csisc_cb_cfg_evt,
    .cb_char_value = app_atc_csisc_cb_value_evt,
    .cb_discovery_cmp = app_atc_csisc_cb_discovery_cmp_evt,
    .cb_ltk = app_atc_csisc_cb_get_ltk,
    .cb_rsi_resolved = app_atc_csisc_cb_rsi_resolved,
    .cb_set_cfg_cmp = app_atc_csisc_cb_set_cfg_cmp_evt,
    .cb_set_lock_val_cmp = app_atc_csisc_cb_set_lock_cmp_evt,
    .cb_prf_status_event = app_atc_csisc_cb_prf_status_event,
};

int app_atc_csisc_init(void)
{
    uint16_t status = BT_STS_SUCCESS;

    LOG_I("app_atc csisc init");
    status = csisc_atc_init();

    csisc_cfg_t init_cfg;

    if (status == BT_STS_SUCCESS)
    {
        status = csisc_init(&init_cfg, &csisc_evt_cb);
    }

    return status;
}

int app_atc_csisc_deinit(void)
{
    uint16_t status = BT_STS_SUCCESS;

    LOG_I("app_atc csisc deinit");
    status = csisc_deinit();

    if (status == BT_STS_SUCCESS)
    {
        status = csisc_atc_deinit();
    }

    return status;
}

int app_atc_csisc_start(uint8_t con_lid)
{
    return csisc_service_discovery(con_lid);
}

int app_atc_csisc_resolve(uint8_t *rsi)
{
    uint8_t resolve_task_cnt = 0;
    uint8_t con_lid = 0;
    uint8_t set_lid = 0;
    uint16_t status = BT_STS_SUCCESS;

    for (con_lid = 0; con_lid < CSISC_CONNECTION_MAX; con_lid++)
    {
        for (set_lid = 0; set_lid < CSIS_SET_MEMBER_INST_NUMER_MAX; set_lid++)
        {
            status = csisc_rsi_resolve_use_set_sirk(con_lid, set_lid, rsi);

            if (status != BT_STS_SUCCESS)
            {
                break;
            }

            resolve_task_cnt++;
        }
    }

    if (resolve_task_cnt == 0)
    {
        return BT_STS_FAILED;
    }

    return BT_STS_SUCCESS;
}

int app_atc_csisc_lock(uint8_t con_lid, uint8_t set_lid, uint8_t lock)
{
    return csisc_set_lock_value(con_lid, set_lid, lock);
}

int app_atc_csisc_get(uint8_t con_lid, uint8_t set_lid, uint8_t char_type)
{
    uint16_t status = BT_STS_SUCCESS;

    uint8_t val;

    uint8_t sirk[CSIS_SIRK_VAL_LEN] = {0};

    uint8_t *p_data = &val;
    uint8_t len = sizeof(uint8_t);

    if (char_type == CSIS_CHARACTER_TYPE_LOCK)
    {
        status = csisc_get_set_member_lock(con_lid, set_lid, (bool *)&val);
    }
    else if (char_type == CSIS_CHARACTER_TYPE_RANK)
    {
        status = csisc_get_set_member_rank(con_lid, set_lid, &val);
    }
    else if (char_type == CSIS_CHARACTER_TYPE_SIZE)
    {
        status = csisc_get_set_member_size(con_lid, set_lid, &val);
    }
    else if (char_type == CSIS_CHARACTER_TYPE_SIRK)
    {
        status = csisc_get_set_member_sirk(con_lid, set_lid, sirk);

        if (status == BT_STS_PENDING)
        {
            return BT_STS_SUCCESS;
        }

        p_data = sirk;
        len = CSIS_SIRK_VAL_LEN;
    }

    /// TODO: May use in bt thread
    app_atc_csisc_cb_value_evt(con_lid, set_lid, char_type, p_data, len, status);

    return status;
}

int app_atc_csisc_get_cfg(uint8_t con_lid, uint8_t set_lid, uint8_t char_type)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_atc_csisc_set_cfg(uint8_t con_lid, uint8_t set_lid, uint8_t char_type, uint8_t enable)
{
    return csisc_character_cccd_write(con_lid, set_lid, char_type, enable);
}

int app_atc_csisc_add_sirk(uint8_t *sirk)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_atc_csisc_remove_sirk(uint8_t key_lid)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

APP_ATC_CSISC_ENV_T *app_atc_csisc_get_env()
{
    return &app_atc_csisc_env;
}

#endif
#endif

/// @} APP
