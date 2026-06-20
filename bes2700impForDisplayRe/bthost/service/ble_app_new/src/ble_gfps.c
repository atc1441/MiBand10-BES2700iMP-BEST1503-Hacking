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
#undef MOUDLE
#define MOUDLE APP_BLE
#include "app_ble.h" // keep first line for CFG_APP_GFPS define
#ifdef GFPS_ENABLED // google fast pair server (server: fast pair provider, client: fast pair seeker)
#include "ble_gfps.h"
#include "bt_drv_interface.h"
#include "ble_gfps_common.h"
#include "hci_i.h"
#include "apps.h"
#include "gfps_ble.h"
#ifdef RTC_CALENDAR
#include "nvrecord_fp_account_key.h"
#include "gfps_ble.h"
#include "pmu.h"
#include "cmsis.h"
#endif


#define GFPS_PREFERRED_MTU 512
#define GFPSP_VAL_MAX_LEN 128
#define GFPSP_SYS_ID_LEN 0x08
#define GFPSP_IEEE_CERTIF_MIN_LEN 0x06
#define GFPSP_PNP_ID_LEN 0x07

#define GFPS_USE_128BIT_UUID
#define BLE_GFPS_SERVICE_UUID            0xFE2C
#define BLE_SPOT_SERVICE_UUID            0xFEAA

#ifdef GFPS_USE_128BIT_UUID
#define gfps_model_id_char_uuid_128_le      0xEA,0x0B,0x10,0x32,0xDE,0x01,0xB0,0x8E,0x14,0x48,0x66,0x83,0x33,0x12,0x2C,0xFE
#define gfps_keybased_pairing_uuid_128_le   0xEA,0x0B,0x10,0x32,0xDE,0x01,0xB0,0x8E,0x14,0x48,0x66,0x83,0x34,0x12,0x2C,0xFE
#define gfps_passkey_char_uuid_128_le       0xEA,0x0B,0x10,0x32,0xDE,0x01,0xB0,0x8E,0x14,0x48,0x66,0x83,0x35,0x12,0x2C,0xFE
#define gfps_account_key_uuid_128_le        0xEA,0x0B,0x10,0x32,0xDE,0x01,0xB0,0x8E,0x14,0x48,0x66,0x83,0x36,0x12,0x2C,0xFE
#define gfps_additional_data_uuid_128_le    0xEA,0x0B,0x10,0x32,0xDE,0x01,0xB0,0x8E,0x14,0x48,0x66,0x83,0x37,0x12,0x2C,0xFE
#define gfps_beacon_actions_uuid_128_le     0xEA,0x0B,0x10,0x32,0xDE,0x01,0xB0,0x8E,0x14,0x48,0x66,0x83,0x38,0x12,0x2C,0xFE
#define gfps_msg_stream_uuid_128_le         0xEA,0x0B,0x10,0x32,0xDE,0x01,0xB0,0x8E,0x14,0x48,0x66,0x83,0x39,0x12,0x2C,0xFE
#define gfps_additional_passkey_uuid_128_le 0xEA,0x0B,0x10,0x32,0xDE,0x01,0xB0,0x8E,0x14,0x48,0x66,0x83,0x3A,0x12,0x2C,0xFE

GATT_DECL_PRI_SERVICE(g_ble_gfps_service,
    BLE_GFPS_SERVICE_UUID);

GATT_DECL_128_LE_CHAR(g_ble_gfps_model_id_character,
    gfps_model_id_char_uuid_128_le,
    GATT_RD_REQ,
    ATT_SEC_NONE);

GATT_DECL_128_LE_CHAR(g_ble_gfps_keybased_pairing_character,
    gfps_keybased_pairing_uuid_128_le,
    GATT_WR_REQ|GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_gfps_keybased_pairing_cccd,
    ATT_SEC_NONE);

GATT_DECL_128_LE_CHAR(g_ble_gfps_passkey_character,
    gfps_passkey_char_uuid_128_le,
    GATT_WR_REQ|GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_gfps_passkey_cccd,
    ATT_SEC_NONE);

GATT_DECL_128_LE_CHAR(g_ble_gfps_account_key_character,
    gfps_account_key_uuid_128_le,
    GATT_WR_REQ,
    ATT_SEC_NONE);

GATT_DECL_128_LE_CHAR(g_ble_gfps_additional_data_character,
    gfps_additional_data_uuid_128_le,
    GATT_WR_REQ|GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_gfps_additional_data_cccd,
    ATT_SEC_NONE);

GATT_DECL_128_LE_CHAR(g_ble_gfps_additional_passkey_character,
    gfps_additional_passkey_uuid_128_le,
    GATT_RD_REQ|GATT_WR_REQ|GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_gfps_additional_passkey_cccd,
    ATT_SEC_NONE);

GATT_DECL_128_LE_CHAR(g_ble_gfps_msg_stream,
    gfps_msg_stream_uuid_128_le,
    GATT_RD_REQ,
    ATT_SEC_NONE);

#ifdef SPOT_ENABLED
GATT_DECL_128_LE_CHAR(g_ble_gfps_beacon_actions_character,
    gfps_beacon_actions_uuid_128_le,
    GATT_RD_REQ|GATT_WR_REQ|GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_gfps_beacon_actions_cccd,
    ATT_SEC_NONE);
#endif

#else /* GFPS_USE_128BIT_UUID */
#define gfps_model_id_char_uuid_16      0x1233
#define gfps_keybased_pairing_uuid_16   0x1234
#define gfps_passkey_char_uuid_16       0x1235
#define gfps_account_key_uuid_16        0x1236
#define gfps_additional_data_uuid_16    0x1237
#define gfps_beacon_actions_uuid_16     0x1238
#define gfps_additional_passkey_uuid_16 0x123A

GATT_DECL_PRI_SERVICE(g_ble_gfps_service,
    BLE_GFPS_SERVICE_UUID);

GATT_DECL_CHAR(g_ble_gfps_model_id_character,
    gfps_model_id_char_uuid_16,
    GATT_RD_REQ,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_gfps_keybased_pairing_character,
    gfps_keybased_pairing_uuid_16,
    GATT_WR_REQ|GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_gfps_keybased_pairing_cccd,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_gfps_passkey_character,
    gfps_passkey_char_uuid_16,
    GATT_WR_REQ|GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_gfps_passkey_cccd,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_gfps_account_key_character,
    gfps_account_key_uuid_16,
    GATT_WR_REQ,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_gfps_additional_data_character,
    gfps_additional_data_uuid_16,
    GATT_WR_REQ|GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_gfps_additional_data_cccd,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_gfps_additional_passkey_character,
    gfps_additional_passkey_uuid_16,
    GATT_RD_REQ|GATT_WR_REQ|GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_gfps_additional_passkey_cccd,
    ATT_SEC_NONE);

#ifdef SPOT_ENABLED
GATT_DECL_CHAR(g_ble_gfps_beacon_actions_character,
    gfps_beacon_actions_uuid_16,
    GATT_RD_REQ|GATT_WR_REQ|GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_gfps_beacon_actions_cccd,
    ATT_SEC_NONE);
#endif

#endif /* GFPS_USE_128BIT_UUID */

static const gatt_attribute_t g_ble_gfps_attr_list[] = {
    /* Service */
    gatt_attribute(g_ble_gfps_service),
    /* Characteristics */
    gatt_attribute(g_ble_gfps_model_id_character),
    /* Characteristics */
    gatt_attribute(g_ble_gfps_keybased_pairing_character),
    gatt_attribute(g_ble_gfps_keybased_pairing_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_gfps_passkey_character),
    gatt_attribute(g_ble_gfps_passkey_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_gfps_account_key_character),
    /* Characteristics */
    gatt_attribute(g_ble_gfps_additional_data_character),
    gatt_attribute(g_ble_gfps_additional_data_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_gfps_additional_passkey_character),
    gatt_attribute(g_ble_gfps_additional_passkey_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_gfps_msg_stream),
#ifdef SPOT_ENABLED
    /* Characteristics */
    gatt_attribute(g_ble_gfps_beacon_actions_character),
    gatt_attribute(g_ble_gfps_beacon_actions_cccd),
#endif

};

typedef struct {
    gatt_svc_t head;
    uint32_t l2cap_handle;
} ble_gfps_t;

typedef struct {
    ble_app_gfps_event_cb callback;
} ble_gfps_global_t;

static ble_gfps_global_t g_ble_app_gfps_global;
ble_gfps_t *ble_get_gfps(uint16_t connhdl)
{
    return (ble_gfps_t *)gatts_get_service(connhdl, g_ble_gfps_service, 0);
}

ble_gfps_global_t *ble_get_gfps_global(void)
{
    return &g_ble_app_gfps_global;
}

ble_gfps_t *ble_get_gfps_by_conidx(uint8_t conidx)
{
    ble_gfps_t *gfps = ble_get_gfps(gap_zero_based_ble_conidx_as_hdl(conidx));
    if (gfps == NULL)
    {
        CO_LOG_ERR_1(BT_STS_INVALID_CONN_INDEX, conidx);
    }
    return gfps;
}

void ble_app_gfps_register_event_callback(ble_app_gfps_event_cb func)
{
    ble_gfps_global_t *g = ble_get_gfps_global();
    if (g)
    {
        g->callback = func;
    }
}

void ble_app_gfps_get_data(uint8_t conidx, uint8_t event, uint8_t *outData, uint16_t *outLen)
{
    ble_gfps_global_t *g = ble_get_gfps_global();

    if (!g || !g->callback || !outData)
    {
        return;
    }

    bool send = true;
    ble_app_gfps_event_param_t entry;
    entry.conidx = conidx;
    entry.outData = outData;
    switch (event)
    {
        case GFPSP_GET_SPOT_VERSION:
            entry.event = GFPS_BLE_EVT_GET_SPOT_VERSION;
            break;

        case GFPSP_GEN_SPOT_NOUNCE:
            entry.event = GFPS_BLE_EVT_GEN_SPOT_NOUNCE;
            break;

        case GFPSP_GET_SPOT_NOUNCE:
            entry.event = GFPS_BLE_EVT_GET_SPOT_NOUNCE;
            break;

        case GFPSP_GET_MODEL_ID:
            entry.event = GFPS_BLE_EVT_GET_MODEL_ID;
            break;
        case GFPSP_GET_ADDITIONAL_PASSKEY:
            entry.event = GFPS_BLE_EVT_GET_ADDITIONAL_PASS_KEY;
            break;
        case GFPSP_GET_EVENT_STREAM:
            entry.event = GFPS_BLE_EVT_GET_EVENT_STREAM;
            break;
        default:
            send = false;
            break;
    }

    if (send)
    {
        g->callback(&entry);
        if (outLen)
        {
            *outLen = entry.outLen;
        }
    }
}

static uint8_t ble_app_gfps_write_data(ble_gfps_t *gfps, uint8_t event, gatt_server_char_write_t *p)
{
    uint8_t write_rsp_status = ATT_ERROR_NO_ERROR;
    ble_gfps_global_t *g = ble_get_gfps_global();
    ble_app_gfps_event_param_t entry;
    
    if (!g || !g->callback)
    {
        return ATT_ERROR_WR_NOT_PERMITTED;
    }
    
    entry.conidx = gfps->head.con_idx;
    entry.event = event;
    entry.p.packet.pBuf = (uint8_t *)p->value;
    entry.p.packet.len  = p->value_len;
    write_rsp_status = g->callback(&entry);
    
    return (att_error_code_t)write_rsp_status;
}

#if BLE_AUDIO_ENABLED
static int ble_app_gfps_l2cap_callback(uintptr_t connhdl, bt_l2cap_event_t event, bt_l2cap_callback_param_t param)
{
    bt_l2cap_param_t *l2cap = param.param_ptr;
    ble_gfps_t *gfps = NULL;
    ble_app_gfps_event_param_t entry;
    ble_gfps_global_t *g = ble_get_gfps_global();
    TRACE(2,"%s conidx %d", __func__, l2cap ? l2cap->device_id : 0xFF);
    ble_bdaddr_t GetPeerAddr = {{0}};
    app_ble_get_peer_solved_addr(l2cap->device_id, &GetPeerAddr);

    if (event == BT_L2CAP_EVENT_ACCEPT)
    {
        return BT_L2CAP_ACCEPT_REQ;
    }
    else if (event == BT_L2CAP_EVENT_OPENED)
    {
        CO_LOG_INFO_S_1(BT_STS_CHANNEL_STATUS, GFPS, l2cap->error_code);
        if (l2cap->error_code)
        {
            return 0;
        }

        gfps = ble_get_gfps((uint16_t)connhdl);
        if (gfps == NULL)
        {
            CO_LOG_ERR_2(BT_STS_NOT_FOUND, event, connhdl);
            return 0;
        }

        gfps->l2cap_handle = l2cap->l2cap_handle;
        if (g->callback)
        {
            entry.event = GFPS_BLE_EVT_L2CAP_CONNECTED;
            entry.conidx = gap_zero_based_conidx(l2cap->device_id);
            memcpy(&(entry.p.conn.peerAddr), GetPeerAddr.addr, sizeof(bt_bdaddr_t));
            g->callback(&entry);
        }
    }
    else
    {
        gfps = ble_get_gfps((uint16_t)connhdl);
        if (gfps == NULL)
        {
            CO_LOG_ERR_2(BT_STS_NOT_FOUND, event, connhdl);
            return 0;
        }

        if (event == BT_L2CAP_EVENT_CLOSED)
        {
            if (g->callback)
            {
                entry.event = GFPS_BLE_EVT_L2CAP_DISCONNECTED;
                entry.conidx = gap_zero_based_conidx(l2cap->device_id);
                g->callback(&entry);
            }
        }
        else if (event == BT_L2CAP_EVENT_RX_DATA)
        {
            if (g->callback)
            {
                struct pp_buff *ppb = l2cap->tx_priv_rx_ppb;
                TRACE(2, "%s len %d", __func__, ppb->len);
                entry.event = GFPS_BLE_EVT_L2CAP_RX_REC;
                entry.conidx = gap_zero_based_conidx(l2cap->device_id);
                entry.p.packet.pBuf = (uint8_t *)ppb_get_data(ppb);
                entry.p.packet.len = ppb->len;
                g->callback(&entry);
            }
        }
    }

    return 0;
}

uint8_t ble_app_gfps_l2cap_send(uint8_t conidx, uint8_t *data, uint32_t length)
{
    ble_gfps_t *gfps = ble_get_gfps_by_conidx(conidx);
    if (gfps == NULL)
    {
        return BT_STS_INVALID_CONN_INDEX;
    }

    if (bt_l2cap_send_packet(gfps->head.connhdl, gfps->l2cap_handle, data, (uint16_t)length, NULL) != BT_STS_SUCCESS)
    {
        return BT_STS_FAILED;
    }

    return BT_STS_SUCCESS;
}

#endif

void ble_app_gfps_l2cap_disconnect(uint8_t conidx)
{
    ble_gfps_t *gfps = ble_get_gfps_by_conidx(conidx);
    if (gfps == NULL)
    {
        return;
    }

    bt_l2cap_disconnect(gfps->l2cap_handle, 0);
}

static void ble_app_gfps_connected_evt_handler(uint8_t conidx, gap_conn_item_t *conn)
{
    ble_gfps_global_t *g = ble_get_gfps_global();

    if (g && g->callback)
    {
        ble_app_gfps_event_param_t entry = {
            .conidx = conidx,
            .event = GFPS_BLE_EVT_LE_CONNECTED,
        };
        memcpy(&(entry.p.conn.localAddr), conn->own_rpa.address, sizeof(bt_bdaddr_t));
        memcpy(&(entry.p.conn.peerAddr), conn->peer_addr.address, sizeof(bt_bdaddr_t));
        g->callback(&entry);
    }
}

static void ble_app_gfps_disconnected_evt_handler(uint8_t conidx)
{
    ble_gfps_global_t *g = ble_get_gfps_global();

    if (g && g->callback)
    {
        ble_app_gfps_event_param_t entry = {
            .conidx = conidx,
            .event = GFPS_BLE_EVT_LE_DISCONNECTED,
        };
        g->callback(&entry);
    }
}

static int ble_gfps_server_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
{
    ble_gfps_t *gfps = (ble_gfps_t *)svc;
    int ret = 1;

    if (!gfps)
    {
        return 0;
    }

    switch (event)
    {
        case GATT_SERV_EVENT_CONN_OPENED:
        {
            gatt_server_conn_opened_t *p = param.opened;
            ble_app_gfps_connected_evt_handler(gfps->head.con_idx, p->conn);
            break;
        }
        case GATT_SERV_EVENT_CONN_CLOSED:
        {
            ble_app_gfps_disconnected_evt_handler(gfps->head.con_idx);
            break;
        }
        case GATT_SERV_EVENT_CHAR_READ:
        {
            gatt_server_char_read_t *p = param.char_read;
            if (p->character == g_ble_gfps_model_id_character)
            {
                uint8_t modelId[4] = {0};
                ble_app_gfps_get_data(gfps->head.con_idx, GFPSP_GET_MODEL_ID, modelId, NULL);
                gatts_write_read_rsp_data(p->ctx, modelId, sizeof(uint32_t));
            }
#ifdef SPOT_ENABLED
            else if (p->character == g_ble_gfps_beacon_actions_character)
            {
                uint8_t nounce[9] = {0};
                ble_app_gfps_get_data(gfps->head.con_idx, GFPSP_GET_SPOT_VERSION, nounce, NULL);
                ble_app_gfps_get_data(gfps->head.con_idx, GFPSP_GET_SPOT_NOUNCE, nounce+1, NULL);
                gatts_write_read_rsp_data(p->ctx, nounce, sizeof(nounce));
            }
#endif
            else if(p->character == g_ble_gfps_additional_passkey_character)
            {
                 uint8_t encrypted_rsp_data[16]={0};
                 ble_app_gfps_get_data(gfps->head.con_idx, GFPSP_GET_ADDITIONAL_PASSKEY, encrypted_rsp_data, NULL);
                 gatts_write_read_rsp_data(p->ctx, encrypted_rsp_data, sizeof(encrypted_rsp_data));
            }
            else if(p->character == g_ble_gfps_msg_stream)
            {
                uint8_t data[3]= {0};
                ble_app_gfps_get_data(gfps->head.con_idx, GFPSP_GET_EVENT_STREAM, data, NULL);
                gatts_write_read_rsp_data(p->ctx, data, sizeof(data));
            }
            break;
        }
        case GATT_SERV_EVENT_CHAR_WRITE:
        {
            gatt_server_char_write_t *p = param.char_write;
            uint8_t gfps_event;
            bool ntf = true;
            if (p->value_offset != 0 || p->value_len == 0 || p->value == NULL)
            {
                return 0;
            }

            if (p->character == g_ble_gfps_keybased_pairing_character)
            {
                gfps_event = GFPS_BLE_EVT_KEY_BASE_PAIRING_IND;
            }
            else if (p->character == g_ble_gfps_passkey_character)
            {
                gfps_event = GFPS_BLE_EVT_PASS_KEY_IND;
            }
            else if (p->character == g_ble_gfps_account_key_character)
            {
                gfps_event = GFPS_BLE_EVT_ACCOUNT_KEY_IND;
            }
            else if (p->character == g_ble_gfps_additional_data_character)
            {
                gfps_event = GFPS_BLE_EVT_WRITE_NAME_IND;
            }
            else if(p->character == g_ble_gfps_additional_passkey_character)
            {
                gfps_event = GFPS_BLE_EVT_ADDITIONAL_PASS_KEY_IND;
            }
#ifdef SPOT_ENABLED
            else if (p->character == g_ble_gfps_beacon_actions_character)
            {
                gfps_event = GFPS_BLE_EVT_SPOT_WRITE_BEACON_IND;
            }
#endif
            else
            {
                ntf = false;
            }

            if (ntf)
            {
                p->rsp_error_code = ble_app_gfps_write_data(gfps, gfps_event, p);
                ret = (p->rsp_error_code == ATT_ERROR_NO_ERROR);
                gatts_send_write_rsp(p->ctx, p->rsp_error_code);
            }
            break;
        }
        case GATT_SERV_EVENT_DESC_READ:
        {
            gatt_server_desc_read_t *p = param.desc_read;
            uint16_t cccd_config = co_host_to_uint16_le(0x0001);
            gatts_write_read_rsp_data(p->ctx, (uint8_t *)&cccd_config, sizeof(uint16_t));
            ret = true;
            break;
        }

        case GATT_SERV_EVENT_DESC_WRITE:
        {
            gatt_server_desc_write_t *p = param.desc_write;
            uint16_t config = CO_COMBINE_UINT16_LE(p->value);
            bool notify_enabled = false;
            ble_gfps_global_t *g = ble_get_gfps_global();
            if (config & GATT_CCCD_SET_NOTIFICATION)
            {
                notify_enabled = true;
            }
            if ((uint8_t *)p->desc_attr->attr_data == g_ble_gfps_keybased_pairing_cccd)
            {
                event = GFPS_BLE_EVT_KEY_BASE_PAIRING_CCCD;
            }
            else if ((uint8_t *)p->desc_attr->attr_data == g_ble_gfps_passkey_cccd)
            {
                event = GFPS_BLE_EVT_PASS_KEY_CCCD;
            }
            else if((uint8_t *)p->desc_attr->attr_data == g_ble_gfps_additional_passkey_cccd)
            {
                event = GFPS_BLE_EVT_ADDITIONAL_PASS_KEY_CCCD;
            }
#ifdef SPOT_ENABLED
            else if ((uint8_t *)p->desc_attr->attr_data == g_ble_gfps_beacon_actions_cccd)
            {
                event = GFPS_BLE_EVT_SPOT_BEACON_CCCD;
            }
#endif
            else
            {
                break;
            }

            if (g->callback)
            {
                ble_app_gfps_event_param_t entry;
                entry.event = event;
                entry.conidx = gfps->head.con_idx;
                entry.p.enabled = notify_enabled;
                g->callback(&entry);
            }
            break;
        }
        default:
        {
            break;
        }
    }

    return ret;
}

static void ble_gfps_send_notification(uint16_t connhdl, const uint8_t* character, const uint8_t *value, uint16_t len)
{
    ble_gfps_t *gfps = (ble_gfps_t *)ble_get_gfps(connhdl);
    if (gfps == NULL)
    {
        return;
    }

    gatts_send_notification(gap_conn_bf(gfps->head.con_idx), character, value, len);
}

void ble_app_gfps_init(app_ble_adv_activity_func func)
{
#if BLE_AUDIO_ENABLED
    bt_l2cap_create_port(PSM_SPSM_GFPS, ble_app_gfps_l2cap_callback);
    bt_l2cap_listen(PSM_SPSM_GFPS);
#endif

    app_ble_register_advertising(BLE_GFPS_ADV_HANDLE, func);

    gatts_cfg_t cfg = {0};
    cfg.svc_size = sizeof(ble_gfps_t);
    cfg.preferred_mtu = GFPS_PREFERRED_MTU;
    cfg.eatt_preferred = false;
    gatts_register_service(g_ble_gfps_attr_list, ARRAY_SIZE(g_ble_gfps_attr_list), ble_gfps_server_callback, &cfg);
}

#ifdef SPOT_ENABLED

#ifdef RTC_ENABLE
void spot_rtc_init(void)
{
    pmu_rtc_enable();
    uint32_t current_time = pmu_rtc_get();
    NV_FP_ACCOUNT_KEY_RECORD_T *nvrecord_fp_p = nv_record_get_fp_data_structure_info();

    if(nvrecord_fp_p != NULL)
    {
        uint32_t has_poweredoff_time = current_time - nvrecord_fp_p->poweroff_time;
        TRACE(1,"nvrecord_ble_p->poweroff_time is %u", nvrecord_fp_p->poweroff_time);
        TRACE(2,"current is %u, poweroff_time is %u", current_time, has_poweredoff_time);
        if(nvrecord_fp_p->poweroff_time != 0 && nv_record_fp_get_spot_adv_enable_value())
        {
            nv_record_fp_update_has_poweredoff_time(has_poweredoff_time);

            gfps_ble_spot_generate_EID_handler(nv_record_fp_get_eph_identity_key());
            nv_record_fp_update_spot_adv_data(gfps_ble_get_EID());
            gfps_ble_spot_generate_hashed_value();
            nv_record_fp_update_spot_hash_value(gfps_ble_get_hashed_value());
            bes_ble_gap_start_connectable_adv(BLE_FASTPAIR_SPOT_ADVERTISING_INTERVAL);
        }
     }
     else
     {
         TRACE(0,"error it is null");
     }
}
#endif

#ifdef RTC_CALENDAR
void spot_rtc_init(void)
{
    NV_FP_ACCOUNT_KEY_RECORD_T *nvrecord_fp_p = nv_record_get_fp_data_structure_info();
    struct RTC_CALENDAR_FORMAT_T rtc_cal_cfg = {
            .year = 2024,
            .month = 8,
            .day = 28,
            .week = 3,
            .hour = 23,
            .minute = 31,
            .second = 0,
    };
    pmu_rtc_calendar_set(&rtc_cal_cfg);

    struct RTC_CALENDAR_FORMAT_T alarm_cal = {0, };
    pmu_rtc_calendar_pwron_enable(true);
    pmu_rtc_calendar_get(&alarm_cal);

    uint32_t current_time;
    current_time = rtc_calendar_to_unix(&alarm_cal);
    TRACE(1, "current_time is %u", current_time);

    if(nvrecord_fp_p != NULL)
    {
        if(nvrecord_fp_p->poweroff_time != 0 && nv_record_fp_get_spot_adv_enable_value())
        {
            uint32_t has_poweredoff_time = current_time - nvrecord_fp_p->poweroff_time;
            TRACE(1,"nvrecord_ble_p->poweroff_time is %u", nvrecord_fp_p->poweroff_time);
            TRACE(2,"current is %u, poweroff_time is %u", current_time, has_poweredoff_time);
            nv_record_fp_update_has_poweredoff_time(has_poweredoff_time);

            gfps_ble_spot_generate_EID_handler(nv_record_fp_get_eph_identity_key());
            nv_record_fp_update_spot_adv_data(gfps_ble_get_EID());
            gfps_ble_spot_generate_hashed_value();
            nv_record_fp_update_spot_hash_value(gfps_ble_get_hashed_value());
            bes_ble_gap_start_connectable_adv(BLE_FASTPAIR_SPOT_ADVERTISING_INTERVAL);
         }
    }
    else
    {
        TRACE(0,"error it is null");
    }
}
#endif

void ble_app_spot_init(app_ble_adv_activity_func func)
{
#if defined(RTC_ENABLE) || defined(RTC_CALENDAR)
    spot_rtc_init();
#endif
    app_ble_register_advertising(BLE_SPOT_ADV_HANDLE, func);
}
#endif

void ble_app_gfps_send_keybase_pairing(uint8_t conidx, const uint8_t *data, uint32_t length)
{
    ble_gfps_t *gfps = ble_get_gfps_by_conidx(conidx);
    if (gfps == NULL)
    {
        CO_LOG_ERR_1(BT_STS_INVALID_GFPS, gfps);
        return;
    }
    ble_gfps_send_notification(gfps->head.connhdl, g_ble_gfps_keybased_pairing_character, data, (uint16_t)length);
}

void ble_app_gfps_send_passkey(uint8_t conidx, const uint8_t *data, uint32_t length)
{
    ble_gfps_t *gfps = ble_get_gfps_by_conidx(conidx);
    if (gfps == NULL)
    {
        CO_LOG_ERR_1(BT_STS_INVALID_GFPS, gfps);
        return;
    }

    ble_gfps_send_notification(gfps->head.connhdl, g_ble_gfps_passkey_character, data, (uint16_t)length);
}

void ble_app_gfps_send_naming_packet(uint8_t conidx, const uint8_t *data, uint32_t length)
{
    ble_gfps_t *gfps = ble_get_gfps_by_conidx(conidx);
    if (gfps == NULL)
    {
        CO_LOG_ERR_1(BT_STS_INVALID_GFPS, gfps);
        return;
    }

    ble_gfps_send_notification(gfps->head.connhdl, g_ble_gfps_additional_data_character, data, (uint16_t)length);
}

void ble_app_gfps_send_additional_passkey(uint8_t conidx, const uint8_t *data, uint32_t length)
{
    ble_gfps_t *gfps = ble_get_gfps_by_conidx(conidx);
    if (gfps == NULL)
    {
        CO_LOG_ERR_1(BT_STS_INVALID_GFPS, gfps);
        return;
    }

    ble_gfps_send_notification(gfps->head.connhdl, g_ble_gfps_additional_passkey_character, data, (uint16_t)length);
}

#ifdef SPOT_ENABLED
void ble_app_gfps_send_beacon_data(uint8_t conidx, const uint8_t *data, uint32_t length)
{
    ble_gfps_t *gfps = ble_get_gfps_by_conidx(conidx);
    if (gfps == NULL)
    {
        CO_LOG_ERR_1(BT_STS_INVALID_GFPS, gfps);
        return;
    }

    ble_gfps_send_notification(gfps->head.connhdl, g_ble_gfps_beacon_actions_character, data, (uint16_t)length);
}

#endif

#endif /* GFPS_ENABLED */
