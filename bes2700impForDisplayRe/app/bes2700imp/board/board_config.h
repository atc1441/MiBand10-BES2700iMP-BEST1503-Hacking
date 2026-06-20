/* board_config.h — BES2700IMP / MiBand9 board definition (all verified findings).
 * Single source of truth for the memory map, chip-specific values, pin map and the
 * peripheral facts. Everything marked TODO(probe) still needs a logic-analyzer trace of
 * the stock FW or the schematic; everything else was measured on real silicon. */
#ifndef BES2700IMP_BOARD_CONFIG_H
#define BES2700IMP_BOARD_CONFIG_H

/* ============================ memory map (measured) ========================= */
#define BRD_FLASH_BASE          0x2C000000u   /* XIP cached, boot magic 0xBE57EC1C @ off 0 */
#define BRD_FLASH_NC_BASE       0x28000000u   /* non-cached alias (use for post-erase reads) */
#define BRD_FLASH_MAGIC         0xBE57EC1Cu

#define BRD_SRAM_BASE           0x20000000u
#define BRD_SRAM_SIZE           0x00160000u   /* ~1.4 MB real on-chip RAM (datasheet-confirmed,
                                                 the stock SDK only declares 512 KB) */
#define BRD_SRAM_TOP            (BRD_SRAM_BASE + BRD_SRAM_SIZE)
#define BRD_SRAM_FW_RESERVED    0x0004c000u   /* the running FW links into 0..~296 KB; the rest
                                                 (0x2004c000..0x20160000, ~1.1 MB) is free */
#define BRD_SRAM_ALIAS          0x24000000u   /* mirrors 0x20000000 */

#define BRD_PSRAM_BASE          0x34000000u   /* external PSRAM window (TODO(probe): controller
                                                 base + clock + ZQ/DDR calib before use) */

/* peripheral bank (best1503): CMU 0x40000000, IOMUX 0x40086000, UART0 0x4000B000, I2C0-5
 * 0x40005000.. , QSPI-flash ctrl 0x40140000. Display DSI/LCDC + PSRAM-MC bases: TODO(probe). */
#define BRD_CMU_BASE            0x40000000u
#define BRD_IOMUX_BASE          0x40086000u
#define BRD_UART0_BASE          0x4000B000u

/* ============================ chip-specific fixes =========================== */
#define BRD_PMU_CHIP_ID         0x153u        /* fix #2: best1503, not 0x136 */
#define BRD_LIBC_ROM            0             /* fix #3: MUST be 0 (best1306 ROM libc incompatible) */
/* fix #1 (skip best1306 CMU teardown) lives in hal_cmu_module_init_state (2700-src). */

/* ============================ console / trace =============================== */
#define BRD_TRACE_UART_PADS     "P2_2/P2_3"   /* func-2, = GPIO18/19 */
#define BRD_TRACE_BAUD          1152000u      /* calibrated actual baud of the SDK trace */

/* ============================ GPIO pad map ================================== *
 * Pad index = port*8+pin (P0_0=0 .. P4_3=35, extended P4_4=36, P4_5=37 on the 170-BGA).
 * Safe-to-drive (free GPIO): P0_0..P1_1, P1_4..P2_1, P2_4..P2_7, P3_6/7, P4_4/P4_5.
 * NEVER re-mux/drive (crash the running FW): */
#define BRD_PAD_IS_CRITICAL(p) ( (p)==18 || (p)==19            /* UART0 trace        */ \
                              || ((p)>=24 && (p)<=29)          /* QSPI flash P3_0..5 */ \
                              || (p)==10 || (p)==11            /* P1_2/3             */ \
                              || ((p)>=32 && (p)<=35)          /* P4_0..3            */ \
                              || (p)>=38 )                     /* P4_6+              */
#define BRD_PAD(port,pin)       ((port)*8 + (pin))

/* ---- per-peripheral pin/address assignments — TODO(probe) ----
 * The MiBand9 sensors/touch sit on board-specific pads + a power rail the stock FW enables;
 * a blind sweep can't reach them (most pads are FW-critical). Fill these from a
 * logic-analyzer trace of the stock FW's I2C lines or the schematic. */
#define BRD_TODO_PIN            (-1)
#define BRD_DISPLAY_RST_PIN     BRD_TODO_PIN  /* RM690B0 reset GPIO            */
#define BRD_DISPLAY_TE_PIN      BRD_TODO_PIN  /* DSI tearing-effect input      */
#define BRD_TOUCH_I2C_ID        BRD_TODO_PIN  /* which I2C controller          */
#define BRD_TOUCH_I2C_ADDR      BRD_TODO_PIN
#define BRD_TOUCH_INT_PIN       BRD_TODO_PIN
#define BRD_SENSOR_I2C_ID       BRD_TODO_PIN  /* accel/IMU/HR/ALS bus          */
#define BRD_SENSOR_PWR_PIN      BRD_TODO_PIN  /* sensor power-enable GPIO/LDO  */
#define BRD_IMU_I2C_ADDR        BRD_TODO_PIN  /* e.g. 0x68/0x6A class          */
#define BRD_ALS_I2C_ADDR        BRD_TODO_PIN
#define BRD_HR_I2C_ADDR         BRD_TODO_PIN  /* PPG/heart-rate AFE            */

/* ============================ display panel ================================= */
#define BRD_PANEL_RM690B0       1
#define BRD_PANEL_W             192           /* TODO(probe): confirm exact res */
#define BRD_PANEL_H             490
#define BRD_PANEL_BPP           16            /* RGB565 to start                */
#define BRD_PANEL_FB_PSRAM      1             /* framebuffer in PSRAM (or extended SRAM) */

#endif /* BES2700IMP_BOARD_CONFIG_H */
