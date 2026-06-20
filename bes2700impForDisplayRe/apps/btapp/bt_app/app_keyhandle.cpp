/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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

#include <stdio.h>
#include "cmsis.h"
#include "cmsis_os.h"
#include "app_audio_control.h"
#include "adapter_service.h"
#include "btapp.h"
#include "app_bt.h"
#include "app_hfp.h"
#include "app_keyhandle.h"
#include "app_key.h"
#include "audio_policy.h"
#include "app_media_player.h"
#include "app_bt_media_manager.h"
#if defined(IBRT)
#include "app_ibrt_internal.h"
#include "earbud_profiles_api.h"
#include "earbud_ux_api.h"
#endif

#if defined(BT_SOURCE)
#include "bt_source.h"
#endif

extern sco_preempted_time_t sco_preempted_time[BT_DEVICE_NUM];
#ifdef BT_HFP_SUPPORT

#ifdef SUPPORT_SIRI
int open_siri_flag = 0;
void bt_key_handle_siri_key(enum APP_KEY_EVENT_T event)
{
     switch(event)
     {
        case  APP_KEY_EVENT_NONE:
            if(open_siri_flag == 1){
                TRACE(0,"open siri");
                app_hfp_siri_voice(true);
                open_siri_flag = 0;
            } /*else {
                TRACE(0,"evnet none close siri");
                app_ibrt_if_enable_hfp_voice_assistant(false);
            }*/
            break;
        case  APP_KEY_EVENT_LONGLONGPRESS:
        case  APP_KEY_EVENT_UP:
            //TRACE(0,"long long/up/click event close siri");
            //app_ibrt_if_enable_hfp_voice_assistant(false);
            break;
        default:
            TRACE(1,"unregister down key event=%x",event);
            break;
        }
}
#endif

void hfcall_next_sta_handler(uint8_t device_id, hf_event_t event)
{
#if BT_DEVICE_NUM > 1
    btif_hf_channel_t* hf_channel_another = app_bt_get_device(app_bt_manager.hfp_key_handle_another_id)->hf_channel;
    btif_hf_channel_t* hf_channel_curr = app_bt_get_device(app_bt_manager.hfp_key_handle_curr_id)->hf_channel;
    TRACE(3, "(d%x) !!!hfcall_next_sta_handler curr/another_hfp_device %x %x hf_call_next_state %d\n",
        device_id, app_bt_manager.hfp_key_handle_curr_id, app_bt_manager.hfp_key_handle_another_id, app_bt_manager.hf_call_next_state);
    switch(app_bt_manager.hf_call_next_state)
    {
        case HFCALL_NEXT_STA_NULL:
            break;
        case HFCALL_NEXT_STA_ANOTHER_ANSWER:
            if(event == BTIF_HF_EVENT_AUDIO_DISCONNECTED)
            {
                TRACE(0,"NEXT_ACTION = HFP_ANSWER_ANOTHER_CALL\n");
                btif_hf_answer_call(hf_channel_another);
                app_bt_manager.hf_call_next_state = HFCALL_NEXT_STA_NULL;
            }
            break;
        case HFCALL_NEXT_STA_ANOTHER_ADDTOEARPHONE:
            if(event == BTIF_HF_EVENT_AUDIO_DISCONNECTED)
            {
                TRACE(0,"NEXT_ACTION = HFP_ANOTHER_ADDTOEARPHONE\n");
                btif_hf_create_audio_link(hf_channel_another);
                app_bt_manager.hf_call_next_state = HFCALL_NEXT_STA_NULL;
            }
            break;
        case HFCALL_NEXT_STA_CURR_ANSWER:
             TRACE(0,"NEXT_ACTION = HF_CURRENT_ANSWER\n");
             btif_hf_answer_call(hf_channel_curr);
             app_bt_manager.hf_call_next_state = HFCALL_NEXT_STA_NULL;
        break;
    }
#endif
}

#endif /* BT_HFP_SUPPORT */

#ifdef BT_AVRCP_SUPPORT

void app_bt_a2dp_send_set_abs_volume(uint8_t device_id, uint8_t volume)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (!curr_device)
    {
        return;
    }

    btif_avrcp_ct_set_absolute_volume(curr_device->avrcp_channel, volume);
}

#endif /* BT_AVRCP_SUPPORT */

#ifdef BT_TEST_CURRENT_KEY
extern "C" void hal_intersys_wakeup_btcore(void);
void bt_drv_i2v_enable_sleep_for_bt_access(void);
void bt_drv_accessmode_switch_test(void)
{
    static uint8_t access_mode=0;

    app_bt_set_access_mode(access_mode);

    if(access_mode ==0)
    {
        access_mode = 2;
    }
    else if(access_mode == 2)
    {
        access_mode= 3;
    }
    else if(access_mode == 3)
    {
        access_mode= 0;
    }
}
#endif


#if defined(BT_SOURCE)
void bt_key_handle_source_func_key(enum APP_KEY_EVENT_T event)
{
    uint8_t connected_source_count = 0;
    TRACE(2,"%s,%d",__FUNCTION__,event);
    switch(event)
    {
        case  APP_KEY_EVENT_UP:
        case  APP_KEY_EVENT_CLICK:
            connected_source_count = app_bt_source_count_connected_device();
            if (connected_source_count < BT_SOURCE_DEVICE_NUM)
            {
                app_bt_source_search_device();
            }
            break;
        case APP_KEY_EVENT_LONGPRESS:
#if defined(BT_HFP_AG_ROLE)
            app_hfp_ag_toggle_audio_link();
#endif
            break;
        case APP_KEY_EVENT_DOUBLECLICK:
            app_a2dp_source_toggle_stream(app_bt_source_get_current_a2dp());
            break;
        case APP_KEY_EVENT_TRIPLECLICK:
            app_a2dp_source_start_stream(app_bt_source_get_current_a2dp());
            break;
        default:
            TRACE(1,"unregister down key event=%x",event);
            break;
    }
}
#endif



HFCALL_MACHINE_ENUM app_get_hfcall_machine(void)
{
#ifdef BT_HFP_SUPPORT
    HFCALL_MACHINE_ENUM status = HFCALL_MACHINE_NUM;
    int current_device_id = app_bt_audio_get_hfp_device_for_user_action();
    struct BT_DEVICE_T* curr_device = app_bt_get_device(current_device_id);
    bt_hfp_call_setup_t    current_callSetup  = curr_device->hfchan_callSetup;
    bt_hfp_call_active_t   current_call       = curr_device->hfchan_call;
    bt_hfp_call_held_t current_callheld   = curr_device->hf_callheld;
    bt_audio_state_t      current_audioState = curr_device->hf_audio_state;

#if BT_DEVICE_NUM > 1
    int another_device_id = app_bt_audio_get_another_hfp_device_for_user_action(current_device_id);
    curr_device = app_bt_get_device(another_device_id);
    bt_hfp_call_setup_t    another_callSetup  = curr_device->hfchan_callSetup;
    bt_hfp_call_active_t   another_call       = curr_device->hfchan_call;
    bt_hfp_call_held_t another_callheld   = curr_device->hf_callheld;
    bt_audio_state_t      another_audioState = curr_device->hf_audio_state;
#endif

    app_bt_hfp_state_checker();

#if BT_DEVICE_NUM < 2
    // current AG is idle.
    if( current_callSetup ==BT_HFP_CALL_SETUP_NONE &&
        current_call == BT_HFP_CALL_NONE &&
        current_audioState == BT_HFP_AUDIO_DISCON )
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_IDLE!!!!");
        status = HFCALL_MACHINE_CURRENT_IDLE;
    }
    // current AG is incomming.
    else if( current_callSetup == BT_HFP_CALL_SETUP_IN &&
             current_call == BT_HFP_CALL_NONE)
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_INCOMMING!!!!");
        status = HFCALL_MACHINE_CURRENT_INCOMMING;
    }
    // current AG is outgoing.
    else if( (current_callSetup >= BT_HFP_CALL_SETUP_OUT) &&
        current_call == BT_HFP_CALL_NONE)
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_OUTGOING!!!!");
        status = HFCALL_MACHINE_CURRENT_OUTGOING;
    }
    // current AG is calling.
    else if( (current_callSetup ==BT_HFP_CALL_SETUP_NONE) &&
            current_call == BT_HFP_CALL_ACTIVE &&
            current_callheld != BT_HFP_CALL_HELD_ACTIVE)
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_CALLING!!!!");
        status = HFCALL_MACHINE_CURRENT_CALLING;
    }
    // current AG is 3way incomming.
    else if( current_callSetup ==BT_HFP_CALL_SETUP_IN &&
            current_call == BT_HFP_CALL_ACTIVE &&
            current_callheld == BT_HFP_CALL_HELD_NONE)
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_3WAY_INCOMMING!!!!");
        status = HFCALL_MACHINE_CURRENT_3WAY_INCOMMING;
    }
    // current AG is 3way hold calling.
    else if( current_callSetup ==BT_HFP_CALL_SETUP_NONE &&
            current_call == BT_HFP_CALL_ACTIVE &&
            current_callheld == BT_HFP_CALL_HELD_ACTIVE)
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING!!!!");
        status = HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING;
    }
    else
    {
        TRACE(0,"current hfcall machine status is not found!!!!!!");
    }
#else
    // current AG is idle , another AG is idle.
    if( current_callSetup ==BT_HFP_CALL_SETUP_NONE &&
        current_call == BT_HFP_CALL_NONE &&
        current_audioState == BT_HFP_AUDIO_DISCON &&
        another_callSetup==BT_HFP_CALL_SETUP_NONE &&
        another_call == BT_HFP_CALL_NONE &&
        another_audioState == BT_HFP_AUDIO_DISCON )
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_IDLE_ANOTHER_IDLE!!!!");
        status = HFCALL_MACHINE_CURRENT_IDLE_ANOTHER_IDLE;
    }
    // current AG is on incomming , another AG is idle.
    else if( current_callSetup == BT_HFP_CALL_SETUP_IN &&
             current_call == BT_HFP_CALL_NONE &&
             another_callSetup==BT_HFP_CALL_SETUP_NONE &&
             another_call == BT_HFP_CALL_NONE &&
             another_audioState == BT_HFP_AUDIO_DISCON  )
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_IDLE!!!!");
        status = HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_IDLE;
    }
    // current AG is on outgoing , another AG is idle.
    else if( current_callSetup >= BT_HFP_CALL_SETUP_OUT &&
            current_call == BT_HFP_CALL_NONE &&
            another_callSetup == BT_HFP_CALL_SETUP_NONE &&
            another_call == BT_HFP_CALL_NONE &&
            another_audioState == BT_HFP_AUDIO_DISCON  )
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_OUTGOING_ANOTHER_IDLE!!!!");
        status = HFCALL_MACHINE_CURRENT_OUTGOING_ANOTHER_IDLE;
    }
    // current AG is on calling , another AG is idle.
    else if( current_callSetup == BT_HFP_CALL_SETUP_NONE &&
            current_call == BT_HFP_CALL_ACTIVE &&
            current_callheld != BT_HFP_CALL_HELD_ACTIVE&&
            another_callSetup == BT_HFP_CALL_SETUP_NONE &&
            another_call == BT_HFP_CALL_NONE &&
            another_audioState == BT_HFP_AUDIO_DISCON  )
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_IDLE!!!!");
        status = HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_IDLE;
    }
    // current AG is 3way incomming , another AG is idle.
    else if( current_callSetup ==BT_HFP_CALL_SETUP_IN &&
            current_call == BT_HFP_CALL_ACTIVE &&
            current_callheld == BT_HFP_CALL_HELD_NONE&&
            another_callSetup == BT_HFP_CALL_SETUP_NONE &&
            another_call == BT_HFP_CALL_NONE &&
            another_audioState == BT_HFP_AUDIO_DISCON   )
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_3WAY_INCOMMING_ANOTHER_IDLE!!!!");
        status = HFCALL_MACHINE_CURRENT_3WAY_INCOMMING_ANOTHER_IDLE;
    }
    // current AG is 3way hold calling , another AG is without connecting.
    else if( current_callSetup ==BT_HFP_CALL_SETUP_NONE &&
            current_call == BT_HFP_CALL_ACTIVE &&
            current_callheld == BT_HFP_CALL_HELD_ACTIVE &&
            another_callSetup == BT_HFP_CALL_SETUP_NONE &&
            another_call == BT_HFP_CALL_NONE &&
            another_audioState == BT_HFP_AUDIO_DISCON   )
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING_ANOTHER_IDLE!!!!");
        status = HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING_ANOTHER_IDLE;
    }
    else if(current_callSetup ==BT_HFP_CALL_SETUP_NONE &&
            current_call == BT_HFP_CALL_ACTIVE &&
            current_callheld == BT_HFP_CALL_HELD_NO_ACTIVE &&
            another_callSetup == BT_HFP_CALL_SETUP_NONE &&
            another_call == BT_HFP_CALL_NONE &&
            another_audioState == BT_HFP_AUDIO_DISCON)
    {
        TRACE(0,"current hfcall machine status is HFCALL_MECHINE_CURRENT_3WAY_HOLD_ANOTHER_IDLE!!!!");
        status = HFCALL_MECHINE_CURRENT_3WAY_HOLD_ANOTHER_IDLE;
    }
    // current AG is incomming , another AG is incomming too.
    else if( current_callSetup == BT_HFP_CALL_SETUP_IN &&
            current_call == BT_HFP_CALL_NONE &&
            current_callheld == BT_HFP_CALL_HELD_NONE&&
            another_callSetup == BT_HFP_CALL_SETUP_IN &&
            another_call == BT_HFP_CALL_NONE &&
            another_callheld == BT_HFP_CALL_HELD_NONE)
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_INCOMMING!!!!");
        status = HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_INCOMMING;
    }
    // current AG is ingcomming , another AG is outgoing too.
    else if( current_callSetup == BT_HFP_CALL_SETUP_IN &&
            current_call == BT_HFP_CALL_NONE &&
            current_callheld == BT_HFP_CALL_HELD_NONE&&
            another_callSetup >= BT_HFP_CALL_SETUP_OUT &&
            another_call == BT_HFP_CALL_NONE &&
            another_callheld == BT_HFP_CALL_HELD_NONE)
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_OUTGOING!!!!");
        status = HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_OUTGOING;
    }
	// current AG is calling , another AG is incomming.(just for PC case)
	else if(current_callSetup == BT_HFP_CALL_SETUP_IN &&
            current_call == BT_HFP_CALL_NONE &&
            current_callheld == BT_HFP_CALL_HELD_NONE &&
            another_call == BT_HFP_CALL_NONE &&
            another_callSetup == BT_HFP_CALL_SETUP_NONE &&
            another_audioState == BT_HFP_AUDIO_CON)
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_INCOMMING!!!!");
        status = HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_CALLING;
    }
    // current AG is on calling , another AG calling changed to phone.
    else if( current_callSetup == BT_HFP_CALL_SETUP_NONE &&
            current_call == BT_HFP_CALL_ACTIVE &&
            current_callheld == BT_HFP_CALL_HELD_NONE&&
            another_callSetup == BT_HFP_CALL_SETUP_NONE &&
            another_call == BT_HFP_CALL_ACTIVE &&
            another_callheld == BT_HFP_CALL_HELD_NONE&&
            another_audioState == BT_HFP_AUDIO_DISCON)
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_CHANGETOPHONE!!!!");
        status = HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_CHANGETOPHONE;
    }
    // current AG is on calling , another AG calling is hold.
    else if( current_callSetup == BT_HFP_CALL_SETUP_NONE &&
            current_call == BT_HFP_CALL_ACTIVE &&
            current_callheld == BT_HFP_CALL_HELD_NONE&&
            another_callSetup == BT_HFP_CALL_SETUP_NONE &&
            another_call == BT_HFP_CALL_ACTIVE &&
            another_callheld == BT_HFP_CALL_HELD_ACTIVE)
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_HOLD!!!!");
        status = HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_HOLD;
    }
    //current AG is on incoming,another AG is calling.
    else if(current_callSetup == BT_HFP_CALL_SETUP_IN &&
            current_call == BT_HFP_CALL_NONE &&
            current_callheld == BT_HFP_CALL_HELD_NONE &&
            another_callSetup == BT_HFP_CALL_SETUP_NONE &&
            another_call == BT_HFP_CALL_ACTIVE &&
            another_callheld == BT_HFP_CALL_HELD_NONE)
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_INCOMING_ANOTHER_CALLING!!!!");
        status = HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_CALLING;
    }
    else
    {
        TRACE(0,"current hfcall machine status is not found!!!!!!");
    }
#endif
    TRACE(0,"%s status is %d",__func__,status);
    return status;
#else
    return HFCALL_MACHINE_CURRENT_IDLE;
#endif /* BT_HFP_SUPPORT */
}


void app_bt_a2dp_send_key_request(uint8_t device_id, uint8_t a2dp_key)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (!curr_device)
    {
        return;
    }

    if(!curr_device->a2dp_conn_flag)
    {
        TRACE(1, "%s a2dp is not connected", __func__);
        return;
    }

    if (!btif_avrcp_is_control_channel_connected(curr_device->avrcp_channel))
    {
        TRACE(2, "%s avrcp_key %d avrcp is not connected", __func__, a2dp_key);
        return;
    }

#ifdef IBRT_UI
//ydz    app_ibrt_conn_rs_task_set(RS_TASK_AVRCP_KEY, 2000);
#endif

    switch(a2dp_key)
    {
        case AVRCP_KEY_STOP:
            TRACE(2, "(d%x) avrcp_key = AVRCP_KEY_STOP %p", device_id, __builtin_return_address(0));
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_STOP,TRUE);
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_STOP,FALSE);
            curr_device->a2dp_play_pause_flag = 0;
            break;
        case AVRCP_KEY_PLAY:
            TRACE(2, "(d%x) avrcp_key = AVRCP_KEY_PLAY press %p", device_id, __builtin_return_address(0));
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_PLAY,TRUE);
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_PLAY,FALSE);
#if defined(A2DP_STREAM_DETECT_NO_DECODE )
            curr_device->stream_detect.mute_sample_count = 0;
#endif // A2DP_STREAM_DETECT_NO_DECODE
            curr_device->a2dp_play_pause_flag = 1;
            break;
        case AVRCP_KEY_PAUSE:
            TRACE(2, "(d%x) avrcp_key = AVRCP_KEY_PAUSE press %p", device_id, __builtin_return_address(0));
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_PAUSE,TRUE);
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_PAUSE,FALSE);
#if defined(A2DP_STREAM_DETECT_NO_DECODE )
            curr_device->stream_detect.normal_sample_count = 0;
#endif // A2DP_STREAM_DETECT_NO_DECODE
            curr_device->a2dp_play_pause_flag = 0;
            break;
        case AVRCP_KEY_FORWARD:
            TRACE(2, "(d%x) avrcp_key = AVRCP_KEY_FORWARD %p", device_id, __builtin_return_address(0));
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_FORWARD,TRUE);
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_FORWARD,FALSE);
            curr_device->a2dp_play_pause_flag = 1;
            break;
        case AVRCP_KEY_BACKWARD:
            TRACE(2, "(d%x) avrcp_key = AVRCP_KEY_BACKWARD %p", device_id, __builtin_return_address(0));
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_BACKWARD,TRUE);
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_BACKWARD,FALSE);
            curr_device->a2dp_play_pause_flag = 1;
            break;
        case AVRCP_KEY_VOLUME_UP:
            TRACE(2, "(d%x) avrcp_key = AVRCP_KEY_VOLUME_UP %p", device_id, __builtin_return_address(0));
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_VOLUME_UP,TRUE);
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_VOLUME_UP,FALSE);
            break;
        case AVRCP_KEY_VOLUME_DOWN:
            TRACE(2, "(d%x) avrcp_key = AVRCP_KEY_VOLUME_DOWN %p", device_id, __builtin_return_address(0));
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_VOLUME_DOWN,TRUE);
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_VOLUME_DOWN,FALSE);
            break;
        case AVRCP_KEY_FAST_FORWARD_START:
            TRACE(2, "(d%x) avrcp_key = AVRCP_KEY_FAST_FORWARD %p", device_id, __builtin_return_address(0));
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_FAST_FORWARD,TRUE);
            break;
        case AVRCP_KEY_FAST_FORWARD_STOP:
            TRACE(2, "(d%x) avrcp_key = AVRCP_KEY_FAST_FORWARD_STOP %p", device_id, __builtin_return_address(0));
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_FAST_FORWARD,FALSE);
            break;
        case AVRCP_KEY_REWIND_START:
            TRACE(2, "(d%x) avrcp_key = AVRCP_KEY_REWIND_START %p", device_id, __builtin_return_address(0));
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_REWIND,TRUE);
            break;
        case AVRCP_KEY_REWIND_STOP:
            TRACE(2, "(d%x) avrcp_key = AVRCP_KEY_REWIND_STOP %p", device_id, __builtin_return_address(0));
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_REWIND,FALSE);
            break;
        default :
            break;
    }
}


extern void a2dp_handleKey(uint8_t a2dp_key)
{
    uint8_t a2dp_id = app_bt_audio_get_curr_a2dp_device();
    struct BT_DEVICE_T* curr_device = NULL;

    TRACE(2,"!!!a2dp_handleKey curr a2dp device %x last_paused_device %x", a2dp_id, app_bt_manager.a2dp_last_paused_device);

    if (a2dp_key == AVRCP_KEY_PLAY && app_bt_manager.a2dp_last_paused_device != BT_DEVICE_INVALID_ID)
    {
        if (app_bt_get_device(app_bt_manager.a2dp_last_paused_device)->a2dp_conn_flag)
        {
            a2dp_id = app_bt_manager.a2dp_last_paused_device; // only select last paused device when it still connected
        }
    }

    curr_device = app_bt_get_device(a2dp_id);

    if(!curr_device->a2dp_conn_flag)
    {
        TRACE(0,"a2dp is not connected:a2dp_conn_flag=0");
        return;
    }

    if (!btif_avrcp_is_control_channel_connected(curr_device->avrcp_channel))
    {
        TRACE(1, "avrcp_key %d the channel is not connected", a2dp_key);
        return;
    }

    if (a2dp_key == AVRCP_KEY_PAUSE)
    {
        app_bt_manager.a2dp_last_paused_device = a2dp_id;
    }
    else
    {
        app_bt_manager.a2dp_last_paused_device = BT_DEVICE_INVALID_ID;
    }

    app_bt_a2dp_send_key_request(a2dp_id, a2dp_key);
}


void hfp_handle_key(uint8_t hfp_key)
{
#ifdef BT_HFP_SUPPORT
    app_bt_manager.hfp_key_handle_curr_id = app_bt_audio_get_hfp_device_for_user_action();
    btif_hf_channel_t* hf_channel_curr = app_bt_get_device(app_bt_manager.hfp_key_handle_curr_id)->hf_channel;
#if BT_DEVICE_NUM > 1
    app_bt_manager.hfp_key_handle_another_id = app_bt_audio_get_another_hfp_device_for_user_action(app_bt_manager.hfp_key_handle_curr_id);
    btif_hf_channel_t* hf_channel_another = app_bt_get_device(app_bt_manager.hfp_key_handle_another_id)->hf_channel;
#endif

    switch(hfp_key)
    {
        case HFP_KEY_ANSWER_CALL:
            ///answer a incomming call
            TRACE(0,"avrcp_key = HFP_KEY_ANSWER_CALL\n");
            btif_hf_answer_call(hf_channel_curr);
            break;
        case HFP_KEY_HANGUP_CALL:
            TRACE(0,"avrcp_key = HFP_KEY_HANGUP_CALL\n");
            btif_hf_hang_up_call(hf_channel_curr);
            break;
        case HFP_KEY_REDIAL_LAST_CALL:
            ///redail the last call
            TRACE(0,"avrcp_key = HFP_KEY_REDIAL_LAST_CALL\n");
            btif_hf_redial_call(hf_channel_curr);
            break;
        case HFP_KEY_CHANGE_TO_PHONE:
            ///remove sco and voice change to phone
            if(app_bt_is_hfp_audio_on())
            {
                TRACE(0,"avrcp_key = HFP_KEY_CHANGE_TO_PHONE\n");
                btif_hf_disc_audio_link(hf_channel_curr);
            }
            break;
        case HFP_KEY_ADD_TO_EARPHONE:
            ///add a sco and voice change to earphone
            if(!app_bt_is_hfp_audio_on())
            {
                TRACE(0,"avrcp_key = HFP_KEY_ADD_TO_EARPHONE ver:%x\n",  btif_hf_get_version(hf_channel_curr));
                btif_hf_create_audio_link(hf_channel_curr);
            }
            break;
        case HFP_KEY_MUTE:
            TRACE(0,"avrcp_key = HFP_KEY_MUTE\n");
            app_bt_manager.hf_tx_mute_flag = 1;
            break;
        case HFP_KEY_CLEAR_MUTE:
            TRACE(0,"avrcp_key = HFP_KEY_CLEAR_MUTE\n");
            app_bt_manager.hf_tx_mute_flag = 0;
            break;
        case HFP_KEY_THREEWAY_HOLD_AND_ANSWER:
            TRACE(0,"avrcp_key = HFP_KEY_THREEWAY_HOLD_AND_ANSWER\n");
            btif_hf_call_hold(hf_channel_curr, BTIF_HF_HOLD_HOLD_ACTIVE_CALLS, 0);
            break;
        case HFP_KEY_THREEWAY_HANGUP_AND_ANSWER:
            TRACE(0,"avrcp_key = HFP_KEY_THREEWAY_HOLD_SWAP_ANSWER\n");
            btif_hf_call_hold(hf_channel_curr, BTIF_HF_HOLD_RELEASE_ACTIVE_CALLS, 0);
            break;
        case HFP_KEY_THREEWAY_HOLD_REL_INCOMING:
            TRACE(0,"avrcp_key = HFP_KEY_THREEWAY_HOLD_REL_INCOMING\n");
            btif_hf_call_hold(hf_channel_curr, BTIF_HF_HOLD_RELEASE_HELD_CALLS, 0);
            break;
#if BT_DEVICE_NUM > 1
        case HFP_KEY_DUAL_HF_HANGUP_ANOTHER:
            TRACE(0,"avrcp_key = HFP_KEY_DUAL_HF_HANGUP_ANOTHER\n");
            btif_hf_hang_up_call(hf_channel_another);
            break;
        case HFP_KEY_DUAL_HF_HANGUP_CURR_ANSWER_ANOTHER:
            TRACE(0,"avrcp_key = HFP_KEY_DUAL_HF_HANGUP_CURR_ANSWER_ANOTHER\n");
            btif_hf_hang_up_call(hf_channel_curr);
            app_bt_manager.hf_call_next_state = HFCALL_NEXT_STA_ANOTHER_ANSWER;
            break;
        case HFP_KEY_DUAL_HF_HOLD_CURR_ANSWER_ANOTHER:
            TRACE(0,"avrcp_key = HFP_KEY_DUAL_HF_HOLD_CURR_ANSWER_ANOTHER\n");
            break;
        case HFP_KEY_DUAL_HF_CHANGETOPHONE_ANSWER_ANOTHER:
            TRACE(0,"avrcp_key = HFP_KEY_DUAL_HF_CHANGETOPHONE_ANSWER_ANOTHER\n");
            btif_hf_disc_audio_link(hf_channel_curr);
            app_bt_manager.hf_call_next_state = HFCALL_NEXT_STA_ANOTHER_ANSWER;
            break;
        case HFP_KEY_DUAL_HF_CHANGETOPHONE_ANOTHER_ADDTOEARPHONE:
            TRACE(0,"avrcp_key = HFP_KEY_DUAL_HF_CHANGETOPHONE_ANOTHER_ADDTOEARPHONE\n");
            btif_hf_disc_audio_link(hf_channel_curr);
            app_bt_manager.hf_call_next_state = HFCALL_NEXT_STA_ANOTHER_ADDTOEARPHONE;
            break;
        case HFP_KEY_DUAL_HF_HANGUP_ANOTHER_ADDTOEARPHONE:
            TRACE(0,"avrcp_key = HFP_KEY_DUAL_HF_HANGUP_ANOTHER_ADDTOEARPHONE\n");
            btif_hf_hang_up_call(hf_channel_curr);
            app_bt_manager.hf_call_next_state = HFCALL_NEXT_STA_ANOTHER_ADDTOEARPHONE;
            break;
        case HFP_KEY_DUAL_HF_CHANGETOPHONE_ANSWER_CURR:
            TRACE(0,"avrcp_key = HFP_KEY_DUAL_HF_CHANGETOPHONE_ANSWER_CURR\n");
            btif_hf_disc_audio_link(hf_channel_another);
            if (app_bt_manager.config.reject_sco_req_within_a_certain_time_after_it_be_preempted)
            {
                sco_preempted_time[app_bt_manager.hfp_key_handle_another_id].time_valid = true;
                sco_preempted_time[app_bt_manager.hfp_key_handle_another_id].time_of_sco_preempted = hal_sys_timer_get();
            }
            app_bt_manager.hf_call_next_state = HFCALL_NEXT_STA_CURR_ANSWER;
            break;
#endif
        default :
            break;
    }
#endif /* BT_HFP_SUPPORT */
}

void bes_bt_a2dp_key_handler(uint8_t a2dp_key)
{
#ifndef BLE_ONLY_ENABLED
    a2dp_handleKey(a2dp_key);
#endif
}

void bes_bt_hfp_key_handler(uint8_t hfp_key)
{
#ifndef BLE_ONLY_ENABLED
    hfp_handle_key(hfp_key);
#endif
}

void bt_key_handle_customer_volume(void)
{
#if defined(IBRT)
    if(app_ibrt_if_is_nv_master())
    {
        app_audio_control_streaming_volume_up();
    }
    else
    {
        app_audio_control_streaming_volume_down();
    }
#endif
}

void app_key_click_handle_bt_func(void)
{
    HFCALL_MACHINE_ENUM hfcall_machine = app_get_hfcall_machine();
    POSSIBLY_UNUSED struct BT_DEVICE_T* a2dp_device = app_bt_get_device(app_bt_audio_get_curr_a2dp_device());
    switch(hfcall_machine)
    {
#ifdef BT_AVRCP_SUPPORT
        case HFCALL_MACHINE_CURRENT_IDLE:
        {
            if(a2dp_device && (a2dp_device->a2dp_play_pause_flag == 0)){
                a2dp_handleKey(AVRCP_KEY_PLAY);
            }else{
                a2dp_handleKey(AVRCP_KEY_PAUSE);
            }
        }
        break;
#if BT_DEVICE_NUM > 1
        case HFCALL_MACHINE_CURRENT_IDLE_ANOTHER_IDLE:
        {
            if(a2dp_device && (a2dp_device->a2dp_play_pause_flag == 0)){
                a2dp_handleKey(AVRCP_KEY_PLAY);
            }else{
                a2dp_handleKey(AVRCP_KEY_PAUSE);
            }
        }
        break;
#endif
#endif /* BT_AVRCP_SUPPORT */
#ifdef BT_HFP_SUPPORT
        case HFCALL_MACHINE_CURRENT_INCOMMING:
           hfp_handle_key(HFP_KEY_ANSWER_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_OUTGOING:
            hfp_handle_key(HFP_KEY_HANGUP_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_CALLING:
            hfp_handle_key(HFP_KEY_HANGUP_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_INCOMMING:
            hfp_handle_key(HFP_KEY_THREEWAY_HANGUP_AND_ANSWER);
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING:
            hfp_handle_key(HFP_KEY_THREEWAY_HOLD_AND_ANSWER);
        break;
#if BT_DEVICE_NUM > 1
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_IDLE:
            hfp_handle_key(HFP_KEY_ANSWER_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_OUTGOING_ANOTHER_IDLE:
            hfp_handle_key(HFP_KEY_HANGUP_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_IDLE:
            hfp_handle_key(HFP_KEY_HANGUP_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_INCOMMING_ANOTHER_IDLE:
            hfp_handle_key(HFP_KEY_THREEWAY_HANGUP_AND_ANSWER);
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING_ANOTHER_IDLE:
            hfp_handle_key(HFP_KEY_THREEWAY_HOLD_AND_ANSWER);
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_INCOMMING:
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_OUTGOING:
        break;
        case HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_CHANGETOPHONE:
            hfp_handle_key(HFP_KEY_DUAL_HF_HANGUP_ANOTHER_ADDTOEARPHONE);
        break;
        case HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_HOLD:
            hfp_handle_key(HFP_KEY_DUAL_HF_HANGUP_CURR_ANSWER_ANOTHER);
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_CALLING:
            hfp_handle_key(HFP_KEY_DUAL_HF_CHANGETOPHONE_ANSWER_CURR);
        break;
        case HFCALL_MECHINE_CURRENT_3WAY_HOLD_ANOTHER_IDLE:
            hfp_handle_key(HFP_KEY_THREEWAY_HANGUP_AND_ANSWER);
        break;
#endif
#endif /* BT_HFP_SUPPORT */
        default:
        break;
    }
}

void app_key_doubleclick_handle_bt_func(void)
{
    TRACE(0,"%s enter",__func__);

    HFCALL_MACHINE_ENUM hfcall_machine = app_get_hfcall_machine();

#ifdef SUPPORT_SIRI
    open_siri_flag=0;
#endif

    switch(hfcall_machine)
    {
        case HFCALL_MACHINE_CURRENT_IDLE:
#ifdef BT_HID_DEVICE
            app_bt_hid_send_capture();
#else
            bt_key_handle_customer_doubleclick();
#endif
        break;
#ifdef BT_HFP_SUPPORT
        case HFCALL_MACHINE_CURRENT_INCOMMING:
            bt_key_handle_call(CALL_STATE_INCOMING);
        break;
        case HFCALL_MACHINE_CURRENT_OUTGOING:
            bt_key_handle_call(CALL_STATE_OUTGOING);
        break;
        case HFCALL_MACHINE_CURRENT_CALLING:
        case HFCALL_MACHINE_CURRENT_3WAY_INCOMMING:
            bt_key_handle_call(CALL_STATE_ACTIVE);
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING:
        break;
#if BT_DEVICE_NUM > 1
        case HFCALL_MACHINE_CURRENT_IDLE_ANOTHER_IDLE:
#ifdef BT_HID_DEVICE
            app_bt_hid_send_capture();
#else
            bt_key_handle_customer_doubleclick();
#endif
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_IDLE:
        break;
        case HFCALL_MACHINE_CURRENT_OUTGOING_ANOTHER_IDLE:
        break;
        case HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_IDLE:
            if(app_bt_manager.hf_tx_mute_flag == 0){
                hfp_handle_key(HFP_KEY_MUTE);
            }else{
                hfp_handle_key(HFP_KEY_CLEAR_MUTE);
            }
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_INCOMMING_ANOTHER_IDLE:
            hfp_handle_key(HFP_KEY_THREEWAY_HOLD_REL_INCOMING);
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING_ANOTHER_IDLE:
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_INCOMMING:
            hfp_handle_key(HFP_KEY_ANSWER_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_OUTGOING:
        break;
        case HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_CHANGETOPHONE:
        break;
        case HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_HOLD:
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_CALLING:
            hfp_handle_key(HFP_KEY_ANSWER_CALL);
        break;
#endif
#endif /* BT_HFP_SUPPORT */
        default:
        break;
    }
}

void app_key_tripleclick_handle_bt_func(void)
{
    TRACE(0,"%s enter",__func__);
    CALL_STATE_E call_state = app_bt_get_call_state();
    PLAYBACK_INFO_T* playback_info = app_bt_get_music_playback_info();

    if(call_state == CALL_STATE_IDLE)
    {
        switch(playback_info->playback_status)
        {
            case IDLE:
            case PAUSED:
                app_hfp_siri_voice(true);
                break;
            case PLAYING:
                {
                    app_audio_control_media_pause();
                    app_hfp_siri_voice(true);
                }
                break;
            default:
                break;
        }
    }
}

void app_key_longpress_handle_bt_func(void)
{
    TRACE(0,"%s enter",__func__);
    HFCALL_MACHINE_ENUM hfcall_machine = app_get_hfcall_machine();
#ifdef SUPPORT_SIRI
    open_siri_flag=0;
#endif
#ifndef FPGA
    media_PlayAudio(AUD_ID_BT_WARNING, 0);
#endif

    switch(hfcall_machine)
    {
        case HFCALL_MACHINE_CURRENT_IDLE:
        {
            bt_key_handle_customer_volume();
#ifdef BT_PBAP_SUPPORT
            app_bt_pbap_client_test();
#endif
#ifdef BT_MAP_SUPPORT
            bt_map_client_test(&app_bt_get_device(BT_DEVICE_ID_1)->remote);
#endif
#if HF_CUSTOM_FEATURE_SUPPORT & HF_CUSTOM_FEATURE_SIRI_REPORT
            if(open_siri_flag == 0 )
            {
#ifndef FPGA
                media_PlayAudio(AUD_ID_BT_WARNING, 0);
#endif
                open_siri_flag = 1;
            }
#endif
        }
        break;
#ifdef BT_HFP_SUPPORT
        case HFCALL_MACHINE_CURRENT_INCOMMING:
            hfp_handle_key(HFP_KEY_HANGUP_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_OUTGOING:
        break;
        case HFCALL_MACHINE_CURRENT_CALLING:
        {
            uint8_t device_id = app_bt_audio_get_curr_hfp_device();
            struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
            if(curr_device->switch_sco_to_earbud) {
                //call is active, switch from phone to earphone
                hfp_handle_key(HFP_KEY_ADD_TO_EARPHONE);
            } else {
                //call is active, switch from earphone to phone
                hfp_handle_key(HFP_KEY_CHANGE_TO_PHONE);
            }
        }
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_INCOMMING:
        {
#ifndef FPGA
#ifdef MEDIA_PLAYER_SUPPORT
            audio_player_play_prompt(AUD_ID_BT_WARNING, 0);
#endif
#endif
            hfp_handle_key(HFP_KEY_THREEWAY_HOLD_AND_ANSWER);
        }
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING:
            hfp_handle_key(HFP_KEY_THREEWAY_HANGUP_AND_ANSWER);
        break;
#if BT_DEVICE_NUM > 1
        case HFCALL_MACHINE_CURRENT_IDLE_ANOTHER_IDLE:
        {
                bt_key_handle_customer_volume();
        }
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_IDLE:
            hfp_handle_key(HFP_KEY_HANGUP_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_OUTGOING_ANOTHER_IDLE:
        break;
        case HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_IDLE:
        {
            uint8_t device_id = app_bt_audio_get_curr_hfp_device();
            struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
            if(curr_device->switch_sco_to_earbud) {
                //call is active, switch from phone to earphone
                hfp_handle_key(HFP_KEY_ADD_TO_EARPHONE);
            } else {
                //call is active, switch from earphone to phone
                hfp_handle_key(HFP_KEY_CHANGE_TO_PHONE);
            }
        }
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_INCOMMING_ANOTHER_IDLE:
            hfp_handle_key(HFP_KEY_THREEWAY_HOLD_AND_ANSWER);
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING_ANOTHER_IDLE:
            hfp_handle_key(HFP_KEY_THREEWAY_HANGUP_AND_ANSWER);
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_INCOMMING:
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_OUTGOING:
        break;
        case HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_CHANGETOPHONE:
            hfp_handle_key(HFP_KEY_DUAL_HF_CHANGETOPHONE_ANOTHER_ADDTOEARPHONE);
        break;
        case HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_HOLD:
            hfp_handle_key(HFP_KEY_DUAL_HF_HANGUP_CURR_ANSWER_ANOTHER);
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_CALLING:
            hfp_handle_key(HFP_KEY_HANGUP_CALL);
        break;
#endif
#endif /* BT_HFP_SUPPORT */
        default:
        break;
    }
}

static const bta_key_click_event_cb_t app_key_click_event_cb =
{
    .key_click_func                         = app_key_click_handle_bt_func,
    .key_doubleclick_func                   = app_key_doubleclick_handle_bt_func,
    .key_tripleclick_func                   = app_key_tripleclick_handle_bt_func,
    .key_longpress_func                     = app_key_longpress_handle_bt_func,
};

void app_key_handle_init(void)
{
    bt_adapter_register_key_evt_handle_cb(&app_key_click_event_cb);
}
