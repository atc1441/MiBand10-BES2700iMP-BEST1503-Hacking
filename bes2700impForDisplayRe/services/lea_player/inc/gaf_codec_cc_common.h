/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifndef __GAF_CODEC_CC_COMMON_H__
#define __GAF_CODEC_CC_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"
#include "hal_trace.h"
#include "gaf_media_common.h"
#include "app_utils.h"

#ifdef GAF_CODEC_CROSS_CORE
#define RPC_CROSS_CORE_TASK_CMD_TO_ADD      M55_CORE_BRIDGE_TASK_COMMAND_TO_ADD
#define APP_RPC_CROSS_CORE                  APP_RPC_CORE_BTH_M55
#else
#define RPC_CROSS_CORE_TASK_CMD_TO_ADD      RPC_BTH_DSP_TASK_CMD_TO_ADD
#define APP_RPC_CROSS_CORE                  APP_RPC_CORE_BTH_DSP
#endif

#ifdef GAF_CODEC_CROSS_CORE
/**************extern global variable***************/
// When the is_support_ble_audio_mobile_m55_decode  macro value is true,
// it represents the role is ble_audio_mobile_role
extern bool is_support_ble_audio_mobile_m55_decode;
extern bool is_playback_state;
extern bool is_capture_state;

/*************global function**********************/
void gaf_codec_encoder_m55_init(void);

void gaf_codec_decoder_m55_init(void);

void gaf_audio_stream_set_playback_state(bool enable);

void gaf_audio_stream_set_capture_state(bool enable);

uint16_t gaf_encoder_cc_check_seq_num(uint16_t actualseq, uint16_t expectedseq);

uint16_t gaf_decoder_cc_check_seq_num(uint16_t actualseq, uint16_t expectedseq);

enum APP_SYSFREQ_FREQ_T gaf_audio_stream_decoder_adjust_m55_freq(uint16_t frame_size, uint8_t channel,
    uint8_t frame_ms, uint32_t sample_rate, enum APP_SYSFREQ_FREQ_T _base_freq_,
    bool is_playback, bool is_capture);

enum APP_SYSFREQ_FREQ_T gaf_audio_stream_encoder_adjust_m55_freq(uint16_t frame_size, uint8_t channel,
    uint8_t frame_ms, uint32_t sample_rate, enum APP_SYSFREQ_FREQ_T _base_freq_, bool is_playback, bool is_capture);

enum APP_SYSFREQ_FREQ_T gaf_audio_media_decoder_adjust_bth_freq(uint16_t frame_size, uint8_t channel,
    uint8_t frame_ms, uint32_t sample_rate, enum APP_SYSFREQ_FREQ_T _base_freq_);

enum APP_SYSFREQ_FREQ_T gaf_audio_flexible_adjust_bth_freq(uint16_t frame_size, uint8_t channel,
    uint8_t frame_ms, uint32_t sample_rate, enum APP_SYSFREQ_FREQ_T _base_freq_,
    bool is_playback, bool is_capture);

/************global structure**********************/
typedef struct
{
    uint32_t frame_samples;
    uint32_t list_samples;
    // encoded frame
    uint32_t encoded_frame;
    uint32_t unencoded_frame;
    uint32_t unencode_min_frames;
    uint32_t unencode_max_frames;
    // decoded frame
    uint32_t decoded_frames;
    uint32_t undecode_frames;
    uint32_t undecode_min_frames;
    uint32_t undecode_max_frames;
    uint32_t average_frames;
    uint32_t check_sum;
    GAF_AUDIO_INPUT_CONFIG_T stream_info;
} GAF_AUDIO_LASTFRAME_INFFO_T;

typedef enum
{
    // ok to process
    GAF_ENCODER_SEQ_CHECK_RESULT_MATCH                 = 1 << 0,
    // fill fake encoding frame or do plc decoding
    GAF_ENCODER_SEQ_CHECK_RESULT_LATER_THAN_EXPECTED   = 1 << 1,
    // discard
    GAF_ENCODER_SEQ_CHECK_RESULT_EARLIER_THAN_EXPECTED = 1 << 2,
    // retrigger
    GAF_ENCODER_SEQ_CHECK_RESULT_DIFF_EXCEEDS_LIMIT    = 1 << 3,
} GAF_ENCODER_SEQ_CHECK_RESULT_T;

typedef struct
{
    int32_t  sample_rate;
    uint8_t  frame_dms;
    uint16_t frame_size;
    uint8_t  channels;
    uint8_t  bitwidth;
    int32_t  bitrate;
} GAF_AUDIO_CC_ENCODE_INPUT_T;

typedef struct
{
    uint16_t                    maxEncodedDataSizePerFrame;
    bool                        stream_playback_state;
    bool                        stream_capture_state;
    bool                        is_support_audio_mobile;
    GAF_AUDIO_CC_ENCODE_INPUT_T stream_cfg;
    GAF_AUDIO_TX_ALGO_CFG_T     tx_algo_cfg;
} GAF_ENCODER_INIT_REQ_T;

typedef struct
{
    uint8_t* flushFrame;
    uint16_t sequenceNumber;
} GAF_ENCODER_CC_FLUSH_T;

typedef struct
{
    // for removing the entry from list after receiving the ack from peer core
    uint8_t*        org_addr;
    void*           coorespondingPcmDataPacketPtr;
    // time stamp
    uint32_t        time_stamp;
    uint16_t        data_len;
    uint8_t         *data;
    uint16_t        frame_size;
} GAF_ENCODER_CC_MEDIA_DATA_T;

typedef struct {
    uint8_t   *pcm_data_buff;
    uint32_t  pcm_data_length;
    uint32_t  dmaIrqTimeUs;
} GAF_ENCODER_PCM_DATA_T;

typedef void (*GAF_AUDIO_ENCODER_CC_INIT)(void);
typedef void (*GAF_AUDIO_ENCODER_CC_DEINIT)(void);
typedef void (*GAF_AUDIO_ENCODER_CC_PCM_FRAME)(uint32_t, uint8_t*, uint16_t, uint8_t*);
typedef void (*GAF_AUDIO_ENCODER_CC_CLEAR_BUFFER_POINTER)(void);

typedef struct
{
    GAF_AUDIO_CC_ENCODE_INPUT_T       stream_config;
    GAF_AUDIO_ENCODER_CC_INIT         gaf_audio_encoder_cc_init;
    GAF_AUDIO_ENCODER_CC_DEINIT       gaf_audio_encoder_cc_deinit;
    GAF_AUDIO_ENCODER_CC_PCM_FRAME    gaf_audio_encoder_cc_pcm_frame;
    GAF_AUDIO_ENCODER_CC_CLEAR_BUFFER_POINTER    gaf_audio_encoder_cc_clear_buffer_pointer;
} GAF_AUDIO_CC_ENCODER_T;

typedef struct
{
    uint32_t reserve;
} GAF_AUDIO_ENCODER_INIT_RSP_T;

typedef struct
{
    GAF_ENCODER_CC_MEDIA_DATA_T ackedDataPacket;
    uint16_t pcmPacketEntryCount;
    uint16_t encodedPacketEntryCount;
} GAF_ENCODER_ACK_FEED_PCM_DATA_REQ_T;

typedef struct {
    bool is_successed;
    void* coorespondingPcmDataPacketPtr;
} GAF_ENCODER_ACK_FEED_PCM_DATA_SUCC_REQ_T;

typedef void (*GAF_AUDIO_DECODER_CC_INIT)(void);
typedef void (*GAF_AUDIO_DECODER_CC_DEINIT)(void);
typedef int (*GAF_AUDIO_DECODER_CC_PCM_FRAME)(uint32_t , uint8_t*, uint8_t*, uint8_t);
typedef void (*GAF_AUDIO_DECODER_CC_CLEAR_BUFFER_POINTER)(void);

typedef enum
{
    // ok to process
    GAF_DECODER_SEQ_CHECK_RESULT_MATCH                 = 1 << 0,
    // fill fake encoded frame or do plc decoding
    GAF_DECODER_SEQ_CHECK_RESULT_LATER_THAN_EXPECTED   = 1 << 1,
    // discard
    GAF_DECODER_SEQ_CHECK_RESULT_EARLIER_THAN_EXPECTED = 1 << 2,
    // retrigger
    GAF_DECODER_SEQ_CHECK_RESULT_DIFF_EXCEEDS_LIMIT    = 1 << 3,
} GAF_DECODER_SEQ_CHECK_RESULT_T;

typedef enum
{
    // ok to process
    GAF_DECODER_TIME_CHECK_RESULT_MATCH_DMAIRQ        = 1 << 0,
    // fill fake encoded frame
    GAF_DECODER_TIME_CHECK_RESULT_LATER_THAN_DMAIRQ   = 1 << 1,
    // discard
    GAF_DECODER_TIME_CHECK_RESULT_EARLIER_THAN_DMAIRQ = 1 << 2,
} GAF_DECODER_TIME_CHECK_RESULT_T;

typedef struct
{
    // for removing the entry from list after receiving the ack from peer core
    uint8_t*        org_addr;
    void*           coorespondingEncodedDataPacketPtr;
    // time stamp
    uint32_t        time_stamp;
    // Packet sequeue NUmber
    uint16_t        pkt_seq_nb;

    uint8_t         channel;

    uint16_t        frame_samples;
    uint8_t         isPlc;
    uint16_t        data_len;
    uint8_t         *data;
} GAF_DECODER_CC_MEDIA_DATA_T;

typedef struct
{
    uint16_t frame_size;
    uint8_t frame_ms;
    uint32_t sample_rate;
    uint8_t num_channels;
    uint8_t bits_depth;
    int32_t bitrate;
}GAF_AUDIO_CC_OUTPUT_CONFIG_T;

typedef struct
{
    bool                           is_support_audio_mobile;
    bool                           stream_playback_state;
    bool                           stream_capture_state;
    uint16_t                       maxPcmDataSizePerFrame;
    GAF_AUDIO_CC_OUTPUT_CONFIG_T   stream_cfg;
} GAF_DECODER_INIT_REQ_T;

typedef struct
{
    uint16_t frame_size;
    uint8_t frame_ms;
    int32_t sample_rate;
    int8_t channels;
    int8_t bitwidth;
    int32_t bitrate;
} GAF_AUDIO_CC_DECODE_OUTPUT_T;

typedef struct
{
    GAF_AUDIO_CC_DECODE_OUTPUT_T      stream_config;
    GAF_AUDIO_DECODER_CC_INIT         gaf_audio_decoder_cc_init;
    GAF_AUDIO_DECODER_CC_DEINIT       gaf_audio_decoder_cc_deinit;
    GAF_AUDIO_DECODER_CC_PCM_FRAME    gaf_audio_decoder_cc_pcm_frame;
    GAF_AUDIO_DECODER_CC_CLEAR_BUFFER_POINTER    gaf_audio_decoder_cc_clear_buffer_pointer;
} GAF_AUDIO_CC_DECODER_T;

typedef struct
{
    uint8_t* flushFrame;
    uint16_t sequenceNumber;
    uint16_t channel;
} GAF_DECODER_CC_FLUSH_T;

typedef struct
{
    uint32_t reserve;
} GAF_AUDIO_DECODER_INIT_RSP_T;

typedef struct
{
    GAF_DECODER_CC_MEDIA_DATA_T ackedDataPacket;
    uint16_t pcmPacketEntryCount;
    uint16_t encodedPacketEntryCount;
} GAF_DECODER_ACK_FEED_PCM_DATA_REQ_T;

typedef struct
{
    bool is_bis;
    bool is_bis_src;
    bool playback_deinit;
    bool playback_deinit_sent;
    bool capture_deinit;
    bool capture_deinit_sent;
    bool is_mobile_role;
    uint8_t con_lid;
    uint32_t context_type;
} GAF_AUDIO_M55_DEINIT_T;
extern GAF_AUDIO_M55_DEINIT_T gaf_m55_deinit_status;
#endif /// GAF_CODEC_CROSS_CORE

#ifdef __cplusplus
}
#endif

#endif/// __GAF_CODEC_CC_COMMON_H__

