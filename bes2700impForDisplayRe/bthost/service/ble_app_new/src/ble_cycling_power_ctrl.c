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
#ifdef BLE_CPC_ENABLED

/* INCLUDE */
#include "gatt_service.h"
#include "app_ble.h"
#include "bes_me_api.h"
#include "bt_common_define.h"

#include "ble_cycling_power_ctrl.h"

/* DEFINES */
typedef struct ble_cpc_connection
{
    /// Control point timer
    uint8_t             timer;
    /// Peer database discovered handle mapping
    gatt_peer_character_t *chars[CPS_CHAR_MAX];
    /// Client is in discovering state
    bool                discover;
    /// Control point operation on-going (see enum #cps_ctrl_pt_code)
    uint8_t             ctrl_pt_op;
} ble_cpc_conn_t;

/// Client environment variable
typedef struct ble_cpc_env
{
    /// Callbacks
    const ble_cpc_cb_t  *p_cbs;
    /// Environment variable pointer for each connections
    ble_cpc_conn_t       *p_env[BLE_CONNECTION_MAX];
    /// GATT Profile local identifier
    uint8_t              prf_lid;
} ble_cpc_env_t;

static ble_cpc_env_t *p_cpc_env = NULL;

/* External Functions */
extern uint8_t ble_prf_unpack_date_time(const uint8_t *p_buf, prf_date_time_t *p_date_time);

/* Internal Functions */
static uint8_t ble_cpc_char_uuid_to_char_type(uint16_t char_uuid)
{
    if (char_uuid == GATT_CHAR_UUID_CYCL_PWR_MEASURE)
    {
        return CPS_MEAS_CHAR;
    }
    else if (char_uuid == GATT_CHAR_UUID_CYCL_PWR_FEATURE)
    {
        return CPS_FEAT_CHAR;
    }
    else if (char_uuid == GATT_CHAR_UUID_CYCL_PWR_VECTOR)
    {
        return CPS_VECTOR_CHAR;
    }
    else if (char_uuid == GATT_CHAR_UUID_CYCL_PWR_CTRL_POINT)
    {
        return CPS_CTRL_PT_CHAR;
    }

    return CPS_CHAR_MAX;
}

static uint8_t ble_cpc_char_uuid_to_val_id(uint16_t char_uuid, bool is_desc, bool is_sccd)
{
    uint8_t char_type = ble_cpc_char_uuid_to_char_type(char_uuid);

    if (is_desc == false)
    {
        return char_type;
    }
    else
    {
        if (char_type == CPS_MEAS_CHAR)
        {
            return BLE_CPC_RD_WR_CP_MEAS_CL_CFG + is_sccd;
        }
        else if (char_type == CPS_VECTOR_CHAR)
        {
            return BLE_CPC_RD_WR_VECTOR_CFG;
        }
        else if (char_type == CPS_CTRL_PT_CHAR)
        {
            return BLE_CPC_RD_WR_CTRL_PT_CFG;
        }
    }

    return 0xFF;
}

static void ble_cpc_read_val_cmp(uint8_t conidx, uint16_t status, uint8_t val_id, const uint8_t *p_data, uint8_t buf_len)
{
    if (p_cpc_env != NULL)
    {
        const ble_cpc_cb_t *p_cb = (const ble_cpc_cb_t *) p_cpc_env->p_cbs;
        switch (val_id)
        {
            // Read CP Feature Characteristic value
            case (BLE_CPC_RD_CP_FEAT):
            {
                uint32_t sensor_feat = 0;
                if (status == BT_STS_SUCCESS)
                {
                    sensor_feat = CO_COMBINE_UINT32_LE(p_data);
                }
                p_cb->cb_read_sensor_feat_cmp(conidx, status, sensor_feat);
            }
            break;

            // Read Sensor Location Characteristic value
            case (BLE_CPC_RD_SENSOR_LOC):
            {
                uint8_t sensor_loc = 0;

                if (status == BT_STS_SUCCESS)
                {
                    sensor_loc = p_data[0];
                }

                p_cb->cb_read_sensor_loc_cmp(conidx, status, sensor_loc);

            }
            break;

            // Read Client Characteristic Configuration Descriptor value
            case (BLE_CPC_RD_WR_CP_MEAS_CL_CFG):
            case (BLE_CPC_RD_WR_CP_MEAS_SV_CFG):
            case (BLE_CPC_RD_WR_VECTOR_CFG):
            case (BLE_CPC_RD_WR_CTRL_PT_CFG):
            {
                uint16_t cfg_val = 0;

                if (status == BT_STS_SUCCESS)
                {
                    cfg_val = CO_COMBINE_UINT16_LE(p_data);
                }

                p_cb->cb_read_cfg_cmp(conidx, status, val_id, cfg_val);
            }
            break;

            default:
            {
                ASSERT_ERR(0);
            }
            break;
        }
    }
}

static uint16_t ble_cpc_read_val(uint8_t conidx, uint16_t val_id)
{
    gatt_prf_t *p_prf = gattc_get_profile(p_cpc_env->prf_lid,
                                          app_ble_get_conhdl_from_conidx(conidx));

    uint16_t status = ATT_ERROR_RD_NOT_PERMITTED;

    if ((conidx < BLE_CONNECTION_MAX) && (p_cpc_env->p_env[conidx] != NULL) && (!p_cpc_env->p_env[conidx]->discover))
    {
        ble_cpc_conn_t *p_con_env = p_cpc_env->p_env[conidx];
        gatt_peer_character_t *p_char = NULL;
        uint16_t desc_uuid = 0;

        switch (val_id)
        {
            case BLE_CPC_RD_CP_FEAT:
            {
                p_char = p_con_env->chars[CPS_FEAT_CHAR];
            }
            break;
            case BLE_CPC_RD_SENSOR_LOC:
            {
                p_char = p_con_env->chars[CPS_SENSOR_LOC_CHAR];
            }
            break;
            case BLE_CPC_RD_WR_CP_MEAS_CL_CFG:
            {
                desc_uuid = GATT_DESC_UUID_CHAR_CLIENT_CONFIG;
                p_char = p_con_env->chars[CPS_MEAS_CHAR];
            }
            break;
            case BLE_CPC_RD_WR_CP_MEAS_SV_CFG:
            {
                desc_uuid = GATT_DESC_UUID_CHAR_SERVER_CONFIG;
                p_char = p_con_env->chars[CPS_MEAS_CHAR];
            }
            break;
            case BLE_CPC_RD_WR_VECTOR_CFG:
            {
                desc_uuid = GATT_DESC_UUID_CHAR_CLIENT_CONFIG;
                p_char = p_con_env->chars[CPS_VECTOR_CHAR];
            }
            break;
            case BLE_CPC_RD_WR_CTRL_PT_CFG:
            {
                desc_uuid = GATT_DESC_UUID_CHAR_CLIENT_CONFIG;
                p_char = p_con_env->chars[CPS_CTRL_PT_CHAR];
            }
            break;
            default:
            { } break;
        }

        if (p_char != NULL)
        {
            if (desc_uuid != 0)
            {
                status = gattc_read_descriptor_value(p_prf, p_char, desc_uuid);
            }
            else
            {
                status = gattc_read_character_value(p_prf, p_char);
            }
        }
    }

    return status;
}

static uint8_t ble_cpc_pack_ctrl_pt_req(uint8_t *p_buf, uint8_t op_code, const union ble_cps_ctrl_pt_req_val *p_value)
{
    uint8_t *p_buf_head = p_buf;
    // Set the operation code
    p_buf[0] = op_code;
    p_buf += 1;

    // Fulfill the message according to the operation code
    switch (op_code)
    {
        case (CPS_CTRL_PT_SET_CUMUL_VAL):
        {
            // Set the cumulative value
            *(uint32_t *)p_buf = co_host_to_uint32_le(p_value->cumul_val);
            p_buf += 4;
        }
        break;

        case (CPS_CTRL_PT_UPD_SENSOR_LOC):
        {
            // Set the sensor location
            p_buf[0] = p_value->sensor_loc;
            p_buf += 1;
        }
        break;

        case (CPS_CTRL_PT_SET_CRANK_LENGTH):
        {
            // Set the crank length
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_value->crank_length);
            p_buf += 2;
        }
        break;

        case (CPS_CTRL_PT_SET_CHAIN_LENGTH):
        {
            // Set the chain length
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_value->chain_length);
            p_buf += 2;
        }
        break;

        case (CPS_CTRL_PT_SET_CHAIN_WEIGHT):
        {
            // Set the chain weight
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_value->chain_weight);
            p_buf += 2;
        }
        break;

        case (CPS_CTRL_PT_SET_SPAN_LENGTH):
        {
            // Set the span length
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_value->span_length);
            p_buf += 2;
        }
        break;

        case (CPS_CTRL_MASK_CP_MEAS_CH_CONTENT):
        {
            // Set the Content Mask
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_value->mask_content);
            p_buf += 2;
        }
        break;

        case (CPS_CTRL_PT_REQ_SUPP_SENSOR_LOC):
        case (CPS_CTRL_PT_REQ_CRANK_LENGTH):
        case (CPS_CTRL_PT_REQ_CHAIN_LENGTH):
        case (CPS_CTRL_PT_REQ_CHAIN_WEIGHT):
        case (CPS_CTRL_PT_REQ_SPAN_LENGTH):
        case (CPS_CTRL_PT_START_OFFSET_COMP):
        case (CPS_CTRL_REQ_SAMPLING_RATE):
        case (CPS_CTRL_REQ_FACTORY_CALIBRATION_DATE):
        case (CPS_CTRL_START_ENHANCED_OFFSET_COMP):
        {
            // Nothing more to do
        } break;

        default:
        { } break;
    }

    return p_buf_head - p_buf;
}

static uint8_t ble_cpc_unpack_meas(uint8_t conidx, const uint8_t *p_buf)
{
    const uint8_t *p_buf_head = p_buf;
    const ble_cpc_cb_t *p_cb = (const ble_cpc_cb_t *) p_cpc_env->p_cbs;
    ble_cps_cp_meas_t meas;
    memset(&meas, 0, sizeof(ble_cps_cp_meas_t));

    // Flags
    meas.flags = CO_COMBINE_UINT16_LE(p_buf);
    p_buf += 2;
    // Instant power
    meas.inst_power = CO_COMBINE_UINT16_LE(p_buf);
    p_buf += 2;

    if (GETB(meas.flags, CPS_MEAS_PEDAL_POWER_BALANCE_PRESENT))
    {
        // Unpack Pedal Power Balance info
        meas.pedal_power_balance = p_buf[0];
        p_buf++;
    }

    if (GETB(meas.flags, CPS_MEAS_ACCUM_TORQUE_PRESENT))
    {
        // Unpack Accumulated Torque info
        meas.accum_torque = CO_COMBINE_UINT16_LE(p_buf);
        p_buf += 2;
    }

    if (GETB(meas.flags, CPS_MEAS_WHEEL_REV_DATA_PRESENT))
    {
        // Unpack Wheel Revolution Data (Cumulative Wheel & Last Wheel Event Time)
        meas.cumul_wheel_rev = CO_COMBINE_UINT32_LE(p_buf);
        p_buf += 4;
        meas.last_wheel_evt_time = CO_COMBINE_UINT16_LE(p_buf);
        p_buf += 2;
    }

    if (GETB(meas.flags, CPS_MEAS_CRANK_REV_DATA_PRESENT))
    {
        // Unpack Crank Revolution Data (Cumulative Crank & Last Crank Event Time)
        meas.cumul_crank_rev = CO_COMBINE_UINT16_LE(p_buf);
        p_buf += 2;
        meas.last_crank_evt_time = CO_COMBINE_UINT16_LE(p_buf);
        p_buf += 2;
    }

    if (GETB(meas.flags, CPS_MEAS_EXTREME_FORCE_MAGNITUDES_PRESENT))
    {
        // Unpack Extreme Force Magnitudes (Maximum Force Magnitude & Minimum Force Magnitude)
        meas.max_force_magnitude = CO_COMBINE_UINT16_LE(p_buf);
        p_buf += 2;
        meas.min_force_magnitude = CO_COMBINE_UINT16_LE(p_buf);
        p_buf += 2;
    }

    else if (GETB(meas.flags, CPS_MEAS_EXTREME_TORQUE_MAGNITUDES_PRESENT))
    {
        // Unpack Extreme Force Magnitudes (Maximum Force Magnitude & Minimum Force Magnitude)
        meas.max_torque_magnitude = CO_COMBINE_UINT16_LE(p_buf);
        p_buf += 2;
        meas.min_torque_magnitude = CO_COMBINE_UINT16_LE(p_buf);
        p_buf += 2;
    }

    if (GETB(meas.flags, CPS_MEAS_EXTREME_ANGLES_PRESENT))
    {
        // Unpack Extreme Angles (Maximum Angle & Minimum Angle)
        uint32_t angle = CO_COMBINE_UINT24_LE(p_buf);
        p_buf += 3;

        //Force to 12 bits
        meas.max_angle = (angle & (0x0FFF));
        meas.min_angle = ((angle >> 12) & 0x0FFF);
    }

    if (GETB(meas.flags, CPS_MEAS_TOP_DEAD_SPOT_ANGLE_PRESENT))
    {
        // Unpack Top Dead Spot Angle
        meas.top_dead_spot_angle = CO_COMBINE_UINT16_LE(p_buf);
        p_buf += 2;
    }

    if (GETB(meas.flags, CPS_MEAS_BOTTOM_DEAD_SPOT_ANGLE_PRESENT))
    {
        // Unpack Bottom Dead Spot Angle
        meas.bot_dead_spot_angle = CO_COMBINE_UINT16_LE(p_buf);
        p_buf += 2;
    }

    if (GETB(meas.flags, CPS_MEAS_ACCUM_ENERGY_PRESENT))
    {
        // Unpack Accumulated Energy
        meas.accum_energy = CO_COMBINE_UINT16_LE(p_buf);
        p_buf += 2;
    }

    // Inform application about received vector
    p_cb->cb_meas(conidx, &meas);

    return p_buf - p_buf_head;
}

static uint8_t ble_cpc_unpack_vector(uint8_t conidx, const uint8_t *p_buf, uint8_t buf_len)
{
    const uint8_t *p_buf_head = p_buf;
    const ble_cpc_cb_t *p_cb = (const ble_cpc_cb_t *) p_cpc_env->p_cbs;
    ble_cps_cp_vector_t vector;
    memset(&vector, 0, sizeof(ble_cps_cp_vector_t));

    // Flags
    vector.flags = p_buf[0];
    p_buf += 1;

    if (GETB(vector.flags, CPS_VECTOR_CRANK_REV_DATA_PRESENT))
    {
        // Unpack Crank Revolution Data
        vector.cumul_crank_rev = CO_COMBINE_UINT16_LE(p_buf);
        p_buf += 2;
        // Unpack Last Crank Evt time
        vector.last_crank_evt_time = CO_COMBINE_UINT16_LE(p_buf);
        p_buf += 2;
    }

    if (GETB(vector.flags, CPS_VECTOR_FIRST_CRANK_MEAS_ANGLE_PRESENT))
    {
        // Unpack First Crank Measurement Angle
        vector.first_crank_meas_angle = CO_COMBINE_UINT16_LE(p_buf);
        p_buf += 2;
    }

    if (!GETB(vector.flags, CPS_VECTOR_INST_FORCE_MAGNITUDE_ARRAY_PRESENT) !=
            !GETB(vector.flags, CPS_VECTOR_INST_TORQUE_MAGNITUDE_ARRAY_PRESENT))
    {
        // Unpack Force or Torque magnitude (mutually excluded)

        while ((buf_len - (p_buf - p_buf_head) >= 2) && (vector.nb < CPS_MAX_TORQUE_NB))
        {
            // Handle the array buffer to extract parameters
            vector.force_torque_magnitude[vector.nb] = CO_COMBINE_UINT16_LE(p_buf);
            p_buf += 2;
            vector.nb += 1;
        }
    }

    // Inform application about received vector
    p_cb->cb_vector(conidx, &vector);

    return p_buf - p_buf_head;
}

static uint8_t ble_cpc_unpack_ctrl_pt_rsp(uint8_t conidx, const uint8_t *p_buf, uint8_t buf_len)
{
    bool valid = (buf_len >= CPS_CP_CTRL_PT_RSP_MIN_LEN);

    const uint8_t *p_buf_head = p_buf;

    uint8_t op_code;
    uint8_t req_op_code;
    uint8_t resp_value;
    union ble_cps_ctrl_pt_rsp_val value;
    memset(&value, 0, sizeof(union ble_cps_ctrl_pt_rsp_val));

    // Response Op code
    op_code = p_buf[0];
    p_buf += 1;

    // Requested operation code
    req_op_code = p_buf[0];
    p_buf += 1;

    // Response value
    resp_value = p_buf[0];
    p_buf += 1;

    if (valid && (op_code == CPS_CTRL_PT_RSP_CODE) && (req_op_code == p_cpc_env->p_env[conidx]->ctrl_pt_op))
    {
        const ble_cpc_cb_t *p_cb = (const ble_cpc_cb_t *) p_cpc_env->p_cbs;

        if (resp_value == CPS_CTRL_PT_RESP_SUCCESS)
        {
            switch (req_op_code)
            {
                case (CPS_CTRL_PT_REQ_SUPP_SENSOR_LOC):
                {
                    // Location
                    value.supp_sensor_loc = 0;
                    while (buf_len - (p_buf - p_buf_head) > 0)
                    {
                        value.supp_sensor_loc |= (1 << p_buf[0]);
                        p_buf += 1;
                    }
                }
                break;

                case (CPS_CTRL_PT_REQ_CRANK_LENGTH):
                {
                    value.crank_length = CO_COMBINE_UINT16_LE(p_buf);
                    p_buf += 2;
                }
                break;

                case (CPS_CTRL_PT_REQ_CHAIN_LENGTH):
                {
                    value.chain_length = CO_COMBINE_UINT16_LE(p_buf);
                    p_buf += 2;
                }
                break;

                case (CPS_CTRL_PT_REQ_CHAIN_WEIGHT):
                {
                    value.chain_weight = CO_COMBINE_UINT16_LE(p_buf);
                    p_buf += 2;
                }
                break;

                case (CPS_CTRL_PT_REQ_SPAN_LENGTH):
                {
                    value.span_length = CO_COMBINE_UINT16_LE(p_buf);
                    p_buf += 2;
                }
                break;

                case (CPS_CTRL_PT_START_OFFSET_COMP):
                {
                    value.offset_comp = CO_COMBINE_UINT16_LE(p_buf);
                    p_buf += 2;
                }
                break;

                case (CPS_CTRL_REQ_SAMPLING_RATE):
                {
                    value.sampling_rate = p_buf[0];
                    p_buf += 2;
                }
                break;

                case (CPS_CTRL_REQ_FACTORY_CALIBRATION_DATE):
                {
                    p_buf += ble_prf_unpack_date_time(p_buf, &(value.factory_calibration));
                }
                break;

                case (CPS_CTRL_START_ENHANCED_OFFSET_COMP):
                {
                    uint8_t i;
                    value.enhanced_offset_comp.comp_offset = CO_COMBINE_UINT16_LE(p_buf);
                    p_buf += 2;

                    value.enhanced_offset_comp.manu_comp_id = CO_COMBINE_UINT16_LE(p_buf);
                    p_buf += 2;

                    value.enhanced_offset_comp.length = co_min(CPS_MAX_MANF_DATA_LEN, p_buf[0]);
                    p_buf += 1;

                    for (i = 0; i < value.enhanced_offset_comp.length; i++)
                    {
                        value.enhanced_offset_comp.data[i] = p_buf[0];
                        p_buf += 1;
                    }
                }
                break;

                case (CPS_CTRL_PT_SET_CUMUL_VAL):
                case (CPS_CTRL_PT_UPD_SENSOR_LOC):
                case (CPS_CTRL_PT_SET_CRANK_LENGTH):
                case (CPS_CTRL_PT_SET_CHAIN_LENGTH):
                case (CPS_CTRL_PT_SET_CHAIN_WEIGHT):
                case (CPS_CTRL_PT_SET_SPAN_LENGTH):
                case (CPS_CTRL_MASK_CP_MEAS_CH_CONTENT):
                {
                    // No parameters
                } break;

                default:
                {

                } break;
            }
        }
        // Response value is not success
        else
        {
            // Start Enhanced Offset Compensation Op code
            if (req_op_code == CPS_CTRL_START_ENHANCED_OFFSET_COMP)
            {
                // obtain response parameter
                value.enhanced_offset_comp.rsp_param = p_buf[0];
                p_buf += 1;

                if (value.enhanced_offset_comp.rsp_param == CPS_CTRL_PT_ERR_RSP_PARAM_MANUF_SPEC_ERR_FOLLOWS)
                {
                    uint8_t i;

                    // obtain manufacturer compensation id
                    value.enhanced_offset_comp.manu_comp_id = CO_COMBINE_UINT16_LE(p_buf);
                    p_buf += 2;

                    value.enhanced_offset_comp.length = co_min(CPS_MAX_MANF_DATA_LEN, p_buf[0]);
                    p_buf += 1;

                    for (i = 0; i < value.enhanced_offset_comp.length; i++)
                    {
                        value.enhanced_offset_comp.data[i] = p_buf[0];
                        p_buf += 1;
                    }
                }
                // else response value is CPS_CTRL_PT_RESP_INV_PARAM
            }
        }

        p_cpc_env->p_env[conidx]->ctrl_pt_op = CPS_CTRL_PT_RESERVED;
        // stop timer
        co_timer_stop(&(p_cpc_env->p_env[conidx]->timer));
        // provide control point response
        p_cb->cb_ctrl_pt_req_cmp(conidx, BT_STS_SUCCESS, req_op_code, resp_value, &value);
    }

    return p_buf - p_buf_head;
}

static void ble_cpc_timer_handler(void *args)
{
    uint32_t conidx = (uint32_t)args;
    // Get the address of the environment
    if (p_cpc_env != NULL)
    {
        ble_cpc_conn_t *p_con_env = p_cpc_env->p_env[conidx];
        ASSERT_ERR(p_con_env != NULL);
        if (p_con_env->ctrl_pt_op != CPS_CTRL_PT_RESERVED)
        {
            const ble_cpc_cb_t *p_cb = (const ble_cpc_cb_t *) p_cpc_env->p_cbs;
            uint8_t op_code = p_con_env->ctrl_pt_op;
            p_con_env->ctrl_pt_op = CPS_CTRL_PT_RESERVED;

            p_cb->cb_ctrl_pt_req_cmp((uint8_t)conidx, ATT_ERROR_RESPONSE_TIMEOUT, op_code, 0, NULL);
        }
    }
}

static void ble_cpc_con_create(uint8_t conidx)
{
    // Nothing to do
}

static void ble_cpc_con_cleanup(uint8_t conidx)
{
    // clean-up environment variable allocated for task instance
    if (p_cpc_env->p_env[conidx] != NULL)
    {
        co_timer_stop(&(p_cpc_env->p_env[conidx]->timer));
        bes_bt_me_bes_bt_buf_free((uint8_t *)p_cpc_env->p_env[conidx]);
        p_cpc_env->p_env[conidx] = NULL;
    }
}

static void ble_cpc_discover_cmp_cb(uint8_t conidx, uint16_t status)
{
    if (status != ATT_ERROR_NO_ERROR)
    {
        ble_cpc_con_cleanup(conidx);
    }
    else
    {
        p_cpc_env->p_env[conidx]->discover = false;
    }

    // Get the address of the environment
    if (p_cpc_env != NULL)
    {
        p_cpc_env->p_cbs->cb_svc_discover_cmp(conidx, status);
    }
}

static void ble_cpc_att_val_cb(uint8_t conidx, gatt_peer_character_t *p_char, bool is_desc, bool is_sccd, const uint8_t *p_buf, uint8_t buf_len)
{
    gatt_peer_character_uuid_t uuid;
    gattc_get_character_uuid(p_char, &uuid);
    uint8_t val_id = ble_cpc_char_uuid_to_val_id(uuid.char_uuid, is_desc, is_sccd);
    ble_cpc_read_val_cmp(conidx, BT_STS_SUCCESS, val_id, p_buf, buf_len);
}

static void ble_cpc_read_cmp_cb(uint8_t conidx, gatt_peer_character_t *p_char, uint16_t status, bool is_desc, bool is_sccd)
{
    if (status != BT_STS_SUCCESS)
    {
        gatt_peer_character_uuid_t uuid;
        gattc_get_character_uuid(p_char, &uuid);
        uint8_t val_id = ble_cpc_char_uuid_to_val_id(uuid.char_uuid, is_desc, is_sccd);
        ble_cpc_read_val_cmp(conidx, status, val_id, NULL, 0);
    }
}

static void ble_cpc_write_cmp_cb(uint8_t conidx, gatt_peer_character_t *p_char, uint16_t status, bool is_desc, bool is_sccd)
{
    if (p_cpc_env != NULL)
    {
        const ble_cpc_cb_t *p_cb = (const ble_cpc_cb_t *) p_cpc_env->p_cbs;
        gatt_peer_character_uuid_t uuid;
        gattc_get_character_uuid(p_char, &uuid);
        uint8_t val_id = ble_cpc_char_uuid_to_val_id(uuid.char_uuid, is_desc, is_sccd);

        switch (val_id)
        {
            // Config control
            case BLE_CPC_RD_WR_CP_MEAS_CL_CFG:
            case BLE_CPC_RD_WR_CP_MEAS_SV_CFG:
            case BLE_CPC_RD_WR_VECTOR_CFG:
            case BLE_CPC_RD_WR_CTRL_PT_CFG:
            {
                p_cb->cb_write_cfg_cmp(conidx, status, val_id);
            }
            break;
            // Control point commands
            case BLE_CPC_IND_CTRL_PT:
            {
                if (status != BT_STS_SUCCESS)
                {
                    uint8_t opcode = p_cpc_env->p_env[conidx]->ctrl_pt_op;
                    p_cpc_env->p_env[conidx]->ctrl_pt_op = CPS_CTRL_PT_RESERVED;
                    p_cb->cb_ctrl_pt_req_cmp(conidx, status, opcode, 0, NULL);
                }
                else
                {
                    // Start Timeout Procedure - wait for Indication reception
                    co_timer_start(&(p_cpc_env->p_env[conidx]->timer));
                }
            }
            break;

            default: { /* Nothing to do */} break;
        }
    }
}

static void ble_cpc_att_val_evt_cb(uint8_t conidx, gatt_peer_character_t *p_char, const uint8_t *p_data, uint8_t buf_len)
{
    if (p_cpc_env != NULL)
    {
        ble_cpc_conn_t *p_con_env = p_cpc_env->p_env[conidx];

        if (p_char == p_con_env->chars[CPS_MEAS_CHAR])
        {
            //Unpack measurement
            ble_cpc_unpack_meas(conidx, p_data);
        }
        else if (p_char == p_con_env->chars[CPS_VECTOR_CHAR])
        {
            //Unpack vector
            ble_cpc_unpack_vector(conidx, p_data, buf_len);
        }
        else if (p_char == p_con_env->chars[CPS_CTRL_PT_CHAR])
        {
            // Unpack control point
            ble_cpc_unpack_ctrl_pt_rsp(conidx, p_data, buf_len);
        }
    }
}

static void ble_cpc_service_disocver_cb(gatt_prf_t *prf, uint8_t conidx, gatt_peer_service_t *p_svc, uint16_t status)
{
    if (p_cpc_env->p_env[conidx] == NULL)
    {
        return;
    }

    if (status != ATT_ERROR_NO_ERROR)
    {
        ble_cpc_discover_cmp_cb(conidx, status);
        return;
    }

    gattc_discover_all_characters(prf, p_svc);
}

static void ble_cpc_character_disocver_cb(gatt_prf_t *prf, uint8_t conidx, uint16_t char_uuid, gatt_peer_character_t *p_char, uint16_t status, bool cmpl)
{
    if (p_cpc_env->p_env[conidx] == NULL)
    {
        return;
    }

    if (status != ATT_ERROR_NO_ERROR)
    {
        ble_cpc_discover_cmp_cb(conidx, status);
        return;
    }

    uint8_t char_type = ble_cpc_char_uuid_to_char_type(char_uuid);

    if (char_type >= CPS_CHAR_MAX)
    {
        return;
    }

    p_cpc_env->p_env[conidx]->chars[char_type] = p_char;

    if ((p_char->char_prop & (GATT_NTF_PROP | GATT_IND_PROP)) != 0)
    {
        gattc_write_cccd_descriptor(prf, p_char, (p_char->char_prop & GATT_NTF_PROP), (p_char->char_prop & GATT_IND_PROP));
    }

    if (p_char->char_prop & GATT_BROADCAST)
    {
        gattc_write_sccd_descriptor(prf, p_char, true);
    }

    if (p_char->char_prop & GATT_RD_REQ)
    {
        gattc_read_character_value(prf, p_char);
    }

    if (cmpl)
    {
        ble_cpc_discover_cmp_cb(conidx, ATT_ERROR_NO_ERROR);
    }
}

static int ble_cps_gatt_client_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param)
{
    if (p_cpc_env == NULL)
    {
        return false;
    }

    if (prf->con_idx < gap_zero_based_conidx_to_ble_conidx(0))
    {
        TRACE(1, "%s not support conidx = %d", __func__, prf->con_idx);
        return false;
    }

    uint8_t conidx = gap_zero_based_conidx(prf->con_idx);

    switch (event)
    {
        case GATT_PROF_EVENT_OPENED:
        {
            ble_cpc_con_create(conidx);
        }
        break;
        case GATT_PROF_EVENT_CLOSED:
        {
            ble_cpc_con_cleanup(conidx);
        }
        break;
        case GATT_PROF_EVENT_SERVICE:
        {
            ble_cpc_service_disocver_cb(prf, conidx, param.service->service, param.service->error_code);
        }
        break;
        case GATT_PROF_EVENT_CHARACTER:
        {
            ble_cpc_character_disocver_cb(prf, conidx, param.character->char_uuid, param.character->character,
                                          param.character->error_code, param.character->discover_cmpl);
        }
        break;
        case GATT_PROF_EVENT_CHAR_READ_RSP:
        {
            ble_cpc_read_cmp_cb(conidx, param.char_read_rsp->character, param.char_read_rsp->error_code, false, false);

            if (param.char_read_rsp->error_code == ATT_ERROR_NO_ERROR)
            {
                ble_cpc_att_val_cb(conidx, param.char_read_rsp->character, false, false,
                                   param.char_read_rsp->value, param.char_read_rsp->value_len);
            }
        }
        break;
        case GATT_PROF_EVENT_NOTIFY:
        {
            ble_cpc_att_val_evt_cb(conidx, param.notify->character, param.notify->value, param.notify->value_len);
        }
        break;
        case GATT_PROF_EVENT_DESC_READ_RSP:
        {
            ble_cpc_read_cmp_cb(conidx, param.desc_read_rsp->character, param.desc_read_rsp->error_code, false, false);

            if (param.desc_read_rsp->error_code == ATT_ERROR_NO_ERROR)
            {
                ble_cpc_att_val_cb(conidx, param.desc_read_rsp->character, true,
                                   param.desc_read_rsp->desc_uuid == GATT_DESC_UUID_CHAR_SERVER_CONFIG,
                                   param.desc_read_rsp->value, param.desc_read_rsp->value_len);
            }
        }
        break;
        case GATT_PROF_EVENT_CHAR_WRITE_RSP:
        {
            ble_cpc_write_cmp_cb(conidx, param.char_write_rsp->character, param.char_write_rsp->error_code, false, false);
        }
        break;
        case GATT_PROF_EVENT_DESC_WRITE_RSP:
        {
            ble_cpc_write_cmp_cb(conidx, param.char_write_rsp->character, param.char_write_rsp->error_code, true,
                                 param.desc_write_rsp->desc_uuid == GATT_DESC_UUID_CHAR_SERVER_CONFIG);
        }
        break;
        default:
        {
            break;
        }
    }

    return 0;
}

/* FUNCTIONS */
uint16_t ble_cpc_service_discover(uint8_t conidx)
{
    uint16_t status = BT_STS_NOT_ALLOW;
    // Client environment
    if (p_cpc_env != NULL)
    {
        gatt_prf_t *p_prf = gattc_get_profile(p_cpc_env->prf_lid,
                                              app_ble_get_conhdl_from_conidx(conidx));
        if ((conidx < BLE_CONNECTION_MAX) && (p_cpc_env->p_env[conidx] == NULL) && p_prf != NULL)
        {
            // allocate environment variable for task instance
            p_cpc_env->p_env[conidx] = (ble_cpc_conn_t *) bes_bt_me_bes_bt_buf_malloc(sizeof(ble_cpc_conn_t));

            if (p_cpc_env->p_env[conidx] != NULL)
            {
                memset(p_cpc_env->p_env[conidx], 0, sizeof(ble_cpc_conn_t));
                co_timer_new(&(p_cpc_env->p_env[conidx]->timer), CPS_CP_TIMEOUT, ble_cpc_timer_handler,
                             (void *)((uint32_t) conidx), 1);

                // start discovery
                status = gattc_discover_service(p_prf, GATT_UUID_CPW_SERVICE, NULL);

                // Go to DISCOVERING state
                p_cpc_env->p_env[conidx]->discover   = true;
                p_cpc_env->p_env[conidx]->ctrl_pt_op = CPS_CTRL_PT_RESERVED;
            }
            else
            {
                status = BT_STS_NO_RESOURCES;
            }
        }
    }

    return status;
}

uint16_t ble_cpc_read_sensor_feat(uint8_t conidx)
{
    uint16_t status = ble_cpc_read_val(conidx, BLE_CPC_RD_CP_FEAT);
    return status;
}

uint16_t ble_cpc_read_sensor_loc(uint8_t conidx)
{
    uint16_t status = ble_cpc_read_val(conidx, BLE_CPC_RD_SENSOR_LOC);
    return status;
}

uint16_t ble_cpc_read_cfg(uint8_t conidx, uint8_t desc_code)
{
    uint16_t status;

    switch (desc_code)
    {
        case BLE_CPC_RD_WR_CP_MEAS_CL_CFG:
        case BLE_CPC_RD_WR_CP_MEAS_SV_CFG:
        case BLE_CPC_RD_WR_VECTOR_CFG:
        case BLE_CPC_RD_WR_CTRL_PT_CFG:
        {
            status = ble_cpc_read_val(conidx, desc_code);
        }
        break;
        default:
        {
            status = BT_STS_INVALID_PARM;
        }
        break;
    }

    return status;
}

uint16_t ble_cpc_write_cfg(uint8_t conidx, uint8_t desc_code, uint16_t cfg_val)
{
    uint16_t status = BT_STS_NOT_ALLOW;
    // Client environment
    if (p_cpc_env != NULL)
    {
        gatt_prf_t *p_prf = gattc_get_profile(p_cpc_env->prf_lid,
                                              app_ble_get_conhdl_from_conidx(conidx));
        if ((conidx < BLE_CONNECTION_MAX) && (p_cpc_env->p_env[conidx] != NULL) && (!p_cpc_env->p_env[conidx]->discover))
        {
            ble_cpc_conn_t *p_con_env = p_cpc_env->p_env[conidx];
            gatt_peer_character_t *p_char = NULL;
            uint16_t cfg_en_val = 0;

            switch (desc_code)
            {
                case BLE_CPC_RD_WR_CP_MEAS_CL_CFG:
                {
                    p_char        = p_con_env->chars[CPS_MEAS_CHAR];
                    cfg_en_val =  PRF_CLI_START_NTF;
                }
                break;
                case BLE_CPC_RD_WR_CP_MEAS_SV_CFG:
                {
                    p_char        = p_con_env->chars[CPS_MEAS_CHAR];
                    cfg_en_val =  PRF_SRV_START_BCST;
                }
                break;
                case BLE_CPC_RD_WR_VECTOR_CFG:
                {
                    p_char        = p_con_env->chars[CPS_VECTOR_CHAR];
                    cfg_en_val =  PRF_CLI_START_NTF;
                }
                break;
                case BLE_CPC_RD_WR_CTRL_PT_CFG:
                {
                    p_char        = p_con_env->chars[CPS_CTRL_PT_CHAR];
                    cfg_en_val =  PRF_CLI_START_IND;
                }
                break;
                default:
                { } break;
            }

            if (p_char == NULL)
            {
                status = BT_STS_NOT_FOUND;
            }
            else if ((cfg_val != PRF_CLI_STOP_NTFIND) && (cfg_val != cfg_en_val))
            {
                status = BT_STS_INVALID_PARM;
            }
            else
            {
                // Force endianess
                cfg_val = co_host_to_uint16_be(cfg_val);

                if (cfg_en_val != PRF_SRV_START_BCST)
                {
                    status = gattc_write_cccd_descriptor(p_prf, p_char, cfg_val == PRF_CLI_START_NTF, cfg_val == PRF_CLI_START_IND);
                }
                else
                {
                    status = gattc_write_sccd_descriptor(p_prf, p_char, cfg_val == cfg_en_val);
                }
            }
        }
    }

    return status;
}

uint16_t ble_cpc_ctrl_pt_req(uint8_t conidx, uint8_t req_op_code, const union ble_cps_ctrl_pt_req_val *p_value)
{
    uint16_t status = BT_STS_NOT_ALLOW;
    // Client environment
    if (p_value == NULL)
    {
        status = BT_STS_INVALID_PARM;
    }
    else if (p_cpc_env != NULL)
    {
        gatt_prf_t *p_prf = gattc_get_profile(p_cpc_env->prf_lid,
                                              app_ble_get_conhdl_from_conidx(conidx));
        if ((conidx < BLE_CONNECTION_MAX) && (p_cpc_env->p_env[conidx] != NULL) && (!p_cpc_env->p_env[conidx]->discover))
        {
            ble_cpc_conn_t *p_con_env = p_cpc_env->p_env[conidx];
            gatt_peer_character_t *p_char = p_con_env->chars[CPS_CTRL_PT_CHAR];

            if (p_char == NULL)
            {
                status = BT_STS_NOT_FOUND;
            }
            // reject if there is an ongoing control point operation
            else if (p_con_env->ctrl_pt_op != CPS_CTRL_PT_RESERVED)
            {
                status = BT_STS_NOT_ALLOW;
            }
            else
            {
                uint8_t *p_buf = NULL;
                uint8_t buf_len = 0;

                // allocate buffer for event transmission
                if ((p_buf = bes_bt_me_bes_bt_buf_malloc(CPS_CP_CTRL_PT_REQ_MAX_LEN)))
                {
                    buf_len = ble_cpc_pack_ctrl_pt_req(p_buf, req_op_code, p_value);

                    if (buf_len != 0)
                    {
                        status = gattc_write_character_value(p_prf, p_char, p_buf, buf_len);
                        if (status == BT_STS_SUCCESS)
                        {
                            // save on-going operation
                            p_con_env->ctrl_pt_op = req_op_code;
                        }
                    }
                    else
                    {
                        status = BT_STS_FAILED;
                    }

                    if (p_buf != NULL)
                    {
                        bes_bt_me_bes_bt_buf_free((uint8_t *)p_buf);
                    }
                }
                else
                {
                    status = BT_STS_NO_RESOURCES;
                }
            }
        }
    }

    return status;
}

uint16_t ble_cpc_init(const ble_cpc_cb_t *p_cb)
{
    uint8_t conidx;
    // DB Creation Status
    uint16_t status = BT_STS_SUCCESS;
    uint8_t prf_lid = 0xFF;

    if (p_cpc_env != NULL)
    {
        return BT_STS_ALREADY_EXIST;
    }

    do
    {
        if ((p_cb == NULL) || (p_cb->cb_svc_discover_cmp == NULL) || (p_cb->cb_read_sensor_feat_cmp == NULL)
                || (p_cb->cb_read_sensor_loc_cmp == NULL) || (p_cb->cb_read_cfg_cmp == NULL) || (p_cb->cb_write_cfg_cmp == NULL)
                || (p_cb->cb_meas == NULL) || (p_cb->cb_vector == NULL) || (p_cb->cb_ctrl_pt_req_cmp == NULL))
        {
            status = BT_STS_INVALID_PARM;
            break;
        }

        gattc_cfg_t init_cfg =
        {
            .preferred_mtu = CPS_CP_MEAS_MAX_LEN + 3,
        };

        // register CPC user
        prf_lid = gattc_register_profile(ble_cps_gatt_client_callback, &init_cfg);

        if (prf_lid == 0xFF)
        {
            status = BT_STS_FAILED;
            break;
        }

        p_cpc_env = (ble_cpc_env_t *) bes_bt_me_bes_bt_buf_malloc(sizeof(ble_cpc_env_t));

        if (p_cpc_env != NULL)
        {
            // initialize environment variable
            p_cpc_env->p_cbs    = p_cb;
            p_cpc_env->prf_lid = prf_lid;
            for (conidx = 0; conidx < BLE_CONNECTION_MAX; conidx++)
            {
                p_cpc_env->p_env[conidx] = NULL;
            }
        }
        else
        {
            status = BT_STS_NO_RESOURCES;
        }
    } while (0);

    if ((status != BT_STS_SUCCESS) && (prf_lid != 0xFF))
    {
        gattc_unregister_profile(prf_lid);
    }

    return status;
}

uint16_t ble_cpc_deinit(void)
{
    if (p_cpc_env != NULL)
    {

        gattc_unregister_profile(p_cpc_env->prf_lid);

        uint8_t conidx;

        // cleanup environment variable for each task instances
        for (conidx = 0; conidx < BLE_CONNECTION_MAX; conidx++)
        {
            if (p_cpc_env->p_env[conidx] != NULL)
            {
                co_timer_stop(&(p_cpc_env->p_env[conidx]->timer));
                bes_bt_me_bes_bt_buf_free((uint8_t *)p_cpc_env->p_env[conidx]);
            }
        }

        bes_bt_me_bes_bt_buf_free((uint8_t *)p_cpc_env);

        p_cpc_env = NULL;
    }

    return BT_STS_SUCCESS;
}

#endif /* BLE_CPC_ENABLED */