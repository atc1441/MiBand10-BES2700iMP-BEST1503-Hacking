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
#ifndef __APP_A2DP_SOURCE_CODEC__
#define __APP_A2DP_SOURCE_CODEC__
#include "bt_source.h"
#include "app_a2dp_source.h"
#include "app_overlay.h"
#include "cmsis.h"
#include "cmsis_os.h"
#include "audioflinger.h"
#include "lockcqueue.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "app_bt_stream.h"
#include "bt_drv_reg_op.h"
#include "app_bt.h"
#include "app_a2dp.h"
#include "app_bt_media_manager.h"
#include "app_utils.h"
#include "app_bt_func.h"
#include "app_audio.h"
#include "audio_process.h"

#define A2DP_LINEIN_SIZE (40*1024) // 40 KB

//2dh3 5frame 40 encode delay
#define A2DP_SBC_TRANS_SIZE     (128*5*2*2) // 2.5 KB
#define A2DP_SBC_BITPOOL        (51)        // frameSize 115 * 5 = packetSize 575
#define BT_A2DP_SOURCE_SBC_LINEIN_BUFF_SIZE (A2DP_SBC_TRANS_SIZE*2)

#define A2DP_AAC_TRANS_SIZE     (128*8*4)   // 4 KB
#define BT_A2DP_SOURCE_AAC_LINEIN_BUFF_SIZE (A2DP_AAC_TRANS_SIZE*2)

#define A2DP_LHDC_TRANS_SIZE    (256*2*2)
#define BT_A2DP_SOURCE_LHDC_LINEIN_BUFF_SIZE (A2DP_LHDC_TRANS_SIZE*2)

#define A2DP_LDAC_TRANS_SIZE    (128*4*2)
#define BT_A2DP_SOURCE_LDAC_LINEIN_BUFF_SIZE (A2DP_LDAC_TRANS_SIZE*2)

#define AAC_OUT_SIZE            (1024*2)
#define LHDCV5_OUT_SIZE         1753 //todo
#define LDAC_OUT_SIZE           660 //todo

/*
960: 240 * 2 * 2
duration: 240 sample within 5ms (sample freq: 48K)
sample bit:  2 BYTE
channel num: 2
*/
#define A2DP_LHDCV5_TRANS_SIZE    (960)
#define BT_A2DP_SOURCE_LHDCV5_LINEIN_BUFF_SIZE (A2DP_LHDCV5_TRANS_SIZE*2)

#if (A2DP_LINEIN_SIZE % A2DP_SBC_TRANS_SIZE) != 0
#error "A2DP_LINEIN_SIZE shall be multiple of A2DP_SBC_TRANS_SIZE"
#endif

#if (A2DP_LINEIN_SIZE % A2DP_AAC_TRANS_SIZE) != 0
#error "A2DP_LINEIN_SIZE shall be multiple of A2DP_AAC_TRANS_SIZE"
#endif

#define A2DP_CODEC_HEADER_SIZE 20
#if defined(BT_SOURCE) && defined(A2DP_SOURCE_LHDCV5_ON)
// lossless on, BR fix to 1400, duration: 5ms interval: 10ms
// the encoder out: 876(frame len) * 2(frame num)=1752,
// but the encoder will encode per 5ms while the MTU equal to A2DP_CODEC_DATA_SIZE) set to 1752
// so increase 1, the encoder encoder will encode per 10 ms
#define A2DP_CODEC_DATA_SIZE 1753
#else
#define A2DP_CODEC_DATA_SIZE 1024
#endif

typedef struct {
    struct list_node node;
    bool already_sent[BT_SOURCE_DEVICE_NUM];
    bool inuse;
    uint8_t codec_type;
    btif_a2dp_sbc_packet_t packet;
    uint8_t buffer[A2DP_CODEC_HEADER_SIZE+A2DP_CODEC_DATA_SIZE];
} a2dp_source_packet_t;

typedef struct  {
    osSemaphoreId _osSemaphoreId;
    osSemaphoreDef_t _osSemaphoreDef;
#ifdef CMSIS_OS_RTX
    uint32_t _semaphore_data[2];
#endif
} a2dp_source_lock_t;

typedef struct{
    CQueue  pcm_queue;
    osThreadId sbc_send_id;
    a2dp_source_lock_t  data_lock;
    a2dp_source_lock_t  sbc_send_lock;
    bool is_encoded_packet;
} A2DP_SOURCE_STRUCT;

#ifdef __cplusplus
extern "C" {
#endif

uint8_t *a2dp_source_sbc_frame_buffer(void);

uint8_t app_a2dp_source_samplerate_2_sbcenc_type(enum AUD_SAMPRATE_T sample_rate);

bool a2dp_source_encode_sbc_packet(a2dp_source_packet_t *source_packet);

uint8_t *a2dp_source_aac_frame_buffer(void);

bool a2dp_source_encode_aac_packet(a2dp_source_packet_t *source_packet);

bool a2dp_source_encode_lhdc_packet(a2dp_source_packet_t *source_packet);

bool a2dp_source_encode_ldac_packet(a2dp_source_packet_t *source_packet);

uint8_t *a2dp_source_lhdcv5_frame_buffer(void);

void a2dp_source_lhdcv5_enc_set_config(uint32_t sample_rate, uint32_t bits_per_sample, uint32_t br, uint32_t is_lossless_on);

bool a2dp_source_encode_lhdcv5_packet(a2dp_source_packet_t *source_packet);

int32_t a2dp_source_wait_sent(uint32_t timeout);

void a2dp_source_register_sbc_codec(btif_a2dp_stream_t *btif_a2dp, btif_avdtp_content_prot_t *sep_cp, uint8_t sep_priority, btif_a2dp_callback callback);

void a2dp_source_register_aac_codec(btif_a2dp_stream_t *btif_a2dp, btif_avdtp_content_prot_t *sep_cp, uint8_t sep_priority, btif_a2dp_callback callback);

void a2dp_source_register_lhdc_codec(btif_a2dp_stream_t *btif_a2dp, btif_avdtp_content_prot_t *sep_cp, uint8_t sep_priority, btif_a2dp_callback callback);

void a2dp_source_register_lhdcv5_codec(btif_a2dp_stream_t *btif_a2dp, btif_avdtp_content_prot_t *sep_cp, uint8_t sep_priority, btif_a2dp_callback callback);

void a2dp_source_register_ldac_codec(btif_a2dp_stream_t *btif_a2dp, btif_avdtp_content_prot_t *sep_cp, uint8_t sep_priority, btif_a2dp_callback callback);

uint8_t *a2dp_source_lhdc_frame_buffer(void);

uint8_t *a2dp_source_ldac_frame_buffer(void);

void a2dp_source_send_lhdc_packet(void);

void a2dp_source_sbc_encoder_init(void);

void a2dp_source_sbc_encoder_deinit(void);

int aacenc_deinit(void);

void a2dp_source_lhdc_encoder_init(void);

void a2dp_source_lhdc_encoder_deinit(void);

void a2dp_source_lhdcv5_encoder_init(void);

void a2dp_source_lhdcv5_encoder_deinit(void);

void a2dp_source_ldac_encoder_init(void);

void a2dp_source_ldac_encoder_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_A2DP_SOURCE_CODEC__ */

