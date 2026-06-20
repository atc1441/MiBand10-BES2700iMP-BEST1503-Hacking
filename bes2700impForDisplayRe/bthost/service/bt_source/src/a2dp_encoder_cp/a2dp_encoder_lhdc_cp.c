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
#include "lhdc_enc_api.h"
#include "a2dp_encoder.h"
#include "a2dp_encoder_lhdc_cp.h"
#include "a2dp_encoder_cp.h"

static bool need_init_encoder = true;
static uint32_t enc_sn = 0;
static uint8_t encode_tmp[600];
extern bool one_frame_per_chennal;
//static btif_sbc_encoder_t sbc_encoder;
typedef struct 
{
    uint32_t sn;
    uint32_t length;
    uint8_t data[0];
} lhdc_packet_t;

TEXT_LHDC_LICE_LOC const uint8_t license_key[256];

TEXT_LHDC_ENC_LOC
int a2dp_cp_lhdc_encode(void)
{
    enum CP_EMPTY_OUT_FRM_T out_frm_st;
    int ret;
    uint8_t *out_buff = NULL;
    uint32_t out_len = 0;

    uint8_t *in_buff = NULL;
    uint32_t in_len = 0;

    uint32_t out_frames = 0;
    uint32_t enc_len = 0;

    lhdc_packet_t * packet_data = NULL;
    
    ret = a2dp_encode_cp_get_in_frame((void **)&in_buff, &in_len);//gei in buffer

    if(ret)
    {
        return -1;
    }
    if((in_buff == NULL)||(in_len == 0))
    {
        return -1;
    }

    lhdc_enc_result_t res = lhdc_enc_encode(in_buff, encode_tmp, &enc_len, &out_frames);

    ret = a2dp_encode_cp_consume_in_frame();
    ASSERT(ret == 0, "%s: a2dp_cp_consume_in_frame() failed: ret=%d", __func__, ret);

    if (res == LHDC_NO_ERR && enc_len && out_frames)
    {
        out_frm_st = a2dp_encode_cp_get_emtpy_out_frame((void **)&out_buff, &out_len);
        if (out_frm_st != CP_EMPTY_OUT_FRM_OK && out_frm_st != CP_EMPTY_OUT_FRM_WORKING) {
            return 1;
        }

        ASSERT(enc_len < A2DP_CODEC_DATA_SIZE, "lhdc encodec packet length too long");

        ASSERT(out_frames == 2, "lhdc encodec frame number incorrect");

        packet_data = (lhdc_packet_t *)out_buff;

        memcpy(packet_data->data, encode_tmp, enc_len);

        packet_data->length = enc_len;

        packet_data->sn = enc_sn++;
    
        //TRACE(2, "Encode: packet info sn:%d, length:%d", packet_data->sn, packet_data->length);

        ret = a2dp_encode_cp_consume_emtpy_out_frame();
        ASSERT(ret == 0, "%s: a2dp_cp_consume_emtpy_out_frame() failed: ret=%d", __func__, ret);
    }


    return 0;
}


int a2dp_encode_lhdc_cp_init(void)
{
    int ret;
    struct BT_SOURCE_DEVICE_T *curr_device = NULL;
    a2dp_encode_cp_heap_init();

    ret = a2dp_encode_cp_init(a2dp_cp_lhdc_encode, CP_PROC_DELAY_2_FRAMES);

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

    ret = a2dp_encode_cp_frame_init(600 + sizeof(lhdc_packet_t), 10);//sbc encode len 575

    if (ret){
        TRACE(1,"[MCU]cp_decoder_init failed: ret=%d", ret);
        a2dp_encode_set_cp_reset_flag(true);
        return -1;
    }

    if (need_init_encoder) 
    {
        uint8_t device_id = app_bt_source_get_streaming_a2dp();

        if (device_id == BT_SOURCE_DEVICE_INVALID_ID)
        {
            return false;
        }

        curr_device = app_bt_source_get_device(device_id);

        TRACE(1, "%s: lhdc_enc_init In", __func__);
#if defined(BT_MULTI_SOURCE)
        lhdc_enc_init(curr_device->aud_sample_rate, curr_device->base_device->sample_bit, 350, 600, 10, one_frame_per_chennal);
#else
        lhdc_enc_init(curr_device->aud_sample_rate, curr_device->base_device->sample_bit, 400, 600, 10, one_frame_per_chennal);
#endif
        TRACE(1, "%s: lhdc_enc_init Out", __func__);
        lhdc_enc_verify_license();
        TRACE(1, "%s: lhdc_enc_verify_license Out", __func__);

        //need_init_encoder = false;
    }

    return 0;       
}



void a2dp_encode_lhdc_cp_deinit(void)
{
    a2dp_source_lhdc_encoder_init();

    a2dp_encode_cp_deinit();

    need_init_encoder = true;

    TRACE(1, "%s", __func__);
}


bool a2dp_source_cp_encode_lhdc_packet(a2dp_source_packet_t *source_packet)//save in data to in cache
{
    int ret;
    uint8_t *out_buffer = NULL;
    uint32_t out_data_len = 0;
    int lock = 0;
    lhdc_packet_t * packet_data;

    lock = int_lock();

    btif_a2dp_sbc_packet_t *packet = &source_packet->packet;

    uint8_t *in_data = NULL;
    uint16_t in_data_len = A2DP_LHDC_TRANS_SIZE ;

    uint32_t frame_size = 1024;
    uint32_t frame_num = A2DP_LHDC_TRANS_SIZE/frame_size;

    in_data = a2dp_source_lhdc_frame_buffer();
    
    ret = a2dp_encode_cp_put_in_frame((void *)in_data, in_data_len); //save line data to CP IN CACHE
    if(ret!=0)
    {
        TRACE(1,"[MCU][LHDC] piff !!!!!!ret: %d ", ret);
    }

    ret = a2dp_encode_cp_get_full_out_frame((void **)&out_buffer, &out_data_len);

    if(ret)
    {
        //TRACE(0,"[MCU][LHDC ENC] cp cache underflow");
        int_unlock(lock);
        return 0;
    }
    
    if (out_data_len == 0) {
        TRACE(0,"[MCU][LHDC ENC]  olz!!!");
        packet->dataLen = 0;
        packet->frameSize = 0;
        a2dp_encode_cp_consume_full_out_frame();
        int_unlock(lock);
        return 0;
    }

    packet_data = (lhdc_packet_t *)out_buffer;

    //TRACE(2, "Send: packet info sn:%d, length:%d", packet_data->sn, packet_data->length);

    memcpy(packet->data, packet_data->data, packet_data->length);

    packet->dataLen = packet_data->length;
    packet->frameSize = packet_data->length/frame_num;
    source_packet->codec_type = BT_A2DP_CODEC_TYPE_NON_A2DP;
    ret=a2dp_encode_cp_consume_full_out_frame();
    
    if(ret){
        TRACE(0,"[MCU][LHDC ENC] cp_consume_full_out_frame failed");
        a2dp_encode_set_cp_reset_flag(true);
        int_unlock(lock);
        return 0;
    }
    int_unlock(lock);
    return 1;
}

#endif /* A2DP_ENCODE_CP_ACCEL */
