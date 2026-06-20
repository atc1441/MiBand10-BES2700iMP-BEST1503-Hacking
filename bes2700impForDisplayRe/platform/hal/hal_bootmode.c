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
#include "hal_cmu.h"
#include "hal_bootmode.h"
#include "hal_location.h"
#include "cmsis.h"

#ifdef AON_CMU_BASE
#include CHIP_SPECIFIC_HDR(reg_aoncmu)

static struct AONCMU_T * const aoncmu = (struct AONCMU_T *)AON_CMU_BASE;
#else
#include CHIP_SPECIFIC_HDR(reg_cmu)

static struct CMU_T * const cmu = (struct CMU_T *)CMU_BASE;
#endif

// BOOTMODE
#define CMU_BOOTMODE_WATCHDOG       (1 << 0)
#define CMU_BOOTMODE_GLOBAL         (1 << 1)
#define CMU_BOOTMODE_RTC            (1 << 2)
#define CMU_BOOTMODE_CHARGER        (1 << 3)

static union HAL_HW_BOOTMODE_T BOOT_BSS_LOC hw_bm;

__STATIC_FORCEINLINE
volatile uint32_t *get_bootmode_addr(void)
{
#ifdef AON_CMU_BASE
    return &aoncmu->BOOTMODE;
#else
    return &cmu->BOOTMODE;
#endif
}

union HAL_HW_BOOTMODE_T hal_hw_bootmode_get(void)
{
    return hw_bm;
}

union HAL_HW_BOOTMODE_T hal_rom_hw_bootmode_get(void)
{
    union HAL_HW_BOOTMODE_T bm;
    volatile uint32_t *addr;

    addr = get_bootmode_addr();

    bm.reg = *addr & HAL_HW_BOOTMODE_MASK;
    return bm;
}

void BOOT_TEXT_FLASH_LOC hal_hw_bootmode_init(void)
{
    volatile uint32_t *addr;

    addr = get_bootmode_addr();

    hw_bm.reg = *addr & HAL_HW_BOOTMODE_MASK;

#if !(defined(OTA_BOOT_IMAGE) || defined(ARM_CMSE))
    hal_hw_bootmode_clear();
#endif
}

void BOOT_TEXT_FLASH_LOC hal_hw_bootmode_clear(void)
{
    volatile uint32_t *addr;

    addr = get_bootmode_addr();

#if defined(CHIP_BEST3001) || defined(CHIP_BEST3005)
    *addr = (*addr | HAL_HW_BOOTMODE_MASK) >> 2;
#else
    *addr = *addr | HAL_HW_BOOTMODE_MASK;
#endif
}

uint32_t hal_sw_bootmode_get(void)
{
    volatile uint32_t *addr;

    addr = get_bootmode_addr();

    return *addr & HAL_SW_BOOTMODE_MASK;
}

void hal_sw_bootmode_set(uint32_t bm)
{
    uint32_t lock;
    volatile uint32_t *addr;

    addr = get_bootmode_addr();

    lock = int_lock();
#if defined(CHIP_BEST3001) || defined(CHIP_BEST3005)
    *addr = ((*addr | bm) & HAL_SW_BOOTMODE_MASK) >> 2;
#else
    *addr = (*addr | bm) & HAL_SW_BOOTMODE_MASK;
#endif
    int_unlock(lock);
}

void hal_sw_bootmode_clear(uint32_t bm)
{
    uint32_t lock;
    volatile uint32_t *addr;

    addr = get_bootmode_addr();

    lock = int_lock();
#if defined(CHIP_BEST3001) || defined(CHIP_BEST3005)
    *addr = ((*addr & ~bm) & HAL_SW_BOOTMODE_MASK) >> 2;
#else
    *addr = (*addr & ~bm) & HAL_SW_BOOTMODE_MASK;
#endif
    int_unlock(lock);
}

