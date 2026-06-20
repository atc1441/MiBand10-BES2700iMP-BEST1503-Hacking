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
#undef MOUDLE
#define MOUDLE APP_BT
#include "cmsis_os.h"
#include "string.h"
#include "hal_trace.h"
#include "bluetooth.h"
#include "besbt.h"
#include "app_bt_func.h"
#include "hfp_api.h"
#include "sco_api.h"
#include "app_bt.h"
#include "bt_if.h"
#include "bes_me_api.h"

#ifdef __IAG_BLE_INCLUDE__
#include "bluetooth_ble_api.h"
#endif

static const char * const app_bt_func_table_str[] =
{
    "Me_switch_sco_req",
    "ME_SwitchRole_req",
    "MeDisconnectLink_req",
    "ME_StopSniff_req",
    "ME_SetAccessibleMode_req",
    "Me_SetLinkPolicy_req",
    "CMGR_SetSniffTimer_req",
    "CMGR_SetSniffInofToAllHandlerByRemDev_req",
    "A2DP_OpenStream_req",
    "A2DP_CloseStream_req",
    "HF_CreateServiceLink_req",
    "HF_DisconnectServiceLink_req",
    "HF_CreateAudioLink_req",
    "HF_DisconnectAudioLink_req",
    "BT_Control_SleepMode_req",
    "BT_Custom_Func_req",
    "BT_Thread_Defer_Func_req",
    "ME_StartSniff_req",
    "DIP_QuryService_req",
    "A2DP_Force_OpenStream_req",
    "HF_Force_CreateServiceLink_req",
    "BT_Red_Ccmp_Client_Open",
    "BT_Set_Access_Mode_Test",
    "BT_Set_Adv_Mode_Test",
    "Write_Controller_Memory_Test",
    "Read_Controller_Memory_Test",
    "GATT_Connect_Req",
    "GATT_Disconnect_Req",
};

#define PENDING_SET_LINKPOLICY_REQ_BUF_CNT  5
static BT_SET_LINKPOLICY_REQ_T pending_set_linkpolicy_req[PENDING_SET_LINKPOLICY_REQ_BUF_CNT];

static uint8_t pending_set_linkpolicy_in_cursor = 0;
static uint8_t pending_set_linkpolicy_out_cursor = 0;

static void app_bt_print_pending_set_linkpolicy_req(void)
{
    TRACE(0,"Pending set link policy requests:");
    uint8_t index = pending_set_linkpolicy_out_cursor;
    while (index != pending_set_linkpolicy_in_cursor)
    {
        TRACE(3,"index %d conn_handle %04x LinkPolicy %d", index,
            pending_set_linkpolicy_req[index].conn_handle,
            pending_set_linkpolicy_req[index].policy);
        index++;
        if (PENDING_SET_LINKPOLICY_REQ_BUF_CNT == index)
        {
            index = 0;
        }
    }
}

static void app_bt_push_pending_set_linkpolicy(uint16_t conn_handle, btif_link_policy_t policy)
{
    // go through the existing pending list to see if the remDev is already in
    uint8_t index = pending_set_linkpolicy_out_cursor;
    while (index != pending_set_linkpolicy_in_cursor)
    {
        if (conn_handle == pending_set_linkpolicy_req[index].conn_handle)
        {
            pending_set_linkpolicy_req[index].policy = policy;
            return;
        }
        index++;
        if (PENDING_SET_LINKPOLICY_REQ_BUF_CNT == index)
        {
            index = 0;
        }
    }

    pending_set_linkpolicy_req[pending_set_linkpolicy_in_cursor].conn_handle = conn_handle;
    pending_set_linkpolicy_req[pending_set_linkpolicy_in_cursor].policy = policy;
    pending_set_linkpolicy_in_cursor++;
    if (PENDING_SET_LINKPOLICY_REQ_BUF_CNT == pending_set_linkpolicy_in_cursor)
    {
        pending_set_linkpolicy_in_cursor = 0;
    }

    app_bt_print_pending_set_linkpolicy_req();
}

BT_SET_LINKPOLICY_REQ_T* app_bt_pop_pending_set_linkpolicy(void)
{
    if (pending_set_linkpolicy_out_cursor == pending_set_linkpolicy_in_cursor)
    {
        return NULL;
    }

    BT_SET_LINKPOLICY_REQ_T* ptReq = &pending_set_linkpolicy_req[pending_set_linkpolicy_out_cursor];
    pending_set_linkpolicy_out_cursor++;
    if (PENDING_SET_LINKPOLICY_REQ_BUF_CNT == pending_set_linkpolicy_out_cursor)
    {
        pending_set_linkpolicy_out_cursor = 0;
    }

    app_bt_print_pending_set_linkpolicy_req();
    return ptReq;
}

void app_bt_set_linkpolicy(uint16_t conn_handle, btif_link_policy_t policy)
{
    btif_remote_device_t* rem_dev = app_bt_get_remote_dev_by_handle(conn_handle);
    if (rem_dev)
    {
        bt_status_t ret = btif_me_set_link_policy(rem_dev, policy);
        TRACE(3,"%s policy %d returns %d", __FUNCTION__, policy, ret);

        if (BT_STS_IN_PROGRESS == ret)
        {
            app_bt_push_pending_set_linkpolicy(conn_handle, policy);
        }
    }
}

#define COUNT_OF_PENDING_REMOTE_DEV_TO_EXIT_SNIFF_MODE  8
static uint16_t pendingRemoteDevToExitSniffMode[COUNT_OF_PENDING_REMOTE_DEV_TO_EXIT_SNIFF_MODE];
static uint8_t  maskOfRemoteDevPendingForExitingSniffMode = 0;
void app_check_pending_stop_sniff_op(void)
{
    if (maskOfRemoteDevPendingForExitingSniffMode > 0)
    {
        for (uint8_t index = 0;index < COUNT_OF_PENDING_REMOTE_DEV_TO_EXIT_SNIFF_MODE;index++)
        {
            if (maskOfRemoteDevPendingForExitingSniffMode & (1 << index))
            {
                uint16_t conn_handle = pendingRemoteDevToExitSniffMode[index];
                btif_remote_device_t* remdev = app_bt_get_remote_dev_by_handle(conn_handle);
                if (remdev) {
                    if (btif_me_get_current_mode(remdev) == BTIF_BLM_SNIFF_MODE){
                        TRACE(1,"!!! stop sniff currmode:%d\n",  btif_me_get_current_mode(remdev));
                        bt_status_t ret = btif_me_stop_sniff(conn_handle);
                        TRACE(1,"Return status %d", ret);
                        if (BT_STS_IN_PROGRESS != ret)
                        {
                            maskOfRemoteDevPendingForExitingSniffMode &= (~(1<<index));
                            break;
                        }
                    }
                }
            }
        }

        if (maskOfRemoteDevPendingForExitingSniffMode > 0)
        {
            btif_osapi_notify_evm();
        }
    }
}

static void app_add_pending_stop_sniff_op(uint16_t conn_handle)
{
    for (uint8_t index = 0;index < COUNT_OF_PENDING_REMOTE_DEV_TO_EXIT_SNIFF_MODE;index++)
    {
        if (maskOfRemoteDevPendingForExitingSniffMode & (1 << index))
        {
            if (pendingRemoteDevToExitSniffMode[index] == conn_handle)
            {
                return;
            }
        }
    }

    for (uint8_t index = 0;index < COUNT_OF_PENDING_REMOTE_DEV_TO_EXIT_SNIFF_MODE;index++)
    {
        if (0 == (maskOfRemoteDevPendingForExitingSniffMode & (1 << index)))
        {
            pendingRemoteDevToExitSniffMode[index] = conn_handle;
            maskOfRemoteDevPendingForExitingSniffMode |= (1 << index);
        }
    }
}

#ifdef FPGA
void app_start_ble_adv_for_test(void);
#endif

int app_bt_mail_process(APP_BT_MAIL* mail_p)
{
    bt_status_t status = BT_STS_FAILED;
    if (mail_p->request_id != CMGR_SetSniffTimer_req &&
        mail_p->request_id != BT_Custom_Func_req &&
        mail_p->request_id != BT_Thread_Defer_Func_req)
    {
        TRACE(3,"[BT_FUNC] src_thread:0x%08x call request_id=%x->:%s", mail_p->src_thread,
            mail_p->request_id, app_bt_func_table_str[mail_p->request_id]);
    }
    switch (mail_p->request_id) {
        case Me_switch_sco_req:
            status = btif_sco_tws_switch(mail_p->param.Me_switch_sco_param.scohandle);
            break;
        case ME_SwitchRole_req:
            status = btif_me_switch_role(mail_p->param.ME_SwitchRole_param.conn_handle);
            break;
        case MeDisconnectLink_req:
            status = btif_me_force_disconnect_link_with_reason(mail_p->param.MeDisconnectLink_param.conn_handle, BTIF_BEC_USER_TERMINATED, TRUE);
            break;
        case ME_StopSniff_req:
        {
            if (app_bt_get_remote_dev_by_handle(mail_p->param.ME_StopSniff_param.conn_handle))
            {
                status = btif_me_stop_sniff(mail_p->param.ME_StopSniff_param.conn_handle);
                if (BT_STS_IN_PROGRESS == status)
                {
                    app_add_pending_stop_sniff_op(mail_p->param.ME_StopSniff_param.conn_handle);
                }
            }
            break;
        }
        case ME_StartSniff_req:
        {
            if (app_bt_get_remote_dev_by_handle(mail_p->param.ME_StartSniff_param.conn_handle))
            {
                status = btif_me_start_sniff (mail_p->param.ME_StartSniff_param.conn_handle,
                    &(mail_p->param.ME_StartSniff_param.sniffInfo));
            }
            break;
        }
        case BT_Control_SleepMode_req:
        {
            bt_adapter_write_sleep_enable(mail_p->param.ME_BtControlSleepMode_param.isEnable);
            break;
        }
        case ME_SetAccessibleMode_req:

            app_bt_set_access_mode(mail_p->param.ME_SetAccessibleMode_param.mode);
#if !defined(IBRT)
        btif_me_write_scan_activity_specific(BTIF_HCC_WRITE_INQ_SCAN_ACTIVITY,
                                                BTIF_BT_DEFAULT_INQ_SCAN_INTERVAL,
                                                BTIF_BT_DEFAULT_INQ_SCAN_WINDOW);
        btif_me_write_scan_activity_specific(BTIF_HCC_WRITE_PAGE_SCAN_ACTIVITY,
                                                BTIF_BT_DEFAULT_PAGE_SCAN_INTERVAL,
                                                BTIF_BT_DEFAULT_PAGE_SCAN_WINDOW);
#endif
            break;
        case Me_SetLinkPolicy_req:
            app_bt_set_linkpolicy(mail_p->param.Me_SetLinkPolicy_param.conn_handle,
                                      mail_p->param.Me_SetLinkPolicy_param.policy);
            break;
        case CMGR_SetSniffTimer_req:
        {
            btif_sniff_timer_t timer_mgr = {0,};
            timer_mgr.updata_sniff_timer = true;
            timer_mgr.timeout = mail_p->param.CMGR_SetSniffTimer_param.Time;
            if (mail_p->param.CMGR_SetSniffTimer_param.SniffInfo.maxInterval == 0){
                status = btif_cmgr_set_sniff_timer(mail_p->param.CMGR_SetSniffTimer_param.Handler,
                                            NULL,
                                            (void *)&timer_mgr);
            }else{
                status = btif_cmgr_set_sniff_timer(mail_p->param.CMGR_SetSniffTimer_param.Handler,
                                            &mail_p->param.CMGR_SetSniffTimer_param.SniffInfo,
                                            (void *)&timer_mgr);
            }
        }
            break;
        case CMGR_SetSniffInofToAllHandlerByRemDev_req:
            status = btif_cmgr_set_sniff_info_by_handle(mail_p->param.CMGR_SetSniffInofToAllHandlerByRemDev_param.conn_handle,
                &mail_p->param.CMGR_SetSniffInofToAllHandlerByRemDev_param.SniffInfo);
            break;
#ifdef BT_A2DP_SUPPORT
        case A2DP_OpenStream_req:
            status = btif_a2dp_open_stream((btif_avdtp_codec_t*)mail_p->param.A2DP_OpenStream_param.Stream,
                                        &mail_p->param.A2DP_OpenStream_param.Addr);
#if !defined(IBRT)
            if (!app_bt_is_acl_connected_byaddr(&mail_p->param.A2DP_OpenStream_param.Addr))
            {
                if ((BT_STS_NO_RESOURCES == status) || (BT_STS_IN_PROGRESS == status))
                {
                    app_bt_set_access_mode(BTIF_BAM_CONNECTABLE_ONLY);
                }
                else
                {
                    app_bt_set_access_mode(BTIF_BAM_NOT_ACCESSIBLE);
                }
            }
#endif
            break;
        case A2DP_CloseStream_req:
            status = btif_a2dp_close_stream(mail_p->param.A2DP_CloseStream_param.Stream);
            break;
#endif /* BT_A2DP_SUPPORT */
#ifdef BT_HFP_SUPPORT
        case HF_CreateServiceLink_req:
            status = btif_hf_create_service_link(&mail_p->param.HF_CreateServiceLink_param.Addr);
#if !defined(IBRT)
            if (!app_bt_is_acl_connected_byaddr(&mail_p->param.HF_CreateServiceLink_param.Addr))
            {
                if ((BT_STS_NO_RESOURCES == status) || (BT_STS_IN_PROGRESS == status))
                {
                    app_bt_set_access_mode(BTIF_BAM_CONNECTABLE_ONLY);
                }
                else
                {
                    app_bt_set_access_mode(BTIF_BAM_NOT_ACCESSIBLE);
                }
            }
#endif
            break;
        case HF_DisconnectServiceLink_req:
            status = btif_hf_disconnect_service_link(mail_p->param.HF_DisconnectServiceLink_param.Chan);
            break;
        case HF_CreateAudioLink_req:
            status = btif_hf_create_audio_link(mail_p->param.HF_CreateAudioLink_param.Chan);
            break;
        case HF_DisconnectAudioLink_req:
            status = btif_hf_disc_audio_link(mail_p->param.HF_DisconnectAudioLink_param.Chan);
            break;
#endif /* BT_HFP_SUPPORT */
#ifdef BT_DIP_SUPPORT
        case DIP_QuryService_req:
            status = btif_dip_query_for_service(mail_p->param.DIP_QuryService_param.conn_handle,
                mail_p->param.DIP_QuryService_param.dip_client);
            break;
#endif
#ifdef __GATT_OVER_BR_EDR__
        case GATT_Connect_Req:
        {
            uint8_t device_id = app_bt_get_device_id_byaddr(&(mail_p->param.GATT_param.Addr));
            bes_ble_gatt_cli_create_bearer(device_id);
            break;
        }
        case GATT_Disconnect_Req:
        {
            uint8_t device_id = app_bt_get_device_id_byaddr(&(mail_p->param.GATT_param.Addr));
            bes_ble_gatt_cli_disconnect_bearer(device_id);
            break;
        }
#endif
        default:
            break;
    }

    if (mail_p->request_id != CMGR_SetSniffTimer_req &&
        mail_p->request_id != BT_Custom_Func_req &&
        mail_p->request_id != BT_Thread_Defer_Func_req)
    {
        TRACE(2,"[BT_FUNC] exit request_id:%d :status:%d", mail_p->request_id, status);
    }
    return 0;
}

int app_bt_Me_switch_sco(uint16_t  scohandle)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = Me_switch_sco_req;
    mail->param.Me_switch_sco_param.scohandle = scohandle;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_ME_SwitchRole(uint16_t conn_handle)
{
#if !defined(IBRT) && !defined(BT_DISABLE_INITIAL_ROLE_SWITCH)
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = ME_SwitchRole_req;
    mail->param.ME_SwitchRole_param.conn_handle = conn_handle;
    app_bt_mail_send(mail);
#endif
    return 0;
}

int app_bt_MeDisconnectLink(uint16_t conn_handle)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = MeDisconnectLink_req;
    mail->param.MeDisconnectLink_param.conn_handle = conn_handle;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_ME_StopSniff(uint16_t conn_handle)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = ME_StopSniff_req;
    mail->param.ME_StopSniff_param.conn_handle = conn_handle;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_ME_StartSniff(uint16_t conn_handle, btif_sniff_info_t* sniffInfo)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = ME_StartSniff_req;
    mail->param.ME_StartSniff_param.conn_handle = conn_handle;
    mail->param.ME_StartSniff_param.sniffInfo = *sniffInfo;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_ME_ControlSleepMode(bool isEnabled)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = BT_Control_SleepMode_req;
    mail->param.ME_BtControlSleepMode_param.isEnable = isEnabled;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_ME_SetAccessibleMode(btif_accessible_mode_t mode)
{
#if defined(BLE_ONLY_ENABLED)
    return 0;
#endif

    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = ME_SetAccessibleMode_req;
    mail->param.ME_SetAccessibleMode_param.mode = mode;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_Me_SetLinkPolicy(uint16_t conn_handle, btif_link_policy_t policy)
{
#if !defined(IBRT)
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = Me_SetLinkPolicy_req;
    mail->param.Me_SetLinkPolicy_param.conn_handle = conn_handle;
    mail->param.Me_SetLinkPolicy_param.policy = policy;
    app_bt_mail_send(mail);
#endif
    return 0;
}

int app_bt_CMGR_SetSniffTimer(   btif_cmgr_handler_t *Handler,
                                                btif_sniff_info_t* SniffInfo,
                                                TimeT Time)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = CMGR_SetSniffTimer_req;
    mail->param.CMGR_SetSniffTimer_param.Handler = Handler;
    if (SniffInfo){
        memcpy(&mail->param.CMGR_SetSniffTimer_param.SniffInfo, SniffInfo, sizeof(btif_sniff_info_t));
    }else{
        memset(&mail->param.CMGR_SetSniffTimer_param.SniffInfo, 0, sizeof(btif_sniff_info_t));
    }
    mail->param.CMGR_SetSniffTimer_param.Time = Time;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_CMGR_SetSniffInfoToAllHandlerByRemDev(uint16_t conn_handle, btif_sniff_info_t* SniffInfo)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = CMGR_SetSniffInofToAllHandlerByRemDev_req;
    memcpy(&mail->param.CMGR_SetSniffInofToAllHandlerByRemDev_param.SniffInfo, SniffInfo, sizeof(btif_sniff_info_t));
    mail->param.CMGR_SetSniffInofToAllHandlerByRemDev_param.conn_handle = conn_handle;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_A2DP_OpenStream(a2dp_stream_t *Stream, bt_bdaddr_t *Addr)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = A2DP_OpenStream_req;
    mail->param.A2DP_OpenStream_param.Stream = Stream;
    mail->param.A2DP_OpenStream_param.Addr = *Addr;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_A2DP_CloseStream(a2dp_stream_t *Stream)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = A2DP_CloseStream_req;
    mail->param.A2DP_CloseStream_param.Stream = Stream;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_HF_CreateServiceLink(btif_hf_channel_t* Chan, bt_bdaddr_t *Addr)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = HF_CreateServiceLink_req;
    mail->param.HF_CreateServiceLink_param.Chan = Chan;
    mail->param.HF_CreateServiceLink_param.Addr = *Addr;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_HF_DisconnectServiceLink(btif_hf_channel_t* Chan)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = HF_DisconnectServiceLink_req;
    mail->param.HF_DisconnectServiceLink_param.Chan = Chan;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_HF_CreateAudioLink(btif_hf_channel_t* Chan)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = HF_CreateAudioLink_req;
    mail->param.HF_CreateAudioLink_param.Chan = Chan;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_HF_DisconnectAudioLink(btif_hf_channel_t* Chan)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = HF_DisconnectAudioLink_req;
    mail->param.HF_DisconnectAudioLink_param.Chan = Chan;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_GATT_Connect(const bt_bdaddr_t *Addr)
{
#ifdef __GATT_OVER_BR_EDR__
    TRACE(1,"%s send gatt connect req",__func__);

    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = GATT_Connect_Req;
    memcpy(&mail->param.GATT_param.Addr, Addr, sizeof(bt_bdaddr_t));
    app_bt_mail_send(mail);
#endif
    return 0;
}

#ifdef BT_DIP_SUPPORT
int app_bt_dip_QuryService(uint16_t conn_handle, btif_dip_client_t *client)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = DIP_QuryService_req;
    mail->param.DIP_QuryService_param.conn_handle = conn_handle;
    mail->param.DIP_QuryService_param.dip_client = client;
    app_bt_mail_send(mail);
    return 0;
}
#endif
