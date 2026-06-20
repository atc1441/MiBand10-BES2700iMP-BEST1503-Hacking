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
#ifndef __BLE_FTMP_H__
#define __BLE_FTMP_H__
#if defined(BLE_FTMC_ENABLED) || defined(BLE_FTMS_ENABLED)
#include "gatt_service.h"
#include "app_ble.h"
#include "prf_types.h"
#include "co_math.h"
#ifdef __cplusplus
extern "C" {
#endif

#define FTMS_CP_NTF_MAX_LEN        (20)

#define FTMS_ADV_MAX_LEN        (6)

/// Control Point Value Max Length
#define FTMS_CTRL_PT_RSP_MAX_LEN        (20)
/// Control Point Value Min Length
#define FTMS_CTRL_PT_RSP_MIN_LEN        (3)

/// threadmill data Value Max Length
#define FTMS_THREADMILL_DATA_MAX_LEN        (38)
/// cross trainer data Value Max Length
#define FTMS_CROSS_TRAINER_DATA_MAX_LEN     (42)
/// step climber data Value Max Length
#define FTMS_STEP_CLIMBER_DATA_MAX_LEN      (24)
/// stair climber data Value Max Length
#define FTMS_STAIR_CLIMBER_DATA_MAX_LEN     (24)
/// rower data Value Max Length
#define FTMS_ROWER_DATA_MAX_LEN             (34)
/// indoor bike data Value Max Length
#define FTMS_INDOOR_BIKE_DATA_MAX_LEN       (32)

#define FTM_CP_TIMEOUT                       (30000)

enum ftm_char_enum{
    FTM_CHAR_FEATURE,
    FTM_CHAR_TREADMILL_DATA,
    FTM_CHAR_CROSS_TRAINER_DATA,
    FTM_CHAR_STEP_CLIMBER_DATA_DATA,
    FTM_CHAR_STAIR_CLIMBER_DATA_DATA,
    FTM_CHAR_ROWER_DATA,
    FTM_CHAR_INDOOR_BIKE_DATA,
    FTM_CHAR_TRAINING_STATUS,
    FTM_CHAR_SUPP_SPEED_RANGE,
    FTM_CHAR_SUPP_INCLINATION_RANGE,
    FTM_CHAR_SUPP_RESISTANCE_LEVEL_RANGE,
    FTM_CHAR_SUPP_POWER_RANGE,
    FTM_CHAR_SUPP_HEART_RATE_RANGE,
    FTM_CHAR_CPT,
    FTM_CHAR_STATUS,
    FTM_CHAR_MAX_NUM,
};

/// Fitness Machine Features and Target Setting Features

/// Fitness Machine Feature bit field
enum ftms_feat_flags_bf
{
    /// Average Speed Supported
    FTMS_FEAT_AVERAGE_SPEED_SUP_POS            = 0,
    FTMS_FEAT_AVERAGE_SPEED_SUP_BIT            = CO_BIT(FTMS_FEAT_AVERAGE_SPEED_SUP_POS),

    /// Cadence Supported
    FTMS_FEAT_CADENCE_SUP_POS                  = 1,
    FTMS_FEAT_CADENCE_SUP_BIT                  = CO_BIT(FTMS_FEAT_CADENCE_SUP_POS),

    /// Total Distance Supported
    FTMS_FEAT_TOTAL_DISTANCE_SUP_POS           = 2,
    FTMS_FEAT_TOTAL_DISTANCE_SUP_BIT           = CO_BIT(FTMS_FEAT_TOTAL_DISTANCE_SUP_POS),

    /// Inclination Supported
    FTMS_FEAT_INCLINATION_SUP_POS              = 3,
    FTMS_FEAT_INCLINATION_SUP_BIT              = CO_BIT(FTMS_FEAT_INCLINATION_SUP_POS),

    /// Elevation Gain Supported
    FTMS_FEAT_ELEVATION_GAIN_SUP_POS           = 4,
    FTMS_FEAT_ELEVATION_GAIN_SUP_BIT           = CO_BIT(FTMS_FEAT_ELEVATION_GAIN_SUP_POS),

    /// Pace Supported
    FTMS_FEAT_PACE_SUP_POS                     = 5,
    FTMS_FEAT_PACE_SUP_BIT                     = CO_BIT(FTMS_FEAT_PACE_SUP_POS),

    /// Step Count Supported
    FTMS_FEAT_STEP_COUNT_SUP_POS               = 6,
    FTMS_FEAT_STEP_COUNT_SUP_BIT               = CO_BIT(FTMS_FEAT_STEP_COUNT_SUP_POS),

    /// Resistance Level Supported
    FTMS_FEAT_RESISTANCE_LEVEL_SUP_POS         = 7,
    FTMS_FEAT_RESISTANCE_LEVEL_SUP_BIT         = CO_BIT(FTMS_FEAT_RESISTANCE_LEVEL_SUP_POS),

    /// Stride Count Supported
    FTMS_FEAT_STRIDE_COUNT_SUP_POS             = 8,
    FTMS_FEAT_STRIDE_COUNT_SUP_BIT             = CO_BIT(FTMS_FEAT_STRIDE_COUNT_SUP_POS),

    /// Expended Energy Supported
    FTMS_FEAT_EXP_ENERGY_SUP_POS               = 9,
    FTMS_FEAT_EXP_ENERGY_SUP_BIT               = CO_BIT(FTMS_FEAT_EXP_ENERGY_SUP_POS),

    /// Heart Rate Measurement Supported
    FTMS_FEAT_HEART_RATE_SUP_POS               = 10,
    FTMS_FEAT_HEART_RATE_SUP_BIT               = CO_BIT(FTMS_FEAT_HEART_RATE_SUP_POS),

    /// Metabolic Equivalent  Supported
    FTMS_FEAT_METABOLIC_EQUIVALENT_SUP_POS     = 11,
    FTMS_FEAT_METABOLIC_EQUIVALENT_SUP_BIT     = CO_BIT(FTMS_FEAT_METABOLIC_EQUIVALENT_SUP_POS),

    /// Elapsed Time Supported
    FTMS_FEAT_ELAPSED_TIME_SUP_POS             = 12,
    FTMS_FEAT_ELAPSED_TIME_SUP_BIT             = CO_BIT(FTMS_FEAT_ELAPSED_TIME_SUP_POS),

    /// Remaining Time Supported
    FTMS_FEAT_REMAINING_TIME_SUP_POS           = 13,
    FTMS_FEAT_REMAINING_TIME_SUP_BIT           = CO_BIT(FTMS_FEAT_REMAINING_TIME_SUP_POS),

    /// Power Measurement Supported
    FTMS_FEAT_POWER_MEASUREMENT_SUP_POS        = 14,
    FTMS_FEAT_POWER_MEASUREMENT_SUP_BIT        = CO_BIT(FTMS_FEAT_POWER_MEASUREMENT_SUP_POS),

    /// Force on Belt and Power Output Supported
    FTMS_FEAT_BELT_POWER_OUT_SUP_POS           = 15,
    FTMS_FEAT_BELT_POWER_OUT_SUP_BIT           = CO_BIT(FTMS_FEAT_BELT_POWER_OUT_SUP_POS),

    /// User Data Retention Supported
    FTMS_FEAT_USER_DATA_RETENTION_SUP_POS      = 16,
    FTMS_FEAT_USER_DATA_RETENTION_SUP_BIT      = CO_BIT(FTMS_FEAT_USER_DATA_RETENTION_SUP_POS),
    // bit 17 - 31 RFU
};

/// Target Setting Feature bit field
enum ftms_target_setting_feat_flags_bf
{
    /// Speed Target Setting Supported
    FTMS_TARGET_FEAT_SPEED_SUP_POS                      = 0,
    FTMS_TARGET_FEAT_SPEED_SUP_BIT                      = CO_BIT(FTMS_TARGET_FEAT_SPEED_SUP_POS),

    /// Inclination Target Setting Supported
    FTMS_TARGET_FEAT_INCLINATION_SUP_POS                = 1,
    FTMS_TARGET_FEAT_INCLINATION_SUP_BIT                = CO_BIT(FTMS_TARGET_FEAT_INCLINATION_SUP_POS),
    
    /// Resistance Target Setting Supported
    FTMS_TARGET_FEAT_RESISTANCE_SUP_POS                 = 2,
    FTMS_TARGET_FEAT_RESISTANCE_SUP_BIT                 = CO_BIT(FTMS_TARGET_FEAT_RESISTANCE_SUP_POS),

    /// Power Target Setting Supported
    FTMS_TARGET_FEAT_POWER_SUP_POS                      = 3,
    FTMS_TARGET_FEAT_POWER_SUP_BIT                      = CO_BIT(FTMS_TARGET_FEAT_POWER_SUP_POS),

    /// Heart Rate Target Setting Supported
    FTMS_TARGET_FEAT_HEART_RATE_SUP_POS                 = 4,
    FTMS_TARGET_FEAT_HEART_RATE_SUP_BIT                 = CO_BIT(FTMS_TARGET_FEAT_HEART_RATE_SUP_POS),

    /// Target Expended Energy Configuration Supported
    FTMS_TARGET_FEAT_EXP_ENERGY_CFG_SUP_POS             = 5,
    FTMS_TARGET_FEAT_EXP_ENERGY_CFG_SUP_BIT             = CO_BIT(FTMS_TARGET_FEAT_EXP_ENERGY_CFG_SUP_POS),

    /// Target Step Number Configuration Supported
    FTMS_TARGET_FEAT_STEP_NUM_CFG_SUP_POS               = 6,
    FTMS_TARGET_FEAT_STEP_NUM_CFG_SUP_BIT               = CO_BIT(FTMS_TARGET_FEAT_STEP_NUM_CFG_SUP_POS),

    /// Target Stride Number Configuration Supported
    FTMS_TARGET_FEAT_STRIDE_NUM_CFG_SUP_POS             = 7,
    FTMS_TARGET_FEAT_STRIDE_NUM_CFG_SUP_BIT             = CO_BIT(FTMS_TARGET_FEAT_STRIDE_NUM_CFG_SUP_POS),

    /// Target Distance Configuration Supported
    FTMS_TARGET_FEAT_DISTANCE_CFG_SUP_POS               = 8,
    FTMS_TARGET_FEAT_DISTANCE_CFG_SUP_BIT               = CO_BIT(FTMS_TARGET_FEAT_DISTANCE_CFG_SUP_POS),

    /// Target Training Time Configuration Supported
    FTMS_TARGET_FEAT_TRAINING_TIME_CFG_SUP_POS          = 9,
    FTMS_TARGET_FEAT_TRAINING_TIME_CFG_SUP_BIT          = CO_BIT(FTMS_TARGET_FEAT_TRAINING_TIME_CFG_SUP_POS),

    /// Target Time in Two Heart Rate Zones Configuration Supported
    FTMS_TARGET_FEAT_TWO_HEART_RATE_CFG_SUP_POS         = 10,
    FTMS_TARGET_FEAT_TWO_HEART_RATE_CFG_SUP_BIT         = CO_BIT(FTMS_TARGET_FEAT_TWO_HEART_RATE_CFG_SUP_POS),

    /// Target Time in Three Heart Rate Zones Configuration Supported
    FTMS_TARGET_FEAT_THREE_HEART_RATE_CFG_SUP_POS       = 11,
    FTMS_TARGET_FEAT_THREE_HEART_RATE_CFG_SUP_BIT       = CO_BIT(FTMS_TARGET_FEAT_THREE_HEART_RATE_CFG_SUP_POS),

    /// Target Time in Five Heart Rate Zones Configuration Supported
    FTMS_TARGET_FEAT_FIVE_HEART_RATE_CFG_SUP_POS        = 12,
    FTMS_TARGET_FEAT_FIVE_HEART_RATE_CFG_SUP_BIT        = CO_BIT(FTMS_TARGET_FEAT_FIVE_HEART_RATE_CFG_SUP_POS),

    /// Indoor Bike Simulation Parameters Supported
    FTMS_TARGET_FEAT_INDOOR_BIKE_SIMULATION_SUP_POS     = 13,
    FTMS_TARGET_FEAT_INDOOR_BIKE_SIMULATION_SUP_BIT     = CO_BIT(FTMS_TARGET_FEAT_INDOOR_BIKE_SIMULATION_SUP_POS),

    /// Wheel Circumference Configuration Supported
    FTMS_TARGET_FEAT_WHEEL_CIRC_CFG_SUP_POS             = 14,
    FTMS_TARGET_FEAT_WHEEL_CIRC_CFG_SUP_BIT             = CO_BIT(FTMS_TARGET_FEAT_WHEEL_CIRC_CFG_SUP_POS),

    /// Spin Down Control Supported
    FTMS_TARGET_FEAT_SPIN_DOWN_CT_SUP_POS               = 15,
    FTMS_TARGET_FEAT_SPIN_DOWN_CT_SUP_BIT               = CO_BIT(FTMS_TARGET_FEAT_SPIN_DOWN_CT_SUP_POS),

    /// Target Cadence Configuration Supported
    FTMS_TARGET_FEAT_CADENCE_CFG_SUP_POS                = 16,
    FTMS_TARGET_FEAT_CADENCE_CFG_SUP_BIT                = CO_BIT(FTMS_TARGET_FEAT_CADENCE_CFG_SUP_POS),
    // bit 17 - 31 RFU
};

/// Treadmill Data Flags bit field
enum treadmill_data_flags_bf
{
    /// Instantaneous Speed Present
    FTMS_TD_MORE_DATA_PRESENT_POS                = 0,
    FTMS_TD_MORE_DATA_PRESENT_BIT                = CO_BIT(FTMS_TD_MORE_DATA_PRESENT_POS),

    /// Average Speed Reference
    FTMS_TD_AVERAGE_SPEED_REFERENCE_POS          = 1,
    FTMS_TD_AVERAGE_SPEED_REFERENCE_BIT          = CO_BIT(FTMS_TD_AVERAGE_SPEED_REFERENCE_POS),

    /// Total Distance Present
    FTMS_TD_TOTAL_DISTANCE_PRESENT_POS           = 2,
    FTMS_TD_TOTAL_DISTANCE_PRESENT_BIT           = CO_BIT(FTMS_TD_TOTAL_DISTANCE_PRESENT_POS),

    /// Inclination and Ramp Angle Setting Present
    FTMS_TD_INCLINATION_RAMP_ANGLE_PRESENT_POS   = 3,
    FTMS_TD_INCLINATION_RAMP_ANGLE_PRESENT_BIT   = CO_BIT(FTMS_TD_INCLINATION_RAMP_ANGLE_PRESENT_POS),

    /// Elevation Gain Present
    FTMS_TD_ELEVATION_GAIN_PRESENT_POS           = 4,
    FTMS_TD_ELEVATION_GAIN_PRESENT_BIT           = CO_BIT(FTMS_TD_ELEVATION_GAIN_PRESENT_POS),

    /// Instantaneous Pace Present
    FTMS_TD_INSTANTANEOUS_PACE_PRESENT_POS       = 5,
    FTMS_TD_INSTANTANEOUS_PACE_PRESENT_BIT       = CO_BIT(FTMS_TD_INSTANTANEOUS_PACE_PRESENT_POS),

    /// Average Pace Present
    FTMS_TD_AVERAGE_PACE_PRESENT_POS             = 6,
    FTMS_TD_AVERAGE_PACE_PRESENT_BIT             = CO_BIT(FTMS_TD_AVERAGE_PACE_PRESENT_POS),

    /// Expended Energy Present
    FTMS_TD_EXPENDED_ENERGY_PRESENT_POS          = 7,
    FTMS_TD_EXPENDED_ENERGY_PRESENT_BIT          = CO_BIT(FTMS_TD_EXPENDED_ENERGY_PRESENT_POS),

    /// Heart Rate Present
    FTMS_TD_HEART_RATE_PRESENT_POS               = 8,
    FTMS_TD_HEART_RATE_PRESENT_BIT               = CO_BIT(FTMS_TD_HEART_RATE_PRESENT_POS),

    /// Metabolic Equivalent Present
    FTMS_TD_METABOLIC_EQUIVALENT_PRESENT_POS     = 9,
    FTMS_TD_METABOLIC_EQUIVALENT_PRESENT_BIT     = CO_BIT(FTMS_TD_METABOLIC_EQUIVALENT_PRESENT_POS),

    /// Elapsed Time Present
    FTMS_TD_ELAPSED_TIME_PRESENT_POS             = 10,
    FTMS_TD_ELAPSED_TIME_PRESENT_BIT             = CO_BIT(FTMS_TD_ELAPSED_TIME_PRESENT_POS),

    /// Remaining Time Present
    FTMS_TD_REMAINING_TIME_PRESENT_POS           = 11,
    FTMS_TD_REMAINING_TIME_PRESENT_BIT           = CO_BIT(FTMS_TD_REMAINING_TIME_PRESENT_POS),

    /// Force on Belt and Power Output Present
    FTMS_TD_BELT_POWER_OUT_PRESENT_POS           = 12,
    FTMS_TD_BELT_POWER_OUT_PRESENT_BIT           = CO_BIT(FTMS_TD_BELT_POWER_OUT_PRESENT_POS),

    // bit 13 - 15 RFU
};

/// Cross Trainer Data Flags bit field
enum cross_trainer_data_flags_bf
{
    /// Instantaneous Speed Present
    FTMS_CTD_MORE_DATA_PRESENT_POS               = 0,
    FTMS_CTD_MORE_DATA_PRESENT_BIT               = CO_BIT(FTMS_CTD_MORE_DATA_PRESENT_POS),

    /// Average Speed Reference
    FTMS_CTD_AVERAGE_SPEED_REFERENCE_POS         = 1,
    FTMS_CTD_AVERAGE_SPEED_REFERENCE_BIT         = CO_BIT(FTMS_CTD_AVERAGE_SPEED_REFERENCE_POS),

    /// Total Distance Present
    FTMS_CTD_TOTAL_DISTANCE_PRESENT_POS          = 2,
    FTMS_CTD_TOTAL_DISTANCE_PRESENT_BIT          = CO_BIT(FTMS_CTD_TOTAL_DISTANCE_PRESENT_POS),

    /// Step Count Present
    FTMS_CTD_STEP_COUNT_PRESENT_POS              = 3,
    FTMS_CTD_STEP_COUNT_PRESENT_BIT              = CO_BIT(FTMS_CTD_STEP_COUNT_PRESENT_POS),

    /// Stride Count Present
    FTMS_CTD_STRIDE_COUNT_PRESENT_POS            = 4,
    FTMS_CTD_STRIDE_COUNT_PRESENT_BIT            = CO_BIT(FTMS_CTD_STRIDE_COUNT_PRESENT_POS),

    /// Elevation Gain Present
    FTMS_CTD_ELEVATION_GAIN_PRESENT_POS          = 5,
    FTMS_CTD_ELEVATION_GAIN_PRESENT_BIT          = CO_BIT(FTMS_CTD_ELEVATION_GAIN_PRESENT_POS),

    /// Inclination and Ramp Angle Setting Present
    FTMS_CTD_INCLINATION_RAMP_ANGLE_PRESENT_POS  = 6,
    FTMS_CTD_INCLINATION_RAMP_ANGLE_PRESENT_BIT  = CO_BIT(FTMS_CTD_INCLINATION_RAMP_ANGLE_PRESENT_POS),

    /// Resistance Level Present
    FTMS_CTD_RESISTANCE_LEVEL_PRESENT_POS        = 7,
    FTMS_CTD_RESISTANCE_LEVEL_PRESENT_BIT        = CO_BIT(FTMS_CTD_RESISTANCE_LEVEL_PRESENT_POS),

    /// Instantaneous Power Present
    FTMS_CTD_INSTANTANEOUS_POWER_PRESENT_POS     = 8,
    FTMS_CTD_INSTANTANEOUS_POWER_PRESENT_BIT     = CO_BIT(FTMS_CTD_INSTANTANEOUS_POWER_PRESENT_POS),
    
    /// Average Power Present
    FTMS_CTD_AVERAGE_POWER_PRESENT_POS           = 9,
    FTMS_CTD_AVERAGE_POWER_PRESENT_BIT           = CO_BIT(FTMS_CTD_AVERAGE_POWER_PRESENT_POS),

    /// Expended Energy Present
    FTMS_CTD_EXPENDED_ENERGY_PRESENT_POS         = 10,
    FTMS_CTD_EXPENDED_ENERGY_PRESENT_BIT         = CO_BIT(FTMS_CTD_EXPENDED_ENERGY_PRESENT_POS),

    /// Heart Rate Present
    FTMS_CTD_HEART_RATE_PRESENT_POS              = 11,
    FTMS_CTD_HEART_RATE_PRESENT_BIT              = CO_BIT(FTMS_CTD_HEART_RATE_PRESENT_POS),

    /// Metabolic Equivalent Present
    FTMS_CTD_METABOLIC_EQUIVALENT_PRESENT_POS    = 12,
    FTMS_CTD_METABOLIC_EQUIVALENT_PRESENT_BIT    = CO_BIT(FTMS_CTD_METABOLIC_EQUIVALENT_PRESENT_POS),
    
    /// Elapsed Time Present
    FTMS_CTD_ELAPSED_TIME_PRESENT_POS            = 13,
    FTMS_CTD_ELAPSED_TIME_PRESENT_BIT            = CO_BIT(FTMS_CTD_ELAPSED_TIME_PRESENT_POS),

    /// Remaining Time Present
    FTMS_CTD_REMAINING_TIME_PRESENT_POS          = 14,
    FTMS_CTD_REMAINING_TIME_PRESENT_BIT          = CO_BIT(FTMS_CTD_REMAINING_TIME_PRESENT_POS),

    /// Movement Direction
    FTMS_CTD_MOVEMEMT_DIRECTION_POS              = 15,
    FTMS_CTD_MOVEMEMT_DIRECTION_BIT              = CO_BIT(FTMS_CTD_MOVEMEMT_DIRECTION_POS),
};

/// Step Climber Data Flags bit field
enum step_climber_data_flags_bf
{
    /// Floors and Step Count fields Present
    FTMS_STD_MORE_DATA_PRESENT_POS               = 0,
    FTMS_STD_MORE_DATA_PRESENT_BIT               = CO_BIT(FTMS_STD_MORE_DATA_PRESENT_POS),

    /// Step per Minute Present
    FTMS_STD_STEP_PER_MINUTE_PRESENT_POS         = 1,
    FTMS_STD_STEP_PER_MINUTE_PRESENT_BIT         = CO_BIT(FTMS_STD_STEP_PER_MINUTE_PRESENT_POS),

    /// Average Step Rate Present
    FTMS_STD_AVERAGE_STEP_RATE_PRESENT_POS       = 2,
    FTMS_STD_AVERAGE_STEP_RATE_PRESENT_BIT       = CO_BIT(FTMS_STD_AVERAGE_STEP_RATE_PRESENT_POS),

    /// Positive Elevation Gain Present
    FTMS_STD_POSITIVE_ELEVATION_GAIN_PRESENT_POS = 3,
    FTMS_STD_POSITIVE_ELEVATION_GAIN_PRESENT_BIT = CO_BIT(FTMS_STD_POSITIVE_ELEVATION_GAIN_PRESENT_POS),

    /// Expended Energy Present
    FTMS_STD_EXPENDED_ENERGY_PRESENT_POS         = 4,
    FTMS_STD_EXPENDED_ENERGY_PRESENT_BIT         = CO_BIT(FTMS_STD_EXPENDED_ENERGY_PRESENT_POS),

    /// Heart Rate Present
    FTMS_STD_HEART_RATE_PRESENT_POS              = 5,
    FTMS_STD_HEART_RATE_PRESENT_BIT              = CO_BIT(FTMS_STD_HEART_RATE_PRESENT_POS),

    /// Metabolic Equivalent Present
    FTMS_STD_METABOLIC_EQUIVALENT_PRESENT_POS    = 6,
    FTMS_STD_METABOLIC_EQUIVALENT_PRESENT_BIT    = CO_BIT(FTMS_STD_METABOLIC_EQUIVALENT_PRESENT_POS),
    
    /// Elapsed Time Present
    FTMS_STD_ELAPSED_TIME_PRESENT_POS            = 7,
    FTMS_STD_ELAPSED_TIME_PRESENT_BIT            = CO_BIT(FTMS_STD_ELAPSED_TIME_PRESENT_POS),

    /// Remaining Time Present
    FTMS_STD_REMAINING_TIME_PRESENT_POS          = 8,
    FTMS_STD_REMAINING_TIME_PRESENT_BIT          = CO_BIT(FTMS_STD_REMAINING_TIME_PRESENT_POS),

    // bit 9 - 15 RFU
};

/// Stair Climber Flags bit field
enum stair_climber_data_flags_bf
{
    /// Floors fields Present
    FTMS_SCD_MORE_DATA_PRESENT_POS               = 0,
    FTMS_SCD_MORE_DATA_PRESENT_BIT               = CO_BIT(FTMS_SCD_MORE_DATA_PRESENT_POS),

    /// Step per Minute Present
    FTMS_SCD_STEP_PER_MINUTE_PRESENT_POS         = 1,
    FTMS_SCD_STEP_PER_MINUTE_PRESENT_BIT         = CO_BIT(FTMS_SCD_STEP_PER_MINUTE_PRESENT_POS),

    /// Average Step Rate Present
    FTMS_SCD_AVERAGE_STEP_RATE_PRESENT_POS       = 2,
    FTMS_SCD_AVERAGE_STEP_RATE_PRESENT_BIT       = CO_BIT(FTMS_SCD_AVERAGE_STEP_RATE_PRESENT_POS),

    /// Positive Elevation Gain Present
    FTMS_SCD_POSITIVE_ELEVATION_GAIN_PRESENT_POS = 3,
    FTMS_SCD_POSITIVE_ELEVATION_GAIN_PRESENT_BIT = CO_BIT(FTMS_SCD_POSITIVE_ELEVATION_GAIN_PRESENT_POS),

    /// Stride Count Present
    FTMS_SCD_STRIDE_COUNT_PRESENT_POS            = 4,
    FTMS_SCD_STRIDE_COUNT_PRESENT_BIT            = CO_BIT(FTMS_SCD_STRIDE_COUNT_PRESENT_POS),

    /// Expended Energy Present
    FTMS_SCD_EXPENDED_ENERGY_PRESENT_POS         = 5,
    FTMS_SCD_EXPENDED_ENERGY_PRESENT_BIT         = CO_BIT(FTMS_SCD_EXPENDED_ENERGY_PRESENT_POS),

    /// Heart Rate Present
    FTMS_SCD_HEART_RATE_PRESENT_POS              = 6,
    FTMS_SCD_HEART_RATE_PRESENT_BIT              = CO_BIT(FTMS_SCD_HEART_RATE_PRESENT_POS),

    /// Metabolic Equivalent Present
    FTMS_SCD_METABOLIC_EQUIVALENT_PRESENT_POS    = 7,
    FTMS_SCD_METABOLIC_EQUIVALENT_PRESENT_BIT    = CO_BIT(FTMS_SCD_METABOLIC_EQUIVALENT_PRESENT_POS),
    
    /// Elapsed Time Present
    FTMS_SCD_ELAPSED_TIME_PRESENT_POS            = 8,
    FTMS_SCD_ELAPSED_TIME_PRESENT_BIT            = CO_BIT(FTMS_SCD_ELAPSED_TIME_PRESENT_POS),

    /// Remaining Time Present
    FTMS_SCD_REMAINING_TIME_PRESENT_POS          = 9,
    FTMS_SCD_REMAINING_TIME_PRESENT_BIT          = CO_BIT(FTMS_SCD_REMAINING_TIME_PRESENT_POS),

    // bit 10 - 15 RFU
};

/// Rower Data Flags bit field
enum rower_data_flags_bf
{
    /// Stroke Rate and Stroke Count field Present
    FTMS_RD_MORE_DATA_PRESENT_POS               = 0,
    FTMS_RD_MORE_DATA_PRESENT_BIT               = CO_BIT(FTMS_RD_MORE_DATA_PRESENT_POS),

    /// Average Stroke Rate Present
    FTMS_RD_AVERAGE_STROKE_RATE_PRESENT_POS     = 1,
    FTMS_RD_AVERAGE_STROKE_RATE_PRESENT_BIT     = CO_BIT(FTMS_RD_AVERAGE_STROKE_RATE_PRESENT_POS),

    /// Total Distance Present
    FTMS_RD_TOTAL_DISTANCE_PRESENT_POS          = 2,
    FTMS_RD_TOTAL_DISTANCE_PRESENT_BIT         = CO_BIT(FTMS_RD_TOTAL_DISTANCE_PRESENT_POS),

    /// Instantaneous Pace Present
    FTMS_RD_INSTANTANEOUS_PACE_PRESENT_POS      = 3,
    FTMS_RD_INSTANTANEOUS_PACE_PRESENT_BIT      = CO_BIT(FTMS_RD_INSTANTANEOUS_PACE_PRESENT_POS),

    /// Average Pace Present
    FTMS_RD_AVERAGE_PACE_PRESENT_POS            = 4,
    FTMS_RD_AVERAGE_PACE_PRESENT_BIT            = CO_BIT(FTMS_RD_AVERAGE_PACE_PRESENT_POS),

    /// Instantaneous Power Present
    FTMS_RD_INSTANTANEOUS_POWER_PRESENT_POS     = 5,
    FTMS_RD_INSTANTANEOUS_POWER_PRESENT_BIT     = CO_BIT(FTMS_RD_INSTANTANEOUS_POWER_PRESENT_POS),

    /// Average Power Present
    FTMS_RD_AVERAGE_POWER_PRESENT_POS           = 6,
    FTMS_RD_AVERAGE_POWER_PRESENT_BIT           = CO_BIT(FTMS_RD_AVERAGE_POWER_PRESENT_POS),

    /// Resistance Level Present
    FTMS_RD_RESISTANCE_LEVEL_PRESENT_POS        = 7,
    FTMS_RD_RESISTANCE_LEVEL_PRESENT_BIT        = CO_BIT(FTMS_RD_RESISTANCE_LEVEL_PRESENT_POS),

    /// Expended Energy Present
    FTMS_RD_EXPENDED_ENERGY_PRESENT_POS         = 8,
    FTMS_RD_EXPENDED_ENERGY_PRESENT_BIT         = CO_BIT(FTMS_RD_EXPENDED_ENERGY_PRESENT_POS),

    /// Heart Rate Present
    FTMS_RD_HEART_RATE_PRESENT_POS              = 9,
    FTMS_RD_HEART_RATE_PRESENT_BIT              = CO_BIT(FTMS_RD_HEART_RATE_PRESENT_POS),

    /// Metabolic Equivalent Present
    FTMS_RD_METABOLIC_EQUIVALENT_PRESENT_POS    = 10,
    FTMS_RD_METABOLIC_EQUIVALENT_PRESENT_BIT    = CO_BIT(FTMS_RD_METABOLIC_EQUIVALENT_PRESENT_POS),
    
    /// Elapsed Time Present
    FTMS_RD_ELAPSED_TIME_PRESENT_POS            = 11,
    FTMS_RD_ELAPSED_TIME_PRESENT_BIT            = CO_BIT(FTMS_RD_ELAPSED_TIME_PRESENT_POS),

    /// Remaining Time Present
    FTMS_RD_REMAINING_TIME_PRESENT_POS          = 12,
    FTMS_RD_REMAINING_TIME_PRESENT_BIT          = CO_BIT(FTMS_RD_REMAINING_TIME_PRESENT_POS),

    // bit 13 - 15 RFU
};

/// Indoor Bike Data Flags bit field
enum indoor_bike_data_flags_bf
{
    /// Instantaneous Speed field Present
    FTMS_IBD_MORE_DATA_PRESENT_POS               = 0,
    FTMS_IBD_MORE_DATA_PRESENT_BIT               = CO_BIT(FTMS_IBD_MORE_DATA_PRESENT_POS),

    //// Average Speed Reference
    FTMS_IBD_AVERAGE_SPEED_REFERENCE_POS          = 1,
    FTMS_IBD_AVERAGE_SPEED_REFERENCE_BIT          = CO_BIT(FTMS_IBD_AVERAGE_SPEED_REFERENCE_POS),

    /// Instantaneous Cadence
    FTMS_IBD_INSTANTNEOUS_CADENCE_PRESENT_POS          = 2,
    FTMS_IBD_INSTANTNEOUS_CADENCE_PRESENT_BIT         = CO_BIT(FTMS_IBD_INSTANTNEOUS_CADENCE_PRESENT_POS),

    /// Average Cadence Present
    FTMS_IBD_AVERAGE_CADENCE_PRESENT_POS      = 3,
    FTMS_IBD_AVERAGE_CADENCE_PRESENT_BIT      = CO_BIT(FTMS_IBD_AVERAGE_CADENCE_PRESENT_POS),

    /// Total Distance Present
    FTMS_IBD_TOTAL_DISTANCE_PRESENT_POS           = 4,
    FTMS_IBD_TOTAL_DISTANCE_PRESENT_BIT          = CO_BIT(FTMS_IBD_TOTAL_DISTANCE_PRESENT_POS),

    /// Resistance Level Present
    FTMS_IBD_RESISTANCE_LEVEL_PRESENT_POS        = 5,
    FTMS_IBD_RESISTANCE_LEVEL_PRESENT_BIT        = CO_BIT(FTMS_IBD_RESISTANCE_LEVEL_PRESENT_POS),

    /// Instantaneous Power Present
    FTMS_IBD_INSTANTANEOUS_POWER_PRESENT_POS     = 6,
    FTMS_IBD_INSTANTANEOUS_POWER_PRESENT_BIT     = CO_BIT(FTMS_IBD_INSTANTANEOUS_POWER_PRESENT_POS),

    /// Average Power Present
    FTMS_IBD_AVERAGE_POWER_PRESENT_POS           = 7,
    FTMS_IBD_AVERAGE_POWER_PRESENT_BIT           = CO_BIT(FTMS_IBD_AVERAGE_POWER_PRESENT_POS),

    /// Expended Energy Present
    FTMS_IBD_EXPENDED_ENERGY_PRESENT_POS         = 8,
    FTMS_IBD_EXPENDED_ENERGY_PRESENT_BIT         = CO_BIT(FTMS_IBD_EXPENDED_ENERGY_PRESENT_POS),

    /// Heart Rate Present
    FTMS_IBD_HEART_RATE_PRESENT_POS              = 9,
    FTMS_IBD_HEART_RATE_PRESENT_BIT              = CO_BIT(FTMS_IBD_HEART_RATE_PRESENT_POS),

    /// Metabolic Equivalent Present
    FTMS_IBD_METABOLIC_EQUIVALENT_PRESENT_POS    = 10,
    FTMS_IBD_METABOLIC_EQUIVALENT_PRESENT_BIT    = CO_BIT(FTMS_IBD_METABOLIC_EQUIVALENT_PRESENT_POS),
    
    /// Elapsed Time Present
    FTMS_IBD_ELAPSED_TIME_PRESENT_POS            = 11,
    FTMS_IBD_ELAPSED_TIME_PRESENT_BIT            = CO_BIT(FTMS_IBD_ELAPSED_TIME_PRESENT_POS),

    /// Remaining Time Present
    FTMS_IBD_REMAINING_TIME_PRESENT_POS          = 12,
    FTMS_IBD_REMAINING_TIME_PRESENT_BIT          = CO_BIT(FTMS_IBD_REMAINING_TIME_PRESENT_POS),

    // bit 13 - 15 RFU
};

/// Profile Configuration Additional Flags ()
enum ftms_prf_cfg_flag_bf
{
    /// treadmill_data - Client Char. Cfg
    FTMS_PRF_CFG_FLAG_TREADMILL_DATA_NTF_POS        = 0,
    FTMS_PRF_CFG_FLAG_TREADMILL_DATA_NTF_BIT        = CO_BIT(FTMS_PRF_CFG_FLAG_TREADMILL_DATA_NTF_POS),

    /// cross_trainer_data - Client Char. Cfg
    FTMS_PRF_CFG_FLAG_CROSS_TRAINER_DATA_NTF_POS    = 1,
    FTMS_PRF_CFG_FLAG_CROSS_TRAINER_DATA_NTF_BIT    = CO_BIT(FTMS_PRF_CFG_FLAG_CROSS_TRAINER_DATA_NTF_POS),

    /// step_climber_data - Client Char. Cfg
    FTMS_PRF_CFG_FLAG_STEP_CLIMBER_DATA_NTF_POS     = 2,
    FTMS_PRF_CFG_FLAG_STEP_CLIMBER_DATA_NTF_BIT     = CO_BIT(FTMS_PRF_CFG_FLAG_STEP_CLIMBER_DATA_NTF_POS),

    /// stair_climber_data - Client Char. Cfg
    FTMS_PRF_CFG_FLAG_STAIR_CLIMBER_DATA_NTF_POS    = 3,
    FTMS_PRF_CFG_FLAG_STAIR_CLIMBER_DATA_NTF_BIT    = CO_BIT(FTMS_PRF_CFG_FLAG_STAIR_CLIMBER_DATA_NTF_POS),

    /// rower_data - Client Char. Cfg
    FTMS_PRF_CFG_FLAG_ROWER_DATA_NTF_POS            = 4,
    FTMS_PRF_CFG_FLAG_ROWER_DATA_NTF_BIT            = CO_BIT(FTMS_PRF_CFG_FLAG_ROWER_DATA_NTF_POS),

    /// indoor_bike_data - Client Char. Cfg
    FTMS_PRF_CFG_FLAG_INDOOR_BIKE_DATA_NTF_POS      = 5,
    FTMS_PRF_CFG_FLAG_INDOOR_BIKE_DATA_NTF_BIT      = CO_BIT(FTMS_PRF_CFG_FLAG_INDOOR_BIKE_DATA_NTF_POS),

    /// training_status - Client Char. Cfg
    FTMS_PRF_CFG_FLAG_TRAINING_STATUS_NTF_POS       = 6,
    FTMS_PRF_CFG_FLAG_TRAINING_STATUS_NTF_BIT       = CO_BIT(FTMS_PRF_CFG_FLAG_TRAINING_STATUS_NTF_POS),

    /// control_point - Client Char. Cfg
    FTMS_PRF_CFG_FLAG_CTRL_POINT_IND_POS            = 7,
    FTMS_PRF_CFG_FLAG_CTRL_POINT_IND_BIT            = CO_BIT(FTMS_PRF_CFG_FLAG_CTRL_POINT_IND_POS),

    /// ftm_status - Client Char. Cfg
    FTMS_PRF_CFG_FLAG_STATUS_NTF_POS                = 8,
    FTMS_PRF_CFG_FLAG_STATUS_NTF_BIT                = CO_BIT(FTMS_PRF_CFG_FLAG_STATUS_NTF_POS),
};

struct ble_ftms_db_cfg
{
    /**
     * Fitness Machine Feature Value - Not supposed to be modified during the lifetime of the device
     * This bit field is set in order to decide which features are supported:
     *   Supported features (specification) ---------------- Bits 0 to 16
     */
    uint32_t ftms_feature;
    /**
     * Target Setting Features Value - Not supposed to be modified during the lifetime of the device
     * This bit field is set in order to decide which features are supported:
     *   Supported features (specification) ---------------- Bits 0 to 16
     */
    uint32_t target_setting_feature;
    /**
     * Profile characteristic configuration:
     *   Enable Treadmill Data Characteristic ----- Bit 0
     *   Enable Cross Trainer Data Characteristic - Bit 1
     *   Enable Step Climber Data Characteristic -- Bit 2
     *   Enable Stair Climber Data Characteristic - Bit 3
     *   Enable Rower Data Characteristic --------- Bit 4
     *   Enable Indoor Bike Data Characteristic --- Bit 5
     *   Enable Training Status Characteristic ---- Bit 6
     *   Enable Control Point Characteristic (*) -- Bit 7
     *
     **/
    uint8_t prfl_config;
};

/// FTM Control Point: Opcode
enum ftmc_opcode_id {
    CTRL_PT_REQUEST     = 0,
    CTRL_PT_RESET,
    CTRL_PT_SET_TARGET_SPEED,
    CTRL_PT_SET_TARGET_INCLINATION,
    CTRL_PT_SET_TARGET_RESISLEVEL,
    CTRL_PT_SET_TARGET_POWER,
    CTRL_PT_SET_TARGET_HEARTRATE,
    CTRL_PT_SET_TARGET_STARTRESU,
    CTRL_PT_SET_TARGET_STOPPAUSE,
    CTRL_PT_SET_TARGET_EXPENERGY,
    CTRL_PT_SET_TARGET_NUMSTEPS,
    CTRL_PT_SET_TARGET_NUMSTRIDS,
    CTRL_PT_SET_TARGET_DISTANCE,
    CTRL_PT_SET_TARGET_TRAINTIME,
    CTRL_PT_SET_TARGET_TIMEINTWOHR,
    CTRL_PT_SET_TARGET_TIMEINTHREEHR,
    CTRL_PT_SET_TARGET_TIMEINFIVEHR,
    CTRL_PT_SET_INDOOR_BIKESIMUL,
    CTRL_PT_SET_WHEEL_CIRCUMFER,
    CTRL_PT_SET_SPINDOWN_CONTRL,
    CTRL_PT_SET_TARGETED_CADENCE,
    CTRL_PT_RESERVED,
    CTRL_PT_RSP_CODE = 128,
};

/// FTM Control Point rsp result code
enum ftmc_result_code_id {
    FTM_RESULT_SUCCESS = 1,
    FTM_RESULT_OPCODE_NOT_SUPPORTED,
    FTM_RESULT_INVALID_PARAM,
    FTM_RESULT_OPERATION_FAILED,
    FTM_RESULT_CONTROL_NOT_PERMITTED,
};

/// FTM status
enum ftmc_status {
    FTM_STATUS_RESET = 1,
    FTM_STATUS_STOP_PAUSE_BY_USER,
    FTM_STATUS_STOP_BY_SAFETY_KEY,
    FTM_STATUS_START_RESU_BY_USER,
    FTM_STATUS_TARGET_SPEED_CHANGED,
    FTM_STATUS_TARGET_INCLINE_CHANGED,
    FTM_STATUS_TARGET_RESIS_LEVEL_CHANGED,
    FTM_STATUS_TARGET_POWER_CHANGED,
    FTM_STATUS_TARGET_HEAR_RATE_CHANGED,
    FTM_STATUS_TARGET_EXPEN_ENERGY_CHANGED,
    FTM_STATUS_TARGET_NUM_STEPS_CHANGED,
    FTM_STATUS_TARGET_DISTANCE_CHNAGED,
    FTM_STATUS_TARGET_TRAIN_TIME_CHANGED,
    FTM_STATUS_TARGET_TIME_TWO_HR_CHANGED,
    FTM_STATUS_TARGET_TIME_THREE_HR_CHANGED,
    FTM_STATUS_TARGET_TIME_FIVE_HR_CHANGED,
    FTM_STATUS_INDOOR_BIKE_SIMUL_PARAM_CHANGED,
    FTM_STATUS_WHEEL_CIRCUM_CHANGED,
    FTM_STATUS_SPIN_DOWN_STATUS,
    FTM_STATUS_TARGET_CADENCE_CHANGED,
    FTM_STATUS_CONTROL_PERMISSION_LOST = 255,
};

//Spin Down Status
enum ftmc_spin_down_status {
    //0 Reserved for Future Use
    FTM_SPIN_DOWN_STATUS_RFU = 0,
    FTM_SPIN_DOWN_STATUS_REQ,
    FTM_SPIN_DOWN_STATUS_SUCCESS,
    FTM_SPIN_DOWN_STATUS_ERROR,
    FTM_SPIN_DOWN_STATUS_STOP_PEDALING,
    //0x05-0xFF Reserved for Future Use
};

/// Fitness Machine Service Characteristic Descriptors
enum ftmc_ftms_descs
{
    /// TREADMILL_DATA Char. - Client Characteristic Configuration
    BLE_FTMC_DESC_TREADMILL_DATA_CL_CFG,
    /// CROSS_TRAINER Char. - Client Characteristic Configuration
    BLE_FTMC_DESC_CROSS_TRAINER_CL_CFG,
    /// STEP_CLIMBER Char. - Client Characteristic Configuration
    BLE_FTMC_DESC_STEP_CLIMBER_CL_CFG,
    /// STAIR_CLIMBER Char. - Client Characteristic Configuration
    BLE_FTMC_DESC_STAIR_CLIMBER_CL_CFG,
    /// ROWER Char. - Client Characteristic Configuration
    BLE_FTMC_DESC_ROWER_CL_CFG,
    /// INDOOR_BIKE Char. - Client Characteristic Configuration
    BLE_FTMC_DESC_INDOOR_BIKE_CL_CFG,
    /// TRAINING_STATUS Char. - Client Characteristic Configuration
    BLE_FTMC_DESC_TRAINING_STATUS_CL_CFG,
    /// Control Point Char. - Client Characteristic Configuration
    BLE_FTMC_DESC_CTRL_PT_CL_CFG,
    /// FITNESS_MACHINE_STATUS Char. - Client Characteristic Configuration
    BLE_FTMC_DESC_FITNESS_MACHINE_STATUS_CFG,

    BLE_FTMC_DESC_MAX,

    BLE_FTMC_DESC_MASK = 0x10,
};

/// Codes for reading/writing a FTMS characteristic with one single request
enum ftmc_code
{
    /// Read FITNESS_MACHINE_FEATURE
    BLE_FITNESS_MACHINE_FEATURE          = FTM_CHAR_FEATURE,
    /// Notified TREADMILL_DATA
    BLE_TREADMILL_DATA                   = FTM_CHAR_TREADMILL_DATA,
    /// Notified CROSS_TRAINER_DATA
    BLE_CROSS_TRAINER_DATA               = FTM_CHAR_CROSS_TRAINER_DATA,
    /// Notified STEP_CLIMBER_DATA
    BLE_STEP_CLIMBER_DATA                = FTM_CHAR_STEP_CLIMBER_DATA_DATA,
    /// Notified STAIR_CLIMBER_DATA
    BLE_STAIR_CLIMBER_DATA               = FTM_CHAR_STAIR_CLIMBER_DATA_DATA,
    /// Notified ROWER_DATA
    BLE_ROWER_DATA                       = FTM_CHAR_ROWER_DATA,
    /// Notified INDOOR_BIKE_DATA
    BLE_INDOOR_BIKE_DATA                 = FTM_CHAR_INDOOR_BIKE_DATA,
    /// Read/Notified TRAINING_STATUS
    BLE_TRAINING_STATUS                  = FTM_CHAR_TRAINING_STATUS,
    /// Read SUPP_SPEED_RANGE
    BLE_SUPP_SPEED_RANGE                 = FTM_CHAR_SUPP_SPEED_RANGE,
    /// Read SUPP_INCLINATION_RANGE
    BLE_SUPP_INCLINATION_RANGE           = FTM_CHAR_SUPP_INCLINATION_RANGE,
    /// Read SUPP_RESIST_LEVEL_RANGE
    BLE_SUPP_RESIST_LEVEL_RANGE          = FTM_CHAR_SUPP_RESISTANCE_LEVEL_RANGE,
    /// Read SUPP_HEART_RATE_RANGE
    BLE_SUPP_HEART_RATE_RANGE            = FTM_CHAR_SUPP_HEART_RATE_RANGE,
    /// Read SUPP_POWER_RANGE
    BLE_SUPP_POWER_RANGE                 = FTM_CHAR_SUPP_POWER_RANGE,
    /// Indicated FTM Control Point
    BLE_FTMC_IND_CTRL_PT                 = FTM_CHAR_CPT,
    /// Indicated FITNESS_MACHINE_STATUS
    BLE_FITNESS_MACHINE_STATUS           = FTM_CHAR_STATUS,

    /// Read/Write TREADMILL_DATA Client Char. Configuration Descriptor
    BLE_FTMC_RD_WR_TREADMILL_DATA_CL_CFG  = (BLE_FTMC_DESC_TREADMILL_DATA_CL_CFG   | BLE_FTMC_DESC_MASK),
    /// Read/Write CROSS_TRAINER Client Char. Configuration Descriptor
    BLE_FTMC_RD_WR_CROSS_TRAINER_CL_CFG   = (BLE_FTMC_DESC_CROSS_TRAINER_CL_CFG   | BLE_FTMC_DESC_MASK),
    /// Read/Write STEP_CLIMBER Client Char. Configuration Descriptor
    BLE_FTMC_RD_WR_STEP_CLIMBER_CL_CFG    = (BLE_FTMC_DESC_STEP_CLIMBER_CL_CFG   | BLE_FTMC_DESC_MASK),
    /// Read/Write STAIR_CLIMBER Client Char. Configuration Descriptor
    BLE_FTMC_RD_WR_STAIR_CLIMBER_CL_CFG   = (BLE_FTMC_DESC_STAIR_CLIMBER_CL_CFG   | BLE_FTMC_DESC_MASK),
    /// Read/Write ROWER Client Char. Configuration Descriptor
    BLE_FTMC_RD_WR_ROWER_CL_CFG           = (BLE_FTMC_DESC_ROWER_CL_CFG   | BLE_FTMC_DESC_MASK),
    /// Read/Write INDOOR_BIKE Client Char. Configuration Descriptor
    BLE_FTMC_RD_WR_INDOOR_BIKE_CL_CFG     = (BLE_FTMC_DESC_INDOOR_BIKE_CL_CFG   | BLE_FTMC_DESC_MASK),
    /// Read/Write TRAINING_STATUS Client Char. Configuration Descriptor
    BLE_FTMC_RD_WR_TRAINING_STATUS_CL_CFG = (BLE_FTMC_DESC_TRAINING_STATUS_CL_CFG   | BLE_FTMC_DESC_MASK),

    /// Read/Write Control Point Client Char. Configuration Descriptor
    BLE_FTMC_RD_WR_CTRL_PT_CFG            = (BLE_FTMC_DESC_CTRL_PT_CL_CFG | BLE_FTMC_DESC_MASK),
    /// Read/Write FITNESS_MACHINE_STATUS Char. Configuration Descriptor
    BLE_FTMC_RD_WR_FITNESS_MACHINE_STATUS_CFG    = (BLE_FTMC_DESC_FITNESS_MACHINE_STATUS_CFG   | BLE_FTMC_DESC_MASK),
};

/**
 * CTRL_PT_REQUEST
 * CTRL_PT_RESET
 * CTRL_PT_SET_TARGET_STARTRESU
 * CTRL_PT_SET_TARGET_SPEED  //uint16_t 0.01km/h
 * CTRL_PT_SET_TARGET_INCLINATION //int16_t 0.1%
 * CTRL_PT_SET_TARGET_RESISLEVEL //uint8_t 0.1 unitless
 * CTRL_PT_SET_TARGET_POWER //int16_t 1 Watt
 * CTRL_PT_SET_TARGET_HEARTRATE //uint8_t 1 BPM
 * CTRL_PT_SET_TARGET_STOPPAUSE //uint8_t 01:stop;02:Pause;Other:RFU
 * CTRL_PT_SET_TARGET_EXPENERGY //uint16_t 1 Calorie
 * CTRL_PT_SET_TARGET_NUMSTEPS //uint16_t 1 step
 * CTRL_PT_SET_TARGET_NUMSTRIDS //uint16_t 1 stride
 * CTRL_PT_SET_TARGET_DISTANCE //uint24_t 1 meter
 * CTRL_PT_SET_TARGET_TRAINTIME //uint16_t 1 second
 * CTRL_PT_SET_TARGET_TIMEINTWOHR //uint16_t 1 second, uint16_t 1 second
 * CTRL_PT_SET_TARGET_TIMEINTHREEHR //uint16_t 1 second, uint16_t 1 second, uint16_t 1 second
 * CTRL_PT_SET_TARGET_TIMEINFIVEHR //uint16_t 1 second *5
 * CTRL_PT_SET_INDOOR_BIKESIMUL //int16_t 0.001 mps, int16_t 0.01%, uint8_t 0.0001 unitless, uint8_t 0.01 kg/m
 * CTRL_PT_SET_WHEEL_CIRCUMFER //uint16_t 0.1 Millimeters
 * CTRL_PT_SET_SPINDOWN_CONTRL //uint8_t 01:start;02:ignore;Other:RFU
 * CTRL_PT_SET_TARGETED_CADENCE //uint16_t 0.5 1/minute
 *
 */
typedef struct {
    uint16_t fat_burn_time;
    uint16_t fitness_time;
} target_training_time_two_heart_rate_param;

typedef struct {
    uint16_t light_time;
    uint16_t moderate_time;
    uint16_t hard_time;
} target_training_time_three_heart_rate_param;

typedef struct {
    uint16_t very_light_time;
    uint16_t light_time;
    uint16_t moderate_time;
    uint16_t hard_time;
    uint16_t max_time;
} target_training_time_five_heart_rate_param;

typedef struct {
    int16_t wind_speed;
    int16_t grade;
    uint8_t crr;
    uint8_t cw;
} indoor_bike_simulation_param;

typedef struct {
    uint8_t opcode;
    union
    {
        uint8_t paramu8_t;
        uint16_t paramu16_t;
        uint32_t paramu32_t;
        int16_t param16_t;
        target_training_time_two_heart_rate_param tws_hr;
        target_training_time_three_heart_rate_param three_hr;
        target_training_time_five_heart_rate_param five_hr;
        indoor_bike_simulation_param indoor_bike;
    } data;
} ftm_cpt_opcode;

typedef struct {
    gatt_prf_t head;
    gatt_peer_service_t *peer_service;
    uint16_t client_cfg;
    gatt_peer_character_t *peer_char[FTM_CHAR_MAX_NUM];
} ftmc_prf_t;

/// CP Control Point Response Value
typedef struct{
    /// SResponse Code Op Code
    uint8_t                   rsp_code;
    /// Request Op Code
    uint8_t                   req_code;
    /// Result Code
    uint16_t                  result_code;
    /// Response Parameter only exit when opcode is Spin Down(0x13)
    uint16_t                  target_speed_low;
    uint16_t                  target_speed_high;
}ble_ftm_ctrl_pt_rsp_val;

typedef struct ftms_threadmill_data
{
    /// Flags
    uint16_t flags;
    /// Instantaneous Speed
    uint16_t inst_speed;
    /// Average Speed
    uint16_t aver_speed;
    /// Total Distance
    uint32_t total_distance;
    /// Inclination
    int16_t inclination;//FTMS_DATA_NOT_AVAILABLE
    /// Ramp Angle Setting
    int16_t ramp_angle_setting;//FTMS_DATA_NOT_AVAILABLE
    /// Positive Elevation Gain and Negative Elevation Gain Field Pair
    uint16_t posi_eleva_gain;
    uint16_t nega_eleva_gain;
    /// Instantaneous Pace 
    uint16_t instantaneous_pace;
    /// Average Pace
    uint16_t average_pace;
    /// Total Energy
    uint16_t total_energy;
    /// Energy per Hour
    uint16_t energy_per_hour;
    /// Energy per Minute 
    uint8_t energy_per_min;
    /// Heart Rate
    uint8_t heart_rate;
    /// Metabolic Equivalent
    uint8_t meta_equi;
    /// Elapsed Time
    uint16_t elapsed_time;
    /// Remaining Time
    uint16_t remaining_time;
    /// Force on Belt
    int16_t belt;
    /// Power Output
    int16_t power_output;
} ftms_threadmill_data_t;

typedef struct ftms_cross_trainer_data
{
    /// Flags
    uint16_t flags;
    /// Instantaneous Speed
    uint16_t inst_speed;
    /// Average Speed
    uint16_t aver_speed;
    /// Total Distance
    uint32_t total_distance;
    /// Step per Minute
    uint16_t step_per_min;
    /// Average Step Rate
    uint16_t ave_step_rate;
    /// Stride Count
    uint16_t stride_count;
    /// Positive Elevation Gain and Negative Elevation Gain Field Pair
    uint16_t posi_eleva_gain;
    uint16_t nega_eleva_gain;
    /// Inclination
    int16_t inclination;//FTMS_DATA_NOT_AVAILABLE
    /// Ramp Angle Setting
    int16_t ramp_angle_setting;//FTMS_DATA_NOT_AVAILABLE
    /// Resistance Level
    int16_t resistance_level;
    /// Instantaneous power
    int16_t instant_power;
    /// Average Power
    int16_t average_power;
    /// Total Energy
    uint16_t total_energy;
    /// Energy per Hour
    uint16_t energy_per_hour;
    /// Energy per Minute 
    uint8_t energy_per_min;
    /// Heart Rate
    uint8_t heart_rate;
    /// Metabolic Equivalent
    uint8_t meta_equi;
    /// Elapsed Time
    uint16_t elapsed_time;
    /// Remaining Time
    uint16_t remaining_time;
} ftms_cross_trainer_data_t;

typedef struct ftms_step_climber_data
{
    /// Flags
    uint16_t flags;
    /// Floors
    uint16_t floors;
    /// Step Count
    uint16_t step_count;
    /// Step per Minute
    uint16_t step_per_min;
    /// Average Step Rate
    uint16_t ave_step_rate;
    /// Positive Elevation Gain
    uint16_t posi_eleva_gain;
    /// Total Energy
    uint16_t total_energy;
    /// Energy per Hour
    uint16_t energy_per_hour;
    /// Energy per Minute 
    uint8_t energy_per_min;
    /// Heart Rate
    uint8_t heart_rate;
    /// Metabolic Equivalent
    uint8_t meta_equi;
    /// Elapsed Time
    uint16_t elapsed_time;
    /// Remaining Time
    uint16_t remaining_time;
} ftms_step_climber_data_t;

typedef struct ftms_stair_climber_data
{
    /// Flags
    uint16_t flags;
    /// Floors
    uint16_t floors;
    /// Step per Minute rate
    uint16_t step_per_min;
    /// Average Step Rate
    uint16_t ave_step_rate;
    /// Positive Elevation Gain
    uint16_t posi_eleva_gain;
    /// Stride Count
    uint16_t stride_count;
    /// Total Energy
    uint16_t total_energy;
    /// Energy per Hour
    uint16_t energy_per_hour;
    /// Energy per Minute 
    uint8_t energy_per_min;
    /// Heart Rate
    uint8_t heart_rate;
    /// Metabolic Equivalent
    uint8_t meta_equi;
    /// Elapsed Time
    uint16_t elapsed_time;
    /// Remaining Time
    uint16_t remaining_time;
} ftms_stair_climber_data_t;

typedef struct ftms_rower_data
{
    /// Flags
    uint16_t flags;
    /// Stroke Rate
    uint8_t stroke_rate;
    /// Stroke Count
    uint16_t stroke_count;
    /// Average Stroke Rate
    uint8_t average_stroke_rate;
    /// Total Distance
    uint32_t total_distance;
    /// Instantaneous Pace 
    uint16_t instantaneous_pace;
    /// Average Pace
    uint16_t average_pace;
    /// Instantaneous power
    int16_t instant_power;
    /// Average Power
    int16_t average_power;
    /// Resistance Level
    int16_t resistance_level;
    /// Total Energy
    uint16_t total_energy;
    /// Energy per Hour
    uint16_t energy_per_hour;
    /// Energy per Minute 
    uint8_t energy_per_min;
    /// Heart Rate
    uint8_t heart_rate;
    /// Metabolic Equivalent
    uint8_t meta_equi;
    /// Elapsed Time
    uint16_t elapsed_time;
    /// Remaining Time
    uint16_t remaining_time;
} ftms_rower_data_t;

typedef struct ftms_indoor_bike_data
{
    /// Flags
    uint16_t flags;
    /// Instantaneous Speed
    uint16_t inst_speed;
    /// Average Speed
    uint16_t aver_speed;
    /// Instantaneous Cadence
    uint16_t inst_cadence;
    /// Average Cadence
    uint16_t aver_cadence;
    /// Total Distance
    uint32_t total_distance;
    /// Resistance Level
    int16_t resistance_level;
    /// Instantaneous power
    int16_t instant_power;
    /// Average Power
    int16_t average_power;
    /// Total Energy
    uint16_t total_energy;
    /// Energy per Hour
    uint16_t energy_per_hour;
    /// Energy per Minute 
    uint8_t energy_per_min;
    /// Heart Rate
    uint8_t heart_rate;
    /// Metabolic Equivalent
    uint8_t meta_equi;
    /// Elapsed Time
    uint16_t elapsed_time;
    /// Remaining Time
    uint16_t remaining_time;
} ftms_indoor_bike_data_t;

/// training_status
enum ftmc_training_status {
    FTM_STRAINING_STATUS_UNKNOWN = 0x00,
    FTM_STRAINING_STATUS_IDLE,
    FTM_STRAINING_STATUS_WARMING_UP,
    FTM_STRAINING_STATUS_LOW_INT_INTERVAL,
    FTM_STRAINING_STATUS_HIGH_INT_INTERVAL,
    FTM_STRAINING_STATUS_RECO_INTERVAL,
    FTM_STRAINING_STATUS_ISOMETRIC,
    FTM_STRAINING_STATUS_HEART_RATE_CTL,
    FTM_STRAINING_STATUS_FITNESS_TEST,
    FTM_STRAINING_STATUS_SPEEDOUTSIDE_LOW,//9
    FTM_STRAINING_STATUS_SPEEDOUTSIDE_HIGH,
    FTM_STRAINING_STATUS_COOL_DOWN,
    FTM_STRAINING_STATUS_WATT_CTL,
    FTM_STRAINING_STATUS_MANUAL_MODE,
    FTM_STRAINING_STATUS_PREWORKOUT,
    FTM_STRAINING_STATUS_POSTWORKOUT,//15
    //0x10-0xff reservd
};

typedef struct ftms_supp_speed_range
{
    /// min speed
    uint16_t min_speed;
    /// max speed
    uint16_t max_speed;
    /// min increment
    uint16_t min_increment;
} ftms_supp_speed_range_t;

typedef struct ftms_supp_inclination_range
{
    /// min inclination
    uint16_t min_inclination;
    /// max inclination
    uint16_t max_inclination;
    /// min increment
    uint16_t min_increment;
} ftms_supp_inclination_range_t;

typedef struct ftms_supp_resis_level_range
{
    /// min inclination
    uint16_t min_inclination;
    /// max inclination
    uint16_t max_inclination;
    /// min increment
    uint16_t min_increment;
} ftms_supp_resis_level_range_t;

typedef struct ftms_supp_power_range
{
    /// min inclination
    uint16_t min_inclination;
    /// max inclination
    uint16_t max_inclination;
    /// min increment
    uint16_t min_increment;
} ftms_supp_power_range_t;

typedef struct ftms_supp_heart_rate_range
{
    /// min heart rate
    uint16_t min_heart_rate;
    /// max heart rate
    uint16_t max_heart_rate;
    /// min increment
    uint16_t min_increment;
} ftms_supp_heart_rate_range_t;
#ifdef __cplusplus
    }
#endif
#endif /* BLE_FTMC_ENABLED||BLE_FTMS_ENABLED */
#endif /* __BLE_FTMP_H__ */
