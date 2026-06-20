/***************************************************************************
 *
 * Copyright 2020-2025 BES.
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
 * @addtogroup GFPS
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#ifdef GFPS_ENABLED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "bluetooth.h"
#include "bt_if.h"
#include "btapp.h"
#include "app_bt_func.h"
#ifdef IBRT
#include "app_tws_ibrt.h"
#include "app_ibrt_customif_cmd.h"
#include "app_ibrt_conn_evt.h"
#include "app_ui_api.h"
#include "app_tws_ctrl_thread.h"
#include "earbud_ux_api.h"
#endif
#include "app_ui_ipscan_mgr.h"
#include "app_media_player.h"
#include "nvrecord_fp_account_key.h"
#include "customparam_section.h"
#include "gfps.h"
#include "gfps_ble.h"
#include "gfps_rfcomm.h"
#include "gfps_crypto.h"
#include "bes_gfps_api.h"
#include "app_ble.h"
#include "bes_gap_api.h"

#ifdef SASS_ENABLED
#include "gfps_sass.h"
#include "gfps_crypto.h"
#include "app_bt.h"
#endif

#ifdef SPOT_ENABLED
#include "bt_drv_interface.h"
#include "hal_timer.h"
#endif
#include "app_status_ind.h"
#include "apps.h"
#include "nvrecord_ble.h"
#include "app_tws_ibrt_cmd_handler.h"

#if BLE_AUDIO_ENABLED
#include "ble_aob_common.h"
#endif

/************************private macro defination***************************/
#define GFPS_MAIL_MAX (20)
#define GFPS_BATTERY_TIMEROUT_VALUE             (10000)

/************************private type defination****************************/
static GFPSEnv_t gfpsEnv = {0};
static __attribute__((unused)) FpRingStatus_t fp_ring_status = {false, false, 0};
static osMailQId gfps_mailbox = NULL;
static uint8_t gfps_mailbox_cnt = 0;

osTimerId ring_timeout_timer_id = NULL;
static void gfps_find_devices_ring_timeout_handler(void const *param);
osTimerDef (GFPS_FIND_DEVICES_RING_TIMEOUT, gfps_find_devices_ring_timeout_handler);

static void gfps_find_mybuds_timer_cb(void const *n);
osTimerDef (GFPS_FIND_MYBUDS_TIMER, gfps_find_mybuds_timer_cb);
static osTimerId gfps_find_mybuds_timer_id = NULL;

static void gfps_thread(void const *argument);
osThreadDef(gfps_thread, osPriorityNormal, 1, 1024*4, "gfps_thread");
osMailQDef (gfps_mailbox, GFPS_MAIL_MAX, GFPS_MESSAGE_BLOCK);

/****************************function defination****************************/
void gfps_link_connection_event_process(uint8_t devId, GFPS_CONNECTION_EVENT *pEvent);
void gfps_srv_event_rfcomm_process(uint8_t devId, GFPS_SRV_EVENT_RFCOM_PARAM_T *param);
void gfps_srv_event_l2cap_process(uint8_t devId, GFPS_SRV_EVENT_L2CAP_PARAM_T *param);
void gfps_link_connect_process(uint8_t devId, const bt_bdaddr_t *addr);

/************************extern function declearation***********************/
#ifdef FIRMWARE_REV
extern "C" void system_get_info(uint8_t *fw_rev_0, uint8_t *fw_rev_1, uint8_t *fw_rev_2, uint8_t *fw_rev_3);
#endif
//MSB->LSB
extern uint8_t ble_public_key[64];
extern uint8_t ble_private_key[32];

static int gfps_mailbox_init(void)
{
    if (!gfps_mailbox)
    {
        gfps_mailbox = osMailCreate(osMailQ(gfps_mailbox), NULL);
    }
    gfps_mailbox_cnt = 0;
    return 0;
}

int gfps_mailbox_free(GFPS_MESSAGE_BLOCK* msg_p)
{
    osStatus status;

    status = osMailFree(gfps_mailbox, msg_p);
    if (osOK != status)
    {
        TRACE(2,"%s error status 0x%02x", __func__, status);
        return (int)status;
    }

    if (gfps_mailbox_cnt)
    {
        gfps_mailbox_cnt--;
    }

    return (int)status;
}

int gfps_mailbox_free_all(void)
{
    GFPS_MESSAGE_BLOCK *msg_p = NULL;
    int status = osOK;
    osEvent evt;

    for (uint8_t i=0; i<gfps_mailbox_cnt; i++)
    {
        evt = osMailGet(gfps_mailbox, 500);
        if (evt.status == osEventMail)
        {
            msg_p = (GFPS_MESSAGE_BLOCK *)evt.value.p;
            status = gfps_mailbox_free(msg_p);
        }
        else
        {
            TRACE(1, "%s get mailbox timeout!!!", __func__);
            continue;
        }
   }
   return status;
}

int gfps_mailbox_put(uint8_t devId, uint8_t event, uint8_t *param, uint16_t len)
{
    osStatus status = osOK;

    GFPS_MESSAGE_BLOCK *msg_p = NULL;

    if(gfps_mailbox_cnt >= GFPS_MAIL_MAX -1)
    {
        TRACE(2,"%s warn gfps_mailbox_cnt is %d", __func__, gfps_mailbox_cnt);
        return osErrorValue;
    }

    msg_p = (GFPS_MESSAGE_BLOCK*)osMailAlloc(gfps_mailbox, 0);
    if (msg_p == NULL)
    {
        gfps_mailbox_free_all();
        msg_p = (GFPS_MESSAGE_BLOCK*)osMailAlloc(gfps_mailbox, 0);
        ASSERT(msg_p, "osMailAlloc error");
    }

    msg_p->devId = devId;
    msg_p->event = event;
    msg_p->len = len;
    if (param && len)
    {
        memcpy((uint8_t *)&(msg_p->p), param, len);
    }

    status = osMailPut(gfps_mailbox, msg_p);
    if (osOK != status)
    {
        TRACE(2,"%s error status 0x%02x", __func__, status);
        return (int)status;
    }

    gfps_mailbox_cnt++;

    return (int)status;
}

int gfps_mailbox_get(GFPS_MESSAGE_BLOCK** msg_p)
{
    osEvent evt;

    evt = osMailGet(gfps_mailbox, osWaitForever);
    if (evt.status == osEventMail)
    {
        *msg_p = (GFPS_MESSAGE_BLOCK *)evt.value.p;
        return 0;
    }
    else
    {
        TRACE(2,"%s evt.status 0x%02x", __func__, evt.status);
        return -1;
    }
}

static void gfps_thread(void const *argument)
{
    GFPS_MESSAGE_BLOCK* msg_p = NULL;
    uint16_t len = 0;
    uint8_t devId = 0;
    uint8_t event;
#ifdef SASS_ENABLED
    GFPS_SASS_PROFILE_EVENT *pEvent;
    uint8_t *p = NULL;
#endif

    while(1)
    {
        if(!gfps_mailbox_get(&msg_p))
        {
            len = msg_p->len;
            devId = msg_p->devId;
            event = msg_p->event;
            TRACE(3, "gfps_thread event:%d device_id:%d %d", event, devId, len);
            switch(event)
            {
                case GFPS_EVENT_CONNECTION:
                    gfps_link_connection_event_process(devId, (GFPS_CONNECTION_EVENT *)&(msg_p->p));
                    break;
                case GFPS_EVENT_PROFILE:
                {
#ifdef SASS_ENABLED
                    uint8_t emptyAddr[6] = {0};
                    memset(emptyAddr, 0, 6);

                    pEvent = (GFPS_SASS_PROFILE_EVENT *)&(msg_p->p);
                    if (pEvent->len > 0) {
                        p = pEvent->param;
                    }

                    gfps_sass_profile_event_handler(pEvent->pro, devId, &(pEvent->btAddr), pEvent->btEvt, p);
#endif
                }
                    break;
                case GFPS_EVENT_FROM_RFCOMM_SEEKER:
                    gfps_srv_event_rfcomm_process(devId, (GFPS_SRV_EVENT_RFCOM_PARAM_T *)&(msg_p->p));
                    break;
                case GFPS_EVENT_FROM_L2CAP_SEEKER:
                     gfps_srv_event_l2cap_process(devId, (GFPS_SRV_EVENT_L2CAP_PARAM_T *)&(msg_p->p));
                    break;
                default:
                    TRACE(1,"%s error msg", __func__);
                    break;
            }
            gfps_mailbox_free(msg_p);
        }
    }
}

void gfps_thread_init(void)
{
    if (gfps_mailbox_init())
    {
        TRACE(0, "ERROR: TOTA mailbox init failed");
        return;
    }
    osThreadId gfps_thread_tid = osThreadCreate(osThread(gfps_thread), NULL);
    if (gfps_thread_tid == NULL)
    {
        TRACE(0, "ERROR: AI thread init failed");
        return;
    }
}

uint8_t gfps_send(uint8_t devId, uint8_t *ptrData, uint32_t length)
{
    uint8_t ret = 0;
#if defined(IBRT) && !defined(FREEMAN_ENABLED_STERO)
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if (TWS_UI_SLAVE == app_ibrt_if_get_ui_role() && p_ibrt_ctrl->init_done)
    {
        return 1;
    }
#endif

    if (IS_BT_DEVICE(devId))
    {
        ret = app_fp_rfcomm_send(GET_BT_ID(devId), ptrData, length);
    }
#if BLE_AUDIO_ENABLED
    else
    {
        if (bes_ble_gfps_l2cap_send(devId, ptrData, length) == BT_STS_SUCCESS)
        {
            ret = 1;
        }
    }
#endif
    TRACE(3, "%s send data ret:%d id:%d", __func__, ret, devId);

    return ret;
}

static __attribute__((unused)) void gfps_send_active_components_rsp(uint8_t devId)
{
    FP_MESSAGE_STREAM_T req = {FP_MSG_GROUP_DEVICE_INFO, FP_MSG_DEVICE_INFO_ACTIVE_COMPONENTS_RSP, 0, 1};

#if defined(IBRT) && !defined(FREEMAN_ENABLED_STERO)
    if (app_tws_ibrt_tws_link_connected())
    {
        req.data[0] = FP_MSG_BOTH_BUDS_ACTIVE;
    }
    else
    {
        if (app_ibrt_if_is_left_side())
        {
            req.data[0] = FP_MSG_LEFT_BUD_ACTIVE;
        }
        else
        {
            req.data[0] = FP_MSG_RIGHT_BUD_ACTIVE;
        }
    }
#else
    req.data[0] = FP_MSG_RIGHT_BUD_ACTIVE;
#endif

    gfps_send(devId, (uint8_t *)&req, FP_MESSAGE_RESERVED_LEN+1);
}

void gfps_send_msg_ack(uint8_t devId, uint8_t msgGroup, uint8_t msgCode)
{
    FP_MESSAGE_STREAM_T req = {FP_MSG_GROUP_ACKNOWLEDGEMENT, FP_MSG_ACK, 0, 2};

    req.data[0] = msgGroup;
    req.data[1] = msgCode;

    gfps_send(devId, (uint8_t *)&req, FP_MESSAGE_RESERVED_LEN+2);
}

void gfps_send_msg_nak(uint8_t devId, uint8_t reason, uint8_t msgGroup, uint8_t msgCode)
{
    FP_MESSAGE_STREAM_T req = {FP_MSG_GROUP_ACKNOWLEDGEMENT, FP_MSG_NAK, 0, 3};

    req.data[0] = reason;
    req.data[1] = msgGroup;
    req.data[2] = msgCode;

    gfps_send(devId, (uint8_t *)&req, FP_MESSAGE_RESERVED_LEN+3);
}

void gfps_send_sync_ring_status(uint8_t devId, uint8_t status)
{
    FP_MESSAGE_STREAM_T req = {FP_MSG_GROUP_DEVICE_ACTION, FP_MSG_DEVICE_ACTION_RING, 0, 1};

    req.data[0] = status;

    gfps_send(devId, (uint8_t *)&req, FP_MESSAGE_RESERVED_LEN+1);
}

uint32_t gfps_get_model_id(void)
{
    return gfpsEnv.keyInfo.model_id;
}

void gfps_get_key_info(void)
{
    gfpsEnv.keyInfo.model_id = Get_ModelId();
    switch(gfpsEnv.keyInfo.model_id)
    {
        //default model id(bes moddel id)
        case FP_DEVICE_MODEL_ID:
        {
            memcpy (gfpsEnv.keyInfo.public_anti_spoofing_key,
                    ble_public_key, sizeof(ble_public_key));
            memcpy (gfpsEnv.keyInfo.private_anti_spoofing_key,
                    ble_private_key, sizeof(ble_private_key));
        }
        break;

       default:
       {
           gfpsEnv.keyInfo.model_id = FP_DEVICE_MODEL_ID;
           memcpy (gfpsEnv.keyInfo.public_anti_spoofing_key,
                    ble_public_key, sizeof(ble_public_key));
           memcpy (gfpsEnv.keyInfo.private_anti_spoofing_key,
                    ble_private_key, sizeof(ble_private_key));
       }
    }
}

const uint8_t* gfps_get_public_key(void) {
  return  gfpsEnv.keyInfo.public_anti_spoofing_key;
}

const uint8_t* gfps_get_private_key(void) {
  return gfpsEnv.keyInfo.private_anti_spoofing_key;
}

void gfps_enter_pairing_mode_handler(void)
{
#if defined(IBRT)
#if defined(IBRT_UI)
    app_ui_update_scan_type_policy(SCAN_EV_ENABLE);
#endif
#else
    app_bt_set_access_mode(BTIF_BAM_GENERAL_ACCESSIBLE);
#endif

#ifdef __INTERCONNECTION__
    clear_discoverable_adv_timeout_flag();
    app_interceonnection_start_discoverable_adv(INTERCONNECTION_BLE_FAST_ADVERTISING_INTERVAL,
            APP_INTERCONNECTION_FAST_ADV_TIMEOUT_IN_MS);
#endif
}

void gfps_enter_fastpairing_mode(void)
{
    TRACE(0,"[FP] enter fast pair mode");
    btif_sec_set_io_capabilities(3);
    btif_sec_set_authrequirements(0);
    gfps_set_in_fastpairing_mode_flag(true);
    bes_ble_gap_start_connectable_adv(BLE_FAST_ADVERTISING_INTERVAL);
}

bool gfps_is_in_fastpairing_mode(void)
{
    return gfpsEnv.isFastPairMode;
}

void gfps_set_in_fastpairing_mode_flag(bool isEnabled)
{
    gfpsEnv.isFastPairMode = isEnabled;
    TRACE(1,"[FP]mode is set to %d", gfpsEnv.isFastPairMode);
}

void gfps_exit_fastpairing_mode(void)
{
#if defined(IBRT)
#if defined(IBRT_UI)
    app_ui_update_scan_type_policy(SCAN_EV_EXIT_PAIRING);
#endif
#else
    app_bt_set_access_mode(BTIF_BAM_CONNECTABLE_ONLY);
#endif

    gfps_set_in_fastpairing_mode_flag(false); 
    // reset ble adv
    app_ble_refresh_adv_state_generic();
}

void gfps_set_ring_mode(uint8_t ring_mode)
{
    TRACE(2, "%s ring_mode:%d", __func__,ring_mode);

    gfpsEnv.ringMode = ring_mode;
}

uint8_t gfps_get_ring_mode()
{
    return gfpsEnv.ringMode;
}

void gfps_start_find_ringtone(void)
{
#ifdef MEDIA_PLAYER_SUPPORT
    media_PlayAudio(AUDIO_ID_FIND_TILE, 0);
#endif
}

void gfps_stop_find_ringtone(void)
{
    app_voice_stop(APP_STATUS_INDICATION_TILE_FIND, 0);
}

POSSIBLY_UNUSED static void gfps_delay_to_start_find_mybuds()
{
    if (gfps_find_mybuds_timer_id == NULL)
        gfps_find_mybuds_timer_id = osTimerCreate(osTimer(GFPS_FIND_MYBUDS_TIMER), osTimerOnce, NULL);
    osTimerStart(gfps_find_mybuds_timer_id, 1000);
}

void gfps_find_sm(bool find_on_off)
{
    bool music_on_off = 0;
    osTimerStop(gfps_find_mybuds_timer_id);
    TRACE(3, "%s %d %d", __func__, find_on_off, gfpsEnv.findState);
    switch(gfpsEnv.findState)
    {
        case GFPS_MUSIC_OFF_FIND_OFF:
            if(find_on_off)
            {
                gfps_start_find_ringtone();
                if(music_on_off)
                    gfpsEnv.findState=GFPS_MUSIC_ON_FIND_ON;
                else
                    gfpsEnv.findState=GFPS_MUSIC_OFF_FIND_ON;
            }
            else
            {
                if(music_on_off)
                {
                    gfpsEnv.findState=GFPS_MUSIC_ON_FIND_OFF;
                }
                gfps_stop_find_ringtone();
            }
            break;

        case GFPS_MUSIC_OFF_FIND_ON:
            if(find_on_off)
            {
                if(music_on_off)
                {
                    gfps_stop_find_ringtone();
                    gfps_delay_to_start_find_mybuds();
                    gfpsEnv.findState=GFPS_MUSIC_ON_FIND_OFF;//DELAY TO SATRT SM,TO OPEN FIND
                }
                else
                {
                    gfps_start_find_ringtone();
                }
                //gfps_start_find_ringtone();
            }
            else
            {
                gfps_stop_find_ringtone();
                if(music_on_off)
                    gfpsEnv.findState=GFPS_MUSIC_ON_FIND_OFF;
                else
                    gfpsEnv.findState=GFPS_MUSIC_OFF_FIND_OFF;
            }
            break;

        case GFPS_MUSIC_ON_FIND_OFF:
            if(find_on_off)
            {
                if(music_on_off)
                {
                    gfpsEnv.findState=GFPS_MUSIC_ON_FIND_ON;
                }
                else
                {
                    gfpsEnv.findState=GFPS_MUSIC_OFF_FIND_ON;
                }
                gfps_start_find_ringtone();
            }
            else
            {
                if(!music_on_off)
                {
                    gfpsEnv.findState=GFPS_MUSIC_OFF_FIND_OFF;
                }
                gfps_stop_find_ringtone();
            }
            break;

        case GFPS_MUSIC_ON_FIND_ON:
            if(find_on_off)
            {
                if(!music_on_off)
                {
                    gfpsEnv.findState=GFPS_MUSIC_OFF_FIND_ON;
                }
                gfps_start_find_ringtone();
            }
            else
            {
                gfps_stop_find_ringtone();
                if(music_on_off)
                {
                    gfpsEnv.findState=GFPS_MUSIC_ON_FIND_OFF;
                }
                else
                {
                    gfpsEnv.findState=GFPS_MUSIC_OFF_FIND_OFF;
                }
            }
            break;
    }
}

static void gfps_find_mybuds_timer_cb(void const *n)
{
    gfps_find_sm(true);
}

void gfps_set_find_my_buds_peer_status(bool onoff)
{
    gfpsEnv.findPeer = onoff;
}

void gfps_set_find_my_buds_stereo(uint8_t mode)
{
    switch (mode)
    {
        case GFPS_FIND_MY_BUDS_CMD_START:
            media_PlayAudio_standalone(AUDIO_ID_FIND_MY_BUDS, 0);
            break;
        case GFPS_FIND_MY_BUDS_CMD_STOP:
            app_voice_stop(APP_STATUS_INDICATION_FIND_MY_BUDS, 0);
        default:
            break;
    }
}

static void gfps_set_find_my_buds(uint8_t cmd)
{
    TRACE(2,"%s, cmd = %d", __func__, cmd);
#if defined(IBRT) && !defined(FREEMAN_ENABLED_STERO)
    if(GFPS_FIND_MY_BUDS_CMD_STOP_DUAL == cmd)
    {
        gfps_set_ring_mode(GFPS_RING_MODE_BOTH_OFF);
        gfps_find_sm(false);
        gfps_set_find_my_buds_peer_status(0);
    }
    else if(GFPS_FIND_MY_BUDS_CMD_START_MASTER_ONLY == cmd)    //right ring, stop left
    {
        gfps_set_ring_mode(GPFS_RING_MODE_RIGHT_ON);
        if (app_ibrt_if_is_left_side() == EAR_SIDE_LEFT)
        {
            gfps_set_find_my_buds_peer_status(1);
            gfps_find_sm(false);
        }
        else
        {
            gfps_find_sm(true);
        }
    }
    else if(GFPS_FIND_MY_BUDS_CMD_START_SLAVE_ONLY == cmd)    //left ring, stop right
    {
        gfps_set_ring_mode(GFPS_RING_MODE_LEFT_ON);
        if (app_ibrt_if_is_left_side() == EAR_SIDE_LEFT)
        {
            gfps_find_sm(true);
        }
        else
        {   
           gfps_set_find_my_buds_peer_status(1);
           gfps_find_sm(false);
        }
    }
    else if(GFPS_FIND_MY_BUDS_CMD_START_DUAL == cmd)    //both ring
    {
        gfps_set_ring_mode(GFPS_RING_MODE_BOTH_ON);
        gfps_find_sm(true);
        gfps_set_find_my_buds_peer_status(1);
    }
#else
    gfps_set_find_my_buds_stereo(cmd);
#endif
}

static void gfps_find_devices_ring_timeout_handler(void const *param)
{
    TRACE(0,"gfps_find_devices_ring_timeout_handler");
    app_bt_start_custom_function_in_bt_thread(GFPS_FIND_MY_BUDS_CMD_STOP_DUAL, 0, \
                                (uint32_t)gfps_set_find_my_buds);
}

void gfps_ring_timer_set(uint8_t period)
{
    TRACE(2,"%s, period = %d", __func__, period);
    if (ring_timeout_timer_id == NULL)
    {
        ring_timeout_timer_id = osTimerCreate(osTimer(GFPS_FIND_DEVICES_RING_TIMEOUT), osTimerOnce, NULL);
    }

    osTimerStop(ring_timeout_timer_id);
    if(period)
    {
        osTimerStart(ring_timeout_timer_id, period*1000);
    }
}

static void gfps_ring_request_handling(uint8_t devId, uint8_t* requestdata, uint16_t datalen)
{
    TRACE(1,"%s,[RFCOMM][FMD] request",__func__);
    DUMP8("%02x ", requestdata, datalen);
#if defined(IBRT) && defined(IBRT_UI) && !defined(FREEMAN_ENABLED_STERO)
    switch (requestdata[0])
    {
        case GFPS_FIND_MY_BUDS_CMD_START_MASTER_ONLY://ring right
            if (!app_ibrt_if_is_left_side())
            {
                if (app_ibrt_if_get_ui_role() == IBRT_MASTER)      //right is master
                {
                    if (app_ui_get_local_box_state() == IBRT_IN_BOX_OPEN)
                    {
                        TRACE(0,"right phone-right master in box");
                        gfps_send_msg_nak(devId, FP_MSG_NAK_REASON_NOT_ALLOWED,FP_MSG_GROUP_DEVICE_ACTION, FP_MSG_DEVICE_ACTION_RING);
                        gfps_send_sync_ring_status(devId, GFPS_FIND_MY_BUDS_CMD_STOP_DUAL);

                        gfps_send_msg_ack(devId, FP_MSG_GROUP_DEVICE_ACTION, FP_MSG_DEVICE_ACTION_RING);
                        if (datalen > 1)
                        {
                             gfps_ring_timer_set(GFPS_FIND_MY_BUDS_CMD_STOP_DUAL);
                         }

                         gfps_set_find_my_buds(GFPS_FIND_MY_BUDS_CMD_STOP_DUAL);
                         return;
                    }
                    else
                    {
                        TRACE(0,"right phone-right master out box");
                    }
                    
                }
                else    //right is slave
                {
                    if(app_ui_get_local_box_state() == IBRT_IN_BOX_OPEN)
                    {
                        TRACE(0,"right phone-right slave in box");
                        return;
                    }
                    {
                         TRACE(0,"right phone-right slave out box");
                     }
                }
            }
            else if (app_ibrt_if_is_left_side())
            {
                if (app_ibrt_if_get_ui_role() == IBRT_MASTER)     //left is master
                {
                    if (app_ui_get_peer_box_state() == IBRT_IN_BOX_OPEN)
                    {
                        TRACE(0,"right phone-left master but right in box");
                        gfps_send_msg_nak(devId, FP_MSG_NAK_REASON_NOT_ALLOWED,FP_MSG_GROUP_DEVICE_ACTION, FP_MSG_DEVICE_ACTION_RING);
                        gfps_send_sync_ring_status(devId, GFPS_FIND_MY_BUDS_CMD_STOP_DUAL);

                        gfps_send_msg_ack(devId, FP_MSG_GROUP_DEVICE_ACTION, FP_MSG_DEVICE_ACTION_RING);
                        if (datalen > 1)
                        {
                            gfps_ring_timer_set(GFPS_FIND_MY_BUDS_CMD_STOP_DUAL);
                        }
                        gfps_set_find_my_buds(GFPS_FIND_MY_BUDS_CMD_STOP_DUAL);
                        return;
                    }
                    else
                    {
                        TRACE(0,"right phone-left master but right out box");
                    }
                }
                else
                {
                    if(app_ui_get_local_box_state() == IBRT_IN_BOX_OPEN)
                    {
                        TRACE(0,"right phone-left slave int box");
                    }
                    else
                    {
                        TRACE(0,"right phone-left slave out box");
                    }
                }
            }
            break;
        case GFPS_FIND_MY_BUDS_CMD_START_SLAVE_ONLY://ring left
            if (app_ibrt_if_is_left_side())
            {
                if (app_ibrt_if_get_ui_role()  == IBRT_MASTER)   //left is master
                {
                    if (app_ui_get_local_box_state()== IBRT_IN_BOX_OPEN)
                    {
                        TRACE(0,"left phone-left master in box");
                        gfps_send_msg_nak(devId, FP_MSG_NAK_REASON_NOT_ALLOWED,FP_MSG_GROUP_DEVICE_ACTION, FP_MSG_DEVICE_ACTION_RING);
                        gfps_send_sync_ring_status(devId, GFPS_FIND_MY_BUDS_CMD_STOP_DUAL);
                        return;
                    }
                    else
                    {
                        TRACE(0,"left phone-left master out box");
                    }
                }
                else  //left is slave
                {
                    if(app_ui_get_local_box_state()== IBRT_IN_BOX_OPEN)
                    {
                        TRACE(0,"left phone-left slave in box");
                        return;
                    }
                    else
                    {
                        TRACE(0,"left phone-left slave out box");
                    }
                }

            }
            else if (!app_ibrt_if_is_left_side())
            {
                if (app_ibrt_if_get_ui_role() == IBRT_MASTER)//right is master
                {
                     if (app_ui_get_peer_box_state()== IBRT_IN_BOX_OPEN)
                    {
                        TRACE(0,"left phone-right master but left in box");
                        gfps_send_msg_nak(devId, FP_MSG_NAK_REASON_NOT_ALLOWED,FP_MSG_GROUP_DEVICE_ACTION, FP_MSG_DEVICE_ACTION_RING);
                        gfps_send_sync_ring_status(devId, GFPS_FIND_MY_BUDS_CMD_STOP_DUAL);
                        return;
                    }
                    else
                    {
                        TRACE(0,"left phone-right master but left out box");
                    }
                }
                else
                {
                    TRACE(0,"left phone-right slave in box or out box");
                }
            }
            break;

            case GFPS_FIND_MY_BUDS_CMD_START_DUAL:
            if (app_ibrt_if_is_left_side())
            {
                if (app_ibrt_if_get_ui_role()  == IBRT_MASTER)    //left is master
                {
                    if (app_ui_get_local_box_state()== IBRT_IN_BOX_OPEN)
                    {
                        TRACE(0,"DUAL left phone-left master in box");
                        gfps_send_msg_nak(devId, FP_MSG_NAK_REASON_NOT_ALLOWED,FP_MSG_GROUP_DEVICE_ACTION, FP_MSG_DEVICE_ACTION_RING);
                        if (app_ui_get_peer_box_state()== IBRT_IN_BOX_OPEN)
                        {
                            gfps_send_sync_ring_status(devId, GFPS_FIND_MY_BUDS_CMD_STOP_DUAL);
                            return ;
                        }
                        else
                        {
                            gfps_send_sync_ring_status(devId, GFPS_FIND_MY_BUDS_CMD_START_MASTER_ONLY);
                            return ;
                        }
                    }
                    else
                    {
                        if (app_ui_get_peer_box_state() == IBRT_IN_BOX_OPEN)
                        {
                            gfps_send_msg_nak(devId, FP_MSG_NAK_REASON_NOT_ALLOWED,FP_MSG_GROUP_DEVICE_ACTION, FP_MSG_DEVICE_ACTION_RING);
                            gfps_send_sync_ring_status(devId, GFPS_FIND_MY_BUDS_CMD_START_SLAVE_ONLY);

                            gfps_send_msg_ack(devId, FP_MSG_GROUP_DEVICE_ACTION, FP_MSG_DEVICE_ACTION_RING);

                            if (datalen > 1)
                            {
                                gfps_ring_timer_set(GFPS_FIND_MY_BUDS_CMD_START_SLAVE_ONLY);
                            }
                            TRACE(0,"DAUL need left ring");
                            return; 
                        }

                        TRACE(0,"DAUL left phone-left master BOTH out box");
                    }
                }
                else    //left is slave
                {
                    if(app_ui_get_local_box_state() == IBRT_IN_BOX_OPEN)
                    {
                        TRACE(0,"left phone-left slave in box");
                        return;
                    }
                    else
                    {
                        TRACE(0,"left phone-left slave out box");
                        if (app_ui_get_peer_box_state() == IBRT_IN_BOX_OPEN)
                        {
                            gfps_set_find_my_buds(GFPS_FIND_MY_BUDS_CMD_START_SLAVE_ONLY);
                        }
                        else
                        {
                            gfps_set_find_my_buds(GFPS_FIND_MY_BUDS_CMD_START_DUAL);
                        }
                        return;
                    }

                }
            }
            else if (!app_ibrt_if_is_left_side())
            {
                if (app_ibrt_if_get_ui_role() == IBRT_MASTER)    //right is master
                {
                    if (app_ui_get_local_box_state()== IBRT_IN_BOX_OPEN)
                    {
                        gfps_send_msg_nak(devId, FP_MSG_NAK_REASON_NOT_ALLOWED,FP_MSG_GROUP_DEVICE_ACTION, FP_MSG_DEVICE_ACTION_RING);
                        if (app_ui_get_peer_box_state()== IBRT_IN_BOX_OPEN)
                        {
                            TRACE(0,"DUAL phone-right master and BOTH in box");
                            gfps_send_sync_ring_status(devId, GFPS_FIND_MY_BUDS_CMD_STOP_DUAL);
                            return ;
                        }
                        else
                        {
                            TRACE(0,"DUAL phone-right master and in box");
                            gfps_send_sync_ring_status(devId, GFPS_FIND_MY_BUDS_CMD_START_SLAVE_ONLY);
                            return ;
                        }
                    }
                    else
                    {
                        if (app_ui_get_peer_box_state()== IBRT_IN_BOX_OPEN)
                        {
                            TRACE(0,"DUAL phone-right master BUT LEFT in box");
                            gfps_send_msg_nak(devId, FP_MSG_NAK_REASON_NOT_ALLOWED,FP_MSG_GROUP_DEVICE_ACTION, FP_MSG_DEVICE_ACTION_RING);
                            gfps_send_sync_ring_status(devId, GFPS_FIND_MY_BUDS_CMD_START_MASTER_ONLY);

                            gfps_send_msg_ack(devId, FP_MSG_GROUP_DEVICE_ACTION, FP_MSG_DEVICE_ACTION_RING);

                            if (datalen > 1)
                            {
                                gfps_ring_timer_set(GFPS_FIND_MY_BUDS_CMD_START_MASTER_ONLY);
                            }
                            TRACE(0,"DUAL ring right");
                            
                            gfps_set_find_my_buds(GFPS_FIND_MY_BUDS_CMD_START_MASTER_ONLY);
                            return;
                        }
                        else
                        {
                            TRACE(0,"DUAL phone-right master BOTH NOT IN box");
                        }
                    }
                }
                else    //left is slave
                {
                    TRACE(0,"left phone-right slave in box or out box");
                    if(app_ui_get_local_box_state()== IBRT_IN_BOX_OPEN)
                    {
                        TRACE(0,"left phone-left slave in box");
                        return ;
                    }
                    else
                    {
                         TRACE(0,"left phone-left slave out box");
                         if(app_ui_get_peer_box_state()== IBRT_IN_BOX_OPEN)
                         {
                             gfps_set_find_my_buds(GFPS_FIND_MY_BUDS_CMD_START_MASTER_ONLY);
                         }
                         else
                         {
                             gfps_set_find_my_buds(GFPS_FIND_MY_BUDS_CMD_START_DUAL);
                         }
                         return;
                    }
                    return;
                }
               
            }
            break;

            default:
                break;
    }
#endif
    if((app_ui_get_peer_box_state()== IBRT_IN_BOX_OPEN) && (app_ui_get_peer_box_state()== IBRT_IN_BOX_OPEN))
    {
        TRACE(0,"######both earbud is in box");
        gfps_send_msg_nak(devId, FP_MSG_NAK_REASON_NOT_ALLOWED, FP_MSG_GROUP_DEVICE_ACTION, FP_MSG_DEVICE_ACTION_RING);
        return;
    }
    
    gfps_send_msg_ack(devId, FP_MSG_GROUP_DEVICE_ACTION, FP_MSG_DEVICE_ACTION_RING);
     if (datalen > 1)
    {
        gfps_ring_timer_set(requestdata[1]);
    }

    gfps_set_find_my_buds(requestdata[0]);
}

// use cases for fp message stream
void gfps_enable_bt_silence_mode(uint8_t devId, bool isEnable)
{
    if (gfpsEnv.fpCap.env.isSilentModeSupported)
    {
        FP_MESSAGE_STREAM_T req = {FP_MSG_GROUP_BLUETOOTH_EVENT, 0, 0, 0};
        if (isEnable)
        {
            req.messageCode = FP_MSG_BT_EVENT_ENABLE_SILENCE_MODE;
        }
        else
        {
            req.messageCode = FP_MSG_BT_EVENT_DISABLE_SILENCE_MODE;
        }

        gfps_send(devId, (uint8_t *)&req, FP_MESSAGE_RESERVED_LEN);
    }
    else
    {
        TRACE(0,"fp silence mode is not supported.");
    }
}

void gfps_send_model_id(uint8_t devId)
{
    TRACE(1,"%s",__func__);
    uint32_t model_id = gfps_get_model_id();

    uint8_t  modelID[3];
    modelID[0] = (model_id >> 16) & 0xFF;
    modelID[1] = (model_id >> 8) & 0xFF;
    modelID[2] = ( model_id )&0xFF;

    uint16_t rawDataLen = sizeof(modelID);

    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_DEVICE_INFO,
         FP_MSG_DEVICE_INFO_MODEL_ID,
         (uint8_t)(rawDataLen >> 8),
         (uint8_t)(rawDataLen & 0xFF)};
    memcpy(req.data, modelID, sizeof(modelID));

    gfps_send(devId, ( uint8_t * )&req, FP_MESSAGE_RESERVED_LEN + rawDataLen);
}

void gfps_get_updated_ble_addr(uint8_t devId, uint8_t *addr)
{
    bool isIdentity = false;
    uint8_t ptr[6] = {0};
    POSSIBLY_UNUSED ble_bdaddr_t bleAddr;
    if (devId == 0xFF)
    {
        return;
    }

    if (IS_BT_DEVICE(devId))
    {
        BT_DEVICE_T *btInfo = app_bt_get_device(GET_BT_ID(devId));
        if (btInfo)
        {
             if (nv_record_blerec_is_paired_from_addr(btInfo->remote.address))
             {
                 TRACE(1, "%s get paired dev from nv", __func__);
                 isIdentity = true;
             }
        }
    }
    else
    {
        isIdentity = true;
    }

    if (isIdentity)
    {
        bleAddr = bes_ble_gap_get_local_identity_addr(0xFF);
        memcpy(ptr, bleAddr.addr, 6);
    }
    else
    {
        gfps_ble_get_addr(ptr);
    }

    for (uint8_t index = 0; index < 6; index++)
    {
        addr[index] = ptr[5 - index];
    }
}

void gfps_send_ble_addr(uint8_t devId)
{
    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_DEVICE_INFO,
         FP_MSG_DEVICE_INFO_BLE_ADD_UPDATED,
         0,
         6};

    if (devId != 0xFF)
    {
        gfps_get_updated_ble_addr(devId, req.data);
        gfps_send(devId,( uint8_t * )&req, FP_MESSAGE_RESERVED_LEN + 6);
    }
}

void gfps_send_battery_levels(uint8_t devId)
{ 
    uint8_t batteryLevelCount = 0;
    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_DEVICE_INFO,
         FP_MSG_DEVICE_INFO_BATTERY_UPDATED,
         0,
         0};

    gfps_get_battery_levels(&batteryLevelCount, req.data);
    req.dataLenLowByte = batteryLevelCount;

    gfps_send(devId, ( uint8_t * )&req, FP_MESSAGE_RESERVED_LEN + batteryLevelCount);
}

#ifdef SPOT_ENABLED
void gfps_send_firmware_version(uint8_t devId)
{
    TRACE(1,"%s",__func__);
    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_DEVICE_INFO,
         FP_MSG_DEVICE_INFO_FIRMWARE_VERSION,
         0};
    char firware_revision[17]= {0};
    uint8_t fw_rev_0 = 0, fw_rev_1 = 0, fw_rev_2 = 0,fw_rev_3=1;
#ifdef FIRMWARE_REV
    system_get_info(&fw_rev_0, &fw_rev_1, &fw_rev_2, &fw_rev_3);
#endif
    snprintf(firware_revision, sizeof(firware_revision), "%d.%d.%d.%d", fw_rev_0, fw_rev_1, fw_rev_2,fw_rev_3);
    memcpy(&req.data[0], firware_revision,strlen(firware_revision));
    req.dataLenLowByte = strlen(firware_revision);

    gfps_send(devId,( uint8_t * )&req, FP_MESSAGE_RESERVED_LEN + strlen(firware_revision));
}

void gfps_send_eddystone_identifier_state(uint8_t devId)
{
    TRACE(1,"%s",__func__);
    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_DEVICE_INFO,
         FP_MSG_DEVICE_INFO_EDD_IDENTIFIER,
         0,
         24};
    uint32_t current_time = nv_record_fp_get_current_beacon_time() + GET_CURRENT_MS()/1000;
    uint8_t time[4];
    memcpy(time, &current_time, sizeof(uint32_t));
    big_little_switch(time, &req.data[0], sizeof(uint32_t));

    memcpy(&req.data[4], nv_record_fp_get_spot_adv_data(), 20);

    gfps_send(devId,( uint8_t * )&req, FP_MESSAGE_RESERVED_LEN + 24);
}

void gfps_spot_event_handler(uint8_t devId)
{
    FP_MESSAGE_STREAM_T req = {FP_MSG_GROUP_DEVICE_CAPABLITY_SYNC, FP_MSG_DEVICE_CAPABLITY_ENABLE_EDD_TRACK, 0, 7};

    req.data[0] = nv_record_fp_get_spot_adv_enable_value();
    if(devId != 0xFF)
    {
        gfps_get_updated_ble_addr(devId, &req.data[1]);
        gfps_send(devId, (uint8_t *)&req, FP_MESSAGE_RESERVED_LEN + 7);
    }
}
#endif

uint8_t gfps_get_bt_iocap(void)
{
    return gfpsEnv.bt_iocap;
}

void gfps_set_bt_iocap(uint8_t ioCap)
{
    if (gfpsEnv.btSetIocap)
    {
        gfpsEnv.btSetIocap(ioCap);
    }
}

uint8_t gfps_get_bt_auth(void)
{
    return gfpsEnv.bt_authrequirements;
}

void gfps_set_bt_auth(uint8_t auth)
{
    if (gfpsEnv.btSetAuthrequirements)
    {
        gfpsEnv.btSetAuthrequirements(auth);
    }
}

void gfps_enter_pairing_mode(void)
{
    if (gfpsEnv.enterPairingMode)
    {
        gfpsEnv.enterPairingMode();
    }
}

uint8_t *gfps_get_last_response(void)
{
    return gfpsEnv.pendingLastResponse;
}

bool gfps_is_last_response_pending(void)
{
    return gfpsEnv.isLastResponsePending;
}

void gfps_reg_battery_handler(gfps_get_battery_info_cb cb)
{
    gfpsEnv.getBatteryHandler = cb;
}

void gfps_set_battery_datatype(GFPS_BATTERY_DATA_TYPE_E batteryDataType)
{
    if (gfpsEnv.batteryDataType != batteryDataType)
    {
        gfpsEnv.batteryDataType = batteryDataType;
        app_ble_refresh_adv_state_generic();
    }
}

GFPS_BATTERY_DATA_TYPE_E gfps_get_battery_datatype(void)
{
    return gfpsEnv.batteryDataType;
}

void gfps_enable_battery_info(bool isEnable)
{
    gfpsEnv.isBatteryInfoIncluded = isEnable;
    app_ble_refresh_adv_state_generic();
}

bool gfps_is_battery_enabled(void)
{
    return gfpsEnv.isBatteryInfoIncluded;
}

void gfps_get_battery_levels(uint8_t *pCount, uint8_t *pBatteryLevel)
{
    *pCount = 0;
    if (gfpsEnv.getBatteryHandler)
    {
        gfpsEnv.getBatteryHandler(pCount, pBatteryLevel);
    }
}

void gfps_enter_connectable_mode_req_handler(uint8_t *response)
{
    TRACE(2,"%s isLastResponsePending:%d", __func__, gfpsEnv.isLastResponsePending);
    TRACE(0,"response data:");
    DUMP8("%02x ", response, GFPSP_ENCRYPTED_RSP_LEN);

#ifdef IBRT
    POSSIBLY_UNUSED ibrt_mobile_info_t *p_mobile_info = app_ibrt_conn_get_mobile_info_ext();
#endif

#ifndef IBRT
    if (btif_me_get_activeCons() > 0)
#else
    if (app_ibrt_conn_get_local_connected_mobile_count() > 1)
#endif
    {
        memcpy(gfpsEnv.pendingLastResponse, response, GFPSP_ENCRYPTED_RSP_LEN);
        gfpsEnv.isLastResponsePending = true;
    #ifndef IBRT
        app_disconnect_all_bt_connections();
    #else
        app_tws_ibrt_disconnect_mobile(&p_mobile_info->mobile_addr);
    #endif
    }
    else
    {
        gfpsEnv.isLastResponsePending = false;
        bes_ble_gfps_send_keybase_pairing(gfps_ble_get_connect_dev(), response, GFPSP_ENCRYPTED_RSP_LEN);
        TRACE(0,"wait for pair req maybe classic or ble");

        gfps_enter_pairing_mode();

        gfps_set_bt_iocap(1);

        gfps_set_bt_auth(1);
    }
}

#ifdef SASS_ENABLED
void gfps_sass_event_handler(uint8_t devId, uint8_t evt, void *param)
{
    TRACE(3,"%s id:%d evt:0x%0x", __func__, devId, evt);
    /* bt_bdaddr_t currAddr;

    if(IS_BT_DEVICE(devId))
    {
        app_bt_get_device_bdaddr(GET_BT_ID(devId), currAddr.address);
    }
    else
    {
        ble_bdaddr_t GetPeerAddr = {{0}};
        app_ble_get_peer_solved_addr(devId, &GetPeerAddr);
        memcpy(currAddr.address, GetPeerAddr.addr, 6);
    }
    gfps_sass_connect_handler(devId, &currAddr); */

    if (evt != FP_MSG_SASS_GET_CAPBILITY && evt != FP_MSG_SASS_GET_SWITCH_PREFERENCE && \
        evt != FP_MSG_SASS_GET_CONN_STATUS && evt != FP_MSG_SASS_SWITCH_BACK && \
        evt != FP_MSG_SASS_SWITCH_ACTIVE_SOURCE)
    {
        gfps_send_msg_ack(devId, FP_MSG_GROUP_SASS, evt);
    }

    gfps_sass_handler(devId, evt, param);
}
#endif


#if defined(IBRT) && !defined(FREEMAN_ENABLED_STERO)
void app_ibrt_share_fastpair_info(uint8_t *p_buff, uint16_t length)
{
    app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_SHARE_FASTPAIR_INFO, p_buff, length);
}

void app_tws_send_fastpair_info_to_slave(void)
{
    TRACE(0,"Send fastpair info to secondary device.");
    NV_FP_ACCOUNT_KEY_RECORD_T *pFpData = nv_record_get_fp_data_structure_info();
    app_ibrt_share_fastpair_info(( uint8_t * )pFpData, sizeof(NV_FP_ACCOUNT_KEY_RECORD_T));
}

void app_ibrt_share_fastpair_info_received_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    NV_FP_ACCOUNT_KEY_RECORD_T *pFpData = ( NV_FP_ACCOUNT_KEY_RECORD_T * )p_buff;
    nv_record_update_fp_data_structure(pFpData);
#ifdef SASS_ENABLED
    if (length > sizeof(NV_FP_ACCOUNT_KEY_RECORD_T))
    {
        gfps_sass_set_sync_info(p_buff + sizeof(NV_FP_ACCOUNT_KEY_RECORD_T), length - sizeof(NV_FP_ACCOUNT_KEY_RECORD_T));
    }
#endif
}

void gfps_tws_info_prepare_handler(uint8_t *buf, uint16_t *totalLen, uint16_t *len, uint16_t expectLen)
{
    TRACE(0,"Send fastpair info to secondary device.");
    NV_FP_ACCOUNT_KEY_RECORD_T *pFpData = nv_record_get_fp_data_structure_info();
    memcpy(buf, pFpData, sizeof(NV_FP_ACCOUNT_KEY_RECORD_T));
    *totalLen = *len = sizeof(NV_FP_ACCOUNT_KEY_RECORD_T);
    
#ifdef SASS_ENABLED
    uint16_t sassLen = 0;
    gfps_sass_get_sync_info(buf+(*len), &sassLen);
    *totalLen += sassLen;
    *len += sassLen;   
#endif  
}

void gfps_tws_info_received_handler(uint8_t *buf, uint16_t length, bool isContinueInfo)
{
    uint16_t offset = 0;
    nv_record_fp_update_all(buf);
    offset += sizeof(NV_FP_ACCOUNT_KEY_RECORD_T);
#ifdef SASS_ENABLED
    gfps_sass_set_sync_info(buf + offset, length - offset);
#endif
}

void gfps_send_ble_disconnect_cmd_to_slave(uint8_t *buf, uint16_t length)
{
    app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_SNED_GFPS_BLE_DISC_CMD, buf, length);
}

#if (BLE_AUDIO_ENABLED == 1)
void gfps_send_streaming_state_to_master(bt_bdaddr_t *addr, uint8_t state)
{
    GFPS_STREAMING_STATE_T syncInfo;
    memcpy(syncInfo.devAddr.address, addr->address, sizeof(bt_bdaddr_t));
    syncInfo.state = state;
    app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_SNED_STREAMING_STATE_CMD, (uint8_t *)&syncInfo, sizeof(GFPS_STREAMING_STATE_T));
}

void gfps_rec_peer_streaming_state_handler(uint8_t *p_buff, uint16_t length)
{
#ifdef SASS_ENABLED
    GFPS_STREAMING_STATE_T *syncInfo = (GFPS_STREAMING_STATE_T *)p_buff;
    gfps_sass_set_peer_streaming_state(&(syncInfo->devAddr), syncInfo->state);
#endif
}
#endif

void gfps_tws_sync_init(void)
{
    // TODO: freddie move to isolated ota file
    TWS_SYNC_USER_T userGfps = {
        gfps_tws_info_prepare_handler,
        gfps_tws_info_received_handler,
        NULL,
        NULL,
        NULL,
    };

    app_ibrt_if_register_sync_user(TWS_SYNC_USER_GFPS_INFO, &userGfps);
}

void gfps_sync_info(void)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();    
    if (TWS_UI_SLAVE != app_ibrt_if_get_ui_role()&& p_ibrt_ctrl->init_done)
    {
        uint8_t info[FP_TWS_MAX_LEN];
        uint16_t offset = 0;
        TRACE(0,"Send fastpair info to secondary device.");
        NV_FP_ACCOUNT_KEY_RECORD_T *pFpData = nv_record_get_fp_data_structure_info();
        memcpy(info, pFpData, sizeof(NV_FP_ACCOUNT_KEY_RECORD_T));
        offset += sizeof(NV_FP_ACCOUNT_KEY_RECORD_T);
    
#ifdef SASS_ENABLED
        uint16_t sassLen = 0;
        gfps_sass_get_sync_info(info + offset, &sassLen);
        offset += sassLen;
        ASSERT(offset <= FP_TWS_MAX_LEN, "Len exceed FP_TWS_MAX_LEN");
#endif
        tws_ctrl_send_cmd(APP_TWS_CMD_SHARE_FASTPAIR_INFO, info, offset);

    }
}

void gfps_info_received_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    uint16_t offset = 0;
    nv_record_fp_update_all((uint8_t *)p_buff);
    offset += sizeof(NV_FP_ACCOUNT_KEY_RECORD_T);
#ifdef SASS_ENABLED
    gfps_sass_set_sync_info(p_buff + offset, length - offset);
#endif
}

void gfps_role_switch_prepare(void)
{
    gfps_sync_info();
}
#endif

void gfps_set_anc_current_state_info(uint8_t current_state_info)
{
    //need to get anc state
}

uint8_t gfps_get_anc_current_state_info(void)
{
    //need to update anc state
    return 0;
}

void gfps_notify_hearable_control_state(uint8_t devId)
{
    TRACE(2,"%s,devId is %d", __func__, devId);

    FpANCState_t ancdata;
    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_HEARABLE_CONTROL,
         FP_MSG_HC_NTF_ANC_STATE,
         0,
         0x04};

    ancdata.version = FP_HEARABLE_CONTROL_VERSION;
    //if transport off and buds are on head, current state is off
    ancdata.ui_transparent = 0x01;
    ancdata.ui_off = 0x01;
    ancdata.ui_anc = 0x01;

    ancdata.settable_transparent = 0x01;
    ancdata.settable_off = 0x01;
    ancdata.settable_anc = 0x01;

    ancdata.current_state_info = gfps_get_anc_current_state_info();

    memcpy(req.data, (uint8_t *)&ancdata, 0x04);
    gfps_send(devId,( uint8_t * )&req, FP_MESSAGE_RESERVED_LEN + 0x04);
}

void gfps_set_hearable_control_state(uint8_t devId, uint8_t *data)
{
    TRACE(2,"%s,devId is %d", __func__, devId);
    DUMP8("%02x ", data, 17);

    gfps_set_anc_current_state_info(data[0]);

    gfps_notify_hearable_control_state(devId);
    gfps_send_msg_ack(devId, FP_MSG_GROUP_HEARABLE_CONTROL, FP_MSG_HC_SET_ANC_STATE);
}

uint16_t gfps_data_handler(uint8_t devId, uint8_t* ptr, uint16_t len)
{
    FP_MESSAGE_STREAM_T* pMsg = (FP_MESSAGE_STREAM_T *)ptr;
    uint16_t datalen = 0;
    uint16_t msgLen = ((pMsg->dataLenHighByte << 8)|pMsg->dataLenLowByte) + \
                       FP_MESSAGE_RESERVED_LEN;
    TRACE(4,"gfps receives msg group %d code %d len %d %d", pMsg->messageGroup, pMsg->messageCode, len, msgLen);
    if (len < msgLen)
    {
        return 0;
    }

    switch (pMsg->messageGroup)
    {
        case FP_MSG_GROUP_DEVICE_INFO:
        {
            switch (pMsg->messageCode)
            {
                case FP_MSG_DEVICE_INFO_ACTIVE_COMPONENTS_REQ:
                    gfps_send_active_components_rsp(devId);
                    break;
                case FP_MSG_DEVICE_INFO_TELL_CAPABILITIES:
                    gfpsEnv.fpCap.content = pMsg->data[0];
                    TRACE(3,"cap 0x%x isCompanionAppInstalled %d isSilentModeSupported %d",
                        gfpsEnv.fpCap.content, gfpsEnv.fpCap.env.isCompanionAppInstalled,
                        gfpsEnv.fpCap.env.isSilentModeSupported);
                    break;
                default:
                    break;
            }
            break;
        }
        case FP_MSG_GROUP_DEVICE_ACTION:
        {
            switch (pMsg->messageCode)
            {
                case FP_MSG_DEVICE_ACTION_RING:
                    datalen = (pMsg->dataLenHighByte<<8)+pMsg->dataLenLowByte;
                    gfps_ring_request_handling(devId, pMsg->data, datalen);
                    break;
                default:
                    break;
            }
            break;
        }

        case FP_MSG_GROUP_SASS:
        {
#ifdef SASS_ENABLED
            gfps_sass_event_handler(devId, pMsg->messageCode, pMsg->data);
#endif
            break;
        }

        case FP_MSG_GROUP_DEVICE_CAPABLITY_SYNC:
        {
#ifdef SPOT_ENABLED
            switch(pMsg->messageCode)
            {
                case FP_MSG_DEVICE_CAPABLITY_CAP_UPDATE_REQ:
                    gfps_spot_event_handler(devId);
                    break;
                default:
                    break;
            }
#endif
            break;
        }

        case FP_MSG_GROUP_HEARABLE_CONTROL:
        {
#ifdef HEARABLE_CTRL_ENABLED
            switch(pMsg->messageCode)
            {
                case FP_MSG_HC_GET_ANC_STATE:
                    gfps_notify_hearable_control_state(devId);
                    break;
                case FP_MSG_HC_SET_ANC_STATE:
                    gfps_set_hearable_control_state(devId, pMsg->data);
                    break;
                default:
                    break;
            }
#endif
            break;
        }

        case FP_MSG_GROUP_ACKNOWLEDGEMENT:
            break;

        default:
            msgLen = 0;
            break;
    }
    return msgLen;
}

static void gfps_srv_connect_handler(uint8_t devId, const bt_bdaddr_t *pBtAddr)
{
    gfps_link_connect_process(devId, pBtAddr);

#if defined(IBRT) && !defined(FREEMAN_ENABLED_STERO)
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    if (TWS_UI_SLAVE != app_ibrt_if_get_ui_role()&& p_ibrt_ctrl->init_done)
#endif
    {
        gfps_send_model_id(devId);
        gfps_send_ble_addr(devId);
        gfps_send_battery_levels(devId);
#ifdef SASS_ENABLED
        gfps_sass_send_info_to_seeker(devId);
#endif

#ifdef SPOT_ENABLED
        gfps_send_firmware_version(devId);
        gfps_send_eddystone_identifier_state(devId);
        gfps_spot_event_handler(devId);
#endif

#if defined(IBRT) && !defined(FREEMAN_ENABLED_STERO)
        gfps_sync_info();
#endif
      
#ifdef SASS_ENABLED
        gfps_sass_exe_pending_switch_media(devId);
#endif
    }
}

static void gfps_srv_disconnect_handler(uint8_t devId)
{
    return;
}

void gfps_link_connect_process(uint8_t devId, const bt_bdaddr_t *addr)
{
#ifdef SASS_ENABLED
    if (NULL == gfps_sass_get_connected_dev(devId))
    {
        gfps_sass_connect_handler(devId, (bt_bdaddr_t *)addr);
    }
#endif
}

void gfps_link_disconnect_process(uint8_t devId, const bt_bdaddr_t *addr, uint8_t errCode)
{
    bool isDisconnectedWithMobile = false;
#if defined(IBRT) && !defined(FREEMAN_ENABLED_STERO)
    ibrt_link_type_e link_type = app_tws_ibrt_get_remote_link_type((bt_bdaddr_t *)addr);
    if (MOBILE_LINK == link_type)
    {
        isDisconnectedWithMobile = true;
    }
#else
    isDisconnectedWithMobile = true;
#endif

    if (isDisconnectedWithMobile)
    {
        if (gfps_is_last_response_pending())
        {
            gfps_enter_connectable_mode_req_handler(gfps_get_last_response());
        }
    }

#ifdef SASS_ENABLED
    gfps_sass_disconnect_handler(devId, addr, errCode);
#endif

    return;
}

void gfps_link_destroy_process(uint8_t devId)
{
#ifdef SASS_ENABLED
    gfps_sass_check_if_need_reconnect(devId);
#endif
}

void gfps_link_connection_event_process(uint8_t devId, GFPS_CONNECTION_EVENT *pEvent)
{
    if (!pEvent)
    {
        return;
    }

    switch(pEvent->event)
    {
        case GFPS_EVENT_LINK_CONNECTED:
            gfps_link_connect_process(devId, &(pEvent->addr));
            break;
        case GFPS_EVENT_LINK_DISCONNECTED:
            gfps_link_disconnect_process(devId, &(pEvent->addr), pEvent->reason);
            break;
        case GFPS_EVENT_LINK_DESTORY:
            gfps_link_destroy_process(devId);
            break;
        default:
            break;
    }
}

void gfps_link_connect_handler(uint8_t devId, const bt_bdaddr_t *addr)
{
    GFPS_CONNECTION_EVENT pEvent;
    if (addr)
    {
        memcpy(pEvent.addr.address, addr, sizeof(bt_bdaddr_t));
    }
    pEvent.event = GFPS_EVENT_LINK_CONNECTED;
    gfps_mailbox_put(devId, GFPS_EVENT_CONNECTION, (uint8_t *)&pEvent, sizeof(GFPS_CONNECTION_EVENT));
}

void gfps_link_disconnect_handler(uint8_t devId, const bt_bdaddr_t *addr, uint8_t errCode)
{
    GFPS_CONNECTION_EVENT pEvent;
    if (addr)
    {
        memcpy(pEvent.addr.address, addr, sizeof(bt_bdaddr_t));
    }
    else
    {
        memset(pEvent.addr.address, 0, sizeof(bt_bdaddr_t));
    }

    pEvent.reason = errCode;
    pEvent.event = GFPS_EVENT_LINK_DISCONNECTED;
    gfps_mailbox_put(devId, GFPS_EVENT_CONNECTION, (uint8_t *)&pEvent, sizeof(GFPS_CONNECTION_EVENT));
}

void gfps_link_destory_handler(void)
{
    GFPS_CONNECTION_EVENT pEvent;
    pEvent.event = GFPS_EVENT_LINK_DESTORY;
    gfps_mailbox_put(0xFF, GFPS_EVENT_CONNECTION, (uint8_t *)&pEvent, sizeof(GFPS_CONNECTION_EVENT));
}

void gfps_disconnect(uint8_t devId)
{
    if (IS_BT_DEVICE(devId))
    {
        app_fp_disconnect_rfcomm(GET_BT_ID(devId));
    }
#if BLE_AUDIO_ENABLED
    else
    {
        bes_ble_gfps_l2cap_disconnect(devId);
    }
#endif
}

void gfps_srv_event_rfcomm_process(uint8_t devId, GFPS_SRV_EVENT_RFCOM_PARAM_T *param)
{
    uint16_t consumeLen = 0;
    uint16_t dataLen;
    uint8_t *buf;
    TRACE(3, "%s, param->event:%d, devId is %d", __func__, param->event, devId);
    switch (param->event)
    {
        case FP_SRV_EVENT_CONNECTED:
            gfps_srv_connect_handler(devId, &(param->p.addr));
            break;

        case FP_SRV_EVENT_DISCONNECTED:
            gfps_srv_disconnect_handler(devId);
            break;

        case FP_SRV_EVENT_RFCOMM_DATA_IND:
            buf = param->p.data.pBuf;
            dataLen = param->p.data.len;
            while (app_fp_rfcomm_get_data_len(devId) > 0)
            {
                consumeLen = gfps_data_handler(devId, buf, dataLen);
                if (!consumeLen)
                {
                    break;
                }
                app_fp_rfcomm_data_done(devId, consumeLen, buf, &dataLen);
            }
            break;

        case FP_SRV_EVENT_SENT_DONE:
            break;

        default:
            break;
    }
    return;
}

void gfps_srv_event_l2cap_process(uint8_t devId, GFPS_SRV_EVENT_L2CAP_PARAM_T *param)
{
    uint16_t dataLen;
    uint8_t *buf;
    TRACE(3, "%s, param->event:%d, devId is %d", __func__, param->event, devId);
    switch (param->event)
    {
        case FP_SRV_EVENT_CONNECTED:
            gfps_srv_connect_handler(devId, &(param->p.addr));
            break;
    
        case FP_SRV_EVENT_DISCONNECTED:
            gfps_srv_disconnect_handler(devId);
            break;
        case FP_SRV_EVNET_L2CAP_DATA_IND:
            buf = param->p.data.pBuf;
            dataLen = param->p.data.len;
            gfps_data_handler(devId, buf, dataLen);
            if(buf[0] == FP_MSG_GROUP_DEVICE_ACTION && buf[1] == FP_MSG_DEVICE_ACTION_RING)
            {
                tws_ctrl_send_cmd(APP_TWS_CMD_SEND_GFPS_RING_INFO, buf, dataLen);
            }
            break;
    
        case FP_SRV_EVENT_SENT_DONE:
            break;
    
        default:
            break;
    }
    return;
}


uint16_t gfps_event_rfcomm_callback(uint8_t devId, GFPS_SRV_EVENT_RFCOM_PARAM_T *param)
{
    uint16_t ret = 0;
    gfps_mailbox_put(devId, GFPS_EVENT_FROM_RFCOMM_SEEKER, (uint8_t *)param, sizeof(GFPS_SRV_EVENT_RFCOM_PARAM_T));
    return ret;
}

uint16_t gfps_event_l2cap_callback(uint8_t devId, GFPS_SRV_EVENT_L2CAP_PARAM_T *param)
{
    uint16_t ret = 0;
    gfps_mailbox_put(devId, GFPS_EVENT_FROM_L2CAP_SEEKER, (uint8_t *)param, sizeof(GFPS_SRV_EVENT_L2CAP_PARAM_T));
    return ret;
}


void gfps_env_init(void)
{
    memset((uint8_t *)&gfpsEnv, 0, sizeof(GFPSEnv_t));
    gfpsEnv.batteryDataType = HIDE_UI_INDICATION;
#if defined(IBRT) && !defined(FREEMAN_ENABLED_STERO)
    gfpsEnv.isBatteryInfoIncluded = true;
#else
    gfpsEnv.isBatteryInfoIncluded = false;
#endif
    gfpsEnv.enterPairingMode = gfps_enter_pairing_mode_handler;
    gfpsEnv.btSetIocap = btif_sec_set_io_capabilities;
    gfpsEnv.btSetAuthrequirements = btif_sec_set_authrequirements;

    gfpsEnv.fpCap.env.isCompanionAppInstalled = false;
    gfpsEnv.fpCap.env.isSilentModeSupported   = false;
}

void gfps_init(void)
{
    nv_record_fp_account_key_init();

    gfps_env_init();

    gfps_crypto_init();
        
    gfps_get_key_info();
#ifndef IS_USE_CUSTOM_FP_INFO
    gfps_crypto_set_p256_key(ble_public_key, ble_private_key);
#else
    gfps_crypto_set_p256_key(gfps_get_public_key(),gfps_get_private_key());
#endif

#ifdef SASS_ENABLED
    gfps_sass_init();
#endif

    gfps_ble_init();

    app_fp_rfcomm_init();

    gfps_thread_init();
}

#if BLE_AUDIO_ENABLED
void gfps_lea_event_handler(void *param)
{
    uint8_t conidx = 0xFF;
    bool valid = true;
    GFPS_SASS_PROFILE_EVENT pEvent;
    AOB_EVENT_HEADER_T *header = (AOB_EVENT_HEADER_T *)param;
#ifdef SASS_ENABLED
    SassLeaParam *sassParam = (SassLeaParam *)pEvent.param;
#endif
    pEvent.pro = GFPS_PROFILE_LEA;
    ble_bdaddr_t GetPeerAddr = {{0}};

    if (param == NULL)
    {
        return;
    }
    TRACE(2, "%s type:%d", __func__, header->type);

    switch (header->type)
    {
        case AOB_EVENT_MOB_CONNECTION_STATE:
        {
            AOB_EVENT_MOB_STATE_T *connEvt = (AOB_EVENT_MOB_STATE_T *)header;
            valid = false;
            conidx = connEvt->conidx;
            TRACE(3, "%s d(%d) connection state:%d", __func__, conidx, connEvt->state.acl_state);
            DUMP8("%2x ", connEvt->peer_bdaddr.addr, 6);
            if (connEvt->state.acl_state == AOB_ACL_DISCONNECTED)
            {
                gfps_link_disconnect_handler(conidx, (bt_bdaddr_t *)connEvt->peer_bdaddr.addr, connEvt->state.err_code);
            }
            else if(connEvt->state.acl_state == AOB_ACL_CONNECTED)
            {
                gfps_link_connect_handler(conidx, (bt_bdaddr_t *)connEvt->peer_bdaddr.addr);

                memcpy(pEvent.btAddr.address, (bt_bdaddr_t *)connEvt->peer_bdaddr.addr, sizeof(bt_bdaddr_t));
            }
            else
            {
            }
        }
        break;

#ifdef SASS_ENABLED
        case AOB_EVENT_STREAM_STATUS_CHANGED:
        {
            AOB_EVENT_STREAM_STATUS_CHANGED_T *streamEvt = (AOB_EVENT_STREAM_STATUS_CHANGED_T *)header;
            if (streamEvt->state == AOB_MGR_STREAM_STATE_CODEC_CONFIGURED ||
                streamEvt->state == AOB_MGR_STREAM_STATE_QOS_CONFIGURED)
            {
                valid = false;
            }
            else
            {
                pEvent.btEvt = SASS_EVENT_LEA_STREAM_STATE;
                pEvent.len = sizeof(SassLeaParam);
                conidx = streamEvt->con_lid;
                sassParam->conLid = streamEvt->con_lid;
                sassParam->aseLid = streamEvt->ase_lid;
                sassParam->state = streamEvt->state;
            }
        }
        break;

        case AOB_EVENT_PLAYBACK_STATUS_CHANGED:
        {
            AOB_EVENT_PLAYBACK_STATUS_CHANGED_T *pbEvt = (AOB_EVENT_PLAYBACK_STATUS_CHANGED_T *)header;
            TRACE(2, "%s pbEvt->state:%d", __func__, pbEvt->state);
            if (pbEvt->state == AOB_MGR_PLAYBACK_STATE_PLAYING ||
                pbEvt->state == AOB_MGR_PLAYBACK_STATE_PAUSED ||
                pbEvt->state == AOB_MGR_PLAYBACK_STATE_INACTIVE)
            {
                pEvent.btEvt = SASS_EVENT_LEA_PLAYBACK_STATE;
                pEvent.len = sizeof(SassLeaParam);
                conidx = pbEvt->con_lid;
                sassParam->conLid = pbEvt->con_lid;
                sassParam->state = pbEvt->state;
            }
            else
            {
                 valid = false;
            }
        }
        break;

        case AOB_EVENT_CALL_STATE_CHANGE:
        {
            AOB_EVENT_CALL_STATE_CHANGE_T *pbEvt = (AOB_EVENT_CALL_STATE_CHANGE_T *)header;
            AOB_SINGLE_CALL_INFO_T *p_state_ind = (AOB_SINGLE_CALL_INFO_T *)pbEvt->param;
            pEvent.btEvt = SASS_EVENT_LEA_CALL_STATE;
            pEvent.len = sizeof(SassLeaParam);
            conidx = pbEvt->con_lid;
            sassParam->conLid = pbEvt->con_lid;
            sassParam->state = p_state_ind->state;
        }
        break;

        case AOB_EVENT_ASE_METADATA_UPDATA_IND:
        {
            AOB_EVENT_ASE_METADATA_UPDATA_IND_T *pbEvt= (AOB_EVENT_ASE_METADATA_UPDATA_IND_T *)header;

            pEvent.btEvt = SASS_EVENT_LEA_METADATA_UPDATA_STATE;
            pEvent.len = sizeof(SassLeaParam);
            conidx = pbEvt->con_lid;
            sassParam->conLid = pbEvt->con_lid;
            sassParam->state = pbEvt->state;
            sassParam->aseLid = pbEvt->ase_lid;
        }
        break;
#endif

        default:
            valid = false;
            break;
    }
    
    if (valid)
    {
        app_ble_get_peer_solved_addr(conidx, &GetPeerAddr);
        memcpy(pEvent.btAddr.address, GetPeerAddr.addr, sizeof(GetPeerAddr.addr));
        gfps_mailbox_put(conidx, GFPS_EVENT_PROFILE, (uint8_t *)&pEvent, sizeof(GFPS_SASS_PROFILE_EVENT));
    }

}
#endif

#endif
