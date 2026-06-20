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
/**
 * NOTE:
 *  1.  BES_BLE_GAF_CONTEXT_TYPE_CONVERSATIONAL is call case
 *      a. ifdef GAF_ENCODER_CROSS_CORE_USE_M55, call tx algo run in M55, so bth process rx pcm
 *      b. ifndef GAF_ENCODER_CROSS_CORE_USE_M55, bth process both tx and rx, because tx and rx algo share
 *          speech init and deinit, so just call in capture. Will fix this issue in future
 *  2.  BES_BLE_GAF_CONTEXT_TYPE_LIVE is binaural recording case
 *      a. Need to enable BINAURAL_RECORD_PROCESS
 *  3.  BES_BLE_GAF_CONTEXT_TYPE_MEDIA is music case
 * 
 * TODO:
 *  1.  Separate speech init and deinit for capture and playback
 *  2.  Provice API to get dynamic RAM from call and binaural recording
 **/
#include "gaf_stream_process.h"
#include "bes_aob_api.h"
// Platform
#include "cmsis.h"
#include "string.h"
#include "hal_trace.h"
#include "audio_dump.h"
// Algo process
#include "bt_sco_chain.h"
#include "audio_process.h"
#include "speech_memory.h"
#include "arm_math_ex.h"
#include "iir_resample.h"
#ifdef BINAURAL_RECORD_PROCESS
#include "binaural_record_process.h"
#endif
#include "signal_generator.h"

// #define GAF_STREAM_CAPTURE_AUDIO_DUMP
// #define GAF_STREAM_CAPTURE_1K_TONE
// #define GAF_CAPTURE_UPSAMPLING_DUMP

static uint16_t g_capture_stream = BES_BLE_GAF_CONTEXT_TYPE_UNSPECIFIED_BIT;
static uint16_t g_playback_stream = BES_BLE_GAF_CONTEXT_TYPE_UNSPECIFIED_BIT;

static struct AF_STREAM_CONFIG_T g_capture_stream_cfg;
static struct AF_STREAM_CONFIG_T g_playback_stream_cfg;

static uint8_t *g_capture_stream_buf = NULL;
static uint8_t *g_playback_stream_buf = NULL;
static uint32_t g_capture_stream_buf_size = 0;
static uint32_t g_playback_stream_buf_size = 0;

#ifndef GAF_ENCODER_CROSS_CORE_USE_M55
static uint8_t *g_aec_echo_buf = NULL;
static uint32_t g_capture_upsampling_factor = 1;
static IirResampleState *g_capture_upsampling_st = NULL;
#endif

uint8_t *g_capture_upsampling_buf = NULL;
uint32_t g_capture_upsampling_buf_size = 0;

bool gaf_stream_process_context_is_call(uint16_t context)
{
    if ((context == BES_BLE_GAF_CONTEXT_TYPE_CONVERSATIONAL_BIT) ||
    (context == BES_BLE_GAF_CONTEXT_TYPE_RINGTONE_BIT)) {
        return true;
    } else {
        return false;
    }
}

bool gaf_stream_process_context_is_binaural_record(uint16_t context)
{
    if (context == BES_BLE_GAF_CONTEXT_TYPE_LIVE_BIT) {
        return true;
    } else {
        return false;
    }
}

// NOTE: It's a workaround method
extern "C" bool is_le_call_mode(void)
{
#ifdef GAF_ENCODER_CROSS_CORE_USE_M55
    // NOTE: Capture algo is running on M55, so don't need to to do any algo on M33.
    return gaf_stream_process_context_is_call(g_playback_stream);
#else
    return gaf_stream_process_context_is_call(g_capture_stream);
#endif
}

extern "C" bool is_le_media_mode(void)
{
    return (g_playback_stream == BES_BLE_GAF_CONTEXT_TYPE_MEDIA_BIT);
}

int32_t gaf_stream_process_init(void)
{
    return 0;
}

int32_t gaf_stream_process_deinit(void)
{
    return 0;
}

int32_t gaf_stream_process_ctrl(void)
{
    return 0;
}

/******************** Capture API ********************/
uint32_t gaf_stream_process_need_capture_buf_size(uint16_t stream)
{
    uint32_t size =0;

    if (0) {
#ifndef GAF_ENCODER_CROSS_CORE_USE_M55
    } else if (gaf_stream_process_context_is_call(stream)) {
        size = 1024 * 60;
        TRACE(1, "[%s] FIXME: Check Call tx algo RAM usage", __func__);
#endif
#ifdef BINAURAL_RECORD_PROCESS
    } else if (gaf_stream_process_context_is_binaural_record(stream)) {
        size = 1024 * 50;
#endif
    }

    TRACE(3, "[%s] stream:0x%x, size: %d", __func__, stream, size);

    return size;
}

int32_t gaf_stream_process_set_capture_buf(uint8_t *buf, uint32_t size)
{
    TRACE(2, "[%s] size: %d", __func__, size);

    g_capture_stream_buf = buf;
    g_capture_stream_buf_size = size;

    return 0;
}
uint32_t gaf_stream_process_need_upsampling_buf_size(uint16_t stream)
{
    if (gaf_stream_process_context_is_call(stream)) {
        return g_capture_upsampling_buf_size;
    } else {
        return 0;
    }
}

int32_t gaf_stream_process_set_upsampling_buf(uint8_t *buf, uint32_t size)
{
    TRACE(2, "[%s] size: %d", __func__, size);

    g_capture_upsampling_buf = buf;
    g_capture_upsampling_buf_size = size;

    return 0;
}

uint8_t *gaf_stream_process_get_upsampling_buf(void)
{
    return g_capture_upsampling_buf;
}

int32_t gaf_stream_process_capture_open(uint16_t stream, struct AF_STREAM_CONFIG_T *stream_cfg)
{
    uint32_t frame_len = stream_cfg->data_size / stream_cfg->channel_num / (stream_cfg->bits <= AUD_BITS_16 ? 2 : 4) / 2;

    TRACE(6, "[%s] stream: %d, sample_rate: %d, bits: %d, ch_num: %d, frame_len: %d", __func__, stream,
        stream_cfg->sample_rate,
        stream_cfg->bits,
        stream_cfg->channel_num,
        frame_len);

    memcpy(&g_capture_stream_cfg, stream_cfg, sizeof(struct AF_STREAM_CONFIG_T));

#ifdef GAF_STREAM_CAPTURE_AUDIO_DUMP
    audio_dump_init(frame_len, sizeof(int32_t), 1);
#endif

#ifdef GAF_STREAM_CAPTURE_1K_TONE
    signal_generator_init(SG_TYPE_TONE_SIN_1K, stream_cfg->sample_rate, stream_cfg->bits, 1);
#endif

    if (gaf_stream_process_context_is_call(stream)) {
        // FIXME: It's a workaround method
        // g_capture_stream = BES_BLE_GAF_CONTEXT_TYPE_CONVERSATIONAL_BIT;
#ifndef GAF_ENCODER_CROSS_CORE_USE_M55
#if defined(SPEECH_ALGO_DSP)
        speech_enable_mcpp(true);
#endif
        uint32_t rx_frame_len = g_playback_stream_cfg.data_size / g_playback_stream_cfg.channel_num / (g_playback_stream_cfg.bits <= AUD_BITS_16 ? 2 : 4) / 2;
        if (g_playback_stream_cfg.sample_rate != 0){
            speech_init2(stream_cfg->sample_rate, g_playback_stream_cfg.sample_rate, frame_len, rx_frame_len, frame_len, g_capture_stream_buf, g_capture_stream_buf_size);
        } else {
            speech_init2(stream_cfg->sample_rate, stream_cfg->sample_rate, frame_len, rx_frame_len, frame_len, g_capture_stream_buf, g_capture_stream_buf_size);
        }
        g_aec_echo_buf = (uint8_t *)speech_calloc(frame_len, (stream_cfg->bits <= AUD_BITS_16 ? 2 : 4));
#endif
    } else if (gaf_stream_process_context_is_binaural_record(stream)) {
        TRACE(1, "[%s] Binaural Recording...", __func__);
#ifdef BINAURAL_RECORD_PROCESS
        uint32_t sample_rate = stream_cfg->sample_rate;
#ifdef BINAURAL_RECORD_ADC_HIGH_SAMPLE_RATE
        extern uint32_t gaf_stream_get_real_capture_sample_rate(void);
        sample_rate = gaf_stream_get_real_capture_sample_rate();
#endif
        binaural_record_process_init(sample_rate, stream_cfg->bits, stream_cfg->channel_num, frame_len, g_capture_stream_buf, g_capture_stream_buf_size);
#endif
    } else {
        TRACE(2, "[%s] WARNING: %d is invalid stream", __func__, stream);
        return -1;
    }

    g_capture_stream = stream;

    return 0;
}

int32_t gaf_stream_process_capture_upsampling_init(GAF_AUDIO_STREAM_INFO_T *stream_info, struct AF_STREAM_CONFIG_T *stream_cfg)
{
    uint16_t stream = stream_info->bap_contextType;
#ifndef GAF_ENCODER_CROSS_CORE_USE_M55
    uint32_t frame_len = stream_cfg->data_size / stream_cfg->channel_num / (stream_cfg->bits <= AUD_BITS_16 ? 2 : 4) / 2;
#endif

    if (gaf_stream_process_context_is_call(stream)) {
#ifndef GAF_ENCODER_CROSS_CORE_USE_M55
        if ((uint32_t)stream_info->captureInfo.sample_rate != (uint32_t)stream_cfg->sample_rate) {
            ASSERT(stream_info->captureInfo.sample_rate % stream_cfg->sample_rate == 0, "[%s] %d, %d. Can not do resample", __func__, stream_info->captureInfo.sample_rate, stream_cfg->sample_rate);
            g_capture_upsampling_factor = stream_info->captureInfo.sample_rate / stream_cfg->sample_rate;
            if (stream_cfg->channel_num < g_capture_upsampling_factor){
                TRACE(3, "[%s] ch_num: %d, factor: %d",  __func__, stream_cfg->channel_num, g_capture_upsampling_factor);
                g_capture_upsampling_buf_size = stream_cfg->data_size * g_capture_upsampling_factor / stream_cfg->channel_num / 2;
            } else {
                g_capture_upsampling_buf_size = 0;
            }
            TRACE(4, "[%s] Resample %d--> %d. factor: %d", __func__, stream_cfg->sample_rate, stream_info->captureInfo.sample_rate, g_capture_upsampling_factor);

            g_capture_upsampling_st = multi_iir_resample_init(frame_len,
                                                            stream_cfg->bits,
                                                            1,
                                                            iir_resample_choose_mode(stream_cfg->sample_rate, stream_info->captureInfo.sample_rate));
        } else {
            g_capture_upsampling_st = NULL;
            g_capture_upsampling_factor = 1;
            g_capture_upsampling_buf_size = 0;
        }

#ifdef GAF_CAPTURE_UPSAMPLING_DUMP
        data_dump_init();
#endif
#endif
    }

    return 0;
}

int32_t gaf_stream_process_capture_close(void)
{
    TRACE(2, "[%s] stream: %d", __func__, g_capture_stream);

#ifdef GAF_STREAM_CAPTURE_1K_TONE
    signal_generator_deinit();
#endif

    if (gaf_stream_process_context_is_call(g_capture_stream)) {
#ifndef GAF_ENCODER_CROSS_CORE_USE_M55
        speech_free(g_aec_echo_buf);
        speech_deinit();
#endif
    } else if (gaf_stream_process_context_is_binaural_record(g_capture_stream)) {
        TRACE(1, "[%s] Binaural Recording...", __func__);
#ifdef BINAURAL_RECORD_PROCESS
        binaural_record_process_deinit();
#endif
    } else {
        TRACE(2, "[%s] %d is invalid stream", __func__, g_capture_stream);
        return -1;
    }

    g_capture_stream = BES_BLE_GAF_CONTEXT_TYPE_UNSPECIFIED_BIT;

    return 0;
}

int32_t gaf_stream_process_capture_upsampling_deinit(void)
{
    if (gaf_stream_process_context_is_call(g_capture_stream)) {
#ifndef GAF_ENCODER_CROSS_CORE_USE_M55
        if (g_capture_upsampling_st) {
            iir_resample_destroy(g_capture_upsampling_st);
            g_capture_upsampling_st = NULL;
            g_capture_upsampling_factor = 1;
        }

        g_capture_upsampling_buf_size = 0;

#ifdef GAF_CAPTURE_UPSAMPLING_DUMP
        data_dump_deinit();
#endif
#endif
    }
    return 0;
}

template <typename T>
static void gaf_stream_split_capture_data(T *pcm_buf, T *echo_buf, uint32_t pcm_len)
{
    T *pcm_buf_j = pcm_buf;
    T *pcm_buf_i = pcm_buf;
    uint32_t count = pcm_len / (SPEECH_CODEC_CAPTURE_CHANNEL_NUM + 1);
    while (count-- > 0) {
#if SPEECH_CODEC_CAPTURE_CHANNEL_NUM == 1
        *pcm_buf_j++ = *pcm_buf_i++;
#elif SPEECH_CODEC_CAPTURE_CHANNEL_NUM == 2
        *pcm_buf_j++ = *pcm_buf_i++;
        *pcm_buf_j++ = *pcm_buf_i++;
#elif SPEECH_CODEC_CAPTURE_CHANNEL_NUM == 3
        *pcm_buf_j++ = *pcm_buf_i++;
        *pcm_buf_j++ = *pcm_buf_i++;
        *pcm_buf_j++ = *pcm_buf_i++;
#elif SPEECH_CODEC_CAPTURE_CHANNEL_NUM == 4
        *pcm_buf_j++ = *pcm_buf_i++;
        *pcm_buf_j++ = *pcm_buf_i++;
        *pcm_buf_j++ = *pcm_buf_i++;
        *pcm_buf_j++ = *pcm_buf_i++;
#elif SPEECH_CODEC_CAPTURE_CHANNEL_NUM == 5
        *pcm_buf_j++ = *pcm_buf_i++;
        *pcm_buf_j++ = *pcm_buf_i++;
        *pcm_buf_j++ = *pcm_buf_i++;
        *pcm_buf_j++ = *pcm_buf_i++;
        *pcm_buf_j++ = *pcm_buf_i++;
#else
#error "Invalid SPEECH_CODEC_CAPTURE_CHANNEL_NUM!!!"
#endif
#if defined(SPEECH_TX_AEC) || defined(SPEECH_TX_AEC2) || defined(SPEECH_TX_AEC3) || defined(SPEECH_TX_AEC2FLOAT) || defined(SPEECH_TX_3MIC_NS) || defined(SPEECH_TX_2MIC_NS4) || defined(SPEECH_TX_THIRDPARTY)
        *echo_buf++ = *pcm_buf_i++;
#endif
    }
}

static int gaf_stream_process_capture_data_extraction(void *buf, uint32_t* len, AUD_BITS_T bits, AUD_CHANNEL_NUM_T channel_num)
{
    uint32_t POSSIBLY_UNUSED sample_bytes = bits <= AUD_BITS_16 ? 2 : 4;
    uint32_t POSSIBLY_UNUSED pcm_len = *len / sample_bytes;
    pcm_len = pcm_len / channel_num;

    if (bits == AUD_BITS_16) {
        int16_t *pcm_buf = (int16_t *)buf;
        for (uint32_t i = 0; i < pcm_len; i++) {
            pcm_buf[i] = pcm_buf[i * channel_num];
        }
    } else if (bits == 24) {
        int32_t *pcm_buf = (int32_t *)buf;
        for (uint32_t i = 0; i < pcm_len; i++) {
            pcm_buf[i] = pcm_buf[i * channel_num];
        }
    }

    return pcm_len;

}

uint32_t gaf_stream_process_capture_run(uint8_t *buf, uint32_t len)
{
    uint32_t POSSIBLY_UNUSED sample_bytes = g_capture_stream_cfg.bits <= AUD_BITS_16 ? 2 : 4;
    uint32_t POSSIBLY_UNUSED pcm_len = len / sample_bytes;

    if (gaf_stream_process_context_is_call(g_capture_stream)) {
#ifndef GAF_ENCODER_CROSS_CORE_USE_M55
        if (g_capture_stream_cfg.channel_num == (SPEECH_CODEC_CAPTURE_CHANNEL_NUM + 1)){
            ASSERT(pcm_len % (SPEECH_CODEC_CAPTURE_CHANNEL_NUM + 1) == 0, "[%s] pcm_len(%d) should be divided by %d", __FUNCTION__, pcm_len, SPEECH_CODEC_CAPTURE_CHANNEL_NUM + 1);
            if (g_capture_stream_cfg.bits == AUD_BITS_16){
                gaf_stream_split_capture_data((int16_t *)buf, (int16_t *)g_aec_echo_buf, pcm_len);
            } else if (g_capture_stream_cfg.bits == AUD_BITS_24){
                gaf_stream_split_capture_data((int32_t *)buf, (int32_t *)g_aec_echo_buf, pcm_len);
            }
            pcm_len = pcm_len / (SPEECH_CODEC_CAPTURE_CHANNEL_NUM + 1) * SPEECH_CODEC_CAPTURE_CHANNEL_NUM;
        }

        speech_tx_process(buf, g_aec_echo_buf, (int32_t *)&pcm_len);
        len = pcm_len * sample_bytes;
#endif
    } else if (gaf_stream_process_context_is_binaural_record(g_capture_stream)) {
#ifdef BINAURAL_RECORD_PROCESS
        binaural_record_process_run(buf, pcm_len);
#endif
    } else {
        pcm_len = gaf_stream_process_capture_data_extraction(buf, (uint32_t *)&len, g_capture_stream_cfg.bits, g_capture_stream_cfg.channel_num);
        len = pcm_len * sample_bytes;
        // Add 18 dB gain for other casse
        if (g_capture_stream_cfg.bits == AUD_BITS_16) {
            int16_t *pcm_buf = (int16_t *)buf;
            for (uint32_t i=0; i<pcm_len; i++) {
                pcm_buf[i] = __SSAT(((int32_t)(pcm_buf[i])) << 3, 16);
            }
        } else if (g_capture_stream_cfg.bits == 24) {
            int32_t *pcm_buf = (int32_t *)buf;
            for (uint32_t i=0; i<pcm_len; i++) {
                pcm_buf[i] = __SSAT(pcm_buf[i] << 3, 24);
            }
        } else {
            ASSERT(0, "[%s] bits(%d) is invalid", __func__, g_capture_stream_cfg.bits);
        }
    }

#ifdef GAF_STREAM_CAPTURE_1K_TONE
    signal_generator_loop_get_data(buf, pcm_len);
#endif

#ifdef GAF_STREAM_CAPTURE_AUDIO_DUMP
    audio_dump_add_channel_data(0, buf, pcm_len);
    audio_dump_run();
#endif

    return len;
}

uint32_t gaf_stream_process_capture_upsampling_run(uint8_t *buf, uint32_t len)
{
    uint32_t POSSIBLY_UNUSED sample_bytes = g_capture_stream_cfg.bits <= AUD_BITS_16 ? 2 : 4;
    uint32_t POSSIBLY_UNUSED pcm_len = len / sample_bytes;

    if (gaf_stream_process_context_is_call(g_capture_stream)) {
#ifndef GAF_ENCODER_CROSS_CORE_USE_M55
#ifdef GAF_CAPTURE_UPSAMPLING_DUMP
        data_dump_run(0, "tx_algo_16k", buf, len);
#endif
        if (g_capture_upsampling_st) {
            if (g_capture_upsampling_buf){
                iir_resample_process(g_capture_upsampling_st, buf, g_capture_upsampling_buf, pcm_len);
            } else {
                iir_resample_process(g_capture_upsampling_st, buf, buf, pcm_len);
            }
            len *= g_capture_upsampling_factor;
        }
#ifdef GAF_CAPTURE_UPSAMPLING_DUMP
        if (g_capture_upsampling_buf){
            data_dump_run(1, "tx_algo_32k", g_capture_upsampling_buf, len);
        }else{
            data_dump_run(1, "tx_algo_32k", buf, len);
        }
#endif
#endif
    }
    return len;
}

/******************** Playback API ********************/
uint32_t gaf_stream_process_need_playback_buf_size(uint16_t stream)
{
    uint32_t size =0;

    if (0) {
#ifdef GAF_ENCODER_CROSS_CORE_USE_M55
    } else if (gaf_stream_process_context_is_call(stream)) {
        /**
         * RX Algo:
         *  16k sample rate: 15k RAM
         *  32k sample rate: 25k RAM
         */
        // size = 1024 * 30;
        TRACE(1, "[%s] FIXME: Check Call rx algo RAM usage", __func__);
#endif
    } else if (stream == BES_BLE_GAF_CONTEXT_TYPE_MEDIA_BIT) {
        size = audio_process_need_audio_buf_size();
    }

    TRACE(3, "[%s] stream:0x%x, size: %d", __func__, stream, size);

    return size;
}

int32_t gaf_stream_process_set_playback_buf(uint8_t *buf, uint32_t size)
{
    TRACE(2, "[%s] size: %d", __func__, size);

    g_playback_stream_buf = buf;
    g_playback_stream_buf_size = size;

    return 0;
}

int32_t gaf_stream_process_playback_open(uint16_t stream, struct AF_STREAM_CONFIG_T *stream_cfg)
{
    uint32_t frame_len = stream_cfg->data_size / stream_cfg->channel_num / (stream_cfg->bits <= AUD_BITS_16 ? 2 : 4) / 2;

    TRACE(6, "[%s] stream: %d, sample_rate: %d, bits: %d, ch_num: %d, frame_len: %d", __func__, stream,
        stream_cfg->sample_rate,
        stream_cfg->bits,
        stream_cfg->channel_num,
        frame_len);

    memcpy(&g_playback_stream_cfg, stream_cfg, sizeof(struct AF_STREAM_CONFIG_T));

    if (gaf_stream_process_context_is_call(stream)) {
        // FIXME: It's a workaround method
        // g_playback_stream = APP_BAP_CONTEXT_TYPE_CONVERSATIONAL;
#ifdef GAF_ENCODER_CROSS_CORE_USE_M55
        speech_init(stream_cfg->sample_rate, stream_cfg->sample_rate, frame_len, frame_len, 0, g_playback_stream_buf, g_playback_stream_buf_size);
#endif
    } else if (stream == BES_BLE_GAF_CONTEXT_TYPE_MEDIA_BIT) {
        audio_process_open(stream_cfg->sample_rate,
            stream_cfg->bits, stream_cfg->channel_num, stream_cfg->channel_num, frame_len, g_playback_stream_buf, g_playback_stream_buf_size);

        void app_le_set_dac_eq(void);
        app_le_set_dac_eq();
    } else {
        TRACE(2, "[%s] WARNING: %d is invalid stream", __func__, stream);
        return -1;
    }
#if defined(AUDIO_OUTPUT_SW_GAIN) && defined(AUDIO_OUTPUT_SW_GAIN_BEFORE_DRC)
        af_codec_dac1_sw_gain_enable(false);
#endif
    g_playback_stream = stream;

    return 0;
}

int32_t gaf_stream_process_playback_close(void)
{
    TRACE(2, "[%s] stream: %d", __func__, g_playback_stream);

    if (gaf_stream_process_context_is_call(g_playback_stream)) {
#ifdef GAF_ENCODER_CROSS_CORE_USE_M55
        speech_deinit();
#endif
    } else if (g_playback_stream == BES_BLE_GAF_CONTEXT_TYPE_MEDIA_BIT) {
        audio_process_close();
    } else {
        TRACE(2, "[%s] %d is invalid stream", __func__, g_playback_stream);
        return -1;
    }

    g_playback_stream = BES_BLE_GAF_CONTEXT_TYPE_UNSPECIFIED_BIT;

    return 0;
}

uint32_t gaf_stream_process_playback_run(uint8_t *buf, uint32_t len)
{
    uint32_t POSSIBLY_UNUSED sample_bytes = g_playback_stream_cfg.bits <= AUD_BITS_16 ? 2 : 4;
    uint32_t POSSIBLY_UNUSED pcm_len = len / sample_bytes;

#ifdef GAF_ENCODER_CROSS_CORE_USE_M55
    if (gaf_stream_process_context_is_call(g_playback_stream)) {
#else
    if ((gaf_stream_process_context_is_call(g_playback_stream)) && (gaf_stream_process_context_is_call(g_capture_stream))) {
#endif
        speech_rx_process(buf, (int32_t *)&pcm_len);
        len = pcm_len * sample_bytes;
    } else if (g_playback_stream == BES_BLE_GAF_CONTEXT_TYPE_MEDIA_BIT) {
        audio_process_run(buf, len);
    }

    return len;
}
