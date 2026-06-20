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
#include "app_utils.h"
#include "hal_trace.h"
#include "hal_aud.h"
#include "app_voice_assist_ai_voice.h"

static assist_ai_voice_mode_t g_mode = ASSIST_AI_VOICE_MODE_QTY;

extern int32_t assist_ai_voice_kws_init(void);
extern int32_t assist_ai_voice_kws_open(struct AF_STREAM_CONFIG_T *stream_cfg);
extern int32_t assist_ai_voice_kws_close(void);

#if defined(DUAL_MIC_RECORDING)
extern int32_t assist_ai_voice_record_open(struct AF_STREAM_CONFIG_T *stream_cfg, mempool_get_buff_t mempool_get_buff);
extern int32_t assist_ai_voice_record_close(void);
#endif

int32_t app_voice_assist_ai_voice_init(void)
{
    assist_ai_voice_kws_init();

    return 0;
}

int32_t app_voice_assist_ai_voice_open(assist_ai_voice_mode_t mode, struct AF_STREAM_CONFIG_T *stream_cfg, mempool_get_buff_t mempool_get_buff)
{
    g_mode = mode;

    TRACE(0, "[%s] mode: %d", __func__, g_mode);

    if (g_mode == ASSIST_AI_VOICE_MODE_KWS) {
        assist_ai_voice_kws_open(stream_cfg);
#if defined(DUAL_MIC_RECORDING)
    } else if (g_mode == ASSIST_AI_VOICE_MODE_RECORD) {
        assist_ai_voice_record_open(stream_cfg, mempool_get_buff);
#endif
    } else {
        ASSERT(0, "[%s] mode(%d) is invalid", __func__, g_mode);
    }

    return 0;
}

int32_t app_voice_assist_ai_voice_close(void)
{
    TRACE(0, "[%s] mode: %d", __func__, g_mode);

    if (g_mode == ASSIST_AI_VOICE_MODE_KWS) {
        assist_ai_voice_kws_close();
#if defined(DUAL_MIC_RECORDING)
    } else if (g_mode == ASSIST_AI_VOICE_MODE_RECORD) {
        assist_ai_voice_record_close();
#endif
    } else {
        ASSERT(0, "[%s] mode(%d) is invalid", __func__, g_mode);
    }

    return 0;
}