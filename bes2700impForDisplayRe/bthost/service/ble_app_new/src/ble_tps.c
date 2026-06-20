/***************************************************************************
 *
 * Copyright 2020-2025 BES.
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
#ifdef BLE_TXP_ENABLED // Tx power services

#include "gatt_service.h"

#define TPS_PREF_MTU             (512)

GATT_DECL_PRI_SERVICE(g_txp_service, GATT_UUID_TXP_SERVICE);

GATT_DECL_CHAR(g_txp_level_character,
    GATT_CHAR_UUID_TX_PWR_LEVEL,
    GATT_RD_REQ, // Readable only when discoverable, Optional writable
    ATT_SEC_NONE);

static const gatt_attribute_t g_txp_service_attr_list[] = {
    /* Service */
    gatt_attribute(g_txp_service),
    /* Characteristics */
    gatt_attribute(g_txp_level_character),
};

static int ble_tps_server_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
{
    int ret = 1;
    switch (event)
    {
        case GATT_SERV_EVENT_CHAR_READ:
        {
            gatt_server_char_read_t *p = param.char_read;
            if (p->character == g_txp_level_character) // -100 ~ 20 dBm
            {
                uint8_t txPower = 6;
                gatts_write_read_rsp_data(p->ctx, &txPower, sizeof(uint8_t));
            }
            else
            {
                ret = 0;
            }
            break;
        }
        default:
        ret = 0;
        break;
    }

    return ret;
}

void ble_tps_init(void)
{
    gatts_cfg_t cfg = {0};
    cfg.svc_size = sizeof(gatt_svc_t);
    cfg.preferred_mtu = TPS_PREF_MTU;
    cfg.eatt_preferred = false;
    gatts_register_service(g_txp_service_attr_list, ARRAY_SIZE(g_txp_service_attr_list), ble_tps_server_callback, &cfg);
}

#endif /* BLE_TPS_ENABLE */
