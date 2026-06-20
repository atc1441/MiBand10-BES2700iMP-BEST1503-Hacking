/**
 * @file aob_ux_stm.h
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


#ifndef __AOB_CODEC_LC3_H__
#define __AOB_CODEC_LC3_H__

/*****************************header include********************************/
#include "cqueue.h"
#include "app_utils.h"
#ifdef GAF_ENCODER_CROSS_CORE_USE_M55
#include "gaf_media_common.h"
#endif
/******************************macro defination*****************************/
#define LC3_MAX_FRAME_SIZE 870

/******************************type defination******************************/

/****************************function declaration***************************/
#ifdef __cplusplus
extern "C"{
#endif

#define GAF_CODEC_LC3_BUF_ENTER_CONT 4

typedef struct {
    bool is_used;
    uint32_t size;
    uint8_t* data;
} lc3_buf_entry_t;

#ifndef AOB_CODEC_CP
void lc3_alloc_data_free(void);
#endif

void gaf_audio_lc3_update_codec_func_list(void *_pStreamEnv);
#ifdef LC3PLUS_SUPPORT
void gaf_audio_lc3plus_update_codec_func_list(void* _pStreamEnv);
#endif

int gaf_audio_lc3_encoder_get_max_frame_size(void);

enum APP_SYSFREQ_FREQ_T gaf_audio_media_decoder_adjust_bth_freq_no_m55(uint16_t frame_size, uint8_t channel,
    uint8_t frame_ms, uint32_t sample_rate, enum APP_SYSFREQ_FREQ_T _base_freq_);

enum APP_SYSFREQ_FREQ_T gaf_audio_flexible_decoder_adjust_bth_freq_no_m55(uint16_t frame_size, uint8_t channel,
    uint8_t frame_ms, uint32_t sample_rate, enum APP_SYSFREQ_FREQ_T _base_freq_);

enum APP_SYSFREQ_FREQ_T gaf_audio_stream_encoder_adjust_bth_freq_no_m55(uint16_t frame_size, uint8_t channel,
    uint8_t frame_ms, uint32_t sample_rate, enum APP_SYSFREQ_FREQ_T _base_freq_);
int gaf_audio_lc3_encoder_get_max_frame_size(void);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __AOB_CODEC_LC3_H__ */
