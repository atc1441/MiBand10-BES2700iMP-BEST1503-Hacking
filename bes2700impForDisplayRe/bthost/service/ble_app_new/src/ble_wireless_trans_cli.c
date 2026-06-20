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
#ifdef BLE_WIRELESS_TRANS_CLI_ENABLED
#include "app_bt.h"
#include "app_ble.h"
#include "app_custom.h"
#include "gatt_service.h"
#include "ble_wireless_trans_cli.h"

#define BLE_WIRELESS_TRANS_CLI_PREFERRED_MTU 512
#define ble_wireless_trans_service_uuid_16                    0xFFF0
#define ble_wireless_trans_send_attr0_char_uuid_16            0xFFF3 // uart tx(write)
#define ble_wireless_trans_recv_attr1_char_uuid_16            0xFFF7 //data sm rx
#define ble_wireless_trans_recv_attr2_char_uuid_16            0xFFF4 //data sm rx
#define ble_wireless_trans_recv_send_attr_char_uuid_16        0xFFF5 //not use

static uint8_t ble_wireless_trans_cli_prf_id = GATT_PRF_INVALID;
static ble_wireless_trans_client_cb_t *ble_wireless_trans_client_cb = NULL;

bt_status_t ble_wireless_trans_cli_write_req(uint16_t connhdl, bool isDescrip,
                                            ble_wireless_trans_cli_char_enum_t char_enum,
                                            const uint8_t *value, uint16_t len)
{
    bt_status_t status = BT_STS_FAILED;
    ble_wireless_trans_cli_prf_t *prf = NULL;
    gatt_peer_character_t *c = NULL;

    if (char_enum >= CLI_CHAR_MAX_NUM || value == NULL || len == 0)
    {
        return status;
    }

    prf = (ble_wireless_trans_cli_prf_t *)gattc_get_profile(ble_wireless_trans_cli_prf_id, connhdl);
    if (prf == NULL)
    {
        return status;
    }

    c = prf->peer_char[char_enum];
    if (c == NULL)
    {
        return status;
    }

    if (char_enum == CLI_CHAR_SEND_ATTR0)
    {
        if (isDescrip && (value[0] & GATT_CCCD_SET_NOTIFICATION))
        {
            status = gattc_write_cccd_descriptor(&prf->head, c, true, false);
            prf->NtfIndCfgAttr[CLI_CHAR_SEND_ATTR0] |= 0x01<<8;
        }
        else if (isDescrip && (value[0] & GATT_CCCD_SET_INDICATION))
        {
            status = gattc_write_cccd_descriptor(&prf->head, c, false, true);
            prf->NtfIndCfgAttr[CLI_CHAR_SEND_ATTR0] |= 0x01;
        }
        else if (isDescrip)
        {
            status = gattc_write_cccd_descriptor(&prf->head, c, false, false);
            prf->NtfIndCfgAttr[CLI_CHAR_SEND_ATTR0] = 0;
        }
        else
        {
            status = gattc_write_character_value(&prf->head, c, value, len);
        }
    }
    else if (char_enum == CLI_CHAR_RECV_ATTR1)
    {
        if (isDescrip && (value[0] & GATT_CCCD_SET_NOTIFICATION))
        {
            status = gattc_write_cccd_descriptor(&prf->head, c, true, false);
            prf->NtfIndCfgAttr[CLI_CHAR_RECV_ATTR1] |= 0x01<<8;
        }
        else if (isDescrip && (value[0] & GATT_CCCD_SET_INDICATION))
        {
            status = gattc_write_cccd_descriptor(&prf->head, c, false, true);
            prf->NtfIndCfgAttr[CLI_CHAR_RECV_ATTR1] |= 0x01;
        }
        else if (isDescrip)
        {
            status = gattc_write_cccd_descriptor(&prf->head, c, false, false);
            prf->NtfIndCfgAttr[CLI_CHAR_RECV_ATTR1] = 0;
        }
        else
        {
            status = gattc_write_character_value(&prf->head, c, value, len);
        }
    }
    else if (char_enum == CLI_CHAR_RECV_ATTR2)
    {
        if (isDescrip && (value[0] & GATT_CCCD_SET_NOTIFICATION))
        {
            status = gattc_write_cccd_descriptor(&prf->head, c, true, false);
            prf->NtfIndCfgAttr[CLI_CHAR_RECV_ATTR2] |= 0x01<<8;
        }
        else if (isDescrip && (value[0] & GATT_CCCD_SET_INDICATION))
        {
            status = gattc_write_cccd_descriptor(&prf->head, c, false, true);
            prf->NtfIndCfgAttr[CLI_CHAR_RECV_ATTR2] |= 0x01;
        }
        else if (isDescrip)
        {
            status = gattc_write_cccd_descriptor(&prf->head, c, false, false);
            prf->NtfIndCfgAttr[CLI_CHAR_RECV_ATTR2] = 0;
        }
        else
        {
            status = gattc_write_character_value(&prf->head, c, value, len);
        }
    }
    else if (char_enum == CLI_CHAR_RECV_SEND_ATTR)
    {
        if (isDescrip && (value[0] & GATT_CCCD_SET_NOTIFICATION))
        {
            status = gattc_write_cccd_descriptor(&prf->head, c, true, false);
            prf->NtfIndCfgAttr[CLI_CHAR_RECV_SEND_ATTR] |= 0x01<<8;
        }
        else if (isDescrip && (value[0] & GATT_CCCD_SET_INDICATION))
        {
            status = gattc_write_cccd_descriptor(&prf->head, c, false, true);
            prf->NtfIndCfgAttr[CLI_CHAR_RECV_SEND_ATTR] |= 0x01;
        }
        else if (isDescrip)
        {
            status = gattc_write_cccd_descriptor(&prf->head, c, false, false);
            prf->NtfIndCfgAttr[CLI_CHAR_RECV_SEND_ATTR] = 0;
        }
        else
        {
            status = gattc_write_character_command(&prf->head, c, value, len, false);
        }
    }

    return status;
}

void ble_wireless_trans_cli_recv_attr0_data_handler(uint8_t con_idx, const uint8_t *value, uint16_t len)
{
    // not use
}

void ble_wireless_trans_cli_recv_attr1_data_handler(uint8_t con_idx, const uint8_t *value, uint16_t len)
{
    uint8_t conidx = gap_zero_based_conidx(con_idx);
    if (ble_wireless_trans_client_cb && ble_wireless_trans_client_cb->data_received_cb)
    {
        ble_wireless_trans_client_cb->data_received_cb(conidx, CLI_CHAR_RECV_ATTR1, (uint8_t *)value, len);
    }
}

void ble_wireless_trans_cli_recv_attr2_data_handler(uint8_t con_idx, const uint8_t *value, uint16_t len)
{
    uint8_t conidx = gap_zero_based_conidx(con_idx);
    if (ble_wireless_trans_client_cb && ble_wireless_trans_client_cb->data_received_cb)
    {
        ble_wireless_trans_client_cb->data_received_cb(conidx, CLI_CHAR_RECV_ATTR2, (uint8_t *)value, len);
    }
}

void ble_wireless_trans_cli_recv_attr_data_handler(uint8_t con_idx, const uint8_t *value, uint16_t len)
{
    // not use
}


uint8_t ble_wireless_trans_cli_send_attr0_data_handler(uint16_t connhdl, const uint8_t *value, uint16_t len)
{
    if (value && len)
    {
        return ble_wireless_trans_cli_write_req(connhdl, false, CLI_CHAR_SEND_ATTR0, value, len);
    }

    return BT_STS_FAILED;
}

void ble_wireless_trans_cli_write_char_complete_handler(uint8_t con_idx,
                                              ble_wireless_trans_cli_prf_t *client_prf,
                                              uint8_t err_code,
                                              gatt_peer_character_t *p_char)
{
    uint8_t char_index = CLI_CHAR_MAX_NUM;
    uint8_t conidx = gap_zero_based_conidx(con_idx);

    if (p_char == client_prf->peer_char[CLI_CHAR_SEND_ATTR0])
    {
        char_index = CLI_CHAR_SEND_ATTR0;
    }
    else if (p_char == client_prf->peer_char[CLI_CHAR_RECV_ATTR1])
    {
        char_index = CLI_CHAR_RECV_ATTR1;
    }
    else if (p_char == client_prf->peer_char[CLI_CHAR_RECV_ATTR2])
    {
        char_index = CLI_CHAR_RECV_ATTR2;
    }
    else if (p_char == client_prf->peer_char[CLI_CHAR_RECV_SEND_ATTR])
    {
        char_index = CLI_CHAR_RECV_SEND_ATTR;
    }
    if (ble_wireless_trans_client_cb && ble_wireless_trans_client_cb->tx_done_cb)
    {
        ble_wireless_trans_client_cb->tx_done_cb(conidx, char_index, err_code);
    }
}

void ble_wireless_trans_cli_write_desc_complete_handler(ble_wireless_trans_cli_prf_t *client_prf,
                                              uint8_t err_code,
                                              gatt_peer_character_t *p_char)
{
    if(err_code != BT_STS_SUCCESS)
    {
        TRACE(0,"%s code 0x%02x %p", __func__, err_code, p_char);
    }
}

void ble_wireless_trans_cli_discover_cmpl(uint8_t con_idx, uint16_t connhdl)
{
    uint8_t conidx = gap_zero_based_conidx(con_idx);
    /// enable attr1 attr2 notify to start rx test
    uint8_t value[2] = {0x01, 0x00};
    ble_wireless_trans_cli_write_req(connhdl, true, CLI_CHAR_RECV_ATTR1, value, 2);
    ble_wireless_trans_cli_write_req(connhdl, true, CLI_CHAR_RECV_ATTR2, value, 2);

    if (ble_wireless_trans_client_cb && ble_wireless_trans_client_cb->discover_done_cb)
    {
        ble_wireless_trans_client_cb->discover_done_cb(conidx);
    }
}

void ble_wireless_trans_cli_conn_handler(uint8_t con_idx, uint16_t connhdl)
{
    uint8_t conidx = gap_zero_based_conidx(con_idx);
    if (ble_wireless_trans_client_cb && ble_wireless_trans_client_cb->connected_done_cb)
    {
        ble_wireless_trans_client_cb->connected_done_cb(conidx, connhdl);
    }
}

void ble_wireless_trans_cli_exchangemtu_handler(uint8_t con_idx, uint16_t mtu)
{
    uint8_t conidx = gap_zero_based_conidx(con_idx);
    if (ble_wireless_trans_client_cb && ble_wireless_trans_client_cb->mtu_exchanged_done_cb)
    {
        ble_wireless_trans_client_cb->mtu_exchanged_done_cb(conidx, mtu);
    }
}

void ble_wireless_trans_cli_disconn_handler(uint8_t con_idx)
{
    uint8_t conidx = gap_zero_based_conidx(con_idx);
    if (ble_wireless_trans_client_cb && ble_wireless_trans_client_cb->disconnected_done_cb)
    {
        ble_wireless_trans_client_cb->disconnected_done_cb(conidx);
    }
}

bt_status_t ble_wireless_trans_cli_start_discover(uint16_t connhdl)
{
    ble_wireless_trans_cli_prf_t *prf = NULL;

    prf = (ble_wireless_trans_cli_prf_t *)gattc_get_profile(ble_wireless_trans_cli_prf_id, connhdl);
    if (prf == NULL)
    {
        return BT_STS_FAILED;
    }

    gattc_discover_service(&prf->head, ble_wireless_trans_service_uuid_16, NULL);

    return BT_STS_SUCCESS;
}

static int ble_wireless_trans_cli_callback(gatt_prf_t *prf,
                                 gatt_profile_event_t event,
                                 gatt_profile_callback_param_t param)
{
    ble_wireless_trans_cli_prf_t *client_prf = (ble_wireless_trans_cli_prf_t *)prf;

    switch (event)
    {
        case GATT_PROF_EVENT_OPENED:
            app_ble_set_phy_mode(gap_zero_based_conidx(param.opened->conn->con_idx), \
                                                    GAP_PHY_BIT_LE_2M, GAP_PHY_BIT_LE_2M, 0);
            ble_wireless_trans_cli_conn_handler(param.opened->conn->con_idx, param.opened->conn->connhdl);
            break;
        case GATT_PROF_EVENT_CLOSED:
            ble_wireless_trans_cli_disconn_handler(param.closed->conn->con_idx);
            break;
        case GATT_PROF_EVENT_MTU_CHANGED:
            ble_wireless_trans_cli_exchangemtu_handler(param.mtu_changed->conn->con_idx, param.mtu_changed->mtu);
            break;
        case GATT_PROF_EVENT_CONN_UPDATED:
            ble_wireless_trans_cli_start_discover(param.conn_updated->conn->connhdl);
            break;
        case GATT_PROF_EVENT_SERVICE:
        {
            gatt_profile_service_t *p = param.service;
            gatt_peer_service_t *s = p->service;
            if (p->error_code != ATT_ERROR_NO_ERROR)
            {
                break;
            }
            client_prf->peer_service = s;
            uint16_t gap_chars[] = {
                    ble_wireless_trans_send_attr0_char_uuid_16,
                    ble_wireless_trans_recv_attr1_char_uuid_16,
                    ble_wireless_trans_recv_attr2_char_uuid_16,
                    ble_wireless_trans_recv_send_attr_char_uuid_16,
                };
            gattc_discover_multi_characters(prf, s, gap_chars, ARRAY_SIZE(gap_chars));
            break;
        }
        case GATT_PROF_EVENT_CHARACTER:
        {
            gatt_profile_character_t *p = param.character;
            gatt_peer_character_t *c = p->character;
            if (p->discover_idx < CLI_CHAR_MAX_NUM)
            {
                if (p->error_code == ATT_ERROR_NO_ERROR)
                {
                    client_prf->peer_char[p->discover_idx] = c;
                }
                else
                {
                    client_prf->peer_char[p->discover_idx] = NULL;
                }
            }
            if (p->discover_cmpl)
            {
                ble_wireless_trans_cli_discover_cmpl(client_prf->head.con_idx, client_prf->head.connhdl);
            }
            break;
        }
        case GATT_PROF_EVENT_NOTIFY:
        {
            gatt_profile_recv_notify_t *p = param.notify;
            if (p->service->service_uuid != ble_wireless_trans_service_uuid_16)
            {
                break;
            }
            if (p->character == client_prf->peer_char[CLI_CHAR_SEND_ATTR0])
            {
                ble_wireless_trans_cli_recv_attr0_data_handler(p->conn->con_idx, p->value, p->value_len);
            }
            else if (p->character == client_prf->peer_char[CLI_CHAR_RECV_ATTR1])
            {
                ble_wireless_trans_cli_recv_attr1_data_handler(p->conn->con_idx, p->value, p->value_len);
            }
            else if (p->character == client_prf->peer_char[CLI_CHAR_RECV_ATTR2])
            {
                ble_wireless_trans_cli_recv_attr2_data_handler(p->conn->con_idx, p->value, p->value_len);
            }
            else if (p->character == client_prf->peer_char[CLI_CHAR_RECV_SEND_ATTR])
            {
                ble_wireless_trans_cli_recv_attr_data_handler(p->conn->con_idx, p->value, p->value_len);
            }

            break;
        }
        case GATT_PROF_EVENT_CHAR_READ_RSP:
        case GATT_PROF_EVENT_DESC_READ_RSP:
            break;
        case GATT_PROF_EVENT_CHAR_WRITE_RSP:
        {
            gatt_profile_char_write_rsp_t *p_char_wr_rsp = param.char_write_rsp;
            ble_wireless_trans_cli_write_char_complete_handler(p_char_wr_rsp->conn->con_idx, client_prf, \
                                                                p_char_wr_rsp->error_code, p_char_wr_rsp->character);
            break;
        }
        case GATT_PROF_EVENT_DESC_WRITE_RSP:
        {
            gatt_profile_desc_write_rsp_t *p_desc_wr_rsp = param.desc_write_rsp;
            ble_wireless_trans_cli_write_desc_complete_handler(client_prf, p_desc_wr_rsp->error_code, p_desc_wr_rsp->character);
            break;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

void ble_wireless_trans_cli_register_callback(ble_wireless_trans_client_cb_t *callback)
{
    ble_wireless_trans_client_cb = callback;
}

void ble_wireless_trans_cli_init(void)
{
    TRACE(0,"%s", __func__);
    gattc_cfg_t prf_cfg = {0};
    prf_cfg.prf_size = sizeof(ble_wireless_trans_cli_prf_t);
    prf_cfg.preferred_mtu = BLE_WIRELESS_TRANS_CLI_PREFERRED_MTU;
    ble_wireless_trans_cli_prf_id = gattc_register_profile(ble_wireless_trans_cli_callback, &prf_cfg);
}

#endif /* BLE_WIRELESS_TRANS_CLI_ENABLED */
