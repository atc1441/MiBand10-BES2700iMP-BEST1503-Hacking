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

#ifndef _BLE_HRP_COMMON_H_
#define _BLE_HRP_COMMON_H_

#include "prf_types.h"
#include "co_math.h"

#ifdef __cplusplus
extern "C" {
#endif

/// maximum number of RR-Interval supported
#define HRS_MAX_RR_INTERVAL  (4)

/// Heart Rate Control Point Not Supported error code
#define HRS_ERR_HR_CNTL_POINT_NOT_SUPPORTED   (0x80)

/// HRS codes for the 2 possible client configuration characteristic descriptors determination
enum
{
    /// Heart Rate Measurement
    HRS_HR_MEAS_CODE = 0x01,
    /// Energy Expended - Heart Rate Control Point
    HRS_HR_CNTL_POINT_CODE,
};

/// Heart Rate Measurement Flags field bit values
enum hrp_meas_flags_bf
{
    /// Heart Rate Value Format bit
    /// 0 for UINT8 and 1 for UINT16. Units: beats per minute (bpm)
    HRS_FLAG_HR_VALUE_FORMAT_POS                = 0,
    HRS_FLAG_HR_VALUE_FORMAT_BIT                = CO_BIT(HRS_FLAG_HR_VALUE_FORMAT_POS),

    /// Sensor Contact Status bits
    /// Sensor Contact feature supported in the current connection
    /// Contact is detected
    HRS_FLAG_SENSOR_CCT_DETECTED_POS            = 1,
    HRS_FLAG_SENSOR_CCT_DETECTED_BIT            = CO_BIT(HRS_FLAG_SENSOR_CCT_DETECTED_POS),

    HRS_FLAG_SENSOR_CCT_FET_SUPPORTED_POS       = 2,
    HRS_FLAG_SENSOR_CCT_FET_SUPPORTED_BIT       = CO_BIT(HRS_FLAG_SENSOR_CCT_FET_SUPPORTED_POS),

    /// Energy Expended Status bit
    /// Energy Expended field is present. Units: kilo Joules
    HRS_FLAG_ENERGY_EXPENDED_PRESENT_POS        = 3,
    HRS_FLAG_ENERGY_EXPENDED_PRESENT_BIT        = CO_BIT(HRS_FLAG_ENERGY_EXPENDED_PRESENT_POS),

    /// RR-Interval bit
    /// One or more RR-Interval values are present. Units: 1/1024 seconds
    HRS_FLAG_RR_INTERVAL_PRESENT_POS            = 4,
    HRS_FLAG_RR_INTERVAL_PRESENT_BIT            = CO_BIT(HRS_FLAG_RR_INTERVAL_PRESENT_POS),
};

/// Body Sensor Location
typedef enum hrp_sen_loc
{
    HRS_LOC_OTHER       = 0,
    HRS_LOC_CHEST       = 1,
    HRS_LOC_WRIST       = 2,
    HRS_LOC_FINGER      = 3,
    HRS_LOC_HAND        = 4,
    HRS_LOC_EAR_LOBE    = 5,
    HRS_LOC_FOOT        = 6,
    HRS_LOC_MAX,
} hrp_sen_loc_t;

/// Heart Rate measurement structure
typedef struct hrs_hr_meas
{
    /// Flag
    uint8_t  flags;
    /// Heart Rate Measurement Value
    uint16_t heart_rate;
    /// Energy Expended
    uint16_t energy_expended;
    /// RR-Interval numbers (max 4)
    uint8_t  nb_rr_interval;
    /// RR-Intervals
    uint16_t rr_intervals[HRS_MAX_RR_INTERVAL];
} hrs_hr_meas_t;

#ifdef __cplusplus
}
#endif

#endif /* _BLE_HRP_COMMON_H_ */