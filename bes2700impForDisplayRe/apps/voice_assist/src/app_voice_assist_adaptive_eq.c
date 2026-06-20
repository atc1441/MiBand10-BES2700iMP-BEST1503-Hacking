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
#include "hal_trace.h"
#include "iir_process.h"
#include "anc_assist.h"
#include "app_anc_assist.h"
#include "app_voice_assist_adaptive_eq.h"
#include "arm_math.h"
#include "audio_dump.h"

#define ADAPTIVE_EQ_TUNNING_SUPPORT_TEST
// #define APP_VOICE_ASSIST_ADAPTIVE_EQ_AUDIO_DUMP

static int32_t _voice_assist_adaptive_eq_callback(void *buf, uint32_t len, void *other);

#ifdef APP_VOICE_ASSIST_ADAPTIVE_EQ_AUDIO_DUMP
typedef int			_PCM_T;
typedef short		_DUMP_PCM_T;
static _DUMP_PCM_T audio_dump_buf[256];
#endif

static bool g_is_opened_flag = false;
static uint32_t frame_len = 120;

#if defined(ADAPTIVE_EQ_TUNNING_SUPPORT_TEST)
#define ADAPTIVE_EQ_CFG_LIST_NUM     3
static int32_t cnt = 0;
static int8_t cfg_num = 0;

extern uint32_t bt_audio_set_adaptive_dac_iir_eq(IIR_CFG_T *iir_cfg);

IIR_CFG_T adaptive_eq_cfg_0 = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 5,
    .param = {
        {IIR_TYPE_HIGH_SHELF, 2,    2000,   0.7},
        {IIR_TYPE_HIGH_SHELF, -8,    1000,   0.7},
        {IIR_TYPE_HIGH_SHELF, -3,    500,   0.7},
        {IIR_TYPE_HIGH_SHELF, -2,    1500,   0.7},
        {IIR_TYPE_HIGH_SHELF, 1,    2500,   0.7},
    }
};

IIR_CFG_T adaptive_eq_cfg_1 = {
    .gain0 = 3,
    .gain1 = 0,
    .num = 5,
    .param = {
        {IIR_TYPE_HIGH_SHELF, 2,    5000,    0.7},
        {IIR_TYPE_LOW_PASS, 2,    1000,    0.7},
        {IIR_TYPE_HIGH_SHELF, 2,    1500,    0.7},
        {IIR_TYPE_LOW_PASS, 2,    2000,    0.7},
        {IIR_TYPE_HIGH_SHELF, 2,    2500,    0.7},
    }
};

IIR_CFG_T adaptive_eq_cfg_2 = {
    .gain0 = -10,
    .gain1 = 0,
    .num = 2,
    .param = {
        {IIR_TYPE_HIGH_SHELF, 2,    2000,    0.7},
        {IIR_TYPE_HIGH_SHELF, -3,    1000,    0.4},
    }
};

IIR_CFG_T * const POSSIBLY_UNUSED adaptive_eq_cfg_1ist[ADAPTIVE_EQ_CFG_LIST_NUM]={
    &adaptive_eq_cfg_0,
    &adaptive_eq_cfg_1,
    &adaptive_eq_cfg_2,
};
#endif

void customer_algorithm_init(void)
{

#if defined(APP_VOICE_ASSIST_ADAPTIVE_EQ_AUDIO_DUMP)
    audio_dump_init(frame_len, sizeof(_DUMP_PCM_T), 2);
#endif

#if defined(ADAPTIVE_EQ_TUNNING_SUPPORT_TEST)
    cnt = 0;
    cfg_num = 0;
#else

    //Replace with customer algorithm init

#endif

}

void customer_algorithm_process(float* fb_pcm_buf, float* ref_pcm_buf, uint32_t frame_len)
{

#if defined(APP_VOICE_ASSIST_ADAPTIVE_EQ_AUDIO_DUMP)
	audio_dump_clear_up();

	for (uint32_t i = 0; i < frame_len; i++) {
		audio_dump_buf[i] = (_PCM_T)fb_pcm_buf[i] >> 8;
	}
	audio_dump_add_channel_data(0, audio_dump_buf, frame_len);

	for (uint32_t i=0; i < frame_len; i++) {
		audio_dump_buf[i] = (_PCM_T)ref_pcm_buf[i] >> 8;
	}
	audio_dump_add_channel_data(1, audio_dump_buf, frame_len);

    audio_dump_run();

#endif

#if defined(ADAPTIVE_EQ_TUNNING_SUPPORT_TEST)

    IIR_CFG_T *iir_cfg = NULL;
    cnt++;
    if(cnt == 500) // 7.5*500 ms
    {

        TRACE(0,"=========================  change eq  ==============================");
        TRACE(0,"=========================  cfg_num = %d  ==============================",(int)(cfg_num));
        iir_cfg = adaptive_eq_cfg_1ist[cfg_num];

        bt_audio_set_adaptive_dac_iir_eq(iir_cfg);

        cfg_num++;
        if(cfg_num == 3){
            cfg_num = 0;
        }
        cnt = 0;
    }
#else

    //Replace with customer algorithm process

#endif

}

void customer_algorithm_deinit(void)
{
    //Replace with customer algorithm deinit
}

int32_t app_voice_assist_adaptive_eq_init(void)
{
    app_anc_assist_register(ANC_ASSIST_USER_ADAPTIVE_EQ, _voice_assist_adaptive_eq_callback);

    return 0;
}

int32_t app_voice_assist_adaptive_eq_open(void)
{
    customer_algorithm_init();
    app_anc_assist_open(ANC_ASSIST_USER_ADAPTIVE_EQ);
    g_is_opened_flag = true;

    return 0;
}

int32_t app_voice_assist_adaptive_eq_close(void)
{
    g_is_opened_flag = false;
    app_anc_assist_close(ANC_ASSIST_USER_ADAPTIVE_EQ);

    customer_algorithm_deinit();

    return 0;
}

static int32_t _voice_assist_adaptive_eq_callback(void *buf, uint32_t len, void *other)
{
    if(!g_is_opened_flag){
        return 0;
    }

    float * fb_pcm_buf = (float *)buf;
    float * ref_pcm_buf = (float *)other;
    frame_len = len;

    // TRACE(0,"[%s]frame_len: %d", __func__, frame_len);

    customer_algorithm_process(fb_pcm_buf, ref_pcm_buf, frame_len);

    return 0;
}

