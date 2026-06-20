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
#include "cmsis.h"

#if (defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)) || \
        (defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U))

#include "cachel1_armv7.h"
#include "dwt.h"
#include "hal_cache.h"
#include "hal_location.h"
#include "hal_trace.h"
#include "pmu_armv8.h"

#define CACHE_SIZE_ICACHE                   0x8000
#define CACHE_SIZE_DCACHE                   0x8000

#define CACHE_LINE_SIZE_ICACHE              __SCB_ICACHE_LINE_SIZE
#define CACHE_LINE_SIZE_DCACHE              __SCB_DCACHE_LINE_SIZE

#define REG(a)                              (*(volatile uint32_t *)(a))

#define MSCR                                0xE001E000

enum CACHE_CNT_IDX_T {
    CACHE_CNT_IDX_ICACHE_ACCESS_L = 0,
    CACHE_CNT_IDX_ICACHE_ACCESS_H,
    CACHE_CNT_IDX_ICACHE_MISS_L,
    CACHE_CNT_IDX_ICACHE_MISS_H,
    CACHE_CNT_IDX_DCACHE_ACCESS_L,
    CACHE_CNT_IDX_DCACHE_ACCESS_H,
    CACHE_CNT_IDX_DCACHE_MISS_L,
    CACHE_CNT_IDX_DCACHE_MISS_H,
};

static uint8_t mon_map;
STATIC_ASSERT(sizeof(mon_map) * 8 >= HAL_CACHE_ID_NUM, "mon_map too small");

__STATIC_FORCEINLINE void cacheip_enable_cache(enum HAL_CACHE_ID_T id)
{
#if (defined (__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE > 0U))
    if (id == HAL_CACHE_ID_I_CACHE) {
        REG(MSCR) |= (1 << 13);
    } else {
        REG(MSCR) |= (1 << 12);
    }
#endif

    if (id == HAL_CACHE_ID_I_CACHE) {
        SCB_EnableICache();
#if (defined (__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE > 0U))
        // Enable Loop and branch info cache
        SCB->CCR |= SCB_CCR_LOB_Msk;
        __DSB();
        __ISB();
#endif
    } else {
        SCB_EnableDCache();
    }
}

__STATIC_FORCEINLINE void cacheip_disable_cache(enum HAL_CACHE_ID_T id)
{
    if (id == HAL_CACHE_ID_I_CACHE) {
#if (defined (__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE > 0U))
        // Disable Loop and branch info cache
        SCB->CCR &= ~SCB_CCR_LOB_Msk;
        __DSB();
        __ISB();
        SCB->BPIALL = 0UL;
        __DSB();
        __ISB();
#endif
        SCB_DisableICache();
    } else {
        SCB_DisableDCache();
    }

#if (defined (__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE > 0U))
    if (id == HAL_CACHE_ID_I_CACHE) {
        REG(MSCR) &= ~(1 << 13);
    } else {
        REG(MSCR) &= ~(1 << 12);
    }
#endif
}

uint8_t BOOT_TEXT_FLASH_LOC hal_cache_enable(enum HAL_CACHE_ID_T id)
{
    cacheip_enable_cache(id);
    return 0;
}

uint8_t SRAM_TEXT_LOC hal_cache_disable(enum HAL_CACHE_ID_T id)
{
    cacheip_disable_cache(id);
    return 0;
}

uint8_t BOOT_TEXT_FLASH_LOC hal_cache_writebuffer_enable(enum HAL_CACHE_ID_T id)
{
    return 0;
}

uint8_t hal_cache_writebuffer_disable(enum HAL_CACHE_ID_T id)
{
    return 0;
}

uint8_t hal_cache_writebuffer_flush(enum HAL_CACHE_ID_T id)
{
    // Drain the store buffer
    __DSB();
    return 0;
}

uint8_t BOOT_TEXT_FLASH_LOC hal_cache_writeback_enable(enum HAL_CACHE_ID_T id)
{
    return 0;
}

uint8_t hal_cache_writeback_disable(enum HAL_CACHE_ID_T id)
{
    return 0;
}

// Wrap is configured during flash init
uint8_t BOOT_TEXT_SRAM_LOC hal_cache_wrap_enable(enum HAL_CACHE_ID_T id)
{
    return 0;
}

uint8_t BOOT_TEXT_SRAM_LOC hal_cache_wrap_disable(enum HAL_CACHE_ID_T id)
{
    return 0;
}

// Flash timing calibration might need to invalidate cache
uint8_t BOOT_TEXT_SRAM_LOC hal_cache_invalidate(enum HAL_CACHE_ID_T id, uint32_t start_address, uint32_t len)
{
    if (id == HAL_CACHE_ID_I_CACHE) {
        if (len > CACHE_SIZE_ICACHE - CACHE_LINE_SIZE_ICACHE) {
            SCB_InvalidateICache();
        } else {
            SCB_InvalidateICache_by_Addr((void *)start_address, len);
        }
    } else {
        // Sync and invalidate operation is much slower than invalidate operation,
        // so sync and invalidate the whole cache only when len is much larger than cache size
        if (len > CACHE_SIZE_DCACHE * 2 - CACHE_LINE_SIZE_DCACHE) {
            SCB_CleanInvalidateDCache();
        } else {
            // PREREQUISITE:
            // To support cache operations not aligned with cache line size,
            // all the buffer shared between processor and hardware device (e.g., DMA)
            // must be invalidated before assigning to the hardware device.
            // Otherwise the hardware device output data might be overwritten by
            // cache sync operaton.

            uint32_t offset;

            // Align start_address to cache line size
            offset = start_address & (CACHE_LINE_SIZE_DCACHE - 1);
            if (offset) {
                offset = CACHE_LINE_SIZE_DCACHE - offset;
                SCB_CleanInvalidateDCache_by_Addr((void *)start_address, offset);
                start_address += offset;
                len -= offset;
            }
            // Align len to cache line size
            offset = len & (CACHE_LINE_SIZE_DCACHE - 1);
            if (offset) {
                len -= offset;
                SCB_CleanInvalidateDCache_by_Addr((void *)start_address + len, offset);
            }
            SCB_InvalidateDCache_by_Addr((void *)start_address, len);
        }
    }

    return 0;
}

uint8_t hal_cache_invalidate_all(enum HAL_CACHE_ID_T id)
{
    if (id == HAL_CACHE_ID_I_CACHE) {
        SCB_InvalidateICache();
    } else {
        SCB_InvalidateDCache();
    }

    return 0;
}

uint8_t BOOT_TEXT_FLASH_LOC hal_cache_boot_sync_all(enum HAL_CACHE_ID_T id)
{
    if (id == HAL_CACHE_ID_D_CACHE) {
        SCB_CleanDCache();
    }

    return 0;
}

uint8_t hal_cache_sync_all(enum HAL_CACHE_ID_T id)
{
    if (id == HAL_CACHE_ID_D_CACHE) {
        SCB_CleanDCache();
    }

    return 0;
}

uint8_t hal_cache_sync_invalidate_all(enum HAL_CACHE_ID_T id)
{
    if (id == HAL_CACHE_ID_I_CACHE) {
        SCB_InvalidateICache();
    } else {
        SCB_CleanInvalidateDCache();
    }

    return 0;
}

uint8_t BOOT_TEXT_SRAM_LOC hal_cache_sync(enum HAL_CACHE_ID_T id, uint32_t start_address, uint32_t len)
{
    if (id == HAL_CACHE_ID_D_CACHE) {
        if (len > CACHE_SIZE_DCACHE - CACHE_LINE_SIZE_DCACHE) {
            SCB_CleanDCache();
        } else {
            SCB_CleanDCache_by_Addr((void *)start_address, len);
        }
    }

    return 0;
}

uint8_t BOOT_TEXT_SRAM_LOC hal_cache_sync_invalidate(enum HAL_CACHE_ID_T id, uint32_t start_address, uint32_t len)
{
    if (id == HAL_CACHE_ID_I_CACHE) {
        if (len > CACHE_SIZE_ICACHE - CACHE_LINE_SIZE_ICACHE) {
            SCB_InvalidateICache();
        } else {
            SCB_InvalidateICache_by_Addr((void *)start_address, len);
        }
    } else {
        if (len > CACHE_SIZE_DCACHE - CACHE_LINE_SIZE_DCACHE) {
            SCB_CleanInvalidateDCache();
        } else {
            SCB_CleanInvalidateDCache_by_Addr((void *)start_address, len);
        }
    }

    return 0;
}

uint8_t hal_cache_monitor_enable(enum HAL_CACHE_ID_T id)
{
    uint32_t lock;

    lock = int_lock();
    if (mon_map == 0) {
        dwt_enable();
        ARM_PMU_Enable();
        hal_cache_reset_monitor_data(HAL_CACHE_ID_I_CACHE);
    }
    mon_map |= (1 << id);
    int_unlock(lock);

    if (id == HAL_CACHE_ID_I_CACHE) {
        ARM_PMU_Set_EVTYPER(CACHE_CNT_IDX_ICACHE_ACCESS_L,  ARM_PMU_L1I_CACHE);
        ARM_PMU_Set_EVTYPER(CACHE_CNT_IDX_ICACHE_ACCESS_H,  ARM_PMU_CHAIN);
        ARM_PMU_Set_EVTYPER(CACHE_CNT_IDX_ICACHE_MISS_L,    ARM_PMU_L1I_CACHE_REFILL);
        ARM_PMU_Set_EVTYPER(CACHE_CNT_IDX_ICACHE_MISS_H,    ARM_PMU_CHAIN);
        ARM_PMU_CNTR_Enable((1 << (CACHE_CNT_IDX_ICACHE_MISS_H + 1)) - 1);
    } else {
        ARM_PMU_Set_EVTYPER(CACHE_CNT_IDX_DCACHE_ACCESS_L,  ARM_PMU_L1D_CACHE);
        ARM_PMU_Set_EVTYPER(CACHE_CNT_IDX_DCACHE_ACCESS_H,  ARM_PMU_CHAIN);
        ARM_PMU_Set_EVTYPER(CACHE_CNT_IDX_DCACHE_MISS_L,    ARM_PMU_L1D_CACHE_REFILL);
        ARM_PMU_Set_EVTYPER(CACHE_CNT_IDX_DCACHE_MISS_H,    ARM_PMU_CHAIN);
        ARM_PMU_CNTR_Enable((1 << (CACHE_CNT_IDX_DCACHE_MISS_H + 1)) - (1 << (CACHE_CNT_IDX_ICACHE_MISS_H + 1)));
    }

    return 0;
}

uint8_t hal_cache_monitor_disable(enum HAL_CACHE_ID_T id)
{
    uint32_t lock;

    if (id == HAL_CACHE_ID_I_CACHE) {
        ARM_PMU_CNTR_Disable((1 << (CACHE_CNT_IDX_ICACHE_MISS_H + 1)) - 1);
    } else {
        ARM_PMU_CNTR_Disable((1 << (CACHE_CNT_IDX_DCACHE_MISS_H + 1)) - (1 << (CACHE_CNT_IDX_ICACHE_MISS_H + 1)));
    }

    lock = int_lock();
    mon_map &= ~(1 << id);
    if (mon_map == 0) {
        ARM_PMU_Disable();
        dwt_disable();
    }
    int_unlock(lock);

    return 0;
}

static uint32_t hal_cache_get_cnt(uint32_t idx)
{
    uint16_t low;
    uint16_t high;
    uint16_t high0;

    high0 = ARM_PMU_Get_EVCNTR(idx + 1);
    low = ARM_PMU_Get_EVCNTR(idx);
    high = ARM_PMU_Get_EVCNTR(idx + 1);
    if (high0 != high) {
        low = ARM_PMU_Get_EVCNTR(idx);
    }

    //TRACE(0, "low[%u]=%u high[%u]=%u", idx, low, idx + 1, high);

    return (low | (high << 16));
}

uint8_t hal_cache_get_monitor_data(enum HAL_CACHE_ID_T id, struct HAL_CACHE_MON_DATA_T *md)
{
    uint32_t lock;
    uint32_t access;
    uint32_t miss;
    uint32_t hit;
    enum CACHE_CNT_IDX_T access_idx;
    enum CACHE_CNT_IDX_T miss_idx;

    if (!md) {
        return 1;
    }
    md->r_hit = 0;
    md->r_miss = 0;
    md->w_hit = 0;
    md->w_miss = 0;

    if (id == HAL_CACHE_ID_I_CACHE) {
        access_idx  = CACHE_CNT_IDX_ICACHE_ACCESS_L;
        miss_idx    = CACHE_CNT_IDX_ICACHE_MISS_L;
    } else {
        access_idx  = CACHE_CNT_IDX_DCACHE_ACCESS_L;
        miss_idx    = CACHE_CNT_IDX_DCACHE_MISS_L;
    }

    lock = int_lock();
    access  = hal_cache_get_cnt(access_idx);
    miss    = hal_cache_get_cnt(miss_idx);
    int_unlock(lock);

    if (access > miss) {
        hit = access - miss;
    } else {
        hit = 0;
    }

    md->r_hit = hit;
    md->r_miss = miss;

    return 0;
}

uint8_t hal_cache_reset_monitor_data(enum HAL_CACHE_ID_T id)
{
    ARM_PMU_EVCNTR_ALL_Reset();
    return 0;
}

void hal_cache_print_stats(void)
{
    struct HAL_CACHE_MON_DATA_T data;
    uint64_t total;
    uint32_t r_ppm;

    TRACE(0, "CACHE STATS:");
    for (int i = HAL_CACHE_ID_I_CACHE; i < HAL_CACHE_ID_NUM; i++) {
        hal_cache_get_monitor_data(i, &data);
        total = data.r_hit + data.r_miss;
        if (total) {
            r_ppm = data.r_miss * 1000000 / total;
        } else {
            r_ppm = 0;
        }
        TRACE(TR_ATTR_NO_TS, "\t[%2u] hit=%u miss=%u miss(ppm)=%3u", i, (uint32_t)data.r_hit, (uint32_t)data.r_miss, r_ppm);
    }

    hal_cache_reset_monitor_data(HAL_CACHE_ID_I_CACHE);
}

#ifdef CORE_SLEEP_POWER_DOWN
SRAM_BSS_LOC
static bool cache_enabled[HAL_CACHE_ID_NUM];

void hal_cache_sleep(void)
{
    bool cache_en;

    cache_en = !!(SCB->CCR & SCB_CCR_DC_Msk);
    if (cache_en) {
        // D-Cache will be cleaned and invalidated after disabled
        cacheip_disable_cache(HAL_CACHE_ID_D_CACHE);
    }
    cache_enabled[HAL_CACHE_ID_D_CACHE] = cache_en;

    cache_en = !!(SCB->CCR & SCB_CCR_IC_Msk);
    if (cache_en) {
        cacheip_disable_cache(HAL_CACHE_ID_I_CACHE);
    }
    cache_enabled[HAL_CACHE_ID_I_CACHE] = cache_en;
}

void SRAM_TEXT_LOC hal_cache_wakeup(void)
{
    __DSB();
    __ISB();
    if (cache_enabled[HAL_CACHE_ID_I_CACHE]) {
        cacheip_enable_cache(HAL_CACHE_ID_I_CACHE);
    }
    if (cache_enabled[HAL_CACHE_ID_D_CACHE]) {
        cacheip_enable_cache(HAL_CACHE_ID_D_CACHE);
    }
}
#endif

#endif
