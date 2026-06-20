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
/**
 * RAM Usage:
 *  4k:  _get_ext_buf
 *  21k: audioflinger, data buffer: Add 21k≈(23040 - 1920) / 1024
 */
#include "app_utils.h"
#include "hal_trace.h"
#include "audioflinger.h"
#include "heap_api.h"
#include "hal_aud.h"
#include "audio_dump.h"
#include "speech_ssat.h"
#include "app_voice_assist_ai_voice.h"
#include "resample_coef.h"
#include "app_ai_if.h"

// #define RESAMPLE_AI_VOICE_AUDIO_DUMP

#if defined(ANC_ASSIST_ENABLED)
#include "app_anc_assist.h"
#endif

#if defined(RECORDING_USE_SCALABLE)
#if defined(PSAP_APP)
/* 32k is not available when PSAP_APP is enable */
#define SRC_SAMPLE_RATE     (48000)
#else
#define SRC_SAMPLE_RATE     (32000)
#endif
#define DST_SAMPLE_RATE     (32000)
#else /* RECORDING_USE_OPUS */
#define SRC_SAMPLE_RATE     (48000)
#define DST_SAMPLE_RATE     (48000)
#endif

#define _RESAMPLE_ENABLE   (DST_SAMPLE_RATE != SRC_SAMPLE_RATE)

#if _RESAMPLE_ENABLE
static uint32_t _resample_open_flag = 0;
#endif

#define FRAME_LEN_16K_15MS          (240)
#define FRAME_LEN_32K_15MS          (FRAME_LEN_16K_15MS * 2)
#define FRAME_LEN_48K_15MS          (FRAME_LEN_16K_15MS * 3)
#define CHANNEL_NUM         (4)

static struct AF_STREAM_CONFIG_T g_stream_cfg;
static uint8_t *g_extern_data_ptr   = NULL;
static uint32_t g_extern_data_size  = 0;
static AF_STREAM_HANDLER_T g_extern_handler = NULL;
static mempool_get_buff_t g_mempool_get_buff = NULL;

static int16_t POSSIBLY_UNUSED *g_ai_buf = NULL;
static uint32_t g_ext_buf_used_size = 0;

static void POSSIBLY_UNUSED *_get_ext_buf(int size, uint8_t user_index)
{
    uint8_t *pBuf = NULL;

    if (size % 4) {
        size = size + (4 - size % 4);
    }

    g_mempool_get_buff(&pBuf, size, user_index);
    g_ext_buf_used_size += size;

    return (void *)pBuf;
}

static void _ext_buf_usage_start(void)
{
    g_ext_buf_used_size = 0;
}

static void _ext_buf_usage_end(void)
{
    TRACE(0, "[AI VOICE Rsample] RAM usage: %d bytes", g_ext_buf_used_size);
}

#if _RESAMPLE_ENABLE
static struct _RESAMPLE_T *g_resample = NULL;
static uint8_t  *g_resample_cache_buf = NULL;
static uint32_t g_resample_cache_buf_len = 0;
typedef int32_t (*_RESAMPLE_CALLBACK)(uint8_t *buf, uint32_t len);

struct _RESAMPLE_T {
    void *id;
    _RESAMPLE_CALLBACK cb;
    uint8_t *iter_buf;
    uint32_t iter_len;
    uint32_t offset;
};

static void memzero_int16(void *dst, uint32_t len)
{
    int16_t *dst16 = (int16_t *)dst;
    int16_t *dst16_end = dst16 + len / 2;

    while (dst16 < dst16_end) {
        *dst16++ = 0;
    }
}

static void _resample_reset(struct _RESAMPLE_T *resamp)
{
    audio_resample_ex_flush((RESAMPLE_ID *)resamp->id);
    resamp->offset = resamp->iter_len;
}

static struct _RESAMPLE_T POSSIBLY_UNUSED *_resample_open(enum AUD_CHANNEL_NUM_T chans, _RESAMPLE_CALLBACK cb, uint32_t iter_len)
{
    struct _RESAMPLE_T *resamp;
    struct RESAMPLE_CFG_T cfg;
    enum RESAMPLE_STATUS_T status;
    uint32_t size, resamp_size;
    uint8_t *buf;
    const struct RESAMPLE_COEF_T *coef = &resample_coef_48k_to_32k;

    resamp_size = audio_resample_ex_get_buffer_size(chans, AUD_BITS_16, coef->phase_coef_num);

    size = sizeof(struct _RESAMPLE_T);
    size += ALIGN(iter_len, 4);
    size += resamp_size;

    buf = (uint8_t *)_get_ext_buf(size, AI_CAPTURE_MEMPOOL_USER_ANC_ASSIST_RESAMPLE);

    resamp = (struct _RESAMPLE_T *)buf;
    buf += sizeof(*resamp);
    resamp->cb = cb;
    resamp->iter_buf = buf;
    buf += ALIGN(iter_len, 4);
    resamp->iter_len = iter_len;
    resamp->offset = iter_len;

    memset(&cfg, 0, sizeof(cfg));
    cfg.chans = chans;
    cfg.bits = AUD_BITS_16;
    cfg.ratio_step = 0;
    cfg.coef = coef;
    cfg.buf = buf;
    cfg.size = resamp_size;

    status = audio_resample_ex_open(&cfg, (RESAMPLE_ID *)&resamp->id);
    ASSERT(status == RESAMPLE_STATUS_OK, "%s: Failed to open resample: %d", __func__, status);

    return resamp;
}

static int32_t POSSIBLY_UNUSED _resample_close(struct _RESAMPLE_T *resamp)
{
    if (resamp) {
        audio_resample_ex_close((RESAMPLE_ID *)resamp->id);
    }

    return 0;
}

static int32_t POSSIBLY_UNUSED _resample_run(struct _RESAMPLE_T *resamp, uint8_t *buf, uint32_t len)
{
    uint32_t in_size, out_size;
    struct RESAMPLE_IO_BUF_T io;
    enum RESAMPLE_STATUS_T status;
    int32_t ret;

    if (resamp == NULL) {
        goto _err_exit;
    }

    io.out_cyclic_start = NULL;
    io.out_cyclic_end = NULL;

    if (resamp->offset < resamp->iter_len) {
        io.in = buf;
        io.in_size = len;
        io.out = resamp->iter_buf + resamp->offset;
        io.out_size = resamp->iter_len - resamp->offset;

        status = audio_resample_ex_run((RESAMPLE_ID *)resamp->id, &io, &in_size, &out_size);
        if (status != RESAMPLE_STATUS_OUT_FULL && status != RESAMPLE_STATUS_IN_EMPTY &&
            status != RESAMPLE_STATUS_DONE) {
            goto _err_exit;
        }

        buf += in_size;
        len -= in_size;
        resamp->offset += out_size;

        ASSERT(len == 0 || resamp->offset == resamp->iter_len,
            "%s: Bad resample offset: len=%d offset=%d iter_len=%d",
            __func__, len, resamp->offset, resamp->iter_len);

        if (resamp->offset == resamp->iter_len) {
            ret = resamp->cb(resamp->iter_buf, resamp->iter_len);
            if (ret) {
                goto _err_exit;
            }
        }
    }

    while (len) {
        io.in = buf;
        io.in_size = len;
        io.out = resamp->iter_buf;
        io.out_size = resamp->iter_len;

        status = audio_resample_ex_run((RESAMPLE_ID *)resamp->id, &io, &in_size, &out_size);
        if (status != RESAMPLE_STATUS_OUT_FULL && status != RESAMPLE_STATUS_IN_EMPTY &&
            status != RESAMPLE_STATUS_DONE) {
            goto _err_exit;
        }

        ASSERT(in_size <= len, "%s: Bad resample in_size: in_size=%d len=%d", __func__, in_size, len);
        ASSERT(out_size <= resamp->iter_len, "%s: Bad resample out_size: out_size=%d iter_len=%d", __func__, out_size, resamp->iter_len);

        buf += in_size;
        len -= in_size;
        if (out_size == resamp->iter_len) {
            ret = resamp->cb(resamp->iter_buf, resamp->iter_len);
            if (ret) {
                goto _err_exit;
            }
        } else {
            resamp->offset = out_size;

            ASSERT(len == 0, "%s: Bad resample len: len=%d in_size=%d", __func__, len, in_size);
        }
    }

    return 0;

_err_exit:
    if (resamp) {
        _resample_reset(resamp);
    }

    memzero_int16(buf, len);

    return 1;
}

// --------------------------------------------
static void _resample_store_data(uint8_t *buf, uint32_t len)
{
    ASSERT(g_resample_cache_buf_len == len, "[%s] len is invalid: %d, %d", __func__, g_resample_cache_buf_len, len);

    memcpy(g_resample_cache_buf, buf, len);
}

static void _resample_get_data(uint8_t *buf, uint32_t len)
{
    ASSERT(g_resample_cache_buf_len == len, "[%s] len is invalid: %d, %d", __func__, g_resample_cache_buf_len, len);

    memcpy(buf, g_resample_cache_buf, len);
}

static int _resample_cb(uint8_t *buf, uint32_t len)
{
    // TRACE(2,"[%s] len = %d", __func__, len);

    _resample_store_data(buf, len);

    return 0;
}
#endif

//--------------------------------------------

static uint32_t ai_voice_record_callback(uint8_t *buf, uint32_t len)
{
#if defined(ANC_ASSIST_ENABLED)
    int32_t *pcm_buf_s32 = (int32_t *)buf;
    uint32_t temp_len = 0;

    temp_len = (g_stream_cfg.sample_rate / AUD_SAMPRATE_16000) * FRAME_LEN_16K_15MS; 
    ASSERT(len == (temp_len * sizeof(int32_t) * CHANNEL_NUM), "[%s] len(%d) is invalid", __func__, len);

    // Convert one channel and 16 bits
    for (uint32_t i=0; i<temp_len; i++) {
        // 0: CH0 --> MIC1
        // 4: Add 24dB(8-->4)
        g_ai_buf[i] = speech_ssat_int16(pcm_buf_s32[CHANNEL_NUM * i + 0] >> 4);
    }

    if(app_anc_assist_is_runing()) {
        uint8_t *buf_ptr = buf;
        uint32_t loop_cnt = 2;
        for (uint32_t cnt=0; cnt<loop_cnt; cnt++) {
            app_anc_assist_process(buf_ptr, len/loop_cnt);
            buf_ptr += len/loop_cnt;
        }
    }
    buf = (uint8_t *)g_ai_buf;
    len = len / 2 / CHANNEL_NUM;    // 2: 32bits --> 16bits
#endif

// Resample from 48k to 32k
#if _RESAMPLE_ENABLE
    if (_resample_open_flag) {
        _resample_run(g_resample, buf, len);
        len = len * DST_SAMPLE_RATE / SRC_SAMPLE_RATE; 
        _resample_get_data(buf, len);

#ifdef RESAMPLE_AI_VOICE_AUDIO_DUMP
        audio_dump_clear_up();
        audio_dump_add_channel_data(0, (short *)buf, len / sizeof(short));
        audio_dump_run();
#endif

    }
#endif

    ASSERT(len == g_extern_data_size / 2, "[%s] len(%d) != g_extern_data_size / 2(%d)", __func__, len, g_extern_data_size / 2);
    g_extern_handler(buf, len);

    return 0;
}

int32_t assist_ai_voice_record_open(struct AF_STREAM_CONFIG_T *stream_cfg, mempool_get_buff_t mempool_get_buff)
{
    g_stream_cfg = *stream_cfg;
    g_mempool_get_buff = mempool_get_buff;

    _ext_buf_usage_start();

    ASSERT(g_stream_cfg.bits == AUD_BITS_16, "[%s] bits(%d) is invalid", __func__, g_stream_cfg.bits);
    ASSERT((g_stream_cfg.sample_rate == AUD_SAMPRATE_48000 || g_stream_cfg.sample_rate == AUD_SAMPRATE_32000 || g_stream_cfg.sample_rate == AUD_SAMPRATE_16000),
          "[%s] sample rate must be one of 48k,32k,16k, sample rate(%d) is invalid", __func__, g_stream_cfg.sample_rate);
    ASSERT(g_stream_cfg.channel_num == AUD_CHANNEL_NUM_1, "[%s] channel number(%d) is invalid", __func__, g_stream_cfg.channel_num);
    ASSERT(g_stream_cfg.data_size == FRAME_LEN_16K_15MS * sizeof(int16_t) * 2 * (g_stream_cfg.sample_rate / AUD_SAMPRATE_16000), "[%s] data_size(%d) is invalid", __func__, g_stream_cfg.data_size);

    g_extern_data_size  = g_stream_cfg.data_size;
    g_extern_data_ptr   = g_stream_cfg.data_ptr;
    g_extern_handler    = g_stream_cfg.handler;

#if defined(ANC_ASSIST_ENABLED)
    app_anc_assist_set_playback_info(AUD_SAMPRATE_NULL);
    app_anc_assist_set_mode(ANC_ASSIST_MODE_RECORD);
    g_stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)app_anc_assist_get_mic_ch_map(g_stream_cfg.io_path);
    g_stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)app_anc_assist_get_mic_ch_num(g_stream_cfg.io_path);
    g_stream_cfg.bits        = AUD_BITS_24;
#endif

    g_stream_cfg.data_size   = g_stream_cfg.data_size * g_stream_cfg.channel_num ;

#if _RESAMPLE_ENABLE
    if (g_stream_cfg.sample_rate == DST_SAMPLE_RATE) {
        _resample_open_flag = 1;
        g_stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)SRC_SAMPLE_RATE;
        g_stream_cfg.data_size *= SRC_SAMPLE_RATE / DST_SAMPLE_RATE;
    } else {
        _resample_open_flag = 0;
    }
#endif
   
#if defined(ANC_ASSIST_ENABLED)
    g_stream_cfg.data_size   *= 2;  // 24 bits
    g_stream_cfg.data_ptr  = (uint8_t *)_get_ext_buf(g_stream_cfg.data_size, AI_CAPTURE_MEMPOOL_USER_ANC_ASSIST);
#endif
    g_stream_cfg.handler   = ai_voice_record_callback;

#if defined(ANC_ASSIST_ENABLED)
    if (FRAME_LEN_16K_15MS * (g_stream_cfg.sample_rate / AUD_SAMPRATE_16000) * sizeof(int16_t) <= g_extern_data_size) {
        g_ai_buf = (int16_t *)g_extern_data_ptr;
    } else {
        g_ai_buf = (int16_t *)_get_ext_buf(FRAME_LEN_16K_15MS * (g_stream_cfg.sample_rate / AUD_SAMPRATE_16000) * 
        sizeof(int16_t), AI_CAPTURE_MEMPOOL_USER_ANC_ASSIST);
    }
#endif

#if _RESAMPLE_ENABLE
    // 2: ping and 32bits->16bits
    if (_resample_open_flag) {
        uint32_t iter_frame_len = FRAME_LEN_32K_15MS * sizeof(int16_t);
        g_resample_cache_buf_len = iter_frame_len;
        g_resample_cache_buf = (uint8_t *)_get_ext_buf(g_resample_cache_buf_len, AI_CAPTURE_MEMPOOL_USER_ANC_ASSIST_RESAMPLE);
        g_resample = _resample_open(AUD_CHANNEL_NUM_1, _resample_cb, iter_frame_len);
#ifdef RESAMPLE_AI_VOICE_AUDIO_DUMP
       audio_dump_init(FRAME_LEN_16K_15MS * (g_stream_cfg.sample_rate / AUD_SAMPRATE_16000), sizeof(short), 1);
#endif
    }
#endif

    _ext_buf_usage_end();

    TRACE(0, "[%s]: sample rate: %d, bits: %d, channel number: %d, data size:%d",
        __func__,
        g_stream_cfg.sample_rate,
        g_stream_cfg.bits,
        g_stream_cfg.channel_num,
        g_stream_cfg.data_size);

    af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &g_stream_cfg);

    return 0;
}

int32_t assist_ai_voice_record_close(void)
{
    af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);

#if _RESAMPLE_ENABLE
    if (_resample_open_flag) {
        _resample_close(g_resample); 
    }
#endif

#if defined(ANC_ASSIST_ENABLED)
	app_anc_assist_set_mode(ANC_ASSIST_MODE_STANDALONE);
#endif

    return 0;
}