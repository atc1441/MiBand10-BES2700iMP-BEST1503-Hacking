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
#ifdef BLE_FTMS_ENABLED // Fitness Machine Service
#include "gatt_service.h"
#include "ble_ftms.h"

/* DEFINE */
/// Check if a specific characteristic has been added in the database (according to mask)
#define FTMS_IS_FEATURE_SUPPORTED(features, flag) ((features & flag) == flag)

#define INVALID_CONIDX 0xFF

/// FTMS Machine Available flag
#define FTMS_FTMS_MACHINE_AVA_FLAG              (0x01)
/// Treadmill Data supported flag
#define FTMS_TREADMILL_DATA_CHAR_SUPP_FLAG      (0x01)
/// Cross Trainer Data supported flag
#define FTMS_CROSS_TRAINER_DATA_CHAR_SUPP_FLAG  (0x02)
/// Step Climber Data supported flag
#define FTMS_STEP_CLIMBER_DATA_CHAR_SUPP_FLAG   (0x04)
/// Stair Climber Data supported flag
#define FTMS_STAIR_CLIMBER_DATA_CHAR_SUPP_FLAG  (0x08)
/// Rower Data supported flag
#define FTMS_ROWER_DATA_CHAR_SUPP_FLAG          (0x10)
/// Indoor Bike Data supported flag
#define FTMS_INDOOR_BIKE_DATA_CHAR_SUPP_FLAG    (0x20)
/// Training Status supported flag
#define FTMS_TRAINING_STATUS_CHAR_SUPP_FLAG     (0x40)
/// Control Point supported flag
#define FTMS_CTRL_PT_CHAR_SUPP_FLAG             (0x80)

#define FTMS_2_GAP_CONIDX_BF(conidx)  (gap_conn_bf(gap_zero_based_conidx_to_ble_conidx(conidx)))

#define FTM_PREFERRED_MTU 512

typedef struct ftms_cnx_env
{
    /// Profile Notify/Indication Flags
    uint16_t  prfl_ntf_ind_cfg;
} ble_ftms_cnx_env_t;

/// FTMS Environment Variable
typedef struct ftms_env
{
    /// Operation Event TX wait queue
    struct list_node   wait_queue;
    /// Event callabcks
    const ble_ftms_cb_t *p_cb;
    /// Environment variable pointer for each connections
    ble_ftms_cnx_env_t env[BLE_CONNECTION_MAX];
    /// Feature Configuration Flags
    uint32_t ftms_feature;
    /// Target Setting Flags
    uint32_t target_setting_feature;
    /// Profile Configuration Flags
    uint8_t prfl_config;
    /// supp_speed_range
    ftms_supp_speed_range_t speed_range;
    /// supp_inclination_range
    ftms_supp_inclination_range_t inclination_range;
    /// supp_resis_level_range
    ftms_supp_resis_level_range_t resis_level_range;
    /// supp_power_range
    ftms_supp_power_range_t power_range;
    /// supp_heart_rate_range
    ftms_supp_heart_rate_range_t heart_rate_range;
    /// Target speed
    uint16_t      target_speed;
    /// Target Inclination
    int16_t      target_inclination;
    /// Target Resistance Level
    uint8_t      target_resis_level;
    /// Target power
    int16_t      target_power;
    /// Target heart rate
    uint8_t      target_heart_rate;
    /// Stop or pause
    uint8_t      stop_pause;
    /// Target Expended Energy
    uint16_t      target_exp_energy;
    /// Target num of steps
    uint16_t      target_num_steps;
    /// Target num of strides
    uint16_t      target_num_strides;
    /// Target Distance
    uint32_t      target_distance;
    /// Target Training Time
    uint16_t      target_trainging_time;
    /// Target two heart rate training time
    target_training_time_two_heart_rate_param      target_two_heart_rate;
    /// Target three heart rate training time
    target_training_time_three_heart_rate_param      target_three_heart_rate;
    /// Target five heart rate training time
    target_training_time_five_heart_rate_param      target_five_heart_rate;
    /// Target indoor bike simulation
    indoor_bike_simulation_param target_indoor_bike;
    /// training_status
    uint8_t training_status;
    /// Wheel circumference
    uint16_t      wheel_circ;
    /// Elapsed Time
    uint16_t      elapsed_time;
    /// Remaining Time
    uint16_t      remaining_time;
    /// Spin down control
    uint8_t      spin_down;
    /// Target cadence
    uint16_t      candence;
    /// Control point operation on-going
    uint8_t        ctrl_pt_op;
    /// request the control of conidx
    uint8_t        ctrl_conidx;
    /// Operation On-going
    bool           op_ongoing;
    /// Prevent recursion in execute_operation function
    bool           in_exe_op;
    /// Service attribute list
    gatt_attribute_t *p_attr_list;
} ble_ftms_env_t;

ble_ftms_env_t *p_ftms_env = NULL;

/// Fitness Machine Service - Attribute List
enum ftms_att_list
{
    /// Fitness Machine Service
    FTMS_IDX_SVC,
    /// FTMS feature
    FTMS_IDX_FEATURE_CHAR,
    /// FTMS treadmill_data
    FTMS_IDX_TREADMILL_DATA_CHAR,
    FTMS_IDX_TREADMILL_DATA_NTF_CFG,
    /// FTMS cross_trainer_data
    FTMS_IDX_CROSS_TRAINER_DATA_CHAR,
    FTMS_IDX_CROSS_TRAINER_DATA_NTF_CFG,
    /// FTMS step_climber_data
    FTMS_IDX_STEP_CLIMBER_DATA_CHAR,
    FTMS_IDX_STEP_CLIMBER_DATA_NTF_CFG,
    /// FTMS stair_climber_data
    FTMS_IDX_STAIR_CLIMBER_DATA_CHAR,
    FTMS_IDX_STAIR_CLIMBER_DATA_NTF_CFG,
    /// FTMS rower_data
    FTMS_IDX_ROWER_DATA_CHAR,
    FTMS_IDX_ROWER_DATA_NTF_CFG,
    /// FTMS indoor_bike_data
    FTMS_IDX_INDOOR_BIKE_DATA_CHAR,
    FTMS_IDX_INDOOR_BIKE_DATA_NTF_CFG,
    /// FTMS training_status
    FTMS_IDX_TRAINING_STATUS_CHAR,
    FTMS_IDX_TRAINING_STATUS_NTF_CFG,
    /// FTMS supp_speed_range
    FTMS_IDX_SUPP_SPEED_RANGE_CHAR,
    /// FTMS supp_inclination_range
    FTMS_IDX_SUPP_INCLINATION_RANGE_CHAR,
    /// FTMS supp_resist_level_range
    FTMS_IDX_SUPP_RESIST_LEVEL_RANGE_CHAR,
    /// FTMS supp_heart_rate_range
    FTMS_IDX_SUPP_HEART_RATE_RANGE_CHAR,
    /// FTMS supp_power_range
    FTMS_IDX_SUPP_POWER_RANGE_CHAR,
    /// FTMS Control Point
    FTMS_IDX_CTRL_PT_CHAR,
    FTMS_IDX_CTRL_PT_IND_CFG,
    /// FTMS ftm_status
    FTMS_IDX_FITNESS_MACHINE_STATUS_CHAR,
    FTMS_IDX_FITNESS_MACHINE_STATUS_NTF_CFG,

    /// Number of attributes
    FTMS_IDX_NB,
};

/// Operation Code used in the profile state machine
enum ftms_op_code
{
    /// Reserved Operation Code
    FTMS_RESERVED_OP_CODE                          = 0x00,
    /// Send treadmill_data Operation Code
    FTMS_NTF_TREADMILL_DATA_OP_CODE                = 0x01,
    /// Send cross_trainer_data Operation Code
    FTMS_NTF_CROSS_TRAINER_DATA_OP_CODE            = 0x02,
    /// Send step_climber_data Operation Code
    FTMS_NTF_STEP_CLIMBER_DATA_OP_CODE             = 0x03,
    /// Send stair_climber_data Operation Code
    FTMS_NTF_STAIR_CLIMBER_DATA_OP_CODE            = 0x04,
    /// Send rower_data Operation Code
    FTMS_NTF_ROWER_DATA_OP_CODE                    = 0x05,
    /// Send indoor_bike_data Operation Code
    FTMS_NTF_INDOOR_BIKE_DATA_OP_CODE              = 0x06,
    /// Send training_status Operation Code
    FTMS_NTF_TRAINING_STATUS_OP_CODE               = 0x07,
    /// Send Control Point Operation Code
    FTMS_IND_CTRL_PT_OP_CODE                       = 0x08,
    /// Send ftm_status response
    FTMS_NTF_FITNESS_MACHINE_STATUS_OP_CODE        = 0x09,
};

/* TYPEDEF */
/// ongoing operation information
typedef struct ftms_buf_meta
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
} ble_ftms_buf_meta_t;

GATT_DECL_PRI_SERVICE(g_ble_ftm_service, GATT_UUID_FIT_SERVICE);

GATT_DECL_CHAR(g_ble_ftm_feature_character,
    GATT_CHAR_UUID_FITNESS_MACHINE_FEATURE,
    GATT_RD_REQ,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_treadmill_data_character,
    GATT_CHAR_UUID_TREADMILL_DATA,
    GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_treadmill_data_cccd,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_cross_trainer_data_character,
    GATT_CHAR_UUID_CROSS_TRAINER_DATA,
    GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_cross_trainer_data_cccd,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_step_climber_data_character,
    GATT_CHAR_UUID_STEP_CLIMBER_DATA,
    GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_step_climber_data_cccd,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_stair_climber_data_character,
    GATT_CHAR_UUID_STAIR_CLIMBER_DATA,
    GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_stair_climber_data_cccd,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_rower_data_character,
    GATT_CHAR_UUID_ROWER_DATA,
    GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_rower_data_cccd,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_indoor_bike_data_character,
    GATT_CHAR_UUID_INDOOR_BIKE_DATA,
    GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_indoor_bike_data_cccd,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_training_status_character,
    GATT_CHAR_UUID_TRAINING_STATUS,
    GATT_RD_REQ|GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_training_status_cccd,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_supp_speed_range_character,
    GATT_CHAR_UUID_SUPP_SPEED_RANGE,
    GATT_RD_REQ,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_supp_inclination_range_character,
    GATT_CHAR_UUID_SUPP_INCLINATION_RANGE,
    GATT_RD_REQ,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_supp_resist_level_range_character,
    GATT_CHAR_UUID_SUPP_RESIST_LEVEL_RANGE,
    GATT_RD_REQ,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_supp_heart_rate_range_character,
    GATT_CHAR_UUID_SUPP_HEART_RATE_RANGE,
    GATT_RD_REQ,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_supp_power_range_character,
    GATT_CHAR_UUID_SUPP_POWER_RANGE,
    GATT_RD_REQ,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_ftm_control_point_character,
    GATT_CHAR_UUID_FITNESS_MACHINE_CTRL_POINT,
    GATT_WR_REQ|GATT_IND_PROP,
    ATT_WR_ENC);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_ftm_control_point_cccd,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_ftm_status_character,
    GATT_CHAR_UUID_FITNESS_MACHINE_STATUS,
    GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_ftm_status_cccd,
    ATT_SEC_NONE);

static const gatt_attribute_t ble_ftms_att_db[FTMS_IDX_NB] = {
    /* Service */
    gatt_attribute(g_ble_ftm_service),
    /* Characteristics */
    gatt_attribute(g_ble_ftm_feature_character),
    /* Characteristics */
    gatt_attribute(g_ble_treadmill_data_character),
        gatt_attribute(g_ble_treadmill_data_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_cross_trainer_data_character),
        gatt_attribute(g_ble_cross_trainer_data_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_step_climber_data_character),
        gatt_attribute(g_ble_step_climber_data_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_stair_climber_data_character),
        gatt_attribute(g_ble_stair_climber_data_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_rower_data_character),
        gatt_attribute(g_ble_rower_data_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_indoor_bike_data_character),
        gatt_attribute(g_ble_indoor_bike_data_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_training_status_character),
        gatt_attribute(g_ble_training_status_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_supp_speed_range_character),
    /* Characteristics */
    gatt_attribute(g_ble_supp_inclination_range_character),
    /* Characteristics */
    gatt_attribute(g_ble_supp_resist_level_range_character),
    /* Characteristics */
    gatt_attribute(g_ble_supp_heart_rate_range_character),
    /* Characteristics */
    gatt_attribute(g_ble_supp_power_range_character),
    /* Characteristics */
    gatt_attribute(g_ble_ftm_control_point_character),
        gatt_attribute(g_ble_ftm_control_point_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_ftm_status_character),
        gatt_attribute(g_ble_ftm_status_cccd),
};

/* EXTERNAL FUNCTIONS */
extern uint8_t ble_prf_pack_date_time(uint8_t *p_buf, const prf_date_time_t *p_date_time);

/* INTERNAL FUNCTIONS */
static void ble_ftms_exe_operation(void);

/// Get database attribute index
static uint8_t ble_ftms_idx_get(const gatt_attribute_t *attr)
{
    uint8_t att_idx = ((uint32_t)attr - (uint32_t)p_ftms_env->p_attr_list) / sizeof(gatt_attribute_t);

    if (att_idx >= FTMS_IDX_TREADMILL_DATA_CHAR)
    {
        if(!FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_TREADMILL_DATA_CHAR_SUPP_FLAG))
        {
            att_idx += 2;
        }

        if(!FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_CROSS_TRAINER_DATA_CHAR_SUPP_FLAG))
        {
            att_idx += 2;
        }

        if(!FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_STEP_CLIMBER_DATA_CHAR_SUPP_FLAG))
        {
            att_idx += 2;
        }

        if(!FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_STAIR_CLIMBER_DATA_CHAR_SUPP_FLAG))
        {
            att_idx += 2;
        }

        if(!FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_ROWER_DATA_CHAR_SUPP_FLAG))
        {
            att_idx += 2;
        }

        if(!FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_INDOOR_BIKE_DATA_CHAR_SUPP_FLAG))
        {
            att_idx += 2;
        }

        if(!FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_TRAINING_STATUS_CHAR_SUPP_FLAG))
        {
            att_idx += 2;
        }

        if(!FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_CTRL_PT_CHAR_SUPP_FLAG))
        {
            ///if control point not support, then the ftms status also not support
            att_idx += 4;
        }

        att_idx++;
    }

    if (att_idx >= FTMS_IDX_SUPP_SPEED_RANGE_CHAR)
    {
        if(!FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->target_setting_feature, FTMS_TARGET_FEAT_SPEED_SUP_BIT))
        {
            att_idx += 1;
        }

        if(!FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->target_setting_feature, FTMS_TARGET_FEAT_INCLINATION_SUP_BIT))
        {
            att_idx += 1;
        }

        if(!FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->target_setting_feature, FTMS_TARGET_FEAT_RESISTANCE_SUP_BIT))
        {
            att_idx += 1;
        }

        if(!FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->target_setting_feature, FTMS_TARGET_FEAT_POWER_SUP_BIT))
        {
            att_idx += 1;
        }

        if(!FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->target_setting_feature, FTMS_TARGET_FEAT_HEART_RATE_SUP_BIT))
        {
            att_idx += 1;
        }
    }

    return att_idx;
}

static void reset_ftms_target_data_record(void)
{
    p_ftms_env->target_speed = 0;
    p_ftms_env->target_inclination = 0;
    p_ftms_env->target_resis_level = 0;
    p_ftms_env->target_power = 0;
    p_ftms_env->target_heart_rate = 0;
    p_ftms_env->stop_pause = 0;
    p_ftms_env->target_exp_energy = 0;
    p_ftms_env->target_num_steps = 0;
    p_ftms_env->target_num_strides = 0;
    p_ftms_env->target_distance = 0;
    p_ftms_env->elapsed_time = 0;
    p_ftms_env->remaining_time = 0;
    p_ftms_env->target_trainging_time = 0;
    memset(&p_ftms_env->target_two_heart_rate, 0, sizeof(target_training_time_two_heart_rate_param));
    memset(&p_ftms_env->target_three_heart_rate, 0, sizeof(target_training_time_three_heart_rate_param));
    memset(&p_ftms_env->target_five_heart_rate, 0, sizeof(target_training_time_five_heart_rate_param));
    memset(&p_ftms_env->target_indoor_bike, 0, sizeof(indoor_bike_simulation_param));
}

static uint16_t ble_ftms_unpack_ctrl_point_req(uint8_t conidx, const uint8_t *p_buf, uint8_t buf_len)
{
    uint8_t op_code;
    uint8_t ctrl_pt_rsp_status = FTM_RESULT_OPCODE_NOT_SUPPORTED;
    uint16_t status = BT_STS_SUCCESS;
    ftm_cpt_opcode value;
    memset(&value, 0, sizeof(ftm_cpt_opcode));
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
        case (CTRL_PT_REQUEST):
        {
            // Check if the Wheel Revolution Data feature is supported
            if (p_ftms_env->ctrl_conidx == INVALID_CONIDX)
            {
                // The request can be handled
                ctrl_pt_rsp_status = FTM_RESULT_SUCCESS;
                // Update the environment
                p_ftms_env->ctrl_conidx = conidx;
                p_ftms_env->ctrl_pt_op = op_code;
            }
        }
        break;

        case (CTRL_PT_RESET):
        {
            if (p_ftms_env->ctrl_conidx == conidx)
            {
                // The request can be handled
                ctrl_pt_rsp_status = FTM_RESULT_SUCCESS;
                // Update the environment
                p_ftms_env->ctrl_pt_op = op_code;
                reset_ftms_target_data_record();
            }
        }
        break;

        case (CTRL_PT_SET_TARGET_SPEED):
        {
            if (p_ftms_env->ctrl_conidx == conidx)
            {
                // The request can be handled
                ctrl_pt_rsp_status = FTM_RESULT_SUCCESS;
                // Update the environment
                p_ftms_env->ctrl_pt_op = op_code;
                p_ftms_env->target_speed = CO_COMBINE_UINT16_LE(p_buf);
            }
        }
        break;

        case (CTRL_PT_SET_TARGET_INCLINATION):
        {
            if (p_ftms_env->ctrl_conidx == conidx)
            {
                // The request can be handled
                ctrl_pt_rsp_status = FTM_RESULT_SUCCESS;
                // Update the environment
                p_ftms_env->ctrl_pt_op = op_code;
                p_ftms_env->target_inclination = CO_COMBINE_UINT16_LE(p_buf);
            }
        }
        break;

        case (CTRL_PT_SET_TARGET_RESISLEVEL):
        {
            if (p_ftms_env->ctrl_conidx == conidx)
            {
                // The request can be handled
                ctrl_pt_rsp_status = FTM_RESULT_SUCCESS;
                // Update the environment
                p_ftms_env->ctrl_pt_op = op_code;
                p_ftms_env->target_resis_level = p_buf[0];
            }
        }
        break;

        case (CTRL_PT_SET_TARGET_POWER):
        {
            if (p_ftms_env->ctrl_conidx == conidx)
            {
                // The request can be handled
                ctrl_pt_rsp_status = FTM_RESULT_SUCCESS;
                // Update the environment
                p_ftms_env->ctrl_pt_op = op_code;
                p_ftms_env->target_power = CO_COMBINE_UINT16_LE(p_buf);
            }
        }
        break;

        case (CTRL_PT_SET_TARGET_HEARTRATE):
        {
            if (p_ftms_env->ctrl_conidx == conidx)
            {
                // The request can be handled
                ctrl_pt_rsp_status = FTM_RESULT_SUCCESS;
                // Update the environment
                p_ftms_env->ctrl_pt_op = op_code;
                p_ftms_env->target_heart_rate = p_buf[0];
            }
        }
        break;

        case (CTRL_PT_SET_TARGET_STARTRESU):
        {
            if (p_ftms_env->ctrl_conidx == conidx)
            {
                // The request can be handled
                ctrl_pt_rsp_status = FTM_RESULT_SUCCESS;
                // Update the environment
                p_ftms_env->ctrl_pt_op = op_code;
            }
        }
        break;

        case (CTRL_PT_SET_TARGET_STOPPAUSE):
        {
            if (p_ftms_env->ctrl_conidx == conidx)
            {
                // The request can be handled
                ctrl_pt_rsp_status = FTM_RESULT_SUCCESS;
                // Update the environment
                p_ftms_env->ctrl_pt_op = op_code;
            }
        }
        break;

        case (CTRL_PT_SET_TARGET_EXPENERGY):
        {
            if (p_ftms_env->ctrl_conidx == conidx)
            {
                // The request can be handled
                ctrl_pt_rsp_status = FTM_RESULT_SUCCESS;
                // Update the environment
                p_ftms_env->ctrl_pt_op = op_code;
                p_ftms_env->target_exp_energy = CO_COMBINE_UINT16_LE(p_buf);
            }
        }
        break;

        case (CTRL_PT_SET_TARGET_NUMSTEPS):
        {
            if (p_ftms_env->ctrl_conidx == conidx)
            {
                // The request can be handled
                ctrl_pt_rsp_status = FTM_RESULT_SUCCESS;
                // Update the environment
                p_ftms_env->ctrl_pt_op = op_code;
                p_ftms_env->target_num_steps = CO_COMBINE_UINT16_LE(p_buf);
            }
        }
        break;

        case (CTRL_PT_SET_TARGET_NUMSTRIDS):
        {
            if (p_ftms_env->ctrl_conidx == conidx)
            {
                // The request can be handled
                ctrl_pt_rsp_status = FTM_RESULT_SUCCESS;
                // Update the environment
                p_ftms_env->ctrl_pt_op = op_code;
                p_ftms_env->target_num_strides = CO_COMBINE_UINT16_LE(p_buf);
            }
        }
        break;

        case (CTRL_PT_SET_TARGET_DISTANCE):
        {
            if (p_ftms_env->ctrl_conidx == conidx)
            {
                // The request can be handled
                ctrl_pt_rsp_status = FTM_RESULT_SUCCESS;
                // Update the environment
                p_ftms_env->ctrl_pt_op = op_code;
                p_ftms_env->target_distance = CO_COMBINE_UINT32_LE(p_buf);
            }
        }
        break;

        case (CTRL_PT_SET_TARGET_TRAINTIME):
        {
            if (p_ftms_env->ctrl_conidx == conidx)
            {
                // The request can be handled
                ctrl_pt_rsp_status = FTM_RESULT_SUCCESS;
                // Update the environment
                p_ftms_env->ctrl_pt_op = op_code;
                p_ftms_env->target_trainging_time = CO_COMBINE_UINT16_LE(p_buf);
            }
        }
        break;

        case (CTRL_PT_SET_TARGET_TIMEINTWOHR):
        {
            if (p_ftms_env->ctrl_conidx == conidx && buf_len == sizeof(target_training_time_two_heart_rate_param))
            {
                // The request can be handled
                ctrl_pt_rsp_status = FTM_RESULT_SUCCESS;
                // Update the environment
                p_ftms_env->ctrl_pt_op = op_code;
                p_ftms_env->target_two_heart_rate.fat_burn_time = CO_COMBINE_UINT16_LE(p_buf);
                p_buf += 2;
                p_ftms_env->target_two_heart_rate.fitness_time = CO_COMBINE_UINT16_LE(p_buf);
            }
        }
        break;

        case (CTRL_PT_SET_TARGET_TIMEINTHREEHR):
        {
            if (p_ftms_env->ctrl_conidx == conidx && buf_len == sizeof(target_training_time_three_heart_rate_param))
            {
                // The request can be handled
                ctrl_pt_rsp_status = FTM_RESULT_SUCCESS;
                // Update the environment
                p_ftms_env->ctrl_pt_op = op_code;
                p_ftms_env->target_three_heart_rate.light_time = CO_COMBINE_UINT16_LE(p_buf);
                p_buf += 2;
                p_ftms_env->target_three_heart_rate.moderate_time = CO_COMBINE_UINT16_LE(p_buf);
                p_buf += 2;
                p_ftms_env->target_three_heart_rate.hard_time = CO_COMBINE_UINT16_LE(p_buf);
            }
        }
        break;

        case (CTRL_PT_SET_TARGET_TIMEINFIVEHR):
        {
            if (p_ftms_env->ctrl_conidx == conidx && buf_len == sizeof(target_training_time_five_heart_rate_param))
            {
                // The request can be handled
                ctrl_pt_rsp_status = FTM_RESULT_SUCCESS;
                // Update the environment
                p_ftms_env->ctrl_pt_op = op_code;
                p_ftms_env->target_five_heart_rate.very_light_time = CO_COMBINE_UINT16_LE(p_buf);
                p_buf += 2;
                p_ftms_env->target_five_heart_rate.light_time = CO_COMBINE_UINT16_LE(p_buf);
                p_buf += 2;
                p_ftms_env->target_five_heart_rate.moderate_time = CO_COMBINE_UINT16_LE(p_buf);
                p_buf += 2;
                p_ftms_env->target_five_heart_rate.max_time = CO_COMBINE_UINT16_LE(p_buf);
                p_buf += 2;
                p_ftms_env->target_five_heart_rate.hard_time = CO_COMBINE_UINT16_LE(p_buf);
            }
        }
        break;

        case (CTRL_PT_SET_INDOOR_BIKESIMUL):
        {
            if (p_ftms_env->ctrl_conidx == conidx && buf_len == sizeof(indoor_bike_simulation_param))
            {
                // The request can be handled
                ctrl_pt_rsp_status = FTM_RESULT_SUCCESS;
                // Update the environment
                p_ftms_env->ctrl_pt_op = op_code;
                p_ftms_env->target_indoor_bike.wind_speed = CO_COMBINE_UINT16_LE(p_buf);
                p_buf += 2;
                p_ftms_env->target_indoor_bike.grade = CO_COMBINE_UINT16_LE(p_buf);
                p_buf += 2;
                p_ftms_env->target_indoor_bike.crr = p_buf[0];
                p_buf += 1;
                p_ftms_env->target_indoor_bike.cw = p_buf[0];
            }
        }
        break;

        case (CTRL_PT_SET_WHEEL_CIRCUMFER):
        {
            if (p_ftms_env->ctrl_conidx == conidx)
            {
                // The request can be handled
                ctrl_pt_rsp_status = FTM_RESULT_SUCCESS;
                // Update the environment
                p_ftms_env->ctrl_pt_op = op_code;
                p_ftms_env->spin_down = p_buf[0];
            }
        }
        break;

        case (CTRL_PT_SET_TARGETED_CADENCE):
        {
            if (p_ftms_env->ctrl_conidx == conidx)
            {
                // The request can be handled
                ctrl_pt_rsp_status = FTM_RESULT_SUCCESS;
                // Update the environment
                p_ftms_env->ctrl_pt_op = op_code;
                p_ftms_env->candence = CO_COMBINE_UINT16_LE(p_buf);
            }
        }
        break;

        default:
        {
            // Operation Code is invalid, status is already CTRL_PT_RESP_NOT_SUPP
        } break;
    }

    const ble_ftms_cb_t *p_cb  = (const ble_ftms_cb_t *) p_ftms_env->p_cb;

    // inform application about control point request
    p_cb->cb_ctrl_pt_req(conidx, op_code, &value);

    uint8_t *p_out_buf = NULL;

    if ((p_out_buf = bes_bt_me_bes_bt_buf_malloc(FTMS_CTRL_PT_RSP_MIN_LEN + sizeof(ble_ftms_buf_meta_t))))
    {
        ble_ftms_buf_meta_t *p_meta = (ble_ftms_buf_meta_t *)(p_out_buf);
        p_meta->buf_len = FTMS_CTRL_PT_RSP_MIN_LEN;
        p_out_buf = p_meta->buf;

        p_ftms_env->ctrl_pt_op    = CTRL_PT_RSP_CODE;
        *p_out_buf++ = CTRL_PT_RSP_CODE;
        *p_out_buf++ = op_code;
        *p_out_buf++ = ctrl_pt_rsp_status;

        p_meta->conidx    = conidx;
        p_meta->operation = FTMS_IND_CTRL_PT_OP_CODE;

        // put event on wait queue
        colist_addto_tail(&(p_meta->node), &(p_ftms_env->wait_queue));
        // execute operation
        ble_ftms_exe_operation();
    }
    else
    {
        status = ATT_ERROR_INSUFF_RESOURCES;
    }

    return status;
}

static uint8_t ble_ftms_pack_threadmill_data(uint8_t *p_buf, const ftms_threadmill_data_t *p_data)
{
    uint8_t flags = p_data->flags;
    uint8_t *p_buf_head = p_buf;

    // reseve 1 byte for flags
    p_buf += 1;

    // Check provided flags
    if (GETB(flags, FTMS_TD_MORE_DATA_PRESENT))
    {
        // Pack Instantaneous Speed
        *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->inst_speed);
        p_buf += 2;
    }

    if (GETB(flags, FTMS_TD_AVERAGE_SPEED_REFERENCE))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_AVERAGE_SPEED_SUP))
        {
            // Pack Average Speed
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->aver_speed);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_TD_AVERAGE_SPEED_REFERENCE, 0);
        }
    }

    if (GETB(flags, FTMS_TD_TOTAL_DISTANCE_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_TOTAL_DISTANCE_SUP))
        {
            // Pack Total Distance
            *(uint32_t *)p_buf = co_host_to_uint32_le(p_data->total_distance);
            p_buf += 4;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_TD_TOTAL_DISTANCE_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_TD_INCLINATION_RAMP_ANGLE_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_INCLINATION_SUP))
        {
            // Pack Inclination and Ramp Angle Setting
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->inclination);
            p_buf += 2;
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->ramp_angle_setting);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_TD_INCLINATION_RAMP_ANGLE_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_TD_ELEVATION_GAIN_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_ELEVATION_GAIN_SUP))
        {
            // Pack Positive Elevation Gain  Negative Elevation Gain
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->posi_eleva_gain);
            p_buf += 2;
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->nega_eleva_gain);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_TD_ELEVATION_GAIN_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_TD_INSTANTANEOUS_PACE_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_PACE_SUP))
        {
            // Pack Instantaneous Pace
            *(uint8_t *)p_buf = p_data->instantaneous_pace;
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_TD_INSTANTANEOUS_PACE_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_TD_AVERAGE_PACE_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_PACE_SUP))
        {
            // Pack Average Pace
            *(uint8_t *)p_buf = p_data->average_pace;
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_TD_AVERAGE_PACE_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_TD_EXPENDED_ENERGY_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_EXP_ENERGY_SUP))
        {
            // Pack Total Energy/Energy Per Hour/Energy Per Minute
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->total_energy);
            p_buf += 2;
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->energy_per_hour);
            p_buf += 2;
            *(uint8_t *)p_buf = p_data->energy_per_min;
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_TD_EXPENDED_ENERGY_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_TD_HEART_RATE_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_HEART_RATE_SUP))
        {
            // Pack Heart Rate
            *(uint8_t *)p_buf = p_data->heart_rate;
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_TD_HEART_RATE_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_TD_METABOLIC_EQUIVALENT_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_METABOLIC_EQUIVALENT_SUP))
        {
            // Pack Metabolic Equivalent
            *(uint8_t *)p_buf = p_data->meta_equi;
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_TD_METABOLIC_EQUIVALENT_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_TD_ELAPSED_TIME_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_ELAPSED_TIME_SUP))
        {
            // Pack Elapsed Time
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->elapsed_time);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_TD_ELAPSED_TIME_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_TD_REMAINING_TIME_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_REMAINING_TIME_SUP))
        {
            // Pack Remaining Time
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->remaining_time);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_TD_REMAINING_TIME_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_TD_BELT_POWER_OUT_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_BELT_POWER_OUT_SUP))
        {
            // Pack Force on Belt/Power Output
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->belt);
            p_buf += 2;
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->power_output);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_TD_BELT_POWER_OUT_PRESENT, 0);
        }
    }

    // Flags value
    p_buf_head[0] = p_data->flags;

    return p_buf - p_buf_head;
}

static uint8_t ble_ftms_pack_cross_trainer_data(uint8_t *p_buf, const ftms_cross_trainer_data_t *p_data)
{
    uint8_t flags = p_data->flags;
    uint8_t *p_buf_head = p_buf;

    // reseve 1 byte for flags
    p_buf += 1;

    // Check provided flags
    if (GETB(flags, FTMS_CTD_MORE_DATA_PRESENT))
    {
        // Pack Instantaneous Speed
        *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->inst_speed);
        p_buf += 2;
    }

    if (GETB(flags, FTMS_CTD_AVERAGE_SPEED_REFERENCE))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_AVERAGE_SPEED_SUP))
        {
            // Pack Average Speed
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->aver_speed);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_CTD_AVERAGE_SPEED_REFERENCE, 0);
        }
    }

    if (GETB(flags, FTMS_CTD_TOTAL_DISTANCE_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_TOTAL_DISTANCE_SUP))
        {
            // Pack Total Distance
            *(uint32_t *)p_buf = co_host_to_uint32_le(p_data->total_distance);
            p_buf += 4;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_CTD_TOTAL_DISTANCE_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_CTD_STEP_COUNT_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_STEP_COUNT_SUP))
        {
            // Pack Step Per Minute/Average Step Rate
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->step_per_min);
            p_buf += 2;
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->ave_step_rate);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_CTD_STEP_COUNT_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_CTD_STRIDE_COUNT_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_STRIDE_COUNT_SUP))
        {
            // Pack Stride Count
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->stride_count);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_CTD_STRIDE_COUNT_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_CTD_ELEVATION_GAIN_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_ELEVATION_GAIN_SUP))
        {
            // Pack Positive Elevation Gain  Negative Elevation Gain
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->posi_eleva_gain);
            p_buf += 2;
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->nega_eleva_gain);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_CTD_ELEVATION_GAIN_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_CTD_INCLINATION_RAMP_ANGLE_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_INCLINATION_SUP))
        {
            // Pack Inclination/Ramp Angle Setting
            *(int16_t *)p_buf = co_host_to_uint16_le(p_data->inclination);
            p_buf += 2;
            *(int16_t *)p_buf = co_host_to_uint16_le(p_data->ramp_angle_setting);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_CTD_INCLINATION_RAMP_ANGLE_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_CTD_RESISTANCE_LEVEL_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_RESISTANCE_LEVEL_SUP))
        {
            // Pack Resistance Level
            *(int16_t *)p_buf = co_host_to_uint16_le(p_data->resistance_level);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_CTD_RESISTANCE_LEVEL_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_CTD_INSTANTANEOUS_POWER_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_POWER_MEASUREMENT_SUP))
        {
            // Pack Total Instantaneous Power
            *(int16_t *)p_buf = co_host_to_uint16_le(p_data->instant_power);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_CTD_INSTANTANEOUS_POWER_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_CTD_AVERAGE_POWER_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_POWER_MEASUREMENT_SUP))
        {
            // Pack Average Power
            *(int16_t *)p_buf = co_host_to_uint16_le(p_data->average_power);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_CTD_AVERAGE_POWER_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_CTD_EXPENDED_ENERGY_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_EXP_ENERGY_SUP))
        {
            // Pack Total Energy/Energy Per Hour/Energy Per Minute
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->total_energy);
            p_buf += 2;
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->energy_per_hour);
            p_buf += 2;
            *(uint8_t *)p_buf = p_data->energy_per_min;
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_CTD_EXPENDED_ENERGY_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_CTD_HEART_RATE_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_HEART_RATE_SUP))
        {
            // Pack Heart Rate
            *(uint8_t *)p_buf = p_data->heart_rate;
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_CTD_HEART_RATE_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_CTD_METABOLIC_EQUIVALENT_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_METABOLIC_EQUIVALENT_SUP))
        {
            // Pack Metabolic Equivalent
            *(uint8_t *)p_buf = p_data->meta_equi;
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_CTD_METABOLIC_EQUIVALENT_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_CTD_ELAPSED_TIME_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_ELAPSED_TIME_SUP))
        {
            // Pack Elapsed Time
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->elapsed_time);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_CTD_ELAPSED_TIME_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_CTD_REMAINING_TIME_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_REMAINING_TIME_SUP))
        {
            // Pack Remaining Time
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->remaining_time);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_CTD_REMAINING_TIME_PRESENT, 0);
        }
    }

    // Flags value
    p_buf_head[0] = p_data->flags;

    return p_buf - p_buf_head;
}

static uint8_t ble_ftms_pack_step_climber_data(uint8_t *p_buf, const ftms_step_climber_data_t *p_data)
{
    uint8_t flags = p_data->flags;
    uint8_t *p_buf_head = p_buf;

    // reseve 1 byte for flags
    p_buf += 1;

    // Check provided flags
    if (GETB(flags, FTMS_STD_MORE_DATA_PRESENT))
    {
        // Pack Floors and Step Count
        *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->floors);
        p_buf += 2;
        *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->step_count);
        p_buf += 2;
    }

    if (GETB(flags, FTMS_STD_STEP_PER_MINUTE_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_STEP_COUNT_SUP))
        {
            // Pack Step per Minute
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->step_per_min);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_STD_STEP_PER_MINUTE_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_STD_AVERAGE_STEP_RATE_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_STEP_COUNT_SUP))
        {
            // Pack Average Step Rate
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->ave_step_rate);
            p_buf += 4;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_STD_AVERAGE_STEP_RATE_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_STD_POSITIVE_ELEVATION_GAIN_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_ELEVATION_GAIN_SUP))
        {
            // Pack Positive Elevation Gain
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->posi_eleva_gain);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_STD_POSITIVE_ELEVATION_GAIN_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_STD_EXPENDED_ENERGY_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_EXP_ENERGY_SUP))
        {
            // Pack Total Energy/Energy Per Hour/Energy Per Minute
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->total_energy);
            p_buf += 2;
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->energy_per_hour);
            p_buf += 2;
            *(uint8_t *)p_buf = p_data->energy_per_min;
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_STD_EXPENDED_ENERGY_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_STD_HEART_RATE_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_HEART_RATE_SUP))
        {
            // Pack Heart Rate
            *(uint8_t *)p_buf = p_data->heart_rate;
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_STD_HEART_RATE_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_STD_METABOLIC_EQUIVALENT_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_METABOLIC_EQUIVALENT_SUP))
        {
            // Pack Metabolic Equivalent
            *(uint8_t *)p_buf = p_data->meta_equi;
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_STD_METABOLIC_EQUIVALENT_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_STD_ELAPSED_TIME_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_ELAPSED_TIME_SUP))
        {
            // Pack Elapsed Time
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->elapsed_time);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_STD_ELAPSED_TIME_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_STD_REMAINING_TIME_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_REMAINING_TIME_SUP))
        {
            // Pack Remaining Time
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->remaining_time);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_STD_REMAINING_TIME_PRESENT, 0);
        }
    }

    // Flags value
    p_buf_head[0] = p_data->flags;

    return p_buf - p_buf_head;
}

static uint8_t ble_ftms_pack_stair_climber_data(uint8_t *p_buf, const ftms_stair_climber_data_t *p_data)
{
    uint8_t flags = p_data->flags;
    uint8_t *p_buf_head = p_buf;

    // reseve 1 byte for flags
    p_buf += 1;

    // Check provided flags
    if (GETB(flags, FTMS_SCD_MORE_DATA_PRESENT))
    {
        // Pack Floors
        *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->floors);
        p_buf += 2;
    }

    if (GETB(flags, FTMS_SCD_STEP_PER_MINUTE_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_STEP_COUNT_SUP))
        {
            // Pack Step per Minute
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->step_per_min);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_SCD_STEP_PER_MINUTE_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_SCD_AVERAGE_STEP_RATE_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_STEP_COUNT_SUP))
        {
            // Pack Average Step Rate
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->ave_step_rate);
            p_buf += 4;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_SCD_AVERAGE_STEP_RATE_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_SCD_POSITIVE_ELEVATION_GAIN_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_ELEVATION_GAIN_SUP))
        {
            // Pack Positive Elevation Gain
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->posi_eleva_gain);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_SCD_POSITIVE_ELEVATION_GAIN_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_SCD_STRIDE_COUNT_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_STRIDE_COUNT_SUP))
        {
            // Pack Stride Count
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->stride_count);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_SCD_STRIDE_COUNT_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_SCD_EXPENDED_ENERGY_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_EXP_ENERGY_SUP))
        {
            // Pack Total Energy/Energy Per Hour/Energy Per Minute
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->total_energy);
            p_buf += 2;
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->energy_per_hour);
            p_buf += 2;
            *(uint8_t *)p_buf = p_data->energy_per_min;
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_SCD_EXPENDED_ENERGY_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_SCD_HEART_RATE_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_HEART_RATE_SUP))
        {
            // Pack Heart Rate
            *(uint8_t *)p_buf = p_data->heart_rate;
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_SCD_HEART_RATE_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_SCD_METABOLIC_EQUIVALENT_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_METABOLIC_EQUIVALENT_SUP))
        {
            // Pack Metabolic Equivalent
            *(uint8_t *)p_buf = p_data->meta_equi;
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_SCD_METABOLIC_EQUIVALENT_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_SCD_ELAPSED_TIME_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_ELAPSED_TIME_SUP))
        {
            // Pack Elapsed Time
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->elapsed_time);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_SCD_ELAPSED_TIME_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_SCD_REMAINING_TIME_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_REMAINING_TIME_SUP))
        {
            // Pack Remaining Time
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->remaining_time);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_SCD_REMAINING_TIME_PRESENT, 0);
        }
    }

    // Flags value
    p_buf_head[0] = p_data->flags;

    return p_buf - p_buf_head;
}

static uint8_t ble_ftms_pack_rower_data(uint8_t *p_buf, const ftms_rower_data_t *p_data)
{
    uint8_t flags = p_data->flags;
    uint8_t *p_buf_head = p_buf;

    // reseve 1 byte for flags
    p_buf += 1;

    // Check provided flags
    if (GETB(flags, FTMS_RD_MORE_DATA_PRESENT))
    {
        // Pack Stroke Rate and Stroke Count
        *(uint8_t *)p_buf = p_data->stroke_rate;
        p_buf += 1;
        *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->stroke_count);
        p_buf += 2;
    }

    if (GETB(flags, FTMS_RD_AVERAGE_STROKE_RATE_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_CADENCE_SUP))
        {
            // Pack Step per Minute
            *(uint8_t *)p_buf = p_data->average_stroke_rate;
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_RD_AVERAGE_STROKE_RATE_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_RD_TOTAL_DISTANCE_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_TOTAL_DISTANCE_SUP))
        {
            // Pack Total Distance
            *(uint32_t *)p_buf = co_host_to_uint32_le(p_data->total_distance);
            p_buf += 4;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_RD_TOTAL_DISTANCE_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_RD_INSTANTANEOUS_PACE_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_PACE_SUP))
        {
            // Pack Instantaneous Pace
            *(uint8_t *)p_buf = p_data->instantaneous_pace;
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_RD_INSTANTANEOUS_PACE_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_RD_AVERAGE_PACE_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_PACE_SUP))
        {
            // Pack Average Pace
            *(uint8_t *)p_buf = p_data->average_pace;
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_RD_AVERAGE_PACE_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_RD_INSTANTANEOUS_POWER_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_POWER_MEASUREMENT_SUP))
        {
            // Pack Total Instantaneous Power
            *(int16_t *)p_buf = co_host_to_uint16_le(p_data->instant_power);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_RD_INSTANTANEOUS_POWER_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_RD_AVERAGE_POWER_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_POWER_MEASUREMENT_SUP))
        {
            // Pack Average Power
            *(int16_t *)p_buf = co_host_to_uint16_le(p_data->average_power);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_RD_AVERAGE_POWER_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_RD_RESISTANCE_LEVEL_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_RESISTANCE_LEVEL_SUP))
        {
            // Pack Resistance Level
            *(int16_t *)p_buf = co_host_to_uint16_le(p_data->resistance_level);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_RD_RESISTANCE_LEVEL_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_RD_EXPENDED_ENERGY_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_EXP_ENERGY_SUP))
        {
            // Pack Total Energy/Energy Per Hour/Energy Per Minute
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->total_energy);
            p_buf += 2;
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->energy_per_hour);
            p_buf += 2;
            *(uint8_t *)p_buf = p_data->energy_per_min;
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_RD_EXPENDED_ENERGY_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_RD_HEART_RATE_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_HEART_RATE_SUP))
        {
            // Pack Heart Rate
            *(uint8_t *)p_buf = p_data->heart_rate;
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_RD_HEART_RATE_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_SCD_METABOLIC_EQUIVALENT_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_METABOLIC_EQUIVALENT_SUP))
        {
            // Pack Metabolic Equivalent
            *(uint8_t *)p_buf = p_data->meta_equi;
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_SCD_METABOLIC_EQUIVALENT_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_SCD_ELAPSED_TIME_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_ELAPSED_TIME_SUP))
        {
            // Pack Elapsed Time
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->elapsed_time);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_SCD_ELAPSED_TIME_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_SCD_REMAINING_TIME_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_REMAINING_TIME_SUP))
        {
            // Pack Remaining Time
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->remaining_time);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_SCD_REMAINING_TIME_PRESENT, 0);
        }
    }

    // Flags value
    p_buf_head[0] = p_data->flags;

    return p_buf - p_buf_head;
}

static uint8_t ble_ftms_pack_indoor_bike_data(uint8_t *p_buf, const ftms_indoor_bike_data_t *p_data)
{
    uint8_t flags = p_data->flags;
    uint8_t *p_buf_head = p_buf;

    // reseve 1 byte for flags
    p_buf += 1;

    // Check provided flags
    if (GETB(flags, FTMS_IBD_MORE_DATA_PRESENT))
    {
        // Pack Instantaneous Speed
        *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->inst_speed);
        p_buf += 2;
    }

    if (GETB(flags, FTMS_IBD_AVERAGE_SPEED_REFERENCE))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_AVERAGE_SPEED_SUP))
        {
            // Pack Average Speed
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->aver_speed);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_IBD_AVERAGE_SPEED_REFERENCE, 0);
        }
    }

    if (GETB(flags, FTMS_IBD_INSTANTNEOUS_CADENCE_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_CADENCE_SUP))
        {
            // Pack Instantaneous Cadence
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->inst_cadence);
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_IBD_INSTANTNEOUS_CADENCE_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_IBD_AVERAGE_CADENCE_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_CADENCE_SUP))
        {
            // Pack Average Cadence
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->aver_cadence);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_IBD_AVERAGE_CADENCE_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_IBD_TOTAL_DISTANCE_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_TOTAL_DISTANCE_SUP))
        {
            // Pack Total Distance
            *(uint32_t *)p_buf = co_host_to_uint32_le(p_data->total_distance);
            p_buf += 4;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_IBD_TOTAL_DISTANCE_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_IBD_RESISTANCE_LEVEL_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_RESISTANCE_LEVEL_SUP))
        {
            // Pack Resistance Level
            *(int16_t *)p_buf = co_host_to_uint16_le(p_data->resistance_level);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_IBD_RESISTANCE_LEVEL_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_IBD_INSTANTANEOUS_POWER_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_POWER_MEASUREMENT_SUP))
        {
            // Pack Total Instantaneous Power
            *(int16_t *)p_buf = co_host_to_uint16_le(p_data->instant_power);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_IBD_INSTANTANEOUS_POWER_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_IBD_AVERAGE_POWER_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_POWER_MEASUREMENT_SUP))
        {
            // Pack Average Power
            *(int16_t *)p_buf = co_host_to_uint16_le(p_data->average_power);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_IBD_AVERAGE_POWER_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_IBD_EXPENDED_ENERGY_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_EXP_ENERGY_SUP))
        {
            // Pack Total Energy/Energy Per Hour/Energy Per Minute
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->total_energy);
            p_buf += 2;
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->energy_per_hour);
            p_buf += 2;
            *(uint8_t *)p_buf = p_data->energy_per_min;
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_IBD_EXPENDED_ENERGY_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_IBD_HEART_RATE_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_HEART_RATE_SUP))
        {
            // Pack Heart Rate
            *(uint8_t *)p_buf = p_data->heart_rate;
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_IBD_HEART_RATE_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_IBD_METABOLIC_EQUIVALENT_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_METABOLIC_EQUIVALENT_SUP))
        {
            // Pack Metabolic Equivalent
            *(uint8_t *)p_buf = p_data->meta_equi;
            p_buf += 1;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_IBD_METABOLIC_EQUIVALENT_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_IBD_ELAPSED_TIME_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_ELAPSED_TIME_SUP))
        {
            // Pack Elapsed Time
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->elapsed_time);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_IBD_ELAPSED_TIME_PRESENT, 0);
        }
    }

    if (GETB(flags, FTMS_IBD_REMAINING_TIME_PRESENT))
    {
        if (GETB(p_ftms_env->ftms_feature, FTMS_FEAT_REMAINING_TIME_SUP))
        {
            // Pack Remaining Time
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_data->remaining_time);
            p_buf += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(flags, FTMS_IBD_REMAINING_TIME_PRESENT, 0);
        }
    }

    // Flags value
    p_buf_head[0] = p_data->flags;

    return p_buf - p_buf_head;
}

uint16_t ble_ftms_trainer_data_send(uint32_t conidx_bf, uint8_t ntf_opcode, const uint8_t *p_data)
{
    uint16_t status = BT_STS_NOT_ALLOW;
    uint8_t max_buf_len = 0;

    if (p_data == NULL)
    {
        status = BT_STS_INVALID_PARM;
    }

    if (p_ftms_env == NULL)
    {
        return BT_STS_FAILED;
    }

    switch (ntf_opcode)
    {
        case FTMS_NTF_TREADMILL_DATA_OP_CODE:
        {
            if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_CROSS_TRAINER_DATA_CHAR_SUPP_FLAG))
            {
                max_buf_len = FTMS_THREADMILL_DATA_MAX_LEN;
            }
        }
        break;

        case FTMS_NTF_CROSS_TRAINER_DATA_OP_CODE:
        {
            if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_CROSS_TRAINER_DATA_CHAR_SUPP_FLAG))
            {
                max_buf_len = FTMS_CROSS_TRAINER_DATA_MAX_LEN;
            }
        }
        break;

        case FTMS_NTF_STEP_CLIMBER_DATA_OP_CODE:
        {
            if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_STEP_CLIMBER_DATA_CHAR_SUPP_FLAG))
            {
                max_buf_len = FTMS_STEP_CLIMBER_DATA_MAX_LEN;
            }
        }
        break;

        case FTMS_NTF_STAIR_CLIMBER_DATA_OP_CODE:
        {
            if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_STAIR_CLIMBER_DATA_CHAR_SUPP_FLAG))
            {
                max_buf_len = FTMS_STAIR_CLIMBER_DATA_MAX_LEN;
            }
        }
        break;

        case FTMS_NTF_ROWER_DATA_OP_CODE:
        {
            if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_ROWER_DATA_CHAR_SUPP_FLAG))
            {
                max_buf_len = FTMS_ROWER_DATA_MAX_LEN;
            }
        }
        break;

        case FTMS_NTF_INDOOR_BIKE_DATA_OP_CODE:
        {
            if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_INDOOR_BIKE_DATA_CHAR_SUPP_FLAG))
            {
                max_buf_len = FTMS_INDOOR_BIKE_DATA_MAX_LEN;
            }
        }
        break;

        default:
            break;
    }
    
    if(!max_buf_len)
    {
        status = BT_STS_NOT_SUPPORTED;
    }
    else
    {
        uint8_t *p_buf = NULL;

        if ((p_buf = bes_bt_me_bes_bt_buf_malloc(max_buf_len)))
        {
            ble_ftms_buf_meta_t *p_buf_meta = (ble_ftms_buf_meta_t *)p_buf;
            p_buf_meta->operation = ntf_opcode;
            p_buf_meta->conidx    = GAP_INVALID_CONIDX;
            p_buf_meta->conidx_bf = conidx_bf & ((1 << BLE_CONNECTION_MAX) - 1);
            p_buf_meta->new       = true;
            p_buf_meta->buf_len   = max_buf_len;

            // Pack structure
            if (ntf_opcode == FTMS_NTF_TREADMILL_DATA_OP_CODE)
            {
                p_buf_meta->buf_len = ble_ftms_pack_threadmill_data(p_buf_meta->buf, (ftms_threadmill_data_t *)p_data);
            }
            else if (ntf_opcode == FTMS_NTF_CROSS_TRAINER_DATA_OP_CODE)
            {
                p_buf_meta->buf_len = ble_ftms_pack_cross_trainer_data(p_buf_meta->buf, (ftms_cross_trainer_data_t *)p_data);
            }
            else if (ntf_opcode == FTMS_NTF_STEP_CLIMBER_DATA_OP_CODE)
            {
                p_buf_meta->buf_len = ble_ftms_pack_step_climber_data(p_buf_meta->buf, (ftms_step_climber_data_t *)p_data);
            }
            else if (ntf_opcode == FTMS_NTF_STAIR_CLIMBER_DATA_OP_CODE)
            {
                p_buf_meta->buf_len = ble_ftms_pack_stair_climber_data(p_buf_meta->buf, (ftms_stair_climber_data_t *)p_data);
            }
            else if (ntf_opcode == FTMS_NTF_ROWER_DATA_OP_CODE)
            {
                p_buf_meta->buf_len = ble_ftms_pack_rower_data(p_buf_meta->buf, (ftms_rower_data_t *)p_data);
            }
            else if (ntf_opcode == FTMS_NTF_INDOOR_BIKE_DATA_OP_CODE)
            {
                p_buf_meta->buf_len = ble_ftms_pack_indoor_bike_data(p_buf_meta->buf, (ftms_indoor_bike_data_t *)p_data);
            }
            // put event on wait queue
            colist_addto_tail(&(p_buf_meta->node), &(p_ftms_env->wait_queue));
            // execute operation
            ble_ftms_exe_operation();

            status = BT_STS_SUCCESS;
        }
        else
        {
            status = BT_STS_NO_RESOURCES;
        }
    }

    return status;
}

static uint8_t ble_ftms_pack_ctrl_point_rsp(uint8_t conidx, uint8_t *p_buf, uint8_t op_code, uint8_t resp_val,
                                           const ble_ftm_ctrl_pt_rsp_val *p_value)
{
    uint8_t *p_buf_head = p_buf;
    // Set the Response Code
    p_buf[0] = CTRL_PT_RSP_CODE;
    p_buf += 1;

    // Set the request operation code
    p_buf[0] = p_ftms_env->ctrl_pt_op;
    p_buf += 1;

    p_buf[0] = resp_val;
    p_buf += 1;
    
    if(p_ftms_env->ctrl_pt_op == CTRL_PT_SET_SPINDOWN_CONTRL)
    {
        *(uint16_t *)p_buf = co_host_to_uint16_le(p_value->target_speed_low);
        p_buf += 2;
        *(uint16_t *)p_buf = co_host_to_uint16_le(p_value->target_speed_high);
        p_buf += 2;
    }

    return p_buf - p_buf_head;
}

static void ble_ftms_exe_operation(void)
{
    if (bt_defer_curr_func_0(ble_ftms_exe_operation))
    {
        return;
    }

    if (!p_ftms_env->in_exe_op)
    {
        p_ftms_env->in_exe_op = true;

        while (!colist_is_list_empty(&(p_ftms_env->wait_queue)) && !(p_ftms_env->op_ongoing))
        {
            uint16_t status = BT_STS_SUCCESS;
            ble_ftms_buf_meta_t *p_meta = (ble_ftms_buf_meta_t *)colist_get_head(&(p_ftms_env->wait_queue));

            switch (p_meta->operation)
            {
                case FTMS_NTF_TREADMILL_DATA_OP_CODE:
                {
                    uint8_t  conidx;
                    uint32_t conidx_bf = 0;

                    // remove buffer from queue
                    colist_delete(&(p_meta->node));

                    // check connection that support notification reception
                    for (conidx = 0 ; conidx < BLE_CONNECTION_MAX ; conidx++)
                    {
                        if (GETB(p_ftms_env->env[conidx].prfl_ntf_ind_cfg, FTMS_PRF_CFG_FLAG_TREADMILL_DATA_NTF))
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
                            .character = g_ble_treadmill_data_character,
                            .service = g_ble_ftm_service,
                        };
                        // send multipoint notification
                        status = gatts_send_value_notification(FTMS_2_GAP_CONIDX_BF(conidx_bf), &ntf_cfg, p_meta->buf, p_meta->buf_len);
                        if (status == BT_STS_SUCCESS)
                        {
                            p_ftms_env->op_ongoing = true;
                        }
                    }

                    bes_bt_me_bes_bt_buf_free((uint8_t *)p_meta);

                    if (!p_ftms_env->op_ongoing)
                    {
                        const ble_ftms_cb_t *p_cb = (const ble_ftms_cb_t *) p_ftms_env->p_cb;
                        // Inform application that event has been sent
                        p_cb->cb_ntf_send_cmp(status);
                    }
                }
                break;

                case FTMS_NTF_CROSS_TRAINER_DATA_OP_CODE:
                {
                    uint8_t  conidx;
                    uint32_t conidx_bf = 0;

                    // remove buffer from queue
                    colist_delete(&(p_meta->node));

                    // check connection that support notification reception
                    for (conidx = 0 ; conidx < BLE_CONNECTION_MAX ; conidx++)
                    {
                        if (GETB(p_ftms_env->env[conidx].prfl_ntf_ind_cfg, FTMS_PRF_CFG_FLAG_CROSS_TRAINER_DATA_NTF))
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
                            .character = g_ble_cross_trainer_data_character,
                            .service = g_ble_ftm_service,
                        };
                        // send multipoint notification
                        status = gatts_send_value_notification(FTMS_2_GAP_CONIDX_BF(conidx_bf), &ntf_cfg, p_meta->buf, p_meta->buf_len);
                        if (status == BT_STS_SUCCESS)
                        {
                            p_ftms_env->op_ongoing = true;
                        }
                    }

                    bes_bt_me_bes_bt_buf_free((uint8_t *)p_meta);

                    if (!p_ftms_env->op_ongoing)
                    {
                        const ble_ftms_cb_t *p_cb = (const ble_ftms_cb_t *) p_ftms_env->p_cb;
                        // Inform application that event has been sent
                        p_cb->cb_ntf_send_cmp(status);
                    }
                }
                break;

                case FTMS_NTF_STEP_CLIMBER_DATA_OP_CODE:
                {
                    uint8_t  conidx;
                    uint32_t conidx_bf = 0;

                    // remove buffer from queue
                    colist_delete(&(p_meta->node));

                    // check connection that support notification reception
                    for (conidx = 0 ; conidx < BLE_CONNECTION_MAX ; conidx++)
                    {
                        if (GETB(p_ftms_env->env[conidx].prfl_ntf_ind_cfg, FTMS_PRF_CFG_FLAG_STEP_CLIMBER_DATA_NTF))
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
                            .character = g_ble_step_climber_data_character,
                            .service = g_ble_ftm_service,
                        };
                        // send multipoint notification
                        status = gatts_send_value_notification(FTMS_2_GAP_CONIDX_BF(conidx_bf), &ntf_cfg, p_meta->buf, p_meta->buf_len);
                        if (status == BT_STS_SUCCESS)
                        {
                            p_ftms_env->op_ongoing = true;
                        }
                    }

                    bes_bt_me_bes_bt_buf_free((uint8_t *)p_meta);

                    if (!p_ftms_env->op_ongoing)
                    {
                        const ble_ftms_cb_t *p_cb = (const ble_ftms_cb_t *) p_ftms_env->p_cb;
                        // Inform application that event has been sent
                        p_cb->cb_ntf_send_cmp(status);
                    }
                }
                break;

                case FTMS_NTF_STAIR_CLIMBER_DATA_OP_CODE:
                {
                    uint8_t  conidx;
                    uint32_t conidx_bf = 0;

                    // remove buffer from queue
                    colist_delete(&(p_meta->node));

                    // check connection that support notification reception
                    for (conidx = 0 ; conidx < BLE_CONNECTION_MAX ; conidx++)
                    {
                        if (GETB(p_ftms_env->env[conidx].prfl_ntf_ind_cfg, FTMS_PRF_CFG_FLAG_STAIR_CLIMBER_DATA_NTF))
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
                            .character = g_ble_stair_climber_data_character,
                            .service = g_ble_ftm_service,
                        };
                        // send multipoint notification
                        status = gatts_send_value_notification(FTMS_2_GAP_CONIDX_BF(conidx_bf), &ntf_cfg, p_meta->buf, p_meta->buf_len);
                        if (status == BT_STS_SUCCESS)
                        {
                            p_ftms_env->op_ongoing = true;
                        }
                    }

                    bes_bt_me_bes_bt_buf_free((uint8_t *)p_meta);

                    if (!p_ftms_env->op_ongoing)
                    {
                        const ble_ftms_cb_t *p_cb = (const ble_ftms_cb_t *) p_ftms_env->p_cb;
                        // Inform application that event has been sent
                        p_cb->cb_ntf_send_cmp(status);
                    }
                }
                break;

                case FTMS_NTF_ROWER_DATA_OP_CODE:
                {
                    uint8_t  conidx;
                    uint32_t conidx_bf = 0;

                    // remove buffer from queue
                    colist_delete(&(p_meta->node));

                    // check connection that support notification reception
                    for (conidx = 0 ; conidx < BLE_CONNECTION_MAX ; conidx++)
                    {
                        if (GETB(p_ftms_env->env[conidx].prfl_ntf_ind_cfg, FTMS_PRF_CFG_FLAG_ROWER_DATA_NTF))
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
                            .character = g_ble_rower_data_character,
                            .service = g_ble_ftm_service,
                        };
                        // send multipoint notification
                        status = gatts_send_value_notification(FTMS_2_GAP_CONIDX_BF(conidx_bf), &ntf_cfg, p_meta->buf, p_meta->buf_len);
                        if (status == BT_STS_SUCCESS)
                        {
                            p_ftms_env->op_ongoing = true;
                        }
                    }

                    bes_bt_me_bes_bt_buf_free((uint8_t *)p_meta);

                    if (!p_ftms_env->op_ongoing)
                    {
                        const ble_ftms_cb_t *p_cb = (const ble_ftms_cb_t *) p_ftms_env->p_cb;
                        // Inform application that event has been sent
                        p_cb->cb_ntf_send_cmp(status);
                    }
                }
                break;

                case FTMS_NTF_INDOOR_BIKE_DATA_OP_CODE:
                {
                    uint8_t  conidx;
                    uint32_t conidx_bf = 0;

                    // remove buffer from queue
                    colist_delete(&(p_meta->node));

                    // check connection that support notification reception
                    for (conidx = 0 ; conidx < BLE_CONNECTION_MAX ; conidx++)
                    {
                        if (GETB(p_ftms_env->env[conidx].prfl_ntf_ind_cfg, FTMS_PRF_CFG_FLAG_INDOOR_BIKE_DATA_NTF))
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
                            .character = g_ble_indoor_bike_data_character,
                            .service = g_ble_ftm_service,
                        };
                        // send multipoint notification
                        status = gatts_send_value_notification(FTMS_2_GAP_CONIDX_BF(conidx_bf), &ntf_cfg, p_meta->buf, p_meta->buf_len);
                        if (status == BT_STS_SUCCESS)
                        {
                            p_ftms_env->op_ongoing = true;
                        }
                    }

                    bes_bt_me_bes_bt_buf_free((uint8_t *)p_meta);

                    if (!p_ftms_env->op_ongoing)
                    {
                        const ble_ftms_cb_t *p_cb = (const ble_ftms_cb_t *) p_ftms_env->p_cb;
                        // Inform application that event has been sent
                        p_cb->cb_ntf_send_cmp(status);
                    }
                }
                break;

                case FTMS_NTF_TRAINING_STATUS_OP_CODE:
                {
                    uint8_t  conidx;
                    uint32_t conidx_bf = 0;

                    // remove buffer from queue
                    colist_delete(&(p_meta->node));

                    // check connection that support notification reception
                    for (conidx = 0 ; conidx < BLE_CONNECTION_MAX ; conidx++)
                    {
                        if (GETB(p_ftms_env->env[conidx].prfl_ntf_ind_cfg, FTMS_PRF_CFG_FLAG_TRAINING_STATUS_NTF))
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
                            .character = g_ble_training_status_character,
                            .service = g_ble_ftm_service,
                        };
                        // send multipoint notification
                        status = gatts_send_value_notification(FTMS_2_GAP_CONIDX_BF(conidx_bf), &ntf_cfg, p_meta->buf, p_meta->buf_len);
                        if (status == BT_STS_SUCCESS)
                        {
                            p_ftms_env->op_ongoing = true;
                        }
                    }

                    bes_bt_me_bes_bt_buf_free((uint8_t *)p_meta);

                    if (!p_ftms_env->op_ongoing)
                    {
                        const ble_ftms_cb_t *p_cb = (const ble_ftms_cb_t *) p_ftms_env->p_cb;
                        // Inform application that event has been sent
                        p_cb->cb_ntf_send_cmp(status);
                    }
                }
                break;

                case FTMS_NTF_FITNESS_MACHINE_STATUS_OP_CODE:
                {
                    uint8_t  conidx;
                    uint32_t conidx_bf = 0;

                    // remove buffer from queue
                    colist_delete(&(p_meta->node));

                    // check connection that support notification reception
                    for (conidx = 0 ; conidx < BLE_CONNECTION_MAX ; conidx++)
                    {
                        if (GETB(p_ftms_env->env[conidx].prfl_ntf_ind_cfg, FTMS_PRF_CFG_FLAG_STATUS_NTF))
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
                            .character = g_ble_ftm_status_character,
                            .service = g_ble_ftm_service,
                        };
                        // send multipoint notification
                        status = gatts_send_value_notification(FTMS_2_GAP_CONIDX_BF(conidx_bf), &ntf_cfg, p_meta->buf, p_meta->buf_len);
                        if (status == BT_STS_SUCCESS)
                        {
                            p_ftms_env->op_ongoing = true;
                        }
                    }

                    bes_bt_me_bes_bt_buf_free((uint8_t *)p_meta);

                    if (!p_ftms_env->op_ongoing)
                    {
                        const ble_ftms_cb_t *p_cb = (const ble_ftms_cb_t *) p_ftms_env->p_cb;
                        // Inform application that event has been sent
                        p_cb->cb_ntf_send_cmp(status);
                    }
                }
                break;

                case FTMS_IND_CTRL_PT_OP_CODE:
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
                            if (GETB(p_ftms_env->env[conidx].prfl_ntf_ind_cfg, FTMS_PRF_CFG_FLAG_CTRL_POINT_IND))
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
                            .character = g_ble_ftm_control_point_character,
                            .service = g_ble_ftm_service,
                        };
                        // send multipoint notification
                        status = gatts_send_value_indication(FTMS_2_GAP_CONIDX_BF(p_meta->conidx), &ntf_cfg, p_meta->buf, p_meta->buf_len);
                        if (status == BT_STS_SUCCESS)
                        {
                            p_ftms_env->op_ongoing = true;
                        }
                    }
                    else
                    {
                        const ble_ftms_cb_t *p_cb = (const ble_ftms_cb_t *) p_ftms_env->p_cb;
                        // Inform application that event has been sent
                        p_cb->cb_ntf_send_cmp(BT_STS_SUCCESS);

                        // remove buffer from queue
                        colist_delete(&(p_meta->node));

                        bes_bt_me_bes_bt_buf_free((uint8_t *)p_meta);
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
                        .character = g_ble_ftm_control_point_character,
                        .service = g_ble_ftm_feature_character,
                    };
                    status = gatts_send_value_indication(FTMS_2_GAP_CONIDX_BF(conidx), &ntf_cfg, p_meta->buf, p_meta->buf_len);

                    if (status == BT_STS_SUCCESS)
                    {
                        p_ftms_env->op_ongoing = true;
                    }
                    else
                    {
                        // Inform application that control point response has been sent
                        if (p_ftms_env->ctrl_pt_op != CTRL_PT_RSP_CODE)
                        {
                            const ble_ftms_cb_t *p_cb = (const ble_ftms_cb_t *) p_ftms_env->p_cb;
                            p_cb->cb_ctrl_pt_rsp_send_cmp(conidx, status);
                        }

                        // consider control point operation done
                        p_ftms_env->ctrl_pt_op = CTRL_PT_RESERVED;
                    }

                    bes_bt_me_bes_bt_buf_free((uint8_t *)p_meta);
                }
                break;
            }
        }

        p_ftms_env->in_exe_op = false;
    }
}

static void ble_ftms_cb_att_read_get(uint16_t conidx, uint32_t token, const gatt_attribute_t *attr, uint16_t offset)
{
    // retrieve value attribute
    uint16_t  status      = ATT_ERROR_NO_ERROR;
    uint8_t *p_buf        = NULL;
    uint16_t  p_buf_len = 0;

    if ((p_buf = bes_bt_me_bes_bt_buf_malloc(FTMS_CP_NTF_MAX_LEN)))
    {
        switch (ble_ftms_idx_get(attr))
        {
            case FTMS_IDX_FEATURE_CHAR:
            {
                *(uint32_t *)p_buf = co_host_to_uint32_le(p_ftms_env->ftms_feature);
                p_buf += 4;
                *(uint32_t *)p_buf = co_host_to_uint32_le(p_ftms_env->target_setting_feature);
                p_buf_len += 8;
            }
            break;

            case FTMS_IDX_TREADMILL_DATA_NTF_CFG:
            {
                if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_TREADMILL_DATA_CHAR_SUPP_FLAG))
                {
                    uint16_t ntf_cfg = GETB(p_ftms_env->env[conidx].prfl_ntf_ind_cfg, FTMS_PRF_CFG_FLAG_TREADMILL_DATA_NTF)
                                    ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;

                    *(uint16_t *)p_buf = co_host_to_uint16_le(ntf_cfg);
                    p_buf_len += 2;
                }
                else
                {
                    status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
                }
            }
            break;

            case FTMS_IDX_CROSS_TRAINER_DATA_NTF_CFG:
            {
                if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_CROSS_TRAINER_DATA_CHAR_SUPP_FLAG))
                {
                    uint16_t ntf_cfg = GETB(p_ftms_env->env[conidx].prfl_ntf_ind_cfg, FTMS_PRF_CFG_FLAG_CROSS_TRAINER_DATA_NTF)
                                    ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;

                    *(uint16_t *)p_buf = co_host_to_uint16_le(ntf_cfg);
                    p_buf_len += 2;
                }
                else
                {
                    status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
                }
            }
            break;

            case FTMS_IDX_STEP_CLIMBER_DATA_NTF_CFG:
            {
                if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_STEP_CLIMBER_DATA_CHAR_SUPP_FLAG))
                {
                    uint16_t ntf_cfg = GETB(p_ftms_env->env[conidx].prfl_ntf_ind_cfg, FTMS_PRF_CFG_FLAG_STEP_CLIMBER_DATA_NTF)
                                    ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;

                    *(uint16_t *)p_buf = co_host_to_uint16_le(ntf_cfg);
                    p_buf_len += 2;
                }
                else
                {
                    status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
                }
            }
            break;

            case FTMS_IDX_STAIR_CLIMBER_DATA_NTF_CFG:
            {
                if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_STAIR_CLIMBER_DATA_CHAR_SUPP_FLAG))
                {
                    uint16_t ntf_cfg = GETB(p_ftms_env->env[conidx].prfl_ntf_ind_cfg, FTMS_PRF_CFG_FLAG_CROSS_TRAINER_DATA_NTF)
                                    ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;

                    *(uint16_t *)p_buf = co_host_to_uint16_le(ntf_cfg);
                    p_buf_len += 2;
                }
                else
                {
                    status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
                }
            }
            break;

            case FTMS_IDX_ROWER_DATA_NTF_CFG:
            {
                if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_ROWER_DATA_CHAR_SUPP_FLAG))
                {
                    uint16_t ntf_cfg = GETB(p_ftms_env->env[conidx].prfl_ntf_ind_cfg, FTMS_PRF_CFG_FLAG_CROSS_TRAINER_DATA_NTF)
                                    ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;

                    *(uint16_t *)p_buf = co_host_to_uint16_le(ntf_cfg);
                    p_buf_len += 2;
                }
                else
                {
                    status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
                }
            }
            break;

            case FTMS_IDX_INDOOR_BIKE_DATA_NTF_CFG:
            {
                if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_INDOOR_BIKE_DATA_CHAR_SUPP_FLAG))
                {
                    uint16_t ntf_cfg = GETB(p_ftms_env->env[conidx].prfl_ntf_ind_cfg, FTMS_PRF_CFG_FLAG_INDOOR_BIKE_DATA_NTF)
                                    ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;

                    *(uint16_t *)p_buf = co_host_to_uint16_le(ntf_cfg);
                    p_buf_len += 2;
                }
                else
                {
                    status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
                }
            }
            break;

            case FTMS_IDX_TRAINING_STATUS_NTF_CFG:
            {
                if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_TRAINING_STATUS_CHAR_SUPP_FLAG))
                {
                    uint16_t ntf_cfg = GETB(p_ftms_env->env[conidx].prfl_ntf_ind_cfg, FTMS_PRF_CFG_FLAG_TRAINING_STATUS_NTF)
                                    ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;

                    *(uint16_t *)p_buf = co_host_to_uint16_le(ntf_cfg);
                    p_buf_len += 2;
                }
                else
                {
                    status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
                }
            }
            break;

            case FTMS_IDX_CTRL_PT_IND_CFG:
            {
                if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_CTRL_PT_CHAR_SUPP_FLAG))
                {
                    uint16_t ind_cfg = GETB(p_ftms_env->env[conidx].prfl_ntf_ind_cfg, FTMS_PRF_CFG_FLAG_CTRL_POINT_IND)
                                    ? PRF_CLI_START_IND : PRF_CLI_STOP_NTFIND;

                    *(uint16_t *)p_buf = co_host_to_uint16_le(ind_cfg);
                    p_buf_len += 2;
                }
                else
                {
                    status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
                }
            }
            break;

            case FTMS_IDX_FITNESS_MACHINE_STATUS_NTF_CFG:
            {
                if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_CTRL_PT_CHAR_SUPP_FLAG))
                {
                    uint16_t ntf_cfg = GETB(p_ftms_env->env[conidx].prfl_ntf_ind_cfg, FTMS_PRF_CFG_FLAG_STATUS_NTF)
                                    ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;

                    *(uint16_t *)p_buf = co_host_to_uint16_le(ntf_cfg);
                    p_buf_len += 2;
                }
                else
                {
                    status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
                }
            }
            break;

            case FTMS_IDX_SUPP_SPEED_RANGE_CHAR:
            {
                if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->target_setting_feature, FTMS_TARGET_FEAT_SPEED_SUP_BIT))
                {
                    *(uint16_t *)p_buf = co_host_to_uint16_le(p_ftms_env->speed_range.min_speed);
                    p_buf_len += 2;
                    *(uint16_t *)p_buf = co_host_to_uint16_le(p_ftms_env->speed_range.max_speed);
                    p_buf_len += 2;
                    *(uint16_t *)p_buf = co_host_to_uint16_le(p_ftms_env->speed_range.min_increment);
                    p_buf_len += 2;
                }
                else
                {
                    status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
                }
            }
            break;

            case FTMS_IDX_SUPP_INCLINATION_RANGE_CHAR:
            {
                if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->target_setting_feature, FTMS_TARGET_FEAT_INCLINATION_SUP_BIT))
                {
                    *(uint16_t *)p_buf = co_host_to_uint16_le(p_ftms_env->inclination_range.min_inclination);
                    p_buf_len += 2;
                    *(uint16_t *)p_buf = co_host_to_uint16_le(p_ftms_env->inclination_range.max_inclination);
                    p_buf_len += 2;
                    *(uint16_t *)p_buf = co_host_to_uint16_le(p_ftms_env->inclination_range.min_increment);
                    p_buf_len += 2;
                }
                else
                {
                    status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
                }
            }
            break;

            case FTMS_IDX_SUPP_RESIST_LEVEL_RANGE_CHAR:
            {
                if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->target_setting_feature, FTMS_TARGET_FEAT_RESISTANCE_SUP_BIT))
                {
                    *(uint16_t *)p_buf = co_host_to_uint16_le(p_ftms_env->resis_level_range.min_inclination);
                    p_buf_len += 2;
                    *(uint16_t *)p_buf = co_host_to_uint16_le(p_ftms_env->resis_level_range.max_inclination);
                    p_buf_len += 2;
                    *(uint16_t *)p_buf = co_host_to_uint16_le(p_ftms_env->resis_level_range.min_increment);
                    p_buf_len += 2;
                }
                else
                {
                    status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
                }
            }
            break;

            case FTMS_IDX_SUPP_HEART_RATE_RANGE_CHAR:
            {
                if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->target_setting_feature, FTMS_TARGET_FEAT_HEART_RATE_SUP_BIT))
                {
                    *(uint16_t *)p_buf = co_host_to_uint16_le(p_ftms_env->heart_rate_range.min_heart_rate);
                    p_buf_len += 2;
                    *(uint16_t *)p_buf = co_host_to_uint16_le(p_ftms_env->heart_rate_range.max_heart_rate);
                    p_buf_len += 2;
                    *(uint16_t *)p_buf = co_host_to_uint16_le(p_ftms_env->heart_rate_range.min_increment);
                    p_buf_len += 2;
                }
                else
                {
                    status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
                }
            }
            break;

            case FTMS_IDX_SUPP_POWER_RANGE_CHAR:
            {
                if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->target_setting_feature, FTMS_TARGET_FEAT_POWER_SUP_BIT))
                {
                    *(uint16_t *)p_buf = co_host_to_uint16_le(p_ftms_env->power_range.min_inclination);
                    p_buf_len += 2;
                    *(uint16_t *)p_buf = co_host_to_uint16_le(p_ftms_env->power_range.max_inclination);
                    p_buf_len += 2;
                    *(uint16_t *)p_buf = co_host_to_uint16_le(p_ftms_env->power_range.min_increment);
                    p_buf_len += 2;
                }
                else
                {
                    status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
                }
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
        bes_bt_me_bes_bt_buf_free(p_buf);
    }
}

static void ble_ftms_cb_att_val_set(uint8_t conidx, uint32_t token, const gatt_attribute_t *attr, uint16_t offset, const uint8_t *p_buf, uint8_t buf_len)
{
    uint16_t status = BT_STS_FAILED;

    if (p_ftms_env != NULL)
    {
        uint16_t cfg_upd_flag  = 0;
        uint16_t cfg_en_val = 0;
        switch (ble_ftms_idx_get(attr))
        {
            case FTMS_IDX_TREADMILL_DATA_NTF_CFG:
            {
                cfg_upd_flag = FTMS_PRF_CFG_FLAG_TREADMILL_DATA_NTF_BIT;
                cfg_en_val   = PRF_CLI_START_NTF;
            }
            break;

            case FTMS_IDX_CROSS_TRAINER_DATA_NTF_CFG:
            {
                cfg_upd_flag = FTMS_PRF_CFG_FLAG_CROSS_TRAINER_DATA_NTF_BIT;
                cfg_en_val   = PRF_CLI_START_NTF;
            }
            break;

            case FTMS_IDX_STEP_CLIMBER_DATA_NTF_CFG:
            {
                cfg_upd_flag = FTMS_PRF_CFG_FLAG_STEP_CLIMBER_DATA_NTF_BIT;
                cfg_en_val   = PRF_CLI_START_NTF;
            }
            break;

            case FTMS_IDX_STAIR_CLIMBER_DATA_NTF_CFG:
            {
                cfg_upd_flag = FTMS_PRF_CFG_FLAG_STAIR_CLIMBER_DATA_NTF_BIT;
                cfg_en_val   = PRF_CLI_START_NTF;
            }
            break;

            case FTMS_IDX_ROWER_DATA_NTF_CFG:
            {
                cfg_upd_flag = FTMS_PRF_CFG_FLAG_ROWER_DATA_NTF_BIT;
                cfg_en_val   = PRF_CLI_START_NTF;
            }
            break;

            case FTMS_IDX_INDOOR_BIKE_DATA_NTF_CFG:
            {
                cfg_upd_flag = FTMS_PRF_CFG_FLAG_INDOOR_BIKE_DATA_NTF_BIT;
                cfg_en_val   = PRF_CLI_START_NTF;
            }
            break;

            case FTMS_IDX_TRAINING_STATUS_NTF_CFG:
            {
                cfg_upd_flag = FTMS_PRF_CFG_FLAG_TRAINING_STATUS_NTF_BIT;
                cfg_en_val   = PRF_CLI_START_NTF;
            }
            break;

            case FTMS_IDX_FITNESS_MACHINE_STATUS_NTF_CFG:
            {
                cfg_upd_flag = FTMS_PRF_CFG_FLAG_STATUS_NTF_BIT;
                cfg_en_val   = PRF_CLI_START_NTF;
            }
            break;

            case FTMS_IDX_CTRL_PT_IND_CFG:
            {
                cfg_upd_flag = FTMS_PRF_CFG_FLAG_CTRL_POINT_IND_BIT;
                cfg_en_val   = PRF_CLI_START_IND;
            }
            break;

            case FTMS_IDX_CTRL_PT_CHAR:
            {
                // Check if sending of indications has been enabled
                if (!GETB(p_ftms_env->env[conidx].prfl_ntf_ind_cfg, FTMS_PRF_CFG_FLAG_CTRL_POINT_IND))
                {
                    // improperly configured
                    status = ATT_ERROR_CCCD_IMPROPER_CONFIGED;
                }
                else if (p_ftms_env->ctrl_pt_op != CTRL_PT_RESERVED)
                {
                    // A procedure is already in progress
                    status = ATT_ERROR_PROC_ALREADY_IN_PROGRESS;
                }
                else
                {
                    // Unpack Control Point parameters
                    status = ble_ftms_unpack_ctrl_point_req(conidx, p_buf, buf_len);
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
                const ble_ftms_cb_t *p_cb  = (const ble_ftms_cb_t *) p_ftms_env->p_cb;

                if (cfg == PRF_CLI_STOP_NTFIND)
                {
                    p_ftms_env->env[conidx].prfl_ntf_ind_cfg &= ~cfg_upd_flag;
                }
                else
                {
                    p_ftms_env->env[conidx].prfl_ntf_ind_cfg |= cfg_upd_flag;
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

static void ble_ftms_cb_event_sent(uint8_t conidx, const gatt_attribute_t *p_char_attr, uint16_t status)
{
    // Consider job done
    const ble_ftms_cb_t *p_cb  = (const ble_ftms_cb_t *) p_ftms_env->p_cb;
    p_ftms_env->op_ongoing = false;

    uint32_t dummy = ble_ftms_idx_get(p_char_attr);

    if (dummy == FTMS_IDX_TREADMILL_DATA_CHAR)
    {
        dummy = FTMS_NTF_TREADMILL_DATA_OP_CODE;
    }
    else if (dummy == FTMS_IDX_CROSS_TRAINER_DATA_CHAR)
    {
        dummy = FTMS_NTF_CROSS_TRAINER_DATA_OP_CODE;
    }
    else if (dummy == FTMS_IDX_STEP_CLIMBER_DATA_CHAR)
    {
        dummy = FTMS_NTF_STEP_CLIMBER_DATA_OP_CODE;
    }
    else if (dummy == FTMS_IDX_STAIR_CLIMBER_DATA_CHAR)
    {
        dummy = FTMS_NTF_STAIR_CLIMBER_DATA_OP_CODE;
    }
    else if (dummy == FTMS_IDX_ROWER_DATA_CHAR)
    {
        dummy = FTMS_NTF_ROWER_DATA_OP_CODE;
    }
    else if (dummy == FTMS_IDX_INDOOR_BIKE_DATA_CHAR)
    {
        dummy = FTMS_NTF_INDOOR_BIKE_DATA_OP_CODE;
    }
    else if (dummy == FTMS_IDX_TRAINING_STATUS_CHAR)
    {
        dummy = FTMS_NTF_TRAINING_STATUS_OP_CODE;
    }
    else if (dummy == FTMS_IDX_FITNESS_MACHINE_STATUS_CHAR)
    {
        dummy = FTMS_NTF_FITNESS_MACHINE_STATUS_OP_CODE;
    }
    else
    {
        dummy = CTRL_PT_RSP_CODE;
    }

    switch (dummy)
    {
        case FTMS_NTF_TREADMILL_DATA_OP_CODE:
        case FTMS_NTF_CROSS_TRAINER_DATA_OP_CODE:
        case FTMS_NTF_STEP_CLIMBER_DATA_OP_CODE:
        case FTMS_NTF_STAIR_CLIMBER_DATA_OP_CODE:
        case FTMS_NTF_ROWER_DATA_OP_CODE:
        case FTMS_NTF_INDOOR_BIKE_DATA_OP_CODE:
        case FTMS_NTF_TRAINING_STATUS_OP_CODE:
        case FTMS_IND_CTRL_PT_OP_CODE:
        case FTMS_NTF_FITNESS_MACHINE_STATUS_OP_CODE:
        {
            p_cb->cb_ntf_send_cmp(status);
        }
        break;
        default:
        {
            // Inform application that control point response has been sent
            if (p_ftms_env->ctrl_pt_op != CTRL_PT_RSP_CODE)
            {
                p_cb->cb_ctrl_pt_rsp_send_cmp(conidx, status);
            }

            p_ftms_env->ctrl_pt_op = CTRL_PT_RESERVED;
        }
        break;
    }

    // continue operation execution
    ble_ftms_exe_operation();
}

static void ble_ftms_conn_alloc(uint8_t conidx)
{
    memset(&(p_ftms_env->env[conidx]), 0, sizeof(ble_ftms_cnx_env_t));
}

static void ble_ftms_conn_free(uint8_t conidx)
{
    memset(&(p_ftms_env->env[conidx]), 0, sizeof(ble_ftms_cnx_env_t));
}

static int ble_ftms_gatt_server_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
{
    if (p_ftms_env == NULL)
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
                TRACE(1, "ble ftms resgiter success: shdl = %d", param.resgiter_cmp->start_handle);
            }
            else
            {
                TRACE(1, "ble ftms resgited fail: status = %d", param.resgiter_cmp->status);
            }

            return true;
        }
        case GATT_SERV_EVENT_CONN_OPENED:
        {
            ble_ftms_conn_alloc(conidx);
            return true;
        }
        case GATT_SERV_EVENT_CONN_CLOSED:
        {
            ble_ftms_conn_free(conidx);
            return true;
        }
        case GATT_SERV_EVENT_CHAR_READ:
        {
            ble_ftms_cb_att_read_get(conidx, param.char_read->ctx->token, param.char_read->char_attr,
                                    param.char_read->value_offset);
            return true;
        }
        case GATT_SERV_EVENT_DESC_READ:
        {
            ble_ftms_cb_att_read_get(conidx, param.desc_read->ctx->token, param.desc_read->desc_attr,
                                    param.desc_read->value_offset);
            return true;
        }
        case GATT_SERV_EVENT_CHAR_WRITE:
        {
            ble_ftms_cb_att_val_set(conidx, param.char_write->ctx->token, param.char_write->char_attr,
                                   param.char_write->value_offset, param.char_write->value,
                                   param.char_write->value_len);
            return true;
        }
        case GATT_SERV_EVENT_DESC_WRITE:
        {
            ble_ftms_cb_att_val_set(conidx, param.desc_write->ctx->token, param.desc_write->desc_attr,
                                   param.desc_write->value_offset, param.desc_write->value,
                                   param.desc_write->value_len);
            return true;
        }
        case GATT_SERV_EVENT_INDICATE_CFM:
        case GATT_SERV_EVENT_NTF_TX_DONE:
        {
            ble_ftms_cb_event_sent(conidx, param.confirm->char_attr, param.confirm->error_code);
            return true;
        }
        default:
        {
            break;
        }
    }

    return false;
}

static uint16_t ble_ftms_prepare_service_attr_list(gatt_attribute_t **pp_attr_list, uint8_t *p_attr_list_len, struct ble_ftms_db_cfg *p_init_cfg)
{
    uint8_t idx = 0;

    if (p_attr_list_len == NULL)
    {
        return BT_STS_INVALID_PARM;
    }

    *p_attr_list_len = ARRAY_SIZE(ble_ftms_att_db) 
                        - !FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->target_setting_feature, FTMS_TARGET_FEAT_SPEED_SUP_BIT)
                        - !FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->target_setting_feature, FTMS_TARGET_FEAT_INCLINATION_SUP_BIT)
                        - !FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->target_setting_feature, FTMS_TARGET_FEAT_RESISTANCE_SUP_BIT)
                        - !FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->target_setting_feature, FTMS_TARGET_FEAT_POWER_SUP_BIT)
                        - !FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->target_setting_feature, FTMS_TARGET_FEAT_HEART_RATE_SUP_BIT)
                        - (!FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->prfl_config, FTMS_TREADMILL_DATA_CHAR_SUPP_FLAG)
                        + !FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->prfl_config, FTMS_CROSS_TRAINER_DATA_CHAR_SUPP_FLAG)
                        + !FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->prfl_config, FTMS_STEP_CLIMBER_DATA_CHAR_SUPP_FLAG)
                        + !FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->prfl_config, FTMS_STAIR_CLIMBER_DATA_CHAR_SUPP_FLAG)
                        + !FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->prfl_config, FTMS_ROWER_DATA_CHAR_SUPP_FLAG)
                        + !FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->prfl_config, FTMS_INDOOR_BIKE_DATA_CHAR_SUPP_FLAG)
                        + !FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->prfl_config, FTMS_TRAINING_STATUS_CHAR_SUPP_FLAG)
                        + (!FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->prfl_config, FTMS_CTRL_PT_CHAR_SUPP_FLAG))*2) * 2;
                        //if not support Control Point, will not support status
    uint8_t size = *p_attr_list_len * sizeof(gatt_attribute_t);

    *pp_attr_list = (gatt_attribute_t *)bes_bt_me_bes_bt_buf_malloc(size);

    if (*pp_attr_list == NULL)
    {
        return BT_STS_NO_RESOURCES;
    }
    memset(*pp_attr_list, 0, size);

    memcpy(*pp_attr_list, ble_ftms_att_db, size);
    idx += 1;

    if (FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->prfl_config, FTMS_TREADMILL_DATA_CHAR_SUPP_FLAG))
    {
        *(*pp_attr_list + idx) = ble_ftms_att_db[FTMS_IDX_TREADMILL_DATA_CHAR];
        idx += 1;
        * (*pp_attr_list + idx) = ble_ftms_att_db[FTMS_IDX_TREADMILL_DATA_NTF_CFG];
        idx += 1;
    }

    if (FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->prfl_config, FTMS_CROSS_TRAINER_DATA_CHAR_SUPP_FLAG))
    {
        *(*pp_attr_list + idx) = ble_ftms_att_db[FTMS_IDX_CROSS_TRAINER_DATA_CHAR];
        idx += 1;
        * (*pp_attr_list + idx) = ble_ftms_att_db[FTMS_IDX_CROSS_TRAINER_DATA_NTF_CFG];
        idx += 1;
    }

    if (FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->prfl_config, FTMS_STEP_CLIMBER_DATA_CHAR_SUPP_FLAG))
    {
        *(*pp_attr_list + idx) = ble_ftms_att_db[FTMS_IDX_STEP_CLIMBER_DATA_CHAR];
        idx += 1;
        * (*pp_attr_list + idx) = ble_ftms_att_db[FTMS_IDX_STEP_CLIMBER_DATA_NTF_CFG];
        idx += 1;
    }

    if (FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->prfl_config, FTMS_STAIR_CLIMBER_DATA_CHAR_SUPP_FLAG))
    {
        *(*pp_attr_list + idx) = ble_ftms_att_db[FTMS_IDX_STAIR_CLIMBER_DATA_CHAR];
        idx += 1;
        * (*pp_attr_list + idx) = ble_ftms_att_db[FTMS_IDX_STAIR_CLIMBER_DATA_NTF_CFG];
        idx += 1;
    }

    if (FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->prfl_config, FTMS_ROWER_DATA_CHAR_SUPP_FLAG))
    {
        *(*pp_attr_list + idx) = ble_ftms_att_db[FTMS_IDX_ROWER_DATA_CHAR];
        idx += 1;
        * (*pp_attr_list + idx) = ble_ftms_att_db[FTMS_IDX_ROWER_DATA_NTF_CFG];
        idx += 1;
    }

    if (FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->prfl_config, FTMS_INDOOR_BIKE_DATA_CHAR_SUPP_FLAG))
    {
        *(*pp_attr_list + idx) = ble_ftms_att_db[FTMS_IDX_INDOOR_BIKE_DATA_CHAR];
        idx += 1;
        * (*pp_attr_list + idx) = ble_ftms_att_db[FTMS_IDX_INDOOR_BIKE_DATA_NTF_CFG];
        idx += 1;
    }

    if (FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->prfl_config, FTMS_TRAINING_STATUS_CHAR_SUPP_FLAG))
    {
        *(*pp_attr_list + idx) = ble_ftms_att_db[FTMS_IDX_TRAINING_STATUS_CHAR];
        idx += 1;
        * (*pp_attr_list + idx) = ble_ftms_att_db[FTMS_IDX_TRAINING_STATUS_NTF_CFG];
        idx += 1;
    }

    if (FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->target_setting_feature, FTMS_TARGET_FEAT_SPEED_SUP_BIT))
    {
        *(*pp_attr_list + idx) = ble_ftms_att_db[FTMS_IDX_SUPP_SPEED_RANGE_CHAR];
        idx += 1;
    }

    if (FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->target_setting_feature, FTMS_TARGET_FEAT_INCLINATION_SUP_BIT))
    {
        *(*pp_attr_list + idx) = ble_ftms_att_db[FTMS_IDX_SUPP_INCLINATION_RANGE_CHAR];
        idx += 1;
    }

    if (FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->target_setting_feature, FTMS_TARGET_FEAT_RESISTANCE_SUP_BIT))
    {
        *(*pp_attr_list + idx) = ble_ftms_att_db[FTMS_IDX_SUPP_RESIST_LEVEL_RANGE_CHAR];
        idx += 1;
    }

    if (FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->target_setting_feature, FTMS_TARGET_FEAT_HEART_RATE_SUP_BIT))
    {
        *(*pp_attr_list + idx) = ble_ftms_att_db[FTMS_IDX_SUPP_HEART_RATE_RANGE_CHAR];
        idx += 1;
    }

    if (FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->target_setting_feature, FTMS_TARGET_FEAT_POWER_SUP_BIT))
    {
        *(*pp_attr_list + idx) = ble_ftms_att_db[FTMS_IDX_SUPP_POWER_RANGE_CHAR];
        idx += 1;
    }

    if (FTMS_IS_FEATURE_SUPPORTED(p_init_cfg->prfl_config, FTMS_CTRL_PT_CHAR_SUPP_FLAG))
    {
        *(*pp_attr_list + idx) = ble_ftms_att_db[FTMS_IDX_CTRL_PT_CHAR];
        idx += 1;
        * (*pp_attr_list + idx) = ble_ftms_att_db[FTMS_IDX_CTRL_PT_IND_CFG];
        idx += 1;
        *(*pp_attr_list + idx) = ble_ftms_att_db[FTMS_IDX_FITNESS_MACHINE_STATUS_CHAR];
        idx += 1;
        * (*pp_attr_list + idx) = ble_ftms_att_db[FTMS_IDX_FITNESS_MACHINE_STATUS_NTF_CFG];
        idx += 1;
    }

    return BT_STS_SUCCESS;
}

/* FUNCSTIONS */
uint16_t ble_ftms_enable(uint8_t conidx, uint8_t ntf_ind_cfg)
{
    uint16_t status = BT_STS_NOT_ALLOW;

    if (p_ftms_env != NULL)
    {
        // check state of the task
        if (app_ble_get_conhdl_from_conidx(conidx) != 0xFFFF)
        {
            p_ftms_env->env[conidx].prfl_ntf_ind_cfg = ntf_ind_cfg;
            status = BT_STS_SUCCESS;
        }
    }

    return status;
}

uint16_t ble_ftms_adv_data_pack(uint8_t *p_buf, uint8_t buf_len)
{
    uint16_t status = BT_STS_NOT_ALLOW;

    if (p_buf == NULL)
    {
        status = BT_STS_INVALID_PARM;
    }
    else if (buf_len < FTMS_ADV_MAX_LEN)
    {
        status = BT_STS_INVALID_LENGTH;
    }
    else if (p_ftms_env != NULL)
    {
        uint16_t type_flags = 0;
        uint8_t machine_flags = 0;
        // Check what char is supported
        if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_TREADMILL_DATA_CHAR_SUPP_FLAG))
        {
            type_flags |= FTMS_TREADMILL_DATA_CHAR_SUPP_FLAG;
        }

        if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_CROSS_TRAINER_DATA_CHAR_SUPP_FLAG))
        {
            type_flags |= FTMS_CROSS_TRAINER_DATA_CHAR_SUPP_FLAG;
        }

        if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_STEP_CLIMBER_DATA_CHAR_SUPP_FLAG))
        {
            type_flags |= FTMS_STEP_CLIMBER_DATA_CHAR_SUPP_FLAG;
        }

        if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_STAIR_CLIMBER_DATA_CHAR_SUPP_FLAG))
        {
            type_flags |= FTMS_STAIR_CLIMBER_DATA_CHAR_SUPP_FLAG;
        }

        if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_ROWER_DATA_CHAR_SUPP_FLAG))
        {
            type_flags |= FTMS_ROWER_DATA_CHAR_SUPP_FLAG;
        }

        if (FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_INDOOR_BIKE_DATA_CHAR_SUPP_FLAG))
        {
            type_flags |= FTMS_INDOOR_BIKE_DATA_CHAR_SUPP_FLAG;
        }

        if(type_flags)
        {
            machine_flags |= FTMS_FTMS_MACHINE_AVA_FLAG;
        }

        // Pack Service Data AD type
        *(p_buf + 1) = GAP_DT_SERVICE_DATA_16BIT_UUID;
        buf_len += 1;

        // Pack UUID of FTMS
        *(uint16_t *)(p_buf + 2) = co_host_to_uint16_le(GATT_UUID_FIT_SERVICE);
        buf_len += 2;

        // Pack Flags
        *(p_buf + 1) = machine_flags;
        buf_len += 1;

        // Pack FTMS type
        *(uint16_t *)(p_buf + 2) = co_host_to_uint16_le(type_flags);
        buf_len += 2;

        // Set AD length
        p_buf[0] = (uint8_t)(buf_len - 1);

        status = BT_STS_SUCCESS;
    }

    return status;
}

uint16_t ble_ftms_data_record_send(uint32_t conidx_bf, uint8_t operation, uint8_t *data, uint8_t len)
{
    uint16_t status = BT_STS_NOT_ALLOW;

    if (data == NULL)
    {
        status = BT_STS_INVALID_PARM;
    }
    else if (p_ftms_env != NULL)
    {
        uint8_t *p_buf = NULL;

        if ((p_buf = bes_bt_me_bes_bt_buf_malloc(sizeof(ble_ftms_buf_meta_t))))
        {
            ble_ftms_buf_meta_t *p_buf_meta = (ble_ftms_buf_meta_t *)p_buf;
            p_buf_meta->operation = operation;
            p_buf_meta->conidx    = 0;
            p_buf_meta->conidx_bf = conidx_bf & ((1 << BLE_CONNECTION_MAX) - 1);
            p_buf_meta->new       = true;
            p_buf_meta->buf_len   = len;

            // copy structure info - use buffer head to ensure that buffer is 32-bit aligned
            memcpy(p_buf_meta->buf, data, len);
            // put event on wait queue
            colist_addto_tail(&(p_buf_meta->node), &(p_ftms_env->wait_queue));
            // execute operation
            ble_ftms_exe_operation();

            status = BT_STS_SUCCESS;
        }
        else
        {
            status = BT_STS_NO_RESOURCES;
        }
    }

    return status;

}

uint16_t ble_ftms_ctrl_pt_rsp_send(uint8_t conidx, uint8_t op_code, uint8_t resp_val, const ble_ftm_ctrl_pt_rsp_val *p_value)
{
    uint16_t status = BT_STS_NOT_ALLOW;

    if (p_value == NULL)
    {
        status = BT_STS_INVALID_PARM;
    }
    else if (p_ftms_env != NULL)
    {
        do
        {
            uint8_t *p_buf = NULL;

            // Check the current operation
            if (p_ftms_env->ctrl_pt_op ==  CTRL_PT_RESERVED)
            {
                // The confirmation has been sent without request indication, ignore
                break;
            }

            // The Control Point Characteristic must be supported if we are here
            if (!FTMS_IS_FEATURE_SUPPORTED(p_ftms_env->prfl_config, FTMS_CTRL_PT_CHAR_SUPP_FLAG))
            {
                status = BT_STS_NOT_ALLOW;
                break;
            }

            // Check if sending of indications has been enabled
            if (!GETB(p_ftms_env->env[conidx].prfl_ntf_ind_cfg, FTMS_PRF_CFG_FLAG_CTRL_POINT_IND))
            {
                // mark operation done
                p_ftms_env->ctrl_pt_op = CTRL_PT_RESERVED;
                // improperly configured
                status = ATT_ERROR_CCCD_IMPROPER_CONFIGED;
                break;
            }

            if ((p_buf = bes_bt_me_bes_bt_buf_malloc(FTMS_CP_NTF_MAX_LEN)))
            {
                ble_ftms_buf_meta_t *p_buf_meta = (ble_ftms_buf_meta_t *)p_buf;
                p_buf_meta->operation = CTRL_PT_RSP_CODE;
                p_buf_meta->conidx    = conidx;
                p_buf_meta->new       = true;
                p_buf_meta->buf_len   = FTMS_CP_NTF_MAX_LEN;

                // Pack structure
                p_buf_meta->buf_len = ble_ftms_pack_ctrl_point_rsp(conidx, p_buf_meta->buf, op_code, resp_val, p_value);
                // put event on wait queue
                colist_addto_tail(&(p_buf_meta->node), &(p_ftms_env->wait_queue));
                // execute operation
                ble_ftms_exe_operation();

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

uint16_t ble_ftms_init(struct ble_ftms_db_cfg *p_init_cfg, const ble_ftms_cb_t *p_cb)
{
    // DB Creation Status
    uint16_t status = BT_STS_SUCCESS;
    gatt_attribute_t *p_attr_list = NULL;
    uint8_t attr_list_len = 0;

    if (p_ftms_env != NULL)
    {
        return BT_STS_ALREADY_REGISTERED;
    }

    do
    {
        if ((p_init_cfg == NULL) ||
                (p_cb == NULL) || (p_cb->cb_bond_data_upd == NULL)  || (p_cb->cb_ctrl_pt_req == NULL)
                || (p_cb->cb_ctrl_pt_rsp_send_cmp == NULL))
        {
            status = BT_STS_INVALID_PARM;
            break;
        }

        gatts_cfg_t cfg =
        {
            .preferred_mtu = FTMS_CP_NTF_MAX_LEN + 3,
        };

        status = ble_ftms_prepare_service_attr_list(&p_attr_list, &attr_list_len, p_init_cfg);

        if (status == BT_STS_SUCCESS)
        {
            // register FTMS user
            status = gatts_register_service(p_attr_list, attr_list_len, ble_ftms_gatt_server_callback, &cfg);
        }

        if (status != BT_STS_SUCCESS)
        {
            break;
        }

        p_ftms_env = (ble_ftms_env_t *) bes_bt_me_bes_bt_buf_malloc(sizeof(ble_ftms_env_t));

        if (p_ftms_env != NULL)
        {
            // allocate CPS required environment variable
            p_ftms_env->prfl_config        = p_init_cfg->prfl_config;
            p_ftms_env->ftms_feature        = p_init_cfg->ftms_feature;
            p_ftms_env->target_setting_feature        = p_init_cfg->target_setting_feature;
            p_ftms_env->op_ongoing      = false;
            p_ftms_env->in_exe_op       = false;
            p_ftms_env->ctrl_conidx     = INVALID_CONIDX;
            p_ftms_env->ctrl_pt_op      = CTRL_PT_RESERVED;
            p_ftms_env->p_attr_list     = p_attr_list;
            memset(p_ftms_env->env, 0, sizeof(p_ftms_env->env));
            INIT_LIST_HEAD(&(p_ftms_env->wait_queue));
            // initialize profile environment variable
            p_ftms_env->p_cb     = p_cb;
        }
        else
        {
            status = BT_STS_NO_RESOURCES;
        }

    } while (0);

    if (status != BT_STS_SUCCESS && p_attr_list != NULL)
    {
        gatts_unregister_service(p_attr_list);
        bes_bt_me_bes_bt_buf_free((uint8_t *)p_attr_list);
    }

    return status;
}

uint16_t ble_ftms_deinit(void)
{
    if (p_ftms_env != NULL)
    {
        // remove buffer in wait queue
        while (!colist_is_list_empty(&p_ftms_env->wait_queue))
        {
            ble_ftms_buf_meta_t *p_meta = (ble_ftms_buf_meta_t *)colist_get_head(&p_ftms_env->wait_queue);
            colist_delete(&(p_meta->node));
            bes_bt_me_bes_bt_buf_free((uint8_t *)p_meta);
        }

        gatts_unregister_service(p_ftms_env->p_attr_list);
        bes_bt_me_bes_bt_buf_free((uint8_t *)p_ftms_env->p_attr_list);

        // free profile environment variables
        bes_bt_me_bes_bt_buf_free((uint8_t *)p_ftms_env);
        p_ftms_env = NULL;
    }

    return BT_STS_SUCCESS;
}

#endif /* BLE_FTMS_ENABLED */
