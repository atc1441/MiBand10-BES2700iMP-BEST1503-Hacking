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
#include "bluetooth.h"
#include "gatt_service.h"

#define IAC_CONN_STATE_DISCOVERING_BIT(i)      ((1 << (i + 1)) & IAC_CONN_STATE_DISCOVERING_MASK)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

enum iac_state
{
    /// Discovering
    IAC_CONN_STATE_DISCOVERING_MASK     = ~(1 << 0),
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Connection information structure
typedef struct iac_con_info
{
    /// GATT Profile env
    gatt_prf_t prf;
    /// Peer service
    gatt_peer_service_t *p_service;
    /// Peer Mute
    gatt_peer_character_t *p_char_alert_lvl;
} iac_con_info_t;

/// IAC environment structure
typedef struct iac_env
{
    /// GATT profile index
    uint8_t gatt_prf_id;
    /// IAC state
    uint8_t state_bf;
    /// Connection handle
    uint16_t conhdl[BLE_CONNECTION_MAX];
} iac_env_t;

static iac_env_t iac_env;

const uint16_t ias_char_uuid_list[1] =
{
    /// IAS CHARACTER MUTE
    GATT_CHAR_UUID_ALERT_LEVEL,
};

/*INTERNAL FUNCTIONS*/

static __INLINE bool iac_is_service_discovery_ongoing(uint8_t con_lid)
{
    return (iac_env.state_bf & IAC_CONN_STATE_DISCOVERING_BIT(con_lid));
}

static int iac_gatt_callback(gatt_prf_t *prf, gatt_profile_event_t event,
                                gatt_profile_callback_param_t param)
{
    switch (event)
    {
        case GATT_PROF_EVENT_SERVICE:
        {
            gatt_profile_service_t *p_prf_svc = param.service;
            // If there is no discovery ongoing
            if (iac_is_service_discovery_ongoing(p_prf_svc->conn->con_idx & 0xF) == false)
            {
                break;
            }

            // Will continue discovery all ias and their chars
            if (p_prf_svc->count != 0)
            {
                iac_con_info_t *p_con_info = colist_structure(prf, iac_con_info_t, prf);

                p_con_info->p_service = p_prf_svc->service;

                gattc_discover_multi_characters(prf, p_prf_svc->service, ias_char_uuid_list,
                            sizeof(ias_char_uuid_list) / sizeof(ias_char_uuid_list[0]));
            }
        }
        break;
        case GATT_PROF_EVENT_CHARACTER:
        {
            gatt_profile_character_t *p_prf_char = param.character;

            // If there is no discovery ongoing
            if (iac_is_service_discovery_ongoing(p_prf_char->conn->con_idx & 0xF) == false)
            {
                break;
            }

            iac_con_info_t *p_con_info = colist_structure(prf, iac_con_info_t, prf);

            p_con_info->p_char_alert_lvl = p_prf_char->character;
        }
        break;
        case GATT_PROF_EVENT_OPENED:
        {
            gatt_profile_param_t *p_opened = param.opened;
            iac_env.conhdl[p_opened->prf->con_idx & 0xF] = p_opened->prf->connhdl; 
        }
        break;
        case GATT_PROF_EVENT_CLOSED:
        case GATT_PROF_EVENT_CHAR_READ_RSP:
        case GATT_PROF_EVENT_DESC_READ_RSP:
        case GATT_PROF_EVENT_CHAR_WRITE_RSP:
        case GATT_PROF_EVENT_DESC_WRITE_RSP:
        case GATT_PROF_EVENT_NOTIFY:
        case GATT_PROF_EVENT_INCLUDE:
        default:
        {
            break;
        }
    }

    return 0;
}

int ble_iac_init(void)
{
    uint8_t iac_prf_lid = GATT_PRF_INVALID;

    gattc_cfg_t prf_cfg = {0};
    prf_cfg.prf_size = sizeof(iac_con_info_t);
    prf_cfg.eatt_preferred = true;
    iac_prf_lid = gattc_register_profile(iac_gatt_callback, &prf_cfg);

    if (iac_prf_lid == GATT_PRF_INVALID)
    {
        return BT_STS_FAILED;
    }

    // Reset and init
    memset(&iac_env, 0, sizeof(iac_env_t));

    /// Assign gatt prf lid
    iac_env.gatt_prf_id = iac_prf_lid;

    return BT_STS_SUCCESS;
}

int ble_iac_deinit(void)
{
    if (iac_env.gatt_prf_id != GATT_PRF_INVALID)
    {
        gattc_unregister_profile(iac_env.gatt_prf_id);
    }

    return BT_STS_SUCCESS;
}

int ble_iac_start_discover(uint8_t con_lid)
{
    if (iac_env.gatt_prf_id == GATT_PRF_INVALID)
    {
        return BT_STS_NOT_READY;
    }

    if (iac_is_service_discovery_ongoing(con_lid))
    {
        return BT_STS_IN_PROGRESS;
    }

    gatt_prf_t *p_prf = gattc_get_profile(iac_env.gatt_prf_id, iac_env.conhdl[con_lid]);

    if (p_prf == NULL)
    {
        return BT_STS_NO_LINK;
    }

    uint16_t status = gattc_discover_service(p_prf, GATT_UUID_IAP_SERVICE, NULL);

    if (status == BT_STS_SUCCESS)
    {
        // Set discovery bit
        iac_env.state_bf |= IAC_CONN_STATE_DISCOVERING_BIT(con_lid);
    }

    return status;
}

int iac_write_alert_level(uint8_t con_lid, uint8_t alert_level)
{
    if (alert_level >= 3)
    {
        return BT_STS_INVALID_PARM;
    }

    gatt_prf_t *prf = gattc_get_profile(iac_env.gatt_prf_id, iac_env.conhdl[con_lid]);

    if (prf == NULL)
    {
        return BT_STS_FAILED;
    }

    iac_con_info_t *p_con_info = colist_structure(prf, iac_con_info_t, prf);

    gatt_peer_character_t *p_char = p_con_info->p_char_alert_lvl;

    if (p_char == NULL)
    {
        return BT_STS_NOT_READY;
    }

    return gattc_write_character_value(&p_con_info->prf, p_char, (uint8_t *)&alert_level, sizeof(alert_level));
}

#endif /// BLE_IAS_ENABLED
#endif /// BLE_HOST_SUPPORT