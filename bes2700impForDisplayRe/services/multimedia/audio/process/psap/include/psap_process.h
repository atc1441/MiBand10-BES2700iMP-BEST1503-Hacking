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
#ifndef __PSAP_PROCESS_H__
#define __PSAP_PROCESS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"
#include "aud_section.h"
#include "hal_aud.h"

typedef enum {
    PSAP_NO_ERR=0,
    PSAP_NUM_ERR,
    PSAP_OTHER_ERR,
    PSAP_ERR_TOTAL,
}PSAP_ERROR;

typedef enum {
    PSAP_MODE_NO=0,
    PSAP_MODE_ONLY_MIC=1,
    PSAP_MODE_MIC_MUSIC=2,
    PSAP_MODE_ONLY_MUSIC=3,
    PSAP_MODE_TOTAL,
}PSAP_MODE;

int psap_open(int ch_map);
void psap_close(void);
void psap_enable(int ch_map);
void psap_disable(int ch_map);
void psap_set_total_gain(int32_t gain_l, int32_t gain_r);
void psap_set_total_gain_f32(float gain_l, float gain_r);
int psap_set_cfg_coef(const struct_psap_cfg * cfg);
int psap_get_gain(int32_t *gain_ch_l, int32_t *gain_ch_r, uint32_t band_index);
int psap_set_gain(int32_t gain_ch_l, int32_t gain_ch_r, uint32_t band_index);
void psap_set_bands_same_gain_f32(float gain_l, float gain_r, uint32_t index_start, uint32_t index_end);
void psap_set_bands_gain_f32(float *gain_l, float *gain_r);
int psap_opened(void);

int psap_select_coef(enum AUD_SAMPRATE_T rate,int index);
int psap_set_mode(int ch_map,PSAP_MODE psap_mode);

#ifdef __cplusplus
}
#endif

#endif
