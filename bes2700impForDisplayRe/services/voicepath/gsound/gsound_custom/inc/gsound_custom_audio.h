/***************************************************************************
*
*Copyright 2015-2019 BES.
*All rights reserved. All unpublished rights reserved.
*
*No part of this work may be used or reproduced in any form or by any
*means, or stored in a database or retrieval system, without prior written
*permission of BES.
*
*Use of this work is governed by a license granted by BES.
*This work contains confidential and proprietary information of
*BES. which is protected by copyright, trade secret,
*trademark and other intellectual property rights.
*
****************************************************************************/

#ifndef __GSOUND_CUSTOM_AUDIO_H__
#define __GSOUND_CUSTOM_AUDIO_H__

#ifdef __cplusplus
extern "C"{
#endif

/*****************************header include********************************/
#include <stdint.h>

/******************************macro defination*****************************/
#define GSOUND_ENCODER_OUTPUT_FRAME_SIZE 312
#define GSOUND_ENCODER_OUTPUT_FRAME_NUM 4
#define GSOUND_ENCODER_OUTPUT_BUF_SIZE (GSOUND_ENCODER_OUTPUT_FRAME_SIZE * GSOUND_ENCODER_OUTPUT_FRAME_NUM)

/******************************type defination******************************/
enum {
    AUDIO_BLOCK_REASON_NULL = 0,
    AUDIO_BLOCK_REASON_ROLE = 1,

    AUDIO_BLOCK_REASON_NUM,
};

enum
{
    STATE_IDLE = 0,
    STATE_BUSY = 1,

    STATE_NUM,
};
/****************************function declearation**************************/
/**
 * @brief init custom audio env
 * 
 */
void gsound_custom_audio_init(void);

/**
 * @brief get output frame length
 * 
 * @return uint32_t output sbc frame length
 */
uint32_t gsound_custom_audio_get_output_sbc_frame_len(void);

/**
 * @brief get sbc frame count per ms
 * 
 * @return double sbc frame count per ms
 */
double gsound_custom_audio_get_sbc_frame_cnt_per_ms(void);

/**
 * @brief set the libgsound request data flag, only when this flag is set
 * should we upload data to libgsound(then transmit to APP)
 * 
 * @param flag flag value to set
 */
void gsound_custom_audio_set_libgsound_request_data_flag(bool flag);

/**
 * @brief store settings comes from library
 * 
 * @param settings library settings, should be in type of @see GSoundTargetAudioInSettings
 */
void gsound_custom_audio_store_audio_in_settings(const void *settings);

/**
 * @brief store audio interfaces come form library
 * 
 * @param handler pointer of audio interface, should be in type of @see GSoundAudioInterface
 */
void gsound_custom_audio_store_lib_interface(const void *handler);

/**
 * @brief get current bitpool
 * 
 * @return uint32_t current bitpool
 */
uint32_t gsound_custom_audio_get_bitpool(void);

/**
 * @brief open mic and start capture stream
 * 
 * @param sbc_bitpool bitpool used for SBC encode
 */
void gsound_custom_audio_open_mic(int sbc_bitpool);

/**
 * @brief close mic and stop the capture stream
 * 
 */
void gsound_custom_audio_close_mic(void);

/**
 * @brief transmit data into libgsound
 * 
 */
void gsound_custom_audio_transmit_data_to_libgsound();

/**
 * @brief handle mic data
 * 
 * @param buf pointer of incoming data
 * @param len length of incoming data
 * @param encoder_output_buf pointer of output data
 * @return uint32_t length of data consumed
 */
uint32_t gsound_custom_audio_handle_mic_data(uint8_t *buf,
                                             uint32_t len,
                                             uint8_t *encoder_output_buf);

void gsound_custom_audio_in_stream_interrupted_handler(uint8_t reason);

/**
 * @brief Callback function when audio upstream is started
 * 
 */
void gsound_custom_audio_on_upstream_started(void);

/**
 * @brief Callback function when audio upstream is stopped
 * 
 */
void gsound_custom_audio_on_upstream_stopped(void);

#ifdef IBRT
void gsound_custom_audio_role_switch_complete_handler(uint8_t role);
#endif

void gsound_custom_audio_trigger_data_process();

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __GSOUND_CUSTOM_AUDIO_H__ */