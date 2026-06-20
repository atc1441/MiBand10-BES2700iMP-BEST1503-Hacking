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

#ifdef VOICE_DETECTOR_EN
#include "string.h"
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "hwtimer_list.h"
#include "sens_msg.h"
#include "audioflinger.h"
#include "sensor_hub_core.h"
#include "sensor_hub_core_app.h"
#include "sensor_hub_vad_core.h"
#include "sensor_hub_vad_adapter.h"
#include "app_voice_detector.h"
#ifdef AI_VOICE
#include "sensor_hub_core_app_ai.h"
#include "sensor_hub_core_app_ai_voice.h"
#endif
#ifdef VAD_ADPT_TEST_ENABLE
#include "sensor_hub_vad_adapter_tc.h"
#endif

#ifdef VAD_ADPT_CLOCK_FREQ
#if (VAD_ADPT_CLOCK_FREQ == 1)
#define VAD_ADPT_DEMO_FREQ HAL_CMU_FREQ_26M
#elif (VAD_ADPT_CLOCK_FREQ == 2)
#define VAD_ADPT_DEMO_FREQ HAL_CMU_FREQ_52M
#elif (VAD_ADPT_CLOCK_FREQ == 3)
#define VAD_ADPT_DEMO_FREQ HAL_CMU_FREQ_104M
#elif (VAD_ADPT_CLOCK_FREQ == 4)
#define VAD_ADPT_DEMO_FREQ HAL_CMU_FREQ_6P5M
#elif (VAD_ADPT_CLOCK_FREQ == 5)
#define VAD_ADPT_DEMO_FREQ HAL_CMU_FREQ_13M
#endif
#endif

#define VAD_ADPT_DEMO_STREAM_TEST

#ifdef VAD_ADPT_DEMO_STREAM_TEST
/*
 * VAD adapter interface data initializations
 **************************************************************
 */
/*
 * capture stream initilizations
 **************************************************************
 */
#define ALIGNED4                        ALIGNED(4)
#define NON_EXP_ALIGN(val, exp)         (((val) + ((exp) - 1)) / (exp) * (exp))
#define AF_DMA_LIST_CNT                 4
#define CHAN_NUM_CAPTURE                1
#define RX_BURST_NUM                    4
#define RX_ALL_CH_DMA_BURST_SIZE        (RX_BURST_NUM * 2 * CHAN_NUM_CAPTURE)
#define RX_BUFF_ALIGN                   (RX_ALL_CH_DMA_BURST_SIZE * AF_DMA_LIST_CNT)

#define CAPTURE_SAMP_RATE               16000 //48000
#define CAPTURE_SAMP_BITS               16
#define CAPTURE_SAMP_SIZE               2
#define CAPTURE_CHAN_NUM                CHAN_NUM_CAPTURE
#define CAPTURE_FRAME_NUM               20
#define CAPTURE_FRAME_SIZE              (CAPTURE_SAMP_RATE / 1000 * CAPTURE_SAMP_SIZE * CHAN_NUM_CAPTURE)
#define CAPTURE_SIZE                    NON_EXP_ALIGN(CAPTURE_FRAME_SIZE * CAPTURE_FRAME_NUM, RX_BUFF_ALIGN)
static uint8_t ALIGNED4 sens_codec_cap_buf[CAPTURE_SIZE];
static uint32_t demo_cap_cnt = 0;

uint32_t sensor_hub_vad_adpt_demo_capture_stream(uint8_t *buf, uint32_t len)
{
#if 0
    int16_t *pd = (int16_t *)buf;
    TRACE(4,"buf=%x, len=%d, mic data=[%d %d %d %d]", (int)pd,len,pd[0],pd[1],pd[2],pd[3]);
#endif
    demo_cap_cnt++;
    if (demo_cap_cnt >= (len/32*10)) {
        enum HAL_CMU_FREQ_T idx = hal_sysfreq_get();
        uint32_t freq[] = {0,6,13,24,48,72,96,192};
        TRACE(4, "demo_cap: idx=%d, CPU freq=%d MHz", idx, freq[idx]);
        demo_cap_cnt = 0;
    }
#ifdef VAD_ADPT_TEST_ENABLE
    vad_adpt_test_cap_stream_handler(buf, len);
#endif
    return 0;
}

uint32_t sensor_hub_vad_adpt_demo_stream_prepare_start(uint8_t *buf, uint32_t len)
{
    app_sensor_hub_core_vad_send_evt_msg(SENS_EVT_ID_VAD_VOICE_CMD, 0, 0, 0);
    return 0;
}

void sensor_hub_vad_adpt_demo_init_capture_stream(struct AF_STREAM_CONFIG_T *cfg)
{
    TRACE(0, "%s:", __func__);

    memset(cfg, 0, sizeof(struct AF_STREAM_CONFIG_T));

    //init capture stream
    //adapter will setup default handler for capture stream
    cfg->sample_rate  = CAPTURE_SAMP_RATE;
    cfg->bits         = CAPTURE_SAMP_BITS;
    cfg->channel_num  = CAPTURE_CHAN_NUM;
    cfg->channel_map  = 0;
    cfg->device       = AUD_STREAM_USE_INT_CODEC;
    cfg->vol          = TGT_ADC_VOL_LEVEL_7;
    cfg->handler      = NULL;
    cfg->io_path      = AUD_INPUT_PATH_VADMIC;
    cfg->data_ptr     = sens_codec_cap_buf;
    cfg->data_size    = CAPTURE_SIZE;

    demo_cap_cnt = 0;
}
#endif

#ifdef SNDP_VAD_ENABLE
uint32_t sensor_hub_vad_adpt_sndp_stream_prepare_start(uint8_t *buf, uint32_t len);
uint32_t sensor_hub_vad_adpt_sndp_capture_stream(uint8_t *buf, uint32_t len);
void sensor_hub_vad_adpt_sndp_init_capture_stream(struct AF_STREAM_CONFIG_T *cfg);
uint32_t sensor_hub_vad_adpt_sndp_stream_prepare_stop(uint8_t *buf, uint32_t len);
#endif

static app_vad_adpt_if_t app_vad_adpt_ifs[] = 
{
#ifdef VAD_ADPT_DEMO_STREAM_TEST
    {
        .id = APP_VAD_ADPT_ID_DEMO,
#ifdef VAD_ADPT_DEMO_FREQ
        .cap_freq = VAD_ADPT_DEMO_FREQ,
#else
        .cap_freq = HAL_CMU_FREQ_52M,
#endif
        .stream_prepare_start_handler = sensor_hub_vad_adpt_demo_stream_prepare_start,
        .stream_prepare_stop_handler = NULL,
        .stream_handler = sensor_hub_vad_adpt_demo_capture_stream,
        .init_func      = sensor_hub_vad_adpt_demo_init_capture_stream,
    },
#endif
#ifdef AI_VOICE
    {
        .id = APP_VAD_ADPT_ID_AI_KWS,
        .cap_freq = HAL_CMU_FREQ_52M,
        .stream_prepare_start_handler = sensor_hub_voice_kws_stream_prepare_start,
        .stream_prepare_stop_handler = sensor_hub_voice_kws_stream_prepare_stop,
        .stream_handler = sensor_hub_voice_kws_raw_pcm_data_input_signal,
        .init_func      = sensor_hub_voice_kws_init_capture_stream,
    },
#endif
#ifdef SNDP_VAD_ENABLE
    {
        .id = APP_VAD_ADPT_ID_SNDP,
        .cap_freq = HAL_CMU_FREQ_52M,
        .stream_prepare_start_handler = sensor_hub_vad_adpt_sndp_stream_prepare_start,
        .stream_prepare_stop_handler = sensor_hub_vad_adpt_sndp_stream_prepare_stop,
        .stream_handler = sensor_hub_vad_adpt_sndp_capture_stream,
        .init_func      = sensor_hub_vad_adpt_sndp_init_capture_stream,
    },
#endif
};

app_vad_adpt_if_t *app_vad_adapter_get_if(enum APP_VAD_ADPT_ID_T id)
{
    uint32_t i;

    for (i = 0; i < ARRAY_SIZE(app_vad_adpt_ifs); i++) {
        if (app_vad_adpt_ifs[i].id == id) {
            return &app_vad_adpt_ifs[i];
        }
    }
    ASSERT(0, "%s: invalid id=%d", __func__, id);
    return NULL;
}

#endif
