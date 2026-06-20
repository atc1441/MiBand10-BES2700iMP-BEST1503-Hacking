/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#ifndef __ASSIST_ANC_FR_FITTING_H__
#define __ASSIST_ANC_FR_FITTING_H__
#include "stdint.h"
#include <stddef.h>
#include "assist_pso.h"

// === CONSTANT ===
#define PI 3.1415926
#define POINTS 500

// number of params
#ifdef LITTLE_FF
#define REAR 9
#define PARAM_NUM 10
#else
#define REAR 15
#define PARAM_NUM 16
#endif
// sample_rate
#ifdef LOW_FS_8k
#define SR 8000
#define FRAME_LEN 60
#else
#define SR 16000
#define FRAME_LEN 120
#endif
// smooth width and total range
#if (defined RESOLUTION_1024)
#define INTERVALS 2000 * 1024 / SR
#define WIN 5
#elif (defined RESOLUTION_2048)
#define INTERVALS 2000 * 2048 / SR
#define WIN 10
#elif (defined RESOLUTION_LOW)
#define INTERVALS 2000 * 512 / SR
#define WIN 5
#endif
#if (defined RESOLUTION_1024) && (defined LOW_FS_8k)
#undef WIN
#define WIN 10
#endif

/**
 * @brief eq_adjustment main function
 * @param[in]       freq        pointer to input array of freq points.[f1, f2, ...]
 * @param[in]       fr          freq response(target db) difference between golden and sample machine at "freq"
 * @param[in]       settings    solving config
 * @param[in,out]   solution    result
 *
 * @details
 *
 * the usage of "settings" see "set_algo_cfg" in assist_anc_fr_fitting.c
 * the freq points of the golden[g1, g2, ...] and sample[f1, f2, ...] should be same.
 * (or say |gi - fi| < tolerance you think.)
 */
int FrequencyResponseFitting(pso_settings_t* settings, pso_result_t* solution, int step, int particle_pieces);

void getba(const float *gfq, size_t sr, float* ba);

typedef struct
{
    float *ff_frame;
    float *fb_frame;

    float   *ff_fft;
    float   *fb_fft;
    // float   *tp_fft;
    int avg_cnt;

    float   *x;
    float   *y;

    int     cnt;
    int   delay;
    int close_strip_music;
    int anc_on;
    int anc_fb_on;
    int t1;
    int t2;
    int t3;
    int t4;
    int work_free;


    size_t     nfft;
    size_t     sr;
    int     last_anc_mode;
    int     cur_anc_mode;
    pso_result_t   *fr_fitting_res;
    pso_settings_t *settings;
    int fitting_done;
} assist_anc_fr_fitting_inst;

#include "anc_assist.h"

int32_t assist_anc_fr_fitting_create(assist_anc_fr_fitting_inst *fr_fitting_state,
                                    unsigned int sample_rate,
                                    int particle_size,
                                    int nfft,
                                    custom_allocator *allocator);
int32_t assist_anc_fr_fitting_destory(assist_anc_fr_fitting_inst *fr_fitting_state,
                                    custom_allocator *allocator);
int32_t assist_anc_fr_fitting_create_dynamic(assist_anc_fr_fitting_inst *fr_fitting_state,
                                    unsigned int sample_rate,
                                    int particle_size,
                                    int nfft);
int32_t assist_anc_fr_fitting_destory_dynamic(assist_anc_fr_fitting_inst *fr_fitting_state);
int32_t assist_anc_fr_fitting_reset(assist_anc_fr_fitting_inst *fr_fitting_state);
int32_t assist_anc_fr_fitting_preprocess(assist_anc_fr_fitting_inst *fr_fitting_state, int steps);
void my_complex_div(const float* src1, const float* src2, float* dst, uint32_t nfft);
int32_t assist_anc_fr_fitting_cpt(assist_anc_fr_fitting_inst *fr_fitting_state, int step, int cpt_steps, int particle_pieces);
#endif // end of FR_FITTING_H
