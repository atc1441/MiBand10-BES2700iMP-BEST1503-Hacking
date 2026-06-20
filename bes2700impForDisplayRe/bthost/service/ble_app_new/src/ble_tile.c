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
#ifdef TILE_DATAPATH
#include "app_ble.h"

#define TILE_ID_LEN 8
#define TILE_MAX_LEN 255
#define TILE_SHIPPING_UUID      0xFEEC               /** Advertized by Tiles in Shipping Mode. */
#define TILE_ACTIVATED_UUID     0xFEED               /** Advertized by Tiles in Activated Mode. */

#define tile_service_uuid_16 TILE_ACTIVATED_UUID
#define tile_tx_rsp_char_uuid_128_le 0xC0,0x91,0xC4,0x8D,0xBD,0xE7,0x60,0xBA,0xDD,0xF4,0xD6,0x35,0x19,0x00,0x41,0x9D
#define tile_rx_cmd_char_uuid_128_le 0xC0,0x91,0xC4,0x8D,0xBD,0xE7,0x60,0xBA,0xDD,0xF4,0xD6,0x35,0x18,0x00,0x41,0x9D
#define tile_id_char_uuid_128_le 0xC0,0x91,0xC4,0x8D,0xBD,0xE7,0x60,0xBA,0xDD,0xF4,0xD6,0x35,0x07,0x00,0x41,0x9D

static bool ntf_enabled[BLE_CONNECTION_MAX] = {0};

GATT_DECL_PRI_SERVICE(g_ble_tile_service,
    tile_service_uuid_16);

GATT_DECL_128_LE_CHAR(g_ble_tile_id_character,
    tile_id_char_uuid_128_le,
    GATT_RD_REQ,
    ATT_SEC_NONE);

GATT_DECL_128_LE_CHAR(g_ble_tile_rx_cmd_character,
    tile_rx_cmd_char_uuid_128_le,
    GATT_WR_CMD,
    ATT_SEC_NONE);

GATT_DECL_128_LE_CHAR(g_ble_tile_tx_rsp_character,
    tile_tx_rsp_char_uuid_128_le,
    GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_tile_tx_rsp_cccd,
    ATT_SEC_NONE);

static const gatt_attribute_t g_ble_tile_attr_list[] = {
    /* Service */
    gatt_attribute(g_ble_tile_service),
    /* Characteristics */
    gatt_attribute(g_ble_tile_id_character),
    /* Characteristics */
    gatt_attribute(g_ble_tile_rx_cmd_character),
    /* Characteristics */
    gatt_attribute(g_ble_tile_tx_rsp_character),
        /* Descriptor */
        gatt_attribute(g_ble_tile_tx_rsp_cccd),
};

extern void app_tile_get_device_id(uint8_t* p_dev_id);
static app_ble_tile_event_cb user_callback = NULL;

static void app_tile_connected_evt_handler(uint8_t conidx, uint16_t interval, uint16_t latency, uint16_t timeout)
{
    app_ble_tile_event_param_t param_info = {0};

    if (user_callback)
    {
        param_info.event_type = APP_BLE_TILE_CONN_EVENT;
        param_info.conidx     = conidx;
        param_info.data.ble_conn_param.interval = interval;
        param_info.data.ble_conn_param.latency = latency;
        param_info.data.ble_conn_param.conn_sup_timeout = timeout;
        user_callback(&param_info);
    }
}

static void app_tile_disconnected_evt_handler(uint8_t conidx, uint16_t connhdl)
{
    app_ble_tile_event_param_t param_info = {0};

    if (user_callback)
    {
        param_info.event_type = APP_BLE_TILE_DISCONN_EVENT;
        param_info.conidx     = conidx;
        user_callback(&param_info);
    }
}

static void app_tile_ble_conn_parameter_updated(uint8_t conidx, uint16_t interval, uint16_t latency, uint16_t timeout)
{
    app_ble_tile_event_param_t param_info = {0};

    if (user_callback)
    {
        param_info.event_type = APP_BLE_TILE_CONN_UPDATE_EVENT;
        param_info.conidx     = conidx;
        param_info.data.ble_conn_param.interval = interval;
        param_info.data.ble_conn_param.latency  = latency;
        param_info.data.ble_conn_param.conn_sup_timeout  = timeout;
        user_callback(&param_info);
    }
}

static void app_tile_channel_connected_handler(uint8_t conidx, uint16_t connhdl, uint16_t attr_handle, uint8_t error_code)
{
    app_ble_tile_event_param_t param_info = {0};

    if (user_callback)
    {
        param_info.event_type = APP_BLE_TILE_CH_CONN_EVENT;
        param_info.conidx     = conidx;
        param_info.data.channel_conn.hdl = attr_handle;
        param_info.data.channel_conn.status = error_code;
        user_callback(&param_info);
    }
}

static void app_tile_channel_disconnected_handler(uint8_t conidx, uint16_t connhdl, uint16_t attr_handle, uint8_t error_code)
{
    app_ble_tile_event_param_t param_info = {0};

    if (user_callback)
    {
        param_info.event_type = APP_BLE_TILE_CH_DISCONN_EVENT;
        param_info.conidx     = conidx;
        param_info.data.channel_conn.hdl = attr_handle;
        param_info.data.channel_conn.status = error_code;
        user_callback(&param_info);
    }
}

static void app_tile_tx_data_sent_done_handler(uint8_t conidx, uint16_t connhdl, uint8_t error_code)
{
    app_ble_tile_event_param_t param_info = {0};

    if (user_callback)
    {
        param_info.event_type  = APP_BLE_TILE_CH_DISCONN_EVENT;
        param_info.conidx      = conidx;
        param_info.data.result = error_code;
        user_callback(&param_info);
    }
}

static void app_tile_rx_received_handler(uint8_t conidx, uint16_t connhdl, const uint8_t *data, uint16_t len)
{
    app_ble_tile_event_param_t param_info = {0};

    if (user_callback)
    {
        param_info.event_type  = APP_BLE_TILE_CH_DISCONN_EVENT;
        param_info.conidx      = conidx;
        param_info.data.receive.data_len = len;
        param_info.data.receive.data     = (uint8_t *)data;
        user_callback(&param_info);
    }
}

static int ble_tile_server_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
{
    switch (event)
    {
        case GATT_SERV_EVENT_CHAR_WRITE:
        {
            gatt_server_char_write_t *p = param.char_write;
            if (p->value_offset != 0 || p->value_len == 0 || p->value == NULL)
            {
                return false;
            }
            app_tile_rx_received_handler(svc->con_idx, svc->connhdl, p->value, p->value_len);
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
            if (notify_enabled)
            {
                app_tile_channel_connected_handler(svc->con_idx, svc->connhdl, 0, 0);
            }
            else
            {
                app_tile_channel_disconnected_handler(svc->con_idx, svc->connhdl, 0, 0);
            }
            return true;
        }
        case GATT_SERV_EVENT_CHAR_READ:
        {
            gatt_server_char_read_t *p = param.char_read;
            uint16_t cccd_config = ntf_enabled[gap_zero_based_conidx(svc->con_idx)] ? 0x0001 : 0x0000;
            uint8_t tile_id[TILE_ID_LEN] = {0};
            cccd_config = co_host_to_uint16_le(cccd_config);
            app_tile_get_device_id(tile_id);
            gatts_write_read_rsp_data(p->ctx, (uint8_t *)&cccd_config, sizeof(uint16_t));
            gatts_write_read_rsp_data(p->ctx, tile_id, TILE_ID_LEN);
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
            app_tile_tx_data_sent_done_handler(p->con_idx, svc->connhdl, p->error_code);
            break;
        }
        case GATT_SERV_EVENT_CONN_OPENED:
        {
            gatt_server_conn_opened_t *p = param.opened;
            app_tile_connected_evt_handler(p->con_idx, p->conn->timing.conn_interval_1_25ms,
                p->conn->timing.peripheral_latency, p->conn->timing.superv_timeout_ms);
            break;
        }
        case GATT_SERV_EVENT_CONN_CLOSED:
        {
            app_tile_disconnected_evt_handler(svc->con_idx, svc->connhdl);
            break;
        }
        case GATT_SERV_EVENT_CONN_UPDATED:
        {
            gatt_server_conn_updated_t *p = param.conn_updated;
            app_tile_ble_conn_parameter_updated(p->con_idx, p->conn->timing.conn_interval_1_25ms,
                p->conn->timing.peripheral_latency, p->conn->timing.superv_timeout_ms);
            break;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

void ble_tile_init(void)
{
    gatts_register_service(g_ble_tile_attr_list, ARRAY_SIZE(g_ble_tile_attr_list), ble_tile_server_callback, NULL);
}

void app_tile_send_ntf(uint8_t conidx, uint8_t* data, uint32_t len)
{
    gatts_send_notification(gap_conn_bf(conidx), g_ble_tile_tx_rsp_character, data, (uint16_t)len);
}

void app_tile_event_cb_reg(app_ble_tile_event_cb cb)
{
    if (!user_callback)
    {
        user_callback = cb;
    }
}

#endif /* TILE_DATAPATH */