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
#ifndef __REG_PSC_BEST1306_H__
#define __REG_PSC_BEST1306_H__

#include "plat_types.h"

struct AONPSC_T {
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
    __IO uint32_t REG_030;
    __IO uint32_t REG_034;
    __IO uint32_t REG_038;
    __IO uint32_t REG_03C;
    __IO uint32_t REG_040;
    __IO uint32_t REG_044;
    __IO uint32_t REG_048;
    __IO uint32_t REG_04C;
    __IO uint32_t REG_050;
    __IO uint32_t REG_054;
    __IO uint32_t REG_058;
    __IO uint32_t REG_05C;
    __IO uint32_t REG_060;
    __IO uint32_t REG_064;
    __IO uint32_t REG_068;
    __IO uint32_t REG_06C;
    __IO uint32_t REG_070;
    __IO uint32_t REG_074;
    __IO uint32_t REG_078;
    __IO uint32_t REG_07C;
    __IO uint32_t REG_080;
    __IO uint32_t REG_084;
    __IO uint32_t REG_088;
    __IO uint32_t REG_08C;
    __IO uint32_t REG_090;
    __IO uint32_t REG_094;
    __IO uint32_t REG_098;
    __IO uint32_t REG_09C;
    __IO uint32_t REG_0A0;
    __IO uint32_t REG_0A4;
    __IO uint32_t REG_0A8;
    __IO uint32_t REG_0AC;
    __IO uint32_t REG_0B0;
    __IO uint32_t REG_0B4;
    __IO uint32_t REG_0B8;
    __IO uint32_t REG_0BC;
    __IO uint32_t REG_0C0;
    __IO uint32_t REG_0C4;
    __IO uint32_t REG_0C8;
    __IO uint32_t REG_0CC;
    __IO uint32_t REG_0D0;
    __IO uint32_t REG_0D4;
    __IO uint32_t REG_0D8;
    __IO uint32_t REG_0DC;
    __IO uint32_t REG_0E0;
    __IO uint32_t REG_0E4;
    __IO uint32_t REG_0E8;
    __IO uint32_t REG_0EC;
    __IO uint32_t REG_0F0;
    __IO uint32_t REG_0F4;
    __IO uint32_t REG_0F8;
    __IO uint32_t REG_0FC;
};

// reg_00
#define PSC_AON_MCU_PG_AUTO_EN                              (1 << 0)
#define PSC_AON_MCU_PG_HW_EN                                (1 << 1)
#define PSC_AON_MCU_BYPASS_PWR_MEM                          (1 << 2)

// reg_04
#define PSC_AON_MCU_PSW_ACK_VALID                           (1 << 0)
#define PSC_AON_MCU_RESERVED(n)                             (((n) & 0x7F) << 1)
#define PSC_AON_MCU_RESERVED_MASK                           (0x7F << 1)
#define PSC_AON_MCU_RESERVED_SHIFT                          (1)
#define PSC_AON_MCU_MAIN_STATE(n)                           (((n) & 0x3) << 8)
#define PSC_AON_MCU_MAIN_STATE_MASK                         (0x3 << 8)
#define PSC_AON_MCU_MAIN_STATE_SHIFT                        (8)
#define PSC_AON_MCU_POWERDN_STATE(n)                        (((n) & 0x7) << 10)
#define PSC_AON_MCU_POWERDN_STATE_MASK                      (0x7 << 10)
#define PSC_AON_MCU_POWERDN_STATE_SHIFT                     (10)
#define PSC_AON_MCU_POWERUP_STATE(n)                        (((n) & 0x7) << 13)
#define PSC_AON_MCU_POWERUP_STATE_MASK                      (0x7 << 13)
#define PSC_AON_MCU_POWERUP_STATE_SHIFT                     (13)

// reg_08
#define PSC_AON_MCU_POWERDN_TIMER1(n)                       (((n) & 0x3F) << 0)
#define PSC_AON_MCU_POWERDN_TIMER1_MASK                     (0x3F << 0)
#define PSC_AON_MCU_POWERDN_TIMER1_SHIFT                    (0)
#define PSC_AON_MCU_POWERDN_TIMER2(n)                       (((n) & 0x3F) << 6)
#define PSC_AON_MCU_POWERDN_TIMER2_MASK                     (0x3F << 6)
#define PSC_AON_MCU_POWERDN_TIMER2_SHIFT                    (6)
#define PSC_AON_MCU_POWERDN_TIMER3(n)                       (((n) & 0x3F) << 12)
#define PSC_AON_MCU_POWERDN_TIMER3_MASK                     (0x3F << 12)
#define PSC_AON_MCU_POWERDN_TIMER3_SHIFT                    (12)
#define PSC_AON_MCU_POWERDN_TIMER4(n)                       (((n) & 0x3F) << 18)
#define PSC_AON_MCU_POWERDN_TIMER4_MASK                     (0x3F << 18)
#define PSC_AON_MCU_POWERDN_TIMER4_SHIFT                    (18)
#define PSC_AON_MCU_POWERDN_TIMER5(n)                       (((n) & 0xFF) << 24)
#define PSC_AON_MCU_POWERDN_TIMER5_MASK                     (0xFF << 24)
#define PSC_AON_MCU_POWERDN_TIMER5_SHIFT                    (24)

// reg_0c
#define PSC_AON_MCU_POWERUP_TIMER1(n)                       (((n) & 0x3F) << 0)
#define PSC_AON_MCU_POWERUP_TIMER1_MASK                     (0x3F << 0)
#define PSC_AON_MCU_POWERUP_TIMER1_SHIFT                    (0)
#define PSC_AON_MCU_POWERUP_TIMER2(n)                       (((n) & 0xFF) << 6)
#define PSC_AON_MCU_POWERUP_TIMER2_MASK                     (0xFF << 6)
#define PSC_AON_MCU_POWERUP_TIMER2_SHIFT                    (6)
#define PSC_AON_MCU_POWERUP_TIMER3(n)                       (((n) & 0x3F) << 14)
#define PSC_AON_MCU_POWERUP_TIMER3_MASK                     (0x3F << 14)
#define PSC_AON_MCU_POWERUP_TIMER3_SHIFT                    (14)
#define PSC_AON_MCU_POWERUP_TIMER4(n)                       (((n) & 0x3F) << 20)
#define PSC_AON_MCU_POWERUP_TIMER4_MASK                     (0x3F << 20)
#define PSC_AON_MCU_POWERUP_TIMER4_SHIFT                    (20)
#define PSC_AON_MCU_POWERUP_TIMER5(n)                       (((n) & 0x3F) << 26)
#define PSC_AON_MCU_POWERUP_TIMER5_MASK                     (0x3F << 26)
#define PSC_AON_MCU_POWERUP_TIMER5_SHIFT                    (26)

// reg_10
#define PSC_AON_MCU_POWERDN_START                           (1 << 0)

// reg_14
#define PSC_AON_MCU_POWERUP_START                           (1 << 0)

// reg_18
#define PSC_AON_MCU_CLK_STOP_REG                            (1 << 0)
#define PSC_AON_MCU_ISO_EN_REG                              (1 << 1)
#define PSC_AON_MCU_RESETN_ASSERT_REG                       (1 << 2)
#define PSC_AON_MCU_PSW_EN_REG                              (1 << 3)
#define PSC_AON_MCU_CLK_STOP_DR                             (1 << 4)
#define PSC_AON_MCU_ISO_EN_DR                               (1 << 5)
#define PSC_AON_MCU_RESETN_ASSERT_DR                        (1 << 6)
#define PSC_AON_MCU_PSW_EN_DR                               (1 << 7)
#define PSC_AON_MCU_MEM_PSW_EN_REG                          (1 << 8)
#define PSC_AON_MCU_MEM_PSW_EN_DR                           (1 << 9)

#if 0
// reg_1c
#define PSC_AON_MCU_MAIN_STATE(n)                           (((n) & 0x3) << 0)
#define PSC_AON_MCU_MAIN_STATE_MASK                         (0x3 << 0)
#define PSC_AON_MCU_MAIN_STATE_SHIFT                        (0)
#define PSC_AON_MCU_POWERDN_STATE(n)                        (((n) & 0x7) << 2)
#define PSC_AON_MCU_POWERDN_STATE_MASK                      (0x7 << 2)
#define PSC_AON_MCU_POWERDN_STATE_SHIFT                     (2)
#define PSC_AON_MCU_POWERUP_STATE(n)                        (((n) & 0x7) << 5)
#define PSC_AON_MCU_POWERUP_STATE_MASK                      (0x7 << 5)
#define PSC_AON_MCU_POWERUP_STATE_SHIFT                     (5)
#define PSC_AON_BT_MAIN_STATE(n)                            (((n) & 0x3) << 8)
#define PSC_AON_BT_MAIN_STATE_MASK                          (0x3 << 8)
#define PSC_AON_BT_MAIN_STATE_SHIFT                         (8)
#define PSC_AON_BT_POWERDN_STATE(n)                         (((n) & 0x7) << 10)
#define PSC_AON_BT_POWERDN_STATE_MASK                       (0x7 << 10)
#define PSC_AON_BT_POWERDN_STATE_SHIFT                      (10)
#define PSC_AON_BT_POWERUP_STATE(n)                         (((n) & 0x7) << 13)
#define PSC_AON_BT_POWERUP_STATE_MASK                       (0x7 << 13)
#define PSC_AON_BT_POWERUP_STATE_SHIFT                      (13)
#define PSC_AON_SENS_MAIN_STATE(n)                          (((n) & 0x3) << 16)
#define PSC_AON_SENS_MAIN_STATE_MASK                        (0x3 << 16)
#define PSC_AON_SENS_MAIN_STATE_SHIFT                       (16)
#define PSC_AON_SENS_POWERDN_STATE(n)                       (((n) & 0x7) << 18)
#define PSC_AON_SENS_POWERDN_STATE_MASK                     (0x7 << 18)
#define PSC_AON_SENS_POWERDN_STATE_SHIFT                    (18)
#define PSC_AON_SENS_POWERUP_STATE(n)                       (((n) & 0x7) << 21)
#define PSC_AON_SENS_POWERUP_STATE_MASK                     (0x7 << 21)
#define PSC_AON_SENS_POWERUP_STATE_SHIFT                    (21)
#define PSC_AON_CODEC_MAIN_STATE(n)                         (((n) & 0x3) << 24)
#define PSC_AON_CODEC_MAIN_STATE_MASK                       (0x3 << 24)
#define PSC_AON_CODEC_MAIN_STATE_SHIFT                      (24)
#define PSC_AON_CODEC_POWERDN_STATE(n)                      (((n) & 0x7) << 26)
#define PSC_AON_CODEC_POWERDN_STATE_MASK                    (0x7 << 26)
#define PSC_AON_CODEC_POWERDN_STATE_SHIFT                   (26)
#define PSC_AON_CODEC_POWERUP_STATE(n)                      (((n) & 0x7) << 29)
#define PSC_AON_CODEC_POWERUP_STATE_MASK                    (0x7 << 29)
#define PSC_AON_CODEC_POWERUP_STATE_SHIFT                   (29)
#endif

// reg_20
#define PSC_AON_BT_PG_AUTO_EN                               (1 << 0)
#define PSC_AON_BT_PG_HW_EN                                 (1 << 1)
#define PSC_AON_BT_BYPASS_PWR_MEM                           (1 << 2)

// reg_24
#define PSC_AON_BT_PSW_ACK_VALID                            (1 << 0)
#define PSC_AON_BT_RESERVED(n)                              (((n) & 0x7F) << 1)
#define PSC_AON_BT_RESERVED_MASK                            (0x7F << 1)
#define PSC_AON_BT_RESERVED_SHIFT                           (1)
#define PSC_AON_BT_MAIN_STATE(n)                            (((n) & 0x3) << 8)
#define PSC_AON_BT_MAIN_STATE_MASK                          (0x3 << 8)
#define PSC_AON_BT_MAIN_STATE_SHIFT                         (8)
#define PSC_AON_BT_POWERDN_STATE(n)                         (((n) & 0x7) << 10)
#define PSC_AON_BT_POWERDN_STATE_MASK                       (0x7 << 10)
#define PSC_AON_BT_POWERDN_STATE_SHIFT                      (10)
#define PSC_AON_BT_POWERUP_STATE(n)                         (((n) & 0x7) << 13)
#define PSC_AON_BT_POWERUP_STATE_MASK                       (0x7 << 13)
#define PSC_AON_BT_POWERUP_STATE_SHIFT                      (13)

#define PSC_AON_BT_SLEEP_NO_WFI                             (1 << 2)

// reg_28
#define PSC_AON_DIS_CLK_STOP_REG                            (1 << 0)
#define PSC_AON_DIS_ISO_EN_REG                              (1 << 1)
#define PSC_AON_DIS_RESETN_ASSERT_REG                       (1 << 2)
#define PSC_AON_DIS_PSW_EN_REG                              (1 << 3)
#define PSC_AON_DIS_CLK_STOP_DR                             (1 << 4)
#define PSC_AON_DIS_ISO_EN_DR                               (1 << 5)
#define PSC_AON_DIS_RESETN_ASSERT_DR                        (1 << 6)
#define PSC_AON_DIS_PSW_EN_DR                               (1 << 7)
#define PSC_AON_DIS_MEM_PSW_EN_REG                          (1 << 8)
#define PSC_AON_DIS_MEM_PSW_EN_DR                           (1 << 9)
 
// reg_2c
#define PSC_AON_PSRAM_MAIN_STATE(n)                         (((n) & 0x3) << 0)
#define PSC_AON_PSRAM_MAIN_STATE_MASK                       (0x3 << 0)
#define PSC_AON_PSRAM_MAIN_STATE_SHIFT                      (0)
#define PSC_AON_PSRAM_POWERDN_STATE(n)                      (((n) & 0x7) << 2)
#define PSC_AON_PSRAM_POWERDN_STATE_MASK                    (0x7 << 2)
#define PSC_AON_PSRAM_POWERDN_STATE_SHIFT                   (2)
#define PSC_AON_PSRAM_POWERUP_STATE(n)                      (((n) & 0x7) << 5)
#define PSC_AON_PSRAM_POWERUP_STATE_MASK                    (0x7 << 5)
#define PSC_AON_PSRAM_POWERUP_STATE_SHIFT                   (5)
#define PSC_AON_DIS_PG_AUTO_EN                              (1 << 8)
#define PSC_AON_DIS_BYPASS_PWR_MEM                          (1 << 9)

// reg_30
#define PSC_AON_BT_POWERDN_START                            (1 << 0)

// reg_34
#define PSC_AON_BT_POWERUP_START                            (1 << 0)

// reg_38
#define PSC_AON_BT_CLK_STOP_REG                             (1 << 0)
#define PSC_AON_BT_ISO_EN_REG                               (1 << 1)
#define PSC_AON_BT_RESETN_ASSERT_REG                        (1 << 2)
#define PSC_AON_BT_PSW_EN_REG                               (1 << 3)
#define PSC_AON_BT_CLK_STOP_DR                              (1 << 4)
#define PSC_AON_BT_ISO_EN_DR                                (1 << 5)
#define PSC_AON_BT_RESETN_ASSERT_DR                         (1 << 6)
#define PSC_AON_BT_PSW_EN_DR                                (1 << 7)
#define PSC_AON_BT_MEM_PSW_EN_REG                           (1 << 8)
#define PSC_AON_BT_MEM_PSW_EN_DR                            (1 << 9)

// reg_3c
#define PSC_AON_DIS_POWERDN_START                           (1 << 0)

// reg_40
#define PSC_AON_SENS_PG_AUTO_EN                             (1 << 0)
#define PSC_AON_SENS_PG_HW_EN                               (1 << 1)
#define PSC_AON_SENS_BYPASS_PWR_MEM                         (1 << 2)

// reg_44
#define PSC_AON_SENS_PSW_ACK_VALID                          (1 << 0)
#define PSC_AON_SENS_RESERVED(n)                            (((n) & 0x7F) << 1)
#define PSC_AON_SENS_RESERVED_MASK                          (0x7F << 1)
#define PSC_AON_SENS_RESERVED_SHIFT                         (1)
#define PSC_AON_SENS_MAIN_STATE(n)                          (((n) & 0x3) << 8)
#define PSC_AON_SENS_MAIN_STATE_MASK                        (0x3 << 8)
#define PSC_AON_SENS_MAIN_STATE_SHIFT                       (8)
#define PSC_AON_SENS_POWERDN_STATE(n)                       (((n) & 0x7) << 10)
#define PSC_AON_SENS_POWERDN_STATE_MASK                     (0x7 << 10)
#define PSC_AON_SENS_POWERDN_STATE_SHIFT                    (10)
#define PSC_AON_SENS_POWERUP_STATE(n)                       (((n) & 0x7) << 13)
#define PSC_AON_SENS_POWERUP_STATE_MASK                     (0x7 << 13)
#define PSC_AON_SENS_POWERUP_STATE_SHIFT                    (13)

// reg_48
#define PSC_AON_SENSCPU_POWERDN_TIMER1(n)                   (((n) & 0x3F) << 0)
#define PSC_AON_SENSCPU_POWERDN_TIMER1_MASK                 (0x3F << 0)
#define PSC_AON_SENSCPU_POWERDN_TIMER1_SHIFT                (0)
#define PSC_AON_SENSCPU_POWERDN_TIMER2(n)                   (((n) & 0x3F) << 6)
#define PSC_AON_SENSCPU_POWERDN_TIMER2_MASK                 (0x3F << 6)
#define PSC_AON_SENSCPU_POWERDN_TIMER2_SHIFT                (6)
#define PSC_AON_SENSCPU_POWERDN_TIMER3(n)                   (((n) & 0x3F) << 12)
#define PSC_AON_SENSCPU_POWERDN_TIMER3_MASK                 (0x3F << 12)
#define PSC_AON_SENSCPU_POWERDN_TIMER3_SHIFT                (12)
#define PSC_AON_SENSCPU_POWERDN_TIMER4(n)                   (((n) & 0x3F) << 18)
#define PSC_AON_SENSCPU_POWERDN_TIMER4_MASK                 (0x3F << 18)
#define PSC_AON_SENSCPU_POWERDN_TIMER4_SHIFT                (18)
#define PSC_AON_SENSCPU_POWERDN_TIMER5(n)                   (((n) & 0xFF) << 24)
#define PSC_AON_SENSCPU_POWERDN_TIMER5_MASK                 (0xFF << 24)
#define PSC_AON_SENSCPU_POWERDN_TIMER5_SHIFT                (24)

// reg_4c
#define PSC_AON_SENSCPU_POWERUP_TIMER1(n)                   (((n) & 0x3F) << 0)
#define PSC_AON_SENSCPU_POWERUP_TIMER1_MASK                 (0x3F << 0)
#define PSC_AON_SENSCPU_POWERUP_TIMER1_SHIFT                (0)
#define PSC_AON_SENSCPU_POWERUP_TIMER2(n)                   (((n) & 0xFF) << 6)
#define PSC_AON_SENSCPU_POWERUP_TIMER2_MASK                 (0xFF << 6)
#define PSC_AON_SENSCPU_POWERUP_TIMER2_SHIFT                (6)
#define PSC_AON_SENSCPU_POWERUP_TIMER3(n)                   (((n) & 0x3F) << 14)
#define PSC_AON_SENSCPU_POWERUP_TIMER3_MASK                 (0x3F << 14)
#define PSC_AON_SENSCPU_POWERUP_TIMER3_SHIFT                (14)
#define PSC_AON_SENSCPU_POWERUP_TIMER4(n)                   (((n) & 0x3F) << 20)
#define PSC_AON_SENSCPU_POWERUP_TIMER4_MASK                 (0x3F << 20)
#define PSC_AON_SENSCPU_POWERUP_TIMER4_SHIFT                (20)
#define PSC_AON_SENSCPU_POWERUP_TIMER5(n)                   (((n) & 0x3F) << 26)
#define PSC_AON_SENSCPU_POWERUP_TIMER5_MASK                 (0x3F << 26)
#define PSC_AON_SENSCPU_POWERUP_TIMER5_SHIFT                (26)

// reg_50
#define PSC_AON_SENS_POWERDN_START                          (1 << 0)

// reg_54
#define PSC_AON_SENS_POWERUP_START                          (1 << 0)

// reg_58
#define PSC_AON_SENS_CLK_STOP_REG                           (1 << 0)
#define PSC_AON_SENS_ISO_EN_REG                             (1 << 1)
#define PSC_AON_SENS_RESETN_ASSERT_REG                      (1 << 2)
#define PSC_AON_SENS_PSW_EN_REG                             (1 << 3)
#define PSC_AON_SENS_CLK_STOP_DR                            (1 << 4)
#define PSC_AON_SENS_ISO_EN_DR                              (1 << 5)
#define PSC_AON_SENS_RESETN_ASSERT_DR                       (1 << 6)
#define PSC_AON_SENS_PSW_EN_DR                              (1 << 7)
#define PSC_AON_SENS_MEM_PSW_EN_REG                         (1 << 8)
#define PSC_AON_SENS_MEM_PSW_EN_DR                          (1 << 9)

// reg_5c
#define PSC_AON_DIS_POWERUP_START                           (1 << 0)

// reg_60
#define PSC_AON_CODEC_PG_AUTO_EN                            (1 << 0)
#define PSC_AON_CODEC_BYPASS_PWR_MEM                        (1 << 1)

// reg_64
#define PSC_AON_CODEC_PSW_ACK_VALID                         (1 << 0)
#define PSC_AON_CODEC_RESERVED(n)                           (((n) & 0x7F) << 1)
#define PSC_AON_CODEC_RESERVED_MASK                         (0x7F << 1)
#define PSC_AON_CODEC_RESERVED_SHIFT                        (1)
#define PSC_AON_CODEC_MAIN_STATE(n)                         (((n) & 0x3) << 8)
#define PSC_AON_CODEC_MAIN_STATE_MASK                       (0x3 << 8)
#define PSC_AON_CODEC_MAIN_STATE_SHIFT                      (8)
#define PSC_AON_CODEC_POWERDN_STATE(n)                      (((n) & 0x7) << 10)
#define PSC_AON_CODEC_POWERDN_STATE_MASK                    (0x7 << 10)
#define PSC_AON_CODEC_POWERDN_STATE_SHIFT                   (10)
#define PSC_AON_CODEC_POWERUP_STATE(n)                      (((n) & 0x7) << 13)
#define PSC_AON_CODEC_POWERUP_STATE_MASK                    (0x7 << 13)
#define PSC_AON_CODEC_POWERUP_STATE_SHIFT                   (13)

// reg_70
#define PSC_AON_CODEC_POWERDN_START                         (1 << 0)

// reg_74
#define PSC_AON_CODEC_POWERUP_START                         (1 << 0)

// reg_78
#define PSC_AON_CODEC_CLK_STOP_REG                          (1 << 0)
#define PSC_AON_CODEC_ISO_EN_REG                            (1 << 1)
#define PSC_AON_CODEC_RESETN_ASSERT_REG                     (1 << 2)
#define PSC_AON_CODEC_PSW_EN_REG                            (1 << 3)
#define PSC_AON_CODEC_CLK_STOP_DR                           (1 << 4)
#define PSC_AON_CODEC_ISO_EN_DR                             (1 << 5)
#define PSC_AON_CODEC_RESETN_ASSERT_DR                      (1 << 6)
#define PSC_AON_CODEC_PSW_EN_DR                             (1 << 7)
#define PSC_AON_CODEC_MEM_PSW_EN_REG                        (1 << 8)
#define PSC_AON_CODEC_MEM_PSW_EN_DR                         (1 << 9)

// reg_7c
#define PSC_AON_SRAM4_ISO_EN_REG                            (1 << 0)
#define PSC_AON_SRAM5_ISO_EN_REG                            (1 << 1)
#define PSC_AON_SRAM6_ISO_EN_REG                            (1 << 2)
#define PSC_AON_SRAM7_ISO_EN_REG                            (1 << 3)
#define PSC_AON_SRAM8_ISO_EN_REG                            (1 << 4)
#define PSC_AON_SRAM4_ISO_EN_DR                             (1 << 5)
#define PSC_AON_SRAM5_ISO_EN_DR                             (1 << 6)
#define PSC_AON_SRAM6_ISO_EN_DR                             (1 << 7)
#define PSC_AON_SRAM7_ISO_EN_DR                             (1 << 8)
#define PSC_AON_SRAM8_ISO_EN_DR                             (1 << 9)

// reg_80
#define PSC_AON_MCU_INTR_MASK(n)                            (((n) & 0xFFFFFFFF) << 0)
#define PSC_AON_MCU_INTR_MASK_MASK                          (0xFFFFFFFF << 0)
#define PSC_AON_MCU_INTR_MASK_SHIFT                         (0)

// reg_84
#define PSC_AON_MCU_INTR_MASK2(n)                           (((n) & 0xFFFFFFFF) << 0)
#define PSC_AON_MCU_INTR_MASK2_MASK                         (0xFFFFFFFF << 0)
#define PSC_AON_MCU_INTR_MASK2_SHIFT                        (0)

// reg_88
#define PSC_AON_MCU_INTR_MASK3(n)                           (((n) & 0x1FFFFFF) << 0)
#define PSC_AON_MCU_INTR_MASK3_MASK                         (0x1FFFFFF << 0)
#define PSC_AON_MCU_INTR_MASK3_SHIFT                        (0)

// reg_8c
#define PSC_AON_MCU_INTR_MASK_STATUS(n)                     (((n) & 0xFFFFFFFF) << 0)
#define PSC_AON_MCU_INTR_MASK_STATUS_MASK                   (0xFFFFFFFF << 0)
#define PSC_AON_MCU_INTR_MASK_STATUS_SHIFT                  (0)

// reg_90
#define PSC_AON_MCU_INTR_MASK_STATUS2(n)                    (((n) & 0xFFFFFFFF) << 0)
#define PSC_AON_MCU_INTR_MASK_STATUS2_MASK                  (0xFFFFFFFF << 0)
#define PSC_AON_MCU_INTR_MASK_STATUS2_SHIFT                 (0)

// reg_94
#define PSC_AON_MCU_INTR_MASK_STATUS3(n)                    (((n) & 0x1FFFFFF) << 0)
#define PSC_AON_MCU_INTR_MASK_STATUS3_MASK                  (0x1FFFFFF << 0)
#define PSC_AON_MCU_INTR_MASK_STATUS3_SHIFT                 (0)

// reg_98
#define PSC_AON_BT_INTR_MASK(n)                             (((n) & 0xFFFFFFFF) << 0)
#define PSC_AON_BT_INTR_MASK_MASK                           (0xFFFFFFFF << 0)
#define PSC_AON_BT_INTR_MASK_SHIFT                          (0)

// reg_9c
#define PSC_AON_BT_INTR_MASK2(n)                            (((n) & 0xFFFFFFFF) << 0)
#define PSC_AON_BT_INTR_MASK2_MASK                          (0xFFFFFFFF << 0)
#define PSC_AON_BT_INTR_MASK2_SHIFT                         (0)

// reg_a0
#define PSC_AON_BT_INTR_MASK3(n)                            (((n) & 0x1FFFFFF) << 0)
#define PSC_AON_BT_INTR_MASK3_MASK                          (0x1FFFFFF << 0)
#define PSC_AON_BT_INTR_MASK3_SHIFT                         (0)

// reg_a4
#define PSC_AON_BT_INTR_MASK_STATUS(n)                      (((n) & 0xFFFFFFFF) << 0)
#define PSC_AON_BT_INTR_MASK_STATUS_MASK                    (0xFFFFFFFF << 0)
#define PSC_AON_BT_INTR_MASK_STATUS_SHIFT                   (0)

// reg_a8
#define PSC_AON_BT_INTR_MASK_STATUS2(n)                     (((n) & 0xFFFFFFFF) << 0)
#define PSC_AON_BT_INTR_MASK_STATUS2_MASK                   (0xFFFFFFFF << 0)
#define PSC_AON_BT_INTR_MASK_STATUS2_SHIFT                  (0)

// reg_ac
#define PSC_AON_BT_INTR_MASK_STATUS3(n)                     (((n) & 0x1FFFFFF) << 0)
#define PSC_AON_BT_INTR_MASK_STATUS3_MASK                   (0x1FFFFFF << 0)
#define PSC_AON_BT_INTR_MASK_STATUS3_SHIFT                  (0)

// reg_b0
#define PSC_AON_SENS_INTR_MASK(n)                           (((n) & 0xFFFFFFFF) << 0)
#define PSC_AON_SENS_INTR_MASK_MASK                         (0xFFFFFFFF << 0)
#define PSC_AON_SENS_INTR_MASK_SHIFT                        (0)

// reg_b4
#define PSC_AON_SENS_INTR_MASK2(n)                          (((n) & 0xFFFFFFFF) << 0)
#define PSC_AON_SENS_INTR_MASK2_MASK                        (0xFFFFFFFF << 0)
#define PSC_AON_SENS_INTR_MASK2_SHIFT                       (0)

// reg_b8
#define PSC_AON_SENS_INTR_MASK3(n)                          (((n) & 0x1FFFFFF) << 0)
#define PSC_AON_SENS_INTR_MASK3_MASK                        (0x1FFFFFF << 0)
#define PSC_AON_SENS_INTR_MASK3_SHIFT                       (0)

// reg_bc
#define PSC_AON_SENS_INTR_MASK_STATUS(n)                    (((n) & 0xFFFFFFFF) << 0)
#define PSC_AON_SENS_INTR_MASK_STATUS_MASK                  (0xFFFFFFFF << 0)
#define PSC_AON_SENS_INTR_MASK_STATUS_SHIFT                 (0)

// reg_c0
#define PSC_AON_SENS_INTR_MASK_STATUS2(n)                   (((n) & 0xFFFFFFFF) << 0)
#define PSC_AON_SENS_INTR_MASK_STATUS2_MASK                 (0xFFFFFFFF << 0)
#define PSC_AON_SENS_INTR_MASK_STATUS2_SHIFT                (0)

// reg_c4
#define PSC_AON_SENS_INTR_MASK_STATUS3(n)                   (((n) & 0x1FFFFFF) << 0)
#define PSC_AON_SENS_INTR_MASK_STATUS3_MASK                 (0x1FFFFFF << 0)
#define PSC_AON_SENS_INTR_MASK_STATUS3_SHIFT                (0)

// reg_c8
#define PSC_AON_INTR_RAW_STATUS(n)                          (((n) & 0xFFFFFFFF) << 0)
#define PSC_AON_INTR_RAW_STATUS_MASK                        (0xFFFFFFFF << 0)
#define PSC_AON_INTR_RAW_STATUS_SHIFT                       (0)

// reg_cc
#define PSC_AON_INTR_RAW_STATUS2(n)                         (((n) & 0xFFFFFFFF) << 0)
#define PSC_AON_INTR_RAW_STATUS2_MASK                       (0xFFFFFFFF << 0)
#define PSC_AON_INTR_RAW_STATUS2_SHIFT                      (0)

// reg_d0
#define PSC_AON_INTR_RAW_STATUS3(n)                         (((n) & 0x1FFFFFF) << 0)
#define PSC_AON_INTR_RAW_STATUS3_MASK                       (0x1FFFFFF << 0)
#define PSC_AON_INTR_RAW_STATUS3_SHIFT                      (0)

// reg_d4
#define PSC_AON_BLK_PG_AUTO_EN                              (1 << 0)
#define PSC_AON_BLK_POWERDN_HW_EN                           (1 << 1)
#define PSC_AON_BLK_POWERUP_HW_EN                           (1 << 2)

// reg_d8
#define PSC_AON_BLK_PSW_ACK_VALID                           (1 << 0)
#define PSC_AON_BLK_RESERVED(n)                             (((n) & 0x7F) << 1)
#define PSC_AON_BLK_RESERVED_MASK                           (0x7F << 1)
#define PSC_AON_BLK_RESERVED_SHIFT                          (1)
#define PSC_AON_BLK_MAIN_STATE(n)                           (((n) & 0x3) << 8)
#define PSC_AON_BLK_MAIN_STATE_MASK                         (0x3 << 8)
#define PSC_AON_BLK_MAIN_STATE_SHIFT                        (8)
#define PSC_AON_BLK_POWERDN_STATE(n)                        (((n) & 0x7) << 10)
#define PSC_AON_BLK_POWERDN_STATE_MASK                      (0x7 << 10)
#define PSC_AON_BLK_POWERDN_STATE_SHIFT                     (10)
#define PSC_AON_BLK_POWERUP_STATE(n)                        (((n) & 0x7) << 13)
#define PSC_AON_BLK_POWERUP_STATE_MASK                      (0x7 << 13)
#define PSC_AON_BLK_POWERUP_STATE_SHIFT                     (13)

// reg_dc
#define PSC_AON_BLK_POWERDN_START                           (1 << 0)

// reg_e0
#define PSC_AON_BLK_POWERUP_START                           (1 << 0)

// reg_e4
#define PSC_AON_BLK_CLK_STOP_REG                            (1 << 0)
#define PSC_AON_BLK_ISO_EN_REG                              (1 << 1)
#define PSC_AON_BLK_RESETN_ASSERT_REG                       (1 << 2)
#define PSC_AON_BLK_PSW_EN_REG                              (1 << 3)
#define PSC_AON_BLK_CLK_STOP_DR                             (1 << 4)
#define PSC_AON_BLK_ISO_EN_DR                               (1 << 5)
#define PSC_AON_BLK_RESETN_ASSERT_DR                        (1 << 6)
#define PSC_AON_BLK_PSW_EN_DR                               (1 << 7)

// reg_e8
#define PSC_AON_SENSCPU_PG_AUTO_EN                          (1 << 0)
#define PSC_AON_SENSCPU_PG_HW_EN                            (1 << 1)

// reg_ec
#define PSC_AON_SENSCPU_PSW_ACK_VALID                       (1 << 0)
#define PSC_AON_SENSCPU_RESERVED(n)                         (((n) & 0x7F) << 1)
#define PSC_AON_SENSCPU_RESERVED_MASK                       (0x7F << 1)
#define PSC_AON_SENSCPU_RESERVED_SHIFT                      (1)
#define PSC_AON_SENSCPU_MAIN_STATE(n)                       (((n) & 0x3) << 8)
#define PSC_AON_SENSCPU_MAIN_STATE_MASK                     (0x3 << 8)
#define PSC_AON_SENSCPU_MAIN_STATE_SHIFT                    (8)
#define PSC_AON_SENSCPU_POWERDN_STATE(n)                    (((n) & 0x7) << 10)
#define PSC_AON_SENSCPU_POWERDN_STATE_MASK                  (0x7 << 10)
#define PSC_AON_SENSCPU_POWERDN_STATE_SHIFT                 (10)
#define PSC_AON_SENSCPU_POWERUP_STATE(n)                    (((n) & 0x7) << 13)
#define PSC_AON_SENSCPU_POWERUP_STATE_MASK                  (0x7 << 13)
#define PSC_AON_SENSCPU_POWERUP_STATE_SHIFT                 (13)
#define PSC_AON_SENSCPU_INT_MASK                            (1 << 16)

// reg_f0
#define PSC_AON_SENSCPU_POWERDN_START                       (1 << 0)

// reg_f4
#define PSC_AON_SENSCPU_POWERUP_START                       (1 << 0)

// reg_f8
#define PSC_AON_SENSCPU_CLK_STOP_REG                        (1 << 0)
#define PSC_AON_SENSCPU_ISO_EN_REG                          (1 << 1)
#define PSC_AON_SENSCPU_RESETN_ASSERT_REG                   (1 << 2)
#define PSC_AON_SENSCPU_PSW_EN_REG                          (1 << 3)
#define PSC_AON_SENSCPU_CLK_STOP_DR                         (1 << 4)
#define PSC_AON_SENSCPU_ISO_EN_DR                           (1 << 5)
#define PSC_AON_SENSCPU_RESETN_ASSERT_DR                    (1 << 6)
#define PSC_AON_SENSCPU_PSW_EN_DR                           (1 << 7)

// reg_fc
#define PSC_AON_SRAM4_PE_PSW_EN_REG                         (1 << 0)
#define PSC_AON_SRAM5_PE_PSW_EN_REG                         (1 << 1)
#define PSC_AON_SRAM6_PE_PSW_EN_REG                         (1 << 2)
#define PSC_AON_SRAM7_PE_PSW_EN_REG                         (1 << 3)
#define PSC_AON_SRAM8_PE_PSW_EN_REG                         (1 << 4)
#define PSC_AON_SRAM4_CE_PSW_EN_REG                         (1 << 5)
#define PSC_AON_SRAM5_CE_PSW_EN_REG                         (1 << 6)
#define PSC_AON_SRAM6_CE_PSW_EN_REG                         (1 << 7)
#define PSC_AON_SRAM7_CE_PSW_EN_REG                         (1 << 8)
#define PSC_AON_SRAM8_CE_PSW_EN_REG                         (1 << 9)
#define PSC_AON_SRAM4_PSW_EN_DR                             (1 << 10)
#define PSC_AON_SRAM5_PSW_EN_DR                             (1 << 11)
#define PSC_AON_SRAM6_PSW_EN_DR                             (1 << 12)
#define PSC_AON_SRAM7_PSW_EN_DR                             (1 << 13)
#define PSC_AON_SRAM8_PSW_EN_DR                             (1 << 14)

#endif
