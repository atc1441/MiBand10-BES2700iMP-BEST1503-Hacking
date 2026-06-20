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
#ifndef __APP_VOICE_ASSIST_AI_VOICE_H__
#define __APP_VOICE_ASSIST_AI_VOICE_H__

#include "plat_types.h"
#include "audioflinger.h"

typedef enum {
    ASSIST_AI_VOICE_MODE_KWS,
    ASSIST_AI_VOICE_MODE_RECORD,
    ASSIST_AI_VOICE_MODE_QTY
} assist_ai_voice_mode_t;

typedef void (*mempool_get_buff_t)(uint8_t **buff, uint32_t size, uint8_t user_index);

#ifdef __cplusplus
extern "C" {
#endif

int32_t app_voice_assist_ai_voice_init(void);
int32_t app_voice_assist_ai_voice_open(assist_ai_voice_mode_t mode, struct AF_STREAM_CONFIG_T *stream_cfg, mempool_get_buff_t mempool_get_buff);
int32_t app_voice_assist_ai_voice_close(void);

#ifdef __cplusplus
}
#endif

#endif