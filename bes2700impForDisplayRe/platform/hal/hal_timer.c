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
#include "plat_addr_map.h"
#ifdef CMU_FREE_TIMER
#include CHIP_SPECIFIC_HDR(reg_aoncmu)
#endif
#include "cmsis_nvic.h"
#include "dwt.h"
#include "hal_cmu.h"
#include "hal_location.h"
#include "hal_sleep.h"
#define IGNORE_HAL_TIMER_RAW_API_CHECK
#include "hal_timer_raw.h"
#include "hal_timer_fast_irq.h"
#include "hal_timer_user_irq.h"
#include "hal_timer.h"
#include "reg_timer.h"
#include "hal_trace.h"

#if defined(CHIP_BEST3001) || defined(CHIP_BEST3003) || defined(CHIP_BEST3005) || defined(CHIP_BEST1400) || defined(CHIP_BEST1402)
#define CLOCK_SYNC_WORKAROUND
#endif

#if defined(TIMER1_BASE) && defined(LOW_SYS_FREQ) && defined(LOW_SYS_FREQ_6P5M)
#if defined(CHIP_BEST1305) || defined(CHIP_BEST1501) || \
        defined(CHIP_BEST2001) || defined(CHIP_BEST2003) || \
        defined(CHIP_BEST2300A) || defined(CHIP_BEST2300P)
#define FAST_TIMER_WORKAROUND
#endif
#endif

#if !(defined(__FPU_USED) && (__FPU_USED == 1))
#undef TIMER_USE_FPU
#endif

#ifndef SLOW_FREE_TIMER_ID
#define SLOW_FREE_TIMER_ID          0
#endif

#ifndef SLOW_TIMER_SUB_ID_WITH_IRQ
#define SLOW_TIMER_SUB_ID_WITH_IRQ  1
#endif

#define SLOW_TIMER_VAL_DELTA        1
#define SLOW_TIMER_VAL_DELTA_SLEEP  10
#define FAST_TIMER_VAL_DELTA        20

#ifdef CALIB_SLOW_TIMER
#define MAX_CALIB_SYSTICK_HZ        (CONFIG_SYSTICK_HZ_NOMINAL * 2)

#define MIN_CALIB_TICKS             (30 * (CONFIG_SYSTICK_HZ_NOMINAL / 1000))

#define MAX_CALIB_TICKS             (30 * CONFIG_SYSTICK_HZ_NOMINAL)

static
#ifdef TIMER_USE_FPU
    float
#else
    uint32_t
#endif
    BOOT_DATA_LOC sys_tick_hz = CONFIG_SYSTICK_HZ_NOMINAL;
static uint32_t BOOT_BSS_LOC slow_val;
static uint32_t BOOT_BSS_LOC fast_val;
#endif

#ifdef FAST_TIMER_COMPENSATE
#ifndef TIMER1_BASE
#error "FAST_TIMER_COMPENSATE need timer1"
#endif
#if !(defined(__FPU_USED) && (__FPU_USED == 1))
#error "FAST_TIMER_COMPENSATE need FPU"
#endif
#ifdef FAST_TIMER_WORKAROUND
#error "FAST_TIMER_COMPENSATE conflicts with FAST_TIMER_WORKAROUND"
#endif
#define FAST_TICK_RATIO_NUM                 3
static float BOOT_BSS_LOC fast_tick_ratio_avg;
static float BOOT_BSS_LOC fast_tick_ratio[FAST_TICK_RATIO_NUM];
#endif

static struct DUAL_TIMER_T * const BOOT_RODATA_SRAM_LOC dual_timer[] = {
    (struct DUAL_TIMER_T *)TIMER0_BASE,
#ifdef TIMER1_BASE
    (struct DUAL_TIMER_T *)TIMER1_BASE,
#endif
#ifdef TIMER2_BASE
    (struct DUAL_TIMER_T *)TIMER2_BASE,
#endif
};

#ifdef CMU_FREE_TIMER
#ifndef TIMER1_BASE
#error "CMU_FREE_TIMER should work with TIMER1_BASE"
#endif
#ifdef CLOCK_SYNC_WORKAROUND
#error "CMU_FREE_TIMER conflicts with CLOCK_SYNC_WORKAROUND"
#endif
#ifdef FAST_TIMER_WORKAROUND
#error "CMU_FREE_TIMER conflicts with FAST_TIMER_WORKAROUND"
#endif
static struct AONCMU_T * const aoncmu = (struct AONCMU_T *)AON_CMU_BASE;
#endif

static IRQn_Type irq_num[ARRAY_SIZE(dual_timer)][2] = {
    { TIMER00_IRQn, TIMER01_IRQn, },
#ifdef TIMER1_BASE
    { INVALID_IRQn, TIMER11_IRQn, },
#endif
#ifdef TIMER2_BASE
#if defined(ASIC_SIMU)
    { INVALID_IRQn, INVALID_IRQn, },
#else
    { TIMER20_IRQn, TIMER21_IRQn, },
#endif
#endif
};

static HAL_TIMER_IRQ_HANDLER_T irq_handler[ARRAY_SIZE(dual_timer)][2];

static uint32_t start_time[ARRAY_SIZE(dual_timer)][2];

static uint32_t hal_timer_common_get_elapsed_time(uint32_t id, uint32_t sub_id);
static int hal_timer_common_irq_pending(uint32_t id, uint32_t sub_id);

__STATIC_FORCEINLINE uint32_t get_timer_value(struct TIMER_T *timer, uint32_t delta)
{
#ifdef CLOCK_SYNC_WORKAROUND
    uint32_t lock;
    uint32_t v1, v2;

    lock = int_lock();
    do {
        v1 = timer->Value;
        v2 = timer->Value;
    } while ((v1 < v2) || (v1 > v2 + delta));
    int_unlock(lock);

    return v2;
#else
    return timer->Value;
#endif
}

__STATIC_FORCEINLINE void clear_timer_irq(struct TIMER_T *timer)
{
#ifdef CLOCK_SYNC_WORKAROUND
    do {
        timer->IntClr = 1;
    } while (timer->RIS & TIMER_RIS_RIS);
#else
    timer->IntClr = 1;
#endif
}

__STATIC_FORCEINLINE void set_timer_load(struct TIMER_T *timer, uint32_t load, uint32_t delta)
{
#ifdef CLOCK_SYNC_WORKAROUND
    uint32_t lock;
    uint32_t val;

    lock = int_lock();
    do {
        timer->Load = load;
        val = timer->Value;
    } while ((load < val) || (load > val + delta));
    int_unlock(lock);
#else
    timer->Load = load;
#endif
}

__STATIC_FORCEINLINE void fast_sys_timer_open(void)
{
#ifdef CMU_FREE_TIMER
    hal_cmu_fast_timer_init();
#endif
#ifdef TIMER1_BASE
    hal_cmu_timer1_select_fast();
    dual_timer[1]->timer[0].Control = TIMER_CTRL_EN | TIMER_CTRL_PRESCALE_DIV_1 | TIMER_CTRL_SIZE_32_BIT;
#endif
#ifdef TIMER2_BASE
    hal_cmu_timer2_select_fast();
#endif
}

void BOOT_TEXT_FLASH_LOC hal_sys_timer_open(void)
{
#ifndef CMU_FREE_TIMER
    hal_cmu_timer0_select_slow();
    dual_timer[SLOW_FREE_TIMER_ID]->timer[0].Control = TIMER_CTRL_EN | TIMER_CTRL_PRESCALE_DIV_1 | TIMER_CTRL_SIZE_32_BIT;
#endif
    fast_sys_timer_open();
#if defined(ASIC_SIMU)
#if defined(TIMER2_BASE)
    dual_timer[2]->timer[0].Control = TIMER_CTRL_EN | TIMER_CTRL_PRESCALE_DIV_1 | TIMER_CTRL_SIZE_32_BIT;
#endif
#endif
}

#ifdef CORE_SLEEP_POWER_DOWN
void SRAM_TEXT_LOC hal_sys_timer_wakeup(void)
{
    fast_sys_timer_open();
}
#endif

uint32_t BOOT_TEXT_SRAM_LOC hal_sys_timer_get(void)
{
#ifdef CMU_FREE_TIMER
    return aoncmu->FREE_TIMER;
#else
    return -get_timer_value(&dual_timer[SLOW_FREE_TIMER_ID]->timer[0], SLOW_TIMER_VAL_DELTA);
#endif
}

uint32_t BOOT_TEXT_SRAM_LOC hal_aco_timer_get(void)
{
#if defined(TIMER2_BASE)
    return -get_timer_value(&dual_timer[2]->timer[0], SLOW_TIMER_VAL_DELTA);
#else
    return hal_sys_timer_get();
#endif
}

#ifdef CLOCK_SYNC_WORKAROUND
uint32_t SRAM_TEXT_LOC hal_sys_timer_get_in_sleep(void)
{
    return -get_timer_value(&dual_timer[SLOW_FREE_TIMER_ID]->timer[0], SLOW_TIMER_VAL_DELTA_SLEEP);
}
#else
uint32_t hal_sys_timer_get_in_sleep(void) __attribute__((alias("hal_sys_timer_get")));
#endif

uint32_t BOOT_TEXT_FLASH_LOC flash_hal_sys_timer_get(void)
{
#ifdef CMU_FREE_TIMER
    return aoncmu->FREE_TIMER;
#else
    return -get_timer_value(&dual_timer[SLOW_FREE_TIMER_ID]->timer[0], SLOW_TIMER_VAL_DELTA);
#endif
}

uint32_t BOOT_TEXT_SRAM_LOC hal_sys_ms_get(void)
{
    return GET_CURRENT_MS();
}

#ifdef TIMER1_BASE
__STATIC_FORCEINLINE uint32_t BOOT_TEXT_SRAM_LOC _real_fast_sys_timer_get(void)
{
    return -get_timer_value(&dual_timer[1]->timer[0], FAST_TIMER_VAL_DELTA);
}
#endif

uint32_t BOOT_TEXT_SRAM_LOC hal_fast_sys_timer_get(void)
{
#ifdef FAST_TIMER_WORKAROUND
#ifdef TIMER_USE_FPU
    return (uint32_t)(hal_sys_timer_get() * ((float)CONFIG_FAST_SYSTICK_HZ / CONFIG_SYSTICK_HZ));
#else
    return (uint32_t)(hal_sys_timer_get() * (uint64_t)CONFIG_FAST_SYSTICK_HZ / CONFIG_SYSTICK_HZ);
#endif
#elif defined(TIMER1_BASE)
    return _real_fast_sys_timer_get();
#else
    return 0;
#endif
}

uint32_t hal_sys_timer_get_max(void)
{
    return 0xFFFFFFFF;
}

void BOOT_TEXT_SRAM_LOC hal_sys_timer_delay(uint32_t ticks)
{
    uint32_t start = hal_sys_timer_get();

    while (hal_sys_timer_get() - start < ticks);
}

#ifdef CLOCK_SYNC_WORKAROUND
void SRAM_TEXT_LOC hal_sys_timer_delay_in_sleep(uint32_t ticks)
{
    uint32_t start = hal_sys_timer_get_in_sleep();

    while (hal_sys_timer_get_in_sleep() - start < ticks);
}
#else
void hal_sys_timer_delay_in_sleep(uint32_t ticks) __attribute__((alias("hal_sys_timer_delay")));
#endif

void BOOT_TEXT_FLASH_LOC flash_hal_sys_timer_delay(uint32_t ticks)
{
    uint32_t start = flash_hal_sys_timer_get();

    while (flash_hal_sys_timer_get() - start < ticks);
}

void BOOT_TEXT_SRAM_LOC hal_sys_timer_delay_us(uint32_t us)
{
#if defined(TIMER1_BASE) && !defined(FAST_TIMER_WORKAROUND)
    uint32_t start = hal_fast_sys_timer_get();
    uint32_t ticks = US_TO_FAST_TICKS(us);

    while (hal_fast_sys_timer_get() - start < ticks);
#else
    enum HAL_CMU_FREQ_T freq = hal_cmu_sys_get_freq();
    uint32_t loop;
    uint32_t i;

    // Assuming:
    // 1) system clock uses audio PLL
    // 2) audio PLL is configured as 48K series, 196.608M
    // 3) crystal is 26M

    if (freq == HAL_CMU_FREQ_208M) {
        loop = 197;
    } else if (freq == HAL_CMU_FREQ_104M) {
        loop = 197 / 2;
    } else if (freq == HAL_CMU_FREQ_78M) {
        loop = 197 / 3;
    } else if (freq == HAL_CMU_FREQ_52M) {
        loop = 52;
    } else {
        loop = 26;
    }

    loop = loop * us / 5;
    for (i = 0; i < loop; i++) {
        asm volatile("nop");
    }
#endif
}

void SRAM_TEXT_LOC hal_sys_timer_delay_ns(uint32_t ns)
{
#if defined(TIMER1_BASE) && !defined(FAST_TIMER_WORKAROUND)
    uint32_t start = hal_fast_sys_timer_get();
    uint32_t ticks = NS_TO_FAST_TICKS(ns);

    while (hal_fast_sys_timer_get() - start < ticks);
#else
    enum HAL_CMU_FREQ_T freq = hal_cmu_sys_get_freq();
    uint32_t loop;
    uint32_t i;

    // Assuming:
    // 1) system clock uses audio PLL
    // 2) audio PLL is configured as 48K series, 196.608M
    // 3) crystal is 26M

    if (freq == HAL_CMU_FREQ_208M) {
        loop = 197;
    } else if (freq == HAL_CMU_FREQ_104M) {
        loop = 197 / 2;
    } else if (freq == HAL_CMU_FREQ_78M) {
        loop = 197 / 3;
    } else if (freq == HAL_CMU_FREQ_52M) {
        loop = 52;
    } else {
        loop = 26;
    }

    loop = loop * ns / 5000;
    for (i = 0; i < loop; i++) {
        asm volatile("nop");
    }
#endif
}

static uint32_t NOINLINE SRAM_TEXT_DEF(measure_cpu_freq_interval)(uint32_t cnt, uint32_t *read_cycles_p)
{
    uint32_t old_start;
    uint32_t start;
    uint32_t end;
    struct DUAL_TIMER_T *t;
    uint32_t delta;

#ifdef TIMER1_BASE
    t = dual_timer[1];
    delta = FAST_TIMER_VAL_DELTA;
#ifdef FAST_TIMER_WORKAROUND
    if (hal_cmu_fast_timer_offline()) {
        t = dual_timer[SLOW_FREE_TIMER_ID];
        delta = SLOW_TIMER_VAL_DELTA;
    }
#endif // FAST_TIMER_WORKAROUND
#else
    t = dual_timer[SLOW_FREE_TIMER_ID];
    delta = SLOW_TIMER_VAL_DELTA;
#endif

    if (read_cycles_p) {
        int ret;

        ret = dwt_enable();
        dwt_reset_cycle_cnt();
        get_timer_value(&t->timer[0], delta);
        *read_cycles_p = dwt_read_cycle_cnt();
        if (ret == 0) {
            dwt_disable();
        }
    }

    old_start = get_timer_value(&t->timer[0], delta);
    while ((start = get_timer_value(&t->timer[0], delta)) == old_start);

    asm volatile(
        "_loop:\n"
#ifdef __ARM_ARCH_ISA_ARM
        "nop\n"
        "nop\n"
#endif
        "subs %0, #1 \n"
        "cmp %0, #0\n"
        "bne _loop\n"
        : : "r"(cnt));

    end = get_timer_value(&t->timer[0], delta);

    return (start - end);
}

uint32_t hal_sys_timer_calc_cpu_freq(uint32_t osc_intvl_ms, int high_res)
{
    uint32_t ref_freq;
    uint32_t cnt;
    uint32_t one_sec;
    uint32_t lock;
    uint32_t run_interval;
    uint32_t base_interval;
    uint32_t timer_read_cycles = 0;
    uint32_t freq;

    // Default measurement interval
    if (osc_intvl_ms == 0) {
#ifdef TIMER1_BASE
        osc_intvl_ms = 10;
#else
        osc_intvl_ms = 100;
#endif
    }

    ref_freq = hal_cmu_get_crystal_freq();
    // CPU loop cycle count
    cnt = ref_freq / 4 * osc_intvl_ms / 1000;

    // Timer ticks per second
#ifdef TIMER1_BASE
    one_sec = CONFIG_FAST_SYSTICK_HZ;
#ifdef FAST_TIMER_WORKAROUND
    if (hal_cmu_fast_timer_offline()) {
        one_sec = CONFIG_SYSTICK_HZ;
    }
#endif // FAST_TIMER_WORKAROUND
#else
    if (high_res) {
        one_sec = CONFIG_FAST_SYSTICK_HZ;
    } else {
        one_sec = CONFIG_SYSTICK_HZ;
    }
#endif
    // Timer ticks per measurement interval
    base_interval = one_sec * osc_intvl_ms / 1000;

    lock = int_lock();

#ifndef TIMER1_BASE
    if (high_res) {
        hal_cmu_timer0_select_fast();
    }
#endif

    // CAUTION:
    // Simultaneous memory access (e.g., trace DMA) might block CPU instruction or data access,
    // and make the run interval longer than expected.
    run_interval = measure_cpu_freq_interval(cnt, high_res ? &timer_read_cycles : NULL);

#ifndef TIMER1_BASE
    if (high_res) {
        hal_cmu_timer0_select_slow();
    }
#endif

    int_unlock(lock);

    //TRACE(0, "calc_cpu_freq: base=%u run=%u read=%u", base_interval, run_interval, timer_read_cycles);

#ifdef TIMER_USE_FPU
    freq = (uint32_t)((float)ref_freq / run_interval * base_interval);
#else
    freq = (uint32_t)((uint64_t)ref_freq * base_interval / run_interval);
#endif

    if (high_res && timer_read_cycles) {
        //TRACE(0, "calc_cpu_freq: freq1=%u", freq);
        base_interval *= 100;
        run_interval = (100 * run_interval) - (100 * timer_read_cycles) * (uint64_t)one_sec / freq;
#ifdef TIMER_USE_FPU
        freq = (uint32_t)((float)ref_freq / run_interval * base_interval);
#else
        freq = (uint32_t)((uint64_t)ref_freq * base_interval / run_interval);
#endif
        //TRACE(0, "calc_cpu_freq: freq2=%u", freq);
    }

    if (high_res == 0) {
        freq = (freq + 500000) / 1000000 * 1000000;
    }

    // (1) Deviation = 1 / run_interval
    // (2) Deviation = 1_tick_time / ((osc_intvl_ms / 1000) / (freq / osc_freq)))
    return freq;
}

#ifdef CALIB_SLOW_TIMER
void BOOT_TEXT_SRAM_LOC hal_sys_timer_calib_start(void)
{
    uint32_t lock;
    uint32_t slow;
    uint32_t fast;

    lock = int_lock();
#ifdef FAST_TIMER_WORKAROUND
    ASSERT(!hal_cmu_fast_timer_offline(), "%s: Cannot calib timer while fast timer offline", __func__);
#endif
    slow = hal_sys_timer_get();
    while (hal_sys_timer_get() == slow);
    __ISB();
    fast = _real_fast_sys_timer_get();
    int_unlock(lock);

    slow_val = slow + 1;
    fast_val = fast;
}

int BOOT_TEXT_SRAM_LOC hal_sys_timer_calib_end(void)
{
    uint32_t lock;
    uint32_t slow;
    uint32_t fast;
    uint32_t slow_diff;

    lock = int_lock();
#ifdef FAST_TIMER_WORKAROUND
    ASSERT(!hal_cmu_fast_timer_offline(), "%s: Cannot calib timer while fast timer offline", __func__);
#endif
    slow = hal_sys_timer_get();
    while (hal_sys_timer_get() == slow);
    __ISB();
    fast = _real_fast_sys_timer_get();
    int_unlock(lock);

    slow += 1;
    slow_diff = slow - slow_val;

    // Avoid computation error
    if (slow_diff < MIN_CALIB_TICKS) {
        return 1;
    }
    // Avoid fast tick overflow
    if (slow_diff > MAX_CALIB_TICKS) {
        return 2;
    }

#ifdef TIMER_USE_FPU
    sys_tick_hz = (float)CONFIG_FAST_SYSTICK_HZ / (fast - fast_val) * slow_diff;
#ifdef FAST_TIMER_COMPENSATE
    uint32_t i;
    for (i = FAST_TICK_RATIO_NUM-1; i >= 1; --i)
       fast_tick_ratio[i] = fast_tick_ratio[i-1];
    fast_tick_ratio[0] = (float)(fast - fast_val) / (float)slow_diff;
    fast_tick_ratio_avg = 0;
    for (i = 0; i < FAST_TICK_RATIO_NUM; ++i) {
        if (fast_tick_ratio[i] < 100.0)
            break;
        fast_tick_ratio_avg += fast_tick_ratio[i];
    }
    fast_tick_ratio_avg = fast_tick_ratio_avg / i;
#endif /*FAST_TIMER_COMPENSATE*/
#else
    uint64_t mul;
    uint32_t fast_diff;

    fast_diff = fast - fast_val;
    mul = (uint64_t)CONFIG_FAST_SYSTICK_HZ * slow_diff + fast_diff / 2;
    if ((mul >> 32) == 0) {
        sys_tick_hz = (uint32_t)mul / fast_diff;
    } else {
        sys_tick_hz = mul / fast_diff;
    }
#endif

    if (sys_tick_hz > MAX_CALIB_SYSTICK_HZ) {
        sys_tick_hz = MAX_CALIB_SYSTICK_HZ;
    }

    return 0;
}

void BOOT_TEXT_SRAM_LOC hal_sys_timer_calib(void)
{
    hal_sys_timer_calib_start();
    hal_sys_timer_delay(MIN_CALIB_TICKS);
    hal_sys_timer_calib_end();
}

uint32_t BOOT_TEXT_SRAM_LOC hal_sys_timer_systick_hz(void)
{
#ifdef TIMER_USE_FPU
    return (uint32_t)(sys_tick_hz + 0.5);
#else
    return sys_tick_hz;
#endif
}

float BOOT_TEXT_SRAM_LOC hal_sys_timer_systick_hz_float(void)
{
    return sys_tick_hz;
}

uint32_t BOOT_TEXT_SRAM_LOC hal_sys_timer_ms_to_ticks(uint32_t ms)
{
#ifdef TIMER_USE_FPU
    return (uint32_t)((float)ms / 1000 * sys_tick_hz + 0.99);
#else
    if (ms <= (~0UL / MAX_CALIB_SYSTICK_HZ)) {
        return ((ms * sys_tick_hz + 1000 - 1) / 1000);
    } else {
        return (((uint64_t)ms * sys_tick_hz + 1000 - 1) / 1000);
    }
#endif
}

uint32_t BOOT_TEXT_SRAM_LOC hal_sys_timer_us_to_ticks(uint32_t us)
{
    uint32_t ticks;

#ifdef TIMER_USE_FPU
    ticks = (uint32_t)((float)us / (1000 * 1000) * sys_tick_hz + 0.99);
#else
    if (us <= (~0UL / MAX_CALIB_SYSTICK_HZ)) {
        ticks = ((us * sys_tick_hz / 1000 + 1000 - 1) / 1000);
    } else {
        ticks = (((uint64_t)us * sys_tick_hz / 1000 + 1000 - 1) / 1000);
    }
#endif

    if (ticks <= 1) {
        ticks += 1;
    }
    return ticks;
}

uint32_t BOOT_TEXT_SRAM_LOC hal_sys_timer_ticks_to_ms(uint32_t tick)
{
#ifdef TIMER_USE_FPU
    return (uint32_t)((float)tick / sys_tick_hz * 1000 + 0.5);
#else
    uint32_t hz = sys_tick_hz;
    if (tick <= ((~0UL - MAX_CALIB_SYSTICK_HZ / 2) / 1000)) {
        return (tick * 1000 + hz / 2) / hz;
    } else {
        return ((uint64_t)tick * 1000 + hz / 2) / hz;
    }
#endif
}

uint32_t BOOT_TEXT_SRAM_LOC hal_sys_timer_ticks_to_us(uint32_t tick)
{
#ifdef TIMER_USE_FPU
    return (uint32_t)((float)tick / sys_tick_hz * (1000 * 1000) + 0.5);
#else
    uint32_t hz = sys_tick_hz;
    // Simple round down is enough for us
    if (tick <= (~0UL / (1000 * 1000))) {
        return (tick * (1000 * 1000)) / hz;
    } else {
        return ((uint64_t)tick * (1000 * 1000)) / hz;
    }
#endif
}
#endif

#ifdef FAST_TIMER_COMPENSATE
static uint32_t BOOT_BSS_LOC slow_timer_begin = 0;
static uint32_t BOOT_BSS_LOC fast_timer_begin = 0;

void BOOT_TEXT_SRAM_LOC hal_fast_timer_sleep()
{
    uint32_t slow, fast;

    slow = hal_sys_timer_get();
    while (hal_sys_timer_get() == slow);
    __ISB();
    fast = -_real_fast_sys_timer_get();
    fast_timer_begin = fast;
    slow_timer_begin = slow + 1;
}

void BOOT_TEXT_SRAM_LOC hal_fast_timer_wakeup()
{
    uint32_t slow, fast;
    uint32_t load;
    float sleep_tick;

    slow = hal_sys_timer_get();
    while (hal_sys_timer_get() == slow);
    __ISB();
    fast = _real_fast_sys_timer_get();
    __ISB();
    sleep_tick = fast_tick_ratio_avg * (float)(slow + 1 - slow_timer_begin) + 0.9;
    load = fast_timer_begin - (uint32_t)sleep_tick;
    __ISB();
    load -= _real_fast_sys_timer_get() - fast;//float compute time
    set_timer_load(&dual_timer[1]->timer[0], load, FAST_TIMER_VAL_DELTA);
}
#endif

#ifndef RTOS
int osDelay(uint32_t ms)
{
    hal_sys_timer_delay(MS_TO_TICKS(ms));
    return 0;
}
#endif

//==========================================================
// Timer IRQ APIs
//==========================================================

__STATIC_FORCEINLINE int timer_is_slow(uint32_t id)
{
    return (id == 0);
}

static void hal_timer_common_irq_handler(uint32_t id, uint32_t sub_id)
{
    uint32_t elapsed;

    clear_timer_irq(&dual_timer[id]->timer[sub_id]);
    if (irq_handler[id][sub_id]) {
        elapsed = hal_timer_common_get_elapsed_time(id, sub_id);
        irq_handler[id][sub_id](elapsed);
    } else {
        dual_timer[id]->timer[sub_id].Control &= ~TIMER_CTRL_INTEN;
    }
}

static void hal_timer_slow_irq_handler(void)
{
    hal_timer_common_irq_handler(0, SLOW_TIMER_SUB_ID_WITH_IRQ);
}

#ifdef TIMER1_BASE
static void hal_timer11_irq_handler(void)
{
    hal_timer_common_irq_handler(1, 1);
}
#endif

#ifdef TIMER2_BASE
static void hal_timer2x_irq_handler(void)
{
    uint32_t sub_id;

    // NOTE: Some chips might combine time20 and time21 IRQ into one interrupt line
    for (sub_id = 0; sub_id < 2; sub_id++) {
        if (hal_timer_common_irq_pending(2, sub_id)) {
            hal_timer_common_irq_handler(2, sub_id);
        }
    }
}
#endif

static const uint32_t irq_entry[ARRAY_SIZE(dual_timer)][2] = {
    { (uint32_t)hal_timer_slow_irq_handler, (uint32_t)hal_timer_slow_irq_handler, },
#ifdef TIMER1_BASE
    { 0, (uint32_t)hal_timer11_irq_handler, },
#endif
#ifdef TIMER2_BASE
    { (uint32_t)hal_timer2x_irq_handler, (uint32_t)hal_timer2x_irq_handler, },
#endif
};

static void hal_timer_common_setup(uint32_t id, uint32_t sub_id, enum HAL_TIMER_TYPE_T type, HAL_TIMER_IRQ_HANDLER_T handler)
{
    uint32_t mode;
    uint32_t irq_en;

    if (type == HAL_TIMER_TYPE_ONESHOT) {
        mode = TIMER_CTRL_ONESHOT;
    } else if (type == HAL_TIMER_TYPE_PERIODIC) {
        mode = TIMER_CTRL_MODE_PERIODIC;
    } else {
        mode = 0;
    }

    irq_handler[id][sub_id] = handler;

    clear_timer_irq(&dual_timer[id]->timer[sub_id]);

    irq_en = (handler && irq_entry[id][sub_id]);

    if (irq_en) {
        NVIC_SetVector(irq_num[id][sub_id], irq_entry[id][sub_id]);
        NVIC_SetPriority(irq_num[id][sub_id], IRQ_PRIORITY_NORMAL);
        NVIC_ClearPendingIRQ(irq_num[id][sub_id]);
        NVIC_EnableIRQ(irq_num[id][sub_id]);
    }

    dual_timer[id]->timer[sub_id].Control = mode |
                                   (irq_en ? TIMER_CTRL_INTEN : 0) |
                                   TIMER_CTRL_PRESCALE_DIV_1 |
                                   TIMER_CTRL_SIZE_32_BIT;
}

POSSIBLY_UNUSED static void hal_timer_common_set_control(uint32_t id, uint32_t sub_id, uint32_t ctrl)
{
    dual_timer[id]->timer[sub_id].Control = ctrl;
}

POSSIBLY_UNUSED static uint32_t hal_timer_common_get_control(uint32_t id, uint32_t sub_id)
{
    return dual_timer[id]->timer[sub_id].Control;
}

static void hal_timer_common_reload(uint32_t id, uint32_t sub_id, uint32_t load)
{
    uint32_t delta;

    delta = timer_is_slow(id) ? SLOW_TIMER_VAL_DELTA : FAST_TIMER_VAL_DELTA;
    if (load > HAL_TIMER_LOAD_DELTA) {
        load -= HAL_TIMER_LOAD_DELTA;
    } else {
        load = HAL_TIMER_LOAD_DELTA;
    }
    set_timer_load(&dual_timer[id]->timer[sub_id], load, delta);
}

POSSIBLY_UNUSED static uint32_t hal_timer_common_get_load(uint32_t id, uint32_t sub_id)
{
    return dual_timer[id]->timer[sub_id].Load + HAL_TIMER_LOAD_DELTA;
}

POSSIBLY_UNUSED static void hal_timer_common_bgload(uint32_t id, uint32_t sub_id, uint32_t load)
{
    if (load > HAL_TIMER_LOAD_DELTA) {
        load -= HAL_TIMER_LOAD_DELTA;
    } else {
        load = HAL_TIMER_LOAD_DELTA;
    }
    dual_timer[id]->timer[sub_id].BGLoad = load;
}

POSSIBLY_UNUSED static void hal_timer_common_pause(uint32_t id, uint32_t sub_id)
{
    dual_timer[id]->timer[sub_id].Control &= ~TIMER_CTRL_EN;
}

static void hal_timer_common_continue(uint32_t id, uint32_t sub_id)
{
    dual_timer[id]->timer[sub_id].Control |= TIMER_CTRL_EN;
}

static void hal_timer_common_start(uint32_t id, uint32_t sub_id, uint32_t load)
{
    start_time[id][sub_id] = timer_is_slow(id) ? hal_sys_timer_get() : hal_fast_sys_timer_get();
    hal_timer_common_reload(id, sub_id, load);
    hal_timer_common_continue(id, sub_id);
}

static void hal_timer_common_stop(uint32_t id, uint32_t sub_id)
{
    dual_timer[id]->timer[sub_id].Control &= ~TIMER_CTRL_EN;
    clear_timer_irq(&dual_timer[id]->timer[sub_id]);
    NVIC_ClearPendingIRQ(irq_num[id][sub_id]);
}

static int hal_timer_common_is_enabled(uint32_t id, uint32_t sub_id)
{
    return !!(dual_timer[id]->timer[sub_id].Control & TIMER_CTRL_EN);
}

static uint32_t hal_timer_common_get_raw_value(uint32_t id, uint32_t sub_id)
{
    uint32_t delta;

    delta = timer_is_slow(id) ? SLOW_TIMER_VAL_DELTA : FAST_TIMER_VAL_DELTA;
    return get_timer_value(&dual_timer[id]->timer[sub_id], delta);
}

static int hal_timer_common_irq_active(uint32_t id, uint32_t sub_id)
{
    return NVIC_GetActive(irq_num[id][sub_id]);
}

static int hal_timer_common_irq_pending(uint32_t id, uint32_t sub_id)
{
    return (dual_timer[id]->timer[sub_id].MIS & TIMER_MIS_MIS);
}

static uint32_t hal_timer_common_get_elapsed_time(uint32_t id, uint32_t sub_id)
{
    uint32_t time;

    time = timer_is_slow(id) ? hal_sys_timer_get() : hal_fast_sys_timer_get();
    return time - start_time[id][sub_id];
}

//----------------------------------------------------------
// Slow timer IRQ API
//----------------------------------------------------------

void hal_timer_setup(enum HAL_TIMER_TYPE_T type, HAL_TIMER_IRQ_HANDLER_T handler)
{
    hal_timer_common_setup(0, SLOW_TIMER_SUB_ID_WITH_IRQ, type, handler);
}

void hal_timer_start(uint32_t load)
{
    hal_timer_common_start(0, SLOW_TIMER_SUB_ID_WITH_IRQ, load);
}

void hal_timer_stop(void)
{
    hal_timer_common_stop(0, SLOW_TIMER_SUB_ID_WITH_IRQ);
}

void hal_timer_continue(void)
{
    hal_timer_common_continue(0, SLOW_TIMER_SUB_ID_WITH_IRQ);
}

int hal_timer_is_enabled(void)
{
    return hal_timer_common_is_enabled(0, SLOW_TIMER_SUB_ID_WITH_IRQ);
}

void hal_timer_reload(uint32_t load)
{
    hal_timer_common_reload(0, SLOW_TIMER_SUB_ID_WITH_IRQ, load);
}

uint32_t hal_timer_get_raw_value(void)
{
    return hal_timer_common_get_raw_value(0, SLOW_TIMER_SUB_ID_WITH_IRQ);
}

int hal_timer_irq_active(void)
{
    return hal_timer_common_irq_active(0, SLOW_TIMER_SUB_ID_WITH_IRQ);
}

int hal_timer_irq_pending(void)
{
    return hal_timer_common_irq_pending(0, SLOW_TIMER_SUB_ID_WITH_IRQ);
}

uint32_t hal_timer_get_elapsed_time(void)
{
    return hal_timer_common_get_elapsed_time(0, SLOW_TIMER_SUB_ID_WITH_IRQ);
}

//----------------------------------------------------------
// Fast timer IRQ API
//----------------------------------------------------------

#ifdef TIMER1_BASE
#ifdef CORE_SLEEP_POWER_DOWN
static uint32_t fast_timer_ctrl;
static uint32_t fast_timer_load;

static void hal_fast_timer_set_control(uint32_t ctrl)
{
    return hal_timer_common_set_control(1, 1, ctrl);
}

static uint32_t hal_fast_timer_get_control(void)
{
    return hal_timer_common_get_control(1, 1);
}

static int hal_fast_timer_pm_notif_handler(enum HAL_PM_STATE_T state)
{
    if (state == HAL_PM_STATE_POWER_DOWN_SLEEP) {
        fast_timer_ctrl = hal_fast_timer_get_control();
        fast_timer_load = hal_fast_timer_get_load();
    } else if (state == HAL_PM_STATE_POWER_DOWN_WAKEUP) {
        hal_fast_timer_set_control(fast_timer_ctrl);
        hal_fast_timer_reload(fast_timer_load);
    }

    return 0;
}
#endif

void hal_fast_timer_setup(enum HAL_TIMER_TYPE_T type, HAL_TIMER_IRQ_HANDLER_T handler)
{
    hal_timer_common_setup(1, 1, type, handler);
#ifdef CORE_SLEEP_POWER_DOWN
    hal_pm_notif_register(HAL_PM_USER_HAL, hal_fast_timer_pm_notif_handler);
#endif
}

void hal_fast_timer_start(uint32_t load)
{
    hal_timer_common_start(1, 1, load);
}

void hal_fast_timer_stop(void)
{
    hal_timer_common_stop(1, 1);
}

void hal_fast_timer_pause(void)
{
    hal_timer_common_pause(1, 1);
}

void hal_fast_timer_continue(void)
{
    hal_timer_common_continue(1, 1);
}

int hal_fast_timer_is_enabled(void)
{
    return hal_timer_common_is_enabled(1, 1);
}

void hal_fast_timer_reload(uint32_t load)
{
    hal_timer_common_reload(1, 1, load);
}

void hal_fast_timer_bgload(uint32_t load)
{
    hal_timer_common_bgload(1, 1, load);
}

uint32_t hal_fast_timer_get_raw_value(void)
{
    return hal_timer_common_get_raw_value(1, 1);
}

int hal_fast_timer_irq_active(void)
{
    return hal_timer_common_irq_active(1, 1);
}

int hal_fast_timer_irq_pending(void)
{
    return hal_timer_common_irq_pending(1, 1);
}

uint32_t hal_fast_timer_get_elapsed_time(void)
{
    return hal_timer_common_get_elapsed_time(1, 1);
}

uint32_t hal_fast_timer_get_load(void)
{
    return hal_timer_common_get_load(1, 1);
}

uint32_t hal_fast_timer_get_count(void)
{
    return dual_timer[1]->timer[1].Load - get_timer_value(&dual_timer[1]->timer[1], FAST_TIMER_VAL_DELTA);
}

#endif

//----------------------------------------------------------
// User timer IRQ API
//----------------------------------------------------------

#ifdef TIMER2_BASE
static void hal_user_timer_clock_enable()
{
    if (hal_cmu_clock_get_status(HAL_CMU_MOD_O_TIMER2) == HAL_CMU_CLK_DISABLED) {
        hal_cmu_clock_enable(HAL_CMU_MOD_O_TIMER2);
        hal_cmu_clock_enable(HAL_CMU_MOD_P_TIMER2);
        hal_cmu_reset_clear(HAL_CMU_MOD_O_TIMER2);
        hal_cmu_reset_clear(HAL_CMU_MOD_P_TIMER2);
    }
}

void hal_user_timer0_setup(enum HAL_TIMER_TYPE_T type, HAL_TIMER_IRQ_HANDLER_T handler)
{
    hal_user_timer_clock_enable();
    hal_timer_common_setup(2, 0, type, handler);
}

void hal_user_timer0_start(uint32_t load)
{
    hal_timer_common_start(2, 0, load);
}

void hal_user_timer0_stop(void)
{
    hal_timer_common_stop(2, 0);
}

void hal_user_timer0_continue(void)
{
    hal_timer_common_continue(2, 0);
}

int hal_user_timer0_is_enabled(void)
{
    return hal_timer_common_is_enabled(2, 0);
}

void hal_user_timer0_reload(uint32_t load)
{
    hal_timer_common_reload(2, 0, load);
}

uint32_t hal_user_timer0_get_raw_value(void)
{
    return hal_timer_common_get_raw_value(2, 0);
}

int hal_user_timer0_irq_active(void)
{
    return hal_timer_common_irq_active(2, 0);
}

int hal_user_timer0_irq_pending(void)
{
    return hal_timer_common_irq_pending(2, 0);
}

uint32_t hal_user_timer0_get_elapsed_time(void)
{
    return hal_timer_common_get_elapsed_time(2, 0);
}

void hal_user_timer1_setup(enum HAL_TIMER_TYPE_T type, HAL_TIMER_IRQ_HANDLER_T handler)
{
    hal_user_timer_clock_enable();
    hal_timer_common_setup(2, 1, type, handler);
}

void hal_user_timer1_start(uint32_t load)
{
    hal_timer_common_start(2, 1, load);
}

void hal_user_timer1_stop(void)
{
    hal_timer_common_stop(2, 1);
}

void hal_user_timer1_continue(void)
{
    hal_timer_common_continue(2, 1);
}

int hal_user_timer1_is_enabled(void)
{
    return hal_timer_common_is_enabled(2, 1);
}

void hal_user_timer1_reload(uint32_t load)
{
    hal_timer_common_reload(2, 1, load);
}

uint32_t hal_user_timer1_get_raw_value(void)
{
    return hal_timer_common_get_raw_value(2, 1);
}

int hal_user_timer1_irq_active(void)
{
    return hal_timer_common_irq_active(2, 1);
}

int hal_user_timer1_irq_pending(void)
{
    return hal_timer_common_irq_pending(2, 1);
}

uint32_t hal_user_timer1_get_elapsed_time(void)
{
    return hal_timer_common_get_elapsed_time(2, 1);
}

#endif
