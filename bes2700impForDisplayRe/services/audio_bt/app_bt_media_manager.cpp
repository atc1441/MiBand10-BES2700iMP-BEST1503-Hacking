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
#include <stdio.h>
#include <assert.h>

#include "cmsis_os.h"
#include "cmsis.h"
#include "hal_uart.h"
#include "hal_timer.h"
#include "hal_chipid.h"
#include "audioflinger.h"
#include "lockcqueue.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "analog.h"
#include "app_bt_stream.h"
#include "app_overlay.h"
#include "app_audio.h"
#include "app_utils.h"
#include "bluetooth_bt_api.h"
#include "nvrecord_bt.h"
#if defined(IBRT)
#include "app_ibrt_internal.h"
#else
#include "app_media_player.h"
#endif

#ifdef MEDIA_PLAYER_SUPPORT
#include "app_media_player.h"
#endif

#include "bt_drv.h"
#include "apps.h"
#include "app_bt_func.h"
#include "app_bt.h"

#include "besbt.h"

#include "cqueue.h"
#include "btapp.h"
#include "app_hfp.h"

#include "app_bt_media_manager.h"
#include "app_thread.h"

#include "app_ring_merge.h"
#include "bt_if.h"
#include "audio_prompt_sbc.h"
#ifdef __AI_VOICE__
#include "app_ai_if.h"
#include "app_ai_voice.h"
#endif
#include "app_bt_stream.h"
#if defined(USE_BASIC_THREADS)
#include "app_thread.h"
#else
#include "app_audio_thread.h"
#endif
#include "app_audio_active_device_manager.h"
#include "app_audio_focus_control.h"
#include "app_audio_control.h"
#include "audio_trigger_common.h"
#include "audio_policy.h"

static int bt_audio_media_play(uint16_t stream_type, const Audio_device_t* device)
{
    if (device->audio_device.device_type == AUDIO_TYPE_BT)
    {
        return app_audio_manager_sendrequest(
            APP_BT_STREAM_MANAGER_START,stream_type, device->audio_device.device_id, device->aud_id);
    }
#if BLE_AUDIO_ENABLED
    if (device->audio_device.device_type == AUDIO_TYPE_LE_AUDIO)
    {
        return app_audio_manager_sendrequest(
            APP_BT_BLE_STREAM_MANAGER_START,stream_type, device->audio_device.device_id, device->aud_id);
    }
#endif
#ifdef BLE_WALKIE_TALKIE
    if (device->audio_device.device_type == AUDIO_TYPE_WALKIE_TALKIE)
    {
        return app_audio_manager_sendrequest(
            APP_BT_WT_STREAM_MANAGER_START,stream_type, device->audio_device.device_id, device->aud_id);
    }
#endif
    if (device->audio_device.device_type == AUDIO_TYPE_BIS_TRANSPORT)
    {
        return app_audio_manager_sendrequest(
            APP_BT_BIS_TRAN_STREAM_MANAGER_START,stream_type, device->audio_device.device_id, device->aud_id);
    }
    TRACE(0,"App_bt_media play error");
    return -1;
}

static int bt_audio_media_stop(uint16_t stream_type, const Audio_device_t* device)
{
    if (device->audio_device.device_type == AUDIO_TYPE_BT)
    {
        return app_audio_manager_sendrequest(
            APP_BT_STREAM_MANAGER_STOP,stream_type, device->audio_device.device_id, device->aud_id);
    }
#if BLE_AUDIO_ENABLED
    if (device->audio_device.device_type == AUDIO_TYPE_LE_AUDIO)
    {
        return app_audio_manager_sendrequest(
            APP_BT_BLE_STREAM_MANAGER_STOP,stream_type, device->audio_device.device_id, device->aud_id);
    }
#endif
#ifdef BLE_WALKIE_TALKIE
    if (device->audio_device.device_type == AUDIO_TYPE_WALKIE_TALKIE)
    {
        return app_audio_manager_sendrequest(
            APP_BT_WT_STREAM_MANAGER_STOP,stream_type, device->audio_device.device_id, device->aud_id);
    }
#endif
    if (device->audio_device.device_type == AUDIO_TYPE_BIS_TRANSPORT)
    {
        return app_audio_manager_sendrequest(
            APP_BT_BIS_TRAN_STREAM_MANAGER_STOP,stream_type, device->audio_device.device_id, device->aud_id);
    }

    if (device->audio_device.device_type == AUDIO_TYPE_UNSPECIFIED)
    {
        return app_audio_manager_sendrequest(
            APP_BT_STREAM_MANAGER_STOP_MEDIA,stream_type, device->audio_device.device_id, device->aud_id);
    }
    TRACE(0,"App_bt_media stop error");

    return -1;
}

void bt_audio_local_volume_up()
{
    app_audio_manager_ctrl_volume_with_callback(APP_AUDIO_MANAGER_VOLUME_CTRL_UP, 0, NULL);
}

void bt_audio_volume_down()
{
    app_audio_manager_ctrl_volume_with_callback(APP_AUDIO_MANAGER_VOLUME_CTRL_DOWN, 0, NULL);
}

static int bt_audio_local_volume_set(uint16_t volume)
{
    return app_audio_manager_ctrl_volume_with_callback(APP_AUDIO_MANAGER_VOLUME_CTRL_SET, volume, NULL);
}

static int bt_audio_swap_sco(uint16_t stream_type,uint8_t device_id)
{
    return  app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_SWAP_SCO, BT_STREAM_VOICE, device_id, 0);
}

static void bt_audio_restore_volume(uint8_t *bdAddr)
{
    app_bt_stream_volume_ptr_update(bdAddr);
}

static void bt_audio_set_output_route(Audio_output_route_t *route_info)
{
    app_audio_manager_set_output_route(route_info);
}

static bool bt_audio_is_continue_media(uint32_t id)
{
    bool is_continue_id = false;
    switch(id)
    {
        case AUD_ID_BT_CALL_INCOMING_CALL:
            is_continue_id = true;
        break;
        default:
        break;
    }
    return is_continue_id;
}

static void bt_audio_play_media(uint32_t id,uint8_t device_id)
{
#ifdef MEDIA_PLAYER_SUPPORT
    if(bt_audio_is_continue_media(id))
    {
        media_PlayAudio_continuous_start((AUD_ID_ENUM)id, device_id);
    }
    else
    {
        media_PlayAudio((AUD_ID_ENUM)id, device_id);
    }
#endif
}

static void bt_audio_start_capture(const Audio_device_t* device)
{

}

static void bt_audio_stop_capture(const Audio_device_t* device)
{

}

static void bt_audio_print_playback_status()
{
    app_bt_audio_state_checker();
}

static Audio_Player_Adapter_t app_bt_media_player =
{
    .play = bt_audio_media_play,
    .stop = bt_audio_media_stop,
    .pause = NULL,
    .play_prompt = bt_audio_play_media,
    .seek  = NULL,
    .dump_playback_status = bt_audio_print_playback_status,
    .set_volume = bt_audio_local_volume_set,
    .volume_up = bt_audio_local_volume_up,
    .volume_down = bt_audio_volume_down,
    .restore_volume = bt_audio_restore_volume,
    .swap_sco = bt_audio_swap_sco,
    .set_output_route = bt_audio_set_output_route,
    .start_capture = bt_audio_start_capture,
    .stop_capture = bt_audio_stop_capture,
};

static void bt_media_player_register()
{
    audio_player_adapter_init(&app_bt_media_player);
}

int bt_sco_player_forcemute(bool mic_mute, bool spk_mute);
extern enum AUD_SAMPRATE_T a2dp_sample_rate;

struct bt_media_manager {
    uint8_t media_curr_music;
    uint8_t media_curr_sco;
    uint16_t curr_active_media; // low 8 bits are out direciton, while high 8 bits are in direction
};

static char _strm_type_str[168];
static char *_catstr(char *dst, const char *src) {
     while(*dst) dst++;
     while((*dst++ = *src++));
     return --dst;
}
const char *strmtype2str(uint16_t stream_type) {
    const char *s = NULL;
    char _cat = 0, first = 1, *d = NULL;
    _strm_type_str[0] = '\0';
    d = _strm_type_str;
    d = _catstr(d, "[");
    if (stream_type != 0) {
        for (int i = 15 ; i >= 0; i--) {
            _cat = 1;
            //TRACE(3,"i=%d,stream_type=0x%d,stream_type&(1<<i)=0x%x", i, stream_type, stream_type&(1<<i));
            switch(stream_type&(1<<i)) {
                case 0: _cat = 0; break;
                case BT_STREAM_MUSIC: s = "MUSIC"; break;
                case BT_STREAM_MEDIA: s = "MEDIA"; break;
                case BT_STREAM_VOICE: s = "VOICE"; break;
                #ifdef RB_CODEC
                case BT_STREAM_RBCODEC: s = "RB_CODEC"; break;
                #endif
                // direction is in
                #ifdef __AI_VOICE__
                case BT_STREAM_AI_VOICE: s = "AI_VOICE"; break;
                #endif
                #ifdef AUDIO_LINEIN
                case BT_STREAM_LINEIN: s  = "LINEIN"; break;
                #endif
                #ifdef AUDIO_PCM_PLAYER
                case BT_STREAM_PCM_PLAYER: s  = "PCM_PLAYER"; break;
                #endif
                default:  s = "UNKNOWN"; break;
            }
            if (_cat) {
                if (!first)
                    d = _catstr(d, "|");
                //TRACE(2,"d=%s,s=%s", d, s);
                d = _catstr(d, s);
                first = 0;
            }
        }
    }

    _catstr(d, "]");

    return _strm_type_str;
}

static const char *handleId2str(uint8_t id) {
    #define CASE_M(s) \
        case s: return "["#s"]";

    switch(id) {
    CASE_M(APP_BT_STREAM_MANAGER_START)
    CASE_M(APP_BT_STREAM_MANAGER_STOP)
    CASE_M(APP_BT_STREAM_MANAGER_STOP_MEDIA)
    CASE_M(APP_BT_STREAM_MANAGER_UPDATE_MEDIA)
    CASE_M(APP_BT_STREAM_MANAGER_SWAP_SCO)
    CASE_M(APP_BT_STREAM_MANAGER_CTRL_VOLUME)
    CASE_M(APP_BT_STREAM_MANAGER_TUNE_SAMPLERATE_RATIO)
    CASE_M(APP_BT_BLE_STREAM_MANAGER_START)
    CASE_M(APP_BT_BLE_STREAM_MANAGER_STOP)
    /*
    CASE_M(BT_STREAM_START)
    CASE_M(BT_STREAM_STOP)
    CASE_M(BT_STREAM_STOP_MEDIA)
    CASE_M(BT_STREAM_UPDATE_MEDIA)
    CASE_M(BT_STREAM_SWAP_SCO)
    CASE_M(BT_STREAM_CTRL_VOLUME)
    CASE_M(BT_STREAM_TUNE_SAMPLERATE_RATIO)
    CASE_M(BLE_STREAM_START)
    CASE_M(BLE_STREAM_STOP)
    */
    }

    return "[]";
}

extern enum AUD_SAMPRATE_T sco_sample_rate;
static struct bt_media_manager bt_meida;

uint8_t app_audio_manager_get_a2dp_codec_type(uint8_t device_id)
{
    return bta_get_a2dp_codec_type_by_id(device_id);
}

uint8_t app_audio_manager_get_a2dp_non_type(uint8_t device_id)
{
    uint8_t vender_codec_type = BT_A2DP_CODEC_NONE_TYPE_INVALID;
    struct BT_DEVICE_T * curr_device = NULL;
    curr_device = app_bt_get_device(device_id);
    if (curr_device)
    {
        vender_codec_type = curr_device->a2dp_non_type;
    }
    return vender_codec_type;
}

static uint16_t g_bt_media_active[BT_DEVICE_NUM+BT_SOURCE_DEVICE_NUM];

uint16_t bt_media_get_media_active(int device_id)
{
    if (device_id < BT_DEVICE_NUM)
    {
        return g_bt_media_active[device_id];
    }
    else if (device_id >= BT_SOURCE_DEVICE_ID_1 && device_id < BT_SOURCE_DEVICE_ID_N)
    {
        return g_bt_media_active[BT_DEVICE_NUM+(device_id-BT_SOURCE_DEVICE_ID_BASE)];
    }
    else
    {
        return 0;
    }
}

void bt_media_set_media_active_clear(int device_id, uint16_t media_type)
{
    if (device_id < BT_DEVICE_NUM)
    {
        g_bt_media_active[device_id] &= (~media_type);
    }
    else if (device_id >= BT_SOURCE_DEVICE_ID_1 && device_id < BT_SOURCE_DEVICE_ID_N)
    {
        g_bt_media_active[BT_DEVICE_NUM+(device_id-BT_SOURCE_DEVICE_ID_BASE)] &= (~media_type);
    }
}

void bt_media_set_media_active_mask(int device_id, uint16_t media_type)
{
    if (device_id < BT_DEVICE_NUM)
    {
        g_bt_media_active[device_id] |= media_type;
    }
    else if (device_id >= BT_SOURCE_DEVICE_ID_1 && device_id < BT_SOURCE_DEVICE_ID_N)
    {
        g_bt_media_active[BT_DEVICE_NUM+(device_id-BT_SOURCE_DEVICE_ID_BASE)] |= media_type;
    }
}

uint8_t bt_media_is_media_active_by_type(uint16_t media_type)
{
    uint8_t i;
    for(i = 0; i < BT_DEVICE_NUM; i++)
    {
        if(bt_media_get_media_active(i) & media_type)
            return 1;
    }
    for (i = BT_SOURCE_DEVICE_ID_BASE; i < BT_SOURCE_DEVICE_ID_BASE + BT_SOURCE_DEVICE_NUM; i += 1)
    {
        if(bt_media_get_media_active(i) & media_type)
            return 1;
    }
    return 0;
}

bool bt_media_is_media_idle(void)
{
    uint8_t i;
    for(i = 0; i < BT_DEVICE_NUM; i++)
    {
        if(bt_media_get_media_active(i) != 0)
            return false;
    }
    for (i = BT_SOURCE_DEVICE_ID_BASE; i < BT_SOURCE_DEVICE_ID_BASE + BT_SOURCE_DEVICE_NUM; i += 1)
    {
        if(bt_media_get_media_active(i) != 0)
            return false;
    }
    return true;
}
void bt_media_clean_up(void)
{
    uint32_t lock = int_lock();

    uint8_t i = 0;
    bt_meida.curr_active_media = 0;
    for (i = 0; i < BT_DEVICE_NUM + BT_SOURCE_DEVICE_NUM; i++)
    {
        g_bt_media_active[i] = 0;
    }
    int_unlock(lock);
}

bool bt_media_is_media_active_by_music(void)
{
    uint8_t i;
    for (i = 0; i < BT_DEVICE_NUM; i++)
    {
        if (bt_media_get_media_active(i) & BT_STREAM_MUSIC)
        {
            return true;
        }
    }
    return false;
}

bool bt_media_is_media_active_by_call(void)
{
    uint8_t i;
    for (i = 0; i < BT_DEVICE_NUM; i++)
    {
        if (bt_media_get_media_active(i) & BT_STREAM_VOICE)
        {
            return true;
        }
    }
    return false;
}

bool bt_is_sco_media_open(void)
{
    return (bt_meida.curr_active_media & BT_STREAM_VOICE)?(true):(false);
}

static int bt_media_get_active_device_by_type(uint16_t media_type)
{
    uint8_t i;
    for(i = 0; i < BT_DEVICE_NUM; i++)
    {
        if(bt_media_get_media_active(i) & media_type)
            return i;
    }
    for (i = BT_SOURCE_DEVICE_ID_BASE; i < BT_SOURCE_DEVICE_ID_BASE + BT_SOURCE_DEVICE_NUM; i += 1)
    {
        if(bt_media_get_media_active(i) & media_type)
            return i;
    }
    return BT_DEVICE_INVALID_ID;
}

uint8_t bt_media_is_media_active_by_device(uint16_t media_type, int device_id)
{
    if (bt_media_get_media_active(device_id) & media_type)
        return 1;
    return 0;
}

uint16_t bt_media_get_current_media(void)
{
    return bt_meida.curr_active_media;
}

bool bt_media_cur_is_bt_stream_music(void)
{
    return (BT_STREAM_MUSIC & bt_meida.curr_active_media)?(true):(false);
}

#ifdef AUDIO_LINEIN
bool bt_media_cur_is_linein_stream(void)
{
    return (BT_STREAM_LINEIN & bt_meida.curr_active_media)?(true):(false);
}
#endif

bool bt_media_cur_is_bt_stream_media(void)
{
    return (BT_STREAM_MEDIA & bt_meida.curr_active_media)?(true):(false);
}

bool bt_media_is_music_media_active(void)
{
    return (bt_media_is_media_active_by_type(BT_STREAM_MUSIC) == 1)?(true):(false);
}

void bt_media_current_music_set(uint8_t id)
{
    TRACE(0, "current music %d->%d", bt_meida.media_curr_music, id);
    bt_meida.media_curr_music = id;
}

uint8_t bt_media_current_music_get(void)
{
    return bt_meida.media_curr_music;
}

uint8_t bt_media_current_sco_get(void)
{
    return bt_meida.media_curr_sco;
}

static void bt_media_set_current_media(uint16_t media_type)
{
    uint32_t lock = int_lock();

    // out direction
#if defined(AUDIO_PROMPT_USE_DAC2_ENABLED)
    // BT_STREAM_MEDIA can run along with other stream
    if (media_type == BT_STREAM_MEDIA)
    {
        bt_meida.curr_active_media |= BT_STREAM_MEDIA;
    }
    else
#endif
    if (media_type < 0x100)
    {
#if defined(AUDIO_PROMPT_USE_DAC2_ENABLED)
        uint16_t media_stream_status = BT_STREAM_MEDIA & bt_meida.curr_active_media;
#endif
        bt_meida.curr_active_media &= (~0xFF);
        bt_meida.curr_active_media |= media_type;
#if defined(AUDIO_PROMPT_USE_DAC2_ENABLED)
        bt_meida.curr_active_media |= media_stream_status;
#endif
    }
    else
    {
        //bt_meida.curr_active_media &= (~0xFF00);
        bt_meida.curr_active_media |= media_type;
    }

    int_unlock(lock);

    TRACE(2,"curr_active_media is set to 0x%x%s", bt_meida.curr_active_media,
            bt_meida.curr_active_media ? strmtype2str(bt_meida.curr_active_media) : "[N/A]");
}

void bt_media_clear_current_media(uint16_t media_type)
{
    uint32_t lock = int_lock();
#if defined(AUDIO_PROMPT_USE_DAC2_ENABLED)
    // BT_STREAM_MEDIA can run along with other stream
    if (media_type == BT_STREAM_MEDIA)
    {
        bt_meida.curr_active_media &= (~BT_STREAM_MEDIA);
    }
    else
#endif
    if (media_type < 0x100)
    {
#if defined(AUDIO_PROMPT_USE_DAC2_ENABLED)
        uint16_t media_stream_status = BT_STREAM_MEDIA & bt_meida.curr_active_media;
#endif
        bt_meida.curr_active_media &= (~0xFF);
#if defined(AUDIO_PROMPT_USE_DAC2_ENABLED)
        bt_meida.curr_active_media |= media_stream_status;
#endif
    }
    else
    {
        bt_meida.curr_active_media &= (~media_type);
    }

    int_unlock(lock);

    TRACE(2, "clear media 0x%x curr media 0x%x", media_type, bt_meida.curr_active_media);
}

static uint8_t bt_media_set_media_type(uint16_t media_type, int device_id)
{
    uint32_t lock = int_lock();
    if (device_id < BT_DEVICE_NUM || (device_id >= BT_SOURCE_DEVICE_ID_BASE && device_id < BT_SOURCE_DEVICE_ID_BASE + BT_SOURCE_DEVICE_NUM))
    {
        TRACE(2, "Add active stream %d for device %d", media_type, device_id);
        bt_media_set_media_active_mask(device_id, media_type);
    }
    else
    {
        TRACE(2,"%s invalid devcie_id:%d",__func__,device_id );
    }
    int_unlock(lock);
    return 0;
}

void bt_media_clear_media_type(uint16_t media_type, int device_id)
{
    uint32_t lock = int_lock();

    uint8_t i;
    TRACE(4,"%s 0x%x active_now 0x%x id %d", __func__, media_type, bt_media_get_media_active(device_id), device_id);
    if (media_type == BT_STREAM_MEDIA)
    {
        for(i = 0; i < BT_DEVICE_NUM; i++)
        {
            bt_media_set_media_active_clear(i, media_type);
        }
        for (i = BT_SOURCE_DEVICE_ID_BASE; i < BT_SOURCE_DEVICE_ID_BASE + BT_SOURCE_DEVICE_NUM; i += 1)
        {
            bt_media_set_media_active_clear(i, media_type);
        }
    }
    else
    {
        bt_media_set_media_active_clear(device_id, media_type);
    }

    int_unlock(lock);
}

static int bt_media_get_active_music_device(void)
{
    uint8_t i;
    for(i = 0; i < BT_DEVICE_NUM; i++) // only a2dp sink has sbc speaker out media
    {
        if((bt_media_get_media_active(i) & BT_STREAM_MUSIC) && (i == bt_meida.media_curr_music))
        {
            return i;
        }
    }
    return BT_DEVICE_INVALID_ID;
}

void app_bt_media_set_current_media(uint16_t media_type)
{
    bt_media_set_current_media(media_type);
}

#ifdef RB_CODEC
bool  bt_media_rbcodec_start_process(uint16_t stream_type, int device_id,AUD_ID_ENUM media_id, uint32_t param, uint32_t ptr)
{
    TRACE(2, "Add active stream %d for device %d", stream_type, device_id);
    int ret_SendReq2AudioThread = -1;
    bt_media_set_media_active_mask(device_id, stream_type);

    ret_SendReq2AudioThread = app_audio_sendrequest(APP_BT_STREAM_RBCODEC, (uint8_t)APP_BT_SETTING_OPEN, media_id);
    bt_media_set_current_media(BT_STREAM_RBCODEC);
    return true;
exit:
    return false;
}
#endif

#ifdef __AI_VOICE__
static bool bt_media_ai_voice_start_process(uint16_t stream_type,
                                             int device_id,
                                             AUD_ID_ENUM media_id,
                                             uint32_t param,
                                             uint32_t ptr)
{
    TRACE(2, "Add active stream %d for device %d", stream_type, device_id);
    uint32_t lock = int_lock();
    bt_media_set_media_active_mask(device_id, stream_type);
    int_unlock(lock);
    app_audio_sendrequest(APP_BT_STREAM_AI_VOICE, ( uint8_t )APP_BT_SETTING_OPEN, media_id);

    bt_media_set_current_media(BT_STREAM_AI_VOICE);
    return true;
}
#endif

void app_stop_a2dp_media_stream(uint8_t devId)
{
    app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_STOP,
            BT_STREAM_MUSIC, devId,0);
}

void app_stop_sco_media_stream(uint8_t devId)
{
    app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_STOP,
            BT_STREAM_VOICE, devId,0);
}

extern "C" char *app_bt_get_global_state_buffer(void);

const char* app_bt_get_active_media_state(void)
{
    int len = 0;
    uint16_t media_active = 0;
#ifdef BT_BUILD_WITH_CUSTOMER_HOST
    char data[100*BT_DEVICE_NUM] = {0};
    char *buffer = data;
#else
    char *buffer = app_bt_get_global_state_buffer();
#endif

    len = sprintf(buffer, "curr_active_media %x%s media_active ", bt_meida.curr_active_media,
                bt_meida.curr_active_media ? strmtype2str(bt_meida.curr_active_media) : "[N/A]");

    for (int i = 0; i < BT_DEVICE_NUM; i += 1)
    {
        media_active = bt_adapter_get_device(i)->media_active;
        len += sprintf(buffer+len, "(d%x %x%s) ", i, media_active, media_active ? strmtype2str(media_active) : "[N/A]");
    }

    return buffer;
}

void app_bt_audio_state_checker(void)
{
    bt_bdaddr_t curr_sco_bdaddr = {0};
    bt_bdaddr_t curr_music_bdaddr = {0};

#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
#else // bes classic bt is disabled
    if (bt_meida.media_curr_sco != BT_DEVICE_INVALID_ID)
    {
        curr_sco_bdaddr = app_bt_get_device(bt_meida.media_curr_sco)->remote;
    }

    if (bt_meida.media_curr_music != BT_DEVICE_INVALID_ID)
    {
        curr_music_bdaddr = app_bt_get_device(bt_meida.media_curr_music)->remote;
    }
#endif

    TRACE(15,"audio_state: tri %d curr_sco %x %02x:%02x:%02x:%02x:%02x:%02x curr_music %x %02x:%02x:%02x:%02x:%02x:%02x",
            app_bt_stream_trigger_stauts_get(),
            bt_meida.media_curr_sco == BT_DEVICE_INVALID_ID ? 0xff : bt_meida.media_curr_sco,
            curr_sco_bdaddr.address[0], curr_sco_bdaddr.address[1], curr_sco_bdaddr.address[2],
            curr_sco_bdaddr.address[3], curr_sco_bdaddr.address[4], curr_sco_bdaddr.address[5],
            bt_meida.media_curr_music == BT_DEVICE_INVALID_ID ? 0xff : bt_meida.media_curr_music,
            curr_music_bdaddr.address[0], curr_music_bdaddr.address[1], curr_music_bdaddr.address[2],
            curr_music_bdaddr.address[3], curr_music_bdaddr.address[4], curr_music_bdaddr.address[5]);

    TRACE(1, "audio_state: %s", app_bt_get_active_media_state());
}

//only used in iamain thread ,can't used in other thread or interrupt
void  bt_media_start(uint16_t stream_type, int device_id, uint16_t media_id)
{
#if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))
#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
    bool isMergingPrompt = true;
#endif
#endif

    bt_media_set_media_type(stream_type, device_id);

    TRACE(4,"STREAM MANAGE bt_media_start stream 0x%x%s device 0x%x media_id 0x%x",
          stream_type,
          stream_type ? strmtype2str(stream_type) : "[N/A]",
          device_id,
          media_id);

    TRACE(1, "bt_media_start %s\n", app_bt_get_active_media_state());

    switch(stream_type)
    {
#ifdef RB_CODEC
        case BT_STREAM_RBCODEC:
            if(!bt_media_rbcodec_start_process(stream_type,device_id, (AUD_ID_ENUM)media_id, NULL, NULL))
                goto exit;
            break;
#endif

#ifdef __AI_VOICE__
        case BT_STREAM_AI_VOICE:
            if (bt_media_get_current_media() & BT_STREAM_AI_VOICE)
            {
                TRACE(0,"there is a ai voice stream exist ,do nothing");
                return;
            }

            if (bt_media_is_media_active_by_type(BT_STREAM_VOICE))
            {
                TRACE(0,"there is a SCO stream exist ,do nothing");
                goto exit;
            }

            if (!bt_media_ai_voice_start_process(BT_STREAM_AI_VOICE, device_id, ( AUD_ID_ENUM )media_id, ( uint32_t )NULL, ( uint32_t )NULL))
                goto exit;
            break;
#endif

        case BT_STREAM_MUSIC:
        {
			if(bt_media_is_media_active_by_type(BT_STREAM_VOICE))
            {
                TRACE(0,"there is a SCO stream exist ,do nothing");
                /// music and voice is all on so set sys freq to 104m
                app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_104M);
                return;
            }
            uint8_t media_pre_music = bt_meida.media_curr_music;

            /// because voice is the highest priority and media report will stop soon
            /// so just store the sbc type
            if (bt_meida.media_curr_music == BT_DEVICE_INVALID_ID)
                bt_meida.media_curr_music = device_id;

            TRACE(2," pre/cur_sbc = %d/%d", media_pre_music, bt_meida.media_curr_music);

#if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))
#ifndef AUDIO_PROMPT_USE_DAC2_ENABLED
#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
            /// clear the pending stop flag if it is set
            audio_prompt_clear_pending_stream(PENDING_TO_STOP_A2DP_STREAMING);
#endif

            if (bt_media_is_media_active_by_type(BT_STREAM_MEDIA))
            {
#ifdef __AI_VOICE__
                if (app_ai_voice_is_need2block_a2dp())
                {
                    TRACE(0,"there is a ai voice stream exist");
                    app_ai_voice_block_a2dp();
                }
#endif
                goto exit;
            }
#endif // #ifndef AUDIO_PROMPT_USE_DAC2_ENABLED
#endif // #if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))

#ifdef RB_CODEC
            if (bt_media_is_media_active_by_type(BT_STREAM_RBCODEC))
            {
                goto exit;
            }
#endif

#ifdef __AI_VOICE__
            if (app_ai_voice_is_need2block_a2dp())
            {
                TRACE(0,"there is a ai voice stream exist");
                app_ai_voice_block_a2dp();
                goto exit;
            }
#endif

#ifdef AUDIO_LINEIN
            if(bt_media_is_media_active_by_type(BT_STREAM_LINEIN))
            {
                if(bt_media_get_current_media() & BT_STREAM_LINEIN)
                {
                    app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_STOP, (uint8_t)BT_STREAM_LINEIN, 0,0);
                    app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_START, BT_STREAM_MUSIC, device_id, 0);
                    return;
                }
            }
#endif

#ifdef AUDIO_PCM_PLAYER
            if(bt_media_is_media_active_by_type(BT_STREAM_PCM_PLAYER))
            {
                if(bt_media_get_current_media() & BT_STREAM_PCM_PLAYER)
                {
                    app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_STOP, (uint8_t)BT_STREAM_PCM_PLAYER, 0,0);
                    app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_START, BT_STREAM_MUSIC, device_id, 0);
                    return;
                }
            }
#endif

            if (!bt_media_cur_is_bt_stream_music())
            {
                app_audio_manager_switch_a2dp(device_id);

                bt_media_set_current_media(BT_STREAM_MUSIC);

                app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC,
                                      (uint8_t)(APP_BT_SETTING_SETUP),
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
                                      app_bt_get_curr_a2dp_sample_rate(device_id));
#else
                                      (uint32_t)(app_bt_get_device(device_id)->sample_rate & A2D_STREAM_SAMP_FREQ_MSK));
#endif
                if (app_bt_audio_count_streaming_a2dp() >= 2)
                {
                    app_audio_sendrequest_param(APP_BT_STREAM_A2DP_SBC, ( uint8_t )APP_BT_SETTING_OPEN, 0, APP_SYSFREQ_104M);
                }
                else
                {
                    app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC, ( uint8_t )APP_BT_SETTING_OPEN, 0);
                }
            }
        }
        break;

#if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))
        case BT_STREAM_MEDIA:
#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
            isMergingPrompt = IS_PROMPT_NEED_MERGING(media_id);
#endif

            if (
#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
                audio_prompt_is_playing_ongoing() ||
#endif
                app_audio_list_playback_exist())
            {
                if (!bt_media_cur_is_bt_stream_media())
                {
                    bt_media_clear_media_type(BT_STREAM_MEDIA, device_id);
                }

                APP_AUDIO_STATUS aud_status = {0};
                aud_status.id = APP_PLAY_BACK_AUDIO;
                aud_status.aud_id = media_id;
                aud_status.device_id = device_id;
                app_audio_list_append(&aud_status);
                break;
            }

#ifndef AUDIO_PROMPT_USE_DAC2_ENABLED
#ifdef AUDIO_LINEIN
            if(bt_media_is_media_active_by_type(BT_STREAM_LINEIN))
            {
#if 1
                {
                    TRACE(0,"AUX START prompt.");
                    app_audio_sendrequest(APP_PLAY_LINEIN_AUDIO, (uint8_t)APP_BT_SETTING_CLOSE, 0);
                    bt_media_set_current_media(BT_STREAM_MEDIA);
                    app_audio_sendrequest(APP_PLAY_BACK_AUDIO, (uint8_t)APP_BT_SETTING_OPEN, media_id);
                }
#else
                if(bt_media_get_current_media() & BT_STREAM_LINEIN)
                {
                    APP_AUDIO_STATUS aud_status = {0};
                    aud_status.id = media_id;
                    app_play_audio_lineinmode_start(&aud_status);
                    bt_media_clear_media_type(BT_STREAM_MEDIA, device_id);
                }
#endif
            }else
#endif
            //first,if the voice is active so  mix "dudu" to the stream
            if(bt_media_is_media_active_by_type(BT_STREAM_VOICE))
            {
                if(bt_media_get_current_media() & BT_STREAM_VOICE)
                {
#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
                    // if the playback is not triggered yet, we just use the stand-alone prompt playing
                    if (!bt_is_playback_triggered())
                    {
                        isMergingPrompt = false;
                    }
#endif
                    //if call is not active so do media report
                    if((btapp_hfp_is_call_active() && !btapp_hfp_incoming_calls()) ||
                       (app_bt_stream_isrun(APP_BT_STREAM_HFP_PCM)))
                    {
#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
                        bt_media_clear_media_type(BT_STREAM_MEDIA, device_id);
                        if (isMergingPrompt)
                        {
                            audio_prompt_start_playing(media_id, sco_sample_rate);
                            goto exit;
                        }
                        else
                        {
                            app_audio_sendrequest(APP_BT_STREAM_HFP_PCM, (uint8_t)APP_BT_SETTING_CLOSE, 0);
                            app_audio_sendrequest(APP_PLAY_BACK_AUDIO, (uint8_t)APP_BT_SETTING_OPEN, media_id);
                            bt_media_set_current_media(BT_STREAM_MEDIA);
                        }
#else
                        //in three way call merge "dudu"
                        TRACE(0,"BT_STREAM_VOICE-->app_ring_merge_start\n");
                        app_ring_merge_start();
                        //meida is done here
                        bt_media_clear_media_type(BT_STREAM_MEDIA, device_id);
#endif
                    }
                    else
                    {
#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
                        bt_media_clear_media_type(BT_STREAM_MEDIA, device_id);
                        if (isMergingPrompt)
                        {
                            audio_prompt_start_playing(media_id, sco_sample_rate);
                            goto exit;
                        }
                        else
                        {
                            app_audio_sendrequest(APP_BT_STREAM_HFP_PCM, (uint8_t)APP_BT_SETTING_CLOSE, 0);
                            app_audio_sendrequest(APP_PLAY_BACK_AUDIO, (uint8_t)APP_BT_SETTING_OPEN, media_id);
                            bt_media_set_current_media(BT_STREAM_MEDIA);
                        }
#else
                        TRACE(0,"stop sco and do media report\n");
                        bt_media_set_current_media(BT_STREAM_MEDIA);
                        app_audio_sendrequest(APP_BT_STREAM_HFP_PCM, (uint8_t)APP_BT_SETTING_CLOSE, 0);
                        app_audio_sendrequest(APP_PLAY_BACK_AUDIO, (uint8_t)APP_BT_SETTING_OPEN, media_id);
#endif
                    }
                }
                else if(bt_media_get_current_media() & BT_STREAM_MEDIA)
                {
                    bt_media_set_current_media(BT_STREAM_MEDIA);
                    app_audio_sendrequest(APP_PLAY_BACK_AUDIO, (uint8_t)APP_BT_SETTING_OPEN, media_id);
                }
                else
                {
                    ///if voice is active but current is not voice something is unkown
                    bt_media_clear_media_type(BT_STREAM_MEDIA, device_id);

                    TRACE(1,"STREAM MANAGE %s", app_bt_get_active_media_state());
                }
            }
            else if (btapp_hfp_is_call_active())
            {
                bt_media_set_current_media(BT_STREAM_MEDIA);
                app_audio_sendrequest(APP_PLAY_BACK_AUDIO, (uint8_t)APP_BT_SETTING_OPEN, media_id);
            }
            /// if music active so
            else if(bt_media_is_media_active_by_type(BT_STREAM_MUSIC))
            {
                if(bt_media_get_current_media() & BT_STREAM_MUSIC)
                {
#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
                    // if the playback is not triggered yet, we just use the stand-alone prompt playing
                    // fix the case that one earbud rejoin a2dp streaming, prompt tone interrupt the A2DP
                    // player before the a2p playback triggered, so it will result to force retrigger when receiving the
                    // MOBILE_PLAYBACK_INFO.
                    if (!bt_a2dp_is_run())
                    {
                        isMergingPrompt = false;
                    }

                    bt_media_clear_media_type(BT_STREAM_MEDIA, device_id);
                    if (isMergingPrompt)
                    {
                        audio_prompt_start_playing(media_id,a2dp_sample_rate);
                        goto exit;
                    }
                    else
                    {
                        TRACE(0,"START prompt.");
                        app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC, (uint8_t)APP_BT_SETTING_CLOSE, 0);
                        bt_media_clear_current_media(BT_STREAM_MUSIC);
                        bt_media_set_current_media(BT_STREAM_MEDIA);
                        app_audio_sendrequest(APP_PLAY_BACK_AUDIO, (uint8_t)APP_BT_SETTING_OPEN, media_id);
                    }
#else
#ifdef __BT_WARNING_TONE_MERGE_INTO_STREAM_SBC__
                    if (PROMPT_ID_FROM_ID_VALUE(media_id) == AUD_ID_BT_WARNING)
                    {
                        TRACE(0,"BT_STREAM_MUSIC-->app_ring_merge_start\n");
                        app_ring_merge_start();
                        //meida is done here
                        bt_media_clear_media_type(BT_STREAM_MEDIA, device_id);
                    }
                    else
#endif
                    {
                        app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC, (uint8_t)APP_BT_SETTING_CLOSE, 0);
                        bt_media_clear_current_media(BT_STREAM_MUSIC);
                        bt_media_set_current_media(BT_STREAM_MEDIA);
                        app_audio_sendrequest(APP_PLAY_BACK_AUDIO, (uint8_t)APP_BT_SETTING_OPEN, media_id);
                    }
#endif // #ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
                }
                else if(bt_media_get_current_media() & BT_STREAM_MEDIA)
                {
                    app_audio_sendrequest(APP_PLAY_BACK_AUDIO, (uint8_t)APP_BT_SETTING_OPEN, media_id);
                }
                else if ((bt_media_get_current_media()&0xFF) == 0)
                {
                    app_audio_sendrequest(APP_PLAY_BACK_AUDIO, (uint8_t)APP_BT_SETTING_OPEN, media_id);
                }
                else
                {
                    ASSERT(0,"media in music  current wrong");
                }
            }
            /// just play the media
            else
#endif // #ifndef AUDIO_PROMPT_USE_DAC2_ENABLED
            {
                bt_media_set_current_media(BT_STREAM_MEDIA);
                app_audio_sendrequest(APP_PLAY_BACK_AUDIO, (uint8_t)APP_BT_SETTING_OPEN, media_id);
            }
            break;
#endif
        case BT_STREAM_VOICE:
        {
            uint8_t curr_playing_sco = app_audio_manager_get_active_sco_num();
            uint8_t curr_request_sco = device_id;

            if (curr_playing_sco == BT_DEVICE_INVALID_ID)
                app_audio_manager_set_active_sco_num(device_id);

#ifdef __AI_VOICE__
            if(bt_media_is_media_active_by_type(BT_STREAM_AI_VOICE)) {
                if(bt_media_get_current_media() & BT_STREAM_AI_VOICE) {
                    app_audio_sendrequest(APP_BT_STREAM_AI_VOICE, (uint8_t)APP_BT_SETTING_CLOSE, 0);
                    bt_media_clear_current_media(BT_STREAM_AI_VOICE);
                }
            }
#endif

#if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))
#ifndef AUDIO_PROMPT_USE_DAC2_ENABLED
#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
            /// clear the pending stop flag if it is set
            audio_prompt_clear_pending_stream(PENDING_TO_STOP_SCO_STREAMING);
#endif

            if(bt_media_is_media_active_by_type(BT_STREAM_MEDIA))
            {
                //if call is active ,so disable media report
                if(bt_media_is_media_active_by_type(BT_STREAM_VOICE))
                {
                    if(bt_media_get_current_media() & BT_STREAM_MEDIA)
                    {
                        if (app_play_audio_get_aud_id() == AUD_ID_BT_CALL_INCOMING_NUMBER)
                        {
                            //if meida is open ,close media clear all media type
                            TRACE(0,"call active so start sco and stop media report\n");
#ifdef __AUDIO_QUEUE_SUPPORT__
                            app_audio_list_clear();
#endif
                            app_prompt_stop_all();

                            app_audio_sendrequest(APP_PLAY_BACK_AUDIO, (uint8_t)APP_BT_SETTING_CLOSE, 0);
                            bt_media_clear_media_type(BT_STREAM_MEDIA, device_id);
                            bt_media_set_current_media(BT_STREAM_VOICE);
                            app_audio_sendrequest(APP_BT_STREAM_HFP_PCM, (uint8_t)APP_BT_SETTING_OPEN, 0);
                        }
                    }
                }
                else
                {
                    ////call is not active so media report continue
                }
                return;
            }
#endif // #ifndef AUDIO_PROMPT_USE_DAC2_ENABLED
#endif // #if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))

            if (curr_playing_sco == BT_DEVICE_INVALID_ID)
            {
                app_audio_manager_set_active_sco_num(curr_request_sco);
                if (bt_media_is_media_active_by_type(BT_STREAM_MUSIC))
                {
                    ///if music is open stop music
                    if(bt_media_get_current_media() & BT_STREAM_MUSIC)
                    {
                        app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC, (uint8_t)APP_BT_SETTING_CLOSE, 0);
                        bt_media_clear_current_media(BT_STREAM_MUSIC);
                    }

                    ////start voice stream
                    bt_media_set_current_media(BT_STREAM_VOICE);
                    app_audio_sendrequest_param(APP_BT_STREAM_HFP_PCM, (uint8_t)APP_BT_SETTING_OPEN, 0, APP_SYSFREQ_104M);
                }
                else
                {
                    bt_media_set_current_media(BT_STREAM_VOICE);
                    app_audio_sendrequest(APP_BT_STREAM_HFP_PCM, (uint8_t)APP_BT_SETTING_OPEN, 0);
                }
            }
            else if (curr_playing_sco != curr_request_sco)
            {
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
                app_audio_manager_set_active_sco_num(curr_playing_sco);
                goto start_bt_stream_voice_label; // non-prompt mode - play current playing one
#else
                if (app_bt_manager.config.call_preempt_play_mode) // prompt mode - play requested one
                {
                    app_audio_manager_swap_sco(curr_request_sco);
                    app_audio_manager_set_active_sco_num(curr_request_sco);
                }
                else // non-prompt mode - play current playing one
                {
                    app_audio_manager_set_active_sco_num(curr_playing_sco);
                    goto start_bt_stream_voice_label;
                }
#endif
            }
            else
            {
                app_audio_manager_set_active_sco_num(curr_request_sco);
start_bt_stream_voice_label:
                if ((bt_media_get_current_media() & BT_STREAM_VOICE) == 0)
                {
                    if (bt_media_is_media_active_by_type(BT_STREAM_MUSIC))
                    {
                        ///if music is open stop music
                        if(bt_media_get_current_media() & BT_STREAM_MUSIC)
                        {
                            app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC, (uint8_t)APP_BT_SETTING_CLOSE, 0);
                            bt_media_clear_current_media(BT_STREAM_MUSIC);
                        }

                        ////start voice stream
                        bt_media_set_current_media(BT_STREAM_VOICE);
                        app_audio_sendrequest_param(APP_BT_STREAM_HFP_PCM, (uint8_t)APP_BT_SETTING_OPEN, 0, APP_SYSFREQ_104M);
                    }
                    else
                    {
                        bt_media_set_current_media(BT_STREAM_VOICE);
                        app_audio_sendrequest(APP_BT_STREAM_HFP_PCM, (uint8_t)APP_BT_SETTING_OPEN, 0);
                    }
                }
            }
        }
        break;
#ifdef AUDIO_LINEIN
        case BT_STREAM_LINEIN:
            if(!bt_media_is_media_active_by_type(BT_STREAM_MUSIC | BT_STREAM_MEDIA | BT_STREAM_VOICE))
            {
                app_audio_sendrequest(APP_PLAY_LINEIN_AUDIO, (uint8_t)APP_BT_SETTING_OPEN, 0);
                bt_media_set_current_media(BT_STREAM_LINEIN);
            }
            break;
#endif
#ifdef AUDIO_PCM_PLAYER
        case BT_STREAM_PCM_PLAYER:
            if (!bt_media_is_media_active_by_type(BT_STREAM_MUSIC | BT_STREAM_MEDIA | BT_STREAM_VOICE)) {
                app_audio_sendrequest(APP_PLAY_PCM_PLAYER, (uint8_t)APP_BT_SETTING_OPEN, 0);
                bt_media_set_current_media(BT_STREAM_PCM_PLAYER);
            }
            break;
#endif

        default:
            ASSERT(0,"bt_media_open ERROR TYPE");
            break;

    }

#if defined(RB_CODEC) || defined(VOICE_DATAPATH) || \
    ((defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))) && \
    !defined(AUDIO_PROMPT_USE_DAC2_ENABLED)) || defined(__AI_VOICE__)
exit:
    return;
#endif
}

#ifdef RB_CODEC

static bool bt_media_rbcodec_stop_process(uint16_t stream_type, int device_id, uint32_t ptr)
{
    int ret_SendReq2AudioThread = -1;
    bt_media_clear_media_type(stream_type,device_id);
    //if current stream is the stop one ,so stop it
    if(bt_media_get_current_media() & BT_STREAM_RBCODEC ) {
        ret_SendReq2AudioThread = app_audio_sendrequest(APP_BT_STREAM_RBCODEC, (uint8_t)APP_BT_SETTING_CLOSE, ptr);
        bt_media_clear_current_media(BT_STREAM_RBCODEC);
        TRACE(0," RBCODEC STOPED! ");
    }

    if(bt_media_is_media_active_by_type(BT_STREAM_MUSIC)) {
        int music_id  = bt_media_get_active_device_by_type(BT_STREAM_MUSIC);
        TRACE(1,"music_id %d",music_id);
        if(music_id < BT_DEVICE_NUM) {
            bt_meida.media_curr_music = music_id;
        }
    } else {
        bt_meida.media_curr_music = BT_DEVICE_INVALID_ID;
    }

    TRACE(1,"bt_meida.media_curr_music %d",bt_meida.media_curr_music);

    if(bt_media_is_media_active_by_type(BT_STREAM_VOICE)) {
    } else if(bt_media_is_media_active_by_type(BT_STREAM_MUSIC)) {
        int music_id  = bt_media_get_active_device_by_type(BT_STREAM_MUSIC);
        if(music_id < BT_DEVICE_NUM) {
#ifdef __TWS__
            bt_media_clear_media_type(BT_STREAM_MUSIC,music_id);
            bt_media_clear_current_media(BT_STREAM_MUSIC);
            notify_tws_player_status(APP_BT_SETTING_OPEN);
#else
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
            bt_parse_store_music_sample_rate(app_bt_get_curr_a2dp_sample_rate(music_id));
#else
            bt_parse_store_music_sample_rate(bta_get_a2dp_sample_rate_by_id(music_id));
#endif
            ret_SendReq2AudioThread = app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC, (uint8_t)APP_BT_SETTING_OPEN, 0);
            bt_media_set_current_media(BT_STREAM_MUSIC);
#endif
        }
    } else if(bt_media_is_media_active_by_type(BT_STREAM_MEDIA)) {
        //do nothing
    }
}
#endif

#ifdef __AI_VOICE__
bool bt_media_ai_voice_stop_process(uint16_t stream_type, int device_id)
{
    bt_media_clear_media_type(BT_STREAM_AI_VOICE, device_id);
    //if current stream is the stop one ,stop it
    if(bt_media_get_current_media() & BT_STREAM_AI_VOICE)
    {
        app_audio_sendrequest(APP_BT_STREAM_AI_VOICE, (uint8_t)APP_BT_SETTING_CLOSE, 0);
        bt_media_clear_current_media(BT_STREAM_AI_VOICE);
        TRACE(0," AI VOICE STOPED! ");

    }

#ifndef IBRT
    int music_id = BT_DEVICE_INVALID_ID;
    music_id = bt_media_get_active_device_by_type(BT_STREAM_MUSIC);
    TRACE(1,"music_id %d",music_id);
    bt_meida.media_curr_music = music_id;

    TRACE(1,"bt_meida.media_curr_music %d",bt_meida.media_curr_music);

    if(bt_media_is_media_active_by_type(BT_STREAM_VOICE))
    {
    }
    else if(music_id < BT_DEVICE_NUM)
    {
        if(!(bt_media_get_current_media() & BT_STREAM_MUSIC))
        {
            //bt_parse_store_music_sample_rate(app_bt_get_device(music_id)->sample_rate);
            app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC,
                                  (uint8_t)(APP_BT_SETTING_SETUP),
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
                                  app_bt_get_curr_a2dp_sample_rate(music_id));
#else
                                  (uint32_t)(app_bt_get_device(music_id)->sample_rate & A2D_STREAM_SAMP_FREQ_MSK));
#endif
            app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC, (uint8_t)APP_BT_SETTING_OPEN, 0);
            bt_media_set_current_media(BT_STREAM_MUSIC);
        }
    }
#endif
    return true;
}
#endif

/*
   bt_media_stop function is called to stop media by app or media play callback
   music is just stop by a2dp stream suspend or close
   voice is just stop by hfp audio disconnect
   media is stop by media player finished call back

*/
void bt_media_stop(uint16_t stream_type, int device_id,uint16_t media_id)
{
    TRACE(4,"bt_media_stop stream 0x%x%s device 0x%x media_id 0x%x", stream_type, stream_type ? strmtype2str(stream_type) : "[N/A]", device_id, media_id);

    TRACE(1, "bt_media_stop %s\n", app_bt_get_active_media_state());

    if (!bt_media_is_media_active_by_device(stream_type, device_id) &&
        !(bt_media_get_current_media() & stream_type) &&
        stream_type != BT_STREAM_MEDIA)
    {
        TRACE(0, "bt_media_stop skip");
        return;
    }

    switch(stream_type)
    {
    #ifdef __AI_VOICE__
        case BT_STREAM_AI_VOICE:
            bt_media_ai_voice_stop_process(stream_type, device_id);
            break;
    #endif

        case BT_STREAM_MUSIC:
            {
                APP_AUDIO_STATUS info = {0};
            #ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
                if (!audio_prompt_check_on_stopping_stream(PENDING_TO_STOP_A2DP_STREAMING, device_id))
                {
                    TRACE(0,"Pending stop BT_STREAM_MUSIC");
                    return;
                }
            #elif defined(AUDIO_PROMPT_USE_DAC2_ENABLED)
            #else
                if (app_ring_merge_isrun())
                {
                    TRACE(0,"bt_media_stop pending BT_STREAM_MUSIC");
                    app_ring_merge_save_pending_start_stream_op(PENDING_TO_STOP_A2DP_STREAMING, device_id);
                    return;
                }
            #endif

                uint8_t media_pre_music = bt_meida.media_curr_music;
                TRACE(2,"MUSIC STOPPING id:%d/%d", bt_meida.media_curr_music, device_id);

                ////if current media is music ,stop the music streaming
                bt_media_clear_media_type(stream_type, device_id);

                info.id     = APP_BT_STREAM_A2DP_SBC;
                info.aud_id = 0;

                app_audio_list_rmv_by_info(&info);

                //if current stream is the stop one ,so stop it
                if ((bt_media_get_current_media() & BT_STREAM_MUSIC)
#if !defined(IBRT)
                    && bt_meida.media_curr_music  == device_id
#endif
                )
                {
                    app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC, (uint8_t)APP_BT_SETTING_CLOSE, 0);
                    bt_media_clear_current_media(BT_STREAM_MUSIC);
                    TRACE(0,"MUSIC STOPED!");
#ifdef __AI_VOICE__
                    app_ai_voice_resume_blocked_a2dp();
#endif
                }

                if(bt_media_is_media_active_by_type(BT_STREAM_MUSIC))
                {
                    int music_id  = bt_media_get_active_device_by_type(BT_STREAM_MUSIC);
                    if (music_id < BT_DEVICE_NUM)
                    {
                        TRACE(1,"want to enable music id= %d", music_id);
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
                        if (app_bt_is_curr_a2dp_streaming(music_id))
#else
                        struct BT_DEVICE_T *curr_device = app_bt_get_device(music_id);
                        if (curr_device->a2dp_streamming)
#endif
                        {
                            bt_meida.media_curr_music = music_id;
                        }
                        else
                        {
                            bt_meida.media_curr_music = BT_DEVICE_INVALID_ID;
                            bt_media_clear_media_type(BT_STREAM_MUSIC,music_id);
                        }
                    }
                }
                else
                {
                    bt_meida.media_curr_music = BT_DEVICE_INVALID_ID;
                }

                if(bt_media_is_media_active_by_type(BT_STREAM_VOICE))
                {

                }
                else if(bt_media_is_media_active_by_type(BT_STREAM_MEDIA))
                {
                    //do nothing
                }
                else if(bt_media_is_media_active_by_type(BT_STREAM_MUSIC))
                {
                    int music_id  = bt_media_get_active_device_by_type(BT_STREAM_MUSIC);
                    if (music_id < BT_DEVICE_NUM && (media_pre_music != bt_meida.media_curr_music))
                    {
                        app_audio_manager_switch_a2dp(music_id);
                        bt_media_set_current_media(BT_STREAM_MUSIC);
                        app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC,
                                (uint8_t)(APP_BT_SETTING_SETUP),
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
                                app_bt_get_curr_a2dp_sample_rate(music_id));
#else
                                (uint32_t)(app_bt_get_device(music_id)->sample_rate & A2D_STREAM_SAMP_FREQ_MSK));
#endif
                        app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC, (uint8_t)APP_BT_SETTING_OPEN, 0);
                    }
                }
#ifdef AUDIO_LINEIN
                else if (bt_media_is_media_active_by_type(BT_STREAM_LINEIN))
                {
                    app_audio_sendrequest(APP_PLAY_LINEIN_AUDIO, (uint8_t)APP_BT_SETTING_OPEN, 0);
                    bt_media_set_current_media(BT_STREAM_LINEIN);
                }
#endif
            }
            break;
#if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))
        case BT_STREAM_MEDIA:
            bt_media_clear_media_type(BT_STREAM_MEDIA ,device_id);

            if(bt_media_is_media_active_by_type(BT_STREAM_MEDIA))
            {
                //also have media report so do nothing
            }
            else if(bt_media_is_media_active_by_type(BT_STREAM_VOICE))
            {
                if(bt_media_get_current_media() & BT_STREAM_VOICE)
                {
                    //do nothing
                }
                else if(bt_media_get_current_media() & BT_STREAM_MEDIA)
                {
                    ///media report is end ,so goto voice
                    uint8_t curr_sco_id;
                    curr_sco_id = app_audio_manager_get_active_sco_num();
                    if (curr_sco_id != BT_DEVICE_INVALID_ID) {
                        bt_media_set_media_type(BT_STREAM_VOICE, curr_sco_id);
                        bt_media_set_current_media(BT_STREAM_VOICE);
#if BT_DEVICE_NUM > 1
                        app_audio_manager_swap_sco(curr_sco_id);
#endif
                        app_audio_sendrequest(APP_BT_STREAM_HFP_PCM, (uint8_t)APP_BT_SETTING_OPEN, 0);
                    }
                }
                else
                {
                    //media report is end ,so goto voice
                    TRACE(2, "curr_active_media is %x%s, goto voice",
                             bt_meida.curr_active_media,
                             bt_meida.curr_active_media ? strmtype2str(bt_meida.curr_active_media) : "[N/A]");
                    uint8_t curr_sco_id;
                    curr_sco_id = app_audio_manager_get_active_sco_num();
                    if (curr_sco_id != BT_DEVICE_INVALID_ID) {
                        TRACE(0, "goto voice open");
                        app_audio_sendrequest(APP_BT_STREAM_HFP_PCM, (uint8_t)APP_BT_SETTING_OPEN, 0);
                        bt_media_set_current_media(BT_STREAM_VOICE);
                    }
                }
            }
            else if (btapp_hfp_is_call_active())
            {
                //do nothing
            }
        #ifdef __AI_VOICE__
            else if(bt_media_is_media_active_by_type(BT_STREAM_AI_VOICE) || ai_if_is_ai_stream_mic_open())
            {
                bt_media_clear_current_media(BT_STREAM_MEDIA);
                if (bt_media_is_media_active_by_type(BT_STREAM_AI_VOICE) && !(bt_media_get_current_media() & BT_STREAM_AI_VOICE))
                {
                    app_audio_sendrequest(APP_BT_STREAM_AI_VOICE, (uint8_t)APP_BT_SETTING_OPEN, 0);
                    bt_media_set_current_media(BT_STREAM_AI_VOICE);
                }
                else if(app_ai_voice_is_need2block_a2dp()){
                    app_ai_voice_block_a2dp();
                }
                else if(bt_media_is_media_active_by_type(BT_STREAM_MUSIC)){
                    // Do nothing when prompt use DAC2
#ifdef AUDIO_PROMPT_USE_DAC2_ENABLED
                    if (media_id == AUDIO_ID_BT_MUTE)
#endif
                    {
                        int music_id  = bt_media_get_active_music_device();
                        bt_media_set_media_type(BT_STREAM_MUSIC, music_id);
                        app_audio_manager_switch_a2dp(music_id);
                        bt_media_set_current_media(BT_STREAM_MUSIC);

                        app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC,
                                            (uint8_t)(APP_BT_SETTING_SETUP),
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
                                            app_bt_get_curr_a2dp_sample_rate(music_id));
#else
                                            (uint32_t)(app_bt_get_device(music_id)->sample_rate & A2D_STREAM_SAMP_FREQ_MSK));
#endif
                        app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC, (uint8_t)APP_BT_SETTING_OPEN, 0);
                    }
                }
#ifdef AUDIO_LINEIN
                if(bt_media_is_media_active_by_type(BT_STREAM_LINEIN) && !app_is_power_off_in_progress())
                {
                    app_audio_sendrequest(APP_PLAY_LINEIN_AUDIO, (uint8_t)APP_BT_SETTING_OPEN, 0);
                    bt_media_set_current_media(BT_STREAM_LINEIN);
                }
#endif
            }
        #endif
            else if(bt_media_is_media_active_by_type(BT_STREAM_MUSIC))
            {
                // Do nothing when prompt use DAC2
#ifdef AUDIO_PROMPT_USE_DAC2_ENABLED
                if (media_id == AUDIO_ID_BT_MUTE)
#endif
                {
                    TRACE(0, "A2DP stream active, start it");
                    ///if another device is also in music mode
                    int music_id  = bt_media_get_active_music_device();
                    bt_media_set_media_type(BT_STREAM_MUSIC, music_id);
                    app_audio_manager_switch_a2dp(music_id);
                    bt_media_set_current_media(BT_STREAM_MUSIC);

                    app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC,
                                        (uint8_t)(APP_BT_SETTING_SETUP),
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
                                        app_bt_get_curr_a2dp_sample_rate(music_id));
#else
                                        (uint32_t)(app_bt_get_device(music_id)->sample_rate & A2D_STREAM_SAMP_FREQ_MSK));
#endif
                    app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC, (uint8_t)APP_BT_SETTING_OPEN, 0);
                }
            }
#ifdef AUDIO_LINEIN
            else if(bt_media_is_media_active_by_type(BT_STREAM_LINEIN) && !app_is_power_off_in_progress())
            {
                app_audio_sendrequest(APP_PLAY_LINEIN_AUDIO, (uint8_t)APP_BT_SETTING_OPEN, 0);
                bt_media_set_current_media(BT_STREAM_LINEIN);
            }
#endif
            else
            {
                //have no meida task,so goto idle
                bt_media_set_current_media(0);
            }
            break;
#endif
        case BT_STREAM_VOICE:
            if(!bt_media_is_media_active_by_device(BT_STREAM_VOICE,device_id)||!(bt_media_get_current_media() & BT_STREAM_VOICE))
            {
                TRACE(0,"bt_media_stop already stop");
                bt_media_clear_media_type(stream_type, device_id);
                return ;
            }

        #ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
            if (!audio_prompt_check_on_stopping_stream(PENDING_TO_STOP_SCO_STREAMING, device_id))
            {
                TRACE(0,"Pending stop BT_STREAM_SCO");
                return;
            }
        #elif defined(AUDIO_PROMPT_USE_DAC2_ENABLED)
        #else
            if (app_ring_merge_isrun())
            {
                TRACE(0,"bt_media_stop pending BT_STREAM_VOICE");
                app_ring_merge_save_pending_start_stream_op(PENDING_TO_STOP_SCO_STREAMING, device_id);
                return;
            }
        #endif

            if (device_id == app_audio_manager_get_active_sco_num())
            {
                app_audio_manager_set_active_sco_num(BT_DEVICE_INVALID_ID);
            }

            bt_media_clear_media_type(stream_type, device_id);

#if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))
#ifndef AUDIO_PROMPT_USE_DAC2_ENABLED
            if(bt_media_is_media_active_by_type(BT_STREAM_MEDIA))
            {
                if(bt_media_get_current_media() & BT_STREAM_MEDIA)
                {
                    //do nothing
                }
                else if (bt_media_get_current_media() & BT_STREAM_VOICE)
                {
                    TRACE(0,"!!!!! WARNING VOICE if voice and media is all on,media should be the current media");
                    if(!bt_media_is_media_active_by_type(BT_STREAM_VOICE)){
                        app_audio_sendrequest(APP_BT_STREAM_HFP_PCM, (uint8_t)APP_BT_SETTING_CLOSE, 0);
                    }
                }
                else if (bt_media_get_current_media() & BT_STREAM_MUSIC)
                {
                    TRACE(0,"!!!!! WARNING MUSIC if voice and media is all on,media should be the current media");
                    if(!bt_media_is_media_active_by_type(BT_STREAM_MUSIC)){
                        app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC, (uint8_t)APP_BT_SETTING_CLOSE, 0);
                    }
                }
            }
            else
#endif
#endif
            if (bt_media_is_media_active_by_type(BT_STREAM_VOICE))
            {
                TRACE(0,"!!!!!!!!!bt_media_stop voice, wait another voice start");
            }
            else if (btapp_hfp_is_call_active())
            {
                TRACE(0,"stop in HF_CALL_ACTIVE and no sco need");
                bt_media_set_current_media(0);
                app_audio_sendrequest(APP_BT_STREAM_HFP_PCM, (uint8_t)APP_BT_SETTING_CLOSE, 0);
                bt_media_clear_media_type(BT_STREAM_VOICE, device_id);
            }
            else
            {
                bt_media_set_current_media(0);
                app_audio_sendrequest(APP_BT_STREAM_HFP_PCM, (uint8_t)APP_BT_SETTING_CLOSE, 0);
            }

#ifdef __AI_VOICE__
            if(bt_media_is_media_active_by_type(BT_STREAM_AI_VOICE)) {
                app_audio_sendrequest(APP_BT_STREAM_AI_VOICE, (uint8_t)APP_BT_SETTING_OPEN, 0);
                bt_media_set_current_media(BT_STREAM_AI_VOICE);
            }
            else
#endif
            if (bt_media_is_media_active_by_type(BT_STREAM_MUSIC) && !bt_media_cur_is_bt_stream_music())
            {
                int music_id  = bt_media_get_active_music_device();
                TRACE(2, "d%x voice is stopped, d%x still have music wait to start", device_id, music_id);
                if (music_id != BT_DEVICE_INVALID_ID)
                {
                    bt_media_set_media_type(BT_STREAM_MUSIC, music_id);
                    app_audio_manager_switch_a2dp(music_id);
                    bt_media_set_current_media(BT_STREAM_MUSIC);

                    app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC,
                                        (uint8_t)(APP_BT_SETTING_SETUP),
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
                                        app_bt_get_curr_a2dp_sample_rate(music_id));
#else
                                        (uint32_t)(app_bt_get_device(music_id)->sample_rate & A2D_STREAM_SAMP_FREQ_MSK));
#endif
                    app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC, (uint8_t)APP_BT_SETTING_OPEN, 0);
                }
            }
#ifdef AUDIO_LINEIN
            else if (bt_media_is_media_active_by_type(BT_STREAM_LINEIN))
            {
                app_audio_sendrequest(APP_PLAY_LINEIN_AUDIO, (uint8_t)APP_BT_SETTING_OPEN, 0);
                bt_media_set_current_media(BT_STREAM_LINEIN);
            }
#endif
            break;

#ifdef RB_CODEC
        case BT_STREAM_RBCODEC:
            bt_media_rbcodec_stop_process(stream_type, device_id, 0);
            break;
#endif

#ifdef AUDIO_LINEIN
        case BT_STREAM_LINEIN:
            if(bt_media_is_media_active_by_type(BT_STREAM_LINEIN))
            {
                 app_audio_sendrequest(APP_PLAY_LINEIN_AUDIO, (uint8_t)APP_BT_SETTING_CLOSE, 0);
                 if(bt_media_get_current_media() & BT_STREAM_LINEIN)
                    bt_media_set_current_media(0);

                 bt_media_clear_media_type(stream_type,device_id);
            }
            break;
#endif

#ifdef AUDIO_PCM_PLAYER
        case BT_STREAM_PCM_PLAYER:
            if(bt_media_is_media_active_by_type(BT_STREAM_PCM_PLAYER))
            {
                 app_audio_sendrequest(APP_PLAY_PCM_PLAYER, (uint8_t)APP_BT_SETTING_CLOSE, 0);
                 if(bt_media_get_current_media() & BT_STREAM_PCM_PLAYER)
                    bt_media_set_current_media(0);

                 bt_media_clear_media_type(stream_type,device_id);
            }
            break;
#endif

        default:
            ASSERT(0,"bt_media_close ERROR TYPE: %x", stream_type);
            break;
    }

    TRACE(1, "bt_media_stop end %s\n", app_bt_get_active_media_state());
}

void app_media_stop_media(uint16_t stream_type, int device_id)
{
#if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))
#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
        audio_prompt_stop_playing();
#endif

    if(bt_media_is_media_active_by_type(BT_STREAM_MEDIA) || app_audio_list_playback_exist())
    {
#ifdef __AUDIO_QUEUE_SUPPORT__
        ////should have no music
        app_audio_list_clear();
#endif
        app_prompt_stop_all();

        bt_media_clear_media_type(BT_STREAM_MEDIA, device_id);
        if(bt_media_is_media_active_by_type(BT_STREAM_VOICE))
        {
            int currScoId = BT_DEVICE_INVALID_ID;
            currScoId = app_audio_manager_get_active_sco_num();

            if (currScoId == BT_DEVICE_INVALID_ID){
                uint8_t i = 0;
                for (i = 0; i < BT_DEVICE_NUM; i++){
                    if (bt_media_is_media_active_by_device(BT_STREAM_VOICE, i)){
                        currScoId = i;
                        break;
                    }
                }
                if (i == BT_DEVICE_NUM)
                {
                    for (i = BT_SOURCE_DEVICE_ID_BASE; i < BT_SOURCE_DEVICE_ID_BASE + BT_SOURCE_DEVICE_NUM; i++){
                        if (bt_media_is_media_active_by_device(BT_STREAM_VOICE, i)){
                            currScoId = i;
                            break;
                        }
                    }
                }
            }

            TRACE(2,"%s try to resume sco:%d", __func__, currScoId);
            if (currScoId != BT_DEVICE_INVALID_ID){
                bt_media_set_media_type(BT_STREAM_VOICE,currScoId);
                bt_media_set_current_media(BT_STREAM_VOICE);
#if BT_DEVICE_NUM > 1
                app_audio_manager_swap_sco(currScoId);
#endif
                app_audio_sendrequest(APP_BT_STREAM_HFP_PCM, (uint8_t)APP_BT_SETTING_OPEN, 0);
            }
        }
#ifdef __AI_VOICE__
        else if(bt_media_is_media_active_by_type(BT_STREAM_AI_VOICE))
        {
            if ((bt_media_get_current_media() & BT_STREAM_AI_VOICE) == 0)
            {
                app_audio_sendrequest(APP_BT_STREAM_AI_VOICE, (uint8_t)APP_BT_SETTING_OPEN, 0);
                bt_media_set_current_media(BT_STREAM_AI_VOICE);
            }
        }
#endif
        else if(bt_media_is_media_active_by_type(BT_STREAM_MUSIC))
        {
            int music_id  = bt_media_get_active_music_device();
            bt_media_set_media_type(BT_STREAM_MUSIC,device_id);
            app_audio_manager_switch_a2dp(music_id);
            bt_media_set_current_media(BT_STREAM_MUSIC);
            app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC,
                                  (uint8_t)(APP_BT_SETTING_SETUP),
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
                                  app_bt_get_curr_a2dp_sample_rate(music_id));
#else
                                  (uint32_t)(app_bt_get_device(music_id)->sample_rate & A2D_STREAM_SAMP_FREQ_MSK));
#endif
            app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC, (uint8_t)APP_BT_SETTING_OPEN, 0);
        }
    }
#endif
}

void app_media_update_media(uint16_t stream_type, int device_id)
{
    TRACE(1,"%s ",__func__);

#if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))
    if(bt_media_is_media_active_by_type(BT_STREAM_MEDIA))
    {
        //do nothing
        TRACE(0,"skip BT_STREAM_MEDIA");
    }
    else
#endif
    if(bt_media_is_media_active_by_type(BT_STREAM_VOICE) || btapp_hfp_is_call_active())
    {
        //do nothing
        TRACE(0,"skip BT_STREAM_VOICE");
        TRACE(3,"DEBUG INFO actByVoc:%d %d %d",    bt_media_is_media_active_by_type(BT_STREAM_VOICE),
                btapp_hfp_is_call_active(),
                btapp_hfp_incoming_calls());
    }
    else if(bt_media_is_media_active_by_type(BT_STREAM_MUSIC))
    {
#ifdef __AI_VOICE__
        if (bt_media_is_media_active_by_type(BT_STREAM_AI_VOICE) || ai_if_is_ai_stream_mic_open())
        {
            TRACE(0,"there is a ai voice stream exist, skip");
            return;
        }
#endif
        ///if another device is also in music mode
        TRACE(0,"try to resume music");
        int music_id  = bt_media_get_active_music_device();
        if (0 == (bt_media_get_current_media() & BT_STREAM_MUSIC)){
            app_audio_manager_switch_a2dp(music_id);
            bt_media_set_current_media(BT_STREAM_MUSIC);
            app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC,
                    (uint8_t)(APP_BT_SETTING_SETUP),
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
                    app_bt_get_curr_a2dp_sample_rate(music_id));
#else
                    (uint32_t)(app_bt_get_device(music_id)->sample_rate & A2D_STREAM_SAMP_FREQ_MSK));
#endif
            app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC, (uint8_t)APP_BT_SETTING_OPEN, 0);
        }
    }else{
        TRACE(0,"skip idle");
    }
}

int app_audio_manager_sco_status_checker(void)
{
#if defined(DEBUG)
    app_bt_audio_state_checker();
#endif
    return 0;
}

int app_audio_manager_swap_sco(int id)
{
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
#else // bes classic bt is disabled
    uint8_t curr_sco_id;
    bt_bdaddr_t bdAdd;
    uint16_t scohandle;
    struct BT_DEVICE_T* curr_device = NULL;

    if (bes_bt_me_get_max_sco_number() <= 1)
        return 0;

    curr_sco_id = app_audio_manager_get_active_sco_num();

    curr_device = app_bt_get_device(id);

    if (btif_hf_get_remote_bdaddr(curr_device->hf_channel, &bdAdd))
    {
        DUMP8("%02x ", bdAdd.address, BT_ADDR_OUTPUT_PRINT_NUM);

        app_audio_manager_set_active_sco_num(id);
        scohandle = btif_hf_get_sco_hcihandle(curr_device->hf_channel);
        if (scohandle !=  BT_INVALID_CONN_HANDLE)
        {
            app_bt_Me_switch_sco(scohandle);
        }
        app_bt_stream_volume_ptr_update(bdAdd.address);
        app_audio_manager_ctrl_volume(APP_AUDIO_MANAGER_VOLUME_CTRL_SET, app_bt_stream_volume_get_ptr()->hfp_vol);
        if(curr_sco_id != id)
        {
            TRACE(2, "%s try restart d%x", __func__, id);
            bt_sco_player_forcemute(true, true);
            bt_media_set_current_media(BT_STREAM_VOICE);
            bt_media_set_media_type(BT_STREAM_VOICE, id);
            app_audio_sendrequest(APP_BT_STREAM_HFP_PCM, (uint8_t)APP_BT_SETTING_RESTART, 0);
        }
        app_audio_manager_sco_status_checker();
    }
#endif
    return 0;
}

typedef void (*app_audio_local_volume_change_callback_t)(uint8_t device_id);

#if defined(USE_BASIC_THREADS)
int app_audio_manager_ctrl_volume_handle(APP_MESSAGE_BODY *msg_body)
#else
int app_audio_manager_ctrl_volume_handle(APP_AUDIO_MESSAGE_BODY *msg_body)
#endif
{
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
#else // bes classic bt is disabled
    enum APP_AUDIO_MANAGER_VOLUME_CTRL_T volume_ctrl;
    uint8_t local_volume_changed_device_id = BT_DEVICE_INVALID_ID;
    uint16_t volume_level = 0;

    volume_ctrl  = (enum APP_AUDIO_MANAGER_VOLUME_CTRL_T)msg_body->message_ptr;
    volume_level = (uint16_t)msg_body->message_Param0;

    switch (volume_ctrl) {
        case APP_AUDIO_MANAGER_VOLUME_CTRL_SET:
            app_bt_stream_volumeset(volume_level);
            app_bt_stream_volume_edge_check();
            break;
        case APP_AUDIO_MANAGER_VOLUME_CTRL_UP:
            local_volume_changed_device_id = app_bt_stream_local_volumeup();
            bes_bt_hfp_report_speak_gain();
            bes_bt_a2dp_report_speak_gain();
            break;
        case APP_AUDIO_MANAGER_VOLUME_CTRL_DOWN:
            local_volume_changed_device_id = app_bt_stream_local_volumedown();
            bes_bt_hfp_report_speak_gain();
            bes_bt_a2dp_report_speak_gain();
            break;
        default:
            break;
    }

    if (local_volume_changed_device_id != BT_DEVICE_INVALID_ID && msg_body->message_Param1)
    {
        ((app_audio_local_volume_change_callback_t)msg_body->message_Param1)(local_volume_changed_device_id);
    }
#endif
    return 0;
}

#if defined(USE_BASIC_THREADS)
int app_audio_manager_tune_samplerate_ratio_handle(APP_MESSAGE_BODY *msg_body)
#else
int app_audio_manager_tune_samplerate_ratio_handle(APP_AUDIO_MESSAGE_BODY *msg_body)
#endif
{
    enum AUD_STREAM_T stream = AUD_STREAM_NUM;
    float ratio = 1.0f;

    stream = (enum AUD_STREAM_T)msg_body->message_ptr;
    ratio = msg_body->message_Param3;

    app_bt_stream_set_tune_samplerate_ratio(stream, ratio);

    return 0;
}

static bool app_audio_manager_init = false;


int app_audio_manager_sendrequest(uint8_t massage_id, uint16_t stream_type, uint8_t device_id, uint16_t aud_id)
{
    uint32_t audevt;
    uint32_t msg0;
#if defined(USE_BASIC_THREADS)
    APP_MESSAGE_BLOCK msg;
#else
    APP_AUDIO_MESSAGE_BLOCK msg;
#endif

    TRACE(7,"%s %d%s %d%s d%x aud %d\n",
        __func__,
        massage_id, handleId2str(massage_id),
        stream_type, stream_type ? strmtype2str(stream_type) : "[N/A]",
        device_id, aud_id);
    TRACE(2, "%s ca %p", __func__, __builtin_return_address(0));

    if(app_audio_manager_init == false)
        return -1;

    // only allow prompt playing if powering-off is on-going
    if (app_is_power_off_in_progress())
    {
        if ((APP_BT_STREAM_MANAGER_START == massage_id) &&
            (BT_STREAM_MEDIA != stream_type))
        {
            return -1;
        }
    }

#if defined(USE_BASIC_THREADS)
    msg.mod_id = APP_MODULE_AUDIO_MANAGE;
    msg.mod_level = APP_MOD_LEVEL_3;
#else
    msg.mod_id = APP_AUDIO_MODULE_AUDIO_MANAGE;
#endif
    APP_AUDIO_MANAGER_SET_MESSAGE(audevt, massage_id, stream_type);
    APP_AUDIO_MANAGER_SET_MESSAGE0(msg0,device_id,aud_id);
    msg.msg_body.message_id = audevt;
    msg.msg_body.message_ptr = msg0;
    msg.msg_body.message_Param0 = msg0;
    msg.msg_body.message_Param1 = 0;
    msg.msg_body.message_Param2 = 0;
#if defined(USE_BASIC_THREADS)
    app_mailbox_put(&msg);
#else
    app_audio_mailbox_put(&msg);
#endif

    return 0;
}

int app_audio_manager_sendrequest_need_callback(
                           uint8_t massage_id, uint16_t stream_type, uint8_t device_id, uint16_t aud_id, uint32_t cb, uint32_t cb_param)
{
    uint32_t audevt;
    uint32_t msg0;
#if defined(USE_BASIC_THREADS)
    APP_MESSAGE_BLOCK msg;
#else
    APP_AUDIO_MESSAGE_BLOCK msg;
#endif

    TRACE(7,"aud_mgr send req:ca=%p %d%s %d%s d%x aud %d\n",
            __builtin_return_address(0),
            massage_id, handleId2str(massage_id),
            stream_type, stream_type ? strmtype2str(stream_type) : "[N/A]",
            device_id, aud_id);

    if(app_audio_manager_init == false)
        return -1;

#if defined(USE_BASIC_THREADS)
    msg.mod_id = APP_MODULE_AUDIO_MANAGE;
    msg.mod_level = APP_MOD_LEVEL_3;
#else
    msg.mod_id = APP_AUDIO_MODULE_AUDIO_MANAGE;
#endif
    APP_AUDIO_MANAGER_SET_MESSAGE(audevt, massage_id, stream_type);
    APP_AUDIO_MANAGER_SET_MESSAGE0(msg0,device_id,aud_id);
    msg.msg_body.message_id = audevt;
    msg.msg_body.message_ptr = msg0;
    msg.msg_body.message_Param0 = msg0;
    msg.msg_body.message_Param1 = cb;
    msg.msg_body.message_Param2 = cb_param;
#if defined(USE_BASIC_THREADS)
    app_mailbox_put(&msg);
#else
    app_audio_mailbox_put(&msg);
#endif

    return 0;
}

int app_audio_manager_ctrl_volume_with_callback(enum APP_AUDIO_MANAGER_VOLUME_CTRL_T volume_ctrl, uint16_t volume_level, void (*cb)(uint8_t device_id))
{
    uint32_t audevt;
#if defined(USE_BASIC_THREADS)
    APP_MESSAGE_BLOCK msg;
#else
    APP_AUDIO_MESSAGE_BLOCK msg;
#endif
    osThreadId currThreadId;

    if(app_audio_manager_init == false)
        return -1;

#if defined(USE_BASIC_THREADS)
    msg.mod_id = APP_MODULE_AUDIO_MANAGE;
    msg.mod_level = APP_MOD_LEVEL_3;
#else
    msg.mod_id = APP_AUDIO_MODULE_AUDIO_MANAGE;
#endif
    APP_AUDIO_MANAGER_SET_MESSAGE(audevt, APP_BT_STREAM_MANAGER_CTRL_VOLUME, 0);
    msg.msg_body.message_id     = audevt;
    msg.msg_body.message_ptr    = (uint32_t)volume_ctrl;
    msg.msg_body.message_Param0 = (uint32_t)volume_level;
    msg.msg_body.message_Param1 = (uint32_t)(uintptr_t)cb;
    msg.msg_body.message_Param2 = 0;
    msg.msg_body.message_Param3 = 0;
    currThreadId = osThreadGetId();
#if defined(USE_BASIC_THREADS)
    if (currThreadId == af_thread_tid_get()){
        app_audio_manager_ctrl_volume_handle(&msg.msg_body);
    }else{
        app_mailbox_put(&msg);
    }
#else
    if (currThreadId == af_thread_tid_get() ||
        currThreadId == app_audio_thread_tid_get()){
        app_audio_manager_ctrl_volume_handle(&msg.msg_body);
    }else{
        app_audio_mailbox_put(&msg);
    }
#endif
    return 0;
}

int app_audio_manager_ctrl_volume(enum APP_AUDIO_MANAGER_VOLUME_CTRL_T volume_ctrl, uint16_t volume_level)
{
    return app_audio_manager_ctrl_volume_with_callback(volume_ctrl, volume_level, NULL);
}

int app_audio_manager_tune_samplerate_ratio(enum AUD_STREAM_T stream, float ratio)
{
    uint32_t audevt;
#if defined(USE_BASIC_THREADS)
    APP_MESSAGE_BLOCK msg;
#else
    APP_AUDIO_MESSAGE_BLOCK msg;
    osThreadId app_audio_thread_id;
#endif
    osThreadId currThreadId;
    osThreadId af_thread_id;

    if(app_audio_manager_init == false)
        return -1;

#if defined(USE_BASIC_THREADS)
    msg.mod_id = APP_MODULE_AUDIO_MANAGE;
    msg.mod_level = APP_MOD_LEVEL_3;
#else
    msg.mod_id = APP_AUDIO_MODULE_AUDIO_MANAGE;
#endif
    APP_AUDIO_MANAGER_SET_MESSAGE(audevt, APP_BT_STREAM_MANAGER_TUNE_SAMPLERATE_RATIO, 0);
    msg.msg_body.message_id     = audevt;
    msg.msg_body.message_ptr    = (uint32_t)stream;
    msg.msg_body.message_Param0 = 0;
    msg.msg_body.message_Param1 = 0;
    msg.msg_body.message_Param2 = 0;
    msg.msg_body.message_Param3 = ratio;

    currThreadId = osThreadGetId();
    af_thread_id = af_thread_tid_get();
#if defined(USE_BASIC_THREADS)
    //TRACE(3, "current id: %p, af thread id: %p, app audio id: %p", 
    //    currThreadId, af_thread_id, app_audio_thread_id);

    if (currThreadId == af_thread_id){
        app_audio_manager_tune_samplerate_ratio_handle(&msg.msg_body);
    }else{
        app_mailbox_put(&msg);
    }
#else
    app_audio_thread_id = app_audio_thread_tid_get();
    //TRACE(3, "current id: %p, af thread id: %p, app audio id: %p", 
    //    currThreadId, af_thread_id, app_audio_thread_id);

    if (currThreadId == af_thread_id ||
        currThreadId == app_audio_thread_id){
        app_audio_manager_tune_samplerate_ratio_handle(&msg.msg_body);
    }else{
        app_audio_mailbox_put(&msg);
    }
#endif
    return 0;
}

void app_audio_manager_set_output_route(Audio_output_route_t *route_info)
{
    if (app_audio_manager_init == false)
        return;

    uint32_t audevt;
#if defined(USE_BASIC_THREADS)
    APP_MESSAGE_BLOCK msg;
    msg.mod_id = APP_MODULE_AUDIO_MANAGE;
    msg.mod_level = APP_MOD_LEVEL_3;
#else
    APP_AUDIO_MESSAGE_BLOCK msg;
    msg.mod_id = APP_AUDIO_MODULE_AUDIO_MANAGE;
#endif
    APP_AUDIO_MANAGER_SET_MESSAGE(audevt, APP_BT_STREAM_MANAGER_SET_OUTPUT_ROUTE, 0);
    msg.msg_body.message_id     = audevt;
    msg.msg_body.message_ptr    = (uint32_t)route_info;
    msg.msg_body.message_Param0 = 0;
    msg.msg_body.message_Param1 = 0;
    msg.msg_body.message_Param2 = 0;
#if defined(USE_BASIC_THREADS)
    app_mailbox_put(&msg);
#else
    app_audio_mailbox_put(&msg);
#endif
}

void audio_manager_stream_ctrl_start_ble_audio(uint8_t conlid, uint16_t stream_type, uint8_t stream_lid)
{
    //need more considerable to update active device and call state/
    bt_meida.media_curr_music = BT_DEVICE_INVALID_ID;
    bt_meida.media_curr_sco = BT_DEVICE_INVALID_ID;
    bt_meida.curr_active_media = 0;
    app_audio_manager_sendrequest(APP_BT_BLE_STREAM_MANAGER_START, stream_type, conlid, stream_lid);
}

void audio_manager_stream_ctrl_stop_ble_audio(uint8_t conlid, uint16_t stream_type, uint8_t stream_lid)
{
    app_audio_manager_sendrequest(APP_BT_BLE_STREAM_MANAGER_STOP, (uint16_t)stream_type, conlid, stream_lid);
}

void audio_manager_stream_ctrl_stop_single_ble_audio_stream(uint8_t conlid, uint16_t stream_type, uint8_t stream_lid)
{
    app_audio_manager_sendrequest(APP_BT_BLE_STREAM_MANAGER_STOP_SINGLE_STREAM, stream_type, conlid, stream_lid);
}

#ifdef BLE_WALKIE_TALKIE

static APP_WT_AUDIO_MANAGER_CALLBACK_T g_app_wt_audio_start_cb = NULL;
static APP_WT_AUDIO_MANAGER_CALLBACK_T g_app_wt_audio_stop_cb = NULL;

void app_audio_manager_register_wt_start_callback(APP_WT_AUDIO_MANAGER_CALLBACK_T cb)
{
    g_app_wt_audio_start_cb = cb;
}

void app_audio_manager_register_wt_stop_callback(APP_WT_AUDIO_MANAGER_CALLBACK_T cb)
{
    g_app_wt_audio_stop_cb = cb;
}

void audio_manager_stream_ctrl_start_wt_audio(uint8_t device_id,uint32_t context_type)
{
    bt_meida.media_curr_music = BT_DEVICE_INVALID_ID;
    bt_meida.media_curr_sco = BT_DEVICE_INVALID_ID;
    bt_meida.curr_active_media = 0;
    app_audio_sendrequest(APP_BT_STREAM_INVALID, (uint8_t)APP_BT_SETTING_CLOSEALL, 0);

    app_audio_manager_sendrequest(APP_BT_WT_STREAM_MANAGER_START, (uint16_t)context_type, device_id, 0);
}

void audio_manager_stream_ctrl_stop_wt_audio(uint8_t device_id,uint32_t context_type)
{
    app_audio_manager_sendrequest(APP_BT_WT_STREAM_MANAGER_STOP, (uint16_t)context_type, device_id, 0);
}

#endif

static APP_BIS_TRAN_AUDIO_MANAGER_CALLBACK_T g_app_bis_tran_audio_start_cb = NULL;
static APP_BIS_TRAN_AUDIO_MANAGER_CALLBACK_T g_app_bis_tran_audio_stop_cb = NULL;

void app_audio_manager_register_bis_tran_callback(APP_BIS_TRAN_AUDIO_MANAGER_CALLBACK_T start_cb,
                                                                    APP_BIS_TRAN_AUDIO_MANAGER_CALLBACK_T stop_cb)
{
    g_app_bis_tran_audio_start_cb = start_cb;
    g_app_bis_tran_audio_stop_cb  = stop_cb;
}

void app_audio_manager_unregister_bis_tran_callback()
{
    g_app_bis_tran_audio_start_cb = NULL;
    g_app_bis_tran_audio_stop_cb  = NULL;
}

void audio_manager_stream_ctrl_start_bis_tran_audio(uint8_t device_id,uint32_t context_type)
{
    bt_meida.media_curr_music = BT_DEVICE_INVALID_ID;
    bt_meida.media_curr_sco = BT_DEVICE_INVALID_ID;
    bt_meida.curr_active_media = 0;
    app_audio_sendrequest(APP_BT_STREAM_INVALID, (uint8_t)APP_BT_SETTING_CLOSEALL, 0);

    app_audio_manager_sendrequest(APP_BT_BIS_TRAN_STREAM_MANAGER_START, (uint16_t)context_type, device_id, 0);
}

void audio_manager_stream_ctrl_stop_bis_tran_audio(uint8_t device_id,uint32_t context_type)
{
    app_audio_manager_sendrequest(APP_BT_BIS_TRAN_STREAM_MANAGER_STOP, (uint16_t)context_type, device_id, 0);
}


#if defined(USE_BASIC_THREADS)
static int app_audio_manager_handle_process(APP_MESSAGE_BODY *msg_body)
#else
static int app_audio_manager_handle_process(APP_AUDIO_MESSAGE_BODY *msg_body)
#endif
{
    int nRet = 0;

    APP_AUDIO_MANAGER_MSG_STRUCT aud_manager_msg;
    APP_AUDIO_MANAGER_CALLBACK_T callback_fn = NULL;
    uint32_t callback_param = 0;

    if(app_audio_manager_init == false)
        return -1;

    APP_AUDIO_MANAGER_GET_ID(msg_body->message_id, aud_manager_msg.id);
    APP_AUDIO_MANAGER_GET_STREAM_TYPE(msg_body->message_id, aud_manager_msg.stream_type);
    APP_AUDIO_MANAGER_GET_DEVICE_ID(msg_body->message_Param0, aud_manager_msg.device_id);
    APP_AUDIO_MANAGER_GET_AUD_ID(msg_body->message_Param0, aud_manager_msg.aud_id);
    APP_AUDIO_MANAGER_GET_CALLBACK(msg_body->message_Param1, callback_fn);
    APP_AUDIO_MANAGER_GET_CALLBACK_PARAM(msg_body->message_Param2, callback_param);

    TRACE(7, "%s %d%s %x%s d%x aud %x", __func__,
          aud_manager_msg.id, handleId2str(aud_manager_msg.id),
          aud_manager_msg.stream_type, aud_manager_msg.stream_type ? strmtype2str(aud_manager_msg.stream_type) : "[N/A]",
          aud_manager_msg.device_id, aud_manager_msg.aud_id);

    switch (aud_manager_msg.id) {
        case APP_BT_STREAM_MANAGER_START:
            bt_media_start(aud_manager_msg.stream_type, aud_manager_msg.device_id, aud_manager_msg.aud_id);
            break;
        case APP_BT_STREAM_MANAGER_STOP:
            bt_media_stop(aud_manager_msg.stream_type, aud_manager_msg.device_id, aud_manager_msg.aud_id);
            break;
        case APP_BT_STREAM_MANAGER_STOP_MEDIA:
            app_media_stop_media(aud_manager_msg.stream_type, aud_manager_msg.device_id);
            break;
        case APP_BT_STREAM_MANAGER_UPDATE_MEDIA:
            app_media_update_media(aud_manager_msg.stream_type, aud_manager_msg.device_id);
            break;
        case APP_BT_STREAM_MANAGER_SWAP_SCO:
            app_audio_manager_swap_sco(aud_manager_msg.device_id);
            break;
        case APP_BT_STREAM_MANAGER_CTRL_VOLUME:
            app_audio_manager_ctrl_volume_handle(msg_body);
            return nRet;
            break;
        case APP_BT_STREAM_MANAGER_TUNE_SAMPLERATE_RATIO:
            app_audio_manager_tune_samplerate_ratio_handle(msg_body);
            break;
        case APP_BT_STREAM_MANAGER_SET_OUTPUT_ROUTE:
            app_audio_sendrequest(APP_BT_STREAM_A2DP_SBC, ( uint8_t )APP_BT_SETTING_OUTPUT_ROUTE, msg_body->message_ptr);
            break;
        case APP_BT_BLE_STREAM_MANAGER_START:
            app_audio_sendrequest(APP_BT_STREAM_INVALID, (uint8_t)APP_BT_SETTING_CLOSEALL, 0);
            app_audio_sendrequest_param(aud_manager_msg.device_id, (uint8_t)APP_BT_SETTING_START_BLEAUDIO,
                aud_manager_msg.stream_type, aud_manager_msg.aud_id);
            break;
        case APP_BT_BLE_STREAM_MANAGER_STOP:
            app_audio_sendrequest_param(aud_manager_msg.device_id, (uint8_t)APP_BT_SETTING_STOP_BLEAUDIO,
                aud_manager_msg.stream_type, aud_manager_msg.aud_id);
            break;
        case APP_BT_BLE_STREAM_MANAGER_STOP_SINGLE_STREAM:
            app_audio_sendrequest_param(aud_manager_msg.device_id, (uint8_t)APP_BT_SETTING_STOP_SINGLE_STREAM,
                aud_manager_msg.stream_type, aud_manager_msg.aud_id);
            break;
#ifdef BLE_WALKIE_TALKIE
        case APP_BT_WT_STREAM_MANAGER_START:
            if (g_app_wt_audio_start_cb)
            {
                g_app_wt_audio_start_cb(aud_manager_msg.device_id,aud_manager_msg.stream_type);
            }
            break;
        case APP_BT_WT_STREAM_MANAGER_STOP:
            if (g_app_wt_audio_stop_cb)
            {
                g_app_wt_audio_stop_cb(aud_manager_msg.device_id,aud_manager_msg.stream_type);
            }
            break;
#endif
        case APP_BT_BIS_TRAN_STREAM_MANAGER_START:
            if (g_app_bis_tran_audio_start_cb)
            {
                g_app_bis_tran_audio_start_cb(aud_manager_msg.device_id,aud_manager_msg.stream_type);
            }
            break;
        case APP_BT_BIS_TRAN_STREAM_MANAGER_STOP:
            if (g_app_bis_tran_audio_stop_cb)
            {
                g_app_bis_tran_audio_stop_cb(aud_manager_msg.device_id,aud_manager_msg.stream_type);
            }
            break;
        default:
            break;
    }
    if (callback_fn){
        callback_fn(aud_manager_msg.device_id, aud_manager_msg.id, callback_param);
    }

    return nRet;
}

void bt_media_volume_ptr_update_by_mediatype(uint16_t stream_type)
{
    POSSIBLY_UNUSED uint8_t id = 0;
    POSSIBLY_UNUSED struct BT_DEVICE_T *curr_device = NULL;

    TRACE(2,"%s %d enter", __func__, stream_type);
    if (stream_type & bt_media_get_current_media()){
        switch (stream_type) {
            case BT_STREAM_MUSIC:
                id = bt_meida.media_curr_music;
                ASSERT(id<BT_DEVICE_NUM, "INVALID_BT_DEVICE_NUM"); // only a2dp sink has speaker out music media
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
#else // bes classic bt is disabled
                curr_device = app_bt_get_device(id);
                app_bt_stream_volume_ptr_update(curr_device->remote.address);
#endif
                break;
            case BT_STREAM_VOICE:
                id = app_audio_manager_get_active_sco_num();
                ASSERT(id<BT_DEVICE_NUM || (id>=BT_SOURCE_DEVICE_ID_BASE && id<BT_SOURCE_DEVICE_ID_BASE+BT_SOURCE_DEVICE_NUM), "INVALID_BT_DEVICE_NUM");
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
#else // bes classic bt is disabled
                curr_device = app_bt_get_device(id);
                app_bt_stream_volume_ptr_update(curr_device->remote.address);
#endif
                break;
            case BT_STREAM_MEDIA:
            default:
                break;
        }
    }
    TRACE(1,"%s exit", __func__);
}

int app_audio_manager_set_active_sco_num(int id)
{
    bt_meida.media_curr_sco = id;
    return 0;
}

int app_audio_manager_get_active_sco_num(void)
{
    return bt_meida.media_curr_sco;
}

#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
#else // bes classic bt is disabled
btif_hf_channel_t* app_audio_manager_get_active_sco_chnl(void)
{
    int curr_sco;

    curr_sco = app_audio_manager_get_active_sco_num();
    if (curr_sco != BT_DEVICE_INVALID_ID)
    {
        return app_bt_get_device(curr_sco)->hf_channel;
    }
    return NULL;
}
#endif

hfp_sco_codec_t app_audio_manager_get_scocodecid(void)
{
    hfp_sco_codec_t scocodecid = BT_HFP_SCO_CODEC_NONE;
    if (bt_meida.media_curr_sco != BT_DEVICE_INVALID_ID){
        scocodecid = (hfp_sco_codec_t)bt_adapter_get_hfp_sco_codec_type(bt_meida.media_curr_sco);
    }
    return scocodecid;
}

int app_audio_manager_switch_a2dp(int id)
{
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
#else // bes classic bt is disabled
    bt_bdaddr_t *bdAdd = NULL;
    bdAdd = app_bt_get_remote_device_address(id);
    TRACE(2, "%s switch_a2dp to id:%d, addr:%p", __func__, id, bdAdd);

    if (bdAdd)
    {
        app_bt_stream_volume_ptr_update(bdAdd->address);
        bt_meida.media_curr_music = id;
    }
#endif
    return 0;
}

bool app_audio_manager_a2dp_is_active(int id)
{
    uint16_t media_type;
    bool nRet = false;

    media_type = bt_media_get_current_media();
    if (media_type & BT_STREAM_MUSIC){
        if (bt_meida.media_curr_music == id){
            nRet = true;
        }
    }

/*#ifndef BES_AUTOMATE_TEST  //only for debug
    TRACE(5,"%s nRet:%d type:%d %d/%d", __func__, nRet, media_type, id, bt_meida.media_curr_music);
#endif*/
    return nRet;
}

bool app_audio_manager_hfp_is_active(int id)
{
    uint16_t media_type;
    bool nRet = false;

    media_type = bt_media_get_current_media();
    if (media_type & BT_STREAM_VOICE){
        if (bt_meida.media_curr_sco == id){
            nRet = true;
        }
    }

#ifndef BES_AUTOMATE_TEST
    TRACE(5,"%s nRet:%d type:%d %d/%d", __func__, nRet, media_type, id, bt_meida.media_curr_sco);
#endif
    return nRet;
}

bool app_audio_manager_media_is_active(void)
{
    uint16_t media_type;
    bool nRet = false;

    media_type = bt_media_get_current_media();
    if (media_type & BT_STREAM_MEDIA){
            nRet = true;
    }

    return nRet;
}

void app_audio_manager_open(void)
{
    if(app_audio_manager_init){
        return;
    }

    bt_media_player_register();

    bt_meida.media_curr_music = BT_DEVICE_INVALID_ID;
    bt_meida.media_curr_sco = BT_DEVICE_INVALID_ID;
    bt_meida.curr_active_media = 0;
#if defined(USE_BASIC_THREADS)
    app_set_threadhandle(APP_MODULE_AUDIO_MANAGE, app_audio_manager_handle_process);
#else
    app_audio_set_threadhandle(APP_AUDIO_MODULE_AUDIO_MANAGE, app_audio_manager_handle_process);
#endif
    app_bt_stream_register_get_a2dp_non_type_callback(app_audio_manager_get_a2dp_non_type);
    app_audio_adm_init();
    app_audio_focus_init();
    app_audio_control_reset_media_call_info();
    app_audio_manager_init = true;

#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
    audio_prompt_init_handler();
#endif
}

void app_audio_manager_close(void)
{
#if defined(USE_BASIC_THREADS)
    app_set_threadhandle(APP_MODULE_AUDIO_MANAGE, NULL);
#else
    app_audio_set_threadhandle(APP_AUDIO_MODULE_AUDIO_MANAGE, NULL);
#endif
    app_bt_stream_register_get_a2dp_non_type_callback(NULL);
    app_audio_manager_init = false;
}

#ifdef RB_CODEC

static bool app_rbcodec_play_status = false;

static bool app_rbplay_player_mode = false;

bool app_rbplay_is_localplayer_mode(void)
{
    return app_rbplay_player_mode;
}

bool app_rbplay_mode_switch(void)
{
    return (app_rbplay_player_mode = !app_rbplay_player_mode);
}

void app_rbplay_set_player_mode(bool isInPlayerMode)
{
    app_rbplay_player_mode = isInPlayerMode;
}

void app_rbcodec_ctr_play_onoff(bool on )
{
    TRACE(3,"%s %d ,turnon?%d ",__func__,app_rbcodec_play_status,on);

    if(app_rbcodec_play_status == on)
        return;
    app_rbcodec_play_status = on;
    if(on)
        app_audio_manager_sendrequest( APP_BT_STREAM_MANAGER_START, BT_STREAM_RBCODEC, 0, 0);
    else
        app_audio_manager_sendrequest( APP_BT_STREAM_MANAGER_STOP, BT_STREAM_RBCODEC, 0, 0);
}

void app_rbcodec_ctl_set_play_status(bool st)
{
    app_rbcodec_play_status = st;
    TRACE(2,"%s %d",__func__,app_rbcodec_play_status);
}

bool app_rbcodec_get_play_status(void)
{
    TRACE(2,"%s %d",__func__,app_rbcodec_play_status);
    return app_rbcodec_play_status;
}

void app_rbcodec_toggle_play_stop(void)
{
    if(app_rbcodec_get_play_status()) {
        app_rbcodec_ctr_play_onoff(false);
    } else {
        app_rbcodec_ctr_play_onoff(true);
    }
}

bool app_rbcodec_check_hfp_active(void )
{
    return (bool)bt_media_is_media_active_by_type(BT_STREAM_VOICE);
}
#endif

void app_ibrt_sync_mix_prompt_req_handler(uint8_t* ptrParam, uint16_t paramLen)
{
#if defined(TWS_PROMPT_SYNC) && !defined(AUDIO_PROMPT_USE_DAC2_ENABLED)
    app_tws_cmd_sync_mix_prompt_req_handler(ptrParam, paramLen);
#endif
}

void app_audio_decode_err_force_trigger(void)
{
#ifdef MEDIA_PLAYER_SUPPORT
#ifndef IBRT
    media_PlayAudio_standalone(AUDIO_ID_BT_MUTE, 0);
#endif
#endif
}
