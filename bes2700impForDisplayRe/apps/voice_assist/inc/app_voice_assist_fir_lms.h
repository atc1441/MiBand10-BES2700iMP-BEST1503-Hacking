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
#ifndef __APP_VOICE_ASSIST_FIR_LMS_H__
#define __APP_VOICE_ASSIST_FIR_LMS_H__

#define FIR_ANC_SYS_FREQ_LOG

#include "plat_types.h"
#include "app_anc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    anc_assist_user_t user;
    uint32_t len;
    uint32_t sub_cmd;
    uint8_t buf[256];
} app_voice_assist_core_interact_data_t;

int32_t app_voice_assist_fir_lms_init(void);
int32_t app_voice_assist_fir_lms_open(app_anc_mode_t mode);
int32_t app_voice_assist_fir_lms_switch(app_anc_mode_t mode);
int32_t app_voice_assist_fir_lms_reset(void);
int32_t app_voice_assist_fir_lms_close(void);
int32_t app_voice_assist_fir_lms_enable_fir(bool enable);
int32_t app_voice_assist_fir_lms_set_fir_status(bool enable);
int32_t app_voice_assist_fir_lms_set_tt_algo_status(int32_t enable);
int32_t app_voice_assist_fir_lms_set_anc_mode(app_anc_mode_t mode);
bool app_voice_assist_fir_lms_get_fir_status(void);
void set_fir_cache(void);
void set_mc_fir_cache(void);

int32_t app_voice_assist_fir_lms_set_fb_gain(float gain);
int32_t app_voice_assist_fir_lms_set_tt_ns_cfg(void *cfg);

int32_t app_anc_assist_fir_lms_open(app_anc_mode_t mode);
int32_t app_anc_assist_fir_lms_close(void);

int32_t app_voice_assist_fir_lms_start_fir(void);
int32_t app_voice_assist_fir_lms_stop_fir(void);

int voice_assist_fir_lms_open(void *calib_gain);
int voice_assist_fir_lms_close(void);
int voice_assist_fir_lms_set_cfg(uint32_t ctrl, uint8_t *tgt_cfg, uint32_t ptr_len);

void app_voice_assist_fir_lms_set_fir_HW_work_mode(int mode);
#ifdef __cplusplus
}
#endif

#endif