/***************************************************************************
* @file ble_mesh.c
* @author BES software team
* @version 0.1
* @date 2024-07-05
* @copyright Copyright 2015-2024 BES.
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
#ifdef BLE_MESH_ENABLE
#include "app_custom.h"
#include "bt_mesh_ble_export.h"
#include "app_ble.h"

static BLE_ADV_ACTIVITY_USER_E mesh_adv_act_usr = BLE_ADV_ACTIVITY_USER_0;
static BLE_ADV_USER_E mesh_adv_user = USER_BLE_CUSTOMER_0;

void app_mesh_set_adv_param(mesh_ext_adv_param_t *param, uint8_t* local_addr)
{
    CUSTOMER_ADV_PARAM_T *adv_param_p = app_ble_custom_adv_param_ptr(mesh_adv_act_usr);
    bool directed = false, scannable = false, connectable = false, high_dc=false;
    directed = !!((param->adv_evt_prorp & BT_MESH_LE_ADV_PROP_DIRECT) | \
                    (param->adv_evt_prorp &BT_MESH_LE_ADV_PROP_HI_DC_CONN));
    scannable = !!(param->adv_evt_prorp & BT_MESH_LE_ADV_PROP_SCAN);
    connectable = !!(param->adv_evt_prorp & BT_MESH_LE_ADV_PROP_CONN);
    high_dc = !!(param->adv_evt_prorp & BT_MESH_LE_ADV_PROP_HI_DC_CONN);

    adv_param_p->adv_actv_user = mesh_adv_act_usr;
    adv_param_p->adv_user = mesh_adv_user;
    adv_param_p->filter_pol = param->filter_policy;
    adv_param_p->tx_power_dbm = param->tx_pwr;
    if(!directed && scannable && connectable)
    {
        adv_param_p->advType = ADV_TYPE_UNDIRECT;
    }
    else if(directed && high_dc)
    {
        adv_param_p->advType = ADV_TYPE_DIRECT_HDC;
    }
    else if(directed)
    {
        adv_param_p->advType = ADV_TYPE_DIRECT_LDC;
    }
    else if(!connectable && scannable)
    {
        adv_param_p->advType = ADV_TYPE_NON_CONN_SCAN;
    }
    else if(!connectable && !scannable)
    {
        adv_param_p->advType = ADV_TYPE_NON_CONN_NON_SCAN;
    }
    else if(!directed && connectable && scannable)
    {
        adv_param_p->advType = ADV_TYPE_CONN_EXT_ADV;
    }
    else if(directed && connectable)
    {
        adv_param_p->advType = ADV_TYPE_EXT_CON_DIRECT;
    }
    else
    {
        TRACE(1, "unknow adv type!!");
    }

    if(!!(param->adv_evt_prorp & BT_MESH_LE_ADV_PROP_LEGACY))
    {
        adv_param_p->advMode = ADV_MODE_LEGACY;
    }
    else
    {
        adv_param_p->advMode = ADV_MODE_EXTENDED;
    }

    adv_param_p->advUserInterval = BLE_ADVERTISING_INTERVAL;
    adv_param_p->PeriodicIntervalMax = param->interval_max;
    adv_param_p->PeriodicIntervalMin = param->interval_min;
    adv_param_p->localAddrType = param->own_addr_type;
    adv_param_p->peerAddr.addr_type = param->peer_addr_type;
    memcpy(adv_param_p->peerAddr.addr, param->peer_addr, 6);

    switch (adv_param_p->localAddrType)
    {
        case BLE_ADV_PUBLIC_STATIC:
        {
             adv_param_p->localAddrType = GAPM_STATIC_ADDR;
             adv_param_p->localAddr = *gap_factory_le_address();
            break;
        }
        case BLE_ADV_PRIVATE_STATIC:
        {
             adv_param_p->localAddrType = GAPM_GEN_RSLV_ADDR;
            if (local_addr)
            {
                 adv_param_p->localAddr = *((bt_bdaddr_t *)local_addr);
            }
            break;
        }
        case BLE_ADV_RPA:
        {
#ifdef BLE_ADV_RPA_ENABLED
             adv_param_p->localAddrType = GAPM_GEN_RSLV_ADDR;
            if (local_addr)
            {
                 adv_param_p->localAddr = *((bt_bdaddr_t *)local_addr);
            }
#else
             adv_param_p->localAddrType = GAPM_STATIC_ADDR;
             adv_param_p->localAddr = *gap_factory_le_address();
#endif
            break;
        }
        default:
        {
            ASSERT(0, "UNKNOWN ADDR TYPE %d", adv_param_p->localAddrType);
            break;
        }
    }
}

void app_mesh_set_adv_data(mesh_ext_adv_data_t *data)
{

    CUSTOMER_ADV_PARAM_T *adv_param_p = app_ble_custom_adv_param_ptr(mesh_adv_act_usr);

    adv_param_p->withFlags = data->withFlag;
    adv_param_p->withName = data->withName;

    if(data->data_len)
    {
        gap_dt_buf_clear(&adv_param_p->adv_data);
        gap_dt_add_raw_data(&adv_param_p->adv_data, data->data, data->data_len);
    }
}

void app_mesh_set_scan_res_data(mesh_ext_adv_data_t *data)
{
    CUSTOMER_ADV_PARAM_T *adv_param_p = app_ble_custom_adv_param_ptr(mesh_adv_act_usr);
    if(data->data_len)
    {
        gap_dt_add_raw_data(&adv_param_p->scan_rsp_data, data->data, data->data_len);
    }
}

static uint8_t app_ble_get_adv_hdl_by_user(BLE_ADV_USER_E user)
{
    uint8_t advHdl;
    switch (user)
    {
        case USER_GFPS:
        advHdl = BLE_GFPS_ADV_HANDLE;
        break;

        case USER_SWIFT:
        advHdl = BLE_SWIFT_ADV_HANDLE;
        break;

        case USER_BLE_AUDIO:
        advHdl = BLE_AUDIO_ADV_HANDLE;
        break;

        case USER_AI:
        case USER_INTERCONNECTION:
        case USER_TILE:
        advHdl = BLE_BASIC_ADV_HANDLE;
        break;

        case USER_BLE_CUSTOMER_0:
        advHdl = BLE_CUSTOMER0_ADV_HANDLE;
        break;
        case USER_BLE_CUSTOMER_1:
        advHdl = BLE_CUSTOMER1_ADV_HANDLE;
        break;
        case USER_BLE_CUSTOMER_2:
        advHdl = BLE_CUSTOMER2_ADV_HANDLE;
        break;
        case USER_BLE_CUSTOMER_3:
        advHdl = BLE_CUSTOMER3_ADV_HANDLE;
        break;

        default:
        advHdl = BLE_BASIC_ADV_HANDLE;
        break;
    }
    return advHdl;
}

uint8_t app_mesh_start_ext_adv(mesh_ext_adv_enable_t *param)
{
    CUSTOMER_ADV_PARAM_T *adv_param_p = app_ble_custom_adv_param_ptr(mesh_adv_act_usr);
    adv_param_p->duration = param->durations;
    adv_param_p->max_adv_evts = param->max_evts;
    app_ble_custom_adv_start(mesh_adv_act_usr);

    return app_ble_get_adv_hdl_by_user(mesh_adv_user);
}

void app_mesh_stop_ext_adv()
{
    app_ble_custom_adv_stop(mesh_adv_act_usr);
}

void app_mesh_ext_adv_init()
{
    mesh_ext_adv_cb_t cbs = {
        .set_ext_adv_param = app_mesh_set_adv_param,
        .set_ext_adv_data = app_mesh_set_adv_data,
        .set_scan_res_data = app_mesh_set_scan_res_data,
        .ext_adv_start = app_mesh_start_ext_adv,
        .ext_adv_stop = app_mesh_stop_ext_adv
    };
    bt_mesh_ext_adv_cb_init(&cbs);
    app_ble_custom_init();
}

//scan
static scan_cb_t mesh_scan_cb = NULL;

uint8_t get_adv_type(const gap_adv_flags_t *flag)
{
    if(flag->connectable && flag->scannable)
    {
        return BT_MESH_ADV_TYPE_ADV_IND;
    }
    if(flag->connectable && flag->directed)
    {
        return BT_MESH_ADV_TYPE_ADV_DIRECT_IND;
    }
    if(flag->scannable && flag->extended_adv)
    {
        return BT_MESH_ADV_TYPE_ADV_SCAN_IND;
    }
    if(flag->scan_rsp && flag->scannable)
    {
        return BT_MESH_ADV_TYPE_SCAN_RSP;
    }
    if(!flag->connectable && !flag->scannable)
    {
        return BT_MESH_ADV_TYPE_ADV_NONCONN_IND;
    }

    return BT_MESH_ADV_TYPE_EXT_ADV;
}

int app_mesh_scan_callback(uintptr_t connhdl, gap_init_event_t event, gap_init_callback_param_t param)
{
    if (event < GAP_INIT_EVENT_CONN_OPENED)
    {
        ble_event_t cb_event;

        memset(&cb_event, 0, sizeof(ble_event_t));
        if (event == GAP_INIT_EVENT_SCAN_STARTED)
        {
        }
        else if (event == GAP_INIT_EVENT_SCAN_STOPPED)
        {
        }
        else if (event == GAP_INIT_EVENT_SCAN_ADV_REPORT)
        {
            const gap_adv_report_t *adv_report = param.adv_report;
            uint8_t adv_type = get_adv_type(&adv_report->adv);
            if(mesh_scan_cb)
            {
                bt_ad_le_t addr;
                addr.type = (uint8_t)adv_report->peer_type;
                memcpy(addr.a.val, adv_report->peer_addr.address, 6);
                mesh_scan_cb(&addr, adv_report->rssi, adv_type, adv_report->data, adv_report->data_length);
            }
        }
    }

    return 0;
}
void app_register_scan_cb(scan_cb_t cb)
{
    mesh_scan_cb = cb;
}

void app_mesh_start_scan(mesh_scan_param_t *param)
{
    if(param)
    {
        BLE_SCAN_PARAM_T p = {
            .ownAddrType = param->addr_type,
            .scanType = param->scan_type,
            .scanFolicyType = param->scan_filter_policy,
            .scanWindowMs = param->window,
            .scanIntervalMs = param->interval,
            .scanDurationMs = param->timeout,
            .phys = param->phys,
            .scanType_coded = param->scan_type_coded,
            .scanIntervalMs_coded =param->interval_coded,
            .scanWindowMs_coded = param->window_coded,
            .filter_duplicates = param->filter_dup,
            .legacy = param->legacy,
            .period = param->period,
            .scan_callback = (gap_scan_callback_t)app_mesh_scan_callback,
        };
        app_ble_start_scan(&p);
    }
    else
    {
        TRACE(1, "scan start failed, scan param is NULL");
    }
}

void app_mesh_stop_scan(void)
{
    app_ble_stop_scan();
}

void app_mesh_scan_init()
{
    mesh_scan_cb_t cbs = {
        .scan_start_func = app_mesh_start_scan,
        .scan_stop_func = app_mesh_stop_scan,
        .set_scan_cb = app_register_scan_cb,
    };
    bt_mesh_scan_cb_init(&cbs);
}


//mesh
void app_ble_mesh_init()
{
    app_mesh_ext_adv_init();
    app_mesh_scan_init();
}
#endif //BLE_MESH_ENABLE