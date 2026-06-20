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
#ifdef __ARM_ARCH_8M_MAIN__

#include "cmsis.h"
#include "mpu.h"
#include "hal_location.h"
#include "hal_trace.h"
#include "hal_overlay.h"

extern uint32_t __sram_text_start__[];
extern uint32_t __sram_text_end__[];
extern uint32_t __fast_sram_text_exec_start__[];
extern uint32_t __fast_sram_text_exec_end__[];

// Non-Transient, Write-Back, Read-Allocation, Write-Allocation
#define NORM_MEM_WT_RA_ATTR             ARM_MPU_ATTR_MEMORY_(1, 0, 1, 0)
#define NORM_MEM_WB_WA_ATTR             ARM_MPU_ATTR_MEMORY_(1, 1, 1, 1)
#define NORM_MEM_NC_ATTR                ARM_MPU_ATTR_NON_CACHEABLE
#define DEV_MEM_ATTR_INNER              ARM_MPU_ATTR_DEVICE_nGnRnE
#define DEV_MEM_ATTR_OUTER              ARM_MPU_ATTR_DEVICE

static uint32_t mpu_region_map;
STATIC_ASSERT(sizeof(mpu_region_map) * 8 >= MPU_ID_QTY, "mpu_regsion_map size too small");

BOOT_TEXT_FLASH_LOC
static void init_mair_attr(void)
{
    MPU->MAIR0 = 0;
    MPU->MAIR1 = 0;

    ARM_MPU_SetMemAttr(MEM_ATTR_NORMAL_WRITE_THROUGH,   ARM_MPU_ATTR(NORM_MEM_WT_RA_ATTR,   NORM_MEM_WT_RA_ATTR));
    ARM_MPU_SetMemAttr(MEM_ATTR_NORMAL_WRITE_BACK,      ARM_MPU_ATTR(NORM_MEM_WB_WA_ATTR,   NORM_MEM_WB_WA_ATTR));
    ARM_MPU_SetMemAttr(MEM_ATTR_NORMAL_NON_CACHEABLE,   ARM_MPU_ATTR(NORM_MEM_NC_ATTR,      NORM_MEM_NC_ATTR));
    ARM_MPU_SetMemAttr(MEM_ATTR_DEVICE,                 ARM_MPU_ATTR(DEV_MEM_ATTR_OUTER,    DEV_MEM_ATTR_INNER));
}

static int mpu_enable(void)
{
    int flags = 0;

#ifdef CP_IN_SAME_EE
    if (get_cpu_id()) {
        /*
         * if cpu is CP, we will use different mpu maps, that is, every map needed by CP will
         * be mapped, and the memory is not mapped will issue abort when access
         */
        flags |= MPU_CTRL_HFNMIENA_Msk;
    } else
#endif
    {
#ifndef NO_MPU_DEFAULT_MAP
        /*
         * mpu maps use default map designed by arm, that is, if the memory is not mapped,
         * the mpu attibutes will use the default setting by ARM.
         */
        flags |= MPU_CTRL_PRIVDEFENA_Msk;
#endif
    }

    ARM_MPU_Enable(flags);
    return 0;
}

static int mpu_disable(void)
{
    ARM_MPU_Disable();
    return 0;
}

BOOT_TEXT_FLASH_LOC
int mpu_open(void)
{
    int i;

    if ((MPU->TYPE & MPU_TYPE_DREGION_Msk) == 0) {
        return -1;
    }

    mpu_disable();

    for (i = 0; i < MPU_ID_QTY; i++) {
        ARM_MPU_ClrRegion(i);
    }

    init_mair_attr();

    return 0;
}

int mpu_close(void)
{
    mpu_disable();
    return 0;
}

__STATIC_FORCEINLINE int mpu_gen_rbar_rlar(uint32_t addr, uint32_t len,
                                           enum MPU_ATTR_T ap_attr, enum MEM_ATTR_T mem_attr,
                                           uint32_t *rbar, uint32_t *rlar)
{
    uint8_t xn;
    uint8_t ro;

    if (len < 32) {
        return -2;
    }
    if (addr & 0x1F) {
        return -3;
    }
    if (ap_attr >= MPU_ATTR_QTY) {
        return -4;
    }

    if (ap_attr == MPU_ATTR_READ_WRITE_EXEC || ap_attr == MPU_ATTR_READ_EXEC ||
            ap_attr == MPU_ATTR_EXEC) {
        xn = 0;
    } else {
        xn = 1;
    }

    if (ap_attr == MPU_ATTR_READ_WRITE_EXEC || ap_attr == MPU_ATTR_READ_WRITE) {
        ro = 0;
    } else if (ap_attr == MPU_ATTR_READ_EXEC || ap_attr == MPU_ATTR_READ || ap_attr == MPU_ATTR_EXEC) {
        ro = 1;
    } else {
#ifdef NO_MPU_DEFAULT_MAP
        ro = 1;
#else
        // Cannot support no access
        return -5;
#endif
    }

    // Sharebility: Non-shareable
    // Non-privilege Access: Enabled
    *rbar = ARM_MPU_RBAR(addr, 0, ro, 1, xn);
    *rlar = ARM_MPU_RLAR((addr + len - 1), mem_attr);

    return 0;
}

static void mpu_set_region_armv8(uint8_t id, uint32_t rbar, uint32_t rlar)
{
    uint32_t lock;

    lock = int_lock_global();
    ARM_MPU_SetRegion(id, rbar, rlar);
    __DSB();
    __ISB();
    int_unlock_global(lock);
}

static int mpu_set_armv8(uint8_t id, uint32_t addr, uint32_t len,
                         enum MPU_ATTR_T ap_attr, enum MEM_ATTR_T mem_attr)
{
    int ret;
    uint32_t rbar;
    uint32_t rlar;

    if (id >= MPU_ID_QTY) {
        return -1;
    }

    ret = mpu_gen_rbar_rlar(addr, len, ap_attr, mem_attr, &rbar, &rlar);
    if (ret) {
        return ret;
    }

    mpu_set_region_armv8(id, rbar, rlar);

    return 0;
}

BOOT_TEXT_FLASH_LOC
static int mpu_boot_set_armv8(uint8_t id, uint32_t addr, uint32_t len,
                              enum MPU_ATTR_T ap_attr, enum MEM_ATTR_T mem_attr)
{
    int ret;
    uint32_t rbar;
    uint32_t rlar;

    if (id >= MPU_ID_QTY) {
        return -1;
    }

    ret = mpu_gen_rbar_rlar(addr, len, ap_attr, mem_attr, &rbar, &rlar);
    if (ret) {
        return ret;
    }

    ARM_MPU_SetRegion(id, rbar, rlar);
    __DSB();
    __ISB();

    return 0;
}

static uint8_t mpu_alloc_region(void)
{
    uint8_t i;
    uint32_t lock;

    lock = int_lock_global();
    for (i = 0; i < MPU_ID_QTY; i++)
        if ((mpu_region_map & (1 << i)) == 0)
            break;

    if (i >= MPU_ID_QTY) {
        int_unlock_global(lock);
        return MPU_INVALID_ID;
    }

    mpu_region_map |= 1 << i;
    int_unlock_global(lock);
    return i;
}

static void mpu_free_region(uint8_t id)
{
    uint32_t lock;

    if (id >= MPU_ID_QTY)
        return;

    lock = int_lock_global();
    mpu_region_map &= ~(1 << id);
    int_unlock_global(lock);
}

static void mpu_get_region(uint8_t id, uint32_t *rbar, uint32_t *rlar)
{
    MPU->RNR = id;
    *rbar = MPU->RBAR;
    *rlar = MPU->RLAR;
}

static inline void mpu_get_region_addr_range(uint32_t rbar, uint32_t rlar, uint32_t *p_addr, uint32_t *p_end)
{
    *p_addr = rbar & MPU_RBAR_BASE_Msk;
    *p_end = (rlar & MPU_RLAR_LIMIT_Msk) | 0x1F;
}

static int mpu_check_active_addr_range(uint32_t addr, uint32_t end, uint8_t *p_id, uint32_t *p_rbar, uint32_t *p_rlar)
{
    int i;
    uint32_t rbar, rlar;
    uint32_t r_start, r_end;

    for (i = 0; i < MPU_ID_QTY; i++) {
        mpu_get_region(i, &rbar, &rlar);
        if (rlar & MPU_RLAR_EN_Msk) {
            mpu_get_region_addr_range(rbar, rlar, &r_start, &r_end);
            if (r_end < addr || end < r_start) {
                continue;
            }
            if (p_id) {
                *p_id = i;
            }
            if (p_rbar) {
                *p_rbar = rbar;
            }
            if (p_rlar) {
                *p_rlar = rlar;
            }
            if (r_start <= addr && end <= r_end) {
                return 1;
            } else {
                return -1;
            }
        }
    }

    return 0;
}

static int mpu_set_rbar_rlar(uint32_t addr, uint32_t len, enum MPU_ATTR_T attr, uint32_t rbar, uint32_t rlar)
{
    uint8_t id;

    uint8_t r_id;
    uint32_t end;
    uint32_t r_rbar, r_rlar;
    uint32_t r_start, r_end;
    int check;

    end = addr + len - 1;
    r_id = 0;
    r_rbar = r_rlar = 0;

    check = mpu_check_active_addr_range(addr, end, &r_id, &r_rbar, &r_rlar);

    mpu_get_region_addr_range(r_rbar, r_rlar, &r_start, &r_end);

#ifdef MPU_DEBUG
    TRACE(0, "%s: check=%d addr=0x%08X end=0x%08X r_id=%u r_addr=0x%08X r_end=0x%08X",
        __func__, check, addr, end, r_id, r_start, r_end);
#endif

    if (check < 0) {
        ASSERT(false, "%s: Partly overlapped: addr=0x%08X end=0x%08X r_id=%u r_addr=0x%08X r_end=0x%08X",
            __func__, addr, end, r_id, r_start, r_end);
        return -21;
    }

    if (check == 0) {
#ifdef NO_MPU_DEFAULT_MAP
        if (attr == MPU_ATTR_NO_ACCESS) {
            return 0;
        }
#endif

        // Setup a new mpu entry
        id = mpu_alloc_region();
        if (id == MPU_INVALID_ID) {
            return -2;
        }

        mpu_set_region_armv8(id, rbar, rlar);
        return 0;
    }

    // check > 0
#ifdef NO_MPU_DEFAULT_MAP
    uint32_t lock;
    uint32_t saved_ctrl;
    uint8_t id_new[3];
    uint8_t id_cnt;
    int i;

    if (addr == r_start && end == r_end) {
        // Same range
        if (attr == MPU_ATTR_NO_ACCESS) {
            lock = int_lock_global();

            ARM_MPU_ClrRegion(r_id);
            __DSB();
            __ISB();
            mpu_free_region(r_id);

            int_unlock_global(lock);
        } else {
            if (rbar == r_rbar && rlar == r_rlar) {
                // Same attribute
                return 0;
            }
            // Update with new attribute
            mpu_set_region_armv8(r_id, rbar, rlar);
        }

        return 0;
    }

    id_cnt = 2;
    if (addr == r_start || end == r_end) {
        id_cnt--;
    }
    if (attr == MPU_ATTR_NO_ACCESS) {
        id_cnt--;
    }
    id_new[0] = r_id;
    id_new[1] = id_new[2] = MPU_INVALID_ID;
    for (i = 0; i < id_cnt; i++) {
        id_new[i + 1] = mpu_alloc_region();
        if (id_new[i + 1] == MPU_INVALID_ID) {
            TRACE(0, "%s: Failed to alloc id: needed=%u alloc=%u", __func__, id_cnt, i);
            for (int k = 0; k < i; k++) {
                mpu_free_region(id_new[k + 1]);
            }
            return -2;
        }
    }

    lock = int_lock_global();

    // Rollback to default system map (to avoid stack error or exec code error)
    saved_ctrl = MPU->CTRL;
    ARM_MPU_Disable();

    i = 0;
    if (addr != r_start) {
        ASSERT(id_new[i] != MPU_INVALID_ID, "%s-0: Bad id: i=%u id_cnt=%u (0x%08X-0x%08X) r(0x%08X-0x%08X) attr=%d",
            __func__, i, id_cnt, addr, end, r_start, r_end, attr);
        r_rlar = (r_rlar & ~MPU_RLAR_LIMIT_Msk) | ((addr - 1) & MPU_RLAR_LIMIT_Msk);
        // Update the first part with new range
        mpu_set_region_armv8(id_new[i], r_rbar, r_rlar);
        i++;
    }
    if (attr != MPU_ATTR_NO_ACCESS) {
        ASSERT(id_new[i] != MPU_INVALID_ID, "%s-1: Bad id: i=%u id_cnt=%u (0x%08X-0x%08X) r(0x%08X-0x%08X) attr=%d",
            __func__, i, id_cnt, addr, end, r_start, r_end, attr);
        // Update the middle part with new range
        mpu_set_region_armv8(id_new[i], rbar, rlar);
        i++;
    }
    if (end != r_end) {
        ASSERT(id_new[i] != MPU_INVALID_ID, "%s-2: Bad id: i=%u id_cnt=%u (0x%08X-0x%08X) r(0x%08X-0x%08X) attr=%d",
            __func__, i, id_cnt, addr, end, r_start, r_end, attr);
        r_rbar = (r_rbar & ~MPU_RBAR_BASE_Msk) | ((end + 1) & MPU_RBAR_BASE_Msk);
        r_rlar = (r_rlar & ~MPU_RLAR_LIMIT_Msk) | ((r_end) & MPU_RLAR_LIMIT_Msk);
        // Update the last part with new range
        mpu_set_region_armv8(id_new[i], r_rbar, r_rlar);
        i++;
    }

    // Enable the new map
    __DMB();
    MPU->CTRL = saved_ctrl ;
    __DSB();
    __ISB();

    int_unlock_global(lock);

#else
    if (!(addr == r_start && end == r_end)) {
        ASSERT(false, "%s: Partly overlapped: addr=0x%08X end=0x%08X r_id=%u r_addr=0x%08X r_end=0x%08X",
            __func__, addr, end, r_id, r_start, r_end);
        return -22;
    }

    // Same range
    if (rbar == r_rbar && rlar == r_rlar) {
        // Same attribute
        return 0;
    }
    // Update with new attribute
    mpu_set_region_armv8(r_id, rbar, rlar);

#endif

    return 0;
}

int mpu_set(uint32_t addr, uint32_t len, enum MPU_ATTR_T attr, enum MEM_ATTR_T mem_attr)
{
    int ret;
    uint32_t rbar, rlar;

    if (len == 0) {
        return 0;
    }

    ret = mpu_gen_rbar_rlar(addr, len, attr, mem_attr, &rbar, &rlar);
    if (ret) {
        return ret;
    }

    ret = mpu_set_rbar_rlar(addr, len, attr, rbar, rlar);

#ifdef MPU_DEBUG
    mpu_region_dump();
#endif

    return ret;
}

int mpu_clear(uint32_t addr, uint32_t len)
{
    uint8_t id;
    uint32_t end;
    uint32_t r_rbar, r_rlar;
    uint32_t r_start, r_end;
    int check;
    uint32_t lock;
    int ret = 0;

    if (len == 0) {
        return 0;
    }

    end = addr + len - 1;

    lock = int_lock_global();

    check = mpu_check_active_addr_range(addr, end, &id, &r_rbar, &r_rlar);
    if (check > 0) {
        mpu_get_region_addr_range(r_rbar, r_rlar, &r_start, &r_end);
        if (addr == r_start && end == r_end) {
            ARM_MPU_ClrRegion(id);
            __DSB();
            __ISB();
            mpu_free_region(id);
        } else {
            ret = -31;
        }
    } else {
        ret = -32;
    }

    int_unlock_global(lock);

    return ret;
}

BOOT_TEXT_FLASH_LOC
static int mpu_boot_init_table(const mpu_regions_t *mpu_table, uint32_t region_num)
{
    int ret;
    int i;
    const mpu_regions_t *region;
    uint32_t cnt = 0;
    uint32_t flags = 0;

    for (i = 0; i < region_num; i++) {
        region = &mpu_table[i];
        if (region->len == 0) {
            continue;
        }
#ifdef NO_MPU_DEFAULT_MAP
        if (region->ap_attr == MPU_ATTR_NO_ACCESS) {
            continue;
        }
#endif
        if (cnt >= MPU_ID_QTY) {
            goto out;
        }
        ret = mpu_boot_set_armv8(cnt, region->addr, region->len, region->ap_attr, region->mem_attr);
        if (ret)
            goto out;

        cnt++;
    }

#ifndef NO_MPU_DEFAULT_MAP
    // Enable use of the default memory map
    flags |= MPU_CTRL_PRIVDEFENA_Msk;
#endif
    ARM_MPU_Enable(flags);

out:
    return ret;
}

static int mpu_init_table(const mpu_regions_t *mpu_table, uint32_t region_num, int update_map)
{
    int ret = -100;
    int i;
    const mpu_regions_t *region;
    uint32_t failed_map = 0;
    uint32_t cnt = 0;

    if (update_map)
        mpu_region_map = 0;

    for (i = 0; i < region_num; i++) {
        region = &mpu_table[i];
        if (region->len == 0) {
            continue;
        }
#ifdef NO_MPU_DEFAULT_MAP
        if (region->ap_attr == MPU_ATTR_NO_ACCESS) {
            continue;
        }
#endif
        if (cnt >= MPU_ID_QTY) {
            failed_map |= (1 << i);
            continue;
        }
        ret = mpu_set_armv8(cnt, region->addr, region->len, region->ap_attr, region->mem_attr);
        if (ret) {
            failed_map |= (1 << i);
            continue;
        }

        if (update_map)
            mpu_region_map |= 1 << cnt;

        cnt++;
    }

#if defined(SCO_OPTIMIZE_FOR_RAM)
    mpu_regions_t cp_region = {0};
    enum HAL_OVERLAY_ID_T id = hal_overlay_get_curr_id();
    if((!update_map)&&(id != HAL_OVERLAY_ID_QTY)){
        cp_region.addr = hal_overlay_get_text_free_addr(id);
        cp_region.len = hal_overlay_get_text_free_size(id);
        cp_region.ap_attr = MPU_ATTR_READ_WRITE;
        cp_region.mem_attr = MEM_ATTR_INT_SRAM;
        if (cp_region.len == 0) {
            goto mpu_enable_func;
        }
#ifdef NO_MPU_DEFAULT_MAP
        if (cp_region.ap_attr == MPU_ATTR_NO_ACCESS) {
            goto mpu_enable_func;
        }
#endif
        if (cnt >= MPU_ID_QTY) {
            failed_map |= (1 << i);
            goto mpu_enable_func;
        }
        ret = mpu_set_armv8(cnt, cp_region.addr, cp_region.len, cp_region.ap_attr, cp_region.mem_attr);
        if (ret) {
            failed_map |= (1 << i);
        }
    }
mpu_enable_func:
#endif
    // Trace after enabling MPU (to avoid trace buffer attribute error)
    for (i = 0; i < region_num; i++) {
        region = &mpu_table[i];
        TRACE(0, "mpu[%s]: addr=0x%08X len=0x%08X ap_attr=%d mem_attr=%d",
            (failed_map & (1 << i)) ? "KO" : "OK",
            region->addr, region->len, region->ap_attr, region->mem_attr);
    }

#if defined(SCO_OPTIMIZE_FOR_RAM)
    if((!update_map)&&(id != HAL_OVERLAY_ID_QTY)){
        TRACE(0, "mpu[%s]: addr=0x%08X len=0x%08X ap_attr=%d mem_attr=%d",
            (failed_map & (1 << i)) ? "KO" : "OK",
            cp_region.addr, cp_region.len, cp_region.ap_attr, cp_region.mem_attr);
    }
#endif

    mpu_enable();

    return ret;
}

BOOT_TEXT_FLASH_LOC
int mpu_boot_setup(const mpu_regions_t *mpu_table, uint32_t region_num)
{
    int ret;

    ret = mpu_open();
    if (ret)
        return ret;

    ret = mpu_boot_init_table(mpu_table, region_num);
    if (ret)
        return ret;

    return 0;
}

int mpu_ram_region_protect(uint32_t start, uint32_t len, enum MPU_ATTR_T ap_attr)
{
    int ret;
    mpu_disable();
    TRACE(0, "sram_text start %x size %x ap_attr %x", start, len, ap_attr);
    if (!len)
        return 0;

    if ((start % 32 != 0) || (len % 32 != 0))
        ASSERT(0, "sram start %x and len %d must be aligned to 32", start, len);

#ifdef NO_MPU_DEFAULT_MAP
    ret = mpu_set(start, len, MPU_ATTR_NO_ACCESS, MEM_ATTR_INT_SRAM);
#else
    uint8_t id;

    id = mpu_alloc_region();
    if (id == MPU_INVALID_ID)
        return -1;

    ret = mpu_set_armv8(id, start, len, ap_attr, MEM_ATTR_INT_SRAM);
    if (ret) {
        mpu_free_region(id);
    } else {
#ifdef MPU_DEBUG
        mpu_region_dump();
#endif
    }
#endif

    if (ret) {
        TRACE(0, "%s: failed ret=%d", __func__, ret);
    }
    mpu_enable();
    return ret;
}

#ifdef MAIN_RAM_USE_TCM
static int mpu_itcm_text_protect(void)
{
    int ret;
    uint32_t start = RAMX_BASE;
    uint32_t end = (uint32_t)((__fast_sram_text_exec_end__ > __sram_text_end__) ?
        __fast_sram_text_exec_end__ : __sram_text_end__);
    uint32_t len = end - start;

    ret = mpu_set(start, len, MPU_ATTR_READ_EXEC, MEM_ATTR_INT_SRAM);
    if (ret) {
        TRACE(0, "%s: failed ret=%d", __func__, ret);
    }

#ifdef MPU_DEBUG
    mpu_region_dump();
#endif

    return ret;
}
#endif

int mpu_setup(const mpu_regions_t *mpu_table, uint32_t region_num)
{
    int ret;
#if defined(SCO_OPTIMIZE_FOR_RAM)
    mpu_close();
#endif
    ret = mpu_open();
    if (ret)
        return ret;

    ret = mpu_init_table(mpu_table, region_num, 1);
    if (ret)
        return ret;

#ifdef MAIN_RAM_USE_TCM
    ret = mpu_itcm_text_protect();
    if (ret)
        return ret;
#else
    ret = mpu_ram_region_protect(RAMX_TO_RAM((uint32_t)__sram_text_start__),RAMX_TO_RAM((uint32_t)__sram_text_end__)-RAMX_TO_RAM((uint32_t)__sram_text_start__),MPU_ATTR_READ_EXEC);
    if (ret)
        return ret;

    ret = mpu_ram_region_protect(RAMX_TO_RAM((uint32_t)__fast_sram_text_exec_start__),RAMX_TO_RAM((uint32_t)__fast_sram_text_exec_end__)-RAMX_TO_RAM((uint32_t)__fast_sram_text_exec_start__),MPU_ATTR_READ_EXEC);
    if (ret)
        return ret;

    enum HAL_OVERLAY_ID_T id = hal_overlay_get_curr_id();
    if(id != HAL_OVERLAY_ID_QTY){
        ret = mpu_ram_region_protect(hal_overlay_get_text_free_addr(id), hal_overlay_get_text_free_size(id),MPU_ATTR_READ_WRITE);
    }
#endif

    return ret;
}

int mpu_setup_cp(const mpu_regions_t *mpu_table, uint32_t region_num)
{
    int ret;

    ret = mpu_open();
    if (ret) {
        return ret;
    }

    ret = mpu_init_table(mpu_table, region_num, 0);
    return ret;
}

void mpu_region_dump(void)
{
    int i;
    uint32_t rbar, rlar;
    uint32_t r_start, r_end;
    uint8_t sh, ap, xn;
    enum MEM_ATTR_T mem_attr;

    TRACE(0, "MPU-CTRL: 0x%08X", MPU->CTRL);
    for (i = 0; i < MPU_ID_QTY; i++) {
        mpu_get_region(i, &rbar, &rlar);
        if (rlar & MPU_RLAR_EN_Msk) {
            mpu_get_region_addr_range(rbar, rlar, &r_start, &r_end);
            sh = (rbar & MPU_RBAR_SH_Msk) >> MPU_RBAR_SH_Pos;
            ap = (rbar & MPU_RBAR_AP_Msk) >> MPU_RBAR_AP_Pos;
            xn = (rbar & MPU_RBAR_XN_Msk) >> MPU_RBAR_XN_Pos;
            mem_attr = (rlar & MPU_RLAR_AttrIndx_Msk) >> MPU_RLAR_AttrIndx_Pos;
            TRACE(0, "MPU-%u: 0x%08X-0x%08X SH=%X AP=%X XN=%u MEM_ATTR=%u",
                i, r_start, r_end, sh, ap, xn, mem_attr);
        }
    }
}

#ifdef CORE_SLEEP_POWER_DOWN
struct MPU_SAVED_REGS_T {
    uint32_t ctrl;
    uint32_t mair[2];
    uint32_t rbar[MPU_ID_QTY];
    uint32_t rlar[MPU_ID_QTY];
};

static struct MPU_SAVED_REGS_T mpu_saved_regs;
#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
static struct MPU_SAVED_REGS_T mpu_ns_regs;
#endif

void SRAM_TEXT_LOC mpu_sleep(void)
{
    int i;

    mpu_saved_regs.ctrl = MPU->CTRL;
    mpu_saved_regs.mair[0] = MPU->MAIR[0];
    mpu_saved_regs.mair[1] = MPU->MAIR[1];
    for (i = 0; i < MPU_ID_QTY; i++) {
        MPU->RNR = i;
        mpu_saved_regs.rbar[i] = MPU->RBAR;
        mpu_saved_regs.rlar[i] = MPU->RLAR;
    }
#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
    mpu_ns_regs.ctrl = MPU_NS->CTRL;
    mpu_ns_regs.mair[0] = MPU_NS->MAIR[0];
    mpu_ns_regs.mair[1] = MPU_NS->MAIR[1];
    for (int i = 0; i < MPU_ID_QTY; i++) {
        MPU_NS->RNR = i;
        mpu_ns_regs.rbar[i] = MPU_NS->RBAR;
        mpu_ns_regs.rlar[i] = MPU_NS->RLAR;
    }
#endif
}

void SRAM_TEXT_LOC mpu_wakeup(void)
{
    int i;

#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
    for (i = 0; i < MPU_ID_QTY; i++) {
        MPU_NS->RNR = i;
        MPU_NS->RBAR = mpu_ns_regs.rbar[i];
        MPU_NS->RLAR = mpu_ns_regs.rlar[i];
    }
    MPU_NS->MAIR[0] = mpu_ns_regs.mair[0];
    MPU_NS->MAIR[1] = mpu_ns_regs.mair[1];
    MPU_NS->CTRL = mpu_ns_regs.ctrl;
#endif
    for (i = 0; i < MPU_ID_QTY; i++) {
        MPU->RNR = i;
        MPU->RBAR = mpu_saved_regs.rbar[i];
        MPU->RLAR = mpu_saved_regs.rlar[i];
    }
    MPU->MAIR[0] = mpu_saved_regs.mair[0];
    MPU->MAIR[1] = mpu_saved_regs.mair[1];
    MPU->CTRL = mpu_saved_regs.ctrl;
    __DSB();
    __ISB();
}
#endif

#endif
