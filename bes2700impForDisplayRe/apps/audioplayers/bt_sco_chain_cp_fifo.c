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
/**
 * Usage:
 *  1.  Enable SCO_CP_ACCEL ?= 1 to enable dual core in sco
 *  2.  Enable SCO_TRACE_CP_ACCEL ?= 1 to see debug log.
 *  3.  Change channel number if the algo(run in cp) input is more than one channel: sco_cp_init(speech_tx_frame_len, 1);
 *  4.  The code between SCO_CP_ACCEL_ALGO_START(); and SCO_CP_ACCEL_ALGO_END(); will run in CP core.
 *  5.  These algorithms will work in AP. Need to move this algorithms from overlay to fast ram.
 *  6.  Enable SCO_CP_ACCEL_FIFO ?= 1 to use bt_sco_chain_cp_fifo.c
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
#include "speech_cfg.h"
#include "bt_sco_chain_cp.h"
#include "cp_accel.h"
#include "hal_location.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "cmsis_os.h"
#include "math.h"
#include "norflash_api.h"
#include "kfifo.h"

#define SCO_CP_FIFO_FRAME_NUM           (3)

enum CP_SCO_STATE_T {
    CP_SCO_STATE_NONE = 0,
    CP_SCO_STATE_IDLE,
    CP_SCO_STATE_WORKING,
};

typedef short SPEECH_PCM_T;

#undef CP_DATA_LOC
#undef CP_BSS_LOC

#define RAM_SCO_LOC_A(n, l)               __attribute__((section(#n "." #l)))
#define RAM_SCO_LOC(n, l)                 RAM_SCO_LOC_A(n, l)

#define CP_DATA_LOC                       RAM_SCO_LOC(.overlay_sco_cache, __LINE__)
#define CP_BSS_LOC                        RAM_SCO_LOC(.overlay_sco_cache, __LINE__)

typedef struct {
    struct kfifo pcm_fifo;
    uint8_t *pcm_fifo_mem;
    struct kfifo ref_fifo;
    uint8_t *ref_fifo_mem;
    struct kfifo fb_fifo;
    uint8_t *fb_fifo_mem;
    struct kfifo out_fifo;
    uint8_t *out_fifo_mem;

    uint8_t algo_frame_num;
    int algo_frame_len;
} capture_ctx_t;

static CP_BSS_LOC capture_ctx_t g_capture_ctx;

static int32_t _nextpow2(int32_t N)
{
    int32_t i = 1;
    while (1) {
        i <<= 1;
        if (i >= N)
            return i;
    }
}

static CP_DATA_LOC enum CP_SCO_STATE_T g_cp_state = CP_SCO_STATE_NONE;

static short *g_in_pcm_buf = NULL;
static short *g_in_ref_buf = NULL;
static short *g_in_fb_buf = NULL;

static CP_BSS_LOC int g_frame_len;
static CP_BSS_LOC int g_channel_num;

static CP_BSS_LOC sco_status_t g_sco_status_in;
static CP_BSS_LOC sco_status_t g_sco_status_out;

static int g_require_cnt = 0;
static int g_run_cnt = 0;

int sco_cp_process(short *pcm_buf, short *ref_buf, short *fb_buf, int *_pcm_len, sco_status_t *status)
{
    int32_t pcm_len = *_pcm_len;

    // TRACE(0, "[%s] pcm_len = %d, g_frame_len = %d", __FUNCTION__, pcm_len, g_frame_len);

#if defined(SCO_TRACE_CP_ACCEL)
    TRACE(4,"[%s] g_require_cnt: %d, status: %d, pcm_len: %d", __func__, g_require_cnt, g_cp_state, pcm_len);
#endif

    capture_ctx_t *ctx = &g_capture_ctx;

    if (kfifo_get_free_space(&ctx->pcm_fifo) >= pcm_len * sizeof(SPEECH_PCM_T)) {
        kfifo_put(&ctx->pcm_fifo, (uint8_t *)pcm_buf, pcm_len * sizeof(SPEECH_PCM_T));
        if (ref_buf){
            kfifo_put(&ctx->ref_fifo, (uint8_t *)ref_buf, pcm_len * sizeof(SPEECH_PCM_T));
        }
        if (fb_buf) {
            kfifo_put(&ctx->fb_fifo, (uint8_t *)fb_buf, pcm_len * sizeof(SPEECH_PCM_T));
        }
    } else {
        TRACE(0, "[%s] Input buffer is overflow", __func__);
    }
    memcpy(&g_sco_status_in, status, sizeof(sco_status_t));

    if (kfifo_len(&ctx->out_fifo) >= pcm_len / g_channel_num * sizeof(SPEECH_PCM_T)) {
        kfifo_get(&ctx->out_fifo, (uint8_t *)pcm_buf, pcm_len / g_channel_num * sizeof(SPEECH_PCM_T));
    } else {
        TRACE(0, "[%s] Output buffer is underflow", __func__);
        memset(pcm_buf, 0, pcm_len / g_channel_num * sizeof(SPEECH_PCM_T));
    }
    memcpy(status, &g_sco_status_out, sizeof(sco_status_t));
    *_pcm_len = pcm_len / g_channel_num;

    g_require_cnt++;
    cp_accel_send_event_mcu2cp(CP_BUILD_ID(CP_TASK_SCO, CP_EVENT_SCO_PROCESSING));

    return 0;
}

CP_TEXT_SRAM_LOC
static unsigned int sco_cp_main(uint8_t event)
{
    // TRACE(0, "[%s] g_channel_num = %d, ctx->algo_frame_len = %d", __FUNCTION__, g_channel_num, g_capture_ctx.algo_frame_len);

#if defined(SCO_TRACE_CP_ACCEL)
    TRACE(2,"[%s] g_run_cnt: %d", __func__, g_run_cnt);
#endif

    capture_ctx_t *ctx = &g_capture_ctx;
    int algo_pcm_len = ctx->algo_frame_len * g_channel_num * sizeof(SPEECH_PCM_T);
    int pcm_len = ctx->algo_frame_len * g_channel_num;

    while (kfifo_len(&ctx->pcm_fifo) >= algo_pcm_len)
    {
        kfifo_get(&ctx->pcm_fifo, (uint8_t *)g_in_pcm_buf, algo_pcm_len);
        kfifo_get(&ctx->ref_fifo, (uint8_t *)g_in_ref_buf, algo_pcm_len / g_channel_num);
        kfifo_get(&ctx->fb_fifo,  (uint8_t *)g_in_fb_buf,  algo_pcm_len / g_channel_num);
        sco_cp_algo(g_in_pcm_buf, g_in_ref_buf, g_in_fb_buf, &pcm_len, &g_sco_status_in);
        if (kfifo_get_free_space(&ctx->out_fifo) >= pcm_len * sizeof(SPEECH_PCM_T)) {
            kfifo_put(&ctx->out_fifo, (uint8_t *)g_in_pcm_buf, pcm_len * sizeof(SPEECH_PCM_T));
        }
        else {
            TRACE(0, "[%s] Output buffer is overflow when algo", __func__);
        }
    }

    memcpy(&g_sco_status_out, &g_sco_status_in, sizeof(sco_status_t));

    g_run_cnt++;

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

    g_frame_len = frame_len;
    g_channel_num = channel_num;

    capture_ctx_t *ctx = &g_capture_ctx;
    ctx->algo_frame_len = frame_len;

    uint32_t pcm_fifo_len = _nextpow2(frame_len * SCO_CP_FIFO_FRAME_NUM * channel_num * sizeof(SPEECH_PCM_T));
    uint32_t ref_fifo_len = _nextpow2(frame_len * SCO_CP_FIFO_FRAME_NUM * sizeof(SPEECH_PCM_T));
    uint32_t fb_fifo_len  = _nextpow2(frame_len * SCO_CP_FIFO_FRAME_NUM * sizeof(SPEECH_PCM_T));
    uint32_t out_fifo_len = _nextpow2(frame_len * SCO_CP_FIFO_FRAME_NUM * sizeof(SPEECH_PCM_T));

    ctx->pcm_fifo_mem = med_calloc(pcm_fifo_len, 1);
    ctx->ref_fifo_mem = med_calloc(ref_fifo_len, 1);
    ctx->fb_fifo_mem  = med_calloc(fb_fifo_len, 1);
    ctx->out_fifo_mem = med_calloc(out_fifo_len, 1);

    kfifo_init(&ctx->pcm_fifo, ctx->pcm_fifo_mem, pcm_fifo_len);
    kfifo_init(&ctx->ref_fifo, ctx->ref_fifo_mem, ref_fifo_len);
    kfifo_init(&ctx->fb_fifo,  ctx->fb_fifo_mem,  fb_fifo_len);
    kfifo_init(&ctx->out_fifo, ctx->out_fifo_mem, out_fifo_len);

    g_in_pcm_buf = med_malloc(ctx->algo_frame_len * g_channel_num * sizeof(SPEECH_PCM_T));
    g_in_ref_buf = med_malloc(ctx->algo_frame_len * sizeof(SPEECH_PCM_T));
    g_in_fb_buf  = med_malloc(ctx->algo_frame_len * sizeof(SPEECH_PCM_T));

    memset(g_in_pcm_buf, 0, ctx->algo_frame_len * g_channel_num * sizeof(SPEECH_PCM_T));

    if (kfifo_get_free_space(&ctx->out_fifo) >= ctx->algo_frame_len * sizeof(SPEECH_PCM_T)) {
        for (uint32_t i=0; i<SCO_CP_FIFO_FRAME_NUM-1; i++){
            kfifo_put(&ctx->out_fifo, (uint8_t *)g_in_pcm_buf, ctx->algo_frame_len * sizeof(SPEECH_PCM_T));
        }
    }

    g_cp_state = CP_SCO_STATE_IDLE;

    TRACE(2,"[%s] status = %d", __func__, g_cp_state);

    return 0;

}

int sco_cp_deinit(void)
{
    TRACE(1,"[%s] ...", __func__);
    capture_ctx_t *ctx = &g_capture_ctx;

    med_free(g_in_fb_buf);
    med_free(g_in_ref_buf);
    med_free(g_in_pcm_buf);

    med_free(ctx->out_fifo_mem);
    med_free(ctx->fb_fifo_mem);
    med_free(ctx->ref_fifo_mem);
    med_free(ctx->pcm_fifo_mem);

    cp_accel_close(CP_TASK_SCO);

    g_cp_state = CP_SCO_STATE_NONE;

    return 0;
}
#endif
