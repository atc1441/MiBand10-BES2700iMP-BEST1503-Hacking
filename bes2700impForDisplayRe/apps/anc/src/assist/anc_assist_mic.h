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
#ifndef __ANC_ASSIST_MIC_H__
#define __ANC_ASSIST_MIC_H__

#include "anc_assist_defs.h"
#include "plat_types.h"
#include "hal_aud.h"
#include "../app_anc_assist_thirdparty.h"

typedef enum {
    MIC_INDEX_FF = 0,
    MIC_INDEX_FF_L = MIC_INDEX_FF,
    MIC_INDEX_FF_R,
    MIC_INDEX_FB = MIC_INDEX_FF + MAX_FF_CHANNEL_NUM,
    MIC_INDEX_FB_L = MIC_INDEX_FB,
    MIC_INDEX_FB_R,
    MIC_INDEX_TALK = MIC_INDEX_FB + MAX_FB_CHANNEL_NUM,
    MIC_INDEX_TALK_L = MIC_INDEX_TALK,
    MIC_INDEX_TALK_R,
    MIC_INDEX_REF = MIC_INDEX_TALK + MAX_TALK_CHANNEL_NUM,
    MIC_INDEX_REF_L = MIC_INDEX_REF,
    MIC_INDEX_REF_R,

    MIC_INDEX_QTY = MIC_INDEX_REF + MAX_REF_CHANNEL_NUM
} anc_assist_mic_index_t;

#ifdef __cplusplus
extern "C" {
#endif

int32_t anc_assist_mic_reset(void);
int32_t anc_assist_mic_set_app_cfg(enum AUD_IO_PATH_T path);
int32_t anc_assist_mic_set_anc_cfg(enum AUD_IO_PATH_T path);
int32_t anc_assist_mic_update_anc_cfg(AncAssistConfig *cfg, ThirdpartyAssistConfig *thirdparty_cfg);
uint32_t anc_assist_mic_parser_anc_cfg(AncAssistConfig *cfg, ThirdpartyAssistConfig *thirdparty_cfg);
uint32_t anc_assist_spk_parser_anc_cfg(AncAssistConfig *cfg, ThirdpartyAssistConfig *thirdparty_cfg);
uint32_t anc_assist_mic_get_cfg(enum AUD_IO_PATH_T path);
uint32_t anc_assist_mic_get_ch_num(enum AUD_IO_PATH_T path);
uint32_t anc_assist_mic_anc_mic_is_enabled(anc_assist_mic_index_t index);

int32_t anc_assist_mic_parser_index(enum AUD_IO_PATH_T path);
int32_t anc_assist_mic_parser_app_buf(void *buf, uint32_t *len);
int32_t anc_assist_mic_parser_anc_buf(bool pcm_interval, enum AUD_IO_PATH_T path, float *anc_buf, uint32_t anc_frame_len, _PCM_T *pcm_buf, uint32_t pcm_len);

#ifdef __cplusplus
}
#endif

#endif