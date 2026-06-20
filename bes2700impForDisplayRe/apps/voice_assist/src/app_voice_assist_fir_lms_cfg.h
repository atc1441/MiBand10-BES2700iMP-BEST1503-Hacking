/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#include "anc_ff_fir_lms.h"

#define BASE_ANC_FILTER_COUNT  (10)
#define MC_FILTER_COUNT        (7)
#define FF_FILTER_COUNT        (4)
#define FB_FILTER_COUNT        (6)
#define FB2EMIC_FILTER_COUNT   (1)

/*
    // low pass filter
    IIR_BIQUARD_LPF,
    // high pass filter
    IIR_BIQUARD_HPF,
    // notch filter
    IIR_BIQUARD_PEAKINGEQ,
    // low shelf filter
    IIR_BIQUARD_LOWSHELF,
    // high shelf filter
    IIR_BIQUARD_HIGHSHELF,
*/
#if (MC_FILTER_COUNT != 0)
#define MC_FILTER_REF_GAIN     (-3.0f)
const static FIR_LMS_FILTER_CFG_T  MC_FILT_CFG[MC_FILTER_COUNT] = {
    // {IIR_BIQUARD_PEAKINGEQ,   -15.0,   15000,  1.5,  FIR_SAMPLE_RATE},
    // {IIR_BIQUARD_PEAKINGEQ,     8.0,  5800,  1,  FIR_SAMPLE_RATE},
    // {IIR_BIQUARD_PEAKINGEQ,    -2.0,  2000,  1,  FIR_SAMPLE_RATE},
    // {IIR_BIQUARD_PEAKINGEQ,   -2.0,  850,  1,  FIR_SAMPLE_RATE},
    // {IIR_BIQUARD_PEAKINGEQ,   1.0, 250,  0.7,  FIR_SAMPLE_RATE},
    // {IIR_BIQUARD_HPF,         0, 18,  0.9,  FIR_SAMPLE_RATE},
    // {IIR_BIQUARD_PEAKINGEQ,    11.5, 85,  0.25,  FIR_SAMPLE_RATE},

    // {IIR_BIQUARD_PEAKINGEQ,   -12.0,   2200,   1.4,  FIR_SAMPLE_RATE},
    // {IIR_BIQUARD_HIGHSHELF,  -10.0,   7000,   1,    FIR_SAMPLE_RATE},
    // {IIR_BIQUARD_LOWSHELF,  -11.0,   13,     0.7,  FIR_SAMPLE_RATE},
    // {IIR_BIQUARD_PEAKINGEQ,   6.6,    90,     0.3,  FIR_SAMPLE_RATE},
    // {IIR_BIQUARD_PEAKINGEQ,     -6,  770,   1, FIR_SAMPLE_RATE},

    {IIR_BIQUARD_PEAKINGEQ,   -10.0,   2200,   1.4,   FIR_SAMPLE_RATE},
    {IIR_BIQUARD_HIGHSHELF,   -19.0,   7000,   0.9,   FIR_SAMPLE_RATE},
    {IIR_BIQUARD_LOWSHELF,    -11.0,   13,     0.7,   FIR_SAMPLE_RATE},
    {IIR_BIQUARD_PEAKINGEQ,    6,      80,     0.25,  FIR_SAMPLE_RATE},
    {IIR_BIQUARD_PEAKINGEQ,   -2.2,    860,    0.9,   FIR_SAMPLE_RATE},
};
#endif

#if (FF_FILTER_COUNT != 0)
static float  FF_FILTER_REF_GAIN[BASE_ANC_FILTER_COUNT] = {-0.0f, -0.0f, -0.0f, -0.0f,-0.0f, -0.0f,-0.0f, -0.0f,-0.0f, -0.0f,};
const static FIR_LMS_FILTER_CFG_T  FF_FILT_CFG[BASE_ANC_FILTER_COUNT][FF_FILTER_COUNT] = {
{
//mode 0
    {IIR_BIQUARD_HIGHSHELF,     -8, 7000,   1, FIR_SAMPLE_RATE},
    {IIR_BIQUARD_PEAKINGEQ,     -6,   11000,  1.3, FIR_SAMPLE_RATE},
    {IIR_BIQUARD_LOWSHELF,     -6,  300,   0.7, FIR_SAMPLE_RATE},
    {IIR_BIQUARD_PEAKINGEQ,     -7,  7000,  1.5, FIR_SAMPLE_RATE},

},
// //mode1
// {
//     {IIR_BIQUARD_HIGHSHELF,     -8, 11000,   1, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_PEAKINGEQ,     -7,  9500,   1.5, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_PEAKINGEQ,     -6,  7300,   1.3, FIR_SAMPLE_RATE},
// },
// //mode2
// {
//     {IIR_BIQUARD_HIGHSHELF,     -8, 11000,   1, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_PEAKINGEQ,     -7,  9500,   1.5, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_PEAKINGEQ,     -6,  7300,   1.3, FIR_SAMPLE_RATE},
// },
// //mode3
// {
//     {IIR_BIQUARD_HIGHSHELF,     -8, 11000,   1, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_PEAKINGEQ,     -7,  9500,   1.5, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_PEAKINGEQ,     -6,  7300,   1.3, FIR_SAMPLE_RATE},
// },
// //mode4
// {
//     {IIR_BIQUARD_HIGHSHELF,     -8, 11000,   1, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_PEAKINGEQ,     -7,  9500,   1.5, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_PEAKINGEQ,     -6,  7300,   1.3, FIR_SAMPLE_RATE},
// },
// //mode5
// {
//     {IIR_BIQUARD_HIGHSHELF,     -8, 11000,   1, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_PEAKINGEQ,     -7,  9500,   1.5, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_PEAKINGEQ,     -6,  7300,   1.3, FIR_SAMPLE_RATE},
// },
// //mode6
// {
//     {IIR_BIQUARD_HIGHSHELF,     -8, 11000,   1, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_PEAKINGEQ,     -7,  9500,   1.5, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_PEAKINGEQ,     -6,  7300,   1.3, FIR_SAMPLE_RATE},
// },
// //mode7
// {
//     {IIR_BIQUARD_HIGHSHELF,     -8, 11000,   1, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_PEAKINGEQ,     -7,  9500,   1.5, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_PEAKINGEQ,     -6,  7300,   1.3, FIR_SAMPLE_RATE},
// },
// //mode8
// {
//     {IIR_BIQUARD_HIGHSHELF,     -8, 11000,   1, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_PEAKINGEQ,     -7,  9500,   1.5, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_PEAKINGEQ,     -6,  7300,   1.3, FIR_SAMPLE_RATE},
// },
// //mode9
// {
//     {IIR_BIQUARD_HIGHSHELF,     -8, 11000,   1, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_PEAKINGEQ,     -7,  9500,   1.5, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_PEAKINGEQ,     -6,  7300,   1.3, FIR_SAMPLE_RATE},
// },
};
#endif

#if (FB_FILTER_COUNT != 0)
// #define FB_FILTER_REF_GAIN     (0.0f)
// const static FIR_LMS_FILTER_CFG_T  FB_FILT_CFG[FB_FILTER_COUNT] = {
//     {IIR_BIQUARD_PEAKINGEQ,  15.0,   120,   0.5, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_PEAKINGEQ,   4.5,   500,   0.5, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_PEAKINGEQ,  -17.0,  3800,   1.0, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_HIGHSHELF,   3.0,   300,   0.8, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_PEAKINGEQ,  -3.0,  2365,   1, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_LOWSHELF,    4.0,   800,   0.9, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_PEAKINGEQ, -5.0,  5500,  1, FIR_SAMPLE_RATE},
//     {IIR_BIQUARD_PEAKINGEQ,  6.0,  300,   0.5, FIR_SAMPLE_RATE},
// };
#define FB_FILTER_REF_GAIN     (0.0f)
const static FIR_LMS_FILTER_CFG_T  FB_FILT_CFG[FB_FILTER_COUNT] = {
    {IIR_BIQUARD_PEAKINGEQ,  -15.0,   6100,   0.8, FIR_SAMPLE_RATE},
    {IIR_BIQUARD_PEAKINGEQ,   17.5,   185,   0.2, FIR_SAMPLE_RATE},
    {IIR_BIQUARD_LOWSHELF,  6.0,  20,   0.7, FIR_SAMPLE_RATE},
    {IIR_BIQUARD_HIGHSHELF,  9.0,  950,   0.5, FIR_SAMPLE_RATE},
    {IIR_BIQUARD_PEAKINGEQ, -10.0,  3300,  1.2, FIR_SAMPLE_RATE},
    {IIR_BIQUARD_PEAKINGEQ, -5.0,  2400,  1, FIR_SAMPLE_RATE},

    // {IIR_BIQUARD_PEAKINGEQ,  -15.0,   5800,   1.0, FIR_SAMPLE_RATE},
    // {IIR_BIQUARD_PEAKINGEQ,   17.0,   180,   0.2, FIR_SAMPLE_RATE},
    // {IIR_BIQUARD_LOWSHELF,  10.0,  30,   0.5, FIR_SAMPLE_RATE},
    // {IIR_BIQUARD_HIGHSHELF,  6.0,  500,   0.7, FIR_SAMPLE_RATE},
    // {IIR_BIQUARD_PEAKINGEQ, -9.0,  3000,  1, FIR_SAMPLE_RATE},
    // {IIR_BIQUARD_PEAKINGEQ,   -5.0,   2400,   1, FIR_SAMPLE_RATE},
};
#endif

POSSIBLY_UNUSED const float fir_Sv_coeffs[10] = {1., 0.,};

const static ANC_FF_FIR_LMS_CFG_T cfg ={
    .debug_en = 1,
    .max_cnt = 0,
    .delta_thresh = 5e-6,
    .period_cnt = 2,
    .mc_max_cnt = 0,
    .mc_period_cnt = 0,
    .ff_filt_num = BASE_ANC_FILTER_COUNT,
    .mc_filt_cnt = MC_FILTER_COUNT,
    .ff_filt_cnt = FF_FILTER_COUNT,
    .fb_filt_cnt = FB_FILTER_COUNT,
    .mc_filt_ref_gain = MC_FILTER_REF_GAIN,
    .ff_filt_ref_gain = FF_FILTER_REF_GAIN,
    .fb_filt_ref_gain = FB_FILTER_REF_GAIN,
    .mc_filt_cfg = MC_FILT_CFG,
    .ff_filt_cfg = FF_FILT_CFG[0],
    .fb_filt_cfg = FB_FILT_CFG,
};

#ifdef MC_FIR_LMS_ENABLED
POSSIBLY_UNUSED const static ANC_MC_FIR_LMS_CFG_T mc_cfg = {
    .debug_en = 0,

    .mc_max_cnt = 0,
    .mc_period_cnt = 0,

    .mc_filt_cnt = MC_FILTER_COUNT,
    .fb_filt_cnt = FB_FILTER_COUNT,
    .mc_filt_ref_gain = MC_FILTER_REF_GAIN,
    .fb_filt_ref_gain = FB_FILTER_REF_GAIN,
    .mc_filt_cfg = MC_FILT_CFG,
    .fb_filt_cfg = FB_FILT_CFG,
};
#endif

POSSIBLY_UNUSED const static EventDetectionConfig event_cfg = {
    .wind_cfg = {
        .debug_en = 0,
        .scale_size = 8,           // freq range,8/scale=1k
        .to_none_targettime = 500, // time=500*7.5ms=3.75s
        .power_thd = 0.0001,
        .no_thd = 0.8,
        .small_thd = 0.7,
        .normal_thd = 0.55,
        .strong_thd = 0.4,
        .gain_none = 1.0,
        .gain_small_to_none = 0.7500,
        .gain_small = 0.5,
        .gain_normal_to_small = 0.3750,
        .gain_normal = 0.25,
        .gain_strong_to_normal = 0.1563,
        .gain_strong = 0.0625,
    },
    .noise_cfg = {
        .debug_en = 0,
        .strong_low_thd = 320,
        .strong_limit_thd = 60,
        .lower_low_thd = 3,
        .lower_mid_thd = 17,
        .quiet_out_thd = 1.5,
        .quiet_thd = 0.2,
        .extremely_quiet_out_thd = 0.25,
        .extremely_quiet_in_thd = 0.1,
        .snr_thd = 100,
        .period = 16,
        .window_size = 5,
        // .strong_count = 18,
        // .normal_count = 12,
        // .quiet_count = 4,
        .trans_table = {
            {0, 4, 12, 12, 18},
            {4, 0, 12, 12, 18},
            {4, 4, 0 , 12, 18},
            {4, 4, 12, 0 , 18},
            {4, 4, 12, 12, 0 },
        },
        .band_freq = {100, 400, 1000, 2000},
        .band_weight = {0.5, 0.5, 0},
    },
    .fb_howling_cfg = {
        .start_freq = 3000,
        .end_freq = 7000,
        .gain_fb = 0.7,
        .trans_table = {
            {0, 0, 0},
            {0, 0, 0},
            {0, 7500, 0},
        },
    },
    .ff_tone_cfg = {
        .debug_en = 0,
        .ind0 = 16,
        .ind1 = 96,
        .time_thd = 1000,
        .power_thd = 1e8f, //4.6e10 0db
    },
    .fb_tone_cfg = {
        .debug_en = 0,
        .ind0 = 16,
        .ind1 = 96,
        .time_thd = 1000,
        .power_thd = 1e9f,
    },
    .env_noise_cfg = {
        .debug_en = 0,
        .up_thresh = -3,
        .quiet_thresh = -60,
        .low_thresh = -85,
        .thresh_delta = 3,
        .trans_table = {
            {0, 67, 67, 67},
            {5,  0, 67,  5},
            {5, 67,  0,  5},
            {67, 5,  5,  0},
        },
    },
    .energy_detection_cfg = {
        .debug_en = 0,
        .timer = 5,
        .up_thresh = 1.2,
        .low_thresh = 0.00005,
        .normal_period = 12,
    },
    .weak_fir_cfg = {
        .debug_en = 0,
        .wear_noise_frame_num = 50,     //100 * 7.5ms
        .wear_change_num = 5,           //10 * (100 * 7.5ms)
        .noise_change_num = 5,          //10 * (100 * 7.5ms)
    },
#ifndef  ANC_ASSIST_VPU
    .speech_detection_cfg = {
        .debug_en = 0,
        .pnc_energy_ratio_thd = 1.0,
        .anc_energy_ratio_thd_quiet = -10.0, //ff energy < 65 dB
        .msc_thd_quiet = 0.6,
        .anc_energy_ratio_thd_middle = -20.0, //if speak,energy_ratio > energy_ratio_thd,msc < msc_thd;
        .msc_thd_middle = 0.78,
        .detection_cnt_thd = 3,//speech_keep_cnt >= detection_cnt_thd,output status change
        .energy_clac_start = (uint32_t)(200/62.5),
        .energy_clac_end = (uint32_t)(1000/62.5),
        .denoise_thd = -10,
    },
#endif
};
