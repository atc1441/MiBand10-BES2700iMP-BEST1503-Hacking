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
#ifndef __REG_AONCMU_BEST1306_H__
#define __REG_AONCMU_BEST1306_H__

#include "plat_types.h"

struct AONCMU_T {
    __I  uint32_t CHIP_ID;          // 0x00
    __IO uint32_t TOP_CLK_ENABLE;   // 0x04
    __IO uint32_t TOP_CLK_DISABLE;  // 0x08
    __IO uint32_t RESET_PULSE;      // 0x0C
    __IO uint32_t RESET_SET;        // 0x10
    __IO uint32_t RESET_CLR;        // 0x14
    __IO uint32_t CLK_SELECT;       // 0x18
    __IO uint32_t CLK_OUT;          // 0x1C
    __IO uint32_t WRITE_UNLOCK;     // 0x20
    __IO uint32_t MEMSC[4];         // 0x24
    __I  uint32_t MEMSC_STATUS;     // 0x34
    __IO uint32_t BOOTMODE;         // 0x38
    __IO uint32_t RESERVED_03C;     // 0x3C
    __IO uint32_t MOD_CLK_ENABLE;   // 0x40
    __IO uint32_t MOD_CLK_DISABLE;  // 0x44
    __IO uint32_t MOD_CLK_MODE;     // 0x48
    __IO uint32_t AON_CLK;          // 0x4C
    __IO uint32_t TIMER0_CLK;       // 0x50
    __IO uint32_t PWM01_CLK;        // 0x54
    __IO uint32_t PWM23_CLK;        // 0x58
    __IO uint32_t RAM_CFG;          // 0x5C
    __IO uint32_t TIMER1_CLK;       // 0x60
    __IO uint32_t MCU_PWR_MODE;     // 0x64
    __IO uint32_t RAM_LP_TIMER;     // 0x68
    __IO uint32_t SLEEP_TIMER_OSC;  // 0x6C
    __IO uint32_t PWM45_CLK;        // 0x70
    __IO uint32_t PWM67_CLK;        // 0x74
    __IO uint32_t MCU_VTOR;         // 0x78
    __IO uint32_t SE_WLOCK;         // 0x7C
    __IO uint32_t SE_RLOCK;         // 0x80
    __IO uint32_t PD_STAB_TIMER;    // 0x84
    __IO uint32_t RAM2_CFG0;        // 0x88
    __IO uint32_t RAM2_CFG1;        // 0x8C
    __IO uint32_t RAM3_CFG0;        // 0x90
    __IO uint32_t RAM3_CFG1;        // 0x94
    __IO uint32_t RAM_CFG_SEL;      // 0x98
    __IO uint32_t RESERVED_09C;     // 0x9C
    __IO uint32_t TOP_CLK1_ENABLE;  // 0xA0
    __IO uint32_t TOP_CLK1_DISABLE; // 0xA4
    __IO uint32_t SENS_CP_VTOR;     // 0xA8
    __IO uint32_t SENS_VTOR;        // 0xAC
    __IO uint32_t RAM4_CFG;         // 0xB0
    __IO uint32_t BT_RAM_CFG;       // 0xB4
    __IO uint32_t SENS_RAM_CFG;     // 0xB8
    __IO uint32_t CODEC_RAM_CFG;    // 0xBC
    __IO uint32_t REG_RAM_CFG;      // 0xC0
    __IO uint32_t GBL_RESET_PULSE;  // 0xC4
    __IO uint32_t GBL_RESET_SET;    // 0xC8
    __IO uint32_t GBL_RESET_CLR;    // 0xCC
    __IO uint32_t FLASH_IODRV;      // 0xD0
    __IO uint32_t RESERVED_0D4;     // 0xD4
    __IO uint32_t ORESETN_SET;      // 0xD8
    __IO uint32_t ORESETN_CLR;      // 0xDC
    __IO uint32_t MANUAL_OCLK_ENABLE;        // 0xE0
    __IO uint32_t MANUAL_OCLK_DISABLE;       // 0xE4
    __IO uint32_t MODE_OCLOCK;      // 0xE8
    __IO uint32_t CAP_OTHER;        // 0xEC
    __IO uint32_t WAKEUP_PC;        // 0xF0
    __IO uint32_t DEBUG_RES;        // 0xF4
    __IO uint32_t SW_INFO;          // 0xF8
    __IO uint32_t CHIP_FEATURE;     // 0xFC
    __IO uint32_t RESERVED_100;     // 0x100
    __IO uint32_t RESERVED_104;     // 0x104
    __IO uint32_t CAP_CLK_CFG;      // 0x108
};

// reg_00
#define AON_CMU_CHIP_ID(n)                                  (((n) & 0xFFFF) << 0)
#define AON_CMU_CHIP_ID_MASK                                (0xFFFF << 0)
#define AON_CMU_CHIP_ID_SHIFT                               (0)
#define AON_CMU_REVISION_ID(n)                              (((n) & 0xFFFF) << 16)
#define AON_CMU_REVISION_ID_MASK                            (0xFFFF << 16)
#define AON_CMU_REVISION_ID_SHIFT                           (16)

// reg_04
#define AON_CMU_EN_CLK_TOP_PLLBB_ENABLE                     (1 << 0)
#define AON_CMU_EN_CLK_TOP_PLLAUD_ENABLE                    (1 << 1)
#define AON_CMU_EN_CLK_TOP_OSCX2_ENABLE                     (1 << 2)
#define AON_CMU_EN_CLK_TOP_OSC_ENABLE                       (1 << 3)
#define AON_CMU_EN_CLK_TOP_JTAG_ENABLE                      (1 << 4)
#define AON_CMU_EN_CLK_TOP_PLLUSB_ENABLE                    (1 << 5)
#define AON_CMU_EN_CLK_32K_CODEC_ENABLE                     (1 << 6)
#define AON_CMU_EN_CLK_32K_MCU_ENABLE                       (1 << 7)
#define AON_CMU_EN_CLK_32K_BT_ENABLE                        (1 << 8)
#define AON_CMU_EN_CLK_DCDC0_ENABLE                         (1 << 9)
#define AON_CMU_EN_CLK_TOP_PLLDSI_ENABLE                    (1 << 10)
#define AON_CMU_PU_PLLDSI_ENABLE                            (1 << 11)
#define AON_CMU_EN_X2_DIG_ENABLE                            (1 << 12)
#define AON_CMU_EN_X4_DIG_ENABLE                            (1 << 13)
#define AON_CMU_PU_PLLBB_ENABLE                             (1 << 14)
#define AON_CMU_PU_PLLUSB_ENABLE                            (1 << 15)
#define AON_CMU_PU_PLLAUD_ENABLE                            (1 << 16)
#define AON_CMU_PU_OSC_ENABLE                               (1 << 17)
#define AON_CMU_EN_CLK_TOP_OSCX4_ENABLE                     (1 << 18)
#define AON_CMU_EN_BT_CLK_SYS_ENABLE                        (1 << 19)

// reg_08
#define AON_CMU_EN_CLK_TOP_PLLBB_DISABLE                    (1 << 0)
#define AON_CMU_EN_CLK_TOP_PLLAUD_DISABLE                   (1 << 1)
#define AON_CMU_EN_CLK_TOP_OSCX2_DISABLE                    (1 << 2)
#define AON_CMU_EN_CLK_TOP_OSC_DISABLE                      (1 << 3)
#define AON_CMU_EN_CLK_TOP_JTAG_DISABLE                     (1 << 4)
#define AON_CMU_EN_CLK_TOP_PLLUSB_DISABLE                   (1 << 5)
#define AON_CMU_EN_CLK_32K_CODEC_DISABLE                    (1 << 6)
#define AON_CMU_EN_CLK_32K_MCU_DISABLE                      (1 << 7)
#define AON_CMU_EN_CLK_32K_BT_DISABLE                       (1 << 8)
#define AON_CMU_EN_CLK_DCDC0_DISABLE                        (1 << 9)
#define AON_CMU_EN_CLK_TOP_PLLDSI_DISABLE                   (1 << 10)
#define AON_CMU_PU_PLLDSI_DISABLE                           (1 << 11)
#define AON_CMU_EN_X2_DIG_DISABLE                           (1 << 12)
#define AON_CMU_EN_X4_DIG_DISABLE                           (1 << 13)
#define AON_CMU_PU_PLLBB_DISABLE                            (1 << 14)
#define AON_CMU_PU_PLLUSB_DISABLE                           (1 << 15)
#define AON_CMU_PU_PLLAUD_DISABLE                           (1 << 16)
#define AON_CMU_PU_OSC_DISABLE                              (1 << 17)
#define AON_CMU_EN_CLK_TOP_OSCX4_DISABLE                    (1 << 18)
#define AON_CMU_EN_BT_CLK_SYS_DISABLE                       (1 << 19)

#define AON_ARST_NUM                                        17
#define AON_ORST_NUM                                        13
#define AON_ACLK_NUM                                        AON_ARST_NUM
#define AON_OCLK_NUM                                        AON_ORST_NUM

// reg_0c
#define AON_CMU_ARESETN_PULSE(n)                            (((n) & 0xFFFFFFFF) << 0)
#define AON_CMU_ARESETN_PULSE_MASK                          (0xFFFFFFFF << 0)
#define AON_CMU_ARESETN_PULSE_SHIFT                         (0)
#define AON_CMU_ORESETN_PULSE(n)                            (((n) & 0xFFFFFFFF) << AON_ARST_NUM)
#define AON_CMU_ORESETN_PULSE_MASK                          (0xFFFFFFFF << AON_ARST_NUM)
#define AON_CMU_ORESETN_PULSE_SHIFT                         (AON_ARST_NUM)

// reg_10
#define AON_CMU_ARESETN_SET(n)                              (((n) & 0xFFFFFFFF) << 0)
#define AON_CMU_ARESETN_SET_MASK                            (0xFFFFFFFF << 0)
#define AON_CMU_ARESETN_SET_SHIFT                           (0)
#define AON_CMU_ORESETN_SET(n)                              (((n) & 0xFFFFFFFF) << AON_ARST_NUM)
#define AON_CMU_ORESETN_SET_MASK                            (0xFFFFFFFF << AON_ARST_NUM)
#define AON_CMU_ORESETN_SET_SHIFT                           (AON_ARST_NUM)

// reg_14
#define AON_CMU_ARESETN_CLR(n)                              (((n) & 0xFFFFFFFF) << 0)
#define AON_CMU_ARESETN_CLR_MASK                            (0xFFFFFFFF << 0)
#define AON_CMU_ARESETN_CLR_SHIFT                           (0)
#define AON_CMU_ORESETN_CLR(n)                              (((n) & 0xFFFFFFFF) << AON_ARST_NUM)
#define AON_CMU_ORESETN_CLR_MASK                            (0xFFFFFFFF << AON_ARST_NUM)
#define AON_CMU_ORESETN_CLR_SHIFT                           (AON_ARST_NUM)

// reg_18
#define AON_CMU_SEL_OSCX2_DIG                               (1 << 0)
#define AON_CMU_SEL_X2_PHASE(n)                             (((n) & 0x1F) << 1)
#define AON_CMU_SEL_X2_PHASE_MASK                           (0x1F << 1)
#define AON_CMU_SEL_X2_PHASE_SHIFT                          (1)
#define AON_CMU_SEL_X4_PHASE(n)                             (((n) & 0x1F) << 6)
#define AON_CMU_SEL_X4_PHASE_MASK                           (0x1F << 6)
#define AON_CMU_SEL_X4_PHASE_SHIFT                          (6)
#define AON_CMU_SEL_X4_DIG                                  (1 << 11)
#define AON_CMU_LPU_AUTO_SWITCH26                           (1 << 12)
#define AON_CMU_EN_MCU_WDG_RESET                            (1 << 13)
#define AON_CMU_TIMER_WT24(n)                               (((n) & 0xFF) << 14)
#define AON_CMU_TIMER_WT24_MASK                             (0xFF << 14)
#define AON_CMU_TIMER_WT24_SHIFT                            (14)
#define AON_CMU_TIMER_WT24_EN                               (1 << 22)
#define AON_CMU_OSC_READY_MODE                              (1 << 23)
#define AON_CMU_SEL_PU_OSC_READY_ANA                        (1 << 24)
#define AON_CMU_EN_SENS_WDG_RESET                           (1 << 25)
#define AON_CMU_OSC_READY_BYPASS_SYNC                       (1 << 26)
#define AON_CMU_SEL_SLOW_SYS_BYPASS                         (1 << 27)

// reg_1c
#define AON_CMU_EN_CLK_OUT                                  (1 << 0)
#define AON_CMU_SEL_CLK_OUT(n)                              (((n) & 0x7) << 1)
#define AON_CMU_SEL_CLK_OUT_MASK                            (0x7 << 1)
#define AON_CMU_SEL_CLK_OUT_SHIFT                           (1)
#define AON_CMU_CFG_CLK_OUT(n)                              (((n) & 0xF) << 4)
#define AON_CMU_CFG_CLK_OUT_MASK                            (0xF << 4)
#define AON_CMU_CFG_CLK_OUT_SHIFT                           (4)
#define AON_CMU_CFG_DIV_DCDC(n)                             (((n) & 0xF) << 8)
#define AON_CMU_CFG_DIV_DCDC_MASK                           (0xF << 8)
#define AON_CMU_CFG_DIV_DCDC_SHIFT                          (8)
#define AON_CMU_BYPASS_DIV_DCDC                             (1 << 12)
#define AON_CMU_CLK_DCDC_DRV(n)                             (((n) & 0x3) << 13)
#define AON_CMU_CLK_DCDC_DRV_MASK                           (0x3 << 13)
#define AON_CMU_CLK_DCDC_DRV_SHIFT                          (13)
#define AON_CMU_SEL_32K_TIMER(n)                            (((n) & 0x3) << 15)
#define AON_CMU_SEL_32K_TIMER_MASK                          (0x3 << 15)
#define AON_CMU_SEL_32K_TIMER_SHIFT                         (15)
#define AON_CMU_SEL_32K_WDT                                 (1 << 17)
#define AON_CMU_SEL_TIMER_FAST(n)                           (((n) & 0x3) << 18)
#define AON_CMU_SEL_TIMER_FAST_MASK                         (0x3 << 18)
#define AON_CMU_SEL_TIMER_FAST_SHIFT                        (18)
#define AON_CMU_CFG_TIMER_FAST(n)                           (((n) & 0x3) << 20)
#define AON_CMU_CFG_TIMER_FAST_MASK                         (0x3 << 20)
#define AON_CMU_CFG_TIMER_FAST_SHIFT                        (20)
#define AON_CMU_CFG_DIV_DCDC_OPTI(n)                        (((n) & 0x7) << 25)
#define AON_CMU_CFG_DIV_DCDC_OPTI_MASK                      (0x7 << 25)
#define AON_CMU_CFG_DIV_DCDC_OPTI_SHIFT                     (25)

// reg_20
#define AON_CMU_WRITE_UNLOCK_H                              (1 << 0)
#define AON_CMU_WRITE_UNLOCK_STATUS                         (1 << 1)

// reg_24
#define AON_CMU_MEMSC0                                      (1 << 0)

// reg_28
#define AON_CMU_MEMSC1                                      (1 << 0)

// reg_2c
#define AON_CMU_MEMSC2                                      (1 << 0)

// reg_30
#define AON_CMU_MEMSC3                                      (1 << 0)

// reg_34
#define AON_CMU_MEMSC_STATUS0                               (1 << 0)
#define AON_CMU_MEMSC_STATUS1                               (1 << 1)
#define AON_CMU_MEMSC_STATUS2                               (1 << 2)
#define AON_CMU_MEMSC_STATUS3                               (1 << 3)

// reg_38
#define AON_CMU_WATCHDOG_RESET                              (1 << 0)
#define AON_CMU_SOFT_GLOBLE_RESET                           (1 << 1)
#define AON_CMU_RTC_INTR_H                                  (1 << 2)
#define AON_CMU_CHG_INTR_H                                  (1 << 3)
#define AON_CMU_SOFT_BOOT_MODE(n)                           (((n) & 0xFFFFFFF) << 4)
#define AON_CMU_SOFT_BOOT_MODE_MASK                         (0xFFFFFFF << 4)
#define AON_CMU_SOFT_BOOT_MODE_SHIFT                        (4)

// reg_3c
#define AON_CMU_RESERVED(n)                                 (((n) & 0xFFFFFFFF) << 0)
#define AON_CMU_RESERVED_MASK                               (0xFFFFFFFF << 0)
#define AON_CMU_RESERVED_SHIFT                              (0)

// reg_40
#define AON_CMU_MANUAL_ACLK_ENABLE(n)                       (((n) & 0xFFFFFFFF) << 0)
#define AON_CMU_MANUAL_ACLK_ENABLE_MASK                     (0xFFFFFFFF << 0)
#define AON_CMU_MANUAL_ACLK_ENABLE_SHIFT                    (0)
#define AON_CMU_MANUAL_OCLK_ENABLE(n)                       (((n) & 0xFFFFFFFF) << AON_ACLK_NUM)
#define AON_CMU_MANUAL_OCLK_ENABLE_MASK                     (0xFFFFFFFF << AON_ACLK_NUM)
#define AON_CMU_MANUAL_OCLK_ENABLE_SHIFT                    (AON_ACLK_NUM)

// reg_44
#define AON_CMU_MANUAL_ACLK_DISABLE(n)                      (((n) & 0xFFFFFFFF) << 0)
#define AON_CMU_MANUAL_ACLK_DISABLE_MASK                    (0xFFFFFFFF << 0)
#define AON_CMU_MANUAL_ACLK_DISABLE_SHIFT                   (0)
#define AON_CMU_MANUAL_OCLK_DISABLE(n)                      (((n) & 0xFFFFFFFF) << AON_ACLK_NUM)
#define AON_CMU_MANUAL_OCLK_DISABLE_MASK                    (0xFFFFFFFF << AON_ACLK_NUM)
#define AON_CMU_MANUAL_OCLK_DISABLE_SHIFT                   (AON_ACLK_NUM)

// reg_48
#define AON_CMU_MODE_ACLK(n)                                (((n) & 0xFFFFFFFF) << 0)
#define AON_CMU_MODE_ACLK_MASK                              (0xFFFFFFFF << 0)
#define AON_CMU_MODE_ACLK_SHIFT                             (0)
#define AON_CMU_MODE_OCLK(n)                                (((n) & 0xFFFFFFFF) << AON_ACLK_NUM)
#define AON_CMU_MODE_OCLK_MASK                              (0xFFFFFFFF << AON_ACLK_NUM)
#define AON_CMU_MODE_OCLK_SHIFT                             (AON_ACLK_NUM)

// reg_4c
#define AON_CMU_SEL_CLK_OSC                                 (1 << 0)
#define AON_CMU_BYPASS_LOCK_PLLUSB                          (1 << 1)
#define AON_CMU_BYPASS_LOCK_PLLBB                           (1 << 2)
#define AON_CMU_BYPASS_LOCK_PLLAUD                          (1 << 3)
#define AON_CMU_POL_SPI_CS(n)                               (((n) & 0x7) << 4)
#define AON_CMU_POL_SPI_CS_MASK                             (0x7 << 4)
#define AON_CMU_POL_SPI_CS_SHIFT                            (4)
#define AON_CMU_CFG_SPI_ARB(n)                              (((n) & 0x7) << 7)
#define AON_CMU_CFG_SPI_ARB_MASK                            (0x7 << 7)
#define AON_CMU_CFG_SPI_ARB_SHIFT                           (7)
#define AON_CMU_PU_FLASH_IO                                 (1 << 10)
#define AON_CMU_POR_SLEEP_MODE                              (1 << 11)
#define AON_CMU_LOCK_PLLBB                                  (1 << 12)
#define AON_CMU_LOCK_PLLUSB                                 (1 << 13)
#define AON_CMU_LOCK_PLLAUD                                 (1 << 14)
#define AON_CMU_LOCK_PLLDSI                                 (1 << 15)
#define AON_CMU_SEL_CODEC_DMA_SENS                          (1 << 16)
#define AON_CMU_SEL_BT_PCM_SENS                             (1 << 17)
#define AON_CMU_BYPASS_LOCK_PLLDSI                          (1 << 18)
#define AON_CMU_PU_FLASH1_IO                                (1 << 19)
#define AON_CMU_SEL_I2S_MCLK(n)                             (((n) & 0x3) << 20)
#define AON_CMU_SEL_I2S_MCLK_MASK                           (0x3 << 20)
#define AON_CMU_SEL_I2S_MCLK_SHIFT                          (20)
#define AON_CMU_EN_I2S_MCLK                                 (1 << 22)
#define AON_CMU_SEL_CLK_RC                                  (1 << 23)
#define AON_CMU_SEL_PDM_CLKOUT0_SENS                        (1 << 24)
#define AON_CMU_SEL_PDM_CLKOUT1_SENS                        (1 << 25)
#define AON_CMU_OSC_READY_AON_SYNC                          (1 << 26)
#define AON_CMU_EN_CLK_RC                                   (1 << 27)

// reg_50
#define AON_CMU_CFG_DIV_TIMER00(n)                          (((n) & 0xFFFF) << 0)
#define AON_CMU_CFG_DIV_TIMER00_MASK                        (0xFFFF << 0)
#define AON_CMU_CFG_DIV_TIMER00_SHIFT                       (0)
#define AON_CMU_CFG_DIV_TIMER01(n)                          (((n) & 0xFFFF) << 16)
#define AON_CMU_CFG_DIV_TIMER01_MASK                        (0xFFFF << 16)
#define AON_CMU_CFG_DIV_TIMER01_SHIFT                       (16)

// reg_54
#define AON_CMU_CFG_DIV_PWM0(n)                             (((n) & 0xFFF) << 0)
#define AON_CMU_CFG_DIV_PWM0_MASK                           (0xFFF << 0)
#define AON_CMU_CFG_DIV_PWM0_SHIFT                          (0)
#define AON_CMU_SEL_OSC_PWM0                                (1 << 12)
#define AON_CMU_EN_OSC_PWM0                                 (1 << 13)
#define AON_CMU_CFG_DIV_PWM1(n)                             (((n) & 0xFFF) << 16)
#define AON_CMU_CFG_DIV_PWM1_MASK                           (0xFFF << 16)
#define AON_CMU_CFG_DIV_PWM1_SHIFT                          (16)
#define AON_CMU_SEL_OSC_PWM1                                (1 << 28)
#define AON_CMU_EN_OSC_PWM1                                 (1 << 29)

// reg_58
#define AON_CMU_CFG_DIV_PWM2(n)                             (((n) & 0xFFF) << 0)
#define AON_CMU_CFG_DIV_PWM2_MASK                           (0xFFF << 0)
#define AON_CMU_CFG_DIV_PWM2_SHIFT                          (0)
#define AON_CMU_SEL_OSC_PWM2                                (1 << 12)
#define AON_CMU_EN_OSC_PWM2                                 (1 << 13)
#define AON_CMU_CFG_DIV_PWM3(n)                             (((n) & 0xFFF) << 16)
#define AON_CMU_CFG_DIV_PWM3_MASK                           (0xFFF << 16)
#define AON_CMU_CFG_DIV_PWM3_SHIFT                          (16)
#define AON_CMU_SEL_OSC_PWM3                                (1 << 28)
#define AON_CMU_EN_OSC_PWM3                                 (1 << 29)

// reg_5c
#define AON_CMU_RAM_EMA(n)                                  (((n) & 0x7) << 0)
#define AON_CMU_RAM_EMA_MASK                                (0x7 << 0)
#define AON_CMU_RAM_EMA_SHIFT                               (0)
#define AON_CMU_RAM_EMAW(n)                                 (((n) & 0x3) << 3)
#define AON_CMU_RAM_EMAW_MASK                               (0x3 << 3)
#define AON_CMU_RAM_EMAW_SHIFT                              (3)
#define AON_CMU_RAM_WABL                                    (1 << 5)
#define AON_CMU_RAM_WABLM(n)                                (((n) & 0x7) << 6)
#define AON_CMU_RAM_WABLM_MASK                              (0x7 << 6)
#define AON_CMU_RAM_WABLM_SHIFT                             (6)
#define AON_CMU_RAM_RET1N0(n)                               (((n) & 0x7) << 9)
#define AON_CMU_RAM_RET1N0_MASK                             (0x7 << 9)
#define AON_CMU_RAM_RET1N0_SHIFT                            (9)
#define AON_CMU_RAM_RET2N0(n)                               (((n) & 0x7) << 12)
#define AON_CMU_RAM_RET2N0_MASK                             (0x7 << 12)
#define AON_CMU_RAM_RET2N0_SHIFT                            (12)
#define AON_CMU_RAM_PGEN0(n)                                (((n) & 0x7) << 15)
#define AON_CMU_RAM_PGEN0_MASK                              (0x7 << 15)
#define AON_CMU_RAM_PGEN0_SHIFT                             (15)
#define AON_CMU_RAM_RET1N1(n)                               (((n) & 0x7) << 18)
#define AON_CMU_RAM_RET1N1_MASK                             (0x7 << 18)
#define AON_CMU_RAM_RET1N1_SHIFT                            (18)
#define AON_CMU_RAM_RET2N1(n)                               (((n) & 0x7) << 21)
#define AON_CMU_RAM_RET2N1_MASK                             (0x7 << 21)
#define AON_CMU_RAM_RET2N1_SHIFT                            (21)
#define AON_CMU_RAM_PGEN1(n)                                (((n) & 0x7) << 24)
#define AON_CMU_RAM_PGEN1_MASK                              (0x7 << 24)
#define AON_CMU_RAM_PGEN1_SHIFT                             (24)
#define AON_CMU_RAM_EMAS                                    (1 << 27)
#define AON_CMU_RAM_RAWL                                    (1 << 28)
#define AON_CMU_RAM_RAWLM(n)                                (((n) & 0x3) << 29)
#define AON_CMU_RAM_RAWLM_MASK                              (0x3 << 29)
#define AON_CMU_RAM_RAWLM_SHIFT                             (29)

// reg_60
#define AON_CMU_CFG_DIV_TIMER10(n)                          (((n) & 0xFFFF) << 0)
#define AON_CMU_CFG_DIV_TIMER10_MASK                        (0xFFFF << 0)
#define AON_CMU_CFG_DIV_TIMER10_SHIFT                       (0)
#define AON_CMU_CFG_DIV_TIMER11(n)                          (((n) & 0xFFFF) << 16)
#define AON_CMU_CFG_DIV_TIMER11_MASK                        (0xFFFF << 16)
#define AON_CMU_CFG_DIV_TIMER11_SHIFT                       (16)

// reg_64
#define AON_CMU_POWER_MODE_MCU(n)                           (((n) & 0xFFFFFFFF) << 0)
#define AON_CMU_POWER_MODE_MCU_MASK                         (0xFFFFFFFF << 0)
#define AON_CMU_POWER_MODE_MCU_SHIFT                        (0)

// reg_68
#define AON_CMU_TIMER1_MCU_REG(n)                           (((n) & 0xF) << 0)
#define AON_CMU_TIMER1_MCU_REG_MASK                         (0xF << 0)
#define AON_CMU_TIMER1_MCU_REG_SHIFT                        (0)
#define AON_CMU_TIMER2_MCU_REG(n)                           (((n) & 0xF) << 4)
#define AON_CMU_TIMER2_MCU_REG_MASK                         (0xF << 4)
#define AON_CMU_TIMER2_MCU_REG_SHIFT                        (4)
#define AON_CMU_TIMER3_MCU_REG(n)                           (((n) & 0xF) << 8)
#define AON_CMU_TIMER3_MCU_REG_MASK                         (0xF << 8)
#define AON_CMU_TIMER3_MCU_REG_SHIFT                        (8)
#define AON_CMU_PG_AUTO_EN_MCU_REG                          (1 << 12)
#define AON_CMU_POWER_MODE_BT_DR                            (1 << 13)

// reg_6c
#define AON_CMU_TIMER_PU_OSC_BT(n)                          (((n) & 0xFF) << 0)
#define AON_CMU_TIMER_PU_OSC_BT_MASK                        (0xFF << 0)
#define AON_CMU_TIMER_PU_OSC_BT_SHIFT                       (0)
#define AON_CMU_TIMER_EN_OSC_BT(n)                          (((n) & 0xFF) << 8)
#define AON_CMU_TIMER_EN_OSC_BT_MASK                        (0xFF << 8)
#define AON_CMU_TIMER_EN_OSC_BT_SHIFT                       (8)
#define AON_CMU_OSCX2_OTHR_ON                               (1 << 16)
#define AON_CMU_OSCX4_OTHR_ON                               (1 << 17)
#define AON_CMU_OSCX4_BTC_ON                                (1 << 18)
#define AON_CMU_OSCX2_PSC_ON                                (1 << 19)
#define AON_CMU_OSCX4_PSC_ON                                (1 << 20)
#define AON_CMU_USB_LS_MODE                                 (1 << 21)
#define AON_CMU_USB_EN_RLS                                  (1 << 22)
#define AON_CMU_USB_RESETB_PHY                              (1 << 23)
#define AON_CMU_USB_LOOP_BACK                               (1 << 24)
#define AON_CMU_USB_RX_PD                                   (1 << 25)
#define AON_CMU_USB_VREF_FSRX_H(n)                          (((n) & 0x7) << 26)
#define AON_CMU_USB_VREF_FSRX_H_MASK                        (0x7 << 26)
#define AON_CMU_USB_VREF_FSRX_H_SHIFT                       (26)
#define AON_CMU_USB_VREF_FSRX_L(n)                          (((n) & 0x7) << 29)
#define AON_CMU_USB_VREF_FSRX_L_MASK                        (0x7 << 29)
#define AON_CMU_USB_VREF_FSRX_L_SHIFT                       (29)

// reg_70
#define AON_CMU_CFG_DIV_PWM4(n)                             (((n) & 0xFFF) << 0)
#define AON_CMU_CFG_DIV_PWM4_MASK                           (0xFFF << 0)
#define AON_CMU_CFG_DIV_PWM4_SHIFT                          (0)
#define AON_CMU_SEL_OSC_PWM4                                (1 << 12)
#define AON_CMU_EN_OSC_PWM4                                 (1 << 13)
#define AON_CMU_CFG_DIV_PWM5(n)                             (((n) & 0xFFF) << 14)
#define AON_CMU_CFG_DIV_PWM5_MASK                           (0xFFF << 14)
#define AON_CMU_CFG_DIV_PWM5_SHIFT                          (14)
#define AON_CMU_SEL_OSC_PWM5                                (1 << 26)
#define AON_CMU_EN_OSC_PWM5                                 (1 << 27)

// reg_74
#define AON_CMU_CFG_DIV_PWM6(n)                             (((n) & 0xFFF) << 0)
#define AON_CMU_CFG_DIV_PWM6_MASK                           (0xFFF << 0)
#define AON_CMU_CFG_DIV_PWM6_SHIFT                          (0)
#define AON_CMU_SEL_OSC_PWM6                                (1 << 12)
#define AON_CMU_EN_OSC_PWM6                                 (1 << 13)
#define AON_CMU_CFG_DIV_PWM7(n)                             (((n) & 0xFFF) << 14)
#define AON_CMU_CFG_DIV_PWM7_MASK                           (0xFFF << 14)
#define AON_CMU_CFG_DIV_PWM7_SHIFT                          (14)
#define AON_CMU_SEL_OSC_PWM7                                (1 << 26)
#define AON_CMU_EN_OSC_PWM7                                 (1 << 27)

// reg_78
#define AON_CMU_VTOR_MCU(n)                                 (((n) & 0x1FFFFFF) << 7)
#define AON_CMU_VTOR_MCU_MASK                               (0x1FFFFFF << 7)
#define AON_CMU_VTOR_MCU_SHIFT                              (7)

// reg_7c
#define AON_CMU_OTP_WR_LOCK(n)                              (((n) & 0xFFFF) << 0)
#define AON_CMU_OTP_WR_LOCK_MASK                            (0xFFFF << 0)
#define AON_CMU_OTP_WR_LOCK_SHIFT                           (0)
#define AON_CMU_OTP_WR_UNLOCK                               (1 << 31)

// reg_80
#define AON_CMU_OTP_RD_LOCK(n)                              (((n) & 0xFFFF) << 0)
#define AON_CMU_OTP_RD_LOCK_MASK                            (0xFFFF << 0)
#define AON_CMU_OTP_RD_LOCK_SHIFT                           (0)
#define AON_CMU_OTP_RD_UNLOCK                               (1 << 31)

// reg_84
#define AON_CMU_CFG_PD_STAB_TIMER(n)                        (((n) & 0xF) << 0)
#define AON_CMU_CFG_PD_STAB_TIMER_MASK                      (0xF << 0)
#define AON_CMU_CFG_PD_STAB_TIMER_SHIFT                     (0)
#define AON_CMU_SEL_PSC_FAST                                (1 << 4)
#define AON_CMU_SEL_32K_PSC                                 (1 << 5)
#define AON_CMU_CFG_DIV_PSC(n)                              (((n) & 0xFF) << 6)
#define AON_CMU_CFG_DIV_PSC_MASK                            (0xFF << 6)
#define AON_CMU_CFG_DIV_PSC_SHIFT                           (6)
#define AON_CMU_SEL_PU_OSC_PMU(n)                           (((n) & 0x3) << 14)
#define AON_CMU_SEL_PU_OSC_PMU_MASK                         (0x3 << 14)
#define AON_CMU_SEL_PU_OSC_PMU_SHIFT                        (14)
#define AON_CMU_SEL_PU_OSC_PMU1(n)                          (((n) & 0x3) << 16)
#define AON_CMU_SEL_PU_OSC_PMU1_MASK                        (0x3 << 16)
#define AON_CMU_SEL_PU_OSC_PMU1_SHIFT                       (16)
#define AON_CMU_SEL_PU_OSC_DELAY_PMU1(n)                    (((n) & 0x3) << 18)
#define AON_CMU_SEL_PU_OSC_DELAY_PMU1_MASK                  (0x3 << 18)
#define AON_CMU_SEL_PU_OSC_DELAY_PMU1_SHIFT                 (18)
#define AON_CMU_SEL_PU_OSC_RF(n)                            (((n) & 0x3) << 20)
#define AON_CMU_SEL_PU_OSC_RF_MASK                          (0x3 << 20)
#define AON_CMU_SEL_PU_OSC_RF_SHIFT                         (20)
#define AON_CMU_SEL_PU_OSC_DPA(n)                           (((n) & 0x3) << 22)
#define AON_CMU_SEL_PU_OSC_DPA_MASK                         (0x3 << 22)
#define AON_CMU_SEL_PU_OSC_DPA_SHIFT                        (22)
#define AON_CMU_OSC_MCU_ON                                  (1 << 24)
#define AON_CMU_OSC_SENS_ON                                 (1 << 25)
#define AON_CMU_OSC_OTHR_ON                                 (1 << 26)
#define AON_CMU_OSC_BTC_ON                                  (1 << 27)
#define AON_CMU_OSC_PSC_ON                                  (1 << 28)
#define AON_CMU_SEL_POL_CAP                                 (1 << 29)

// reg_88
#define AON_CMU_RAM2_RET1N0(n)                              (((n) & 0x3FF) << 0)
#define AON_CMU_RAM2_RET1N0_MASK                            (0x3FF << 0)
#define AON_CMU_RAM2_RET1N0_SHIFT                           (0)
#define AON_CMU_RAM2_RET2N0(n)                              (((n) & 0x3FF) << 10)
#define AON_CMU_RAM2_RET2N0_MASK                            (0x3FF << 10)
#define AON_CMU_RAM2_RET2N0_SHIFT                           (10)
#define AON_CMU_RAM2_PGEN0(n)                               (((n) & 0x3FF) << 20)
#define AON_CMU_RAM2_PGEN0_MASK                             (0x3FF << 20)
#define AON_CMU_RAM2_PGEN0_SHIFT                            (20)

// reg_8c
#define AON_CMU_RAM2_RET1N1(n)                              (((n) & 0x3FF) << 0)
#define AON_CMU_RAM2_RET1N1_MASK                            (0x3FF << 0)
#define AON_CMU_RAM2_RET1N1_SHIFT                           (0)
#define AON_CMU_RAM2_RET2N1(n)                              (((n) & 0x3FF) << 10)
#define AON_CMU_RAM2_RET2N1_MASK                            (0x3FF << 10)
#define AON_CMU_RAM2_RET2N1_SHIFT                           (10)
#define AON_CMU_RAM2_PGEN1(n)                               (((n) & 0x3FF) << 20)
#define AON_CMU_RAM2_PGEN1_MASK                             (0x3FF << 20)
#define AON_CMU_RAM2_PGEN1_SHIFT                            (20)

// reg_90
#define AON_CMU_RAM3_RET1N0(n)                              (((n) & 0x3FF) << 0)
#define AON_CMU_RAM3_RET1N0_MASK                            (0x3FF << 0)
#define AON_CMU_RAM3_RET1N0_SHIFT                           (0)
#define AON_CMU_RAM3_RET2N0(n)                              (((n) & 0x3FF) << 10)
#define AON_CMU_RAM3_RET2N0_MASK                            (0x3FF << 10)
#define AON_CMU_RAM3_RET2N0_SHIFT                           (10)
#define AON_CMU_RAM3_PGEN0(n)                               (((n) & 0x3FF) << 20)
#define AON_CMU_RAM3_PGEN0_MASK                             (0x3FF << 20)
#define AON_CMU_RAM3_PGEN0_SHIFT                            (20)

// reg_94
#define AON_CMU_RAM3_RET1N1(n)                              (((n) & 0x3FF) << 0)
#define AON_CMU_RAM3_RET1N1_MASK                            (0x3FF << 0)
#define AON_CMU_RAM3_RET1N1_SHIFT                           (0)
#define AON_CMU_RAM3_RET2N1(n)                              (((n) & 0x3FF) << 10)
#define AON_CMU_RAM3_RET2N1_MASK                            (0x3FF << 10)
#define AON_CMU_RAM3_RET2N1_SHIFT                           (10)
#define AON_CMU_RAM3_PGEN1(n)                               (((n) & 0x3FF) << 20)
#define AON_CMU_RAM3_PGEN1_MASK                             (0x3FF << 20)
#define AON_CMU_RAM3_PGEN1_SHIFT                            (20)

// reg_98
#define AON_CMU_SRAM0_CFG_SEL(n)                            (((n) & 0x3) << 0)
#define AON_CMU_SRAM0_CFG_SEL_MASK                          (0x3 << 0)
#define AON_CMU_SRAM0_CFG_SEL_SHIFT                         (0)
#define AON_CMU_SRAM1_CFG_SEL(n)                            (((n) & 0x3) << 2)
#define AON_CMU_SRAM1_CFG_SEL_MASK                          (0x3 << 2)
#define AON_CMU_SRAM1_CFG_SEL_SHIFT                         (2)
#define AON_CMU_SRAM2_CFG_SEL(n)                            (((n) & 0x3) << 4)
#define AON_CMU_SRAM2_CFG_SEL_MASK                          (0x3 << 4)
#define AON_CMU_SRAM2_CFG_SEL_SHIFT                         (4)
#define AON_CMU_SRAM3_CFG_SEL(n)                            (((n) & 0x3) << 6)
#define AON_CMU_SRAM3_CFG_SEL_MASK                          (0x3 << 6)
#define AON_CMU_SRAM3_CFG_SEL_SHIFT                         (6)
#define AON_CMU_SRAM4_CFG_SEL(n)                            (((n) & 0x3) << 8)
#define AON_CMU_SRAM4_CFG_SEL_MASK                          (0x3 << 8)
#define AON_CMU_SRAM4_CFG_SEL_SHIFT                         (8)
#define AON_CMU_SRAM5_CFG_SEL(n)                            (((n) & 0x3) << 10)
#define AON_CMU_SRAM5_CFG_SEL_MASK                          (0x3 << 10)
#define AON_CMU_SRAM5_CFG_SEL_SHIFT                         (10)
#define AON_CMU_SRAM6_CFG_SEL(n)                            (((n) & 0x3) << 12)
#define AON_CMU_SRAM6_CFG_SEL_MASK                          (0x3 << 12)
#define AON_CMU_SRAM6_CFG_SEL_SHIFT                         (12)
#define AON_CMU_SRAM7_CFG_SEL(n)                            (((n) & 0x3) << 14)
#define AON_CMU_SRAM7_CFG_SEL_MASK                          (0x3 << 14)
#define AON_CMU_SRAM7_CFG_SEL_SHIFT                         (14)
#define AON_CMU_SRAM8_CFG_SEL(n)                            (((n) & 0x3) << 16)
#define AON_CMU_SRAM8_CFG_SEL_MASK                          (0x3 << 16)
#define AON_CMU_SRAM8_CFG_SEL_SHIFT                         (16)

// reg_a0
#define AON_CMU_EN_CLK_PLLBB_MCU_ENABLE                     (1 << 0)
#define AON_CMU_EN_CLK_PLLDSI_MCU_ENABLE                    (1 << 1)
#define AON_CMU_EN_CLK_PLLUSB_MCU_ENABLE                    (1 << 2)
#define AON_CMU_EN_CLK_PLLAUD_MCU_ENABLE                    (1 << 3)
#define AON_CMU_EN_CLK_OSCX4_BT_ENABLE                      (1 << 4)
#define AON_CMU_EN_CLK_PLLBB_BT_ENABLE                      (1 << 5)
#define AON_CMU_EN_CLK_OSC_CODEC_ENABLE                     (1 << 6)
#define AON_CMU_EN_CLK_OSCX2_CODEC_ENABLE                   (1 << 7)
#define AON_CMU_EN_CLK_OSCX4_CODEC_ENABLE                   (1 << 8)
#define AON_CMU_EN_CLK_PLLBB_CODEC_ENABLE                   (1 << 9)
#define AON_CMU_EN_CLK_PLLUSB_CODEC_ENABLE                  (1 << 10)
#define AON_CMU_EN_CLK_PLLAUD_CODEC_ENABLE                  (1 << 11)
#define AON_CMU_EN_CLK_32K_SENS_ENABLE                      (1 << 12)
#define AON_CMU_EN_CLK_PLLBB_SENS_ENABLE                    (1 << 13)
#define AON_CMU_EN_CLK_PLLAUD_SENS_ENABLE                   (1 << 14)

// reg_a4
#define AON_CMU_EN_CLK_PLLBB_MCU_DISABLE                    (1 << 0)
#define AON_CMU_EN_CLK_PLLDSI_MCU_DISABLE                   (1 << 1)
#define AON_CMU_EN_CLK_PLLUSB_MCU_DISABLE                   (1 << 2)
#define AON_CMU_EN_CLK_PLLAUD_MCU_DISABLE                   (1 << 3)
#define AON_CMU_EN_CLK_OSCX4_BT_DISABLE                     (1 << 4)
#define AON_CMU_EN_CLK_PLLBB_BT_DISABLE                     (1 << 5)
#define AON_CMU_EN_CLK_OSC_CODEC_DISABLE                    (1 << 6)
#define AON_CMU_EN_CLK_OSCX2_CODEC_DISABLE                  (1 << 7)
#define AON_CMU_EN_CLK_OSCX4_CODEC_DISABLE                  (1 << 8)
#define AON_CMU_EN_CLK_PLLBB_CODEC_DISABLE                  (1 << 9)
#define AON_CMU_EN_CLK_PLLUSB_CODEC_DISABLE                 (1 << 10)
#define AON_CMU_EN_CLK_PLLAUD_CODEC_DISABLE                 (1 << 11)
#define AON_CMU_EN_CLK_32K_SENS_DISABLE                     (1 << 12)
#define AON_CMU_EN_CLK_PLLBB_SENS_DISABLE                   (1 << 13)
#define AON_CMU_EN_CLK_PLLAUD_SENS_DISABLE                  (1 << 14)

// reg_a8
#define AON_CMU_VTOR_CORE1(n)                                   (((n) & 0x1FFFFFF) << 7)
#define AON_CMU_VTOR_CORE1_MASK                                 (0x1FFFFFF << 7)
#define AON_CMU_VTOR_CORE1_SHIFT                                (7)

// reg_ac
#define AON_CMU_VTOR_CORE_SENS(n)                           (((n) & 0x1FFFFFF) << 7)
#define AON_CMU_VTOR_CORE_SENS_MASK                         (0x1FFFFFF << 7)
#define AON_CMU_VTOR_CORE_SENS_SHIFT                        (7)

// reg_b0
#define AON_CMU_RAM4_RET1N0(n)                              (((n) & 0x1F) << 0)
#define AON_CMU_RAM4_RET1N0_MASK                            (0x1F << 0)
#define AON_CMU_RAM4_RET1N0_SHIFT                           (0)
#define AON_CMU_RAM4_RET2N0(n)                              (((n) & 0x1F) << 5)
#define AON_CMU_RAM4_RET2N0_MASK                            (0x1F << 5)
#define AON_CMU_RAM4_RET2N0_SHIFT                           (5)
#define AON_CMU_RAM4_PGEN0(n)                               (((n) & 0x1F) << 10)
#define AON_CMU_RAM4_PGEN0_MASK                             (0x1F << 10)
#define AON_CMU_RAM4_PGEN0_SHIFT                            (10)
#define AON_CMU_RAM4_RET1N1(n)                              (((n) & 0x1F) << 15)
#define AON_CMU_RAM4_RET1N1_MASK                            (0x1F << 15)
#define AON_CMU_RAM4_RET1N1_SHIFT                           (15)
#define AON_CMU_RAM4_RET2N1(n)                              (((n) & 0x1F) << 20)
#define AON_CMU_RAM4_RET2N1_MASK                            (0x1F << 20)
#define AON_CMU_RAM4_RET2N1_SHIFT                           (20)
#define AON_CMU_RAM4_PGEN1(n)                               (((n) & 0x1F) << 25)
#define AON_CMU_RAM4_PGEN1_MASK                             (0x1F << 25)
#define AON_CMU_RAM4_PGEN1_SHIFT                            (25)

// reg_b4
#define AON_CMU_BT_RAM_EMA(n)                               (((n) & 0x7) << 0)
#define AON_CMU_BT_RAM_EMA_MASK                             (0x7 << 0)
#define AON_CMU_BT_RAM_EMA_SHIFT                            (0)
#define AON_CMU_BT_RAM_EMAW(n)                              (((n) & 0x3) << 3)
#define AON_CMU_BT_RAM_EMAW_MASK                            (0x3 << 3)
#define AON_CMU_BT_RAM_EMAW_SHIFT                           (3)
#define AON_CMU_BT_RAM_WABL                                 (1 << 5)
#define AON_CMU_BT_RAM_WABLM(n)                             (((n) & 0x7) << 6)
#define AON_CMU_BT_RAM_WABLM_MASK                           (0x7 << 6)
#define AON_CMU_BT_RAM_WABLM_SHIFT                          (6)
#define AON_CMU_BT_RAM_EMAS                                 (1 << 9)
#define AON_CMU_BT_RAM_RAWL                                 (1 << 10)
#define AON_CMU_BT_RAM_RAWLM(n)                             (((n) & 0x3) << 11)
#define AON_CMU_BT_RAM_RAWLM_MASK                           (0x3 << 11)
#define AON_CMU_BT_RAM_RAWLM_SHIFT                          (11)
#define AON_CMU_BT_RF_EMA(n)                                (((n) & 0x7) << 13)
#define AON_CMU_BT_RF_EMA_MASK                              (0x7 << 13)
#define AON_CMU_BT_RF_EMA_SHIFT                             (13)
#define AON_CMU_BT_RF_EMAW(n)                               (((n) & 0x3) << 16)
#define AON_CMU_BT_RF_EMAW_MASK                             (0x3 << 16)
#define AON_CMU_BT_RF_EMAW_SHIFT                            (16)
#define AON_CMU_BT_RF_WABL                                  (1 << 18)
#define AON_CMU_BT_RF_WABLM(n)                              (((n) & 0x3) << 19)
#define AON_CMU_BT_RF_WABLM_MASK                            (0x3 << 19)
#define AON_CMU_BT_RF_WABLM_SHIFT                           (19)
#define AON_CMU_BT_RF_EMAS                                  (1 << 21)
#define AON_CMU_BT_RF_RAWL                                  (1 << 22)
#define AON_CMU_BT_RF_RAWLM(n)                              (((n) & 0x3) << 23)
#define AON_CMU_BT_RF_RAWLM_MASK                            (0x3 << 23)
#define AON_CMU_BT_RF_RAWLM_SHIFT                           (23)

// reg_b8
#define AON_CMU_SENS_RAM_EMA(n)                             (((n) & 0x7) << 0)
#define AON_CMU_SENS_RAM_EMA_MASK                           (0x7 << 0)
#define AON_CMU_SENS_RAM_EMA_SHIFT                          (0)
#define AON_CMU_SENS_RAM_EMAW(n)                            (((n) & 0x3) << 3)
#define AON_CMU_SENS_RAM_EMAW_MASK                          (0x3 << 3)
#define AON_CMU_SENS_RAM_EMAW_SHIFT                         (3)
#define AON_CMU_SENS_RAM_WABL                               (1 << 5)
#define AON_CMU_SENS_RAM_WABLM(n)                           (((n) & 0x7) << 6)
#define AON_CMU_SENS_RAM_WABLM_MASK                         (0x7 << 6)
#define AON_CMU_SENS_RAM_WABLM_SHIFT                        (6)
#define AON_CMU_SENS_RAM_EMAS                               (1 << 9)
#define AON_CMU_SENS_RAM_RAWL                               (1 << 10)
#define AON_CMU_SENS_RAM_RAWLM(n)                           (((n) & 0x3) << 11)
#define AON_CMU_SENS_RAM_RAWLM_MASK                         (0x3 << 11)
#define AON_CMU_SENS_RAM_RAWLM_SHIFT                        (11)
#define AON_CMU_SENS_RF_EMA(n)                              (((n) & 0x7) << 13)
#define AON_CMU_SENS_RF_EMA_MASK                            (0x7 << 13)
#define AON_CMU_SENS_RF_EMA_SHIFT                           (13)
#define AON_CMU_SENS_RF_EMAW(n)                             (((n) & 0x3) << 16)
#define AON_CMU_SENS_RF_EMAW_MASK                           (0x3 << 16)
#define AON_CMU_SENS_RF_EMAW_SHIFT                          (16)
#define AON_CMU_SENS_RF_WABL                                (1 << 18)
#define AON_CMU_SENS_RF_WABLM(n)                            (((n) & 0x3) << 19)
#define AON_CMU_SENS_RF_WABLM_MASK                          (0x3 << 19)
#define AON_CMU_SENS_RF_WABLM_SHIFT                         (19)
#define AON_CMU_SENS_RF_EMAS                                (1 << 21)
#define AON_CMU_SENS_RF_RAWL                                (1 << 22)
#define AON_CMU_SENS_RF_RAWLM(n)                            (((n) & 0x3) << 23)
#define AON_CMU_SENS_RF_RAWLM_MASK                          (0x3 << 23)
#define AON_CMU_SENS_RF_RAWLM_SHIFT                         (23)

// reg_bc
#define AON_CMU_CODEC_RAM_EMA(n)                            (((n) & 0x7) << 0)
#define AON_CMU_CODEC_RAM_EMA_MASK                          (0x7 << 0)
#define AON_CMU_CODEC_RAM_EMA_SHIFT                         (0)
#define AON_CMU_CODEC_RAM_EMAW(n)                           (((n) & 0x3) << 3)
#define AON_CMU_CODEC_RAM_EMAW_MASK                         (0x3 << 3)
#define AON_CMU_CODEC_RAM_EMAW_SHIFT                        (3)
#define AON_CMU_CODEC_RAM_WABL                              (1 << 5)
#define AON_CMU_CODEC_RAM_WABLM(n)                          (((n) & 0x7) << 6)
#define AON_CMU_CODEC_RAM_WABLM_MASK                        (0x7 << 6)
#define AON_CMU_CODEC_RAM_WABLM_SHIFT                       (6)
#define AON_CMU_CODEC_RAM_EMAS                              (1 << 9)
#define AON_CMU_CODEC_RAM_RAWL                              (1 << 10)
#define AON_CMU_CODEC_RAM_RAWLM(n)                          (((n) & 0x3) << 11)
#define AON_CMU_CODEC_RAM_RAWLM_MASK                        (0x3 << 11)
#define AON_CMU_CODEC_RAM_RAWLM_SHIFT                       (11)
#define AON_CMU_CODEC_RF_EMA(n)                             (((n) & 0x7) << 13)
#define AON_CMU_CODEC_RF_EMA_MASK                           (0x7 << 13)
#define AON_CMU_CODEC_RF_EMA_SHIFT                          (13)
#define AON_CMU_CODEC_RF_EMAW(n)                            (((n) & 0x3) << 16)
#define AON_CMU_CODEC_RF_EMAW_MASK                          (0x3 << 16)
#define AON_CMU_CODEC_RF_EMAW_SHIFT                         (16)
#define AON_CMU_CODEC_RF_WABL                               (1 << 18)
#define AON_CMU_CODEC_RF_WABLM(n)                           (((n) & 0x3) << 19)
#define AON_CMU_CODEC_RF_WABLM_MASK                         (0x3 << 19)
#define AON_CMU_CODEC_RF_WABLM_SHIFT                        (19)
#define AON_CMU_CODEC_RF_EMAS                               (1 << 21)
#define AON_CMU_CODEC_RF_RAWL                               (1 << 22)
#define AON_CMU_CODEC_RF_RAWLM(n)                           (((n) & 0x3) << 23)
#define AON_CMU_CODEC_RF_RAWLM_MASK                         (0x3 << 23)
#define AON_CMU_CODEC_RF_RAWLM_SHIFT                        (23)

// reg_c0
#define AON_CMU_RF_EMA(n)                                   (((n) & 0x7) << 0)
#define AON_CMU_RF_EMA_MASK                                 (0x7 << 0)
#define AON_CMU_RF_EMA_SHIFT                                (0)
#define AON_CMU_RF_EMAW(n)                                  (((n) & 0x3) << 3)
#define AON_CMU_RF_EMAW_MASK                                (0x3 << 3)
#define AON_CMU_RF_EMAW_SHIFT                               (3)
#define AON_CMU_RF_WABL                                     (1 << 5)
#define AON_CMU_RF_WABLM(n)                                 (((n) & 0x3) << 6)
#define AON_CMU_RF_WABLM_MASK                               (0x3 << 6)
#define AON_CMU_RF_WABLM_SHIFT                              (6)
#define AON_CMU_RF_EMAS                                     (1 << 8)
#define AON_CMU_RF_RAWL                                     (1 << 9)
#define AON_CMU_RF_RAWLM(n)                                 (((n) & 0x3) << 10)
#define AON_CMU_RF_RAWLM_MASK                               (0x3 << 10)
#define AON_CMU_RF_RAWLM_SHIFT                              (10)

// reg_c4
#define AON_CMU_SOFT_RSTN_MCU_PULSE                         (1 << 0)
#define AON_CMU_SOFT_RSTN_CODEC_PULSE                       (1 << 1)
#define AON_CMU_SOFT_RSTN_SENS_PULSE                        (1 << 2)
#define AON_CMU_SOFT_RSTN_BT_PULSE                          (1 << 3)
#define AON_CMU_SOFT_RSTN_MCUCPU_PULSE                      (1 << 4)
#define AON_CMU_SOFT_RSTN_SENSCPU_PULSE                     (1 << 5)
#define AON_CMU_SOFT_RSTN_BTCPU_PULSE                       (1 << 6)
#define AON_CMU_GLOBAL_RESETN_PULSE                         (1 << 7)

// reg_c8
#define AON_CMU_SOFT_RSTN_MCU_SET                           (1 << 0)
#define AON_CMU_SOFT_RSTN_CODEC_SET                         (1 << 1)
#define AON_CMU_SOFT_RSTN_SENS_SET                          (1 << 2)
#define AON_CMU_SOFT_RSTN_BT_SET                            (1 << 3)
#define AON_CMU_SOFT_RSTN_MCUCPU_SET                        (1 << 4)
#define AON_CMU_SOFT_RSTN_SENSCPU_SET                       (1 << 5)
#define AON_CMU_SOFT_RSTN_BTCPU_SET                         (1 << 6)
#define AON_CMU_GLOBAL_RESETN_SET                           (1 << 7)

// reg_cc
#define AON_CMU_SOFT_RSTN_MCU_CLR                           (1 << 0)
#define AON_CMU_SOFT_RSTN_CODEC_CLR                         (1 << 1)
#define AON_CMU_SOFT_RSTN_SENS_CLR                          (1 << 2)
#define AON_CMU_SOFT_RSTN_BT_CLR                            (1 << 3)
#define AON_CMU_SOFT_RSTN_MCUCPU_CLR                        (1 << 4)
#define AON_CMU_SOFT_RSTN_SENSCPU_CLR                       (1 << 5)
#define AON_CMU_SOFT_RSTN_BTCPU_CLR                         (1 << 6)
#define AON_CMU_GLOBAL_RESETN_CLR                           (1 << 7)

// reg_d0
#define AON_CMU_FLASH0_IODRV(n)                             (((n) & 0x7) << 0)
#define AON_CMU_FLASH0_IODRV_MASK                           (0x7 << 0)
#define AON_CMU_FLASH0_IODRV_SHIFT                          (0)
#define AON_CMU_FLASH0_IORES(n)                             (((n) & 0xF) << 3)
#define AON_CMU_FLASH0_IORES_MASK                           (0xF << 3)
#define AON_CMU_FLASH0_IORES_SHIFT                          (3)
#define AON_CMU_FLASH0_IOEN_DR                              (1 << 7)
#define AON_CMU_FLASH0_IOEN(n)                              (((n) & 0xFF) << 8)
#define AON_CMU_FLASH0_IOEN_MASK                            (0xFF << 8)
#define AON_CMU_FLASH0_IOEN_SHIFT                           (8)
#define AON_CMU_FLASH1_IODRV(n)                             (((n) & 0x7) << 16)
#define AON_CMU_FLASH1_IODRV_MASK                           (0x7 << 16)
#define AON_CMU_FLASH1_IODRV_SHIFT                          (16)
#define AON_CMU_FLASH1_IORES(n)                             (((n) & 0xF) << 19)
#define AON_CMU_FLASH1_IORES_MASK                           (0xF << 19)
#define AON_CMU_FLASH1_IORES_SHIFT                          (19)
#define AON_CMU_FLASH1_IOEN_DR                              (1 << 23)
#define AON_CMU_FLASH1_IOEN(n)                              (((n) & 0xFF) << 24)
#define AON_CMU_FLASH1_IOEN_MASK                            (0xFF << 24)
#define AON_CMU_FLASH1_IOEN_SHIFT                           (24)

// reg_d8
#define AON_CMU_ORESET_SET_16BIT                            (1 << 16)

// reg_dc
#define AON_CMU_ORESET_SET_16BIT                            (1 << 16)

// reg_e0
#define AON_CMU_MANUAL_OCLK_ENABLE_16BIT                    (1 << 16)

// reg_e4
#define AON_CMU_MANUAL_OCLK_DISABLE_16BIT                   (1 << 16)

// reg_e8
#define AON_CMU_MODE_OCLK_16BIT                             (1 << 16)

// reg_f0
#define AON_CMU_DEBUG0(n)                                   (((n) & 0xFFFFFFFF) << 0)
#define AON_CMU_DEBUG0_MASK                                 (0xFFFFFFFF << 0)
#define AON_CMU_DEBUG0_SHIFT                                (0)

// reg_f4
#define AON_CMU_DEBUG1(n)                                   (((n) & 0xFFFFFFFF) << 0)
#define AON_CMU_DEBUG1_MASK                                 (0xFFFFFFFF << 0)
#define AON_CMU_DEBUG1_SHIFT                                (0)

// reg_f8
#define AON_CMU_DEBUG2(n)                                   (((n) & 0xFFFFFFFF) << 0)
#define AON_CMU_DEBUG2_MASK                                 (0xFFFFFFFF << 0)
#define AON_CMU_DEBUG2_SHIFT                                (0)

// reg_fc
#define AON_CMU_EFUSE(n)                                    (((n) & 0xFFFF) << 0)
#define AON_CMU_EFUSE_MASK                                  (0xFFFF << 0)
#define AON_CMU_EFUSE_SHIFT                                 (0)
#define AON_CMU_EFUSE_LOCK                                  (1 << 31)

// reg_108
#define AON_CMU_EN_OSC_CAP                                  (1 << 0)
#define AON_CMU_SEL_OSC_CAP                                 (1 << 1)
#define AON_CMU_CFG_DIV_CAP(n)                              (((n) & 0xFF) << 2)
#define AON_CMU_CFG_DIV_CAP_MASK                            (0xFF << 2)
#define AON_CMU_CFG_DIV_CAP_SHIFT                           (2)
#define AON_CMU_EN_32K_CAP                                  (1 << 10)

// APB and AHB Clocks:
#define AON_ACLK_CMU                                        (1 << 0)
#define AON_ARST_CMU                                        (1 << 0)
#define AON_ACLK_GPIO0                                      (1 << 1)
#define AON_ARST_GPIO0                                      (1 << 1)
#define AON_ACLK_GPIO0_INT                                  (1 << 2)
#define AON_ARST_GPIO0_INT                                  (1 << 2)
#define AON_ACLK_WDT                                        (1 << 3)
#define AON_ARST_WDT                                        (1 << 3)
#define AON_ACLK_PWM                                        (1 << 4)
#define AON_ARST_PWM                                        (1 << 4)
#define AON_ACLK_TIMER0                                     (1 << 5)
#define AON_ARST_TIMER0                                     (1 << 5)
#define AON_ACLK_PSC                                        (1 << 6)
#define AON_ARST_PSC                                        (1 << 6)
#define AON_ACLK_IOMUX                                      (1 << 7)
#define AON_ARST_IOMUX                                      (1 << 7)
#define AON_ACLK_APBC                                       (1 << 8)
#define AON_ARST_APBC                                       (1 << 8)
#define AON_ACLK_H2H_MCU                                    (1 << 9)
#define AON_ARST_H2H_MCU                                    (1 << 9)
#define AON_ACLK_I2C_SLV                                    (1 << 10)
#define AON_ARST_I2C_SLV                                    (1 << 10)
#define AON_ACLK_TZC                                        (1 << 11)
#define AON_ARST_TZC                                        (1 << 11)
#define AON_ACLK_GPIO1                                      (1 << 12)
#define AON_ARST_GPIO1                                      (1 << 12)
#define AON_ACLK_GPIO1_INT                                  (1 << 13)
#define AON_ARST_GPIO1_INT                                  (1 << 13)
#define AON_ACLK_TIMER1                                     (1 << 14)
#define AON_ARST_TIMER1                                     (1 << 14)
#define AON_ACLK_PWM1                                       (1 << 15)
#define AON_ARST_PWM1                                       (1 << 15)
#define AON_ACLK_CAP                                        (1 << 16)
#define AON_ARST_CAP                                        (1 << 16)
#define AON_ACLK_PPG                                        (1 << 17)
#define AON_ARST_PPG                                        (1 << 17)

// AON other Clocks:
#define AON_OCLK_WDT                                        (1 << 0)
#define AON_ORST_WDT                                        (1 << 0)
#define AON_OCLK_TIMER0                                     (1 << 1)
#define AON_ORST_TIMER0                                     (1 << 1)
#define AON_OCLK_GPIO0                                      (1 << 2)
#define AON_ORST_GPIO0                                      (1 << 2)
#define AON_OCLK_PWM0                                       (1 << 3)
#define AON_ORST_PWM0                                       (1 << 3)
#define AON_OCLK_PWM1                                       (1 << 4)
#define AON_ORST_PWM1                                       (1 << 4)
#define AON_OCLK_PWM2                                       (1 << 5)
#define AON_ORST_PWM2                                       (1 << 5)
#define AON_OCLK_PWM3                                       (1 << 6)
#define AON_ORST_PWM3                                       (1 << 6)
#define AON_OCLK_IOMUX                                      (1 << 7)
#define AON_ORST_IOMUX                                      (1 << 7)
#define AON_OCLK_GPIO1                                      (1 << 8)
#define AON_ORST_GPIO1                                      (1 << 8)
#define AON_OCLK_BTAON                                      (1 << 9)
#define AON_ORST_BTAON                                      (1 << 9)
#define AON_OCLK_PSC                                        (1 << 10)
#define AON_ORST_PSC                                        (1 << 10)
#define AON_OCLK_TIMER1                                     (1 << 11)
#define AON_ORST_TIMER1                                     (1 << 11)
#define AON_OCLK_PWM4                                       (1 << 12)
#define AON_ORST_PWM4                                       (1 << 12)
#define AON_OCLK_PWM5                                       (1 << 13)
#define AON_ORST_PWM5                                       (1 << 13)
#define AON_OCLK_PWM6                                       (1 << 14)
#define AON_ORST_PWM6                                       (1 << 14)
#define AON_OCLK_PWM7                                       (1 << 15)
#define AON_ORST_PWM7                                       (1 << 15)
#define AON_OCLK_USB                                        (1 << 16)
#define AON_ORST_USB                                        (1 << 16)
#define AON_OCLK_CAP                                        (1 << 17)
#define AON_ORST_CAP                                        (1 << 17)
#define AON_OCLK_CAP_32K                                    (1 << 18)
#define AON_ORST_CAP_32K                                    (1 << 18)
#define AON_OCLK_PPG_6M                                     (1 << 19)
#define AON_ORST_PPG_6M                                     (1 << 19)
#define AON_OCLK_PPG_1M                                     (1 << 20)
#define AON_ORST_PPG_1M                                     (1 << 20)
#define AON_OCLK_PPG_CALIB                                  (1 << 21)
#define AON_ORST_PPG_CALIB                                  (1 << 21)
#define AON_OCLK_PPG_32K                                    (1 << 22)
#define AON_ORST_PPG_32K                                    (1 << 22)

#endif

