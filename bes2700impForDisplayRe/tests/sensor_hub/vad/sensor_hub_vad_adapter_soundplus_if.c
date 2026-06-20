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
#ifdef SNDP_VAD_ENABLE
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
//#include "sensor_hub_core_app_soundplus.h"

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
#define CAPTURE_FRAME_NUM               180
#define CAPTURE_FRAME_SIZE              (CAPTURE_SAMP_RATE / 1000 * CAPTURE_SAMP_SIZE * CHAN_NUM_CAPTURE)
#define CAPTURE_SIZE                    NON_EXP_ALIGN(CAPTURE_FRAME_SIZE * CAPTURE_FRAME_NUM, RX_BUFF_ALIGN)
static uint8_t ALIGNED4 sens_codec_cap_buf[CAPTURE_SIZE];

int soundplus_kws_init();
int soundplus_kws_recognition(short* in, int numsamples);
int soundplus_kws_deinit(void);

uint32_t sensor_hub_vad_adpt_sndp_capture_stream(uint8_t *buf, uint32_t len)
{
#if 0
    int16_t *pd = (int16_t *)buf;
    TRACE(4,"buf=%x, len=%d, mic data=[%d %d %d %d]", (int)pd,len,pd[0],pd[1],pd[2],pd[3]);
#endif
    int ret= soundplus_kws_recognition((short *)buf, len/2);
    if(ret != -1) {
        //TRACE(2,"retcode: %d, %d", retcode, sndp_counter);
        TRACE(1,"retcode: %d", ret);
        app_sensor_hub_core_vad_send_evt_msg(SENS_EVT_ID_VAD_VOICE_CMD, ret+1, 0, 0);
    }
    return 0;
}

uint32_t sensor_hub_vad_adpt_sndp_stream_prepare_start(uint8_t *buf, uint32_t len)
{
    soundplus_kws_init();
    app_sensor_hub_core_vad_send_evt_msg(SENS_EVT_ID_VAD_VOICE_CMD, 0, 0, 0);
    return 0;
}

uint32_t sensor_hub_vad_adpt_sndp_stream_prepare_stop(uint8_t *buf, uint32_t len)
{
    soundplus_kws_deinit();
    app_sensor_hub_core_vad_send_evt_msg(SENS_EVT_ID_VAD_VOICE_CMD, 0, 0, 0);
    return 0;
}

void sensor_hub_vad_adpt_sndp_init_capture_stream(struct AF_STREAM_CONFIG_T *cfg)
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
    cfg->vol          = TGT_ADC_VOL_LEVEL_15;
    cfg->handler      = NULL;
    cfg->io_path      = AUD_INPUT_PATH_VADMIC;
    cfg->data_ptr     = sens_codec_cap_buf;
    cfg->data_size    = CAPTURE_SIZE;
}
#endif
#endif
