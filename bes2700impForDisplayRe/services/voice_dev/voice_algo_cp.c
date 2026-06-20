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
 *  1.  Enable VOICE_ALGO_CP_ACCEL ?= 1 to enable dual core for voice algo
 *  2.  Enable VOICE_ALGO_TRACE_CP_ACCEL ?= 1 to see debug log.
 *  3.  open CP when music start; close CP when music stop
 *
 * NOTE:
 *  1.  spx fft and hw fft will share buffer, so just one core can use these fft.
 *  2.  audio_dump_add_channel_data function can not work correctly in CP core, because
 *      audio_dump_add_channel_data is possible called after audio_dump_run();
 *  3.  AP and CP just can use 85%
 *
 * TODO:
 *  1.  FFT, RAM, CODE overlay
 **/
#if defined(VOICE_ALGO_CP_ACCEL)
#include "cp_accel.h"
#include "hal_location.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "speech_cfg.h"
#include "norflash_api.h"
#include "voice_dev.h"
#include "voice_dev_defs.h"

#define SAMPLE_BYTES            SAMPLE_BITS_TO_BYTES(SAMPLE_BITS)
#define NOISE_EST_RES_NUM       (2)

enum CP_VOICE_ALGO_STATE_T {
    CP_VOICE_ALGO_STATE_NONE = 0,
    CP_VOICE_ALGO_STATE_IDLE,
    CP_VOICE_ALGO_STATE_WORKING,
};

static CP_DATA_LOC enum CP_VOICE_ALGO_STATE_T g_cp_state = CP_VOICE_ALGO_STATE_NONE;

// malloc data from pool in init function
static CP_BSS_LOC uint8_t *g_in_buf_ptr[VOICE_DEV_OUT_BUF_PTR_NUM];     // Pointer to g_mic0_buf for voice_dev algo
static CP_BSS_LOC uint8_t g_mic0_buf[FRAME_SIZE_MAX];
static CP_BSS_LOC uint8_t g_ref0_buf[FRAME_SIZE_MAX];
static CP_BSS_LOC uint8_t g_noise_est_buf[NOISE_EST_RES_NUM * sizeof(float)];
static CP_BSS_LOC uint8_t g_out_mic0_buf[FRAME_SIZE_MAX];

static CP_BSS_LOC uint32_t g_frame_size;
static CP_BSS_LOC uint32_t g_mic_channel_num;
static CP_BSS_LOC uint32_t g_ref_channel_num;

static uint32_t g_require_cnt = 0;
static uint32_t g_run_cnt = 0;

extern int32_t voice_algo_process_impl(uint8_t *buf_ptr[], uint32_t frame_len);

int32_t voice_algo_cp_process(uint8_t *buf_ptr[], uint32_t frame_len)
{
    uint32_t wait_cnt = 0;

    ASSERT(g_frame_size / SAMPLE_BYTES == frame_len, "[%s] g_frame_len(%d) / SAMPLE_BYTES != frame_len(%d)", __func__, g_frame_size, frame_len);

    // Check CP has new data to get and can get data from buffer
#if defined(VOICE_ALGO_TRACE_CP_ACCEL)
    TRACE(4, "[%s] g_require_cnt: %d, status: %d, frame_len: %d", __func__, g_require_cnt, g_cp_state, frame_len);
#endif

    if (g_cp_state == CP_VOICE_ALGO_STATE_NONE) {
        voice_algo_process_impl(buf_ptr, frame_len);
    }

    while (g_cp_state == CP_VOICE_ALGO_STATE_WORKING) {
        hal_sys_timer_delay_us(10);

        if (wait_cnt++ > 300000) {      // 3s
            ASSERT(0, "cp is hung %d", g_cp_state);
        }
    }

    if (g_cp_state == CP_VOICE_ALGO_STATE_IDLE) {
        speech_copy_uint8(g_mic0_buf, buf_ptr[VOICE_DEV_BUF_INDEX_MIC0], g_frame_size);
        speech_copy_uint8(g_ref0_buf, buf_ptr[VOICE_DEV_BUF_INDEX_REF0], g_frame_size);

        speech_copy_uint8(buf_ptr[VOICE_DEV_BUF_INDEX_MIC0], g_out_mic0_buf, g_frame_size);
        speech_copy_uint8(buf_ptr[VOICE_DEV_BUF_INDEX_NOISE_EST], g_noise_est_buf, sizeof(g_noise_est_buf));

        g_require_cnt++;
        g_cp_state = CP_VOICE_ALGO_STATE_WORKING;
        cp_accel_send_event_mcu2cp(CP_BUILD_ID(CP_TASK_VOICE_ALGO, CP_EVENT_VOICE_ALGO_PROCESSING));
    } else {
        // Check abs(g_require_cnt - g_run_cnt) > threshold, reset or assert

        TRACE(2, "[%s] ERROR: status = %d", __func__, g_cp_state);
    }

    return 0;
}

CP_TEXT_SRAM_LOC
static uint32_t _algo_cp_main(uint8_t event)
{
#if defined(VOICE_ALGO_TRACE_CP_ACCEL)
    TRACE(2, "[%s] g_run_cnt: %d", __func__, g_run_cnt);
#endif

    voice_algo_process_impl((uint8_t **)g_in_buf_ptr, g_frame_size / SAMPLE_BYTES);
    speech_copy_uint8(g_out_mic0_buf, g_mic0_buf, g_frame_size);

    // set status
    g_run_cnt++;
    g_cp_state = CP_VOICE_ALGO_STATE_IDLE;

#if defined(VOICE_ALGO_TRACE_CP_ACCEL)
    TRACE(1, "[%s] CP_VOICE_ALGO_STATE_IDLE", __func__);
#endif

    // UNLOCK BUFFER

    return 0;
}

static const struct cp_task_desc task_desc_voice_algo = {CP_ACCEL_STATE_CLOSED, _algo_cp_main, NULL, NULL, NULL};

int32_t voice_algo_cp_open(uint32_t frame_len, uint32_t mic_channel_num, uint32_t ref_channel_num)
{
    TRACE(4, "[%s] frame_len: %d, mic_channel_num: %d, ref_channel_num: %d", __func__, frame_len, mic_channel_num, ref_channel_num);
    ASSERT(frame_len <= FRAME_LEN_MAX, "[%s] frame_len(%d) > FRAME_LEN_MAX", __func__, frame_len);

    g_require_cnt = 0;
    g_run_cnt = 0;

    norflash_api_flush_disable(NORFLASH_API_USER_CP,(uint32_t)cp_accel_init_done, false);
    cp_accel_open(CP_TASK_VOICE_ALGO, &task_desc_voice_algo);

    uint32_t cnt=0;
    while(cp_accel_init_done() == false) {
        hal_sys_timer_delay_us(100);
        
        cnt++;
        if (cnt % 10 == 0) {
            if (cnt == 10 * 200) {     // 200ms
                ASSERT(0, "[%s] ERROR: Can not init cp!!!", __func__);
            } else {
                TRACE(2, "[%s] Wait CP init done...%d(ms)", __func__, cnt/10);
            }
        }
    }
    norflash_api_flush_enable(NORFLASH_API_USER_CP);
#if 0
    speech_heap_cp_start();
    speech_heap_add_block(g_cp_heap_buf, sizeof(g_cp_heap_buf));
    speech_heap_cp_end();
#endif

    g_frame_size = frame_len * SAMPLE_BYTES;
    g_mic_channel_num = mic_channel_num;
    g_ref_channel_num = ref_channel_num;

    speech_set_uint8(g_mic0_buf, 0, sizeof(g_mic0_buf));
    speech_set_uint8(g_ref0_buf, 0, sizeof(g_ref0_buf));
    speech_set_uint8(g_noise_est_buf, 0, sizeof(g_noise_est_buf));
    speech_set_uint8(g_out_mic0_buf, 0, sizeof(g_out_mic0_buf));

    for (uint32_t i=0; i <VOICE_DEV_OUT_BUF_PTR_NUM; i++) {
        g_in_buf_ptr[i] = NULL;
    }

    g_in_buf_ptr[VOICE_DEV_BUF_INDEX_MIC0] = (uint8_t *)g_mic0_buf;
    g_in_buf_ptr[VOICE_DEV_BUF_INDEX_REF0] = (uint8_t *)g_ref0_buf;
    g_in_buf_ptr[VOICE_DEV_BUF_INDEX_NOISE_EST] = (uint8_t *)g_noise_est_buf;

    g_cp_state = CP_VOICE_ALGO_STATE_IDLE;

    TRACE(2, "[%s] status = %d", __func__, g_cp_state);

    return 0;

}

int32_t voice_algo_cp_close(void)
{
    TRACE(1, "[%s] ...", __func__);

    cp_accel_close(CP_TASK_VOICE_ALGO);

    g_cp_state = CP_VOICE_ALGO_STATE_NONE;

    return 0;
}
#endif
