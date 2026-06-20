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
#include "plat_addr_map.h"
#include "cmsis.h"
#include "hal_location.h"
#include "hal_cache.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "reg_cache.h"

#if defined(ICACHE_CTRL_BASE) || defined(DCACHE_CTRL_BASE)

#if (CHIP_CACHE_VER <= 1)
#include "hal_norflash.h"
#endif

#define HAL_CACHE_YES                       1
#define HAL_CACHE_NO                        0

#if 0
#elif defined(CHIP_BEST1000) || defined(CHIP_BEST2000) || defined(CHIP_BEST3001)
#define CACHE_LINE_SIZE                     0x10
#else
#define CACHE_LINE_SIZE                     0x20
#endif

#define CACHE_ASSOCIATIVITY_WAY_NUM         4

#define CACHEC_ACTIVE_MASK                  (FETCHING_0 | SM_STATE_0_MASK | FETCHING_1 | SM_STATE_1_MASK)

struct SAVED_CACHE_REGS_T {
    uint32_t REG_CACHE_EN;
    uint32_t REG_WRITE_BACK_EN;
    uint32_t REG_MONITOR_EN;
};

#ifdef CORE_SLEEP_POWER_DOWN
static SRAM_BSS_LOC struct SAVED_CACHE_REGS_T icache_regs;
#ifdef DCACHE_CTRL_BASE
static SRAM_BSS_LOC struct SAVED_CACHE_REGS_T dcache_regs;
#endif
#endif

__STATIC_FORCEINLINE uint32_t _cache_get_reg_base(enum HAL_CACHE_ID_T id)
{
    uint32_t base;

    if (id == HAL_CACHE_ID_I_CACHE) {
        base = ICACHE_CTRL_BASE;
#ifdef DCACHE_CTRL_BASE
    } else if (id == HAL_CACHE_ID_D_CACHE) {
        base = DCACHE_CTRL_BASE;
#endif
    } else {
        base = 0;
    }
    return base;
}
__STATIC_FORCEINLINE uint32_t _cache_get_size(enum HAL_CACHE_ID_T id)
{
    uint32_t size;

    if (id == HAL_CACHE_ID_I_CACHE) {
        size = ICACHE_SIZE;
#ifdef DCACHE_CTRL_BASE
    } else if (id == HAL_CACHE_ID_D_CACHE) {
        size = DCACHE_SIZE;
#endif
    } else {
        size = 0;
    }
    return size;
}
__STATIC_FORCEINLINE void cacheip_enable_cache(uint32_t reg_base, uint32_t v)
{
    struct CACHE_T *cache = (struct CACHE_T *)reg_base;
    uint32_t val;
    if (v) {
        val = CACHEC_CACHE_EN;
    } else {
        val = 0;
    }
    cache->CACHE_EN = val;
}
__STATIC_FORCEINLINE POSSIBLY_UNUSED int cacheip_cached_enabled(uint32_t reg_base)
{
    struct CACHE_T *cache = (struct CACHE_T *)reg_base;
    return !!(cache->CACHE_EN & CACHEC_CACHE_EN);
}
__STATIC_FORCEINLINE void cacheip_enable_wrap(uint32_t reg_base, uint32_t v)
{
    struct CACHE_T *cache = (struct CACHE_T *)reg_base;
    if (v) {
        cache->CACHE_EN |= CACHEC_WRAP_EN;
    } else {
        cache->CACHE_EN &= ~CACHEC_WRAP_EN;
    }
}
__STATIC_FORCEINLINE POSSIBLY_UNUSED int cacheip_wrap_enabled(uint32_t reg_base)
{
    struct CACHE_T *cache = (struct CACHE_T *)reg_base;
    return !!(cache->CACHE_EN & CACHEC_WRAP_EN);
}
__STATIC_FORCEINLINE void cacheip_enable_writebuffer(uint32_t reg_base, uint32_t v)
{
	//caches remove write buffer since 2003
#if (CHIP_CACHE_VER <= 2)
    // PSRAM controller V2 has an embedded write buffer and the cache write buffer can be ignored
#if defined(CHIP_HAS_PSRAM) && defined(PSRAM_ENABLE) && defined(CHIP_PSRAM_CTRL_VER) && (CHIP_PSRAM_CTRL_VER == 1)
    struct CACHE_T *cache = (struct CACHE_T *)reg_base;
    if (v) {
        cache->WRITE_BACK_EN |= CACHEC_WRITE_BUFFER_EN;
    } else {
        cache->WRITE_BACK_EN &= ~CACHEC_WRITE_BUFFER_EN;
    }
#endif
#endif
}
__STATIC_FORCEINLINE void cacheip_enable_writeback(uint32_t reg_base, uint32_t v)
{
    // Cache implements write back feature since PSRAM controller V2
#if (defined(CHIP_HAS_PSRAM) && defined(CHIP_PSRAM_CTRL_VER) && (CHIP_PSRAM_CTRL_VER >= 2)) || defined(CHIP_HAS_PSRAMUHS)
    struct CACHE_T *cache = (struct CACHE_T *)reg_base;
    if (v) {
        // Enable NO_WRITE_ALLOCATE mode
        cache->WRITE_BACK_EN = (cache->WRITE_BACK_EN & ~CACHEC_DISNWAMODE) | CACHEC_WRITE_BACK_EN;
    } else {
        cache->WRITE_BACK_EN &= ~CACHEC_WRITE_BACK_EN;
    }
#endif
}
__STATIC_FORCEINLINE void cacheip_enable_double_linefill(uint32_t reg_base, uint32_t v)
{
    struct CACHE_T *cache = (struct CACHE_T *)reg_base;
    if (v) {
        cache->WRITE_BACK_EN |= CACHEC_DOUBLE_LINEFILL;
    } else {
        cache->WRITE_BACK_EN &= ~CACHEC_DOUBLE_LINEFILL;
    }
}
__STATIC_FORCEINLINE void cacheip_flush_writebuffer(uint32_t reg_base)
{
    struct CACHE_T *cache = (struct CACHE_T *)reg_base;
    cache->WRITE_BUFFER_FLUSH = CACHEC_WB_FLUSH;
}
__STATIC_FORCEINLINE void cacheip_invalidate_address(uint32_t reg_base, uint32_t v)
{
    struct CACHE_T *cache = (struct CACHE_T *)reg_base;
#if (CHIP_CACHE_VER <= 4)
    cache->CLEAN_INVLD_ADDR = v;
    cache->CLEAN_INVLD_TRIG = CACHEC_INVALID_TRIG;
#ifdef CHIP_BEST2001 // V2
    // For writeback data cache only (psram data)
    cache->CLEAN_INVLD_TRIG = CACHEC_INVALID_TRIG;
#endif
#else
    cache->INVALID_LINE = v;
#endif
}
#if (CHIP_CACHE_VER >= 3)
__STATIC_FORCEINLINE void cacheip_sync_address(uint32_t reg_base, uint32_t v)
{
    struct CACHE_T *cache = (struct CACHE_T *)reg_base;
#if (CHIP_CACHE_VER <= 4)
    cache->CLEAN_INVLD_ADDR = v;
    cache->CLEAN_TRIG = CACHEC_CLEAN_TRIG;
#else
    cache->CLEAN_LINE = v;
#endif
}
#endif
POSSIBLY_UNUSED
__STATIC_FORCEINLINE void cacheip_sync_invalidate_address(uint32_t reg_base, uint32_t v)
{
#if (CHIP_CACHE_VER <= 4)
    cacheip_invalidate_address(reg_base, v);
#else
    struct CACHE_T *cache = (struct CACHE_T *)reg_base;
    cache->CLEAN_INVLD_LINE = v;
#endif
}
__STATIC_FORCEINLINE void cacheip_sync_all(uint32_t reg_base)
{
    struct CACHE_T *cache = (struct CACHE_T *)reg_base;
    cache->CLEAN_ALL = CACHEC_CLEAN_ALL;
}
__STATIC_FORCEINLINE void cacheip_invalidate_all(uint32_t reg_base)
{
    struct CACHE_T *cache = (struct CACHE_T *)reg_base;
    cache->INVALID_ALL = CACHEC_INVALID_ALL;
#ifdef CHIP_BEST2001 // V2
    // For writeback data cache only (psram data)
    cache->INVALID_ALL = CACHEC_INVALID_ALL;
#endif
}
__STATIC_FORCEINLINE void cacheip_sync_invalidate_all(uint32_t reg_base)
{
#if (CHIP_CACHE_VER <= 2)
    cacheip_invalidate_all(reg_base);
#else
    struct CACHE_T *cache = (struct CACHE_T *)reg_base;
    cache->CLEAN_INVLD_ALL = CACHEC_CLEAN_AND_INVALID_ALL;
#endif
}
__STATIC_FORCEINLINE void cacheip_enable_monitor(uint32_t reg_base, uint32_t v)
{
    struct CACHE_T *cache = (struct CACHE_T *)reg_base;
    uint32_t val;
    if (v) {
        val = CACHEC_MONITOR_EN;
    } else {
        val = 0;
    }
    cache->MONITOR_EN = val;
}
__STATIC_FORCEINLINE uint64_t cacheip_read64(volatile uint32_t *reg_l, volatile uint32_t *reg_h)
{
    uint32_t data[2];
    uint32_t tmp;

    data[1] = *reg_h;
    data[0] = *reg_l;
    tmp = *reg_h;
    if (data[1] != tmp) {
        data[1] = tmp;
        data[0] = *reg_l;
    }
    return (((uint64_t)data[1] << 32) | data[0]);
}
__STATIC_FORCEINLINE void cacheip_get_monitor_data(uint32_t reg_base, struct HAL_CACHE_MON_DATA_T *md)
{
#if (CHIP_CACHE_VER >= 3)
    struct CACHE_T *cache = (struct CACHE_T *)reg_base;
    uint32_t lock;

    lock = int_lock();
    md->r_hit   = cacheip_read64(&cache->READ_HIT0_L, &cache->READ_HIT0_H);
    md->r_miss  = cacheip_read64(&cache->READ_MISS0_L, &cache->READ_MISS0_H);
    md->w_hit   = cacheip_read64(&cache->WRITE_HIT0_L, &cache->WRITE_HIT0_H);
    md->w_miss  = cacheip_read64(&cache->WRITE_MISS0_L, &cache->WRITE_MISS0_H);
#if (CHIP_CACHE_VER >= 4)
    md->r_hit  += cacheip_read64(&cache->READ_HIT1_L, &cache->READ_HIT1_H);
    md->r_miss += cacheip_read64(&cache->READ_MISS1_L, &cache->READ_MISS1_H);
    md->w_hit  += cacheip_read64(&cache->WRITE_HIT1_L, &cache->WRITE_HIT1_H);
    md->w_miss += cacheip_read64(&cache->WRITE_MISS1_L, &cache->WRITE_MISS1_H);
#endif
    int_unlock(lock);
#endif
}
uint8_t BOOT_TEXT_FLASH_LOC hal_cache_enable(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cache_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

#if (CHIP_CACHE_VER >= 3)
    if (cacheip_cached_enabled(reg_base)) {
        cacheip_sync_invalidate_all(reg_base);
    } else
#endif
    {
        cacheip_invalidate_all(reg_base);
        cacheip_enable_cache(reg_base, HAL_CACHE_YES);
    }

    return 0;
}
uint8_t SRAM_TEXT_LOC hal_cache_disable(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cache_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }
#if (CHIP_CACHE_VER >= 3)
    if (!cacheip_cached_enabled(reg_base))
        return 0;
#endif

#if !(defined(ROM_BUILD) || defined(PROGRAMMER))
    uint32_t time;

    time = hal_sys_timer_get();
#if (CHIP_CACHE_VER <= 1)
    while (hal_norflash_busy(HAL_FLASH_ID_NUM) && (hal_sys_timer_get() - time) < MS_TO_TICKS(2));
    // Delay for at least 8 cycles till the cache becomes idle
    for (int delay = 0; delay < 8; delay++) {
        asm volatile("nop");
    }
#else // CHIP_CACHE_VER >= 2
    struct CACHE_T *cache = (struct CACHE_T *)reg_base;
    while ((cache->STATUS & CACHEC_ACTIVE_MASK) && (hal_sys_timer_get() - time) < MS_TO_TICKS(2));
#endif // CHIP_CACHE_VER >= 2
#endif

    cacheip_sync_invalidate_all(reg_base);
    cacheip_enable_cache(reg_base, HAL_CACHE_NO);

    return 0;
}
uint8_t BOOT_TEXT_FLASH_LOC hal_cache_writebuffer_enable(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cache_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_enable_writebuffer(reg_base, HAL_CACHE_YES);

    return 0;
}
uint8_t hal_cache_writebuffer_disable(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cache_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_enable_writebuffer(reg_base, HAL_CACHE_NO);

    return 0;
}
uint8_t hal_cache_writebuffer_flush(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cache_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_flush_writebuffer(reg_base);

    return 0;
}
uint8_t BOOT_TEXT_FLASH_LOC hal_cache_writeback_enable(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cache_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_enable_writeback(reg_base, HAL_CACHE_YES);

    return 0;
}
uint8_t hal_cache_writeback_disable(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cache_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_enable_writeback(reg_base, HAL_CACHE_NO);

    return 0;
}
uint8_t BOOT_TEXT_SRAM_LOC hal_cache_double_linefill_enable(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cache_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_enable_double_linefill(reg_base, HAL_CACHE_YES);

    return 0;
}
uint8_t BOOT_TEXT_SRAM_LOC hal_cache_double_linefill_disable(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cache_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_enable_double_linefill(reg_base, HAL_CACHE_NO);

    return 0;
}
// Wrap is configured during flash init
uint8_t BOOT_TEXT_SRAM_LOC hal_cache_wrap_enable(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cache_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_enable_wrap(reg_base, HAL_CACHE_YES);

    return 0;
}
uint8_t BOOT_TEXT_SRAM_LOC hal_cache_wrap_disable(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cache_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_enable_wrap(reg_base, HAL_CACHE_NO);

    return 0;
}
// Flash timing calibration might need to invalidate cache
uint8_t BOOT_TEXT_SRAM_LOC hal_cache_invalidate(enum HAL_CACHE_ID_T id, uint32_t start_address, uint32_t len)
{
    uint32_t reg_base;
    uint32_t end_address;

#ifndef DCACHE_CTRL_BASE
    if (id == HAL_CACHE_ID_D_CACHE) {
        id = HAL_CACHE_ID_I_CACHE;
    }
#endif

    reg_base = _cache_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }
    uint32_t cache_size = _cache_get_size(id);

#if (CHIP_CACHE_VER <= 4)
#if defined(CHIP_BEST2300) || defined(CHIP_BEST1400) || defined(CACHE_INVD_WAIT_IDLE)
    uint32_t lock;
    uint32_t time;

    lock = int_lock_global();

    time = hal_sys_timer_get();
#if defined(CHIP_BEST2300) || defined(CHIP_BEST1400)
    while (hal_norflash_busy(HAL_FLASH_ID_NUM) && (hal_sys_timer_get() - time) < MS_TO_TICKS(2));
    // Delay for at least 8 cycles till the cache becomes idle
    for (int delay = 0; delay < 8; delay++) {
        asm volatile("nop");
    }
#else
    struct CACHE_T *cache = (struct CACHE_T *)reg_base;
    while ((cache->STATUS & CACHEC_ACTIVE_MASK) && (hal_sys_timer_get() - time) < MS_TO_TICKS(2));
#endif
#endif

    if (len > cache_size / CACHE_ASSOCIATIVITY_WAY_NUM - CACHE_LINE_SIZE) {
        cacheip_sync_invalidate_all(reg_base);
    } else {
        end_address = start_address + len;
        start_address &= ~(CACHE_LINE_SIZE - 1);
        while (start_address < end_address) {
            // In fact it is to sync and invalidate the whole set
            cacheip_invalidate_address(reg_base, start_address);
            start_address += CACHE_LINE_SIZE;
        }
    }

#if defined(CHIP_BEST2300) || defined(CHIP_BEST1400) || defined(CACHE_INVD_WAIT_IDLE)
    int_unlock_global(lock);
#endif

#else // CHIP_CACHE_VER >= 5
    bool proc_whole;

#ifdef DCACHE_CTRL_BASE
    if (id == HAL_CACHE_ID_D_CACHE) {
        // Sync and invalidate operation is much slower than invalidate operation,
        // so sync and invalidate the whole cache only when len is much larger than cache size
        proc_whole = (len > cache_size * 2 - CACHE_LINE_SIZE);
    } else
#endif
    {
        proc_whole = (len > cache_size - CACHE_LINE_SIZE);
    }

    if (proc_whole) {
        cacheip_sync_invalidate_all(reg_base);
    } else {
        // PREREQUISITE:
        // To support cache operations not aligned with cache line size,
        // all the buffer shared between processor and hardware device (e.g., DMA)
        // must be invalidated before assigning to the hardware device.
        // Otherwise the hardware device output data might be overwritten by
        // cache sync operaton.

        uint32_t offset;

        // Align start_address to cache line size
        offset = start_address & (CACHE_LINE_SIZE - 1);
        if (offset) {
            offset = CACHE_LINE_SIZE - offset;
            cacheip_sync_invalidate_address(reg_base, start_address);
            start_address += offset;
            len -= offset;
        }
        // Align len to cache line size
        offset = len & (CACHE_LINE_SIZE - 1);
        if (offset) {
            cacheip_sync_invalidate_address(reg_base, start_address + len);
            len -= offset;
        }
        end_address = start_address + len;
        while (start_address < end_address) {
            cacheip_invalidate_address(reg_base, start_address);
            start_address += CACHE_LINE_SIZE;
        }
    }

#endif // CHIP_CACHE_VER >= 5

    return 0;
}
uint8_t hal_cache_invalidate_all(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

#ifndef DCACHE_CTRL_BASE
    if (id == HAL_CACHE_ID_D_CACHE) {
        id = HAL_CACHE_ID_I_CACHE;
    }
#endif

    reg_base = _cache_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

#if defined(CACHE_INVD_WAIT_IDLE)
    uint32_t lock;
    uint32_t time;

    lock = int_lock_global();

    time = hal_sys_timer_get();
    struct CACHE_T *cache = (struct CACHE_T *)reg_base;
    while ((cache->STATUS & CACHEC_ACTIVE_MASK) && (hal_sys_timer_get() - time) < MS_TO_TICKS(2));
#endif

    cacheip_invalidate_all(reg_base);

#if defined(CACHE_INVD_WAIT_IDLE)
    int_unlock_global(lock);
#endif

    return 0;
}
uint8_t BOOT_TEXT_FLASH_LOC hal_cache_boot_sync_all(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cache_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_sync_all(reg_base);
    return 0;
}
uint8_t hal_cache_sync_all(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cache_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_sync_all(reg_base);
    return 0;
}
#if (CHIP_CACHE_VER <= 2)
uint8_t hal_cache_sync(enum HAL_CACHE_ID_T id, uint32_t start_address, uint32_t len) __attribute__((alias("hal_cache_invalidate")));
#else // CHIP_CACHE_VER >= 3
uint8_t BOOT_TEXT_SRAM_LOC hal_cache_sync(enum HAL_CACHE_ID_T id, uint32_t start_address, uint32_t len)
{
    uint32_t reg_base;
    uint32_t end_address;

    reg_base = _cache_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }
    uint32_t cache_size = _cache_get_size(id);

    if (len > cache_size / CACHE_ASSOCIATIVITY_WAY_NUM - CACHE_LINE_SIZE) {
        cacheip_sync_all(reg_base);
    } else {
        end_address = start_address + len;
        start_address &= ~(CACHE_LINE_SIZE - 1);
        while (start_address < end_address) {
            // V3/V4: Sync the whole set
            // V5 and later: Sync the whole line
            cacheip_sync_address(reg_base, start_address);
            start_address += CACHE_LINE_SIZE;
        }
    }
    return 0;
}
#endif // CHIP_CACHE_VER >= 3
uint8_t BOOT_TEXT_SRAM_LOC hal_cache_sync_invalidate_all(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base;

#ifndef DCACHE_CTRL_BASE
    if (id == HAL_CACHE_ID_D_CACHE) {
        id = HAL_CACHE_ID_I_CACHE;
    }
#endif

    reg_base = _cache_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_sync_invalidate_all(reg_base);
    return 0;
}
#if (CHIP_CACHE_VER <= 4)
uint8_t hal_cache_sync_invalidate(enum HAL_CACHE_ID_T id, uint32_t start_address, uint32_t len) __attribute__((alias("hal_cache_invalidate")));
#else // CHIP_CACHE_VER >= 5
uint8_t BOOT_TEXT_SRAM_LOC hal_cache_sync_invalidate(enum HAL_CACHE_ID_T id, uint32_t start_address, uint32_t len)
{
    uint32_t reg_base;
    uint32_t end_address;

#ifndef DCACHE_CTRL_BASE
    if (id == HAL_CACHE_ID_D_CACHE) {
        id = HAL_CACHE_ID_I_CACHE;
    }
#endif

    reg_base = _cache_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }
    uint32_t cache_size = _cache_get_size(id);

    if (len > cache_size - CACHE_LINE_SIZE) {
        cacheip_sync_invalidate_all(reg_base);
    } else {
        end_address = start_address + len;
        start_address &= ~(CACHE_LINE_SIZE - 1);
        while (start_address < end_address) {
            cacheip_sync_invalidate_address(reg_base, start_address);
            start_address += CACHE_LINE_SIZE;
        }
    }
    return 0;
}
#endif // CHIP_CACHE_VER >= 5
uint8_t hal_cache_monitor_enable(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cache_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_enable_monitor(reg_base, 1);
    return 0;
}
uint8_t hal_cache_monitor_disable(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cache_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_enable_monitor(reg_base, 0);
    return 0;
}
uint8_t hal_cache_get_monitor_data(enum HAL_CACHE_ID_T id, struct HAL_CACHE_MON_DATA_T *md)
{
    uint32_t reg_base = 0;

    if (!md) {
        return 1;
    }
    md->r_hit = 0;
    md->r_miss = 0;
    md->w_hit = 0;
    md->w_miss = 0;

    reg_base = _cache_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_get_monitor_data(reg_base, md);
    return 0;
}
uint8_t hal_cache_reset_monitor_data(enum HAL_CACHE_ID_T id)
{
    hal_cache_monitor_disable(id);
    hal_cache_monitor_enable(id);
    return 0;
}
void hal_cache_print_stats(void)
{
    struct HAL_CACHE_MON_DATA_T data;
    uint64_t total;
    uint32_t r_ppm;
    uint32_t w_ppm;

    TRACE(0, "CACHE STATS:");
    for (int i = HAL_CACHE_ID_I_CACHE; i < HAL_CACHE_ID_NUM; i++) {
        hal_cache_get_monitor_data(i, &data);
        hal_cache_reset_monitor_data(i);
        total = data.r_hit + data.r_miss;
        if (total) {
            r_ppm = data.r_miss * 1000000 / total;
        } else {
            r_ppm = 0;
        }
        total = data.w_hit + data.w_miss;
        if (total) {
            w_ppm = data.w_miss * 1000000 / total;
        } else {
            w_ppm = 0;
        }
        TRACE(TR_ATTR_NO_TS, "\t[%2u] r_miss=%3u w_miss=%3u (ppm)", i, r_ppm, w_ppm);
    }
}

#ifdef CORE_SLEEP_POWER_DOWN
void hal_cache_sleep(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
#ifdef ARM_CMNS
    return;
#endif

    if (mode != HAL_CMU_LPU_SLEEP_MODE_SUBSYS_PD) {
        return;
    }

    hal_cache_sync_all(HAL_CACHE_ID_D_CACHE);

    struct CACHE_T *cache = (struct CACHE_T *)ICACHE_CTRL_BASE;
    icache_regs.REG_MONITOR_EN = cache->MONITOR_EN;
    icache_regs.REG_WRITE_BACK_EN = cache->WRITE_BACK_EN;
    icache_regs.REG_CACHE_EN = cache->CACHE_EN;
#ifdef DCACHE_CTRL_BASE
    cache = (struct CACHE_T *)DCACHE_CTRL_BASE;
    dcache_regs.REG_MONITOR_EN = cache->MONITOR_EN;
    dcache_regs.REG_WRITE_BACK_EN = cache->WRITE_BACK_EN;
    dcache_regs.REG_CACHE_EN = cache->CACHE_EN;
#endif
}
void SRAM_TEXT_LOC hal_cache_wakeup(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
#ifdef ARM_CMNS
    return;
#endif

    if (mode != HAL_CMU_LPU_SLEEP_MODE_SUBSYS_PD) {
        return;
    }

    if (icache_regs.REG_CACHE_EN & CACHEC_CACHE_EN) {
        cacheip_invalidate_all(ICACHE_CTRL_BASE);
    }
    struct CACHE_T *cache = (struct CACHE_T *)ICACHE_CTRL_BASE;
    cache->CACHE_EN = icache_regs.REG_CACHE_EN;
    cache->WRITE_BACK_EN = icache_regs.REG_WRITE_BACK_EN;
    cache->MONITOR_EN = icache_regs.REG_MONITOR_EN;
#ifdef DCACHE_CTRL_BASE
    if (dcache_regs.REG_CACHE_EN & CACHEC_CACHE_EN) {
        cacheip_invalidate_all(DCACHE_CTRL_BASE);
    }
    cache = (struct CACHE_T *)DCACHE_CTRL_BASE;
    cache->CACHE_EN = dcache_regs.REG_CACHE_EN;
    cache->WRITE_BACK_EN = dcache_regs.REG_WRITE_BACK_EN;
    cache->MONITOR_EN = dcache_regs.REG_MONITOR_EN;
#endif
}
#endif

#if defined(CP_IN_SAME_EE) && defined(ICACHECP_CTRL_BASE)
__STATIC_FORCEINLINE uint32_t _cachecp_get_reg_base(enum HAL_CACHE_ID_T id)
{
    uint32_t base;

#if (ICACHECP_CTRL_BASE == ICACHE_CTRL_BASE)
    if (get_cpu_id() == 0) {
        ASSERT(0, "MCU can't access to CP's cache, because their cache base is equal.");
        return 0;
    }
#endif

    if (id == HAL_CACHE_ID_I_CACHE) {
        base = ICACHECP_CTRL_BASE;
#ifdef DCACHECP_CTRL_BASE
    } else if (id == HAL_CACHE_ID_D_CACHE) {
        base = DCACHECP_CTRL_BASE;
#endif
    } else {
        base = 0;
    }
    return base;
}
POSSIBLY_UNUSED
__STATIC_FORCEINLINE uint32_t _cachecp_get_size(enum HAL_CACHE_ID_T id)
{
    uint32_t size;

    if (id == HAL_CACHE_ID_I_CACHE) {
        size = ICACHECP_SIZE;
#ifdef DCACHECP_CTRL_BASE
    } else if (id == HAL_CACHE_ID_D_CACHE) {
        size = DCACHECP_SIZE;
#endif
    } else {
        size = 0;
    }
    return size;
}
uint8_t hal_cachecp_enable(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;
    uint32_t main_cache_reg_base;
    enum HAL_CMU_MOD_ID_T mod;

    reg_base = _cachecp_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

#ifdef DCACHECP_CTRL_BASE
    if (id == HAL_CACHE_ID_D_CACHE) {
        mod = HAL_CMU_MOD_H_DCACHECP;
    } else
#endif
    {
        mod = HAL_CMU_MOD_H_ICACHECP;
    }
    hal_cmu_clock_enable(mod);
    hal_cmu_reset_clear(mod);

    cacheip_invalidate_all(reg_base);
    cacheip_enable_cache(reg_base, HAL_CACHE_YES);
    // Init wrap option
    main_cache_reg_base = _cache_get_reg_base(id);
    if (main_cache_reg_base == 0) {
        return 0;
    }
    cacheip_enable_wrap(reg_base, cacheip_wrap_enabled(main_cache_reg_base));

    return 0;
}
uint8_t CP_TEXT_SRAM_LOC hal_cachecp_disable(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;
    enum HAL_CMU_MOD_ID_T mod;

    reg_base = _cachecp_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

#if !(defined(ROM_BUILD) || defined(PROGRAMMER))
    uint32_t time = hal_sys_timer_get();
    struct CACHE_T *cache = (struct CACHE_T *)reg_base;
    while ((cache->STATUS & CACHEC_ACTIVE_MASK) && (hal_sys_timer_get() - time) < MS_TO_TICKS(2));
#endif

    cacheip_enable_cache(reg_base, HAL_CACHE_NO);

#ifdef DCACHECP_CTRL_BASE
    if (id == HAL_CACHE_ID_D_CACHE) {
        mod = HAL_CMU_MOD_H_DCACHECP;
    } else
#endif
    {
        mod = HAL_CMU_MOD_H_ICACHECP;
    }
    hal_cmu_reset_set(mod);
    hal_cmu_clock_disable(mod);

    return 0;
}
uint8_t hal_cachecp_writebuffer_enable(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cachecp_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_enable_writebuffer(reg_base, HAL_CACHE_YES);

    return 0;
}
uint8_t hal_cachecp_writebuffer_disable(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cachecp_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_enable_writebuffer(reg_base, HAL_CACHE_NO);

    return 0;
}
uint8_t hal_cachecp_writebuffer_flush(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cachecp_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_flush_writebuffer(reg_base);

    return 0;
}
uint8_t hal_cachecp_writeback_enable(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cachecp_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_enable_writeback(reg_base, HAL_CACHE_YES);

    return 0;
}
uint8_t hal_cachecp_writeback_disable(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cachecp_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_enable_writeback(reg_base, HAL_CACHE_NO);

    return 0;
}
uint8_t hal_cachecp_double_linefill_enable(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cachecp_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_enable_double_linefill(reg_base, HAL_CACHE_YES);

    return 0;
}
uint8_t hal_cachecp_double_linefill_disable(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cachecp_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_enable_double_linefill(reg_base, HAL_CACHE_NO);

    return 0;
}
uint8_t CP_TEXT_SRAM_LOC hal_cachecp_invalidate(enum HAL_CACHE_ID_T id, uint32_t start_address, uint32_t len)
{
    uint32_t reg_base;
    uint32_t end_address;

#ifndef DCACHECP_CTRL_BASE
    if (id == HAL_CACHE_ID_D_CACHE) {
        id = HAL_CACHE_ID_I_CACHE;
    }
#endif

    reg_base = _cachecp_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    end_address = start_address + len;
    start_address &= ~(CACHE_LINE_SIZE - 1);
    while (start_address < end_address) {
        cacheip_invalidate_address(reg_base, start_address);
        start_address += CACHE_LINE_SIZE;
    }

    return 0;
}
uint8_t hal_cachecp_sync_all(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cachecp_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_sync_all(reg_base);

    return 0;
}
uint8_t hal_cachecp_monitor_enable(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cachecp_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_enable_monitor(reg_base, 1);
    return 0;
}
uint8_t hal_cachecp_monitor_disable(enum HAL_CACHE_ID_T id)
{
    uint32_t reg_base = 0;

    reg_base = _cachecp_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_enable_monitor(reg_base, 0);
    return 0;
}
uint8_t hal_cachecp_get_monitor_data(enum HAL_CACHE_ID_T id, struct HAL_CACHE_MON_DATA_T *md)
{
    uint32_t reg_base = 0;

    if (!md) {
        return 1;
    }
    md->r_hit = 0;
    md->r_miss = 0;
    md->w_hit = 0;
    md->w_miss = 0;

    reg_base = _cachecp_get_reg_base(id);
    if (reg_base == 0) {
        return 0;
    }

    cacheip_get_monitor_data(reg_base, md);
    return 0;
}
uint8_t hal_cachecp_reset_monitor_data(enum HAL_CACHE_ID_T id)
{
    hal_cachecp_monitor_disable(id);
    hal_cachecp_monitor_enable(id);
    return 0;
}
void hal_cachecp_print_stats(void)
{
    struct HAL_CACHE_MON_DATA_T data;
    uint64_t total;
    uint32_t r_ppm;
    uint32_t w_ppm;

    TRACE(0, "CACHE-CP STATS:");
    for (int i = HAL_CACHE_ID_I_CACHE; i < HAL_CACHE_ID_NUM; i++) {
        hal_cachecp_get_monitor_data(i, &data);
        hal_cachecp_reset_monitor_data(i);
        total = data.r_hit + data.r_miss;
        if (total) {
            r_ppm = data.r_miss * 1000000 / total;
        } else {
            r_ppm = 0;
        }
        total = data.w_hit + data.w_miss;
        if (total) {
            w_ppm = data.w_miss * 1000000 / total;
        } else {
            w_ppm = 0;
        }
        TRACE(TR_ATTR_NO_TS, "\t[%2u] r_miss=%3u w_miss=%3u (ppm)", i, r_ppm, w_ppm);
    }
}
#endif

#endif
