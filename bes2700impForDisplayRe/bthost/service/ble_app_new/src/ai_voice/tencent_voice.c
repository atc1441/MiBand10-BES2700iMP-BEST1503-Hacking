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
#if defined(__TENCENT_VOICE__)
#include "ble_ai_voice.h"
#include "gatt_service.h"

#define TENCENT_VOICE_MAX_LEN (509)

#define tencent_service_uuid_16 0x0709
#define tencent_service_uuid_128_le 0xfb,0x34,0x9b,0x5f,0x80,0x00,0x00,0x80,0x00,0x10,0x00,0x00,0x09,0x07,0x00,0x00
#define tencent_tx_character_uuid_128_le 0xf9,0x47,0xdc,0xe3,0x8b,0xeb,0x48,0x82,0xf7,0x47,0x0c,0xe4,0x6e,0x41,0xd4,0xdf
#define tencent_rx_character_uuid_128_le 0xac,0x1f,0x85,0x1a,0xc1,0xb2,0xd3,0x94,0x24,0x4a,0xfe,0x61,0x12,0xd8,0x84,0x98

GATT_DECL_PRI_SERVICE(g_ble_ai_tencent_service,
    tencent_service_uuid_16);

GATT_DECL_128_LE_CHAR(g_ble_ai_tencent_rx_character,
    tencent_rx_character_uuid_128_le,
    GATT_WR_REQ,
    ATT_SEC_NONE);

GATT_DECL_128_LE_CHAR(g_ble_ai_tencent_tx_character,
    tencent_tx_character_uuid_128_le,
    GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_ai_tencent_tx_cccd,
    ATT_SEC_NONE);

static const gatt_attribute_t g_ble_ai_tencent_attr_list[] = {
    /* Service */
    gatt_attribute(g_ble_ai_tencent_service),
    /* Characteristics */
    gatt_attribute(g_ble_ai_tencent_rx_character),
    /* Characteristics */
    gatt_attribute(g_ble_ai_tencent_tx_character),
        /* Descriptor */
        gatt_attribute(g_ble_ai_tencent_tx_cccd),
};

void ble_ai_tencent_voice_send_data(app_ble_ai_data_send_param_t *param)
{
    enum gatt_evt_type evt_type = param->gatt_event_type;
    if (evt_type == GATT_NOTIFY)
    {
        gatts_send_notification(gap_conn_bf(param->conidx), g_ble_ai_tencent_tx_character, param->data, (uint16_t)param->data_len);
    }
    else
    {
        gatts_send_indication(gap_conn_bf(param->conidx), g_ble_ai_tencent_tx_character, param->data, (uint16_t)param->data_len);
    }
}

static bool ntf_enabled[BLE_CONNECTION_MAX] = {0};

static int ble_ai_tencent_server_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
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
            ble_ai_report_data_received(APP_BLE_AI_SPEC_TENCENT, (svc->connhdl << 16)|p->con_idx, BLE_AI_CMD_TYPE, p);
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
                ble_ai_report_connected(APP_BLE_AI_SPEC_TENCENT, svc->con_idx, svc->connhdl);
            }
            else
            {
                ble_ai_report_disconnected(APP_BLE_AI_SPEC_TENCENT, svc->con_idx, svc->connhdl);
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
            ble_ai_report_data_tx_done(APP_BLE_AI_SPEC_TENCENT, svc->con_idx, svc->connhdl, BLE_AI_ANY_TYPE);
            break;
        }
        case GATT_SERV_EVENT_MTU_CHANGED:
        {
            gatt_server_mtu_changed_t *p = param.mtu_changed;
            ble_ai_mtu_exchanged(svc->con_idx, svc->connhdl, p->mtu);
            break;
        }
        case GATT_SERV_EVENT_CONN_CLOSED:
        {
            ble_ai_report_disconnected(APP_BLE_AI_SPEC_TENCENT, svc->con_idx, svc->connhdl);
            break;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

void ble_ai_tencent_voice_init(void)
{
    gatts_register_service(g_ble_ai_tencent_attr_list, ARRAY_SIZE(g_ble_ai_tencent_attr_list), ble_ai_tencent_server_callback, NULL);
}

#endif /* __TENCENT_VOICE__ */
