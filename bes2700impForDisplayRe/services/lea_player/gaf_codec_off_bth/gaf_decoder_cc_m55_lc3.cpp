/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifdef GAF_DECODER_CROSS_CORE_USE_M55
#include "cmsis.h"
#include "string.h"
#include "heap_api.h"
#include "hal_location.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_aud.h"
#include "cmsis_os.h"
#include "app_overlay.h"
#include "gaf_codec_cc_common.h"
#include "lc3_process.h"
#include "gaf_codec_lc3.h"
#include "app_gaf_dbg.h"
#if defined(GAF_LC3_BES_PLC_ON)
#include "sbcplc.h"
#endif

/************************private macro defination***************************/
#define M55_LC3_AUDIO_DOWNLOAD_DUMP 0
#define AOB_CALCULATE_CODEC_MIPS    0

#if defined(GAF_LC3_BES_PLC_ON)
#define GAF_DECODER_LC3_MEM_POOL_SIZE (1024*65)
#else
#define GAF_DECODER_LC3_MEM_POOL_SIZE (1024*35)
#endif

#define GAF_MOBILE_DECODER_LC3_MEM_POOL_SIZE (1024 * 70)

/*********************private variable defination***************************/
#if defined(GAF_LC3_BES_PLC_ON)
static PLC_State *lc3_plc_state = NULL;
static float *lc3_cos_buf = NULL;
static int lc3_packeet_len = 0;
static int lc3_smooth_len = 0;
static int plc_is_16bit = 0;
#endif

uint8_t channel_value = 0;
bool is_right_channel_init = false;
bool is_left_channel_init  = false;
bool is_mobile_heap_init = false;
LC3_Dec_Info* lc3_cc_dec_info_right_channel = NULL;
LC3_Dec_Info* lc3_cc_dec_info_left_channel = NULL;
extern bool is_support_ble_audio_mobile_m55_decode_cc_core;
extern uint8_t right_cis_channel;
extern uint8_t left_cis_channel;

uint32_t lc3_cc_pcm_frame_len = 0;
LC3_Dec_Info* lc3_cc_dec_info = NULL;
short* lc3_cc_noninterleave_pcm_buf_left = NULL;
short* lc3_cc_noninterleave_pcm_buf_right = NULL;
heap_handle_t gaf_decode_lc3_heap_handle = NULL;
static uint8_t *gaf_decoder_lc3_mem_pool = NULL;

#if AOB_CALCULATE_CODEC_MIPS
/* AOB_CALCULATE_CODEC_MIPS structure */
typedef struct
{
    uint32_t start_time_in_ms;
    uint32_t total_time_in_us;
    bool codec_started;
    uint32_t codec_mips;
} aob_codec_time_info_t;

/* AOB_CALCULATE_CODEC_MIPS variable */
aob_codec_time_info_t aob_decode_time_info = {0};
#endif

/*********************external function declaration*************************/

/**********************private function declaration*************************/
void gaf_audio_decoder_cc_lc3_init(void);

void gaf_audio_decoder_cc_lc3_deinit(void);

static void gaf_audio_decoder_cc_lc3_clear_buffer_pointer(void);

static int gaf_audio_decoder_cc_lc3_decode_frame(uint32_t inputDataLength,
                                                        uint8_t* input, uint8_t* output, uint8_t isPLC);

GAF_AUDIO_CC_DECODER_T gaf_audio_cc_lc3_decoder_config =
{
    {44100, 2},
    gaf_audio_decoder_cc_lc3_init,
    gaf_audio_decoder_cc_lc3_deinit,
    gaf_audio_decoder_cc_lc3_decode_frame,
    gaf_audio_decoder_cc_lc3_clear_buffer_pointer,
};

#if AOB_CALCULATE_CODEC_MIPS
/* AOB_CALCULATE_CODEC_MIPS function */
POSSIBLY_UNUSED static uint32_t aob_lc3_get_sysfreq(void)
{
    enum HAL_CMU_FREQ_T cpu_freq = hal_sysfreq_get();

    switch(cpu_freq)
    {
        case HAL_CMU_FREQ_32K:
            return 0;
            break;
        case HAL_CMU_FREQ_26M:
            return 26;
            break;
        case HAL_CMU_FREQ_52M:
            return 52;
            break;
        case HAL_CMU_FREQ_78M:
            return 78;
            break;
        case HAL_CMU_FREQ_104M:
            return 104;
            break;
        case HAL_CMU_FREQ_208M:
            return 208;
            break;
        default:
            return 0;
            break;
    }
}
#endif

static void gaf_audio_decoder_cc_lc3_clear_buffer_pointer(void)
{
    gaf_decoder_lc3_mem_pool = NULL;
}

static void gaf_decoder_lc3_heap_init(void *begin_addr, uint32_t size)
{
    gaf_decode_lc3_heap_handle = heap_register(begin_addr, size);
}

void *gaf_stream_lc3_decode_heap_malloc(uint32_t size)
{
    uint32_t lock = int_lock();
    size = ((size >> 2) + 1) << 2;
    void *ptr = heap_malloc(gaf_decode_lc3_heap_handle, size);
    if (ptr == NULL)
    {
        LOG_I("gaf stream heap malloc failed:%d", 1);
    }
    ASSERT(ptr, "%s size:%d", __func__, size);
    int_unlock(lock);
    return ptr;
}

void gaf_stream_lc3_decode_heap_free(void *rmem)
{
    ASSERT(rmem, "%s rmem:%p", __func__, rmem);

    uint32_t lock = int_lock();
    heap_free(gaf_decode_lc3_heap_handle, rmem);
    int_unlock(lock);
}

int gaf_stream_lc3_decode_bits(void *lc3_dec_info, void *input_bytes, int32_t num_bytes,
    void *output_samples, uint8_t isPLC)
{
    int32_t err = LC3_API_OK;
    LC3_Dec_Info *p_lc3_dec_info = (LC3_Dec_Info *)lc3_dec_info;
    ASSERT(p_lc3_dec_info, "p_lc3_dec_info is empty in %s ", __func__);

    int32_t bfi_ext = 0;
    /// LC3 library plc algorithm flag

    if ((0 == num_bytes) || (1 == isPLC))
    {
        bfi_ext = 1;
    }

#if AOB_CALCULATE_CODEC_MIPS
    uint32_t cpu_freq = 0;
    uint32_t stime = 0, etime = 0;
    uint32_t use_time_in_us = 0, total_etime_in_ms = 0;
    if (!aob_decode_time_info.codec_started)
    {
        aob_decode_time_info.codec_started = true;
        aob_decode_time_info.total_time_in_us = 0;
        aob_decode_time_info.start_time_in_ms = hal_sys_timer_get();
    }

    stime = hal_sys_timer_get();
#endif
    err = p_lc3_dec_info->cb_decode_interlaced(p_lc3_dec_info, p_lc3_dec_info->scratch, input_bytes,
        num_bytes, output_samples, bfi_ext);

#if defined(GAF_LC3_BES_PLC_ON)
    /// BES plc algorithm flag
    int32_t bes_ext = 0;
    if ((0 == num_bytes) || (1 == isPLC))
    {
        bes_ext = 1;
    }
    if(plc_is_16bit == 1)
    {
        if(bes_ext){
            a2dp_plc_bad_frame(lc3_plc_state, (short *)output_samples, (short *)output_samples, lc3_cos_buf, lc3_packeet_len, 1, 0);
        }else{
            a2dp_plc_good_frame(lc3_plc_state, (short *)output_samples, (short *)output_samples, lc3_cos_buf, lc3_packeet_len, 1, 0);
        }      
    }else{
        if(bes_ext){
            a2dp_plc_bad_frame_24bit(lc3_plc_state, (int32_t *)output_samples, (int32_t *)output_samples, lc3_cos_buf, lc3_packeet_len, 1, 0);
        }else{
            a2dp_plc_good_frame_24bit(lc3_plc_state, (int32_t *)output_samples, (int32_t *)output_samples, lc3_cos_buf, lc3_packeet_len, 1, 0);
        }
    }
#endif

#if AOB_CALCULATE_CODEC_MIPS
    etime = hal_sys_timer_get();
    use_time_in_us = TICKS_TO_US(etime - stime);
    total_etime_in_ms = TICKS_TO_MS(etime - aob_decode_time_info.start_time_in_ms);
    aob_decode_time_info.total_time_in_us += use_time_in_us;
    cpu_freq = aob_lc3_get_sysfreq();
    if (total_etime_in_ms)
    {
        aob_decode_time_info.codec_mips =
            cpu_freq * (aob_decode_time_info.total_time_in_us / 1000) / total_etime_in_ms;
    }
    LOG_I("err %d ticks:%d time:%d us",
        err, (etime - stime), use_time_in_us);
    LOG_I("freq %d use:%d ms total:%d ms mips: %d M", cpu_freq,
        aob_decode_time_info.total_time_in_us/1000,
        total_etime_in_ms, aob_decode_time_info.codec_mips);
#endif
    return (int)err;
}

static void* lc3_alloc(void* pool,unsigned size){
    return gaf_stream_lc3_decode_heap_malloc(size);
}
static void lc3_free(void* pool, void*ptr){
    gaf_stream_lc3_decode_heap_free(ptr);
}

void gaf_audio_mobile_decoder_lc3_init(void)
{
    if (false == is_mobile_heap_init)
    {
        if (NULL == gaf_decoder_lc3_mem_pool)
        {
            off_bth_syspool_get_buff(&gaf_decoder_lc3_mem_pool, GAF_MOBILE_DECODER_LC3_MEM_POOL_SIZE);
            ASSERT(gaf_decoder_lc3_mem_pool, "%s size:%d", __func__, GAF_MOBILE_DECODER_LC3_MEM_POOL_SIZE);
        }
        gaf_decoder_lc3_heap_init(gaf_decoder_lc3_mem_pool, GAF_MOBILE_DECODER_LC3_MEM_POOL_SIZE);
        is_mobile_heap_init = true;
    }

    if (false == is_right_channel_init)
    {
        is_right_channel_init = true;
        /// m55 malloc decoder information heap in the gaf audio heap_buff
        lc3_cc_dec_info_right_channel = (LC3_Dec_Info*)gaf_stream_lc3_decode_heap_malloc(sizeof(LC3_Dec_Info));
        memset((void*)lc3_cc_dec_info_right_channel, 0x0, sizeof(LC3_Dec_Info));

        /// Add the lc3_cc_dec_info structure
        lc3_cc_dec_info_right_channel->sample_rate  = gaf_audio_cc_lc3_decoder_config.stream_config.sample_rate;
        lc3_cc_dec_info_right_channel->channels     = (int8_t)gaf_audio_cc_lc3_decoder_config.stream_config.channels;
        lc3_cc_dec_info_right_channel->bitwidth     = (int8_t)gaf_audio_cc_lc3_decoder_config.stream_config.bitwidth;
        lc3_cc_dec_info_right_channel->bitalign     = (lc3_cc_dec_info_right_channel->bitwidth==24)?32:0;
        lc3_cc_dec_info_right_channel->frame_dms    = (uint8_t)gaf_audio_cc_lc3_decoder_config.stream_config.frame_ms;
        lc3_cc_dec_info_right_channel->frame_size   = (uint16_t)gaf_audio_cc_lc3_decoder_config.stream_config.frame_size;
        lc3_cc_dec_info_right_channel->bitrate      = gaf_audio_cc_lc3_decoder_config.stream_config.bitrate;
        lc3_cc_dec_info_right_channel->plcMeth      = LC3_API_PLC_ADVANCED;
        lc3_cc_dec_info_right_channel->epmode       = LC3_API_EP_OFF;
        lc3_cc_dec_info_right_channel->pool         = 0;
        lc3_cc_dec_info_right_channel->is_interlaced = 1;
        lc3_cc_dec_info_right_channel->cb_alloc     = &lc3_alloc;
        lc3_cc_dec_info_right_channel->cb_free      = &lc3_free;

        /// use lc3 decoder init api to get other parameters
        int err = lc3_api_decoder_init(lc3_cc_dec_info_right_channel);
        ASSERT(LC3_API_OK == err, "%s right_channel_err: %d", __func__, err);
    }

    if (false == is_left_channel_init)
    {
        is_left_channel_init = true;
        /// m55 malloc decoder information heap in the gaf audio heap_buff
        lc3_cc_dec_info_left_channel = (LC3_Dec_Info*)gaf_stream_lc3_decode_heap_malloc(sizeof(LC3_Dec_Info));
        memset((void*)lc3_cc_dec_info_left_channel, 0x0, sizeof(LC3_Dec_Info));

        /// Add the lc3_cc_dec_info structure
        lc3_cc_dec_info_left_channel->sample_rate  = gaf_audio_cc_lc3_decoder_config.stream_config.sample_rate;
        lc3_cc_dec_info_left_channel->channels     = (int8_t)gaf_audio_cc_lc3_decoder_config.stream_config.channels;
        lc3_cc_dec_info_left_channel->bitwidth     = (int8_t)gaf_audio_cc_lc3_decoder_config.stream_config.bitwidth;
        lc3_cc_dec_info_left_channel->bitalign     = (lc3_cc_dec_info_left_channel->bitwidth==24)?32:0;
        lc3_cc_dec_info_left_channel->frame_dms    = (uint8_t)gaf_audio_cc_lc3_decoder_config.stream_config.frame_ms;
        lc3_cc_dec_info_left_channel->frame_size   = (uint16_t)gaf_audio_cc_lc3_decoder_config.stream_config.frame_size;
        lc3_cc_dec_info_left_channel->bitrate      = gaf_audio_cc_lc3_decoder_config.stream_config.bitrate;
        lc3_cc_dec_info_left_channel->plcMeth      = LC3_API_PLC_ADVANCED;
        lc3_cc_dec_info_left_channel->epmode       = LC3_API_EP_OFF;
        lc3_cc_dec_info_left_channel->pool         = 0;
        lc3_cc_dec_info_left_channel->is_interlaced = 1;
        lc3_cc_dec_info_left_channel->cb_alloc     = &lc3_alloc;
        lc3_cc_dec_info_left_channel->cb_free      = &lc3_free;

        /// use lc3 decoder init api to get other parameters
        int err = lc3_api_decoder_init(lc3_cc_dec_info_left_channel);
        ASSERT(LC3_API_OK == err, "%s left_channel_err: %d", __func__, err);
    }
}

void gaf_audio_earbud_decoder_lc3_init(void)
{
    ///m55 lc3 decoder init start
    uint32_t decoder_size = 0;
    uint32_t scratch_size = 0;

    if (NULL == gaf_decoder_lc3_mem_pool)
    {
        off_bth_syspool_get_buff(&gaf_decoder_lc3_mem_pool, GAF_DECODER_LC3_MEM_POOL_SIZE);
        ASSERT(gaf_decoder_lc3_mem_pool, "%s size:%d", __func__, GAF_DECODER_LC3_MEM_POOL_SIZE);
    }
    gaf_decoder_lc3_heap_init(gaf_decoder_lc3_mem_pool, GAF_DECODER_LC3_MEM_POOL_SIZE);

    /// m55 malloc decoder information heap in the gaf audio heap_buff
    lc3_cc_dec_info = (LC3_Dec_Info*)gaf_stream_lc3_decode_heap_malloc(sizeof(LC3_Dec_Info));
    memset((void*)lc3_cc_dec_info, 0x0, sizeof(LC3_Dec_Info));

#if defined(GAF_LC3_BES_PLC_ON)
    lc3_packeet_len = (gaf_audio_cc_lc3_decoder_config.stream_config.sample_rate * gaf_audio_cc_lc3_decoder_config.stream_config.frame_ms / 10000) * 2;
    lc3_smooth_len = lc3_packeet_len * 4;
    if(gaf_audio_cc_lc3_decoder_config.stream_config.bitwidth == 16){
        plc_is_16bit = 1;
    }else{
        plc_is_16bit = 0;
    }
    lc3_plc_state = (PLC_State *)gaf_stream_lc3_decode_heap_malloc(sizeof(PLC_State));
    lc3_cos_buf = (float *)gaf_stream_lc3_decode_heap_malloc(lc3_smooth_len * sizeof(float));
    a2dp_plc_lc3_init(lc3_plc_state, A2DP_PLC_CODEC_TYPE_LC3, lc3_packeet_len/2);
    cos_generate(lc3_cos_buf, lc3_smooth_len, lc3_packeet_len);
#endif
    /// Add the lc3_cc_dec_info structure
    lc3_cc_dec_info->sample_rate  = gaf_audio_cc_lc3_decoder_config.stream_config.sample_rate;
    lc3_cc_dec_info->channels     = (int8_t)gaf_audio_cc_lc3_decoder_config.stream_config.channels;
    lc3_cc_dec_info->bitwidth     = (int8_t)gaf_audio_cc_lc3_decoder_config.stream_config.bitwidth;
    lc3_cc_dec_info->bitalign     = (lc3_cc_dec_info->bitwidth==24)?32:0;
    lc3_cc_dec_info->frame_dms    = (uint8_t)gaf_audio_cc_lc3_decoder_config.stream_config.frame_ms;
    lc3_cc_dec_info->frame_size   = (uint16_t)gaf_audio_cc_lc3_decoder_config.stream_config.frame_size;
    lc3_cc_dec_info->bitrate      = gaf_audio_cc_lc3_decoder_config.stream_config.bitrate;
    lc3_cc_dec_info->plcMeth      = LC3_API_PLC_ADVANCED;
    lc3_cc_dec_info->epmode       = LC3_API_EP_OFF;
    lc3_cc_dec_info->pool         = 0;
    lc3_cc_dec_info->is_interlaced = 1;
    lc3_cc_dec_info->cb_alloc     = &lc3_alloc;
    lc3_cc_dec_info->cb_free      = &lc3_free;

    LOG_I("CC_Sample rate:      %i", lc3_cc_dec_info->sample_rate);
    LOG_I("CC_Channels:         %i", lc3_cc_dec_info->channels);
    LOG_I("CC_bitwidth:         %i", lc3_cc_dec_info->bitwidth);
    LOG_I("bitrate:             %i", lc3_cc_dec_info->bitrate);
    /// use lc3 decoder init api to get other parameters
    int err = lc3_api_decoder_init(lc3_cc_dec_info);
    ASSERT(LC3_API_OK == err, "%s err: %d", __func__, err);

    LOG_I("CC_Decoder size:     %i", decoder_size);
    LOG_I("CC_Scratch size:     %i", scratch_size);
}

void gaf_audio_decoder_cc_lc3_init(void)
{
    LOG_I("[%s] start", __func__);
    if (is_support_ble_audio_mobile_m55_decode_cc_core)
    {
        gaf_audio_mobile_decoder_lc3_init();
    }
    else
    {
        gaf_audio_earbud_decoder_lc3_init();
    }
    LOG_I("[%s] end", __func__);
}

void gaf_audio_decoder_cc_lc3_deinit(void)
{
    LOG_I("[%s] start", __func__);
    if (is_support_ble_audio_mobile_m55_decode_cc_core)
    {
        is_mobile_heap_init = false;
        if (NULL == lc3_cc_dec_info_right_channel)
        {
            return ;
        }
        else
        {
            is_right_channel_init = false;
            if (lc3_cc_dec_info_right_channel->instance){
                gaf_stream_lc3_decode_heap_free(lc3_cc_dec_info_right_channel->instance);
                lc3_cc_dec_info_right_channel->instance = NULL;
            }
            if (lc3_cc_dec_info_right_channel->scratch){
                gaf_stream_lc3_decode_heap_free(lc3_cc_dec_info_right_channel->scratch);
                lc3_cc_dec_info_right_channel->scratch = NULL;
            }
            lc3_api_decoder_extra_buf_deinit(lc3_cc_dec_info_right_channel);
            if (lc3_cc_dec_info_right_channel) {
                gaf_stream_lc3_decode_heap_free(lc3_cc_dec_info_right_channel);
                lc3_cc_dec_info_right_channel = NULL;
            }
        }

        if (NULL == lc3_cc_dec_info_left_channel)
        {
            return ;
        }
        else
        {
            is_left_channel_init = false;
            if (lc3_cc_dec_info_left_channel->instance){
                gaf_stream_lc3_decode_heap_free(lc3_cc_dec_info_left_channel->instance);
                lc3_cc_dec_info_left_channel->instance = NULL;
            }
            if (lc3_cc_dec_info_left_channel->scratch){
                gaf_stream_lc3_decode_heap_free(lc3_cc_dec_info_left_channel->scratch);
                lc3_cc_dec_info_left_channel->scratch = NULL;
            }
            lc3_api_decoder_extra_buf_deinit(lc3_cc_dec_info_left_channel);
            if (lc3_cc_dec_info_left_channel) {
                gaf_stream_lc3_decode_heap_free(lc3_cc_dec_info_left_channel);
                lc3_cc_dec_info_left_channel = NULL;
            }
        }
    }
    else
    {
        if (lc3_cc_dec_info->instance){
            gaf_stream_lc3_decode_heap_free(lc3_cc_dec_info->instance);
            lc3_cc_dec_info->instance = NULL;
        }
        if (lc3_cc_dec_info->scratch){
            gaf_stream_lc3_decode_heap_free(lc3_cc_dec_info->scratch);
            lc3_cc_dec_info->scratch = NULL;
        }
        lc3_api_decoder_extra_buf_deinit(lc3_cc_dec_info);
        if (lc3_cc_dec_info) {
            gaf_stream_lc3_decode_heap_free(lc3_cc_dec_info);
            lc3_cc_dec_info = NULL;
        }
    }
    LOG_I("[%s] end", __func__);
}

static int gaf_audio_decoder_cc_lc3_decode_frame(uint32_t inputDataLength, uint8_t* input, uint8_t* output, uint8_t isPLC)
{
    int ret = LC3_API_OK;
    if (is_support_ble_audio_mobile_m55_decode_cc_core)
    {
        if (right_cis_channel == channel_value)
        {
            /// right channel
            ret = gaf_stream_lc3_decode_bits(lc3_cc_dec_info_right_channel, input, inputDataLength, output, isPLC);
        }
        else if (left_cis_channel == channel_value)
        {
            /// left channel
            ret = gaf_stream_lc3_decode_bits(lc3_cc_dec_info_left_channel, input, inputDataLength, output, isPLC);
        }
    }
    else
    {
        ret = gaf_stream_lc3_decode_bits(lc3_cc_dec_info, input, inputDataLength, output, isPLC);
    }

    if (ret)
    {
        LOG_I("LC3 dec err %d\n", ret);
    }

#if M55_LC3_AUDIO_DOWNLOAD_DUMP
        uint32_t output_len = (uint32_t)(lc3_cc_dec_info->sample_rate* lc3_cc_dec_info->frame_dms/1000);
        audio_dump_clear_up();
        audio_dump_add_channel_data(0, output, output_len);
        audio_dump_run();
#endif
    return ret;
}

#endif /// GAF_DECODER_CROSS_CORE_USE_M55