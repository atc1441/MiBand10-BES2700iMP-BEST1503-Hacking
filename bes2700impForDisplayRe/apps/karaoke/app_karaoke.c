/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#include "app_utils.h"
#include "audioflinger.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_codec.h"
#include "string.h"
#include "audio_dump.h"
#include "app_audio.h"
#include "speech_cfg.h"
#include "app_overlay.h"
#include "audio_process_vol.h"

// #define KARAOKE_AUDIO_DUMP

// #define VOICE_COMPEXP
#if defined(KARAOKE_ALGO_EQ)
#define VOICE_EQ
#endif

#if defined(KARAOKE_ALGO_NS)
#define VOICE_NS2FLOAT
#endif

#if defined(VOICE_NS2FLOAT)
#define KARAOKE_HEAP_BUF_SIZE       (1024*15)
#else
#define KARAOKE_HEAP_BUF_SIZE       (1024*4)
#endif

#define CHAR_BYTES          (1)
#define SHORT_BYTES         (2)
#define INT_BYTES           (4)

#define SAMPLE_BITS         (24)
#define SAMPLE_BYTES        (2 * ((SAMPLE_BITS + 8) / 16))

#define FRAME_LEN           (128)

#define TX_SAMPLE_RATE      (16000)
#define RX_SAMPLE_RATE      (16000)

#define TX_FRAME_BUF_SIZE   (FRAME_LEN * SAMPLE_BYTES * 2)
#define RX_FRAME_BUF_SIZE   (FRAME_LEN * SAMPLE_BYTES * 2)

#if SAMPLE_BYTES == SHORT_BYTES
typedef short   VOICE_PCM_T;
#elif SAMPLE_BYTES == INT_BYTES
typedef int     VOICE_PCM_T;
#else
#error "Invalid SAMPLE_BYTES!!!"
#endif

#define CODEC_STREAM_ID     AUD_STREAM_ID_3

// static uint8_t codec_capture_buf[TX_FRAME_BUF_SIZE];
// static uint8_t codec_playback_buf[RX_FRAME_BUF_SIZE];

// static uint8_t g_karaoke_heap_buf[KARAOKE_HEAP_BUF_SIZE];
// static VOICE_PCM_T g_loop_buf[FRAME_LEN];

static uint8_t *codec_capture_buf = NULL;
static uint8_t *codec_playback_buf = NULL;

static uint8_t *g_karaoke_heap_buf = NULL;
static VOICE_PCM_T *g_loop_buf = NULL;

#ifdef VOICE_NS2FLOAT
static SpeechNs2FloatConfig voice_ns2float_cfg = {
    .bypass     = 0,
    .denoise_dB = -15,
    .banks      = 64,
};
#endif

#ifdef VOICE_COMPEXP
static MultiCompexpConfig voice_compexp_cfg = {
    .bypass = 0,
    .num = 2,
    .xover_freq = {5000},
    .order = 4,
    .params = {
        {
            .bypass             = 0,
            .type               = 0,
            .comp_threshold     = -10.f,
            .comp_ratio         = 2.f,
            .expand_threshold   = -60.f,
            .expand_ratio       = 0.5556f,
            .attack_time        = 0.001f,
            .release_time       = 0.006f,
            .makeup_gain        = 0,
            .delay              = 128,
            .tav                = 0.2f,
        },
        {
            .bypass             = 0,
            .type               = 0,
            .comp_threshold     = -10.f,
            .comp_ratio         = 2.f,
            .expand_threshold   = -60.f,
            .expand_ratio       = 0.5556f,
            .attack_time        = 0.001f,
            .release_time       = 0.006f,
            .makeup_gain        = 0,
            .delay              = 128,
            .tav                = 0.2f,
        }
    }
};
#endif

#ifdef VOICE_EQ
static EqConfig voice_eq_cfg = {
    .bypass = 0,
    .gain   = 0.f,
    .num    = 1,
    .params = {
        {IIR_BIQUARD_PEAKINGEQ, {{1000, 6, 0.707f}}},
    },
};
#endif

#if defined(VOICE_NS2FLOAT)
static SpeechNs2FloatState *voice_ns2float_st = NULL;
#endif

#if defined(VOICE_COMPEXP)
static MultiCompexpState *voice_compexp_st = NULL;
#endif

#if defined(VOICE_EQ)
static EqState *voice_eq_st = NULL;
#endif

uint32_t app_karaoke_need_buf_size(void)
{
    return KARAOKE_HEAP_BUF_SIZE;
}

int32_t app_karaoke_set_buf(uint8_t *buf, uint32_t size)
{
    TRACE(0, "[%s] size: %d", __func__, size);
    // g_karaoke_heap_buf = (uint8_t *)app_audio_mempool_calloc(KARAOKE_HEAP_BUF_SIZE, sizeof(uint8_t));
    g_karaoke_heap_buf = buf;

    return 0;
}

static uint32_t codec_capture_callback(uint8_t *buf, uint32_t len)
{
    int POSSIBLY_UNUSED frame_len = len / sizeof(VOICE_PCM_T);

#if defined(VOICE_NS2FLOAT)
    speech_ns2float_process_int24(voice_ns2float_st, (int32_t *)buf, frame_len);
#endif

#if defined(VOICE_COMPEXP)
    multi_compexp_process_int24(voice_compexp_st, (int32_t *)buf, frame_len);
#endif

#if defined(VOICE_EQ)
    eq_process_int24(voice_eq_st, (int32_t *)buf, frame_len);
#endif

    memcpy(g_loop_buf, buf, frame_len * sizeof(VOICE_PCM_T));

#if defined(KARAOKE_AUDIO_DUMP)
    audio_dump_run();
#endif

    return len;
}

static uint32_t codec_playback_callback(uint8_t *buf, uint32_t len)
{
    int frame_len = len / sizeof(VOICE_PCM_T);

    memcpy(buf, g_loop_buf, frame_len * sizeof(VOICE_PCM_T));

    audio_process_vol_run((int32_t *)buf, frame_len);

    return len;
}

int32_t app_karaoke_start(bool on)
{
    int ret = 0;
    static bool isRun =  false;

    /*If it crashes nearby,please lower the frequency*/
    enum APP_SYSFREQ_FREQ_T freq = APP_SYSFREQ_208M;
    struct AF_STREAM_CONFIG_T stream_cfg;

    if (isRun == on) {
        return 0;
    }

    if (on) {
        TRACE(1, "[%s]] ON", __func__);

        af_set_priority(AF_USER_TEST, osPriorityHigh);

        app_sysfreq_req(APP_SYSFREQ_USER_APP_0, freq);
        // TRACE(2, "[%s] sys freq calc : %d\n", __func__, hal_sys_timer_calc_cpu_freq(5, 0));
#if 1
        if (g_karaoke_heap_buf == NULL) {
            TRACE(0, "[%s] Need to set heap buf firstly", __func__);
            return 1;
        }
#else
        app_audio_mempool_init_with_specific_size(app_audio_mempool_original_size());
        g_karaoke_heap_buf = (uint8_t *)app_audio_mempool_calloc(KARAOKE_HEAP_BUF_SIZE, sizeof(uint8_t));
#endif
        speech_heap_init(g_karaoke_heap_buf, KARAOKE_HEAP_BUF_SIZE);

        codec_capture_buf = (uint8_t *)speech_calloc(TX_FRAME_BUF_SIZE, sizeof(uint8_t));
        // codec_capture_buf = BT_AUDIO_CACHE_2_UNCACHE(codec_capture_buf);
        codec_playback_buf = (uint8_t *)speech_calloc(RX_FRAME_BUF_SIZE, sizeof(uint8_t));
        // codec_playback_buf = BT_AUDIO_CACHE_2_UNCACHE(codec_playback_buf);

        g_loop_buf = (VOICE_PCM_T *)speech_calloc(FRAME_LEN, sizeof(VOICE_PCM_T));

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_num = AUD_CHANNEL_NUM_1;
        stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)(AUD_CHANNEL_MAP_CH0);
        stream_cfg.data_size = TX_FRAME_BUF_SIZE;
        stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)TX_SAMPLE_RATE;
        stream_cfg.bits = (enum AUD_BITS_T)SAMPLE_BITS;
        stream_cfg.vol = 12;
        stream_cfg.chan_sep_buf = true;
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.io_path = AUD_INPUT_PATH_MAINMIC;
        stream_cfg.handler = codec_capture_callback;
        stream_cfg.data_ptr = codec_capture_buf;

        TRACE(3, "[%s] Capture: sample rate: %d, data size: %d, channel num: %d", __func__, stream_cfg.sample_rate, stream_cfg.data_size, stream_cfg.channel_num);
        af_stream_open(CODEC_STREAM_ID, AUD_STREAM_CAPTURE, &stream_cfg);
        // audio_process_vol_start_impl(1, ms);
        ASSERT(ret == 0, "codec capture failed: %d", ret);

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_num = AUD_CHANNEL_NUM_1;
        stream_cfg.data_size = RX_FRAME_BUF_SIZE;
        stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)RX_SAMPLE_RATE;
        stream_cfg.bits = (enum AUD_BITS_T)SAMPLE_BITS;
        stream_cfg.vol = 12;
        stream_cfg.chan_sep_buf = true;
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC2;
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.handler = codec_playback_callback;
        stream_cfg.data_ptr = codec_playback_buf;

        TRACE(3, "[%s] Playback: sample rate: %d, data_size: %d, channel num: %d", __func__, stream_cfg.sample_rate, stream_cfg.data_size, stream_cfg.channel_num);
        af_stream_open(CODEC_STREAM_ID, AUD_STREAM_PLAYBACK, &stream_cfg);
        ASSERT(ret == 0, "codec playback failed: %d", ret);

#if defined(KARAOKE_AUDIO_DUMP)
        audio_dump_init(FRAME_LEN, sizeof(VOICE_PCM_T), tx_channel_num);
#endif

        audio_process_vol_init(stream_cfg.sample_rate, stream_cfg.bits, stream_cfg.channel_num, 1.0);

#if defined(VOICE_NS2FLOAT)
        voice_ns2float_st = speech_ns2float_create(stream_cfg.sample_rate, FRAME_LEN, 0, &voice_ns2float_cfg);
#endif

#if defined(VOICE_COMPEXP)
        voice_compexp_st = multi_compexp_create(stream_cfg.sample_rate, FRAME_LEN, &voice_compexp_cfg);
#endif

#if defined(VOICE_EQ)
        voice_eq_st = eq_init(stream_cfg.sample_rate, FRAME_LEN, &voice_eq_cfg);
#endif

        // Start
        af_stream_start(CODEC_STREAM_ID, AUD_STREAM_CAPTURE);
        af_stream_start(CODEC_STREAM_ID, AUD_STREAM_PLAYBACK);
    } else {
        // Close stream
        af_stream_stop(CODEC_STREAM_ID, AUD_STREAM_PLAYBACK);
        af_stream_stop(CODEC_STREAM_ID, AUD_STREAM_CAPTURE);

#if defined(KARAOKE_AUDIO_DUMP)
        audio_dump_deinit();
#endif

        af_stream_close(CODEC_STREAM_ID, AUD_STREAM_PLAYBACK);
        af_stream_close(CODEC_STREAM_ID, AUD_STREAM_CAPTURE);

#if defined(VOICE_EQ)
        eq_destroy(voice_eq_st);
#endif

#if defined(VOICE_COMPEXP)
        multi_compexp_destroy(voice_compexp_st);
#endif

#if defined(VOICE_NS2FLOAT)
        speech_ns2float_destroy(voice_ns2float_st);
#endif

        speech_free(codec_capture_buf);
        codec_capture_buf = NULL;

        speech_free(codec_playback_buf);
        codec_playback_buf = NULL;

        speech_free(g_loop_buf);
        g_loop_buf = NULL;

        size_t total = 0, used = 0, max_used = 0;
        speech_memory_info(&total, &used, &max_used);
        TRACE(3,"APP KARAO RAM: total - %d, used - %d, max_used - %d.", total, used, max_used);
        ASSERT(used == 0, "[%s] used != 0", __func__);

        app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_32K);

        af_set_priority(AF_USER_TEST, osPriorityAboveNormal);

        TRACE(1, "[%s] OFF", __func__);
    }

    isRun=on;
    return 0;
}