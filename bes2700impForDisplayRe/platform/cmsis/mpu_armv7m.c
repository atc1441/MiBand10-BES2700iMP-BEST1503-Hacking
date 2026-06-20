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
#ifdef __ARM_ARCH_7EM__

#include "mpu.h"
#include "cmsis.h"
#include "hal_location.h"
#include "hal_trace.h"

static int mpu_region_map;

int mpu_open(void)
{
    int i;

    if ((MPU->TYPE & MPU_TYPE_DREGION_Msk) == 0) {
        return -1;
    }

    ARM_MPU_Disable();

    for (i = 0; i < MPU_ID_QTY; i++) {
        ARM_MPU_ClrRegion(i);
    }

    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);

    return 0;
}

int mpu_close(void)
{
    ARM_MPU_Disable();

    return 0;
}

static int mpu_set_armv7(uint8_t id, uint32_t addr, uint32_t len,
                                        int srd_bits, enum MPU_ATTR_T attr)
{
    int ret;
    uint32_t rbar;
    uint32_t rasr;
    uint8_t xn;
    uint8_t ap;
    uint8_t size;

    if ((MPU->CTRL & MPU_CTRL_ENABLE_Msk) == 0) {
        ret = mpu_open();
        if (ret) {
            return ret;
        }
    }

    if (id >= MPU_ID_QTY) {
        return -2;
    }
    if (len < 32 || (len & (len - 1))) {
        return -3;
    }
    if (addr & (len - 1)) {
        return -4;
    }
    if (attr >= MPU_ATTR_QTY) {
        return -5;
    }

    if (attr == MPU_ATTR_READ_WRITE_EXEC || attr == MPU_ATTR_READ_EXEC ||
            attr == MPU_ATTR_EXEC) {
        xn = 0;
    } else {
        xn = 1;
    }

    ap = ARM_MPU_AP_NONE;
    if (attr == MPU_ATTR_READ_WRITE_EXEC || attr == MPU_ATTR_READ_WRITE) {
        ap = ARM_MPU_AP_FULL;
    } else if (attr == MPU_ATTR_READ_EXEC || attr == MPU_ATTR_READ) {
        ap = ARM_MPU_AP_RO;
    }

    size = __CLZ(__RBIT(len)) - 1;

    ARM_MPU_Disable();
    rbar = ARM_MPU_RBAR(id, addr);
    // Type Extention: 0
    // Shareable: 1
    // Cacheable: 1
    // Bufferable: 1
    // Subregion: 0
    rasr = ARM_MPU_RASR(xn, ap, 0, 1, 1, 1, srd_bits, size);

    ARM_MPU_SetRegion(rbar, rasr);
    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);
    __DSB();
    __ISB();

    return 0;
}

static uint8_t mpu_alloc_region(void)
{
    uint8_t i;
    uint32_t lock;

    lock = int_lock();
    for (i = 0; i < MPU_ID_QTY; i++)
        if ((mpu_region_map & (1 << i)) == 0)
            break;

    if (i >= MPU_ID_QTY) {
        int_unlock(lock);
        return MPU_INVALID_ID;
    }

    mpu_region_map |= 1 << i;
    int_unlock(lock);
    return i;
}

static void mpu_free_region(uint8_t id)
{
    uint32_t lock;

    if (id >= MPU_ID_QTY)
        return;

    lock = int_lock();
    mpu_region_map &= ~(1 << id);
    int_unlock(lock);
}

/*
 * sub region is 8 bits, and if one bits is 1, the sub resion will be
 * disabled
 * like 0b00011111, the top 3 sub region will be disabled
 * if it is 0b11110000, the bottom 4 sub region will be disabled.
 */
static int mpu_get_top_srd(uint32_t rg_end, uint32_t fr_end,
                                                    uint32_t sub_rg_sz)
{
    int dis_nums;
    uint8_t srd_bits;

    dis_nums = (rg_end - fr_end) / sub_rg_sz;
    if ((fr_end & (sub_rg_sz - 1)) != 0)
        dis_nums += 1;

    srd_bits = 0xff;
    srd_bits &= ~((1UL << (8 - dis_nums)) - 1);

    return srd_bits;
}

static int mpu_get_bottom_srd(uint32_t rg_start, uint32_t fr_start,
                                                    uint32_t sub_rg_sz)
{
    int dis_nums;
    uint8_t srd_bits;

    dis_nums = (fr_start - rg_start) / sub_rg_sz;
    srd_bits = 0xff;
    srd_bits &= ((1UL << dis_nums) - 1);

    return srd_bits;
}

static int calc_sub_region_size(uint32_t fr_start)
{
    int lsb_bit;
    uint32_t sz;

    //sub region size aligned to fram start
    lsb_bit = get_lsb_pos(fr_start);
    sz = 1 << lsb_bit;
    if (sz < 0x80) {
        /*cortex-m4 doesn't support sub region size less than 128 */
        TRACE(1,"no mpu region for fram start %x", fr_start);
        return -1;
    }
    if (sz > 0x4000)
        sz = 0x4000;


    return sz;
}

/*
Allocate two mpu sections to protect the fast ram.  The cortext M4
mpu has a lot of restrict, such as one region's start address must
be aligned to the size of region, and the sub region number is fixed
to 8.
The layout like:
          ------------------
          | sub region 8   |
          ------------------
          | sub region 7   |
          | .....          |
          |                |
          ------------------>fast_ram_end
          |////////////////|
          |////////////////|
          |////////////////|
          |////////////////|
          ------------------>mpu region2
          |//sub region 8//|
          |////////////////|
          |////////////////|
          |////////////////|
          ------------------>fast_ram_start aligned to sub region size
          | .....          |
          |                |
          |sub region 1    |
          ------------------
          |sub region 0    |
          ------------------>mpu region1

If the __fast_sram_text_exec_end__ exceed the region2's end, just leave
the rest unprotect.
*/
static int mpu_fram_protect_armv7(uint32_t fr_start, uint32_t fr_end)
{
    uint32_t fr_sz;
    uint32_t sub_rg_sz;
    uint32_t rg_sz;
    uint32_t rg_start;
    uint32_t rg_end;
    int ret = 0;
    int srd_bits;
    int finished = 0;
    uint8_t id;

    fr_sz = fr_end - fr_start;
    sub_rg_sz = calc_sub_region_size(fr_start);
    if (sub_rg_sz < 0)
        return -1;

    /* according to cortex-m4 spec, the region is divived to 8 sub region*/
    rg_sz = sub_rg_sz * 8;
    /*now we just protect two region */
    if (fr_sz > (rg_sz * 2)) {
        TRACE(0,"Warning fram is too big, mpu can not protect so much");
        TRACE(2,"region_sz %x, fram size %x", rg_sz, fr_sz);
    }

    /*aliged the region start to region size according to cortext m4 spec*/
    srd_bits = 0;
    rg_start = fr_start & ~(rg_sz - 1);
    if (fr_start > rg_start) {
        int b_srd_bits = 0;

        b_srd_bits = mpu_get_bottom_srd(rg_start, fr_start, sub_rg_sz);
        srd_bits |= b_srd_bits;
    }

    rg_end = rg_start + rg_sz;
    if (fr_end < rg_end) {
        int t_srd_bits = 0;

        t_srd_bits = mpu_get_top_srd(rg_end, fr_end, sub_rg_sz);
        srd_bits |= t_srd_bits;
        finished = 1;
    }

    id = mpu_alloc_region();
    if (id == MPU_INVALID_ID) {
        ret = -2;
        goto out;
    }

    ret = mpu_set_armv7(id, rg_start, rg_sz, srd_bits, MPU_ATTR_READ);
    if (ret || finished)
        goto out;

    /* need another section, and start from next region*/
    rg_start += rg_sz;

    srd_bits = 0;
    rg_end = rg_start + rg_sz;
    if (fr_end < rg_end) {
        srd_bits = mpu_get_top_srd(rg_end, fr_end, sub_rg_sz);
    }

    id = mpu_alloc_region();
    if (id == MPU_INVALID_ID) {
        ret = -2;
        goto out;
    }
    ret = mpu_set_armv7(id, rg_start, rg_sz, srd_bits, MPU_ATTR_READ_EXEC);
    /* if fr_end large than two section, just pass */
out:
    return ret;
}

static int mpu_find_region(uint32_t addr, uint32_t len, uint8_t *p_id)
{
    uint32_t rbar, rasr;
    uint32_t r_addr, r_size;
    uint32_t size;
    int i;

    size = __CLZ(__RBIT(len)) - 1;

    for (i = 0; i < MPU_ID_QTY; i++) {
        MPU->RNR = i;
        rasr = MPU->RASR;
        if ((rasr & MPU_RASR_ENABLE_Msk) == 0) {
            continue;
        }
        rbar = MPU->RBAR;

        r_addr = rbar & MPU_RBAR_ADDR_Msk;
        r_size = (rasr & MPU_RASR_SIZE_Msk) >> MPU_RASR_SIZE_Pos;
        if (addr == r_addr && size == r_size) {
            *p_id = i;
            return 1;
        }
    }

    return 0;
}

int mpu_set(uint32_t addr, uint32_t len, enum MPU_ATTR_T attr, enum MEM_ATTR_T mem_attr)
{
    int found;
    int ret;
    uint8_t id = MPU_INVALID_ID;

    found = mpu_find_region(addr, len, &id);
    if (!found) {
        id = mpu_alloc_region();
        if (id == MPU_INVALID_ID) {
            return -2;
        }
    }

    ret = mpu_set_armv7(id, addr, len, 0, attr);
    if (ret && !found) {
        mpu_free_region(id);
    }

    return ret;
}

int mpu_clear(uint32_t addr, uint32_t len)
{
    uint32_t lock;
    int found;
    uint8_t id;

    lock = int_lock();

    found = mpu_find_region(addr, len, &id);
    if (found) {
        ARM_MPU_ClrRegion(id);
        __DSB();
        __ISB();
        mpu_free_region(id);
    }

    int_unlock(lock);

    return !found;
}

extern uint32_t __fast_sram_text_exec_start__[];
extern uint32_t __fast_sram_text_exec_end__[];

static int mpu_fast_ram_protect(void)
{
    uint32_t ramx_start  = (uint32_t)__fast_sram_text_exec_start__;
    uint32_t ramx_end = (uint32_t)__fast_sram_text_exec_end__;
    uint32_t ram_start = RAMX_TO_RAM(ramx_start);
    uint32_t ram_end = RAMX_TO_RAM(ramx_end);

    return mpu_fram_protect_armv7(ram_start, ram_end);
}

#if 0
int mpu_fram_test(void)
{
    uint32_t ramx_start  = (uint32_t)__fast_sram_text_exec_start__;
    uint32_t ram_start = RAMX_TO_RAM(ramx_start);
    uint32_t test_start =  ram_start - 1024;

    for (int i = 0; i < 1024; i++) {
        TRACE(1,"test_start %x", test_start);
        *(volatile uint32_t *)test_start = 0x1;
        test_start += 128;
    }

    uint32_t ramx_end = (uint32_t)__fast_sram_text_exec_end__;
    uint32_t ram_end = RAMX_TO_RAM(ramx_end);
    uint32_t test_end =  ram_end + 1024;
    for (int i = 0; i < 1024; i++) {
        TRACE(1,"test_end %x", test_end);
        *(volatile uint32_t *)test_end = 0x1;
        test_end -= 128;
    }

    return 0;
}
#endif

static int mpu_init_table(const mpu_regions_t *mpu_table, uint32_t region_num, int cp)
{
    int ret;
    int i;
    uint32_t lock;

    if (region_num > MPU_ID_QTY) {
        return -1;
    }

    lock = int_lock();
    for (i = 0; i < region_num; i++) {
        const mpu_regions_t *region;
        region = &mpu_table[i];
        ret = mpu_set_armv7(i, region->addr, region->len, 0, region->ap_attr);
        if (ret)
            goto out;

        if (!cp)
            mpu_region_map |= 1 << i;
    }

out:
    int_unlock(lock);
    return ret;
}

int mpu_setup(const mpu_regions_t *mpu_table, uint32_t region_num)
{
    int ret;

    ret = mpu_open();
    if (ret)
        return ret;

    ret = mpu_init_table(mpu_table, region_num, 0);
    if (ret)
        return ret;

    ret = mpu_fast_ram_protect();
    return ret;
}

int mpu_setup_cp(const mpu_regions_t *mpu_table, uint32_t region_num)
{
    int ret;

    ret = mpu_open();
    if (ret) {
        return ret;
    }

    ret = mpu_init_table(mpu_table, region_num, 1);
    return ret;
}

#ifdef CORE_SLEEP_POWER_DOWN
void SRAM_TEXT_LOC mpu_sleep(void)
{
}

void SRAM_TEXT_LOC mpu_wakeup(void)
{
}
#endif

#endif
