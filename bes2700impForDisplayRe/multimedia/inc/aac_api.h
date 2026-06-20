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
#ifndef __AAC_API_H__
#define __AAC_API_H__
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    void *(*malloc) (const uint32_t size);
    void (*free) (void *ptr);
} heap_api_t;

typedef enum {
    AAC_DECODER_CHANNEL_MODE_STEREO = 0,
    AAC_DECODER_CHANNEL_MODE_LRMERGE = 1,
    AAC_DECODER_CHANNEL_MODE_LCHNL = 2,
    AAC_DECODER_CHANNEL_MODE_RCHNL = 3,
} aac_decoder_channel_mode_e;

typedef enum {
    AAC_AOT_MAIN = 1,
    AAC_AOT_LC = 2,
    AAC_AOT_HE_AAC = 5,
    AAC_AOT_HE_AAC_V2 = 29,
    AAC_AOT_LC_MPEG2 = 129,
} aac_aot_e;

typedef enum {
    AAC_PACKAGE_RAW = 0,
    AAC_PACKAGE_ADTS = 2,
    AAC_PACKAGE_MCP1 = 6,
    AAC_PACKAGE_MCP0 = 7,
} aac_package_e;

typedef enum {
    AAC_BRMODE_CBR = 0,
    AAC_BRMODE_VBR_1_VERY_LOW_BITRATE = 1,
    AAC_BRMODE_VBR_2_LOW_BITRATE = 2,
    AAC_BRMODE_VBR_3_MEDIUM_BITRATE = 3,
    AAC_BRMODE_VBR_4_HIGH_BITRATE = 4,
    AAC_BRMODE_VBR_5_VERY_HIGH_BITRATE = 5,
} aac_brmode_e;

typedef enum {
    AAC_CHANNEL_MODE = 0, /*set decode channel mode*/
    AAC_DEC_CLEAR_BUFFER = 1, /*!< Clear internal bit stream buffer of transport layers. The
                                decoder will start decoding at new data passed after this event
                                and any previous data is discarded. */
    AAC_CONFIG_DATA = 3, /*set config data*/
    AAC_VALID_BYTES = 4, /*get valid bytes from aac handle input buffer*/
} aac_dec_para_e;

typedef enum {
    AAC_ENC_PARA_NULL,
} aac_enc_para_e;

typedef struct {
    uint32_t sample_rate; /* The sample rate in Hz of the decoded PCM audio signal. */
    uint16_t frame_size;  /* The frame size of the decoded PCM audio signal.
                       Typically this is:
                       1024 or 960 for AAC-LC
                       2048 or 1920 for HE-AAC (v2)
                       512 or 480 for AAC-LD and AAC-ELD */
    uint8_t num_channels;/*!< The number of output audio channels before the rendering
                      module, i.e. the original channel configuration. */
    uint32_t aac_sample_rate;
    uint8_t channel_cfg;
} aac_streaminfo_t;

typedef struct {
    uint8_t package;
    uint8_t channel_select;
} aac_dec_para_t;

typedef struct {
    uint32_t sample_rate;
    uint8_t channels;
    uint8_t aot;
    uint8_t package;
    uint8_t vbr;
    uint32_t bitrate;
} aac_enc_para_t;

typedef struct {
    uint8_t * aac_data; /*aac frame data*/
    uint32_t buffer_size; /*buffer size, Bytes*/
    uint32_t valid_size; /*IO, valid data length, Bytes*/
} aac_frame_t;

typedef struct {
    int16_t * pcm_data; /*pcm frame data*/
    uint32_t buffer_size; /*buffer size, Bytes*/
    uint32_t valid_size; /*IO, valid data length, Bytes*/
} aac_pcm_frame_t;

typedef struct {
    uint8_t * config_data; /*out-of-band config data*/
    uint32_t size; /*data length, Bytes*/
} config_frame_t;

#ifdef __cplusplus
extern "C" {
#endif
/*******************decoder interface start***********************/
/**
 * \brief           Create AAC decoder handle.
 * \param heap_api  malloc and free callback function pointer
 * \param para      parameter.
 * \return          AAC decoder handle.
 */
void * aac_decoder_open(heap_api_t heap_api, aac_dec_para_t para);

/**
 * \brief           Set decoder parameter.
 * \param handle    AAC decoder handle.
 * \param para      parameter type, enum aac_dec_para_e.
 * \param para      parameter value.
 * \return          Success or failed.
 */
bool aac_decoder_set(void * handle, aac_dec_para_e para, void * value);

/**
 * \brief           Get decoder parameter.
 * \param handle    AAC decoder handle.
 * \param para      parameter type, enum aac_dec_para_e.
 * \param para      parameter value.
 * \return          Success or failed.
 */
bool aac_decoder_get(void * handle, aac_dec_para_e para, void * value);

/**
 * \brief           De-allocate all resources of an AAC decoder instance.
 * \param handle    AAC decoder handle.
 * \return          void.
 */
bool aac_decoder_close(void * handle);

/**
 * \brief           Get stream info.
 * \param handle    Handle of AAC decoder instance.
 * \param info      Stream info.
 * \return          Error code.
 */
int aac_decoder_get_info(void * handle, aac_streaminfo_t * info);

/**
 * \brief           decode one frame,
 * \param handle    Handle of AAC decoder instance.
 * \param aac_data  Input aac data, update valid_size.
 * \param pcm_data  Output pcm data, update valid_size.
 * \return          Error code.
 */
int aac_decoder_process_frame(void * handle, aac_frame_t * aac_data, aac_pcm_frame_t * pcm_data);

/**
 * \brief           Check whether handle is valid.
 * \param handle    Handle of AAC decoder instance.
 * \return          true:valid, false:invalid.
 */
bool aac_decoder_handle_valid(void* handle);

/**
 * \brief           Get one aac mute data frame.
 * \param buff      Use to store data.
 * \param rate      Sample rate.
 * \param ch        Channels.
 * \param package   Only MCP1 frame is enabled currently.
 * \return          Frame length, bytes.
 */
int aac_get_mute_frame(uint8_t * buff, int rate, int ch, aac_package_e package);
/*******************decoder interface end***********************/

/*******************encoder interface start***********************/
/**
 * \brief           Create AAC encoder handle.
 * \param heap_api  malloc and free callback function pointer
 * \param para      parameter.
 * \return          AAC encoder handle.
 */
void * aac_encoder_open(heap_api_t heap_api, aac_enc_para_t para);

/**
 * \brief           Set encoder parameter.
 * \param handle    AAC encoder handle.
 * \param para      parameter type, enum aac_enc_para_e.
 * \param para      parameter value.
 * \return          Success or failed.
 */
bool aac_encoder_set(void * handle, aac_enc_para_e para, void * value);

/**
 * \brief           Get encoder parameter.
 * \param handle    AAC encoder handle.
 * \param para      parameter type, enum aac_enc_para_e.
 * \param para      parameter value.
 * \return          Success or failed.
 */
bool aac_encoder_get(void * handle, aac_enc_para_e para, void * value);

/**
 * \brief           De-allocate all resources of an AAC encoder instance.
 * \param handle    AAC encoder handle.
 * \return          void.
 */
bool aac_encoder_close(void * handle);

/**
 * \brief           encode one frame.
 * \param handle    Handle of AAC encoder instance.
 * \param pcm_data  Input pcm data, update valid_size.
 * \param aac_data  Output aac data, update valid_size.
 * \return          Error code.
 */
int aac_encoder_process_frame(void * handle, aac_pcm_frame_t * pcm_data, aac_frame_t * aac_data);
/*******************encoder interface end***********************/

/**
 * \brief           enable or disable mutex.
 * \param enable    true:enable, false:disable.
 * \return          void.
 */
bool aac_set_mutex_enable(bool enable);

#ifdef __cplusplus
}
#endif
#endif /* AAC_API_H */
