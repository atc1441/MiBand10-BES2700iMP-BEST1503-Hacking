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
/**
 * Usage:
 *  1.  Enable SCO_CP_ACCEL ?= 1 to enable dual core in sco
 *  2.  Enable SCO_TRACE_CP_ACCEL ?= 1 to see debug log.
 *  3.  Change channel number if the algo(run in cp) input is more than one channel: sco_cp_init(speech_tx_frame_len, 1);
 *  4.  The code between SCO_CP_ACCEL_ALGO_START(); and SCO_CP_ACCEL_ALGO_END(); will run in CP core.
 *  5.  These algorithms will work in AP. Need to move this algorithms from overlay to fast ram.
 *
 * NOTE:
 *  1.  spx fft and hw fft will share buffer, so just one core can use these fft.
 *  2.  audio_dump_add_channel_data function can not work correctly in CP core, because
 *      audio_dump_add_channel_data is possible called after audio_dump_run();
 *  3.  AP and CP just can use 85%
 *
 *
 *
 * TODO:
 *  1.  FFT, RAM, CODE overlay
 **/
#if defined(SCO_CP_ACCEL)
#include "cp_accel.h"
#include "hal_location.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "cmsis_os.h"
#include "speech_cfg.h"
#include "math.h"
#include "norflash_api.h"
#if defined(SNDP_TX_3MIC_ENABLE)
#include "Soundplus_adapter.h"
#endif
#include "app_bt_stream.h"
#include "app_anc.h"

// malloc data from pool in init function
#define FRAME_LEN_MAX           (256)
#define CHANNEL_NUM_MAX         (3)

enum CP_SCO_STATE_T {
    CP_SCO_STATE_NONE = 0,
    CP_SCO_STATE_IDLE,
    CP_SCO_STATE_WORKING,
};

enum SCO_CP_CMD_T {
    SCO_CP_CMD_PRO = 0,
    SCO_CP_CMD_OTHER,

    SCO_CP_CMD_QTY,
};

#undef CP_DATA_LOC
#undef CP_BSS_LOC

#define RAM_SCO_LOC_A(n, l)               __attribute__((section(#n "." #l)))
#define RAM_SCO_LOC(n, l)                 RAM_SCO_LOC_A(n, l)

#define CP_DATA_LOC                       RAM_SCO_LOC(.overlay_sco_cache, __LINE__)
#define CP_BSS_LOC                        RAM_SCO_LOC(.overlay_sco_cache, __LINE__)

static CP_DATA_LOC enum CP_SCO_STATE_T g_cp_state = CP_SCO_STATE_NONE;

// TODO: Use malloc to replace array
// static CP_BSS_LOC char g_cp_heap_buf[1024 * 100];
static CP_BSS_LOC float g_in_pcm_buf[FRAME_LEN_MAX * CHANNEL_NUM_MAX];
static CP_BSS_LOC short g_out_pcm_buf[FRAME_LEN_MAX * CHANNEL_NUM_MAX];
static CP_BSS_LOC float g_in_fb_buf[FRAME_LEN_MAX];
static CP_BSS_LOC int g_pcm_len;
static CP_BSS_LOC int g_cvsd_flag;  //add by rex

static CP_BSS_LOC int g_frame_len;
static CP_BSS_LOC int g_channel_num;

static CP_BSS_LOC sco_status_t g_sco_status_in;
static CP_BSS_LOC sco_status_t g_sco_status_out;

static int g_require_cnt = 0;
static int g_run_cnt = 0;

int sco_cp_process(short *pcm_buf, float *in_buf, float *fb_buf, int *_pcm_len, sco_status_t *status)
{
    int32_t pcm_len = *_pcm_len;
    uint32_t wait_cnt = 0;

    ASSERT(g_frame_len * g_channel_num == pcm_len, "[%s] g_frame_len(%d) * g_channel_num(%d) != pcm_len(%d)", __func__, g_frame_len, g_channel_num, pcm_len);

    // Check CP has new data to get and can get data from buffer
#if defined(SCO_TRACE_CP_ACCEL)
    TRACE(4,"[%s] g_require_cnt: %d, status: %d, pcm_len: %d", __func__, g_require_cnt, g_cp_state, pcm_len);
#endif

    while (g_cp_state == CP_SCO_STATE_WORKING)
    {
        hal_sys_timer_delay_us(10);

        if (wait_cnt++ > 300000) {      // 3s
            ASSERT(0, "cp is hung %d", g_cp_state);
        }
    }

    if (g_cp_state == CP_SCO_STATE_IDLE)
    {
        speech_copy_f32(g_in_pcm_buf, in_buf, pcm_len);

        if (fb_buf)
        {
            speech_copy_f32(g_in_fb_buf, fb_buf, pcm_len / g_channel_num);
        }
        memcpy(&g_sco_status_in, status, sizeof(sco_status_t));

        speech_copy_int16(pcm_buf, g_out_pcm_buf, g_pcm_len);
        memcpy(status, &g_sco_status_out, sizeof(sco_status_t));

        *_pcm_len = g_pcm_len;
        g_pcm_len = pcm_len;

        g_require_cnt++;
        g_cp_state = CP_SCO_STATE_WORKING;
        cp_accel_send_event_mcu2cp(CP_BUILD_ID(CP_TASK_SCO, CP_EVENT_SCO_PROCESSING));
    }
    else
    {
        // Multi channels to one channel
#if 0
        for (int i = 0; i < pcm_len / g_channel_num; i++)
        {
            pcm_buf[i] = pcm_buf[2*i];
        }

        *_pcm_len = pcm_len / g_channel_num;
#endif

        // Check abs(g_require_cnt - g_run_cnt) > threshold, reset or assert

        TRACE(2,"[%s] ERROR: status = %d", __func__, g_cp_state);
    }

    return 0;
}

extern int sco_cp_algo(short *pcm_buf, short *ref_buf, int *_pcm_len, sco_status_t *status);

CP_TEXT_SRAM_LOC
static unsigned int sco_cp_main(uint8_t event)
{
#if defined(SCO_TRACE_CP_ACCEL)
    TRACE(2,"[%s] g_run_cnt: %d", __func__, g_run_cnt);
#endif

    // LOCK BUFFER

#if defined(SNDP_TX_3MIC_ENABLE)
#if defined(SNDP_TX_3MIC_TIME_ENABLE)
    uint32_t start_ticks, end_ticks;
    start_ticks = hal_fast_sys_timer_get();
#endif
    //AncFlag: default 0 -- anc off
    soundplus_deal_Tx(g_out_pcm_buf, g_in_pcm_buf, g_in_fb_buf, g_pcm_len/g_channel_num,app_anc_work_status());
    if(g_cvsd_flag == 1)          //pcm_len
    {
        g_pcm_len = g_pcm_len / (g_channel_num * 2);
    }
    else
    {
        g_pcm_len = g_pcm_len / g_channel_num;
    }
#if defined(SNDP_TX_3MIC_TIME_ENABLE)
    end_ticks = hal_fast_sys_timer_get();
    TRACE(4,"[sco_cp_process] takes %d us\n", FAST_TICKS_TO_US(end_ticks - start_ticks));
#endif

    // process pcm
#elif 0
    // speech_copy_int16(g_out_pcm_buf, g_in_pcm_buf, g_pcm_len);

    for (int i = 0; i < g_pcm_len; i++)
    {
        g_out_pcm_buf[i] = (short)(sinf(2 * 3.1415926 * i / 16 ) * 10000);
    }
#else
    sco_cp_algo(g_in_pcm_buf, g_in_ref_buf, &g_pcm_len, &g_sco_status_in);
    speech_copy_int16(g_out_pcm_buf, g_in_pcm_buf, g_pcm_len);
    memcpy(&g_sco_status_out, &g_sco_status_in, sizeof(sco_status_t));
#endif

    // set status
    g_run_cnt++;
    g_cp_state = CP_SCO_STATE_IDLE;

#if defined(SCO_TRACE_CP_ACCEL)
    TRACE(1,"[%s] CP_SCO_STATE_IDLE", __func__);
#endif

    // UNLOCK BUFFER

    return 0;
}

static const struct cp_task_desc task_desc_sco = {CP_ACCEL_STATE_CLOSED, sco_cp_main, NULL, NULL, NULL};
int sco_cp_init(int frame_len, int channel_num)
{
    TRACE(3,"[%s] frame_len: %d, channel_num: %d", __func__, frame_len, channel_num);
    ASSERT(frame_len <= FRAME_LEN_MAX, "[%s] frame_len(%d) > FRAME_LEN_MAX", __func__, frame_len);
    ASSERT(channel_num <= CHANNEL_NUM_MAX, "[%s] channel_num(%d) > CHANNEL_NUM_MAX", __func__, channel_num);

    g_require_cnt = 0;
    g_run_cnt = 0;

    cp_accel_init();
    norflash_api_flush_disable(NORFLASH_API_USER_CP,(uint32_t)cp_accel_init_done, false);
    cp_accel_open(CP_TASK_SCO, &task_desc_sco);

    uint32_t cnt=0;
    while(cp_accel_init_done() == false) {
        hal_sys_timer_delay_us(100);
        cnt++;
        if (cnt % 10 == 0) {
            if (cnt == 10 * 200) {     // 200ms
                ASSERT(0, "[%s] ERROR: Can not init cp!!!", __func__);
            } else {
                TRACE(1, "[%s] Wait CP init done...%d(ms)", __func__, cnt/10);
            }
        }
    }
    norflash_api_flush_enable(NORFLASH_API_USER_CP);
#if 0
    speech_heap_cp_start();
    speech_heap_add_block(g_cp_heap_buf, sizeof(g_cp_heap_buf));
    speech_heap_cp_end();
#endif

    g_frame_len = frame_len;
    g_channel_num = channel_num;
    g_pcm_len = frame_len;          // Initialize output pcm_len
    g_cvsd_flag = bt_sco_codec_is_cvsd();

    speech_set_f32(g_in_pcm_buf, 0, g_frame_len * g_channel_num);
    speech_set_int16(g_out_pcm_buf, 0, g_frame_len * g_channel_num);
    speech_set_f32(g_in_fb_buf, 0, g_frame_len);
    g_cp_state = CP_SCO_STATE_IDLE;

    TRACE(2,"[%s] status = %d", __func__, g_cp_state);

    return 0;

}

int sco_cp_deinit(void)
{
    TRACE(1,"[%s] ...", __func__);

    cp_accel_close(CP_TASK_SCO);

    g_cp_state = CP_SCO_STATE_NONE;

    return 0;
}
#endif
