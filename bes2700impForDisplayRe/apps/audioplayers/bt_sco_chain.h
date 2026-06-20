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
#ifndef __BT_SCO_CHAIN_H__
#define __BT_SCO_CHAIN_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Init speech algorithm, init all related states and memory
 * Input:
 *      tx_sample_rate: capture sample rate
 *      rx_sample_rate: playback sample rate
 *      tx_frame_ms:    capture frame size in ms
 *      rx_frame_ms:    playback frame size in ms
 *      sco_frame_ms:   sco frame size in ms(must be multiply of 7.5)
 *      buf:            buffer to be used for algorithms
 *      len:            buffer size
 */
int speech_init(int tx_sample_rate, int rx_sample_rate,
                     int tx_frame_ms, int rx_frame_ms,
                     int sco_frame_ms,
                     uint8_t *buf, int len);

int speech_init2(int tx_sample_rate, int rx_sample_rate,
                     int tx_frame_len, int rx_frame_len,
                     int sco_frame_len,
                     uint8_t *buf, int len);

/*
 * Deinit speech algorithm, free all related states and memory
 */
int speech_deinit(void);

/*
 * Uplink process
 * Input:
 *      pcm_buf: captured buffer by microphone, multi-channel is interleaved.
 *      ref_buf: reference buffer for acoustic echo canceller.
 *               buffee sample num should be tx_sample_rate * tx_frame_ms
 *      pcm_len: capture buffer sample num, should be tx_sample_rate * sco_frame_ms * ch_num
 * Output:
 *      pcm_buf: output buffer of speech algorithm, should be single channel
 *      pcm_len: output buffer sample num, should be tx_sample_rate * sco_frame_ms
 */
int speech_tx_process(void *pcm_buf, void *ref_buf, int *pcm_len);

/*
 * Downlink process
 * Input:
 *      pcm_buf: input buffer to be processed
 *      pcm_len: input buffer sample num, should be rx_sample_rate * sco_frame_ms
 * Output:
 *      pcm_buf: output buffer
 *      pcm_len: output buffer sample num, should be rx_sample_rate * sco_frame_ms
 */
int speech_rx_process(void *pcm_buf, int *pcm_len);

void *speech_get_ext_buff(int size);

/*audio dump test*/
int speech_tx_process_audio_dump(void *pcm_buf, int *_pcm_len, int channel_num);

#ifdef SPEECH_ALGO_DSP
int speech_enable_mcpp(bool en);
#endif

#ifdef __cplusplus
}
#endif

#endif