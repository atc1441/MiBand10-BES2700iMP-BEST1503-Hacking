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
#include "plat_types.h"
#include "filter_debug_trace.h"
#include "iir_process.h"
#include "eq_cfg.h"
#include "hw_codec_iir_process.h"
#include "hw_iir_process.h"
#include "math.h"
#include "arm_math.h"
#include "freqz.h"
#include <string.h>

#define IIR_FIXED_Q (1<<27)

#ifndef PI
#define PI                                  (3.14159265358979)
#endif

static HW_CODEC_IIR_CFG_T  hw_codec_iir_eq;
static HW_IIR_CFG_T  hw_iir_eq;

void iir_lowshelf_coefs_generate(float gain, float fn, float Q, float *coefs)
{
    float A = (float)sqrtf(powf(10, gain / 20));
    float w0 = 2 * PI*fn;
    float alpha = (float)sinf(w0)/(2*Q);

    float a0 = (A + 1) + (A - 1)*(float)cosf(w0) + 2 * (float)sqrtf(A)*alpha;
    float a1 = (-2 * ((A - 1) + (A + 1)*(float)cosf(w0)));
    float a2 = ((A + 1) + (A - 1)*(float)cosf(w0) - 2 * (float)sqrtf(A)*alpha);
    float b0 = (A*((A + 1) - (A - 1)*(float)cosf(w0) + 2 *(float)sqrtf(A)*alpha));
    float b1 = (2 * A*((A - 1) - (A + 1)*(float)cosf(w0)));
    float b2 = (A*((A + 1) - (A - 1)*(float)cosf(w0) - 2 * (float)sqrtf(A)*alpha));

    coefs[0] = 1;
    coefs[1] = a1 / a0;
    coefs[2] = a2 / a0;
    coefs[3] = b0 / a0;
    coefs[4] = b1 / a0;
    coefs[5] = b2 / a0;

//    IIR_TRACE(6,"fn = %f, gain = %f, Q = %f, A = %f, w0 = %f, alpha = %f", fn, gain, Q, A, w0, alpha);
//    FILTER_DEBUG_TRACE(7,"[%s] %f, %f, %f, %f, %f, %f", __func__, (double)coefs[0], (double)coefs[1], (double)coefs[2], (double)coefs[3], (double)coefs[4], (double)coefs[5]);
}

void iir_highshelf_coefs_generate(float gain, float fn, float Q, float *coefs)
{
    float A = (float)sqrtf(powf(10, gain / 20));
    float w0 = 2 * PI*fn;
    float alpha = (float)sinf(w0) / (2 * Q);

    float a0 = (A + 1) - (A - 1)*(float)cosf(w0) + 2 * (float)sqrtf(A)*alpha;  //  a0
    float a1 = (2 * ((A - 1) - (A + 1)*(float)cosf(w0)));  // a1
    float a2 = ((A + 1) - (A - 1)*(float)cosf(w0) - 2 * (float)sqrtf(A)*alpha);  //a2
    float b0 = (A*((A + 1) + (A - 1)*(float)cosf(w0) + 2 * (float)sqrtf(A)*alpha));  //b0
    float b1 = (-2 * A*((A - 1) + (A + 1)*(float)cosf(w0)));   // b1
    float b2 = (A*((A + 1) + (A - 1)*(float)cosf(w0) - 2 * (float)sqrtf(A)*alpha));  // b2

    coefs[0] = 1;
    coefs[1] = a1 / a0;
    coefs[2] = a2 / a0;
    coefs[3] = b0 / a0;
    coefs[4] = b1 / a0;
    coefs[5] = b2 / a0;

//    IIR_TRACE(6,"fn = %f, gain = %f, Q = %f, A = %f, w0 = %f, alpha = %f", fn, gain, Q, A, w0, alpha);
 //   FILTER_DEBUG_TRACE(7,"[%s] %f, %f, %f, %f, %f, %f", __func__, (double)coefs[0], (double)coefs[1], (double)coefs[2], (double)coefs[3], (double)coefs[4], (double)coefs[5]);
}

// fn: Normalized frequency
void iir_peak_notch_coefs_generate(float gain, float fn, float Q, float *coefs)
{
//    float Fs = 1000.0 / 48000.0;
//    float gain = -5.0;
//    float fo = 1000.0;
//    float Q = 0.7;
    float A = (float)sqrtf(powf(10, gain/20));
    float w0 = 2*PI*fn;
    float alpha = (float)sinf(w0)/(2*Q);

    float a0 = 1 + alpha/A;
    float a1 = -2*(float)cosf(w0);
    float a2 = 1 - alpha/A;
    float b0 = 1 + alpha*A;
    float b1 = -2*(float)cosf(w0);
    float b2 = 1 - alpha*A;

    coefs[0] = 1;
    coefs[1] = a1 / a0;
    coefs[2] = a2 / a0;
    coefs[3] = b0 / a0;
    coefs[4] = b1 / a0;
    coefs[5] = b2 / a0;

//    IIR_TRACE(6,"fn = %f, gain = %f, Q = %f, A = %f, w0 = %f, alpha = %f", fn, gain, Q, A, w0, alpha);
 //   FILTER_DEBUG_TRACE(7,"[%s] %f, %f, %f, %f, %f, %f", __func__, (double)coefs[0], (double)coefs[1], (double)coefs[2], (double)coefs[3], (double)coefs[4], (double)coefs[5]);
}

void iir_highpass_coefs_generate(float gain, float fn, float Q, float *coefs)
{
    float w0 = 2 * PI*fn;
    float alpha = (float)sinf(w0) / (2 * Q);

    float a0 = 1 + alpha;
    float a1 = -2 * (float)cosf(w0);
    float a2 = 1 - alpha;
    float b0 = (1 + (float)cosf(w0))/2;
    float b1 = -(1 + (float)cosf(w0));
    float b2 = (1 + (float)cosf(w0))/2;

    coefs[0] = 1;
    coefs[1] = a1/a0;
    coefs[2] = a2/a0;
    coefs[3] = b0/a0;
    coefs[4] = b1/a0;
    coefs[5] = b2/a0;
}

void iir_lowspass_coefs_generate(float gain, float fn, float Q, float *coefs)
{
    float w0 = 2 * PI*fn;
    float alpha = (float)sinf(w0) / (2 * Q);

    float a0 = 1 + alpha;
    float a1 = -2 * (float)cosf(w0);
    float a2 = 1 - alpha;
    float b0 = (1 - (float)cosf(w0))/2;
    float b1 = 1 - (float)cosf(w0);
    float b2 = (1 - (float)cosf(w0))/2;

    coefs[0] = 1;
    coefs[1] = a1/a0;
    coefs[2] = a2/a0;
    coefs[3] = b0/a0;
    coefs[4] = b1/a0;
    coefs[5] = b2/a0;
}

void iir_allpass_coefs_generate(float gain, float fn, float Q, float *coefs)
{
    float w0 = 2 * PI*fn;
    float alpha = sinf(w0) / (2 * Q);

    float a0 = 1 + alpha;
    float a1 = -2 * cosf(w0);
    float a2 = 1 - alpha;
    float b0 = 1 - alpha;
    float b1 = -2 * cosf(w0);
    float b2 = 1 + alpha;

    coefs[0] = 1;
    coefs[1] = a1/a0;
    coefs[2] = a2/a0;
    coefs[3] = b0 / a0;
    coefs[4] = b1 / a0;
    coefs[5] = b2 / a0;
}

float iir_convert_db_to_multiple(float db)
{
    return (float)powf(10, db/20);
}

void iir_coefs_generate(IIR_TYPE_T type, float gain, float fn, float Q, float *coefs)
{
    // As it is meaningless to set an eq at or above fs/2, just skip it when we receive
    // such request.
    if (fn >= 0.5f)
    {
        coefs[0] = 1;
        coefs[1] = 0;
        coefs[2] = 0;
        coefs[3] = 1;
        coefs[4] = 0;
        coefs[5] = 0;

        FILTER_DEBUG_TRACE(1,"[%s] center frequency is above nyquist frequency, skip this band", __FUNCTION__);
        return;
    }

    if (type == IIR_TYPE_LOW_SHELF)
    {
        iir_lowshelf_coefs_generate(gain, fn, Q, coefs);
    }
    else if (type == IIR_TYPE_PEAK)
    {
        iir_peak_notch_coefs_generate(gain, fn, Q, coefs);
    }
    else if (type == IIR_TYPE_HIGH_SHELF)
    {
        iir_highshelf_coefs_generate(gain, fn, Q, coefs);
    }
    else if (type == IIR_TYPE_LOW_PASS)
    {
        iir_lowspass_coefs_generate(gain, fn, Q, coefs);
    }
    else if (type == IIR_TYPE_HIGH_PASS)
    {
        iir_highpass_coefs_generate(gain, fn, Q, coefs);
    }
    else if (type == IIR_TYPE_ALL_PASS)
    {
        iir_allpass_coefs_generate(gain, fn, Q, coefs);
    }
    else
    {
        ASSERT(false, "[%s] %d is not supported", __func__, type);
    }
}

HW_CODEC_IIR_CFG_T *hw_codec_iir_get_cfg(enum AUD_SAMPRATE_T sample_rate, const IIR_CFG_T *ext_cfg)
{
    const IIR_CFG_T *cfg;
    float coefs_l[6],coefs_r[6];
    float gain0,gain1;
    int sample_rate_iir=sample_rate;
	
#if defined(CHIP_BEST2001)
#ifdef __AUDIO_RESAMPLE__
	sample_rate_iir=50781.25f*8;
#else
    switch(sample_rate)
    {
	case AUD_SAMPRATE_22050:
	case AUD_SAMPRATE_44100:
	case AUD_SAMPRATE_88200:		
	case AUD_SAMPRATE_176400:
	case AUD_SAMPRATE_352800:	
	case AUD_SAMPRATE_705600:
	sample_rate_iir=44100*8;
	break;

	default:
	sample_rate_iir=48000*8 ;
	break;
   };
#endif
#endif
    // FILTER_DEBUG_TRACE(2,"%s,sample_rate:%d\n", __func__,sample_rate_iir);

    hw_codec_iir_eq.iir_filtes_l.iir_bypass_flag=1;
    hw_codec_iir_eq.iir_filtes_r.iir_bypass_flag=1;

    if (ext_cfg)
    {
        cfg = ext_cfg;
    }
    else
    {
        FILTER_DEBUG_TRACE(0,"Error HW IIR CODEC cfg!");
        return NULL;
    }

    // Check parameter
    ASSERT(cfg->num <= AUD_DAC_IIR_NUM_EQ, "[%s] num(%d) is too large, max_num=%d", __func__, cfg->num, AUD_DAC_IIR_NUM_EQ);

    gain0 = iir_convert_db_to_multiple(cfg->gain0);
    gain1 = iir_convert_db_to_multiple(cfg->gain1);

    // FILTER_DEBUG_TRACE(3,"[%s] gain0 = %f, gain1 = %f", __func__, (double)gain0, (double)gain1);
    // FILTER_DEBUG_TRACE(2,"[%s] len = %d", __func__, cfg->num);

    memset(&hw_codec_iir_eq, 0, sizeof(HW_CODEC_IIR_CFG_T));

    if (cfg->num == 0) {
        if (cfg->gain0 != 0.f) {
            coefs_l[0]=1.0f;
            coefs_l[1]=0.0f;
            coefs_l[2]=0.0f;
            coefs_l[3]=gain0;
            coefs_l[4]=0.0f;
            coefs_l[5]=0.0f;

            hw_codec_iir_eq.iir_filtes_l.iir_coef[0].coef_a[0]=(int32_t)((double)coefs_l[0]*(int32_t)IIR_FIXED_Q);
            hw_codec_iir_eq.iir_filtes_l.iir_coef[0].coef_a[1]=(int32_t)((double)coefs_l[1]*(int32_t)IIR_FIXED_Q);
            hw_codec_iir_eq.iir_filtes_l.iir_coef[0].coef_a[2]=(int32_t)((double)coefs_l[2]*(int32_t)IIR_FIXED_Q);
            hw_codec_iir_eq.iir_filtes_l.iir_coef[0].coef_b[0]=(int32_t)((double)coefs_l[3]*(int32_t)IIR_FIXED_Q);
            hw_codec_iir_eq.iir_filtes_l.iir_coef[0].coef_b[1]=(int32_t)((double)coefs_l[4]*(int32_t)IIR_FIXED_Q);
            hw_codec_iir_eq.iir_filtes_l.iir_coef[0].coef_b[2]=(int32_t)((double)coefs_l[5]*(int32_t)IIR_FIXED_Q);

            hw_codec_iir_eq.iir_filtes_l.iir_bypass_flag=0;
            hw_codec_iir_eq.iir_filtes_l.iir_counter=1;
        } else {
            hw_codec_iir_eq.iir_filtes_l.iir_bypass_flag=1;
            hw_codec_iir_eq.iir_filtes_l.iir_counter=0;
        }

        if (cfg->gain1 != 0.f) {
            coefs_r[0]=1.0f;
            coefs_r[1]=0.0f;
            coefs_r[2]=0.0f;
            coefs_r[3]=gain1;
            coefs_r[4]=0.0f;
            coefs_r[5]=0.0f;

            hw_codec_iir_eq.iir_filtes_r.iir_coef[0].coef_a[0]=(int32_t)((double)coefs_r[0]*(int32_t)IIR_FIXED_Q);
            hw_codec_iir_eq.iir_filtes_r.iir_coef[0].coef_a[1]=(int32_t)((double)coefs_r[1]*(int32_t)IIR_FIXED_Q);
            hw_codec_iir_eq.iir_filtes_r.iir_coef[0].coef_a[2]=(int32_t)((double)coefs_r[2]*(int32_t)IIR_FIXED_Q);
            hw_codec_iir_eq.iir_filtes_r.iir_coef[0].coef_b[0]=(int32_t)((double)coefs_r[3]*(int32_t)IIR_FIXED_Q);
            hw_codec_iir_eq.iir_filtes_r.iir_coef[0].coef_b[1]=(int32_t)((double)coefs_r[4]*(int32_t)IIR_FIXED_Q);
            hw_codec_iir_eq.iir_filtes_r.iir_coef[0].coef_b[2]=(int32_t)((double)coefs_r[5]*(int32_t)IIR_FIXED_Q);

            hw_codec_iir_eq.iir_filtes_r.iir_bypass_flag=0;
            hw_codec_iir_eq.iir_filtes_r.iir_counter=1;
        } else {
            hw_codec_iir_eq.iir_filtes_r.iir_bypass_flag=1;
            hw_codec_iir_eq.iir_filtes_r.iir_counter=0;
        }
    } else {
        for(int i=0; i<cfg->num; i++)
        {
            if (cfg->param[i].fc<((float)sample_rate_iir/2))
            {
                iir_coefs_generate(cfg->param[i].type,
                                cfg->param[i].gain,
                                cfg->param[i].fc / sample_rate_iir,
                                cfg->param[i].Q,
                                coefs_l);
            }
            else
            {
                FILTER_DEBUG_TRACE(2,"[%s]Error iir parameters i = %d", __func__, i);
                coefs_l[0]=1.0f;
                coefs_l[1]=0.0f;
                coefs_l[2]=0.0f;
                coefs_l[3]=1.0f;
                coefs_l[4]=0.0f;
                coefs_l[5]=0.0f;
            }

            coefs_r[0]=coefs_l[0];
            coefs_r[1]=coefs_l[1];
            coefs_r[2]=coefs_l[2];
            coefs_r[3]=coefs_l[3];
            coefs_r[4]=coefs_l[4];
            coefs_r[5]=coefs_l[5];

            if(i==0)
            {
                coefs_l[3] = coefs_l[3]*gain0;
                coefs_l[4] = coefs_l[4]*gain0;
                coefs_l[5] = coefs_l[5]*gain0;

                coefs_r[3] = coefs_r[3]*gain1;
                coefs_r[4] = coefs_r[4]*gain1;
                coefs_r[5] = coefs_r[5]*gain1;
            }

            hw_codec_iir_eq.iir_filtes_l.iir_coef[i].coef_a[0]=(int32_t)((double)coefs_l[0]*(int32_t)IIR_FIXED_Q);
            hw_codec_iir_eq.iir_filtes_l.iir_coef[i].coef_a[1]=(int32_t)((double)coefs_l[1]*(int32_t)IIR_FIXED_Q);
            hw_codec_iir_eq.iir_filtes_l.iir_coef[i].coef_a[2]=(int32_t)((double)coefs_l[2]*(int32_t)IIR_FIXED_Q);
            hw_codec_iir_eq.iir_filtes_l.iir_coef[i].coef_b[0]=(int32_t)((double)coefs_l[3]*(int32_t)IIR_FIXED_Q);
            hw_codec_iir_eq.iir_filtes_l.iir_coef[i].coef_b[1]=(int32_t)((double)coefs_l[4]*(int32_t)IIR_FIXED_Q);
            hw_codec_iir_eq.iir_filtes_l.iir_coef[i].coef_b[2]=(int32_t)((double)coefs_l[5]*(int32_t)IIR_FIXED_Q);

            hw_codec_iir_eq.iir_filtes_r.iir_coef[i].coef_a[0]=(int32_t)((double)coefs_r[0]*(int32_t)IIR_FIXED_Q);
            hw_codec_iir_eq.iir_filtes_r.iir_coef[i].coef_a[1]=(int32_t)((double)coefs_r[1]*(int32_t)IIR_FIXED_Q);
            hw_codec_iir_eq.iir_filtes_r.iir_coef[i].coef_a[2]=(int32_t)((double)coefs_r[2]*(int32_t)IIR_FIXED_Q);
            hw_codec_iir_eq.iir_filtes_r.iir_coef[i].coef_b[0]=(int32_t)((double)coefs_r[3]*(int32_t)IIR_FIXED_Q);
            hw_codec_iir_eq.iir_filtes_r.iir_coef[i].coef_b[1]=(int32_t)((double)coefs_r[4]*(int32_t)IIR_FIXED_Q);
            hw_codec_iir_eq.iir_filtes_r.iir_coef[i].coef_b[2]=(int32_t)((double)coefs_r[5]*(int32_t)IIR_FIXED_Q);

        }

        /*
        FILTER_DEBUG_TRACE(2,"\n[%s] coef_a = %d\n", __func__, hw_codec_iir_eq.iir_filtes_l.iir_coef[0].coef_a[0]);
        FILTER_DEBUG_TRACE(2,"[%s] coef_a = %d\n", __func__, hw_codec_iir_eq.iir_filtes_l.iir_coef[0].coef_a[1]);
        FILTER_DEBUG_TRACE(2,"[%s] coef_a = %d\n", __func__, hw_codec_iir_eq.iir_filtes_l.iir_coef[0].coef_a[2]);
        FILTER_DEBUG_TRACE(2,"[%s] coef_a = %d\n", __func__, hw_codec_iir_eq.iir_filtes_l.iir_coef[0].coef_b[0]);
        FILTER_DEBUG_TRACE(2,"[%s] coef_a = %d\n", __func__, hw_codec_iir_eq.iir_filtes_l.iir_coef[0].coef_b[1]);
        FILTER_DEBUG_TRACE(2,"[%s] coef_a = %d\n", __func__, hw_codec_iir_eq.iir_filtes_l.iir_coef[0].coef_b[2]);
        */
        if(cfg->num>=0)
        {
            hw_codec_iir_eq.iir_filtes_l.iir_bypass_flag=0;
            hw_codec_iir_eq.iir_filtes_r.iir_bypass_flag=0;

            hw_codec_iir_eq.iir_filtes_l.iir_counter=cfg->num;
            hw_codec_iir_eq.iir_filtes_r.iir_counter=cfg->num;
        }
    }

    return &hw_codec_iir_eq;
}

static inline void hw_codec_iir_convert_coef(hw_codec_iir_coefs *coef_t, hw_codec_iir_coefs_f *coef_f)
{
    coef_t->coef_a[0] = (int32_t)(coef_f->coef_a[0] / coef_f->coef_a[0] * (int32_t)IIR_FIXED_Q);
    coef_t->coef_a[1] = (int32_t)(coef_f->coef_a[1] / coef_f->coef_a[0] * (int32_t)IIR_FIXED_Q);
    coef_t->coef_a[2] = (int32_t)(coef_f->coef_a[2] / coef_f->coef_a[0] * (int32_t)IIR_FIXED_Q);
    coef_t->coef_b[0] = (int32_t)(coef_f->coef_b[0] / coef_f->coef_a[0] * (int32_t)IIR_FIXED_Q);
    coef_t->coef_b[1] = (int32_t)(coef_f->coef_b[1] / coef_f->coef_a[0] * (int32_t)IIR_FIXED_Q);
    coef_t->coef_b[2] = (int32_t)(coef_f->coef_b[2] / coef_f->coef_a[0] * (int32_t)IIR_FIXED_Q);
}

static void hw_codec_iir_convert_filter(HW_CODEC_IIR_FILTERS_T *cfg_t, HW_CODEC_IIR_FILTERS_F *cfg_f)
{
    cfg_t->iir_bypass_flag = cfg_f->iir_bypass_flag;
    cfg_t->iir_counter = cfg_f->iir_counter;
    for (int i = 0; i < cfg_t->iir_counter; i++) {
        hw_codec_iir_convert_coef(&cfg_t->iir_coef[i], &cfg_f->iir_coef[i]);
    }
}

HW_CODEC_IIR_CFG_T *hw_codec_iir_convert_cfg(HW_CODEC_IIR_CFG_F *cfg)
{
    hw_codec_iir_convert_filter(&hw_codec_iir_eq.iir_filtes_l, &cfg->iir_filtes_l);
    hw_codec_iir_convert_filter(&hw_codec_iir_eq.iir_filtes_r, &cfg->iir_filtes_r);

    return &hw_codec_iir_eq;
}

HW_IIR_CFG_T *hw_iir_get_cfg(enum AUD_SAMPRATE_T sample_rate, const IIR_CFG_T* ext_cfg)
{
    const IIR_CFG_T *cfg;
    float coefs_l[6],coefs_r[6];
    float gain0,gain1;

    if (ext_cfg)
    {
        cfg = ext_cfg;
    }
    else
    {
        FILTER_DEBUG_TRACE(0,"Error HW IIR cfg!");
        return NULL;
    }

    FILTER_DEBUG_TRACE(3,"[%s]sample_rate:%d,iir num:%d.", __func__,sample_rate,cfg->num);

    // Check parameter
    ASSERT(cfg->num <= AUD_IIR_NUM_EQ, "[%s] num(%d) is too large", __func__, cfg->num);

    gain0 = iir_convert_db_to_multiple(cfg->gain0);
    gain1 = iir_convert_db_to_multiple(cfg->gain1);

    // FILTER_DEBUG_TRACE(3,"[%s] gain0 = %f, gain1 = %f", __func__, (double)gain0, (double)gain1);

    memset(&hw_iir_eq, 0, sizeof(HW_IIR_CFG_T));

    if (cfg->num == 0) {
        if (cfg->gain0 != 0.f || cfg->gain1 != 0.f) {
            coefs_l[0]=1.0f;
            coefs_l[1]=0.0f;
            coefs_l[2]=0.0f;
            coefs_l[3]=gain0;
            coefs_l[4]=0.0f;
            coefs_l[5]=0.0f;

            coefs_r[0]=1.0f;
            coefs_r[1]=0.0f;
            coefs_r[2]=0.0f;
            coefs_r[3]=gain1;
            coefs_r[4]=0.0f;
            coefs_r[5]=0.0f;

            hw_iir_eq.iir_filtes_l.iir_coef[0].coef_a[0]=(int32_t)((double)coefs_l[0]*(int32_t)IIR_FIXED_Q);
            hw_iir_eq.iir_filtes_l.iir_coef[0].coef_a[1]=(int32_t)((double)coefs_l[1]*(int32_t)IIR_FIXED_Q);
            hw_iir_eq.iir_filtes_l.iir_coef[0].coef_a[2]=(int32_t)((double)coefs_l[2]*(int32_t)IIR_FIXED_Q);
            hw_iir_eq.iir_filtes_l.iir_coef[0].coef_b[0]=(int32_t)((double)coefs_l[3]*(int32_t)IIR_FIXED_Q);
            hw_iir_eq.iir_filtes_l.iir_coef[0].coef_b[1]=(int32_t)((double)coefs_l[4]*(int32_t)IIR_FIXED_Q);
            hw_iir_eq.iir_filtes_l.iir_coef[0].coef_b[2]=(int32_t)((double)coefs_l[5]*(int32_t)IIR_FIXED_Q);

            hw_iir_eq.iir_filtes_r.iir_coef[0].coef_a[0]=(int32_t)((double)coefs_r[0]*(int32_t)IIR_FIXED_Q);
            hw_iir_eq.iir_filtes_r.iir_coef[0].coef_a[1]=(int32_t)((double)coefs_r[1]*(int32_t)IIR_FIXED_Q);
            hw_iir_eq.iir_filtes_r.iir_coef[0].coef_a[2]=(int32_t)((double)coefs_r[2]*(int32_t)IIR_FIXED_Q);
            hw_iir_eq.iir_filtes_r.iir_coef[0].coef_b[0]=(int32_t)((double)coefs_r[3]*(int32_t)IIR_FIXED_Q);
            hw_iir_eq.iir_filtes_r.iir_coef[0].coef_b[1]=(int32_t)((double)coefs_r[4]*(int32_t)IIR_FIXED_Q);
            hw_iir_eq.iir_filtes_r.iir_coef[0].coef_b[2]=(int32_t)((double)coefs_r[5]*(int32_t)IIR_FIXED_Q);

            hw_iir_eq.iir_filtes_l.iir_counter=1;
            hw_iir_eq.iir_filtes_r.iir_counter=1;
        } else {
            hw_iir_eq.iir_filtes_l.iir_counter=0;
            hw_iir_eq.iir_filtes_r.iir_counter=0;
        }
    } else {
        for(int i=0; i<cfg->num; i++)
        {
            if (cfg->param[i].fc<((float)sample_rate/2))
            {
                iir_coefs_generate(cfg->param[i].type,
                                cfg->param[i].gain,
                                cfg->param[i].fc / sample_rate,
                                cfg->param[i].Q,
                                coefs_l);
            }
            else
            {
                FILTER_DEBUG_TRACE(2,"[%s]Error iir parameters i = %d", __func__, i);
                coefs_l[0]=1.0f;
                coefs_l[1]=0.0f;
                coefs_l[2]=0.0f;
                coefs_l[3]=1.0f;
                coefs_l[4]=0.0f;
                coefs_l[5]=0.0f;
            }

            coefs_r[0]=coefs_l[0];
            coefs_r[1]=coefs_l[1];
            coefs_r[2]=coefs_l[2];
            coefs_r[3]=coefs_l[3];
            coefs_r[4]=coefs_l[4];
            coefs_r[5]=coefs_l[5];

            if(i==0)
            {
                coefs_l[3] = coefs_l[3]*gain0;
                coefs_l[4] = coefs_l[4]*gain0;
                coefs_l[5] = coefs_l[5]*gain0;

                coefs_r[3] = coefs_r[3]*gain1;
                coefs_r[4] = coefs_r[4]*gain1;
                coefs_r[5] = coefs_r[5]*gain1;
            }

            hw_iir_eq.iir_filtes_l.iir_coef[i].coef_a[0]=(int32_t)((double)coefs_l[0]*(int32_t)IIR_FIXED_Q);
            hw_iir_eq.iir_filtes_l.iir_coef[i].coef_a[1]=(int32_t)((double)coefs_l[1]*(int32_t)IIR_FIXED_Q);
            hw_iir_eq.iir_filtes_l.iir_coef[i].coef_a[2]=(int32_t)((double)coefs_l[2]*(int32_t)IIR_FIXED_Q);
            hw_iir_eq.iir_filtes_l.iir_coef[i].coef_b[0]=(int32_t)((double)coefs_l[3]*(int32_t)IIR_FIXED_Q);
            hw_iir_eq.iir_filtes_l.iir_coef[i].coef_b[1]=(int32_t)((double)coefs_l[4]*(int32_t)IIR_FIXED_Q);
            hw_iir_eq.iir_filtes_l.iir_coef[i].coef_b[2]=(int32_t)((double)coefs_l[5]*(int32_t)IIR_FIXED_Q);

            hw_iir_eq.iir_filtes_r.iir_coef[i].coef_a[0]=(int32_t)((double)coefs_r[0]*(int32_t)IIR_FIXED_Q);
            hw_iir_eq.iir_filtes_r.iir_coef[i].coef_a[1]=(int32_t)((double)coefs_r[1]*(int32_t)IIR_FIXED_Q);
            hw_iir_eq.iir_filtes_r.iir_coef[i].coef_a[2]=(int32_t)((double)coefs_r[2]*(int32_t)IIR_FIXED_Q);
            hw_iir_eq.iir_filtes_r.iir_coef[i].coef_b[0]=(int32_t)((double)coefs_r[3]*(int32_t)IIR_FIXED_Q);
            hw_iir_eq.iir_filtes_r.iir_coef[i].coef_b[1]=(int32_t)((double)coefs_r[4]*(int32_t)IIR_FIXED_Q);
            hw_iir_eq.iir_filtes_r.iir_coef[i].coef_b[2]=(int32_t)((double)coefs_r[5]*(int32_t)IIR_FIXED_Q);

        }

        /*
        FILTER_DEBUG_TRACE(2,"\n[%s] coef_a0 = %d\n", __func__, hw_iir_eq.iir_filtes_l.iir_coef[0].coef_a[0]);
        FILTER_DEBUG_TRACE(2,"[%s] coef_a1 = %d\n", __func__, hw_iir_eq.iir_filtes_l.iir_coef[0].coef_a[1]);
        FILTER_DEBUG_TRACE(2,"[%s] coef_a2 = %d\n", __func__, hw_iir_eq.iir_filtes_l.iir_coef[0].coef_a[2]);
        FILTER_DEBUG_TRACE(2,"[%s] coef_b0 = %d\n", __func__, hw_iir_eq.iir_filtes_l.iir_coef[0].coef_b[0]);
        FILTER_DEBUG_TRACE(2,"[%s] coef_b1 = %d\n", __func__, hw_iir_eq.iir_filtes_l.iir_coef[0].coef_b[1]);
        FILTER_DEBUG_TRACE(2,"[%s] coef_b2 = %d\n", __func__, hw_iir_eq.iir_filtes_l.iir_coef[0].coef_b[2]);
        */
        if(cfg->num>=0)
        {
            hw_iir_eq.iir_filtes_l.iir_counter=cfg->num;
            hw_iir_eq.iir_filtes_r.iir_counter=cfg->num;
        }
    }

    return &hw_iir_eq;
}

#ifdef VQE_SIMULATE
#include <stdio.h>

void fprintf_vec_f32(FILE *fd, char *name, float *vec, int len)
{
    fprintf(fd, "%s = [", name);
    for (int i = 0; i < len - 1; i++) {
        fprintf(fd, "%f, ", vec[i]);
    }
    fprintf(fd, "%f]; \n", vec[len - 1]);
}
#endif

void iir_cfg_sos2tf(enum AUD_SAMPRATE_T sample_rate, const IIR_CFG_T* cfg, float *b, float *a, float *work_buffer)
{
    int offset = 0;
    float coefs[6];

    memset(coefs,0,sizeof(coefs));
    for (int i = 0; i < cfg->num; i++)
    {
        if (cfg->param[i].fc < (float)(sample_rate / 2))
        {
            iir_coefs_generate(cfg->param[i].type,
                cfg->param[i].gain,
                cfg->param[i].fc / (float)sample_rate,
                cfg->param[i].Q,
                coefs);
        }

#ifdef VQE_SIMULATE
        fprintf(stdout, "a = [%f, %f, %f]; b = [%f, %f, %f]\n", coefs[0], coefs[1], coefs[2], coefs[3], coefs[4], coefs[5]);
#endif

        if (i == 0) {
            a[0] = coefs[0];
            a[1] = coefs[1];
            a[2] = coefs[2];
            b[0] = coefs[3];
            b[1] = coefs[4];
            b[2] = coefs[5];

            offset += 3;
        } else {
            arm_copy_f32(a, work_buffer, offset);
            arm_conv_f32(work_buffer, offset, coefs, 3, a);

            arm_copy_f32(b, work_buffer, offset);
            arm_conv_f32(work_buffer, offset, coefs + 3, 3, b);

            offset += 2;
        }
    }

#ifdef VQE_SIMULATE
    fprintf_vec_f32(stdout, "a", a, offset);
    fprintf_vec_f32(stdout, "b", b, offset);
#endif
}

void iir_cfg_sos2arm(enum AUD_SAMPRATE_T sample_rate, const IIR_CFG_T* cfg, float *pCoeffs)
{
    float *p_coeffs = pCoeffs;
    float coefs[6];

    memset(coefs,0,sizeof(coefs));
    for (int i = 0; i < cfg->num; i++)
    {
        if (cfg->param[i].fc < sample_rate / 2)
        {
            iir_coefs_generate(cfg->param[i].type,
                cfg->param[i].gain,
                cfg->param[i].fc / sample_rate,
                cfg->param[i].Q,
                coefs);
        }

#ifdef VQE_SIMULATE
        fprintf(stdout, "a = [%f, %f, %f]; b = [%f, %f, %f]\n", coefs[0], coefs[1], coefs[2], coefs[3], coefs[4], coefs[5]);
#endif

        p_coeffs[0] = coefs[3];
        p_coeffs[1] = coefs[4];
        p_coeffs[2] = coefs[5];
        p_coeffs[3] = -coefs[1];
        p_coeffs[4] = -coefs[2];

        p_coeffs += 5;
    }

#ifdef VQE_SIMULATE
    fprintf_vec_f32(stdout, "pCoeffs", pCoeffs, cfg->num * 5);
#endif
}

#define FREQZ_USE_FILTER

uint32_t iir_cfg_find_peak_needed_size(uint32_t stages, int32_t nfft)
{
#if defined(FREQZ_USE_FILTER)
    /*                      b_len + a_len + h + h_abs + vec_buffer for freqz + coeff num + state num */
    return sizeof(float) * (nfft + 0 + nfft + (nfft / 2 + 1) + (nfft * 2) + stages * 5 + stages * 4);
#else
    /*                      b_len + a_len +          h +       h_abs +      vec_buffer for freqz */
    return sizeof(float) * ((2 * stages + 1) * 2 + nfft + nfft / 2 + 1 + nfft * 2);
#endif
}

float iir_cfg_find_peak(enum AUD_SAMPRATE_T sample_rate, const IIR_CFG_T* cfg, int32_t nfft, float *peak_idx, float *work_buffer)
{
    int ps_size = nfft / 2 + 1;
    int stages = cfg->num;

#if defined(FREQZ_USE_FILTER)
    int b_len = nfft;
    int a_len = 0;

    float* b = work_buffer;
    float* a = b + b_len;
    float* h = a + a_len;
    float* h_abs = h + nfft;
    float* vec_buffer = h_abs + ps_size;
    float* pCoeffs = vec_buffer + nfft * 2;
    float* pState = pCoeffs + stages * 5;

    memset(b, 0, nfft * sizeof(float));
    b[0] = 1.f;

    iir_cfg_sos2arm(sample_rate, cfg, pCoeffs);

    arm_biquad_casd_df1_inst_f32 band;

    arm_biquad_cascade_df1_init_f32(&band, stages, pCoeffs, pState);

    arm_biquad_cascade_df1_f32(&band, b, b, nfft);
#else
    int b_len = 2 * stages + 1;
    int a_len = 2 * stages + 1;

    float *b = work_buffer;
    float *a = b + b_len;
    float *h = a + a_len;
    float *h_abs = h + nfft;
    float *vec_buffer = h_abs + ps_size;

    iir_cfg_sos2tf(sample_rate, cfg, b, a, vec_buffer);
#endif

    freqz(b, b_len, a, a_len, h, nfft, vec_buffer);
    freqz_mag(h, nfft, h_abs);

    float peak = find_peak(h_abs, ps_size, peak_idx);

#ifdef VQE_SIMULATE
    FILE *fd = fopen("test_freqz.m", "w");
    fprintf(fd, "h1 = [");
    for (int i = 0; i < ps_size - 1; i++) {
        fprintf(fd, "%f, ", h_abs[i]);
    }
    fprintf(fd, "%f];\n", h_abs[ps_size - 1]);
    fprintf(fd, "w1 = linspace(0, pi, length(h1));\n\n");
    fprintf_vec_f32(fd, "a", a, a_len);
    fprintf_vec_f32(fd, "b", b, b_len);
    fprintf(fd, "[h2, w2] = freqz(b, a, %d);\n", ps_size);
    fprintf(fd, "figure;\n");
    fprintf(fd, "subplot(2, 1, 1)\n");
    fprintf(fd, "plot(w1, lin2db(abs(h1)), 'r', w2, lin2db(abs(h2)), 'g');\n");
    fprintf(fd, "subplot(2, 1, 2)\n");
    fprintf(fd, "plot(w1, lin2db(h1(:))- lin2db(abs(h2(:))));\n");
    fclose(fd);
#endif

    return peak;
}
