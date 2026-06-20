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
#include <stdlib.h>
#include <assert.h>
#include "cmsis.h"
#include "cmsis_os.h"
#include "tgt_hardware.h"
#include "list.h"

#ifdef MEDIA_PLAYER_SUPPORT
#if defined(USE_BASIC_THREADS)
#include "app_thread.h"
#endif
#include "hal_uart.h"
#include "hal_timer.h"
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
#include "app_media_player.h"
#include "res_audio_ring.h"
#include "audio_prompt_sbc.h"
#include "app_bt.h"
#include "besbt.h"
#include "audio_policy.h"
#if defined(BT_SOURCE)
#include "bt_source.h"
#endif
#include "cqueue.h"
#include "btapp.h"
#include "app_bt_media_manager.h"
#include "a2dp_decoder.h"
#ifdef PROMPT_IN_FLASH
#include "nvrecord_prompt.h"
#else /*PROMPT_IN_FLASH*/

#define SUPPORT_ENGLISH (1 << 0)
#define SUPPORT_CHINESE (1 << 1)
#define SUPPORT_GENEMY (1 << 2)

// #define MEDIA_SUPPORT_LANG (SUPPORT_ENGLISH | SUPPORT_CHINESE)
#define MEDIA_SUPPORT_LANG SUPPORT_ENGLISH

#if MEDIA_SUPPORT_LANG & SUPPORT_ENGLISH
#include "res_audio_data.h"
#endif

#if MEDIA_SUPPORT_LANG & SUPPORT_CHINESE
#include "res_audio_data_cn.h"
#endif
#endif /*PROMPT_IN_FLASH*/

#if defined(AUDIO_ANC_FB_MC_MEDIA) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
#include"anc_process.h"
#include "hal_codec.h"
#endif

#ifdef __THIRDPARTY
#include "app_thirdparty.h"
#endif

#if defined(IBRT)
#include "app_tws_ibrt.h"
#include "app_ibrt_internal.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_tws_ctrl_thread.h"
#include "app_ibrt_voice_report.h"
#endif

#if defined(ANC_ASSIST_ENABLED) && defined(ANC_ASSIST_USE_INT_CODEC)
#include "app_anc_assist.h"
#endif

#if defined(__SW_IIR_PROMPT_EQ_PROCESS__)
#include "iir_process.h"
#endif

#if defined(AUDIO_PROMPT_USE_DAC2_ENABLED)
#define MEDIA_PLAYER_USE_CODEC2
#endif

#if defined(FREEMAN_ENABLED_STERO)
#define MEDIA_PLAYER_CHANNEL_NUM (AUD_CHANNEL_NUM_2)
#elif defined(PLAYBACK_USE_I2S)
#define MEDIA_PLAYER_CHANNEL_NUM (AUD_CHANNEL_NUM_2)
#else
#define MEDIA_PLAYER_CHANNEL_NUM (AUD_CHANNEL_NUM_1)
#endif

#ifdef MEDIA_PLAYER_USE_CODEC2
#define MEDIA_PLAYER_OUTPUT_DEVICE (AUD_STREAM_USE_INT_CODEC2)
#define AF_CODEC_TUNE af_codec_tune_dac2
#else
#define MEDIA_PLAYER_OUTPUT_DEVICE (AUD_STREAM_USE_INT_CODEC)
#define AF_CODEC_TUNE af_codec_tune
#endif

#ifdef __INTERACTION__
uint8_t g_findme_fadein_vol = TGT_VOLUME_LEVEL_1;
#endif
#include "sbc_api.h"
#include "sbc_error_code.h"
static char need_init_decoder = 1;
static sbc_decoder_t *media_sbc_decoder = NULL;

#define SBC_TEMP_BUFFER_SIZE 64
#define SBC_QUEUE_SIZE (SBC_TEMP_BUFFER_SIZE*4)
static CQueue media_sbc_queue;

#define CFG_HW_AUD_EQ_NUM_BANDS (8)
static const float media_sbc_eq_band_gain[CFG_HW_AUD_EQ_NUM_BANDS] = {1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0};

// 128 * 2: 16ms for 16k sample rate
#if defined(PROMPT_USE_AAC)
#include "aac_api.h"
#include "aac_error_code.h"
#include "heap_api.h"
static void * aacDec_handle = NULL;
static heap_handle_t aac_memhandle = NULL;
static uint8_t *aac_mempoll = NULL;
static heap_api_t aac_heap_api;
static void * aac_malloc(const unsigned size)
{
    void * ptr = NULL;
    multi_heap_info_t info;
    heap_get_info(aac_memhandle, &info);
    if (size >= info.total_free_bytes){
        TRACE(0, "aac_malloc failed need:%d, free_bytes:%d \n", size, info.total_free_bytes);
        return ptr;
    }
    TRACE(0, "aac_malloc size=%u free=%u alloc=%u", size, info.total_free_bytes, info.total_allocated_bytes);
    ptr = heap_malloc(aac_memhandle,size);
    return ptr;
}

static void aac_free(void * ptr)
{
    heap_free(aac_memhandle, (int*)ptr);
}

static heap_api_t aac_get_heap_api()
{
    heap_api_t api;
    api.malloc = &aac_malloc;
    api.free = &aac_free;
    return api;
}
#define APP_AUDIO_PLAYBACK_BUFF_SIZE        (1024 * 1 * sizeof(int16_t) * MEDIA_PLAYER_CHANNEL_NUM * 2)
#else
#define APP_AUDIO_PLAYBACK_BUFF_SIZE        (128 * 2 * sizeof(int16_t) * MEDIA_PLAYER_CHANNEL_NUM * 2)
#endif

#if defined(AUDIO_ANC_FB_MC_MEDIA) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
static enum AUD_BITS_T sample_size_play_bt;
static enum AUD_SAMPRATE_T sample_rate_play_bt;
static uint32_t data_size_play_bt;

static uint8_t *playback_buf_bt;
static uint32_t playback_size_bt;
static int32_t playback_samplerate_ratio_bt;

static uint8_t *playback_buf_mc;
static uint32_t playback_size_mc;
static enum AUD_CHANNEL_NUM_T  playback_ch_num_bt;
#endif

#if defined(ANC_ASSIST_ENABLED)
#include "app_voice_assist_prompt_leak_detect.h"
#include "app_voice_assist_custom_leak_detect.h"
#endif

typedef void (*media_PlayAudio_api)(AUD_ID_ENUM id,uint8_t device_id);
typedef struct {
    uint8_t  reqType;    // one of APP_PROMPT_PLAY_REQ_TYPE_E
    uint8_t  isLocalPlaying;
    uint8_t  deviceId;
    uint16_t promptId;
    media_PlayAudio_api func;
} APP_PROMPT_PLAY_REQ_T;

/**< signal of the prompt handler thread */
#define PROMPT_HANDLER_SIGNAL_NEW_PROMPT_REQ        0x01
#define PROMPT_HANDLER_SIGNAL_CLEAR_REQ             0x02
#define PROMPT_HANDLER_SIGNAL_PLAYING_COMPLETED     0x04

static list_t *app_prompt_list;
static bool app_prompt_is_on_going = false;

static bool app_prompt_list_append(list_t *list, APP_PROMPT_PLAY_REQ_T *req);

#if defined(USE_BASIC_THREADS)
static int app_prompt_handler(APP_MESSAGE_BODY *msg_body);
#else
static osThreadId app_prompt_handler_tid;
static void app_prompt_handler_thread(void const *argument);
osThreadDef(app_prompt_handler_thread, osPriorityHigh, 1, 1536, "app_prompt_handler");
#endif

#define  SBC_FRAME_LEN  64 //0x5c   /* pcm 512 bytes*/
static uint8_t* g_app_audio_data = NULL;
static uint32_t g_app_audio_length = 0;
static uint32_t g_app_audio_read = 0;

static uint32_t g_play_continue_mark = 0;

static bool g_is_continuous_prompt_on_going = false;
static AUD_ID_ENUM continuous_prompt_id = AUD_ID_INVALID;
static bool g_is_local_continuous_prompt_on_going = false;
#if defined(__SW_IIR_PROMPT_EQ_PROCESS__)
extern IIR_CFG_T * const POSSIBLY_UNUSED audio_eq_hw_dac_iir_cfg_list[EQ_HW_DAC_IIR_LIST_NUM];
#endif

static void app_prompt_start_continuous_checker(void);
static void app_prompt_stop_continuous_checker(void);

static uint8_t app_play_sbc_stop_proc_cnt = 0;

static uint16_t g_prompt_chnlsel = PROMOT_ID_BIT_MASK_CHNLSEl_ALL;

static uint16_t app_prompt_id_on_going;

static void prompt_check_user_when_streaming_is_completed(void);
//for continue play

typedef struct {
    uint8_t *data;  //total files
    uint32_t fsize; //file index
} media_sound_map_t;

static const media_sound_map_t*  media_sound_map;

static const media_sound_map_t * get_sound_media_map(int language);

#define _CONCAT(l, r) l##r
#define CONCAT(l, r) _CONCAT(l, r)

#define SOUND_ID(l, r) CONCAT(l, r)

enum sound_id {
    SOUND_ZERO = 0,
    SOUND_ONE,
    SOUND_TWO,
    SOUND_THREE,
    SOUND_FOUR,
    SOUND_FIVE,
    SOUND_SIX,
    SOUND_SEVEN,
    SOUND_EIGHT,
    SOUND_NINE,
#if defined ANC_ASSIST_ENABLED
    SOUND_PROMPT_ADAPTIVE_ANC,
    SOUND_CUSTOM_LEAK_DETECT,
#endif

    POWER_ON,
    POWER_OFF,
    BT_PAIR_ENABLE,
    BT_PAIRING,
    BT_PAIRING_SUCCESS,
    BT_PAIRING_FAIL,
    BT_REFUSE,
    BT_OVER,
    BT_ANSWER,
    BT_HUNG_UP,
    BT_INCOMING_CALL,
    CHARGE_PLEASE,
    CHARGE_FINISH,
    BT_CONNECTED,
    BT_DIS_CONNECT,
    BT_WARNING,
    BT_ALEXA_START,
    BT_ALEXA_STOP,

#if defined(BISTO_ENABLED) || defined(__AI_VOICE__)
    BT_GSOUND_MIC_OPEN,
    BT_GSOUND_MIC_CLOSE,
    BT_GSOUND_NC,
#endif

    LANGUAGE_SWITCH,
    BT_MUTE,
#ifdef __BT_WARNING_TONE_MERGE_INTO_STREAM_SBC__
    RES_AUD_RING_SAMPRATE_16000,
#endif

#ifdef __INTERACTION__
    BT_FINDME,
#endif
    MAX_SOUND_ID
};

#ifdef PROMPT_IN_FLASH
static media_sound_map_t media_number_sound_map[] = {0};
#else
#define SOUND_ITEM_DEF(l, item)    \
    [item] = {(uint8_t *)SOUND_ID(l, item), sizeof(SOUND_ID(l, item))}

#if MEDIA_SUPPORT_LANG & SUPPORT_CHINESE
static const media_sound_map_t media_sound_map_cn[] =
{
    SOUND_ITEM_DEF(CN_, SOUND_ZERO),
    SOUND_ITEM_DEF(CN_, SOUND_ONE),
    SOUND_ITEM_DEF(CN_, SOUND_TWO),
    SOUND_ITEM_DEF(CN_, SOUND_THREE),
    SOUND_ITEM_DEF(CN_, SOUND_FOUR),
    SOUND_ITEM_DEF(CN_, SOUND_FIVE),
    SOUND_ITEM_DEF(CN_, SOUND_SIX),
    SOUND_ITEM_DEF(CN_, SOUND_SEVEN),
    SOUND_ITEM_DEF(CN_, SOUND_EIGHT),
    SOUND_ITEM_DEF(CN_, SOUND_NINE),
#if defined ANC_ASSIST_ENABLED
    SOUND_ITEM_DEF(CN_, SOUND_PROMPT_ADAPTIVE_ANC),
    SOUND_ITEM_DEF(CN_, SOUND_CUSTOM_LEAK_DETECT),
#endif

    SOUND_ITEM_DEF(CN_, POWER_ON),
    SOUND_ITEM_DEF(CN_, POWER_OFF),
    SOUND_ITEM_DEF(CN_, BT_PAIR_ENABLE),
    SOUND_ITEM_DEF(CN_, BT_PAIRING),
    SOUND_ITEM_DEF(CN_, BT_PAIRING_SUCCESS),
    SOUND_ITEM_DEF(CN_, BT_PAIRING_FAIL),
    SOUND_ITEM_DEF(CN_, BT_REFUSE),
    SOUND_ITEM_DEF(CN_, BT_OVER),
    SOUND_ITEM_DEF(CN_, BT_ANSWER),
    SOUND_ITEM_DEF(CN_, BT_HUNG_UP),
    SOUND_ITEM_DEF(CN_, BT_INCOMING_CALL),
    SOUND_ITEM_DEF(CN_, CHARGE_PLEASE),
    SOUND_ITEM_DEF(CN_, CHARGE_FINISH),
    SOUND_ITEM_DEF(CN_, BT_CONNECTED),
    SOUND_ITEM_DEF(CN_, BT_DIS_CONNECT),
    SOUND_ITEM_DEF(CN_, BT_WARNING),
    SOUND_ITEM_DEF(CN_, BT_ALEXA_START),
    SOUND_ITEM_DEF(CN_, BT_ALEXA_STOP),

#if defined(BISTO_ENABLED) || defined(__AI_VOICE__)
    SOUND_ITEM_DEF(CN_, BT_GSOUND_MIC_OPEN),
    SOUND_ITEM_DEF(CN_, BT_GSOUND_MIC_CLOSE),
    SOUND_ITEM_DEF(CN_, BT_GSOUND_NC),
#endif

    SOUND_ITEM_DEF(CN_, LANGUAGE_SWITCH),
    SOUND_ITEM_DEF(CN_, BT_MUTE),
#ifdef __BT_WARNING_TONE_MERGE_INTO_STREAM_SBC__
    SOUND_ITEM_DEF(CN_, RES_AUD_RING_SAMPRATE_16000),
#endif

#ifdef __INTERACTION__
    SOUND_ITEM_DEF(CN_, BT_FINDME),
#endif

};
#endif

#if MEDIA_SUPPORT_LANG & SUPPORT_ENGLISH
static const media_sound_map_t media_sound_map_en[] =
{
    SOUND_ITEM_DEF(EN_, SOUND_ZERO),
    SOUND_ITEM_DEF(EN_, SOUND_ONE),
    SOUND_ITEM_DEF(EN_, SOUND_TWO),
    SOUND_ITEM_DEF(EN_, SOUND_THREE),
    SOUND_ITEM_DEF(EN_, SOUND_FOUR),
    SOUND_ITEM_DEF(EN_, SOUND_FIVE),
    SOUND_ITEM_DEF(EN_, SOUND_SIX),
    SOUND_ITEM_DEF(EN_, SOUND_SEVEN),
    SOUND_ITEM_DEF(EN_, SOUND_EIGHT),
    SOUND_ITEM_DEF(EN_, SOUND_NINE),
#if defined ANC_ASSIST_ENABLED
    SOUND_ITEM_DEF(EN_, SOUND_PROMPT_ADAPTIVE_ANC),
    SOUND_ITEM_DEF(EN_, SOUND_CUSTOM_LEAK_DETECT),
#endif

    SOUND_ITEM_DEF(EN_, POWER_ON),
    SOUND_ITEM_DEF(EN_, POWER_OFF),
    SOUND_ITEM_DEF(EN_, BT_PAIR_ENABLE),
    SOUND_ITEM_DEF(EN_, BT_PAIRING),
    SOUND_ITEM_DEF(EN_, BT_PAIRING_SUCCESS),
    SOUND_ITEM_DEF(EN_, BT_PAIRING_FAIL),
    SOUND_ITEM_DEF(EN_, BT_REFUSE),
    SOUND_ITEM_DEF(EN_, BT_OVER),
    SOUND_ITEM_DEF(EN_, BT_ANSWER),
    SOUND_ITEM_DEF(EN_, BT_HUNG_UP),
    SOUND_ITEM_DEF(EN_, BT_INCOMING_CALL),
    SOUND_ITEM_DEF(EN_, CHARGE_PLEASE),
    SOUND_ITEM_DEF(EN_, CHARGE_FINISH),
    SOUND_ITEM_DEF(EN_, BT_CONNECTED),
    SOUND_ITEM_DEF(EN_, BT_DIS_CONNECT),
    SOUND_ITEM_DEF(EN_, BT_WARNING),
    SOUND_ITEM_DEF(EN_, BT_ALEXA_START),
    SOUND_ITEM_DEF(EN_, BT_ALEXA_STOP),

#if defined(BISTO_ENABLED) || defined(__AI_VOICE__)
    SOUND_ITEM_DEF(EN_, BT_GSOUND_MIC_OPEN),
    SOUND_ITEM_DEF(EN_, BT_GSOUND_MIC_CLOSE),
    SOUND_ITEM_DEF(EN_, BT_GSOUND_NC),
#endif

    SOUND_ITEM_DEF(EN_, LANGUAGE_SWITCH),
    SOUND_ITEM_DEF(EN_, BT_MUTE),
#ifdef __BT_WARNING_TONE_MERGE_INTO_STREAM_SBC__
    SOUND_ITEM_DEF(EN_, RES_AUD_RING_SAMPRATE_16000),
#endif

#ifdef __INTERACTION__
    SOUND_ITEM_DEF(EN_, BT_FINDME),
#endif

};

#endif

#endif /*PROMPT_IN_FLASH */

static char Media_player_number[MAX_PHB_NUMBER];

typedef struct tPlayContContext
{
    uint32_t g_play_continue_total; //total files
    uint32_t g_play_continue_n; //file index

    uint32_t g_play_continue_fread; //per file have readed

    uint8_t g_play_continue_array[MAX_PHB_NUMBER];

}_tPlayContContext;

static tPlayContContext pCont_context;

static int g_language = MEDIA_DEFAULT_LANGUAGE;
#ifdef AUDIO_LINEIN
static enum AUD_SAMPRATE_T app_play_audio_sample_rate = AUD_SAMPRATE_16000;
#endif

#define PROMPT_MIX_PROPERTY_PTR_FROM_ENTRY_INDEX(index)  \
    ((PROMPT_MIX_PROPERTY_T *)((uint32_t)__mixprompt_property_table_start + \
    (index)*sizeof(PROMPT_MIX_PROPERTY_T)))

#if defined(MEDIA_PLAYER_USE_CODEC2)
#undef app_audio_mempool_get_buff
#define app_audio_mempool_get_buff app_media_mempool_get_buff
#define MEDIA_PLAY_STREAM_ID (AUD_STREAM_ID_3)
#else
#define MEDIA_PLAY_STREAM_ID (AUD_STREAM_ID_0)
#endif

int media_audio_init(void)
{
#if !defined(PROMPT_USE_AAC)
    uint8_t *buff = NULL;
    app_audio_mempool_get_buff(&buff, SBC_QUEUE_SIZE);
    if (!buff) {
        TRACE(2,"buff is NULL");
        return -1;
    }
    memset(buff, 0, SBC_QUEUE_SIZE);

    LOCK_APP_AUDIO_QUEUE();
    APP_AUDIO_InitCQueue(&media_sbc_queue, SBC_QUEUE_SIZE, buff);
    UNLOCK_APP_AUDIO_QUEUE();

    app_audio_mempool_get_buff((uint8_t **)&media_sbc_decoder, sizeof(sbc_decoder_t) + 4);
#endif
    need_init_decoder = 1;

    app_play_sbc_stop_proc_cnt = 0;

    return 0;
}
static int decode_sbc_frame(unsigned char *pcm_buffer, unsigned int pcm_len)
{
    uint8_t underflow = 0;

    int r = 0;
    unsigned char *e1 = NULL, *e2 = NULL;
    unsigned int len1 = 0, len2 = 0;
    unsigned int peek_len = 0;

    int ret = SBC_OK;
    unsigned short byte_decode = 0;
    static pcm_frame_t pcm_data;
    pcm_data.pcm_data = (int16_t *)pcm_buffer;
    pcm_data.buffer_size = pcm_len;
    pcm_data.valid_size = 0;
    sbc_frame_t sbc_data;
    LOCK_APP_AUDIO_QUEUE();
again:
    if(need_init_decoder) {
        pcm_data.pcm_data = (int16_t *)pcm_buffer;
        pcm_data.valid_size = 0;
        sbc_decoder_open(media_sbc_decoder);

    }

get_again:
    len1 = len2 = 0;
    peek_len = MIN(SBC_TEMP_BUFFER_SIZE, APP_AUDIO_LengthOfCQueue(&media_sbc_queue));
    if (peek_len == 0) {
        need_init_decoder = 1;
        underflow = 1;
        r = pcm_data.valid_size;
        TRACE(0,"last chunk of prompt");
        goto exit;
    }

    r = APP_AUDIO_PeekCQueue(&media_sbc_queue, peek_len, &e1, &len1, &e2, &len2);
    ASSERT(r == CQ_OK, "[%s] peek queue should not failed", __FUNCTION__);

    if (!len1){
        TRACE(2,"len1 %d/%d\n", len1, len2);
        goto get_again;
    }

    sbc_data.sbc_data = e1;
    sbc_data.buffer_size = len1;
    sbc_data.valid_size = len1;

    ret = sbc_decoder_process_frame(media_sbc_decoder, &sbc_data, &pcm_data);
    byte_decode = sbc_data.buffer_size - sbc_data.valid_size;
    if(ret == BT_STS_CONTINUE) {
        need_init_decoder = 0;
        APP_AUDIO_DeCQueue(&media_sbc_queue, 0, len1);
        goto again;

        /* back again */
    }
    else if(ret == BT_STS_SUCCESS) {
        need_init_decoder = 0;
        r = pcm_data.valid_size;
        pcm_data.valid_size = 0;

        APP_AUDIO_DeCQueue(&media_sbc_queue, 0, byte_decode);

        //TRACE(1,"p %d\n", pcm_data.sampleFreq);

        /* leave */
    }
    else if(ret == BT_STS_FAILED) {
        need_init_decoder = 1;
        r = pcm_data.valid_size;
        TRACE(0,"err\n");

        APP_AUDIO_DeCQueue(&media_sbc_queue, 0, byte_decode);

        /* leave */
    }
    else if(ret == BT_STS_NO_RESOURCES) {
        need_init_decoder = 0;

        TRACE(0,"no\n");

        /* leav */
        r = 0;
    }

exit:
    if (underflow){
        TRACE(1,"media_sbc_decoder len:%d\n ", pcm_len);
    }
    UNLOCK_APP_AUDIO_QUEUE();
    return r;
}

static int app_media_store_sbc_buffer(uint8_t device_id, unsigned char *buf, unsigned int len)
{
    int nRet;
    LOCK_APP_AUDIO_QUEUE();
    nRet = APP_AUDIO_EnCQueue(&media_sbc_queue, buf, len);
    UNLOCK_APP_AUDIO_QUEUE();

    return nRet;
}

#if defined(IBRT)

#define PENDING_SYNC_PROMPT_BUFFER_CNT  8
// cleared when tws is disconnected
static uint16_t pendingSyncPromptId[PENDING_SYNC_PROMPT_BUFFER_CNT];
static uint8_t pending_sync_prompt_in_index = 0;
static uint8_t pending_sync_prompt_out_index = 0;
static uint8_t pending_sync_prompt_cnt = 0;

void app_tws_sync_prompt_manager_reset(void)
{
    pending_sync_prompt_in_index = 0;
    pending_sync_prompt_out_index = 0;
    pending_sync_prompt_cnt = 0;
}

void app_tws_sync_prompt_check(void)
{
    if (0 == pending_sync_prompt_cnt)
    {
        app_bt_active_mode_clear(BT_ACTIVE_MODE_KEEP_USER_SYNC_VOICE_PROMPT, UPDATE_ACTIVE_MODE_FOR_ALL_LINKS);
    }

    if (IBRT_ACTIVE_MODE != app_ibrt_if_get_bt_ctrl_ctx()->tws_mode)
    {
        return;
    }

    bool isPlayPendingPrompt = false;
    uint16_t promptIdToPlay = 0;

    uint32_t lock = int_lock();
    if (pending_sync_prompt_cnt > 0)
    {
        isPlayPendingPrompt = true;
        promptIdToPlay = pendingSyncPromptId[pending_sync_prompt_out_index];
        pending_sync_prompt_out_index++;
        if (PENDING_SYNC_PROMPT_BUFFER_CNT == pending_sync_prompt_out_index)
        {
            pending_sync_prompt_out_index = 0;
        }
        pending_sync_prompt_cnt--;
    }
    int_unlock(lock);

    if (isPlayPendingPrompt)
    {
        TRACE(1,"pop pending prompt 0x%x to play", promptIdToPlay);
        app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_START,
            BT_STREAM_MEDIA, 0, promptIdToPlay);
    }
}
#endif

void trigger_media_play(AUD_ID_ENUM id, uint8_t device_id, uint16_t aud_pram)
{
    uint16_t convertedId = (uint16_t)id;
    convertedId |= aud_pram;

#if (!defined(PROMPT_SELF_MANAGEMENT))    
    app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_START,BT_STREAM_MEDIA,device_id,convertedId);
#else
    app_audio_sendrequest(APP_PLAY_BACK_AUDIO, APP_BT_SETTING_START_PROMPT, convertedId);
#endif
}

void trigger_media_stop(AUD_ID_ENUM id, uint8_t device_id)
{
    /* Only the stop loop mode is supported */
    if (id == AUDIO_ID_FIND_MY_BUDS)
        app_play_sbc_stop_proc_cnt = 1;
}

#if defined(IBRT)
static bool isTriggerPromptLocal = false;
void trigger_prompt_local(bool isTriggerLocal)
{
    isTriggerPromptLocal = isTriggerLocal;
}

bool is_trigger_prompt_local(void)
{
    return isTriggerPromptLocal;
}

static bool is_prompt_playing_handling_locally(AUD_ID_ENUM promptId)
{
    if ((AUDIO_ID_BT_MUTE != promptId) && isTriggerPromptLocal)
    {
        return true;
    }
    switch ((uint16_t)PROMPT_ID_FROM_ID_VALUE(promptId))
    {
        case AUDIO_ID_FIND_MY_BUDS:
        case AUD_ID_POWER_OFF:
            return true;
        default:
            return false;
    }
}

static bool is_prompt_launched_only_by_slave(AUD_ID_ENUM promptId)
{
    switch ((uint16_t)PROMPT_ID_FROM_ID_VALUE(promptId))
    {
        case AUD_ID_BT_DIS_CONNECT:
            return true;
        default:
            return false;
    }
}
#endif

static bool media_PlayAudio_handler(AUD_ID_ENUM id,uint8_t device_id, bool isLocalPlaying)
{
    uint16_t aud_pram = PROMOT_ID_BIT_MASK_MERGING|PROMOT_ID_BIT_MASK_CHNLSEl_ALL;
    if (isLocalPlaying)
    {
        aud_pram |= PROMOT_ID_BIT_MASK_LOCAL_PLAYING;
    }

    trigger_media_play(id, device_id, aud_pram);

    return true;
}

bool media_playAudio_is_continuous_prompt_on_going(void)
{
    return g_is_continuous_prompt_on_going;
}

void media_playAudio_set_continous_prompt_env(AUD_ID_ENUM id)
{
    g_is_continuous_prompt_on_going = true;
    continuous_prompt_id = id;
}

void media_PlayAudio_continuous_start(AUD_ID_ENUM id, uint8_t device_id)
{
    if (g_is_continuous_prompt_on_going)
    {
        TRACE(0, "A continuous prompt is already on-going!");
        return;
    }

    bool isLocalPlaying = true;

#ifdef IBRT
    isLocalPlaying = is_prompt_playing_handling_locally(id);

    if (app_tws_ibrt_tws_link_connected()&&(!isLocalPlaying))
    {
        voice_report_role_t report_role = app_ibrt_voice_report_get_role();
        if (VOICE_REPORT_SLAVE == report_role)
        {
            app_tws_let_peer_device_play_audio_prompt((uint16_t)id, APP_PROMPT_CONTINUOUS_NORMAL_PLAY, device_id);
            return;
        }
    }
#endif

    media_playAudio_set_continous_prompt_env(id);
    app_prompt_push_request(APP_PROMPT_CONTINUOUS_NORMAL_PLAY, id, device_id, isLocalPlaying);
#if defined(IBRT)
    trigger_prompt_local(false);
#endif
}

static bool media_PlayAudio_stop_continuous_prompt(void)
{
    TRACE(0, "stop_continuous_prompt:%d", g_is_continuous_prompt_on_going);
    if (g_is_continuous_prompt_on_going)
    {
        g_is_continuous_prompt_on_going = false;
        continuous_prompt_id = AUD_ID_INVALID;
        app_prompt_stop_continuous_checker();

        app_prompt_stop_all();

        return true;
    }

    return false;
}

void media_PlayAudio_continuous_end(AUD_ID_ENUM id, uint8_t device_id)
{
    if (!media_PlayAudio_stop_continuous_prompt())
    {
        app_tws_stop_peer_prompt();
    }
}

static bool media_playAudio_pre_processing(AUD_ID_ENUM id, uint8_t device_id, media_PlayAudio_api func)
{
    if (app_prompt_is_on_going && (AUDIO_ID_BT_MUTE != id))
    {
#ifdef IBRT
        if (app_tws_ibrt_tws_link_connected())
        {
            voice_report_role_t report_role =  app_ibrt_voice_report_get_role();
            if (VOICE_REPORT_SLAVE == report_role)
            {
                // for voice report slave role, always wait for the former prompt
                // to be completed before processing the new prompt.
                // otherwise following issue may happen:
                // 1. slave role starts a local prompt playing
                // 2. slave role tries to start a sync prompt playing
                // 3. slave role sends request to master role, asking the master to
                //    trigger sync prompt playing
                // 4. master role has no on-going prompt, so it will send prompt trigger
                //    request to slave role
                // 5. slave role processes the prompt trigger request but finds that an on-going prompt
                //    exist, so this new prompt trigger request is missing
                APP_PROMPT_PLAY_REQ_T req;
                req.reqType = APP_PROMPT_CACHED_ORG_REQUEST;
                req.deviceId = device_id;
                req.promptId = (uint32_t)id;
                req.func = func;
                
                TRACE(0, "push rq prompt request devId %d promptId 0x%x func 0x%x",
                    id, device_id, (uint32_t)func);
                
                bool ret = app_prompt_list_append(app_prompt_list, &req);
                if (ret)
                {
#if defined(USE_BASIC_THREADS)
                    app_prompt_inform_new_prompt_event();
#else
                    osSignalSet(app_prompt_handler_tid, PROMPT_HANDLER_SIGNAL_NEW_PROMPT_REQ);
#endif
                    // do not handle but push it into pending queue
                    return false;
                }
            }
        }
#endif /* IBRT */
    }

    return true;
}

#ifdef IBRT
static void media_playAudio_forware_playing_request_to_peer_dev(AUD_ID_ENUM id)
{
    TRACE(0, "Let peer bud play prompt 0x%x", id);
    tws_ctrl_send_cmd(APP_TWS_CMD_FORWARD_PROMPT_PLAYING_REQ, \
                   (uint8_t *)&id, \
                   sizeof(id));
}
#endif

void media_PlayAudio(AUD_ID_ENUM id,uint8_t device_id)
{
    bool isToBeProcessed = media_playAudio_pre_processing(id, device_id, media_PlayAudio);
    if (!isToBeProcessed)
    {
        return;
    }

    bool isLocalPlaying = true;

#ifdef IBRT
    isLocalPlaying = is_prompt_playing_handling_locally(id);

    if (app_tws_ibrt_tws_link_connected()&&(!isLocalPlaying))
    {
        voice_report_role_t report_role =  app_ibrt_voice_report_get_role();
        if (VOICE_REPORT_SLAVE == report_role)
        {
            if (!app_tws_let_peer_device_play_audio_prompt((uint16_t)id, APP_PROMPT_NORMAL_PLAY, device_id))
            {
                goto prompt_local;
            }
            else
            {
                // push this request to assure that following prompt request will be in pending queue,
                // until the sync prompt play handling is completed
                app_prompt_push_request(APP_PROMPT_PENDING_FOR_MASTER_SYNC_PLAY_REQ, id, device_id, isLocalPlaying);
            }
            return;
        }
        else if (VOICE_REPORT_MASTER == report_role && false == is_prompt_launched_only_by_slave(id))
        {
            if ((!app_prompt_is_on_going) && (AUDIO_ID_BT_MUTE != id))
            {
                media_playAudio_forware_playing_request_to_peer_dev(id);
                return;
            }
        }
    }

prompt_local:
#endif

    app_prompt_push_request(APP_PROMPT_NORMAL_PLAY, id, device_id, isLocalPlaying);
#if defined(IBRT)
    trigger_prompt_local(false);
#endif
}

static bool media_PlayAudio_standalone_handler(AUD_ID_ENUM id, uint8_t device_id, bool isLocalPlaying)
{
    uint16_t aud_pram = PROMOT_ID_BIT_MASK_CHNLSEl_ALL;
    if (isLocalPlaying)
    {
        aud_pram |= PROMOT_ID_BIT_MASK_LOCAL_PLAYING;
    }

    trigger_media_play(id, device_id, aud_pram);
    return true;
}

void media_PlayAudio_standalone(AUD_ID_ENUM id, uint8_t device_id)
{
    bool isToBeProcessed = media_playAudio_pre_processing(id, device_id, media_PlayAudio_standalone);
    if (!isToBeProcessed)
    {
        return;
    }
    if (AUDIO_ID_BT_MUTE == id)
    {
        app_start_post_chopping_timer();
    }

    bool isLocalPlaying = true;

#ifdef IBRT
    isLocalPlaying = is_prompt_playing_handling_locally(id);

    if (app_tws_ibrt_tws_link_connected()&&(!isLocalPlaying))
    {
        voice_report_role_t report_role =  app_ibrt_voice_report_get_role();
        if (VOICE_REPORT_SLAVE == report_role)
        {
            if (!app_tws_let_peer_device_play_audio_prompt((uint16_t)id, APP_PROMPT_STANDALONE_PLAY, device_id)) {
                goto prompt_local;
            }
            return;
        }
    }
    
prompt_local:
#endif

    app_prompt_push_request(APP_PROMPT_STANDALONE_PLAY, id, device_id, isLocalPlaying);
#if defined(IBRT)
    trigger_prompt_local(false);
#endif
}


bool media_playAudio_locally_is_continuous_prompt_on_going(void)
{
    return g_is_local_continuous_prompt_on_going;
}

void media_PlayAudio_locally_continuous_start(AUD_ID_ENUM id, uint8_t device_id)
{
    bool isToBeProcessed = media_playAudio_pre_processing(id, device_id, media_PlayAudio_locally_continuous_start);
    if (!isToBeProcessed)
    {
        return;
    }
    if (g_is_local_continuous_prompt_on_going)
    {
        TRACE(0, "A local continuous prompt is already on-going!");
        return;
    }


    g_is_local_continuous_prompt_on_going = true;
    continuous_prompt_id = id;
    app_prompt_push_request(APP_PROMPT_LOCAL_CONTINUOUS_NORMAL_PLAY, id, device_id, true);
}

static void media_PlayAudio_stop_local_continuous_prompt(void)
{
    if (g_is_local_continuous_prompt_on_going)
    {
        g_is_local_continuous_prompt_on_going = false;
        continuous_prompt_id = AUD_ID_INVALID;
        app_prompt_stop_all();
    }
}

void media_PlayAudio_locally_continuous_end(AUD_ID_ENUM id, uint8_t device_id)
{
    media_PlayAudio_stop_local_continuous_prompt();
}

static bool media_PlayAudio_locally_handler(AUD_ID_ENUM id, uint8_t device_id, bool isLocalPlaying)
{
    uint16_t aud_param;
#if defined(IBRT)
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    if (A2DP_AUDIO_CHANNEL_SELECT_LCHNL == p_ibrt_ctrl->audio_chnl_sel)
    {
        aud_param = PROMOT_ID_BIT_MASK_MERGING|PROMOT_ID_BIT_MASK_CHNLSEl_LCHNL;
    }
    else
    {
        aud_param = PROMOT_ID_BIT_MASK_MERGING|PROMOT_ID_BIT_MASK_CHNLSEl_RCHNL;
    }
#else
    aud_param = PROMOT_ID_BIT_MASK_MERGING|PROMOT_ID_BIT_MASK_CHNLSEl_ALL;
#endif

    if (isLocalPlaying)
    {
        aud_param |= PROMOT_ID_BIT_MASK_LOCAL_PLAYING;
    }

    trigger_media_play(id, device_id, aud_param);
    return true;
}

void media_PlayAudio_locally(AUD_ID_ENUM id, uint8_t device_id)
{
    bool isToBeProcessed = media_playAudio_pre_processing(id, device_id, media_PlayAudio_locally);
    if (!isToBeProcessed)
    {
        return;
    }

    bool isLocalPlaying = true;

#ifdef IBRT
    isLocalPlaying = is_prompt_playing_handling_locally(id);

    if (app_tws_ibrt_tws_link_connected()&&(!isLocalPlaying))
    {
        voice_report_role_t report_role =  app_ibrt_voice_report_get_role();
        if (VOICE_REPORT_SLAVE == report_role)
        {
            app_tws_let_peer_device_play_audio_prompt((uint16_t)id, APP_PROMPT_LOCAL_PLAY, device_id);
            return;
        }
    }
#endif

    app_prompt_push_request(APP_PROMPT_LOCAL_PLAY, id, device_id, isLocalPlaying);
#if defined(IBRT)
    trigger_prompt_local(false);
#endif
}

static bool media_PlayAudio_standalone_locally_handler(AUD_ID_ENUM id, uint8_t device_id, bool isLocalPlaying)
{
    uint16_t aud_param;
#if defined(IBRT)
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    if (A2DP_AUDIO_CHANNEL_SELECT_LCHNL == p_ibrt_ctrl->audio_chnl_sel)
    {
        aud_param = PROMOT_ID_BIT_MASK_CHNLSEl_LCHNL;
    }
    else
    {
        aud_param = PROMOT_ID_BIT_MASK_CHNLSEl_RCHNL;
    }
#else
    aud_param = PROMOT_ID_BIT_MASK_CHNLSEl_ALL;
#endif

    if (isLocalPlaying)
    {
        aud_param |= PROMOT_ID_BIT_MASK_LOCAL_PLAYING;
    }

    trigger_media_play(id, device_id, aud_param);
    return true;
}

void media_PlayAudio_standalone_locally(AUD_ID_ENUM id, uint8_t device_id)
{
    bool isToBeProcessed = media_playAudio_pre_processing(id, device_id, media_PlayAudio_standalone_locally);
    if (!isToBeProcessed)
    {
        return;
    }
    bool isLocalPlaying = true;

#ifdef IBRT
    isLocalPlaying = is_prompt_playing_handling_locally(id);

    if (app_tws_ibrt_tws_link_connected()&&(!isLocalPlaying))
    {
        voice_report_role_t report_role =  app_ibrt_voice_report_get_role();
        if (VOICE_REPORT_SLAVE == report_role)
        {
            app_tws_let_peer_device_play_audio_prompt((uint16_t)id, APP_PROMPT_STANDALONE_LOCAL_PLAY, device_id);
            return;
        }
    }
#endif

    app_prompt_push_request(APP_PROMPT_STANDALONE_LOCAL_PLAY, id, device_id, isLocalPlaying);
#if defined(IBRT)
    trigger_prompt_local(false);
#endif
}

static bool media_PlayAudio_remotely_handler(AUD_ID_ENUM id, uint8_t device_id, bool isLocalPlaying)
{
    uint16_t aud_param;

#if defined(IBRT)
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    if (A2DP_AUDIO_CHANNEL_SELECT_LCHNL == p_ibrt_ctrl->audio_chnl_sel)
    {
        aud_param = PROMOT_ID_BIT_MASK_MERGING|PROMOT_ID_BIT_MASK_CHNLSEl_RCHNL;
    }
    else
    {
        aud_param = PROMOT_ID_BIT_MASK_MERGING|PROMOT_ID_BIT_MASK_CHNLSEl_LCHNL;
    }
#else
    aud_param = PROMOT_ID_BIT_MASK_MERGING|PROMOT_ID_BIT_MASK_CHNLSEl_ALL;
#endif

    if (isLocalPlaying)
    {
        aud_param |= PROMOT_ID_BIT_MASK_LOCAL_PLAYING;
    }

    trigger_media_play(id, device_id, aud_param);
    return true;
}

void media_PlayAudio_remotely(AUD_ID_ENUM id, uint8_t device_id)
{
    bool isToBeProcessed = media_playAudio_pre_processing(id, device_id, media_PlayAudio_remotely);
    if (!isToBeProcessed)
    {
        return;
    }
    bool isLocalPlaying = true;

#ifdef IBRT
    isLocalPlaying = is_prompt_playing_handling_locally(id);

    if (app_tws_ibrt_tws_link_connected()&&(!isLocalPlaying))
    {
        voice_report_role_t report_role =  app_ibrt_voice_report_get_role();
        if (VOICE_REPORT_SLAVE == report_role)
        {
            app_tws_let_peer_device_play_audio_prompt((uint16_t)id, APP_PROMPT_REMOTE_PLAY, device_id);
            return;
        }
    }
#endif

    app_prompt_push_request(APP_PROMPT_REMOTE_PLAY, id, device_id, isLocalPlaying);
#if defined(IBRT)
    trigger_prompt_local(false);
#endif
}

static bool media_PlayAudio_standalone_remotely_handler(AUD_ID_ENUM id, uint8_t device_id, bool isLocalPlaying)
{
    uint16_t aud_param;

#if defined(IBRT)
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    if (A2DP_AUDIO_CHANNEL_SELECT_LCHNL == p_ibrt_ctrl->audio_chnl_sel)
    {
        aud_param = PROMOT_ID_BIT_MASK_CHNLSEl_RCHNL;
    }
    else
    {
        aud_param = PROMOT_ID_BIT_MASK_CHNLSEl_LCHNL;
    }
#else
    aud_param = PROMOT_ID_BIT_MASK_CHNLSEl_ALL;
#endif

    if (isLocalPlaying)
    {
        aud_param |= PROMOT_ID_BIT_MASK_LOCAL_PLAYING;
    }

    trigger_media_play(id, device_id, aud_param);
    return true;
}

void media_PlayAudio_standalone_remotely(AUD_ID_ENUM id, uint8_t device_id)
{
    bool isToBeProcessed = media_playAudio_pre_processing(id, device_id, media_PlayAudio_standalone_remotely);
    if (!isToBeProcessed)
    {
        return;
    }
    bool isLocalPlaying = true;

#ifdef IBRT
    isLocalPlaying = is_prompt_playing_handling_locally(id);

    if (app_tws_ibrt_tws_link_connected()&&(!isLocalPlaying))
    {
        voice_report_role_t report_role =  app_ibrt_voice_report_get_role();
        if (VOICE_REPORT_SLAVE == report_role)
        {
            app_tws_let_peer_device_play_audio_prompt((uint16_t)id, APP_PROMPT_STANDALONE_REMOTE_PLAY, device_id);
            return;
        }
    }
#endif

    app_prompt_push_request(APP_PROMPT_STANDALONE_REMOTE_PLAY, id, device_id, isLocalPlaying);
#if defined(IBRT)
    trigger_prompt_local(false);
#endif
}

AUD_ID_ENUM app_get_current_standalone_promptId(void)
{
    return AUD_ID_INVALID;
}

AUD_ID_ENUM media_GetCurrentPrompt(uint8_t device_id)
{
    AUD_ID_ENUM currentPromptId = AUD_ID_INVALID;
    if (app_bt_stream_isrun(APP_PLAY_BACK_AUDIO))
    {
        currentPromptId = app_get_current_standalone_promptId();
    }
#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
    else if (audio_prompt_is_playing_ongoing())
    {
        currentPromptId = (AUD_ID_ENUM)audio_prompt_get_prompt_id();
    }
#endif
    return currentPromptId;
}

#define IsDigit(c) (((c)>='0')&&((c)<='9'))
void media_Set_IncomingNumber(const char* pNumber)
{
    char *p_num = Media_player_number;
    uint8_t cnt = 0;
    for(uint8_t idx = 0; idx < MAX_PHB_NUMBER; idx++) {
        if(*(pNumber + idx) == 0)
            break;

        if(IsDigit(*(pNumber + idx))) {
            *(p_num + cnt) = *(pNumber + idx);
            TRACE(2,"media_Set_IncomingNumber: cnt %d ,p_num  %d", cnt, *(p_num + cnt));
            cnt ++;
        }
    }
}

PROMPT_MIX_PROPERTY_T* get_prompt_mix_property(uint16_t promptId)
{
    for (uint32_t index = 0;
         index < ((uint32_t)__mixprompt_property_table_end -
         (uint32_t)__mixprompt_property_table_start) / sizeof(PROMPT_MIX_PROPERTY_T);
         index++)
    {
        PROMPT_MIX_PROPERTY_T *property;
        property = PROMPT_MIX_PROPERTY_PTR_FROM_ENTRY_INDEX(index);
        if (property->promptId == promptId)
        {
            return property;
        }
    }

    return NULL;
}
/*
Reference information for how to pass
parameters into PROMPT_MIX_PROPERTY_TO_ADD:

PROMPT_MIX_PROPERTY_TO_ADD(
promptId,
volume_level_override,
coeff_for_mix_prompt_for_music,
coeff_for_mix_music_for_music,
coeff_for_mix_prompt_for_call,
coeff_for_mix_call_for_call)
*/

#ifndef PROMPT_IN_FLASH
static int get_sound_id_info(enum sound_id id, uint8_t **sound_data, uint32_t *length)
{
    const media_sound_map_t *map;
    if (id >= MAX_SOUND_ID)
        ASSERT(0, "wrong sound id %d", id);

    map = get_sound_media_map(g_language);
    *sound_data = (map + id)->data;
    *length = (map + id)->fsize;

    return 0;
}
#endif

void media_runtime_audio_prompt_update(uint16_t id, uint8_t** ptr, uint32_t* len)
{
#if defined(PROMPT_CTRL_SWITCH_ANC_MODE)
    uint16_t app_prompt_ctrl_anc_init(uint16_t aud_id);
    id = app_prompt_ctrl_anc_init(id);
#endif

#ifdef PROMPT_IN_FLASH
    nv_record_prompt_get_prompt_info(g_language, id, &g_app_audio_data, &g_app_audio_length);
    *ptr = g_app_audio_data;
    *len = g_app_audio_length;
#else
    uint8_t *sound_data = NULL;
    uint32_t length = 0;

    switch (id) {
    case AUD_ID_POWER_ON:
        get_sound_id_info(POWER_ON, &sound_data, &length);
        break;
    case AUD_ID_POWER_OFF:
        get_sound_id_info(POWER_OFF, &sound_data, &length);
        break;
    case AUD_ID_BT_PAIR_ENABLE:
        get_sound_id_info(BT_PAIR_ENABLE, &sound_data, &length);
        break;
    case AUD_ID_BT_PAIRING:
        get_sound_id_info(BT_PAIRING, &sound_data, &length);
        break;
    case AUD_ID_BT_PAIRING_SUC:
        get_sound_id_info(BT_PAIRING_SUCCESS, &sound_data, &length);
        break;
    case AUD_ID_BT_PAIRING_FAIL:
        get_sound_id_info(BT_PAIRING_FAIL, &sound_data, &length);
        break;
    case AUD_ID_BT_CALL_REFUSE:
        get_sound_id_info(BT_REFUSE, &sound_data, &length);
        break;
    case AUD_ID_BT_CALL_OVER:
        get_sound_id_info(BT_OVER, &sound_data, &length);
        break;
    case AUD_ID_BT_CALL_ANSWER:
        get_sound_id_info(BT_ANSWER, &sound_data, &length);
        break;
    case AUD_ID_BT_CALL_HUNG_UP:
        get_sound_id_info(BT_HUNG_UP, &sound_data, &length);
        break;
    case AUD_ID_BT_CALL_INCOMING_CALL:
#if BLE_AUDIO_ENABLED
    case AUD_ID_LE_AUD_INCOMING_CALL:
#endif
        get_sound_id_info(BT_INCOMING_CALL, &sound_data, &length);
        break;
    case AUD_ID_BT_CHARGE_PLEASE:
        get_sound_id_info(CHARGE_PLEASE, &sound_data, &length);
        break;
    case AUD_ID_BT_CHARGE_FINISH:
        get_sound_id_info(CHARGE_FINISH, &sound_data, &length);
        break;
    case AUD_ID_BT_CONNECTED:
        get_sound_id_info(BT_CONNECTED, &sound_data, &length);
        break;
    case AUD_ID_BT_DIS_CONNECT:
        get_sound_id_info(BT_DIS_CONNECT, &sound_data, &length);
        break;
    case AUD_ID_BT_WARNING:
        get_sound_id_info(BT_WARNING, &sound_data, &length);
        break;
    case AUDIO_ID_BT_ALEXA_START:
        get_sound_id_info(BT_ALEXA_START, &sound_data, &length);
        break;
    case AUDIO_ID_BT_ALEXA_STOP:
    case AUDIO_ID_FIND_MY_BUDS:
    case AUDIO_ID_FIND_TILE:
        get_sound_id_info(BT_ALEXA_STOP, &sound_data, &length);
        break;
#if defined(BISTO_ENABLED) || defined(__AI_VOICE__)
    case AUDIO_ID_BT_GSOUND_MIC_OPEN:
        get_sound_id_info(BT_GSOUND_MIC_OPEN, &sound_data, &length);
        break;
    case AUDIO_ID_BT_GSOUND_MIC_CLOSE:
        get_sound_id_info(BT_GSOUND_MIC_CLOSE, &sound_data, &length);
        break;
    case AUDIO_ID_BT_GSOUND_NC:
        get_sound_id_info(BT_GSOUND_NC, &sound_data, &length);
        break;
#endif
    case AUD_ID_LANGUAGE_SWITCH:
        get_sound_id_info(LANGUAGE_SWITCH, &sound_data, &length);
        break;
    case AUDIO_ID_BT_MUTE:
        get_sound_id_info(BT_MUTE, &sound_data, &length);
        break;
    case AUD_ID_NUM_0:
        get_sound_id_info(SOUND_ZERO, &sound_data, &length);
        break;
    case AUD_ID_NUM_1:
        get_sound_id_info(SOUND_ONE, &sound_data, &length);
        break;
    case AUD_ID_NUM_2:
        get_sound_id_info(SOUND_TWO, &sound_data, &length);
        break;
    case AUD_ID_NUM_3:
        get_sound_id_info(SOUND_THREE, &sound_data, &length);
        break;
    case AUD_ID_NUM_4:
        get_sound_id_info(SOUND_FOUR, &sound_data, &length);
        break;
    case AUD_ID_NUM_5:
        get_sound_id_info(SOUND_FIVE, &sound_data, &length);
        break;
    case AUD_ID_NUM_6:
        get_sound_id_info(SOUND_SIX, &sound_data, &length);
        break;
    case AUD_ID_NUM_7:
        get_sound_id_info(SOUND_SEVEN, &sound_data, &length);
        break;
    case AUD_ID_NUM_8:
        get_sound_id_info(SOUND_EIGHT, &sound_data, &length);
        break;
    case AUD_ID_NUM_9:
        get_sound_id_info(SOUND_NINE, &sound_data, &length);
        break;
#if defined ANC_ASSIST_ENABLED
    case AUD_ID_ANC_PROMPT:
        get_sound_id_info(SOUND_PROMPT_ADAPTIVE_ANC, &sound_data, &length);
        break;
    case AUD_ID_CUSTOM_LEAK_DETECT:
        get_sound_id_info(SOUND_CUSTOM_LEAK_DETECT, &sound_data, &length);
        break;
#endif
#ifdef __BT_WARNING_TONE_MERGE_INTO_STREAM_SBC__
    case AUD_ID_RING_WARNING:
        get_sound_id_info(RES_AUD_RING_SAMPRATE_16000, &sound_data, &length);
        break;
#endif
#ifdef __INTERACTION__
    case AUD_ID_BT_FINDME:
        get_sound_id_info(BT_FINDME, &sound_data, &length);
        break;
#endif
    default:
        g_app_audio_length = 0;
        break;
    }

    *ptr = g_app_audio_data = sound_data;
    *len = g_app_audio_length = length;
#endif
}

static const media_sound_map_t * get_sound_media_map(int language)
{
    const media_sound_map_t * map = NULL;
#ifdef PROMPT_IN_FLASH
    for (uint8_t promptId = AUD_ID_NUM_0; promptId <= AUD_ID_NUM_9; promptId++)
    {
        nv_record_prompt_get_prompt_info(language, promptId,
                                         &media_number_sound_map[promptId - AUD_ID_NUM_0].data,
                                         &media_number_sound_map[promptId - AUD_ID_NUM_0].fsize);
    }

    map = media_number_sound_map;
#else
    if (language == MEDIA_DEFAULT_LANGUAGE)
    {
#if MEDIA_SUPPORT_LANG & SUPPORT_ENGLISH
        map = media_sound_map_en;
#endif
    }
    else
    {
#if MEDIA_SUPPORT_LANG & SUPPORT_CHINESE
        map = media_sound_map_cn;
#endif
    }

#endif
    ASSERT(map != NULL, "media map invalide");
    return map;
}

static void media_Play_init_audio(uint16_t aud_id)
{
    if (aud_id == AUD_ID_BT_CALL_INCOMING_NUMBER)
    {
        g_play_continue_mark = 1;
        media_sound_map = get_sound_media_map(g_language);

        memset(&pCont_context, 0x0, sizeof(pCont_context));

        pCont_context.g_play_continue_total = strlen((const char *)Media_player_number);

        for (uint32_t i = 0; (i < pCont_context.g_play_continue_total) && (i < MAX_PHB_NUMBER); i++)
        {
            pCont_context.g_play_continue_array[i] = Media_player_number[i] - '0';

            TRACE(3, "media_PlayNumber, pCont_context.g_play_continue_array[%d] = %d, total =%d",
                  i, pCont_context.g_play_continue_array[i], pCont_context.g_play_continue_total);
        }
    }
    else
    {
        g_app_audio_read = 0;
        g_play_continue_mark = 0;

        media_runtime_audio_prompt_update(aud_id, &g_app_audio_data, &g_app_audio_length);
#if defined(PROMPT_USE_AAC)
        g_app_audio_read = g_app_audio_length;
#endif
    }
}

uint32_t app_play_sbc_more_data_fadeout(int16_t *buf, uint32_t len)
{
    uint32_t i;
    uint32_t j = 0;

    for (i = len; i > 0; i--){
        *(buf+j) = *(buf+j)*i/len;
        j++;
    }

    return len;
}

static uint32_t need_fadein_len = 0;
static uint32_t need_fadein_len_processed = 0;

int app_play_sbc_more_data_fadein_config(uint32_t len)
{
    TRACE(1,"fadein_config l:%d", len);
    need_fadein_len = len;
    need_fadein_len_processed = 0;
    return 0;
}
uint32_t app_play_sbc_more_data_fadein(int16_t *buf, uint32_t len)
{
    uint32_t i;
    uint32_t j = 0;
    uint32_t base;
    uint32_t dest;

    base = need_fadein_len_processed;
    dest = need_fadein_len_processed + len < need_fadein_len ?
           need_fadein_len_processed + len :
           need_fadein_len_processed;

    if (base >= dest){
//        TRACE(0,"skip fadein");
        return len;
    }
//    TRACE(3,"fadein l:%d base:%d dest:%d", len, base, dest);
//    DUMP16("%5d ", buf, 20);
//    DUMP16("%5d ", buf+len-19, 20);

    for (i = base; i < dest; i++){
        *(buf+j) = *(buf+j)*i/need_fadein_len;
        j++;
    }

    need_fadein_len_processed += j;
//    DUMP16("%05d ", buf, 20);
//    DUMP16("%5d ", buf+len-19, 20);
    return len;
}

static uint32_t app_play_single_sbc_more_data(uint8_t device_id, uint8_t *buf, uint32_t len)
{
    //int32_t stime, etime;
    //U16 byte_decode;
#if defined(PROMPT_USE_AAC)
    aac_frame_t aac_data;
    aac_data.aac_data = g_app_audio_data;
    aac_data.buffer_size = g_app_audio_length;
    aac_data.valid_size = g_app_audio_read;
    aac_pcm_frame_t pcm_data;
    pcm_data.pcm_data = (short*)buf;
    pcm_data.buffer_size = len;
    pcm_data.valid_size = 0;
    /* decode one AAC frame */
    int decoder_err = aac_decoder_process_frame(aacDec_handle, &aac_data, &pcm_data);
    g_app_audio_read = aac_data.valid_size;
    if (decoder_err != AAC_OK){
        if(decoder_err == AAC_NOT_ENOUGH_BITS)
            TRACE(0,"aac_lc_decode, not enough bits");
        else
            TRACE(0,"aac_lc_decode failed:0x%x", decoder_err);
        return 0;
    }

    uint32_t free_bytes = 0;
    bool ret = aac_decoder_get(aacDec_handle, AAC_VALID_BYTES, &free_bytes);
    //decoder_err = aacDecoder_GetFreeBytes(aacDec_handle,&free_bytes);
    if (ret == false){
        TRACE(0,"decoder_GetFreeBytes failed");
        return 0;
    }
    //TRACE(2,"app_play_sbc_more_data2 : %d, %d %d", g_app_audio_length, g_app_audio_read,free_bytes);
    return len;
#else
    uint32_t l = 0;

    //TRACE(2,"app_play_sbc_more_data : %d, %d", g_app_audio_read, g_app_audio_length);

    if (g_app_audio_read < g_app_audio_length){
        unsigned int available_len = 0;
        unsigned int store_len = 0;

        available_len = AvailableOfCQueue(&media_sbc_queue);
        store_len = MIN((g_app_audio_length - g_app_audio_read), available_len);
        app_media_store_sbc_buffer(device_id, (unsigned char *)(g_app_audio_data + g_app_audio_read), store_len);
        g_app_audio_read += store_len;
    }


    l = decode_sbc_frame(buf, len);

    if (l != len)
    {
        g_app_audio_read = g_app_audio_length;
        //af_stream_stop(AUD_STREAM_PLAYBACK);
        //af_stream_close(AUD_STREAM_PLAYBACK);
        TRACE(4,"[%s]-->need close, length:%d len:%d l:%d", __func__,g_app_audio_length, len, l);
    }

    return l;
#endif
}


/* play continue sound */
static uint32_t app_play_continue_sbc_more_data(uint8_t device_id, uint8_t *buf, uint32_t len)
{

    uint32_t l, n, fsize = 0;

    uint8_t*  pdata;

// store data
    unsigned int available_len = 0;
    unsigned int store_len = 0;

    if (pCont_context.g_play_continue_n < pCont_context.g_play_continue_total){
        do {
            n = pCont_context.g_play_continue_n;
            pdata = media_sound_map[pCont_context.g_play_continue_array[n]].data;
            fsize = media_sound_map[pCont_context.g_play_continue_array[n]].fsize;

            available_len = AvailableOfCQueue(&media_sbc_queue);
            if (!available_len)
                break;

            store_len = MIN((fsize - pCont_context.g_play_continue_fread), available_len);
            app_media_store_sbc_buffer(device_id, (unsigned char *)(pdata + pCont_context.g_play_continue_fread), store_len);
            pCont_context.g_play_continue_fread += store_len;
            if (pCont_context.g_play_continue_fread == fsize){
                pCont_context.g_play_continue_n++;
                pCont_context.g_play_continue_fread = 0;
            }
        }while(pCont_context.g_play_continue_n < pCont_context.g_play_continue_total);
    }

    l = decode_sbc_frame(buf, len);

    if (l !=len){
        TRACE(0,"continue sbc decode ---> APP_BT_SETTING_CLOSE");
    }

    return l;
}

uint32_t app_play_sbc_channel_select(int16_t *dst_buf, int16_t *src_buf, uint32_t src_len)
{
#if defined(IBRT)
    if (IS_PROMPT_CHNLSEl_ALL(g_prompt_chnlsel) || app_ibrt_voice_report_is_me(PROMPT_CHNLSEl_FROM_ID_VALUE(g_prompt_chnlsel)))
    {
        // Copy from tail so that it works even if dst_buf == src_buf
        for (int i = (int)(src_len - 1); i >= 0; i--)
        {
            dst_buf[i*2 + 0] = dst_buf[i*2 + 1] = src_buf[i];
        }
    }
    else
    {
        // Copy from tail so that it works even if dst_buf == src_buf
        for (int i = (int)(src_len - 1); i >= 0; i--)
        {
            dst_buf[i*2 + 0] = dst_buf[i*2 + 1] = 0;
        }
    }
#else
    if (IS_PROMPT_CHNLSEl_ALL(g_prompt_chnlsel))
    {
        // Copy from tail so that it works even if dst_buf == src_buf
        for (int i = (int)(src_len - 1); i >= 0; i--)
        {
            dst_buf[i*2 + 0] = dst_buf[i*2 + 1] = src_buf[i];
        }
    }
    else if (IS_PROMPT_CHNLSEl_LCHNL(g_prompt_chnlsel))
    {
        // Copy from tail so that it works even if dst_buf == src_buf
        for (int i = (int)(src_len - 1); i >= 0; i--)
        {
            dst_buf[i*2 + 0] = src_buf[i];
            dst_buf[i*2 + 1] = 0;
        }
    }
    else if (IS_PROMPT_CHNLSEl_RCHNL(g_prompt_chnlsel))
    {
        // Copy from tail so that it works even if dst_buf == src_buf
        for (int i = (int)(src_len - 1); i >= 0; i--)
        {
            dst_buf[i*2 + 0] = 0;
            dst_buf[i*2 + 1] = src_buf[i];
        }
    }
    else
    {
        // Copy from tail so that it works even if dst_buf == src_buf
        for (int i = (int)(src_len - 1); i >= 0; i--)
        {
            dst_buf[i*2 + 0] = dst_buf[i*2 + 1] = 0;
        }
    }
#endif
    return 0;
}

#if defined(__BT_ANC__) || defined(PSAP_FORCE_STREAM_48K)
static uint32_t g_prompt_upsampling_ratio = 3;
extern void us_fir_init(uint32_t upsampling_ratio);
extern uint32_t us_fir_run(short *src_buf, short *dst_buf, uint32_t in_samp_num);
#endif

uint32_t g_cache_buff_sz = 0;

#if defined(PROMPT_CTRL_SWITCH_ANC_MODE)
#include "app_anc.h"
#include "app_anc_utils.h"
static app_anc_mode_t g_anc_mode    = APP_ANC_MODE_QTY;

#define PROMOT_ID_ANC_MASK                      (0x0FFF)

#define PROMOT_ID_ANC_FLAG                      (0x01 << 7)

#define PROMOT_ID_SET_ANC_MODE(mode)            (((uint16_t)mode & 0x07) << 8)
#define PROMOT_ID_GET_ANC_MODE(id)              ((id >> 8) & 0x07)

extern "C" void app_prompt_ctrl_anc(app_anc_mode_t mode)
{
    uint16_t aud_id = PROMOT_ID_ANC_FLAG;

    aud_id |= PROMOT_ID_SET_ANC_MODE(mode);
    
    TRACE(5, "[%s] aud_id: 0x%x, mode: %d", __func__, aud_id, mode);

    media_PlayAudio((AUD_ID_ENUM)aud_id, 0);
}

uint16_t app_prompt_ctrl_anc_init(uint16_t aud_id)
{
    if ((aud_id & PROMOT_ID_ANC_FLAG) == false) {
        return PROMPT_ID_FROM_ID_VALUE(aud_id);
    }

    aud_id &= PROMOT_ID_ANC_MASK;
    g_anc_mode = (app_anc_mode_t)PROMOT_ID_GET_ANC_MODE(aud_id);

    if (g_anc_mode == APP_ANC_MODE_OFF) {
        //TODO: replace with needed one
        aud_id = AUDIO_ID_BT_MUTE;
    } else if (g_anc_mode == APP_ANC_MODE1) {
        //TODO:
        aud_id = AUDIO_ID_BT_MUTE;
    } else {
        //TODO:
        aud_id = AUDIO_ID_BT_MUTE;
    }

    return aud_id;
}

void app_prompt_ctrl_anc_process(void)
{
    if (g_anc_mode < APP_ANC_MODE_QTY) {
        TRACE(2, "[%s] Mode: %d", __func__, g_anc_mode);
        app_anc_switch_locally(g_anc_mode);
        g_anc_mode = APP_ANC_MODE_QTY;
    }
}
#endif

static int16_t *app_play_sbc_cache = NULL;
uint32_t app_play_sbc_more_data(uint8_t *buf, uint32_t len)
{
    uint8_t device_id = app_bt_audio_get_curr_a2dp_device();
#if defined(IBRT)
    app_ibrt_voice_report_trigger_checker();
#endif

#if defined(PROMPT_CTRL_SWITCH_ANC_MODE)
    app_prompt_ctrl_anc_process();
#endif

#if defined(AUDIO_OUTPUT_SW_GAIN) && defined(MEDIA_PLAYER_USE_CODEC2)
    af_codec_dac1_set_algo_gain(0.1);
#endif
    uint32_t l = 0;

    memset(buf, 0, len);
#if defined(VOICE_ASSIST_PROMPT_LEAK_DETECT)
    app_voice_assist_prompt_leak_detect_set_working_status(1);
#endif

#if defined(MEDIA_PLAY_24BIT)
    len /= 2;
#endif

#if defined(__BT_ANC__) || defined(PSAP_FORCE_STREAM_48K)
    uint32_t dec_len = len/g_prompt_upsampling_ratio;
#endif

    if(app_play_sbc_cache)
        memset(app_play_sbc_cache, 0, g_cache_buff_sz);

    if (app_play_sbc_stop_proc_cnt) {
        if (app_play_sbc_stop_proc_cnt == 1) {
            app_play_sbc_stop_proc_cnt = 2;
        } else if (app_play_sbc_stop_proc_cnt == 2) {
            app_play_sbc_stop_proc_cnt = 3;

#if defined(AUDIO_OUTPUT_SW_GAIN) && defined(MEDIA_PLAYER_USE_CODEC2)
            af_codec_dac1_set_algo_gain(1.0);
#endif

            // For 8K sample rate data, it takes about 4ms (or 12ms if h/w resample in use) from codec to DAC PA.
            // The playback stream should be stopped after the last data arrives at DAC PA, otherwise there
            // might be some pop sound.
            app_play_audio_stop();
        }
    } else {
        if (app_play_sbc_cache) {
#if defined(__BT_ANC__) || defined(PSAP_FORCE_STREAM_48K)
            len = dec_len;
#endif
            if (g_play_continue_mark) {
                l = app_play_continue_sbc_more_data(device_id, (uint8_t *)app_play_sbc_cache, len/MEDIA_PLAYER_CHANNEL_NUM);
            } else {
                l = app_play_single_sbc_more_data(device_id, (uint8_t *)app_play_sbc_cache, len/MEDIA_PLAYER_CHANNEL_NUM);
            }
            if (l != len / MEDIA_PLAYER_CHANNEL_NUM) {
#if defined(__BT_ANC__) || defined(PSAP_FORCE_STREAM_48K)
                len = dec_len*3;
#endif
                memset(app_play_sbc_cache+l, 0, len/MEDIA_PLAYER_CHANNEL_NUM-l);
                app_play_sbc_stop_proc_cnt = 1;
            }
#if defined(__BT_ANC__) || defined(PSAP_FORCE_STREAM_48K)
            len = dec_len*3;
            l = l*3;
            us_fir_run((short*)app_play_sbc_cache, (short*)buf, dec_len/MEDIA_PLAYER_CHANNEL_NUM/sizeof(int16_t));
            if (MEDIA_PLAYER_CHANNEL_NUM == AUD_CHANNEL_NUM_2) {
                app_play_sbc_channel_select((int16_t*)buf, (int16_t*)buf, len/MEDIA_PLAYER_CHANNEL_NUM/sizeof(int16_t));
            } else if (MEDIA_PLAYER_CHANNEL_NUM == AUD_CHANNEL_NUM_1) {
                if (IS_PROMPT_CHNLSEl_ALL(g_prompt_chnlsel)
#ifdef IBRT
                    || app_ibrt_voice_report_is_me(PROMPT_CHNLSEl_FROM_ID_VALUE(g_prompt_chnlsel))
#endif
                    ) {
                    memcpy(buf, app_play_sbc_cache, len);
                } else {
                    memset(buf, 0, len);
                }
            }
#else
            if (MEDIA_PLAYER_CHANNEL_NUM == AUD_CHANNEL_NUM_2) {
                app_play_sbc_channel_select((int16_t *)buf, app_play_sbc_cache, len/MEDIA_PLAYER_CHANNEL_NUM/sizeof(int16_t));
            } else if (MEDIA_PLAYER_CHANNEL_NUM == AUD_CHANNEL_NUM_1) {
                if (IS_PROMPT_CHNLSEl_ALL(g_prompt_chnlsel)
#ifdef IBRT
                    || app_ibrt_voice_report_is_me(PROMPT_CHNLSEl_FROM_ID_VALUE(g_prompt_chnlsel))
#endif
                    ) {
                    memcpy(buf, app_play_sbc_cache, len);
                } else {
                    memset(buf, 0, len);
                }
            }
#endif

#if defined(MEDIA_PLAY_24BIT)
            int32_t *buf32 = (int32_t *)buf;
            int16_t *buf16 = (int16_t *)buf;

            for (int16_t i = len/2 - 1; i >= 0; i--) {
                buf32[i] = ((int32_t)buf16[i] << 8);
            }
            len *= 2;
#endif
        } else {
#if defined(MEDIA_PLAY_24BIT)
            len *= 2;
#endif
            memset(buf, 0, len);
        }
    }
#if defined(__SW_IIR_PROMPT_EQ_PROCESS__)
#if defined(MEDIA_PLAY_24BIT)
    iir_run(buf, len/sizeof(int32_t));
#else
    iir_run(buf, len/sizeof(int16_t));
#endif
#endif

    return l;
}

#ifdef AUDIO_LINEIN
static uint8_t app_play_lineinmode_merge = 0;
static uint8_t app_play_lineinmode_mode = 0;

static inline void app_play_audio_lineinmode_mono_merge(int16_t *aud_buf_mono,
                                    int16_t *ring_buf_mono, uint32_t aud_buf_len)
{
    uint32_t i = 0;
    for (i = 0; i < aud_buf_len; i++) {
        aud_buf_mono[i] = (aud_buf_mono[i]>>1) + (ring_buf_mono[i]>>1);
    }
}

static inline void app_play_audio_lineinmode_stereo_merge(int16_t *aud_buf_stereo,
                                        int16_t *ring_buf_mono, uint32_t aud_buf_len)
{
    uint32_t stereo_offs = 0;
    uint32_t mono_offs = 0;
    for (stereo_offs = 0; stereo_offs < aud_buf_len; ) {
        aud_buf_stereo[stereo_offs] = aud_buf_stereo[stereo_offs] + (ring_buf_mono[mono_offs]>>1);
        stereo_offs++;
        aud_buf_stereo[stereo_offs] = aud_buf_stereo[stereo_offs] + (ring_buf_mono[mono_offs]>>1);
        stereo_offs++;
        mono_offs++;
    }
}

uint32_t app_play_audio_lineinmode_more_data(uint8_t *buf, uint32_t len)
{
    uint32_t l = 0;
    if (app_play_lineinmode_merge && app_play_sbc_cache){
        TRACE(1,"line in mode:%d ", len);
        if (app_play_lineinmode_mode == 1){
            if (g_play_continue_mark){
                l = app_play_continue_sbc_more_data(BT_DEVICE_ID_1, (uint8_t *)app_play_sbc_cache, len);
            }else{
                l = app_play_single_sbc_more_data(BT_DEVICE_ID_1, (uint8_t *)app_play_sbc_cache, len);
            }
            if (l != len){
                memset(app_play_sbc_cache+l, 0, len-l);
                app_play_lineinmode_merge = 0;
            }
            app_play_audio_lineinmode_mono_merge((int16_t *)buf, (int16_t *)app_play_sbc_cache, len/2);
        }else if (app_play_lineinmode_mode == 2){
            if (g_play_continue_mark){
                l = app_play_continue_sbc_more_data(BT_DEVICE_ID_1, (uint8_t *)app_play_sbc_cache, len/2);
            }else{
                l = app_play_single_sbc_more_data(BT_DEVICE_ID_1, (uint8_t *)app_play_sbc_cache, len/2);
            }
            if (l != len/2){
                memset(app_play_sbc_cache+l, 0, len/2-l);
                app_play_lineinmode_merge = 0;
            }
            app_play_audio_lineinmode_stereo_merge((int16_t *)buf, (int16_t *)app_play_sbc_cache, len/2);
        }
    }

    return l;
}

int app_play_audio_lineinmode_init(uint8_t mode, uint32_t buff_len)
{
    TRACE(1,"lapp_play_audio_lineinmode_init:%d ", buff_len);
    app_play_lineinmode_mode = mode;
    app_audio_mempool_get_buff((uint8_t **)&app_play_sbc_cache, buff_len);
    media_audio_init();
    return 0;
}

int app_play_audio_lineinmode_start(APP_AUDIO_STATUS* status)
{
    if (app_play_audio_sample_rate == AUD_SAMPRATE_44100){
        LOCK_APP_AUDIO_QUEUE();
        APP_AUDIO_DeCQueue(&media_sbc_queue, 0, APP_AUDIO_LengthOfCQueue(&media_sbc_queue));
        UNLOCK_APP_AUDIO_QUEUE();
        app_play_lineinmode_merge = 1;
        need_init_decoder = 1;
        media_Play_init_audio(status->aud_id);
    }
    return 0;
}

int app_play_audio_lineinmode_stop(APP_AUDIO_STATUS* status)
{
    app_play_lineinmode_merge = 0;
    return 0;
}
#endif


#if defined(AUDIO_ANC_FB_MC_MEDIA) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
#define DELAY_SAMPLE_MC (33*2)     //  2:ch
static int32_t delay_buf_media[DELAY_SAMPLE_MC];
static uint32_t audio_mc_data_playback_media(uint8_t *buf, uint32_t mc_len_bytes)
{
   uint32_t begin_time;
   //uint32_t end_time;
   begin_time = hal_sys_timer_get();
   TRACE(1,"media cancel: %d",begin_time);

   float left_gain;
   float right_gain;
   int32_t playback_len_bytes,mc_len_bytes_8;
   int32_t i,j,k;
   int delay_sample;

   mc_len_bytes_8=mc_len_bytes/8;

   hal_codec_get_dac_gain(&left_gain,&right_gain);

   //TRACE(1,"playback_samplerate_ratio:  %d",playback_samplerate_ratio);

  // TRACE(1,"left_gain:  %d",(int)(left_gain*(1<<12)));
  // TRACE(1,"right_gain: %d",(int)(right_gain*(1<<12)));

   playback_len_bytes=mc_len_bytes/playback_samplerate_ratio_bt;

    if (sample_size_play_bt == AUD_BITS_16)
    {
        int16_t *sour_p=(int16_t *)(playback_buf_bt+playback_size_bt/2);
        int16_t *mid_p=(int16_t *)(buf);
        int16_t *mid_p_8=(int16_t *)(buf+mc_len_bytes-mc_len_bytes_8);
        int16_t *dest_p=(int16_t *)buf;

        if(buf == playback_buf_mc)
        {
            sour_p=(int16_t *)playback_buf_bt;
        }

        delay_sample=DELAY_SAMPLE_MC;

        for(i=0,j=0;i<delay_sample;i=i+2)
        {
            mid_p[j++]=delay_buf_media[i];
            mid_p[j++]=delay_buf_media[i+1];
        }

        for(i=0;i<playback_len_bytes/2-delay_sample;i=i+2)
        {
            mid_p[j++]=sour_p[i];
            mid_p[j++]=sour_p[i+1];
        }

        for(j=0;i<playback_len_bytes/2;i=i+2)
        {
            delay_buf_media[j++]=sour_p[i];
            delay_buf_media[j++]=sour_p[i+1];
        }

        if(playback_samplerate_ratio_bt<=8)
        {
            for(i=0,j=0;i<playback_len_bytes/2;i=i+2*(8/playback_samplerate_ratio_bt))
            {
                mid_p_8[j++]=mid_p[i];
                mid_p_8[j++]=mid_p[i+1];
            }
        }
        else
        {
            for(i=0,j=0;i<playback_len_bytes/2;i=i+2)
            {
                for(k=0;k<playback_samplerate_ratio_bt/8;k++)
                {
                    mid_p_8[j++]=mid_p[i];
                    mid_p_8[j++]=mid_p[i+1];
                }
            }
        }

        anc_mc_run_stereo((uint8_t *)mid_p_8,mc_len_bytes_8,left_gain,right_gain,AUD_BITS_16);

        for(i=0,j=0;i<(mc_len_bytes_8)/2;i=i+2)
        {
            for(k=0;k<8;k++)
            {
                dest_p[j++]=mid_p_8[i];
                dest_p[j++]=mid_p_8[i+1];
            }
        }


    }
    else if (sample_size_play_bt == AUD_BITS_24)
    {
        int32_t *sour_p=(int32_t *)(playback_buf_bt+playback_size_bt/2);
        int32_t *mid_p=(int32_t *)(buf);
        int32_t *mid_p_8=(int32_t *)(buf+mc_len_bytes-mc_len_bytes_8);
        int32_t *dest_p=(int32_t *)buf;

        if(buf == (playback_buf_mc))
        {
            sour_p=(int32_t *)playback_buf_bt;
        }

        delay_sample=DELAY_SAMPLE_MC;

        for(i=0,j=0;i<delay_sample;i=i+2)
        {
            mid_p[j++]=delay_buf_media[i];
            mid_p[j++]=delay_buf_media[i+1];

        }

         for(i=0;i<playback_len_bytes/4-delay_sample;i=i+2)
        {
            mid_p[j++]=sour_p[i];
            mid_p[j++]=sour_p[i+1];
        }

         for(j=0;i<playback_len_bytes/4;i=i+2)
        {
            delay_buf_media[j++]=sour_p[i];
            delay_buf_media[j++]=sour_p[i+1];
        }

        if(playback_samplerate_ratio_bt<=8)
        {
            for(i=0,j=0;i<playback_len_bytes/4;i=i+2*(8/playback_samplerate_ratio_bt))
            {
                mid_p_8[j++]=mid_p[i];
                mid_p_8[j++]=mid_p[i+1];
            }
        }
        else
        {
            for(i=0,j=0;i<playback_len_bytes/4;i=i+2)
            {
                for(k=0;k<playback_samplerate_ratio_bt/8;k++)
                {
                    mid_p_8[j++]=mid_p[i];
                    mid_p_8[j++]=mid_p[i+1];
                }
            }
        }

        anc_mc_run_stereo((uint8_t *)mid_p_8,mc_len_bytes_8,left_gain,right_gain,AUD_BITS_24);

        for(i=0,j=0;i<(mc_len_bytes_8)/4;i=i+2)
        {
            for(k=0;k<8;k++)
            {
                dest_p[j++]=mid_p_8[i];
                dest_p[j++]=mid_p_8[i+1];
            }
        }

    }

  //  end_time = hal_sys_timer_get();

 //   TRACE(2,"%s:run time: %d", __FUNCTION__, end_time-begin_time);

    return 0;
}
#endif

void app_audio_playback_done(void)
{
#if defined(IBRT)
    app_tws_sync_prompt_check();
#endif
}

extern void bt_media_clear_media_type(uint16_t media_type, int device_id);

int app_play_audio_stop(void)
{
#if (!defined(PROMPT_SELF_MANAGEMENT))
    app_audio_sendrequest(APP_PLAY_BACK_AUDIO, APP_BT_SETTING_CLOSE, 0);
    bt_media_clear_current_media(BT_STREAM_MEDIA);
    for(uint8_t i = 0; i < BT_DEVICE_NUM; i++)
    {
        bt_media_clear_media_type(BT_STREAM_MEDIA, i);
    }
#else
    app_audio_sendrequest(APP_PLAY_BACK_AUDIO, APP_BT_SETTING_STOP_PROMPT, 0);
#endif
    return 0;
}


static uint32_t g_active_aud_id = MAX_RECORD_NUM;

int app_play_audio_set_aud_id(uint32_t aud_id)
{
    g_active_aud_id = aud_id;
    return 0;
}

int app_play_audio_get_aud_id(void)
{
    return g_active_aud_id;
}

void app_play_audio_set_lang(int L)
{
    g_language = L;
    TRACE(1, "language is set to: %d", g_language);
}

int app_play_audio_get_lang()
{
    return g_language;
}

#if defined(__SW_IIR_PROMPT_EQ_PROCESS__)
static int copy_prompt_sw_eq_band(enum AUD_SAMPRATE_T sample_rate, IIR_CFG_T *prompt_sw_eq_cfg, IIR_CFG_T *a2dp_eq_cfg)
{
    prompt_sw_eq_cfg->gain0 = a2dp_eq_cfg->gain0;
    prompt_sw_eq_cfg->gain1 = a2dp_eq_cfg->gain1;
    prompt_sw_eq_cfg->num = 0;
    for(int i =0, j=0; i< a2dp_eq_cfg->num; i++){
        if(a2dp_eq_cfg->param[i].fc < ((float)sample_rate/2)){
            prompt_sw_eq_cfg->param[j++] = a2dp_eq_cfg->param[i];
            prompt_sw_eq_cfg->num++;
        }
    }

    return 0;
}
#endif

static bool isPromptStreamingOn = false;
static bool app_prompt_is_streaming_on(void)
{
    return isPromptStreamingOn;
}

static uint32_t prompt_sample_rate = AUD_SAMPRATE_16000;
void app_prompt_config_sample_rate(uint32_t new_sample_rate)
{
    prompt_sample_rate = new_sample_rate;
}

static enum AUD_SAMPRATE_T app_prompt_get_current_sample_rate(void)
{
    return (enum AUD_SAMPRATE_T)prompt_sample_rate;
}

static app_prompt_finish_cb app_prompt_completed_callback = NULL;
int32_t app_prompt_finish_callback_register(app_prompt_finish_cb cb)
{
    app_prompt_completed_callback = cb;
    return 0;
}

int app_prompt_finish_callback(int aud_id)
{
    if (app_prompt_completed_callback)
    {
        app_prompt_completed_callback(aud_id);
    }
    return 0;
}

static app_prompt_start_cb app_prompt_started_callback = NULL;
int32_t app_prompt_start_callback_register(app_prompt_start_cb cb)
{
    app_prompt_started_callback = cb;
    return 0;
}

int app_prompt_start_callback(int aud_id)
{
    if (app_prompt_started_callback)
    {
        app_prompt_started_callback(aud_id);
    }
    return 0;
}

extern "C" uint8_t is_a2dp_mode(void);
int app_play_audio_onoff(bool onoff, APP_AUDIO_STATUS* status)
{
    struct AF_STREAM_CONFIG_T stream_cfg;
    uint8_t* bt_audio_buff = NULL;
    uint16_t aud_id = 0;
    uint16_t aud_pram = 0;
    int ret = 0;

    TRACE(2,"Audio prompt stream state %s, to %s it",
        isPromptStreamingOn?"Running":"Idle", onoff?"start":"stop");

    if (isPromptStreamingOn == onoff)
    {
        return 0;
    }

    if (onoff) {
        app_sysfreq_req(APP_SYSFREQ_USER_PROMPT_MIXER, APP_SYSFREQ_104M);
#if defined(__THIRDPARTY)
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO1,THIRDPARTY_STOP, 0);
#endif

#if defined(IBRT)
        bool isPlayingLocally = false;
        if (g_is_local_continuous_prompt_on_going)
        {
            isPlayingLocally = true;
        }

        if (IS_PROMPT_PLAYED_LOCALLY(status->aud_id))
        {
            isPlayingLocally = true;
        }
#endif

        aud_id = PROMPT_ID_FROM_ID_VALUE(status->aud_id);
        aud_pram = PROMPT_PRAM_FROM_ID_VALUE(status->aud_id);
        g_prompt_chnlsel = PROMPT_CHNLSEl_FROM_ID_VALUE(aud_pram);
        app_prompt_id_on_going = aud_id;
        TRACE(4,"aud_id:%04x %s aud_pram:%04x chnlsel:%d", status->aud_id, aud_id2str(aud_id), aud_pram, g_prompt_chnlsel);

#if defined(PROMPT_CTRL_SWITCH_ANC_MODE)
        media_Play_init_audio(status->aud_id);
#else
        media_Play_init_audio(aud_id);
#endif

#if defined(VOICE_ASSIST_PROMPT_LEAK_DETECT)
        if(aud_id == AUD_ID_ANC_PROMPT){
            app_voice_assist_prompt_leak_detect_open();
            app_voice_assist_prompt_leak_detect_set_working_status(0);
            // TRACE(0,"!!!!!!!!!!#################");
        }
#endif

#if defined(VOICE_ASSIST_CUSTOM_LEAK_DETECT)
        if(aud_id == AUD_ID_CUSTOM_LEAK_DETECT){
            app_voice_assist_custom_leak_detect_open();
        }
#endif
        app_play_audio_set_aud_id(aud_id);
        if (!g_app_audio_length){
#if defined(PROMPT_CTRL_SWITCH_ANC_MODE)
            app_prompt_ctrl_anc_process();
#endif
            isPromptStreamingOn = onoff;
#if (!defined(PROMPT_SELF_MANAGEMENT))
            app_audio_sendrequest(APP_PLAY_BACK_AUDIO, APP_BT_SETTING_CLOSE, 0);
#else
            app_audio_sendrequest(APP_PLAY_BACK_AUDIO, APP_BT_SETTING_STOP_PROMPT, 0);
#endif
            return 0;
        }

        af_set_priority(AF_USER_AUDIO, osPriorityHigh);
#if defined(MEDIA_PLAYER_USE_CODEC2)
        app_media_mempool_init();
#else
        app_audio_mempool_init_with_specific_size(app_audio_mempool_size());
#endif
        media_audio_init();
#if defined(PROMPT_USE_AAC)
        if(aac_mempoll == NULL)
            app_audio_mempool_get_buff(&aac_mempoll, AAC_MEMPOOL_SIZE);

        if(aac_memhandle == NULL) {
            aac_memhandle = heap_register(aac_mempoll, AAC_MEMPOOL_SIZE);
            aac_heap_api = aac_get_heap_api();
        }
        if (aacDec_handle == NULL){
            aac_dec_para_t dec_para;
            dec_para.package = AAC_PACKAGE_ADTS;
            dec_para.channel_select = 0;
            aacDec_handle = aac_decoder_open(aac_heap_api, dec_para);
            ASSERT(aacDec_handle, "aacDecoder_Open failed");
        }
#else
#ifdef SBC_RAM_OPT_1306
        extern void init_1306_sbc_rom_api();
        init_1306_sbc_rom_api();
#endif
        enum AUD_SAMPRATE_T sample_rate POSSIBLY_UNUSED = AUD_SAMPRATE_16000;
        sbc_decoder_open(media_sbc_decoder);

        sbc_frame_t sbc_data;
        sbc_data.sbc_data = g_app_audio_data;
        sbc_data.buffer_size = g_app_audio_length;
        sbc_data.valid_size = g_app_audio_length;
        sbc_decoder_parser_frame(media_sbc_decoder, &sbc_data);
        sbc_stream_info_t info;
        sbc_decoder_get_stream_info(media_sbc_decoder, &info);
        switch (info.sample_rate)
        {
            case SBC_SAMPLERATE_16K:
                sample_rate = AUD_SAMPRATE_16000;
                break;
            case SBC_SAMPLERATE_32K:
                sample_rate = AUD_SAMPRATE_32000;
                break;
            case SBC_SAMPLERATE_44_1K:
                sample_rate = AUD_SAMPRATE_44100;
                break;
            case SBC_SAMPLERATE_48K:
                sample_rate = AUD_SAMPRATE_48000;
                break;
            default:
                sample_rate = AUD_SAMPRATE_16000;
                break;
        }
#endif
#ifdef PLAYBACK_USE_I2S
        hal_cmu_audio_resample_disable();
#endif

        memset(&stream_cfg, 0, sizeof(stream_cfg));

#if defined(MEDIA_PLAY_24BIT)
        stream_cfg.bits = AUD_BITS_24;
#else
        stream_cfg.bits = AUD_BITS_16;
#endif
        stream_cfg.channel_num = MEDIA_PLAYER_CHANNEL_NUM;
#if defined(__BT_ANC__) || defined(PSAP_FORCE_STREAM_48K)
        stream_cfg.sample_rate =  AUD_SAMPRATE_48000;
#else
        stream_cfg.sample_rate =  AUD_SAMPRATE_16000;
#endif

        stream_cfg.sample_rate = app_prompt_get_current_sample_rate();

#ifdef PLAYBACK_USE_I2S
        stream_cfg.device = AUD_STREAM_USE_I2S0_MASTER;
        stream_cfg.io_path = AUD_IO_PATH_NULL;
#else
        stream_cfg.device = MEDIA_PLAYER_OUTPUT_DEVICE;
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
#endif
#ifdef __INTERACTION__
        if(aud_id == AUD_ID_BT_FINDME)
        {
            stream_cfg.vol = g_findme_fadein_vol;
        }
        else
#endif
        {
            stream_cfg.vol = MEDIA_VOLUME_LEVEL_WARNINGTONE;
        }
        stream_cfg.handler = app_play_sbc_more_data;

        stream_cfg.data_size = APP_AUDIO_PLAYBACK_BUFF_SIZE;

        g_cache_buff_sz = stream_cfg.data_size/MEDIA_PLAYER_CHANNEL_NUM/2 ;

#if defined(MEDIA_PLAY_24BIT)
        stream_cfg.data_size *= 2;
#endif

        app_audio_mempool_get_buff((uint8_t **)&app_play_sbc_cache, g_cache_buff_sz);
        app_audio_mempool_get_buff(&bt_audio_buff, stream_cfg.data_size);
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(bt_audio_buff);

#if defined(__BT_ANC__) || defined(PSAP_FORCE_STREAM_48K)
        us_fir_init(g_prompt_upsampling_ratio);
#endif

#if defined(AUDIO_ANC_FB_MC_MEDIA) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        sample_size_play_bt=stream_cfg.bits;
        sample_rate_play_bt=stream_cfg.sample_rate;
        data_size_play_bt=stream_cfg.data_size;
        playback_buf_bt=stream_cfg.data_ptr;
        playback_size_bt=stream_cfg.data_size;
#if defined(__BT_ANC__) || defined(PSAP_FORCE_STREAM_48K)
        playback_samplerate_ratio_bt=8;
#else
        playback_samplerate_ratio_bt=8*3;
#endif
        playback_ch_num_bt=stream_cfg.channel_num;
#endif

#if defined(ANC_ASSIST_ENABLED) && defined(ANC_ASSIST_USE_INT_CODEC)
        app_anc_assist_set_playback_info(stream_cfg.sample_rate);
        app_anc_assist_set_mode(ANC_ASSIST_MODE_NONE);
#endif

        TRACE(0, "[%s]: sample rate: %d, bits: %d, channel number: %d, data size:%d",
            __func__,
            stream_cfg.sample_rate,
            stream_cfg.bits,
            stream_cfg.channel_num,
            stream_cfg.data_size);

#if defined(__SW_IIR_PROMPT_EQ_PROCESS__)
        IIR_CFG_T prompt_iir_cfg;
        copy_prompt_sw_eq_band(stream_cfg.sample_rate, &prompt_iir_cfg, audio_eq_hw_dac_iir_cfg_list[0]);
        iir_open(stream_cfg.sample_rate, stream_cfg.bits, stream_cfg.channel_num);
        iir_set_cfg(&prompt_iir_cfg);
#endif
        af_stream_open(MEDIA_PLAY_STREAM_ID, AUD_STREAM_PLAYBACK, &stream_cfg);

#if defined(AUDIO_ANC_FB_MC_MEDIA) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        stream_cfg.bits = sample_size_play_bt;
        stream_cfg.channel_num = playback_ch_num_bt;
        stream_cfg.sample_rate = sample_rate_play_bt;
        stream_cfg.device = AUD_STREAM_USE_MC;
        stream_cfg.vol = 0;
        stream_cfg.handler = audio_mc_data_playback_media;
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;

        app_audio_mempool_get_buff(&bt_audio_buff, data_size_play_bt*playback_samplerate_ratio_bt);
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(bt_audio_buff);
        stream_cfg.data_size = data_size_play_bt*playback_samplerate_ratio_bt;

        playback_buf_mc=stream_cfg.data_ptr;
        playback_size_mc=stream_cfg.data_size;

        anc_mc_run_init(hal_codec_anc_convert_rate(sample_rate_play_bt));
        af_stream_open(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK, &stream_cfg);
#endif

#if !(defined(__AUDIO_RESAMPLE__) && defined(SW_PLAYBACK_RESAMPLE))
        AF_CODEC_TUNE(AUD_STREAM_PLAYBACK, 0);
#endif

#if defined(IBRT)
        if(!isPlayingLocally)
        {
#if defined(PLAYBACK_USE_I2S)
            app_ibrt_voice_resport_trigger_device_t trigger_device = {
                .device = AUD_STREAM_USE_I2S0_MASTER,
                .trigger_channel = 0,
            };
#elif defined(MEDIA_PLAYER_USE_CODEC2)
            app_ibrt_voice_resport_trigger_device_t trigger_device = {
                .device = AUD_STREAM_USE_INT_CODEC2,
                .trigger_channel = 1,
            };
#else
            app_ibrt_voice_resport_trigger_device_t trigger_device = {
                .device = AUD_STREAM_USE_INT_CODEC,
                .trigger_channel = 0,
            };
#endif

            ret = app_ibrt_voice_report_trigger_init(aud_id, aud_pram, &trigger_device);
        }
#endif
        app_prompt_start_callback(app_prompt_id_on_going);

        if (0 == ret)
        {
            af_stream_start(MEDIA_PLAY_STREAM_ID, AUD_STREAM_PLAYBACK);
#if defined(AUDIO_ANC_FB_MC_MEDIA) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
            af_stream_start(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK);
#endif
        } else {
#if defined(PROMPT_CTRL_SWITCH_ANC_MODE)
            app_prompt_ctrl_anc_process();
#endif
        }
    }
    else
    {
    
#if defined(AUDIO_OUTPUT_SW_GAIN) && defined(MEDIA_PLAYER_USE_CODEC2)
        af_codec_dac1_set_algo_gain(1.0);
#endif

#if !(defined(__AUDIO_RESAMPLE__) && defined(SW_PLAYBACK_RESAMPLE))
        AF_CODEC_TUNE(AUD_STREAM_PLAYBACK, 0);
#endif
#ifdef AF_STREAM_ID_0_PLAYBACK_FADEOUT
	if((is_a2dp_mode()==false) && (app_play_audio_get_aud_id() == AUD_ID_BT_CALL_INCOMING_CALL)){
    	af_stream_playback_fadeout(MEDIA_PLAY_STREAM_ID, 7);
	}
#endif

        af_stream_stop(MEDIA_PLAY_STREAM_ID, AUD_STREAM_PLAYBACK);
        af_stream_close(MEDIA_PLAY_STREAM_ID, AUD_STREAM_PLAYBACK);

#if defined(VOICE_ASSIST_PROMPT_LEAK_DETECT)
        app_voice_assist_prompt_leak_detect_close();
#endif
#if defined(VOICE_ASSIST_CUSTOM_LEAK_DETECT)
        app_voice_assist_custom_leak_detect_set_working_status(0);
#endif

#if defined(__SW_IIR_PROMPT_EQ_PROCESS__)
        iir_close();
#endif

#if defined(AUDIO_ANC_FB_MC_MEDIA) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        af_stream_stop(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK);
        af_stream_close(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK);
#endif
#if defined(IBRT)
        app_ibrt_voice_report_trigger_deinit();
#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
        audio_prompt_stop_playing();
#endif
#endif
#ifdef PLAYBACK_USE_I2S
        hal_cmu_audio_resample_enable();
#endif

        app_play_sbc_cache = NULL;
        g_cache_buff_sz = 0;
        g_prompt_chnlsel = PROMOT_ID_BIT_MASK_CHNLSEl_ALL;
        app_play_audio_set_aud_id(MAX_RECORD_NUM);
        af_set_priority(AF_USER_AUDIO, osPriorityAboveNormal);

        app_audio_playback_done();
#if defined(__THIRDPARTY)
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO1,THIRDPARTY_START, 0);
#endif

#if defined(ANC_ASSIST_ENABLED) && defined(ANC_ASSIST_USE_INT_CODEC)
        app_anc_assist_set_playback_info(AUD_SAMPRATE_NULL);
        app_anc_assist_set_mode(ANC_ASSIST_MODE_STANDALONE);
#endif

#if defined(PROMPT_USE_AAC)
        if (aacDec_handle){
            aac_decoder_close(aacDec_handle);
            aacDec_handle = NULL;
        }

        aac_memhandle = NULL;
        aac_mempoll = NULL;
#endif
    }

#if defined(ENABLE_CALCU_CPU_FREQ_LOG)
    TRACE(0, "[%s] sysfreq calc : %d", __FUNCTION__, hal_sys_timer_calc_cpu_freq(5, 0));
#endif

    isPromptStreamingOn = onoff;

    prompt_check_user_when_streaming_is_completed();

    if (!onoff)
    {
        app_prompt_finish_callback(app_prompt_id_on_going);
        app_sysfreq_req(APP_SYSFREQ_USER_PROMPT_MIXER, APP_SYSFREQ_32K);
    }

    return 0;
}

POSSIBLY_UNUSED static void app_stop_local_prompt_playing(void)
{
#if (!defined(PROMPT_SELF_MANAGEMENT))
    app_audio_sendrequest(APP_PLAY_BACK_AUDIO, APP_BT_SETTING_CLOSE, 0);
#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
    audio_prompt_stop_playing();
#endif
#else            
    app_audio_sendrequest(APP_PLAY_BACK_AUDIO, APP_BT_SETTING_STOP_PROMPT, 0);
#endif
}

void app_stop_both_prompt_playing(void)
{
    app_prompt_stop_all();
#if defined(IBRT)
    app_tws_stop_peer_prompt();
#endif
    media_PlayAudio_stop_continuous_prompt();
    media_PlayAudio_stop_local_continuous_prompt();
}

void app_tws_cmd_stop_prompt_handler(uint8_t* ptrParam, uint16_t paramLen)
{
    TRACE(1,"%s", __func__);
    //app_stop_local_prompt_playing();

    media_PlayAudio_stop_continuous_prompt();
}

static void app_prompt_continuous_checker_callback(bool status)
{
    TRACE(0, "continuous_checker: %d", status);
    app_prompt_inform_completed_event();
}

static void app_prompt_start_continuous_checker(void)
{
    TRACE(0, "start continuous prompt checker");
    app_bt_stream_register_notify_trigger_status_callback(app_prompt_continuous_checker_callback);
}
static void app_prompt_stop_continuous_checker(void)
{
    app_bt_stream_register_notify_trigger_status_callback(NULL);
}

static void app_prompt_protector_timer_handler(void const *param);
osTimerDef(app_prompt_protector_timer,
    app_prompt_protector_timer_handler);
static osTimerId app_prompt_protector_timer_id = NULL;

static const AUD_ID_ENUM app_prompt_continous_id[] =
    {AUDIO_ID_FIND_MY_BUDS };
static AUD_ID_ENUM app_prompt_on_going_prompt_id;

// shall cover the longest prompt, if a prompt is longer than this expire time,
// increase it
#define APP_PROMPT_PROTECTOR_EXPIRE_TIMEOUT_MS  15000

static void app_prompt_start_protector(void)
{
    if (NULL == app_prompt_protector_timer_id)
    {
        app_prompt_protector_timer_id =
            osTimerCreate(osTimer(app_prompt_protector_timer), osTimerOnce, NULL);
    }

    osTimerStart(app_prompt_protector_timer_id, APP_PROMPT_PROTECTOR_EXPIRE_TIMEOUT_MS);
}

static void app_prompt_protector_timer_handler(void const *param)
{
    TRACE(0, "a wild prompt has blocked system for %d ms!",
        APP_PROMPT_PROTECTOR_EXPIRE_TIMEOUT_MS);
    // timeout, clear the prompt list
    for (uint32_t i = 0;i < ARRAY_SIZE(app_prompt_continous_id); i++)
    {
        if ((app_prompt_on_going_prompt_id == app_prompt_continous_id[i])&&
            app_bt_stream_isrun(APP_PLAY_BACK_AUDIO))
        {
            app_prompt_start_protector();
            return;
        }
    }
    app_prompt_stop_all();
}

static void app_prompt_stop_protector(void)
{
    if (app_prompt_protector_timer_id)
    {
        osTimerStop(app_prompt_protector_timer_id);
    }
}

#define APP_PROMPT_MAXIMUM_QUEUED_CNT   8

osPoolDef (app_prompt_request_mempool, APP_PROMPT_MAXIMUM_QUEUED_CNT, APP_PROMPT_PLAY_REQ_T);
static osPoolId app_prompt_request_mempool = NULL;

static list_node_t *app_prompt_list_begin(const list_t *list)
{
    uint32_t lock = int_lock();
    list_node_t *node = list_begin(list);
    int_unlock(lock);
    return node;
}

static APP_PROMPT_PLAY_REQ_T *app_prompt_list_node(const list_node_t *node)
{
    uint32_t lock = int_lock();
    void *data = list_node(node);
    int_unlock(lock);
    return (APP_PROMPT_PLAY_REQ_T *)data;
}

static bool app_prompt_list_remove(list_t *list, void *data)
{
    uint32_t lock = int_lock();
    bool nRet = list_remove(list, data);
    int_unlock(lock);
    return nRet;
}

static bool app_prompt_list_node_cmp(const void *x,const void *y)
{
    APP_PROMPT_PLAY_REQ_T* x_prompt = (APP_PROMPT_PLAY_REQ_T*)x;
    APP_PROMPT_PLAY_REQ_T* y_prompt = (APP_PROMPT_PLAY_REQ_T*)y;

    if ((x_prompt->deviceId == y_prompt->deviceId)
        && (x_prompt->promptId == y_prompt->promptId)
        && (x_prompt->reqType == y_prompt->reqType))
    {
        return true;
    }
    else
    {
        return false;
    }
}

static bool app_prompt_list_append(list_t *list, APP_PROMPT_PLAY_REQ_T *req)
{
    APP_PROMPT_PLAY_REQ_T* reqToAppendList = NULL;
    bool nRet = false;

    uint32_t lock = int_lock();
    if (!list_find(list, app_prompt_list_node_cmp, (void*)req))
    {
        reqToAppendList = (APP_PROMPT_PLAY_REQ_T *)osPoolCAlloc (app_prompt_request_mempool);
        if (NULL == reqToAppendList)
        {
            TRACE(0, "prompt request memory pool is full.");
            int_unlock(lock);
            return false;
        }

        memcpy((uint8_t *)reqToAppendList, (uint8_t *)req, sizeof(APP_PROMPT_PLAY_REQ_T));
        nRet = list_append(list, (void *)reqToAppendList);
    }
    else
    {
        TRACE(0, "duplicated prompt request.");
        int_unlock(lock);
        return true;
    }
    int_unlock(lock);
    return nRet;
}

static void app_prompt_list_clear(list_t *list)
{
    uint32_t lock = int_lock();
    list_clear(list);
    int_unlock(lock);
}

static void app_prompt_list_free(void* data)
{
    osPoolFree (app_prompt_request_mempool, data);
}

void app_prompt_list_init(void)
{
    if (NULL == app_prompt_request_mempool)
    {
        app_prompt_request_mempool =
            osPoolCreate(osPool(app_prompt_request_mempool));
    }

    if (app_prompt_list == NULL)
    {
        app_prompt_list = list_new(app_prompt_list_free, NULL, NULL);
    }
    TRACE(2, "%s ca %p", __func__, __builtin_return_address(0));

#if defined(USE_BASIC_THREADS)
    app_set_threadhandle(APP_MODULE_MEDIA, app_prompt_handler);
#else
    app_prompt_handler_tid =
        osThreadCreate(osThread(app_prompt_handler_thread), NULL);
#endif
}

static void app_prompt_clear_prompt_list(void);
static void app_prompt_clear_non_continuous_prompt(void);

void app_prompt_push_request(APP_PROMPT_PLAY_REQ_TYPE_E reqType,
    AUD_ID_ENUM id, uint8_t device_id, bool isLocalPlaying)
{
    APP_PROMPT_PLAY_REQ_T req;
    req.deviceId = device_id;
    req.promptId = (uint16_t)id;
    req.reqType = reqType;
    req.isLocalPlaying = isLocalPlaying;

    TRACE(0, "push prompt request type %d devId %d promptId 0x%x isLocalPlaying %d",
        reqType, device_id, id, isLocalPlaying);

    if (AUDIO_ID_BT_MUTE == id)
    {

        TRACE(2, "%s ca %p", __func__, __builtin_return_address(0));
        // keep the continuous prompt if any
        if (media_playAudio_is_continuous_prompt_on_going())
        {
            app_prompt_clear_non_continuous_prompt();
        }
        else
        {
            app_prompt_clear_prompt_list();
        }
    }

    bool ret = app_prompt_list_append(app_prompt_list, &req);
    if (ret)
    {
#if defined(USE_BASIC_THREADS)
        app_prompt_inform_new_prompt_event();
#else
        osSignalSet(app_prompt_handler_tid, PROMPT_HANDLER_SIGNAL_NEW_PROMPT_REQ);
#endif
    }
}

AUD_ID_ENUM app_prompt_get_current_prompt_id(void)
{
    return app_prompt_on_going_prompt_id;
}

static bool app_prompt_refresh_list(void)
{
    if (app_prompt_is_on_going)
    {
        TRACE(0, "A prompt playing is on-going.");
        return false;
    }

    list_t *list = app_prompt_list;
    list_node_t *node = NULL;
    APP_PROMPT_PLAY_REQ_T* pPlayReq = NULL;

    if ((node = app_prompt_list_begin(list)) != NULL) {
        pPlayReq = app_prompt_list_node(node);
        if (APP_PROMPT_CACHED_ORG_REQUEST == pPlayReq->reqType)
        {
            APP_PROMPT_PLAY_REQ_T localReq = *pPlayReq; 
            app_prompt_list_remove(list, pPlayReq);
            localReq.func((AUD_ID_ENUM)(localReq.promptId), localReq.deviceId);
            return true;
        }
        app_prompt_is_on_going = true;
        app_prompt_on_going_prompt_id = (AUD_ID_ENUM)pPlayReq->promptId;
        app_prompt_start_protector();
        bool isConitnuousPrompt = false;

        switch (pPlayReq->reqType)
        {
            case APP_PROMPT_NORMAL_PLAY:
                media_PlayAudio_handler(
                    (AUD_ID_ENUM)pPlayReq->promptId, pPlayReq->deviceId, pPlayReq->isLocalPlaying);
                break;
            case APP_PROMPT_CONTINUOUS_NORMAL_PLAY:
                // if a2dp streaming is active but its playback is not triggered yet
                if (bt_media_is_media_active_by_type(BT_STREAM_MUSIC) &&
                    (!bt_is_playback_triggered()))
                {
                    app_prompt_is_on_going = false;
                    app_prompt_on_going_prompt_id = AUD_ID_INVALID;
                    app_prompt_stop_protector();
                    app_prompt_start_continuous_checker();
                    return false;
                }
                isConitnuousPrompt = true;
                media_PlayAudio_handler(
                    (AUD_ID_ENUM)pPlayReq->promptId, pPlayReq->deviceId, pPlayReq->isLocalPlaying);
                break;
            case APP_PROMPT_LOCAL_PLAY:
                media_PlayAudio_locally_handler(
                    (AUD_ID_ENUM)pPlayReq->promptId, pPlayReq->deviceId, pPlayReq->isLocalPlaying);
                break;
            case APP_PROMPT_LOCAL_CONTINUOUS_NORMAL_PLAY:
                // if a2dp streaming is active but its playback is not triggered yet
                if (bt_media_is_media_active_by_type(BT_STREAM_MUSIC) &&
                    (!bt_is_playback_triggered()))
                {
                    app_prompt_is_on_going = false;
                    app_prompt_on_going_prompt_id = AUD_ID_INVALID;
                    app_prompt_stop_protector();
                    app_prompt_start_continuous_checker();
                    return false;
                }
                isConitnuousPrompt = true;
                media_PlayAudio_locally_handler(
                    (AUD_ID_ENUM)pPlayReq->promptId, pPlayReq->deviceId, pPlayReq->isLocalPlaying);
                break;
            case APP_PROMPT_REMOTE_PLAY:
                media_PlayAudio_remotely_handler(
                    (AUD_ID_ENUM)pPlayReq->promptId, pPlayReq->deviceId, pPlayReq->isLocalPlaying);
                break;
            case APP_PROMPT_CONTINUOUS_REMOTE_PLAY:
                // if a2dp streaming is active but its playback is not triggered yet
                if (bt_media_is_media_active_by_type(BT_STREAM_MUSIC) &&
                    (!bt_is_playback_triggered()))
                {
                    app_prompt_is_on_going = false;
                    app_prompt_on_going_prompt_id = AUD_ID_INVALID;
                    app_prompt_stop_protector();
                    return false;
                }
                isConitnuousPrompt = true;
                media_PlayAudio_remotely_handler(
                    (AUD_ID_ENUM)pPlayReq->promptId, pPlayReq->deviceId, pPlayReq->isLocalPlaying);
                break;
            case APP_PROMPT_STANDALONE_PLAY:
                media_PlayAudio_standalone_handler(
                    (AUD_ID_ENUM)pPlayReq->promptId, pPlayReq->deviceId, pPlayReq->isLocalPlaying);
                break;
            case APP_PROMPT_STANDALONE_LOCAL_PLAY:
                media_PlayAudio_standalone_locally_handler(
                    (AUD_ID_ENUM)pPlayReq->promptId, pPlayReq->deviceId, pPlayReq->isLocalPlaying);
                break;
            case APP_PROMPT_STANDALONE_REMOTE_PLAY:
                media_PlayAudio_standalone_remotely_handler(
                    (AUD_ID_ENUM)pPlayReq->promptId, pPlayReq->deviceId, pPlayReq->isLocalPlaying);
                break;
            case APP_PROMPT_PENDING_FOR_MASTER_SYNC_PLAY_REQ:
                TRACE(0, "Wait for master to send sync play request");
                break;
            default:
                ASSERT(false, "Invalid prompt req type %d.", pPlayReq->reqType);
                break;
        }

        if (!isConitnuousPrompt)
        {
            app_prompt_list_remove(list, pPlayReq);
        }

        return true;
    }else {
        TRACE(0, "Prompt list is empty");
    }

    return false;
}

void app_prompt_inform_completed_event(void)
{
#if defined(USE_BASIC_THREADS)
    APP_MESSAGE_BLOCK msg;
    msg.mod_id = APP_MODULE_MEDIA;
    msg.mod_level = APP_MOD_LEVEL_2;
    msg.msg_body.message_Param0 = PROMPT_HANDLER_SIGNAL_PLAYING_COMPLETED;
    app_mailbox_put(&msg);
#else
    osSignalSet(app_prompt_handler_tid, PROMPT_HANDLER_SIGNAL_PLAYING_COMPLETED);
#endif
}

void app_prompt_stop_all(void)
{
#if defined(USE_BASIC_THREADS)
    APP_MESSAGE_BLOCK msg;
    msg.mod_id = APP_MODULE_MEDIA;
    msg.mod_level = APP_MOD_LEVEL_2;
    msg.msg_body.message_Param0 = PROMPT_HANDLER_SIGNAL_CLEAR_REQ;
    app_mailbox_put(&msg);
#else
    osSignalSet(app_prompt_handler_tid, PROMPT_HANDLER_SIGNAL_CLEAR_REQ);
#endif
}

void app_prompt_inform_new_prompt_event(void)
{
#if defined(USE_BASIC_THREADS)
    APP_MESSAGE_BLOCK msg;
    msg.mod_id = APP_MODULE_MEDIA;
    msg.mod_level = APP_MOD_LEVEL_2;
    msg.msg_body.message_Param0 = PROMPT_HANDLER_SIGNAL_NEW_PROMPT_REQ;
    app_mailbox_put(&msg);
#else
    osSignalSet(app_prompt_handler_tid, PROMPT_HANDLER_SIGNAL_NEW_PROMPT_REQ);
#endif
}

// assure that all pending prompts especailly power-off can be played
// at the time of power-off, after that, disconnect all of the links,
// to avoid the possible issue that only one-earbud plays power-off prompt
#define APP_PROMPT_FLUSH_TIMEOUT_MS APP_PROMPT_PROTECTOR_EXPIRE_TIMEOUT_MS
#define APP_PROMPT_FLUSH_CHECK_INTERVAL_MS  100
#define APP_PROMPT_FLUSH_CHECK_COUNT        ((APP_PROMPT_FLUSH_TIMEOUT_MS)/(APP_PROMPT_FLUSH_CHECK_INTERVAL_MS))
void app_prompt_flush_pending_prompts(void)
{
    uint32_t checkCounter = 0;
    while (checkCounter++ < APP_PROMPT_FLUSH_CHECK_COUNT)
    {
        if (!app_prompt_is_on_going)
        {
            app_prompt_stop_all();
            break;
        }
    }
}

static void app_prompt_clear_non_continuous_prompt(void)
{
    list_t *list = app_prompt_list;
    list_node_t *node = NULL;
    APP_PROMPT_PLAY_REQ_T* pPlayReq = NULL;
    TRACE(2, "%s ca %p", __func__, __builtin_return_address(0));
    if ((node = app_prompt_list_begin(list)) != NULL) {
        pPlayReq = app_prompt_list_node(node);
        if (APP_PROMPT_CONTINUOUS_NORMAL_PLAY != pPlayReq->reqType)
        {
            app_prompt_list_remove(list, pPlayReq);
        }
    }
}

static void app_prompt_clear_prompt_list(void)
{
    bool POSSIBLY_UNUSED stop_prompt_immediately = true;
    list_t *list = app_prompt_list;
    list_node_t *node = NULL;
    APP_PROMPT_PLAY_REQ_T* pPlayReq = NULL;

    g_is_continuous_prompt_on_going = false;
    if ((node = app_prompt_list_begin(list)) != NULL)
    {
        pPlayReq = app_prompt_list_node(node);
        if (APP_PROMPT_CONTINUOUS_NORMAL_PLAY == pPlayReq->reqType)
        {
            stop_prompt_immediately = false;
            TRACE(0,"don't stop prompt immediately");
        }
    }

    TRACE(2, "%s ca %p", __func__, __builtin_return_address(0));

    // clear the request list
    app_prompt_list_clear(app_prompt_list);

    // stop the on-going prompt

#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
    audio_prompt_stop_playing();
#endif

#if (!defined(PROMPT_SELF_MANAGEMENT))
    app_audio_sendrequest(APP_PLAY_BACK_AUDIO, APP_BT_SETTING_CLOSE, 0);
#else
    if (stop_prompt_immediately)
    {
        app_audio_sendrequest(APP_PLAY_BACK_AUDIO, APP_BT_SETTING_STOP_PROMPT, 0);
    }
#endif
}

#if defined(USE_BASIC_THREADS)
static int app_prompt_handler(APP_MESSAGE_BODY *msg_body)
{
    if (msg_body->message_Param0 & PROMPT_HANDLER_SIGNAL_NEW_PROMPT_REQ)
    {
        app_prompt_refresh_list();
    }
    else if (msg_body->message_Param0 & PROMPT_HANDLER_SIGNAL_CLEAR_REQ)
    {
        app_prompt_clear_prompt_list();

        if (g_is_local_continuous_prompt_on_going && (continuous_prompt_id != AUD_ID_INVALID))
        {
            app_prompt_push_request(APP_PROMPT_LOCAL_CONTINUOUS_NORMAL_PLAY, continuous_prompt_id, 0, true);
        }
        else if (g_is_continuous_prompt_on_going && (continuous_prompt_id != AUD_ID_INVALID))
        {
            app_prompt_push_request(APP_PROMPT_CONTINUOUS_NORMAL_PLAY, continuous_prompt_id, 0, false);
        }
    }
    else if (msg_body->message_Param0 & PROMPT_HANDLER_SIGNAL_PLAYING_COMPLETED)
    {
        app_prompt_is_on_going = false;
        app_prompt_on_going_prompt_id = AUD_ID_INVALID;
        app_prompt_stop_protector();
        app_prompt_refresh_list();
    }
    return 0;
}
#else
static void app_prompt_handler_thread(void const *argument)
{
    TRACE(0, "enter prompt handler thread.");
    while(1)
    {
        osEvent evt;
        // wait any signal
        evt = osSignalWait(0x0, osWaitForever);

        // get role from signal value
        if (evt.status == osEventSignal)
        {
            TRACE(0, "prompt handler thread gets signal 0x%x", evt.value.signals);

            if (evt.value.signals & PROMPT_HANDLER_SIGNAL_NEW_PROMPT_REQ)
            {
                app_prompt_refresh_list();
            }
            else if (evt.value.signals & PROMPT_HANDLER_SIGNAL_CLEAR_REQ)
            {
                app_prompt_clear_prompt_list();

                if (g_is_local_continuous_prompt_on_going && (continuous_prompt_id != AUD_ID_INVALID))
                {
                    app_prompt_push_request(APP_PROMPT_LOCAL_CONTINUOUS_NORMAL_PLAY, continuous_prompt_id, 0, true);
                }
                else if (g_is_continuous_prompt_on_going && (continuous_prompt_id != AUD_ID_INVALID))
                {
                    app_prompt_push_request(APP_PROMPT_CONTINUOUS_NORMAL_PLAY, continuous_prompt_id, 0, false);
                }
            }
            else if (evt.value.signals & PROMPT_HANDLER_SIGNAL_PLAYING_COMPLETED)
            {
                app_prompt_is_on_going = false;
                app_prompt_on_going_prompt_id = AUD_ID_INVALID;
                app_prompt_stop_protector();
                app_prompt_refresh_list();
            }
        }
    }
}
#endif
static void demo_prompt_user_callback_func(PROMPT_CHECK_USER_E user)
{
    TRACE(0, "prompt is completed!");
}

static PROMPT_CHECK_USER_ENV_T promptCheckUserEnv[PROMPT_CHECK_USER_NUM];

void prompt_check_user_register_post_prompt_callback_func(PROMPT_CHECK_USER_E user,
    post_prompt_user_callback_func func)
{
    if (user < PROMPT_CHECK_USER_NUM)
    {
        promptCheckUserEnv[user].postPromptFunc = func;
    }
}

void prompt_check_user_init(void)
{
    memset((void *)promptCheckUserEnv, 0, sizeof(promptCheckUserEnv));
    prompt_check_user_register_post_prompt_callback_func(PROMPT_CHECK_USER_DEMO,
        demo_prompt_user_callback_func);
}

static void prompt_check_user_when_streaming_is_completed(void)
{
    for (uint32_t user = 0; user < PROMPT_CHECK_USER_NUM; user++)
    {
        if (promptCheckUserEnv[user].isPendingForPromptCompleted)
        {
            promptCheckUserEnv[user].isPendingForPromptCompleted = false;
            if (promptCheckUserEnv[user].postPromptFunc)
            {
                TRACE(0, "Blocked user %d can be processed when prompt is completed",
                    user);
                promptCheckUserEnv[user].postPromptFunc((PROMPT_CHECK_USER_E)user);
                TRACE(0, "Post prompt handling of user %d done", user);
            }
        }
    }
}

bool prompt_check_user_if_need_to_wait(PROMPT_CHECK_USER_E user)
{
    if (user < PROMPT_CHECK_USER_NUM)
    {
        if (app_prompt_is_streaming_on())
        {
            promptCheckUserEnv[user].isPendingForPromptCompleted = true;
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

#ifdef IBRT
void audio_prompt_req_send_failed_cb(uint16_t cmdCode, uint8_t* p_buff, uint16_t length)
{
    app_tws_voice_prompt_to_play_t req;

    if (APP_TWS_CMD_LET_PEER_PLAY_PROMPT == cmdCode) {
        memcpy(&req, p_buff, sizeof(app_tws_voice_prompt_to_play_t));
        if ((AUD_ID_ENUM)req.voicePrompt == AUDIO_ID_BT_MUTE) {
            app_prompt_push_request(APP_PROMPT_STANDALONE_PLAY, (AUD_ID_ENUM)req.voicePrompt , req.dev_id, true);
        }
        else if ((AUD_ID_ENUM)req.voicePrompt == AUD_ID_BT_DIS_CONNECT)
        {
            app_prompt_push_request(APP_PROMPT_NORMAL_PLAY, (AUD_ID_ENUM)req.voicePrompt, req.dev_id, true);
        }
    }
}
#endif
#endif

