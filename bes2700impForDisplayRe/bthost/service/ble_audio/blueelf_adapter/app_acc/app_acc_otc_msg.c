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
#include "bluetooth.h"

#include "app_gaf.h"
#include "ble_app_dbg.h"
#include "app_gaf_custom_api.h"

#include "app_acc_otc_msg.h"

#include "otc.h"

/*CALLBACKS*/

/// Callback function called when a discovery is complete
void app_acc_otc_cb_discovery_cmp_evt(uint8_t con_lid, uint16_t err_code)
{
    LOG_I("app_acc otc discovery cmp, con_lid = %d, err_code = %d", con_lid, err_code);
}

/// Callback function called when an instance of the Object Transfer Service
/// has been found in Server device database
void app_acc_otc_cb_bond_data(uint8_t con_lid, uint8_t transfer_lid, const otc_prf_svc_info_t *param)
{
    LOG_D("app_acc otc service found transfer_lid = %d, shdl = %04x, ehdl = %04x",
          transfer_lid, param->svc_range.shdl, param->svc_range.ehdl);
}

/// Callback for otc gatt set cfg cmp evt
void app_acc_otc_cb_set_cfg_cmp_evt(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type, uint16_t err_code)
{
    LOG_D("app_acc otc set cfg cmp, con_lid = %d, transfer_lid = %d, char_type = %d, err_code = %d",
          con_lid, transfer_lid, char_type, err_code);
}

/// Callback function called when non-empty execution response for current object of
/// an instance of the Object Transfer Service has been received
void app_acc_otc_cb_execute_rsp(uint8_t con_lid, uint8_t transfer_lid, uint16_t rsp_len,
                                const uint8_t *p_rsp)
{
    LOG_D("app_acc p_execute_rsp_ind con_lid= %d, transfer_lid = %d, rsp_len = %d",
          con_lid, transfer_lid, rsp_len);

    DUMP8("%02x ", p_rsp, rsp_len);
}

/// Callback function called when information about current object have been received
void app_acc_otc_cb_value(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type,
                          uint32_t value1, uint32_t value2)
{
    LOG_D("app_acc otc_val_ind con_lid= %d, transfer_lid = %d, char_type = %d",
          con_lid, transfer_lid, char_type);

    if (OTP_CHAR_TYPE_FEATURE == char_type)
    {
        LOG_D("app_acc otc_val_ind oacp_features = %08x, olcp_features = %08x",
              value1, value2);
    }
    else if (OTP_CHAR_TYPE_PROPERTIES == char_type)
    {
        LOG_D("app_acc otc_val_ind properties = %08x", value1);
    }

    else if (OTP_CHAR_TYPE_SIZE == char_type)
    {
        LOG_D("app_acc otc_val_ind cur_size = %u, alloc_size = %u",
              value1, value2);
    }
}

/// Callback function called when either time of creation or time of last update
/// for current object for an instance of the Object Transfer Service has been received
void app_acc_otc_cb_time(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type,
                         const gaf_prf_date_time_t *p_time)
{
    LOG_D("app_acc otc_time_ind con_lid= %d, transfer_lid = %d, char_type = %d",
          con_lid, transfer_lid, char_type);

    if (char_type == OTP_CHAR_TYPE_FIRST_CREATED)
    {
        LOG_D("app_acc otc_first_create_time %d-%d-%d %d:%d:%d",
              p_time->year, p_time->month, p_time->day,
              p_time->hour, p_time->min, p_time->sec);
    }
    else if (char_type == OTP_CHAR_TYPE_LAST_MODIFIED)
    {
        LOG_D("app_acc otc_last_modified_time %d-%d-%d %d:%d:%d",
              p_time->year, p_time->month, p_time->day,
              p_time->hour, p_time->min, p_time->sec);
    }
}

/// Callback function called when a alcp command has been handled
void app_acc_otc_cb_olcp_cmp_evt(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode, uint8_t result_code, uint32_t checksum, uint16_t err_code)
{
    LOG_D("app_acc otc olcp cmp, con_lid = %d, transfer_lid = %d, op_code = %d, result_code = %d, checksum = %d, err_code = %d",
          con_lid, transfer_lid, opcode, result_code, checksum, err_code);
}

/// Callback function called when a oacp command has been handled
void app_acc_otc_cb_oacp_cmp_evt(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode, uint8_t result_code, uint8_t nb_obj, uint16_t err_code)
{
    LOG_D("app_acc otc oacp cmp, con_lid = %d, transfer_lid = %d, op_code = %d, result_code = %d, nb_objects = %d, err_code = %d",
          con_lid, transfer_lid, opcode, result_code, nb_obj, err_code);
}

/// Callback function called when a command has been handled
void app_acc_otc_cb_set_cmp_evt(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type, uint8_t char_inst_id, uint16_t err_code)
{
    LOG_D("app_acc otc set cmp, con_lid = %d, transfer_lid = %d, char_type = %d, filter_lid = %d, err_code = %d",
          con_lid, transfer_lid, char_type, char_inst_id, err_code);
}

/// Callback function called when Object ID of current object for an instance of
/// the Object Transfer Service has been received
void app_acc_otc_cb_object_id(uint8_t con_lid, uint8_t transfer_lid, const ot_object_id_t *p_object_id)
{
    LOG_D("app_acc otc_obj_id_ind con_lid= %d, transfer_lid = %d, object_id:",
          con_lid, transfer_lid);

    DUMP8("%02x ", p_object_id->object_id, OTP_ID_LEN);
}

/// Callback function called when UUID of current object for an instance of the
/// Object Transfer Service has been received
void app_acc_otc_cb_type(uint8_t con_lid, uint8_t transfer_lid, uint8_t uuid_type, const void *p_uuid)
{
    LOG_D("app_acc otc_type_ind con_lid= %d, transfer_lid = %d, uuid_type = %d, uuid = %04x",
          con_lid, transfer_lid, uuid_type, *(uint16_t *)p_uuid);
}

/// Callback function called when name of current object for an instance of the
/// Object Transfer Service has been received
void app_acc_otc_cb_name(uint8_t con_lid, uint8_t transfer_lid, uint8_t name_len, const uint8_t *p_name)
{
    LOG_D("app_acc otc_name_ind con_lid = %d, transfer_lid = %d, name_len = %d, name:",
          con_lid, transfer_lid, name_len);

    DUMP8("%02x ", p_name, name_len);
}

/// Callback function called when a filter rule has been received
void app_acc_otc_cb_filter(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid,
                           uint8_t filter_val, const void *p_value1, const void *p_value2)
{
    LOG_D("app_acc otc_filter_ind con_lid= %d, transfer_lid = %d", con_lid, transfer_lid);

    switch (filter_val)
    {
        case (OTP_FILTER_TYPE_CREATED_BETW):
        case (OTP_FILTER_TYPE_MODIFIED_BETW):
        {
            gaf_prf_date_time_t *p_time = (gaf_prf_date_time_t *)p_value1;

            LOG_D("otc_filter_time_start_ind %d-%d-%d %d:%d:%d",
                  p_time->year, p_time->month,
                  p_time->day, p_time->hour,
                  p_time->min, p_time->sec);

            p_time = (gaf_prf_date_time_t *)p_value2;

            LOG_D("otc_filter_time_end_ind %d-%d-%d %d:%d:%d",
                  p_time->year, p_time->month,
                  p_time->day, p_time->hour,
                  p_time->min, p_time->sec);
        }
        break;
        case (OTP_FILTER_TYPE_CURRENT_SIZE_BETW):
        case (OTP_FILTER_TYPE_ALLOCATED_SIZE_BETW):
        {
            LOG_D("otc_filter_type size_min = %d, size_max = %d", *(uint32_t *)p_value1, *(uint32_t *)p_value2);
        }
        break;
        case (OTP_FILTER_TYPE_NAME_STARTS_WITH):
        case (OTP_FILTER_TYPE_NAME_ENDS_WITH):
        case (OTP_FILTER_TYPE_NAME_CONTAINS):
        case (OTP_FILTER_TYPE_NAME_IS_EXACTLY):
        {
            LOG_D("otc_filter_name_ind name_len = %d, name:", *(uint16_t *)p_value1);

            DUMP8("%c", p_value2, *(uint16_t *)p_value1);
        }
        break;
        case (OTP_FILTER_TYPE_OBJECT_TYPE):
        {
            LOG_D("otc_filter_type_ind uuid_type = %d, uuid = %04x",
                  *(uint8_t *)p_value1, *(uint16_t *)p_value2);
        }
        break;
        case (OTP_FILTER_TYPE_NO_FILTER):
        case (OTP_FILTER_TYPE_MARKED_OBJECTS):
        default:
            break;
    }
}

/// Callback function called when peer Server notifies that current object for
/// an instance of the Object Transfer Service has been updated
void app_acc_otc_cb_changed(uint8_t con_lid, uint8_t transfer_lid, uint8_t flags,
                            const ot_object_id_t *p_object_id)
{
    LOG_D("app_acc otc_changed_ind con_lid= %d, transfer_lid = %d, flags = %d, object_id:",
          con_lid, transfer_lid, flags);

    DUMP8("%02x ", p_object_id->object_id, OTP_ID_LEN);
}

/// Callback function called when an LE Credit Based Connection Oriented Link has
/// been established
void app_acc_otc_cb_coc_connected(uint8_t con_lid, uint16_t peer_mtu, uint16_t local_mtu)
{
    LOG_D("app_acc otc_coc_connected_ind con_lid= %d, peer_mtu = %d, local_mtu = %d",
          con_lid, peer_mtu, local_mtu);
}

/// Callback function called when an LE Credit Based Connection Oriented Link has
/// been disconnected for an instance of the Object Transfer Service
void app_acc_otc_cb_coc_disconnected(uint8_t con_lid, uint16_t reason)
{
    LOG_D("app_acc otc_coc_disconnected_ind con_lid= %d, reason = %04x",
          con_lid, reason);
}

/// Callback function called when data is received through LE Credit Based Connection
/// Oriented Link for an instance of the Object Transfer Service
void app_acc_otc_cb_coc_data(uint8_t con_lid, uint16_t length, const uint8_t *p_sdu)
{
    LOG_D("app_acc otc_coc_data_ind con_lid= %d, length = %d, sdu:",
          con_lid, length);

    DUMP8("%02x ", p_sdu, length);
}

/// Callback function called when a coc send command has been handled
void app_acc_otc_cb_coc_send_cmp_evt(uint8_t con_lid, uint16_t err_code)
{

}

static void app_acc_otc_cb_prf_status_event(uint8_t con_lid, bool is_central, gatt_prf_status_event_e event)
{

}

static otc_evt_cb_t app_acc_otc_callback =
{
    .cb_bond_data = app_acc_otc_cb_bond_data,
    .cb_discovery_cmp = app_acc_otc_cb_discovery_cmp_evt,
    .cb_set_cfg_cmp = app_acc_otc_cb_set_cfg_cmp_evt,
    .cb_changed = app_acc_otc_cb_changed,
    .cb_filter = app_acc_otc_cb_filter,
    .cb_object_id = app_acc_otc_cb_object_id,
    .cb_time = app_acc_otc_cb_time,
    .cb_name = app_acc_otc_cb_name,
    .cb_type = app_acc_otc_cb_type,
    .cb_value = app_acc_otc_cb_value,
    .cb_set_cmp_evt = app_acc_otc_cb_set_cmp_evt,
    .cb_execute_rsp = app_acc_otc_cb_execute_rsp,
    .cb_oacp_cmp_evt = app_acc_otc_cb_oacp_cmp_evt,
    .cb_olcp_cmp_evt = app_acc_otc_cb_olcp_cmp_evt,
    .cb_coc_connected = app_acc_otc_cb_coc_connected,
    .cb_coc_disconnected = app_acc_otc_cb_coc_disconnected,
    .cb_coc_data = app_acc_otc_cb_coc_data,
    .cb_coc_send_cmp = app_acc_otc_cb_coc_send_cmp_evt,
    .cb_prf_status_event = app_acc_otc_cb_prf_status_event,
};

int app_acc_otc_init(void)
{
    LOG_I("%s", __func__);

    otc_init_cfg_t otc_init_cfg =
    {
        .supp_ots_found_max = 2,
    };

    return otc_init(&otc_init_cfg, &app_acc_otc_callback);
}

int app_acc_otc_deinit(void)
{
    LOG_I("%s", __func__);

    return otc_deinit();
}

int app_acc_otc_start(uint8_t con_lid)
{
    return otc_service_discovery(con_lid);
}

int app_acc_otc_get(uint8_t con_lid, uint8_t transfer_lid, uint8_t get_type, uint8_t char_type)
{
    return otc_character_value_read(con_lid, transfer_lid, char_type);
}

int app_acc_otc_get_cfg(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_acc_otc_set_cfg(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type, uint8_t enable)
{
    return otc_character_cccd_write(con_lid, transfer_lid, char_type, enable);
}

int app_acc_otc_set_name(uint8_t con_lid, uint8_t transfer_lid, uint8_t *name, uint8_t name_len)
{
    return otc_set_name(con_lid, transfer_lid, name_len, name);
}

int app_acc_otc_set_time(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type,
                         app_gaf_prf_date_time_t *time)
{
    return otc_set_time(con_lid, transfer_lid, char_type, (gaf_prf_date_time_t *)time);
}

int app_acc_otc_set_properties(uint8_t con_lid, uint8_t transfer_lid, uint32_t properties)
{
    return otc_set_properties(con_lid, transfer_lid, properties);
}

int app_acc_otc_object_create(uint8_t con_lid, uint8_t transfer_lid, uint8_t size, uint16_t uuid)
{
    return otc_object_create(con_lid, transfer_lid, size, OTP_UUID_TYPE_16_BIT, (uint8_t *)&uuid);
}

int app_acc_otc_object_control(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode)
{
    return otc_object_control(con_lid, transfer_lid, opcode);
}

int app_acc_otc_object_manipulate(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode,
                                  uint32_t offset, uint32_t length, uint8_t mode)
{
    return otc_object_manipulate(con_lid, transfer_lid, opcode, offset, length, mode);
}

int app_acc_otc_object_execute(uint8_t con_lid, uint8_t transfer_lid, uint8_t param_len,
                               uint8_t *param)
{
    return otc_object_execute(con_lid, transfer_lid, param_len, param);
}

int app_acc_otc_list_control(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode, uint8_t order)
{
    return otc_list_control(con_lid, transfer_lid, opcode, order);
}

int app_acc_otc_list_goto(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode, uint8_t *object_id)
{
    return otc_list_goto(con_lid, transfer_lid, opcode, (ot_object_id_t *)object_id);
}

int app_acc_otc_list_filter_set(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid,
                                uint8_t filter_val)
{
    return otc_filter_set(con_lid, transfer_lid, filter_lid, filter_val);
}

int app_acc_otc_list_filter_set_time(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid,
                                     uint8_t filter_val, app_gaf_prf_date_time_t *time_start, app_gaf_prf_date_time_t *time_end)
{
    return otc_filter_set_time(con_lid, transfer_lid, filter_lid, filter_val, (gaf_prf_date_time_t *)time_start, (gaf_prf_date_time_t *)time_end);
}

int app_acc_otc_list_filter_set_size(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid,
                                     uint8_t filter_val, uint32_t size_min, uint32_t size_max)
{
    return otc_filter_set_size(con_lid, transfer_lid, filter_lid, filter_val, size_min, size_max);
}

int app_acc_otc_list_filter_set_name(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid,
                                     uint8_t filter_val, uint32_t *name, uint32_t name_len)
{
    return otc_filter_set_name(con_lid, transfer_lid, filter_lid, filter_val, name_len, (uint8_t *)name);
}

int app_acc_otc_list_filter_set_type(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid,
                                     uint16_t uuid)
{
    return otc_filter_set_type(con_lid, transfer_lid, filter_lid, OTP_UUID_TYPE_16_BIT, (uint8_t *)&uuid);
}

int app_acc_otc_coc_connect(uint8_t con_lid, uint16_t local_max_sdu)
{
    return otc_coc_connect(con_lid, local_max_sdu);
}

int app_acc_otc_coc_disconnect(uint8_t con_lid)
{
    return otc_coc_disconnect(con_lid);
}

int app_acc_otc_coc_send(uint8_t con_lid, uint8_t length, uint8_t *sdu)
{
    return otc_coc_send(con_lid, length, sdu);
}

int app_acc_otc_coc_release(uint8_t con_lid)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

#endif

/// @} APP
