/**
 * @file gaf_media_stream.h
 * @author BES AI team
 * @version 0.1
 * @date 2020-08-31
 *
 * @copyright Copyright (c) 2015-2021 BES Technic.
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


#ifndef __GAF_MEDIA_STREAM_H__
#define __GAF_MEDIA_STREAM_H__

#include "gaf_media_common.h"

#ifdef __cplusplus
extern "C"{
#endif
/******************************macro defination*****************************/
#define GAF_AUDIO_CAPTURE_JOIN_DELAY_DMA_CHUNK_CNT    10
// TODO: may need to changed to run-time configurable value
#ifdef __BLE_AUDIO_24BIT__
#define GAF_AUDIO_STREAM_BIT_NUM    AUD_BITS_24
#else
#define GAF_AUDIO_STREAM_BIT_NUM    AUD_BITS_16
#endif

#if defined (BLE_AUDIO_USE_TWO_MIC_SRC_FOR_DONGLE) || defined (BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT)
#define GAF_AUDIO_STREAM_CAPTURE_CHANNEL_NUM     AUD_CHANNEL_NUM_2
#else
#define GAF_AUDIO_STREAM_CAPTURE_CHANNEL_NUM     AUD_CHANNEL_NUM_1
#endif

#if defined (BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT)
#define GAF_AUDIO_STREAM_PLAYBACK_CHANNEL_NUM     AUD_CHANNEL_NUM_2
#else
#define GAF_AUDIO_STREAM_PLAYBACK_CHANNEL_NUM     AUD_CHANNEL_NUM_1
#endif

#define GAF_AUDIO_STREAM_CAPTURE_BIT_NUM    AUD_BITS_16

#define APP_BAP_MAX_ASCS_NB_ASE_CHAR              (4)

enum gaf_audio_stream_start_cnt
{
    /// playback or capture stream is not started
    GAF_AUDIO_STREAM_NON_START = 0,
    /// playback or capture stream started cnt is 1
    GAF_AUDIO_STREAM_START_FIRST_CNT,
};

typedef enum app_gaf_stream_context_state
{
    APP_GAF_CONTEXT_STREAM_STARTED = 0,

    APP_GAF_CONTEXT_SINGLE_STREAM_STOPPED    = 1,
    APP_GAF_CONTEXT_ALL_STREAMS_STOPPED      = 2,
    APP_GAF_CONTEXT_CAPTURE_STREAMS_STOPPED  = 3,
    APP_GAF_CONTEXT_PLAYBACK_STREAMS_STOPPED = 4,

} gaf_stream_context_state_t;

/****************************function declaration***************************/
void gaf_audio_stream_start(uint8_t con_lid);
void gaf_audio_stream_stop(uint8_t con_lid);
void gaf_audio_stream_refresh_and_stop(uint8_t con_lid);
gaf_stream_context_state_t gaf_audio_update_stream_state_info_from_ase(
                            GAF_AUDIO_UPDATE_STREAM_INFO_PURPOSE_E purpose, uint8_t ase_lid);
gaf_stream_context_state_t gaf_audio_stream_update_and_start_handler(uint8_t ase_lid);
 void gaf_audio_update_stream_iso_hdl(uint8_t ase_lid);
void gaf_audio_stream_init(void);
uint8_t gaf_audio_get_stream_started_type(uint8_t ase_lid);

bool gaf_audio_is_playback_stream_on(void);
uint32_t gaf_get_ble_audio_playback_sample_rate(void);

void gaf_stream_dump_dma_trigger_status(void);

GAF_PLAYBACK_STREAM_STATE_E gaf_stream_get_playback_stream_state(uint8_t con_lid);

#ifdef IS_BLE_AUDIO_DEBUG_INFO_COLLECTOR_ENABLED
GAF_AUDIO_STREAM_ENV_T* gaf_audio_get_stream_env_from_con_lid(uint8_t con_lid);
void gaf_stream_capture_register_dma_irq_cb(gaf_stream_dma_irq_cb func);
#endif

void gaf_media_stream_start_interference();
void gaf_media_stream_stop_interference();

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __GAF_MEDIA_STREAM_H__ */
