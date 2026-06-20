/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#pragma once
#ifndef __LC3_API_PROCESS_H__
#define __LC3_API_PROCESS_H__
#include <stdint.h>

#ifndef EXTERNC
#ifdef __cplusplus
#define EXTERNC extern "C" 
#else
#define EXTERNC
#endif
#endif

/*! Maximum number of samples per channel that can be stored in one LC3 frame. */
#define LC3_CODEC_MAX_SAMPLES 480

/*! Maximum number of bytes of one LC3 frame. */
#define LC3_CODEC_MAX_BYTES 870


/*! Decoder packet loss concealment mode */
typedef enum
{
    LC3_API_PLC_STANDARD = 0, /*!< Less complex than advanced method */
    LC3_API_PLC_MUSICPLC = 1,
    LC3_API_PLC_ADVANCED = 3  /*!< Enhanced concealment method */
} LC3_API_PlcMode;

/*! Error protection mode. LC3_EP_ZERO differs to LC3_EP_OFF in that
 *  errors can be detected but not corrected. */
typedef enum
{
    LC3_API_EP_OFF = 0, /*!< Error protection is disabled */
    LC3_API_EP_ZERO = 1, /*!< Error protection with 0 bit correction */
    LC3_API_EP_LOW = 2, /*!< Error protection correcting one symbol per codeword */
    LC3_API_EP_MEDIUM = 3, /*!< Error protection correcting two symbols per codeword */
    LC3_API_EP_HIGH = 4  /*!< Error protection correcting three symbols per codeword */
} LC3_API_EpMode;

/*! Error protection mode request. On the encoder sidem, LC3_EPMR_ZERO to LC3_EPMR_HIGH
 *  can be set. The decoder returns mode requests with different confidences. */
typedef enum
{
    LC3_API_EPMR_ZERO = 0,  /*!< Request no error correction. High confidence if returned by decoder. */
    LC3_API_EPMR_LOW = 1,  /*!< Request low error correction. High confidence if returned by decoder. */
    LC3_API_EPMR_MEDIUM = 2,  /*!< Request medium error correction. High confidence if returned by decoder. */
    LC3_API_EPMR_HIGH = 3,  /*!< Request high error correction. High confidence if returned by decoder. */
    LC3_API_EPMR_ZERO_MC = 4,  /*!< No error correction requested, medium confidence. */
    LC3_API_EPMR_LOW_MC = 5,  /*!< Low error correction requested, medium confidence. */
    LC3_API_EPMR_MEDIUM_MC = 6,  /*!< Medium error correction requested, medium confidence. */
    LC3_API_EPMR_HIGH_MC = 7,  /*!< High error correction requested, medium confidence. */
    LC3_API_EPMR_ZERO_NC = 8,  /*!< No error correction requested, unvalidated. */
    LC3_API_EPMR_LOW_NC = 9,  /*!< Low error correction requested, unvalidated. */
    LC3_API_EPMR_MEDIUM_NC = 10, /*!< Medium error correction requested, unvalidated. */
    LC3_API_EPMR_HIGH_NC = 11  /*!< High error correction requested, unvalidated. */
} LC3_API_EpModeRequest;

typedef enum
{
    LC3_API_OK                  = 0,  /*!< No error occurred */
    LC3_API_ERROR               = -1,  /*!< Function call failed */
} LC3_API_Error;

typedef struct LC3_Info
{
    void* instance;
    void* scratch;
    uint16_t instance_size;
    uint16_t scratch_size;
    //
    int32_t sample_rate;
    //
    uint8_t channels;
    uint8_t bitwidth;
    uint8_t bitalign;
    union{
        uint8_t flags;
        struct{
            uint8_t is_interlaced : 1;
            uint8_t is_lc3plus : 1;
            uint8_t is_encode : 1;
            uint8_t is_hrmode : 1;
        };
    };
    //
    uint8_t frame_dms;//0.1ms
    uint8_t epmode;//LC3_API_EpMode
    uint8_t plcMeth;//LC3_API_PlcMode
    uint8_t epmr;///[lc3+]LC3_API_EpModeRequest
    //
    uint32_t bitrate;
    uint32_t bandwidth;
    //
    uint16_t frame_samples;
    uint16_t frame_size;
    //
    //function pointer 
    int32_t (*cb_uninit)(struct LC3_Info* instance);
    union {
        struct {
            int32_t(*cb_encode)(struct LC3_Info* instance, void* scratch, void** input_samples, void* output_bytes, int32_t output_size);
            int32_t(*cb_encode_interlaced)(struct LC3_Info* instance, void* scratch, void* input_samples, void* output_bytes, int32_t output_size);
        };
        struct {
            int32_t(*cb_decode)(struct LC3_Info* instance, void* scratch, void* input_bytes, int32_t num_bytes, void** output_samples, int32_t bfi_ext);
            int32_t(*cb_decode_interlaced)(struct LC3_Info* instance, void* scratch, void* input_bytes, int32_t num_bytes, void* output_samples, int32_t bfi_ext);
        };
    };
    //
    void* pool;
    void* (*cb_alloc)(void* pool,unsigned size);
    void (*cb_free)(void* pool, void* ptr);
    //
    void (*cb_overlay)(struct LC3_Info* instance);
    //
    void* enabled_frame_buff;
    //
    uint32_t quan_prec;
    uint32_t ltpf_flag;
    //
    uint32_t extended_instance_size;
}LC3_Info;
typedef LC3_Info LC3_Dec_Info;
typedef LC3_Info LC3_Enc_Info;
//
EXTERNC int32_t lc3_api_encoder_init(LC3_Enc_Info* instance);
EXTERNC int32_t lc3_api_decoder_init(LC3_Dec_Info* instance);
EXTERNC void lc3_api_encoder_extra_buf_deinit(LC3_Enc_Info* info);
EXTERNC void lc3_api_decoder_extra_buf_deinit(LC3_Dec_Info* info);
EXTERNC bool lc3_api_decoder_bfi_detect(LC3_Dec_Info* info, uint8_t *data);
#endif

