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
#include "hal_timer.h"
#include "hsm.h"

#include "walkie_talkie_dbg.h"
#include "app_walkie_talkie_key_handler.h"
#include "walkie_talkie_config.h"
#include "walkie_talkie.h"
#include "app_walkie_talkie_half_duplex.h"

#include "app_bt_media_manager.h"
/*
 *  Demo version
*/

#ifdef BLE_WALKIE_TALKIE_HALF_DUPLEX

typedef enum
{
    WT_SM_TALK_BTN_DOWN,
    WT_SM_TALK_BTN_UP,
    WT_SM_GAP_INIT_DONE,
}HALF_DUP_WT_EVENT_T;

typedef struct
{
    Hsm walkie_talkie_sm;
    State talking;
    State receive;

    bool streaming_state;
} app_walkie_talkie_sm_t;

static app_walkie_talkie_sm_t app_walkie_talkie_sm;

static const char* app_walkie_talkie_event_to_str(Event event)
{
    switch(event) {
    case WT_SM_TALK_BTN_DOWN:
        return "BTN DOWN";
    case WT_SM_TALK_BTN_UP:
        return "BTN UP";
    case WT_SM_GAP_INIT_DONE:
        return "GAP_INIT_DONE";
    case START_EVT:
        return "Start";
    case ENTRY_EVT:
        return "Entry";
    case EXIT_EVT:
        return "Exit";
    default:
        return "Unknow Event";
    }
}

Msg const* app_walkie_talkie_super_state(app_walkie_talkie_sm_t *me, Msg *msg)
{
    LOG_I("W-T-HD-SM:init state on event %s",app_walkie_talkie_event_to_str(msg->evt));
    switch (msg->evt)
    {
        case START_EVT:
            return 0;
        case ENTRY_EVT:
            return 0;
        case WT_SM_GAP_INIT_DONE:
             STATE_START(me, &me->receive);
            return 0;
        case EXIT_EVT:
            return 0;
    }

    return msg;
}

Msg const* app_walkie_talkie_talking_state(app_walkie_talkie_sm_t *me, Msg *msg)
{
    LOG_I("W-T-HD-SM:talking state on event %s",app_walkie_talkie_event_to_str(msg->evt));
    switch (msg->evt)
    {
        case START_EVT:
            return 0;
        case ENTRY_EVT:
            walkie_talkie_start_capture();
            return 0;
        case WT_TALK_BTN_DOWN:
            return 0;
        case WT_TALK_BTN_UP:
            STATE_TRAN(me, &me->receive);
            walkie_talkie_stop_capture();
            return 0;
        case EXIT_EVT:
            return 0;
    }

    return msg;
}

static bool app_walkie_talkie_allow_to_speak(app_walkie_talkie_sm_t *me)
{
    if (me->streaming_state)
    {
        return false;
    }

    return true;
}

Msg const* app_walkie_talkie_receive_state(app_walkie_talkie_sm_t *me, Msg *msg)
{
    LOG_I("W-T-HD-SM:receive state on event %s",app_walkie_talkie_event_to_str(msg->evt));

    switch (msg->evt)
    {
        case START_EVT:
            return 0;
        case ENTRY_EVT:
            walkie_talkie_start_recv_data();
            return 0;
        case WT_TALK_BTN_DOWN:
            if (app_walkie_talkie_allow_to_speak(me))
            {
                STATE_TRAN(me, &me->talking);
            }
            return 0;
        case WT_TALK_BTN_UP:
            return 0;
        case EXIT_EVT:
            walkie_talkie_stop_recv_data();
            return 0;
    }

    return msg;
}

static void app_walkie_talkie_half_dup_handle_event(HALF_DUP_WT_EVENT_T event)
{
    Msg message;

    message.evt = event;
    message.param0 = 0;
    message.param1 = 0;
    message.param2 = 0;
    HsmOnEvent((Hsm *)&app_walkie_talkie_sm, &message);
}

void app_walkie_talkie_half_dup_handle_key_event(APP_W_T_KEY_EVENT_E event)
{
    if (WT_TALK_BTN_DOWN == event)
    {
        app_walkie_talkie_half_dup_handle_event(WT_SM_TALK_BTN_DOWN);
    }
    else if (WT_TALK_BTN_UP == event)
    {
        app_walkie_talkie_half_dup_handle_event(WT_SM_TALK_BTN_UP);
    }
}

static void app_walkie_talkie_event_callback(WALKIE_TAKIE_SRV_EVENT_T event)
{
    if (event == AUDIO_STREAMING_START)
    {
        app_walkie_talkie_sm.streaming_state = true;
    }
    else if(event == AUDIO_STREAMING_STOP)
    {
        app_walkie_talkie_sm.streaming_state = false;
    }
    else if (event == BLE_GAP_INIT_DONE)
    {
        app_walkie_talkie_half_dup_handle_event(WT_SM_GAP_INIT_DONE);
    }
}

POSSIBLY_UNUSED static void app_walkie_talkie_sm_ctor(app_walkie_talkie_sm_t *me)
{
    LOG_I("W-T-HD-SM: sm ctor = %p",me);
    HsmCtor((Hsm *)me, "super", (EvtHndlr)app_walkie_talkie_super_state);
      AddState(&me->talking, "talking", &((Hsm *)me)->top,(EvtHndlr)app_walkie_talkie_talking_state);
      AddState(&me->receive, "receive", &((Hsm *)me)->top, (EvtHndlr)app_walkie_talkie_receive_state);

    me->streaming_state = false;
}

void app_walkie_talkie_half_dup_init(const walkie_bdaddr* filter_addr_list,uint8_t list_size,uint8_t work_mode)
{
    wakie_talke_register_event_call_back(app_walkie_talkie_event_callback);

    app_walkie_talkie_sm_ctor(&app_walkie_talkie_sm);
    HsmOnStart((Hsm *)&app_walkie_talkie_sm);

    walkie_talkie_srv_init(filter_addr_list,list_size,work_mode);
}

#endif /* BLE_WALKIE_TALKIE_HALF_DUPLEX */
