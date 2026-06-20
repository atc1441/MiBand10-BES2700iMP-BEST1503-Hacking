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
#include <stdbool.h>
#include "voice_dev.h"
#include "voice_algo.h"
#include "hal_trace.h"
#include "audioflinger.h"
#include "string.h"
#include "audio_dump.h"
#include "voice_dev_defs.h"

/**
 * Algo:
 *      1. Flow:
 *                  set/get algo
 *      voice_dev ----------------> voice_dev_algo --> voice_tx_dc_filter/voice_tx_aec/voice_tx_bf/voice_tx_gain and so on...
 *      
 *      2. Call voice_dev_algo open/close when voice_dev open/close
 *      3. Call voice_dev_ctl to enable/disable algo after voice_dev is opened.
 *      4. voice_dev manage dev_algos
 *      5. provide notification function to notify any changes.
 * */

/**
 * Add notify function, if want different mic map
 **/

typedef enum
{
    VOICE_DEV_STATUS_NONE = 0x00,
    VOICE_DEV_STATUS_CLOSE = 0x01 << 0,
    VOICE_DEV_STATUS_OPEN = 0x01 << 1,
    VOICE_DEV_STATUS_STOP = 0x01 << 2,
    VOICE_DEV_STATUS_START = 0x01 << 3,
} voice_dev_status_t;

// #define VOICE_DEV_AUDIO_DUMP

// TODO: CTRL with vmic
#define PINGPONG_NUM            (2)

#define CHANNEL_NUM             (2)
#define CHANNEL_NUM_MAX         (2)

#define CHANNEL_MAP_AEC         (AUD_CHANNEL_MAP_ECMIC_CH0)
#define CHANNEL_MAP_BF          (AUD_CHANNEL_MAP_CH1)
#define CHANNEL_MAP_BASE        (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_ECMIC_CH0 | AUD_VMIC_MAP_VMIC1)

#define CODEC_SAMPLE_BITS_MAX   (32)
#define CODEC_SAMPLE_BYTES_MAX  SAMPLE_BITS_TO_BYTES(CODEC_SAMPLE_BITS_MAX)

#define CODEC_BUF_SIZE          (FRAME_LEN_MAX * CODEC_SAMPLE_BYTES_MAX * CHANNEL_NUM_MAX * PINGPONG_NUM)

#define CODEC_STREAM_ID         AUD_STREAM_ID_0

static uint8_t __attribute__((aligned(4))) codec_capture_buf[CODEC_BUF_SIZE];

//--------------------status machine--------------------
static voice_dev_status_t dev_user_status[VOICE_DEV_USER_QTY];
static callback_handler_t dev_user_callback[VOICE_DEV_USER_QTY];
static uint32_t dev_user_algos[VOICE_DEV_USER_QTY];
static uint32_t dev_frame_len = 0;
static uint32_t dev_sample_rate = SAMPLE_RATE;
static uint32_t dev_channel_num = CHANNEL_NUM;
static uint32_t dev_channel_map = CHANNEL_MAP_BASE;

// stream parameters
static uint32_t dev_non_interleave = 0;     // NOTE: Will remove this parameter
static uint32_t dev_adc_bits = 0;
static uint32_t dev_frame_ms = 0;
static uint32_t dev_algos = 0;

static uint8_t *dev_cur_buf_ptr[VOICE_DEV_OUT_BUF_PTR_NUM];
// TODO: Need to clean this code
static float noise_est_buf[2];

static uint32_t dev_heartbeat_cnt = 0;

#define DEV_USER_CHECK(user)    {   \
                                ASSERT(user < VOICE_DEV_USER_QTY, "[%s] user(%d) >= VOICE_DEV_USER_QTY", __func__, user);   \
                                TRACE(3, "[%s] user = %d, status = %0x", __func__, user, dev_user_status[user]);}

//--------------------Static function--------------------
static void dev_cur_but_ptr_reset(void)
{
    for (uint32_t i = 0; i < VOICE_DEV_OUT_BUF_PTR_NUM; i++) {
        dev_cur_buf_ptr[i] = NULL;
    }

    dev_cur_buf_ptr[VOICE_DEV_BUF_INDEX_NOISE_EST] = (uint8_t *)noise_est_buf;
}

#define AQE_MIC_DELAY_SAMPLES   (15)

#if defined(AQE_MIC_DELAY_SAMPLES)
static short aqe_mic_delay_buf[2][AQE_MIC_DELAY_SAMPLES];
static uint32_t aqe_mic_delay_buf_pos = 0;

static void aqe_mic_delay_init(void)
{
    memset(aqe_mic_delay_buf, 0, sizeof(aqe_mic_delay_buf));
    aqe_mic_delay_buf_pos = 0;
}

static void aqe_mic_delay_process(short *pcm_buf, uint32_t pcm_len)
{
    uint32_t curr_pos = aqe_mic_delay_buf_pos;
    uint32_t new_pos = !aqe_mic_delay_buf_pos;

    for(uint32_t i=0; i<AQE_MIC_DELAY_SAMPLES; i++) {
        aqe_mic_delay_buf[new_pos][AQE_MIC_DELAY_SAMPLES - 1 - i] = pcm_buf[pcm_len - 1 - i];
    }

    for(int32_t i=pcm_len - 1 - AQE_MIC_DELAY_SAMPLES; i >= 0; i--) {
        pcm_buf[i + AQE_MIC_DELAY_SAMPLES] = pcm_buf[i];
    }

    for(uint32_t i=0; i<AQE_MIC_DELAY_SAMPLES; i++) {
        pcm_buf[i] = aqe_mic_delay_buf[curr_pos][i];
    }

    aqe_mic_delay_buf_pos = !aqe_mic_delay_buf_pos;
}
#endif

static uint32_t dev_user_callback_handler(void)
{
#if defined(AQE_MIC_DELAY_SAMPLES)
    aqe_mic_delay_process((short *)dev_cur_buf_ptr[VOICE_DEV_BUF_INDEX_MIC0], dev_frame_len);
#endif

#ifdef VOICE_DEV_AUDIO_DUMP
    audio_dump_clear_up();
    audio_dump_add_channel_data(0, dev_cur_buf_ptr[VOICE_DEV_BUF_INDEX_REF0], dev_frame_len);
    audio_dump_add_channel_data(1, dev_cur_buf_ptr[VOICE_DEV_BUF_INDEX_MIC0], dev_frame_len);
#endif

    if (dev_algos != VOICE_DEV_ALGO_NONE) {
        voice_algo_process(dev_cur_buf_ptr, dev_frame_len);
    }

#ifdef VOICE_DEV_AUDIO_DUMP
    audio_dump_add_channel_data(2, dev_cur_buf_ptr[VOICE_DEV_BUF_INDEX_MIC0], dev_frame_len);
    audio_dump_run();
#endif

    for (uint32_t i = 0; i < VOICE_DEV_USER_QTY; i++) {
        if ((dev_user_status[i] == VOICE_DEV_STATUS_START) && dev_user_callback[i]) {
            dev_user_callback[i](dev_cur_buf_ptr, dev_frame_len);
        }
    }

    dev_cur_but_ptr_reset();

    return 0;
}

static uint32_t codec_capture_callback(uint8_t *buf, uint32_t len)
{
    short *POSSIBLY_UNUSED pcm_buf = (short *)buf;
    uint32_t POSSIBLY_UNUSED frame_len = len / dev_channel_num;
    uint32_t POSSIBLY_UNUSED interval_len = len * 2 / dev_channel_num;

    // TRACE(3, "[%s] buf = %p, len = %d", __func__, buf, len);

    // If change CHANNEL_NUM or MAINMIC, need find a better way to adaptive configure
    dev_cur_buf_ptr[VOICE_DEV_BUF_INDEX_MIC0] = buf;
    if (dev_non_interleave) {
        dev_cur_buf_ptr[VOICE_DEV_BUF_INDEX_REF0] = buf + len;
    } else {
        dev_cur_buf_ptr[VOICE_DEV_BUF_INDEX_REF0] = NULL;
    }

    dev_user_callback_handler();

    // heartbeat: 6s
    if (++dev_heartbeat_cnt % 125 == 0) {
        TRACE(0, "voice dev is running...");
        dev_heartbeat_cnt = 0;
    }

    return len;
}

static uint32_t codec_capture_open(uint32_t buf_szie)
{
    int ret = 0;
    struct AF_STREAM_CONFIG_T stream_cfg;

    memset(&stream_cfg, 0, sizeof(stream_cfg));
    stream_cfg.bits             = (enum AUD_BITS_T) dev_adc_bits;
    stream_cfg.sample_rate      = (enum AUD_SAMPRATE_T) dev_sample_rate;
    stream_cfg.channel_num      = (enum AUD_CHANNEL_NUM_T) dev_channel_num;
    // stream_cfg.channel_map      = (enum AUD_CHANNEL_MAP_T) dev_channel_map;
    stream_cfg.vol              = 12;
    stream_cfg.device           = AUD_STREAM_USE_INT_CODEC;
    stream_cfg.io_path          = AUD_INPUT_PATH_VOICE_DEV;
    stream_cfg.chan_sep_buf     = dev_non_interleave;
    stream_cfg.data_size        = buf_szie;
    stream_cfg.data_ptr         = codec_capture_buf;
    stream_cfg.handler          = codec_capture_callback;

    TRACE(2, "codec capture sample_rate:%d, data_size:%d", stream_cfg.sample_rate, stream_cfg.data_size);
    ret = af_stream_open(CODEC_STREAM_ID, AUD_STREAM_CAPTURE, &stream_cfg);
    ASSERT(ret == 0, "codec capture failed: %d", ret);

    return 0;
}

static uint32_t codec_capture_close(void)
{
    af_stream_close(CODEC_STREAM_ID, AUD_STREAM_CAPTURE);

    return 0;
}

static uint32_t codec_capture_start(void)
{
    af_stream_start(CODEC_STREAM_ID, AUD_STREAM_CAPTURE);

    return 0;
}

static uint32_t codec_capture_stop(void)
{
    af_stream_stop(CODEC_STREAM_ID, AUD_STREAM_CAPTURE);

    return 0;
}

static int32_t dev_open_impl(void)
{
    int32_t codec_buf_size = 0;
    int32_t codec_sample_bytes = SAMPLE_BITS_TO_BYTES(dev_adc_bits);

    dev_frame_len = FRAME_MS_TO_LEN(dev_sample_rate, dev_frame_ms);

    codec_buf_size = dev_frame_len * codec_sample_bytes * dev_channel_num * PINGPONG_NUM;

    TRACE(3, "[%s] frame_ms = %d, stream_buf_size = %d", __func__, dev_frame_ms, codec_buf_size);

#ifdef VOICE_DEV_AUDIO_DUMP
    audio_dump_init(dev_frame_len, codec_sample_bytes, 3);
#endif

#if defined(AQE_MIC_DELAY_SAMPLES)
    aqe_mic_delay_init();
#endif

    codec_capture_open(codec_buf_size);

    return 0;
}

static int32_t dev_status_reset(void)
{
    dev_heartbeat_cnt = 0;

    dev_non_interleave = 0;
    dev_adc_bits = 0;
    dev_frame_ms = 0;
    dev_algos = VOICE_DEV_ALGO_NONE;

    dev_cur_but_ptr_reset();

    for (uint32_t i = 0; i < VOICE_DEV_USER_QTY; i++) {
        dev_user_status[i] = VOICE_DEV_STATUS_CLOSE;
        dev_user_callback[i] = NULL;
        dev_user_algos[i] = VOICE_DEV_ALGO_NONE;
    }

    dev_frame_len = 0;
    dev_channel_num = CHANNEL_NUM;
    dev_channel_map = CHANNEL_MAP_BASE;   

    return 0;
}

static int32_t dev_user_open(voice_dev_user_t user, callback_handler_t cb, uint32_t algos)
{
    dev_user_callback[user] = cb;
    dev_user_status[user] = VOICE_DEV_STATUS_OPEN;

    dev_user_algos[user] = algos;

    return 0;
}

static int32_t dev_user_close(voice_dev_user_t user)
{
    dev_user_status[user] = VOICE_DEV_STATUS_CLOSE;
    dev_user_callback[user] = NULL;

    dev_user_algos[user] = VOICE_DEV_ALGO_NONE;

    return 0;
}

static int32_t dev_user_start(voice_dev_user_t user)
{
    dev_user_status[user] = VOICE_DEV_STATUS_START;

    voice_algo_start(dev_user_algos[user]);

    return 0;
}

static int32_t dev_user_stop(voice_dev_user_t user)
{
    voice_algo_stop(dev_user_algos[user]);

    dev_user_status[user] = VOICE_DEV_STATUS_STOP;

    return 0;
}

/*
Use lock to deal with different threads to modify dev status

The State Machine:
  +---------------------------------------+
  |                                       v
+------+     +-------+     +------+     +-------+
| open | --> | start | --> | stop | --> | close |
+------+     +-------+     +------+     +-------+
  ^            ^             |            |
  |            +-------------+            |
  |                                       |
  |                                       |
  +---------------------------------------+
*/
static bool dev_user_operate_check_self(voice_dev_user_t user, voice_dev_status_t status)
{
    bool check_res = false;

    if (dev_user_status[user] == VOICE_DEV_STATUS_CLOSE) {
        if (status == VOICE_DEV_STATUS_OPEN) {
            check_res = true;
        }
    } else if (dev_user_status[user] == VOICE_DEV_STATUS_OPEN) {
        if ((status == VOICE_DEV_STATUS_START) || (status == VOICE_DEV_STATUS_CLOSE)) {
            check_res = true;
        }
    } else if (dev_user_status[user] == VOICE_DEV_STATUS_START) {
        if (status == VOICE_DEV_STATUS_STOP) {
            check_res = true;
        }
    } else if (dev_user_status[user] == VOICE_DEV_STATUS_STOP) {
        if ((status == VOICE_DEV_STATUS_CLOSE) || (status == VOICE_DEV_STATUS_START)) {
            check_res = true;
        }
    } else {
        ASSERT(0, "[%s] dev_user_status[%d] = %d is invalid", __func__, user, dev_user_status[user]);
    }

    return check_res;
}

/*
+-------------+--------------+
| user status | other status |
+=============+==============+
| open        | close        |
+-------------+--------------+
| start       | not start    |
+-------------+--------------+
| stop        | not start    |
+-------------+--------------+
| close       | close        |
+-------------+--------------+
*/
static bool dev_user_operate_check_other(voice_dev_user_t user, voice_dev_status_t status)
{
    bool check_res = true;

    if ((status == VOICE_DEV_STATUS_OPEN) || (status == VOICE_DEV_STATUS_CLOSE)) {
        for (uint32_t i = 0; i < VOICE_DEV_USER_QTY; i++) {
            if ((i != user) && dev_user_status[i] != VOICE_DEV_STATUS_CLOSE) {
                check_res = false;
            }
        }
    } else if ((status == VOICE_DEV_STATUS_START) || (status == VOICE_DEV_STATUS_STOP)) {
        for (uint32_t i = 0; i < VOICE_DEV_USER_QTY; i++) {
            if ((i != user) && dev_user_status[i] == VOICE_DEV_STATUS_START) {
                check_res = false;
            }
        }
    } else {
        ASSERT(0, "[%s] dev_user_status[%d] = %d is invalid", __func__, user, dev_user_status[user]);
    }

    return check_res;
}

static bool check_parameter_is_same(const voice_dev_cfg_t *cfg)
{
    if (dev_non_interleave != cfg->non_interleave) {
        return false;
    }

    if (dev_adc_bits != cfg->adc_bits) {
        return false;
    }

    if (dev_frame_ms != cfg->frame_ms) {
        return false;
    }

    // Add more

    return true;
}

static void save_parameter(const voice_dev_cfg_t *cfg)
{
    dev_non_interleave = cfg->non_interleave;
    dev_adc_bits = cfg->adc_bits;
    dev_frame_ms = cfg->frame_ms;

    // Add more
}

static void dump_parameter(const voice_dev_cfg_t *cfg)
{
    TRACE(3, "[%s] dev_non_interleave = %d, non_interleave = %d", __func__, dev_non_interleave, cfg->non_interleave);
    TRACE(3, "[%s] dev_adc_bits = %d, adc_bits = %d", __func__, dev_adc_bits, cfg->adc_bits);
    TRACE(3, "[%s] dev_frame_ms = %d, frame_ms = %d", __func__, dev_frame_ms, cfg->frame_ms);
    TRACE(3, "[%s] dev_algos = %x, algos = %x", __func__, dev_algos, cfg->algos);

    // Add more
}

static POSSIBLY_UNUSED void dev_dump_status(void)
{
    TRACE(1, "---------- %s ----------", __func__);
    for (uint32_t i = 0; i < VOICE_DEV_USER_QTY; i++) {
        TRACE(2, "%d: status = %x", i, dev_user_status[i]);
    }
}

static POSSIBLY_UNUSED int32_t dev_status_is_existed(uint32_t status)
{
    for (uint32_t i = 0; i < VOICE_DEV_USER_QTY; i++) {
        if (dev_user_status[i] & status) {
            return 1;
        }
    }

    return 0;
}

static int32_t dev_algos_is_existed(uint32_t algos)
{
    if ((dev_algos & algos) == algos) {
        return 1;
    } else {
        return 0;
    }

}

static int32_t dev_algo_update(uint32_t algos)
{
    // dev_channel_num = CHANNEL_NUM;
    // dev_channel_map = CHANNEL_MAP_BASE;

    // if (algos & VOICE_DEV_ALGO_AEC) {
    //     dev_channel_num += 1;
    //     dev_channel_map |= CHANNEL_MAP_AEC;
    // }

    // TRACE(2, "[%s] Set channel num(%d) because of AEC", __func__, dev_channel_num);

    if (algos == VOICE_DEV_ALGO_NONE) {
        voice_algo_close();
    } else {
        voice_algo_open(algos);
    }

    dev_algos = algos;

    return 0;
}

static int32_t dev_algo_user_update(voice_dev_user_t user, uint32_t algos)
{
    TRACE(5, "[%s] user = %d, status = %x, old algos = %x, new algos = %x", __func__, user, dev_user_status[user], dev_user_algos[user], algos);

    if (!dev_algos_is_existed(algos)) {
        TRACE(3, "[%s] WARNING: dev_algos = %x, algos = %x", __func__, dev_algos, algos);
        return 1;
    }

    if (dev_user_status[user] == VOICE_DEV_STATUS_START) {
        voice_algo_stop(dev_user_algos[user]);
        voice_algo_start(algos);
    }

    dev_user_algos[user] = algos;

    return 0;
}

//--------------------API function--------------------
int32_t voice_dev_init(void)
{
    dev_status_reset();
    voice_algo_init();

    return 0;
}

int32_t voice_dev_open(voice_dev_user_t user, const voice_dev_cfg_t *cfg)
{   
    DEV_USER_CHECK(user);

    ASSERT((cfg->adc_bits == 16) || (cfg->adc_bits == 32), "[%s] adc_bits(%d) just can be 16 or 32", __func__, cfg->adc_bits);
    ASSERT(cfg->frame_ms <= FRAME_MS_MAX, "[%s] frame_ms(%d) > FRAME_MS_MAX(%d)", __func__, cfg->frame_ms, FRAME_MS_MAX);
    ASSERT(cfg->handler, "[%s] handler == NULL", __func__);
    ASSERT((dev_algos & cfg->algos) == cfg->algos, "[%s] dev_algos = %x, cfg->algos = %x", __func__, dev_algos, cfg->algos);
    ASSERT(dev_algos_is_existed(cfg->algos), "[%s] dev_algos = %x, cfg->algos = %x", __func__, dev_algos, cfg->algos);

    if (dev_user_operate_check_self(user, VOICE_DEV_STATUS_OPEN)) {
        if (dev_user_operate_check_other(user, VOICE_DEV_STATUS_OPEN)) {
            save_parameter(cfg);
            dev_open_impl();
            dev_user_open(user, cfg->handler, cfg->algos);
        } else if (check_parameter_is_same(cfg)) {
            dev_user_open(user, cfg->handler, cfg->algos);
            TRACE(1, "[%s] Other user status is open", __func__);
        } else {
            dump_parameter(cfg);
            ASSERT(0, "[%s] ...", __func__);
        }
    } else {
        ASSERT(0, "[%s] dev_user_status[%d] = %0x", __func__, user, dev_user_status[user]);
    }

    return 0;
}

int32_t voice_dev_close(voice_dev_user_t user)
{
    DEV_USER_CHECK(user);

    if (dev_user_operate_check_self(user, VOICE_DEV_STATUS_CLOSE)) {
        dev_user_close(user);

        if (dev_user_operate_check_other(user, VOICE_DEV_STATUS_CLOSE)) {
            codec_capture_close();
            dev_status_reset();
        } else {
            TRACE(1, "[%s] Other user status is open", __func__);
        }
    } else {
        ASSERT(0, "[%s] dev_user_status[%d] = %0x", __func__, user, dev_user_status[user]);
    }
    
    return 0;
}

int32_t voice_dev_start(voice_dev_user_t user)
{
    DEV_USER_CHECK(user);

    if (dev_user_operate_check_self(user, VOICE_DEV_STATUS_START)) {
        if (dev_user_operate_check_other(user, VOICE_DEV_STATUS_START)) {
            dev_user_start(user);
            codec_capture_start();  
        } else {
            dev_user_start(user);
            TRACE(1, "[%s] Other user status is start", __func__);
        }  
    } else {
        ASSERT(0, "[%s] dev_user_status[%d] = %0x", __func__, user, dev_user_status[user]);
    }

    return 0;
}

int32_t voice_dev_stop(voice_dev_user_t user)
{
    DEV_USER_CHECK(user);

    if (dev_user_operate_check_self(user, VOICE_DEV_STATUS_STOP)) {
        dev_user_stop(user);

        if (dev_user_operate_check_other(user, VOICE_DEV_STATUS_STOP)) {
            codec_capture_stop();
        } else {
            TRACE(1, "[%s] Other user status is start", __func__);
        }
    } else {
        ASSERT(0, "[%s] dev_user_status[%d] = %d", __func__, user, dev_user_status[user]);
    }

    return 0;
}

/**
 * ctl VOICE_DEV_SET_USER_ALGOS after VOICE_DEV_SET_SUPPORT_ALGOS. can ctl before or after start/stop
 **/ 
int32_t voice_dev_ctl(voice_dev_user_t user, voice_dev_ctl_t ctl, void *ptr)
{
    DEV_USER_CHECK(user);

    TRACE(2, "[%s] ctl = %d", __func__, ctl);

    switch(ctl) {
        case VOICE_DEV_SET_SUPPORT_ALGOS: {
                uint32_t algos = (*(int32_t*)ptr);
                dev_algo_update(algos);
            }
            break;

        case VOICE_DEV_SET_USER_ALGOS: {
                uint32_t algos = (*(int32_t*)ptr);
                dev_algo_user_update(user, algos);
            }
            break; 

        case VOICE_DEV_GET_ALGO:
            (*(uint32_t*)ptr) = dev_algos;
            break;

        case VOICE_DEV_SET_ALGO_FRAME_LEN:
            voice_algo_ctl(VOICE_ALGO_SET_FRAME_LEN, ptr);
            break;

        default:
            TRACE(2, "[%s] ctl(%d) is not valid", __func__, ctl);
            return -1;
    }

    return 0;
}

// For test voice_dev module

#define VOICE_DEV_USER_TEST             VOICE_DEV_USER8

static uint32_t test_callback_handler(uint8_t *buf_ptr[], uint32_t frame_len)
{
    // TRACE(2, "[%s] frame_len = %d", __func__, frame_len);

    return 0;
}

voice_dev_cfg_t voice_dev_cfg_test = {
    .non_interleave = 1,
    .adc_bits = 16,
    .frame_ms = 16,
    // .algos = VOICE_DEV_ALGO_NONE,
    .algos = VOICE_DEV_ALGO_AEC,
    .handler = test_callback_handler,
};

void test_voice_dev_open(void)
{
    voice_dev_open(VOICE_DEV_USER_TEST, &voice_dev_cfg_test);
}

void test_voice_dev_start(void)
{
    voice_dev_start(VOICE_DEV_USER_TEST);
}

void test_voice_dev_stop(void)
{
    voice_dev_stop(VOICE_DEV_USER_TEST);
}

void test_voice_dev_close(void)
{
    voice_dev_close(VOICE_DEV_USER_TEST);
}