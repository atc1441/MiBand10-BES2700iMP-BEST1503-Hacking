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
#include "app_utils.h"
#include "audioflinger.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_codec.h"
#include "string.h"
#include "audio_dump.h"
// #include "local_wav.h"

// #define AUDIO_STREAM_TEST_AUDIO_DUMP
// #define DEBUG_DUMP_CNT
// #define DEBUG_ADDA_LOOP

#define CHAR_BYTES          (1)
#define SHORT_BYTES         (2)
#define INT_BYTES           (4)

#define SAMPLE_BITS         (16)
#define SAMPLE_BYTES        (2 * ((SAMPLE_BITS + 8) / 16))

#define FRAME_LEN           (256)

#define TX_SAMPLE_RATE      (16000)
#define RX_SAMPLE_RATE      (16000)

#define TX_CHANNEL_NUM_MAX  (6)
#define RX_CHANNEL_NUM_MAX  (2)

#define TX_FRAME_BUF_SIZE   (FRAME_LEN * SAMPLE_BYTES * 2)
#define RX_FRAME_BUF_SIZE   (FRAME_LEN * SAMPLE_BYTES * 2)

#if SAMPLE_BYTES == SHORT_BYTES
typedef short   VOICE_PCM_T;
#elif SAMPLE_BYTES == INT_BYTES
typedef int     VOICE_PCM_T;
#else
#error "Invalid SAMPLE_BYTES!!!"
#endif

#define CODEC_STREAM_ID     AUD_STREAM_ID_0

static uint8_t POSSIBLY_UNUSED codec_capture_buf[TX_FRAME_BUF_SIZE * TX_CHANNEL_NUM_MAX];
static uint8_t POSSIBLY_UNUSED codec_playback_buf[RX_FRAME_BUF_SIZE * RX_CHANNEL_NUM_MAX];

static uint32_t POSSIBLY_UNUSED codec_capture_cnt = 0;
static uint32_t POSSIBLY_UNUSED codec_playback_cnt = 0;

static uint32_t tx_channel_num = 0;
static uint32_t rx_channel_num = 0;

#if defined(DEBUG_ADDA_LOOP)
static VOICE_PCM_T adda_loop_buf[FRAME_LEN];
#endif

#if defined(DEBUG_DUMP_CNT)
static int16_t dump_pcm_cnt = 0;
#endif

static uint32_t codec_capture_callback(uint8_t *buf, uint32_t len)
{
    int POSSIBLY_UNUSED frame_len = len / sizeof(VOICE_PCM_T) / tx_channel_num;
    VOICE_PCM_T POSSIBLY_UNUSED *pcm_buf[TX_CHANNEL_NUM_MAX];
    int interval_len = len * 2 / tx_channel_num;

    for (int i=0; i<tx_channel_num; i++) {
        pcm_buf[i] = (VOICE_PCM_T *)(buf + i * interval_len);
#if defined(AUDIO_STREAM_TEST_AUDIO_DUMP)
        audio_dump_add_channel_data(i, pcm_buf[i], frame_len);
#endif
    }

    // TRACE(2,"[%s] cnt = %d", __func__, codec_capture_cnt++);

#if defined(DEBUG_ADDA_LOOP)
    memcpy(adda_loop_buf, pcm_buf[0], frame_len * sizeof(VOICE_PCM_T));
#endif

#if defined(DEBUG_DUMP_CNT)
    for (int i=0; i<frame_len; i++) {
        pcm_buf[0][i] = dump_pcm_cnt++;
    }
#endif

#if defined(AUDIO_STREAM_TEST_AUDIO_DUMP)
    audio_dump_run();
#endif

    return len;
}

static uint32_t codec_playback_callback(uint8_t *buf, uint32_t len)
{
    int POSSIBLY_UNUSED frame_len = len / sizeof(VOICE_PCM_T) / rx_channel_num;
    VOICE_PCM_T POSSIBLY_UNUSED *pcm_buf[RX_CHANNEL_NUM_MAX];
    int interval_len = len * 2 / rx_channel_num;

    for (int i=0; i<rx_channel_num; i++) {
        pcm_buf[i] = (VOICE_PCM_T *)(buf + i * interval_len);
    }

#if defined(DEBUG_ADDA_LOOP)
    memcpy(pcm_buf[0], adda_loop_buf, frame_len * sizeof(VOICE_PCM_T));
#endif

    // TRACE(2,"[%s] cnt = %d", __func__, codec_playback_cnt++);

    return len;
}

static int audio_test_stream_start(bool on)
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

        // Initialize Cqueue
        codec_capture_cnt = 0;
        codec_playback_cnt = 0;

#if defined(DEBUG_DUMP_CNT)
        dump_pcm_cnt = 0;
#endif

        tx_channel_num = hal_codec_get_input_path_chan_num(AUD_INPUT_PATH_MAINMIC);
        ASSERT(tx_channel_num <= TX_CHANNEL_NUM_MAX, "[%s] tx_channel_num(%d) > TX_CHANNEL_NUM_MAX", __func__, tx_channel_num);

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_num = tx_channel_num;
        stream_cfg.data_size = TX_FRAME_BUF_SIZE * tx_channel_num;
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
        ASSERT(ret == 0, "codec capture failed: %d", ret);

        rx_channel_num = RX_CHANNEL_NUM_MAX;

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_num = rx_channel_num;
        stream_cfg.data_size = RX_FRAME_BUF_SIZE * rx_channel_num;
        stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)RX_SAMPLE_RATE;
        stream_cfg.bits = (enum AUD_BITS_T)SAMPLE_BITS;
        stream_cfg.vol = 12;
        stream_cfg.chan_sep_buf = true;
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.handler = codec_playback_callback;
        stream_cfg.data_ptr = codec_playback_buf;

        TRACE(3, "[%s] Playback: sample rate: %d, data_size: %d, channel num: %d", __func__, stream_cfg.sample_rate, stream_cfg.data_size, stream_cfg.channel_num);
        af_stream_open(CODEC_STREAM_ID, AUD_STREAM_PLAYBACK, &stream_cfg);
        ASSERT(ret == 0, "codec playback failed: %d", ret);

#if defined(AUDIO_STREAM_TEST_AUDIO_DUMP)        
        audio_dump_init(FRAME_LEN, sizeof(VOICE_PCM_T), tx_channel_num);
#endif

        // Start
        af_stream_start(CODEC_STREAM_ID, AUD_STREAM_CAPTURE);
        af_stream_start(CODEC_STREAM_ID, AUD_STREAM_PLAYBACK);
    } else {
        // Close stream
        af_stream_stop(CODEC_STREAM_ID, AUD_STREAM_PLAYBACK);  
        af_stream_stop(CODEC_STREAM_ID, AUD_STREAM_CAPTURE);

        audio_dump_deinit();

        af_stream_close(CODEC_STREAM_ID, AUD_STREAM_PLAYBACK);
        af_stream_close(CODEC_STREAM_ID, AUD_STREAM_CAPTURE);

        app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_32K);

        af_set_priority(AF_USER_TEST, osPriorityAboveNormal);

        TRACE(1, "[%s] OFF", __func__);
    }

    isRun=on;
    return 0;   
}

static bool audio_test_stream_status = true;
int32_t audio_test_stream(void)
{
    TRACE(0, "[%s] status = %d", __func__, audio_test_stream_status);

    audio_test_stream_start(audio_test_stream_status);
    audio_test_stream_status = !audio_test_stream_status;

    return 0;
}