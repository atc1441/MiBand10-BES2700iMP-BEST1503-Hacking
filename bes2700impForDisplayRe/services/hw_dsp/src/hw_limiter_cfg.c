/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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

#include "hal_trace.h"
#include "hw_limiter_cfg.h"
#include "tgt_hardware.h"

#if defined(__AUDIO_HW_LIMITER__)
#include "psap_process.h"
#endif

#ifndef CODEC_OUTPUT_DEV
#define CODEC_OUTPUT_DEV                    CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV
#endif

#if defined(__AUDIO_HW_LIMITER__)
extern const struct_psap_cfg * hwlimiter_para_list_50p7k[HWLIMITER_PARA_LIST_NUM];
extern const struct_psap_cfg * hwlimiter_para_list_48k[HWLIMITER_PARA_LIST_NUM];
extern const struct_psap_cfg * hwlimiter_para_list_44p1k[HWLIMITER_PARA_LIST_NUM];

const struct_psap_cfg * WEAK hwlimiter_para_list_50p7k[HWLIMITER_PARA_LIST_NUM] = { };
const struct_psap_cfg * WEAK hwlimiter_para_list_48k[HWLIMITER_PARA_LIST_NUM] = { };
const struct_psap_cfg * WEAK hwlimiter_para_list_44p1k[HWLIMITER_PARA_LIST_NUM] = { };

int hwlimiter_open(int ch_map)
{
    TRACE(1,"%s", __func__);
    int err = 0;
    err = psap_open(ch_map);
    return err;
}

void hwlimiter_enable(int ch_map)
{
    TRACE(1,"%s", __func__);
    psap_enable(ch_map);
}

void hwlimiter_disable(int ch_map)
{
    TRACE(1,"%s", __func__);
    psap_disable(ch_map);
}

void hwlimiter_close(void)
{
    TRACE(1,"%s", __func__);
    psap_close();
}

int hwlimiter_set_cfg(enum AUD_SAMPRATE_T rate,int index)
{
    TRACE(1,"%s", __func__);
    const struct_psap_cfg **list=NULL;

    if (index >= HWLIMITER_PARA_LIST_NUM) {
        return 1;
    }

    switch(rate)
    {
        case AUD_SAMPRATE_48000:
            list=hwlimiter_para_list_48k;
            break;

        case AUD_SAMPRATE_44100:
            list=hwlimiter_para_list_44p1k;
            break;

#ifdef __AUDIO_RESAMPLE__
        case AUD_SAMPRATE_50781:
            list=hwlimiter_para_list_50p7k;
            break;
#endif

        default:
            break;
    }

    ASSERT(list!=NULL&&list[index]!=NULL,"The hwlimiter para of Samprate %d is NULL",rate);

    if(psap_opened())
    {
        psap_set_cfg_coef(list[index]);
    }

    return 0;
}

#endif




