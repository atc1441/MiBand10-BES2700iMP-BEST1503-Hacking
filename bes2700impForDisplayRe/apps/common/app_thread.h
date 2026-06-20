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
#ifndef __APP_THREAD_H__
#define __APP_THREAD_H__

#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

#define APP_MAILBOX_MAX (20)

enum APP_MOD_LEVEL_E {
    APP_MOD_LEVEL_0 = 0,
    APP_MOD_LEVEL_1,
    APP_MOD_LEVEL_2,
    APP_MOD_LEVEL_3,
    APP_MOD_LEVEL_4,
};

#ifndef CHIP_SUBSYS_SENS
enum APP_MODULE_ID_T {
    APP_MODULE_KEY = 0,
    APP_MODULE_AUDIO,
    APP_MODULE_BATTERY,
    APP_MODULE_BT,
    APP_MODULE_FM,
    APP_MODULE_SD,
    APP_MODULE_LINEIN,
    APP_MODULE_USBHOST,
    APP_MODULE_USBDEVICE,
    APP_MODULE_WATCHDOG,
    APP_MODULE_ANC,
    APP_MODULE_VOICE_ASSIST,
    APP_MODULE_SMART_MIC,

#ifdef __PC_CMD_UART__
    APP_MODULE_CMD,
#endif
#ifdef TILE_DATAPATH
    APP_MODULE_TILE,
#endif
    APP_MODULE_MIC,
#ifdef VOICE_DETECTOR_EN
    APP_MODULE_VOICE_DETECTOR,
#endif
#ifdef AUDIO_HEARING_COMPSATN
    APP_MODULE_HEAR_COMP,
#endif
    APP_MODULE_OHTER,
#if defined(USE_BASIC_THREADS)
    APP_MODULE_AUDIO_MANAGE,
    APP_MODULE_MEDIA,
    APP_MODULE_BTSYNC,
    APP_MODULE_ANC_FADE,
    APP_MODULE_UX,
    APP_MODULE_TWSCTRL,
#else
    APP_MODULE_CUSTOM0,
    APP_MODULE_CUSTOM1,
    APP_MODULE_CUSTOM2,
    APP_MODULE_CUSTOM3,
    APP_MODULE_CUSTOM4,
    APP_MODULE_CUSTOM5,
#endif
    APP_MODULE_NUM
};

#else /* defined(CHIP_SUBSYS_SENS) */
enum APP_MODULE_ID_T {
    APP_MODULE_KEY = 0,
    APP_MODULE_AUDIO,
#ifdef VOICE_DETECTOR_EN
    APP_MODULE_VOICE_DETECTOR,
#endif
    APP_MODULE_OHTER,

    APP_MODULE_NUM
};
#endif /* CHIP_SUBSYS_SENS */

typedef struct {
    uint32_t message_id;
    uint32_t message_ptr;
    uint32_t message_Param0;
    uint32_t message_Param1;
    uint32_t message_Param2;
    float    message_Param3;
#if defined(USE_BASIC_THREADS)
    void* p;
#endif
} APP_MESSAGE_BODY;

typedef struct {
    uint32_t src_thread;
    uint32_t dest_thread;
    uint32_t system_time;
    uint32_t mod_id;
    enum APP_MOD_LEVEL_E mod_level;
    APP_MESSAGE_BODY msg_body;
} APP_MESSAGE_BLOCK;

typedef int (*APP_MOD_HANDLER_T)(APP_MESSAGE_BODY *);

int app_mailbox_put(APP_MESSAGE_BLOCK* msg_src);

int app_mailbox_free(APP_MESSAGE_BLOCK* msg_p);

int app_mailbox_get(APP_MESSAGE_BLOCK** msg_p);

int app_os_init(void);

int app_set_threadhandle(enum APP_MODULE_ID_T mod_id, APP_MOD_HANDLER_T handler);

void * app_os_tid_get(void);

bool app_is_module_registered(enum APP_MODULE_ID_T mod_id);

#ifdef __cplusplus
    }
#endif

#endif//__FMDEC_H__
