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
#ifndef __APP_MEDIA_PLAYER_H__
#define __APP_MEDIA_PLAYER_H__

#include "bluetooth_bt_api.h"
#include "app_bt_stream.h"

#ifdef PROMPT_IN_FLASH
#define MEDIA_DEFAULT_LANGUAGE (LANGUAGE_ID_EN)
#else
#define MEDIA_DEFAULT_LANGUAGE (LANGUAGE_ID_EN - 1)
#endif

typedef enum
{
    T_AUDIO_ID = 0x0,

    T_AUDIO_NUMBER,
    
}MEDIA_AUDIO_TYPE;

typedef enum
{
    LANGUAGE_ID_INVALID = 0,
    LANGUAGE_ID_EN      = 1,
    LANGUAGE_ID_CN      = 2,
    LANGUAGE_ID_GN      = 3,
    LANGUAGE_ID_FN      = 4,
    LANGUAGE_ID_KN      = 5,
    LANGUAGE_ID_JN      = 6,
    LANGUAGE_ID_RN      = 7,

    LANGUAGE_ID_NUM = 0xFF,
} LANGUAGE_ID_E;

// Customized warning tone volume should be defined in tgt_hardware.h
#ifndef MEDIA_VOLUME_LEVEL_WARNINGTONE
#define MEDIA_VOLUME_LEVEL_WARNINGTONE TGT_VOLUME_LEVEL_8
#endif

#define MAX_PHB_NUMBER 20

#define PROMOT_ID_BIT_MASK                      (0xff << 8)
#define PROMOT_ID_BIT_MASK_MERGING              (1 << 15)
#define PROMOT_ID_BIT_MASK_CHNLSEl_LCHNL        (1 << 14)
#define PROMOT_ID_BIT_MASK_CHNLSEl_RCHNL        (1 << 13)
#define PROMOT_ID_BIT_MASK_LOCAL_PLAYING        (1 << 12)
#define PROMOT_ID_BIT_MASK_CHNLSEl_ALL          (PROMOT_ID_BIT_MASK_CHNLSEl_LCHNL|PROMOT_ID_BIT_MASK_CHNLSEl_RCHNL)

#define IS_PROMPT_NEED_MERGING(promptId)          ((promptId)&PROMOT_ID_BIT_MASK_MERGING ? true : false)
#define PROMPT_ID_FROM_ID_VALUE(promptIdVal)      ((promptIdVal)&(~PROMOT_ID_BIT_MASK))
#define PROMPT_CHNLSEl_FROM_ID_VALUE(promptIdVal) ((promptIdVal)&PROMOT_ID_BIT_MASK_CHNLSEl_ALL)
#define PROMPT_PRAM_FROM_ID_VALUE(promptIdVal)    ((promptIdVal)&PROMOT_ID_BIT_MASK)
#define IS_PROMPT_CHNLSEl_ALL(promptId)           (((promptId)&PROMOT_ID_BIT_MASK_CHNLSEl_ALL) == PROMOT_ID_BIT_MASK_CHNLSEl_ALL  ? true : false)
#define IS_PROMPT_CHNLSEl_LCHNL(promptId)         (((promptId)&PROMOT_ID_BIT_MASK_CHNLSEl_ALL) == PROMOT_ID_BIT_MASK_CHNLSEl_LCHNL  ? true : false)
#define IS_PROMPT_CHNLSEl_RCHNL(promptId)         (((promptId)&PROMOT_ID_BIT_MASK_CHNLSEl_ALL) == PROMOT_ID_BIT_MASK_CHNLSEl_RCHNL  ? true : false)
#define IS_PROMPT_PLAYED_LOCALLY(promptId)        ((promptId)&PROMOT_ID_BIT_MASK_LOCAL_PLAYING ? true : false)

typedef enum
{
    APP_PROMPT_NORMAL_PLAY              = 0,
    APP_PROMPT_LOCAL_PLAY               = 1,
    APP_PROMPT_REMOTE_PLAY              = 2,
    APP_PROMPT_STANDALONE_PLAY          = 3,
    APP_PROMPT_STANDALONE_LOCAL_PLAY    = 4,
    APP_PROMPT_STANDALONE_REMOTE_PLAY   = 5,
    APP_PROMPT_CONTINUOUS_NORMAL_PLAY   = 6,
    APP_PROMPT_LOCAL_CONTINUOUS_NORMAL_PLAY = 7,
    APP_PROMPT_CONTINUOUS_REMOTE_PLAY       = 8,
    APP_PROMPT_CACHED_ORG_REQUEST       = 9,
    APP_PROMPT_PENDING_FOR_MASTER_SYNC_PLAY_REQ = 10,
} APP_PROMPT_PLAY_REQ_TYPE_E;

typedef struct 
{
    uint16_t id;
    uint16_t status;
    // APP_AUDIO_STATUS
    uint16_t aud_type;
    uint16_t aud_id;
    char phb_number[MAX_PHB_NUMBER];
}msg_num_ptr;

typedef struct
{
    uint16_t    promptId;
    int16_t     volume_level_override;
    float       coeff_for_mix_prompt_for_music;
    float       coeff_for_mix_music_for_music;
    float       coeff_for_mix_prompt_for_call;
    float       coeff_for_mix_call_for_call;

} __attribute__((packed)) PROMPT_MIX_PROPERTY_T;

#define PROMPT_MIX_PROPERTY_TO_ADD(promptId, \
                                    volume_level_override,  \
                                    coeff_for_mix_prompt_for_music, \
                                    coeff_for_mix_music_for_music,  \
                                    coeff_for_mix_prompt_for_call,  \
                                    coeff_for_mix_call_for_call)  \
static PROMPT_MIX_PROPERTY_T promptId##_entry __attribute__((used, section(".mixprompt_property_table"))) =     \
    {(promptId),                        \
    (volume_level_override),            \
    (coeff_for_mix_prompt_for_music),   \
    (coeff_for_mix_music_for_music),    \
    (coeff_for_mix_prompt_for_call),    \
    (coeff_for_mix_call_for_call),      \
    };

extern uint32_t __mixprompt_property_table_start[];
extern uint32_t __mixprompt_property_table_end[];

#ifdef __cplusplus
extern "C" {
#endif

// Prompt will be played on the both sides if they're connected.
// If there are music/phone call on-going, the prompt will be mixed.
void media_PlayAudio(AUD_ID_ENUM id,uint8_t device_id);

// Prompt will be played locally no matter whether earbuds are connected or not
// If there are music/phone call on-going, the prompt will be mixed.
void media_PlayAudio_locally(AUD_ID_ENUM id, uint8_t device_id);

// Prompt will be played on peer device only if they're connected. 
// If there are music/phone call on-going, the music or phone call output will be stopped
// until the prompt playing is completed.
void media_PlayAudio_remotely(AUD_ID_ENUM id, uint8_t device_id);

// Prompt will be played on the both sides if they're connected.
// If there are music/phone call on-going, the music or phone call output will be stopped
// until the prompt playing is completed.
void media_PlayAudio_standalone(AUD_ID_ENUM id, uint8_t device_id);

// Prompt will be played locally no matter whether earbuds are connected or not
// If there are music/phone call on-going, the music or phone call output will be stopped
// until the prompt playing is completed.
void media_PlayAudio_standalone_locally(AUD_ID_ENUM id, uint8_t device_id);

// Prompt will be played on the peer device only if they're connected.
// If there are music/phone call on-going, the music or phone call output will be stopped
// until the prompt playing is completed.
void media_PlayAudio_standalone_remotely(AUD_ID_ENUM id, uint8_t device_id);

// Prompt will be played on the both sides if they're connected.
// If there are music/phone call on-going, the prompt will be mixed.
// Prompt will keep playing time and time again until media_PlayAudio_continuous_end is called
void media_PlayAudio_continuous_start(AUD_ID_ENUM id, uint8_t device_id);

// Stop the on-going continuous prompt playing
void media_PlayAudio_continuous_end(AUD_ID_ENUM id, uint8_t device_id);

bool media_playAudio_is_continuous_prompt_on_going(void);

bool media_playAudio_locally_is_continuous_prompt_on_going(void);

void media_PlayAudio_locally_continuous_start(AUD_ID_ENUM id, uint8_t device_id);

void media_PlayAudio_locally_continuous_end(AUD_ID_ENUM id, uint8_t device_id);

void app_prompt_push_request(APP_PROMPT_PLAY_REQ_TYPE_E reqType,
    AUD_ID_ENUM id, uint8_t device_id, bool isLocalPlaying);

void media_Set_IncomingNumber(const char* pNumber);

void media_FreeMemory(void* data);

int app_play_audio_onoff(bool on, APP_AUDIO_STATUS* status);

void stand_alone_prompt_supervision_timer_init(void);

void app_play_audio_set_lang(int L);

int app_play_audio_get_lang();

void app_audio_playback_done(void);

void media_runtime_audio_prompt_update(uint16_t id, uint8_t** ptr, uint32_t* len);

PROMPT_MIX_PROPERTY_T* get_prompt_mix_property(uint16_t promptId);

AUD_ID_ENUM media_GetCurrentPrompt(uint8_t device_id);

AUD_ID_ENUM app_get_current_standalone_promptId(void);

void update_standalone_prompt_volume(int8_t vol);

void app_tws_cmd_stop_prompt_handler(uint8_t* ptrParam, uint16_t paramLen);

void app_stop_both_prompt_playing(void);

void app_tws_stop_peer_prompt(void);

int app_play_audio_stop(void);

int app_play_audio_get_aud_id(void);

void trigger_media_play(AUD_ID_ENUM id, uint8_t device_id, uint16_t aud_pram);
void trigger_media_stop(AUD_ID_ENUM id, uint8_t device_id);

#ifdef IBRT
void trigger_prompt_local(bool isTriggerLocal);
bool is_trigger_prompt_local(void);
#endif

#ifdef AUDIO_LINEIN
uint32_t app_play_audio_lineinmode_more_data(uint8_t *buf, uint32_t len);

int app_play_audio_lineinmode_init(uint8_t mode, uint32_t buff_len);

int app_play_audio_lineinmode_start(APP_AUDIO_STATUS* status);

int app_play_audio_lineinmode_stop(APP_AUDIO_STATUS* status);

#endif

void app_tws_sync_prompt_check(void);

void app_tws_sync_prompt_manager_reset(void);

void app_prompt_inform_completed_event(void);

void app_prompt_inform_new_prompt_event(void);

void app_prompt_list_init(void);

void app_prompt_stop_all(void);

void app_prompt_flush_pending_prompts(void);

void media_playAudio_set_continous_prompt_env(AUD_ID_ENUM id);

typedef enum
{
    PROMPT_CHECK_USER_DEMO = 0,
    // add users here
    // TODO:

    // leave maximum 6 user here, increase it if need more
    PROMPT_CHECK_USER_NUM = 6,
} PROMPT_CHECK_USER_E;

typedef void (*post_prompt_user_callback_func)(PROMPT_CHECK_USER_E user);

typedef struct
{
    bool isPendingForPromptCompleted;
    post_prompt_user_callback_func postPromptFunc;
} PROMPT_CHECK_USER_ENV_T;

void prompt_check_user_register_post_prompt_callback_func(PROMPT_CHECK_USER_E user,
    post_prompt_user_callback_func func);

void prompt_check_user_init(void);

bool prompt_check_user_if_need_to_wait(PROMPT_CHECK_USER_E user);

void app_prompt_config_sample_rate(uint32_t new_sample_rate);

typedef int (*app_prompt_finish_cb) (int aud_id);
typedef int (*app_prompt_start_cb) (int aud_id);

int32_t app_prompt_finish_callback_register(app_prompt_finish_cb cb);
int32_t app_prompt_start_callback_register(app_prompt_start_cb cb);

int32_t app_prompt_volume_set(uint32_t volume);
int32_t app_prompt_volume_get(void);

void audio_prompt_req_send_failed_cb(uint16_t cmdCode, uint8_t* p_buff, uint16_t length);

AUD_ID_ENUM app_prompt_get_current_prompt_id(void);

int app_prompt_start_callback(int aud_id);

int app_prompt_finish_callback(int aud_id);

#ifdef __cplusplus
}
#endif

#endif

