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
#include "aud_section.h"
#include "aud_section_inc.h"
#include "tgt_hardware.h"
#include "hal_trace.h"
#include "anc_process.h"
#include "hal_location.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"

#ifdef USB_ANC_MC_EQ_TUNING
#include "string.h"
#include "hal_timer.h"
#include "hal_aud.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_iomux.h"
#include "hal_codec.h"
#include "audioflinger.h"
#include "pmu.h"
#include "analog.h"
#include "hal_norflash.h"
#include "stdint.h"
#include "crc32_c.h"
#include "hw_iir_process.h"
#include "hw_codec_iir_process.h"
#endif

#if defined(PSAP_APP)
#include "psap_process.h"
#endif

#ifdef AUDIO_ANC_FB_MC
#include "cmsis.h"
#endif

#ifdef ANC_COEF_LIST_NUM
#if (ANC_COEF_LIST_NUM < 1)
#error "Invalid ANC_COEF_LIST_NUM configuration"
#endif
#else
#define ANC_COEF_LIST_NUM                   (1)
#endif

extern const struct_anc_cfg * anc_coef_list_50p7k[ANC_COEF_LIST_NUM];
extern const struct_anc_cfg * anc_coef_list_48k[ANC_COEF_LIST_NUM];
extern const struct_anc_cfg * anc_coef_list_44p1k[ANC_COEF_LIST_NUM];

const struct_anc_cfg * WEAK anc_coef_list_50p7k[ANC_COEF_LIST_NUM] = { };
const struct_anc_cfg * WEAK anc_coef_list_48k[ANC_COEF_LIST_NUM] = { };
const struct_anc_cfg * WEAK anc_coef_list_44p1k[ANC_COEF_LIST_NUM] = { };

#if defined(PSAP_APP)
extern const struct_psap_cfg * psap_coef_list_50p7k[PSAP_COEF_LIST_NUM];
extern const struct_psap_cfg * psap_coef_list_48k[PSAP_COEF_LIST_NUM];
extern const struct_psap_cfg * psap_coef_list_44p1k[PSAP_COEF_LIST_NUM];

const struct_psap_cfg * WEAK psap_coef_list_50p7k[PSAP_COEF_LIST_NUM] = { };
const struct_psap_cfg * WEAK psap_coef_list_48k[PSAP_COEF_LIST_NUM] = { };
const struct_psap_cfg * WEAK psap_coef_list_44p1k[PSAP_COEF_LIST_NUM] = { };
#endif


#if defined(AUDIO_ANC_SPKCALIB_HW)
extern const struct_spkcalib_cfg * spkcalib_coef_list_50p7k[SPKCALIB_COEF_LIST_NUM];
extern const struct_spkcalib_cfg * spkcalib_coef_list_48k[SPKCALIB_COEF_LIST_NUM];
extern const struct_spkcalib_cfg * spkcalib_coef_list_44p1k[SPKCALIB_COEF_LIST_NUM];

const struct_spkcalib_cfg * WEAK spkcalib_coef_list_50p7k[SPKCALIB_COEF_LIST_NUM] = { };
const struct_spkcalib_cfg * WEAK spkcalib_coef_list_48k[SPKCALIB_COEF_LIST_NUM] = { };
const struct_spkcalib_cfg * WEAK spkcalib_coef_list_44p1k[SPKCALIB_COEF_LIST_NUM] = { };
#endif

static enum ANC_INDEX cur_coef_idx=ANC_INDEX_0;
static enum AUD_SAMPRATE_T cur_coef_samprate;

WEAK int anc_set_gain2_float(float gain)
{
    ASSERT(0,"[%s] not defined for current chip",__func__);
    return 0;
}
WEAK float anc_get_gain2_float(void){
    ASSERT(0,"[%s] not defined for current chip",__func__);
    return 0;
}

#ifdef AUDIO_ANC_FB_MC
typedef struct {
    aud_item anc_cfg_mc_l;
    aud_item anc_cfg_mc_r;
    float mc_history_l[AUD_IIR_NUM][4];
    float mc_history_r[AUD_IIR_NUM][4];
} IIR_MC_CFG_T;

static IIR_MC_CFG_T mc_iir_cfg;

#endif

#ifndef CODEC_OUTPUT_DEV
#define CODEC_OUTPUT_DEV                    CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV
#endif

int anc_load_cfg(void)
{
    int res = 0;
    const struct_anc_cfg **list;

    TRACE(0,"[%s]:AUD_SECTION_STRUCT_VERSION=%d!",__func__,AUD_SECTION_STRUCT_VERSION);

#if defined(PSAP_APP)
    const struct_psap_cfg **list_psap;
#endif

#if defined(AUDIO_ANC_SPKCALIB_HW)
    const struct_spkcalib_cfg **list_spkcalib;
#endif

    anc_set_ch_map(CODEC_OUTPUT_DEV);

#ifdef __AUDIO_RESAMPLE__
    res = anccfg_loadfrom_audsec(anc_coef_list_50p7k, anc_coef_list_48k, ANC_COEF_LIST_NUM);
    list = anc_coef_list_50p7k;
    TRACE(0,"50.7k!!!!");

    if(res) {
        TRACE(2,"[%s] WARNING(%d): Can not load anc coefficient from audio section!!!", __func__, res);
        } else {
        TRACE(1,"[%s] Load anc coefficient from audio section.", __func__);
#if (AUD_SECTION_STRUCT_VERSION == 1)
        TRACE(5,"[%s] L: gain = %d, len = %d, dac = %d, adc = %d", __func__, list[0]->anc_cfg_ff_l.total_gain, list[0]->anc_cfg_ff_l.fir_len, list[0]->anc_cfg_ff_l.dac_gain_offset, list[0]->anc_cfg_ff_l.adc_gain_offset);
        TRACE(5,"[%s] R: gain = %d, len = %d, dac = %d, adc = %d", __func__, list[0]->anc_cfg_ff_r.total_gain, list[0]->anc_cfg_ff_r.fir_len, list[0]->anc_cfg_ff_r.dac_gain_offset, list[0]->anc_cfg_ff_r.adc_gain_offset);
#elif  ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)
        for(int i = 0; i < ANC_COEF_LIST_NUM; i++) {
            TRACE(3,"appmode%d,FEEDFORWARD,L:gain %d,R:gain %d",i, list[i]->anc_cfg_ff_l.total_gain, list[i]->anc_cfg_ff_r.total_gain);
            for(int j = 0; j <AUD_IIR_NUM; j++)
            {
                TRACE(7,"appmode%d,iir coef ff 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x", i, \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_b[0], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_b[1], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_b[2], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_a[0], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_a[1], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_a[2]);
            }

            TRACE(3,"appmode%d,FEEDBACK,L:gain %d,R:gain %d",i, list[i]->anc_cfg_fb_l.total_gain, list[i]->anc_cfg_fb_r.total_gain);
            for(int j = 0; j <AUD_IIR_NUM; j++)
            {
                TRACE(7,"appmode%d,iir coef fb 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x", i, \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_b[0], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_b[1], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_b[2], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_a[0], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_a[1], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_a[2]);
            }
        }
#elif  ((AUD_SECTION_STRUCT_VERSION & 0x1) != 0)
        for(int i = 0; i < ANC_COEF_LIST_NUM; i++) {
            TRACE(2,"appmode%d,FEEDFORWARD,L:gain %d",i, list[i]->anc_cfg_ff_l.total_gain);
            for(int j = 0; j <AUD_IIR_NUM; j++)
            {
                TRACE(7,"appmode%d,iir coef ff 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x", i, \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_b[0], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_b[1], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_b[2], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_a[0], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_a[1], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_a[2]);
            }

            TRACE(2,"appmode%d,FEEDBACK,L:gain %d",i, list[i]->anc_cfg_fb_l.total_gain);
            for(int j = 0; j <AUD_IIR_NUM; j++)
            {
                TRACE(7,"appmode%d,iir coef fb 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x", i, \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_b[0], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_b[1], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_b[2], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_a[0], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_a[1], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_a[2]);
            }
        }
#endif
    }

#else
    res = anccfg_loadfrom_audsec(anc_coef_list_48k, anc_coef_list_44p1k, ANC_COEF_LIST_NUM);
    list = anc_coef_list_44p1k;
    TRACE(0,"44.1k!!!!");

    if(res) {
        TRACE(2,"[%s] WARNING(%d): Can not load anc coefficient from audio section!!!", __func__, res);
    } else {
        TRACE(1,"[%s] Load anc coefficient from audio section.", __func__);
#if (AUD_SECTION_STRUCT_VERSION == 1)
        TRACE(5,"[%s] L: gain = %d, len = %d, dac = %d, adc = %d", __func__, list[0]->anc_cfg_ff_l.total_gain, list[0]->anc_cfg_ff_l.fir_len, list[0]->anc_cfg_ff_l.dac_gain_offset, list[0]->anc_cfg_ff_l.adc_gain_offset);
        TRACE(5,"[%s] R: gain = %d, len = %d, dac = %d, adc = %d", __func__, list[0]->anc_cfg_ff_r.total_gain, list[0]->anc_cfg_ff_r.fir_len, list[0]->anc_cfg_ff_r.dac_gain_offset, list[0]->anc_cfg_ff_r.adc_gain_offset);
#elif  ((AUD_SECTION_STRUCT_VERSION & 0x1)==0)
        for(int i = 0; i < ANC_COEF_LIST_NUM; i++) {
            TRACE(3,"appmode%d,FEEDFORWARD,L:gain %d,R:gain %d",i, list[i]->anc_cfg_ff_l.total_gain, list[i]->anc_cfg_ff_r.total_gain);
            for(int j = 0; j <AUD_IIR_NUM; j++)
            {
                TRACE(7,"appmode%d,iir coef ff 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x", i, \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_b[0], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_b[1], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_b[2], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_a[0], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_a[1], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_a[2]);
            }

            TRACE(3,"appmode%d,FEEDBACK,L:gain %d,R:gain %d",i, list[i]->anc_cfg_fb_l.total_gain, list[i]->anc_cfg_fb_r.total_gain);
            for(int j = 0; j <AUD_IIR_NUM; j++)
            {
                TRACE(7,"appmode%d,iir coef fb 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x", i, \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_b[0], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_b[1], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_b[2], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_a[0], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_a[1], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_a[2]);
            }
        }
#elif  ((AUD_SECTION_STRUCT_VERSION & 0x1) != 0)
        for(int i = 0; i < ANC_COEF_LIST_NUM; i++) {
            TRACE(2,"appmode%d,FEEDFORWARD,L:gain %d",i, list[i]->anc_cfg_ff_l.total_gain);
            for(int j = 0; j <AUD_IIR_NUM; j++)
            {
                TRACE(7,"appmode%d,iir coef ff 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x", i, \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_b[0], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_b[1], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_b[2], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_a[0], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_a[1], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_a[2]);
            }

            TRACE(2,"appmode%d,FEEDBACK,L:gain %d",i, list[i]->anc_cfg_fb_l.total_gain);
            for(int j = 0; j <AUD_IIR_NUM; j++)
            {
                TRACE(7,"appmode%d,iir coef fb 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x", i, \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_b[0], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_b[1], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_b[2], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_a[0], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_a[1], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_a[2]);
            }
        }

#endif
    }

    res = anccfg_loadfrom_audsec(anc_coef_list_48k, anc_coef_list_44p1k, ANC_COEF_LIST_NUM);
    list = anc_coef_list_48k;
    TRACE(0,"48k!!!!");

    if(res) {
        TRACE(2,"[%s] WARNING(%d): Can not load anc coefficient from audio section!!!", __func__, res);
    } else {
        TRACE(1,"[%s] Load anc coefficient from audio section.", __func__);
#if (AUD_SECTION_STRUCT_VERSION == 1)
        TRACE(5,"[%s] L: gain = %d, len = %d, dac = %d, adc = %d", __func__, list[0]->anc_cfg_ff_l.total_gain, list[0]->anc_cfg_ff_l.fir_len, list[0]->anc_cfg_ff_l.dac_gain_offset, list[0]->anc_cfg_ff_l.adc_gain_offset);
        TRACE(5,"[%s] R: gain = %d, len = %d, dac = %d, adc = %d", __func__, list[0]->anc_cfg_ff_r.total_gain, list[0]->anc_cfg_ff_r.fir_len, list[0]->anc_cfg_ff_r.dac_gain_offset, list[0]->anc_cfg_ff_r.adc_gain_offset);
#elif  ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)
        for(int i = 0; i < ANC_COEF_LIST_NUM; i++) {
            TRACE(3,"appmode%d,FEEDFORWARD,L:gain %d,R:gain %d",i, list[i]->anc_cfg_ff_l.total_gain, list[i]->anc_cfg_ff_r.total_gain);
            for(int j = 0; j <AUD_IIR_NUM; j++)
            {
                TRACE(7,"appmode%d,iir coef ff 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x", i, \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_b[0], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_b[1], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_b[2], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_a[0], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_a[1], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_a[2]);
            }

            TRACE(3,"appmode%d,FEEDBACK,L:gain %d,R:gain %d",i, list[i]->anc_cfg_fb_l.total_gain, list[i]->anc_cfg_fb_r.total_gain);
            for(int j = 0; j <AUD_IIR_NUM; j++)
            {
                TRACE(7,"appmode%d,iir coef fb 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x", i, \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_b[0], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_b[1], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_b[2], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_a[0], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_a[1], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_a[2]);
            }
        }
#elif  ((AUD_SECTION_STRUCT_VERSION & 0x1) != 0)
        for(int i = 0; i < ANC_COEF_LIST_NUM; i++) {
            TRACE(2,"appmode%d,FEEDFORWARD,L:gain %d",i, list[i]->anc_cfg_ff_l.total_gain);
            for(int j = 0; j <AUD_IIR_NUM; j++)
            {
                TRACE(7,"appmode%d,iir coef ff 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x", i, \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_b[0], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_b[1], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_b[2], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_a[0], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_a[1], \
                list[i]->anc_cfg_ff_l.iir_coef[j].coef_a[2]);
            }

            TRACE(2,"appmode%d,FEEDBACK,L:gain %d",i, list[i]->anc_cfg_fb_l.total_gain);
            for(int j = 0; j <AUD_IIR_NUM; j++)
            {
                TRACE(7,"appmode%d,iir coef fb 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x", i, \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_b[0], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_b[1], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_b[2], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_a[0], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_a[1], \
                list[i]->anc_cfg_fb_l.iir_coef[j].coef_a[2]);
            }
        }
#endif
    }
#endif

#if defined(PSAP_APP)
#ifdef __AUDIO_RESAMPLE__
       res = psapcfg_loadfrom_audsec(psap_coef_list_50p7k, psap_coef_list_48k, PSAP_COEF_LIST_NUM);
       list_psap = psap_coef_list_50p7k;

#else
       res = psapcfg_loadfrom_audsec(psap_coef_list_48k, psap_coef_list_44p1k, PSAP_COEF_LIST_NUM);
       list_psap = psap_coef_list_48k;
#endif

    if(res)
       {
        TRACE(2,"[%s] WARNING(%d): Can not load PSAP coefficient from audio section!!!", __func__, res);
    }
       else
       {
        for(int i = 0; i < PSAP_COEF_LIST_NUM; i++)
              {
                     TRACE(2,"app mode: %d:",i);
            TRACE(2,"psap_total_gain: %d", list_psap[i]->psap_cfg_l.psap_total_gain);
            TRACE(2,"psap_band_num: %d",list_psap[i]->psap_cfg_l.psap_band_num);

            for(int j = 0; j <AUD_PSAP_IIR_NUM; j++)
            {
                            TRACE(7,"PSAP_IIR_NUM_%d:",j);

                TRACE(7,"iir0.coef_b: 0x%08x, 0x%08x, 0x%08x, 0x%08x",\
                list_psap[i]->psap_cfg_l.psap_iir_coef[j].iir0.coef_b[0], \
                list_psap[i]->psap_cfg_l.psap_iir_coef[j].iir0.coef_b[1], \
                list_psap[i]->psap_cfg_l.psap_iir_coef[j].iir0.coef_b[2], \
                list_psap[i]->psap_cfg_l.psap_iir_coef[j].iir0.coef_b[3]);
                TRACE(7,"iir0.coef_a: 0x%08x, 0x%08x, 0x%08x, 0x%08x",\
                list_psap[i]->psap_cfg_l.psap_iir_coef[j].iir0.coef_a[0], \
                list_psap[i]->psap_cfg_l.psap_iir_coef[j].iir0.coef_a[1], \
                list_psap[i]->psap_cfg_l.psap_iir_coef[j].iir0.coef_a[2], \
                list_psap[i]->psap_cfg_l.psap_iir_coef[j].iir0.coef_a[3]);

                TRACE(7,"iir1.coef_b: 0x%08x, 0x%08x, 0x%08x, 0x%08x",\
                list_psap[i]->psap_cfg_l.psap_iir_coef[j].iir1.coef_b[0], \
                list_psap[i]->psap_cfg_l.psap_iir_coef[j].iir1.coef_b[1], \
                list_psap[i]->psap_cfg_l.psap_iir_coef[j].iir1.coef_b[2], \
                list_psap[i]->psap_cfg_l.psap_iir_coef[j].iir1.coef_b[3]);
                TRACE(7,"iir1.coef_a: 0x%08x, 0x%08x, 0x%08x, 0x%08x",\
                list_psap[i]->psap_cfg_l.psap_iir_coef[j].iir1.coef_a[0], \
                list_psap[i]->psap_cfg_l.psap_iir_coef[j].iir1.coef_a[1], \
                list_psap[i]->psap_cfg_l.psap_iir_coef[j].iir1.coef_a[2], \
                list_psap[i]->psap_cfg_l.psap_iir_coef[j].iir1.coef_a[3]);
            }

            for(int j = 0; j <=AUD_PSAP_IIR_NUM; j++)
            {

                TRACE(2,"PSAP_BAND_NUM_%02d_GAIN: 0x%08x",i, list_psap[i]->psap_cfg_l.psap_band_gain[j]);
            }
        }
       }

#endif

#if defined(AUDIO_ANC_SPKCALIB_HW)
#ifdef __AUDIO_RESAMPLE__
       res = spkcalibcfg_loadfrom_audsec(spkcalib_coef_list_50p7k, spkcalib_coef_list_48k, SPKCALIB_COEF_LIST_NUM);
       list_spkcalib = spkcalib_coef_list_50p7k;
#else
       res = spkcalibcfg_loadfrom_audsec(spkcalib_coef_list_48k, spkcalib_coef_list_44p1k, SPKCALIB_COEF_LIST_NUM);
       list_spkcalib = spkcalib_coef_list_48k;
#endif

    if(res)
       {
        TRACE(2,"[%s] WARNING(%d): Can not load SPKCALIB coefficient from audio section!!!", __func__, res);
    }
       else
       {
        for(int i = 0; i < SPKCALIB_COEF_LIST_NUM; i++)
              {
            TRACE(2,"appmode%d,SPK CALIB,L:gain %d",i, list_spkcalib[i]->spkcalib_cfg_l.total_gain);
            for(int j = 0; j <AUD_IIR_NUM; j++)
            {
                TRACE(7,"appmode%d,iir coef ff 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x", i, \
                list_spkcalib[i]->spkcalib_cfg_l.iir_coef[j].coef_b[0], \
                list_spkcalib[i]->spkcalib_cfg_l.iir_coef[j].coef_b[1], \
                list_spkcalib[i]->spkcalib_cfg_l.iir_coef[j].coef_b[2], \
                list_spkcalib[i]->spkcalib_cfg_l.iir_coef[j].coef_a[0], \
                list_spkcalib[i]->spkcalib_cfg_l.iir_coef[j].coef_a[1], \
                list_spkcalib[i]->spkcalib_cfg_l.iir_coef[j].coef_a[2]);
            }
        }
       }

#endif

     return res;
}
#if defined(PSAP_APP)
int psap_select_coef(enum AUD_SAMPRATE_T rate,int index)
{
    const struct_psap_cfg **list=NULL;

    if (index >= PSAP_COEF_LIST_NUM) {
        return 1;
    }

    switch(rate)
    {
        case AUD_SAMPRATE_48000:
            list=psap_coef_list_48k;
            break;

        case AUD_SAMPRATE_44100:
            list=psap_coef_list_44p1k;
            break;

#ifdef __AUDIO_RESAMPLE__
        case AUD_SAMPRATE_50781:
            list=psap_coef_list_50p7k;
            break;
#endif

        default:
            break;
    }

    ASSERT(list!=NULL&&list[index]!=NULL,"The psap coefs of Samprate %d is NULL",  rate);

    if(psap_opened())
    {
        hal_sysfreq_req(HAL_SYSFREQ_USER_ANC, HAL_CMU_FREQ_104M);
        psap_set_cfg_coef(list[index]);
        hal_sysfreq_req(HAL_SYSFREQ_USER_ANC, HAL_CMU_FREQ_32K);
    }

    return 0;
}

#endif

#if defined(AUDIO_ANC_SPKCALIB_HW)
int spkcalib_select_coef(enum AUD_SAMPRATE_T rate,int index)
{
    const struct_spkcalib_cfg **list=NULL;

    if (index >= SPKCALIB_COEF_LIST_NUM) {
        return 1;
    }

    switch(rate)
    {
        case AUD_SAMPRATE_48000:
            list=spkcalib_coef_list_48k;
            break;

        case AUD_SAMPRATE_44100:
            list=spkcalib_coef_list_44p1k;
            break;

#ifdef __AUDIO_RESAMPLE__
        case AUD_SAMPRATE_50781:
            list=spkcalib_coef_list_50p7k;
            break;
#endif

        default:
            break;
    }

    ASSERT(list!=NULL&&list[index]!=NULL,"The spkcalib coefs of Samprate %d is NULL",  rate);

    if(anc_opened(ANC_SPKCALIB))
    {
        hal_sysfreq_req(HAL_SYSFREQ_USER_ANC, HAL_CMU_FREQ_104M);
        anc_set_spkcalib_cfg(list[index]);
        hal_sysfreq_req(HAL_SYSFREQ_USER_ANC, HAL_CMU_FREQ_32K);
    }

    return 0;
}

#endif


int anc_select_coef(enum AUD_SAMPRATE_T rate,enum ANC_INDEX index,enum ANC_TYPE_T anc_type,ANC_GAIN_TIME anc_gain_delay)
{
    const struct_anc_cfg **list=NULL;

    if (index >= ANC_COEF_LIST_NUM) {
        TRACE(0, "[%s] WARNING: anc_type: 0x%x, index(%d) >= ANC_COEF_LIST_NUM", __func__, anc_type, index);
        return 1;
    }

#ifdef CHIP_BEST1000
    switch(rate)
    {
        case AUD_SAMPRATE_96000:
            list=anc_coef_list_48k;
            break;

        case AUD_SAMPRATE_88200:
            list=anc_coef_list_44p1k;
            break;

        default:
            break;
    }

#else
    switch(rate)
    {
        case AUD_SAMPRATE_48000:
            list=anc_coef_list_48k;
            break;

        case AUD_SAMPRATE_44100:
            list=anc_coef_list_44p1k;
            break;

#ifdef __AUDIO_RESAMPLE__
        case AUD_SAMPRATE_50781:
            list=anc_coef_list_50p7k;
            break;
#endif

        default:
            break;
    }
#endif

    ASSERT(list!=NULL&&list[index]!=NULL,"The coefs of Samprate %d is NULL",  rate);

    if(anc_opened(anc_type))
    {
        hal_sysfreq_req(HAL_SYSFREQ_USER_ANC,HAL_CMU_FREQ_104M);
        anc_set_cfg(list[index],anc_type,anc_gain_delay);
        hal_sysfreq_req(HAL_SYSFREQ_USER_ANC,HAL_CMU_FREQ_32K);
#ifdef AUDIO_ANC_FB_MC
        mc_iir_cfg.anc_cfg_mc_l=(*list[index]).anc_cfg_mc_l;
#if ((AUD_SECTION_STRUCT_VERSION & 0x1) != 0)
        mc_iir_cfg.anc_cfg_mc_r=(*list[index]).anc_cfg_mc_r;
#endif
#endif
    }

    cur_coef_idx = index;
    cur_coef_samprate=rate;

    return 0;
}

enum ANC_INDEX anc_get_current_coef_index(void)
{
    return cur_coef_idx;
}

enum AUD_SAMPRATE_T anc_get_current_coef_samplerate(void)
{
    return cur_coef_samprate;
}

#ifdef AUDIO_ANC_FB_MC

void anc_mc_run_init(enum AUD_SAMPRATE_T rate)
{
    const struct_anc_cfg **list=NULL;

    TRACE(0, "[%s] rate = %d", __func__, rate);

    switch(rate)
    {
        case AUD_SAMPRATE_48000:
        list=anc_coef_list_48k;
        break;

        case AUD_SAMPRATE_44100:
        list=anc_coef_list_44p1k;
        break;

#ifdef __AUDIO_RESAMPLE__
        case AUD_SAMPRATE_50781:
        list=anc_coef_list_50p7k;
        break;
#endif
        default:
            break;
    }

    ASSERT(list!=NULL&&list[cur_coef_idx]!=NULL,"The coefs of Samprate %d is NULL",  rate);

    mc_iir_cfg.anc_cfg_mc_l=(*list[cur_coef_idx]).anc_cfg_mc_l;
#if ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)
    mc_iir_cfg.anc_cfg_mc_r=(*list[cur_coef_idx]).anc_cfg_mc_r;
#endif

    for(int j=0;j<AUD_IIR_NUM;j++)
    {
        for(int i=0;i<4;i++)
        {
            mc_iir_cfg.mc_history_l[j][i]=0.0f;
#if ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)
            mc_iir_cfg.mc_history_r[j][i]=0.0f;
#endif
        }
    }
    return;
}


void anc_mc_run_setup(enum AUD_SAMPRATE_T rate)
{
    const struct_anc_cfg **list=NULL;

    switch(rate)
    {
        case AUD_SAMPRATE_48000:
        list=anc_coef_list_48k;
        break;

        case AUD_SAMPRATE_44100:
        list=anc_coef_list_44p1k;
        break;

#ifdef __AUDIO_RESAMPLE__
        case AUD_SAMPRATE_50781:
        list=anc_coef_list_50p7k;
        break;
#endif
        default:
            break;
    }

    ASSERT(list!=NULL&&list[cur_coef_idx]!=NULL,"The coefs of Samprate %d is NULL",  rate);

    mc_iir_cfg.anc_cfg_mc_l=(*list[cur_coef_idx]).anc_cfg_mc_l;
#if ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)
    mc_iir_cfg.anc_cfg_mc_r=(*list[cur_coef_idx]).anc_cfg_mc_r;
#endif

    return;
}


static inline int32_t iir_ssat_24bits(float in)
{
    int res = 0;
    int32_t out;

    res = (int)in;
    out = __SSAT(res,24);

    return out;
}

static inline int32_t iir_ssat_16bits(float in)
{
    int res = 0;
    int32_t out;

    res = (int)in;
    out = __SSAT(res,16);

    return out;
}

SRAM_TEXT_LOC int anc_mc_run_stereo(uint8_t *buf, int len,float left_gain,float right_gain,enum AUD_BITS_T sample_bit)
{
    int len_mono;
    float gain_l = 0, gain_r = 0;
    int32_t *coefs = NULL;
    float *history = NULL;

    float x0, x1, x2;
    float y0, y1, y2;

    // Coefs
    float a0, a1, a2;
    float b0, b1, b2;

    //ASSERT(mc_iir_cfg.anc_cfg_mc_l.iir_counter==mc_iir_cfg.anc_cfg_mc_r.iir_counter, "mc need the same counter in left and right ch L:%d,R:%d",
        //         mc_iir_cfg.anc_cfg_mc_l.iir_counter,mc_iir_cfg.anc_cfg_mc_r.iir_counter);

    if(sample_bit==AUD_BITS_16)
    {
        int16_t *iir_buf;

        len_mono = len>>2;

        gain_l = (mc_iir_cfg.anc_cfg_mc_l.total_gain/512.0f)*left_gain;

        if(mc_iir_cfg.anc_cfg_mc_l.iir_counter==0||mc_iir_cfg.anc_cfg_mc_l.iir_bypass_flag)
        {
            iir_buf = (int16_t *)buf;

            for(int j=0; j<len_mono; j++)
            {
                x0 = *iir_buf*gain_l;
                *iir_buf++ = iir_ssat_16bits(x0);
                iir_buf++;
            }
        }
        else
        {
            for(int i=0; i<mc_iir_cfg.anc_cfg_mc_l.iir_counter; i++)
            {
                // Coef
                coefs = mc_iir_cfg.anc_cfg_mc_l.iir_coef[i].coef_a;
                a0 = *coefs++;
                a1 = *coefs++;
                a2 = *coefs;
                coefs = mc_iir_cfg.anc_cfg_mc_l.iir_coef[i].coef_b;
                b0 = *coefs++;
                b1 = *coefs++;
                b2 = *coefs;

                a1=a1/a0;
                a2=a2/a0;
                b0=b0/a0;
                b1=b1/a0;
                b2=b2/a0;

              //TRACE(7,"[%d] %f, %f, %f, %f, %f, %f", i, a0, a1, a2, b0, b1, b2);

                // Left
                history = mc_iir_cfg.mc_history_l[i];
                x1 = *history++;
                x2 = *history++;
                y1 = *history++;
                y2 = *history;

                iir_buf = (int16_t *)buf;
                if(i==0)
                {
                    for(int j=0; j<len_mono; j++)
                    {
                        // Left channel
                        x0 = *iir_buf*gain_l;
                        y0 = x0*b0 + x1*b1 + x2*b2 - y1*a1 - y2*a2;
                        y2 = y1;
                        y1 = y0;
                        x2 = x1;
                        x1 = x0;
                        *iir_buf++ = iir_ssat_16bits(y0);
                        iir_buf++;
                    }
                }
                else
                {
                    iir_buf++;
                    for(int j=0; j<len_mono; j++)
                    {
                        // Left channel
                        x0 = *iir_buf;
                        y0 = x0*b0 + x1*b1 + x2*b2 - y1*a1 - y2*a2;
                        y2 = y1;
                        y1 = y0;
                        x2 = x1;
                        x1 = x0;
                        *iir_buf++ = iir_ssat_16bits(y0);
                        iir_buf++;
                    }
                }
                // Left
                history = mc_iir_cfg.mc_history_l[i];
                *history++ = x1;
                *history++ = x2;
                *history++ = y1;
                *history = y2;
            }
        }

        gain_r = (mc_iir_cfg.anc_cfg_mc_r.total_gain/512.0f)*right_gain;

        if(mc_iir_cfg.anc_cfg_mc_r.iir_counter==0||mc_iir_cfg.anc_cfg_mc_r.iir_bypass_flag)
        {
            iir_buf = (int16_t *)buf;
            iir_buf++;
            for(int j=0; j<len_mono; j++)
            {
                x0 = *iir_buf*gain_r;
                *iir_buf++ = iir_ssat_16bits(x0);
                iir_buf++;
            }
        }
        else
        {
            for(int i=0; i<mc_iir_cfg.anc_cfg_mc_r.iir_counter; i++)
            {
                // Coef
                coefs = mc_iir_cfg.anc_cfg_mc_r.iir_coef[i].coef_a;
                a0 = *coefs++;
                a1 = *coefs++;
                a2 = *coefs;
                coefs = mc_iir_cfg.anc_cfg_mc_r.iir_coef[i].coef_b;
                b0 = *coefs++;
                b1 = *coefs++;
                b2 = *coefs;

                a1=a1/a0;
                a2=a2/a0;
                b0=b0/a0;
                b1=b1/a0;
                b2=b2/a0;

              //TRACE(7,"[%d] %f, %f, %f, %f, %f, %f", i, a0, a1, a2, b0, b1, b2);

                // right
                history = mc_iir_cfg.mc_history_r[i];
                x1 = *history++;
                x2 = *history++;
                y1 = *history++;
                y2 = *history;

                iir_buf = (int16_t *)buf;
                iir_buf++;
                if(i==0)
                {
                    for(int j=0; j<len_mono; j++)
                    {
                        // right channel
                        x0 = *iir_buf*gain_r;
                        y0 = x0*b0 + x1*b1 + x2*b2 - y1*a1 - y2*a2;
                        y2 = y1;
                        y1 = y0;
                        x2 = x1;
                        x1 = x0;
                        *iir_buf++ = iir_ssat_16bits(y0);
                        iir_buf++;
                    }
                }
                else
                {
                    for(int j=0; j<len_mono; j++)
                    {
                        // right channel
                        x0 = *iir_buf;
                        y0 = x0*b0 + x1*b1 + x2*b2 - y1*a1 - y2*a2;
                        y2 = y1;
                        y1 = y0;
                        x2 = x1;
                        x1 = x0;
                        *iir_buf++ = iir_ssat_16bits(y0);
                        iir_buf++;
                    }
                }
                // right
                history = mc_iir_cfg.mc_history_r[i];
                *history++ = x1;
                *history++ = x2;
                *history++ = y1;
                *history = y2;
            }
        }

    }
    else if(sample_bit==AUD_BITS_24)
    {
        int32_t *iir_buf;

        len_mono = len>>3;

        gain_l = (mc_iir_cfg.anc_cfg_mc_l.total_gain/512.0f)*left_gain;

        if(mc_iir_cfg.anc_cfg_mc_l.iir_counter==0||mc_iir_cfg.anc_cfg_mc_l.iir_bypass_flag)
        {
            iir_buf = (int32_t *)buf;
            for(int j=0; j<len_mono; j++)
            {
                x0 = *iir_buf*gain_l;
                *iir_buf++ = iir_ssat_24bits(x0);
                iir_buf++;
            }
        }
        else
        {
            for(int i=0; i<mc_iir_cfg.anc_cfg_mc_l.iir_counter; i++)
            {
                // Coef
                coefs = mc_iir_cfg.anc_cfg_mc_l.iir_coef[i].coef_a;
                a0 = *coefs++;
                a1 = *coefs++;
                a2 = *coefs;
                coefs = mc_iir_cfg.anc_cfg_mc_l.iir_coef[i].coef_b;
                b0 = *coefs++;
                b1 = *coefs++;
                b2 = *coefs;

                a1=a1/a0;
                a2=a2/a0;
                b0=b0/a0;
                b1=b1/a0;
                b2=b2/a0;

              //TRACE(7,"[%d] %f, %f, %f, %f, %f, %f", i, a0, a1, a2, b0, b1, b2);

                // Left
                history = mc_iir_cfg.mc_history_l[i];
                x1 = *history++;
                x2 = *history++;
                y1 = *history++;
                y2 = *history;

                iir_buf = (int32_t *)buf;
                if(i==0)
                {
                    for(int j=0; j<len_mono; j++)
                    {
                        // Left channel
                        x0 = *iir_buf*gain_l;
                        y0 = x0*b0 + x1*b1 + x2*b2 - y1*a1 - y2*a2;
                        y2 = y1;
                        y1 = y0;
                        x2 = x1;
                        x1 = x0;
                        *iir_buf++ = iir_ssat_24bits(y0);
                        iir_buf++;
                    }
                }
                else
                {
                    for(int j=0; j<len_mono; j++)
                    {
                        // Left channel
                        x0 = *iir_buf;
                        y0 = x0*b0 + x1*b1 + x2*b2 - y1*a1 - y2*a2;
                        y2 = y1;
                        y1 = y0;
                        x2 = x1;
                        x1 = x0;
                        *iir_buf++ = iir_ssat_24bits(y0);
                        iir_buf++;
                    }
                }
                // Left
                history = mc_iir_cfg.mc_history_l[i];
                *history++ = x1;
                *history++ = x2;
                *history++ = y1;
                *history = y2;
            }
        }

         gain_r = (mc_iir_cfg.anc_cfg_mc_r.total_gain/512.0f)*right_gain;
        if(mc_iir_cfg.anc_cfg_mc_r.iir_counter==0||mc_iir_cfg.anc_cfg_mc_r.iir_bypass_flag)
        {
            iir_buf = (int32_t *)buf;
            iir_buf++;
            for(int j=0; j<len_mono; j++)
            {
                x0 = *iir_buf*gain_r;
                *iir_buf++ = iir_ssat_24bits(x0);
                iir_buf++;
            }

        }
        else
        {
            for(int i=0; i<mc_iir_cfg.anc_cfg_mc_r.iir_counter; i++)
            {
                // Coef
                coefs = mc_iir_cfg.anc_cfg_mc_r.iir_coef[i].coef_a;
                a0 = *coefs++;
                a1 = *coefs++;
                a2 = *coefs;
                coefs = mc_iir_cfg.anc_cfg_mc_r.iir_coef[i].coef_b;
                b0 = *coefs++;
                b1 = *coefs++;
                b2 = *coefs;

                a1=a1/a0;
                a2=a2/a0;
                b0=b0/a0;
                b1=b1/a0;
                b2=b2/a0;

              //TRACE(7,"[%d] %f, %f, %f, %f, %f, %f", i, a0, a1, a2, b0, b1, b2);

                // right
                history = mc_iir_cfg.mc_history_r[i];
                x1 = *history++;
                x2 = *history++;
                y1 = *history++;
                y2 = *history;

                iir_buf = (int32_t *)buf;
                iir_buf++;
                if(i==0)
                {
                    for(int j=0; j<len_mono; j++)
                    {
                        // right channel
                        x0 = *iir_buf*gain_r;
                        y0 = x0*b0 + x1*b1 + x2*b2 - y1*a1 - y2*a2;
                        y2 = y1;
                        y1 = y0;
                        x2 = x1;
                        x1 = x0;
                        *iir_buf++ = iir_ssat_24bits(y0);
                        iir_buf++;
                    }
                }
                else
                {
                    for(int j=0; j<len_mono; j++)
                    {
                        // right channel
                        x0 = *iir_buf;
                        y0 = x0*b0 + x1*b1 + x2*b2 - y1*a1 - y2*a2;
                        y2 = y1;
                        y1 = y0;
                        x2 = x1;
                        x1 = x0;
                        *iir_buf++ = iir_ssat_24bits(y0);
                        iir_buf++;
                    }
                }
                // right
                history = mc_iir_cfg.mc_history_r[i];
                *history++ = x1;
                *history++ = x2;
                *history++ = y1;
                *history = y2;
            }
        }

    }
    else
    {
        ASSERT(false, "Can't support sample bit mode:%d",sample_bit);
    }

    return 0;
}

SRAM_TEXT_LOC int anc_mc_run_mono(uint8_t *buf, int len,float left_gain,enum AUD_BITS_T sample_bit)
{
    int len_mono;
    int num;
    float gain_l = 0;
    int32_t *coefs = NULL;
    float *history = NULL;

    // Left
    float x0, x1, x2;
    float y0, y1, y2;

    // Coefs
    float POSSIBLY_UNUSED a0, a1, a2;
    float b0, b1, b2;

    if(sample_bit==AUD_BITS_16)
    {
        int16_t *iir_buf;

        len_mono = len>>1;

        gain_l = (mc_iir_cfg.anc_cfg_mc_l.total_gain/512.0f)*left_gain;
        num = mc_iir_cfg.anc_cfg_mc_l.iir_counter;

        if(num==0||mc_iir_cfg.anc_cfg_mc_l.iir_bypass_flag)
        {
            iir_buf = (int16_t *)buf;

            for(int j=0; j<len_mono; j++)
            {
                x0 = *iir_buf*gain_l;
                *iir_buf++ = iir_ssat_16bits(x0);
            }

            return 0;
        }

        for(int i=0; i<num; i++)
        {
            // Coef
            coefs = mc_iir_cfg.anc_cfg_mc_l.iir_coef[i].coef_a;
            a0 = *coefs++;
            a1 = *coefs++;
            a2 = *coefs;
            coefs = mc_iir_cfg.anc_cfg_mc_l.iir_coef[i].coef_b;
            b0 = *coefs++;
            b1 = *coefs++;
            b2 = *coefs;

            a1=a1/a0;
            a2=a2/a0;
            b0=b0/a0;
            b1=b1/a0;
            b2=b2/a0;

    //      TRACE(7,"[%d] %f, %f, %f, %f, %f, %f", i, a0, a1, a2, b0, b1, b2);

            // Left
            history = mc_iir_cfg.mc_history_l[i];
            x1 = *history++;
            x2 = *history++;
            y1 = *history++;
            y2 = *history;

            iir_buf = (int16_t *)buf;
            if(i==0)
            {
                for(int j=0; j<len_mono; j++)
                {
                    // Left channel
                    x0 = *iir_buf*gain_l;
                    y0 = x0*b0 + x1*b1 + x2*b2 - y1*a1 - y2*a2;
                    y2 = y1;
                    y1 = y0;
                    x2 = x1;
                    x1 = x0;
                    *iir_buf++ = iir_ssat_16bits(y0);
                }
            }
            else
            {
                for(int j=0; j<len_mono; j++)
                {
                    // Left channel
                    x0 = *iir_buf;
                    y0 = x0*b0 + x1*b1 + x2*b2 - y1*a1 - y2*a2;
                    y2 = y1;
                    y1 = y0;
                    x2 = x1;
                    x1 = x0;
                    *iir_buf++ = iir_ssat_16bits(y0);
                }
            }
            // Left
            history = mc_iir_cfg.mc_history_l[i];
            *history++ = x1;
            *history++ = x2;
            *history++ = y1;
            *history = y2;
        }

    }
    else if(sample_bit==AUD_BITS_24)
    {
        int32_t *iir_buf;

        len_mono = len>>2;

        gain_l = (mc_iir_cfg.anc_cfg_mc_l.total_gain/512.0f)*left_gain;
        num = mc_iir_cfg.anc_cfg_mc_l.iir_counter;

        if(num==0)
        {
            iir_buf = (int32_t *)buf;

            for(int j=0; j<len_mono; j++)
            {
                x0 = *iir_buf*gain_l;
                *iir_buf++ = iir_ssat_24bits(x0);
            }

            return 0;
        }

        for(int i=0; i<num; i++)
        {
            // Coef
            coefs = mc_iir_cfg.anc_cfg_mc_l.iir_coef[i].coef_a;
            a0 = *coefs++;
            a1 = *coefs++;
            a2 = *coefs;
            coefs = mc_iir_cfg.anc_cfg_mc_l.iir_coef[i].coef_b;
            b0 = *coefs++;
            b1 = *coefs++;
            b2 = *coefs;

            a1=a1/a0;
            a2=a2/a0;
            b0=b0/a0;
            b1=b1/a0;
            b2=b2/a0;


    //        TRACE(7,"[%d] %f, %f, %f, %f, %f, %f", i, a0, a1, a2, b0, b1, b2);

            // Left
            history = mc_iir_cfg.mc_history_l[i];
            x1 = *history++;
            x2 = *history++;
            y1 = *history++;
            y2 = *history;

            iir_buf = (int32_t *)buf;
            if(i==0)
            {
                for(int j=0; j<len_mono; j++)
                {
                    // Left channel
                    x0 = *iir_buf*gain_l;
                    y0 = x0*b0 + x1*b1 + x2*b2 - y1*a1 - y2*a2;
                    y2 = y1;
                    y1 = y0;
                    x2 = x1;
                    x1 = x0;
                    *iir_buf++ = iir_ssat_24bits(y0);
                }
            }
            else
            {
                for(int j=0; j<len_mono; j++)
                {
                    // Left channel
                    x0 = *iir_buf;
                    y0 = x0*b0 + x1*b1 + x2*b2 - y1*a1 - y2*a2;
                    y2 = y1;
                    y1 = y0;
                    x2 = x1;
                    x1 = x0;
                    *iir_buf++ = iir_ssat_24bits(y0);
                }
            }
            // Left
            history = mc_iir_cfg.mc_history_l[i];
            *history++ = x1;
            *history++ = x2;
            *history++ = y1;
            *history = y2;
        }

    }
    else
    {
        ASSERT(false, "Can't support sample bit mode:%d",sample_bit);
    }

    return 0;
}


#endif
#ifdef USB_ANC_MC_EQ_TUNING
struct message_t {
    struct msg_hdr_t {
        unsigned char prefix;
        unsigned char type;
        unsigned char seq;
        unsigned char len;
    } hdr;
    unsigned char data[255];
};

#define PREFIX_CHAR                         0xBE

enum MSG_TYPE {
    TYPE_SYS = 0x00,
    TYPE_READ = 0x01,
    TYPE_WRITE = 0x02,
    TYPE_BULK_READ = 0x03,
    TYPE_SYNC = 0x50,
    TYPE_SIG_INFO = 0x51,
    TYPE_SIG = 0x52,
    TYPE_CODE_INFO = 0x53,
    TYPE_CODE = 0x54,
    TYPE_RUN = 0x55,
    TYPE_SECTOR_SIZE = 0x60,
    TYPE_ERASE_BURN_START = 0x61,
    TYPE_ERASE_BURN_DATA = 0x62,
    TYPE_OBSOLETED_63 = 0x63,
    TYPE_OBSOLETED_64 = 0x64,
    TYPE_FLASH_CMD = 0x65,
    TYPE_GET_SECTOR_INFO = 0x66,
    TYPE_SEC_REG_ERASE_BURN_START = 0x67,
    TYPE_SEC_REG_ERASE_BURN_DATA = 0x68,

    // Extended types
    TYPE_PROD_TEST = 0x81,
    TYPE_RUNTIME_CMD = 0x82,
    TYPE_BT_CALIB_CMD = 0x83,
    TYPE_PROTO_EL = 0xA0,

    TYPE_INVALID = 0xFF,
};

enum ERR_CODE {
    ERR_NONE = 0x00,
    ERR_LEN = 0x01,
    ERR_CHECKSUM = 0x02,
    ERR_NOT_SYNC = 0x03,
    ERR_NOT_SEC = 0x04,
    ERR_SYNC_WORD = 0x05,
    ERR_SYS_CMD = 0x06,
    ERR_DATA_ADDR = 0x07,
    ERR_DATA_LEN = 0x08,
    ERR_ACCESS_RIGHT = 0x09,

    ERR_TYPE_INVALID = 0x0F,

    //ERR_BOOT_OK = 0x10,
    ERR_BOOT_MAGIC = 0x11,
    ERR_BOOT_SEC = 0x12,
    ERR_BOOT_HASH_TYPE = 0x13,
    ERR_BOOT_KEY_TYPE = 0x14,
    ERR_BOOT_KEY_LEN = 0x15,
    ERR_BOOT_SIG_LEN = 0x16,
    ERR_BOOT_SIG = 0x17,
    ERR_BOOT_CRC = 0x18,
    ERR_BOOT_LEN = 0x19,
    ERR_SIG_CODE_SIZE = 0x1A,
    ERR_SIG_SIG_LEN = 0x1B,
    ERR_SIG_INFO_MISSING = 0x1C,
    ERR_BOOT_KEY_ID = 0x1D,
    ERR_BOOT_HASH = 0x1E,

    ERR_CODE_OK = 0x20,
    ERR_BOOT_MISSING = 0x21,
    ERR_CODE_SIZE_SIG = 0x22,
    ERR_CODE_ADDR_SIZE = 0x23,
    ERR_CODE_INFO_MISSING = 0x24,
    ERR_CODE_CRC = 0x25,
    ERR_CODE_SIG = 0x26,

    ERR_CODE_MISSING = 0x31,
    ERR_VERSION = 0x32,

    ERR_BURN_OK = 0x60,
    ERR_SECTOR_SIZE = 0x61,
    ERR_SECTOR_SEQ_OVERFLOW = 0x62,
    ERR_BURN_INFO_MISSING = 0x63,
    ERR_SECTOR_DATA_LEN = 0x64,
    ERR_SECTOR_DATA_CRC = 0x65,
    ERR_SECTOR_SEQ = 0x66,
    ERR_ERASE_FLSH = 0x67,
    ERR_BURN_FLSH = 0x68,
    ERR_VERIFY_FLSH = 0x69,
    ERR_FLASH_CMD = 0x6A,

    ERR_TYPE_MISMATCHED = 0xE1,
    ERR_SEQ_MISMATCHED = 0xE2,
    ERR_BUF_TOO_SMALL = 0xE3,

    ERR_INTERNAL = 0xFF,
};


//#define PROGRAMMER_ANC_DEBUG
enum ANC_CMD_T {
    ANC_CMD_CLOSE = 0,
    ANC_CMD_OPEN = 1,
    ANC_CMD_GET_CFG = 2,
    ANC_CMD_APPLY_CFG = 3,
    ANC_CMD_CFG_SETUP = 4,
    ANC_CMD_CHANNEL_SETUP = 5,
    ANC_CMD_SET_SAMP_RATE = 6,
    ANC_CMD_PSAP_GET_CFG = 7,
    ANC_CMD_PSAP_APPLY_CFG = 8,
    ANC_CMD_PSAP_APPLY_GAIN = 9,
    ANC_CMD_SPEAKER_CALIBRATION_GET_CFG = 10,
    ANC_CMD_SPEAKER_CALIBRATION_CFG = 11,
    ANC_CMD_SET_VMIC_VOLT = 12,
};

enum FLASH_CMD_TYPE {
    FLASH_CMD_ERASE_SECTOR      = 0x21,
    FLASH_CMD_BURN_DATA         = 0x22,
    FLASH_CMD_ERASE_CHIP        = 0x31,
    FLASH_CMD_SEC_REG_ERASE     = 0x41,
    FLASH_CMD_SEC_REG_BURN      = 0x42,
    FLASH_CMD_SEC_REG_LOCK      = 0x43,
    FLASH_CMD_SEC_REG_READ      = 0x44,
    FLASH_CMD_ENABLE_REMAP      = 0x51,
    FLASH_CMD_DISABLE_REMAP     = 0x52,
};

enum PROD_TEST_CMD_T {
    PROD_TEST_CMD_ANC = 0x00000001,
};

static struct_anc_cfg g_anc_config;

#define MAX_READ_DATA_LEN 255
#define MAX_WRITE_DATA_LEN 255

static struct_psap_cfg g_psap_config;

static struct_spkcalib_cfg g_spkcalib_config;

static struct message_t recv_msg;
static struct message_t send_msg = { { PREFIX_CHAR, }, };


#define MAX_SNED_MSG_QUEUE  (10)
static unsigned int send_msg_push_seq = 0;
static unsigned int send_msg_pop_seq = 0;
static struct message_t send_msg_queue[MAX_SNED_MSG_QUEUE];


static uint8_t pcsuppt_anc_type = ANC_NOTYPE;
static enum AUD_SAMPRATE_T anc_sample_rate;

static unsigned int burn_addr;
static unsigned int burn_total_len;
static unsigned int sector_size;
static unsigned int sector_cnt;
static unsigned int last_sector_len;
static unsigned int cur_sector_seq;
static unsigned int burn_len;

uint16_t vmic1_volt = 2200;
uint16_t vmic2_volt = 2200;
uint16_t vmic3_volt = 2200;
uint16_t vmic4_volt = 2200;
uint16_t vmic5_volt = 2200;


static struct_psap_cfg *get_psap_config(void)
{
    return &g_psap_config;
}

static struct_spkcalib_cfg *get_spkcalib_config(void)
{
    return &g_spkcalib_config;
}


void anc_set_gpio(enum HAL_GPIO_PIN_T io_pin, bool set_flag)
{
// #define ANC_SET_GPIO_PIN   HAL_IOMUX_PIN_P2_4
    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_anc_set_gpio[1] = {
        {io_pin, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };

    hal_iomux_init(pinmux_anc_set_gpio, ARRAY_SIZE(pinmux_anc_set_gpio));
    hal_gpio_pin_set_dir(io_pin, HAL_GPIO_DIR_OUT, 0);
    if(set_flag)
        hal_gpio_pin_set(io_pin);
    else
        hal_gpio_pin_clr(io_pin);
}

#define MSG_TOTAL_LEN(msg)          (sizeof((msg)->hdr) + (msg)->hdr.len + 1)

#define TRACE_TIME(num,str, ...)            TRACE(1+num,"[%05u] " str, TICKS_TO_MS(hal_sys_timer_get()), ##__VA_ARGS__)


static unsigned char check_sum(const unsigned char *buf, unsigned char len)
{
    int i;
    unsigned char sum = 0;

    for (i = 0; i < len; i++) {
        sum += buf[i];
    }

    return sum;
}
static void trace_stage_info(const char *name)
{
    TRACE_TIME(1,"------ %s ------", name);
}
static void trace_rw_len_err(const char *name, unsigned int len)
{
    TRACE(2,"[%s] Length error: %u", name, len);
}
static void trace_rw_info(const char *name, unsigned int addr, unsigned int len)
{
   // TRACE(3,"[%s] addr=0x%08X len=%u", name, addr, len);
}

static void trace_flash_cmd_info(const char *name)
{
    TRACE_TIME(1,"- %s -", name);
}

static void trace_flash_cmd_len_err(const char *name, unsigned int len)
{
    TRACE(2,"Invalid %s cmd param len: %u", name, len);
}

static void trace_flash_cmd_err(const char *name)
{
    TRACE_TIME(1,"%s failed", name);
}

static void trace_flash_cmd_done(const char *name)
{
    TRACE_TIME(1,"%s done", name);
}

int send_reply(const unsigned char *payload, unsigned int len)
{
    int ret = 0;

    if (len + 1 > sizeof(send_msg.data)) {
        TRACE(1,"Packet length too long: %u", len);
        return -1;
    }

    send_msg.hdr.type = recv_msg.hdr.type;
    send_msg.hdr.seq = recv_msg.hdr.seq;
    send_msg.hdr.len = len;
    memcpy(&send_msg.data[0], payload, len);
    send_msg.data[len] = ~check_sum((unsigned char *)&send_msg, MSG_TOTAL_LEN(&send_msg) - 1);

    send_msg_push_seq=send_msg_push_seq%MAX_SNED_MSG_QUEUE;
    send_msg_queue[send_msg_push_seq]=send_msg;
    send_msg_push_seq++;

    return ret;
}
static enum ERR_CODE handle_read_cmd(unsigned int type, unsigned int addr, unsigned int len)
{
    union {
        unsigned int data[1 + (MAX_READ_DATA_LEN + 3) / 4];
        unsigned char buf[(1 + (MAX_READ_DATA_LEN + 3) / 4) * 4];
    } d;
    int i;

    int cnt;
    unsigned int *p32;
    unsigned short *p16;

    const char *name = NULL;

    if (type == TYPE_READ) {
        name = "READ";
    } else {
        return ERR_INTERNAL;
    }


    if (len > MAX_READ_DATA_LEN) {
        trace_rw_len_err(name, len);
        return ERR_DATA_LEN;
    }


    if (type == TYPE_READ) {
        // Handle half-word and word register reading
        if ((len & 0x03) == 0 && (addr & 0x03) == 0) {
            cnt = len / 4;
            p32 = (unsigned int *)&d.data[1];
            for (i = 0; i < cnt; i++) {
                p32[i] = *((unsigned int *)addr + i);
            }
        } else if ((len & 0x01) == 0 && (addr & 0x01) == 0) {
            cnt = len / 2;
            p16 = (unsigned short *)&d.data[1];
            for (i = 0; i < cnt; i++) {
                p16[i] = *((unsigned short *)addr + i);
            }
        } else {
            memcpy(&d.data[1], (unsigned char *)addr, len);
        }
    }

    d.buf[3] = ERR_NONE;
    send_reply((unsigned char *)&d.buf[3], 1 + len);

    return ERR_NONE;
}
static enum ERR_CODE handle_write_cmd(unsigned int addr, unsigned int len, unsigned char *wdata)
{
    unsigned int data;
    int i;
    int cnt;
    const char *name = "WRITE";

    trace_rw_info(name, addr, len);

    if (len > MAX_WRITE_DATA_LEN) {
        trace_rw_len_err(name, len);
        return ERR_DATA_LEN;
    }
    // Handle half-word and word register writing
    if ((len & 0x03) == 0 && (addr & 0x03) == 0) {
        cnt = len / 4;
        for (i = 0; i < cnt; i++) {
            data = wdata[4 * i] | (wdata[4 * i + 1] << 8) | (wdata[4 * i + 2] << 16) | (wdata[4 * i + 3] << 24);
            *((unsigned int *)addr + i) = data;
        }
    } else if ((len & 0x01) == 0 && (addr & 0x01) == 0) {
        cnt = len / 2;
        for (i = 0; i < cnt; i++) {
            data = wdata[2 * i] | (wdata[2 * i + 1] << 8);
            *((unsigned short *)addr + i) = (unsigned short)data;
        }
    } else {
        memcpy((unsigned char *)addr, wdata, len);
    }

    data = ERR_NONE;
    send_reply((unsigned char *)&data, 1);

    return ERR_NONE;
}
static struct_anc_cfg *get_anc_config(void)
{
    return &g_anc_config;
}


static void tool_anc_close(void)
{
    if(pcsuppt_anc_type & ANC_FEEDFORWARD)
    {
        anc_close(ANC_FEEDFORWARD);
        af_anc_close(ANC_FEEDFORWARD);
#ifdef  AUDIO_ANC_TT_HW
        anc_close(ANC_TALKTHRU);
        af_anc_close(ANC_TALKTHRU);
#endif
    }

    if(pcsuppt_anc_type & ANC_FEEDBACK)
    {
#ifdef  AUDIO_ANC_FB_MC_HW
        anc_close(ANC_MUSICCANCLE);
#endif

        anc_close(ANC_FEEDBACK);
        af_anc_close(ANC_FEEDBACK);

    }
#if defined(PSAP_APP)
    psap_disable(CODEC_OUTPUT_DEV);
    psap_close();
    af_anc_close(ANC_TALKTHRU);
#endif

#if defined(AUDIO_ANC_SPKCALIB_HW)
        anc_close(ANC_SPKCALIB);
#endif

}
static void init_flash(void);
const static enum HAL_FLASH_ID_T boot_flash_ctrl_id = HAL_FLASH_ID_0;
#ifndef FLASH1_CTRL_BASE
const
#endif
static enum HAL_FLASH_ID_T cur_flash_ctrl_id = HAL_FLASH_ID_0;
static bool flash_valid[HAL_FLASH_ID_NUM];

static void tool_anc_open(void)
{
    TRACE(0,"[%s]:AUD_SECTION_STRUCT_VERSION=%d!",__func__,AUD_SECTION_STRUCT_VERSION);

	init_flash();

#if defined(CHIP_BEST1501) || defined(CHIP_BEST1600) || defined(CHIP_BEST1501P) || defined(CHIP_BEST1502X)
    pmu_codec_mic_bias_set_volt(AUD_VMIC_MAP_VMIC1, vmic1_volt);
    pmu_codec_mic_bias_set_volt(AUD_VMIC_MAP_VMIC2, vmic2_volt);
    pmu_codec_mic_bias_set_volt(AUD_VMIC_MAP_VMIC3, vmic3_volt);
    pmu_codec_mic_bias_set_volt(AUD_VMIC_MAP_VMIC4, vmic4_volt);
    pmu_codec_mic_bias_set_volt(AUD_VMIC_MAP_VMIC5, vmic5_volt);
#endif

    if(pcsuppt_anc_type & ANC_FEEDFORWARD)
    {
        af_anc_open(ANC_FEEDFORWARD, anc_sample_rate, anc_sample_rate, NULL);
        anc_open(ANC_FEEDFORWARD);
        anc_set_gain(512,512,ANC_FEEDFORWARD);

#ifdef  AUDIO_ANC_TT_HW
        af_anc_open(ANC_TALKTHRU, anc_sample_rate, anc_sample_rate, NULL);
        anc_open(ANC_TALKTHRU);
        anc_set_gain(512,512,ANC_TALKTHRU);
#endif

    }

    if(pcsuppt_anc_type & ANC_FEEDBACK)
    {
        af_anc_open(ANC_FEEDBACK, anc_sample_rate, anc_sample_rate, NULL);
        anc_open(ANC_FEEDBACK);
        anc_set_gain(512,512,ANC_FEEDBACK);

#ifdef  AUDIO_ANC_FB_MC_HW
        anc_open(ANC_MUSICCANCLE);
        anc_set_gain(512,512,ANC_MUSICCANCLE);
#endif

    }

#if defined(PSAP_APP)
    af_anc_open(ANC_TALKTHRU, anc_sample_rate, anc_sample_rate, NULL);
    psap_open(CODEC_OUTPUT_DEV);
    psap_enable(CODEC_OUTPUT_DEV);
#endif

#if defined(AUDIO_ANC_SPKCALIB_HW)
        anc_open(ANC_SPKCALIB);
#endif
}
#if !defined( AUDIO_ANC_TT_HW)
#if defined(__HW_IIR_EQ_PROCESS__)
static HW_IIR_CFG_T hw_iir_cfg;
#endif

#if defined(__HW_DAC_IIR_EQ_PROCESS__)
static HW_CODEC_IIR_CFG_T hw_codec_iir_cfg;
#endif
#endif
//struct_anc_fir_cfg fir_cfg;
//int preview_flag=0;
static int handle_anc_cmd(enum ANC_CMD_T cmd, const uint8_t *data, uint32_t len)
{

    unsigned char cret = ERR_NONE;

    switch (cmd) {
        case ANC_CMD_CLOSE:
        {
            TRACE(0,"ANC_CMD_CLOSE ------");
            if (len != 0)
            {
                return ERR_LEN;
            }
            anc_disable();
            tool_anc_close();
            send_reply(&cret, 1);
            break;
        }
        case ANC_CMD_OPEN:
        {
            TRACE(0,"ANC_CMD_OPEN ------");
            if (len != 0) {
                return ERR_LEN;
            }
            hal_sysfreq_req(HAL_SYSFREQ_USER_ANC, HAL_CMU_FREQ_52M);
            analog_debug_config_anc_calib_mode(true);
            tool_anc_open();
            anc_enable();
            send_reply(&cret, 1);
            break;
        }
        case ANC_CMD_GET_CFG:
        {
               TRACE(0,"ANC_CMD_GET_CFG ------");

                struct_anc_cfg *anccfg_addr = get_anc_config();
                uint32_t addr = (uint32_t)anccfg_addr;

                if (len != 0) {
                    return ERR_LEN;
                }

                TRACE(1,"send anccfg address 0x%x ------",addr);
                send_reply((unsigned char *)&addr, sizeof(addr));
                break;
        }
        case ANC_CMD_APPLY_CFG:
        {
                TRACE(0,"ANC_CMD_APPLY_CFG ------");

                if (len != 0) {
                    return ERR_LEN;
                }
                TRACE(0,"apply anccfg ------");
               // best2000_prod_test_anccfg_apply();
                struct_anc_cfg *anccfg = get_anc_config();
/*
        fir_cfg.anc_fir_cfg_ff_l.fir_len=384;
        fir_cfg.anc_fir_cfg_ff_l.fir_bypass_flag=0;
         for(int i=0;i<fir_cfg.anc_fir_cfg_ff_l.fir_len;i++)
        {
            fir_cfg.anc_fir_cfg_ff_l.fir_coef[i]=0;
        }

         if(preview_flag==0)
         {
             fir_cfg.anc_fir_cfg_ff_l.fir_coef[0]=0x7ff;
         }
         else
         {
            fir_cfg.anc_fir_cfg_ff_l.fir_coef[0]=0x7fffff;
         }

         preview_flag=1-preview_flag;

         anc_set_fir_cfg(&fir_cfg,ANC_FEEDFORWARD);
          */
                 //process ANC
                if(pcsuppt_anc_type & ANC_FEEDFORWARD)
                {

#ifdef  AUDIO_ANC_TT_HW
                    anc_set_cfg(anccfg,ANC_FEEDFORWARD|ANC_TALKTHRU,ANC_GAIN_NO_DELAY);
#else
                    anc_set_cfg(anccfg,ANC_FEEDFORWARD,ANC_GAIN_NO_DELAY);
#endif
                }

                if(pcsuppt_anc_type & ANC_FEEDBACK)
                {
#ifdef  AUDIO_ANC_FB_MC_HW
                    anc_set_cfg(anccfg,ANC_FEEDBACK|ANC_MUSICCANCLE,ANC_GAIN_NO_DELAY);
#else
                    anc_set_cfg(anccfg,ANC_FEEDBACK,ANC_GAIN_NO_DELAY);
#endif
                }
#ifdef AUDIO_ANC_FB_MC
                //process MC
                mc_iir_cfg.anc_cfg_mc_l=anccfg->anc_cfg_mc_l;
#if ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)
                mc_iir_cfg.anc_cfg_mc_r=anccfg->anc_cfg_mc_r;
#endif
#endif

                if(anccfg->anc_cfg_mc_l.adc_gain_offset==0)
                {
                    anc_adc_data_select(ANC_ADC_ADC_ADD_MC);
                    TRACE(0,"ADC ADD MC........");
                }
                else
                {
                    anc_adc_data_select(ANC_ADC_ONLY_MC);
                    TRACE(0,"ONLY MC........");
                }

                if(anccfg->anc_cfg_mc_l.dac_gain_offset==0)
                {
                    analog_aud_codec_nomute();
                    TRACE(0,"DAC UNMUTE........");
                }
                else
                {
                    analog_aud_codec_mute();
                    TRACE(0,"DAC MUTE........");
                }

#if !defined( AUDIO_ANC_TT_HW)

#if defined(__HW_IIR_EQ_PROCESS__)
                //process EQ
                if(anccfg->anc_cfg_tt_l.total_gain ==0)
                {
                    hw_iir_cfg.iir_filtes_l.iir_counter=0;
                }
                else
                {
                    hw_iir_cfg.iir_filtes_l.iir_counter=anccfg->anc_cfg_tt_l.iir_counter;
                }

                if(anccfg->anc_cfg_tt_r.total_gain ==0)
                {
                    hw_iir_cfg.iir_filtes_r.iir_counter=0;
                }
                else
                {
                    hw_iir_cfg.iir_filtes_r.iir_counter=anccfg->anc_cfg_tt_r.iir_counter;
                }

                for(int i=0;i<AUD_IIR_NUM_EQ;i++)
                {
                    hw_iir_cfg.iir_filtes_l.iir_coef[i].coef_b[0]=(anccfg->anc_cfg_tt_l.iir_coef[i].coef_b[0]);
                    hw_iir_cfg.iir_filtes_l.iir_coef[i].coef_b[1]=(anccfg->anc_cfg_tt_l.iir_coef[i].coef_b[1]);
                    hw_iir_cfg.iir_filtes_l.iir_coef[i].coef_b[2]=(anccfg->anc_cfg_tt_l.iir_coef[i].coef_b[2]);
                    hw_iir_cfg.iir_filtes_l.iir_coef[i].coef_a[0]=(anccfg->anc_cfg_tt_l.iir_coef[i].coef_a[0]);
                    hw_iir_cfg.iir_filtes_l.iir_coef[i].coef_a[1]=(anccfg->anc_cfg_tt_l.iir_coef[i].coef_a[1]);
                    hw_iir_cfg.iir_filtes_l.iir_coef[i].coef_a[2]=(anccfg->anc_cfg_tt_l.iir_coef[i].coef_a[2]);

                    hw_iir_cfg.iir_filtes_r.iir_coef[i].coef_b[0]=(anccfg->anc_cfg_tt_r.iir_coef[i].coef_b[0]);
                    hw_iir_cfg.iir_filtes_r.iir_coef[i].coef_b[1]=(anccfg->anc_cfg_tt_r.iir_coef[i].coef_b[1]);
                    hw_iir_cfg.iir_filtes_r.iir_coef[i].coef_b[2]=(anccfg->anc_cfg_tt_r.iir_coef[i].coef_b[2]);
                    hw_iir_cfg.iir_filtes_r.iir_coef[i].coef_a[0]=(anccfg->anc_cfg_tt_r.iir_coef[i].coef_a[0]);
                    hw_iir_cfg.iir_filtes_r.iir_coef[i].coef_a[1]=(anccfg->anc_cfg_tt_r.iir_coef[i].coef_a[1]);
                    hw_iir_cfg.iir_filtes_r.iir_coef[i].coef_a[2]=(anccfg->anc_cfg_tt_r.iir_coef[i].coef_a[2]);
                 }

                hw_iir_set_cfg(&hw_iir_cfg);
#endif

#if defined(__HW_DAC_IIR_EQ_PROCESS__)

#if 0
                 TRACE(1,"__HW_DAC_IIR_EQ_PROCESS__ ........");

                TRACE(1,"eq gain: %d, counter: %d iir_bypass_flag: %d ",anccfg->anc_cfg_tt_l.total_gain, anccfg->anc_cfg_tt_l.iir_counter,anccfg->anc_cfg_tt_l.iir_bypass_flag);
                TRACE(1,"eq dac_gain_offset %d, adc_gain_offset %d",anccfg->anc_cfg_tt_l.dac_gain_offset, anccfg->anc_cfg_tt_l.adc_gain_offset);

                for(int j = 0; j <AUD_IIR_NUM_EQ; j++)
                {
                    //TRACE(1,"iir coef ff l %10d, %10d, %10d, %10d, %10d, %10d",
                    TRACE(1,"iir coef eq l 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x",\
                            anccfg->anc_cfg_tt_l.iir_coef[j].coef_b[0], \
                            anccfg->anc_cfg_tt_l.iir_coef[j].coef_b[1], \
                            anccfg->anc_cfg_tt_l.iir_coef[j].coef_b[2], \
                            anccfg->anc_cfg_tt_l.iir_coef[j].coef_a[0], \
                            anccfg->anc_cfg_tt_l.iir_coef[j].coef_a[1], \
                            anccfg->anc_cfg_tt_l.iir_coef[j].coef_a[2]);
                }
#endif

                //process EQ
                if(anccfg->anc_cfg_tt_l.total_gain ==0)
                {
                    hw_codec_iir_cfg.iir_filtes_l.iir_counter=0;
                }
                else
                {
                    hw_codec_iir_cfg.iir_filtes_l.iir_counter=anccfg->anc_cfg_tt_l.iir_counter;
                }

                if(anccfg->anc_cfg_tt_r.total_gain ==0)
                {
                    hw_codec_iir_cfg.iir_filtes_r.iir_counter=0;
                }
                else
                {
                    hw_codec_iir_cfg.iir_filtes_r.iir_counter=anccfg->anc_cfg_tt_r.iir_counter;
                }

                for(int i=0;i<AUD_IIR_NUM_EQ;i++)
                {
                    hw_codec_iir_cfg.iir_filtes_l.iir_coef[i].coef_b[0]=(anccfg->anc_cfg_tt_l.iir_coef[i].coef_b[0]);
                    hw_codec_iir_cfg.iir_filtes_l.iir_coef[i].coef_b[1]=(anccfg->anc_cfg_tt_l.iir_coef[i].coef_b[1]);
                    hw_codec_iir_cfg.iir_filtes_l.iir_coef[i].coef_b[2]=(anccfg->anc_cfg_tt_l.iir_coef[i].coef_b[2]);
                    hw_codec_iir_cfg.iir_filtes_l.iir_coef[i].coef_a[0]=(anccfg->anc_cfg_tt_l.iir_coef[i].coef_a[0]);
                    hw_codec_iir_cfg.iir_filtes_l.iir_coef[i].coef_a[1]=(anccfg->anc_cfg_tt_l.iir_coef[i].coef_a[1]);
                    hw_codec_iir_cfg.iir_filtes_l.iir_coef[i].coef_a[2]=(anccfg->anc_cfg_tt_l.iir_coef[i].coef_a[2]);

                    hw_codec_iir_cfg.iir_filtes_r.iir_coef[i].coef_b[0]=(anccfg->anc_cfg_tt_r.iir_coef[i].coef_b[0]);
                    hw_codec_iir_cfg.iir_filtes_r.iir_coef[i].coef_b[1]=(anccfg->anc_cfg_tt_r.iir_coef[i].coef_b[1]);
                    hw_codec_iir_cfg.iir_filtes_r.iir_coef[i].coef_b[2]=(anccfg->anc_cfg_tt_r.iir_coef[i].coef_b[2]);
                    hw_codec_iir_cfg.iir_filtes_r.iir_coef[i].coef_a[0]=(anccfg->anc_cfg_tt_r.iir_coef[i].coef_a[0]);
                    hw_codec_iir_cfg.iir_filtes_r.iir_coef[i].coef_a[1]=(anccfg->anc_cfg_tt_r.iir_coef[i].coef_a[1]);
                    hw_codec_iir_cfg.iir_filtes_r.iir_coef[i].coef_a[2]=(anccfg->anc_cfg_tt_r.iir_coef[i].coef_a[2]);
                 }

                if (anc_sample_rate == AUD_SAMPRATE_50781)
                {
                    hw_codec_iir_set_cfg(&hw_codec_iir_cfg, AUD_SAMPRATE_50781, HW_CODEC_IIR_DAC);
                }
                else
                {
                    hw_codec_iir_set_cfg(&hw_codec_iir_cfg, AUD_SAMPRATE_48000, HW_CODEC_IIR_DAC);
                }
#endif
#endif
                send_reply(&cret, 1);
                break;
        }
        case ANC_CMD_CFG_SETUP:
        {
            TRACE(0,"ANC_CMD_CFG_SETUP ------");

            int ret = 0;
            bool diff;
            bool high_performance_adc;
            bool vcrystal_on;
            uint16_t vcodec;
            pctool_iocfg *iocfg1, *iocfg2;

            if(data[0]==1)
            {
                pcsuppt_anc_type = ANC_FEEDFORWARD;
            }
            else if(data[0]==2)
            {
                pcsuppt_anc_type = ANC_FEEDBACK;
            }
            else if(data[0]==3)
            {
                pcsuppt_anc_type = ANC_FEEDFORWARD|ANC_FEEDBACK;
            }

            vcodec = (data[1] | (data[2] << 8));
            diff = (bool)data[3];
            high_performance_adc = false; //(bool)data[4]; // default 0
            anc_sample_rate = (data[5] | (data[6] << 8) | (data[7] << 16) | (data[8] << 24));
            vcrystal_on = data[9]; //(bool)data[?]; // default 0
            TRACE(4,"vcodec:%d,diff:%d,anc_sample_rate:%d,vcrystal_on:%d.", vcodec,diff,anc_sample_rate,vcrystal_on);
            ret |= pmu_debug_config_vcrystal(vcrystal_on);
            ret |= analog_debug_config_audio_output(diff);
            ret |= analog_debug_config_codec(vcodec);
            ret |= analog_debug_config_low_power_adc(!high_performance_adc);
            if (anc_sample_rate == AUD_SAMPRATE_50781) {
                hal_cmu_audio_resample_enable();
            } else {
                hal_cmu_audio_resample_disable();
            }
            iocfg1 = (pctool_iocfg *)(&data[10]);
            iocfg2 = (pctool_iocfg *)(&data[12]);
            TRACE(4,"io cfg:%d    %d    %d    %d",iocfg1->io_pin,iocfg1->set_flag,iocfg2->io_pin,iocfg2->set_flag);
            if(iocfg1->io_pin>=0)
                anc_set_gpio(iocfg1->io_pin,iocfg1->set_flag);
            if(iocfg2->io_pin>=0)
                anc_set_gpio(iocfg2->io_pin,iocfg2->set_flag);
             cret = ret ? ERR_INTERNAL : ERR_NONE;

            send_reply((unsigned char *)&cret, 1);
            break;
        }
        case ANC_CMD_CHANNEL_SETUP:
        {

            const enum AUD_CHANNEL_MAP_T channel_map_arr[16] = {
                                                    AUD_CHANNEL_MAP_CH0,
                                                    AUD_CHANNEL_MAP_CH1,
                                                    AUD_CHANNEL_MAP_CH2,
                                                    AUD_CHANNEL_MAP_CH3,
                                                    AUD_CHANNEL_MAP_CH4,
                                                    AUD_CHANNEL_MAP_CH5,
                                                    AUD_CHANNEL_MAP_CH6,
                                                    AUD_CHANNEL_MAP_CH7,
                                                    AUD_CHANNEL_MAP_DIGMIC_CH0,
                                                    AUD_CHANNEL_MAP_DIGMIC_CH1,
                                                    AUD_CHANNEL_MAP_DIGMIC_CH2,
                                                    AUD_CHANNEL_MAP_DIGMIC_CH3,
                                                    AUD_CHANNEL_MAP_DIGMIC_CH4,
                                                    AUD_CHANNEL_MAP_DIGMIC_CH5,
                                                    AUD_CHANNEL_MAP_DIGMIC_CH6,
                                                    AUD_CHANNEL_MAP_DIGMIC_CH7,
                                                };
            anc_ff_mic_ch_l = channel_map_arr[data[0]];
            anc_ff_mic_ch_r = channel_map_arr[data[1]];
            anc_fb_mic_ch_l = channel_map_arr[data[2]];
            anc_fb_mic_ch_r = channel_map_arr[data[3]];
            TRACE(4,"anc_ff_mic_ch_l 0x%x,anc_ff_mic_ch_r 0x%x,anc_fb_mic_ch_l 0x%x,anc_fb_mic_ch_r 0x%x",
                anc_ff_mic_ch_l, anc_ff_mic_ch_r, anc_fb_mic_ch_l, anc_fb_mic_ch_r);

            hal_iomux_set_dig_mic_clock_pin(data[4]);
            hal_iomux_set_dig_mic_data0_pin(data[5]);
            hal_iomux_set_dig_mic_data1_pin(data[6]);
            hal_iomux_set_dig_mic_data2_pin(data[7]);
            TRACE(4,"dig_mic_clock 0x%x,dig_mic_data0 0x%x,dig_mic_data1 0x%x,dig_mic_data2 0x%x",
                data[4], data[5], data[6], data[7]);

            uint8_t phase = data[8];
#if defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A)
            analog_debug_config_vad_mic(!!(phase & (1 << 7)));
            phase &= ~(1 << 7);
#endif
            hal_codec_config_digmic_phase(phase);

            if(data[9]>=0&&data[9]<16)
            {
                anc_tt_mic_ch_l = channel_map_arr[data[9]];
            }
            if(data[10]>=0&&data[10]<16)
            {
                anc_tt_mic_ch_r = channel_map_arr[data[10]];
            }

            TRACE(4,"anc_tt_mic_ch_l 0x%x,anc_tt_mic_ch_r 0x%x",anc_tt_mic_ch_l, anc_tt_mic_ch_r);

            send_reply((unsigned char *)&cret, 1);
            break;
        }
        case ANC_CMD_SET_SAMP_RATE:
        {
            bool opened;

            opened = (anc_opened(ANC_FEEDFORWARD) || anc_opened(ANC_FEEDBACK));
            if (opened)
            {
                anc_disable();
                tool_anc_close();
            }
            anc_sample_rate = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
            if (anc_sample_rate == AUD_SAMPRATE_50781) {
                hal_cmu_audio_resample_enable();
            } else {
                hal_cmu_audio_resample_disable();
            }
            if (opened)
            {
               tool_anc_open();
               anc_enable();
            }
            send_reply((unsigned char *)&cret, 1);
            break;
        }
        case ANC_CMD_PSAP_GET_CFG:
        {
            struct_psap_cfg *psapcfg_addr = get_psap_config();
            uint32_t addr = (uint32_t)psapcfg_addr;

            if (len != 0) {
                return ERR_LEN;
            }

            TRACE(1,"send psapcfg address 0x%x ------",addr);
            send_reply((unsigned char *)&addr, sizeof(addr));
            break;
        }
        case ANC_CMD_PSAP_APPLY_CFG:
        {
            if (len != 0) {
                return ERR_LEN;
            }
            TRACE(0,"apply psapcfg ------");
#if defined(PSAP_APP)
            psap_set_cfg_coef(get_psap_config());
#else
            TRACE(0,"PSAP_APP need to be opened!");
#endif

            cret = ERR_NONE;
            send_reply((unsigned char *)&cret, 1);
            break;
        }
        case ANC_CMD_PSAP_APPLY_GAIN:
        {

            TRACE(0,"apply left psap gain ------");
#if defined(PSAP_APP)
            int32_t gain_ch_l=data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
            int32_t gain_ch_r=data[4] | (data[5] << 8) | (data[6] << 16) | (data[7] << 24);
            uint32_t band_index=data[8];
            psap_set_gain(gain_ch_l,gain_ch_r, band_index);
#else
            TRACE(0,"PSAP_APP need to be opened!");
#endif
            cret = ERR_NONE;
            send_reply((unsigned char *)&cret, 1);
            break;
        }
        case ANC_CMD_SPEAKER_CALIBRATION_GET_CFG:
        {
            struct_spkcalib_cfg *spkcalibcfg_addr = get_spkcalib_config();
            uint32_t addr = (uint32_t)spkcalibcfg_addr;

            if (len != 0) {
                return ERR_LEN;
            }

            TRACE(1,"send spkcalibcfg_addr address 0x%x ------",addr);
            send_reply((unsigned char *)&addr, sizeof(addr));
            break;
        }
        case ANC_CMD_SPEAKER_CALIBRATION_CFG:
        {
            if (len != 0) {
                return ERR_LEN;
            }
            TRACE(0,"apply spkcalib cfg ------");
#if defined(AUDIO_ANC_SPKCALIB_HW)
            anc_set_spkcalib_cfg(get_spkcalib_config());
#else
            TRACE(0,"AUDIO_ANC_SPKCALIB_HW need to be opened!");
#endif

            cret = ERR_NONE;
            send_reply((unsigned char *)&cret, 1);
            break;
        }
        case ANC_CMD_SET_VMIC_VOLT:
        {
            TRACE(0, "apply VMIC VOLT cfg ------len :%d",len);
            vmic1_volt = data[0] | (data[1] << 8);
            vmic2_volt = data[2] | (data[3] << 8);
            vmic3_volt = data[4] | (data[5] << 8);
            vmic4_volt = data[6] | (data[7] << 8);
            vmic5_volt = data[8] | (data[9] << 8);

            TRACE(0, "apply VMIC1 VOLT cfg :%d mv", vmic1_volt);
            TRACE(0, "apply VMIC2 VOLT cfg :%d mv", vmic2_volt);
            TRACE(0, "apply VMIC3 VOLT cfg :%d mv", vmic3_volt);
            TRACE(0, "apply VMIC4 VOLT cfg :%d mv", vmic4_volt);
            TRACE(0, "apply VMIC5 VOLT cfg :%d mv", vmic5_volt);

#if defined(CHIP_BEST1501) || defined(CHIP_BEST1600) || defined(CHIP_BEST1501P) || defined(CHIP_BEST1502X)

#else
            TRACE(0,"can't support to set vmic!");
#endif
            cret = ERR_NONE;
            send_reply((unsigned char *)&cret, 1);
            break;
        }
    }

    return ERR_NONE;
 }


#ifdef FLASH1_CTRL_BASE
static int set_flash_ctrl_id(unsigned int addr)
{
#define ADDR_IN_RANGE(base, addr)           ((base) <= (addr) && (addr) <= ((base) | HAL_NORFLASH_ADDR_MASK))

    if (ADDR_IN_RANGE(FLASH1_BASE, addr) || ADDR_IN_RANGE(FLASH1_NC_BASE, addr) ||
            ADDR_IN_RANGE(FLASH1X_BASE, addr) || ADDR_IN_RANGE(FLASH1X_NC_BASE, addr)) {
        cur_flash_ctrl_id = HAL_FLASH_ID_1;
    } else {
        cur_flash_ctrl_id = HAL_FLASH_ID_0;
    }
    if (!flash_valid[cur_flash_ctrl_id]) {
        return 1;
    }
    return 0;
}
#endif


static void init_flash(void)
{
    int ret;
    int boot_flash_ret;
    uint8_t flash_id[HAL_NORFLASH_DEVICE_ID_LEN];
    uint8_t event[HAL_NORFLASH_DEVICE_ID_LEN + 3];
    const struct HAL_NORFLASH_CONFIG_T *cfg;

    cfg = hal_norflash_get_init_config();
    ret = hal_norflash_open(boot_flash_ctrl_id, cfg);

#ifdef FLASH_DUAL_CHIP
    if (FLASH_IS_DUAL_CHIP(boot_flash_ctrl_id)) {
        uint8_t flash_id1[HAL_NORFLASH_DEVICE_ID_LEN];
        hal_norflash_dual_chip_get_id(boot_flash_ctrl_id, flash_id, flash_id1, ARRAY_SIZE(flash_id));
        TRACE(0, "FLASH_ID:%u: %02X-%02X-%02X / %02X-%02X-%02X", boot_flash_ctrl_id,
            flash_id[0], flash_id[1], flash_id[2],
            flash_id1[0], flash_id1[1], flash_id1[2]);
    } else
#endif
    {
        hal_norflash_get_id(boot_flash_ctrl_id, flash_id, ARRAY_SIZE(flash_id));
        TRACE(0, "FLASH_ID:%u: %02X-%02X-%02X", boot_flash_ctrl_id, flash_id[0], flash_id[1], flash_id[2]);
    }

    if (ret) {
        // <bootId> <curId> <openStatus> <flashId 0> <flashId 1> <flashId 2>
        event[0] = boot_flash_ctrl_id;
        event[1] = boot_flash_ctrl_id;
        event[2] = ret;
        memcpy(&event[3], &flash_id[0], sizeof(flash_id));
        //send_debug_event(&event[0], sizeof(event));
    } else {
#ifdef FLASH_UNIQUE_ID
        uint8_t unique_id[HAL_NORFLASH_UNIQUE_ID_LEN + HAL_NORFLASH_CP_ID_LEN];
#ifdef FLASH_DUAL_CHIP
        if (FLASH_IS_DUAL_CHIP(boot_flash_ctrl_id)) {
            uint8_t unique_id1[HAL_NORFLASH_UNIQUE_ID_LEN + HAL_NORFLASH_CP_ID_LEN];
            hal_norflash_dual_chip_get_unique_id(boot_flash_ctrl_id, unique_id, unique_id1, ARRAY_SIZE(unique_id));
            TRACE(0, "UNIQUE_ID:%u:A:", boot_flash_ctrl_id);
            DUMP8("%02X ", unique_id, ARRAY_SIZE(unique_id));
            TRACE(0, "UNIQUE_ID:%u:B:", boot_flash_ctrl_id);
            DUMP8("%02X ", unique_id1, ARRAY_SIZE(unique_id));
        } else
#endif
        {
            hal_norflash_get_unique_id(boot_flash_ctrl_id, unique_id, ARRAY_SIZE(unique_id));
            TRACE(0, "UNIQUE_ID:%u:", boot_flash_ctrl_id);
            DUMP8("%02X ", unique_id, ARRAY_SIZE(unique_id));
        }
#endif
    }
    hal_norflash_show_calib_result(boot_flash_ctrl_id);

    boot_flash_ret = ret;

#ifdef FLASH1_CTRL_BASE
    hal_cmu_clock_enable(HAL_CMU_MOD_H_FLASH1);
    hal_cmu_clock_enable(HAL_CMU_MOD_O_FLASH1);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_FLASH1);
    hal_cmu_reset_clear(HAL_CMU_MOD_O_FLASH1);

    cur_flash_ctrl_id = HAL_FLASH_ID_1;

    ret = hal_norflash_open(cur_flash_ctrl_id, cfg);

#ifdef FLASH_DUAL_CHIP
    if (FLASH_IS_DUAL_CHIP(cur_flash_ctrl_id)) {
        uint8_t flash_id1[HAL_NORFLASH_DEVICE_ID_LEN];
        hal_norflash_dual_chip_get_id(cur_flash_ctrl_id, flash_id, flash_id1, ARRAY_SIZE(flash_id));
        TRACE(0, "FLASH_ID:%u: %02X-%02X-%02X / %02X-%02X-%02X", cur_flash_ctrl_id,
            flash_id[0], flash_id[1], flash_id[2],
            flash_id1[0], flash_id1[1], flash_id1[2]);
    } else
#endif
    {
        hal_norflash_get_id(cur_flash_ctrl_id, flash_id, ARRAY_SIZE(flash_id));
        TRACE(0, "FLASH_ID:%u: %02X-%02X-%02X", cur_flash_ctrl_id, flash_id[0], flash_id[1], flash_id[2]);
    }

    if (ret) {
        // <bootId> <curId> <openStatus> <flashId 0> <flashId 1> <flashId 2>
        event[0] = boot_flash_ctrl_id;
        event[1] = cur_flash_ctrl_id;
        event[2] = ret;
        memcpy(&event[3], &flash_id[0], sizeof(flash_id));
        //send_debug_event(&event[0], sizeof(event));
    } else {
#ifdef FLASH_UNIQUE_ID
        uint8_t unique_id[HAL_NORFLASH_UNIQUE_ID_LEN + HAL_NORFLASH_CP_ID_LEN];
#ifdef FLASH_DUAL_CHIP
        if (FLASH_IS_DUAL_CHIP(cur_flash_ctrl_id)) {
            uint8_t unique_id1[HAL_NORFLASH_UNIQUE_ID_LEN + HAL_NORFLASH_CP_ID_LEN];
            hal_norflash_dual_chip_get_unique_id(cur_flash_ctrl_id, unique_id, unique_id1, ARRAY_SIZE(unique_id));
            TRACE(0, "UNIQUE_ID:%u:A:", cur_flash_ctrl_id);
            DUMP8("%02X ", unique_id, ARRAY_SIZE(unique_id));
            TRACE(0, "UNIQUE_ID:%u:B:", cur_flash_ctrl_id);
            DUMP8("%02X ", unique_id1, ARRAY_SIZE(unique_id));
        } else
#endif
        {
            hal_norflash_get_unique_id(cur_flash_ctrl_id, unique_id, ARRAY_SIZE(unique_id));
            TRACE(0, "UNIQUE_ID:%u:", cur_flash_ctrl_id);
            DUMP8("%02X ", unique_id, ARRAY_SIZE(unique_id));
        }
#endif
    }
    hal_norflash_show_calib_result(cur_flash_ctrl_id);

    flash_valid[boot_flash_ctrl_id] = !boot_flash_ret;
    flash_valid[cur_flash_ctrl_id] = !ret;

    cur_flash_ctrl_id = boot_flash_ctrl_id;
#endif

    // Check boot flash status after opening all the flash
   // ASSERT(boot_flash_ret == 0, "Failed to open boot norflash: %d", boot_flash_ret);
}

static int get_sector_info(unsigned int addr, unsigned int *sector_addr, unsigned int *sector_len)
{
    int ret;
    uint32_t base;
    uint32_t size;

#ifdef FLASH1_CTRL_BASE
    ret = set_flash_ctrl_id(addr);
    if (ret) {
        return HAL_NORFLASH_NOT_OPENED;
    }
#endif

    if (sector_addr) {
        ret = hal_norflash_get_boundary(cur_flash_ctrl_id, addr, NULL, &base);
        if (ret) {
            return ret;
        }
        *sector_addr = base;
    }
    if (sector_len) {
        ret = hal_norflash_get_size(cur_flash_ctrl_id, NULL, NULL, &size, NULL);
        *sector_len = size;
    }

    return 0;
}

static int erase_sector(unsigned int sector_addr, unsigned int sector_len)
{
    int ret;

#ifdef FLASH1_CTRL_BASE
    ret = set_flash_ctrl_id(sector_addr);
    if (ret) {
        return HAL_NORFLASH_NOT_OPENED;
    }
#endif

    ret = hal_norflash_erase(cur_flash_ctrl_id, sector_addr, sector_len);
#ifdef FLASH_REMAP
    if (ret == 0 && burn_remap_both) {
        hal_norflash_disable_remap(cur_flash_ctrl_id, HAL_NORFLASH_REMAP_ID_0);

        ret = hal_norflash_erase(cur_flash_ctrl_id, sector_addr, sector_len);

        hal_norflash_enable_remap(cur_flash_ctrl_id, HAL_NORFLASH_REMAP_ID_0);
    }
#endif
    return ret;
}

static int erase_chip(void)
{
	hal_norflash_erase_chip(HAL_FLASH_ID_0);

#ifdef FLASH1_CTRL_BASE
	hal_norflash_erase_chip(HAL_FLASH_ID_1);
#endif

    return 0;
}

int burn_data(unsigned int addr, const unsigned char *data, unsigned int len)
{
    int ret;

#ifdef FLASH1_CTRL_BASE
    ret = set_flash_ctrl_id(addr);
    if (ret) {
        return HAL_NORFLASH_NOT_OPENED;
    }
#endif

    ret = hal_norflash_write(cur_flash_ctrl_id, addr, data, len);
#ifdef FLASH_REMAP
    if (ret == 0 && burn_remap_both) {
        hal_norflash_disable_remap(cur_flash_ctrl_id, HAL_NORFLASH_REMAP_ID_0);

        ret = hal_norflash_write(cur_flash_ctrl_id, addr, data, len);

        hal_norflash_enable_remap(cur_flash_ctrl_id, HAL_NORFLASH_REMAP_ID_0);
    }
#endif
    return ret;
}


static int verify_flash_data(unsigned int addr, const unsigned char *data, unsigned int len)
{
    const unsigned char *fdata;
    const unsigned char *mdata;
    int i;

    fdata = (unsigned char *)addr;
    mdata = data;
    for (i = 0; i < len; i++) {
        if (*fdata++ != *mdata++) {
            --fdata;
            --mdata;
            TRACE(4,"*** Verify flash data failed: 0x%02X @ %p != 0x%02X @ %p", *fdata, fdata, *mdata, mdata);
            return *fdata - *mdata;
        }
    }
    return 0;
}
static enum ERR_CODE handle_sector_info_cmd(unsigned int addr)
{
    unsigned int sector_addr;
    unsigned int sector_len;
    unsigned char buf[9];
    int ret;

    ret = get_sector_info(addr, &sector_addr, &sector_len);
    if (ret) {
        return ERR_DATA_ADDR;
    }

    TRACE(3,"addr=0x%08X sector_addr=0x%08X sector_len=%u", addr, sector_addr, sector_len);

    buf[0] = ERR_NONE;
    memcpy(&buf[1], &sector_addr, 4);
    memcpy(&buf[5], &sector_len, 4);

    send_reply(buf, 9);

    return ERR_NONE;
}
static enum ERR_CODE handle_flash_cmd(enum FLASH_CMD_TYPE cmd, unsigned char *param, unsigned int len)
{
    int ret = 0;
    unsigned char cret = ERR_NONE;
    const char *name = NULL;

    switch (cmd) {
        case FLASH_CMD_ERASE_SECTOR:
        {
            unsigned int addr;
            unsigned int size;

            if (cmd == FLASH_CMD_ERASE_SECTOR)
            {
                name = "ERASE_SECTOR";
            }

            trace_flash_cmd_info(name);

            if (len != 8)
            {
                trace_flash_cmd_len_err(name, len);
                return ERR_LEN;
            }

            addr = param[0] | (param[1] << 8) | (param[2] << 16) | (param[3] << 24);
            size = param[4] | (param[5] << 8) | (param[6] << 16) | (param[7] << 24);
            TRACE(2,"addr=0x%08X size=%u", addr, size);

            if (cmd == FLASH_CMD_ERASE_SECTOR)
            {
                ret = erase_sector(addr, size);
            }

            if (ret) {
                trace_flash_cmd_err(name);
                return ERR_ERASE_FLSH;
            }

            trace_flash_cmd_done(name);

            send_reply(&cret, 1);
            break;
        }
        case FLASH_CMD_BURN_DATA:
        {
            unsigned int addr;

            if (cmd == FLASH_CMD_BURN_DATA)
            {
                name = "BURN_DATA";
            }

            trace_flash_cmd_info(name);

            if (len <= 4 || len > 20) {
                trace_flash_cmd_len_err(name, len);
                return ERR_LEN;
            }

            addr = param[0] | (param[1] << 8) | (param[2] << 16) | (param[3] << 24);
            TRACE(2,"addr=0x%08X len=%u", addr, len - 4);

            if (cmd == FLASH_CMD_BURN_DATA)
            {
                ret = burn_data(addr, &param[4], len - 4);
            }

            if (ret) {
                trace_flash_cmd_err(name);
                return ERR_BURN_FLSH;
            }

            TRACE_TIME(1,"%s verifying", name);

            if (cmd == FLASH_CMD_BURN_DATA)
            {
                ret = verify_flash_data(addr, &param[4], len - 4);
            }
            if (ret) {
                TRACE(1,"%s verify failed", name);
                return ERR_VERIFY_FLSH;
            }

            trace_flash_cmd_done(name);

            send_reply(&cret, 1);
            break;
        }
        case FLASH_CMD_ERASE_CHIP: {
            name = "CMD_ERASE_CHIP";

            trace_flash_cmd_info(name);

            if (len != 0) {
                trace_flash_cmd_len_err(name, len);
                return ERR_LEN;
            }

            ret = erase_chip();
            if (ret) {
                trace_flash_cmd_err(name);
                return ERR_ERASE_FLSH;
            }

            trace_flash_cmd_done(name);

            send_reply(&cret, 1);
            break;
        }
        default:
            TRACE(1,"Unsupported flash cmd: 0x%x", cmd);
            return ERR_FLASH_CMD;
    }

    return ERR_NONE;
}
/*
#define SECTOR_SIZE_64K                     (1 << 16)
#define SECTOR_SIZE_32K                     (1 << 15)
#define SECTOR_SIZE_16K                     (1 << 14)
#define SECTOR_SIZE_4K                      (1 << 12)
*/
#define BURN_DATA_MSG_OVERHEAD              16

//static const unsigned int size_mask = SECTOR_SIZE_32K | SECTOR_SIZE_4K;

enum PROGRAMMER_STATE {
    PROGRAMMER_NONE,
    PROGRAMMER_ERASE_BURN_START,
    PROGRAMMER_SEC_REG_ERASE_BURN_START,
};


#define BURN_BUFFER_LOC                 __attribute__((section(".burn_buffer")))

static unsigned char BURN_BUFFER_LOC data_buf[65536+2048];
static enum PROGRAMMER_STATE programmer_state=PROGRAMMER_NONE;

/*
static unsigned int count_set_bits(unsigned int i)
{
    i = i - ((i >> 1) & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}
*/

static int send_burn_data_reply(enum ERR_CODE code, unsigned short sec_seq, unsigned char seq)
{
    int ret = 0;
    enum MSG_TYPE type = TYPE_INVALID;

    if (programmer_state == PROGRAMMER_ERASE_BURN_START) {
        type = TYPE_ERASE_BURN_DATA;
    }

    send_msg.hdr.type = type;
    send_msg.hdr.seq = recv_msg.hdr.seq;
    send_msg.hdr.len = 3;
    send_msg.data[0] = code;
    send_msg.data[1] = sec_seq & 0xFF;
    send_msg.data[2] = (sec_seq >> 8) & 0xFF;
    send_msg.data[3] = ~check_sum((unsigned char *)&send_msg, MSG_TOTAL_LEN(&send_msg) - 1);

    send_msg_push_seq=send_msg_push_seq%MAX_SNED_MSG_QUEUE;
    send_msg_queue[send_msg_push_seq]=send_msg;
    send_msg_push_seq++;

    //ret = send_data((unsigned char *)&send_msg, MSG_TOTAL_LEN(&send_msg));
    return ret;
}

int anc_cmd_receve_process(uint8_t *buf,uint32_t len)
{
    enum MSG_TYPE type;
    enum ERR_CODE errcode=ERR_NONE;
    unsigned char cret = ERR_NONE;

    if(len>sizeof(recv_msg))
    {
        memcpy(&recv_msg,buf,sizeof(recv_msg));
    }
    else
    {
        memcpy(&recv_msg,buf,len);
    }
    // Checksum
    if (check_sum((unsigned char *)&recv_msg, MSG_TOTAL_LEN(&recv_msg)) != 0xFF) {
        trace_stage_info("Checksum error");
        return ERR_CHECKSUM;
    }
    type = recv_msg.hdr.type;

    //TRACE(1,"COMMAND:0x%x",type);

    switch (type) {
        case TYPE_SYS:
        {
            TRACE(0,"SYS CMD");
            break;
        }
        case TYPE_READ:
        {
            unsigned int addr;
            unsigned int len;

            trace_stage_info("READ CMD");

            addr = recv_msg.data[0] | (recv_msg.data[1] << 8) | (recv_msg.data[2] << 16) | (recv_msg.data[3] << 24);
            len = recv_msg.data[4];

            TRACE(2,"addr:0x%x,len:%d",addr,len);
#if defined(CHIP_BEST1501)
            addr=addr&0xefffffff;
#endif
            errcode = handle_read_cmd(type, addr, len);
            if (errcode != ERR_NONE) {
                return errcode;
            }
            break;
        }
        case TYPE_WRITE:
        {
            unsigned int addr;
            unsigned int len;
            unsigned char *wdata;

          //  trace_stage_info("WRITE CMD");

            addr = recv_msg.data[0] | (recv_msg.data[1] << 8) | (recv_msg.data[2] << 16) | (recv_msg.data[3] << 24);
            len = recv_msg.hdr.len - 4;
            wdata = &recv_msg.data[4];

#if defined(CHIP_BEST1501)
            addr=addr&0xefffffff;
#endif

            errcode = handle_write_cmd(addr, len, wdata);
            if (errcode != ERR_NONE) {
                return errcode;
            }
            break;
        }
       case TYPE_ERASE_BURN_START:
       {
            TRACE(0,"TYPE_ERASE_BURN_START CMD");

            if(programmer_state == PROGRAMMER_NONE)
            {
                trace_stage_info("ERASE_BURN_START");

                burn_addr = recv_msg.data[0] | (recv_msg.data[1] << 8) | (recv_msg.data[2] << 16) | (recv_msg.data[3] << 24);
                burn_total_len = recv_msg.data[4] | (recv_msg.data[5] << 8) | (recv_msg.data[6] << 16) | (recv_msg.data[7] << 24);
                sector_size = recv_msg.data[8] | (recv_msg.data[9] << 8) | (recv_msg.data[10] << 16) | (recv_msg.data[11] << 24);

#if defined(CHIP_BEST1501)
            burn_addr=burn_addr&0xefffffff;
#endif

                TRACE(3,"burn_addr=0x%08X burn_total_len=%u sector_size=%u", burn_addr, burn_total_len, sector_size);

                /*if ((size_mask & sector_size) == 0 || count_set_bits(sector_size) != 1)
                {
                    TRACE(2,"Unsupported sector_size=0x%08X mask=0x%08X", sector_size, size_mask);
                    return ERR_SECTOR_SIZE;
                }*/

                sector_cnt = burn_total_len / sector_size;
                last_sector_len = burn_total_len % sector_size;

                if (last_sector_len)
                {
                    sector_cnt++;
                }
                else
                {
                    last_sector_len = sector_size;
                }

                if (sector_cnt > 0xFFFF)
                {
                    TRACE(1,"Sector seq overflow: %u", sector_cnt);
                    return ERR_SECTOR_SEQ_OVERFLOW;
                }

                send_reply(&cret, 1);

                if (burn_total_len == 0) {
                    TRACE(0,"Burn length = 0");
                    break;
                }

                programmer_state = PROGRAMMER_ERASE_BURN_START;

                burn_len=0;


                trace_stage_info("ERASE_BURN_START end");
            }
            else
            {
                TRACE(1,"ERROR programmer_state status:%d",programmer_state);
            }
            break;
        }
       case TYPE_ERASE_BURN_DATA:
       {

            if(programmer_state == PROGRAMMER_ERASE_BURN_START)
            {
                unsigned int dlen;
                unsigned int mcrc;
                unsigned int crc;

                dlen = recv_msg.data[0] | (recv_msg.data[1] << 8) |  (recv_msg.data[2] << 16) | (recv_msg.data[3] << 24);
                mcrc = recv_msg.data[4] | (recv_msg.data[5]  << 8) | (recv_msg.data[6]  << 16) | (recv_msg.data[7]  << 24);

                cur_sector_seq = recv_msg.data[8]  | (recv_msg.data[9] << 8);

                TRACE(2," sec_seq=%u dlen=%u",  cur_sector_seq, dlen);

                if (cur_sector_seq >= sector_cnt) {
                    TRACE(2,"Bad sector seq: sec_seq=%u sector_cnt=%u", cur_sector_seq, sector_cnt);
                    send_burn_data_reply(ERR_SECTOR_SEQ, cur_sector_seq, recv_msg.hdr.seq);
                    return ERR_NONE;
                }

                if (((cur_sector_seq + 1) == sector_cnt && dlen != last_sector_len) ||
                        ((cur_sector_seq + 1) != sector_cnt && dlen != sector_size)) {
                    TRACE(2," Bad data len: sec_seq=%u dlen=%u", cur_sector_seq, dlen);
                    send_burn_data_reply(ERR_SECTOR_DATA_LEN, cur_sector_seq, recv_msg.hdr.seq);
                    return ERR_NONE;
                }

                crc = crc32_c(0, (unsigned char *)&buf[BURN_DATA_MSG_OVERHEAD], dlen);
                if (crc != mcrc)
                {
                    TRACE(0,"Bad CRC");
                    send_burn_data_reply(ERR_SECTOR_DATA_CRC, cur_sector_seq, recv_msg.hdr.seq);
                    return ERR_NONE;
                }

                if(burn_len+dlen<=burn_total_len)
                {
                    memcpy(&data_buf[burn_len],(unsigned char *)&buf[BURN_DATA_MSG_OVERHEAD], dlen);
                    burn_len=burn_len+dlen;
                }
                else
                {
                    TRACE(2,"Error burn_len=%d,burn_total_len=%d", burn_len, burn_total_len);
                }

                if(burn_len==burn_total_len)
                {
                    TRACE(2,"BURN_DATA addr=0x%08X len=%u .........................", burn_addr, burn_total_len);
                    int ret=burn_data(burn_addr,data_buf,burn_total_len);
                    if (ret)
                    {
                        TRACE(2,"### FLASH_TASK: BURN_DATA failed: addr=0x%08X len=%u ###", burn_addr, burn_total_len);
                        return 0;
                    }
                    else
                    {
                        TRACE(0,"burn sucessful");
                    }
                    ret = verify_flash_data(burn_addr,data_buf,burn_total_len);
                    if (ret)
                    {
                        TRACE(0,"verify failed");
                        return ERR_VERIFY_FLSH;
                    }
                    else
                    {
                        TRACE(0,"verify sucessful");
                    }

                }

                send_burn_data_reply(ERR_BURN_OK,cur_sector_seq,recv_msg.hdr.seq);

            }
            else
            {
                TRACE(1,"ERROR programmer_state status:%d",programmer_state);
            }
            break;
        }
        case TYPE_FLASH_CMD:
        {
            trace_stage_info("FLASH CMD");

            errcode = handle_flash_cmd((enum FLASH_CMD_TYPE)recv_msg.data[0], &recv_msg.data[1], recv_msg.hdr.len - 1);

            TRACE_TIME(1,"FLASH CMD ret=%d", errcode);

            if (errcode != ERR_NONE) {
                return errcode;
            }
            break;
        }
        case TYPE_GET_SECTOR_INFO:
        {
            unsigned int addr;

            trace_stage_info("GET SECTOR INFO");

            addr = recv_msg.data[0] | (recv_msg.data[1] << 8) | (recv_msg.data[2] << 16) | (recv_msg.data[3] << 24);

            errcode = handle_sector_info_cmd(addr);
            if (errcode != ERR_NONE) {
                return errcode;
            }
            break;
        }
       case TYPE_PROD_TEST: {
            uint32_t cmd;

            trace_stage_info("ANC CMD");

            cmd = recv_msg.data[0] | (recv_msg.data[1] << 8) | (recv_msg.data[2] << 16) | (recv_msg.data[3] << 24);
            if (cmd != PROD_TEST_CMD_ANC) {
                return ERR_TYPE_INVALID;
            }

            if (recv_msg.hdr.len < 5) {
                TRACE(1,"PROD_TEST/ANC msg length error: %d", recv_msg.hdr.len);
                return ERR_LEN;
            }

            errcode = handle_anc_cmd(recv_msg.data[4], &recv_msg.data[5], recv_msg.hdr.len - 5);
            if (errcode != ERR_NONE) {
                return errcode;
            }

            break;
        }
        default:
            TRACE(1,"COMMAND:0x%x",type);

            cret = ERR_TYPE_INVALID;
            send_reply((unsigned char *)&cret, 1);
            break;

        }


    return 0;
}
int anc_cmd_send_process(uint8_t **pbuf,uint16_t *len)
{


    send_msg_pop_seq=send_msg_pop_seq%MAX_SNED_MSG_QUEUE;
    *pbuf = (uint8_t *)&send_msg_queue[send_msg_pop_seq];
    *len = MSG_TOTAL_LEN(&send_msg);
    send_msg_pop_seq++;

    return 0;
}
#endif









