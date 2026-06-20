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
#include "cmsis_os.h"
#include "string.h"
#include "cqueue.h"
#include "list.h"

#include "hal_trace.h"
#include "hal_aud.h"
#include "bluetooth_bt_api.h"
#if defined(USE_BASIC_THREADS)
#include "app_thread.h"
#else
#include "app_audio_thread.h"
#endif
#include "app_audio.h"
#include "app_media_player.h"
#include "app_ring_merge.h"
#include "nvrecord_bt.h"
#include <assert.h>


#include "a2dp_api.h"

#include "btapp.h"
#include "app_bt_media_manager.h"
#include "hal_location.h"

#include "app_anc_assist.h"

#ifdef __AI_VOICE__
#include "app_ai_voice.h"
#endif

static bool app_audio_init = false;
static uint32_t ANC_ASSIST_BUFF_SIZE = 0;
static uint32_t TOTA_BUFF_SIZE = 0;
static uint32_t ANC_TEST_BUFF_SIZE = 0;

static uint32_t capture_audio_buff_size_used;
// from the bottom of the system available memory pool, size is APP_CAPTURE_AUDIO_BUFFER_SIZE
// can be overlayed with the sco used audio buffer
static uint8_t* capture_audio_buffer;

int app_capture_audio_mempool_init(void)
{
    TRACE(1,"init the app capture audio mem pool size %d.", APP_CAPTURE_AUDIO_BUFFER_SIZE);
    capture_audio_buffer = syspool_start_addr() + app_audio_mempool_size();
    capture_audio_buff_size_used = 0;

    memset((uint8_t *)capture_audio_buffer, 0, APP_CAPTURE_AUDIO_BUFFER_SIZE);
    TRACE(1,"capture syspool start addr: %p, %p.", syspool_start_addr(), capture_audio_buffer);

    return 0;
}

uint32_t app_capture_audio_mempool_free_buff_size()
{
    return APP_CAPTURE_AUDIO_BUFFER_SIZE - capture_audio_buff_size_used;
}

int app_capture_audio_mempool_get_buff(uint8_t **buff, uint32_t size)
{
    uint32_t buff_size_free;
    uint8_t* capture_buf_addr = (uint8_t *)capture_audio_buffer;
    buff_size_free = app_capture_audio_mempool_free_buff_size();

    if (size % 4){
        size = size + (4 - size % 4);
    }

    TRACE(2,"Get capture buf, current free %d to allocate %d", buff_size_free, size);

    ASSERT(size <= buff_size_free, "[%s] size = %d > free size = %d", __func__, size, buff_size_free);

    *buff = capture_buf_addr + capture_audio_buff_size_used;

    capture_audio_buff_size_used += size;
    TRACE(3,"Allocate %d, now used %d left %d",
        size, capture_audio_buff_size_used, app_capture_audio_mempool_free_buff_size());

    return 0;
}

#if defined(ANC_ASSIST_ENABLED)
static uint32_t anc_assist_buffer_size_used;
static uint8_t *anc_assist_buffer = NULL;
int app_anc_assist_mempool_init(uint32_t buff_size)
{
    ANC_ASSIST_BUFF_SIZE = buff_size;
    TRACE(1,"[%s]init the app anc assist mem pool size %d.", __func__, ANC_ASSIST_BUFF_SIZE);
    anc_assist_buffer =  syspool_start_addr() + app_audio_mempool_size() + APP_CAPTURE_AUDIO_BUFFER_SIZE;
    anc_assist_buffer_size_used = 0;

    memset((uint8_t *)anc_assist_buffer, 0, ANC_ASSIST_BUFF_SIZE);
    TRACE(1,"[%s]capture syspool start addr: %p, %p.", __func__, syspool_start_addr(), anc_assist_buffer);

    return 0;
}

uint32_t app_anc_assist_mempool_free_buff_size()
{
    return ANC_ASSIST_BUFF_SIZE - anc_assist_buffer_size_used;
}

int app_anc_assist_mempool_get_buff(uint8_t **buff, uint32_t size)
{
    uint32_t buff_size_free;
    uint8_t* capture_buf_addr = (uint8_t *)anc_assist_buffer;
    buff_size_free = app_anc_assist_mempool_free_buff_size();

    if (size % 4){
        size = size + (4 - size % 4);
    }

    TRACE(2,"[%s]Get capture buf, current free %d to allocate %d", __func__, buff_size_free, size);

    ASSERT(size <= buff_size_free, "[%s] size = %d > free size = %d", __func__, size, buff_size_free);

    *buff = capture_buf_addr + anc_assist_buffer_size_used;

    anc_assist_buffer_size_used += size;
    TRACE(3,"[%s]Allocate %d, now used %d left %d",
        __func__, size, anc_assist_buffer_size_used, app_anc_assist_mempool_free_buff_size());

    return 0;
}
#endif

#if defined(TOTA_FACTORY_USED)
static uint32_t tota_buffer_size_used;
static uint8_t *tota_buffer = NULL;

int app_tota_mempool_init(uint32_t buff_size)
{
    if (buff_size % 4){
        buff_size = buff_size + (4 - buff_size % 4);
    }

    TOTA_BUFF_SIZE = buff_size;
    TRACE(1,"[%s]init the app anc test mem pool size %d.", __func__, TOTA_BUFF_SIZE);
    tota_buffer =  syspool_start_addr() + app_audio_mempool_size() + APP_CAPTURE_AUDIO_BUFFER_SIZE + ANC_ASSIST_BUFF_SIZE;
    tota_buffer_size_used = 0;

    memset((uint8_t *)tota_buffer, 0, TOTA_BUFF_SIZE);
    TRACE(1,"[%s]capture syspool start addr: %p, %p.", __func__, syspool_start_addr(), tota_buffer);

    return 0;
}

uint32_t app_tota_mempool_free_buff_size()
{
    return TOTA_BUFF_SIZE - tota_buffer_size_used;
}

int app_tota_mempool_get_buff(uint8_t **buff, uint32_t size)
{
    uint32_t buff_size_free;
    uint8_t* capture_buf_addr = (uint8_t *)tota_buffer;
    buff_size_free = app_tota_mempool_free_buff_size();

    if (size % 4){
        size = size + (4 - size % 4);
    }

    TRACE(2,"[%s]Get capture buf, current free %d to allocate %d", __func__, buff_size_free, size);

    ASSERT(size <= buff_size_free, "[%s] size = %d > free size = %d", __func__, size, buff_size_free);

    *buff = capture_buf_addr + tota_buffer_size_used;

    tota_buffer_size_used += size;
    TRACE(3,"[%s]Allocate %d, now used %d left %d",
        __func__, size, tota_buffer_size_used, app_tota_mempool_free_buff_size());

    return 0;
}
#endif

#if defined(APP_ANC_TEST)
static uint32_t anc_test_buffer_size_used;
static uint8_t *anc_test_buffer = NULL;
int app_anc_test_mempool_init(uint32_t buff_size)
{
    ANC_TEST_BUFF_SIZE = buff_size;
    TRACE(1,"[%s]init the app anc test mem pool size %d.", __func__, ANC_TEST_BUFF_SIZE);
    anc_test_buffer =  syspool_start_addr() + app_audio_mempool_size() + 
                        APP_CAPTURE_AUDIO_BUFFER_SIZE + ANC_ASSIST_BUFF_SIZE + TOTA_BUFF_SIZE;
    anc_test_buffer_size_used = 0;

    memset((uint8_t *)anc_test_buffer, 0, ANC_TEST_BUFF_SIZE);
    TRACE(1,"[%s]capture syspool start addr: %p, %p.", __func__, syspool_start_addr(), anc_test_buffer);

    return 0;
}

uint32_t app_anc_test_mempool_free_buff_size()
{
    return ANC_TEST_BUFF_SIZE - anc_test_buffer_size_used;
}

int app_anc_test_mempool_get_buff(uint8_t **buff, uint32_t size)
{
    uint32_t buff_size_free;
    uint8_t* capture_buf_addr = (uint8_t *)anc_test_buffer;
    buff_size_free = app_anc_test_mempool_free_buff_size();

    if (size % 4){
        size = size + (4 - size % 4);
    }

    TRACE(2,"[%s]Get capture buf, current free %d to allocate %d", __func__, buff_size_free, size);

    ASSERT(size <= buff_size_free, "[%s] size = %d > free size = %d", __func__, size, buff_size_free);

    *buff = capture_buf_addr + anc_test_buffer_size_used;

    anc_test_buffer_size_used += size;
    TRACE(3,"[%s]Allocate %d, now used %d left %d",
        __func__, size, anc_test_buffer_size_used, app_anc_test_mempool_free_buff_size());

    return 0;
}
#endif



#if (defined(PLAYBACK_USE_I2S) || defined(FREEMAN_ENABLED_STERO))
#define APP_MEDIA_BUFFER_SIZE (11*1024)
#elif (PROMPT_USE_AAC) 
#define APP_MEDIA_BUFFER_SIZE (50*1024)
#else
//SBC_Original need 5888, SBC Reduce need 5888+2424
#define APP_MEDIA_BUFFER_SIZE (5888+2424)
#endif

static uint8_t media_buffer[APP_MEDIA_BUFFER_SIZE] __attribute__((aligned(4)));

static uint32_t media_buff_size_used;

app_ble_audio_callback_t g_app_ble_audio_start_cb = NULL;
app_ble_audio_callback_t g_app_ble_audio_stop_cb = NULL;
app_ble_audio_callback_t g_app_ble_audio_stop_single_stream_cb = NULL;

int app_media_mempool_init(void)
{
    TRACE(1, "init the app media mem pool size %d.", APP_MEDIA_BUFFER_SIZE);
    media_buff_size_used = 0;
    memset((uint8_t *)media_buffer, 0, APP_MEDIA_BUFFER_SIZE);

    return 0;
}

uint32_t app_media_mempool_free_buff_size()
{
    return APP_MEDIA_BUFFER_SIZE - media_buff_size_used;
}

int app_media_mempool_get_buff(uint8_t **buff, uint32_t size)
{
    uint32_t buff_size_free;
    uint8_t* media_buf_addr = (uint8_t *)media_buffer;
    buff_size_free = app_media_mempool_free_buff_size();

    if (size % 4){
        size = size + (4 - size % 4);
    }

    TRACE(2,"Get media buf, current free %d to allocate %d", buff_size_free, size);

    ASSERT(size <= buff_size_free, "[%s] size = %d > free size = %d", __func__, size, buff_size_free);

    *buff = media_buf_addr + media_buff_size_used;

    media_buff_size_used += size;
    TRACE(3,"Allocate %d, now used %d left %d",
        size, media_buff_size_used, app_media_mempool_free_buff_size());

    return 0;
}

osPoolDef (app_audio_status_mempool, 20, APP_AUDIO_STATUS);
osPoolId   app_audio_status_mempool = NULL;

// control queue access
osMutexId g_app_audio_queue_mutex_id = NULL;
osMutexDef(g_app_audio_queue_mutex);

// control pcmbuff access
static CQueue app_audio_pcm_queue;
static osMutexId app_audio_pcmbuff_mutex_id = NULL;
osMutexDef(app_audio_pcmbuff_mutex);

#ifdef __AUDIO_QUEUE_SUPPORT__
#define DEBUG_AUDIO
#ifdef DEBUG_AUDIO
#define debug_trace TRACE
#else
#define debug_trace(...)
#endif

typedef struct {
  list_t *audio_list;
}APP_AUDIO_CONFIG;

APP_AUDIO_CONFIG app_audio_conifg = {
    .audio_list = NULL
};

#endif

void LOCK_APP_AUDIO_QUEUE()
{
    osMutexWait(g_app_audio_queue_mutex_id, osWaitForever);
}

void UNLOCK_APP_AUDIO_QUEUE()
{
    osMutexRelease(g_app_audio_queue_mutex_id);
}

uint32_t app_audio_lr_balance(uint8_t *buf, uint32_t len, int8_t balance)
{
	short *balance_buf=(short *)buf;
	uint32_t balance_len = len/2;
	float factor;

	ASSERT((balance >= -100) && (balance <= 100), "balance = %d is invalid!", balance);

	if(balance > 0)
	{
		//reduce L channel
		factor = 1 - 0.01 * balance;
		for(uint32_t i=0; i<balance_len;i+=2)
		{
			balance_buf[i] = (short) (factor * balance_buf[i]);
		}
	}
	else if(balance < 0)
	{
		//reduce R channel
		factor = 1 + 0.01 * balance;
		for(uint32_t i=0; i<balance_len;i+=2)
		{
			balance_buf[i+1] = (short) (factor * balance_buf[i+1]);
		}
	}
    return 0;
}

void app_audio_mempool_init_with_specific_size(uint32_t size)
{
	syspool_init_specific_size(size);
}

uint32_t app_audio_mempool_original_size(void)
{
    return syspool_original_size();
}

uint32_t app_audio_mempool_size(void)
{
    return syspool_original_size() - APP_CAPTURE_AUDIO_BUFFER_SIZE - ANC_ASSIST_BUFF_SIZE - TOTA_BUFF_SIZE - ANC_TEST_BUFF_SIZE;
}

int app_audio_pcmbuff_init(uint8_t *buff, uint16_t len)
{
    if (app_audio_pcmbuff_mutex_id == NULL)
        app_audio_pcmbuff_mutex_id = osMutexCreate((osMutex(app_audio_pcmbuff_mutex)));

    if ((buff == NULL)||(app_audio_pcmbuff_mutex_id == NULL))
        return -1;

    osMutexWait(app_audio_pcmbuff_mutex_id, osWaitForever);
    InitCQueue(&app_audio_pcm_queue, len, buff);
    memset(buff, 0x00, len);
    osMutexRelease(app_audio_pcmbuff_mutex_id);

    return 0;
}

int app_audio_pcmbuff_space(void)
{
    int len;

    osMutexWait(app_audio_pcmbuff_mutex_id, osWaitForever);
    len = AvailableOfCQueue(&app_audio_pcm_queue);
    osMutexRelease(app_audio_pcmbuff_mutex_id);

    return len;
}

int app_audio_pcmbuff_length(void)
{
    int len;

    osMutexWait(app_audio_pcmbuff_mutex_id, osWaitForever);
    len = LengthOfCQueue(&app_audio_pcm_queue);
    osMutexRelease(app_audio_pcmbuff_mutex_id);

    return len;
}

int app_audio_pcmbuff_put(uint8_t *buff, uint16_t len)
{
    int status;

    osMutexWait(app_audio_pcmbuff_mutex_id, osWaitForever);
    status = EnCQueue(&app_audio_pcm_queue, buff, len);
    osMutexRelease(app_audio_pcmbuff_mutex_id);

    return status;
}

int app_audio_pcmbuff_get(uint8_t *buff, uint16_t len)
{
    unsigned char *e1 = NULL, *e2 = NULL;
    unsigned int len1 = 0, len2 = 0;
    int status;

    osMutexWait(app_audio_pcmbuff_mutex_id, osWaitForever);
    status = PeekCQueue(&app_audio_pcm_queue, len, &e1, &len1, &e2, &len2);
    if (len==(len1+len2)){
        memcpy(buff,e1,len1);
        memcpy(buff+len1,e2,len2);
        DeCQueue(&app_audio_pcm_queue, 0, len1);
        DeCQueue(&app_audio_pcm_queue, 0, len2);
    }else{
        memset(buff, 0x00, len);
        status = -1;
    }
    osMutexRelease(app_audio_pcmbuff_mutex_id);

    return status;
}

int app_audio_pcmbuff_discard(uint16_t len)
{
    int status;

    osMutexWait(app_audio_pcmbuff_mutex_id, osWaitForever);
    status = DeCQueue(&app_audio_pcm_queue, 0, len);
    osMutexRelease(app_audio_pcmbuff_mutex_id);

    return status;
}

FRAM_TEXT_LOC
void app_audio_memcpy_16bit(int16_t *des, int16_t *src, int len)
{
    // Check input
    for(int i=0; i<len; i++)
    {
        des[i] = src[i];
    }
}

FRAM_TEXT_LOC
void app_audio_memset_16bit(int16_t *des, int16_t val, int len)
{
    // Check input
    for(int i=0; i<len; i++)
    {
        des[i] = val;
    }
}

#ifdef __AUDIO_QUEUE_SUPPORT__
#if defined(USE_BASIC_THREADS)
int app_audio_sendrequest_param(uint16_t id, uint8_t status, uint32_t ptr, uint32_t param)
{
    uint32_t audevt;
    APP_MESSAGE_BLOCK msg;

    TRACE(7,"%s ca=%p type %d%s op %d%s aud %d", __func__, __builtin_return_address(0),
                id, player2str(id), status, audio_op_to_str((enum APP_BT_SETTING_T)status), ptr);

    if(app_audio_init == false)
        return -1;

    msg.mod_id = APP_MODULE_AUDIO;
    msg.mod_level = APP_MOD_LEVEL_3;
    APP_AUDIO_SET_MESSAGE(audevt, id, status);
    msg.msg_body.message_id = audevt;
    msg.msg_body.message_ptr = ptr;
    msg.msg_body.message_Param0 = param;
    app_mailbox_put(&msg);

    return 0;
}
#else
int app_audio_sendrequest_param(uint16_t id, uint8_t status, uint32_t ptr, uint32_t param)
{
    uint32_t audevt;
    APP_AUDIO_MESSAGE_BLOCK msg;

    TRACE(7,"%s ca=%p type %d%s op %d%s aud %d", __func__, __builtin_return_address(0),
                id, player2str(id), status, audio_op_to_str((enum APP_BT_SETTING_T)status), ptr);

    if(app_audio_init == false)
        return -1;

    msg.mod_id = APP_AUDIO_MODULE_AUDIO;
    APP_AUDIO_SET_MESSAGE(audevt, id, status);
    msg.msg_body.message_id = audevt;
    msg.msg_body.message_ptr = ptr;
    msg.msg_body.message_Param0 = param;
    app_audio_mailbox_put(&msg);

    return 0;
}
#endif

extern bool app_audio_list_playback_exist(void);
#if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))
static uint8_t app_audio_get_list_playback_num(void)
{
    APP_AUDIO_STATUS *audio_handle = NULL;
    list_node_t *node = NULL;
    uint8_t num=0;
    for (node = list_begin(app_audio_conifg.audio_list); node != list_end(app_audio_conifg.audio_list); node = list_next(node))
    {
        audio_handle = (APP_AUDIO_STATUS *)list_node(node);
        if (audio_handle->id == APP_PLAY_BACK_AUDIO)
            num++;
    }
    return num;
}
#endif

#endif

static bool need_flush_flash_switch_audio = false;
void app_audio_switch_flash_flush_req(void)
{
    uint32_t lock;

    lock = int_lock();
    need_flush_flash_switch_audio = true;
    int_unlock(lock);
}

static void app_audio_switch_flash_proc(void)
{
    uint32_t lock;
    bool need_flush_flash = false;

    lock = int_lock();
    if (need_flush_flash_switch_audio){
        need_flush_flash_switch_audio = false;
        need_flush_flash = true;
    }
    int_unlock(lock);

    if (need_flush_flash){
#ifndef FPGA
        nv_record_flash_flush();
#endif
    }
}

#if defined(USE_BASIC_THREADS)
static int app_audio_handle_process(APP_MESSAGE_BODY *msg_body)
#else
static int app_audio_handle_process(APP_AUDIO_MESSAGE_BODY *msg_body)
#endif
{
    int nRet = 0;

    APP_AUDIO_STATUS aud_status = {0};
    POSSIBLY_UNUSED APP_AUDIO_STATUS next_status = {0};

    if (app_audio_init == false)
        return -1;

    APP_AUDIO_GET_ID(msg_body->message_id, aud_status.id);
    APP_AUDIO_GET_STATUS(msg_body->message_id, aud_status.status);
    APP_AUDIO_GET_AUD_ID(msg_body->message_ptr, aud_status.aud_id);
    APP_AUDIO_GET_FREQ(msg_body->message_Param0, aud_status.freq);

    switch (aud_status.status)
    {
    case APP_BT_SETTING_OPEN:
#ifdef __AUDIO_QUEUE_SUPPORT__
        debug_trace(4,
                    "=======>APP_BT_SETTING_OPEN,##before status_id: 0x%x%s, aud_id: 0x%x, len = %d",
                    aud_status.id,
                    player2str(aud_status.id),
                    aud_status.aud_id,
                    list_length(app_audio_conifg.audio_list));

        if (app_audio_list_append(&aud_status))
        {
            app_bt_stream_open(&aud_status);
            debug_trace(3,
                        "=======>APP_BT_SETTING_OPEN, ##after status_id: 0x%x%s, len = %d",
                        aud_status.id,
                        player2str(aud_status.id),
                        list_length(app_audio_conifg.audio_list));
            if(aud_status.id == APP_BT_STREAM_HFP_PCM)
            {
                app_bt_media_set_current_media(BT_STREAM_VOICE);
            }
        }
        else if(!app_bt_stream_isrun(APP_PLAY_BACK_AUDIO))
        {
            app_bt_stream_open(&aud_status);
        }
#else
        app_bt_stream_open(&aud_status);
#endif

        break;
    case APP_BT_SETTING_CLOSE:
        app_audio_switch_flash_proc();
#ifdef __AUDIO_QUEUE_SUPPORT__
        debug_trace(2,
                    "=======>APP_BT_SETTING_CLOSE, current id: 0x%x%s",
                    aud_status.id,
                    player2str(aud_status.id));
        app_bt_stream_close(aud_status.id);
        app_audio_switch_flash_proc();
#if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))
        if (aud_status.id == APP_PLAY_BACK_AUDIO)
        {
            debug_trace(1, "=======>APP_BT_SETTING_CLOSE, list: %d", app_audio_get_list_playback_num());
            if (app_audio_get_list_playback_num() == 1)
            {
                debug_trace(0, "=======>APP_BT_SETTING_CLOSE MEDIA");
                bt_media_stop(BT_STREAM_MEDIA, BT_DEVICE_ID_1, aud_status.aud_id);
            }
            app_prompt_inform_completed_event();
        }
#endif
        if (app_audio_list_rmv_callback(&aud_status, &next_status, APP_BT_SETTING_Q_POS_HEAD))
        {
            debug_trace(4,
                        "=======>APP_BT_SETTING_CLOSE, %p, next id: 0x%x%s, status %d",
                        &next_status,
                        next_status.id,
                        player2str(next_status.id),
                        next_status.status);
#ifdef __AI_VOICE__
            if (app_ai_voice_is_need2block_a2dp() &&
                ((APP_BT_STREAM_A2DP_SBC == next_status.id) || (APP_BT_STREAM_A2DP_AAC == next_status.id)))
            {
            }
            else
#endif
            {
                app_bt_stream_open(&next_status);
            }
        }
#else
        app_bt_stream_close(aud_status.id);
        app_audio_switch_flash_proc();
#endif
        break;
    case APP_BT_SETTING_SETUP:
        app_bt_stream_setup(aud_status.id, msg_body->message_ptr);
        break;
    case APP_BT_SETTING_RESTART:
        app_bt_stream_restart(&aud_status);
        break;
    case APP_BT_SETTING_CLOSEALL:
        app_bt_stream_closeall();
#ifdef __AUDIO_QUEUE_SUPPORT__
        app_audio_list_clear();
#endif
#if defined(MEDIA_PLAYER_SUPPORT)
        app_prompt_stop_all();
#endif
        app_audio_switch_flash_proc();
        break;
#if defined(MEDIA_PLAYER_SUPPORT)
    case APP_BT_SETTING_START_PROMPT:
    {
        app_play_audio_onoff(true, &aud_status);
        break;
    }
    case APP_BT_SETTING_STOP_PROMPT:
    {
        app_play_audio_onoff(false, &aud_status);
        app_prompt_inform_completed_event();
        break;
    }
#endif
    case APP_BT_SETTING_START_BLEAUDIO:
    if (g_app_ble_audio_start_cb)
    {
        g_app_ble_audio_start_cb((uint8_t)aud_status.id);
    } break;
    case APP_BT_SETTING_STOP_BLEAUDIO:
    if (g_app_ble_audio_stop_cb)
    {
        g_app_ble_audio_stop_cb((uint8_t)aud_status.id);
    } break;
    case APP_BT_SETTING_STOP_SINGLE_STREAM:
    if (g_app_ble_audio_stop_single_stream_cb)
    {
        g_app_ble_audio_stop_single_stream_cb((uint8_t)aud_status.id);
    } break;
    case APP_BT_SETTING_OUTPUT_ROUTE:
    {
        app_bt_stream_set_output_route((void *)msg_body->message_ptr);
    } break;
    default:
        break;
    }

    return nRet;
}

#ifdef __AUDIO_QUEUE_SUPPORT__
#if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))
//go through the audio list to find out the first stream which is not AI_VOICE
static bool app_audio_list_find_next_available_id(uint16_t audio_id, APP_AUDIO_STATUS *audioId)
{
    APP_AUDIO_STATUS *audio_handle = NULL;
    list_node_t *node = NULL;

    for (node = list_begin(app_audio_conifg.audio_list); node != list_end(app_audio_conifg.audio_list); node = list_next(node))
    {
        audio_handle = (APP_AUDIO_STATUS *)list_node(node);
        if (audio_id == audio_handle->id)
        {
            *audioId = *audio_handle;
            return true;
        }
    }

    return false;
}
#endif

static void app_audio_handle_free(void* data)
{
#if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))
    APP_AUDIO_STATUS * status = (APP_AUDIO_STATUS * )data;

    if(status->id == APP_PLAY_BACK_AUDIO)
    {
        debug_trace(2,"=======>app_audio_handle_free playback , aud_id: 0x%x, type = 0x%x", status->aud_id, status->aud_type);
    }
#endif
    osPoolFree (app_audio_status_mempool, data);
}

void app_audio_list_create()
{
    if (app_audio_conifg.audio_list == NULL)
    {
        app_audio_conifg.audio_list = list_new(app_audio_handle_free, NULL, NULL);
    }
}

bool app_audio_list_stream_exist()
{
    APP_AUDIO_STATUS *audio_handle = NULL;
    list_node_t *node = NULL;

    for (node = list_begin(app_audio_conifg.audio_list); node != list_end(app_audio_conifg.audio_list); node = list_next(node)) {

        audio_handle = (APP_AUDIO_STATUS *)list_node(node);

        if (audio_handle->id == APP_BT_STREAM_HFP_PCM ||
            audio_handle->id == APP_BT_STREAM_HFP_CVSD ||
            audio_handle->id == APP_BT_STREAM_HFP_VENDOR ||
            audio_handle->id == APP_BT_STREAM_A2DP_SBC ||
            audio_handle->id == APP_BT_STREAM_A2DP_AAC ||
#ifdef __AI_VOICE__
            audio_handle->id == APP_BT_STREAM_AI_VOICE ||
#endif
#ifdef AUDIO_LINEIN
            audio_handle->id == APP_PLAY_LINEIN_AUDIO ||
#endif
            audio_handle->id == APP_BT_STREAM_A2DP_VENDOR
        ) {
            debug_trace(3,"%s id 0x%x%s", __func__, audio_handle->id, player2str(audio_handle->id));
            return true;
        }
    }

    return false;
}

bool app_audio_list_filter_exist(APP_AUDIO_STATUS* aud_status)
{
#if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))
    APP_AUDIO_STATUS *audio_handle = NULL;
    list_node_t *node = NULL;
    uint8_t cnt = 0;

    if (aud_status->id != APP_PLAY_BACK_AUDIO){
        return false;
    }

    if(aud_status->aud_id == AUD_ID_BT_CALL_INCOMING_CALL) {
        for (node = list_begin(app_audio_conifg.audio_list);
             node != list_end(app_audio_conifg.audio_list);
             node = list_next(node)) {
            audio_handle = (APP_AUDIO_STATUS *)list_node(node);
            if (audio_handle->id == APP_PLAY_BACK_AUDIO
                && audio_handle->aud_id == AUD_ID_BT_CALL_INCOMING_CALL) {
                debug_trace(3,"%s id 0x%x%s", __func__, audio_handle->id, player2str(audio_handle->id));
                return true;
            }
        }
    } else {
        for (node = list_begin(app_audio_conifg.audio_list);
             node != list_end(app_audio_conifg.audio_list);
             node = list_next(node)) {
            audio_handle = (APP_AUDIO_STATUS *)list_node(node);
            if (cnt++ > 1) {
                debug_trace(2,"%s cnt %d", __func__, cnt);
                return true;
            }
        }
    }
#endif
    return false;
}

bool app_audio_list_playback_exist(void)
{
#if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))
    APP_AUDIO_STATUS *audio_handle = NULL;
    list_node_t *node = NULL;

    for (node = list_begin(app_audio_conifg.audio_list);
         node != list_end(app_audio_conifg.audio_list);
         node = list_next(node)) {
        audio_handle = (APP_AUDIO_STATUS *)list_node(node);
#if defined(A2DP_SOURCE_TEST) && defined(APP_LINEIN_A2DP_SOURCE)
        if ((audio_handle->id == APP_PLAY_BACK_AUDIO)
            || (audio_handle->id == APP_A2DP_SOURCE_LINEIN_AUDIO)) {
#else
        if (audio_handle->id == APP_PLAY_BACK_AUDIO) {
#endif
            debug_trace(1,"%s", __func__);
            return true;
        }
    }
#endif
    return false;
}

void app_audio_list_playback_clear(void)
{
#if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))
    APP_AUDIO_STATUS *audio_handle = NULL;
    list_node_t *node = NULL;

    for (node = list_begin(app_audio_conifg.audio_list);
         node != list_end(app_audio_conifg.audio_list);
         node = list_next(node)) {
        audio_handle = (APP_AUDIO_STATUS *)list_node(node);
        if (audio_handle->id == APP_PLAY_BACK_AUDIO){
            list_remove(app_audio_conifg.audio_list, list_node(node));
        }
    }
#endif
}

bool app_audio_list_append(APP_AUDIO_STATUS* aud_status)
{
    APP_AUDIO_STATUS *data_to_append = NULL;
    bool add_data_to_head_of_list = false;
    bool ret = true;

    debug_trace(3,"%s id 0x%x%s", __func__, aud_status->id, player2str(aud_status->id));
#if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))
#if defined(A2DP_SOURCE_TEST) && defined(APP_LINEIN_A2DP_SOURCE)
    if ((aud_status->id == APP_PLAY_BACK_AUDIO) || (aud_status->id == APP_A2DP_SOURCE_LINEIN_AUDIO))
#else
    if (aud_status->id == APP_PLAY_BACK_AUDIO)
#endif
    {
        //ignore redundant ring ind from hfp...
        if (app_audio_list_filter_exist(aud_status))
        {
            return false;
        }

        if (app_audio_list_playback_exist())
        {
            if (list_length(app_audio_conifg.audio_list) >= MAX_AUDIO_BUF_LIST)
            {
                if (app_audio_list_stream_exist())
                {
                    data_to_append = (APP_AUDIO_STATUS *)osPoolCAlloc (app_audio_status_mempool);
                    memcpy(data_to_append, (const void *)list_front(app_audio_conifg.audio_list), sizeof(APP_AUDIO_STATUS));
                    add_data_to_head_of_list = true;
                }

                app_audio_list_clear();
                app_prompt_stop_all();
                TRACE(0,"#####app_audio_list_append error!!! FIXME!!!!\n ");
            }
            else
            {
                ret = false;
            }
        }
    }
    else
#endif
    {
        add_data_to_head_of_list = true;
    }

    if (data_to_append == NULL)
    {
        data_to_append = (APP_AUDIO_STATUS *)osPoolCAlloc (app_audio_status_mempool);
        memcpy(data_to_append, aud_status, sizeof(APP_AUDIO_STATUS));
    }

    if (add_data_to_head_of_list)
    {
        list_prepend(app_audio_conifg.audio_list, (void*)data_to_append);
    }
    else
    {
        list_append(app_audio_conifg.audio_list, (void*)data_to_append);
    }
    if (data_to_append)
    {
        debug_trace(7,"%s id 0x%x%s status %d len %d ret %d aud_id %p",
            __func__, data_to_append->id, player2str(data_to_append->id), \
            data_to_append->status, list_length(app_audio_conifg.audio_list),\
            ret, data_to_append);
    }
    return ret;
}

bool app_audio_list_rmv_callback(APP_AUDIO_STATUS *status_close, APP_AUDIO_STATUS *status_next, enum APP_BT_AUDIO_Q_POS pos)
{
    void *data_to_remove = NULL;
    bool ret = false;

    //for status: first bt_a2dp->APP_BT_SETTING_CLOSE,then ring-> APP_BT_SETTING_CLOSE
    debug_trace(3, "pre audio list len %d close_id 0x%x%s, pos:%d",
                list_length(app_audio_conifg.audio_list),
                status_close->id,
                player2str(status_close->id), pos);

    if(0 == list_length(app_audio_conifg.audio_list))
    {
        debug_trace(0, "no listed audio anymore.");
        return false;
    }

    POSSIBLY_UNUSED APP_AUDIO_STATUS *audio_handle = NULL;
    POSSIBLY_UNUSED list_node_t *node = NULL;
#if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))
    if (APP_PLAY_BACK_AUDIO == status_close->id)
    {
        // 1. find the first prompt and remove it
        for (node = list_begin(app_audio_conifg.audio_list); node != list_end(app_audio_conifg.audio_list); node = list_next(node))
        {
            audio_handle = (APP_AUDIO_STATUS *)list_node(node);
            debug_trace(2, "Aud list, ID:0x%x%s", audio_handle->id, player2str(audio_handle->id));

            if (APP_PLAY_BACK_AUDIO == audio_handle->id)
            {
                data_to_remove = list_node(node);
                list_remove(app_audio_conifg.audio_list, data_to_remove);
                break;
            }
        }

        // 2. find the next audio entry to play
        // 2.1 find the prompt
        ret = app_audio_list_find_next_available_id(APP_PLAY_BACK_AUDIO, status_next);

        // 2.2 if no prompt, find music
        if (!ret)
        {
            ret = app_audio_list_find_next_available_id(APP_BT_STREAM_A2DP_SBC, status_next);
        }

        // 2.3 otherwise, just pop the next one no matter what's it
        if (!ret)
        {
            if (list_length(app_audio_conifg.audio_list) > 0)
            {
                node = list_begin(app_audio_conifg.audio_list);
                if (node)
                {
                    memcpy(status_next, list_node(node), sizeof(APP_AUDIO_STATUS));
                }
                ret = true;
            }
        }

        // 2.4 in case music was not pushed into audio list for corner case,
        //     check whether music is in active media list while call is not
        if (!ret)
        {
            if (bt_media_is_media_active_by_music() &&
                (!bt_media_is_media_active_by_call()))
            {
                TRACE(0, "music in active list but not pushed into audio list for some reason!!!");
                status_next->id = APP_BT_STREAM_A2DP_SBC;
                ret = true;
            }
        }
    }
    else //maybe...a2dp send >> APP_BT_SETTING_CLOSE,  when ring
#endif
    {
        if (app_audio_list_stream_exist())
        {
#ifdef __AI_VOICE__
            if (APP_BT_STREAM_AI_VOICE == status_close->id)
            {
                for (node = list_begin(app_audio_conifg.audio_list);
                     node != list_end(app_audio_conifg.audio_list);
                     node = list_next(node)) {
                    audio_handle = (APP_AUDIO_STATUS *)list_node(node);
                    debug_trace(2, "Aud list, ID:0x%x%s", audio_handle->id, player2str(audio_handle->id));

                    if (APP_BT_STREAM_AI_VOICE == audio_handle->id)
                    {
                        data_to_remove = list_node(node);
                    }
                }
            }
            else
#endif
            {
                if (APP_BT_SETTING_Q_POS_HEAD == pos)
                {
                    data_to_remove = list_front(app_audio_conifg.audio_list);
                }
                else if (APP_BT_SETTING_Q_POS_TAIL == pos)
                {
                    data_to_remove = list_back(app_audio_conifg.audio_list);
                }
            }
        }

        if (data_to_remove)
        {
            list_remove(app_audio_conifg.audio_list, data_to_remove);
        }
    }

    debug_trace(3,"Updated audio list len = %d ret %d data %p", list_length(app_audio_conifg.audio_list), ret, data_to_remove);
    return ret;
}

void app_audio_list_rmv_by_info(APP_AUDIO_STATUS *info)
{
    TRACE(4, "%s id:0x%x%s list_len:%d", __func__, info->id, player2str(info->id), list_length(app_audio_conifg.audio_list));

    list_node_t *node = NULL, *next = NULL;
    APP_AUDIO_STATUS *nodeInfo = NULL;
    for (node = list_begin(app_audio_conifg.audio_list); node != list_end(app_audio_conifg.audio_list); )
    {
        next = list_next(node);

        nodeInfo = (APP_AUDIO_STATUS *)list_node(node);

        if (nodeInfo)
        {
            TRACE(3, "%s listed id:0x%x, aud_id:%p, list_len:%d", __func__,
                    nodeInfo->id, nodeInfo, list_length(app_audio_conifg.audio_list));
            // TODO: add more info check here
            if ((info->id == nodeInfo->id))
            {
                debug_trace(3, "%s remove:0x%x%s", __func__, info->id, player2str(info->id));
                list_remove(app_audio_conifg.audio_list, nodeInfo);
                break;
            }
        }
        else
        {
            TRACE(0, "ERROR: null pointer detected!!!");
            break;
        }

        node = next;
    }
}

void app_audio_list_clear()
{
    list_clear(app_audio_conifg.audio_list);
}
#endif

void app_audio_open(void)
{
    if(app_audio_init)
    {
        return;
    }
    if (g_app_audio_queue_mutex_id == NULL)
    {
        g_app_audio_queue_mutex_id = osMutexCreate((osMutex(g_app_audio_queue_mutex)));
    }
    else
    {
        ASSERT(0, "[%s] ERROR: g_app_audio_queue_mutex_id != NULL", __func__);
    }

    if (app_audio_status_mempool == NULL)
        app_audio_status_mempool = osPoolCreate(osPool(app_audio_status_mempool));
    ASSERT(app_audio_status_mempool, "[%s] ERROR: app_audio_status_mempool != NULL", __func__);

#ifdef __AUDIO_QUEUE_SUPPORT__
    app_audio_list_create();
#endif
#ifdef MEDIA_PLAYER_SUPPORT
    app_prompt_list_init();
#endif

#ifndef PROMPT_SELF_MANAGEMENT 
    app_ring_merge_init();
#endif

#if defined(USE_BASIC_THREADS)
    app_set_threadhandle(APP_MODULE_AUDIO, app_audio_handle_process);
#else
    app_audio_thread_init();

    app_audio_set_threadhandle(APP_AUDIO_MODULE_AUDIO, app_audio_handle_process);
#endif

    app_bt_stream_init();

    app_audio_init = true;
}

void app_audio_close(void)
{
#if defined(USE_BASIC_THREADS)
    app_set_threadhandle(APP_MODULE_AUDIO, NULL);
#else
    app_audio_set_threadhandle(APP_AUDIO_MODULE_AUDIO, NULL);
#endif
    app_audio_init = false;
}

void app_ble_audio_register_start_callback(app_ble_audio_callback_t cb)
{
    g_app_ble_audio_start_cb = cb;
}

void app_ble_audio_register_stop_callback(app_ble_audio_callback_t cb)
{
    g_app_ble_audio_stop_cb = cb;
}

void app_ble_audio_register_stop_single_stream_callback(app_ble_audio_callback_t cb)
{
    g_app_ble_audio_stop_single_stream_cb = cb;
}
