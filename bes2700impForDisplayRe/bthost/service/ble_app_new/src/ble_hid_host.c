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
#if defined(BLE_HID_HOST)
#include "gatt_service.h"

#define BLE_HID_HOST_MAX_INSTANCE 2
#define BLE_HID_HOST_MAX_REPORT 5

#define HOGP_BOOT_PROTOCOL_MODE     0x00
#define HOGP_REPORT_PROTOCOL_MODE   0x01

#define HOGP_INPUT_REPORT_TYPE      0x01 // send from hid device to hid host, hid host may GET_REPORT
#define HOGP_OUTPUT_REPORT_TYPE     0x02 // send from hid host to hid device, hid host SET_REPORT
#define HOGP_FEATURE_REPORT_TYPE    0x03 // no time-critical app-specific or init info, hid host may GET/SET_REPORT

static uint8_t hid_prf_id;

typedef enum {
    HID_CHAR_INFORMATION,
    HID_CHAR_CTRL_POINT,
    HID_CHAR_PROTO_MODE,
    HID_CHAR_REPORT_MAP,
    HID_CHAR_KEYBOARD_BOOT_INPUT,
    HID_CHAR_KEYBOARD_BOOT_OUTPUT,
    HID_CHAR_MOUSE_BOOT_INPUT,
    HID_CHAR_HID_REPORT,
    HID_CHAR_UUID_MAX,
} hid_character_enum_t;

typedef struct {
    gatt_peer_character_t *peer_char[HID_CHAR_UUID_MAX];
    gatt_rrcd_value_t report[BLE_HID_HOST_MAX_REPORT];
    uint8_t nb_report;
    bool discovered;
} hid_peer_service_t;

typedef struct {
    gatt_prf_t head;
    uint8_t peer_service_count;
    gatt_peer_service_t *peer_service;
    hid_peer_service_t service[BLE_HID_HOST_MAX_INSTANCE];
} hid_prf_t;

typedef struct {
    hid_character_enum_t char_enum;
    uint8_t report_idx;
    uint8_t hid_idx;
    uint8_t error_code;
} hid_character_t;

static void ble_hid_host_discover_complete(hid_prf_t *prf)
{
    TRACE(0, "%s %d hids_nb %d", __func__, __LINE__, prf->peer_service_count);
}

static void ble_hid_host_read_complete(hid_prf_t *prf, uint8_t hid_idx, const hid_character_t *c, const uint8_t *value, uint16_t len)
{
    TRACE(0, "%s %d status %d hid_idx %d char %d", __func__, __LINE__, c->error_code, hid_idx, c->char_enum);
    if (c->error_code)
    {
        return;
    }
    switch (c->char_enum)
    {
        case HID_CHAR_INFORMATION:
            break;
        case HID_CHAR_PROTO_MODE:
            break;
        case HID_CHAR_REPORT_MAP:
            break;
        case HID_CHAR_KEYBOARD_BOOT_INPUT:
            break;
        case HID_CHAR_KEYBOARD_BOOT_OUTPUT:
            break;
        case HID_CHAR_MOUSE_BOOT_INPUT:
            break;
        case HID_CHAR_HID_REPORT:
            break;
        default:
            break;
    }
}

static void ble_hid_host_write_complete(hid_prf_t *prf, uint8_t hid_idx, const hid_character_t *c)
{
    TRACE(0, "%s %d status %d hid_idx %d char %d", __func__, __LINE__, c->error_code, hid_idx, c->char_enum);
    if (c->error_code)
    {
        return;
    }
    switch (c->char_enum)
    {
        case HID_CHAR_CTRL_POINT:
            break;
        case HID_CHAR_PROTO_MODE:
            break;
        case HID_CHAR_KEYBOARD_BOOT_OUTPUT:
            break;
        case HID_CHAR_HID_REPORT:
            break;
        default:
            break;
    }
}

static void ble_hid_host_recv_report_ind(hid_prf_t *prf, uint8_t hid_idx, const hid_character_t *c, const uint8_t *value, uint16_t len)
{
    TRACE(0, "%s %d status %d hid_idx %d char %d", __func__, __LINE__, c->error_code, hid_idx, c->char_enum);
    if (c->error_code)
    {
        return;
    }
    switch (c->char_enum)
    {
        case HID_CHAR_KEYBOARD_BOOT_INPUT:
            break;
        case HID_CHAR_KEYBOARD_BOOT_OUTPUT:
            break;
        case HID_CHAR_MOUSE_BOOT_INPUT:
            break;
        case HID_CHAR_HID_REPORT:
            break;
        default:
            break;
    }
}

static hid_character_t ble_hid_get_character(hid_prf_t *prf, gatt_peer_character_t *peer_char)
{
    hid_character_t hid_char = {0, 0, ATT_ERROR_NOT_FOUND};
    hid_peer_service_t *s = NULL;
    gatt_peer_character_t *hid_report = NULL;
    gatt_peer_character_t *curr_report = NULL;
    int i = 0;
    int j = 0;
    int report_idx = 0;

    if (peer_char == NULL)
    {
        return hid_char;
    }

    for (i = 0; i < prf->peer_service_count; i += 1)
    {
        s = prf->service + i;
        for (j = 0; j < HID_CHAR_HID_REPORT; j += 1)
        {
            if (peer_char == s->peer_char[j])
            {
                hid_char.hid_idx = i;
                hid_char.char_enum = j;
                hid_char.error_code = ATT_ERROR_NO_ERROR;
                return hid_char;
            }
        }
        hid_report = s->peer_char[HID_CHAR_HID_REPORT];
        if (hid_report == NULL)
        {
            continue;
        }
        for (report_idx = 0; report_idx < s->nb_report; report_idx += 1)
        {
            curr_report = hid_report + report_idx;
            if (peer_char == curr_report)
            {
                hid_char.hid_idx = i;
                hid_char.char_enum = HID_CHAR_HID_REPORT;
                hid_char.report_idx = report_idx;
                hid_char.error_code = ATT_ERROR_NO_ERROR;
                return hid_char;
            }
        }
    }

    return hid_char;
}

bt_status_t ble_hid_host_start_discover(uint16_t connhdl)
{
    hid_prf_t *prf = NULL;

    prf = (hid_prf_t *)gattc_get_profile(hid_prf_id, connhdl);
    if (prf == NULL)
    {
        return BT_STS_FAILED;
    }

    gattc_discover_service(&prf->head, GATT_UUID_HID_SERVICE, NULL);
    return BT_STS_SUCCESS;
}

static int ble_hid_client_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param)
{
    hid_prf_t *hid = (hid_prf_t *)prf;
    hid_character_t hid_char;
    uint8_t nb_report = 0;
    int report_idx = 0;
    int i = 0;

    switch (event)
    {
        case GATT_PROF_EVENT_OPENED:
        {
            break;
        }
        case GATT_PROF_EVENT_CLOSED:
        {
            hid->peer_service = NULL;
            hid->peer_service_count = 0;
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
            hid->peer_service = s;
            hid->peer_service_count = p->count;
            if (hid->peer_service_count > BLE_HID_HOST_MAX_INSTANCE)
            {
                hid->peer_service_count = BLE_HID_HOST_MAX_INSTANCE;
            }
            uint16_t gap_chars[] = {
                    GATT_CHAR_UUID_HID_INFORMATION,
                    GATT_CHAR_UUID_HID_CTRL_POINT,
                    GATT_CHAR_UUID_HID_REPORT_MAP,
                    GATT_CHAR_UUID_HID_PROTOCOL_MODE,
                    GATT_CHAR_UUID_BOOT_KEYBOARD_INPUT_REPORT,
                    GATT_CHAR_UUID_BOOT_KEYBOARD_OUTPUT_REPORT,
                    GATT_CHAR_UUID_BOOT_MOUSE_INPUT_REPORT,
                    GATT_CHAR_UUID_HID_REPORT,
                };
            for (i = 0; i < hid->peer_service_count; i += 1)
            {
                gattc_discover_multi_characters(prf, s + i, gap_chars, sizeof(gap_chars)/sizeof(uint16_t));
            }
            break;
        }
        case GATT_PROF_EVENT_CHARACTER:
        {
            gatt_profile_character_t *p = param.character;
            gatt_peer_service_t *s = p->service;
            gatt_peer_character_t *c = p->character;
            bool not_discovered = false;
            for (i = 0; i < hid->peer_service_count; i += 1)
            {
                if (s != hid->peer_service + i)
                {
                    continue;
                }
                if (p->discover_cmpl)
                {
                    hid->service[i].discovered = true;
                }
                if (p->discover_idx >= HID_CHAR_UUID_MAX)
                {
                    CO_LOG_ERR_2(BT_STS_INVALID_HID_CHAR_INDEX, p->discover_idx, HID_CHAR_UUID_MAX);
                    break;
                }
                if (p->error_code != ATT_ERROR_NO_ERROR)
                {
                    hid->service[i].peer_char[p->discover_idx] = NULL; // this character not exist
                    break;
                }
                hid->service[i].peer_char[p->discover_idx] = c;
                if (p->discover_idx == HID_CHAR_KEYBOARD_BOOT_INPUT ||
                    p->discover_idx == HID_CHAR_MOUSE_BOOT_INPUT)
                {
                    gattc_write_cccd_descriptor(prf, c, true, false);
                }
                else if (p->discover_idx == HID_CHAR_HID_REPORT)
                {
                    nb_report = (uint8_t)p->count;
                    if (nb_report > BLE_HID_HOST_MAX_REPORT)
                    {
                        nb_report = BLE_HID_HOST_MAX_REPORT;
                    }
                    hid->service[i].nb_report = nb_report;
                    for (report_idx = 0; report_idx < nb_report; report_idx += 1)
                    {
                        gattc_read_rrcd_descriptor(prf, c + report_idx);
                    }
                }
                break;
            }
            for (i = 0; i < hid->peer_service_count; i += 1)
            {
                if (!hid->service[i].discovered)
                {
                    not_discovered = true;
                }
            }
            if (!not_discovered)
            {
                ble_hid_host_discover_complete(hid);
            }
            break;
        }
        case GATT_PROF_EVENT_CHAR_READ_RSP:
        {
            gatt_profile_char_read_rsp_t *p = param.char_read_rsp;
            hid_char = ble_hid_get_character(hid, p->character);
            if (p->error_code)
            {
                hid_char.error_code = p->error_code;
            }
            if (hid_char.error_code)
            {
                ble_hid_host_read_complete(hid, hid_char.hid_idx, &hid_char, NULL, 0);
            }
            else
            {
                ble_hid_host_read_complete(hid, hid_char.hid_idx, &hid_char, p->value, p->value_len);
            }
            break;
        }
        case GATT_PROF_EVENT_CHAR_WRITE_RSP:
        {
            gatt_profile_char_write_rsp_t *p = param.char_write_rsp;
            hid_char = ble_hid_get_character(hid, p->character);
            if (p->error_code)
            {
                hid_char.error_code = p->error_code;
            }
            ble_hid_host_write_complete(hid, hid_char.hid_idx, &hid_char);
            break;
        }
        case GATT_PROF_EVENT_DESC_READ_RSP:
        {
            gatt_profile_desc_read_rsp_t *p = param.desc_read_rsp;
            hid_char = ble_hid_get_character(hid, p->character);
            if (p->desc_uuid == GATT_DESC_UUID_REPORT_REFERENCE)
            {
                gatt_rrcd_value_t *value = (gatt_rrcd_value_t *)p->value;
                if (value->report_type == HOGP_INPUT_REPORT_TYPE)
                {
                    gattc_write_cccd_descriptor(prf, p->character, true, false);
                }
                if (hid_char.error_code == ATT_ERROR_NO_ERROR && hid_char.char_enum == HID_CHAR_HID_REPORT)
                {
                    hid->service[hid_char.hid_idx].report[hid_char.report_idx] = *value;
                }
                else
                {
                    CO_LOG_ERR_2(BT_STS_INVALID_STATUS, hid_char.error_code, hid_char.char_enum);
                }
            }
            break;
        }
        case GATT_PROF_EVENT_DESC_WRITE_RSP:
        {
            break;
        }
        case GATT_PROF_EVENT_NOTIFY:
        {
            gatt_profile_recv_notify_t *p = param.notify;
            if (p->service->service_uuid == GATT_UUID_HID_SERVICE)
            {
                hid_char = ble_hid_get_character(hid, p->character);
                ble_hid_host_recv_report_ind(hid, hid_char.hid_idx, &hid_char, p->value, p->value_len);
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

void ble_hid_host_init(void)
{
    gattc_cfg_t prf_cfg = {0};
    prf_cfg.prf_size = sizeof(hid_prf_t);
    hid_prf_id = gattc_register_profile(ble_hid_client_callback, &prf_cfg);
}

bt_status_t ble_hid_host_read_req(uint16_t connhdl, uint8_t hid_idx, const hid_character_t *hid_char)
{
    hid_prf_t *prf = NULL;

    prf = (hid_prf_t *)gattc_get_profile(hid_prf_id, connhdl);
    if (prf == NULL || hid_idx >= prf->peer_service_count)
    {
        return BT_STS_FAILED;
    }

    switch (hid_char->char_enum)
    {
        case HID_CHAR_INFORMATION:
            gattc_read_character_by_uuid(&prf->head, prf->peer_service + hid_idx, GATT_CHAR_UUID_HID_INFORMATION, NULL);
            break;
        case HID_CHAR_PROTO_MODE:
            gattc_read_character_by_uuid(&prf->head, prf->peer_service + hid_idx, GATT_CHAR_UUID_HID_PROTOCOL_MODE, NULL);
            break;
        case HID_CHAR_REPORT_MAP:
            gattc_read_character_by_uuid(&prf->head, prf->peer_service + hid_idx, GATT_CHAR_UUID_HID_REPORT_MAP, NULL);
            break;
        case HID_CHAR_KEYBOARD_BOOT_INPUT:
            gattc_read_character_by_uuid(&prf->head, prf->peer_service + hid_idx, GATT_CHAR_UUID_BOOT_KEYBOARD_INPUT_REPORT, NULL);
            break;
        case HID_CHAR_KEYBOARD_BOOT_OUTPUT:
            gattc_read_character_by_uuid(&prf->head, prf->peer_service + hid_idx, GATT_CHAR_UUID_BOOT_KEYBOARD_OUTPUT_REPORT, NULL);
            break;
        case HID_CHAR_MOUSE_BOOT_INPUT:
            gattc_read_character_by_uuid(&prf->head, prf->peer_service + hid_idx, GATT_CHAR_UUID_BOOT_MOUSE_INPUT_REPORT, NULL);
            break;
        case HID_CHAR_HID_REPORT:
            if (prf->service[hid_idx].peer_char[HID_CHAR_HID_REPORT] == NULL || hid_char->report_idx >= prf->service[hid_idx].nb_report)
            {
                return BT_STS_FAILED;
            }
            gattc_read_character_value(&prf->head, prf->service[hid_idx].peer_char[HID_CHAR_HID_REPORT] + hid_char->report_idx);
            break;
        default:
            break;
    }

    return BT_STS_SUCCESS;
}

bt_status_t ble_hid_host_write_req(uint16_t connhdl, uint8_t hid_idx, const hid_character_t *hid_char, const uint8_t *value, uint16_t len)
{
    hid_prf_t *prf = NULL;
    gatt_peer_character_t *peer_char = NULL;

    prf = (hid_prf_t *)gattc_get_profile(hid_prf_id, connhdl);
    if (prf == NULL || hid_idx >= prf->peer_service_count)
    {
        return BT_STS_FAILED;
    }

    if (hid_char->char_enum >= HID_CHAR_UUID_MAX || prf->service[hid_idx].peer_char[hid_char->char_enum] == NULL)
    {
        return BT_STS_FAILED;
    }

    peer_char = prf->service[hid_idx].peer_char[hid_char->char_enum];

    switch (hid_char->char_enum)
    {
        case HID_CHAR_CTRL_POINT:
        {
            uint8_t exit_suspend = value[0];
            gattc_write_character_value(&prf->head, peer_char, &exit_suspend, 1);
            break;
        }
        case HID_CHAR_PROTO_MODE:
        {
            uint8_t proto_mode = value[0];
            gattc_write_character_value(&prf->head, peer_char, &proto_mode, 1);
            break;
        }
        case HID_CHAR_KEYBOARD_BOOT_OUTPUT:
        {
            hid_keyboard_boot_output_report_t *report = (hid_keyboard_boot_output_report_t *)value;
            gattc_write_character_value(&prf->head, peer_char, (uint8_t *)report, sizeof(hid_keyboard_boot_output_report_t));
            break;
        }
        case HID_CHAR_HID_REPORT:
        {
            if (hid_char->report_idx >= prf->service[hid_idx].nb_report)
            {
                return BT_STS_FAILED;
            }
            if (prf->service[hid_idx].report[hid_char->report_idx].report_type == HOGP_INPUT_REPORT_TYPE)
            {
                return BT_STS_FAILED;
            }
            gattc_write_character_value(&prf->head, peer_char + hid_char->report_idx, value, len);
            break;
        }
        default:
            break;
    }

    return BT_STS_SUCCESS;
}

#endif /* BLE_HID_HOST */
