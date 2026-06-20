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
#ifndef __PLAT_ADDR_MAP_BEST1306_H__
#define __PLAT_ADDR_MAP_BEST1306_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(ROM_BUILD) && !defined(SIMU) && !defined(FPGA) && !defined(NO_MCU_RAM_ONLY)
#define MCU_RAM_ONLY
#define RAM_BASE                                RAM0_BASE
#define RAM_SIZE                                (RAM3_BASE + RAM3_SIZE - RAM_BASE)
#ifdef CORE_SLEEP_POWER_DOWN
#error "MCU RAM will be power down when CORE_SLEEP_POWER_DOWN defined"
#endif
#endif

#define ROM_BASE                                0x24000000
#define ROMX_BASE                               0x00020000

#ifndef ROM_SIZE
#define ROM_SIZE                                0x00010000
#endif
#define ROM_EXT_SIZE                            0x00008000

#define PATCH_ENTRY_NUM                         8
#define PATCH_CTRL_BASE                         0x0004F000
#define PATCH_DATA_BASE                         0x0004F100

#define RAM0_BASE                               0x20000000
#define RAMX0_BASE                              0x00200000
#define RAM1_BASE                               0x20020000
#define RAMX1_BASE                              0x00220000
#define RAM2_BASE                               0x20040000
#define RAMX2_BASE                              0x00240000
#define RAM3_BASE                               0x20060000
#define RAMX3_BASE                              0x00260000

/* best1503/MiBand9 has 1.4MB of contiguous on-chip SRAM (RAM mirror at
 * RAMX0_BASE 0x00200000 is 0x160000 long, confirmed by the maskrom dump size of
 * 1441792 = 0x160000 bytes). The stock best1306 SDK only declared RAM0..RAM3 as
 * 4x128K = 0x80000 (512K), which artificially capped the linker. RAM0..RAM2 stay
 * 128K each; RAM3 is extended to absorb the remaining 0x100000 so the top RAM
 * region (.bss/heap/stack) can use the full physical SRAM:
 *   RAM_TOTAL_SIZE = RAM3_BASE + RAM3_SIZE - RAM0_BASE
 *                  = 0x20060000 + 0x100000 - 0x20000000 = 0x160000 (1.4MB). */
#define RAM3_SIZE                               0x00100000

#define RAM_TOTAL_SIZE                          (RAM3_BASE + RAM3_SIZE - RAM0_BASE)

#if defined(CP_IN_SAME_EE) && !defined(MCU_RAM_ONLY)
#ifndef RAMCPX_SIZE
#define RAMCPX_SIZE                             (RAM2_BASE - RAM1_BASE)
#endif
#define RAMCPX_BASE                             (RAM_BASE + RAM_SIZE - RAM0_BASE + RAMX0_BASE)

#ifndef RAMCP_SIZE
#define RAMCP_SIZE                              (RAM3_BASE - RAM2_BASE)
#endif
#define RAMCP_BASE                              (RAMCPX_BASE + RAMCPX_SIZE - RAMX0_BASE + RAM0_BASE)

#define RAMCP_TOP                               (RAMCP_BASE + RAMCP_SIZE)
#else
#define RAMCP_TOP                               RAM0_BASE
#endif

#ifdef CORE_SLEEP_POWER_DOWN

#ifdef __BT_RAMRUN__
/* RAM0-RAM3: 768K volatile memory
 * RAM4-RAM5: 512K BT controller memory
 * RAM6-RAM8: 384K retention memory
 */
#error "New LDS configuration is needed"
#endif

#ifdef MEM_POOL_BASE
#if (MEM_POOL_BASE < RAMCP_TOP)
#error "Bad MEM_POOL_BASE with CORE_SLEEP_POWER_DOWN"
#endif
#else
#define MEM_POOL_BASE                           RAMCP_TOP
#endif

#ifdef MEM_POOL_SIZE
#if (MEM_POOL_BASE + MEM_POOL_SIZE > RAM_BASE)
#error "Bad MEM_POOL_SIZE with CORE_SLEEP_POWER_DOWN"
#endif
#else
#define MEM_POOL_SIZE                           (RAM3_BASE - MEM_POOL_BASE)
#endif

#ifdef RAM_BASE
#if (RAM_BASE < MEM_POOL_BASE)
#error "Bad RAM_BASE with CORE_SLEEP_POWER_DOWN"
#endif
#else
#define RAM_BASE                                (MEM_POOL_BASE + MEM_POOL_SIZE)
#endif
#define RAMX_BASE                               (RAM_BASE - RAM0_BASE + RAMX0_BASE)

#else /* !CORE_SLEEP_POWER_DOWN */

#ifdef RAM_BASE
#if (RAM_BASE < RAMCP_TOP)
#error "Bad RAM_BASE"
#endif
#else
#define RAM_BASE                                RAM0_BASE
#endif
#define RAMX_BASE                               (RAM_BASE - RAM0_BASE + RAMX0_BASE)
#endif /* !CORE_SLEEP_POWER_DOWN */

#ifdef RAM_SIZE
#if (RAM_BASE + RAM_SIZE) > (RAM3_BASE + RAM3_SIZE)
#error "Bad RAM_SIZE"
#endif
#else
#ifdef __BT_RAMRUN__
#if (RAM_BASE >= RAM3_BASE)
#error "Bad RAM_BASE with BT ramrun"
#endif
#define RAM_SIZE                                (RAM3_BASE - RAM_BASE)
#else
#if defined(CP_IN_SAME_EE)
#define RAM_SIZE                                (RAM_TOTAL_SIZE - RAMCP_SIZE - RAMCPX_SIZE)
#else
#define RAM_SIZE                                (RAM_TOTAL_SIZE)
#endif
#endif
#endif

#if defined(ARM_CMSE) || defined(ARM_CMNS)
/*MPC: SRAM block size: 0x8000, FLASH block size 0x40000*/
#define RAM_S_SIZE                              0x0001E000
#define RAM_NSC_SIZE                            0x00002000
#ifndef FLASH_S_SIZE
#define FLASH_S_SIZE                            0x00040000
#endif

#undef RAM_BASE
#undef RAMX_BASE
#undef RAM_SIZE
#undef RAMCP_BASE
#undef RAMCPX_BASE
#undef RAMCP_SIZE
#undef RAMCPX_SIZE

#ifdef CP_IN_SAME_EE
#define RAMCPX_BASE                             (RAMX0_BASE + RAM_S_SIZE + RAM_NSC_SIZE)
#define RAMCPX_SIZE                             0x20000
#define RAMCP_BASE                              (RAM0_BASE + RAM_S_SIZE + RAM_NSC_SIZE + RAMCPX_SIZE)
#define RAMCP_SIZE                              0x20000
#define RAM_NS_BASE                             (RAMCP_BASE + RAMCP_SIZE)
#define RAMX_NS_BASE                            (RAMCPX_BASE + RAMCPX_SIZE + RAMCP_SIZE)
#define RAM_NS_SIZE                             (RAM3_BASE + RAM3_SIZE - RAM_BASE)
#else
#define RAMCPX_BASE (RAMX0_BASE)
#define RAMCPX_SIZE 0
#define RAMCP_BASE (RAM0_BASE)
#define RAMCP_SIZE  0

#define RAM_NS_BASE                             (RAM1_BASE)
#define RAMX_NS_BASE                            (RAMX1_BASE)
#define RAM_NS_SIZE                             (RAM3_BASE - RAM1_BASE + RAM3_SIZE)
#endif

#if defined(ARM_CMNS)
#define RAM_BASE                                RAM_NS_BASE
#define RAMX_BASE                               RAMX_NS_BASE
#define RAM_SIZE                                RAM_NS_SIZE
#else
#if ((RAM_S_SIZE + RAM_NSC_SIZE) & (0x8000-1))
#error "RAM_S_SIZE should be 0x8000 aligned"
#endif
#if (FLASH_S_SIZE & (0x40000-1))
#error "FLASH_S_SIZE should be 0x40000 aligned"
#endif
#define RAM_BASE                                RAM0_BASE
#define RAMX_BASE                               RAMX0_BASE
#define RAM_SIZE                                RAM_S_SIZE
#ifndef NS_APP_START_OFFSET
#define NS_APP_START_OFFSET                     (FLASH_S_SIZE)
#endif
#ifndef FLASH_REGION_SIZE
#define FLASH_REGION_SIZE                       FLASH_S_SIZE
#endif
#endif
#endif /* defined(ARM_CMSE) || defined(ARM_CMNS) */

#define REAL_FLASH_BASE                         0x2C000000
#define REAL_FLASH_NC_BASE                      0x28000000
#define REAL_FLASHX_BASE                        0x0C000000
#define REAL_FLASHX_NC_BASE                     0x08000000

#define REAL_FLASH1_BASE                        0x34000000
#define REAL_FLASH1_NC_BASE                     0x30000000
#define REAL_FLASH1X_BASE                       0x14000000
#define REAL_FLASH1X_NC_BASE                    0x10000000

#define ICACHE_CTRL_BASE                        0x07FFA000
#define ICACHE_SIZE                             0x00004000

#define CMU_BASE                                0x40000000
#define MCU_WDT_BASE                            0x40001000
#define MCU_TIMER0_BASE                         0x40002000
#define MCU_TIMER1_BASE                         0x40003000
#define MCU_TIMER2_BASE                         0x40004000
#define I2C0_BASE                               0x40005000
#define I2C1_BASE                               0x40006000
#define SPI_BASE                                0x40007000
#define TRNG_BASE                               0x40008000
#define ISPI_BASE                               0x40009000
#define SPI3_BASE                               0x4000A000
#define UART0_BASE                              0x4000B000
#define UART1_BASE                              0x4000C000
#define UART2_BASE                              0x4000D000
#define BTPCM_BASE                              0x4000E000
#define I2S0_BASE                               0x4000F000
#define SPDIF0_BASE                             0x40010000
#define I2S1_BASE                               0x40011000
#define SEC_CTRL_BASE                           0x40030000
#define MPC_FLASH0_BASE                         0x40032000
#define PAGE_SPY_BASE                           0x40050000
#define I2C2_BASE                               0x40060000
#define I2C3_BASE                               0x40061000
#define I2C4_BASE                               0x40062000
#define I2C5_BASE                               0x40063000
#define I2S2_BASE                               0x40064000
#define I2C_SLAVE_BASE                          0x40065000
#define CPUDUMP_BASE                            0x401E0000
#define BTC_CPUDUMP_BASE                        0xD0700000

#define AON_CMU_BASE                            0x40080000
#define AON_GPIO_BASE                           0x40081000
#define AON_WDT_BASE                            0x40082000
#define AON_PWM_BASE                            0x40083000
#define AON_TIMER0_BASE                         0x40084000
#define AON_PSC_BASE                            0x40085000
#define AON_IOMUX_BASE                          0x40086000
#define AON_SEC_CTRL_BASE                       0x40087000
#define AON_GPIO1_BASE                          0x40088000
#define AON_TIMER1_BASE                         0x40089000
#define AON_PWM1_BASE                           0x4008a000
#define AON_CAP_BASE                            0x4008B000

#define REAL_FLASH1_CTRL_BASE                   0x40500000
#define SDMMC0_BASE                             0x40110000
#define AUDMA_BASE                              0x40120000
#define GPDMA_BASE                              0x40130000
#define REAL_FLASH_CTRL_BASE                    0x40140000
#define BTDUMP_BASE                             0x40150000

/* ---- best1503 display controllers (MIPI-DSI + LCD controller) ----------------
 * Needed to wire platform/hal/hal_dsi.c + hal_lcdc.c for best1503 so the RM690B0
 * AMOLED can be driven. The best1306-based SDK ships hal_dsi.c/hal_lcdc.c but gates
 * them behind DSI_BASE/LCDC_BASE which were never defined for best1503.
 *
 * TODO(verify): exact bases must be confirmed by REing the MiBand9 Vela FW
 * (vela_ap.bin @0x2C150000) hal_dsi_init/hal_lcdc_* register writes once Ghidra's
 * full auto-analysis has resolved its PIC/GOT references. The values below are the
 * strongest candidates from the Vela peripheral-base-reference enumeration (0x40160000
 * 22 refs, 0x40168000 14 refs) and they fit this free peripheral slot right after
 * BTDUMP — but they are UNVERIFIED.
 * Gated behind CONFIG_DISPLAY_DSI_BEST1503 so the stock running firmware is
 * unaffected: defining DSI_BASE/LCDC_BASE auto-enables hal_dsi.c + hal_lcdc.c +
 * hal_display_best1503.c. Only switch this on once the values are verified. */
#ifdef CONFIG_DISPLAY_DSI_BEST1503
#define LCDC_BASE                               0x40200000u
#define DSI_BASE                                0x40168000  /* TODO verify via vela RE */

/* PSRAM (PSRAMUHS) for the display framebuffer. best1503/MiBand9 has PSRAM (CMU
 * has PSRAM clock controls) but this SDK header never mapped it. TODO(vela): confirm
 * the real PSRAM window + size from the MiBand9 Vela FW. Placeholders below pick a
 * free slot (flash uses 0x10/0x14/0x28/0x2C). */
#define PSRAM_BASE                              0x1C000000  /* TODO verify              */
#define PSRAMUHS_BASE                           0x1C000000  /* TODO verify: cacheable   */
#define PSRAMUHSX_BASE                          0x1C000000
#define PSRAM_NC_BASE                           0x18000000  /* TODO verify: non-cacheable */
#define SYS_PSRAM_BASE                          PSRAMUHS_BASE
#define SYS_PSRAM_NC_BASE                       PSRAM_NC_BASE
#define MCU_PSRAMUHS_BASE                       PSRAMUHS_BASE
#define PSRAMUHS_SIZE                           0x00800000  /* TODO verify: 8MB          */
#endif

#define CODEC_BASE                              0x40300000

#define BT_SUBSYS_BASE                          0xA0000000
#define BT_RAM_BASE                             0xC0000000
#define BT_RAM_SIZE                             0x00010000
#define BT_EXCH_MEM_BASE                        0xD0210000
#define BT_EXCH_MEM_SIZE                        0x00010000
#define BT_UART_BASE                            0xD0300000
#define BT_CMU_BASE                             0xD0330000

#ifdef CORE_SLEEP_POWER_DOWN
#define TIMER0_BASE                             AON_TIMER0_BASE
#define TIMER2_BASE                             MCU_TIMER0_BASE
#else
#define TIMER0_BASE                             MCU_TIMER0_BASE
#define TIMER2_BASE                             AON_TIMER0_BASE
#endif
#define TIMER1_BASE                             MCU_TIMER1_BASE

#define IOMUX_BASE                              AON_IOMUX_BASE
#define GPIO_BASE                               AON_GPIO_BASE
#define GPIO1_BASE                              AON_GPIO1_BASE
#define PWM_BASE                                AON_PWM_BASE
#define PWM1_BASE                               AON_PWM1_BASE
#define WDT_BASE                                AON_WDT_BASE

#ifdef ALT_BOOT_FLASH
#define FLASH_BASE                              REAL_FLASH1_BASE
#define FLASH_NC_BASE                           REAL_FLASH1_NC_BASE
#define FLASHX_BASE                             REAL_FLASH1X_BASE
#define FLASHX_NC_BASE                          REAL_FLASH1X_NC_BASE

#define FLASH1_BASE                             REAL_FLASH_BASE
#define FLASH1_NC_BASE                          REAL_FLASH_NC_BASE
#define FLASH1X_BASE                            REAL_FLASHX_BASE
#define FLASH1X_NC_BASE                         REAL_FLASHX_NC_BASE

#define FLASH_CTRL_BASE                         REAL_FLASH1_CTRL_BASE
#define FLASH1_CTRL_BASE                        REAL_FLASH_CTRL_BASE
#else
#define FLASH_BASE                              REAL_FLASH_BASE
#define FLASH_NC_BASE                           REAL_FLASH_NC_BASE
#define FLASHX_BASE                             REAL_FLASHX_BASE
#define FLASHX_NC_BASE                          REAL_FLASHX_NC_BASE

#define FLASH1_BASE                             REAL_FLASH1_BASE
#define FLASH1_NC_BASE                          REAL_FLASH1_NC_BASE
#define FLASH1X_BASE                            REAL_FLASH1X_BASE
#define FLASH1X_NC_BASE                         REAL_FLASH1X_NC_BASE

#define FLASH_CTRL_BASE                         REAL_FLASH_CTRL_BASE
#define FLASH1_CTRL_BASE                        REAL_FLASH1_CTRL_BASE
#endif

/* For linker scripts */
#define VECTOR_SECTION_SIZE                     380
#define REBOOT_PARAM_SECTION_SIZE               64
#define ROM_BUILD_INFO_SECTION_SIZE             40
#define ROM_EXPORT_FN_SECTION_SIZE              128
#define BT_INTESYS_MEM_OFFSET                   0x00004000

/* For module features */
#define CODEC_FREQ_CRYSTAL                      CODEC_FREQ_24M
#define CODEC_FREQ_EXTRA_DIV                    2
#define CODEC_PLL_DIV                           16
#define CODEC_CMU_DIV                           8
#define CODEC_PLAYBACK_BIT_DEPTH                24
#define CODEC_HAS_FIR
#define GPADC_CTRL_VER                          3
#define GPADC_VALUE_BITS                        16
#define GPADC_HAS_VSYS_DIV
#define GPADC_CUSTOM_CALIB_VAL
#define WORKAROUND_ATP_VAL                      HAL_GPADC_ATP_1MS
#define GPADC_CALIB_CH_DIV                      5
#define GPADC_VBAT_VOLT_DIV                     5
#define APP_BATTERY_GPADC_CH_NUM                HAL_GPADC_CHAN_6
#define DCDC_CLOCK_CONTROL
#define MORE_THAN_ONE_TYPE_OF_CHARGER
#define PWRKEY_IRQ_IN_PMU
#ifndef AUD_SECTION_STRUCT_VERSION
#ifdef FREEMAN_ENABLED_STERO
#define AUD_SECTION_STRUCT_VERSION              4
#else
#define AUD_SECTION_STRUCT_VERSION              3
#endif
#endif

#define DUMPPC_TOTAL_NUM                        (0x400/4)
#define DUMPPC_NUM                              100

#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
#define NO_FLASH_S_ACCESS
#endif

/* For boot struct version */
#ifndef SECURE_BOOT_VER
#define SECURE_BOOT_VER                         4
#endif

/* For ROM export functions */
#define NO_EXPORT_QSORT
#define NO_EXPORT_BSEARCH
#define NO_EXPORT_VSSCANF

#ifdef __cplusplus
}
#endif

#endif
