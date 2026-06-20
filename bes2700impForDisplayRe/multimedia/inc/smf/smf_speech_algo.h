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
#ifndef __SMF_AUDIO_SPEECH_H__
#define __SMF_AUDIO_SPEECH_H__

#include "stdint.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
#if defined(SPEECH_TX_DC_FILTER)
    SpeechDcFilterConfig    tx_dc_filter;
#endif
#if defined(SPEECH_TX_AEC2FLOAT)
    Ec2FloatConfig          tx_aec2float;
#endif
#if defined(SPEECH_TX_2MIC_NS8)
    Speech2MicNs8Config     tx_2mic_ns8;
#endif
#if defined(SPEECH_TX_NS8)
    SpeechNs8Config         tx_ns8;
#endif
#if defined(SPEECH_TX_NS9)
    SpeechNs9Config         tx_ns9;
#endif
#if defined(SPEECH_TX_COMPEXP)
    CompexpConfig           tx_compexp;
#endif
#if defined(SPEECH_TX_AGC)
    AgcConfig               tx_agc;
#endif
#if defined(SPEECH_TX_EQ)
    EqConfig                tx_eq;
#endif
#if defined(SPEECH_TX_POST_GAIN)
    SpeechGainConfig        tx_post_gain;
#endif

#if defined(SPEECH_RX_NS2FLOAT)
    SpeechNs2FloatConfig    rx_ns2float;
#endif
#if defined(SPEECH_RX_COMPEXP)
    MultiCompexpConfig      rx_compexp;
#endif
#if defined(SPEECH_RX_AGC)
    AgcConfig               rx_agc;
#endif
#if defined(SPEECH_RX_EQ)
    EqConfig                rx_eq;
#endif
#if defined(SPEECH_RX_DAC_EQ)
    EqConfig                rx_dac_eq;
#endif
#if defined(SPEECH_RX_POST_GAIN)
    SpeechGainConfig        rx_post_gain;
#endif
    // Add more process
} SpeechAlgoConfig;

typedef struct {
    uint32_t sample_rate;
    uint32_t sample_bits;
    uint32_t ch_num;
    uint32_t frame_len;
    uint32_t ref_len;
    uint32_t mic_num;
} AUDIO_SPEECH_CONFIG_T;

uint32_t smf_speech_algo_get_mem_size(void);
bool smf_speech_algo_mem_init(void* buff, uint32_t size);
bool smf_speech_algo_set_cfg(void *buff, uint32_t len);
bool smf_speech_algo_tx_open(AUDIO_SPEECH_CONFIG_T *param, void* buff, uint32_t len);
bool smf_speech_algo_rx_open(AUDIO_SPEECH_CONFIG_T *param, void* buff, uint32_t len);
bool smf_speech_algo_tx_close(void);
bool smf_speech_algo_rx_close(void);
bool smf_speech_algo_capture_process_run(void *mic_buff, void *ref_buff, void *out_buff, uint32_t frame_len);
bool smf_speech_algo_playback_process_run(void *in_buff, void *out_buff, uint32_t frame_len);

#ifdef __cplusplus
}
#endif

#endif
