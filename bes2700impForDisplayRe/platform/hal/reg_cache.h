/***************************************************************************
 *
 * Copyright 2022-2023 BES.
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
#ifndef __REG_CACHE_H__
#define __REG_CACHE_H__

#include "plat_types.h"

struct CACHE_T {
    __IO uint32_t CACHE_EN;             // 0x000
    __IO uint32_t INVALID_ALL;          // 0x004
    __IO uint32_t WRITE_BACK_EN;        // 0x008
    __IO uint32_t WRITE_BUFFER_FLUSH;   // 0x00C
    __IO uint32_t LOCK_UNCACHEABLE;     // 0x010
#if (CHIP_CACHE_VER <= 4)
    __IO uint32_t CLEAN_INVLD_ADDR;     // 0x014
    __IO uint32_t CLEAN_INVLD_TRIG;     // 0x018
#else
    __IO uint32_t CLEAN_INVLD_SET;      // 0x014
    __IO uint32_t CLEAN_INVLD_LINE;     // 0x018
#endif
    __IO uint32_t MONITOR_EN;           // 0x01C
    __IO uint32_t READ_HIT0_L;          // 0x020
    __IO uint32_t READ_HIT0_H;          // 0x024
    __IO uint32_t READ_MISS0_L;         // 0x028
    __IO uint32_t READ_MISS0_H;         // 0x02C
    __IO uint32_t STATUS;               // 0x030
    __IO uint32_t CLEAN_ALL;            // 0x034
    __IO uint32_t CLEAN_INVLD_ALL;      // 0x038
    __IO uint32_t BUS_HALT;             // 0x03C
#if (CHIP_CACHE_VER <= 4)
    __IO uint32_t CLEAN_TRIG;           // 0x040
#else
    __IO uint32_t CLEAN_SET;            // 0x040
#endif
    __IO uint32_t REMAP;                // 0x044
    __IO uint32_t WRITE_HIT0_L;         // 0x048
    __IO uint32_t WRITE_HIT0_H;         // 0x04C
    __IO uint32_t WRITE_MISS0_L;        // 0x050
    __IO uint32_t WRITE_MISS0_H;        // 0x054
    __IO uint32_t READ_HIT1_L;          // 0x058
    __IO uint32_t READ_HIT1_H;          // 0x05C
    __IO uint32_t READ_MISS1_L;         // 0x060
    __IO uint32_t READ_MISS1_H;         // 0x064
    __IO uint32_t WRITE_HIT1_L;         // 0x068
    __IO uint32_t WRITE_HIT1_H;         // 0x06C
    __IO uint32_t WRITE_MISS1_L;        // 0x070
    __IO uint32_t WRITE_MISS1_H;        // 0x074
    __IO uint32_t RESERVED_078[2];      // 0x078
    __IO uint32_t CLEAN_LINE;           // 0x080
    __IO uint32_t INVALID_LINE;         // 0x084
    __IO uint32_t INVALID_SET;          // 0x088
};

// reg_000
#define CACHEC_CACHE_EN                                     (1 << 0)
#define CACHEC_WRAP_EN                                      (1 << 1)
#define CACHEC_READ_ONLY(n)                                 (((n) & 0x3) << 2)
#define CACHEC_READ_ONLY_MASK                               (0x3 << 2)
#define CACHEC_READ_ONLY_SHIFT                              (2)

// reg_004
#define CACHEC_INVALID_ALL                                  (1 << 0)

// reg_008
#if (CHIP_CACHE_VER <= 2)
#define CACHEC_WRITE_BUFFER_EN                              (1 << 0)
#else
#define CACHEC_RESERVE_REG8_BIT0                            (1 << 0)
#endif
// Since best2300
#define CACHEC_WRITE_BACK_EN                                (1 << 1)
#define CACHEC_WRITE_BACK_HALT_ACCESS                       (1 << 2)
// Since V5
#define CACHEC_DISNWAMODE                                   (1 << 3)
#define CACHEC_DOUBLE_LINEFILL                              (1 << 4)

// reg_00c
#define CACHEC_WB_FLUSH                                     (1 << 0)

// reg_010
#define CACHEC_LOCK_UNCACHEABLE                             (1 << 0)

// reg_014
#define CACHEC_CLEAN_AND_INVALID_BY_SET(n)                  (((n) & 0xFFFFFFFF) << 0)
#define CACHEC_CLEAN_AND_INVALID_BY_SET_MASK                (0xFFFFFFFF << 0)
#define CACHEC_CLEAN_AND_INVALID_BY_SET_SHIFT               (0)

// reg_018
#if (CHIP_CACHE_VER <= 4)
#define CACHEC_INVALID_TRIG                                 (1 << 0)
#else
#define CACHEC_CLEAN_AND_INVALID_BY_ADDR(n)                 (((n) & 0xFFFFFFFF) << 0)
#define CACHEC_CLEAN_AND_INVALID_BY_ADDR_MASK               (0xFFFFFFFF << 0)
#define CACHEC_CLEAN_AND_INVALID_BY_ADDR_SHIFT              (0)
#endif

// reg_01c
// Since best2300
#define CACHEC_MONITOR_EN                                   (1 << 0)

// reg_020
// 64 bits when >= V4 and 40 bits when <= V3
#define CACHEC_MONITOR_READ_HIT_0_CNT_31_0(n)               (((n) & 0xFFFFFFFF) << 0)
#define CACHEC_MONITOR_READ_HIT_0_CNT_31_0_MASK             (0xFFFFFFFF << 0)
#define CACHEC_MONITOR_READ_HIT_0_CNT_31_0_SHIFT            (0)

// reg_024
#define CACHEC_MONITOR_READ_HIT_0_CNT_63_32(n)              (((n) & 0xFFFFFFFF) << 0)
#define CACHEC_MONITOR_READ_HIT_0_CNT_63_32_MASK            (0xFFFFFFFF << 0)
#define CACHEC_MONITOR_READ_HIT_0_CNT_63_32_SHIFT           (0)

// reg_028
#define CACHEC_MONITOR_READ_MISS_0_CNT_31_0(n)              (((n) & 0xFFFFFFFF) << 0)
#define CACHEC_MONITOR_READ_MISS_0_CNT_31_0_MASK            (0xFFFFFFFF << 0)
#define CACHEC_MONITOR_READ_MISS_0_CNT_31_0_SHIFT           (0)

// reg_02c
#define CACHEC_MONITOR_READ_MISS_0_CNT_63_32(n)             (((n) & 0xFFFFFFFF) << 0)
#define CACHEC_MONITOR_READ_MISS_0_CNT_63_32_MASK           (0xFFFFFFFF << 0)
#define CACHEC_MONITOR_READ_MISS_0_CNT_63_32_SHIFT          (0)

// reg_030
// Since best2300p
#define FETCHING_0                                          (1 << 0)
#define SM_STATE_0_SHIFT                                    1
#define SM_STATE_0_MASK                                     (0xF << SM_STATE_0_SHIFT)
#define SM_STATE_0(n)                                       BITFIELD_VAL(SM_STATE_0, n)
#define FETCHING_1                                          (1 << 5)
#define SM_STATE_1_SHIFT                                    6
#define SM_STATE_1_MASK                                     (0xF << SM_STATE_1_SHIFT)
#define SM_STATE_1(n)                                       BITFIELD_VAL(SM_STATE_1, n)

// reg_034
// Since best2300p
#define CACHEC_CLEAN_ALL                                    (1 << 0)

// reg_038
// Since V3
#define CACHEC_CLEAN_AND_INVALID_ALL                        (1 << 0)

// reg_03c
// Since V3
#define CACHEC_BUS_HALT_DR                                  (1 << 0)
#define CACHEC_EVB_SNP_CTRL(n)                              (((n) & 0x3) << 1)
#define CACHEC_EVB_SNP_CTRL_MASK                            (0x3 << 1)
#define CACHEC_EVB_SNP_CTRL_SHIFT                           (1)
#define CACHEC_BUSCTRL_SEL_LN                               (1 << 3)
#define CACHEC_WB_EN                                        (1 << 4)
#define CACHEC_TCM_EN                                       (1 << 5)

// reg_040
#if (CHIP_CACHE_VER <= 4)
// Since V3
#define CACHEC_CLEAN_TRIG                                   (1 << 0)
#else
#define CACHEC_CLEAN_BY_SET(n)                              (((n) & 0xFFFFFFFF) << 0)
#define CACHEC_CLEAN_BY_SET_MASK                            (0xFFFFFFFF << 0)
#define CACHEC_CLEAN_BY_SET_SHIFT                           (0)
#endif

// reg_044
#define CACHEC_REMAP(n)                                     (((n) & 0xF) << 0)
#define CACHEC_REMAP_MASK                                   (0xF << 0)
#define CACHEC_REMAP_SHIFT                                  (0)

// reg_048
#define CACHEC_MONITOR_WRITE_HIT_0_CNT_31_0(n)              (((n) & 0xFFFFFFFF) << 0)
#define CACHEC_MONITOR_WRITE_HIT_0_CNT_31_0_MASK            (0xFFFFFFFF << 0)
#define CACHEC_MONITOR_WRITE_HIT_0_CNT_31_0_SHIFT           (0)

// reg_04c
#define CACHEC_MONITOR_WRITE_HIT_0_CNT_63_32(n)             (((n) & 0xFFFFFFFF) << 0)
#define CACHEC_MONITOR_WRITE_HIT_0_CNT_63_32_MASK           (0xFFFFFFFF << 0)
#define CACHEC_MONITOR_WRITE_HIT_0_CNT_63_32_SHIFT          (0)

// reg_050
#define CACHEC_MONITOR_WRITE_MISS_0_CNT_31_0(n)             (((n) & 0xFFFFFFFF) << 0)
#define CACHEC_MONITOR_WRITE_MISS_0_CNT_31_0_MASK           (0xFFFFFFFF << 0)
#define CACHEC_MONITOR_WRITE_MISS_0_CNT_31_0_SHIFT          (0)

// reg_054
#define CACHEC_MONITOR_WRITE_MISS_0_CNT_63_32(n)            (((n) & 0xFFFFFFFF) << 0)
#define CACHEC_MONITOR_WRITE_MISS_0_CNT_63_32_MASK          (0xFFFFFFFF << 0)
#define CACHEC_MONITOR_WRITE_MISS_0_CNT_63_32_SHIFT         (0)

// reg_058
// Since V4
#define CACHEC_MONITOR_READ_HIT_1_CNT_31_0(n)               (((n) & 0xFFFFFFFF) << 0)
#define CACHEC_MONITOR_READ_HIT_1_CNT_31_0_MASK             (0xFFFFFFFF << 0)
#define CACHEC_MONITOR_READ_HIT_1_CNT_31_0_SHIFT            (0)

// reg_05c
#define CACHEC_MONITOR_READ_HIT_1_CNT_63_32(n)              (((n) & 0xFFFFFFFF) << 0)
#define CACHEC_MONITOR_READ_HIT_1_CNT_63_32_MASK            (0xFFFFFFFF << 0)
#define CACHEC_MONITOR_READ_HIT_1_CNT_63_32_SHIFT           (0)

// reg_060
#define CACHEC_MONITOR_READ_MISS_1_CNT_31_0(n)              (((n) & 0xFFFFFFFF) << 0)
#define CACHEC_MONITOR_READ_MISS_1_CNT_31_0_MASK            (0xFFFFFFFF << 0)
#define CACHEC_MONITOR_READ_MISS_1_CNT_31_0_SHIFT           (0)

// reg_064
#define CACHEC_MONITOR_READ_MISS_1_CNT_63_32(n)             (((n) & 0xFFFFFFFF) << 0)
#define CACHEC_MONITOR_READ_MISS_1_CNT_63_32_MASK           (0xFFFFFFFF << 0)
#define CACHEC_MONITOR_READ_MISS_1_CNT_63_32_SHIFT          (0)

// reg_068
#define CACHEC_MONITOR_WRITE_HIT_1_CNT_31_0(n)              (((n) & 0xFFFFFFFF) << 0)
#define CACHEC_MONITOR_WRITE_HIT_1_CNT_31_0_MASK            (0xFFFFFFFF << 0)
#define CACHEC_MONITOR_WRITE_HIT_1_CNT_31_0_SHIFT           (0)

// reg_06c
#define CACHEC_MONITOR_WRITE_HIT_1_CNT_63_32(n)             (((n) & 0xFFFFFFFF) << 0)
#define CACHEC_MONITOR_WRITE_HIT_1_CNT_63_32_MASK           (0xFFFFFFFF << 0)
#define CACHEC_MONITOR_WRITE_HIT_1_CNT_63_32_SHIFT          (0)

// reg_070
#define CACHEC_MONITOR_WRITE_MISS_1_CNT_31_0(n)             (((n) & 0xFFFFFFFF) << 0)
#define CACHEC_MONITOR_WRITE_MISS_1_CNT_31_0_MASK           (0xFFFFFFFF << 0)
#define CACHEC_MONITOR_WRITE_MISS_1_CNT_31_0_SHIFT          (0)

// reg_074
#define CACHEC_MONITOR_WRITE_MISS_1_CNT_63_32(n)            (((n) & 0xFFFFFFFF) << 0)
#define CACHEC_MONITOR_WRITE_MISS_1_CNT_63_32_MASK          (0xFFFFFFFF << 0)
#define CACHEC_MONITOR_WRITE_MISS_1_CNT_63_32_SHIFT         (0)

// reg_080
#define CACHEC_CLEAN_BY_ADDR(n)                             (((n) & 0xFFFFFFFF) << 0)
#define CACHEC_CLEAN_BY_ADDR_MASK                           (0xFFFFFFFF << 0)
#define CACHEC_CLEAN_BY_ADDR_SHIFT                          (0)

// reg_084
#define CACHEC_INVALID_BY_ADDR(n)                           (((n) & 0xFFFFFFFF) << 0)
#define CACHEC_INVALID_BY_ADDR_MASK                         (0xFFFFFFFF << 0)
#define CACHEC_INVALID_BY_ADDR_SHIFT                        (0)

// reg_088
#define CACHEC_INVALID_BY_SET(n)                            (((n) & 0xFFFFFFFF) << 0)
#define CACHEC_INVALID_BY_SET_MASK                          (0xFFFFFFFF << 0)
#define CACHEC_INVALID_BY_SET_SHIFT                         (0)

#endif
