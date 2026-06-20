/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifdef BLE_WALKIE_TALKIE

#include "walkie_talkie_ble_gapm_cmd.h"
#include "bluetooth.h"
#include "gatt_service.h"
#include "me_api.h"

#define BLE_WALKIE_GAP_TEST 1
#define BLE_WALKIE_GAP_ADV_MAX 3
#define WALKIE_MESH_LIST_MAX 10
#define WALKIE_RD_MESH_LIST_SIZE_CMD_OPCODE          0xFDE0
#define WALKIE_CLEAR_MESH_LIST_CMD_OPCODE          0xFDE1
#define WALKIE_ADD_DEV_TO_MESH_LIST_CMD_OPCODE          0xFDE2
#define WALKIE_RMV_DEV_FROM_MESH_LIST_CMD_OPCODE          0xFDE3

typedef struct
{
    ble_walkie_gap_adv_param param;
    uint8_t adv_handle;
    uint32 duration_ms;
    bool is_used;
}ble_walkie_gap_adv_status;

typedef struct
{
    uint8_t addr[6];
    uint8_t set_id;
}ble_walkie_all_mesh_set_id_item;

static ble_walkie_gap_adv_status walkie_adv_info[BLE_WALKIE_GAP_ADV_MAX]={0};
static ble_walkie_gap_callback *walkie_gap_cb;
static bool ble_walkie_is_syncing = false;
static bool ble_walkie_gap_inited = false;
static uint8_t ble_walkie_ble_mesh_list_num = 0;
static uint8_t ble_walkie_ble_mesh_pre_list_num = 0;
static uint8_t walkie_ble_mesh_list[WALKIE_MESH_LIST_MAX][6];
static uint8_t walkie_ble_mesh_list_mem_set_id[WALKIE_MESH_LIST_MAX];
ble_walkie_all_mesh_set_id_item walkie_all_mesh_set_id[WALKIE_MESH_LIST_MAX];

static ble_walkie_gap_adv_status *ble_walkie_get_info_by_handle(uint8_t adv_handle)
{
    for (uint8_t i = 0; i < BLE_WALKIE_GAP_ADV_MAX; i++)
    {
        if(walkie_adv_info[i].adv_handle == adv_handle && walkie_adv_info[i].is_used == true)
        {
            return &walkie_adv_info[i];
        }
    }
    return NULL;
}

static ble_walkie_gap_adv_status *ble_walkie_get_free_info()
{
    for (uint8_t i = 0; i < BLE_WALKIE_GAP_ADV_MAX; i++)
    {
        if(walkie_adv_info[i].is_used == false)
        {
            return &walkie_adv_info[i];
        }
    }
    return NULL;
}

ble_walkie_all_mesh_set_id_item *ble_walkie_gap_free_set_id_iter()
{
    for (uint8_t i = 0; i < WALKIE_MESH_LIST_MAX; i++)
    {
        if(walkie_all_mesh_set_id[i].set_id == 0xff)
        {
            return &walkie_all_mesh_set_id[i];
        }
    }
    return NULL;
}

static int ble_walkie_gap_adv_callback(uintptr_t adv, gap_adv_event_t event, gap_adv_callback_param_t param)
{
    if(event != GAP_ADV_EVENT_SET_DATA)
    {
        TRACE(0, "W-T-ST: adv cb %d %d", event, adv);
    }
    switch (event)
    {
        case GAP_ADV_EVENT_STARTED:
            {
                ble_walkie_gap_adv_status *actv = ble_walkie_get_info_by_handle(param.adv_started->adv_handle);
                ASSERT(actv, "cur wt handle not register %d", param.adv_started->adv_handle);
                if(param.adv_started->error_code == BT_STS_SUCCESS && actv->param.is_contain_pa)
                {
                    gap_set_pa_enable(param.adv_started->adv_handle, true, false);
                }
                else
                {
                    walkie_gap_cb->adv_start(param.adv_started->adv_handle, param.adv_started->error_code, 0);
                }
            }
            break;
        case GAP_ADV_EVENT_STOPPED:
            walkie_gap_cb->adv_stop(param.adv_stopped->adv_handle, param.adv_stopped->error_code, 0);
            break;
        case GAP_ADV_EVENT_SET_DATA:
            break;
        case GAP_ADV_EVENT_PA_ENABLED:
            walkie_gap_cb->adv_start(param.pa_enabled->adv_handle, 0, param.pa_enabled->error_code);
            break;
        case GAP_ADV_EVENT_PA_DISABLED:
            gap_disable_advertising(param.pa_enabled->adv_handle, false, 0);
            break;
        default:
            TRACE(0, "W-T-ST: adv cb not cur case: %d %d", event, adv);
            break;
    }
    return 0;
}

static int ble_walkie_gap_scan_callback(uintptr_t scan, gap_scan_event_t event, gap_scan_callback_param_t param)
{
    if(event != GAP_SCAN_EVENT_ADV_REPORT)
    {
        TRACE(0, "W-T-ST: scan cb %d %d", event, scan);
    }
    switch (event)
    {
        case GAP_SCAN_EVENT_STARTED:
            walkie_gap_cb->scan_evt(true, param.scan_started->error_code);
            break;
        case GAP_SCAN_EVENT_STOPPED:
            walkie_gap_cb->scan_evt(false, param.scan_stopped->error_code);
            break;
        case GAP_SCAN_EVENT_ADV_REPORT:
            {
                uint8_t i = 0;
                bool is_mesh_list_dev = false;
                for (i = 0; i < ble_walkie_ble_mesh_list_num; i++)
                {
                    if(memcmp(param.adv_report->peer_addr.address, &walkie_ble_mesh_list[i][0], 6) == 0)
                    {
                        is_mesh_list_dev = true;
                        break;
                    }
                }
                if(ble_walkie_ble_mesh_list_num == 0)
                {
                    is_mesh_list_dev = true;;
                }
                if(is_mesh_list_dev)
                {
                    walkie_gap_cb->recv_ea_data(param.adv_report->data, param.adv_report->data_length, param.adv_report->peer_addr.address, param.adv_report->rssi);
                    if(ble_walkie_ble_mesh_list_num > 0)
                    {
                        walkie_ble_mesh_list_mem_set_id[i] = param.adv_report->adv_set_id;
                    }
                }
            }
            break;
        default:
            TRACE(0, "W-T-ST: scan cb not cur case: %d %d", event, scan);
            break;
    }
    return 0;
}

static int ble_walkie_gap_pa_sync_callback(uintptr_t scan, gap_scan_event_t event, gap_scan_callback_param_t param)
{
    if(GAP_SCAN_EVENT_PA_REPORT != event)
    {
        TRACE(0, "W-T-ST: pa cb %d %d", event, scan);
    }
    switch (event)
    {
        case GAP_SCAN_EVENT_PA_SYNC_ESTABLISHED:
            ble_walkie_is_syncing = false;
            {
                uint8_t addr[6] = {0};
                memcpy(addr, param.pa_sync_estb->adv_addr.address, 6);
                if(param.pa_sync_estb->error_code == BT_STS_SUCCESS)
                {
                    walkie_gap_cb->pa_sync_est(param.pa_sync_estb->pa_sync_hdl, addr, param.pa_sync_estb->error_code);
                }
                else
                {
                    walkie_gap_cb->pa_sync_est(0xffff, addr, param.pa_sync_estb->error_code);
                }
            }
            break;
        case GAP_SCAN_EVENT_PA_SYNC_TERMINATED:
            ble_walkie_is_syncing = false;
            if(param.pa_sync_term->reason == BTIF_BEC_LOCAL_TERMINATED)
            {
                walkie_gap_cb->pa_sync_terminate(param.pa_sync_term->pa_sync_hdl, NULL);
            }
            else if(param.pa_sync_term->reason == BTIF_BEC_CONNECTION_TIMEOUT)
            {
                walkie_gap_cb->pa_sync_lost(param.pa_sync_term->pa_sync_hdl, NULL);
            }
            else
            {
                TRACE(0, "W-T-ST: pa sync ter %d", param.pa_sync_term->reason);
            }
            break;
        case GAP_SCAN_EVENT_PA_REPORT:
            {
                walkie_gap_cb->recv_pa_data(param.adv_report->sync_handle, param.per_adv_report->data, param.adv_report->data_length, param.per_adv_report->peer_addr.address, param.adv_report->rssi);
            }
            break;
        default:
            TRACE(0, "W-T-ST: pa cb not cur case: %d %d", event, scan);
            break;
    }
    return 0;
}

static void ble_walkie_gap_read_mesh_list_cmd_evt_complete(uint8_t status, uint8_t size)
{
    TRACE(0, "[%s] status=%0x size=%d", __FUNCTION__, status, size);
}

static void ble_walkie_gap_clear_mesh_list_cmd_evt_complete(uint8_t status)
{
    TRACE(0, "[%s] status=%0x", __FUNCTION__, status);
    if(!status)
    {
        ble_walkie_ble_mesh_pre_list_num = 0;
        ble_walkie_ble_mesh_list_num = 0;
        memset(&walkie_ble_mesh_list[0][0], 0, 6*WALKIE_MESH_LIST_MAX);
    }
    walkie_gap_cb->clear_mesh_list(status);
}

static void ble_walkie_gap_add_mesh_list_cmd_evt_complete(uint8_t status)
{
    TRACE(0, "[%s] status=%0x", __FUNCTION__, status);
    if(!status)
    {
        ble_walkie_ble_mesh_list_num++;
        if(ble_walkie_ble_mesh_list_num < ble_walkie_ble_mesh_pre_list_num)
        {
            btif_me_ble_add_dev_to_mesh_list(BT_ADDR_TYPE_PUBLIC, (bt_bdaddr_t *)(&walkie_ble_mesh_list[ble_walkie_ble_mesh_list_num][0]));
        }
    }
    if(status ||(!status && ble_walkie_ble_mesh_list_num == ble_walkie_ble_mesh_pre_list_num))
    {
        walkie_gap_cb->add_mesh_list(status);
    }
}

static void ble_walkie_gap_remove_mesh_list_cmd_evt_complete(uint8_t status)
{
    TRACE(0, "[%s] status=%0x", __FUNCTION__, status);
}

static void ble_walkie_gap_mesh_list_cmd_evt_complete(uint16_t opcode, uint8_t status, uint8_t size)
{
    switch (opcode)
    {
        case WALKIE_RD_MESH_LIST_SIZE_CMD_OPCODE:
            ble_walkie_gap_read_mesh_list_cmd_evt_complete(status, size);
            break;
        case WALKIE_CLEAR_MESH_LIST_CMD_OPCODE:
            ble_walkie_gap_clear_mesh_list_cmd_evt_complete(status);
            break;
        case WALKIE_ADD_DEV_TO_MESH_LIST_CMD_OPCODE:
            ble_walkie_gap_add_mesh_list_cmd_evt_complete(status);
            break;
        case WALKIE_RMV_DEV_FROM_MESH_LIST_CMD_OPCODE:
            ble_walkie_gap_remove_mesh_list_cmd_evt_complete(status);
            break;

    }
}

void ble_walkie_gap_register_callback(ble_walkie_gap_callback * cb)
{
    walkie_gap_cb = cb;
    if(walkie_gap_cb == NULL)
    {
        TRACE(0, "W-T-ST: reg cb NULL");
    }
}

int ble_walkie_gap_init(ble_walkie_gap_callback *cb)
{
    btif_me_set_mesh_list_callback(ble_walkie_gap_mesh_list_cmd_evt_complete);
    ble_walkie_gap_register_callback(cb);
    for (uint8_t i = 0; i < WALKIE_MESH_LIST_MAX; i++)
    {
        walkie_all_mesh_set_id[i].set_id = 0xff;
    }
    ASSERT(cb , "W-T-ST: cb not NULL");
    ble_walkie_gap_inited = true;
    return 0;
}

int ble_walkie_gap_deinit()
{
    btif_me_set_mesh_list_callback(NULL);
    ble_walkie_gap_register_callback(NULL);
    ble_walkie_gap_inited = false;
    return 0;
}

uint8_t ble_walkie_gap_adv_creat(ble_walkie_gap_adv_param* param)
{
    gap_adv_param_t *p_ext_adv_param = (gap_adv_param_t *)cobuf_malloc(sizeof(gap_adv_param_t));
    uint8_t adv_handle = GAP_INVALID_ADV_HANDLE;
    if(ble_walkie_gap_inited == false)
    {
        cobuf_free(p_ext_adv_param);
        TRACE(0, "W-T-ST: adv creat gap not inited");
        return GAP_INVALID_ADV_HANDLE;
    }

    ble_walkie_gap_adv_status *item = ble_walkie_get_free_info();
    ASSERT(item, "W-T-ST:adv creat no enough source");
    item->is_used = true;
    item->adv_handle = GAP_INVALID_ADV_HANDLE;
    memcpy(&item->param, param, sizeof(ble_walkie_gap_adv_param));

    memset(p_ext_adv_param, 0, sizeof(gap_adv_param_t));
    p_ext_adv_param->primary_adv_phy = param->primary_adv_phy;
    p_ext_adv_param->secondary_adv_phy = param->secondary_adv_phy;
    p_ext_adv_param->adv_timing.min_adv_slow_interval_ms = param->ea_min_interval_ms;
    p_ext_adv_param->adv_timing.max_adv_slow_interval_ms = param->ea_max_interval_ms;
    p_ext_adv_param->use_legacy_pdu = false;
    p_ext_adv_param->policy = GAP_ADV_ACCEPT_ALL_CONN_SCAN_REQS;
    p_ext_adv_param->connectable = false;
    p_ext_adv_param->scannable = false;
    p_ext_adv_param->peer_type = BT_ADDR_TYPE_PUBLIC;
    p_ext_adv_param->own_addr_type = BT_ADDR_TYPE_PUBLIC;
    p_ext_adv_param->limited_discoverable_mode = false;
    p_ext_adv_param->has_custom_adv_timing = true;
    adv_handle = gap_set_adv_parameters(p_ext_adv_param, ble_walkie_gap_adv_callback);//默认成功
    item->adv_handle = adv_handle;
    if(adv_handle == GAP_INVALID_ADV_HANDLE)
    {
        item->is_used = false;
        cobuf_free(p_ext_adv_param);
        return GAP_INVALID_ADV_HANDLE;
    }
    if(param->is_contain_pa)
    {
        int res = gap_set_pa_parameters(adv_handle, param->pa_interval_1_25ms, false, NULL);//默认成功
        if(res != BT_STS_SUCCESS)
        {
            TRACE(0, "W-T-ST adv creat: set pa error is %d", res);
            item->is_used = false;
            cobuf_free(p_ext_adv_param);
            return GAP_INVALID_ADV_HANDLE;
        }
    }
    cobuf_free(p_ext_adv_param);

    return adv_handle;
}

int ble_walkie_gap_adv_start(uint8_t adv_handle, uint32_t duartion)
{
    if(ble_walkie_gap_inited == false)
    {
        TRACE(0, "W-T-ST: adv start gap not inited");
        return GAP_INVALID_ADV_HANDLE;
    }
    return gap_enable_advertising(adv_handle, duartion, 0);//打开adv， 且成功打开才上报，有pa则打开pa，pa打开所有结果上报
}

int ble_walkie_gap_adv_stop(uint8_t adv_handle)
{
    if(ble_walkie_gap_inited == false)
    {
        TRACE(0, "W-T-ST: adv stop gap not inited");
        return GAP_INVALID_ADV_HANDLE;
    }
    ble_walkie_gap_adv_status *actv = ble_walkie_get_info_by_handle(adv_handle);
    ASSERT(actv, "cur wt handle not register %d", adv_handle);
    if(actv->param.is_contain_pa)
    {
        return gap_set_pa_enable(adv_handle, false, false);//pa关闭有上报，顺带关闭ea
    }
    else
    {
        return gap_disable_advertising(adv_handle, false, 0);//默认成功，顺带关闭pa
    }
}

int ble_talkie_gap_adv_set_data(uint8_t adv_handle, uint8_t *data, uint8_t data_len)
{
    return gap_set_adv_data(adv_handle, data, data_len);
}

int ble_walkie_gap_scan_start(bool filter_duplicates, uint32_t duration_ms)
{
    return gap_enable_scanning(filter_duplicates,  duration_ms);
}

int ble_walkie_gap_scan_creat(uint16_t interval_ms, uint16_t windows_ms, bool use_filter_list)
{
    if(ble_walkie_gap_inited == false)
    {
        TRACE(0, "W-T-ST: sacn start gap not inited");
        return GAP_INVALID_ADV_HANDLE;
    }
    gap_scan_param_t param;
    memset(&param, 0, sizeof(gap_scan_param_t));
    param.filter_duplicated = false;
    param.own_addr_type = BT_ADDR_TYPE_PUBLIC;
    param.active_scan = false;
    param.filter_policy = use_filter_list ? GAP_SCAN_FILTER_POLICY_BASIC_FILTERED : GAP_SCAN_FILTER_POLICY_BASIC_UNFILTERED;
    param.phys = 0x01;//GAP_PHY_BIT_LE_1M
    param.has_custom_scan_timing = true;
    param.scan_timing.fg_scan_interval_ms = interval_ms;
    param.scan_timing.fg_scan_window_ms = windows_ms;
    param.dont_auto_start = true;

    return gap_start_scanning(&param, ble_walkie_gap_scan_callback);//返回值判断set是否成功，open scan 全结果返回
}

int ble_walkie_gap_scan_stop()
{
    return gap_disable_scanning();//全结果返回
}

int ble_walkie_gap_pa_sync_create(const uint8_t *mac_addr)
{
    gap_pa_sync_param_t sync_param;
    uint8_t i = 0;
    sync_param.options = 0;//ingore sid
    sync_param.adv_addr_type = BT_ADDR_TYPE_PUBLIC;
    memcpy(sync_param.adv_addr.address ,mac_addr, 6);
    sync_param.skip = 0x0000;
    sync_param.sync_timeout_10ms = 100;
    sync_param.sync_cte_type = 0x00;
    if(ble_walkie_is_syncing == true)
    {
        return BT_STS_ALREADY_EXIST;
    }
    for (i = 0; i < ble_walkie_ble_mesh_list_num; i++)
    {
        if(memcmp(mac_addr, &walkie_ble_mesh_list[i][0], 6) == 0)
        {
            if(walkie_ble_mesh_list_mem_set_id[i] == 0xff)
            {
                return BT_STS_INVALID_PARM;
            }
            sync_param.adv_set_id = walkie_ble_mesh_list_mem_set_id[i];
        }
    }
    ble_walkie_is_syncing = true;
    return gap_pa_create_sync(&sync_param, ble_walkie_gap_pa_sync_callback);//全结果 返回
}

int ble_walkie_gap_pa_sync_stop(uint16_t pa_sync_hdl)
{
    if(ble_walkie_is_syncing == true)
    {
        int res = gap_pa_create_sync_cancel();//默认成功
        if(!res)
        {
            ble_walkie_is_syncing = false;
        }
        else
        {
            TRACE(0, "W-T-ST: pa sync stop err %d", res);
        }
        return res;
    }
    else
    {
        return gap_pa_terminate_sync(pa_sync_hdl);//全结果返回
    }
}

int ble_walkie_gap_pa_set_data(uint8_t adv_handle, const uint8_t *pa_data, uint8_t data_len)
{
    return gap_set_pa_data(adv_handle, GAP_ADV_DATA_COMPLETE, pa_data, data_len);
}

int ble_walkie_gap_set_white_list(uint8_t *mac_addr, uint8_t count)
{
    gap_filter_item_t wl_list[10] = {0};
    if(count > 10)
    {
        TRACE(0, "W-T-ST: add wl count exceed max");
        return 0xffff;
    }
    for (uint8_t i = 0; i < count; i++)
    {
        wl_list[i].peer_type = BT_ADDR_TYPE_PUBLIC;
        memcpy(wl_list[i].peer_addr.address, &mac_addr[i*6], 6);
    }
    return gap_filter_list_set_devices(wl_list, count);
}

int ble_walkie_gap_clear_white_list()
{
    return gap_filter_list_clear();
}

int ble_walkie_gap_set_mesh_list(uint8_t *mac_addr, uint8_t count)
{
    TRACE(0, "%s size=%d bdaddr=%d", __func__, count, mac_addr != NULL ? 1 : 0);
    if(!count || !mac_addr)
    {
        return btif_me_ble_clear_mesh_list();
    }
    ble_walkie_ble_mesh_pre_list_num = count;
    for (uint8_t i = 0; i < count; i++)
    {
        memcpy(&walkie_ble_mesh_list[i][0], &mac_addr[i*6], 6);
        DUMP8("%02x ", &walkie_ble_mesh_list[i][0], 6);
        walkie_ble_mesh_list_mem_set_id[i] = 0xff;
    }

    return btif_me_ble_add_dev_to_mesh_list(BT_ADDR_TYPE_PUBLIC, (bt_bdaddr_t*)mac_addr);
}

int ble_walkie_gap_clear_mesh_list()
{
    return btif_me_ble_clear_mesh_list();
}
#endif