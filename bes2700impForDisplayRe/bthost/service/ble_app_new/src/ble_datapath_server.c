/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifdef CFG_APP_DATAPATH_SERVER
#include "gatt_service.h"
#include "app_ble.h"
#include "hal_norflash.h"
#include "hal_bootmode.h"
#include "retention_ram.h"
#include "hal_trace.h"

#define USE_128BIT_UUID 1
#define DATAPATHPS_MAX_LEN (509)

#if USE_128BIT_UUID

#ifdef IS_USE_CUSTOM_BLE_DATAPATH_PROFILE_UUID_ENABLED
#define datapath_service_uuid_128_le TW_BLE_DATAPATH_SERVICE_UUID
#define datapath_tx_character_uuid_128_le TW_BLE_DATAPATH_TX_CHAR_VAL_UUID
#define datapath_rx_character_uuid_128_le TW_BLE_DATAPATH_RX_CHAR_VAL_UUID
#else
#define datapath_service_uuid_128_le 0x12,0x34,0x56,0x78,0x90,0x00,0x00,0x80,0x00,0x10,0x00,0x00,0x00,0x01,0x00,0x01
#define datapath_tx_character_uuid_128_le 0x12,0x34,0x56,0x78,0x91,0x00,0x00,0x80,0x00,0x10,0x00,0x00,0x00,0x02,0x00,0x02
#define datapath_rx_character_uuid_128_le 0x12,0x34,0x56,0x78,0x92,0x00,0x00,0x80,0x00,0x10,0x00,0x00,0x00,0x03,0x00,0x03
#endif

#define GET_BLE_CMD_OPCODE(op)     (op - CUSTOM_CMD_GROUP_INDEX_CUSTOM)

GATT_DECL_128_LE_PRI_SERVICE(g_ble_datapath_service,
    datapath_service_uuid_128_le);

GATT_DECL_128_LE_CHAR(g_ble_datapath_rx_character,
    datapath_rx_character_uuid_128_le,
    GATT_WR_REQ|GATT_WR_CMD|GATT_RD_REQ,
    ATT_SEC_NONE);

GATT_DECL_CUDD_DESCRIPTOR(g_ble_datapath_rx_cudd,
    ATT_SEC_NONE);

GATT_DECL_128_LE_CHAR(g_ble_datapath_tx_character,
    datapath_tx_character_uuid_128_le,
    GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_datapath_tx_cccd,
    ATT_WR_ENC);

GATT_DECL_CUDD_DESCRIPTOR(g_ble_datapath_tx_cudd,
    ATT_SEC_NONE);

#else /* USE_128BIT_UUID */

#define datapath_service_uuid_16 0xFEF8
#define datapath_tx_character_uuid_16 0xFEF9
#define datapath_rx_character_uuid_16 0xFEFA

GATT_DECL_PRI_SERVICE(g_ble_datapath_service,
    datapath_service_uuid_16);

GATT_DECL_CHAR(g_ble_datapath_rx_character,
    datapath_rx_character_uuid_16,
    GATT_WR_REQ|GATT_WR_CMD|GATT_RD_REQ,
    ATT_SEC_NONE);

GATT_DECL_CUDD_DESCRIPTOR(g_ble_datapath_rx_cudd,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_datapath_tx_character,
    datapath_tx_character_uuid_16,
    GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_datapath_tx_cccd,
    ATT_WR_ENC);

GATT_DECL_CUDD_DESCRIPTOR(g_ble_datapath_tx_cudd,
    ATT_SEC_NONE);

#endif /* USE_128BIT_UUID */

static const gatt_attribute_t g_ble_datapath_attr_list[] = {
    /* Service */
    gatt_attribute(g_ble_datapath_service),
    /* Characteristics */
    gatt_attribute(g_ble_datapath_rx_character),
        gatt_attribute(g_ble_datapath_rx_cudd),
    /* Characteristics */
    gatt_attribute(g_ble_datapath_tx_character),
        gatt_attribute(g_ble_datapath_tx_cccd),
        gatt_attribute(g_ble_datapath_tx_cudd),
};

struct app_datapath_server_env_tag app_datapath_server_env;
static app_datapath_event_cb dp_event_callback = NULL;
static app_datapath_server_tx_done_t tx_done_callback = NULL;
static app_datapath_server_data_received_callback_func_t rx_done_callback = NULL;
static app_datapath_server_disconnected_done_t disconnected_done_callback = NULL;
static app_datapath_server_connected_done_t connected_done_callback = NULL;
static app_datapath_server_mtuexchanged_done_t mtuexchanged_done_callback = NULL;
static app_datapath_server_role_switch_callback_t le_rs_callback = NULL;

static const char custom_tx_desc[] = "Data Path TX Data";
static const char custom_rx_desc[] = "Data Path RX Data";

static uint16_t app_datapath_server_alloc_con_info_with_conidx(uint8_t conidx)
{
    uint8_t idx = 0;

    for (idx = 0; idx < BLE_CONNECTION_MAX; idx++)
    {
        if (app_datapath_server_env.con_info[idx].connectionIndex == conidx ||
            app_datapath_server_env.con_info[idx].connectionIndex == BLE_INVALID_CONNECTION_INDEX)
        {
            app_datapath_server_env.con_info[idx].connectionIndex = conidx;
            return BT_STS_SUCCESS;
        }
    }

    return BT_STS_FAILED;
}

static uint16_t app_datapath_server_free_con_info_by_conidx(uint8_t conidx)
{
    uint8_t idx = 0;

    for (idx = 0; idx < BLE_CONNECTION_MAX; idx++)
    {
        if (app_datapath_server_env.con_info[idx].connectionIndex == conidx)
        {
            app_datapath_server_env.con_info[idx].connectionIndex = BLE_INVALID_CONNECTION_INDEX;
            app_datapath_server_env.con_info[idx].isNotificationEnabled = false;
            return BT_STS_SUCCESS;
        }
    }

    return BT_STS_FAILED;
}

static void app_datapath_server_set_latest_recv_cmd_by_conidx(uint8_t conidx, uint32_t cmd)
{
    uint8_t idx = 0;

    for (idx = 0; idx < BLE_CONNECTION_MAX; idx++)
    {
        if (app_datapath_server_env.con_info[idx].connectionIndex == conidx)
        {
            app_datapath_server_env.con_info[idx].recv_cmd_latest = cmd;
        }
    }
}

static uint8_t app_datapath_server_get_latest_recv_cmd_by_conidx(uint8_t conidx)
{
    uint8_t idx = 0;

    for (idx = 0; idx < BLE_CONNECTION_MAX; idx++)
    {
        if (app_datapath_server_env.con_info[idx].connectionIndex == conidx)
        {
            return app_datapath_server_env.con_info[idx].recv_cmd_latest;
        }
    }

    return 0;
}

static void app_datapath_server_register_tx_ntf_cccd_by_conidx(uint8_t conidx, bool enable)
{
    uint8_t idx = 0;

    for (idx = 0; idx < BLE_CONNECTION_MAX; idx++)
    {
        if (app_datapath_server_env.con_info[idx].connectionIndex == conidx)
        {
            app_datapath_server_env.con_info[idx].isNotificationEnabled = enable;
        }
    }
}

static bool app_datapath_server_get_tx_ntf_en_by_conidx(uint8_t conidx)
{
    uint8_t idx = 0;

    for (idx = 0; idx < BLE_CONNECTION_MAX; idx++)
    {
        if (app_datapath_server_env.con_info[idx].connectionIndex == conidx)
        {
            return app_datapath_server_env.con_info[idx].isNotificationEnabled;
        }
    }

    return false;
}

void app_datapath_server_send_data_via_notification(uint8_t conidx, uint8_t* data, uint32_t len)
{
    uint8_t con_idx = gap_zero_based_conidx_to_ble_conidx(conidx);
    gatts_send_notification(gap_conn_bf(con_idx), g_ble_datapath_tx_character, data, (uint16_t)len);
}

void app_datapath_server_send_data_via_indication(uint8_t conidx, uint8_t* data, uint32_t len)
{
    uint8_t con_idx = gap_zero_based_conidx_to_ble_conidx(conidx);
    gatts_send_indication(gap_conn_bf(con_idx), g_ble_datapath_tx_character, data, (uint16_t)len);
}




#define BLE_CUSTOM_CMD_WAITING_RSP_TIMEOUT_COUNT 8
#define BLE_RAW_DATA_XFER_BUF_SIZE 80

void app_datapath_server_register_event_callback(app_datapath_event_cb cb)
{
    dp_event_callback = cb;
}

static void app_datapath_server_mtu_exchanged(uint8_t con_idx, uint16_t connhdl, uint16_t mtu)
{
    uint8_t conidx = gap_zero_based_conidx(con_idx);

    if (NULL != mtuexchanged_done_callback)
    {
        mtuexchanged_done_callback(conidx, mtu);
    }

    if(dp_event_callback)
    {
        app_dp_mtu_exchange_msg_t msg_data;
        msg_data.conidx = conidx;
        msg_data.mtu    = mtu;
        dp_event_callback(DP_MTU_CHANGE_DONE, (ble_if_app_dp_param_u *)&msg_data);
    }
}

static void app_datapath_server_connected(uint8_t conidx, uint16_t connhdl)
{
    uint16_t status = app_datapath_server_alloc_con_info_with_conidx(conidx);

    if (status != BT_STS_SUCCESS)
    {
        return;
    }

    TRACE(0,"app datapath server connected.");

    app_datapath_server_register_tx_ntf_cccd_by_conidx(conidx, true);

    if (NULL != connected_done_callback)
    {
        connected_done_callback(conidx);
    }

    if (dp_event_callback)
    {
        dp_event_callback(DP_CONN_DONE, (ble_if_app_dp_param_u *)&conidx);
    }
}

static void app_datapath_server_disconnected(uint8_t con_idx, uint16_t connhdl)
{
    uint8_t conidx = gap_zero_based_conidx(con_idx);

    if (app_datapath_server_free_con_info_by_conidx(conidx) == BT_STS_SUCCESS)
    {
        TRACE(0,"app datapath server dis-connected.");

        tx_done_callback = NULL;
    }
    else
    {
        return;
    }

    if (NULL != disconnected_done_callback)
    {
        disconnected_done_callback(conidx);
    }

    if (dp_event_callback)
    {
        dp_event_callback(DP_DISCONN_DONE, (ble_if_app_dp_param_u *)&conidx);
    }
}

static void app_datapath_server_tx_ccc_changed(uint8_t con_idx, uint16_t connhdl, bool notify_enabled)
{
    uint8_t conidx = gap_zero_based_conidx(con_idx);

    if (notify_enabled)
    {
        app_datapath_server_connected(conidx, connhdl);
    }
    else
    {
        app_datapath_server_register_tx_ntf_cccd_by_conidx(conidx, false);
    }
}

static void app_datapath_server_tx_data_sent(uint8_t con_idx, uint16_t connhdl)
{
    if (NULL != tx_done_callback)
    {
        tx_done_callback();
    }

    if(dp_event_callback)
    {
        dp_event_callback(DP_TX_DONE, NULL);
    }
}

static void app_datapath_server_rx_data_received(uint8_t con_idx, uint16_t connhdl, const uint8_t *data, uint16_t len)
{
    uint8_t conidx = gap_zero_based_conidx(con_idx);
    // loop back the received data
    if (app_datapath_server_get_tx_ntf_en_by_conidx(conidx))
    {
        app_datapath_server_send_data_via_notification(conidx, (uint8_t *)data, len);
    }

    TRACE(2,"%s length %d", __func__, len);

    uint8_t cmd = *data;

    app_datapath_server_alloc_con_info_with_conidx(conidx);
    app_datapath_server_set_latest_recv_cmd_by_conidx(conidx, cmd);

    if (NULL != rx_done_callback)
    {
        rx_done_callback((uint8_t *)data, len);
    }

    if (dp_event_callback)
    {
        app_dp_rec_data_msg_t data_msg;
        data_msg.data     = (uint8_t *)data;
        data_msg.data_len = len;
        data_msg.conidx   = conidx;
        dp_event_callback(DP_DATA_RECEIVED, (ble_if_app_dp_param_u *)&data_msg);
    }
}

static void app_datapath_server_read_latest_cmd_received(uint8_t conidx, uint16_t connhdl, uint32_t token)
{
    uint8_t latest_cmd = app_datapath_server_get_latest_recv_cmd_by_conidx(conidx);
    gatts_send_defer_read_rsp(connhdl, token, 0, (uint8_t *)&latest_cmd, sizeof(latest_cmd));
}

static void ble_datapath_send_desc_read_response(uint16_t connhdl, uint32_t token, bool is_tx_desc)
{
    uint8_t *buf = is_tx_desc ? (uint8_t *)custom_tx_desc : (uint8_t *)custom_rx_desc;
    uint16_t size = is_tx_desc ? sizeof(custom_tx_desc) : sizeof(custom_rx_desc);
    gatts_send_defer_read_rsp(connhdl, token, 0, buf, size);
}

static int ble_datapath_server_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
{
    uint8_t conidx = gap_zero_based_conidx(svc->con_idx);

    switch (event)
    {
        case GATT_SERV_EVENT_CHAR_WRITE:
        {
            gatt_server_char_write_t *p = param.char_write;
            if (p->value_offset != 0 || p->value_len == 0 || p->value == NULL)
            {
                return false;
            }
            app_datapath_server_rx_data_received(svc->con_idx, svc->connhdl, p->value, p->value_len);
            // Here for validate async call write rsp
            bt_thread_call_func_3(gatts_send_defer_write_rsp, bt_fixed_param(svc->connhdl),
                                                                bt_fixed_param(p->ctx->token),
                                                                bt_fixed_param(0));
            return true;
        }
        case GATT_SERV_EVENT_CHAR_READ:
        {
            gatt_server_char_read_t *p = param.char_read;
            if (p->value_offset != 0)
            {
                return false;
            }

            // Here for validate async call write rsp
            bt_thread_call_func_3(app_datapath_server_read_latest_cmd_received,
                                                                bt_fixed_param(conidx),
                                                                bt_fixed_param(svc->connhdl),
                                                                bt_fixed_param(p->ctx->token));
            return true;
        }
        case GATT_SERV_EVENT_DESC_WRITE:
        {
            gatt_server_desc_write_t *p = param.desc_write;
            uint16_t config = CO_COMBINE_UINT16_LE(p->value);
            bool notify_enabled = false;
            if (config & GATT_CCCD_SET_NOTIFICATION)
            {
                notify_enabled = true;
            }
            app_datapath_server_tx_ccc_changed(svc->con_idx, svc->connhdl, notify_enabled);
            // Here for validate async call write rsp
            bt_thread_call_func_3(gatts_send_defer_write_rsp, bt_fixed_param(svc->connhdl),
                                                                bt_fixed_param(p->ctx->token),
                                                                bt_fixed_param(0));
            return true;
        }
        case GATT_SERV_EVENT_NTF_TX_DONE:
        case GATT_SERV_EVENT_INDICATE_CFM:
        {
            app_datapath_server_tx_data_sent(svc->con_idx, svc->connhdl);
            break;
        }
        case GATT_SERV_EVENT_MTU_CHANGED:
        {
            gatt_server_mtu_changed_t *p = param.mtu_changed;
            app_datapath_server_mtu_exchanged(svc->con_idx, svc->connhdl, p->mtu);
            break;
        }
        case GATT_SERV_EVENT_CONN_CLOSED:
        {
            app_datapath_server_disconnected(svc->con_idx, svc->connhdl);
            break;
        }
        case GATT_SERV_EVENT_DESC_READ:
        {
            gatt_server_desc_read_t *p = param.desc_read;
            // Check cccd notify enable bit
            if ((uint8_t *)p->desc_attr->attr_data == g_ble_datapath_tx_cccd)
            {
                uint16_t cccd_config = co_host_to_uint16_le(
                app_datapath_server_get_tx_ntf_en_by_conidx(conidx) ? 0x0001 : 0x0000);
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)&cccd_config, sizeof(cccd_config));
                return true;
            }

            bool is_tx_desc = ((uint8_t *)p->desc_attr->attr_data == g_ble_datapath_tx_cudd);
            // Here for validate async call read rsp
            bt_thread_call_func_3(ble_datapath_send_desc_read_response,
                                                        bt_fixed_param(svc->connhdl),
                                                        bt_fixed_param(p->ctx->token),
                                                        bt_fixed_param(is_tx_desc));
            return true;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

void app_datapath_server_register_event_cb(app_datapath_event_cb callback)
{
    dp_event_callback = callback;
}

void app_datapath_server_register_tx_done(app_datapath_server_tx_done_t callback)
{
    tx_done_callback = callback;
}

void app_datapath_server_register_rx_done(app_datapath_server_data_received_callback_func_t callback)
{
    rx_done_callback = callback;
}

void app_datapath_server_register_disconnected_done(app_datapath_server_disconnected_done_t callback)
{
    disconnected_done_callback = callback;
}

void app_datapath_server_register_connected_done(app_datapath_server_connected_done_t callback)
{
    connected_done_callback = callback;
}

void app_datapath_server_register_mtu_exchanged_done(app_datapath_server_mtuexchanged_done_t callback)
{
    mtuexchanged_done_callback = callback;
}

void app_datapath_server_register_le_rs_callback(app_datapath_server_role_switch_callback_t callback)
{
    le_rs_callback = callback;
}

void ble_datapath_server_init(void)
{
    uint8_t idx = 0;

    for (idx = 0; idx < BLE_CONNECTION_MAX; idx++)
    {
        app_datapath_server_env.con_info[idx].connectionIndex = BLE_INVALID_CONNECTION_INDEX;
        app_datapath_server_env.con_info[idx].isNotificationEnabled = false;
    }

    gatts_register_service(g_ble_datapath_attr_list, ARRAY_SIZE(g_ble_datapath_attr_list), ble_datapath_server_callback, NULL);
}

uint32_t ble_datapath_save_ctx(uint8_t conidx, uint8_t *buf, uint32_t buf_len)
{
    uint16_t offset = 0;
    uint8_t idx = 0;

    TRACE(1, "%s conidx:%d", __func__, conidx);

    if (conidx >= BLE_CONNECTION_MAX)
    {
        return BT_STS_NOT_FOUND;
    }

    if (buf_len < sizeof(uint8_t))
    {
        TRACE(1, "%s no more ctx buf:%d", __func__, buf_len);
        return offset;
    }

    for (idx = 0; idx < BLE_CONNECTION_MAX; idx++)
    {
        if (app_datapath_server_env.con_info[idx].connectionIndex == conidx)
        {
            break;
        }
    }

    BTIF_CTX_INIT(buf);

    if (idx < BLE_CONNECTION_MAX)
    {
        BTIF_CTX_STR_VAL8(app_datapath_server_env.con_info[idx].recv_cmd_latest);
        BTIF_CTX_STR_VAL8(app_datapath_server_env.con_info[idx].isNotificationEnabled);
    }
    else
    {
        BTIF_CTX_STR_VAL8(0xFF);
    }

    BTIF_CTX_SAVE_UPDATE_DATA_LEN();
    offset += BTIF_CTX_GET_TOTAL_LEN();

    // Custom save context callback
    if (le_rs_callback != NULL)
    {
        offset += le_rs_callback(conidx, false, buf + offset, buf_len - offset);
    }

    return offset;
}

uint32_t ble_datapath_restore_ctx(uint8_t conidx, uint8_t *buf, uint32_t buf_len)
{
    uint16_t offset = 0;
    uint8_t is_notify_enabled = false;
    uint8_t cmd_code = 0;
    uint8_t con_idx = 0;

    TRACE(1, "%s conidx:%d", __func__, conidx);

    if (conidx >= BLE_CONNECTION_MAX)
    {
        return BT_STS_NOT_FOUND;
    }

    if (buf_len < sizeof(uint8_t))
    {
        TRACE(1, "%s no more ctx buf:%d", __func__, buf_len);
        return offset;
    }

    BTIF_CTX_INIT(buf);

    BTIF_CTX_LDR_VAL8(cmd_code)

    con_idx = gap_zero_based_conidx_to_ble_conidx(conidx);

    if (cmd_code != 0xFF)
    {
        uint16_t status = app_datapath_server_alloc_con_info_with_conidx(conidx);

        if (status == BT_STS_SUCCESS)
        {
            BTIF_CTX_LDR_VAL8(is_notify_enabled);
            // Restore cmd latest write
            app_datapath_server_set_latest_recv_cmd_by_conidx(conidx, cmd_code);
            // Callback upper in bthread
            bt_thread_call_func_3(app_datapath_server_tx_ccc_changed,
                                                        bt_fixed_param(con_idx),
                                                        bt_fixed_param(gap_conn_hdl(con_idx)),
                                                        bt_fixed_param(is_notify_enabled));
        }
    }

    offset += BTIF_CTX_GET_TOTAL_LEN();

    // Custom restore context callback
    if (le_rs_callback != NULL)
    {
        offset += le_rs_callback(conidx, true, buf + offset, buf_len - offset);
    }

    return offset;
}

#ifdef __SW_IIR_EQ_PROCESS__
int audio_config_eq_iir_via_config_structure(uint8_t *buf, uint32_t  len);
void BLE_iir_eq_handler(uint32_t funcCode, uint8_t* ptrParam, uint32_t paramLen)
{

    audio_config_eq_iir_via_config_structure(BLE_custom_command_raw_data_buffer_pointer(), BLE_custom_command_received_raw_data_size());
}
#endif

#endif /* CFG_APP_DATAPATH_SERVER */
