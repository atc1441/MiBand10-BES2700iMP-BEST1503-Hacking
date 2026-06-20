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
#include "analog.h"
#include "cmsis.h"
#include "hal_cache.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_gpadc.h"
#include "hal_location.h"
#include "hal_norflash.h"
#include "hal_psram.h"
#include "hal_psramuhs.h"
#include "hal_sleep.h"
#include "hal_spi.h"
#include "hal_sysfreq.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_uart.h"
#include "pmu.h"

#if !(defined(CHIP_ROLE_CP) || defined(CHIP_SUBSYS_SENS) || \
        (defined(CHIP_SUBSYS_BTH) ^ defined(BTH_AS_MAIN_MCU)))
#define MAIN_MCU_SLEEP_CTRL
#endif
#if (defined(MAIN_MCU_SLEEP_CTRL) || defined(SUBSYS_FLASH_BOOT)) && !defined(NO_FLASH_INIT)
#define FLASH_SLEEP_CTRL
#endif

#ifdef CORE_SLEEP_POWER_DOWN
#define PM_NOTIF_HDLR_CNT                   10
#else
#define PM_NOTIF_HDLR_CNT                   3
#endif

//static uint8_t SRAM_STACK_LOC sleep_stack[128];

static HAL_SLEEP_HOOK_HANDLER sleep_hook_handler[HAL_SLEEP_HOOK_USER_QTY];
static HAL_DEEP_SLEEP_HOOK_HANDLER deep_sleep_hook_handler[HAL_DEEP_SLEEP_HOOK_USER_QTY];

static uint32_t cpu_wake_lock_map;
static uint32_t sys_wake_lock_map;
static uint32_t chip_wake_lock_map;
#ifdef CORE_SLEEP_POWER_DOWN
static bool power_down_en = true;
#endif

static uint8_t pm_user_cnt;
STATIC_ASSERT((1 << (sizeof(pm_user_cnt) * 8)) > PM_NOTIF_HDLR_CNT, "pm_user_cnt to small to hold PM_NOTIF_HDLR_CNT");
static enum HAL_PM_USER_TYPE_T pm_user[PM_NOTIF_HDLR_CNT];
static HAL_PM_NOTIF_HANDLER pm_notif[PM_NOTIF_HDLR_CNT];

#ifdef SLEEP_STATS_TRACE
static uint32_t stats_trace_interval;
static uint32_t stats_trace_time;
#ifdef WAKEUP_SRC_STATS
static uint32_t wakeup_start_time;
static uint32_t wakeup_src_map[(USER_IRQn_QTY + 31) / 32];
static uint32_t wakeup_multi_src_map[(USER_IRQn_QTY + 31) / 32];
static uint32_t wakeup_total_interval[USER_IRQn_QTY];
#endif
#endif
static uint32_t stats_interval;
static uint32_t stats_start_time;
static uint32_t light_sleep_time;
static uint32_t sys_deep_sleep_time;
static uint32_t chip_deep_sleep_time;
static bool stats_started;
static bool stats_valid;
static uint8_t light_sleep_ratio;
static uint8_t sys_deep_sleep_ratio;
static uint8_t chip_deep_sleep_ratio;
static SYSTEM_SLEEP_STAT_T system_sleep_stat_info;

void hal_sleep_start_stats(uint32_t stats_interval_ms, uint32_t trace_interval_ms)
{
    uint32_t lock;

    lock = int_lock();
    if (stats_interval_ms) {
        stats_interval = MS_TO_TICKS(stats_interval_ms);
        stats_start_time = hal_sys_timer_get();
        light_sleep_time = 0;
        sys_deep_sleep_time = 0;
        chip_deep_sleep_time = 0;
        stats_valid = false;
        stats_started = true;
    } else {
        stats_started = false;
    }
    int_unlock(lock);

#ifdef SLEEP_STATS_TRACE
    if (stats_interval_ms && trace_interval_ms) {
        stats_trace_interval = MS_TO_TICKS(trace_interval_ms);
        stats_trace_time = stats_start_time;
    } else {
        stats_trace_interval = 0;
    }
#ifdef WAKEUP_SRC_STATS
    int i;

    for (i = 0; i < ARRAY_SIZE(wakeup_src_map); i++) {
        wakeup_src_map[i] = 0;
        wakeup_multi_src_map[i] = 0;
    }
    for (i = 0; i < ARRAY_SIZE(wakeup_total_interval); i++) {
        wakeup_total_interval[i] = 0;
    }
#endif
#ifdef CACHE_STATS
    hal_cache_monitor_enable(HAL_CACHE_ID_I_CACHE);
    hal_cache_monitor_enable(HAL_CACHE_ID_D_CACHE);
#endif
#endif
}

int hal_sleep_get_stats(struct CPU_USAGE_T *usage)
{
    int ret;
    uint32_t lock;

    if (!usage) {
        return -1;
    }

    lock = int_lock();
    if (stats_valid) {
        usage->light_sleep = light_sleep_ratio;
        usage->sys_deep_sleep = sys_deep_sleep_ratio;
        usage->chip_deep_sleep = chip_deep_sleep_ratio;
        usage->busy = 100 - (light_sleep_ratio + sys_deep_sleep_ratio + chip_deep_sleep_ratio);
        ret = 0;
    } else {
        ret = 1;
    }
    int_unlock(lock);

    return ret;
}

#if defined(SLEEP_STATS_TRACE) && defined(WAKEUP_SRC_STATS)
static void SRAM_TEXT_LOC hal_sleep_calc_wakeup_interval(uint32_t cur_time)
{
    uint32_t interval;
    int i;
    int j;
    int index;
    uint32_t src_cnt = 0;

    interval = cur_time - wakeup_start_time;

    for (i = 0; i < ARRAY_SIZE(wakeup_src_map); i++) {
        for (j = 0; j < 32; j++) {
            if (wakeup_src_map[i] & (1 << j)) {
                index = i * 32 + j;
                if (index < ARRAY_SIZE(wakeup_total_interval)) {
                    wakeup_total_interval[index] += interval;
                    src_cnt++;
                }
            }
        }
    }

    if (src_cnt >= 2) {
        for (i = 0; i < ARRAY_SIZE(wakeup_src_map); i++) {
            wakeup_multi_src_map[i] |= wakeup_src_map[i];
        }
    }
}

static void SRAM_TEXT_LOC hal_sleep_save_wakeup_src(void)
{
    int i;

    wakeup_start_time = hal_sys_timer_get();

    for (i = 0; i < ARRAY_SIZE(wakeup_src_map); i++) {
        wakeup_src_map[i] = (NVIC->ICPR[i] & NVIC->ISER[i]);
    }
}

static void hal_sleep_print_wakeup_src(void)
{
    int i;
    uint32_t cur_time;

    cur_time = hal_sys_timer_get();

    hal_sleep_calc_wakeup_interval(cur_time);

    // Set wakeup start time to current time
    wakeup_start_time = cur_time;

    TRACE(0, "DEEP SLEEP WAKEUP SRC STATS:");
    for (i = 0; i < ARRAY_SIZE(wakeup_total_interval); i++) {
        if (wakeup_total_interval[i]) {
            TRACE(TR_ATTR_NO_TS, "\t[%2u]: %5u ms", i, TICKS_TO_MS(wakeup_total_interval[i]));
            // Reset the total interval
            wakeup_total_interval[i] = 0;
        }
    }
    // Reset the multi src map but keep current wakeup src map
    for (i = 0; i < ARRAY_SIZE(wakeup_src_map); i++) {
        wakeup_multi_src_map[i] = 0;
    }
}
#endif

int hal_sleep_set_sleep_hook(enum HAL_SLEEP_HOOK_USER_T user, HAL_SLEEP_HOOK_HANDLER handler)
{
    if (user >= ARRAY_SIZE(sleep_hook_handler)) {
        return 1;
    }
    sleep_hook_handler[user] = handler;
    return 0;
}

static int SRAM_TEXT_LOC hal_sleep_exec_sleep_hook(void)
{
    int i;
    int ret;

    for (i = 0; i < ARRAY_SIZE(sleep_hook_handler); i++) {
        if (sleep_hook_handler[i]) {
            ret = sleep_hook_handler[i]();
            if (ret) {
                return ret;
            }
        }
    }

    return 0;
}

int hal_sleep_set_deep_sleep_hook(enum HAL_DEEP_SLEEP_HOOK_USER_T user, HAL_DEEP_SLEEP_HOOK_HANDLER handler)
{
    if (user >= ARRAY_SIZE(deep_sleep_hook_handler)) {
        return 1;
    }
    deep_sleep_hook_handler[user] = handler;
    return 0;
}

static int SRAM_TEXT_LOC hal_sleep_exec_deep_sleep_hook(void)
{
    int i;
    int ret;

    for (i = 0; i < ARRAY_SIZE(deep_sleep_hook_handler); i++) {
        if (deep_sleep_hook_handler[i]) {
            ret = deep_sleep_hook_handler[i]();
            if (ret) {
                return ret;
            }
        }
    }

    return 0;
}

int SRAM_TEXT_LOC hal_sleep_irq_pending(void)
{
#if defined(__GIC_PRESENT) && (__GIC_PRESENT)
    int i;

    for (i = 0; i < (USER_IRQn_QTY + 31) / 32; i++) {
        if (GICDistributor->ICPENDR[i] & GICDistributor->ISENABLER[i]) {
            return 1;
        }
    }
#else
#if 0
    int i;

    for (i = 0; i < (USER_IRQn_QTY + 31) / 32; i++) {
        if (NVIC->ICPR[i] & NVIC->ISER[i]) {
            return 1;
        }
    }
#else
    // If there is any pending and enabled exception (including sysTick)
    // SCB_ICSR_VECTPENDING is not banked between security states
    if (SCB->ICSR & SCB_ICSR_VECTPENDING_Msk) {
        return 1;
    }
#endif
#endif

    return 0;
}

int SRAM_TEXT_LOC hal_sleep_specific_irq_pending(const uint32_t *irq, uint32_t cnt)
{
    int i;
    uint32_t check_cnt;

    check_cnt = (USER_IRQn_QTY + 31) / 32;
    if (check_cnt > cnt) {
        check_cnt = cnt;
    }

#if defined(__GIC_PRESENT) && (__GIC_PRESENT)
    for (i = 0; i < check_cnt; i++) {
        if (GICDistributor->ICPENDR[i] & GICDistributor->ISENABLER[i] & irq[i]) {
            return 1;
        }
    }
#else
    for (i = 0; i < check_cnt; i++) {
        if (NVIC->ICPR[i] & NVIC->ISER[i] & irq[i]) {
            return 1;
        }
#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
        if (NVIC_NS->ICPR[i] & NVIC_NS->ISER[i] & irq[i]) {
            return 1;
        }
#endif
    }
#endif

    return 0;
}

static int pm_notif_sleep(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    enum HAL_PM_STATE_T state;
    int i;

    if (pm_user_cnt == 0) {
        return 0;
    }

    if (0) {
#ifdef CORE_SLEEP_POWER_DOWN
    } else if (mode == HAL_CMU_LPU_SLEEP_MODE_POWER_DOWN) {
        state = HAL_PM_STATE_POWER_DOWN_SLEEP;
#endif
    } else {
        state = HAL_PM_STATE_NORMAL_SLEEP;
    }

    for (i = pm_user_cnt - 1; i >= 0; i--) {
        if (pm_notif[i]) {
            pm_notif[i](state);
        }
    }

    return 0;
}

static int pm_notif_wakeup(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    enum HAL_PM_STATE_T state;
    int i;

    if (pm_user_cnt == 0) {
        return 0;
    }

    if (0) {
#ifdef CORE_SLEEP_POWER_DOWN
    } else if (mode == HAL_CMU_LPU_SLEEP_MODE_POWER_DOWN) {
        state = HAL_PM_STATE_POWER_DOWN_WAKEUP;
#endif
    } else {
        state = HAL_PM_STATE_NORMAL_WAKEUP;
    }

    for (i = 0; i < pm_user_cnt; i++) {
        if (pm_notif[i]) {
            pm_notif[i](state);
        }
    }

    return 0;
}

static enum HAL_SLEEP_STATUS_T SRAM_TEXT_LOC hal_sleep_lowpower_mode(void)
{
    enum HAL_SLEEP_STATUS_T ret;
    enum HAL_CMU_LPU_SLEEP_MODE_T mode;
    uint32_t prev_time = 0;
    uint32_t cur_time;
    uint32_t interval;

    ret = HAL_SLEEP_STATUS_LIGHT;

    // Deep sleep hook
    if (hal_sleep_exec_deep_sleep_hook() || hal_trace_busy()) {
        return ret;
    }

#ifdef SLEEP_SYS_FREQ_52M
    enum HAL_CMU_FREQ_T old_min_freq;

    old_min_freq = hal_sysfreq_inc_min_freq(HAL_CMU_FREQ_52M);
#endif

#ifdef CORE_SLEEP_POWER_DOWN
    if (power_down_en) {
        if (chip_wake_lock_map) {
            mode = HAL_CMU_LPU_SLEEP_MODE_SYS_32K_CORE_PD;
        } else {
            mode = HAL_CMU_LPU_SLEEP_MODE_POWER_DOWN;
        }
    } else
#endif
    {
        if (chip_wake_lock_map) {
            mode = HAL_CMU_LPU_SLEEP_MODE_SYS;
        } else {
            mode = HAL_CMU_LPU_SLEEP_MODE_CHIP;
        }
    }

    // Stop normal modules
    pm_notif_sleep(mode);

    // analog interface is handled by NS image
#if defined(MAIN_MCU_SLEEP_CTRL) && !defined(ARM_CMSE)
    if (chip_wake_lock_map == 0) {
        analog_sleep();
        hal_gpadc_sleep();
        pmu_sleep();
    }
#endif
    // End of stopping normal modules

#ifdef CORE_SLEEP_POWER_DOWN
    // Save uart trace related cfg
    hal_dma_sleep();
    hal_uart_sleep();
    hal_cache_sleep();
#endif

    // flash and psram is handled by SE image
#ifndef ARM_CMNS
#ifdef MAIN_MCU_SLEEP_CTRL
    // Stop psram and then flash
#ifdef PSRAMUHS_ENABLE
    hal_psramuhs_sleep();
#endif
#ifdef PSRAM_ENABLE
    hal_psram_sleep();
#endif
#endif // MAIN_MCU_SLEEP_CTRL
#ifdef FLASH_SLEEP_CTRL
    hal_norflash_sleep(HAL_FLASH_ID_0);
#endif
#endif // !ARM_CMNS

#ifdef CORE_SLEEP_POWER_DOWN
    // Save ispi cfg
    hal_spi_sleep();
#endif

#ifdef FAST_TIMER_COMPENSATE
    // Save fast timer
    hal_fast_timer_sleep();
#endif

    if (!hal_sleep_irq_pending()) {
        if (stats_started) {
            prev_time = hal_sys_timer_get();
#ifdef SLEEP_STATS_TRACE
#ifdef WAKEUP_SRC_STATS
            hal_sleep_calc_wakeup_interval(prev_time);
#endif
#ifdef SYSFREQ_STATS
            hal_sysfreq_add_freq_time(false, prev_time);
#endif
#endif
        }

        hal_cmu_lpu_sleep(mode);

        if (stats_started) {
            cur_time = hal_sys_timer_get();
            interval = cur_time - prev_time;
            if (chip_wake_lock_map) {
                sys_deep_sleep_time += interval;
            } else {
                chip_deep_sleep_time += interval;
            }
#ifdef SLEEP_STATS_TRACE
#ifdef WAKEUP_SRC_STATS
            hal_sleep_save_wakeup_src();
#endif
#ifdef SYSFREQ_STATS
            hal_sysfreq_add_freq_time(true, cur_time);
#endif
#endif
        }

        ret = HAL_SLEEP_STATUS_DEEP;
    }

#ifdef FAST_TIMER_COMPENSATE
    // Restore fast timer
    hal_fast_timer_wakeup();
#endif

#ifdef CORE_SLEEP_POWER_DOWN
    // Restore ispi cfg
    hal_spi_wakeup();
#endif

    // flash and psram is handled by SE image
#ifndef ARM_CMNS
    // Restore flash and then psram
#ifdef FLASH_SLEEP_CTRL
    hal_norflash_wakeup(HAL_FLASH_ID_0);
#endif
#ifdef MAIN_MCU_SLEEP_CTRL
#ifdef PSRAM_ENABLE
    hal_psram_wakeup();
#endif
#ifdef PSRAMUHS_ENABLE
    hal_psramuhs_wakeup();
#endif
#endif // MAIN_MCU_SLEEP_CTRL
#endif // !ARM_CMNS

#ifdef CORE_SLEEP_POWER_DOWN
    hal_cache_wakeup();
    // Restore uart trace related cfg
    hal_uart_wakeup();
    hal_dma_wakeup();
#endif

    // Restore normal modules
    // analog interface is handled by NS image
#if defined(MAIN_MCU_SLEEP_CTRL) && !defined(ARM_CMSE)
    if (chip_wake_lock_map == 0) {
        pmu_wakeup();
        hal_gpadc_wakeup();
        analog_wakeup();
    }
#endif

    pm_notif_wakeup(mode);
    // End of restoring normal modules

#ifdef SLEEP_SYS_FREQ_52M
    hal_sysfreq_set_min_freq(old_min_freq);
#endif

    return ret;
}

// GCC has trouble in detecting static function usage in embedded ASM statements.
// The following function might be optimized away if there is no explicted call in C codes.
// Specifying "used" (or "noclone") attribute on the function can avoid the mistaken optimization.
static enum HAL_SLEEP_STATUS_T SRAM_TEXT_LOC NOINLINE USED hal_sleep_proc(int light_sleep)
{
    enum HAL_SLEEP_STATUS_T ret;
    uint32_t prev_time = 0;
    uint32_t cur_time;
    uint32_t interval;
    bool lpu_busy = false;
    bool dma_busy = false;
    POSSIBLY_UNUSED bool trace_busy = false;

    ret = HAL_SLEEP_STATUS_LIGHT;

    // Check the sleep conditions in interrupt-locked context
    if (cpu_wake_lock_map || (lpu_busy = hal_cmu_lpu_busy())) {
        // Cannot sleep
    } else {
        // Sleep hook
        if (hal_sleep_exec_sleep_hook()) {
            goto _exit_sleep;
        }

        if (sys_wake_lock_map || hal_sysfreq_busy() || (dma_busy = hal_dma_busy())) {
            // Light sleep

            if (stats_started) {
                prev_time = hal_sys_timer_get();
#ifdef SYSFREQ_STATS
                hal_sysfreq_add_freq_time(false, prev_time);
#endif
            }

#ifdef NO_LIGHT_SLEEP
            // WFI during USB ISO transfer might generate very weak (0.1 mV) 1K tone interference ???
            while (!hal_sleep_irq_pending());
#else
#ifndef __ARM_ARCH_ISA_ARM
            SCB->SCR = 0;
#endif
            __DSB();
            __WFI();
#endif

            if (stats_started) {
                cur_time = hal_sys_timer_get();
                light_sleep_time += cur_time - prev_time;
#ifdef SYSFREQ_STATS
                hal_sysfreq_add_freq_time(true, cur_time);
#endif
            }
#ifdef DEBUG
        } else if ((trace_busy = hal_trace_busy())) {
            // Light sleep with trace busy only

            if (stats_started) {
                prev_time = hal_sys_timer_get();
#ifdef SYSFREQ_STATS
                hal_sysfreq_add_freq_time(false, prev_time);
#endif
            }

            // No irq will be generated when trace becomes idle, so the trace status should
            // be kept polling actively instead of entering WFI
            while (!hal_sleep_irq_pending() && hal_trace_busy());

            if (stats_started) {
                cur_time = hal_sys_timer_get();
                light_sleep_time += cur_time - prev_time;
#ifdef SYSFREQ_STATS
                hal_sysfreq_add_freq_time(true, cur_time);
#endif
            }

            if (!hal_sleep_irq_pending()) {
                goto _deep_sleep;
            }
#endif
        } else {
            // Deep sleep

_deep_sleep: POSSIBLY_UNUSED;

            if (light_sleep) {
                ret = HAL_SLEEP_STATUS_DEEP;
            } else {
                ret = hal_sleep_lowpower_mode();
            }
        }
    }

_exit_sleep:
    if (stats_started) {
        cur_time = hal_sys_timer_get();
        interval = cur_time - stats_start_time;
        if (interval >= stats_interval) {
            if (light_sleep_time > UINT32_MAX / 100) {
                light_sleep_ratio = (uint64_t)light_sleep_time * 100 / interval;
            } else {
                light_sleep_ratio = light_sleep_time * 100 / interval;
            }
            if (sys_deep_sleep_time > UINT32_MAX / 100) {
                sys_deep_sleep_ratio = (uint64_t)sys_deep_sleep_time * 100 / interval;
            } else {
                sys_deep_sleep_ratio = sys_deep_sleep_time * 100 / interval;
            }
            if (chip_deep_sleep_time > UINT32_MAX / 100) {
                chip_deep_sleep_ratio = (uint64_t)chip_deep_sleep_time * 100 / interval;
            } else {
                chip_deep_sleep_ratio = chip_deep_sleep_time * 100 / interval;
            }
            stats_valid = true;
            light_sleep_time = 0;
            sys_deep_sleep_time = 0;
            chip_deep_sleep_time = 0;
            stats_start_time = cur_time;

            system_sleep_stat_update(light_sleep_ratio, sys_deep_sleep_ratio, chip_deep_sleep_ratio);
        }
#ifdef SLEEP_STATS_TRACE
        if (stats_valid && stats_trace_interval) {
            if (cur_time - stats_trace_time >= stats_trace_interval) {
#ifdef DEBUG_SLEEP_USER
                hal_dma_record_busy_chan();
#endif
                TRACE(0, "CPU USAGE: busy=%d light=%d sys_deep=%d chip_deep=%d",
                    100 - (light_sleep_ratio + sys_deep_sleep_ratio + chip_deep_sleep_ratio),
                    light_sleep_ratio, sys_deep_sleep_ratio, chip_deep_sleep_ratio);
                stats_trace_time = cur_time;
#ifdef DEBUG_SLEEP_USER
                TRACE(0, "SLEEP USER: cpu=0x%X sys=0x%X chip=0x%X irq=0x%08X_%08X",
                    cpu_wake_lock_map, sys_wake_lock_map, chip_wake_lock_map,
                    (NVIC->ICPR[1] & NVIC->ISER[1]), (NVIC->ICPR[0] & NVIC->ISER[0]));
                TRACE(0, "BUSY: LPU=%d DMA=%d TRACE=%d", lpu_busy, dma_busy, trace_busy);
                hal_sysfreq_print_user_freq();
                hal_dma_print_busy_chan();
#endif
#ifdef WAKEUP_SRC_STATS
                hal_sleep_print_wakeup_src();
#endif
#ifdef CACHE_STATS
                hal_cache_print_stats();
#endif
#ifdef SYSFREQ_STATS
                hal_sysfreq_print_freq_stats();
#endif
            }
        }
#endif
    }

    return ret;
}

#ifndef __ARM_ARCH_ISA_ARM
static enum HAL_SLEEP_STATUS_T SRAM_TEXT_LOC NOINLINE USED NAKED hal_sleep_deep_sleep_wrapper(void)
{
    asm volatile(
        "push {r4, lr} \n"
        // Switch current stack pointer to MSP
        "mrs r4, control \n"
        "bic r4, #2 \n"
        "msr control, r4 \n"
        "isb \n"
        "movs r0, #0 \n"
        "bl hal_sleep_proc \n"
        // Switch current stack pointer back to PSP
        "orr r4, #2 \n"
        "msr control, r4 \n"
        "isb \n"
        "pop {r4, pc} \n"
        );

#ifndef __ARMCC_VERSION
    return HAL_SLEEP_STATUS_LIGHT;
#endif
}
#endif

enum HAL_SLEEP_STATUS_T SRAM_TEXT_LOC hal_sleep_enter_sleep(void)
{
    enum HAL_SLEEP_STATUS_T ret;
    uint32_t lock;

    ret = HAL_SLEEP_STATUS_LIGHT;

#ifdef NO_SLEEP
    return ret;
#endif

    lock = int_lock_global();

#ifndef __ARM_ARCH_ISA_ARM
    if (__get_CONTROL() & 0x02) {
        ret = hal_sleep_deep_sleep_wrapper();
    } else
#endif
    {
        ret = hal_sleep_proc(false);
    }

    int_unlock_global(lock);

    return ret;
}

enum HAL_SLEEP_STATUS_T SRAM_TEXT_LOC hal_sleep_light_sleep(void)
{
    enum HAL_SLEEP_STATUS_T ret;
    uint32_t lock;

    ret = HAL_SLEEP_STATUS_LIGHT;

#ifdef NO_SLEEP
    return ret;
#endif

    lock = int_lock_global();

    ret = hal_sleep_proc(true);

    int_unlock_global(lock);

    return ret;
}

int hal_cpu_wake_lock(enum HAL_CPU_WAKE_LOCK_USER_T user)
{
    uint32_t lock;

    if (user >= HAL_CPU_WAKE_LOCK_USER_QTY) {
        return 1;
    }

    lock = int_lock();
    cpu_wake_lock_map |= (1 << user);
    int_unlock(lock);

    return 0;
}

int hal_cpu_wake_unlock(enum HAL_CPU_WAKE_LOCK_USER_T user)
{
    uint32_t lock;

    if (user >= HAL_CPU_WAKE_LOCK_USER_QTY) {
        return 1;
    }

    lock = int_lock();
    cpu_wake_lock_map &= ~(1 << user);
    int_unlock(lock);

    return 0;
}

uint32_t hal_cpu_wake_lock_get()
{
    return cpu_wake_lock_map;
}

int hal_sys_wake_lock(enum HAL_SYS_WAKE_LOCK_USER_T user)
{
    uint32_t lock;

    if (user >= HAL_SYS_WAKE_LOCK_USER_QTY) {
        return 1;
    }

    lock = int_lock();
    sys_wake_lock_map |= (1 << user);
    int_unlock(lock);

    return 0;
}

int hal_sys_wake_unlock(enum HAL_SYS_WAKE_LOCK_USER_T user)
{
    uint32_t lock;

    if (user >= HAL_SYS_WAKE_LOCK_USER_QTY) {
        return 1;
    }

    lock = int_lock();
    sys_wake_lock_map &= ~(1 << user);
    int_unlock(lock);

    return 0;
}

uint32_t hal_sys_wake_lock_get()
{
    return sys_wake_lock_map;
}

#if defined(__NuttX__)
int hal_bus_wake_lock(enum HAL_SYS_WAKE_LOCK_USER_T user)
{
    return hal_sys_wake_lock(user);
}

int hal_bus_wake_unlock(enum HAL_SYS_WAKE_LOCK_USER_T user)
{
    return hal_sys_wake_unlock(user);
}

uint32_t hal_bus_wake_lock_get()
{
    return hal_sys_wake_lock_get();
}
#endif

int hal_chip_wake_lock(enum HAL_CHIP_WAKE_LOCK_USER_T user)
{
    uint32_t lock;

    if (user >= HAL_CHIP_WAKE_LOCK_USER_QTY) {
        return 1;
    }

    lock = int_lock();
    chip_wake_lock_map |= (1 << user);
    int_unlock(lock);

    return 0;
}

int hal_chip_wake_unlock(enum HAL_CHIP_WAKE_LOCK_USER_T user)
{
    uint32_t lock;

    if (user >= HAL_CHIP_WAKE_LOCK_USER_QTY) {
        return 1;
    }

    lock = int_lock();
    chip_wake_lock_map &= ~(1 << user);
    int_unlock(lock);

    return 0;
}

uint32_t hal_chip_wake_lock_get()
{
    return chip_wake_lock_map;
}

void hal_sleep_power_down_enable(void)
{
#ifdef CORE_SLEEP_POWER_DOWN
    power_down_en = true;
#ifdef ARM_CMNS
    TZ_hal_sleep_power_down_enable_S();
#endif
#endif
}

void hal_sleep_power_down_disable(void)
{
#ifdef CORE_SLEEP_POWER_DOWN
    power_down_en = false;
#ifdef ARM_CMNS
    TZ_hal_sleep_power_down_disable_S();
#endif
#endif
}

int hal_sleep_power_down_enabled(void)
{
#ifdef CORE_SLEEP_POWER_DOWN
    return power_down_en;
#else
    return false;
#endif
}

#ifdef ARM_CMSE
#ifdef CORE_SLEEP_POWER_DOWN
__attribute__((cmse_nonsecure_entry))
void TZ_hal_sleep_power_down_enable_S(void)
{
    hal_sleep_power_down_enable();
}

__attribute__((cmse_nonsecure_entry))
void TZ_hal_sleep_power_down_disable_S(void)
{
    hal_sleep_power_down_disable();
}
#endif

__attribute__((cmse_nonsecure_entry))
int TZ_hal_sleep_deep_sleep_S(void)
{
    enum HAL_SLEEP_STATUS_T ret;

    ret = hal_sleep_enter_sleep();
    return ret;
}
#endif

int hal_pm_notif_register(enum HAL_PM_USER_TYPE_T user, HAL_PM_NOTIF_HANDLER handler)
{
    int i;
    uint32_t lock;
    int ret;

    if (handler == NULL) {
        ASSERT(false, "%s: handler cannot be NULL: user=%d", __func__, user);
        return 1;
    }

    ret = 0;

    lock = int_lock();

    if (pm_user_cnt >= PM_NOTIF_HDLR_CNT) {
        ASSERT(false, "%s: handler list full: user=%d handler=%p curCnt=%u", __func__, user, handler, pm_user_cnt);
        ret = 2;
        goto _exit;
    }

    for (i = 0; i < pm_user_cnt; i++) {
        if (pm_notif[i] == handler) {
            ASSERT(false, "%s: handler already registered: user=%d handler=%p existedUser=%d", __func__, user, handler, pm_user[i]);
            ret = 3;
            goto _exit;
        }
    }

    if (pm_user_cnt == 0) {
        pm_user[i] = user;
        pm_notif[i] = handler;
    } else {
        for (i = pm_user_cnt; i >= 0; i--) {
            if (i == 0 || pm_user[i - 1] > user) {
                pm_user[i] = user;
                pm_notif[i] = handler;
                break;
            } else {
                pm_user[i] = pm_user[i - 1];
                pm_notif[i] = pm_notif[i - 1];
            }
        }
    }
    pm_user_cnt++;

_exit:
    int_unlock(lock);

    return ret;
}

int hal_pm_notif_deregister(enum HAL_PM_USER_TYPE_T user, HAL_PM_NOTIF_HANDLER handler)
{
    int i;
    uint32_t lock;
    int ret;

    if (handler == NULL) {
        return 1;
    }

    ret = 0;

    lock = int_lock();

    for (i = 0; i < pm_user_cnt; i++) {
        if (pm_notif[i] == handler) {
            break;
        }
    }

    if (i >= pm_user_cnt) {
        ret = 2;
        goto _exit;
    }

    for (; (i + 1) < pm_user_cnt; i++) {
        pm_user[i] = pm_user[i + 1];
        pm_notif[i] = pm_notif[i + 1];
    }
    pm_user_cnt--;

_exit:
    int_unlock(lock);

    return ret;
}

WEAK SYSTEM_SLEEP_STAT_T* system_sleep_stat_get(void)
{
    return &system_sleep_stat_info;
}

WEAK void system_sleep_stat_update(uint8_t light, uint8_t sys_deep, uint8_t chip_deep)
{
    system_sleep_stat_info.light_sleep_ratio = light;
    system_sleep_stat_info.sys_deep_sleep_ratio = sys_deep;
    system_sleep_stat_info.chip_deep_sleep_ratio = chip_deep;
}
