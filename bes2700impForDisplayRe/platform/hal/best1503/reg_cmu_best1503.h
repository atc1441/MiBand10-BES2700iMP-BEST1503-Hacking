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
#ifndef __REG_CMU_BEST1306_H__
#define __REG_CMU_BEST1306_H__

#include "plat_types.h"

struct CMU_T {
    __IO uint32_t HCLK_ENABLE;      // 0x00
    __IO uint32_t HCLK_DISABLE;     // 0x04
    __IO uint32_t PCLK_ENABLE;      // 0x08
    __IO uint32_t PCLK_DISABLE;     // 0x0C
    __IO uint32_t OCLK_ENABLE;      // 0x10
    __IO uint32_t OCLK_DISABLE;     // 0x14
    __IO uint32_t HCLK_MODE;        // 0x18
    __IO uint32_t PCLK_MODE;        // 0x1C
    __IO uint32_t OCLK_MODE;        // 0x20
    __IO uint32_t REG_RAM_CFG;      // 0x24
    __IO uint32_t HRESET_PULSE;     // 0x28
    __IO uint32_t PRESET_PULSE;     // 0x2C
    __IO uint32_t ORESET_PULSE;     // 0x30
    __IO uint32_t HRESET_SET;       // 0x34
    __IO uint32_t HRESET_CLR;       // 0x38
    __IO uint32_t PRESET_SET;       // 0x3C
    __IO uint32_t PRESET_CLR;       // 0x40
    __IO uint32_t ORESET_SET;       // 0x44
    __IO uint32_t ORESET_CLR;       // 0x48
    __IO uint32_t TIMER0_CLK;       // 0x4C
    __IO uint32_t BOOTMODE;         // 0x50
    __IO uint32_t MCU_TIMER;        // 0x54
    __IO uint32_t SLEEP;            // 0x58
    __IO uint32_t PERIPH_CLK;       // 0x5C
    __IO uint32_t SYS_CLK_ENABLE;   // 0x60
    __IO uint32_t SYS_CLK_DISABLE;  // 0x64
    __IO uint32_t ADMA_CH15_REQ;    // 0x68
    __IO uint32_t REG_RAM_CFG1;     // 0x6C
    __IO uint32_t UART_CLK;         // 0x70
    __IO uint32_t I2C_CLK;          // 0x74
    __IO uint32_t MCU2SENS_MASK0;   // 0x78
    __IO uint32_t MCU2SENS_MASK1;   // 0x7C
    __IO uint32_t WRITE_UNLOCK;     // 0x80
    __IO uint32_t WAKEUP_MASK0;     // 0x84
    __IO uint32_t WAKEUP_MASK1;     // 0x88
    __IO uint32_t WAKEUP_CLK_CFG;   // 0x8C
    __IO uint32_t TIMER1_CLK;       // 0x90
    __IO uint32_t TIMER2_CLK;       // 0x94
    __IO uint32_t CP2MCU_IRQ_SET;   // 0x98
    __IO uint32_t CP2MCU_IRQ_CLR;   // 0x9C
    __IO uint32_t ISIRQ_SET;        // 0xA0
    __IO uint32_t ISIRQ_CLR;        // 0xA4
    __IO uint32_t SYS_DIV;          // 0xA8
    __IO uint32_t RESERVED_0AC;     // 0xAC
    __IO uint32_t MCU2BT_INTMASK0;  // 0xB0
    __IO uint32_t MCU2BT_INTMASK1;  // 0xB4
    __IO uint32_t MCU2CP_IRQ_SET;   // 0xB8
    __IO uint32_t MCU2CP_IRQ_CLR;   // 0xBC
    __IO uint32_t MEMSC[4];         // 0xC0
    __I  uint32_t MEMSC_STATUS;     // 0xD0
    __IO uint32_t ADMA_CH0_4_REQ;   // 0xD4
    __IO uint32_t ADMA_CH5_9_REQ;   // 0xD8
    __IO uint32_t ADMA_CH10_14_REQ; // 0xDC
    __IO uint32_t GDMA_CH0_4_REQ;   // 0xE0
    __IO uint32_t GDMA_CH5_9_REQ;   // 0xE4
    __IO uint32_t GDMA_CH10_14_REQ; // 0xE8
    __IO uint32_t GDMA_CH15_REQ;    // 0xEC
    __IO uint32_t MISC;             // 0xF0
    __IO uint32_t SIMU_RES;         // 0xF4
    __IO uint32_t MISC_0F8;         // 0xF8
    __IO uint32_t RESERVED_0FC;     // 0xFC
    __IO uint32_t DSI_CLK_ENABLE;   // 0x100
    __IO uint32_t DSI_CLK_DISABLE;  // 0x104
    __IO uint32_t CP_VTOR;          // 0x108
    __IO uint32_t I2S0_CLK;         // 0x10C
    __IO uint32_t I2S1_CLK;         // 0x110
    __IO uint32_t REG_RAM_CFG2;     // 0x114
    __IO uint32_t TPORT_IRQ_LEN;    // 0x118
    __IO uint32_t TPORT_CUR_ADDR;   // 0x11C
    __IO uint32_t TPORT_START;      // 0x120
    __IO uint32_t TPORT_END;        // 0x124
    __IO uint32_t MCU2SENS_IRQ_SET; // 0x128
    __IO uint32_t MCU2SENS_IRQ_CLR; // 0x12C
    __IO uint32_t TPORT_CTRL;       // 0x130
    __IO uint32_t RESERVED_134;     // 0x134
    __IO uint32_t RESERVED_138;     // 0x138
    __IO uint32_t RESERVED_13C;     // 0x13C
    __IO uint32_t XCLK_ENABLE;      // 0x140
    __IO uint32_t XCLK_DISABLE;     // 0x144
    __IO uint32_t XCLK_MODE;        // 0x148
    __IO uint32_t XRESET_PULSE;     // 0x14C
    __IO uint32_t XRESET_SET;       // 0x150
    __IO uint32_t XRESET_CLR;       // 0x154
};

struct SAVED_CMU_REGS_T {
    uint32_t HCLK_ENABLE;
    uint32_t PCLK_ENABLE;
    uint32_t OCLK_ENABLE;
    uint32_t HCLK_MODE;
    uint32_t PCLK_MODE;
    uint32_t OCLK_MODE;
    uint32_t REG_RAM_CFG;
    uint32_t HRESET_CLR;
    uint32_t PRESET_CLR;
    uint32_t ORESET_CLR;
    uint32_t TIMER0_CLK;
    uint32_t MCU_TIMER;
    uint32_t SLEEP;
    uint32_t PERIPH_CLK;
    uint32_t SYS_CLK_ENABLE;
    uint32_t ADMA_CH15_REQ;
    uint32_t REG_RAM_CFG1;
    uint32_t UART_CLK;
    uint32_t I2C_CLK;
    uint32_t MCU2SENS_MASK0;
    uint32_t MCU2SENS_MASK1;
    uint32_t WAKEUP_CLK_CFG;
    uint32_t TIMER1_CLK;
    uint32_t TIMER2_CLK;
    uint32_t SYS_DIV;
    uint32_t RESERVED_0AC;
    uint32_t MCU2BT_INTMASK0;
    uint32_t MCU2BT_INTMASK1;
    uint32_t ADMA_CH0_4_REQ;
    uint32_t ADMA_CH5_9_REQ;
    uint32_t ADMA_CH10_14_REQ;
    uint32_t GDMA_CH0_4_REQ;
    uint32_t GDMA_CH5_9_REQ;
    uint32_t GDMA_CH10_14_REQ;
    uint32_t GDMA_CH15_REQ;
    uint32_t MISC;
    uint32_t SIMU_RES;
    uint32_t MISC_0F8;
    uint32_t RESERVED_0FC;
    uint32_t DSI_CLK_ENABLE;
    uint32_t CP_VTOR;
    uint32_t I2S0_CLK;
    uint32_t I2S1_CLK;
    uint32_t REG_RAM_CFG2;
    uint32_t TPORT_IRQ_LEN;
    uint32_t TPORT_CUR_ADDR;
    uint32_t TPORT_START;
    uint32_t TPORT_END;
    uint32_t TPORT_CTRL;
    uint32_t XCLK_ENABLE;
    uint32_t XCLK_MODE;
    uint32_t XRESET_CLR;
};

// reg_000
#define CMU_MANUAL_HCLK_ENABLE(n)                           (((n) & 0xFFFFFFFF) << 0)
#define CMU_MANUAL_HCLK_ENABLE_MASK                         (0xFFFFFFFF << 0)
#define CMU_MANUAL_HCLK_ENABLE_SHIFT                        (0)

// reg_004
#define CMU_MANUAL_HCLK_DISABLE(n)                          (((n) & 0xFFFFFFFF) << 0)
#define CMU_MANUAL_HCLK_DISABLE_MASK                        (0xFFFFFFFF << 0)
#define CMU_MANUAL_HCLK_DISABLE_SHIFT                       (0)

// reg_008
#define CMU_MANUAL_PCLK_ENABLE(n)                           (((n) & 0xFFFFFFFF) << 0)
#define CMU_MANUAL_PCLK_ENABLE_MASK                         (0xFFFFFFFF << 0)
#define CMU_MANUAL_PCLK_ENABLE_SHIFT                        (0)

// reg_00c
#define CMU_MANUAL_PCLK_DISABLE(n)                          (((n) & 0xFFFFFFFF) << 0)
#define CMU_MANUAL_PCLK_DISABLE_MASK                        (0xFFFFFFFF << 0)
#define CMU_MANUAL_PCLK_DISABLE_SHIFT                       (0)

// reg_010
#define CMU_MANUAL_OCLK_ENABLE(n)                           (((n) & 0xFFFFFFFF) << 0)
#define CMU_MANUAL_OCLK_ENABLE_MASK                         (0xFFFFFFFF << 0)
#define CMU_MANUAL_OCLK_ENABLE_SHIFT                        (0)

// reg_014
#define CMU_MANUAL_OCLK_DISABLE(n)                          (((n) & 0xFFFFFFFF) << 0)
#define CMU_MANUAL_OCLK_DISABLE_MASK                        (0xFFFFFFFF << 0)
#define CMU_MANUAL_OCLK_DISABLE_SHIFT                       (0)

// reg_018
#define CMU_MODE_HCLK(n)                                    (((n) & 0xFFFFFFFF) << 0)
#define CMU_MODE_HCLK_MASK                                  (0xFFFFFFFF << 0)
#define CMU_MODE_HCLK_SHIFT                                 (0)

// reg_01c
#define CMU_MODE_PCLK(n)                                    (((n) & 0xFFFFFFFF) << 0)
#define CMU_MODE_PCLK_MASK                                  (0xFFFFFFFF << 0)
#define CMU_MODE_PCLK_SHIFT                                 (0)

// reg_020
#define CMU_MODE_OCLK(n)                                    (((n) & 0xFFFFFFFF) << 0)
#define CMU_MODE_OCLK_MASK                                  (0xFFFFFFFF << 0)
#define CMU_MODE_OCLK_SHIFT                                 (0)

// reg_024
#define CMU_RF_RET1N0(n)                                    (((n) & 0xF) << 0)
#define CMU_RF_RET1N0_MASK                                  (0xF << 0)
#define CMU_RF_RET1N0_SHIFT                                 (0)
#define CMU_RF_RET2N0(n)                                    (((n) & 0xF) << 4)
#define CMU_RF_RET2N0_MASK                                  (0xF << 4)
#define CMU_RF_RET2N0_SHIFT                                 (4)
#define CMU_RF_PGEN0(n)                                     (((n) & 0xF) << 8)
#define CMU_RF_PGEN0_MASK                                   (0xF << 8)
#define CMU_RF_PGEN0_SHIFT                                  (8)
#define CMU_RF_RET1N1(n)                                    (((n) & 0xF) << 12)
#define CMU_RF_RET1N1_MASK                                  (0xF << 12)
#define CMU_RF_RET1N1_SHIFT                                 (12)
#define CMU_RF_RET2N1(n)                                    (((n) & 0xF) << 16)
#define CMU_RF_RET2N1_MASK                                  (0xF << 16)
#define CMU_RF_RET2N1_SHIFT                                 (16)
#define CMU_RF_PGEN1(n)                                     (((n) & 0xF) << 20)
#define CMU_RF_PGEN1_MASK                                   (0xF << 20)
#define CMU_RF_PGEN1_SHIFT                                  (20)
#define CMU_RF_EMAA(n)                                      (((n) & 0x7) << 24)
#define CMU_RF_EMAA_MASK                                    (0x7 << 24)
#define CMU_RF_EMAA_SHIFT                                   (24)
#define CMU_RF_EMAB(n)                                      (((n) & 0x7) << 27)
#define CMU_RF_EMAB_MASK                                    (0x7 << 27)
#define CMU_RF_EMAB_SHIFT                                   (27)
#define CMU_RF_EMASA                                        (1 << 30)

// reg_028
#define CMU_HRESETN_PULSE(n)                                (((n) & 0xFFFFFFFF) << 0)
#define CMU_HRESETN_PULSE_MASK                              (0xFFFFFFFF << 0)
#define CMU_HRESETN_PULSE_SHIFT                             (0)

#define SYS_PRST_NUM                                        21

// reg_02c
#define CMU_PRESETN_PULSE(n)                                (((n) & 0xFFFFFFFF) << 0)
#define CMU_PRESETN_PULSE_MASK                              (0xFFFFFFFF << 0)
#define CMU_PRESETN_PULSE_SHIFT                             (0)
#define CMU_GLOBAL_RESETN_PULSE                             (1 << (SYS_PRST_NUM+1-1))

// reg_030
#define CMU_ORESETN_PULSE(n)                                (((n) & 0xFFFFFFFF) << 0)
#define CMU_ORESETN_PULSE_MASK                              (0xFFFFFFFF << 0)
#define CMU_ORESETN_PULSE_SHIFT                             (0)

// reg_034
#define CMU_HRESETN_SET(n)                                  (((n) & 0xFFFFFFFF) << 0)
#define CMU_HRESETN_SET_MASK                                (0xFFFFFFFF << 0)
#define CMU_HRESETN_SET_SHIFT                               (0)

// reg_038
#define CMU_HRESETN_CLR(n)                                  (((n) & 0xFFFFFFFF) << 0)
#define CMU_HRESETN_CLR_MASK                                (0xFFFFFFFF << 0)
#define CMU_HRESETN_CLR_SHIFT                               (0)

// reg_03c
#define CMU_PRESETN_SET(n)                                  (((n) & 0xFFFFFFFF) << 0)
#define CMU_PRESETN_SET_MASK                                (0xFFFFFFFF << 0)
#define CMU_PRESETN_SET_SHIFT                               (0)
#define CMU_GLOBAL_RESETN_SET                               (1 << (SYS_PRST_NUM+1-1))

// reg_040
#define CMU_PRESETN_CLR(n)                                  (((n) & 0xFFFFFFFF) << 0)
#define CMU_PRESETN_CLR_MASK                                (0xFFFFFFFF << 0)
#define CMU_PRESETN_CLR_SHIFT                               (0)
#define CMU_GLOBAL_RESETN_CLR                               (1 << (SYS_PRST_NUM+1-1))

// reg_044
#define CMU_ORESETN_SET(n)                                  (((n) & 0xFFFFFFFF) << 0)
#define CMU_ORESETN_SET_MASK                                (0xFFFFFFFF << 0)
#define CMU_ORESETN_SET_SHIFT                               (0)

// reg_048
#define CMU_ORESETN_CLR(n)                                  (((n) & 0xFFFFFFFF) << 0)
#define CMU_ORESETN_CLR_MASK                                (0xFFFFFFFF << 0)
#define CMU_ORESETN_CLR_SHIFT                               (0)

// reg_04c
#define CMU_CFG_DIV_TIMER00(n)                              (((n) & 0xFFFF) << 0)
#define CMU_CFG_DIV_TIMER00_MASK                            (0xFFFF << 0)
#define CMU_CFG_DIV_TIMER00_SHIFT                           (0)
#define CMU_CFG_DIV_TIMER01(n)                              (((n) & 0xFFFF) << 16)
#define CMU_CFG_DIV_TIMER01_MASK                            (0xFFFF << 16)
#define CMU_CFG_DIV_TIMER01_SHIFT                           (16)

// reg_050
#define CMU_WATCHDOG_RESET                                  (1 << 0)
#define CMU_SOFT_GLOBLE_RESET                               (1 << 1)
#define CMU_RTC_INTR_H                                      (1 << 2)
#define CMU_CHG_INTR_H                                      (1 << 3)
#define CMU_SOFT_BOOT_MODE(n)                               (((n) & 0xFFFFFFF) << 4)
#define CMU_SOFT_BOOT_MODE_MASK                             (0xFFFFFFF << 4)
#define CMU_SOFT_BOOT_MODE_SHIFT                            (4)

// reg_054
#define CMU_CFG_HCLK_MCU_OFF_TIMER(n)                       (((n) & 0xFF) << 0)
#define CMU_CFG_HCLK_MCU_OFF_TIMER_MASK                     (0xFF << 0)
#define CMU_CFG_HCLK_MCU_OFF_TIMER_SHIFT                    (0)
#define CMU_HCLK_MCU_ENABLE                                 (1 << 8)
#define CMU_RAM_RETN_UP_EARLY                               (1 << 9)
#define CMU_FLS_SEC_MSK_EN                                  (1 << 10)
#define CMU_DEBUG_REG_SEL(n)                                (((n) & 0x7) << 11)
#define CMU_DEBUG_REG_SEL_MASK                              (0x7 << 11)
#define CMU_DEBUG_REG_SEL_SHIFT                             (11)

// reg_058
#define CMU_SLEEP_TIMER(n)                                  (((n) & 0xFFFFFF) << 0)
#define CMU_SLEEP_TIMER_MASK                                (0xFFFFFF << 0)
#define CMU_SLEEP_TIMER_SHIFT                               (0)
#define CMU_SLEEP_TIMER_EN                                  (1 << 24)
#define CMU_DEEPSLEEP_EN                                    (1 << 25)
#define CMU_DEEPSLEEP_ROMRAM_EN                             (1 << 26)
#define CMU_MANUAL_RAM_RETN                                 (1 << 27)
#define CMU_DEEPSLEEP_START                                 (1 << 28)
#define CMU_DEEPSLEEP_MODE                                  (1 << 29)
#define CMU_PU_OSC                                          (1 << 30)
#define CMU_WAKEUP_DEEPSLEEP_L                              (1 << 31)

// reg_05c
#define CMU_CFG_DIV_SDMMC(n)                                (((n) & 0x3) << 0)
#define CMU_CFG_DIV_SDMMC_MASK                              (0x3 << 0)
#define CMU_CFG_DIV_SDMMC_SHIFT                             (0)
#define CMU_SEL_OSCX2_SDMMC                                 (1 << 2)
#define CMU_SEL_PLL_SDMMC                                   (1 << 3)
#define CMU_EN_PLL_SDMMC                                    (1 << 4)
#define CMU_SEL_32K_TIMER(n)                                (((n) & 0x7) << 5)
#define CMU_SEL_32K_TIMER_MASK                              (0x7 << 5)
#define CMU_SEL_32K_TIMER_SHIFT                             (5)
#define CMU_SEL_32K_WDT                                     (1 << 8)
#define CMU_SEL_TIMER_FAST(n)                               (((n) & 0x7) << 9)
#define CMU_SEL_TIMER_FAST_MASK                             (0x7 << 9)
#define CMU_SEL_TIMER_FAST_SHIFT                            (9)
#define CMU_CFG_CLK_OUT(n)                                  (((n) & 0x1F) << 12)
#define CMU_CFG_CLK_OUT_MASK                                (0x1F << 12)
#define CMU_CFG_CLK_OUT_SHIFT                               (12)
#define CMU_SPI_I2C_DMAREQ_SEL                              (1 << 17)
#define CMU_MASK_OBS(n)                                     (((n) & 0x3F) << 18)
#define CMU_MASK_OBS_MASK                                   (0x3F << 18)
#define CMU_MASK_OBS_SHIFT                                  (18)
#define CMU_JTAG_SEL_CP                                     (1 << 24)
#define CMU_BT_PLAYTIME_STAMP_MASK                          (1 << 25)
#define CMU_BT_PLAYTIME_STAMP1_MASK                         (1 << 26)
#define CMU_BT_PLAYTIME_STAMP2_MASK                         (1 << 27)
#define CMU_BT_PLAYTIME_STAMP3_MASK                         (1 << 28)

// reg_060
#define CMU_RSTN_DIV_FLS0_ENABLE                            (1 << 0)
#define CMU_SEL_OSCX4_FLS0_ENABLE                           (1 << 1)
#define CMU_SEL_OSCX2_FLS0_ENABLE                           (1 << 2)
#define CMU_SEL_PLL_FLS0_ENABLE                             (1 << 3)
#define CMU_BYPASS_DIV_FLS0_ENABLE                          (1 << 4)
#define CMU_RSTN_DIV_SYS_ENABLE                             (1 << 5)
#define CMU_SEL_OSC_2_SYS_ENABLE                            (1 << 6)
#define CMU_SEL_OSC_4_SYS_ENABLE                            (1 << 7)
#define CMU_SEL_SLOW_SYS_ENABLE                             (1 << 8)
#define CMU_SEL_OSCX2_SYS_ENABLE                            (1 << 9)
#define CMU_SEL_FAST_SYS_ENABLE                             (1 << 10)
#define CMU_SEL_PLL_SYS_ENABLE                              (1 << 11)
#define CMU_BYPASS_DIV_SYS_ENABLE                           (1 << 12)
#define CMU_EN_OSCX2_ENABLE                                 (1 << 13)
#define CMU_PU_OSCX2_ENABLE                                 (1 << 14)
#define CMU_EN_OSCX4_ENABLE                                 (1 << 15)
#define CMU_PU_OSCX4_ENABLE                                 (1 << 16)
#define CMU_EN_PLL_ENABLE                                   (1 << 17)
#define CMU_PU_PLL_ENABLE                                   (1 << 18)
#define CMU_RSTN_DIV_FLS1_ENABLE                            (1 << 19)
#define CMU_SEL_OSCX4_FLS1_ENABLE                           (1 << 20)
#define CMU_SEL_OSCX2_FLS1_ENABLE                           (1 << 21)
#define CMU_SEL_PLL_FLS1_ENABLE                             (1 << 22)
#define CMU_BYPASS_DIV_FLS1_ENABLE                          (1 << 23)
#define CMU_RSTN_DIV_PSRAM_ENABLE                           (1 << 24)
#define CMU_SEL_OSCX4_PSRAM_ENABLE                          (1 << 25)
#define CMU_SEL_OSCX2_PSRAM_ENABLE                          (1 << 26)
#define CMU_SEL_PLL_PSRAM_ENABLE                            (1 << 27)
#define CMU_BYPASS_DIV_PSRAM_ENABLE                         (1 << 28)
#define CMU_SEL_PLL_FLS0_FAST_ENABLE                        (1 << 29)
#define CMU_SEL_PLL_FLS1_FAST_ENABLE                        (1 << 30)

// reg_064
#define CMU_RSTN_DIV_FLS0_DISABLE                           (1 << 0)
#define CMU_SEL_OSCX4_FLS0_DISABLE                          (1 << 1)
#define CMU_SEL_OSCX2_FLS0_DISABLE                          (1 << 2)
#define CMU_SEL_PLL_FLS0_DISABLE                            (1 << 3)
#define CMU_BYPASS_DIV_FLS0_DISABLE                         (1 << 4)
#define CMU_RSTN_DIV_SYS_DISABLE                            (1 << 5)
#define CMU_SEL_OSC_2_SYS_DISABLE                           (1 << 6)
#define CMU_SEL_OSC_4_SYS_DISABLE                           (1 << 7)
#define CMU_SEL_SLOW_SYS_DISABLE                            (1 << 8)
#define CMU_SEL_OSCX2_SYS_DISABLE                           (1 << 9)
#define CMU_SEL_FAST_SYS_DISABLE                            (1 << 10)
#define CMU_SEL_PLL_SYS_DISABLE                             (1 << 11)
#define CMU_BYPASS_DIV_SYS_DISABLE                          (1 << 12)
#define CMU_EN_OSCX2_DISABLE                                (1 << 13)
#define CMU_PU_OSCX2_DISABLE                                (1 << 14)
#define CMU_EN_OSCX4_DISABLE                                (1 << 15)
#define CMU_PU_OSCX4_DISABLE                                (1 << 16)
#define CMU_EN_PLL_DISABLE                                  (1 << 17)
#define CMU_PU_PLL_DISABLE                                  (1 << 18)
#define CMU_RSTN_DIV_FLS1_DISABLE                           (1 << 19)
#define CMU_SEL_OSCX4_FLS1_DISABLE                          (1 << 20)
#define CMU_SEL_OSCX2_FLS1_DISABLE                          (1 << 21)
#define CMU_SEL_PLL_FLS1_DISABLE                            (1 << 22)
#define CMU_BYPASS_DIV_FLS1_DISABLE                         (1 << 23)
#define CMU_RSTN_DIV_PSRAM_DISABLE                          (1 << 24)
#define CMU_SEL_OSCX4_PSRAM_DISABLE                         (1 << 25)
#define CMU_SEL_OSCX2_PSRAM_DISABLE                         (1 << 26)
#define CMU_SEL_PLL_PSRAM_DISABLE                           (1 << 27)
#define CMU_BYPASS_DIV_PSRAM_DISABLE                        (1 << 28)
#define CMU_SEL_PLL_FLS0_FAST_DISABLE                       (1 << 29)
#define CMU_SEL_PLL_FLS1_FAST_DISABLE                       (1 << 30)

// reg_068
#define CMU_ADMA_CH15_REQ_IDX(n)                            (((n) & 0x3F) << 0)
#define CMU_ADMA_CH15_REQ_IDX_MASK                          (0x3F << 0)
#define CMU_ADMA_CH15_REQ_IDX_SHIFT                         (0)
#define CMU_MASK_OBS_CODEC_CORE1(n)                         (((n) & 0x3F) << 6)
#define CMU_MASK_OBS_CODEC_CORE1_MASK                       (0x3F << 6)
#define CMU_MASK_OBS_CODEC_CORE1_SHIFT                      (6)
#define CMU_MASK_OBS_GPIO_CORE0(n)                          (((n) & 0x3F) << 12)
#define CMU_MASK_OBS_GPIO_CORE0_MASK                        (0x3F << 12)
#define CMU_MASK_OBS_GPIO_CORE0_SHIFT                       (12)
#define CMU_MASK_OBS_GPIO_CORE1(n)                          (((n) & 0x3F) << 18)
#define CMU_MASK_OBS_GPIO_CORE1_MASK                        (0x3F << 18)
#define CMU_MASK_OBS_GPIO_CORE1_SHIFT                       (18)

// reg_06c
#define CMU_ROM_RTSEL(n)                                    (((n) & 0x3) << 0)
#define CMU_ROM_RTSEL_MASK                                  (0x3 << 0)
#define CMU_ROM_RTSEL_SHIFT                                 (0)
#define CMU_ROM_PTSEL(n)                                    (((n) & 0x3) << 2)
#define CMU_ROM_PTSEL_MASK                                  (0x3 << 2)
#define CMU_ROM_PTSEL_SHIFT                                 (2)
#define CMU_ROM_TRB(n)                                      (((n) & 0x3) << 4)
#define CMU_ROM_TRB_MASK                                    (0x3 << 4)
#define CMU_ROM_TRB_SHIFT                                   (4)
#define CMU_ROM_PGEN                                        (1 << 6)
#define CMU_RF2_RET1N0(n)                                   (((n) & 0x3) << 7)
#define CMU_RF2_RET1N0_MASK                                 (0x3 << 7)
#define CMU_RF2_RET1N0_SHIFT                                (7)
#define CMU_RF2_RET2N0(n)                                   (((n) & 0x3) << 9)
#define CMU_RF2_RET2N0_MASK                                 (0x3 << 9)
#define CMU_RF2_RET2N0_SHIFT                                (9)
#define CMU_RF2_PGEN0(n)                                    (((n) & 0x3) << 11)
#define CMU_RF2_PGEN0_MASK                                  (0x3 << 11)
#define CMU_RF2_PGEN0_SHIFT                                 (11)
#define CMU_RF2_RET1N1(n)                                   (((n) & 0x3) << 13)
#define CMU_RF2_RET1N1_MASK                                 (0x3 << 13)
#define CMU_RF2_RET1N1_SHIFT                                (13)
#define CMU_RF2_RET2N1(n)                                   (((n) & 0x3) << 15)
#define CMU_RF2_RET2N1_MASK                                 (0x3 << 15)
#define CMU_RF2_RET2N1_SHIFT                                (15)
#define CMU_RF2_PGEN1(n)                                    (((n) & 0x3) << 17)
#define CMU_RF2_PGEN1_MASK                                  (0x3 << 17)
#define CMU_RF2_PGEN1_SHIFT                                 (17)
#define CMU_RF_EMA(n)                                       (((n) & 0x7) << 19)
#define CMU_RF_EMA_MASK                                     (0x7 << 19)
#define CMU_RF_EMA_SHIFT                                    (19)
#define CMU_RF_EMAW(n)                                      (((n) & 0x3) << 22)
#define CMU_RF_EMAW_MASK                                    (0x3 << 22)
#define CMU_RF_EMAW_SHIFT                                   (22)
#define CMU_RF_WABL                                         (1 << 24)
#define CMU_RF_WABLM(n)                                     (((n) & 0x3) << 25)
#define CMU_RF_WABLM_MASK                                   (0x3 << 25)
#define CMU_RF_WABLM_SHIFT                                  (25)
#define CMU_RF_EMAS                                         (1 << 27)
#define CMU_RF_RAWL                                         (1 << 28)
#define CMU_RF_RAWLM(n)                                     (((n) & 0x3) << 29)
#define CMU_RF_RAWLM_MASK                                   (0x3 << 29)
#define CMU_RF_RAWLM_SHIFT                                  (29)

// reg_070
#define CMU_CFG_DIV_UART0(n)                                (((n) & 0x3) << 0)
#define CMU_CFG_DIV_UART0_MASK                              (0x3 << 0)
#define CMU_CFG_DIV_UART0_SHIFT                             (0)
#define CMU_SEL_OSCX2_UART0                                 (1 << 2)
#define CMU_SEL_PLL_UART0                                   (1 << 3)
#define CMU_EN_PLL_UART0                                    (1 << 4)
#define CMU_CFG_DIV_UART1(n)                                (((n) & 0x3) << 5)
#define CMU_CFG_DIV_UART1_MASK                              (0x3 << 5)
#define CMU_CFG_DIV_UART1_SHIFT                             (5)
#define CMU_SEL_OSCX2_UART1                                 (1 << 7)
#define CMU_SEL_PLL_UART1                                   (1 << 8)
#define CMU_EN_PLL_UART1                                    (1 << 9)
#define CMU_CFG_DIV_UART2(n)                                (((n) & 0x3) << 10)
#define CMU_CFG_DIV_UART2_MASK                              (0x3 << 10)
#define CMU_CFG_DIV_UART2_SHIFT                             (10)
#define CMU_SEL_OSCX2_UART2                                 (1 << 12)
#define CMU_SEL_PLL_UART2                                   (1 << 13)
#define CMU_EN_PLL_UART2                                    (1 << 14)
#define CMU_CFG_DIV_DSI(n)                                  (((n) & 0x3) << 15)
#define CMU_CFG_DIV_DSI_MASK                                (0x3 << 15)
#define CMU_CFG_DIV_DSI_SHIFT                               (15)
#define CMU_SEL_PLL_SOURCE(n)                               (((n) & 0x3) << 17)
#define CMU_SEL_PLL_SOURCE_MASK                             (0x3 << 17)
#define CMU_SEL_PLL_SOURCE_SHIFT                            (17)
#define CMU_MINIMA_BYPASS                                   (1 << 19)
#define CMU_MINIMA_EXTEND_180DEG                            (1 << 20)
#define CMU_MINIMA_ENABLE_FCLK                              (1 << 21)
#define CMU_MINIMA_CORE0_EN                                 (1 << 22)
#define CMU_MINIMA_CORE1_EN                                 (1 << 23)

// reg_074
#define CMU_CFG_DIV_I2C(n)                                  (((n) & 0x3) << 0)
#define CMU_CFG_DIV_I2C_MASK                                (0x3 << 0)
#define CMU_CFG_DIV_I2C_SHIFT                               (0)
#define CMU_SEL_OSC_I2C                                     (1 << 2)
#define CMU_SEL_OSCX2_I2C                                   (1 << 3)
#define CMU_SEL_PLL_I2C                                     (1 << 4)
#define CMU_EN_PLL_I2C                                      (1 << 5)
#define CMU_POL_CLK_PCM_IN                                  (1 << 6)
#define CMU_SEL_PCM_CLKIN                                   (1 << 7)
#define CMU_EN_CLK_PCM_OUT                                  (1 << 8)
#define CMU_POL_CLK_PCM_OUT                                 (1 << 9)
#define CMU_POL_CLK_I2S0_IN                                 (1 << 10)
#define CMU_SEL_I2S0_CLKIN                                  (1 << 11)
#define CMU_EN_CLK_I2S0_OUT                                 (1 << 12)
#define CMU_POL_CLK_I2S0_OUT                                (1 << 13)
#define CMU_FORCE_PU_OFF                                    (1 << 14)
#define CMU_LOCK_CPU_EN                                     (1 << 15)
#define CMU_SEL_ROM_FAST                                    (1 << 16)
#define CMU_POL_CLK_I2S1_IN                                 (1 << 17)
#define CMU_SEL_I2S1_CLKIN                                  (1 << 18)
#define CMU_EN_CLK_I2S1_OUT                                 (1 << 19)
#define CMU_POL_CLK_I2S1_OUT                                (1 << 20)
#define CMU_EN_I2S_MCLK                                     (1 << 21)
#define CMU_SEL_I2S_MCLK(n)                                 (((n) & 0x7) << 22)
#define CMU_SEL_I2S_MCLK_MASK                               (0x7 << 22)
#define CMU_SEL_I2S_MCLK_SHIFT                              (22)
#define CMU_POL_CLK_DSI_IN                                  (1 << 25)

// reg_078
#define CMU_MCU2SENS_INTISR_MASK0(n)                        (((n) & 0xFFFFFFFF) << 0)
#define CMU_MCU2SENS_INTISR_MASK0_MASK                      (0xFFFFFFFF << 0)
#define CMU_MCU2SENS_INTISR_MASK0_SHIFT                     (0)

// reg_07c
#define CMU_MCU2SENS_INTISR_MASK1(n)                        (((n) & 0xFFFFFFFF) << 0)
#define CMU_MCU2SENS_INTISR_MASK1_MASK                      (0xFFFFFFFF << 0)
#define CMU_MCU2SENS_INTISR_MASK1_SHIFT                     (0)

// reg_080
#define CMU_WRITE_UNLOCK_H                                  (1 << 0)
#define CMU_WRITE_UNLOCK_STATUS                             (1 << 1)

// reg_084
#define CMU_WAKEUP_IRQ_MASK0(n)                             (((n) & 0xFFFFFFFF) << 0)
#define CMU_WAKEUP_IRQ_MASK0_MASK                           (0xFFFFFFFF << 0)
#define CMU_WAKEUP_IRQ_MASK0_SHIFT                          (0)

// reg_088
#define CMU_WAKEUP_IRQ_MASK1(n)                             (((n) & 0xFFFFFFFF) << 0)
#define CMU_WAKEUP_IRQ_MASK1_MASK                           (0xFFFFFFFF << 0)
#define CMU_WAKEUP_IRQ_MASK1_SHIFT                          (0)

// reg_08c
#define CMU_TIMER_WT26(n)                                   (((n) & 0xFF) << 0)
#define CMU_TIMER_WT26_MASK                                 (0xFF << 0)
#define CMU_TIMER_WT26_SHIFT                                (0)
#define CMU_TIMER_WTPLL(n)                                  (((n) & 0xF) << 8)
#define CMU_TIMER_WTPLL_MASK                                (0xF << 8)
#define CMU_TIMER_WTPLL_SHIFT                               (8)
#define CMU_LPU_AUTO_SWITCH26                               (1 << 12)
#define CMU_LPU_AUTO_SWITCHPLL                              (1 << 13)
#define CMU_LPU_STATUS_26M                                  (1 << 14)
#define CMU_LPU_STATUS_PLL                                  (1 << 15)
#define CMU_LPU_AUTO_MID                                    (1 << 16)

// reg_090
#define CMU_CFG_DIV_TIMER10(n)                              (((n) & 0xFFFF) << 0)
#define CMU_CFG_DIV_TIMER10_MASK                            (0xFFFF << 0)
#define CMU_CFG_DIV_TIMER10_SHIFT                           (0)
#define CMU_CFG_DIV_TIMER11(n)                              (((n) & 0xFFFF) << 16)
#define CMU_CFG_DIV_TIMER11_MASK                            (0xFFFF << 16)
#define CMU_CFG_DIV_TIMER11_SHIFT                           (16)

// reg_094
#define CMU_CFG_DIV_TIMER20(n)                              (((n) & 0xFFFF) << 0)
#define CMU_CFG_DIV_TIMER20_MASK                            (0xFFFF << 0)
#define CMU_CFG_DIV_TIMER20_SHIFT                           (0)
#define CMU_CFG_DIV_TIMER21(n)                              (((n) & 0xFFFF) << 16)
#define CMU_CFG_DIV_TIMER21_MASK                            (0xFFFF << 16)
#define CMU_CFG_DIV_TIMER21_SHIFT                           (16)

// reg_098
#define CMU_MCU2CP_DATA_DONE_SET                            (1 << 0)
#define CMU_MCU2CP_DATA1_DONE_SET                           (1 << 1)
#define CMU_MCU2CP_DATA2_DONE_SET                           (1 << 2)
#define CMU_MCU2CP_DATA3_DONE_SET                           (1 << 3)
#define CMU_CP2MCU_DATA_IND_SET                             (1 << 4)
#define CMU_CP2MCU_DATA1_IND_SET                            (1 << 5)
#define CMU_CP2MCU_DATA2_IND_SET                            (1 << 6)
#define CMU_CP2MCU_DATA3_IND_SET                            (1 << 7)

// reg_09c
#define CMU_MCU2CP_DATA_DONE_CLR                            (1 << 0)
#define CMU_MCU2CP_DATA1_DONE_CLR                           (1 << 1)
#define CMU_MCU2CP_DATA2_DONE_CLR                           (1 << 2)
#define CMU_MCU2CP_DATA3_DONE_CLR                           (1 << 3)
#define CMU_CP2MCU_DATA_IND_CLR                             (1 << 4)
#define CMU_CP2MCU_DATA1_IND_CLR                            (1 << 5)
#define CMU_CP2MCU_DATA2_IND_CLR                            (1 << 6)
#define CMU_CP2MCU_DATA3_IND_CLR                            (1 << 7)

// reg_0a0
#define CMU_BT2MCU_DATA_DONE_SET                            (1 << 0)
#define CMU_BT2MCU_DATA1_DONE_SET                           (1 << 1)
#define CMU_MCU2BT_DATA_IND_SET                             (1 << 2)
#define CMU_MCU2BT_DATA1_IND_SET                            (1 << 3)
#define CMU_BT_ALLIRQ_MASK_SET                              (1 << 4)
#define CMU_BT_PLAYTIME_STAMP_INTR                          (1 << 5)
#define CMU_BT_PLAYTIME_STAMP1_INTR                         (1 << 6)
#define CMU_BT_PLAYTIME_STAMP2_INTR                         (1 << 7)
#define CMU_BT_PLAYTIME_STAMP3_INTR                         (1 << 8)
#define CMU_BT_PLAYTIME_STAMP_INTR_MSK                      (1 << 9)
#define CMU_BT_PLAYTIME_STAMP1_INTR_MSK                     (1 << 10)
#define CMU_BT_PLAYTIME_STAMP2_INTR_MSK                     (1 << 11)
#define CMU_BT_PLAYTIME_STAMP3_INTR_MSK                     (1 << 12)
#define CMU_BT2MCU_DATA_MSK_SET                             (1 << 13)
#define CMU_BT2MCU_DATA1_MSK_SET                            (1 << 14)
#define CMU_MCU2BT_DATA_MSK_SET                             (1 << 15)
#define CMU_MCU2BT_DATA1_MSK_SET                            (1 << 16)
#define CMU_BT2MCU_DATA_INTR                                (1 << 17)
#define CMU_BT2MCU_DATA1_INTR                               (1 << 18)
#define CMU_MCU2BT_DATA_INTR                                (1 << 19)
#define CMU_MCU2BT_DATA1_INTR                               (1 << 20)
#define CMU_BT2MCU_DATA_INTR_MSK                            (1 << 21)
#define CMU_BT2MCU_DATA1_INTR_MSK                           (1 << 22)
#define CMU_MCU2BT_DATA_INTR_MSK                            (1 << 23)
#define CMU_MCU2BT_DATA1_INTR_MSK                           (1 << 24)
#define CMU_SENS_ALLIRQ_MASK_SET                            (1 << 25)

// reg_0a4
#define CMU_BT2MCU_DATA_DONE_CLR                            (1 << 0)
#define CMU_BT2MCU_DATA1_DONE_CLR                           (1 << 1)
#define CMU_MCU2BT_DATA_IND_CLR                             (1 << 2)
#define CMU_MCU2BT_DATA1_IND_CLR                            (1 << 3)
#define CMU_BT_ALLIRQ_MASK_CLR                              (1 << 4)
#define CMU_BT_PLAYTIME_STAMP_INTR_CLR                      (1 << 5)
#define CMU_BT_PLAYTIME_STAMP1_INTR_CLR                     (1 << 6)
#define CMU_BT_PLAYTIME_STAMP2_INTR_CLR                     (1 << 7)
#define CMU_BT_PLAYTIME_STAMP3_INTR_CLR                     (1 << 8)
#define CMU_BT2MCU_DATA_MSK_CLR                             (1 << 13)
#define CMU_BT2MCU_DATA1_MSK_CLR                            (1 << 14)
#define CMU_MCU2BT_DATA_MSK_CLR                             (1 << 15)
#define CMU_MCU2BT_DATA1_MSK_CLR                            (1 << 16)
#define CMU_SENS_ALLIRQ_MASK_CLR                            (1 << 25)

// reg_0a8
#define CMU_CFG_DIV_SYS(n)                                  (((n) & 0x3) << 0)
#define CMU_CFG_DIV_SYS_MASK                                (0x3 << 0)
#define CMU_CFG_DIV_SYS_SHIFT                               (0)
#define CMU_SEL_SMP_MCU(n)                                  (((n) & 0xf) << 2)
#define CMU_SEL_SMP_MCU_MASK                                (0xf << 2)
#define CMU_SEL_SMP_MCU_SHIFT                               (2)
#define CMU_CFG_DIV_FLS0(n)                                 (((n) & 0x3) << 6)
#define CMU_CFG_DIV_FLS0_MASK                               (0x3 << 6)
#define CMU_CFG_DIV_FLS0_SHIFT                              (6)
#define CMU_CFG_DIV_FLS1(n)                                 (((n) & 0x3) << 8)
#define CMU_CFG_DIV_FLS1_MASK                               (0x3 << 8)
#define CMU_CFG_DIV_FLS1_SHIFT                              (8)
#define CMU_SEL_USB_6M                                      (1 << 10)
#define CMU_SEL_USB_SRC                                     (1 << 11)
#define CMU_POL_CLK_USB                                     (1 << 12)
#define CMU_USB_ID                                          (1 << 13)
#define CMU_CFG_DIV_PCLK(n)                                 (((n) & 0x3) << 14)
#define CMU_CFG_DIV_PCLK_MASK                               (0x3 << 14)
#define CMU_CFG_DIV_PCLK_SHIFT                              (14)
#define CMU_CFG_DIV_SPI1(n)                                 (((n) & 0x3) << 16)
#define CMU_CFG_DIV_SPI1_MASK                               (0x3 << 16)
#define CMU_CFG_DIV_SPI1_SHIFT                              (16)
#define CMU_SEL_OSCX2_SPI1                                  (1 << 18)
#define CMU_SEL_PLL_SPI1                                    (1 << 19)
#define CMU_EN_PLL_SPI1                                     (1 << 20)
#define CMU_SEL_OSCX2_SPI2                                  (1 << 21)
#define CMU_DSD_PCM_DMAREQ_SEL                              (1 << 22)
#define CMU_CFG_DIV_PSRAM(n)                                (((n) & 0x3) << 23)
#define CMU_CFG_DIV_PSRAM_MASK                              (0x3 << 23)
#define CMU_CFG_DIV_PSRAM_SHIFT                             (23)

// reg_0b0
#define CMU_MCU2BT_INTISR_MASK0(n)                          (((n) & 0xFFFFFFFF) << 0)
#define CMU_MCU2BT_INTISR_MASK0_MASK                        (0xFFFFFFFF << 0)
#define CMU_MCU2BT_INTISR_MASK0_SHIFT                       (0)

// reg_0b4
#define CMU_MCU2BT_INTISR_MASK1(n)                          (((n) & 0xFFFFFFFF) << 0)
#define CMU_MCU2BT_INTISR_MASK1_MASK                        (0xFFFFFFFF << 0)
#define CMU_MCU2BT_INTISR_MASK1_SHIFT                       (0)

// reg_0b8
#define CMU_CP2MCU_DATA_DONE_SET                            (1 << 0)
#define CMU_CP2MCU_DATA1_DONE_SET                           (1 << 1)
#define CMU_CP2MCU_DATA2_DONE_SET                           (1 << 2)
#define CMU_CP2MCU_DATA3_DONE_SET                           (1 << 3)
#define CMU_MCU2CP_DATA_IND_SET                             (1 << 4)
#define CMU_MCU2CP_DATA1_IND_SET                            (1 << 5)
#define CMU_MCU2CP_DATA2_IND_SET                            (1 << 6)
#define CMU_MCU2CP_DATA3_IND_SET                            (1 << 7)

// reg_0bc
#define CMU_CP2MCU_DATA_DONE_CLR                            (1 << 0)
#define CMU_CP2MCU_DATA1_DONE_CLR                           (1 << 1)
#define CMU_CP2MCU_DATA2_DONE_CLR                           (1 << 2)
#define CMU_CP2MCU_DATA3_DONE_CLR                           (1 << 3)
#define CMU_MCU2CP_DATA_IND_CLR                             (1 << 4)
#define CMU_MCU2CP_DATA1_IND_CLR                            (1 << 5)
#define CMU_MCU2CP_DATA2_IND_CLR                            (1 << 6)
#define CMU_MCU2CP_DATA3_IND_CLR                            (1 << 7)

// reg_0c0
#define CMU_MEMSC0                                          (1 << 0)

// reg_0c4
#define CMU_MEMSC1                                          (1 << 0)

// reg_0c8
#define CMU_MEMSC2                                          (1 << 0)

// reg_0cc
#define CMU_MEMSC3                                          (1 << 0)

// reg_0d0
#define CMU_MEMSC_STATUS0                                   (1 << 0)
#define CMU_MEMSC_STATUS1                                   (1 << 1)
#define CMU_MEMSC_STATUS2                                   (1 << 2)
#define CMU_MEMSC_STATUS3                                   (1 << 3)

// reg_0d4
#define CMU_ADMA_CH0_REQ_IDX(n)                             (((n) & 0x3F) << 0)
#define CMU_ADMA_CH0_REQ_IDX_MASK                           (0x3F << 0)
#define CMU_ADMA_CH0_REQ_IDX_SHIFT                          (0)
#define CMU_ADMA_CH1_REQ_IDX(n)                             (((n) & 0x3F) << 6)
#define CMU_ADMA_CH1_REQ_IDX_MASK                           (0x3F << 6)
#define CMU_ADMA_CH1_REQ_IDX_SHIFT                          (6)
#define CMU_ADMA_CH2_REQ_IDX(n)                             (((n) & 0x3F) << 12)
#define CMU_ADMA_CH2_REQ_IDX_MASK                           (0x3F << 12)
#define CMU_ADMA_CH2_REQ_IDX_SHIFT                          (12)
#define CMU_ADMA_CH3_REQ_IDX(n)                             (((n) & 0x3F) << 18)
#define CMU_ADMA_CH3_REQ_IDX_MASK                           (0x3F << 18)
#define CMU_ADMA_CH3_REQ_IDX_SHIFT                          (18)
#define CMU_ADMA_CH4_REQ_IDX(n)                             (((n) & 0x3F) << 24)
#define CMU_ADMA_CH4_REQ_IDX_MASK                           (0x3F << 24)
#define CMU_ADMA_CH4_REQ_IDX_SHIFT                          (24)

// reg_0d8
#define CMU_ADMA_CH5_REQ_IDX(n)                             (((n) & 0x3F) << 0)
#define CMU_ADMA_CH5_REQ_IDX_MASK                           (0x3F << 0)
#define CMU_ADMA_CH5_REQ_IDX_SHIFT                          (0)
#define CMU_ADMA_CH6_REQ_IDX(n)                             (((n) & 0x3F) << 6)
#define CMU_ADMA_CH6_REQ_IDX_MASK                           (0x3F << 6)
#define CMU_ADMA_CH6_REQ_IDX_SHIFT                          (6)
#define CMU_ADMA_CH7_REQ_IDX(n)                             (((n) & 0x3F) << 12)
#define CMU_ADMA_CH7_REQ_IDX_MASK                           (0x3F << 12)
#define CMU_ADMA_CH7_REQ_IDX_SHIFT                          (12)
#define CMU_ADMA_CH8_REQ_IDX(n)                             (((n) & 0x3F) << 18)
#define CMU_ADMA_CH8_REQ_IDX_MASK                           (0x3F << 18)
#define CMU_ADMA_CH8_REQ_IDX_SHIFT                          (18)
#define CMU_ADMA_CH9_REQ_IDX(n)                             (((n) & 0x3F) << 24)
#define CMU_ADMA_CH9_REQ_IDX_MASK                           (0x3F << 24)
#define CMU_ADMA_CH9_REQ_IDX_SHIFT                          (24)

// reg_0dc
#define CMU_ADMA_CH10_REQ_IDX(n)                            (((n) & 0x3F) << 0)
#define CMU_ADMA_CH10_REQ_IDX_MASK                          (0x3F << 0)
#define CMU_ADMA_CH10_REQ_IDX_SHIFT                         (0)
#define CMU_ADMA_CH11_REQ_IDX(n)                            (((n) & 0x3F) << 6)
#define CMU_ADMA_CH11_REQ_IDX_MASK                          (0x3F << 6)
#define CMU_ADMA_CH11_REQ_IDX_SHIFT                         (6)
#define CMU_ADMA_CH12_REQ_IDX(n)                            (((n) & 0x3F) << 12)
#define CMU_ADMA_CH12_REQ_IDX_MASK                          (0x3F << 12)
#define CMU_ADMA_CH12_REQ_IDX_SHIFT                         (12)
#define CMU_ADMA_CH13_REQ_IDX(n)                            (((n) & 0x3F) << 18)
#define CMU_ADMA_CH13_REQ_IDX_MASK                          (0x3F << 18)
#define CMU_ADMA_CH13_REQ_IDX_SHIFT                         (18)
#define CMU_ADMA_CH14_REQ_IDX(n)                            (((n) & 0x3F) << 24)
#define CMU_ADMA_CH14_REQ_IDX_MASK                          (0x3F << 24)
#define CMU_ADMA_CH14_REQ_IDX_SHIFT                         (24)

// reg_0e0
#define CMU_GDMA_CH0_REQ_IDX(n)                             (((n) & 0x3F) << 0)
#define CMU_GDMA_CH0_REQ_IDX_MASK                           (0x3F << 0)
#define CMU_GDMA_CH0_REQ_IDX_SHIFT                          (0)
#define CMU_GDMA_CH1_REQ_IDX(n)                             (((n) & 0x3F) << 6)
#define CMU_GDMA_CH1_REQ_IDX_MASK                           (0x3F << 6)
#define CMU_GDMA_CH1_REQ_IDX_SHIFT                          (6)
#define CMU_GDMA_CH2_REQ_IDX(n)                             (((n) & 0x3F) << 12)
#define CMU_GDMA_CH2_REQ_IDX_MASK                           (0x3F << 12)
#define CMU_GDMA_CH2_REQ_IDX_SHIFT                          (12)
#define CMU_GDMA_CH3_REQ_IDX(n)                             (((n) & 0x3F) << 18)
#define CMU_GDMA_CH3_REQ_IDX_MASK                           (0x3F << 18)
#define CMU_GDMA_CH3_REQ_IDX_SHIFT                          (18)
#define CMU_GDMA_CH4_REQ_IDX(n)                             (((n) & 0x3F) << 24)
#define CMU_GDMA_CH4_REQ_IDX_MASK                           (0x3F << 24)
#define CMU_GDMA_CH4_REQ_IDX_SHIFT                          (24)

// reg_0e4
#define CMU_GDMA_CH5_REQ_IDX(n)                             (((n) & 0x3F) << 0)
#define CMU_GDMA_CH5_REQ_IDX_MASK                           (0x3F << 0)
#define CMU_GDMA_CH5_REQ_IDX_SHIFT                          (0)
#define CMU_GDMA_CH6_REQ_IDX(n)                             (((n) & 0x3F) << 6)
#define CMU_GDMA_CH6_REQ_IDX_MASK                           (0x3F << 6)
#define CMU_GDMA_CH6_REQ_IDX_SHIFT                          (6)
#define CMU_GDMA_CH7_REQ_IDX(n)                             (((n) & 0x3F) << 12)
#define CMU_GDMA_CH7_REQ_IDX_MASK                           (0x3F << 12)
#define CMU_GDMA_CH7_REQ_IDX_SHIFT                          (12)
#define CMU_GDMA_CH8_REQ_IDX(n)                             (((n) & 0x3F) << 18)
#define CMU_GDMA_CH8_REQ_IDX_MASK                           (0x3F << 18)
#define CMU_GDMA_CH8_REQ_IDX_SHIFT                          (18)
#define CMU_GDMA_CH9_REQ_IDX(n)                             (((n) & 0x3F) << 24)
#define CMU_GDMA_CH9_REQ_IDX_MASK                           (0x3F << 24)
#define CMU_GDMA_CH9_REQ_IDX_SHIFT                          (24)

// reg_0e8
#define CMU_GDMA_CH10_REQ_IDX(n)                            (((n) & 0x3F) << 0)
#define CMU_GDMA_CH10_REQ_IDX_MASK                          (0x3F << 0)
#define CMU_GDMA_CH10_REQ_IDX_SHIFT                         (0)
#define CMU_GDMA_CH11_REQ_IDX(n)                            (((n) & 0x3F) << 6)
#define CMU_GDMA_CH11_REQ_IDX_MASK                          (0x3F << 6)
#define CMU_GDMA_CH11_REQ_IDX_SHIFT                         (6)
#define CMU_GDMA_CH12_REQ_IDX(n)                            (((n) & 0x3F) << 12)
#define CMU_GDMA_CH12_REQ_IDX_MASK                          (0x3F << 12)
#define CMU_GDMA_CH12_REQ_IDX_SHIFT                         (12)
#define CMU_GDMA_CH13_REQ_IDX(n)                            (((n) & 0x3F) << 18)
#define CMU_GDMA_CH13_REQ_IDX_MASK                          (0x3F << 18)
#define CMU_GDMA_CH13_REQ_IDX_SHIFT                         (18)
#define CMU_GDMA_CH14_REQ_IDX(n)                            (((n) & 0x3F) << 24)
#define CMU_GDMA_CH14_REQ_IDX_MASK                          (0x3F << 24)
#define CMU_GDMA_CH14_REQ_IDX_SHIFT                         (24)

// reg_0ec
#define CMU_GDMA_CH15_REQ_IDX(n)                            (((n) & 0x3F) << 0)
#define CMU_GDMA_CH15_REQ_IDX_MASK                          (0x3F << 0)
#define CMU_GDMA_CH15_REQ_IDX_SHIFT                         (0)

// reg_0f0
#define CMU_RESERVED(n)                                     (((n) & 0xFFFFFFFF) << 0)
#define CMU_RESERVED_MASK                                   (0xFFFFFFFF << 0)
#define CMU_RESERVED_SHIFT                                  (0)

// reg_0f4
#define CMU_DEBUG(n)                                        (((n) & 0xFFFFFFFF) << 0)
#define CMU_DEBUG_MASK                                      (0xFFFFFFFF << 0)
#define CMU_DEBUG_SHIFT                                     (0)

// reg_0f8
#define CMU_RESERVED_3(n)                                   (((n) & 0xFFFFFFFF) << 0)
#define CMU_RESERVED_3_MASK                                 (0xFFFFFFFF << 0)
#define CMU_RESERVED_3_SHIFT                                (0)

// reg_100
#define CMU_RSTN_DIV_DSI_ENABLE                             (1 << 0)
#define CMU_SEL_OSCX4_DSI_ENABLE                            (1 << 1)
#define CMU_SEL_OSCX2_DSI_ENABLE                            (1 << 2)
#define CMU_SEL_PLL_DSI_ENABLE                              (1 << 3)
#define CMU_BYPASS_DIV_DSI_ENABLE                           (1 << 4)
#define CMU_RSTN_DIV_PER_ENABLE                             (1 << 5)
#define CMU_BYPASS_DIV_PER_ENABLE                           (1 << 6)
#define CMU_SEL_CLKIN_DSI_ENABLE                            (1 << 7)
#define CMU_RSTN_DIV_PIX_ENABLE                             (1 << 8)
#define CMU_SEL_OSCX4_PIX_ENABLE                            (1 << 9)
#define CMU_SEL_OSCX2_PIX_ENABLE                            (1 << 10)
#define CMU_SEL_PLL_PIX_ENABLE                              (1 << 11)
#define CMU_BYPASS_DIV_PIX_ENABLE                           (1 << 12)

// reg_104
#define CMU_RSTN_DIV_DSI_DISABLE                            (1 << 0)
#define CMU_SEL_OSCX4_DSI_DISABLE                           (1 << 1)
#define CMU_SEL_OSCX2_DSI_DISABLE                           (1 << 2)
#define CMU_SEL_PLL_DSI_DISABLE                             (1 << 3)
#define CMU_BYPASS_DIV_DSI_DISABLE                          (1 << 4)
#define CMU_RSTN_DIV_PER_DISABLE                            (1 << 5)
#define CMU_BYPASS_DIV_PER_DISABLE                          (1 << 6)
#define CMU_SEL_CLKIN_DSI_DISABLE                           (1 << 7)
#define CMU_RSTN_DIV_PIX_DISABLE                            (1 << 8)
#define CMU_SEL_OSCX4_PIX_DISABLE                           (1 << 9)
#define CMU_SEL_OSCX2_PIX_DISABLE                           (1 << 10)
#define CMU_SEL_PLL_PIX_DISABLE                             (1 << 11)
#define CMU_BYPASS_DIV_PIX_DISABLE                          (1 << 12)

// reg_108
#define CMU_VTOR_CORE1(n)                                   (((n) & 0x1FFFFFF) << 7)
#define CMU_VTOR_CORE1_MASK                                 (0x1FFFFFF << 7)
#define CMU_VTOR_CORE1_SHIFT                                (7)

// reg_10c
#define CMU_CFG_DIV_PCM(n)                                  (((n) & 0x1FFF) << 0)
#define CMU_CFG_DIV_PCM_MASK                                (0x1FFF << 0)
#define CMU_CFG_DIV_PCM_SHIFT                               (0)
#define CMU_CFG_DIV_I2S0(n)                                 (((n) & 0x1FFF) << 13)
#define CMU_CFG_DIV_I2S0_MASK                               (0x1FFF << 13)
#define CMU_CFG_DIV_I2S0_SHIFT                              (13)
#define CMU_EN_CLK_PLL_I2S0                                 (1 << 26)
#define CMU_EN_CLK_PLL_PCM                                  (1 << 27)
#define CMU_CFG_DIV_PER(n)                                  (((n) & 0x3) << 28)
#define CMU_CFG_DIV_PER_MASK                                (0x3 << 28)
#define CMU_CFG_DIV_PER_SHIFT                               (28)
#define CMU_SEL_PLL_AUD(n)                                  (((n) & 0x3) << 30)
#define CMU_SEL_PLL_AUD_MASK                                (0x3 << 30)
#define CMU_SEL_PLL_AUD_SHIFT                               (30)

// reg_110
#define CMU_CFG_DIV_SPDIF0(n)                               (((n) & 0x1FFF) << 0)
#define CMU_CFG_DIV_SPDIF0_MASK                             (0x1FFF << 0)
#define CMU_CFG_DIV_SPDIF0_SHIFT                            (0)
#define CMU_CFG_DIV_I2S1(n)                                 (((n) & 0x1FFF) << 13)
#define CMU_CFG_DIV_I2S1_MASK                               (0x1FFF << 13)
#define CMU_CFG_DIV_I2S1_SHIFT                              (13)
#define CMU_EN_CLK_PLL_SPDIF0                               (1 << 26)
#define CMU_EN_CLK_PLL_I2S1                                 (1 << 27)
#define CMU_CFG_DIV_PIX(n)                                  (((n) & 0xF) << 28)
#define CMU_CFG_DIV_PIX_MASK                                (0xF << 28)
#define CMU_CFG_DIV_PIX_SHIFT                               (28)

// reg_114
#define CMU_RF3_RET1N0                                      (1 << 0)
#define CMU_RF3_RET2N0                                      (1 << 1)
#define CMU_RF3_PGEN0                                       (1 << 2)
#define CMU_RF3_RET1N1                                      (1 << 3)
#define CMU_RF3_RET2N1                                      (1 << 4)
#define CMU_RF3_PGEN1                                       (1 << 5)

// reg_118
#define CMU_TPORT_INTR_LEN(n)                               (((n) & 0xFFFFFFFF) << 0)
#define CMU_TPORT_INTR_LEN_MASK                             (0xFFFFFFFF << 0)
#define CMU_TPORT_INTR_LEN_SHIFT                            (0)

// reg_11c
#define CMU_TPORT_CURR_ADDR(n)                              (((n) & 0xFFFFFFFF) << 0)
#define CMU_TPORT_CURR_ADDR_MASK                            (0xFFFFFFFF << 0)
#define CMU_TPORT_CURR_ADDR_SHIFT                           (0)

// reg_120
#define CMU_TPORT_START_ADDR(n)                             (((n) & 0xFFFFFFFF) << 0)
#define CMU_TPORT_START_ADDR_MASK                           (0xFFFFFFFF << 0)
#define CMU_TPORT_START_ADDR_SHIFT                          (0)

// reg_124
#define CMU_TPORT_END_ADDR(n)                               (((n) & 0xFFFFFFFF) << 0)
#define CMU_TPORT_END_ADDR_MASK                             (0xFFFFFFFF << 0)
#define CMU_TPORT_END_ADDR_SHIFT                            (0)

// reg_128
#define CMU_SENS2MCU_DATA_DONE_SET                          (1 << 0)
#define CMU_SENS2MCU_DATA1_DONE_SET                         (1 << 1)
#define CMU_SENS2MCU_DATA2_DONE_SET                         (1 << 2)
#define CMU_SENS2MCU_DATA3_DONE_SET                         (1 << 3)
#define CMU_MCU2SENS_DATA_IND_SET                           (1 << 4)
#define CMU_MCU2SENS_DATA1_IND_SET                          (1 << 5)
#define CMU_MCU2SENS_DATA2_IND_SET                          (1 << 6)
#define CMU_MCU2SENS_DATA3_IND_SET                          (1 << 7)
#define CMU_SENS2MCU_DATA_MSK_SET                           (1 << 8)
#define CMU_SENS2MCU_DATA1_MSK_SET                          (1 << 9)
#define CMU_SENS2MCU_DATA2_MSK_SET                          (1 << 10)
#define CMU_SENS2MCU_DATA3_MSK_SET                          (1 << 11)
#define CMU_MCU2SENS_DATA_MSK_SET                           (1 << 12)
#define CMU_MCU2SENS_DATA1_MSK_SET                          (1 << 13)
#define CMU_MCU2SENS_DATA2_MSK_SET                          (1 << 14)
#define CMU_MCU2SENS_DATA3_MSK_SET                          (1 << 15)
#define CMU_SENS2MCU_DATA_INTR                              (1 << 16)
#define CMU_SENS2MCU_DATA1_INTR                             (1 << 17)
#define CMU_SENS2MCU_DATA2_INTR                             (1 << 18)
#define CMU_SENS2MCU_DATA3_INTR                             (1 << 19)
#define CMU_MCU2SENS_DATA_INTR                              (1 << 20)
#define CMU_MCU2SENS_DATA1_INTR                             (1 << 21)
#define CMU_MCU2SENS_DATA2_INTR                             (1 << 22)
#define CMU_MCU2SENS_DATA3_INTR                             (1 << 23)
#define CMU_SENS2MCU_DATA_INTR_MSK                          (1 << 24)
#define CMU_SENS2MCU_DATA1_INTR_MSK                         (1 << 25)
#define CMU_SENS2MCU_DATA2_INTR_MSK                         (1 << 26)
#define CMU_SENS2MCU_DATA3_INTR_MSK                         (1 << 27)
#define CMU_MCU2SENS_DATA_INTR_MSK                          (1 << 28)
#define CMU_MCU2SENS_DATA1_INTR_MSK                         (1 << 29)
#define CMU_MCU2SENS_DATA2_INTR_MSK                         (1 << 30)
#define CMU_MCU2SENS_DATA3_INTR_MSK                         (1 << 31)

// reg_12c
#define CMU_SENS2MCU_DATA_DONE_CLR                          (1 << 0)
#define CMU_SENS2MCU_DATA1_DONE_CLR                         (1 << 1)
#define CMU_SENS2MCU_DATA2_DONE_CLR                         (1 << 2)
#define CMU_SENS2MCU_DATA3_DONE_CLR                         (1 << 3)
#define CMU_MCU2SENS_DATA_IND_CLR                           (1 << 4)
#define CMU_MCU2SENS_DATA1_IND_CLR                          (1 << 5)
#define CMU_MCU2SENS_DATA2_IND_CLR                          (1 << 6)
#define CMU_MCU2SENS_DATA3_IND_CLR                          (1 << 7)
#define CMU_SENS2MCU_DATA_MSK_CLR                           (1 << 8)
#define CMU_SENS2MCU_DATA1_MSK_CLR                          (1 << 9)
#define CMU_SENS2MCU_DATA2_MSK_CLR                          (1 << 10)
#define CMU_SENS2MCU_DATA3_MSK_CLR                          (1 << 11)
#define CMU_MCU2SENS_DATA_MSK_CLR                           (1 << 12)
#define CMU_MCU2SENS_DATA1_MSK_CLR                          (1 << 13)
#define CMU_MCU2SENS_DATA2_MSK_CLR                          (1 << 14)
#define CMU_MCU2SENS_DATA3_MSK_CLR                          (1 << 15)

// reg_130
#define CMU_TPORT_INTR_MSK                                  (1 << 0)
#define CMU_TPORT_INTR_EN                                   (1 << 1)
#define CMU_TPORT_INTR_CLR                                  (1 << 2)
#define CMU_TPORT_ADDR_BYPASS                               (1 << 3)
#define CMU_TPORT_FILTER_SEL                                (1 << 4)
#define CMU_TPORT_EN                                        (1 << 5)
#define CMU_TPORT_INTR_STATUS                               (1 << 6)

// reg_138
#define CMU_I_CHARGER_INTR_WAKEUP_MASK                      (1 << 8)

// MCU System AHB Clocks:
#define SYS_HCLK_CORE0                                      (1 << 0)
#define SYS_HRST_CORE0                                      (1 << 0)
#define SYS_HCLK_ROM0                                       (1 << 1)
#define SYS_HRST_ROM0                                       (1 << 1)
#define SYS_HCLK_RAM8                                       (1 << 2)
#define SYS_HRST_RAM8                                       (1 << 2)
#define SYS_HCLK_FLASH1                                     (1 << 3)
#define SYS_HRST_FLASH1                                     (1 << 3)
#define SYS_HCLK_RAM0                                       (1 << 4)
#define SYS_HRST_RAM0                                       (1 << 4)
#define SYS_HCLK_RAM1                                       (1 << 5)
#define SYS_HRST_RAM1                                       (1 << 5)
#define SYS_HCLK_RAM2                                       (1 << 6)
#define SYS_HRST_RAM2                                       (1 << 6)
#define SYS_HCLK_RAM7                                       (1 << 7)
#define SYS_HRST_RAM7                                       (1 << 7)
#define SYS_HCLK_AHB0                                       (1 << 8)
#define SYS_HRST_AHB0                                       (1 << 8)
#define SYS_HCLK_AHB1                                       (1 << 9)
#define SYS_HRST_AHB1                                       (1 << 9)
#define SYS_HCLK_AH2H_BT                                    (1 << 10)
#define SYS_HRST_AH2H_BT                                    (1 << 10)
#define SYS_HCLK_ADMA                                       (1 << 11)
#define SYS_HRST_ADMA                                       (1 << 11)
#define SYS_HCLK_GDMA                                       (1 << 12)
#define SYS_HRST_GDMA                                       (1 << 12)
#define SYS_HCLK_PSPY                                       (1 << 13)
#define SYS_HRST_PSPY                                       (1 << 13)
#define SYS_HCLK_FLASH                                      (1 << 14)
#define SYS_HRST_FLASH                                      (1 << 14)
#define SYS_HCLK_SDMMC                                      (1 << 15)
#define SYS_HRST_SDMMC                                      (1 << 15)
#define SYS_HCLK_USBC                                       (1 << 16)
#define SYS_HRST_USBC                                       (1 << 16)
#define SYS_HCLK_CODEC                                      (1 << 17)
#define SYS_HRST_CODEC                                      (1 << 17)
#define SYS_HCLK_CP0                                        (1 << 18)
#define SYS_HRST_CP0                                        (1 << 18)
#define SYS_HCLK_BT_TPORT                                   (1 << 19)
#define SYS_HRST_BT_TPORT                                   (1 << 19)
#define SYS_HCLK_USBH                                       (1 << 20)
#define SYS_HRST_USBH                                       (1 << 20)
#define SYS_HCLK_I2C_SLAVE                                  (1 << 21)
#define SYS_HRST_I2C_SLAVE                                  (1 << 21)
#define SYS_HCLK_BT_DUMP                                    (1 << 22)
#define SYS_HRST_BT_DUMP                                    (1 << 22)
#define SYS_HCLK_CORE1                                      (1 << 23)
#define SYS_HRST_CORE1                                      (1 << 23)
#define SYS_HCLK_RAM3                                       (1 << 24)
#define SYS_HRST_RAM3                                       (1 << 24)
#define SYS_HCLK_RAM4                                       (1 << 25)
#define SYS_HRST_RAM4                                       (1 << 25)
#define SYS_HCLK_RAM5                                       (1 << 26)
#define SYS_HRST_RAM5                                       (1 << 26)
#define SYS_HCLK_RAM6                                       (1 << 27)
#define SYS_HRST_RAM6                                       (1 << 27)
#define SYS_HCLK_BCM                                        (1 << 28)
#define SYS_HRST_BCM                                        (1 << 28)
#define SYS_HCLK_CACHE0                                     (1 << 29)
#define SYS_HRST_CACHE0                                     (1 << 29)
#define SYS_HCLK_AH2H_SENS                                  (1 << 30)
#define SYS_HRST_AH2H_SENS                                  (1 << 30)
#define SYS_HCLK_SPI_AHB                                    (1 << 31)
#define SYS_HRST_SPI_AHB                                    (1 << 31)

// MCU System APB Clocks:
#define SYS_PCLK_CMU                                        (1 << 0)
#define SYS_PRST_CMU                                        (1 << 0)
#define SYS_PCLK_WDT                                        (1 << 1)
#define SYS_PRST_WDT                                        (1 << 1)
#define SYS_PCLK_TIMER0                                     (1 << 2)
#define SYS_PRST_TIMER0                                     (1 << 2)
#define SYS_PCLK_TIMER1                                     (1 << 3)
#define SYS_PRST_TIMER1                                     (1 << 3)
#define SYS_PCLK_TIMER2                                     (1 << 4)
#define SYS_PRST_TIMER2                                     (1 << 4)
#define SYS_PCLK_I2C0                                       (1 << 5)
#define SYS_PRST_I2C0                                       (1 << 5)
#define SYS_PCLK_I2C1                                       (1 << 6)
#define SYS_PRST_I2C1                                       (1 << 6)
#define SYS_PCLK_SPI                                        (1 << 7)
#define SYS_PRST_SPI                                        (1 << 7)
#define SYS_PCLK_TRNG                                       (1 << 8)
#define SYS_PRST_TRNG                                       (1 << 8)
#define SYS_PCLK_SPI_ITN                                    (1 << 9)
#define SYS_PRST_SPI_ITN                                    (1 << 9)
#define SYS_PCLK_UART0                                      (1 << 10)
#define SYS_PRST_UART0                                      (1 << 10)
#define SYS_PCLK_UART1                                      (1 << 11)
#define SYS_PRST_UART1                                      (1 << 11)
#define SYS_PCLK_UART2                                      (1 << 12)
#define SYS_PRST_UART2                                      (1 << 12)
#define SYS_PCLK_PCM                                        (1 << 13)
#define SYS_PRST_PCM                                        (1 << 13)
#define SYS_PCLK_I2S0                                       (1 << 14)
#define SYS_PRST_I2S0                                       (1 << 14)
#define SYS_PCLK_SPDIF0                                     (1 << 15)
#define SYS_PRST_SPDIF0                                     (1 << 15)
#define SYS_PCLK_I2S1                                       (1 << 16)
#define SYS_PRST_I2S1                                       (1 << 16)
#define SYS_PCLK_BCM                                        (1 << 17)
#define SYS_PRST_BCM                                        (1 << 17)
#define SYS_PCLK_TZC                                        (1 << 18)
#define SYS_PRST_TZC                                        (1 << 18)
#define SYS_PCLK_DIS                                        (1 << 19)
#define SYS_PRST_DIS                                        (1 << 19)
#define SYS_PCLK_PSPY                                       (1 << 20)
#define SYS_PRST_PSPY                                       (1 << 20)
#define SYS_PCLK_I2C2                                       (1 << 21)
#define SYS_PRST_I2C2                                       (1 << 21)
#define SYS_PCLK_I2C3                                       (1 << 22)
#define SYS_PRST_I2C3                                       (1 << 22)
#define SYS_PCLK_I2C4                                       (1 << 23)
#define SYS_PRST_I2C4                                       (1 << 23)
#define SYS_PCLK_I2C5                                       (1 << 24)
#define SYS_PRST_I2C5                                       (1 << 24)
#define SYS_PCLK_I2S2                                       (1 << 25)
#define SYS_PRST_I2S2                                       (1 << 25)
#define SYS_PCLK_SPI3                                       (1 << 26)
#define SYS_PRST_SPI3                                       (1 << 26)

// MCU System Other Clocks:
#define SYS_OCLK_SLEEP                                      (1 << 0)
#define SYS_ORST_SLEEP                                      (1 << 0)
#define SYS_OCLK_FLASH                                      (1 << 1)
#define SYS_ORST_FLASH                                      (1 << 1)
#define SYS_OCLK_USB                                        (1 << 2)
#define SYS_ORST_USB                                        (1 << 2)
#define SYS_OCLK_RESERVED_03                                (1 << 3)
#define SYS_ORST_RESERVED_03                                (1 << 3)
#define SYS_OCLK_WDT                                        (1 << 4)
#define SYS_ORST_WDT                                        (1 << 4)
#define SYS_OCLK_TIMER0                                     (1 << 5)
#define SYS_ORST_TIMER0                                     (1 << 5)
#define SYS_OCLK_TIMER1                                     (1 << 6)
#define SYS_ORST_TIMER1                                     (1 << 6)
#define SYS_OCLK_TIMER2                                     (1 << 7)
#define SYS_ORST_TIMER2                                     (1 << 7)
#define SYS_OCLK_I2C0                                       (1 << 8)
#define SYS_ORST_I2C0                                       (1 << 8)
#define SYS_OCLK_I2C1                                       (1 << 9)
#define SYS_ORST_I2C1                                       (1 << 9)
#define SYS_OCLK_SPI                                        (1 << 10)
#define SYS_ORST_SPI                                        (1 << 10)
#define SYS_OCLK_SPI_ITN                                    (1 << 11)
#define SYS_ORST_SPI_ITN                                    (1 << 11)
#define SYS_OCLK_UART0                                      (1 << 12)
#define SYS_ORST_UART0                                      (1 << 12)
#define SYS_OCLK_UART1                                      (1 << 13)
#define SYS_ORST_UART1                                      (1 << 13)
#define SYS_OCLK_UART2                                      (1 << 14)
#define SYS_ORST_UART2                                      (1 << 14)
#define SYS_OCLK_I2S0                                       (1 << 15)
#define SYS_ORST_I2S0                                       (1 << 15)
#define SYS_OCLK_SPDIF0                                     (1 << 16)
#define SYS_ORST_SPDIF0                                     (1 << 16)
#define SYS_OCLK_PCM                                        (1 << 17)
#define SYS_ORST_PCM                                        (1 << 17)
#define SYS_OCLK_USB32K                                     (1 << 18)
#define SYS_ORST_USB32K                                     (1 << 18)
#define SYS_OCLK_I2S1                                       (1 << 19)
#define SYS_ORST_I2S1                                       (1 << 19)
#define SYS_OCLK_FLASH1                                     (1 << 20)
#define SYS_ORST_FLASH1                                     (1 << 20)
#define SYS_OCLK_I2C4                                       (1 << 21)
#define SYS_ORST_I2C4                                       (1 << 21)
#define SYS_OCLK_I2C5                                       (1 << 22)
#define SYS_ORST_I2C5                                       (1 << 22)
#define SYS_OCLK_I2C2                                       (1 << 23)
#define SYS_ORST_I2C2                                       (1 << 23)
#define SYS_OCLK_I2C3                                       (1 << 24)
#define SYS_ORST_I2C3                                       (1 << 24)
#define SYS_OCLK_SDMMC0                                     (1 << 25)
#define SYS_ORST_SDMMC0                                     (1 << 25)
#define SYS_OCLK_RESERVED_26                                (1 << 26)
#define SYS_ORST_RESERVED_26                                (1 << 26)
#define SYS_OCLK_RESERVED_27                                (1 << 27)
#define SYS_ORST_RESERVED_27                                (1 << 27)
#define SYS_OCLK_RESERVED_28                                (1 << 28)
#define SYS_ORST_RESERVED_28                                (1 << 28)
#define SYS_OCLK_RESERVED_29                                (1 << 29)
#define SYS_ORST_RESERVED_29                                (1 << 29)
#define SYS_OCLK_I2S2                                       (1 << 30)
#define SYS_ORST_I2S2                                       (1 << 30)
#define SYS_OCLK_SPI3                                       (1 << 31)
#define SYS_ORST_SPI3                                       (1 << 31)

// MCU System AXI Clocks:
#define SYS_XCLK_PSRAM                                      (1 << 0)
#define SYS_XRST_PSRAM                                      (1 << 0)
#define SYS_XCLK_SDMMC0                                     (1 << 1)
#define SYS_XRST_SDMMC0                                     (1 << 1)
#define SYS_XCLK_GPU                                        (1 << 2)
#define SYS_XRST_GPU                                        (1 << 2)
#define SYS_XCLK_L2CC                                       (1 << 3)
#define SYS_XRST_L2CC                                       (1 << 3)
#define SYS_XCLK_VMMU                                       (1 << 4)
#define SYS_XRST_VMMU                                       (1 << 4)
#define SYS_XCLK_GA2D                                       (1 << 5)
#define SYS_XRST_GA2D                                       (1 << 5)
#define SYS_XCLK_MISC                                       (1 << 6)
#define SYS_XRST_MISC                                       (1 << 6)
#define SYS_XCLK_LCDC                                       (1 << 7)
#define SYS_XRST_LCDC                                       (1 << 7)
#define SYS_XCLK_AHB2                                       (1 << 8)
#define SYS_XRST_AHB2                                       (1 << 8)
#define SYS_XCLK_CACHE1                                     (1 << 9)
#define SYS_XRST_CACHE1                                     (1 << 9)
#define SYS_XCLK_CP1                                        (1 << 10)
#define SYS_XRST_CP1                                        (1 << 10)
#define SYS_XCLK_CPUDUMP                                    (1 << 11)
#define SYS_XRST_CPUCUMP                                    (1 << 11)

#endif

