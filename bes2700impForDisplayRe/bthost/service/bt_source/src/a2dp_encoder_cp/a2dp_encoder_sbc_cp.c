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
#if defined(A2DP_ENCODE_CP_ACCEL)

#include <stdio.h>
#include <stdbool.h>
#include "app_source_codec.h"
#include "a2dp_codec_sbc.h"
#include "cp_accel.h"
#include "hal_location.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "heap_api.h"
#include "norflash_api.h"
#include "a2dp_encoder.h"
#include "a2dp_encoder_sbc_cp.h"
#include "a2dp_encoder_cp.h"

static bool need_init_encoder = true;
static btif_sbc_encoder_t sbc_encoder;
static uint32_t enc_sn = 0;

static uint8_t sbc_encode_tmp[A2DP_CODEC_DATA_SIZE];

typedef struct 
{
    uint32_t sn;
    uint32_t length;
    uint8_t data[0];
} sbc_packet_t;


CP_TEXT_SRAM_LOC
int a2dp_cp_sbc_encode(void)
{
    enum CP_EMPTY_OUT_FRM_T out_frm_st;
    int ret;
    bt_status_t encode_ret;
    uint8_t *out_buff = NULL;
    uint32_t out_len = 0;

    uint8_t *in_buff = NULL;
    uint32_t in_len = 0;

    uint16_t byte_encoded = 0;
    uint16_t enc_len = 0;

    btif_sbc_pcm_data_t PcmEncData;
    sbc_packet_t * packet_data = NULL;
    
    out_frm_st = a2dp_encode_cp_get_emtpy_out_frame((void **)&out_buff, &out_len);
    if (out_frm_st != CP_EMPTY_OUT_FRM_OK && out_frm_st != CP_EMPTY_OUT_FRM_WORKING) {
        return 1;
    }
    
    ret = a2dp_encode_cp_get_in_frame((void **)&in_buff, &in_len);//gei in buffer
    if(ret)
    {
        return -1;
    }
    if((in_buff == NULL)||(in_len == 0))
    {
        return -1;
    }
    PcmEncData.data = (uint8_t *)in_buff;
    PcmEncData.dataLen = in_len;
    PcmEncData.numChannels = sbc_encoder.streamInfo.numChannels;
    PcmEncData.sampleFreq = sbc_encoder.streamInfo.sampleFreq;

    encode_ret = btif_sbc_encode_frames(&sbc_encoder, &PcmEncData, &byte_encoded,
                                (unsigned char*)sbc_encode_tmp, &enc_len, A2DP_CODEC_DATA_SIZE);
    ASSERT(enc_len < A2DP_CODEC_DATA_SIZE, "sbc encodec packet length too long");

    if(encode_ret==0)
    {
        packet_data = (sbc_packet_t *)out_buff;

        memcpy(packet_data->data, sbc_encode_tmp, enc_len);

        packet_data->length = enc_len;

        packet_data->sn = enc_sn++;

    }
    ret = a2dp_encode_cp_consume_in_frame();
    ASSERT(ret == 0, "%s: a2dp_cp_consume_in_frame() failed: ret=%d", __func__, ret);

    ret = a2dp_encode_cp_consume_emtpy_out_frame();
    ASSERT(ret == 0, "%s: a2dp_cp_consume_emtpy_out_frame() failed: ret=%d", __func__, ret);

    return 0;
}


int a2dp_encode_sbc_cp_init(void)
{
    int ret;
    a2dp_encode_cp_heap_init();

    ret = a2dp_encode_cp_init(a2dp_cp_sbc_encode, CP_PROC_DELAY_2_FRAMES);

    ASSERT_A2DP_ENCODER(ret == 0, "%s: a2dp_encode_cp_init() failed: ret=%d", __func__, ret);

    uint32_t cnt=0;
    while(a2dp_encode_is_cp_init_done() == false) {
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

    ret = a2dp_encode_cp_frame_init(A2DP_CODEC_DATA_SIZE, 10);//sbc encode len 575

    if (ret){
        TRACE(1,"[MCU]cp_decoder_init failed: ret=%d", ret);
        a2dp_encode_set_cp_reset_flag(true);
        return -1;
    }

    if (need_init_encoder) {
        btif_sbc_init_encoder(&sbc_encoder);
        sbc_encoder.streamInfo.channelMode = BTIF_SBC_CHNL_MODE_JOINT_STEREO;
        sbc_encoder.streamInfo.bitPool     = A2DP_SBC_BITPOOL;
        sbc_encoder.streamInfo.allocMethod = BTIF_SBC_ALLOC_METHOD_SNR;
        sbc_encoder.streamInfo.numBlocks   = 16;
        sbc_encoder.streamInfo.numSubBands = 8;
        sbc_encoder.streamInfo.mSbcFlag    = 0;
        need_init_encoder = 0;
    }

#if BT_SOURCE_DEVICE_NUM > 1
    sbc_encoder.streamInfo.numChannels = 2;
    sbc_encoder.streamInfo.sampleFreq = app_a2dp_source_samplerate_2_sbcenc_type(AUD_SAMPRATE_44100);
#else

    uint8_t device_id = app_bt_source_get_streaming_a2dp();
    
    if (device_id == BT_SOURCE_DEVICE_INVALID_ID)
    {
        return false;
    }

    struct BT_SOURCE_DEVICE_T *curr_device = NULL;

    curr_device = app_bt_source_get_device(device_id);

    sbc_encoder.streamInfo.numChannels = curr_device->base_device->a2dp_channel_num;
    sbc_encoder.streamInfo.sampleFreq = app_a2dp_source_samplerate_2_sbcenc_type(curr_device->aud_sample_rate);
#endif
    return 0;       
}



void a2dp_encode_sbc_cp_deinit(void)
{
    a2dp_encode_cp_deinit();
}


bool a2dp_source_cp_encode_sbc_packet(a2dp_source_packet_t *source_packet)//save in data to in cache
{
    int ret;
    uint8_t *out_buffer = NULL;
    uint32_t out_data_len = 0;
    int lock = 0;
    sbc_packet_t * packet_data;

    lock = int_lock();

    btif_a2dp_sbc_packet_t *packet = &source_packet->packet;

    uint8_t *in_data = NULL;
    uint16_t in_data_len = A2DP_SBC_TRANS_SIZE;

    uint32_t frame_size = 512;
    uint32_t frame_num = A2DP_SBC_TRANS_SIZE/frame_size;

    in_data = a2dp_source_sbc_frame_buffer();
    
    ret = a2dp_encode_cp_put_in_frame((void *)in_data, in_data_len); //save line data to CP IN CACHE
    if(ret!=0)
    {
        TRACE(1,"[MCU][SBC] piff !!!!!!ret: %d ", ret);
    }

    ret = a2dp_encode_cp_get_full_out_frame((void **)&out_buffer, &out_data_len);
    if(ret)
    {
        TRACE(0,"[MCU][SBC ENC] cp cache underflow");
        int_unlock(lock);
        return 0;
    }
    
    if (out_data_len == 0) {
        TRACE(0,"[MCU][SBC ENC]  olz!!!");
        packet->dataLen = 0;
        packet->frameSize = 0;
        a2dp_encode_cp_consume_full_out_frame();
        int_unlock(lock);
        return 0;
    }
    packet_data = (sbc_packet_t *)out_buffer;

    memcpy(packet->data, packet_data->data, packet_data->length);

    packet->dataLen = packet_data->length;
    packet->frameSize = packet_data->length/frame_num;
    source_packet->codec_type = BT_A2DP_CODEC_TYPE_SBC;
    ret=a2dp_encode_cp_consume_full_out_frame();
    
    if(ret){
        TRACE(0,"[MCU][SBC ENC] cp_consume_full_out_frame failed");
        a2dp_encode_set_cp_reset_flag(true);
        int_unlock(lock);
        return 0;
    }
    int_unlock(lock);
    return 1;
}

#endif /* A2DP_ENCODE_CP_ACCEL */
