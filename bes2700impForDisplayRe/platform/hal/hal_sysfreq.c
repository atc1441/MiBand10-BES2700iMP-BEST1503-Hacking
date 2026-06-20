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
#include "hal_sysfreq.h"
#include "hal_location.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "cmsis.h"

#if !(defined(ROM_BUILD) || defined(CHIP_ROLE_CP) || \
        (defined(CHIP_SUBSYS_BTH) ^ defined(BTH_AS_MAIN_MCU)) || \
        defined(CHIP_SUBSYS_SENS))
#define PMU_CTRL_ENABLE
#endif

#ifdef PMU_CTRL_ENABLE
#include "pmu.h"
#include <string.h>
#endif

static uint32_t BOOT_BSS_LOC sysfreq_bundle[(HAL_SYSFREQ_USER_QTY + 3) / 4];

static uint8_t * const sysfreq_per_user = (uint8_t *)&sysfreq_bundle[0];

static enum HAL_SYSFREQ_USER_T BOOT_DATA_LOC top_user = HAL_SYSFREQ_USER_QTY;

// NOTE: min_sysfreq must be initialized to 32K to avoid first hal_sysfreq_req() error
static enum HAL_CMU_FREQ_T BOOT_DATA_LOC min_sysfreq = HAL_CMU_FREQ_32K;

#ifdef SYSFREQ_STATS
static uint32_t BOOT_BSS_LOC last_check_time;
static uint32_t BOOT_BSS_LOC sysfreq_intvl[HAL_SYSFREQ_USER_QTY][2];
static SYSTEM_SYSFREQ_STAT_T system_sysfreq_stat_info;
#endif

#ifdef SYSFREQ_STATS
BOOT_TEXT_SRAM_LOC
#endif
static enum HAL_CMU_FREQ_T hal_sysfreq_revise_freq(enum HAL_CMU_FREQ_T freq)
{
    if (freq == HAL_CMU_FREQ_32K) {
        freq = HAL_CMU_FREQ_26M;
    }
    return (freq > min_sysfreq) ? freq : min_sysfreq;
}

void hal_sysfreq_set_min_freq(enum HAL_CMU_FREQ_T freq)
{
    uint32_t lock;
    enum HAL_CMU_FREQ_T cur_freq;
    enum HAL_CMU_FREQ_T old_revised_freq;
    enum HAL_CMU_FREQ_T new_revised_freq;

    lock = int_lock();

    if (min_sysfreq != freq) {
        cur_freq = hal_sysfreq_get();
        old_revised_freq = hal_sysfreq_revise_freq(cur_freq);
        min_sysfreq = freq;
        new_revised_freq = hal_sysfreq_revise_freq(cur_freq);
        if (old_revised_freq != new_revised_freq) {
            hal_cmu_sys_set_freq(new_revised_freq);
        }
    }

    int_unlock(lock);
}

enum HAL_CMU_FREQ_T hal_sysfreq_inc_min_freq(enum HAL_CMU_FREQ_T freq)
{
    uint32_t lock;
    enum HAL_CMU_FREQ_T old_min_freq;

    lock = int_lock();

    old_min_freq = min_sysfreq;

    if (freq > min_sysfreq) {
        hal_sysfreq_set_min_freq(freq);
    }

    int_unlock(lock);

    return old_min_freq;
}

int hal_sysfreq_req(enum HAL_SYSFREQ_USER_T user, enum HAL_CMU_FREQ_T freq)
{
    uint32_t lock;
    enum HAL_CMU_FREQ_T cur_freq;
    enum HAL_CMU_FREQ_T real_freq;
    enum HAL_CMU_FREQ_T real_cur_freq;
    int i;

    if (user >= HAL_SYSFREQ_USER_QTY) {
        return 1;
    }
    if (freq >= HAL_CMU_FREQ_QTY) {
        return 2;
    }

#ifdef HAL_CMU_FREQ_MAX
    freq = freq > HAL_CMU_FREQ_MAX ? HAL_CMU_FREQ_MAX : freq;
#elif defined(HAL_CMU_FREQ_FIXED)
    freq = HAL_CMU_FREQ_FIXED;
#endif

    lock = int_lock();

    cur_freq = hal_sysfreq_get();

    sysfreq_per_user[user] = freq;

    if (freq == cur_freq) {
        top_user = user;
    } else if (freq > cur_freq) {
        top_user = user;
        real_freq = hal_sysfreq_revise_freq(freq);
        real_cur_freq = hal_sysfreq_revise_freq(cur_freq);
        // It is possible that revised freq <= revised cur_freq (e.g., when cur_freq=32K)
        if (real_freq != real_cur_freq) {
#ifdef PMU_CTRL_ENABLE
            pmu_sys_freq_config(real_freq);
#endif
#ifdef ULTRA_LOW_POWER
            // Enable PLL if required
            hal_cmu_low_freq_mode_disable(real_cur_freq, real_freq);
#endif
            hal_cmu_sys_set_freq(real_freq);
        }
#ifdef SYSFREQ_STATS
        if (real_freq != real_cur_freq || cur_freq == HAL_CMU_FREQ_32K) {
            uint32_t idx = (cur_freq == HAL_CMU_FREQ_32K) ? cur_freq : real_cur_freq;
            uint32_t cur_time = hal_sys_timer_get();
            sysfreq_intvl[idx][0] += cur_time - last_check_time;
            last_check_time = cur_time;
        }
#endif
    } else /* if (freq < cur_freq) */ {
        if (top_user == user || top_user == HAL_SYSFREQ_USER_QTY) {
            if (top_user == user) {
                freq = sysfreq_per_user[0];
                user = 0;
                for (i = 1; i < HAL_SYSFREQ_USER_QTY; i++) {
                    if (freq < sysfreq_per_user[i]) {
                        freq = sysfreq_per_user[i];
                        user = i;
                    }
                }
            }
            top_user = user;
            if (freq != cur_freq) {
                real_freq = hal_sysfreq_revise_freq(freq);
                real_cur_freq = hal_sysfreq_revise_freq(cur_freq);
                // It is possible that revised freq >= revised cur_freq (e.g., when freq=32K)
                if (real_freq != real_cur_freq) {
                    hal_cmu_sys_set_freq(real_freq);
#ifdef ULTRA_LOW_POWER
                    // Disable PLL if capable
                    hal_cmu_low_freq_mode_enable(real_cur_freq, real_freq);
#endif
#ifdef PMU_CTRL_ENABLE
                    pmu_sys_freq_config(real_freq);
#endif
                }
#ifdef SYSFREQ_STATS
                if (real_freq != real_cur_freq || freq == HAL_CMU_FREQ_32K) {
                    uint32_t cur_time = hal_sys_timer_get();
                    sysfreq_intvl[real_cur_freq][0] += cur_time - last_check_time;
                    last_check_time = cur_time;
                }
#endif
            }
        }
    }

    int_unlock(lock);

    return 0;
}

enum HAL_CMU_FREQ_T hal_sysfreq_get(void)
{
    if (top_user < HAL_SYSFREQ_USER_QTY) {
        return sysfreq_per_user[top_user];
    } else {
        return hal_cmu_sys_get_freq();
    }
}

enum HAL_CMU_FREQ_T hal_sysfreq_get_hw_freq(void)
{
    if (top_user < HAL_SYSFREQ_USER_QTY) {
        return hal_sysfreq_revise_freq(sysfreq_per_user[top_user]);
    } else {
        return hal_cmu_sys_get_freq();
    }
}

int hal_sysfreq_busy(void)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(sysfreq_bundle); i++) {
        if (sysfreq_bundle[i] != 0) {
            return 1;
        }
    }

    return 0;
}

void hal_sysfreq_print_user_freq(void)
{
    int i;

    TRACE(0, "SYSFREQ USER FREQ:");
    for (i = 0; i < HAL_SYSFREQ_USER_QTY; i++) {
        if (sysfreq_per_user[i] != 0) {
            TRACE(TR_ATTR_NO_TS,"\t[%2u] f=%2u", i, sysfreq_per_user[i]);
        }
    }
    TRACE(TR_ATTR_NO_TS,"\ttop_user=%2u", top_user);
}

#ifdef SYSFREQ_STATS
SRAM_TEXT_LOC
static uint32_t hal_sysfreq_get_freq_stats_index(void)
{
    enum HAL_CMU_FREQ_T cur_freq;
    uint32_t idx;

    if (top_user >= HAL_SYSFREQ_USER_QTY) {
        return HAL_CMU_FREQ_26M;
    }
    cur_freq = sysfreq_per_user[top_user];
    if (cur_freq == HAL_CMU_FREQ_32K) {
        idx = cur_freq;
    } else {
        idx = hal_sysfreq_revise_freq(cur_freq);
    }

    return idx;
}

SRAM_TEXT_LOC
void hal_sysfreq_add_freq_time(int idle, uint32_t cur_time)
{
    uint32_t idx;

    idx = hal_sysfreq_get_freq_stats_index();
    sysfreq_intvl[idx][idle ? 1 : 0] += cur_time - last_check_time;
    last_check_time = cur_time;
}

void hal_sysfreq_print_freq_stats(void)
{
    uint32_t total_intvl;
    uint32_t lock;
    uint32_t idx;
    uint32_t cur_time;
    uint32_t i;
    uint32_t ratio[2];

    total_intvl = 0;

    lock = int_lock();

    idx = hal_sysfreq_get_freq_stats_index();
    cur_time = hal_sys_timer_get();
    sysfreq_intvl[idx][0] += cur_time - last_check_time;

    for (i = 0; i < ARRAY_SIZE(sysfreq_intvl); i++) {
        total_intvl += sysfreq_intvl[i][0] + sysfreq_intvl[i][1];
    }

    TRACE(0, "SYSFREQ FREQ STATS:");

    if (total_intvl) {
        for (i = 0; i < ARRAY_SIZE(sysfreq_intvl); i++) {
            if (sysfreq_intvl[i][0] || sysfreq_intvl[i][1]) {
                ratio[0] = sysfreq_intvl[i][0] * 100 / total_intvl;
                ratio[1] = sysfreq_intvl[i][1] * 100 / total_intvl;
                TRACE(TR_ATTR_NO_TS, "\t[%2u]: B=(%5u ms %2u%%) I=(%5u ms %2u%%)", i,
                    TICKS_TO_MS(sysfreq_intvl[i][0]), ratio[0],
                    TICKS_TO_MS(sysfreq_intvl[i][1]), ratio[1]);
            }
        }
        system_sysfreq_stat_update(total_intvl, sysfreq_intvl, sizeof(sysfreq_intvl));
    }

    // Reset intervals
    for (i = 0; i < ARRAY_SIZE(sysfreq_intvl); i++) {
        sysfreq_intvl[i][0] = 0;
        sysfreq_intvl[i][1] = 0;
    }
    last_check_time = cur_time;

    int_unlock(lock);
}

WEAK SYSTEM_SYSFREQ_STAT_T* system_sysfreq_stat_get(void)
{
    return &system_sysfreq_stat_info;
}

WEAK void system_sysfreq_stat_update(uint32_t tot, uint32_t arrFreq[][2], uint8_t size)
{
    system_sysfreq_stat_info.total_intvl = tot;
    memcpy((uint8_t*)(system_sysfreq_stat_info.sysfreq_intvl), (uint8_t*)arrFreq, size);
}

#endif
