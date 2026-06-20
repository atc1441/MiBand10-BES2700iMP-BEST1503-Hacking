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
#include "hal_aud.h"
#include "hal_chipid.h"
#include "hal_codec.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_location.h"
#include "apps.h"
#include "app_thread.h"
#include "app_status_ind.h"
#include "bluetooth.h"
#include "me_api.h"
#include "nvrecord_bt.h"
#include "besbt.h"
#include "besbt_cfg.h"
#include "a2dp_api.h"
#include "hci_api.h"
#include "l2cap_api.h"
#include "hfp_api.h"
#include "dip_api.h"
#include "btapp.h"
#include "app_bt.h"
#include "app_hfp.h"
#include "audio_policy.h"
#include "app_bt_func.h"
#include "bt_drv_interface.h"
#include "bt_drv_reg_op.h"
#include "app_a2dp.h"
#include "app_dip.h"
#include "app_bt_media_manager.h"
#include "app_bt_stream.h"
#include "app_audio.h"
#include "bt_if.h"
#include "intersyshci.h"
#include "app_utils.h"
#include "ecc_p192.h"
#ifdef BLE_HOST_SUPPORT
#include "ecc_p256.h"
#endif

#if defined(BT_SOURCE)
#include "bt_source.h"
#endif

#if defined(__AI_VOICE__) || defined(BISTO_ENABLED)
#include "app_ai_manager_api.h"
#endif

#ifdef WATCH_AI_ENABLED
#include "app_ai_control.h"
#endif

#ifdef BT_HID_DEVICE
#include "app_bt_hid.h"
#endif

#if defined(IBRT)
#include "app_tws_ibrt.h"
#include "app_ibrt_internal.h"
#include "earbud_ux_api.h"
#include "app_tws_ibrt_cmd_sync_a2dp_status.h"
#include "app_ibrt_a2dp.h"
#include "app_tws_ctrl_thread.h"
#include "app_ibrt_nvrecord.h"
#include "earbud_profiles_api.h"
#include "earbud_ux_api.h"
#include "earbud_ux_duplicate_api.h"

#if defined(IBRT_UI)
#include "app_tws_ibrt_conn_api.h"
#include "app_custom_api.h"
#include "app_audio_control.h"
#endif
#endif

#ifdef __THIRDPARTY
#include "app_thirdparty.h"
#endif

#ifdef __IAG_BLE_INCLUDE__
#include "bluetooth_ble_api.h"
#endif

#ifdef GFPS_ENABLED
#include "bluetooth_ble_api.h"
#include "gfps.h"
#endif

#ifdef __AI_VOICE__
#include "ai_spp.h"
#include "ai_thread.h"
#include "ai_manager.h"
#endif

#ifdef __INTERCONNECTION__
#include "app_interconnection.h"
#include "app_interconnection_ble.h"
#include "spp_api.h"
#include "app_interconnection_ccmp.h"
#include "app_interconnection_spp.h"
#endif

#ifdef __INTERACTION__
#include "app_interaction.h"
#endif

#ifdef BISTO_ENABLED
#include "gsound_custom_bt.h"
#include "gsound_custom.h"
#if defined(IBRT)
#include "app_ai_tws.h"
#endif
#endif

#ifdef TILE_DATAPATH
#include "tile_target_ble.h"
#include "bluetooth_ble_api.h"
#endif

#if (A2DP_DECODER_VER >= 2)
#include "a2dp_decoder.h"
#endif
extern "C"
{
#include "ddbif.h"
}
#include "bluetooth_nv_mgr.h"

#if BLE_AUDIO_ENABLED
#include "bluetooth_ble_api.h"

#ifdef IS_BLE_AUDIO_DEBUG_INFO_COLLECTOR_ENABLED
#include "app_dbg_ble_audio_info.h"
#endif

#include "app_bt_stream.h"
#include "app_bt_media_manager.h"
#include "app_media_player.h"
#include "app_audio_control.h"
#include "app_audio_focus_control.h"
#include "aob_conn_api.h"
#include "app_audio_active_device_manager.h"
#endif
#if defined(__CONNECTIVITY_LOG_REPORT__)
#include "app_ibrt_link_monitor.h"
#endif

#ifdef __IAG_BLE_INCLUDE__
#include "ble_core_common.h"
#include "bes_gap_api.h"
#endif

#ifdef CUSTOM_BITRATE
#include "app_ibrt_customif_ui.h"
#endif

#include "audio_player_adapter.h"

extern "C" void gaf_bis_stream_dump_dma_trigger_status(void);
extern "C" void app_read_rpa_when_bt_connect(const bt_bdaddr_t *peer_addr);
extern "C" bool app_anc_work_status(void);

extern uint8_t bt_media_current_music_get(void);
extern uint8_t bt_media_current_sco_get(void);
extern void bt_media_clear_media_type(uint16_t media_type, int device_id);
extern void bt_media_clear_current_media(uint16_t media_type);

U16 bt_accessory_feature_feature = BTIF_HF_CUSTOM_FEATURE_SUPPORT;

#define APP_BT_PROFILE_RECONNECT_WAIT_SCO_DISC_MS (3000)

//reconnect = (INTERVAL+PAGETO)*CNT = (3000ms+5000ms)*15 = 120s
#define APP_BT_PROFILE_RECONNECT_RETRY_INTERVAL_MS (3000)
#define APP_BT_PROFILE_OPENNING_RECONNECT_RETRY_LIMIT_CNT   (2)
#define APP_BT_PROFILE_RECONNECT_RETRY_LIMIT_CNT (15)
#define APP_BT_PROFILE_CONNECT_RETRY_MS (10000)

static void app_bt_profile_reconnect_timehandler(void const *param);

osTimerDef (BT_PROFILE_CONNECT_TIMER0, app_bt_profile_reconnect_timehandler);
#if BT_DEVICE_NUM > 1
osTimerDef (BT_PROFILE_CONNECT_TIMER1, app_bt_profile_reconnect_timehandler);
#endif
#if BT_DEVICE_NUM > 2
osTimerDef (BT_PROFILE_CONNECT_TIMER2, app_bt_profile_reconnect_timehandler);
#endif

void app_bt_device_reconnect_timehandler(void const *param);
osTimerDef (BT_DEVICE_CONNECT_TIMER0, app_bt_device_reconnect_timehandler);
#if BT_DEVICE_NUM > 1
osTimerDef (BT_DEVICE_CONNECT_TIMER1, app_bt_device_reconnect_timehandler);
#endif
#if BT_DEVICE_NUM > 2
osTimerDef (BT_DEVICE_CONNECT_TIMER2, app_bt_device_reconnect_timehandler);
#endif

bt_pair_state_change_cb_t g_bt_pair_state_callback = NULL;
static bt_sink_event_global_cb_t bt_sink_event_global_callback = NULL;
static bt_audio_coex_event_cb_t bt_audio_coex_event_callback = NULL;
#ifdef IBRT
void app_bt_device_snoop_acl_connected(uint8_t device_id, void* remote, void* btm_conn);
void app_bt_device_snoop_acl_disconnected(uint8_t device_id, void* remote);
void app_bt_ibrt_reconnect_mobile_profile_flag_init(void);
static bool ibrt_reconnect_mobile_profile_flag[BT_DEVICE_NUM];
#endif

void app_bt_reset_profile_manager(struct app_bt_profile_manager *mgr)
{
    mgr->profile_connected = false;
    mgr->remote_support_a2dp = true;
    mgr->remote_support_hfp = true;
    mgr->hfp_connect = bt_profile_connect_status_unknow;
    mgr->a2dp_connect = bt_profile_connect_status_unknow;
    mgr->avrcp_connect = bt_profile_connect_status_unknow;
    mgr->reconnect_mode = bt_profile_reconnect_null;
    mgr->reconnect_cnt = 0;
    mgr->connectingState = APP_BT_IDLE_STATE;
    if(mgr->reconnect_timer != NULL)
    {
        osTimerStop(mgr->reconnect_timer);
    }
}

#define APP_BT_A2DP_PROMPT_DELAY_MS (3000)

void app_bt_init_config_postphase(struct app_bt_config *config);

static void app_bt_init_config(struct app_bt_config *config)
{
    uint32_t default_vol = hal_codec_get_default_dac_volume_index();

    config->a2dp_default_abs_volume = a2dp_convert_local_vol_to_bt_vol(default_vol);
#ifdef BT_HFP_SUPPORT
    config->hfp_default_volume = hfp_convert_local_vol_to_bt_vol(default_vol);
#endif
    config->a2dp_force_use_the_codec = BT_A2DP_CODEC_TYPE_INVALID;

    config->a2dp_force_use_prev_codec = false;

    config->hid_capture_non_invade_mode = false;

    config->music_preempt_play_mode = false;

    config->call_preempt_play_mode = false;

    config->dont_auto_play_bg_a2dp = false;

    config->second_sco_handle_mode = IBRT_REJECT_SECOND_SCO;

    config->a2dp_prompt_play_only_when_avrcp_play_received = false;

    config->a2dp_delay_prompt_play = false;

    config->a2dp_prompt_delay_ms = 0;

    config->mute_a2dp_stream = true;

    config->pause_a2dp_stream = false;

    config->close_a2dp_stream = false;

    config->keep_only_one_stream_close_connected_a2dp = false;

    config->pause_a2dp_when_call_exist = false;

    config->reconn_sco_if_fast_disc_after_call_active_for_iphone_auto_mode = false;

    config->reject_sco_req_within_a_certain_time_after_it_be_preempted = false;

    config->host_reject_unexcept_sco_packet = false;

    config->second_sco_bg_action = IBRT_ACTION_ROUTE_SCO_TO_PHONE;

    config->allow_duck_ringtone = true;

    config->virtual_call_handle = VIRTUAL_HANDLE_NORMAL_WAY;

    config->create_sco_for_call_active_device = true;
	
    config->pc_second_sco_discon_acl = true;

    config->second_sco_handle_mode = IBRT_ACCEPT_SECOND_SCO;

    config->dont_resume_music_when_preempted_by_another_music = false;

#if defined(SASS_ENABLED)
    config->call_preempt_play_mode = false;
    config->second_sco_handle_mode = IBRT_REJECT_SECOND_SCO;
#elif defined(AUTO_ACCEPT_SECOND_SCO)
    config->call_preempt_play_mode = false;
    config->second_sco_bg_action = IBRT_ACTION_HOLD_ACTIVE_SCO;
    config->second_sco_handle_mode = IBRT_ACCEPT_SECOND_SCO;
#else
    config->call_preempt_play_mode = true;
    config->second_sco_bg_action = IBRT_ACTION_ROUTE_SCO_TO_PHONE;
    config->second_sco_handle_mode = IBRT_HOST_DECIDE_SECONED_SCO;
#endif

#if defined(BT_HOST_REJECT_UNEXCEPT_SCO_PACKET)
    config->host_reject_unexcept_sco_packet = true;
#endif
#if defined(BT_PAUSE_A2DP_WHEN_CALL_EXIST)
    config->pause_a2dp_when_call_exist = true;
#endif
#if defined(BT_CREATE_SCO_FOR_ACTIVE_DEVICE)
    config->create_sco_for_call_active_device = true;
#endif

#if defined(BT_MUTE_A2DP) || defined(BT_CLOSE_A2DP) || defined(BT_KEEP_ONE_STREAM_CLOSE_CONNECTED_A2DP)
    config->music_preempt_play_mode = false;
    config->pause_a2dp_stream = false;
#endif

#if defined(BT_MUTE_A2DP)
    config->mute_a2dp_stream = true;
    config->pause_a2dp_stream = false;
    config->close_a2dp_stream = false;
#elif defined(BT_PAUSE_A2DP)
    config->pause_a2dp_stream = true;
    config->mute_a2dp_stream = false;
    config->close_a2dp_stream = false;
#elif defined(BT_CLOSE_A2DP)
    config->close_a2dp_stream = true;
    config->mute_a2dp_stream = false;
    config->pause_a2dp_stream = false;
#endif

    if (config->music_preempt_play_mode)
    {
#if defined(BT_DONT_AUTO_PLAY_BG_A2DP)
        config->dont_auto_play_bg_a2dp = true;
#endif

#if defined(A2DP_PROMPT_PLAY_ONLY_AVRCP_PLAY_RECEIVED)
        config->a2dp_prompt_play_only_when_avrcp_play_received = true;
#endif

#if defined(A2DP_DELAY_PROMPT_PLAY)
        config->a2dp_delay_prompt_play = true;
        config->a2dp_prompt_delay_ms = APP_BT_A2DP_PROMPT_DELAY_MS;
#endif
    }
#if defined(PROMPT_TONE_CANT_PREEMPT_MUSIC)
    config->prompt_tone_cant_preempt_music = true;
#else
    config->prompt_tone_cant_preempt_music = false;
#endif

#if defined(BT_BLEAUDIO_COEXIST)
    config->music_preempt_play_mode = false;
    config->call_preempt_play_mode = false;
    config->mute_a2dp_stream = true;
    config->second_sco_bg_action = IBRT_ACTION_HOLD_ACTIVE_SCO;
    config->second_sco_handle_mode = IBRT_ACCEPT_SECOND_SCO;
#endif

#if defined(BT_REJECT_SCO_REQ_WITHIN_A_CERTAIN_TIME_AFTER_IT_BE_PREEPMPTED)
    config->reject_sco_req_within_a_certain_time_after_it_be_preempted = true;
#endif // 
#if defined(BT_RESUME_PC_SCO_BY_DIS_THEN_CONN_HFP)
    config->resume_pc_sco_by_dis_then_conn_hfp = true;
#else
    config->resume_pc_sco_by_dis_then_conn_hfp = false;
#endif // BT_RESUME_PC_SCO_BY_DIS_THEN_CONN_HFP
}

#define A2DP_AUDIO_SYSFREQ_BOOST_CNT_FOR_RX_ACL_TOO_MANY 20
void app_bt_too_many_rx_acl_packets_pending_unhandled(void)
{
#ifndef BTH_SUBSYS_ONLY
    if (bt_a2dp_is_run())
    {
        a2dp_audio_sysfreq_boost_start(A2DP_AUDIO_SYSFREQ_BOOST_CNT_FOR_RX_ACL_TOO_MANY);
    }
#endif
}

struct BT_DEVICE_MANAGER_T app_bt_manager;

void app_bt_manager_init(void)
{
    struct BT_DEVICE_T *curr_device;
    struct BT_DEVICE_RECONNECT_T *reconnect_node;
    int i = 0;

    memset(&app_bt_manager, 0, sizeof(app_bt_manager));

    app_bt_init_config(&app_bt_manager.config);

    app_bt_init_config_postphase(&app_bt_manager.config);

    app_bt_manager.current_a2dp_conhdl = 0xffff;
    app_bt_manager.device_routed_sco_to_phone = BT_DEVICE_INVALID_ID;
    app_bt_manager.device_routing_sco_back = BT_DEVICE_INVALID_ID;
    app_bt_manager.prev_active_audio_link = 0xff;
    app_bt_manager.curr_playing_a2dp_id = BT_DEVICE_INVALID_ID;
    app_bt_manager.curr_playing_sco_id = BT_DEVICE_INVALID_ID;
    app_bt_manager.interrupted_a2dp_id = BT_DEVICE_INVALID_ID;
    app_bt_manager.wait_sco_connected_device_id = BT_DEVICE_INVALID_ID;
    app_bt_manager.trigger_sco_device_id = BT_DEVICE_INVALID_ID;

    initialize_list_head(&app_bt_manager.poweron_reconnect_list);
    initialize_list_head(&app_bt_manager.linkloss_reconnect_list);
    app_bt_audio_strategy_init();
    for (i = 0; i < BT_DEVICE_NUM; i += 1)
    {
        curr_device = app_bt_get_device(i);
        curr_device->device_id = i;
        curr_device->remember_interrupted_a2dp_for_a_while = BT_DEVICE_INVALID_ID;
        curr_device->a2dp_default_abs_volume = app_bt_manager.config.a2dp_default_abs_volume;
        curr_device->a2dp_current_abs_volume = app_bt_manager.config.a2dp_default_abs_volume;

        app_bt_reset_profile_manager(&curr_device->profile_mgr);

        if (i == 0)
        {
            curr_device->profile_mgr.reconnect_timer = osTimerCreate(osTimer(BT_PROFILE_CONNECT_TIMER0), osTimerOnce, &curr_device->profile_mgr);
        }
#if BT_DEVICE_NUM > 1
        else if (i == 1)
        {
            curr_device->profile_mgr.reconnect_timer = osTimerCreate(osTimer(BT_PROFILE_CONNECT_TIMER1), osTimerOnce, &curr_device->profile_mgr);
        }
#endif
#if BT_DEVICE_NUM > 2
        else if (i == 2)
        {
            curr_device->profile_mgr.reconnect_timer = osTimerCreate(osTimer(BT_PROFILE_CONNECT_TIMER2), osTimerOnce, &curr_device->profile_mgr);
        }
#endif

        reconnect_node = app_bt_manager.reconnect_node + i;

        if (i == 0)
        {
            reconnect_node->acl_reconnect_timer = osTimerCreate(osTimer(BT_DEVICE_CONNECT_TIMER0), osTimerOnce, reconnect_node);
        }
#if BT_DEVICE_NUM > 1
        else if (i == 1)
        {
            reconnect_node->acl_reconnect_timer = osTimerCreate(osTimer(BT_DEVICE_CONNECT_TIMER1), osTimerOnce, reconnect_node);
        }
#endif
#if BT_DEVICE_NUM > 2
        else if (i == 2)
        {
            reconnect_node->acl_reconnect_timer = osTimerCreate(osTimer(BT_DEVICE_CONNECT_TIMER2), osTimerOnce, reconnect_node);
        }
#endif
        app_bt_audio_init_focus_listener((void*)curr_device);
    }
#if (A2DP_DECODER_VER == 2)
    btif_me_register_pending_too_many_rx_acl_packets_callback(app_bt_too_many_rx_acl_packets_pending_unhandled);
#endif
#ifdef IBRT
    btif_me_register_snoop_acl_connection_callback(
        app_bt_device_snoop_acl_connected, app_bt_device_snoop_acl_disconnected);

    TRACE(3, "%s call_preempt_play_mode %d %d", __func__, app_bt_manager.config.call_preempt_play_mode, app_bt_manager.config.second_sco_handle_mode);

    if (app_bt_manager.config.call_preempt_play_mode)
    {
        if (app_bt_manager.config.second_sco_handle_mode == IBRT_ACCEPT_SECOND_SCO_DISC_FIRST_SCO)
        {
            bt_drv_reg_op_set_ibrt_second_sco_decision(0x00); // IBRT_REJECT_SECOND_SCO
        }
        else if (app_bt_manager.config.second_sco_handle_mode == IBRT_HOST_DECIDE_SECONED_SCO)
        {
            DEBUG_INFO(0, "host decide second sco");
            bt_drv_reg_op_set_ibrt_second_sco_decision(0x03);
        }
        else
        {
            bt_drv_reg_op_set_ibrt_second_sco_decision(0x01); // IBRT_ACCEPT_SECOND_SCO_DISC_FIRST_SCO
        }
    }
    else
    {
        if(app_bt_manager.config.second_sco_handle_mode == IBRT_REJECT_SECOND_SCO)
        {
            DEBUG_INFO(0, "auto reject second sco");
            bt_drv_reg_op_set_ibrt_second_sco_decision(0x00); // IBRT_REJECT_SECOND_SCO
        }
        else if (app_bt_manager.config.second_sco_handle_mode == IBRT_HOST_DECIDE_SECONED_SCO)
        {
            DEBUG_INFO(0, "host decide second sco");
            bt_drv_reg_op_set_ibrt_second_sco_decision(0x03);
        }
        else if(app_bt_manager.config.second_sco_handle_mode == IBRT_ACCEPT_SECOND_SCO)
        {
            if(bt_drv_is_support_set_bt_ble_active_link_switch())
            {
                DEBUG_INFO(0, "auto accept second sco");
                bt_drv_reg_op_set_ibrt_second_sco_decision(0x04);
            }
            else
            {
                DEBUG_INFO(0,"BT rom do not support dual sco feature");
            }
        }
        else
        {
            DEBUG_INFO(0,"switch to host decide second sco mode");
        }
    }
#endif
}

void app_bt_local_volume_up(void (*cb)(uint8_t device_id))
{
#ifndef BTH_SUBSYS_ONLY
    app_audio_manager_ctrl_volume_with_callback(APP_AUDIO_MANAGER_VOLUME_CTRL_UP, 0, cb);
#endif
}

void app_bt_local_volume_down(void (*cb)(uint8_t device_id))
{
#ifndef BTH_SUBSYS_ONLY
    app_audio_manager_ctrl_volume_with_callback(APP_AUDIO_MANAGER_VOLUME_CTRL_DOWN, 0, cb);
#endif
}

void app_bt_local_volume_set(uint16_t volume, void (*cb)(uint8_t device_id))
{
#ifndef BTH_SUBSYS_ONLY
    app_audio_manager_ctrl_volume_with_callback(APP_AUDIO_MANAGER_VOLUME_CTRL_SET, volume, cb);
#endif
}

#if defined(BT_AVRCP_SUPPORT) || defined(BT_A2DP_SUPPORT)

void app_bt_a2dp_current_abs_volume_just_set(int device_id, uint8_t volume)
{
    DEBUG_INFO(3, "(d%x) %s %d", device_id, __func__, volume);

    a2dp_volume_set((enum BT_DEVICE_ID_T)device_id, volume);
}

#endif

void app_bt_a2dp_abs_volume_mix_version_handled(int device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (curr_device)
    {
        DEBUG_INFO(4, "(d%x) %s init %d curr %d", device_id, __func__, curr_device->a2dp_initial_volume, curr_device->a2dp_current_abs_volume);

        if (curr_device->a2dp_current_abs_volume != curr_device->a2dp_initial_volume)
        {
#if defined(BT_AVRCP_SUPPORT) || defined(BT_A2DP_SUPPORT)
            app_bt_a2dp_send_volume_change(device_id);
#endif
        }
    }
}

#ifdef BT_A2DP_SUPPORT

void app_bt_set_a2dp_default_abs_volume(uint8_t volume)
{
    DEBUG_INFO(2, "%s %d", __func__, volume);
    app_bt_manager.config.a2dp_default_abs_volume = volume > 127 ? 127 : volume;
}

void app_bt_update_a2dp_default_abs_volume(int device_id, uint8_t volume)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    DEBUG_INFO(5, "(d%x) %s %d prev default %d %d", device_id, __func__, volume,
        curr_device ? curr_device->a2dp_default_abs_volume : 0, curr_device ? curr_device->a2dp_current_abs_volume : 0);
    if (curr_device && curr_device->a2dp_current_abs_volume == curr_device->a2dp_default_abs_volume)
    {
        curr_device->a2dp_default_abs_volume = volume > 127 ? 127 : volume;
        curr_device->a2dp_current_abs_volume = curr_device->a2dp_default_abs_volume;
    }
}

void app_bt_set_a2dp_current_abs_volume(int device_id, uint8_t volume)
{
    DEBUG_INFO(3, "(d%x) %s %d", device_id, __func__, volume);

    a2dp_volume_set(device_id, volume);

    app_bt_a2dp_send_volume_change(device_id); // report volume change to remote
}

uint8_t app_bt_get_a2dp_current_abs_volume(int device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    uint8_t volume = 0;
    if (curr_device)
    {
        volume = curr_device->a2dp_current_abs_volume;
    }
    else
    {
        volume = app_bt_manager.config.a2dp_default_abs_volume;
    }
    DEBUG_INFO(3, "(d%x) %s %d", device_id, __func__, volume);
    return volume;
}

uint8_t app_bt_a2dp_adjust_volume(uint8_t device_id, bool up, bool adjust_local_vol_level)
{
    AUD_ID_ENUM prompt_id = AUD_ID_INVALID;
    uint8_t a2dp_local_vol = a2dp_volume_local_get(device_id);

    if (up)
    {
        if(adjust_local_vol_level)
        {
            // get current local volume
            a2dp_local_vol++;
            if (a2dp_local_vol > TGT_VOLUME_LEVEL_MAX)
            {
                a2dp_local_vol = TGT_VOLUME_LEVEL_MAX;
                prompt_id = AUD_ID_BT_WARNING;
            }
        }
        else
        {
            // get current bt volume
            uint8_t currentBtVol = a2dp_abs_volume_get(device_id);

            // increase bt volume
            if (currentBtVol >= MAX_A2DP_VOL)
            {
                currentBtVol = MAX_A2DP_VOL;
                prompt_id = AUD_ID_BT_WARNING;

            }
            else
            {
                currentBtVol++;
                //prompt_id = AUD_ID_VOLUME_UP;
            }

            a2dp_volume_set(device_id, currentBtVol);

            a2dp_local_vol = a2dp_convert_bt_vol_to_local_vol(currentBtVol);
        }
    }
    else
    {
        if(adjust_local_vol_level)
        {
            // get current local volume
            if (a2dp_local_vol)
            {
                a2dp_local_vol--;
            }
            if (a2dp_local_vol <= TGT_VOLUME_LEVEL_MUTE)
            {
                a2dp_local_vol = TGT_VOLUME_LEVEL_MUTE;
                prompt_id = AUD_ID_BT_WARNING;
            }
        }
        else
        {
            // get current bt volume
            uint8_t currentBtVol = a2dp_abs_volume_get(device_id);

            if (currentBtVol <= 0)
            {
                currentBtVol = 0;
                prompt_id = AUD_ID_BT_WARNING;
            }
            else
            {
                currentBtVol--;
                //prompt_id = AUD_ID_VOLUME_DOWN;
            }

            a2dp_volume_set(device_id, currentBtVol);

            a2dp_local_vol = a2dp_convert_bt_vol_to_local_vol(currentBtVol);
        }
    }

    a2dp_volume_set_local_vol(device_id, a2dp_local_vol);

#if defined(IBRT)
    if (!app_ibrt_if_is_ui_slave())
#endif
    {
        if (prompt_id != AUD_ID_INVALID)
        {
            DEBUG_INFO(1, "AUD_ID=%d", prompt_id);
#ifdef MEDIA_PLAYER_SUPPORT
            audio_player_play_prompt(prompt_id, 0);
#endif
        }
    }

#ifndef FPGA
    nv_record_touch_cause_flush();
#endif

    return a2dp_local_vol;
}

#endif /* BT_A2DP_SUPPORT */


#ifdef BT_HFP_SUPPORT
uint8_t app_bt_hfp_adjust_volume(uint8_t device_id, bool up, bool adjust_local_vol_level)
{
    AUD_ID_ENUM prompt_id = AUD_ID_INVALID;
    uint8_t hfp_local_vol = hfp_volume_local_get(device_id);

    if (up)
    {
        if (adjust_local_vol_level)
        {
            // get current local volume
            hfp_local_vol++;
            if (hfp_local_vol > TGT_VOLUME_LEVEL_MAX)
            {
                hfp_local_vol = TGT_VOLUME_LEVEL_MAX;
                prompt_id = AUD_ID_BT_WARNING;
            }
        }
        else
        {
            // get current bt volume
            uint8_t currentBtVol = hfp_convert_local_vol_to_bt_vol(hfp_local_vol);

            // increase bt volume
            if (currentBtVol >= MAX_HFP_VOL)
            {
                currentBtVol = MAX_HFP_VOL;
                prompt_id = AUD_ID_BT_WARNING;
            }
            else
            {
                currentBtVol++;
                //prompt_id = AUD_ID_VOLUME_UP;
            }

            hfp_local_vol = hfp_convert_bt_vol_to_local_vol(currentBtVol);
        }
    }
    else
    {
        if(adjust_local_vol_level)
        {
            // get current local volume
            if (hfp_local_vol)
            {
                hfp_local_vol--;
            }
            if (hfp_local_vol <= TGT_VOLUME_LEVEL_MUTE)
            {
                hfp_local_vol = TGT_VOLUME_LEVEL_MUTE;
                prompt_id = AUD_ID_BT_WARNING;
            }
        }
        else
        {
            // get current bt volume
            uint8_t currentBtVol = hfp_convert_local_vol_to_bt_vol(hfp_local_vol);

            if (currentBtVol <= 0)
            {
                currentBtVol = 0;
                prompt_id = AUD_ID_BT_WARNING;
            }
            else
            {
                currentBtVol--;
                //prompt_id = AUD_ID_VOLUME_DOWN;
            }

            hfp_local_vol = hfp_convert_bt_vol_to_local_vol(currentBtVol);
        }
    }

    hfp_volume_local_set(device_id, hfp_local_vol);

#if defined(IBRT)
    if (!app_ibrt_if_is_ui_slave())
#endif
    {
        if (prompt_id != AUD_ID_INVALID)
        {
            DEBUG_INFO(1, "AUD_ID=%d", prompt_id);
#ifdef MEDIA_PLAYER_SUPPORT
            audio_player_play_prompt(prompt_id, 0);
#endif
        }
    }

#ifndef FPGA
    nv_record_touch_cause_flush();
#endif

    return hfp_local_vol;
}
#endif /* BT_HFP_SUPPORT */

uint8_t app_bt_a2dp_hfp_adjust_volume(uint8_t device_id, uint8_t stream_player, uint8_t vol)
{
    AUD_ID_ENUM prompt_id = AUD_ID_INVALID;

    if (stream_player & APP_BT_STREAM_HFP_PCM)
    {
        uint8_t hfp_local_vol = hfp_volume_local_get(device_id);
        hfp_local_vol = vol;
        if (hfp_local_vol > TGT_VOLUME_LEVEL_MAX)
        {
            hfp_local_vol = TGT_VOLUME_LEVEL_MAX;
            prompt_id = AUD_ID_BT_WARNING;
        }
        hfp_volume_local_set(device_id, hfp_local_vol);
    }
    else
    {
        uint8_t a2dp_local_vol = a2dp_volume_local_get(device_id);
        a2dp_local_vol = vol;
        if (a2dp_local_vol > TGT_VOLUME_LEVEL_MAX)
        {
            a2dp_local_vol = TGT_VOLUME_LEVEL_MAX;
            prompt_id = AUD_ID_BT_WARNING;
        }
        a2dp_volume_set_local_vol(device_id, a2dp_local_vol);
    }

#if defined(IBRT)
    if (!app_ibrt_if_is_ui_slave())
#endif
    {
        if (prompt_id != AUD_ID_INVALID)
        {
            DEBUG_INFO(1, "AUD_ID=%d", prompt_id);
#ifdef MEDIA_PLAYER_SUPPORT
            audio_player_play_prompt(prompt_id, 0);
#endif
        }
    }

#ifndef FPGA
    nv_record_touch_cause_flush();
#endif

    return vol;
}


static struct BT_DEVICE_RECONNECT_T *app_bt_get_poweron_reconnect_device(void)
{
    list_entry_t *head = &app_bt_manager.poweron_reconnect_list;
    list_entry_t *first = head->Flink;
    if (first && first != head)
    {
        return (struct BT_DEVICE_RECONNECT_T *)first;
    }
    else
    {
        return NULL;
    }
}

static struct BT_DEVICE_RECONNECT_T *app_bt_find_reconnect_device(bt_bdaddr_t *remote)
{
    struct BT_DEVICE_RECONNECT_T *reconnect = NULL;
    list_entry_t *head = &app_bt_manager.poweron_reconnect_list;
    list_entry_t *curr = head->Flink;

    for (; curr != head; curr = curr->Flink)
    {
        reconnect = (struct BT_DEVICE_RECONNECT_T *)curr;
        if (memcmp(&reconnect->rmt_addr, remote, sizeof(bt_bdaddr_t)) == 0)
        {
            return reconnect;
        }
    }

    head = &app_bt_manager.linkloss_reconnect_list;
    curr = head->Flink;

    for (; curr != head; curr = curr->Flink)
    {
        reconnect = (struct BT_DEVICE_RECONNECT_T *)curr;
        if (memcmp(&reconnect->rmt_addr, remote, sizeof(bt_bdaddr_t)) == 0)
        {
            return reconnect;
        }
    }

    return NULL;
}

static void app_bt_delete_reconnect_device(struct BT_DEVICE_RECONNECT_T *node)
{
    node->inuse = false;
    osTimerStop(node->acl_reconnect_timer);
    btif_remove_entry_list(&node->node);
}

void app_bt_start_connfail_reconnect(bt_bdaddr_t *remote, uint8_t errcode, bool is_for_source_device);
void app_bt_start_linkloss_reconnect(bt_bdaddr_t *remote, bool is_for_source_device);

static void app_bt_device_report_acl_connected(uint8_t errcode, btif_remote_device_t *rem_dev)
{
    uint8_t device_id = btif_me_get_device_id_from_rdev(rem_dev);
    struct BT_DEVICE_T* curr_device = NULL;
    struct BT_DEVICE_RECONNECT_T *reconnect = NULL;
    bt_bdaddr_t *remote = btif_me_get_remote_device_bdaddr(rem_dev);

    DEBUG_INFO(3, "%s errcode 0x%x device %x\n", __func__, errcode, device_id);

    if (device_id < BT_DEVICE_NUM)
    {
        curr_device = app_bt_get_device(device_id);

        if (errcode == BTIF_BEC_ACL_ALREADY_EXISTS)
        {
            return;
        }

        if (errcode == BTIF_BEC_NO_ERROR)
        {
            curr_device->acl_is_connected = true;
            curr_device->acl_conn_hdl = btif_me_get_remote_device_hci_handle(rem_dev);
            curr_device->btm_conn = rem_dev;
            curr_device->remote = *remote;
            curr_device->profile_mgr.rmt_addr = curr_device->remote;
            app_bt_reset_device_info(device_id);
            bt_audio_event_handler(device_id,APP_BT_AUDIO_EVENT_ACL_CONNECTED,0);

            app_bt_reset_profile_manager(&curr_device->profile_mgr);

            app_bt_set_connecting_profiles_state(device_id);

            reconnect = app_bt_find_reconnect_device(&curr_device->remote);
            if (reconnect)
            {
                curr_device->profile_mgr.reconnect_mode = reconnect->reconnect_mode;
                app_bt_delete_reconnect_device(reconnect);
            }
            else
            {
                curr_device->profile_mgr.reconnect_mode = bt_profile_reconnect_null;
            }
            return;
        }

        curr_device->acl_is_connected = false;

#ifdef RESUME_MUSIC_AFTER_CRASH_REBOOT
        app_bt_reset_curr_playback_device(device_id);
#endif
    }

#if defined IBRT && !defined(FREEMAN_ENABLED_STERO)
    if (device_id == BT_DEVICE_TWS_ID)
    {
        if (errcode == BTIF_BEC_NO_ERROR)
        {
            app_bt_manager.tws_conn.acl_is_connected = true;
            app_bt_manager.tws_conn.acl_conn_hdl = btif_me_get_remote_device_hci_handle(rem_dev);
            app_bt_manager.tws_conn.btm_conn = rem_dev;
            app_bt_manager.tws_conn.remote = *remote;
        }
        else
        {
            app_bt_manager.tws_conn.acl_is_connected = false;
            app_bt_manager.tws_conn.acl_conn_hdl = BT_INVALID_CONN_HANDLE;
            app_bt_manager.tws_conn.btm_conn = NULL;
        }
    }
#else
    if (errcode != BTIF_BEC_NO_ERROR)
    {
        app_bt_start_connfail_reconnect(remote, errcode, false);
    }
#endif
}

static void app_bt_device_report_acl_disconnected(uint8_t errcode, btif_remote_device_t *rem_dev)
{
    uint8_t device_id = btif_me_get_device_id_from_rdev(rem_dev);
    struct BT_DEVICE_T* curr_device = NULL;

    DEBUG_INFO(3, "%s errcode 0x%x device %x\n", __func__, errcode, device_id);

    if (device_id < BT_DEVICE_NUM)
    {
        curr_device = app_bt_get_device(device_id);
        curr_device->acl_is_connected = false;
        app_bt_reset_device_info(device_id);
        bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_ACL_DISC, 0);
        app_bt_reset_profile_manager(&curr_device->profile_mgr);

        app_bt_clear_connecting_profiles_state(device_id);

        app_bt_active_mode_reset(device_id);

#if defined(IBRT) && defined(IBRT_UI)
        app_ibrt_stop_profile_protect_timer(device_id);
#endif

#ifndef IBRT
        if (errcode == BTIF_BEC_CONNECTION_TIMEOUT ||
            errcode == BTIF_BEC_LMP_RESPONSE_TIMEOUT)
        {
            bt_bdaddr_t *remote = btif_me_get_remote_device_bdaddr(rem_dev);
            app_bt_start_linkloss_reconnect(remote, false);
        }
#endif
    }

#ifdef IBRT
    if (device_id == BT_DEVICE_TWS_ID)
    {
        app_bt_manager.tws_conn.acl_is_connected = false;
        app_bt_manager.tws_conn.btm_conn = NULL;
#ifdef BISTO_ENABLED
        gsound_on_tws_role_updated(app_ai_tws_get_local_role());
#endif
    }

    app_ibrt_internal_link_disconnected();
#endif
}

static void app_bt_device_report_authenticated(uint8_t errcode, btif_remote_device_t *rem_dev)
{
    uint8_t device_id = btif_me_get_device_id_from_rdev(rem_dev);

    DEBUG_INFO(3, "%s errcode 0x%x device %x\n", __func__, errcode, device_id);
    if(errcode == BTIF_BEC_NO_ERROR && device_id < BT_DEVICE_NUM)
    {
        bt_audio_event_handler(device_id,APP_BT_AUDIO_EVENT_AUTHENTICATED,0);
    }
}

#ifdef IBRT
void app_bt_device_snoop_acl_connected(uint8_t device_id, void* addr, void* btm_conn)
{
    struct BT_DEVICE_T* curr_device = NULL;
    bt_bdaddr_t *remote = (bt_bdaddr_t *)addr;
    uint16 connhdl = btif_me_get_remote_device_hci_handle((btif_remote_device_t *)btm_conn);

    DEBUG_INFO(9, "%s device %x %02x:%02x:%02x:%02x:%02x:%02x connhdl %04x", __func__, device_id,
          remote->address[0], remote->address[1], remote->address[2],
          remote->address[3], remote->address[4], remote->address[5], connhdl);

    if (device_id < BT_DEVICE_NUM)
    {
        curr_device = app_bt_get_device(device_id);
        curr_device->acl_is_connected = true;
        curr_device->acl_conn_hdl = connhdl;
        curr_device->btm_conn = (btif_remote_device_t *)btm_conn;
        curr_device->remote = *remote;
        curr_device->profile_mgr.rmt_addr = curr_device->remote;
        app_bt_reset_profile_manager(&curr_device->profile_mgr);
        app_bt_reset_device_info(device_id);
    }
}

void app_bt_device_snoop_acl_disconnected(uint8_t device_id, void* addr)
{
    struct BT_DEVICE_T* curr_device = NULL;
    POSSIBLY_UNUSED bt_bdaddr_t *remote = (bt_bdaddr_t *)addr;

    DEBUG_INFO(8, "%s device %x %02x:%02x:%02x:%02x:%02x:%02x\n", __func__, device_id,
          remote->address[0], remote->address[1], remote->address[2],
          remote->address[3], remote->address[4], remote->address[5]);

    if (device_id < BT_DEVICE_NUM)
    {
        curr_device = app_bt_get_device(device_id);
        curr_device->acl_is_connected = false;
        app_bt_reset_device_info(device_id);
        app_bt_reset_profile_manager(&curr_device->profile_mgr);
        bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_ACL_DISC, 0);
    }

    app_ibrt_internal_link_disconnected();
}
#endif

int bes_bt_cfg_vendor_codec_en(void)
{
    return besbt_cfg.vendor_codec_en;
}

int bes_bt_cfg_dont_play_mute_when_a2dp_stuck(void)
{
    return besbt_cfg.dont_play_mute_when_a2dp_stuck;
}

int bes_bt_cfg_mark_some_code_for_fuzz_test(void)
{
    return besbt_cfg.mark_some_code_for_fuzz_test;
}

int bes_bt_cfg_keep_only_one_stream_close_connected_a2dp(void)
{
    return app_bt_manager.config.keep_only_one_stream_close_connected_a2dp;
}

#ifdef IBRT
int bes_bt_gap_tws_acl_is_connected(void)
{
    return app_bt_manager.tws_conn.acl_is_connected;
}

uint8_t *bes_bt_gap_get_tws_remote_addr(void)
{
    return (uint8_t *)app_bt_manager.tws_conn.remote.address;
}
#endif

int bes_bt_a2dp_get_last_paused_device(void)
{
    return app_bt_manager.a2dp_last_paused_device;
}

int bes_bt_a2dp_set_last_paused_device(int device_id)
{
    int prev_device = app_bt_manager.a2dp_last_paused_device;
    app_bt_manager.a2dp_last_paused_device = (uint8_t)device_id;
    return prev_device;
}

struct BT_DEVICE_RECONNECT_T *app_bt_append_to_reconnect_list(bt_profile_reconnect_mode reconnect_mode, bt_bdaddr_t *remote, bool is_for_source_device)
{
    list_entry_t *head = NULL;
    list_entry_t *curr = NULL;
    struct BT_DEVICE_RECONNECT_T *node = NULL;
    struct BT_DEVICE_RECONNECT_T *new_node = NULL;

    if (reconnect_mode == bt_profile_reconnect_openreconnecting)
    {
        head = &app_bt_manager.poweron_reconnect_list;
    }
    else if (reconnect_mode == bt_profile_reconnect_reconnecting)
    {
        head = &app_bt_manager.linkloss_reconnect_list;
    }

    if (!head)
    {
        DEBUG_INFO(1, "%s no reconnect list", __func__);
        return NULL;
    }

    for (curr = head->Flink; curr != head; curr = curr->Flink)
    {
        node = (struct BT_DEVICE_RECONNECT_T *)curr;
        if (memcmp(&node->rmt_addr, remote, sizeof(bt_bdaddr_t)) == 0)
        {
            return node; // the device is already in the list
        }
    }
#ifdef BT_SOURCE
    for (int i = 0; i < (BT_DEVICE_NUM + BT_SOURCE_DEVICE_NUM); i += 1)
#else
    for (int i = 0; i < BT_DEVICE_NUM; i += 1)
#endif
    {
        if (!app_bt_manager.reconnect_node[i].inuse && app_bt_manager.reconnect_node[i].for_source_device == is_for_source_device)
        {
            new_node = &app_bt_manager.reconnect_node[i];
            break;
        }
    }

    if (new_node == NULL)
    {
        DEBUG_INFO(1, "%s no resource", __func__);
        return NULL;
    }

    new_node->inuse = true;
    new_node->reconnect_mode = reconnect_mode;
    new_node->acl_reconnect_cnt = 0;
    new_node->rmt_addr = *remote;
    insert_tail_list(head, &new_node->node);
    return new_node;
}

bool app_bt_is_device_profile_connected(uint8_t device_id)
{
    if (device_id < BT_DEVICE_NUM)
    {
        return app_bt_get_device(device_id)->profile_mgr.profile_connected;
    }
    else
    {
        // Indicate no connection is user passes invalid deviceId
        return false;
    }
}

bool app_bt_is_acl_connected(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (curr_device)
    {
        return curr_device->acl_is_connected;
    }
    else
    {
        return false;
    }
}

bool app_bt_is_sco_connected(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (curr_device)
    {
        return curr_device->acl_is_connected &&
               curr_device->hf_audio_state == BT_HFP_AUDIO_CON;
    }
    else
    {
        return false;
    }
}

uint8_t app_bt_get_device_id_byaddr(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T* curr_device = NULL;
    int i = 0;

    if (remote == NULL)
    {
        return BT_DEVICE_INVALID_ID;
    }

    for (i = 0; i < BT_DEVICE_NUM; i += 1)
    {
        curr_device = app_bt_get_device(i);
        if (memcmp(remote, &curr_device->remote, sizeof(bt_bdaddr_t)) == 0)
        {
            return i;
        }
    }

#if defined(BT_SOURCE)
    for (i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i += 1)
    {
        curr_device = app_bt_get_device(i);
        if (memcmp(remote, &curr_device->remote, sizeof(bt_bdaddr_t)) == 0)
        {
            return i;
        }
    }
#endif

#ifdef IBRT
    if (memcmp(remote, &app_bt_manager.tws_conn.remote, sizeof(bt_bdaddr_t)) == 0)
    {
        return BT_DEVICE_TWS_ID;
    }
#endif

    return BT_DEVICE_INVALID_ID;
}

uint16_t app_bt_get_handle_byaddr(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T* curr_device = NULL;
    int i = 0;

    if (remote == NULL)
    {
        return BT_INVALID_CONN_HANDLE;
    }

    for (i = 0; i < BT_DEVICE_NUM; i += 1)
    {
        curr_device = app_bt_get_device(i);
        if (memcmp(remote, &curr_device->remote, sizeof(bt_bdaddr_t)) == 0)
        {
            return curr_device->acl_conn_hdl;
        }
    }

#if defined(BT_SOURCE)
    for (i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i += 1)
    {
        curr_device = app_bt_get_device(i);
        if (memcmp(remote, &curr_device->remote, sizeof(bt_bdaddr_t)) == 0)
        {
            return curr_device->acl_conn_hdl;
        }
    }
#endif

    return BT_INVALID_CONN_HANDLE;
}

uint8_t* app_bt_get_addr_byhandle(uint16_t conn_hdl)
{
    struct BT_DEVICE_T* curr_device = NULL;
    int i = 0;

    for (i = 0; i < BT_DEVICE_NUM; i += 1)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->acl_conn_hdl == conn_hdl)
        {
            return curr_device->remote.address;
        }
    }

#if defined(BT_SOURCE)
    for (i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i += 1)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->acl_conn_hdl == conn_hdl)
        {
            return curr_device->remote.address;
        }
    }
#endif

    return NULL;
}

uint8_t app_bt_get_connected_device_id_byaddr(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T* curr_device = NULL;
    int i = 0;

    if (remote == NULL)
    {
        return BT_DEVICE_INVALID_ID;
    }

    for (i = 0; i < BT_DEVICE_NUM; i += 1)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->acl_is_connected && memcmp(remote, &curr_device->remote, sizeof(bt_bdaddr_t)) == 0)
        {
            return i;
        }
    }

#if defined(BT_SOURCE)
    for (i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i += 1)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->acl_is_connected && memcmp(remote, &curr_device->remote, sizeof(bt_bdaddr_t)) == 0)
        {
            return i;
        }
    }
#endif

#ifdef IBRT
    if (app_bt_manager.tws_conn.acl_is_connected &&
        memcmp(remote, &app_bt_manager.tws_conn.remote, sizeof(bt_bdaddr_t)) == 0)
    {
        return BT_DEVICE_TWS_ID;
    }
#endif

    return BT_DEVICE_INVALID_ID;
}

uint8_t app_bt_get_device_id_byhandle(uint16_t conn_handle)
{
    struct BT_DEVICE_T* curr_device = NULL;
    int i = 0;

    for (i = 0; i < BT_DEVICE_NUM; i += 1)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->acl_conn_hdl == conn_handle)
        {
            return i;
        }
    }

#if defined(BT_SOURCE)
    for (i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i += 1)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->acl_conn_hdl == conn_handle)
        {
            return i;
        }
    }
#endif

#ifdef IBRT
    if (conn_handle == app_bt_manager.tws_conn.acl_conn_hdl)
    {
        return BT_DEVICE_TWS_ID;
    }
#endif

    return BT_DEVICE_INVALID_ID;
}

uint16_t app_bt_get_handle_by_device_id(uint8_t device_id)
{
    uint16_t conn_handle = BT_INVALID_CONN_HANDLE;
    struct BT_DEVICE_T *curr_device = NULL;

#ifdef IBRT
    if (device_id == BT_DEVICE_TWS_ID)
    {
        return app_bt_manager.tws_conn.acl_is_connected ?
            app_bt_manager.tws_conn.acl_conn_hdl : BT_INVALID_CONN_HANDLE;
    }
#endif

    curr_device = app_bt_get_device(device_id);
    if (curr_device && curr_device->acl_is_connected)
    {
        conn_handle = curr_device->acl_conn_hdl;
    }

    return conn_handle;
}

bool app_bt_is_hfp_connected(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (curr_device)
    {
        return curr_device->acl_is_connected && curr_device->hf_conn_flag;
    }
    else
    {
        return false;
    }
}

bool app_bt_is_a2dp_connected(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (curr_device)
    {
        return curr_device->acl_is_connected && curr_device->a2dp_conn_flag;
    }
    else
    {
        return false;
    }
}

bool app_bt_is_profile_connected_before(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    return curr_device && curr_device->profiles_connected_before;
}

bool app_bt_is_acl_connected_byaddr(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T* curr_device = NULL;
    for (int i = 0; i < BT_DEVICE_NUM; i += 1)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->acl_is_connected &&
            memcmp(remote, &curr_device->remote, sizeof(bt_bdaddr_t)) == 0)
        {
            return true;
        }
    }
    return false;
}

bool app_bt_is_sco_connected_byaddr(bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T* curr_device = NULL;
    for (int i = 0; i < BT_DEVICE_NUM; i += 1)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->acl_is_connected &&
            curr_device->hf_audio_state == BT_HFP_AUDIO_CON &&
            memcmp(remote, &curr_device->remote, sizeof(bt_bdaddr_t)) == 0)
        {
            return true;
        }
    }
    return false;
}

bool app_bt_is_a2dp_connected_byaddr(bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T* curr_device = NULL;
    for (int i = 0; i < BT_DEVICE_NUM; i += 1)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->acl_is_connected &&
            curr_device->a2dp_conn_flag &&
            memcmp(remote, &curr_device->remote, sizeof(bt_bdaddr_t)) == 0)
        {
            return true;
        }
    }
    return false;
}

bool app_bt_is_hfp_connected_byaddr(bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T* curr_device = NULL;
    for (int i = 0; i < BT_DEVICE_NUM; i += 1)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->acl_is_connected &&
            curr_device->hf_conn_flag &&
            memcmp(remote, &curr_device->remote, sizeof(bt_bdaddr_t)) == 0)
        {
            return true;
        }
    }
    return false;
}

struct BT_DEVICE_T* app_bt_get_device(int i)
{
    if (i < BT_DEVICE_NUM)
    {
        return app_bt_manager.bt_devices + i;
    }
#if defined(BT_SOURCE)
    else if (i >= BT_SOURCE_DEVICE_ID_BASE && i < BT_SOURCE_DEVICE_ID_BASE + BT_SOURCE_DEVICE_NUM)
    {
        return app_bt_manager.source_base_devices + (i - BT_SOURCE_DEVICE_ID_BASE);
    }
#endif
    else
    {
        DEBUG_INFO(3,"%s invalid device id %02x ca=%p\n", __func__, i, __builtin_return_address(0));
        return NULL;
    }
}

void app_bt_response_acl_conn_req(bt_bdaddr_t *remote, bool accept)
{
    btif_me_response_acl_conn_req(remote, accept, BTIF_BEC_LIMITED_RESOURCE);
}

void app_bt_reconnect_hfp_profile(const bt_bdaddr_t *mobile_addr)
{
    app_bt_HF_CreateServiceLink(NULL, (bt_bdaddr_t *)mobile_addr);
}

void app_bt_disconnect_hfp_profile(btif_hf_channel_t *hfp)
{
    app_bt_HF_DisconnectServiceLink(hfp);
}

void app_bt_disconnect_a2dp_profile(a2dp_stream_t *a2dp)
{
    app_bt_A2DP_CloseStream(a2dp);
}

void app_bt_reconnect_avrcp_profile(const bt_bdaddr_t *mobile_addr)
{
#ifdef BT_AVRCP_SUPPORT
    TRACE(0, "app_bt_reconnect_avrcp_profile ");
    DUMP8("%02x ", mobile_addr, 6);
    struct bt_defer_param_t param_a = bt_alloc_param_size(mobile_addr, 6);
    bt_defer_call_func_1((uint32_t)btif_avrcp_connect, param_a);
#endif
}

void app_bt_disconnect_avrcp_profile(btif_avrcp_channel_t *avrcp)
{
#ifdef BT_AVRCP_SUPPORT
    app_bt_start_custom_function_in_bt_thread((uint32_t)avrcp, (uint32_t)NULL, (uint32_t)btif_avrcp_disconnect);
#endif /* BT_AVRCP_SUPPORT */
}

void app_bt_query_hfp_sdp_when_mobile_reconnect(uint8_t device_id)
{
#ifdef BT_HFP_SUPPORT
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (!curr_device || !curr_device->acl_is_connected)
    {
        DEBUG_INFO(0, "(d%x) app_bt_query_hfp_sdp: acl not connected", device_id);
        return;
    }

    if (btif_me_get_remote_device_initiator(curr_device->btm_conn))
    {
        DEBUG_INFO(0, "(d%x) app_bt_query_hfp_sdp: not incoming connect", device_id);
        return;
    }

    if (curr_device->hf_conn_flag)
    {
        DEBUG_INFO(0, "(d%x) app_bt_query_hfp_sdp: hfp already open", device_id);
        return;
    }

    if (btif_hfp_profile_is_connecting(device_id))
    {
        DEBUG_INFO(0, "(d%x) app_bt_query_hfp_sdp: hfp is connecting", device_id);
        return;
    }

    app_bt_start_custom_function_in_bt_thread((uint32_t)&curr_device->remote,
        (uint32_t)NULL, (uint32_t)btif_hf_query_remote_sdp);
#endif /* BT_HFP_SUPPORT */
}

void app_bt_reconnect_a2dp_profile(const bt_bdaddr_t *mobile_addr)
{
#ifdef BT_A2DP_SUPPORT
    nvrec_btdevicerecord *mobile_record = NULL;
    btif_avdtp_codec_t *prev_a2dp_codec = NULL;
    uint8_t force_use_codec = BT_A2DP_CODEC_TYPE_INVALID;

    if (app_bt_manager.config.a2dp_force_use_the_codec != BT_A2DP_CODEC_TYPE_INVALID)
    {
        force_use_codec = app_bt_manager.config.a2dp_force_use_the_codec;
    }
    else if (app_bt_manager.config.a2dp_force_use_prev_codec && !nv_record_btdevicerecord_find(mobile_addr, &mobile_record))
    {
        force_use_codec = mobile_record->device_plf.a2dp_codectype;
    }

    if (force_use_codec != BT_A2DP_CODEC_TYPE_INVALID)
    {
        if (force_use_codec == BT_A2DP_CODEC_TYPE_SBC)
        {
            prev_a2dp_codec = &a2dp_avdtpcodec;
        }
#if defined(A2DP_AAC_ON)
        else if (force_use_codec == BT_A2DP_CODEC_TYPE_MPEG2_4_AAC)
        {
            prev_a2dp_codec = &a2dp_aac_avdtpcodec;
        }
#endif
#if defined(A2DP_SCALABLE_ON)
        else if (force_use_codec == BT_A2DP_CODEC_TYPE_NON_A2DP)
        {
            prev_a2dp_codec = &a2dp_scalable_avdtpcodec;
        }
#endif
#if defined(A2DP_LHDC_ON)
        else if (force_use_codec == BT_A2DP_CODEC_TYPE_NON_A2DP)
        {
            prev_a2dp_codec = &a2dp_lhdc_avdtpcodec;
        }
#endif
#if defined(A2DP_LHDCV5_ON)
        else if (force_use_codec == BT_A2DP_CODEC_TYPE_NON_A2DP)
        {
            prev_a2dp_codec = &a2dp_lhdcv5_avdtpcodec;
        }
#endif
#if defined(A2DP_LDAC_ON)
        else if (force_use_codec == BT_A2DP_CODEC_TYPE_NON_A2DP)
        {
            prev_a2dp_codec = &a2dp_ldac_avdtpcodec;
        }
#endif
        else
        {
            DEBUG_INFO(2,"%s provided codec is invalid %d", __func__, force_use_codec);
            prev_a2dp_codec = NULL;
        }
    }

    app_bt_A2DP_OpenStream((a2dp_stream_t*)prev_a2dp_codec, (bt_bdaddr_t *)mobile_addr);
#endif /* BT_A2DP_SUPPORT */
}

#ifdef BT_A2DP_SUPPORT

const char* app_bt_a2dp_get_all_device_streaming_state(void)
{
    char *state = app_bt_get_global_state_buffer();
    struct BT_DEVICE_T* curr_device = NULL;
    int j = 0;
    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        state[j++] = curr_device->a2dp_streamming ? '1' : '0';
        state[j++] = ' ';
    }
    state[j] = '\0';
    return state;
}

const char* app_bt_a2dp_get_all_device_state(void)
{
    struct BT_DEVICE_T *curr_device = NULL;
    char* buffer = app_bt_get_global_state_buffer();
    int len = 0;
    len += sprintf(buffer, "(conn state strming playstat avrcp)=");
    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        len += sprintf(buffer+len, "(%d %d %d %d ar %d)",
                       curr_device->a2dp_conn_flag,
                       btif_a2dp_get_stream_state(curr_device->a2dp_connected_stream),
                       curr_device->a2dp_streamming,
                       curr_device->avrcp_playback_status,
                       curr_device->avrcp_conn_flag);
    }
    return buffer;
}

void app_bt_a2dp_state_checker(void)
{
    DEBUG_INFO(3,"a2dp_state: curr_playing_a2dp %x curr_a2dp_stream_id %x int_a2dp %x",
        app_bt_manager.curr_playing_a2dp_id, app_bt_manager.curr_a2dp_stream_id, app_bt_manager.interrupted_a2dp_id);
    DEBUG_INFO(1,"a2dp_state: %s", app_bt_a2dp_get_all_device_state());
}
#endif /* BT_A2DP_SUPPORT */

const char* app_bt_hf_get_all_device_state(void)
{
    struct BT_DEVICE_T* curr_device = NULL;
    char *state = app_bt_get_global_state_buffer();
    int len = 0;
    len += sprintf(state, "(conn call setup held audio)=");
    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        len += sprintf(state+len, "(%d %d %d %d %d)",
                       curr_device->hf_conn_flag, curr_device->hfchan_call, curr_device->hfchan_callSetup,
                       curr_device->hf_callheld, curr_device->hf_audio_state);
    }
    return state;
}

void app_bt_hfp_state_checker(void)
{
#ifdef BT_HFP_SUPPORT
    POSSIBLY_UNUSED struct BT_DEVICE_T* curr_device = app_bt_get_device(app_bt_audio_get_curr_hfp_device());

    DEBUG_INFO(3,"hfp_state: curr_playing_sco %x curr_hf_channel_id %x phone_earphone_mark %d",
          app_bt_manager.curr_playing_sco_id, app_bt_manager.curr_hf_channel_id, curr_device->switch_sco_to_earbud);

    DEBUG_INFO(1, "hfp_state: %s", app_bt_hf_get_all_device_state());
#endif
}

#if defined(__CONNECTIVITY_LOG_REPORT__)
extern "C" int32_t bt_syn_get_clkoffset(uint16_t conhdl);
#endif

bool app_bt_checker_print_link_state(const char* tag, btif_remote_device_t *btm_conn)
{
    btif_cmgr_handler_t *cmgr_handler = NULL;
    POSSIBLY_UNUSED bt_bdaddr_t *remote = NULL;
    rx_agc_t link_agc_info = {0};
    uint8_t chlMap[10] = {0};
    char strChlMap[32];
    int pos = 0;

    if (btm_conn && (cmgr_handler = btif_lock_free_cmgr_get_acl_handler(btm_conn)))
    {
        remote = btif_me_get_remote_device_bdaddr(btm_conn);
        uint16_t conhdl = btif_me_get_remote_device_hci_handle(btm_conn);
        int8_t tx_power_id = 0;

        TRACE(13, "link_state: %s [d%x] %02x:%02x:%02x:%02x:%02x:%02x hdl %x state %d role %d mode %d  interv %d",
              tag ? tag : "",
              btif_me_get_device_id_from_rdev(btm_conn),
              remote->address[0], remote->address[1], remote->address[2],
              remote->address[3], remote->address[4], remote->address[5],
              conhdl,
              btif_me_get_remote_device_state(btm_conn),
              btif_me_get_remote_device_role(btm_conn),
              btif_me_get_remote_device_mode(btm_conn),
              btif_cmgr_get_cmgrhandler_sniff_interval(cmgr_handler));

        bool ret = bt_drv_reg_op_read_rssi_in_dbm(conhdl, &link_agc_info);
        if(ret)
        {
            app_bt_get_tx_power_idx(conhdl, &tx_power_id);
            DEBUG_INFO(3, "%s RSSI=%d,RX gain =%d,TXpwr id=%d,Piconet CLK=0x%x", tag ? tag : "",link_agc_info.rssi,
                link_agc_info.rxgain, tx_power_id,bt_syn_get_curr_ticks(conhdl));

#if defined(__CONNECTIVITY_LOG_REPORT__)
            app_ibrt_if_update_rssi_info(tag, link_agc_info, btif_me_get_device_id_from_rdev(btm_conn));
            app_ibrt_if_save_bt_clkoffset(bt_syn_get_clkoffset(conhdl), btif_me_get_device_id_from_rdev(btm_conn));
#endif
        }

        if (0 == bt_drv_reg_op_acl_chnmap(conhdl, chlMap, sizeof(chlMap)))
        {
            for (uint8_t i = 0; i < 10; i++)
            {
                pos += format_string(strChlMap + pos, sizeof(strChlMap) - pos, "%02x ", chlMap[i]);
            }
            DEBUG_INFO(2, "%s chlMap %s", tag ? tag : "", strChlMap);
#if defined(__CONNECTIVITY_LOG_REPORT__)
            app_ibrt_if_update_chlMap_info(tag, chlMap, btif_me_get_device_id_from_rdev(btm_conn));
#endif
        }
        if (!tag)
        {
            return true;
        }
        if (strcmp(tag,"PEER TWS") == 0)
        {
            ret = bt_drv_reg_op_read_fa_rssi_in_dbm(&link_agc_info);
            if (ret)
            {
                TRACE(2, "FA RSSI=%d,RX gain =%d", link_agc_info.rssi,link_agc_info.rxgain);
            }
        }
        return true;
    }

    return false;
}

void app_bt_watch_profile_state_checker(void)
{
    BT_DEVICE_T *device = NULL;

    for (uint8_t i=0; i<BT_DEVICE_NUM; i++)
    {
        device = app_bt_get_device(i);
        if(device != NULL)
        {
            // bt_bdaddr_t bt_addr= device->remote;
            TRACE(9, "%s profile_state: [d%x] a2rvp: %d %d a2dp: %d %d hfp: %d %d %d",
                __FUNCTION__, device->device_id,
                device->avrcp_conn_flag, device->avrcp_playback_status,
                device->a2dp_conn_flag, device->a2dp_streamming,
                device->hf_conn_flag, device->hf_audio_state, device->hfchan_callSetup);
        }
        else
        {
            TRACE(2, "%s profile_state: [d%d] a2rvp: 0 0 a2dp: 0 0 hfp: 0 0 0", __FUNCTION__, i);
        }
    }
}

void app_bt_link_state_checker(void)
{
    btif_remote_device_t *btm_conn = NULL;
    const char* mobile_role_str = "IBRT_UNKNOWN";
    for (uint8_t i=0; i<BT_DEVICE_NUM; i++)
    {
        btm_conn = btif_me_enumerate_remote_devices(i);
        app_bt_checker_print_link_state(NULL, btm_conn);
        if(!btm_conn)
        {
            TRACE(2, "link_state: %s [d%x] 00:00:00:00:00:00 acl 0 state unknown", mobile_role_str, i);
        }
    }
}

void app_bt_reconnect_gatt_profile(const bt_bdaddr_t *mobile_addr)
{
#if defined(__GATT_OVER_BR_EDR__)
    app_bt_GATT_Connect(mobile_addr);
#endif
}

#ifdef IBRT
const char *app_bt_get_profile_exchanged_state(void)
{
    ibrt_mobile_info_t *p_mobile_info = NULL;
    struct BT_DEVICE_T *curr_device = NULL;
    char *buffer = app_bt_get_global_state_buffer();
    int len = 0;
    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        p_mobile_info = app_ibrt_conn_get_mobile_info_by_addr(&curr_device->remote);
        len += sprintf(buffer+len, "%d ", (p_mobile_info && app_tws_ibrt_is_profile_exchanged(&p_mobile_info->mobile_addr)) ? 1 : 0);
    }
    return buffer;
}

const char *app_bt_get_device_current_roles(void)
{
    ibrt_mobile_info_t *p_mobile_info = NULL;
    struct BT_DEVICE_T *curr_device = NULL;
    char *buffer = app_bt_get_global_state_buffer();
    int len = 0;
    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        p_mobile_info = app_ibrt_conn_get_mobile_info_by_addr(&curr_device->remote);
        if (p_mobile_info && p_mobile_info->current_role != IBRT_UNKNOW)
        {
            len += sprintf(buffer+len, "%d ", p_mobile_info->current_role);
        }
        else
        {
            len += sprintf(buffer+len, "%d ", IBRT_UNKNOW);
        }
    }
    return buffer;
}

#if defined(GET_PEER_RSSI_ENABLE)
void app_bt_ibrt_rssi_status_checker(void)
{
    struct BT_DEVICE_T *curr_device = NULL;
    ibrt_mobile_info_t *p_mobile_info = NULL;
    bt_bdaddr_t mobile_addr[BT_DEVICE_NUM] = {0,};
    bool is_need_get_peer_rssi = FALSE;

    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        p_mobile_info = app_ibrt_conn_get_mobile_info_by_addr(&curr_device->remote);
        if (p_mobile_info)
        {
            if (p_mobile_info->mobile_conhandle != 0 && p_mobile_info->mobile_conhandle != BT_INVALID_CONN_HANDLE)
            {
                if((p_mobile_info->profile_exchanged))
                {
                    is_need_get_peer_rssi = TRUE;
                    memcpy(&mobile_addr[i].address[0], &curr_device->remote.address[0], BTIF_BD_ADDR_SIZE);
                    DEBUG_INFO(0, "prepare get peer rssi dev(%d): %02x:%02x:*:*:*:%02x", i,
                        mobile_addr[i].address[0], mobile_addr[i].address[1], mobile_addr[i].address[5]);
                }
            }
        }
    }
    if (is_need_get_peer_rssi)
    {
        tws_ctrl_send_cmd(APP_TWS_CMD_GET_PEER_MOBILE_RSSI, (uint8_t *)&mobile_addr, sizeof(bt_bdaddr_t)*BT_DEVICE_NUM);
    }
}
#endif

void app_bt_ibrt_profile_state_checker(const char *tag, BT_DEVICE_T *device)
{
#ifdef IBRT_UI
    if(NULL != device)
    {
        bt_bdaddr_t bt_addr= device->remote;
        AppIbrtCallStatus call_state;
        POSSIBLY_UNUSED AppIbrtStatus call_status = app_ibrt_if_get_hfp_call_status(&bt_addr, &call_state);
        DEBUG_INFO(8, "%s profile_state: [d%x] a2rvp con: %d, %d, a2dp con: %d, %d, hfp conn: %d, %d",
              tag ? tag : "",  device->device_id,
              device->avrcp_conn_flag,  device->avrcp_playback_status,
              device->a2dp_conn_flag,  device->a2dp_streamming,
              device->hf_conn_flag, call_status == IBRT_PROFILE_STATUS_ERROR_INVALID_PARAMETERS ? 0xff: call_state);
    }
#endif
}

void app_bt_ibrt_mobile_link_state_checker(void)
{
    struct BT_DEVICE_T *curr_device = NULL;
    ibrt_mobile_info_t *p_mobile_info = NULL;
    bt_bdaddr_t mobile_addr[BT_DEVICE_NUM] = {0,};
    const char* mobile_role_str = "IBRT_UNKNOWN";

    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        p_mobile_info = app_ibrt_conn_get_mobile_info_by_addr(&curr_device->remote);
        if (p_mobile_info)
        {
            if (p_mobile_info->mobile_conhandle != 0 && p_mobile_info->mobile_conhandle != BT_INVALID_CONN_HANDLE)
            {
                mobile_role_str = "MASTER MOBILE";
                if((app_tws_ibrt_is_profile_exchanged(&curr_device->remote)))
                {
                    memcpy(&mobile_addr[i].address[0], &curr_device->remote.address[0], BTIF_BD_ADDR_SIZE);
                    DEBUG_INFO(4, "prepare get peer rssi dev(%d): %02x:%02x:*:*:*:%02x", i,
                        mobile_addr[i].address[0], mobile_addr[i].address[1], mobile_addr[i].address[5]);
                }
                if (app_bt_checker_print_link_state(mobile_role_str, p_mobile_info->p_mobile_remote_dev))
                {
                    app_bt_ibrt_profile_state_checker(mobile_role_str, curr_device);
                    continue;
                }
            }
            else if (p_mobile_info->ibrt_conhandle != 0 && p_mobile_info->ibrt_conhandle != BT_INVALID_CONN_HANDLE)
            {
                mobile_role_str = "SNOOP MOBILE";
                if (app_bt_checker_print_link_state(mobile_role_str, p_mobile_info->p_mobile_remote_dev))
                {
                    app_bt_ibrt_profile_state_checker(mobile_role_str, curr_device);
                    continue;
                }
            }
            else
            {
                mobile_role_str = "IBRT_UNKNOWN";
            }
        }
        DEBUG_INFO(8, "link_state: %s [d%x] %02x:%02x:%02x:%02x:%02x:%02x acl %d state unknown",
              mobile_role_str, i,
              mobile_addr[i].address[0], mobile_addr[i].address[0], mobile_addr[i].address[0],
              mobile_addr[i].address[0], mobile_addr[i].address[0], mobile_addr[i].address[0],
              curr_device->acl_is_connected);
    }
}

void app_bt_ibrt_tws_link_state_checker(void)
{
    POSSIBLY_UNUSED bt_bdaddr_t tws_addr = {.address = {0} };
    if(app_bt_manager.tws_conn.btm_conn && btif_lock_free_cmgr_get_acl_handler(app_bt_manager.tws_conn.btm_conn))
    {
        app_bt_checker_print_link_state("PEER TWS", app_bt_manager.tws_conn.btm_conn);
    }
    else
    {
        DEBUG_INFO(6, "PEER TWS: %02x:%02x:%02x:%02x:%02x:%02x state unknown",
              tws_addr.address[0], tws_addr.address[0], tws_addr.address[0],
              tws_addr.address[0], tws_addr.address[0], tws_addr.address[0]);
    }
}

bool app_bt_ibrt_has_mobile_link_connected(void)
{
    struct BT_DEVICE_T *curr_device = NULL;
    ibrt_mobile_info_t *p_mobile_info = NULL;
    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        p_mobile_info = app_ibrt_conn_get_mobile_info_by_addr(&curr_device->remote);
        if (p_mobile_info && p_mobile_info->mobile_conhandle != 0 && p_mobile_info->mobile_conhandle != BT_INVALID_CONN_HANDLE)
        {
            return true;
        }
    }
    return false;
}

bool app_bt_ibrt_has_snoop_link_connected(void)
{
    struct BT_DEVICE_T *curr_device = NULL;
    ibrt_mobile_info_t *p_mobile_info = NULL;
    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        p_mobile_info = app_ibrt_conn_get_mobile_info_by_addr(&curr_device->remote);
        if (p_mobile_info && p_mobile_info->ibrt_conhandle != 0 && p_mobile_info->ibrt_conhandle != BT_INVALID_CONN_HANDLE)
        {
            return true;
        }
    }
    return false;
}
#endif

#if defined(BT_SOURCE) && defined(A2DP_SOURCE_TEST)
void app_bt_source_link_state_checker(void)
{
    POSSIBLY_UNUSED struct BT_DEVICE_T *device = NULL;
    const char* mobile_role_str = "MASTER MOBILE";

    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        device = app_bt_get_device(i);
        if (device->acl_is_connected)
        {
             DEBUG_INFO(8, "%s profile_state: [d%x] a2rvp con: %d, %d, a2dp con: %d, %d, hfp conn: %d, %d",
              mobile_role_str,  device->device_id,
              device->avrcp_conn_flag,  device->avrcp_playback_status,
              device->a2dp_conn_flag,  device->a2dp_streamming,
              device->hf_conn_flag, device->hf_audio_state);
        }
    }
}
#endif

a2dp_stream_t * app_bt_get_mobile_a2dp_stream(uint32_t deviceId)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(deviceId);
    return curr_device->btif_a2dp_stream->a2dp_stream;
}

static void app_bt_precheck_before_starting_connecting(uint8_t isBtConnected);

#ifdef FPGA
void app_bt_accessmode_set_for_test(btif_accessible_mode_t mode)
{
    DEBUG_INFO(2,"%s %d",__func__, mode);
    app_bt_ME_SetAccessibleMode_Fortest(mode, NULL);
}

void app_bt_adv_mode_set_for_test(uint8_t en)
{

    DEBUG_INFO(2,"%s %d",__func__, en);
    app_bt_ME_Set_Advmode_Fortest(en);
}

void app_start_ble_adv_for_test(void)
{
    DEBUG_INFO(1,"%s",__func__);

    U8 adv_data[31];
    U8 adv_data_len = 0;
    U8 scan_rsp_data[31];
    U8 scan_rsp_data_len = 0;

    adv_data[adv_data_len++] = 2;
    adv_data[adv_data_len++] = 0x01;
    adv_data[adv_data_len++] = 0x1A;

    adv_data[adv_data_len++] = 17;
    adv_data[adv_data_len++] = 0xFF;

    adv_data[adv_data_len++] = 0x9A;
    adv_data[adv_data_len++] = 0x07;

    adv_data[adv_data_len++] = 0x10;
    adv_data[adv_data_len++] = 0x04;
    adv_data[adv_data_len++] = 0x06;

    adv_data[adv_data_len++] = 0x07;
    adv_data[adv_data_len++] = 0x00;

    adv_data[adv_data_len++] = 0x01;
    adv_data[adv_data_len++] = 0x98;

    adv_data[adv_data_len++] = 1;

    adv_data[adv_data_len++] = 0xFF;
    adv_data[adv_data_len++] = 0xFF;
    adv_data[adv_data_len++] = 0xFF;
    adv_data[adv_data_len++] = 0xFF;
    adv_data[adv_data_len++] = 0xFF;
    adv_data[adv_data_len++] = 0xFF;

    // Get default Device Name (No name if not enough space)
    const char* ble_name_in_nv = BLE_DEFAULT_NAME;
    uint32_t nameLen = strlen(ble_name_in_nv);
    // Get remaining space in the Advertising Data - 2 bytes are used for name length/flag
    int8_t avail_space = 31-adv_data_len;
    if(avail_space - 2 >= (int8_t)nameLen)
    {
        // Check if data can be added to the adv Data
        adv_data[adv_data_len++] = nameLen + 1;
        // Fill Device Name Flag
        adv_data[adv_data_len++] = '\x09';
        // Copy device name
        memcpy(&adv_data[adv_data_len], ble_name_in_nv, nameLen);
        // Update adv Data Length
        adv_data_len += nameLen;
        btif_me_ble_set_adv_data(adv_data_len, adv_data);

        btif_adv_para_struct_t adv_para;
        adv_para.interval_min = 32;
        adv_para.interval_max = 32;
        adv_para.adv_type = 0;
        adv_para.own_addr_type = 0;
        adv_para.peer_addr_type = 0;
        adv_para.adv_chanmap = 0x07;
        adv_para.adv_filter_policy = 0;
        btif_me_ble_set_adv_parameters(&adv_para);
        btif_me_set_ble_bd_address(ble_global_addr);
        btif_me_ble_set_adv_en(1);
    }
    else
    {
        nameLen = avail_space - 2;
        // Check if data can be added to the adv Data
        adv_data[adv_data_len++] = nameLen + 1;
        // Fill Device Name Flag
        adv_data[adv_data_len++] = '\x08';
        // Copy device name
        memcpy(&adv_data[adv_data_len], ble_name_in_nv, nameLen);
        // Update adv Data Length
        adv_data_len += nameLen;
        btif_me_ble_set_adv_data(adv_data_len, adv_data);

        btif_adv_para_struct_t adv_para;
        adv_para.interval_min = 256;
        adv_para.interval_max = 256;
        adv_para.adv_type = 0;
        adv_para.own_addr_type = 0;
        adv_para.peer_addr_type = 0;
        adv_para.adv_chanmap = 0x07;
        adv_para.adv_filter_policy = 0;
        btif_me_ble_set_adv_parameters(&adv_para);
        btif_me_set_ble_bd_address(ble_global_addr);

        avail_space = 31;
        nameLen = strlen(ble_name_in_nv);
        if(avail_space - 2 < (int8_t)nameLen)
            nameLen = avail_space - 2;

        scan_rsp_data[scan_rsp_data_len++] = nameLen + 1;
        // Fill Device Name Flag
        scan_rsp_data[scan_rsp_data_len++] = '\x09';
        // Copy device name
        memcpy(&scan_rsp_data[scan_rsp_data_len], ble_name_in_nv, nameLen);
        // Update Scan response Data Length
        scan_rsp_data_len += nameLen;
        btif_me_ble_set_scan_rsp_data(scan_rsp_data_len, scan_rsp_data);
        btif_me_ble_set_adv_en(1);
    }
}

void app_bt_write_controller_memory_for_test(uint32_t addr,uint32_t val,uint8_t type)
{
    DEBUG_INFO(2,"%s addr=0x%x val=0x%x type=%d",__func__, addr,val,type);
    app_bt_ME_Write_Controller_Memory_Fortest(addr,val,type);
}

void app_bt_read_controller_memory_for_test(uint32_t addr,uint32_t len,uint8_t type)
{
    DEBUG_INFO(2,"%s addr=0x%x len=%x type=%d",__func__, addr,len,type);
    app_bt_ME_Read_Controller_Memory_Fortest(addr,len,type);
}
#endif

enum
{
    INITIATE_PAIRING_NONE = 0,
    INITIATE_PAIRING_RUN = 1,
};
static uint8_t initiate_pairing = INITIATE_PAIRING_NONE;
void app_bt_connectable_state_set(uint8_t set)
{
    initiate_pairing = set;
}
bool is_app_bt_pairing_running(void)
{
    return (initiate_pairing == INITIATE_PAIRING_RUN)?(true):(false);
}

#define APP_DISABLE_PAGE_SCAN_AFTER_CONN
#ifdef APP_DISABLE_PAGE_SCAN_AFTER_CONN
osTimerId disable_page_scan_check_timer = NULL;
static void disable_page_scan_check_timer_handler(void const *param);
osTimerDef (DISABLE_PAGE_SCAN_CHECK_TIMER, (void (*)(void const *))disable_page_scan_check_timer_handler);                      // define timers
static void disable_page_scan_check_timer_handler(void const *param)
{
#if !defined(IBRT)
    if((app_bt_get_active_cons() >= BT_DEVICE_NUM) && (initiate_pairing == INITIATE_PAIRING_NONE))
    {
        app_bt_accessmode_set_req(BTIF_BAM_NOT_ACCESSIBLE);
    }
#endif
}

void disable_page_scan_check_timer_start(void)
{
    if(disable_page_scan_check_timer == NULL)
    {
        disable_page_scan_check_timer = osTimerCreate(osTimer(DISABLE_PAGE_SCAN_CHECK_TIMER), osTimerOnce, NULL);
    }
    osTimerStart(disable_page_scan_check_timer, 4000);
}
#endif

void PairingTransferToConnectable(void)
{
    int activeCons;

    activeCons = app_bt_get_active_cons();

    DEBUG_INFO(1,"%s",__func__);

    app_bt_connectable_state_set(INITIATE_PAIRING_NONE);
    if(activeCons == 0)
    {
        DEBUG_INFO(0,"!!!PairingTransferToConnectable  BAM_CONNECTABLE_ONLY\n");
        app_bt_accessmode_set_req(BTIF_BAM_CONNECTABLE_ONLY);
    }
}

int app_bt_state_checker(void)
{
    POSSIBLY_UNUSED uint8_t active_cons;

    active_cons = app_bt_get_active_cons();

    if (!besbt_cfg.bt_sink_enable)
    {
        return 0;
    }

    enum APP_SYSFREQ_FREQ_T bt_freq = (enum APP_SYSFREQ_FREQ_T)besmain_sysfreq_get();
    POSSIBLY_UNUSED enum APP_SYSFREQ_FREQ_T former_bt_freq = bt_freq;

    if(bt_freq < APP_SYSFREQ_52M)
    {
        bt_freq = APP_SYSFREQ_52M;
    }
    app_sysfreq_req(APP_SYSFREQ_USER_BT_MAIN, bt_freq);

#if defined(IBRT)
    POSSIBLY_UNUSED ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    DEBUG_INFO(3,"checker: TWS_PARAMS slot:%d, interval:0x%x, interval in sco:0x%x",p_ibrt_ctrl->acl_slot_num, p_ibrt_ctrl->acl_interval, p_ibrt_ctrl->acl_interval_in_sco);
    DEBUG_INFO(1,"checker: IBRT_MULTIPOINT FREQ=%d activeCons:%d profile_exchanged:%s",hal_sysfreq_get(),active_cons, app_bt_get_profile_exchanged_state());

    hal_sysfreq_print_user_freq();
    
    app_bt_ibrt_tws_link_state_checker();

    app_bt_ibrt_mobile_link_state_checker();

#ifdef BT_HFP_SUPPORT
    app_bt_hfp_state_checker();
#endif

#ifdef BT_A2DP_SUPPORT
    app_bt_a2dp_state_checker();
#endif

    audio_player_dump_playback_status();

    app_ibrt_internal_ctx_checker();

    hci_rx_flowctrl_info();
    hci_tx_flowctrl_info();

    //BT controller state checker
    ASSERT(bt_drv_reg_op_check_bt_controller_state(), "BT controller crash!");
#else
#ifdef IS_MULTI_AI_ENABLED
    DEBUG_INFO(1,"current_spec %d", app_ai_manager_get_current_spec());
#endif
    DEBUG_INFO(2,"%s app_bt_get_active_cons = %d",__func__,active_cons);

    app_bt_link_state_checker();

#ifdef BT_WATCH_APP
    app_bt_watch_profile_state_checker();
#endif

#if defined(BT_SOURCE) && defined(A2DP_SOURCE_TEST)
    app_bt_source_link_state_checker();
#endif

#ifdef BT_HFP_SUPPORT
    app_bt_hfp_state_checker();
#endif

#ifdef BT_A2DP_SUPPORT
    app_bt_a2dp_state_checker();
#endif

    audio_player_dump_playback_status();
#endif

    btdrv_compensate_pwr_based_on_temp();

#ifdef BLE_HOST_SUPPORT
    gap_dump_ble_status();
#endif

#if (BLE_AUDIO_ENABLED)
#ifdef AOB_MOBILE_ENABLED
    gaf_bis_stream_dump_dma_trigger_status();
#endif
    bes_ble_aob_conn_dump_state_info();
    app_bt_audio_control_state_checker();
    app_audio_adm_active_device_dump();
#endif
#ifdef IS_BLE_AUDIO_DEBUG_INFO_COLLECTOR_ENABLED
    app_dbg_dump_ble_info();
#endif   //IS_BLE_AUDIO_DEBUG_INFO_COLLECTOR_ENABLED

    app_sysfreq_req(APP_SYSFREQ_USER_BT_MAIN, former_bt_freq);

    return 0;
}

void app_bt_set_accessible_by_source(void)
{
    uint8_t active_cons = app_bt_get_active_cons();
    if( active_cons == 0)
    {
#ifdef __EARPHONE_STAY_BOTH_SCAN__
    app_bt_accessmode_set_req(BTIF_BT_DEFAULT_ACCESS_MODE_PAIR);
#else
    app_bt_accessmode_set_req(BTIF_BAM_CONNECTABLE_ONLY);
#endif
    }
    else if( active_cons < BT_DEVICE_NUM)
    {
        app_bt_accessmode_set_req(BTIF_BAM_CONNECTABLE_ONLY);
    }
    else
    {
        app_bt_accessmode_set_req(BTIF_BAM_NOT_ACCESSIBLE);
    }
}

#ifdef BT_SOURCE
extern void bt_source_set_accessmode_by_sink(void);
#endif

void app_bt_accessible_manager_process(const btif_event_t *Event)
{
#if defined(IBRT)
    //IBRT device's access mode will be controlled by UI
    return;
#else
    uint8_t active_cons;
    btif_event_type_t etype = btif_me_get_callback_event_type(Event);

    active_cons = app_bt_get_active_cons();
    switch (etype)
    {
        case BTIF_BTEVENT_LINK_CONNECT_CNF:
        case BTIF_BTEVENT_LINK_CONNECT_IND:
            DEBUG_INFO(1,"BTEVENT_LINK_CONNECT_IND/CNF activeCons:%d", active_cons);
#if defined(__BTIF_EARPHONE__)   && !defined(FPGA)
            app_stop_10_second_timer(APP_PAIR_TIMER_ID);
#endif
            if( active_cons == 0)
            {
#ifdef __EARPHONE_STAY_BOTH_SCAN__
                app_bt_accessmode_set_req(BTIF_BT_DEFAULT_ACCESS_MODE_PAIR);
#else
                app_bt_accessmode_set_req(BTIF_BAM_CONNECTABLE_ONLY);
#endif
            }
            else if( active_cons < BT_DEVICE_NUM)
            {
                app_bt_accessmode_set_req(BTIF_BAM_CONNECTABLE_ONLY);
            }
            else
            {
                #ifdef BT_SOURCE
                if(app_bt_source_is_enabled())
                {
                    bt_source_set_accessmode_by_sink();
                }
                else
                #endif
                {
                    app_bt_accessmode_set_req(BTIF_BAM_NOT_ACCESSIBLE);
                }
            }
            break;
        case BTIF_BTEVENT_LINK_DISCONNECT:
            DEBUG_INFO(1,"DISCONNECT activeCons:%d", active_cons);
#ifdef __EARPHONE_STAY_BOTH_SCAN__
            if( active_cons == 0)
            {
                app_bt_accessmode_set_req(BTIF_BT_DEFAULT_ACCESS_MODE_PAIR);
            }
            else if( active_cons < BT_DEVICE_NUM)
            {
                app_bt_accessmode_set_req(BTIF_BAM_CONNECTABLE_ONLY);
            }
            else
            {
                app_bt_accessmode_set_req(BTIF_BAM_NOT_ACCESSIBLE);
            }
#else
            app_bt_accessmode_set_req(BTIF_BAM_CONNECTABLE_ONLY);
#endif
            break;
        case BTIF_BTEVENT_SCO_CONNECT_IND:
        case BTIF_BTEVENT_SCO_CONNECT_CNF: // dont accept connection when sco connected
            if( active_cons == 1 && BT_DEVICE_NUM > 1)
            {
                app_bt_accessmode_set_req(BTIF_BAM_NOT_ACCESSIBLE);
            }
            break;
        case BTIF_BTEVENT_SCO_DISCONNECT:
            if( active_cons == 1 && BT_DEVICE_NUM > 1)
            {
                app_bt_accessmode_set_req(BTIF_BAM_CONNECTABLE_ONLY);
            }
            break;
        default:
            break;
    }
#endif
}

/**
 * dont role switch immediately this will lead iphone8 disc acl link
 * retry it later when hfp profile and a2dp profile connected
 */
#define APP_BT_SWITCHROLE_LIMIT (1)

void app_bt_role_manager_process(const btif_event_t *Event)
{
#if defined(IBRT)
    return;
#else
    static uint8_t switchrole_cnt = 0;
    btif_event_type_t etype = btif_me_get_callback_event_type(Event);
    uint8_t active_cons;

    active_cons = app_bt_get_active_cons();
    //on phone connecting
    uint16_t conn_handle = btif_me_get_callback_event_handle(Event);
    switch (etype)
    {
        case BTIF_BTEVENT_LINK_CONNECT_IND:
            if(  btif_me_get_callback_event_err_code(Event) == BTIF_BEC_NO_ERROR)
            {
                if (active_cons == 1)
                {
                    switch ( btif_me_get_callback_event_rem_dev_role (Event))
                    {
#if defined(__SET_OUR_AS_MASTER__)
                        case BTIF_BCR_SLAVE:
                        case BTIF_BCR_PSLAVE:
#else
                        case BTIF_BCR_MASTER:
                        case BTIF_BCR_PMASTER:
#endif
                            DEBUG_INFO(1,"CONNECT_IND try to handle:%d\n", conn_handle);
                            //curr connectrot try to role
                            switchrole_cnt = 0;
                            app_bt_Me_SetLinkPolicy(conn_handle, BTIF_BLP_MASTER_SLAVE_SWITCH|BTIF_BLP_SNIFF_MODE);
                            break;
#if defined(__SET_OUR_AS_MASTER__)
                        case BTIF_BCR_MASTER:
                        case BTIF_BCR_PMASTER:
#else
                        case BTIF_BCR_SLAVE:
                        case BTIF_BCR_PSLAVE:
#endif
                        case BTIF_BCR_ANY:
                        case BTIF_BCR_UNKNOWN:
                        default:
                            DEBUG_INFO(1,"CONNECT_IND disable handle:%d\n",conn_handle);
                            //disable roleswitch when 1 connect
                            app_bt_Me_SetLinkPolicy(conn_handle, BTIF_BLP_SNIFF_MODE);
                            break;
                    }
                }
                else if (active_cons > 1)
                {
                    switch (btif_me_get_callback_event_rem_dev_role (Event))
                    {
                        case BTIF_BCR_MASTER:
                        case BTIF_BCR_PMASTER:
                            DEBUG_INFO(1,"CONNECT_IND disable handle:%d\n",conn_handle);
                            //disable roleswitch
                            app_bt_Me_SetLinkPolicy(conn_handle, BTIF_BLP_SNIFF_MODE);
                            break;
                        case BTIF_BCR_SLAVE:
                        case BTIF_BCR_PSLAVE:
                        case BTIF_BCR_ANY:
                        case BTIF_BCR_UNKNOWN:
                        default:
                            //disconnect slave
                            DEBUG_INFO(1,"CONNECT_IND disconnect handle:%d\n",conn_handle);
                            app_bt_MeDisconnectLink(conn_handle);
                            break;
                    }
                }
            }
            break;
        case BTIF_BTEVENT_LINK_CONNECT_CNF:
            if (active_cons == 1)
            {
                switch (btif_me_get_callback_event_rem_dev_role (Event))
                {
#if defined(__SET_OUR_AS_MASTER__)
                    case BTIF_BCR_SLAVE:
                    case BTIF_BCR_PSLAVE:
#else
                    case BTIF_BCR_MASTER:
                    case BTIF_BCR_PMASTER:
#endif
                        DEBUG_INFO(1,"CONNECT_CNF try to handle:%d\n",conn_handle);
                        //curr connectrot try to role
                        switchrole_cnt = 0;
                        app_bt_Me_SetLinkPolicy(conn_handle, BTIF_BLP_MASTER_SLAVE_SWITCH|BTIF_BLP_SNIFF_MODE);
                        app_bt_ME_SwitchRole(conn_handle);
                        break;
#if defined(__SET_OUR_AS_MASTER__)
                    case BTIF_BCR_MASTER:
                    case BTIF_BCR_PMASTER:
#else
                    case BTIF_BCR_SLAVE:
                    case BTIF_BCR_PSLAVE:
#endif
                    case BTIF_BCR_ANY:
                    case BTIF_BCR_UNKNOWN:
                    default:
                        DEBUG_INFO(1,"CONNECT_CNF disable role, handle:%d\n",conn_handle);
                        //disable roleswitch
                        app_bt_Me_SetLinkPolicy(conn_handle, BTIF_BLP_SNIFF_MODE);
                        break;
                }
            }
            else if (active_cons > 1)
            {
                switch (btif_me_get_callback_event_rem_dev_role (Event))
                {
                    case BTIF_BCR_MASTER:
                    case BTIF_BCR_PMASTER :
                        DEBUG_INFO(1,"CONNECT_CNF disable handle:%d\n",conn_handle);
                        //disable roleswitch
                        app_bt_Me_SetLinkPolicy(conn_handle, BTIF_BLP_SNIFF_MODE);
                        break;
                    case BTIF_BCR_SLAVE:
                    case BTIF_BCR_ANY:
                    case BTIF_BCR_UNKNOWN:
                    default:
                        //disconnect slave
                        DEBUG_INFO(1,"CONNECT_CNF disconnect handle:%d\n",conn_handle);
                        app_bt_MeDisconnectLink(conn_handle);
                        break;
                }
            }
            break;
        case BTIF_BTEVENT_LINK_DISCONNECT:
            switchrole_cnt = 0;
            break;
        case BTIF_BTEVENT_ROLE_CHANGE:
            switch ( btif_me_get_callback_event_role_change_new_role(Event))
            {
#if defined(__SET_OUR_AS_MASTER__)
                case BTIF_BCR_SLAVE:
#else
                case BTIF_BCR_MASTER:
#endif
                    if (++switchrole_cnt<=APP_BT_SWITCHROLE_LIMIT)
                    {
                        app_bt_ME_SwitchRole(conn_handle);
                    }
                    else
                    {
#if defined(__SET_OUR_AS_MASTER__)
                        DEBUG_INFO(2,"ROLE TO MASTER FAILED handle:%d cnt:%d\n", conn_handle, switchrole_cnt);
#else
                        DEBUG_INFO(2,"ROLE TO SLAVE FAILED handle:%d cnt:%d\n", conn_handle, switchrole_cnt);
#endif
                        switchrole_cnt = 0;
                    }
                    break;
#if defined(__SET_OUR_AS_MASTER__)
                case BTIF_BCR_MASTER:
                    DEBUG_INFO(2,"ROLE TO MASTER SUCCESS handle:%d cnt:%d\n", conn_handle, switchrole_cnt);
#else
                case BTIF_BCR_SLAVE:
                    DEBUG_INFO(2,"ROLE TO SLAVE SUCCESS handle:%d cnt:%d\n", conn_handle, switchrole_cnt);
#endif
                    switchrole_cnt = 0;
                    app_bt_Me_SetLinkPolicy(conn_handle,BTIF_BLP_SNIFF_MODE);
                    break;
                case BTIF_BCR_ANY:
                    break;
                case BTIF_BCR_UNKNOWN:
                    break;
                default:
                    break;
            }

            if (active_cons > 1)
            {
                uint8_t slave_cnt = 0;
                for (uint8_t i=0; i<BT_DEVICE_NUM; i++)
                {
                    btif_remote_device_t *remDev = btif_me_enumerate_remote_devices(i);
                    if ( btif_me_get_current_role(remDev) == BTIF_BCR_SLAVE)
                    {
                        slave_cnt++;
                    }
                }
                if (slave_cnt>1)
                {
                    DEBUG_INFO(1,"ROLE_CHANGE disconnect slave: handle %04x", conn_handle);
                    app_bt_MeDisconnectLink(conn_handle);
                }
            }
            break;
        default:
            break;
    }
#endif
}

void app_bt_switch_role_if_needed(const bt_bdaddr_t *remote)
{
#if defined(IBRT)
    return;
#else
    btif_remote_device_t *btm_conn = app_bt_get_remote_dev_by_address(remote);
    if (btm_conn == NULL)
    {
        return;
    }
    int current_role = btif_me_get_remote_device_role(btm_conn);
    uint16_t conn_handle = btif_me_get_remote_device_hci_handle(btm_conn);
#if defined(__SET_OUR_AS_MASTER__)
    if (current_role == BTIF_BCR_SLAVE || current_role == BTIF_BCR_PSLAVE)
#else
    if (current_role == BTIF_BCR_MASTER || current_role == BTIF_BCR_PMASTER)
#endif
    {
        app_bt_Me_SetLinkPolicy(conn_handle, BTIF_BLP_MASTER_SLAVE_SWITCH|BTIF_BLP_SNIFF_MODE);
        app_bt_ME_SwitchRole(conn_handle);
    }
#endif
}

void app_bt_role_manager_process_dual_slave(const btif_event_t *Event)
{
#if defined(IBRT)
    return;
#else
    static uint8_t switchrole_cnt = 0;
    btif_event_type_t etype = btif_me_get_callback_event_type(Event);
    //on phone connecting
    uint16_t conn_handle = btif_me_get_callback_event_handle(Event);
    switch (etype)
    {
        case BTIF_BTEVENT_LINK_CONNECT_IND:
        case BTIF_BTEVENT_LINK_CONNECT_CNF:
            if(btif_me_get_callback_event_err_code(Event) == BTIF_BEC_NO_ERROR)
            {
                switch (btif_me_get_callback_event_rem_dev_role (Event))
                {
#if defined(__SET_OUR_AS_MASTER__)
                    case BTIF_BCR_SLAVE:
                    case BTIF_BCR_PSLAVE:
#else
                    case BTIF_BCR_MASTER:
                    case BTIF_BCR_PMASTER:
#endif
                        DEBUG_INFO(1,"CONNECT_IND/CNF try to role: handle %04x", conn_handle);
                        switchrole_cnt = 0;
                        app_bt_Me_SetLinkPolicy( conn_handle, BTIF_BLP_MASTER_SLAVE_SWITCH|BTIF_BLP_SNIFF_MODE);
                        app_bt_ME_SwitchRole( conn_handle);
                        break;
#if defined(__SET_OUR_AS_MASTER__)
                    case BTIF_BCR_MASTER:
                    case BTIF_BCR_PMASTER:
#else
                    case BTIF_BCR_SLAVE:
                    case BTIF_BCR_PSLAVE:
#endif
                    case BTIF_BCR_ANY:
                    case BTIF_BCR_UNKNOWN:
                    default:
                        DEBUG_INFO(1,"CONNECT_IND disable role: handle %04x", conn_handle);
                        app_bt_Me_SetLinkPolicy( conn_handle, BTIF_BLP_SNIFF_MODE);
                        break;
                }
            }
            break;
        case BTIF_BTEVENT_LINK_DISCONNECT:
            switchrole_cnt = 0;
            break;
        case BTIF_BTEVENT_ROLE_CHANGE:
            switch (btif_me_get_callback_event_role_change_new_role(Event))
            {
#if defined(__SET_OUR_AS_MASTER__)
                case BTIF_BCR_SLAVE:
#else
                case BTIF_BCR_MASTER:
#endif
                    if (++switchrole_cnt<=APP_BT_SWITCHROLE_LIMIT)
                    {
                        DEBUG_INFO(1,"ROLE_CHANGE try to role again: %d", switchrole_cnt);
                        app_bt_Me_SetLinkPolicy(conn_handle, BTIF_BLP_MASTER_SLAVE_SWITCH|BTIF_BLP_SNIFF_MODE);
                        app_bt_ME_SwitchRole(conn_handle);
                    }
                    else
                    {
#if defined(__SET_OUR_AS_MASTER__)
                        DEBUG_INFO(2,"ROLE TO MASTER FAILED: handle %04x cnt:%d\n",conn_handle, switchrole_cnt);
#else
                        DEBUG_INFO(2,"ROLE TO SLAVE FAILED: handle %04x cnt:%d\n", conn_handle, switchrole_cnt);
#endif
                        switchrole_cnt = 0;
                    }
                    break;
#if defined(__SET_OUR_AS_MASTER__)
                case BTIF_BCR_MASTER:
                    DEBUG_INFO(2,"ROLE TO MASTER SUCCESS: handle %04x cnt:%d\n", conn_handle, switchrole_cnt);
#else
                case BTIF_BCR_SLAVE:
                    DEBUG_INFO(2,"ROLE TO SLAVE SUCCESS: handle %04x cnt:%d\n", conn_handle, switchrole_cnt);
#endif
                    switchrole_cnt = 0;
                    //workaround for power reset opening reconnect sometime unsuccessfully in sniff mode,
                    //only after authentication completes, enable sniff mode.
                    if (btif_me_get_remote_device_auth_state(btif_me_get_callback_event_rem_dev(Event)) == BTIF_BAS_AUTHENTICATED)
                    {
                        app_bt_Me_SetLinkPolicy( conn_handle, BTIF_BLP_SNIFF_MODE);
                    }
                    else
                    {
                        app_bt_Me_SetLinkPolicy( conn_handle, BTIF_BLP_DISABLE_ALL);
                    }
                    break;
                case BTIF_BCR_ANY:
                    break;
                case BTIF_BCR_UNKNOWN:
                    break;
                default:
                    break;
            }
            break;
    }
#endif
}

void app_bt_sniff_config(uint16_t conn_handle)
{
    btif_sniff_info_t sniffInfo;
    sniffInfo.maxInterval = BTIF_CMGR_SNIFF_MAX_INTERVAL;
    sniffInfo.minInterval = BTIF_CMGR_SNIFF_MIN_INTERVAL;
    sniffInfo.attempt = BTIF_CMGR_SNIFF_ATTEMPT;
    sniffInfo.timeout = BTIF_CMGR_SNIFF_TIMEOUT;
    app_bt_CMGR_SetSniffInfoToAllHandlerByRemDev(conn_handle, &sniffInfo);

#if !defined(IBRT)
    uint8_t active_cons;
    active_cons = app_bt_get_active_cons();
    if (active_cons > 1)
    {
        btif_cmgr_handler_t    *currbtif_cmgr_handler_t = NULL;
        btif_cmgr_handler_t    *otherbtif_cmgr_handler_t = NULL;
        currbtif_cmgr_handler_t = btif_cmgr_get_conn_ind_handler(conn_handle);
        struct BT_DEVICE_T *curr_device = NULL;
        for (uint8_t i = 0; i < BT_DEVICE_NUM; i++)
        {
            curr_device = app_bt_get_device(i);
            if (curr_device->acl_is_connected && curr_device->acl_conn_hdl != conn_handle)
            {
                otherbtif_cmgr_handler_t = btif_cmgr_get_acl_handler(curr_device->acl_conn_hdl);
                if (otherbtif_cmgr_handler_t && currbtif_cmgr_handler_t)
                {
                    btif_sniff_info_t *other_cmgr_info = btif_cmgr_get_cmgrhandler_sniff_info(otherbtif_cmgr_handler_t);
                    btif_sniff_info_t *curr_cmgr_info = btif_cmgr_get_cmgrhandler_sniff_info(currbtif_cmgr_handler_t);
                    if ( other_cmgr_info->maxInterval == curr_cmgr_info->maxInterval)
                    {
                        sniffInfo.maxInterval = other_cmgr_info->maxInterval -20;
                        sniffInfo.minInterval = other_cmgr_info->minInterval - 20;
                        sniffInfo.attempt = BTIF_CMGR_SNIFF_ATTEMPT;
                        sniffInfo.timeout = BTIF_CMGR_SNIFF_TIMEOUT;
                        app_bt_CMGR_SetSniffInfoToAllHandlerByRemDev(curr_device->acl_conn_hdl, &sniffInfo);
                    }
                }
                break;
            }
            else
            {
                DEBUG_INFO(3,"%s:enumerate i:%d, acl connected %d conn_handle %04x, this may cause error!", __func__, i,
                    curr_device->acl_is_connected, curr_device->acl_conn_hdl);
            }
        }
    }
#endif
}

void app_bt_sniff_manager_process(const btif_event_t *Event)
{
    uint16_t conn_handle = BT_INVALID_CONN_HANDLE;

    btif_sniff_info_t sniffInfo;
    uint8_t active_cons;

    if (!besbt_cfg.sniff)
        return;

    active_cons = app_bt_get_active_cons();
    btif_event_type_t etype = btif_me_get_callback_event_type(Event);

    conn_handle = btif_me_get_callback_event_handle( Event);
    switch (etype)
    {
        case BTIF_BTEVENT_LINK_CONNECT_IND:
            break;
        case BTIF_BTEVENT_LINK_CONNECT_CNF:
            break;
        case BTIF_BTEVENT_LINK_DISCONNECT:
            sniffInfo.maxInterval = BTIF_CMGR_SNIFF_MAX_INTERVAL;
            sniffInfo.minInterval = BTIF_CMGR_SNIFF_MIN_INTERVAL;
            sniffInfo.attempt = BTIF_CMGR_SNIFF_ATTEMPT;
            sniffInfo.timeout = BTIF_CMGR_SNIFF_TIMEOUT;
            app_bt_CMGR_SetSniffInfoToAllHandlerByRemDev(conn_handle, &sniffInfo);
            break;
        case BTIF_BTEVENT_MODE_CHANGE:
            break;
        case BTIF_BTEVENT_ACL_DATA_ACTIVE:
            btif_cmgr_handler_t    *cmgrHandler;
            /* Start the sniff timer */
            cmgrHandler = btif_cmgr_get_acl_handler(conn_handle);
            if (cmgrHandler)
                app_bt_CMGR_SetSniffTimer(cmgrHandler, NULL, BTIF_CMGR_SNIFF_TIMER);
            break;
        case BTIF_BTEVENT_SCO_CONNECT_IND:
        case BTIF_BTEVENT_SCO_CONNECT_CNF:
            DEBUG_INFO(1,"BTEVENT_SCO_CONNECT_IND/CNF : handle %04x", conn_handle);
            app_bt_Me_SetLinkPolicy(conn_handle, BTIF_BLP_DISABLE_ALL);
            if (active_cons > 1)
            {
                struct BT_DEVICE_T *curr_device = NULL;
                for (uint8_t i=0; i<BT_DEVICE_NUM; i++)
                {
                    curr_device = app_bt_get_device(i);
                    if (!curr_device->acl_is_connected || curr_device->acl_conn_hdl == conn_handle)
                    {
                        continue;
                    }
                    if (btif_me_get_current_mode(curr_device->btm_conn) == BTIF_BLM_ACTIVE_MODE)
                    {
                        DEBUG_INFO(0,"other dev disable sniff");
                        app_bt_Me_SetLinkPolicy(curr_device->acl_conn_hdl, BTIF_BLP_DISABLE_ALL);
                    }
                    else if (btif_me_get_current_mode(curr_device->btm_conn) == BTIF_BLM_SNIFF_MODE)
                    {
                        DEBUG_INFO(0," ohter dev exit & disable sniff");
                        app_bt_ME_StopSniff(curr_device->acl_conn_hdl);
                        app_bt_Me_SetLinkPolicy(curr_device->acl_conn_hdl, BTIF_BLP_DISABLE_ALL);
                    }
                }
            }
            break;
        case BTIF_BTEVENT_SCO_DISCONNECT:
            if (app_bt_audio_count_streaming_a2dp())
            {
                break;
            }
            if (active_cons == 1)
            {
                app_bt_Me_SetLinkPolicy(conn_handle, BTIF_BLP_SNIFF_MODE);
            }
            else
            {
#if defined(BT_HFP_SUPPORT)
                if (app_bt_audio_count_connected_sco())
                {
                    break;
                }
#endif
                struct BT_DEVICE_T *curr_device = NULL;
                for (int i = 0; i < BT_DEVICE_NUM; i++)
                {
                    curr_device = app_bt_get_device(i);
                    if (curr_device->acl_is_connected)
                    {
                        app_bt_Me_SetLinkPolicy(curr_device->acl_conn_hdl, BTIF_BLP_SNIFF_MODE);
                    }
                }
            }
            break;
        default:
            break;
    }
}

APP_BT_GOLBAL_HANDLE_HOOK_HANDLER app_bt_global_handle_hook_handler[APP_BT_GOLBAL_HANDLE_HOOK_USER_QTY] = {0};
void app_bt_global_handle_hook(const btif_event_t *Event)
{
    uint8_t i;
    for (i=0; i<APP_BT_GOLBAL_HANDLE_HOOK_USER_QTY; i++)
    {
        if (app_bt_global_handle_hook_handler[i])
            app_bt_global_handle_hook_handler[i](Event);
    }
}

int app_bt_global_handle_hook_set(enum APP_BT_GOLBAL_HANDLE_HOOK_USER_T user, APP_BT_GOLBAL_HANDLE_HOOK_HANDLER handler)
{
    app_bt_global_handle_hook_handler[user] = handler;
    return 0;
}

APP_BT_GOLBAL_HANDLE_HOOK_HANDLER app_bt_global_handle_hook_get(enum APP_BT_GOLBAL_HANDLE_HOOK_USER_T user)
{
    return app_bt_global_handle_hook_handler[user];
}

void app_bt_pair_state_callback_register(bt_pair_state_change_cb_t cb)
{
    g_bt_pair_state_callback = cb;
    DEBUG_INFO(1, "%s %p", __func__, g_bt_pair_state_callback);
}

void app_bt_pair_state_callback_deregister(void)
{
    g_bt_pair_state_callback = NULL;
}

bt_pair_state_change_cb_t app_bt_get_pair_state_callback(void)
{
    return g_bt_pair_state_callback;
}

static btif_device_record_t name_record;
uint8_t *app_get_current_remote_device_name(void)
{
    return (uint8_t*)&name_record.remote_dev_name;
}

//Add weak func,because of watch_v1.0 branch build fial
WEAK void nv_record_execute_async_flush(void)
{

}

/////There is a device connected, so stop PAIR_TIMER and POWEROFF_TIMER of earphone.
btif_handler app_bt_handler;
void app_bt_global_handle(const btif_event_t *Event)
{
    POSSIBLY_UNUSED uint8_t active_cons;
    uint16_t conn_handle = BT_INVALID_CONN_HANDLE;
    btif_remote_device_t *btm_conn = NULL;

    active_cons = app_bt_get_active_cons();

    uint8_t error_code = btif_me_get_callback_event_err_code(Event);
    btif_event_type_t etype = btif_me_get_callback_event_type(Event);
    conn_handle = btif_me_get_callback_event_handle(Event);
    app_bt_coex_sink_global_event_ind(Event);

    switch (etype)
    {
        case BTIF_BTEVENT_HCI_INITIALIZED:
            break;
#if defined(IBRT)
        case BTIF_BTEVENT_HCI_COMMAND_SENT:
            return;
#else
        case BTIF_BTEVENT_HCI_COMMAND_SENT:
        case BTIF_BTEVENT_ACL_DATA_NOT_ACTIVE:
            return;
        case BTIF_BTEVENT_ACL_DATA_ACTIVE:
            btif_cmgr_handler_t    *cmgrHandler;
            /* Start the sniff timer */
            cmgrHandler = btif_cmgr_get_acl_handler(conn_handle);
            if (cmgrHandler)
                app_bt_CMGR_SetSniffTimer(cmgrHandler, NULL, BTIF_CMGR_SNIFF_TIMER);
            return;
#endif
        case BTIF_BTEVENT_AUTHENTICATED:
        {
            bool delete_record_link_key_only = true;

            DEBUG_INFO(1,"[BTEVENT] HANDER AUTH error=%x", error_code);

            //after authentication completes, re-enable sniff mode.
            if(error_code == BTIF_BEC_NO_ERROR)
            {
                btm_conn = btif_me_get_callback_event_rem_dev(Event);
                app_bt_device_report_authenticated(BTIF_BEC_NO_ERROR, btif_me_get_callback_event_rem_dev(Event));
                if (btif_me_is_conn_preferred_as_slave(btm_conn) && btif_me_current_bt_role_is_master(btm_conn))
                {
                    app_bt_Me_SetLinkPolicy(conn_handle, BTIF_BLP_MASTER_SLAVE_SWITCH|BTIF_BLP_SNIFF_MODE);
                }
                else
                {
                    app_bt_Me_SetLinkPolicy(conn_handle, BTIF_BLP_SNIFF_MODE);
                }
            }
            else if (error_code == BTIF_BEC_AUTHENTICATE_FAILURE || error_code == BTIF_BEC_MISSING_KEY)
            {
                //auth failed should clear nv record link key
                bt_bdaddr_t *bd_ddr = btif_me_get_callback_event_address(Event);
                btif_device_record_t record;
                DUMP8("%02x ", bd_ddr->address, 6);
                if (ddbif_find_record(bd_ddr, &record) == BT_STS_SUCCESS)
                {
                    DEBUG_INFO(0,"delete link key as authen fail");
                    char remote_dev_name[BTM_NAME_MAX_LEN];
                    strcpy(remote_dev_name, record.remote_dev_name);
                    bluetooth_nv_mgr_bt_record_del(BT_NV_REC_DEL_AUTHEN_FAILED, bd_ddr->address);

                    if (g_bt_pair_state_callback)
                    {
                        g_bt_pair_state_callback(&record.bdAddr, APP_BT_PAIR_NONE);
                    }

#if defined(IBRT)
                    bool delete_record_when_remote_key_missing = false;
                    btm_conn = app_bt_get_remote_dev_by_address(bd_ddr);
                    if (app_tws_ibrt_mobile_pair_canceled(bd_ddr))
                    {
                        delete_record_link_key_only = false;
                    }
                    else if (delete_record_when_remote_key_missing && btm_conn &&
                        error_code == BTIF_BEC_MISSING_KEY &&
                        btif_me_get_remote_device_initiator(btm_conn) &&
                        MOBILE_LINK == app_tws_ibrt_get_link_type_by_addr((bt_bdaddr_t *)bd_ddr))
                    {
                        delete_record_link_key_only = false;
                    }
#endif
                    if (delete_record_link_key_only)
                    {
                        DEBUG_INFO(0, "app_bt_global_handle: delete link key");
                        memset(&record, 0, sizeof(record));
                        record.bdAddr = *bd_ddr;
                        strcpy(record.remote_dev_name, remote_dev_name);
                        bluetooth_nv_mgr_bt_record_add(BT_NV_REC_ADD_AUTH_FAIL_KEEP, &record);
                    }
                    else
                    {
                        DEBUG_INFO(0, "app_bt_global_handle: delete record");
                    }
                    nv_record_execute_async_flush();
                }
            }
        }
        break;
    }
    // trace filter
    switch (etype)
    {
        case BTIF_BTEVENT_HCI_COMMAND_SENT:
        case BTIF_BTEVENT_ACL_DATA_NOT_ACTIVE:
        case BTIF_BTEVENT_ACL_DATA_ACTIVE:
            break;
        default:
            DEBUG_INFO(1,"[BTEVENT] evt = %d", etype);
            break;
    }
    switch (etype)
    {
        case BTIF_BTEVENT_LINK_CONNECT_IND:
        case BTIF_BTEVENT_LINK_CONNECT_CNF:
            btm_conn = btif_me_get_callback_event_rem_dev(Event);

            app_bt_device_report_acl_connected(error_code, btm_conn);

            if (BTIF_BEC_NO_ERROR == error_code)
            {
                DEBUG_INFO(1,"MEC(pendCons) is %d", btif_me_get_pendCons());

                app_bt_stay_active_rem_dev(conn_handle);

#if (defined(__AI_VOICE__) || defined(BISTO_ENABLED)) && !defined(IBRT)
                app_ai_if_mobile_connect_handle((void *)btif_me_get_callback_event_address(Event));
#endif
#if defined(WATCH_AI_ENABLED) && !defined(IBRT)
                app_watch_ai_mobile_connect_handle((void *)btif_me_get_callback_event_address(Event));
#endif
            }

            DEBUG_INFO(4,"[BTEVENT] CONNECT_IND/CNF evt:%d errCode:0x%0x newRole:%d activeCons:%d",etype,
                  error_code,btif_me_get_callback_event_rem_dev_role (Event), active_cons);
            DUMP8("%02x ", btif_me_get_callback_event_address(Event), BT_ADDR_OUTPUT_PRINT_NUM);

#if defined(__BTIF_EARPHONE__) && defined(__BTIF_AUTOPOWEROFF__)  && !defined(FPGA)
            if (active_cons == 0)
            {
                app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
            }
            else
            {
                app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
            }
#endif

#if !defined(IBRT)
            if (active_cons > BT_DEVICE_NUM)
            {
                DEBUG_INFO(1,"CONNECT_IND/CNF activeCons:%d so disconnect it", active_cons);
                app_bt_MeDisconnectLink(conn_handle);
            }
            else
            {
    #ifdef BT_DIP_SUPPORT
                app_dip_get_remote_info(btif_me_get_device_id_from_rdev(btm_conn));
    #endif
            }
#endif
            break;
        case BTIF_BTEVENT_LINK_DISCONNECT:
        {
#ifdef CUSTOM_BITRATE
            app_ibrt_user_a2dp_codec_info_action();
#endif
            btm_conn = btif_me_get_callback_event_rem_dev(Event);
            bt_bdaddr_t *bd_ddr = btif_me_get_remote_device_bdaddr(btm_conn);
            app_bt_device_report_acl_disconnected(error_code, btm_conn);

            DEBUG_INFO(5,"[BTEVENT] DISCONNECT evt = %d encryptState:%d reason:0x%02x/0x%02x activeCons:%d",
                  etype,
                  btif_me_get_remote_sevice_encrypt_state(btm_conn),
                  btif_me_get_remote_device_disc_reason_saved(btm_conn),
                  btif_me_get_remote_device_disc_reason(btm_conn),
                  active_cons);
            DUMP8("%02x ", bd_ddr, BT_ADDR_OUTPUT_PRINT_NUM);
            hci_rx_flowctrl_info();
            hci_tx_flowctrl_info();

            //disconnect from reconnect connection, and the HF don't connect successful once
            //(whitch will release the saved_reconnect_mode ). so we are reconnect fail with remote link key loss.
            // goto pairing.
            //reason 07 maybe from the controller's error .
            //05  auth error
            //16  io cap reject.

#if defined(__BTIF_EARPHONE__) && defined(__BTIF_AUTOPOWEROFF__) && !defined(FPGA)
            if (active_cons == 0)
            {
                app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
            }
#endif

#if defined(BISTO_ENABLED) && !defined(IBRT)
            gsound_custom_bt_link_disconnected_handler(&bd_ddr->address[0]);
#endif

#if defined(__AI_VOICE__)
            app_ai_mobile_disconnect_handle(&bd_ddr->address[0]);
#endif

#ifdef  __IAG_BLE_INCLUDE__
            // start BLE adv
            // bes_ble_gap_force_switch_adv(BLE_SWITCH_USER_BT_CONNECT, true);
#endif

#ifdef BT_DIP_SUPPORT
            btif_dip_clear(bd_ddr);
#endif

#ifdef CTKD_ENABLE
#ifndef IBRT
            set_rsp_dist_lk_bit_field_func dist_lk_set_cb = bes_ble_gap_sec_reg_dist_lk_bit_get_callback();
            if ((dist_lk_set_cb && dist_lk_set_cb()) || (!dist_lk_set_cb))
            {
                if (app_bt_ctkd_is_connecting_mobile_pending())
                {
                    app_bt_ctkd_connecting_mobile_handler();
                }
            }
#endif
#endif

#if defined(__CONNECTIVITY_LOG_REPORT__)
            app_ibrt_if_disconnect_event(btif_me_get_callback_event_address(Event),
                                         btif_me_get_remote_device_disc_reason(btm_conn),
                                         active_cons);
#endif
            break;
        }
        case BTIF_BTEVENT_ROLE_DISCOVERED:
            DEBUG_INFO(3,"[BTEVENT] ROLE_DISCOVERED eType:0x%x errCode:0x%x newRole:%d activeCons:%d", etype,
                  error_code, btif_me_get_callback_event_role_change_new_role(Event), active_cons);
            break;
        case BTIF_BTEVENT_ROLE_CHANGE:
            DEBUG_INFO(3,"[BTEVENT] ROLE_CHANGE eType:0x%x errCode:0x%x newRole:%d activeCons:%d", etype,
                  error_code, btif_me_get_callback_event_role_change_new_role(Event), active_cons);
            break;
        case BTIF_BTEVENT_MODE_CHANGE:
            DEBUG_INFO(4,"[BTEVENT] MODE_CHANGE evt:%d errCode:0x%0x curMode=0x%0x, interval=%d ",etype,
                  error_code, btif_me_get_callback_event_mode_change_curMode(Event),
                  btif_me_get_callback_event_mode_change_interval(Event));
            btm_conn = btif_me_get_callback_event_rem_dev(Event);
            DUMP8("%02x ", btif_me_get_remote_device_bdaddr(btm_conn), BT_ADDR_OUTPUT_PRINT_NUM);
#if defined(__CONNECTIVITY_LOG_REPORT__)
            app_ibrt_if_save_curr_mode_to_disconnect_info(btif_me_get_callback_event_mode_change_curMode(Event),
                                                          btif_me_get_callback_event_mode_change_interval(Event),
                                                          btif_me_get_remote_device_bdaddr(btm_conn));
#endif
            break;
        case BTIF_BTEVENT_ACCESSIBLE_CHANGE:
            {
                error_code = btif_me_get_callback_event_err_code(Event);
                TRACE(3,"[BTEVENT] ACCESSIBLE_CHANGE evt:%d errCode:0x%0x aMode=0x%0x",
                    btif_me_get_callback_event_type(Event),
                    error_code,
                    btif_me_get_callback_event_a_mode(Event));
                app_bt_access_mode_change_event_handle(Event, error_code);
            }
            break;
        case BTIF_BTEVENT_LINK_POLICY_CHANGED:
        {
            BT_SET_LINKPOLICY_REQ_T* pReq = app_bt_pop_pending_set_linkpolicy();
            if (NULL != pReq)
            {
                app_bt_Me_SetLinkPolicy(pReq->conn_handle, pReq->policy);
            }
            break;
        }
        case BTIF_BTEVENT_DEFAULT_LINK_POLICY_CHANGED:
        {
            DEBUG_INFO(0,"[BTEVENT] DEFAULT_LINK_POLICY_CHANGED-->BT_STACK_INITIALIZED");
            bt_host_ready(STACK_READY_BT);
            break;
        }
        case BTIF_BTEVENT_NAME_RESULT:
        {
            const uint8_t* ptrName;
            uint8_t nameLen;
            nameLen = btif_me_get_callback_event_remote_dev_name(Event, &ptrName);
            DEBUG_INFO(1,"[BTEVENT] NAME_RESULT name len %d", nameLen);
            if (nameLen > 0)
            {
                bt_bdaddr_t *bd_ddr = btif_me_get_callback_event_address(Event);
                POSSIBLY_UNUSED uint8_t *addr = bd_ddr->address;
                DEBUG_INFO(1,"remote name: %s, %02x:%02x:%02x:%02x:%02x:%02x", ptrName,
                        addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
                btif_device_record_t record;
                if (BT_STS_SUCCESS != ddbif_find_record(bd_ddr, &record)) {
                    break;
                }
                if (nameLen >= BTM_SHORT_NAME_MAX_LEN) {
                    nameLen = BTM_SHORT_NAME_MAX_LEN - 1;
                }
                if (memcmp(record.remote_dev_name, ptrName, nameLen)){
                    memcpy(record.remote_dev_name, ptrName, nameLen);
                    record.remote_dev_name[nameLen] = '\0';
                    bluetooth_nv_mgr_bt_record_add(BT_NV_REC_ADD_NAME_RESULT_SAVE, &record);
                }
            }
            break;
        }
        case BTIF_BTEVENT_USER_PASSKEY_NOTIFY_EVENT:
        {
            btm_conn = btif_me_get_callback_event_rem_dev(Event);
            app_bt_user_passkey_notify_callback(btm_conn, btif_me_get_callback_event_passkey_notify_passkey(Event));
            break;
        }
        case BTIF_BTEVENT_USER_PASSKEY_REQUEST_EVENT:
        {
            btm_conn = btif_me_get_callback_event_rem_dev(Event);
            app_bt_user_passkey_request_callback(btm_conn);
            break;
        }
        case BTIF_BTEVENT_SET_BT_BLE_ACTIVE_LINK_COMPLETE:
        {
            bt_audio_event_handler(0, APP_BT_AUDIO_EVENT_SET_BT_BLE_ACTIVE_LINK_COMPLETE, (uint32_t)error_code);
            break;
        }
        default:
            break;
    }

#ifdef MULTIPOINT_DUAL_SLAVE
    app_bt_role_manager_process_dual_slave(Event);
#else
    app_bt_role_manager_process(Event);
#endif
    app_bt_accessible_manager_process(Event);
#if !defined(IBRT)
    app_bt_sniff_manager_process(Event);
#endif
    app_bt_global_handle_hook(Event);
#if defined(IBRT)
    app_tws_ibrt_global_callback(Event);
#endif
}

#ifdef BT_HFP_SUPPORT
osTimerId bt_sco_recov_timer = NULL;
static void bt_sco_recov_timer_handler(void const *param);
osTimerDef (BT_SCO_RECOV_TIMER, (void (*)(void const *))bt_sco_recov_timer_handler);                      // define timers
void hfp_reconnect_sco(uint8_t flag);
static void bt_sco_recov_timer_handler(void const *param)
{
    DEBUG_INFO(1,"%s",__func__);
    hfp_reconnect_sco(0);
}
static void bt_sco_recov_timer_start()
{
    osTimerStop(bt_sco_recov_timer);
    osTimerStart(bt_sco_recov_timer, 2500);
}


enum
{
    SCO_DISCONNECT_RECONN_START,
    SCO_DISCONNECT_RECONN_RUN,
    SCO_DISCONNECT_RECONN_NONE,
};

static uint8_t sco_reconnect_status =  SCO_DISCONNECT_RECONN_NONE;

void hfp_reconnect_sco(uint8_t set)
{
    uint8_t curr_hfp_device = app_bt_audio_get_curr_hfp_device();
    DEBUG_INFO(3,"%s curr_hfp_device %x reconnect_status %x", __func__, curr_hfp_device, sco_reconnect_status);
    if(set == 1)
    {
        sco_reconnect_status = SCO_DISCONNECT_RECONN_START;
    }
    if(sco_reconnect_status == SCO_DISCONNECT_RECONN_START)
    {
        Audio_device_t audio_device;
        audio_device.audio_device.device_id = curr_hfp_device;
        audio_device.audio_device.device_type = AUDIO_TYPE_BT;
        audio_device.aud_id = MAX_RECORD_NUM;
        audio_player_playback_stop(AUDIO_STREAM_VOICE, &audio_device);
        app_bt_HF_DisconnectAudioLink(app_bt_get_device(curr_hfp_device)->hf_channel);
        sco_reconnect_status = SCO_DISCONNECT_RECONN_RUN;
        bt_sco_recov_timer_start();
    }
    else if(sco_reconnect_status == SCO_DISCONNECT_RECONN_RUN)
    {
        app_bt_HF_CreateAudioLink(app_bt_get_device(curr_hfp_device)->hf_channel);
        sco_reconnect_status = SCO_DISCONNECT_RECONN_NONE;
    }
}
#endif

void app_bt_global_handle_init(void)
{
    btif_event_mask_t mask = BTIF_BEM_NO_EVENTS;
    btif_me_init_handler(&app_bt_handler);
    app_bt_handler.callback = app_bt_global_handle;
    btif_me_register_global_handler(&app_bt_handler);
#if defined(IBRT)
    btif_me_register_accept_handler(&app_bt_handler);
#endif
#ifdef IBRT_SEARCH_UI
    app_bt_global_handle_hook_set(APP_BT_GOLBAL_HANDLE_HOOK_USER_0,app_bt_manager_ibrt_role_process);
#endif

    mask |= BTIF_BEM_ROLE_CHANGE | BTIF_BEM_SCO_CONNECT_CNF | BTIF_BEM_SCO_DISCONNECT | BTIF_BEM_SCO_CONNECT_IND;
    mask |= BTIF_BEM_ROLE_DISCOVERED;
    mask |= BTIF_BEM_AUTHENTICATED;
    mask |= BTIF_BEM_LINK_CONNECT_IND;
    mask |= BTIF_BEM_LINK_DISCONNECT;
    mask |= BTIF_BEM_LINK_CONNECT_CNF;
    mask |= BTIF_BEM_ACCESSIBLE_CHANGE;
    mask |= BTIF_BEM_ENCRYPTION_CHANGE;
    mask |= BTIF_BEM_SIMPLE_PAIRING_COMPLETE;
    mask |= BTIF_BEM_MODE_CHANGE;
    mask |= BTIF_BEM_LINK_POLICY_CHANGED;

    btif_me_set_event_mask(&app_bt_handler, mask);
#ifdef BT_HFP_SUPPORT
    bt_sco_recov_timer = osTimerCreate (osTimer(BT_SCO_RECOV_TIMER), osTimerOnce, NULL);
#endif
}

btif_remote_device_t* app_bt_get_remote_dev_by_address(const bt_bdaddr_t *bdaddr)
{
    btif_remote_device_t *conn = NULL;
    struct BT_DEVICE_T* curr_device = NULL;

    if (!bdaddr)
    {
        return NULL;
    }

    if (!(in_isr() || in_int_locked()))
    {
        bt_adapter_mutex_lock();
    }

    for (int i = 0; i < BT_DEVICE_NUM; i += 1)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->acl_is_connected && memcmp(&curr_device->remote, bdaddr, sizeof(bt_bdaddr_t)) == 0)
        {
            conn = curr_device->btm_conn;
            break;
        }
    }

#if defined(BT_SOURCE)
    if (conn == NULL)
    {
        for (int i = BT_SOURCE_DEVICE_ID_BASE; i < BT_SOURCE_DEVICE_ID_N; i += 1)
        {
            curr_device = app_bt_get_device(i);
            if (curr_device->acl_is_connected && memcmp(&curr_device->remote, bdaddr, sizeof(bt_bdaddr_t)) == 0)
            {
                conn = curr_device->btm_conn;
                break;
            }
        }
    }
#endif

#ifdef IBRT
    if (conn == NULL)
    {
        if (app_bt_manager.tws_conn.acl_is_connected &&
            memcmp(&app_bt_manager.tws_conn.remote, bdaddr, sizeof(bt_bdaddr_t)) == 0)
        {
            conn = app_bt_manager.tws_conn.btm_conn;
        }
    }
#endif

    if (!(in_isr() || in_int_locked()))
    {
        bt_adapter_mutex_unlock();
    }

    return conn;
}

btif_remote_device_t* app_bt_get_remote_dev_by_handle(uint16_t hci_handle)
{
    btif_remote_device_t *conn = NULL;
    struct BT_DEVICE_T *curr_device = NULL;

    if (!(in_isr() || in_int_locked()))
    {
        bt_adapter_mutex_lock();
    }

    for (int i = 0; i < BT_DEVICE_NUM; i += 1)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->acl_is_connected && curr_device->acl_conn_hdl == hci_handle)
        {
            conn = curr_device->btm_conn;
            break;
        }
    }

#if defined(BT_SOURCE)
    if (conn == NULL)
    {
        for (int i = BT_SOURCE_DEVICE_ID_BASE; i < BT_SOURCE_DEVICE_ID_N; i += 1)
        {
            curr_device = app_bt_get_device(i);
            if (curr_device->acl_is_connected && curr_device->acl_conn_hdl == hci_handle)
            {
                conn = curr_device->btm_conn;
                break;
            }
        }
    }
#endif

#ifdef IBRT
    if (conn == NULL)
    {
        if (app_bt_manager.tws_conn.acl_is_connected && app_bt_manager.tws_conn.acl_conn_hdl == hci_handle)
        {
            conn = app_bt_manager.tws_conn.btm_conn;
        }
    }
#endif

    if (!(in_isr() || in_int_locked()))
    {
        bt_adapter_mutex_unlock();
    }

    return conn;
}

uint8_t app_bt_get_remote_sync_id_by_bdaddr(bt_bdaddr_t *bdaddr)
{
    btif_remote_device_t *p_remote_dev = app_bt_get_remote_dev_by_address(bdaddr);
    if (p_remote_dev == NULL)
    {
        DEBUG_INFO(1,"%s: remote dev error",__func__);
        return BT_DEVICE_INVALID_ID;
    }

    uint16_t conhdl = btif_me_get_remote_device_hci_handle(p_remote_dev);
    return bt_drv_reg_op_get_sync_id_by_conhdl(conhdl);
}

uint8_t app_bt_count_mobile_link(void)
{
    int i = 0;
    uint8_t active_cons = 0;
    struct BT_DEVICE_T *curr_device = NULL;

    if (!(in_isr() || in_int_locked()))
    {
        bt_adapter_mutex_lock();
    }

    for (; i < BT_DEVICE_NUM; i += 1)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->acl_is_connected)
        {
            active_cons += 1;
        }
    }

    if (!(in_isr() || in_int_locked()))
    {
        bt_adapter_mutex_unlock();
    }

    return active_cons;
}

uint8_t app_bt_get_active_cons(void)
{
    uint8_t active_cons = app_bt_count_mobile_link();

#ifdef IBRT
    if (app_tws_ibrt_tws_link_connected())
    {
        active_cons += 1;
    }
#endif

    return active_cons;
}

bt_status_t app_bt_set_fix_tws_interval_param(uint16_t duration, uint16_t interval, uint16_t interval_in_sco)
{
    btdrv_reg_op_set_tws_link_duration(duration);
    btdrv_reg_op_set_private_tws_poll_interval(interval, interval_in_sco);
    return BT_STS_PENDING;
}

/*****************************************************************************
 Prototype    : app_bt_get_remote_device_name
 Description  : API for getting the remote device name if ACL has been connected
 Input        : void
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2021/6/6
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

void app_bt_get_remote_device_name(const bt_bdaddr_t * bdaddr)
{
    if (app_bt_is_acl_connected_byaddr(bdaddr))
    {
        app_bt_start_custom_function_in_bt_thread((uint32_t)(uintptr_t)bdaddr, 0, (uint32_t)(uintptr_t)btif_me_get_remote_device_name);
    }
}

/*****************************************************************************
 Prototype    : app_bt_inquiry_remote_device_name
 Description  : API for inquiry remote device name even if the BD address is not disconnected

 Input        : void
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2021/6/6
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_bt_inquiry_remote_device_name(const bt_bdaddr_t * bdaddr)
{
    app_bt_start_custom_function_in_bt_thread((uint32_t)(uintptr_t)bdaddr, 0, (uint32_t)(uintptr_t)btif_me_get_remote_device_name);
}

extern void app_start_10_second_timer(uint8_t timer_id);
static int app_bt_handle_process(APP_MESSAGE_BODY *msg_body)
{
    btif_accessible_mode_t old_access_mode;

    switch (msg_body->message_id)
    {
        case APP_BT_REQ_ACCESS_MODE_SET:
            old_access_mode = app_bt_get_curr_access_mode();
#if !defined(IBRT)
            app_bt_set_access_mode(msg_body->message_Param0);
#endif
            if (msg_body->message_Param0 == BTIF_BAM_GENERAL_ACCESSIBLE &&
                old_access_mode != BTIF_BAM_GENERAL_ACCESSIBLE)
            {
#ifndef FPGA
                app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
#ifdef MEDIA_PLAYER_SUPPORT
                audio_player_play_prompt(AUD_ID_BT_PAIR_ENABLE, 0);
#endif
                app_start_10_second_timer(APP_PAIR_TIMER_ID);
#endif
            }
            else
            {
#ifndef FPGA
                app_status_indication_set(APP_STATUS_INDICATION_PAGESCAN);
#endif
            }
            break;
#if defined(IBRT) && defined(IBRT_UI)
        case APP_BT_DO_FUNCTION:
            app_ibrt_conn_startup_mobile_sm(msg_body->message_Param0);
            break;
        default:
            app_ibrt_conn_dispatch_event(msg_body->message_id,msg_body->message_Param0,
                                            msg_body->message_Param1,msg_body->message_Param2);
#endif
            break;
    }

    return 0;
}

void app_bt_device_reconnect_handler(struct BT_DEVICE_RECONNECT_T *reconnect)
{
#ifdef BT_SOURCE
    if (reconnect->for_source_device)
    {
        bt_source_perform_profile_reconnect(&reconnect->rmt_addr);
    }
    else
#endif
    {
        if (app_bt_is_acl_connected_byaddr(&reconnect->rmt_addr))
        {
            DEBUG_INFO(1,"%s acl already connected", __func__);
            app_bt_delete_reconnect_device(reconnect);
            return;
        }

        if (btif_me_get_pendCons() > 0)
        {
            DEBUG_INFO(1,"%s exist pending acl cons, reset the timer", __func__);
            osTimerStart(reconnect->acl_reconnect_timer, BTIF_BT_DEFAULT_PAGE_TIMEOUT_IN_MS);
            return;
        }
#if defined(BT_HFP_SUPPORT)
        if (app_bt_audio_count_connected_sco())
        {
            DEBUG_INFO(1,"%s exist sco streaming, reset the timer", __func__);
            osTimerStart(reconnect->acl_reconnect_timer, APP_BT_PROFILE_RECONNECT_WAIT_SCO_DISC_MS);
            return;
        }
#endif // BT_HFP_SUPPORT

#ifdef BT_A2DP_SUPPORT
        if(app_bt_audio_get_curr_playing_a2dp() != BT_DEVICE_INVALID_ID &&
           !app_bt_is_acl_connected_byaddr(&reconnect->rmt_addr))
        {
            DEBUG_INFO(1,"%s alloc more timing to a2dp link when doing page", __func__);
            app_bt_audio_enable_active_link(true, app_bt_audio_get_curr_playing_a2dp());
        }
#endif

        if (!app_bt_is_a2dp_connected_byaddr(&reconnect->rmt_addr) && besbt_cfg.a2dp_sink_enable)
        {
            DEBUG_INFO(0,"try connect a2dp");
            app_bt_precheck_before_starting_connecting(false);
            app_bt_reconnect_a2dp_profile(&reconnect->rmt_addr);
        }
        else if (!app_bt_is_hfp_connected_byaddr(&reconnect->rmt_addr))
        {
            DEBUG_INFO(0,"try connect hf");
            app_bt_precheck_before_starting_connecting(false);
            app_bt_reconnect_hfp_profile(&reconnect->rmt_addr);
        }
    }
}

void app_bt_device_reconnect_timehandler(void const *param)
{
    app_bt_start_custom_function_in_bt_thread((uint32_t)param, 0,
            (uint32_t)app_bt_device_reconnect_handler);
}

static void app_bt_profile_reconnect_handler(void const *param)
{
#if !defined(IBRT)
    struct app_bt_profile_manager *profile_mgr = (struct app_bt_profile_manager *)param;
    btdevice_profile *btdevice_plf_p = NULL;

    DEBUG_INFO(3,"%s reconnect mode %d cnt %d", __func__, profile_mgr->reconnect_mode, profile_mgr->reconnect_cnt);

    if (app_bt_is_acl_connected_byaddr(&profile_mgr->rmt_addr))
    {
        if (profile_mgr->connect_timer_cb)
        {
            profile_mgr->connect_timer_cb(param);
            profile_mgr->connect_timer_cb = NULL;
        }
        else
        {
            btdevice_plf_p = (btdevice_profile *)app_bt_profile_active_store_ptr_get(profile_mgr->rmt_addr.address);
            if((btdevice_plf_p->a2dp_act)
               &&(profile_mgr->a2dp_connect != bt_profile_connect_status_success))
            {
                DEBUG_INFO(0,"try connect a2dp");
                app_bt_precheck_before_starting_connecting(profile_mgr->profile_connected);
                app_bt_reconnect_a2dp_profile(&profile_mgr->rmt_addr);
            }
            else if ((btdevice_plf_p->hfp_act)
                     &&(profile_mgr->hfp_connect != bt_profile_connect_status_success))
            {
                DEBUG_INFO(0,"try connect hf");
                app_bt_precheck_before_starting_connecting(profile_mgr->profile_connected);
                app_bt_reconnect_hfp_profile((bt_bdaddr_t *)&profile_mgr->rmt_addr);
            }
        }
    }
    else
    {
        struct BT_DEVICE_RECONNECT_T *reconnect = NULL;
        reconnect = app_bt_append_to_reconnect_list(profile_mgr->reconnect_mode, &profile_mgr->rmt_addr, false);
        if (!reconnect)
        {
            DEBUG_INFO(1,"%s cannot add device", __func__);
            return;
        }
        app_bt_device_reconnect_handler(reconnect);
    }
#else
    DEBUG_INFO(0,"app_bt_profile_reconnect_timehandler called");
#if defined(BT_AVRCP_SUPPORT) && !defined(BT_A2DP_SUPPORT)
    struct app_bt_profile_manager *profile_mgr = (struct app_bt_profile_manager *)param;
    if (profile_mgr->avrcp_connect != bt_profile_connect_status_success)
    {
        app_bt_reconnect_avrcp_profile((bt_bdaddr_t *)&profile_mgr->rmt_addr);
    }
#endif
#endif
}

static void app_bt_profile_reconnect_timehandler(void const *param)
{
    app_bt_start_custom_function_in_bt_thread((uint32_t)param, 0,
            (uint32_t)app_bt_profile_reconnect_handler);
}

static void app_bt_start_poweron_reconnect(void)
{
    struct BT_DEVICE_RECONNECT_T *reconnect = NULL;
    reconnect = app_bt_get_poweron_reconnect_device();
    if (!reconnect)
    {
        return;
    }

    app_bt_start_custom_function_in_bt_thread((uint32_t)reconnect, 0,
            (uint32_t)app_bt_device_reconnect_handler);
}

static void app_bt_start_reconnect_next_device(void)
{
    struct BT_DEVICE_RECONNECT_T *reconnect = NULL;
    reconnect = app_bt_get_poweron_reconnect_device();
    if (!reconnect)
    {
        return;
    }

    DEBUG_INFO(0,"!!!start reconnect next device\n");
    app_bt_start_poweron_reconnect();
}

void app_bt_start_linkloss_reconnect(bt_bdaddr_t *remote, bool is_for_source_device)
{
    struct BT_DEVICE_RECONNECT_T *reconnect = NULL;
    reconnect = app_bt_append_to_reconnect_list(bt_profile_reconnect_reconnecting, remote, is_for_source_device);
    if (!reconnect)
    {
        DEBUG_INFO(1,"%s cannot add device", __func__);
        return;
    }

    DEBUG_INFO(8,"%s %02x:%02x:%02x:%02x:%02x:%02x wait %dms", __func__,
          remote->address[5], remote->address[4], remote->address[3],
          remote->address[2], remote->address[1], remote->address[0],
          APP_BT_PROFILE_RECONNECT_RETRY_INTERVAL_MS);

    if (!is_for_source_device)
    {
#if !defined(IBRT)
        app_bt_set_access_mode(BTIF_BAM_CONNECTABLE_ONLY);
#endif

#ifdef __IAG_BLE_INCLUDE__
        bes_ble_gap_start_connectable_adv(BLE_ADVERTISING_INTERVAL);
#endif
    }

    osTimerStart(reconnect->acl_reconnect_timer, APP_BT_PROFILE_RECONNECT_RETRY_INTERVAL_MS);
}

void app_bt_start_connfail_reconnect(bt_bdaddr_t *remote, uint8_t errcode, bool is_for_source_device)
{
    struct BT_DEVICE_RECONNECT_T *reconnect = NULL;
    reconnect = app_bt_find_reconnect_device(remote);
    if (!reconnect)
    {
        DEBUG_INFO(1, "%s not in the list", __func__);
        return;
    }

    DEBUG_INFO(9,"%s %02x:%02x:%02x:%02x:%02x:%02x mode %d errcode %x", __func__,
          remote->address[5], remote->address[4], remote->address[3],
          remote->address[2], remote->address[1], remote->address[0],
          reconnect->reconnect_mode, errcode);

    if (reconnect->reconnect_mode == bt_profile_reconnect_openreconnecting)
    {
        if (++reconnect->acl_reconnect_cnt < APP_BT_PROFILE_OPENNING_RECONNECT_RETRY_LIMIT_CNT)
        {
            if (!is_for_source_device)
            {
#if !defined(IBRT)
                app_bt_set_access_mode(BTIF_BAM_CONNECTABLE_ONLY);
#endif
#ifdef  __IAG_BLE_INCLUDE__
                bes_ble_gap_start_connectable_adv(BLE_ADVERTISING_INTERVAL);
#endif
            }
            osTimerStart(reconnect->acl_reconnect_timer, APP_BT_PROFILE_RECONNECT_RETRY_INTERVAL_MS);
        }
        else
        {
            DEBUG_INFO(2, "%s reach max open reconnect_cnt %d", __func__, reconnect->acl_reconnect_cnt);
            app_bt_delete_reconnect_device(reconnect);
            app_bt_start_reconnect_next_device();
        }
    }
    else if (reconnect->reconnect_mode == bt_profile_reconnect_reconnecting)
    {
        if (++reconnect->acl_reconnect_cnt < APP_BT_PROFILE_RECONNECT_RETRY_LIMIT_CNT)
        {
            if (!is_for_source_device)
            {
#if !defined(IBRT)
                app_bt_set_access_mode(BTIF_BAM_CONNECTABLE_ONLY);
#endif
#ifdef  __IAG_BLE_INCLUDE__
                bes_ble_gap_start_connectable_adv(BLE_ADVERTISING_INTERVAL);
#endif
            }
            osTimerStart(reconnect->acl_reconnect_timer, APP_BT_PROFILE_RECONNECT_RETRY_INTERVAL_MS);
        }
        else
        {
            DEBUG_INFO(2, "%s reach max reconnect_cnt %d", __func__, reconnect->acl_reconnect_cnt);
            app_bt_delete_reconnect_device(reconnect);
        }
    }
    else
    {
        app_bt_delete_reconnect_device(reconnect);
    }

#if defined(IBRT) && defined(FREEMAN_ENABLED_STERO)
    if (app_bt_get_poweron_reconnect_device() == NULL)
    {
        app_ibrt_internal_enter_freeman_pairing();
#ifdef GFPS_ENABLED
        app_enter_fastpairing_mode();
#endif
    }
#endif
}

bool app_bt_is_in_connecting_profiles_state(void)
{
    for (uint8_t devId = 0; devId < BT_DEVICE_NUM; devId++)
    {
        if (APP_BT_IN_CONNECTING_PROFILES_STATE == app_bt_get_device(devId)->profile_mgr.connectingState)
        {
            return true;
        }
    }

    return false;
}

void app_bt_clear_connecting_profiles_state(uint8_t devId)
{
    DEBUG_INFO(1,"Dev %d exists connecting profiles state", devId);

    app_bt_get_device(devId)->profile_mgr.connectingState = APP_BT_IDLE_STATE;
    if (!app_bt_is_in_connecting_profiles_state())
    {
#ifdef __IAG_BLE_INCLUDE__
#ifndef BLE_ADV_NOT_START_DEFAULT
        app_start_fast_connectable_ble_adv(BLE_FAST_ADVERTISING_INTERVAL);
#endif
#endif
    }
}

void app_bt_set_connecting_profiles_state(uint8_t devId)
{
    DEBUG_INFO(1,"Dev %d enters connecting profiles state", devId);

    app_bt_get_device(devId)->profile_mgr.connectingState = APP_BT_IN_CONNECTING_PROFILES_STATE;
#ifdef  __IAG_BLE_INCLUDE__
    // stop BLE adv
#ifndef IBRT
    // bes_ble_gap_force_switch_adv(BLE_SWITCH_USER_BT_CONNECT, false);
    // bes_ble_gap_start_connectable_adv(BLE_ADVERTISING_INTERVAL);
#endif
#endif
}

bool app_bt_is_in_reconnecting(void)
{
    uint8_t devId;
    for (devId = 0; devId < BT_DEVICE_NUM; devId++)
    {
        if (bt_profile_reconnect_null != app_bt_get_device(devId)->profile_mgr.reconnect_mode)
        {
            return true;
        }
    }

    return false;
}

void app_bt_profile_connect_manager_opening_reconnect(void)
{
    int ret;
    btif_device_record_t record1;
    btif_device_record_t record2;
    btdevice_profile *btdevice_plf_p;
    int find_invalid_record_cnt;

    if(!app_bt_sink_is_enabled())
    {
        return ;
    }

    if (BT_DEVICE_NUM == 1 && app_bt_get_active_cons() != 0)
    {
        DEBUG_INFO(0,"bt link disconnect not complete,ignore this time reconnect");
        return;
    }

    do
    {
        find_invalid_record_cnt = 0;
        ret = nv_record_enum_latest_two_paired_dev(&record1,&record2);
        if(ret == 1)
        {
            btdevice_plf_p = (btdevice_profile *)app_bt_profile_active_store_ptr_get(record1.bdAddr.address);
            if (!(btdevice_plf_p->hfp_act)&&!(btdevice_plf_p->a2dp_act))
            {
                nv_record_ddbrec_delete((bt_bdaddr_t *)&record1.bdAddr);
                find_invalid_record_cnt++;
            }
        }
        else if(ret == 2)
        {
            btdevice_plf_p = (btdevice_profile *)app_bt_profile_active_store_ptr_get(record1.bdAddr.address);
            if (!(btdevice_plf_p->hfp_act)&&!(btdevice_plf_p->a2dp_act))
            {
                nv_record_ddbrec_delete((bt_bdaddr_t *)&record1.bdAddr);
                find_invalid_record_cnt++;
            }
            btdevice_plf_p = (btdevice_profile *)app_bt_profile_active_store_ptr_get(record2.bdAddr.address);
            if (!(btdevice_plf_p->hfp_act)&&!(btdevice_plf_p->a2dp_act))
            {
                nv_record_ddbrec_delete((bt_bdaddr_t *)&record2.bdAddr);
                find_invalid_record_cnt++;
            }
        }
    }
    while(find_invalid_record_cnt);

    DEBUG_INFO(1,"!!!app_bt_opening_reconnect: devices %d\n", ret);
    DUMP8("%02x ", &record1.bdAddr, BT_ADDR_OUTPUT_PRINT_NUM);
    DUMP8("%02x ", &record2.bdAddr, BT_ADDR_OUTPUT_PRINT_NUM);

    if(ret > 0)
    {
        DEBUG_INFO(0,"!!!start reconnect first device\n");

#if defined(FREEMAN_ENABLED_STERO)
#ifdef IBRT_UI
        app_ibrt_if_event_entry(APP_UI_EV_CASE_OPEN);
#endif
#else
        if (btif_me_get_pendCons() == 0)
        {
#ifdef BT_SOURCE
            app_bt_append_to_reconnect_list(bt_profile_reconnect_openreconnecting, &record1.bdAddr, record1.for_bt_source);
#else
            app_bt_append_to_reconnect_list(bt_profile_reconnect_openreconnecting, &record1.bdAddr, false);
#endif
        }

#ifdef BT_SOURCE
        if(ret > 1 && (BT_DEVICE_NUM + BT_SOURCE_DEVICE_NUM) > 1)
#else
        if(ret > 1 && BT_DEVICE_NUM > 1)
#endif
        {
#ifdef BT_SOURCE
            app_bt_append_to_reconnect_list(bt_profile_reconnect_openreconnecting, &record2.bdAddr, record2.for_bt_source);
#else
            app_bt_append_to_reconnect_list(bt_profile_reconnect_openreconnecting, &record2.bdAddr, false);
#endif
        }

        app_bt_start_poweron_reconnect();
#endif
    }
    else
    {
        DEBUG_INFO(0,"!!!go to pairing\n");
#ifdef FREEMAN_ENABLED_STERO
        app_ibrt_internal_enter_freeman_pairing();
#ifdef GFPS_ENABLED
        app_enter_fastpairing_mode();
#endif
#else
#ifdef __EARPHONE_STAY_BOTH_SCAN__
        app_bt_accessmode_set_req(BTIF_BT_DEFAULT_ACCESS_MODE_PAIR);
#else
        app_bt_accessmode_set_req(BTIF_BAM_CONNECTABLE_ONLY);
#endif
#endif
    }
}


void app_bt_resume_sniff_mode(uint8_t deviceId)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(deviceId);
    if (curr_device == NULL)
    {
        return;
    }
    if (bt_profile_connect_status_success == curr_device->profile_mgr.a2dp_connect||
        bt_profile_connect_status_success == curr_device->profile_mgr.hfp_connect)
    {
        app_bt_allow_sniff(deviceId);
        app_bt_sniff_config(curr_device->acl_conn_hdl);
    }
}

uint8_t app_bt_count_connected_device(void)
{
    uint8_t num_of_connected_dev = 0;
    uint8_t deviceId;
    struct BT_DEVICE_T *curr_device = NULL;

    for (deviceId = 0; deviceId < BT_DEVICE_NUM; deviceId++)
    {
        curr_device = app_bt_get_device(deviceId);
        if (curr_device->acl_is_connected)
        {
            num_of_connected_dev++;
        }
    }

    return num_of_connected_dev;
}

struct BT_DEVICE_T * app_bt_get_connected_sink_device(void)
{
    struct BT_DEVICE_T *curr_device = NULL;
    int i = 0;
        for (i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i += 1)
        {
            curr_device = app_bt_get_device(i);
            if (curr_device == NULL)
            {
                DEBUG_INFO(0, "curr_device NULL dev_id %d", i);
                continue;
            }
            if (curr_device->acl_is_connected)
            {
                return curr_device;
            }
        }
        return NULL;
}

struct BT_DEVICE_T *app_bt_get_connected_device_byaddr(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T *curr_device = NULL;
    int i = 0;
    if (remote == NULL)
    {
        return NULL;
    }
    for (i = 0; i < BT_DEVICE_NUM; i++)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->acl_is_connected && memcmp(&curr_device->remote, remote, sizeof(bt_bdaddr_t)) == 0)
        {
            return curr_device;
        }
    }
#if defined(BT_SOURCE)
    for (i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i += 1)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->acl_is_connected && memcmp(remote, &curr_device->remote, sizeof(bt_bdaddr_t)) == 0)
        {
            return curr_device;
        }
    }
#endif
    return NULL;
}

bt_status_t app_bt_avrcp_key_operation(const bt_bdaddr_t *remote, avrcp_panel_operation_t key, bool is_press)
{
    btif_avrcp_channel_t *channel = btif_get_avrcp_channel_by_addr((uint8_t *)remote->address);

    if (!channel)
    {
        return BT_STS_FAILED;
    }

    if (key == BTIF_AVRCP_POP_RESERVED)
    {
        return BT_STS_FAILED;
    }

    return btif_avrcp_set_panel_key(channel, key, is_press);
}

static void app_bt_precheck_before_starting_connecting(uint8_t isBtConnected)
{
#ifdef __IAG_BLE_INCLUDE__
    if (!isBtConnected)
    {
        // bes_ble_gap_force_switch_adv(BLE_SWITCH_USER_BT_CONNECT, false);
    }
#endif
}

void app_bt_restore_reconnecting_idle_mode(uint8_t deviceId)
{
    app_bt_get_device(deviceId)->profile_mgr.reconnect_mode = bt_profile_reconnect_null;
#ifdef __IAG_BLE_INCLUDE__
#ifndef BLE_ADV_NOT_START_DEFAULT
    app_start_fast_connectable_ble_adv(BLE_FAST_ADVERTISING_INTERVAL);
#endif
#endif
}

#ifndef IBRT
void app_bt_update_connectable_mode_after_connection_management(void)
{
    uint8_t deviceId;
    bool isEnterConnetableOnlyState = true;
    struct BT_DEVICE_T *curr_device = NULL;

    for (deviceId = 0; deviceId < BT_DEVICE_NUM; deviceId++)
    {
        // assure none of the device is in reconnecting mode
        curr_device = app_bt_get_device(deviceId);
        if (curr_device->profile_mgr.reconnect_mode != bt_profile_reconnect_null)
        {
            isEnterConnetableOnlyState = false;
            break;
        }
    }

    if (isEnterConnetableOnlyState)
    {
        for (deviceId = 0; deviceId < BT_DEVICE_NUM; deviceId++)
        {
            curr_device = app_bt_get_device(deviceId);
            if (!curr_device->profile_mgr.profile_connected)
            {
#if !defined(IBRT)
                app_bt_set_access_mode(BTIF_BAM_CONNECTABLE_ONLY);
#endif
                return;
            }
        }
    }
}
#endif

bool app_bt_check_is_ios_device(const bt_bdaddr_t *remote_addr)
{
#ifdef BT_DIP_SUPPORT
    bt_dip_pnp_info_t pnp_info = {0};
    bool pnp_is_exist = nv_record_get_pnp_info((bt_bdaddr_t *)remote_addr, &pnp_info);
    if(!pnp_is_exist)
    {
        return false;
    }

    return pnp_is_exist && bes_bt_dip_check_is_ios_device(pnp_info.vend_id, pnp_info.vend_id_source);
#else
    return false;
#endif
}

void app_audio_switch_flash_flush_req(void);

void app_bt_profile_connect_manager_hf(int id, btif_hf_channel_t* Chan, struct hfp_context *ctx)
{
#ifdef BT_HFP_SUPPORT
    btdevice_profile *btdevice_plf_p = (btdevice_profile *)app_bt_profile_active_store_ptr_get((uint8_t *)ctx->remote_dev_bdaddr.address);
    struct BT_DEVICE_T *curr_device = app_bt_get_device(id);
    struct app_bt_profile_manager *profile_mgr = &curr_device->profile_mgr;
    POSSIBLY_UNUSED uint8_t active_cons;

    ASSERT(curr_device->device_id == id, "bt profile manager device id shall match");

    osTimerStop(profile_mgr->reconnect_timer);
    profile_mgr->connect_timer_cb = NULL;
    active_cons = app_bt_get_active_cons();

    if (Chan)
    {
        switch(ctx->event)
        {
            case BTIF_HF_EVENT_REMOTE_NOT_SUPPORT:
                profile_mgr->hfp_connect = bt_profile_connect_status_success;
                profile_mgr->remote_support_hfp = false;
                profile_mgr->reconnect_cnt = 0;
                goto profile_reconnect_check_label;
                break;
            case BTIF_HF_EVENT_SERVICE_CONNECTED:
                DEBUG_INFO(1,"%s HF_EVENT_SERVICE_CONNECTED",__func__);
                nv_record_btdevicerecord_set_hfp_profile_active_state(btdevice_plf_p, true);
#ifndef FPGA
                nv_record_touch_cause_flush();
#endif
                profile_mgr->hfp_connect = bt_profile_connect_status_success;
                profile_mgr->reconnect_cnt = 0;

#ifndef IBRT
                if (false == profile_mgr->profile_connected)
                {
                    app_bt_resume_sniff_mode(id);
                }
#endif

#ifdef BT_DIP_SUPPORT
                app_dip_get_remote_info(id);
#endif

#ifdef BT_PBAP_SUPPORT
                app_bt_connect_pbap_profile(&curr_device->remote);
#endif

#ifdef BT_PAN_SUPPORT
                app_bt_connect_pan_profile(&curr_device->remote);
#endif

#if defined(BT_AVRCP_SUPPORT) && !defined(BT_A2DP_SUPPORT)
                DEBUG_INFO(0,"!!!continue connect avrcp\n");
                app_bt_reconnect_avrcp_profile(&curr_device->remote);
#endif

            profile_reconnect_check_label:
                if (profile_mgr->reconnect_mode == bt_profile_reconnect_openreconnecting)
                {
                    //do nothing
                }
#if defined(IBRT)
                else if (app_bt_ibrt_reconnect_mobile_profile_flag_get(&curr_device->remote))
                {
                    app_bt_ibrt_reconnect_mobile_profile_flag_clear(&curr_device->remote);
#else
                else if (profile_mgr->reconnect_mode == bt_profile_reconnect_reconnecting)
                {
#endif
                    DEBUG_INFO(2,"app_bt: a2dp_act in NV =%d,a2dp_connect=%d",btdevice_plf_p->a2dp_act,profile_mgr->a2dp_connect);
                    if (btdevice_plf_p->a2dp_act && profile_mgr->a2dp_connect != bt_profile_connect_status_success)
                    {
                        DEBUG_INFO(0,"!!!continue connect a2dp\n");
                        app_bt_precheck_before_starting_connecting(profile_mgr->profile_connected);
                        app_bt_reconnect_a2dp_profile(&profile_mgr->rmt_addr);
                    }
#if defined(__GATT_OVER_BR_EDR__)
                    if (app_bt_check_is_ios_device(&profile_mgr->rmt_addr) && !app_btgatt_is_connected(curr_device->device_id))
                    {
                        app_bt_reconnect_gatt_profile(&profile_mgr->rmt_addr);
                    }
#endif
                }

                app_bt_switch_role_if_needed(&curr_device->remote);
                break;
            case BTIF_HF_EVENT_SERVICE_DISCONNECTED:
                DEBUG_INFO(3,"%s HF_EVENT_SERVICE_DISCONNECTED discReason:%d/%d",__func__, ctx->disc_reason, ctx->disc_reason_saved);
                profile_mgr->hfp_connect = bt_profile_connect_status_failure;
                if (profile_mgr->reconnect_mode == bt_profile_reconnect_openreconnecting)
                {
                    if (++profile_mgr->reconnect_cnt < APP_BT_PROFILE_OPENNING_RECONNECT_RETRY_LIMIT_CNT)
                    {
#if !defined(IBRT)
                        app_bt_set_access_mode(BTIF_BAM_CONNECTABLE_ONLY);
#endif
#ifdef  __IAG_BLE_INCLUDE__
                        bes_ble_gap_start_connectable_adv(BLE_ADVERTISING_INTERVAL);
#endif
                        osTimerStart(profile_mgr->reconnect_timer, APP_BT_PROFILE_RECONNECT_RETRY_INTERVAL_MS);
                        profile_mgr->hfp_connect = bt_profile_connect_status_unknow;
                    }
                }
                else if (profile_mgr->reconnect_mode == bt_profile_reconnect_reconnecting)
                {
                    if (++profile_mgr->reconnect_cnt < APP_BT_PROFILE_RECONNECT_RETRY_LIMIT_CNT)
                    {
#if !defined(IBRT)
                        app_bt_set_access_mode(BTIF_BAM_CONNECTABLE_ONLY);
#endif
#ifdef  __IAG_BLE_INCLUDE__
                        bes_ble_gap_start_connectable_adv(BLE_ADVERTISING_INTERVAL);
#endif
                        osTimerStart(profile_mgr->reconnect_timer, APP_BT_PROFILE_RECONNECT_RETRY_INTERVAL_MS);
                    }
                    else
                    {
                        app_bt_restore_reconnecting_idle_mode(id);
                    }
                    DEBUG_INFO(2,"%s try to reconnect cnt:%d",__func__, profile_mgr->reconnect_cnt);
                }
                break;
            default:
                break;
        }
    }
    DUMP8("%02x ", &profile_mgr->rmt_addr.address, BT_ADDR_OUTPUT_PRINT_NUM);
    btdevice_profile *btdevice_plf_p1 = (btdevice_profile *)app_bt_profile_active_store_ptr_get((uint8_t *)&profile_mgr->rmt_addr.address);

    if (profile_mgr->reconnect_mode == bt_profile_reconnect_reconnecting)
    {
        bool reconnect_hfp_proc_final = false;
        bool reconnect_a2dp_proc_final = false;

        if (btdevice_plf_p1->hfp_act && profile_mgr->hfp_connect != bt_profile_connect_status_success)
        {
            reconnect_hfp_proc_final = false;
        }
        else
        {
            reconnect_hfp_proc_final = true;
        }
        if (btdevice_plf_p1->a2dp_act && profile_mgr->a2dp_connect != bt_profile_connect_status_success)
        {
            reconnect_a2dp_proc_final = false;
        }
        else
        {
            reconnect_a2dp_proc_final = true;
        }
        if (reconnect_hfp_proc_final && reconnect_a2dp_proc_final)
        {
            DEBUG_INFO(2,"!!!reconnect success %d/%d\n", profile_mgr->hfp_connect, profile_mgr->a2dp_connect);
            app_bt_restore_reconnecting_idle_mode(id);
        }
    }
    else if (profile_mgr->reconnect_mode == bt_profile_reconnect_openreconnecting)
    {
        bool opening_hfp_proc_final = false;
        bool opening_a2dp_proc_final = false;

        if (btdevice_plf_p1->hfp_act && profile_mgr->hfp_connect == bt_profile_connect_status_unknow)
        {
            opening_hfp_proc_final = false;
        }
        else
        {
            opening_hfp_proc_final = true;
        }

        if (btdevice_plf_p1->a2dp_act && profile_mgr->a2dp_connect == bt_profile_connect_status_unknow && besbt_cfg.a2dp_sink_enable)
        {
            opening_a2dp_proc_final = false;
        }
        else
        {
            opening_a2dp_proc_final = true;
        }

        if(opening_hfp_proc_final && opening_a2dp_proc_final)
        {
            DEBUG_INFO(2,"!!!reconnect success %d/%d\n", profile_mgr->hfp_connect, profile_mgr->a2dp_connect);
            app_bt_restore_reconnecting_idle_mode(id);
        }
        else if(profile_mgr->hfp_connect == bt_profile_connect_status_failure)
        {
            DEBUG_INFO(3,"reconnect_mode888:%d",profile_mgr->reconnect_mode);
            DEBUG_INFO(2,"!!!reconnect success %d/%d\n", profile_mgr->hfp_connect, profile_mgr->a2dp_connect);
            if ((profile_mgr->reconnect_mode == bt_profile_reconnect_openreconnecting)
                &&(profile_mgr->reconnect_cnt >= APP_BT_PROFILE_OPENNING_RECONNECT_RETRY_LIMIT_CNT))
            {
                app_bt_restore_reconnecting_idle_mode(id);
            }
        }

        if (btdevice_plf_p1->hfp_act && profile_mgr->hfp_connect == bt_profile_connect_status_success)
        {
            if (btdevice_plf_p1->a2dp_act && !opening_a2dp_proc_final)
            {
                if (besbt_cfg.a2dp_sink_enable)
                {
                    DEBUG_INFO(0,"!!!continue connect a2dp\n");
                    app_bt_precheck_before_starting_connecting(profile_mgr->profile_connected);
                    app_bt_reconnect_a2dp_profile(&profile_mgr->rmt_addr);
                }
            }
        }

        if (profile_mgr->reconnect_mode == bt_profile_reconnect_null)
        {
            app_bt_start_reconnect_next_device();
        }
    }
#ifdef __INTERCONNECTION__
    if (profile_mgr->hfp_connect == bt_profile_connect_status_success &&
        profile_mgr->a2dp_connect == bt_profile_connect_status_success)
    {
        app_interconnection_start_disappear_adv(INTERCONNECTION_BLE_ADVERTISING_INTERVAL,
                                                APP_INTERCONNECTION_DISAPPEAR_ADV_IN_MS);

        if (active_cons <= 2)
        {
            app_interconnection_spp_open(&curr_device->remote);
        }
    }
#endif

#ifdef  __IAG_BLE_INCLUDE__
    DEBUG_INFO(3, "%s hfp %d a2dp %d", __func__, profile_mgr->hfp_connect, profile_mgr->a2dp_connect);
    if (profile_mgr->hfp_connect == bt_profile_connect_status_success &&
        profile_mgr->a2dp_connect == bt_profile_connect_status_success)
    {
        // bes_ble_gap_force_switch_adv(BLE_SWITCH_USER_BT_CONNECT, true);
    }
#endif

    if (!profile_mgr->profile_connected &&
        (profile_mgr->hfp_connect == bt_profile_connect_status_success ||
         profile_mgr->a2dp_connect == bt_profile_connect_status_success))
    {

        profile_mgr->profile_connected = true;
        DEBUG_INFO(0,"BT connected!!!");

#ifndef IBRT
        app_bt_get_remote_device_name(&curr_device->remote);
#endif
#if defined(MEDIA_PLAYER_SUPPORT)&& !defined(IBRT)
        audio_player_play_prompt(AUD_ID_BT_CONNECTED, id);
#endif
    }

    if (profile_mgr->profile_connected &&
        (profile_mgr->hfp_connect != bt_profile_connect_status_success &&
         profile_mgr->a2dp_connect != bt_profile_connect_status_success))
    {

        profile_mgr->profile_connected = false;
        DEBUG_INFO(0,"BT disconnected!!!");

#ifdef GFPS_ENABLED
        if (gfps_is_last_response_pending())
        {
            gfps_enter_connectable_mode_req_handler(gfps_get_last_response());
        }
#endif

#if defined(MEDIA_PLAYER_SUPPORT)&& !defined(IBRT)
        audio_player_play_prompt(AUD_ID_BT_DIS_CONNECT, id);
#endif
#ifdef __INTERCONNECTION__
        app_interconnection_disconnected_callback();
#endif

        app_set_disconnecting_all_bt_connections(false);
    }

#ifndef IBRT
    app_bt_update_connectable_mode_after_connection_management();
#endif
#endif /* BT_HFP_SUPPORT */
}

void app_bt_profile_connect_manager_a2dp(int id, a2dp_stream_t *Stream, const   a2dp_callback_parms_t *info)
{
#ifdef BT_A2DP_SUPPORT
    btdevice_profile *btdevice_plf_p = NULL;
    btif_remote_device_t *remDev = NULL;
    btif_a2dp_callback_parms_t* Info = (btif_a2dp_callback_parms_t*)info;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(id);
    struct app_bt_profile_manager *profile_mgr = &curr_device->profile_mgr;
    ASSERT(curr_device->device_id == id, "bt profile manager device id shall match");

    osTimerStop(profile_mgr->reconnect_timer);
    profile_mgr->connect_timer_cb = NULL;

    remDev = btif_a2dp_get_remote_device(Stream);
    if (remDev)
    {
        btdevice_plf_p = (btdevice_profile *)app_bt_profile_active_store_ptr_get(btif_me_get_remote_device_bdaddr(remDev)->address);
    }
    else
    {
        btdevice_plf_p = (btdevice_profile *)app_bt_profile_active_store_ptr_get(NULL);
    }

    if (Stream&&Info)
    {

        switch(Info->event)
        {
            case BTIF_A2DP_REMOTE_NOT_SUPPORT:
                profile_mgr->a2dp_connect = bt_profile_connect_status_success;
                profile_mgr->remote_support_a2dp = false;
                profile_mgr->reconnect_cnt = 0;
                goto profile_reconnect_check_label;
                break;
            case BTIF_A2DP_EVENT_STREAM_OPEN:
            case BTIF_A2DP_EVENT_STREAM_OPEN_MOCK:
                DEBUG_INFO(4,"%s A2DP_EVENT_STREAM_OPEN,codec type=%x a2dp:%d mode:%d",
                      __func__, Info->p.configReq->codec.codecType,
                      profile_mgr->a2dp_connect,
                      profile_mgr->reconnect_mode);

                nv_record_btdevicerecord_set_a2dp_profile_active_state(btdevice_plf_p, true);
                nv_record_btdevicerecord_set_a2dp_profile_codec(btdevice_plf_p, Info->p.configReq->codec.codecType);
#ifndef FPGA
                nv_record_touch_cause_flush();
#endif

                if(profile_mgr->a2dp_connect == bt_profile_connect_status_success)
                {
                    DEBUG_INFO(0,"!!!a2dp has opened   force return ");
                    return;
                }
                profile_mgr->a2dp_connect = bt_profile_connect_status_success;
                profile_mgr->reconnect_cnt = 0;

#ifndef IBRT
                if (false == profile_mgr->profile_connected)
                {
                    app_bt_resume_sniff_mode(id);
                }
#endif

#ifdef BT_DIP_SUPPORT
                app_dip_get_remote_info(id);
#endif

            profile_reconnect_check_label:
                if (profile_mgr->reconnect_mode == bt_profile_reconnect_openreconnecting)
                {
                    //do nothing
                }
#if defined(IBRT)
                else if (app_bt_ibrt_reconnect_mobile_profile_flag_get(&curr_device->remote))
                {
                    app_bt_ibrt_reconnect_mobile_profile_flag_clear(&curr_device->remote);
#else
                else if (profile_mgr->reconnect_mode == bt_profile_reconnect_reconnecting)
                {
#endif
                    DEBUG_INFO(2,"app_bt: hfp_act in NV =%d,a2dp_connect=%d",btdevice_plf_p->hfp_act,profile_mgr->hfp_connect);
                    if (btdevice_plf_p->hfp_act && profile_mgr->hfp_connect != bt_profile_connect_status_success)
                    {
                        DEBUG_INFO(0,"!!!continue connect hfp\n");
                        app_bt_precheck_before_starting_connecting(profile_mgr->profile_connected);
                        app_bt_reconnect_hfp_profile((bt_bdaddr_t *)&profile_mgr->rmt_addr);
                    }
                }

#ifdef APP_DISABLE_PAGE_SCAN_AFTER_CONN
                disable_page_scan_check_timer_start();
#endif

                app_bt_switch_role_if_needed(&curr_device->remote);
                break;
            case BTIF_A2DP_EVENT_STREAM_CLOSED:

                DEBUG_INFO(2,"%s A2DP_EVENT_STREAM_CLOSED discReason1:%d",__func__, Info->discReason);

                if(Stream!=NULL)
                {
                    btif_remote_device_t *rmt_dev = btif_a2dp_get_remote_device(Stream);
                    if(rmt_dev)
                        DEBUG_INFO(2,"%s A2DP_EVENT_STREAM_CLOSED discReason2:%d",__func__,btif_me_get_remote_device_disc_reason_saved(rmt_dev));
                }

                profile_mgr->a2dp_connect = bt_profile_connect_status_failure;

                if (profile_mgr->reconnect_mode == bt_profile_reconnect_openreconnecting)
                {
                    if (++profile_mgr->reconnect_cnt < APP_BT_PROFILE_OPENNING_RECONNECT_RETRY_LIMIT_CNT)
                    {
#if !defined(IBRT)
                        app_bt_set_access_mode(BTIF_BAM_CONNECTABLE_ONLY);
#endif
#ifdef __IAG_BLE_INCLUDE__
                        bes_ble_gap_start_connectable_adv(BLE_ADVERTISING_INTERVAL);
#endif
                        osTimerStart(profile_mgr->reconnect_timer, APP_BT_PROFILE_RECONNECT_RETRY_INTERVAL_MS);
                        DEBUG_INFO(2,"%s: a2dp_connect = %d",__func__, bt_profile_connect_status_failure);
                        profile_mgr->a2dp_connect = bt_profile_connect_status_unknow;
                    }
                }
                else if (profile_mgr->reconnect_mode == bt_profile_reconnect_reconnecting)
                {
                    if (++profile_mgr->reconnect_cnt < APP_BT_PROFILE_RECONNECT_RETRY_LIMIT_CNT)
                    {
#if !defined(IBRT)
                        app_bt_set_access_mode(BTIF_BAM_CONNECTABLE_ONLY);
#endif
#ifdef __IAG_BLE_INCLUDE__
                        bes_ble_gap_start_connectable_adv(BLE_ADVERTISING_INTERVAL);
#endif
                        osTimerStart(profile_mgr->reconnect_timer, APP_BT_PROFILE_RECONNECT_RETRY_INTERVAL_MS);
                    }
                    else
                    {
                        app_bt_restore_reconnecting_idle_mode(id);
                    }
                    DEBUG_INFO(2,"%s try to reconnect cnt:%d",__func__, profile_mgr->reconnect_cnt);
                }
                break;
            default:
                break;
        }
    }

    if (profile_mgr->reconnect_mode == bt_profile_reconnect_reconnecting)
    {
        bool reconnect_hfp_proc_final = true;
        bool reconnect_a2dp_proc_final = true;
        if (profile_mgr->hfp_connect == bt_profile_connect_status_failure)
        {
            reconnect_hfp_proc_final = false;
        }
        if (profile_mgr->a2dp_connect == bt_profile_connect_status_failure)
        {
            reconnect_a2dp_proc_final = false;
        }
        if (reconnect_hfp_proc_final && reconnect_a2dp_proc_final)
        {
            DEBUG_INFO(2,"!!!reconnect success %d/%d\n", profile_mgr->hfp_connect, profile_mgr->a2dp_connect);
            app_bt_restore_reconnecting_idle_mode(id);
        }
    }
    else if (profile_mgr->reconnect_mode == bt_profile_reconnect_openreconnecting)
    {
        bool opening_hfp_proc_final = false;
        bool opening_a2dp_proc_final = false;

        if (btdevice_plf_p->hfp_act && profile_mgr->hfp_connect == bt_profile_connect_status_unknow)
        {
            opening_hfp_proc_final = false;
        }
        else
        {
            opening_hfp_proc_final = true;
        }

        if (btdevice_plf_p->a2dp_act && profile_mgr->a2dp_connect == bt_profile_connect_status_unknow)
        {
            opening_a2dp_proc_final = false;
        }
        else
        {
            opening_a2dp_proc_final = true;
        }

        if ((opening_hfp_proc_final && opening_a2dp_proc_final) ||
            (profile_mgr->a2dp_connect == bt_profile_connect_status_failure))
        {
            DEBUG_INFO(2,"!!!reconnect success %d/%d\n", profile_mgr->hfp_connect, profile_mgr->a2dp_connect);
            app_bt_restore_reconnecting_idle_mode(id);
        }

        if (btdevice_plf_p->a2dp_act && profile_mgr->a2dp_connect== bt_profile_connect_status_success)
        {
            if (btdevice_plf_p->hfp_act && !opening_hfp_proc_final)
            {
                DEBUG_INFO(0,"!!!continue connect hf\n");
                app_bt_precheck_before_starting_connecting(profile_mgr->profile_connected);
                app_bt_reconnect_hfp_profile((bt_bdaddr_t *)&profile_mgr->rmt_addr);
            }
        }

        if (profile_mgr->reconnect_mode == bt_profile_reconnect_null)
        {
            app_bt_start_reconnect_next_device();
        }
    }

#ifdef __INTERCONNECTION__
    if (profile_mgr->hfp_connect == bt_profile_connect_status_success &&
        profile_mgr->a2dp_connect == bt_profile_connect_status_success)
    {
        app_interconnection_start_disappear_adv(INTERCONNECTION_BLE_ADVERTISING_INTERVAL,
                                                APP_INTERCONNECTION_DISAPPEAR_ADV_IN_MS);

        if (app_bt_get_active_cons() <= 2)
        {
            app_interconnection_spp_open(&curr_device->remote);
        }
    }
#endif

#ifdef  __IAG_BLE_INCLUDE__
    DEBUG_INFO(3, "%s hfp %d a2dp %d", __func__, profile_mgr->hfp_connect, profile_mgr->a2dp_connect);
    if (profile_mgr->hfp_connect == bt_profile_connect_status_success &&
        profile_mgr->a2dp_connect == bt_profile_connect_status_success)
    {
        // bes_ble_gap_force_switch_adv(BLE_SWITCH_USER_BT_CONNECT, true);
    }
#endif

    if (!profile_mgr->profile_connected &&
        (profile_mgr->hfp_connect == bt_profile_connect_status_success ||
         profile_mgr->a2dp_connect == bt_profile_connect_status_success))
    {

        profile_mgr->profile_connected = true;
        DEBUG_INFO(0,"BT connected!!!");

#ifndef IBRT
        app_bt_get_remote_device_name(&curr_device->remote);
#endif
#if defined(MEDIA_PLAYER_SUPPORT)&& !defined(IBRT)
        audio_player_play_prompt(AUD_ID_BT_CONNECTED, id);
#endif
    }

    if (profile_mgr->profile_connected &&
        (profile_mgr->hfp_connect != bt_profile_connect_status_success &&
         profile_mgr->a2dp_connect != bt_profile_connect_status_success))
    {

        profile_mgr->profile_connected = false;
        DEBUG_INFO(0,"BT disconnected!!!");

#ifdef GFPS_ENABLED
        if (gfps_is_last_response_pending())
        {
            gfps_enter_connectable_mode_req_handler(gfps_get_last_response());
        }
#endif

#if defined(MEDIA_PLAYER_SUPPORT)&& !defined(IBRT)
        audio_player_play_prompt(AUD_ID_BT_DIS_CONNECT, id);
#endif
#ifdef __INTERCONNECTION__
        app_interconnection_disconnected_callback();
#endif

        app_set_disconnecting_all_bt_connections(false);
    }

#ifndef IBRT
    app_bt_update_connectable_mode_after_connection_management();
#endif

    DEBUG_INFO(1,"%s done.",__func__);
#endif /* BT_A2DP_SUPPORT */
}

void app_bt_profile_connect_manager_avrcp(uint8_t device_id, btif_avctp_event_t event)
{
#if defined(BT_AVRCP_SUPPORT) && !defined(BT_A2DP_SUPPORT)
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    struct app_bt_profile_manager *profile_mgr = &curr_device->profile_mgr;
    osTimerStop(profile_mgr->reconnect_timer);
    profile_mgr->connect_timer_cb = NULL;
    TRACE(1,"%s %d event=%d reconnect_cnt=%d.",__func__, __LINE__, event, profile_mgr->reconnect_cnt);

    switch(event)
    {
        case BTIF_AVRCP_EVENT_CONNECT:
            profile_mgr->avrcp_connect = bt_profile_connect_status_success;
            profile_mgr->reconnect_cnt = 0;
            break;
        case BTIF_AVRCP_EVENT_DISCONNECT:
            profile_mgr->avrcp_connect = bt_profile_connect_status_failure;
            if (++profile_mgr->reconnect_cnt < APP_BT_PROFILE_OPENNING_RECONNECT_RETRY_LIMIT_CNT)
            {
                osTimerStart(profile_mgr->reconnect_timer, APP_BT_PROFILE_RECONNECT_RETRY_INTERVAL_MS);
                profile_mgr->a2dp_connect = bt_profile_connect_status_unknow;
            }
            break;
        default:
            break;
    }
#endif /* BT_AVRCP_SUPPORT */
}

static bool isDisconnectAllBtConnections = false;

bool app_is_disconnecting_all_bt_connections(void)
{
    return isDisconnectAllBtConnections;
}

void app_set_disconnecting_all_bt_connections(bool isEnable)
{
    isDisconnectAllBtConnections = isEnable;
}

#if defined(IBRT)
bool app_bt_is_any_connection(void)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    if (p_ibrt_ctrl->init_done)
    {
        struct BT_DEVICE_T *curr_device = NULL;
        for (int i = 0; i < BT_DEVICE_NUM; ++i)
        {
            curr_device = app_bt_get_device(i);
            if (curr_device->acl_is_connected && IBRT_MASTER == app_tws_get_ibrt_role(&curr_device->remote))
            {
                if (app_tws_ibrt_mobile_link_connected(&curr_device->remote))
                {
                    return true;
                }
            }
        }

        if (app_tws_ibrt_tws_link_connected())
        {
            return true;
        }
    }

    return false;
}
#endif

#define APP_BT_DELAY_TO_DICONNECT_ACL_LINK_MS 300
osTimerId app_bt_delay_to_disconnect_acl_timer_id = 0;
static void app_bt_delay_to_disconnect_acl_timer_handler(void const *n);
osTimerDef (APP_BT_DELAY_TO_DISCONNECT_ACL_TIER, app_bt_delay_to_disconnect_acl_timer_handler);
btif_remote_device_t *disconnect_device = NULL;

static void app_bt_delay_disconnect_acl_link(void)
{
    if (app_bt_delay_to_disconnect_acl_timer_id == 0)
    {
        app_bt_delay_to_disconnect_acl_timer_id =
            osTimerCreate(osTimer(APP_BT_DELAY_TO_DISCONNECT_ACL_TIER), osTimerOnce, NULL);
    }
    if (app_bt_delay_to_disconnect_acl_timer_id)
    {
        osTimerStop(app_bt_delay_to_disconnect_acl_timer_id);
        osTimerStart(app_bt_delay_to_disconnect_acl_timer_id, APP_BT_DELAY_TO_DICONNECT_ACL_LINK_MS);
    }
    else
    {
        DEBUG_INFO(0, "%s invalid timer", __func__);
    }
}

static void app_bt_disconnect_mobile_acl_link(void)
{
    if (disconnect_device)
    {
#ifdef IBRT
        app_tws_ibrt_disconnect_connection(disconnect_device);
#endif
        disconnect_device = NULL;
    }
}

static void app_bt_delay_to_disconnect_acl_timer_handler(void const *n)
{
    app_bt_start_custom_function_in_bt_thread((uint32_t)NULL, (uint32_t)NULL, (uint32_t)app_bt_disconnect_mobile_acl_link);
}

void app_bt_disconnect_all_profiles_and_acl(btif_remote_device_t *remote_device)
{
    if (!remote_device)
    {
        DEBUG_INFO(1, "%s invalid device", __func__);
    }

    uint8_t device_id = btif_me_get_device_id_from_rdev(remote_device);

    DEBUG_INFO(1, "%s device id:%d", __func__, device_id);

    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);

    if (curr_device != NULL)
    {
        if (btif_get_hf_chan_state(curr_device->hf_channel) == BT_HFP_CHAN_STATE_OPEN)
        {
            app_bt_HF_DisconnectServiceLink(curr_device->hf_channel);
        }

        if (btif_a2dp_get_stream_state(curr_device->btif_a2dp_stream->a2dp_stream) == BT_A2DP_STREAM_STATE_STREAMING ||
            btif_a2dp_get_stream_state(curr_device->btif_a2dp_stream->a2dp_stream) == BT_A2DP_STREAM_STATE_OPEN)
        {
            app_bt_A2DP_CloseStream(curr_device->btif_a2dp_stream->a2dp_stream);
        }

        if (btif_avrcp_get_remote_device(curr_device->avrcp_channel))
        {
            btif_avrcp_disconnect(curr_device->avrcp_channel);
        }

        disconnect_device = remote_device;
        app_bt_delay_disconnect_acl_link();
    }
}

bool app_bt_need_disconnect_profiles_before_acl(btif_remote_device_t *remote_device)
{
    if(remote_device){
        bt_remver_t * rem_ver = (bt_remver_t *)btif_me_get_remote_device_version(remote_device);
        if(rem_ver){
            DEBUG_INFO(1, "compid:0x%x, subvers:0x%x", rem_ver->compid, rem_ver->subvers);
            if ((rem_ver->compid == 0xa) && (rem_ver->subvers == 0x2918)){   //Sony S313
                DEBUG_INFO(1, "need disconnect profiles before acl.");
                return true;
            }
        }
    }
    return false;
}

bt_status_t LinkDisconnectDirectly(bool PowerOffFlag)
{
    app_set_disconnecting_all_bt_connections(true);
    //DEBUG_INFO(1,"osapi_lock_is_exist:%d", btif_osapi_lock_is_exist());
    if(btif_osapi_lock_is_exist())
        btif_osapi_lock_stack();
#ifdef __IAG_BLE_INCLUDE__
    DEBUG_INFO(1,"ble_connected_state:%d", bes_ble_gap_is_any_connection_exist());
#endif

#if defined(IBRT)
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if(true==PowerOffFlag)
        p_ibrt_ctrl->ibrt_in_poweroff= true;

    if (p_ibrt_ctrl->init_done)
    {
#if defined(USE_SAFE_DISCONNECT)
        app_custom_ui_cancel_all_connection();
        app_custom_ui_all_safe_disconnect();
#else
        struct BT_DEVICE_T *curr_device = NULL;
        for (int i = 0; i < BT_DEVICE_NUM; ++i)
        {
            curr_device = app_bt_get_device(i);
            if (curr_device->acl_is_connected)
            {
                #ifdef IBRT_UI
                    app_tws_ibrt_disconnect_connection(app_bt_get_remote_dev_by_handle(curr_device->acl_conn_hdl));
                #else
                    app_tws_ibrt_disconnect_connection(p_ibrt_ctrl->tws_conhandle);
                #endif
            }
        }
        if (app_tws_ibrt_tws_link_connected())
        {
        #ifdef IBRT_UI
            app_ibrt_if_tws_disconnect_request();
        #else
            app_tws_ibrt_disconnect_connection(p_ibrt_ctrl->tws_conhandle);
        #endif
        }
#endif
    }

    if(btif_osapi_lock_is_exist())
        btif_osapi_unlock_stack();

    return BT_STS_SUCCESS;
#endif

    uint8_t active_cons = app_bt_get_active_cons();

    DEBUG_INFO(1,"activeCons:%d", active_cons);

    if(active_cons)
    {

#if defined(BT_SOURCE)
        if (app_bt_source_is_enabled())
        {
            app_bt_source_disconnect_all_connections(PowerOffFlag);
        }
#endif

        if (app_bt_sink_is_enabled())
        {
            POSSIBLY_UNUSED struct BT_DEVICE_T* curr_device = NULL;

#ifdef BT_HID_DEVICE
            app_bt_hid_disconnect_all_channels();
#endif
            for (int i = 0; i < BT_DEVICE_NUM; ++i)
            {
                curr_device = app_bt_get_device(i);
#ifdef BT_HFP_SUPPORT
                if (btif_get_hf_chan_state(curr_device->hf_channel) == BT_HFP_CHAN_STATE_OPEN)
                {
                    app_bt_HF_DisconnectServiceLink(curr_device->hf_channel);
                }
#endif
#ifdef BT_A2DP_SUPPORT
                bt_a2dp_stream_state_t stream_state;
                stream_state = btif_a2dp_get_stream_state(curr_device->btif_a2dp_stream->a2dp_stream);
                if(stream_state == BT_A2DP_STREAM_STATE_STREAMING ||
                   stream_state == BT_A2DP_STREAM_STATE_OPEN)
                {
                    app_bt_A2DP_CloseStream(curr_device->btif_a2dp_stream->a2dp_stream);
                }
#endif
#ifdef BT_AVRCP_SUPPORT
                if(btif_avrcp_get_remote_device(curr_device->avrcp_channel))
                {
                    app_bt_disconnect_avrcp_profile(curr_device->avrcp_channel);
                }
#endif
#ifdef BT_PBAP_SUPPORT
                if (app_bt_pbap_is_connected(curr_device->pbap_channel))
                {
                    app_bt_disconnect_pbap_profile(curr_device->pbap_channel);
                }
#endif
#ifdef BT_PAN_SUPPORT
                if (app_bt_pan_is_connected(i))
                {
                    app_bt_disconnect_pan_profile(i);
                }
#endif
            }
        }

#ifdef BISTO_ENABLED
        gsound_custom_bt_disconnect_all_channel();
#endif
    }

#ifdef __IAG_BLE_INCLUDE__
    if(bes_ble_gap_is_any_connection_exist())
    {
#ifdef GFPS_ENABLED
        if (!gfps_is_last_response_pending())
#endif
            bes_ble_gap_disconnect_all();
    }
#endif

    if(btif_osapi_lock_is_exist())
        btif_osapi_unlock_stack();

    osDelay(500);

    if(active_cons)
    {
        if (app_bt_sink_is_enabled())
        {
            struct BT_DEVICE_T *curr_device = NULL;
            for (int i = 0; i < BT_DEVICE_NUM; ++i)
            {
                curr_device = app_bt_get_device(i);
                if (curr_device->acl_is_connected)
                {
                    app_bt_MeDisconnectLink(curr_device->acl_conn_hdl);
                    osDelay(200);
                }
            }
        }
    }

    return BT_STS_SUCCESS;
}

void app_disconnect_all_bt_connections(bool power_off_flag)
{
    LinkDisconnectDirectly(power_off_flag);
}

void app_bt_disconnect_link_by_id(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    DEBUG_INFO(3, "(d%x) %s %p", device_id, __func__, curr_device ? curr_device->btm_conn : NULL);

    if (curr_device && curr_device->btm_conn)
    {
        app_bt_MeDisconnectLink(curr_device->acl_conn_hdl);
    }
}

void app_bt_disconnect_link_byaddr(bt_bdaddr_t * remote)
{
    uint8_t device_id = app_bt_get_connected_device_id_byaddr(remote);

    DEBUG_INFO(0, "(d%x) %s %02x:%02x:%02x:%02x:%02x:%02x", device_id, __func__,
        remote->address[0], remote->address[1], remote->address[2],
        remote->address[3], remote->address[4], remote->address[5]);

    app_bt_disconnect_link_by_id(device_id);
}

void app_bt_disconnect_acl_link(void)
{
    struct BT_DEVICE_T *curr_device = NULL;
    struct BT_DEVICE_T *device_start = app_bt_manager_get_device_all_start();
    struct BT_DEVICE_T *device_end = app_bt_manager_get_device_all_end();

    for (curr_device = device_start; curr_device < device_end; curr_device += 1)
    {
        if (curr_device->acl_is_connected)
        {
            app_bt_disconnect_link_by_id(curr_device->device_id);
            return;
        }
    }

    DEBUG_INFO(2, "%s no connected link found", __func__);
}

void app_bt_disconnect_all_acl_link(void)
{
    struct BT_DEVICE_T *curr_device = NULL;
    struct BT_DEVICE_T *device_start = app_bt_manager_get_device_all_start();
    struct BT_DEVICE_T *device_end = app_bt_manager_get_device_all_end();

    for (curr_device = device_start; curr_device < device_end; curr_device += 1)
    {
        if (curr_device->acl_is_connected)
        {
            app_bt_disconnect_link_by_id(curr_device->device_id);
        }
    }
}

void app_bt_init(void)
{
    //All use APP_MODULE_BT from ep
    app_set_threadhandle(APP_MODULE_BT, app_bt_handle_process);
    app_bt_active_mode_manager_init();
    btif_me_register_get_local_device_callback(app_bt_get_local_device_address);
    bt_audio_set_event_handler(app_bt_audio_event_handler);

#if defined(__CONNECTIVITY_LOG_REPORT__)
    hci_register_log_report_handler_callback(app_bt_acl_data_packet_check);
#endif
#if defined(IBRT)
    app_bt_ibrt_reconnect_mobile_profile_flag_init();
#endif
#ifdef RESUME_MUSIC_AFTER_CRASH_REBOOT
    app_bt_resume_music_after_crash_reboot_init();
#endif
}

extern "C" bool app_bt_has_connectivitys(void)
{
    int activeCons;

    activeCons = app_bt_get_active_cons();

    if(activeCons > 0)
        return true;

    return false;
}


#ifdef __TWS_CHARGER_BOX__

extern "C" {
    bt_status_t ME_Ble_Clear_Whitelist(void);
    bt_status_t ME_Ble_Set_Private_Address(BT_BD_ADDR *addr);
    bt_status_t ME_Ble_Add_Dev_To_Whitelist(U8 addr_type,BT_BD_ADDR *addr);
    bt_status_t ME_Ble_SetAdv_data(U8 len, U8 *data);
    bt_status_t ME_Ble_SetScanRsp_data(U8 len, U8 *data);
    bt_status_t ME_Ble_SetAdv_parameters(adv_para_struct *para);
    bt_status_t ME_Ble_SetAdv_en(U8 en);
    bt_status_t ME_Ble_Setscan_parameter(scan_para_struct *para);
    bt_status_t ME_Ble_Setscan_en(U8 scan_en,  U8 filter_duplicate);
}


int8_t power_level=0;
#define TWS_BOX_OPEN 1
#define TWS_BOX_CLOSE 0
void app_tws_box_set_slave_adv_data(uint8_t power_level,uint8_t box_status)
{
    uint8_t adv_data[] =
    {
        0x02,0xfe, 0x00,
        0x02, 0xfd, 0x00  // manufacturer data
    };

    adv_data[2] = power_level;

    adv_data[5] = box_status;
    ME_Ble_SetAdv_data(sizeof(adv_data), adv_data);

}


void app_tws_box_set_slave_adv_para(void)
{
    uint8_t  peer_addr[BTIF_BD_ADDR_SIZE] = {0};
    adv_para_struct para;


    para.interval_min =             0x0040; // 20ms
    para.interval_max =             0x0040; // 20ms
    para.adv_type =                 0x03;
    para.own_addr_type =            0x01;
    para.peer_addr_type =           0x01;
    para.adv_chanmap =              0x07;
    para.adv_filter_policy =        0x00;
    memcpy(para.bd_addr.addr, peer_addr, BTIF_BD_ADDR_SIZE);

    ME_Ble_SetAdv_parameters(&para);
}

void app_tws_start_chargerbox_adv(void)
{
    app_tws_box_set_slave_adv_data(power_level,TWS_BOX_OPEN);
    ME_Ble_Set_Private_Address((BT_BD_ADDR *)bt_get_local_address());
    app_tws_box_set_slave_adv_para();
    ME_Ble_SetAdv_en(1);
}
#endif

bool app_is_hfp_service_connected(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    return (curr_device->profile_mgr.hfp_connect == bt_profile_connect_status_success &&
            curr_device->profile_mgr.remote_support_hfp == true);
}

bool app_bt_has_mobile_device_in_sniff_mode(void)
{
    struct BT_DEVICE_T *curr_device = NULL;

    for (int i = 0; i < BT_DEVICE_NUM; i += 1)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->acl_is_connected && btif_me_is_in_sniff_mode(&curr_device->remote))
        {
            return true;
        }
    }

    return false;
}

bool app_bt_has_mobile_device_in_active_mode(void)
{
    struct BT_DEVICE_T *curr_device = NULL;

    for (int i = 0; i < BT_DEVICE_NUM; i += 1)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->acl_is_connected && btif_me_is_in_active_mode(&curr_device->remote))
        {
            return true;
        }
    }

    return false;
}


void app_bt_stay_active_rem_dev(uint16_t conn_handle)
{
    if (conn_handle != BT_INVALID_CONN_HANDLE)
    {
        btif_cmgr_handler_t    *cmgrHandler;
        /* Clear the sniff timer */
        cmgrHandler = btif_cmgr_get_acl_handler(conn_handle);

        if (cmgrHandler != NULL)
        {
            btif_cmgr_clear_sniff_timer(cmgrHandler);
            btif_cmgr_disable_sniff_timer(cmgrHandler);
            app_bt_Me_SetLinkPolicy(conn_handle, BTIF_BLP_MASTER_SLAVE_SWITCH);
        }
        else
        {
            DEBUG_INFO(2, "%s: cmgrHandler is null,conn_handle=%x", __func__, conn_handle);
        }
    }
}

void app_bt_stay_active(uint8_t deviceId)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(deviceId);

    if (curr_device->acl_is_connected)
    {
        app_bt_stay_active_rem_dev(curr_device->acl_conn_hdl);
    }
    else
    {
        DEBUG_INFO(2, "%s: acl is disconnected,device_id=%d", __func__, deviceId);
    }
}

void app_bt_allow_sniff_rem_dev(uint16_t conn_handle)
{
    btif_remote_device_t *btm_conn = app_bt_get_remote_dev_by_handle(conn_handle);
    if (!btm_conn)
    {
        DEBUG_INFO(0, "%s: %d btm_conn is NULL!", __func__, __LINE__);
        return;
    }
    struct BT_DEVICE_T *curr_device = app_bt_get_device(btif_me_get_device_id_from_rdev(btm_conn));
    if (curr_device && curr_device->acl_is_connected)
    {
        btif_cmgr_handler_t *cmgrHandler = btif_cmgr_get_acl_handler(curr_device->acl_conn_hdl);

        /* Start the sniff timer */
        btif_sniff_info_t sniffInfo;
        sniffInfo.minInterval = BTIF_CMGR_SNIFF_MIN_INTERVAL;
        sniffInfo.maxInterval = BTIF_CMGR_SNIFF_MAX_INTERVAL;
        sniffInfo.attempt = BTIF_CMGR_SNIFF_ATTEMPT;
        sniffInfo.timeout = BTIF_CMGR_SNIFF_TIMEOUT;

        btif_sniff_timer_t timer_mgr = {0,};
        timer_mgr.updata_sniff_timer = true;
        timer_mgr.timeout = BTIF_CMGR_SNIFF_TIMER;

        if (cmgrHandler)
        {
            btif_cmgr_set_sniff_timer(cmgrHandler, &sniffInfo, (void *)&timer_mgr);
        }
        app_bt_Me_SetLinkPolicy(curr_device->acl_conn_hdl, BTIF_BLP_MASTER_SLAVE_SWITCH | BTIF_BLP_SNIFF_MODE);
    }
}

void app_bt_allow_sniff(int device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (!curr_device)
    {
        return;
    }
    if (app_bt_audio_count_straming_mobile_links() > 0)
    {
        DEBUG_INFO(0, "%s: %d", __func__, __LINE__);
        return;
    }
    app_bt_allow_sniff_rem_dev(curr_device->acl_conn_hdl);
}

void app_bt_conn_stop_sniff(uint16_t conn_handle)
{
    app_bt_ME_StopSniff(conn_handle);
}

void app_bt_stop_sniff(uint8_t deviceId)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(deviceId);
    if (curr_device && curr_device->acl_is_connected)
    {
        app_bt_conn_stop_sniff(curr_device->acl_conn_hdl);
    }
}

btif_remote_device_t *app_bt_get_remote_dev_by_device_id(uint8_t device_id)
{
    btif_remote_device_t* currentRemDev = NULL;

    if (device_id != BT_DEVICE_TWS_ID)
    {
        struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
        if (curr_device && curr_device->acl_is_connected)
        {
            currentRemDev = app_bt_get_remote_dev_by_address(&curr_device->remote);
        }
    }
#ifdef IBRT
    else
    {
        if (app_bt_manager.tws_conn.acl_is_connected)
        {
            currentRemDev = app_bt_get_remote_dev_by_address(&app_bt_manager.tws_conn.remote);
        }
        else
        {
            DEBUG_INFO(0, "app_bt_get_remote_dev_by_device_id: no tws conn hdl %04x conn %p",
                app_bt_manager.tws_conn.acl_conn_hdl, app_bt_manager.tws_conn.btm_conn);
        }
    }
#endif

    return currentRemDev;
}

bool app_bt_get_device_bdaddr(uint8_t deviceId, uint8_t* btAddr)
{
    bool ret = false;

    if (app_bt_is_device_profile_connected(deviceId))
    {
        btif_remote_device_t* currentRemDev = app_bt_get_remote_dev_by_device_id(deviceId);
        bt_bdaddr_t *bt_addr;

        if (currentRemDev)
        {
            bt_addr = btif_me_get_remote_device_bdaddr(currentRemDev);
            memcpy(btAddr, bt_addr->address, BTIF_BD_ADDR_SIZE);
            ret = true;
        }
    }

    return ret;
}

bool app_bt_is_hfp_audio_on(void)
{
    bool hfp_audio_is_on = false;
    for (uint8_t i=0; i<BT_DEVICE_NUM; i++)
    {
        if(BT_HFP_AUDIO_CON == app_bt_get_device(i)->hf_audio_state)
        {
            hfp_audio_is_on = true;
            break;
        }
    }
    return hfp_audio_is_on;
}

bt_bdaddr_t* app_bt_get_remote_device_address(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (curr_device->acl_is_connected)
    {
        return &curr_device->remote;
    }
    else
    {
        return NULL;
    }
}

struct app_bt_search_t
{
    bool search_start;
    bool inquiry_pending;
    bool device_searched;
    uint8_t search_times;
    bt_bdaddr_t address;
};

#ifdef __SOURCE_TRACE_RX__
#define DEVICE_NUMBER 30
static device_info_t device_list[DEVICE_NUMBER];

static void device_list_init()
{
    memset(device_list, 0, sizeof(device_list));
}

static bool is_bt_bdaddr_t_null(bt_bdaddr_t *btaddr)
{
    bt_bdaddr_t btaddr_temp;

    memset(&btaddr_temp, 0, sizeof(btaddr_temp));

    if(memcmp(btaddr_temp.address, btaddr->address, sizeof(bt_bdaddr_t)) == 0)
    {
        return true;
    }
    return false;
}

static void device_list_add(bt_bdaddr_t *btaddr, char *device_name, uint32_t name_len)
{
    for(int i = 0; i < DEVICE_NUMBER; i++)
    {
        if(memcmp(btaddr, device_list[i].addr.address, sizeof(bt_bdaddr_t)) == 0)
        {
            break;
        }

        if(is_bt_bdaddr_t_null(&device_list[i].addr))
        {
            if(name_len > 0)
            {
                device_list[i].index = i;
                memcpy(&device_list[i].addr, btaddr, sizeof(bt_bdaddr_t));
                memcpy(device_list[i].name, device_name, name_len);
            }
            break;
        }
    }
}

device_info_t get_device_info_by_index(uint32_t index)
{
    if(index >= DEVICE_NUMBER)
    {
        index = 0;
    }

    return device_list[index];
}
#else

#define APP_BT_MAX_EXCEPT_DEVICE_NUM 3

struct bt_search_except_device
{
    bt_bdaddr_t device;
    bool inuse;
};

static struct bt_search_except_device g_except_device_list[APP_BT_MAX_EXCEPT_DEVICE_NUM];

void app_bt_clear_search_except_device_list(void)
{
    for (int i = 0; i < APP_BT_MAX_EXCEPT_DEVICE_NUM; i += 1)
    {
        g_except_device_list[i].inuse = false;
    }
}

void app_bt_del_search_except_device(const bt_bdaddr_t *bdaddr)
{
    for (int i = 0; i < APP_BT_MAX_EXCEPT_DEVICE_NUM; i += 1)
    {
        if (memcmp(bdaddr, g_except_device_list[i].device.address, sizeof(bt_bdaddr_t)) == 0)
        {
            g_except_device_list[i].inuse = false;
        }
    }
}

void app_bt_add_search_except_device(const bt_bdaddr_t *bdaddr)
{
    for (int i = 0; i < APP_BT_MAX_EXCEPT_DEVICE_NUM; i += 1)
    {
        if (!g_except_device_list[i].inuse)
        {
            g_except_device_list[i].device = *bdaddr;
            g_except_device_list[i].inuse = true;
            DEBUG_INFO(7, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
                  bdaddr->address[0], bdaddr->address[1], bdaddr->address[2],
                  bdaddr->address[3], bdaddr->address[4], bdaddr->address[5]);
            break;
        }
    }
}

static bool app_bt_search_device_match(const bt_bdaddr_t* bdaddr, const char* name)
{
    int count = 0;
    int i = 0;
    int j = 0;
    bt_bdaddr_t device_list[] =
    {
        {{0xd2, 0x53, 0x86, 0x42, 0x71, 0x31}},
        {{0xd3, 0x53, 0x86, 0x42, 0x71, 0x31}},
        {{0x28, 0x7e, 0x42, 0xa5, 0x11, 0x28}},
        {{0x86, 0xf3, 0x2b, 0x96, 0x42, 0x41}},
        {{0x88, 0xaa, 0x33, 0x22, 0x11, 0x11}},
        {{0x50, 0x33, 0x33, 0x22, 0x11, 0x11}},
        {{0x51, 0x33, 0x33, 0x22, 0x11, 0x11}},
        {{0x52, 0x33, 0x33, 0x22, 0x11, 0x11}},
        {{0x53, 0x33, 0x33, 0x22, 0x11, 0x11}},
    };

    DEBUG_INFO(7,"app_bt_search_callback found device %02x:%02x:%02x:%02x:%02x:%02x '%s'\n",
          bdaddr->address[0], bdaddr->address[1], bdaddr->address[2], bdaddr->address[3],
          bdaddr->address[4], bdaddr->address[5], name);

    count = sizeof(device_list) / sizeof(bt_bdaddr_t);

    for (i = 0; i < count; i += 1)
    {
        if (memcmp(bdaddr, device_list[i].address, sizeof(bt_bdaddr_t)) == 0)
        {
            for (j = 0; j < APP_BT_MAX_EXCEPT_DEVICE_NUM; j += 1)
            {
                if (g_except_device_list[j].inuse && memcmp(bdaddr, g_except_device_list[j].device.address, sizeof(bt_bdaddr_t)) == 0)
                {
                    DEBUG_INFO(7, "%s filtered %02x:%02x:%02x:%02x:%02x:%02x", __func__,
                          bdaddr->address[0], bdaddr->address[1], bdaddr->address[2],
                          bdaddr->address[3], bdaddr->address[4], bdaddr->address[5]);
                    return false;
                }
            }
            return true;
        }
    }

    return false;
}
#endif

#ifdef __SOURCE_TRACE_RX__
#define APP_BT_DEVICE_SEARCH_MAX_TIMES (1) /* 12.8s */
#else
#define APP_BT_DEVICE_SEARCH_MAX_TIMES (5) /* 12.8s * 5 = 64s */
#endif
static struct app_bt_search_t g_bt_search;
static void (*g_bt_device_searched_callback)(bt_bdaddr_t *remote);
static void (*g_bt_device_search_result_callback)(app_bt_search_result_t *result);
static inquiryResponseCallback_t g_bt_inquiry_response_callback = NULL;
#ifdef __SOURCE_TRACE_RX__
void set_bt_search_address(bt_bdaddr_t address)
{
    g_bt_search.address = address;
}

void set_bt_search_device_searched(bool device_searched)
{
    g_bt_search.device_searched = device_searched;
}
#endif

static void app_bt_search_callback(const btif_event_t* event)
{
    btif_event_type_t etype = btif_me_get_callback_event_type(event);
    DEBUG_INFO(2,"%s event %d\n", __func__, etype);

    switch(etype)
    {
        case BTIF_BTEVENT_INQUIRY_RESULT:
        {
            bt_bdaddr_t *addr = btif_me_get_callback_event_inq_result_bd_addr(event);
            uint8_t mode = btif_me_get_callback_event_inq_result_inq_mode(event);
            const int NAME_MAX_LEN = 255;
            char device_name[NAME_MAX_LEN+1] = {0};
            int device_name_len = 0;
            uint8_t *eir = NULL;

            if ((mode == BTIF_INQ_MODE_EXTENDED) &&
                (eir = btif_me_get_callback_event_inq_result_ext_inq_resp(event)))
            {
                device_name_len = btif_me_get_ext_inq_data(eir, 0x09, (uint8_t *)device_name, NAME_MAX_LEN);
            }

            if (g_bt_device_search_result_callback != NULL)
            {
                app_bt_search_result_t result;
                result.addr = addr;
                result.name = device_name;
                result.name_len = device_name_len;
                g_bt_device_search_result_callback(&result);
            }

#ifdef __SOURCE_TRACE_RX__
            device_list_add(addr, device_name, device_name_len);
#else
            if (app_bt_search_device_match(addr, device_name_len > 0 ? device_name : ""))
            {
                g_bt_search.address = *addr;
                g_bt_search.device_searched = true;
                btif_me_cancel_inquiry();
            }
#endif
        }
        break;
        case BTIF_BTEVENT_INQUIRY_COMPLETE:
        case BTIF_BTEVENT_INQUIRY_CANCELED:
            btif_me_unregister_globa_handler((btif_handler *)btif_me_get_bt_handler());
            g_bt_search.search_start = false;
            g_bt_search.inquiry_pending = false;

#ifdef __SOURCE_TRACE_RX__
            for(int i=0; i < DEVICE_NUMBER; i++)
            {
                if(is_bt_bdaddr_t_null(&device_list[i].addr))
                {
                    continue;
                }

                DEBUG_INFO(7,"[%d] [%02x:%02x:%02x:%02x:%02x:%02x] [%s]",
                      i,
                      device_list[i].addr.address[0], device_list[i].addr.address[1], device_list[i].addr.address[2], device_list[i].addr.address[3],
                      device_list[i].addr.address[4], device_list[i].addr.address[5],
                      device_list[i].name);
            }
#endif
            if (g_bt_search.device_searched)
            {
                if (g_bt_device_searched_callback)
                {
                    g_bt_device_searched_callback(&g_bt_search.address);
                }
            }
            else
            {
                g_bt_search.search_times += 1;
                if (g_bt_search.search_times == APP_BT_DEVICE_SEARCH_MAX_TIMES)
                {
                    DEBUG_INFO(1,"%s end with no device matched\n", __func__);
                    g_bt_search.search_times = 0;
                    if (g_bt_device_searched_callback)
                    {
                        g_bt_device_searched_callback(NULL);
                    }
                }
                else
                {
                    DEBUG_INFO(1,"%s no device matched, continue to search...\n", __func__);
                    app_bt_start_search_with_callback(g_bt_device_searched_callback, g_bt_device_search_result_callback);
                }
            }
            break;
        default:
            break;
    }
}

static void app_bt_search_default_callback(bt_bdaddr_t *remote)
{
    if (remote == NULL)
    {
        DEBUG_INFO(0, "app_bt_search_default_callback no device matched");
        return;
    }

    DEBUG_INFO(6,"app_bt_search_default_callback matched %02x:%02x:%02x:%02x:%02x:%02x\n",
          remote->address[0], remote->address[1], remote->address[2],
          remote->address[3], remote->address[4], remote->address[5]);

#if defined(BT_HFP_AG_ROLE)
    app_bt_reconnect_hfp_profile(remote);
#endif
}

void app_bt_start_search_with_callback(void (*cb)(bt_bdaddr_t *remote), void (*result_cb)(app_bt_search_result_t *result))
{
    uint8_t max_search_time = 10; /* 12.8s */
#if (BT_SOURCE_DEVICE_NUM < 2)
    if (g_bt_search.search_start)
    {
        DEBUG_INFO(1,"%s already started\n", __func__);
        return;
    }
#endif
    g_bt_device_searched_callback = cb;
    g_bt_device_search_result_callback = result_cb;

#ifdef __SOURCE_TRACE_RX__
    device_list_init();
#endif

    void *bt_handler = btif_me_get_bt_handler();
    btif_me_set_handler(bt_handler, app_bt_search_callback);

    btif_me_set_event_mask(bt_handler,
                           BTIF_BEM_INQUIRY_RESULT | BTIF_BEM_INQUIRY_COMPLETE | BTIF_BEM_INQUIRY_CANCELED |
                           BTIF_BEM_LINK_CONNECT_IND | BTIF_BEM_LINK_CONNECT_CNF | BTIF_BEM_LINK_DISCONNECT |
                           BTIF_BEM_ROLE_CHANGE | BTIF_BEM_MODE_CHANGE);

    btif_me_register_global_handler(bt_handler);

    g_bt_search.search_start = true;
    g_bt_search.device_searched = false;
    g_bt_search.inquiry_pending = false;

    if (BT_STS_PENDING != btif_me_inquiry(BTIF_BT_IAC_GIAC, max_search_time, 0))
    {
        DEBUG_INFO(1,"%s start inquiry failed\n", __func__);
        g_bt_search.inquiry_pending = true;
    }
}

void app_bt_start_search(void)
{
    app_bt_start_search_with_callback(app_bt_search_default_callback, NULL);
}

#ifdef BT_AVRCP_SUPPORT

uint8_t app_bt_avrcp_get_volume_change_trans_id(uint8_t device_id)
{
    return btif_avrcp_get_volume_change_trans_id(app_bt_get_device(device_id)->avrcp_channel);
}

void app_bt_avrcp_set_volume_change_trans_id(uint8_t device_id, uint8_t trans_id)
{
    DEBUG_INFO(3,"%s device_id %x trans_id %d\n", __func__, device_id, trans_id);
    btif_a2dp_set_volume_change_trans_id(app_bt_get_device(device_id)->avrcp_channel, trans_id);
}

uint8_t app_bt_avrcp_get_ctl_trans_id(uint8_t device_id)
{
    return btif_avrcp_get_ctl_trans_id(app_bt_get_device(device_id)->avrcp_channel);
}

void app_bt_avrcp_set_ctl_trans_id(uint8_t device_id, uint8_t trans_id)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);

    DEBUG_INFO(3,"%s %d %p\n", __func__, trans_id, curr_device->avrcp_channel);
    btif_avrcp_set_ctl_trans_id(curr_device->avrcp_channel, trans_id);
}

#endif /* BT_AVRCP_SUPPORT */


static void app_bt_inquiry_event_callback(const btif_event_t* event)
{
    btif_event_type_t etype = btif_me_get_callback_event_type(event);
    //DEBUG_INFO(2,"%s event %d\n", __func__, etype);

    switch(etype)
    {
        case BTIF_BTEVENT_INQUIRY_RESULT:
        {
            bt_bdaddr_t POSSIBLY_UNUSED *addr = btif_me_get_callback_event_inq_result_bd_addr(event);
            uint8_t mode = btif_me_get_callback_event_inq_result_inq_mode(event);
            const int NAME_MAX_LEN = 255;
            char device_name[NAME_MAX_LEN+1] = {0};
            int POSSIBLY_UNUSED device_name_len = 0;
            uint8_t *eir = NULL;
            int8_t POSSIBLY_UNUSED rssi = BTIF_INQ_INVALID_RSSI;

            if ((mode == BTIF_INQ_MODE_EXTENDED) &&
                (eir = btif_me_get_callback_event_inq_result_ext_inq_resp(event)))
            {
                device_name_len = btif_me_get_ext_inq_data(eir, 0x09, (uint8_t *)device_name, NAME_MAX_LEN);
            }

            if (mode == BTIF_INQ_MODE_RSSI || mode == BTIF_INQ_MODE_EXTENDED)
            {
                rssi = (int8_t)btif_me_get_callback_event_rssi(event);
            }

            if (g_bt_inquiry_response_callback)
            {
                g_bt_inquiry_response_callback((uint8_t *)addr, (uint8_t *)device_name, eir);
            }

            DEBUG_INFO(1, "device rssi: %d", rssi);
            DEBUG_INFO(1, "device addr is: ");
            DUMP8("0x%0x ", addr, BT_ADDR_OUTPUT_PRINT_NUM);
        }
        break;
        case BTIF_BTEVENT_INQUIRY_COMPLETE:
        case BTIF_BTEVENT_INQUIRY_CANCELED:
            break;
        default:
            break;
    }
}

void app_bt_register_inquiry_response_callback(inquiryResponseCallback_t callback)
{
    g_bt_inquiry_response_callback = callback;
}

void app_bt_start_inquiry(uint8_t inquiryLastingTime)
{
    if (0 == inquiryLastingTime)
    {
        DEBUG_INFO(1,"%s start inquiry failed, inquiry lasting time is 0\n", __func__);
        return;
    }

    if (0x30 < inquiryLastingTime)
    {
        inquiryLastingTime = 0x30;
    }

    void *bt_handler = btif_me_get_bt_handler();

    btif_me_set_handler(bt_handler, app_bt_inquiry_event_callback);

    btif_me_set_event_mask(bt_handler,
                           BTIF_BEM_INQUIRY_RESULT | BTIF_BEM_INQUIRY_COMPLETE | BTIF_BEM_INQUIRY_CANCELED );

    btif_me_register_global_handler(bt_handler);

    if (BT_STS_PENDING != btif_me_inquiry(BTIF_BT_IAC_GIAC, inquiryLastingTime, 0))
    {
        DEBUG_INFO(1,"%s start inquiry failed\n", __func__);
    }
}

void app_bt_stop_inquiry(void)
{
    g_bt_search.device_searched = true;
    btif_me_cancel_inquiry();
}

bt_status_t bt_adapter_init(bt_adapter_callback_t callback)
{
    if (callback)
    {
        bt_add_event_callback((bt_event_callback_t)callback, BT_EVENT_MASK_LINK_GROUP);
    }
    return BT_STS_SUCCESS;
}

bt_status_t bt_adapter_cleanup(void)
{
    return BT_STS_SUCCESS;
}

bt_status_t bt_adapter_start_inquiry(void)
{
    app_bt_start_search();
    return BT_STS_SUCCESS;
}

bt_status_t bt_adapter_cancel_inquiry(void)
{
    app_bt_stop_inquiry();
    return BT_STS_SUCCESS;
}

bt_status_t bt_adapter_disconnect_acl(const bt_bdaddr_t *bd_addr)
{
    return btif_cmgr_remove_data_link(bd_addr);
}

bt_status_t bt_adapter_create_bond(const bt_bdaddr_t *bd_addr)
{
    if (!app_bt_get_connected_device_byaddr(bd_addr))
    {
        btif_create_acl_to_slave(bd_addr);
    }
    return BT_STS_SUCCESS;
}

bt_status_t bt_adapter_remove_bond(const bt_bdaddr_t *bd_addr)
{
    if (app_bt_get_connected_device_byaddr(bd_addr))
    {
        bt_adapter_disconnect_acl(bd_addr);
    }
    bluetooth_nv_mgr_bt_record_del(BT_NV_REC_DEL_REMOVE_BOND, bd_addr->address);
    bt_adapter_bond_state_change_param_t param;
    param.is_bonded = false;
    btif_report_bt_event(bd_addr, BT_EVENT_BOND_STATE_CHANGE, &param);
    return BT_STS_SUCCESS;
}

bt_status_t bt_adapter_set_property(const bt_adapter_property_param_t *property)
{
    if (property->type == BT_PROPERTY_TYPE_LOCAL_BDNAME)
    {
        btif_update_bt_name((unsigned char *)property->data, (uint8_t)property->len);
        return BT_STS_SUCCESS;
    }
    if (property->type == BT_PROPERTY_TYPE_LOCAL_BDADDR)
    {
        btif_me_set_bt_address((uint8_t *)property->data);
        return BT_STS_SUCCESS;
    }
    if (property->type == BT_PROPERTY_TYPE_ACCESS_MODE)
    {
#if !defined(IBRT)
        app_bt_set_access_mode((btif_accessible_mode_t)property->data);
#endif
        return BT_STS_SUCCESS;
    }
    DEBUG_INFO(0, "bt_adapter_set_property: unsupported type %d", property->type);
    return BT_STS_FAILED;
}

bt_status_t bt_adapter_get_property(const bt_bdaddr_t *bd_addr, bt_property_data_type_t type)
{
    if (type == BT_PROPERTY_TYPE_REMOTE_NAME)
    {
        btif_me_get_remote_device_name(bd_addr, NULL);
        return BT_STS_SUCCESS;
    }
    DEBUG_INFO(0, "bt_adapter_get_property: unsupported type %d", type);
    return BT_STS_FAILED;
}

/**
 ****************************************************************************************
 * @brief bt set channel classification map which related "Set AFH Host Channel Classification command,vore spec,vol4,7.3.46,0x0C3F"
 * AFH_Host_Channel_Classification: Size: 10 octets (79 bits meaningful)
 * This parameter contains 80 1-bit fields
 * The nth such field (in the range 0 to 78) contains the value for channel n:
 *    0: channel n is bad
 *    1: channel n is unknown
 *
 * The most significant bit (bit 79) is reserved for future use
 * At least (Nmin == 20) channels shall be marked as unknown.
 * default all bits value is 1
 * example: if you mask channel 0-19,the chal_map value is:
 *   {0x00,0x00,0xF0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}
 *
 ****************************************************************************************
 */
void app_bt_set_chnl_classification(uint8_t *chnl_map)
{
    app_bt_start_custom_function_in_bt_thread((uint32_t)chnl_map, (uint32_t)NULL, (uint32_t)btif_me_set_afh_chnl_classification);
}

#if defined(IBRT)
uint32_t app_bt_save_bd_addr_ctx(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len)
{
    memcpy(buf, remote, BTIF_BD_ADDR_SIZE);
    return BTIF_BD_ADDR_SIZE;
}

uint32_t app_bt_restore_bd_addr_ctx(bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len)
{
    memcpy(bdaddr_p,buf, BTIF_BD_ADDR_SIZE);
    return BTIF_BD_ADDR_SIZE;
}

uint32_t app_bt_save_spp_app_ctx(uint64_t app_id, const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len)
{
    return 0;
}

uint32_t app_bt_restore_spp_app_ctx(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len, uint64_t app_id)
{
    return 0;
}

uint32_t app_bt_save_hfp_app_ctx(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len)
{
    uint8_t device_id = app_bt_get_connected_device_id_byaddr((bt_bdaddr_t *)remote);
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);

    BTIF_CTX_INIT(buf);

    BTIF_CTX_STR_BUF(remote, BTIF_BD_ADDR_SIZE);

    BTIF_CTX_STR_VAL8(curr_device->hfchan_call);
    BTIF_CTX_STR_VAL8(curr_device->hfchan_callSetup);
    BTIF_CTX_STR_VAL8(curr_device->hf_callheld);

    BTIF_CTX_STR_VAL8(curr_device->profile_mgr.remote_support_a2dp);
    BTIF_CTX_STR_VAL8(curr_device->profile_mgr.remote_support_hfp);
    BTIF_CTX_STR_VAL8(curr_device->profile_mgr.a2dp_connect);
    BTIF_CTX_STR_VAL8(curr_device->profile_mgr.hfp_connect);
    BTIF_CTX_STR_VAL8(curr_device->profile_mgr.profile_connected);

    BTIF_CTX_SAVE_UPDATE_DATA_LEN();
    return BTIF_CTX_GET_TOTAL_LEN();
}

uint32_t app_bt_restore_hfp_app_ctx(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len)
{
    bt_bdaddr_t remote;
    struct BT_DEVICE_T* curr_device = NULL;
    uint8_t call, callsetup, callheld;
    uint8_t remote_support_a2dp;
    uint8_t remote_support_hfp;
    uint8_t a2dp_connect, hfp_connect, profile_connected;
    uint8_t device_id = 0;
    BTIF_CTX_INIT(buf);

    BTIF_CTX_LDR_BUF(&remote, BTIF_BD_ADDR_SIZE);

    BTIF_CTX_LDR_VAL8(call);
    BTIF_CTX_LDR_VAL8(callsetup);
    BTIF_CTX_LDR_VAL8(callheld);

    BTIF_CTX_LDR_VAL8(remote_support_a2dp);
    BTIF_CTX_LDR_VAL8(remote_support_hfp);
    BTIF_CTX_LDR_VAL8(a2dp_connect);
    BTIF_CTX_LDR_VAL8(hfp_connect);
    BTIF_CTX_LDR_VAL8(profile_connected);

    if (bdaddr_p)
    {
        device_id = btif_me_get_device_id_from_addr((bt_bdaddr_t *)bdaddr_p);
    }
    else
    {
        device_id = BT_DEVICE_ID_1;
    }

    curr_device = app_bt_get_device(device_id);

    curr_device->hfchan_call = call;
    curr_device->hfchan_callSetup = callsetup;
    curr_device->hf_callheld = callheld;

    curr_device->profile_mgr.remote_support_a2dp = remote_support_a2dp;
    curr_device->profile_mgr.remote_support_hfp = remote_support_hfp;
    curr_device->profile_mgr.a2dp_connect = (bt_profile_connect_status)a2dp_connect;
    curr_device->profile_mgr.hfp_connect = (bt_profile_connect_status)hfp_connect;
    curr_device->profile_mgr.profile_connected = profile_connected;
    curr_device->profile_mgr.reconnect_mode = bt_profile_reconnect_null;
    curr_device->profile_mgr.rmt_addr = curr_device->remote;
    curr_device->hfp_status_recheck = HFP_STATUS_ALL_RECHECK;
    DEBUG_INFO(4,"%s call %d callsetup %d callheld %d", __func__, call, callsetup, callheld);

    return BTIF_CTX_GET_TOTAL_LEN();
}

#ifdef BT_A2DP_SUPPORT
uint32_t app_bt_save_a2dp_app_ctx(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len)
{
    uint32_t offset = 0;
    uint32_t factor = 0;
    uint8_t bg_a2dp_device = BT_DEVICE_INVALID_ID;
    uint8_t device_id = app_bt_get_connected_device_id_byaddr((bt_bdaddr_t *)remote);
    struct BT_DEVICE_T* curr_device = NULL;

    DEBUG_INFO(1,"save_a2dp_app_ctx device id=%d",device_id);

    curr_device = app_bt_get_device(device_id);

    memcpy(buf+offset, &curr_device->remote, BTIF_BD_ADDR_SIZE);
    offset += BTIF_BD_ADDR_SIZE;

    buf[offset++] = curr_device->a2dp_conn_flag;
    buf[offset++] = curr_device->a2dp_play_pause_flag;
    buf[offset++] = curr_device->avrcp_playback_status;
    bg_a2dp_device = app_bt_audio_select_bg_a2dp_to_resume();
    if (bg_a2dp_device != BT_DEVICE_INVALID_ID && bg_a2dp_device == device_id)
    {
        struct BT_DEVICE_T* bg_device = app_bt_get_device(bg_a2dp_device);
        buf[offset++] = true;
        buf[offset++] = bg_device->this_is_bg_a2dp;
        memcpy(buf+offset, &bg_device->remote, BTIF_BD_ADDR_SIZE);
        offset += BTIF_BD_ADDR_SIZE;
    }
    else
    {
        buf[offset++] = false;
    }

    buf[offset++] = curr_device->profile_mgr.remote_support_a2dp;
    buf[offset++] = curr_device->profile_mgr.remote_support_hfp;
    buf[offset++] = curr_device->profile_mgr.a2dp_connect;
    buf[offset++] = curr_device->profile_mgr.hfp_connect;
    buf[offset++] = curr_device->profile_mgr.profile_connected;

    //codec
    buf[offset++] = curr_device->codec_type;

    buf[offset++] = curr_device->sample_rate & 0xFF;
    buf[offset++] = (curr_device->sample_rate >> 8) & 0xFF;
    buf[offset++] = (curr_device->sample_rate >> 16) & 0xFF;
    buf[offset++] = (curr_device->sample_rate >> 24) & 0xFF;

    buf[offset++] = curr_device->l2hc_bitrate & 0xFF;
    buf[offset++] = (curr_device->l2hc_bitrate >> 8) & 0xFF;

    buf[offset++] = curr_device->sample_bit;
    buf[offset++] = curr_device->a2dp_frame_dr;
#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)
    buf[offset++] = curr_device->a2dp_lhdc_llc;
#endif
#if defined(__A2DP_AVDTP_CP__)
    buf[offset++] = curr_device->avdtp_cp;
#endif

    //volume
    buf[offset++] = curr_device->a2dp_current_abs_volume;

    //latency factor
#ifndef BTH_SUBSYS_ONLY
    factor =  (uint32_t)a2dp_audio_latency_factor_get();
#endif
    buf[offset++] = factor & 0xFF;
    buf[offset++] = (factor >> 8) & 0xFF;
    buf[offset++] = (factor >> 16) & 0xFF;
    buf[offset++] = (factor >> 24) & 0xFF;

    //a2dp session
    buf[offset++] = a2dp_ibrt_session_get(device_id) & 0xFF;
    buf[offset++] = (a2dp_ibrt_session_get(device_id) >> 8) & 0xFF;
    buf[offset++] = (a2dp_ibrt_session_get(device_id) >> 16) & 0xFF;
    buf[offset++] = (a2dp_ibrt_session_get(device_id) >> 24) & 0xFF;

    return offset;
}

uint32_t app_bt_restore_a2dp_app_ctx(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len)
{
    uint32_t offset = 0;
    bt_bdaddr_t remote = {0};
    bt_bdaddr_t bg_a2dp_bdaddr = {0};
    uint8_t device_id = 0;
    struct BT_DEVICE_T* curr_device = NULL;

    if (bdaddr_p)
    {
        device_id = btif_me_get_device_id_from_addr((bt_bdaddr_t *)bdaddr_p);
    }
    else
    {
        device_id = BT_DEVICE_ID_1;
    }

    curr_device = app_bt_get_device(device_id);

    DEBUG_INFO(1,"restore_a2dp_app_ctx device id=%d",device_id);

    memcpy(&remote,buf+offset,BTIF_BD_ADDR_SIZE);
    offset += BTIF_BD_ADDR_SIZE;

    curr_device->a2dp_conn_flag  = buf[offset++];
    curr_device->a2dp_play_pause_flag = buf[offset++];
    curr_device->avrcp_playback_status = buf[offset++];

    if (buf[offset++])
    {
        bool is_bg_a2dp = buf[offset++];
        memcpy(&bg_a2dp_bdaddr, buf+offset, BTIF_BD_ADDR_SIZE);
        offset += BTIF_BD_ADDR_SIZE;
        curr_device->this_is_bg_a2dp = is_bg_a2dp;
    }

    curr_device->profile_mgr.remote_support_a2dp = buf[offset++];
    curr_device->profile_mgr.remote_support_hfp = buf[offset++];
    curr_device->profile_mgr.a2dp_connect = (bt_profile_connect_status)buf[offset++];
    curr_device->profile_mgr.hfp_connect = (bt_profile_connect_status)buf[offset++];
    curr_device->profile_mgr.profile_connected = buf[offset++];
    curr_device->profile_mgr.reconnect_mode = bt_profile_reconnect_null;

    //codec type
    curr_device->codec_type = buf[offset++];
    curr_device->sample_rate = buf[offset] + (buf[offset+1] << 8) + (buf[offset+2] << 16) + (buf[offset+3] << 24);
    offset += 4;
    curr_device->l2hc_bitrate = buf[offset] + (buf[offset+1] << 8);
    offset += 2;
    curr_device->sample_bit = buf[offset++];
    curr_device->a2dp_frame_dr = buf[offset++];
#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)
    curr_device->a2dp_lhdc_llc = buf[offset++];
#endif
#if defined(__A2DP_AVDTP_CP__)
    curr_device->avdtp_cp = buf[offset++];
#endif
    curr_device->a2dp_status_recheck = A2DP_STATUS_ALL_RECHECK;
    //volume
    app_bt_a2dp_current_abs_volume_just_set(device_id, buf[offset++]);

    //latency factor
#ifndef BTH_SUBSYS_ONLY
    a2dp_audio_latency_factor_set((float)(buf[offset] + (buf[offset+1] << 8) + (buf[offset+2] << 16) + (buf[offset+3] << 24)));
    offset += 4;
#endif

    //a2dp session
    a2dp_ibrt_session_set((buf[offset] + (buf[offset+1] << 8) + (buf[offset+2] << 16) + (buf[offset+3] << 24)),device_id);
    offset += 4;

    curr_device->a2dp_connected_stream = curr_device->btif_a2dp_stream->a2dp_stream;
    memcpy(curr_device->profile_mgr.rmt_addr.address, &remote, BTIF_BD_ADDR_SIZE);

    return offset;
}
#endif

uint32_t app_bt_save_avrcp_app_ctx(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len)
{
    uint32_t offset = 0;
    uint8_t device_id = app_bt_get_connected_device_id_byaddr((bt_bdaddr_t *)remote);
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);

    buf[offset++] = curr_device->avrcp_remote_support_playback_status_change_event;
    buf[offset++] = curr_device->avrcp_conn_flag;
    buf[offset++] = curr_device->volume_report;
    buf[offset++] = app_bt_avrcp_get_volume_change_trans_id(device_id);
    buf[offset++] = app_bt_manager.config.a2dp_default_abs_volume;

    return offset;
}

uint32_t app_bt_restore_avrcp_app_ctx(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len)
{
    uint32_t offset = 0;
    uint8_t trans_id = 0;
    uint8_t device_id = 0;
    uint8_t a2dp_default_abs_volume = 0;
    struct BT_DEVICE_T* curr_device = NULL;

    if (bdaddr_p)
    {
        device_id = btif_me_get_device_id_from_addr((bt_bdaddr_t *)bdaddr_p);
    }
    else
    {
        device_id = BT_DEVICE_ID_1;
    }

    curr_device = app_bt_get_device(device_id);

    curr_device->avrcp_remote_support_playback_status_change_event = buf[offset++];
    curr_device->avrcp_conn_flag = buf[offset++];
    curr_device->volume_report = buf[offset++];
    trans_id = buf[offset++];
    a2dp_default_abs_volume = buf[offset++];

    DEBUG_INFO(5,"app_bt_restore_avrcp_app_ctx state %d report %d trans_id %d default abs %d playback %d",
          curr_device->avrcp_conn_flag, curr_device->volume_report, trans_id, a2dp_default_abs_volume,
          curr_device->avrcp_remote_support_playback_status_change_event);

    app_bt_avrcp_set_volume_change_trans_id(device_id, trans_id);

    app_bt_manager.config.a2dp_default_abs_volume = a2dp_default_abs_volume;
    curr_device->a2dp_default_abs_volume = a2dp_default_abs_volume;

    return offset;
}

#ifdef BT_MAP_SUPPORT
uint32_t app_bt_save_map_app_ctx(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len)
{
    return 0;
}

uint32_t app_bt_restore_map_app_ctx(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len)
{
    return 0;
}
#endif

#ifdef BT_HID_DEVICE
uint32_t app_bt_save_hid_app_ctx(btif_remote_device_t *rem_dev, uint8_t *buf, uint32_t buf_len)
{
    uint32_t offset = 0;
    uint8_t device_id = btif_me_get_device_id_from_rdev(rem_dev);
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);

    if (curr_device == NULL || curr_device->hid_channel == NULL)
    {
        DEBUG_INFO(0, "(d%x) app_bt_save_hid_app_ctx: invalid device", device_id);
        return offset;
    }

    return offset;
}

uint32_t app_bt_restore_hid_app_ctx(bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len)
{
    uint32_t offset = 0;
    uint8_t device_id = btif_me_get_device_id_from_addr(bdaddr_p);
    struct BT_DEVICE_T* curr_device = NULL;

    curr_device = app_bt_get_device(device_id);
    if (curr_device == NULL || curr_device->hid_channel == NULL)
    {
        DEBUG_INFO(0, "(d%x) app_bt_save_hid_app_ctx: invalid device", device_id);
        return offset;
    }

    curr_device->hid_conn_flag = true;

    return offset;
}
#endif

void app_bt_ibrt_reconnect_mobile_profile_flag_init(void)
{
    for (uint8_t i = 0;i<BT_DEVICE_NUM;i++)
    {
        ibrt_reconnect_mobile_profile_flag[i] = false;
    }
}

void app_bt_ibrt_reconnect_mobile_profile_flag_set(bt_bdaddr_t *remote_addr)
{
    uint8_t device_id = btif_me_get_device_id_from_addr(remote_addr);
    if (device_id < BT_DEVICE_NUM)
    {
        ibrt_reconnect_mobile_profile_flag[device_id] = true;
    }
}

void app_bt_ibrt_reconnect_mobile_profile_flag_clear(bt_bdaddr_t *remote_addr)
{
    uint8_t device_id = btif_me_get_device_id_from_addr(remote_addr);
    if (device_id < BT_DEVICE_NUM)
    {
        ibrt_reconnect_mobile_profile_flag[device_id] = false;
    }
}

bool app_bt_ibrt_reconnect_mobile_profile_flag_get(bt_bdaddr_t *remote_addr)
{
    uint8_t device_id = btif_me_get_device_id_from_addr(remote_addr);
    if (device_id < BT_DEVICE_NUM)
    {
        return ibrt_reconnect_mobile_profile_flag[device_id];
    }
    return false;
}

void app_bt_ibrt_reconnect_mobile_profile(const bt_bdaddr_t *mobile_addr)
{
    bool profile_concurrency_supported = true;
#if defined(IBRT) && defined(IBRT_UI)
    ibrt_core_param_t *p_ibrt_core_config = app_tws_ibrt_get_custom_config();
    profile_concurrency_supported = p_ibrt_core_config->profile_concurrency_supported;
#endif

    DEBUG_INFO(0,"start recon mobile, addr:%02x:%02x:*:*:*:%02x",
        mobile_addr->address[0], mobile_addr->address[1], mobile_addr->address[5]);
    app_bt_ibrt_reconnect_mobile_profile_flag_set((bt_bdaddr_t *)mobile_addr);
    app_bt_precheck_before_starting_connecting(false);

    if (profile_concurrency_supported)
    {
        app_bt_reconnect_a2dp_profile(mobile_addr);
        app_bt_reconnect_hfp_profile(mobile_addr);
#if defined(__GATT_OVER_BR_EDR__)
        uint8_t device_id = btif_me_get_device_id_from_addr(mobile_addr);
        if (app_bt_check_is_ios_device(mobile_addr) && !app_btgatt_is_connected(device_id))
        {
            app_bt_reconnect_gatt_profile(mobile_addr);
        }
#endif
    }
    else
    {
#ifdef BT_A2DP_SUPPORT
        app_bt_reconnect_a2dp_profile((bt_bdaddr_t *)mobile_addr);
#else
        app_bt_reconnect_hfp_profile((bt_bdaddr_t *)mobile_addr);
#endif
    }
}

void app_bt_ibrt_connect_mobile_a2dp_profile(const bt_bdaddr_t *addr)
{
    app_bt_reconnect_a2dp_profile((bt_bdaddr_t *)addr);
}

void app_bt_ibrt_connect_mobile_hfp_profile(const bt_bdaddr_t *addr)
{
    app_bt_reconnect_hfp_profile((bt_bdaddr_t *)addr);
}

#endif

#if defined(IBRT) && defined(IBRT_UI)
//IBRT v2 module has its own sniff manager
void app_bt_active_mode_manager_init(void)
{
}

void app_bt_active_mode_reset(uint32_t link_id)
{
}

void app_bt_active_mode_set(BT_ACTIVE_MODE_KEEP_USER_T user, uint32_t link_id)
{
}

void app_bt_active_mode_clear(BT_ACTIVE_MODE_KEEP_USER_T user, uint32_t link_id)
{
}
#else
static uint32_t bt_link_active_mode_bits[MAX_ACTIVE_MODE_MANAGED_LINKS];

void app_bt_active_mode_manager_init(void)
{
    memset(bt_link_active_mode_bits, 0, sizeof(bt_link_active_mode_bits));
}

void app_bt_active_mode_reset(uint32_t link_id)
{
    bt_link_active_mode_bits[link_id] = 0;
}

void app_bt_active_mode_set(BT_ACTIVE_MODE_KEEP_USER_T user, uint32_t link_id)
{
    bool isAlreadyInActiveMode = false;
    if (link_id < MAX_ACTIVE_MODE_MANAGED_LINKS)
    {
        uint32_t lock = int_lock();
        if (bt_link_active_mode_bits[link_id] > 0)
        {
            isAlreadyInActiveMode = true;
        }
        else
        {
            isAlreadyInActiveMode = false;
        }
        bt_link_active_mode_bits[link_id] |= (1 << user);
        int_unlock(lock);

        if (!isAlreadyInActiveMode)
        {
            app_bt_stop_sniff(link_id);
            app_bt_stay_active(link_id);
        }

    }
    else if (MAX_ACTIVE_MODE_MANAGED_LINKS == link_id)
    {
        for (uint8_t devId = 0; devId < BT_DEVICE_NUM; devId++)
        {
            uint32_t lock = int_lock();
            if (bt_link_active_mode_bits[devId] > 0)
            {
                isAlreadyInActiveMode = true;
            }
            else
            {
                isAlreadyInActiveMode = false;
            }
            bt_link_active_mode_bits[devId] |= (1 << user);
            int_unlock(lock);

            if (!isAlreadyInActiveMode)
            {
                app_bt_stop_sniff(devId);
                app_bt_stay_active(devId);
            }
        }
    }

    DEBUG_INFO(2,"set active mode for user %d, link %d, now state:", user, link_id);
    DUMP32("%08x", bt_link_active_mode_bits, MAX_ACTIVE_MODE_MANAGED_LINKS);
}

void app_bt_active_mode_clear(BT_ACTIVE_MODE_KEEP_USER_T user, uint32_t link_id)
{
    bool isAlreadyAllowSniff = false;
    if (link_id < MAX_ACTIVE_MODE_MANAGED_LINKS)
    {
        uint32_t lock = int_lock();

        if (0 == bt_link_active_mode_bits[link_id])
        {
            isAlreadyAllowSniff = true;
        }
        else
        {
            isAlreadyAllowSniff = false;
        }

        bt_link_active_mode_bits[link_id] &= (~(1 << user));

        int_unlock(lock);

        if (!isAlreadyAllowSniff)
        {
            app_bt_allow_sniff(link_id);
        }
    }
    else if (MAX_ACTIVE_MODE_MANAGED_LINKS == link_id)
    {
        for (uint8_t devId = 0; devId < BT_DEVICE_NUM; devId++)
        {
            uint32_t lock = int_lock();
            if (0 == bt_link_active_mode_bits[devId])
            {
                isAlreadyAllowSniff = true;
            }
            else
            {
                isAlreadyAllowSniff = false;
            }
            bt_link_active_mode_bits[devId] &= (~(1 << user));
            int_unlock(lock);

            if (!isAlreadyAllowSniff)
            {
                app_bt_allow_sniff(devId);
            }
        }
    }

    DEBUG_INFO(2,"clear active mode for user %d, link %d, now state:", user, link_id);
    DUMP32("%08x ", bt_link_active_mode_bits, MAX_ACTIVE_MODE_MANAGED_LINKS);
}
#endif

void app_bt_set_keep_active_mode(bool set_keep, BT_ACTIVE_MODE_KEEP_USER_T user, uint32_t link_id)
{
    if (set_keep)
    {
        app_bt_active_mode_set(user, link_id);
    }
    else
    {
        app_bt_active_mode_clear(user, link_id);
    }
}

int8_t app_bt_get_rssi(void)
{
    bool ret = false;
    uint8_t i;
    btif_remote_device_t *remDev = NULL;
    rx_agc_t agc_struct = {0};

    for (i=0; i<BT_DEVICE_NUM; i++)
    {
        remDev = btif_me_enumerate_remote_devices(i);
        if (remDev)
        {
            if(btif_me_get_remote_device_hci_handle(remDev))
            {
                ret = bt_drv_reg_op_read_rssi_in_dbm(btif_me_get_remote_device_hci_handle(remDev),&agc_struct);
                if(ret)
                {
                    DEBUG_INFO(1," headset to mobile RSSI:%d dBm, rx gain=%d",agc_struct.rssi, agc_struct.rxgain);
                }
            }
        }
    }
    return agc_struct.rssi;
}

#ifdef TILE_DATAPATH
int8_t app_tile_get_ble_rssi(void)
{
    int8_t rssi=127;
    uint8_t i;
    btif_remote_device_t *remDev = NULL;
    rx_agc_t tws_agc = {0};

    for (i=0; i<BT_DEVICE_NUM; i++)
    {
        remDev = btif_me_enumerate_remote_devices(i);
        if (remDev)
        {
            if(app_tile_ble_get_connection_index() != BLE_INVALID_CONNECTION_INDEX)
            {
                rssi = bt_drv_reg_op_read_ble_rssi_in_dbm(app_tile_ble_get_connection_index(),&tws_agc);
                rssi = bt_drv_reg_op_rssi_correction(tws_agc.rssi);
                //TRACE(1," headset to mobile RSSI:%d dBm",rssi);
            }
        }
    }
    return rssi;
}
#endif

#ifdef BT_USB_AUDIO_DUAL_MODE
#include "a2dp_api.h"
extern "C" a2dp_stream_t* app_bt_get_steam(int id)
{
    a2dp_stream_t* stream;

    stream = (a2dp_stream_t*)app_bt_get_device(id)->profile_mgr.stream;
    return stream;
}

extern "C" bool app_bt_get_bt_addr(int id,bt_bdaddr_t *bdaddr)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(id);
    if (curr_device->acl_is_connected)
    {
        memcpy(bdaddr->address, curr_device->remote.address, sizeof(bt_bdaddr_t));
        return true;
    }
    else
    {
        return false;
    }
}
#endif

bool app_bt_a2dp_service_is_connected(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    return (curr_device->profile_mgr.a2dp_connect == bt_profile_connect_status_success &&
            curr_device->profile_mgr.remote_support_a2dp == true);
}

bool app_bt_is_a2dp_streaming_exist(void)
{
    struct BT_DEVICE_T* curr_device = NULL;
    for (int i = 0; i < BT_DEVICE_NUM; i++)
    {
        curr_device = app_bt_get_device(i);
        if(curr_device->a2dp_streamming)
        {
            return true;
        }
    }
    return false;
}

bool app_bt_is_sco_connected_exist(void)
{
    struct BT_DEVICE_T *curr_device;
    for (int i = 0; i < BT_DEVICE_NUM; i++)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->acl_is_connected &&
            curr_device->hf_audio_state == BT_HFP_AUDIO_CON)
        {
            return true;
        }
    }
    return false;
}


bool app_bt_is_bt_active_mode(void)
{
    return false;//(app_bt_is_a2dp_streaming_exist()|| app_bt_is_sco_connected_exist());
}

#ifdef CTKD_ENABLE
static bool isCtkdConnectingMobilePending = false;
POSSIBLY_UNUSED static void app_bt_ctkd_set_connecting_mobile_pending(bool isPending)
{
    isCtkdConnectingMobilePending = isPending;
}

bool app_bt_ctkd_is_connecting_mobile_pending(void)
{
    return isCtkdConnectingMobilePending;
}

void app_bt_ctkd_connecting_mobile_handler(void)
{
    DEBUG_INFO(0, "warning!!! app_bt_ctkd_connecting_mobile_handler is not used in ibrt_ui");
    return;
}
#endif

void app_bt_get_remote_cod(uint8_t *cod0, uint8_t *cod1)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(BT_DEVICE_ID_1);
    nvrec_btdevicerecord *record = NULL;

    if (!nv_record_btdevicerecord_find(&curr_device->remote, &record)) {
        memcpy(cod0, record->record.cod, 3);
    }
#if BT_DEVICE_NUM > 1
    curr_device = app_bt_get_device(BT_DEVICE_ID_2);
    if (!nv_record_btdevicerecord_find(&curr_device->remote, &record)) {
        memcpy(cod1, record->record.cod, 3);
    }
#endif
}

bool app_bt_get_remote_cod_by_addr(const bt_bdaddr_t *bd_ddr, uint8_t *cod)
{
    nvrec_btdevicerecord *record = NULL;
    if (!nv_record_btdevicerecord_find(bd_ddr, &record)) {
        memcpy(cod, record->record.cod, 3);
        return true;
    } else {
        return false;
    }
}

bool app_bt_is_remote_device_support_le_audio(const bt_bdaddr_t *p_addr)
{
    uint8_t remoteCod[3] = {0};
    bool ret = false;

    ret = app_bt_get_remote_cod_by_addr(p_addr, remoteCod);
    DEBUG_INFO(2, "Get remote devcie %d,cod bit_14 = 0x%x",ret,remoteCod[1]);
    if (ret && ((remoteCod[1] & 0x40) == 0x40))
    {
        DEBUG_INFO(0, "This device support le audio");
        return true;
    }
    else
    {
        return false;
    }
}

#if BLE_AUDIO_ENABLED
void app_bt_set_device_support_le_audio(const bt_bdaddr_t *p_addr)
{
    if (app_tws_ibrt_get_remote_link_type(p_addr) == TWS_LINK)
    {
        bt_drv_reg_op_set_page_device_support_le_audio(true);
    }
    else
    {
        if (app_bt_is_remote_device_support_le_audio(p_addr))
        {
            TRACE(0, "[M]support le audio,enable sec connection");
            bt_drv_reg_op_set_page_device_support_le_audio(true);
        }
        else
        {
            TRACE(0, "[M]not support le audio,disable sec connection");
            bt_drv_reg_op_set_page_device_support_le_audio(false);
        }
    }
}
#endif

void app_bt_report_audio_retrigger(uint8_t retrgigerType)
{
#if defined(__CONNECTIVITY_LOG_REPORT__)
    app_ibrt_if_report_audio_retrigger(retrgigerType);
#endif
}

void app_bt_reset_tws_acl_data_packet_check(void)
{
#if defined(__CONNECTIVITY_LOG_REPORT__)
    app_ibrt_if_reset_tws_acl_data_packet_check();
#endif
}

void app_bt_update_link_monitor_info(uint8_t *ptr)
{
#if defined(__CONNECTIVITY_LOG_REPORT__)
    app_ibrt_if_update_link_monitor_info(ptr);
#endif
}

void app_bt_acl_data_packet_check(uint8_t *data)
{
#if defined(__CONNECTIVITY_LOG_REPORT__)
    if (ibrt_if_report_intersys_callback)
    {
        ibrt_if_report_intersys_callback(data);
    }
#endif
}

#ifdef RESUME_MUSIC_AFTER_CRASH_REBOOT
app_bt_curr_palyback_device_t REBOOT_CUSTOM_PARAM_LOC app_bt_curr_palyback_device;
void app_bt_save_curr_palyback_device_handler(HAL_TRACE_STATE_T state)
{
    if (HAL_TRACE_STATE_CRASH_END == state)
    {
        uint8_t device_id = app_bt_audio_get_curr_playing_a2dp();
        if (BT_DEVICE_INVALID_ID != device_id)
        {
            struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
            memcpy(&app_bt_curr_palyback_device.addr, &curr_device->remote, sizeof(bt_bdaddr_t));
        }
    }
}

void app_bt_resume_music_after_crash_reboot_init(void)
{
    hal_trace_app_register(HAL_TRACE_APP_REG_ID_1, app_bt_save_curr_palyback_device_handler, NULL);
}

void app_bt_resume_music_after_crash_reboot(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (0 == memcmp(&curr_device->remote, &app_bt_curr_palyback_device.addr, sizeof(bt_bdaddr_t)))
    {
        app_bt_manager.a2dp_last_paused_device = BT_DEVICE_INVALID_ID;
        app_bt_a2dp_send_key_request(device_id,  AVRCP_KEY_PLAY);
    }
    app_bt_reset_curr_playback_device(device_id);
}

void app_bt_reset_curr_playback_device(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (0 == memcmp(&curr_device->remote, &app_bt_curr_palyback_device.addr, sizeof(bt_bdaddr_t)))
    {
        memset(&app_bt_curr_palyback_device, 0xff, sizeof(app_bt_curr_palyback_device));
    }
}
#endif

#define APP_BT_RSSI_AVERAGE_CALCULATE_COUNT  7
static int32_t app_bt_average_rssi = 0;
static uint8_t app_bt_accumulated_rssi_count = 0;

void app_bt_reset_rssi_collector(void)
{
    app_bt_average_rssi = 0;
    app_bt_accumulated_rssi_count = 0;
}

int32_t app_bt_tx_rssi_analyzer(int8_t rssi)
{
    if (app_bt_accumulated_rssi_count > 0)
    {
        app_bt_average_rssi =
            ((app_bt_average_rssi*app_bt_accumulated_rssi_count) + rssi)/(app_bt_accumulated_rssi_count + 1);
    }
    else
    {
        app_bt_average_rssi = rssi;
    }

    if (app_bt_accumulated_rssi_count < APP_BT_RSSI_AVERAGE_CALCULATE_COUNT)
    {
        app_bt_accumulated_rssi_count++;
    }

    return app_bt_average_rssi;
}

//custom callback fucntion to handle passkey notify event
void app_bt_user_passkey_notify_callback(btif_remote_device_t *remdev,uint32_t passkey)
{
    if (remdev != NULL)
    {
        bt_bdaddr_t *remote = btif_me_get_remote_device_bdaddr(remdev);
        DUMP8("%02x ", remote->address, 6);

        DEBUG_INFO(2,"%s,passkey notify %x",__func__,passkey);

        //custom callback fucntion to handle passkey notify event
    }
}


//custom callback fucntion to handle passkey request event
void app_bt_user_passkey_request_callback(btif_remote_device_t *remdev)
{
    //uint32_t passkey = 0xFFFFFFF;
    //valid range from  0x00000000 to 0x000F423F

    if (remdev != NULL)
    {
        bt_bdaddr_t *remote = btif_me_get_remote_device_bdaddr(remdev);
        DUMP8("%02x ", remote->address, 6);

        DEBUG_INFO(1,"%s,passkey request",__func__);

        //custom will get valid passkey from UI and input it

        //btif_me_input_user_passkey(remdev,passkey);
    }
}

uint8_t app_bt_find_connected_device(void)
{
    uint8_t device_id = BT_DEVICE_INVALID_ID;
    struct BT_DEVICE_T *curr_device = NULL;
    struct BT_DEVICE_T *device_start = app_bt_manager_get_device_all_start();
    struct BT_DEVICE_T *device_end = app_bt_manager_get_device_all_end();

    for (curr_device = device_start; curr_device < device_end; curr_device++)
    {
        if (curr_device->acl_is_connected)
        {
            device_id = curr_device->device_id;
            break;
        }
    }

    DEBUG_INFO(2, "%s d%x", __func__, device_id);
    return device_id;
}

struct BT_DEVICE_T *app_bt_manager_get_device_all_start(void)
{
    return app_bt_manager.bt_devices;
}

struct BT_DEVICE_T *app_bt_manager_get_device_all_end(void)
{
    return app_bt_manager.bt_devices + (BT_DEVICE_NUM + BT_SOURCE_DEVICE_NUM);
}

bool app_bt_device_is_computer(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    uint8_t cod = 0xFF;
    if(curr_device)
    {
        cod = btif_me_get_remote_class_of_device(curr_device->btm_conn);
        if(cod == APP_BT_COD_COMPUTER)
        {
            return true;
        }
    }
    return false;
}

void app_bt_reset_device_info(uint8_t device_id)
{
    if(device_id == BT_DEVICE_INVALID_ID)
    {
        return;
    }
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    curr_device->profiles_connected_before = false;
    curr_device->a2dp_conn_flag = 0;
    curr_device->a2dp_streamming = 0;
    curr_device->a2dp_play_pause_flag = 0;
    curr_device->a2dp_need_resume_flag = 0;
    curr_device->a2dp_disc_on_process = 0;
    curr_device->this_is_closed_bg_a2dp = false;
    curr_device->this_is_delay_reconnect_a2dp = false;
    curr_device->auto_make_remote_play = false;
    curr_device->a2dp_is_auto_paused_by_phone = false;
    curr_device->this_sco_wait_to_play = false;
    curr_device->avrcp_conn_flag = 0;
    curr_device->hf_conn_flag = 0;
    curr_device->is_accepting_sco_request = false;
    curr_device->disc_sco_when_it_connected = false;
    curr_device->a2dp_audio_focus = AUDIOFOCUS_NONE;
    curr_device->call_audio_focus = AUDIOFOCUS_NONE;
    curr_device->a2dp_streaming_available = false;
    curr_device->sco_streaming_available = false;
    curr_device->this_is_bg_a2dp = false;
    curr_device->waiting_pause_suspend = false;
    curr_device->ignore_ring_and_play_tone_self = false;
    curr_device->this_is_paused_bg_a2dp = false;
    DEBUG_INFO(0,"app_bt:clear all bt flag!");
}

#if defined(IBRT)
void app_bt_enable_bt_pcm(bool en)
{
    if (app_bt_manager.config.second_sco_handle_mode == IBRT_ACCEPT_SECOND_SCO &&
        app_bt_manager.config.call_preempt_play_mode == false)
    {
        if(en)
        {
            if(bt_drv_is_support_set_bt_ble_active_link_switch())
            {
                app_ibrt_if_write_btpcm_en(true);
            }
            else
            {
                ASSERT(0, "Do not support dual-sco feature");
            }
        }
    }
    else
    {
        bt_drv_reg_op_set_btpcm_trig_flag(en);
    }
}

void bes_bt_device_snoop_acl_disconnected(uint8_t device_id, void* addr)
    __attribute__((alias("app_bt_device_snoop_acl_disconnected")));
#endif

void app_bt_multi_ibrt_music_bulid_hci(uint8_t* link_id, uint8_t* active, uint8_t num)
{
    if(num == 0)
    {
        return;
    }

    me_link_env_t updated_sch_para[num];
    uint8_t i = 0;
    uint8_t active_temp = 0;
    uint8_t linkid_temp = 0xff;
    uint8_t num_temp = num;
    while(num_temp > 0)
    {
        linkid_temp = link_id[i];
        active_temp = active[i];
        DEBUG_INFO(3,"%s linkid %d,active %d,nb=%d", __func__, linkid_temp, active_temp, num_temp);
        updated_sch_para[i].link_id = linkid_temp;
        updated_sch_para[i].timeslice = bt_drv_reg_op_get_multi_ibrt_slice(active_temp);
        num_temp--;
        i++;
    }

    num_temp = i;
    //sanity check updated_sch_para
    if(num_temp >= 2) {
        for (int j=0; j < i; j++) {
            for (int k=j+1; k < i; k++) {
                if (updated_sch_para[j].link_id == updated_sch_para[k].link_id) {
                    // Replace the contents of the first struct with the second struct
                    updated_sch_para[j] = updated_sch_para[k];
                    num_temp--;
                }
            }
        }
    }

    if(num_temp > 0)
    {
        DEBUG_INFO(1,"%s linkid nb=%d", __func__, num_temp);
        btif_dbg_ibrt_update_time_slice(num_temp, &updated_sch_para[0]);
    }
}

void app_audio_policy_set_bt_ble_active_link(link_traffic_mode_t traffic_mode, uint16_t link_handle)
{
    DEBUG_INFO(0, "traffic mode %d link_handle 0x%x", traffic_mode, link_handle);

    if (bt_drv_is_support_set_bt_ble_active_link_switch() == false)
    {
        TRACE(0, "not_support_set_bt_ble_active_link_switc");
        return;
    }

    if (btif_dbg_set_bt_ble_active_link(traffic_mode, link_handle) != BT_STS_FAILED)
    {
        app_bt_manager.active_link_info.traffic_mode = traffic_mode;
        app_bt_manager.active_link_info.link_handle = link_handle;
    }
    else
    {
        TRACE(0, "%s error!", __func__);
    }
}

const bt_ble_active_link_info_t *app_audio_policy_get_bt_ble_active_link_info(void)
{
    return &app_bt_manager.active_link_info;
}

void app_bt_multi_ibrt_music_config(uint8_t* link_id, uint8_t* active, uint8_t num)
{
    if (bt_drv_is_support_hci_config_multi_ibrt_sche())
    {
        app_bt_multi_ibrt_music_bulid_hci(link_id,active,num);
    }
    else
    {
        bt_drv_reg_op_multi_ibrt_music_config(link_id,active,num);
    }
}

#define ACCESS_MODE_ARRAY_MAX           4

typedef struct
{
    uint8_t  backup_index;
    uint8_t  pickup_index;
    uint8_t  array_counter;
    btif_accessible_mode_t access_mode_array[ACCESS_MODE_ARRAY_MAX];
} access_mode_backup_t;

typedef struct
{
    bool access_mode_sending;
    btif_accessible_mode_t access_mode;
    access_mode_backup_t access_mode_backup;
    btif_accessible_mode_t setting_access_mode;
} app_bt_access_mode_mgr_t;

static app_bt_access_mode_mgr_t access_mode_mgr = {0};

app_bt_access_mode_mgr_t *app_bt_get_access_mode_mgr(void)
{
    return &access_mode_mgr;
}

btif_accessible_mode_t app_bt_get_curr_access_mode(void)
{
    return access_mode_mgr.access_mode;
}

access_mode_backup_t *app_bt_get_access_mode_backup_info(void)
{
    return &access_mode_mgr.access_mode_backup;
}

void app_bt_access_mode_ctrl_init(void)
{
    access_mode_backup_t *access_mode_backup = app_bt_get_access_mode_backup_info();
    access_mode_backup->backup_index = 0;
    access_mode_backup->pickup_index = 0;
    access_mode_backup->array_counter = 0;
    access_mode_backup->access_mode_array[0] = BTIF_BAM_INVALID_ACCESS_MODE;
}

btif_accessible_mode_t app_bt_access_mode_pickup(void)
{
    access_mode_backup_t *access_mode_backup = app_bt_get_access_mode_backup_info();
    uint8_t index = access_mode_backup->pickup_index;
    if (access_mode_backup->array_counter)
    {
        access_mode_backup->array_counter--;
        access_mode_backup->pickup_index = (index + 1) % ACCESS_MODE_ARRAY_MAX;
        TRACE(1,"app_bt_scan_mgr:pickup access mode=%d",\
              access_mode_backup->access_mode_array[index]);

        return access_mode_backup->access_mode_array[index];
    }
    else
    {
        return BTIF_BAM_INVALID_ACCESS_MODE;
    }
}

void app_bt_access_mode_backup(btif_accessible_mode_t mode)
{
    access_mode_backup_t *access_mode_backup = app_bt_get_access_mode_backup_info();
    uint8_t index = access_mode_backup->backup_index;

    if (access_mode_backup->array_counter)
    {
        //backup array is not empty
        uint8_t pre_index = (index == 0 ? ACCESS_MODE_ARRAY_MAX - 1 : index - 1);
        btif_accessible_mode_t access_mode_now = access_mode_backup->access_mode_array[pre_index];
        if (mode != access_mode_now)
        {
            TRACE(1,"app_bt_scan_mgr:backup access mode<0>=%d, index=%d",mode, index);
            access_mode_backup->access_mode_array[index] = mode;
            access_mode_backup->backup_index = (index + 1) % ACCESS_MODE_ARRAY_MAX;
            access_mode_backup->array_counter++;
        }
        else
        {
            TRACE(1,"app_bt_scan_mgr:skipped backup access mode=%d",mode);
        }

        if (access_mode_backup->array_counter >= ACCESS_MODE_ARRAY_MAX)
        {
            if (app_bt_get_curr_access_mode() == mode)
            {
                app_bt_access_mode_ctrl_init();
                return;
            }
            if (mode == BTIF_BAM_NOT_ACCESSIBLE)
            {
                access_mode_backup->backup_index = 1;
                access_mode_backup->pickup_index = 0;
                access_mode_backup->array_counter = 1;
                access_mode_backup->access_mode_array[0] = mode;
            }
            else
            {
                access_mode_backup->backup_index = 2;
                access_mode_backup->pickup_index = 0;
                access_mode_backup->array_counter = 2;
                access_mode_backup->access_mode_array[0] = BTIF_BAM_NOT_ACCESSIBLE;
                access_mode_backup->access_mode_array[1] = mode;
            }
        }
    }
    else
    {
        //backup array is empty, store in directly
        TRACE(1,"app_bt_scan_mgr:backup access mode<1>=%d, index=%d",mode, index);
        access_mode_backup->access_mode_array[index] = mode;
        access_mode_backup->backup_index = (index + 1) % ACCESS_MODE_ARRAY_MAX;
        access_mode_backup->array_counter++;
    }
}

bt_status_t app_bt_set_access_mode_handler(btif_accessible_mode_t mode)
{
    bt_status_t status = BT_STS_SUCCESS;
    access_mode_backup_t *access_mode_backup = app_bt_get_access_mode_backup_info();
    app_bt_access_mode_mgr_t *access_mode_ctrl = app_bt_get_access_mode_mgr();

#if defined(IBRT)
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if(true == p_ibrt_ctrl->ibrt_in_poweroff)
    {
        TRACE(0,"ibrt_scan_mgr:in power off !");
        mode=BTIF_BAM_NOT_ACCESSIBLE;
    }
#endif

    if ((access_mode_ctrl->access_mode_sending == false) && (access_mode_ctrl->access_mode == mode) && (access_mode_backup->array_counter == 0))
    {
        TRACE(2,"app_bt_scan_mgr:filter access mode=%d, current access mode=%d",
                mode,access_mode_ctrl->access_mode);
        return status;
    }

    //backup array is empty, filter duplicate mode
    if ((access_mode_ctrl->access_mode_sending == false) && (access_mode_backup->array_counter == 0))
    {
        status = btif_me_set_accessible_mode(mode, NULL);

        TRACE(1,"app_bt_scan_mgr:btif_me_set_accessible_mode status:%02x", status);
        if (BT_STS_PENDING == status)
        {
            access_mode_ctrl->setting_access_mode = mode;
            access_mode_ctrl->access_mode_sending = true;
        }
        else
        {
            //other access mode is sending, backup access mode
            app_bt_access_mode_backup(mode);
        }
    }
    else
    {
        TRACE(3,"app_bt_scan_mgr:btif_me_set_accessible_mode status sending:%d cnt:%d",
                    access_mode_ctrl->access_mode_sending, access_mode_backup->array_counter);
        app_bt_access_mode_backup(mode);
    }
    return status;
}

bt_status_t app_bt_set_access_mode(btif_accessible_mode_t mode)
{
    bt_status_t status = BT_STS_SUCCESS;
    TRACE(2,"app_bt_scan_mgr:set_access_mode=%d, LR=%p", mode, __builtin_return_address(0));
    bt_defer_call_func_1(app_bt_set_access_mode_handler, bt_fixed_param(mode));
    return status;
}

void app_bt_retry_setting_access_mode(void)
{
    app_bt_access_mode_mgr_t *access_mode_ctrl = app_bt_get_access_mode_mgr();
    btif_accessible_mode_t access_mode = access_mode_ctrl->setting_access_mode;
    if (access_mode != BTIF_BAM_INVALID_ACCESS_MODE)
    {
        TRACE(1, "app_bt_scan_mgr:retry setting access mode %d", access_mode);
        btif_me_set_accessible_mode(access_mode, NULL);
    }
}

void app_bt_access_mode_change_event_handle(const btif_event_t *event, uint8_t error_code)
{
    app_bt_access_mode_mgr_t *access_mode_ctrl = app_bt_get_access_mode_mgr();
    btif_accessible_mode_t access_mode = app_bt_access_mode_pickup();
    btif_accessible_mode_t curr_access_mode = btif_me_get_callback_event_a_mode(event);
    TRACE(2,"app_bt_scan_mgr:accessible_change dest:%d curr:%d", access_mode,curr_access_mode);

    if (BTIF_BEC_NO_ERROR == error_code)
    {
        access_mode_ctrl->access_mode = curr_access_mode;
    }
    if (access_mode != BTIF_BAM_INVALID_ACCESS_MODE)
    {
        access_mode_ctrl->setting_access_mode = access_mode;
        btif_me_set_accessible_mode(access_mode, NULL);
    }
    else
    {
        if (BTIF_BEC_NO_ERROR != error_code)
        {
            app_bt_retry_setting_access_mode();
            return;
        }
        access_mode_ctrl->setting_access_mode = BTIF_BAM_INVALID_ACCESS_MODE;
        access_mode_ctrl->access_mode_sending = false;
    }
}

struct app_bt_config *app_bt_get_config(void)
{
    return  &app_bt_manager.config;
}

/**
 * @brief Class of device for generic headset
 *
 */
uint32_t app_bt_get_class_of_device_headset(void)
{
    uint32_t class_of_device = 0;
    uint32_t services = CFG_COD_AUDIO | CFG_COD_RENDERING;

#ifdef BT_A2DP_SRC_ROLE
    services |= CFG_COD_CAPTURING;
#endif

    if (besbt_cfg.bt_hid_cod_enable)
    {
        class_of_device = CFG_COD_MAJOR_PERIPHERAL | CFG_COD_MINOR_PERIPH_KEYBOARD;
    }
    else
    {
#if defined(BLE_USB_AUDIO_SUPPORT) || defined(APP_USB_A2DP_SOURCE)
        class_of_device = CFG_COD_MAJOR_AUDIO | CFG_COD_MINOR_AUDIO_PORTABLEAUDIO;
#else
        class_of_device = CFG_COD_MAJOR_AUDIO | CFG_COD_MINOR_AUDIO_HEADSET;
#endif
    }

#if BLE_AUDIO_ENABLED
    if (besbt_cfg.le_audio_enabled)
    {
        class_of_device |= CFG_COD_MAJOR_LE_AUDIO;
    }
#endif

    return services | class_of_device;
}

/**
 * @brief Class of device for generic watch
 *
 */
uint32_t app_bt_get_class_of_device_watch(void)
{
    uint32_t class_of_device = 0;
    uint32_t services = CFG_COD_AUDIO | CFG_COD_TELEPHONY |
                        CFG_COD_OBJECT_TRANSFER | CFG_COD_CAPTURING | CFG_COD_NETWORKING;

    if (besbt_cfg.bt_hid_cod_enable)
    {
        class_of_device = CFG_COD_MAJOR_PERIPHERAL | CFG_COD_MINOR_PERIPH_KEYBOARD;
    }
    else
    {
        class_of_device = CFG_COD_MAJOR_WEARABLE | CFG_COD_MINOR_PHONE_CELLULAR;
    }

#if BLE_AUDIO_ENABLED
    if (besbt_cfg.le_audio_enabled)
    {
        class_of_device |= CFG_COD_MAJOR_LE_AUDIO;
    }
#endif

    return services | class_of_device;
}

/**
 ****************************************************************************************
 * @brief ble set channel classification map allows the Host to specify
 *        a channel classification for the data, secondary advertising, periodic,
 *        and isochronous physical channels based on its "local information"
 * HCI_LE_SET_HOST_CHL_CLASSIFICATION: Size: 5 octets (39 bits meaningful)
 * This parameter contains 37 1-bit fields
 * The nth such field (in the range 0 to 36) contains the value for channel n:
 *    0: channel n is bad
 *    1: channel n is unknown
 *
 * The most significant bits are reserved for future use.
 * At least one channel shall be marked as unknown.
 *
 ****************************************************************************************
 */
void app_bt_set_le_host_chnl_classification(uint8_t *chnl_map)
{
    app_bt_start_custom_function_in_bt_thread((uint32_t)chnl_map,
        (uint32_t)NULL, (uint32_t)btif_me_ble_set_host_chnl_classification);
}

/**
 ****************************************************************************************
 * @brief app_bt_read_le_chnl_map returns the current Channel_Map for the specified Connection_Handle.
 * The nth such field (in the range 0 to 36) contains the value for the Link Layer channel index n:
 *    0: channel n is bad
 *    1: channel n is unknown
 *
 * The most significant bits are reserved for future use.
 ****************************************************************************************
 */
void app_bt_read_le_chnl_map(uint16_t conn_handle)
{
    app_bt_start_custom_function_in_bt_thread((uint32_t)conn_handle,
        (uint32_t)NULL, (uint32_t)btif_me_ble_read_chnle_map);
}

APP_BT_LE_READ_CHANNEL_MAP_CALLBACK g_le_read_channel_map_cb = NULL;
void app_bt_registe_read_le_chnl_map_callback(APP_BT_LE_READ_CHANNEL_MAP_CALLBACK cb)
{
    g_le_read_channel_map_cb = cb;
}

void app_bt_process_cmd_complete_read_le_host_chnl_map(uint8 *data)
{
    if (g_le_read_channel_map_cb)
    {
        g_le_read_channel_map_cb(data);
    }
}

void app_bt_register_coex_sink_global_event_handle(bt_sink_event_global_cb_t func)
{
    bt_sink_event_global_callback = func;
}

void app_bt_register_coex_audio_sink_event_handler(bt_audio_coex_event_cb_t func)
{
    bt_audio_coex_event_callback = func;
}

void app_bt_coex_sink_global_event_ind(const btif_event_t * event)
{
    if (bt_sink_event_global_callback)
    {
        bt_sink_event_global_callback(event);
    }
}

void app_bt_coex_audio_sink_event_ind(bt_bdaddr_t *addr, enum app_bt_audio_event_t event, uint32_t data)
{
    if (bt_audio_coex_event_callback)
    {
        bt_audio_coex_event_callback(addr, event, data);
    }
}

btif_remote_device_t* bes_bt_get_remote_dev_by_handle(uint16_t hci_handle)
    __attribute__((alias("app_bt_get_remote_dev_by_handle")));

uint8_t bes_bt_get_active_cons(void)
    __attribute__((alias("app_bt_get_active_cons")));

btif_remote_device_t* bes_bt_get_remote_dev_by_address(const bt_bdaddr_t *bdaddr)
    __attribute__((alias("app_bt_get_remote_dev_by_address")));

bt_status_t bes_bt_set_fix_tws_interval_param(uint16_t duration, uint16_t interval, uint16_t interval_in_sco)
    __attribute__((alias("app_bt_set_fix_tws_interval_param")));

btif_remote_device_t *bes_bt_get_remote_dev_by_device_id(uint8_t device_id)
    __attribute__((alias("app_bt_get_remote_dev_by_device_id")));

uint8_t app_bt_hfp_streaming_sco_count(void)
{
    uint8_t count = 0;
#ifdef BT_HFP_SUPPORT
    struct BT_DEVICE_T* curr_device = NULL;
    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->hf_audio_state == BT_HFP_AUDIO_CON)
        {
            count += 1;
        }
    }
#endif
    return count;
}
