
/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#include "bluetooth_bt_api.h"
#include "hal_trace.h"
#include "ble_audio_define.h"
#include "app_audio_focus_control.h"
#include "aob_media_api.h"
#include "aob_call_api.h"
#include "aob_bis_api.h"
#include "aob_volume_api.h"
#include "gaf_media_stream.h"
#include "aob_stream_handler.h"
#include "app_audio_bt_device.h"
#include "app_audio_active_device_manager.h"
#include "app_bt_media_manager.h"
#include "apps.h"
#include "app_media_player.h"
#include "ble_audio_earphone_info.h"
#include "bt_drv_reg_op.h"
#include "app_audio_control.h"
#include "audioflinger.h"
#include "gaf_media_stream.h"
#include "gaf_bis_media_stream.h"
#include "app_audio.h"
#include "btapp.h"
#if defined(IBRT)
#include "app_ibrt_tws_ext_cmd.h"
#endif
#include "bes_gap_api.h"
#include "app_bap_data_path_itf.h"
#include "ble_tws.h"
#include "app_ble.h"
#include "bt_callback_func.h"

#define BLE_AUDIO_PLAY_RING_TONE_INTERVAL    (3000)
#define BLE_AUDIO_PLAY_STATUS_CHECK_TIME     (1000)
typedef enum
{
    STREAMMING_MUSIC    = 0x01,
    STREAMMING_CALL     = 0x02,
    STREAMMING_GAME     = 0x04,
    STREAMMING_INVALID  = 0xFF
}streaming_available_context_t;
static SWITCH_FOCUS_INFO_T switch_focus_info ={{0}};
typedef struct
{
    ble_bdaddr_t address;
    uint8_t con_lid;
    uint8_t cis_sink_ase_lid;
    uint8_t cis_src_ase_lid;
    uint8_t bis_sink_stream_lid;
    int8_t music_audio_focus;
    int8_t call_audio_focus;
    int8_t ring_audio_focus;
    int8_t game_audio_focus;
    bool set_as_bg_device;
    streaming_available_context_t streamming_context;
    bool bis_streaming_available;
    AOB_MGR_PLAYBACK_STATE_E media_play_status;
    osTimerDefEx_t ring_tone_play_timer_def;
    osTimerId ring_tone_play_timer;
    osTimerDefEx_t playback_status_check_timer_def;
    osTimerId playback_status_check_timer;
    uint32_t current_context_type;

    on_audio_focus_change_listener music_focus_changed_listener;
    on_audio_focus_change_listener call_focus_changed_listener;
    on_audio_focus_change_listener game_focus_changed_listener;
    on_audio_focus_change_listener ring_focus_changed_listener;

    on_audio_focus_resume_callback audio_focus_resume_callabck;
} BLE_AUDIO_SINK_DEVICE_T;

static BLE_AUDIO_SINK_DEVICE_T ble_audio_sink_device[AOB_COMMON_MOBILE_CONNECTION_MAX];
static BLE_AUDIO_POLICY_CONFIG_T ble_audio_policy_config;
static void(*cis_switch_cmp_cb)(uint8_t device_id);
static void(*gaf_media_status_handler_cb)(uint8_t con_lid, bool paused);
uint8_t curr_playing_le_id = INVALID_BLE_CONIDX;

BLE_AUDIO_SINK_DEVICE_T *app_ble_audio_get_device(uint8_t con_lid)
{
    if (con_lid < AOB_COMMON_MOBILE_CONNECTION_MAX)
    {
        return ble_audio_sink_device + con_lid;
    }

    return NULL;
}

void app_ble_audio_sink_stream_start(uint8_t conlid)
{
    BLE_AUDIO_SINK_DEVICE_T *ble_device = app_ble_audio_get_device(conlid);
    if (ble_device->current_context_type == GAF_AUDIO_STREAM_CONTEXT_TYPE_BIS)
    {
#ifdef APP_BLE_BIS_SINK_ENABLE
        ///for bis frist param is stream_lid
        gaf_bis_audio_stream_start_handler(0);
#endif
    }
    else
    {
        gaf_audio_stream_start(conlid);
    }
}

void app_ble_audio_sink_stream_stop(uint8_t conlid)
{
    BLE_AUDIO_SINK_DEVICE_T *ble_device = app_ble_audio_get_device(conlid);
    if (ble_device->current_context_type == GAF_AUDIO_STREAM_CONTEXT_TYPE_BIS)
    {
#ifdef APP_BLE_BIS_SINK_ENABLE
        ///for bis frist param is stream_lid
        gaf_bis_audio_stream_stop_handler(0);
#endif
    }
    else
    {
        gaf_audio_stream_stop(conlid);
    }
}

void app_ble_audio_policy_config(void)
{
    ble_audio_policy_config.mute_new_music_stream = false;
    ble_audio_policy_config.pause_new_music_stream = true;
}

BLE_AUDIO_POLICY_CONFIG_T *app_ble_audio_get_policy_config(void)
{
    return &ble_audio_policy_config;
}

static void app_ble_set_ble_active_link(BLE_AUDIO_SINK_DEVICE_T *device)
{
    uint8_t streaming_ase_lid = device->cis_sink_ase_lid;
    if(streaming_ase_lid == 0xFF)
    {
        streaming_ase_lid = device->cis_src_ase_lid;
        if (streaming_ase_lid == 0XFF)
        {
            TRACE(0, "cant find streaming ase_id");
            return;
        }
    }

    app_bap_ascs_ase_t *p_bap_ase_info = app_bap_uc_srv_get_ase_info(streaming_ase_lid);

    if (p_bap_ase_info == NULL)
    {
        TRACE(0, "p_bap_ase_info NULL ase_id %d", streaming_ase_lid);
        return;
    }
    app_audio_policy_set_bt_ble_active_link(CIS_MODE, p_bap_ase_info->cis_hdl);

}

static int app_ble_audio_music_ext_policy(focus_device_info_t *cdevice_info, focus_device_info_t *rdevice_info)
{
    BLE_AUDIO_SINK_DEVICE_T *device = app_ble_audio_get_device(cdevice_info->device_idx);
#ifdef APP_SOUND_ENABLE
    if(rdevice_info->stream_type == USAGE_MEDIA &&
       device->current_context_type == GAF_AUDIO_STREAM_CONTEXT_TYPE_BIS)
    {
        TRACE(0, "lea_music_ext: new music play!");
        return AUDIOFOCUS_REQUEST_GRANTED;
    }
#else
    if (rdevice_info->stream_type == USAGE_MEDIA)
    {
        TRACE(0, "lea_music_ext: new music delay play!");
        return AUDIOFOCUS_REQUEST_DELAYED;
    }
#endif

    if (rdevice_info->audio_type == AUDIO_TYPE_BT
        && (rdevice_info->stream_type == USAGE_CALL || rdevice_info->stream_type == USAGE_RINGTONE))
    {
        TRACE(0, "lea_music_ext: call grant!");
        return AUDIOFOCUS_REQUEST_GRANTED;
    }

    if ((rdevice_info->stream_type == USAGE_MEDIA) && (device->current_context_type != GAF_AUDIO_STREAM_CONTEXT_TYPE_BIS))
    {
        TRACE(0, "[%s][%d]:dont allow new music play!", __FUNCTION__, __LINE__);
        return AUDIOFOCUS_REQUEST_FAILED;
    }

    return AUDIOFOCUS_REQUEST_GRANTED;
}

static int app_ble_audio_flexible_ext_policy(focus_device_info_t* cdevice_info, focus_device_info_t *rdevice_info)
{
    if (rdevice_info->stream_type == USAGE_MEDIA)
    {
        TRACE(0, "[%s][%d]: new music delay!", __FUNCTION__, __LINE__);
        // Allow request music play
        return AUDIOFOCUS_REQUEST_DELAYED;
    }
    else if (rdevice_info->stream_type == USAGE_FLEXIBLE)
    {
#ifdef BLE_USB_AUDIO_SUPPORT
        /*
         * Allow new media play if upload streaming is not exist.
        */
        uint8_t startedStreamTypes = 0;
        BLE_AUDIO_SINK_DEVICE_T *device = app_ble_audio_get_device(cdevice_info->device_idx);
        if (device->cis_src_ase_lid == 0xFF)
        {
            TRACE(0, "unconfigured src ase id,allow new streaming play ");
            return AUDIOFOCUS_REQUEST_GRANTED;
        }
        startedStreamTypes = gaf_audio_get_stream_started_type(device->con_lid);
        if ((startedStreamTypes & GAF_AUDIO_STREAM_TYPE_CAPTURE) == 0)
        {
            TRACE(0, "no upstreaming, accept new streaming");
            return AUDIOFOCUS_REQUEST_GRANTED;
        }
        else
        {
            return AUDIOFOCUS_REQUEST_FAILED;
        }
#else
        return AUDIOFOCUS_REQUEST_GRANTED;
#endif
    }

#ifdef SASS_ENABLED
    if (rdevice_info->audio_type == AUDIO_TYPE_BT && rdevice_info->stream_type == USAGE_CALL)
    {
        TRACE(0, "bt call can prompt le flexible");
        return AUDIOFOCUS_REQUEST_GRANTED;
    }
#endif

    return AUDIOFOCUS_REQUEST_FAILED;
}

static int app_ble_audio_call_ext_policy(focus_device_info_t* cdevice_info, focus_device_info_t *rdevice_info)
{
    if (rdevice_info->stream_type == USAGE_MEDIA)
    {
        TRACE(0, "ble_call_ext: new music delay!");
        // Allow request music play
        return AUDIOFOCUS_REQUEST_DELAYED;
    }
    else if(rdevice_info->stream_type == USAGE_CALL)
    {
        TRACE(0, "ble_call_ext:new call preempt!");
        return AUDIOFOCUS_REQUEST_GRANTED;
    }
    else if (rdevice_info->stream_type == USAGE_FLEXIBLE)
    {
#ifdef BLE_USB_AUDIO_SUPPORT
        /*
         * Allow new media play if upload streaming is not exist.
        */
        uint8_t startedStreamTypes = 0;
        BLE_AUDIO_SINK_DEVICE_T *device = app_ble_audio_get_device(cdevice_info->device_idx);
        if (device->cis_src_ase_lid == 0xFF)
        {
            TRACE(0, "unconfigured src ase id,allow new streaming play ");
            return AUDIOFOCUS_REQUEST_GRANTED;
        }
        startedStreamTypes = gaf_audio_get_stream_started_type(device->con_lid);
        if ((startedStreamTypes & GAF_AUDIO_STREAM_TYPE_CAPTURE) == 0)
        {
            TRACE(0, "no upstreaming, accept new streaming");
            return AUDIOFOCUS_REQUEST_GRANTED;
        }
        else
        {
            return AUDIOFOCUS_REQUEST_FAILED;
        }
#else
        return AUDIOFOCUS_REQUEST_GRANTED;
#endif
    }

#ifdef SASS_ENABLED
    if (rdevice_info->audio_type == AUDIO_TYPE_BT && rdevice_info->stream_type == USAGE_CALL)
    {
        TRACE(0, "bt call cant prompt le flexible");
        return AUDIOFOCUS_REQUEST_FAILED;
    }
#endif

    return AUDIOFOCUS_REQUEST_FAILED;
}

static int app_ble_audio_ringtone_stream_ext_policy(focus_device_info_t *cdevice_info, focus_device_info_t *rdevice_info)
{
    if (rdevice_info->stream_type == USAGE_CALL)
    {
        return AUDIOFOCUS_REQUEST_GRANTED;
    }
    return AUDIOFOCUS_REQUEST_FAILED;
}

static uint16_t app_ble_audio_get_curr_device_streaming_context_bf(uint8_t con_lid)
{
    uint8_t nb_ase = 0;
    uint8_t idx = 0;
    uint16_t streaming_context = 0;
    uint8_t ase_lid_list[APP_BAP_DFT_ASCS_NB_ASE_CHAR] = {0};
    app_bap_ascs_ase_t *p_ase_info_temp = NULL;
    nb_ase = aob_media_get_ready_for_stream_ase_lid_list(con_lid, ase_lid_list);

    for (idx = 0; idx < nb_ase; idx++)
    {
        p_ase_info_temp = app_bap_uc_srv_get_ase_info(ase_lid_list[idx]);
        streaming_context |= p_ase_info_temp->init_context_bf;
        TRACE(3, "%s ase_lid: %d check context 0x%x", __func__, ase_lid_list[idx], p_ase_info_temp->init_context_bf);
    }

    return streaming_context;
}

static AUDIO_STATUS_DEVICE_EVNET app_ble_audio_get_device_stream_event(BLE_AUDIO_SINK_DEVICE_T *device)
{
    AUDIO_STATUS_DEVICE_EVNET stream_event = AUDIO_STATUS_DEVICE_IDLE;
    uint8_t call_active_id = AOB_COMMON_INVALID_CALL_ID;
    if(device->current_context_type == GAF_AUDIO_STREAM_CONTEXT_TYPE_BIS && device->streamming_context == STREAMMING_MUSIC)
    {
        return AUDIO_STATUS_DEVICE_BIS_MUSIC;
    }

    uint16_t streaming_context_type = app_ble_audio_get_curr_device_streaming_context_bf(device->con_lid);
    switch (device->streamming_context)
    {
        case STREAMMING_MUSIC:
            if (streaming_context_type & AOB_AUDIO_CONTEXT_TYPE_MEDIA)
            {
               stream_event = AUDIO_STATUS_DEVICE_CIS_MUSIC;
            }
            else if(streaming_context_type & AOB_AUDIO_CONTEXT_TYPE_SOUND_EFFECT)
            {
                stream_event = AUDIO_STATUS_DEVICE_CIS_PROMPT;
            }
            else
            {
                stream_event = AUDIO_STATUS_DEVICE_CIS_MUSIC;
            }
            break;
        case STREAMMING_CALL:
            call_active_id = ble_audio_earphoe_info_get_call_id_by_conidx_and_type(device->con_lid, AOB_CALL_STATE_ACTIVE);
            if (call_active_id != AOB_COMMON_INVALID_CALL_ID)
            {
                stream_event = AUDIO_STATUS_DEVICE_CIS_CALL;
            }
            else
            {
                call_active_id = ble_audio_earphoe_info_get_call_id_by_conidx_and_type(device->con_lid, AOB_CALL_STATE_ALERTING);
                if (call_active_id != AOB_COMMON_INVALID_CALL_ID)
                {
                    stream_event = AUDIO_STATUS_DEVICE_CIS_OUTGOING;
                }
                else
                {
                    call_active_id = ble_audio_earphoe_info_get_call_id_by_conidx_and_type(device->con_lid, AOB_CALL_STATE_INCOMING);
                    if (call_active_id != AOB_COMMON_INVALID_CALL_ID)
                    {
                        stream_event = AUDIO_STATUS_DEVICE_CIS_INCOMING;
                    }
                    else
                    {
                       stream_event = AUDIO_STATUS_DEVICE_CIS_CALL;
                    }
                }
            }
            break;
        case STREAMMING_GAME:
            if (streaming_context_type & AOB_AUDIO_CONTEXT_TYPE_GAME)
            {
                stream_event = AUDIO_STATUS_DEVICE_CIS_GAME;
            }
            if (streaming_context_type & AOB_AUDIO_CONTEXT_TYPE_SOUND_EFFECT)
            {
                stream_event = AUDIO_STATUS_DEVICE_CIS_PROMPT;
            }
            break;
        case STREAMMING_INVALID:
        default:
        break;
    }
    TRACE(2,"d%x stream event:%d", device->con_lid, stream_event);
    return stream_event;
}

static int app_ble_audio_request_audio_focus(
    BLE_AUDIO_SINK_DEVICE_T *device, int request_type, AUDIO_USAGE_TYPE_E stream_type, bool allow_delay)
{
    audio_focus_req_info_t request_info;
    int result;

    memcpy((uint8_t *)&request_info.device_info.device_addr, (void *)&device->address, sizeof(ble_bdaddr_t));
    request_info.device_info.device_idx = device->con_lid;
    request_info.device_info.audio_type = AUDIO_TYPE_LE_AUDIO;
    request_info.device_info.focus_request_type = request_type;
    request_info.device_info.stream_type = stream_type;
    request_info.device_info.delayed_focus_allow = allow_delay;
    request_info.device_info.stream_event = app_ble_audio_get_device_stream_event(device);
    if (stream_type == USAGE_MEDIA)
    {
        request_info.focus_changed_listener = device->music_focus_changed_listener;
        request_info.ext_policy = app_ble_audio_music_ext_policy;
    }
    else if (stream_type == USAGE_FLEXIBLE)
    {
        request_info.focus_changed_listener = device->game_focus_changed_listener;
        request_info.ext_policy = app_ble_audio_flexible_ext_policy;
    }
    else if (stream_type == USAGE_CALL)
    {
        request_info.focus_changed_listener = device->call_focus_changed_listener;
        request_info.ext_policy = app_ble_audio_call_ext_policy;
    }
    else if (stream_type == USAGE_RINGTONE)
    {
        request_info.focus_changed_listener = device->ring_focus_changed_listener;
        request_info.ext_policy = app_ble_audio_ringtone_stream_ext_policy;
    }
    result = app_audio_request_focus(&request_info);
    TRACE(1, "le_audio_policy:requst audio focus result = %d", result);
    return result;
}

static int app_ble_audio_abandon_audio_focus(BLE_AUDIO_SINK_DEVICE_T *device, AUDIO_USAGE_TYPE_E stream_type)
{
    audio_focus_req_info_t removed_fouces;

    //memcpy((uint8_t*)&removed_fouces.device_addr,(void*)&device->remote,sizeof(bt_bdaddr_t));
    removed_fouces.device_info.device_idx = device->con_lid;
    removed_fouces.device_info.audio_type = AUDIO_TYPE_LE_AUDIO;
    removed_fouces.device_info.stream_type = stream_type;

    return app_audio_abandon_focus(&removed_fouces);
}

static void app_ble_audio_abandon_device_focus(BLE_AUDIO_SINK_DEVICE_T *device)
{
    audio_focus_req_info_t removed_fouces;
    removed_fouces.device_info.device_idx = device->con_lid;
    removed_fouces.device_info.audio_type = AUDIO_TYPE_LE_AUDIO;
    for(int i=0; i<USAGE_MAX; i++)
    {
        removed_fouces.device_info.stream_type = (AUDIO_USAGE_TYPE_E)i;
        app_audio_abandon_focus(&removed_fouces);
    }
    device->music_audio_focus = AUDIOFOCUS_NONE;
    device->call_audio_focus = AUDIOFOCUS_NONE;
    device->game_audio_focus = AUDIOFOCUS_NONE;
}

static void app_ble_audio_sink_start_play(uint8_t conid, AUDIO_USAGE_TYPE_E stream_type, uint8_t stream_lid)
{
    audio_manager_stream_ctrl_start_ble_audio(conid, GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE, stream_lid);
    uint8_t call_id = 0;
    uint8_t call_state = aob_call_if_get_not_idle_call_by_conidx(conid, &call_id);
    BLE_AUDIO_SINK_DEVICE_T *device = app_ble_audio_get_device(conid);
    if(stream_type == USAGE_MEDIA)
    {
        app_audio_adm_le_audio_handle_action(conid, LE_MUSIC_STREAMING_CHANGED, STRAEMING_START);
        app_audio_ctrl_update_ble_audio_music_state(conid, STRAEMING_START);
    }
    else if(stream_type == USAGE_CALL)
    {
        app_audio_adm_le_audio_handle_action(conid, LE_CALL_STREAMING_CHANGED, STRAEMING_START);
        app_audio_ctrl_le_update_call_state(conid, call_id, call_state);
    }
    else if(stream_type == USAGE_FLEXIBLE)
    {
        app_audio_adm_le_audio_handle_action(conid, LE_CALL_STREAMING_CHANGED, STRAEMING_START);
        app_audio_ctrl_le_update_call_state(conid, call_id, call_state);
    }
    app_ble_set_ble_active_link(device);
    curr_playing_le_id = conid;
    TRACE(3,"[d%x]app_ble_audio_sink_start_play,call_id:%d, call_state:%d", conid, call_id, call_state);
}

static void app_ble_audio_sink_stop_play(uint8_t conid, AUDIO_USAGE_TYPE_E stream_type, uint8_t stream_lid)
{
    BLE_AUDIO_SINK_DEVICE_T *device = app_ble_audio_get_device(conid);
    audio_manager_stream_ctrl_stop_ble_audio(conid, GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE, stream_lid);
    app_ble_audio_abandon_audio_focus(device, stream_type);
    if(stream_type == USAGE_MEDIA)
    {
        device->music_audio_focus = AUDIOFOCUS_NONE;
        app_audio_adm_le_audio_handle_action(conid, LE_MUSIC_STREAMING_CHANGED, STRAEMING_STOP);
    }
    else if(stream_type == USAGE_CALL)
    {
        device->call_audio_focus = AUDIOFOCUS_NONE;
        app_audio_adm_le_audio_handle_action(conid, LE_CALL_STREAMING_CHANGED, STRAEMING_STOP);
    }
    else if(stream_type == USAGE_FLEXIBLE)
    {
        device->game_audio_focus = AUDIOFOCUS_NONE;
        app_audio_adm_le_audio_handle_action(conid, LE_CALL_STREAMING_CHANGED, STRAEMING_STOP);
    }
    curr_playing_le_id = INVALID_BLE_CONIDX;
    TRACE(1,"[d%x]app_ble_audio_sink_stop_play!", conid);
}

static void app_ble_audio_stream_check_and_start_all_handler(uint8_t con_lid);

static int app_ble_audio_update_context_type_handle(BLE_AUDIO_SINK_DEVICE_T *device, AUDIO_USAGE_TYPE_E updated_type, AUDIO_USAGE_TYPE_E original_type,  bool updated_allow_delay)
{
    audio_focus_req_info_t updated_focus;

    memcpy((uint8_t *)&updated_focus.device_info.device_addr, (void *)&device->address, sizeof(bt_bdaddr_t));
    updated_focus.device_info.device_idx = device->con_lid;
    updated_focus.device_info.audio_type = AUDIO_TYPE_LE_AUDIO;
    updated_focus.device_info.stream_type = updated_type;
    updated_focus.device_info.stream_event = app_ble_audio_get_device_stream_event(device);
    if (updated_type == USAGE_MEDIA)
    {
        updated_focus.device_info.focus_request_type = AUDIOFOCUS_GAIN;
        updated_focus.focus_changed_listener = device->music_focus_changed_listener;
        updated_focus.ext_policy = app_ble_audio_music_ext_policy;
    }
    else if (updated_type == USAGE_FLEXIBLE)
    {
        updated_focus.device_info.focus_request_type = AUDIOFOCUS_GAIN;
        updated_focus.focus_changed_listener = device->game_focus_changed_listener;
        updated_focus.ext_policy = app_ble_audio_flexible_ext_policy;
    }
    else if (updated_type == USAGE_CALL)
    {
        updated_focus.device_info.focus_request_type = AUDIOFOCUS_GAIN_TRANSIENT;
        updated_focus.focus_changed_listener = device->call_focus_changed_listener;
        updated_focus.ext_policy = app_ble_audio_call_ext_policy;
    }
    else if (updated_type == USAGE_RINGTONE)
    {
        updated_focus.device_info.focus_request_type = AUDIOFOCUS_GAIN_TRANSIENT_MAY_DUCK;
        updated_focus.focus_changed_listener = device->ring_focus_changed_listener;
        updated_focus.ext_policy = app_ble_audio_ringtone_stream_ext_policy;
    }

    int update_ret = app_audio_ble_update_focus(&updated_focus);

    TRACE(0, "_update_ctx_handle ret %d", update_ret);

    if (update_ret == AUDIOFOCUS_UPDATE_DOWNGRADE)
    {
        // Stop play
        audio_manager_stream_ctrl_stop_ble_audio(device->con_lid, GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE, device->cis_sink_ase_lid);
        // abandon focus, this will play the focus below top
        app_ble_audio_abandon_audio_focus(device, original_type);
        // Req again, it must insert below top
        int req_ret = app_ble_audio_request_audio_focus(device, updated_focus.device_info.focus_request_type, updated_type, updated_allow_delay);
        if (req_ret == AUDIOFOCUS_REQUEST_GRANTED)
        {
            TRACE(0, "req_ret not equal update_ret, Invalid");
        }
        else if (req_ret == AUDIOFOCUS_REQUEST_DELAYED)
        {
            if (device->streamming_context == STREAMMING_MUSIC)
            {
                device->music_audio_focus = AUDIOFOCUS_LOSS_TRANSIENT;
            }
            else if (device->streamming_context == STREAMMING_CALL)
            {
                device->call_audio_focus = AUDIOFOCUS_LOSS_TRANSIENT;
            }
            else if (device->streamming_context == STREAMMING_GAME)
            {
                device->game_audio_focus = AUDIOFOCUS_LOSS_TRANSIENT;
            }
        }
        else if (req_ret == AUDIOFOCUS_REQUEST_FAILED)
        {
            if (device->streamming_context == STREAMMING_MUSIC)
            {
                device->music_audio_focus = AUDIOFOCUS_LOSS;
            }
            else if (device->streamming_context == STREAMMING_CALL)
            {
                device->call_audio_focus = AUDIOFOCUS_LOSS;
            }
            else if (device->streamming_context == STREAMMING_GAME)
            {
                device->game_audio_focus = AUDIOFOCUS_LOSS;
            }
        }
    }
    else if (update_ret == AUDIOFOCUS_UPDATE_UPGRADE)
    {
        // abandon focus
        app_ble_audio_abandon_audio_focus(device, original_type);
        // Req again, it must insert below top
        int req_ret = app_ble_audio_request_audio_focus(device, updated_focus.device_info.focus_request_type, updated_type, updated_allow_delay);
        if (req_ret == AUDIOFOCUS_REQUEST_GRANTED)
        {
            app_ble_audio_stream_check_and_start_all_handler(device->con_lid);
            device->set_as_bg_device = false;
            if (device->streamming_context == STREAMMING_MUSIC)
            {
                device->music_audio_focus = AUDIOFOCUS_GAIN;
            }
            else if (device->streamming_context == STREAMMING_CALL)
            {
                device->call_audio_focus = AUDIOFOCUS_GAIN;
            }
            else if (device->streamming_context == STREAMMING_GAME)
            {
                device->game_audio_focus = AUDIOFOCUS_GAIN;
            }
        }
        else
        {
            TRACE(0, "req_ret invalid %d", req_ret);
        }
    }
    else if (update_ret == AUDIOFOCUS_ONLY_REPLACE)
    {
        TRACE(0, "Replace only");
    }
    else if (update_ret == AUDIOFOCUS_NO_CHANGE)
    {
        TRACE(0, "No change");
    }

    /* Update adm stream device */
    switch (original_type)
    {
        case USAGE_MEDIA:
            if (updated_type == USAGE_CALL ||
                updated_type == USAGE_FLEXIBLE)
            {
                /* flexible in adm stream device as call, see app_ble_audio_sink_start/stop_play */
                app_audio_adm_le_audio_handle_action(device->con_lid, LE_MUSIC_STREAMING_CHANGED, STRAEMING_STOP);
                app_audio_adm_le_audio_handle_action(device->con_lid, LE_CALL_STREAMING_CHANGED, STRAEMING_START);
            }
            break;
        case USAGE_CALL:
        case USAGE_FLEXIBLE:
            if (updated_type == USAGE_MEDIA)
            {
                app_audio_adm_le_audio_handle_action(device->con_lid, LE_CALL_STREAMING_CHANGED, STRAEMING_STOP);
                app_audio_adm_le_audio_handle_action(device->con_lid, LE_MUSIC_STREAMING_CHANGED, STRAEMING_START);
            }
            break;
        default:
            TRACE(0, "Invalid update %d/%d", original_type, updated_type);
            break;
    }

    return 0;
}

void app_ble_audio_stream_start_handler(uint8_t ase_lid)
{
    gaf_stream_context_state_t updatedContextStreamState = gaf_audio_stream_update_and_start_handler(ase_lid);
    if(APP_GAF_CONTEXT_STREAM_STARTED != updatedContextStreamState)
    {
        return;
    }
    app_bap_ascs_ase_t *p_bap_ase_info = app_bap_uc_srv_get_ase_info(ase_lid);
    app_ble_audio_event_t evt = BLE_AUDIO_MAX_IND;
    uint16_t con_context = app_ble_audio_get_curr_device_streaming_context_bf(p_bap_ase_info->con_lid);

    if (con_context & AOB_AUDIO_CONTEXT_TYPE_CONVERSATIONAL)
    {
        evt = BLE_AUDIO_CALL_STREAM_START_IND;
    }
    else if (con_context & AOB_AUDIO_CONTEXT_TYPE_MEDIA)
    {
        evt = BLE_AUDIO_MUSIC_STREAM_START_IND;
    }
    else
    {
        evt = BLE_AUDIO_FLEXIBLE_STREAM_START_IND;
    }

    TRACE(3, "%s context:%d, evt:%d", __func__, con_context, evt);
    app_ble_audio_sink_streaming_handle_event(p_bap_ase_info->con_lid, ase_lid, APP_GAF_DIRECTION_MAX, evt);
}

static void app_ble_audio_stream_check_and_start_all_handler(uint8_t con_lid)
{
    uint8_t streaming_ase_lid_list[APP_BAP_MAX_ASCS_NB_ASE_CHAR] = {0};
    uint8_t enableing_ase_lid_list[APP_BAP_MAX_ASCS_NB_ASE_CHAR] = {0};
    uint8_t streaming_ase_cnt = aob_media_get_curr_streaming_ase_lid_list(con_lid, streaming_ase_lid_list);
    uint8_t enableing_ase_cnt = aob_media_get_enableing_ase_lid_list(con_lid, enableing_ase_lid_list);
    uint8_t ase_cnt = 0;

    for (ase_cnt = 0; ase_cnt < streaming_ase_cnt; ase_cnt++)
    {
        app_ble_audio_stream_start_handler(streaming_ase_lid_list[ase_cnt]);
    }

    for (ase_cnt = 0; ase_cnt < enableing_ase_cnt; ase_cnt++)
    {
        app_bap_ascs_ase_t *p_bap_ase_info = app_bap_uc_srv_get_ase_info(enableing_ase_lid_list[ase_cnt]);
        if(p_bap_ase_info && p_bap_ase_info->direction == APP_GAF_DIRECTION_SINK)
        {
            TRACE(2,"d%x check and start enableing ase_id:%d", con_lid, enableing_ase_lid_list[ase_cnt]);
            app_ble_audio_stream_start_handler(enableing_ase_lid_list[ase_cnt]);
        }
    }
}

static void app_ble_audio_on_music_focus_changed(uint8_t device_id, AUDIO_USAGE_TYPE_E media_type, int audio_focus_change)
{
    uint8_t stream_lid;
    TRACE(3, "le_audio_policy:music focus change device_id:%d,media_type:%d,focus_changed = %d", device_id, media_type, audio_focus_change);
    ASSERT(media_type == USAGE_MEDIA, "media_type error %d", media_type);
    BLE_AUDIO_SINK_DEVICE_T *device = app_ble_audio_get_device(device_id);
    if (!device)
    {
        return;
    }
    TRACE(2, "le_audio_policy:music streaming status = %d,bg device = %d", device->streamming_context, device->set_as_bg_device);

    if(device->bis_streaming_available)
    {
        stream_lid = device->bis_sink_stream_lid;
    }
    else
    {
        stream_lid = device->cis_sink_ase_lid;
    }

    switch (audio_focus_change)
    {
        case AUDIOFOCUS_GAIN:
            device->music_audio_focus = AUDIOFOCUS_GAIN;
            if (device->current_context_type == GAF_AUDIO_STREAM_CONTEXT_TYPE_BIS)
            {
                if (device->audio_focus_resume_callabck != NULL)
                {
                    device->audio_focus_resume_callabck(device->bis_sink_stream_lid, 0);
                }
                else
                {
                    aob_bis_restart_deleg_bis_sink();
                }
            }
            else
            {
                if (device->set_as_bg_device == true)
                {
                    aob_media_play(device_id);
                }
                // audio_manager_stream_ctrl_start_ble_audio(device_id, GAF_AUDIO_STREAM_CONTEXT_TYPE_MEDIA);
                /// ringtone and LE Audio cocurr situation, because ringtone start but stream not stop.
                /// so need check start after ringtone stopped
                app_ble_audio_stream_check_and_start_all_handler(device_id);
            }
            break;
        case AUDIOFOCUS_LOSS:
            if (device->streamming_context == STREAMMING_MUSIC)
            {
                device->set_as_bg_device = true;
                if (device->current_context_type == GAF_AUDIO_STREAM_CONTEXT_TYPE_BIS)
                {
                    aob_bis_sink_stop();
                }
                else
                {
                    aob_media_pause(device_id);
                }
                audio_manager_stream_ctrl_stop_ble_audio(device_id, device->current_context_type, stream_lid);
            }
            device->music_audio_focus = AUDIOFOCUS_NONE;
            app_ble_audio_abandon_audio_focus(device, USAGE_MEDIA);
            break;
        case AUDIOFOCUS_LOSS_TRANSIENT:
            device->music_audio_focus = AUDIOFOCUS_NONE;
            if (device->streamming_context == STREAMMING_MUSIC)
            {
                device->set_as_bg_device = true;
                if (device->current_context_type == GAF_AUDIO_STREAM_CONTEXT_TYPE_BIS)
                {
                    aob_bis_sink_stop();
                }
                else
                {
                    aob_media_pause(device_id);
                }
                audio_manager_stream_ctrl_stop_ble_audio(device_id, device->current_context_type, stream_lid);
            }
            break;
        case AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
            if (device->streamming_context == STREAMMING_MUSIC)
            {
                audio_manager_stream_ctrl_stop_ble_audio(device_id, device->current_context_type, stream_lid);
                TRACE(0, "waiting phone stop media!");
            }
            break;
        default:
            break;
    }
}

static void app_ble_audio_pull_call_backto_earbud(uint8_t sink_ase_lid, uint8_t src_ase_lid)
{
#if defined(BLE_AUDIO_CENTRAL_APP_ENABLED)
        app_bap_ascs_ase_t *call_sink_ase_info = app_bap_uc_srv_get_ase_info(sink_ase_lid);
        app_bap_ascs_ase_t *call_src_ase_info = app_bap_uc_srv_get_ase_info(src_ase_lid);
        // TODO: If ase state is idle, no bap_cfg is stored
        aob_media_ascs_srv_set_codec(sink_ase_lid, &call_sink_ase_info->codec_id, &call_sink_ase_info->qos_req, call_sink_ase_info->p_cfg);
        aob_media_ascs_srv_set_codec(src_ase_lid, &call_src_ase_info->codec_id, &call_src_ase_info->qos_req, call_src_ase_info->p_cfg);
#else
#ifdef BLE_USB_AUDIO_SUPPORT
    app_bap_ascs_ase_t *call_sink_ase_info = app_bap_uc_srv_get_ase_info(sink_ase_lid);
    app_bap_ascs_ase_t *call_src_ase_info = app_bap_uc_srv_get_ase_info(src_ase_lid);
    // TODO: If ase state is idle, no bap_cfg is stored
    if (call_sink_ase_info)
    {
        app_bap_uc_srv_configure_codec_req(call_sink_ase_info, call_sink_ase_info->p_cfg);
    }
    if (call_src_ase_info)
    {
        app_bap_uc_srv_configure_codec_req(call_src_ase_info, call_src_ase_info->p_cfg);
    }
    //aob_media_set_codec(device->con_lid, device->cis_sink_ase_lid, AOB_MGR_DIRECTION_SINK, app_gaf_codec_id_t *codec);
    //aob_media_set_codec(device->con_lid, device->cis_sink_ase_lid, AOB_MGR_DIRECTION_SRC, app_gaf_codec_id_t *codec);
#endif
#endif
}

static void app_ble_audio_on_phone_call_focus_changed(uint8_t device_id, AUDIO_USAGE_TYPE_E media_type, int audio_focus_change)
{
    TRACE(3, "le_audio_policy:call focus change device_id:%d,media_type:%d,focus_changed = %d",
          device_id, media_type, audio_focus_change);
    ASSERT((USAGE_CALL == media_type) || (USAGE_RINGTONE == media_type), "call type error:%d", media_type);
    BLE_AUDIO_SINK_DEVICE_T *device = app_ble_audio_get_device(device_id);
    AOB_CALL_ENV_INFO_T *pCallEnvInfo = ble_audio_earphone_info_get_call_env_info(device_id);
    TRACE(0, "streamming_ctx %d", device->streamming_context);

    switch (audio_focus_change)
    {
        case AUDIOFOCUS_GAIN:
            if (media_type == USAGE_CALL)
            {
                if (device->streamming_context == STREAMMING_CALL)
                {
                    device->call_audio_focus = AUDIOFOCUS_GAIN;
                    gaf_audio_stream_update_and_start_handler(device->cis_sink_ase_lid);
                    app_ble_audio_stream_check_and_start_all_handler(device->con_lid);
                }
                else
                {
                    if (pCallEnvInfo)
                    {
                        if ((pCallEnvInfo->status_flags.inband_ring_enable &&
                                ble_audio_earphone_info_get_incoming_call_id_by_conidx(device_id) != INVALID_CALL_ID)
                                || ble_audio_earphone_info_get_calling_call_id_by_conidx(device_id) != INVALID_CALL_ID)
                        {
                            app_ble_audio_pull_call_backto_earbud(device->cis_sink_ase_lid, device->cis_src_ase_lid);
                        }
                    }
                    else
                    {
                        app_ble_audio_abandon_audio_focus(device, USAGE_CALL);
                        device->call_audio_focus = AUDIOFOCUS_NONE;
                    }
                }
            }
            else if (media_type == USAGE_RINGTONE)
            {
                app_ble_audio_abandon_audio_focus(device, USAGE_RINGTONE);
            }
            break;
        case AUDIOFOCUS_LOSS:
            if (device->streamming_context == STREAMMING_CALL)
            {
                app_bap_uc_srv_stream_release(device_id, false);
                audio_manager_stream_ctrl_stop_ble_audio(device_id, device->current_context_type, device->cis_sink_ase_lid);
            }
            app_ble_audio_abandon_audio_focus(device, USAGE_MEDIA);
            device->call_audio_focus = AUDIOFOCUS_NONE;
            break;
        case AUDIOFOCUS_LOSS_TRANSIENT:
            TRACE(1, "le_audio_policy phone streaming status = %d", device->streamming_context);
            device->call_audio_focus = AUDIOFOCUS_NONE;
            if (device->streamming_context == STREAMMING_CALL && media_type == USAGE_CALL)
            {
                device->set_as_bg_device = true;
                audio_manager_stream_ctrl_stop_ble_audio(device_id, GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE, device->cis_sink_ase_lid);
            }
            break;
        case AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
            break;
        default:
            break;
    }
}

static void app_ble_audio_on_phone_ring_focus_changed(uint8_t device_id, AUDIO_USAGE_TYPE_E media_type, int audio_focus_change)
{
    TRACE(3, "le_audio_policy:ring focus change device_id:%d,media_type:%d,focus_changed = %d", device_id, media_type, audio_focus_change);
    ASSERT(media_type == USAGE_RINGTONE, "ring type error %d", media_type);
    BLE_AUDIO_SINK_DEVICE_T *device = app_ble_audio_get_device(device_id);
    CALL_STATE_INFO_T *curr_call_state = app_bt_get_call_state_info();
    TRACE(1, "le_audio_policy:LE_Audio curr call state = %d", curr_call_state->state);

    switch (audio_focus_change)
    {
        case AUDIOFOCUS_GAIN:
            if (AOB_CALL_STATE_INCOMING == (AOB_CALL_STATE_E)curr_call_state->state)
            {
                device->ring_audio_focus = AUDIOFOCUS_GAIN;
                app_audio_adm_le_audio_handle_action(device->con_lid, RINGTONE_CHANGED, RINGTONE_START);
#ifdef MEDIA_PLAYER_SUPPORT
                media_PlayAudio(AUD_ID_LE_AUD_INCOMING_CALL, device->con_lid);
#endif
            }
            else
            {
                app_ble_audio_abandon_audio_focus(device, USAGE_RINGTONE);
                device->ring_audio_focus = AUDIOFOCUS_NONE;
            }
            break;
        case AUDIOFOCUS_LOSS:
            app_ble_audio_abandon_audio_focus(device, USAGE_RINGTONE);
            device->ring_audio_focus = AUDIOFOCUS_NONE;
            break;
        case AUDIOFOCUS_LOSS_TRANSIENT:
            break;
        case AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
            break;
        default:
            break;
    }
}


static int app_le_audio_toggle_cis_to_cis(BLE_AUDIO_SINK_DEVICE_T *device, AUDIO_USAGE_TYPE_E stream_type)
{
    audio_focus_req_info_t request_info;
    int ret;

    memcpy((uint8_t*)&request_info.device_info.device_addr.address[0],(void*)&device->address,sizeof(bt_bdaddr_t));
    request_info.device_info.device_idx = device->con_lid;
    request_info.device_info.audio_type = AUDIO_TYPE_LE_AUDIO;
    request_info.device_info.focus_request_type = AUDIOFOCUS_GAIN;
    request_info.device_info.stream_type = stream_type;
    request_info.device_info.stream_event = app_ble_audio_get_device_stream_event(device);
    request_info.device_info.delayed_focus_allow = ble_audio_policy_config.mute_new_music_stream;
    uint8_t sink_ase_id = device->cis_sink_ase_lid;

    switch(stream_type)
    {
    case USAGE_MEDIA:
        request_info.focus_changed_listener = device->music_focus_changed_listener;
        request_info.ext_policy = app_ble_audio_music_ext_policy;
        break;
    case USAGE_FLEXIBLE:
        request_info.focus_changed_listener = device->game_focus_changed_listener;
        request_info.ext_policy = app_ble_audio_flexible_ext_policy;
        break;
    case USAGE_CALL:
        request_info.focus_changed_listener = device->call_focus_changed_listener;
        request_info.ext_policy = app_ble_audio_flexible_ext_policy;
        break;
    default:
        TRACE(0, "stream_type invalid");
        return AUDIOFOCUS_REQUEST_FAILED;
        break;
    }

    ret = app_audio_request_switch_focus(&request_info);

    if(ret == AUDIOFOCUS_REQUEST_GRANTED)
    {
        device->music_audio_focus = AUDIOFOCUS_GAIN;
        app_ble_audio_sink_start_play(device->con_lid, stream_type, sink_ase_id);
        if(cis_switch_cmp_cb)
        {
            cis_switch_cmp_cb(device->con_lid);
        }
    }
    TRACE(2,"[d%x]le_audio_policy:switch request:%d", device->con_lid, ret);
    return ret;
}

static void app_ble_audio_sink_ringtone_timer_handler(void const *param)
{
    BLE_AUDIO_SINK_DEVICE_T *device = (BLE_AUDIO_SINK_DEVICE_T *)param;
    if (device == NULL)
    {
        return;
    }

    TRACE(0, "app_ble_audio_ring_tone_play");
    if (app_ble_audio_request_audio_focus(
                device, AUDIOFOCUS_GAIN_TRANSIENT_MAY_DUCK, USAGE_RINGTONE, false) == AUDIOFOCUS_REQUEST_GRANTED)
    {
        app_audio_adm_le_audio_handle_action(device->con_lid, RINGTONE_CHANGED, RINGTONE_START);
#ifdef MEDIA_PLAYER_SUPPORT
        media_PlayAudio(AUD_ID_LE_AUD_INCOMING_CALL, device->con_lid);
#endif
    }
    else
    {
        TRACE(0, "le_audio_policy:ringtone request focus fail/delay");
    }

    osTimerStop(device->ring_tone_play_timer);
    osTimerStart(device->ring_tone_play_timer, BLE_AUDIO_PLAY_RING_TONE_INTERVAL);
}

static void app_ble_audio_sink_playback_status_check_handler(void const *param)
{
    BLE_AUDIO_SINK_DEVICE_T *device = (BLE_AUDIO_SINK_DEVICE_T *)param;
    uint8_t other_streaming_id = AOB_COMMON_INVALID_CON_ID;
    BLE_AUDIO_SINK_DEVICE_T *streaming_device = NULL;
    AUDIO_USAGE_TYPE_E stream_type = USAGE_UNKNOWN;
#if defined(IBRT)
    ble_bdaddr_t remote_addr = {{0}};
#endif
    uint8_t count = app_ble_audio_count_music_streaming_device();
    if(device->media_play_status == AOB_MGR_PLAYBACK_STATE_PAUSED && count >= AOB_COMMON_DUAL_BLEAUDIO_CONNECT)
    {
        for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
        {
            if(i == device->con_lid)
            {
                continue;
            }
            streaming_device = app_ble_audio_get_device(i);
            if(streaming_device->streamming_context == STREAMMING_MUSIC)
            {
                other_streaming_id = i;
                stream_type = USAGE_MEDIA;
                break;
            }
            if(streaming_device->streamming_context == STREAMMING_GAME)
            {
                other_streaming_id = i;
                stream_type = USAGE_FLEXIBLE;
                break;
            }
        }
    }

    if(other_streaming_id != AOB_COMMON_INVALID_CON_ID)
    {
        //To notify peer earbud avoid peer not accept playstaus
        if(bes_bt_tws_besaud_is_connected())
        {
#if defined(IBRT)
            bool ret = app_ble_get_peer_solved_addr(streaming_device->con_lid, &remote_addr);
            if(ret)
            {
#if defined (BT_SERVICE_ENABLE)
                app_tws_send_ext_switch_info((bt_bdaddr_t*)&remote_addr);
#endif
            }
#endif
        }
        app_le_audio_toggle_cis_to_cis(streaming_device, stream_type);
    }
    TRACE(1,"le_audio_policy:switch to other device:%x", other_streaming_id);
}

void app_ble_audio_handle_peer_swicth_info(void *addr)
{
    ble_bdaddr_t *remote_addr = (ble_bdaddr_t*)addr;
    uint8_t conid = 0xFF;
    uint8_t count = app_ble_audio_count_music_streaming_device();
    ble_bdaddr_t addr_get = {0};
    AUDIO_USAGE_TYPE_E stream_type = USAGE_UNKNOWN;
    for (uint8_t conidx = 0; conidx < AOB_COMMON_MOBILE_CONNECTION_MAX; conidx++)
    {
        bool ret = app_ble_get_peer_solved_addr(conidx, &addr_get);

        if (ret == true && !memcmp(remote_addr, &addr_get, sizeof(addr_get)))
        {
            conid = conidx;
        }
    }
    if(conid != 0xFF)
    {
        BLE_AUDIO_SINK_DEVICE_T *device = app_ble_audio_get_device(conid);
        if(device->streamming_context == STREAMMING_MUSIC)
        {
            stream_type = USAGE_MEDIA;
        }
        else if(device->streamming_context == STREAMMING_GAME)
        {
            stream_type = USAGE_FLEXIBLE;
        }
        if(count >= AOB_COMMON_DUAL_BLEAUDIO_CONNECT)
        {
            app_le_audio_toggle_cis_to_cis(device, stream_type);
        }
        TRACE(1,"le_audio_policy switch to conid:%d", device->con_lid);
    }
}

static void app_ble_audio_sink_playback_status_changed_handler(uint8_t con_lid)
{
    BLE_AUDIO_SINK_DEVICE_T *device = app_ble_audio_get_device(con_lid);
    if(device->media_play_status == AOB_MGR_PLAYBACK_STATE_PLAYING)
    {
        osTimerStop(device->playback_status_check_timer);
        TRACE(0,"le_audio_policy:playback_status_playing");
    }
    else if(device->media_play_status == AOB_MGR_PLAYBACK_STATE_PAUSED)
    {
        osTimerStop(device->playback_status_check_timer);
        osTimerStart(device->playback_status_check_timer, BLE_AUDIO_PLAY_STATUS_CHECK_TIME);
        TRACE(0,"le_audio_policy:playback_status_pause");
    }
}

void app_ble_audio_stop_ring_tone_play(BLE_AUDIO_SINK_DEVICE_T *device)
{
    osTimerStop(device->ring_tone_play_timer);
    app_audio_adm_le_audio_handle_action(device->con_lid, RINGTONE_CHANGED, RINGTONE_STOP);
    //app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_STOP_MEDIA, BT_STREAM_MEDIA, device->con_lid, 0);
}

#if defined(BLE_USB_AUDIO_SUPPORT)
static void  app_ble_audio_pullback_dongle_audio()
{
    bool ret = false;
#if BLE_BUDS
    uint8_t cmd_buf[1] = {0xbb};
    ret = buds_send_data_via_write_command(0, cmd_buf, 1);
#endif
    TRACE(0, "%s,ret %d", __func__, ret);
}
#endif

static void POSSIBLY_UNUSED app_ble_audio_disable_game_ase(BLE_AUDIO_SINK_DEVICE_T *device)
{
    app_bap_ascs_ase_t *p_ascs_ase = NULL;

    p_ascs_ase = app_bap_uc_srv_get_ase_info(device->cis_src_ase_lid);
    if (p_ascs_ase && p_ascs_ase->ase_state == AOB_MGR_STREAM_STATE_STREAMING)
    {
        TRACE(1, "le_audio_policy:disable game ase src id = %d", device->cis_src_ase_lid);
        app_bap_uc_srv_stream_disable(device->cis_src_ase_lid);
    }
    p_ascs_ase = app_bap_uc_srv_get_ase_info(device->cis_sink_ase_lid);
    if (p_ascs_ase && p_ascs_ase->ase_state == AOB_MGR_STREAM_STATE_STREAMING)
    {
        TRACE(1, "le_audio_policy:disable game ase sink id = %d", device->cis_sink_ase_lid);
        app_bap_uc_srv_stream_disable(device->cis_sink_ase_lid);
    }
}

static void app_ble_audio_on_game_focus_changed(uint8_t device_id, AUDIO_USAGE_TYPE_E media_type, int audio_focus_change)
{

    BLE_AUDIO_SINK_DEVICE_T *device = app_ble_audio_get_device(device_id);

    TRACE(3, "le_audio_policy:game focus change devid:%d,media_type:%d,focus_changed = %d ctx %d",
            device_id,
            media_type,
            audio_focus_change,
            device->streamming_context);

    switch (audio_focus_change)
    {
        case AUDIOFOCUS_GAIN:
            device->game_audio_focus = AUDIOFOCUS_GAIN;
            if (device->streamming_context == STREAMMING_GAME)
            {
                app_ble_audio_stream_check_and_start_all_handler(device_id);
            }
            else
            {
#if defined(BLE_USB_AUDIO_SUPPORT)
                app_ble_audio_pullback_dongle_audio();
#endif
            }
            break;
        case AUDIOFOCUS_LOSS:
            if (device->streamming_context == STREAMMING_GAME)
            {
#ifdef SASS_ENABLED
                TRACE(0, "sass: only stop ble audio_01");
                audio_manager_stream_ctrl_stop_ble_audio(device_id, GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE, device->cis_sink_ase_lid);
#else
                app_bap_uc_srv_stream_release(device_id, false);
                audio_manager_stream_ctrl_stop_ble_audio(device_id, GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE, device->cis_sink_ase_lid);
#endif
            }
            else
            {
                device->game_audio_focus = AUDIOFOCUS_NONE;
                app_ble_audio_abandon_audio_focus(device, USAGE_FLEXIBLE);
            }
            break;
        case AUDIOFOCUS_LOSS_TRANSIENT:
            TRACE(1, "le_audio_policy game streaming status = %d", device->streamming_context);
            if (device->streamming_context == STREAMMING_GAME)
            {
#ifdef SASS_ENABLED
                TRACE(0, "sass: only stop ble audio_02");
                audio_manager_stream_ctrl_stop_ble_audio(device_id, GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE, device->cis_sink_ase_lid);
#else
                audio_manager_stream_ctrl_stop_ble_audio(device_id, GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE, device->cis_sink_ase_lid);
#endif
            }
            else
            {
                device->game_audio_focus = AUDIOFOCUS_LOSS_TRANSIENT;
                app_ble_audio_abandon_audio_focus(device, USAGE_FLEXIBLE);
            }
            break;
        case AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
            break;
        default:
            break;
    }
}

static void app_ble_audio_stop_ringtone(uint8_t con_lid)
{
    BLE_AUDIO_SINK_DEVICE_T *device = app_ble_audio_get_device(con_lid);
    TRACE(0, "le_audio_policy,stop ringtone");
    app_ble_audio_stop_ring_tone_play(device);
    app_ble_audio_abandon_audio_focus(device, USAGE_RINGTONE);
    device->ring_audio_focus = AUDIOFOCUS_NONE;
}

static int app_le_audio_toggle_a2dp_to_cis(BLE_AUDIO_SINK_DEVICE_T *device, AUDIO_USAGE_TYPE_E stream_type)
{
    audio_focus_req_info_t request_info;
    int ret;

    memcpy((uint8_t*)&request_info.device_info.device_addr.address[0],(void*)&device->address,sizeof(bt_bdaddr_t));
    request_info.device_info.device_idx = device->con_lid;
    request_info.device_info.audio_type = AUDIO_TYPE_LE_AUDIO;
    request_info.device_info.focus_request_type = AUDIOFOCUS_GAIN;
    request_info.device_info.stream_type = stream_type;
    request_info.device_info.delayed_focus_allow = ble_audio_policy_config.mute_new_music_stream;
    uint8_t sink_ase_id = device->cis_sink_ase_lid;

    switch(stream_type)
    {
    case USAGE_MEDIA:
        request_info.focus_changed_listener = device->music_focus_changed_listener;
        request_info.ext_policy = app_ble_audio_music_ext_policy;
        break;
    case USAGE_FLEXIBLE:
        request_info.focus_changed_listener = device->game_focus_changed_listener;
        request_info.ext_policy = app_ble_audio_flexible_ext_policy;
        break;
    case USAGE_CALL:
        request_info.focus_changed_listener = device->call_focus_changed_listener;
        request_info.ext_policy = app_ble_audio_flexible_ext_policy;
        break;
    default:
        TRACE(0, "stream_type invalid");
        return AUDIOFOCUS_REQUEST_FAILED;
        break;
    }

    ret = app_audio_request_switch_focus(&request_info);

    if(ret == AUDIOFOCUS_REQUEST_GRANTED)
    {
        device->music_audio_focus = AUDIOFOCUS_GAIN;
        app_ble_audio_sink_start_play(device->con_lid, stream_type, sink_ase_id);
        // TODO: this place has some question
        app_audio_adm_le_audio_handle_action(device->con_lid,LE_CALL_STREAMING_CHANGED,STRAEMING_START);
    }
    return ret;
}

uint8_t app_ble_audio_count_music_streaming_device()
{
    BLE_AUDIO_SINK_DEVICE_T *device = NULL;
    uint8_t count = 0;
    for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        device = app_ble_audio_get_device(i);
        if(device->streamming_context != STREAMMING_INVALID)
        {
            count++;
        }
    }
    return count;
}

int app_ble_audio_bis_stream_set_resume_callback(void (*resume_cb)(uint8_t device_id, uint32_t param))
{
    BLE_AUDIO_SINK_DEVICE_T *device = app_ble_audio_get_device(AOB_COMMON_MOBILE_CONNECTION_MAX - 1);

    if (device == NULL)
    {
        return BT_STS_NOT_FOUND;
    }

    TRACE(1, "%s register bis resume cb:%p", __func__, resume_cb);
    device->audio_focus_resume_callabck = (on_audio_focus_resume_callback)resume_cb;

    return BT_STS_SUCCESS;
}

void app_ble_audio_sink_streaming_handle_event(uint8_t con_lid, uint8_t stream_lid, uint8_t data, app_ble_audio_event_t event)
{
    TRACE(2, "d(%d)ble_audio_sink_streaming handle event = %d,%d %d", con_lid, event, stream_lid, data);
    BLE_AUDIO_SINK_DEVICE_T *device = app_ble_audio_get_device(con_lid);
    ble_bdaddr_t remote_addr = {{0}};
    if (NULL == device)
    {
        TRACE(0, "can't find ble audio device!");
        return;
    }

    device->con_lid = con_lid;
    switch (event)
    {
        case BLE_AUDIO_LE_LINK_CONNECTED_IND:
        case BLE_AUDIO_MUSIC_QOS_CONFIG_IND:
            app_ble_get_peer_solved_addr(con_lid, &remote_addr);
            memcpy((void *)&device->address, (void *)&remote_addr, sizeof(ble_bdaddr_t));
            /* QOS configed means le audio connected */
            app_audio_adm_le_audio_handle_action(con_lid, DEVICE_CONN_STATUS_CHANGED, STATE_CONNECTED);
            break;
        case BLE_AUDIO_CALL_QOS_CONFIG_IND:
            app_audio_adm_le_audio_handle_action(con_lid, DEVICE_CONN_STATUS_CHANGED, STATE_CONNECTED);
            break;

        /*
         * Audio focus rule:Request audio focus when enable and abandon focus when cis stop.
         * if earbud accept enable but setup cis fail,no streaming stop
         * indication,so after recv release need also abandon audio focus.
        */
        case BLE_AUDIO_MUSIC_ENABLE_REQ:
            device->cis_sink_ase_lid = stream_lid;
            aob_media_send_enable_rsp(device->cis_sink_ase_lid, true);
            break;
        case BLE_AUDIO_CALL_ENABLE_REQ:
        case BLE_AUDIO_FLEXIBLE_ENABLE_REQ:
        case BLE_AUDIO_PROMPT_SOUND_ENABLE_REQ:
            if (APP_GAF_DIRECTION_SINK == data)
            {
                device->cis_sink_ase_lid = stream_lid;
            }
            else
            {
                device->cis_src_ase_lid = stream_lid;
            }

            aob_media_send_enable_rsp(stream_lid, true);
            break;
        case BLE_AUDIO_MUSIC_RELEASE_REQ:
            app_ble_audio_sink_stop_play(con_lid, USAGE_MEDIA, stream_lid);
            break;
        case BLE_AUDIO_CALL_RELEASE_REQ:
            app_ble_audio_sink_stop_play(con_lid, USAGE_CALL, stream_lid);
            break;
        case BLE_AUDIO_FLEXIBLE_RELEASE_REQ:
            app_ble_audio_sink_stop_play(con_lid, USAGE_FLEXIBLE, stream_lid);
            break;
        case BLE_AUDIO_EVENT_ROUTE_CALL_TO_BT:
            app_ble_audio_sink_stop_play(con_lid, USAGE_CALL, stream_lid);
            break;
        case BLE_AUDIO_MUSIC_STREAM_START_IND:
        case BLE_AUDIO_PROMPT_SOUND_START:
            device->streamming_context = STREAMMING_MUSIC;
            device->set_as_bg_device = false;

            if (device->music_audio_focus == AUDIOFOCUS_GAIN)
            {
                device->current_context_type = GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE;
                app_ble_audio_sink_start_play(con_lid, USAGE_MEDIA, stream_lid);
            }
            else
            {
                if (app_ble_audio_request_audio_focus(device, AUDIOFOCUS_GAIN, USAGE_MEDIA,
                                                      ble_audio_policy_config.mute_new_music_stream) == AUDIOFOCUS_REQUEST_GRANTED)
                {
                    device->music_audio_focus = AUDIOFOCUS_GAIN;
                    device->current_context_type = GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE;
                    app_ble_audio_sink_start_play(con_lid, USAGE_MEDIA, stream_lid);
                }
            }
            break;
        case BLE_AUDIO_MUSIC_STREAM_STOP_IND:
            device->streamming_context = STREAMMING_INVALID;
            app_ble_audio_sink_stop_play(con_lid, USAGE_MEDIA, stream_lid);
            app_audio_adm_le_audio_handle_action(con_lid, LE_MUSIC_STREAMING_CHANGED, STRAEMING_STOP);
            app_audio_ctrl_update_ble_audio_music_state(con_lid, STRAEMING_STOP);
            break;
        case BLE_AUDIO_CALL_STREAM_START_IND:
            device->streamming_context = STREAMMING_CALL;
            if (device->call_audio_focus == AUDIOFOCUS_GAIN)
            {
                // it's possible that down or up stream is started asynchronously, for this case,
                // when the second stream is started, the call's
                // focus has already been gained, so just send the request to let gaf audio to take
                // care which stream direction to start
                device->current_context_type = GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE;
                app_ble_audio_sink_start_play(con_lid, USAGE_CALL, stream_lid);
            }
            else
            {
                if (app_ble_audio_request_audio_focus(
                            device, AUDIOFOCUS_GAIN_TRANSIENT, USAGE_CALL, false) == AUDIOFOCUS_REQUEST_GRANTED)
                {
                    device->call_audio_focus = AUDIOFOCUS_GAIN;
                    device->current_context_type = GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE;
                    app_ble_audio_sink_start_play(con_lid, USAGE_CALL, stream_lid);
                }
            }
            break;
        case BLE_AUDIO_CALL_CAPTURE_STREAM_STOP_IND:
        case BLE_AUDIO_CALL_PLAYBACK_STREAM_STOP_IND:
        case BLE_AUDIO_CALL_SINGLE_STREAM_STOP_IND:
            audio_manager_stream_ctrl_stop_single_ble_audio_stream(con_lid, GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE, stream_lid);
            break;
        case BLE_AUDIO_CALL_ALL_STREAMS_STOP_IND:
            /*
             * How to handle wechat behavior.When streaming stop it means call is end.
            */
            device->streamming_context = STREAMMING_INVALID;
            app_ble_audio_sink_stop_play(con_lid, USAGE_CALL, stream_lid);
            app_audio_adm_le_audio_handle_action(con_lid, LE_CALL_STREAMING_CHANGED, STRAEMING_STOP);
            break;
        case BLE_AUDIO_CALL_RINGING_IND:
            app_ble_audio_sink_ringtone_timer_handler(device);
            break;
        case BLE_AUDIO_CALL_TERMINATE_IND:
            app_ble_audio_stop_ringtone(con_lid);
            break;
        case BLE_AUDIO_FLEXIBLE_STREAM_START_IND:
            device->streamming_context = STREAMMING_GAME;
            TRACE(0, "game_audio_focus %d", device->game_audio_focus);
            if (device->game_audio_focus == AUDIOFOCUS_GAIN)
            {
                device->current_context_type = GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE;
                app_ble_audio_sink_start_play(con_lid, USAGE_FLEXIBLE, stream_lid);
            }
            else
            {
                if (app_ble_audio_request_audio_focus(
                            device, AUDIOFOCUS_GAIN, USAGE_FLEXIBLE, false) == AUDIOFOCUS_REQUEST_GRANTED)
                {
                    device->game_audio_focus = AUDIOFOCUS_GAIN;
                    device->current_context_type = GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE;
                    app_ble_audio_sink_start_play(con_lid, USAGE_FLEXIBLE, stream_lid);
                }
            }
            break;
        case BLE_AUDIO_FLEXIBLE_CAPTURE_STREAM_STOP_IND:
        case BLE_AUDIO_FLEXIBLE_PLAYBACK_STREAM_STOP_IND:
            audio_manager_stream_ctrl_stop_single_ble_audio_stream(con_lid, GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE, stream_lid);
            break;
        case BLE_AUDIO_FLEXIBLE_ALL_STREAMS_STOP_IND:
            device->streamming_context = STREAMMING_INVALID;
            app_ble_audio_sink_stop_play(con_lid, USAGE_FLEXIBLE, stream_lid);
            app_ble_audio_abandon_device_focus(device);
            break;
        case BLE_AUDIO_LE_LINK_DISCONCETED_IND:
            app_ble_audio_stop_ringtone(con_lid);
            // af_codec_set_bt_sync_source(AUD_STREAM_PLAYBACK, 0);
            // af_codec_set_bt_sync_source(AUD_STREAM_CAPTURE, 0);
            /*
             * The Released operation shall be initiated autonomously by the server if:
             * The server has detected the loss of the LE-ACL for an ASE in any state.
             * If the server does not want to cache a codec configuration:
             * Transition the ASE to the Idle state and write a value of 0x00 (Idle) to the ASE_State field.
             *
             * The service received client Enable and request audio focus success but link loss occure,the
             * service ase state transition to idle,in this case,audio policy need to abandon audio focus.
            */
            if(curr_playing_le_id == con_lid)
            {
                app_ble_audio_sink_stop_play(con_lid, USAGE_FLEXIBLE, stream_lid);
            }
            app_ble_audio_abandon_device_focus(device);
            app_audio_adm_le_audio_handle_action(con_lid, DEVICE_CONN_STATUS_CHANGED, STATE_DISCONNECTED);
            break;
        case BLE_AUDIO_TOGGLE_A2DP_CIS_REQ:
            app_le_audio_toggle_a2dp_to_cis(device, (AUDIO_USAGE_TYPE_E)stream_lid);
            break;
        case BLE_AUDIO_BIS_STREAM_START_IND:
            device->bis_streaming_available = true;
            device->set_as_bg_device = false;
            device->streamming_context = STREAMMING_MUSIC;
            device->current_context_type = GAF_AUDIO_STREAM_CONTEXT_TYPE_BIS;
            if (device->music_audio_focus == AUDIOFOCUS_GAIN)
            {
                audio_manager_stream_ctrl_start_ble_audio(con_lid, GAF_AUDIO_STREAM_CONTEXT_TYPE_BIS, stream_lid);
            }
            else
            {
                if (app_ble_audio_request_audio_focus(device, AUDIOFOCUS_GAIN, USAGE_MEDIA,
                                                      ble_audio_policy_config.mute_new_music_stream) == AUDIOFOCUS_REQUEST_GRANTED)
                {
                    device->music_audio_focus = AUDIOFOCUS_GAIN;
                    audio_manager_stream_ctrl_start_ble_audio(con_lid, GAF_AUDIO_STREAM_CONTEXT_TYPE_BIS, stream_lid);
                }
                else
                {
                    aob_bis_sink_stop();
                }
            }

            app_audio_adm_le_audio_handle_action(con_lid, LE_MUSIC_STREAMING_CHANGED, STRAEMING_START);
            break;
        case BLE_AUDIO_BIS_STREAM_STOP_IND:
            device->bis_streaming_available = false;
            audio_manager_stream_ctrl_stop_ble_audio(con_lid, GAF_AUDIO_STREAM_CONTEXT_TYPE_BIS, stream_lid);
            app_audio_adm_le_audio_handle_action(con_lid, LE_MUSIC_STREAMING_CHANGED, STRAEMING_STOP);
            device->streamming_context = STREAMMING_INVALID;
            device->music_audio_focus = AUDIOFOCUS_NONE;
            if(!device->set_as_bg_device)
            {
                app_ble_audio_abandon_audio_focus(device, USAGE_MEDIA);
            }
            break;
        case BLE_AUDIO_CIS_CONNECTED_IND:
        {
            audio_focus_req_info_t* curr_audio_focus = app_audio_get_curr_audio_focus();
            if (curr_audio_focus == NULL)
            {
                TRACE(0, "curr_audio_focus NULL");
                break;
            }

            // stream_lid represents ase_lid
            app_bap_ascs_ase_t *p_bap_ase_info = app_bap_uc_srv_get_ase_info(stream_lid);
            if (p_bap_ase_info == NULL)
            {
                TRACE(0, "bap_ase_info NULL ase_id %d", stream_lid);
                break;
            }

            TRACE(0, "dev_id %d audio_type %d cis handle 0x%x",
                      curr_audio_focus->device_info.device_idx,
                      curr_audio_focus->device_info.audio_type,
                      p_bap_ase_info->cis_hdl);

            if (curr_audio_focus->device_info.device_idx == con_lid
                && curr_audio_focus->device_info.audio_type == AUDIO_TYPE_LE_AUDIO)
            {
                app_audio_policy_set_bt_ble_active_link(CIS_MODE, p_bap_ase_info->cis_hdl);
            }
        }
        break;
        case BLE_AUDIO_UPDATE_CONTEXT_TYPE_TO_CALL:
        {
            TRACE(0, "uptocall %d", device->streamming_context);
            AUDIO_USAGE_TYPE_E origin_usage = USAGE_UNKNOWN;
            if (device->streamming_context == STREAMMING_MUSIC)
            {
                device->call_audio_focus = device->music_audio_focus;
                device->music_audio_focus = AUDIOFOCUS_NONE;
                origin_usage = USAGE_MEDIA;
            }
            else if (device->streamming_context == STREAMMING_GAME)
            {
                device->call_audio_focus = device->game_audio_focus;
                device->game_audio_focus = AUDIOFOCUS_NONE;
                origin_usage = USAGE_FLEXIBLE;
            }
            device->streamming_context = STREAMMING_CALL;
            app_ble_audio_update_context_type_handle(device, USAGE_CALL, origin_usage, false);
        }
        break;
        case BLE_AUDIO_UPDATE_CONTEXT_TYPE_TO_MUSIC:
        {
            TRACE(0, "uptomusic %d", device->streamming_context);
            AUDIO_USAGE_TYPE_E origin_usage = USAGE_UNKNOWN;
            if (device->streamming_context == STREAMMING_CALL)
            {
                device->music_audio_focus = device->call_audio_focus;
                device->call_audio_focus = AUDIOFOCUS_NONE;
                origin_usage = USAGE_CALL;
            }
            else if (device->streamming_context == STREAMMING_GAME)
            {
                device->music_audio_focus = device->game_audio_focus;
                device->game_audio_focus = AUDIOFOCUS_NONE;
                origin_usage = USAGE_FLEXIBLE;
            }
            device->streamming_context = STREAMMING_MUSIC;
            app_ble_audio_update_context_type_handle(device, USAGE_MEDIA, origin_usage,ble_audio_policy_config.mute_new_music_stream);
        }
        break;
        case BLE_AUDIO_UPDATE_CONTEXT_TYPE_TO_FLEXIBLE:
        {
             TRACE(0, "uptoflex %d", device->streamming_context);
             AUDIO_USAGE_TYPE_E origin_usage = USAGE_UNKNOWN;
             if (device->streamming_context == STREAMMING_CALL)
             {
                 device->game_audio_focus = device->call_audio_focus;
                 device->call_audio_focus = AUDIOFOCUS_NONE;
                 origin_usage = USAGE_CALL;
             }
             else if (device->streamming_context == STREAMMING_MUSIC)
             {
                 device->game_audio_focus = device->music_audio_focus;
                 device->music_audio_focus = AUDIOFOCUS_NONE;
                 origin_usage = USAGE_MEDIA;
             }
             device->streamming_context = STREAMMING_GAME;
             app_ble_audio_update_context_type_handle(device, USAGE_FLEXIBLE, origin_usage, false);
        }
        break;
        case BLE_AUDIO_MEDIA_PLAYSTATUS_CHANGED:
            device->media_play_status = (AOB_MGR_PLAYBACK_STATE_E)data;
            if(app_ble_audio_count_music_streaming_device() >= AOB_COMMON_DUAL_BLEAUDIO_CONNECT)
            {
                app_ble_audio_sink_playback_status_changed_handler(con_lid);
            }

            if (gaf_media_status_handler_cb != NULL)
            {
                if (device->media_play_status == AOB_MGR_PLAYBACK_STATE_PLAYING)
                {
                    gaf_media_status_handler_cb(con_lid, false);
                }
                else if (device->media_play_status == AOB_MGR_PLAYBACK_STATE_PAUSED)
                {
                    gaf_media_status_handler_cb(con_lid, true);
                }
            }
        break;
        case BLE_AUDIO_CALL_STATUS_CHANGED:
            if(data != AOB_CALL_STATE_IDLE && device->streamming_context != STREAMMING_INVALID)
            {
                if (app_ble_audio_request_audio_focus(
                            device, AUDIOFOCUS_GAIN_TRANSIENT, USAGE_CALL, false) == AUDIOFOCUS_REQUEST_GRANTED)
                {
                    device->call_audio_focus = AUDIOFOCUS_GAIN;
                    device->current_context_type = GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE;
                    app_ble_audio_sink_start_play(con_lid, USAGE_CALL, stream_lid);
                }
            }
            switch(data)
            {
                case AOB_CALL_STATE_INCOMING:
                    break;
                case AOB_CALL_STATE_DIALING:
                    break;
                case AOB_CALL_STATE_ALERTING:
                    break;
                case AOB_CALL_STATE_ACTIVE:
                    app_ble_audio_stop_ringtone(con_lid);
                    break;
                case AOB_CALL_STATE_LOCAL_HELD:
                case AOB_CALL_STATE_REMOTE_HELD:
                case AOB_CALL_STATE_LOCAL_AND_REMOTE_HELD:
                    break;
                case AOB_CALL_STATE_IDLE:
                    app_ble_audio_stop_ringtone(con_lid);
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

static void app_ble_audio_switch_focus_prepare(uint8_t local_tws_role)
{
    uint8_t count_streaming = app_ble_audio_count_music_streaming_device();
    audio_focus_req_info_t* top_focus = app_audio_focus_ctrl_stack_top();
    audio_focus_req_info_t* second_focus = app_audio_focus_ctrl_stack_find_specific_focus(1);
    BLE_AUDIO_SINK_DEVICE_T *device = NULL;
    uint8_t ble_role = local_tws_role;
    if(!top_focus || !second_focus)
    {
        TRACE(0,"prepare focus NULL swith exit!!");
        return;
    }

    if(ble_role != BLE_AUDIO_TWS_MASTER)
    {
        TRACE(1,"%s role error exit!", __func__);
    }

    if(bes_bt_tws_besaud_is_connected())
    {
        if(count_streaming < 2)
        {
            TRACE(1,"%s exit", __func__);
            return;
        }
        if (top_focus->device_info.audio_type == AUDIO_TYPE_LE_AUDIO && second_focus->device_info.audio_type == AUDIO_TYPE_LE_AUDIO)
        {
            //prepare switch focus data
            switch_focus_info.tobg_stream_type = top_focus->device_info.stream_type;
            switch_focus_info.tofg_stream_type = second_focus->device_info.stream_type;
            memcpy((void*)&switch_focus_info.bg_addr, (void*)&top_focus->device_info.device_addr, sizeof(bt_bdaddr_t));
            memcpy((void*)&switch_focus_info.fg_addr, (void*)&second_focus->device_info.device_addr, sizeof(bt_bdaddr_t));
            switch_focus_info.bg_addr.addr_type = 0;
            switch_focus_info.fg_addr.addr_type = 0;
            app_ble_sync_switch_focus_info((uint8_t *)&switch_focus_info, sizeof(SWITCH_FOCUS_INFO_T));
        }
    }
    else
    {
        if(count_streaming == 2)
        {
            //switch directly
            device = app_ble_audio_get_device(second_focus->device_info.device_idx);
            app_le_audio_toggle_cis_to_cis(device, second_focus->device_info.stream_type);
            TRACE(1,"%s switch directly!", __func__);
        }
    }
}

static void app_ble_audio_switch_focus_recv_data(uint8_t *data, uint8_t data_len)
{
    //check data size
    uint8_t len = sizeof(SWITCH_FOCUS_INFO_T);
    SWITCH_FOCUS_INFO_T *sw_focus_info =(SWITCH_FOCUS_INFO_T *)data;
    if(len != data_len)
    {
        TRACE(2,"focus len mismatch len!=data_len:%d!=%d", len, data_len);
    }
    else
    {
        switch_focus_info.tobg_stream_type = sw_focus_info->tobg_stream_type;
        switch_focus_info.tofg_stream_type = sw_focus_info->tofg_stream_type;
        memcpy((void*)&switch_focus_info.bg_addr, (void*)&sw_focus_info->bg_addr, sizeof(ble_bdaddr_t));
        memcpy((void*)&switch_focus_info.fg_addr, (void*)&sw_focus_info->fg_addr, sizeof(ble_bdaddr_t));
        switch_focus_info.bg_addr.addr_type = 0;
        switch_focus_info.fg_addr.addr_type = 0;
        TRACE(1,"%s done", __func__);
    }
}

void app_ble_audio_switch_cis_cmp_register(void (*cis_switch_cb)(uint8_t device_id))
{
    cis_switch_cmp_cb = cis_switch_cb;
}

void app_ble_audio_switch_focus(uint8_t local_tws_role)
{
    app_ble_audio_switch_focus_prepare(local_tws_role);
}

static void app_ble_audio_switch_focus_trigger_handler(void)
{
    audio_focus_req_info_t* top_focus = app_audio_focus_ctrl_stack_top();
    audio_focus_req_info_t* second_focus = app_audio_focus_ctrl_stack_find_specific_focus(1);
    BLE_AUDIO_SINK_DEVICE_T *device = NULL;
    uint8_t conid = 0xFF;
    if(!top_focus || !second_focus)
    {
        TRACE(0,"trigger focus NULL swith exit!!");
        return;
    }
    TRACE(2,"[SW_FOCUS]peer stream_type&addr:bg->fg(%d->%d)", switch_focus_info.tobg_stream_type, switch_focus_info.tofg_stream_type);
    DUMP8("%02x ", &switch_focus_info.bg_addr.addr, BT_ADDR_OUTPUT_PRINT_NUM);
    DUMP8("%02x ", &switch_focus_info.fg_addr.addr, BT_ADDR_OUTPUT_PRINT_NUM);
    TRACE(2,"[SW_FOCUS]local stream_type&addr:bg->fg(%d->%d)", top_focus->device_info.stream_type, second_focus->device_info.stream_type);
    DUMP8("%02x ", &top_focus->device_info.device_addr.address, BT_ADDR_OUTPUT_PRINT_NUM);
    DUMP8("%02x ", &second_focus->device_info.device_addr.address, BT_ADDR_OUTPUT_PRINT_NUM);
    //compare remote focus addr equal
    bool top_equal = (switch_focus_info.tobg_stream_type == top_focus->device_info.stream_type &&
                      memcmp((void*)&switch_focus_info.bg_addr, (void*)&top_focus->device_info.device_addr, sizeof(bt_bdaddr_t)) == 0);
    bool second_equal = (switch_focus_info.tofg_stream_type == second_focus->device_info.stream_type &&
                         memcmp((void*)&switch_focus_info.fg_addr, (void*)&second_focus->device_info.device_addr, sizeof(bt_bdaddr_t)) == 0);

    if(top_equal && second_equal)
    {
        conid = bes_ble_get_con_id_by_addr((const ble_bdaddr_t *)&switch_focus_info.fg_addr);
        device = app_ble_audio_get_device(conid);
        TRACE(2,"d%x[SW_FOCUS] stream_type:%d", conid, device->streamming_context);
        if(device && device->streamming_context != STREAMMING_INVALID)
        {
            app_le_audio_toggle_cis_to_cis(device, (AUDIO_USAGE_TYPE_E)switch_focus_info.tofg_stream_type);
        }
    }
    memset(&switch_focus_info, 0, sizeof(switch_focus_info));
}

uint8_t app_ble_get_curr_play_bleaudio_id(void)
{
    return curr_playing_le_id;
}

void app_ble_audio_gaf_media_status_handler_cb_register(void (*cb)(uint8_t con_lid, bool paused))
{
    if (cb)
    {
        gaf_media_status_handler_cb = cb;
    }
}

void app_ble_audio_sink_streaming_init(void)
{
    for (uint32_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        ble_audio_sink_device[i].con_lid = i;
        ble_audio_sink_device[i].streamming_context = STREAMMING_INVALID;
        ble_audio_sink_device[i].music_audio_focus  = AUDIOFOCUS_NONE;
        ble_audio_sink_device[i].cis_sink_ase_lid = 0xFF;
        ble_audio_sink_device[i].cis_src_ase_lid = 0xFF;

        ble_audio_sink_device[i].music_focus_changed_listener = app_ble_audio_on_music_focus_changed;

        ble_audio_sink_device[i].call_audio_focus = AUDIOFOCUS_NONE;
        ble_audio_sink_device[i].call_focus_changed_listener = app_ble_audio_on_phone_call_focus_changed;

        ble_audio_sink_device[i].game_audio_focus = AUDIOFOCUS_NONE;
        ble_audio_sink_device[i].game_focus_changed_listener = app_ble_audio_on_game_focus_changed;

        ble_audio_sink_device[i].ring_focus_changed_listener = app_ble_audio_on_phone_ring_focus_changed;
        ble_audio_sink_device[i].ring_audio_focus = AUDIOFOCUS_NONE;

        ble_audio_sink_device[i].set_as_bg_device = false;
        ble_audio_sink_device[i].media_play_status = AOB_MGR_PLAYBACK_STATE_INACTIVE;

        ble_audio_sink_device[i].bis_sink_stream_lid = 0xFF;
        ble_audio_sink_device[i].current_context_type = GAF_AUDIO_STREAM_CONTEXT_TYPE_MIN;
        osTimerInit(ble_audio_sink_device[i].ring_tone_play_timer_def, app_ble_audio_sink_ringtone_timer_handler);
        ble_audio_sink_device[i].ring_tone_play_timer = osTimerCreate(
                                                            &ble_audio_sink_device[i].ring_tone_play_timer_def.os_timer_def, osTimerOnce, (void *)&ble_audio_sink_device[i]);
        osTimerInit(ble_audio_sink_device[i].playback_status_check_timer_def, app_ble_audio_sink_playback_status_check_handler);
        ble_audio_sink_device[i].playback_status_check_timer = osTimerCreate(&ble_audio_sink_device[i].playback_status_check_timer_def.os_timer_def,
                                                                               osTimerOnce, (void *)&ble_audio_sink_device[i]);
    }
    memset(&switch_focus_info, 0, sizeof(switch_focus_info));
    app_ble_audio_policy_config();
    /// Audio manager register
    app_ble_audio_register_start_callback(app_ble_audio_sink_stream_start);
    app_ble_audio_register_stop_callback(app_ble_audio_sink_stream_stop);
    app_ble_audio_register_stop_single_stream_callback(gaf_audio_stream_refresh_and_stop);
    app_ble_sync_focus_switch_register(app_ble_audio_switch_focus_recv_data, app_ble_audio_switch_focus_trigger_handler);
}
