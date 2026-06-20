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
#ifndef __VOICE_SBC_H__
#define __VOICE_SBC_H__
#include "cqueue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t channelCnt;
    uint8_t channelMode;
    uint8_t bitPool;
    uint8_t sizePerSample;
    uint8_t sampleRate;
    uint8_t numBlocks;
    uint8_t numSubBands;
    uint8_t mSbcFlag;
    uint8_t allocMethod;
} VOICE_SBC_CONFIG_T;

#define VOICE_SBC_FRAME_HEADER_LEN (4)

#define VOICE_SBC_CHANNEL_COUNT (1)
#define VOICE_SBC_CHANNEL_MODE (BTIF_SBC_CHNL_MODE_MONO)
// bitpool vs block size:
/*
block size = 4 + (4 * nrof_subbands * nrof_channels ) / 8 + round_up(nrof_blocks * nrof_channels * bitpool / 8)

block size = 4 + (4 * 8 * 1) / 8 + round_up(16 * 1 * bitpool / 8)

block size = 56
*/
// 10 - 28: 3.5KB/s (suggested to be used for android ble)
// 12 - 32: 4KB/s 	(suggested to be used for ios ble)
// 24 - 56: 7KB/s	(suggested to be used for android RFComm)

#define VOICE_SBC_BIT_POOL (24)
#define VOICE_SBC_BLOCK_SIZE 56

#define VOICE_SBC_SIZE_PER_SAMPLE (2)  // 16 bits, 1 channel
#define VOICE_SBC_SAMPLE_RATE (BTIF_SBC_CHNL_SAMPLE_FREQ_16)
#define VOICE_SBC_NUM_BLOCKS 16
#define VOICE_SBC_NUM_SUB_BANDS 8
#define VOICE_SBC_MSBC_FLAG 0
#define VOICE_SBC_ALLOC_METHOD BTIF_SBC_ALLOC_METHOD_LOUDNESS

#define VOICE_SBC_SAMPLE_RATE_VALUE 16000
#define VOICE_SBC_FRAME_PERIOD_IN_MS (20)
#define VOICE_SBC_HANDLE_INTERVAL_MS (48)

#define VOICE_SBC_PCM_DATA_SIZE_PER_FRAME \
    (((VOICE_SBC_FRAME_PERIOD_IN_MS * VOICE_SBC_SAMPLE_RATE_VALUE) / 1000) * VOICE_SBC_SIZE_PER_SAMPLE)
#define VOICE_SBC_PCM_DATA_SIZE_PER_HANDLE \
    (((VOICE_SBC_HANDLE_INTERVAL_MS * VOICE_SBC_SAMPLE_RATE_VALUE) / 1000) * VOICE_SBC_SIZE_PER_SAMPLE)

/**
 * @brief encode data into SBC frames
 * 
 * @param input pointer of input PCM data
 * @param inputBytes lenght of input PCM data
 * @param purchasedBytes length of data consumed by SBC encoder
 * @param output pointer of output encoded SBC data
 * @param isReset flag of if encoder need to be reset
 * @return uint32_t length of output encoded SBC data
 */
uint32_t voice_sbc_encode(uint8_t *input, uint32_t inputBytes, uint32_t *purchasedBytes, uint8_t *output, uint8_t isReset);

/**
 * @brief configure the SBC encoder related configurations
 * NOTE: this configuration will not take effect immediately,
 *       need to call @see voice_sbc_init_encoder to take effect
 * 
 * @param pConfig pointer of updated configuration
 * @return int result
 */
int voice_sbc_init(VOICE_SBC_CONFIG_T *pConfig);

uint32_t voice_sbc_get_frame_len(void);

#ifdef __cplusplus
}
#endif

#endif  // __VOICE_SBC_H__
