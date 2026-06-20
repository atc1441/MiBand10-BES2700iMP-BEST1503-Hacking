/***************************************************************************
 *
 * Copyright 2023-2028 BES.
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
#ifdef  A2DP_SOURCE_LHDCV5_ON
#include "app_source_codec.h"
#include "lhdcv5_util_enc.h"
#include "a2dp_encoder_lhdc_cp.h"
#ifdef A2DP_ENCODER_CROSS_CORE
#include "a2dp_encoder_cc_bth.h"
#endif

#include "hal_location.h"

typedef struct _lhdcv5_encoder_t{
    uint32_t sample_rate;
    uint32_t bits_per_sample;
    uint32_t frame_duration;
    int32_t samples_per_frame;
    uint32_t audio_format;
    uint32_t version;
    uint32_t islossless;
    uint32_t bitrate_inx;
    uint32_t mtu;
    uint32_t interval;
    void* ptr;
}lhdcv5_encoder_t;
lhdcv5_encoder_t inst;

#define LHDCV5_MEMPOOL_SIZE (131*1024)
static uint8_t  SRAM_BSS_LOC lhdcv5_pool_buff[LHDCV5_MEMPOOL_SIZE];
static uint8_t * SRAM_DATA_LOC  source_lhdcv5_mempoll = NULL;

btif_avdtp_codec_t a2dp_source_lhdcv5_avdtpcodec;
static char a2dp_lhdcv5_transmit_buffer[A2DP_LHDCV5_TRANS_SIZE * 2];

const unsigned char a2dp_source_codec_lhdcv5_elements[A2DP_LHDCV5_OCTET_NUMBER] = {
    0x3A, 0x05, 0x00, 0x00, //Vendor ID
    0x35, 0x4c,             //Codec ID
    (A2DP_LHDCV5_SR_192000 | A2DP_LHDCV5_SR_96000 | A2DP_LHDCV5_SR_48000 | A2DP_LHDCV5_SR_44100),
    (/*A2DP_LHDCV5_FMT_32 | A2DP_LHDCV5_FMT_24 |*/ A2DP_LHDCV5_FMT_16 | A2DP_LHDCV5_MAX_BR_1000 | A2DP_LHDCV5_MIN_BR_64),
    (A2DP_LHDCV5_FRAME_5MS | A2DP_LHDCV5_VERSION_NUM),
    (A2DP_LHDCV5_HAS_AR | A2DP_LHDCV5_HAS_JAS | A2DP_LHDCV5_HAS_META | A2DP_LHDCV5_LL_MODE | A2DP_LHDCV5_LOSSLESS_MODE),
    (0x00 /*A2DP_LHDCV5_AR_ON*/)
};

static heap_handle_t lhdcv5_memhandle = NULL;
static int source_lhdcv5_meminit(void)
{
    TRACE(0, "source_lhdcv5_meminit");
    source_lhdcv5_mempoll = lhdcv5_pool_buff;

    if(lhdcv5_memhandle == NULL)
        lhdcv5_memhandle = heap_register(source_lhdcv5_mempoll, LHDCV5_MEMPOOL_SIZE);

    return 1;
}


void a2dp_source_lhdcv5_enc_set_config(uint32_t sample_rate, uint32_t bits_per_sample, uint32_t br, uint32_t is_lossless_on)
{
    /* sample rate could be 44100|48000|96000|192000 */
    TRACE(0, "a2dp_source_lhdcv5_enc_set_config SR %d  bps %d br %d lossless %d", sample_rate, bits_per_sample, br, is_lossless_on);
    inst.sample_rate = sample_rate;

    /* bits per sample could be 16|24 */
    inst.bits_per_sample = bits_per_sample;

    /* frame duration could only be 50 */
    inst.frame_duration = LHDCV5_FRAME_5MS;

    /* version could only be VERSION_5(1) */
    inst.version = LHDCV5_VERSION_1;

    /* support lossless */
    inst.islossless = is_lossless_on;

    /* samples per frame will be obtained by calling lhdcv5_util_dec_get_sample_size() later
        *
        * Initialize to 0 here
    */
    inst.samples_per_frame = 0;

    /* set bitrate switch to index*/
    switch(br)
    {
    case 64:
        inst.bitrate_inx = LHDCV5_QUALITY_LOW0;
        break;
    case 128:
        inst.bitrate_inx = LHDCV5_QUALITY_LOW1;
        break;
    case 192:
        inst.bitrate_inx = LHDCV5_QUALITY_LOW2;
        break;
    case 256:
        inst.bitrate_inx = LHDCV5_QUALITY_LOW3;
        break;
    case 320:
        inst.bitrate_inx = LHDCV5_QUALITY_LOW4;
        break;
    case 400:
        inst.bitrate_inx = LHDCV5_QUALITY_LOW;
        break;
    case 500:
        inst.bitrate_inx = LHDCV5_QUALITY_MID;
        break;
    case 900:
        inst.bitrate_inx = LHDCV5_QUALITY_HIGH;
        break;
    case 1000:
        inst.bitrate_inx = LHDCV5_QUALITY_HIGH1;
        break;
    case 1100:
        inst.bitrate_inx = LHDCV5_QUALITY_HIGH2;
        break;
    case 1200:
        inst.bitrate_inx = LHDCV5_QUALITY_HIGH3;
        break;
    case 1300:
        inst.bitrate_inx = LHDCV5_QUALITY_HIGH4;
        break;
    case 1400:
        inst.bitrate_inx = LHDCV5_QUALITY_HIGH5;
        break;
    case 2000:
        inst.bitrate_inx = LHDCV5_QUALITY_AUTO;
        break;
    default:
        inst.bitrate_inx = LHDCV5_QUALITY_INVALID;
    }
    if (inst.bitrate_inx == LHDCV5_QUALITY_INVALID)
    {
        ASSERT(0, "Quality invalid br %d\n", br);
        return;
    }

    /* set mtu 1023 */
    inst.mtu = LHDCV5_MTU_4MBPS;

    /* set interval*/
    inst.interval = LHDCV5_ENC_INTERVAL_10MS;

    /* ptr is the pointer that point to the allocated memory needed by LHDC5.0 decoder
        * It will be obtained by calling malloc() or other specific memory allocation function later
        *
        * Initialize to NULL here
        */
    inst.ptr = NULL;
}

//#define LHDC_FRAME_BYTE_SIZE (LHDCV3_BLOCK_SIZE * sizeof(int16_t) * 2)//256 sample
//static uint8_t fake_sample[LHDC_FRAME_BYTE_SIZE];

uint8_t *a2dp_source_lhdcv5_frame_buffer(void)
{
    return (uint8_t *)a2dp_lhdcv5_transmit_buffer;
}

bool a2dp_source_mcu_encode_lhdcv5_packet(a2dp_source_packet_t *source_packet)
{
    uint8_t streaming_a2dp_id = app_bt_source_get_streaming_a2dp();

    btif_a2dp_sbc_packet_t *lhdcv5_Packet = &source_packet->packet;

    //TRACE(0,"%s TRANS_SIZE %d DATA_SIZE %d", __func__, A2DP_LHDCV5_TRANS_SIZE, A2DP_CODEC_DATA_SIZE);

    if (streaming_a2dp_id == BT_SOURCE_DEVICE_INVALID_ID)
    {
        TRACE(0, "No streaming a2dp id");
        return false;
    }

    uint32_t written = 0;
    uint32_t out_frames = 0;
    int32_t res = lhdcv5_util_enc_process(  inst.ptr,
                                            a2dp_lhdcv5_transmit_buffer,
                                            A2DP_LHDCV5_TRANS_SIZE,
                                            lhdcv5_Packet->data,
                                            A2DP_CODEC_DATA_SIZE,
                                            &written,
                                            &out_frames);
    if (res != LHDCV5_FRET_SUCCESS)
    {
        TRACE(0, "%s res %d", __func__, res);
        return false;
    }
    ASSERT(written < A2DP_CODEC_DATA_SIZE, "lhdc encodec packet length too long");

    if (written && out_frames)
    {
        
        lhdcv5_Packet->dataLen = written;
        lhdcv5_Packet->frameSize = written/out_frames;
        lhdcv5_Packet->frameNum = out_frames;
        //TRACE(1,"%s dL %d fS %d w %d out %d", __func__, lhdcv5_Packet->dataLen, lhdcv5_Packet->frameSize, written, out_frames);
        source_packet->codec_type = BT_A2DP_CODEC_TYPE_NON_A2DP;
        return true;
    }
    else
    {
        //TRACE(1,"123%s encode time(us)= %d", __func__, FAST_TICKS_TO_US(ticks_new - ticks_old));
        //TRACE(0, "written %d out_frames %d", written, out_frames);
        return false;
    }
}

bool a2dp_source_encode_lhdcv5_packet(a2dp_source_packet_t *source_packet)
{
#ifndef A2DP_ENCODER_CROSS_CORE
    if (lhdcv5_memhandle == NULL || app_get_current_overlay() != APP_OVERLAY_A2DP_LHDC_V5_ENCODER)
    {
        TRACE(0, "lhdcv5_memhandle NULL need init");
        uint8_t device_id = app_bt_source_get_current_a2dp();
        struct BT_SOURCE_DEVICE_T *curr_device = app_bt_source_get_device(device_id);
        if (curr_device->base_device->codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP && curr_device->base_device->a2dp_non_type == A2DP_SOURCE_NON_TYPE_LHDCV5)
        {
            //TRACE(0, "aud_sample_rate %d sample_bit %d lossless %d", curr_device->aud_sample_rate, curr_device->base_device->sample_bit, curr_device->is_lossless_on);
            app_overlay_select(APP_OVERLAY_A2DP_LHDC_V5_ENCODER);
            uint32_t bit_rate = 2000;
            if (curr_device->is_lossless_on)
            {
                bit_rate = 1400;
            }
            a2dp_source_lhdcv5_enc_set_config(curr_device->aud_sample_rate, curr_device->base_device->sample_bit, bit_rate, curr_device->is_lossless_on);
            a2dp_source_lhdcv5_encoder_init();
        }
    }
#endif

#if defined(A2DP_ENCODE_CP_ACCEL)
    return a2dp_source_cp_encode_lhdc_packet(source_packet);
#elif defined(A2DP_ENCODER_CROSS_CORE)
    //TRACE(1, "%s, %d", __func__, __LINE__);
    return a2dp_encoder_bth_fetch_encoded_data(source_packet);
#else
    return a2dp_source_mcu_encode_lhdcv5_packet(source_packet);
#endif
}

void a2dp_source_lhdcv5_encoder_init(void)
{
    int32_t     func_ret = LHDCV5_FRET_SUCCESS;
    uint32_t    mem_req_bytes = 0;

    TRACE(1, "%s version %d", __func__, inst.version);
    func_ret = lhdcv5_util_enc_get_mem_req (inst.version, &mem_req_bytes);
    TRACE (0, "%s: lhdcv5_util_get_mem_req (%d) mem_req_bytes %d\n", __func__, func_ret, mem_req_bytes);

    if (lhdcv5_memhandle == NULL)
    {
        source_lhdcv5_meminit();
    }
    inst.ptr = source_lhdcv5_mempoll;
    if (inst.ptr == NULL)
    {
        TRACE (0, "malloc() err\n");
        return;
    }

    uint32_t i = 0;
    for (i = 0; i < LHDCV5_MEMPOOL_SIZE; i++)
    {
        source_lhdcv5_mempoll[i] = 1;
    }

    TRACE(0, "write 1 over");
    for (i = 0; i < LHDCV5_MEMPOOL_SIZE; i++)
    {
        source_lhdcv5_mempoll[i] = 0;
    }
    TRACE(0, "write 0 over");

    func_ret = lhdcv5_util_enc_get_handle (inst.version, inst.ptr, mem_req_bytes);
    TRACE (0, "%s: lhdcv5_util_enc_get_handle (%d)\n", __func__, func_ret);

    TRACE(0, "ptr %p sr %d bits_per_sample %d frm_drt %d mtu %d interval %d is_ll %d",
            inst.ptr,
            inst.sample_rate,
            inst.bits_per_sample,
            inst.frame_duration,
            inst.mtu,
            inst.interval,
            inst.islossless);

    func_ret = lhdcv5_util_init_encoder (inst.ptr,
                                            inst.sample_rate,
                                            inst.bits_per_sample,
                                            inst.bitrate_inx,
                                            inst.frame_duration,
                                            inst.mtu,
                                            inst.interval,
                                            inst.islossless);

    TRACE (0, "%s: lhdcv5_util_init_encoder (%d)\n", __func__,func_ret);

    unsigned int hp_tmp=0;
    lhdcv5_util_enc_set_target_bitrate_inx(inst.ptr, inst.bitrate_inx, &hp_tmp, 1);
    TRACE (0, "%s: bitrate_inx %d hp_tmp %d", __func__, inst.bitrate_inx, hp_tmp);
}

void a2dp_source_lhdcv5_encoder_deinit(void)
{
    TRACE(1,"%s",__func__);
#if defined(A2DP_ENCODE_CP_ACCEL)
    a2dp_encode_lhdc_cp_deinit();
#else
    if (lhdcv5_memhandle != NULL)
    {
        lhdcv5_memhandle = NULL;
    }
#endif
}

void a2dp_source_register_lhdcv5_codec(btif_a2dp_stream_t *btif_a2dp, btif_avdtp_content_prot_t *sep_cp, uint8_t sep_priority, btif_a2dp_callback callback)
{
    TRACE(0, "a2dp_source_register_lhdcv5_codec");
    a2dp_source_lhdcv5_avdtpcodec.codecType = BT_A2DP_CODEC_TYPE_NON_A2DP;
    a2dp_source_lhdcv5_avdtpcodec.discoverable = 1;
    a2dp_source_lhdcv5_avdtpcodec.elements = (U8 *)&a2dp_source_codec_lhdcv5_elements;
    a2dp_source_lhdcv5_avdtpcodec.elemLen  = sizeof(a2dp_source_codec_lhdcv5_elements);

    btif_a2dp_register(btif_a2dp, BTIF_A2DP_STREAM_TYPE_SOURCE, &a2dp_source_lhdcv5_avdtpcodec, sep_cp, sep_priority, callback);
}

#endif /* A2DP_SOURCE_LHDCV5_ON */
#endif /* BT_A2DP_SUPPORT */