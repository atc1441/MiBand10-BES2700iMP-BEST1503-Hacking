/***************************************************************************
 *
 * Copyright 2020-2022 BES.
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
#ifndef __REG_PAGE_SPY_H__
#define __REG_PAGE_SPY_H__

#include "plat_types.h"

struct PAGE_SPY_T {
    __IO uint32_t REG_000;
    __IO uint32_t REG_004;
    __IO uint32_t REG_008;
    __IO uint32_t REG_00C;
    __IO uint32_t REG_010;
    __IO uint32_t REG_014;
    __IO uint32_t REG_018;
    __IO uint32_t REG_01C;
    __IO uint32_t REG_020;
    __IO uint32_t REG_024;
    __IO uint32_t REG_028;
    __IO uint32_t REG_02C;
};

// reg_00
#ifdef CHIP_BEST2003
#define PAGE_SPY_CFG_NONSEC_BYPASS                          (1 << 0)
#define PAGE_SPY_IRQ_EN_RD                                  (1 << 1)
#define PAGE_SPY_IRQ_EN_WR                                  (1 << 2)
#define PAGE_SPY_CFG_SEC_RESP                               (1 << 3)
#define PAGE_SPY_CFG_INIT_VALUE                             (1 << 4)
#else
#define PAGE_SPY_CFG_AP                                     (1 << 0)
#define PAGE_SPY_CFG_NONSEC                                 (1 << 1)
#define PAGE_SPY_CFG_SEC_RESP                               (1 << 2)
#define PAGE_SPY_CFG_NONSEC_BYPASS                          (1 << 3)
#define PAGE_SPY_CFG_SEC2NSEC                               (1 << 4)
#define PAGE_SPY_PPC_IRQ_CLEAR                              (1 << 5)
#define PAGE_SPY_PPC_IRQ_ENABLE_RD                          (1 << 6)
#define PAGE_SPY_PPC_IRQ_ENABLE_WR                          (1 << 7)
#define PAGE_SPY_CFG_INIT_VALUE                             (1 << 8)
#endif

// reg_04
#define PAGE_SPY_REG_ENABLE0                                (1 << 0)
#define PAGE_SPY_REG_DETECT_READ0                           (1 << 1)
#define PAGE_SPY_REG_DETECT_WRITE0                          (1 << 2)
#define PAGE_SPY_REG_ENABLE1                                (1 << 3)
#define PAGE_SPY_REG_DETECT_READ1                           (1 << 4)
#define PAGE_SPY_REG_DETECT_WRITE1                          (1 << 5)

// reg_08
#define PAGE_SPY_REG_STR_ADDR0(n)                           (((n) & 0xFFFFFFFF) << 0)
#define PAGE_SPY_REG_STR_ADDR0_MASK                         (0xFFFFFFFF << 0)
#define PAGE_SPY_REG_STR_ADDR0_SHIFT                        (0)

// reg_0c
#define PAGE_SPY_REG_END_ADDR0(n)                           (((n) & 0xFFFFFFFF) << 0)
#define PAGE_SPY_REG_END_ADDR0_MASK                         (0xFFFFFFFF << 0)
#define PAGE_SPY_REG_END_ADDR0_SHIFT                        (0)

// reg_10
#define PAGE_SPY_REG_STR_ADDR1(n)                           (((n) & 0xFFFFFFFF) << 0)
#define PAGE_SPY_REG_STR_ADDR1_MASK                         (0xFFFFFFFF << 0)
#define PAGE_SPY_REG_STR_ADDR1_SHIFT                        (0)

// reg_14
#define PAGE_SPY_REG_END_ADDR1(n)                           (((n) & 0xFFFFFFFF) << 0)
#define PAGE_SPY_REG_END_ADDR1_MASK                         (0xFFFFFFFF << 0)
#define PAGE_SPY_REG_END_ADDR1_SHIFT                        (0)

// reg_18
#define PAGE_SPY_HIT_INT0                                   (1 << 0)
#define PAGE_SPY_HIT_READ0                                  (1 << 1)
#define PAGE_SPY_HIT_WRITE0                                 (1 << 2)
#define PAGE_SPY_HIT_HMASTER0(n)                            (((n) & 0x7FFF) << 3)
#define PAGE_SPY_HIT_HMASTER0_MASK                          (0x7FFF << 3)
#define PAGE_SPY_HIT_HMASTER0_SHIFT                         (3)

// reg_1c
#define PAGE_SPY_HIT_ADDR0_B(n)                             (((n) & 0xFFFFFFFF) << 0)
#define PAGE_SPY_HIT_ADDR0_B_MASK                           (0xFFFFFFFF << 0)
#define PAGE_SPY_HIT_ADDR0_B_SHIFT                          (0)

// reg_20
#define PAGE_SPY_HIT_ADDR0_E(n)                             (((n) & 0xFFFFFFFF) << 0)
#define PAGE_SPY_HIT_ADDR0_E_MASK                           (0xFFFFFFFF << 0)
#define PAGE_SPY_HIT_ADDR0_E_SHIFT                          (0)

// reg_24
#define PAGE_SPY_HIT_INT1                                   (1 << 0)
#define PAGE_SPY_HIT_READ1                                  (1 << 1)
#define PAGE_SPY_HIT_WRITE1                                 (1 << 2)
#define PAGE_SPY_HIT_HMASTER1(n)                            (((n) & 0x7FFF) << 3)
#define PAGE_SPY_HIT_HMASTER1_MASK                          (0x7FFF << 3)
#define PAGE_SPY_HIT_HMASTER1_SHIFT                         (3)

// reg_28
#define PAGE_SPY_HIT_ADDR1_B(n)                             (((n) & 0xFFFFFFFF) << 0)
#define PAGE_SPY_HIT_ADDR1_B_MASK                           (0xFFFFFFFF << 0)
#define PAGE_SPY_HIT_ADDR1_B_SHIFT                          (0)

// reg_2c
#define PAGE_SPY_HIT_ADDR1_E(n)                             (((n) & 0xFFFFFFFF) << 0)
#define PAGE_SPY_HIT_ADDR1_E_MASK                           (0xFFFFFFFF << 0)
#define PAGE_SPY_HIT_ADDR1_E_SHIFT                          (0)

#endif
