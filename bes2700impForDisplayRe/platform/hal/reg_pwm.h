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
#ifndef __REG_PWM_H__
#define __REG_PWM_H__

#include "plat_types.h"

// PWM Registers
struct PWM_T
{
    __I  uint32_t ID;                       // 0x000
    __IO uint32_t EN;                       // 0x004
    __IO uint32_t INV;                      // 0x008
    __IO uint32_t PHASE01;                  // 0x00C
    __IO uint32_t PHASE23;                  // 0x010
    __IO uint32_t LOAD01;                   // 0x014
    __IO uint32_t LOAD23;                   // 0x018
    __IO uint32_t TOGGLE01;                 // 0x01C
    __IO uint32_t TOGGLE23;                 // 0x020
#if !(defined(CHIP_BEST2003))
    __IO uint32_t PHASEMOD;                 // 0x024
    __IO uint32_t ST1_23;                   // 0x028
    __IO uint32_t TWINKLE23;                // 0x02C
#else
    __IO uint32_t ST01;                     // 0x024
    __IO uint32_t ST23;                     // 0x028
    __IO uint32_t TWINKLE01;                // 0x02C
    __IO uint32_t TWINKLE23;                // 0x030
#endif
};

#define PWM_EN_0                            (1 << 0)
#define PWM_EN_1                            (1 << 1)
#define PWM_EN_2                            (1 << 2)
#define PWM_EN_3                            (1 << 3)

#define PWM_INV_0                           (1 << 0)
#define PWM_INV_1                           (1 << 1)
#define PWM_INV_2                           (1 << 2)
#define PWM_INV_3                           (1 << 3)

#define PWM_PHASE01_0(n)                    (((n) & 0xFFFF) << 0)
#define PWM_PHASE01_0_MASK                  (0xFFFF << 0)
#define PWM_PHASE01_0_SHIFT                 (0)
#define PWM_PHASE01_1(n)                    (((n) & 0xFFFF) << 16)
#define PWM_PHASE01_1_MASK                  (0xFFFF << 16)
#define PWM_PHASE01_1_SHIFT                 (16)

#define PWM_PHASE23_2(n)                    (((n) & 0xFFFF) << 0)
#define PWM_PHASE23_2_MASK                  (0xFFFF << 0)
#define PWM_PHASE23_2_SHIFT                 (0)
#define PWM_PHASE23_3(n)                    (((n) & 0xFFFF) << 16)
#define PWM_PHASE23_3_MASK                  (0xFFFF << 16)
#define PWM_PHASE23_3_SHIFT                 (16)

#define PWM_LOAD01_0(n)                     (((n) & 0xFFFF) << 0)
#define PWM_LOAD01_0_MASK                   (0xFFFF << 0)
#define PWM_LOAD01_0_SHIFT                  (0)
#define PWM_LOAD01_1(n)                     (((n) & 0xFFFF) << 16)
#define PWM_LOAD01_1_MASK                   (0xFFFF << 16)
#define PWM_LOAD01_1_SHIFT                  (16)

#define PWM_LOAD23_2(n)                     (((n) & 0xFFFF) << 0)
#define PWM_LOAD23_2_MASK                   (0xFFFF << 0)
#define PWM_LOAD23_2_SHIFT                  (0)
#define PWM_LOAD23_3(n)                     (((n) & 0xFFFF) << 16)
#define PWM_LOAD23_3_MASK                   (0xFFFF << 16)
#define PWM_LOAD23_3_SHIFT                  (16)

#define PWM_TOGGLE01_0(n)                   (((n) & 0xFFFF) << 0)
#define PWM_TOGGLE01_0_MASK                 (0xFFFF << 0)
#define PWM_TOGGLE01_0_SHIFT                (0)
#define PWM_TOGGLE01_1(n)                   (((n) & 0xFFFF) << 16)
#define PWM_TOGGLE01_1_MASK                 (0xFFFF << 16)
#define PWM_TOGGLE01_1_SHIFT                (16)

#define PWM_TOGGLE23_2(n)                   (((n) & 0xFFFF) << 0)
#define PWM_TOGGLE23_2_MASK                 (0xFFFF << 0)
#define PWM_TOGGLE23_2_SHIFT                (0)
#define PWM_TOGGLE23_3(n)                   (((n) & 0xFFFF) << 16)
#define PWM_TOGGLE23_3_MASK                 (0xFFFF << 16)
#define PWM_TOGGLE23_3_SHIFT                (16)

#define PWM_PHASEMOD_0                      (1 << 0)
#define PWM_PHASEMOD_1                      (1 << 1)
#define PWM_PHASEMOD_2                      (1 << 2)
#define PWM_PHASEMOD_3                      (1 << 3)

#if !(defined(CHIP_BEST2003))
#define SUBCNT_DATA2_SHIFT                  0
#define SUBCNT_DATA2_MASK                   (0xFF << SUBCNT_DATA2_SHIFT)
#define SUBCNT_DATA2(n)                     BITFIELD_VAL(SUBCNT_DATA2, n)
#define SUBCNT_DATA3_SHIFT                  8
#define SUBCNT_DATA3_MASK                   (0xFF << SUBCNT_DATA3_SHIFT)
#define SUBCNT_DATA3(n)                     BITFIELD_VAL(SUBCNT_DATA3, n)
#define TG_SUBCNT_D2_ST_SHIFT               16
#define TG_SUBCNT_D2_ST_MASK                (0x7F << TG_SUBCNT_D2_ST_SHIFT)
#define TG_SUBCNT_D2_ST(n)                  BITFIELD_VAL(TG_SUBCNT_D2_ST, n)
#define REG_PWM2_BR_EN                      (1 << 23)
#define TG_SUBCNT_D3_ST_SHIFT               24
#define TG_SUBCNT_D3_ST_MASK                (0x7F << TG_SUBCNT_D3_ST_SHIFT)
#define TG_SUBCNT_D3_ST(n)                  BITFIELD_VAL(TG_SUBCNT_D3_ST, n)
#define REG_PWM3_BR_EN                      (1 << 31)

#define REG_PWM2_ST1_SHIFT                  0
#define REG_PWM2_ST1_MASK                   (0xFFFF << REG_PWM2_ST1_SHIFT)
#define REG_PWM2_ST1(n)                     BITFIELD_VAL(REG_PWM2_ST1, n)
#define REG_PWM3_ST1_SHIFT                  16
#define REG_PWM3_ST1_MASK                   (0xFFFF << REG_PWM3_ST1_SHIFT)
#define REG_PWM3_ST1(n)                     BITFIELD_VAL(REG_PWM3_ST1, n)

#else

#define SUBCNT_DATA0_SHIFT                  0
#define SUBCNT_DATA0_MASK                   (0xFF << SUBCNT_DATA0_SHIFT)
#define SUBCNT_DATA0(n)                     BITFIELD_VAL(SUBCNT_DATA0, n)
#define SUBCNT_DATA1_SHIFT                  16
#define SUBCNT_DATA1_MASK                   (0xFF << SUBCNT_DATA1_SHIFT)
#define SUBCNT_DATA1(n)                     BITFIELD_VAL(SUBCNT_DATA1, n)
#define SUBCNT_DATA2_SHIFT                  0
#define SUBCNT_DATA2_MASK                   (0xFF << SUBCNT_DATA2_SHIFT)
#define SUBCNT_DATA2(n)                     BITFIELD_VAL(SUBCNT_DATA2, n)
#define SUBCNT_DATA3_SHIFT                  16
#define SUBCNT_DATA3_MASK                   (0xFF << SUBCNT_DATA3_SHIFT)
#define SUBCNT_DATA3(n)                     BITFIELD_VAL(SUBCNT_DATA3, n)

#define TG_SUBCNT_D0_ST_SHIFT               8
#define TG_SUBCNT_D0_ST_MASK                (0x7F << TG_SUBCNT_D0_ST_SHIFT)
#define TG_SUBCNT_D0_ST(n)                  BITFIELD_VAL(TG_SUBCNT_D0_ST, n)
#define REG_PWM0_BR_EN                      (1 << 15)
#define TG_SUBCNT_D1_ST_SHIFT               24
#define TG_SUBCNT_D1_ST_MASK                (0x7F << TG_SUBCNT_D1_ST_SHIFT)
#define TG_SUBCNT_D1_ST(n)                  BITFIELD_VAL(TG_SUBCNT_D1_ST, n)
#define REG_PWM1_BR_EN                      (1 << 31)
#define TG_SUBCNT_D2_ST_SHIFT               8
#define TG_SUBCNT_D2_ST_MASK                (0x7F << TG_SUBCNT_D2_ST_SHIFT)
#define TG_SUBCNT_D2_ST(n)                  BITFIELD_VAL(TG_SUBCNT_D2_ST, n)
#define REG_PWM2_BR_EN                      (1 << 15)
#define TG_SUBCNT_D3_ST_SHIFT               24
#define TG_SUBCNT_D3_ST_MASK                (0x7F << TG_SUBCNT_D3_ST_SHIFT)
#define TG_SUBCNT_D3_ST(n)                  BITFIELD_VAL(TG_SUBCNT_D3_ST, n)
#define REG_PWM3_BR_EN                      (1 << 31)

#define REG_PWM0_ST1_SHIFT                  0
#define REG_PWM0_ST1_MASK                   (0xFFFF << REG_PWM0_ST1_SHIFT)
#define REG_PWM0_ST1(n)                     BITFIELD_VAL(REG_PWM0_ST1, n)
#define REG_PWM1_ST1_SHIFT                  16
#define REG_PWM1_ST1_MASK                   (0xFFFF << REG_PWM1_ST1_SHIFT)
#define REG_PWM1_ST1(n)                     BITFIELD_VAL(REG_PWM1_ST1, n)
#define REG_PWM2_ST1_SHIFT                  0
#define REG_PWM2_ST1_MASK                   (0xFFFF << REG_PWM2_ST1_SHIFT)
#define REG_PWM2_ST1(n)                     BITFIELD_VAL(REG_PWM2_ST1, n)
#define REG_PWM3_ST1_SHIFT                  16
#define REG_PWM3_ST1_MASK                   (0xFFFF << REG_PWM3_ST1_SHIFT)
#define REG_PWM3_ST1(n)                     BITFIELD_VAL(REG_PWM3_ST1, n)
#endif

#endif

