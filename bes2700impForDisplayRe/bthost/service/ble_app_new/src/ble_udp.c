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
#if defined(BLE_UDP_ENABLE)
#include "gatt_service.h"
#include "ble_uds.h"
#include "app_ble.h"

/// User Data Service - Attribute List
enum uds_att_list
{
    /// User Data Service
    UDS_IDX_SVC,
    /// database change increment
    UDS_IDX_DATABASE_CHANGE_INC,
    /// user index
    UDS_IDX_USER_INDEX,
    /// Control Point
    UDS_IDX_CTRL_PT_CHAR,
    UDS_IDX_CTRL_PT_IND_CFG,
    /// Number of attributes
    CPS_IDX_NB,
};
/// Operation Code used in the profile state machine
enum uds_op_code
{
    /// Reserved Operation Code
    UDS_RESERVED_OP_CODE                          = 0x00,
    /// ctrl pt
    UDS_IND_CTRL_PT_OP_CODE                       = 0x01,
};

/* TYPEDEF */
/// ongoing operation information
typedef struct uds_buf_meta
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
} ble_uds_buf_meta_t;

/// User Data Service Sensor environment variable per connection
typedef struct uds_cnx_env
{
    /// Profile Notify/Indication Flags
    uint8_t  prfl_ntf_ind_cfg;
} ble_uds_cnx_env_t;

/// UDS Environment Variable
typedef struct uds_env
{
    /// Operation Event TX wait queue
    struct list_node   wait_queue;
    /// Event callabcks
    const ble_uds_cb_t *p_cb;
    /// Environment variable pointer for each connections
    ble_uds_cnx_env_t env[BLE_CONNECTION_MAX];
    /// Database Change Increment
    uint32_t       database_change_inc;
    /// User Index
    uint32_t       user_index;
    /// Control point operation on-going (see enum #cps_ctrl_pt_code)
    uint8_t        ctrl_pt_op;
    /// Operation On-going
    bool           op_ongoing;
    /// Prevent recursion in execute_operation function
    bool           in_exe_op;
    /// Service attribute list
    gatt_attribute_t *p_attr_list;
} ble_uds_env_t;
ble_uds_env_t *p_uds_env = NULL;

GATT_DECL_PRI_SERVICE(g_ble_udp_service, GATT_UUID_UDP_SERVICE);

GATT_DECL_CHAR(g_ble_udp_database_change_inc,
    GATT_CHAR_UUID_DATABASE_CHANGE_INC,
    GATT_RD_REQ|GATT_WR_REQ,
    ATT_WR_ENC);

GATT_DECL_CHAR(g_ble_udp_user_index,
    GATT_CHAR_UUID_USER_INDEX,
    GATT_RD_REQ,
    ATT_RD_ENC);

GATT_DECL_CHAR(g_ble_udp_ctrl_pt,
    GATT_CHAR_UUID_USER_CTRL_POINT,
    GATT_WR_REQ|GATT_IND_PROP,
    ATT_RD_ENC|ATT_WR_ENC);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_udp_ctrl_pt_cccd,
    ATT_RD_ENC|ATT_WR_ENC);

static const gatt_attribute_t g_ble_udp_service_attr_list[] = {
    /* Service */
    gatt_attribute(g_ble_udp_service),
    /* Characteristics */
    gatt_attribute(g_ble_udp_database_change_inc),
    gatt_attribute(g_ble_udp_user_index),
    gatt_attribute(g_ble_udp_ctrl_pt),
    gatt_attribute(g_ble_udp_ctrl_pt_cccd),
};

/* INTERNAL FUNCTIONS */
static void ble_uds_exe_operation(void);
/// Get database attribute index
static uint8_t ble_uds_idx_get(const gatt_attribute_t *attr)
{
    uint8_t att_idx = ((uint32_t)attr - (uint32_t)p_uds_env->p_attr_list) / sizeof(gatt_attribute_t);
    return att_idx;
}

static uint16_t ble_uds_unpack_ctrl_point_req(uint8_t conidx, const uint8_t *p_buf, uint8_t buf_len)
{
    uint8_t op_code;
    uint8_t ctrl_pt_rsp_status = UDS_RESULT_OPCODE_NOT_SUPPORTED;
    uint16_t status = BT_STS_SUCCESS;
    uds_cpt_opcode value;
    memset(&value, 0, sizeof(uds_cpt_opcode));
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
        case (UDS_CTRL_PT_REGIS_NEW_USER):
        {
            // The request can be handled
            ctrl_pt_rsp_status = UDS_RESULT_SUCCESS;
            // Update the environment
            p_uds_env->ctrl_pt_op = op_code;
            value.data.consent_code = CO_COMBINE_UINT16_LE(p_buf);
        }
        break;
        case (UDS_CTRL_PT_CONSENT):
        {
            // The request can be handled
            ctrl_pt_rsp_status = UDS_RESULT_SUCCESS;
            // Update the environment
            p_uds_env->ctrl_pt_op = op_code;
            value.data.consent.user_index = p_buf[0];
            p_buf += 1;
            value.data.consent.consent_code = CO_COMBINE_UINT16_LE(p_buf);
        }
        break;
        case (UDS_CTRL_PT_DELET_USER_DATA):
        case (UDS_CTRL_PT_LIST_ALL_USERS):
        {
            // The request can be handled
            ctrl_pt_rsp_status = UDS_RESULT_SUCCESS;
            // Update the environment
            p_uds_env->ctrl_pt_op = op_code;
        }
        break;
        case (UDS_CTRL_PT_DELET_USER):
        {
            // Optional even if feature not supported
            ctrl_pt_rsp_status = UDS_RESULT_SUCCESS;
            // Update the environment
            p_uds_env->ctrl_pt_op = op_code;
            value.data.user_index = p_buf[0];
        }
        break;
        default:
        {
            // Operation Code is invalid, status is already UDS_CTRL_PT_RESP_NOT_SUPP
        } break;
    }
    // If no error raised, inform the application about the request
    if (ctrl_pt_rsp_status == UDS_RESULT_SUCCESS)
    {
        const ble_uds_cb_t *p_cb  = (const ble_uds_cb_t *) p_uds_env->p_cb;
        // inform application about control point request
        p_cb->cb_ctrl_pt_req(conidx, op_code, &value);
    }
    else
    {
        uint8_t *p_out_buf = NULL;
        if ((p_out_buf = bes_bt_me_bes_bt_buf_malloc(UDS_CTRL_PT_RSP_MIN_LEN + sizeof(ble_uds_buf_meta_t))))
        {
            ble_uds_buf_meta_t *p_meta = (ble_uds_buf_meta_t *)(p_out_buf);
            p_meta->buf_len = UDS_CTRL_PT_RSP_MIN_LEN;
            p_out_buf = p_meta->buf;
            p_uds_env->ctrl_pt_op    = UDS_CTRL_PT_RSP_CODE;
            *p_out_buf++ = UDS_CTRL_PT_RSP_CODE;
            *p_out_buf++ = op_code;
            *p_out_buf++ = ctrl_pt_rsp_status;
            p_meta->conidx    = conidx;
            p_meta->operation = UDS_CTRL_PT_RSP_CODE;
            // put event on wait queue
            colist_addto_tail(&(p_meta->node), &(p_uds_env->wait_queue));
            // execute operation
            ble_uds_exe_operation();
        }
        else
        {
            status = ATT_ERROR_INSUFF_RESOURCES;
        }
    }
    return status;
}

static void ble_uds_exe_operation(void)
{
    if (bt_defer_curr_func_0(ble_uds_exe_operation))
    {
        return;
    }
    if (!p_uds_env->in_exe_op)
    {
        p_uds_env->in_exe_op = true;
        while (!colist_is_list_empty(&(p_uds_env->wait_queue)) && !(p_uds_env->op_ongoing))
        {
            uint16_t status = BT_STS_SUCCESS;
            ble_uds_buf_meta_t *p_meta = (ble_uds_buf_meta_t *)colist_get_head(&(p_uds_env->wait_queue));
            switch (p_meta->operation)
            {
                default:
                {
                    uint8_t conidx = p_meta->conidx;
                    // remove buffer from queue
                    colist_delete(&(p_meta->node));
                    gatt_char_notify_t ntf_cfg =
                    {
                        .character = g_ble_udp_ctrl_pt,
                        .service = g_ble_udp_service,
                    };
                    status = gatts_send_value_indication(UDS_2_GAP_CONIDX_BF(conidx), &ntf_cfg, p_meta->buf, p_meta->buf_len);
                    if (status == BT_STS_SUCCESS)
                    {
                        p_uds_env->op_ongoing = true;
                    }
                    else
                    {
                        // Inform application that control point response has been sent
                        if (p_uds_env->ctrl_pt_op != UDS_CTRL_PT_RSP_CODE)
                        {
                            const ble_uds_cb_t *p_cb = (const ble_uds_cb_t *) p_uds_env->p_cb;
                            p_cb->cb_ctrl_pt_rsp_send_cmp(conidx, status);
                        }
                        // consider control point operation done
                        p_uds_env->ctrl_pt_op = UDS_CTRL_PT_RESERVED;
                    }
                    bes_bt_me_bes_bt_buf_free((uint8_t *)p_meta);
                }
                break;
            }
        }
        p_uds_env->in_exe_op = false;
    }
}
static void ble_uds_cb_att_read_get(uint16_t conidx, uint32_t token, const gatt_attribute_t *attr, uint16_t offset)
{
    // retrieve value attribute
    uint16_t  status      = ATT_ERROR_NO_ERROR;
    uint8_t *p_buf        = NULL;
    uint16_t  p_buf_len = 0;
    if ((p_buf = bes_bt_me_bes_bt_buf_malloc(UDS_CTRL_PT_RSP_MAX_LEN)))
    {
        switch (ble_uds_idx_get(attr))
        {
            case UDS_IDX_DATABASE_CHANGE_INC:
            {
                *(uint16_t *)p_buf = co_host_to_uint16_le(p_uds_env->database_change_inc);
                p_buf_len += 2;
            }
            break;
            case UDS_IDX_USER_INDEX:
            {
                p_buf[0] = p_uds_env->user_index;
                p_buf_len += 1;
            }
            break;
            case UDS_IDX_CTRL_PT_IND_CFG:
            {
                *(uint16_t *)p_buf = co_host_to_uint16_le(p_uds_env->env[conidx].prfl_ntf_ind_cfg);
                p_buf_len += 2;
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

static void ble_uds_cb_att_val_set(uint8_t conidx, uint32_t token, const gatt_attribute_t *attr, uint16_t offset, const uint8_t *p_buf, uint8_t buf_len)
{
    uint16_t status = BT_STS_FAILED;
    if (p_uds_env != NULL)
    {
        switch (ble_uds_idx_get(attr))
        {
            case UDS_IDX_CTRL_PT_IND_CFG:
            {
                uint16_t cfg = CO_COMBINE_UINT16_LE(p_buf);
                // parameter check
                if (buf_len == sizeof(uint16_t))
                {
                    const ble_uds_cb_t *p_cb  = (const ble_uds_cb_t *) p_uds_env->p_cb;
                    if (cfg == PRF_CLI_STOP_NTFIND)
                    {
                        p_uds_env->env[conidx].prfl_ntf_ind_cfg &= ~cfg;
                    }
                    else
                    {
                        p_uds_env->env[conidx].prfl_ntf_ind_cfg |= cfg;
                    }
                    // inform application about update
                    p_cb->cb_bond_data_upd(conidx, cfg);
                    status = BT_STS_SUCCESS;
                }
                else
                {
                    status = ATT_ERROR_CCCD_IMPROPER_CONFIGED;
                }
            }
            break;
            case UDS_IDX_USER_INDEX:
            {
                p_uds_env->user_index = p_buf[0];
            }
            break;
            case UDS_IDX_CTRL_PT_CHAR:
            {
                if (p_uds_env->ctrl_pt_op != UDS_CTRL_PT_RESERVED)
                {
                    // A procedure is already in progress
                    status = ATT_ERROR_PROC_ALREADY_IN_PROGRESS;
                }
                else
                {
                    // Unpack Control Point parameters
                    status = ble_uds_unpack_ctrl_point_req(conidx, p_buf, buf_len);
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
    gatts_send_defer_write_rsp(app_ble_get_conhdl_from_conidx(conidx), token, status);
}

static void ble_uds_cb_event_sent(uint8_t conidx, const gatt_attribute_t *p_char_attr, uint16_t status)
{
    // Consider job done
    const ble_uds_cb_t *p_cb  = (const ble_uds_cb_t *) p_uds_env->p_cb;
    p_uds_env->op_ongoing = false;
    uint32_t dummy = ble_uds_idx_get(p_char_attr);
    if (dummy == UDS_IDX_CTRL_PT_CHAR)
    {
        dummy = UDS_IND_CTRL_PT_OP_CODE;
    }
    else
    {
        dummy = UDS_CTRL_PT_RSP_CODE;
    }
    switch (dummy)
    {
        default:
        {
            // Inform application that control point response has been sent
            if (p_uds_env->ctrl_pt_op != UDS_CTRL_PT_RSP_CODE)
            {
                p_cb->cb_ctrl_pt_rsp_send_cmp(conidx, status);
            }
            p_uds_env->ctrl_pt_op = UDS_CTRL_PT_RESERVED;
        }
        break;
    }
    // continue operation execution
    ble_uds_exe_operation();
}

static void ble_uds_conn_alloc(uint8_t conidx)
{
    memset(&(p_uds_env->env[conidx]), 0, sizeof(ble_uds_cnx_env_t));
}
static void ble_uds_conn_free(uint8_t conidx)
{
    memset(&(p_uds_env->env[conidx]), 0, sizeof(ble_uds_cnx_env_t));
}
static int ble_uds_gatt_server_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
{
    if (p_uds_env == NULL)
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
                TRACE(1, "ble uds resgiter success: shdl = %d", param.resgiter_cmp->start_handle);
            }
            else
            {
                TRACE(1, "ble uds resgited fail: status = %d", param.resgiter_cmp->status);
            }
            return true;
        }
        case GATT_SERV_EVENT_CONN_OPENED:
        {
            ble_uds_conn_alloc(conidx);
            return true;
        }
        case GATT_SERV_EVENT_CONN_CLOSED:
        {
            ble_uds_conn_free(conidx);
            return true;
        }
        case GATT_SERV_EVENT_CHAR_READ:
        {
            ble_uds_cb_att_read_get(conidx, param.char_read->ctx->token, param.char_read->char_attr,
                                    param.char_read->value_offset);
            return true;
        }
        case GATT_SERV_EVENT_DESC_READ:
        {
            ble_uds_cb_att_read_get(conidx, param.desc_read->ctx->token, param.desc_read->desc_attr,
                                    param.desc_read->value_offset);
            return true;
        }
        case GATT_SERV_EVENT_CHAR_WRITE:
        {
            ble_uds_cb_att_val_set(conidx, param.char_write->ctx->token, param.char_write->char_attr,
                                   param.char_write->value_offset, param.char_write->value,
                                   param.char_write->value_len);
            return true;
        }
        case GATT_SERV_EVENT_DESC_WRITE:
        {
            ble_uds_cb_att_val_set(conidx, param.desc_write->ctx->token, param.desc_write->desc_attr,
                                   param.desc_write->value_offset, param.desc_write->value,
                                   param.desc_write->value_len);
            return true;
        }
        case GATT_SERV_EVENT_INDICATE_CFM:
        case GATT_SERV_EVENT_NTF_TX_DONE:
        {
            ble_uds_cb_event_sent(conidx, param.confirm->char_attr, param.confirm->error_code);
            return true;
        }
        default:
        {
            break;
        }
    }
    return false;
}

static uint16_t ble_uds_prepare_service_attr_list(gatt_attribute_t **pp_attr_list, uint8_t *p_attr_list_len)
{
    if (p_attr_list_len == NULL)
    {
        return BT_STS_INVALID_PARM;
    }
    *p_attr_list_len = ARRAY_SIZE(g_ble_udp_service_attr_list);
    uint8_t size = sizeof(g_ble_udp_service_attr_list);
    *pp_attr_list = (gatt_attribute_t *)bes_bt_me_bes_bt_buf_malloc(size);
    if (*pp_attr_list == NULL)
    {
        return BT_STS_NO_RESOURCES;
    }
    memset(*pp_attr_list, 0, size);
    memcpy(*pp_attr_list, g_ble_udp_service_attr_list, size);
    return BT_STS_SUCCESS;
}

/* FUNCSTIONS */
uint16_t ble_uds_enable(uint8_t conidx, uint8_t ntf_ind_cfg)
{
    uint16_t status = BT_STS_NOT_ALLOW;
    if (p_uds_env != NULL)
    {
        // check state of the task
        if (app_ble_get_conhdl_from_conidx(conidx) != 0xFFFF)
        {
            p_uds_env->env[conidx].prfl_ntf_ind_cfg = ntf_ind_cfg;
            status = BT_STS_SUCCESS;
        }
    }
    return status;
}

uint16_t ble_uds_ctrl_pt_rsp_send(uint8_t conidx, uint8_t op_code, uint8_t resp_val, const uds_cpt_rsp *p_value)
{
    uint16_t status = BT_STS_NOT_ALLOW;
    if (p_value == NULL)
    {
        status = BT_STS_INVALID_PARM;
    }
    else if (p_uds_env != NULL)
    {
        do
        {
            uint8_t *p_buf = NULL;
            // Check the current operation
            if (p_uds_env->ctrl_pt_op ==  UDS_CTRL_PT_RESERVED)
            {
                // The confirmation has been sent without request indication, ignore
                break;
            }
            // Check if sending of indications has been enabled
            if (p_uds_env->env[conidx].prfl_ntf_ind_cfg)
            {
                // mark operation done
                p_uds_env->ctrl_pt_op = UDS_CTRL_PT_RESERVED;
                // improperly configured
                status = ATT_ERROR_CCCD_IMPROPER_CONFIGED;
                break;
            }
            if ((p_buf = bes_bt_me_bes_bt_buf_malloc(sizeof(uds_cpt_rsp))))
            {
                ble_uds_buf_meta_t *p_buf_meta = (ble_uds_buf_meta_t *)p_buf;
                p_buf_meta->operation = UDS_CTRL_PT_RSP_CODE;
                p_buf_meta->conidx    = conidx;
                p_buf_meta->new       = true;
                p_buf_meta->buf_len   = sizeof(uds_cpt_rsp);
                // put event on wait queue
                colist_addto_tail(&(p_buf_meta->node), &(p_uds_env->wait_queue));
                // execute operation
                ble_uds_exe_operation();
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

uint16_t ble_uds_init(const ble_uds_cb_t *p_cb)
{
    // DB Creation Status
    uint16_t status = BT_STS_SUCCESS;
    gatt_attribute_t *p_attr_list = NULL;
    uint8_t attr_list_len = 0;
    if (p_uds_env != NULL)
    {
        return BT_STS_ALREADY_REGISTERED;
    }
    do
    {
        if ((p_cb == NULL) || (p_cb->cb_bond_data_upd == NULL)  || (p_cb->cb_ctrl_pt_req == NULL)
                || (p_cb->cb_ctrl_pt_rsp_send_cmp == NULL))
        {
            status = BT_STS_INVALID_PARM;
            break;
        }

        gatts_cfg_t cfg =
        {
            .preferred_mtu = UDS_CP_NTF_MAX_LEN + 3,
        };
        status = ble_uds_prepare_service_attr_list(&p_attr_list, &attr_list_len);
        if (status == BT_STS_SUCCESS)
        {
            // register UDS user
            status = gatts_register_service(p_attr_list, attr_list_len, ble_uds_gatt_server_callback, &cfg);
        }
        if (status != BT_STS_SUCCESS)
        {
            break;
        }
        p_uds_env = (ble_uds_env_t *) bes_bt_me_bes_bt_buf_malloc(sizeof(ble_uds_env_t));
        if (p_uds_env != NULL)
        {
            // allocate UDS required environment variable
            p_uds_env->op_ongoing      = false;
            p_uds_env->in_exe_op       = false;
            p_uds_env->ctrl_pt_op      = UDS_CTRL_PT_RESERVED;
            p_uds_env->p_attr_list     = p_attr_list;
            memset(p_uds_env->env, 0, sizeof(p_uds_env->env));
            INIT_LIST_HEAD(&(p_uds_env->wait_queue));
            // initialize profile environment variable
            p_uds_env->p_cb     = p_cb;
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

uint16_t ble_uds_deinit(void)
{
    if (p_uds_env != NULL)
    {
        // remove buffer in wait queue
        while (!colist_is_list_empty(&p_uds_env->wait_queue))
        {
            ble_uds_buf_meta_t *p_meta = (ble_uds_buf_meta_t *)colist_get_head(&p_uds_env->wait_queue);
            colist_delete(&(p_meta->node));
            bes_bt_me_bes_bt_buf_free((uint8_t *)p_meta);
        }
        gatts_unregister_service(p_uds_env->p_attr_list);
        bes_bt_me_bes_bt_buf_free((uint8_t *)p_uds_env->p_attr_list);
        // free profile environment variables
        bes_bt_me_bes_bt_buf_free((uint8_t *)p_uds_env);
        p_uds_env = NULL;
    }
    return BT_STS_SUCCESS;
}
#endif /* BLE_UDP_ENABLE */
