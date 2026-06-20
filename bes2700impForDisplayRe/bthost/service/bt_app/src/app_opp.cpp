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
#ifdef BT_OPP_SUPPORT

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
#include "app_opp.h"
#include "opp_api.h"

#define OPP_PUSH_VCARD_TYPE         "text/x-vcard"

#define OPP_PUSH_VCARD1_NAME        "bes_vcard_test1.vcf"
#define OPP_PUSH_VCARD2_NAME        "bes_vcard_test2.vcf"

#define OPP_PUSH_VCAL_TOTAL_NAME     "OPHBV24.vcs"
#define OPP_PUSH_VCAL1_NAME         "bes_vcal_test1.vcs"
#define OPP_PUSH_VCAL2_NAME         "bes_vcal_test2.vcs"

#define OPP_PUSH_VMSG_TOTAL_NAME     "OPHBV25.vmg"
#define OPP_PUSH_VMSG1_NAME         "bes_vmg_test1.vmg"
#define OPP_PUSH_VMSG2_NAME         "bes_vmg_test2.vmg"

#define OPP_PUSH_VNT_TOTAL_NAME     "OPHBV26.vnt"
#define OPP_PUSH_VNT1_NAME          "bes_vnt_test1.vnt"
#define OPP_PUSH_VNT2_NAME          "bes_vnt_test2.vnt"

#define OPP_PUSH_UNSUPPORT_NAME     "OPHBV22.vdt"

/**
 *BEGIN:VCARD\r\nVERSION:2.1\r\nN:OPHBV23-1\r\nTEL;Work:1-425-691-3532\r\nEND:VCARD\r\n
 *BEGIN:VCARD\r\nVERSION:2.1\r\nN:OPHBV23-2\r\nTEL;Work:1-425-691-3532\r\nEND:VCARD\r\n
**/

const static char pts_vcard1_content[] = {
0x42, 0x45, 0x47, 0x49, 0x4E, 0x3A, 0x56, 0x43, 0x41, 0x52, 0x44, 0x0D, 0x0A, 0x56, 0x45, 0x52, 0x53, 0x49, 0x4F, 0x4E,
0x3A, 0x32, 0x2E, 0x31, 0x0D, 0x0A, 0x4E, 0x3A, 0x4F, 0x50, 0x48, 0x42, 0x56, 0x32, 0x33, 0x2D, 0x31, 0x0D, 0x0A, 0x54,
0x45, 0x4C, 0x3B, 0x57, 0x6F, 0x72, 0x6B, 0x3A, 0x31, 0x2D, 0x34, 0x32, 0x35, 0x2D, 0x36, 0x39, 0x31, 0x2D, 0x33, 0x35,
0x33, 0x32, 0x0D, 0x0A, 0x45, 0x4E, 0x44, 0x3A, 0x56, 0x43, 0x41, 0x52, 0x44, 0x0D, 0x0A};

/**
 *"BEGIN:VCALENDAR\r\nVERSION:1.0\r\nBEGIN:VEVENT\r\nDTSTART:20220824T021701Z\r\nDTEND:20220824T024701Z\r\nSUMMARY:OPHBV24-1\r\nDESCRIPTION:OPHBV24-1\r\nEND:VEVENT\r\n"
 *"BEGIN:VEVENT\r\nDTSTART:20220824T021701Z\r\nDTEND:20220824T024701Z\r\nSUMMARY:OPHBV24-2\r\nDESCRIPTION:OPHBV24-2\r\nEND:VEVENT\r\nEND:VCALENDAR\r\n"
**/
const static char pts_vcal1_content[] = {
0x42, 0x45, 0x47, 0x49, 0x4E, 0x3A, 0x56, 0x43, 0x41, 0x4C, 0x45, 0x4E, 0x44, 0x41, 0x52, 0x0D, 0x0A, 0x56, 0x45, 0x52,
0x53, 0x49, 0x4F, 0x4E, 0x3A, 0x31, 0x2E, 0x30, 0x0D, 0x0A, 0x42, 0x45, 0x47, 0x49, 0x4E, 0x3A, 0x56, 0x45, 0x56, 0x45,
0x4E, 0x54, 0x0D, 0x0A, 0x44, 0x54, 0x53, 0x54, 0x41, 0x52, 0x54, 0x3A, 0x32, 0x30, 0x32, 0x32, 0x30, 0x38, 0x32, 0x34,
0x54, 0x30, 0x32, 0x31, 0x37, 0x30, 0x31, 0x5A, 0x0D, 0x0A, 0x44, 0x54, 0x45, 0x4E, 0x44, 0x3A, 0x32, 0x30, 0x32, 0x32,
0x30, 0x38, 0x32, 0x34, 0x54, 0x30, 0x32, 0x34, 0x37, 0x30, 0x31, 0x5A, 0x0D, 0x0A, 0x53, 0x55, 0x4D, 0x4D, 0x41, 0x52,
0x59, 0x3A, 0x4F, 0x50, 0x48, 0x42, 0x56, 0x32, 0x34, 0x2D, 0x31, 0x0D, 0x0A, 0x44, 0x45, 0x53, 0x43, 0x52, 0x49, 0x50,
0x54, 0x49, 0x4F, 0x4E, 0x3A, 0x4F, 0x50, 0x48, 0x42, 0x56, 0x32, 0x34, 0x2D, 0x31, 0x0D, 0x0A, 0x45, 0x4E, 0x44, 0x3A,
0x56, 0x45, 0x56, 0x45, 0x4E, 0x54, 0x0D, 0x0A};

/**
 *"BEGIN:VMSG\r\nVERSION:1.0\r\nBEGIN:VENV\r\nBEGIN:VBODY\r\nSubject: OPHBV25-1\r\nOPHBV25-1\r\nEND:VBODY\r\nEND:VENV\r\nEND:VMSG\r\n"
 *"BEGIN:VMSG\r\nVERSION:1.0\r\nBEGIN:VENV\r\nBEGIN:VBODY\r\nSubject: OPHBV25-2\r\nOPHBV25-2\r\nEND:VBODY\r\nEND:VENV\r\nEND:VMSG\r\n"
**/
const static char pts_msg1_content[] = {
0x42, 0x45, 0x47, 0x49, 0x4E, 0x3A, 0x56, 0x4D, 0x53, 0x47, 0x0D, 0x0A, 0x56, 0x45, 0x52, 0x53, 0x49, 0x4F, 0x4E, 0x3A,
0x31, 0x2E, 0x30, 0x0D, 0x0A, 0x42, 0x45, 0x47, 0x49, 0x4E, 0x3A, 0x56, 0x45, 0x4E, 0x56, 0x0D, 0x0A, 0x42, 0x45, 0x47,
0x49, 0x4E, 0x3A, 0x56, 0x42, 0x4F, 0x44, 0x59, 0x0D, 0x0A, 0x53, 0x75, 0x62, 0x6A, 0x65, 0x63, 0x74, 0x3A, 0x20, 0x4F,
0x50, 0x48, 0x42, 0x56, 0x32, 0x35, 0x2D, 0x31, 0x0D, 0x0A, 0x4F, 0x50, 0x48, 0x42, 0x56, 0x32, 0x35, 0x2D, 0x31, 0x0D,
0x0A, 0x45, 0x4E, 0x44, 0x3A, 0x56, 0x42, 0x4F, 0x44, 0x59, 0x0D, 0x0A, 0x45, 0x4E, 0x44, 0x3A, 0x56, 0x45, 0x4E, 0x56,
0x0D, 0x0A, 0x45, 0x4E, 0x44, 0x3A, 0x56, 0x4D, 0x53, 0x47, 0x0D, 0x0A};

/**
 *BEGIN:VNOTE\r\nVERSION:1.1\r\nBODY: OPHBV16-1\r\nEND:VNOTE\r\n
 *BEGIN:VNOTE\r\nVERSION:1.1\r\nBODY: OPHBV16-2\r\nEND:VNOTE\r\n
**/
const static char pts_note1_content[] = {
0x42, 0x45, 0x47, 0x49, 0x4E, 0x3A, 0x56, 0x4E, 0x4F, 0x54, 0x45, 0x0D, 0x0A, 0x56, 0x45, 0x52, 0x53, 0x49, 0x4F, 0x4E,
0x3A, 0x31, 0x2E, 0x31, 0x0D, 0x0A, 0x42, 0x4F, 0x44, 0x59, 0x3A, 0x20, 0x4F, 0x50, 0x48, 0x42, 0x56, 0x32, 0x36, 0x2D,
0x31, 0x0D, 0x0A, 0x45, 0x4E, 0x44, 0x3A, 0x56, 0x4E, 0x4F, 0x54, 0x45, 0x0D, 0x0A};

const static char pts_large_file_content[1024] = {
0x42, 0x45, 0x47, 0x49, 0x4E, 0x3A, 0x56, 0x43, 0x41, 0x52, 0x44, 0x0D, 0x0A, 0x56, 0x45, 0x52, 0x53, 0x49, 0x4F, 0x4E,
0x3A, 0x32, 0x2E, 0x31, 0x0D, 0x0A, 0x4E, 0x3A, 0x4F, 0x50, 0x48, 0x42, 0x56, 0x32, 0x33, 0x2D, 0x31, 0x0D, 0x0A, 0x54,
0x45, 0x4C, 0x3B, 0x57, 0x6F, 0x72, 0x6B, 0x3A, 0x31, 0x2D, 0x34, 0x32, 0x35, 0x2D, 0x36, 0x39, 0x31, 0x2D, 0x33, 0x35,
0x33, 0x32, 0x0D, 0x0A, 0x45, 0x4E, 0x44, 0x3A, 0x56, 0x43, 0x41, 0x52, 0x44, 0x0D, 0x0A, 0x42, 0x45, 0x47, 0x49, 0x4E,
0x3A, 0x56, 0x43, 0x41, 0x52, 0x44, 0x0D, 0x0A, 0x56, 0x45, 0x52, 0x53, 0x49, 0x4F, 0x4E, 0x3A, 0x32, 0x2E, 0x31, 0x0D,
0x42, 0x45, 0x47, 0x49, 0x4E, 0x3A, 0x56, 0x43, 0x41, 0x52, 0x44, 0x0D, 0x0A, 0x56, 0x45, 0x52, 0x53, 0x49, 0x4F, 0x4E,
0x3A, 0x32, 0x2E, 0x31, 0x0D, 0x0A, 0x4E, 0x3A, 0x4F, 0x50, 0x48, 0x42, 0x56, 0x32, 0x33, 0x2D, 0x31, 0x0D, 0x0A, 0x54,
0x45, 0x4C, 0x3B, 0x57, 0x6F, 0x72, 0x6B, 0x3A, 0x31, 0x2D, 0x34, 0x32, 0x35, 0x2D, 0x36, 0x39, 0x31, 0x2D, 0x33, 0x35,
0x33, 0x32, 0x0D, 0x0A, 0x45, 0x4E, 0x44, 0x3A, 0x56, 0x43, 0x41, 0x52, 0x44, 0x0D, 0x0A, 0x42, 0x45, 0x47, 0x49, 0x4E,
0x3A, 0x56, 0x43, 0x41, 0x52, 0x44, 0x0D, 0x0A, 0x56, 0x45, 0x52, 0x53, 0x49, 0x4F, 0x4E, 0x3A, 0x32, 0x2E, 0x31, 0x0D,
0x42, 0x45, 0x47, 0x49, 0x4E, 0x3A, 0x56, 0x43, 0x41, 0x52, 0x44, 0x0D, 0x0A, 0x56, 0x45, 0x52, 0x53, 0x49, 0x4F, 0x4E,
0x3A, 0x32, 0x2E, 0x31, 0x0D, 0x0A, 0x4E, 0x3A, 0x4F, 0x50, 0x48, 0x42, 0x56, 0x32, 0x33, 0x2D, 0x31, 0x0D, 0x0A, 0x54,
0x45, 0x4C, 0x3B, 0x57, 0x6F, 0x72, 0x6B, 0x3A, 0x31, 0x2D, 0x34, 0x32, 0x35, 0x2D, 0x36, 0x39, 0x31, 0x2D, 0x33, 0x35,
0x33, 0x32, 0x0D, 0x0A, 0x45, 0x4E, 0x44, 0x3A, 0x56, 0x43, 0x41, 0x52, 0x44, 0x0D, 0x0A, 0x42, 0x45, 0x47, 0x49, 0x4E,
0x3A, 0x56, 0x43, 0x41, 0x52, 0x44, 0x0D, 0x0A, 0x56, 0x45, 0x52, 0x53, 0x49, 0x4F, 0x4E, 0x3A, 0x32, 0x2E, 0x31, 0x0D,
0x42, 0x45, 0x47, 0x49, 0x4E, 0x3A, 0x56, 0x43, 0x41, 0x52, 0x44, 0x0D, 0x0A, 0x56, 0x45, 0x52, 0x53, 0x49, 0x4F, 0x4E,
0x3A, 0x32, 0x2E, 0x31, 0x0D, 0x0A, 0x4E, 0x3A, 0x4F, 0x50, 0x48, 0x42, 0x56, 0x32, 0x33, 0x2D, 0x31, 0x0D, 0x0A, 0x54,
0x45, 0x4C, 0x3B, 0x57, 0x6F, 0x72, 0x6B, 0x3A, 0x31, 0x2D, 0x34, 0x32, 0x35, 0x2D, 0x36, 0x39, 0x31, 0x2D, 0x33, 0x35,
0x33, 0x32, 0x0D, 0x0A, 0x45, 0x4E, 0x44, 0x3A, 0x56, 0x43, 0x41, 0x52, 0x44, 0x0D, 0x0A, 0x42, 0x45, 0x47, 0x49, 0x4E,
0x3A, 0x56, 0x43, 0x41, 0x52, 0x44, 0x0D, 0x0A, 0x56, 0x45, 0x52, 0x53, 0x49, 0x4F, 0x4E, 0x3A, 0x32, 0x2E, 0x31, 0x0D,
0x42, 0x45, 0x47, 0x49, 0x4E, 0x3A, 0x56, 0x43, 0x41, 0x52, 0x44, 0x0D, 0x0A, 0x56, 0x45, 0x52, 0x53, 0x49, 0x4F, 0x4E,
0x3A, 0x32, 0x2E, 0x31, 0x0D, 0x0A, 0x4E, 0x3A, 0x4F, 0x50, 0x48, 0x42, 0x56, 0x32, 0x33, 0x2D, 0x31, 0x0D, 0x0A, 0x54,
0x45, 0x4C, 0x3B, 0x57, 0x6F, 0x72, 0x6B, 0x3A, 0x31, 0x2D, 0x34, 0x32, 0x35, 0x2D, 0x36, 0x39, 0x31, 0x2D, 0x33, 0x35,
0x33, 0x32, 0x0D, 0x0A, 0x45, 0x4E, 0x44, 0x3A, 0x56, 0x43, 0x41, 0x52, 0x44, 0x0D, 0x0A, 0x42, 0x45, 0x47, 0x49, 0x4E,
0x3A, 0x56, 0x43, 0x41, 0x52, 0x44, 0x0D, 0x0A, 0x56, 0x45, 0x52, 0x53, 0x49, 0x4F, 0x4E, 0x3A, 0x32, 0x2E, 0x31, 0x0D,
0x42, 0x45, 0x47, 0x49, 0x4E, 0x3A, 0x56, 0x43, 0x41, 0x52, 0x44, 0x0D, 0x0A, 0x56, 0x45, 0x52, 0x53, 0x49, 0x4F, 0x4E,
0x3A, 0x32, 0x2E, 0x31, 0x0D, 0x0A, 0x4E, 0x3A, 0x4F, 0x50, 0x48, 0x42, 0x56, 0x32, 0x33, 0x2D, 0x31, 0x0D, 0x0A, 0x54,
0x45, 0x4C, 0x3B, 0x57, 0x6F, 0x72, 0x6B, 0x3A, 0x31, 0x2D, 0x34, 0x32, 0x35, 0x2D, 0x36, 0x39, 0x31, 0x2D, 0x33, 0x35,
0x33, 0x32, 0x0D, 0x0A, 0x45, 0x4E, 0x44, 0x3A, 0x56, 0x43, 0x41, 0x52, 0x44, 0x0D, 0x0A, 0x42, 0x45, 0x47, 0x49, 0x4E,
0x3A, 0x56, 0x43, 0x41, 0x52, 0x44, 0x0D, 0x0A, 0x56, 0x45, 0x52, 0x53, 0x49, 0x4F, 0x4E, 0x3A, 0x32, 0x2E, 0x31, 0x0D};

static opp_pts_event_t pts_test_status = OPP_PTS_IDLE;
static uint8 app_bt_opp_callback(uint8_t device_id, struct btif_opp_channel_t *opp_channel, const struct opp_callback_parm_t *parm)
{
    uint8 ret = OPP_OBJECT_ACCEPT;
    if (device_id == BT_DEVICE_INVALID_ID && parm->event == OPP_CHANNEL_CLOSED)
    {
        // opp profile is closed due to acl created fail
        TRACE(2,"%s CHANNEL_CLOSED acl created error %02x", __func__, parm->error_code);
        return ret;
    }
    switch (parm->event)
    {
        case OPP_OBEX_CONNECTED:
            TRACE(2,"(d%x) %s OBEX_CONNECTED", device_id, __func__);
            break;
        case OPP_CHANNEL_OPENED:
            TRACE(2,"(d%x) %s CHANNEL_OPENED", device_id, __func__);
            break;
        case OPP_CHANNEL_CLOSED:
            TRACE(3,"(d%x) %s CHANNEL_CLOSED %02x", device_id, __func__, parm->error_code);
            break;
        case OPP_RECEIVED_PUT_RSP:
            TRACE(3,"(d%x) %s REV OPP_RECEIVED_PUT_RSP error_code=%d", device_id, __func__, parm->error_code);
            app_bt_pts_opp_object_status_mgr();
            break;
        case OPP_RECEIVED_GET_RSP:
            TRACE(5,"(d%x) %s REV OPP_RECEIVED_GET_RSP error_code=%d body=%p len=%d",
                    device_id, __func__, parm->error_code,
                    parm->pull_context->body_data, parm->pull_context->body_data_length);
            break;
        case OPP_PULL_BUSINESS_VCARD_DONE:
            TRACE(2,"(d%x) %s OPP_PULL_BUSINESS_VCARD_DONE", device_id, __func__);
            break;
        case OPP_PUSH_EXCHANGE_DONE:
            TRACE(2,"(d%x) %s OPP_PUSH_EXCHANGE_DONE", device_id, __func__);
            break;
        case OPP_PUSH_OBJECT_DONE:
            TRACE(3,"(d%x) %s OPP_PUSH_OBJECT_DONE error_code=%d", device_id, __func__, parm->error_code);
            app_bt_pts_opp_object_status_mgr();
            break;
         case OPP_RECEIVED_PUT_REQ:
            TRACE(3,"(d%x) %s REV OPP_RECEIVED_PUT_REQ object_type %s", device_id, __func__,
                (const char*)parm->put_req->object_type ? (const char*)parm->put_req->object_type : "");
            TRACE(3,"(d%x) %s REV OPP_RECEIVED_PUT_REQ unicode_name=%s", device_id, __func__,
                (const char*)parm->put_req->unicode_name ? (const char*)parm->put_req->unicode_name : "");
            app_bt_opp_send_obex_abort_req(opp_channel);
            break;
        case OPP_RECEIVED_GET_REQ:
            TRACE(3,"(d%x) %s REV OPP_RECEIVED_GET_REQ object_type=%s", device_id, __func__,
                (const char*)parm->pull_req->object_type ? (const char*)parm->pull_req->object_type : "");
            TRACE(3,"(d%x) %s REV OPP_RECEIVED_GET_REQ unicode_name=%s", device_id, __func__,
                (const char*)parm->pull_req->unicode_name ? (const char*)parm->pull_req->unicode_name : "");
            break;
        default:
            TRACE(3,"(d%x) %s invalid event %d", device_id, __func__, parm->event);
            break;
    }
    return ret;

}

void app_bt_opp_server_init(void)
{
    struct BT_DEVICE_T *curr_device = NULL;

    btif_opp_server_init(app_bt_opp_callback);
    for (int i = 0; i < BT_DEVICE_NUM; i++)
    {
        curr_device = app_bt_get_device(i);
        curr_device->opp_server_channel = btif_alloc_opp_server_channel();
    }
}

void app_bt_opp_client_init(void)
{
    struct BT_DEVICE_T *curr_device = NULL;
    btif_opp_init(app_bt_opp_callback);
    for (int i = 0; i < BT_DEVICE_NUM; i++)
    {
        curr_device = app_bt_get_device(i);
        curr_device->opp_channel = btif_alloc_opp_channel();
    }
}

void app_bt_connect_opp_profile(bt_bdaddr_t *bdaddr)
{
    static bt_bdaddr_t remote;

    remote = *bdaddr;

    TRACE(7, "%s address %02x:%02x:%02x:%02x:%02x:%02x", __func__,
        remote.address[0], remote.address[1], remote.address[2],
        remote.address[3], remote.address[4], remote.address[5]);

    app_bt_start_custom_function_in_bt_thread((uint32_t)&remote, (uint32_t)NULL, (uint32_t)btif_opp_connect);
}

void app_bt_disconnect_opp_profile(struct btif_opp_channel_t *opp_channel)
{
    TRACE(2, "%s opp_channel=%p", __func__, opp_channel);
    app_bt_start_custom_function_in_bt_thread((uint32_t)opp_channel, (uint32_t)NULL, (uint32_t)btif_opp_disconnect);
}

void app_bt_opp_exchang_object(struct btif_opp_channel_t *opp_channel, const char *param)
{
    app_bt_call_func_in_bt_thread((uint32_t)opp_channel, (uint32_t)param,
                (uint32_t)NULL, (uint32_t)NULL, (uint32_t)btif_opp_exchang_object);
}

void app_bt_opp_pull_vcard_object(struct btif_opp_channel_t *opp_channel, const char *param)
{
    app_bt_call_func_in_bt_thread((uint32_t)opp_channel, (uint32_t)param, 
                (uint32_t)NULL, (uint32_t)NULL, (uint32_t)btif_opp_pull_business_card_object);
}

/**
 *First, add the parameters of the object to the push list and wait for sending,
 *If you need to send multiple objects, you need to add all the sent objects to 
 *the push list, and then send them. When multiple objects are sent together, you 
 *need to transmit the all header ID parameter. If a single fragment is transmitted,
 *you do not need the all header ID parameter, but only the body header ID
**/
bool app_bt_opp_add_to_push_object_init(struct btif_opp_channel_t *opp_channel)
{
    if (btif_opp_push_object_list_init(opp_channel))
    {
        return true;
    }
    return false;
}

bool app_bt_opp_add_push_object_count(struct btif_opp_channel_t *opp_channel, uint16 body_length)
{
    if (btif_opp_push_object_count(opp_channel, body_length))
    {
        return true;
    }
    return false;
}

bool app_bt_opp_add_to_push_object_list(struct btif_opp_channel_t *opp_channel, struct app_opp_object_info_t *obejct_info, bool is_final_put_req)
{
    if (!opp_channel)
    {
        TRACE(2, "%s:add fail, channel is not connected,opp_channel=%p", __func__, opp_channel);
        return false;
    }

    struct btif_opp_push_param_t opp_push_param = {0};
    TRACE(3, "%s:enter to object len=%d is_final_put_req=%d", __func__, obejct_info->body_length, is_final_put_req);

    if (obejct_info->object_name)
    {
        opp_push_param.name_is_unicode = true;
        opp_push_param.object_name = obejct_info->object_name;
        opp_push_param.name_length = obejct_info->name_length;
    }

    if (obejct_info->object_type)
    {
        opp_push_param.object_type = obejct_info->object_type;
    }

    if (obejct_info->app_parameters)
    {
        opp_push_param.app_parameters = obejct_info->app_parameters;
        opp_push_param.app_parameters_length = obejct_info->app_parameters_length;
    }

    if (obejct_info->body_content)
    {
        opp_push_param.body_content = obejct_info->body_content;
        opp_push_param.body_length = obejct_info->body_length;
    }
    opp_push_param.push_status = OPP_WAIT_PUSH;
    opp_push_param.is_final_put_req = is_final_put_req;
    app_bt_opp_add_push_object_count(opp_channel, obejct_info->body_length);
    /*****************used deal upper fragment push object,here is not used set defual value*******************/
    TRACE(2, "%s:add to object len=%d is_final_put_req=%d", __func__, obejct_info->body_length, is_final_put_req);

    return btif_opp_add_to_push_object_list(opp_channel, &opp_push_param);
}

void app_bt_opp_push_object(struct btif_opp_channel_t *opp_channel, bool once_multiple)
{
    if (!opp_channel)
    {
        TRACE(2, "%s:push fail, opp_channel=%p", __func__, opp_channel);
        return;
    }
    if (btif_opp_is_connected(opp_channel))
    {
        btif_opp_push_object(opp_channel, once_multiple);
    }
    else
    {
        TRACE(1, "%s:fail channel not is connected!", __func__);
    }
}

void app_bt_opp_send_pull_rsp_object(struct btif_opp_channel_t *opp_channel, struct app_opp_object_info_t *obejct_info)
{
    if (!opp_channel)
    {
        TRACE(2, "%s:push fail, opp_channel=%p", __func__, opp_channel);
        return;
    }
    if (btif_opp_is_connected(opp_channel))
    {
        btif_opp_pull_rsp_object(opp_channel, (void *)obejct_info);
    }
    else
    {
        TRACE(1, "%s:fail channel not is connected!", __func__);
    }
}

void app_bt_opp_set_srm_in_wait(struct btif_opp_channel_t *opp_channel, bool wait_flag)
{
    btif_opp_set_srm_in_wait(opp_channel, wait_flag);
}

void app_bt_opp_send_obex_disc_req(struct btif_opp_channel_t *opp_channel)
{
    TRACE(2, "%s opp_channel=%p", __func__, opp_channel);
    app_bt_start_custom_function_in_bt_thread((uint32_t)opp_channel, (uint32_t)NULL, (uint32_t)btif_opp_send_obex_disconnect_req);
}

void app_bt_opp_send_obex_abort_req(struct btif_opp_channel_t *opp_channel)
{
    TRACE(2, "%s opp_channel=%p", __func__, opp_channel);
    app_bt_start_custom_function_in_bt_thread((uint32_t)opp_channel, (uint32_t)NULL, (uint32_t)btif_opp_send_abort_req);
}

static struct btif_opp_channel_t *app_bt_opp_get_channel(void)
{
    struct BT_DEVICE_T *curr_device = NULL;
    curr_device = app_bt_get_device(BT_DEVICE_ID_1);
    TRACE(3, "%s curr_device = %p  opp_channel = %p", __func__, curr_device, curr_device->opp_channel);
    return curr_device->opp_channel;
}

static void app_bt_pts_opp_create_channel(const char* param, uint32_t len)
{
    app_bt_connect_opp_profile(app_bt_get_pts_address());
}

static void app_bt_pts_opp_disconnect_channel(const char* param, uint32_t len)
{
    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    app_bt_disconnect_opp_profile(opp_channel);
}

static void app_bt_pts_opp_pull_vcard_object(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_pull_vcard_object))
    {
        return;
    }
    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    btif_opp_pull_business_card_object(opp_channel, NULL, NULL);
}

static void app_bt_pts_opp_srm_wait_pull_vcard_object(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_pull_vcard_object))
    {
        return;
    }
    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    app_bt_opp_set_srm_in_wait(opp_channel, true);
    btif_opp_pull_business_card_object(opp_channel, NULL, NULL);
}

static void app_bt_pts_opp_push_vcard_object(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_push_vcard_object))
    {
        return;
    }
    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    struct app_opp_object_info_t obejct_info = {0};
    if (app_bt_opp_add_to_push_object_init(opp_channel) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_init fail!", __func__);
        return;
    }

    obejct_info.object_name = OPP_PUSH_VCARD1_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VCARD1_NAME);
    obejct_info.object_type = OPP_PUSH_VCARD_TYPE;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_vcard1_content[0];
    obejct_info.body_length = sizeof(pts_vcard1_content);

    if (app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, true) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_list fail!", __func__);
        return;
    }

    app_bt_opp_push_object(opp_channel, false);
}

static void app_bt_pts_opp_push_vcard_object2(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_push_vcard_object2))
    {
        return;
    }
    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    struct app_opp_object_info_t obejct_info = {0};
    if (app_bt_opp_add_to_push_object_init(opp_channel) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_init fail!", __func__);
        return;
    }

    obejct_info.object_name = OPP_PUSH_VCARD2_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VCARD2_NAME);
    obejct_info.object_type = OPP_PUSH_VCARD_TYPE;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_vcard1_content[0];
    obejct_info.body_length = sizeof(pts_vcard1_content);

    if (app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, true) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_list fail!", __func__);
        return;
    }

    app_bt_opp_push_object(opp_channel, false);
}

static void app_bt_pts_opp_push_two_vcard_on_single_put_operation(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_push_two_vcard_on_single_put_operation))
    {
        return;
    }

    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    struct app_opp_object_info_t obejct_info = {0};
    TRACE(1, "%s", __func__);
    if (app_bt_opp_add_to_push_object_init(opp_channel) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_init fail!", __func__);
        return;
    }
    obejct_info.object_name = OPP_PUSH_VCARD1_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VCARD1_NAME);
    obejct_info.object_type = OPP_PUSH_VCARD_TYPE;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_vcard1_content[0];
    obejct_info.body_length = sizeof(pts_vcard1_content);

    if (app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, false) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_list fail!", __func__);
        return;
    }
    pts_test_status = OPP_PTS_PUSH_TWO_VCARD_OBJECT_START;
    app_bt_opp_push_object(opp_channel, false);
}

static void app_bt_pts_opp_push_two_vcard_on_single_put_operation_countinue(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_push_two_vcard_on_single_put_operation_countinue))
    {
        return;
    }

    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    struct app_opp_object_info_t obejct_info = {0};
    obejct_info.object_name = OPP_PUSH_VCARD2_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VCARD2_NAME);
    obejct_info.object_type = OPP_PUSH_VCARD_TYPE;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_vcard1_content[0];
    obejct_info.body_length = sizeof(pts_vcard1_content);
    if (app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, true) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_list fail!", __func__);
        return;
    }
    pts_test_status = OPP_PTS_PUSH_TWO_VCARD_OBJECT_END;
    app_bt_opp_push_object(opp_channel, false);
}

static void app_bt_pts_opp_push_both_vcard_on_once_put(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_push_both_vcard_on_once_put))
    {
        return;
    }

    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    struct app_opp_object_info_t obejct_info = {0};
    TRACE(1, "%s", __func__);
    if (app_bt_opp_add_to_push_object_init(opp_channel) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_init fail!", __func__);
        return;
    }
    obejct_info.object_name = OPP_PUSH_VCARD1_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VCARD1_NAME);
    obejct_info.object_type = OPP_PUSH_VCARD_TYPE;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_vcard1_content[0];
    obejct_info.body_length = sizeof(pts_vcard1_content);
    app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, false);

    obejct_info.object_name = OPP_PUSH_VCARD2_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VCARD2_NAME);
    obejct_info.object_type = OPP_PUSH_VCARD_TYPE;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_vcard1_content[0];
    obejct_info.body_length = sizeof(pts_vcard1_content);
    app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, true);
    app_bt_opp_push_object(opp_channel, true);
}

static void app_bt_pts_opp_push_vcal_object(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_push_vcal_object))
    {
        return;
    }
    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    struct app_opp_object_info_t obejct_info = {0};
    if (app_bt_opp_add_to_push_object_init(opp_channel) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_init fail!", __func__);
        return;
    }

    obejct_info.object_name = OPP_PUSH_VCAL1_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VCAL1_NAME);
    obejct_info.object_type = NULL;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_vcal1_content[0];
    obejct_info.body_length = sizeof(pts_vcal1_content);

    if (app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, true) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_list fail!", __func__);
        return;
    }

    app_bt_opp_push_object(opp_channel, false);
}

static void app_bt_pts_opp_push_vcal_object2(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_push_vcal_object2))
    {
        return;
    }
    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    struct app_opp_object_info_t obejct_info = {0};
    if (app_bt_opp_add_to_push_object_init(opp_channel) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_init fail!", __func__);
        return;
    }

    obejct_info.object_name = OPP_PUSH_VCAL2_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VCAL2_NAME);
    obejct_info.object_type = NULL;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_vcal1_content[0];
    obejct_info.body_length = sizeof(pts_vcal1_content);

    if (app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, true) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_list fail!", __func__);
        return;
    }

    app_bt_opp_push_object(opp_channel, false);
}

static void app_bt_pts_opp_push_two_vcal_on_single_put_operation(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_push_two_vcal_on_single_put_operation))
    {
        return;
    }

    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    struct app_opp_object_info_t obejct_info = {0};
    TRACE(1, "%s", __func__);
    if (app_bt_opp_add_to_push_object_init(opp_channel) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_init fail!", __func__);
        return;
    }
    obejct_info.object_name = OPP_PUSH_VCAL1_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VCAL1_NAME);
    obejct_info.object_type = OPP_PUSH_VCARD_TYPE;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_vcal1_content[0];
    obejct_info.body_length = sizeof(pts_vcal1_content);

    if (app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, false) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_list fail!", __func__);
        return;
    }
    pts_test_status = OPP_PTS_PUSH_TWO_VCAL_OBJECT_START;
    app_bt_opp_push_object(opp_channel, false);
}

static void app_bt_pts_opp_push_two_vcal_on_single_put_operation_countinue(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_push_two_vcal_on_single_put_operation_countinue))
    {
        return;
    }

    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    struct app_opp_object_info_t obejct_info = {0};
    obejct_info.object_name = OPP_PUSH_VCAL2_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VCAL2_NAME);
    obejct_info.object_type = OPP_PUSH_VCARD_TYPE;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_vcal1_content[0];
    obejct_info.body_length = sizeof(pts_vcal1_content);
    if (app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, true) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_list fail!", __func__);
        return;
    }
    pts_test_status = OPP_PTS_PUSH_TWO_VCAL_OBJECT_END;
    app_bt_opp_push_object(opp_channel, false);
}

static void app_bt_pts_opp_push_both_vcal_on_once_object(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_push_both_vcal_on_once_object))
    {
        return;
    }

    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    struct app_opp_object_info_t obejct_info = {0};
    TRACE(1, "%s", __func__);
    if (app_bt_opp_add_to_push_object_init(opp_channel) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_init fail!", __func__);
        return;
    }
    obejct_info.object_name = OPP_PUSH_VCAL1_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VCAL1_NAME);
    obejct_info.object_type = OPP_PUSH_VCARD_TYPE;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_vcal1_content[0];
    obejct_info.body_length = sizeof(pts_vcal1_content);
    app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, false);

    obejct_info.object_name = OPP_PUSH_VCAL2_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VCAL2_NAME);
    obejct_info.object_type = OPP_PUSH_VCARD_TYPE;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_vcal1_content[0];
    obejct_info.body_length = sizeof(pts_vcal1_content);
    app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, true);
    app_bt_opp_push_object(opp_channel, true);
}

static void app_bt_pts_opp_push_vmsg_object(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_push_vmsg_object))
    {
        return;
    }
    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    struct app_opp_object_info_t obejct_info = {0};
    if (app_bt_opp_add_to_push_object_init(opp_channel) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_init fail!", __func__);
        return;
    }

    obejct_info.object_name = OPP_PUSH_VMSG1_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VMSG1_NAME);
    obejct_info.object_type = NULL;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_msg1_content[0];
    obejct_info.body_length = sizeof(pts_msg1_content);

    if (app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, true) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_list fail!", __func__);
        return;
    }

    app_bt_opp_push_object(opp_channel, false);
}

static void app_bt_pts_opp_push_vmsg_object2(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_push_vmsg_object2))
    {
        return;
    }
    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    struct app_opp_object_info_t obejct_info = {0};
    if (app_bt_opp_add_to_push_object_init(opp_channel) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_init fail!", __func__);
        return;
    }

    obejct_info.object_name = OPP_PUSH_VMSG2_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VMSG2_NAME);
    obejct_info.object_type = NULL;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_msg1_content[0];
    obejct_info.body_length = sizeof(pts_msg1_content);

    if (app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, true) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_list fail!", __func__);
        return;
    }

    app_bt_opp_push_object(opp_channel, false);
}

static void app_bt_pts_opp_push_two_vmsg_on_single_put_operation(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_push_two_vmsg_on_single_put_operation))
    {
        return;
    }

    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    struct app_opp_object_info_t obejct_info = {0};
    TRACE(1, "%s", __func__);
    if (app_bt_opp_add_to_push_object_init(opp_channel) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_init fail!", __func__);
        return;
    }
    obejct_info.object_name = OPP_PUSH_VMSG1_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VMSG1_NAME);
    obejct_info.object_type = OPP_PUSH_VCARD_TYPE;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_msg1_content[0];
    obejct_info.body_length = sizeof(pts_msg1_content);

    if (app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, false) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_list fail!", __func__);
        return;
    }
    pts_test_status = OPP_PTS_PUSH_TWO_VMSG_OBJECT_START;
    app_bt_opp_push_object(opp_channel, false);
}

static void app_bt_pts_opp_push_two_vmsg_on_single_put_operation_countinue(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_push_two_vmsg_on_single_put_operation_countinue))
    {
        return;
    }

    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    struct app_opp_object_info_t obejct_info = {0};
    obejct_info.object_name = OPP_PUSH_VMSG2_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VMSG2_NAME);
    obejct_info.object_type = OPP_PUSH_VCARD_TYPE;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_msg1_content[0];
    obejct_info.body_length = sizeof(pts_msg1_content);
    if (app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, true) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_list fail!", __func__);
        return;
    }
    pts_test_status = OPP_PTS_PUSH_TWO_VMSG_OBJECT_END;
    app_bt_opp_push_object(opp_channel, false);
}

static void app_bt_pts_opp_push_both_vmsg_on_once_object(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_push_both_vmsg_on_once_object))
    {
        return;
    }

    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    struct app_opp_object_info_t obejct_info = {0};
    TRACE(1, "%s", __func__);
    if (app_bt_opp_add_to_push_object_init(opp_channel) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_init fail!", __func__);
        return;
    }
    obejct_info.object_name = OPP_PUSH_VMSG1_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VMSG1_NAME);
    obejct_info.object_type = OPP_PUSH_VCARD_TYPE;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_msg1_content[0];
    obejct_info.body_length = sizeof(pts_msg1_content);
    app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, false);

    obejct_info.object_name = OPP_PUSH_VMSG2_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VMSG2_NAME);
    obejct_info.object_type = OPP_PUSH_VCARD_TYPE;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_msg1_content[0];
    obejct_info.body_length = sizeof(pts_msg1_content);
    app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, true);
    app_bt_opp_push_object(opp_channel, true);
}

static void app_bt_pts_opp_push_vnt_object(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_push_vnt_object))
    {
        return;
    }
    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    struct app_opp_object_info_t obejct_info = {0};
    if (app_bt_opp_add_to_push_object_init(opp_channel) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_init fail!", __func__);
        return;
    }

    obejct_info.object_name = OPP_PUSH_VNT1_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VNT1_NAME);
    obejct_info.object_type = NULL;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_note1_content[0];
    obejct_info.body_length = sizeof(pts_note1_content);

    if (app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, true) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_list fail!", __func__);
        return;
    }

    app_bt_opp_push_object(opp_channel, false);
}

static void app_bt_pts_opp_push_vnt_object2(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_push_vnt_object2))
    {
        return;
    }
    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    struct app_opp_object_info_t obejct_info = {0};
    if (app_bt_opp_add_to_push_object_init(opp_channel) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_init fail!", __func__);
        return;
    }

    obejct_info.object_name = OPP_PUSH_VNT2_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VNT2_NAME);
    obejct_info.object_type = NULL;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_note1_content[0];
    obejct_info.body_length = sizeof(pts_note1_content);

    if (app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, true) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_list fail!", __func__);
        return;
    }

    app_bt_opp_push_object(opp_channel, false);
}

static void app_bt_pts_opp_push_two_vnt_on_single_put_operation(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_push_two_vnt_on_single_put_operation))
    {
        return;
    }

    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    struct app_opp_object_info_t obejct_info = {0};
    TRACE(1, "%s", __func__);
    if (app_bt_opp_add_to_push_object_init(opp_channel) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_init fail!", __func__);
        return;
    }
    obejct_info.object_name = OPP_PUSH_VNT1_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VNT1_NAME);
    obejct_info.object_type = OPP_PUSH_VCARD_TYPE;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_note1_content[0];
    obejct_info.body_length = sizeof(pts_note1_content);

    if (app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, false) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_list fail!", __func__);
        return;
    }
    pts_test_status = OPP_PTS_PUSH_TWO_VNT_OBJECT_START;
    app_bt_opp_push_object(opp_channel, false);
}

static void app_bt_pts_opp_push_two_vnt_on_single_put_operation_countinue(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_push_two_vnt_on_single_put_operation_countinue))
    {
        return;
    }

    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    struct app_opp_object_info_t obejct_info = {0};
    obejct_info.object_name = OPP_PUSH_VNT2_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VNT2_NAME);
    obejct_info.object_type = OPP_PUSH_VCARD_TYPE;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_note1_content[0];
    obejct_info.body_length = sizeof(pts_note1_content);
    if (app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, true) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_list fail!", __func__);
        return;
    }
    pts_test_status = OPP_PTS_PUSH_TWO_VNT_OBJECT_END;
    app_bt_opp_push_object(opp_channel, false);
}

static void app_bt_pts_opp_push_both_vnt_on_once_object(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_push_both_vnt_on_once_object))
    {
        return;
    }

    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    struct app_opp_object_info_t obejct_info = {0};
    TRACE(1, "%s", __func__);
    if (app_bt_opp_add_to_push_object_init(opp_channel) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_init fail!", __func__);
        return;
    }
    obejct_info.object_name = OPP_PUSH_VNT1_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VNT1_NAME);
    obejct_info.object_type = OPP_PUSH_VCARD_TYPE;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_note1_content[0];
    obejct_info.body_length = sizeof(pts_note1_content);
    app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, false);

    obejct_info.object_name = OPP_PUSH_VNT2_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VNT2_NAME);
    obejct_info.object_type = OPP_PUSH_VCARD_TYPE;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_note1_content[0];
    obejct_info.body_length = sizeof(pts_note1_content);
    app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, true);
    app_bt_opp_push_object(opp_channel, true);
}

static void app_bt_pts_opp_push_1kfile_start_object(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_push_1kfile_start_object))
    {
        return;
    }
    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    struct app_opp_object_info_t obejct_info = {0};
    TRACE(1, "%s", __func__);
    if (app_bt_opp_add_to_push_object_init(opp_channel) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_init fail!", __func__);
        return;
    }

    obejct_info.object_name = OPP_PUSH_VCARD1_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VCARD1_NAME);
    obejct_info.object_type = OPP_PUSH_VCARD_TYPE;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_large_file_content[0];
    obejct_info.body_length =  1024;

    if (app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, true) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_list fail!", __func__);
        return;
    }

    app_bt_opp_push_object(opp_channel, false);
}

static void app_bt_pts_opp_push_unsupport_object(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_push_unsupport_object))
    {
        return;
    }
    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    struct app_opp_object_info_t obejct_info = {0};
    TRACE(1, "%s", __func__);
    if (app_bt_opp_add_to_push_object_init(opp_channel) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_init fail!", __func__);
        return;
    }

    obejct_info.object_name = OPP_PUSH_UNSUPPORT_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_UNSUPPORT_NAME);
    obejct_info.object_type = OPP_PUSH_VCARD_TYPE;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_note1_content[0];
    obejct_info.body_length =  sizeof(pts_note1_content);

    if (app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, true) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_list fail!", __func__);
        return;
    }

    app_bt_opp_push_object(opp_channel, false);

}

static void app_bt_pts_opp_object_status_mgr(const char* param, uint32_t len)
{
    switch (pts_test_status)
    {
        case OPP_PTS_PUSH_TWO_VCARD_OBJECT_START:
            app_bt_pts_opp_push_two_vcard_on_single_put_operation_countinue();
            break;

        case OPP_PTS_PUSH_TWO_VCAL_OBJECT_START:
            app_bt_pts_opp_push_two_vcal_on_single_put_operation_countinue();
            break;

        case OPP_PTS_PUSH_TWO_VMSG_OBJECT_START:
            app_bt_pts_opp_push_two_vmsg_on_single_put_operation_countinue();
            break;

        case OPP_PTS_PUSH_TWO_VNT_OBJECT_START:
            app_bt_pts_opp_push_two_vnt_on_single_put_operation_countinue();
            break;

        case OPP_PTS_PUSH_TWO_VCARD_OBJECT_END:
        case OPP_PTS_PUSH_TWO_VCAL_OBJECT_END:
        case OPP_PTS_PUSH_TWO_VMSG_OBJECT_END:
        case OPP_PTS_PUSH_TWO_VNT_OBJECT_END:
        case OPP_PTS_IDLE:
            pts_test_status = OPP_PTS_IDLE;
            break;
        default:
            TRACE(3," %s invalid pts_test_status %d", __func__, pts_test_status);
            break;
    }
}

static void app_bt_pts_opp_send_obex_disc_req(const char* param, uint32_t len)
{
    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();

    app_bt_opp_send_obex_disc_req(opp_channel);
}

static void app_bt_pts_opp_send_obex_abort_req(const char* param, uint32_t len)
{
    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();

    app_bt_opp_send_obex_abort_req(opp_channel);
}

static void app_bt_pts_opp_exchange_vcard_object(const char* param, uint32_t len)
{
    if (bt_defer_curr_func_0(app_bt_pts_opp_exchange_vcard_object))
    {
        return;
    }

    struct btif_opp_channel_t *opp_channel = app_bt_opp_get_channel();
    struct app_opp_object_info_t obejct_info = {0};
    TRACE(1, "%s", __func__);
    if (app_bt_opp_add_to_push_object_init(opp_channel) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_init fail!", __func__);
        return;
    }

    obejct_info.object_name = OPP_PUSH_VCARD1_NAME;
    obejct_info.name_length = sizeof(OPP_PUSH_VCARD1_NAME);
    obejct_info.object_type = OPP_PUSH_VCARD_TYPE;
    obejct_info.app_parameters = NULL;
    obejct_info.app_parameters_length = 0;
    obejct_info.body_content = (const uint8 *) &pts_vcard1_content[0];
    obejct_info.body_length =  sizeof(pts_vcard1_content);

    if (app_bt_opp_add_to_push_object_list(opp_channel, &obejct_info, true) == false)
    {
        TRACE(1, "%s app_bt_opp_add_to_push_object_list fail!", __func__);
        return;
    }
    app_bt_opp_exchang_object(opp_channel, NULL);
}

static void app_bt_pts_opp_send_pull_reject_rsp(const char* param, uint32_t len)
{

}

static app_bt_host_cmd_table_t app_opp_test_cmd_table[] =
{
    {"opp_connect",             app_bt_pts_opp_create_channel},
    {"op_disconnect",           app_bt_pts_opp_disconnect_channel},
    {"op_pull_vcard",           app_bt_pts_opp_pull_vcard_object},
    {"op_pull_srm_wait_vcard",  app_bt_pts_opp_srm_wait_pull_vcard_object},
    {"op_push_vcard",           app_bt_pts_opp_push_vcard_object},
    {"op_push_vcard2",          app_bt_pts_opp_push_vcard_object2},
    {"op_push_2vcard_1put",     app_bt_pts_opp_push_two_vcard_on_single_put_operation},
    {"op_push_1kfile",          app_bt_pts_opp_push_1kfile_start_object},
    {"op_push_vcal",            app_bt_pts_opp_push_vcal_object},
    {"op_push_vcal2",           app_bt_pts_opp_push_vcal_object2},
    {"op_push_2vcal_1put",      app_bt_pts_opp_push_two_vcal_on_single_put_operation},
    {"op_push_vmsg",            app_bt_pts_opp_push_vmsg_object},
    {"op_push_vmsg2",           app_bt_pts_opp_push_vmsg_object2},
    {"op_push_2vmsg_1put",      app_bt_pts_opp_push_two_vmsg_on_single_put_operation},
    {"op_push_vnt",             app_bt_pts_opp_push_vnt_object},
    {"op_push_vnt2",            app_bt_pts_opp_push_vnt_object2},
    {"op_push_2vnt_1put",       app_bt_pts_opp_push_two_vnt_on_single_put_operation},
    {"op_push_unsupport_object",app_bt_pts_opp_push_unsupport_object},
    {"op_obex_abort",           app_bt_pts_opp_send_obex_abort_req},
    {"op_obex_disc",            app_bt_pts_opp_send_obex_disc_req},
    {"op_push_both_vcard_1put", app_bt_pts_opp_push_both_vcard_on_once_put},
    {"op_push_both_vcal",       app_bt_pts_opp_push_both_vcal_on_once_object},
    {"op_push_both_vmsg",       app_bt_pts_opp_push_both_vmsg_on_once_object},
    {"op_push_both_vnt",        app_bt_pts_opp_push_both_vnt_on_once_object},
    {"op_exchange_vcard",       app_bt_pts_opp_exchange_vcard_object},
    {"op_pull_reject",          app_bt_pts_opp_send_pull_reject_rsp},
};

void app_bt_opp_init(void)
{
    app_bt_opp_client_init();
    app_bt_opp_server_init();

    app_bt_host_add_cmd_table(sizeof(app_opp_test_cmd_table)/sizeof(app_opp_test_cmd_table[0]),
        app_opp_test_cmd_table);
}

#endif /* BT_OPP_SUPPORT */

