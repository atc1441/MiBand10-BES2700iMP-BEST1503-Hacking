/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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

/*
=========================================================================
    Core Sleep Power Down and Wakeup Power Up Helper Functions
=========================================================================
*/

    #include "cmsis.h"

    .syntax unified

#ifdef CORE_SLEEP_POWER_DOWN

#ifndef ARM_CMNS
#if defined(ROM_BUILD) || defined(PROGRAMMER)
    .section .bss.core_sleep_stack
#else
    .section .sram_bss.core_sleep_stack
#endif
    .align    2
core_sleep_stack:
    .space 4
#endif

#if defined(ROM_BUILD) || defined(PROGRAMMER)
    .section .text.hal_sleep_core_power_down, "ax", %progbits
#else
    .section .sram_text.hal_sleep_core_power_down, "ax", %progbits
#endif
    .thumb
    .thumb_func
    .align    2
    .globl    hal_sleep_core_power_down
    .type    hal_sleep_core_power_down, %function
hal_sleep_core_power_down:

#ifdef ARM_CMNS
    ldr pc, =hal_psc_core_auto_power_down
#else
    push {r4-r12,lr}
    movs r4, r0
    mrs r0, psp
    push {r0,r1}
#ifdef __ARM_ARCH_8M_MAIN__
    mrs r0, msplim
    mrs r1, psplim
    push {r0,r1}
#endif
    mrs r0, primask
    mrs r1, basepri
    mrs r2, faultmask
    mrs r3, control
    push {r0-r3}
#if defined(__ARM_ARCH_8M_MAIN__) && defined(ARM_CMSE)
    mrs r0, msp_ns
    mrs r1, psp_ns
    mrs r2, msplim_ns
    mrs r3, psplim_ns
    push {r0-r3}
    mrs r0, primask_ns
    mrs r1, basepri_ns
    mrs r2, faultmask_ns
    mrs r3, control_ns
    push {r0-r3}
#endif
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    /* TODO: Save FPU registers? */
#endif
    ldr r3, =core_sleep_stack
#ifdef __PIE__
    ldr r0, =hal_sleep_core_power_down
    adr r1, hal_sleep_core_power_down
    sub r1, r0
    add r3, r1
#endif
    str sp, [r3]
    dsb
    movs r0, r4
    bl hal_psc_core_auto_power_down
    b .
#endif

    .pool
    .size    hal_sleep_core_power_down, . - hal_sleep_core_power_down


#ifndef ARM_CMNS
#if defined(ROM_BUILD) || defined(PROGRAMMER)
    .section .text.hal_sleep_core_power_up, "ax", %progbits
#else
    .section .sram_text.hal_sleep_core_power_up, "ax", %progbits
#endif
    .thumb
    .thumb_func
    .align    2
    .globl    hal_sleep_core_power_up
    .type    hal_sleep_core_power_up, %function
hal_sleep_core_power_up:

    ldr r3, =core_sleep_stack
#ifdef __PIE__
    ldr r0, =hal_sleep_core_power_up
    adr r1, hal_sleep_core_power_up
    sub r1, r0
    add r3, r1
#endif
    ldr sp, [r3]
#ifdef __ARM_ARCH_8M_MAIN__
    movs r3, #0
    msr msplim, r3
#endif
    dsb
    isb
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    /* TODO: Restore FPU registers? */
#endif
#if defined(__ARM_ARCH_8M_MAIN__) && defined(ARM_CMSE)
    pop {r0-r3}
    msr primask_ns, r0
    msr basepri_ns, r1
    msr faultmask_ns, r2
    msr control_ns, r3
    pop {r0-r3}
    msr msp_ns, r0
    msr psp_ns, r1
    msr msplim_ns, r2
    msr psplim_ns, r3
#endif
    pop {r0-r3}
    msr primask, r0
    msr basepri, r1
    msr faultmask, r2
    msr control, r3
#ifdef __ARM_ARCH_8M_MAIN__
    pop {r0,r1}
    msr msplim, r0
    msr psplim, r1
#endif
    pop {r0,r1}
    msr psp, r0
    pop {r4-r12,lr}
    bx lr

    .pool
    .size    hal_sleep_core_power_up, . - hal_sleep_core_power_up
#endif

#endif

    .end
