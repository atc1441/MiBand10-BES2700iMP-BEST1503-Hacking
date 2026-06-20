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
#ifdef BLE_WIRELESS_TRANS_SRV_ENABLED
#include "gatt_service.h"
#include "ble_wireless_trans_srv.h"
#include "app_ble.h"

#define BLE_WIRELESS_TRANS_SRV_PREFERRED_MTU      512
#define BLE_WIRELESS_TRANS_SRV_CONN_INTERVAL      (15) //ms

#define ble_wireless_trans_service_uuid_16                    0xFFF0
#define ble_wireless_trans_rx_attr0_character_uuid_16         0xFFF3 //uart rx (write)
#define ble_wireless_trans_tx_attr1_character_uuid_16         0xFFF7 //data sm TX (notify)
#define ble_wireless_trans_tx_attr2_character_uuid_16         0xFFF4 //uart tx (notify)
#define ble_wireless_trans_txrx_attr_character_uuid_16        0xFFF5 // not use

GATT_DECL_PRI_SERVICE(g_ble_wireless_trans_service,
    ble_wireless_trans_service_uuid_16);

GATT_DECL_CHAR(g_ble_wireless_trans_rx_attr0_character,
    ble_wireless_trans_rx_attr0_character_uuid_16,
    GATT_RD_REQ|GATT_WR_REQ|GATT_NTF_PROP|GATT_IND_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_wireless_trans_rx_attr0_cccd,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_wireless_trans_tx_attr1_character,
    ble_wireless_trans_tx_attr1_character_uuid_16,
    GATT_RD_REQ|GATT_WR_REQ|GATT_NTF_PROP|GATT_IND_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_wireless_trans_tx_attr1_cccd,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_wireless_trans_tx_attr2_character,
    ble_wireless_trans_tx_attr2_character_uuid_16,
    GATT_RD_REQ|GATT_WR_REQ|GATT_NTF_PROP|GATT_IND_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_wireless_trans_tx_attr2_cccd,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_wireless_trans_txrx_attr_character,
    ble_wireless_trans_txrx_attr_character_uuid_16,
    GATT_RD_REQ|GATT_WR_CMD|GATT_NTF_PROP|GATT_IND_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_wireless_trans_txrx_attr_cccd,
    ATT_SEC_NONE);

static const gatt_attribute_t g_ble_wireless_trans_attr_list[] = {
    /* Service */
    gatt_attribute(g_ble_wireless_trans_service),
    /* Characteristics */
    gatt_attribute(g_ble_wireless_trans_rx_attr0_character),
        gatt_attribute(g_ble_wireless_trans_rx_attr0_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_wireless_trans_tx_attr1_character),
        gatt_attribute(g_ble_wireless_trans_tx_attr1_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_wireless_trans_tx_attr2_character),
        gatt_attribute(g_ble_wireless_trans_tx_attr2_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_wireless_trans_txrx_attr_character),
        gatt_attribute(g_ble_wireless_trans_txrx_attr_cccd),
};

static struct ble_wireless_trans_server_env_tag ble_wireless_trans_server_env = {0};
static ble_wireless_trans_server_cb_t *ble_wireless_trans_server_cb = NULL;

uint8_t ble_wireless_trans_srv_send_attr0_data_via_notification(uint8_t* data, uint32_t len)
{
    uint8_t conidx = ble_wireless_trans_server_env.connectionIndex;
    if (BLE_INVALID_CONNECTION_INDEX == conidx || \
            !(ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_RECV_ATTR0]>>8))
    {
        return BT_STS_FAILED;
    }

    return gatts_send_notification(gap_conn_bf(gap_zero_based_conidx_to_ble_conidx(conidx)), \
                                                    g_ble_wireless_trans_rx_attr0_character, data, (uint16_t)len);
}

uint8_t ble_wireless_trans_srv_send_attr0_data_via_indication(uint8_t* data, uint32_t len)
{
    uint8_t conidx = ble_wireless_trans_server_env.connectionIndex;
    if (BLE_INVALID_CONNECTION_INDEX == conidx || \
            !(ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_RECV_ATTR0]&0xff))
    {
        return BT_STS_FAILED;
    }

    return gatts_send_indication(gap_conn_bf(gap_zero_based_conidx_to_ble_conidx(conidx)), \
                                                    g_ble_wireless_trans_rx_attr0_character, data, (uint16_t)len);
}

uint8_t ble_wireless_trans_srv_send_attr1_data_via_notification(uint8_t* data, uint32_t len)
{
    uint8_t conidx = ble_wireless_trans_server_env.connectionIndex;
    if (BLE_INVALID_CONNECTION_INDEX == conidx || \
            !(ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_SEND_ATTR1]>>8))
    {
        return BT_STS_FAILED;
    }

    return gatts_send_notification(gap_conn_bf(gap_zero_based_conidx_to_ble_conidx(conidx)), \
                                                    g_ble_wireless_trans_tx_attr1_character, data, (uint16_t)len);
}

uint8_t ble_wireless_trans_srv_send_attr1_data_via_indication(uint8_t* data, uint32_t len)
{
    uint8_t conidx = ble_wireless_trans_server_env.connectionIndex;
    if (BLE_INVALID_CONNECTION_INDEX == conidx || \
            !(ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_SEND_ATTR1]&0xff))
    {
        return BT_STS_FAILED;
    }

    return gatts_send_indication(gap_conn_bf(gap_zero_based_conidx_to_ble_conidx(conidx)), \
                                                    g_ble_wireless_trans_tx_attr1_character, data, (uint16_t)len);
}

uint8_t ble_wireless_trans_srv_send_attr2_data_via_notification(uint8_t* data, uint32_t len)
{
    uint8_t conidx = ble_wireless_trans_server_env.connectionIndex;
    if (BLE_INVALID_CONNECTION_INDEX == conidx || \
            !(ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_SEND_ATTR2]>>8))
    {
        return BT_STS_FAILED;
    }

    return gatts_send_notification(gap_conn_bf(gap_zero_based_conidx_to_ble_conidx(conidx)), \
                                                    g_ble_wireless_trans_tx_attr2_character, data, (uint16_t)len);
}

uint8_t ble_wireless_trans_srv_send_attr2_data_via_indication(uint8_t* data, uint32_t len)
{
    uint8_t conidx = ble_wireless_trans_server_env.connectionIndex;
    if (BLE_INVALID_CONNECTION_INDEX == conidx || \
            !(ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_SEND_ATTR2]&0xff))
    {
        return BT_STS_FAILED;
    }

    return gatts_send_indication(gap_conn_bf(gap_zero_based_conidx_to_ble_conidx(conidx)), \
                                                    g_ble_wireless_trans_tx_attr2_character, data, (uint16_t)len);
}

uint8_t ble_wireless_trans_srv_send_attr_data_via_notification(uint8_t* data, uint32_t len)
{
    uint8_t conidx = ble_wireless_trans_server_env.connectionIndex;
    if (BLE_INVALID_CONNECTION_INDEX == conidx || \
            !(ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_RECV_SEND_ATTR]>>8))
    {
        return BT_STS_FAILED;
    }

    return gatts_send_notification(gap_conn_bf(gap_zero_based_conidx_to_ble_conidx(conidx)), \
                                                    g_ble_wireless_trans_txrx_attr_character, data, (uint16_t)len);
}

uint8_t ble_wireless_trans_srv_send_attr_data_via_indication(uint8_t* data, uint32_t len)
{
    uint8_t conidx = ble_wireless_trans_server_env.connectionIndex;
    if (BLE_INVALID_CONNECTION_INDEX == conidx || \
            !(ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_RECV_SEND_ATTR]&0xff))
    {
        return BT_STS_FAILED;
    }

    return gatts_send_indication(gap_conn_bf(gap_zero_based_conidx_to_ble_conidx(conidx)), \
                                                    g_ble_wireless_trans_txrx_attr_character, data, (uint16_t)len);
}

static void ble_wireless_trans_srv_mtu_exchanged(uint8_t conidx, uint16_t connhdl, uint16_t mtu)
{
    if (ble_wireless_trans_server_cb && ble_wireless_trans_server_cb->mtu_exchanged_done_cb)
    {
        ble_wireless_trans_server_cb->mtu_exchanged_done_cb(conidx, mtu);
    }
}

static void ble_wireless_trans_srv_connected(uint8_t conidx, bool notify_enabled, \
                                                bool indicate_enabled, const uint8_t *descriptor)
{
    uint8_t char_index = SRV_CHAR_MAX_NUM;
    uint16_t value = 0;

    ble_wireless_trans_server_env.connectionIndex = conidx;
    if (g_ble_wireless_trans_rx_attr0_cccd == descriptor)
    {
        if (notify_enabled) {
            ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_RECV_ATTR0] |= notify_enabled<<8;
        } else if (indicate_enabled) {
            ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_RECV_ATTR0] |= indicate_enabled;
        }
        value = ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_RECV_ATTR0];
        char_index = SRV_CHAR_RECV_ATTR0;
    }
    else if (g_ble_wireless_trans_tx_attr1_cccd == descriptor)
    {
        if (notify_enabled) {
            ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_SEND_ATTR1] |= notify_enabled<<8;
        } else if (indicate_enabled) {
            ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_SEND_ATTR1] |= indicate_enabled;
        }
        value = ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_SEND_ATTR1];
        char_index = SRV_CHAR_SEND_ATTR1;
    }
    else if(g_ble_wireless_trans_tx_attr2_cccd == descriptor)
    {
        if (notify_enabled) {
            ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_SEND_ATTR2] |= notify_enabled<<8;
        } else if (indicate_enabled) {
            ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_SEND_ATTR2] |= indicate_enabled;
        }
        value = ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_SEND_ATTR2];
        char_index = SRV_CHAR_SEND_ATTR2;
    }
    else if(g_ble_wireless_trans_txrx_attr_cccd == descriptor)
    {
        if (notify_enabled) {
            ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_RECV_SEND_ATTR] |= notify_enabled<<8;
        } else if (indicate_enabled) {
            ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_RECV_SEND_ATTR] |= indicate_enabled;
        }
        value = ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_RECV_SEND_ATTR];
        char_index = SRV_CHAR_RECV_SEND_ATTR;
    }

    if (ble_wireless_trans_server_cb && ble_wireless_trans_server_cb->connected_done_cb)
    {
        ble_wireless_trans_server_cb->connected_done_cb(conidx, char_index, value);
    }
}

static void app_ble_wireless_trans_server_disconnected(uint8_t conidx, const uint8_t *descriptor)
{
    uint8_t char_index = SRV_CHAR_MAX_NUM;

    if (NULL == descriptor)
    {
        memset(&ble_wireless_trans_server_env, 0, sizeof(struct ble_wireless_trans_server_env_tag));
        ble_wireless_trans_server_env.connectionIndex = BLE_INVALID_CONNECTION_INDEX;
    }
    else if(g_ble_wireless_trans_rx_attr0_cccd == descriptor)
    {
        char_index = SRV_CHAR_RECV_ATTR0;
        ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_RECV_ATTR0] = 0;
    }
    else if(g_ble_wireless_trans_tx_attr1_cccd == descriptor)
    {
        char_index = SRV_CHAR_SEND_ATTR1;
        ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_SEND_ATTR1] = 0;
    }
    else if (g_ble_wireless_trans_tx_attr2_cccd == descriptor)
    {
        char_index = SRV_CHAR_SEND_ATTR2;
        ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_SEND_ATTR2] = 0;
    }
    else if(g_ble_wireless_trans_txrx_attr_cccd == descriptor)
    {
        char_index = SRV_CHAR_RECV_SEND_ATTR;
        ble_wireless_trans_server_env.NtfIndCfgAttr[SRV_CHAR_RECV_SEND_ATTR] = 0;
    }

    if (ble_wireless_trans_server_cb && ble_wireless_trans_server_cb->disconnected_done_cb)
    {
        ble_wireless_trans_server_cb->disconnected_done_cb(conidx, char_index);
    }
}

static void app_ble_wireless_trans_server_tx_ccc_changed(uint8_t conidx, uint16_t config, const uint8_t *descriptor)
{
    if (config & GATT_CCCD_SET_NOTIFICATION)
    {
        ble_wireless_trans_srv_connected(conidx, true, false, descriptor);
    }
    else if (config & GATT_CCCD_SET_INDICATION)
    {
        ble_wireless_trans_srv_connected(conidx, false, true, descriptor);
    }
    else
    {
        app_ble_wireless_trans_server_disconnected(conidx, descriptor);
    }
}

static void ble_wireless_trans_srv_tx_data_sent(uint8_t conidx, const uint8_t *character)
{
    uint8_t char_index = SRV_CHAR_MAX_NUM;
    if (character == g_ble_wireless_trans_rx_attr0_character)
    {
        char_index = SRV_CHAR_RECV_ATTR0;
    }
    else if (character == g_ble_wireless_trans_tx_attr1_character)
    {
        char_index = SRV_CHAR_SEND_ATTR1;
    }
    else if (character == g_ble_wireless_trans_tx_attr2_character)
    {
        char_index = SRV_CHAR_SEND_ATTR2;
    }
    else if (character == g_ble_wireless_trans_txrx_attr_character)
    {
        char_index = SRV_CHAR_RECV_SEND_ATTR;
    }

    if (ble_wireless_trans_server_cb && ble_wireless_trans_server_cb->tx_done_cb && char_index < SRV_CHAR_MAX_NUM)
    {
        ble_wireless_trans_server_cb->tx_done_cb(conidx, char_index);
    }
}

static att_error_code_t ble_wireless_trans_srv_rx_data_received(uint8_t conidx, const uint8_t *data, \
                                                                uint16_t len, const uint8_t *character)
{
    att_error_code_t status = ATT_ERROR_NO_ERROR;
    uint8_t char_index = SRV_CHAR_MAX_NUM;

    if (character == g_ble_wireless_trans_rx_attr0_character)
    {
        char_index = SRV_CHAR_RECV_ATTR0;
    }
    else if (character == g_ble_wireless_trans_tx_attr1_character)
    {
        char_index = SRV_CHAR_SEND_ATTR1;
    }
    else if (character == g_ble_wireless_trans_tx_attr2_character)
    {
        char_index = SRV_CHAR_SEND_ATTR2;
    }
    else if(character == g_ble_wireless_trans_txrx_attr_character)
    {
        char_index = SRV_CHAR_RECV_SEND_ATTR;
    }
    else
    {
        status = ATT_ERROR_REQ_NOT_SUPPORT;
    }

    if (ble_wireless_trans_server_cb && ble_wireless_trans_server_cb->data_received_cb && char_index < SRV_CHAR_MAX_NUM)
    {
        ble_wireless_trans_server_cb->data_received_cb(conidx, char_index, (uint8_t *)data, len);
    }

    return status;
}

static int ble_wireless_trans_srv_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
{
    switch (event)
    {
        case GATT_SERV_EVENT_CONN_OPENED:
            break;
        case GATT_SERV_EVENT_CONN_CLOSED:
        {
            app_ble_wireless_trans_server_disconnected(gap_zero_based_conidx(svc->con_idx), NULL);
            break;
        }
        case GATT_SERV_EVENT_CONN_UPDATED:
            break;
        case GATT_SERV_EVENT_MTU_CHANGED:
        {
            gatt_server_mtu_changed_t *p = param.mtu_changed;
            ble_wireless_trans_srv_mtu_exchanged(gap_zero_based_conidx(svc->con_idx), svc->connhdl, p->mtu);
            gap_update_params_t update_param = {0};
            update_param.conn_interval_min_1_25ms = (BLE_WIRELESS_TRANS_SRV_CONN_INTERVAL * 100) / 125;
            update_param.conn_interval_max_1_25ms = (BLE_WIRELESS_TRANS_SRV_CONN_INTERVAL * 100) / 125;
            update_param.max_peripheral_latency = 0;
            gap_update_le_conn_parameters(svc->connhdl, &update_param);
            break;
        }
        case GATT_SERV_EVENT_CHAR_WRITE:
        {
            gatt_server_char_write_t *p = param.char_write;
            if (p->value_offset != 0 || p->value_len == 0 || p->value == NULL)
            {
                return false;
            }
            p->rsp_error_code = ble_wireless_trans_srv_rx_data_received(gap_zero_based_conidx(svc->con_idx), \
                                                                p->value, p->value_len, p->character);
            if (p->rsp_error_code == ATT_ERROR_NO_ERROR)
            {
                gatts_send_write_rsp(p->ctx, p->rsp_error_code);
            }
            return p->rsp_error_code;
        }
        case GATT_SERV_EVENT_DESC_WRITE:
        {
            gatt_server_desc_write_t *p = param.desc_write;
            uint16_t config = CO_COMBINE_UINT16_LE(p->value);
            app_ble_wireless_trans_server_tx_ccc_changed(gap_zero_based_conidx(svc->con_idx), config, (uint8_t *)p->desc_attr->attr_data);
            return true;
        }
        case GATT_SERV_EVENT_NTF_TX_DONE:
        case GATT_SERV_EVENT_INDICATE_CFM:
        {
            gatt_server_indicate_cfm_t *p = param.confirm;
            ble_wireless_trans_srv_tx_data_sent(gap_zero_based_conidx(svc->con_idx), p->character);
            break;
        }
        case GATT_SERV_EVENT_DESC_READ:
        {
            break;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

void ble_wireless_trans_srv_register_callback(ble_wireless_trans_server_cb_t *callback)
{
    ble_wireless_trans_server_cb = callback;
}

void ble_wireless_trans_srv_init(void)
{
    memset(&ble_wireless_trans_server_env, 0, sizeof(struct ble_wireless_trans_server_env_tag));
    ble_wireless_trans_server_env.connectionIndex =  BLE_INVALID_CONNECTION_INDEX;

    gatts_cfg_t gatt_svc_cfg = {0};
    gatt_svc_cfg.preferred_mtu = BLE_WIRELESS_TRANS_SRV_PREFERRED_MTU;
    gatt_svc_cfg.dont_delay_report_conn_open = true;

    gatts_register_service(g_ble_wireless_trans_attr_list, \
                            ARRAY_SIZE(g_ble_wireless_trans_attr_list), \
                            ble_wireless_trans_srv_callback, &gatt_svc_cfg);
}

#endif /* BLE_WIRELESS_TRANS_SRV_ENABLED */
