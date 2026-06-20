/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#if BLE_AUDIO_ENABLED
#ifdef AOB_MOBILE_ENABLED
#include "bluetooth.h"

#include "app_gaf.h"
#include "ble_app_dbg.h"

#include "app_acc_ots_msg.h"

#include "ots.h"

/*VARIABLE*/
const ot_object_id_t g_ot_obj_id[] =
{
    {{0x00, 0x00, 0x00, 0x00, 0x01, 0x01}},
    {{0x00, 0x00, 0x00, 0x00, 0x01, 0x02}},
    {{0x00, 0x00, 0x00, 0x00, 0x01, 0x03}},
    {{0x00, 0x00, 0x00, 0x00, 0x01, 0x04}},
    {{0x00, 0x00, 0x00, 0x00, 0x01, 0x05}},
    {{0x00, 0x00, 0x00, 0x00, 0x01, 0x06}},
    {{0x00, 0x00, 0x00, 0x00, 0x01, 0x07}}
};

static uint8_t test_data[APP_ACC_DFT_LOCAL_MAX_SDU] = {0};

/*FUNCTIONS*/

/// Callback function called when client configuration for an instance of
///  the Object Transfer Service has been updated
void app_acc_ots_cb_bond_data(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type, uint16_t cli_cfg_bf)
{
    LOG_D("app_acc ots bond_data_ind con_lid= %d, transfer_lid = %d, char_type = %d, cli_cfg_bf = %02x",
          con_lid, transfer_lid, char_type, cli_cfg_bf);
}

/// Callback function called when an LE Credit Based Connection Oriented Link has
/// been established for an instance of the Object Transfer Service
void app_acc_ots_cb_coc_connected(uint8_t con_lid, uint16_t rx_mtu, uint16_t tx_mtu)
{
    LOG_D("app_acc ots coc_connected_ind con_lid= %d, rx_mtu = %d, tx_mtu = %d",
          con_lid, rx_mtu, tx_mtu);
}

/// Callback function called when an LE Credit Based Connection Oriented Link has
/// been disconnected for an instance of the Object Transfer Service
void app_acc_ots_cb_coc_disconnected(uint8_t con_lid, uint16_t reason)
{
    LOG_D("app_acc ots coc_disconnected_ind con_lid= %d, reason = %04x",
          con_lid, reason);
}

/// Callback function called when data is received through LE Credit Based Connection
/// Oriented Link for an instance of the Object Transfer Service
void app_acc_ots_cb_coc_data(uint8_t con_lid, uint16_t length, const uint8_t *p_sdu)
{
    LOG_D("app_acc ots coc_data_ind con_lid= %d, len = %d, sdu:",
          con_lid, length);

    DUMP8("%02x ", p_sdu, length);

    if (length <= APP_ACC_COC_TEST_SDU_LEN)
    {
        memcpy(test_data, p_sdu, length);
    }
    else
    {
        memcpy(test_data, p_sdu, APP_ACC_COC_TEST_SDU_LEN);
    }
}

/// Callback function called in order to retrieve name of current object
/// for an instance of the Object Transfer Service
void app_acc_ots_cb_get_name(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid, uint16_t offset)
{
    ots_cfm_get_name(BT_STS_SUCCESS, con_lid, transfer_lid, sizeof(APP_ACC_DFT_OTS_NAME), (uint8_t *)APP_ACC_DFT_OTS_NAME);
}

/// Callback function called when a peer Client requests to set the object name
/// for an instance of the Object Transfer Service
void app_acc_ots_cb_set_name(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid, uint8_t name_len, const uint8_t *p_name)
{
    ots_cfm_set_name(BT_STS_SUCCESS, con_lid, transfer_lid);
}

/// Callback function called when a peer Client requests to create a new object
/// for an instance of the Object Transfer Service
void app_acc_ots_cb_object_create(uint8_t con_lid, uint8_t transfer_lid, uint32_t size, uint8_t uuid_type, const void *p_uuid)
{
    ots_cfm_object_control(BT_STS_SUCCESS, con_lid, transfer_lid, OTP_OACP_RESULT_SUCCESS, 0);
}

/// Callback function called when a peer Client requests to manipulate
/// (read, write, calculate checksum) current object for an instance of
/// the Object Transfer Service
void app_acc_ots_cb_object_execute(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid, uint16_t param_len, const uint8_t *p_param)
{
    ots_cfm_object_execute(BT_STS_SUCCESS, con_lid, transfer_lid, OTP_OACP_RESULT_SUCCESS, param_len, p_param);
}

/// Callback function called when a peer Client requests to manipulate
/// (read, write, calculate checksum) current object for an instance of
/// the Object Transfer Service
void app_acc_ots_cb_object_manipulate(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid, uint8_t opcode, uint32_t offset, uint32_t length, uint8_t mode)
{
    ots_cfm_object_control(BT_STS_SUCCESS, con_lid, transfer_lid, OTP_OACP_RESULT_SUCCESS, 0);
}

/// Callback function called when a peer Client requests an operation on the
/// current object for an instance of the Object Transfer Service
void app_acc_ots_cb_object_control(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid, uint8_t opcode)
{
    ots_cfm_object_control(BT_STS_SUCCESS, con_lid, transfer_lid, OTP_OACP_RESULT_SUCCESS, 0);
}

/// Callback function called when a peer Client requests to get one of the
/// current filter rules for an instance of the Object Transfer Service
void app_acc_ots_cb_filter_get(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid, uint32_t ots_token, uint16_t offset)
{
    ots_cfm_filter_get(BT_STS_SUCCESS, con_lid, transfer_lid, OTP_FILTER_TYPE_NO_FILTER);
}

/// Callback function called when a peer Client requests information about
/// the list of objects or request to change current object for an instance of
/// the Object Transfer Service
void app_acc_ots_cb_list(uint16_t req_ind_code, uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode, const void *p_value)
{
    ots_cfm_list_control(BT_STS_SUCCESS, con_lid, transfer_lid, OTP_OLCP_RESULT_SUCCESS, 1);
}

/// Callback function called when a peer Client requests to set one of the
/// current filter rules for an instance of the Object Transfer Service
void app_acc_ots_cb_filter_set(uint16_t req_ind_code, uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid, uint8_t filter_val, const void *p_value1, const void *p_value2)
{
    ots_cfm_filter_set(BT_STS_SUCCESS, con_lid, transfer_lid);
}

/// Callback function called when a peer Client requests to establish a LE Credit
/// Based Connection Oriented Link for an instance of the Object Transfer Service
void app_acc_ots_cb_coc_connect(uint8_t con_lid, uint16_t token, uint16_t peer_mtu)
{
    LOG_D("app_acc ots coc_connect rx_mtu = %d, token = 0x%x", peer_mtu, token);
    ots_cfm_coc_connect(BT_STS_SUCCESS, con_lid, token);
}

/// Callback function called when a command has been handled
void app_acc_ots_cb_cmp_evt(uint16_t cmd_code, uint16_t status, uint8_t con_lid)
{
    if (status != BT_STS_SUCCESS)
    {
        LOG_W("app_acc ots_cmp_evt: cmd_code = %d, status = %04x",
              cmd_code - OTS_COC_DISCONNECT, status);
        return;
    }

    switch (cmd_code)
    {
        case (OTS_COC_DISCONNECT):
        {
            LOG_D("app_acc ots coc disconnect cmp, con_lid = %d", con_lid);
        }
        break;

        case (OTS_COC_SEND):
        {
            LOG_D("app_acc ots coc send cmp, con_lid = %d", con_lid);
        }
        break;

        case (OTS_COC_RELEASE):
        {
            LOG_D("app_acc ots coc release cmp, con_lid = %d", con_lid);
        }
        break;

        default:
            break;
    }
}

static ots_evt_cb_t app_acc_ots_cb =
{
    .cb_bond_data = app_acc_ots_cb_bond_data,
    .cb_cmp_evt = app_acc_ots_cb_cmp_evt,
    .cb_coc_data = app_acc_ots_cb_coc_data,
    .cb_coc_connect = app_acc_ots_cb_coc_connect,
    .cb_coc_disconnected = app_acc_ots_cb_coc_disconnected,
    .cb_coc_connected = app_acc_ots_cb_coc_connected,
    .cb_filter_get = app_acc_ots_cb_filter_get,
    .cb_get_name = app_acc_ots_cb_get_name,
    .cb_list = app_acc_ots_cb_list,
    .cb_filter_set = app_acc_ots_cb_filter_set,
    .cb_object_control = app_acc_ots_cb_object_control,
    .cb_object_execute = app_acc_ots_cb_object_execute,
    .cb_object_manipulate = app_acc_ots_cb_object_manipulate,
    .cb_object_create = app_acc_ots_cb_object_create,
    .cb_set_name = app_acc_ots_cb_set_name,
};

int app_acc_ots_init(void)
{
    LOG_I("%s", __func__);

    ots_init_cfg_t ots_init_cfg =
    {
        .nb_transfers_supp_max = APP_ACC_DFT_OTS_NUM,
    };

    uint16_t status = ots_init(&ots_init_cfg, &app_acc_ots_cb);

    if (status == BT_STS_SUCCESS)
    {
        ots_inst_cfg_t ots_inst_cfg =
        {
            .pref_mtu = GAF_PREFERRED_MTU,
            .is_primary_service = false,
            .oacp_features = ~OTP_OACP_FEAT_RFU_MASK,
            .olcp_features = ~OTP_OLCP_FEAT_RFU_MASK,
        };

        uint8_t transfer_lid_ret = 0;

        uint8_t idx = 0;

        for (idx = 0; idx < APP_ACC_DFT_OTS_NUM; idx++)
        {
            // Only one ots secondary service is included by mcs
            ots_inst_cfg.is_primary_service = (idx == 0) ? false : true;

            status = ots_add_ots_instant(&ots_inst_cfg, &transfer_lid_ret);

            if (status != BT_STS_SUCCESS)
            {
                LOG_E("acc_ots add instant failed");
                return status;
            }

            LOG_I("acc ots add instant lid = %d", transfer_lid_ret);
        }
    }

    return BT_STS_SUCCESS;
}

int app_acc_ots_deinit(void)
{
    LOG_I("%s", __func__);

    return ots_deinit();
}

int app_acc_ots_start(uint8_t con_lid)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_acc_ots_coc_disconnect(uint8_t con_lid)
{
    return ots_coc_disconnect(con_lid);
}

int app_acc_ots_coc_send(uint8_t con_lid, uint8_t length, uint8_t *sdu)
{
    return ots_coc_send(con_lid, length, sdu);
}

int app_acc_ots_coc_release(uint8_t con_lid)
{
    return ots_coc_release(con_lid);
}

/* add a object(NOTICE: a object can be exposed to multi clients) */
int app_acc_ots_obj_add(uint8_t mc_obj_idx, uint32_t allocated_size, uint16_t uuid)
{
    gaf_prf_date_time_t test_time =
    {
        .year = 2023,
        .month = 6,
        .day = 20,
        .hour = 19,
        .min = 29,
        .sec = 59,
    };

    uint16_t uuid_test = 0x1234;

    uint8_t obj_lid = 0;

    uint16_t status = ots_object_add(&g_ot_obj_id[mc_obj_idx], 0, allocated_size, &test_time, &test_time, ~(OTP_PROP_RFU_MASK), 0, &uuid_test, &obj_lid);

    if (status == BT_STS_SUCCESS)
    {
        LOG_D("acc ots add ots obj lid = %d", obj_lid);
    }

    return status;
}

int app_acc_ots_obj_remove(uint8_t object_lid)
{
    return ots_object_remove(object_lid);
}

/*change current object that expose to clients(such as: next track in media control)*/
int app_acc_ots_obj_change(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid)
{
    return ots_object_change(con_lid, transfer_lid, object_lid);
}

/* indicate clients when object content changed*/
int app_acc_ots_obj_changed(uint8_t flags, uint8_t *object_id)
{
    return ots_object_changed(flags, (ot_object_id_t *)object_id);
}

/*
set_type:
/// Set Current Size value
OTS_SET_TYPE_CURRENT_SIZE = 0,
/// Set Allocated Size value
OTS_SET_TYPE_ALLOCATED_SIZE,
/// Set number of objects
OTS_SET_TYPE_NUM_OBJECTS,
/// Set Properties value
OTS_SET_TYPE_PROPERTIES,
*/
int app_acc_ots_set(uint8_t object_lid, uint8_t set_type, uint32_t val)
{
    return ots_set(object_lid, set_type, val);
}

int app_acc_ots_set_time(uint8_t object_lid, app_gaf_prf_date_time_t *time)
{
    return ots_set_time(object_lid, (gaf_prf_date_time_t *)time);
}

#endif
#endif

/// @} APP
