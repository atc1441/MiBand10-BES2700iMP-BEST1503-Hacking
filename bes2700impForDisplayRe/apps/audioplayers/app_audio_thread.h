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
#if !defined(USE_BASIC_THREADS)
#ifndef __APP_AUDIO_THREAD_H__
#define __APP_AUDIO_THREAD_H__

#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

#define APP_AUDIO_MAILBOX_MAX (20)

enum APP_AUDIO_MODULE_ID_T {
    APP_AUDIO_MODULE_KEY = 0,
    APP_AUDIO_MODULE_AUDIO,
    APP_AUDIO_MODULE_AUDIO_MANAGE,
    APP_AUDIO_MODULE_VOICE_ASSIST,

    APP_AUDIO_MODULE_NUM
};

typedef struct {
    uint32_t message_id;
    uint32_t message_ptr;
    uint32_t message_Param0;
    uint32_t message_Param1;
    uint32_t message_Param2;
    float    message_Param3;
} APP_AUDIO_MESSAGE_BODY;

typedef struct {
    uint32_t src_thread;
    uint32_t dest_thread;
    uint32_t system_time;
    uint32_t mod_id;
    APP_AUDIO_MESSAGE_BODY msg_body;
} APP_AUDIO_MESSAGE_BLOCK;

typedef int (*APP_AUDIO_MOD_HANDLER_T)(APP_AUDIO_MESSAGE_BODY *);

int app_audio_mailbox_put(APP_AUDIO_MESSAGE_BLOCK* msg_src);

int app_audio_mailbox_free(APP_AUDIO_MESSAGE_BLOCK* msg_p);

int app_audio_mailbox_get(APP_AUDIO_MESSAGE_BLOCK** msg_p);

int app_audio_thread_init(void);

int app_audio_set_threadhandle(enum APP_AUDIO_MODULE_ID_T mod_id, APP_AUDIO_MOD_HANDLER_T handler);

void * app_audio_thread_tid_get(void);

bool app_audio_is_module_registered(enum APP_AUDIO_MODULE_ID_T mod_id);

#ifdef __cplusplus
    }
#endif

#endif//__FMDEC_H__
#endif
