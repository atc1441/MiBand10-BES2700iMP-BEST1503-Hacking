/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifndef __SENSOR_HUB_CORE_APP_AI_VOICE_H__
#define __SENSOR_HUB_CORE_APP_AI_VOICE_H__

#define KWS_INFOR_PAYLOAD_LEN   (256)

#define SENSOR_HUB_KWS_SIGNAL_ID    0x99

#define KWS_RAW_PCM_DATA_QUEUE_BUF_LEN  (24*1024)
#define KWS_RAW_PCM_DATA_BUF_LEN_PER_SHOT   (2048)


typedef void (*voice_kws_key_word_handler_t)(void * kws_info,uint16_t len);
typedef int (*voice_kws_init_handler_t)(bool init, voice_kws_key_word_handler_t handler);
typedef uint32_t (*voice_kws_shot_get_frame_len_handler_t)(uint32_t ** read_ind);
typedef uint32_t (*voice_kws_data_handler_t)(uint8_t *buf, uint32_t len);
typedef void (*voice_kws_stream_prepare_start)(void);
typedef void (*voice_kws_stream_prepare_stop)(void);


typedef struct{
    voice_kws_init_handler_t init_handler;
    voice_kws_shot_get_frame_len_handler_t voice_kws_shot_frame_info_handler;
    voice_kws_data_handler_t voice_kws_frame_data_handler;
    voice_kws_stream_prepare_start stream_prepare_start_handler;
    voice_kws_stream_prepare_stop stream_prepare_stop_handler;
}voice_kws_handler_t;

typedef struct{
    uint16_t voice_user;
    bool activate;
    voice_kws_handler_t handler;
}ai_voice_kws_user_instant_t;

uint32_t sensor_hub_voice_kws_raw_pcm_data_input_signal(uint8_t*buf,uint32_t len);

void sensor_hub_voice_kws_init_capture_stream(struct AF_STREAM_CONFIG_T *cfg);

uint32_t sensor_hub_voice_kws_stream_prepare_start(uint8_t *buf, uint32_t len);

uint32_t sensor_hub_voice_kws_stream_prepare_stop(uint8_t *buf, uint32_t len);

bool sensor_hub_voice_kws_activate_op(SENSOR_HUB_AI_USER_E user, bool activate);

/* app do pre-env setup*/
void sensor_hub_voice_kws_ai_env_setup(uint8_t * buf,uint32_t len);

void sensor_hub_voice_kws_ai_user_activate(SENSOR_HUB_AI_USER_E role, uint32_t op);

#endif

