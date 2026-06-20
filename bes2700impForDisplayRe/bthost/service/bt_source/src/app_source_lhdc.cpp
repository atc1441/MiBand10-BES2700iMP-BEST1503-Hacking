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
#ifdef  A2DP_SOURCE_LHDC_ON 
#include "app_source_codec.h"
#include "lhdc_enc_api.h"
#include "a2dp_encoder_lhdc_cp.h"

btif_avdtp_codec_t a2dp_lhdc_avdtpcodec;
bool one_frame_per_chennal;

#ifndef A2DP_ENCODE_CP_ACCEL
static bool need_init_encoder = true;
#endif
static char a2dp_lhdc_transmit_buffer[A2DP_LHDC_TRANS_SIZE * 2];
#define LHDC_FRAME_BYTE_SIZE (LHDCV3_BLOCK_SIZE * sizeof(int16_t) * 2)//256 sample
static uint8_t fake_sample[LHDC_FRAME_BYTE_SIZE];

extern int a2dp_source_pcm_buffer_read(uint8_t *buff, uint16_t len);


#if 0
const unsigned char a2dp_codec_lhdc_elements[A2DP_LHDC_OCTET_NUMBER] = {
    0x3A, 0x05, 0x00, 0x00, //Vendor ID
#if 1//lhdc v3.0
    0x33, 0x4c,         //Codec ID
#else
    0x32, 0x4c,         //Codec ID
#endif
    (A2DP_LHDC_SR_96000 | A2DP_LHDC_SR_48000 | A2DP_LHDC_SR_44100) | (A2DP_LHDC_FMT_16 | A2DP_LHDC_FMT_24),
    (
#if defined(IBRT)
    A2DP_LHDC_LLC_ENABLE | 
#endif
    A2DP_LHDC_MAX_SR_900 | A2DP_LHDC_VERSION_NUM),
    (A2DP_LHDC_COF_CSC_DISABLE)
};
#else//current lib only support 48k,16bit
const unsigned char a2dp_codec_lhdc_elements[A2DP_LHDC_OCTET_NUMBER] = {
    0x3A, 0x05, 0x00, 0x00, //Vendor ID
#if 1//lhdc v3.0
    0x33, 0x4c,         //Codec ID
#else
    0x32, 0x4c,         //Codec ID
#endif
    (A2DP_LHDC_SR_48000) | (A2DP_LHDC_FMT_16),
    (
#if defined(IBRT)
    A2DP_LHDC_LLC_ENABLE | 
#endif
    A2DP_LHDC_MAX_SR_900 | A2DP_LHDC_VERSION_NUM),
    (A2DP_LHDC_COF_CSC_DISABLE)
};
#endif

void a2dp_source_register_lhdc_codec(btif_a2dp_stream_t *btif_a2dp, btif_avdtp_content_prot_t *sep_cp, uint8_t sep_priority, btif_a2dp_callback callback)
{
    a2dp_lhdc_avdtpcodec.codecType = BT_A2DP_CODEC_TYPE_NON_A2DP;
    a2dp_lhdc_avdtpcodec.discoverable = 1;
    a2dp_lhdc_avdtpcodec.elements = (U8 *)&a2dp_codec_lhdc_elements;
    a2dp_lhdc_avdtpcodec.elemLen  = sizeof(a2dp_codec_lhdc_elements);

    btif_a2dp_register(btif_a2dp, BTIF_A2DP_STREAM_TYPE_SOURCE, &a2dp_lhdc_avdtpcodec, sep_cp, sep_priority, callback);
}

uint8_t *a2dp_source_lhdc_frame_buffer(void)
{
    return (uint8_t *)a2dp_lhdc_transmit_buffer;
}

bool a2dp_source_mcu_encode_lhdc_packet(a2dp_source_packet_t *source_packet)
{
    uint8_t streaming_a2dp_id = app_bt_source_get_streaming_a2dp();

    uint32_t frame_size = 1024;
    uint32_t frame_num = A2DP_LHDC_TRANS_SIZE/frame_size;
    btif_a2dp_sbc_packet_t *sbcPacket = &source_packet->packet;
    uint8_t * packet_buffer = source_packet->buffer;

    TRACE(0,"a2dp_source_encode_lhdc_packet");

    if (streaming_a2dp_id == BT_SOURCE_DEVICE_INVALID_ID)
    {
        return false;
    }
//encode process
    uint32_t out_frames;
    uint32_t out_len;
    
    uint16_t *packet_len = (uint16_t *)packet_buffer;        
    packet_buffer[3] = packet_buffer[2] = 0;        
    *packet_len = 0;

    do {
        //int lock = int_lock();
        if(a2dp_source_pcm_buffer_read((uint8_t *)a2dp_lhdc_transmit_buffer,A2DP_LHDC_TRANS_SIZE)==0){           
            memcpy(fake_sample,a2dp_lhdc_transmit_buffer,A2DP_LHDC_TRANS_SIZE);//len==256*2*2
            lhdc_enc_result_t res = lhdc_enc_encode(fake_sample, packet_buffer +  sizeof(uint16_t), &out_len, &out_frames);
            TRACE(1,"res=%d",res);
        }
        //int_unlock(lock);
    }while (!out_frames && !out_len);

    ASSERT(out_len < 1024, "lhdc encodec packet length too long");

    if (out_frames && out_len)
    {
        *packet_len = out_len;
        sbcPacket->data=&packet_buffer[2];
        sbcPacket->dataLen = out_len;
        sbcPacket->frameSize = out_len/frame_num;
        TRACE(1,"%s %d",__func__,sbcPacket->dataLen);
        //DUMP8("%02x ",sbcPacket->data,8);
        source_packet->codec_type = BT_A2DP_CODEC_TYPE_NON_A2DP;
        return true;
    }
    else
    {
        return false;
    }
}

bool a2dp_source_encode_lhdc_packet(a2dp_source_packet_t *source_packet)
{
#if defined(A2DP_ENCODE_CP_ACCEL)
    if (a2dp_source_pcm_buffer_read(a2dp_source_lhdc_frame_buffer(), A2DP_LHDC_TRANS_SIZE) == 0)
    {
        return a2dp_source_cp_encode_lhdc_packet(source_packet);
    }
#else
    return a2dp_source_mcu_encode_lhdc_packet(source_packet);
#endif
    return 0;
}


void a2dp_source_lhdc_encoder_init(void)
{
    //a2dp_source_sbc_init();
    TRACE(1, "%s", __func__);
#if defined(A2DP_ENCODE_CP_ACCEL)
    TRACE(1, "%s: Run CP", __func__);
    a2dp_encode_lhdc_cp_init();
#else
    struct BT_SOURCE_DEVICE_T *curr_device = NULL;
    TRACE(1, "%s: No CP", __func__);

    if (need_init_encoder) {
        uint8_t device_id = app_bt_source_get_streaming_a2dp();

        if (device_id == BT_SOURCE_DEVICE_INVALID_ID)
        {
            return;
        }

        curr_device = app_bt_source_get_device(device_id);

#if defined(BT_MULTI_SOURCE)
        lhdc_enc_init(curr_device->aud_sample_rate, curr_device->base_device->sample_bit, 350, 600, 10, one_frame_per_chennal);
#else
        lhdc_enc_init(curr_device->aud_sample_rate, curr_device->base_device->sample_bit, 400, 600, 10, one_frame_per_chennal);
#endif
        lhdc_enc_verify_license();

        need_init_encoder = false;
    }
#endif
}

void a2dp_source_lhdc_encoder_deinit(void)
{
    TRACE(1,"%s",__func__);
#if defined(A2DP_ENCODE_CP_ACCEL)
    a2dp_encode_lhdc_cp_deinit();
#endif
}

#endif /* A2DP_SOURCE_LHDC_ON */
#endif /* BT_A2DP_SUPPORT */