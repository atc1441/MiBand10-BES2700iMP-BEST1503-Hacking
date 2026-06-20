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

#ifdef GAF_ENCODER_CROSS_CORE_USE_M55
#include "cmsis.h"
#include "string.h"
#include "heap_api.h"
#include "hal_location.h"
#include "hal_timer.h"
#include "hal_aud.h"
#include "cmsis_os.h"
#include "gaf_codec_cc_common.h"
#include "lc3_process.h"
#include "gaf_codec_lc3.h"
#include "app_overlay.h"
#include "app_gaf_dbg.h"

/************************private macro defination***************************/
#define AOB_CALCULATE_CODEC_MIPS        0
#define M55_LC3_AUDIO_UPLOAD_DUMP       0
#define GAF_ENCODER_LC3_MEM_POOL_SIZE   (1024*26)

/************************private variable defination***************************/
LC3_Enc_Info* lc3_cc_enc_info            = NULL;
uint32_t lc3_cc_encode_frame_len         = 0;
heap_handle_t gaf_encode_lc3_heap_handle = NULL;
static uint8_t *gaf_encoder_lc3_mem_pool = NULL;

/* AOB_CALCULATE_CODEC_MIPS structure */
#if AOB_CALCULATE_CODEC_MIPS
typedef struct
{
    uint32_t start_time_in_ms;
    uint32_t total_time_in_us;
    bool codec_started;
    uint32_t codec_mips;
} aob_codec_time_info_t;

/* AOB_CALCULATE_CODEC_MIPS variable */
aob_codec_time_info_t aob_encode_time_info = {0};
#endif

/* AOB_CALCULATE_CODEC_MIPS function */
#if AOB_CALCULATE_CODEC_MIPS
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

/**********************private function declaration*************************/
static void gaf_audio_encoder_cc_lc3_init(void);

static void gaf_audio_encoder_cc_lc3_deinit(void);

static void gaf_audio_encoder_cc_lc3_clear_buffer_pointer(void);

static void gaf_audio_encoder_cc_l3c_pcm_frame(uint32_t inputDataLength, uint8_t* input, 
                                                    uint16_t frame_size, uint8_t* output_bytes);

GAF_AUDIO_CC_ENCODER_T gaf_audio_cc_lc3_encoder_config =
{
    {44100, 1},
    gaf_audio_encoder_cc_lc3_init,
    gaf_audio_encoder_cc_lc3_deinit,
    gaf_audio_encoder_cc_l3c_pcm_frame,
    gaf_audio_encoder_cc_lc3_clear_buffer_pointer,
};

static void gaf_audio_encoder_cc_lc3_clear_buffer_pointer(void)
{
    gaf_encoder_lc3_mem_pool = NULL;
}

static void gaf_encoder_lc3_heap_init(void *begin_addr, uint32_t size)
{
    if (NULL == begin_addr)
    {
        return;
    }
    gaf_encode_lc3_heap_handle = heap_register(begin_addr, size);
}

void *gaf_stream_lc3_encode_heap_malloc(uint32_t size)
{
    uint32_t lock = int_lock();
    size = ((size >> 2) + 1) << 2;
    void *ptr = heap_malloc(gaf_encode_lc3_heap_handle, size);
    if (ptr == NULL)
    {
        LOG_I("gaf stream heap malloc failed:%d", 1);
    }
    ASSERT(ptr, "%s size:%d", __func__, size);
    int_unlock(lock);
    return ptr;
}

void gaf_stream_lc3_encode_heap_free(void *rmem)
{
    ASSERT(rmem, "%s rmem:%p", __func__, rmem);

    uint32_t lock = int_lock();
    heap_free(gaf_encode_lc3_heap_handle, rmem);
    int_unlock(lock);
}

static int gaf_stream_lc3_encode_bits(void *lc3_enc_info, void *input_samples,
    void *output_bytes, int32_t num_bytes)
{
    int err = LC3_API_OK;
    LC3_Enc_Info *p_lc3_enc_info = (LC3_Enc_Info *)lc3_enc_info;
    ASSERT(p_lc3_enc_info, "p_lc3_enc_info is empty in %s ", __func__);
#if AOB_CALCULATE_CODEC_MIPS
    uint32_t cpu_freq = 0;
    uint32_t stime = 0, etime = 0;
    uint32_t use_time_in_us = 0, total_etime_in_ms = 0;
    stime = hal_sys_timer_get();

    if (!aob_encode_time_info.codec_started)
    {
        aob_encode_time_info.codec_started = true;
        aob_encode_time_info.total_time_in_us = 0;
        aob_encode_time_info.start_time_in_ms = hal_sys_timer_get();
    }
#endif

    err = p_lc3_enc_info->cb_encode_interlaced(p_lc3_enc_info, p_lc3_enc_info->scratch,
        input_samples, output_bytes, num_bytes);

#if AOB_CALCULATE_CODEC_MIPS
    etime = hal_sys_timer_get();
    use_time_in_us = TICKS_TO_US(etime - stime);
    aob_encode_time_info.total_time_in_us += use_time_in_us;
    total_etime_in_ms = TICKS_TO_MS(etime - aob_encode_time_info.start_time_in_ms);
    cpu_freq = aob_lc3_get_sysfreq();
    aob_encode_time_info.codec_mips = cpu_freq *
        (aob_encode_time_info.total_time_in_us / 1000) / total_etime_in_ms;
    LOG_I("err %d ticks:%d time:%d us", err, (etime - stime), use_time_in_us);
    LOG_I("freq %d use:%d ms total:%d ms mips: %d M", cpu_freq,
        aob_encode_time_info.total_time_in_us/1000, total_etime_in_ms,
        aob_encode_time_info.codec_mips);
#endif
    if(err){
        LOG_E("lc3 enc err:%d\n",err);
    }
    return (int)err;
}

static void* lc3_alloc(void* pool,unsigned size){
    return gaf_stream_lc3_encode_heap_malloc(size);
}
static void lc3_free(void* pool, void*ptr){
    gaf_stream_lc3_encode_heap_free(ptr);
}

static void gaf_audio_encoder_cc_lc3_init(void)
{
    LOG_I("[%s] start", __func__);

    if (NULL == gaf_encoder_lc3_mem_pool)
    {
        off_bth_syspool_get_buff(&gaf_encoder_lc3_mem_pool, GAF_ENCODER_LC3_MEM_POOL_SIZE);
        ASSERT(gaf_encoder_lc3_mem_pool, "%s size:%d", __func__, GAF_ENCODER_LC3_MEM_POOL_SIZE);
    }
    gaf_encoder_lc3_heap_init(gaf_encoder_lc3_mem_pool, GAF_ENCODER_LC3_MEM_POOL_SIZE);

    /// m55 malloc decoder information heap in the gaf audio heap_buff
    lc3_cc_enc_info = (LC3_Enc_Info*)gaf_stream_lc3_encode_heap_malloc(sizeof(LC3_Enc_Info));
    memset((void*)lc3_cc_enc_info, 0x0, sizeof(LC3_Enc_Info));

    lc3_cc_enc_info->sample_rate  = gaf_audio_cc_lc3_encoder_config.stream_config.sample_rate;
    lc3_cc_enc_info->channels     = (int8_t)gaf_audio_cc_lc3_encoder_config.stream_config.channels;
    lc3_cc_enc_info->bitwidth     = (int8_t)gaf_audio_cc_lc3_encoder_config.stream_config.bitwidth;
    lc3_cc_enc_info->bitalign     = (lc3_cc_enc_info->bitwidth==24)?32:lc3_cc_enc_info->bitwidth;
    lc3_cc_enc_info->frame_dms    = (uint8_t)gaf_audio_cc_lc3_encoder_config.stream_config.frame_dms;
    lc3_cc_enc_info->frame_size   = (uint16_t)gaf_audio_cc_lc3_encoder_config.stream_config.frame_size;
    lc3_cc_enc_info->bitrate      = gaf_audio_cc_lc3_encoder_config.stream_config.bitrate;
    lc3_cc_enc_info->epmode       = LC3_API_EP_OFF;
    lc3_cc_enc_info->is_interlaced = 1;
    lc3_cc_enc_info->cb_alloc     = lc3_alloc;
    lc3_cc_enc_info->cb_free      = lc3_free;

    int err = LC3_API_OK;
    err = lc3_api_encoder_init(lc3_cc_enc_info);
    ASSERT(LC3_API_OK == err, "%s err %d", __func__, err);
/* Print info */
    LOG_I("[lc3]");
    LOG_I("Encoder size:     %d",              lc3_cc_enc_info->instance_size);
    LOG_I("Scratch size:     %d",              lc3_cc_enc_info->scratch_size);
    LOG_I("Sample rate:      %d",              lc3_cc_enc_info->sample_rate);
    LOG_I("Channels:         %d",              lc3_cc_enc_info->channels);
    LOG_I("Frame samples:    %d",              lc3_cc_enc_info->frame_samples);
    LOG_I("Frame length:     %d",              lc3_cc_enc_info->frame_size);
    LOG_I("Output format:    %d bits",         lc3_cc_enc_info->bitwidth);
    LOG_I("Output format:    %d bits align",   lc3_cc_enc_info->bitalign);
    LOG_I("Target bitrate:   %d",              lc3_cc_enc_info->bitrate);
    LOG_I("[%s] end", __func__);
}

static void gaf_audio_encoder_cc_lc3_deinit(void)
{
    LOG_I("[%s] start", __func__);
    if (lc3_cc_enc_info->instance){
        gaf_stream_lc3_encode_heap_free(lc3_cc_enc_info->instance);
        lc3_cc_enc_info->instance = NULL;
    }
    if (lc3_cc_enc_info->scratch){
        gaf_stream_lc3_encode_heap_free(lc3_cc_enc_info->scratch);
        lc3_cc_enc_info->scratch = NULL;
    }
    lc3_api_encoder_extra_buf_deinit(lc3_cc_enc_info);
    if (lc3_cc_enc_info){
        gaf_stream_lc3_encode_heap_free(lc3_cc_enc_info);
        lc3_cc_enc_info = NULL;
    }
    LOG_I("[%s] end", __func__);
}

static void gaf_audio_encoder_cc_l3c_pcm_frame(uint32_t inputDataLength, uint8_t* input, uint16_t frame_size, uint8_t* output_bytes)
{
    uint32_t lc3_encoded_frame_len = frame_size;
    GAF_ENCODER_CC_MEDIA_DATA_T* p_encoded_data = (GAF_ENCODER_CC_MEDIA_DATA_T*) output_bytes;
    p_encoded_data->data_len = lc3_encoded_frame_len;
#if M55_LC3_AUDIO_UPLOAD_DUMP
    uint32_t frame_samples = lc3_cc_enc_info->frame_samples;
    char* input_ptr = (char*)input;
    int input_cnt = lc3_encoded_frame_len/2;
    LOG_D("%s [len] inputDataLength %d  dump_frame_len %d channels%d", __func__,
        inputDataLength,
        input_cnt,
        lc3_cc_enc_info->channels);
    audio_dump_clear_up();
    for(uint32_t i = 0; i < lc3_cc_enc_info->channels; i++)
    {
        audio_dump_add_channel_data(i, input_ptr, frame_samples);
        input_ptr += input_cnt;
    }
    audio_dump_run();
#endif
    gaf_stream_lc3_encode_bits(lc3_cc_enc_info, input, p_encoded_data->data, p_encoded_data->data_len);
}

#endif /// GAF_ENCODER_CROSS_CORE_USE_M55
