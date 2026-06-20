/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#if !defined(CHIP_SUBSYS_SENS) && defined(VAD_IF_TEST)
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_trace.h"
#include "app_sensor_hub.h"
#include "sensor_hub.h"
#include "mcu_sensor_hub_app.h"
#include "analog.h"
#include "string.h"
#include "sens_msg.h"
#include "hal_aud.h"
#include "hal_timer.h"
#include "audioflinger.h"
#include "hal_sysfreq.h"
//#include "hal_iomux.h"

#include "mcu_sensor_hub_app_ai.h"
/*
Open: automatically stop log output after voice recognition for a period of time
Shield: Log output after voice recognition
*/
#define VAD_IRQ_TEST
//#define PLAY_VAD_DATA_TEST
#define VAD_POWER_TEST
//#define MCU_SENSOR_HUB_AUDIO_DUMP_EN

#define VAD_DAC2_TEST

#ifdef MCU_SENSOR_HUB_AUDIO_DUMP_EN
#include "audio_dump.h"
#endif
#ifdef VAD_DAC2_TEST
#include "apps.h"
#endif

#define ALIGNED4 ALIGNED(4)

#define VAD_REQ_PKT_CNT 200//600
volatile static bool vad_req_start = false;
volatile static bool vad_pkt_req = false;
volatile static uint32_t vad_pkt_cnt = 0;
POSSIBLY_UNUSED static uint32_t wpos = 0;
POSSIBLY_UNUSED static uint32_t rpos = 0;

#define PLAYBACK_CHAN (1)
#define PLAY_FRM_NUM  (4)
#define PLAY_BUFF_SIZE (APP_VAD_DATA_PKT_SIZE * PLAYBACK_CHAN * PLAY_FRM_NUM)

#define CAPTURE_CHAN  (1)
#define CAP_FRM_NUM   (8)
#define CAP_BUFF_SIZE (APP_VAD_DATA_PKT_SIZE * CAPTURE_CHAN * CAP_FRM_NUM)

#ifdef PLAY_VAD_DATA_TEST
volatile static bool vad_start_play = false;
volatile static bool vad_play_state = false;
static uint8_t ALIGNED4 play_buff[PLAY_BUFF_SIZE];
static uint8_t ALIGNED4 capture_buff[CAP_BUFF_SIZE];

static enum AUD_CHANNEL_NUM_T chan_num_to_enum(uint32_t num)
{
    if (num == 2) {
        return AUD_CHANNEL_NUM_2;
    } else {
        return AUD_CHANNEL_NUM_1;
    }
}

static uint32_t codec_data_play_handler(uint8_t *buf, uint32_t len)
{
    uint32_t avail;
    uint32_t time = TICKS_TO_MS(hal_sys_timer_get());

    if (wpos >= rpos) {
        avail = wpos - rpos;
    } else {
        avail = sizeof(capture_buff) - rpos + wpos;
    }

    if (avail * PLAYBACK_CHAN / CAPTURE_CHAN >= len) {
        int16_t *src     = (int16_t *)(capture_buff + rpos);
        int16_t *src_end = (int16_t *)(capture_buff + sizeof(capture_buff));
        int16_t *dst     = (int16_t *)(buf);
        int16_t *dst_end = (int16_t *)(buf+len);

        while (dst < dst_end) {
            *dst++ = *src++;
            if (src == src_end) {
                src = (int16_t *)capture_buff;
            }
#if (PLAYBACK_CHAN == 2)
            *dst = *(dst - 1);
            dst++;
#endif
        }
        rpos = (uint32_t)src - (uint32_t)capture_buff;
    } else {
        memset(buf, 0, len);
        TRACE(0, "avail=%d",avail);
    }
    TRACE(0,"[%u] CODEC_PLAY:buf=%x,len=%d,rpos=%d",time,(int)buf,len,rpos);
    return 0;
}

static uint32_t chan_num_to_map(uint32_t num)
{
    if (num == 1) {
        return AUD_CHANNEL_MAP_CH0;
    } else {
        return AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1;
    }
}

int app_mcu_vad_start_play(bool on)
{
    int ret;
    struct AF_STREAM_CONFIG_T stream_cfg;

    TRACE(3, "%s:", __func__);

    memset(&stream_cfg, 0, sizeof(stream_cfg));

    if (on) {
        hal_sysfreq_req(HAL_SYSFREQ_USER_APP_3, HAL_CMU_FREQ_104M);

        stream_cfg.bits        = AUD_BITS_16;
        stream_cfg.channel_num = chan_num_to_enum(PLAYBACK_CHAN);
        stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)chan_num_to_map(PLAYBACK_CHAN);
        stream_cfg.sample_rate = AUD_SAMPRATE_16000;
        stream_cfg.device      = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.vol         = 15;
        stream_cfg.handler     = codec_data_play_handler;
        stream_cfg.io_path     = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.data_ptr    = play_buff;
        stream_cfg.data_size   = sizeof(play_buff);

        TRACE(3," sample_rate=%d, bits=%d", stream_cfg.sample_rate, stream_cfg.bits);
        TRACE(3, "playback_buff=%x, size=%d", (int)play_buff, sizeof(play_buff));
        TRACE(3, "capture_buff=%x, size=%d", (int)capture_buff, sizeof(capture_buff));

        ret = af_stream_open(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK, &stream_cfg);
        ASSERT(ret == 0, "af_stream_open playback failed: %d", ret);
        ret = af_stream_start(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK);
        ASSERT(ret == 0, "af_stream_start playback failed: %d", ret);
    } else {
        ret = af_stream_stop(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK);
        ret += af_stream_close(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK);

        hal_sysfreq_req(HAL_SYSFREQ_USER_APP_3, HAL_CMU_FREQ_32K);
    }
    return 0;
}
#endif

static void app_mcu_vad_event_handler(enum APP_VAD_EVENT_T event, uint8_t *param)
{
    TRACE(0, "%s: event=%d", __func__, event);
    if (event == APP_VAD_EVT_VOICE_CMD) {
        if (!vad_req_start) {
            vad_pkt_cnt = 0;
            vad_req_start = true;
            app_mcu_sensor_hub_request_vad_data(true,0);
            TRACE(0, "APP_VAD_EVT_VOICE_CMD");
#ifdef PLAY_VAD_DATA_TEST
            //app_mcu_sensor_hub_bypass_vad(true);
            vad_start_play = true;
#endif
#ifdef VAD_DAC2_TEST
            //media_PlayAudio(AUD_ID_POWER_ON,0);
#endif
        }
    }
}

static void print_vad_data(uint8_t *buf, uint32_t len)
{
    static int cnt = 0;
    int16_t *p = (int16_t *)buf;
    uint32_t n = len / 2;

    TRACE(0, "VAD_DATA[%d]: %d %d %d %d -- %d %d %d %d", cnt++,
        p[0],p[1],p[2],p[3], p[n-4], p[n-3], p[n-2], p[n-1]);
}

static void app_mcu_vad_save_data(uint8_t *buf, uint32_t len)
{
    uint32_t time = TICKS_TO_MS(hal_sys_timer_get());

#ifdef PLAY_VAD_DATA_TEST
    int16_t *dst = (int16_t *)(capture_buff + wpos);
    int16_t *dst_end = (int16_t *)(capture_buff + sizeof(capture_buff));
    int16_t *src = (int16_t *)buf;
    int16_t *src_end = (int16_t *)(buf+len);

    while (src < src_end) {
        *dst++ = *src++;
        if (dst == dst_end) {
            dst = (int16_t *)(capture_buff);
        }
    }
    wpos += len;
    if (wpos >= sizeof(capture_buff)) {
        wpos -= sizeof(capture_buff);
    }
#endif

#ifdef MCU_SENSOR_HUB_AUDIO_DUMP_EN
    audio_dump_add_channel_data(0, (void *)buf, len/2);
    audio_dump_run();
#endif
    print_vad_data(buf, len);
    TRACE(0,"[%u] VAD_SAVE:buf=%x,len=%d,wpos=%d",time,(int)buf,len,wpos);
}

static void app_mcu_vad_data_handler(uint32_t pkt_id, uint8_t *payload, uint32_t bytes)
{
    TRACE(0, "%s: pkt_id=%d, bytes=%d", __func__, pkt_id, bytes);
    app_mcu_vad_save_data(payload, bytes);
    vad_pkt_cnt++;
    vad_pkt_req = true;

#ifdef VAD_DAC2_TEST
    if (vad_pkt_cnt == 20) {
        media_PlayAudio(AUD_ID_POWER_ON,0);
    }
#endif
#ifdef VAD_IRQ_TEST
    if (vad_pkt_cnt >= VAD_REQ_PKT_CNT) {
        vad_req_start = false;
        app_mcu_sensor_hub_request_vad_data(false,0);
//        app_mcu_sensor_hub_bypass_vad(false);
        hal_sysfreq_req(HAL_SYSFREQ_USER_APP_12, HAL_CMU_FREQ_32K);
    }
#endif
}

POSSIBLY_UNUSED static void app_mcu_vad_if_ai_test(void)
{
    app_mcu_sensor_hub_start_vad(APP_VAD_ADPT_ID_AI_KWS);
    app_sensor_hub_ai_mcu_activate_ai_user(SENSOR_HUB_AI_USER_GG,1);
    app_sensor_hub_ai_mcu_activate_ai_user(SENSOR_HUB_AI_USER_ALEXA,1);
}

POSSIBLY_UNUSED static void app_mcu_vad_if_stream_test(void)
{
    app_mcu_sensor_hub_start_vad(APP_VAD_ADPT_ID_DEMO);
}

void app_mcu_vad_if_test(void)
{
    TRACE(0, "%s: start", __func__);

    // waiting for SENSOR CPU boot done
    osDelay(200);

    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_26M);

#if 0
    hal_iomux_set_clock_out();
    hal_cmu_clock_out_enable(HAL_CMU_CLOCK_OUT_SENS_SYS);
#endif

#ifdef MCU_SENSOR_HUB_AUDIO_DUMP_EN
    audio_dump_init(APP_VAD_DATA_PKT_SIZE/2, 2, 1);
#endif

    app_mcu_sensor_hub_setup_vad_event_handler(app_mcu_vad_event_handler);
    app_mcu_sensor_hub_setup_vad_data_handler(app_mcu_vad_data_handler);
//    app_mcu_vad_if_ai_test();
    app_mcu_vad_if_stream_test();
#ifdef PLAY_VAD_DATA_TEST
    app_mcu_vad_start_play(true);
#endif

#ifdef VAD_POWER_TEST
    // waiting for VAD start done
    //hal_sys_timer_delay(MS_TO_TICKS(50));
    // release system high freq clock
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);
#endif
    hal_sysfreq_print_user_freq();
    TRACE(0, "cpu freq=%d", hal_sys_timer_calc_cpu_freq(5,0));

    while (1) {
#ifdef PLAY_VAD_DATA_TEST
        if (vad_start_play) {
            vad_start_play = false;
            if (vad_play_state == false) {
                vad_play_state = true;
                //app_mcu_sensor_hub_bypass_vad(true);
                //app_mcu_vad_start_play(true);
            }
        }
#endif
        if (vad_req_start) {
            if (vad_pkt_req == true) {
                vad_pkt_req = false;
                TRACE(0,"%s: ========> req one pkt[%d]", __func__,vad_pkt_cnt);
            }
        }
#ifdef VAD_POWER_TEST
        osSignalWait(0, osWaitForever);
#endif
    }
}
#endif
