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
#if defined(AUDIO_PCM_PLAYER)

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "cmsis_os.h"

// platform related
#include "plat_types.h"
#include "hal_trace.h"
#include "tgt_hardware.h"
#include "app_status_ind.h"
#include "app_bt_stream.h"
#include "app_bt_media_manager.h"
#include "app_audio.h"
#include "audio_process.h"
#include "app_overlay.h"
#include "app_media_player.h"
#include "app_utils.h"
#include "audioflinger.h"

#include "app_bt_stream_pcm_player.h"

#define PCM_PLAYER_LOGD(...) TRACE(1, __VA_ARGS__)

static int audio_pcm_player_on = 0;
static audio_pcm_player_t *g_player = NULL;

static void audio_pcm_player_get_size(uint32_t *sample_size, uint32_t *dma_size)
{
    audio_pcm_player_config_t *player_config = &g_player->config;

    // sample size
    if (player_config->bits == 24) {
        *sample_size = 4*player_config->channel_num;
    }
    else {
        *sample_size = (player_config->bits/8)*player_config->channel_num;
    }

    // dma size
    *dma_size = (player_config->frame_length_samples*(*sample_size)) * 2;

    PCM_PLAYER_LOGD("audio_pcm_player_get_size:sample_size=%d,dma_size=%d", *sample_size, *dma_size);
}

// API
int audio_pcm_player_open(audio_pcm_player_t *player, audio_pcm_player_callback_t cb, void *player_data)
{
    player->cb          = cb;
    player->player_data = player_data;

    player->config.bits                 = 16;
    player->config.sample_rate          = 48000;
    player->config.channel_num          = 2;
    player->config.frame_length_samples = 960;
    player->config.volume               = 17;

    g_player = player;

    return 0;
}

int audio_pcm_player_start(audio_pcm_player_t *player)
{
    app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_START, BT_STREAM_PCM_PLAYER, 0, 0);    
    return 0;
}

int audio_pcm_player_stop(audio_pcm_player_t *player)
{
    app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_STOP, BT_STREAM_PCM_PLAYER, 0, 0);    
    return 0;
}

int audio_pcm_player_close(audio_pcm_player_t *player)
{
    g_player = NULL;
    return 0;
}

int audio_pcm_player_setup(audio_pcm_player_t *player, audio_pcm_player_config_t *config)
{
    memcpy(&player->config, config, sizeof(audio_pcm_player_config_t));
    return 0;
}

static uint32_t audio_pcm_player_need_pcm_data(uint8_t* pcm_buf, uint32_t len)
{
    audio_pcm_player_event_param_t param;

    if (g_player != NULL && g_player->cb != NULL) {
        param.player      = g_player;
        param.player_data = g_player->player_data;
        param.p.more_data.buff     = pcm_buf;
        param.p.more_data.buff_len = len;
        g_player->cb(AUDIO_PCM_PLAYER_EVENT_MORE_DATA, &param);
    }

    return 0;
}

int audio_pcm_player_onoff(char onoff)
{
    uint32_t dma_size = 0;
    uint32_t play_samp_size;
    struct AF_STREAM_CONFIG_T stream_cfg;
    uint8_t *audio_pcm_player_audio_play_buff = 0;
    audio_pcm_player_config_t *player_config = &g_player->config;

    (void)play_samp_size;

    PCM_PLAYER_LOGD("audio_pcm_player_onoff : now %d, onoff %d", audio_pcm_player_on, onoff);

    if (audio_pcm_player_on == onoff) {
        PCM_PLAYER_LOGD("audio_pcm_player_onoff : waring,already %d, do nothing!", onoff);
        return 0;
    }

     if (onoff) {
        app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_104M);
        app_audio_mempool_init();
        audio_pcm_player_get_size(&play_samp_size, &dma_size);
        app_audio_mempool_get_buff(&audio_pcm_player_audio_play_buff, dma_size);
        memset(&stream_cfg, 0, sizeof(stream_cfg));

        stream_cfg.bits = AUD_BITS_16;
        stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)player_config->channel_num;
#if defined(__AUDIO_RESAMPLE__)
        stream_cfg.sample_rate = AUD_SAMPRATE_50781;
#else
        stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)player_config->sample_rate;
#endif
#if FPGA==0
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
#else
        stream_cfg.device = AUD_STREAM_USE_EXT_CODEC;
#endif
        stream_cfg.vol = player_config->volume;
        PCM_PLAYER_LOGD("audio_pcm_player_onoff : vol = %d", player_config->volume);
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.handler = audio_pcm_player_need_pcm_data;
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(audio_pcm_player_audio_play_buff);
        stream_cfg.data_size = dma_size;

        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    }
     else {
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);

        app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_32K);
    }

    audio_pcm_player_on = onoff;
    PCM_PLAYER_LOGD("audio_pcm_player_onoff : end!\n");
    return 0;
}

#endif /* AUDIO_PCM_PLAYER */