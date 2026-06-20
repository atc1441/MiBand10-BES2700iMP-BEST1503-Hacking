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

#ifdef BLE_HRPS_ENABLED

#include "ble_hrps.h"

#include "gatt_service.h"
#include "bluetooth.h"
#include "app_ble.h"


/********************************************
 ******* HRPS Configuration Flag Masks ******
 ********************************************/
#define HRPS_MANDATORY_MASK                 (0x0F)
#define HRPS_BODY_SENSOR_LOC_MASK           (0x30)
#define HRPS_HR_CTNL_PT_MASK                (0xC0)

#define HRPS_HT_MEAS_MAX_LEN                (5 + (2 * HRS_MAX_RR_INTERVAL))
#define HRP_PRF_CFG_PERFORMED_OK            (0x80)
#define HRPS_IS_SUPPORTED(features, mask)   ((features & mask) == mask)

/// Feature bit field
enum hrps_feat_bf
{
    /// Body Sensor Location Feature Supported
    HRPS_BODY_SENSOR_LOC_CHAR_SUP_POS = 0,
    HRPS_BODY_SENSOR_LOC_CHAR_SUP_BIT = CO_BIT(HRPS_BODY_SENSOR_LOC_CHAR_SUP_POS),

    /// Energy Expanded Feature Supported
    HRPS_ENGY_EXP_FEAT_SUP_POS = 1,
    HRPS_ENGY_EXP_FEAT_SUP_BIT = CO_BIT(HRPS_ENGY_EXP_FEAT_SUP_POS),

    /// Heart Rate Measurement Notification Supported
    HRPS_HR_MEAS_NTF_CFG_POS = 2,
    HRPS_HR_MEAS_NTF_CFG_BIT = CO_BIT(HRPS_HR_MEAS_NTF_CFG_POS),
};

/// ongoing operation information
typedef struct hrps_op_ctx
{
    struct list_node node_hdr;
    /// meaningful for some operation
    uint32_t  conidx_bf;
    /// Operation
    uint8_t   operation;
    /// Connection index targeted
    uint8_t   conidx;
} hrps_op_ctx_t;

/// Heart rate sensor server environment variable
typedef struct hrps_env
{
    const hrps_cb_t *p_upper_cb;
    /// Operation Event TX wait queue
    struct list_node wait_queue;
    /// Services features
    uint16_t  features;
    /// Sensor location
    uint8_t   sensor_loc;
    /// Operation On-going
    bool      op_ongoing;
    /// Prevent recursion in execute_operation function
    bool      in_exe_op;
    /// Measurement notification config
    uint16_t  hr_meas_ntf[BLE_CONNECTION_MAX];
} hrps_env_t;

__STATIC hrps_env_t *p_hrps_env = NULL;


#define HRPS_GATT_ATTRIBUTE_MAX_LEN (5)

GATT_DECL_PRI_SERVICE(g_ble_hr_service, GATT_UUID_HRP_SERVICE);

GATT_DECL_CHAR(g_ble_hr_measurement, GATT_CHAR_UUID_HEART_RATE_MEASURE,
                GATT_NTF_PROP,
                ATT_SEC_NONE);
GATT_DECL_CCCD_DESCRIPTOR(g_ble_hr_measurement_cccd, ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_hr_sensor_location, GATT_CHAR_UUID_BODY_SENSOR_LOCATION,
                GATT_RD_REQ,
                ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_hr_control_point, GATT_CHAR_UUID_HEART_RATE_CTRL_POINT,
                GATT_WR_CMD | GATT_WR_REQ,
                ATT_SEC_NONE);

__STATIC gatt_attribute_t hrps_hrs_att_list[HRPS_GATT_ATTRIBUTE_MAX_LEN] =
{
    /* Service */
    gatt_attribute(g_ble_hr_service),
    /* Characteristics */
    gatt_attribute(g_ble_hr_measurement),
    gatt_attribute(g_ble_hr_measurement_cccd),

    /* reserve for optional char */
};

__STATIC uint16_t hrps_prepare_service_attr_list(uint32_t cfg_flag, gatt_attribute_t **pp_attr_list, uint8_t *p_attr_list_len)
{
    if ((cfg_flag & HRPS_MANDATORY_MASK) != HRPS_MANDATORY_MASK)
    {
        return BT_STS_INVALID_PARM;
    }

    *p_attr_list_len = 3;

    if ((cfg_flag & HRPS_BODY_SENSOR_LOC_MASK) == HRPS_BODY_SENSOR_LOC_MASK)
    {
        gatt_attribute_t sen_loc = gatt_attribute(g_ble_hr_sensor_location);
        hrps_hrs_att_list[(*p_attr_list_len)++] = sen_loc;
    }
    if ((cfg_flag & HRPS_HR_CTNL_PT_MASK) == HRPS_HR_CTNL_PT_MASK)
    {
        gatt_attribute_t ctnl_pt = gatt_attribute(g_ble_hr_control_point);
        hrps_hrs_att_list[(*p_attr_list_len)++] = ctnl_pt;
    }

    *pp_attr_list = hrps_hrs_att_list;
    return BT_STS_SUCCESS;
}

__STATIC void hrps_exe_operation()
{
    uint16_t status = BT_STS_SUCCESS;
    uint8_t conidx = GAP_INVALID_CONIDX;

    if (!p_hrps_env->in_exe_op)
    {
        p_hrps_env->in_exe_op = true;

        while (!colist_is_list_empty(&(p_hrps_env->wait_queue)) && !(p_hrps_env->op_ongoing))
        {
            struct list_node *node = colist_get_head(&(p_hrps_env->wait_queue));
            if (node == NULL)
            {
                p_hrps_env->in_exe_op = false;
                return;
            }
            hrps_op_ctx_t *p_op_ctx = colist_structure(node, hrps_op_ctx_t, node_hdr);
            colist_delete(node);

            switch (p_op_ctx->operation)
            {
                case HRPS_MEAS_SEND_CMD_OP_CODE:
                {
                    uint32_t conidx_bf = 0;
                    // check connection that support notification reception
                    for (conidx = 0 ; conidx < BLE_CONNECTION_MAX ; conidx++)
                    {
                        if ((p_hrps_env->hr_meas_ntf[conidx] & ~HRP_PRF_CFG_PERFORMED_OK) == PRF_CLI_START_NTF)
                        {
                            conidx_bf |= CO_BIT(conidx);
                        }
                    }

                    // send notification only on selected connections
                    conidx_bf &= p_op_ctx->conidx_bf;

                    if (conidx_bf != 0)
                    {
                        gatt_char_notify_t char_notify =
                        {
                            .service = g_ble_hr_service,
                            .service_inst_id = 0,
                            .char_instance_id = 0,
                            .character = g_ble_hr_measurement,
                        };

                        // send multi-point notification
                        status = gatts_send_value_notification(conidx_bf, &char_notify, (uint8_t *)(p_op_ctx + 1), strlen((char *)(p_op_ctx + 1)));
                        if(status == BT_STS_SUCCESS)
                        {
                            p_hrps_env->op_ongoing = true;
                        }
                    }

                    if (!p_hrps_env->op_ongoing)
                    {
                        // Inform application that event has been sent
                        p_hrps_env->p_upper_cb->cb_meas_send_cmp(status);
                    }
                } break;
                default: { ASSERT_ERR(0); } break;
            }

            bes_bt_buf_free(p_op_ctx);
        }

        p_hrps_env->in_exe_op = false;
    }
}

__STATIC void hrps_pack_meas(uint8_t *p_buf, const hrs_hr_meas_t *p_meas)
{
    uint8_t *p_buf_end = p_buf + HRPS_HT_MEAS_MAX_LEN;

    *p_buf++ = p_meas->flags;

    // Heart Rate Measurement Value 16 bits or 8 bits
    if (GETB(p_meas->flags, HRS_FLAG_HR_VALUE_FORMAT))
    {
        *(uint16_t *)p_buf = co_host_to_uint16_le(p_meas->heart_rate);
        p_buf += 2;
    }
    else
    {
        *p_buf++ = p_meas->heart_rate;
    }

    // Energy Expended present
    if (GETB(p_meas->flags, HRS_FLAG_ENERGY_EXPENDED_PRESENT))
    {
        *(uint16_t *)p_buf = co_host_to_uint16_le(p_meas->energy_expended);
        p_buf += 2;
    }

    // RR-Intervals
    if (GETB(p_meas->flags, HRS_FLAG_RR_INTERVAL_PRESENT))
    {
        uint8_t cursor;
        for (cursor = 0; (cursor < (p_meas->nb_rr_interval)) && (cursor < (HRS_MAX_RR_INTERVAL)); cursor++)
        {
            *(uint16_t *)p_buf = co_host_to_uint16_le(p_meas->rr_intervals[cursor]);
            p_buf += 2;
        }
    }

    if (p_buf > p_buf_end)
    {
        TRACE(0, "%s error!", __func__);
    }
}

uint16_t ble_hrps_enable(uint8_t conidx, uint16_t hr_meas_ntf)
{
    if (p_hrps_env != NULL)
    {
        // check state of the task
        if (app_ble_get_conhdl_from_conidx(conidx) != GAP_INVALID_CONN_HANDLE)
        {
            // Bonded data was not used before
            if (!HRPS_IS_SUPPORTED(p_hrps_env->hr_meas_ntf[conidx], HRP_PRF_CFG_PERFORMED_OK))
            {
                // Save notification config
                p_hrps_env->hr_meas_ntf[conidx] = hr_meas_ntf;
                // Enable Bonded Data
                p_hrps_env->hr_meas_ntf[conidx] |= HRP_PRF_CFG_PERFORMED_OK;
            }
            return BT_STS_SUCCESS;
        }
    }

    return BT_STS_FAILED;
}

uint16_t ble_hrps_meas_send(uint32_t conidx_bf, const hrs_hr_meas_t* p_meas)
{
    if ((p_meas == NULL) || (p_meas->nb_rr_interval > HRS_MAX_RR_INTERVAL))
    {
        return BT_STS_INVALID_CHAR_VALUE;
    }
    else if (p_hrps_env != NULL)
    {
        hrps_op_ctx_t *p_op_ctx = (hrps_op_ctx_t *)bes_bt_buf_malloc(sizeof(hrps_op_ctx_t) + HRPS_HT_MEAS_MAX_LEN);
        if (p_op_ctx == NULL)
        {
            return BT_STS_NO_RESOURCES;
        }
        memset(p_op_ctx, 0, sizeof(hrps_op_ctx_t) + HRPS_HT_MEAS_MAX_LEN);
        INIT_LIST_HEAD(&p_op_ctx->node_hdr);
        p_op_ctx->conidx_bf = conidx_bf & ((1 << BLE_CONNECTION_MAX) - 1);
        p_op_ctx->operation = HRPS_MEAS_SEND_CMD_OP_CODE;
        p_op_ctx->conidx    = GAP_INVALID_CONIDX;

        // pack measurement
        hrps_pack_meas((uint8_t*)(p_op_ctx + 1), p_meas);
        // put event on wait queue
        colist_addto_tail(&(p_op_ctx->node_hdr), &(p_hrps_env->wait_queue));
        hrps_exe_operation();
        return BT_STS_SUCCESS;
    }

    return BT_STS_FAILED;
}


__STATIC int app_ble_hrp_server_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
{
    uint8_t conidx = GAP_INVALID_CONIDX;
    TRACE(0, "%s event=%d", __func__, event);

    if (p_hrps_env == NULL)
    {
        return BT_STS_NOT_READY;
    }

    switch (event)
    {
        case GATT_SERV_EVENT_CHAR_READ:
        {
            gatt_server_char_read_t *p = param.char_read;
            p->rsp_error_code = ATT_ERROR_NO_ERROR;
            if (p->character == g_ble_hr_sensor_location)
            {
                uint8_t sensor_loc = p_hrps_env->sensor_loc;
                gatts_write_read_rsp_data(p->ctx, &sensor_loc, sizeof(uint8_t));
            }
            else
            {
                TRACE(0, "%s read error char", __func__);
                p->rsp_error_code = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
            }
            return (p->rsp_error_code == ATT_ERROR_NO_ERROR ? true : false);
        }
        case GATT_SERV_EVENT_DESC_READ:
        {
            gatt_server_desc_read_t *p = param.desc_read;
            p->rsp_error_code = ATT_ERROR_NO_ERROR;
            conidx = p->con_idx;
            if ((uint8_t *)p->desc_attr->attr_data == g_ble_hr_measurement_cccd)
            {
                uint16_t config = (p_hrps_env->hr_meas_ntf[conidx] & ~HRP_PRF_CFG_PERFORMED_OK);
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)&config, sizeof(uint16_t));
            }
            else
            {
                TRACE(0, "%s read error desc", __func__);
                p->rsp_error_code = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
            }
            return (p->rsp_error_code == ATT_ERROR_NO_ERROR ? true : false);
        }
        case GATT_SERV_EVENT_CHAR_WRITE:
        {
            gatt_server_char_write_t *p = param.char_write;
            p->rsp_error_code = ATT_ERROR_NO_ERROR;
            conidx = p->con_idx;
            if (p->value_offset != 0 || p->value_len == 0 || p->value == NULL)
            {
                p->rsp_error_code = ATT_ERROR_INVALID_VALUE_LEN;
            }
            if (p->character == g_ble_hr_control_point)
            {
                if (GETB(p_hrps_env->features, HRPS_ENGY_EXP_FEAT_SUP))
                {
                    uint8_t value = *(p->value);
                    if (value == 0x01 && p->value_len >= 0)
                    {
                        // inform application that energy reset is requested
                        p_hrps_env->p_upper_cb->cb_energy_exp_reset(conidx);
                    }
                    else
                    {
                        p->rsp_error_code = ATT_ERROR_VALUE_NOT_ALLOWED;
                    }
                }
                else
                {
                    // Allowed to send Application Error if value inconvenient
                    TRACE(0, "%s not suport", __func__);
                    p->rsp_error_code = ATT_ERROR_WR_NOT_PERMITTED;
                }
            }
            else
            {
                p->rsp_error_code = ATT_ERROR_WR_NOT_PERMITTED;
            }
            gatts_send_write_rsp(p->ctx, p->rsp_error_code);
            return (p->rsp_error_code == ATT_ERROR_NO_ERROR ? true : false);
        }
        case GATT_SERV_EVENT_DESC_WRITE:
        {
            gatt_server_desc_write_t *p = param.desc_write;
            p->rsp_error_code = ATT_ERROR_NO_ERROR;
            conidx = p->con_idx;
            uint16_t config = CO_COMBINE_UINT16_LE(p->value);
            if ((uint8_t *)p->desc_attr->attr_data == g_ble_hr_measurement_cccd)
            {
                if ((p->value_len == sizeof(uint16_t)) &&
                    ((config == PRF_CLI_STOP_NTFIND) || (config == PRF_CLI_START_NTF)))
                {
                    p_hrps_env->hr_meas_ntf[conidx] = (config | HRP_PRF_CFG_PERFORMED_OK);
                    // inform application about update
                    p_hrps_env->p_upper_cb->cb_bond_data_upd(conidx, config);
                }
                else
                {
                    p->rsp_error_code = ATT_ERROR_VALUE_NOT_ALLOWED;
                }
            }
            else
            {
                p->rsp_error_code = ATT_ERROR_WR_NOT_PERMITTED;
            }
            gatts_send_write_rsp(p->ctx, p->rsp_error_code);
            return (p->rsp_error_code == ATT_ERROR_NO_ERROR ? true : false);
        }
        case GATT_SERV_EVENT_NTF_TX_DONE:
        {
            gatt_server_indicate_cfm_t *p = param.confirm;
            p_hrps_env->op_ongoing = false;
            // inform application that measurement has been sent
            p_hrps_env->p_upper_cb->cb_meas_send_cmp(p->error_code);
            // continue operation execution
            hrps_exe_operation();
            return true;
        }
        case GATT_SERV_EVENT_INDICATE_CFM:
        case GATT_SERV_EVENT_REGISTER_CMP:
        case GATT_SERV_EVENT_CONN_OPENED:
        case GATT_SERV_EVENT_CONN_CLOSED:
        case GATT_SERV_EVENT_MTU_CHANGED:
        case GATT_SERV_EVENT_CONN_UPDATED:
        case GATT_SERV_EVENT_CONN_ENCRYPTED:
        {
            return true;
        }
        default:
        {
            break;
        }
    }

    return false;
}

uint16_t ble_hrps_init(const struct hrps_init_cfg *hrps_cfg, const hrps_cb_t* hrps_upper_cb)
{
    uint16_t status = BT_STS_SUCCESS;
    uint32_t cfg_flag = HRPS_MANDATORY_MASK;
    gatt_attribute_t *p_attr_list = NULL;
    uint8_t p_attr_list_len = 0;

    if (p_hrps_env != NULL)
    {
        return BT_STS_IN_USE;
    }

    if(hrps_cfg == NULL || hrps_upper_cb == NULL ||
       hrps_upper_cb->cb_meas_send_cmp == NULL ||
       hrps_upper_cb->cb_bond_data_upd == NULL ||
       hrps_upper_cb->cb_energy_exp_reset == NULL)
    {
        return BT_STS_INVALID_PARM;
    }

    // Set Configuration Flag Value
    if (GETB(hrps_cfg->features, HRPS_BODY_SENSOR_LOC_CHAR_SUP))
    {
        cfg_flag |= HRPS_BODY_SENSOR_LOC_MASK;
    }
    if (GETB(hrps_cfg->features, HRPS_ENGY_EXP_FEAT_SUP))
    {
        cfg_flag |= HRPS_HR_CTNL_PT_MASK;
    }

    status = hrps_prepare_service_attr_list(cfg_flag, &p_attr_list, &p_attr_list_len);
    if (status != BT_STS_SUCCESS)
    {
        return status;
    }

    // Malloc for env
    p_hrps_env = (hrps_env_t *)bes_bt_buf_malloc(sizeof(hrps_env_t));
    if (p_hrps_env == NULL)
    {
        return BT_STS_NO_RESOURCES;
    }
    memset(p_hrps_env, 0, sizeof(hrps_env_t));

    p_hrps_env->p_upper_cb  = hrps_upper_cb;
    p_hrps_env->features    = hrps_cfg->features;
    p_hrps_env->sensor_loc  = hrps_cfg->body_sensor_loc;
    p_hrps_env->op_ongoing  = false;
    p_hrps_env->in_exe_op   = false;
    INIT_LIST_HEAD(&(p_hrps_env->wait_queue));

    // Add service to gatt db and register callback(s)
    gatts_cfg_t svc_cfg =
    {
        .service_inst_id = 0,
        .svc_size = sizeof(gatt_svc_t),
    };

    status = gatts_register_service(p_attr_list,  p_attr_list_len, app_ble_hrp_server_callback, &svc_cfg);

    if (status != BT_STS_SUCCESS)
    {
        bes_bt_buf_free(p_hrps_env);
        p_hrps_env = NULL;
    }

    return status;
}

uint16_t ble_hrps_deinit()
{
    if (p_hrps_env == NULL)
    {
        return BT_STS_FAILED;
    }

    gatts_unregister_service(hrps_hrs_att_list);

    while(!colist_is_list_empty(&(p_hrps_env->wait_queue)))
    {
        struct list_node *node = colist_get_head(&(p_hrps_env->wait_queue));
        if (node)
        {
            hrps_op_ctx_t *p_op_ctx = colist_structure(node, hrps_op_ctx_t, node_hdr);
            colist_delete(node);
            bes_bt_buf_free(p_op_ctx);
        }
    }

    bes_bt_buf_free(p_hrps_env);
    return BT_STS_SUCCESS;
}

#endif /* BLE_HRPS_ENABLED */
