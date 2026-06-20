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
#ifdef BT_A2DP_SUPPORT
#undef MOUDLE
#define MOUDLE A2DP_APP
#include <stdio.h>
#include "cmsis_os.h"
#include "cmsis.h"
#include "hal_uart.h"
#include "hal_timer.h"
#include "audioflinger.h"
#include "lockcqueue.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "hal_codec.h"
#include "analog.h"
#include "bluetooth.h"
#include "me_api.h"
#include "spp_api.h"
#include "besaud_api.h"
#include "bt_if.h"
#include "app_audio.h"
#include "nvrecord_bt.h"
#include "nvrecord_env.h"
#include "nvrecord_dev.h"
#include "hal_location.h"
#include "a2dp_api.h"
#include "app_a2dp.h"
#include "app_trace_rx.h"
#include "ecc_p192.h"

#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)
#include "lhdcUtil.h"
#endif
#include "co_bt_defines.h"

#if defined(A2DP_LDAC_ON)
#include"ldacBT.h"
#endif

#include "a2dp_api.h"
#include "avrcp_api.h"
#include "besbt.h"
#include "besbt_cfg.h"
#include "cqueue.h"
#include "btapp.h"
#include "app_bt.h"
#include "apps.h"
#include "audio_policy.h"
#include "app_bt_media_manager.h"
#include "tgt_hardware.h"
#include "bt_drv_interface.h"
#include "hci_api.h"
#include "hal_bootmode.h"
#include "bt_drv_reg_op.h"

#ifdef BT_USB_AUDIO_DUAL_MODE
#include "btusb_audio.h"
#endif

#if defined(BT_MAP_SUPPORT)
#include "app_map.h"
#endif

#ifdef __THIRDPARTY
#include "app_thirdparty.h"
#endif

#ifdef __AI_VOICE__
#include "app_ai_voice.h"
#endif

#ifdef CUSTOM_BITRATE
#include "app_ibrt_customif_cmd.h"
#endif

#define _FILE_TAG_ "A2DP"
#include "color_log.h"
#include "app_bt_func.h"

#if (A2DP_DECODER_VER >= 2)
#include "a2dp_decoder.h"
#endif

#if defined(IBRT)
#include "app_ibrt_internal.h"
#include "app_ibrt_a2dp.h"
#include "app_tws_ibrt_cmd_sync_a2dp_status.h"
#if defined(IBRT_UI)
#include "app_tws_ibrt_conn_api.h"
#endif
#include "app_tws_besaud.h"
#endif

#ifdef BISTO_ENABLED
#include "gsound_custom_bt.h"
#endif

#ifdef __IAG_BLE_INCLUDE__
#include "bluetooth_ble_api.h"
#endif

#ifdef __AI_VOICE__
#include "ai_spp.h"
#endif

#ifdef __BIXBY_VOICE__
#include "app_bixby_thirdparty_if.h"
#endif

#include "app_audio_active_device_manager.h"

#include "app_bt_stream.h"
#include "app_bt_media_manager.h"

#if defined(__CONNECTIVITY_LOG_REPORT__)
#include "app_ibrt_link_monitor.h"
#endif

#include "audio_player_adapter.h"

#define APP_A2DP_STRM_FLAG_QUERY_CODEC  0x08

extern int store_sbc_buffer(uint8_t device_id, unsigned char *buf, unsigned int len);

static RMT_A2DP_SINK_DEV_DATA_IND_CALLBACK rmt_a2dp_sink_dev_data_ind_cb = NULL;


void (*app_a2dp_notify_receive_stream_cb)(void) = NULL;


uint8_t a2dp_get_current_codec_non_type(uint8_t *elements);

static bool (*app_a2dp_custom_allow_receive_steam_cb)(void) = NULL;

void app_a2dp_register_custom_allow_receive_steam(bool (*cb)(void))
{
    app_a2dp_custom_allow_receive_steam_cb = cb;
    DEBUG_INFO(1, "%s %p", __func__, app_a2dp_custom_allow_receive_steam_cb);
}

static bool app_a2dp_custom_allow_receive_steam()
{
    bool ret = true;
    if (app_a2dp_custom_allow_receive_steam_cb)
    {
        ret = app_a2dp_custom_allow_receive_steam_cb();
    }
    return ret;
}

void rmt_a2dp_sink_dev_data_ind_callback_register(RMT_A2DP_SINK_DEV_DATA_IND_CALLBACK data_ind_cb)
{
    rmt_a2dp_sink_dev_data_ind_cb = data_ind_cb;
}

WEAK int a2dp_audio_latency_factor_setlow(void)
{
    return 0;
}

extern void app_bt_audio_a2dp_stream_recheck_timer_callback(void const *param);
extern void app_bt_delay_send_hfp_hold_callback(void const *param);
extern void app_bt_audio_check_a2dp_restreaming_timer_handler(void const *param);
extern void app_bt_audio_delay_play_a2dp_timer_handler(void const *param);

osTimerDef (APP_A2DP_STREAM_RECHECK_TIMER0, app_bt_audio_a2dp_stream_recheck_timer_callback);
osTimerDef (APP_A2DP_ABANDON_FOCUS_TIMER0,app_bt_audio_abandon_focus_timer_handler);
osTimerDef (APP_CHECK_A2DP_RESTERAMING_TIMER0, app_bt_audio_check_a2dp_restreaming_timer_handler);
osTimerDef (APP_A2DP_DELAY_PLAY_A2DP_TIMER0,app_bt_audio_delay_play_a2dp_timer_handler);
#ifdef BT_HFP_SUPPORT
osTimerDef (APP_HFP_DELAY_SEND_HOLD_TIMER0, app_bt_delay_send_hfp_hold_callback);
osTimerDef (APP_SELF_RING_TONE_PLAY_TIMER0, app_bt_audio_self_ring_tone_play_timer_handler);
#endif

#if BT_DEVICE_NUM > 1
osTimerDef (APP_A2DP_STREAM_RECHECK_TIMER1, app_bt_audio_a2dp_stream_recheck_timer_callback);
osTimerDef (APP_CHECK_A2DP_RESTERAMING_TIMER1, app_bt_audio_check_a2dp_restreaming_timer_handler);
osTimerDef (APP_A2DP_DELAY_PLAY_A2DP_TIMER1,app_bt_audio_delay_play_a2dp_timer_handler);
#ifdef BT_HFP_SUPPORT
osTimerDef (APP_HFP_DELAY_SEND_HOLD_TIMER1, app_bt_delay_send_hfp_hold_callback);
osTimerDef (APP_SELF_RING_TONE_PLAY_TIMER1, app_bt_audio_self_ring_tone_play_timer_handler);
#endif
osTimerDef (APP_A2DP_ABANDON_FOCUS_TIMER1,app_bt_audio_abandon_focus_timer_handler);
#endif

#if BT_DEVICE_NUM > 2
osTimerDef (APP_A2DP_STREAM_RECHECK_TIMER2, app_bt_audio_a2dp_stream_recheck_timer_callback);
#ifdef BT_HFP_SUPPORT
osTimerDef (APP_HFP_DELAY_SEND_HOLD_TIMER2, app_bt_delay_send_hfp_hold_callback);
osTimerDef (APP_SELF_RING_TONE_PLAY_TIMER2, app_bt_audio_self_ring_tone_play_timer_handler);
#endif
osTimerDef (APP_A2DP_ABANDON_FOCUS_TIMER2,app_bt_audio_abandon_focus_timer_handler);
osTimerDef (APP_CHECK_A2DP_RESTERAMING_TIMER2, app_bt_audio_check_a2dp_restreaming_timer_handler);
osTimerDef (APP_A2DP_DELAY_PLAY_A2DP_TIMER2,app_bt_audio_delay_play_a2dp_timer_handler);
#endif

#ifdef IBRT
static uint8_t a2dp_sniff_reject_device = BT_DEVICE_NUM;
osTimerId a2dp_sniff_reject_guard_timer = NULL;
static void app_a2dp_reject_sniff_guard_timer_callback(void const *param);
osTimerDef (APP_A2DP_REJECTSNIFF_TIMER, app_a2dp_reject_sniff_guard_timer_callback);
#endif

void get_value1_pos(U8 mask,U8 *start_pos, U8 *end_pos)
{
    U8 num = 0;

    for(U8 i=0;i<8;i++){
        if((0x01<<i) & mask){
            *start_pos = i;//start_pos,end_pos stands for the start and end position of value 1 in mask
            break;
        }
    }
    for(U8 i=0;i<8;i++){
        if((0x01<<i) & mask)
            num++;//number of value1 in mask
    }
    *end_pos = *start_pos + num - 1;
}
U8 a2dp_codec_sbc_get_valid_bit(U8 elements, U8 mask)
{
    U8 start_pos,end_pos;

    get_value1_pos(mask,&start_pos,&end_pos);
//    DEBUG_INFO(2,"!!!start_pos:%d,end_pos:%d\n",start_pos,end_pos);
    for(U8 i = start_pos; i <= end_pos; i++){
        if((0x01<<i) & elements){
            elements = ((0x01<<i) | (elements & (~mask)));
            break;
        }
    }
    return elements;
}

static void a2dp_set_codec_info(uint8_t dev_num, const uint8_t *codec)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(dev_num);
    curr_device->codec_type = codec[0];
    curr_device->sample_bit = codec[1];
    curr_device->sample_rate = codec[2];
#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)
    curr_device->a2dp_lhdc_llc = codec[3];
#endif
    bt_adapter_set_a2dp_codec_info(dev_num, curr_device->codec_type, curr_device->sample_rate, curr_device->sample_bit);
}

#if defined(IBRT) && defined(IBRT_UI)
static void app_sdp_connect_ibrt_tws_switch_protect_handle(uint8_t device_id, bt_bdaddr_t *remote, bool set_protect)
{
    if (set_protect) {
        if (remote && app_ibrt_conn_is_profile_exchanged(remote)) {
            app_ibrt_set_profile_connect_protect(device_id, APP_IBRT_SDP_PROFILE_ID);
        }
    } else {
        app_ibrt_clear_profile_connect_protect(device_id, APP_IBRT_SDP_PROFILE_ID);
    }
}
#endif

static void a2dp_get_codec_info(uint8_t dev_num, uint8_t *codec)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(dev_num);
    codec[0] = curr_device->codec_type;
    codec[1] = curr_device->sample_bit;
    codec[2] = curr_device->sample_rate;
#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)
    codec[3] = curr_device->a2dp_lhdc_llc;
#endif
}

static bool g_a2dp_is_inited = false;

#define SBC_RET_STATUS_SUCCESS         0
#define SBC_RET_STATUS_FAILED          1
#define SBC_RET_STATUS_NO_RESOURCES      12
#define SBC_RET_STATUS_NOT_FOUND         13
#define SBC_RET_STATUS_INVALID_PARM      18
#define SBC_RET_STATUS_CONTINUE          24

void a2dp_init(void)
{
    ASSERT(SBC_RET_STATUS_SUCCESS == BT_STS_SUCCESS &&
        SBC_RET_STATUS_FAILED == BT_STS_FAILED &&
        SBC_RET_STATUS_NO_RESOURCES == BT_STS_NO_RESOURCES &&
        SBC_RET_STATUS_NOT_FOUND == BT_STS_NOT_FOUND &&
        SBC_RET_STATUS_INVALID_PARM == BT_STS_INVALID_PARM &&
        SBC_RET_STATUS_CONTINUE == BT_STS_CONTINUE,
        "sbc status must match bt status");

    if (g_a2dp_is_inited)
    {
        return;
    }

    g_a2dp_is_inited = true;

#ifdef CUSTOM_BITRATE
    a2dp_avdtpcodec_sbc_user_configure_set(nv_record_get_extension_entry_ptr()->codec_user_info.sbc_bitpool, true);
    a2dp_avdtpcodec_aac_user_configure_set(nv_record_get_extension_entry_ptr()->codec_user_info.aac_bitrate, true);
    app_audio_dynamic_update_dest_packet_mtu_set(A2DP_AUDIO_CODEC_TYPE_SBC, (nv_record_get_extension_entry_ptr()->codec_user_info.audio_latentcy-USER_CONFIG_SBC_AUDIO_LATENCY_ERROR)/3, true);//sbc
    app_audio_dynamic_update_dest_packet_mtu_set(A2DP_AUDIO_CODEC_TYPE_MPEG2_4_AAC, (nv_record_get_extension_entry_ptr()->codec_user_info.audio_latentcy-USER_CONFIG_AAC_AUDIO_LATENCY_ERROR)/23, true);//aac
#endif

    btif_a2dp_register_multi_link_close_req_allowed_callback(app_bt_audio_a2dp_close_req_allow);

    btif_a2dp_init(a2dp_callback,
#ifdef BT_SOURCE
        a2dp_source_callback
#else
        NULL
#endif
        );

    btif_a2dp_get_codec_info_func(a2dp_get_codec_info);

    btif_a2dp_set_codec_info_func(a2dp_set_codec_info);

    btif_a2dp_get_codec_non_type_func(a2dp_get_current_codec_non_type);

    if (app_bt_sink_is_enabled())
    {
#if BT_DEVICE_NUM > 1
        btif_a2dp_register_multi_link_connect_not_allowed_callback(app_bt_audio_a2dp_disconnect_self_check);
#endif
#ifdef IBRT
        if(a2dp_sniff_reject_guard_timer == NULL)
        {
            a2dp_sniff_reject_guard_timer =  osTimerCreate(osTimer(APP_A2DP_REJECTSNIFF_TIMER), osTimerOnce,
                &a2dp_sniff_reject_device);
        }
#endif

#if defined(IBRT) && defined(IBRT_UI)
        btif_sdp_register_ibrt_tws_switch_protect_handle(app_sdp_connect_ibrt_tws_switch_protect_handle);
#endif

#ifdef BT_AVRCP_SUPPORT
        avrcp_init();
#endif
        for(int i =0; i< BT_DEVICE_NUM; i++ )
        {
            struct BT_DEVICE_T* curr_device = app_bt_get_device(i);
            curr_device->btif_a2dp_stream = btif_a2dp_alloc_sink_stream();
            curr_device->profile_mgr.stream = curr_device->btif_a2dp_stream->a2dp_stream;
            curr_device->a2dp_connected_stream = curr_device->btif_a2dp_stream->a2dp_stream;
            curr_device->channel_mode = 0;
            curr_device->a2dp_channel_num = 2;
            curr_device->a2dp_conn_flag = 0;
            curr_device->a2dp_streamming = 0;
            curr_device->a2dp_play_pause_flag = 0;
            curr_device->avrcp_play_status_wait_to_handle = false;
            curr_device->a2dp_disc_on_process = 0;
            curr_device->rsv_avdtp_start_signal = false;
            curr_device->a2dp_status_recheck = 0;
            curr_device->hfp_status_recheck = 0;
#ifdef A2DP_STREAM_DETECT_NO_DECODE
            app_a2dp_stream_detect_time_init(i);
#endif // A2DP_STREAM_DETECT_NO_DECODE
#ifdef __A2DP_AVDTP_CP__
            curr_device->avdtp_cp = 0;
            memset(curr_device->a2dp_avdtp_cp_security_data, 0, sizeof(curr_device->a2dp_avdtp_cp_security_data));
#endif
            btif_a2dp_stream_init(curr_device->btif_a2dp_stream, BTIF_A2DP_STREAM_TYPE_SINK);

            a2dp_codec_sbc_init(i);

#if defined(A2DP_AAC_ON)
            a2dp_codec_aac_init(i);
#endif
#if defined(A2DP_LDAC_ON)
            a2dp_codec_ldac_init(i);
#endif
#if defined(A2DP_LHDC_ON)
            a2dp_codec_lhdc_init(i);
#endif
#if defined(A2DP_LHDCV5_ON)
            a2dp_codec_lhdcv5_init(i);
#endif
#if defined(MASTER_USE_OPUS) || defined(ALL_USE_OPUS)
            a2dp_codec_opus_init(i);
#endif
#if defined(A2DP_SCALABLE_ON)
            a2dp_codec_scalable_init(i);
#endif
#if defined(A2DP_LC3_ON)
            a2dp_codec_lc3_init(i);
#endif
#if defined(A2DP_L2HC_ON)
            a2dp_codec_l2hc_init(i);
#endif
#if defined(A2DP_MIHC_ON)
            a2dp_codec_mihc_init(i);
#endif
            if (i == 0)
            {
                curr_device->a2dp_stream_recheck_timer = osTimerCreate(osTimer(APP_A2DP_STREAM_RECHECK_TIMER0), osTimerOnce, curr_device);
                //curr_device->a2dp_delay_reconnect_timer = osTimerCreate(osTimer(APP_A2DP_DELAY_RECONNECT_TIMER),osTimerOnce,curr_device);
#ifdef BT_HFP_SUPPORT
                curr_device->hfp_delay_send_hold_timer = osTimerCreate(osTimer(APP_HFP_DELAY_SEND_HOLD_TIMER0),osTimerOnce,curr_device);
                curr_device->self_ring_tone_play_timer = osTimerCreate(osTimer(APP_SELF_RING_TONE_PLAY_TIMER0),osTimerOnce,curr_device);
#endif
                curr_device->abandon_focus_timer = osTimerCreate(osTimer(APP_A2DP_ABANDON_FOCUS_TIMER0), osTimerOnce, curr_device);
                curr_device->check_a2dp_restreaming_timer = osTimerCreate(osTimer(APP_CHECK_A2DP_RESTERAMING_TIMER0), osTimerOnce, curr_device);
                curr_device->delay_play_a2dp_timer = osTimerCreate(osTimer(APP_A2DP_DELAY_PLAY_A2DP_TIMER0), osTimerOnce, curr_device);
            }
#if BT_DEVICE_NUM > 1
            else if (i == 1)
            {
                curr_device->a2dp_stream_recheck_timer = osTimerCreate(osTimer(APP_A2DP_STREAM_RECHECK_TIMER1), osTimerOnce, curr_device);
                //curr_device->a2dp_delay_reconnect_timer = osTimerCreate(osTimer(APP_A2DP_DELAY_RECONNECT_TIMER),osTimerOnce,curr_device);
#ifdef BT_HFP_SUPPORT
                curr_device->hfp_delay_send_hold_timer = osTimerCreate(osTimer(APP_HFP_DELAY_SEND_HOLD_TIMER1),osTimerOnce,curr_device);
                curr_device->self_ring_tone_play_timer = osTimerCreate(osTimer(APP_SELF_RING_TONE_PLAY_TIMER1),osTimerOnce,curr_device);
#endif
                curr_device->abandon_focus_timer = osTimerCreate(osTimer(APP_A2DP_ABANDON_FOCUS_TIMER1), osTimerOnce, curr_device);
                curr_device->check_a2dp_restreaming_timer = osTimerCreate(osTimer(APP_CHECK_A2DP_RESTERAMING_TIMER1), osTimerOnce, curr_device);
                curr_device->delay_play_a2dp_timer = osTimerCreate(osTimer(APP_A2DP_DELAY_PLAY_A2DP_TIMER1), osTimerOnce, curr_device);
            }
#endif
#if BT_DEVICE_NUM > 2
            else if (i == 2)
            {
#ifndef IBRT
                curr_device->avrcp_reconnect_timer = osTimerCreate(osTimer(APP_AVRCP_RECONNECT_TIMER2), osTimerOnce, curr_device);
#endif
                curr_device->a2dp_stream_recheck_timer = osTimerCreate(osTimer(APP_A2DP_STREAM_RECHECK_TIMER2), osTimerOnce, curr_device);
                curr_device->hfp_delay_send_hold_timer = osTimerCreate(osTimer(APP_HFP_DELAY_SEND_HOLD_TIMER2),osTimerOnce,curr_device);
                curr_device->self_ring_tone_play_timer = osTimerCreate(osTimer(APP_SELF_RING_TONE_PLAY_TIMER2),osTimerOnce,curr_device);
                curr_device->abandon_focus_timer = osTimerCreate(osTimer(APP_A2DP_ABANDON_FOCUS_TIMER2), osTimerOnce, curr_device);
                curr_device->check_a2dp_restreaming_timer = osTimerCreate(osTimer(APP_CHECK_A2DP_RESTERAMING_TIMER2), osTimerOnce, curr_device);
                curr_device->delay_play_a2dp_timer = osTimerCreate(osTimer(APP_A2DP_DELAY_PLAY_A2DP_TIMER2), osTimerOnce, curr_device);
            }
#endif
        }
        app_bt_manager.curr_a2dp_stream_id = BT_DEVICE_ID_1;
    }
}

static bool a2dp_bdaddr_from_id(uint8_t id, bt_bdaddr_t *bd_addr) {
  btif_remote_device_t *remDev = NULL;
  ASSERT(id < BT_DEVICE_NUM, "invalid bt device id");
  if(NULL != bd_addr) {
    remDev = btif_a2dp_get_remote_device(app_bt_get_device(id)->a2dp_connected_stream);
    memset(bd_addr, 0, sizeof(bt_bdaddr_t));
    if (NULL != remDev) {
      memcpy(bd_addr, btif_me_get_remote_device_bdaddr(remDev), sizeof(bt_bdaddr_t));
      return true;
    }
  }
  return false;
}

static bool a2dp_bdaddr_cmp(const bt_bdaddr_t *bd_addr_1, const bt_bdaddr_t *bd_addr_2)
{
  if((NULL == bd_addr_1) || (NULL == bd_addr_2)) {
    return false;
  }
  return (memcmp(bd_addr_1->address,
      bd_addr_2->address, BTIF_BD_ADDR_SIZE) == 0);
}

bool a2dp_id_from_bdaddr(const bt_bdaddr_t *bd_addr, uint8_t *id)
{
    bt_bdaddr_t curr_addr = {0};
    uint8_t curr_id = BT_DEVICE_NUM;
    int i = 0;

    for (; i < BT_DEVICE_NUM; ++i) {
        if (app_bt_is_device_profile_connected(i)) {
            a2dp_bdaddr_from_id(i, &curr_addr);
            if(a2dp_bdaddr_cmp(&curr_addr, bd_addr)) {
                curr_id = i;
                break;
            }
        }
    }

    if(id) {
        *id = curr_id;
    }

    return (curr_id < BT_DEVICE_NUM);
}

#define APP_BT_PAUSE_MEDIA_PLAYER_DELAY 300
osTimerId app_bt_pause_media_player_delay_timer_id = NULL;
static uint8_t deviceIdPendingForMediaPlayerPause = 0xff;
static void app_bt_pause_media_player_delay_timer_handler(void const *n);
osTimerDef (APP_BT_PAUSE_MEDIA_PLAYER_DELAY_TIMER, app_bt_pause_media_player_delay_timer_handler);
static void app_bt_pause_media_player_delay_timer_handler(void const *n)
{
    app_bt_start_custom_function_in_bt_thread(deviceIdPendingForMediaPlayerPause, 0, (uint32_t)app_bt_pause_music_player);
}

bool app_bt_pause_music_player(uint8_t deviceId)
{
    struct BT_DEVICE_T* curr_device = NULL;

    if (!app_bt_is_music_player_working(deviceId))
    {
        return false;
    }

    curr_device = app_bt_get_device(deviceId);

    DEBUG_INFO(1,"Pause music player of device %d", deviceId);
    app_bt_suspend_a2dp_streaming(deviceId);

    btif_avrcp_set_panel_key(curr_device->avrcp_channel, BTIF_AVRCP_POP_PAUSE, TRUE);
    btif_avrcp_set_panel_key(curr_device->avrcp_channel, BTIF_AVRCP_POP_PAUSE, FALSE);

    curr_device->a2dp_play_pause_flag = 0;
    curr_device->a2dp_need_resume_flag = 1;

    deviceIdPendingForMediaPlayerPause = 0xff;

    return true;
}

void app_bt_pause_media_player_again(uint8_t deviceId)
{
    if (NULL == app_bt_pause_media_player_delay_timer_id)
    {
        app_bt_pause_media_player_delay_timer_id = osTimerCreate(
            osTimer(APP_BT_PAUSE_MEDIA_PLAYER_DELAY_TIMER), osTimerOnce, NULL);
    }

    if (deviceIdPendingForMediaPlayerPause == 0xff)
    {
        DEBUG_INFO(1,"(d%x) The media player is resumed before it's allowed, pause again", deviceId);
        deviceIdPendingForMediaPlayerPause = deviceId;
        osTimerStart(app_bt_pause_media_player_delay_timer_id, APP_BT_PAUSE_MEDIA_PLAYER_DELAY);
    }
    else
    {
        DEBUG_INFO(1,"(d%x) %s another device %d is pending", deviceId, __func__,
                                                deviceIdPendingForMediaPlayerPause);
    }
}

bool app_bt_is_music_player_working(uint8_t deviceId)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(deviceId);
    bool is_streaming = app_bt_is_a2dp_streaming(deviceId);

    DEBUG_INFO(3,"device %d a2dp streaming %d playback state %d", deviceId, is_streaming,
                                                curr_device->avrcp_playback_status);
    return (is_streaming && 0x01 == curr_device->avrcp_playback_status);
}

void app_bt_suspend_a2dp_streaming(uint8_t deviceId)
{
    if (!app_bt_is_a2dp_streaming(deviceId))
    {
        return;
    }

    DEBUG_INFO(1,"Suspend a2dp streaming of device %d", deviceId);
    btif_a2dp_suspend_stream(app_bt_get_device(deviceId)->a2dp_connected_stream);
}

void app_bt_resume_music_player(uint8_t deviceId)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(deviceId);

    if (!curr_device || !curr_device->a2dp_need_resume_flag)
    {
        return;
    }

    curr_device->a2dp_need_resume_flag = 0;

    if (app_bt_is_music_player_working(deviceId))
    {
        return;
    }

    DEBUG_INFO(1,"Resume music player of device %d", deviceId);

    btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_PLAY,TRUE);
    btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_PLAY,FALSE);

    curr_device->a2dp_play_pause_flag = 1;
}

bool app_bt_is_a2dp_streaming(uint8_t deviceId)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(deviceId);
    if (NULL != curr_device)
    {
        return curr_device->a2dp_streamming;
    }
    return false;
}

bool app_bt_is_a2dp_disconnected(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    return curr_device && (0 == curr_device->a2dp_conn_flag);
}

void app_bt_a2dp_disable_aac_codec(bool disable)
{
    app_bt_start_custom_function_in_bt_thread((uint32_t)(uintptr_t)disable, 0, (uint32_t)(uintptr_t)btif_a2dp_disable_aac_codec);
}

void app_bt_a2dp_disable_sbc_codec(bool disable)
{
    app_bt_start_custom_function_in_bt_thread((uint32_t)(uintptr_t)disable, 0, (uint32_t)(uintptr_t)btif_a2dp_disable_sbc_codec);
}

void app_bt_a2dp_set_codec_param_handle(codec_act_param_t * action)
{
    struct bt_defer_param_t param_a =  bt_alloc_param_size(action, sizeof(codec_act_param_t ));

    bt_thread_call_func_1((uint32_t)(uintptr_t)btif_a2dp_set_codec_parameters, param_a);
}

void app_bt_a2dp_disable_vendor_codec(bool disable)
{
    app_bt_start_custom_function_in_bt_thread((uint32_t)(uintptr_t)disable, 0, (uint32_t)(uintptr_t)btif_a2dp_disable_vendor_codec);
}

void app_bt_a2dp_reconfig_to_aac(a2dp_stream_t *stream)
{
    app_bt_start_custom_function_in_bt_thread((uint32_t)(uintptr_t)stream, 0, (uint32_t)(uintptr_t)btif_a2dp_reconfig_codec_to_aac);
}

void app_bt_a2dp_reconfig_to_vendor_codec(a2dp_stream_t *stream, uint8_t codec_id, uint8_t a2dp_non_type)
{
    app_bt_call_func_in_bt_thread((uint32_t)(uintptr_t)stream, codec_id, a2dp_non_type, 0, (uint32_t)(uintptr_t)btif_a2dp_reconfig_codec_to_vendor_codec);
}

void app_bt_a2dp_reconfig_to_sbc(a2dp_stream_t *stream)
{
    app_bt_start_custom_function_in_bt_thread((uint32_t)(uintptr_t)stream, 0, (uint32_t)(uintptr_t)btif_a2dp_reconfig_codec_to_sbc);
}

void app_bt_a2dp_reconfig_codec(a2dp_stream_t *stream, uint8_t code_type)
{
    app_bt_start_custom_function_in_bt_thread((uint32_t)(uintptr_t)stream, (uint32_t)code_type, (uint32_t)(uintptr_t)btif_a2dp_reconfig_codec);
}

FRAM_TEXT_LOC uint8_t bt_sbc_player_get_sample_bit(void) {
    return app_bt_get_device(app_bt_audio_get_curr_a2dp_device())->sample_bit;
}

FRAM_TEXT_LOC uint8_t bt_sbc_player_get_duration(void) {
    return app_bt_get_device(app_bt_audio_get_curr_a2dp_device())->a2dp_frame_dr;
}

extern "C" uint16_t a2dp_Get_curr_a2dp_conhdl(void)
{
    return app_bt_manager.current_a2dp_conhdl;
}

void a2dp_get_curStream_remDev(btif_remote_device_t **p_remDev)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(app_bt_audio_get_curr_a2dp_device());
    *p_remDev = btif_a2dp_get_remote_device(curr_device->a2dp_connected_stream);
}

int a2dp_audio_sbc_set_frame_info(int rcv_len, int frame_num);

extern enum AUD_SAMPRATE_T a2dp_sample_rate;

#define A2DP_TIMESTAMP_TRACE(s,...)

#define A2DP_TIMESTAMP_DEBOUNCE_DURATION (1000)
#define A2DP_TIMESTAMP_MODE_SAMPLE_THRESHOLD (2000)

#define A2DP_TIMESTAMP_SYNC_LIMIT_CNT (100)
#define A2DP_TIMESTAMP_SYNC_TIME_THRESHOLD (60)
#define A2DP_TIMESTAMP_SYNC_SAMPLE_THRESHOLD ((int64_t)a2dp_sample_rate*A2DP_TIMESTAMP_SYNC_TIME_THRESHOLD/1000)

#define RICE_THRESHOLD
#define RICE_THRESHOLD

struct A2DP_TIMESTAMP_INFO_T{
    uint16_t rtp_timestamp;
    uint32_t loc_timestamp;
    uint16_t frame_num;
    int32_t rtp_timestamp_diff_sum;
};

enum A2DP_TIMESTAMP_MODE_T{
    A2DP_TIMESTAMP_MODE_NONE,
    A2DP_TIMESTAMP_MODE_SAMPLE,
    A2DP_TIMESTAMP_MODE_TIME,
};

enum A2DP_TIMESTAMP_MODE_T a2dp_timestamp_mode = A2DP_TIMESTAMP_MODE_NONE;

struct A2DP_TIMESTAMP_INFO_T a2dp_timestamp_pre = {0,0,0};
bool a2dp_timestamp_parser_need_sync = false;

int a2dp_timestamp_parser_init(void)
{
    a2dp_timestamp_mode = A2DP_TIMESTAMP_MODE_NONE;
    a2dp_timestamp_pre.rtp_timestamp = 0;
    a2dp_timestamp_pre.loc_timestamp = 0;
    a2dp_timestamp_pre.frame_num = 0;
    a2dp_timestamp_pre.rtp_timestamp_diff_sum = 0;
    a2dp_timestamp_parser_need_sync = false;
    return 0;
}

int a2dp_timestamp_parser_needsync(void)
{
    a2dp_timestamp_parser_need_sync = true;
    return 0;
}

int a2dp_timestamp_parser_run(uint16_t timestamp, uint16_t framenum)
{
    static int skip_cnt = 0;
    struct A2DP_TIMESTAMP_INFO_T curr_timestamp;
    int skipframe = 0;
    uint16_t rtpdiff;
    int32_t locdiff;
    bool needsave_rtp_timestamp = true;
    bool needsave_loc_timestamp = true;

    curr_timestamp.rtp_timestamp = timestamp;
    curr_timestamp.loc_timestamp = hal_sys_timer_get();
    curr_timestamp.frame_num = framenum;

    switch(a2dp_timestamp_mode) {
        case A2DP_TIMESTAMP_MODE_NONE:

//            DEBUG_INFO(5,"parser rtp:%d loc:%d num:%d prertp:%d preloc:%d\n", curr_timestamp.rtp_timestamp, curr_timestamp.loc_timestamp, curr_timestamp.frame_num,
//                                                   a2dp_timestamp_pre.rtp_timestamp, a2dp_timestamp_pre.loc_timestamp);
            if (a2dp_timestamp_pre.rtp_timestamp){
                locdiff = curr_timestamp.loc_timestamp - a2dp_timestamp_pre.loc_timestamp;
                if (TICKS_TO_MS(locdiff) > A2DP_TIMESTAMP_DEBOUNCE_DURATION){
                    rtpdiff = curr_timestamp.rtp_timestamp - a2dp_timestamp_pre.rtp_timestamp;
                    if (ABS((int16_t)TICKS_TO_MS(locdiff)-rtpdiff)>A2DP_TIMESTAMP_MODE_SAMPLE_THRESHOLD){
                        a2dp_timestamp_mode = A2DP_TIMESTAMP_MODE_SAMPLE;
                        DEBUG_INFO(0,"A2DP_TIMESTAMP_MODE_SAMPLE\n");
                    }else{
                        a2dp_timestamp_mode = A2DP_TIMESTAMP_MODE_TIME;
                        DEBUG_INFO(0,"A2DP_TIMESTAMP_MODE_TIME\n");
                    }
                }else{
                    needsave_rtp_timestamp = false;
                    needsave_loc_timestamp = false;
                }
            }
            break;
        case A2DP_TIMESTAMP_MODE_SAMPLE:
            if (a2dp_timestamp_parser_need_sync){
                skip_cnt++;
                rtpdiff = curr_timestamp.rtp_timestamp - a2dp_timestamp_pre.rtp_timestamp;
                locdiff = curr_timestamp.loc_timestamp - a2dp_timestamp_pre.loc_timestamp;
                a2dp_timestamp_pre.rtp_timestamp_diff_sum += rtpdiff;

                A2DP_TIMESTAMP_TRACE(3,"%d-%d=%d",  curr_timestamp.rtp_timestamp, a2dp_timestamp_pre.rtp_timestamp, rtpdiff);

                A2DP_TIMESTAMP_TRACE(3,"%d-%d=%d",  curr_timestamp.loc_timestamp , a2dp_timestamp_pre.loc_timestamp, locdiff);

                A2DP_TIMESTAMP_TRACE(3,"%d-%d=%d", (int32_t)((int64_t)(TICKS_TO_MS(locdiff))*(uint32_t)a2dp_sample_rate/1000),
                                     a2dp_timestamp_pre.rtp_timestamp_diff_sum,
                                     (int32_t)((TICKS_TO_MS(locdiff)*a2dp_sample_rate/1000) - a2dp_timestamp_pre.rtp_timestamp_diff_sum));


                A2DP_TIMESTAMP_TRACE(2,"A2DP_TIMESTAMP_MODE_SAMPLE SYNC diff:%d cnt:%d\n", (int32_t)((int64_t)(TICKS_TO_MS(locdiff)*a2dp_sample_rate/1000) - a2dp_timestamp_pre.rtp_timestamp_diff_sum), skip_cnt);
                if (((int64_t)(TICKS_TO_MS(locdiff)*a2dp_sample_rate/1000) - a2dp_timestamp_pre.rtp_timestamp_diff_sum) < (int32_t)A2DP_TIMESTAMP_SYNC_SAMPLE_THRESHOLD){
                    DEBUG_INFO(1,"A2DP_TIMESTAMP_MODE_SAMPLE RESYNC OK cnt:%d\n", skip_cnt);
                    skip_cnt = 0;
                    a2dp_timestamp_parser_need_sync = false;
                }else if (skip_cnt > A2DP_TIMESTAMP_SYNC_LIMIT_CNT){
                    DEBUG_INFO(0,"A2DP_TIMESTAMP_MODE_SAMPLE RESYNC FORCE END\n");
                    skip_cnt = 0;
                    a2dp_timestamp_parser_need_sync = false;
                }else{
                    needsave_loc_timestamp = false;
                    skipframe = 1;
                }
            }else{
                a2dp_timestamp_pre.rtp_timestamp_diff_sum = 0;
            }
            break;
        case A2DP_TIMESTAMP_MODE_TIME:
            if (a2dp_timestamp_parser_need_sync){
                skip_cnt++;
                rtpdiff = curr_timestamp.rtp_timestamp - a2dp_timestamp_pre.rtp_timestamp;
                locdiff = curr_timestamp.loc_timestamp - a2dp_timestamp_pre.loc_timestamp;
                a2dp_timestamp_pre.rtp_timestamp_diff_sum += rtpdiff;

                A2DP_TIMESTAMP_TRACE(5,"%d/%d/ %d/%d %d\n",  rtpdiff,a2dp_timestamp_pre.rtp_timestamp_diff_sum,
                                            a2dp_timestamp_pre.loc_timestamp, curr_timestamp.loc_timestamp,
                                            TICKS_TO_MS(locdiff));
                A2DP_TIMESTAMP_TRACE(2,"A2DP_TIMESTAMP_MODE_TIME SYNC diff:%d cnt:%d\n", (int32_t)ABS(TICKS_TO_MS(locdiff) - a2dp_timestamp_pre.rtp_timestamp_diff_sum), skip_cnt);
                if (((int64_t)TICKS_TO_MS(locdiff) - a2dp_timestamp_pre.rtp_timestamp_diff_sum) < A2DP_TIMESTAMP_SYNC_TIME_THRESHOLD){
                    DEBUG_INFO(1,"A2DP_TIMESTAMP_MODE_TIME RESYNC OK cnt:%d\n", skip_cnt);
                    skip_cnt = 0;
                    needsave_loc_timestamp = false;
                    a2dp_timestamp_parser_need_sync = false;
                }else if (skip_cnt > A2DP_TIMESTAMP_SYNC_LIMIT_CNT){
                    DEBUG_INFO(0,"A2DP_TIMESTAMP_MODE_TIME RESYNC FORCE END\n");
                    skip_cnt = 0;
                    a2dp_timestamp_parser_need_sync = false;
                }else{
                    needsave_loc_timestamp = false;
                    skipframe = 1;
                }
            }else{
                a2dp_timestamp_pre.rtp_timestamp_diff_sum = 0;
            }
            break;
    }

    if (needsave_rtp_timestamp){
        a2dp_timestamp_pre.rtp_timestamp = curr_timestamp.rtp_timestamp;
    }

    if (needsave_loc_timestamp){
        a2dp_timestamp_pre.loc_timestamp = curr_timestamp.loc_timestamp;
    }

    return skipframe;
}

#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)
uint8_t lhdc_ext_flags;
#if defined(A2DP_LHDCV5_ON)
//uint8_t lhdcv5_ext_flags;
#define LHDCV5_EXT_FLAGS_HAS_AR          0x01
#define LHDCV5_EXT_FLAGS_HAS_JAS         0x02
#define LHDCV5_EXT_FLAGS_HAS_META        0x04
#define LHDCV5_EXT_FLAGS_LL_MODE         0x40
#define LHDCV5_EXT_FLAGS_LOSSLESS_MODE   0x80
//#define LHDCV5_SET_EXT_FLAGS(X) (lhdcv5_ext_flags |= X)
//#define LHDCV5_CLR_EXT_FLAGS(X) (lhdcv5_ext_flags &= ~X)
//#define LHDCV5_CLR_ALL_EXT_FLAGS() (lhdcv5_ext_flags = 0)
#endif
#define LHDC_EXT_FLAGS_JAS    0x01
#define LHDC_EXT_FLAGS_AR     0x02
#define LHDC_EXT_FLAGS_LLAC   0x04
#define LHDC_EXT_FLAGS_MQA    0x08
#define LHDC_EXT_FLAGS_MBR    0x10
#define LHDC_EXT_FLAGS_LARC   0x20
#define LHDC_EXT_FLAGS_V4     0x40
#define LHDC_SET_EXT_FLAGS(X) (lhdc_ext_flags |= X)
#define LHDC_CLR_EXT_FLAGS(X) (lhdc_ext_flags &= ~X)
#define LHDC_CLR_ALL_EXT_FLAGS() (lhdc_ext_flags = 0)

uint8_t bt_sbc_player_get_bitsDepth(void)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(app_bt_audio_get_curr_a2dp_device());
    return curr_device->sample_bit;
}

void a2dp_lhdc_config(uint8_t device_id, uint8_t * elements)
{
    //uint8_t * elements = &(Info->p.configReq->codec.elements[0]);
    uint32_t vendor_id = (uint32_t)elements[0];
    vendor_id |= ((uint32_t)elements[1]) << 8;
    vendor_id |= ((uint32_t)elements[2]) << 16;
    vendor_id |= ((uint32_t)elements[3]) << 24;
    uint16_t codec_id = (uint16_t)elements[4];
    codec_id |= ((uint16_t)elements[5]) << 8;
    uint8_t config = elements[6];
    DEBUG_INFO(4,"codecType: LHDC Codec, config value=0x%02x, elements[6]=0x%02x [7]=0x%02x [8]=0x%02x\n",
            A2DP_LHDC_SR_DATA(config), elements[6], elements[7], elements[8]);
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
    if (vendor_id == A2DP_LHDC_VENDOR_ID && codec_id == A2DP_LHDC_CODEC_ID) {
        DEBUG_INFO(2,"Vendor ID = 0x%08x, Codec ID = 0x%04x, LHDC Codec\n", vendor_id, codec_id);
        switch (A2DP_LHDC_SR_DATA(config)) {
            case A2DP_LHDC_SR_96000:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_96;
            DEBUG_INFO(1,"%s:CodecCfg sample_rate 96000\n", __func__);
            break;
            case A2DP_LHDC_SR_48000:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_48;
            DEBUG_INFO(1,"%s:CodecCfg sample_rate 48000\n", __func__);
            break;
            case A2DP_LHDC_SR_44100:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_44;
            DEBUG_INFO(1,"%s:CodecCfg sample_rate 44100\n", __func__);
            break;
        }
        switch (A2DP_LHDC_FMT_DATA(config)) {
            case A2DP_LHDC_FMT_16:
            curr_device->sample_bit = 16;
            DEBUG_INFO(1,"%s:CodecCfg bits per sampe = 16", __func__);
            break;
            case A2DP_LHDC_FMT_24:
            DEBUG_INFO(1,"%s:CodecCfg bits per sampe = 24", __func__);
            curr_device->sample_bit = 24;
            break;
        }

        LHDC_CLR_ALL_EXT_FLAGS();

        if (config & A2DP_LHDC_JAS)
            LHDC_SET_EXT_FLAGS(LHDC_EXT_FLAGS_JAS);

        if (config & A2DP_LHDC_AR)
            LHDC_SET_EXT_FLAGS(LHDC_EXT_FLAGS_AR);

        if (elements[7] & A2DP_LHDC_LLAC)
            LHDC_SET_EXT_FLAGS(LHDC_EXT_FLAGS_LLAC);

        if (elements[8] & A2DP_LHDC_MQA)
            LHDC_SET_EXT_FLAGS(LHDC_EXT_FLAGS_MQA);

        if (elements[8] & A2DP_LHDC_MinBR)
            LHDC_SET_EXT_FLAGS(LHDC_EXT_FLAGS_MBR);

        if (elements[8] & A2DP_LHDC_LARC)
            LHDC_SET_EXT_FLAGS(LHDC_EXT_FLAGS_LARC);

        if (elements[8] & A2DP_LHDC_V4)
            LHDC_SET_EXT_FLAGS(LHDC_EXT_FLAGS_V4);

        curr_device->lhdc_ext_flags = lhdc_ext_flags;
        DEBUG_INFO(2,"%s:EXT flags = 0x%08x", __func__, curr_device->lhdc_ext_flags);
        if (elements[7]&A2DP_LHDC_LLC_ENABLE){
            curr_device->a2dp_lhdc_llc = true;
        }else{
            curr_device->a2dp_lhdc_llc = false;
        }
    }
}

#if defined(A2DP_LHDCV5_ON)
void a2dp_lhdcv5_config(uint8_t device_id, uint8_t * elements)
{
    //uint8_t * elements = &(Info->p.configReq->codec.elements[0]);
    uint32_t vendor_id = (uint32_t)elements[0];
    vendor_id |= ((uint32_t)elements[1]) << 8;
    vendor_id |= ((uint32_t)elements[2]) << 16;
    vendor_id |= ((uint32_t)elements[3]) << 24;
    uint16_t codec_id = (uint16_t)elements[4];
    codec_id |= ((uint16_t)elements[5]) << 8;
    uint8_t config = elements[6];

// [SVNY] Need to check capability max size
    //DEBUG_INFO(6,"codecType: LHDC Codec, config value=0x%02x, elements[6]=0x%02x [7]=0x%02x [8]=0x%02x [9]=0x%02x [10]=0x%02x\n",
    //        A2DP_LHDCV5_SR_DATA(config), elements[6], elements[7], elements[8], elements[9], elements[10]);
    DEBUG_INFO(5,"codecType: LHDC Codec, config value=0x%02x, elements[6]=0x%02x [7]=0x%02x [8]=0x%02x [9]=0x%02x\n",
            A2DP_LHDCV5_SR_DATA(config), elements[6], elements[7], elements[8], elements[9]);

    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);

    if (vendor_id == A2DP_LHDCV5_VENDOR_ID && codec_id == A2DP_LHDCV5_CODEC_ID){
        DEBUG_INFO(2,"Vendor ID = 0x%08x, Codec ID = 0x%04x, LHDC Codec\n", vendor_id, codec_id);
        switch (A2DP_LHDCV5_SR_DATA(config)) {
//#if 0
            case A2DP_LHDCV5_SR_192000:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_192;
            DEBUG_INFO(1,"%s:CodecCfg sample_rate 192000\n", __func__);
            break;
//#endif
            case A2DP_LHDCV5_SR_96000:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_96;
            DEBUG_INFO(1,"%s:CodecCfg sample_rate 96000\n", __func__);
            break;
            case A2DP_LHDCV5_SR_48000:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_48;
            DEBUG_INFO(1,"%s:CodecCfg sample_rate 48000\n", __func__);
            break;
            case A2DP_LHDCV5_SR_44100:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_44;
            DEBUG_INFO(1,"%s:CodecCfg sample_rate 44100\n", __func__);
            break;
        }
        config = elements[7];
        switch (A2DP_LHDCV5_FMT_DATA(config)) {
            case A2DP_LHDCV5_FMT_16:
            curr_device->sample_bit = 16;
            DEBUG_INFO(1,"%s:CodecCfg bits per sampe = 16", __func__);
            break;
            case A2DP_LHDCV5_FMT_24:
            DEBUG_INFO(1,"%s:CodecCfg bits per sampe = 24", __func__);
            curr_device->sample_bit = 24;
            break;
//#if 0
            case A2DP_LHDCV5_FMT_32:
            DEBUG_INFO(1,"%s:CodecCfg bits per sampe = 32", __func__);
            curr_device->sample_bit = 32;
            break;
//#endif
        }

        LHDC_CLR_ALL_EXT_FLAGS();
#if 1
        if (elements[9] & A2DP_LHDCV5_HAS_AR)
            LHDC_SET_EXT_FLAGS(LHDCV5_EXT_FLAGS_HAS_AR);
        if (elements[9] & A2DP_LHDCV5_HAS_JAS)
            LHDC_SET_EXT_FLAGS(LHDCV5_EXT_FLAGS_HAS_JAS);
        if (elements[9] & A2DP_LHDCV5_HAS_META)
            LHDC_SET_EXT_FLAGS(LHDCV5_EXT_FLAGS_HAS_META);
        if (elements[9] & A2DP_LHDCV5_LL_MODE){
            LHDC_SET_EXT_FLAGS(LHDCV5_EXT_FLAGS_LL_MODE);
            curr_device->a2dp_lhdc_llc = true;
        }
        else{
            curr_device->a2dp_lhdc_llc = false;
        }
        if (elements[9] & A2DP_LHDCV5_LOSSLESS_MODE)
            LHDC_SET_EXT_FLAGS(LHDCV5_EXT_FLAGS_LOSSLESS_MODE);

//        if (elements[10] & A2DP_LHDC_AR_ON)
//            LHDC_SET_EXT_FLAGS(LHDC_EXT_FLAGS_AR_ON);
#endif
        curr_device->lhdc_ext_flags = lhdc_ext_flags;
        DEBUG_INFO(2,"%s:EXT flags = 0x%08x", __func__, curr_device->lhdc_ext_flags);
    }
}
#endif

uint8_t a2dp_lhdc_config_llc_get(void)
{
    uint8_t device_id = app_bt_audio_get_curr_a2dp_device();
    return app_bt_get_device(device_id)->a2dp_lhdc_llc;
}

uint32_t a2dp_lhdc_config_bitrate_get(void)
{
    return 400000;
}

//TODO
extern "C" bool a2dp_lhdc_get_ext_flags(uint32_t flags)
{
    uint8_t device_id = app_bt_audio_get_curr_a2dp_device();
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
    DEBUG_INFO(3,"%s:EXT flags = 0x%08x, flag = 0x%02x", __func__, curr_device->lhdc_ext_flags, (uint8_t)flags);
    return ((curr_device->lhdc_ext_flags & flags) == flags);
}

uint8_t bt_lhdc_player_get_ext_flags(void)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(app_bt_audio_get_curr_a2dp_device());
     if (curr_device->lhdc_ext_flags != lhdc_ext_flags) {
        /* code */
        lhdc_ext_flags = curr_device->lhdc_ext_flags;
    }
    return lhdc_ext_flags;
}

#endif

uint8_t a2dp_get_non_type_by_device_id(uint8_t device_id)
{
    return app_bt_get_device(device_id)->a2dp_non_type;
}

#if defined(A2DP_SCALABLE_ON)
void a2dp_scalable_config(uint8_t device_id, uint8_t * elements){
    uint32_t vendor_id = (uint32_t)elements[0];
    vendor_id |= ((uint32_t)elements[1]) << 8;
    vendor_id |= ((uint32_t)elements[2]) << 16;
    vendor_id |= ((uint32_t)elements[3]) << 24;
    uint16_t codec_id = (uint16_t)elements[4];
    codec_id |= ((uint16_t)elements[5]) << 8;
    uint8_t config = elements[6];
    DEBUG_INFO(2,"##codecType: Scalable Codec, config value = 0x%02x, elements[6]=0x%02x\n", A2DP_SCALABLE_SR_DATA(config), elements[6]);
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
    if (vendor_id == A2DP_SCALABLE_VENDOR_ID && codec_id == A2DP_SCALABLE_CODEC_ID) {
        DEBUG_INFO(2,"Vendor ID = 0x%08x, Codec ID = 0x%04x, Scalable Codec\n", vendor_id, codec_id);
        switch (A2DP_SCALABLE_SR_DATA(config)) {
            case A2DP_SCALABLE_SR_96000:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_96;
            DEBUG_INFO(1,"%s:CodecCfg sample_rate 96000\n", __func__);
            break;
            case A2DP_SCALABLE_SR_48000:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_48;
            DEBUG_INFO(1,"%s:CodecCfg sample_rate 48000\n", __func__);
            break;
            case A2DP_SCALABLE_SR_44100:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_44;
            DEBUG_INFO(1,"%s:CodecCfg sample_rate 44100\n", __func__);
            break;
            case A2DP_SCALABLE_SR_32000:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_32;
            DEBUG_INFO(1,"%s:CodecCfg sample_rate 32000\n", __func__);
            break;
        }
        switch (A2DP_SCALABLE_FMT_DATA(config)) {
            case A2DP_SCALABLE_FMT_16:
            curr_device->sample_bit = 16;
            DEBUG_INFO(1,"%s:CodecCfg bits per sampe = 16", __func__);
            break;
            case A2DP_SCALABLE_FMT_24:
            curr_device->sample_bit = 24;
            DEBUG_INFO(1,"%s:CodecCfg bits per sampe = 24", __func__);
            if (curr_device->sample_rate != A2D_SBC_IE_SAMP_FREQ_96) {
                curr_device->sample_bit = 16;
                DEBUG_INFO(1,"%s:CodeCfg reset bit per sample to 16 when samplerate is not 96k", __func__);
            }
            break;
        }
    }
}
#endif

#if defined(A2DP_LDAC_ON)
void a2dp_ldac_config(uint8_t device_id, uint8_t * elements){
    //uint8_t * elements = &(Info->p.configReq->codec.elements[0]);
    uint32_t vendor_id = (uint32_t)elements[0];
    vendor_id |= ((uint32_t)elements[1]) << 8;
    vendor_id |= ((uint32_t)elements[2]) << 16;
    vendor_id |= ((uint32_t)elements[3]) << 24;
    uint16_t codec_id = (uint16_t)elements[4];
    codec_id |= ((uint16_t)elements[5]) << 8;
    uint8_t sf_config = elements[6];
    uint8_t cm_config = elements[7];
    DEBUG_INFO(2,"##codecType: LDAC Codec, config value = 0x%02x, elements[6]=0x%02x\n", A2DP_LDAC_SR_DATA(sf_config), elements[6]);
    DEBUG_INFO(2,"##codecType: LDAC Codec, config value = 0x%02x, elements[7]=0x%02x\n", A2DP_LDAC_CM_DATA(cm_config), elements[7]);
    DEBUG_INFO(2,"Vendor ID = 0x%08x, Codec ID = 0x%04x, LDAC Codec\n", vendor_id, codec_id);
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
    if (vendor_id == A2DP_LDAC_VENDOR_ID && codec_id == A2DP_LDAC_CODEC_ID) {
        switch (A2DP_LDAC_SR_DATA(sf_config)) {
            case A2DP_LDAC_SR_96000:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_96;
            DEBUG_INFO(1,"%s:ldac CodecCfg sample_rate 96000\n", __func__);
            break;
            case A2DP_LDAC_SR_88200:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_88D2;
            DEBUG_INFO(1,"%s:ldac CodecCfg sample_rate 88200\n", __func__);
            break;
            case A2DP_LDAC_SR_48000:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_48;
            DEBUG_INFO(1,"%s:ldac CodecCfg sample_rate 48000\n", __func__);
            break;
            case A2DP_LDAC_SR_44100:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_44;
            DEBUG_INFO(1,"%sldac :CodecCfg sample_rate 44100\n", __func__);
            break;
        }
        switch (A2DP_LDAC_CM_DATA(cm_config)) {
            case A2DP_LDAC_CM_MONO:
            curr_device->channel_mode = LDACBT_CHANNEL_MODE_MONO;
            DEBUG_INFO(1,"%s:ldac CodecCfg A2DP_LDAC_CM_MONO", __func__);
            break;
            case A2DP_LDAC_CM_DUAL:
            DEBUG_INFO(1,"%s:ldac CodecCfg A2DP_LDAC_CM_DUAL", __func__);
            curr_device->channel_mode = LDACBT_CHANNEL_MODE_DUAL_CHANNEL;
            break;
            case A2DP_LDAC_CM_STEREO:
            DEBUG_INFO(1,"%s:ldac ldac CodecCfg A2DP_LDAC_CM_STEREO", __func__);
            curr_device->channel_mode = LDACBT_CHANNEL_MODE_STEREO;
            break;
        }
    }
}
#endif

#if defined(A2DP_LC3_ON)
void a2dp_lc3_config(uint8_t device_id, uint8_t * elements)
{

#if defined(A2DP_LC3_HR)
    //uint8_t * elements = &(Info->p.configReq->codec.elements[0]);
    uint32_t vendor_id = (uint32_t)elements[0];
    vendor_id |= ((uint32_t)elements[1]) << 8;
    vendor_id |= ((uint32_t)elements[2]) << 16;
    vendor_id |= ((uint32_t)elements[3]) << 24;
    uint16_t codec_id = (uint16_t)elements[4];
    codec_id |= ((uint16_t)elements[5]) << 8;
    uint8_t config1 = elements[8]|elements[9];
    uint8_t config2 = elements[10];
    uint8_t config3 = elements[6];
    uint8_t config4 = elements[7];

    DEBUG_INFO(3,"##codecType: LC3 Codec, config value = 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
        elements[6], elements[7], elements[8], elements[9]);

    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);


    if ((vendor_id == A2DP_LC3_VENDOR_ID) && ((codec_id == A2DP_LC3_CODEC_ID) || (codec_id == A2DP_LC3_PLUS_CODEC_ID))) {
        DEBUG_INFO(2,"Vendor ID = 0x%08x, Codec ID = 0x%04x, LC3 Codec\n", vendor_id, codec_id);
        switch (A2DP_LC3_SR_DATA(config1)) {
            case A2DP_LC3_SR_96000:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_96;
            DEBUG_INFO(1,"%s:CodecCfg sample_rate 96000\n", __func__);
            break;
            case A2DP_LC3_SR_48000:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_48;
            DEBUG_INFO(1,"%s:CodecCfg sample_rate 48000\n", __func__);
            break;
        }

        switch (A2DP_LC3_FMT_DATA(config2)) {
            case A2DP_LC3_FMT_16BIT:
            curr_device->sample_bit = 16;
            DEBUG_INFO(1,"%s:CodecCfg bits per sample = 16", __func__);
            break;
            case A2DP_LC3_FMT_24BIT:
            curr_device->sample_bit = 24;
            DEBUG_INFO(1,"%s:CodecCfg bits per sample = 24", __func__);
            break;
            case A2DP_LC3_FMT_32BIT:
            curr_device->sample_bit = 32;
            DEBUG_INFO(1,"%s:CodecCfg bits per sample = 32", __func__);
            break;
        }

        switch (A2DP_LC3_FRAME_LEN_DATA(config3)) {
            case A2DP_LC3_FRAME_LEN_10MS:
            curr_device->a2dp_frame_dr = 100;
            DEBUG_INFO(1,"%s:CodecCfg frame len = 10ms", __func__);
            break;
            case A2DP_LC3_FRAME_LEN_5MS:
            curr_device->a2dp_frame_dr= 50;
            DEBUG_INFO(1,"%s:CodecCfg frame len = 5ms", __func__);
            break;
            case A2DP_LC3_FRAME_LEN_2POINT5MS:
            curr_device->a2dp_frame_dr = 25;
            DEBUG_INFO(1,"%s:CodecCfg frame len = 2.5ms", __func__);
            break;
        }

        switch (A2DP_LC3_CH_MD_DATA(config4)) {
            case A2DP_LC3_CH_MD_MONO:
            curr_device->channel_mode = 1;
            DEBUG_INFO(1,"%s:CodecCfg channel num = 1", __func__);
            break;
            case A2DP_LC3_CH_MD_STEREO:
            curr_device->channel_mode = 2;
            DEBUG_INFO(1,"%s:CodecCfg channel num = 2", __func__);
            break;
        }
    }

#else
    //uint8_t * elements = &(Info->p.configReq->codec.elements[0]);
    uint32_t vendor_id = (uint32_t)elements[0];
    vendor_id |= ((uint32_t)elements[1]) << 8;
    vendor_id |= ((uint32_t)elements[2]) << 16;
    vendor_id |= ((uint32_t)elements[3]) << 24;
    uint16_t codec_id = (uint16_t)elements[4];
    codec_id |= ((uint16_t)elements[5]) << 8;
    uint8_t config1 = elements[6];
    uint8_t config2 = elements[7];
    uint8_t config3 = elements[8];
    uint8_t config4 = elements[9];

    DEBUG_INFO(3,"##codecType: LC3 Codec, config value = 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
        elements[6], elements[7], elements[8], elements[9]);

    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);


    if ((vendor_id == A2DP_LC3_VENDOR_ID) && ((codec_id == A2DP_LC3_CODEC_ID) || (codec_id == A2DP_LC3_PLUS_CODEC_ID))) {
        DEBUG_INFO(2,"Vendor ID = 0x%08x, Codec ID = 0x%04x, LC3 Codec\n", vendor_id, codec_id);
        switch (A2DP_LC3_SR_DATA(config1)) {
            case A2DP_LC3_SR_96000:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_96;
            DEBUG_INFO(1,"%s:CodecCfg sample_rate 96000\n", __func__);
            break;
            case A2DP_LC3_SR_48000:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_48;
            DEBUG_INFO(1,"%s:CodecCfg sample_rate 48000\n", __func__);
            break;
            case A2DP_LC3_SR_44100:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_44;
            DEBUG_INFO(1,"%s:CodecCfg sample_rate 44100\n", __func__);
            break;
            case A2DP_LC3_SR_32000:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_32;
            DEBUG_INFO(1,"%s:CodecCfg sample_rate 3200\n", __func__);
            break;
            case A2DP_LC3_SR_16000:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_16;
            DEBUG_INFO(1,"%s:CodecCfg sample_rate 16000\n", __func__);
            break;
        }

        switch (A2DP_LC3_FMT_DATA(config2)) {
            case A2DP_LC3_FMT_16BIT:
            curr_device->sample_bit = 16;
            DEBUG_INFO(1,"%s:CodecCfg bits per sample = 16", __func__);
            break;
            case A2DP_LC3_FMT_24BIT:
            curr_device->sample_bit = 24;
            DEBUG_INFO(1,"%s:CodecCfg bits per sample = 24", __func__);
            break;
            case A2DP_LC3_FMT_32BIT:
            curr_device->sample_bit = 32;
            DEBUG_INFO(1,"%s:CodecCfg bits per sample = 32", __func__);
            break;
        }

        switch (A2DP_LC3_FRAME_LEN_DATA(config2)) {
            case A2DP_LC3_FRAME_LEN_10MS:
            curr_device->a2dp_frame_dr = 100;
            DEBUG_INFO(1,"%s:CodecCfg frame len = 10ms", __func__);
            break;
            case A2DP_LC3_FRAME_LEN_7POINT5MS:
            curr_device->a2dp_frame_dr = 75;
            DEBUG_INFO(1,"%s:CodecCfg frame len = 7.5ms", __func__);
            break;
            case A2DP_LC3_FRAME_LEN_5MS:
            curr_device->a2dp_frame_dr= 50;
            DEBUG_INFO(1,"%s:CodecCfg frame len = 5ms", __func__);
            break;
            case A2DP_LC3_FRAME_LEN_2POINT5MS:
            curr_device->a2dp_frame_dr = 25;
            DEBUG_INFO(1,"%s:CodecCfg frame len = 2.5ms", __func__);
            break;
        }

        switch (A2DP_LC3_BITRATE_DATA(config3))
        {
            case A2DP_LC3_BITRATE_900kBPS:
            curr_device->a2dp_lc3_bitrate= 900;
            DEBUG_INFO(1,"%s:CodecCfg bitrate = 900kbps", __func__);
            break;
            case A2DP_LC3_BITRATE_600kBPS:
            curr_device->a2dp_lc3_bitrate = 600;
            DEBUG_INFO(1,"%s:CodecCfg bitrate = 600kbps", __func__);
            break;
            case A2DP_LC3_BITRATE_500kBPS:
            curr_device->a2dp_lc3_bitrate = 500;
            DEBUG_INFO(1,"%s:CodecCfg bitrate = 500kbps", __func__);
            break;
            case A2DP_LC3_BITRATE_400kBPS:
            curr_device->a2dp_lc3_bitrate= 400;
            DEBUG_INFO(1,"%s:CodecCfg bitrate = 400kbps", __func__);
            break;
            case A2DP_LC3_BITRATE_300kBPS:
            curr_device->a2dp_lc3_bitrate= 300;
            DEBUG_INFO(1,"%s:CodecCfg bitrate = 300kbps", __func__);
            break;
            case A2DP_LC3_BITRATE_200kBPS:
            curr_device->a2dp_lc3_bitrate = 200;
            DEBUG_INFO(1,"%s:CodecCfg bitrate = 200kbps", __func__);
            break;
            case A2DP_LC3_BITRATE_100kBPS:
            curr_device->a2dp_lc3_bitrate= 100;
            DEBUG_INFO(1,"%s:CodecCfg bitrate = 100kbps", __func__);
            break;
            case A2DP_LC3_BITRATE_64kBPS:
            curr_device->a2dp_lc3_bitrate= 64;
            DEBUG_INFO(1,"%s:CodecCfg bitrate = 64kbps", __func__);
            break;
        }

        switch (A2DP_LC3_CH_MD_DATA(config4)) {
            case A2DP_LC3_CH_MD_MONO:
            curr_device->channel_mode = 1;
            DEBUG_INFO(1,"%s:CodecCfg channel num = 1", __func__);
            break;
            case A2DP_LC3_CH_MD_STEREO:
            curr_device->channel_mode = 2;
            DEBUG_INFO(1,"%s:CodecCfg channel num = 2", __func__);
            break;
            case A2DP_LC3_CH_MD_MUlTI_MONO:
            curr_device->channel_mode = 2;
            DEBUG_INFO(1,"%s:CodecCfg channel num = 2", __func__);
            break;
        }
    }

#endif
}

uint8_t bt_get_lc3_frame_dms(void)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(app_bt_audio_get_curr_a2dp_device());
    return curr_device->a2dp_frame_dr;
}

uint8_t bt_get_lc3_bitrate(void)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(app_bt_audio_get_curr_a2dp_device());
    return curr_device->a2dp_lc3_bitrate;
}
#endif

#if defined(A2DP_L2HC_ON)
void a2dp_l2hc_config(uint8_t device_id, uint8_t * elements)
{
    uint32_t vendor_id = (uint32_t)elements[0];
    vendor_id |= ((uint32_t)elements[1]) << 8;
    vendor_id |= ((uint32_t)elements[2]) << 16;
    vendor_id |= ((uint32_t)elements[3]) << 24;
    uint16_t codec_id = (uint16_t)elements[4];
    codec_id |= ((uint16_t)elements[5]) << 8;

    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);

    uint8_t res = elements[6] & 0x07;
    switch (A2DP_L2HC_RES_DATA(res))
    {
    case A2DP_L2HC_RES_32:
        curr_device->sample_bit = 32;
        break;
    case A2DP_L2HC_RES_24:
        curr_device->sample_bit = 24;
        break;
    case A2DP_L2HC_RES_16:
        curr_device->sample_bit = 16;
        break;
    default:
        DEBUG_WARNING(0, "sample_bit error 0x%x, need check", elements[6]);
        curr_device->sample_bit = 16;
        break;
    }
    DEBUG_INFO(0, "%s d(%d) sample_bit %d", __func__, device_id, curr_device->sample_bit);

    uint8_t sample_rate = elements[7] & 0x7F;
    switch (A2DP_L2HC_SR_DATA(sample_rate))
    {
    case A2DP_L2HC_SR_192000:
        curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_192;
        break;
    case A2DP_L2HC_SR_176400:
        curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_176D4;
        break;
    case A2DP_L2HC_SR_96000:
        curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_96;
        break;
    case A2DP_L2HC_SR_88200:
        curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_88D2;
        break;
    case A2DP_L2HC_SR_48000:
        curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_48;
        break;
    case A2DP_L2HC_SR_44100:
        curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_44;
        break;
    case A2DP_L2HC_SR_32000:
        curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_32;
        break;
    default:
        DEBUG_WARNING(0, "sample_rate error 0x%x, need check", elements[7]);
        curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_32;
        break;
    }
    DEBUG_INFO(0, "%s d(%d) sr %d", __func__, device_id, curr_device->sample_rate);

    uint8_t br_msb = elements[8] & 0x7F;
    uint8_t br_lsb = elements[9] & 0xF8;
    if (br_msb || br_lsb)
    {
        if (br_msb & A2DP_L2HC_BR_1920)
        {
            curr_device->l2hc_bitrate |= A2DP_L2HC_BR_REMAP_1920;
        }
        if (br_msb & A2DP_L2HC_BR_1600)
        {
            curr_device->l2hc_bitrate |= A2DP_L2HC_BR_REMAP_1600;
        }
        if (br_msb & A2DP_L2HC_BR_1280)
        {
            curr_device->l2hc_bitrate |= A2DP_L2HC_BR_REMAP_1280;
        }
        if (br_msb & A2DP_L2HC_BR_960)
        {
            curr_device->l2hc_bitrate |= A2DP_L2HC_BR_REMAP_960;
        }
        if (br_msb & A2DP_L2HC_BR_640)
        {
            curr_device->l2hc_bitrate |= A2DP_L2HC_BR_REMAP_640;
        }
        if (br_msb & A2DP_L2HC_BR_480)
        {
            curr_device->l2hc_bitrate |= A2DP_L2HC_BR_REMAP_480;
        }
        if (br_msb & A2DP_L2HC_BR_320)
        {
            curr_device->l2hc_bitrate |= A2DP_L2HC_BR_REMAP_320;
        }

        if (br_lsb & A2DP_L2HC_BR_256)
        {
            curr_device->l2hc_bitrate |= A2DP_L2HC_BR_REMAP_256;
        }
        if (br_lsb & A2DP_L2HC_BR_192)
        {
            curr_device->l2hc_bitrate |= A2DP_L2HC_BR_REMAP_192;
        }
        if (br_lsb & A2DP_L2HC_BR_128)
        {
            curr_device->l2hc_bitrate |= A2DP_L2HC_BR_REMAP_128;
        }
        if (br_lsb & A2DP_L2HC_BR_96)
        {
            curr_device->l2hc_bitrate |= A2DP_L2HC_BR_REMAP_96;
        }
        if (br_lsb & A2DP_L2HC_BR_64)
        {
            curr_device->l2hc_bitrate |= A2DP_L2HC_BR_REMAP_64;
        }
    }
    else
    {
        DEBUG_WARNING(0, "bitrate error 0x%x 0x%x, need check", elements[8], elements[9]);
    }

    DEBUG_INFO(0, "%s d(%d) br %d", __func__, device_id, curr_device->l2hc_bitrate);

    uint8_t fd_msb = elements[9] & 0x03;
    uint8_t fd_lsb = elements[10] & 0x80;
    if (fd_msb)
    {
        switch (A2DP_L2HC_FD_MSB_DATA(fd_msb))
        {
        case A2DP_L2HC_FRAME_10MS:
            curr_device->a2dp_frame_dr = 100;
            break;
        case A2DP_L2HC_FRAME_7D5MS:
            curr_device->a2dp_frame_dr = 75;
            break;
        default:
            DEBUG_WARNING(0, "fd_msb error 0x%x, need check", elements[9]);
            break;
        }
    }
    else if (fd_lsb)
    {
        if (A2DP_L2HC_FD_LSB_DATA(fd_lsb))
        {
            curr_device->a2dp_frame_dr = 50;
        }
    }
    else
    {
        curr_device->a2dp_frame_dr = 50;
        DEBUG_WARNING(0, "fd_msb fd_lsb error 0x%x 0x%x, need check", elements[9], elements[10]);
    }
    DEBUG_INFO(0, "%s d(%d) br %d", __func__, device_id, curr_device->l2hc_bitrate);

    uint8_t chnnel_mode = elements[10] & 0x0C;
    if (chnnel_mode)
    {
        switch (A2DP_L2HC_CM_DATA(chnnel_mode))
        {
        case A2DP_L2HC_CM_DUAL:
            curr_device->channel_mode = A2DP_L2HC_CM_DUAL;
            break;
        case A2DP_L2HC_CM_MONO:
            curr_device->channel_mode = A2DP_L2HC_CM_MONO;
            break;
        default:
            DEBUG_WARNING(0, "chnnel_mode error 0x%x, need check", elements[10]);
            break;
        }
    }
    else
    {
        curr_device->channel_mode = A2DP_L2HC_CM_DUAL;
    }
}
#endif // A2DP_L2HC_ON

#if defined(A2DP_MIHC_ON)
void a2dp_mihc_config(uint8_t device_id, uint8_t *elements)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);

    uint32_t vendor_id = (uint32_t)elements[0];
    vendor_id |= ((uint32_t)elements[1]) << 8;
    vendor_id |= ((uint32_t)elements[2]) << 16;
    vendor_id |= ((uint32_t)elements[3]) << 24;

    uint16_t codec_id = (uint16_t)elements[4];
    codec_id |= ((uint16_t)elements[5]) << 8;

    uint8_t version = elements[6];

    uint8_t sample_rate = A2DP_MIHC_SR_DATA(elements[7]);
    switch (sample_rate)
    {
        case A2DP_MIHC_SR_48000:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_48;
            break;
        case A2DP_MIHC_SR_44100:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_44;
            break;
        case A2DP_MIHC_SR_32000:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_32;
            break;
        case A2DP_MIHC_SR_24000:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_24;
            break;
        case A2DP_MIHC_SR_16000:
            curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_16;
            break;
        default:
            DEBUG_ERROR(0, "sample_rate error 0x%x, need check", elements[7]);
            break;
    }

    bool cbr = (elements[8] & A2DP_MIHC_CBR);

    uint8_t frame_duration = A2DP_MIHC_FD_DATA(elements[8]);
    switch (frame_duration)
    {
        case A2DP_MIHC_FD_10MS:
            curr_device->a2dp_frame_dr = 100;
            break;
        case A2DP_MIHC_FD_5MS:
            curr_device->a2dp_frame_dr = 50;
            break;
        default:
            DEBUG_ERROR(0, "frame_duration error 0x%x, need check", elements[8]);
            break;
    }

    uint8_t bit_deepth = A2DP_MIHC_BD_DATA(elements[8]);
    switch (bit_deepth)
    {
        case A2DP_MIHC_BD_32:
            curr_device->sample_bit = 32;
            break;
        case A2DP_MIHC_BD_24:
            curr_device->sample_bit = 24;
            break;
        case A2DP_MIHC_BD_16:
            curr_device->sample_bit = 16;
            break;
        default:
            DEBUG_ERROR(0, "bit_deepth error 0x%x, need check", elements[8]);
            break;
    }

    uint8_t channel_mode = A2DP_MIHC_CM_DATA(elements[9]);
    switch (channel_mode)
    {
        case A2DP_MIHC_CM_STEREO:
            curr_device->channel_mode = A2DP_MIHC_CM_STEREO;
            curr_device->a2dp_channel_num = 2;
            break;
        case A2DP_MIHC_CM_MONO:
            curr_device->channel_mode = A2DP_MIHC_CM_MONO;
            curr_device->a2dp_channel_num = 1;
            break;
        default:
            DEBUG_ERROR(0, "channel_mode error 0x%x, need check", elements[9]);
            break;
    }

    uint8_t mode = A2DP_MIHC_MODE_DATA(elements[9]);

    uint8_t min_bitrate = elements[10];
    switch (min_bitrate)
    {
        case A2DP_MIHC_MIN_BR_1280:
        case A2DP_MIHC_MIN_BR_960:
        case A2DP_MIHC_MIN_BR_640:
        case A2DP_MIHC_MIN_BR_320:
            // curr_device->mihc_min_bitrate = min_bitrate;
            break;
        default:
            DEBUG_ERROR(0, "min_bitrate error 0x%x, need check", elements[10]);
            break;
    }

    bool md = (A2DP_MIHC_MD & elements[11]);

    DEBUG_INFO(0, "d(%d)%s info: ", device_id, __func__);
    DEBUG_INFO(0, "vendor_id=%04x, codec_id=%02x, version=%x, sample_rate=%x, cbr=%d, frame_duration=%x",
                    vendor_id, codec_id, version, sample_rate, cbr, frame_duration);
    DEBUG_INFO(0, "bit_deepth=%x, channel_mode=%x, mode=%x, min_bitrate=%x, md=%d",
                    bit_deepth, channel_mode, mode, min_bitrate, md);
}
#endif /* A2DP_MIHC_ON */

#if defined(IBRT)
a2dp_stream_t * app_bt_get_mobile_a2dp_stream(uint32_t deviceId);
int app_bt_stream_ibrt_audio_mismatch_stopaudio(uint8_t device_id);
void app_bt_stream_ibrt_audio_mismatch_resume(uint8_t device_id);

int a2dp_ibrt_session_reset(uint8_t devId)
{
    app_bt_get_device(devId)->a2dp_session = 0;
    return 0;
}

int a2dp_ibrt_session_new(uint8_t devId)
{
    app_bt_get_device(devId)->a2dp_session++;
    return 0;
}

int a2dp_ibrt_session_set(uint8_t session,uint8_t devId)
{
    DEBUG_INFO(3, "d%x %s,session:%d", devId, __func__, session);
    app_bt_get_device(devId)->a2dp_session = session;
    return 0;
}

uint32_t a2dp_ibrt_session_get(uint8_t devId)
{
    return app_bt_get_device(devId)->a2dp_session;
}

static int a2dp_ibrt_autotrigger_flag = 0;
int a2dp_ibrt_stream_need_autotrigger_set_flag(void)
{
    a2dp_ibrt_autotrigger_flag = 1;
    return 0;
}

int a2dp_bt_stream_need_autotrigger_getandclean_flag(void)
{
    uint32_t flag;
#if defined(IBRT_A2DP_TRIGGER_BY_MYSELF)
    flag = a2dp_ibrt_autotrigger_flag;
    a2dp_ibrt_autotrigger_flag = 0;
#else
    a2dp_ibrt_autotrigger_flag = 0;
    flag = 1;
#endif
    return flag;
}

int a2dp_ibrt_sync_get_status(uint8_t device_id, ibrt_a2dp_status_t *a2dp_status)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_ibrt_if_get_bt_ctrl_ctx();
#if defined(IBRT_UI)
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    ibrt_mobile_info_t *mobile_info = app_ibrt_conn_get_mobile_info_by_addr(&curr_device->remote);
#endif

    a2dp_status->codec           = p_ibrt_ctrl->a2dp_codec;
    a2dp_status->localVolume     = a2dp_volume_local_get((enum BT_DEVICE_ID_T)device_id);
    a2dp_status->state           = btif_a2dp_get_stream_state(app_bt_get_mobile_a2dp_stream(device_id));
#ifndef BTH_SUBSYS_ONLY
    a2dp_status->latency_factor  = a2dp_audio_latency_factor_get();
#else
    a2dp_status->latency_factor  = 0;
#endif
    a2dp_status->session         = a2dp_ibrt_session_get(device_id);

#if defined(IBRT_UI)
    a2dp_status->avrcp_play_status = app_bt_get_device(device_id)->avrcp_playback_status;

    if (mobile_info != NULL)
    {
        a2dp_status->codec       = mobile_info->a2dp_codec;
    }
    else
    {
        DEBUG_INFO(1,"%s,mobile info null",__func__);
    }
#endif

    DEBUG_INFO(4,"%s,sync a2dp stream ac = %p ; stream_status = %d ; codec_type =  %d ",__func__,app_bt_get_device(device_id)->a2dp_connected_stream,a2dp_status->state,a2dp_status->codec.codec_type);
    return 0;
}

int a2dp_ibrt_sync_set_status(uint8_t device_id,ibrt_a2dp_status_t *a2dp_status)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    a2dp_stream_t *Stream = curr_device->btif_a2dp_stream->a2dp_stream;
    bt_a2dp_stream_state_t old_avdtp_stream_state = btif_a2dp_get_stream_state(Stream);
    btif_a2dp_callback_parms_t info;
    btif_avdtp_config_request_t avdtp_config_req;

    DEBUG_INFO(5,"%s,stream_state:[%d]->[%d], codec_type:%d localVolume:%d, recheck:0x%x",
                    __func__, old_avdtp_stream_state, a2dp_status->state,
                    a2dp_status->codec.codec_type, a2dp_status->localVolume, curr_device->a2dp_status_recheck);

    if ((!(app_ibrt_conn_get_mobile_constate(&curr_device->remote) & BTIF_APP_A2DP_PROFILE_ID)) &&
        (!(app_ibrt_conn_get_ibrt_constate(&curr_device->remote) & BTIF_APP_A2DP_PROFILE_ID)))
    {
        DEBUG_INFO(1,"%s,a2dp profile not connected",__func__);
        return 1;
    }
    if(curr_device->a2dp_status_recheck & A2DP_STATUS_SYNC_CODEC_STATE)
    {
        curr_device->a2dp_status_recheck = curr_device->a2dp_status_recheck & (~A2DP_STATUS_SYNC_CODEC_STATE);
        btif_a2dp_set_codec_info(device_id, (uint8_t*)&a2dp_status->codec);
    }
    info.event = BTIF_A2DP_EVENT_STREAM_OPEN_MOCK;

    app_tws_ibrt_set_a2dp_codec_v2(&curr_device->remote, (a2dp_callback_parms_t*)&info);

    audio_player_restore_volume((uint8_t *)&curr_device->remote);

    a2dp_volume_set_local_vol(device_id, a2dp_status->localVolume);
#ifndef BTH_SUBSYS_ONLY
    a2dp_audio_latency_factor_set(a2dp_status->latency_factor);
#endif
    a2dp_ibrt_session_set(a2dp_status->session,device_id);

#if defined(IBRT_UI)
    if (curr_device->avrcp_playback_status != a2dp_status->avrcp_play_status &&
	     (curr_device->a2dp_status_recheck & A2DP_STATUS_SYNC_AVRCP_STATE) &&
	     curr_device->avrcp_conn_flag)
    {
        curr_device->a2dp_status_recheck = curr_device->a2dp_status_recheck & (~A2DP_STATUS_SYNC_AVRCP_STATE);
        curr_device->avrcp_playback_status = a2dp_status->avrcp_play_status;
        bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_AVRCP_PLAY_STATUS_MOCK, curr_device->avrcp_playback_status);
    }
#endif

    if (a2dp_status->state != old_avdtp_stream_state && (curr_device->a2dp_status_recheck & A2DP_STATUS_SYNC_A2DP_STATE)){
        curr_device->a2dp_status_recheck = curr_device->a2dp_status_recheck & (~A2DP_STATUS_SYNC_A2DP_STATE);
        switch(a2dp_status->state) {
            case BT_A2DP_STREAM_STATE_STREAMING:
                app_bt_clear_connecting_profiles_state(device_id);
                a2dp_timestamp_parser_init();
                btif_a2dp_set_stream_state(Stream, a2dp_status->state);
                info.event = BTIF_A2DP_EVENT_STREAM_STARTED_MOCK;
                DEBUG_INFO(0,"::A2DP_EVENT_STREAM_STARTED mock");
                a2dp_ibrt_stream_need_autotrigger_set_flag();
#if defined(IBRT_FORCE_AUDIO_RETRIGGER)
                a2dp_callback(device_id, Stream, &info);
                app_ibrt_if_force_audio_retrigger();
#else
//                app_bt_stream_ibrt_audio_mismatch_resume();
                a2dp_callback(device_id, Stream, (a2dp_callback_parms_t*)&info);
#endif
                break;
            case BT_A2DP_STREAM_STATE_OPEN:
                //Ignore START->OPEN transition since itslef can received SUSPEND CMD
                if (old_avdtp_stream_state != BT_A2DP_STREAM_STATE_STREAMING)
                {
                    DEBUG_INFO(0,"::A2DP_EVENT_STREAM_OPEN mock");
                    btif_a2dp_set_stream_state(Stream, a2dp_status->state);
                    info.event = BTIF_A2DP_EVENT_STREAM_OPEN_MOCK;
                    info.p.configReq = &avdtp_config_req;
                    info.p.configReq->codec.codecType = a2dp_status->codec.codec_type;
                    info.p.configReq->codec.elements = ((uint8_t*)btif_a2dp_get_stream_codec(Stream))+2;
                    a2dp_callback(device_id, Stream, (a2dp_callback_parms_t*)&info);
                }
                break;
            default:
                if (btif_a2dp_get_stream_state(Stream) != BT_A2DP_STREAM_STATE_IDLE){
                    DEBUG_INFO(0,"::A2DP_EVENT_STREAM_SUSPENDED mock");
                    btif_a2dp_set_stream_state(Stream, a2dp_status->state);
                    info.event = BTIF_A2DP_EVENT_STREAM_SUSPENDED;
                    a2dp_callback(device_id, Stream, (a2dp_callback_parms_t*)&info);
                }
                break;
        }
        curr_device->a2dp_status_recheck = curr_device->a2dp_status_recheck & (~A2DP_STATUS_SYNC_A2DP_STATE);
    }
    return 0;
}

int a2dp_bt_stream_open_mock(uint8_t device_id, bt_bdaddr_t *remote)
{
    DEBUG_INFO(0,"::A2DP_EVENT_STREAM_OPEN mock");

    btif_a2dp_callback_parms_t info;
    btif_avdtp_config_request_t avdtp_config_req;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    info.event = BTIF_A2DP_EVENT_STREAM_OPEN_MOCK;
    info.p.configReq = &avdtp_config_req;
    info.p.configReq->codec.codecType = curr_device->codec_type;
    info.p.configReq->codec.elements = ((uint8_t*)btif_a2dp_get_stream_codec(curr_device->btif_a2dp_stream->a2dp_stream))+2;
    a2dp_callback(device_id, curr_device->btif_a2dp_stream->a2dp_stream, (a2dp_callback_parms_t*)&info);

    btdevice_profile *btdevice_plf_p = NULL;
    btdevice_plf_p = (btdevice_profile *)app_bt_profile_active_store_ptr_get(curr_device->remote.address);
    nv_record_btdevicerecord_set_a2dp_profile_active_state(btdevice_plf_p, true);
    nv_record_btdevicerecord_set_a2dp_profile_codec(btdevice_plf_p, curr_device->codec_type);
    audio_player_restore_volume((uint8_t *)curr_device->remote.address);

    DEBUG_INFO(2, "::A2DP_EVENT_STREAM_OPEN mock codec_type:%d vol:%d", curr_device->codec_type, app_bt_stream_volume_get_ptr()->a2dp_vol);

    return 0;
}

#define A2DP_IBRT_STREAM_SKIP_TWS_SNIFF_STATUS         (1)
int a2dp_ibrt_stream_event_stream_data_ind_needskip(uint8_t device_id, a2dp_stream_t *Stream)
{
    int nRet = 0;
    ibrt_ctrl_t  *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    POSSIBLY_UNUSED struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (app_tws_ibrt_mobile_link_connected(&curr_device->remote)){
        if (app_ibrt_conn_get_mobile_mode(&curr_device->remote) == IBRT_SNIFF_MODE){
            DEBUG_INFO(0,"::A2DP_EVENT_STREAM_DATA_IND ibrt_link skip (mobile) skip sniff\n");
            nRet = 1;
        }
        if (app_tws_ibrt_tws_link_connected()){
            if (app_ibrt_conn_is_profile_exchanged(&curr_device->remote) &&
                p_ibrt_ctrl->tws_mode == IBRT_SNIFF_MODE){
                DEBUG_INFO(0,"::A2DP_EVENT_STREAM_DATA_IND mobile_link (tws) skip sniff\n");
#ifndef A2DP_IBRT_STREAM_SKIP_TWS_SNIFF_STATUS
                nRet = 1;
#endif
            }
        }
    }else if (app_tws_ibrt_slave_ibrt_link_connected(&curr_device->remote)){
        if (app_ibrt_conn_get_mobile_mode(&curr_device->remote) == IBRT_SNIFF_MODE){
            DEBUG_INFO(0,"::A2DP_EVENT_STREAM_DATA_IND ibrt_link skip (mobile) skip sniff\n");
            nRet = 1;
        }
        if (p_ibrt_ctrl->tws_mode == IBRT_SNIFF_MODE){
            DEBUG_INFO(0,"::A2DP_EVENT_STREAM_DATA_IND ibrt_link skip (tws) skip sniff\n");
#ifndef A2DP_IBRT_STREAM_SKIP_TWS_SNIFF_STATUS
            nRet = 1;
#endif
        }
    }
    return nRet;
}
#endif

#ifdef A2DP_STREAM_DETECT_NO_DECODE
#define SBC_MUTE_DETECTE_TIME       (3000)  // uint: ms
#define SBC_NORMAL_DETECTE_TIME     (3000)  // uint: ms
#define AAC_MUTE_DETECTE_TIME       (3000)  // uint: ms
#define AAC_NORMAL_DETECTE_TIME     (3000)  // uint: ms

void app_a2dp_stream_detect_time_init(uint8_t dev_id)
{
    struct BT_DEVICE_T* curr_dev = app_bt_get_device(dev_id);

    if (!curr_dev)
    {
        return;
    }

    curr_dev->stream_detect.sbc_mute_detect_time = SBC_MUTE_DETECTE_TIME;
    curr_dev->stream_detect.sbc_normal_detect_time = SBC_NORMAL_DETECTE_TIME;
    curr_dev->stream_detect.aac_mute_detect_time = AAC_MUTE_DETECTE_TIME;
    curr_dev->stream_detect.aac_normal_detect_time = AAC_NORMAL_DETECTE_TIME;
}

void app_a2dp_stream_set_detect_time(uint8_t dev_id, codec_type_t codec, time_flag_t time_type, uint16_t time)
{
    struct BT_DEVICE_T* curr_dev = app_bt_get_device(dev_id);

    if (!curr_dev)
    {
        return;
    }

    TRACE(0, "set_detect_time d(%d) codec %d tt %d time %d", dev_id, codec, time_type, time);

    if (time_type == MUTE_TIME)
    {
        if (codec == SBC_CODEC)
        {
            curr_dev->stream_detect.sbc_mute_detect_time = time;
        }
        else if (codec == AAC_CODEC)
        {
            curr_dev->stream_detect.aac_mute_detect_time = time;
        }
    }
    else if (time_type == NORMAL_TIME)
    {
        if (codec == SBC_CODEC)
        {
            curr_dev->stream_detect.sbc_normal_detect_time = time;
        }
        else
        {
            curr_dev->stream_detect.aac_normal_detect_time = time;
        }
    }
}

uint32_t app_a2dp_convert_samplerate_to_sample_count(uint8_t sr)
{
    uint32_t sc = 0;
    switch (sr)
    {
        case A2D_SBC_IE_SAMP_FREQ_16:
            sc = 16000;
            break;
        case A2D_SBC_IE_SAMP_FREQ_32:
            sc = 32000;
            break;
        case A2D_SBC_IE_SAMP_FREQ_44:
            sc = 44100;
            break;
        case A2D_SBC_IE_SAMP_FREQ_48:
            sc = 48000;
            break;
        default:
            TRACE(0, "sample rate error");
    }

    return sc;
}

void app_a2dp_stream_detect_handle(uint8_t device_id, btif_a2dp_callback_parms_t *Info)
{
    struct BT_DEVICE_T* curr_dev = app_bt_get_device(device_id);
    detect_result_t ret = {UNKNOWN_STREAM, 0};
    btif_avdtp_codec_type_t curr_codec = 0XEF;  //invalid value
    app_bt_mute_detect_t detect_result = APP_BT_A2DP_STREAM_DETECT_INVALID;
    uint32_t sc_ps = 0; // sample count per second
    uint32_t mute_count_threshold = 0;
    uint32_t normal_count_threshold = 0;
    int header_len = 0;
    btif_avdtp_media_header_t header;
    U8 * media_payload = NULL;
    uint8_t media_payload_len;
    POSSIBLY_UNUSED uint16_t sbc_mute_detect_time = 0;
    POSSIBLY_UNUSED uint16_t sbc_normal_detect_time = 0;
    POSSIBLY_UNUSED uint16_t aac_mute_detect_time = 0;
    POSSIBLY_UNUSED uint16_t aac_normal_detect_time = 0;

    if (curr_dev == NULL)
    {
        TRACE(0, "a2dp_mute_stream_detect curr_dev NULL");
        return;
    }

    curr_codec = curr_dev->codec_type;

    sc_ps = app_a2dp_convert_samplerate_to_sample_count(curr_dev->sample_rate);

    if (sc_ps == 0)
    {
        return;
    }

#ifdef __A2DP_AVDTP_CP__ //zadd bug fixed sony Z5 no sound
    header_len = btif_avdtp_parse_mediaHeader(&header, (btif_a2dp_callback_parms_t *)Info, curr_device->avdtp_cp);
#else
    header_len = btif_avdtp_parse_mediaHeader(&header, (btif_a2dp_callback_parms_t *)Info, 0);
#endif // __A2DP_AVDTP_CP__

    media_payload = (unsigned char *)(Info->p.data) + header_len;
    media_payload_len = Info->len - header_len;
    DUMP8("%02x ", media_payload, media_payload_len);

    switch (curr_codec)
    {
    case BT_A2DP_CODEC_TYPE_SBC:
        mute_count_threshold = sc_ps * curr_dev->stream_detect.sbc_mute_detect_time / 1000;
        normal_count_threshold = sc_ps * curr_dev->stream_detect.sbc_normal_detect_time / 1000;
        ret = app_a2dp_detect_sbc_stream(media_payload);
        break;

    case BT_A2DP_CODEC_TYPE_MPEG2_4_AAC:
        mute_count_threshold = sc_ps * curr_dev->stream_detect.aac_mute_detect_time / 1000;
        normal_count_threshold = sc_ps * curr_dev->stream_detect.aac_normal_detect_time / 1000;
        TRACE(0, "device_id %d", device_id);
        ret = app_a2dp_detect_aac_stream(media_payload, media_payload_len);
        break;
    default:
        //TRACE(0, "unknow codec type %d", curr_codec);
        return;
        break;
    }

    if (ret.result == UNKNOWN_STREAM || ret.result != curr_dev->stream_detect.pre_packet_state)
    {
        return;
    }

    curr_dev->stream_detect.pre_packet_state = ret.result;

    if (ret.result == MUTE_STREAM)
    {
        curr_dev->stream_detect.mute_sample_count += ret.sample_count;
        curr_dev->stream_detect.normal_sample_count = 0;
    }
    else if (ret.result == NORMAL_STREAM)
    {
        curr_dev->stream_detect.normal_sample_count += ret.sample_count;
        curr_dev->stream_detect.mute_sample_count = 0;
    }

    if (curr_dev->stream_detect.mute_sample_count >= mute_count_threshold)
    {
        curr_dev->stream_detect.mute_sample_count = 0;
        detect_result = APP_BT_A2DP_STREAM_DETECT_STOP;
    }

    if (curr_dev->stream_detect.normal_sample_count >= normal_count_threshold)
    {
        curr_dev->stream_detect.normal_sample_count = 0;
        detect_result = APP_BT_A2DP_STREAM_DETECT_PLAYING;
    }

    if (detect_result == APP_BT_A2DP_STREAM_DETECT_INVALID)
    {
        //TRACE(0, "cur_mute_detect_result invalid");
        return;
    }

    curr_dev->stream_detect.result = detect_result;
    app_bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_MUTE_STREAM_DETECT_NOTIFY, curr_dev->stream_detect.result);

    return;
}

#define START_DETECT_AFTER_RCV_A2DP_START_TIME (2000)
void a2dp_a2dp_steam_detect_entry(uint8_t device_id, btif_a2dp_callback_parms_t *Info)
{
    struct BT_DEVICE_T* curr_dev = app_bt_get_device(device_id);
    uint8_t conn_a2dp_dev_cnt = 0;

    if (!curr_dev)
    {
        return;
    }

    // dont detect the first 2 second
    uint32_t diff_time = TICKS_TO_MS(hal_sys_timer_get() - curr_dev->stream_detect.a2dp_start_time);
    if (!curr_dev->stream_detect.can_start_detect && (diff_time > START_DETECT_AFTER_RCV_A2DP_START_TIME))
    {
        curr_dev->stream_detect.can_start_detect = true;
    }

    if (!curr_dev->stream_detect.can_start_detect)
    {
        return;
    }

    conn_a2dp_dev_cnt =  app_bt_audio_count_connected_a2dp();
    if (conn_a2dp_dev_cnt < 2)  // only one device, dont detect
    {
        return;
    }

    if ( app_bt_manager.curr_playing_a2dp_id == device_id)
    {
        return;
    }

    app_a2dp_stream_detect_handle(device_id, Info);
}

#endif // #ifdef A2DP_STREAM_DETECT_NO_DECODE

void app_a2dp_bt_driver_callback(uint8_t device_id, btif_a2dp_event_t event)
{
    struct bt_cb_tag* bt_drv_func_cb = bt_drv_get_func_cb_ptr();
    uint8_t conn_sco_count = 0;

#if defined(BT_HFP_SUPPORT)
    conn_sco_count = app_bt_audio_count_connected_sco();
#endif // BT_HFP_SUPPORT

    switch(event)
    {
        case BTIF_A2DP_EVENT_STREAM_STARTED:
        //fall through
        case BTIF_A2DP_EVENT_STREAM_STARTED_MOCK:
            if (conn_sco_count == 0)
            {
                if(bt_drv_func_cb->bt_switch_agc != NULL)
                {
                    bt_drv_func_cb->bt_switch_agc(BT_A2DP_WORK_MODE);
                }
            }
            break;
        case BTIF_A2DP_EVENT_STREAM_CLOSED:
        //fall through
        case BTIF_A2DP_EVENT_STREAM_IDLE:
        //fall through
        case BTIF_A2DP_EVENT_STREAM_SUSPENDED:
            if (app_bt_audio_count_streaming_a2dp() == 0 && conn_sco_count == 0)
            {
                if(bt_drv_func_cb->bt_switch_agc != NULL)
                {
                    bt_drv_func_cb->bt_switch_agc(BT_IDLE_MODE);
                }
            }
            break;
        default:
            break;
    }
}

#define A2DP_STREAM_REPEAT_DIFF_TIME_MS 500
#define A2DP_STREAM_REPEAT_ALLOWED_COUNT 3

uint8_t a2dp_get_current_codec_non_type(uint8_t *elements)
{
    uint8_t current_codec_type = BT_A2DP_CODEC_NONE_TYPE_INVALID;

    DEBUG_INFO(6,"INFO element[0]:0x%x, element[1]:0x%x, element[2]:0x%x, element[3]:0x%x, element[4]:0x%x, element[5]:0x%x",
        elements[0], elements[1],elements[2], elements[3], elements[4], elements[5]);

    uint32_t vendor_id = (uint32_t) elements[0];
    vendor_id |= ((uint32_t) elements[1]) << 8;
    vendor_id |= ((uint32_t) elements[2]) << 16;
    vendor_id |= ((uint32_t) elements[3]) << 24;
    uint16_t codec_id = (uint16_t) elements[4];
    codec_id |= ((uint16_t) elements[5]) << 8;

#if defined(A2DP_LHDC_ON)
        if (vendor_id == A2DP_LHDC_VENDOR_ID && codec_id == A2DP_LHDC_CODEC_ID) {
                DEBUG_INFO(0,"USE codec type is LHDC.");
            current_codec_type = BT_A2DP_CODEC_NONE_TYPE_LHDC;
        }
#endif // A2DP_LHDC_ON
#if defined(A2DP_LHDCV5_ON)
        if (vendor_id == A2DP_LHDCV5_VENDOR_ID && codec_id == A2DP_LHDCV5_CODEC_ID) {
            DEBUG_INFO(0,"USE codec type is LHDCV5.");
            current_codec_type = BT_A2DP_CODEC_NONE_TYPE_LHDCV5;
        }
#endif //A2DP_LHDCV5_ON

#if defined(A2DP_SCALABLE_ON)
        if (vendor_id == A2DP_SCALABLE_VENDOR_ID && codec_id == A2DP_SCALABLE_CODEC_ID)
        {
            DEBUG_INFO(0,"USE codec type is SCALABLE.");
            current_codec_type = BT_A2DP_CODEC_NONE_TYPE_SCALABLE;
        }
#endif // A2DP_SCALABLE_ON

#if defined(A2DP_LDAC_ON)
        if (vendor_id == A2DP_LDAC_VENDOR_ID && codec_id == A2DP_LDAC_CODEC_ID)
        {
            DEBUG_INFO(0,"USE codec type is LDAC.");
            current_codec_type = BT_A2DP_CODEC_NONE_TYPE_LDAC;
        }
#endif // A2DP_LDAC_ON

#if defined(A2DP_LC3_ON)
        if (vendor_id == A2DP_LC3_VENDOR_ID && (codec_id == A2DP_LC3_CODEC_ID || codec_id == A2DP_LC3_PLUS_CODEC_ID))
        {
            DEBUG_INFO(0,"USE codec type is LDAC.");
            current_codec_type = BT_A2DP_CODEC_NONE_TYPE_LC3;
        }
 #endif // A2DP_LC3_ON

 #if defined(A2DP_L2HC_ON)
        if (vendor_id == A2DP_L2HC_VENDOR_ID && codec_id == A2DP_L2HC_CODEC_ID )
        {
            DEBUG_INFO(0,"USE codec type is L2HC.");
            current_codec_type = BT_A2DP_CODEC_NONE_TYPE_L2HC;
        }
 #endif // A2DP_L2HC_ON

#if defined(A2DP_MIHC_ON)
        if (vendor_id == A2DP_MIHC_VENDOR_ID && codec_id == A2DP_MIHC_CODEC_ID )
        {
            DEBUG_INFO(0,"USE codec type is MIHC.");
            current_codec_type = BT_A2DP_CODEC_NONE_TYPE_MIHC;
        }
#endif // A2DP_MIHC_ON

    return current_codec_type;
}

#define A2DP_MSG_EVENT_QUEUE_NUM_MAX 20
#define A2DP_MSG_EVENT_FREE_NUM      2

typedef struct {
    single_link_head_t head;
    enum app_bt_audio_event_t event;
    uint8_t link_id;
    uint32_t timeout;
} a2dp_evevnt_msg;

static struct single_link_head_t a2dp_event_queue;

static void app_a2dp_message_send_timeout_callback(void const *para);
osTimerId a2dp_message_send_timeout_timer_id;
static uint32_t start_time = 0;
osTimerDef (A2DP_MSG_SEND_TIMER, app_a2dp_message_send_timeout_callback);

static void app_a2dp_message_send_timeout_callback(void const *para)
{
    TRACE(1,"%s", __func__);
    if(bt_defer_curr_func_1(app_a2dp_message_send_timeout_callback, bt_fixed_param(NULL)))
    {
        return;
    }

    a2dp_evevnt_msg * node = (a2dp_evevnt_msg *)colist_single_link_pop_head(&a2dp_event_queue);
    TRACE(1,"%s pop a2dp msg", __func__);
    if (node != NULL)
    {
        TRACE(2,"app_bt_audio_event_handler%d %d", node->link_id, node->event);
        bt_audio_event_handler(node->link_id, node->event, 0);
        bes_bt_buf_free(node);
        node = (a2dp_evevnt_msg *)colist_single_link_get_head(&a2dp_event_queue);
        if(node != NULL)
        {
            osTimerStart(a2dp_message_send_timeout_timer_id, node->timeout);
            TRACE(0,"a2dp_message_send time start 0");
        }
    }
}

void a2dp_event_handle(enum app_bt_audio_event_t event, uint8_t link_id)
{
    if(bt_defer_curr_func_2(a2dp_event_handle, bt_fixed_param(event), bt_fixed_param(link_id)))
    {
        return;
    }

#ifndef SLIM_TICKS_TO_MS
    uint32_t end_time = (uint32_t)__SLIM_TICKS_TO_MS(hal_sys_timer_get());
#else
    uint32_t end_time = (uint32_t)SLIM_TICKS_TO_MS(hal_sys_timer_get());
#endif
    uint32_t deltaMs = end_time - start_time;
    start_time = end_time;

    TRACE(3,"%s deltaMs deltaMs:%d end_time:%d", __func__, deltaMs, end_time);
    if (a2dp_message_send_timeout_timer_id == NULL)
    {
        a2dp_message_send_timeout_timer_id = osTimerCreate(osTimer(A2DP_MSG_SEND_TIMER), osTimerOnce, NULL);
        INIT_SINGLE_LINK_HEAD(&a2dp_event_queue);
    }

    a2dp_evevnt_msg* a2dp_msg = (a2dp_evevnt_msg*)bes_bt_buf_malloc(sizeof(a2dp_evevnt_msg));

    if(a2dp_msg == NULL)
    {
        Plt_Assert(0, "bes_bt_buf_malloc fail, please check");
    }

    a2dp_msg->event = event;
    a2dp_msg->link_id = link_id;
    a2dp_msg->timeout = (deltaMs >= 200) ? 200 : 200 - deltaMs;

    if(colist_single_link_is_list_empty(&a2dp_event_queue) && (deltaMs > 200))
    {
        TRACE(2,"list is null, delttime long send msg %d %d",a2dp_msg->link_id, a2dp_msg->event);
        bt_audio_event_handler(a2dp_msg->link_id, a2dp_msg->event, 0);
        bes_bt_buf_free(a2dp_msg);
        a2dp_msg = NULL;
    }
    else
    {
        if(colist_single_link_is_list_empty(&a2dp_event_queue))
        {
            TRACE(0,"a2dp_message_send time start 1");
            osTimerStart(a2dp_message_send_timeout_timer_id, a2dp_msg->timeout);
        }

        if(colist_single_link_size(&a2dp_event_queue) < A2DP_MSG_EVENT_QUEUE_NUM_MAX)
        {
            colist_single_link_push_tail((single_link_node_t *)a2dp_msg, &a2dp_event_queue);
            TRACE(0,"push a2dp_message_send time to list");
        }
        else
        {
            TRACE(0,"queue is full, pop/free a2dp_msg_event queue");
            for (int i = 0; i < A2DP_MSG_EVENT_FREE_NUM; i++)
            {
                a2dp_evevnt_msg * node = (a2dp_evevnt_msg *)colist_single_link_pop_head(&a2dp_event_queue);
                if (node != NULL)
                {
                    bes_bt_buf_free(node);
                    node = NULL;
                }
            }
            colist_single_link_push_tail((single_link_node_t *)a2dp_msg, &a2dp_event_queue);
            TRACE(0,"free queue and push a2dp_message_send time to list");
        }
    }
}

void app_a2dp_set_vender_codec_non_type(uint8_t device_id, uint8_t a2dp_non_type)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);

    if (curr_device == NULL)
    {
        DEBUG_WARNING(0, "app_a2dp_set_vender_codec_non_type d(%d)) NULL", device_id);
        return;
    }

    curr_device->a2dp_non_type = a2dp_non_type;
    return;
}

void a2dp_callback(uint8_t device_id, a2dp_stream_t *Stream, const a2dp_callback_parms_t *info)
{
    int header_len = 0;
    btif_avdtp_media_header_t header;
    btif_a2dp_callback_parms_t * Info = (btif_a2dp_callback_parms_t *)info;
    POSSIBLY_UNUSED btif_avdtp_codec_t  *codec =  NULL;
    btif_remote_device_t *rdev = NULL;
    bt_bdaddr_t *bt_addr;
    bool is_duplicated_stream_event = false;

    POSSIBLY_UNUSED uint8_t a2dp_codec_non_type = BT_A2DP_CODEC_NONE_TYPE_INVALID;

    if (device_id == BT_DEVICE_INVALID_ID && Info->event == BTIF_A2DP_EVENT_STREAM_CLOSED)
    {
        // a2dp profile is closed due to acl created fail
        DEBUG_INFO(1,"::A2DP_EVENT_STREAM_CLOSED acl created error %x", Info->discReason);
        return;
    }

    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
    ASSERT(device_id < BT_DEVICE_NUM && curr_device->btif_a2dp_stream->a2dp_stream == Stream, "a2dp device channel must match");

    static uint8_t detect_first_packet[BT_DEVICE_NUM] = {0,};
    static btif_avdtp_codec_t   setconfig_codec;
    static u8 tmp_element[10];

    if (BTIF_A2DP_EVENT_STREAM_DATA_IND != Info->event)
    {
        DEBUG_INFO(3,"(d%x) %s event %d", device_id, __func__, Info->event);
    }

    if(BTIF_A2DP_EVENT_STREAM_STARTED == Info->event || BTIF_A2DP_EVENT_STREAM_SUSPENDED == Info->event)
    {
        if(curr_device->a2dp_status_recheck & A2DP_STATUS_SYNC_A2DP_STATE)
        {
            curr_device->a2dp_status_recheck &= ~(A2DP_STATUS_SYNC_A2DP_STATE);
        }
    }

    codec = btif_a2dp_get_stream_codec(Stream);

    switch(Info->event) {
        case BTIF_A2DP_REMOTE_NOT_SUPPORT:
            DEBUG_INFO(2,"(d%x) ::A2DP_EVENT_REMOTE_NOT_SUPPORT %d", device_id, Info->event);
            app_bt_profile_connect_manager_a2dp(device_id, Stream, (a2dp_callback_parms_t *)Info);
            break;
        case BTIF_A2DP_EVENT_AVDTP_DISCONNECT:
            DEBUG_INFO(3,"(d%x) ::A2DP_EVENT_AVDTP_DISCONNECT %d st = %p", device_id, Info->event, Stream);
            break;
        case BTIF_A2DP_EVENT_AVDTP_CONNECT:
            DEBUG_INFO(3,"(d%x) ::A2DP_EVENT_AVDTP_CONNECT %d st = %p", device_id, Info->event, Stream);

#ifdef BT_USB_AUDIO_DUAL_MODE
            if(!btusb_is_bt_mode())
            {
                btusb_btaudio_close(false);
            }
#endif
            break;
        case BTIF_A2DP_EVENT_STREAM_OPEN:
            curr_device->mock_a2dp_after_force_disc = false;
            //fall through
        case BTIF_A2DP_EVENT_STREAM_OPEN_MOCK:
            DEBUG_INFO(3,"(d%x) ::A2DP_EVENT_STREAM_OPEN codec %d codec.elements %x", device_id, codec->codecType, Info->p.configReq->codec.elements[0]);

            if (app_bt_is_hfp_connected(device_id))
            {
                curr_device->profiles_connected_before = true;
            }

            curr_device->a2dp_conn_flag = 1;
            curr_device->a2dp_disc_on_process = 0;

            curr_device->ibrt_disc_a2dp_profile_only = false;
            curr_device->ibrt_slave_force_disc_a2dp = false;

#ifdef GFPS_ENABLED
            app_exit_fastpairing_mode();
#endif

            app_bt_clear_connecting_profiles_state(device_id);

            rdev = btif_a2dp_get_remote_device(Stream);
            bt_addr = btif_me_get_remote_device_bdaddr(rdev);

#if defined(BT_MAP_SUPPORT)
            if (Info->event == BTIF_A2DP_EVENT_STREAM_OPEN)
            {
                bt_map_connect(&curr_device->remote);
            }
#endif
            a2dp_timestamp_parser_init();
            audio_player_restore_volume((uint8_t *) bt_addr);

            if (Info->p.configReq->codec.codecType == BT_A2DP_CODEC_TYPE_NON_A2DP)
            {
                bool find_non_type_success = false;
                curr_device->codec_type = BT_A2DP_CODEC_TYPE_NON_A2DP;
                a2dp_codec_non_type = a2dp_get_current_codec_non_type(Info->p.configReq->codec.elements);
                app_a2dp_set_vender_codec_non_type(device_id, a2dp_codec_non_type);
                if (a2dp_codec_non_type == BT_A2DP_CODEC_NONE_TYPE_INVALID)
                {
                    TRACE(0, "a2dp_codec_non_type INVALID");
                    goto find_complete;
                }
#if defined(A2DP_LHDC_ON)
                if (a2dp_codec_non_type == BT_A2DP_CODEC_NONE_TYPE_LHDC)
                {
                    DEBUG_INFO(3,"(d%x) ##codecType: LHDC Codec, Element length = %d", device_id, Info->p.configReq->codec.elemLen);
                    a2dp_lhdc_config(device_id, &(Info->p.configReq->codec.elements[0]));
                    find_non_type_success = true;
                    goto find_complete;
                }
#endif // A2DP_LHDC_ON
#if defined(A2DP_LHDCV5_ON)
                if (a2dp_codec_non_type == BT_A2DP_CODEC_NONE_TYPE_LHDCV5)
                {
                    DEBUG_INFO(3,"(d%x) ##codecType: LHDCV5 Codec, Element length = %d, AVDTP_MAX_CODEC_ELEM_SIZE = %d",
                                        device_id, Info->p.configReq->codec.elemLen, 11);
                    a2dp_lhdcv5_config(device_id, &(Info->p.configReq->codec.elements[0]));
                    find_non_type_success = true;
                    goto find_complete;
                }
#endif // A2DP_LHDCV5_ON
#if defined(A2DP_LDAC_ON)
                if (a2dp_codec_non_type == BT_A2DP_CODEC_NONE_TYPE_LDAC)
                {
                    //Codec Info Element: 0x 2d 01 00 00 aa 00 34 07
                    if(Info->p.codec->elements[0] == 0x2d)
                    {
#ifndef A2DP_DECODER_CROSS_CORE
                        curr_device->sample_bit = 16;
#else
                        curr_device->sample_bit = 24;
#endif
                        a2dp_ldac_config(device_id, &(Info->p.configReq->codec.elements[0]));
                    }
                    else
                    {
                        if(Info->p.codec->pstreamflags != NULL)
                        {
                            Info->p.codec->pstreamflags[0] &= ~APP_A2DP_STRM_FLAG_QUERY_CODEC;
                        }
                        else
                        {
                            ASSERT(false, "pstreamflags not init ..");
                        }
                        curr_device->a2dp_channel_num = 2;
                    }
                    find_non_type_success = true;
                    goto find_complete;
                }
#endif // A2DP_LDAC_ON
#if defined(A2DP_SCALABLE_ON)
                if (a2dp_codec_non_type == BT_A2DP_CODEC_NONE_TYPE_SCALABLE)
                {
                    DEBUG_INFO(1,"(d%x) ##codecType scalable", device_id);
                    a2dp_scalable_config(device_id, &(Info->p.configReq->codec.elements[0]));
                    //0x75 0x00 0x00 0x00 Vid
                    //0x03 0x01 Codec id
                    if(Info->p.codec->elements[0]==0x75 && Info->p.codec->elements[4]==0x03 && Info->p.codec->elements[5]==0x01)
                    {
                        setconfig_codec.elements = a2dp_scalable_avdtpcodec.elements;
                    }
                    else
                    {
                        if(Info->p.codec->pstreamflags != NULL)
                        {
                            Info->p.codec->pstreamflags[0] &= ~APP_A2DP_STRM_FLAG_QUERY_CODEC;
                        }
                        else
                        {
                            ASSERT(false, "pstreamflags not init ..");
                        }
                        curr_device->a2dp_channel_num = 2;
                    }
                    find_non_type_success = true;
                    goto find_complete;
                }
#endif // A2DP_SCALABLE_ON
#if defined(A2DP_LC3_ON)
                if (a2dp_codec_non_type == BT_A2DP_CODEC_NONE_TYPE_LC3)
                {
                    DEBUG_INFO(2,"##codecType: LC3 Codec, Element length = %d, AVDTP_MAX_CODEC_ELEM_SIZE = %d\n", Info->p.configReq->codec.elemLen, BT_A2DP_MAX_CODEC_ELEM_SIZE);
                    a2dp_lc3_config(device_id, &(Info->p.configReq->codec.elements[0]));
                    find_non_type_success = true;
                    goto find_complete;
                }
#endif // A2DP_LC3_ON

#if defined(A2DP_L2HC_ON)
                if (a2dp_codec_non_type == BT_A2DP_CODEC_NONE_TYPE_L2HC)
                {
                    DEBUG_INFO(2,"##codecType: L2HC Codec, Element length = %d, AVDTP_MAX_CODEC_ELEM_SIZE = %d\n", Info->p.configReq->codec.elemLen, BT_A2DP_MAX_CODEC_ELEM_SIZE);
                    a2dp_l2hc_config(device_id, &(Info->p.configReq->codec.elements[0]));
                    find_non_type_success = true;
                    goto find_complete;
                }
#endif // A2DP_L2HC_ON

#if defined(A2DP_MIHC_ON)
                if (a2dp_codec_non_type == BT_A2DP_CODEC_NONE_TYPE_MIHC)
                {
                    DEBUG_INFO(2,"##codecType: MIHC Codec, Element length = %d, AVDTP_MAX_CODEC_ELEM_SIZE = %d\n", Info->p.configReq->codec.elemLen, BT_A2DP_MAX_CODEC_ELEM_SIZE);
                    a2dp_mihc_config(device_id, &(Info->p.configReq->codec.elements[0]));
                    find_non_type_success = true;
                    goto find_complete;
                }
#endif

find_complete:
                if (find_non_type_success == false)
                {
                    DEBUG_INFO(3,"(d%x) ##codecType: unknown Codec, Element length = %d, BT_A2DP_MAX_CODEC_ELEM_SIZE = %d",
                                        device_id, Info->p.configReq->codec.elemLen, BT_A2DP_MAX_CODEC_ELEM_SIZE);

                    curr_device->codec_type = BT_A2DP_CODEC_TYPE_INVALID;
                    app_a2dp_set_vender_codec_non_type(device_id, BT_A2DP_CODEC_NONE_TYPE_INVALID);
                }
            }
            else if (Info->p.configReq->codec.codecType == BT_A2DP_CODEC_TYPE_MPEG2_4_AAC)
            {
#if defined(A2DP_AAC_ON)
                DEBUG_INFO(2,"(d%x) ::A2DP_EVENT_STREAM_OPEN aac codec.elements %x", device_id, Info->p.configReq->codec.elements[1]);
                curr_device->codec_type = BT_A2DP_CODEC_TYPE_MPEG2_4_AAC;
                app_a2dp_set_vender_codec_non_type(device_id, BT_A2DP_CODEC_NONE_TYPE_INVALID);
                curr_device->sample_bit = 16;
                // convert aac sample_rate to sbc sample_rate format
                if (Info->p.configReq->codec.elements[1] & A2DP_AAC_OCTET1_SAMPLING_FREQUENCY_44100) {
                    DEBUG_INFO(1,"(d%x) ::A2DP_EVENT_STREAM_OPEN aac sample_rate 44100", device_id);
                    curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_44;
                }
                else if (Info->p.configReq->codec.elements[2] & A2DP_AAC_OCTET2_SAMPLING_FREQUENCY_48000) {
                    DEBUG_INFO(1,"(d%x) ::A2DP_EVENT_STREAM_OPEN aac sample_rate 48000", device_id);
                    curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_48;
                }
                else {
                    DEBUG_INFO(1,"(d%x) ::A2DP_EVENT_STREAM_OPEN aac sample_rate not 48000 or 44100, set to 44100", device_id);
                    curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_44;
                }

                if (Info->p.configReq->codec.elements[2] & A2DP_AAC_OCTET2_CHANNELS_1) {
                    curr_device->a2dp_channel_num = 1;
                }
                else {
                    curr_device->a2dp_channel_num = 2;
                }
#endif // A2DP_AAC_ON
            }
            else
            {
                DEBUG_INFO(6,"(d%x) sample_rate::elements[0] %d BITPOOL:%d/%d %02x/%02x", device_id,
                                    Info->p.codec->elements[0],
                                    Info->p.codec->elements[2],
                                    Info->p.codec->elements[3],
                                    Info->p.codec->elements[2],
                                    Info->p.codec->elements[3]);

                curr_device->codec_type = BT_A2DP_CODEC_TYPE_SBC;
                curr_device->sample_bit = 16;
#if defined(A2DP_LDAC_ON)
                curr_device->sample_rate = (Info->p.configReq->codec.elements[0] & (A2D_SBC_IE_SAMP_FREQ_MSK == 0xff ? (A2D_SBC_IE_SAMP_FREQ_MSK & 0xf0) : A2D_SBC_IE_SAMP_FREQ_MSK));
#else
                curr_device->sample_rate = (Info->p.configReq->codec.elements[0] & A2D_SBC_IE_SAMP_FREQ_MSK);
#endif

                if(Info->p.configReq->codec.elements[0] & A2D_SBC_IE_CH_MD_MONO)
                    curr_device->a2dp_channel_num  = 1;
                else
                    curr_device->a2dp_channel_num = 2;

            }

            if (Info->event == BTIF_A2DP_EVENT_STREAM_OPEN) // dont auto reconnect avrcp when open mock
            {
                if (besbt_cfg.mark_some_code_for_fuzz_test)
                {
                    // cannot initiate profile connect for fuzz test
                }
#if defined(IBRT)
#if defined(IBRT_UI)
                else if ((IBRT_SLAVE == app_tws_get_ibrt_role(&curr_device->remote)) && (app_ibrt_conn_is_ibrt_connected(&curr_device->remote)))
#else
                else if ((IBRT_SLAVE == app_tws_get_ibrt_role(&curr_device->remote)) && (app_tws_ibrt_get_bt_ctrl_ctx()->snoop_connected))
#endif
                {
                    DEBUG_INFO(0,"dont auto reconnect avrcp when IBRT SLAVE snoop connected");
                }
#endif
                else
                {
                    app_bt_reconnect_avrcp_profile(&curr_device->remote);
                }
            }

            bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_A2DP_STREAM_OPEN, 0);

            app_bt_profile_connect_manager_a2dp(device_id, Stream, (a2dp_callback_parms_t *)Info);

#if defined(IBRT)
            a2dp_ibrt_session_reset(device_id);
#endif

#if defined(IBRT) && defined(IBRT_UI)
            app_ibrt_clear_profile_connect_protect(device_id, APP_IBRT_A2DP_PROFILE_ID);
            app_ibrt_clear_profile_disconnect_protect(device_id, APP_IBRT_A2DP_PROFILE_ID);
#endif
            break;
        case BTIF_A2DP_EVENT_STREAM_OPEN_IND:
            DEBUG_INFO(2,"(d%x) ::A2DP_EVENT_STREAM_OPEN_IND %d", device_id, Info->event);
            btif_a2dp_open_stream_rsp(Stream, BTIF_A2DP_ERR_NO_ERROR, BTIF_AVDTP_SRV_CAT_MEDIA_TRANSPORT);
            break;
        case BTIF_A2DP_EVENT_CODEC_INFO_MOCK:
            break;
        case BTIF_A2DP_EVENT_STREAM_STARTED:
            if (curr_device->a2dp_streamming)
            {
                uint32_t curr_time_ms = hal_sys_timer_get();
                uint32_t diff_time_ms = TICKS_TO_MS(curr_time_ms - curr_device->a2dp_stream_start_time);
                DEBUG_INFO(0, "(d%x) ::A2DP_EVENT_STREAM_STARTED diff %d count %d", device_id, diff_time_ms, curr_device->a2dp_stream_start_repeat_count);
                curr_device->a2dp_stream_start_time = curr_time_ms;
                if (diff_time_ms < A2DP_STREAM_REPEAT_DIFF_TIME_MS)
                {
                    curr_device->a2dp_stream_start_repeat_count += 1;
                    is_duplicated_stream_event = true;
                }
                if (curr_device->a2dp_stream_start_repeat_count > A2DP_STREAM_REPEAT_ALLOWED_COUNT)
                {
                    return; // filter fuzz many duplicated start to avoid mem overflow
                }
            }

            if (is_duplicated_stream_event == false)
            {
                curr_device->a2dp_stream_start_time = hal_sys_timer_get();
                curr_device->a2dp_stream_start_repeat_count = 0;
            }

#if defined(IBRT) &&  !defined(BTH_SUBSYS_ONLY)
            a2dp_ibrt_session_new(device_id);

            if (app_tws_ibrt_mobile_link_connected(&curr_device->remote))
            {
                rdev = btif_a2dp_get_remote_device(Stream);
#if defined(__3M_PACK__)
                btif_me_send_prefer_rate(btif_me_get_remote_device_hci_handle(rdev),
                    USE_5_SLOT_EDR_PKT|USE_3_MBPS_RATE);
#else
                btif_me_send_prefer_rate(btif_me_get_remote_device_hci_handle(rdev),
                    USE_5_SLOT_EDR_PKT|USE_2_MBPS_RATE);
#endif
            }
            // FALLTHROUGH
        case BTIF_A2DP_EVENT_STREAM_STARTED_MOCK:
            a2dp_audio_retrigger_set_on_process(false);
#endif

#if defined(_AUTO_TEST_)
            AUTO_TEST_SEND("Music on ok.");
#endif

            app_bt_set_keep_active_mode(true, BT_ACTIVE_MODE_KEEP_USER_A2DP_STREAMING, device_id);
            if (!besbt_cfg.dont_auto_report_delay_report && btif_a2dp_is_stream_device_has_delay_reporting(Stream))
            {
                btif_a2dp_set_sink_delay(device_id, 150);
            }

            a2dp_timestamp_parser_init();
#if defined(__CONNECTIVITY_LOG_REPORT__)
            app_ibrt_if_reset_acl_data_packet_check();
#endif
            curr_device->a2dp_streamming = 1;
            curr_device->a2dp_play_pause_flag = 1;
            detect_first_packet[device_id] = 1;
            curr_device->rsv_avdtp_start_signal = true;

            DEBUG_INFO(3,"(d%x) ::A2DP_EVENT_STREAM_STARTED codec %d streaming %s",
                            device_id,
                            codec->codecType,
                            app_bt_a2dp_get_all_device_streaming_state());

            a2dp_event_handle(Info->event == BTIF_A2DP_EVENT_STREAM_STARTED ? APP_BT_AUDIO_EVENT_A2DP_STREAM_START : APP_BT_AUDIO_EVENT_A2DP_STREAM_MOCK_START, device_id);
#if (A2DP_DECODER_VER == 2)
#if defined(IBRT)
            if (Info->event == BTIF_A2DP_EVENT_STREAM_STARTED)
            {
                a2dp_audio_latency_factor_setlow();
            }
#else
            a2dp_audio_latency_factor_setlow();
#endif
#endif

#ifdef __IAG_BLE_INCLUDE__
            bes_ble_gap_update_conn_param_mode(BLE_CONN_PARAM_MODE_A2DP_ON, true);
#endif
            break;
        case BTIF_A2DP_EVENT_STREAM_START_IND:
            DEBUG_INFO(3,"(d%x) ::A2DP_EVENT_STREAM_START_IND codec %d streaming %s",
                device_id, codec->codecType, app_bt_a2dp_get_all_device_streaming_state());
#ifdef BT_USB_AUDIO_DUAL_MODE
            if(!btusb_is_bt_mode())
            {
                btif_a2dp_start_stream_rsp(Stream, BTIF_A2DP_ERR_INSUFFICIENT_RESOURCE);
            }
            else
#endif
            {
                btif_a2dp_start_stream_rsp(Stream, BTIF_A2DP_ERR_NO_ERROR);
                curr_device->a2dp_play_pause_flag = 1;
            }
            break;
        case BTIF_A2DP_EVENT_STREAM_IDLE:
            DEBUG_INFO(1,"(d%x) ::A2DP_EVENT_STREAM_IDLE", device_id);
            // FALLTHROUGH
        case BTIF_A2DP_EVENT_STREAM_SUSPENDED:
            curr_device->waiting_pause_suspend = false;
#if defined(IBRT) &&  !defined(BTH_SUBSYS_ONLY)
            a2dp_audio_retrigger_set_on_process(false);
#endif
            DEBUG_INFO(2,"(d%x) ::A2DP_EVENT_STREAM_SUSPENDED streaming %s",
                device_id, app_bt_a2dp_get_all_device_streaming_state());
            if (curr_device->a2dp_streamming == 0)
            {
                uint32_t curr_time_ms = hal_sys_timer_get();
                uint32_t diff_time_ms = TICKS_TO_MS(curr_time_ms - curr_device->a2dp_stream_suspend_time);
                DEBUG_INFO(0, "(d%x) ::A2DP_EVENT_STREAM_SUSPENDED diff %d count %d", device_id, diff_time_ms, curr_device->a2dp_stream_suspend_repeat_count);
                curr_device->a2dp_stream_suspend_time = curr_time_ms;
                if (diff_time_ms < A2DP_STREAM_REPEAT_DIFF_TIME_MS)
                {
                    curr_device->a2dp_stream_suspend_repeat_count += 1;
                    is_duplicated_stream_event = true;
                }
                if (curr_device->a2dp_stream_suspend_repeat_count > A2DP_STREAM_REPEAT_ALLOWED_COUNT)
                {
                    return; // filter fuzz many duplicated suspend to avoid mem overflow
                }
            }

            if (is_duplicated_stream_event == false)
            {
                curr_device->a2dp_stream_suspend_time = hal_sys_timer_get();
                curr_device->a2dp_stream_suspend_repeat_count = 0;
            }

#if defined(_AUTO_TEST_)
            AUTO_TEST_SEND("Music suspend ok.");
#endif
            curr_device->a2dp_streamming = 0;
            app_bt_set_keep_active_mode(false, BT_ACTIVE_MODE_KEEP_USER_A2DP_STREAMING, device_id);
            a2dp_timestamp_parser_init();
#if defined(__CONNECTIVITY_LOG_REPORT__)
            app_ibrt_if_reset_acl_data_packet_check();
#endif
            curr_device->a2dp_play_pause_flag = 0;

            a2dp_event_handle(APP_BT_AUDIO_EVENT_A2DP_STREAM_SUSPEND, device_id);

#ifdef __IAG_BLE_INCLUDE__
            bes_ble_gap_update_conn_param_mode(BLE_CONN_PARAM_MODE_A2DP_ON, false);
#endif
            break;
        case BTIF_A2DP_EVENT_STREAM_DATA_IND:
            if (!app_a2dp_custom_allow_receive_steam())
            {
                DEBUG_INFO(0,"CUSTOM AF CONFIG block A2DP data");
                break;
            }
#ifdef __AI_VOICE__
            if(app_ai_voice_is_need2block_a2dp()) {
                DEBUG_INFO(0,"AI block A2DP data");
                break;
            }
#endif
#ifdef A2DP_STREAM_DETECT_NO_DECODE
            a2dp_a2dp_steam_detect_entry(device_id, Info);
#endif  // A2DP_STREAM_DETECT_NO_DECODE

#if defined(IBRT)
            if (a2dp_ibrt_stream_event_stream_data_ind_needskip(device_id, Stream)){
                header_len = btif_avdtp_parse_mediaHeader(&header, (btif_a2dp_callback_parms_t *)Info,0);
                DEBUG_INFO(3,"(d%x) ::A2DP_EVENT_STREAM_DATA_IND skip seq:%d timestamp:%d", device_id, header.sequenceNumber, header.timestamp);
                break;
            }
#else
            if (btif_me_get_current_mode(btif_a2dp_get_remote_device(Stream)) == BTIF_BLM_SNIFF_MODE){
                DEBUG_INFO(1,"(d%x) ::A2DP_EVENT_STREAM_DATA_IND skip", device_id);
                break;
            }
#endif
            if (detect_first_packet[device_id]) {
                detect_first_packet[device_id] = 0;
                //avrcp_get_current_media_status(device_id);
            }

            if (app_audio_adm_curr_a2dp_data_need_receive(device_id))
            {
#if defined(MUTE_FRAME_DETECT)
                if(a2dp_audio_is_mute_frame(device_id))
                {
                    app_bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_A2DP_MUTE_FRAME, 0);
                }
#endif

#ifdef __A2DP_AVDTP_CP__ //zadd bug fixed sony Z5 no sound
                header_len = btif_avdtp_parse_mediaHeader(&header, (btif_a2dp_callback_parms_t *)Info, curr_device->avdtp_cp);
#else
                header_len = btif_avdtp_parse_mediaHeader(&header, (btif_a2dp_callback_parms_t *)Info, 0);
#endif
#ifdef __A2DP_TIMESTAMP_PARSER__
                if (a2dp_timestamp_parser_run(header.timestamp,(*(((unsigned char *)Info->p.data) + header_len))))
                {
                    DEBUG_INFO(1,"(d%x) ::A2DP_EVENT_STREAM_DATA_IND skip frame", device_id);
                }
                else
#endif
                {
                    if (NULL != rmt_a2dp_sink_dev_data_ind_cb)
                    {
                        rmt_a2dp_sink_dev_data_ind_cb(((unsigned char *)Info->p.data) + header_len , Info->len - header_len);
                        break;
                    }
#ifndef BTH_SUBSYS_ONLY
#if (A2DP_DECODER_VER >= 2)
                    a2dp_audio_store_packet(device_id, &header, ((unsigned char *)Info->p.data) + header_len , Info->len - header_len);
#else
                    a2dp_audio_sbc_set_frame_info(Info->len - header_len - 1, (*(((unsigned char *)Info->p.data) + header_len)));
#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)  || defined(A2DP_LDAC_ON) || defined(A2DP_SCALABLE_ON) || defined(A2DP_LC3_ON)
                    if (curr_device->codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP)
                    {
                        store_sbc_buffer(device_id, ((unsigned char *)Info->p.data) + header_len , Info->len - header_len);
                    }
                    else
#endif
#if defined(A2DP_AAC_ON)
                    if (curr_device->codec_type == BT_A2DP_CODEC_TYPE_MPEG2_4_AAC)
                    {
#ifdef BT_USB_AUDIO_DUAL_MODE
                        if(btusb_is_bt_mode())
#endif
                        {
                            store_sbc_buffer(device_id, ((unsigned char *)Info->p.data) + header_len , Info->len - header_len);
                        }
                    }
                    else
#endif
                    {
                        store_sbc_buffer(device_id, ((unsigned char *)Info->p.data) + header_len + 1 , Info->len - header_len - 1);
                    }
#endif
#endif
                }
            }
            else
            {
                 // DEBUG_INFO(0, "Current A2DP data not need receive");
            }
            break;
        case BTIF_A2DP_EVENT_STREAM_CLOSED:
            DEBUG_INFO(3,"(d%x) ::A2DP_EVENT_STREAM_CLOSED reason %x force_disc %d", device_id, Info->discReason, curr_device->ibrt_slave_force_disc_a2dp);
#if defined(IBRT) &&  !defined(BTH_SUBSYS_ONLY)
            a2dp_audio_retrigger_set_on_process(false);
#endif
            if (btif_a2dp_is_disconnected(Stream))
            {
                curr_device->a2dp_conn_flag = 0;
                curr_device->a2dp_disc_on_process = 0;
                if (curr_device->ibrt_disc_a2dp_profile_only || curr_device->ibrt_slave_force_disc_a2dp)
                {
                    DEBUG_INFO(2, "%s dont close avrcp due to disc a2dp only %d", __func__, curr_device->ibrt_disc_a2dp_profile_only);
                }
                else if (Info->a2dp_closed_due_to_sdp_fail)
                {
                    DEBUG_INFO(1, "%s dont close avrcp due to sdp fail", __func__);
                }
                else
                {
                    app_bt_disconnect_avrcp_profile(curr_device->avrcp_channel);
                }

                curr_device->ibrt_disc_a2dp_profile_only = false;
                curr_device->ibrt_slave_force_disc_a2dp = false;

                app_bt_profile_connect_manager_a2dp(device_id, Stream, info);
#if defined(IBRT)
                a2dp_ibrt_session_reset(device_id);
#endif
#if defined(IBRT) && defined(IBRT_UI)
                app_ibrt_clear_profile_connect_protect(device_id, APP_IBRT_A2DP_PROFILE_ID);
                app_ibrt_clear_profile_disconnect_protect(device_id, APP_IBRT_A2DP_PROFILE_ID);
#endif
            }

            a2dp_timestamp_parser_init();
            curr_device->a2dp_streamming = 0;
            curr_device->a2dp_play_pause_flag = 0;
#ifdef __A2DP_AVDTP_CP__
            curr_device->avdtp_cp = 0;
#endif
            bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_A2DP_STREAM_CLOSE, 0);
            break;
        case BTIF_A2DP_EVENT_CODEC_INFO:
            DEBUG_INFO(2,"(d%x) ::A2DP_EVENT_CODEC_INFO %d", device_id, Info->event);
            if (!besbt_cfg.dont_auto_report_delay_report && btif_a2dp_is_stream_device_has_delay_reporting(Stream))
            {
                btif_a2dp_set_sink_delay(device_id, 150);
            }
            setconfig_codec.codecType = Info->p.codec->codecType;
            setconfig_codec.discoverable = Info->p.codec->discoverable;
            setconfig_codec.elemLen = Info->p.codec->elemLen;
            setconfig_codec.elements = tmp_element;
            memset(tmp_element, 0, sizeof(tmp_element));
            DUMP8("%02x ", (setconfig_codec.elements), 8);
            if (Info->p.codec->codecType == BT_A2DP_CODEC_TYPE_SBC) {
                setconfig_codec.elements[0] = (Info->p.codec->elements[0]) & (a2dp_codec_elements[0]);
                setconfig_codec.elements[1] = (Info->p.codec->elements[1]) & (a2dp_codec_elements[1]);

                if (Info->p.codec->elements[2] <= a2dp_codec_elements[2])
                    setconfig_codec.elements[2] = a2dp_codec_elements[2];////[2]:MIN_BITPOOL
                else
                    setconfig_codec.elements[2] = Info->p.codec->elements[2];

                if (Info->p.codec->elements[3] >= a2dp_codec_elements[3])
                    setconfig_codec.elements[3] = a2dp_codec_elements[3];////[3]:MAX_BITPOOL
                else
                    setconfig_codec.elements[3] = Info->p.codec->elements[3];

                ///////null set situation:
                if (setconfig_codec.elements[3] < a2dp_codec_elements[2]) {
                    setconfig_codec.elements[2] = a2dp_codec_elements[2];
                    setconfig_codec.elements[3] = a2dp_codec_elements[3];
                }
                else if (setconfig_codec.elements[2] > a2dp_codec_elements[3]) {
                    setconfig_codec.elements[2] = a2dp_codec_elements[3];
                    setconfig_codec.elements[3] = a2dp_codec_elements[3];
                }
                DEBUG_INFO(3,"(d%x) !!!setconfig_codec.elements[2]:%d,setconfig_codec.elements[3]:%d",
                    device_id,setconfig_codec.elements[2],setconfig_codec.elements[3]);

                setconfig_codec.elements[0] = a2dp_codec_sbc_get_valid_bit(setconfig_codec.elements[0],A2D_SBC_IE_SAMP_FREQ_MSK);
                setconfig_codec.elements[0] = a2dp_codec_sbc_get_valid_bit(setconfig_codec.elements[0],A2D_SBC_IE_CH_MD_MSK);
                setconfig_codec.elements[1] = a2dp_codec_sbc_get_valid_bit(setconfig_codec.elements[1],A2D_SBC_IE_BLOCKS_MSK);
                setconfig_codec.elements[1] = a2dp_codec_sbc_get_valid_bit(setconfig_codec.elements[1],A2D_SBC_IE_SUBBAND_MSK);
                setconfig_codec.elements[1] = a2dp_codec_sbc_get_valid_bit(setconfig_codec.elements[1],A2D_SBC_IE_ALLOC_MD_MSK);
            }
#if defined(A2DP_AAC_ON)
            else if(Info->p.codec->codecType == BT_A2DP_CODEC_TYPE_MPEG2_4_AAC) {
                setconfig_codec.elements[0] = a2dp_codec_aac_elements[0];
                if (Info->p.codec->elements[1] & A2DP_AAC_OCTET1_SAMPLING_FREQUENCY_44100)
                    setconfig_codec.elements[1] |= A2DP_AAC_OCTET1_SAMPLING_FREQUENCY_44100;
                else if (Info->p.codec->elements[2] & A2DP_AAC_OCTET2_SAMPLING_FREQUENCY_48000)
                    setconfig_codec.elements[2] |= A2DP_AAC_OCTET2_SAMPLING_FREQUENCY_48000;

                if (Info->p.codec->elements[2] & A2DP_AAC_OCTET2_CHANNELS_2)
                    setconfig_codec.elements[2] |= A2DP_AAC_OCTET2_CHANNELS_2;
                else if (Info->p.codec->elements[2] & A2DP_AAC_OCTET2_CHANNELS_1)
                    setconfig_codec.elements[2] |= A2DP_AAC_OCTET2_CHANNELS_1;

                setconfig_codec.elements[3] = (Info->p.codec->elements[3]) & A2DP_AAC_OCTET3_VBR_SUPPORTED;

                if (((Info->p.codec->elements[3]) & A2DP_AAC_OCTET3_VBR_SUPPORTED) &&
                    (((a2dp_codec_aac_elements[3]) & A2DP_AAC_OCTET3_VBR_SUPPORTED) == 0))
                {
                    Info->error = BTIF_A2DP_ERR_NOT_SUPPORTED_VBR;
                    DEBUG_INFO(1,"(d%x) setconfig: VBR  UNSUPPORTED!!!!!!", device_id);
                }

                uint32_t bit_rate = 0;
                bit_rate = ((Info->p.codec->elements[3]) & 0x7f) << 16;
                bit_rate |= (Info->p.codec->elements[4]) << 8;
                bit_rate |= (Info->p.codec->elements[5]);
                DEBUG_INFO(2,"(d%x) bit_rate = %d", device_id, bit_rate);
                if (bit_rate == 0) {
                    bit_rate = MAX_AAC_BITRATE;
                }
                else if(bit_rate > MAX_AAC_BITRATE) {
                    bit_rate = MAX_AAC_BITRATE;
                }

                setconfig_codec.elements[3] |= (bit_rate >> 16) &0x7f;
                setconfig_codec.elements[4] = (bit_rate >> 8) & 0xff;
                setconfig_codec.elements[5] = bit_rate & 0xff;
            }
#endif
            else if(Info->p.codec->codecType == BT_A2DP_CODEC_TYPE_NON_A2DP) {
#if defined(A2DP_SCALABLE_ON)
                //0x75 0x00 0x00 0x00 Vid
                //0x03 0x01 Codec id
                if(Info->p.codec->elements[0]==0x75 && Info->p.codec->elements[1]==0x00 && Info->p.codec->elements[2]==0x00 &&
                   Info->p.codec->elements[3]==0x00 && Info->p.codec->elements[4]==0x03 && Info->p.codec->elements[5]==0x01)
                {
                    memcpy(&setconfig_codec.elements[0], &a2dp_codec_scalable_elements[0], 6);
                    DUMP8("%02x ", (setconfig_codec.elements), 8);
                    DUMP8("%02x ", &(Info->p.codec->elements[0]), 8);
                    setconfig_codec.elements[6] = 0x00;
                    //Audio format setting
#if defined(A2DP_SCALABLE_UHQ_SUPPORT)
                    if (Info->p.codec->elements[6] & A2DP_SCALABLE_SR_96000) {
                        setconfig_codec.elements[6] |= A2DP_SCALABLE_SR_96000;
                    }
#endif
                    if (Info->p.codec->elements[6] & A2DP_SCALABLE_SR_32000) {
                        setconfig_codec.elements[6] |= A2DP_SCALABLE_SR_32000;
                    }
                    else if (Info->p.codec->elements[6] & A2DP_SCALABLE_SR_44100) {
                        setconfig_codec.elements[6] |= A2DP_SCALABLE_SR_44100;
                    }
                    else if (Info->p.codec->elements[6] & A2DP_SCALABLE_SR_48000) {
                        setconfig_codec.elements[6] |= A2DP_SCALABLE_SR_48000;
                    }

                    if (Info->p.codec->elements[6] & A2DP_SCALABLE_HQ) {
                        setconfig_codec.elements[6] |= A2DP_SCALABLE_HQ;
                    }
                    DUMP8("0x%02x ", setconfig_codec.elements, setconfig_codec.elemLen);
                }
#endif

#if defined(A2DP_LHDC_ON)
                //0x3A 0x05 0x00 0x00Vid
                //0x33 0x4c   Codec id
                if(Info->p.codec->elements[0] == a2dp_codec_lhdc_elements[0]
                    && Info->p.codec->elements[1] == a2dp_codec_lhdc_elements[1]
                    && Info->p.codec->elements[2] == a2dp_codec_lhdc_elements[2]
                    && Info->p.codec->elements[3] == a2dp_codec_lhdc_elements[3]
                    && Info->p.codec->elements[4] == a2dp_codec_lhdc_elements[4]
                    && Info->p.codec->elements[5] == a2dp_codec_lhdc_elements[5])
                {
                    memcpy(&setconfig_codec.elements[0], &a2dp_codec_lhdc_elements[0], 6);
                    DUMP8("%02x ", (setconfig_codec.elements), 8);
                    DUMP8("%02x ", &(Info->p.codec->elements[0]), 8);

                    //Audio format setting
                    //(A2DP_LHDC_SR_96000|A2DP_LHDC_SR_48000 |A2DP_LHDC_SR_44100) | (A2DP_LHDC_FMT_16),
                    if (Info->p.codec->elements[6] & A2DP_LHDC_SR_96000) {
                        setconfig_codec.elements[6] |= A2DP_LHDC_SR_96000;
                    }
                    else if (Info->p.codec->elements[6] & A2DP_LHDC_SR_48000) {
                        setconfig_codec.elements[6] |= A2DP_LHDC_SR_48000;
                    }
                    else if (Info->p.codec->elements[6] & A2DP_LHDC_SR_44100) {
                        setconfig_codec.elements[6] |= A2DP_LHDC_SR_44100;
                    }

                    if (Info->p.codec->elements[6] & A2DP_LHDC_FMT_24) {
                        setconfig_codec.elements[6] |= A2DP_LHDC_FMT_24;
                    }
                    else if (Info->p.codec->elements[6] & A2DP_LHDC_FMT_16) {
                        setconfig_codec.elements[6] |= A2DP_LHDC_FMT_16;
                    }
                }
#endif
#if defined(A2DP_LHDCV5_ON)
                if(Info->p.codec->elements[0] == a2dp_codec_lhdcv5_elements[0]
                    && Info->p.codec->elements[1] == a2dp_codec_lhdcv5_elements[1]
                    && Info->p.codec->elements[2] == a2dp_codec_lhdcv5_elements[2]
                    && Info->p.codec->elements[3] == a2dp_codec_lhdcv5_elements[3]
                    && Info->p.codec->elements[4] == a2dp_codec_lhdcv5_elements[4]
                    && Info->p.codec->elements[5] == a2dp_codec_lhdcv5_elements[5])
                {
                    memcpy(&setconfig_codec.elements[0], &a2dp_codec_lhdcv5_elements[0], 6);

                    DUMP8("%02x ", (setconfig_codec.elements), 8);
                    DUMP8("%02x ", &(Info->p.codec->elements[0]), 8);

                    if (Info->p.codec->elements[6] & A2DP_LHDCV5_SR_96000) {
                        setconfig_codec.elements[6] |= A2DP_LHDCV5_SR_96000;
                    }
                    else if (Info->p.codec->elements[6] & A2DP_LHDCV5_SR_48000) {
                        setconfig_codec.elements[6] |= A2DP_LHDCV5_SR_48000;
                    }
                    else if (Info->p.codec->elements[6] & A2DP_LHDCV5_SR_44100) {
                        setconfig_codec.elements[6] |= A2DP_LHDCV5_SR_44100;
                    }
                    else if (Info->p.codec->elements[6] & A2DP_LHDCV5_SR_192000) {
                        setconfig_codec.elements[6] |= A2DP_LHDCV5_SR_192000;
                    }

                    if (Info->p.codec->elements[7] & A2DP_LHDCV5_FMT_32) {
                        setconfig_codec.elements[7] |= A2DP_LHDCV5_FMT_32;
                    }
                    else if (Info->p.codec->elements[7] & A2DP_LHDCV5_FMT_24) {
                        setconfig_codec.elements[7] |= A2DP_LHDCV5_FMT_24;
                    }
                    else if (Info->p.codec->elements[7] & A2DP_LHDCV5_FMT_16) {
                        setconfig_codec.elements[7] |= A2DP_LHDCV5_FMT_16;
                    }
                }
#endif
#if defined(A2DP_LDAC_ON)
                //0x2d, 0x01, 0x00, 0x00, //Vendor ID
                //0xaa, 0x00,     //Codec ID
                if(Info->p.codec->elements[0]==0x2d && Info->p.codec->elements[1]==0x01 && Info->p.codec->elements[2]==0x00 &&
                   Info->p.codec->elements[3]==0x00 && Info->p.codec->elements[4]==0xaa && Info->p.codec->elements[5]==0x00)
                {
                    memcpy(&setconfig_codec.elements[0], &a2dp_codec_ldac_elements[0], 6);

                    DUMP8("%02x ", (setconfig_codec.elements), 8);
                    DUMP8("%02x ", &(Info->p.codec->elements[0]), 8);
                    //Audio format setting
                    //3c 03
                    //34 07
                    if (Info->p.codec->elements[6] & A2DP_LDAC_SR_96000) {
                        setconfig_codec.elements[6] |= A2DP_LDAC_SR_96000;
                    }
                    else if (Info->p.codec->elements[6] & A2DP_LDAC_SR_48000) {
                        setconfig_codec.elements[6] |= A2DP_LDAC_SR_48000;
                    }
                    else if (Info->p.codec->elements[6] & A2DP_LDAC_SR_44100) {
                        setconfig_codec.elements[6] |= A2DP_LDAC_SR_44100;
                    }
                    else if (Info->p.codec->elements[6] & A2DP_LDAC_SR_88200) {
                        setconfig_codec.elements[6] |= A2DP_LDAC_SR_88200;
                    }
                    if (Info->p.codec->elements[7] & A2DP_LDAC_CM_MONO) {
                        setconfig_codec.elements[7] |= A2DP_LDAC_CM_MONO;
                    }
                    else if (Info->p.codec->elements[7] & A2DP_LDAC_CM_DUAL) {
                        setconfig_codec.elements[7] |= A2DP_LDAC_CM_DUAL;
                    }
                    else if (Info->p.codec->elements[7] & A2DP_LDAC_CM_STEREO) {
                        setconfig_codec.elements[7] |= A2DP_LDAC_CM_STEREO;
                    }

                    DEBUG_INFO(2,"(d%x) setconfig_codec.elemLen = %d", device_id, setconfig_codec.elemLen);
                    DEBUG_INFO(2,"(d%x) setconfig_codec.elements[7] = 0x%02x", device_id, setconfig_codec.elements[7]);

                    DUMP8("%02x ", (setconfig_codec.elements), 8);
                }
#endif

#if defined(A2DP_LC3_ON)

#if defined(A2DP_LC3_HR)
                //0xA9 0x08 0x00 0x00  vendor id
                //0x01 0x00   Codec id
                if(Info->p.codec->elements[0]==0xA9 && Info->p.codec->elements[1]==0x08 && Info->p.codec->elements[2]==0x00 &&
                    Info->p.codec->elements[3]==0x00 && (Info->p.codec->elements[4] == 0x01) &&
                    Info->p.codec->elements[5]==0x00){
                    memcpy(&setconfig_codec.elements[0], &a2dp_codec_lc3_elements[0], 6);
                    DUMP8("%02x ", (setconfig_codec.elements), 8);
                    DUMP8("%02x ", &(Info->p.codec->elements[0]), 8);


                    if (Info->p.codec->elements[6] & A2DP_LC3_FRAME_LEN_10MS) {
                        setconfig_codec.elements[6] |= A2DP_LC3_FRAME_LEN_10MS;
                    }else if (Info->p.codec->elements[6] & A2DP_LC3_FRAME_LEN_5MS) {
                        setconfig_codec.elements[6] |= A2DP_LC3_FRAME_LEN_5MS;
                    }else if (Info->p.codec->elements[6] & A2DP_LC3_FRAME_LEN_2POINT5MS) {
                        setconfig_codec.elements[6] |= A2DP_LC3_FRAME_LEN_2POINT5MS;
                    }

                    if (Info->p.codec->elements[7] & A2DP_LC3_CH_MD_STEREO) {
                        setconfig_codec.elements[7] |= A2DP_LC3_CH_MD_STEREO;
                    }else if (Info->p.codec->elements[7] & A2DP_LC3_CH_MD_MONO) {
                        setconfig_codec.elements[7] |= A2DP_LC3_CH_MD_MONO;
                    }


                    if (Info->p.codec->elements[9] & A2DP_LC3_SR_96000) {
                        setconfig_codec.elements[9] |= A2DP_LC3_SR_96000;
                    }else if (Info->p.codec->elements[8] & A2DP_LC3_SR_48000) {
                        setconfig_codec.elements[8] |= A2DP_LC3_SR_48000;
                    }

                    if (Info->p.codec->elements[10] & A2DP_LC3_FMT_32BIT) {
                        setconfig_codec.elements[10] |= A2DP_LC3_FMT_32BIT;
                    }else if (Info->p.codec->elements[10] & A2DP_LC3_FMT_24BIT) {
                        setconfig_codec.elements[10] |= A2DP_LC3_FMT_24BIT;
                    }else if (Info->p.codec->elements[10] & A2DP_LC3_FMT_16BIT) {
                        setconfig_codec.elements[10] |= A2DP_LC3_FMT_16BIT;
                    }
                }

#else
                //0x3A 0x05 0x00 0x00Vid
                //0x4c 0x48   Codec id
                if(Info->p.codec->elements[0]==0x8F && Info->p.codec->elements[1]==0x03 && Info->p.codec->elements[2]==0x00 &&
                    Info->p.codec->elements[3]==0x00 && (Info->p.codec->elements[4] == 0xAA || Info->p.codec->elements[4] == 0xAB) &&
                    Info->p.codec->elements[5]==0x8F){
                    memcpy(&setconfig_codec.elements[0], &a2dp_codec_lc3_elements[0], 6);
                    DUMP8("%02x ", (setconfig_codec.elements), 8);
                    DUMP8("%02x ", &(Info->p.codec->elements[0]), 8);

                    //Audio format setting
                    if (Info->p.codec->elements[6] & A2DP_LC3_SR_96000) {
                        setconfig_codec.elements[6] |= A2DP_LC3_SR_96000;
                    }else if (Info->p.codec->elements[6] & A2DP_LC3_SR_48000) {
                        setconfig_codec.elements[6] |= A2DP_LC3_SR_48000;
                    }else if (Info->p.codec->elements[6] & A2DP_LC3_SR_44100) {
                        setconfig_codec.elements[6] |= A2DP_LC3_SR_44100;
                    }else if (Info->p.codec->elements[6] & A2DP_LC3_SR_32000) {
                        setconfig_codec.elements[6] |= A2DP_LC3_SR_32000;
                    }else if (Info->p.codec->elements[6] & A2DP_LC3_SR_16000) {
                        setconfig_codec.elements[6] |= A2DP_LC3_SR_16000;
                    }else if (Info->p.codec->elements[6] & A2DP_LC3_SR_8000) {
                        setconfig_codec.elements[6] |= A2DP_LC3_SR_8000;
                    }

                    if (Info->p.codec->elements[7] & A2DP_LC3_FMT_32BIT) {
                        setconfig_codec.elements[7] |= A2DP_LC3_FMT_32BIT;
                    }else if (Info->p.codec->elements[7] & A2DP_LC3_FMT_24BIT) {
                        setconfig_codec.elements[7] |= A2DP_LC3_FMT_24BIT;
                    }else if (Info->p.codec->elements[7] & A2DP_LC3_FMT_16BIT) {
                        setconfig_codec.elements[7] |= A2DP_LC3_FMT_16BIT;
                    }

                    if (Info->p.codec->elements[7] & A2DP_LC3_FRAME_LEN_10MS) {
                        setconfig_codec.elements[7] |= A2DP_LC3_FRAME_LEN_10MS;
                    }else if (Info->p.codec->elements[7] & A2DP_LC3_FRAME_LEN_7POINT5MS) {
                        setconfig_codec.elements[7] |= A2DP_LC3_FRAME_LEN_7POINT5MS;
                    }else if (Info->p.codec->elements[7] & A2DP_LC3_FRAME_LEN_5MS) {
                        setconfig_codec.elements[7] |= A2DP_LC3_FRAME_LEN_5MS;
                    }else if (Info->p.codec->elements[7] & A2DP_LC3_FRAME_LEN_2POINT5MS) {
                        setconfig_codec.elements[7] |= A2DP_LC3_FRAME_LEN_2POINT5MS;
                    }

                    if (Info->p.codec->elements[8] & A2DP_LC3_BITRATE_900kBPS) {
                        setconfig_codec.elements[8] |= A2DP_LC3_BITRATE_900kBPS;
                    }else if (Info->p.codec->elements[8] & A2DP_LC3_BITRATE_600kBPS) {
                        setconfig_codec.elements[8] |= A2DP_LC3_BITRATE_600kBPS;
                    }else if (Info->p.codec->elements[8] & A2DP_LC3_BITRATE_500kBPS) {
                        setconfig_codec.elements[8] |= A2DP_LC3_BITRATE_500kBPS;
                    }else if (Info->p.codec->elements[8] & A2DP_LC3_BITRATE_400kBPS) {
                        setconfig_codec.elements[8] |= A2DP_LC3_BITRATE_400kBPS;
                    }else if (Info->p.codec->elements[8] & A2DP_LC3_BITRATE_300kBPS) {
                        setconfig_codec.elements[8] |= A2DP_LC3_BITRATE_300kBPS;
                    }else if (Info->p.codec->elements[8] & A2DP_LC3_BITRATE_200kBPS) {
                        setconfig_codec.elements[8] |= A2DP_LC3_BITRATE_200kBPS;
                    }else if (Info->p.codec->elements[8] & A2DP_LC3_BITRATE_100kBPS) {
                        setconfig_codec.elements[8] |= A2DP_LC3_BITRATE_100kBPS;
                    }else if (Info->p.codec->elements[8] & A2DP_LC3_BITRATE_64kBPS) {
                        setconfig_codec.elements[8] |= A2DP_LC3_BITRATE_64kBPS;
                    }

                    if (Info->p.codec->elements[9] & A2DP_LC3_CH_MD_STEREO) {
                        setconfig_codec.elements[9] |= A2DP_LC3_CH_MD_STEREO;
                    }else if (Info->p.codec->elements[9] & A2DP_LC3_CH_MD_MONO) {
                        setconfig_codec.elements[9] |= A2DP_LC3_CH_MD_MONO;
                    }else if (Info->p.codec->elements[9] & A2DP_LC3_CH_MD_MUlTI_MONO) {
                        setconfig_codec.elements[9] |= A2DP_LC3_CH_MD_MUlTI_MONO;
                    }
                }
#endif
#endif
            }
            break;
        case BTIF_A2DP_EVENT_GET_CONFIG_IND:
            DEBUG_INFO(2,"(d%x) ::A2DP_EVENT_GET_CONFIG_IND %d\n", device_id, Info->event);
            break;
        case BTIF_A2DP_EVENT_STREAM_RECONFIG_IND:
            if(curr_device->a2dp_status_recheck & A2DP_STATUS_SYNC_CODEC_STATE)
            {
                curr_device->a2dp_status_recheck &= ~(A2DP_STATUS_SYNC_CODEC_STATE);
            }
            DEBUG_INFO(3,"(d%x) ::A2DP_EVENT_STREAM_RECONFIG_IND %d codec %d", device_id, Info->event, Info->p.configReq->codec.codecType);
            if (Info->p.configReq->codec.codecType == BT_A2DP_CODEC_TYPE_NON_A2DP)
            {
                uint8_t recfg_a2dp_non_type = a2dp_get_current_codec_non_type(Info->p.configReq->codec.elements);
                curr_device->codec_type = BT_A2DP_CODEC_TYPE_NON_A2DP;
                app_a2dp_set_vender_codec_non_type(device_id, recfg_a2dp_non_type);
                if (recfg_a2dp_non_type == BT_A2DP_CODEC_NONE_TYPE_SCALABLE)
                {
#if defined(A2DP_SCALABLE_ON)
                    DEBUG_INFO(1,"(d%x) ::##SCALABLE A2DP_EVENT_STREAM_RECONFIG_IND", device_id);
                    a2dp_scalable_config(device_id, &(Info->p.configReq->codec.elements[0]));
                    //0x75 0x00 0x00 0x00Vid
                    //0x03 0x01   Codec id
                    if(Info->p.codec->elements[0]==0x75 && Info->p.codec->elements[4]==0x03 && Info->p.codec->elements[5]==0x01)
                    {
                        setconfig_codec.elements = a2dp_scalable_avdtpcodec.elements;
                    }
                    else
                    {
                        if(Info->p.codec->pstreamflags!=NULL) {
                            Info->p.codec->pstreamflags[0] &= ~APP_A2DP_STRM_FLAG_QUERY_CODEC;
                        }
                        else {
                            ASSERT(false, "pstreamflags not init ..");
                        }
                    }
                    curr_device->a2dp_channel_num = 2;
#endif // A2DP_SCALABLE_ON
                }
                else if(recfg_a2dp_non_type == BT_A2DP_CODEC_NONE_TYPE_LHDC || recfg_a2dp_non_type == BT_A2DP_CODEC_NONE_TYPE_LHDCV5)
                {
#if defined(A2DP_LHDC_ON)
                    if (recfg_a2dp_non_type == BT_A2DP_CODEC_NONE_TYPE_LHDC)
                    {
                        DEBUG_INFO(2,"(d%x) ::A2DP_EVENT_STREAM_RECONFIG_IND ##LHDC len %d", device_id, Info->p.configReq->codec.elemLen);
                        curr_device->a2dp_channel_num = 2;
                        a2dp_lhdc_config(device_id, &(Info->p.configReq->codec.elements[0]));
                    }
#endif  // A2DP_LHDC_ON
#if defined(A2DP_LHDCV5_ON)
                    if(recfg_a2dp_non_type == BT_A2DP_CODEC_NONE_TYPE_LHDCV5)
                    {
                        DEBUG_INFO(2,"(d%x) ::A2DP_EVENT_STREAM_RECONFIG_IND ##LHDCV5 len %d", device_id, Info->p.configReq->codec.elemLen);
                        curr_device->a2dp_channel_num = 2;
                        a2dp_lhdcv5_config(device_id, &(Info->p.configReq->codec.elements[0]));
                    }
#endif // A2DP_LHDCV5_ON
                }
                else if (recfg_a2dp_non_type == BT_A2DP_CODEC_NONE_TYPE_LDAC)
                {
#if defined(A2DP_LDAC_ON)
                    curr_device->a2dp_channel_num = 2;
                    a2dp_ldac_config(device_id, &(Info->p.configReq->codec.elements[0]));
#endif // A2DP_LDAC_ON
                }
                else if (recfg_a2dp_non_type == BT_A2DP_CODEC_NONE_TYPE_LC3)
                {
#if defined(A2DP_LC3_ON)
                    DEBUG_INFO(0,"::##LC3 A2DP_EVENT_STREAM_RECONFIG_IND\n");
                    a2dp_lc3_config(device_id,&(Info->p.configReq->codec.elements[0]));
#endif // A2DP_LC3_ON
                }
                else if (recfg_a2dp_non_type == BT_A2DP_CODEC_NONE_TYPE_L2HC)
                {
#if defined(A2DP_L2HC_ON)
                    DEBUG_INFO(2,"##L2HC A2DP_EVENT_STREAM_RECONFIG_IND");
                    a2dp_l2hc_config(device_id, &(Info->p.configReq->codec.elements[0]));
#endif // A2DP_L2HC_ON
                }
                else if (recfg_a2dp_non_type == BT_A2DP_CODEC_NONE_TYPE_MIHC)
                {
#if defined(A2DP_MIHC_ON)
                    DEBUG_INFO(2,"##MIHC A2DP_EVENT_STREAM_RECONFIG_IND");
                    a2dp_mihc_config(device_id, &(Info->p.configReq->codec.elements[0]));
#endif // A2DP_MIHC_ON
                }
                else
                {
                    DEBUG_WARNING(3,"(d%x) WARNING unknown Codec, Element length = %d", device_id, Info->p.configReq->codec.elemLen);
                    curr_device->codec_type = BT_A2DP_CODEC_NONE_TYPE_INVALID;
                    app_a2dp_set_vender_codec_non_type(device_id, recfg_a2dp_non_type);
                }
            }
            else if (Info->p.configReq->codec.codecType == BT_A2DP_CODEC_TYPE_MPEG2_4_AAC)
            {
#if defined(A2DP_AAC_ON)
                DEBUG_INFO(1,"(d%x) ::A2DP_EVENT_STREAM_RECONFIG_IND ##AAC", device_id);
                if (((Info->p.codec->elements[3]) & A2DP_AAC_OCTET3_VBR_SUPPORTED) &&
                    (((a2dp_codec_aac_elements[3]) & A2DP_AAC_OCTET3_VBR_SUPPORTED) == 0))
                {
                    Info->error = BTIF_A2DP_ERR_NOT_SUPPORTED_VBR;
                    DEBUG_INFO(1,"(d%x) stream reconfig: VBR  UNSUPPORTED!!!!!!", device_id);
                }
                curr_device->codec_type = BT_A2DP_CODEC_TYPE_MPEG2_4_AAC;
                app_a2dp_set_vender_codec_non_type(device_id, BT_A2DP_CODEC_NONE_TYPE_INVALID);
                curr_device->sample_bit = 16;
                // convert aac sample_rate to sbc sample_rate format
                if (Info->p.configReq->codec.elements[1] & A2DP_AAC_OCTET1_SAMPLING_FREQUENCY_44100) {
                    DEBUG_INFO(1,"(d%x) ::A2DP_EVENT_STREAM_OPEN  aac sample_rate 44100", device_id);
                    curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_44;
                }
                else if (Info->p.configReq->codec.elements[2] & A2DP_AAC_OCTET2_SAMPLING_FREQUENCY_48000) {
                    DEBUG_INFO(1,"(d%x) ::A2DP_EVENT_STREAM_OPEN aac sample_rate 48000", device_id);
                    curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_48;
                }
                else {
                    DEBUG_INFO(1,"(d%x) ::A2DP_EVENT_STREAM_OPEN aac sample_rate not 48000 or 44100, set to 44100", device_id);
                    curr_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_44;
                }
                if (Info->p.configReq->codec.elements[2] & A2DP_AAC_OCTET2_CHANNELS_1){
                    curr_device->a2dp_channel_num = 1;
                }else{
                    curr_device->a2dp_channel_num = 2;
                }
#endif // A2DP_AAC_ON
            }
            else if (Info->p.configReq->codec.codecType == BT_A2DP_CODEC_TYPE_SBC)
            {
                DEBUG_INFO(6,"(d%x) ::A2DP_EVENT_STREAM_RECONFIG_IND ##SBC sample_rate::elements[0] %d BITPOOL:%d/%d %02x/%02x",
                        device_id,
                        Info->p.configReq->codec.elements[0],
                        Info->p.configReq->codec.elements[2],
                        Info->p.configReq->codec.elements[3],
                        Info->p.configReq->codec.elements[2],
                        Info->p.configReq->codec.elements[3]);

                curr_device->codec_type = BT_A2DP_CODEC_TYPE_SBC;
                app_a2dp_set_vender_codec_non_type(device_id, BT_A2DP_CODEC_NONE_TYPE_INVALID);
                curr_device->sample_bit = 16;
                curr_device->sample_rate = (Info->p.configReq->codec.elements[0] & A2D_SBC_IE_SAMP_FREQ_MSK);

                if (Info->p.configReq->codec.elements[0] & A2D_SBC_IE_CH_MD_MONO)
                    curr_device->a2dp_channel_num  = 1;
                else
                    curr_device->a2dp_channel_num = 2;
            }
            break;
#ifdef __A2DP_AVDTP_CP__
        case BTIF_A2DP_EVENT_CP_INFO:
            DEBUG_INFO(3,"(d%x) ::A2DP_EVENT_CP_INFO %d cpType: %x", device_id, Info->event, Info->p.cp->cpType);
            if(Info->p.cp && Info->p.cp->cpType == BTIF_AVDTP_CP_TYPE_SCMS_T)
            {
                curr_device->avdtp_cp = 1;
            }
            else
            {
                curr_device->avdtp_cp = 0;
            }
            btif_a2dp_set_copy_protection_enable(Stream, curr_device->avdtp_cp);
            break;
        case BTIF_A2DP_EVENT_STREAM_SECURITY_IND:
            DEBUG_INFO(2,"(d%x) ::A2DP_EVENT_STREAM_SECURITY_IND %d", device_id, Info->event);
            DUMP8("%x ",Info->p.data,Info->len);
             btif_a2dp_security_control_rsp(Stream,&Info->p.data[1],Info->len-1,Info->error);
            break;
        case BTIF_A2DP_EVENT_STREAM_SECURITY_CNF:
            curr_device->avdtp_cp = 1;
            DEBUG_INFO(2,"(d%x) ::A2DP_EVENT_STREAM_SECURITY_CNF %d", device_id, Info->event);
            break;
#endif
    }

    app_a2dp_bt_driver_callback(device_id, Info->event);

#if defined(IBRT)
    app_tws_ibrt_profile_callback(device_id, BTIF_APP_A2DP_PROFILE_ID, (void *)Stream, (void *)info,&curr_device->remote);
#endif

#if defined(IBRT)
    uint8_t *codec_info = (uint8_t *)btif_a2dp_get_stream_codec(curr_device->a2dp_connected_stream);
    uint8_t *cp_info = (uint8_t *)btif_a2dp_get_stream_cp_info(curr_device->a2dp_connected_stream);
    if (Info->event == BTIF_A2DP_EVENT_STREAM_OPEN_MOCK)
    {
        bt_a2dp_opened_param_t param;
        param.error_code = 0;
        param.codec_type = codec_info[0];
        param.codec_info_len = codec_info[1];
        param.codec_info = codec_info + 2;
        if (besbt_cfg.avdtp_cp_enable && cp_info)
        {
            param.cp_info = cp_info + 3;
            param.cp_info_len = cp_info[1];
            param.cp_type = cp_info[0];
        }
        else
        {
            param.cp_info = NULL;
            param.cp_info_len = 0;
            param.cp_type = 0;
        }
        btif_report_bt_event(&curr_device->remote, BT_EVENT_A2DP_OPENED, &param);
    }
    else if (Info->event == BTIF_A2DP_EVENT_STREAM_STARTED_MOCK)
    {
        bt_a2dp_stream_start_param_t param;
        param.error_code = 0;
        param.codec_type = codec_info[0];
        param.codec_info_len = codec_info[1];
        param.codec_info = codec_info + 2;
        if (besbt_cfg.avdtp_cp_enable && cp_info)
        {
            param.cp_info = cp_info + 3;
            param.cp_info_len = cp_info[1];
            param.cp_type = cp_info[0];
        }
        else
        {
            param.cp_info = NULL;
            param.cp_info_len = 0;
            param.cp_type = 0;
        }
        btif_report_bt_event(&curr_device->remote, BT_EVENT_A2DP_STREAM_START, &param);
    }
#endif
}

void app_pts_av_disc_channel(void)
{
    btif_a2dp_close_stream(app_bt_get_device(BT_DEVICE_ID_1)->btif_a2dp_stream->a2dp_stream);
}

void app_pts_av_close_channel(void)
{
    btif_a2dp_close_stream_for_PTS(app_bt_get_device(BT_DEVICE_ID_1)->btif_a2dp_stream->a2dp_stream);
}

bool a2dp_is_music_ongoing(void)
{
    struct BT_DEVICE_T *device = NULL;

    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        device = app_bt_get_device(i);
        if (device->a2dp_streamming)
        {
            return true;
        }
    }

    return false;
}

#if defined(A2DP_LHDC_ON)||defined(A2DP_LHDCV5_ON)

/*
testkey_bin

BES2000iZ
Start: 11 11 22 33 33 33
End: 111122333396
Quantity: 100
License Key: 3F0F524EA8270008A036B254
*/

extern "C" uint8_t* factory_section_get_bt_name(void);
extern "C" uint8_t* factory_section_get_ble_name(void);

const uint8_t lhdc_test_bt_addr[]  = {0x45,0x33,0x33,0x22,0x11,0x11};
const uint8_t lhdc_test_ble_addr[] = {0x45,0x33,0x33,0x22,0x11,0x11};

int bes_bt_local_info_get(bes_bt_local_info *local_info)
{
#if 1
    uint8_t *bt_name;
    uint8_t *ble_name;

    bt_name = factory_section_get_bt_name();
    ble_name = factory_section_get_ble_name();

    local_info->bt_len = strlen((char *)bt_name)+1;
    local_info->ble_len = strlen((char *)ble_name)+1;

    memcpy(local_info->bt_addr, bt_global_addr, BTIF_BD_ADDR_SIZE);
    memcpy((void*)local_info->bt_name, (const void*)bt_name, local_info->bt_len);

    memcpy(local_info->ble_addr, ble_global_addr, BTIF_BD_ADDR_SIZE);
    memcpy((void*)local_info->ble_name, (const void*)ble_name, local_info->ble_len);
#else
    const char *bt_name2 ="BES_LHDC_TEST";

    memcpy(local_info->bt_addr, lhdc_test_bt_addr, BTIF_BD_ADDR_SIZE);
//    local_info->bt_name = (const char *)bt_name2;
    local_info->bt_len = strlen(bt_name2)+1;
    memcpy((void *)local_info->bt_name, (void *)bt_name2, local_info->bt_len);

    memcpy(local_info->ble_addr, lhdc_test_ble_addr, BTIF_BD_ADDR_SIZE);
 //   local_info->ble_name = (const char *)bt_name2;
    local_info->ble_len = strlen(bt_name2)+1;
    memcpy((void *)local_info->ble_name, (void *)bt_name2, local_info->ble_len);
#endif
    DEBUG_INFO(0,"LHDC_info_get addr");
    DUMP8("%02x ", local_info->bt_addr, BT_ADDR_OUTPUT_PRINT_NUM);
    DEBUG_INFO(1,"LHDC_info_get name:%s", local_info->bt_name);
    DEBUG_INFO(1,"LHDC_info_get len:%d", local_info->bt_len);

    return 0;
}

#endif /* A2DP_LHDC_ON */

#ifdef IBRT
static bool isRejectSniffGuardTimerOn = false;
static void app_a2dp_reject_sniff_guard_timer_callback(void const *param)
{
    uint8_t curr_device_id = *(uint8_t *)param;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(curr_device_id);

    if(curr_device){
        app_ibrt_if_prevent_sniff_clear((uint8_t *)&curr_device->remote, AVRCP_STATUS_CHANING);
    }

    DEBUG_INFO(1,"AVRCP: reject sniff timer out %p", curr_device);
    a2dp_sniff_reject_device = BT_DEVICE_NUM;
    isRejectSniffGuardTimerOn = false;
}

void app_a2dp_reject_sniff_start(uint8_t device_id, uint32_t timeout)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (curr_device && a2dp_sniff_reject_guard_timer){
        a2dp_sniff_reject_device = device_id;
        if(!isRejectSniffGuardTimerOn){
            app_ibrt_if_prevent_sniff_set((uint8_t *)&curr_device->remote, AVRCP_STATUS_CHANING);
        }
        isRejectSniffGuardTimerOn = true;
        osTimerStop(a2dp_sniff_reject_guard_timer);
        osTimerStart(a2dp_sniff_reject_guard_timer, timeout);
        DEBUG_INFO(1,"AVRCP: start reject sniff timer%p", curr_device);
    }
}
#endif
#else /* BT_A2DP_SUPPORT */

#include "app_a2dp.h"

void a2dp_callback(uint8_t device_id, a2dp_stream_t *Stream, const a2dp_callback_parms_t *info)
{

}

extern "C" uint16_t a2dp_Get_curr_a2dp_conhdl(void)
{
    return app_bt_manager.current_a2dp_conhdl;
}

uint8_t app_bt_a2dp_adjust_volume(uint8_t device_id, bool up, bool adjust_local_vol_level)
{
    return TGT_VOLUME_LEVEL_MUTE;
}

uint8_t a2dp_convert_local_vol_to_bt_vol(uint8_t localVol)
{
    return 0;
}

int a2dp_bt_stream_need_autotrigger_getandclean_flag(void)
{
    return 0;
}

bool app_bt_is_a2dp_disconnected(uint8_t device_id)
{
    return false;
}
#endif /* BT_A2DP_SUPPORT */
