/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#if defined(BLE_BATT_ENABLE)
#include "gatt_service.h"
#include "app_ble.h"
#include "nvrecord_env.h"

#define BLE_BATTERY_INSTANCE_NUM (2)

GATT_DECL_PRI_SERVICE(g_ble_batt_service, GATT_UUID_BAT_SERVICE);

GATT_DECL_CHAR(g_ble_battery_level_character,
    GATT_CHAR_UUID_BATT_LEVEL,
    GATT_RD_REQ|GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_battery_level_cccd,
    ATT_SEC_NONE);

GATT_DECL_CPFD_DESCRIPTOR(g_ble_battery_level_cpfd);

static const gatt_attribute_t g_ble_batt_service_attr_list[] = {
    /* Service */
    gatt_attribute(g_ble_batt_service),
    /* Characteristic */
    gatt_attribute(g_ble_battery_level_character),
        /* Descriptor */
        gatt_attribute(g_ble_battery_level_cccd),
        gatt_attribute(g_ble_battery_level_cpfd),
};

typedef struct {
    uint8_t battery_level;
    gatt_cpfd_data_t cpfd_data;
} app_ble_battery_inst_t;

static app_ble_battery_inst_t g_ble_battery_instance[BLE_BATTERY_INSTANCE_NUM];

static int app_ble_batt_server_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
{
    app_ble_battery_inst_t *inst = NULL;
    int i = 0;

    for (; i < BLE_BATTERY_INSTANCE_NUM; i += 1)
    {
        if (svc->service_inst_id == i)
        {
            inst = g_ble_battery_instance + i;
            break;
        }
    }

    if (inst == NULL)
    {
        return 0;
    }

    switch (event)
    {
        case GATT_SERV_EVENT_CHAR_READ:
        {
            gatt_server_char_read_t *p = param.char_read;
            if (p->character == g_ble_battery_level_character)
            {
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)&inst->battery_level, sizeof(uint8_t));
                return true;
            }
            break;
        }
        case GATT_SERV_EVENT_DESC_READ:
        {
            gatt_server_desc_read_t *p = param.desc_read;
            if ((uint8_t *)p->desc_attr->attr_data == g_ble_battery_level_cpfd)
            {
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)&inst->cpfd_data, sizeof(gatt_cpfd_data_t));
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

bt_status_t app_ble_report_battery_level(uint8_t instance, uint8_t battery_level)
{
    gatt_char_notify_t notify = {NULL};
    app_ble_battery_inst_t *inst = NULL;

    if (instance >= BLE_BATTERY_INSTANCE_NUM)
    {
        return BT_STS_INVALID_PARAMS;
    }

    notify.service = g_ble_batt_service;
    notify.character = g_ble_battery_level_character;
    notify.service_inst_id = instance;

    if (battery_level > 100)
    {
        battery_level = 100;
    }

    inst = g_ble_battery_instance + instance;
    inst->battery_level = battery_level;

    return gatts_send_value_notification(GAP_ALL_CONNS, &notify, &battery_level, sizeof(uint8_t));
}

/**
 * battery client
 *
*/

typedef struct {
    uint8_t battery_level;
    uint8_t name_space;
    uint16_t description;
    gatt_peer_service_t *peer_service;
    gatt_peer_character_t *peer_battery_level;
} app_ble_peer_batt_service_t;

typedef struct {
    gatt_prf_t head;
    uint16_t service_count;
    app_ble_peer_batt_service_t peer_service[BLE_BATTERY_INSTANCE_NUM];
} app_ble_batt_prf_conn_t;

static uint8_t g_batt_prf_id;
static app_ble_recv_battery_callback_t g_batt_prf_cb;

bt_status_t app_ble_register_battery_callback(app_ble_recv_battery_callback_t callback)
{
    g_batt_prf_cb = callback;
    return BT_STS_SUCCESS;
}

static void app_ble_batt_client_report_battery(app_ble_batt_prf_conn_t *conn, uint8_t instance, uint8_t battery_level)
{
    app_ble_battery_param_t param = {0};
    app_ble_peer_batt_service_t *s = conn->peer_service + instance;

    s->battery_level = battery_level;

    param.instance = instance;
    param.con_idx = conn->head.con_idx;
    param.connhdl = conn->head.connhdl;
    param.battery_level = battery_level;
    param.name_space = s->name_space;
    param.description = s->description;

    if (g_batt_prf_cb)
    {
        g_batt_prf_cb(&param);
    }
}

static void app_ble_batt_clear_prf_conn(app_ble_batt_prf_conn_t *conn)
{
    conn->service_count = 0;
    memset(conn->peer_service, 0, sizeof(conn->peer_service));
}

bt_status_t app_ble_discover_batt_service(uint16_t connhdl)
{
    app_ble_batt_prf_conn_t *conn = NULL;

    conn = (app_ble_batt_prf_conn_t *)gattc_get_profile(g_batt_prf_id, connhdl);
    if (conn == NULL)
    {
        TRACE(0, "batt invalid connhdl: %d %04x", g_batt_prf_id, connhdl);
        return BT_STS_INVALID_CONN_HANDLE;
    }

    TRACE(0, "batt start discover: %d %04x", g_batt_prf_id, connhdl);

    app_ble_batt_clear_prf_conn(conn);

    gattc_discover_service(&conn->head, GATT_UUID_BAT_SERVICE, NULL);
    return BT_STS_SUCCESS;
}

bt_status_t app_ble_read_peer_battery_level(uint16_t connhdl, uint8_t instance)
{
    app_ble_batt_prf_conn_t *conn = NULL;
    gatt_peer_character_t *character = NULL;

    if (instance >= BLE_BATTERY_INSTANCE_NUM)
    {
        return BT_STS_INVALID_PARAMS;
    }

    conn = (app_ble_batt_prf_conn_t *)gattc_get_profile(g_batt_prf_id, connhdl);
    if (conn == NULL || (character = conn->peer_service[instance].peer_battery_level) == NULL)
    {
        TRACE(0, "batt invalid conn: %d %04x %p", g_batt_prf_id, connhdl, character);
        return BT_STS_INVALID_CONN_HANDLE;
    }

    TRACE(0, "batt read peer battery: %d %04x %d", g_batt_prf_id, connhdl, instance);

    return gattc_read_character_value(&conn->head, character);
}

static int app_ble_batt_client_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param)
{
    app_ble_batt_prf_conn_t *conn = (app_ble_batt_prf_conn_t *)prf;
    app_ble_peer_batt_service_t *s = NULL;
    int i = 0;

    switch (event)
    {
        case GATT_PROF_EVENT_OPENED:
        case GATT_PROF_EVENT_SERVICE_CHANGED:
        {
            app_ble_batt_clear_prf_conn(conn);
            break;
        }
        case GATT_PROF_EVENT_CLOSED:
        {
            app_ble_batt_clear_prf_conn(conn);
            break;
        }
        case GATT_PROF_EVENT_SERVICE:
        {
            gatt_profile_service_t *p = (gatt_profile_service_t *)param.service;
            gatt_peer_service_t *peer_service = NULL;
            uint16_t service_count = 0;
            if (p->error_code || p->count == 0)
            {
                TRACE(0, "service not found: %02x %d", p->error_code, p->count);
                break;
            }
            service_count = (p->count <= BLE_BATTERY_INSTANCE_NUM) ? p->count : BLE_BATTERY_INSTANCE_NUM;
            conn->service_count = service_count;
            for (; i < service_count; i += 1)
            {
                peer_service = p->service + i;
                s = conn->peer_service + i;
                memset(s, 0, sizeof(app_ble_peer_batt_service_t));
                s->peer_service = p->service + i;
                gattc_discover_character(&conn->head, peer_service, GATT_CHAR_UUID_BATT_LEVEL, NULL);
            }
            break;
        }
        case GATT_PROF_EVENT_CHARACTER:
        {
            gatt_profile_character_t *p = (gatt_profile_character_t *)param.character;
            gatt_peer_character_t *peer_character = p->character;
            if (p->error_code || p->count == 0)
            {
                TRACE(0, "character not found: %02x %d", p->error_code, p->count);
                break;
            }
            for (; i < conn->service_count; i += 1)
            {
                s = conn->peer_service + i;
                if (s->peer_service == p->service)
                {
                    s->peer_battery_level = peer_character;
                    if (peer_character->char_prop & GATT_NTF_PROP)
                    {
                        gattc_write_cccd_descriptor(prf, peer_character, true, false);
                    }
                    gattc_read_cpfd_descriptor(prf, peer_character);
                    app_ble_read_peer_battery_level(prf->connhdl, i);
                    break;
                }
            }
            break;
        }
        case GATT_PROF_EVENT_CHAR_READ_RSP:
        {
            gatt_profile_char_read_rsp_t *p = (gatt_profile_char_read_rsp_t *)param.char_read_rsp;
            if (p->error_code || p->value_len == 0)
            {
                TRACE(0, "read failed: %02x %d", p->error_code, p->value_len);
                break;
            }
            for (; i < conn->service_count; i += 1)
            {
                s = conn->peer_service + i;
                if (s->peer_battery_level == p->character)
                {
                    app_ble_batt_client_report_battery(conn, i, p->value[0]);
                    break;
                }
            }
            break;
        }
        case GATT_PROF_EVENT_DESC_READ_RSP:
        {
            gatt_profile_desc_read_rsp_t *p = (gatt_profile_desc_read_rsp_t *)param.desc_read_rsp;
            gatt_cpfd_data_t *cpfd_data = (gatt_cpfd_data_t *)p->value;
            if (p->error_code || p->value_len != sizeof(gatt_cpfd_data_t))
            {
                TRACE(0, "read failed: %02x %d", p->error_code, p->value_len);
                break;
            }
            for (; i < conn->service_count; i += 1)
            {
                s = conn->peer_service + i;
                if (s->peer_battery_level == p->character)
                {
                    s->name_space = cpfd_data->name_space;
                    s->description = co_uint16_le_to_host(cpfd_data->description);
                    break;
                }
            }
            break;
        }
        case GATT_PROF_EVENT_NOTIFY:
        {
            gatt_profile_recv_notify_t *p = (gatt_profile_recv_notify_t *)param.notify;
            if (p->service->service_uuid != GATT_UUID_BAT_SERVICE || p->value_len == 0)
            {
                break;
            }
            for (; i < conn->service_count; i += 1)
            {
                s = conn->peer_service + i;
                if (s->peer_battery_level == p->character)
                {
                    app_ble_batt_client_report_battery(conn, i, p->value[0]);
                    break;
                }
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

void ble_batt_init(void)
{
    app_ble_battery_inst_t *inst = NULL;
    struct nvrecord_env_t *nvrecord_env = NULL;
    gatts_cfg_t cfg = {0};
    int i = 0;

    nv_record_env_get(&nvrecord_env);

    TRACE(0, "batt init: mode %d instance %d", nvrecord_env->ibrt_mode.mode, BLE_BATTERY_INSTANCE_NUM);

    for (; i < BLE_BATTERY_INSTANCE_NUM; i += 1)
    {
        cfg.service_inst_id = i;
        inst = g_ble_battery_instance + i;
        inst->battery_level = 100;
        memset(&inst->cpfd_data, 0, sizeof(gatt_cpfd_data_t));
        inst->cpfd_data.name_space = GATT_NS_BLUETOOTH_SIG;
        if (i == 0)
        {
            inst->cpfd_data.description = co_host_to_uint16_le(GATT_SIG_NS_EXTERNAL);
        }
        else if (i == 1)
        {
            // mode 0: master, right;   mode 1: slave, left
            if (nvrecord_env->ibrt_mode.mode == 0)
            {
                // Audio Location Definitions, side right
                inst->cpfd_data.description = co_host_to_uint16_le(0x0800);
            }
            else if (nvrecord_env->ibrt_mode.mode == 1)
            {
                // Audio Location Definitions, side left
                inst->cpfd_data.description = co_host_to_uint16_le(0x0400);
            }
        }
        gatts_register_service(g_ble_batt_service_attr_list, ARRAY_SIZE(g_ble_batt_service_attr_list), app_ble_batt_server_callback, &cfg);
    }

    gattc_cfg_t prf_cfg = {0};
    prf_cfg.prf_size = sizeof(app_ble_batt_prf_conn_t);
    g_batt_prf_id = gattc_register_profile(app_ble_batt_client_callback, &prf_cfg);
}

#endif /* BLE_BATT_ENABLE */
