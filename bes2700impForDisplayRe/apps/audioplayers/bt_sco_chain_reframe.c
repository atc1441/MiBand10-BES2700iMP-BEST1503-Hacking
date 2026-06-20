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
#include "bt_sco_chain_reframe.h"
#include "speech_memory.h"
#include "speech_utils.h"
#include "hal_trace.h"
#include "audio_dump.h"
#include "speech_cfg.h"
#include "cmsis_os.h"
#if defined(SCO_CP_ACCEL)
#include "hal_location.h"
#include "bt_sco_chain_cp_thirdparty.h"
#endif
#include "bt_sco_chain_thirdparty_def.h"
// TODO: #include "thirdparty_algo.h"

// #define _REFRAME_DEBUG
// #define _CALCULATE_MIPS
// #define ASSERT_QUEUE_STATUS
// #define BT_SCO_CHAIN_REFRAME_AUDIO_DUMP

#define _MIPS_LOG(str, ...)        TR_CRITICAL(TR_MOD(AUD), str, ##__VA_ARGS__)
#define BT_SCO_CHAIN_LOG_I(str, ...)        TR_INFO(TR_MOD(AUD), "[BT_SCO_CHAIN]" str, ##__VA_ARGS__)
#if defined(_REFRAME_DEBUG)
#define BT_SCO_CHAIN_LOG_D(str, ...)        TR_DEBUG(TR_MOD(AUD), "[BT_SCO_CHAIN]" str, ##__VA_ARGS__)
static uint32_t g_debug_index = 0;
#else
#define BT_SCO_CHAIN_LOG_D(str, ...)
#endif

#define SAMPLE_RATE             (16000)
#if defined(SPEECH_TX_24BIT)
#define SAMPLE_BYTES            (4)
#else
#define SAMPLE_BYTES            (2)
#endif

#define PROCESS_FRAME_LEN       (THIRDPARTY_FRAME_LEN)
#define QUEUE_BUF_PCM_NUM       (PROCESS_FRAME_LEN * 4)

#if defined(LOW_DELAY_SCO)
#define IO_FRAME_LEN            (120)   // 7.5ms
#else
#define IO_FRAME_LEN            (240)   // 15ms
#endif

// Reframe delay: 47.5ms
// Trade off between CPU Usage and delay.
// + IO_FRAME_LEN: CPU Usage: 96%
// If remove (+ IO_FRAME_LEN), CPU Usage: 86%
#define PRE_CACHE_OUT_PCM       (PROCESS_FRAME_LEN * 2)

#if SAMPLE_BYTES == 2
typedef short   _PCM_T;
#define speech_set_pcm(a, b, c)         speech_set_int16(a, b, c)
#define speech_copy_pcm(a, b, c)        speech_copy_int16(a, b, c)
#else
typedef int     _PCM_T;
#define speech_set_pcm(a, b, c)         speech_set_int32(a, b, c)
#define speech_copy_pcm(a, b, c)        speech_copy_int32(a, b, c)
#endif

static uint32_t g_in_queue_len = 0;
static uint32_t g_out_queue_len = 0;

// TODO: input and output share buffer. 
static _PCM_T *g_in_cache_buf[REFRAME_CHANNEL_QTY] = {NULL};
static _PCM_T *g_in_queue_buf[REFRAME_CHANNEL_QTY] = {NULL};
static _PCM_T *g_process_buf[REFRAME_CHANNEL_QTY] = {NULL};
static _PCM_T *g_process_tmp_buf[REFRAME_CHANNEL_QTY] = {NULL};
static _PCM_T *g_process_out_buf = NULL;
static _PCM_T *g_out_queue_buf = NULL;

#if defined(_CALCULATE_MIPS)
static void calculate_mips_create(void);
static void calculate_mips_destroy(void);
static void calculate_mips_run_start(bool cp);
static void calculate_mips_run_end(bool cp);
#endif

#ifdef BT_SCO_CHAIN_REFRAME_AUDIO_DUMP
typedef short		_DUMP_PCM_T;
static _DUMP_PCM_T audio_dump_buf[PROCESS_FRAME_LEN];

static void dump_pcm_data(int channel_id, _PCM_T *pcm_buf, int frame_len)
{
    for (uint32_t i=0; i<frame_len; i++) {
        audio_dump_buf[i] = pcm_buf[i];
    }
    audio_dump_add_channel_data(channel_id, audio_dump_buf, frame_len);
}
#endif

// Thread
#define REFRAME_THREAD_STACK_SIZE          (1024 * 2)
static osThreadId reframe_thread_tid;
static void _reframe_thread(void const *argument);
osThreadDef(_reframe_thread, osPriorityAboveNormal, 1, REFRAME_THREAD_STACK_SIZE, "speech_reframe");

// Signal
#define REFRAME_SIGNAL_WRITE   1

// Lock
osMutexId _reframe_mutex_id = NULL;
osMutexDef(_reframe_mutex);

static void _reframe_create_lock(void)
{
    if (_reframe_mutex_id == NULL) {
        _reframe_mutex_id = osMutexCreate((osMutex(_reframe_mutex)));
    }
}

static void _reframe_lock(void)
{
    osMutexWait(_reframe_mutex_id, osWaitForever);
}

static void _reframe_unlock(void)
{
    osMutexRelease(_reframe_mutex_id);
}

static uint32_t POSSIBLY_UNUSED _get_in_pcm_data_len(void)
{
    return g_in_queue_len;
}

static uint32_t POSSIBLY_UNUSED _get_out_pcm_data_len(void)
{
    return g_out_queue_len;
}

static int32_t _write_input_queue(_PCM_T *pcm_buf[], uint32_t frame_len)
{
#if defined(ASSERT_QUEUE_STATUS)
    ASSERT(g_in_queue_len + frame_len <= QUEUE_BUF_PCM_NUM, "[%s] queue overflow!", __func__);
#else
    if (g_in_queue_len + frame_len > QUEUE_BUF_PCM_NUM) {
        TRACE(0, "[%s] WARNING: queue overflow!", __func__);
        return -1;
    }
#endif

    _reframe_lock();
    for (uint32_t ch = 0; ch < REFRAME_CHANNEL_QTY; ch++) {
        speech_copy_pcm(g_in_queue_buf[ch] + g_in_queue_len, pcm_buf[ch], frame_len);
    }
    g_in_queue_len += frame_len;
    _reframe_unlock();

    BT_SCO_CHAIN_LOG_D("[%s] Size: %d", __func__, g_in_queue_len);

    return 0;
}

static int32_t _read_input_queue(_PCM_T *pcm_buf[], uint32_t frame_len)
{
    int32_t ret = 0;

    _reframe_lock();
    if (g_in_queue_len >= frame_len) {
        for (uint32_t ch = 0; ch < REFRAME_CHANNEL_QTY; ch++) {
            speech_copy_pcm(pcm_buf[ch], g_in_queue_buf[ch], frame_len);
            speech_copy_pcm(g_in_queue_buf[ch], g_in_queue_buf[ch] + frame_len, g_in_queue_len - frame_len);
        }
        g_in_queue_len -= frame_len;
    } else {
        for (uint32_t ch = 0; ch < REFRAME_CHANNEL_QTY; ch++) {
            speech_set_pcm(pcm_buf[ch], 0, frame_len);
        }
        ret = -1;
    }
    _reframe_unlock();

    BT_SCO_CHAIN_LOG_D("[%s] Size: %d", __func__, g_in_queue_len);

    return ret;
}

static int32_t _write_output_queue(_PCM_T *pcm_buf, uint32_t frame_len)
{
#if defined(ASSERT_QUEUE_STATUS)
    ASSERT(g_out_queue_len + frame_len <= QUEUE_BUF_PCM_NUM, "[%s] queue overflow!", __func__);
#else
    if (g_out_queue_len + frame_len > QUEUE_BUF_PCM_NUM) {
        TRACE(0, "[%s] WARNING: queue overflow!", __func__);
        return -1;
    }
#endif

    _reframe_lock();
    speech_copy_pcm(g_out_queue_buf + g_out_queue_len, pcm_buf, frame_len);
    g_out_queue_len += frame_len;
    _reframe_unlock();

    BT_SCO_CHAIN_LOG_D("[%s] Size: %d", __func__, g_out_queue_len);

    return 0;
}

static int32_t _read_output_queue(_PCM_T *pcm_buf, uint32_t frame_len)
{
#if defined(ASSERT_QUEUE_STATUS)
    ASSERT(g_out_queue_len >= frame_len, "[%s] queue underflow! Try to increase PRE_CACHE_OUT_PCM(+ IO_FRAME_LEN)", __func__);
#else
    if (g_out_queue_len < frame_len) {
        TRACE(0, "[%s] WARNING: queue underflow! Try to increase PRE_CACHE_OUT_PCM(+ IO_FRAME_LEN)", __func__);
        speech_set_pcm(pcm_buf, 0, frame_len);
        return -1;
    }
#endif

    _reframe_lock();
    speech_copy_pcm(pcm_buf, g_out_queue_buf, frame_len);
    speech_copy_pcm(g_out_queue_buf, g_out_queue_buf + frame_len, g_out_queue_len - frame_len);
    g_out_queue_len -= frame_len;
    _reframe_unlock();

    BT_SCO_CHAIN_LOG_D("[%s] Size: %d", __func__, g_out_queue_len);

    return 0;
}

#if defined(SCO_CP_ACCEL)
CP_TEXT_SRAM_LOC
int32_t sco_cp_algo(_PCM_T *pcm_buf[], _PCM_T *out_buf, int frame_len)
{
    ASSERT(frame_len == PROCESS_FRAME_LEN, "[%s] frame_len(%d) != PROCESS_FRAME_LEN", __func__, frame_len);

    // calculate_mips_run_start(true);

#if 1
    // TODO: Call thirdparty algo process function which run in CP Core
#else
    for (uint32_t i=0; i<frame_len; i++) {
        out_buf[i] = pcm_buf[REFRAME_CHANNEL_FB][i];
    }
#endif

    // calculate_mips_run_end(true);
    
    return 0;
}
#endif

static void _reframe_thread(void const *argument)
{
    osEvent evt;
    uint32_t signals = 0;

    while (1) {
        evt = osSignalWait(0x0, osWaitForever);
        signals = evt.value.signals;
        
        // BT_SCO_CHAIN_LOG_D("[%s] status = %x, signals = %d", __func__, evt.status, evt.value.signals);

        if (evt.status == osEventSignal) {
            // BT_SCO_CHAIN_LOG_D("signal = %ld", signals);
            if (signals & (1 << REFRAME_SIGNAL_WRITE)) {
                while (1) {
                    if (!_read_input_queue(g_process_buf, PROCESS_FRAME_LEN) &&
                        (_get_out_pcm_data_len() + PROCESS_FRAME_LEN <=  QUEUE_BUF_PCM_NUM)) {
                        BT_SCO_CHAIN_LOG_D("RUN Start ...");
#if defined(_CALCULATE_MIPS)
                        calculate_mips_run_start(false);
#endif
#ifdef BT_SCO_CHAIN_REFRAME_AUDIO_DUMP
                        uint32_t dump_ch = 0;
                        audio_dump_clear_up();

                        dump_pcm_data(dump_ch++, g_process_buf[REFRAME_CHANNEL_VP], PROCESS_FRAME_LEN);
                        dump_pcm_data(dump_ch++, g_process_buf[REFRAME_CHANNEL_FF], PROCESS_FRAME_LEN);
                        dump_pcm_data(dump_ch++, g_process_buf[REFRAME_CHANNEL_FB], PROCESS_FRAME_LEN);
                        dump_pcm_data(dump_ch++, g_process_buf[REFRAME_CHANNEL_REF], PROCESS_FRAME_LEN);
                        dump_pcm_data(dump_ch++, g_process_buf[REFRAME_CHANNEL_BS], PROCESS_FRAME_LEN);
#endif

                        //---------- Process Start ----------
#if 0
                        // TODO: Call thirdparty algo process function which run in Main Core

#if defined(SCO_CP_ACCEL)
                        sco_cp_process(g_process_tmp_buf, g_process_out_buf, PROCESS_FRAME_LEN);
#else
                        // TODO: Call thirdparty algo process function which run in CP Core
#endif
#else
                        for (uint32_t i = 0; i < PROCESS_FRAME_LEN; i++) {
                            g_process_out_buf[i] = g_process_buf[REFRAME_CHANNEL_VP][i];
                        }
#endif
                        //---------- Process End ----------

#ifdef BT_SCO_CHAIN_REFRAME_AUDIO_DUMP
                        dump_pcm_data(dump_ch++, g_process_out_buf, PROCESS_FRAME_LEN);
                        audio_dump_run();
#endif
#if defined(_CALCULATE_MIPS)
                        calculate_mips_run_end(false);
#endif
                        _write_output_queue(g_process_out_buf, PROCESS_FRAME_LEN);
                    } else {
                        break;
                    }
                }
                BT_SCO_CHAIN_LOG_D("RUN End ...");
            }
        } else {
            ASSERT(0, "[%s] ERROR: evt.status = %d", __func__, evt.status);
            continue;
        }
    }
}

int32_t bt_sco_chain_reframe_open(uint32_t tx_sample_rate, uint32_t tx_frame_len)
{
    ASSERT(tx_sample_rate == SAMPLE_RATE, "[%s] tx_sample_rate(%d) != SAMPLE_RATE", __func__, tx_sample_rate);
    ASSERT(tx_frame_len == IO_FRAME_LEN, "[%s] tx_frame_len(%d) != IO_FRAME_LEN", __func__, tx_frame_len);

    BT_SCO_CHAIN_LOG_I("[%s] ...", __func__);

#if defined(SCO_CP_ACCEL)
    sco_cp_init(PROCESS_FRAME_LEN, REFRAME_CHANNEL_QTY);
#endif

    g_in_queue_len = 0;
    g_out_queue_len = 0;

    for (uint32_t ch = 0; ch < REFRAME_CHANNEL_QTY; ch++) {
        g_in_cache_buf[ch]  = (_PCM_T *)speech_calloc(IO_FRAME_LEN, sizeof(_PCM_T));
        g_in_queue_buf[ch]  = (_PCM_T *)speech_calloc(QUEUE_BUF_PCM_NUM, sizeof(_PCM_T));
        g_process_buf[ch]   = (_PCM_T *)speech_calloc(PROCESS_FRAME_LEN, sizeof(_PCM_T));
        g_process_tmp_buf[ch]   = (_PCM_T *)speech_calloc(THIRDPARTY_FFT_LEN, sizeof(_PCM_T));
    }
    g_process_out_buf = (_PCM_T *)speech_calloc(PROCESS_FRAME_LEN, sizeof(_PCM_T));
    g_out_queue_buf = (_PCM_T *)speech_calloc(QUEUE_BUF_PCM_NUM, sizeof(_PCM_T));

    _write_output_queue(g_process_out_buf, PRE_CACHE_OUT_PCM);

    _reframe_create_lock();
    reframe_thread_tid = osThreadCreate(osThread(_reframe_thread), NULL);
    osSignalSet(reframe_thread_tid, 0x0);

    // TODO: Call thridpart algo init function

#if defined(_REFRAME_DEBUG)
    g_debug_index = 0;
#endif

#ifdef BT_SCO_CHAIN_REFRAME_AUDIO_DUMP
    audio_dump_init(PROCESS_FRAME_LEN, sizeof(_DUMP_PCM_T), 6);
#endif

#if defined(_CALCULATE_MIPS)
    calculate_mips_create();
#endif

    return 0;
}

int32_t bt_sco_chain_reframe_close(void)
{
    BT_SCO_CHAIN_LOG_I("[%s] ...", __func__);

#if defined(_CALCULATE_MIPS)
    calculate_mips_destroy();
#endif

    // TODO: Call thridpart algo deinit function

    osThreadTerminate(reframe_thread_tid);

    for (uint32_t ch = 0; ch < REFRAME_CHANNEL_QTY; ch++) {
        speech_free(g_in_cache_buf[ch]);
        speech_free(g_in_queue_buf[ch]);
        speech_free(g_process_buf[ch]);
        speech_free(g_process_tmp_buf[ch]);
    }
    speech_free(g_process_out_buf);
    speech_free(g_out_queue_buf);

#if defined(SCO_CP_ACCEL)
    sco_cp_deinit();
#endif

    return 0;
}

int32_t bt_sco_chain_reframe_cache_tx_pcm(void *pcm_buf, uint32_t frame_len, reframe_channel_t ch)
{
    ASSERT(frame_len == IO_FRAME_LEN, "[%s] frame_len(%d) != IO_FRAME_LEN", __func__, frame_len);

    speech_copy_pcm(g_in_cache_buf[ch], (_PCM_T *)pcm_buf, frame_len);

    return 0;
}

int32_t bt_sco_chain_reframe_tx_process(void *in_buf[], uint32_t frame_len, void *out_buf)
{
    ASSERT(frame_len == IO_FRAME_LEN, "[%s] frame_len(%d) != IO_FRAME_LEN", __func__, frame_len);
    ASSERT(out_buf != NULL, "[%s] out_buf == NULL", __func__);

#if defined(_REFRAME_DEBUG)
    BT_SCO_CHAIN_LOG_D("[REFRAME] %d: frame_len = %d", g_debug_index++, frame_len);
#endif

#if 1
    if (in_buf == NULL) {
        _write_input_queue(g_in_cache_buf, frame_len);
    } else {
        _write_input_queue((_PCM_T **)in_buf, frame_len);
    }

    _read_output_queue((_PCM_T *)out_buf, frame_len);
    osSignalSet(reframe_thread_tid, 0x01 << REFRAME_SIGNAL_WRITE);
#else
    for (uint32_t i = 0; i < frame_len; i++) {
        ((_PCM_T *)out_buf)[i] = g_in_cache_buf[REFRAME_CHANNEL_VP][i];
    } 
#endif

    return 0;
}

#if defined(_CALCULATE_MIPS)
#include "hal_timer.h"

static uint32_t create_flag = 0;
static uint32_t cpu_freq = 0;
static uint32_t start_ticks = 0;
static uint32_t start_ticks_cp = 0;
static uint32_t end_ticks = 0;
static uint32_t used_mips = 0;
static uint32_t mips_log_cnt = 0;

#define MIPS_LOG_PRINT_MS   (2000)

static void calculate_mips_create(void)
{
    ASSERT(create_flag == 0, "[%s] calculate mips has been created", __func__);

    cpu_freq = hal_sys_timer_calc_cpu_freq(100, 0) / 1000000;
    _MIPS_LOG("[%s] sys freq calc : %d M", __func__, cpu_freq);

    mips_log_cnt = 0;

    create_flag = 1; 
}

static void calculate_mips_destroy(void)
{
    create_flag = 0;
}

static void calculate_mips_run_start(bool cp)
{
    if (cp) {
        start_ticks_cp = hal_fast_sys_timer_get();
    } else {
        start_ticks = hal_fast_sys_timer_get();
    }
}

static void calculate_mips_run_end(bool cp)
{
    uint32_t used_us = 0;
    uint32_t period_us = (1000 * PROCESS_FRAME_LEN / 16);

    end_ticks = hal_fast_sys_timer_get();

    if (cp) {
        used_us = FAST_TICKS_TO_US(end_ticks - start_ticks_cp);

        used_mips = used_us * cpu_freq / period_us;

        // TRACE(0, "[%s] CP: Usage: %d / %d (us).", __func__, used_us, period_us);
        if (mips_log_cnt == MIPS_LOG_PRINT_MS / ((PROCESS_FRAME_LEN * 1000) /SAMPLE_RATE)) {
            _MIPS_LOG("[%s] CP: Usage: %d (MIPS).", __func__, used_mips);
            mips_log_cnt = 0;
        }
    } else {
        used_us = FAST_TICKS_TO_US(end_ticks - start_ticks);

        used_mips = used_us * cpu_freq / period_us;

        // TRACE(0, "[%s] Main: Usage: %d / %d (us).", __func__, used_us, period_us);
        if (mips_log_cnt == MIPS_LOG_PRINT_MS / ((PROCESS_FRAME_LEN * 1000) /SAMPLE_RATE) ) {
            _MIPS_LOG("[%s] Main: Usage: %d (MIPS).", __func__, used_mips);
            mips_log_cnt = 0;
        }  
    }

    mips_log_cnt++;
}
#endif
