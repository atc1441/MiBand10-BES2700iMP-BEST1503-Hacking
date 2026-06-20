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

#ifdef BLE_CSCPC_ENABLED

#include "ble_cscpc.h"

#include "bluetooth.h"
#include "gatt_service.h"
#include "app_ble.h"
#include "gaf_cfg.h"

/// Maximum number of Client task instances
/// CSCP Control point procedure timeout (30s in milliseconds)
#define CSCP_SP_TIMEOUT  (30000)


typedef struct cscpc_cscs_content
{
    /// GATT Profile env
    gatt_prf_t prf;
    /// Peer service
    gatt_peer_service_t *p_service;
    /// Characteristic info:
    ///  - CSC Measurement
    ///  - CSC Feature
    ///  - Sensor Location
    ///  - SC Control Point
    gatt_peer_character_t *p_chars[CSCP_CSCS_CHAR_MAX];

} cscpc_cscs_content_t;

/// Environment variable for each Connections
typedef struct cscpc_cnx_env
{
    /// Control point timer
    uint8                   timer_handle;
    /// Peer database discovered handle mapping
    cscpc_cscs_content_t    cscs;
    /// counter used to check service uniqueness
    uint8_t                 nb_svc;
    /// Client is in discovering state
    bool                    discover;
    /// Control point operation on-going (see enum #cpp_ctnl_pt_code)
    uint8_t                 ctrl_pt_op;
} cscpc_cnx_env_t;

/// Client environment variable
typedef struct cscpc_env
{
    const cscpc_cb_t *p_upper_cb;
    /// GATT profile index
    uint8_t gatt_prf_id;
    /// Environment variable pointer for each connections
    cscpc_cnx_env_t *p_env[BLE_CONNECTION_MAX];
} cscpc_env_t;

__STATIC cscpc_env_t *p_cscpc_env = NULL;


__STATIC void cscpc_timer_handler(void *arg)
{
    uint32_t conidx = (uint32_t)arg;

    if (p_cscpc_env != NULL)
    {
        cscpc_cnx_env_t* p_con_env = p_cscpc_env->p_env[conidx];
        ASSERT_ERR(p_con_env != NULL);
        if(p_con_env->ctrl_pt_op != CSCP_CTNL_PT_OP_RESERVED)
        {
            const cscpc_cb_t* p_cb = (const cscpc_cb_t*) p_cscpc_env->p_upper_cb;
            uint8_t op_code = p_con_env->ctrl_pt_op;
            p_con_env->ctrl_pt_op = CSCP_CTNL_PT_OP_RESERVED;

            p_cb->cb_ctnl_pt_req_cmp((uint8_t)conidx, BT_STS_INIT_TIMER_TIMEOUT, op_code, 0, NULL);
        }
    }
}

__STATIC void cscpc_timer_start(uint32_t conidx)
{
    uint8 *p_timer_handle = NULL;

    if (p_cscpc_env == NULL || p_cscpc_env->p_env[conidx] == NULL)
    {
        return;
    }

    p_timer_handle = &(p_cscpc_env->p_env[conidx]->timer_handle);

    if (*p_timer_handle == 0)
    {
        co_timer_new(p_timer_handle, CSCP_SP_TIMEOUT, cscpc_timer_handler, (void *)(uintptr_t)conidx, 1);
    }
    if (*p_timer_handle)
    {
        co_timer_start(p_timer_handle);
    }
    else
    {
        TRACE(0, "%s error", __func__);
    }
}

__STATIC void cscpc_timer_stop(uint32_t conidx)
{
    uint8 *p_timer_handle = NULL;

    if (p_cscpc_env == NULL || p_cscpc_env->p_env[conidx] == NULL)
    {
        return;
    }

    p_timer_handle = &(p_cscpc_env->p_env[conidx]->timer_handle);

    if (*p_timer_handle)
    {
        co_timer_del(p_timer_handle);
        *p_timer_handle = 0;
    }
}

__STATIC uint16_t cscpc_read_val(uint8_t conidx, uint16_t val_id)
{
    uint16_t conhdl = app_ble_get_conhdl_from_conidx(conidx);
    if(conhdl == GAP_INVALID_CONN_HANDLE)
    {
        return BT_STS_INVALID_CONN_HANDLE;
    }

    if(p_cscpc_env != NULL)
    {
        if ((conidx < BLE_CONNECTION_MAX) && (p_cscpc_env->p_env[conidx] != NULL) && (!p_cscpc_env->p_env[conidx]->discover))
        {
            cscpc_cnx_env_t* p_con_env = p_cscpc_env->p_env[conidx];
            cscpc_cscs_content_t* p_cscs = &(p_con_env->cscs);
            gatt_peer_character_t* p_char = NULL;
            gatt_prf_t *p_prf = gattc_get_profile(p_cscpc_env->gatt_prf_id, conhdl);
            bool is_cfg = (val_id & CSCPC_DESC_MASK) ? true : false;

            switch(val_id)
            {
                case CSCPC_RD_CSC_FEAT:          { p_char = p_cscs->p_chars[CSCP_CSCS_CSC_FEAT_CHAR];   } break;
                case CSCPC_RD_SENSOR_LOC:        { p_char = p_cscs->p_chars[CSCP_CSCS_SENSOR_LOC_CHAR]; } break;
                case CSCPC_RD_WR_CSC_MEAS_CFG:   { p_char = p_cscs->p_chars[CSCP_CSCS_CSC_MEAS_CHAR];   } break;
                case CSCPC_RD_WR_SC_CTNL_PT_CFG: { p_char = p_cscs->p_chars[CSCP_CSCS_SC_CTNL_PT_CHAR]; } break;
                default: break;
            }

            if (p_prf == NULL || p_char == NULL)
            {
                return BT_STS_FAILED;
            }
            else
            {
                // perform read request
                if (!is_cfg)
                {
                    return gattc_read_character_value(p_prf, p_char);
                }
                else
                {
                    return gattc_read_descriptor_value(p_prf, p_char, GATT_DESC_UUID_CHAR_CLIENT_CONFIG);
                }
            }
        }
    }

    return BT_STS_FAILED;
}

__STATIC uint16_t cscpc_pack_ctnl_pt_req(uint8_t* p_buf, uint8_t op_code, const union cscp_sc_ctnl_pt_req_val* p_value)
{
    uint16_t status = BT_STS_SUCCESS;
    // uint8_t *p_buf_end = p_buf + CSCP_SC_CNTL_PT_REQ_MAX_LEN;

    // Set the operation code
    *p_buf++ = op_code;

    // Fulfill the message according to the operation code
    switch (op_code)
    {
        case (CSCP_CTNL_PT_OP_SET_CUMUL_VAL):
        {
            // Set the cumulative value
            *(uint32_t *)p_buf = co_host_to_uint16_le(p_value->cumul_val);
            p_buf += 4;
        } break;

        case (CSCP_CTNL_PT_OP_UPD_LOC):
        {
            // Set the sensor location
            *p_buf++ = p_value->sensor_loc;
        } break;

        case (CSCP_CTNL_PT_OP_RESERVED):
        case (CSCP_CTNL_PT_OP_START_CALIB):
        case (CSCP_CTNL_PT_OP_REQ_SUPP_LOC):
        {
            // Nothing more to do
        } break;

        default:
        {
            status = BT_STS_INVALID_PARM;
        } break;
    }

    return status;
}

__STATIC void cscpc_unpack_meas(uint8_t conidx, const uint8_t *packet, uint16_t length)
{
    const uint8_t *p_buf = packet;
    // const uint8_t *p_buf_end = p_buf + length;

    const cscpc_cb_t* p_cb = (const cscpc_cb_t*) p_cscpc_env->p_upper_cb;
    cscp_csc_meas_t meas;
    memset(&meas, 0, sizeof(cscp_csc_meas_t));

    // Flags
    meas.flags = *p_buf++;

    if (GETB(meas.flags, CSCP_MEAS_WHEEL_REV_DATA_PRESENT))
    {
        // Unpack Wheel Revolution Data (Cumulative Wheel & Last Wheel Event Time)
        meas.cumul_wheel_rev = CO_COMBINE_UINT32_LE(p_buf);
        p_buf += 4;

        meas.last_wheel_evt_time = CO_COMBINE_UINT16_LE(p_buf);
        p_buf += 2;
    }

    if (GETB(meas.flags, CSCP_MEAS_CRANK_REV_DATA_PRESENT))
    {
        // Unpack Crank Revolution Data (Cumulative Crank & Last Crank Event Time)
        meas.cumul_crank_rev = CO_COMBINE_UINT16_LE(p_buf);
        p_buf += 2;

        meas.last_crank_evt_time = CO_COMBINE_UINT16_LE(p_buf);
        p_buf += 2;
    }

    // Inform application about received vector
    p_cb->cb_meas(conidx, &meas);
}

__STATIC void cscpc_unpack_ctln_pt_rsp(uint8_t conidx, const uint8_t *packet, uint16_t length)
{
    const uint8_t *p_buf = packet;
    const uint8_t *p_buf_end = p_buf + length;
    bool valid = (length >= CSCP_SC_CNTL_PT_RSP_MIN_LEN);

    uint8_t op_code;
    uint8_t req_op_code;
    uint8_t resp_value;
    union cscp_sc_ctnl_pt_rsp_val value;
    memset(&value, 0, sizeof(union cscp_sc_ctnl_pt_rsp_val));

    // Response Op code
    op_code = *p_buf++;

    // Requested operation code
    req_op_code = *p_buf++;

    // Response value
    resp_value = *p_buf++;

    if(valid && (op_code == CSCP_CTNL_PT_RSP_CODE) && (req_op_code == p_cscpc_env->p_env[conidx]->ctrl_pt_op))
    {
        const cscpc_cb_t* p_cb = (const cscpc_cb_t*) p_cscpc_env->p_upper_cb;

        if (resp_value == CSCP_CTNL_PT_RESP_SUCCESS)
        {
            switch (req_op_code)
            {
                case (CSCP_CTNL_PT_OP_REQ_SUPP_LOC):
                {
                   // Location
                   value.supp_sensor_loc = 0;
                   while((p_buf_end - p_buf) > 0)
                   {
                       value.supp_sensor_loc |= (1 << (*p_buf++));
                   }
                } break;

                case (CSCP_CTNL_PT_OP_SET_CUMUL_VAL):
                case (CSCP_CTNL_PT_OP_START_CALIB):
                case (CSCP_CTNL_PT_OP_UPD_LOC):
                {
                    // No parameters
                } break;

                default:
                {

                } break;
            }
        } // else Response value is not success

        p_cscpc_env->p_env[conidx]->ctrl_pt_op = CSCP_CTNL_PT_OP_RESERVED;
        // stop timer
        cscpc_timer_stop(conidx);
        // provide control point response
        p_cb->cb_ctnl_pt_req_cmp(conidx, BT_STS_SUCCESS, req_op_code, resp_value, &value);
    }
}


uint16_t ble_cscpc_enable(uint8_t conidx)
{
    uint16_t conhdl = app_ble_get_conhdl_from_conidx(conidx);

    if (p_cscpc_env != NULL)
    {
        if (conidx < BLE_CONNECTION_MAX)
        {
            if (p_cscpc_env->p_env[conidx] == NULL)
            {
                p_cscpc_env->p_env[conidx] = (struct cscpc_cnx_env *)bes_bt_buf_malloc(sizeof(struct cscpc_cnx_env));
                if (p_cscpc_env->p_env[conidx] == NULL)
                {
                    return BT_STS_NO_RESOURCES;
                }
                memset(p_cscpc_env->p_env[conidx], 0, sizeof(struct cscpc_cnx_env));
                co_timer_new(&(p_cscpc_env->p_env[conidx]->timer_handle), CSCP_SP_TIMEOUT, cscpc_timer_handler, (void *)(uintptr_t)conidx, 1);

                gatt_prf_t *p_prf = gattc_get_profile(p_cscpc_env->gatt_prf_id, conhdl);
                if (p_prf == NULL)
                {
                    TRACE(0, "cscp invalid conn: %d %04x", p_cscpc_env->gatt_prf_id, conhdl);
                    bes_bt_buf_free(p_cscpc_env->p_env[conidx]);
                    p_cscpc_env->p_env[conidx] = NULL;
                    return BT_STS_INVALID_CONN_HANDLE;
                }

                uint16_t status = gattc_discover_service(p_prf, GATT_UUID_CSC_SERVICE, NULL);
                if (status != BT_STS_SUCCESS)
                {
                    bes_bt_buf_free(p_cscpc_env->p_env[conidx]);
                    p_cscpc_env->p_env[conidx] = NULL;
                    return status;
                }
                p_cscpc_env->p_env[conidx]->discover = true;
                p_cscpc_env->p_env[conidx]->ctrl_pt_op = CSCP_CTNL_PT_OP_RESERVED;
                return BT_STS_SUCCESS;
            }
            else
            {
                if (p_cscpc_env->p_env[conidx]->discover == true)
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

uint16_t ble_cscpc_read_sensor_feat(uint8_t conidx)
{
    uint16_t status = cscpc_read_val(conidx, CSCPC_RD_CSC_FEAT);
    return (status);
}

uint16_t ble_cscpc_read_sensor_loc(uint8_t conidx)
{
    uint16_t status = cscpc_read_val(conidx, CSCPC_RD_SENSOR_LOC);
    return (status);
}

uint16_t ble_cscpc_read_cfg(uint8_t conidx, uint8_t desc_code)
{
    uint16_t status;

    switch(desc_code)
    {
        case CSCPC_RD_WR_CSC_MEAS_CFG:
        case CSCPC_RD_WR_SC_CTNL_PT_CFG: { status = cscpc_read_val(conidx, desc_code);  } break;
        default:                         { status = BT_STS_INVALID_PARM;                } break;
    }

    return (status);
}

uint16_t ble_cscpc_write_cfg(uint8_t conidx, uint8_t desc_code, uint16_t cfg_val)
{
    uint16_t conhdl = app_ble_get_conhdl_from_conidx(conidx);
    if (conhdl == GAP_INVALID_CONN_HANDLE)
    {
        return BT_STS_INVALID_CONN_HANDLE;
    }

    if (p_cscpc_env != NULL)
    {
        if ((conidx < BLE_CONNECTION_MAX) && (p_cscpc_env->p_env[conidx] != NULL) && (!p_cscpc_env->p_env[conidx]->discover))
        {
            cscpc_cnx_env_t* p_con_env = p_cscpc_env->p_env[conidx];
            cscpc_cscs_content_t* p_cscs = &(p_con_env->cscs);
            gatt_peer_character_t *p_char = NULL;
            uint16_t cfg_en_val = 0;

            gatt_prf_t *p_prf = gattc_get_profile(p_cscpc_env->gatt_prf_id, conhdl);
            if (p_prf == NULL)
            {
                return BT_STS_FAILED;
            }

            switch(desc_code)
            {
                case CSCPC_RD_WR_CSC_MEAS_CFG:
                {
                    p_char = p_cscs->p_chars[CSCP_CSCS_CSC_MEAS_CHAR];
                    cfg_en_val =  PRF_CLI_START_NTF;
                } break;
                case CSCPC_RD_WR_SC_CTNL_PT_CFG:
                {
                    p_char = p_cscs->p_chars[CSCP_CSCS_SC_CTNL_PT_CHAR];
                    cfg_en_val =  PRF_CLI_START_IND;
                } break;
                default: break;
            }

            if (p_char == NULL)
            {
                return BT_STS_NOT_READY;
            }
            else if ((cfg_val != PRF_CLI_STOP_NTFIND) && (cfg_val != cfg_en_val))
            {
                return BT_STS_INVALID_PARM;
            }
            else
            {
                bool enable_ntf = (cfg_val == PRF_CLI_START_NTF ? true : false);
                bool enable_ind = (cfg_val == PRF_CLI_START_IND ? true : false);
                return gattc_write_cccd_descriptor(p_prf, p_char, enable_ntf, enable_ind);
            }
        }
    }

    return BT_STS_FAILED;
}

uint16_t ble_cscpc_ctnl_pt_req(uint8_t conidx, uint8_t req_op_code, const union cscp_sc_ctnl_pt_req_val* p_value)
{
    uint16_t conhdl = app_ble_get_conhdl_from_conidx(conidx);
    if (conhdl == GAP_INVALID_CONN_HANDLE)
    {
        return BT_STS_INVALID_CONN_HANDLE;
    }

    if (p_value == NULL)
    {
        return BT_STS_INVALID_PARM;
    }
    else if (p_cscpc_env != NULL)
    {
        if ((conidx < BLE_CONNECTION_MAX) && (p_cscpc_env->p_env[conidx] != NULL) && (!p_cscpc_env->p_env[conidx]->discover))
        {
            cscpc_cnx_env_t* p_con_env = p_cscpc_env->p_env[conidx];
            cscpc_cscs_content_t* p_cscs = &(p_con_env->cscs);
            gatt_peer_character_t *p_char = p_cscs->p_chars[CSCP_CSCS_SC_CTNL_PT_CHAR];
            gatt_prf_t *p_prf = gattc_get_profile(p_cscpc_env->gatt_prf_id, conhdl);
            if (p_prf == NULL || p_char == NULL)
            {
                return BT_STS_FAILED;
            }
            // reject if there is an ongoing control point operation
            else if (p_con_env->ctrl_pt_op != CSCP_CTNL_PT_OP_RESERVED)
            {
                return BT_STS_ONGOING;
            }
            else
            {
                uint8_t *p_buf = (uint8_t *)bes_bt_buf_malloc(CSCP_SC_CNTL_PT_REQ_MAX_LEN);
                if (p_buf == NULL)
                {
                    return BT_STS_NO_RESOURCES;
                }
                memset(p_buf, 0, CSCP_SC_CNTL_PT_REQ_MAX_LEN);

                if (cscpc_pack_ctnl_pt_req(p_buf, req_op_code, p_value) == BT_STS_SUCCESS)
                {
                    if (gattc_write_character_value(p_prf, p_char, p_buf, strlen((char *)p_buf)) == BT_STS_SUCCESS)
                    {
                        // save on-going operation
                        p_con_env->ctrl_pt_op = req_op_code;
                    }
                }

                bes_bt_buf_free(p_buf);
                return BT_STS_SUCCESS;
            }
        }
    }

    return BT_STS_FAILED;
}


static int app_ble_cscp_client_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param)
{
    uint8_t conidx = prf->con_idx;
    cscpc_cnx_env_t* p_con_env = NULL;
    cscpc_cscs_content_t* p_cscs = NULL;
    uint16_t status = 0;

    TRACE(0, "%s event=%d", __func__, event);

    if (p_cscpc_env == NULL)
    {
        return false;
    }

    p_con_env = p_cscpc_env->p_env[conidx];
    if (p_con_env == NULL)
    {
        /* not enable */
        return false;
    }
    p_cscs = &(p_con_env->cscs);

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
                if (s->service_uuid == GATT_UUID_CSC_SERVICE)
                {
                    p_cscs->p_service = s;
                    uint16_t gatt_chars[] = { GATT_CHAR_UUID_CSC_MEASURE,
                                              GATT_CHAR_UUID_CSC_FEATURE,
                                              GATT_CHAR_UUID_SENS_OR_LOCATION,
                                              GATT_CHAR_UUID_SC_CTRL_POINT };
                    gattc_discover_multi_characters(prf, s, gatt_chars, sizeof(gatt_chars)/sizeof(uint16_t));
                }
            }
            p_con_env->nb_svc++;
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

            if (p->char_uuid == GATT_CHAR_UUID_CSC_MEASURE)
            {
                p_cscs->p_chars[CSCP_CSCS_CSC_MEAS_CHAR] = p->character;
                if ((p->char_prop & GATT_NTF_PROP) != 0)
                {
                    gattc_write_cccd_descriptor(prf, p->character, true, false);
                }
            }
            else if (p->char_uuid == GATT_CHAR_UUID_CSC_FEATURE)
            {
                p_cscs->p_chars[CSCP_CSCS_CSC_FEAT_CHAR] = p->character;
            }
            else if (p->char_uuid == GATT_CHAR_UUID_SENS_OR_LOCATION)
            {
                p_cscs->p_chars[CSCP_CSCS_SENSOR_LOC_CHAR] = p->character;
            }
            else if (p->char_uuid == GATT_CHAR_UUID_SC_CTRL_POINT)
            {
                p_cscs->p_chars[CSCP_CSCS_SC_CTNL_PT_CHAR] = p->character;
                if ((p->char_prop & GATT_IND_PROP) != 0)
                {
                    gattc_write_cccd_descriptor(prf, p->character, false, true);
                }
            }

            if (p->discover_cmpl)
            {
                p_cscpc_env->p_upper_cb->cb_enable_cmp(conidx, status);
                p_con_env->discover = false;
            }
        } break;
        case GATT_PROF_EVENT_CHAR_READ_RSP:
        {
            gatt_profile_char_read_rsp_t *p = param.char_read_rsp;
            status = p->error_code;
            if (p->character == p_cscs->p_chars[CSCP_CSCS_CSC_FEAT_CHAR])
            {
                uint16_t sensor_feat = (status == ATT_ERROR_NO_ERROR ? CO_COMBINE_UINT16_LE(p->value) : 0);
                p_cscpc_env->p_upper_cb->cb_read_sensor_feat_cmp(conidx, status, sensor_feat);
            }
            else if (p->character == p_cscs->p_chars[CSCP_CSCS_SENSOR_LOC_CHAR])
            {
                uint8_t sensor_loc = (status == ATT_ERROR_NO_ERROR ? *(p->value) : 0);
                p_cscpc_env->p_upper_cb->cb_read_sensor_loc_cmp(conidx, status, sensor_loc);
            }
        } break;
        case GATT_PROF_EVENT_DESC_READ_RSP:
        {
            uint8_t desc_code = 0;
            gatt_profile_desc_read_rsp_t *p = param.desc_read_rsp;
            status = p->error_code;
            if (p->character == p_cscs->p_chars[CSCP_CSCS_CSC_MEAS_CHAR])
            {
                desc_code = CSCPC_RD_WR_CSC_MEAS_CFG;
            }
            else if (p->character == p_cscs->p_chars[CSCP_CSCS_SC_CTNL_PT_CHAR])
            {
                desc_code = CSCPC_RD_WR_SC_CTNL_PT_CFG;
            }
            uint16_t cfg_val = (status == ATT_ERROR_NO_ERROR ? CO_COMBINE_UINT16_LE(p->value) : 0);
            p_cscpc_env->p_upper_cb->cb_read_cfg_cmp(conidx, status, desc_code, cfg_val);
        } break;
        case GATT_PROF_EVENT_CHAR_WRITE_RSP:
        {
            gatt_profile_char_write_rsp_t *p = param.char_write_rsp;
            status = p->error_code;
            if(status != ATT_ERROR_NO_ERROR)
            {
                uint8_t opcode = p_cscpc_env->p_env[conidx]->ctrl_pt_op;
                p_cscpc_env->p_env[conidx]->ctrl_pt_op = CSCP_CTNL_PT_OP_RESERVED;
                p_cscpc_env->p_upper_cb->cb_ctnl_pt_req_cmp(conidx, status, opcode, 0, NULL);
            }
            else
            {
                // Start Timeout Procedure - wait for Indication reception
                cscpc_timer_start(conidx);
            }
        } break;
        case GATT_PROF_EVENT_DESC_WRITE_RSP:
        {
            uint8_t desc_code;
            gatt_profile_desc_write_rsp_t *p = param.desc_write_rsp;
            status = p->error_code;
            if (p->character == p_cscs->p_chars[CSCP_CSCS_CSC_MEAS_CHAR])
            {
                desc_code = CSCPC_RD_WR_CSC_MEAS_CFG;
            }
            else if (p->character == p_cscs->p_chars[CSCP_CSCS_SC_CTNL_PT_CHAR])
            {
                desc_code = CSCPC_RD_WR_SC_CTNL_PT_CFG;
            }
            else
            {
                TRACE(0, "ERROR: invalid param");
                break;
            }
            p_cscpc_env->p_upper_cb->cb_write_cfg_cmp(conidx, status, desc_code);
        } break;
        case GATT_PROF_EVENT_NOTIFY:
        {
            gatt_profile_recv_notify_t *ntf = param.notify;
            if (ntf->character == p_cscs->p_chars[CSCP_CSCS_CSC_MEAS_CHAR])
            {
                // Unpack measurement
                cscpc_unpack_meas(conidx, ntf->value, ntf->value_len);
            }
            else if (ntf->character == p_cscs->p_chars[CSCP_CSCS_SC_CTNL_PT_CHAR])
            {
                // Unpack control point
                cscpc_unpack_ctln_pt_rsp(conidx, ntf->value, ntf->value_len);
                if (ntf->is_indicate == true)
                {
                    // Send cfm
                }
                else
                {
                    TRACE(0, "ERROR: not ind");
                }
            }
        } break;
        case GATT_PROF_EVENT_OPENED:
        {
            p_cscs->prf = *prf;
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

uint16_t ble_cscpc_init(const cscpc_cb_t* p_cb)
{
    uint8_t cscpc_prf_lid = GATT_PRF_INVALID;

    if (p_cscpc_env != NULL)
    {
        return BT_STS_IN_USE;
    }

    if (p_cb == NULL ||
        p_cb->cb_enable_cmp == NULL ||
        p_cb->cb_read_sensor_feat_cmp == NULL ||
        p_cb->cb_read_sensor_loc_cmp == NULL ||
        p_cb->cb_read_cfg_cmp == NULL ||
        p_cb->cb_write_cfg_cmp == NULL ||
        p_cb->cb_meas == NULL ||
        p_cb->cb_ctnl_pt_req_cmp == NULL)
    {
        return BT_STS_INVALID_PARM;
    }

    gattc_cfg_t prf_cfg = {0};
    // prf_cfg.prf_size = sizeof(app_ble_cscp_prf_t);
    cscpc_prf_lid = gattc_register_profile(app_ble_cscp_client_callback, &prf_cfg);
    if (cscpc_prf_lid == GATT_PRF_INVALID)
    {
        return BT_STS_FAILED;
    }

    // Malloc for environment
    p_cscpc_env = (cscpc_env_t *)bes_bt_buf_malloc(sizeof(cscpc_env_t));
    if (p_cscpc_env == NULL)
    {
        gattc_unregister_profile(cscpc_prf_lid);
        return BT_STS_NO_RESOURCES;
    }
    memset(p_cscpc_env, 0, sizeof(cscpc_env_t));

    p_cscpc_env->p_upper_cb = p_cb;
    p_cscpc_env->gatt_prf_id = cscpc_prf_lid;

    return BT_STS_SUCCESS;
}

uint16_t ble_cscpc_deinit()
{
    uint8_t conidx;

    if (p_cscpc_env == NULL)
    {
        return BT_STS_FAILED;
    }

    if (p_cscpc_env->gatt_prf_id != GAF_INVALID_PRF_LID)
    {
        gattc_unregister_profile(p_cscpc_env->gatt_prf_id);
    }

    // cleanup environment variable for each task instances
    for (conidx = 0; conidx < BLE_CONNECTION_MAX; conidx++)
    {
        if (p_cscpc_env->p_env[conidx] != NULL)
        {
            co_timer_del(&(p_cscpc_env->p_env[conidx]->timer_handle));
            p_cscpc_env->p_env[conidx]->timer_handle = 0;

            bes_bt_buf_free(p_cscpc_env->p_env[conidx]);
            p_cscpc_env->p_env[conidx] = NULL;
        }
    }

    bes_bt_buf_free(p_cscpc_env);
    p_cscpc_env = NULL;

    return BT_STS_SUCCESS;
}

#endif