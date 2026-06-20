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
#include "hal_trace.h"
#include "plat_types.h"
#include "audioflinger.h"
#include "speech_ssat.h"
#include "app_anc_assist.h"

static uint8_t *g_stream_buf_ptr    = NULL;
static AF_STREAM_HANDLER_T g_stream_handler = NULL;

static int32_t _assist_ai_voice_kws_callback(void *buf, uint32_t len, void *other);

int32_t assist_ai_voice_kws_init(void)
{
    app_anc_assist_register(ANC_ASSIST_USER_KWS, _assist_ai_voice_kws_callback);

    return 0;
}

int32_t assist_ai_voice_kws_open(struct AF_STREAM_CONFIG_T *stream_cfg)
{
    ASSERT(stream_cfg != NULL, "[%s] stream_cfg = NULL", __func__);
    ASSERT(stream_cfg->bits == AUD_BITS_16, "[%s] bits(%d) is invalid", __func__, stream_cfg->bits);
    ASSERT(stream_cfg->channel_num == AUD_CHANNEL_NUM_1, "[%s] ch(%d) is invalid", __func__, stream_cfg->channel_num);

    TRACE(0, "[%s] KWS open stream: fs: %d, ch: %d, bits: %d, size: %d", __func__,
        stream_cfg->sample_rate,
        stream_cfg->channel_num,
        stream_cfg->bits,
        stream_cfg->data_size);

    g_stream_buf_ptr = stream_cfg->data_ptr;
    g_stream_handler = stream_cfg->handler;

    app_anc_assist_open(ANC_ASSIST_USER_KWS);

    return 0;
}

int32_t assist_ai_voice_kws_close(void)
{
    TRACE(0, "[%s] KWS close stream", __func__);

    app_anc_assist_close(ANC_ASSIST_USER_KWS);
    g_stream_buf_ptr = NULL;
    g_stream_handler = NULL;

    return 0;
}

// int16_t _get_max_pcm_val(int16_t *pcm_buf, uint32_t pcm_len)
// {
//     int16_t max = 0;
//     for (uint32_t i=0; i<pcm_len; i++) {
//         if (max < pcm_buf[i]) {
//             max = pcm_buf[i];
//         }
//     }

//     return max;
// }

static int32_t _assist_ai_voice_kws_callback(void *buf, uint32_t len, void *other)
{
    // TRACE(0, "[%s] len = %d", __func__, len);

    if ((g_stream_handler == NULL) || (g_stream_buf_ptr == NULL)) {
        return 1;
    }

    float   *input_pcm_buf = (float *)buf;
    int16_t *out_pcm_buf = (int16_t *)g_stream_buf_ptr;

    for (uint32_t i=0; i<len; i++) {
        out_pcm_buf[i] = speech_ssat_int16((int32_t)(input_pcm_buf[i] / (256 / 1)));   // 1: 0dB
    }
    g_stream_handler((uint8_t *)out_pcm_buf, len * sizeof(int16_t));

    // TRACE(0, "[%s] Max: %d", __func__, (uint32_t)_get_max_pcm_val(out_pcm_buf, len));

    return 0;
}
