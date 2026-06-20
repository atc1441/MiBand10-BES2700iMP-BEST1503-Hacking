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

#ifndef _CPS_COMMON_I_H_
#define _CPS_COMMON_I_H_

#include "prf_types.h"
#include "co_math.h"

/// Procedure Already in Progress Error Code
#define CPS_ERROR_PROC_IN_PROGRESS        (0xFE)

/// CP Measurement Notification Value Max Length
#define CPS_CP_MEAS_MAX_LEN               (34)
/// CP Measurement Value Min Length
#define CPS_CP_MEAS_NTF_MIN_LEN           (4)

/// ADV Header size
#define CPS_CP_ADV_HEADER_LEN             (3)
/// ADV Length size
#define CPS_CP_ADV_LENGTH_LEN             (1)

/// CP Measurement Advertisement Value Max Length
#define CPS_CP_MEAS_ADV_MAX_LEN           (CPS_CP_MEAS_MAX_LEN + CPS_CP_ADV_HEADER_LEN)
/// CP Measurement Value Min Length
#define CPS_CP_MEAS_ADV_MIN_LEN           (CPS_CP_MEAS_NTF_MIN_LEN + CPS_CP_ADV_HEADER_LEN)

/// CP Vector Value Max Length
#define CPS_CP_VECTOR_MAX_LEN             (19)
/// CP Vector Value Min Length
#define CPS_CP_VECTOR_MIN_LEN             (1)
/// Maximum number of torque present in vector
#define CPS_MAX_TORQUE_NB                 (6)

/// CP Control Point Value Max Length
#define CPS_CP_CTRL_PT_REQ_MAX_LEN        (9)
/// CP Control Point Value Min Length
#define CPS_CP_CTRL_PT_REQ_MIN_LEN        (1)

/// CP Control Point Value Max Length
#define CPS_CP_CTRL_PT_RSP_MAX_LEN        (20)
/// CP Control Point Value Min Length
#define CPS_CP_CTRL_PT_RSP_MIN_LEN        (3)
/// Maximum manufacturer data length in control point response
#define CPS_MAX_MANF_DATA_LEN             (13)

/// CPS Measurement all supported
#define CPS_MEAS_ALL_SUPP                 (0x1FFF)

/// CPS Feature all supported
#define CPS_FEAT_ALL_SUPP                 (0x0007FFFF)

/// CPS Vector all supported
#define CPS_VECTOR_ALL_SUPP               (0x3F)

/// CPS CT Point timeout (in milliseconds)
#define CPS_CP_TIMEOUT                    (30000)

/// CPS Service Characteristics
enum cps_char
{
    /// CPS Measurement
    CPS_MEAS_CHAR,
    /// CPS Feature
    CPS_FEAT_CHAR,
    /// Sensor Location
    CPS_SENSOR_LOC_CHAR,
    ///Cycling Power Vector
    CPS_VECTOR_CHAR,
    /// CP Control Point
    CPS_CTRL_PT_CHAR,

    CPS_CHAR_MAX,
};

/// CPS Measurement Flags bit field
enum cps_meas_flags_bf
{
    /// Pedal Power Balance Present
    CPS_MEAS_PEDAL_POWER_BALANCE_PRESENT_POS        = 0,
    CPS_MEAS_PEDAL_POWER_BALANCE_PRESENT_BIT        = CO_BIT(CPS_MEAS_PEDAL_POWER_BALANCE_PRESENT_POS),

    /// Pedal Power Balance Reference
    CPS_MEAS_PEDAL_POWER_BALANCE_REFERENCE_POS      = 1,
    CPS_MEAS_PEDAL_POWER_BALANCE_REFERENCE_BIT      = CO_BIT(CPS_MEAS_PEDAL_POWER_BALANCE_REFERENCE_POS),

    /// Accumulated Torque Present
    CPS_MEAS_ACCUM_TORQUE_PRESENT_POS               = 2,
    CPS_MEAS_ACCUM_TORQUE_PRESENT_BIT               = CO_BIT(CPS_MEAS_ACCUM_TORQUE_PRESENT_POS),

    /// Accumulated Torque Source
    CPS_MEAS_ACCUM_TORQUE_SOURCE_POS                = 3,
    CPS_MEAS_ACCUM_TORQUE_SOURCE_BIT                = CO_BIT(CPS_MEAS_ACCUM_TORQUE_SOURCE_POS),

    /// Wheel Revolution Data Present
    CPS_MEAS_WHEEL_REV_DATA_PRESENT_POS             = 4,
    CPS_MEAS_WHEEL_REV_DATA_PRESENT_BIT             = CO_BIT(CPS_MEAS_WHEEL_REV_DATA_PRESENT_POS),

    /// Crank Revolution Data Present
    CPS_MEAS_CRANK_REV_DATA_PRESENT_POS             = 5,
    CPS_MEAS_CRANK_REV_DATA_PRESENT_BIT             = CO_BIT(CPS_MEAS_CRANK_REV_DATA_PRESENT_POS),

    /// Extreme Force Magnitudes Present
    CPS_MEAS_EXTREME_FORCE_MAGNITUDES_PRESENT_POS   = 6,
    CPS_MEAS_EXTREME_FORCE_MAGNITUDES_PRESENT_BIT   = CO_BIT(CPS_MEAS_EXTREME_FORCE_MAGNITUDES_PRESENT_POS),

    /// Extreme Torque Magnitudes Present
    CPS_MEAS_EXTREME_TORQUE_MAGNITUDES_PRESENT_POS  = 7,
    CPS_MEAS_EXTREME_TORQUE_MAGNITUDES_PRESENT_BIT  = CO_BIT(CPS_MEAS_EXTREME_TORQUE_MAGNITUDES_PRESENT_POS),

    /// Extreme Angles Present
    CPS_MEAS_EXTREME_ANGLES_PRESENT_POS             = 8,
    CPS_MEAS_EXTREME_ANGLES_PRESENT_BIT             = CO_BIT(CPS_MEAS_EXTREME_ANGLES_PRESENT_POS),

    /// Top Dead Spot Angle Present
    CPS_MEAS_TOP_DEAD_SPOT_ANGLE_PRESENT_POS        = 9,
    CPS_MEAS_TOP_DEAD_SPOT_ANGLE_PRESENT_BIT        = CO_BIT(CPS_MEAS_TOP_DEAD_SPOT_ANGLE_PRESENT_POS),

    /// Bottom Dead Spot Angle Present
    CPS_MEAS_BOTTOM_DEAD_SPOT_ANGLE_PRESENT_POS     = 10,
    CPS_MEAS_BOTTOM_DEAD_SPOT_ANGLE_PRESENT_BIT     = CO_BIT(CPS_MEAS_BOTTOM_DEAD_SPOT_ANGLE_PRESENT_POS),

    /// Accumulated Energy Present
    CPS_MEAS_ACCUM_ENERGY_PRESENT_POS               = 11,
    CPS_MEAS_ACCUM_ENERGY_PRESENT_BIT               = CO_BIT(CPS_MEAS_ACCUM_ENERGY_PRESENT_POS),

    /// Offset Compensation Indicator
    CPS_MEAS_OFFSET_COMPENSATION_INDICATOR_POS      = 12,
    CPS_MEAS_OFFSET_COMPENSATION_INDICATOR_BIT      = CO_BIT(CPS_MEAS_OFFSET_COMPENSATION_INDICATOR_POS),

    // bit 13 - 15 RFU
};

/// CPS Feature Flags bit field
enum cps_feat_flags_bf
{
    /// Pedal Power Balance Supported
    CPS_FEAT_PEDAL_POWER_BALANCE_SUP_POS                = 0,
    CPS_FEAT_PEDAL_POWER_BALANCE_SUP_BIT                = CO_BIT(CPS_FEAT_PEDAL_POWER_BALANCE_SUP_POS),

    /// Accumulated Torque Supported
    CPS_FEAT_ACCUM_TORQUE_SUP_POS                       = 1,
    CPS_FEAT_ACCUM_TORQUE_SUP_BIT                       = CO_BIT(CPS_FEAT_ACCUM_TORQUE_SUP_POS),

    /// Wheel Revolution Data Supported
    CPS_FEAT_WHEEL_REV_DATA_SUP_POS                     = 2,
    CPS_FEAT_WHEEL_REV_DATA_SUP_BIT                     = CO_BIT(CPS_FEAT_WHEEL_REV_DATA_SUP_POS),

    /// Crank Revolution Data Supported
    CPS_FEAT_CRANK_REV_DATA_SUP_POS                     = 3,
    CPS_FEAT_CRANK_REV_DATA_SUP_BIT                     = CO_BIT(CPS_FEAT_CRANK_REV_DATA_SUP_POS),

    /// Extreme Magnitudes Supported
    CPS_FEAT_EXTREME_MAGNITUDES_SUP_POS                 = 4,
    CPS_FEAT_EXTREME_MAGNITUDES_SUP_BIT                 = CO_BIT(CPS_FEAT_EXTREME_MAGNITUDES_SUP_POS),

    /// Extreme Angles Supported
    CPS_FEAT_EXTREME_ANGLES_SUP_POS                     = 5,
    CPS_FEAT_EXTREME_ANGLES_SUP_BIT                     = CO_BIT(CPS_FEAT_EXTREME_ANGLES_SUP_POS),

    /// Top and Bottom Dead Spot Angles Supported
    CPS_FEAT_TOPBOT_DEAD_SPOT_ANGLES_SUP_POS            = 6,
    CPS_FEAT_TOPBOT_DEAD_SPOT_ANGLES_SUP_BIT            = CO_BIT(CPS_FEAT_TOPBOT_DEAD_SPOT_ANGLES_SUP_POS),

    /// Accumulated Energy Supported
    CPS_FEAT_ACCUM_ENERGY_SUP_POS                       = 7,
    CPS_FEAT_ACCUM_ENERGY_SUP_BIT                       = CO_BIT(CPS_FEAT_ACCUM_ENERGY_SUP_POS),

    /// Offset Compensation Indicator Supported
    CPS_FEAT_OFFSET_COMP_IND_SUP_POS                    = 8,
    CPS_FEAT_OFFSET_COMP_IND_SUP_BIT                    = CO_BIT(CPS_FEAT_OFFSET_COMP_IND_SUP_POS),

    /// Offset Compensation Supported
    CPS_FEAT_OFFSET_COMP_SUP_POS                        = 9,
    CPS_FEAT_OFFSET_COMP_SUP_BIT                        = CO_BIT(CPS_FEAT_OFFSET_COMP_SUP_POS),

    /// CP Measurement CH Content Masking Supported
    CPS_FEAT_CP_MEAS_CH_CONTENT_MASKING_SUP_POS         = 10,
    CPS_FEAT_CP_MEAS_CH_CONTENT_MASKING_SUP_BIT         = CO_BIT(CPS_FEAT_CP_MEAS_CH_CONTENT_MASKING_SUP_POS),

    /// Multiple Sensor Locations Supported
    CPS_FEAT_MULT_SENSOR_LOC_SUP_POS                    = 11,
    CPS_FEAT_MULT_SENSOR_LOC_SUP_BIT                    = CO_BIT(CPS_FEAT_MULT_SENSOR_LOC_SUP_POS),

    /// Crank Length Adjustment Supported
    CPS_FEAT_CRANK_LENGTH_ADJ_SUP_POS                   = 12,
    CPS_FEAT_CRANK_LENGTH_ADJ_SUP_BIT                   = CO_BIT(CPS_FEAT_CRANK_LENGTH_ADJ_SUP_POS),

    /// Chain Length Adjustment Supported
    CPS_FEAT_CHAIN_LENGTH_ADJ_SUP_POS                   = 13,
    CPS_FEAT_CHAIN_LENGTH_ADJ_SUP_BIT                   = CO_BIT(CPS_FEAT_CHAIN_LENGTH_ADJ_SUP_POS),

    /// Chain Weight Adjustment Supported
    CPS_FEAT_CHAIN_WEIGHT_ADJ_SUP_POS                   = 14,
    CPS_FEAT_CHAIN_WEIGHT_ADJ_SUP_BIT                   = CO_BIT(CPS_FEAT_CHAIN_WEIGHT_ADJ_SUP_POS),

    /// Span Length Adjustment Supported
    CPS_FEAT_SPAN_LENGTH_ADJ_SUP_POS                    = 15,
    CPS_FEAT_SPAN_LENGTH_ADJ_SUP_BIT                    = CO_BIT(CPS_FEAT_SPAN_LENGTH_ADJ_SUP_POS),

    /// Sensor Measurement Context
    CPS_FEAT_SENSOR_MEAS_CONTEXT_POS                    = 16,
    CPS_FEAT_SENSOR_MEAS_CONTEXT_BIT                    = CO_BIT(CPS_FEAT_SENSOR_MEAS_CONTEXT_POS),

    /// Instantaneous Measurement Direction Supported
    CPS_FEAT_INSTANT_MEAS_DIRECTION_SUP_POS             = 17,
    CPS_FEAT_INSTANT_MEAS_DIRECTION_SUP_BIT             = CO_BIT(CPS_FEAT_INSTANT_MEAS_DIRECTION_SUP_POS),

    /// Factory Calibration Date Supported
    CPS_FEAT_FACTORY_CALIBRATION_DATE_SUP_POS           = 18,
    CPS_FEAT_FACTORY_CALIBRATION_DATE_SUP_BIT           = CO_BIT(CPS_FEAT_FACTORY_CALIBRATION_DATE_SUP_POS),

    /// Enhanced Offset Compensation Supported
    CPS_FEAT_ENHANCED_OFFSET_COMPENS_SUP_POS            = 19,
    CPS_FEAT_ENHANCED_OFFSET_COMPENS_SUP_BIT            = CO_BIT(CPS_FEAT_ENHANCED_OFFSET_COMPENS_SUP_POS),

    /// Distribute System Supported LSB
    CPS_FEAT_FACTORY_CALIBRATION_DATE_SUP_LSB_POS       = 20,
    CPS_FEAT_FACTORY_CALIBRATION_DATE_SUP_LSB_BIT       = CO_BIT(CPS_FEAT_FACTORY_CALIBRATION_DATE_SUP_LSB_POS),

    /// Distribute System Supported MSB
    CPS_FEAT_FACTORY_CALIBRATION_DATE_SUP_MSB_POS       = 21,
    CPS_FEAT_FACTORY_CALIBRATION_DATE_SUP_MSB_BIT       = CO_BIT(CPS_FEAT_FACTORY_CALIBRATION_DATE_SUP_MSB_POS),

    // bit 22 - 31 RFU
};

/// CPS Sensor Locations Keys
enum cps_sensor_loc
{
    /// Other (0)
    CPS_LOC_OTHER          = 0,
    /// Top of shoe (1)
    CPS_LOC_TOP_SHOE,
    /// In shoe (2)
    CPS_LOC_IN_SHOE,
    /// Hip (3)
    CPS_LOC_HIP,
    /// Front Wheel (4)
    CPS_LOC_FRONT_WHEEL,
    /// Left Crank (5)
    CPS_LOC_LEFT_CRANK,
    /// Right Crank (6)
    CPS_LOC_RIGHT_CRANK,
    /// Left Pedal (7)
    CPS_LOC_LEFT_PEDAL,
    /// Right Pedal (8)
    CPS_LOC_RIGHT_PEDAL,
    /// Front Hub (9)
    CPS_LOC_FRONT_HUB,
    /// Rear Dropout (10)
    CPS_LOC_REAR_DROPOUT,
    /// Chainstay (11)
    CPS_LOC_CHAINSTAY,
    /// Rear Wheel (12)
    CPS_LOC_REAR_WHEEL,
    /// Rear Hub (13)
    CPS_LOC_REAR_HUB,
    /// Chest (14)
    CPS_LOC_CHEST,

    CPS_LOC_MAX,
};

/// CPS Vector Flags bit field
enum cps_vector_flags_bf
{
    /// Crank Revolution Data Present
    CPS_VECTOR_CRANK_REV_DATA_PRESENT_POS               = 0,
    CPS_VECTOR_CRANK_REV_DATA_PRESENT_BIT               = CO_BIT(CPS_VECTOR_CRANK_REV_DATA_PRESENT_POS),

    /// First Crank Measurement Angle Present
    CPS_VECTOR_FIRST_CRANK_MEAS_ANGLE_PRESENT_POS       = 1,
    CPS_VECTOR_FIRST_CRANK_MEAS_ANGLE_PRESENT_BIT       = CO_BIT(CPS_VECTOR_FIRST_CRANK_MEAS_ANGLE_PRESENT_POS),

    /// Instantaneous Force Magnitude Array Present
    CPS_VECTOR_INST_FORCE_MAGNITUDE_ARRAY_PRESENT_POS   = 2,
    CPS_VECTOR_INST_FORCE_MAGNITUDE_ARRAY_PRESENT_BIT   = CO_BIT(CPS_VECTOR_INST_FORCE_MAGNITUDE_ARRAY_PRESENT_POS),

    /// Instantaneous Torque Magnitude Array Present
    CPS_VECTOR_INST_TORQUE_MAGNITUDE_ARRAY_PRESENT_POS  = 3,
    CPS_VECTOR_INST_TORQUE_MAGNITUDE_ARRAY_PRESENT_BIT  = CO_BIT(CPS_VECTOR_INST_TORQUE_MAGNITUDE_ARRAY_PRESENT_POS),

    /// Instantaneous Measurement Direction LSB
    CPS_VECTOR_INST_MEAS_DIRECTION_LSB_POS              = 4,
    CPS_VECTOR_INST_MEAS_DIRECTION_LSB_BIT              = CO_BIT(CPS_VECTOR_INST_MEAS_DIRECTION_LSB_POS),

    /// Instantaneous Measurement Direction MSB
    CPS_VECTOR_INST_MEAS_DIRECTION_MSB_POS              = 5,
    CPS_VECTOR_INST_MEAS_DIRECTION_MSB_BIT              = CO_BIT(CPS_VECTOR_INST_MEAS_DIRECTION_MSB_POS),

    // bit 6 - 7 RFU
};

/// CPS Control Point Code Keys
enum cps_ctrl_pt_code
{
    /// Reserved value
    CPS_CTRL_PT_RESERVED                    = 0,

    /// Set Cumulative Value
    CPS_CTRL_PT_SET_CUMUL_VAL               = 1,
    /// Update Sensor Location
    CPS_CTRL_PT_UPD_SENSOR_LOC              = 2,
    /// Request Supported Sensor Locations
    CPS_CTRL_PT_REQ_SUPP_SENSOR_LOC         = 3,
    /// Set Crank Length
    CPS_CTRL_PT_SET_CRANK_LENGTH            = 4,
    /// Request Crank Length
    CPS_CTRL_PT_REQ_CRANK_LENGTH            = 5,
    /// Set Chain Length
    CPS_CTRL_PT_SET_CHAIN_LENGTH            = 6,
    /// Request Chain Length
    CPS_CTRL_PT_REQ_CHAIN_LENGTH            = 7,
    /// Set Chain Weight
    CPS_CTRL_PT_SET_CHAIN_WEIGHT            = 8,
    /// Request Chain Weight
    CPS_CTRL_PT_REQ_CHAIN_WEIGHT            = 9,
    /// Set Span Length
    CPS_CTRL_PT_SET_SPAN_LENGTH             = 10,
    /// Request Span Length
    CPS_CTRL_PT_REQ_SPAN_LENGTH             = 11,
    /// Start Offset Compensation
    CPS_CTRL_PT_START_OFFSET_COMP           = 12,
    /// Mask CP Measurement Characteristic Content
    CPS_CTRL_MASK_CP_MEAS_CH_CONTENT        = 13,
    /// Request Sampling Rate
    CPS_CTRL_REQ_SAMPLING_RATE              = 14,
    /// Request Factory Calibration Date
    CPS_CTRL_REQ_FACTORY_CALIBRATION_DATE   = 15,
    /// Start Enhanced Offset Compensation
    CPS_CTRL_START_ENHANCED_OFFSET_COMP     = 16,

    // Value 17 - 31 RFU

    /// Response Code
    CPS_CTRL_PT_RSP_CODE                    = 32,

    // Value 33 - 255 RFU
};

/// CPS Control Point Response Value
enum cps_ctrl_pt_resp_val
{
    /// Reserved value
    CPS_CTRL_PT_RESP_RESERVED      = 0,

    /// Success
    CPS_CTRL_PT_RESP_SUCCESS,
    /// Operation Code Not Supported
    CPS_CTRL_PT_RESP_NOT_SUPP,
    /// Invalid Parameter
    CPS_CTRL_PT_RESP_INV_PARAM,
    /// Operation Failed
    CPS_CTRL_PT_RESP_FAILED,
};

/// CPS Control Point Error Response Parameter
enum cps_ctrl_pt_err_resp_param
{
    /// Reserved value
    CPS_CTRL_PT_ERR_RSP_PARAM_RESERVED = 0x00,

    /// Incorrect calibration position
    CPS_CTRL_PT_ERR_RSP_PARAM_INCORRECT_CALIB_POS = 0x01,

    // Value 0x02 - 0xFE RFU

    /// Manufacturer specific error follows
    CPS_CTRL_PT_ERR_RSP_PARAM_MANUF_SPEC_ERR_FOLLOWS = 0xFF,

};

/// CP Measurement
typedef struct cps_cp_meas
{
    /// Flags
    uint16_t flags;
    /// Instantaneous Power
    int16_t inst_power;
    /// Pedal Power Balance
    uint8_t pedal_power_balance;
    /// Accumulated torque
    uint16_t accum_torque;
    /// Cumulative Wheel Revolutions
    uint32_t cumul_wheel_rev;  // Ignored on server side
    /// Last Wheel Event Time
    uint16_t last_wheel_evt_time;
    /// Cumulative Crank Revolution
    uint16_t cumul_crank_rev;
    /// Last Crank Event Time
    uint16_t last_crank_evt_time;
    /// Maximum Force Magnitude
    int16_t max_force_magnitude;
    /// Minimum Force Magnitude
    int16_t min_force_magnitude;
    /// Maximum Torque Magnitude
    int16_t max_torque_magnitude;
    /// Minimum Torque Magnitude
    int16_t min_torque_magnitude;
    /// Maximum Angle (12 bits)
    uint16_t max_angle;
    /// Minimum Angle (12bits)
    uint16_t min_angle;
    /// Top Dead Spot Angle
    uint16_t top_dead_spot_angle;
    /// Bottom Dead Spot Angle
    uint16_t bot_dead_spot_angle;
    ///Accumulated energy
    uint16_t accum_energy;
} ble_cps_cp_meas_t;

/// CP Measurement
typedef struct cps_cp_meas_ind
{
    /// Flags
    uint16_t flags;
    /// Instantaneous Power
    int16_t inst_power;
    /// Pedal Power Balance
    uint8_t pedal_power_balance;
    /// Accumulated torque
    uint16_t accum_torque;
    /// Cumulative Wheel Revolutions
    uint32_t cumul_wheel_rev;
    /// Last Wheel Event Time
    uint16_t last_wheel_evt_time;
    /// Cumulative Crank Revolution
    uint16_t cumul_crank_rev;
    /// Last Crank Event Time
    uint16_t last_crank_evt_time;
    /// Maximum Force Magnitude
    int16_t max_force_magnitude;
    /// Minimum Force Magnitude
    int16_t min_force_magnitude;
    /// Maximum Torque Magnitude
    int16_t max_torque_magnitude;
    /// Minimum Torque Magnitude
    int16_t min_torque_magnitude;
    /// Maximum Angle (12 bits)
    uint16_t max_angle;
    /// Minimum Angle (12bits)
    uint16_t min_angle;
    /// Top Dead Spot Angle
    uint16_t top_dead_spot_angle;
    /// Bottom Dead Spot Angle
    uint16_t bot_dead_spot_angle;
    /// Accumulated energy
    uint16_t accum_energy;
} ble_cps_cp_meas_ind_t;

/// Enhanced Offset Compensation
typedef struct cps_enhanced_offset_comp
{
    /// Offset
    int16_t  comp_offset;
    /// Manufacturer Company ID
    uint16_t manu_comp_id;
    /// Response Parameter (error condition)
    uint8_t  rsp_param;
    /// Manufacturer specific data length
    uint8_t  length;
    /// Manufacturer specific data
    uint8_t  data[CPS_MAX_MANF_DATA_LEN];
} ble_cps_enhanced_offset_comp_t;

/// CP Vector
typedef struct cps_cp_vector
{
    /// Flags
    uint8_t  flags;
    /// Force-Torque Magnitude Array Length
    uint8_t  nb;
    /// Cumulative Crank Revolutions
    uint16_t cumul_crank_rev;
    /// Last Crank Event Time
    uint16_t last_crank_evt_time;
    /// First Crank Measurement Angle
    uint16_t first_crank_meas_angle;
    /// Mutually excluded Force and Torque Magnitude Arrays
    int16_t  force_torque_magnitude[CPS_MAX_TORQUE_NB];
} ble_cps_cp_vector_t;

/// CP Control Point Request Value
union ble_cps_ctrl_pt_req_val
{
    /// Cumulative Value
    uint32_t cumul_val;
    /// Sensor Location
    uint8_t  sensor_loc;
    /// Crank Length
    uint16_t crank_length;
    /// Chain Length
    uint16_t chain_length;
    /// Chain Weight
    uint16_t chain_weight;
    /// Span Length
    uint16_t span_length;
    /// Mask Content
    uint16_t mask_content;
};

/// CP Control Point Response Value
union ble_cps_ctrl_pt_rsp_val
{
    /// Supported sensor locations (up to 17)
    uint32_t                   supp_sensor_loc;
    /// Crank Length
    uint16_t                   crank_length;
    /// Chain Length
    uint16_t                   chain_length;
    /// Chain Weight
    uint16_t                   chain_weight;
    /// Span Length
    uint16_t                   span_length;
    /// Offset compensation
    int16_t                    offset_comp;
    ///Sampling rate
    uint8_t                    sampling_rate;
    /// Enhanced Offset Compensation
    ble_cps_enhanced_offset_comp_t enhanced_offset_comp;
    /// Calibration date
    prf_date_time_t            factory_calibration;
    // Not transmitted over the air (only on server side)
    /// New Cumulative Wheel revolution Value
    uint32_t                   cumul_wheel_rev;
    /// Mask Measurement content
    uint16_t                   mask_meas_content;
    /// New Sensor Location
    uint8_t                    sensor_loc;
};

#endif //(_CPS_COMMON_I_H_)