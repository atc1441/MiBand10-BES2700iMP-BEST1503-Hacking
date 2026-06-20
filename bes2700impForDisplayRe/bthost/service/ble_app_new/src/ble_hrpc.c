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

#ifdef BLE_HRPC_ENABLED

#include "ble_hrpc.h"

#include "gatt_service.h"
#include "bluetooth.h"
#include "app_ble.h"


typedef struct {
    /// GATT Profile env
    gatt_prf_t prf;
    /// Peer service
    gatt_peer_service_t *service;
    /// Peer character
    gatt_peer_character_t *heart_rate_measurement;
    gatt_peer_character_t *body_sensor_location;
    gatt_peer_character_t *control_point;
} hrs_content_t;

/// Environment variable for each Connections
typedef struct hrpc_cnx_env
{
    /// Peer database discovered handle mapping
    hrs_content_t       hrs;
    /// counter used to check service uniqueness
    uint8_t             nb_svc;
    /// Client is in discovering state
    bool                discover;
} hrpc_cnx_env_t;

/// Client environment variable
typedef struct hrpc_env
{
    const hrpc_cb_t *p_upper_cb;
    /// GATT profile index
    uint8_t gatt_prf_id;
    /// Environment variable pointer for each connections
    hrpc_cnx_env_t* p_env[BLE_CONNECTION_MAX];
} hrpc_env_t;

__STATIC hrpc_env_t *p_hrpc_env = NULL;


__STATIC void hrpc_unpack_meas(uint8_t conidx, const uint8_t* packet, uint16_t length)
{
    uint8_t i = 0;
    const uint8_t *p_buf = packet;
    const uint8_t *p_buf_end = p_buf + length;
    hrs_hr_meas_t meas;
    memset(&meas, 0, sizeof(hrs_hr_meas_t));

    // Flags
    meas.flags = *p_buf++;

    if (GETB(meas.flags, HRS_FLAG_HR_VALUE_FORMAT))
    {
        // Heart Rate Measurement Value 16 bits
        meas.heart_rate = CO_COMBINE_UINT16_LE(p_buf);
        p_buf += 2;
    }
    else
    {
        // Heart Rate Measurement Value 8 bits
        meas.heart_rate = *p_buf++;
    }

    if (GETB(meas.flags, HRS_FLAG_ENERGY_EXPENDED_PRESENT))
    {
        // Energy Expended present
        meas.energy_expended = CO_COMBINE_UINT16_LE(p_buf);
        p_buf += 2;
    }

    // retrieve number of rr intervals
    meas.nb_rr_interval = co_min((p_buf_end - p_buf) / 2, HRS_MAX_RR_INTERVAL);

    for (i = 0; i < meas.nb_rr_interval; i++)
    {
        // RR-Intervals
        meas.rr_intervals[i] = CO_COMBINE_UINT16_LE(p_buf);
        p_buf += 2;
    }

    if (p_buf > p_buf_end)
    {
        TRACE(0, "%s error!", __func__);
        return;
    }
    // Inform application about received vector
    p_hrpc_env->p_upper_cb->cb_meas(conidx, &meas);
}

uint16_t ble_hrpc_enable(uint8_t conidx)
{
    uint16_t conhdl = app_ble_get_conhdl_from_conidx(conidx);

    if(p_hrpc_env != NULL)
    {
        if (conidx < BLE_CONNECTION_MAX)
        {
            if(p_hrpc_env->p_env[conidx] == NULL)
            {
                p_hrpc_env->p_env[conidx] = (struct hrpc_cnx_env *)bes_bt_buf_malloc(sizeof(struct hrpc_cnx_env));
                if (p_hrpc_env->p_env[conidx] == NULL)
                {
                    return BT_STS_NO_RESOURCES;
                }
                memset(p_hrpc_env->p_env[conidx], 0, sizeof(struct hrpc_cnx_env));

                gatt_prf_t *p_prf = gattc_get_profile(p_hrpc_env->gatt_prf_id, conhdl);
                if (p_prf == NULL)
                {
                    TRACE(0, "hrp invalid conn: %d %04x", p_hrpc_env->gatt_prf_id, conhdl);
                    bes_bt_buf_free(p_hrpc_env->p_env[conidx]);
                    p_hrpc_env->p_env[conidx] = NULL;
                    return BT_STS_INVALID_CONN_HANDLE;
                }

                uint16_t status = gattc_discover_service(p_prf, GATT_UUID_HRP_SERVICE, NULL);
                if (status != BT_STS_SUCCESS)
                {
                    bes_bt_buf_free(p_hrpc_env->p_env[conidx]);
                    p_hrpc_env->p_env[conidx] = NULL;
                    return status;
                }
                p_hrpc_env->p_env[conidx]->discover = true;
                return BT_STS_SUCCESS;
            }
            else
            {
                if (p_hrpc_env->p_env[conidx]->discover == true)
                {
                    return BT_STS_IN_PROGRESS;
                }
                else
                {
                    /* already enable */
                    return BT_STS_FAILED;
                }
            }
        }
    }

    return BT_STS_FAILED;
}

uint16_t ble_hrpc_read_sensor_loc(uint8_t conidx)
{
    uint16_t conhdl = app_ble_get_conhdl_from_conidx(conidx);
    if(conhdl == GAP_INVALID_CONN_HANDLE)
    {
        return BT_STS_INVALID_CONN_HANDLE;
    }

    if(p_hrpc_env != NULL)
    {
        if ((conidx < BLE_CONNECTION_MAX) && (p_hrpc_env->p_env[conidx] != NULL) && (!p_hrpc_env->p_env[conidx]->discover))
        {
            hrs_content_t* p_hrs = &(p_hrpc_env->p_env[conidx]->hrs);
            gatt_peer_character_t *p_char = p_hrs->body_sensor_location;
            if (p_char == NULL)
            {
                return BT_STS_NOT_READY;
            }
            gatt_prf_t *p_prf = gattc_get_profile(p_hrpc_env->gatt_prf_id, conhdl);
            if (p_prf == NULL)
            {
                return BT_STS_FAILED;
            }
            return gattc_read_character_value(p_prf, p_char);
        }
    }

    return BT_STS_FAILED;
}

uint16_t ble_hrpc_read_cfg(uint8_t conidx)
{
    uint16_t conhdl = app_ble_get_conhdl_from_conidx(conidx);
    if(conhdl == GAP_INVALID_CONN_HANDLE)
    {
        return BT_STS_INVALID_CONN_HANDLE;
    }

    if(p_hrpc_env != NULL)
    {
        if ((conidx < BLE_CONNECTION_MAX) && (p_hrpc_env->p_env[conidx] != NULL) && (!p_hrpc_env->p_env[conidx]->discover))
        {
            hrs_content_t* p_hrs = &(p_hrpc_env->p_env[conidx]->hrs);
            gatt_peer_character_t *p_char = p_hrs->heart_rate_measurement;
            if (p_char == NULL)
            {
                return BT_STS_NOT_READY;
            }
            gatt_prf_t *p_prf = gattc_get_profile(p_hrpc_env->gatt_prf_id, conhdl);
            if (p_prf == NULL)
            {
                return BT_STS_FAILED;
            }
            return gattc_read_descriptor_value(p_prf, p_char, GATT_DESC_UUID_CHAR_CLIENT_CONFIG);
        }
    }

    return BT_STS_FAILED;
}

uint16_t ble_hrpc_write_cfg(uint8_t conidx, uint16_t cfg_val)
{
    uint16_t conhdl = app_ble_get_conhdl_from_conidx(conidx);
    if(conhdl == GAP_INVALID_CONN_HANDLE)
    {
        return BT_STS_INVALID_CONN_HANDLE;
    }

    if ((cfg_val != PRF_CLI_STOP_NTFIND) && (cfg_val != PRF_CLI_START_NTF))
    {
        return BT_STS_INVALID_PARM;
    }

    if(p_hrpc_env != NULL)
    {
        if ((conidx < BLE_CONNECTION_MAX) && (p_hrpc_env->p_env[conidx] != NULL) && (!p_hrpc_env->p_env[conidx]->discover))
        {
            hrs_content_t* p_hrs = &(p_hrpc_env->p_env[conidx]->hrs);
            gatt_peer_character_t *p_char = p_hrs->heart_rate_measurement;
            if (p_char == NULL)
            {
                return BT_STS_NOT_READY;
            }
            gatt_prf_t *p_prf = gattc_get_profile(p_hrpc_env->gatt_prf_id, conhdl);
            if (p_prf == NULL)
            {
                return BT_STS_FAILED;
            }

            bool enable_ntf = (cfg_val == PRF_CLI_STOP_NTFIND ? false : true);

            return gattc_write_cccd_descriptor(p_prf, p_char, enable_ntf, false);
        }
    }

    return BT_STS_FAILED;
}

uint16_t ble_hrpc_ctnl_pt_req(uint8_t conidx, uint8_t value)
{
    uint16_t conhdl = app_ble_get_conhdl_from_conidx(conidx);
    if(conhdl == GAP_INVALID_CONN_HANDLE)
    {
        return BT_STS_INVALID_CONN_HANDLE;
    }

    if(p_hrpc_env != NULL)
    {
        if ((conidx < BLE_CONNECTION_MAX) && (p_hrpc_env->p_env[conidx] != NULL) && (!p_hrpc_env->p_env[conidx]->discover))
        {
            hrs_content_t* p_hrs = &(p_hrpc_env->p_env[conidx]->hrs);
            gatt_peer_character_t *p_char = p_hrs->control_point;
            if (p_char == NULL)
            {
                return BT_STS_NOT_READY;
            }
            gatt_prf_t *p_prf = gattc_get_profile(p_hrpc_env->gatt_prf_id, conhdl);
            if (p_prf == NULL)
            {
                return BT_STS_FAILED;
            }
            return gattc_write_character_value(p_prf, p_char, &value, sizeof(uint8_t));
        }
    }

    return BT_STS_FAILED;
}


__STATIC int app_ble_hrp_client_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param)
{
    uint8_t conidx = prf->con_idx;
    hrpc_cnx_env_t* p_con_env = NULL;
    hrs_content_t* p_hrs = NULL;
    uint16_t status = 0;

    if (p_hrpc_env == NULL)
    {
        return false;
    }

    p_con_env = p_hrpc_env->p_env[conidx];
    if (p_con_env == NULL)
    {
        /* not enable */
        return false;
    }
    p_hrs = &(p_con_env->hrs);

    switch (event)
    {
        case GATT_PROF_EVENT_SERVICE:
        {
            gatt_profile_service_t *p = param.service;
            gatt_peer_service_t *s = p->service;

            if (p_con_env->nb_svc == 0)
            {
                if (p->error_code != ATT_ERROR_NO_ERROR)
                {
                    break;
                }
                if (s->service_uuid == GATT_UUID_HRP_SERVICE)
                {
                    p_hrs->service = s;

                    uint16_t gatt_chars[] = {GATT_CHAR_UUID_HEART_RATE_MEASURE,
                                             GATT_CHAR_UUID_BODY_SENSOR_LOCATION,
                                             GATT_CHAR_UUID_HEART_RATE_CTRL_POINT};
                    gattc_discover_multi_characters(prf, s, gatt_chars, sizeof(gatt_chars)/sizeof(uint16_t));
                }

                p_con_env->nb_svc++;
            }
        } break;
        case GATT_PROF_EVENT_CHARACTER:
        {
            gatt_profile_character_t *p = param.character;
            status = p->error_code;
            TRACE(0, "Char discover uuid=%d, cmpl=%d, status=%d", p->char_uuid, p->discover_cmpl, status);
            if (status != ATT_ERROR_NO_ERROR)
            {
                break;
            }

            if (p->char_uuid == GATT_CHAR_UUID_HEART_RATE_MEASURE)
            {
                p_hrs->heart_rate_measurement = p->character;
                if ((p->char_prop & GATT_NTF_PROP) != 0)
                {
                    gattc_write_cccd_descriptor(prf, p->character, true, false);
                }
            }
            else if (p->char_uuid == GATT_CHAR_UUID_BODY_SENSOR_LOCATION)
            {
                p_hrs->body_sensor_location = p->character;
            }
            else if (p->char_uuid == GATT_CHAR_UUID_HEART_RATE_CTRL_POINT)
            {
                p_hrs->control_point = p->character;
            }

            if (p->discover_cmpl)
            {
                p_hrpc_env->p_upper_cb->cb_enable_cmp(conidx, status);
                p_con_env->discover = false;
            }
        } break;
        case GATT_PROF_EVENT_CHAR_READ_RSP:
        {
            gatt_profile_char_read_rsp_t *p = param.char_read_rsp;
            uint8_t sensor_loc = (p->error_code == ATT_ERROR_NO_ERROR ? *(p->value) : 0);
            status = p->error_code;
            p_hrpc_env->p_upper_cb->cb_read_sensor_loc_cmp(conidx, status, sensor_loc);
        } break;
        case GATT_PROF_EVENT_DESC_READ_RSP:
        {
            gatt_profile_desc_read_rsp_t *p = param.desc_read_rsp;
            uint16_t cfg_val = (p->error_code == ATT_ERROR_NO_ERROR ? CO_COMBINE_UINT16_LE(p->value) : 0);
            status = p->error_code;
            p_hrpc_env->p_upper_cb->cb_read_cfg_cmp(conidx, status, cfg_val);
        } break;
        case GATT_PROF_EVENT_CHAR_WRITE_RSP:
        {
            gatt_profile_char_write_rsp_t *p = param.char_write_rsp;
            status = p->error_code;
            p_hrpc_env->p_upper_cb->cb_ctnl_pt_req_cmp(conidx, status);
        } break;
        case GATT_PROF_EVENT_DESC_WRITE_RSP:
        {
            gatt_profile_desc_write_rsp_t *p = param.desc_write_rsp;
            status = p->error_code;
            p_hrpc_env->p_upper_cb->cb_write_cfg_cmp(conidx, status);
        } break;
        case GATT_PROF_EVENT_NOTIFY:
        {
            gatt_profile_recv_notify_t *ntf = param.notify;
            if (ntf->character == p_hrs->heart_rate_measurement)
            {
                hrpc_unpack_meas(conidx, ntf->value, ntf->value_len);
            }
        } break;
        case GATT_PROF_EVENT_OPENED:
        {
            p_hrs->prf = *prf;
        } break;
        case GATT_PROF_EVENT_CLOSED:
        case GATT_PROF_EVENT_INCLUDE:
            break;
        default:
        {
            break;
        }
    }

    return 0;
}

uint16_t ble_hrpc_init(const hrpc_cb_t* p_cb)
{
    uint8_t hrpc_prf_lid = GATT_PRF_INVALID;

    if (p_hrpc_env != NULL)
    {
        return BT_STS_IN_USE;
    }

    if (p_cb == NULL ||
        p_cb->cb_enable_cmp == NULL ||
        p_cb->cb_read_sensor_loc_cmp == NULL ||
        p_cb->cb_read_cfg_cmp == NULL ||
        p_cb->cb_write_cfg_cmp == NULL ||
        p_cb->cb_meas == NULL ||
        p_cb->cb_ctnl_pt_req_cmp == NULL)
    {
        return BT_STS_INVALID_PARM;
    }

    gattc_cfg_t prf_cfg = {0};
    // prf_cfg.prf_size = sizeof(app_ble_hrp_prf_t);
    hrpc_prf_lid = gattc_register_profile(app_ble_hrp_client_callback, &prf_cfg);
    if (hrpc_prf_lid == GATT_PRF_INVALID)
    {
        return BT_STS_FAILED;
    }

    // Malloc for environment
    p_hrpc_env = (hrpc_env_t *)bes_bt_buf_malloc(sizeof(hrpc_env_t));
    if (p_hrpc_env == NULL)
    {
        gattc_unregister_profile(hrpc_prf_lid);
        return BT_STS_NO_RESOURCES;
    }
    memset(p_hrpc_env, 0, sizeof(hrpc_env_t));

    p_hrpc_env->p_upper_cb = p_cb;
    p_hrpc_env->gatt_prf_id = hrpc_prf_lid;

    return BT_STS_SUCCESS;
}

uint16_t ble_hrpc_deinit()
{
    return BT_STS_SUCCESS;
}

#endif