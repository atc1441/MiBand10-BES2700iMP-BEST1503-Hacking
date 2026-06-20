/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifdef BT_PBAP_SUPPORT

#include <stdio.h>
#include "cmsis_os.h"
#include "hal_uart.h"
#include "hal_timer.h"
#include "lockcqueue.h"
#include "hal_trace.h"
#include "bluetooth.h"
#include "besbt.h"
#include "cqueue.h"
#include "btapp.h"
#include "app_bt.h"
#include "apps.h"
#include "besbt_cfg.h"
#include "app_bt_func.h"
#include "app_bt_cmd.h"
#include "app_pbap.h"
#include "pbap_api.h"

static void (*g_app_bt_pbap_obex_connected_handler)(uint8_t device_t, struct btif_pbap_channel_t *pbap_ctl);
static void (*g_app_bt_pbap_vcard_listing_item_handler)(uint8_t device_t, struct btif_pbap_channel_t *pbap_ctl, const struct pbap_vcard_listing_item_t *item);
static void (*g_app_bt_pbap_vcard_entry_item_handler)(uint8_t device_t, struct btif_pbap_channel_t *pbap_ctl, const struct pbap_vcard_entry_item_t *item);

static void app_bt_pbap_callback(uint8_t device_id, struct btif_pbap_channel_t *pbap_ctl, const struct pbap_callback_parm_t *parm)
{
    if (device_id == BT_DEVICE_INVALID_ID && parm->event == PBAP_CHANNEL_CLOSED)
    {
        // pbap profile is closed due to acl created fail
        TRACE(2,"%s CHANNEL_CLOSED acl created error %02x", __func__, parm->error_code);
        return;
    }

    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    switch (parm->event)
    {
        case PBAP_OBEX_CONNECTED:
            TRACE(2,"(d%x) %s OBEX_CONNECTED", device_id, __func__);
            if (g_app_bt_pbap_obex_connected_handler)
            {
                g_app_bt_pbap_obex_connected_handler(device_id, pbap_ctl);
            }
            break;
        case PBAP_CHANNEL_OPENED:
            TRACE(2,"(d%x) %s CHANNEL_OPENED", device_id, __func__);
            break;
        case PBAP_CHANNEL_CLOSED:
            TRACE(3,"(d%x) %s CHANNEL_CLOSED %02x", device_id, __func__, parm->error_code);
            break;
        case PBAP_PHONEBOOK_SIZE_RSP:
            TRACE(4, "(d%x) %s PHONEBOOK_SIZE_RSP %d error %02x", device_id, __func__, parm->phonebook_size, parm->error_code);
            break;
        case PBAP_NEW_MISSED_CALLS_RSP:
            TRACE(4, "(d%x) %s NEW_MISSED_CALLS_RSP %d error %02x", device_id, __func__, parm->new_missed_calls, parm->error_code);
            break;
        case PBAP_VCARD_LISTING_ITEM_RSP:
            TRACE(3, "(d%x) %s VCARD_LISTING_ITEM_RSP %02x", device_id, __func__, parm->error_code);
            TRACE(4, "(d%x) %s vCard handle %d %s", device_id, __func__, parm->listing_item->entry_nane_len, parm->listing_item->vcard_entry_name);
            TRACE(4, "(d%x) %s vCard name %d %s", device_id, __func__, parm->listing_item->contact_name_len, parm->listing_item->vcard_contact_name);
            if (g_app_bt_pbap_vcard_listing_item_handler)
            {
                g_app_bt_pbap_vcard_listing_item_handler(device_id, pbap_ctl, parm->listing_item);
            }
            else
            {
                btif_pbap_pull_vcard_entry(curr_device->pbap_channel, parm->listing_item->vcard_entry_name, NULL);
            }
            break;
        case PBAP_VCARD_ENTRY_ITEM_RSP:
            TRACE(3,"(d%x) %s VCARD_ENTRY_ITEM_RSP %02x", device_id, __func__, parm->error_code);
            if (parm->entry_item->formatted_name_len)
            {
                TRACE(4, "(d%x) %s FN: %d %s", device_id, __func__, parm->entry_item->formatted_name_len, parm->entry_item->formatted_name);
            }
            if (parm->entry_item->contact_name_len)
            {
                TRACE(4, "(d%x) %s N: %d %s", device_id, __func__, parm->entry_item->contact_name_len, parm->entry_item->contact_name);
            }
            if (parm->entry_item->contact_address_len)
            {
                TRACE(4, "(d%x) %s ADR: %d %s", device_id, __func__, parm->entry_item->contact_address_len, parm->entry_item->contact_addr);
            }
            if (parm->entry_item->contact_tel_len)
            {
                TRACE(4, "(d%x) %s TEL: %d %s", device_id, __func__, parm->entry_item->contact_tel_len, parm->entry_item->contact_tel);
            }
            if (parm->entry_item->contact_email_len)
            {
                TRACE(4, "(d%x) %s EMAIL: %d %s", device_id, __func__, parm->entry_item->contact_email_len, parm->entry_item->contact_email);
            }
            if (parm->entry_item->contact_x_bt_uid)
            {
                TRACE(4, "(d%x) %s X-BT-UID: %d %s", device_id, __func__, parm->entry_item->contact_x_bt_uid_len, parm->entry_item->contact_x_bt_uid);
            }
            if (g_app_bt_pbap_vcard_entry_item_handler)
            {
                g_app_bt_pbap_vcard_entry_item_handler(device_id, pbap_ctl, parm->entry_item);
            }
            break;
        case PBAP_SET_PHONEBOOK_DONE:
            TRACE(3, "(d%x) %s SET_PHONEBOOK_DONE %02x", device_id, __func__, parm->error_code);
            break;
        case PBAP_ABORT_CURR_OP_DONE:
            TRACE(3,"(d%x) %s ABORT_CURR_OP_DONE %02x", device_id, __func__, parm->error_code);
            break;
        case PBAP_PULL_PHONEBOOK_DONE:
            TRACE(2,"(d%x) %s PBAP_PULL_PHONEBOOK_DONE", device_id, __func__);
            break;
        case PBAP_PULL_VCARD_LISTING_DONE:
            TRACE(2,"(d%x) %s PBAP_PULL_VCARD_LISTING_DONE", device_id, __func__);
            break;
        case PBAP_PULL_VCARD_ENTRY_DONE:
            TRACE(2,"(d%x) %s PBAP_PULL_VCARD_ENTRY_DONE", device_id, __func__);
            break;
        default:
            TRACE(3,"(d%x) %s invalid event %d", device_id, __func__, parm->event);
            break;
    }
}

bool app_bt_pbap_is_connected(struct btif_pbap_channel_t *pbap_chan)
{
    return btif_pbap_is_connected(pbap_chan);
}

void app_bt_connect_pbap_profile(bt_bdaddr_t *bdaddr)
{
    static bt_bdaddr_t remote;

    remote = *bdaddr;

    TRACE(7, "%s address %02x:%02x:%02x:%02x:%02x:%02x", __func__,
        remote.address[0], remote.address[1], remote.address[2],
        remote.address[3], remote.address[4], remote.address[5]);

    app_bt_start_custom_function_in_bt_thread((uint32_t)&remote, (uint32_t)NULL, (uint32_t)btif_pbap_connect);
}

void app_bt_disconnect_pbap_profile(struct btif_pbap_channel_t *pbap_chan)
{
    TRACE(1, "%s channel %p", __func__, pbap_chan);

    app_bt_start_custom_function_in_bt_thread((uint32_t)pbap_chan, (uint32_t)NULL, (uint32_t)btif_pbap_disconnect);
}

void app_bt_pbap_send_obex_disconnect_req(struct btif_pbap_channel_t *pbap_chan)
{
    TRACE(1, "%s channel %p", __func__, pbap_chan);

    app_bt_start_custom_function_in_bt_thread((uint32_t)pbap_chan, (uint32_t)NULL, (uint32_t)btif_pbap_send_obex_disconnect_req);
}

void app_bt_pbap_send_abort_req(struct btif_pbap_channel_t *pbap_chan)
{
    TRACE(1, "%s channel %p", __func__, pbap_chan);

    app_bt_start_custom_function_in_bt_thread((uint32_t)pbap_chan, (uint32_t)NULL, (uint32_t)btif_pbap_send_abort_req);
}

static struct btif_pbap_channel_t *app_bt_pbap_get_channel(void)
{
    struct BT_DEVICE_T *curr_device = NULL;
    g_app_bt_pbap_obex_connected_handler = NULL;
    g_app_bt_pbap_vcard_listing_item_handler = NULL;
    g_app_bt_pbap_vcard_entry_item_handler = NULL;
    curr_device = app_bt_get_device(BT_DEVICE_ID_1);
    return curr_device->pbap_channel;
}

void app_bt_pbap_get_phonebook_size(struct btif_pbap_channel_t *pbap_chan, const char* phonebook_object_path_name)
{
    struct pbap_pull_phonebook_parameters param = {0};
    param.dont_use_default_max_list_count = true;
    param.max_list_count = 0;
    TRACE(2, "%s %s", __func__, phonebook_object_path_name);
    btif_pbap_pull_phonebook(pbap_chan, phonebook_object_path_name, &param);
}

void app_bt_pbap_pull_single_phonebook(struct btif_pbap_channel_t *pbap_chan, const char* phonebook_object_path_name, uint16_t pb_index)
{
    struct pbap_pull_phonebook_parameters param = {0};
    param.dont_use_default_max_list_count = true;
    param.max_list_count = 1;
    param.list_start_offset = pb_index;
    TRACE(3, "%s %s pb_index %d", __func__, phonebook_object_path_name, pb_index);
    btif_pbap_pull_phonebook(pbap_chan, phonebook_object_path_name, &param);
}

void app_bt_pbap_client_test(void)
{
    if (bt_defer_curr_func_0(app_bt_pbap_client_test))
    {
        return;
    }

    struct btif_pbap_channel_t *pbap_channel = app_bt_pbap_get_channel();

    if (!btif_pbap_is_connected(pbap_channel))
    {
        TRACE(1, "%s pbap is not connected", __func__);
        return;
    }

    TRACE(1, "%s", __func__);

    btif_pbap_set_phonebook_to_root(pbap_channel);

    btif_pbap_pull_phonebook(pbap_channel, PBAP_MAIN_PHONEBOOK_OBJECT_PATH_NAME, NULL);

    btif_pbap_set_phonebook_to_child(pbap_channel, PBAP_TELECOM_FOLDER_NAME);

    btif_pbap_pull_vcard_listing(pbap_channel, PBAP_MAIN_PHONEBOOK_FOLDER_NAME, NULL);

    btif_pbap_set_phonebook_to_child(pbap_channel, PBAP_MAIN_PHONEBOOK_FOLDER_NAME);
}

#ifdef BT_PBAP_TEST_SUPPORT

static void app_bt_pts_pbap_create_channel(const char* param, uint32_t len)
{
    app_bt_connect_pbap_profile(app_bt_get_pts_address());
}

static void app_bt_pbap_obex_connected_handler_auth_conn_req(uint8_t device_t, struct btif_pbap_channel_t *pbap_ctl)
{
    btif_pbap_set_auth_connect_req(pbap_ctl, true);
    g_app_bt_pbap_obex_connected_handler = NULL;
}

static void app_bt_pts_pbap_send_auth_conn_req(const char* param, uint32_t len)
{
    g_app_bt_pbap_obex_connected_handler = app_bt_pbap_obex_connected_handler_auth_conn_req;
    app_bt_connect_pbap_profile(app_bt_get_pts_address());
}

static void app_bt_pts_pbap_disconnect_channel(const char* param, uint32_t len)
{
    struct btif_pbap_channel_t *pbap_channel = app_bt_pbap_get_channel();
    app_bt_disconnect_pbap_profile(pbap_channel);
}

static void app_bt_pts_pbap_send_obex_disc_req(const char* param, uint32_t len)
{
    struct btif_pbap_channel_t *pbap_channel = app_bt_pbap_get_channel();
    app_bt_pbap_send_obex_disconnect_req(pbap_channel);
}

static void app_bt_pts_pbap_send_obex_get_req(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_pbap_send_obex_get_req))
    {
        return;
    }

    struct btif_pbap_channel_t *pbap_channel = app_bt_pbap_get_channel();
    struct pbap_pull_phonebook_parameters param = {0};
    param.dont_use_default_max_list_count = true;
    param.max_list_count = 1;
    btif_pbap_pull_phonebook(pbap_channel, PBAP_MAIN_PHONEBOOK_OBJECT_PATH_NAME, &param);
}

static void app_bt_pts_pbap_send_abort(const char* param, uint32_t len)
{
    struct btif_pbap_channel_t *pbap_channel = app_bt_pbap_get_channel();
    app_bt_pbap_send_abort_req(pbap_channel);
}

static void app_bt_pts_pbap_client_test(const char* param, uint32_t len)
{
    app_bt_pbap_client_test();
}

static void app_bt_pts_pbap_pull_phonebook_pb(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_pbap_pull_phonebook_pb))
    {
        return;
    }

    struct btif_pbap_channel_t *pbap_channel = app_bt_pbap_get_channel();
    btif_pbap_pull_phonebook(pbap_channel, PBAP_MAIN_PHONEBOOK_OBJECT_PATH_NAME, NULL);
}

static void app_bt_pts_pbap_get_phonebook_size(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_pbap_get_phonebook_size))
    {
        return;
    }

    struct btif_pbap_channel_t *pbap_channel = app_bt_pbap_get_channel();
    struct pbap_pull_phonebook_parameters param = {0};
    param.dont_use_default_max_list_count = true;
    param.max_list_count = 0;
    btif_pbap_pull_phonebook(pbap_channel, PBAP_MAIN_PHONEBOOK_OBJECT_PATH_NAME, &param);
}

static void app_bt_pts_pbap_set_path_to_root(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_pbap_set_path_to_root))
    {
        return;
    }

    struct btif_pbap_channel_t *pbap_channel = app_bt_pbap_get_channel();
    btif_pbap_set_phonebook_to_root(pbap_channel);
}

static void app_bt_pts_pbap_set_path_to_parent(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_pbap_set_path_to_parent))
    {
        return;
    }

    struct btif_pbap_channel_t *pbap_channel = app_bt_pbap_get_channel();
    btif_pbap_set_phonebook_to_parent(pbap_channel);
}

static void app_bt_pts_pbap_enter_path_to_telecom(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_pbap_enter_path_to_telecom))
    {
        return;
    }

    struct btif_pbap_channel_t *pbap_channel = app_bt_pbap_get_channel();
    btif_pbap_set_phonebook_to_child(pbap_channel, PBAP_TELECOM_FOLDER_NAME);
}

static void app_bt_pts_pbap_list_phonebook_pb(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_pbap_list_phonebook_pb))
    {
        return;
    }

    struct btif_pbap_channel_t *pbap_channel = app_bt_pbap_get_channel();
    btif_pbap_set_phonebook_to_root(pbap_channel);
    btif_pbap_set_phonebook_to_child(pbap_channel, PBAP_TELECOM_FOLDER_NAME);
    btif_pbap_pull_vcard_listing(pbap_channel, PBAP_MAIN_PHONEBOOK_FOLDER_NAME, NULL);
    btif_pbap_set_phonebook_to_child(pbap_channel, PBAP_MAIN_PHONEBOOK_FOLDER_NAME);
}

static void app_bt_pts_pbap_list_phonebook_pb_size(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_pbap_list_phonebook_pb_size))
    {
        return;
    }

    struct btif_pbap_channel_t *pbap_channel = app_bt_pbap_get_channel();
    struct pbap_pull_vcard_listing_parameters param = {0};
    param.dont_use_default_max_list_count = true;
    param.max_list_count = 0;
    btif_pbap_set_phonebook_to_root(pbap_channel);
    btif_pbap_set_phonebook_to_child(pbap_channel, PBAP_TELECOM_FOLDER_NAME);
    btif_pbap_pull_vcard_listing(pbap_channel, PBAP_MAIN_PHONEBOOK_FOLDER_NAME, &param);
    btif_pbap_set_phonebook_to_child(pbap_channel, PBAP_MAIN_PHONEBOOK_FOLDER_NAME);
}

static void app_bt_pts_pbap_list_phonebook_pb_callback_get_n_tel(uint8_t device_t, struct btif_pbap_channel_t *pbap_ctl, const struct pbap_vcard_listing_item_t *item)
{
    struct pbap_pull_vcard_entry_parameters param = {0};
    param.has_property_selector_paramter = true;
    param.property_selector_parameter_byte0 = PBAP_PROPERTY_BYTE0_N | PBAP_PROPERTY_BYTE0_TEL;
    btif_pbap_pull_vcard_entry(pbap_ctl, item->vcard_entry_name, &param);
    g_app_bt_pbap_vcard_listing_item_handler = NULL;
}

static void app_bt_pts_pbap_pull_pb_entry_n_tel(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_pbap_pull_pb_entry_n_tel))
    {
        return;
    }

    struct btif_pbap_channel_t *pbap_channel = app_bt_pbap_get_channel();
    struct pbap_pull_vcard_listing_parameters param = {0};
    param.dont_use_default_max_list_count = true;
    param.max_list_count = 1;
    g_app_bt_pbap_vcard_listing_item_handler = app_bt_pts_pbap_list_phonebook_pb_callback_get_n_tel;
    btif_pbap_set_phonebook_to_root(pbap_channel);
    btif_pbap_set_phonebook_to_child(pbap_channel, PBAP_TELECOM_FOLDER_NAME);
    btif_pbap_pull_vcard_listing(pbap_channel, PBAP_MAIN_PHONEBOOK_FOLDER_NAME, &param);
    btif_pbap_set_phonebook_to_child(pbap_channel, PBAP_MAIN_PHONEBOOK_FOLDER_NAME);
}

static void app_bt_pts_pbap_vcard_entry_item_handler_uid(uint8_t device_t, struct btif_pbap_channel_t *pbap_ctl, const struct pbap_vcard_entry_item_t *item)
{
    g_app_bt_pbap_vcard_entry_item_handler = NULL;
    btif_pbap_pull_vcard_entry(pbap_ctl, item->context_x_bt_uid_start, NULL);
}

static void app_bt_pts_pbap_list_phonebook_pb_callback_uid(uint8_t device_t, struct btif_pbap_channel_t *pbap_ctl, const struct pbap_vcard_listing_item_t *item)
{
    struct pbap_pull_vcard_entry_parameters param = {0};
    g_app_bt_pbap_vcard_listing_item_handler = NULL;
    g_app_bt_pbap_vcard_entry_item_handler = app_bt_pts_pbap_vcard_entry_item_handler_uid;
    param.has_property_selector_paramter = true;
    param.property_selector_parameter_byte3 = PBAP_PROPERTY_BYTE3_X_BT_UID;
    btif_pbap_pull_vcard_entry(pbap_ctl, item->vcard_entry_name, &param);
}

static void app_bt_pts_pbap_pull_uid_entry(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_pbap_pull_uid_entry))
    {
        return;
    }

    struct btif_pbap_channel_t *pbap_channel = app_bt_pbap_get_channel();
    struct pbap_pull_vcard_listing_parameters param = {0};
    param.dont_use_default_max_list_count = true;
    param.max_list_count = 1;
    param.has_vcard_selector_parameter = true;
    param.vcard_selector_parameter_byte3 = PBAP_PROPERTY_BYTE3_X_BT_UID;
    g_app_bt_pbap_vcard_listing_item_handler = app_bt_pts_pbap_list_phonebook_pb_callback_uid;
    btif_pbap_set_phonebook_to_root(pbap_channel);
    btif_pbap_set_phonebook_to_child(pbap_channel, PBAP_TELECOM_FOLDER_NAME);
    btif_pbap_pull_vcard_listing(pbap_channel, PBAP_MAIN_PHONEBOOK_FOLDER_NAME, &param);
    btif_pbap_set_phonebook_to_child(pbap_channel, PBAP_MAIN_PHONEBOOK_FOLDER_NAME);
}

static void app_bt_pts_pbap_set_path_to(const char* name, uint32_t len)
{
    struct btif_pbap_channel_t *pbap_channel = app_bt_pbap_get_channel();

    if (name == NULL || len == 0 || len > 7)
    {
        TRACE(3, "%s invalid param %p %d", __func__, name, len);
        return;
    }

    TRACE(2, "%s %s", __func__, name);
    btif_pbap_set_phonebook_to_child(pbap_channel, name);
}

static void app_bt_pts_pbap_list_phonebook(const char* name, uint32_t len)
{
    struct btif_pbap_channel_t *pbap_channel = app_bt_pbap_get_channel();
    struct pbap_pull_vcard_listing_parameters param = {0};

    if (name == NULL || len == 0 || len > 5)
    {
        TRACE(3, "%s invalid param %p %d", __func__, name, len);
        return;
    }

    param.dont_use_default_max_list_count = true;
    param.max_list_count = 1;

    TRACE(2, "%s %s", __func__, name);
    btif_pbap_set_phonebook_to_root(pbap_channel);
    btif_pbap_set_phonebook_to_child(pbap_channel, PBAP_TELECOM_FOLDER_NAME);
    btif_pbap_pull_vcard_listing(pbap_channel, name, &param);
    btif_pbap_set_phonebook_to_child(pbap_channel, name);
}

static void app_bt_pts_pbap_list_reset_missed_call(const char* name, uint32_t len)
{
    struct btif_pbap_channel_t *pbap_channel = app_bt_pbap_get_channel();
    struct pbap_pull_vcard_listing_parameters param = {0};

    param.dont_use_default_max_list_count = true;
    param.max_list_count = 1;
    param.reset_new_missed_calls = true;

    if (name == NULL || len == 0 || len > 5)
    {
        TRACE(3, "%s invalid param %p %d", __func__, name, len);
        return;
    }

    TRACE(2, "%s %s", __func__, name);
    btif_pbap_set_phonebook_to_root(pbap_channel);
    btif_pbap_set_phonebook_to_child(pbap_channel, PBAP_TELECOM_FOLDER_NAME);
    btif_pbap_pull_vcard_listing(pbap_channel, name, &param);
    btif_pbap_set_phonebook_to_child(pbap_channel, name);
}

static void app_bt_pts_pbap_list_vcard_select(const char* name, uint32_t len)
{
    struct btif_pbap_channel_t *pbap_channel = app_bt_pbap_get_channel();
    struct pbap_pull_vcard_listing_parameters param = {0};

    param.dont_use_default_max_list_count = true;
    param.max_list_count = 1;
    param.has_vcard_selector_parameter = true;
    param.vcard_selector_parameter_byte0 = PBAP_PROPERTY_BYTE0_N | PBAP_PROPERTY_BYTE0_TEL;
    param.vcard_selector_operator = PBAP_VCARD_SELECTOR_OPERATOR_AND;

    if (name == NULL || len == 0 || len > 5)
    {
        TRACE(3, "%s invalid param %p %d", __func__, name, len);
        return;
    }

    TRACE(2, "%s %s", __func__, name);
    btif_pbap_set_phonebook_to_root(pbap_channel);
    btif_pbap_set_phonebook_to_child(pbap_channel, PBAP_TELECOM_FOLDER_NAME);
    btif_pbap_pull_vcard_listing(pbap_channel, name, &param);
    btif_pbap_set_phonebook_to_child(pbap_channel, name);
}

static void app_bt_pts_pbap_list_vcard_search(const char* name, uint32_t len)
{
    struct btif_pbap_channel_t *pbap_channel = app_bt_pbap_get_channel();
    struct pbap_pull_vcard_listing_parameters param = {0};

    param.dont_use_default_max_list_count = true;
    param.max_list_count = 1;
    param.use_search_property_to_filter_vcard = true;
    param.search_property = PBAP_SEARCH_PROPERTY_NAME;
    param.search_text = "PTS";

    if (name == NULL || len == 0 || len > 5)
    {
        TRACE(3, "%s invalid param %p %d", __func__, name, len);
        return;
    }

    TRACE(2, "%s %s", __func__, name);
    btif_pbap_set_phonebook_to_root(pbap_channel);
    btif_pbap_set_phonebook_to_child(pbap_channel, PBAP_TELECOM_FOLDER_NAME);
    btif_pbap_pull_vcard_listing(pbap_channel, name, &param);
    btif_pbap_set_phonebook_to_child(pbap_channel, name);
}

static void app_bt_pts_pbap_pull_phonebook(const char* name, uint32_t len)
{
    struct btif_pbap_channel_t *pbap_channel = app_bt_pbap_get_channel();
    struct pbap_pull_phonebook_parameters param = {0};
    char phonebook_object_path[64] = {0};
    param.dont_use_default_max_list_count = true;
    param.max_list_count = 1;
    if (name == NULL || len == 0 || len > 8)
    {
        TRACE(3, "%s invalid param %p %d", __func__, name, len);
        return;
    }
    TRACE(2, "%s %s", __func__, name);
    if (strncmp(name, "sim_", 4) == 0)
    {
        snprintf(phonebook_object_path, 60, "SIM1/telecom/%s.vcf", name+4);
    }
    else
    {
        snprintf(phonebook_object_path, 60, "telecom/%s.vcf", name);
    }
    btif_pbap_pull_phonebook(pbap_channel, phonebook_object_path, &param);
}

static void app_bt_pts_pbap_pull_reset_missed_call(const char* name, uint32_t len)
{
    struct btif_pbap_channel_t *pbap_channel = app_bt_pbap_get_channel();
    struct pbap_pull_phonebook_parameters param = {0};
    char phonebook_object_path[64] = {0};
    param.dont_use_default_max_list_count = true;
    param.max_list_count = 1;
    param.reset_new_missed_calls = true;
    if (name == NULL || len == 0 || len > 8)
    {
        TRACE(3, "%s invalid param %p %d", __func__, name, len);
        return;
    }
    TRACE(2, "%s %s", __func__, name);
    if (strncmp(name, "sim_", 4) == 0)
    {
        snprintf(phonebook_object_path, 60, "SIM1/telecom/%s.vcf", name+4);
    }
    else
    {
        snprintf(phonebook_object_path, 60, "telecom/%s.vcf", name);
    }
    btif_pbap_pull_phonebook(pbap_channel, phonebook_object_path, &param);
}

static void app_bt_pts_pbap_pull_vcard_select(const char* name, uint32_t len)
{
    struct btif_pbap_channel_t *pbap_channel = app_bt_pbap_get_channel();
    struct pbap_pull_phonebook_parameters param = {0};
    char phonebook_object_path[64] = {0};
    param.dont_use_default_max_list_count = true;
    param.max_list_count = 1;
    param.has_vcard_selector_parameter = true;
    param.vcard_selector_parameter_byte0 = PBAP_PROPERTY_BYTE0_N | PBAP_PROPERTY_BYTE0_TEL;
    param.vcard_selector_operator = PBAP_VCARD_SELECTOR_OPERATOR_AND;
    if (name == NULL || len == 0 || len > 8)
    {
        TRACE(3, "%s invalid param %p %d", __func__, name, len);
        return;
    }
    TRACE(2, "%s %s", __func__, name);
    if (strncmp(name, "sim_", 4) == 0)
    {
        snprintf(phonebook_object_path, 60, "SIM1/telecom/%s.vcf", name+4);
    }
    else
    {
        snprintf(phonebook_object_path, 60, "telecom/%s.vcf", name);
    }
    btif_pbap_pull_phonebook(pbap_channel, phonebook_object_path, &param);
}

static app_bt_host_cmd_table_t app_pbap_test_cmd_table[] =
{
    {"pb_connect",              app_bt_pts_pbap_create_channel},
    {"pb_disconnect",           app_bt_pts_pbap_disconnect_channel},
    {"pb_obex_disc",            app_bt_pts_pbap_send_obex_disc_req},
    {"pb_get_req",              app_bt_pts_pbap_send_obex_get_req},
    {"pb_auth_req",             app_bt_pts_pbap_send_auth_conn_req},
    {"pb_abort",                app_bt_pts_pbap_send_abort},
    {"pb_client_test",          app_bt_pts_pbap_client_test},
    {"pb_pull_pb",              app_bt_pts_pbap_pull_phonebook_pb},
    {"pb_pull_size",            app_bt_pts_pbap_get_phonebook_size},
    {"pb_to_root",              app_bt_pts_pbap_set_path_to_root},
    {"pb_to_parent",            app_bt_pts_pbap_set_path_to_parent},
    {"pb_to_telecom",           app_bt_pts_pbap_enter_path_to_telecom},
    {"pb_list_pb",              app_bt_pts_pbap_list_phonebook_pb},
    {"pb_list_size",            app_bt_pts_pbap_list_phonebook_pb_size},
    {"pb_entry_n_tel",          app_bt_pts_pbap_pull_pb_entry_n_tel},
    {"pb_entry_uid",            app_bt_pts_pbap_pull_uid_entry},
    {"pb_to",                   app_bt_pts_pbap_set_path_to},
    {"pb_list",                 app_bt_pts_pbap_list_phonebook},
    {"pb_pull",                 app_bt_pts_pbap_pull_phonebook},
    {"pb_reset",                app_bt_pts_pbap_pull_reset_missed_call},
    {"pb_vcsel",                app_bt_pts_pbap_pull_vcard_select},
    {"pb_list_reset",           app_bt_pts_pbap_list_reset_missed_call},
    {"pb_list_vcsel",           app_bt_pts_pbap_list_vcard_select},
    {"pb_list_vcsch",           app_bt_pts_pbap_list_vcard_search},
};
#endif /* BT_PBAP_TEST_SUPPORT */


void app_bt_pbap_init(void)
{
    struct BT_DEVICE_T *curr_device = NULL;

    if (besbt_cfg.bt_sink_enable)
    {
        btif_pbap_init(app_bt_pbap_callback);

        for (int i = 0; i < BT_DEVICE_NUM; i += 1)
        {
            curr_device = app_bt_get_device(i);
            curr_device->pbap_channel = btif_alloc_pbap_channel();
        }
    }

#ifdef BT_PBAP_TEST_SUPPORT
    app_bt_host_add_cmd_table(sizeof(app_pbap_test_cmd_table)/sizeof(app_pbap_test_cmd_table[0]),
        app_pbap_test_cmd_table);
#endif
}

#endif /* BT_PBAP_SUPPORT */

