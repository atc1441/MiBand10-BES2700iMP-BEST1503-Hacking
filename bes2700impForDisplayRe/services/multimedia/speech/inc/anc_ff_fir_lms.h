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


#ifndef __ANC_FF_FIR_LMS_H__
#define __ANC_FF_FIR_LMS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "anc_fir_lms_common.h"
#include "event_detection.h"

typedef enum
{
    ANC_FF_FIR_LMS_STAGE_IDLE = 0,
    ANC_FF_FIR_LMS_STAGE_SKIP,
    ANC_FF_FIR_LMS_STAGE_PNC,
    ANC_FF_FIR_LMS_STAGE_WAITING_ANC_ON,
    ANC_FF_FIR_LMS_STAGE_ANC,
    ANC_FF_FIR_LMS_STAGE_WAITING_ADAPTIVE_ANC_ON,
    ANC_FF_FIR_LMS_STAGE_ADAPTIVE_IIR,
    ANC_FF_FIR_LMS_STAGE_ADAPTIVE_ANC,
    ANC_FF_FIR_LMS_STAGE_NORMAL_ANC,
    ANC_FF_FIR_LMS_STAGE_WAITING_ANC_OFF,
    ANC_FF_FIR_LMS_STAGE_NUM,
} ANC_FF_FIR_LMS_STAGE;

typedef enum
{
    ANC_FF_FIR_LMS_RES_NORMAL = 0,
    ANC_FF_FIR_LMS_RES_PZ_ERR,
    ANC_FF_FIR_LMS_RES_SZ_ERR,
    ANC_FF_FIR_LMS_RES_BACKTRACK,
    ANC_FF_FIR_LMS_RES_BYPASS_FF_FIR,
    ANC_FF_FIR_LMS_RES_CONVERGENT,
    ANC_FF_FIR_LMS_RES_IN_CONVERGING,
} ANC_FF_FIR_LMS_RES;

static POSSIBLY_UNUSED char *stage_desc[ANC_FF_FIR_LMS_STAGE_NUM] = {
    "IDLE",
    "SKIP",
    "PNC",
    "WAITING ANC ON",
    "ANC",
    "WAITING ADAPTIVE ANC ON",
    "ADAPTIVE IIR",
    "ADAPTIVE ANC",
    "NORMAL ANC",
    "WAITING ANC OFF",
};

typedef struct {
    uint32_t debug_en;
    int32_t max_cnt;
    float delta_thresh;
    int32_t period_cnt;
    uint32_t ff_filt_num;

    int32_t mc_max_cnt;
    int32_t mc_period_cnt;

    uint32_t mc_filt_cnt;
    uint32_t ff_filt_cnt;
    uint32_t fb_filt_cnt;

    float mc_filt_ref_gain;
    float *ff_filt_ref_gain;
    float fb_filt_ref_gain;

    const FIR_LMS_FILTER_CFG_T  *mc_filt_cfg;
    const FIR_LMS_FILTER_CFG_T  *ff_filt_cfg;
    const FIR_LMS_FILTER_CFG_T  *fb_filt_cfg;

    EventDetectionConfig event_cfg;
} ANC_FF_FIR_LMS_CFG_T;

typedef struct
{
    // Status changed flag
    uint32_t    ff_gain_changed[MAX_FF_CHANNEL_NUM];
    uint32_t    fb_gain_changed[MAX_FB_CHANNEL_NUM];
    uint32_t    curve_changed[MAX_FB_CHANNEL_NUM];
    uint32_t    fir_flag_changed;

    // Who change gain or curve
    anc_assist_algo_id_t    ff_gain_id[MAX_FF_CHANNEL_NUM];
    anc_assist_algo_id_t    fb_gain_id[MAX_FB_CHANNEL_NUM];
    anc_assist_algo_id_t    curve_id[MAX_FB_CHANNEL_NUM];
    anc_assist_algo_id_t    fir_flag_id;

    // Gain or curve
    float   ff_gain[MAX_FF_CHANNEL_NUM];
    float   fb_gain[MAX_FB_CHANNEL_NUM];
    uint32_t    curve_index[MAX_FB_CHANNEL_NUM];
    uint32_t    fir_flag;

    noise_status_t noise_status;
    wind_status_t wind_status;

} ANCFFFirLmsRes;

typedef struct ANCFFFirLmsSt_ ANCFFFirLmsSt;

typedef void (*anc_ff_fir_lms_set_fir_freq_handler_t)(int freq, int line);
uint32_t anc_ff_fir_lms_register_set_fir_freq_handler(anc_ff_fir_lms_set_fir_freq_handler_t func);

ANCFFFirLmsSt * anc_ff_fir_lms_create(int32_t sample_rate, int32_t frame_len, const ANC_FF_FIR_LMS_CFG_T * cfg, custom_allocator *allocator, FIR_LMS_CALIB_GAIN *calib_gain);
void anc_ff_fir_lms_destroy(ANCFFFirLmsSt* st);
int32_t anc_ff_fir_lms_reset(ANCFFFirLmsSt * st, int32_t status, enum AUD_CHANNEL_MAP_T map);

int32_t anc_ff_fir_lms_process(ANCFFFirLmsSt* st, float **mic_data, int frame_len, ANC_FF_FIR_LMS_STAGE stage, EventController *curr_controller, enum AUD_CHANNEL_MAP_T map);
int32_t *fir_lms_coeff_cache(ANCFFFirLmsSt* st);
// int32_t *get_local_fir_cache(ANCFFFirLmsSt* st);
// int32_t *get_local_fir_cache2(ANCFFFirLmsSt* st);
float *anc_ff_fir_lms_get_Pz(ANCFFFirLmsSt* st, uint32_t *len);
float *anc_ff_fir_lms_get_Sz(ANCFFFirLmsSt* st, uint32_t *len);
float anc_ff_fir_lms_get_TF_feature(ANCFFFirLmsSt* st);
void anc_ff_fir_lms_get_ff_filter(ANCFFFirLmsSt* st, float* filter);
int32_t anc_ff_fir_lms_set_ff_fir_status(ANCFFFirLmsSt* st, EventDetectionState* event_st, bool enable);
uint32_t anc_ff_fir_lms_sync_fir_flag(uint32_t local_fir_flag, uint32_t remote_fir_flag);
void anc_ff_fir_lms_reset_ff_FIR(ANCFFFirLmsSt* st, enum AUD_CHANNEL_MAP_T map);
void anc_ff_fir_lms_update_mc_fir(ANCFFFirLmsSt* st,float *mc_fir_new);

void switch_virtual_filt(uint32_t index);

void anc_ff_fir_lms_set_ff_fir_calib_gain(ANCFFFirLmsSt *st, FIR_LMS_CALIB_GAIN *calib_gain);
void anc_ff_fir_lms_update_FB_gain(ANCFFFirLmsSt *st, float gain);

#if defined(WEAR_LEAK_STATUS_CHANGED_DETECT)
int32_t fir_anc_online_leak_detect_process(float * ff_mic, float * fb_mic, int frame_len);
#endif

#ifdef __cplusplus
}
#endif

#endif