/**
 *
 * @copyright Copyright (c) 2015-2022 BES Technic.
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
 */

#ifndef __AOB_CODEC_CC_BTH_H__
#define __AOB_CODEC_CC_BTH_H__

/*****************************header include********************************/
#include "cqueue.h"
#include "app_utils.h"

#ifdef __cplusplus
extern "C"{
#endif

#ifdef GAF_CODEC_CROSS_CORE

/******************************private macro defination*****************************/

/***** encoder section ******/
#define GAF_ENCODER_PCM_DATA_BUFF_LIST_MAX_LENGTH 30

/***** decoder section ******/

/******************************private structure defination*****************************/

/***** encoder section ******/
typedef struct
{
    bool        is_m55_received_pcm_data;
    void*       coorespondingpcmDataPointer;
    uint32_t    maxEncodeDataSizePerFrame;
    uint16_t    pcmPacketEntryCount;
    uint16_t    encodedPacketEntryCount;
    uint32_t    cachedPacketCount;

    uint8_t     isGafReceivingJustStarted;
    uint8_t     isEncodingFirstRun;

    uint16_t    lastReceivedSeqNum;
    CQueue      cachedEncodedDataFromCoreQueue;
} GAF_ENCODER_CC_MCU_ENV_T;

/***** decoder section ******/
typedef struct
{
    uint32_t    maxPCMSizePerFrame;
    uint16_t    encodedPacketEntryCount;
    uint16_t    pcmPacketEntryCount;
    uint32_t    cachedPacketCount;

    uint8_t     isGafMobileReceivingJustStarted[2];
    uint16_t    lastMobileReceivedSeqNum[2];

    uint8_t     isGafReceivingJustStarted;
    uint16_t    lastReceivedSeqNum;
    uint32_t    lastReceivedTimeStamp;
    CQueue      cachedDecodedDataFromCoreQueue;
    bool        offCoreIsReady;
} GAF_DECODER_CC_MCU_ENV_T;

/******************************extern variable defination*****************************/

/***** encoder section ******/
extern GAF_ENCODER_CC_MCU_ENV_T gaf_encoder_cc_mcu_env;

/***** decoder section ******/
extern GAF_DECODER_CC_MCU_ENV_T gaf_decoder_cc_mcu_env;
extern GAF_AUDIO_STREAM_ENV_T* pLocalCaptureStreamEnvPtr;
extern GAF_AUDIO_STREAM_ENV_T* pLocalCaptureBisStreamEnvPtr;
#ifdef AOB_MOBILE_ENABLED
extern GAF_AUDIO_STREAM_ENV_T* pLocalMobileCaptureStreamEnvPtr;
#endif

/******************************global function defination*****************************/

/***** encoder section ******/
bool gaf_encoder_mcu_feed_pcm_data_into_off_m55_core(uint32_t timeStampToFill, uint16_t frame_size, uint32_t inputDataLength,
                                                                uint8_t* input, void* correspondingPcmptr);
bool gaf_encoder_core_fetch_encoded_data(void* _pStreamEnv, uint8_t* ptrData, uint16_t dataLen, uint32_t currentDmaIrqTimeUs);

/***** decoder section ******/
#ifdef AOB_MOBILE_ENABLED
GAF_DECODER_CC_MCU_ENV_T* gaf_mobile_m55_get_decoder_cc_env(void);
void _gaf_mobile_audio_bth_encoder_received_deinit_signal_from_m55(uint8_t con_id);
void _gaf_mobile_audio_bth_decoder_received_deinit_signal_from_m55(uint8_t con_id);
#endif

void gaf_encoder_core_reset(void);

void gaf_decoder_core_reset(void);

void gaf_decoder_mcu_remove_specfic_frame(void* frameaddr, uint16_t seq_num);

void _gaf_audio_bth_received_decoder_deinit_signal_from_m55(uint8_t con_id, uint32_t context_type);

void _gaf_audio_bth_received_encoder_deinit_signal_from_m55(uint8_t con_id, uint32_t context_type);

void _gaf_audio_bis_bth_decoder_received_deinit_signal_from_m55(uint8_t con_id, uint32_t context_type);

void _gaf_audio_bis_bth_encoder_received_deinit_signal_from_m55(uint8_t con_id, uint32_t context_type);
bool gaf_decoder_mcu_feed_encoded_data_into_off_m55_core(
    uint8_t* list_data,
    uint32_t time_stamp, uint16_t seq_num,
    uint16_t data_len, uint8_t* pData, bool plc);
#ifdef LOW_LATENCY_TEST
bool gaf_decoder_mcu_feed_low_latency_encoded_data_into_off_m55_core(
    uint8_t* list_data, uint32_t test_numb,
    uint32_t time_stamp, uint16_t seq_num,
    uint16_t data_len, uint8_t* pData, bool plc);
#endif

#ifdef AOB_MOBILE_ENABLED
bool gaf_mobile_decoder_mcu_feed_encoded_data_into_off_m55_core(
    uint8_t* list_data,
    uint32_t time_stamp, uint16_t seq_num,
    uint16_t data_len, uint8_t* pData, uint8_t cis_channel, bool plc);
#endif

bool gaf_decoder_core_fetch_pcm_data(
    void* _pStreamEnv, uint16_t expectedSeqNum,
    uint8_t* ptrData, uint16_t dataLen, uint32_t dmaIrqHappeningTimeUs);

bool gaf_bis_decoder_core_fetch_pcm_data(
    void* _pStreamEnv, uint16_t expectedSeqNum,
    uint8_t* ptrData, uint16_t dataLen, uint32_t dmaIrqHappeningTimeUs);

void gaf_stream_packet_recover_proc(GAF_AUDIO_STREAM_ENV_T* pStreamEnv, gaf_media_data_t *header);

void gaf_decoder_cc_update_lastframe(gaf_media_data_t* pFrameHeader);
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __AOB_CODEC_CC_BTH_H__ */