/**
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
#include "hal_timer.h"
#include "tgt_hardware.h"
#include "bluetooth_bt_api.h"
#include "app_bt_func.h"
#include "nvrecord_ble.h"
#include "ble_audio_core_api.h"
#include "ble_audio_earphone_info.h"
#include "ble_audio_mobile_info.h"

#include "app_ble.h"
#include "app_gaf_dbg.h"
#include "app_gaf_custom_api.h"
#include "aob_mgr_gaf_evt.h"
#include "aob_csip_api.h"
#include "aob_conn_api.h"

/************************private macro defination***************************/

/************************private variable defination****************************/
static AOB_CSIP_ENV_T aob_csip_earphone_env = {0};
bool is_use_custom_sirk = false;
/****************************function defination****************************/

// UI layer can periodically update RSI adv value if need, for example: update the RSI data every 15min
void aob_csip_if_update_rsi_data()
{
    uint8_t set_lid = aob_csip_earphone_env.gaf_csism_scb->active_set_lid;
    app_atc_csism_update_rsi(set_lid);
}

// add api for UI layer to achieve the RSI value
bool aob_csip_if_get_rsi_data(uint8_t *p_rsi_get)
{
    // Empty value rsi
    const uint8_t RSI_EMPTY[APP_ATC_CSIS_RSI_LEN] = {0};

    if (memcmp(aob_csip_earphone_env.rsi, RSI_EMPTY, APP_ATC_CSIS_RSI_LEN) == 0)
    {
        return false;
    }

    if (p_rsi_get != NULL)
    {
        memcpy(p_rsi_get, aob_csip_earphone_env.rsi, APP_ATC_CSIS_RSI_LEN);
    }

    return true;
}

// UI layer can update SIRK value dynamically, if update it, need update all devcies SIRK value of the same group.
// Usually the SIRK has been configured during device power on.
void aob_csip_if_update_sirk(uint8_t *sirk, uint8_t sirk_len)
{
    if ((!sirk) || (sirk_len != APP_ATC_CSIS_SIRK_LEN))
    {
        LOG_I("[%s], sirk param is invalid! sirk=%p, len=%d", __func__, sirk, sirk_len);
        return;
    }

    if (!aob_csip_earphone_env.gaf_csism_scb)
    {
        return;
    }
    uint8_t set_lid = aob_csip_earphone_env.gaf_csism_scb->active_set_lid;
    LOG_I("aob_csip_if_update_sirk:");
    DUMP8("%02x ", sirk, 16);
    app_atc_csism_set_sirk(set_lid, sirk);

    const BLE_AUD_CORE_EVT_CB_T *p_cbs = ble_audio_get_evt_cb();
    if (p_cbs)
    {
        p_cbs->ble_tws_sirk_refreshed();
    }
}

bool aob_csip_if_get_sirk(uint8_t *sirk)
{
    NV_RECORD_BLE_AUDIO_DEV_INFO_T *p_bleaudio_info = nv_record_bleaudio_get_ptr();
    if (!p_bleaudio_info)
    {
        return false;
    }
    memcpy(sirk, p_bleaudio_info->sirk, APP_GAF_CSIS_SIRK_LEN_VALUE);
    return true;
}

bool aob_csip_if_set_device_numbers(uint8_t dev_num)
{
    APP_ATC_CSISM_ENV_T *csip_env = app_atc_csism_get_env();

    if (!csip_env)
    {
        return false;
    }

    if (BT_STS_SUCCESS == app_atc_csism_set_size(csip_env->active_set_lid, dev_num))
    {
        return true;
    }

    return false;
}

uint8_t aob_csip_if_get_device_numbers(void)
{
     APP_ATC_CSISM_ENV_T *csip_env = app_atc_csism_get_env();

    if (!csip_env)
    {
        return false;
    }

    return csip_env->config_info[0].group_include_dev_nb;
}

void aob_csip_config_is_use_custom_sirk(bool use_custom_sirk)
{
    is_use_custom_sirk = use_custom_sirk;
}

void aob_csip_set_custom_sirk(uint8_t *sirk)
{
    NV_RECORD_BLE_AUDIO_DEV_INFO_T pInfo;
    NV_RECORD_BLE_AUDIO_DEV_INFO_T *pCurrentLeAudioNvInfo =
        nv_record_bleaudio_get_ptr();
    if (pCurrentLeAudioNvInfo)
    {
        pInfo = *pCurrentLeAudioNvInfo;
    }
    memcpy(pInfo.sirk, sirk, APP_GAF_CSIS_SIRK_LEN_VALUE);
    LOG_I("%s sirk:", __func__);
    DUMP8("%02x ", sirk, 16);
    nv_record_bleaudio_update_devinfo((uint8_t *)&pInfo);
    aob_csip_if_update_sirk(sirk, APP_GAF_CSIS_SIRK_LEN_VALUE);
}

bool aob_csip_is_use_custom_sirk(void)
{
    LOG_I("%s is_use_custom_sirk: %d", __func__, is_use_custom_sirk);
    return is_use_custom_sirk;
}

static void aob_csip_generate_random_sirk(uint8_t *sirk)
{
    uint32_t generatedSeed = hal_sys_timer_get();
    uint8_t index = 0;

    for (index = 0; index < sizeof(ble_global_addr); index++)
    {
        generatedSeed ^= (((uint32_t)(ble_global_addr[index])) << (hal_sys_timer_get() & 0xF));
    }
    srand(generatedSeed);

    for (index = 0; index < APP_GAF_CSIS_SIRK_LEN_VALUE; index++)
    {
        sirk[index] = (uint8_t)rand() % 255 + 1;
    }
}

void aob_csip_if_use_temporary_sirk(void)
{
    uint8_t sirk[APP_GAF_CSIS_SIRK_LEN_VALUE] = {0};

    aob_csip_generate_random_sirk(sirk);
    aob_csip_if_update_sirk(sirk, APP_GAF_CSIS_SIRK_LEN_VALUE);
}

void aob_csip_if_refresh_sirk(uint8_t *sirk)
{
    LOG_I("%s", __func__);
    if (!sirk)
    {
        LOG_I("[%s], sirk param is invalid! sirk=%p", __func__, sirk);
        return;
    }

    NV_RECORD_BLE_AUDIO_DEV_INFO_T pInfo;
    NV_RECORD_BLE_AUDIO_DEV_INFO_T *pCurrentLeAudioNvInfo =
        nv_record_bleaudio_get_ptr();
    if (pCurrentLeAudioNvInfo)
    {
        pInfo = *pCurrentLeAudioNvInfo;
    }
    pInfo.set_member = 1;
    memcpy(pInfo.sirk, sirk, APP_GAF_CSIS_SIRK_LEN_VALUE);
    nv_record_bleaudio_update_devinfo((uint8_t *)&pInfo);
    aob_csip_if_update_sirk(sirk, APP_GAF_CSIS_SIRK_LEN_VALUE);
}

void aob_csip_if_delete_nv_sirk(void)
{
    NV_RECORD_BLE_AUDIO_DEV_INFO_T *p_bleaudio_info = nv_record_bleaudio_get_ptr();

    if (!p_bleaudio_info)
    {
        return;
    }

    uint32_t lock = nv_record_pre_write_operation();
    memset(p_bleaudio_info->sirk, 0x00, APP_ATC_CSIS_SIRK_LEN);

    nv_record_update_runtime_userdata();
    nv_record_post_write_operation(lock);
    LOG_I("%s", __func__);
}

bool aob_csip_sirk_already_refreshed(void)
{
    NV_RECORD_BLE_AUDIO_DEV_INFO_T *pInfoLocal = nv_record_bleaudio_get_ptr();
    if (pInfoLocal == NULL)
    {
        return false;
    }

    if (pInfoLocal->set_member)
    {
        DUMP8("%02x ", pInfoLocal->sirk, 16);
        return true;
    }
    else
    {
        return false;
    }
}

void aob_csip_erase_sirk_record(void)
{
    NV_RECORD_BLE_AUDIO_DEV_INFO_T pInfo =
    {
        0, {0,}
    };

    nv_record_bleaudio_update_devinfo((uint8_t *)&pInfo);
}

// TODO: According to CSIS spec. Device will notify lock status after bonded
void aob_csip_if_restore_bond(uint8_t con_lid, uint8_t set_lid, bool locked)
{
    app_atc_csism_restore_bond_data(con_lid, set_lid, locked);
}

static void aob_csip_prsi_value_updated_cb(uint8_t *rsi, uint8_t rsi_len)
{
    LOG_I("%s", __func__);

    memcpy(aob_csip_earphone_env.rsi, rsi, rsi_len);

    const BLE_AUD_CORE_EVT_CB_T *p_cbs = ble_audio_get_evt_cb();

    if (p_cbs && NULL != p_cbs->ble_csip_rsi_updated_cb)
    {
        p_cbs->ble_csip_rsi_updated_cb(rsi);
    }
}

static void aob_csip_ntf_sent_cb(uint8_t con_lid, uint8_t char_type)
{
    const BLE_AUD_CORE_EVT_CB_T *p_cbs = ble_audio_get_evt_cb();
    if (p_cbs && NULL != p_cbs->ble_csip_ntf_sent_cb)
    {
        p_cbs->ble_csip_ntf_sent_cb(con_lid, char_type);
    }
}

static void aob_csip_read_rsp_sent_cb(uint8_t con_lid, uint8_t char_type, uint8_t *p_data, uint8_t data_len)
{
    const BLE_AUD_CORE_EVT_CB_T *p_cbs = ble_audio_get_evt_cb();
    if (p_cbs && NULL != p_cbs->ble_csip_read_rsp_sent_cb)
    {
        p_cbs->ble_csip_read_rsp_sent_cb(con_lid, char_type, p_data, data_len);
    }
}

static csip_event_handler_t csip_event_cb =
{
    .csip_rsi_value_updated_cb   = aob_csip_prsi_value_updated_cb,
    .csip_ntf_sent_cb            = aob_csip_ntf_sent_cb,
    .csip_read_rsp_sent_cb       = aob_csip_read_rsp_sent_cb,
};

static bool aob_csip_sets_info_user_config_cb(CSISM_SET_INFO_CONFIG_T *pinfo)
{
    pinfo->lock_timeout = AOB_CSIP_LOCK_TIMEOUT;
    pinfo->group_include_dev_nb = aob_csip_earphone_env.size;
    if (BLE_AUDIO_TWS_MASTER == ble_audio_get_tws_nv_role())
    {
        pinfo->rank_index = 0x01;
    }
    else
    {
        pinfo->rank_index = 0x02;
    }
    NV_RECORD_BLE_AUDIO_DEV_INFO_T *p_bleaudio_info = nv_record_bleaudio_get_ptr();
    if (!p_bleaudio_info)
    {
        return false;
    }

    memcpy(pinfo->sirk_value, p_bleaudio_info->sirk, APP_ATC_CSIS_SIRK_LEN);
    LOG_I("%s size:%d, rank:%d, sirk:", __func__, pinfo->group_include_dev_nb, pinfo->rank_index);
    DUMP8("%02x ", p_bleaudio_info->sirk, APP_ATC_CSIS_SIRK_LEN);
    return true;
}

void aob_csip_if_user_parameters_init(uint8_t csip_size)
{
    aob_csip_earphone_env.size = csip_size;
    app_atc_csism_register_sets_config_cb(aob_csip_sets_info_user_config_cb);
}

void aob_csip_if_register_sets_config_cb(csism_info_user_config_cb cb)
{
    app_atc_csism_register_sets_config_cb(cb);
}

void aob_csip_if_init(void)
{
    LOG_I("%s", __func__);
    aob_csip_earphone_env.gaf_csism_scb = app_atc_csism_get_env();
    aob_mgr_csip_evt_handler_register(&csip_event_cb);
    // trigger to adv rsi data
    aob_csip_if_update_rsi_data();
}

/*****************************************************************************************************************
*
*                        CSIP Set Role API&Callback Implementation (mobilephone side)                              *
*
******************************************************************************************************************/
#ifdef AOB_MOBILE_ENABLED
#include "app_gaf_custom_api.h"
#include "ble_audio_mobile_info.h"
#include "app_atc_csisc_msg.h"

static AOB_CSIP_MOBILE_ENV_T aob_csip_mobile_env = {0};

static void aob_csip_mobile_discover_members_timeout_cb(void const *n);
osTimerDef(AOB_CSIP_MOBILE_DISCOVER_MEMBERS_TIMER, aob_csip_mobile_discover_members_timeout_cb);
static void aob_csip_mobile_discover_members_timeout_cb(void const *n)
{
    LOG_W("scan csip group members timerout, stop scan!!!");
    app_ble_stop_scan();
    aob_csip_mobile_env.start_discover_memebers = false;
}

bool aob_csip_mobile_if_get_group_device_numbers(uint8_t con_lid, uint8_t *dev_nbs_out)
{
    bool ret = false;
    uint8_t dev_index = 0;
    for (dev_index = 0; dev_index < APP_ATC_CSIS_SETS_SIZE_MAX; dev_index++)
    {
        if (aob_csip_mobile_env.p_csisc_env->device_info[dev_index].discover_cmp_done && \
                aob_csip_mobile_env.p_csisc_env->device_info[dev_index].con_lid == con_lid)
        {
            uint8_t active_set = aob_csip_mobile_env.p_csisc_env->device_info[dev_index].active_sets_index;
            *dev_nbs_out =
                aob_csip_mobile_env.p_csisc_env->device_info[dev_index].sets_info[active_set].grp_device_numbers;
            ret = true;
            break;
        }
    }
    return ret;
}

bool aob_csip_mobile_if_get_device_rank_index(uint8_t con_lid, uint8_t *rank_index_out)
{
    bool ret = false;
    uint8_t dev_index = 0;
    for (dev_index = 0; dev_index < APP_ATC_CSIS_SETS_SIZE_MAX; dev_index++)
    {
        if (aob_csip_mobile_env.p_csisc_env->device_info[dev_index].discover_cmp_done && \
                aob_csip_mobile_env.p_csisc_env->device_info[dev_index].con_lid == con_lid)
        {
            uint8_t active_set = aob_csip_mobile_env.p_csisc_env->device_info[dev_index].active_sets_index;
            *rank_index_out =
                aob_csip_mobile_env.p_csisc_env->device_info[dev_index].sets_info[active_set].rank_index;
            ret = true;
            break;
        }
    }
    return ret;
}

bool aob_csip_mobile_if_get_device_lock_status(uint8_t con_lid, uint8_t *lock_status_out)
{
    bool ret = false;
    uint8_t dev_index = 0;
    for (dev_index = 0; dev_index < APP_ATC_CSIS_SETS_SIZE_MAX; dev_index++)
    {
        if (aob_csip_mobile_env.p_csisc_env->device_info[dev_index].discover_cmp_done && \
                aob_csip_mobile_env.p_csisc_env->device_info[dev_index].con_lid == con_lid)
        {
            uint8_t active_set = aob_csip_mobile_env.p_csisc_env->device_info[dev_index].active_sets_index;
            *lock_status_out =
                aob_csip_mobile_env.p_csisc_env->device_info[dev_index].sets_info[active_set].lock_status;
            ret = true;
            break;
        }
    }
    return ret;
}

bool aob_csip_mobile_if_get_sirk_value(uint8_t con_lid, uint8_t *sirk_value_out)
{
    LOG_I("%s", __func__);
    bool    ret = false;
    uint8_t dev_index = 0;
    for (dev_index = 0; dev_index < APP_ATC_CSIS_SETS_SIZE_MAX; dev_index++)
    {
        if (aob_csip_mobile_env.p_csisc_env->device_info[dev_index].discover_cmp_done && \
                aob_csip_mobile_env.p_csisc_env->device_info[dev_index].con_lid == con_lid)
        {
            uint8_t active_set = aob_csip_mobile_env.p_csisc_env->device_info[dev_index].active_sets_index;
            memcpy(sirk_value_out, aob_csip_mobile_env.p_csisc_env->device_info[dev_index].sets_info[active_set].sirk.sirk, 16);
            ret = true;
            break;
        }
    }
    return ret;
}

bool aob_csip_mobile_if_add_sirk(uint8_t *sirk)
{
    bool ret = false;
    if (sirk && !aob_csip_mobile_env.cmd_result_info.sirk_opcode_cmd.inprogress_flag)
    {
        aob_csip_mobile_env.cmd_result_info.sirk_opcode_cmd.inprogress_flag = true;
        memcpy(aob_csip_mobile_env.cmd_result_info.sirk_opcode_cmd.sirk, sirk, 16);
        app_bt_call_func_in_bt_thread((uint32_t)aob_csip_mobile_env.cmd_result_info.sirk_opcode_cmd.sirk, 0, 0, 0, (uint32_t)app_atc_csisc_add_sirk);
        ret = true;
    }
    return ret;
}

bool aob_csip_mobile_if_remove_sirk(uint8_t *sirk)
{
    bool ret = false;
    uint8_t key_lid = 0xFF;
    uint8_t dev_index = 0;
    uint8_t sets_index = 0;
    APP_ATC_CSISC_DEV_INFO_T *p_dev_info = NULL;

    if (!sirk || aob_csip_mobile_env.cmd_result_info.sirk_opcode_cmd.inprogress_flag)
    {
        return ret;
    }

    for (dev_index = 0; dev_index < APP_ATC_CSIS_SETS_SIZE_MAX; dev_index++)
    {
        p_dev_info = &(aob_csip_mobile_env.p_csisc_env->device_info[dev_index]);
        for (sets_index = 0; sets_index < APP_ATC_CSIS_SETS_NUM_MAX; sets_index++)
        {
            if (!memcmp(p_dev_info->sets_info[sets_index].sirk.sirk, sirk, 16))
            {
                key_lid = p_dev_info->sets_info[sets_index].key_lid;
                ret = true;
                break;
            }
        }
    }

    if (ret)
    {
        aob_csip_mobile_env.cmd_result_info.sirk_opcode_cmd.inprogress_flag = true;
        memcpy(aob_csip_mobile_env.cmd_result_info.sirk_opcode_cmd.sirk, sirk, 16);
        aob_csip_mobile_env.cmd_result_info.sirk_opcode_cmd.key_lid = key_lid;
        app_bt_call_func_in_bt_thread((uint32_t)aob_csip_mobile_env.cmd_result_info.sirk_opcode_cmd.key_lid, 0, 0, 0, (uint32_t)app_atc_csisc_remove_sirk);
    }

    return ret;
}

void aob_csip_mobile_if_resolve_rsi_data(uint8_t *data, uint8_t data_len, ble_bdaddr_t *bdaddr_to_connect)
{
    if (data_len != APP_ATC_CSIS_RSI_LEN)
    {
        LOG_W("csip rsi data adv data len error!!!");
        return;
    }

    if (!aob_csip_mobile_env.start_discover_memebers)
    {
        osTimerStart(aob_csip_mobile_env.discover_members_timer, AOB_CSIP_MOBILE_DISCOVER_MEMBERS_TIMEOUT_VALUE);
        aob_csip_mobile_env.start_discover_memebers = true;
    }

    if (!aob_csip_mobile_env.cmd_result_info.rsi_resolve_cmd.inprogress_flag && bdaddr_to_connect)
    {
        memcpy(aob_csip_mobile_env.cmd_result_info.rsi_resolve_cmd.addr2connect.addr, bdaddr_to_connect->addr, 6);
        aob_csip_mobile_env.cmd_result_info.rsi_resolve_cmd.addr2connect.addr_type = bdaddr_to_connect->addr_type;
        memcpy(aob_csip_mobile_env.cmd_result_info.rsi_resolve_cmd.rsi, data, APP_ATC_CSIS_RSI_LEN);
        aob_csip_mobile_env.cmd_result_info.rsi_resolve_cmd.inprogress_flag = true;

        LOG_I("%s", __func__);
        DUMP8("%02x ", data, APP_ATC_CSIS_RSI_LEN);
        app_bt_call_func_in_bt_thread((uint32_t)aob_csip_mobile_env.cmd_result_info.rsi_resolve_cmd.rsi, 0, 0, 0, (uint32_t)app_atc_csisc_resolve);
    }
}

bool aob_csip_mobile_if_has_connected_device()
{
    bool ret = false;
    bool peer_device_connected = ble_audio_mobile_is_any_device_connected();

    if (peer_device_connected && aob_csip_mobile_env.discover_done)
    {
        ret = true;
    }

    return ret;
}

// UI layer can Initiate the discovery service by the mobile
void aob_csip_if_discovery_service(uint8_t conlid)
{
    app_atc_csisc_start(conlid);
}

// UI layer can initiate the lock by the mobile
void aob_csip_if_lock(uint8_t conlid, uint8_t set_lid, uint8_t lock)
{
    LOG_I("aob_csip_if_discovery_service:");
    app_atc_csisc_lock(conlid, set_lid, lock);
}

/*****************************************************************************************************************
*
*                    csip callback function of mobile phone sides                                                *
*
******************************************************************************************************************/
static void aob_csip_mobile_sirk_value_report_cb(uint8_t con_lid, uint8_t *sirk)
{
    bool group_add_new_dev = false;

    if (!aob_csip_mobile_env.group_info.inuse)
    {
        aob_csip_mobile_env.group_info.inuse = true;
        memcpy(aob_csip_mobile_env.group_info.sirk, sirk, 16);
        group_add_new_dev = true;
        // TODO: move add sirk to UI layer manager.
        app_atc_csisc_add_sirk(sirk);
    }
    else
    {
        if (!memcmp(aob_csip_mobile_env.group_info.sirk, sirk, 16))
        {
            group_add_new_dev = true;
        }
    }

    if (group_add_new_dev)
    {
        for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
        {
            if (!aob_csip_mobile_env.group_info.dev_info[i].inuse)
            {
                aob_csip_mobile_env.group_info.dev_info[i].inuse = true;
                aob_csip_mobile_env.group_info.dev_info[i].con_lid = con_lid;
                aob_csip_mobile_env.group_info.member_numbers++;
                break;
            }
        }
    }

    LOG_I("%s group_member_nbs:%d add_flag:%d", __func__, aob_csip_mobile_env.group_info.member_numbers, group_add_new_dev);
}

static void aob_csip_mobile_resolve_result_report_cb(uint8_t con_lid, uint8_t result)
{
    LOG_I("%s lid:%d result:%d", __func__, con_lid, result);
    AOB_CSIP_CMD_RESULT_T *p_cmd_result_info = &(aob_csip_mobile_env.cmd_result_info);

    if (result == BT_STS_SUCCESS)
    {
        LOG_I("AOB-CSIP: found the fox!");
        DUMP8("%02x ", p_cmd_result_info->rsi_resolve_cmd.addr2connect.addr, 6);
        osTimerStop(aob_csip_mobile_env.discover_members_timer);
        aob_csip_mobile_env.start_discover_memebers = false;
        app_ble_stop_scan();
        app_ble_connect_ble_audio_device(&(p_cmd_result_info->rsi_resolve_cmd.addr2connect), APP_GAPM_STATIC_ADDR, 5000);
    }

    p_cmd_result_info->rsi_resolve_cmd.result = result;
    p_cmd_result_info->rsi_resolve_cmd.inprogress_flag = false;
}

static void aob_csip_mobile_discover_server_cmp_ind_cb(uint8_t con_lid, uint8_t result)
{
    LOG_I("%s", __func__);

    if (result == BT_STS_SUCCESS)
    {
        aob_csip_mobile_env.discover_done = true;
    }
}

static void aob_csip_mobile_sirk_add_cmp_result_report_cb(uint8_t key_lid, uint8_t result)
{
    LOG_I("%s", __func__);
    if (result == BT_STS_SUCCESS)
    {
        aob_csip_mobile_env.cmd_result_info.sirk_opcode_cmd.key_lid = key_lid;

        // update csisc env key_lid elements.
        uint8_t dev_index = 0;
        uint8_t set_index = 0;
        APP_ATC_CSISC_DEV_INFO_T *p_dev_info = NULL;

        for (dev_index = 0; dev_index < APP_ATC_CSIS_SETS_SIZE_MAX; dev_index++)
        {
            p_dev_info = &(aob_csip_mobile_env.p_csisc_env->device_info[dev_index]);
            for (set_index = 0; set_index < APP_ATC_CSIS_SETS_NUM_MAX; set_index++)
            {
                if (!memcmp(p_dev_info->sets_info[set_index].sirk.sirk,
                            aob_csip_mobile_env.cmd_result_info.sirk_opcode_cmd.sirk, 16))
                {
                    p_dev_info->sets_info[set_index].key_lid = key_lid;
                }
            }
        }
    }

    aob_csip_mobile_env.cmd_result_info.sirk_opcode_cmd.result = result;
    aob_csip_mobile_env.cmd_result_info.sirk_opcode_cmd.inprogress_flag = false;
}

static void aob_csip_mobile_sirk_remove_cmp_result_report_cb(uint8_t key_lid, uint8_t result)
{
    LOG_I("%s", __func__);

    if (result == BT_STS_SUCCESS)
    {
        aob_csip_mobile_env.cmd_result_info.sirk_opcode_cmd.key_lid = key_lid;

        // remove csisc env key_lid and sirk elements.
        uint8_t dev_index = 0;
        uint8_t set_index = 0;
        APP_ATC_CSISC_DEV_INFO_T *p_dev_info = NULL;
        for (dev_index = 0; dev_index < APP_ATC_CSIS_SETS_SIZE_MAX; dev_index++)
        {
            p_dev_info = &(aob_csip_mobile_env.p_csisc_env->device_info[dev_index]);
            for (set_index = 0; set_index < APP_ATC_CSIS_SETS_NUM_MAX; set_index++)
            {
                if (!memcmp(p_dev_info->sets_info[set_index].sirk.sirk,
                            aob_csip_mobile_env.cmd_result_info.sirk_opcode_cmd.sirk, 16))
                {
                    p_dev_info->sets_info[set_index].key_lid = 0xFF;
                    memset(p_dev_info->sets_info[set_index].sirk.sirk, 0, 16);
                }
            }
        }
    }

    aob_csip_mobile_env.cmd_result_info.sirk_opcode_cmd.result = result;
    aob_csip_mobile_env.cmd_result_info.sirk_opcode_cmd.inprogress_flag = false;
}

static csip_mobile_event_handler_t csip_mobile_event_cb =
{
    .csip_sirk_value_report_cb              = aob_csip_mobile_sirk_value_report_cb,
    .csip_resolve_result_report_cb          = aob_csip_mobile_resolve_result_report_cb,
    .csip_discover_server_cmp_ind           = aob_csip_mobile_discover_server_cmp_ind_cb,
    .csip_sirk_add_result_report_cb         = aob_csip_mobile_sirk_add_cmp_result_report_cb,
    .csip_sirk_remove_result_report_cb      = aob_csip_mobile_sirk_remove_cmp_result_report_cb,
};

void aob_csip_mobile_if_init(void)
{
    aob_csip_mobile_env.discover_done = false;
    aob_csip_mobile_env.group_info.inuse = false;
    memset(aob_csip_mobile_env.group_info.sirk, 0, 16);
    aob_csip_mobile_env.group_info.member_numbers = 0;

    for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        aob_csip_mobile_env.group_info.dev_info[i].inuse = false;
        aob_csip_mobile_env.group_info.dev_info[i].con_lid = 0xFF;
        memset(aob_csip_mobile_env.group_info.dev_info[i].addr, 0, 6);
        aob_csip_mobile_env.group_info.dev_info[i].addr_type = 0;
    }

    if (aob_csip_mobile_env.discover_members_timer == NULL)
    {
        aob_csip_mobile_env.discover_members_timer = osTimerCreate(osTimer(AOB_CSIP_MOBILE_DISCOVER_MEMBERS_TIMER), osTimerOnce, NULL);
    }

    aob_csip_mobile_env.p_csisc_env = app_atc_csisc_get_env();
    aob_mgr_gaf_mobile_csip_evt_handler_register(&csip_mobile_event_cb);
}
#endif

/// @} AOB_APP
