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
#if defined(BLE_DIP_ENABLE)
#include "gatt_service.h"
#include "app_ble.h"

#define MANUFACTURER_NAME "DEMO"
#define FIRMWARE_REVISION_STRING "0000"

#define PNP_VID_SOURCE  0x01
#define PNP_VENDOR_ID   0x02B0
#define PNP_PRODUCT_ID  0x0000
#define PNP_PRODUCT_VER 0x001F

GATT_DECL_PRI_SERVICE(g_ble_dip_service, GATT_UUID_DIP_SERVICE);

GATT_DECL_CHAR(g_ble_dip_manufacturer_name,
    GATT_CHAR_UUID_MANUFACTURER_NAME_STRING,
    GATT_RD_REQ,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_dip_firmware_revision,
    GATT_CHAR_UUID_FW_REVISION_STRING,
    GATT_RD_REQ,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_dip_pnp_info,
    GATT_CHAR_UUID_PNP_ID,
    GATT_RD_REQ,
    ATT_SEC_NONE);

static const gatt_attribute_t g_ble_dip_service_attr_list[] = {
    /* Service */
    gatt_attribute(g_ble_dip_service),
    /* Characteristics */
    gatt_attribute(g_ble_dip_manufacturer_name),
    gatt_attribute(g_ble_dip_firmware_revision),
    gatt_attribute(g_ble_dip_pnp_info),
};

static int app_ble_dip_server_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
{
    switch (event)
    {
        case GATT_SERV_EVENT_CHAR_READ:
        {
            gatt_server_char_read_t *p = param.char_read;
            const uint8_t *c = p->character;
            if (c == g_ble_dip_manufacturer_name)
            {
                char manufacturer_name[] = {MANUFACTURER_NAME};
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)manufacturer_name, strlen(manufacturer_name));
                return true;
            }
            else if (c == g_ble_dip_firmware_revision)
            {
                char fw_revision[] = {FIRMWARE_REVISION_STRING};
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)fw_revision, strlen(fw_revision));
                return true;
            }
            else if (c == g_ble_dip_pnp_info)
            {
                app_ble_dip_pnp_info_t pnp_info;
                pnp_info.vendor_id_source = PNP_VID_SOURCE;
                pnp_info.vendor_id = co_host_to_uint16_le(PNP_VENDOR_ID);
                pnp_info.product_id = co_host_to_uint16_le(PNP_PRODUCT_ID);
                pnp_info.product_version = co_host_to_uint16_le(PNP_PRODUCT_VER);
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)&pnp_info, sizeof(pnp_info));
                return true;
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

/**
 * dip client
 *
*/

typedef struct {
    gatt_prf_t head;
    gatt_peer_character_t *manufacturer_name;
    gatt_peer_character_t *model_number_string;
    gatt_peer_character_t *serial_number_string;
    gatt_peer_character_t *hw_revision_string;
    gatt_peer_character_t *fw_revision_string;
    gatt_peer_character_t *sw_revision_string;
    gatt_peer_character_t *system_id;
    gatt_peer_character_t *ieee_cdl;
    gatt_peer_character_t *pnp_info;
    gatt_peer_character_t *medical_udi;
} app_ble_dip_prf_conn_t;

static uint8_t g_dip_prf_id;
static app_ble_dip_client_callback_t g_dip_prf_cb;

bt_status_t app_ble_dip_register_callback(app_ble_dip_client_callback_t callback)
{
    g_dip_prf_cb = callback;
    return BT_STS_SUCCESS;
}

static void app_ble_dip_client_report_info(app_ble_dip_prf_conn_t *conn, app_ble_dip_info_type_t type, const uint8_t *value, uint16_t len)
{
    app_ble_dip_client_callback_param_t param = {0};
    app_ble_dip_pnp_info_t *pnp_info = (app_ble_dip_pnp_info_t *)value;

    param.type = type;
    param.con_idx = conn->head.con_idx;
    param.connhdl = conn->head.connhdl;
    param.value = (char *)value;
    param.value_len = len;

    if (value == NULL || len == 0)
    {
        return;
    }

    if (type == BLE_DIP_PNP_INFO)
    {
        param.pnp_info.vendor_id_source = pnp_info->vendor_id_source;
        param.pnp_info.vendor_id = co_uint16_le_to_host(pnp_info->vendor_id);
        param.pnp_info.product_id = co_uint16_le_to_host(pnp_info->product_id);
        param.pnp_info.product_version = co_uint16_le_to_host(pnp_info->product_version);
    }

    if (g_dip_prf_cb)
    {
        g_dip_prf_cb(&param);
    }
}

static void app_ble_dip_clear_prf_conn(app_ble_dip_prf_conn_t *conn)
{
    conn->manufacturer_name = NULL;
    conn->model_number_string = NULL;
    conn->serial_number_string = NULL;
    conn->hw_revision_string = NULL;
    conn->fw_revision_string = NULL;
    conn->sw_revision_string = NULL;
    conn->system_id = NULL;
    conn->ieee_cdl = NULL;
    conn->pnp_info = NULL;
    conn->medical_udi = NULL;
}

bt_status_t app_ble_discover_dip_service(uint16_t connhdl)
{
    app_ble_dip_prf_conn_t *conn = NULL;

    conn = (app_ble_dip_prf_conn_t *)gattc_get_profile(g_dip_prf_id, connhdl);
    if (conn == NULL)
    {
        TRACE(0, "dip invalid conn: %d %04x", g_dip_prf_id, connhdl);
        return BT_STS_INVALID_CONN_HANDLE;
    }

    TRACE(0, "dip start discover: %d %04x", g_dip_prf_id, connhdl);

    app_ble_dip_clear_prf_conn(conn);

    gattc_discover_service(&conn->head, GATT_UUID_DIP_SERVICE, NULL);
    return BT_STS_SUCCESS;
}

bt_status_t app_ble_dip_read_peer_info(uint16_t connhdl, app_ble_dip_info_type_t type)
{
    app_ble_dip_prf_conn_t *conn = NULL;
    gatt_peer_character_t *peer_character = NULL;

    conn = (app_ble_dip_prf_conn_t *)gattc_get_profile(g_dip_prf_id, connhdl);
    if (conn == NULL || type > BLE_DIP_MEDICAL_UDI)
    {
        TRACE(0, "dip invalid conn: %d %04x %d", g_dip_prf_id, connhdl, type);
        return BT_STS_INVALID_CONN_HANDLE;
    }

    TRACE(0, "dip read info: %d %04x type %d", g_dip_prf_id, connhdl, type);

    switch (type)
    {
        case BLE_DIP_MANUFACTURER_NAME:
            peer_character = conn->manufacturer_name;
            break;
        case BLE_DIP_MODEL_NUMBER_STRING:
            peer_character = conn->model_number_string;
            break;
        case BLE_DIP_SERIAL_NUMBER_STRING:
            peer_character = conn->serial_number_string;
            break;
        case BLE_DIP_HW_REVISION_STRING:
            peer_character = conn->hw_revision_string;
            break;
        case BLE_DIP_FW_REVISION_STRING:
            peer_character = conn->fw_revision_string;
            break;
        case BLE_DIP_SW_REVISION_STRING:
            peer_character = conn->sw_revision_string;
            break;
        case BLE_DIP_SYSTEM_ID:
            peer_character = conn->system_id;
            break;
        case BLE_DIP_IEEE_CDL:
            peer_character = conn->ieee_cdl;
            break;
        case BLE_DIP_PNP_INFO:
            peer_character = conn->pnp_info;
            break;
        case BLE_DIP_MEDICAL_UDI:
            peer_character = conn->medical_udi;
            break;
        default:
            break;
    }

    if (peer_character == NULL)
    {
        TRACE(0, "dip character not found: %d %04x %d", g_dip_prf_id, connhdl, type);
        return BT_STS_CHARACTER_NOT_FOUND;
    }

    return gattc_read_character_value(&conn->head, peer_character);
}

static int app_ble_dip_client_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param)
{
    app_ble_dip_prf_conn_t *conn = (app_ble_dip_prf_conn_t *)prf;

    switch (event)
    {
        case GATT_PROF_EVENT_OPENED:
        case GATT_PROF_EVENT_SERVICE_CHANGED:
        {
            app_ble_dip_clear_prf_conn(conn);
            break;
        }
        case GATT_PROF_EVENT_CLOSED:
        {
            app_ble_dip_clear_prf_conn(conn);
            break;
        }
        case GATT_PROF_EVENT_SERVICE:
        {
            gatt_profile_service_t *p = (gatt_profile_service_t *)param.service;
            if (p->error_code || p->count == 0)
            {
                TRACE(0, "dip service not found: %02x %d", p->error_code, p->count);
                break;
            }
            uint16_t read_chars[] = {
                GATT_CHAR_UUID_PNP_ID,
                GATT_CHAR_UUID_MANUFACTURER_NAME_STRING,
                GATT_CHAR_UUID_MODEL_NUMBER_STRING,
                GATT_CHAR_UUID_SERIAL_NUMBER_STRING,
                GATT_CHAR_UUID_HW_REVISION_STRING,
                GATT_CHAR_UUID_FW_REVISION_STRING,
                GATT_CHAR_UUID_SW_REVISION_STRING,
                GATT_CHAR_UUID_SYSTEM_ID,
                GATT_CHAR_UUID_IEEE_CERT_DATA_LIST,
                GATT_CHAR_UUID_UDI_FOR_MEDICAL_DEVICES,
            };
            gattc_discover_multi_characters(prf, p->service, read_chars, ARRAY_SIZE(read_chars));
            break;
        }
        case GATT_PROF_EVENT_CHARACTER:
        {
            gatt_profile_character_t *p = (gatt_profile_character_t *)param.character;
            if (p->error_code || p->count == 0)
            {
                TRACE(0, "dip char not found: %02x %04x", p->error_code, p->char_uuid);
                break;
            }
            gattc_read_character_value(&conn->head, p->character);
            if (p->char_uuid == GATT_CHAR_UUID_PNP_ID)
            {
                conn->pnp_info = p->character;
            }
            else if (p->char_uuid == GATT_CHAR_UUID_MANUFACTURER_NAME_STRING)
            {
                conn->manufacturer_name = p->character;
            }
            else if (p->char_uuid == GATT_CHAR_UUID_MODEL_NUMBER_STRING)
            {
                conn->model_number_string = p->character;
            }
            else if (p->char_uuid == GATT_CHAR_UUID_SERIAL_NUMBER_STRING)
            {
                conn->serial_number_string = p->character;
            }
            else if (p->char_uuid == GATT_CHAR_UUID_HW_REVISION_STRING)
            {
                conn->hw_revision_string = p->character;
            }
            else if (p->char_uuid == GATT_CHAR_UUID_FW_REVISION_STRING)
            {
                conn->fw_revision_string = p->character;
            }
            else if (p->char_uuid == GATT_CHAR_UUID_SW_REVISION_STRING)
            {
                conn->sw_revision_string = p->character;
            }
            else if (p->char_uuid == GATT_CHAR_UUID_SYSTEM_ID)
            {
                conn->system_id = p->character;
            }
            else if (p->char_uuid == GATT_CHAR_UUID_IEEE_CERT_DATA_LIST)
            {
                conn->ieee_cdl = p->character;
            }
            else if (p->char_uuid == GATT_CHAR_UUID_UDI_FOR_MEDICAL_DEVICES)
            {
                conn->medical_udi = p->character;
            }
            break;
        }
        case GATT_PROF_EVENT_CHAR_READ_RSP:
        {
            app_ble_dip_info_type_t type = BLE_DIP_INVALID_TYPE;
            gatt_profile_char_read_rsp_t *p = (gatt_profile_char_read_rsp_t *)param.char_read_rsp;
            if (p->error_code || p->value_len == 0)
            {
                TRACE(0, "dip read fail: %02x %d", p->error_code, p->value_len);
                break;
            }
            if (p->character == conn->pnp_info)
            {
                TRACE(0, "peer pnp info %d: %08x %08x", p->value_len, CO_COMBINE_UINT32_BE(p->value), CO_COMBINE_UINT32_BE(p->value+4));
                type = BLE_DIP_PNP_INFO;
            }
            else if (p->character == conn->manufacturer_name)
            {
                TRACE(0, "peer manufacturer name %d: %s", p->value_len, p->value);
                type = BLE_DIP_MANUFACTURER_NAME;
            }
            else if (p->character == conn->model_number_string)
            {
                TRACE(0, "peer model number %d: %s", p->value_len, p->value);
                type = BLE_DIP_MODEL_NUMBER_STRING;
            }
            else if (p->character == conn->serial_number_string)
            {
                TRACE(0, "peer serial number %d: %s", p->value_len, p->value);
                type = BLE_DIP_SERIAL_NUMBER_STRING;
            }
            else if (p->character == conn->hw_revision_string)
            {
                TRACE(0, "peer hw reversion %d: %s", p->value_len, p->value);
                type = BLE_DIP_HW_REVISION_STRING;
            }
            else if (p->character == conn->fw_revision_string)
            {
                TRACE(0, "peer fw reversion %d: %s", p->value_len, p->value);
                type = BLE_DIP_FW_REVISION_STRING;
            }
            else if (p->character == conn->sw_revision_string)
            {
                TRACE(0, "peer sw reversion %d: %s", p->value_len, p->value);
                type = BLE_DIP_SW_REVISION_STRING;
            }
            else if (p->character == conn->system_id)
            {
                TRACE(0, "peer system id %d: %08x", p->value_len, CO_COMBINE_UINT32_BE(p->value));
                type = BLE_DIP_SYSTEM_ID;
            }
            else if (p->character == conn->ieee_cdl)
            {
                TRACE(0, "peer ieee cdl %d: %08x", p->value_len, CO_COMBINE_UINT32_BE(p->value));
                type = BLE_DIP_IEEE_CDL;
            }
            else if (p->character == conn->medical_udi)
            {
                TRACE(0, "peer medical udi %d: %08x", p->value_len, CO_COMBINE_UINT32_BE(p->value));
                type = BLE_DIP_MEDICAL_UDI;
            }
            if (type != BLE_DIP_INVALID_TYPE)
            {
                app_ble_dip_client_report_info(conn, type, p->value, p->value_len);
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

void ble_dip_init(void)
{
    gatts_register_service(g_ble_dip_service_attr_list, ARRAY_SIZE(g_ble_dip_service_attr_list), app_ble_dip_server_callback, NULL);

    gattc_cfg_t prf_cfg = {0};
    prf_cfg.prf_size = sizeof(app_ble_dip_prf_conn_t);
    g_dip_prf_id = gattc_register_profile(app_ble_dip_client_callback, &prf_cfg);
}

#endif /* BLE_DIP_ENABLE */
