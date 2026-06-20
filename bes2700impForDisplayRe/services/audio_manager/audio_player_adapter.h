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
#ifndef __AUDIO_PLAYER_ADAPTER_H__
#define __AUDIO_PLAYER_ADAPTER_H__

#include "app_audio_bt_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Music
#define  AUDIO_STREAM_MUSIC               0x1
/// Prompt
#define  AUDIO_STREAM_MEDIA               0x2
/// Call
#define  AUDIO_STREAM_VOICE               0x4

#define  AUDIO_STREAM_LOCAL               0x8
/// Walkie talkie stream
#define  AUDIO_STREAM_W_T                 0x100

typedef uint32_t (*Audio_player_output_read_callback)(uint8_t *data_ptr, uint32_t data_len);

typedef struct
{
    BT_AUDIO_DEVICE_T  audio_device;
    uint32_t aud_id;
}Audio_device_t;

typedef struct
{
    // 0: player send,  1:device read
    uint8_t data_transfer_mode;
    void (*output_start)(uint32_t sample_rate, uint8_t bits_depth, uint8_t num_channels, uint32_t frame_samples);
    void (*output_stop)();
    void (*output_set_volume)(uint8_t vol);
    // data_transfer_mode = 0
    void (*output_data)(uint8_t *data_ptr, uint32_t data_len);
    // data_transfer_mode = 1
    void (*output_receive_ntf)(void);
    void (*output_reg_read_cb)(Audio_player_output_read_callback cb);
} Audio_output_route_t;

typedef struct
{
    int32_t (*play)( uint16_t stream_type, const Audio_device_t* device);
    int32_t (*stop)( uint16_t stream_type, const Audio_device_t* device);
    int32_t (*pause)(uint16_t stream_type, const Audio_device_t* device);
    void (*play_prompt)(uint32_t id,uint8_t device_id);
    int32_t (*seek)(int64_t time_in_ms);
    void (*dump_playback_status)(void);
    int32_t (*set_volume)(uint16_t volume);
    void (*volume_up)();
    void (*volume_down)();
    void (*restore_volume)(uint8_t *bdAddr);
    int32_t (*swap_sco)( uint16_t stream_type, uint8_t device_id);
    void (*set_output_route)(Audio_output_route_t *outpu_route);
    void (*start_capture)(const Audio_device_t* device);
    void (*stop_capture)(const Audio_device_t* device);
}Audio_Player_Adapter_t;

/**
 ****************************************************************************************
 * @brief init adapter
 *
 * @param[in] audio_player        PAC local index
 * @return void
 ****************************************************************************************
 */

void audio_player_adapter_init(const Audio_Player_Adapter_t* audio_player);
/**
 ****************************************************************************************
 * @brief play audio
 *
 * @param[in] stream_type         PAC local index
 * @param[in] device              Record identifier
 *
 * @return playback status
 ****************************************************************************************
 */

int32_t audio_player_play(uint16_t stream_type, const Audio_device_t* device);

/**
 ****************************************************************************************
 * @brief audio_player_playback_stop
 *
 * @param[in] stream_type         AUDIO_STREAM_MUSIC/
 * @param[in] device              device
 *
 * @return playback status
 ****************************************************************************************
 */
int32_t audio_player_playback_stop(uint16_t stream_type, const Audio_device_t* device);

/**
 ****************************************************************************************
 * @brief audio_player_pause
 *
 * @return playback status
 ****************************************************************************************
 */
int32_t audio_player_pause(uint16_t stream_type, const Audio_device_t* device);

/**
 ****************************************************************************************
 * @brief audio_player_seek
 *
 * @param[in] time_in_ms
 *
 * @return playback status
 ****************************************************************************************
 */
int32_t audio_player_seek(int64_t time_in_ms);

/**
 ****************************************************************************************
 * @brief audio_player_dump_playback_status
 *
 * @param[in] time_in_ms
 *
 * @return playback status
 ****************************************************************************************
 */
void audio_player_dump_playback_status(void);

/**
 ****************************************************************************************
 * @brief audio_player_play_prompt
 *
 * @param[in] id
 * @param[in] device
 *
 * @return playback status
 ****************************************************************************************
 */
void audio_player_play_prompt(uint32_t id,uint8_t device_id);

/**
 ****************************************************************************************
 * @brief audio_player_volume_up
 *
 * @return void
 ****************************************************************************************
 */
void audio_player_volume_up();

/**
 ****************************************************************************************
 * @brief audio_player_volume_up
 *
 * @return void
 ****************************************************************************************
 */
void audio_player_volume_down();

/**
 ****************************************************************************************
 * @brief audio_player_set_volume
 *
 * @param[in] stream_type
 * @param[in] device
 *
 * @return playback status
 ****************************************************************************************
 */
 
int32_t audio_player_set_volume(uint16_t volume);

/**
 ****************************************************************************************
 * @brief audio_player_restore_volume
 *
 * @param[in] device
 *
 * @return void
 ****************************************************************************************
 */
void audio_player_restore_volume(uint8_t *bdAddr);

/**
 ****************************************************************************************
 * @brief audio_player_swap_sco
 *
 * @param[in] stream_type
 * @param[in] device_id
 *
 * @return void
 ****************************************************************************************
 */
int32_t audio_player_swap_sco(uint16_t stream_type, uint8_t device_id);

/**
 ****************************************************************************************
 * @brief audio_player_start_capture
 *
 * @param[in] stream_type
 * @param[in] device
 *
 * @return void
 ****************************************************************************************
 */
void audio_player_start_capture(const Audio_device_t* device);

/**
 ****************************************************************************************
 * @brief audio_player_stop_capture
 *
 * @param[in] stream_type
 * @param[in] device
 *
 * @return void
 ****************************************************************************************
 */
void audio_player_stop_capture(const Audio_device_t* device);

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_PLAYER_ADAPTER_H__ */