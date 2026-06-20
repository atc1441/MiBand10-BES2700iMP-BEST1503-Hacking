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
#if 1//defined(BLE_CPS_ENABLED)

/* INCLUDE */
#include "bt_common_define.h"
#include "gatt_service.h"
#include "co_math.h"
#include "bes_me_api.h"
#include "plat_types.h"

#include "app_ble.h"

#include "ble_cycling_power_sens.h"

/* DEFINE */
/// Check if a specific characteristic has been added in the database (according to mask)
#define CPS_IS_FEATURE_SUPPORTED(features, flag) ((features & flag) == flag)

/// Mandatory Attributes (CP Measurement + CP Feature + CP Sensor Location)
#define CPS_MANDATORY_MASK           (0x01EF)
/// Broadcast Attribute
#define CPS_MEAS_BCST_MASK           (0x0010)
/// Vector Attributes
#define CPS_VECTOR_MASK              (0x0E00)
/// Control Point Attributes
#define CPS_CTRL_PT_MASK             (0x7000)

/// Broadcast supported flag
#define CPS_BROADCASTER_SUPP_FLAG    (0x01)
/// Control Point supported flag
#define CPS_CTRL_PT_CHAR_SUPP_FLAG   (0x02)

#define CPS_2_GAP_CONIDX_BF(conidx)  (gap_conn_bf(gap_zero_based_conidx_to_ble_conidx(conidx)))

/// Cycling Power Service - Attribute List
enum cps_cps_att_list
{
    /// Cycling Power Service
    CPS_IDX_SVC,
    /// CP Measurement
    CPS_IDX_CP_MEAS_CHAR,
    CPS_IDX_CP_MEAS_NTF_CFG,
    CPS_IDX_CP_MEAS_BCST_CFG,
    /// CP Feature
    CPS_IDX_CP_FEAT_CHAR,
    /// Sensor Location
    CPS_IDX_SENSOR_LOC_CHAR,
    /// CP Vector
    CPS_IDX_VECTOR_CHAR,
    CPS_IDX_VECTOR_NTF_CFG,
    /// CP Control Point
    CPS_IDX_CTRL_PT_CHAR,
    CPS_IDX_CTRL_PT_IND_CFG,

    /// Number of attributes
    CPS_IDX_NB,
};

/// Operation Code used in the profile state machine
enum cps_op_code
{
    /// Reserved Operation Code
    CPS_RESERVED_OP_CODE                          = 0x00,
    /// Send CP Measurement Operation Code
    CPS_NTF_MEAS_OP_CODE                          = 0x01,
    /// Send Vector Operation Code
    CPS_NTF_VECTOR_OP_CODE                        = 0x02,
    /// Send Control Point response
    CPS_CTRL_PT_RESP_OP_CODE                      = 0x03,
};

/// Profile Configuration Additional Flags ()
enum cps_prf_cfg_flag_bf
{
    /// CP Measurement - Client Char. Cfg
    CPS_PRF_CFG_FLAG_CP_MEAS_NTF_POS        = 0,
    CPS_PRF_CFG_FLAG_CP_MEAS_NTF_BIT        = CO_BIT(CPS_PRF_CFG_FLAG_CP_MEAS_NTF_POS),

    /// CP Measurement - Server Char. Cfg
    CPS_PRF_CFG_FLAG_SP_MEAS_NTF_POS        = 1,
    CPS_PRF_CFG_FLAG_SP_MEAS_NTF_BIT        = CO_BIT(CPS_PRF_CFG_FLAG_SP_MEAS_NTF_POS),

    /// CP Vector - Client Characteristic configuration
    CPS_PRF_CFG_FLAG_VECTOR_NTF_POS         = 2,
    CPS_PRF_CFG_FLAG_VECTOR_NTF_BIT         = CO_BIT(CPS_PRF_CFG_FLAG_VECTOR_NTF_POS),

    /// Control Point - Client Characteristic configuration
    CPS_PRF_CFG_FLAG_CTRL_PT_IND_POS        = 3,
    CPS_PRF_CFG_FLAG_CTRL_PT_IND_BIT        = CO_BIT(CPS_PRF_CFG_FLAG_CTRL_PT_IND_POS),

    /// Bonded data used
    CPS_PRF_CFG_PERFORMED_OK_POS            = 4,
    CPS_PRF_CFG_PERFORMED_OK_BIT            = CO_BIT(CPS_PRF_CFG_PERFORMED_OK_POS),
};

/* TYPEDEF */
/// ongoing operation information
typedef struct cps_buf_meta
{
    struct list_node node;
    /// meaningful for some operation
    uint32_t  conidx_bf;
    /// Operation
    uint8_t   operation;
    /// used to know on which device interval update has been requested, and to prevent
    /// indication to be triggered on this connection index
    uint8_t   conidx;
    /// last transmitted information flag
    uint16_t  last_flag;
    /// True if measurement must be sent onto a new connection.
    bool      new;
    /// Buf length
    uint8_t   buf_len;
    /// Buf
    uint8_t   buf[0];
} ble_cps_buf_meta_t;

/// Cycling Power Profile Sensor environment variable per connection
typedef struct cps_cnx_env
{
    /// Measurement content mask
    uint16_t mask_meas_content;
    /// Profile Notify/Indication Flags
    uint8_t  prfl_ntf_ind_cfg;
} ble_cps_cnx_env_t;

/// CPS Environment Variable
typedef struct cps_env
{
    /// Operation Event TX wait queue
    struct list_node   wait_queue;
    /// Event callabcks
    const ble_cps_cb_t *p_cb;
    /// Environment variable pointer for each connections
    ble_cps_cnx_env_t env[BLE_CONNECTION_MAX];
    /// Instantaneous Power
    uint32_t       inst_power;
    /// Cumulative Value
    uint32_t       cumul_wheel_rev;
    /// Feature Configuration Flags
    uint32_t       features;
    /// Profile Configuration Flags
    uint16_t       prfl_cfg;
    /// Sensor Location
    uint8_t        sensor_loc;
    /// Control point operation on-going (see enum #cps_ctrl_pt_code)
    uint8_t        ctrl_pt_op;
    /// Operation On-going
    bool           op_ongoing;
    /// Prevent recursion in execute_operation function
    bool           in_exe_op;
    /// Service attribute list
    gatt_attribute_t *p_attr_list;
} ble_cps_env_t;

ble_cps_env_t *p_cps_env = NULL;

/* SERVICE DEFINE*/
// Cycling Power Service Declaration
GATT_DECL_PRI_SERVICE(CPS_SERVICE, GATT_UUID_CPW_SERVICE);

// CP Measurement Characteristic Declaration
GATT_DECL_CHAR(CPS_MEASUREMENT, GATT_CHAR_UUID_CYCL_PWR_MEASURE, GATT_NTF_PROP, ATT_SEC_NONE);
// CP Measurement Characteristic - Client Characteristic Configuration Descriptor
GATT_DECL_CCCD_DESCRIPTOR(CPS_MEASUREMENT_CCCD, ATT_SEC_NONE);
// CP Measurement Characteristic - Server Characteristic Configuration Descriptor
GATT_DECL_SCCD_DESCRIPTOR(CPS_MEASUREMENT_SCCD, ATT_SEC_NONE);

// CP Feature Characteristic Declaration
GATT_DECL_CHAR(CPS_FEATURE, GATT_CHAR_UUID_CYCL_PWR_FEATURE, GATT_RD_REQ, ATT_SEC_NONE);

// Sensor Location Characteristic Declaration
GATT_DECL_CHAR(CPS_SENS_LOCATION, GATT_CHAR_UUID_SENS_OR_LOCATION, GATT_RD_REQ, ATT_SEC_NONE);

// CP Vector Characteristic Declaration
GATT_DECL_CHAR(CPS_VECTOR, GATT_CHAR_UUID_CYCL_PWR_VECTOR, GATT_NTF_PROP, ATT_SEC_NONE);
// CP Vector Characteristic - Client Characteristic Configuration Descriptor
GATT_DECL_CCCD_DESCRIPTOR(CPS_VECTOR_CCCD, ATT_SEC_NONE);

// CP Control Point Characteristic Declaration
GATT_DECL_CHAR(CPS_CTRL_POINT, GATT_CHAR_UUID_CYCL_PWR_CTRL_POINT, GATT_IND_PROP | GATT_WR_REQ, ATT_SEC_NONE);
// CP Control Point Characteristic - Client Characteristic Configuration Descriptor
GATT_DECL_CCCD_DESCRIPTOR(CPS_CTRL_POINT_CCCD, ATT_SEC_NONE);

/// Full CPS Database Description - Used to add attributes into the database
static const gatt_attribute_t ble_cps_att_db[CPS_IDX_NB] =
{
    // Cycling Power Service Declaration
    gatt_attribute(CPS_SERVICE),
    // CP Measurement Characteristic Declaration
    gatt_attribute(CPS_MEASUREMENT),
    // CP Measurement Characteristic - Client Characteristic Configuration Descriptor
    gatt_attribute(CPS_MEASUREMENT_CCCD),
    // CP Measurement Characteristic - Server Characteristic Configuration Descriptor
    gatt_attribute(CPS_MEASUREMENT_SCCD),
    // CP Feature Characteristic Declaration
    gatt_attribute(CPS_FEATURE),
    // Sensor Location Characteristic Declaration
    gatt_attribute(CPS_SENS_LOCATION),
    // CP Vector Characteristic Declaration
    gatt_attribute(CPS_VECTOR),
    // CP Vector Characteristic - Client Characteristic Configuration Descriptor
    gatt_attribute(CPS_VECTOR_CCCD),
    // CP Control Point Characteristic Declaration
    gatt_attribute(CPS_CTRL_POINT),
    // CP Control Point Characteristic - Client Characteristic Configuration Descriptor
    gatt_attribute(CPS_CTRL_POINT_CCCD),
};

/* EXTERNAL FUNCTIONS */
extern uint8_t ble_prf_pack_date_time(uint8_t *p_buf, const prf_date_time_t *p_date_time);

/* INTERNAL FUNCTIONS */
static void ble_cps_exe_operation(void);

/// Get database attribute index
static uint8_t ble_cps_idx_get(const gatt_attribute_t *attr)
{
    uint8_t att_idx = ((uint32_t)attr - (uint32_t)p_cps_env->p_attr_list) / sizeof(gatt_attribute_t);

    if ((att_idx >= CPS_IDX_CP_MEAS_BCST_CFG)
            && !CPS_IS_FEATURE_SUPPORTED(p_cps_env->prfl_cfg, CPS_MEAS_BCST_MASK))
    {
        // No sccd
        att_idx += 1;
    }

    if ((att_idx >= CPS_IDX_VECTOR_CHAR)
            && !CPS_IS_FEATURE_SUPPORTED(p_cps_env->prfl_cfg, CPS_VECTOR_MASK))
    {
        att_idx += 2;
    }

    return att_idx;
}

static bool ble_cps_pack_meas(uint8_t *p_buf, uint8_t buf_len, const ble_cps_cp_meas_t *p_meas,
                              uint16_t max_size, uint16_t mask_content, uint16_t *p_last_flags)
{
    uint16_t meas_flags = p_meas->flags;
    uint16_t pkt_flag;
    bool complete = false;
    uint8_t *p_buf_tail = p_buf + buf_len;

    // Mask unwanted fields if supported
    if (GETB(p_cps_env->features, CPS_FEAT_CP_MEAS_CH_CONTENT_MASKING_SUP))
    {
        meas_flags &= ~mask_content;
    }

    // keep mandatory flags
    pkt_flag = (meas_flags & (CPS_MEAS_PEDAL_POWER_BALANCE_REFERENCE_BIT | CPS_MEAS_ACCUM_TORQUE_SOURCE_BIT
                              | CPS_MEAS_OFFSET_COMPENSATION_INDICATOR_BIT));

    // remove flags already transmitted
    meas_flags &= ~(*p_last_flags);

    do
    {
        // prepare 4 bytes for header
        max_size -= 4;

        // Check provided flags
        if (GETB(meas_flags, CPS_MEAS_PEDAL_POWER_BALANCE_PRESENT)) // 5
        {
            if (GETB(p_cps_env->features, CPS_FEAT_PEDAL_POWER_BALANCE_SUP))
            {
                if (max_size < sizeof(p_meas->pedal_power_balance))
                {
                    break;    // Stop packing
                }
                // Pack Pedal Power Balance info
                SETB(pkt_flag, CPS_MEAS_PEDAL_POWER_BALANCE_PRESENT, 1);
                p_buf_tail -= sizeof(p_meas->pedal_power_balance);
                *(p_buf_tail) = p_meas->pedal_power_balance;
                max_size -= sizeof(p_meas->pedal_power_balance);
            }
        }

        if (GETB(meas_flags, CPS_MEAS_ACCUM_TORQUE_PRESENT)) // 7
        {
            if (GETB(p_cps_env->features, CPS_FEAT_ACCUM_TORQUE_SUP))
            {
                if (max_size < sizeof(p_meas->accum_torque))
                {
                    break;    // Stop packing
                }
                // Pack Accumulated Torque info
                SETB(pkt_flag, CPS_MEAS_ACCUM_TORQUE_PRESENT, 1);
                p_buf_tail -= sizeof(p_meas->accum_torque);
                *(uint16_t *)p_buf_tail = co_host_to_uint16_le(p_meas->accum_torque);
                max_size -= sizeof(p_meas->accum_torque);
            }
        }

        if (GETB(meas_flags, CPS_MEAS_WHEEL_REV_DATA_PRESENT)) // 13
        {
            if (GETB(p_cps_env->features, CPS_FEAT_WHEEL_REV_DATA_SUP))
            {
                if (max_size < sizeof(p_meas->cumul_wheel_rev) + sizeof(p_meas->last_wheel_evt_time))
                {
                    break;    // Stop packing
                }
                // Pack Wheel Revolution Data (Cumulative Wheel & Last Wheel Event Time)
                SETB(pkt_flag, CPS_MEAS_WHEEL_REV_DATA_PRESENT, 1);
                p_buf_tail -= sizeof(p_meas->accum_torque);
                *(uint32_t *)p_buf_tail = co_host_to_uint32_le(p_meas->cumul_wheel_rev);
                p_buf_tail -= sizeof(p_meas->last_wheel_evt_time);
                *(uint16_t *)p_buf_tail = co_host_to_uint16_le(p_meas->last_wheel_evt_time);
                max_size -= sizeof(p_meas->cumul_wheel_rev) + sizeof(p_meas->last_wheel_evt_time);
            }
        }

        if (GETB(meas_flags, CPS_MEAS_CRANK_REV_DATA_PRESENT)) // 17
        {
            if (GETB(p_cps_env->features, CPS_FEAT_CRANK_REV_DATA_SUP))
            {
                if (max_size < sizeof(p_meas->cumul_crank_rev) + sizeof(p_meas->last_crank_evt_time))
                {
                    break;    // Stop packing
                }
                // Pack Crank Revolution Data (Cumulative Crank & Last Crank Event Time)
                SETB(pkt_flag, CPS_MEAS_CRANK_REV_DATA_PRESENT, 1);
                p_buf_tail -= sizeof(p_meas->cumul_crank_rev);
                *(uint32_t *)p_buf_tail = co_host_to_uint16_le(p_meas->cumul_crank_rev);
                p_buf_tail -= sizeof(p_meas->last_crank_evt_time);
                *(uint16_t *)p_buf_tail = co_host_to_uint16_le(p_meas->last_crank_evt_time);
                max_size -= sizeof(p_meas->cumul_crank_rev) + sizeof(p_meas->last_crank_evt_time);
            }
        }

        if (GETB(meas_flags, CPS_MEAS_EXTREME_FORCE_MAGNITUDES_PRESENT)) // 21 - Greater than Min packet
        {
            if (GETB(p_cps_env->features, CPS_FEAT_EXTREME_MAGNITUDES_SUP)
                    && (!GETB(p_cps_env->features, CPS_FEAT_SENSOR_MEAS_CONTEXT)))
            {
                if (max_size < sizeof(p_meas->max_force_magnitude) + sizeof(p_meas->min_force_magnitude))
                {
                    break;    // Stop packing
                }

                // Pack Extreme Force Magnitudes (Maximum Force Magnitude & Minimum Force Magnitude)
                SETB(pkt_flag, CPS_MEAS_EXTREME_FORCE_MAGNITUDES_PRESENT, 1);
                p_buf_tail -= sizeof(p_meas->max_force_magnitude);
                *(uint32_t *)p_buf_tail = co_host_to_uint16_le(p_meas->max_force_magnitude);
                p_buf_tail -= sizeof(p_meas->min_force_magnitude);
                *(uint16_t *)p_buf_tail = co_host_to_uint16_le(p_meas->min_force_magnitude);
                max_size -= sizeof(p_meas->max_force_magnitude) + sizeof(p_meas->min_force_magnitude);
            }
        }

        if (GETB(meas_flags, CPS_MEAS_EXTREME_TORQUE_MAGNITUDES_PRESENT)) // 25
        {
            if (GETB(p_cps_env->features, CPS_FEAT_EXTREME_MAGNITUDES_SUP)
                    && GETB(p_cps_env->features, CPS_FEAT_SENSOR_MEAS_CONTEXT))
            {
                if (max_size < sizeof(p_meas->max_torque_magnitude) + sizeof(p_meas->min_torque_magnitude))
                {
                    break;    // Stop packing
                }

                // Pack Extreme Force Magnitudes (Maximum Force Magnitude & Minimum Force Magnitude)
                SETB(pkt_flag, CPS_MEAS_EXTREME_TORQUE_MAGNITUDES_PRESENT, 1);
                p_buf_tail -= sizeof(p_meas->max_torque_magnitude);
                *(uint32_t *)p_buf_tail = co_host_to_uint16_le(p_meas->max_torque_magnitude);
                p_buf_tail -= sizeof(p_meas->min_torque_magnitude);
                *(uint16_t *)p_buf_tail = co_host_to_uint16_le(p_meas->min_torque_magnitude);
                max_size -= sizeof(p_meas->max_torque_magnitude) + sizeof(p_meas->min_torque_magnitude);
            }
        }

        if (GETB(meas_flags, CPS_MEAS_EXTREME_ANGLES_PRESENT)) // 28
        {
            if (GETB(p_cps_env->features, CPS_FEAT_EXTREME_ANGLES_SUP))
            {
                uint32_t angle;
                if (max_size < 3)
                {
                    break;    // Stop packing
                }

                // Pack Extreme Angles (Maximum Angle & Minimum Angle)
                // Force to 12 bits
                SETB(pkt_flag, CPS_MEAS_EXTREME_ANGLES_PRESENT, 1);
                angle = (uint32_t)((p_meas->max_angle & 0x0FFF) | ((p_meas->min_angle & 0x0FFF) << 12));
                p_buf_tail -= 3;
                uint8_t angle_v[3] = {CO_SPLIT_UINT24_LE(angle)};
                memcpy(p_buf_tail, angle_v, 3);
                max_size -= 3;
            }
        }

        if (GETB(meas_flags, CPS_MEAS_TOP_DEAD_SPOT_ANGLE_PRESENT)) // 30
        {
            if (GETB(p_cps_env->features, CPS_FEAT_TOPBOT_DEAD_SPOT_ANGLES_SUP))
            {
                if (max_size < sizeof(p_meas->top_dead_spot_angle))
                {
                    break;    // Stop packing
                }

                // Pack Top Dead Spot Angle
                SETB(pkt_flag, CPS_MEAS_TOP_DEAD_SPOT_ANGLE_PRESENT, 1);
                p_buf_tail -= sizeof(p_meas->top_dead_spot_angle);
                *(uint32_t *)p_buf_tail = co_host_to_uint16_le(p_meas->top_dead_spot_angle);
                max_size -= sizeof(p_meas->top_dead_spot_angle);
            }
        }

        if (GETB(meas_flags, CPS_MEAS_BOTTOM_DEAD_SPOT_ANGLE_PRESENT)) // 32
        {
            if (GETB(p_cps_env->features, CPS_FEAT_TOPBOT_DEAD_SPOT_ANGLES_SUP))
            {
                if (max_size < sizeof(p_meas->bot_dead_spot_angle))
                {
                    break;    // Stop packing
                }

                // Pack Bottom Dead Spot Angle
                SETB(pkt_flag, CPS_MEAS_BOTTOM_DEAD_SPOT_ANGLE_PRESENT, 1);
                p_buf_tail -= sizeof(p_meas->bot_dead_spot_angle);
                *(uint32_t *)p_buf_tail = co_host_to_uint16_le(p_meas->bot_dead_spot_angle);
                max_size -= sizeof(p_meas->bot_dead_spot_angle);
            }
        }

        if (GETB(meas_flags, CPS_MEAS_ACCUM_ENERGY_PRESENT))  // 34
        {
            if (GETB(p_cps_env->features, CPS_FEAT_ACCUM_ENERGY_SUP))
            {
                if (max_size < sizeof(p_meas->accum_energy))
                {
                    break;    // Stop packing
                }

                // Pack Accumulated Energy
                SETB(pkt_flag, CPS_MEAS_ACCUM_ENERGY_PRESENT, 1);
                p_buf_tail -= sizeof(p_meas->accum_energy);
                *(uint32_t *)p_buf_tail = co_host_to_uint16_le(p_meas->accum_energy);
                max_size -= sizeof(p_meas->accum_energy);
            }
        }

        complete = true;
    } while (0);

    // Flags value
    *(uint16_t *)p_buf = co_host_to_uint16_le(pkt_flag);
    p_buf += sizeof(pkt_flag);

    *p_last_flags = pkt_flag;

    // Instant Power (Mandatory)
    *(uint16_t *)p_buf = co_host_to_uint16_le(p_meas->inst_power);

    return (complete);
}

static uint16_t ble_cps_unpack_ctrl_point_req(uint8_t conidx, const uint8_t *p_buf, uint8_t buf_len)
{
    uint8_t op_code;
    uint8_t ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_NOT_SUPP;
    uint16_t status = BT_STS_SUCCESS;
    union ble_cps_ctrl_pt_req_val value;
    memset(&value, 0, sizeof(union ble_cps_ctrl_pt_req_val));
    op_code =  p_buf[0];

    p_buf += 1;

    if (buf_len <= 1)
    {
        return ATT_ERROR_INVALID_VALUE_LEN;
    }

    buf_len -= 1;

    // Operation Code
    switch (op_code)
    {
        case (CPS_CTRL_PT_SET_CUMUL_VAL):
        {
            // Check if the Wheel Revolution Data feature is supported
            if (GETB(p_cps_env->features, CPS_FEAT_WHEEL_REV_DATA_SUP))
            {
                // Provided parameter in not within the defined range
                ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_INV_PARAM;

                if (buf_len == 4)
                {
                    // The request can be handled
                    ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_SUCCESS;
                    // Update the environment
                    p_cps_env->ctrl_pt_op = op_code;
                    // Cumulative value
                    value.cumul_val = CO_COMBINE_UINT32_LE(p_buf);
                }
            }
        }
        break;

        case (CPS_CTRL_PT_UPD_SENSOR_LOC):
        {
            // Check if the Multiple Sensor Location feature is supported
            if (GETB(p_cps_env->features, CPS_FEAT_MULT_SENSOR_LOC_SUP))
            {
                // Provided parameter in not within the defined range
                ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_INV_PARAM;

                if (buf_len == 1)
                {
                    uint8_t sensor_loc = p_buf[0];

                    // Check the sensor location value
                    if (sensor_loc < CPS_LOC_MAX)
                    {
                        value.sensor_loc = sensor_loc;
                        // The request can be handled
                        ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_SUCCESS;
                        // Update the environment
                        p_cps_env->ctrl_pt_op = op_code;
                    }
                }
            }
        }
        break;

        case (CPS_CTRL_PT_REQ_SUPP_SENSOR_LOC):
        {
            // Check if the Multiple Sensor Location feature is supported
            if (GETB(p_cps_env->features, CPS_FEAT_MULT_SENSOR_LOC_SUP))
            {
                // The request can be handled
                ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_SUCCESS;
                // Update the environment
                p_cps_env->ctrl_pt_op = op_code;
            }
        }
        break;

        case (CPS_CTRL_PT_SET_CRANK_LENGTH):
        {
            // Check if the Crank Length Adjustment feature is supported
            if (GETB(p_cps_env->features, CPS_FEAT_CRANK_LENGTH_ADJ_SUP))
            {
                // Provided parameter in not within the defined range
                ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_INV_PARAM;

                if (buf_len == 2)
                {
                    // The request can be handled
                    ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_SUCCESS;
                    // Update the environment
                    p_cps_env->ctrl_pt_op = op_code;
                    // Crank Length
                    value.crank_length = CO_COMBINE_UINT16_LE(p_buf);
                }
            }
        }
        break;

        case (CPS_CTRL_PT_REQ_CRANK_LENGTH):
        {
            // Optional even if feature not supported
            ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_SUCCESS;
            // Update the environment
            p_cps_env->ctrl_pt_op = op_code;
        }
        break;

        case (CPS_CTRL_PT_SET_CHAIN_LENGTH):
        {
            // Check if the Chain Length Adjustment feature is supported
            if (GETB(p_cps_env->features, CPS_FEAT_CHAIN_LENGTH_ADJ_SUP))
            {
                // Provided parameter in not within the defined range
                ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_INV_PARAM;

                if (buf_len == 2)
                {
                    // The request can be handled
                    ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_SUCCESS;
                    // Update the environment
                    p_cps_env->ctrl_pt_op = op_code;
                    // Chain Length
                    value.chain_length = CO_COMBINE_UINT16_LE(p_buf);
                }
            }
        }
        break;

        case (CPS_CTRL_PT_REQ_CHAIN_LENGTH):
        {
            // Optional even if feature not supported
            ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_SUCCESS;
            // Update the environment
            p_cps_env->ctrl_pt_op = op_code;
        }
        break;

        case (CPS_CTRL_PT_SET_CHAIN_WEIGHT):
        {
            // Check if the Chain Weight Adjustment feature is supported
            if (GETB(p_cps_env->features, CPS_FEAT_CHAIN_WEIGHT_ADJ_SUP))
            {
                // Provided parameter in not within the defined range
                ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_INV_PARAM;

                if (buf_len == 2)
                {
                    // The request can be handled
                    ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_SUCCESS;
                    // Update the environment
                    p_cps_env->ctrl_pt_op = op_code;
                    // Chain Weight
                    value.chain_weight = CO_COMBINE_UINT16_LE(p_buf);
                }
            }
        }
        break;

        case (CPS_CTRL_PT_REQ_CHAIN_WEIGHT):
        {
            // Optional even if feature not supported
            ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_SUCCESS;
            // Update the environment
            p_cps_env->ctrl_pt_op = op_code;
        }
        break;

        case (CPS_CTRL_PT_SET_SPAN_LENGTH):
        {
            // Check if the Span Length Adjustment feature is supported
            if (GETB(p_cps_env->features, CPS_FEAT_SPAN_LENGTH_ADJ_SUP))
            {
                // Provided parameter in not within the defined range
                ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_INV_PARAM;

                if (buf_len == 2)
                {
                    // The request can be handled
                    ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_SUCCESS;
                    // Update the environment
                    p_cps_env->ctrl_pt_op = op_code;
                    // Span Length
                    value.span_length = CO_COMBINE_UINT16_LE(p_buf);
                }
            }

        }
        break;

        case (CPS_CTRL_PT_REQ_SPAN_LENGTH):
        {
            // Optional even if feature not supported
            ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_SUCCESS;
            // Update the environment
            p_cps_env->ctrl_pt_op = op_code;
        }
        break;

        case (CPS_CTRL_PT_START_OFFSET_COMP):
        {
            // Check if the Offset Compensation feature is supported
            if (GETB(p_cps_env->features, CPS_FEAT_OFFSET_COMP_SUP))
            {
                // The request can be handled
                ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_SUCCESS;
                // Update the environment
                p_cps_env->ctrl_pt_op = op_code;
            }
        }
        break;

        case (CPS_CTRL_MASK_CP_MEAS_CH_CONTENT):
        {
            // Check if the CP Masking feature is supported
            if (GETB(p_cps_env->features, CPS_FEAT_CP_MEAS_CH_CONTENT_MASKING_SUP))
            {
                // Provided parameter in not within the defined range
                ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_INV_PARAM;

                if (buf_len == 2)
                {
                    // The request can be handled
                    ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_SUCCESS;
                    // Update the environment
                    p_cps_env->ctrl_pt_op = op_code;
                    // Mask content
                    value.mask_content = CO_COMBINE_UINT16_LE(p_buf);
                }
            }
        }
        break;

        case (CPS_CTRL_REQ_SAMPLING_RATE):
        {
            // Optional even if feature not supported
            ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_SUCCESS;
            // Update the environment
            p_cps_env->ctrl_pt_op = op_code;
        }
        break;

        case (CPS_CTRL_REQ_FACTORY_CALIBRATION_DATE):
        {
            // Optional even if feature not supported
            ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_SUCCESS;
            // Update the environment
            p_cps_env->ctrl_pt_op = op_code;
        }
        break;

        case (CPS_CTRL_START_ENHANCED_OFFSET_COMP):
        {
            // Check if the Enhanced Offset Compensation feature is supported
            if (GETB(p_cps_env->features, CPS_FEAT_ENHANCED_OFFSET_COMPENS_SUP))
            {
                // The request can be handled
                ctrl_pt_rsp_status = CPS_CTRL_PT_RESP_SUCCESS;
                // Update the environment
                p_cps_env->ctrl_pt_op = op_code;
            }
        }
        break;

        default:
        {
            // Operation Code is invalid, status is already CPS_CTRL_PT_RESP_NOT_SUPP
        } break;
    }

    // If no error raised, inform the application about the request
    if (ctrl_pt_rsp_status == CPS_CTRL_PT_RESP_SUCCESS)
    {
        const ble_cps_cb_t *p_cb  = (const ble_cps_cb_t *) p_cps_env->p_cb;

        // inform application about control point request
        p_cb->cb_ctrl_pt_req(conidx, op_code, &value);
    }
    else
    {
        uint8_t *p_out_buf = NULL;

        if ((p_out_buf = bes_bt_buf_malloc(CPS_CP_CTRL_PT_RSP_MIN_LEN + sizeof(ble_cps_buf_meta_t))))
        {
            ble_cps_buf_meta_t *p_meta = (ble_cps_buf_meta_t *)(p_out_buf);
            p_meta->buf_len = CPS_CP_CTRL_PT_RSP_MIN_LEN;
            p_out_buf = p_meta->buf;

            p_cps_env->ctrl_pt_op    = CPS_CTRL_PT_RSP_CODE;
            *p_out_buf++ = CPS_CTRL_PT_RSP_CODE;
            *p_out_buf++ = op_code;
            *p_out_buf++ = ctrl_pt_rsp_status;

            p_meta->conidx    = conidx;
            p_meta->operation = CPS_CTRL_PT_RESP_OP_CODE;

            // put event on wait queue
            colist_addto_tail(&(p_meta->node), &(p_cps_env->wait_queue));
            // execute operation
            ble_cps_exe_operation();
        }
        else
        {
            status = ATT_ERROR_INSUFF_RESOURCES;
        }
    }

    return status;
}

static uint8_t ble_cps_pack_vector(uint8_t *p_buf, const ble_cps_cp_vector_t *p_vector)
{
    uint8_t flags = p_vector->flags;
    bool force_torque_magnitude_present = false;
    uint8_t *p_buf_head = p_buf;

    // reseve 1 byte for flags
    p_buf += 1;

    // Check provided flags
    if (GETB(flags, CPS_VECTOR_CRANK_REV_DATA_PRESENT))
    {
        if (GETB(p_cps_env->features, CPS_FEAT_CRANK_REV_DATA_SUP))
        {
            // Pack Crank Revolution Data (Cumulative Crank & Last Crank Event Time)
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_vector->cumul_crank_rev);
            p_buf += 2;
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_vector->last_crank_evt_time);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, CPS_VECTOR_CRANK_REV_DATA_PRESENT, 0);
        }
    }

    if (GETB(flags, CPS_VECTOR_FIRST_CRANK_MEAS_ANGLE_PRESENT))
    {
        if (GETB(p_cps_env->features, CPS_FEAT_EXTREME_ANGLES_SUP))
        {
            // Pack First Crank Measurement Angle
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_vector->first_crank_meas_angle);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, CPS_VECTOR_FIRST_CRANK_MEAS_ANGLE_PRESENT, 0);
        }
    }

    if (GETB(flags, CPS_VECTOR_INST_FORCE_MAGNITUDE_ARRAY_PRESENT))
    {
        if (!GETB(p_cps_env->features, CPS_FEAT_SENSOR_MEAS_CONTEXT))
        {
            force_torque_magnitude_present = true;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, CPS_VECTOR_INST_FORCE_MAGNITUDE_ARRAY_PRESENT, 0);
        }
    }

    if (GETB(flags, CPS_VECTOR_INST_TORQUE_MAGNITUDE_ARRAY_PRESENT))
    {
        if (GETB(p_cps_env->features, CPS_FEAT_SENSOR_MEAS_CONTEXT))
        {
            force_torque_magnitude_present = true;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, CPS_VECTOR_INST_TORQUE_MAGNITUDE_ARRAY_PRESENT, 0);
        }
    }

    if (force_torque_magnitude_present)
    {
        uint8_t cursor;
        uint8_t max_nb = co_min(CPS_MAX_TORQUE_NB, p_vector->nb);
        // Pack Instantaneous Torque Magnitude Array
        for (cursor = 0 ; cursor < max_nb ; cursor++)
        {
            // Pack First Crank Measurement Angle
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_vector->force_torque_magnitude[cursor]);
            p_buf += 2;
        }
    }

    // Flags value
    p_buf_head[0] = p_vector->flags;

    return p_buf - p_buf_head;
}

static uint8_t ble_cps_pack_ctrl_point_rsp(uint8_t conidx, uint8_t *p_buf, uint8_t op_code, uint8_t resp_val,
                                           const union ble_cps_ctrl_pt_rsp_val *p_value)
{
    uint8_t *p_buf_head = p_buf;
    // Set the Response Code
    p_buf[0] = CPS_CTRL_PT_RSP_CODE;
    p_buf += 1;

    // Set the request operation code
    p_buf[0] = p_cps_env->ctrl_pt_op;
    p_buf += 1;

    if (resp_val == CPS_CTRL_PT_RESP_SUCCESS)
    {
        p_buf[0] = resp_val;
        p_buf += 1;

        switch (p_cps_env->ctrl_pt_op)
        {
            case (CPS_CTRL_PT_SET_CUMUL_VAL):
            {
                // Save in the environment
                p_cps_env->cumul_wheel_rev = p_value->cumul_wheel_rev;
            }
            break;

            case (CPS_CTRL_PT_UPD_SENSOR_LOC):
            {
                // Store the new value in the environment
                p_cps_env->sensor_loc = p_value->sensor_loc;
            }
            break;

            case (CPS_CTRL_PT_REQ_SUPP_SENSOR_LOC):
            {
                // Set the list of supported location
                for (uint8_t counter = 0; counter < CPS_LOC_MAX; counter++)
                {
                    if ((p_value->supp_sensor_loc >> counter) & 0x0001)
                    {
                        p_buf[0] = counter;
                        p_buf += 1;
                    }
                }
            }
            break;
            case (CPS_CTRL_PT_SET_CRANK_LENGTH): { /* Nothing to do */ } break;
            case (CPS_CTRL_PT_REQ_CRANK_LENGTH):
            {
                // Set the response parameter
                *(uint16_t *)p_buf = co_host_to_uint16_le(p_value->crank_length);
                p_buf += 2;
            }
            break;

            case (CPS_CTRL_PT_SET_CHAIN_LENGTH): { /* Nothing to do */ } break;
            case (CPS_CTRL_PT_REQ_CHAIN_LENGTH):
            {
                // Set the response parameter
                *(uint16_t *)p_buf = co_host_to_uint16_le(p_value->chain_length);
                p_buf += 2;
            }
            break;
            case (CPS_CTRL_PT_SET_CHAIN_WEIGHT): { /* Nothing to do */ } break;
            case (CPS_CTRL_PT_REQ_CHAIN_WEIGHT):
            {
                // Set the response parameter
                *(uint16_t *)p_buf = co_host_to_uint16_le(p_value->chain_weight);
                p_buf += 2;
            }
            break;

            case (CPS_CTRL_PT_SET_SPAN_LENGTH): { /* Nothing to do */ } break;
            case (CPS_CTRL_PT_REQ_SPAN_LENGTH):
            {
                // Set the response parameter
                *(uint16_t *)p_buf = co_host_to_uint16_le(p_value->span_length);
                p_buf += 2;
            }
            break;

            case (CPS_CTRL_PT_START_OFFSET_COMP):
            {
                // Set the response parameter
                *(uint16_t *)p_buf = co_host_to_uint16_le(p_value->offset_comp);
                p_buf += 2;
            }
            break;

            case (CPS_CTRL_MASK_CP_MEAS_CH_CONTENT):
            {
                uint16_t ble_cps_mask_cp_meas_flags [] =
                {
                    CPS_MEAS_PEDAL_POWER_BALANCE_PRESENT_BIT,
                    CPS_MEAS_ACCUM_TORQUE_PRESENT_BIT,
                    CPS_MEAS_WHEEL_REV_DATA_PRESENT_BIT,
                    CPS_MEAS_CRANK_REV_DATA_PRESENT_BIT,
                    CPS_MEAS_EXTREME_FORCE_MAGNITUDES_PRESENT_BIT |
                    CPS_MEAS_EXTREME_TORQUE_MAGNITUDES_PRESENT_BIT,
                    CPS_MEAS_EXTREME_ANGLES_PRESENT_BIT,
                    CPS_MEAS_TOP_DEAD_SPOT_ANGLE_PRESENT_BIT,
                    CPS_MEAS_BOTTOM_DEAD_SPOT_ANGLE_PRESENT_BIT,
                    CPS_MEAS_ACCUM_ENERGY_PRESENT_BIT,
                };

                uint16_t mask = 0;

                for (uint8_t count = 0; count < 9; count++)
                {
                    if ((p_value->mask_meas_content >> count) & 0x0001)
                    {
                        mask |= ble_cps_mask_cp_meas_flags[count];
                    }
                }

                p_cps_env->env[conidx].mask_meas_content = mask;
            }
            break;

            case (CPS_CTRL_REQ_SAMPLING_RATE):
            {
                // Set the response parameter
                p_buf[0] = p_value->sampling_rate;
                p_buf += 1;
            }
            break;

            case (CPS_CTRL_REQ_FACTORY_CALIBRATION_DATE):
            {
                // Set the response parameter
                p_buf += ble_prf_pack_date_time(p_buf, &(p_value->factory_calibration));
            }
            break;

            case (CPS_CTRL_START_ENHANCED_OFFSET_COMP):
            {
                uint8_t cursor;
                uint8_t length;

                // Set the response parameter
                *(uint16_t *)p_buf = co_host_to_uint16_le(p_value->enhanced_offset_comp.comp_offset);
                p_buf += 2;
                // manufacturer company ID
                *(uint16_t *)p_buf = co_host_to_uint16_le(p_value->enhanced_offset_comp.manu_comp_id);
                p_buf += 2;
                // length
                length = co_min(p_value->enhanced_offset_comp.length,
                                CPS_CP_CTRL_PT_RSP_MAX_LEN - (p_buf - p_buf_head) - 1);

                p_buf[0] = length;
                p_buf += 1;

                for (cursor = 0; cursor < length ; cursor++)
                {
                    p_buf[0] = p_value->enhanced_offset_comp.data[cursor];
                    p_buf += 1;
                }
            }
            break;
            default: { /* Nothing to do */ } break;
        }
    }
    else
    {
        // Operation results in an error condition
        if ((p_cps_env->ctrl_pt_op == CPS_CTRL_START_ENHANCED_OFFSET_COMP)
                && (p_value->enhanced_offset_comp.rsp_param != CPS_CTRL_PT_ERR_RSP_PARAM_INCORRECT_CALIB_POS)
                && (p_value->enhanced_offset_comp.rsp_param != CPS_CTRL_PT_ERR_RSP_PARAM_MANUF_SPEC_ERR_FOLLOWS))
        {
            resp_val = CPS_CTRL_PT_RESP_INV_PARAM;
        }

        // Set the Response Value
        p_buf[0] = (resp_val > CPS_CTRL_PT_RESP_FAILED) ? CPS_CTRL_PT_RESP_FAILED : resp_val;
        p_buf += 1;

        if (p_cps_env->ctrl_pt_op == CPS_CTRL_START_ENHANCED_OFFSET_COMP)
        {
            // Response parameter
            p_buf[0] = p_value->enhanced_offset_comp.rsp_param;
            p_buf += 1;

            if (p_value->enhanced_offset_comp.rsp_param == CPS_CTRL_PT_ERR_RSP_PARAM_MANUF_SPEC_ERR_FOLLOWS)
            {
                uint8_t cursor;
                uint8_t length;

                // manufacturer company ID
                *(uint16_t *)p_buf = co_host_to_uint16_le(p_value->enhanced_offset_comp.manu_comp_id);
                p_buf += 2;
                // length
                length = co_min(p_value->enhanced_offset_comp.length,
                                CPS_CP_CTRL_PT_RSP_MAX_LEN - (p_buf - p_buf_head) - 1);

                p_buf[0] = length;
                p_buf += 1;

                for (cursor = 0; cursor < length ; cursor++)
                {
                    p_buf[0] = p_value->enhanced_offset_comp.data[cursor];
                    p_buf += 1;
                }
            }
        }
    }

    return p_buf - p_buf_head;
}

static void ble_cps_exe_operation(void)
{
    if (bt_defer_curr_func_0(ble_cps_exe_operation))
    {
        return;
    }

    if (!p_cps_env->in_exe_op)
    {
        p_cps_env->in_exe_op = true;

        while (!colist_is_list_empty(&(p_cps_env->wait_queue)) && !(p_cps_env->op_ongoing))
        {
            uint16_t status = BT_STS_SUCCESS;
            ble_cps_buf_meta_t *p_meta = (ble_cps_buf_meta_t *)colist_get_head(&(p_cps_env->wait_queue));

            switch (p_meta->operation)
            {
                case CPS_NTF_VECTOR_OP_CODE:
                {
                    uint8_t  conidx;
                    uint32_t conidx_bf = 0;

                    // remove buffer from queue
                    colist_delete(&(p_meta->node));

                    // check connection that support notification reception
                    for (conidx = 0 ; conidx < BLE_CONNECTION_MAX ; conidx++)
                    {
                        if (GETB(p_cps_env->env[conidx].prfl_ntf_ind_cfg, CPS_PRF_CFG_FLAG_VECTOR_NTF))
                        {
                            conidx_bf |= CO_BIT(conidx);
                        }
                    }

                    // send notification only on selected connections
                    conidx_bf &= p_meta->conidx_bf;

                    if (conidx_bf != 0)
                    {
                        gatt_char_notify_t ntf_cfg =
                        {
                            .character = CPS_VECTOR,
                            .service = CPS_SERVICE,
                        };
                        // send multipoint notification
                        status = gatts_send_value_notification(CPS_2_GAP_CONIDX_BF(conidx_bf), &ntf_cfg, p_meta->buf, p_meta->buf_len);
                        if (status == BT_STS_SUCCESS)
                        {
                            p_cps_env->op_ongoing = true;
                        }
                    }

                    bes_bt_buf_free((uint8_t *)p_meta);

                    if (!p_cps_env->op_ongoing)
                    {
                        const ble_cps_cb_t *p_cb = (const ble_cps_cb_t *) p_cps_env->p_cb;
                        // Inform application that event has been sent
                        p_cb->cb_vector_send_cmp(status);
                    }
                }
                break;

                case CPS_NTF_MEAS_OP_CODE:
                {
                    if (p_meta->new)
                    {
                        p_meta->conidx = GAP_INVALID_CONIDX;
                        p_meta->last_flag = 0;

                        while (p_meta->conidx_bf != 0)
                        {
                            // retrieve first valid bit
                            uint8_t conidx = co_ctz(p_meta->conidx_bf);
                            p_meta->conidx_bf &= ~CO_BIT(conidx);

                            // check if notification enabled
                            if (GETB(p_cps_env->env[conidx].prfl_ntf_ind_cfg, CPS_PRF_CFG_FLAG_CP_MEAS_NTF))
                            {
                                p_meta->conidx = conidx;
                                break;
                            }
                        }
                    }

                    // Use Reliable write
                    if (p_meta->conidx != GAP_INVALID_CONIDX)
                    {
                        gatt_char_notify_t ntf_cfg =
                        {
                            .character = CPS_MEASUREMENT,
                            .service = CPS_SERVICE,
                        };
                        // send multipoint notification
                        status = gatts_send_value_notification(CPS_2_GAP_CONIDX_BF(p_meta->conidx), &ntf_cfg, p_meta->buf, p_meta->buf_len);
                        if (status == BT_STS_SUCCESS)
                        {
                            p_cps_env->op_ongoing = true;
                        }
                    }
                    else
                    {
                        const ble_cps_cb_t *p_cb = (const ble_cps_cb_t *) p_cps_env->p_cb;
                        // Inform application that event has been sent
                        p_cb->cb_meas_send_cmp(BT_STS_SUCCESS);

                        // remove buffer from queue
                        colist_delete(&(p_meta->node));

                        bes_bt_buf_free((uint8_t *)p_meta);
                    }
                }
                break;

                default:
                {
                    uint8_t conidx = p_meta->conidx;
                    // remove buffer from queue
                    colist_delete(&(p_meta->node));

                    gatt_char_notify_t ntf_cfg =
                    {
                        .character = CPS_CTRL_POINT,
                        .service = CPS_SERVICE,
                    };
                    status = gatts_send_value_indication(CPS_2_GAP_CONIDX_BF(conidx), &ntf_cfg, p_meta->buf, p_meta->buf_len);

                    if (status == BT_STS_SUCCESS)
                    {
                        p_cps_env->op_ongoing = true;
                    }
                    else
                    {
                        // Inform application that control point response has been sent
                        if (p_cps_env->ctrl_pt_op != CPS_CTRL_PT_RSP_CODE)
                        {
                            const ble_cps_cb_t *p_cb = (const ble_cps_cb_t *) p_cps_env->p_cb;
                            p_cb->cb_ctrl_pt_rsp_send_cmp(conidx, status);
                        }

                        // consider control point operation done
                        p_cps_env->ctrl_pt_op = CPS_CTRL_PT_RESERVED;
                    }

                    bes_bt_buf_free((uint8_t *)p_meta);
                }
                break;
            }
        }

        p_cps_env->in_exe_op = false;
    }
}

static void ble_cps_cb_att_read_get(uint16_t conidx, uint32_t token, const gatt_attribute_t *attr, uint16_t offset)
{
    // retrieve value attribute
    uint16_t  status      = ATT_ERROR_NO_ERROR;
    uint8_t *p_buf        = NULL;
    uint16_t  p_buf_len = 0;

    if ((p_buf = bes_bt_buf_malloc(CPS_CP_CTRL_PT_RSP_MAX_LEN)))
    {
        switch (ble_cps_idx_get(attr))
        {
            case CPS_IDX_CP_MEAS_NTF_CFG:
            {
                uint16_t ntf_cfg = GETB(p_cps_env->env[conidx].prfl_ntf_ind_cfg, CPS_PRF_CFG_FLAG_CP_MEAS_NTF)
                                   ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;

                *(uint16_t *)p_buf = co_host_to_uint16_le(ntf_cfg);
                p_buf_len += 2;
            }
            break;

            case CPS_IDX_CP_MEAS_BCST_CFG:
            {
                // Broadcast feature is profile specific
                if (CPS_IS_FEATURE_SUPPORTED(p_cps_env->prfl_cfg, CPS_MEAS_BCST_MASK))
                {
                    uint16_t bcst_cfg = GETB(p_cps_env->env[conidx].prfl_ntf_ind_cfg, CPS_PRF_CFG_FLAG_SP_MEAS_NTF)
                                        ? PRF_SRV_START_BCST : PRF_SRV_STOP_BCST;

                    *(uint16_t *)p_buf = co_host_to_uint16_le(bcst_cfg);
                    p_buf_len += 2;
                }
                else
                {
                    status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
                }
            }
            break;

            case CPS_IDX_VECTOR_NTF_CFG:
            {
                uint16_t ntf_cfg = GETB(p_cps_env->env[conidx].prfl_ntf_ind_cfg, CPS_PRF_CFG_FLAG_VECTOR_NTF)
                                   ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;

                *(uint16_t *)p_buf = co_host_to_uint16_le(ntf_cfg);
                p_buf_len += 2;
            }
            break;

            case CPS_IDX_CTRL_PT_IND_CFG:
            {
                uint16_t ind_cfg = GETB(p_cps_env->env[conidx].prfl_ntf_ind_cfg, CPS_PRF_CFG_FLAG_CTRL_PT_IND)
                                   ? PRF_CLI_START_IND : PRF_CLI_STOP_NTFIND;

                *(uint16_t *)p_buf = co_host_to_uint16_le(ind_cfg);
                p_buf_len += 2;
            }
            break;

            case CPS_IDX_CP_FEAT_CHAR:
            {
                *(uint32_t *)p_buf = co_host_to_uint16_le(p_cps_env->features);
                p_buf_len += 4;
            }
            break;

            case CPS_IDX_SENSOR_LOC_CHAR:
            {
                p_buf[0] = p_cps_env->sensor_loc;
                p_buf_len += 1;
            }
            break;

            default:
            {
                status = ATT_ERROR_REQ_NOT_SUPPORT;
            }
            break;
        }
    }
    else
    {
        status = ATT_ERROR_INSUFF_RESOURCES;
    }

    gatts_send_defer_read_rsp(app_ble_get_conhdl_from_conidx(conidx), token, status, p_buf, p_buf_len);

    if (p_buf != NULL)
    {
        bes_bt_buf_free(p_buf);
    }
}

static void ble_cps_cb_att_val_set(uint8_t conidx, uint32_t token, const gatt_attribute_t *attr, uint16_t offset, const uint8_t *p_buf, uint8_t buf_len)
{
    uint16_t status = BT_STS_FAILED;

    if (p_cps_env != NULL)
    {
        uint8_t cfg_upd_flag  = 0;
        uint16_t cfg_en_val = 0;

        switch (ble_cps_idx_get(attr))
        {
            case CPS_IDX_CP_MEAS_NTF_CFG:
            {
                cfg_upd_flag = CPS_PRF_CFG_FLAG_CP_MEAS_NTF_BIT;
                cfg_en_val   = PRF_CLI_START_NTF;
            }
            break;

            case CPS_IDX_CP_MEAS_BCST_CFG:
            {
                cfg_upd_flag = CPS_PRF_CFG_FLAG_SP_MEAS_NTF_BIT;
                cfg_en_val   = PRF_SRV_START_BCST;
            }
            break;

            case CPS_IDX_VECTOR_NTF_CFG:
            {
                cfg_upd_flag = CPS_PRF_CFG_FLAG_VECTOR_NTF_BIT;
                cfg_en_val   = PRF_CLI_START_NTF;
            }
            break;

            case CPS_IDX_CTRL_PT_IND_CFG:
            {
                cfg_upd_flag = CPS_PRF_CFG_FLAG_CTRL_PT_IND_BIT;
                cfg_en_val   = PRF_CLI_START_IND;
            }
            break;

            case CPS_IDX_CTRL_PT_CHAR:
            {
                // Check if sending of indications has been enabled
                if (!GETB(p_cps_env->env[conidx].prfl_ntf_ind_cfg, CPS_PRF_CFG_FLAG_CTRL_PT_IND))
                {
                    // CPP improperly configured
                    status = ATT_ERROR_CCCD_IMPROPER_CONFIGED;
                }
                else if (p_cps_env->ctrl_pt_op != CPS_CTRL_PT_RESERVED)
                {
                    // A procedure is already in progress
                    status = CPS_ERROR_PROC_IN_PROGRESS;
                }
                else
                {
                    // Unpack Control Point parameters
                    status = ble_cps_unpack_ctrl_point_req(conidx, p_buf, buf_len);
                }
            }
            break;

            default:
            {
                status = ATT_ERROR_REQ_NOT_SUPPORT;
            }
            break;
        }

        if (cfg_upd_flag != 0)
        {
            uint16_t cfg = CO_COMBINE_UINT16_LE(p_buf);

            // parameter check
            if (buf_len == sizeof(uint16_t)
                    && ((cfg == PRF_CLI_STOP_NTFIND) || (cfg == cfg_en_val)))
            {
                const ble_cps_cb_t *p_cb  = (const ble_cps_cb_t *) p_cps_env->p_cb;

                if (cfg == PRF_CLI_STOP_NTFIND)
                {
                    p_cps_env->env[conidx].prfl_ntf_ind_cfg &= ~cfg_upd_flag;
                }
                else
                {
                    p_cps_env->env[conidx].prfl_ntf_ind_cfg |= cfg_upd_flag;
                }

                // inform application about update
                p_cb->cb_bond_data_upd(conidx, cfg_upd_flag, cfg);
                status = BT_STS_SUCCESS;
            }
            else
            {
                status = ATT_ERROR_CCCD_IMPROPER_CONFIGED;
            }
        }
    }

    gatts_send_defer_write_rsp(app_ble_get_conhdl_from_conidx(conidx), token, status);
}

static void ble_cps_cb_event_sent(uint8_t conidx, const gatt_attribute_t *p_char_attr, uint16_t status)
{
    // Consider job done
    const ble_cps_cb_t *p_cb  = (const ble_cps_cb_t *) p_cps_env->p_cb;
    p_cps_env->op_ongoing = false;

    uint32_t dummy = ble_cps_idx_get(p_char_attr);

    if (dummy == CPS_IDX_CP_MEAS_CHAR)
    {
        dummy = CPS_NTF_MEAS_OP_CODE;
    }
    else if (dummy == CPS_IDX_VECTOR_CHAR)
    {
        dummy = CPS_NTF_VECTOR_OP_CODE;
    }
    else
    {
        dummy = CPS_CTRL_PT_RESP_OP_CODE;
    }

    switch (dummy)
    {
        case CPS_NTF_VECTOR_OP_CODE:
        {
            p_cb->cb_vector_send_cmp(status);
        }
        break;
        case CPS_NTF_MEAS_OP_CODE: { /* Nothing to do */ } break;
        default:
        {
            // Inform application that control point response has been sent
            if (p_cps_env->ctrl_pt_op != CPS_CTRL_PT_RSP_CODE)
            {
                p_cb->cb_ctrl_pt_rsp_send_cmp(conidx, status);
            }

            p_cps_env->ctrl_pt_op = CPS_CTRL_PT_RESERVED;
        }
        break;
    }

    // continue operation execution
    ble_cps_exe_operation();
}

static void ble_cps_conn_alloc(uint8_t conidx)
{
    memset(&(p_cps_env->env[conidx]), 0, sizeof(ble_cps_cnx_env_t));
}

static void ble_cps_conn_free(uint8_t conidx)
{
    memset(&(p_cps_env->env[conidx]), 0, sizeof(ble_cps_cnx_env_t));
}

static int ble_cps_gatt_server_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
{
    if (p_cps_env == NULL)
    {
        return BT_STS_NOT_READY;
    }

    if (svc->con_idx < gap_zero_based_conidx_to_ble_conidx(0))
    {
        TRACE(1, "%s not support conidx = %d", __func__, svc->con_idx);
        return false;
    }

    uint8_t conidx = gap_zero_based_conidx(svc->con_idx);

    switch (event)
    {
        case GATT_SERV_EVENT_REGISTER_CMP:
        {
            if (param.resgiter_cmp->status == BT_STS_SUCCESS)
            {
                TRACE(1, "ble cps resgiter success: shdl = %d", param.resgiter_cmp->start_handle);
            }
            else
            {
                TRACE(1, "ble cps resgited fail: status = %d", param.resgiter_cmp->status);
            }

            return true;
        }
        case GATT_SERV_EVENT_CONN_OPENED:
        {
            ble_cps_conn_alloc(conidx);
            return true;
        }
        case GATT_SERV_EVENT_CONN_CLOSED:
        {
            ble_cps_conn_free(conidx);
            return true;
        }
        case GATT_SERV_EVENT_CHAR_READ:
        {
            ble_cps_cb_att_read_get(conidx, param.char_read->ctx->token, param.char_read->char_attr,
                                    param.char_read->value_offset);
            return true;
        }
        case GATT_SERV_EVENT_DESC_READ:
        {
            ble_cps_cb_att_read_get(conidx, param.desc_read->ctx->token, param.desc_read->desc_attr,
                                    param.desc_read->value_offset);
            return true;
        }
        case GATT_SERV_EVENT_CHAR_WRITE:
        {
            ble_cps_cb_att_val_set(conidx, param.char_write->ctx->token, param.char_write->char_attr,
                                   param.char_write->value_offset, param.char_write->value,
                                   param.char_write->value_len);
            return true;
        }
        case GATT_SERV_EVENT_DESC_WRITE:
        {
            ble_cps_cb_att_val_set(conidx, param.desc_write->ctx->token, param.desc_write->desc_attr,
                                   param.desc_write->value_offset, param.desc_write->value,
                                   param.desc_write->value_len);
            return true;
        }
        case GATT_SERV_EVENT_INDICATE_CFM:
        case GATT_SERV_EVENT_NTF_TX_DONE:
        {
            ble_cps_cb_event_sent(conidx, param.confirm->char_attr, param.confirm->error_code);
            return true;
        }
        default:
        {
            break;
        }
    }

    return false;
}

static uint16_t ble_cps_prepare_service_attr_list(gatt_attribute_t **pp_attr_list, uint8_t *p_attr_list_len, uint32_t cps_cfg_flag)
{
    uint8_t idx = 0;

    if (pp_attr_list == NULL || p_attr_list_len == NULL)
    {
        return BT_STS_INVALID_PARM;
    }

    *p_attr_list_len = ARRAY_SIZE(ble_cps_att_db) - !CPS_IS_FEATURE_SUPPORTED(cps_cfg_flag, CPS_MEAS_BCST_MASK) -
                       (!CPS_IS_FEATURE_SUPPORTED(cps_cfg_flag, CPS_VECTOR_MASK)
                        + !CPS_IS_FEATURE_SUPPORTED(cps_cfg_flag, CPS_CTRL_PT_MASK)) * 2;

    uint8_t size = *p_attr_list_len * sizeof(gatt_attribute_t);

    *pp_attr_list = (gatt_attribute_t *)bes_bt_buf_malloc(size +
                                                               CPS_IS_FEATURE_SUPPORTED(cps_cfg_flag, CPS_MEAS_BCST_MASK) *
                                                               sizeof(CPS_MEASUREMENT));

    if (*pp_attr_list == NULL)
    {
        return BT_STS_NO_RESOURCES;
    }

    memcpy(*pp_attr_list, ble_cps_att_db, 3 * sizeof(gatt_attribute_t));
    idx += 3;

    // Check if the Broadcaster role shall be added.
    if (CPS_IS_FEATURE_SUPPORTED(cps_cfg_flag, CPS_MEAS_BCST_MASK))
    {
        // Point to Measurement character decalration and change it to static declaration
        (*pp_attr_list + 1)->attr_data = (attr_byte_array_t *)((uint32_t)(*pp_attr_list) + size);
        // Copy Measurement character decalration value
        memcpy((*pp_attr_list + 1)->attr_data, CPS_MEASUREMENT, sizeof(CPS_MEASUREMENT));
        // Get prop
        uint8_t *p_prop = (uint8_t *)(((*pp_attr_list + 1)->attr_data) + 1);
        // Add Broadcast property
        *p_prop |= GATT_BROADCAST;

        *(*pp_attr_list + idx) = ble_cps_att_db[CPS_IDX_CP_MEAS_BCST_CFG];
        idx += 1;
    }

    memcpy(*pp_attr_list + idx, &ble_cps_att_db[CPS_IDX_CP_FEAT_CHAR], 2 * sizeof(gatt_attribute_t));
    idx += 2;

    if (CPS_IS_FEATURE_SUPPORTED(cps_cfg_flag, CPS_VECTOR_MASK))
    {
        *(*pp_attr_list + idx) = ble_cps_att_db[CPS_IDX_VECTOR_CHAR];
        idx += 1;
        * (*pp_attr_list + idx) = ble_cps_att_db[CPS_IDX_VECTOR_NTF_CFG];
        idx += 1;
    }

    if (CPS_IS_FEATURE_SUPPORTED(cps_cfg_flag, CPS_CTRL_PT_MASK))
    {
        *(*pp_attr_list + idx) = ble_cps_att_db[CPS_IDX_CTRL_PT_CHAR];
        idx += 1;
        *(*pp_attr_list + idx) = ble_cps_att_db[CPS_IDX_CTRL_PT_IND_CFG];
        idx += 1;
    }

    return BT_STS_SUCCESS;
}

/* FUNCSTIONS */
uint16_t ble_cps_enable(uint8_t conidx, uint8_t ntf_ind_cfg)
{
    uint16_t status = BT_STS_NOT_ALLOW;

    if (p_cps_env != NULL)
    {
        // check state of the task
        if (app_ble_get_conhdl_from_conidx(conidx) != 0xFFFF)
        {
            p_cps_env->env[conidx].prfl_ntf_ind_cfg = ntf_ind_cfg;
            status = BT_STS_SUCCESS;
        }
    }

    return status;
}

uint16_t ble_cps_adv_data_pack(uint8_t *p_buf, uint8_t buf_len, const ble_cps_cp_meas_t *p_meas)
{
    uint16_t status = BT_STS_NOT_ALLOW;

    if (p_buf == NULL)
    {
        status = BT_STS_INVALID_PARM;
    }
    else if (buf_len < CPS_CP_MEAS_ADV_MAX_LEN)
    {
        status = BT_STS_INVALID_LENGTH;
    }
    else if (p_cps_env != NULL)
    {
        // Check Broadcast is supported
        if (CPS_IS_FEATURE_SUPPORTED(p_cps_env->prfl_cfg, CPS_MEAS_BCST_MASK))
        {
            uint16_t flags = 0;
            uint16_t mask_content = 0;

            // Pack Cp Measurement
            ble_cps_pack_meas(p_buf + CPS_CP_ADV_HEADER_LEN + CPS_CP_ADV_LENGTH_LEN, buf_len, p_meas, CPS_CP_MEAS_ADV_MAX_LEN, mask_content, &flags);

            // Pack UUID of CPS
            *(uint16_t *)(p_buf + 2) = co_host_to_uint16_le(GATT_UUID_CPW_SERVICE);

            // Pack Service Data AD type
            *(p_buf + 1) = GAP_DT_SERVICE_DATA_16BIT_UUID;

            // Set AD length
            p_buf[0] = (uint8_t)(buf_len - 1);

            status = BT_STS_SUCCESS;
        }
    }

    return status;
}

uint16_t ble_cps_meas_send(uint32_t conidx_bf, int16_t cumul_wheel_rev, const ble_cps_cp_meas_t *p_meas)
{
    uint16_t status = BT_STS_NOT_ALLOW;

    if (p_meas == NULL)
    {
        status = BT_STS_INVALID_PARM;
    }
    else if (p_cps_env != NULL)
    {
        uint8_t *p_buf = NULL;
        // Should be updated just once
        if (GETB(p_meas->flags, CPS_MEAS_WHEEL_REV_DATA_PRESENT))
        {
            if (GETB(p_cps_env->features, CPS_FEAT_WHEEL_REV_DATA_SUP))
            {
                // Update the cumulative wheel revolutions value stored in the environment
                // The value shall not decrement below zero
                if (cumul_wheel_rev < 0)
                {
                    p_cps_env->cumul_wheel_rev = ((uint32_t)co_abs(cumul_wheel_rev) > p_cps_env->cumul_wheel_rev)
                                                 ? 0 : (p_cps_env->cumul_wheel_rev + cumul_wheel_rev);
                }
                else
                {
                    p_cps_env->cumul_wheel_rev += cumul_wheel_rev;
                }
            }
        }

        if ((p_buf = bes_bt_buf_malloc(sizeof(ble_cps_cp_meas_t))))
        {
            ble_cps_buf_meta_t *p_buf_meta = (ble_cps_buf_meta_t *)p_buf;
            p_buf_meta->operation = CPS_NTF_MEAS_OP_CODE;
            p_buf_meta->conidx    = 0;
            p_buf_meta->conidx_bf = conidx_bf & ((1 << BLE_CONNECTION_MAX) - 1);
            p_buf_meta->new       = true;
            p_buf_meta->buf_len   = sizeof(ble_cps_cp_meas_t);

            // copy structure info - use buffer head to ensure that buffer is 32-bit aligned
            memcpy(p_buf_meta->buf, p_meas, sizeof(ble_cps_cp_meas_t));
            // put event on wait queue
            colist_addto_tail(&(p_buf_meta->node), &(p_cps_env->wait_queue));
            // execute operation
            ble_cps_exe_operation();

            status = BT_STS_SUCCESS;
        }
        else
        {
            status = BT_STS_NO_RESOURCES;
        }
    }

    return status;

}

uint16_t ble_cps_vector_send(uint32_t conidx_bf, const ble_cps_cp_vector_t *p_vector)
{
    uint16_t status = BT_STS_NOT_ALLOW;

    if (p_vector == NULL)
    {
        status = BT_STS_INVALID_PARM;
    }
    else if (p_cps_env != NULL)
    {
        if (! CPS_IS_FEATURE_SUPPORTED(p_cps_env->prfl_cfg, CPS_VECTOR_MASK))
        {
            status = BT_STS_NOT_SUPPORTED;
        }
        else
        {
            uint8_t *p_buf = NULL;

            if ((p_buf = bes_bt_buf_malloc(CPS_CP_VECTOR_MAX_LEN)))
            {
                ble_cps_buf_meta_t *p_buf_meta = (ble_cps_buf_meta_t *)p_buf;
                p_buf_meta->operation = CPS_NTF_VECTOR_OP_CODE;
                p_buf_meta->conidx    = GAP_INVALID_CONIDX;
                p_buf_meta->conidx_bf = conidx_bf & ((1 << BLE_CONNECTION_MAX) - 1);
                p_buf_meta->new       = true;
                p_buf_meta->buf_len   = CPS_CP_VECTOR_MAX_LEN;

                // Pack structure
                p_buf_meta->buf_len = ble_cps_pack_vector(p_buf_meta->buf, p_vector);
                // put event on wait queue
                colist_addto_tail(&(p_buf_meta->node), &(p_cps_env->wait_queue));
                // execute operation
                ble_cps_exe_operation();

                status = BT_STS_SUCCESS;
            }
            else
            {
                status = BT_STS_NO_RESOURCES;
            }
        }
    }

    return status;
}

uint16_t ble_cps_ctrl_pt_rsp_send(uint8_t conidx, uint8_t op_code, uint8_t resp_val, const union ble_cps_ctrl_pt_rsp_val *p_value)
{
    uint16_t status = BT_STS_NOT_ALLOW;

    if (p_value == NULL)
    {
        status = BT_STS_INVALID_PARM;
    }
    else if (p_cps_env != NULL)
    {
        do
        {
            uint8_t *p_buf = NULL;

            // Check the current operation
            if (p_cps_env->ctrl_pt_op ==  CPS_CTRL_PT_RESERVED)
            {
                // The confirmation has been sent without request indication, ignore
                break;
            }

            // The CP Control Point Characteristic must be supported if we are here
            if (!CPS_IS_FEATURE_SUPPORTED(p_cps_env->prfl_cfg, CPS_CTRL_PT_MASK))
            {
                status = BT_STS_NOT_ALLOW;
                break;
            }

            // Check if sending of indications has been enabled
            if (!GETB(p_cps_env->env[conidx].prfl_ntf_ind_cfg, CPS_PRF_CFG_FLAG_CTRL_PT_IND))
            {
                // mark operation done
                p_cps_env->ctrl_pt_op = CPS_CTRL_PT_RESERVED;
                // CPP improperly configured
                status = ATT_ERROR_CCCD_IMPROPER_CONFIGED;
                break;
            }

            if ((p_buf = bes_bt_buf_malloc(CPS_CP_CTRL_PT_RSP_MAX_LEN)))
            {
                ble_cps_buf_meta_t *p_buf_meta = (ble_cps_buf_meta_t *)p_buf;
                p_buf_meta->operation = CPS_CTRL_PT_RESP_OP_CODE;
                p_buf_meta->conidx    = conidx;
                p_buf_meta->new       = true;
                p_buf_meta->buf_len   = CPS_CP_CTRL_PT_RSP_MAX_LEN;

                // Pack structure
                p_buf_meta->buf_len = ble_cps_pack_ctrl_point_rsp(conidx, p_buf_meta->buf, op_code, resp_val, p_value);
                // put event on wait queue
                colist_addto_tail(&(p_buf_meta->node), &(p_cps_env->wait_queue));
                // execute operation
                ble_cps_exe_operation();

                status = BT_STS_SUCCESS;
            }
            else
            {
                status = BT_STS_NO_RESOURCES;
            }

        } while (0);
    }

    return status;
}

uint16_t ble_cps_init(struct ble_cps_db_cfg *p_init_cfg, const ble_cps_cb_t *p_cb)
{
    // DB Creation Status
    uint16_t status = BT_STS_SUCCESS;
    gatt_attribute_t *p_attr_list = NULL;
    uint8_t attr_list_len = 0;

    if (p_cps_env != NULL)
    {
        return BT_STS_ALREADY_REGISTERED;
    }

    do
    {
        // Service content flag
        uint32_t cfg_flag = CPS_MANDATORY_MASK;

        if ((p_init_cfg == NULL) ||
                (p_cb == NULL) || (p_cb->cb_meas_send_cmp == NULL)
                || (p_cb->cb_vector_send_cmp == NULL)  || (p_cb->cb_bond_data_upd == NULL)  || (p_cb->cb_ctrl_pt_req == NULL)
                || (p_cb->cb_ctrl_pt_rsp_send_cmp == NULL))
        {
            status = BT_STS_INVALID_PARM;
            break;
        }

        // Check if Broadcaster role shall be added.
        if (CPS_IS_FEATURE_SUPPORTED(p_init_cfg->prfl_config, CPS_BROADCASTER_SUPP_FLAG))
        {
            //Add configuration to the database
            cfg_flag |= CPS_MEAS_BCST_MASK;
        }

        // Check if the CP Vector characteristic shall be added.
        // Mandatory if at least one Vector procedure is supported, otherwise excluded.
        if (GETB(p_init_cfg->cp_feature, CPS_FEAT_CRANK_REV_DATA_SUP)
                || GETB(p_init_cfg->cp_feature, CPS_FEAT_EXTREME_ANGLES_SUP)
                || GETB(p_init_cfg->cp_feature, CPS_FEAT_INSTANT_MEAS_DIRECTION_SUP))
        {
            cfg_flag |= CPS_VECTOR_MASK;
        }

        // Check if the Control Point characteristic shall be added
        // Mandatory if server supports:
        //     - Wheel Revolution Data
        //     - Multiple Sensor Locations
        //     - Configurable Settings (CPS_CTRL_PT_SET codes)
        //     - Offset Compensation
        //     - Server allows to be requested for parameters (CPS_CTRL_PT_REQ codes)
        if (CPS_IS_FEATURE_SUPPORTED(p_init_cfg->prfl_config, CPS_CTRL_PT_CHAR_SUPP_FLAG)
                || GETB(p_init_cfg->cp_feature, CPS_FEAT_WHEEL_REV_DATA_SUP)
                || GETB(p_init_cfg->cp_feature, CPS_FEAT_MULT_SENSOR_LOC_SUP)
                || GETB(p_init_cfg->cp_feature, CPS_FEAT_CRANK_LENGTH_ADJ_SUP)
                || GETB(p_init_cfg->cp_feature, CPS_FEAT_CHAIN_LENGTH_ADJ_SUP)
                || GETB(p_init_cfg->cp_feature, CPS_FEAT_CHAIN_WEIGHT_ADJ_SUP)
                || GETB(p_init_cfg->cp_feature, CPS_FEAT_SPAN_LENGTH_ADJ_SUP)
                || GETB(p_init_cfg->cp_feature, CPS_FEAT_OFFSET_COMP_SUP))
        {
            cfg_flag |= CPS_CTRL_PT_MASK;
        }

        gatts_cfg_t cfg =
        {
            .preferred_mtu = CPS_CP_MEAS_MAX_LEN + 3,
        };

        status = ble_cps_prepare_service_attr_list(&p_attr_list, &attr_list_len, cfg_flag);

        if (status == BT_STS_SUCCESS)
        {
            // register CPS user
            status = gatts_register_service(p_attr_list, attr_list_len, ble_cps_gatt_server_callback, &cfg);
        }

        if (status != BT_STS_SUCCESS)
        {
            break;
        }

        p_cps_env = (ble_cps_env_t *) bes_bt_buf_malloc(sizeof(ble_cps_env_t));

        if (p_cps_env != NULL)
        {
            // allocate CPS required environment variable
            p_cps_env->prfl_cfg        = cfg_flag;
            p_cps_env->features        = p_init_cfg->cp_feature;
            p_cps_env->sensor_loc      = p_init_cfg->sensor_loc;
            p_cps_env->cumul_wheel_rev = p_init_cfg->wheel_rev;
            p_cps_env->op_ongoing      = false;
            p_cps_env->in_exe_op       = false;
            p_cps_env->ctrl_pt_op      = CPS_CTRL_PT_RESERVED;
            p_cps_env->p_attr_list     = p_attr_list;
            memset(p_cps_env->env, 0, sizeof(p_cps_env->env));
            INIT_LIST_HEAD(&(p_cps_env->wait_queue));
            // initialize profile environment variable
            p_cps_env->p_cb     = p_cb;
        }
        else
        {
            status = BT_STS_NO_RESOURCES;
        }

    } while (0);

    if (status != BT_STS_SUCCESS && p_attr_list != NULL)
    {
        gatts_unregister_service(p_attr_list);
        bes_bt_buf_free((uint8_t *)p_attr_list);
    }

    return status;
}

uint16_t ble_cps_deinit(void)
{
    if (p_cps_env != NULL)
    {
        // remove buffer in wait queue
        while (!colist_is_list_empty(&p_cps_env->wait_queue))
        {
            ble_cps_buf_meta_t *p_meta = (ble_cps_buf_meta_t *)colist_get_head(&p_cps_env->wait_queue);
            colist_delete(&(p_meta->node));
            bes_bt_buf_free((uint8_t *)p_meta);
        }

        gatts_unregister_service(p_cps_env->p_attr_list);
        bes_bt_buf_free((uint8_t *)p_cps_env->p_attr_list);

        // free profile environment variables
        bes_bt_buf_free((uint8_t *)p_cps_env);
        p_cps_env = NULL;
    }

    return BT_STS_SUCCESS;
}

#endif /* BLE_CPS_ENABLED */