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
#if defined(__AMA_VOICE__)
#include "ble_ai_voice.h"
#include "gatt_service.h"

#define AMA_MAX_LEN (509)

#define ama_service_uuid_16 0xFE03
#define ama_service_uuid_128_le 0xFB,0x34,0x9B,0x5F,0x80,0x00,0x00,0x80,0x00,0x10,0x00,0x00,0x03,0xFE,0x00,0x00
#define ama_rx_character_uuid_128_le 0x76,0x30,0xF8,0xDD,0x90,0xA3,0x61,0xAC,0xA7,0x43,0x05,0x30,0x77,0xB1,0x4E,0xF0
#define ama_tx_character_uuid_128_le 0x0B,0x42,0x82,0x1F,0x64,0x72,0x2F,0x8A,0xB4,0x4B,0x79,0x18,0x5B,0xA0,0xEE,0x2B

GATT_DECL_PRI_SERVICE(g_ble_ai_ama_service,
    ama_service_uuid_16);

GATT_DECL_128_LE_CHAR(g_ble_ai_ama_rx_character,
    ama_rx_character_uuid_128_le,
    GATT_WR_REQ,
    ATT_WR_ENC);

GATT_DECL_128_LE_CHAR(g_ble_ai_ama_tx_character,
    ama_tx_character_uuid_128_le,
    GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_ai_ama_tx_cccd,
    ATT_WR_ENC);

static const gatt_attribute_t g_ble_ai_ama_attr_list[] = {
    /* Service */
    gatt_attribute(g_ble_ai_ama_service),
    /* Characteristics */
    gatt_attribute(g_ble_ai_ama_rx_character),
    /* Characteristics */
    gatt_attribute(g_ble_ai_ama_tx_character),
        /* Descriptor */
        gatt_attribute(g_ble_ai_ama_tx_cccd),
};

void ble_ai_ama_send_data(app_ble_ai_data_send_param_t *param)
{
    enum gatt_evt_type evt_type = param->gatt_event_type;
    if (evt_type == GATT_NOTIFY)
    {
        gatts_send_notification(gap_conn_bf(param->conidx), g_ble_ai_ama_tx_character, param->data, (uint16_t)param->data_len);
    }
    else
    {
        gatts_send_indication(gap_conn_bf(param->conidx), g_ble_ai_ama_tx_character, param->data, (uint16_t)param->data_len);
    }
}

static bool ntf_enabled[BLE_CONNECTION_MAX] = {0};

static int ble_ai_ama_server_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
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
            ble_ai_report_data_received(APP_BLE_AI_SPEC_AMA, (svc->connhdl << 16)|svc->con_idx, BLE_AI_CMD_TYPE, p);
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
                ble_ai_report_connected(APP_BLE_AI_SPEC_AMA, svc->con_idx, svc->connhdl);
            }
            else
            {
                ble_ai_report_disconnected(APP_BLE_AI_SPEC_AMA, svc->con_idx, svc->connhdl);
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
            ble_ai_report_data_tx_done(APP_BLE_AI_SPEC_AMA, svc->con_idx, svc->connhdl, BLE_AI_ANY_TYPE);
            break;
        }
        case GATT_SERV_EVENT_MTU_CHANGED:
        {
            gatt_server_mtu_changed_t *p = param.mtu_changed;
            ble_ai_mtu_exchanged(p->con_idx, svc->connhdl, p->mtu);
            break;
        }
        case GATT_SERV_EVENT_CONN_CLOSED:
        {
            ble_ai_report_disconnected(APP_BLE_AI_SPEC_AMA, svc->con_idx, svc->connhdl);
            break;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

void ble_ai_ama_init(void)
{
    gatts_register_service(g_ble_ai_ama_attr_list, ARRAY_SIZE(g_ble_ai_ama_attr_list), ble_ai_ama_server_callback, NULL);
}

#endif /* __AMA_VOICE__ */
