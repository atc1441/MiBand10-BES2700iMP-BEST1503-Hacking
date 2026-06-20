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
#ifndef __ARM_ARCH_ISA_ARM
#include "plat_addr_map.h"
#include "cmsis.h"
#include "hal_cmu.h"
#include "hal_iomux.h"
#include "hal_trace.h"
#include "hal_uart.h"
#include "reloc_run.h"

extern uint32_t __StackTop[];
extern uint32_t __StackLimit[];
extern uint32_t __rel_dyn_start[];
extern uint32_t __rel_dyn_end[];

#define GET_RELOC_TYPE(t)                   ((t) & 0xFF)

#define RELOC_STACK_SYMBOL

#ifdef RELOC_TRACE
#define RELOC_LOG(n, str, ...)              TRACE_IMM(n, str, ##__VA_ARGS__)
#elif defined(RELOC_PRINTF)
#define RELOC_LOG(n, str, ...)              hal_uart_printf(str, ##__VA_ARGS__)
#else
#define RELOC_LOG(n, str, ...)              TRACE_DUMMY(n, str, ##__VA_ARGS__)
#endif

enum R_ARM_RELOC_T {
    R_ARM_REL32 = 3,
    R_ARM_RELATIVE = 23,
};

struct ELF_RELOC_T {
    uint32_t offset;
    uint32_t info;
};

typedef NORETURN void (*RUN_FUNC)(void);

NORETURN NOINLINE NAKED
static void run_code(int32_t offset, uint32_t sp, uint32_t sp_lim, uint32_t entry)
{
    asm volatile(
        // Update GOT base register
        "add r9, r0;"
        // Update SP
        "msr msp, r1;"
#ifdef __ARM_ARCH_8M_MAIN__
        "msr msplim, r2;"
#endif
        "isb;"
        // Jump to relocated function
        "bx r3;"
        : : );
}

__STATIC_FORCEINLINE
int addr_need_reloc(uint32_t addr, uint32_t reloc_start, uint32_t reloc_end)
{
#if (RAM_BASE != RAM_TO_RAMX(RAM_BASE))
    const uint32_t relocx_start = RAM_TO_RAMX(reloc_start);
    const uint32_t relocx_end = RAM_TO_RAMX(reloc_end);
#endif

    // reloc_end as a boundary varaible should be relocated too
    if ((reloc_start <= addr && addr <= reloc_end) ||
#if (RAM_BASE == RAM_TO_RAMX(RAM_BASE))
            false
#else
            (relocx_start <= addr && addr <= relocx_end)
#endif
            ) {
        return true;
    }
    return false;
}

NORETURN
void reloc_run(const struct RELOC_RUN_CFG_T *cfg)
{
    uint32_t *dst;
    uint32_t *src;
    struct ELF_RELOC_T *reloc;
    struct ELF_RELOC_T *new_reloc;
    uint32_t sym_addr;

    RELOC_LOG(0, "RELOC_RUN_CFG:");
    RELOC_LOG(0, "  reloc_start = %08X", cfg->reloc_start);
    RELOC_LOG(0, "  reloc_end   = %08X", cfg->reloc_end);
    RELOC_LOG(0, "  offset      = %08X", cfg->offset);
    RELOC_LOG(0, "  new_sp      = %08X", cfg->new_sp);
    RELOC_LOG(0, "  new_sp_lim  = %08X", cfg->new_sp_lim);
    RELOC_LOG(0, "  new_entry   = %08X", cfg->new_entry);

    // Copy code and data
    dst = (uint32_t *)(cfg->reloc_start + cfg->offset);
    src = (uint32_t *)(cfg->reloc_start);

    for (; src < (uint32_t *)cfg->reloc_end; dst++, src++) {
        *dst = *src;
    }

    // Relocate symbols
    RELOC_LOG(0, "Relocating:");
    reloc = (struct ELF_RELOC_T *)__rel_dyn_start;
    for (; reloc < (struct ELF_RELOC_T *)__rel_dyn_end; reloc++) {
        if (GET_RELOC_TYPE(reloc->info) == R_ARM_RELATIVE) {
            if (addr_need_reloc(reloc->offset, cfg->reloc_start, cfg->reloc_end)) {
                new_reloc = (struct ELF_RELOC_T *)((uint32_t)reloc + cfg->offset);
                new_reloc->offset += cfg->offset;
                sym_addr = *(uint32_t *)reloc->offset;
                if (0) {
                } else if (addr_need_reloc(sym_addr, cfg->reloc_start, cfg->reloc_end)) {
                    *(uint32_t *)(reloc->offset + cfg->offset) = sym_addr + cfg->offset;
                    RELOC_LOG(0, "  [%08X] %08X/%08X: %08X -> %08X", (uint32_t)reloc, reloc->offset, reloc->info, sym_addr, (sym_addr + cfg->offset));
#ifdef RELOC_STACK_SYMBOL
                } else if (sym_addr == (uint32_t)__StackTop) {
                    *(uint32_t *)(reloc->offset + cfg->offset) = cfg->new_sp;
                    RELOC_LOG(0, "  [%08X] %08X/%08X: %08X -> %08X (SP)", (uint32_t)reloc, reloc->offset, reloc->info, sym_addr, cfg->new_sp);
                } else if (sym_addr == (uint32_t)__StackLimit) {
                    // NOTE:
                    // If (__StackLimit == cfg->reloc_end), cfg->reloc_end will take precedence
                    *(uint32_t *)(reloc->offset + cfg->offset) = cfg->new_sp_lim;
                    RELOC_LOG(0, "  [%08X] %08X/%08X: %08X -> %08X (SPLIM)", (uint32_t)reloc, reloc->offset, reloc->info, sym_addr, cfg->new_sp_lim);
#endif
                } else {
                    RELOC_LOG(0, "  [%08X] %08X/%08X: %08X: Addr out of range", (uint32_t)reloc, reloc->offset, reloc->info, sym_addr);
                }
            } else {
                RELOC_LOG(0, "  [%08X] %08X/%08X: Offset out of range", (uint32_t)reloc, reloc->offset, reloc->info);
            }
        } else {
            RELOC_LOG(0, "  [%08X] %08X/%08X: Bad type", (uint32_t)reloc, reloc->offset, reloc->info);
            hal_cmu_simu_set_val(0xEEEE6E10);
#if 0
            hal_iomux_set_analog_i2c();
            hal_cmu_jtag_clock_enable();
            hal_iomux_set_jtag();
#endif
            SAFE_PROGRAM_STOP();
        }
    }

    run_code(cfg->offset, cfg->new_sp, cfg->new_sp_lim, cfg->new_entry);
}
#endif
