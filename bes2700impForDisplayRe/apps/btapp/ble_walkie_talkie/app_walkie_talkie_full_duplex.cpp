/**
 *
 * @copyright Copyright (c) 2015-2022 BES Technic.
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
 */
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hsm.h"

#include "walkie_talkie_dbg.h"
#include "app_walkie_talkie_key_handler.h"
#include "walkie_talkie_ble_gap.h"
#include "walkie_talkie_config.h"
#include "walkie_talkie.h"
#include "app_walkie_talkie_full_duplex.h"
#include "walkie_talkie_build_network.h"
#include "string.h"

#ifdef BLE_WALKIE_TALKIE_FULL_DUPLEX

#define SEARCH_DEVICE_TIME_OUT   (3500)
#define BUILD_NETWORK_TIME       (3000)

typedef enum
{
    WT_SM_BTN_DOWN,
    WT_SM_BTN_UP,
    WT_SM_PA_SYNC_ESTB,
    WT_SM_PA_SYNC_LOSS,
    WT_SM_GAP_INIT_DONE,
    WT_LOOK_FOR_DEVS_TO,
    WT_RECV_AUDIO_START,
    WT_RECV_AUDIO_STOP,
    WT_NET_BUILDING,
    WT_WHITE_COMPLETE,
    WT_NET_COMPLETE,               // 10
    WT_NET_QUIT,
    WT_NOTIFY_OTHER_DEV,
    WT_AUDIO_START,
    WT_ALLOW_SPEAKING,
    WT_NOT_ALLOW_SPEAKING,       // 15
    WT_SM_START_EXIT,            // Start to free walkie talkie resource
    WT_SM_SRV_DESTORY_CMP,       // All walkie talkie resource relase complete
}WALKIE_TALKIE_SM_EVT_T;

typedef struct
{
    Hsm walkie_talkie_dup_sm;
    State look_for_network;
    State full_dup_work;
    State build_network;
    State wait_destory_network;
    State idle;

    osTimerId           look_for_devices_timer_id;
    osTimerDefEx_t      look_for_devices_timer_def;

    osTimerId           build_network_timer_id;
    osTimerDefEx_t      build_network_timer_def;

    void *evt_mutex;
    WALKIE_TALKIE_SM_EVT_T      next_evt;

    const walkie_full_dup_event_cb* state_change_cb;

} walkie_talkie_full_dup_sm_t;

osMutexDef(full_dup_evt_mutex);
static walkie_talkie_full_dup_sm_t walkie_talkie_dup_sm;

static const char* app_wt_full_dup_event_to_string(int event)
{
    #define CASES(s) case s:return "["#s"]";

    switch(event)
    {
        CASES(AUDIO_STREAMING_START)
        CASES(AUDIO_STREAMING_STOP)
        CASES(PA_SYNC_ESTB)
        CASES(PA_SYNC_LOSS)
        CASES(BLE_GAP_INIT_DONE)
        CASES(NET_BUILDING)
        CASES(NET_COMPLETE)
        CASES(NET_QUIT)
        CASES(WHITE_COMPLETE)
        CASES(NOTIFY_OTHER_DEV)
        CASES(AUDIO_ALLOW_SPKING)
        CASES(AUDIO_NOT_ALLOW_SPKING)
        CASES(WT_SRV_DESTORY_CMP)
    }

    return "[UNDEFINE EVENT]";
}

Msg const* app_walkie_talkie_full_dup_super_state(walkie_talkie_full_dup_sm_t *me, Msg *msg)
{
    LOG_I("W-T-FD-SM::Top state on event %d",(msg->evt));
    switch (msg->evt)
    {
        case START_EVT:
            return 0;
        case ENTRY_EVT:
            STATE_START(me, &me->build_network);
            return 0;
        case WT_SM_GAP_INIT_DONE:
            return 0;
        case WT_SM_START_EXIT:
             STATE_TRAN(me, &me->wait_destory_network);
            return 0;
        case EXIT_EVT:
            return 0;
    }

    return msg;
}

Msg const* app_walkie_talkie_build_network_state(walkie_talkie_full_dup_sm_t *me, Msg *msg)
{
    LOG_I("W-T-FD-SM::build network state on event %d",(msg->evt));
    switch (msg->evt)
    {
        case START_EVT:
            return 0;
        case ENTRY_EVT:
            return 0;
        case WT_NET_BUILDING:
           if(wt_network_get_status()->network_status == WALKIE_TALKIE_BUILDING)
            {
                walkie_scan_start();
                osTimerStop(me->build_network_timer_id);
                osTimerStart(me->build_network_timer_id, 500);
                walkie_talkie_dup_sm.next_evt = WT_NOTIFY_OTHER_DEV;
            }
            return 0;
        case WT_NOTIFY_OTHER_DEV:
            {
                uint8_t adv_data[10] = {0};
                uint8_t len =0;
                if(wt_network_get_status()->reason == WT_KEY_BUILD_NETWORK)
                {
                    len = wt_white_list_get_event_data(WT_KEY_BUILD_EVT, adv_data, NULL, NULL);
                }
                else
                {
                    len = wt_white_list_get_event_data(WT_BUILD_EVT, adv_data, NULL, NULL);
                }
                walkie_talkie_send_data(WALKIE_GAP_DATA_DEV_INFO, adv_data, len, wt_network_get_status()->reason);
            }
            return 0;
        case WT_WHITE_COMPLETE:
            {
                uint8_t adv_data[(WALKIE_DEVICE_MAX+1) * 6 + 10] = {0};
                uint8_t len = 0;
                const uint8_t *local_addr = walkie_get_local_ble_addr();
                uint16_t scan_delay = 200;
                uint8_t supplement_data_len = 2;
                uint8_t supplement_data[] = {1,1};
                if(wt_network_get_status()->reason == WT_CONFIRM_NETWORK &&
                    memcmp(wt_network_get_status()->network_leader, local_addr, 6) == 0)
                {
                    len = wt_white_list_get_event_data(WT_CONFIRM_EVT, adv_data, (uint8_t*)&supplement_data_len, supplement_data);
                    walkie_talkie_send_data(WALKIE_GAP_DATA_DEV_INFO, adv_data, len, WT_CONFIRM_EVT);
                    TRACE(0, "W-T-FD-SM: conrfirm adv:");
                    DUMP8("%x ", adv_data, len);
                    scan_delay = 1000;
                }
                osTimerStop(me->build_network_timer_id);
                osTimerStart(me->build_network_timer_id, scan_delay);
                walkie_talkie_dup_sm.next_evt = WT_NET_COMPLETE;
            }
            return 0;
        case WT_NET_COMPLETE:
            STATE_TRAN(me, &me->look_for_network);
            return 0;
        case WT_NET_QUIT:
            osTimerStop(me->build_network_timer_id);
            return 0;
        case WT_AUDIO_START:
            return 0;
        case EXIT_EVT:
            return 0;
    }

    return msg;
}

Msg const* app_walkie_talkie_look_for_network_state(walkie_talkie_full_dup_sm_t *me, Msg *msg)
{
    LOG_I("W-T-FD-SM::Look for network state on event %d",(msg->evt));
    switch (msg->evt)
    {
        case START_EVT:
            return 0;
        case ENTRY_EVT:
            if(wt_network_get_status()->network_status == WALKIE_TALKIE_NET_ACTIVE)
            {
                walkie_scan_start();
            }
            osTimerStop(me->look_for_devices_timer_id);
            osTimerStart(me->look_for_devices_timer_id,SEARCH_DEVICE_TIME_OUT);
            return 0;
        case WT_LOOK_FOR_DEVS_TO:
            STATE_TRAN(me, &me->full_dup_work);
            return 0;
        case WT_SM_PA_SYNC_ESTB:
            walkie_talkie_dup_sm.state_change_cb->wt_find_device(msg->param0,(uint8_t*)msg->param1);
            osTimerStop(me->look_for_devices_timer_id);
            osTimerStart(me->look_for_devices_timer_id,SEARCH_DEVICE_TIME_OUT);
            return 0;
        case WT_NET_QUIT:
        case WT_NET_BUILDING:
        case WT_WHITE_COMPLETE:
        case WT_NET_COMPLETE:
            osTimerStop(me->look_for_devices_timer_id);
            STATE_TRAN(me, &me->build_network);
            return 0;
        case WT_AUDIO_START:
            return 0;
        case EXIT_EVT:
            return 0;
    }

    return msg;
}

Msg const* app_walkie_talkie_full_dup_state(walkie_talkie_full_dup_sm_t *me, Msg *msg)
{
    LOG_I("W-T-FD-SM:full duplex state on event %d",msg->evt);

    switch (msg->evt)
    {
        case START_EVT:
            return 0;
        case ENTRY_EVT:
            if(wt_network_get_status()->network_status == WALKIE_TALKIE_NET_ACTIVE)
            {
                walkie_talkie_send_data(WALKIE_GAP_DATA_DEV_INFO, NULL, 0,WT_NONE_EVT);
                walkie_talkie_start_capture();
                walkie_talkie_dup_sm.state_change_cb->wt_ready_to_send_data();
            }
            return 0;
        case WT_TALK_BTN_DOWN:
        /* press down to start talk in default */
            if (walkie_talkie_get_neighbor_count() > 0)
            {
                walkie_talkie_start_capture();
            }
            return 0;
        case WT_TALK_BTN_UP:
         /* press down to stop talk */
            walkie_talkie_stop_capture();
            return 0;
         case WT_SM_PA_SYNC_ESTB:
            walkie_talkie_dup_sm.state_change_cb->wt_find_device(msg->param0,(uint8_t*)msg->param1);
            return 0;
        case WT_SM_PA_SYNC_LOSS:
            walkie_talkie_dup_sm.state_change_cb->wt_device_loss(msg->param0,(uint8_t*)msg->param1);
            return 0;
        case WT_RECV_AUDIO_START:
            walkie_talkie_dup_sm.state_change_cb->wt_device_is_stalking(msg->param0,(uint8_t*)msg->param1);
            return 0;
        case WT_RECV_AUDIO_STOP:
            walkie_talkie_dup_sm.state_change_cb->wt_device_end_stalking(msg->param0,(uint8_t*)msg->param1);
            return 0;
        case WT_ALLOW_SPEAKING:
            walkie_talkie_dup_sm.state_change_cb->wt_allow_speaking();
            return 0;
        case WT_NOT_ALLOW_SPEAKING:
            walkie_talkie_dup_sm.state_change_cb->wt_not_allow_speaking();
            return 0;
        case WT_NET_QUIT:
            STATE_TRAN(me, &me->build_network);
            return 0;
        case WT_AUDIO_START:
            return 0 ;
        case EXIT_EVT:
            return 0;
    }

    return msg;
}

Msg const* app_wt_wait_desotry_network_state(walkie_talkie_full_dup_sm_t *me, Msg *msg)
{
    LOG_I("W-T-FD-SM:wait destory network on event %d",msg->evt);

    switch (msg->evt)
    {
        case START_EVT:
            return 0;
        case ENTRY_EVT:
            walkie_talkie_srv_deinit();
            return 0;
        case WT_SM_SRV_DESTORY_CMP:
             STATE_TRAN(me, &me->idle);
             return 0;
        case EXIT_EVT:
            return 0;
    }

    return msg;
}

Msg const* app_wt_idle_state(walkie_talkie_full_dup_sm_t *me, Msg *msg)
{
    LOG_I("W-T-FD-SM:idle on event %d",msg->evt);
    switch (msg->evt)
    {
        case START_EVT:
            return 0;
        case ENTRY_EVT:
            return 0;
        case EXIT_EVT:
            return 0;
    }

    return msg;
}

static void app_walkie_talkie_full_dup_sm_handle_event(
                WALKIE_TALKIE_SM_EVT_T event,uint8_t device_id = 0,uint8_t *addr = NULL)
{
    Msg message;

    osMutexWait((osMutexId)walkie_talkie_dup_sm.evt_mutex, osWaitForever);
    message.evt = event;
    message.param0 = device_id;
    message.param1 = (uint32_t)addr;
    message.param2 = 0;
    HsmOnEvent((Hsm *)&walkie_talkie_dup_sm, &message);
    osMutexRelease((osMutexId)walkie_talkie_dup_sm.evt_mutex);
}

void app_walkie_talkie_full_dup_handle_key_event(APP_W_T_KEY_EVENT_E event)
{
    if (WT_TALK_BTN_DOWN == event)
    {
        app_walkie_talkie_full_dup_sm_handle_event(WT_SM_BTN_DOWN);
    }
    else if (WT_TALK_BTN_UP == event)
    {
        app_walkie_talkie_full_dup_sm_handle_event(WT_SM_BTN_UP);
    }
}

static void app_walkie_talkie_full_dup_event_callback(
                    WALKIE_TAKIE_SRV_EVENT_T event,uint8_t device_id,uint8_t *addr)
{
    TRACE(0, "W-T-FD-SM:recv event %s", app_wt_full_dup_event_to_string(event));
    switch (event)
    {
        case AUDIO_STREAMING_START:
            app_walkie_talkie_full_dup_sm_handle_event(WT_RECV_AUDIO_START,device_id,addr);
            break;
        case AUDIO_STREAMING_STOP:
            app_walkie_talkie_full_dup_sm_handle_event(WT_RECV_AUDIO_STOP,device_id,addr);
            break;
        case PA_SYNC_ESTB:
            app_walkie_talkie_full_dup_sm_handle_event(WT_SM_PA_SYNC_ESTB,device_id,addr);
            break;
        case PA_SYNC_LOSS:
            app_walkie_talkie_full_dup_sm_handle_event(WT_SM_PA_SYNC_LOSS,device_id,addr);
            break;
        case BLE_GAP_INIT_DONE:
            app_walkie_talkie_full_dup_sm_handle_event(WT_SM_GAP_INIT_DONE);
            break;
        case NET_BUILDING:
            app_walkie_talkie_full_dup_sm_handle_event(WT_NET_BUILDING);
            break;
        case WHITE_COMPLETE:
            app_walkie_talkie_full_dup_sm_handle_event(WT_WHITE_COMPLETE);
            break;
        case NET_COMPLETE:
            app_walkie_talkie_full_dup_sm_handle_event(WT_NET_COMPLETE);
            break;
        case NOTIFY_OTHER_DEV:
            app_walkie_talkie_full_dup_sm_handle_event(WT_NOTIFY_OTHER_DEV);
            break;
        case NET_QUIT:
            app_walkie_talkie_full_dup_sm_handle_event(WT_NET_QUIT);
            break;
        case AUDIO_ALLOW_SPKING:
            app_walkie_talkie_full_dup_sm_handle_event(WT_ALLOW_SPEAKING);
            break;
        case AUDIO_NOT_ALLOW_SPKING:
            app_walkie_talkie_full_dup_sm_handle_event(WT_NOT_ALLOW_SPEAKING);
            break;
        case WT_SRV_DESTORY_CMP:
            app_walkie_talkie_full_dup_sm_handle_event(WT_SM_SRV_DESTORY_CMP);
            break;
    }
}

void app_wt_full_dup_start_exit()
{
    app_walkie_talkie_full_dup_sm_handle_event(WT_SM_START_EXIT);
}

POSSIBLY_UNUSED static void app_walkie_talkie_create_full_dup_sm(walkie_talkie_full_dup_sm_t *me)
{
    HsmCtor((Hsm *)me, "super", (EvtHndlr)app_walkie_talkie_full_dup_super_state);
    AddState(&me->build_network, "build_net", &((Hsm *)me)->top, (EvtHndlr)app_walkie_talkie_build_network_state);
    AddState(&me->look_for_network, "look_for_network", &((Hsm *)me)->top,(EvtHndlr)app_walkie_talkie_look_for_network_state);
    AddState(&me->full_dup_work, "full_dup", &((Hsm *)me)->top, (EvtHndlr)app_walkie_talkie_full_dup_state);
    AddState(&me->wait_destory_network,"destory_network", &((Hsm *)me)->top, (EvtHndlr)app_wt_wait_desotry_network_state);
    AddState(&me->idle,"idle", &((Hsm *)me)->top, (EvtHndlr)app_wt_idle_state);

    //
}

void app_walkie_talkie_search_devices_timeout_cb(void const *ctx)
{
    LOG_I("W-T-FD-SM:Look for device timeout");
    app_walkie_talkie_full_dup_sm_handle_event(WT_LOOK_FOR_DEVS_TO);
}

void app_walkie_talkie_build_net_cb(void const *ctx)
{
    const walkie_talkie_full_dup_sm_t *sm = (walkie_talkie_full_dup_sm_t *)ctx;
    WALKIE_TALKIE_SM_EVT_T evt = sm->next_evt;
    LOG_I("W-T-FD-SM:build network %d", evt);
    app_walkie_talkie_full_dup_sm_handle_event(evt);
}

void app_walkie_talkie_full_dup_init(const walkie_bdaddr* filter_addr_list,uint8_t list_size,uint8_t work_mode)
{
    wakie_talke_register_event_call_back(app_walkie_talkie_full_dup_event_callback);

    osTimerInit(walkie_talkie_dup_sm.look_for_devices_timer_def,app_walkie_talkie_search_devices_timeout_cb);
    walkie_talkie_dup_sm.look_for_devices_timer_id = \
        osTimerCreate(&walkie_talkie_dup_sm.look_for_devices_timer_def.os_timer_def,osTimerOnce, &walkie_talkie_dup_sm);

    osTimerInit(walkie_talkie_dup_sm.build_network_timer_def,app_walkie_talkie_build_net_cb);
    walkie_talkie_dup_sm.build_network_timer_id = \
        osTimerCreate(&walkie_talkie_dup_sm.build_network_timer_def.os_timer_def,osTimerOnce, &walkie_talkie_dup_sm);

    if (walkie_talkie_dup_sm.evt_mutex == NULL)
    {
        walkie_talkie_dup_sm.evt_mutex = osMutexCreate((osMutex(full_dup_evt_mutex)));
    }
    app_walkie_talkie_create_full_dup_sm(&walkie_talkie_dup_sm);
    HsmOnStart((Hsm *)&walkie_talkie_dup_sm);

    walkie_talkie_srv_init(filter_addr_list,list_size,work_mode);
}

void app_wt_full_dup_reg_state_changed_callback(const walkie_full_dup_event_cb* cbs)
{
    walkie_talkie_dup_sm.state_change_cb = cbs;
}

#endif /* BLE_WALKIE_TALKIE_HALF_DUPLEX */
