/**
 * @file aob_ux_stm.cpp
 * @author BES AI team
 * @version 0.1
 * @date 2020-08-31
 *
 * @copyright Copyright (c) 2015-2021 BES Technic.
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

/*****************************header include********************************/
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_aud.h"
#include "bt_drv_reg_op.h"
#include "audioflinger.h"
#include "bt_drv_interface.h"
#include "app_trace_rx.h"
#include "bluetooth_bt_api.h"
#include "besbt_string.h"
#include "app_bt_func.h"
#include "plat_types.h"
#include "heap_api.h"

#include "gaf_media_common.h"
#include "gaf_media_sync.h"
#include "gaf_stream_dbg.h"
#include "gaf_codec_lc3.h"
#include "rwble_config.h"
#include "gaf_codec_cc_bth.h"
#ifdef GAF_DECODER_CROSS_CORE_USE_M55
#include "gaf_codec_cc_common.h"
#endif

#include "app_tws_ctrl_thread.h"
#include "app_tws_ibrt_conn_api.h"
#include "app_bt_sync.h"

#include "bes_aob_api.h"

/*********************external function declaration*************************/

/************************private macro defination***************************/
#define GAF_STREAM_HEAP_DEBUG 0
#define GAF_STREAM_TRIGGER_TIMEROUT 3000
#define GAF_STREAM_MAX_RETRIGGER_TIMES 2
#define GAF_STREAM_INVALID_CLOCK_CNT 0xFFFFFFFF
#define GAF_STREAM_INVALID_US_SINCE_LAST_ANCHOR_POINT 0xFFFF

#define GAF_AUDIO_USE_ONE_STREAM_ONLY_ENABLED     (true)

/************************private type defination****************************/
typedef struct
{
    uint8_t  streamContext;
    uint32_t master_clk_cnt;
    uint16_t master_bit_cnt;	
    int32_t  usSinceLatestAnchorPoint;
    uint32_t triggertimeUs;
    uint8_t  reserve[6];
} AOB_TWS_SYNC_TRIGGER_CAPTURE_INFO_T;

typedef struct
{
    int32_t  usSinceLatestAnchorPoint;
    uint8_t  streamContext;	
    uint8_t  reserve[7];
} AOB_TWS_SYNC_US_SINCE_LATEST_ANCHOR_POINT_T;

/**********************private function declaration*************************/

/************************private variable defination************************/
static GAF_ADUIO_STREAM_RETRIGGER gaf_audio_stream_retrigger_cb = NULL;
static GAF_ADUIO_STREAM_RETRIGGER gaf_mobile_audio_stream_retrigger_cb = NULL;
static GAF_AUDIO_STREAM_ENV_T *gaf_audio_running_stream_ref[GAF_MAXIMUM_CONNECTION_COUNT] = {NULL};
static uint8_t max_playback_retrigger_times = 0;
static uint8_t max_capture_retrigger_times = 0;
static bool gaf_is_doing_prefill = false;

/// PresDelay in us set by custom
static uint32_t gaf_stream_custom_presdelay_us = 0;

static void playback_trigger_supervisor_timer_cb(void const *param);
osTimerDef(GAF_STREAM_PLAYBACK_TRIGGER_TIMEOUT, playback_trigger_supervisor_timer_cb);

static void capture_trigger_supervisor_timer_cb(void const *param);
osTimerDef(GAF_STREAM_CAPTURE_TRIGGER_TIMEOUT, capture_trigger_supervisor_timer_cb);

static heap_handle_t codec_data_heap;
extern "C" uint32_t btdrv_reg_op_cig_anchor_timestamp(uint8_t link_id);
/**********************************GAF CUSTOM******************************/
static GAF_STREAM_COMMON_CUSTOM_DATA_HANDLER_FUNC_T gaf_stream_common_custom_data_func_table[GAF_STREAM_USER_CASE_MAX] = {{NULL}};

/****************************function defination****************************/
void gaf_stream_register_retrigger_callback(GAF_ADUIO_STREAM_RETRIGGER retrigger_cb)
{
    gaf_audio_stream_retrigger_cb = retrigger_cb;
}

void gaf_mobile_stream_register_retrigger_callback(GAF_ADUIO_STREAM_RETRIGGER retrigger_cb)
{
    gaf_mobile_audio_stream_retrigger_cb = retrigger_cb;
}

static void gaf_stream_retrigger_handler(GAF_AUDIO_STREAM_ENV_T* pStreamEnv)
{
    if (pStreamEnv->stream_info.is_mobile && gaf_mobile_audio_stream_retrigger_cb)
    {
        gaf_mobile_audio_stream_retrigger_cb((void *)pStreamEnv);
    }
    else if (gaf_audio_stream_retrigger_cb)
    {
        gaf_audio_stream_retrigger_cb((void *)pStreamEnv);
    }
}

static void playback_trigger_supervisor_timer_cb(void const *param)
{
    LOG_I("%s, gaf stream trigger timeout!", __func__);

    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T*)param;

    if (pStreamEnv)
    {
        pStreamEnv->stream_context.playback_retrigger_onprocess = true;
        app_bt_call_func_in_bt_thread((uint32_t)pStreamEnv, 0, 0, 0,
            (uint32_t) gaf_stream_retrigger_handler);
    }
}

static void capture_trigger_supervisor_timer_cb(void const *param)
{
    LOG_I("%s, gaf stream trigger timeout!", __func__);

    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T*)param;

    if (pStreamEnv)
    {
        app_bt_call_func_in_bt_thread((uint32_t)pStreamEnv, 0, 0, 0,
            (uint32_t) gaf_stream_retrigger_handler);
    }
}

uint8_t gaf_media_common_get_ase_chan_lid_from_iso_channel(GAF_AUDIO_STREAM_ENV_T* pStreamEnv,
                                                           uint8_t direction, uint8_t iso_channel)
{
    GAF_AUDIO_STREAM_COMMON_INFO_T *pCommonInfo = NULL;
    if (direction == BES_BLE_GAF_DIRECTION_SINK)
    {
        pCommonInfo = &pStreamEnv->stream_info.playbackInfo;
    }
    else
    {
        pCommonInfo = &pStreamEnv->stream_info.captureInfo;
    }

    for (uint8_t chn_lid = 0; chn_lid < GAF_AUDIO_ASE_TOTAL_COUNT; chn_lid++)
    {
        if (pCommonInfo->aseChInfo[chn_lid].iso_channel_hdl == iso_channel)
        {
            return chn_lid;
        }
    }

    return GAF_AUDIO_ASE_TOTAL_COUNT;
}

uint32_t gaf_media_common_get_latest_tx_iso_evt_timestamp(GAF_AUDIO_STREAM_ENV_T* pStreamEnv)
{
    for (uint8_t i = 0; i < GAF_AUDIO_ASE_TOTAL_COUNT; i++)
    {
        if (GAF_AUDIO_INVALID_ISO_CHANNEL != pStreamEnv->stream_info.captureInfo.aseChInfo[i].iso_channel_hdl)
        {
            return btdrv_reg_op_cig_anchor_timestamp(
                                        pStreamEnv->stream_info.captureInfo.aseChInfo[i].iso_channel_hdl);
        }
    }
    LOG_W("%s err!!!", __func__);
    return 0;
}

uint32_t gaf_media_common_get_latest_rx_iso_evt_timestamp(GAF_AUDIO_STREAM_ENV_T* pStreamEnv)
{
    for (uint8_t i = 0; i < GAF_AUDIO_ASE_TOTAL_COUNT; i++)
    {
        if (GAF_AUDIO_INVALID_ISO_CHANNEL != pStreamEnv->stream_info.playbackInfo.aseChInfo[i].iso_channel_hdl)
        {
            return btdrv_reg_op_cig_anchor_timestamp(
                                        pStreamEnv->stream_info.playbackInfo.aseChInfo[i].iso_channel_hdl);
        }
    }
    LOG_W("%s err!!!", __func__);
    return 0;
}

int gaf_stream_playback_trigger_checker_start(GAF_AUDIO_STREAM_ENV_T* pStreamEnv)
{
    if (max_playback_retrigger_times > GAF_STREAM_MAX_RETRIGGER_TIMES) {
        max_playback_retrigger_times = 0;
        return -1;
    }

    if (pStreamEnv->stream_context.playback_trigger_supervisor_timer_id)
    {
        LOG_I("%s", __func__);
        osTimerStart(pStreamEnv->stream_context.playback_trigger_supervisor_timer_id,
            GAF_STREAM_TRIGGER_TIMEROUT);
        max_playback_retrigger_times++;
    }
    return 0;
}

int gaf_stream_playback_trigger_checker_stop(GAF_AUDIO_STREAM_ENV_T* pStreamEnv)
{
    if (pStreamEnv->stream_context.playback_trigger_supervisor_timer_id != NULL)
    {
        pStreamEnv->stream_context.playback_retrigger_onprocess = false;
        osTimerStop(pStreamEnv->stream_context.playback_trigger_supervisor_timer_id);
    }
    return 0;
}

int gaf_stream_capture_trigger_checker_start(GAF_AUDIO_STREAM_ENV_T* pStreamEnv)
{
    if (max_capture_retrigger_times > GAF_STREAM_MAX_RETRIGGER_TIMES) {
        max_capture_retrigger_times = 0;
        return -1;
    }

    if (pStreamEnv->stream_context.capture_trigger_supervisor_timer_id)
    {
        LOG_I("%s", __func__);
        osTimerStart(pStreamEnv->stream_context.capture_trigger_supervisor_timer_id, GAF_STREAM_TRIGGER_TIMEROUT);
        max_capture_retrigger_times++;
    }
    return 0;
}

int gaf_stream_capture_trigger_checker_stop(GAF_AUDIO_STREAM_ENV_T* pStreamEnv)
{
    if (pStreamEnv->stream_context.capture_trigger_supervisor_timer_id != NULL)
    {
        osTimerStop(pStreamEnv->stream_context.capture_trigger_supervisor_timer_id);
    }
    return 0;
}

void gaf_stream_heap_init(void *begin_addr, uint32_t size)
{
    codec_data_heap = heap_register(begin_addr,size);
}

void *gaf_stream_heap_malloc(uint32_t size)
{
    size = ((size >> 2) + 1) << 2;
    void *ptr = heap_malloc(codec_data_heap, size);
    ASSERT(ptr, "%s size:%d", __func__, size);
#if GAF_STREAM_HEAP_DEBUG
    LOG_I("[%s] ptr=%p size=%u user=%p left/mini left=%u/%u",
        __func__, ptr, size, __builtin_return_address(0),
        heap_free_size(codec_data_heap),
        heap_minimum_free_size(codec_data_heap));
#endif
    return ptr;
}

void *gaf_stream_heap_cmalloc(uint32_t size)
{
    void *ptr = heap_malloc(codec_data_heap, size);
    ASSERT(ptr, "%s size:%d", __func__, size);
    memset(ptr, 0, size);
#if GAF_STREAM_HEAP_DEBUG
    LOG_I("[%s] ptr=%p size=%u user=%p left/mini left=%u/%u",
        __func__, ptr, size, __builtin_return_address(0),
        heap_free_size(codec_data_heap),
        heap_minimum_free_size(codec_data_heap));
#endif
    return ptr;
}


#ifdef GAF_CODEC_CROSS_CORE
void gaf_m55_stream_encoder_heap_free(void *rmem)
{
#if GAF_STREAM_HEAP_DEBUG
    LOG_I("[%s] ptr=%p user=%p", __func__, rmem, __builtin_return_address(0));
#endif
    ASSERT(rmem, "%s rmem:%p", __func__, rmem);

    uint32_t lock = int_lock();
    heap_free(codec_data_heap,rmem);
    int_unlock(lock);
}

void gaf_m55_stream_encoder_data_free(void *packet)
{
    ASSERT(packet, "%s packet = %p", __func__, packet);

    GAF_ENCODER_CC_MEDIA_DATA_T *encoder_frame_p = (GAF_ENCODER_CC_MEDIA_DATA_T *)packet;
    if (encoder_frame_p->data)
    {
        gaf_m55_stream_encoder_heap_free(encoder_frame_p->data);
    }
    gaf_m55_stream_encoder_heap_free(encoder_frame_p);
}

void *gaf_stream_pcm_data_frame_malloc(uint32_t packet_len)

{
    GAF_ENCODER_CC_MEDIA_DATA_T *encoder_frame_p = NULL;
    uint8_t *buffer = NULL;

    if (packet_len)
    {
        buffer = (uint8_t *)gaf_stream_heap_malloc(packet_len);
    }

    encoder_frame_p = (GAF_ENCODER_CC_MEDIA_DATA_T *)gaf_stream_heap_malloc(sizeof(GAF_ENCODER_CC_MEDIA_DATA_T));
    encoder_frame_p->org_addr = (uint8_t *) encoder_frame_p;
    encoder_frame_p->data     = buffer;
    encoder_frame_p->data_len = packet_len;
    return (void *)encoder_frame_p;
}
#endif

void *gaf_stream_heap_realloc(void *rmem, uint32_t newsize)
{
    void *ptr = heap_realloc(codec_data_heap, rmem, newsize);
    ASSERT(ptr, "%s rmem:%p size:%d", __func__, rmem,newsize);
#if GAF_STREAM_HEAP_DEBUG
    LOG_I("[%s] ptr=%p/%p size=%u user=%p left/mini left=%u/%u",
        __func__, rmem, ptr, newsize, __builtin_return_address(0),
        heap_free_size(codec_data_heap),
        heap_minimum_free_size(codec_data_heap));
#endif
    return ptr;
}

void gaf_stream_heap_free(void *rmem)
{
#if GAF_STREAM_HEAP_DEBUG
    LOG_I("[%s] ptr=%p user=%p", __func__, rmem, __builtin_return_address(0));
#endif
    ASSERT(rmem, "%s rmem:%p", __func__, rmem);

    heap_free(codec_data_heap,rmem);
}

void gaf_stream_data_free(void *packet)
{
    ASSERT(packet, "%s packet = %p", __func__, packet);

    gaf_media_data_t *decoder_frame_p = (gaf_media_data_t *)packet;
    if (decoder_frame_p->origin_buffer)
    {
        gaf_stream_heap_free(decoder_frame_p->origin_buffer);
        decoder_frame_p->origin_buffer = NULL;
    }
    gaf_stream_heap_free(decoder_frame_p);
}

void *gaf_stream_data_frame_malloc(uint32_t packet_len)

{
    gaf_media_data_t *decoder_frame_p = NULL;
    uint8_t *buffer = NULL;

    if (packet_len)
    {
        buffer = (uint8_t *)gaf_stream_heap_malloc(packet_len);
    }

    decoder_frame_p = (gaf_media_data_t *)gaf_stream_heap_malloc(sizeof(gaf_media_data_t));
    decoder_frame_p->origin_buffer = decoder_frame_p->sdu_data = buffer;
    decoder_frame_p->data_len = packet_len;
    return (void *)decoder_frame_p;
}

int inline gaf_buffer_mutex_lock(void *mutex)
{
    osMutexWait((osMutexId)mutex, osWaitForever);
    return 0;
}

int inline gaf_buffer_mutex_unlock(void *mutex)
{
    osMutexRelease((osMutexId)mutex);
    return 0;
}

int gaf_playback_status_mutex_lock(void *mutex)
{
    osMutexWait((osMutexId)mutex, osWaitForever);
    return 0;
}

int gaf_playback_status_mutex_unlock(void *mutex)
{
    osMutexRelease((osMutexId)mutex);
    return 0;
}

list_node_t *gaf_list_begin(gaf_stream_buff_list_t *list_info)
{
    gaf_buffer_mutex_lock(list_info->mutex);
    list_node_t *node = list_begin(list_info->list);
    gaf_buffer_mutex_unlock(list_info->mutex);
    return node;
}

list_node_t *gaf_list_end(gaf_stream_buff_list_t *list_info)
{
    gaf_buffer_mutex_lock(list_info->mutex);
    list_node_t *node = list_end(list_info->list);
    gaf_buffer_mutex_unlock(list_info->mutex);
    return node;
}

void *gaf_list_back(gaf_stream_buff_list_t *list_info)
{
    gaf_buffer_mutex_lock(list_info->mutex);
    void *data = list_back(list_info->list);
    gaf_buffer_mutex_unlock(list_info->mutex);
    return data;
}

uint32_t gaf_list_length(gaf_stream_buff_list_t *list_info)
{
    gaf_buffer_mutex_lock(list_info->mutex);
    uint32_t length = list_length(list_info->list);
    gaf_buffer_mutex_unlock(list_info->mutex);
    return length;
}

void *gaf_list_node(gaf_stream_buff_list_t *list_info)
{
    gaf_buffer_mutex_lock(list_info->mutex);
    void *data = list_node(list_info->node);
    gaf_buffer_mutex_unlock(list_info->mutex);
    return data;
}

list_node_t *gaf_list_next(gaf_stream_buff_list_t *list_info)
{
    gaf_buffer_mutex_lock(list_info->mutex);
    list_node_t *next =list_next(list_info->node);
    gaf_buffer_mutex_unlock(list_info->mutex);
    return next;
}

bool gaf_list_remove_generic(gaf_stream_buff_list_t *list_info, void *data)
{
    if ((!list_info) || (!data)) {
        LOG_W("%s, list:%p, data:%p", __func__, list_info, data);
        return false;
    }

    bool isSuccessful;
    gaf_buffer_mutex_lock(list_info->mutex);
    isSuccessful = list_remove(list_info->list, data);
    gaf_buffer_mutex_unlock(list_info->mutex);

    return isSuccessful;
}

bool gaf_list_remove(gaf_stream_buff_list_t *list_info, void *data)
{
    if ((!list_info) || (!data)) {
        LOG_W("%s, list:%p, data:%p", __func__, list_info, data);
        return false;
    }

#ifdef GAF_DECODER_CROSS_CORE_USE_M55
    gaf_media_data_t header = *(gaf_media_data_t *)data;
#endif

    gaf_buffer_mutex_lock(list_info->mutex);
    bool nRet = list_remove(list_info->list, data);
    gaf_buffer_mutex_unlock(list_info->mutex);

#ifdef GAF_DECODER_CROSS_CORE_USE_M55
    if (nRet)
    {
        // remove specfic frame
        gaf_decoder_mcu_remove_specfic_frame(data, header.pkt_seq_nb);
    }
#endif
    return nRet;
}

#ifdef GAF_CODEC_CROSS_CORE
void gaf_list_feed_off_core(gaf_stream_buff_list_t *list_info)
{
    list_node_t *node = gaf_list_begin(list_info);
    gaf_media_data_t* pFrameHeader = NULL;
    bool ret = true;
    uint8_t cnt = 0;

    if (node == NULL) {
        return;
    }

    do {
        pFrameHeader = (gaf_media_data_t *)list_node(node);
        if (!pFrameHeader->isFeedoffCore) {
#ifdef AOB_MOBILE_ENABLED
            if (is_support_ble_audio_mobile_m55_decode == true)
            {
                ret = gaf_mobile_decoder_mcu_feed_encoded_data_into_off_m55_core(
                    (uint8_t *)pFrameHeader, \
                    pFrameHeader->time_stamp, pFrameHeader->pkt_seq_nb, \
                    pFrameHeader->data_len, pFrameHeader->sdu_data, pFrameHeader->cisChannel, pFrameHeader->isPLC);
            }
            else
#endif
            {
#ifdef LOW_LATENCY_TEST
                ret = gaf_decoder_mcu_feed_low_latency_encoded_data_into_off_m55_core(
                    (uint8_t *)pFrameHeader, pFrameHeader->test_numb, \
                    pFrameHeader->time_stamp, pFrameHeader->pkt_seq_nb, \
                    pFrameHeader->data_len, pFrameHeader->sdu_data, pFrameHeader->isPLC);
#else
                ret = gaf_decoder_mcu_feed_encoded_data_into_off_m55_core(
                    (uint8_t *)pFrameHeader, \
                    pFrameHeader->time_stamp, pFrameHeader->pkt_seq_nb, \
                    pFrameHeader->data_len, pFrameHeader->sdu_data, pFrameHeader->isPLC);
#endif
            }
            if (!ret) {
                break;
            }
            pFrameHeader->isFeedoffCore = true;
            cnt++;
        }
        node = list_next(node);
    } while(node);
}
#endif

bool gaf_list_only_remove_node(gaf_stream_buff_list_t *list_info, void *data)
{
    list_free_cb free_cb = NULL;
    bool nRet = false;

    gaf_buffer_mutex_lock(list_info->mutex);

    // set free_cb to NULL, dont free gaf_media_data_t
    free_cb = list_info->list->free_cb;
    list_info->list->free_cb = NULL;

    nRet = gaf_list_remove_generic(list_info, data);

    // restore free_cb back
    list_info->list->free_cb = free_cb;

    gaf_buffer_mutex_unlock(list_info->mutex);

    return nRet;
}

bool gaf_list_append(gaf_stream_buff_list_t *list_info, void *data)
{
    gaf_buffer_mutex_lock(list_info->mutex);
    bool nRet = list_append(list_info->list, data);
    gaf_buffer_mutex_unlock(list_info->mutex);

#ifdef GAF_DECODER_CROSS_CORE_USE_M55
    if (nRet)
    {
        gaf_media_data_t* pFrameHeader = (gaf_media_data_t *)data;
        pFrameHeader->isFeedoffCore = false;
        gaf_list_feed_off_core(list_info);
        gaf_decoder_cc_update_lastframe(pFrameHeader);
    }
#endif
    return nRet;
}

void gaf_list_clear(gaf_stream_buff_list_t *list_info)
{
    gaf_buffer_mutex_lock(list_info->mutex);
    list_clear(list_info->list);
    gaf_buffer_mutex_unlock(list_info->mutex);
}

void gaf_list_free(gaf_stream_buff_list_t *list_info)
{
    gaf_buffer_mutex_lock(list_info->mutex);
    list_free(list_info->list);
    gaf_buffer_mutex_unlock(list_info->mutex);
}

void gaf_list_new(gaf_stream_buff_list_t *list_info, const osMutexDef_t *mutex_def,
    list_free_cb callback, list_mempool_zmalloc zmalloc, list_mempool_free free)
{
    if (NULL == list_info->mutex)
    {
        list_info->mutex = osMutexCreate(mutex_def);
    }
    ASSERT(list_info->mutex, "%s mutex create failed", __func__);

    gaf_buffer_mutex_lock(list_info->mutex);
    list_info->list = list_new(callback, zmalloc, free);
    gaf_buffer_mutex_unlock(list_info->mutex);
}

uint32_t gaf_stream_common_sample_freq_parse(uint8_t sample_freq)
{
    switch (sample_freq)
    {
        case BES_BLE_GAF_SAMPLE_FREQ_8000:
            return AUD_SAMPRATE_8000;
        break;
        case BES_BLE_GAF_SAMPLE_FREQ_16000:
            return AUD_SAMPRATE_16000;
        break;
        case BES_BLE_GAF_SAMPLE_FREQ_22050:
            return AUD_SAMPRATE_22050;
        break;
        case BES_BLE_GAF_SAMPLE_FREQ_24000:
            return AUD_SAMPRATE_24000;
        break;
        case BES_BLE_GAF_SAMPLE_FREQ_32000:
            return AUD_SAMPRATE_32000;
        break;
        case BES_BLE_GAF_SAMPLE_FREQ_44100:
            return AUD_SAMPRATE_44100;
        break;
        case BES_BLE_GAF_SAMPLE_FREQ_48000:
            return AUD_SAMPRATE_48000;
        break;
        case BES_BLE_GAF_SAMPLE_FREQ_88200:
            return AUD_SAMPRATE_88200;
        break;
        case BES_BLE_GAF_SAMPLE_FREQ_96000:
            return AUD_SAMPRATE_96000;
        break;
        case BES_BLE_GAF_SAMPLE_FREQ_176400:
            return AUD_SAMPRATE_176400;
        break;
        case BES_BLE_GAF_SAMPLE_FREQ_192000:
            return AUD_SAMPRATE_192000;
        break;
        case BES_BLE_GAF_SAMPLE_FREQ_384000:
            return AUD_SAMPRATE_384000;
        break;
        default:
            LOG_I("%s unsupported freq %d", __func__, sample_freq);
        break;
    }

    return 0;
}

float gaf_stream_common_frame_duration_parse(uint8_t frame_duration)
{
    LOG_I("%s  duration %d", __func__, frame_duration);

#ifdef AOB_LOW_LATENCY_MODE
    return 5;
#endif
    switch (frame_duration)
    {
#ifdef LC3PLUS_SUPPORT
        case BES_BLE_GAF_BAP_FRAME_DURATION_2_5MS:
            return 2.5;
        break;
        case BES_BLE_GAF_BAP_FRAME_DURATION_5MS:
            return 5;
        break;
#endif
        case BES_BLE_GAF_BAP_FRAME_DURATION_7_5MS:
            return 7.5;
        break;
        case BES_BLE_GAF_BAP_FRAME_DURATION_10MS:
            return 10;
        break;
        default:
            LOG_I("%s unsupported duration %d", __func__, frame_duration);
            ASSERT(0, "%s, wrong Divided ", __func__);
        break;
    }

    return 0;
}

static const char * const gaf_playback_stream_str[] =
{
    "IDLE",
    "WAITING_M55_INIT",
    "INITIALIZED",
    "START_TRIGGERING",
    "TRIGGERED",
};

void gaf_stream_common_update_playback_stream_state(GAF_AUDIO_STREAM_ENV_T* pStreamEnv,
    GAF_PLAYBACK_STREAM_STATE_E newState)
{
    if (newState != pStreamEnv->stream_context.playback_stream_state)
    {
        LOG_I("gaf playback stream state changes from %s to %s",
            gaf_playback_stream_str[pStreamEnv->stream_context.playback_stream_state],
            gaf_playback_stream_str[newState]);
        pStreamEnv->stream_context.playback_stream_state = newState;
    }

#ifndef BLE_USB_AUDIO_SUPPORT
    if (GAF_PLAYBACK_STREAM_START_TRIGGERING == pStreamEnv->stream_context.playback_stream_state)
    {
        if (pStreamEnv->stream_context.playback_trigger_supervisor_timer_id == NULL)
        {
            pStreamEnv->stream_context.playback_trigger_supervisor_timer_id =
                osTimerCreate(osTimer(GAF_STREAM_PLAYBACK_TRIGGER_TIMEOUT), osTimerOnce, pStreamEnv);
        }
        gaf_stream_playback_trigger_checker_start(pStreamEnv);
    }
    else if (GAF_PLAYBACK_STREAM_STREAMING_TRIGGERED == pStreamEnv->stream_context.playback_stream_state
        || GAF_PLAYBACK_STREAM_IDLE == pStreamEnv->stream_context.playback_stream_state)
    {
        gaf_stream_playback_trigger_checker_stop(pStreamEnv);
    }
#endif
}

static const char * const gaf_capture_stream_str[] =
{
    "IDLE",
    "INITIALIZED",
    "START_TRIGGERING",
    "TRIGGERED",
};

void gaf_stream_common_update_capture_stream_state(GAF_AUDIO_STREAM_ENV_T* pStreamEnv,
    GAF_CAPTURE_STREAM_STATE_E newState)
{
    if (newState != pStreamEnv->stream_context.capture_stream_state)
    {
        LOG_I("gaf capture stream state changes from %s to %s",
            gaf_capture_stream_str[pStreamEnv->stream_context.capture_stream_state],
            gaf_capture_stream_str[newState]);
        pStreamEnv->stream_context.capture_stream_state = newState;
    }

#ifndef BLE_USB_AUDIO_SUPPORT
    if (GAF_CAPTURE_STREAM_INITIALIZED == pStreamEnv->stream_context.capture_stream_state)
    {
        if (pStreamEnv->stream_context.capture_trigger_supervisor_timer_id == NULL)
        {
            pStreamEnv->stream_context.capture_trigger_supervisor_timer_id =
                osTimerCreate(osTimer(GAF_STREAM_CAPTURE_TRIGGER_TIMEOUT), osTimerOnce, pStreamEnv);
        }
        gaf_stream_capture_trigger_checker_start(pStreamEnv);
    }
    else if (GAF_CAPTURE_STREAM_STREAMING_TRIGGERED == pStreamEnv->stream_context.capture_stream_state
        || GAF_CAPTURE_STREAM_IDLE == pStreamEnv->stream_context.capture_stream_state)
    {
        gaf_stream_capture_trigger_checker_stop(pStreamEnv);
    }
#endif
}

const char* gaf_stream_common_get_capture_stream_state(GAF_CAPTURE_STREAM_STATE_E capture_stream_state)
{
    return gaf_capture_stream_str[capture_stream_state];
}

void gaf_stream_common_set_playback_trigger_time_generic(GAF_AUDIO_STREAM_ENV_T* pStreamEnv,
    uint8_t dstStreamType, uint32_t tg_tick)
{
    pStreamEnv->stream_context.lastPlaybackDmaIrqTimeUs = tg_tick;
    pStreamEnv->stream_context.playbackTriggerStartTicks = tg_tick;

    af_codec_set_device_bt_sync_source(AUD_STREAM_USE_INT_CODEC, (enum AUD_STREAM_T)dstStreamType,
        pStreamEnv->stream_context.playbackTriggerChannel);

    btdrv_enable_playback_triggler(ACL_TRIGGLE_MODE);
    bt_syn_ble_set_tg_ticks(tg_tick, pStreamEnv->stream_context.playbackTriggerChannel);

    LOG_I("[AOB TRIG] set playback trigger tg_tick:%u", tg_tick);

    gaf_stream_common_update_playback_stream_state(pStreamEnv, GAF_PLAYBACK_STREAM_START_TRIGGERING);
}

void gaf_stream_common_set_playback_trigger_time(GAF_AUDIO_STREAM_ENV_T* pStreamEnv, uint32_t tg_tick)
{
    gaf_stream_common_set_playback_trigger_time_generic(pStreamEnv, AUD_STREAM_PLAYBACK, tg_tick);
}

void gaf_stream_common_set_capture_trigger_time_generic(GAF_AUDIO_STREAM_ENV_T* pStreamEnv,
    uint8_t srcStreamType, uint32_t tg_tick)
{
    if ((GAF_CAPTURE_STREAM_START_TRIGGERING ==
        pStreamEnv->stream_context.capture_stream_state) ||
        (GAF_CAPTURE_STREAM_INITIALIZED ==
        pStreamEnv->stream_context.capture_stream_state))
    {
        pStreamEnv->stream_context.captureTriggerStartTicks = tg_tick;
        pStreamEnv->stream_context.lastCaptureDmaIrqTimeUs = tg_tick;
        pStreamEnv->stream_context.lastCaptureDmaIrqTimeUsInTriggerPoint = tg_tick;

        pStreamEnv->stream_context.captureAverageDmaChunkIntervalUs =
            pStreamEnv->stream_info.captureInfo.dmaChunkIntervalUs;
        pStreamEnv->stream_context.latestCaptureSeqNum = 0;

#ifdef GAF_ENCODER_CROSS_CORE_USE_M55
        pStreamEnv->stream_context.isUpStreamingStarted = false;
        if(pStreamEnv->stream_info.captureInfo.bnS2M >= 3)
        {
            pStreamEnv->stream_context.usGapBetweenCapturedFrameAndTransmittedFrame =
                pStreamEnv->stream_info.captureInfo.bnS2M * pStreamEnv->stream_info.captureInfo.dmaChunkIntervalUs;
            pStreamEnv->stream_context.capturedSeqNumToStartUpStreaming =
                pStreamEnv->stream_info.captureInfo.bnS2M;
        }
        else
        {
            pStreamEnv->stream_context.usGapBetweenCapturedFrameAndTransmittedFrame =
                (pStreamEnv->stream_info.captureInfo.bnS2M + 2) * pStreamEnv->stream_info.captureInfo.dmaChunkIntervalUs;
            pStreamEnv->stream_context.capturedSeqNumToStartUpStreaming =
                pStreamEnv->stream_info.captureInfo.bnS2M + 2;
        }
#else
        pStreamEnv->stream_context.isUpStreamingStarted = true;
        pStreamEnv->stream_context.usGapBetweenCapturedFrameAndTransmittedFrame = 0;
        pStreamEnv->stream_context.capturedSeqNumToStartUpStreaming = 0;
#endif

        af_codec_sync_device_config(AUD_STREAM_USE_INT_CODEC, (enum AUD_STREAM_T)srcStreamType,
            AF_CODEC_SYNC_TYPE_BT, false);
        af_codec_sync_device_config(AUD_STREAM_USE_INT_CODEC, (enum AUD_STREAM_T)srcStreamType,
            AF_CODEC_SYNC_TYPE_BT, true);
        af_codec_set_device_bt_sync_source(AUD_STREAM_USE_INT_CODEC,
            (enum AUD_STREAM_T)srcStreamType, pStreamEnv->stream_context.captureTriggerChannel);

        btdrv_enable_playback_triggler(ACL_TRIGGLE_MODE);

        bt_syn_ble_set_tg_ticks(tg_tick, pStreamEnv->stream_context.captureTriggerChannel);

        LOG_I("[AOB TRIG] set capture trigger tg_tick:%u", tg_tick);

        gaf_stream_common_update_capture_stream_state(pStreamEnv, GAF_CAPTURE_STREAM_START_TRIGGERING);
    }
    else
    {
        LOG_I("[AOB TRIG] cannot set trigger time when capture state is %s",
            gaf_capture_stream_str[pStreamEnv->stream_context.capture_stream_state]);
    }
}

void gaf_stream_common_set_capture_trigger_time(GAF_AUDIO_STREAM_ENV_T* pStreamEnv, uint32_t tg_tick)
{
    gaf_stream_common_set_capture_trigger_time_generic(pStreamEnv, AUD_STREAM_CAPTURE, tg_tick);
}

static void gaf_stream_common_updated_expeceted_playback_seq_num(GAF_AUDIO_STREAM_ENV_T* pStreamEnv,
                                                    uint8_t list_idx, uint32_t dmaIrqHappeningTimeUs)
{
    LOG_D("%s start", __func__);
    uint32_t expectedDmaIrqHappeningTimeUs =
        pStreamEnv->stream_context.lastPlaybackDmaIrqTimeUs +
        pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs;

    int32_t gapUs = GAF_AUDIO_CLK_32_BIT_DIFF(expectedDmaIrqHappeningTimeUs, dmaIrqHappeningTimeUs);
    int32_t gapUs_abs = GAF_AUDIO_ABS(gapUs);
    LOG_D("playback, irqTime:%u-%u, gapUs:%d, Abs:%u",
        expectedDmaIrqHappeningTimeUs, dmaIrqHappeningTimeUs,
        gapUs, gapUs_abs);

#ifdef GAF_DECODER_CROSS_CORE_USE_M55
    if (true ==is_support_ble_audio_mobile_m55_decode)
    {
        pStreamEnv->stream_context.lastestPlaybackSeqNumR++;
        pStreamEnv->stream_context.lastestPlaybackSeqNumL++;
    }
    else
    {
        pStreamEnv->stream_context.lastestPlaybackSeqNum[list_idx]++;
    }
#else
    pStreamEnv->stream_context.lastestPlaybackSeqNum[list_idx]++;
#endif

    if (gapUs_abs > (int32_t)pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs/2)
    {
        do
        {
#ifdef GAF_DECODER_CROSS_CORE_USE_M55
            if (true ==is_support_ble_audio_mobile_m55_decode)
            {
                pStreamEnv->stream_context.lastestPlaybackSeqNumR++;
                pStreamEnv->stream_context.lastestPlaybackSeqNumL++;
            }
            else
            {
                pStreamEnv->stream_context.lastestPlaybackSeqNum[list_idx]++;
            }
#else
                pStreamEnv->stream_context.lastestPlaybackSeqNum[list_idx]++;
#endif
            gapUs_abs = GAF_AUDIO_CLK_32_BIT_DIFF(gapUs_abs,
                (int32_t)pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs);
            gapUs_abs = GAF_AUDIO_ABS(gapUs_abs);
            LOG_I("[%d] updated gapus %d, irqTime:%u-%u", list_idx, gapUs_abs,
                expectedDmaIrqHappeningTimeUs, dmaIrqHappeningTimeUs);
        } while (gapUs_abs >= (int32_t)pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs/2);
    } else if (gapUs_abs > (int32_t)pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs) {
        LOG_W("playback gapUs exceed the dmaChunkIntervalUs %d, %u->%u", gapUs_abs,
            dmaIrqHappeningTimeUs, expectedDmaIrqHappeningTimeUs);
    }

    LOG_D(" %s Playback %u %u 0x%x", __func__,dmaIrqHappeningTimeUs, expectedDmaIrqHappeningTimeUs,
        pStreamEnv->stream_context.lastestPlaybackSeqNum[list_idx]);

    LOG_D("%s end", __func__);
}

void gaf_stream_common_updated_expeceted_playback_seq_and_time(GAF_AUDIO_STREAM_ENV_T* pStreamEnv,
                                                    uint8_t list_idx, uint32_t dmaIrqHappeningTimeUs)
{
    gaf_stream_common_updated_expeceted_playback_seq_num(pStreamEnv, list_idx, dmaIrqHappeningTimeUs);
    /// Update expected timestamp for next time dma irq
    pStreamEnv->stream_context.lastPlaybackDmaIrqTimeUs = dmaIrqHappeningTimeUs;
}

#if defined (BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT)
void gaf_stream_common_update_multi_channel_expect_seq_and_time(GAF_AUDIO_STREAM_ENV_T* pStreamEnv, uint32_t dmaIrqHappeningTimeUs)
{
    uint8_t ase_idx = 0;
    for (ase_idx = 0; ase_idx < (GAF_AUDIO_ASE_TOTAL_COUNT - 1); ase_idx++)
    {
        if (GAF_AUDIO_INVALID_ISO_CHANNEL != pStreamEnv->stream_info.playbackInfo.aseChInfo[ase_idx].iso_channel_hdl)
        {
            gaf_stream_common_updated_expeceted_playback_seq_num(pStreamEnv, ase_idx, dmaIrqHappeningTimeUs);
        }
    }
    /// Update expected timestamp for next time dma irq
    pStreamEnv->stream_context.lastPlaybackDmaIrqTimeUs = dmaIrqHappeningTimeUs;
}
#endif

static int32_t gaf_stream_common_update_average_dma_chunk_interval(int32_t y, int32_t x)
{
    if (y){
        y = ((GAF_AUDIO_ALPHA_PRAMS_1*y)+x)/GAF_AUDIO_ALPHA_PRAMS_2;
    }else{
        y = x;
    }
    return y;
}

void gaf_stream_common_capture_timestamp_checker_generic(
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv, uint32_t dmaIrqHappeningTimeUs, uint8_t StreamType)
{
    uint32_t latest_iso_bt_time = 0;
    int32_t usSinceLatestAnchorPoint = 0;

    latest_iso_bt_time = gaf_media_common_get_latest_tx_iso_evt_timestamp(pStreamEnv);
    usSinceLatestAnchorPoint = GAF_AUDIO_CLK_32_BIT_DIFF(dmaIrqHappeningTimeUs, latest_iso_bt_time);

    uint32_t expectedDmaIrqHappeningTimeUs =
        pStreamEnv->stream_context.lastCaptureDmaIrqTimeUs +
        (uint32_t)pStreamEnv->stream_context.captureAverageDmaChunkIntervalUs;

    int32_t gapUs = GAF_AUDIO_CLK_32_BIT_DIFF(dmaIrqHappeningTimeUs, expectedDmaIrqHappeningTimeUs);

    LOG_I("timestamp_checker,dma irq ts: %d - anchor point: %d", dmaIrqHappeningTimeUs, latest_iso_bt_time);
    int32_t gapUs_abs = GAF_AUDIO_ABS(gapUs);

    pStreamEnv->stream_context.latestCaptureSeqNum++;

    if (gapUs_abs < (int32_t)pStreamEnv->stream_info.captureInfo.dmaChunkIntervalUs/2)
    {
        int32_t dmaChunkInterval = GAF_AUDIO_CLK_32_BIT_DIFF(
            dmaIrqHappeningTimeUs,
            pStreamEnv->stream_context.lastCaptureDmaIrqTimeUs);
        dmaChunkInterval = GAF_AUDIO_ABS(dmaChunkInterval);

        pStreamEnv->stream_context.captureAverageDmaChunkIntervalUs =
            gaf_stream_common_update_average_dma_chunk_interval(pStreamEnv->stream_context.
            captureAverageDmaChunkIntervalUs,dmaChunkInterval);

        int32_t intervalGap;

        intervalGap = pStreamEnv->stream_context.usSinceLatestAnchorPoint - usSinceLatestAnchorPoint;

        if (GAF_MEDIA_PID_ABS(intervalGap) < pStreamEnv->stream_info.captureInfo.dmaChunkIntervalUs/2)
        {
            pStreamEnv->stream_context.lastCaptureDmaIrqTimeUsInTriggerPoint = dmaIrqHappeningTimeUs;
        }

        LOG_D("capture pid %d - %d us diff %d us", usSinceLatestAnchorPoint,
            pStreamEnv->stream_context.usSinceLatestAnchorPoint,
            intervalGap);

        LOG_D("capture anch %d dma irq %d gap %d - %d", latest_iso_bt_time,
            dmaIrqHappeningTimeUs, pStreamEnv->stream_context.usSinceLatestAnchorPoint,
            intervalGap);
        if (AUD_STREAM_CAPTURE == StreamType) {
            // tune the capture codec clock
            gaf_media_pid_adjust(AUD_STREAM_CAPTURE, &(pStreamEnv->stream_context.capture_pid_env),
                intervalGap);
        }
        else if (AUD_STREAM_PLAYBACK == StreamType) {
            gaf_media_pid_adjust(AUD_STREAM_PLAYBACK, &(pStreamEnv->stream_context.playback_pid_env),
                intervalGap);
        }
    }
    else
    {
        LOG_I("DMA happening interval is longer than chunk interval!");
        LOG_I("last dma irq %u us, this dma irq %u us, diff %d us",
            pStreamEnv->stream_context.lastCaptureDmaIrqTimeUs,
            dmaIrqHappeningTimeUs,
            gapUs_abs);
        do
        {
            pStreamEnv->stream_context.latestCaptureSeqNum++;
            gapUs_abs = GAF_AUDIO_CLK_32_BIT_DIFF(gapUs_abs,
                (int32_t)pStreamEnv->stream_info.captureInfo.dmaChunkIntervalUs);
            gapUs_abs = GAF_AUDIO_ABS(gapUs_abs);
        } while (gapUs_abs >= (int32_t)pStreamEnv->stream_info.captureInfo.dmaChunkIntervalUs/2);
    }

    if ((!pStreamEnv->stream_context.isUpStreamingStarted) &&
        (pStreamEnv->stream_context.latestCaptureSeqNum >=
        pStreamEnv->stream_context.capturedSeqNumToStartUpStreaming))
    {
        pStreamEnv->stream_context.isUpStreamingStarted = true;
        LOG_I("Cached seq num %d start up streaming", pStreamEnv->stream_context.latestCaptureSeqNum-1);
    }
    LOG_D("Capture dmaIrqTimeUs %u expectedDmaIrqTimeUs %u expected seq 0x%x",dmaIrqHappeningTimeUs,
        expectedDmaIrqHappeningTimeUs, pStreamEnv->stream_context.latestCaptureSeqNum);

    pStreamEnv->stream_context.lastCaptureDmaIrqTimeUs = dmaIrqHappeningTimeUs;
}

void gaf_stream_common_capture_timestamp_checker(
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv, uint32_t dmaIrqHappeningTimeUs)
{
    gaf_stream_common_capture_timestamp_checker_generic(pStreamEnv, dmaIrqHappeningTimeUs,
        AUD_STREAM_CAPTURE);
}

void gaf_stream_common_playback_timestamp_checker(
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv, uint32_t dmaIrqHappeningTimeUs)
{
    gaf_stream_common_capture_timestamp_checker_generic(pStreamEnv, dmaIrqHappeningTimeUs,AUD_STREAM_PLAYBACK);
}

#ifdef IS_BLE_AUDIO_DEBUG_INFO_COLLECTOR_ENABLED
static gaf_stream_common_plc_packet_occurs_cb plc_packet_occurs_handler;
void gaf_stream_common_register_stream_plc_packet_occurs_cb(gaf_stream_common_plc_packet_occurs_cb func)
{
    plc_packet_occurs_handler = func;
}

static gaf_stream_common_get_packet_cb stream_get_packet_handler;
void gaf_stream_common_register_stream_get_packet_cb(gaf_stream_common_get_packet_cb func)
{
    stream_get_packet_handler = func;
}

static gaf_stream_common_dma_irq_happens_cb stream_dma_irq_happens_handler;
void gaf_stream_common_register_dma_irq_happens_cb(gaf_stream_common_dma_irq_happens_cb func)
{
    stream_dma_irq_happens_handler = func;
}
#endif

gaf_media_data_t *gaf_stream_common_get_packet(GAF_AUDIO_STREAM_ENV_T* pStreamEnv, uint8_t list_index,
    uint32_t dmaIrqHappeningTimeUs)
{
    bool isPacketValid = false;
    bool next_packet_valid = false;
    bool play_time_arrived = false;
    bool play_time_not_arrive = false;
    gaf_stream_buff_list_t *list = &pStreamEnv->stream_context.playback_buff_list[list_index].buff_list;
    gaf_media_data_t *decoder_frame_p = NULL;
    gaf_media_data_t *decoder_frame_read = NULL;

    uint16_t expectedSeq = pStreamEnv->stream_context.lastestPlaybackSeqNum[list_index];
#ifdef IS_BLE_AUDIO_DEBUG_INFO_COLLECTOR_ENABLED
    if (NULL != stream_dma_irq_happens_handler)
    {
        stream_dma_irq_happens_handler(pStreamEnv->stream_info.con_lid, pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs);
    }
#endif
    do
    {
        list->node = gaf_list_begin(list);
        if (NULL == list->node)
        {
            LOG_W("gaf playback list is empty.");
            break;
        }
        decoder_frame_read = (gaf_media_data_t *)gaf_list_node(list);
#ifdef IS_BLE_AUDIO_DEBUG_INFO_COLLECTOR_ENABLED
        if (NULL != stream_get_packet_handler)
        {
            stream_get_packet_handler(pStreamEnv->stream_info.con_lid, decoder_frame_read->pkt_seq_nb, decoder_frame_read->pkt_status,
                decoder_frame_read->time_stamp, dmaIrqHappeningTimeUs);
        }
#endif
        if ((decoder_frame_read->pkt_seq_nb == expectedSeq)
            && (GAF_ISO_PKT_STATUS_VALID == decoder_frame_read->pkt_status))
        {
            int32_t bt_time_diff = 0;
            bt_time_diff = GAF_AUDIO_CLK_32_BIT_DIFF(decoder_frame_read->time_stamp, dmaIrqHappeningTimeUs);
            // check the time-stamp

#ifdef ADVANCE_FILL_ENABLED
            bt_time_diff += pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs;
#endif
            if (gaf_stream_get_prefill_status())
            {
                isPacketValid = true;
            }
            else if (GAF_AUDIO_MAX_DIFF_BT_TIME < (int32_t)(bt_time_diff - pStreamEnv->stream_info.playbackInfo.presDelayUs))
            {
                LOG_I("received packet's playtime has passed.");
                LOG_I("Seq: 0x%x Local time:%u packet's time stampt:%u time diff:%d",
                    decoder_frame_read->pkt_seq_nb,
                    dmaIrqHappeningTimeUs,
                    decoder_frame_read->time_stamp,
                    bt_time_diff);

#ifdef IS_BLE_AUDIO_DEBUG_INFO_COLLECTOR_ENABLED
                if (NULL != plc_packet_occurs_handler)
                {
                    plc_packet_occurs_handler(decoder_frame_read->pkt_seq_nb, decoder_frame_read->pkt_status,
                        decoder_frame_read->time_stamp, dmaIrqHappeningTimeUs);
                }
#endif
                gaf_list_remove_generic(list, decoder_frame_read);
                next_packet_valid = gaf_list_begin(list);
                if (next_packet_valid)
                {
                    pStreamEnv->stream_context.lastestPlaybackSeqNum[list_index]++;
                    expectedSeq = pStreamEnv->stream_context.lastestPlaybackSeqNum[list_index];
                    continue;
                }
                else
                {
                    play_time_arrived = true;
                }
            }
            else if (GAF_AUDIO_MAX_DIFF_BT_TIME < (int32_t)(pStreamEnv->stream_info.playbackInfo.presDelayUs - bt_time_diff))
            {
                LOG_I("received packet's playtime hasn't arrived.");
                LOG_I("Seq: 0x%x Local time:%u packet's time stampt:%u time diff:%d",
                    decoder_frame_read->pkt_seq_nb,
                    dmaIrqHappeningTimeUs,
                    decoder_frame_read->time_stamp,
                    bt_time_diff);
                pStreamEnv->stream_context.lastestPlaybackSeqNum[list_index] =
                    decoder_frame_read->pkt_seq_nb-1;
                play_time_not_arrive = true;

#ifdef IS_BLE_AUDIO_DEBUG_INFO_COLLECTOR_ENABLED
                if (NULL != plc_packet_occurs_handler)
                {
                    plc_packet_occurs_handler(decoder_frame_read->pkt_seq_nb, decoder_frame_read->pkt_status,
                        decoder_frame_read->time_stamp, dmaIrqHappeningTimeUs);
                }
#endif
            }
            else if (decoder_frame_read->data_len == 0)
            {
                LOG_I("received packet lenth is 0, timestamp %d", decoder_frame_read->time_stamp);
                gaf_list_remove_generic(list, decoder_frame_read);
                isPacketValid = false;
            }
            else
            {
                isPacketValid = true;
            }
        }
        else
        {
#ifdef IS_BLE_AUDIO_DEBUG_INFO_COLLECTOR_ENABLED
            if (NULL != plc_packet_occurs_handler)
            {
                plc_packet_occurs_handler(decoder_frame_read->pkt_seq_nb, decoder_frame_read->pkt_status,
                    decoder_frame_read->time_stamp, dmaIrqHappeningTimeUs);
            }
#endif
            if (decoder_frame_read->pkt_seq_nb != expectedSeq)
            {
                int32_t bt_time_diff = 0;
                bt_time_diff = GAF_AUDIO_CLK_32_BIT_DIFF(decoder_frame_read->time_stamp,
                    dmaIrqHappeningTimeUs);
                bt_time_diff = GAF_AUDIO_CLK_32_BIT_DIFF(bt_time_diff, pStreamEnv->stream_info.
                    playbackInfo.presDelayUs);
                bt_time_diff = GAF_AUDIO_ABS(bt_time_diff);
                if (bt_time_diff < GAF_AUDIO_MAX_DIFF_BT_TIME)
                {
                    expectedSeq = decoder_frame_read->pkt_seq_nb;
                    pStreamEnv->stream_context.lastestPlaybackSeqNum[list_index] = expectedSeq;
                    LOG_I("Get frame with right ts:%u local ts:%u",
                        decoder_frame_read->time_stamp, dmaIrqHappeningTimeUs);
                    continue;
                }

                LOG_I("Seq num error: expected 0x%x actual 0x%x local ts:%u rec ts:%u",
                    expectedSeq, decoder_frame_read->pkt_seq_nb, dmaIrqHappeningTimeUs,
                    decoder_frame_read->time_stamp);
                if (expectedSeq > 0)
                {
                    if (decoder_frame_read->pkt_seq_nb < expectedSeq)
                    {
                        gaf_list_remove_generic(list, decoder_frame_read);
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    if (decoder_frame_read->pkt_seq_nb > 0xFF00)
                    {
                        gaf_list_remove_generic(list, decoder_frame_read);
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else if (GAF_ISO_PKT_STATUS_VALID != decoder_frame_read->pkt_status)
            {
                LOG_I("packet is invalid, status %d", decoder_frame_read->pkt_status);
                // for those valid seq number but invalid content packets, consume them
                gaf_list_remove_generic(list, decoder_frame_read);
            }
        }

        break;

    } while (1);

    if (isPacketValid)
    {
        decoder_frame_p = decoder_frame_read;
        decoder_frame_p->isPLC = false;
        gaf_list_only_remove_node(list, decoder_frame_read);
    }
    else
    {
        LOG_I("[%d] Hit PLC event!", list_index);
        decoder_frame_p = (gaf_media_data_t *)gaf_stream_data_frame_malloc
                (pStreamEnv->stream_info.playbackInfo.encoded_frame_size*pStreamEnv->stream_info.playbackInfo.num_channels);
        decoder_frame_p->time_stamp = dmaIrqHappeningTimeUs - pStreamEnv->stream_info.playbackInfo.presDelayUs;
#ifdef ADVANCE_FILL_ENABLED
        decoder_frame_p->time_stamp += pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs;
#endif
        if (play_time_arrived)
        {
            decoder_frame_p->time_stamp -= GAF_AUDIO_MAX_DIFF_BT_TIME;
        }
        if (play_time_not_arrive)
        {
            decoder_frame_p->time_stamp += GAF_AUDIO_MAX_DIFF_BT_TIME;
        }
        if (decoder_frame_read)
        {
            decoder_frame_p->pkt_seq_nb = decoder_frame_read->pkt_seq_nb;
        }
        else
        {
            decoder_frame_p->pkt_seq_nb = 0;
        }
        decoder_frame_p->pkt_status = GAF_ISO_PKT_STATUS_INVALID;
        decoder_frame_p->data_len = pStreamEnv->stream_info.playbackInfo.encoded_frame_size*pStreamEnv->stream_info.playbackInfo.num_channels;
        decoder_frame_p->isPLC = true;
    }

    return decoder_frame_p;
}

#if defined (BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT)
gaf_media_data_t *gaf_stream_common_get_combined_packet_from_multi_channels(GAF_AUDIO_STREAM_ENV_T* pStreamEnv,
                                                                                            uint32_t dmaIrqHappeningTimeUs)
{
    POSSIBLY_UNUSED gaf_media_data_t *decoder_frame_p_fill = NULL;
    gaf_media_data_t *decoder_frame_p_l = NULL;
    gaf_media_data_t *decoder_frame_p_r = NULL;
    gaf_media_data_t *decoder_frame_p = NULL;
    POSSIBLY_UNUSED uint16_t channel_shift = 0;
    uint16_t packet_max_len = pStreamEnv->stream_info.playbackInfo.num_channels *
                              pStreamEnv->stream_info.playbackInfo.encoded_frame_size;

    uint8_t iso_chan_lid = 0;
    for (iso_chan_lid = 0; iso_chan_lid < (GAF_AUDIO_ASE_TOTAL_COUNT - 1); iso_chan_lid++)
    {
        /// Do not handle invalid iso channel
        if (pStreamEnv->stream_info.playbackInfo.aseChInfo[iso_chan_lid].iso_channel_hdl ==
            GAF_AUDIO_INVALID_ISO_CHANNEL)
        {
            continue;
        }

        uint8_t is_left_channel = ((pStreamEnv->stream_info.playbackInfo.aseChInfo[iso_chan_lid].allocation_bf &
                                                    (uint32_t)(BES_BLE_LOC_SIDE_LEFT | BES_BLE_LOC_FRONT_LEFT)) != 0);

        if (is_left_channel)
        {
            decoder_frame_p_l = gaf_stream_common_get_packet(pStreamEnv, iso_chan_lid, dmaIrqHappeningTimeUs);
        }
        else
        {
            decoder_frame_p_r = gaf_stream_common_get_packet(pStreamEnv, iso_chan_lid, dmaIrqHappeningTimeUs);
        }

        /// Now we combine two packet into one packet
        if (decoder_frame_p_l && decoder_frame_p_r)
        {
            decoder_frame_p = (gaf_media_data_t *)gaf_stream_data_frame_malloc(packet_max_len);
            memset_s(decoder_frame_p->sdu_data, packet_max_len, 0, packet_max_len);
            /// Use one channel packet to apply pid
            decoder_frame_p->pkt_status = decoder_frame_p_l->pkt_status;
            decoder_frame_p->pkt_seq_nb = decoder_frame_p_l->pkt_seq_nb;
            decoder_frame_p->time_stamp = decoder_frame_p_l->time_stamp;
            decoder_frame_p->data_len = packet_max_len;
            decoder_frame_p->isPLC = decoder_frame_p_l->isPLC || decoder_frame_p_r->isPLC;
            if (decoder_frame_p->isPLC == false)
            {
                /// Move two channel data to one packet
                memcpy_s(decoder_frame_p->sdu_data, packet_max_len / 2,
                                    decoder_frame_p_l->sdu_data, decoder_frame_p_l->data_len);
                memcpy_s(decoder_frame_p->sdu_data + (packet_max_len/2), packet_max_len / 2,
                                    decoder_frame_p_r->sdu_data, decoder_frame_p_r->data_len);
            }
            /// Free two channel packet
            gaf_stream_data_free(decoder_frame_p_l);
            gaf_stream_data_free(decoder_frame_p_r);
            decoder_frame_p_l = NULL;
            decoder_frame_p_r = NULL;
        }
    }

    /// If !decoder_frame_p:
    /// [1]just mono, use L or R packet, [2]stereo over one cis, L+R in one packet
    /// [3]stereo over one cis, only L or R in one packet, [4]stereo over two cis, but only L or R packet
    if (!decoder_frame_p)
    {
        decoder_frame_p = (decoder_frame_p_l == NULL ? decoder_frame_p_r : decoder_frame_p_l);
        /// Sanity check
        ASSERT(decoder_frame_p, "Invalid Frame NULL %d", __LINE__);
        /// Not hit plc ,just device only give one channel data, but we use two channel cfg init
        /// We should fill left channel with empty zero
        if (decoder_frame_p->data_len !=0 && ((decoder_frame_p->data_len * 2) == packet_max_len))
        {
            /// Check packet is lack of left or right
            channel_shift = decoder_frame_p_l == NULL ? (packet_max_len/2) : 0;
            /// Malloc packet max len to fill lc3 decoder
            decoder_frame_p_fill = (gaf_media_data_t *)gaf_stream_data_frame_malloc(packet_max_len);
            ASSERT(decoder_frame_p_fill, "Invalid Frame NULL %d", __LINE__);
            memset_s(decoder_frame_p_fill->sdu_data, packet_max_len, 0, packet_max_len);
            /// Use one channel packet to apply pid
            decoder_frame_p_fill->pkt_status = decoder_frame_p->pkt_status;
            decoder_frame_p_fill->pkt_seq_nb = decoder_frame_p->pkt_seq_nb;
            decoder_frame_p_fill->time_stamp = decoder_frame_p->time_stamp;
            decoder_frame_p_fill->isPLC = decoder_frame_p->isPLC;
            decoder_frame_p_fill->data_len = packet_max_len;
            /// Move channel data, left channel remains zero
            if (decoder_frame_p_fill->isPLC == false)
            {
                memcpy_s(decoder_frame_p_fill->sdu_data + channel_shift, packet_max_len/2,
                                            decoder_frame_p->sdu_data, decoder_frame_p->data_len);
            }
            /// Free previous channel packet
            gaf_stream_data_free(decoder_frame_p);
            decoder_frame_p = decoder_frame_p_fill;
        }
    }

    return decoder_frame_p;
}
#endif

#define GAF_PACKET_LOST_TESTx
#ifdef GAF_PACKET_LOST_TEST
static uint16_t cnt = 1;
static uint16_t limit = 50;
#endif
gaf_media_data_t* gaf_stream_common_store_received_packet(void* _pStreamEnv, uint8_t list_index, gaf_media_data_t *header)
{
#ifdef GAF_PACKET_LOST_TEST
    cnt+=1;
    if (cnt % limit == 0){
        TRACE(1, "INPUT FILL ERR!!!:%d", header->pkt_seq_nb);
        header->data_len = 0;
    }
#endif

    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    gaf_media_data_t *first_frame = NULL;
    gaf_stream_buff_list_t *list = &pStreamEnv->stream_context.playback_buff_list[list_index].buff_list;
    list_node_t *list_node = NULL;
    gaf_media_data_t *decoder_frame_p = NULL;
#ifdef GAF_CODEC_CROSS_CORE
    gaf_stream_packet_recover_proc(pStreamEnv, header);
#endif
    if (gaf_list_length(list) >= pStreamEnv->stream_info.playbackInfo.maxCachedEncodedAudioPacketCount)
    {
        LOG_I("%s list full,list_len:%d data_len:%d,remove first", __func__, gaf_list_length(list), header->data_len);
        list_node = gaf_list_begin(list);
        gaf_list_remove(list, list_node->data);
        first_frame = (gaf_media_data_t *)gaf_list_begin(list)->data;
        if (first_frame->pkt_status != GAF_ISO_PKT_STATUS_VALID)
        {
            pStreamEnv->stream_context.lastestPlaybackSeqNum[list_index]++;
        }
        else
        {
            pStreamEnv->stream_context.lastestPlaybackSeqNum[list_index] = first_frame->pkt_seq_nb;
        }
        LOG_I("Update starting playback seq num 0x%x, now first seq num 0x%x, pkt status %d",
                                        pStreamEnv->stream_context.lastestPlaybackSeqNum[list_index],
                                        first_frame->pkt_seq_nb, first_frame->pkt_status);
    }

    decoder_frame_p = header;

    if (!header->data_len)
    {
        decoder_frame_p->isPLC = true;
    }
    else
    {
        decoder_frame_p->isPLC = false;
    }

    gaf_list_append(list, decoder_frame_p);
    LOG_D("rec iso data: len %d list len-> %d seq 0x%x timestamp %u status %d",
         header->data_len, gaf_list_length(list), header->pkt_seq_nb,
         header->time_stamp, header->pkt_status);

    return decoder_frame_p;
}

#ifdef GAF_ENCODER_CROSS_CORE_USE_M55
/**
 ****************************************************************************************
 * @brief It is used to move pcm data packet into capture buff list
 *
 * @param[in] _pStreamEnv                audio stream global variable
 * @param[in] dmairqhappentime           the packet's happen time
 * @param[in] ptrBuf                     pcm_data
 * @param[in] length                     the length of pcm data packet
 *
 *
 * @param[out] nRet                      return status
 ****************************************************************************************
 */
bool gaf_stream_common_store_received_pcm_packet(void* _pStreamEnv, uint32_t dmairqhappentime,
                                                                                            uint8_t* ptrBuf, uint32_t length)
{
    /// Get the audio_stream_env global variable
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    /// Malloc capture buffer list
    gaf_stream_buff_list_t *list_info = &(pStreamEnv->stream_context.m55_capture_buff_list.buff_list);

    /// malloc list_node to identify if the list length more than the max length
    /// if the length is more than 30, will be removed
    list_node_t *list_node = NULL;
    if (gaf_list_length(list_info) >= GAF_ENCODER_PCM_DATA_BUFF_LIST_MAX_LENGTH){
        LOG_I("%s pcm_buffer_list full, list_len:%d data_len:%d, remove first", __func__, gaf_list_length(list_info), length);
        list_node = gaf_list_begin(list_info);
        gaf_list_remove(list_info, list_node->data);
    }

    /// malloc pcm data buffer
    GAF_ENCODER_CC_MEDIA_DATA_T *encoder_frame_p = NULL;
    encoder_frame_p = (GAF_ENCODER_CC_MEDIA_DATA_T *)gaf_stream_pcm_data_frame_malloc(length);
    encoder_frame_p->time_stamp                    = dmairqhappentime;
    encoder_frame_p->data_len                      = length;
    encoder_frame_p->frame_size                    = pStreamEnv->stream_info.captureInfo.encoded_frame_size;

    ///  move pcm data into the buffer list
    if (length){
        memcpy(encoder_frame_p->data, ptrBuf, length);
    }

    /// list append the pcm data packet into buff list
    uint32_t lock;
    lock = int_lock();
    bool nRet = list_append(list_info->list, encoder_frame_p);
    int_unlock(lock);

    if (nRet){
        LOG_D("rec iso pcm data: len %d list len-> %d timestamp %u", length, gaf_list_length(list_info), dmairqhappentime);
        LOG_D("bth reveived the pcm data is successed");
        /// send to m55 to encoder
        gaf_encoder_mcu_feed_pcm_data_into_off_m55_core(dmairqhappentime, encoder_frame_p->frame_size, length, ptrBuf, (void*)encoder_frame_p->org_addr);
    }
    return nRet;
}
#endif

void gaf_stream_common_register_func_list(GAF_AUDIO_STREAM_ENV_T* pStreamEnv,
    GAF_AUDIO_FUNC_LIST_T* funcList)
{
    pStreamEnv->func_list = funcList;
}

static const char * const gaf_stream_codec_type_str[] =
{
    "LC3",
#ifdef LC3PLUS_SUPPORT
    "LC3plus",
#endif
    "Unknown codec type",
};

const char* gaf_stream_common_print_code_type(uint8_t codec_id)
{
    switch (codec_id)
    {
        case BES_BLE_GAF_CODEC_TYPE_LC3:
            return gaf_stream_codec_type_str[0];
#ifdef LC3PLUS_SUPPORT
        case BES_BLE_GAF_CODEC_TYPE_VENDOR:
            return gaf_stream_codec_type_str[1];
        default:
            return gaf_stream_codec_type_str[2];
#else
        default:
            return gaf_stream_codec_type_str[1];
#endif
    }
}

static const char * const gaf_stream_context_type_str[] =
{
    "Conversational",
    "Media",
    "AI",
    "Live",
    "Game",
    "RingTone",
    "Instructional",
    "Notification",
    "Alerts",
    "Emergency alarm",
    "Unknown context",
};

const char* gaf_stream_common_print_context(uint16_t context_bf)
{
    switch (context_bf)
    {
        case BES_BLE_GAF_CONTEXT_TYPE_CONVERSATIONAL_BIT:
            return gaf_stream_context_type_str[0];
        case BES_BLE_GAF_CONTEXT_TYPE_MEDIA_BIT:
            return gaf_stream_context_type_str[1];
        case BES_BLE_GAF_CONTEXT_TYPE_MAN_MACHINE_BIT:
            return gaf_stream_context_type_str[2];
        case BES_BLE_GAF_CONTEXT_TYPE_LIVE_BIT:
            return gaf_stream_context_type_str[3];
        case BES_BLE_GAF_CONTEXT_TYPE_GAME_BIT:
            return gaf_stream_context_type_str[4];
        case BES_BLE_GAF_CONTEXT_TYPE_RINGTONE_BIT:
            return gaf_stream_context_type_str[5];
        case BES_BLE_GAF_CONTEXT_TYPE_INSTRUCTIONAL_BIT:
            return gaf_stream_context_type_str[6];
        case BES_BLE_GAF_CONTEXT_TYPE_ATTENTION_SEEKING_BIT:
            return gaf_stream_context_type_str[7];
        case BES_BLE_GAF_CONTEXT_TYPE_IMMEDIATE_ALERT_BIT:
            return gaf_stream_context_type_str[8];
        case BES_BLE_GAF_CONTEXT_TYPE_EMERGENCY_ALERT_BIT:
            return gaf_stream_context_type_str[9];
        default:
            return gaf_stream_context_type_str[10];//BES intentional code.The size of gaf_stream_context_type_str is 10
    }
}

void gaf_stream_common_clr_trigger(uint8_t triChannel)
{
    btdrv_syn_clr_trigger(triChannel);
    app_bt_sync_release_trigger_channel(triChannel);
}

void gaf_stream_register_running_stream_ref(uint8_t con_lid, GAF_AUDIO_STREAM_ENV_T* pStreamEnv)
{
#if defined (GAF_AUDIO_USE_ONE_STREAM_ONLY_ENABLED)
    gaf_audio_running_stream_ref[0] = pStreamEnv;
#else
    gaf_audio_running_stream_ref[con_lid] = pStreamEnv;
#endif
}


static void gaf_stream_common_trigger_sync_capture(
     GAF_AUDIO_STREAM_CONTEXT_TYPE_E streamContext, uint32_t master_clk_cnt,
     uint16_t master_bit_cnt, int32_t usSinceLatestAnchorPoint, uint32_t triggertimeUs)
{
    AOB_TWS_SYNC_TRIGGER_CAPTURE_INFO_T streamStatus;
    streamStatus.streamContext = streamContext;
    streamStatus.master_clk_cnt = master_clk_cnt;
    streamStatus.master_bit_cnt = master_bit_cnt;
    streamStatus.usSinceLatestAnchorPoint = usSinceLatestAnchorPoint;
    streamStatus.triggertimeUs = triggertimeUs;

    LOG_I("gaf sync capture info:");
    LOG_I("context %d usGap %d triggerUs %d",
        streamContext, usSinceLatestAnchorPoint, triggertimeUs);

    tws_ctrl_send_cmd(APP_TWS_CMD_REQ_TRIGGER_SYNC_CAPTURE,
                                       (uint8_t*)&streamStatus, sizeof(streamStatus));
}

static void gaf_stream_common_rsp_sync_capture_req(uint16_t req_seq,
     GAF_AUDIO_STREAM_CONTEXT_TYPE_E streamContext, uint32_t master_clk_cnt,
     uint16_t master_bit_cnt, int32_t usSinceLatestAnchorPoint, uint32_t triggertimeUs)
{
    AOB_TWS_SYNC_TRIGGER_CAPTURE_INFO_T streamStatus;
    streamStatus.streamContext = streamContext;
    streamStatus.master_clk_cnt = master_clk_cnt;
    streamStatus.master_bit_cnt = master_bit_cnt;
    streamStatus.usSinceLatestAnchorPoint = usSinceLatestAnchorPoint;
    streamStatus.triggertimeUs = triggertimeUs;
    tws_ctrl_send_rsp(APP_TWS_CMD_REQ_TRIGGER_SYNC_CAPTURE,
                            req_seq, (uint8_t*)&streamStatus, sizeof(streamStatus));
}

static void aob_tws_capture_stream_set_trigger_time(GAF_AUDIO_STREAM_ENV_T* pStreamEnv, uint32_t trigger_time_us)
{
    uint32_t current_bt_time = gaf_media_sync_get_curr_time();
    LOG_I("set trigger time %d current time %d", trigger_time_us, current_bt_time);
    int32_t gapUs = 0;
    // check how long since now to the trigger time
    while (gapUs < GAF_AUDIO_CAPTURE_TRIGGER_GUARD_TIME_US)
    {
        trigger_time_us += pStreamEnv->stream_info.captureInfo.isoIntervalUs;
        gapUs = GAF_AUDIO_CLK_32_BIT_DIFF(current_bt_time, trigger_time_us);
    }
    LOG_I("coordinated trigger time %d", trigger_time_us);
    gaf_stream_common_set_capture_trigger_time(pStreamEnv, trigger_time_us);
}

static void aob_tws_capture_stream_trigger_time_request_handler(uint16_t req_seq, AOB_TWS_SYNC_TRIGGER_CAPTURE_INFO_T* ptrPeerStreamStatus, GAF_AUDIO_STREAM_ENV_T* pStreamEnv)
{
    uint32_t trigger_time_master_clk_cnt = 0;
    uint16_t trigger_time_master_bit_cnt = 0;
    uint32_t trigger_time_us = 0;
    int32_t usSinceLatestAnchorPoint = GAF_STREAM_INVALID_US_SINCE_LAST_ANCHOR_POINT;
    uint32_t responsed_trigger_time_us = 0;

    btif_connection_role_t connection_role = app_tws_ibrt_get_local_tws_role();

    // if stream is already triggred, share the local start trigger time to peer device
    if (GAF_CAPTURE_STREAM_STREAMING_TRIGGERED ==
        pStreamEnv->stream_context.capture_stream_state)
    {
        trigger_time_us = pStreamEnv->stream_context.lastCaptureDmaIrqTimeUsInTriggerPoint-
                        pStreamEnv->stream_info.captureInfo.dmaChunkIntervalUs;
        usSinceLatestAnchorPoint = pStreamEnv->stream_context.usSinceLatestAnchorPoint;
        LOG_I("last trigger time %u usSinceLatestAnchorPoint %u",
            trigger_time_us, usSinceLatestAnchorPoint);
    }
    else
    {
        if (connection_role == BTIF_BCR_SLAVE)
        {
            // let primary just use its trigger time
            trigger_time_master_clk_cnt = GAF_STREAM_INVALID_CLOCK_CNT;
            responsed_trigger_time_us = ptrPeerStreamStatus->triggertimeUs;

            if ((GAF_CAPTURE_STREAM_START_TRIGGERING ==
                pStreamEnv->stream_context.capture_stream_state) &&
                (0 != ptrPeerStreamStatus->master_clk_cnt))
            {
                uint32_t master_trigger_time_us =
                    app_bt_sync_get_slave_time_from_master_time(
                        ptrPeerStreamStatus->master_clk_cnt, ptrPeerStreamStatus->master_bit_cnt);
                aob_tws_capture_stream_set_trigger_time(pStreamEnv, master_trigger_time_us);
            }
        }
        else
        {
            if (GAF_CAPTURE_STREAM_START_TRIGGERING ==
                pStreamEnv->stream_context.capture_stream_state)
            {
                trigger_time_us = pStreamEnv->stream_context.captureTriggerStartTicks;
            }
            else
            {
                trigger_time_master_clk_cnt = GAF_STREAM_INVALID_CLOCK_CNT;
                responsed_trigger_time_us = ptrPeerStreamStatus->triggertimeUs;
            }
        }
    }

    if (trigger_time_master_clk_cnt != GAF_STREAM_INVALID_CLOCK_CNT)
    {
        if (connection_role == BTIF_BCR_SLAVE)
        {
            app_bt_sync_get_master_time_from_slave_time(trigger_time_us,
                &trigger_time_master_clk_cnt, &trigger_time_master_bit_cnt);
        }
        else
        {
            btdrv_reg_op_bts_to_bt_time(trigger_time_us,
                &trigger_time_master_clk_cnt, &trigger_time_master_bit_cnt);
        }
    }

    gaf_stream_common_rsp_sync_capture_req(
        req_seq,
        pStreamEnv->stream_info.contextType,
        trigger_time_master_clk_cnt,
        trigger_time_master_bit_cnt,
        usSinceLatestAnchorPoint, responsed_trigger_time_us);
}

static GAF_AUDIO_STREAM_ENV_T* aob_tws_sync_capture_get_stream_env(AOB_TWS_SYNC_TRIGGER_CAPTURE_INFO_T* pStreamStatus)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = NULL;

#if !defined (GAF_AUDIO_USE_ONE_STREAM_ONLY_ENABLED)
    ASSERT(0, "add more function to get stream you want");
#endif

    if (gaf_audio_running_stream_ref[0] != NULL)
    {
        pStreamEnv = gaf_audio_running_stream_ref[0];
    }
    else
    {
        LOG_I("Local stream cb don't register.");
        return NULL;
    }

    if (NULL == pStreamEnv)
    {
        LOG_I("Local stream don't find.");
        return NULL;
    }

    if (GAF_CAPTURE_STREAM_IDLE == pStreamEnv->stream_context.capture_stream_state)
    {
        LOG_I("Local stream not configured yet.");
        return NULL;
    }

    return pStreamEnv;

}


void gaf_stream_common_tws_sync_capture_trigger_handler(uint16_t req_seq, uint8_t* data, uint16_t len)
{
    AOB_TWS_SYNC_TRIGGER_CAPTURE_INFO_T* pStreamStatus = (AOB_TWS_SYNC_TRIGGER_CAPTURE_INFO_T *)data;
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = aob_tws_sync_capture_get_stream_env(pStreamStatus);

    if (NULL == pStreamEnv)
    {
        // response peer device to let it just do its local trigger
        gaf_stream_common_rsp_sync_capture_req(req_seq, (GAF_AUDIO_STREAM_CONTEXT_TYPE_E)pStreamStatus->streamContext,
            GAF_STREAM_INVALID_CLOCK_CNT, 0, 0, pStreamStatus->triggertimeUs);
    }
    else
    {
        aob_tws_capture_stream_trigger_time_request_handler(req_seq, pStreamStatus, pStreamEnv);
    }
}

void gaf_stream_common_tws_sync_capture_trigger_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    AOB_TWS_SYNC_TRIGGER_CAPTURE_INFO_T* pStreamStatus = (AOB_TWS_SYNC_TRIGGER_CAPTURE_INFO_T *)p_buff;
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = aob_tws_sync_capture_get_stream_env(pStreamStatus);
    if (NULL != pStreamEnv)
    {
        aob_tws_capture_stream_set_trigger_time(pStreamEnv, pStreamStatus->triggertimeUs);
    }
}

void gaf_stream_common_tws_sync_capture_trigger_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    AOB_TWS_SYNC_TRIGGER_CAPTURE_INFO_T* ptrPeerStreamStatus = (AOB_TWS_SYNC_TRIGGER_CAPTURE_INFO_T *)p_buff;

    uint32_t trigger_time_us = 0;
    LOG_I("local clk cnt %u master_clk_cnt %u master_bit_cnt %u usSinceLatestAnchorPoint %d",
        btdrv_syn_get_curr_ticks(),
        ptrPeerStreamStatus->master_clk_cnt,
        ptrPeerStreamStatus->master_bit_cnt,
        ptrPeerStreamStatus->usSinceLatestAnchorPoint);

    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = aob_tws_sync_capture_get_stream_env(ptrPeerStreamStatus);

    if (ptrPeerStreamStatus->master_clk_cnt == GAF_STREAM_INVALID_CLOCK_CNT)
    {
        if (NULL != pStreamEnv)
        {
            aob_tws_capture_stream_set_trigger_time(pStreamEnv, ptrPeerStreamStatus->triggertimeUs);
        }
        return;
    }

    if (app_tws_ibrt_get_local_tws_role() == BTIF_BCR_SLAVE)
    {
        trigger_time_us = app_bt_sync_get_slave_time_from_master_time(
            ptrPeerStreamStatus->master_clk_cnt,
            ptrPeerStreamStatus->master_bit_cnt);
    }
    else
    {
        trigger_time_us = bt_syn_ble_bt_time_to_bts(ptrPeerStreamStatus->master_clk_cnt,
            ptrPeerStreamStatus->master_bit_cnt);
    }

    if (NULL != pStreamEnv)
    {
        aob_tws_capture_stream_set_trigger_time(pStreamEnv, trigger_time_us);
        if (GAF_STREAM_INVALID_US_SINCE_LAST_ANCHOR_POINT != ptrPeerStreamStatus->usSinceLatestAnchorPoint)
        {
            pStreamEnv->stream_context.isUsSinceLatestAnchorPointConfigured = true;
            pStreamEnv->stream_context.usSinceLatestAnchorPoint = ptrPeerStreamStatus->usSinceLatestAnchorPoint;
        }
    }
}

void gaf_stream_common_tws_sync_us_since_latest_anchor_point_handler(uint16_t rsp_seq, uint8_t* data, uint16_t len)
{
    AOB_TWS_SYNC_US_SINCE_LATEST_ANCHOR_POINT_T* info = (AOB_TWS_SYNC_US_SINCE_LATEST_ANCHOR_POINT_T *)data;
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = NULL;

#if !defined (GAF_AUDIO_USE_ONE_STREAM_ONLY_ENABLED)
    ASSERT(0, "add more function to get stream you want");
#endif

    if (gaf_audio_running_stream_ref[0] !=NULL)
    {
        pStreamEnv = gaf_audio_running_stream_ref[0];
        LOG_I("get peer side context %d usSinceLatestAnchorPoint %d", info->streamContext, info->usSinceLatestAnchorPoint);
    }
    else
    {
        LOG_I("Local stream cb don't register.");
        return;
    }

    if (pStreamEnv)
    {
        LOG_I("local stream state %d", pStreamEnv->stream_context.capture_stream_state);
        LOG_I("isUsSinceLatestAnchorPointConfigured %d usSinceLatestAnchorPoint %d",
            pStreamEnv->stream_context.isUsSinceLatestAnchorPointConfigured, pStreamEnv->stream_context.usSinceLatestAnchorPoint);
        if (pStreamEnv->stream_context.capture_stream_state >= GAF_CAPTURE_STREAM_INITIALIZED)
        {
            pStreamEnv->stream_context.isUsSinceLatestAnchorPointConfigured = true;
            pStreamEnv->stream_context.usSinceLatestAnchorPoint = info->usSinceLatestAnchorPoint;
        }
    }
}

void gaf_stream_common_sync_us_since_latest_anchor_point(GAF_AUDIO_STREAM_ENV_T* pStreamEnv)
{
    if (app_ibrt_conn_is_tws_connected())
    {
        if (app_tws_ibrt_get_local_tws_role() != BTIF_BCR_SLAVE)
        {
            AOB_TWS_SYNC_US_SINCE_LATEST_ANCHOR_POINT_T info;
            info.streamContext = pStreamEnv->stream_info.contextType;
            info.usSinceLatestAnchorPoint = pStreamEnv->stream_context.usSinceLatestAnchorPoint;
            tws_ctrl_send_cmd(APP_TWS_CMD_CAPTURE_US_SINCE_LATEST_ANCHOR,
                                                                    (uint8_t*)&info, sizeof(info));
        }
    }
}

void gaf_stream_common_start_sync_capture(GAF_AUDIO_STREAM_ENV_T* pStreamEnv)
{
    uint32_t latest_iso_bt_time = gaf_media_common_get_latest_tx_iso_evt_timestamp(pStreamEnv);
    uint32_t current_bt_time = gaf_media_sync_get_curr_time();
    uint32_t trigger_bt_time = latest_iso_bt_time+(pStreamEnv->stream_info.captureInfo.cigSyncDelayUs/pStreamEnv->stream_info.captureInfo.bnS2M/2);
    // move ahead of trigger time by 1ms to leave more margin for long CIG delay
    if (pStreamEnv->stream_info.captureInfo.cigSyncDelayUs > 3000)
    {
        trigger_bt_time -= 1000;
    }

    while (trigger_bt_time < current_bt_time + GAF_AUDIO_MAX_DIFF_BT_TIME)
    {
        trigger_bt_time += pStreamEnv->stream_info.captureInfo.isoIntervalUs;
    }


    LOG_I("iso anch %d cur time %d trigger time %d",
        latest_iso_bt_time, current_bt_time, trigger_bt_time);

    if (app_ibrt_conn_is_tws_connected())
    {
        uint32_t triggertimeUs = trigger_bt_time+
            ((200000/pStreamEnv->stream_info.captureInfo.isoIntervalUs)*
            pStreamEnv->stream_info.captureInfo.isoIntervalUs);
        if (app_tws_ibrt_get_local_tws_role() == BTIF_BCR_SLAVE)
        {
            gaf_stream_common_trigger_sync_capture(
                pStreamEnv->stream_info.contextType, 0, 0,
                GAF_STREAM_INVALID_US_SINCE_LAST_ANCHOR_POINT, triggertimeUs);
        }
        else
        {
            uint32_t masterClockCnt;
            uint16_t finecnt;
            btdrv_reg_op_bts_to_bt_time(triggertimeUs, &masterClockCnt, &finecnt);
            gaf_stream_common_trigger_sync_capture(
                pStreamEnv->stream_info.contextType, masterClockCnt, finecnt,
                GAF_STREAM_INVALID_US_SINCE_LAST_ANCHOR_POINT, triggertimeUs);
        }
    }
    else
    {
        gaf_stream_common_set_capture_trigger_time(pStreamEnv,
            trigger_bt_time+((20000/pStreamEnv->stream_info.captureInfo.isoIntervalUs)*
            pStreamEnv->stream_info.captureInfo.isoIntervalUs));
    }
}


bool gaf_stream_get_prefill_status(void)
{
    return gaf_is_doing_prefill;
}

void gaf_stream_set_prefill_status(bool is_doing)
{
    gaf_is_doing_prefill = is_doing;
}

void gaf_stream_common_set_custom_data_handler(GAF_STREAM_USER_CASE_E gaf_user_case,
                                               const GAF_STREAM_COMMON_CUSTOM_DATA_HANDLER_FUNC_T *func_list)
{
    ASSERT((gaf_user_case < GAF_STREAM_USER_CASE_MAX), "%s case idx err", __func__);

    GAF_STREAM_COMMON_CUSTOM_DATA_HANDLER_FUNC_T *func_local = &gaf_stream_common_custom_data_func_table[gaf_user_case];
    *func_local = *func_list;
    LOG_I("%s, [%p] [%p] [%p] [%p]", __func__, func_local->decoded_raw_data_cb, func_local->raw_pcm_data_cb,
                                               func_local->encoded_packet_recv_cb, func_local->decoded_raw_data_cb);
}

const GAF_STREAM_COMMON_CUSTOM_DATA_HANDLER_FUNC_T
     *gaf_stream_common_get_custom_data_handler(GAF_STREAM_USER_CASE_E gaf_user_case)
{
    ASSERT((gaf_user_case < GAF_STREAM_USER_CASE_MAX), "%s case idx err", __func__);

    return &gaf_stream_common_custom_data_func_table[gaf_user_case];
}

void gaf_stream_common_set_custom_presdelay_us(uint32_t presDelayUs)
{
    if (presDelayUs <= 0)
    {
        LOG_E("%s presDelayUs %d is not right", __func__, presDelayUs);
        return;
    }

    gaf_stream_custom_presdelay_us = presDelayUs;
    LOG_I("%s %d", __func__, presDelayUs);
}

uint32_t gaf_stream_common_get_custom_presdelay_us(void)
{
    return gaf_stream_custom_presdelay_us;
}

uint8_t gaf_stream_common_get_ase_idx_in_ase_lid_list(uint8_t* ase_lid_list, uint8_t ase_lid)
{
    for (uint8_t idx = 0; idx < GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT; idx++)
    {
        if (ase_lid_list[idx] == ase_lid)
        {
            return idx;
        }
    }
    return GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT;
}

uint8_t gaf_stream_common_get_valid_idx_in_ase_lid_list(uint8_t* ase_lid_list)
{
    for (uint8_t idx = 0; idx < GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT; idx++)
    {
        if (ase_lid_list[idx] == GAF_INVALID_ASE_INDEX)
        {
            return idx;
        }
    }

    return GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT;
}