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
#include "cmsis_os.h"
#include "app_utils.h"
#include "audioflinger.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_codec.h"
#include "string.h"
#include "audio_dump.h"

#if defined(APP_MCPP_CLI)
#include "app_mcpp.h"
#endif

// #define AUDIO_TEST_APP_MCPP_DUMP
// #define DEBUG_DUMP_CNT

#define CHAR_BYTES          (1)
#define SHORT_BYTES         (2)
#define INT_BYTES           (4)

#define SAMPLE_BITS         (24)
#define SAMPLE_BYTES        (2 * ((SAMPLE_BITS + 8) / 16))

#define FRAME_LEN           (120)

#define TX_SAMPLE_RATE      (16000)
#define RX_SAMPLE_RATE      (16000)

// Analog MIC
#define MIC_CHANNEL_MAP     (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1 | AUD_CHANNEL_MAP_CH4)
// Digital MIC
// #define MIC_CHANNEL_MAP     (AUD_CHANNEL_MAP_DIGMIC_CH1 | AUD_CHANNEL_MAP_DIGMIC_CH2 | AUD_CHANNEL_MAP_DIGMIC_CH3)

#define REF_CHANNEL_MAP     (AUD_CHANNEL_MAP_ECMIC_CH0)

#define TX_CHANNEL_NUM_MAX  (6)
#define RX_CHANNEL_NUM_MAX  (2)
#define REF_CHANNEL_NUM_MAX (2)

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

static uint32_t mic_channel_num = 0;
static uint32_t ref_channel_num = 0;
static uint32_t tx_channel_num = 0;
static uint32_t rx_channel_num = 0;

static VOICE_PCM_T ref_buf[RX_CHANNEL_NUM_MAX][FRAME_LEN];

#if defined(DEBUG_DUMP_CNT)
static int16_t dump_pcm_cnt = 0;
#endif

typedef enum {
    AUDIO_TEST_APP_MCPP_SER_NONE,

    AUDIO_TEST_APP_MCPP_SER_M55,
    AUDIO_TEST_APP_MCPP_SER_HIFI4,
    AUDIO_TEST_APP_MCPP_SER_SENSER,
    AUDIO_TEST_APP_MCPP_SER_M55_HIFI4,
    AUDIO_TEST_APP_MCPP_SER_CP_SUBSYS,
} AUDIO_TEST_APP_MCPP_SER_QTY;

static AUDIO_TEST_APP_MCPP_SER_QTY g_mcpp_server_list = AUDIO_TEST_APP_MCPP_SER_NONE;

static void gaf_stream_split_capture_data(VOICE_PCM_T *pcm_buf, VOICE_PCM_T *echo_buf, uint32_t pcm_len)
{
    VOICE_PCM_T *pcm_buf_j = pcm_buf;
    VOICE_PCM_T *pcm_buf_i = pcm_buf;
    uint32_t count = pcm_len / tx_channel_num;
    while (count-- > 0) {
        for (int i=0; i<mic_channel_num; ++i){
            *pcm_buf_j++ = *pcm_buf_i++;
        }
        *echo_buf++ = *pcm_buf_i++;
    }
}

static uint32_t codec_capture_callback(uint8_t *buf, uint32_t len)
{
    uint32_t    POSSIBLY_UNUSED pcm_len = len / sizeof(VOICE_PCM_T);
    uint32_t    POSSIBLY_UNUSED frame_len = pcm_len / tx_channel_num;
    VOICE_PCM_T POSSIBLY_UNUSED *pcm_buf = (VOICE_PCM_T *)buf;

#if defined(AUDIO_TEST_APP_MCPP_DUMP)
    uint32_t dump_ch = 0;
    audio_dump_clear_up();

    for (uint32_t ch=0; ch<tx_channel_num; ch++) {
#if SAMPLE_BITS == 16
        audio_dump_add_channel_data_from_multi_channels(dump_ch++, pcm_buf, frame_len, tx_channel_num, ch);
#else
        audio_dump_add_channel_data_from_multi_channels_32bit_to_16bit(dump_ch++, pcm_buf, frame_len, tx_channel_num, ch, 8);
#endif
    }
#endif

    gaf_stream_split_capture_data(pcm_buf, ref_buf[0], pcm_len);
    // TRACE(2,"[%s] cnt = %d", __func__, codec_capture_cnt++);

#if defined(APP_MCPP_CLI)
    APP_MCPP_CAP_PCM_T pcm_cfg;
    memset(&pcm_cfg, 0, sizeof(pcm_cfg));
    pcm_cfg.in = pcm_buf;
    pcm_cfg.ref = ref_buf;
    pcm_cfg.vpu = NULL;
    pcm_cfg.out = pcm_buf;
    pcm_cfg.frame_len = frame_len;
    app_mcpp_capture_process(APP_MCPP_USER_TEST_01, &pcm_cfg);

    if (g_mcpp_server_list == AUDIO_TEST_APP_MCPP_SER_M55_HIFI4) {
        APP_MCPP_CAP_PCM_T pcm_cfg_02;
        memset(&pcm_cfg_02, 0, sizeof(pcm_cfg_02));
        pcm_cfg_02.in = pcm_buf;
        pcm_cfg_02.ref = NULL;
        pcm_cfg_02.vpu = NULL;
        pcm_cfg_02.out = pcm_buf;
        pcm_cfg_02.frame_len = frame_len;
        app_mcpp_capture_process(APP_MCPP_USER_TEST_02, &pcm_cfg_02);
    }
#endif

#if defined(DEBUG_DUMP_CNT)
    for (int i=0; i<frame_len; i++) {
        pcm_buf[i] = dump_pcm_cnt++;
    }
#endif

#if defined(AUDIO_TEST_APP_MCPP_DUMP)
#if SAMPLE_BITS == 16
    audio_dump_add_channel_data_from_multi_channels(dump_ch++, pcm_buf, frame_len, 1, 0);
#else
    audio_dump_add_channel_data_from_multi_channels_32bit_to_16bit(dump_ch++, pcm_buf, frame_len, 1, 0, 8);
#endif
    audio_dump_run();
#endif

    return len;
}

static uint32_t codec_playback_callback(uint8_t *buf, uint32_t len)
{
    uint32_t    POSSIBLY_UNUSED pcm_len = len / sizeof(VOICE_PCM_T);
    uint32_t    POSSIBLY_UNUSED frame_len = pcm_len / rx_channel_num;
    VOICE_PCM_T POSSIBLY_UNUSED *pcm_buf = (VOICE_PCM_T *)buf;

    // TRACE(2,"[%s] cnt = %d", __func__, codec_playback_cnt++);

    return len;
}

static int audio_test_app_mcpp_start(bool on)
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

        mic_channel_num = hal_codec_get_input_map_chan_num(MIC_CHANNEL_MAP);
        ref_channel_num = hal_codec_get_input_map_chan_num(REF_CHANNEL_MAP);
        tx_channel_num = hal_codec_get_input_map_chan_num(MIC_CHANNEL_MAP | REF_CHANNEL_MAP);
        ASSERT(tx_channel_num <= TX_CHANNEL_NUM_MAX, "[%s] tx_channel_num(%d) > TX_CHANNEL_NUM_MAX", __func__, tx_channel_num);

#if defined(APP_MCPP_CLI)
        APP_MCPP_CFG_T dsp_cfg;
        memset(&dsp_cfg, 0, sizeof(APP_MCPP_CFG_T));
        dsp_cfg.capture.stream_enable = true;
        dsp_cfg.capture.sample_rate = TX_SAMPLE_RATE;
        dsp_cfg.capture.sample_bytes = SAMPLE_BYTES;
        dsp_cfg.capture.algo_frame_len = FRAME_LEN;
        dsp_cfg.capture.params[0] = 1;
        dsp_cfg.capture.params[1] = app_mcpp_get_cap_channel_map(true, false, false);
        dsp_cfg.capture.channel_num = mic_channel_num;
        if (g_mcpp_server_list == AUDIO_TEST_APP_MCPP_SER_M55) {
            dsp_cfg.capture.core_server = APP_MCPP_CORE_M55;
        } else if (g_mcpp_server_list == AUDIO_TEST_APP_MCPP_SER_HIFI4) {
            dsp_cfg.capture.core_server = APP_MCPP_CORE_HIFI;
        } else if (g_mcpp_server_list == AUDIO_TEST_APP_MCPP_SER_M55_HIFI4) {
            dsp_cfg.capture.core_server = APP_MCPP_CORE_M55;
        } else if (g_mcpp_server_list == AUDIO_TEST_APP_MCPP_SER_SENSER) {
            dsp_cfg.capture.core_server = APP_MCPP_CORE_SENS;
        } else if (g_mcpp_server_list == AUDIO_TEST_APP_MCPP_SER_CP_SUBSYS) {
            dsp_cfg.capture.core_server = APP_MCPP_CORE_CP_SUBSYS;
        }

        dsp_cfg.playback.stream_enable = false;

        app_mcpp_open(APP_MCPP_USER_TEST_01, &dsp_cfg);

        if (g_mcpp_server_list == AUDIO_TEST_APP_MCPP_SER_M55_HIFI4) {
            dsp_cfg.capture.params[0] = 1;
            dsp_cfg.capture.params[1] = app_mcpp_get_cap_channel_map(false, false, false);
            dsp_cfg.capture.channel_num = 1;
            dsp_cfg.capture.core_server = APP_MCPP_CORE_HIFI;

            app_mcpp_open(APP_MCPP_USER_TEST_02, &dsp_cfg);
        }
#endif

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_map = MIC_CHANNEL_MAP | REF_CHANNEL_MAP;
        stream_cfg.channel_num = tx_channel_num;
        stream_cfg.data_size = TX_FRAME_BUF_SIZE * tx_channel_num;
        stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)TX_SAMPLE_RATE;
        stream_cfg.bits = (enum AUD_BITS_T)SAMPLE_BITS;
        stream_cfg.vol = 12;
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
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.handler = codec_playback_callback;
        stream_cfg.data_ptr = codec_playback_buf;

        TRACE(3, "[%s] Playback: sample rate: %d, data_size: %d, channel num: %d", __func__, stream_cfg.sample_rate, stream_cfg.data_size, stream_cfg.channel_num);
        af_stream_open(CODEC_STREAM_ID, AUD_STREAM_PLAYBACK, &stream_cfg);
        ASSERT(ret == 0, "codec playback failed: %d", ret);

#if defined(AUDIO_TEST_APP_MCPP_DUMP)
        audio_dump_init(FRAME_LEN, sizeof(short), tx_channel_num + 1);
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

#if defined(APP_MCPP_CLI)
        if (g_mcpp_server_list == AUDIO_TEST_APP_MCPP_SER_M55_HIFI4) {
            app_mcpp_close(APP_MCPP_USER_TEST_02);
        }
        app_mcpp_close(APP_MCPP_USER_TEST_01);
#endif

        app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_32K);

        af_set_priority(AF_USER_TEST, osPriorityAboveNormal);

        TRACE(1, "[%s] OFF", __func__);
    }

    isRun=on;
    return 0;
}

static bool audio_test_app_mcpp_status = true;
int32_t audio_test_app_mcpp(const char *cmd)
{
    if(!strncmp(cmd, "on", strlen("on"))) {
        audio_test_app_mcpp_status = true;
        if (!strncmp(cmd + strlen("on "), "M55_HIFI4", strlen("M55_HIFI4"))) {
            g_mcpp_server_list = AUDIO_TEST_APP_MCPP_SER_M55_HIFI4;
        } else if (!strncmp(cmd + strlen("on "), "CP_SUBSYS", strlen("CP_SUBSYS"))) {
            g_mcpp_server_list = AUDIO_TEST_APP_MCPP_SER_CP_SUBSYS;
        } else if (!strncmp(cmd + strlen("on "), "M55", strlen("M55"))) {
            g_mcpp_server_list = AUDIO_TEST_APP_MCPP_SER_M55;
        } else if (!strncmp(cmd + strlen("on "), "SENSER", strlen("SENSER"))) {
            g_mcpp_server_list = AUDIO_TEST_APP_MCPP_SER_SENSER;
        } else if (!strncmp(cmd + strlen("on "), "HIFI4", strlen("HIFI4"))) {
            g_mcpp_server_list = AUDIO_TEST_APP_MCPP_SER_HIFI4;
        } else {
            ASSERT(0, "[%s] cmd is error: %s", __func__, (cmd + strlen("on ")));
        }
    } else if(!strncmp(cmd, "off", strlen("off"))) {
        audio_test_app_mcpp_status = false;
    }

    TRACE(0, "[%s] status = %d", __func__, audio_test_app_mcpp_status);
    audio_test_app_mcpp_start(audio_test_app_mcpp_status);

    return 0;
}