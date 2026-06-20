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
#include "voice_tx_aec.h"
#include "hal_trace.h"
#include "audio_dump.h"
#include "speech_cfg.h"
#include "ssp_aec.h"
#include "voice_dev_defs.h"

/**
 * FRAME SIZE: 384
 *  RAM: 52k
 *  MIPS: < 11M
 * 
 * FRAME SIZE: 256
 *  RAM: xxx
 *  MIPS: < 15
 */

// #define VOICE_TX_AEC_AUDIO_DUMP

static uint8_t *aec_out_buf = NULL;
static ssp_aec_st_t *voice_tx_aec_st = NULL;

static uint32_t voice_tx_aec_bypass = 1;
static uint32_t g_frame_len = FRAME_LEN;

#define VOICE_TX_AEC_BUF_SIZE   (1024 * 55)
#define _HEAP_SIZE              (VOICE_TX_AEC_BUF_SIZE)

extern void *voice_algo_get_ext_buf(int size);
extern void voice_algo_ext_buf_usage_start(void);
extern void voice_algo_ext_buf_usage_end(const char *name);

int32_t voice_tx_aec_open(uint32_t frame_len, uint32_t bits)
{
    TRACE(3, "[%s] frame_len = %d, bits = %d", __func__, frame_len, bits);

    ASSERT((frame_len == FRAME_LEN) || (frame_len == FRAME_LEN_MAX), "[%s] frame_len(%d) is invalid", __func__, frame_len);
    ASSERT(bits == SAMPLE_BITS, "[%s] bits(%d) != SAMPLE_BITS", __func__, bits);

    g_frame_len = frame_len;
    voice_tx_aec_bypass = 0;

    uint8_t *pBuff = (uint8_t *)voice_algo_get_ext_buf(_HEAP_SIZE);

    speech_heap_init(pBuff, _HEAP_SIZE);

    aec_out_buf = (uint8_t *)speech_calloc(g_frame_len, sizeof(short));

    #define FFT_LEN (512)
    voice_tx_aec_st = ssp_aec_create(SAMPLE_RATE, g_frame_len, FFT_LEN - g_frame_len, 0, 1, 1);

#ifdef VOICE_TX_AEC_AUDIO_DUMP
    audio_dump_init(g_frame_len, sizeof(short), 3);
#endif

    return 0;
}

int32_t voice_tx_aec_close(void)
{
    TRACE(1, "[%s] ...", __func__);

    ssp_aec_destroy(voice_tx_aec_st);

    speech_free(aec_out_buf);

    size_t total = 0, used = 0, max_used = 0;
    speech_memory_info(&total, &used, &max_used);
    TRACE(4, "[%s] MEM: total - %d, used - %d, max_used - %d.", __func__, total, used, max_used);

    voice_tx_aec_bypass = 1;

    return 0;
}

int32_t voice_tx_aec_ctl(voice_tx_aec_ctl_t ctl, void *ptr)
{
    TRACE(2, "[%s] ctl = %d", __func__, ctl);

    switch (ctl) {
        case VOICE_TX_AEC_SET_BYPASS_ENABLE:
            voice_tx_aec_bypass = 1;
            // Clean up buffer data and status
            break;

        case VOICE_TX_AEC_SET_BYPASS_DISABLE:
            voice_tx_aec_bypass = 0;
            break;

        default:
            TRACE(2, "[%s] WARNING: ctl(%d) is not valid", __func__, ctl);
            return -1;
    }

    return 0;
}

int32_t voice_tx_aec_process(int16_t *pcm_buf, int16_t *ref_buf, int32_t pcm_len, int16_t *out_buf)
{
    if (voice_tx_aec_bypass) {
        return 0;
    }

    // TRACE(2, "[%s] pcm_len = %d", __func__, pcm_len);

    ASSERT(pcm_len == g_frame_len, "[%s] pcm_len(%d) != g_frame_len(%d)", __func__, pcm_len, g_frame_len);

#ifdef VOICE_TX_AEC_AUDIO_DUMP
    audio_dump_clear_up();
    audio_dump_add_channel_data(0, ref_buf, pcm_len);
    audio_dump_add_channel_data(1, pcm_buf, pcm_len);
#endif

    ssp_aec_process(voice_tx_aec_st, pcm_buf, (int16_t *)ref_buf, (int16_t *)aec_out_buf);
    speech_copy_int16(out_buf, (int16_t *)aec_out_buf, pcm_len);

#ifdef VOICE_TX_AEC_AUDIO_DUMP
    audio_dump_add_channel_data(2, out_buf, pcm_len);
    audio_dump_run();
#endif

    return 0;
}

int32_t voice_tx_aec_dump(void)
{
    return 0;
}
