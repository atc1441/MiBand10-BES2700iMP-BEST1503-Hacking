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
// Standard C Included Files
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "cmsis_os.h"
#include "plat_types.h"
#include "hal_uart.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "cqueue.h"
#include "app_utils.h"
#include "app_audio.h"
#include "app_overlay.h"
#include "bluetooth_bt_api.h"
#include "app_ring_merge.h"
#include "tgt_hardware.h"
#include "bt_sco_chain.h"
#include "iir_resample.h"
#include "hfp_api.h"
#include "audio_prompt_sbc.h"
#include "speech_utils.h"
#include "bt_sco_codec.h"
#include "bt_drv.h"
#include "arm_math_ex.h"

#ifdef IBRT
#include "app_tws_ibrt_conn_api.h"
#endif

#if defined(SPEECH_TX_24BIT)
typedef int     TX_PCM_T;
#else
typedef short   TX_PCM_T;
#endif

#if defined(SPEECH_RX_24BIT)
typedef int     RX_PCM_T;
#else
typedef short   RX_PCM_T;
#endif

extern TX_PCM_T *aec_echo_buf;

// app_bt_stream.cpp::bt_sco_player(), used buffer size
#define APP_BT_STREAM_USE_BUF_SIZE      (1024*2)

static int sco_sample_rate;
static int tx_vqe_sample_rate;
static int rx_vqe_sample_rate;
static int tx_codec_sample_rate;
static int rx_codec_sample_rate;
static int sco_frame_length;
static int tx_vqe_frame_length;
static int rx_vqe_frame_length;
static int tx_codec_frame_length;
static int rx_codec_frame_length;
static int16_t *resample_buf = NULL;
static IirResampleState *uplink_resample_codec2vqe_st = NULL;
static IirResampleState *downlink_resample_sco2vqe_st = NULL;
static IirResampleState *uplink_resample_vqe2sco_st = NULL;
static IirResampleState *downlink_resample_vqe2codec_st = NULL;
static IirResampleState *resample_echo_st = NULL;

static IirResampleState *uplink_resample_vpu2vqe_st = NULL;

static bool speech_inited = false;
static uint32_t g_bypass_tx_algo_sel_ch = 0xFF;

#define VOICEBTPCM_TRACE(s,...)
//TRACE(s, ##__VA_ARGS__)

#ifdef AUDIO_RMS_MONITOR_ENABLE
extern int32_t rms_dac_volume;
extern uint32_t rms_debug_cnt;
#endif

static bt_sco_codec_t *g_bt_sco_codec_ptr = NULL;

extern hfp_sco_codec_t bt_sco_codec_get_type(void);
extern "C" uint8_t is_sco_mode(void);

#if defined(CALL_BYPASS_SLAVE_TX_PROCESS)
#define TX_CHAIN_BYPASS_LOG_THD     (200)   // 3s: 200 * 15ms

static uint32_t g_tx_chain_bypass_log_cnt = 0;
static bool g_tx_chain_bypass_flag = false;

static bool bt_sco_chain_get_bypass_flag(void)
{
    return g_tx_chain_bypass_flag;
}
#endif

int bt_sco_chain_set_master_role(bool is_master)
{
#if defined(CALL_BYPASS_SLAVE_TX_PROCESS)
    TRACE(2, "[%s] is_master: %d", __func__, is_master);

    if (is_sco_mode()) {
        if (is_master) {
            // TODO: This is CALL system frequency.
            app_sysfreq_req(APP_SYSFREQ_USER_BT_SCO, APP_SYSFREQ_USER_BT_SCO_MASTER);
            g_tx_chain_bypass_flag = false;
        } else {
            // TODO: Perhaps you can change APP_SYSFREQ_52M to APP_SYSFREQ_26M.
            g_tx_chain_bypass_flag = true;
            app_sysfreq_req(APP_SYSFREQ_USER_BT_SCO, APP_SYSFREQ_USER_BT_SCO_SLAVE);
        }
    }
#endif

    return 0;
}

extern "C" int32_t bt_sco_chain_bypass_tx_algo(uint32_t sel_ch)
{
    TRACE(2, "[%s] sel_ch: %d", __func__, sel_ch);

    if (sel_ch < SPEECH_CODEC_CAPTURE_CHANNEL_NUM) {
        g_bypass_tx_algo_sel_ch = sel_ch;
        return 0;
    } else {
        TRACE(2, "[%s] WARNING: Failed to set sel_ch: %d ", __func__, SPEECH_CODEC_CAPTURE_CHANNEL_NUM);
        return -1;
    }
}

extern "C" int32_t speech_bone_sensor_resample_process(void *pcm_buf, int pcm_len)
{
    if (uplink_resample_vpu2vqe_st) {
        iir_resample_process(uplink_resample_vpu2vqe_st, (RX_PCM_T *)pcm_buf, (RX_PCM_T *)pcm_buf, pcm_len);
        // pcm_len = pcm_len * 16000 / sco_sample_rate;
    }
    return pcm_len;
}

/*
 * possible downlink resample routine
 * msbc: 16k -> 16k -> 16k (normal)
 *       16k -> 16k -> 48k (PASP)
 *       16k -> 8k -> 8k (fake msbc) // need extra buffer to hold downlink data
 *       16k -> 8k -> 48k (fake msbc + PASP)
 * cvsd: 8k -> 8k -> 8k (normal)
 *       8k -> 16k -> 16k (algo only support 16k)
 *       8k -> 8k -> 48k (PASP)
 *       8k -> 16k -> 48k (algo only support 16k + PASP)
 *
 * resample sco2vqe could be upsample or downsample
 * resample vqe2codec should only be upsample, downsample is wasting mips
 */
int process_downlink_bt_voice_frames(uint8_t *in_buf, uint32_t in_len, uint8_t *out_buf, uint32_t out_len)
{
    // TRACE(3,"[%s] in_len = %d, out_len = %d", __FUNCTION__, in_len, out_len);

    if (speech_inited == false) {
        TRACE(1, "[%s] warnning: speech init not finished", __FUNCTION__);
        memset(out_buf, 0, out_len);
        return 0;
    }

#if !defined(SPEECH_TX_AEC_CODEC_REF)
#if defined(SPEECH_TX_AEC) || defined(SPEECH_TX_AEC2) || defined(SPEECH_TX_AEC3) || defined(SPEECH_TX_AEC2FLOAT) || defined(SPEECH_TX_3MIC_NS) || defined(SPEECH_TX_2MIC_NS4) || defined(SPEECH_TX_2MIC_NS7) || defined(SPEECH_TX_2MIC_NS8) || defined(SPEECH_TX_THIRDPARTY)
    int ref_pcm_len = out_len / sizeof(RX_PCM_T);
    RX_PCM_T *ref_pcm_buf = (RX_PCM_T *)out_buf;

    if (resample_echo_st) {
        iir_resample_process(resample_echo_st, ref_pcm_buf, aec_echo_buf, ref_pcm_len);
        ref_pcm_buf = (RX_PCM_T *)aec_echo_buf;
        ref_pcm_len = ref_pcm_len * tx_codec_sample_rate / rx_codec_sample_rate;
    }

#if defined(SPEECH_RX_24BIT) && !defined(SPEECH_TX_24BIT)
    arm_q23_to_q15(ref_pcm_buf, aec_echo_buf, ref_pcm_len);
#elif !defined(SPEECH_RX_24BIT) && defined(SPEECH_TX_24BIT)
    arm_q15_to_q23(ref_pcm_buf, aec_echo_buf, ref_pcm_len);
#else
    memcpy(aec_echo_buf, ref_pcm_buf, ref_pcm_len * sizeof(RX_PCM_T));
#endif
#endif
#endif // #if !defined(SPEECH_TX_AEC_CODEC_REF)

#if defined(SPEECH_RX_24BIT)
    out_len /= 2;
#endif

    int16_t *pcm_buf = (int16_t *)out_buf;
    int pcm_len = sco_frame_length;

    if (downlink_resample_sco2vqe_st && resample_buf) {
        pcm_buf = resample_buf;
    }

    if (g_bt_sco_codec_ptr) {
        g_bt_sco_codec_ptr->decoder(in_buf, in_len, (uint8_t *)pcm_buf, pcm_len * sizeof(int16_t));
    }

#ifdef AUDIO_RMS_MONITOR_ENABLE
    hfp_sco_codec_t sco_codec_type = bt_sco_codec_get_type();
    if (sco_codec_type == BT_HFP_SCO_CODEC_CVSD)
    {
        rms_debug_cnt ++;
        if(rms_debug_cnt >= 256)
        {
            int16_t* rms_buff = (int16_t*)pcm_buf;
            rms_debug_cnt = 0;
            TRACE(7, "RMS CVSD:%d dbfs %d %d %d %d %d %d", rms_dac_volume, rms_buff[0], rms_buff[1], rms_buff[2], rms_buff[3], rms_buff[4], rms_buff[5]);
        }
    }
    else if (sco_codec_type == BT_HFP_SCO_CODEC_MSBC)
    {
        rms_debug_cnt ++;
        if(rms_debug_cnt >= 256)
        {
            int16_t* rms_buff = (int16_t*)pcm_buf;
            rms_debug_cnt = 0;
            TRACE(7, "RMS MSBC:%d dbfs %d %d %d %d %d %d", rms_dac_volume, rms_buff[0], rms_buff[1], rms_buff[2], rms_buff[3], rms_buff[4], rms_buff[5]);
        }
    }
#endif

    if (downlink_resample_sco2vqe_st) {
        iir_resample_process(downlink_resample_sco2vqe_st, pcm_buf, (int16_t *)out_buf, pcm_len);
        pcm_buf = (int16_t *)out_buf;
        pcm_len = pcm_len * rx_vqe_sample_rate / sco_sample_rate;
    }

#if defined(SPEECH_RX_24BIT)
    arm_q15_to_q23((int16_t *)pcm_buf, (int32_t *)out_buf, pcm_len);
#endif

    speech_rx_process(pcm_buf, &pcm_len);

    if (downlink_resample_vqe2codec_st) {
        ASSERT(rx_codec_sample_rate >= rx_vqe_sample_rate, "[%s] codec sample rate(%d) should be large than vqe sample rate(%d)", __FUNCTION__, rx_codec_sample_rate, rx_vqe_sample_rate);

        iir_resample_process(downlink_resample_vqe2codec_st, pcm_buf, pcm_buf, pcm_len);
    }

#if defined(SPEECH_RX_24BIT)
    out_len *= 2;
#endif

    return 0;
}

static inline void split_uplink_data(TX_PCM_T *pcm_buf, TX_PCM_T *echo_buf, uint32_t pcm_len)
{
    TX_PCM_T *pcm_buf_j = pcm_buf;
    TX_PCM_T *pcm_buf_i = pcm_buf;
    uint32_t count = pcm_len / (SPEECH_CODEC_CAPTURE_CHANNEL_NUM + 1);
    while (count-- > 0) {
#if SPEECH_CODEC_CAPTURE_CHANNEL_NUM == 1
        *pcm_buf_j++ = *pcm_buf_i++;
#elif SPEECH_CODEC_CAPTURE_CHANNEL_NUM == 2
        *pcm_buf_j++ = *pcm_buf_i++;
        *pcm_buf_j++ = *pcm_buf_i++;
#elif SPEECH_CODEC_CAPTURE_CHANNEL_NUM == 3
        *pcm_buf_j++ = *pcm_buf_i++;
        *pcm_buf_j++ = *pcm_buf_i++;
        *pcm_buf_j++ = *pcm_buf_i++;
#elif SPEECH_CODEC_CAPTURE_CHANNEL_NUM == 4
        *pcm_buf_j++ = *pcm_buf_i++;
        *pcm_buf_j++ = *pcm_buf_i++;
        *pcm_buf_j++ = *pcm_buf_i++;
        *pcm_buf_j++ = *pcm_buf_i++;
#elif SPEECH_CODEC_CAPTURE_CHANNEL_NUM == 5
        *pcm_buf_j++ = *pcm_buf_i++;
        *pcm_buf_j++ = *pcm_buf_i++;
        *pcm_buf_j++ = *pcm_buf_i++;
        *pcm_buf_j++ = *pcm_buf_i++;
        *pcm_buf_j++ = *pcm_buf_i++;
#else
#error "Invalid SPEECH_CODEC_CAPTURE_CHANNEL_NUM!!!"
#endif
#if defined(SPEECH_TX_AEC) || defined(SPEECH_TX_AEC2) || defined(SPEECH_TX_AEC3) || defined(SPEECH_TX_AEC2FLOAT) || \
    defined(SPEECH_TX_3MIC_NS) || defined(SPEECH_TX_2MIC_NS4) || defined(SPEECH_TX_2MIC_NS7) || defined(SPEECH_TX_THIRDPARTY) || \
    defined(SPEECH_TX_2MIC_NS8) || defined(SPEECH_TX_XIAOMI_AEC)
        *echo_buf++ = *pcm_buf_i++;
#endif
    }
}

/*
 * possible uplink resample routine
 * msbc: 16k -> 16k -> 16k (normal)
 *       8k -> 8k -> 16k (fake msbc)
 *       48k -> 16k -> 16k (PSAP)
 *       48k -> 8k -> 16k (fake msbc + PSAP)
 * cvsd: 8k -> 8k -> 8k (normal)
 *       16k -> 16k -> 8k (algo only support 16k)
 *       48k -> 8k -> 8k (PSAP)
 *       48k -> 16k -> 8k (algo only support 16k + PSAP)
 */
int process_uplink_bt_voice_frames(uint8_t *in_buf, uint32_t in_len, uint8_t *ref_buf, uint32_t ref_len, uint8_t *out_buf, uint32_t out_len)
{
    //TRACE(3,"[%s] in_len = %d, out_len = %d", __FUNCTION__, in_len, out_len);

    if (speech_inited == false) {
        TRACE(1, "[%s] warnning: speech init not finished", __FUNCTION__);
        memset(out_buf, MSBC_MUTE_PATTERN, out_len);
        return 0;
    }

#if defined(CALL_BYPASS_SLAVE_TX_PROCESS)
#ifdef IBRT
    if (app_ibrt_conn_get_ui_role() != TWS_UI_UNKNOWN)
    {
        if (bt_sco_chain_get_bypass_flag()) {
            if (g_tx_chain_bypass_log_cnt++ >= TX_CHAIN_BYPASS_LOG_THD) {
                g_tx_chain_bypass_log_cnt = 0;
                TRACE(0, "[MUTE] Bypass phone call tx chain for slave side ...");
            }
            return 0;
        }
    } else
#endif
    {
        if (bt_sco_chain_get_bypass_flag()) {
            bt_sco_chain_set_master_role(true);
        }
    } 
#endif

    TX_PCM_T *pcm_buf = (TX_PCM_T *)in_buf;
    int pcm_len = in_len / sizeof(TX_PCM_T);

    if (uplink_resample_codec2vqe_st) {
        ASSERT(tx_codec_sample_rate >= tx_vqe_sample_rate, "[%s] codec sample rate(%d) should be large than vqe sample rate(%d)", __FUNCTION__, tx_codec_sample_rate, tx_vqe_sample_rate);

        iir_resample_process(uplink_resample_codec2vqe_st, pcm_buf, pcm_buf, pcm_len);
        pcm_len = pcm_len * tx_vqe_sample_rate / tx_codec_sample_rate;
    }

#if defined(SPEECH_TX_AEC_CODEC_REF)
    ASSERT(pcm_len % (SPEECH_CODEC_CAPTURE_CHANNEL_NUM + 1) == 0, "[%s] pcm_len(%d) should be divided by %d", __FUNCTION__, pcm_len, SPEECH_CODEC_CAPTURE_CHANNEL_NUM + 1);
    split_uplink_data(pcm_buf, aec_echo_buf, pcm_len);
    pcm_len = pcm_len / (SPEECH_CODEC_CAPTURE_CHANNEL_NUM + 1) * SPEECH_CODEC_CAPTURE_CHANNEL_NUM;
#endif

    if (g_bypass_tx_algo_sel_ch == 0xFF) {
        speech_tx_process(pcm_buf, aec_echo_buf, &pcm_len);
    } else {
    	/* This function supports 1-4 channels of audio dump
    	*  Please use this function when "audio channel > 1", see:
    	*  {"bypass_tx_algo",      audio_test_bypass_tx_algo},
        */
        #if defined(BT_SCO_CHAIN_AUDIO_DUMP)
        speech_tx_process_audio_dump(pcm_buf, &pcm_len, SPEECH_CODEC_CAPTURE_CHANNEL_NUM);
        #endif
        for(int32_t i=0; i<pcm_len / SPEECH_CODEC_CAPTURE_CHANNEL_NUM; i++) {
            pcm_buf[i] = pcm_buf[SPEECH_CODEC_CAPTURE_CHANNEL_NUM * i + g_bypass_tx_algo_sel_ch];
        }
        pcm_len = pcm_len / SPEECH_CODEC_CAPTURE_CHANNEL_NUM;
    }

#if defined(SPEECH_TX_24BIT) && !defined(XIAOMI_TX_3MIC_ENABLE)
    arm_q23_to_q15((q23_t *)pcm_buf, (q15_t *)pcm_buf, pcm_len);
#endif

    if (uplink_resample_vqe2sco_st) {
        int16_t *uplink_buf = (int16_t *)pcm_buf;
        if (resample_buf != NULL) {
            uplink_buf = resample_buf;
        }

        iir_resample_process(uplink_resample_vqe2sco_st, pcm_buf, uplink_buf, pcm_len);
        pcm_buf = (TX_PCM_T *)uplink_buf;
        pcm_len = pcm_len * sco_sample_rate / tx_vqe_sample_rate;
    }

    if (g_bt_sco_codec_ptr) {
        g_bt_sco_codec_ptr->encoder(out_buf, out_len, (uint8_t *)pcm_buf, pcm_len * sizeof(int16_t));
    }

    return 0;
}

void *voicebtpcm_get_ext_buff(int size)
{
    uint8_t *pBuff = NULL;
    if (size % 4)
    {
        size = size + (4 - size % 4);
    }
    app_audio_mempool_get_buff(&pBuff, size);
    VOICEBTPCM_TRACE(2,"[%s] len:%d", __func__, size);
    return (void*)pBuff;
}
// sco sample rate: encoder/decoder sample rate
// vqe sample rate: speech algorithm process sample rate
// codec sample rate: hardware sample rate
/*
 *                    downlink_resample_sco2vqe             downlink_resample_vqe2codec
 * bt ---> msbc/cvsd --------------------------> algorithm ----------------------------> codec
 *                     uplink_resample_vqe2sco               uplink_resample_codec2vqe
 * bt <--- msbc/cvsd <-------------------------- algorithm <---------------------------- codec
 */
int voicebtpcm_pcm_audio_init(int _sco_sample_rate,
                              int _tx_vqe_sample_rate, int _rx_vqe_sample_rate,
                              int _tx_codec_sample_rate, int _rx_codec_sample_rate,
                              int _capture_channel_num)
{
    uint8_t POSSIBLY_UNUSED *speech_buf = NULL;
    int POSSIBLY_UNUSED speech_len = 0;

#if defined(CALL_BYPASS_SLAVE_TX_PROCESS)
    g_tx_chain_bypass_log_cnt = 0;
#endif

    sco_sample_rate = _sco_sample_rate;
    tx_vqe_sample_rate = _tx_vqe_sample_rate;
    rx_vqe_sample_rate = _rx_vqe_sample_rate;
    tx_codec_sample_rate = _tx_codec_sample_rate;
    rx_codec_sample_rate = _rx_codec_sample_rate;

    sco_frame_length = SPEECH_FRAME_MS_TO_LEN(sco_sample_rate, SPEECH_SCO_FRAME_MS);
    tx_vqe_frame_length = SPEECH_FRAME_MS_TO_LEN(tx_vqe_sample_rate, SPEECH_SCO_FRAME_MS);
    rx_vqe_frame_length = SPEECH_FRAME_MS_TO_LEN(rx_vqe_sample_rate, SPEECH_SCO_FRAME_MS);
    tx_codec_frame_length = SPEECH_FRAME_MS_TO_LEN(tx_codec_sample_rate, SPEECH_SCO_FRAME_MS);
    rx_codec_frame_length = SPEECH_FRAME_MS_TO_LEN(rx_codec_sample_rate, SPEECH_SCO_FRAME_MS);

    TRACE(3, "[%s] SCO  : sample rate = %d, frame len = %d", __func__, sco_sample_rate, sco_frame_length);
    TRACE(3, "[%s] VQE  : tx_sample rate = %d, tx_frame len = %d, rx_sample_rate = %d, rx_frame_len = %d", __func__,
        tx_vqe_sample_rate, tx_vqe_frame_length, rx_vqe_sample_rate, rx_vqe_frame_length);
    TRACE(3, "[%s] CODEC: tx_sample rate = %d, tx_frame len = %d, rx_sample_rate = %d, rx_frame_len = %d", __func__,
        tx_codec_sample_rate, tx_codec_frame_length, rx_codec_sample_rate, rx_codec_frame_length);

    if ((sco_sample_rate < tx_codec_sample_rate) || (sco_sample_rate < rx_codec_sample_rate) || (tx_codec_sample_rate > rx_codec_sample_rate)) {
        TRACE(1, "[%s] SCO <-- Resample --> VQE", __func__);
        resample_buf = (int16_t *)voicebtpcm_get_ext_buff(sizeof(int16_t) * sco_frame_length);
    }

    speech_len = app_audio_mempool_free_buff_size() - APP_BT_STREAM_USE_BUF_SIZE;
    speech_buf = (uint8_t *)voicebtpcm_get_ext_buff(speech_len);

    int tx_frame_ms = SPEECH_PROCESS_FRAME_MS;
    int rx_frame_ms = SPEECH_SCO_FRAME_MS;
    speech_init(tx_vqe_sample_rate, rx_vqe_sample_rate, tx_frame_ms, rx_frame_ms, SPEECH_SCO_FRAME_MS, speech_buf, speech_len);

    // NOTE: Some modules must be created after speech init if they use speech heap

    if (tx_vqe_sample_rate != sco_sample_rate) {
        TRACE(1, "[%s] TX VQE <-- Resample --> SCO", __func__);
        uplink_resample_vqe2sco_st = iir_resample_init(tx_vqe_frame_length, 16, iir_resample_choose_mode(tx_vqe_sample_rate, sco_sample_rate));
    } else {
        uplink_resample_vqe2sco_st = NULL;
    }

    if (sco_sample_rate != rx_vqe_sample_rate) {
        TRACE(1, "[%s] RX VQE <-- Resample --> SCO", __func__);
        downlink_resample_sco2vqe_st = iir_resample_init(sco_frame_length, 16, iir_resample_choose_mode(sco_sample_rate, rx_vqe_sample_rate));
    } else {
        downlink_resample_sco2vqe_st = NULL;
    }

    if (tx_codec_sample_rate != tx_vqe_sample_rate) {
        TRACE(1, "[%s] TX CODEC <-- Resample --> VQE", __func__);
        uplink_resample_codec2vqe_st = multi_iir_resample_init(tx_codec_frame_length * _capture_channel_num, sizeof(TX_PCM_T) * 8,  _capture_channel_num, iir_resample_choose_mode(tx_codec_sample_rate, tx_vqe_sample_rate));
    } else {
        uplink_resample_codec2vqe_st = NULL;
    }

    if (rx_vqe_sample_rate != rx_codec_sample_rate) {
        TRACE(1, "[%s] RX VQE <-- Resample --> CODEC", __func__);
        downlink_resample_vqe2codec_st = iir_resample_init(rx_vqe_frame_length, sizeof(RX_PCM_T) * 8, iir_resample_choose_mode(rx_vqe_sample_rate, rx_codec_sample_rate));
    } else {
        downlink_resample_vqe2codec_st = NULL;
    }

    if (16000 != tx_vqe_sample_rate) {
        TRACE(1, "[%s] TX VPU <-- Resample --> VQE", __func__);
        uplink_resample_vpu2vqe_st = iir_resample_init(240, sizeof(TX_PCM_T), iir_resample_choose_mode(16000, tx_vqe_sample_rate));
    } else {
        uplink_resample_vpu2vqe_st = NULL;
    }

#if !defined(SPEECH_TX_AEC_CODEC_REF)
#if defined(SPEECH_TX_AEC) || defined(SPEECH_TX_AEC2) || defined(SPEECH_TX_AEC3) || defined(SPEECH_TX_AEC2FLOAT) || defined(SPEECH_TX_3MIC_NS) || defined(SPEECH_TX_2MIC_NS4) || defined(SPEECH_TX_2MIC_NS7) || defined(SPEECH_TX_2MIC_NS8) || defined(SPEECH_TX_THIRDPARTY)
    if (tx_codec_sample_rate != rx_codec_sample_rate) {
        TRACE(1, "[%s] RX ECHO <-- Resample --> TX ECHO", __func__);
        resample_echo_st = iir_resample_init(rx_codec_frame_length, sizeof(RX_PCM_T) * 8, iir_resample_choose_mode(rx_codec_sample_rate, tx_vqe_sample_rate));
    } else
#endif
#endif
    {
        resample_echo_st = NULL;
    }

    if (g_bt_sco_codec_ptr == NULL) {
        hfp_sco_codec_t sco_codec_type = bt_sco_codec_get_type();
        if (sco_codec_type == BT_HFP_SCO_CODEC_CVSD) {
            g_bt_sco_codec_ptr = &bt_sco_codec_cvsd;
        } else if (sco_codec_type == BT_HFP_SCO_CODEC_MSBC) {
            g_bt_sco_codec_ptr = &bt_sco_codec_msbc;
#if defined(SCO_ADD_CUSTOMER_CODEC)
        } else if (sco_codec_type == BT_HFP_SCO_CODEC_XXXX) {
            g_bt_sco_codec_ptr = &bt_sco_codec_xxxx;
#endif
        } else {
            ASSERT(0, "[%s] %x is invalid sco codec type!", __func__, sco_codec_type);
        }

        g_bt_sco_codec_ptr->init(sco_sample_rate);
    } else {
        ASSERT(0, "[%s] g_bt_sco_codec_ptr != NULL", __func__);
    }

    speech_inited = true;

    return 0;
}

int voicebtpcm_pcm_audio_deinit(void)
{
    TRACE(1,"[%s] Close...", __func__);
    // TRACE(2,"[%s] app audio buffer free = %d", __func__, app_audio_mempool_free_buff_size());

    g_bt_sco_codec_ptr->deinit();
    g_bt_sco_codec_ptr = NULL;

    if (uplink_resample_vqe2sco_st) {
        iir_resample_destroy(uplink_resample_vqe2sco_st);
    }

    if (downlink_resample_sco2vqe_st) {
        iir_resample_destroy(downlink_resample_sco2vqe_st);
    }

    if (uplink_resample_codec2vqe_st) {
        iir_resample_destroy(uplink_resample_codec2vqe_st);
    }

    if (downlink_resample_vqe2codec_st) {
        iir_resample_destroy(downlink_resample_vqe2codec_st);
    }

    if (resample_echo_st) {
        iir_resample_destroy(resample_echo_st);
    }

    if (uplink_resample_vpu2vqe_st) {
        iir_resample_destroy(uplink_resample_vpu2vqe_st);
    }

    resample_buf = NULL;

    speech_deinit();

    speech_inited = false;

    // TRACE(1,"Free buf = %d", app_audio_mempool_free_buff_size());

    return 0;
}
