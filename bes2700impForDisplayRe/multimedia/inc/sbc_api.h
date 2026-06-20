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
#ifndef _SBC_API_H_
#define _SBC_API_H_
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SBC_MAX_NUM_BLK     16
#define SBC_MAX_NUM_SB      8
#define SBC_MAX_NUM_CHNL    2
#define SBC_MAX_PCM_DATA    512
#define MSBC_BLOCKS         15

typedef enum {
    SBC_SAMPLERATE_16K = 0,
    SBC_SAMPLERATE_32K = 1,
    SBC_SAMPLERATE_44_1K = 2,
    SBC_SAMPLERATE_48K = 3,
} sbc_samplerate_e;

typedef enum {
    SBC_CHANNEL_MODE_MONO = 0,
    SBC_CHANNEL_MODE_DUAL_CHNL = 1,
    SBC_CHANNEL_MODE_STEREO = 2,
    SBC_CHANNEL_MODE_JOINT_STEREO = 3,
} sbc_channel_mode_e;

typedef enum {
    SBC_ALLOC_METHOD_LOUDNESS = 0,
    SBC_ALLOC_METHOD_SNR = 1,
} sbc_alloc_method_e;

typedef enum {
    SBC_DECODER_CHANNEL_SELECT_STEREO = 0,
    SBC_DECODER_CHANNEL_SELECT_LRMERGE = 1,
    SBC_DECODER_CHANNEL_SELECT_LCHNL = 2,
    SBC_DECODER_CHANNEL_SELECT_RCHNL = 3,
} sbc_decoder_channel_select_e;

typedef enum {
    SBC_SELECT_CHANNEL_MODE = 0, /*set sbc decoder channel select mode,
                                type sbc_decoder_channel_select_e*/
    SBC_FLAGS = 1,
} sbc_dec_para_e;

typedef enum {
    SBC_FLAGS_DEFAULT = 0,
    SBC_FLAGS_MSBC = 1,
} sbc_flags_e;

typedef struct {
    uint8_t bit_pool; /*1-250*/
    uint8_t sample_rate; /*sbc_samplerate_e*/
    uint8_t channel_mode; /*sbc_channel_mode_e*/
    uint8_t alloc_method; /*sbc_alloc_method_e*/
    uint8_t num_blocks;
    uint8_t num_subbands;
    uint8_t num_channels;
    uint8_t flags;
    uint32_t pcm_samples;/*output pcm samples*/
} sbc_stream_info_t;

typedef struct {
    char reserved[2176];
} sbc_encoder_t;

typedef struct {
    char reserved[5376];
} sbc_decoder_t;

typedef struct {
    uint8_t * sbc_data; /*sbc frame data*/
    uint16_t buffer_size; /*buffer size, Bytes*/
    uint16_t valid_size; /*IO, valid data length, Bytes*/
} sbc_frame_t;

typedef struct {
    int16_t * pcm_data; /*pcm frame data*/
    uint16_t buffer_size; /*buffer size, Bytes*/
    uint16_t valid_size; /*IO, valid data length, Bytes*/
} pcm_frame_t;

/**
 * \brief           Init SBC decoder handle.
 * \param handle    SBC decoder handle.
 * \return          Error code.
 */
int sbc_decoder_open(sbc_decoder_t * handle);

/**
 * \brief           Set decoder parameter.
 * \param handle    SBC decoder handle.
 * \param para      Parameter type, enum sbc_dec_para_e.
 * \param para      Parameter value.
 * \return          true:success, false:failed.
 */
bool sbc_decoder_set(sbc_decoder_t * handle, sbc_dec_para_e para, void * value);

/**
 * \brief           Get stream info.
 * \param handle    SBC decoder handle.
 * \param info      Stream info.
 * \return          Error code.
 */
int sbc_decoder_get_stream_info(sbc_decoder_t * handle, sbc_stream_info_t * info);

/**
 * \brief           Get sbc frame length.
 * \param info      Stream info.
 * \return          Length, Bytes.
 */
int sbc_get_frame_len(sbc_stream_info_t info);

/**
 * \brief           Decode one frame.
 * \param handle    Handle of SBC decoder instance.
 * \param aac_data  Input sbc data, update valid_size.
 * \param pcm_data  Output pcm data, update valid_size.
 * \return          Error code.
 */
int sbc_decoder_process_frame(sbc_decoder_t * handle, sbc_frame_t * sbc_data, pcm_frame_t * pcm_data);

/**
 * \brief           Parser one frame.
 * \param handle    Handle of SBC decoder instance.
 * \param aac_data  Input sbc data, update valid_size.
 * \return          Error code.
 */
int sbc_decoder_parser_frame(sbc_decoder_t * handle, sbc_frame_t * sbc_data);

/**
 * \brief           Update decoder.
 * \param decoder   Handle of SBC decoder instance.
 * \param encoder   Handle of SBC encoder instance.
 * \param pcm_data
 * \return          true:success, false:failed.
 */
bool sbc_decoder_update_state(sbc_decoder_t * decoder, sbc_encoder_t * encoder, pcm_frame_t *pcm_data);

/**
 * \brief           Init SBC encoder handle with stream info.
 * \param handle    SBC encoder handle.
 * \param info      Stream info.
 * \return          Error code.
 */
int sbc_encoder_open(sbc_encoder_t * handle, sbc_stream_info_t info);

/**
 * \brief           Encode one frame.
 * \param handle    Handle of SBC encoder instance.
 * \param pcm_data  Input pcm data, update valid_size.
 * \param aac_data  Output sbc data, update valid_size.
 * \return          Error code.
 */
int sbc_encoder_process_frame(sbc_encoder_t * handle, pcm_frame_t * pcm_data, sbc_frame_t * sbc_data);

/**
 * \brief           Get stream info.
 * \param handle    SBC encoder handle.
 * \param info      Stream info.
 * \return          Error code.
 */
int sbc_encoder_get_stream_info(sbc_encoder_t * handle, sbc_stream_info_t * info);

/**
 * \brief           Set stream info.
 * \param handle    SBC encoder handle.
 * \param info      Stream info.
 * \return          Error code.
 */
int sbc_encoder_set_stream_info(sbc_encoder_t * handle, sbc_stream_info_t info);

#ifdef __cplusplus
}
#endif
#endif /* SBC_API */
