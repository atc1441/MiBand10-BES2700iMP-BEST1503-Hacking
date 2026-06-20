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
#include "voice_sbc.h"
#include "hal_trace.h"

static VOICE_SBC_CONFIG_T voice_sbc_config =
{
    VOICE_SBC_CHANNEL_COUNT     ,
    VOICE_SBC_CHANNEL_MODE      ,
    VOICE_SBC_BIT_POOL          ,
    VOICE_SBC_SIZE_PER_SAMPLE   ,
    VOICE_SBC_SAMPLE_RATE       ,
    VOICE_SBC_NUM_BLOCKS        ,
    VOICE_SBC_NUM_SUB_BANDS     ,
    VOICE_SBC_MSBC_FLAG         ,
    VOICE_SBC_ALLOC_METHOD      ,
};

static int voice_sbc_init_encoder(void);

static btif_sbc_encoder_t voice_sbc_encoder;
static btif_sbc_decoder_t voice_sbc_decoder;
static uint32_t voice_sbc_frame_len = 0;

static float POSSIBLY_UNUSED voice_sbc_eq_band_gain[8] = {1, 1, 1, 1, 1, 1, 1, 1};

static int voice_sbc_init_encoder(void)
{
    btif_sbc_init_encoder(&voice_sbc_encoder);
    voice_sbc_encoder.streamInfo.numChannels = voice_sbc_config.channelCnt;
    voice_sbc_encoder.streamInfo.channelMode = voice_sbc_config.channelMode;
    voice_sbc_encoder.streamInfo.bitPool     = voice_sbc_config.bitPool;
    voice_sbc_encoder.streamInfo.sampleFreq  = voice_sbc_config.sampleRate;
    voice_sbc_encoder.streamInfo.allocMethod = voice_sbc_config.allocMethod;
    voice_sbc_encoder.streamInfo.numBlocks   = voice_sbc_config.numBlocks;
    voice_sbc_encoder.streamInfo.numSubBands = voice_sbc_config.numSubBands;
    voice_sbc_encoder.streamInfo.mSbcFlag    = voice_sbc_config.mSbcFlag;

    voice_sbc_frame_len = btif_sbc_frame_len(&(voice_sbc_encoder.streamInfo));

    TRACE(1,"frame len is %d", voice_sbc_frame_len);
    return 0;
}

static int POSSIBLY_UNUSED voice_sbc_init_decoder(void)
{
    btif_sbc_init_decoder(&voice_sbc_decoder);
    voice_sbc_decoder.streamInfo.numChannels = voice_sbc_config.channelCnt;
    voice_sbc_decoder.streamInfo.channelMode = voice_sbc_config.channelMode;
    voice_sbc_decoder.streamInfo.bitPool     = voice_sbc_config.bitPool;
    voice_sbc_decoder.streamInfo.sampleFreq  = voice_sbc_config.sampleRate;
    voice_sbc_decoder.streamInfo.allocMethod = voice_sbc_config.allocMethod;
    voice_sbc_decoder.streamInfo.numBlocks   = voice_sbc_config.numBlocks;
    voice_sbc_decoder.streamInfo.numSubBands = voice_sbc_config.numSubBands;
    voice_sbc_decoder.streamInfo.mSbcFlag    = voice_sbc_config.mSbcFlag;
    return 0;
}

uint32_t voice_sbc_get_frame_len(void)
{
    return voice_sbc_frame_len;
}

uint32_t voice_sbc_encode(uint8_t *input, uint32_t inputBytes, uint32_t* purchasedBytes, uint8_t *output, uint8_t isReset)
{
    if (isReset)
    {
        voice_sbc_init_encoder();
    }

    btif_sbc_pcm_data_t PcmEncData;
    PcmEncData.data = input;
    PcmEncData.dataLen = inputBytes;
    PcmEncData.numChannels = voice_sbc_encoder.streamInfo.numChannels;
    PcmEncData.sampleFreq = voice_sbc_encoder.streamInfo.sampleFreq;

    uint16_t outputSbcBytes, bytes_encoded;
    btif_sbc_encode_frames(&voice_sbc_encoder,
                           &PcmEncData,
                           &bytes_encoded,
                           output,
                           &outputSbcBytes,
                           0xFFFF);

    *purchasedBytes = bytes_encoded;
    return outputSbcBytes;
}

int voice_sbc_init(VOICE_SBC_CONFIG_T* pConfig)
{
    voice_sbc_config = *pConfig;
    return 0;
}

