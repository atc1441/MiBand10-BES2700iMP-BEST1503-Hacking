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
#if (defined(BES_OTA) || defined(BES_OTA_BASIC))&& !defined(OTA_OVER_TOTA_ENABLED)
#include "app_ble.h"
#include "gatt_service.h"

#define OTA_MAX_LEN (495)

#define ota_service_uuid_128_le 0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66
#define ota_character_uuid_128_le 0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77

GATT_DECL_128_LE_PRI_SERVICE(g_ble_ota_service,
    ota_service_uuid_128_le);

GATT_DECL_128_LE_CHAR(g_ble_ota_character,
    ota_character_uuid_128_le,
    GATT_WR_REQ|GATT_WR_CMD|GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_ota_desc_cccd,
    ATT_SEC_NONE);

static const gatt_attribute_t g_ble_ota_attr_list[] = {
    /* Service */
    gatt_attribute(g_ble_ota_service),
    /* Characteristics */
    gatt_attribute(g_ble_ota_character),
        /* Descriptor */
        gatt_attribute(g_ble_ota_desc_cccd),
};

static app_ota_event_callback ota_event_cb = NULL;

static bool ntf_enabled[BLE_CONNECTION_MAX] = {0};

static int ble_ota_server_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
{
    app_ota_event_param_t cb_param = {0};

    cb_param.connhdl = svc->connhdl;

    switch (event)
    {
        case GATT_SERV_EVENT_CHAR_WRITE:
        {
            gatt_server_char_write_t *p = param.char_write;
            if (p->value_offset != 0 || p->value_len == 0 || p->value == NULL)
            {
                return false;
            }
            if (ota_event_cb)
            {
                cb_param.event_type   = APP_OTA_RECEVICE_DATA;
                cb_param.conidx = p->con_idx;
                cb_param.param.receive_data.data_len = p->value_len;
                cb_param.param.receive_data.data     = (uint8_t *)p->value;
                ota_event_cb(&cb_param);
            }
            gatts_send_write_rsp(p->ctx, ATT_ERROR_NO_ERROR);
            return true;
        }
        case GATT_SERV_EVENT_DESC_WRITE:
        {
            gatt_server_desc_write_t *p = param.desc_write;
            uint16_t config = CO_COMBINE_UINT16_LE(p->value);
            bool notify_enabled = false;
            if (config & GATT_CCCD_SET_NOTIFICATION)
            {
                notify_enabled = true;
            }
            ntf_enabled[gap_zero_based_conidx(svc->con_idx)] = notify_enabled;
            if (ota_event_cb)
            {
                cb_param.event_type   = APP_OTA_CCC_CHANGED;
                cb_param.conidx       = p->con_idx;
                cb_param.param.ntf_en = notify_enabled;
                ota_event_cb(&cb_param);
            }
            return true;
        }
        case GATT_SERV_EVENT_DESC_READ:
        {
            gatt_server_desc_read_t *p = param.desc_read;
            uint16_t cccd_config = ntf_enabled[gap_zero_based_conidx(svc->con_idx)] ? 0x0001 : 0x0000;
            cccd_config = co_host_to_uint16_le(cccd_config);
            gatts_write_read_rsp_data(p->ctx, (uint8_t *)&cccd_config, sizeof(uint16_t));
            return true;
        }
        case GATT_SERV_EVENT_NTF_TX_DONE:
        case GATT_SERV_EVENT_INDICATE_CFM:
        {
            gatt_server_indicate_cfm_t *p = param.confirm;
            if (ota_event_cb)
            {
                cb_param.event_type   = APP_OTA_SEND_DONE;
                cb_param.conidx       = p->con_idx;
                cb_param.param.status = p->error_code;
                ota_event_cb(&cb_param);
            }
            break;
        }
        case GATT_SERV_EVENT_MTU_CHANGED:
        {
            gatt_server_mtu_changed_t *p = param.mtu_changed;
            if (ota_event_cb)
            {
                cb_param.event_type = APP_OTA_MTU_UPDATE;
                cb_param.conidx     = p->con_idx;
                cb_param.param.mtu  = p->mtu;
                ota_event_cb(&cb_param);
            }
            break;
        }
        case GATT_SERV_EVENT_CONN_CLOSED:
        {
            gatt_server_conn_closed_t *p = param.closed;
            if (ota_event_cb)
            {
                cb_param.event_type = APP_OTA_DIS_CONN;
                cb_param.conidx     = p->con_idx;
                cb_param.param.status = p->error_code;
                ota_event_cb(&cb_param);
            }
            break;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

void ble_ota_init(void)
{
    gatts_cfg_t cfg = {.preferred_mtu = OTA_MAX_LEN};
    gatts_register_service(g_ble_ota_attr_list, ARRAY_SIZE(g_ble_ota_attr_list), ble_ota_server_callback, &cfg);
}

void app_ota_event_reg(app_ota_event_callback cb)
{
    if (!ota_event_cb)
    {
        ota_event_cb = cb;
    }
}

void app_ota_event_unreg(void)
{
    ota_event_cb = NULL;
}

void app_ota_send_rx_cfm(uint8_t conidx)
{

}

bool app_ota_send_notification(uint8_t conidx, uint8_t* data, uint32_t len)
{
    bt_status_t status = BT_STS_SUCCESS;
    status = gatts_send_notification(gap_conn_bf(conidx), g_ble_ota_character, data, (uint16_t)len);
    return (status != BT_STS_FAILED);
}

bool app_ota_send_indication(uint8_t conidx, uint8_t* data, uint32_t len)
{
    bt_status_t status = BT_STS_SUCCESS;
    status = gatts_send_indication(gap_conn_bf(conidx), g_ble_ota_character, data, (uint16_t)len);
    return (status != BT_STS_FAILED);
}

#endif /* BES_OTA && !OTA_OVER_TOTA_ENABLED */
