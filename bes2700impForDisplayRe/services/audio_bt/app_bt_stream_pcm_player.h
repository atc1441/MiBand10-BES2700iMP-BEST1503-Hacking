/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifndef __AUDIO_PCM_PLAYER__
#define __AUDIO_PCM_PLAYER__

/* AUDIO_PCM_PLAYER is a pcm player based on app_bt_stream framework */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct audio_pcm_player_config {
    int bits;
    int sample_rate;
    int channel_num;
    int frame_length_samples;
    int volume;
} audio_pcm_player_config_t;

typedef enum audio_pcm_player_event {
    AUDIO_PCM_PLAYER_EVENT_MORE_DATA = 0,
} audio_pcm_player_event_t;

typedef struct audio_pcm_player_event_param {
    struct audio_pcm_player *player;
    void *player_data;
    union {
        struct {
            unsigned char *buff;
            unsigned int   buff_len;
        } more_data;
    } p;
} audio_pcm_player_event_param_t;

typedef int (*audio_pcm_player_callback_t)(audio_pcm_player_event_t event, audio_pcm_player_event_param_t *param);

typedef struct audio_pcm_player {
    void *player_data;
    audio_pcm_player_callback_t cb;
    audio_pcm_player_config_t config;
} audio_pcm_player_t;

// API
int audio_pcm_player_start(audio_pcm_player_t *player);
int audio_pcm_player_stop(audio_pcm_player_t *player);
int audio_pcm_player_close(audio_pcm_player_t *player);
int audio_pcm_player_setup(audio_pcm_player_t *player, audio_pcm_player_config_t *config);
int audio_pcm_player_open(audio_pcm_player_t *player, audio_pcm_player_callback_t cb, void *player_data);

// app_bt_stream layer
int audio_pcm_player_onoff(char onoff);

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_PCM_PLAYER__ */