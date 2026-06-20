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

#ifdef BLE_CSCPS_ENABLED

#include "ble_cscps.h"

#include "gatt_service.h"
#include "bluetooth.h"
#include "app_ble.h"


/********************************************
 ******* CSCPS Configuration Flag Masks *****
 ********************************************/
/// Mandatory Attributes (CSC Measurement + CSC Feature)
#define CSCPS_MANDATORY_MASK               (0x003F)
/// Sensor Location Attributes
#define CSCPS_SENSOR_LOC_MASK              (0x00C0)
/// SC Control Point Attributes
#define CSCPS_SC_CTNL_PT_MASK              (0x0700)

/// Profile Configuration Additional Flags bit field
enum cscps_prf_cfg_flag_bf
{
    /// CSC Measurement - Client Char. Cfg
    CSCP_PRF_CFG_FLAG_CSC_MEAS_NTF_POS = 3,
    CSCP_PRF_CFG_FLAG_CSC_MEAS_NTF_BIT = CO_BIT(CSCP_PRF_CFG_FLAG_CSC_MEAS_NTF_POS),

    /// SC Control Point - Client Char. Cfg
    CSCP_PRF_CFG_FLAG_SC_CTNL_PT_IND_POS = 4,
    CSCP_PRF_CFG_FLAG_SC_CTNL_PT_IND_BIT = CO_BIT(CSCP_PRF_CFG_FLAG_SC_CTNL_PT_IND_POS),

    /// Bonded data used
    CSCP_PRF_CFG_PERFORMED_OK_POS = 7,
    CSCP_PRF_CFG_PERFORMED_OK_BIT = CO_BIT(CSCP_PRF_CFG_PERFORMED_OK_POS),
};

/// Sensor Location Supported Flag
enum cscps_sensor_loc_supp
{
    /// Sensor Location Char. is not supported
    CSCPS_SENSOR_LOC_NOT_SUPP,
    /// Sensor Location Char. is supported
    CSCPS_SENSOR_LOC_SUPP,
};

#define CSCPS_IS_FEATURE_SUPPORTED(features, flag) ((features & flag) == flag)

/// ongoing operation information
typedef struct cscps_buf_meta
{
    struct list_node node_hdr;
    /// meaningful for some operation
    uint32_t  conidx_bf;
    /// Operation
    uint8_t   operation;
    /// Connection index targeted
    uint8_t   conidx;
} cscps_buf_meta_t;

/// cycling speed and cadence server environment variable
typedef struct cscps_env
{
    const cscps_cb_t *p_upper_cb;
    /// Operation Event TX wait queue
    struct list_node wait_queue;
    /// Wheel revolution
    uint32_t  tot_wheel_rev;
    /// Services features
    uint16_t  features;
    /// profile configuration
    uint16_t  prfl_cfg;
    /// Sensor location
    uint8_t   sensor_loc;
    /// Control point operation on-going (see enum #cscp_sc_ctnl_pt_op_code)
    uint8_t   ctrl_pt_op;
    /// Operation On-going
    bool      op_ongoing;
    /// Prevent recursion in execute_operation function
    bool      in_exe_op;
    /// Environment variable pointer for each connections
    uint8_t   prfl_ntf_ind_cfg[BLE_CONNECTION_MAX];

} cscps_env_t;

__STATIC cscps_env_t *p_cscps_env = NULL;


#define CSCPS_GATT_ATTRIBUTE_MAX_LEN (7)

GATT_DECL_PRI_SERVICE(g_ble_csc_service, GATT_UUID_CSC_SERVICE);

GATT_DECL_CHAR(g_ble_csc_measurement, GATT_CHAR_UUID_CSC_MEASURE,
                GATT_NTF_PROP,
                ATT_SEC_NONE);
GATT_DECL_CCCD_DESCRIPTOR(g_ble_csc_measurement_cccd, ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_csc_feature, GATT_CHAR_UUID_CSC_FEATURE,
                GATT_RD_REQ,
                ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_csc_sensor_location, GATT_CHAR_UUID_SENS_OR_LOCATION,
                GATT_RD_REQ,
                ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_csc_control_point, GATT_CHAR_UUID_SC_CTRL_POINT,
                GATT_WR_CMD | GATT_WR_REQ | GATT_IND_PROP,
                ATT_SEC_NONE);
GATT_DECL_CCCD_DESCRIPTOR(g_ble_csc_control_point_cccd, ATT_SEC_NONE);

__STATIC gatt_attribute_t cscps_hrs_att_list[CSCPS_GATT_ATTRIBUTE_MAX_LEN] =
{
    gatt_attribute(g_ble_csc_service),

    gatt_attribute(g_ble_csc_measurement),
    gatt_attribute(g_ble_csc_measurement_cccd),

    gatt_attribute(g_ble_csc_feature),

    // reserve for optional char
};

__STATIC uint16_t cscps_prepare_service_attr_list(uint32_t cfg_flag, gatt_attribute_t **pp_attr_list, uint8_t *p_attr_list_len)
{
    if ((cfg_flag & CSCPS_MANDATORY_MASK) != CSCPS_MANDATORY_MASK)
    {
        return BT_STS_INVALID_PARM;
    }

    *p_attr_list_len = 4;

    if ((cfg_flag & CSCPS_SENSOR_LOC_MASK) == CSCPS_SENSOR_LOC_MASK)
    {
        gatt_attribute_t sen_loc = gatt_attribute(g_ble_csc_sensor_location);
        cscps_hrs_att_list[(*p_attr_list_len)++] = sen_loc;
    }
    if ((cfg_flag & CSCPS_SC_CTNL_PT_MASK) == CSCPS_SC_CTNL_PT_MASK)
    {
        gatt_attribute_t ctnl_pt = gatt_attribute(g_ble_csc_control_point);
        gatt_attribute_t ctnl_pt_cccd = gatt_attribute(g_ble_csc_control_point_cccd);
        cscps_hrs_att_list[(*p_attr_list_len)++] = ctnl_pt;
        cscps_hrs_att_list[(*p_attr_list_len)++] = ctnl_pt_cccd;
    }

    *pp_attr_list = cscps_hrs_att_list;
    return BT_STS_SUCCESS;
}

__STATIC void cscps_exe_operation()
{
    uint16_t status = BT_STS_SUCCESS;
    uint8_t  conidx = GAP_INVALID_CONIDX;

    if(!p_cscps_env->in_exe_op)
    {
        p_cscps_env->in_exe_op = true;

        while(!colist_is_list_empty(&(p_cscps_env->wait_queue)) && !(p_cscps_env->op_ongoing))
        {
            struct list_node *node = colist_get_head(&(p_cscps_env->wait_queue));
            if (node == NULL)
            {
                p_cscps_env->in_exe_op = false;
                return;
            }
            cscps_buf_meta_t* p_meta = colist_structure(node, cscps_buf_meta_t, node_hdr);
            colist_delete(node);

            switch(p_meta->operation)
            {
                case CSCPS_SEND_CSC_MEAS_OP_CODE:
                {
                    uint32_t conidx_bf = 0;

                    // check connection that support notification reception
                    for(conidx = 0 ; conidx < BLE_CONNECTION_MAX ; conidx++)
                    {
                        if(GETB(p_cscps_env->prfl_ntf_ind_cfg[conidx], CSCP_PRF_CFG_FLAG_CSC_MEAS_NTF))
                        {
                            conidx_bf |= CO_BIT(conidx);
                        }
                    }

                    // send notification only on selected connections
                    conidx_bf &= p_meta->conidx_bf;

                    if(conidx_bf != 0)
                    {
                        gatt_char_notify_t char_notify =
                        {
                            .service = g_ble_csc_service,
                            .service_inst_id = 0,       /* only one service and one char for client */
                            .char_instance_id = 0,
                            .character = g_ble_csc_measurement,
                        };

                        // send multi-point notification
                        status = gatts_send_value_notification(conidx_bf, &char_notify, (uint8_t *)(p_meta + 1),
                                                               strlen((char *)(p_meta + 1)));
                        if(status == BT_STS_SUCCESS)
                        {
                            p_cscps_env->op_ongoing = true;
                        }
                    }

                    bes_bt_buf_free(p_meta);

                    if(!p_cscps_env->op_ongoing)
                    {
                        const cscps_cb_t* p_cb = (const cscps_cb_t*)p_cscps_env->p_upper_cb;
                        // Inform application that event has been sent
                        p_cb->cb_meas_send_cmp(status);
                    }
                } break;
                default:
                {
                    conidx = p_meta->conidx;

                    gatt_char_notify_t char_indicate =
                    {
                        .service = g_ble_csc_service,
                        .service_inst_id = 0,       /* only one service and one char for client */
                        .char_instance_id = 0,
                        .character = g_ble_csc_control_point,
                    };

                    status = gatts_send_value_indication(gap_conn_bf(conidx), &char_indicate,
                                                        (uint8_t *)(p_meta + 1), strlen((char *)(p_meta + 1)));
                    if(status == BT_STS_SUCCESS)
                    {
                        p_cscps_env->op_ongoing = true;
                    }
                    else
                    {
                        // Inform application that control point response has been sent
                        if (p_cscps_env->ctrl_pt_op != CSCP_CTNL_PT_RSP_CODE)
                        {
                            const cscps_cb_t* p_cb = (const cscps_cb_t*)p_cscps_env->p_upper_cb;
                            p_cb->cb_ctnl_pt_rsp_send_cmp(conidx, status);
                        }

                        // consider control point operation done
                        p_cscps_env->ctrl_pt_op = CSCP_CTNL_PT_OP_RESERVED;
                    }

                    bes_bt_buf_free(p_meta);
                } break;
            }
        }

        p_cscps_env->in_exe_op = false;
    }
}

__STATIC void cscps_pack_meas(uint8_t *p_buf, const cscp_csc_meas_t *p_meas)
{
    uint8_t *p_buf_end = p_buf + CSCP_CSC_MEAS_MAX_LEN;
    uint8_t meas_flags = p_meas->flags;

    // Check the provided flags value
    if (!GETB(p_cscps_env->prfl_cfg, CSCP_FEAT_WHEEL_REV_DATA_SUPP))
    {
        // Force Wheel Revolution Data to No (Not supported)
        SETB(meas_flags, CSCP_MEAS_WHEEL_REV_DATA_PRESENT, 0);
    }

    if (!GETB(p_cscps_env->prfl_cfg, CSCP_FEAT_CRANK_REV_DATA_SUPP))
    {
        // Force Crank Revolution Data Present to No (Not supported)
        SETB(meas_flags, CSCP_MEAS_CRANK_REV_DATA_PRESENT, 0);
    }

    // Force the unused bits of the flag value to 0
    *p_buf++ = (meas_flags & CSCP_MEAS_ALL_PRESENT);

    // Cumulative Wheel Resolutions
    // Last Wheel Event Time
    if (GETB(meas_flags, CSCP_MEAS_WHEEL_REV_DATA_PRESENT))
    {
        // Cumulative Wheel Resolutions
        *(uint32_t *)p_buf = co_host_to_uint32_le(p_cscps_env->tot_wheel_rev);
        p_buf += 4;

        // Last Wheel Event Time
        *(uint16_t *)p_buf = co_host_to_uint16_le(p_meas->last_wheel_evt_time);
        p_buf += 2;
    }

    // Cumulative Crank Revolutions
    // Last Crank Event Time
    if (GETB(meas_flags, CSCP_MEAS_CRANK_REV_DATA_PRESENT))
    {
        // Cumulative Crank Revolutions
        *(uint16_t *)p_buf = co_host_to_uint16_le(p_meas->cumul_crank_rev);
        p_buf += 2;

        // Last Crank Event Time
        *(uint16_t *)p_buf = co_host_to_uint16_le(p_meas->last_crank_evt_time);
        p_buf += 2;
    }

    // check
    if (p_buf > p_buf_end)
    {
        TRACE(0, "%s error!", __func__);
    }
}

__STATIC uint16_t cscps_unpack_ctnl_point_req(uint8_t conidx, const uint8_t *packet, uint16_t length)
{
    const uint8_t *p_buf = packet;
    const uint8_t *p_buf_end = p_buf + length;

    uint8_t op_code;
    uint8_t ctnl_pt_rsp_status = CSCP_CTNL_PT_RESP_NOT_SUPP;
    // uint16_t status = ATT_ERROR_NO_ERROR;
    union cscp_sc_ctnl_pt_req_val value;
    memset(&value, 0, sizeof(union cscp_sc_ctnl_pt_req_val));
    op_code = *p_buf++;

    switch (op_code)
    {
        case (CSCP_CTNL_PT_OP_SET_CUMUL_VAL):
        {
            // Check if the Wheel Revolution Data feature is supported
            if (GETB(p_cscps_env->features, CSCP_FEAT_WHEEL_REV_DATA_SUPP))
            {
                // Provided parameter in not within the defined range
                ctnl_pt_rsp_status = CSCP_CTNL_PT_RESP_INV_PARAM;

                if ((p_buf_end - p_buf) == 4)
                {
                    // The request can be handled
                    ctnl_pt_rsp_status = CSCP_CTNL_PT_RESP_SUCCESS;
                    // Update the environment
                    p_cscps_env->ctrl_pt_op = op_code;
                    // Cumulative value
                    value.cumul_val = CO_COMBINE_UINT32_LE(p_buf);
                    p_buf += 4;
                }
            }
        } break;
        case (CSCP_CTNL_PT_OP_UPD_LOC):
        {
            // Check if the Multiple Sensor Location feature is supported
            if (GETB(p_cscps_env->features, CSCP_FEAT_MULT_SENSOR_LOC_SUPP))
            {
                // Provided parameter in not within the defined range
                ctnl_pt_rsp_status = CSCP_CTNL_PT_RESP_INV_PARAM;

                if ((p_buf_end - p_buf) == 1)
                {
                    uint8_t sensor_loc = *p_buf++;

                    // Check the sensor location value
                    if (sensor_loc < CSCP_LOC_MAX)
                    {
                        value.sensor_loc = sensor_loc;
                        // The request can be handled
                        ctnl_pt_rsp_status = CSCP_CTNL_PT_RESP_SUCCESS;
                        // Update the environment
                        p_cscps_env->ctrl_pt_op = op_code;
                    }
                }
            }
        } break;
        case (CSCP_CTNL_PT_OP_REQ_SUPP_LOC):
        {
            // Check if the Multiple Sensor Location feature is supported
            if (GETB(p_cscps_env->features, CSCP_FEAT_MULT_SENSOR_LOC_SUPP))
            {
                // The request can be handled
                ctnl_pt_rsp_status = CSCP_CTNL_PT_RESP_SUCCESS;
                // Update the environment
                p_cscps_env->ctrl_pt_op = op_code;
            }
        } break;
        default:
        {
            // Operation Code is invalid, status is already CSCP_CTNL_PT_RESP_NOT_SUPP
        } break;
    }

    // If no error raised, inform the application about the request
    if (ctnl_pt_rsp_status == CSCP_CTNL_PT_RESP_SUCCESS)
    {
        const cscps_cb_t* p_cb  = (const cscps_cb_t*)p_cscps_env->p_upper_cb;
        // inform application about control point request
        p_cb->cb_ctnl_pt_req(conidx, op_code, &value);
    }
    else
    {
        uint8_t *p_data = NULL;
        cscps_buf_meta_t *p_buf_meta = (cscps_buf_meta_t *)bes_bt_buf_malloc(sizeof(cscps_buf_meta_t) + CSCP_SC_CNTL_PT_RSP_MIN_LEN);
        if (p_buf_meta == NULL)
        {
            return BT_STS_NO_RESOURCES;
        }
        memset(p_buf_meta, 0, sizeof(cscps_buf_meta_t) + CSCP_SC_CNTL_PT_RSP_MIN_LEN);
        INIT_LIST_HEAD(&p_buf_meta->node_hdr);
        p_buf_meta->operation = CSCPS_CTNL_PT_RSP_OP_CODE;
        p_buf_meta->conidx = conidx;
        p_cscps_env->ctrl_pt_op = CSCP_CTNL_PT_RSP_CODE;

        p_data = (uint8_t *)(p_buf_meta + 1);
        *p_data++ = CSCP_CTNL_PT_RSP_CODE;
        *p_data++ = op_code;
        *p_data++ = ctnl_pt_rsp_status;

        colist_addto_tail(&(p_buf_meta->node_hdr), &(p_cscps_env->wait_queue));
        cscps_exe_operation();
    }

    return BT_STS_SUCCESS;
}

__STATIC void cscps_pack_ctnl_point_rsp(uint8_t conidx, uint8_t *p_buf, uint8_t op_code, uint8_t resp_val,
                               const union cscp_sc_ctnl_pt_rsp_val* p_value)
{
    uint8_t *p_buf_end = p_buf + CSCP_SC_CNTL_PT_RSP_MAX_LEN;

    // Set the Response Code
    *p_buf++ = CSCP_CTNL_PT_RSP_CODE;

    // Set the request operation code
    *p_buf++ = p_cscps_env->ctrl_pt_op;

    if (resp_val == CSCP_CTNL_PT_RESP_SUCCESS)
    {
        *p_buf++ = resp_val;

        switch (p_cscps_env->ctrl_pt_op)
        {
            case (CSCP_CTNL_PT_OP_SET_CUMUL_VAL):
            {
                // Save in the environment
                p_cscps_env->tot_wheel_rev = p_value->cumul_wheel_rev;
            } break;
            case (CSCP_CTNL_PT_OP_UPD_LOC):
            {
                // Store the new value in the environment
                p_cscps_env->sensor_loc = p_value->sensor_loc;
            } break;
            case (CSCP_CTNL_PT_OP_REQ_SUPP_LOC):
            {
                // Set the list of supported location
                for (uint8_t counter = 0; counter < CSCP_LOC_MAX; counter++)
                {
                    if ((p_value->supp_sensor_loc >> counter) & 0x0001)
                    {
                        *p_buf++ = counter;
                    }
                }
            } break;
            default: { /* Nothing to do */ } break;
        }
    }
    else
    {
        // Set the Response Value
        *p_buf++ = (resp_val > CSCP_CTNL_PT_RESP_FAILED ? CSCP_CTNL_PT_RESP_FAILED : resp_val);
    }

    if (p_buf > p_buf_end)
    {
        TRACE(0, "%s error!", __func__);
    }
}

uint16_t ble_cscps_enable(uint8_t conidx, uint16_t csc_meas_ntf_cfg, uint16_t sc_ctnl_pt_ntf_cfg)
{
    if(p_cscps_env == NULL)
    {
        return BT_STS_FAILED;
    }

    // check state of the task
    if (app_ble_get_conhdl_from_conidx(conidx) != GAP_INVALID_CONN_HANDLE)
    {
        // Check the provided value
        if (csc_meas_ntf_cfg == PRF_CLI_START_NTF)
        {
            // Store the status
            SETB(p_cscps_env->prfl_ntf_ind_cfg[conidx], CSCP_PRF_CFG_FLAG_CSC_MEAS_NTF, 1);
        }

        if (CSCPS_IS_FEATURE_SUPPORTED(p_cscps_env->prfl_cfg, CSCPS_SC_CTNL_PT_MASK))
        {
            // Check the provided value
            if (sc_ctnl_pt_ntf_cfg == PRF_CLI_START_IND)
            {
                // Store the status
                SETB(p_cscps_env->prfl_ntf_ind_cfg[conidx], CSCP_PRF_CFG_FLAG_SC_CTNL_PT_IND, 1);
            }
        }

        // Enable Bonded Data
        SETB(p_cscps_env->prfl_ntf_ind_cfg[conidx], CSCP_PRF_CFG_PERFORMED_OK, 1);

        return BT_STS_SUCCESS;
    }

    return BT_STS_FAILED;
}

uint16_t ble_cscps_meas_send(uint32_t conidx_bf, int16_t wheel_rev,  const cscp_csc_meas_t* p_meas)
{
    if(p_meas == NULL)
    {
        return BT_STS_INVALID_PARM;
    }
    else if(p_cscps_env != NULL)
    {
        // Should be updated just once
        if (GETB(p_meas->flags, CSCP_MEAS_WHEEL_REV_DATA_PRESENT) &&
            GETB(p_cscps_env->features, CSCP_FEAT_WHEEL_REV_DATA_SUPP))
        {
            // Update the cumulative wheel revolutions value stored in the environment
            if (wheel_rev < 0)
            {
                // The value shall not decrement below zero
                p_cscps_env->tot_wheel_rev = (((uint32_t)co_abs(wheel_rev) > p_cscps_env->tot_wheel_rev) ?
                                                0 : (p_cscps_env->tot_wheel_rev + wheel_rev));
            }
            else
            {
                p_cscps_env->tot_wheel_rev += wheel_rev;
            }
        }

        cscps_buf_meta_t *p_buf_meta = (cscps_buf_meta_t *)bes_bt_buf_malloc(sizeof(cscps_buf_meta_t) + CSCP_CSC_MEAS_MAX_LEN);
        if (p_buf_meta == NULL)
        {
            return BT_STS_NO_RESOURCES;
        }
        memset(p_buf_meta, 0, sizeof(cscps_buf_meta_t) + CSCP_CSC_MEAS_MAX_LEN);
        INIT_LIST_HEAD(&p_buf_meta->node_hdr);
        p_buf_meta->operation = CSCPS_SEND_CSC_MEAS_OP_CODE;
        p_buf_meta->conidx    = GAP_INVALID_CONIDX;
        p_buf_meta->conidx_bf = conidx_bf & ((1 << BLE_CONNECTION_MAX) - 1);

        // pack measurement
        cscps_pack_meas((uint8_t*)(p_buf_meta + 1), p_meas);
        // put event on wait queue
        colist_addto_tail(&(p_buf_meta->node_hdr), &(p_cscps_env->wait_queue));
        // execute operation
        cscps_exe_operation();

        return BT_STS_SUCCESS;
    }

    return BT_STS_FAILED;
}

uint16_t ble_cscps_ctnl_pt_rsp_send(uint8_t conidx, uint8_t op_code, uint8_t resp_val,
                                    const union cscp_sc_ctnl_pt_rsp_val* p_value)
{
    if(p_value == NULL)
    {
        return BT_STS_INVALID_PARM;
    }
    else if(p_cscps_env != NULL)
    {
        // Check the current operation
        if (p_cscps_env->ctrl_pt_op == CSCP_CTNL_PT_OP_RESERVED)
        {
            // The confirmation has been sent without request indication, ignore
            return BT_STS_INVALID_STATUS;
        }

        // The CP Control Point Characteristic must be supported if we are here
        if (!CSCPS_IS_FEATURE_SUPPORTED(p_cscps_env->prfl_cfg, CSCPS_SC_CTNL_PT_MASK))
        {
            return BT_STS_NOT_SUPPORTED;
        }

        // Check if sending of indications has been enabled
        if (!GETB(p_cscps_env->prfl_ntf_ind_cfg[conidx], CSCP_PRF_CFG_FLAG_SC_CTNL_PT_IND))
        {
            // mark operation done
            p_cscps_env->ctrl_pt_op = CSCP_CTNL_PT_OP_RESERVED;
            // CPP improperly configured
            return BT_STS_NOT_SUPPORTED;
        }

        cscps_buf_meta_t *p_buf_meta = (cscps_buf_meta_t *)bes_bt_buf_malloc(sizeof(cscps_buf_meta_t) + CSCP_SC_CNTL_PT_RSP_MAX_LEN);
        if (p_buf_meta == NULL)
        {
            return BT_STS_NO_RESOURCES;
        }
        memset(p_buf_meta, 0, sizeof(cscps_buf_meta_t) + CSCP_SC_CNTL_PT_RSP_MAX_LEN);
        INIT_LIST_HEAD(&p_buf_meta->node_hdr);
        p_buf_meta->operation = CSCPS_CTNL_PT_RSP_OP_CODE;
        p_buf_meta->conidx    = conidx;

        // Pack structure
        cscps_pack_ctnl_point_rsp(conidx, (uint8_t *)(p_buf_meta + 1), op_code, resp_val, p_value);
        // put event on wait queue
        colist_addto_tail(&(p_buf_meta->node_hdr), &(p_cscps_env->wait_queue));
        // execute operation
        cscps_exe_operation();

        return BT_STS_SUCCESS;
    }

    return BT_STS_FAILED;
}

__STATIC int app_ble_cscp_server_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
{
    uint8_t conidx = GAP_INVALID_CONIDX;
    const cscps_cb_t* p_cb  = (const cscps_cb_t*)p_cscps_env->p_upper_cb;

    TRACE(0, "%s event=%d", __func__, event);

    if (p_cscps_env == NULL)
    {
        return BT_STS_NOT_READY;
    }

    switch (event)
    {
        case GATT_SERV_EVENT_CHAR_READ:
        {
            gatt_server_char_read_t *p = param.char_read;
            p->rsp_error_code = ATT_ERROR_NO_ERROR;
            if (p->character == g_ble_csc_feature)
            {
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)&(p_cscps_env->features), sizeof(uint16_t));
            }
            else if (p->character == g_ble_csc_sensor_location)
            {
                gatts_write_read_rsp_data(p->ctx, &p_cscps_env->sensor_loc, sizeof(uint8_t));
            }
            else
            {
                p->rsp_error_code = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
            }
            return (p->rsp_error_code == ATT_ERROR_NO_ERROR ? true : false);
        }
        case GATT_SERV_EVENT_DESC_READ:
        {
            gatt_server_desc_read_t *p = param.desc_read;
            p->rsp_error_code = ATT_ERROR_NO_ERROR;
            conidx = p->con_idx;
            if ((uint8_t *)p->desc_attr->attr_data == g_ble_csc_measurement_cccd)
            {
                uint16_t ntf_cfg = GETB(p_cscps_env->prfl_ntf_ind_cfg[conidx], CSCP_PRF_CFG_FLAG_CSC_MEAS_NTF) ?
                                        PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)&ntf_cfg, sizeof(uint16_t));
            }
            else if ((uint8_t *)p->desc_attr->attr_data == g_ble_csc_control_point_cccd)
            {
                uint16_t ind_cfg = GETB(p_cscps_env->prfl_ntf_ind_cfg[conidx], CSCP_PRF_CFG_FLAG_SC_CTNL_PT_IND) ?
                                        PRF_CLI_START_IND : PRF_CLI_STOP_NTFIND;
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)&ind_cfg, sizeof(uint16_t));
            }
            else
            {
                p->rsp_error_code = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
            }
            return (p->rsp_error_code == ATT_ERROR_NO_ERROR ? true : false);
        }
        case GATT_SERV_EVENT_CHAR_WRITE:
        {
            gatt_server_char_write_t *p = param.char_write;
            p->rsp_error_code = ATT_ERROR_NO_ERROR;
            conidx = p->con_idx;
            if (p->character == g_ble_csc_control_point)
            {
                // Check if sending of indications has been enabled
                if (!GETB(p_cscps_env->prfl_ntf_ind_cfg[conidx], CSCP_PRF_CFG_FLAG_SC_CTNL_PT_IND))
                {
                    // CPP improperly configured
                    p->rsp_error_code = ATT_ERROR_CCCD_IMPROPER_CONFIGED;
                }
                else if (p_cscps_env->ctrl_pt_op != CSCP_CTNL_PT_OP_RESERVED)
                {
                    // A procedure is already in progress
                    p->rsp_error_code = ATT_ERROR_PROC_ALREADY_IN_PROGRESS;
                }
                else
                {
                    // Unpack Control Point parameters
                    if (cscps_unpack_ctnl_point_req(conidx, p->value, p->value_len) != BT_STS_SUCCESS)
                    {
                        p->rsp_error_code = ATT_ERROR_VALUE_NOT_ALLOWED;
                    }
                }
            }
            else 
            {
                p->rsp_error_code = ATT_ERROR_REQ_NOT_SUPPORT;
            }
            gatts_send_write_rsp(p->ctx, p->rsp_error_code);
            return (p->rsp_error_code == ATT_ERROR_NO_ERROR ? true : false);
        }
        case GATT_SERV_EVENT_DESC_WRITE:
        {
            gatt_server_desc_write_t *p = param.desc_write;
            conidx = p->con_idx;
            uint16_t cfg = CO_COMBINE_UINT16_LE(p->value);
            p->rsp_error_code = ATT_ERROR_NO_ERROR;

            uint8_t cfg_upd_flag  = 0;
            uint8_t cfg_upd_char  = 0;
            uint16_t cfg_en_val = 0;

            if ((uint8_t *)p->desc_attr->attr_data == g_ble_csc_measurement_cccd)
            {
                cfg_upd_char = CSCP_CSCS_CSC_MEAS_CHAR;
                cfg_upd_flag = CSCP_PRF_CFG_FLAG_CSC_MEAS_NTF_BIT;
                cfg_en_val   = PRF_CLI_START_NTF;
            }
            else if ((uint8_t *)p->desc_attr->attr_data == g_ble_csc_control_point_cccd)
            {
                cfg_upd_char = CSCP_CSCS_SC_CTNL_PT_CHAR;
                cfg_upd_flag = CSCP_PRF_CFG_FLAG_SC_CTNL_PT_IND_BIT;
                cfg_en_val   = PRF_CLI_START_IND;
            }

            if(cfg_upd_flag != 0)
            {
                // parameter check
                if ((p->value_len == sizeof(uint16_t)) &&
                    ((cfg == PRF_CLI_STOP_NTFIND) || (cfg == cfg_en_val)))
                {
                    if(cfg == PRF_CLI_STOP_NTFIND)
                    {
                        p_cscps_env->prfl_ntf_ind_cfg[conidx] &= ~cfg_upd_flag;
                    }
                    else
                    {
                        p_cscps_env->prfl_ntf_ind_cfg[conidx] |= cfg_upd_flag;
                    }

                    // inform application about update
                    p_cb->cb_bond_data_upd(conidx, cfg_upd_char, cfg);
                }
                else
                {
                    p->rsp_error_code = ATT_ERROR_CCCD_IMPROPER_CONFIGED;
                }
            }
            gatts_send_write_rsp(p->ctx, p->rsp_error_code);
            return (p->rsp_error_code == ATT_ERROR_NO_ERROR ? true : false);
        }
        case GATT_SERV_EVENT_NTF_TX_DONE:
        {
            gatt_server_indicate_cfm_t *p = param.confirm;
            p_cscps_env->op_ongoing = false;

            if (p->character == g_ble_csc_measurement)
            {
                p_cb->cb_meas_send_cmp(p->error_code);
            }
            // continue operation execution
            cscps_exe_operation();
            return true;
        }   /*  */
        case GATT_SERV_EVENT_INDICATE_CFM:
        {
            gatt_server_indicate_cfm_t *p = param.confirm;
            p_cscps_env->op_ongoing = false;

            if (p->character == g_ble_csc_control_point)
            {
                // Inform application that control point response has been sent
                if (p_cscps_env->ctrl_pt_op != CSCP_CTNL_PT_RSP_CODE)
                {
                    p_cb->cb_ctnl_pt_rsp_send_cmp(conidx, p->error_code);
                }
                p_cscps_env->ctrl_pt_op = CSCP_CTNL_PT_OP_RESERVED;
            }
            cscps_exe_operation();
            return true;
        }
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

uint16_t ble_cscps_init(struct cscps_db_cfg* cscps_cfg, const cscps_cb_t* cscps_upper_cb)
{
    // DB Creation Status
    uint16_t status = BT_STS_SUCCESS;
    uint32_t cfg_flag = CSCPS_MANDATORY_MASK;
    gatt_attribute_t *p_attr_list = NULL;
    uint8_t p_attr_list_len = 0;

    if (p_cscps_env != NULL)
    {
        return BT_STS_IN_USE;
    }

    if(cscps_cfg == NULL || cscps_upper_cb == NULL ||
       cscps_upper_cb->cb_meas_send_cmp == NULL ||
       cscps_upper_cb->cb_bond_data_upd == NULL ||
       cscps_upper_cb->cb_ctnl_pt_req == NULL ||
       cscps_upper_cb->cb_ctnl_pt_rsp_send_cmp == NULL)
    {
        return BT_STS_INVALID_PARM;
    }

    /*
     * Check if the Sensor Location characteristic shall be added.
     * Mandatory if the Multiple Sensor Location feature is supported, otherwise optional.
     */
    if ((cscps_cfg->sensor_loc_supp == CSCPS_SENSOR_LOC_SUPP) ||
        (GETB(cscps_cfg->csc_feature, CSCP_FEAT_MULT_SENSOR_LOC_SUPP)))
    {
        cfg_flag |= CSCPS_SENSOR_LOC_MASK;
    }

    /*
     * Check if the SC Control Point characteristic shall be added
     * Mandatory if at least one SC Control Point procedure is supported, otherwise excluded.
     */
    if (GETB(cscps_cfg->csc_feature, CSCP_FEAT_WHEEL_REV_DATA_SUPP) ||
        GETB(cscps_cfg->csc_feature, CSCP_FEAT_MULT_SENSOR_LOC_SUPP))
    {
        cfg_flag |= CSCPS_SC_CTNL_PT_MASK;
    }

    status = cscps_prepare_service_attr_list(cfg_flag, &p_attr_list, &p_attr_list_len);
    if (status != BT_STS_SUCCESS)
    {
        return status;
    }

    gatts_cfg_t svc_cfg =
    {
        .service_inst_id = 0,
        .svc_size = p_attr_list_len,
    };
    status = gatts_register_service(p_attr_list, p_attr_list_len, app_ble_cscp_server_callback, &svc_cfg);

    // Malloc for env
    p_cscps_env = (cscps_env_t *)bes_bt_buf_malloc(sizeof(cscps_env_t));
    if (p_cscps_env == NULL)
    {
        return BT_STS_NO_RESOURCES;
    }
    memset(p_cscps_env, 0, sizeof(cscps_env_t));

    p_cscps_env->p_upper_cb = cscps_upper_cb;
    p_cscps_env->features        = cscps_cfg->csc_feature;
    p_cscps_env->prfl_cfg        = cfg_flag;
    p_cscps_env->tot_wheel_rev   = cscps_cfg->wheel_rev;
    p_cscps_env->op_ongoing      = false;
    p_cscps_env->in_exe_op       = false;
    p_cscps_env->ctrl_pt_op      = CSCPS_RESERVED_OP_CODE;
    memset(p_cscps_env->prfl_ntf_ind_cfg, 0, sizeof(p_cscps_env->prfl_ntf_ind_cfg));
    INIT_LIST_HEAD(&(p_cscps_env->wait_queue));
    if (CSCPS_IS_FEATURE_SUPPORTED(p_cscps_env->prfl_cfg, CSCPS_SENSOR_LOC_MASK))
    {
        p_cscps_env->sensor_loc = (cscps_cfg->sensor_loc >= CSCP_LOC_MAX ? CSCP_LOC_OTHER : cscps_cfg->sensor_loc);
    }

    return status;
}

uint16_t ble_cscps_deinit()
{
    if (p_cscps_env == NULL)
    {
        return BT_STS_FAILED;
    }

    gatts_unregister_service(cscps_hrs_att_list);
    memset(cscps_hrs_att_list, 0, (CSCPS_GATT_ATTRIBUTE_MAX_LEN * sizeof(gatt_attribute_t)));

    while(!colist_is_list_empty(&(p_cscps_env->wait_queue)))
    {
        struct list_node *node = colist_get_head(&(p_cscps_env->wait_queue));
        if (node)
        {
            cscps_buf_meta_t *p_buf_meta = colist_structure(node, cscps_buf_meta_t, node_hdr);
            colist_delete(node);
            bes_bt_buf_free(p_buf_meta);
        }
    }

    bes_bt_buf_free(p_cscps_env);
    return BT_STS_SUCCESS;
}

#endif /* BLE_CSCPS_ENABLED */