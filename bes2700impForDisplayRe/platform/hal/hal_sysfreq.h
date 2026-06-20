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
#ifndef __HAL_SYSFREQ_H__
#define __HAL_SYSFREQ_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_cmu.h"

enum HAL_SYSFREQ_USER_T {
    HAL_SYSFREQ_USER_INIT,              // 0
    HAL_SYSFREQ_USER_OVERLAY,           // 1
    HAL_SYSFREQ_USER_OVERLAY_SUBSYS,    // 2
    HAL_SYSFREQ_USER_USB,               // 3
    HAL_SYSFREQ_USER_BT,                // 4
    HAL_SYSFREQ_USER_ANC,               // 5
    HAL_SYSFREQ_USER_TRNG,              // 6
    HAL_SYSFREQ_USER_FIR,               // 7
    HAL_SYSFREQ_USER_SPDIF,             // 8
    HAL_SYSFREQ_USER_DSP,               // 9
    HAL_SYSFREQ_USER_EMMC,              // 10
    HAL_SYSFREQ_USER_LCDC,              // 11

    HAL_SYSFREQ_USER_APP_0,             // 12
    HAL_SYSFREQ_USER_APP_1,             // 13
    HAL_SYSFREQ_USER_APP_2,             // 14
    HAL_SYSFREQ_USER_APP_3,             // 15
    HAL_SYSFREQ_USER_APP_4,             // 16
    HAL_SYSFREQ_USER_APP_5,             // 17
    HAL_SYSFREQ_USER_APP_6,             // 18
    HAL_SYSFREQ_USER_APP_7,             // 19
    HAL_SYSFREQ_USER_APP_8,             // 20
    HAL_SYSFREQ_USER_APP_9,             // 21
    HAL_SYSFREQ_USER_APP_10,            // 22
    HAL_SYSFREQ_USER_APP_11,            // 23
    HAL_SYSFREQ_USER_APP_12,            // 24
    HAL_SYSFREQ_USER_APP_13,            // 25
    HAL_SYSFREQ_USER_APP_14,            // 26
    HAL_SYSFREQ_USER_APP_15,            // 27
    HAL_SYSFREQ_USER_APP_16,            // 28
    HAL_SYSFREQ_USER_APP_17,            // 29
    HAL_SYSFREQ_USER_APP_18,            // 30
    HAL_SYSFREQ_USER_APP_19,            // 31
    HAL_SYSFREQ_USER_APP_20,            // 32
    HAL_SYSFREQ_USER_APP_21,            // 33
    HAL_SYSFREQ_USER_APP_22,            // 34
    HAL_SYSFREQ_USER_APP_23,            // 35
    HAL_SYSFREQ_USER_APP_24,            // 36
    HAL_SYSFREQ_USER_APP_25,            // 37
    HAL_SYSFREQ_USER_APP_26,            // 38
    HAL_SYSFREQ_USER_APP_27,            // 39

    HAL_SYSFREQ_USER_QTY
};

#ifdef SYSFREQ_STATS
typedef struct {
    uint32_t total_intvl;
    uint32_t sysfreq_intvl[HAL_SYSFREQ_USER_QTY][2];
} SYSTEM_SYSFREQ_STAT_T;
#endif

void hal_sysfreq_set_min_freq(enum HAL_CMU_FREQ_T freq);

enum HAL_CMU_FREQ_T hal_sysfreq_inc_min_freq(enum HAL_CMU_FREQ_T freq);

int hal_sysfreq_req(enum HAL_SYSFREQ_USER_T user, enum HAL_CMU_FREQ_T freq);

enum HAL_CMU_FREQ_T hal_sysfreq_get(void);

enum HAL_CMU_FREQ_T hal_sysfreq_get_hw_freq(void);

int hal_sysfreq_busy(void);

void hal_sysfreq_print_user_freq(void);

void hal_sysfreq_add_freq_time(int idle, uint32_t cur_time);

void hal_sysfreq_print_freq_stats(void);

#ifdef SYSFREQ_STATS
SYSTEM_SYSFREQ_STAT_T* system_sysfreq_stat_get(void);

void system_sysfreq_stat_update(uint32_t tot, uint32_t arrFreq[][2], uint8_t size);
#endif

#ifdef __cplusplus
}
#endif

#endif
