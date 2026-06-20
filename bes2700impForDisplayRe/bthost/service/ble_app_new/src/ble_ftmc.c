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
#if defined(BLE_FTMC_ENABLED)
#include "gatt_service.h"
#include "ble_ftmc.h"

#define FTMC_MAX_LEN (128)

/**
 * The Fitness Machine shall instantiate one and only one Fitness Machine Service
 *
 */

/* DEFINES */
typedef struct ble_ftmc_connection
{
    /// Control point timer
    uint8_t             timer;
    /// Peer database discovered handle mapping
    gatt_peer_character_t *chars[FTM_CHAR_MAX_NUM];
    /// Client is in discovering state
    bool                discover;
    /// Control point operation on-going (see enum #ftmc_opcode_id)
    uint8_t             ctrl_pt_op;
} ble_ftmc_conn_t;

/// Client environment variable
typedef struct ble_ftmc_env
{
    /// Callbacks
    const ble_ftmc_cb_t  *p_cbs;
    /// Environment variable pointer for each connections
    ble_ftmc_conn_t       *p_env[BLE_CONNECTION_MAX];
    /// GATT Profile local identifier
    uint8_t              prf_lid;
} ble_ftmc_env_t;

static ble_ftmc_env_t *p_ftmc_env = NULL;

static uint16_t ble_ftmc_read_val(uint8_t conidx, uint16_t val_id)
{
    gatt_prf_t *p_prf = gattc_get_profile(p_ftmc_env->prf_lid,
                                          app_ble_get_conhdl_from_conidx(conidx));

    uint16_t status = ATT_ERROR_RD_NOT_PERMITTED;

    if ((conidx < BLE_CONNECTION_MAX) && (p_ftmc_env->p_env[conidx] != NULL) && (!p_ftmc_env->p_env[conidx]->discover))
    {
        ble_ftmc_conn_t *p_con_env = p_ftmc_env->p_env[conidx];
        gatt_peer_character_t *p_char = NULL;
        uint16_t desc_uuid = 0;

        switch (val_id)
        {
            case BLE_FITNESS_MACHINE_FEATURE:
            {
                p_char = p_con_env->chars[FTM_CHAR_FEATURE];
            }
            break;
            case BLE_TRAINING_STATUS:
            {
                p_char = p_con_env->chars[FTM_CHAR_TRAINING_STATUS];
            }
            break;
            case BLE_SUPP_SPEED_RANGE:
            {
                p_char = p_con_env->chars[FTM_CHAR_SUPP_SPEED_RANGE];
            }
            break;
            case BLE_SUPP_INCLINATION_RANGE:
            {
                p_char = p_con_env->chars[FTM_CHAR_SUPP_INCLINATION_RANGE];
            }
            break;
            case BLE_SUPP_RESIST_LEVEL_RANGE:
            {
                p_char = p_con_env->chars[FTM_CHAR_SUPP_RESISTANCE_LEVEL_RANGE];
            }
            break;
            case BLE_SUPP_HEART_RATE_RANGE:
            {
                p_char = p_con_env->chars[FTM_CHAR_SUPP_HEART_RATE_RANGE];
            }
            break;
            case BLE_SUPP_POWER_RANGE:
            {
                p_char = p_con_env->chars[FTM_CHAR_SUPP_POWER_RANGE];
            }
            break;
            case BLE_FTMC_RD_WR_TREADMILL_DATA_CL_CFG:
            {
                desc_uuid = GATT_DESC_UUID_CHAR_CLIENT_CONFIG;
                p_char = p_con_env->chars[FTM_CHAR_TREADMILL_DATA];
            }
            break;
            case BLE_FTMC_RD_WR_CROSS_TRAINER_CL_CFG:
            {
                desc_uuid = GATT_DESC_UUID_CHAR_CLIENT_CONFIG;
                p_char = p_con_env->chars[FTM_CHAR_CROSS_TRAINER_DATA];
            }
            break;
            case BLE_FTMC_RD_WR_STEP_CLIMBER_CL_CFG:
            {
                desc_uuid = GATT_DESC_UUID_CHAR_CLIENT_CONFIG;
                p_char = p_con_env->chars[FTM_CHAR_STEP_CLIMBER_DATA_DATA];
            }
            break;
            case BLE_FTMC_RD_WR_STAIR_CLIMBER_CL_CFG:
            {
                desc_uuid = GATT_DESC_UUID_CHAR_CLIENT_CONFIG;
                p_char = p_con_env->chars[FTM_CHAR_STAIR_CLIMBER_DATA_DATA];
            }
            break;
            case BLE_FTMC_RD_WR_ROWER_CL_CFG:
            {
                desc_uuid = GATT_DESC_UUID_CHAR_CLIENT_CONFIG;
                p_char = p_con_env->chars[FTM_CHAR_ROWER_DATA];
            }
            break;
            case BLE_FTMC_RD_WR_INDOOR_BIKE_CL_CFG:
            {
                desc_uuid = GATT_DESC_UUID_CHAR_CLIENT_CONFIG;
                p_char = p_con_env->chars[FTM_CHAR_INDOOR_BIKE_DATA];
            }
            break;
            case BLE_FTMC_RD_WR_TRAINING_STATUS_CL_CFG:
            {
                desc_uuid = GATT_DESC_UUID_CHAR_CLIENT_CONFIG;
                p_char = p_con_env->chars[FTM_CHAR_TRAINING_STATUS];
            }
            break;
            case BLE_FTMC_RD_WR_CTRL_PT_CFG:
            {
                desc_uuid = GATT_DESC_UUID_CHAR_CLIENT_CONFIG;
                p_char = p_con_env->chars[FTM_CHAR_CPT];
            }
            break;
            case BLE_FTMC_RD_WR_FITNESS_MACHINE_STATUS_CFG:
            {
                desc_uuid = GATT_DESC_UUID_CHAR_CLIENT_CONFIG;
                p_char = p_con_env->chars[FTM_CHAR_STATUS];
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

static void ble_ftmc_read_val_cmp(uint8_t conidx, uint16_t status, uint8_t val_id, const uint8_t *p_data, uint8_t buf_len)
{
    if (p_ftmc_env != NULL)
    {
        const ble_ftmc_cb_t *p_cb = (const ble_ftmc_cb_t *) p_ftmc_env->p_cbs;
        switch (val_id)
        {
            case (BLE_FITNESS_MACHINE_FEATURE):
            {
                if (status == BT_STS_SUCCESS)
                {
                    p_cb->cb_read_char_cmp(conidx, status, p_data, buf_len);
                }
            }
            break;
            // Read Client Characteristic Configuration Descriptor value
            case (BLE_FTMC_RD_WR_TREADMILL_DATA_CL_CFG):
            case (BLE_FTMC_RD_WR_CROSS_TRAINER_CL_CFG):
            case (BLE_FTMC_RD_WR_STEP_CLIMBER_CL_CFG):
            case (BLE_FTMC_RD_WR_STAIR_CLIMBER_CL_CFG):
            case (BLE_FTMC_RD_WR_ROWER_CL_CFG):
            case (BLE_FTMC_RD_WR_INDOOR_BIKE_CL_CFG):
            case (BLE_FTMC_RD_WR_TRAINING_STATUS_CL_CFG):
            case (BLE_FTMC_RD_WR_CTRL_PT_CFG):
            case (BLE_FTMC_RD_WR_FITNESS_MACHINE_STATUS_CFG):
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

static uint8_t ble_ftmc_char_uuid_to_char_type(uint16_t char_uuid)
{
    if (char_uuid == GATT_CHAR_UUID_FITNESS_MACHINE_FEATURE)
    {
        return FTM_CHAR_FEATURE;
    }
    else if (char_uuid == GATT_CHAR_UUID_TREADMILL_DATA)
    {
        return FTM_CHAR_TREADMILL_DATA;
    }
    else if (char_uuid == GATT_CHAR_UUID_CROSS_TRAINER_DATA)
    {
        return FTM_CHAR_CROSS_TRAINER_DATA;
    }
    else if (char_uuid == GATT_CHAR_UUID_STEP_CLIMBER_DATA)
    {
        return FTM_CHAR_STEP_CLIMBER_DATA_DATA;
    }
    else if (char_uuid == GATT_CHAR_UUID_STAIR_CLIMBER_DATA)
    {
        return FTM_CHAR_STAIR_CLIMBER_DATA_DATA;
    }
    else if (char_uuid == GATT_CHAR_UUID_ROWER_DATA)
    {
        return FTM_CHAR_ROWER_DATA;
    }
    else if (char_uuid == GATT_CHAR_UUID_INDOOR_BIKE_DATA)
    {
        return FTM_CHAR_INDOOR_BIKE_DATA;
    }
    else if (char_uuid == GATT_CHAR_UUID_TRAINING_STATUS)
    {
        return FTM_CHAR_TRAINING_STATUS;
    }
    else if (char_uuid == GATT_CHAR_UUID_SUPP_SPEED_RANGE)
    {
        return FTM_CHAR_SUPP_SPEED_RANGE;
    }
    else if (char_uuid == GATT_CHAR_UUID_SUPP_INCLINATION_RANGE)
    {
        return FTM_CHAR_SUPP_INCLINATION_RANGE;
    }
    else if (char_uuid == GATT_CHAR_UUID_SUPP_RESIST_LEVEL_RANGE)
    {
        return FTM_CHAR_SUPP_RESISTANCE_LEVEL_RANGE;
    }
    else if (char_uuid == GATT_CHAR_UUID_SUPP_HEART_RATE_RANGE)
    {
        return FTM_CHAR_SUPP_HEART_RATE_RANGE;
    }
    else if (char_uuid == GATT_CHAR_UUID_SUPP_POWER_RANGE)
    {
        return FTM_CHAR_SUPP_POWER_RANGE;
    }
    else if (char_uuid == GATT_CHAR_UUID_FITNESS_MACHINE_CTRL_POINT)
    {
        return FTM_CHAR_CPT;
    }
    else if (char_uuid == GATT_CHAR_UUID_FITNESS_MACHINE_STATUS)
    {
        return FTM_CHAR_STATUS;
    }

    return FTM_CHAR_MAX_NUM;
}

static uint8_t ble_ftmc_char_uuid_to_val_id(uint16_t char_uuid, bool is_desc, bool is_sccd)
{
    uint8_t char_type = ble_ftmc_char_uuid_to_char_type(char_uuid);

    if (is_desc == false)
    {
        return char_type;
    }
    else
    {
        if (char_type == FTM_CHAR_TREADMILL_DATA)
        {
            return BLE_FTMC_DESC_TREADMILL_DATA_CL_CFG;
        }
        else if (char_type == FTM_CHAR_CROSS_TRAINER_DATA)
        {
            return BLE_FTMC_DESC_CROSS_TRAINER_CL_CFG;
        }
        else if (char_type == FTM_CHAR_STEP_CLIMBER_DATA_DATA)
        {
            return BLE_FTMC_DESC_STEP_CLIMBER_CL_CFG;
        }
        else if (char_type == FTM_CHAR_STAIR_CLIMBER_DATA_DATA)
        {
            return BLE_FTMC_DESC_STAIR_CLIMBER_CL_CFG;
        }
        else if (char_type == FTM_CHAR_ROWER_DATA)
        {
            return BLE_FTMC_DESC_ROWER_CL_CFG;
        }
        else if (char_type == FTM_CHAR_INDOOR_BIKE_DATA)
        {
            return BLE_FTMC_DESC_INDOOR_BIKE_CL_CFG;
        }
        else if (char_type == FTM_CHAR_TRAINING_STATUS)
        {
            return BLE_FTMC_DESC_TRAINING_STATUS_CL_CFG;
        }
        else if (char_type == FTM_CHAR_CPT)
        {
            return BLE_FTMC_DESC_CTRL_PT_CL_CFG;
        }
        else if (char_type == FTM_CHAR_STATUS)
        {
            return BLE_FTMC_DESC_FITNESS_MACHINE_STATUS_CFG;
        }
    }

    return 0xFF;
}

static void app_ftmc_parse_ctrl_pt_notification_info(uint8_t conidx, uint8_t char_enum, const uint8_t *value, uint16_t len)
{
    if (value == NULL || len == 0)
    {
        return;
    }

    bool valid = (len >= FTMS_CTRL_PT_RSP_MIN_LEN);

    const uint8_t *p_buf_head = value;

    uint8_t rsp_code;
    uint8_t req_op_code;
    uint8_t result_code;
    uint8_t buf_len = 0;
    ble_ftm_ctrl_pt_rsp_val rsp_value;
    memset(&rsp_value, 0, sizeof(ble_ftm_ctrl_pt_rsp_val));

    // Response Op code
    rsp_code = value[0];
    value += 1;

    // Requested operation code
    req_op_code = value[0];
    value += 1;

    // Result code
    result_code = value[0];
    value += 1;

    if (valid && (rsp_code == CTRL_PT_RSP_CODE) && (req_op_code == p_ftmc_env->p_env[conidx]->ctrl_pt_op))
    {
        const ble_ftmc_cb_t *p_cb = (const ble_ftmc_cb_t *) p_ftmc_env->p_cbs;

        if (result_code == FTM_RESULT_SUCCESS)
        {
            switch (req_op_code)
            {
                //only Spin Down Op Code has resp param
                case (CTRL_PT_SET_SPINDOWN_CONTRL):
                {
                    while (buf_len - (value - p_buf_head) > 0)
                    {
                        rsp_value.target_speed_low = CO_COMBINE_UINT16_LE(value);
                        value += 2;
                        rsp_value.target_speed_high = CO_COMBINE_UINT16_LE(value);
                        value += 2;
                    }
                }
                break;

                default:
                {

                } break;
            }
        }

        rsp_value.rsp_code = rsp_code;
        rsp_value.req_code = req_op_code;
        rsp_value.result_code = result_code;
        p_ftmc_env->p_env[conidx]->ctrl_pt_op = CTRL_PT_RESERVED;
        // stop timer
        co_timer_stop(&(p_ftmc_env->p_env[conidx]->timer));
        // provide control point response
        p_cb->cb_ctrl_pt_req_cmp(conidx, BT_STS_SUCCESS, req_op_code, result_code, &rsp_value);
    }
}

static void ble_ftmc_timer_handler(void *args)
{
    uint32_t conidx = (uint32_t)args;
    // Get the address of the environment
    if (p_ftmc_env != NULL)
    {
        ble_ftmc_conn_t *p_con_env = p_ftmc_env->p_env[conidx];
        ASSERT_ERR(p_con_env != NULL);
        if (p_con_env->ctrl_pt_op != CTRL_PT_RESERVED)
        {
            const ble_ftmc_cb_t *p_cb = (const ble_ftmc_cb_t *) p_ftmc_env->p_cbs;
            uint8_t op_code = p_con_env->ctrl_pt_op;
            p_con_env->ctrl_pt_op = CTRL_PT_RESERVED;

            p_cb->cb_ctrl_pt_req_cmp((uint8_t)conidx, ATT_ERROR_RESPONSE_TIMEOUT, op_code, 0, NULL);
        }
    }
}

static uint8_t ble_check_char_is_exist(uint8_t conidx, gatt_peer_character_t * peer_char)
{
    uint8_t ftm_char = FTM_CHAR_FEATURE;
    for(; ftm_char < FTM_CHAR_MAX_NUM; ftm_char++)
    {
        if(peer_char == p_ftmc_env->p_env[conidx]->chars[ftm_char])
        {
            return ftm_char;
        }
    }

    return FTM_CHAR_MAX_NUM;
}

static void ble_ftmc_con_create(uint8_t conidx)
{
    // Nothing to do
}

static void ble_ftmc_con_cleanup(uint8_t conidx)
{
    // clean-up environment variable allocated for task instance
    if (p_ftmc_env->p_env[conidx] != NULL)
    {
        osTimerStop(&(p_ftmc_env->p_env[conidx]->timer));
        bes_bt_me_bes_bt_buf_free((uint8_t *)p_ftmc_env->p_env[conidx]);
        p_ftmc_env->p_env[conidx] = NULL;
    }
}

static void ble_ftmc_discover_cmp_cb(uint8_t conidx, uint16_t status)
{
    if (status != ATT_ERROR_NO_ERROR)
    {
        ble_ftmc_con_cleanup(conidx);
    }
    else
    {
        p_ftmc_env->p_env[conidx]->discover = false;
    }

    // Get the address of the environment
    if (p_ftmc_env != NULL)
    {
        p_ftmc_env->p_cbs->cb_svc_discover_cmp(conidx, status);
    }
}

static void ble_ftmc_att_val_cb(uint8_t conidx, gatt_peer_character_t *p_char, bool is_desc, bool is_sccd, const uint8_t *p_buf, uint8_t buf_len)
{
    gatt_peer_character_uuid_t uuid;
    gattc_get_character_uuid(p_char, &uuid);
    uint8_t val_id = ble_ftmc_char_uuid_to_val_id(uuid.char_uuid, is_desc, is_sccd);
    ble_ftmc_read_val_cmp(conidx, BT_STS_SUCCESS, val_id, p_buf, buf_len);
}

static void ble_ftmc_read_cmp_cb(uint8_t conidx, gatt_peer_character_t *p_char, uint16_t status, bool is_desc, bool is_sccd)
{
    if (status != BT_STS_SUCCESS)
    {
        gatt_peer_character_uuid_t uuid;
        gattc_get_character_uuid(p_char, &uuid);
        uint8_t val_id = ble_ftmc_char_uuid_to_val_id(uuid.char_uuid, is_desc, is_sccd);
        ble_ftmc_read_val_cmp(conidx, status, val_id, NULL, 0);
    }
}

static void ble_ftmc_write_cmp_cb(uint8_t conidx, gatt_peer_character_t *p_char, uint16_t status, bool is_desc, bool is_sccd)
{
    if (p_ftmc_env != NULL)
    {
        const ble_ftmc_cb_t *p_cb = (const ble_ftmc_cb_t *) p_ftmc_env->p_cbs;
        gatt_peer_character_uuid_t uuid;
        gattc_get_character_uuid(p_char, &uuid);
        uint8_t val_id = ble_ftmc_char_uuid_to_val_id(uuid.char_uuid, is_desc, is_sccd);

        switch (val_id)
        {
            // Config control
            case BLE_FTMC_RD_WR_TREADMILL_DATA_CL_CFG:
            case BLE_FTMC_RD_WR_CROSS_TRAINER_CL_CFG:
            case BLE_FTMC_RD_WR_STEP_CLIMBER_CL_CFG:
            case BLE_FTMC_RD_WR_STAIR_CLIMBER_CL_CFG:
            case BLE_FTMC_RD_WR_ROWER_CL_CFG:
            case BLE_FTMC_RD_WR_INDOOR_BIKE_CL_CFG:
            case BLE_FTMC_RD_WR_TRAINING_STATUS_CL_CFG:
            case BLE_FTMC_RD_WR_FITNESS_MACHINE_STATUS_CFG:
            {
                p_cb->cb_write_cfg_cmp(conidx, status, val_id);
            }
            break;
            // Control point commands
            case BLE_FTMC_RD_WR_CTRL_PT_CFG:
            {
                if (status != BT_STS_SUCCESS)
                {
                    uint8_t opcode = p_ftmc_env->p_env[conidx]->ctrl_pt_op;
                    p_ftmc_env->p_env[conidx]->ctrl_pt_op = CTRL_PT_RESERVED;
                    p_cb->cb_ctrl_pt_req_cmp(conidx, status, opcode, 0, NULL);
                }
                else
                {
                    // Start Timeout Procedure - wait for Indication reception
                    osTimerStart(&(p_ftmc_env->p_env[conidx]->timer), FTM_CP_TIMEOUT);
                }
            }
            break;

            default: { /* Nothing to do */} break;
        }
    }
}

static void ble_ftmc_service_disocver_cb(gatt_prf_t *prf, uint8_t conidx, gatt_peer_service_t *p_svc, uint16_t status)
{
    if (p_ftmc_env->p_env[conidx] == NULL)
    {
        return;
    }

    if (status != ATT_ERROR_NO_ERROR)
    {
        ble_ftmc_discover_cmp_cb(conidx, status);
        return;
    }

    gattc_discover_all_characters(prf, p_svc);
}

static void ble_ftmc_character_disocver_cb(gatt_prf_t *prf, uint8_t conidx, uint16_t char_uuid, gatt_peer_character_t *p_char, uint16_t status, bool cmpl)
{
    if (p_ftmc_env->p_env[conidx] == NULL)
    {
        return;
    }

    if (status != ATT_ERROR_NO_ERROR)
    {
        ble_ftmc_discover_cmp_cb(conidx, status);
        return;
    }

    uint8_t char_type = ble_ftmc_char_uuid_to_char_type(char_uuid);

    if (char_type >= FTM_CHAR_MAX_NUM)
    {
        return;
    }

    p_ftmc_env->p_env[conidx]->chars[char_type] = p_char;

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
        ble_ftmc_discover_cmp_cb(conidx, ATT_ERROR_NO_ERROR);
    }
}

static int ble_ftmc_client_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param)
{
    if (p_ftmc_env == NULL)
    {
        return false;
    }

    if (prf->con_idx < gap_zero_based_conidx_to_ble_conidx(0))
    {
        TRACE(1, "%s not support conidx = %d", __func__, prf->con_idx);
        return false;
    }

    uint8_t conidx = gap_zero_based_conidx(prf->con_idx);

    ftmc_prf_t *ftmc = (ftmc_prf_t *)prf;

    switch (event)
    {
        case GATT_PROF_EVENT_OPENED:
        {
            ble_ftmc_con_create(conidx);
        }
        break;
        case GATT_PROF_EVENT_CLOSED:
        {
            ble_ftmc_con_cleanup(conidx);
        }
        break;
        case GATT_PROF_EVENT_SERVICE:
        {
            ble_ftmc_service_disocver_cb(prf, conidx, param.service->service, param.service->error_code);
        }
        break;
        case GATT_PROF_EVENT_CHARACTER:
        {
            gatt_profile_character_t *p = param.character;
            gatt_peer_character_t *c = p->character;
            if (p->discover_idx < FTM_CHAR_MAX_NUM)
            {
                if (p->error_code == ATT_ERROR_NO_ERROR)
                {
                    ftmc->peer_char[p->discover_idx] = c;
                }
                else
                {
                    ftmc->peer_char[p->discover_idx] = NULL;
                }
            }

            ble_ftmc_character_disocver_cb(prf, conidx, param.character->char_uuid, param.character->character,
                                          param.character->error_code, param.character->discover_cmpl);
            if (p->discover_cmpl)
            {
                if (ftmc->peer_char[FTM_CHAR_TREADMILL_DATA])
                {
                    gattc_write_cccd_descriptor(prf, ftmc->peer_char[FTM_CHAR_TREADMILL_DATA], true, false);
                    ftmc->client_cfg |= FTMS_PRF_CFG_FLAG_TREADMILL_DATA_NTF_BIT;
                }

                if (ftmc->peer_char[FTM_CHAR_CROSS_TRAINER_DATA])
                {
                    gattc_write_cccd_descriptor(prf, ftmc->peer_char[FTM_CHAR_CROSS_TRAINER_DATA], true, false);
                    ftmc->client_cfg |= FTMS_PRF_CFG_FLAG_CROSS_TRAINER_DATA_NTF_BIT;
                }

                if (ftmc->peer_char[FTM_CHAR_STEP_CLIMBER_DATA_DATA])
                {
                    gattc_write_cccd_descriptor(prf, ftmc->peer_char[FTM_CHAR_STEP_CLIMBER_DATA_DATA], true, false);
                    ftmc->client_cfg |= FTMS_PRF_CFG_FLAG_STEP_CLIMBER_DATA_NTF_BIT;
                }

                if (ftmc->peer_char[FTM_CHAR_STAIR_CLIMBER_DATA_DATA])
                {
                    gattc_write_cccd_descriptor(prf, ftmc->peer_char[FTM_CHAR_STAIR_CLIMBER_DATA_DATA], true, false);
                    ftmc->client_cfg |= FTMS_PRF_CFG_FLAG_STAIR_CLIMBER_DATA_NTF_BIT;
                }

                if (ftmc->peer_char[FTM_CHAR_ROWER_DATA])
                {
                    gattc_write_cccd_descriptor(prf, ftmc->peer_char[FTM_CHAR_ROWER_DATA], true, false);
                    ftmc->client_cfg |= FTMS_PRF_CFG_FLAG_ROWER_DATA_NTF_BIT;
                }

                if (ftmc->peer_char[FTM_CHAR_INDOOR_BIKE_DATA])
                {
                    gattc_write_cccd_descriptor(prf, ftmc->peer_char[FTM_CHAR_INDOOR_BIKE_DATA], true, false);
                    ftmc->client_cfg |= FTMS_PRF_CFG_FLAG_INDOOR_BIKE_DATA_NTF_BIT;
                }

                if (ftmc->peer_char[FTM_CHAR_TRAINING_STATUS])
                {
                    gattc_write_cccd_descriptor(prf, ftmc->peer_char[FTM_CHAR_TRAINING_STATUS], true, false);
                    ftmc->client_cfg |= FTMS_PRF_CFG_FLAG_TRAINING_STATUS_NTF_BIT;
                }

                if (ftmc->peer_char[FTM_CHAR_STATUS])
                {
                    gattc_write_cccd_descriptor(prf, ftmc->peer_char[FTM_CHAR_STATUS], true, false);
                    ftmc->client_cfg |= FTMS_PRF_CFG_FLAG_STATUS_NTF_BIT;
                }

                if (ftmc->peer_char[FTM_CHAR_CPT])
                {
                    gattc_write_cccd_descriptor(prf, ftmc->peer_char[FTM_CHAR_CPT], false, true);
                   ftmc->client_cfg |= FTMS_PRF_CFG_FLAG_CTRL_POINT_IND_BIT;
                }
            }
            break;
        }
        break;
        case GATT_PROF_EVENT_CHAR_READ_RSP:
        {
            ble_ftmc_read_cmp_cb(conidx, param.char_read_rsp->character, param.char_read_rsp->error_code, false, false);

            if (param.char_read_rsp->error_code == ATT_ERROR_NO_ERROR)
            {
                ble_ftmc_att_val_cb(conidx, param.char_read_rsp->character, false, false,
                                   param.char_read_rsp->value, param.char_read_rsp->value_len);
            }
        }
        break;
        case GATT_PROF_EVENT_NOTIFY:
        {
            gatt_profile_recv_notify_t *p = param.notify;
            gatt_peer_service_uuid_t uuid  = {0};
            gattc_get_service_uuid(p->service, &uuid);
            if (uuid.service_uuid == GATT_UUID_FIT_SERVICE)
            {
                break;
            }
            
            uint8_t ftm_char = ble_check_char_is_exist(p->conn->con_idx, p->character);
            if (ftm_char != FTM_CHAR_MAX_NUM)
            {
                // todo if ct point
                app_ftmc_parse_ctrl_pt_notification_info(p->conn->con_idx, ftm_char, p->value, p->value_len);
                // todo if data ntf
            }

            break;
        }
        case GATT_PROF_EVENT_DESC_READ_RSP:
        {
            ble_ftmc_read_cmp_cb(conidx, param.desc_read_rsp->character, param.desc_read_rsp->error_code, false, false);

            if (param.desc_read_rsp->error_code == ATT_ERROR_NO_ERROR)
            {
                ble_ftmc_att_val_cb(conidx, param.desc_read_rsp->character, true,
                                   param.desc_read_rsp->desc_uuid == GATT_DESC_UUID_CHAR_SERVER_CONFIG,
                                   param.desc_read_rsp->value, param.desc_read_rsp->value_len);
            }
        }
        break;
        case GATT_PROF_EVENT_CHAR_WRITE_RSP:
        {
            ble_ftmc_write_cmp_cb(conidx, param.char_write_rsp->character, param.char_write_rsp->error_code, false, false);
        }
        break;
        case GATT_PROF_EVENT_DESC_WRITE_RSP:
        {
            ble_ftmc_write_cmp_cb(conidx, param.char_write_rsp->character, param.char_write_rsp->error_code, true,
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

uint16_t ftmc_write_cfg_req(uint8_t conidx, uint8_t desc_code, uint16_t cfg_val)
{
    uint16_t status = BT_STS_NOT_ALLOW;
    // Client environment
    if (p_ftmc_env != NULL)
    {
        gatt_prf_t *p_prf = gattc_get_profile(p_ftmc_env->prf_lid,
                                              app_ble_get_conhdl_from_conidx(conidx));
        if ((conidx < BLE_CONNECTION_MAX) && (p_ftmc_env->p_env[conidx] != NULL) && (!p_ftmc_env->p_env[conidx]->discover))
        {
            ble_ftmc_conn_t *p_con_env = p_ftmc_env->p_env[conidx];
            gatt_peer_character_t *p_char = NULL;
            uint16_t cfg_en_val = 0;

            switch (desc_code)
            {
                case BLE_FTMC_RD_WR_TREADMILL_DATA_CL_CFG:
                {
                    p_char        = p_con_env->chars[FTM_CHAR_TREADMILL_DATA];
                    cfg_en_val =  PRF_CLI_START_NTF;
                }
                break;
                case BLE_FTMC_RD_WR_CROSS_TRAINER_CL_CFG:
                {
                    p_char        = p_con_env->chars[FTM_CHAR_CROSS_TRAINER_DATA];
                    cfg_en_val =  PRF_CLI_START_NTF;
                }
                break;
                case BLE_FTMC_RD_WR_STEP_CLIMBER_CL_CFG:
                {
                    p_char        = p_con_env->chars[FTM_CHAR_STEP_CLIMBER_DATA_DATA];
                    cfg_en_val =  PRF_CLI_START_NTF;
                }
                break;
                case BLE_FTMC_RD_WR_STAIR_CLIMBER_CL_CFG:
                {
                    p_char        = p_con_env->chars[FTM_CHAR_STAIR_CLIMBER_DATA_DATA];
                    cfg_en_val =  PRF_CLI_START_NTF;
                }
                break;
                case BLE_FTMC_RD_WR_ROWER_CL_CFG:
                {
                    p_char        = p_con_env->chars[FTM_CHAR_ROWER_DATA];
                    cfg_en_val =  PRF_CLI_START_NTF;
                }
                break;
                case BLE_FTMC_RD_WR_INDOOR_BIKE_CL_CFG:
                {
                    p_char        = p_con_env->chars[FTM_CHAR_INDOOR_BIKE_DATA];
                    cfg_en_val =  PRF_CLI_START_NTF;
                }
                break;
                case BLE_FTMC_RD_WR_TRAINING_STATUS_CL_CFG:
                {
                    p_char        = p_con_env->chars[FTM_CHAR_TRAINING_STATUS];
                    cfg_en_val =  PRF_CLI_START_NTF;
                }
                break;
                case BLE_FTMC_RD_WR_CTRL_PT_CFG:
                {
                    p_char        = p_con_env->chars[FTM_CHAR_CPT];
                    cfg_en_val =  PRF_CLI_START_IND;
                }
                break;
                case BLE_FTMC_RD_WR_FITNESS_MACHINE_STATUS_CFG:
                {
                    p_char        = p_con_env->chars[FTM_CHAR_STATUS];
                    cfg_en_val =  PRF_CLI_START_NTF;
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

uint16_t ftmc_read_cfg_req(uint8_t conidx, uint8_t desc_code)
{
    uint16_t status;

    switch (desc_code)
    {
        case BLE_FTMC_RD_WR_TREADMILL_DATA_CL_CFG:
        case BLE_FTMC_RD_WR_CROSS_TRAINER_CL_CFG:
        case BLE_FTMC_RD_WR_STEP_CLIMBER_CL_CFG:
        case BLE_FTMC_RD_WR_STAIR_CLIMBER_CL_CFG:
        case BLE_FTMC_RD_WR_ROWER_CL_CFG:
        case BLE_FTMC_RD_WR_INDOOR_BIKE_CL_CFG:
        case BLE_FTMC_RD_WR_TRAINING_STATUS_CL_CFG:
        case BLE_FTMC_RD_WR_CTRL_PT_CFG:
        case BLE_FTMC_RD_WR_FITNESS_MACHINE_STATUS_CFG:
        {
            status = ble_ftmc_read_val(conidx, desc_code);
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

void ftmc_write_req_ctrl_point(uint16_t connhdl, uint8_t opcode, const uint8_t *value, uint16_t len)
{
    ftmc_prf_t *prf = NULL;
    gatt_peer_character_t *c = NULL;

    if (value == NULL || len == 0)
    {
        return;
    }

    prf = (ftmc_prf_t *)gattc_get_profile(p_ftmc_env->prf_lid, connhdl);
    if (prf == NULL)
    {
        return;
    }

    c = prf->peer_char[FTM_CHAR_CPT];
    if (c == NULL)
    {
        return;
    }

    ftm_cpt_opcode opcode_data;

    if(opcode == CTRL_PT_REQUEST || opcode == CTRL_PT_RESET || opcode == CTRL_PT_SET_TARGET_STARTRESU)
    {
        opcode_data.opcode = opcode;
        len += sizeof(uint8_t);
    }
    else if(opcode == CTRL_PT_SET_TARGET_SPEED || opcode == CTRL_PT_SET_TARGET_EXPENERGY 
            || opcode == CTRL_PT_SET_TARGET_NUMSTEPS || opcode == CTRL_PT_SET_TARGET_NUMSTRIDS 
            || opcode == CTRL_PT_SET_TARGET_TRAINTIME || opcode == CTRL_PT_SET_WHEEL_CIRCUMFER
            || opcode == CTRL_PT_SET_TARGETED_CADENCE)
    {
        opcode_data.opcode = opcode;
        opcode_data.data.paramu16_t = CO_COMBINE_UINT16_LE(value);
        value += 2;
        len += sizeof(uint16_t);
    }
    else if(opcode == CTRL_PT_SET_TARGET_RESISLEVEL || opcode == CTRL_PT_SET_TARGET_HEARTRATE || opcode == CTRL_PT_SET_TARGET_STOPPAUSE)
    {
        opcode_data.opcode = opcode;
        opcode_data.data.paramu8_t = value[0];
        value += 1;
        len += sizeof(uint8_t);
    }
    else if(opcode == CTRL_PT_SET_TARGET_INCLINATION || opcode == CTRL_PT_SET_TARGET_POWER)
    {
        opcode_data.opcode = opcode;
        opcode_data.data.param16_t = CO_COMBINE_UINT16_LE(value);
        value += 2;
        len += sizeof(int16_t);
    }
    else if(opcode == CTRL_PT_SET_TARGET_DISTANCE)
    {
        opcode_data.opcode = opcode;
        opcode_data.data.paramu32_t = CO_COMBINE_UINT32_LE(value);
        value += 4;
        len += sizeof(uint32_t);
    }
    else if(opcode == CTRL_PT_SET_TARGET_DISTANCE)
    {
        opcode_data.opcode = opcode;
        opcode_data.data.paramu32_t = CO_COMBINE_UINT32_LE(value);
        value += 4;
        len += sizeof(uint32_t);
    }
    else if(opcode == CTRL_PT_SET_TARGET_TIMEINTWOHR)
    {
        opcode_data.opcode = opcode;
        target_training_time_two_heart_rate_param *param = (target_training_time_two_heart_rate_param *)value;
        opcode_data.data.tws_hr.fat_burn_time = param->fat_burn_time;
        opcode_data.data.tws_hr.fitness_time = param->fitness_time;
        value += sizeof(target_training_time_two_heart_rate_param);
        len += sizeof(target_training_time_two_heart_rate_param);
    }
    else if(opcode == CTRL_PT_SET_TARGET_TIMEINTHREEHR)
    {
        opcode_data.opcode = opcode;
        target_training_time_three_heart_rate_param *param = (target_training_time_three_heart_rate_param *)value;
        opcode_data.data.three_hr.light_time = param->light_time;
        opcode_data.data.three_hr.moderate_time = param->moderate_time;
        opcode_data.data.three_hr.hard_time = param->hard_time;
        value += sizeof(target_training_time_three_heart_rate_param);
        len += sizeof(target_training_time_three_heart_rate_param);
    }
    else if(opcode == CTRL_PT_SET_TARGET_TIMEINFIVEHR)
    {
        opcode_data.opcode = opcode;
        target_training_time_five_heart_rate_param *param = (target_training_time_five_heart_rate_param *)value;
        opcode_data.data.five_hr.very_light_time = param->very_light_time;
        opcode_data.data.five_hr.light_time = param->light_time;
        opcode_data.data.five_hr.moderate_time = param->moderate_time;
        opcode_data.data.five_hr.hard_time = param->hard_time;
        opcode_data.data.five_hr.max_time = param->max_time;
        value += sizeof(target_training_time_five_heart_rate_param);
        len += sizeof(target_training_time_five_heart_rate_param);
    }
    else if(opcode == CTRL_PT_SET_INDOOR_BIKESIMUL)
    {
        opcode_data.opcode = opcode;
        indoor_bike_simulation_param *param = (indoor_bike_simulation_param *)value;
        opcode_data.data.indoor_bike.crr = param->crr;
        opcode_data.data.indoor_bike.cw = param->cw;
        opcode_data.data.indoor_bike.wind_speed = param->wind_speed;
        opcode_data.data.indoor_bike.grade = param->grade;
        value += sizeof(indoor_bike_simulation_param);
        len += sizeof(indoor_bike_simulation_param);
    }

    gattc_write_character_value((gatt_prf_t *)prf, c, (uint8_t *)&opcode_data, len);
}

void ftmc_read_req(uint16_t connhdl, uint8_t char_enum)
{
    ftmc_prf_t *prf = NULL;
    gatt_peer_character_t *c = NULL;

    if (char_enum >= FTM_CHAR_MAX_NUM)
    {
        return;
    }

    prf = (ftmc_prf_t *)gattc_get_profile(p_ftmc_env->prf_lid, connhdl);
    if (prf == NULL)
    {
        return;
    }

    c = prf->peer_char[char_enum];
    if (c == NULL)
    {
        return;
    }

    if (char_enum == FTM_CHAR_FEATURE || char_enum >= FTM_CHAR_TRAINING_STATUS || char_enum <= FTM_CHAR_SUPP_HEART_RATE_RANGE)
    {
        gattc_read_descriptor_value(&prf->head, c, GATT_UUID_CHAR_DECLARE);
    }
    else
    {
        gattc_read_descriptor_value(&prf->head, c, GATT_DESC_UUID_CHAR_CLIENT_CONFIG);
    }
}

bt_status_t ble_ftmc_start_discover(uint16_t connhdl)
{
    uint16_t status = BT_STS_NOT_ALLOW;
    // Client environment
    if (p_ftmc_env != NULL)
    {
        gatt_prf_t *p_prf = gattc_get_profile(p_ftmc_env->prf_lid, connhdl);
        if ((p_prf->con_idx < BLE_CONNECTION_MAX) && (p_ftmc_env->p_env[p_prf->con_idx] == NULL) && p_prf != NULL)
        {
            // allocate environment variable for task instance
            p_ftmc_env->p_env[p_prf->con_idx] = (ble_ftmc_conn_t *) bes_bt_me_bes_bt_buf_malloc(sizeof(ble_ftmc_conn_t));

            if (p_ftmc_env->p_env[p_prf->con_idx] != NULL)
            {
                memset(p_ftmc_env->p_env[p_prf->con_idx], 0, sizeof(ble_ftmc_conn_t));
                co_timer_new(&(p_ftmc_env->p_env[p_prf->con_idx]->timer), FTM_CP_TIMEOUT, ble_ftmc_timer_handler,
                             (void *)((uint32_t) p_prf->con_idx), 1);

                // start discovery
                status = gattc_discover_service(p_prf, GATT_UUID_FIT_SERVICE, NULL);

                // Go to DISCOVERING state
                p_ftmc_env->p_env[p_prf->con_idx]->discover   = true;
                p_ftmc_env->p_env[p_prf->con_idx]->ctrl_pt_op = CTRL_PT_RESERVED;
            }
            else
            {
                status = BT_STS_NO_RESOURCES;
            }
        }
    }

    return status;
}

uint16_t ble_ftmc_init(const ble_ftmc_cb_t *p_cb)
{
    uint8_t conidx;
    // DB Creation Status
    uint16_t status = BT_STS_SUCCESS;
    uint8_t prf_lid = 0xFF;

    if (p_ftmc_env != NULL)
    {
        return BT_STS_ALREADY_EXIST;
    }

    do
    {
        if ((p_cb == NULL) || (p_cb->cb_svc_discover_cmp == NULL) || (p_cb->cb_read_char_cmp == NULL)
                || (p_cb->cb_read_cfg_cmp == NULL) || (p_cb->cb_write_cfg_cmp == NULL)
                || (p_cb->cb_ctrl_pt_req_cmp == NULL))
        {
            status = BT_STS_INVALID_PARM;
            break;
        }

        gattc_cfg_t init_cfg =
        {
            .preferred_mtu = FTMS_CP_NTF_MAX_LEN + 3,
        };

        // register ftmc user
        prf_lid = gattc_register_profile(ble_ftmc_client_callback, &init_cfg);

        if (prf_lid == 0xFF)
        {
            status = BT_STS_FAILED;
            break;
        }

        p_ftmc_env = (ble_ftmc_env_t *) bes_bt_me_bes_bt_buf_malloc(sizeof(ble_ftmc_env_t));

        if (p_ftmc_env != NULL)
        {
            // initialize environment variable
            p_ftmc_env->p_cbs    = p_cb;
            p_ftmc_env->prf_lid = prf_lid;
            for (conidx = 0; conidx < BLE_CONNECTION_MAX; conidx++)
            {
                p_ftmc_env->p_env[conidx] = NULL;
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

uint16_t ble_ftmc_deinit(void)
{
    if (p_ftmc_env != NULL)
    {

        gattc_unregister_profile(p_ftmc_env->prf_lid);

        uint8_t conidx;

        // cleanup environment variable for each task instances
        for (conidx = 0; conidx < BLE_CONNECTION_MAX; conidx++)
        {
            if (p_ftmc_env->p_env[conidx] != NULL)
            {
                co_timer_stop(&(p_ftmc_env->p_env[conidx]->timer));
                bes_bt_me_bes_bt_buf_free((uint8_t *)p_ftmc_env->p_env[conidx]);
            }
        }

        bes_bt_me_bes_bt_buf_free((uint8_t *)p_ftmc_env);

        p_ftmc_env = NULL;
    }

    return BT_STS_SUCCESS;
}
#endif /* BLE_FTMC_ENABLED */
