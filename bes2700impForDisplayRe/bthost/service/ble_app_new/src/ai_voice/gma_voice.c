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
#if defined(__GMA_VOICE__)
#include "ble_ai_voice.h"
#include "gatt_service.h"

#define GMA_MAX_LEN 255

#define gma_service_uuid 0xFEB3
#define gma_rd_char_uuid 0xFED4
#define gma_wr_req_rx_char_uuid 0xFED5
#define gma_wr_cmd_rx_char_uuid 0xFED7
#define gma_ind_tx_char_uuid 0xFED6
#define dma_ntf_tx_char_uuid 0xFED8

GATT_DECL_PRI_SERVICE(g_ble_ai_gma_service,
    gma_service_uuid);

GATT_DECL_CHAR(g_ble_ai_gma_wr_req_rx_character,
    gma_wr_req_rx_char_uuid,
    GATT_WR_REQ,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_ai_gma_wr_cmd_rx_character,
    gma_wr_cmd_rx_char_uuid,
    GATT_WR_CMD,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_ai_gma_ind_tx_character,
    gma_ind_tx_char_uuid,
    GATT_IND_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_ai_gma_ind_tx_cccd,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_ai_gma_ntf_tx_character,
    dma_ntf_tx_char_uuid,
    GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_ai_gma_ntf_tx_cccd,
    ATT_SEC_NONE);

static const gatt_attribute_t g_ble_ai_gma_attr_list[] = {
    /* Service */
    gatt_attribute(g_ble_ai_gma_service),
    /* Characteristics */
    gatt_attribute(g_ble_ai_gma_wr_req_rx_character),
    /* Characteristics */
    gatt_attribute(g_ble_ai_gma_wr_cmd_rx_character),
    /* Characteristics */
    gatt_attribute(g_ble_ai_gma_ind_tx_character),
        gatt_attribute(g_ble_ai_gma_ind_tx_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_ai_gma_ntf_tx_character),
        gatt_attribute(g_ble_ai_gma_ntf_tx_cccd),
};

void ble_ai_gma_send_data(app_ble_ai_data_send_param_t *param)
{
    enum gatt_evt_type evt_type = param->gatt_event_type;
    if (evt_type == GATT_NOTIFY)
    {
        gatts_send_notification(gap_conn_bf(param->conidx), g_ble_ai_gma_ntf_tx_character, param->data, (uint16_t)param->data_len);
    }
    else
    {
        gatts_send_indication(gap_conn_bf(param->conidx), g_ble_ai_gma_ind_tx_character, param->data, (uint16_t)param->data_len);
    }
}

static bool ntf_enabled[BLE_CONNECTION_MAX] = {0};

static int ble_ai_gma_server_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
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
            ble_ai_report_data_received(APP_BLE_AI_SPEC_GMA, (svc->connhdl << 16)|p->con_idx, BLE_AI_CMD_TYPE, p);
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
            if ((uint8_t *)p->desc_attr->attr_data == g_ble_ai_gma_ntf_tx_character)
            {
                if (notify_enabled)
                {
                    ble_ai_report_connected(APP_BLE_AI_SPEC_GMA, svc->con_idx, svc->connhdl);
                }
                else
                {
                    ble_ai_report_disconnected(APP_BLE_AI_SPEC_GMA, svc->con_idx, svc->connhdl);
                }
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
            ble_ai_report_data_tx_done(APP_BLE_AI_SPEC_GMA, svc->con_idx, svc->connhdl, BLE_AI_ANY_TYPE);
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
            ble_ai_report_disconnected(APP_BLE_AI_SPEC_GMA, svc->con_idx, svc->connhdl);
            break;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

void ble_ai_gma_init(void)
{
    gatts_register_service(g_ble_ai_gma_attr_list, ARRAY_SIZE(g_ble_ai_gma_attr_list), ble_ai_gma_server_callback, NULL);
}

#endif /* __GMA_VOICE__ */
