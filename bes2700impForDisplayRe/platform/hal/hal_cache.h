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
#ifndef CACHE_HAL_H
#define CACHE_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cmsis.h"
#include "hal_cmu.h"
#include "plat_addr_map.h"
#include "plat_types.h"

#ifndef MAX_CACHE_LINE_SIZE
#if defined(FLASH_CACHE_DOUBLE_LINEFILL) || defined(PSRAM_CACHE_DOUBLE_LINEFILL) || \
    (defined(L1CACHE_API_CTRL_L2CACHE) && defined(L2CC0_BASE))
#define MAX_CACHE_LINE_SIZE             64U
#else
#define MAX_CACHE_LINE_SIZE             32U
#endif
#endif

#define LEN_ALIGN_TO_CACHE(len, addr)   (((uint32_t)(len) + \
                                          ((uint32_t)(addr) & (MAX_CACHE_LINE_SIZE - 1)) + \
                                          (MAX_CACHE_LINE_SIZE - 1)) & ~(MAX_CACHE_LINE_SIZE - 1))
#define ADDR_ALIGN_TO_CACHE(addr)       ((uint32_t)(addr) & ~(MAX_CACHE_LINE_SIZE - 1))

#if (defined(ICACHE_CTRL_BASE) || defined(DCACHE_CTRL_BASE)) || \
        (defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)) || \
        (defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U))
#define CACHE_FUNC_DECLARE(d, r)        d
#else
#define CACHE_FUNC_DECLARE(d, r)        __STATIC_FORCEINLINE d { r; }
#endif

#ifdef ICACHECP_CTRL_BASE
#define CACHECP_FUNC_DECLARE(d, r)      d
#else
#define CACHECP_FUNC_DECLARE(d, r)      __STATIC_FORCEINLINE d { r; }
#endif

#if defined(ICACHE_CTRL_BASE) || defined(DCACHE_CTRL_BASE)
#define BESCACHE_FUNC_DECLARE(d, r)     d
#else
#define BESCACHE_FUNC_DECLARE(d, r)     __STATIC_FORCEINLINE d { r; }
#endif

#if (defined(ICACHE_CTRL_BASE) || defined(DCACHE_CTRL_BASE)) && !(defined(CHIP_BEST1502P) || defined(CHIP_BEST2001))
#define HAL_CACHE_ID_FLASH_EXEC         HAL_CACHE_ID_I_CACHE
#define HAL_CACHE_ID_FLASH_DATA         HAL_CACHE_ID_I_CACHE
#define HAL_CACHE_ID_PSRAM_EXEC         HAL_CACHE_ID_D_CACHE
#define HAL_CACHE_ID_PSRAM_DATA         HAL_CACHE_ID_D_CACHE
#else
#define HAL_CACHE_ID_FLASH_EXEC         HAL_CACHE_ID_I_CACHE
#define HAL_CACHE_ID_FLASH_DATA         HAL_CACHE_ID_D_CACHE
#define HAL_CACHE_ID_PSRAM_EXEC         HAL_CACHE_ID_I_CACHE
#define HAL_CACHE_ID_PSRAM_DATA         HAL_CACHE_ID_D_CACHE
#endif

enum HAL_CACHE_ID_T {
    HAL_CACHE_ID_I_CACHE = 0,
    HAL_CACHE_ID_D_CACHE,
    HAL_CACHE_ID_NUM,
};

struct HAL_CACHE_MON_DATA_T {
    uint64_t r_hit;
    uint64_t r_miss;
    uint64_t w_hit;
    uint64_t w_miss;
};

CACHE_FUNC_DECLARE(uint8_t hal_cache_enable(enum HAL_CACHE_ID_T id), return 0);
CACHE_FUNC_DECLARE(uint8_t hal_cache_disable(enum HAL_CACHE_ID_T id), return 0);
BESCACHE_FUNC_DECLARE(uint8_t hal_cache_writebuffer_enable(enum HAL_CACHE_ID_T id), return 0);
BESCACHE_FUNC_DECLARE(uint8_t hal_cache_writebuffer_disable(enum HAL_CACHE_ID_T id), return 0);
CACHE_FUNC_DECLARE(uint8_t hal_cache_writebuffer_flush(enum HAL_CACHE_ID_T id), return 0);
BESCACHE_FUNC_DECLARE(uint8_t hal_cache_writeback_enable(enum HAL_CACHE_ID_T id), return 0);
BESCACHE_FUNC_DECLARE(uint8_t hal_cache_writeback_disable(enum HAL_CACHE_ID_T id), return 0);
BESCACHE_FUNC_DECLARE(uint8_t hal_cache_double_linefill_enable(enum HAL_CACHE_ID_T id), return 0);
BESCACHE_FUNC_DECLARE(uint8_t hal_cache_double_linefill_disable(enum HAL_CACHE_ID_T id), return 0);
BESCACHE_FUNC_DECLARE(uint8_t hal_cache_wrap_enable(enum HAL_CACHE_ID_T id), return 0);
BESCACHE_FUNC_DECLARE(uint8_t hal_cache_wrap_disable(enum HAL_CACHE_ID_T id), return 0);
CACHE_FUNC_DECLARE(uint8_t hal_cache_invalidate_all(enum HAL_CACHE_ID_T id), return 0);
CACHE_FUNC_DECLARE(uint8_t hal_cache_invalidate(enum HAL_CACHE_ID_T id, uint32_t start_address, uint32_t len), return 0);
CACHE_FUNC_DECLARE(uint8_t hal_cache_boot_sync_all(enum HAL_CACHE_ID_T id), return 0);
CACHE_FUNC_DECLARE(uint8_t hal_cache_sync_all(enum HAL_CACHE_ID_T id), return 0);
CACHE_FUNC_DECLARE(uint8_t hal_cache_sync(enum HAL_CACHE_ID_T id, uint32_t start_address, uint32_t len), return 0);
CACHE_FUNC_DECLARE(uint8_t hal_cache_sync_invalidate_all(enum HAL_CACHE_ID_T id), return 0);
CACHE_FUNC_DECLARE(uint8_t hal_cache_sync_invalidate(enum HAL_CACHE_ID_T id, uint32_t start_address, uint32_t len), return 0);
CACHE_FUNC_DECLARE(uint8_t hal_cache_monitor_enable(enum HAL_CACHE_ID_T id), return 0);
CACHE_FUNC_DECLARE(uint8_t hal_cache_monitor_disable(enum HAL_CACHE_ID_T id), return 0);
CACHE_FUNC_DECLARE(uint8_t hal_cache_get_monitor_data(enum HAL_CACHE_ID_T id, struct HAL_CACHE_MON_DATA_T *md), return 0);
CACHE_FUNC_DECLARE(uint8_t hal_cache_reset_monitor_data(enum HAL_CACHE_ID_T id), return 0);
CACHE_FUNC_DECLARE(void hal_cache_print_stats(void), return);
CACHE_FUNC_DECLARE(void hal_cache_sleep(enum HAL_CMU_LPU_SLEEP_MODE_T mode), return);
CACHE_FUNC_DECLARE(void hal_cache_wakeup(enum HAL_CMU_LPU_SLEEP_MODE_T mode), return);

CACHECP_FUNC_DECLARE(uint8_t hal_cachecp_enable(enum HAL_CACHE_ID_T id), return 0);
CACHECP_FUNC_DECLARE(uint8_t hal_cachecp_disable(enum HAL_CACHE_ID_T id), return 0);
CACHECP_FUNC_DECLARE(uint8_t hal_cachecp_writebuffer_enable(enum HAL_CACHE_ID_T id), return 0);
CACHECP_FUNC_DECLARE(uint8_t hal_cachecp_writebuffer_disable(enum HAL_CACHE_ID_T id), return 0);
CACHECP_FUNC_DECLARE(uint8_t hal_cachecp_writebuffer_flush(enum HAL_CACHE_ID_T id), return 0);
CACHECP_FUNC_DECLARE(uint8_t hal_cachecp_writeback_enable(enum HAL_CACHE_ID_T id), return 0);
CACHECP_FUNC_DECLARE(uint8_t hal_cachecp_writeback_disable(enum HAL_CACHE_ID_T id), return 0);
CACHECP_FUNC_DECLARE(uint8_t hal_cachecp_double_linefill_enable(enum HAL_CACHE_ID_T id), return 0);
CACHECP_FUNC_DECLARE(uint8_t hal_cachecp_double_linefill_disable(enum HAL_CACHE_ID_T id), return 0);
CACHECP_FUNC_DECLARE(uint8_t hal_cachecp_invalidate(enum HAL_CACHE_ID_T id, uint32_t start_address, uint32_t len), return 0);
CACHECP_FUNC_DECLARE(uint8_t hal_cachecp_sync_all(enum HAL_CACHE_ID_T id), return 0);
CACHECP_FUNC_DECLARE(uint8_t hal_cachecp_monitor_enable(enum HAL_CACHE_ID_T id), return 0);
CACHECP_FUNC_DECLARE(uint8_t hal_cachecp_monitor_disable(enum HAL_CACHE_ID_T id), return 0);
CACHECP_FUNC_DECLARE(uint8_t hal_cachecp_get_monitor_data(enum HAL_CACHE_ID_T id, struct HAL_CACHE_MON_DATA_T *md), return 0);
CACHECP_FUNC_DECLARE(uint8_t hal_cachecp_reset_monitor_data(enum HAL_CACHE_ID_T id), return 0);
CACHECP_FUNC_DECLARE(void hal_cachecp_print_stats(void), return);

#ifdef __cplusplus
}
#endif

#endif /* CACHE_HAL_H */
