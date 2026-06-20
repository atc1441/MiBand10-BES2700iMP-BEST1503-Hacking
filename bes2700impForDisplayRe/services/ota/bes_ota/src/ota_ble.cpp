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
 *
 *    Leonardo        2018/12/20
 ****************************************************************************/
#include "ota_ble.h"
#include "ota_basic.h"
#include "hal_trace.h"
#include "cmsis_os.h"
#include "hal_timer.h"
#include "app_utils.h"
#include "bluetooth_ble_api.h"
#include "cqueue.h"
#include "ota_dbg.h"

#if defined(__IAG_BLE_INCLUDE__)&&!defined(OTA_OVER_TOTA_ENABLED)
struct ota_ble_env_info
{
    uint8_t  connectionIndex;
    uint8_t  isNotificationEnabled;
};

struct ota_ble_env_info ota_ble_env;

static void ota_ble_conn_hdl(uint8_t conidx)
{
    ota_ble_env.connectionIndex = conidx;
    bes_ble_gap_update_conn_param_mode(BLE_CONN_PARAM_MODE_OTA, true);
}

static void ota_ble_disconn_hdl(void)
{
    ota_ble_env.connectionIndex = INVALID_CONNECTION_INDEX;
    ota_ble_env.isNotificationEnabled = false;
    bes_ble_gap_update_conn_param_mode(BLE_CONN_PARAM_MODE_OTA, false);
}

static BES_OTA_PATH_TYPE_E ota_ble_get_path_type()
{
#if defined(__GATT_OVER_BR_EDR__)
    if (app_btgatt_is_connected_by_conidx(app_ota_get_conidx())) {
        return DATA_PATH_BTGATT;
    } else
#endif
    {
        return DATA_PATH_BLE;
    }
}

static void ota_ble_event_callback(bes_ble_ota_event_param_t *param)
{
    bes_ota_event_param_t otaParam = {.conidx = INVALID_CONNECTION_INDEX};
    otaParam.pathType = ota_ble_get_path_type();
    otaParam.conidx = param->conidx;

    switch (param->event_type){
    case BES_BLE_OTA_CCC_CHANGED:{
        TRACE(1,"ota data ccc changed to %d", param->param.ntf_en);
        ota_ble_env.isNotificationEnabled = (param->param.ntf_en > 0);

        if (ota_ble_env.isNotificationEnabled)
        {
            otaParam.event = BES_OTA_CONN;
            ota_ble_conn_hdl(param->conidx);
        }else{
            if (param->conidx == ota_ble_env.connectionIndex)
            {
                otaParam.event = BES_OTA_DISCONN;
                ota_ble_disconn_hdl();
            }
        }
        app_ota_push_rx_data(BLE_RX_DATA_SELF_OTA, &otaParam);
        break;        
    }

    case BES_BLE_OTA_DISCONN:{
        if (param->conidx == ota_ble_env.connectionIndex &&
            (ota_ble_get_path_type() == app_ota_get_connected_type()))
        {
            otaParam.event = BES_OTA_DISCONN;
            ota_ble_disconn_hdl();
            app_ota_push_rx_data(BLE_RX_DATA_SELF_OTA, &otaParam);
        }
        break;        
    }

    case BES_BLE_OTA_RECEVICE_DATA:{
        otaParam.event = BES_OTA_RECEVICE_DATA;
        otaParam.param.receive_data.dataLen = param->param.receive_data.data_len;
        otaParam.param.receive_data.data = (uint8_t *)param->param.receive_data.data;
        app_ota_push_rx_data(BLE_RX_DATA_SELF_OTA, &otaParam);
        break;
    }
    case BES_BLE_OTA_MTU_UPDATE:{
        otaParam.param.mtu = param->param.mtu;
        otaParam.event = BES_OTA_MTU_UPDATE;
        app_ota_push_rx_data(BLE_RX_DATA_SELF_OTA, &otaParam);
        break;
    }

    case BES_BLE_OTA_SEND_DONE:
        break;
    default:
        break;
    }
}


void ota_ble_send_notification(uint8_t *ptrData, uint16_t length)
{
    bes_ble_ota_send_notification(app_ota_get_conidx(), ptrData, length);
}

void ota_ble_init(void)
{
    bes_ble_ota_event_reg(ota_ble_event_callback);
}

#endif