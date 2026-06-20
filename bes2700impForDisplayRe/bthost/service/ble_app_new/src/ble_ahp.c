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
#include "bluetooth.h"

#if (BLE_AHP_SERVER_SUPPORT) // advanced headphone service
#include "gatt_service.h"
#include "ble_ahp.h"

#define AHP_PREFERRED_MTU                       512

#define INVALID_CONNECTION_INDEX                0xFF
#define ADVANCED_HEADPHONE_SERVICE_BRC_BIT_MASK 0x0001
#define ADVANCED_HEADPHONE_SERVICE_HTC_BIT_MASK 0x0001

typedef struct
{
    gatt_svc_t head;
    uint8_t is_brc_notify_enabled;
    uint8_t is_brc_on; // Binaural Rendoring Control fucntionailty status
    uint8_t is_htc_notify_enabled;
    uint8_t is_htc_on; // Head Tracking Control fuctionailty status
} ble_ahs_t;

GATT_DECL_PRI_SERVICE(g_ble_ahp_service, GATT_UUID_AHP_SERVICE);

GATT_DECL_CHAR(g_ble_ahp_brc_character, GATT_CHAR_UUID_BINAURAL_RENDERING_CONTROL,
               GATT_WR_REQ | GATT_NTF_PROP, ATT_WR_ENC);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_ahp_brc_cccd, ATT_WR_ENC);

GATT_DECL_CHAR(g_ble_ahp_htc_character, GATT_CHAR_UUID_HEAD_TRACKING_CONTROL,
               GATT_WR_REQ | GATT_NTF_PROP, ATT_WR_ENC);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_ahp_htc_cccd, ATT_WR_ENC);

GATT_DECL_CHAR(g_ble_ahp_amt_features_character, GATT_CHAR_UUID_AMT_FEATURES,
               GATT_WR_REQ | GATT_RD_REQ | GATT_NTF_PROP, ATT_WR_ENC);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_ahp_amt_features_cccd, ATT_WR_ENC);

GATT_DECL_CHAR(g_ble_ahp_amg_features_character, GATT_CHAR_UUID_AMG_FEATURES,
               GATT_WR_REQ | GATT_RD_REQ | GATT_NTF_PROP, ATT_WR_ENC);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_ahp_amg_features_cccd, ATT_WR_ENC);

GATT_DECL_CHAR(g_ble_ahp_role_character, GATT_CHAR_UUID_AHP_ROLE,
               GATT_WR_REQ | GATT_RD_REQ | GATT_NTF_PROP, ATT_WR_ENC);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_ahp_role_cccd, ATT_WR_ENC);

static const gatt_attribute_t g_ble_ahp_attr_list[] =
{
    /* Service */
    gatt_attribute(g_ble_ahp_service),
    /* Characteristics */
    gatt_attribute(g_ble_ahp_brc_character),
    gatt_attribute(g_ble_ahp_brc_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_ahp_htc_character),
    gatt_attribute(g_ble_ahp_htc_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_ahp_amt_features_character),
    gatt_attribute(g_ble_ahp_amt_features_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_ahp_amg_features_character),
    gatt_attribute(g_ble_ahp_amg_features_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_ahp_role_character),
    gatt_attribute(g_ble_ahp_role_cccd),
};

typedef enum
{
    AHP_SERVER_BRC_OFF = 0,
    AHP_SERVER_BRC_ON  = 1
} ADVANCED_HEADPHONE_SERVICE_BRC_STATUS;

/// AHPS HTC Application Module Environment Structure
typedef enum
{
    AHP_SERVER_HTC_OFF = 0,
    AHP_SERVER_HTC_ON  = 1
} ADVANCED_HEADPHONE_SERVICE_HTC_STATUS;

typedef void(*app_ahps_server_data_received_cb_func_t)(uint8_t status);

typedef void(*app_ahps_server_connected_cb_func_t)(uint8_t conidx);

typedef void(*app_ahps_server_disconnected_cb_func_t)(uint8_t conidx);

typedef struct
{
    /// is_BRC_notification
    bool isBRCNotificationEnabled;
    /// is_HTC_notification
    bool isHTCNotificationEnabled;
    /// Connection handle
    uint8_t conidx;
    /// Current ARC status
    ADVANCED_HEADPHONE_SERVICE_BRC_STATUS ahps_brc_status;
    /// Current HTC status
    ADVANCED_HEADPHONE_SERVICE_HTC_STATUS ahps_htc_status;
    /// Prefix AHP Role Characteristic
    ahp_service_char_ahp_role_t           ahps_ahp_role;
    /// Prefix AMG Features Characteristic
    ahp_service_char_amg_features_t       ahps_amg_features;
    /// Prefix AMT Features Characteristic
    ahp_service_char_amt_features_t       ahps_amt_features;
    // called when AHP Server characteristic is written
    app_ahps_server_data_received_cb_func_t data_recieved;
    // called when ccc is written
    app_ahps_server_connected_cb_func_t connected_cb;
    // called when the cooresponding connection is disconnected
    app_ahps_server_disconnected_cb_func_t disconnected_cb;

} app_ahps_env_tag_t;

typedef struct
{
    uint8_t     aft_id;
    uint8_t     num;
    uint32_t    ft_id;
    uint8_t     len_ft_despcriptor;
    uint32_t    generic_aud_chl;
    uint8_t     supported_aud_chl_cnts;
} __attribute__((packed)) APP_AHPS_AFT_RSP_T;

app_ahps_env_tag_t app_ahps_env;

static void ahps_brc_write_send_notification_handler(uint8_t conidx, uint8_t brc_status)
{
    gatts_send_notification(gap_conn_bf(conidx), g_ble_ahp_brc_character, &brc_status, sizeof(brc_status));
}

static void ahps_htc_write_send_notification_handler(uint8_t conidx, uint8_t htc_status)
{
    gatts_send_notification(gap_conn_bf(conidx), g_ble_ahp_htc_character, &htc_status, sizeof(htc_status));
}

static void app_ahp_server_disconnected_evt_handler(uint8_t conidx, uint16_t connhdl)
{
    if (conidx == app_ahps_env.conidx)
    {
        app_ahps_env.conidx = INVALID_CONNECTION_INDEX;
        app_ahps_env.isBRCNotificationEnabled = false;
        app_ahps_env.isHTCNotificationEnabled = false;
    }

    if (NULL != app_ahps_env.disconnected_cb)
    {
        app_ahps_env.disconnected_cb(conidx);
    }
}

static void app_ahp_server_inform_brc_status(void)
{
    if ((INVALID_CONNECTION_INDEX != app_ahps_env.conidx) && app_ahps_env.isBRCNotificationEnabled)
    {
        ahps_brc_write_send_notification_handler(app_ahps_env.conidx, app_ahps_env.ahps_brc_status);
    }
    else
    {
        TRACE(0, "ahps conidx 0x%x ccc val %d", app_ahps_env.conidx, app_ahps_env.isBRCNotificationEnabled);
    }
}

static void app_ahp_server_inform_htc_status(void)
{
    if ((INVALID_CONNECTION_INDEX != app_ahps_env.conidx) && app_ahps_env.isHTCNotificationEnabled)
    {
        ahps_htc_write_send_notification_handler(app_ahps_env.conidx, app_ahps_env.ahps_htc_status);
    }
    else
    {
        TRACE(0, "ahps conidx 0x%x ccc val %d", app_ahps_env.conidx, app_ahps_env.isHTCNotificationEnabled);
    }
}

static void app_ahp_server_update_brc_status(ADVANCED_HEADPHONE_SERVICE_BRC_STATUS status)
{
    app_ahps_env.ahps_brc_status = status;
    app_ahp_server_inform_brc_status();
}

static void app_ahp_server_update_htc_status(ADVANCED_HEADPHONE_SERVICE_HTC_STATUS status)
{
    app_ahps_env.ahps_htc_status = status;
    app_ahp_server_inform_htc_status();
}

static void app_ahp_server_brc_ccc_changed_handler(uint8_t conidx, uint16_t connhdl, bool notify_enabled)
{
    app_ahps_env.isBRCNotificationEnabled = notify_enabled;

    if (app_ahps_env.isBRCNotificationEnabled)
    {
        // the app ahp server is connected when receiving the first enable CCC request
        if (INVALID_CONNECTION_INDEX == app_ahps_env.conidx)
        {
            if (app_ahps_env.connected_cb)
            {
                app_ahps_env.connected_cb(conidx);
            }
            app_ahps_env.conidx = conidx;
        }
    }
}

static void app_ahp_server_htc_ccc_changed_handler(uint8_t conidx, uint16_t connhdl, bool notify_enabled)
{
    app_ahps_env.isHTCNotificationEnabled = notify_enabled;

    if (app_ahps_env.isHTCNotificationEnabled)
    {
        // the app ahp server is connected when receiving the first enable CCC request
        if (INVALID_CONNECTION_INDEX == app_ahps_env.conidx)
        {
            if (app_ahps_env.connected_cb)
            {
                app_ahps_env.connected_cb(conidx);
            }
            app_ahps_env.conidx = conidx;
        }
    }
}

static void app_ahp_server_brc_data_received_handler(uint8_t conidx, uint16_t connhdl, uint8_t brc_status)
{
    app_ahp_server_update_brc_status(brc_status);

    if (NULL != app_ahps_env.data_recieved)
    {
        app_ahps_env.data_recieved(brc_status);
    }
}

static void app_ahp_server_htc_data_received_handler(uint8_t conidx, uint16_t connhdl, uint8_t htc_status)
{
    app_ahp_server_update_htc_status(htc_status);

    if (NULL != app_ahps_env.data_recieved)
    {
        app_ahps_env.data_recieved(htc_status);
    }
}

static int ble_ahp_server_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
{
    ble_ahs_t *ahs = (ble_ahs_t *)svc;

    switch (event)
    {
        case GATT_SERV_EVENT_CHAR_WRITE:
        {
            uint16_t option = 0;
            gatt_server_char_write_t *p = param.char_write;
            if (p->value_offset != 0 || p->value_len == 0 || p->value == NULL)
            {
                return false;
            }
            option = CO_COMBINE_UINT16_LE(p->value);
            if (p->character == g_ble_ahp_brc_character)
            {
                if (option & ADVANCED_HEADPHONE_SERVICE_BRC_BIT_MASK)
                {
                    ahs->is_brc_on =  ahs->is_brc_on ? false : true;
                }
                app_ahp_server_brc_data_received_handler(svc->con_idx, svc->connhdl, ahs->is_brc_on);
            }
            else
            {
                if (option & ADVANCED_HEADPHONE_SERVICE_HTC_BIT_MASK)
                {
                    ahs->is_htc_on = ahs->is_htc_on ? false : true;
                }
                app_ahp_server_htc_data_received_handler(svc->con_idx, svc->connhdl, ahs->is_htc_on);
            }
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
            if ((uint8_t *)p->desc_attr->attr_data == g_ble_ahp_brc_cccd)
            {
                ahs->is_brc_notify_enabled = notify_enabled;
                app_ahp_server_brc_ccc_changed_handler(svc->con_idx, svc->connhdl, notify_enabled);
            }
            else
            {
                ahs->is_htc_notify_enabled = notify_enabled;
                app_ahp_server_htc_ccc_changed_handler(svc->con_idx, svc->connhdl, notify_enabled);
            }
            return true;
        }
        case GATT_SERV_EVENT_CHAR_READ:
        {
            gatt_server_char_read_t *p = param.char_read;
            uint16_t char_switch_on = 0;
            if (p->character == g_ble_ahp_brc_character)
            {
                char_switch_on = ahs->is_brc_on;
                char_switch_on = co_host_to_uint16_le(char_switch_on);
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)&char_switch_on, sizeof(char_switch_on));
                return true;
            }
            else if (p->character == g_ble_ahp_htc_character)
            {
                char_switch_on = ahs->is_htc_on;
                char_switch_on = co_host_to_uint16_le(char_switch_on);
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)&char_switch_on, sizeof(char_switch_on));
                return true;
            }
            else if (p->character == g_ble_ahp_role_character)
            {
                ahp_service_char_ahp_role_t ahp_role_rsp;
                ahp_role_rsp = app_ahps_env.ahps_ahp_role;

                gatts_write_read_rsp_data(p->ctx, (uint8_t *)&ahp_role_rsp, sizeof(ahp_role_rsp));
                return true;
            }
            else if (p->character == g_ble_ahp_amg_features_character)
            {
                ahp_service_char_amg_features_t amg_features_rsp;
                amg_features_rsp = app_ahps_env.ahps_amg_features;

                gatts_write_read_rsp_data(p->ctx, (uint8_t *)&amg_features_rsp, sizeof(amg_features_rsp));
                return true;
            }
            else if (p->character == g_ble_ahp_amt_features_character)
            {
                ahp_service_char_amg_features_t amt_features_rsp;
                amt_features_rsp = app_ahps_env.ahps_amt_features;

                gatts_write_read_rsp_data(p->ctx, (uint8_t *)&amt_features_rsp, sizeof(amt_features_rsp));
                return true;
            }
            break;
        }
        case GATT_SERV_EVENT_CONN_CLOSED:
        {
            app_ahp_server_disconnected_evt_handler(svc->con_idx, svc->connhdl);
            ahs->is_brc_on = false;
            ahs->is_htc_on = false;
            ahs->is_brc_notify_enabled = false;
            ahs->is_htc_notify_enabled = false;
            break;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

void ble_ahp_init(void)
{
    gatts_cfg_t cfg = {0};
    cfg.svc_size = sizeof(ble_ahs_t);
    cfg.preferred_mtu = AHP_PREFERRED_MTU;
    cfg.eatt_preferred = false;

    memset(&app_ahps_env, 0, sizeof(app_ahps_env_tag_t));
    app_ahps_env.ahps_brc_status = AHP_SERVER_BRC_OFF;
    app_ahps_env.ahps_htc_status = AHP_SERVER_HTC_OFF;
    app_ahps_env.conidx = INVALID_CONNECTION_INDEX;
    app_ahps_env.isBRCNotificationEnabled = false;
    app_ahps_env.isHTCNotificationEnabled = false;
    app_ahps_env.ahps_ahp_role = AHS_AHP_ROLE_AMT;
    app_ahps_env.ahps_amg_features = 0x00;
    app_ahps_env.ahps_amt_features = AHS_AMT_FEATURES_SOURCE_SUPPORTED | AHS_AMT_FEATURES_48K_SOURCE_SUPPORTED |
                                     AHS_AMT_FEATURES_64K_SOURCE_SUPPORTED | AHS_AMT_FEATURES_64K_SINK_SUPPORTED |
                                     AHS_AMT_FEATURES_96k_SINK_SUPPORTED;

    gatts_register_service(g_ble_ahp_attr_list, ARRAY_SIZE(g_ble_ahp_attr_list), ble_ahp_server_callback, &cfg);
}

void app_ahps_server_register_data_received_cb(app_ahps_server_data_received_cb_func_t callback)
{
    app_ahps_env.data_recieved = callback;
}

void app_ahps_server_register_disconnected_cb(app_ahps_server_disconnected_cb_func_t callback)
{
    app_ahps_env.disconnected_cb = callback;
}

void app_ahps_server_register_connected_cb(app_ahps_server_connected_cb_func_t callback)
{
    app_ahps_env.connected_cb = callback;
}

#endif /* CFG_APP_AHP_SERVER */
