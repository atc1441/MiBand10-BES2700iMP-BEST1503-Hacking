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


#ifndef __ANC_FIR_LMS_COMMON_H__
#define __ANC_FIR_LMS_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"
#include "stdint.h"
#include "iirfilt.h"
#include "custom_allocator.h"
#include "anc_assist.h"


#ifndef VQE_SIMULATE
#include "anc_process.h"
#else
// copy from anc_process.h
#define AUD_COEF_LEN        (500)

enum ANC_TYPE_T {
    ANC_NOTYPE          = 0,
    ANC_FEEDFORWARD     = (1 << 0),
    ANC_FEEDBACK        = (1 << 1),
    ANC_TALKTHRU        = (1 << 2),
    ANC_MUSICCANCLE     = (1 << 3),
    ANC_SPKCALIB        = (1 << 4),
    ANC_DEHOWLING       = (1 << 5),
    PSAP_FEEDFORWARD    = (1 << 6),
    PSAP_EQ             = (1 << 7),
    PSAP_SW             = (1 << 8),
};

typedef struct _aud_fir_item
{
    int32_t    fir_bypass_flag;
    int32_t    fir_len;
    int32_t    fir_coef[AUD_COEF_LEN];
} aud_fir_item;

typedef struct _struct_anc_fir_cfg
{
    aud_fir_item anc_fir_cfg_ff_l;
    aud_fir_item anc_fir_cfg_ff_r;
    aud_fir_item anc_fir_cfg_fb_l;
    aud_fir_item anc_fir_cfg_fb_r;
    aud_fir_item anc_fir_cfg_tt_l;
    aud_fir_item anc_fir_cfg_tt_r;
    aud_fir_item anc_fir_cfg_mc_l;
    aud_fir_item anc_fir_cfg_mc_r;
} struct_anc_fir_cfg;
#endif

#if defined(FREEMAN_ENABLED_STERO)
#define FIR_CHANNEL_NUM (2)
#else
#define FIR_CHANNEL_NUM (1)
#endif

// #define WEAR_LEAK_STATUS_CHANGED_DETECT
// #define MC_FIR_LMS_ENABLED

#if (FIR_CHANNEL_NUM == 2)
#define LOCAL_FIR_LEN (960)
#else
#define LOCAL_FIR_LEN (960 * 2)
#endif

#define FIR_SAMPLE_RATE (32000)

#if FIR_SAMPLE_RATE == 48000
#define FIR_BLOCK_SIZE (360)
#elif FIR_SAMPLE_RATE == 32000
#define FIR_BLOCK_SIZE (240)
#else
#define FIR_BLOCK_SIZE (120)
#endif

typedef struct {
    float ff_gain;
    float fb_gain;
    float mc_gain;
} FIR_LMS_CALIB_GAIN;

typedef struct {
    enum IIR_BIQUARD_TYPE type;
    float       gain;
    int         freq;
    float       q;
    int         sample_rate;
}FIR_LMS_FILTER_CFG_T;

typedef struct {
    float **ff_mic;
    uint8_t ff_ch_num;
    float **fb_mic;
    uint8_t fb_ch_num;
    float **talk_mic;
    uint8_t talk_ch_num;
    float **ref;
    uint8_t ref_ch_num;
    float *vpu_mic;
    uint32_t frame_len;
} process_frame_data_t;

int32_t dsp_set_anc_fir_cfg(struct_anc_fir_cfg *cfg, enum ANC_TYPE_T cmd, enum AUD_CHANNEL_MAP_T map);

#ifdef __cplusplus
}
#endif

#endif