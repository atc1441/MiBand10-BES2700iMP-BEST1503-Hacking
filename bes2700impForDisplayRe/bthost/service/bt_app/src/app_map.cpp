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

#if defined(BT_MAP_SUPPORT)
#include "bluetooth.h"
#include "me_api.h"
#include "obex_i.h"
#include "map_i.h"
#include "map_api.h"
#include "app_bt_cmd.h"
#include "app_bt.h"
#include "btapp.h"

#ifdef IBRT
#include "app_tws_ibrt.h"
#endif

int bt_map_event_callback(const bt_bdaddr_t *bd_addr, BT_EVENT_T event, BT_CALLBACK_PARAM_T param)
{
    POSSIBLY_UNUSED struct BT_DEVICE_T *curr_device = NULL;
    POSSIBLY_UNUSED uint8_t device_id = BT_DEVICE_INVALID_ID;

    switch (event)
    {
        case BT_EVENT_MAP_OPENED:
#if defined(IBRT)
            curr_device = app_bt_get_connected_device_byaddr(bd_addr);
            device_id = curr_device ? curr_device->device_id : BT_DEVICE_INVALID_ID;
            app_tws_ibrt_profile_callback(device_id, BTIF_APP_MAP_PROFILE_ID,
                (void *)(uintptr_t)event, (void *)param.param_ptr, (void *)bd_addr);
#endif
            break;
        case BT_EVENT_MAP_CLOSED:
#if defined(IBRT)
            curr_device = app_bt_get_connected_device_byaddr(bd_addr);
            device_id = curr_device ? curr_device->device_id : BT_DEVICE_INVALID_ID;
            app_tws_ibrt_profile_callback(device_id, BTIF_APP_MAP_PROFILE_ID,
                (void *)(uintptr_t)event, (void *)param.param_ptr, (void *)bd_addr);
#endif
            break;
        case BT_EVENT_MAP_MSG_LISTING_ITEM_RSP:
            break;
        default:
            break;
    }

    return 0;
}

void bt_map_set_obex_over_rfcomm(void)
{
    btif_map_set_obex_over_rfcomm(true);
}

void bt_map_clear_obex_over_rfcomm(void)
{
    btif_map_set_obex_over_rfcomm(false);
}

bt_status_t bt_map_cleanup(void)
{
    return BT_STS_SUCCESS;
}

bt_status_t bt_map_connect(const bt_bdaddr_t *remote)
{
    btif_map_connect(remote);
    return BT_STS_SUCCESS;
}

bt_status_t bt_map_disconnect(const bt_bdaddr_t *remote)
{
    btif_map_disconnect(remote);
    return BT_STS_SUCCESS;
}

static uint16_t bt_map_get_remote_connhdl(const bt_bdaddr_t *remote)
{
    uint16_t connhdl = BT_INVALID_CONN_HANDLE;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (curr_device)
    {
        connhdl = curr_device->acl_conn_hdl;
    }
    return connhdl;
}

bt_map_state_t bt_map_get_state(const bt_bdaddr_t *remote)
{
    uint16_t connhdl = bt_map_get_remote_connhdl(remote);
    return btif_map_get_state(connhdl);
}

int bt_map_is_connected(const bt_bdaddr_t *remote)
{
    uint16_t connhdl = bt_map_get_remote_connhdl(remote);
    return btif_map_get_state(connhdl) == BT_MAP_STATE_OPEN;
}

bt_status_t bt_map_notify_register(const bt_bdaddr_t *remote, bool enable)
{
    uint16_t connhdl = bt_map_get_remote_connhdl(remote);
    btif_map_notify_register(connhdl, MAP_DEFAULT_MAS_INSTANCE, enable);
    return BT_STS_SUCCESS;
}

bt_status_t bt_map_enter_to_root_folder(const bt_bdaddr_t *remote)
{
    uint16_t connhdl = bt_map_get_remote_connhdl(remote);
    btif_map_enter_to_root_folder(connhdl, MAP_DEFAULT_MAS_INSTANCE);
    return BT_STS_SUCCESS;
}

bt_status_t bt_map_enter_to_parent_folder(const bt_bdaddr_t *remote)
{
    uint16_t connhdl = bt_map_get_remote_connhdl(remote);
    btif_map_enter_to_parent_folder(connhdl, MAP_DEFAULT_MAS_INSTANCE);
    return BT_STS_SUCCESS;
}

bt_status_t bt_map_enter_to_msg_folder(const bt_bdaddr_t *remote)
{
    uint16_t connhdl = bt_map_get_remote_connhdl(remote);
    btif_map_enter_to_msg_folder(connhdl, MAP_DEFAULT_MAS_INSTANCE);
    return BT_STS_SUCCESS;
}

bt_status_t bt_map_enter_to_child_folder(const bt_bdaddr_t *remote, const char *folder)
{
    uint16_t connhdl = bt_map_get_remote_connhdl(remote);
    btif_map_enter_to_child_folder(connhdl, MAP_DEFAULT_MAS_INSTANCE, folder);
    return BT_STS_SUCCESS;
}

bt_status_t bt_map_get_folder_listing(const bt_bdaddr_t *bd_addr)
{
    uint16_t connhdl = bt_map_get_remote_connhdl(bd_addr);
    btif_map_get_folder_listing(connhdl, MAP_DEFAULT_MAS_INSTANCE, NULL);
    return BT_STS_SUCCESS;
}

bt_status_t bt_map_get_msg_listing(const bt_bdaddr_t *bd_addr)
{
    uint16_t connhdl = bt_map_get_remote_connhdl(bd_addr);
    btif_map_get_message_listing(connhdl, MAP_DEFAULT_MAS_INSTANCE, NULL);
    return BT_STS_SUCCESS;
}

bt_status_t bt_map_get_unread_message(const bt_bdaddr_t *remote)
{
    struct map_get_msg_listing_param_t gl_param = {{0},0};
    uint16_t connhdl = bt_map_get_remote_connhdl(remote);
    gl_param.max_list_count = 10;
    gl_param.filter_read_status = MAP_FILTER_READ_STATUS_GET_UNREAD_MSGS;
    btif_map_get_message_listing(connhdl, MAP_DEFAULT_MAS_INSTANCE, &gl_param);
    return BT_STS_SUCCESS;
}

bt_status_t bt_map_get_message(const bt_bdaddr_t *bd_addr, uint64_t handle)
{
    uint16_t connhdl = bt_map_get_remote_connhdl(bd_addr);
    btif_map_get_message(connhdl, MAP_DEFAULT_MAS_INSTANCE, handle);
    return BT_STS_SUCCESS;
}

bt_status_t bt_map_set_message_status(const bt_bdaddr_t *bd_addr, uint64_t handle, bt_map_msg_status_t status)
{
    uint16_t connhdl = bt_map_get_remote_connhdl(bd_addr);
    if (status == BT_MAP_MSG_STATUS_UNREAD || status == BT_MAP_MSG_STATUS_READ)
    {
        btif_map_set_msg_read_status(connhdl, MAP_DEFAULT_MAS_INSTANCE, handle, status == BT_MAP_MSG_STATUS_READ);
    }
    else
    {
        btif_map_set_msg_delete_status(connhdl, MAP_DEFAULT_MAS_INSTANCE, handle, status == BT_MAP_MSG_STATUS_DELETED);
    }
    return BT_STS_SUCCESS;
}

void bt_map_client_test(const bt_bdaddr_t *remote)
{
    bt_map_notify_register(remote, true);
    bt_map_enter_to_msg_folder(remote);
    bt_map_enter_to_child_folder(remote, "inbox");
    bt_map_get_unread_message(remote);
    bt_map_enter_to_parent_folder(remote);
}

bt_status_t bt_map_send_sms(const bt_bdaddr_t *remote, struct bt_map_sms_t *sms)
{
    uint8_t mas_instance_id = MAP_DEFAULT_MAS_INSTANCE;
    bt_map_message_t msg = {0};
    struct map_vcard_info_t recipent;
    uint16_t connhdl = bt_map_get_remote_connhdl(remote);

    if (sms->msg == NULL || sms->msg_len == 0)
    {
        TRACE(0, "bt_map_send_sms: empty message");
        return BT_STS_FAILED;
    }

    if (sms->tel && sms->tel_len)
    {
        int tel_len = sms->tel_len;
        if (tel_len > MAP_VCARD_ELEM_MAX_LEN)
        {
            TRACE(0, "bt_map_send_sms: tel_len too long %d", tel_len);
            tel_len = MAP_VCARD_ELEM_MAX_LEN;
        }
        memcpy(recipent.name, "default", sizeof("default"));
        memcpy(recipent.tel, sms->tel, tel_len);
        recipent.tel[tel_len] = 0;
        recipent.email[0] = 0;

        msg.recipient.recipient_num = 1;
        msg.recipient.vcard = &recipent;
    }
    else
    {
        msg.recipient.recipient_num = 0;
    }

    msg.content_data = sms->msg;
    msg.content_len = sms->msg_len;
    msg.msg_type = MAP_MSG_TYPE_SMS_GSM;

    btif_map_enter_to_msg_folder(connhdl, mas_instance_id);
    btif_map_send_message(connhdl, mas_instance_id, &msg, NULL);
    return BT_STS_SUCCESS;
}


#ifdef BT_MAP_TEST_SUPPORT
static void app_bt_pts_map_connect(const char* param, uint32_t len)
{
    btif_map_connect(app_bt_get_pts_address());
}

static void app_bt_pts_map_disconnect(const char* param, uint32_t len)
{
    btif_map_disconnect(app_bt_get_pts_address());
}

static void app_bt_pts_set_obex_over_rfcomm(const char* param, uint32_t len)
{
    btif_map_set_obex_over_rfcomm(true);
}

static void app_bt_pts_clear_obex_over_rfcomm(const char* param, uint32_t param_len)
{
    btif_map_set_obex_over_rfcomm(false);
}

static void app_bt_pts_map_mns_obex_disc_req(const char* param, uint32_t len)
{
    struct BT_DEVICE_T *curr_device = NULL;
    curr_device = app_bt_get_connected_device_byaddr(app_bt_get_pts_address());
    if (curr_device == NULL)
    {
        TRACE(0, "app_bt_pts_map_mns_obex_disc_req: not connected");
        return;
    }
    btif_map_send_mns_obex_disc_req(curr_device->acl_conn_hdl);
}

static app_bt_pts_map_close_mns_channel(const char* param, uint32_t len)
{
    struct BT_DEVICE_T *curr_device = NULL;
    curr_device = app_bt_get_connected_device_byaddr(app_bt_get_pts_address());
    if (curr_device == NULL)
    {
        TRACE(0, "app_bt_pts_map_close_mns_channel: not connected");
        return;
    }
    btif_map_close_mns_channel(curr_device->acl_conn_hdl);
}

struct app_bt_map_param_t {
    uint16_t connhdl;
    uint8_t mas_instance_id;
    bool succ;
    char *extra_param;
};

static struct app_bt_map_param_t app_bt_pts_map_get_param(const char* name, uint32_t len)
{
    struct BT_DEVICE_T *curr_device = NULL;
    struct app_bt_map_param_t param = {0};
    char instance_id = 0;
    char seperate = 0;
    curr_device = app_bt_get_connected_device_byaddr(app_bt_get_pts_address());
    if (curr_device == NULL)
    {
        TRACE(0, "app_bt_pts_map_get_param: not connected");
        return param;
    }
    if (name == NULL || name[0] == 0)
    {
        TRACE(0, "app_bt_pts_map_get_param: invalid param");
        return param;
    }
    instance_id = name[0];
    seperate = name[1];
    if (instance_id < '0' || instance_id > '9')
    {
        TRACE(0, "app_bt_pts_map_get_param: invalid instance id %c", instance_id);
        return param;
    }
    param.succ = true;
    param.connhdl = curr_device->acl_conn_hdl;
    param.mas_instance_id = instance_id - '0';
    param.extra_param = (seperate == '|') ? (char *)(name + 2) : NULL;
    TRACE(0, "app_bt_pts_map_get_param: connhdl %04x len %d param %s extra %s",
        curr_device->acl_conn_hdl, len, name, param.extra_param ? param.extra_param : "n/a");
    return param;
}

static void app_bt_pts_map_obex_disc_req(const char* name, uint32_t len)
{
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (param.succ)
    {
        btif_map_send_obex_disconnect_req(param.connhdl, param.mas_instance_id);
    }
}

static void app_bt_pts_map_obex_conn_req(const char* name, uint32_t len)
{
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (param.succ)
    {
        btif_map_send_obex_connect_req(param.connhdl, param.mas_instance_id);
    }
}

static void app_bt_pts_map_dont_auto_conn_req(const char* name, uint32_t len)
{
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (param.succ)
    {
        btif_map_dont_auto_send_obex_conn_req(param.connhdl, param.mas_instance_id);
    }
}

static void app_bt_pts_map_connect_mas(const char* name, uint32_t len)
{
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (param.succ)
    {
        btif_map_connect_mas(param.connhdl, param.mas_instance_id);
    }
}

static void app_bt_pts_map_open_mas_channel(const char* name, uint32_t len)
{
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (param.succ)
    {
        btif_map_open_mas_channel(param.connhdl, param.mas_instance_id);
    }
}

static void app_bt_pts_map_close_mas_channel(const char* name, uint32_t len)
{
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (param.succ)
    {
        btif_map_close_mas_channel(param.connhdl, param.mas_instance_id);
    }
}

static void app_bt_pts_map_enter_to_root_folder(const char* name, uint32_t len)
{
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (param.succ)
    {
        btif_map_enter_to_root_folder(param.connhdl, param.mas_instance_id);
    }
}

static void app_bt_pts_map_enter_to_parent_folder(const char* name, uint32_t len)
{
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (param.succ)
    {
        btif_map_enter_to_parent_folder(param.connhdl, param.mas_instance_id);
    }
}

static void app_bt_pts_map_enter_to_msg_folder(const char* name, uint32_t len)
{
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (param.succ)
    {
        btif_map_enter_to_msg_folder(param.connhdl, param.mas_instance_id);
    }
}

static void app_bt_pts_map_enter_to_child_folder(const char* name, uint32_t len)
{
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (param.succ)
    {
        btif_map_enter_to_child_folder(param.connhdl, param.mas_instance_id, param.extra_param);
    }
}

static static void app_bt_pts_map_open_all_mas_channel(const char* param, uint32_t len)
{
    struct BT_DEVICE_T *curr_device = NULL;
    curr_device = app_bt_get_connected_device_byaddr(app_bt_get_pts_address());
    if (curr_device == NULL)
    {
        TRACE(0, "app_bt_pts_map_open_all_mas_channel: not connected");
        return;
    }
    btif_map_open_all_mas_channel(curr_device->acl_conn_hdl);
}

static static void app_bt_pts_map_client_test(const char* param, uint32_t len)
{
    bt_map_client_test(app_bt_get_pts_address());
}

static static void app_bt_pts_map_send_sms(const char* param, uint32_t len)
{
    struct bt_map_sms_t sms = {0};
    struct BT_DEVICE_T *curr_device = NULL;
    curr_device = app_bt_get_connected_device_byaddr(app_bt_get_pts_address());
    if (!curr_device)
    {
        TRACE(0, "app_bt_map_send_sms_test: conn not exist");
        return;
    }
    sms.tel = "13912345678";
    sms.tel_len = strlen(sms.tel);
    sms.msg = "GSM SMS TEXT\r\n";
    sms.msg_len = strlen(sms.msg);
    bt_map_send_sms(&curr_device->remote, &sms);
}

static void app_bt_pts_map_send_gsm_sms(const char* name, uint32_t len)
{
    struct bt_map_message_t sms = {0};
    struct app_bt_map_param_t param;
    struct map_vcard_info_t recipent;

    param = app_bt_pts_map_get_param(name, len);
    if (!param.succ)
    {
        return;
    }

    memcpy(recipent.name, "IUT", sizeof("IUT"));
    memcpy(recipent.tel, "0000000", sizeof("0000000"));
    recipent.email[0] = 0;

    memcpy(sms.originator.name, "PTS", sizeof("PTS"));
    memcpy(sms.originator.tel, "0000001", sizeof("0000001"));
    sms.originator.email[0] = 0;

    sms.recipient.recipient_num = 1;
    sms.recipient.vcard = &recipent;

    sms.content_data = 
        "0191000E9100949821436587000011303231\r\n"
        "12928211CC32FD34079DDF20737A8E4EBBCF21\r\n";

    sms.msg_type = MAP_MSG_TYPE_SMS_GSM;
    sms.is_native_sms_pdu = true;
    sms.content_len = strlen(sms.content_data);

    btif_map_send_message(param.connhdl, param.mas_instance_id, &sms, NULL);
}

static void app_bt_pts_map_send_cdma_sms(const char* name, uint32_t len)
{
    struct bt_map_message_t sms = {0};
    struct app_bt_map_param_t param;
    struct map_vcard_info_t recipent;

    param = app_bt_pts_map_get_param(name, len);
    if (!param.succ)
    {
        return;
    }

    memcpy(recipent.name, "IUT", sizeof("IUT"));
    memcpy(recipent.tel, "0000000", sizeof("0000000"));
    recipent.email[0] = 0;

    memcpy(sms.originator.name, "PTS", sizeof("PTS"));
    memcpy(sms.originator.tel, "0000001", sizeof("0000001"));
    sms.originator.email[0] = 0;

    sms.recipient.recipient_num = 1;
    sms.recipient.vcard = &recipent;

    sms.content_data = "CDMA SMS TEXT\r\n";

    sms.msg_type = MAP_MSG_TYPE_SMS_CDMA;

    btif_map_send_message(param.connhdl, param.mas_instance_id, &sms, NULL);
}

static void app_bt_pts_map_send_mms(const char* name, uint32_t len)
{
    struct bt_map_message_t mms = {0};
    struct app_bt_map_param_t param;
    struct map_vcard_info_t recipent;

    param = app_bt_pts_map_get_param(name, len);
    if (!param.succ)
    {
        return;
    }

    memcpy(recipent.name, "IUT", sizeof("IUT"));
    memcpy(recipent.email, "Iut_emailaddress", sizeof("Iut_emailaddress"));
    memcpy(recipent.tel, "0000000", sizeof("0000000"));

    memcpy(mms.originator.name, "PTS", sizeof("PTS"));
    memcpy(mms.originator.email, "Pts_emailaddress", sizeof("Pts_emailaddress"));
    memcpy(mms.originator.tel, "0000001", sizeof("0000001"));

    mms.recipient.recipient_num = 1;
    mms.recipient.vcard = &recipent;

    mms.content_data = "Let's go fishing!\r\n";

    mms.msg_type = MAP_MSG_TYPE_MMS;

    btif_map_send_message(param.connhdl, param.mas_instance_id, &mms, NULL);
}

static void app_bt_pts_map_push_to_conversation(const char* name, uint32_t len)
{
    struct bt_map_message_t im = {0};
    struct bt_map_send_param_t send_param = {0};
    struct app_bt_map_param_t param;
    struct map_vcard_info_t recipent;

    param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_push_msg_to_convo: invalid param");
        return;
    }

    memcpy(recipent.name, "IUT;ONE", sizeof("IUT;ONE"));
    memcpy(recipent.tel, "skype:beastmode1 ", sizeof("skype:beastmode1 "));
    recipent.email[0] = 0;

    memcpy(im.originator.name, "PTS;ONE", sizeof("PTS;ONE"));
    memcpy(im.originator.tel, "skype:beastmode2", sizeof("skype:beastmode2"));
    im.originator.email[0] = 0;

    im.recipient.recipient_num = 1;
    im.recipient.vcard = &recipent;

    im.content_data =
        "Date: Fri, 27 Aug 2022 20:29:12 +08:00\r\n"
        "From: skype:beastmode1\r\n"
        "To: skype:beastmode2\r\n"
        "Content-Type: text/plain;"
        "\r\n"
        "Let's go fishing!\r\n";

    im.msg_type = MAP_MSG_TYPE_IM;
    im.push_to_inbox_convo = true;

    send_param.conversation_id = param.extra_param;
    btif_map_send_message(param.connhdl, param.mas_instance_id, &im, &send_param);
}

static void app_bt_pts_map_send_im(const char* name, uint32_t len)
{
    struct bt_map_message_t im = {0};
    struct app_bt_map_param_t param;
    struct map_vcard_info_t recipent;

    param = app_bt_pts_map_get_param(name, len);
    if (!param.succ)
    {
        return;
    }

    memcpy(recipent.name, "Alois", sizeof("Alois"));
    memcpy(recipent.tel, "whateverapp:4912345@s.whateverapp.net", sizeof("whateverapp:4912345@s.whateverapp.net"));
    recipent.email[0] = 0;

    memcpy(im.originator.name, "Marry", sizeof("Marry"));
    memcpy(im.originator.tel, "whateverapp:497654321@s.whateverapp.net", sizeof("whateverapp:497654321@s.whateverapp.net"));
    im.originator.email[0] = 0;

    im.recipient.recipient_num = 1;
    im.recipient.vcard = &recipent;

    im.content_data =
        "Date: Fri, 1 Aug 2014 01:29:12 +01:00\r\n"
        "From: whateverapp:497654321@s.whateverapp.net\r\n"
        "To: whateverapp:4912345@s.whateverapp.net\r\n"
        "Content-Type: text/plain;\r\n"
        "\r\n"
        "Happy birthday\r\n";

    im.msg_type = MAP_MSG_TYPE_IM;

    btif_map_send_message(param.connhdl, param.mas_instance_id, &im, NULL);
}

static void app_bt_pts_map_send_email(const char* name, uint32_t len)
{
    struct bt_map_message_t email = {0};
    struct app_bt_map_param_t param;
    struct map_vcard_info_t recipent;

    param = app_bt_pts_map_get_param(name, len);
    if (!param.succ)
    {
        return;
    }

    memcpy(recipent.name, "name1", sizeof("name1"));
    memcpy(recipent.email, "name1@defemail.addr", sizeof("name1@defemail.addr"));
    recipent.tel[0] = 0;

    memcpy(email.originator.name, "originator_name", sizeof("originator_name"));
    memcpy(email.originator.email, "originator_name@defemail.addr", sizeof("originator_name@defemail.addr"));
    email.originator.tel[0] = 0;

    email.recipient.recipient_num = 1;
    email.recipient.vcard = &recipent;

    email.content_data =
        "Date: 20 Jun 96\r\n"
        "Subject: Fish\r\n"
        "From:originator_name@defemail.addr\r\n"
        "To:name1@defemail.addr\r\n"
        "Let's go fishing! and long long test texture content \r\n"
        "long long long long long long long test texture content \r\n"
        "long long long long long long long test texture content \r\n"
        "long long long long long long long test texture content \r\n"
        "long long long long long long long test texture content \r\n"
        "long long long long long long long test texture content \r\n"
        "long long long long long long long test texture content \r\n"
        "long long long long long long long test texture content \r\n"
        "long long long long long long long test texture content \r\n"
        "long long long long long long long test texture content \r\n"
        "long long long long long long long test texture content \r\n"
        "BR, Ori\r\n";

    email.msg_type = MAP_MSG_TYPE_EMAIL;

    btif_map_send_message(param.connhdl, param.mas_instance_id, &email, NULL);
}

static void app_bt_pts_map_replace_email(const char* name, uint32_t len)
{
    struct bt_map_message_t email = {0};
    struct bt_map_send_param_t send_param = {0};
    struct app_bt_map_param_t param;
    struct map_vcard_info_t recipent;

    param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_put_start: invalid param");
        return;
    }

    memcpy(recipent.name, "PTS2", sizeof("PTS2"));
    memcpy(recipent.email, "PTS2@bluetooth.com", sizeof("PTS2@bluetooth.com"));
    recipent.tel[0] = 0;

    memcpy(email.originator.name, "IUT", sizeof("IUT"));
    memcpy(email.originator.email, "momber@iut.com", sizeof("momber@iut.com"));
    email.originator.tel[0] = 0;

    email.recipient.recipient_num = 1;
    email.recipient.vcard = &recipent;

    email.content_data = "Let's go fishing!\r\n";
    email.msg_type = MAP_MSG_TYPE_EMAIL;

    send_param.origin_msg_handle = map_str_to_hex_value(param.extra_param, strlen(param.extra_param));
    send_param.forward_attachment = false;
    send_param.prepend_to_origin_msg = false;
    btif_map_send_message(param.connhdl, param.mas_instance_id, &email, &send_param);
}

static void app_bt_pts_map_forward_email(const char* name, uint32_t len)
{
    struct bt_map_message_t email = {0};
    struct bt_map_send_param_t send_param = {0};
    struct app_bt_map_param_t param;
    struct map_vcard_info_t recipent;

    param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_put_start: invalid param");
        return;
    }

    memcpy(recipent.name, "PTS2", sizeof("PTS2"));
    memcpy(recipent.email, "PTS2@bluetooth.com", sizeof("PTS2@bluetooth.com"));
    recipent.tel[0] = 0;

    memcpy(email.originator.name, "IUT", sizeof("IUT"));
    memcpy(email.originator.email, "momber@iut.com", sizeof("momber@iut.com"));
    email.originator.tel[0] = 0;

    email.recipient.recipient_num = 1;
    email.recipient.vcard = &recipent;

    email.content_data = "FYI\r\n";
    email.msg_type = MAP_MSG_TYPE_EMAIL;

    send_param.origin_msg_handle = map_str_to_hex_value(param.extra_param, strlen(param.extra_param));
    send_param.forward_attachment = false;
    send_param.prepend_to_origin_msg = true;
    btif_map_send_message(param.connhdl, param.mas_instance_id, &email, &send_param);
}

static void app_bt_pts_map_forward_including_attachment(const char* name, uint32_t len)
{
    struct bt_map_message_t email = {0};
    struct bt_map_send_param_t send_param = {0};
    struct app_bt_map_param_t param;
    struct map_vcard_info_t recipent;

    param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_put_start: invalid param");
        return;
    }

    memcpy(recipent.name, "PTS2", sizeof("PTS2"));
    memcpy(recipent.email, "PTS2@bluetooth.com", sizeof("PTS2@bluetooth.com"));
    recipent.tel[0] = 0;

    memcpy(email.originator.name, "IUT", sizeof("IUT"));
    memcpy(email.originator.email, "momber@iut.com", sizeof("momber@iut.com"));
    email.originator.tel[0] = 0;

    email.recipient.recipient_num = 1;
    email.recipient.vcard = &recipent;

    email.content_data = "FYI\r\n";
    email.msg_type = MAP_MSG_TYPE_EMAIL;

    send_param.origin_msg_handle = map_str_to_hex_value(param.extra_param, strlen(param.extra_param));
    send_param.forward_attachment = true;
    send_param.prepend_to_origin_msg = true;
    btif_map_send_message(param.connhdl, param.mas_instance_id, &email, &send_param);
}

static void app_bt_pts_map_put_start(const char* name, uint32_t len)
{
    struct bt_map_message_t email = {0};
    struct app_bt_map_param_t param;
    struct map_vcard_info_t recipent;
    char digit = 0;
    uint16_t count = 0;

    param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_put_start: invalid param");
        return;
    }

    memcpy(recipent.name, "IUT", sizeof("IUT"));
    memcpy(recipent.email, "Iut_emailaddress", sizeof("Iut_emailaddress"));
    recipent.tel[0] = 0;

    memcpy(email.originator.name, "PTS", sizeof("PTS"));
    memcpy(email.originator.email, "Pts_emailaddress", sizeof("Pts_emailaddress"));
    email.originator.tel[0] = 0;

    email.recipient.recipient_num = 1;
    email.recipient.vcard = &recipent;
    email.is_draft_msg = true;
    email.msg_type = MAP_MSG_TYPE_EMAIL;

    email.content_data =
        "Date: 20 Jun 96\r\n"
        "Subject: Fish\r\n"
        "From:Pts_emailaddress\r\n"
        "To:Iut_emailaddress\r\n"
        "Let's go fishing!\r\n"
        "BR, PtsTest\r\n";

    digit = param.extra_param[0];
    if (digit >= '0' && digit <= '9')
    {
        count = count * 10 + (digit - '0');
        digit = param.extra_param[1];
        if (digit >= '0' && digit <= '9')
        {
            count = count * 10 + (digit - '0');
        }
    }

    TRACE(0, "app_bt_pts_map_put_start: count %d", count);

    btif_map_put_email_start(param.connhdl, param.mas_instance_id, &email, count);
}

static void app_bt_pts_map_put_continue(const char* name, uint32_t len)
{
    struct app_bt_map_param_t param;
    param = app_bt_pts_map_get_param(name, len);
    if (!param.succ)
    {
        return;
    }

    const char *content_data =
        "Date: 20 Jun 96\r\n"
        "Subject: Fish\r\n"
        "From:Pts_emailaddress\r\n"
        "To:Iut_emailaddress\r\n"
        "Let's go fishing!\r\n"
        "BR, PtsTest\r\n";

    btif_map_put_email_continue(param.connhdl, param.mas_instance_id, content_data);
}

static void app_bt_pts_map_put_end(const char* name, uint32_t len)
{
    struct app_bt_map_param_t param;
    param = app_bt_pts_map_get_param(name, len);
    if (!param.succ)
    {
        return;
    }

    const char *content_data =
        "Date: 20 Jun 96\r\n"
        "Subject: Fish\r\n"
        "From:Pts_emailaddress\r\n"
        "To:Iut_emailaddress\r\n"
        "Let's go fishing!\r\n"
        "BR, PtsTest\r\n";

    btif_map_put_email_end(param.connhdl, param.mas_instance_id, content_data);
}

static void app_bt_pts_map_get_instance_info(const char* name, uint32_t len)
{
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (param.succ)
    {
        btif_map_get_instance_info(param.connhdl, param.mas_instance_id);
    }
}

static void app_bt_pts_map_get_object_test(const char* name, uint32_t len)
{
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    const char *object_type = NULL;
    const char *object_name = NULL;
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_get_object_test: invalid param");
        return;
    }
    if (param.extra_param[0] == '|')
    {
        object_type = NULL;
        object_name = param.extra_param + 1;
    }
    else
    {
        char *p = param.extra_param;
        while (*p != 0 && *p != '|')
        {
            p += 1;
        }
        if (*p == '|')
        {
            *p = 0;
            object_type = param.extra_param;
            object_name = p + 1;
        }
        else
        {
            object_type = NULL;
            object_name = param.extra_param;
        }
    }
    TRACE(0, "app_bt_pts_map_get_object_test: type %s name %s", object_type ? object_type : "n/a",
        object_name ? object_name : "n/a");
    btif_map_get_object(param.connhdl, param.mas_instance_id, object_type, object_name);
}

static void app_bt_pts_map_update_inbox(const char* name, uint32_t len)
{
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (param.succ)
    {
        btif_map_update_inbox(param.connhdl, param.mas_instance_id);
    }
}

static void app_bt_pts_map_notify_register(const char* name, uint32_t len)
{
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_notify_register: invalid param");
        return;
    }
    btif_map_notify_register(param.connhdl, param.mas_instance_id, param.extra_param[0] == '1');
}

static void app_bt_pts_map_notify_filter(const char* name, uint32_t len)
{
    uint32_t masks = 0;
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_notify_register: invalid param");
        return;
    }
    masks = (uint32_t)map_str_to_hex_value(param.extra_param, strlen(param.extra_param));
    TRACE(0, "app_bt_pts_map_notify_filter: masks %04x", masks);
    btif_map_notify_filter(param.connhdl, param.mas_instance_id, masks);
}

static void app_bt_pts_map_set_msg_read(const char* name, uint32_t len)
{
    uint64_t handle = 0;
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_set_msg_read: invalid param");
        return;
    }
    handle = map_str_to_hex_value(param.extra_param, strlen(param.extra_param));
    TRACE(0, "app_bt_pts_map_set_msg_read: handle %04x %04x", (uint32_t)(handle>>32), (uint32_t)handle);
    btif_map_set_msg_read_status(param.connhdl, param.mas_instance_id, handle, true);
}

static void app_bt_pts_map_set_msg_unread(const char* name, uint32_t len)
{
    uint64_t handle = 0;
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_set_msg_unread: invalid param");
        return;
    }
    handle = map_str_to_hex_value(param.extra_param, strlen(param.extra_param));
    TRACE(0, "app_bt_pts_map_set_msg_unread: handle %04x %04x", (uint32_t)(handle>>32), (uint32_t)handle);
    btif_map_set_msg_read_status(param.connhdl, param.mas_instance_id, handle, false);
}

static void app_bt_pts_map_set_msg_delete(const char* name, uint32_t len)
{
    uint64_t handle = 0;
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_set_msg_delete: invalid param");
        return;
    }
    handle = map_str_to_hex_value(param.extra_param, strlen(param.extra_param));
    TRACE(0, "app_bt_pts_map_set_msg_delete: handle %04x %04x", (uint32_t)(handle>>32), (uint32_t)handle);
    btif_map_set_msg_delete_status(param.connhdl, param.mas_instance_id, handle, true);
}

static void app_bt_pts_map_set_msg_undelete(const char* name, uint32_t len)
{
    uint64_t handle = 0;
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_set_msg_undelete: invalid param");
        return;
    }
    handle = map_str_to_hex_value(param.extra_param, strlen(param.extra_param));
    TRACE(0, "app_bt_pts_map_set_msg_undelete: handle %04x %04x", (uint32_t)(handle>>32), (uint32_t)handle);
    btif_map_set_msg_delete_status(param.connhdl, param.mas_instance_id, handle, false);
}

static void app_bt_pts_map_set_msg_extdata(const char* name, uint32_t len)
{
    uint64_t handle = 0;
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_set_msg_extdata: invalid param");
        return;
    }
    handle = map_str_to_hex_value(param.extra_param, strlen(param.extra_param));
    TRACE(0, "app_bt_pts_map_set_msg_extdata: handle %04x %04x", (uint32_t)(handle>>32), (uint32_t)handle);
    btif_map_set_msg_extended_data(param.connhdl, param.mas_instance_id, handle, "0:18;2:486;3:11;");
}

static void app_bt_pts_map_get_folder_listing_size(const char* name, uint32_t len)
{
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (param.succ)
    {
        btif_map_get_folder_listing_size(param.connhdl, param.mas_instance_id);
    }
}

static void app_bt_pts_map_get_folder_listing(const char* name, uint32_t len)
{
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (param.succ)
    {
        btif_map_get_folder_listing(param.connhdl, param.mas_instance_id, NULL);
    }
}

static void app_bt_pts_map_set_srm_in_wait(const char* name, uint32_t len)
{
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_set_srm_in_wait: invalid param");
        return;
    }
    btif_map_set_srm_in_wait(param.connhdl, param.mas_instance_id, param.extra_param[0] == '1');
}

static void app_bt_pts_map_get_message(const char* name, uint32_t len)
{
    uint64_t handle = 0;
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_get_message: invalid param");
        return;
    }
    handle = map_str_to_hex_value(param.extra_param, strlen(param.extra_param));
    TRACE(0, "app_bt_pts_map_get_message: handle %04x %04x", (uint32_t)(handle>>32), (uint32_t)handle);
    btif_map_get_message(param.connhdl, param.mas_instance_id, handle);
}

static void app_bt_pts_map_get_msg_listing_size(const char* name, uint32_t len)
{
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (param.succ)
    {
        btif_map_get_msg_listing_size(param.connhdl, param.mas_instance_id, NULL);
    }
}

static void app_bt_pts_map_get_message_listing(const char* name, uint32_t len)
{
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (param.succ)
    {
        btif_map_get_message_listing(param.connhdl, param.mas_instance_id, NULL);
    }
}

static void app_bt_pts_map_get_message_listing_of_type(const char* name, uint32_t len)
{
    uint8_t masks = 0;
    struct map_get_msg_listing_param_t gl_param = {{0},0};
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_get_message_listing_of_type: invalid param");
        return;
    }
    gl_param.has_message_type_filter = true;
    masks = (uint8_t)map_str_to_hex_value(param.extra_param, strlen(param.extra_param));
    if (masks & 0x80)
    {
        masks = masks & 0x1F;
        if (masks == 0x01) masks = MAP_FILTER_MSG_TYPE_GET_SMS_GSM;
        else if (masks == 0x02) masks = MAP_FILTER_MSG_TYPE_GET_SMS_CDMA;
        else if (masks == 0x04) masks = MAP_FILTER_MSG_TYPE_GET_EMAIL;
        else if (masks == 0x08) masks = MAP_FILTER_MSG_TYPE_GET_MMS;
        else if (masks == 0x10) masks = MAP_FILTER_MSG_TYPE_GET_IM;
        else masks = MAP_FILTER_MSG_TYPE_GET_SMS_GSM;
    }
    gl_param.filter_msg_type_masks = masks;
    btif_map_get_message_listing(param.connhdl, param.mas_instance_id, &gl_param);
}

static void app_bt_pts_map_get_message_listing_of_handle(const char* name, uint32_t len)
{
    struct map_get_msg_listing_param_t gl_param = {{0},0};
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_get_message_listing_of_handle: invalid param");
        return;
    }
    gl_param.use_message_handle = true;
    gl_param.message_handle = map_str_to_hex_value(param.extra_param, strlen(param.extra_param));
    btif_map_get_message_listing(param.connhdl, param.mas_instance_id, &gl_param);
}

static void app_bt_pts_map_get_message_listing_of_readstatus(const char* name, uint32_t len)
{
    struct map_get_msg_listing_param_t gl_param = {{0},0};
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_get_message_listing_of_readstatus: invalid param");
        return;
    }
    gl_param.filter_read_status = (param.extra_param[0] == '0') ?
        MAP_FILTER_READ_STATUS_GET_UNREAD_MSGS : MAP_FILTER_READ_STATUS_GET_READED_MSGS;
    btif_map_get_message_listing(param.connhdl, param.mas_instance_id, &gl_param);
}

static void app_bt_pts_map_get_message_listing_of_priority(const char* name, uint32_t len)
{
    struct map_get_msg_listing_param_t gl_param = {{0},0};
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_get_message_listing_of_priority: invalid param");
        return;
    }
    gl_param.filter_priority = (param.extra_param[0] == '0') ?
        MAP_FILTER_PRIORITY_GET_NON_HIGH_PRIO_MSG : MAP_FILTER_PRIORITY_GET_HIGH_PRIO_MSG;
    btif_map_get_message_listing(param.connhdl, param.mas_instance_id, &gl_param);
}

static void app_bt_pts_map_get_message_listing_of_originator(const char* name, uint32_t len)
{
    struct map_get_msg_listing_param_t gl_param = {{0},0};
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ)
    {
        TRACE(0, "app_bt_pts_map_get_message_listing_of_originator: invalid param");
        return;
    }
    gl_param.has_originator_fileter = true;
    memcpy(gl_param.originator, "PTS", sizeof("PTS"));
    btif_map_get_message_listing(param.connhdl, param.mas_instance_id, &gl_param);
}

static void app_bt_pts_map_get_message_listing_of_recipient(const char* name, uint32_t len)
{
    struct map_get_msg_listing_param_t gl_param = {{0},0};
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ)
    {
        TRACE(0, "app_bt_pts_map_get_message_listing_of_recipient: invalid param");
        return;
    }
    gl_param.has_recipient_filter = true;
    memcpy(gl_param.recipient, "IUT", sizeof("IUT"));
    btif_map_get_message_listing(param.connhdl, param.mas_instance_id, &gl_param);
}

static void app_bt_pts_map_get_message_listing_of_period_begin(const char* name, uint32_t len)
{
    struct map_get_msg_listing_param_t gl_param = {{0},0};
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ)
    {
        TRACE(0, "app_bt_pts_map_get_message_listing_of_period_begin: invalid param");
        return;
    }
    gl_param.has_period_begin_filter = true;
    memcpy(gl_param.filter_period_begin, "20100101T000000", sizeof("20100101T000000"));
    btif_map_get_message_listing(param.connhdl, param.mas_instance_id, &gl_param);
}

static void app_bt_pts_map_get_message_listing_of_period_end(const char* name, uint32_t len)
{
    struct map_get_msg_listing_param_t gl_param = {{0},0};
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ)
    {
        TRACE(0, "app_bt_pts_map_get_message_listing_of_period_end: invalid param");
        return;
    }
    gl_param.has_period_end_filter = true;
    memcpy(gl_param.filter_period_end, "20111231T125959", sizeof("20111231T125959"));
    btif_map_get_message_listing(param.connhdl, param.mas_instance_id, &gl_param);
}

static void app_bt_pts_map_get_message_listing_of_period_bend(const char* name, uint32_t len)
{
    struct map_get_msg_listing_param_t gl_param = {{0},0};
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ)
    {
        TRACE(0, "app_bt_pts_map_get_message_listing_of_period_bend: invalid param");
        return;
    }
    gl_param.has_period_begin_filter = true;
    gl_param.has_period_end_filter = true;
    memcpy(gl_param.filter_period_begin, "20100101T000000", sizeof("20100101T000000"));
    memcpy(gl_param.filter_period_end, "20111231T125959", sizeof("20111231T125959"));
    btif_map_get_message_listing(param.connhdl, param.mas_instance_id, &gl_param);
}

static void app_bt_pts_map_get_msg_listing_size_of_convoid(const char* name, uint32_t len)
{
    int convo_id_len = 0;
    struct map_get_msg_listing_param_t gl_param = {{0},0};
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_get_msg_listing_size_of_convoid: invalid param");
        return;
    }
    gl_param.use_conversation_id = true;
    convo_id_len = strlen(param.extra_param);
    if (convo_id_len > MAP_CONVERSATION_ID_STR_LEN)
    {
        convo_id_len = MAP_CONVERSATION_ID_STR_LEN;
    }
    if (convo_id_len)
    {
        memcpy(gl_param.folder_name, param.extra_param, convo_id_len);
    }
    btif_map_get_msg_listing_size(param.connhdl, param.mas_instance_id, &gl_param);
}

static void app_bt_pts_map_get_message_listing_of_convoid(const char* name, uint32_t len)
{
    int convo_id_len = 0;
    struct map_get_msg_listing_param_t gl_param = {{0},0};
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_get_message_listing_of_convoid: invalid param");
        return;
    }
    gl_param.use_conversation_id = true;
    convo_id_len = strlen(param.extra_param);
    if (convo_id_len > MAP_CONVERSATION_ID_STR_LEN)
    {
        convo_id_len = MAP_CONVERSATION_ID_STR_LEN;
    }
    if (convo_id_len)
    {
        memcpy(gl_param.folder_name, param.extra_param, convo_id_len);
    }
    btif_map_get_message_listing(param.connhdl, param.mas_instance_id, &gl_param);
}


static void app_bt_pts_map_get_owner_status(const char* name, uint32_t len)
{
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_get_owner_status: invalid param");
        return;
    }
    TRACE(0, "app_bt_pts_map_get_owner_status: conversation id %s", param.extra_param);
    btif_map_get_owner_status(param.connhdl, param.mas_instance_id, param.extra_param);
}

static void app_bt_pts_map_set_owner_status(const char* name, uint32_t len)
{
    uint16_t convo_id_len = 0;
    struct map_set_owner_status_param_t set_param = {0};
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_set_owner_status: invalid param");
        return;
    }
    TRACE(0, "app_bt_pts_map_set_owner_status: conversation id %s", param.extra_param);
    convo_id_len = strlen(param.extra_param);
    if (convo_id_len > MAP_CONVERSATION_ID_STR_LEN)
    {
        convo_id_len = MAP_CONVERSATION_ID_STR_LEN;
    }
    if (convo_id_len)
    {
        memcpy(set_param.convo_id, param.extra_param, convo_id_len);
    }
    set_param.has_presence_avail = true;
    set_param.presence_avail = MAP_PRESENCE_IN_A_MEETING;
    set_param.has_chat_state = true;
    set_param.chat_state = MAP_CHATSTATE_INACTIVE;
    btif_map_set_owner_status(param.connhdl, param.mas_instance_id, &set_param);
}

static void app_bt_pts_map_get_convo_listing_size(const char* name, uint32_t len)
{
    uint16_t convo_id_len = 0;
    struct map_get_convo_listing_param_t get_param = {0};
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_get_convo_listing_size: invalid param");
        return;
    }
    TRACE(0, "app_bt_pts_map_get_convo_listing_size: conversation id %s", param.extra_param);
    convo_id_len = strlen(param.extra_param);
    if (convo_id_len > MAP_CONVERSATION_ID_STR_LEN)
    {
        convo_id_len = MAP_CONVERSATION_ID_STR_LEN;
    }
    if (convo_id_len)
    {
        memcpy(get_param.convo_id, param.extra_param, convo_id_len);
    }
    btif_map_get_convo_listing_size(param.connhdl, param.mas_instance_id, &get_param);
}

static void app_bt_pts_map_get_convo_listing(const char* name, uint32_t len)
{
    uint16_t convo_id_len = 0;
    struct map_get_convo_listing_param_t get_param = {0};
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_get_convo_listing: invalid param");
        return;
    }
    TRACE(0, "app_bt_pts_map_get_convo_listing: conversation id %s", param.extra_param);
    convo_id_len = strlen(param.extra_param);
    if (convo_id_len > MAP_CONVERSATION_ID_STR_LEN)
    {
        convo_id_len = MAP_CONVERSATION_ID_STR_LEN;
    }
    if (convo_id_len)
    {
        memcpy(get_param.convo_id, param.extra_param, convo_id_len);
    }
    btif_map_get_convo_listing(param.connhdl, param.mas_instance_id, &get_param);
}

static void app_bt_pts_map_get_convo_listing_by_readstatus(const char* name, uint32_t len)
{
    uint16_t convo_id_len = 0;
    const char *convo_id_str = NULL;
    const char *extra_str = NULL;
    struct map_get_convo_listing_param_t get_param = {0};
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_get_convo_listing_by_readstatus: invalid param");
        return;
    }
    char *p = param.extra_param;
    while (*p != 0 && *p != '|')
    {
        p += 1;
    }
    if (*p == '|')
    {
        *p = 0;
        convo_id_str = param.extra_param;
        extra_str = p + 1;
    }
    else
    {
        TRACE(0, "app_bt_pts_map_get_convo_listing_by_readstatus: invalid readstatus param");
        return;
    }
    TRACE(0, "app_bt_pts_map_get_convo_listing_by_readstatus: conversation id %s readstatus %s", convo_id_str, extra_str);
    convo_id_len = strlen(convo_id_str);
    if (convo_id_len > MAP_CONVERSATION_ID_STR_LEN)
    {
        convo_id_len = MAP_CONVERSATION_ID_STR_LEN;
    }
    if (convo_id_len)
    {
        memcpy(get_param.convo_id, convo_id_str, convo_id_len);
    }
    get_param.filter_read_status = (extra_str[0] == '0') ?
        MAP_FILTER_READ_STATUS_GET_UNREAD_MSGS : MAP_FILTER_READ_STATUS_GET_READED_MSGS;
    btif_map_get_convo_listing(param.connhdl, param.mas_instance_id, &get_param);
}

static void app_bt_pts_map_get_convo_listing_by_recipient(const char* name, uint32_t len)
{
    uint16_t convo_id_len = 0;
    struct map_get_convo_listing_param_t get_param = {0};
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_get_convo_listing_by_recipient: invalid param");
        return;
    }
    TRACE(0, "app_bt_pts_map_get_convo_listing_by_recipient: conversation id %s", param.extra_param);
    convo_id_len = strlen(param.extra_param);
    if (convo_id_len > MAP_CONVERSATION_ID_STR_LEN)
    {
        convo_id_len = MAP_CONVERSATION_ID_STR_LEN;
    }
    if (convo_id_len)
    {
        memcpy(get_param.convo_id, param.extra_param, convo_id_len);
    }
    get_param.has_recipient_filter = true;
    memcpy(get_param.recipient, "IUT", sizeof("IUT"));
    btif_map_get_convo_listing(param.connhdl, param.mas_instance_id, &get_param);
}

static void app_bt_pts_map_get_convo_listing_by_last_activity(const char* name, uint32_t len)
{
    uint16_t convo_id_len = 0;
    struct map_get_convo_listing_param_t get_param = {0};
    struct app_bt_map_param_t param = app_bt_pts_map_get_param(name, len);
    if (!param.succ || param.extra_param == NULL)
    {
        TRACE(0, "app_bt_pts_map_get_convo_listing_by_last_activity: invalid param");
        return;
    }
    TRACE(0, "app_bt_pts_map_get_convo_listing_by_last_activity: conversation id %s", param.extra_param);
    convo_id_len = strlen(param.extra_param);
    if (convo_id_len > MAP_CONVERSATION_ID_STR_LEN)
    {
        convo_id_len = MAP_CONVERSATION_ID_STR_LEN;
    }
    if (convo_id_len)
    {
        memcpy(get_param.convo_id, param.extra_param, convo_id_len);
    }
    get_param.has_last_activity_begin_filter = true;
    get_param.has_last_activity_end_filter = true;
    memcpy(get_param.filter_last_activity_begin, "20100101T000000", sizeof("20100101T000000"));
    memcpy(get_param.filter_last_activity_end, "20111231T125959", sizeof("20111231T125959"));
    btif_map_get_convo_listing(param.connhdl, param.mas_instance_id, &get_param);
}

static app_bt_host_cmd_table_t app_map_cmd_table[] =
{
    {"map_connect",             app_bt_pts_map_connect},
    {"map_disconnect",          app_bt_pts_map_disconnect},
    {"map_set_over_rfcomm",     app_bt_pts_set_obex_over_rfcomm},
    {"map_clear_over_rfcomm",   app_bt_pts_clear_obex_over_rfcomm},
    {"map_open_all_mas",        app_bt_pts_map_open_all_mas_channel},
    {"map_mns_obex_disc",       app_bt_pts_map_mns_obex_disc_req},
    {"map_close_mns",           app_bt_pts_map_close_mns_channel},
    {"map_client_test",         app_bt_pts_map_client_test},
    {"map_send_sms",            app_bt_pts_map_send_sms},
    {"map_obex_disc",           app_bt_pts_map_obex_disc_req},          // map_obex_disc|0:mas_instance_id
    {"map_obex_conn",           app_bt_pts_map_obex_conn_req},          // map_obex_conn|0
    {"map_dont_auto_conn",      app_bt_pts_map_dont_auto_conn_req},     // map_dont_auto_conn|0
    {"map_connect_mas",         app_bt_pts_map_connect_mas},            // map_connect_mas|0
    {"map_open_mas",            app_bt_pts_map_open_mas_channel},       // map_open_mas|0
    {"map_close_mas",           app_bt_pts_map_close_mas_channel},      // map_close_mas|0
    {"map_to_root",             app_bt_pts_map_enter_to_root_folder},   // map_to_root|0
    {"map_to_parent",           app_bt_pts_map_enter_to_parent_folder}, // map_to_parent|0
    {"map_to_msg_folder",       app_bt_pts_map_enter_to_msg_folder},    // map_to_msg_foler|0
    {"map_to_child",            app_bt_pts_map_enter_to_child_folder},  // map_to_child|0|telecom:folder_name
    {"map_send_gsm_sms",        app_bt_pts_map_send_gsm_sms},           // map_send_gsm_sms|0
    {"map_send_cdma_sms",       app_bt_pts_map_send_cdma_sms},          // map_send_cdma_sms|0
    {"map_send_mms",            app_bt_pts_map_send_mms},               // map_send_mms|0
    {"map_send_im",             app_bt_pts_map_send_im},                // map_send_im|0
    {"map_send_email",          app_bt_pts_map_send_email},             // map_send_email|0
    {"map_replace_email",       app_bt_pts_map_replace_email},          // map_replace_email|0|dd:msg_handle
    {"map_forward_email",       app_bt_pts_map_forward_email},          // map_forward_email|0|dd
    {"map_forward_incl_attach", app_bt_pts_map_forward_including_attachment},
    {"map_push_to_convo",       app_bt_pts_map_push_to_conversation},   // map_push_to_convo:0|cc:conversation_id
    {"map_put_start",           app_bt_pts_map_put_start},              // map_put_start|0|30
    {"map_put_cont",            app_bt_pts_map_put_continue},           // map_put_cont|0
    {"map_put_end",             app_bt_pts_map_put_end},                // map_put_end|0
    {"map_get_inst_info",       app_bt_pts_map_get_instance_info},      // map_get_inst_info|0
    {"map_get_object",          app_bt_pts_map_get_object_test},        // map_get_object|0|put.gif
    {"map_update_inbox",        app_bt_pts_map_update_inbox},           // map_update_inbox|0
    {"map_notify_register",     app_bt_pts_map_notify_register},        // map_notify_register|0|1:on_or_off
    {"map_notify_filter",       app_bt_pts_map_notify_filter},          // map_notify_filter|0|1ff:notify_masks
    {"map_set_read",            app_bt_pts_map_set_msg_read},           // map_set_read|0|dd:msg_handle
    {"map_set_unread",          app_bt_pts_map_set_msg_unread},         // map_set_unread|0|dd
    {"map_set_delete",          app_bt_pts_map_set_msg_delete},         // map_set_delete|0|dd
    {"map_set_undelete",        app_bt_pts_map_set_msg_undelete},       // map_set_undelete|0|dd
    {"map_set_extdata",         app_bt_pts_map_set_msg_extdata},        // map_set_extdata|0|dd
    {"map_get_fl",              app_bt_pts_map_get_folder_listing},     // map_get_fl|0
    {"map_get_fl_size",         app_bt_pts_map_get_folder_listing_size},// map_get_fl_size|0
    {"map_srm_wait",            app_bt_pts_map_set_srm_in_wait},        // map_srm_wait|0|1
    {"map_get_msg",             app_bt_pts_map_get_message},            // map_get_msg|0|dd
    {"map_get_ml",              app_bt_pts_map_get_message_listing},    // map_get_ml|0
    {"map_get_ml_size",         app_bt_pts_map_get_msg_listing_size},   // map_get_ml_size|0
    {"map_get_ml_type",         app_bt_pts_map_get_message_listing_of_type},        // map_get_ml_type|0|84
    {"map_get_ml_handle",       app_bt_pts_map_get_message_listing_of_handle},      // map_get_ml_handle|0|dd
    {"map_get_ml_sz_convoid",   app_bt_pts_map_get_msg_listing_size_of_convoid},    // map_get_ml_sz_convoid|0|cc
    {"map_get_ml_convoid",      app_bt_pts_map_get_message_listing_of_convoid},     // map_get_ml_convoid|0|cc
    {"map_get_ml_readstatus",   app_bt_pts_map_get_message_listing_of_readstatus},  // map_get_ml_readstatus|0|0
    {"map_get_ml_priority",     app_bt_pts_map_get_message_listing_of_priority},    // map_get_ml_priority|0|1
    {"map_get_ml_originator",   app_bt_pts_map_get_message_listing_of_originator},  // map_get_ml_originator|0
    {"map_get_ml_recipient",    app_bt_pts_map_get_message_listing_of_recipient},   // map_get_ml_recipient|0
    {"map_get_ml_period_begin", app_bt_pts_map_get_message_listing_of_period_begin},// map_get_ml_period_begin|0
    {"map_get_ml_period_end",   app_bt_pts_map_get_message_listing_of_period_end},  // map_get_ml_period_end|0
    {"map_get_ml_period_bend",  app_bt_pts_map_get_message_listing_of_period_bend}, // map_get_ml_period_bend|0
    {"map_get_owner",           app_bt_pts_map_get_owner_status},       // map_get_owner|0|cc:conversation_id
    {"map_set_owner",           app_bt_pts_map_set_owner_status},       // map_set_owner|0|cc
    {"map_get_cl",              app_bt_pts_map_get_convo_listing},      // map_get_cl|0|cc
    {"map_get_cl_size",         app_bt_pts_map_get_convo_listing_size}, // map_get_cl_size|0|cc
    {"map_get_cl_readstatus",   app_bt_pts_map_get_convo_listing_by_readstatus},        // map_get_cl_readstatus|0|cc|1
    {"map_get_cl_recipient",    app_bt_pts_map_get_convo_listing_by_recipient},         // map_get_cl_recipient|0|cc
    {"map_get_cl_last_activity",app_bt_pts_map_get_convo_listing_by_last_activity},     // map_get_cl_last_activity|0|cc
}
#endif /* BT_MAP_TEST_SUPPORT */

static bool g_bt_map_initialized = false;

bt_status_t bt_map_init(bt_map_callback_t callback)
{
    TRACE(0, "bt_map_init: callback %p", callback);
    if (callback)
    {
        btif_add_bt_event_callback((bt_event_callback_t)callback, BT_EVENT_MASK_MAP_GROUP);
    }
    if (g_bt_map_initialized)
    {
        return BT_STS_SUCCESS;
    }
    g_bt_map_initialized = true;
    btif_map_init();
    btif_add_bt_event_callback(bt_map_event_callback, BT_EVENT_MASK_MAP_GROUP);

#ifdef BT_MAP_TEST_SUPPORT
    app_bt_host_add_cmd_table(sizeof(app_map_cmd_table)/sizeof(app_map_cmd_table[0]),
        app_map_cmd_table);
#endif

    return BT_STS_SUCCESS;
}
#endif /* BT_MAP_SUPPORT */

