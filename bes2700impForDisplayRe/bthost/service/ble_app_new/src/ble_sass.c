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
#ifdef CFG_APP_SAS_SERVER
#include "gatt_service.h"

#ifdef ANC_APP
#include "app_anc.h"
#endif

#define SAS_PREFERRED_MTU 512
#define INVALID_CONNECTION_INDEX 0xFF
#define AMBIENT_SERVICE_SWITCH_BIT_MASK 0x0001

#define sas_service_uuid    0xFEF0
#define sas_character_uuid  0xFE66

typedef enum
{
    SAS_AMBIENT_SERVICE_OFF = 0,
    SAS_AMBIENT_SERVICE_ON  = 1
} SAS_AMBIENT_SERVICE_STATUS_E;

typedef struct {
    gatt_svc_t head;
    uint8_t is_notify_enabled;
    uint16_t is_ambient_service_on;
} ble_sas_t;

GATT_DECL_PRI_SERVICE(g_ble_sas_service,
    sas_service_uuid);

GATT_DECL_CHAR(g_ble_sas_character,
    sas_character_uuid,
    GATT_RD_REQ|GATT_WR_REQ|GATT_WR_CMD|GATT_NTF_PROP,
    ATT_WR_ENC);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_sas_cccd,
    ATT_WR_ENC);

static const gatt_attribute_t g_ble_sas_attr_list[] = {
    /* Service */
    gatt_attribute(g_ble_sas_service),
    /* Characteristics */
    gatt_attribute(g_ble_sas_character),
        gatt_attribute(g_ble_sas_cccd),
};

typedef enum
{
    AMBIENT_SERVICE_OFF = 0,
    AMBIENT_SERVICE_ON  = 1
} AMBIENT_SERVICE_STATUS;

typedef void(*app_sas_server_data_received_cb_func_t)(AMBIENT_SERVICE_STATUS status);
typedef void(*app_sas_server_connected_cb_func_t)(uint8_t conidx);
typedef void(*app_sas_server_disconnected_cb_func_t)(uint8_t conidx);

typedef struct
{
    /// is_notification
    bool isNotificationEnabled;
    /// Connection handle
    uint8_t conidx;
    /// Current ambeient status
    AMBIENT_SERVICE_STATUS ambient_status;

    // called when ambient service characteristic is written
    app_sas_server_data_received_cb_func_t data_recieved;

    // called when ccc is written
    app_sas_server_connected_cb_func_t connected_cb;

    // called when the cooresponding connection is disconnected
    app_sas_server_disconnected_cb_func_t disconnected_cb;
} app_sas_env_tag_t;

app_sas_env_tag_t app_sas_env;

static void sas_write_send_notification_handler(uint8_t conidx, uint16_t audio_status)
{
    audio_status = co_host_to_uint16_le(audio_status);
    gatts_send_notification(gap_conn_bf(conidx), g_ble_sas_character, (uint8_t *)&audio_status, sizeof(audio_status));
}

static void app_sas_server_disconnected_evt_handler(uint8_t conidx, uint16_t connhdl)
{
    if (conidx == app_sas_env.conidx)
    {
        app_sas_env.conidx = INVALID_CONNECTION_INDEX;
        app_sas_env.isNotificationEnabled = false;
    }

    if (NULL != app_sas_env.disconnected_cb)
    {
        app_sas_env.disconnected_cb(conidx);
    }
}

static void app_sas_server_inform_ambient_status(void)
{
    if ((INVALID_CONNECTION_INDEX != app_sas_env.conidx) && app_sas_env.isNotificationEnabled)
    {
        sas_write_send_notification_handler(app_sas_env.conidx, app_sas_env.ambient_status);
    }
    else
    {
        TRACE(0, "sas conidx 0x%x ccc val %d", app_sas_env.conidx, app_sas_env.isNotificationEnabled);
    }
}

static void app_sas_server_update_ambient_status(AMBIENT_SERVICE_STATUS status)
{
    app_sas_env.ambient_status = status;
#ifdef ANC_APP
    if (AMBIENT_SERVICE_ON == status)
    {
        app_anc_switch(APP_ANC_MODE1);
    }
    else
    {
        app_anc_switch(APP_ANC_MODE_OFF);
    }
#endif
    app_sas_server_inform_ambient_status();
}

static void app_sas_server_ccc_changed_handler(uint8_t conidx, uint16_t connhdl, bool notify_enabled)
{
    app_sas_env.isNotificationEnabled = notify_enabled;

    if (app_sas_env.isNotificationEnabled)
    {
        // the app datapath server is connected when receiving the first enable CCC request
        if (INVALID_CONNECTION_INDEX == app_sas_env.conidx)
        {
            if (app_sas_env.connected_cb)
            {
                app_sas_env.connected_cb(conidx);
            }
            app_sas_env.conidx = conidx;
            // for test purpose
            // app_sas_server_inform_ambient_status();
        }
    }
}

static void app_sas_server_data_received_handler(uint8_t conidx, uint16_t connhdl, uint16_t audio_status)
{
    app_sas_server_update_ambient_status(audio_status);
    if (NULL != app_sas_env.data_recieved)
    {
        app_sas_env.data_recieved(audio_status);
    }
}

static int ble_sas_server_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
{
    ble_sas_t *sas = (ble_sas_t *)svc;

    switch (event)
    {
        case GATT_SERV_EVENT_CHAR_READ:
        {
            gatt_server_char_read_t *p = param.char_read;
            uint16_t service_on = sas->is_ambient_service_on;
            service_on = co_host_to_uint16_le(service_on);
            gatts_write_read_rsp_data(p->ctx, (uint8_t *)&service_on, sizeof(service_on));
            return true;
        }
        case GATT_SERV_EVENT_CHAR_WRITE:
        {
            uint16_t option = 0;
            gatt_server_char_write_t *p = param.char_write;
            if (p->value_offset != 0 || p->value_len == 0 || p->value == NULL)
            {
                return false;
            }
            option = CO_COMBINE_UINT16_LE(p->value);
            if (option & AMBIENT_SERVICE_SWITCH_BIT_MASK)
            {
                sas->is_ambient_service_on = sas->is_ambient_service_on ? false : true;
            }
            app_sas_server_data_received_handler(svc->con_idx, svc->connhdl, sas->is_ambient_service_on);
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
            sas->is_notify_enabled = notify_enabled;
            app_sas_server_ccc_changed_handler(svc->con_idx, svc->connhdl, notify_enabled);
            return true;
        }
        case GATT_SERV_EVENT_DESC_READ:
        {
            gatt_server_desc_read_t *p = param.desc_read;
            uint16_t cccd_config = sas->is_notify_enabled ? 0x0001 : 0;
            cccd_config = co_host_to_uint16_le(cccd_config);
            gatts_write_read_rsp_data(p->ctx, (uint8_t *)&cccd_config, sizeof(uint16_t));
            return true;
        }
        case GATT_SERV_EVENT_CONN_CLOSED:
        {
            app_sas_server_disconnected_evt_handler(svc->con_idx, svc->connhdl);
            sas->is_ambient_service_on = false;
            sas->is_notify_enabled = false;
            break;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

void ble_sass_init(void)
{
    gatts_cfg_t cfg = {0};
    cfg.svc_size = sizeof(ble_sas_t);
    cfg.preferred_mtu = SAS_PREFERRED_MTU;
    cfg.eatt_preferred = false;

    memset(&app_sas_env, 0, sizeof(app_sas_env_tag_t));
    app_sas_env.ambient_status = AMBIENT_SERVICE_OFF;
    app_sas_env.conidx = INVALID_CONNECTION_INDEX;
#ifdef ANC_APP
    app_anc_switch(APP_ANC_MODE_OFF);
#endif

    gatts_register_service(g_ble_sas_attr_list, ARRAY_SIZE(g_ble_sas_attr_list), ble_sas_server_callback, &cfg);
}

void app_sas_server_register_data_received_cb(app_sas_server_data_received_cb_func_t callback)
{
    app_sas_env.data_recieved = callback;
}

void app_sas_server_register_disconnected_cb(app_sas_server_disconnected_cb_func_t callback)
{
    app_sas_env.disconnected_cb = callback;
}

void app_sas_server_register_connected_cb(app_sas_server_connected_cb_func_t callback)
{
    app_sas_env.connected_cb = callback;
}

#endif /* CFG_APP_SAS_SERVER */