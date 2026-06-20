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
#ifdef ANCS_ENABLED // Apple Notification Center Service Proxy
#include "gatt_service.h"

#define ANC_MAX_LEN 244
#define ANC_PREFERRED_MTU 512

typedef enum {
    ANC_CHAR_DATA_SOURCE,
    ANC_CHAR_NTFY_SOURCE,
    ANC_CHAR_CTRL_POINT,
    ANC_CHAR_MAX_NUM,
} anc_char_enum_t;

// ANCS Service Proxy - 67A846AD-DE3E-451B-A6D8-7B2899CA2370
#define anc_service_uuid_128_le     0x70,0x23,0xCA,0x99,0x28,0x7B,0xD8,0xA6,0x1B,0x45,0x3E,0xDE,0xAD,0x46,0xA8,0x67
#define anc_ready_char_uuid_128_le  0x42,0xC1,0x2D,0x7B,0xFC,0x67,0xED,0xBA,0xBB,0x45,0x84,0xA5,0x35,0xED,0x3E,0x75
#define anc_ntf_source_uuid_128_le  0xBD,0x1D,0xA2,0x99,0xE6,0x25,0x58,0x8C,0xD9,0x42,0x01,0x63,0x0D,0x12,0xBF,0x9F
#define anc_data_source_uuid_128_le 0xFB,0x7B,0x7C,0xCE,0x6A,0xB3,0x44,0xBE,0xB5,0x4B,0xD6,0x24,0xE9,0xC6,0xEA,0x22
#define anc_ctl_point_uuid_128_le   0xD9,0xD9,0xAA,0xFD,0xBD,0x9B,0x21,0x98,0xA8,0x49,0xE1,0x45,0xF3,0xD8,0xD1,0x69

typedef struct {
    gatt_svc_t head;
    uint8_t ready;
    uint8_t ready_notify_enabled;
    uint8_t notify_source_enabled;
    uint8_t data_source_enabled;
    bool peer_char_discovered[ANC_CHAR_MAX_NUM];
} ble_anc_t;

GATT_DECL_128_LE_PRI_SERVICE(g_ble_anc_service,
    anc_service_uuid_128_le);

/**************************************************
* ANCS Ready (Google specific)
*    - UUID: FBE87F6C-3F1A-44B6-B577-0BAC731F6E85
*    - provides the current state of ANCS (0 - not ready, 1 - ready)
*    - read/notify characteristic
**************************************************/

GATT_DECL_128_LE_CHAR(g_ble_anc_ready_character,
    anc_ready_char_uuid_128_le,
    GATT_RD_REQ|GATT_NTF_PROP,
    ATT_RD_ENC);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_anc_ready_cccd,
    ATT_WR_ENC);

/**************************************************
* ANCS Notification Source
*    - UUID: 9FBF120D-6301-42D9-8C58-25E699A21DBD
*    - Notifiable (Direct proxy of Apple's ANCS characteristic and matches
*its UUID and properties)
**************************************************/

GATT_DECL_128_LE_CHAR(g_ble_anc_notify_source_character,
    anc_ntf_source_uuid_128_le,
    GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_anc_notify_source_cccd,
    ATT_WR_ENC);

/**************************************************
 * ANCS Data Source
 *    - UUID: 22EAC6E9-24D6-4BB5-BE44-B36ACE7C7BFB
 *    - Notifiable (Direct proxy of Apple's ANCS characteristic and matches
 *its UUID and properties)
 **************************************************/

GATT_DECL_128_LE_CHAR(g_ble_anc_data_source_character,
    anc_data_source_uuid_128_le,
    GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_anc_data_source_cccd,
    ATT_WR_ENC);

/**************************************************
* ANCS Control Point
*    - UUID: 69D1D8F3-45E1-49A8-9821-9BBDFDAAD9D9
*    - Writeable with response (Direct proxy of Apple's ANCS characteristic
*and matches its UUID and properties)
**************************************************/

GATT_DECL_128_LE_CHAR(g_ble_anc_control_point_character,
    anc_ctl_point_uuid_128_le,
    GATT_WR_REQ,
    ATT_WR_ENC);

static const gatt_attribute_t g_ble_anc_attr_list[] = {
    /* Service */
    gatt_attribute(g_ble_anc_service),
    /* Characteristics */
    gatt_attribute(g_ble_anc_ready_character),
        gatt_attribute(g_ble_anc_ready_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_anc_notify_source_character),
        gatt_attribute(g_ble_anc_notify_source_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_anc_data_source_character),
        gatt_attribute(g_ble_anc_data_source_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_anc_control_point_character),
};

#if defined(ANCC_ENABLED)
void ancc_write_req(uint16_t connhdl, anc_char_enum_t char_enum, const uint8_t *value, uint16_t len);
#endif

static void ancs_proxy_deliver_write(uint16_t connhdl, anc_char_enum_t char_enum, const uint8_t *value, uint16_t len)
{
#if defined(ANCC_ENABLED)
    ancc_write_req(connhdl, char_enum, value, len);
#endif
}

static int ble_anc_server_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
{
    ble_anc_t *anc = (ble_anc_t *)svc;

    switch (event)
    {
        case GATT_SERV_EVENT_CHAR_READ:
        {
            gatt_server_char_read_t *p = param.char_read;
            if (p->character == g_ble_anc_ready_character)
            {
                uint8_t ready_value[2] = {CO_SPLIT_UINT16_LE(anc->ready)};
                gatts_write_read_rsp_data(p->ctx, ready_value, sizeof(ready_value));
                return true;
            }
            break;
        }
        case GATT_SERV_EVENT_CHAR_WRITE:
        {
            gatt_server_char_write_t *p = param.char_write;
            if (p->value_offset != 0 || p->value_len == 0 || p->value == NULL)
            {
                return false;
            }
            if (p->character == g_ble_anc_control_point_character)
            {
                ancs_proxy_deliver_write(svc->connhdl, ANC_CHAR_CTRL_POINT, p->value, p->value_len);
                return true;
            }
            break;
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
            if ((uint8_t *)p->desc_attr->attr_data == g_ble_anc_ready_cccd)
            {
                anc->ready_notify_enabled = notify_enabled;
            }
            else if ((uint8_t *)p->desc_attr->attr_data == g_ble_anc_data_source_cccd)
            {
                anc->data_source_enabled = notify_enabled;
                ancs_proxy_deliver_write(svc->connhdl, ANC_CHAR_DATA_SOURCE, p->value, p->value_len);
            }
            else if ((uint8_t *)p->desc_attr->attr_data == g_ble_anc_notify_source_cccd)
            {
                anc->notify_source_enabled = notify_enabled;
                ancs_proxy_deliver_write(svc->connhdl, ANC_CHAR_NTFY_SOURCE, p->value, p->value_len);
            }
            break;
        }
        case GATT_SERV_EVENT_DESC_READ:
        {
            gatt_server_desc_read_t *p = param.desc_read;
            uint16_t cccd_config = 0;
            if ((uint8_t *)p->desc_attr->attr_data == g_ble_anc_ready_cccd)
            {
                cccd_config = anc->ready_notify_enabled ? 0x0001 : 0;
            }
            else if ((uint8_t *)p->desc_attr->attr_data == g_ble_anc_data_source_cccd)
            {
                cccd_config = anc->data_source_enabled ? 0x0001 : 0;
            }
            else if ((uint8_t *)p->desc_attr->attr_data == g_ble_anc_notify_source_cccd)
            {
                cccd_config = anc->notify_source_enabled ? 0x0001 : 0;
            }

            cccd_config = co_host_to_uint16_le(cccd_config);
            gatts_write_read_rsp_data(p->ctx, (uint8_t *)&cccd_config, sizeof(uint16_t));
            return true;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

void ble_ancs_init(void)
{
    gatts_cfg_t cfg = {0};
    cfg.svc_size = sizeof(ble_anc_t);
    cfg.preferred_mtu = ANC_PREFERRED_MTU;
    cfg.eatt_preferred = false;

    gatts_register_service(g_ble_anc_attr_list, ARRAY_SIZE(g_ble_anc_attr_list), ble_anc_server_callback, &cfg);
}

static void ancs_send_ready_notification(ble_anc_t *svc)
{
    if (svc->ready_notify_enabled)
    {
        uint8_t ready_value[2] = {CO_SPLIT_UINT16_LE(svc->ready)};
        gatts_send_notification(gap_conn_bf(svc->head.con_idx), g_ble_anc_ready_character, ready_value, sizeof(ready_value));
    }
}

void ancs_proxy_set_ready_flag(uint16_t connhdl, bool peer_ds_discovered, bool peer_ns_discovered, bool peer_cp_discovered)
{
    ble_anc_t *anc = (ble_anc_t *)gatts_get_service(connhdl, g_ble_anc_service, 0);
    if (anc == NULL)
    {
        return;
    }

    anc->peer_char_discovered[ANC_CHAR_DATA_SOURCE] = peer_ds_discovered;
    anc->peer_char_discovered[ANC_CHAR_NTFY_SOURCE] = peer_ns_discovered;
    anc->peer_char_discovered[ANC_CHAR_CTRL_POINT] = peer_cp_discovered;

    anc->ready = (peer_ds_discovered && peer_ns_discovered && peer_cp_discovered);
    ancs_send_ready_notification(anc);
}

void ancs_proxy_send_notification(uint16_t connhdl, anc_char_enum_t char_enum, const uint8_t *value, uint16_t len)
{
    ble_anc_t *anc = (ble_anc_t *)gatts_get_service(connhdl, g_ble_anc_service, 0);
    if (anc == NULL)
    {
        return;
    }

    if (char_enum == ANC_CHAR_DATA_SOURCE)
    {
        gatts_send_notification(gap_conn_bf(anc->head.con_idx), g_ble_anc_data_source_character, value, len);
    }
    else if (char_enum == ANC_CHAR_NTFY_SOURCE)
    {
        gatts_send_notification(gap_conn_bf(anc->head.con_idx), g_ble_anc_notify_source_character, value, len);
    }
}

#endif /* ANCS_ENABLED */
