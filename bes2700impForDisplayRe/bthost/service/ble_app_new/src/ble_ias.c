/***************************************************************************
 *
 * Copyright (c) 2015-2023 BES Technic
 *
 * Authored by BES CD team (Blueelf Prj).
 *
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
#ifdef BLE_HOST_SUPPORT

#ifdef BLE_IAS_ENABLED
#include "gatt_service.h"

typedef struct ias_env
{
    /// Alert Level
    uint8_t alert_level;
} ias_env_t;

static int ias_gatt_server_callback(gatt_svc_t *svc, gatt_server_event_t event,
                                       gatt_server_callback_param_t param);

/*IAS environment*/
static ias_env_t ias_env;

/*IAS declaration*/
GATT_DECL_PRI_SERVICE(IA_SERVICE, GATT_UUID_IAP_SERVICE);

GATT_DECL_CHAR(IAS_ALERT_LVL, GATT_CHAR_UUID_ALERT_LEVEL,
               GATT_WR_CMD, ATT_SEC_NONE);

static const gatt_attribute_t ia_service_attr_list[] =
{
    /* Service */
    gatt_attribute(IA_SERVICE),
    /* Characteristics */
    gatt_attribute(IAS_ALERT_LVL),
};

/*IAS gatt service callback*/
static int ias_gatt_server_callback(gatt_svc_t *svc, gatt_server_event_t event,
                                       gatt_server_callback_param_t param)
{
    switch (event)
    {
        case GATT_SERV_EVENT_CHAR_WRITE:
        {
            gatt_server_char_write_t *p = param.char_write;

            if (p->value_len != sizeof(uint8_t))
            {
                p->rsp_error_code = ATT_ERROR_INVALID_VALUE_LEN;
                return false;
            }

            uint8_t alert_level = *p->value;

            if (p->character == IAS_ALERT_LVL)
            {
                if (ias_env.alert_level != alert_level)
                {
                    ias_env.alert_level = alert_level;
                    CO_LOG_INFO_0(alert_level);
                }
            }
            else
            {
                p->rsp_error_code = ATT_ERROR_WR_NOT_PERMITTED;
                return false;
            }
            return true;
        }
        case GATT_SERV_EVENT_CONN_OPENED:
        case GATT_SERV_EVENT_CONN_CLOSED:
        case GATT_SERV_EVENT_CHAR_READ:
        case GATT_SERV_EVENT_DESC_READ:
        case GATT_SERV_EVENT_DESC_WRITE:
        case GATT_SERV_EVENT_INDICATE_CFM:
        default:
        {
            break;
        }
    }

    return false;
}

int ble_ias_init(void)
{
    uint16_t status = BT_STS_SUCCESS;

    memset(&ias_env, 0, sizeof(ias_env_t));

    status = gatts_register_service(ia_service_attr_list, sizeof(ia_service_attr_list) / sizeof(ia_service_attr_list[0]), ias_gatt_server_callback, NULL);

    CO_LOG_INFO_1("status: %d", status);

    return status;
}

int ble_ias_deinit(void)
{
    uint16_t status = BT_STS_SUCCESS;

    status = gatts_unregister_service(ia_service_attr_list);

    return status;
}

#endif /// BLE_IAS_ENABLED
#endif /// BLE_HOST_SUPPORT
