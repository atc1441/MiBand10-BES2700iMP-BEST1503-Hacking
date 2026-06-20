/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifdef BT_A2DP_SUPPORT
#include "app_source_codec.h"
#include "a2dp_codec_sbc.h"
#include "a2dp_encoder_sbc_cp.h"
#include "audio_codec_api.h"
#include "sbc_api.h"
#include "sbc_error_code.h"

#ifndef A2DP_ENCODE_CP_ACCEL
static bool need_init_encoder = true;
#endif
static sbc_encoder_t __attribute__((aligned(4))) sbc_encoder;
static char a2dp_sbc_transmit_buffer[A2DP_SBC_TRANS_SIZE];

uint8_t *a2dp_source_sbc_frame_buffer(void)
{
    return (uint8_t *)a2dp_sbc_transmit_buffer;
}

uint8_t app_a2dp_source_samplerate_2_sbcenc_type(enum AUD_SAMPRATE_T sample_rate)
{
    uint8_t rate=SBC_SAMPLERATE_16K;
    switch(sample_rate)
    {
        case AUD_SAMPRATE_16000:
            rate =  SBC_SAMPLERATE_16K;
            break;
        case AUD_SAMPRATE_32000:
            rate =  SBC_SAMPLERATE_32K;
            break;
        case AUD_SAMPRATE_44100:
            rate =  SBC_SAMPLERATE_44_1K;
            break;
        case AUD_SAMPRATE_48000:
            rate =  SBC_SAMPLERATE_48K;
            break;
         default:
            TRACE(0,"error!  sbc enc don't support other samplerate");
            break;
    }
    //TRACE(3,"\n%s %d rate = %x\n", __func__, __LINE__, rate);
    return rate;
}

bool a2dp_source_mcu_encode_sbc_packet(a2dp_source_packet_t *source_packet)
{
    uint32_t frame_size = 512;
    uint32_t frame_num = A2DP_SBC_TRANS_SIZE/frame_size;
    unsigned short enc_len = 0;

    btif_a2dp_sbc_packet_t *packet = &source_packet->packet;

    sbc_stream_info_t info;
    audio_bt_sbc_encoder_get_stream_info(&sbc_encoder, &info);
    pcm_frame_t pcm_data;

#if BT_SOURCE_DEVICE_NUM > 1
    info.num_channels = 2;
    info.sample_rate = app_a2dp_source_samplerate_2_sbcenc_type(AUD_SAMPRATE_48000);
#else
    uint8_t device_id = app_bt_source_get_streaming_a2dp();
    struct BT_SOURCE_DEVICE_T *curr_device = NULL;

    if (device_id == BT_SOURCE_DEVICE_INVALID_ID)
    {
        return false;
    }

    curr_device = app_bt_source_get_device(device_id);

    info.num_channels = curr_device->base_device->a2dp_channel_num;
    info.sample_rate = app_a2dp_source_samplerate_2_sbcenc_type(curr_device->aud_sample_rate);
#endif
    sbc_stream_info_t prev_info;
    audio_bt_sbc_encoder_get_stream_info(&sbc_encoder, &prev_info);
    if(prev_info.sample_rate != info.sample_rate) 
    {
        audio_bt_sbc_encoder_set_stream_info(&sbc_encoder, &info);
        audio_bt_sbc_init_encoder(&sbc_encoder, &info);
    }
    pcm_data.pcm_data = (int16_t *)a2dp_sbc_transmit_buffer;
    pcm_data.buffer_size = A2DP_SBC_TRANS_SIZE;
    pcm_data.valid_size = A2DP_SBC_TRANS_SIZE;
    sbc_frame_t sbc_data;
    sbc_data.sbc_data = packet->data;
    sbc_data.buffer_size = packet->reserved_data_size;
    sbc_data.valid_size = 0;

    for (uint32_t i = 0; i < frame_num; i++)
    {
        pcm_data.pcm_data = (int16_t *)(a2dp_sbc_transmit_buffer + i * frame_size);
        pcm_data.buffer_size = frame_size;
        pcm_data.valid_size = frame_size;

        sbc_data.sbc_data = packet->data + i * sbc_data.valid_size;
        sbc_data.buffer_size = packet->reserved_data_size;
        sbc_data.valid_size = 0;

        audio_bt_sbc_encode_frames(&sbc_encoder, &pcm_data, &sbc_data);
        enc_len += sbc_data.valid_size;
    }

    ASSERT(enc_len < packet->reserved_data_size, "sbc encodec packet length too long");
    packet->dataLen = enc_len;
    packet->frameSize = enc_len/frame_num;

#if 1
    TRACE(4, "%s dataLen %d frameSize %d frame_num %d\n", __func__,
        packet->dataLen, packet->frameSize, frame_num);
#endif

    source_packet->codec_type = BT_A2DP_CODEC_TYPE_SBC;
    return true;
}


bool a2dp_source_encode_sbc_packet(a2dp_source_packet_t *source_packet)
{
#if defined(A2DP_ENCODE_CP_ACCEL)
    return a2dp_source_cp_encode_sbc_packet(source_packet);
#else
    return a2dp_source_mcu_encode_sbc_packet(source_packet);
#endif
}

int a2dp_source_sbc_init(void)
{
#if defined(A2DP_ENCODE_CP_ACCEL)
        int ret = 0;
        ret = a2dp_encode_sbc_cp_init();
        if (ret != 0) {
            TRACE(0, "a2dp_source_sbc_init : error, encode sbc cp init fialed, ret=%d", ret);
        }
#else
    if (need_init_encoder) {
        sbc_stream_info_t info;
        info.num_channels = 2;
        info.channel_mode = SBC_CHANNEL_MODE_JOINT_STEREO;
        info.bit_pool     = A2DP_SBC_BITPOOL;
        info.sample_rate  = SBC_SAMPLERATE_48K;
        info.alloc_method = SBC_ALLOC_METHOD_SNR;
        info.num_blocks   = 16;
        info.num_subbands = 8;
        info.flags = SBC_FLAGS_DEFAULT;
        audio_bt_sbc_init_encoder(&sbc_encoder, &info);
        need_init_encoder = 0;
    }
#endif

    return 0;
}


void a2dp_source_sbc_encoder_init(void)
{
    a2dp_source_sbc_init();
}

void a2dp_source_sbc_encoder_deinit(void)
{
    TRACE(1,"%s",__func__);
#if defined(A2DP_ENCODE_CP_ACCEL)
    a2dp_encode_sbc_cp_deinit();
#endif
}


/**
 * For the decoder in the SNK the sampling frequencies 44.1kHz and 48kHz
 * are mandatory to support. The encoder in the SRC shall support at
 * least one of the sampling frequencies of 44.1kHz and 48kHz.
 *
 * The channel mode for SBC: MONO, DUAL CHANNEL, STEREO, JOINT STEREO.
 * For the decoder in the SNK all features shall be supported. The
 * encoder in the SRC shall support at least MONO and one of DUAL CHANNEL,
 * STEREO and JOINT STEREO modes.
 *
 * The Number of Subbands, for the decoder in the SNK, all features 4
 * and 8 subbands shall be supported. The encoder in the SRC shall support
 * at least 8 subbands case.
 *
 */

static btif_avdtp_codec_t a2dp_source_sbc_avdtpcodec;

#if BT_SOURCE_DEVICE_NUM > 1
static const unsigned char a2dp_source_sbc_codec_elements[] = {
    A2D_SBC_IE_SAMP_FREQ_44 | A2D_SBC_IE_CH_MD_STEREO | A2D_SBC_IE_CH_MD_JOINT | A2D_SBC_IE_CH_MD_DUAL,
    A2D_SBC_IE_BLOCKS_16 | A2D_SBC_IE_SUBBAND_8 | A2D_SBC_IE_ALLOC_MD_S,
    A2D_SBC_IE_MIN_BITPOOL,
    A2DP_SBC_BITPOOL, //BTA_AV_CO_SBC_MAX_BITPOOL
};
#else
static const unsigned char a2dp_source_sbc_codec_elements[] = {
    A2D_SBC_IE_SAMP_FREQ_48 | A2D_SBC_IE_SAMP_FREQ_44 | A2D_SBC_IE_CH_MD_MONO | A2D_SBC_IE_CH_MD_STEREO | A2D_SBC_IE_CH_MD_JOINT | A2D_SBC_IE_CH_MD_DUAL,
    A2D_SBC_IE_BLOCKS_16 | A2D_SBC_IE_BLOCKS_12 | A2D_SBC_IE_BLOCKS_8  |A2D_SBC_IE_BLOCKS_4 | A2D_SBC_IE_SUBBAND_8 | A2D_SBC_IE_ALLOC_MD_L | A2D_SBC_IE_ALLOC_MD_S,
    A2D_SBC_IE_MIN_BITPOOL,
    A2DP_SBC_BITPOOL, //BTA_AV_CO_SBC_MAX_BITPOOL
};
#endif

void a2dp_source_register_sbc_codec(btif_a2dp_stream_t *btif_a2dp, btif_avdtp_content_prot_t *sep_cp, uint8_t sep_priority, btif_a2dp_callback callback)
{
    a2dp_source_sbc_avdtpcodec.codecType = BT_A2DP_CODEC_TYPE_SBC;
    a2dp_source_sbc_avdtpcodec.discoverable = 1;
    a2dp_source_sbc_avdtpcodec.elements = (U8 *)&a2dp_source_sbc_codec_elements;
    a2dp_source_sbc_avdtpcodec.elemLen  = sizeof(a2dp_source_sbc_codec_elements);

    btif_a2dp_register(btif_a2dp, BTIF_A2DP_STREAM_TYPE_SOURCE, &a2dp_source_sbc_avdtpcodec, sep_cp, sep_priority, callback);
}

void a2dp_source_pts_sbc_init(uint8_t *codec_info)
{
    sbc_stream_info_t info;
    info.num_channels = 2;
    info.channel_mode = codec_info[0];
    info.bit_pool     = codec_info[1];
    info.sample_rate  = SBC_SAMPLERATE_48K;
    info.alloc_method =  codec_info[2];
    info.num_blocks   = codec_info[3];;
    info.num_subbands = 8;
    info.flags = codec_info[4];
    audio_bt_sbc_encoder_set_stream_info(&sbc_encoder, &info);
}

#endif /* BT_A2DP_SUPPORT */
