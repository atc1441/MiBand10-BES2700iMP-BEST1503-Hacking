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
#ifdef  A2DP_SOURCE_LDAC_ON
#include "ldacBT.h"
#include "bes_mem_api.h"
#include "app_source_codec.h"
#include "a2dp_codec_ldac.h"
#ifdef A2DP_ENCODER_CROSS_CORE
#include "a2dp_encoder_cc_bth.h"
#endif

#include "hal_location.h"

static bool ldac_init_flag = false;
static HANDLE_LDAC_BT hData = 0;
static char a2dp_ldac_transmit_buffer[A2DP_LDAC_TRANS_SIZE*3];
uint8_t* a2dp_source_ldac_frame_buffer(void)
{
    return (uint8_t*)a2dp_ldac_transmit_buffer;
}

#ifndef A2DP_ENCODER_CROSS_CORE
static bool a2dp_source_mcu_encode_ldac_packet(a2dp_source_packet_t *source_packet)
{
    //todo
    int frame_num;
    uint8_t ret = 0;
    btif_a2dp_sbc_packet_t *packet = &source_packet->packet;
    int used_pcm_size = 0;
    ret = ldacBT_encode(hData, a2dp_ldac_transmit_buffer, &used_pcm_size, (unsigned char *)packet->data,
        (int*)&packet->dataLen, &frame_num);

    if (ret) {
        TRACE(0, "[ldac] LDAC encode error: %d", ret);
        return false;
    } else {
        TRACE(0, "encode_size: %d, %d, %d", packet->dataLen, used_pcm_size,frame_num);
    }
    return true;
}
#endif

bool a2dp_source_encode_ldac_packet(a2dp_source_packet_t *source_packet)
{
#ifndef A2DP_ENCODER_CROSS_CORE
    //LDAC init
    if (!ldac_init_flag) {
        ldac_cc_mem_init();
        if (hData) ldacBT_free_handle(hData);
        hData = ldacBT_get_handle();
        if(!hData)
        {
            TRACE(0, "Error: Can not Get LDAC Handle!");
            return false;
        }
    }
    return a2dp_source_mcu_encode_ldac_packet(source_packet);
#else
    return a2dp_encoder_bth_fetch_encoded_data(source_packet);
#endif
}

void a2dp_source_ldac_encoder_init(void)
{
    ldac_cc_mem_init();
    if (hData) ldacBT_free_handle(hData);
    hData = ldacBT_get_handle();
    if(!hData)
    {
        TRACE(0, "Error: Can not Get LDAC Handle!");
        return;
    }

    /* Initialize for Encoding */
    int ret = ldacBT_init_handle_encode(hData, 679, 0, 1, LDACBT_SMPL_FMT_S32, 96000);
    if(ret)
    {
        TRACE(0, "Initializing LDAC Handle for analysis!");
        return;
    }
    ldac_init_flag = true;
    TRACE(0, "Initializing LDAC done, %d, %d", 0, 1);

    return;
}

void a2dp_source_ldac_encoder_deinit(void)
{
    if (hData) ldacBT_free_handle(hData);
    hData = 0;
    TRACE(1,"%s",__func__);
}

static btif_avdtp_codec_t a2dp_source_ldac_avdtpcodec;

static const unsigned char a2dp_source_codec_ldac_elements[A2DP_LDAC_OCTET_NUMBER] =
{
    0x2d, 0x01, 0x00, 0x00, //Vendor ID
    0xaa, 0x00,     //Codec ID
    (A2DP_LDAC_SR_96000|A2DP_LDAC_SR_48000 |A2DP_LDAC_SR_44100),
//    (A2DP_LDAC_SR_48000 |A2DP_LDAC_SR_44100),
    (A2DP_LDAC_CM_MONO|A2DP_LDAC_CM_DUAL|A2DP_LDAC_CM_STEREO),
};

void a2dp_source_register_ldac_codec(btif_a2dp_stream_t *btif_a2dp, btif_avdtp_content_prot_t *sep_cp, uint8_t sep_priority, btif_a2dp_callback callback)
{
    a2dp_source_ldac_avdtpcodec.codecType = BT_A2DP_CODEC_TYPE_NON_A2DP;
    a2dp_source_ldac_avdtpcodec.discoverable = 1;
    a2dp_source_ldac_avdtpcodec.elements = (U8 *)&a2dp_source_codec_ldac_elements;
    a2dp_source_ldac_avdtpcodec.elemLen  = sizeof(a2dp_source_codec_ldac_elements);

    btif_a2dp_register(btif_a2dp, BTIF_A2DP_STREAM_TYPE_SOURCE, &a2dp_source_ldac_avdtpcodec, sep_cp, sep_priority, callback);
}
#endif /* A2DP_SOURCE_LDAC_ON */
#endif /* BT_A2DP_SUPPORT */