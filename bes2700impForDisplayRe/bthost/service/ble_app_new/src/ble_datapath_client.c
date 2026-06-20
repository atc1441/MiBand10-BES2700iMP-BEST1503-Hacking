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
#ifdef CFG_APP_DATAPATH_CLIENT
#include "app_ble.h"
#include "gatt_service.h"
#include "ble_datapath_client.h"
#include "app_ble_cmd_handler.h"

#define USE_128BIT_UUID 1

#if USE_128BIT_UUID

#ifdef IS_USE_CUSTOM_BLE_DATAPATH_PROFILE_UUID_ENABLED
#define dpc_service_uuid_128_le      TW_BLE_DATAPATH_SERVICE_UUID
#define dpc_rx_character_uuid_128_le TW_BLE_DATAPATH_TX_CHAR_VAL_UUID
#define dpc_tx_character_uuid_128_le TW_BLE_DATAPATH_RX_CHAR_VAL_UUID
#else
#define dpc_service_uuid_128_le      0x12,0x34,0x56,0x78,0x90,0x00,0x00,0x80,0x00,0x10,0x00,0x00,0x00,0x01,0x00,0x01
#define dpc_rx_character_uuid_128_le 0x12,0x34,0x56,0x78,0x91,0x00,0x00,0x80,0x00,0x10,0x00,0x00,0x00,0x02,0x00,0x02
#define dpc_tx_character_uuid_128_le 0x12,0x34,0x56,0x78,0x92,0x00,0x00,0x80,0x00,0x10,0x00,0x00,0x00,0x03,0x00,0x03
#endif

#else //USE_128BIT_UUID

#define dpc_service_uuid_16      0xFEF8
#define dpc_rx_character_uuid_16 0xFEF9
#define dpc_tx_character_uuid_16 0xFEFA
#endif

static uint8_t dpc_prf_id = 0;
static app_datapath_client_event_cb_t *dpc_client_cb = NULL;

typedef struct {
    gatt_prf_t head;
    bool peer_write_notified;
    gatt_peer_service_t *peer_service;
    gatt_peer_character_t *peer_char[DPC_CHAR_MAX_NUM];
} dpc_prf_t;

bt_status_t ble_datapath_client_read_req(uint16_t connhdl, uint16_t code)
{
    dpc_prf_t *prf = NULL;
    gatt_peer_character_t *c = NULL;
    bt_status_t status = BT_STS_FAILED;

    bool is_desc = (code & DPC_DESC_MASK) ? true : false;
    uint16_t desc_uuid = 0;

    prf = (dpc_prf_t *)gattc_get_profile(dpc_prf_id, connhdl);
    if (prf == NULL)
    {
        return status;
    }

    switch(code)
    {
        case DPC_NTF_CHAR_DATA: {
            c = prf->peer_char[DPC_CHAR_DATA_RX];
        } break;
        case DPC_WRITE_CHAR_DATA: {
            c = prf->peer_char[DPC_CHAR_DATA_TX];
        } break;
        case DPC_RD_WR_NTF_CCC_CFG: {
            c = prf->peer_char[DPC_CHAR_DATA_RX];
            desc_uuid = GATT_DESC_UUID_CHAR_CLIENT_CONFIG;
        } break;
        case DPC_RD_NTF_CUD: {
            c = prf->peer_char[DPC_CHAR_DATA_RX];
            desc_uuid = GATT_DESC_UUID_CHAR_USER_DESC;
        } break;
        case DPC_RD_WR_CUD: {
            c = prf->peer_char[DPC_CHAR_DATA_TX];
            desc_uuid = GATT_DESC_UUID_CHAR_USER_DESC;
        } break;
        default: break;
    }
    if (c == NULL)
    {
        return status;
    }

    if (!is_desc) {
        status = gattc_read_character_value(&prf->head, c);
    } else {
        status = gattc_read_descriptor_value(&prf->head, c, desc_uuid);
    }

    return status;
}

bt_status_t ble_datapath_client_write_req(uint16_t connhdl,
                                          uint16_t code,
                                          uint8_t write_cmd,
                                          const uint8_t *value, uint16_t len)
{
    dpc_prf_t *prf = NULL;
    gatt_peer_character_t *c = NULL;
    bool notify_enabled = false;
    bt_status_t status = BT_STS_FAILED;

    if (code >= DPC_CHAR_MAX_NUM || value == NULL || len == 0)
    {
        return status;
    }

    prf = (dpc_prf_t *)gattc_get_profile(dpc_prf_id, connhdl);
    if (prf == NULL)
    {
        return status;
    }

    c = prf->peer_char[code];
    if (c == NULL)
    {
        return status;
    }

    if(code == DPC_CHAR_DATA_RX)
    {
        notify_enabled = value[0] ? true : false;
        status = gattc_write_cccd_descriptor(&prf->head, c, notify_enabled, false);
        prf->peer_write_notified = notify_enabled;
    }
    else if(code == DPC_CHAR_DATA_TX)
    {
        if (write_cmd)
        {
            status = gattc_write_character_command(&prf->head, c, value, len, false);
        }
        else
        {
            status = gattc_write_character_value(&prf->head, c, value, len);
        }
    }

    return status;
}

static void ble_datapath_client_conn_handler(dpc_prf_t *prf)
{
    uint8_t conidx = gap_zero_based_conidx(prf->head.con_idx);

    ble_datapath_client_start_discover(conidx);

    if(dpc_client_cb && dpc_client_cb->dpc_connected_done_cb)
    {
        dpc_client_cb->dpc_connected_done_cb(conidx);
    }
}

static void ble_datapath_client_disconn_handler(dpc_prf_t *prf)
{
    uint8_t conidx = gap_zero_based_conidx(prf->head.con_idx);

    if(dpc_client_cb && dpc_client_cb->dpc_disconnected_done_cb)
    {
        dpc_client_cb->dpc_disconnected_done_cb(conidx);
    }
}

static void ble_datapath_client_mtu_changed(dpc_prf_t *prf, uint16_t mtu)
{
    uint8_t conidx = gap_zero_based_conidx(prf->head.con_idx);

    if(dpc_client_cb && dpc_client_cb->dpc_mtu_exchanged_done_cb)
    {
        dpc_client_cb->dpc_mtu_exchanged_done_cb(conidx, mtu);
    }
}

static void ble_datapath_client_discover_complete(dpc_prf_t *prf)
{
    uint8_t conidx = gap_zero_based_conidx(prf->head.con_idx);
    uint8_t value[2] = {0x01, 00};

    ble_datapath_client_write_req(prf->head.connhdl, DPC_CHAR_DATA_RX, false, value, 2);

    if(dpc_client_cb && dpc_client_cb->dpc_discover_done_cb)
    {
        dpc_client_cb->dpc_discover_done_cb(conidx);
    }
}

static void ble_datapath_client_char_read_complete(dpc_prf_t *prf,
                                                   uint8_t error_code,
                                                   const gatt_peer_character_t *c,
                                                   const uint8_t *value, uint16_t len)
{
    dpc_char_enum_t char_enum = DPC_CHAR_MAX_NUM;

    if (error_code)
    {
        TRACE(0,"%s err code:%d", __func__, error_code);
    }

    if (c == prf->peer_char[DPC_CHAR_DATA_RX]) {
        char_enum = DPC_CHAR_DATA_RX;
    } else if (c == prf->peer_char[DPC_CHAR_DATA_TX]) {
        char_enum = DPC_CHAR_DATA_TX;
    }

    if(value)
    {
        TRACE(0,"%s char_enum %d read cmp:", __func__, char_enum);
        DUMP8("%02x ", value, len);
    }
}

static void ble_datapath_client_desc_read_complete(dpc_prf_t *prf,
                                                   uint8_t error_code,
                                                   const gatt_peer_character_t *c,
                                                   uint16_t desc_uuid,
                                                   const uint8_t *value, uint16_t len)
{
    dpc_char_enum_t char_enum = DPC_CHAR_MAX_NUM;
    dpc_desc_enum_t desc_enum = DPC_DESC_MAX_NUM;

    if (error_code)
    {
        TRACE(0,"%s err code:%d", __func__, error_code);
    }

    if (c == prf->peer_char[DPC_CHAR_DATA_RX])
    {
        char_enum = DPC_CHAR_DATA_RX;
        if(desc_uuid == GATT_DESC_UUID_CHAR_CLIENT_CONFIG)
        {
            desc_enum = DPC_DESC_RX_CFG;
        }
        else if(desc_uuid == GATT_DESC_UUID_CHAR_USER_DESC)
        {
            desc_enum = DPC_DESC_RX_CUD;
        }
    }
    else if (c == prf->peer_char[DPC_CHAR_DATA_TX])
    {
        char_enum = DPC_CHAR_DATA_TX;
        if(desc_uuid == GATT_DESC_UUID_CHAR_USER_DESC)
        {
            desc_enum = DPC_DESC_TX_CUD;
        }
    }

    if(value)
    {
        TRACE(0,"%s char_enum %d desc_enmu %d read cmp:", __func__, char_enum, desc_enum);
        DUMP8("%02x ", value, len);
    }
}

static void ble_datapath_client_char_write_complete(dpc_prf_t *prf,
                                                    uint8_t error_code,
                                                    const gatt_peer_character_t *c)
{
    uint8_t conidx = gap_zero_based_conidx(prf->head.con_idx);

    if (error_code)
    {
        TRACE(0, "%s err_code %d", __func__, error_code);
    }

    if(dpc_client_cb && dpc_client_cb->dpc_tx_done_cb)
    {
        dpc_client_cb->dpc_tx_done_cb(conidx, error_code);
    }
}

static void ble_datapath_client_desc_write_complete(dpc_prf_t *prf,
                                                    uint8_t error_code,
                                                    const gatt_peer_character_t *c,
                                                    uint16_t desc_uuid)
{
    TRACE(0, "%s err_code %d", __func__, error_code);
}

static void ble_datapath_client_rx_data_received(dpc_prf_t *prf,
                                                 const gatt_peer_character_t *c,
                                                 const uint8_t *value, uint16_t len)
{
    uint8_t conidx = gap_zero_based_conidx(prf->head.con_idx);

    // TRACE(0, "%s len %d", __func__, len);
    // DUMP8("%02x ", value, len);
    #ifndef __INTERCONNECTION__
        BLE_custom_command_receive_data((uint8_t *)value, len, prf->head.con_idx);
    #endif
    if(dpc_client_cb && dpc_client_cb->dpc_data_received_cb)
    {
        dpc_client_cb->dpc_data_received_cb(conidx, value, len);
    }
}

void ble_datapath_client_start_discover(uint8_t conidx)
{
    uint16_t connhdl = app_ble_get_conhdl_from_conidx(conidx);

    dpc_prf_t *prf = (dpc_prf_t *)gattc_get_profile(dpc_prf_id, connhdl);
    if (prf == NULL)
    {
        return;
    }

#if USE_128BIT_UUID
    uint8_t dp_service_uuid_le[16] = {dpc_service_uuid_128_le};
    gattc_discover_service(&prf->head, 0, dp_service_uuid_le);
#else
    gattc_discover_service(&prf->head, dpc_service_uuid_16, NULL);
#endif

    return;
}

static int ble_datapath_client_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param)
{
    dpc_prf_t *dpc = (dpc_prf_t *)prf;

    switch (event)
    {
        case GATT_PROF_EVENT_OPENED:
        {
            ble_datapath_client_conn_handler(dpc);
            break;
        }
        case GATT_PROF_EVENT_CLOSED:
        {
            ble_datapath_client_disconn_handler(dpc);
            break;
        }
        case GATT_PROF_EVENT_MTU_CHANGED:
        {
            ble_datapath_client_mtu_changed(dpc, param.mtu_changed->mtu);
            break;
        }
        case GATT_PROF_EVENT_SERVICE:
        {
            gatt_profile_service_t *p = param.service;
            gatt_peer_service_t *s = p->service;
            if (p->error_code != ATT_ERROR_NO_ERROR)
            {
                break;
            }
            dpc->peer_service = s;
#if USE_128BIT_UUID
            gatt_char_uuid_t gap_chars[] = {
                    {{dpc_rx_character_uuid_128_le}, true},
                    {{dpc_tx_character_uuid_128_le}, true},
                };

            gattc_discover_multi_128_characters(prf, s, gap_chars, ARRAY_SIZE(gap_chars));
#else
            uint16_t gap_chars[] = {
                    dpc_rx_character_uuid_16,
                    dpc_tx_character_uuid_16,
                };

            gattc_discover_multi_characters(prf, s, gap_chars, sizeof(gap_chars)/sizeof(uint16_t));
#endif
            break;
        }
        case GATT_PROF_EVENT_CHARACTER:
        {
            gatt_profile_character_t *p = param.character;
            gatt_peer_character_t *c = p->character;
            if (p->discover_idx < DPC_CHAR_MAX_NUM)
            {
                if (p->error_code == ATT_ERROR_NO_ERROR)
                {
                    dpc->peer_char[p->discover_idx] = c;
                }
                else
                {
                    dpc->peer_char[p->discover_idx] = NULL;
                }
            }
            if (p->discover_cmpl)
            {
                ble_datapath_client_discover_complete(dpc);
            }
            break;
        }
        case GATT_PROF_EVENT_CHAR_READ_RSP:
        {
            gatt_profile_char_read_rsp_t *p = param.char_read_rsp;
            ble_datapath_client_char_read_complete(dpc, p->error_code, p->character, p->value, p->value_len);
            break;
        }
        case GATT_PROF_EVENT_DESC_READ_RSP:
        {
            gatt_profile_desc_read_rsp_t *p = param.desc_read_rsp;
            ble_datapath_client_desc_read_complete(dpc, p->error_code, p->character, p->desc_uuid, p->value, p->value_len);
            break;
        }
        case GATT_PROF_EVENT_CHAR_WRITE_RSP:
        {
            gatt_profile_char_write_rsp_t *p = param.char_write_rsp;
            ble_datapath_client_char_write_complete(dpc,p->error_code, p->character);
            break;
        }
        case GATT_PROF_EVENT_DESC_WRITE_RSP:
        {
            gatt_profile_desc_write_rsp_t *p = param.desc_write_rsp;
            ble_datapath_client_desc_write_complete(dpc, p->error_code, p->character, p->desc_uuid);
            break;
        }
        case GATT_PROF_EVENT_NOTIFY:
        {
            gatt_profile_recv_notify_t *ntf = param.notify;
            if (ntf->character == dpc->peer_char[DPC_CHAR_DATA_RX])
            {
                ble_datapath_client_rx_data_received(dpc, ntf->character, ntf->value, ntf->value_len);
            }
            break;
        }
        default:
            break;
    }
    return 0;
}

void ble_datapath_client_init(void)
{
    gattc_cfg_t prf_cfg = {0};
    prf_cfg.prf_size = sizeof(dpc_prf_t);
    dpc_prf_id = gattc_register_profile(ble_datapath_client_callback, &prf_cfg);
}

void app_datapath_client_register_callback(app_datapath_client_event_cb_t *callback)
{
    dpc_client_cb = callback;
}

void app_datapath_client_control_notification(uint8_t conidx, bool isEnable)
{
    uint16_t connhdl = app_ble_get_conhdl_from_conidx(conidx);
    uint8_t value[2] = {0};
    dpc_prf_t *prf = (dpc_prf_t *)gattc_get_profile(dpc_prf_id, connhdl);
    if (prf == NULL)
    {
        return;
    }

    if (isEnable != prf->peer_write_notified)
    {
        value[0] = isEnable;
        ble_datapath_client_write_req(connhdl, DPC_CHAR_DATA_RX, false, value, 2);
    }
}

void app_datapath_client_send_data_via_write_command(uint8_t conidx, uint8_t* ptrData, uint32_t length)
{
    uint16_t connhdl = app_ble_get_conhdl_from_conidx(conidx);
    ble_datapath_client_write_req(connhdl, DPC_CHAR_DATA_TX, true, ptrData, length);
}

void app_datapath_client_send_data_via_write_request(uint8_t conidx, uint8_t* ptrData, uint32_t length)
{
    uint16_t connhdl = app_ble_get_conhdl_from_conidx(conidx);
    ble_datapath_client_write_req(connhdl, DPC_CHAR_DATA_TX, false, ptrData, length);
}


#endif //CFG_APP_DATAPATH_CLIENT
