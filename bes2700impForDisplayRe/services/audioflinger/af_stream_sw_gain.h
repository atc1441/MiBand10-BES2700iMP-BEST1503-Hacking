/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifndef AF_STREAM_SW_GAIN_H
#define AF_STREAM_SW_GAIN_H

#include "plat_types.h"
#include "stdbool.h"
#include "hal_aud.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUDIO_OUTPUT_SW_GAIN_MODE_IIR       (0)
#define AUDIO_OUTPUT_SW_GAIN_MODE_LINEAR    (1)

/**
 * @brief Call this function when init software gain module
 *
 * @return
*/
int32_t af_stream_sw_gain_init(void);

/**
 * @brief Call this function when open stream
 *
 * @param id Audioflinger stream id
 * @param stream Audioflinger stream
 * @param sample_rate Stream sample rate
 * @param bits Stream data bits
 * @param chans Stream channel numbers
 * @param gain_mode Select software gain mode, the value is AUDIO_OUTPUT_SW_GAIN_MODE_IIR or AUDIO_OUTPUT_SW_GAIN_MODE_LINEAR
 * @return
*/
int32_t af_stream_sw_gain_open(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, enum AUD_SAMPRATE_T sample_rate, enum AUD_BITS_T bits,
                                enum AUD_CHANNEL_NUM_T chans, int32_t gain_mode);

/**
 * @brief Call this function when close stream
 *
 * @param id Audioflinger stream id
 * @param stream Audioflinger stream
 * @return
*/
int32_t af_stream_sw_gain_close(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream);

/**
 * @brief Call this function when processing stream data
 *
 * @param id Audioflinger stream id
 * @param stream Audioflinger stream
 * @param buf Data buffer
 * @param len Data length
*/
void af_stream_sw_gain_process(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, uint8_t *buf, uint32_t len);

/**
 * @brief Call this function when setting the software gain coefficient
 *
 * @param id Audioflinger stream id
 * @param stream Audioflinger stream
 * @param coef The software gain coefficient
*/
void af_stream_sw_gain_set_gain_coef(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, float coef);

/**
 * @brief Call this function when setting the software volume level
 *
 * @param id Audioflinger stream id
 * @param stream Audioflinger stream
 * @param vol The volume level
*/
void af_stream_sw_gain_set_volume_level(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, uint32_t vol);

/**
 * @brief Call this function when changing smoothing time in linear mode
 *
 * @param id Audioflinger stream id
 * @param stream Audioflinger stream
 * @param time_ms The new smoothing time
*/
void af_stream_sw_gain_set_smooth_time(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, uint32_t time_ms);

/**
 * @brief Call this function when getting the software gain coefficient
 *
 * @param id Audioflinger stream id
 * @param stream Audioflinger stream
 * @return The software gain coefficient
*/
float af_stream_sw_gain_get_gain_coef(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream);

/**
 * @brief When the processing position of software gain needs to be adjusted,
 *      this function can be used together with 'af_stream_sw_gain_get_enable' to achieve the purpose.
 *
 * @param id Audioflinger stream id
 * @param stream Audioflinger stream
 * @param enable When this parameter is false, skip processing in the normal flow
*/
void af_stream_sw_gain_enable(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, bool enable);

/**
 * @brief When the processing position of software gain needs to be adjusted,
 *      this function can be used together with 'af_stream_sw_gain_enable' to achieve the purpose.
 *
 * @param id Audioflinger stream id
 * @param stream Audioflinger stream
 * @return
*/
bool af_stream_sw_gain_get_enable(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream);

/**
 * @brief Call this function when setting the algorithm second coefficient
 *
 * @param id Audioflinger stream id
 * @param stream Audioflinger stream
 * @param coef The algorithm second coefficient
*/
void af_stream_sw_gain_set_second_coef(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, float coef);

#ifdef __cplusplus
}
#endif

#endif /* AF_STREAM_SW_GAIN_H */