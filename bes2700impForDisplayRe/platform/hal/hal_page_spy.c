/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#include "cmsis_nvic.h"
#include "plat_addr_map.h"
#include "hal_page_spy.h"
#include "hal_sec.h"
#include "hal_trace.h"
#include "reg_page_spy.h"
#include "string.h"

#define MAX_PAGE_SPY_ID                     2

static uint32_t *valid_base;
static uint32_t valid_cnt;
static HAL_PAGE_SPY_IRQ_HANDLER_T spy_handler;

static int page_spy_irq_on(uint32_t spy_base, uint32_t id)
{
    struct PAGE_SPY_T *base = (struct PAGE_SPY_T *)spy_base;

    if (id == 0) {
        return !!(base->REG_018 & PAGE_SPY_HIT_INT0);
    } else {
        return !!(base->REG_024 & PAGE_SPY_HIT_INT1);
    }
}

POSSIBLY_UNUSED
static int proc_page_spy_callback(void)
{
    struct HAL_PAGE_SPY_HIT_INFO_T info;
    uint32_t i;
    uint32_t j;
    int ret = 1;

    memset(&info, 0, sizeof(info));

    for (i = 0; i < valid_cnt; i++) {
        for (j = 0; j < MAX_PAGE_SPY_ID; j++) {
            if (page_spy_irq_on(valid_base[i], j)) {
                hal_page_spy_get_hit_info(valid_base[i], j, &info);
                // Clear IRQ
                hal_page_spy_clear(valid_base[i], j);
            }
        }
    }

    if (spy_handler) {
        ret = spy_handler(&info);
    } else {
        TRACE(0, "*** PAGE-SPY HIT: addr=0x%08X len=0x%08X master_id=%u flags=0x%X", info.addr, info.len, info.master_id, info.flags);
    }

    return ret;
}

POSSIBLY_UNUSED NAKED
static void page_spy_irq_handler(void)
{
    asm volatile (
        "push {r0-r3, r4, lr};"
        "bl proc_page_spy_callback;"
        "cbnz r0, _fault;"
        "pop {r0-r3};"
        "bx lr;"
        "_fault:;"
        // Branch to MemManage_Handler
        "bl NVIC_GetVectorTab;"
        "ldr r1, [r0, #(4*4)];"
        "str r1, [sp, #-4];"
        "pop {r0-r3, r4, lr};"
        "ldr pc, [sp, #-(4*7)];"
    );
}

void hal_page_spy_set_irq_handler(HAL_PAGE_SPY_IRQ_HANDLER_T hdlr)
{
    spy_handler = hdlr;
}

static void get_valid_base(void)
{
    if (!valid_base) {
        hal_sec_get_spy_base_list(&valid_base, &valid_cnt);
        if (!valid_base || !valid_cnt) {
            valid_base = &valid_cnt;
            valid_cnt = 0;
        }
    }
}

static int spy_base_valid(uint32_t spy_base)
{
    uint32_t i;

    get_valid_base();

    for (i = 0; i < valid_cnt; i++) {
        if (spy_base == valid_base[i]) {
            return true;
        }
    }
    return false;
}

int hal_page_spy_set(uint32_t spy_base, uint32_t id, uint32_t start_addr, uint32_t len, enum HAL_PAGE_SPY_FLAG_T flags)
{
    struct PAGE_SPY_T *base = (struct PAGE_SPY_T *)spy_base;
    uint32_t end_addr;
    uint32_t mask;
    uint32_t val;

    if (!spy_base_valid(spy_base)) {
        return 1;
    }
    if (id >= MAX_PAGE_SPY_ID) {
        return 2;
    }

    end_addr = start_addr + len - 1;
    val = 0;

    if (id == 0) {
        mask = PAGE_SPY_REG_ENABLE0 | PAGE_SPY_REG_DETECT_READ0 | PAGE_SPY_REG_DETECT_WRITE0;
        if (flags & (HAL_PAGE_SPY_FLAG_READ | HAL_PAGE_SPY_FLAG_WRITE)) {
            val |= PAGE_SPY_REG_ENABLE0;
            if (flags & HAL_PAGE_SPY_FLAG_READ) {
                val |= PAGE_SPY_REG_DETECT_READ0;
            }
            if (flags & HAL_PAGE_SPY_FLAG_WRITE) {
                val |= PAGE_SPY_REG_DETECT_WRITE0;
            }
            base->REG_008 = start_addr;
            base->REG_00C = end_addr;
        }
    } else {
        mask = PAGE_SPY_REG_ENABLE1 | PAGE_SPY_REG_DETECT_READ1 | PAGE_SPY_REG_DETECT_WRITE1;
        if (flags & (HAL_PAGE_SPY_FLAG_READ | HAL_PAGE_SPY_FLAG_WRITE)) {
            val |= PAGE_SPY_REG_ENABLE1;
            if (flags & HAL_PAGE_SPY_FLAG_READ) {
                val |= PAGE_SPY_REG_DETECT_READ1;
            }
            if (flags & HAL_PAGE_SPY_FLAG_WRITE) {
                val |= PAGE_SPY_REG_DETECT_WRITE1;
            }
            base->REG_010 = start_addr;
            base->REG_014 = end_addr;
        }
    }
    base->REG_004 = (base->REG_004 & ~mask) | val;

#ifdef PAGE_SPY_HAS_IRQ
    if (flags & HAL_PAGE_SPY_FLAG_IRQ) {
        NVIC_SetVector(PAGE_SPY_IRQn, (uint32_t)page_spy_irq_handler);
        NVIC_SetPriority(PAGE_SPY_IRQn, IRQ_PRIORITY_REALTIME);
        NVIC_ClearPendingIRQ(PAGE_SPY_IRQn);
        NVIC_EnableIRQ(PAGE_SPY_IRQn);
    } else {
        NVIC_DisableIRQ(PAGE_SPY_IRQn);
    }
#endif

    return 0;
}

int hal_page_spy_clear(uint32_t spy_base, uint32_t id)
{
    struct PAGE_SPY_T *base = (struct PAGE_SPY_T *)spy_base;
    uint32_t mask;

    if (!spy_base_valid(spy_base)) {
        return 1;
    }
    if (id >= MAX_PAGE_SPY_ID) {
        return 2;
    }

    if (id == 0) {
        mask = PAGE_SPY_REG_ENABLE0 | PAGE_SPY_REG_DETECT_READ0 | PAGE_SPY_REG_DETECT_WRITE0;
    } else {
        mask = PAGE_SPY_REG_ENABLE1 | PAGE_SPY_REG_DETECT_READ1 | PAGE_SPY_REG_DETECT_WRITE1;
    }
    base->REG_004 &= ~mask;
    return 0;
}

int hal_page_spy_hit(uint32_t spy_base, uint32_t id)
{
    struct PAGE_SPY_T *base = (struct PAGE_SPY_T *)spy_base;

    if (!spy_base_valid(spy_base)) {
        return 1;
    }
    if (id >= MAX_PAGE_SPY_ID) {
        return 2;
    }

    if (id == 0) {
        return !!(base->REG_018 & (PAGE_SPY_HIT_READ0 | PAGE_SPY_HIT_WRITE0));
    } else {
        return !!(base->REG_024 & (PAGE_SPY_HIT_READ1 | PAGE_SPY_HIT_WRITE1));
    }
}

int hal_page_spy_get_hit_info(uint32_t spy_base, uint32_t id, struct HAL_PAGE_SPY_HIT_INFO_T *info)
{
    struct PAGE_SPY_T *base = (struct PAGE_SPY_T *)spy_base;
    uint32_t val;
    uint32_t end_addr;

    if (!spy_base_valid(spy_base)) {
        return 1;
    }
    if (id >= MAX_PAGE_SPY_ID) {
        return 2;
    }

    memset(info, 0, sizeof(*info));

    if (id == 0) {
        val = base->REG_018;
        if (val & PAGE_SPY_HIT_READ0) {
            info->flags |= HAL_PAGE_SPY_FLAG_READ;
        }
        if (val & PAGE_SPY_HIT_WRITE0) {
            info->flags |= HAL_PAGE_SPY_FLAG_WRITE;
        }
        info->master_id = GET_BITFIELD(val, PAGE_SPY_HIT_HMASTER0);
        info->addr = base->REG_01C;
        end_addr = base->REG_020;
        if (end_addr > info->addr) {
            info->len = end_addr - info->addr;
        }
    } else {
        val = base->REG_024;
        if (val & PAGE_SPY_HIT_READ1) {
            info->flags |= HAL_PAGE_SPY_FLAG_READ;
        }
        if (val & PAGE_SPY_HIT_WRITE1) {
            info->flags |= HAL_PAGE_SPY_FLAG_WRITE;
        }
        info->master_id = GET_BITFIELD(val, PAGE_SPY_HIT_HMASTER1);
        info->addr = base->REG_028;
        end_addr = base->REG_02C;
        if (end_addr > info->addr) {
            info->len = end_addr - info->addr;
        }
    }

    return 0;
}
