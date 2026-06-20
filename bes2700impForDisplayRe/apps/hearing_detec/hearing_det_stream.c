/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#include "string.h"
#include "audio_dump.h"
//#include "ae_math.h"
#include <math.h>
#include "speech_memory.h"
#include "noise_detection.h"
#include "hear_det_process.h"

//#define TX_HEAR_COMPEN_DUMP
//#define RX_HEAR_COMPEN_DUMP

#if defined(SPEECH_TX_AEC_CODEC_REF)
#define TX_CHANNEL_NUM      (2)
#else
#define TX_CHANNEL_NUM      (1)
#endif
#define RX_CHANNEL_NUM      (1)

#define CHAR_BYTES          (1)
#define SHORT_BYTES         (2)
#define BIT24_BYTES         (3)
#define INT_BYTES           (4)

#define TX_SAMPLE_BITS      (16)
#define TX_SAMPLE_BYTES     (TX_SAMPLE_BITS / 8)

#define RX_SAMPLE_BITS      (24)
#define RX_SAMPLE_BYTES     (32 / 8)

#define TX_SAMPLE_RATE      (16000)
#define RX_SAMPLE_RATE      (48000)

#define TX_FRAME_LEN        (256)
#define RX_FRAME_LEN        (256*3)

#define TX_BUF_SIZE         (TX_FRAME_LEN * TX_CHANNEL_NUM * TX_SAMPLE_BYTES * 2)
#define RX_BUF_SIZE         (RX_FRAME_LEN * RX_CHANNEL_NUM * RX_SAMPLE_BYTES * 2)

#if TX_SAMPLE_BYTES == SHORT_BYTES
typedef short   TX_VOICE_PCM_T;
#elif TX_SAMPLE_BYTES == INT_BYTES || TX_SAMPLE_BYTES == BIT24_BYTES
typedef int     TX_VOICE_PCM_T;
#else
#error "Invalid TX_SAMPLE_BYTES!!!"
#endif

#if RX_SAMPLE_BYTES == SHORT_BYTES
typedef short   RX_VOICE_PCM_T;
#elif RX_SAMPLE_BYTES == INT_BYTES || RX_SAMPLE_BYTES == BIT24_BYTES
typedef int     RX_VOICE_PCM_T;
#else
#error "Invalid RX_SAMPLE_BYTES!!!"
#endif

static uint8_t POSSIBLY_UNUSED codec_capture_buf[TX_BUF_SIZE];
static uint8_t POSSIBLY_UNUSED codec_playback_buf[RX_BUF_SIZE];

static uint32_t POSSIBLY_UNUSED codec_capture_cnt = 0;
static uint32_t POSSIBLY_UNUSED codec_playback_cnt = 0;

#define CODEC_STREAM_ID     AUD_STREAM_ID_0

#ifdef AUDIO_HEARING_COMPSATN

#define HEAR_DET_STREAM_BUF_SIZE    1024*17
#ifdef HEARING_USE_STATIC_RAM
char    HEAR_DET_STREAM_BUF[HEAR_DET_STREAM_BUF_SIZE];
#else
uint8_t *hear_det_buf = NULL;
#endif

NoiseDetectionState* noise_det_st = NULL;
const NoiseDetectionConfig noise_det_cfg = {
    .bypass = 0,
    .debug_en = 0,
    .snr_thd = 1000000,
    .noisepower_thd0 = 150,
    .noisepower_thd1 = 200,
    .pwr_start_bin  = 125,
    .pwr_end_bin    = 750,
    .ksi_start_bin  = 500,
    .ksi_end_bin    = 1000,
    .average_period = 8,
};
#endif
#if defined(TX_HEAR_COMPEN_DUMP)
short dump_buf[TX_FRAME_LEN] = {0};
#elif defined(RX_HEAR_COMPEN_DUMP)
short dump_buf[RX_FRAME_LEN] = {0};
#endif

float hear_spk_calib_cfg[6] = {-1.0F, -2.0F, -3.0F, -4.0F, -5.0F, -6.0F};
uint8_t hear_noise_state = 0;
#define DB2LIN(x) (powf(10.f, (x) / 20.f))
static int32_t __SSAT(int32_t val, uint32_t sat)
{
  if ((sat >= 1U) && (sat <= 32U))
  {
    const int32_t max = (int32_t)((1U << (sat - 1U)) - 1U);
    const int32_t min = -1 - max ;
    if (val > max)
    {
      return max;
    }
    else if (val < min)
    {
      return min;
    }
  }
  return val;
}
void tx_data_gain(TX_VOICE_PCM_T *pcm_buf, uint32_t pcm_len,float gain_db)
{
    float gain_val = DB2LIN(gain_db);
    int32_t pcm_out = 0;
    if(1==TX_CHANNEL_NUM)
    {
        for(int i=0;i<pcm_len;i++)
        {
            pcm_out = (int32_t)((float)pcm_buf[i] * gain_val);
            pcm_out = __SSAT(pcm_out,TX_SAMPLE_BITS);
            pcm_buf[i] = (int32_t)pcm_out;
        }
    }
}
static uint32_t codec_capture_callback(uint8_t *buf, uint32_t len)
{
    int POSSIBLY_UNUSED pcm_len = len / sizeof(TX_VOICE_PCM_T) / TX_CHANNEL_NUM;
    TX_VOICE_PCM_T POSSIBLY_UNUSED *pcm_buf = (TX_VOICE_PCM_T *)buf;

#if defined(TX_HEAR_COMPEN_DUMP)
    audio_dump_clear_up();
    if(2==TX_CHANNEL_NUM)
    {
        for(int i=0;i<pcm_len;i++)
            dump_buf[i] = pcm_buf[2 * i];
        audio_dump_add_channel_data(0, dump_buf, pcm_len);
        for(int i=0;i<pcm_len;i++)
            dump_buf[i] = pcm_buf[2 * i + 1];
        audio_dump_add_channel_data(1, dump_buf, pcm_len);
    }
    else if(1==TX_CHANNEL_NUM)
    {
        audio_dump_add_channel_data(0, pcm_buf, pcm_len);
    }

#endif

    tx_data_gain(pcm_buf, pcm_len, 0);

#if defined(TX_HEAR_COMPEN_DUMP)
    if(1==TX_CHANNEL_NUM)
    {
        audio_dump_add_channel_data(1, pcm_buf, pcm_len);
    }
    audio_dump_run();
#endif
#ifdef AUDIO_HEARING_COMPSATN
    static int trace_cnt = 0;
    static int cur_noise_staus = 1;
    int last_status = cur_noise_staus;
    if(NULL!=noise_det_st)
        cur_noise_staus = NoiseDetection_process(noise_det_st, pcm_buf, pcm_len, TX_CHANNEL_NUM);
    else
        TRACE(1,"[%s] WARNING!!! state is NULL",__func__);
    if(0==cur_noise_staus)
        hear_noise_state = 1;
    else
        hear_noise_state = 0;
    if(last_status != cur_noise_staus || 0 == trace_cnt)
        TRACE(1,"---------------cur_noise_staus = %d---------",cur_noise_staus);
    if(0==trace_cnt && 1==hear_noise_state)
        TRACE(0,"------------- Noise env, pls retry!!! -------------");
    trace_cnt = (trace_cnt+1)%64;
#endif
    return len;
}



static uint32_t codec_playback_callback(uint8_t *buf, uint32_t len)
{
    int POSSIBLY_UNUSED pcm_len = len / sizeof(RX_VOICE_PCM_T) / RX_CHANNEL_NUM;
    RX_VOICE_PCM_T POSSIBLY_UNUSED *pcm_buf = (RX_VOICE_PCM_T *)buf;

    hear_rx_process(pcm_buf, pcm_len);

#if defined(RX_HEAR_COMPEN_DUMP)
    for(int i=0;i<pcm_len;i++)
        dump_buf[i] = (short)(pcm_buf[i] >> 8);
    audio_dump_add_channel_data(0, dump_buf, pcm_len);
    //audio_dump_add_channel_data(1, pcm_buf[0], pcm_len);
    audio_dump_run();
#endif

    return len;
}

int hearing_det_stream_start(bool on)
{
    int ret = 0;
    static bool isRun =  false;
#if defined(RX_HEAR_COMPEN_DUMP) || defined(TX_HEAR_COMPEN_DUMP) 
    enum APP_SYSFREQ_FREQ_T freq = APP_SYSFREQ_104M;
#else
    enum APP_SYSFREQ_FREQ_T freq = APP_SYSFREQ_26M;
#endif
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

        reset_hear_det_para();
        update_spk_calib_gain(hear_spk_calib_cfg);
        get_cur_step();
        get_cur_amp();

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)TX_CHANNEL_NUM;
        stream_cfg.data_size = TX_BUF_SIZE;
        stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)TX_SAMPLE_RATE;
        stream_cfg.bits = (enum AUD_BITS_T)TX_SAMPLE_BITS;
        stream_cfg.vol = 12;
        stream_cfg.chan_sep_buf = false;
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.io_path = AUD_INPUT_PATH_HEARING;
        stream_cfg.handler = codec_capture_callback;
        stream_cfg.data_ptr = codec_capture_buf;

        TRACE(3, "[%s] codec capture sample_rate: %d, data_size: %d", __func__, stream_cfg.sample_rate, stream_cfg.data_size);
        af_stream_open(CODEC_STREAM_ID, AUD_STREAM_CAPTURE, &stream_cfg);
        ASSERT(ret == 0, "codec capture failed: %d", ret);

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)RX_CHANNEL_NUM;
        stream_cfg.data_size = RX_BUF_SIZE;
        stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)RX_SAMPLE_RATE;
        stream_cfg.bits = (enum AUD_BITS_T)RX_SAMPLE_BITS;
        stream_cfg.vol = 16;
        stream_cfg.chan_sep_buf = false; //ture is for no interleave; false is for interleave
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.handler = codec_playback_callback;
        stream_cfg.data_ptr = codec_playback_buf;

        TRACE(3, "[%s] codec playback sample_rate: %d, data_size: %d", __func__, stream_cfg.sample_rate, stream_cfg.data_size);
        af_stream_open(CODEC_STREAM_ID, AUD_STREAM_PLAYBACK, &stream_cfg);
        ASSERT(ret == 0, "codec playback failed: %d", ret);
#if defined(TX_HEAR_COMPEN_DUMP)
        audio_dump_init(TX_FRAME_LEN, sizeof(short), 2);
#elif defined(RX_HEAR_COMPEN_DUMP)
        audio_dump_init(RX_FRAME_LEN, sizeof(short), RX_CHANNEL_NUM);
#endif
#ifdef AUDIO_HEARING_COMPSATN
#ifdef HEARING_USE_STATIC_RAM
        speech_heap_init(HEAR_DET_STREAM_BUF, HEAR_DET_STREAM_BUF_SIZE);
#else
        if(NULL==hear_det_buf)
            syspool_get_buff(&hear_det_buf, HEAR_DET_STREAM_BUF_SIZE);
        speech_heap_init(hear_det_buf, HEAR_DET_STREAM_BUF_SIZE);
#endif
        noise_det_st = NoiseDetection_create(16000,256,&noise_det_cfg);
#endif
        // Start
        af_stream_start(CODEC_STREAM_ID, AUD_STREAM_CAPTURE);
        af_stream_start(CODEC_STREAM_ID, AUD_STREAM_PLAYBACK);
    } else {
        // Close stream
#ifdef AF_STREAM_ID_0_PLAYBACK_FADEOUT
        af_stream_playback_fadeout(AUD_STREAM_ID_0, 10);
#endif
        af_stream_stop(CODEC_STREAM_ID, AUD_STREAM_PLAYBACK);
        af_stream_stop(CODEC_STREAM_ID, AUD_STREAM_CAPTURE);
#ifdef AUDIO_HEARING_COMPSATN
        NoiseDetection_destroy(noise_det_st);
        size_t total = 0, used = 0, max_used = 0;
        speech_memory_info(&total, &used, &max_used);
        TRACE(3,"HEAP INFO: total=%d, used=%d, max_used=%d", total, used, max_used);
        ASSERT(used == 0, "[%s] used != 0", __func__);
#endif
        af_stream_close(CODEC_STREAM_ID, AUD_STREAM_PLAYBACK);
        af_stream_close(CODEC_STREAM_ID, AUD_STREAM_CAPTURE);

        app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_32K);

        //af_set_priority(AF_USER_TEST, osPriorityAboveNormal);

        TRACE(1, "[%s] OFF", __func__);
    }

    isRun=on;
    return 0;   
}


