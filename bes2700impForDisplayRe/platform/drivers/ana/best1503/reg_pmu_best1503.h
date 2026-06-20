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
#ifndef __REG_PMU_BEST1306_H__
#define __REG_PMU_BEST1306_H__

#include "plat_types.h"

enum PMU_REG_T {
    PMU_REG_METAL_ID            = 0x00,
    PMU_REG_POWER_KEY_CFG       = 0x02,
    PMU_REG_BIAS_CFG            = 0x03,
    PMU_REG_BIAS_CFG2           = 0x04,
    PMU_REG_CHARGER_CFG         = 0x05,
    PMU_REG_LPO_CFG             = 0x06,
    PMU_REG_ANA_CFG             = 0x07,
    PMU_REG_DIG_CFG             = 0x08,
    PMU_REG_IO_CFG              = 0x09,
    PMU_REG_USB_CFG             = 0x0A,
    PMU_REG_CRYSTAL_CFG         = 0x0C,
    PMU_REG_AVDD_EN             = 0x0D,
    PMU_REG_CODEC_CFG           = 0x0E,
    PMU_REG_DCDC_ANA_VOLT       = 0x13,
    PMU_REG_DCDC_EN             = 0x14,
    PMU_REG_DCDC_ANA_CFG_15     = 0x15,
    PMU_REG_DCDC_V2I_BIT_CFG    = 0x16,
    PMU_REG_SLEEP_CFG           = 0x1D,
    PMU_REG_SAR_GPADC_CFG       = 0x1E,
    PMU_REG_INT_MASK            = 0x1F,
    PMU_REG_INT_EN              = 0x20,
    PMU_REG_RTC_DIV_1HZ         = 0x22,
    PMU_REG_RTC_LOAD_LOW        = 0x23,
    PMU_REG_RTC_LOAD_HIGH       = 0x24,
    PMU_REG_RTC_MATCH1_LOW      = 0x27,
    PMU_REG_RTC_MATCH1_HIGH     = 0x28,
    PMU_REG_MIC_BIAS_A          = 0x29,
    PMU_REG_MIC_BIAS_B          = 0x2A,
    PMU_REG_LED_CFG_IO1         = 0x2B,
    PMU_REG_PWM2_TOGGLE         = 0x2D,
    PMU_REG_PWM2_ST1            = 0x2E,
    PMU_REG_PWM2_EN             = 0x2F,
    PMU_REG_PWM2_BR_EN          = 0x30,
    PMU_REG_IPTAT_CORE_I_CFG    = 0x34,
    PMU_REG_DIV_HW_RESET_CFG    = 0x35,
    PMU_REG_WDT_RESET_TIMER     = 0x36,
    PMU_REG_WDT_CFG             = 0x37,
    PMU_REG_RESERVED_ANA        = 0x38,
    PMU_REG_PWMB_TOGGLE         = 0x3A,
    PMU_REG_PWMB_ST1            = 0x3B,
    PMU_REG_PWMB_EN             = 0x3C,
    PMU_REG_PWMB_BR_EN          = 0x3D,
    PMU_REG_LED_CFG_IO2         = 0x3E,
    PMU_REG_USB_CFG_3F          = 0x3F,
    PMU_REG_USB_CFG2            = 0x40,
    PMU_REG_USB_CFG3            = 0x41,
    PMU_REG_WDT_INT_CFG         = 0x43,
    PMU_REG_MIC_BIAS_C          = 0x45,
    PMU_REG_UART1_CFG           = 0x46,
    PMU_REG_UART2_CFG           = 0x47,
    PMU_REG_LED_IO_IN           = 0x48,
    PMU_REG_DCDC_ANA_CFG_49     = 0x49,
    PMU_REG_DCDC_DIG_VOLT       = 0x4A,
    PMU_REG_WDT_IRQ_TIMER       = 0x4D,
    PMU_REG_POWER_OFF           = 0x4F,
    PMU_REG_INT_STATUS          = 0x50,
    PMU_REG_INT_MSKED_STATUS    = 0x51,
    PMU_REG_INT_CLR             = 0x51,
    PMU_REG_RTC_VAL_LOW         = 0x54,
    PMU_REG_RTC_VAL_HIGH        = 0x55,
    PMU_REG_CHARGER_STATUS      = 0x5E,
    PMU_REG_HW_STATUS_5F        = 0x5F,
    PMU_REG_COMMAND_NOT_RESET   = 0x61,
    PMU_REG_DCDC_ANA_DIG_CFG_63 = 0x63,
    PMU_REG_DCDC_DIG_SYNC       = 0x64,
    PMU_REG_MIC_PULL_DOWN       = 0x67,
    PMU_REG_MEM_CFG             = 0x68,
    PMU_REG_PA_CFG              = 0x69,
    PMU_REG_DCDC_RAMP_EN        = 0x6A,
    PMU_REG_SAR_EN              = 0x8F,
    PMU_REG_SAR_CLK_CFG         = 0x90,
    PMU_REG_SAR_PU_CFG          = 0x93,
    PMU_REG_EFUSE_CTRL          = 0xB7,
    PMU_REG_EFUSE_DATA_HIGH     = 0xBD,
    PMU_REG_EFUSE_DATA_LOW      = 0xBE,
    PMU_REG_DCDC_HPPA_CAP       = 0xC1,
    PMU_REG_DCDC_HPPA_VOLT      = 0xC3,
    PMU_REG_DCDC_CFG_C4         = 0xC4,
    PMU_REG_DCDC_HPPA_EN        = 0xCA,
    PMU_REG_AC_PRESENT_CFG      = 0xCB,
    PMU_REG_RC_SLEEP_CFG        = 0xCE,
    PMU_REG_DIG_ANA_VOLT_RC     = 0xD2,
    PMU_REG_VCODEC_VOLT_RC      = 0xD3,
    PMU_REG_BUCK_CFG_D5         = 0xD5,
    PMU_REG_DCDC_SIMO_IX_CFG    = 0xD9,
    PMU_REG_PU_VSYS_DIV         = 0xDA,
    PMU_REG_SENSOR_CFG          = 0xDD,
    PMU_REG_GP_SENS_VOLT_SLEEP  = 0xDE,
    PMU_REG_SAR_VREF_CFG        = 0xE6,

    PMU_REG_MODULE_START        = PMU_REG_ANA_CFG,
};

// REG_00
#define REVID_SHIFT                         0
#define REVID_MASK                          (0xF << REVID_SHIFT)
#define REVID(n)                            BITFIELD_VAL(REVID, n)
#define CHIPID_SHIFT                        4
#define CHIPID_MASK                         (0xFFF << CHIPID_SHIFT)
#define CHIPID(n)                           BITFIELD_VAL(CHIPID, n)

// REG_01
#define LPO_OFF_CNT_SHIFT                   12
#define LPO_OFF_CNT_MASK                    (0xF << LPO_OFF_CNT_SHIFT)
#define LPO_OFF_CNT(n)                      BITFIELD_VAL(LPO_OFF_CNT, n)
#define POWER_ON_DB_CNT_SHIFT               8
#define POWER_ON_DB_CNT_MASK                (0xF << POWER_ON_DB_CNT_SHIFT)
#define POWER_ON_DB_CNT(n)                  BITFIELD_VAL(POWER_ON_DB_CNT, n)
#define POWER_OFF_CNT_SHIFT                 4
#define POWER_OFF_CNT_MASK                  (0xF << POWER_OFF_CNT_SHIFT)
#define POWER_OFF_CNT(n)                    BITFIELD_VAL(POWER_OFF_CNT, n)
#define CLK_STABLE_CNT_SHIFT                0
#define CLK_STABLE_CNT_MASK                 (0xF << CLK_STABLE_CNT_SHIFT)
#define CLK_STABLE_CNT(n)                   BITFIELD_VAL(CLK_STABLE_CNT, n)

// REG_02
#define REG_PU_VBAT_DIV                     (1 << 15)
#define PU_LPO_DR                           (1 << 14)
#define PU_LPO_REG                          (1 << 13)
#define POWERKEY_WAKEUP_OSC_EN              (1 << 12)
#define RTC_POWER_ON_EN                     (1 << 11)
#define PU_ALL_REG                          (1 << 10)
#define RESERVED_ANA_16                     (1 << 9)
#define CLK_32K_SEL_0                       (1 << 8)
#define DEEPSLEEP_MODE_DIGI_DR              (1 << 7)
#define DEEPSLEEP_MODE_DIGI_REG             (1 << 6)
#define RESERVED_ANA_18_17_SHIFT            4
#define RESERVED_ANA_18_17_MASK             (0x3 << RESERVED_ANA_18_17_SHIFT)
#define RESERVED_ANA_18_17(n)               BITFIELD_VAL(RESERVED_ANA_18_17, n)
#define RESETN_ANA_DR                       (1 << 3)
#define RESETN_ANA_REG                      (1 << 2)
#define RESETN_DIG_DR                       (1 << 1)
#define RESETN_DIG_REG                      (1 << 0)

// REG_03
#define RESERVED_ANA_19                     (1 << 15)
#define PU_LP_BIAS_LDO_DSLEEP               (1 << 14)
#define PU_LP_BIAS_LDO_DR                   (1 << 13)
#define PU_LP_BIAS_LDO_REG                  (1 << 12)
#define PU_BIAS_LDO_DR                      (1 << 11)
#define PU_BIAS_LDO_REG                     (1 << 10)
#define BG_VBG_SEL_DR                       (1 << 9)
#define BG_VBG_SEL_REG                      (1 << 8)
#define BG_CONSTANT_GM_BIAS_DR              (1 << 7)
#define BG_CONSTANT_GM_BIAS_REG             (1 << 6)
#define BG_CORE_EN_DR                       (1 << 5)
#define BG_CORE_EN_REG                      (1 << 4)
#define BG_VTOI_EN_DR                       (1 << 3)
#define BG_VTOI_EN_REG                      (1 << 2)
#define BG_NOTCH_EN_DR                      (1 << 1)
#define BG_NOTCH_EN_REG                     (1 << 0)

// REG_04
#define BG_NOTCH_LPF_LOW_BW                 (1 << 15)
#define BG_OPX2                             (1 << 14)
#define BG_PTATX2                           (1 << 13)
#define BG_VBG_RES_SHIFT                    9
#define BG_VBG_RES_MASK                     (0xF << BG_VBG_RES_SHIFT)
#define BG_VBG_RES(n)                       BITFIELD_VAL(BG_VBG_RES, n)
#define BG_CONSTANT_GM_BIAS_BIT_SHIFT       7
#define BG_CONSTANT_GM_BIAS_BIT_MASK        (0x3 << BG_CONSTANT_GM_BIAS_BIT_SHIFT)
#define BG_CONSTANT_GM_BIAS_BIT(n)          BITFIELD_VAL(BG_CONSTANT_GM_BIAS_BIT, n)
#define BG_VTOI_IABS_BIT_SHIFT              2
#define BG_VTOI_IABS_BIT_MASK               (0x1F << BG_VTOI_IABS_BIT_SHIFT)
#define BG_VTOI_IABS_BIT(n)                 BITFIELD_VAL(BG_VTOI_IABS_BIT, n)
#define BG_VTOI_VCAS_BIT_SHIFT              0
#define BG_VTOI_VCAS_BIT_MASK               (0x3 << BG_VTOI_VCAS_BIT_SHIFT)
#define BG_VTOI_VCAS_BIT(n)                 BITFIELD_VAL(BG_VTOI_VCAS_BIT, n)

// REG_05
#define REG_PU_LDO_VRTC_RF_DSLEEP           (1 << 15)
#define REG_PU_LDO_VRTC_RF_DR               (1 << 14)
#define REG_PU_LDO_VRTC_RF_REG              (1 << 13)
#define REG_CHARGE_OUT_INTR_MSK             (1 << 12)
#define REG_CHARGE_IN_INTR_MSK              (1 << 11)
#define REG_AC_ON_OUT_EN                    (1 << 10)
#define REG_AC_ON_IN_EN                     (1 << 9)
#define REG_CHARGE_INTR_EN                  (1 << 8)
#define REG_AC_ON_DB_VALUE_SHIFT            0
#define REG_AC_ON_DB_VALUE_MASK             (0xFF << REG_AC_ON_DB_VALUE_SHIFT)
#define REG_AC_ON_DB_VALUE(n)               BITFIELD_VAL(REG_AC_ON_DB_VALUE, n)

// REG_06
#define REG_BYPASS_VBG_RF_BUFFER_DR         (1 << 15)
#define REG_BYPASS_VBG_RF_BUFFER_REG        (1 << 14)
#define PU_VBG_RF_BUFFER_DR                 (1 << 13)
#define PU_VBG_RF_BUFFER_REG                (1 << 12)
#define RESERVED_ANA_21_20_SHIFT            10
#define RESERVED_ANA_21_20_MASK             (0x3 << RESERVED_ANA_21_20_SHIFT)
#define RESERVED_ANA_21_20(n)               BITFIELD_VAL(RESERVED_ANA_21_20, n)
#define REG_LPO_KTRIM_SHIFT                 4
#define REG_LPO_KTRIM_MASK                  (0x3F << REG_LPO_KTRIM_SHIFT)
#define REG_LPO_KTRIM(n)                    BITFIELD_VAL(REG_LPO_KTRIM, n)
#define REG_LPO_ITRIM_SHIFT                 0
#define REG_LPO_ITRIM_MASK                  (0xF << REG_LPO_ITRIM_SHIFT)
#define REG_LPO_ITRIM(n)                    BITFIELD_VAL(REG_LPO_ITRIM, n)

// REG_07
#define REG_LIGHT_LOAD_VDCDC_LDO            (1 << 15)
#define REG_PULLDOWN_VANA_LDO               (1 << 14)
#define LP_EN_VANA_LDO_DSLEEP               (1 << 13)
#define LP_EN_VANA_LDO_DR                   (1 << 12)
#define LP_EN_VANA_LDO_REG                  (1 << 11)
#define REG_PU_LDO_VANA_DSLEEP              (1 << 10)
#define REG_PU_LDO_VANA_DR                  (1 << 9)
#define REG_PU_LDO_VANA                     (1 << 8)
#define LDO_ANA_VBIT_DSLEEP_SHIFT           4
#define LDO_ANA_VBIT_DSLEEP_MASK            (0xF << LDO_ANA_VBIT_DSLEEP_SHIFT)
#define LDO_ANA_VBIT_DSLEEP(n)              BITFIELD_VAL(LDO_ANA_VBIT_DSLEEP, n)
#define LDO_ANA_VBIT_NORMAL_SHIFT           0
#define LDO_ANA_VBIT_NORMAL_MASK            (0xF << LDO_ANA_VBIT_NORMAL_SHIFT)
#define LDO_ANA_VBIT_NORMAL(n)              BITFIELD_VAL(LDO_ANA_VBIT_NORMAL, n)

#define REG_PU_LDO_VANA_REG                 REG_PU_LDO_VANA
#define LDO_VANA_VBIT_DSLEEP_SHIFT          LDO_ANA_VBIT_DSLEEP_SHIFT
#define LDO_VANA_VBIT_DSLEEP_MASK           LDO_ANA_VBIT_DSLEEP_MASK
#define LDO_VANA_VBIT_DSLEEP(n)             LDO_ANA_VBIT_DSLEEP(n)
#define LDO_VANA_VBIT_NORMAL_SHIFT          LDO_ANA_VBIT_NORMAL_SHIFT
#define LDO_VANA_VBIT_NORMAL_MASK           LDO_ANA_VBIT_NORMAL_MASK
#define LDO_VANA_VBIT_NORMAL(n)             LDO_ANA_VBIT_NORMAL(n)

// REG_08
#define REG_LOOP_CTL_VCORE_LDO              (1 << 15)
#define REG_PULLDOWN_VCORE                  (1 << 14)
#define LP_EN_VCORE_LDO_DSLEEP              (1 << 13)
#define LP_EN_VCORE_LDO_DR                  (1 << 12)
#define LP_EN_VCORE_LDO_REG                 (1 << 11)
#define REG_PU_LDO_DIG_DSLEEP               (1 << 10)
#define REG_PU_LDO_DIG_DR                   (1 << 9)
#define REG_PU_LDO_DIG_REG                  (1 << 8)
#define LDO_DIG_VBIT_DSLEEP_SHIFT           4
#define LDO_DIG_VBIT_DSLEEP_MASK            (0xF << LDO_DIG_VBIT_DSLEEP_SHIFT)
#define LDO_DIG_VBIT_DSLEEP(n)              BITFIELD_VAL(LDO_DIG_VBIT_DSLEEP, n)
#define LDO_DIG_VBIT_NORMAL_SHIFT           0
#define LDO_DIG_VBIT_NORMAL_MASK            (0xF << LDO_DIG_VBIT_NORMAL_SHIFT)
#define LDO_DIG_VBIT_NORMAL(n)              BITFIELD_VAL(LDO_DIG_VBIT_NORMAL, n)

#define LP_EN_VDIG_LDO_DSLEEP               LP_EN_VCORE_LDO_DSLEEP
#define LP_EN_VDIG_LDO_DR                   LP_EN_VCORE_LDO_DR
#define LP_EN_VDIG_LDO_REG                  LP_EN_VCORE_LDO_REG
#define REG_PU_LDO_VDIG_DSLEEP              REG_PU_LDO_DIG_DSLEEP
#define REG_PU_LDO_VDIG_DR                  REG_PU_LDO_DIG_DR
#define REG_PU_LDO_VDIG_REG                 REG_PU_LDO_DIG_REG
#define LDO_VDIG_VBIT_DSLEEP_SHIFT          LDO_DIG_VBIT_DSLEEP_SHIFT
#define LDO_VDIG_VBIT_DSLEEP_MASK           LDO_DIG_VBIT_DSLEEP_MASK
#define LDO_VDIG_VBIT_DSLEEP(n)             LDO_DIG_VBIT_DSLEEP(n)
#define LDO_VDIG_VBIT_NORMAL_SHIFT          LDO_DIG_VBIT_NORMAL_SHIFT
#define LDO_VDIG_VBIT_NORMAL_MASK           LDO_DIG_VBIT_NORMAL_MASK
#define LDO_VDIG_VBIT_NORMAL(n)             LDO_DIG_VBIT_NORMAL(n)

// REG_09
#define LP_EN_VIO_LDO_DSLEEP                (1 << 15)
#define LP_EN_VIO_LDO_DR                    (1 << 14)
#define LP_EN_VIO_LDO_REG                   (1 << 13)
#define REG_PU_LDO_VIO_DSLEEP               (1 << 12)
#define REG_PU_LDO_VIO_DR                   (1 << 11)
#define REG_PU_LDO_VIO                      (1 << 10)
#define LDO_VIO_VBIT_NORMAL_SHIFT           5
#define LDO_VIO_VBIT_NORMAL_MASK            (0x1F << LDO_VIO_VBIT_NORMAL_SHIFT)
#define LDO_VIO_VBIT_NORMAL(n)              BITFIELD_VAL(LDO_VIO_VBIT_NORMAL, n)
#define LDO_VIO_VBIT_DSLEEP_SHIFT           0
#define LDO_VIO_VBIT_DSLEEP_MASK            (0x1F << LDO_VIO_VBIT_DSLEEP_SHIFT)
#define LDO_VIO_VBIT_DSLEEP(n)              BITFIELD_VAL(LDO_VIO_VBIT_DSLEEP, n)

#define REG_PU_LDO_VIO_REG                  REG_PU_LDO_VIO
// REG_0A
#define LP_EN_VUSB_LDO_DSLEEP               (1 << 15)
#define LP_EN_VUSB_LDO_DR                   (1 << 14)
#define LP_EN_VUSB_LDO_REG                  (1 << 13)
#define REG_PU_LDO_VUSB_DSLEEP              (1 << 12)
#define REG_PU_LDO_VUSB_DR                  (1 << 11)
#define REG_PU_LDO_VUSB_REG                 (1 << 10)
#define LDO_VUSB_VBIT_NORMAL_SHIFT          5
#define LDO_VUSB_VBIT_NORMAL_MASK           (0x1F << LDO_VUSB_VBIT_NORMAL_SHIFT)
#define LDO_VUSB_VBIT_NORMAL(n)             BITFIELD_VAL(LDO_VUSB_VBIT_NORMAL, n)
#define LDO_VUSB_VBIT_DSLEEP_SHIFT          0
#define LDO_VUSB_VBIT_DSLEEP_MASK           (0x1F << LDO_VUSB_VBIT_DSLEEP_SHIFT)
#define LDO_VUSB_VBIT_DSLEEP(n)             BITFIELD_VAL(LDO_VUSB_VBIT_DSLEEP, n)

// REG_0B
#define REG_PULLDOWN_VIO                    (1 << 15)
#define REG_PULLDOWN_VUSB                   (1 << 14)
#define REG_LP_BIAS_SEL_LDO_DR              (1 << 8)
#define POWER_DOWN_DB_CNT_SHIFT             4
#define POWER_DOWN_DB_CNT_MASK              (0xF << POWER_DOWN_DB_CNT_SHIFT)
#define POWER_DOWN_DB_CNT(n)                BITFIELD_VAL(POWER_DOWN_DB_CNT, n)
#define REG_LP_BIAS_SEL_RAMP_EN             (1 << 3)
#define REG_VCORE_L_RAMP_EN                 (1 << 2)
#define REG_VUSB_RAMP_EN                    (1 << 1)
#define REG_VCORE_RAMP_EN                   (1 << 0)

// REG_0C
#define REG_VCORE_SSTIME_MODE_SHIFT         14
#define REG_VCORE_SSTIME_MODE_MASK          (0x3 << REG_VCORE_SSTIME_MODE_SHIFT)
#define REG_VCORE_SSTIME_MODE(n)            BITFIELD_VAL(REG_VCORE_SSTIME_MODE, n)
#define REG_HW_WDT_COM_RST_TIME_SHIFT       0
#define REG_HW_WDT_COM_RST_TIME_MASK        (0xFFF << REG_HW_WDT_COM_RST_TIME_SHIFT)
#define REG_HW_WDT_COM_RST_TIME(n)          BITFIELD_VAL(REG_HW_WDT_COM_RST_TIME, n)

// REG_0D
#define REG_BUCK_CC_CAP_BIT_SHIFT           12
#define REG_BUCK_CC_CAP_BIT_MASK            (0xF << REG_BUCK_CC_CAP_BIT_SHIFT)
#define REG_BUCK_CC_CAP_BIT(n)              BITFIELD_VAL(REG_BUCK_CC_CAP_BIT, n)
#define REG_BUCK_CC_ILIMIT_SHIFT            8
#define REG_BUCK_CC_ILIMIT_MASK             (0xF << REG_BUCK_CC_ILIMIT_SHIFT)
#define REG_BUCK_CC_ILIMIT(n)               BITFIELD_VAL(REG_BUCK_CC_ILIMIT, n)
#define REG_LP_BIAS_SEL_LDO_SHIFT           6
#define REG_LP_BIAS_SEL_LDO_MASK            (0x3 << REG_LP_BIAS_SEL_LDO_SHIFT)
#define REG_LP_BIAS_SEL_LDO(n)              BITFIELD_VAL(REG_LP_BIAS_SEL_LDO, n)
#define REG_LP_EN_VUSB11                    (1 << 5)
#define REG_BUCK_ANA_IS_GAIN_DSLEEP_SHIFT   2
#define REG_BUCK_ANA_IS_GAIN_DSLEEP_MASK    (0x7 << REG_BUCK_ANA_IS_GAIN_DSLEEP_SHIFT)
#define REG_BUCK_ANA_IS_GAIN_DSLEEP(n)      BITFIELD_VAL(REG_BUCK_ANA_IS_GAIN_DSLEEP, n)
#define REG_PU_AVDD25_ANA                   (1 << 1)
#define REG_BYPASS_VCORE                    (1 << 0)

// REG_0E
#define PU_LDO_VCODEC_DSLEEP                (1 << 15)
#define PU_LDO_VCODEC_DR                    (1 << 14)
#define PU_LDO_VCODEC_REG                   (1 << 13)
#define LP_EN_VCODEC_LDO_DSLEEP             (1 << 12)
#define LP_EN_VCODEC_LDO_DR                 (1 << 11)
#define LP_EN_VCODEC_LDO_REG                (1 << 10)
#define LDO_VCODEC_VBIT_NORMAL_SHIFT        5
#define LDO_VCODEC_VBIT_NORMAL_MASK         (0x1F << LDO_VCODEC_VBIT_NORMAL_SHIFT)
#define LDO_VCODEC_VBIT_NORMAL(n)           BITFIELD_VAL(LDO_VCODEC_VBIT_NORMAL, n)
#define LDO_VCODEC_VBIT_DSLEEP_SHIFT        0
#define LDO_VCODEC_VBIT_DSLEEP_MASK         (0x1F << LDO_VCODEC_VBIT_DSLEEP_SHIFT)
#define LDO_VCODEC_VBIT_DSLEEP(n)           BITFIELD_VAL(LDO_VCODEC_VBIT_DSLEEP, n)

#define REG_PU_LDO_VCODEC_DSLEEP            PU_LDO_VCODEC_DSLEEP
#define REG_PU_LDO_VCODEC_DR                PU_LDO_VCODEC_DR
#define REG_PU_LDO_VCODEC_REG               PU_LDO_VCODEC_REG

// REG_0F
#define REG_LOW_VIO                         (1 << 15)
#define REG_PULLDOWN_VCODEC                 (1 << 14)
#define REG_UVLO_SEL_SHIFT                  12
#define REG_UVLO_SEL_MASK                   (0x3 << REG_UVLO_SEL_SHIFT)
#define REG_UVLO_SEL(n)                     BITFIELD_VAL(REG_UVLO_SEL, n)
#define POWER_UP_SOFT_CNT_SHIFT             8
#define POWER_UP_SOFT_CNT_MASK              (0xF << POWER_UP_SOFT_CNT_SHIFT)
#define POWER_UP_SOFT_CNT(n)                BITFIELD_VAL(POWER_UP_SOFT_CNT, n)
#define POWER_UP_BIAS_CNT_SHIFT             4
#define POWER_UP_BIAS_CNT_MASK              (0xF << POWER_UP_BIAS_CNT_SHIFT)
#define POWER_UP_BIAS_CNT(n)                BITFIELD_VAL(POWER_UP_BIAS_CNT, n)
#define RESETN_MOD2_CNT_SHIFT               0
#define RESETN_MOD2_CNT_MASK                (0xF << RESETN_MOD2_CNT_SHIFT)
#define RESETN_MOD2_CNT(n)                  BITFIELD_VAL(RESETN_MOD2_CNT, n)

// REG_10
#define POWER_UP_MOD2_CNT_SHIFT             8
#define POWER_UP_MOD2_CNT_MASK              (0xFF << POWER_UP_MOD2_CNT_SHIFT)
#define POWER_UP_MOD2_CNT(n)                BITFIELD_VAL(POWER_UP_MOD2_CNT, n)
#define POWER_UP_MOD1_CNT_SHIFT             0
#define POWER_UP_MOD1_CNT_MASK              (0xFF << POWER_UP_MOD1_CNT_SHIFT)
#define POWER_UP_MOD1_CNT(n)                BITFIELD_VAL(POWER_UP_MOD1_CNT, n)

// REG_11
#define POWER_UP_MOD4_CNT_SHIFT             8
#define POWER_UP_MOD4_CNT_MASK              (0xFF << POWER_UP_MOD4_CNT_SHIFT)
#define POWER_UP_MOD4_CNT(n)                BITFIELD_VAL(POWER_UP_MOD4_CNT, n)
#define POWER_UP_MOD3_CNT_SHIFT             0
#define POWER_UP_MOD3_CNT_MASK              (0xFF << POWER_UP_MOD3_CNT_SHIFT)
#define POWER_UP_MOD3_CNT(n)                BITFIELD_VAL(POWER_UP_MOD3_CNT, n)

// REG_12
#define POWER_UP_MOD6_CNT_SHIFT             8
#define POWER_UP_MOD6_CNT_MASK              (0xFF << POWER_UP_MOD6_CNT_SHIFT)
#define POWER_UP_MOD6_CNT(n)                BITFIELD_VAL(POWER_UP_MOD6_CNT, n)
#define POWER_UP_MOD5_CNT_SHIFT             0
#define POWER_UP_MOD5_CNT_MASK              (0xFF << POWER_UP_MOD5_CNT_SHIFT)
#define POWER_UP_MOD5_CNT(n)                BITFIELD_VAL(POWER_UP_MOD5_CNT, n)

// REG_13
#define BUCK_VANA_BIT_NORMAL_SHIFT          8
#define BUCK_VANA_BIT_NORMAL_MASK           (0xFF << BUCK_VANA_BIT_NORMAL_SHIFT)
#define BUCK_VANA_BIT_NORMAL(n)             BITFIELD_VAL(BUCK_VANA_BIT_NORMAL, n)
#define BUCK_VANA_BIT_DSLEEP_SHIFT          0
#define BUCK_VANA_BIT_DSLEEP_MASK           (0xFF << BUCK_VANA_BIT_DSLEEP_SHIFT)
#define BUCK_VANA_BIT_DSLEEP(n)             BITFIELD_VAL(BUCK_VANA_BIT_DSLEEP, n)

// REG_14
#define TEST_MODE_SHIFT                     13
#define TEST_MODE_MASK                      (0x7 << TEST_MODE_SHIFT)
#define TEST_MODE(n)                        BITFIELD_VAL(TEST_MODE, n)
#define REG_BUCK_CC_MODE                    (1 << 12)
#define DCDC_ANA_LP_EN_DSLEEP               (1 << 11)
#define PU_DCDC_ANA_DR                      (1 << 10)
#define PU_DCDC_ANA_REG                     (1 << 9)
#define DCDC_ANA_LP_EN_DR                   (1 << 8)
#define DCDC_ANA_LP_EN_REG                  (1 << 7)
#define PU_DCDC_ANA_DSLEEP                  (1 << 6)
#define DCDC_DIG_LP_EN_DSLEEP               (1 << 5)
#define PU_DCDC_DIG_DR                      (1 << 4)
#define PU_DCDC_DIG_REG                     (1 << 3)
#define DCDC_DIG_LP_EN_DR                   (1 << 2)
#define DCDC_DIG_LP_EN_REG                  (1 << 1)
#define PU_DCDC_DIG_DSLEEP                  (1 << 0)

// REG_15
#define REG_BUCK_PDRV_BIT_SHIFT             14
#define REG_BUCK_PDRV_BIT_MASK              (0x3 << REG_BUCK_PDRV_BIT_SHIFT)
#define REG_BUCK_PDRV_BIT(n)                BITFIELD_VAL(REG_BUCK_PDRV_BIT, n)
#define REG_BUCK_ANA_IS_GAIN_NORMAL_SHIFT   11
#define REG_BUCK_ANA_IS_GAIN_NORMAL_MASK    (0x7 << REG_BUCK_ANA_IS_GAIN_NORMAL_SHIFT)
#define REG_BUCK_ANA_IS_GAIN_NORMAL(n)      BITFIELD_VAL(REG_BUCK_ANA_IS_GAIN_NORMAL, n)
#define REG_BUCK_ANA_INTERNAL_FREQUENCY_SHIFT 8
#define REG_BUCK_ANA_INTERNAL_FREQUENCY_MASK (0x3 << REG_BUCK_ANA_INTERNAL_FREQUENCY_SHIFT)
#define REG_BUCK_ANA_INTERNAL_FREQUENCY(n)  BITFIELD_VAL(REG_BUCK_ANA_INTERNAL_FREQUENCY, n)
#define REG_BUCK_ANA_LOW_VOLTAGE            (1 << 7)
#define REG_BUCK_ANA_DT_BIT                 (1 << 6)
#define REG_BUCK_ANA_PULLDOWN_EN            (1 << 5)
#define REG_BUCK_ANA_ANTI_RES_DISABLE       (1 << 4)
#define REG_BUCK_ANA_SYNC_DISABLE           (1 << 3)
#define REG_BUCK_ANA_SOFT_START_EN          (1 << 2)
#define REG_BUCK_ANA_RECOVER                (1 << 1)
#define REG_BUCK_ANA_SLOPE_DOUBLE           (1 << 0)

// REG_16
#define REG_BUCK_CC_SENSE_BIT_SHIFT         14
#define REG_BUCK_CC_SENSE_BIT_MASK          (0x3 << REG_BUCK_CC_SENSE_BIT_SHIFT)
#define REG_BUCK_CC_SENSE_BIT(n)            BITFIELD_VAL(REG_BUCK_CC_SENSE_BIT, n)
#define REG_BUCK_CC_SLOPE_BIT_SHIFT         12
#define REG_BUCK_CC_SLOPE_BIT_MASK          (0x3 << REG_BUCK_CC_SLOPE_BIT_SHIFT)
#define REG_BUCK_CC_SLOPE_BIT(n)            BITFIELD_VAL(REG_BUCK_CC_SLOPE_BIT, n)
#define REG_BUCK_I2V_BIT2_SHIFT             9
#define REG_BUCK_I2V_BIT2_MASK              (0x7 << REG_BUCK_I2V_BIT2_SHIFT)
#define REG_BUCK_I2V_BIT2(n)                BITFIELD_VAL(REG_BUCK_I2V_BIT2, n)
#define REG_BUCK_I2V_BIT1_SHIFT             6
#define REG_BUCK_I2V_BIT1_MASK              (0x7 << REG_BUCK_I2V_BIT1_SHIFT)
#define REG_BUCK_I2V_BIT1(n)                BITFIELD_VAL(REG_BUCK_I2V_BIT1, n)
#define REG_BUCK_V2I_BIT1_SHIFT             3
#define REG_BUCK_V2I_BIT1_MASK              (0x7 << REG_BUCK_V2I_BIT1_SHIFT)
#define REG_BUCK_V2I_BIT1(n)                BITFIELD_VAL(REG_BUCK_V2I_BIT1, n)
#define REG_BUCK_V2I_BIT2_SHIFT             0
#define REG_BUCK_V2I_BIT2_MASK              (0x7 << REG_BUCK_V2I_BIT2_SHIFT)
#define REG_BUCK_V2I_BIT2(n)                BITFIELD_VAL(REG_BUCK_V2I_BIT2, n)

// REG_17
#define REG_SS_VCORE_EN                     (1 << 15)
#define RESERVED_ANA_23                     (1 << 14)
#define REG_BUCK_HV_ANA_NORMAL              (1 << 13)
#define REG_BUCK_HV_ANA_DSLEEP              (1 << 12)
#define REG_GPADC_RESETN_DR                 (1 << 11)
#define REG_GPADC_RESETN                    (1 << 10)
#define SAR_PWR_BIT_SHIFT                   8
#define SAR_PWR_BIT_MASK                    (0x3 << SAR_PWR_BIT_SHIFT)
#define SAR_PWR_BIT(n)                      BITFIELD_VAL(SAR_PWR_BIT, n)
#define SAR_OP_IBIT_SHIFT                   5
#define SAR_OP_IBIT_MASK                    (0x7 << SAR_OP_IBIT_SHIFT)
#define SAR_OP_IBIT(n)                      BITFIELD_VAL(SAR_OP_IBIT, n)
#define SAR_THERM_GAIN_SHIFT                3
#define SAR_THERM_GAIN_MASK                 (0x3 << SAR_THERM_GAIN_SHIFT)
#define SAR_THERM_GAIN(n)                   BITFIELD_VAL(SAR_THERM_GAIN, n)
#define SAR_VREF_BIT_SHIFT                  0
#define SAR_VREF_BIT_MASK                   (0x7 << SAR_VREF_BIT_SHIFT)
#define SAR_VREF_BIT(n)                     BITFIELD_VAL(SAR_VREF_BIT, n)

// REG_18
#define SAR_MODE_SEL                        (1 << 15)
#define KEY_DB_DSB                          (1 << 14)
#define KEY_INTERVAL_MODE                   (1 << 13)
#define GPADC_INTERVAL_MODE                 (1 << 12)
#define STATE_RESET                         (1 << 11)
#define SAR_ADC_MODE                        (1 << 10)
#define DELAY_BEFORE_SAMP_SHIFT             8
#define DELAY_BEFORE_SAMP_MASK              (0x3 << DELAY_BEFORE_SAMP_SHIFT)
#define DELAY_BEFORE_SAMP(n)                BITFIELD_VAL(DELAY_BEFORE_SAMP, n)
#define CONV_CLK_INV                        (1 << 7)
#define SAMP_CLK_INV                        (1 << 6)
#define TIME_SAMP_NEG                       (1 << 5)
#define TIME_SAMP_POS                       (1 << 4)
#define SAR_OUT_POLARITY                    (1 << 3)
#define TIMER_SAR_STABLE_SEL_SHIFT          0
#define TIMER_SAR_STABLE_SEL_MASK           (0x7 << TIMER_SAR_STABLE_SEL_SHIFT)
#define TIMER_SAR_STABLE_SEL(n)             BITFIELD_VAL(TIMER_SAR_STABLE_SEL, n)

// REG_19
#define REG_CLK_GPADC_DIV_VALUE_SHIFT       11
#define REG_CLK_GPADC_DIV_VALUE_MASK        (0x1F << REG_CLK_GPADC_DIV_VALUE_SHIFT)
#define REG_CLK_GPADC_DIV_VALUE(n)          BITFIELD_VAL(REG_CLK_GPADC_DIV_VALUE, n)
#define REG_KEY_IN_DR                       (1 << 10)
#define REG_KEY_IN_REG                      (1 << 9)
#define REG_GPADC_EN_DR                     (1 << 8)
#define REG_GPADC_EN_REG_SHIFT              0
#define REG_GPADC_EN_REG_MASK               (0xFF << REG_GPADC_EN_REG_SHIFT)
#define REG_GPADC_EN_REG(n)                 BITFIELD_VAL(REG_GPADC_EN_REG, n)

// REG_1A
#define REG_PMU_VSEL1_SHIFT                 13
#define REG_PMU_VSEL1_MASK                  (0x7 << REG_PMU_VSEL1_SHIFT)
#define REG_PMU_VSEL1(n)                    BITFIELD_VAL(REG_PMU_VSEL1, n)
#define REG_POWER_SEL_CNT_SHIFT             8
#define REG_POWER_SEL_CNT_MASK              (0x1F << REG_POWER_SEL_CNT_SHIFT)
#define REG_POWER_SEL_CNT(n)                BITFIELD_VAL(REG_POWER_SEL_CNT, n)
#define REG_PWR_SEL_DR                      (1 << 7)
#define REG_PWR_SEL                         (1 << 6)
#define CLK_BG_EN                           (1 << 5)
#define CLK_BG_DIV_VALUE_SHIFT              0
#define CLK_BG_DIV_VALUE_MASK               (0x1F << CLK_BG_DIV_VALUE_SHIFT)
#define CLK_BG_DIV_VALUE(n)                 BITFIELD_VAL(CLK_BG_DIV_VALUE, n)

// REG_1B
#define KEY_INTERVAL_SHIFT                  0
#define KEY_INTERVAL_MASK                   (0xFFFF << KEY_INTERVAL_SHIFT)
#define KEY_INTERVAL(n)                     BITFIELD_VAL(KEY_INTERVAL, n)

// REG_1C
#define GPADC_INTERVAL_SHIFT                0
#define GPADC_INTERVAL_MASK                 (0xFFFF << GPADC_INTERVAL_SHIFT)
#define GPADC_INTERVAL(n)                   BITFIELD_VAL(GPADC_INTERVAL, n)

// REG_1D
#define SLEEP_ALLOW                         (1 << 15)
#define CHIP_ADDR_I2C_SHIFT                 8
#define CHIP_ADDR_I2C_MASK                  (0x7F << CHIP_ADDR_I2C_SHIFT)
#define CHIP_ADDR_I2C(n)                    BITFIELD_VAL(CHIP_ADDR_I2C, n)
#define CHAN_EN_REG_SHIFT                   0
#define CHAN_EN_REG_MASK                    (0xFF << CHAN_EN_REG_SHIFT)
#define CHAN_EN_REG(n)                      BITFIELD_VAL(CHAN_EN_REG, n)

// REG_1E
#define RESERVED_ANA_24                     (1 << 15)
#define REG_SAR_REF_MODE_SEL                (1 << 14)
#define REG_CLK_GPADC_EN                    (1 << 13)
#define REG_DR_PU_SAR                       (1 << 12)
#define REG_PU_SAR                          (1 << 11)
#define REG_DR_TSC_SAR_BIT                  (1 << 10)
#define REG_TSC_SAR_BIT_SHIFT               0
#define REG_TSC_SAR_BIT_MASK                (0x3FF << REG_TSC_SAR_BIT_SHIFT)
#define REG_TSC_SAR_BIT(n)                  BITFIELD_VAL(REG_TSC_SAR_BIT, n)

// REG_1F
#define USB_INSERT_INTR_MSK                 (1 << 15)
#define RTC_INT1_MSK                        (1 << 14)
#define RTC_INT0_MSK                        (1 << 13)
#define KEY_ERR1_INTR_MSK                   (1 << 12)
#define KEY_ERR0_INTR_MSK                   (1 << 11)
#define KEY_PRESS_INTR_MSK                  (1 << 10)
#define KEY_RELEASE_INTR_MSK                (1 << 9)
#define SAMPLE_DONE_INTR_MSK                (1 << 8)
#define CHAN_DATA_INTR_MSK_SHIFT            0
#define CHAN_DATA_INTR_MSK_MASK             (0xFF << CHAN_DATA_INTR_MSK_SHIFT)
#define CHAN_DATA_INTR_MSK(n)               BITFIELD_VAL(CHAN_DATA_INTR_MSK, n)

// REG_20
#define USB_INSERT_INTR_EN                  (1 << 15)
#define RTC_INT_EN_1                        (1 << 14)
#define RTC_INT_EN_0                        (1 << 13)
#define KEY_ERR1_INTR_EN                    (1 << 12)
#define KEY_ERR0_INTR_EN                    (1 << 11)
#define KEY_PRESS_INTR_EN                   (1 << 10)
#define KEY_RELEASE_INTR_EN                 (1 << 9)
#define SAMPLE_DONE_INTR_EN                 (1 << 8)
#define CHAN_DATA_INTR_EN_SHIFT             0
#define CHAN_DATA_INTR_EN_MASK              (0xFF << CHAN_DATA_INTR_EN_SHIFT)
#define CHAN_DATA_INTR_EN(n)                BITFIELD_VAL(CHAN_DATA_INTR_EN, n)

// REG_21
#define RESERVED_ANA_26_25_SHIFT            14
#define RESERVED_ANA_26_25_MASK             (0x3 << RESERVED_ANA_26_25_SHIFT)
#define RESERVED_ANA_26_25(n)               BITFIELD_VAL(RESERVED_ANA_26_25, n)
#define USB_POL_USB_RX_DP                   (1 << 13)
#define USB_POL_USB_RX_DM                   (1 << 12)
#define REG_EN_USBDIGPHY_CLK                (1 << 11)
#define RESETN_USBPHY_DR                    (1 << 10)
#define RESETN_USBPHY_REG                   (1 << 9)

#define REG_VCORE_BIT_START_SHIFT           4
#define REG_VCORE_BIT_START_MASK            (0xF << REG_VCORE_BIT_START_SHIFT)
#define REG_VCORE_BIT_START(n)              BITFIELD_VAL(REG_VCORE_BIT_START, n)
#define REG_VCORE_BIT_STOP_SHIFT            0
#define REG_VCORE_BIT_STOP_MASK             (0xF << REG_VCORE_BIT_STOP_SHIFT)
#define REG_VCORE_BIT_STOP(n)               BITFIELD_VAL(REG_VCORE_BIT_STOP, n)

// REG_22
#define CFG_DIV_RTC_1HZ_SHIFT               0
#define CFG_DIV_RTC_1HZ_MASK                (0xFFFF << CFG_DIV_RTC_1HZ_SHIFT)
#define CFG_DIV_RTC_1HZ(n)                  BITFIELD_VAL(CFG_DIV_RTC_1HZ, n)

// REG_23
#define RTC_LOAD_VALUE_15_0_SHIFT           0
#define RTC_LOAD_VALUE_15_0_MASK            (0xFFFF << RTC_LOAD_VALUE_15_0_SHIFT)
#define RTC_LOAD_VALUE_15_0(n)              BITFIELD_VAL(RTC_LOAD_VALUE_15_0, n)

// REG_24
#define RTC_LOAD_VALUE_31_16_SHIFT          0
#define RTC_LOAD_VALUE_31_16_MASK           (0xFFFF << RTC_LOAD_VALUE_31_16_SHIFT)
#define RTC_LOAD_VALUE_31_16(n)             BITFIELD_VAL(RTC_LOAD_VALUE_31_16, n)

// REG_25
#define RTC_MATCH_VALUE_0_15_0_SHIFT        0
#define RTC_MATCH_VALUE_0_15_0_MASK         (0xFFFF << RTC_MATCH_VALUE_0_15_0_SHIFT)
#define RTC_MATCH_VALUE_0_15_0(n)           BITFIELD_VAL(RTC_MATCH_VALUE_0_15_0, n)

// REG_26
#define RTC_MATCH_VALUE_0_31_16_SHIFT       0
#define RTC_MATCH_VALUE_0_31_16_MASK        (0xFFFF << RTC_MATCH_VALUE_0_31_16_SHIFT)
#define RTC_MATCH_VALUE_0_31_16(n)          BITFIELD_VAL(RTC_MATCH_VALUE_0_31_16, n)

// REG_27
#define RTC_MATCH_VALUE_1_15_0_SHIFT        0
#define RTC_MATCH_VALUE_1_15_0_MASK         (0xFFFF << RTC_MATCH_VALUE_1_15_0_SHIFT)
#define RTC_MATCH_VALUE_1_15_0(n)           BITFIELD_VAL(RTC_MATCH_VALUE_1_15_0, n)

// REG_28
#define RTC_MATCH_VALUE_1_31_16_SHIFT       0
#define RTC_MATCH_VALUE_1_31_16_MASK        (0xFFFF << RTC_MATCH_VALUE_1_31_16_SHIFT)
#define RTC_MATCH_VALUE_1_31_16(n)          BITFIELD_VAL(RTC_MATCH_VALUE_1_31_16, n)

// REG_29
#define REG_MIC_BIASA_CHANSEL_SHIFT         14
#define REG_MIC_BIASA_CHANSEL_MASK          (0x3 << REG_MIC_BIASA_CHANSEL_SHIFT)
#define REG_MIC_BIASA_CHANSEL(n)            BITFIELD_VAL(REG_MIC_BIASA_CHANSEL, n)
#define REG_MIC_BIASA_EN                    (1 << 13)
#define REG_MIC_BIASA_ENLPF                 (1 << 12)
#define RESERVED_ANA_22                     (1 << 11)
#define REG_MIC_BIASA_VSEL_SHIFT            5
#define REG_MIC_BIASA_VSEL_MASK             (0x3F << REG_MIC_BIASA_VSEL_SHIFT)
#define REG_MIC_BIASA_VSEL(n)               BITFIELD_VAL(REG_MIC_BIASA_VSEL, n)
#define REG_MIC_LDO_RES_SHIFT               1
#define REG_MIC_LDO_RES_MASK                (0xF << REG_MIC_LDO_RES_SHIFT)
#define REG_MIC_LDO_RES(n)                  BITFIELD_VAL(REG_MIC_LDO_RES, n)
#define REG_MIC_LDO_LOOPCTRL                (1 << 0)

// REG_2A
#define REG_MIC_BIASB_CHANSEL_SHIFT         14
#define REG_MIC_BIASB_CHANSEL_MASK          (0x3 << REG_MIC_BIASB_CHANSEL_SHIFT)
#define REG_MIC_BIASB_CHANSEL(n)            BITFIELD_VAL(REG_MIC_BIASB_CHANSEL, n)
#define REG_MIC_BIASB_EN                    (1 << 13)
#define REG_MIC_BIASB_ENLPF                 (1 << 12)
#define CLK_32K_COUNT_EN                    (1 << 11)
#define REG_MIC_BIASB_VSEL_SHIFT            5
#define REG_MIC_BIASB_VSEL_MASK             (0x3F << REG_MIC_BIASB_VSEL_SHIFT)
#define REG_MIC_BIASB_VSEL(n)               BITFIELD_VAL(REG_MIC_BIASB_VSEL, n)
#define REG_MIC_LDO_EN                      (1 << 4)
#define REG_MIC_LDO_PULLDOWN                (1 << 3)
#define USB_DEBOUNCE_EN                     (1 << 2)
#define USB_NOLS_MODE                       (1 << 1)
#define USB_INSERT_DET_EN                   (1 << 0)

#define MAX_VMIC_BIAS_VSEL_BIT_VAL          (REG_MIC_BIASA_VSEL_MASK >> REG_MIC_BIASA_VSEL_SHIFT)

// REG_2B
#define REG_LED_IO1_IBIT_SHIFT              14
#define REG_LED_IO1_IBIT_MASK               (0x3 << REG_LED_IO1_IBIT_SHIFT)
#define REG_LED_IO1_IBIT(n)                 BITFIELD_VAL(REG_LED_IO1_IBIT, n)
#define REG_LED_IO1_OENB_PRE                (1 << 13)
#define REG_LED_IO1_PDEN                    (1 << 12)
#define REG_LED_IO1_PU                      (1 << 11)
#define REG_LED_IO1_PUEN                    (1 << 10)
#define REG_LED_IO1_SEL_SHIFT               8
#define REG_LED_IO1_SEL_MASK                (0x3 << REG_LED_IO1_SEL_SHIFT)
#define REG_LED_IO1_SEL(n)                  BITFIELD_VAL(REG_LED_IO1_SEL, n)
#define REG_LED_IO1_RX_EN                   (1 << 7)
#define REG_LED_IO1_AIO_EN                  (1 << 6)
#define RESERVED_ANA_27                     (1 << 5)
#define VBATDET_RES_SEL_SHIFT               0
#define VBATDET_RES_SEL_MASK                (0x1F << VBATDET_RES_SEL_SHIFT)
#define VBATDET_RES_SEL(n)                  BITFIELD_VAL(VBATDET_RES_SEL, n)

// REG_2C
#define VBATDET_LP_MODE                     (1 << 15)
#define PU_VBATDET                          (1 << 14)
#define RESERVED_ANA_28                     (1 << 13)
#define PU_LDO_VUSB11_DSLEEP                (1 << 12)
#define PU_LDO_VUSB11_DR                    (1 << 11)
#define PU_LDO_VUSB11_REG                   (1 << 10)
#define REG_BYPASS_VUSB11                   (1 << 9)
#define REG_PULLDOWN_VUSB11                 (1 << 8)
#define LDO_VUSB11_VBIT_NORMAL_SHIFT        4
#define LDO_VUSB11_VBIT_NORMAL_MASK         (0xF << LDO_VUSB11_VBIT_NORMAL_SHIFT)
#define LDO_VUSB11_VBIT_NORMAL(n)           BITFIELD_VAL(LDO_VUSB11_VBIT_NORMAL, n)
#define LDO_VUSB11_VBIT_DSLEEP_SHIFT        0
#define LDO_VUSB11_VBIT_DSLEEP_MASK         (0xF << LDO_VUSB11_VBIT_DSLEEP_SHIFT)
#define LDO_VUSB11_VBIT_DSLEEP(n)           BITFIELD_VAL(LDO_VUSB11_VBIT_DSLEEP, n)

// REG_2D
#define R_PWM2_TOGGLE_SHIFT                 0
#define R_PWM2_TOGGLE_MASK                  (0xFFFF << R_PWM2_TOGGLE_SHIFT)
#define R_PWM2_TOGGLE(n)                    BITFIELD_VAL(R_PWM2_TOGGLE, n)

// REG_2E
#define REG_PWM2_ST1_SHIFT                  0
#define REG_PWM2_ST1_MASK                   (0xFFFF << REG_PWM2_ST1_SHIFT)
#define REG_PWM2_ST1(n)                     BITFIELD_VAL(REG_PWM2_ST1, n)

// REG_2F
#define SUBCNT_DATA2_SHIFT                  8
#define SUBCNT_DATA2_MASK                   (0xFF << SUBCNT_DATA2_SHIFT)
#define SUBCNT_DATA2(n)                     BITFIELD_VAL(SUBCNT_DATA2, n)
#define TG_SUBCNT_D2_ST_SHIFT               1
#define TG_SUBCNT_D2_ST_MASK                (0x7F << TG_SUBCNT_D2_ST_SHIFT)
#define TG_SUBCNT_D2_ST(n)                  BITFIELD_VAL(TG_SUBCNT_D2_ST, n)
#define REG_LED0_OUT                        (1 << 0)

// REG_30
#define RESERVED_ANA_29                     (1 << 15)
#define RESETN_DB_NUMBER_SHIFT              11
#define RESETN_DB_NUMBER_MASK               (0xF << RESETN_DB_NUMBER_SHIFT)
#define RESETN_DB_NUMBER(n)                 BITFIELD_VAL(RESETN_DB_NUMBER, n)
#define RESETN_DB_EN                        (1 << 10)
#define REG_PWM_CLK_EN                      (1 << 9)
#define REG_CLK_PWM_DIV_SHIFT               4
#define REG_CLK_PWM_DIV_MASK                (0x1F << REG_CLK_PWM_DIV_SHIFT)
#define REG_CLK_PWM_DIV(n)                  BITFIELD_VAL(REG_CLK_PWM_DIV, n)
#define REG_PWM2_BR_EN                      (1 << 3)
#define PWM_SELECT_EN                       (1 << 2)
#define PWM_SELECT_INV                      (1 << 1)
#define LED_GPIO_SELECT                     (1 << 0)

// REG_34
#define RESERVED_ANA_30                     (1 << 15)
#define CLK_32K_COUNT_NUM_SHIFT             11
#define CLK_32K_COUNT_NUM_MASK              (0xF << CLK_32K_COUNT_NUM_SHIFT)
#define CLK_32K_COUNT_NUM(n)                BITFIELD_VAL(CLK_32K_COUNT_NUM, n)
#define REG_IPTAT_CORE_I_SEL_SHIFT          5
#define REG_IPTAT_CORE_I_SEL_MASK           (0x3F << REG_IPTAT_CORE_I_SEL_SHIFT)
#define REG_IPTAT_CORE_I_SEL(n)             BITFIELD_VAL(REG_IPTAT_CORE_I_SEL, n)
#define DIG_IPTAT_CORE_EN                   (1 << 4)
#define REG_EFUSE_WRITE_AUTO_MODE           (1 << 2)
#define REG_EFUSE_TRIGGER_WRITE             (1 << 1)
#define REG_EFUSE_TRIGGER_READ              (1 << 0)

// REG_35
#define REG_DIV_HW_RESET_SHIFT              0
#define REG_DIV_HW_RESET_MASK               (0xFFFF << REG_DIV_HW_RESET_SHIFT)
#define REG_DIV_HW_RESET(n)                 BITFIELD_VAL(REG_DIV_HW_RESET, n)

// REG_36
#define REG_WDT_TIMER_SHIFT                 0
#define REG_WDT_TIMER_MASK                  (0xFFFF << REG_WDT_TIMER_SHIFT)
#define REG_WDT_TIMER(n)                    BITFIELD_VAL(REG_WDT_TIMER, n)

// REG_37
#define RESERVED_ANA_33_31_SHIFT            13
#define RESERVED_ANA_33_31_MASK             (0x7 << RESERVED_ANA_33_31_SHIFT)
#define RESERVED_ANA_33_31(n)               BITFIELD_VAL(RESERVED_ANA_33_31, n)

#define CLK_32K_COUNT_EN_TRIGGER            (1 << 12)
#define CLK_32K_COUNT_CLOCK_EN              (1 << 11)
#define POWERON_DETECT_EN                   (1 << 10)
#define MERGE_INTR                          (1 << 9)
#define REG_WDT_EN                          (1 << 8)
#define REG_WDT_RESET_EN                    (1 << 7)
#define REG_HW_RESET_TIME_SHIFT             1
#define REG_HW_RESET_TIME_MASK              (0x3F << REG_HW_RESET_TIME_SHIFT)
#define REG_HW_RESET_TIME(n)                BITFIELD_VAL(REG_HW_RESET_TIME, n)
#define REG_HW_RESET_EN                     (1 << 0)

// REG_38
#define RESERVED_ANA_EN                     (1 << 15)
#define RESERVED_ANA_15_0_SHIFT             0
#define RESERVED_ANA_15_0_MASK              (0xFFFF << RESERVED_ANA_15_0_SHIFT)
#define RESERVED_ANA_15_0(n)                BITFIELD_VAL(RESERVED_ANA_15_0, n)

// REG_39
#define RESERVED_DIG_15_0_SHIFT             0
#define RESERVED_DIG_15_0_MASK              (0xFFFF << RESERVED_DIG_15_0_SHIFT)
#define RESERVED_DIG_15_0(n)                BITFIELD_VAL(RESERVED_DIG_15_0, n)

// REG_3A
#define R_PWMB_TOGGLE_SHIFT                 0
#define R_PWMB_TOGGLE_MASK                  (0xFFFF << R_PWMB_TOGGLE_SHIFT)
#define R_PWMB_TOGGLE(n)                    BITFIELD_VAL(R_PWMB_TOGGLE, n)

// REG_3B
#define REG_PWMB_ST1_SHIFT                  0
#define REG_PWMB_ST1_MASK                   (0xFFFF << REG_PWMB_ST1_SHIFT)
#define REG_PWMB_ST1(n)                     BITFIELD_VAL(REG_PWMB_ST1, n)

// REG_3C
#define SUBCNT_DATAB_SHIFT                  8
#define SUBCNT_DATAB_MASK                   (0xFF << SUBCNT_DATAB_SHIFT)
#define SUBCNT_DATAB(n)                     BITFIELD_VAL(SUBCNT_DATAB, n)
#define TG_SUBCNT_DB_ST_SHIFT               1
#define TG_SUBCNT_DB_ST_MASK                (0x7F << TG_SUBCNT_DB_ST_SHIFT)
#define TG_SUBCNT_DB_ST(n)                  BITFIELD_VAL(TG_SUBCNT_DB_ST, n)
#define REG_LEDB_OUT                        (1 << 0)

// REG_3D
#define RESERVED_PWMB_SHIFT                 10
#define RESERVED_PWMB_MASK                  (0x3F << RESERVED_PWMB_SHIFT)
#define RESERVED_PWMB(n)                    BITFIELD_VAL(RESERVED_PWMB, n)
#define REG_PWMB_CLK_EN                     (1 << 9)
#define REG_CLK_PWMB_DIV_SHIFT              4
#define REG_CLK_PWMB_DIV_MASK               (0x1F << REG_CLK_PWMB_DIV_SHIFT)
#define REG_CLK_PWMB_DIV(n)                 BITFIELD_VAL(REG_CLK_PWMB_DIV, n)
#define REG_PWMB_BR_EN                      (1 << 3)
#define PWMB_SELECT_EN                      (1 << 2)
#define PWMB_SELECT_INV                     (1 << 1)
#define LEDB_GPIO_SELECT                    (1 << 0)

// REG_3E
#define REG_LED_IO2_IBIT_SHIFT              14
#define REG_LED_IO2_IBIT_MASK               (0x3 << REG_LED_IO2_IBIT_SHIFT)
#define REG_LED_IO2_IBIT(n)                 BITFIELD_VAL(REG_LED_IO2_IBIT, n)
#define REG_LED_IO2_OENB_PRE                (1 << 13)
#define REG_LED_IO2_PDEN                    (1 << 12)
#define REG_LED_IO2_PU                      (1 << 11)
#define REG_LED_IO2_PUEN                    (1 << 10)
#define REG_LED_IO2_SEL_SHIFT               8
#define REG_LED_IO2_SEL_MASK                (0x3 << REG_LED_IO2_SEL_SHIFT)
#define REG_LED_IO2_SEL(n)                  BITFIELD_VAL(REG_LED_IO2_SEL, n)
#define REG_LED_IO2_RX_EN                   (1 << 7)
#define REG_LED_IO2_AIO_EN                  (1 << 6)
#define RESERVED_3E_SHIFT                   0
#define RESERVED_3E_MASK                    (0x3F << RESERVED_3E_SHIFT)
#define RESERVED_3E(n)                      BITFIELD_VAL(RESERVED_3E, n)

// REG_3F
#define AC_OUT_LDO_ON_EN                    (1 << 15)
#define USB_FS_DRV_SEL_SHIFT                12
#define USB_FS_DRV_SEL_MASK                 (0x7 << USB_FS_DRV_SEL_SHIFT)
#define USB_FS_DRV_SEL(n)                   BITFIELD_VAL(USB_FS_DRV_SEL, n)
#define USB_FS_CDR_BYPASS_REG               (1 << 11)
#define USB_FS_RPU_SEL_REG_SHIFT            7
#define USB_FS_RPU_SEL_REG_MASK             (0xF << USB_FS_RPU_SEL_REG_SHIFT)
#define USB_FS_RPU_SEL_REG(n)               BITFIELD_VAL(USB_FS_RPU_SEL_REG, n)
#define USB_FS_RPD_SEL_REG_SHIFT            3
#define USB_FS_RPD_SEL_REG_MASK             (0xF << USB_FS_RPD_SEL_REG_SHIFT)
#define USB_FS_RPD_SEL_REG(n)               BITFIELD_VAL(USB_FS_RPD_SEL_REG, n)
#define USB_HOST_MODE                       (1 << 2)
#define CFG_LOW_SPEED_MODE                  (1 << 1)
#define CFG_ANAPHY_RESETN                   (1 << 0)

// REG_40
#define RESERVED_40                         (1 << 15)
#define RTC_INTR_TMP_MERGED_MSK             (1 << 14)
#define GPADC_INTR_MERGED_MSK               (1 << 13)
#define CHARGE_INTR_MERGED_MSK              (1 << 12)
#define USB_PU_AVDD33                       (1 << 11)
#define USB_PU_AVDD11                       (1 << 10)
#define USB_LOOP_BACK_REG                   (1 << 9)
#define USB_HYS_EN_IN_REG                   (1 << 8)
#define USB_BYP_SUSPENDM_REG                (1 << 7)
#define USB_DISCON_DET_EN                   (1 << 6)
#define USB_FS_LS_SEL_IN                    (1 << 5)
#define USB_FS_EDGE_SEL_IN_REG              (1 << 4)
#define USB_FS_DRV_SL_REG                   (1 << 3)
#define USB_DISCON_VTHSEL_REG_SHIFT         0
#define USB_DISCON_VTHSEL_REG_MASK          (0x7 << USB_DISCON_VTHSEL_REG_SHIFT)
#define USB_DISCON_VTHSEL_REG(n)            BITFIELD_VAL(USB_DISCON_VTHSEL_REG, n)

// REG_41
#define USB_INTR_MERGED_MSK                 (1 << 15)
#define POWER_ON_INTR_MERGED_MSK            (1 << 14)
#define PMU_GPIO_INTR_MSKED1_MERGED_MSK     (1 << 13)
#define PMU_GPIO_INTR_MSKED2_MERGED_MSK     (1 << 12)
#define WDT_INTR_MSKED_MERGED_MSK           (1 << 11)
#define USB_ATEST_SELX_DISCON               (1 << 10)
#define USB_DTEST_SEL_REG_SHIFT             8
#define USB_DTEST_SEL_REG_MASK              (0x3 << USB_DTEST_SEL_REG_SHIFT)
#define USB_DTEST_SEL_REG(n)                BITFIELD_VAL(USB_DTEST_SEL_REG, n)
#define USB_DTEST_SEL_TX_REG_SHIFT          6
#define USB_DTEST_SEL_TX_REG_MASK           (0x3 << USB_DTEST_SEL_TX_REG_SHIFT)
#define USB_DTEST_SEL_TX_REG(n)             BITFIELD_VAL(USB_DTEST_SEL_TX_REG, n)
#define USB_DTESTEN_TX_REG                  (1 << 5)
#define USB_DTEST_SEL_FS_IN_REG_SHIFT       3
#define USB_DTEST_SEL_FS_IN_REG_MASK        (0x3 << USB_DTEST_SEL_FS_IN_REG_SHIFT)
#define USB_DTEST_SEL_FS_IN_REG(n)          BITFIELD_VAL(USB_DTEST_SEL_FS_IN_REG, n)
#define USB_DTESTEN2_FS_IN_REG              (1 << 2)
#define USB_DTESTEN1_FS_IN_REG              (1 << 1)
#define USB_ATEST_EN_DISCON_REG             (1 << 0)

// REG_42
#define RESERVED_42_SHIFT                   10
#define RESERVED_42_MASK                    (0x3F << RESERVED_42_SHIFT)
#define RESERVED_42(n)                      BITFIELD_VAL(RESERVED_42, n)
#define REG_BUCK_CC_CAP_BIT2_SHIFT          8
#define REG_BUCK_CC_CAP_BIT2_MASK           (0x3 << REG_BUCK_CC_CAP_BIT2_SHIFT)
#define REG_BUCK_CC_CAP_BIT2(n)             BITFIELD_VAL(REG_BUCK_CC_CAP_BIT2, n)
#define REG_BUCK_ANA_I_DELTAV_X2            (1 << 7)
#define CFG_DR_FSTXDB                       (1 << 6)
#define CFG_DR_FSTXD                        (1 << 5)
#define CFG_DR_FSTXEN                       (1 << 4)
#define CFG_DR_OPMODE                       (1 << 3)
#define CFG_DR_XCVRSEL                      (1 << 2)
#define CFG_DR_TERMSEL                      (1 << 1)
#define CFG_DR_FSLS_SEL                     (1 << 0)

// REG_43
#define REG_LOW_LEVEL_INTR_SEL1             (1 << 15)
#define REG_WDT_INTR_EN                     (1 << 14)
#define REG_WDT_INTR_MSK                    (1 << 13)
#define LDO_DCDC2ANA_VBIT_DSLEEP_SHIFT      8
#define LDO_DCDC2ANA_VBIT_DSLEEP_MASK       (0x1F << LDO_DCDC2ANA_VBIT_DSLEEP_SHIFT)
#define LDO_DCDC2ANA_VBIT_DSLEEP(n)         BITFIELD_VAL(LDO_DCDC2ANA_VBIT_DSLEEP, n)
#define CFG_REG_FSTXDB                      (1 << 7)
#define CFG_REG_FSTXD                       (1 << 6)
#define CFG_REG_FSTXEN                      (1 << 5)
#define CFG_REG_OPMODE_SHIFT                3
#define CFG_REG_OPMODE_MASK                 (0x3 << CFG_REG_OPMODE_SHIFT)
#define CFG_REG_OPMODE(n)                   BITFIELD_VAL(CFG_REG_OPMODE, n)
#define CFG_REG_XCVRSEL                     (1 << 2)
#define CFG_REG_TERM                        (1 << 1)
#define CFG_REG_FSLS_SEL                    (1 << 0)

// REG_44
#define REG_BUCK_ANA_EDGE_CON_SHIFT         12
#define REG_BUCK_ANA_EDGE_CON_MASK          (0xF << REG_BUCK_ANA_EDGE_CON_SHIFT)
#define REG_BUCK_ANA_EDGE_CON(n)            BITFIELD_VAL(REG_BUCK_ANA_EDGE_CON, n)
#define REG_BUCK_ANA_HALF_BIAS              (1 << 11)
#define REG_BUCK_ANA_LP_ERR                 (1 << 10)
#define REG_32KFRM26M_DIV_SHIFT             6
#define REG_32KFRM26M_DIV_MASK              (0xF << REG_32KFRM26M_DIV_SHIFT)
#define REG_32KFRM26M_DIV(n)                BITFIELD_VAL(REG_32KFRM26M_DIV, n)
#define RESETN_RF_DR                        (1 << 5)
#define RESETN_RF_REG                       (1 << 4)
#define RESETN_MOD1_CNT_SHIFT               0
#define RESETN_MOD1_CNT_MASK                (0xF << RESETN_MOD1_CNT_SHIFT)
#define RESETN_MOD1_CNT(n)                  BITFIELD_VAL(RESETN_MOD1_CNT, n)

// REG_45
#define CLK_32K_SEL_1                       (1 << 15)
#define REG_EDGE_INTR_SEL2                  (1 << 14)
#define REG_POS_INTR_SEL2                   (1 << 13)
#define REG_EDGE_INTR_SEL1                  (1 << 12)
#define REG_POS_INTR_SEL1                   (1 << 11)
#define REG_MIC_BIASC_CHANSEL_SHIFT         9
#define REG_MIC_BIASC_CHANSEL_MASK          (0x3 << REG_MIC_BIASC_CHANSEL_SHIFT)
#define REG_MIC_BIASC_CHANSEL(n)            BITFIELD_VAL(REG_MIC_BIASC_CHANSEL, n)
#define REG_MIC_BIASC_EN                    (1 << 8)
#define REG_MIC_BIASC_ENLPF                 (1 << 7)
#define REG_MIC_BIASC_LPFSEL_SHIFT          5
#define REG_MIC_BIASC_LPFSEL_MASK           (0x3 << REG_MIC_BIASC_LPFSEL_SHIFT)
#define REG_MIC_BIASC_LPFSEL(n)             BITFIELD_VAL(REG_MIC_BIASC_LPFSEL, n)
#define REG_MIC_BIASC_VSEL_SHIFT            0
#define REG_MIC_BIASC_VSEL_MASK             (0x1F << REG_MIC_BIASC_VSEL_SHIFT)
#define REG_MIC_BIASC_VSEL(n)               BITFIELD_VAL(REG_MIC_BIASC_VSEL, n)

// REG_46
#define REG_GPIO_I_SEL                      (1 << 15)
#define PMU_DB_BYPASS1                      (1 << 14)
#define PMU_DB_TARGET1_SHIFT                6
#define PMU_DB_TARGET1_MASK                 (0xFF << PMU_DB_TARGET1_SHIFT)
#define PMU_DB_TARGET1(n)                   BITFIELD_VAL(PMU_DB_TARGET1, n)
#define REG_PMU_UART_DR1                    (1 << 5)
#define REG_PMU_UART_TX1                    (1 << 4)
#define REG_PMU_UART_OENB1                  (1 << 3)
#define REG_UART_LEDA_SEL                   (1 << 2)
#define REG_PMU_GPIO_INTR_MSK1              (1 << 1)
#define REG_PMU_GPIO_INTR_EN1               (1 << 0)

// REG_47
#define REG_LOW_LEVEL_INTR_SEL2             (1 << 15)
#define PMU_DB_BYPASS2                      (1 << 14)
#define PMU_DB_TARGET2_SHIFT                6
#define PMU_DB_TARGET2_MASK                 (0xFF << PMU_DB_TARGET2_SHIFT)
#define PMU_DB_TARGET2(n)                   BITFIELD_VAL(PMU_DB_TARGET2, n)
#define REG_PMU_UART_DR2                    (1 << 5)
#define REG_PMU_UART_TX2                    (1 << 4)
#define REG_PMU_UART_OENB2                  (1 << 3)
#define REG_UART_LEDB_SEL                   (1 << 2)
#define REG_PMU_GPIO_INTR_MSK2              (1 << 1)
#define REG_PMU_GPIO_INTR_EN2               (1 << 0)

// REG_48
#define PMU_GPIO_INTR_MSKED1                (1 << 15)
#define PMU_GPIO_INTR_MSKED2                (1 << 14)
#define LED_IO1_IN_DB                       (1 << 13)
#define LED_IO2_IN_DB                       (1 << 12)
#define WDT_INTR_MSKED                      (1 << 11)
#define POWER_DOWN_INTR_MSKED               (1 << 10)

#define REG_PMU_GPIO_INTR_CLR1              (1 << 15)
#define REG_PMU_GPIO_INTR_CLR2              (1 << 14)
#define REG_WDT_INTR_CLR                    (1 << 13)
#define REG_POWER_DOWN_INTR_CLR             (1 << 10)

// REG_49
#define REG_BUCK_ANA_LP_VCOMP               (1 << 15)
#define REG_BUCK_ANA_BURST_RES              (1 << 14)
#define REG_BUCK_ANA_DELTAV_SHIFT           8
#define REG_BUCK_ANA_DELTAV_MASK            (0x3F << REG_BUCK_ANA_DELTAV_SHIFT)
#define REG_BUCK_ANA_DELTAV(n)              BITFIELD_VAL(REG_BUCK_ANA_DELTAV, n)
#define REG_BUCK_ANA_EN_ZCD_CAL             (1 << 7)
#define REG_BUCK_LP_VCOMP2_SHIFT            5
#define REG_BUCK_LP_VCOMP2_MASK             (0x3 << REG_BUCK_LP_VCOMP2_SHIFT)
#define REG_BUCK_LP_VCOMP2(n)               BITFIELD_VAL(REG_BUCK_LP_VCOMP2, n)
#define REG_BUCK_SLOPE_HALF_ENB             (1 << 4)
#define REG_BUCK_ANA_ZCD_CAP_BIT_SHIFT      0
#define REG_BUCK_ANA_ZCD_CAP_BIT_MASK       (0xF << REG_BUCK_ANA_ZCD_CAP_BIT_SHIFT)
#define REG_BUCK_ANA_ZCD_CAP_BIT(n)         BITFIELD_VAL(REG_BUCK_ANA_ZCD_CAP_BIT, n)

// REG_4A
#define BUCK_VCORE_BIT_NORMAL_SHIFT         8
#define BUCK_VCORE_BIT_NORMAL_MASK          (0xFF << BUCK_VCORE_BIT_NORMAL_SHIFT)
#define BUCK_VCORE_BIT_NORMAL(n)            BITFIELD_VAL(BUCK_VCORE_BIT_NORMAL, n)
#define BUCK_VCORE_BIT_DSLEEP_SHIFT         0
#define BUCK_VCORE_BIT_DSLEEP_MASK          (0xFF << BUCK_VCORE_BIT_DSLEEP_SHIFT)
#define BUCK_VCORE_BIT_DSLEEP(n)            BITFIELD_VAL(BUCK_VCORE_BIT_DSLEEP, n)

// REG_4B
#define DATA_CHAN_MSB_SHIFT                 0
#define DATA_CHAN_MSB_MASK                  (0xFF << DATA_CHAN_MSB_SHIFT)
#define DATA_CHAN_MSB(n)                    BITFIELD_VAL(DATA_CHAN_MSB, n)

// REG_4C
#define REG_VCORE_ON_DELAY_DR               (1 << 15)
#define REG_VCORE_ON_DELAY                  (1 << 14)
#define LDO_DCDC2ANA_VBIT_NORMAL_SHIFT      9
#define LDO_DCDC2ANA_VBIT_NORMAL_MASK       (0x1F << LDO_DCDC2ANA_VBIT_NORMAL_SHIFT)
#define LDO_DCDC2ANA_VBIT_NORMAL(n)         BITFIELD_VAL(LDO_DCDC2ANA_VBIT_NORMAL, n)
#define REG_PU_LDO_VDCDC2VANA_DSLEEP        (1 << 8)
#define REG_PU_LDO_VDCDC2VANA_DR            (1 << 7)
#define REG_PU_LDO_VDCDC2VANA               (1 << 6)
#define LP_EN_VDCDC2VANA_LDO_DSLEEP         (1 << 5)
#define LP_EN_VDCDC2VANA_LDO_DR             (1 << 4)
#define LP_EN_VDCDC2VANA_LDO_REG            (1 << 3)
#define REG_PULLDOWN_VDCDC2VANA             (1 << 2)
#define REG_BYPASS_VDCDC2VANA               (1 << 1)
#define REG_LIGHT_LOAD_VDCDC2VANA           (1 << 0)

// REG_4D
#define REG_WDT_TIMER_INTR_SHIFT            0
#define REG_WDT_TIMER_INTR_MASK             (0xFFFF << REG_WDT_TIMER_INTR_SHIFT)
#define REG_WDT_TIMER_INTR(n)               BITFIELD_VAL(REG_WDT_TIMER_INTR, n)

// REG_4E
#define CLK_32K_COUNTER_26M_SHIFT           0
#define CLK_32K_COUNTER_26M_MASK            (0x7FFF << CLK_32K_COUNTER_26M_SHIFT)
#define CLK_32K_COUNTER_26M(n)              BITFIELD_VAL(CLK_32K_COUNTER_26M, n)
#define CLK_32K_COUNTER_26M_READY           (1 << 15)

// REG_4F
#define GPADC_START                         (1 << 5)
#define KEY_START                           (1 << 4)
#define AC_ON_EN                            (1 << 2)
#define HARDWARE_POWER_OFF_EN               (1 << 1)
#define SOFT_POWER_OFF                      (1 << 0)

// REG_50
#define USBINSERT_INTR2CPU                  (1 << 15)
#define RTC_INT_1                           (1 << 14)
#define RTC_INT_0                           (1 << 13)
#define KEY_ERR1_INTR                       (1 << 12)
#define KEY_ERR0_INTR                       (1 << 11)
#define KEY_PRESS_INTR                      (1 << 10)
#define KEY_RELEASE_INTR                    (1 << 9)
#define SAMPLE_PERIOD_DONE_INTR             (1 << 8)
#define CHAN_DATA_VALID_INTR_SHIFT          0
#define CHAN_DATA_VALID_INTR_MASK           (0xFF << CHAN_DATA_VALID_INTR_SHIFT)
#define CHAN_DATA_VALID_INTR(n)             BITFIELD_VAL(CHAN_DATA_VALID_INTR, n)

// REG_51
#define USB_INSERT_INTR_MSKED               (1 << 15)
#define RTC_INT1_MSKED                      (1 << 14)
#define RTC_INT0_MSKED                      (1 << 13)
#define KEY_ERR1_INTR_MSKED                 (1 << 12)
#define KEY_ERR0_INTR_MSKED                 (1 << 11)
#define KEY_PRESS_INTR_MSKED                (1 << 10)
#define KEY_RELEASE_INTR_MSKED              (1 << 9)
#define SAMPLE_DONE_INTR_MSKED              (1 << 8)
#define CHAN_DATA_INTR_MSKED_SHIFT          0
#define CHAN_DATA_INTR_MSKED_MASK           (0xFF << CHAN_DATA_INTR_MSKED_SHIFT)
#define CHAN_DATA_INTR_MSKED(n)             BITFIELD_VAL(CHAN_DATA_INTR_MSKED, n)

#define USB_INTR_CLR                        (1 << 15)
#define RTC_INT_CLR_1                       (1 << 14)
#define RTC_INT_CLR_0                       (1 << 13)
#define KEY_ERR1_INTR_CLR                   (1 << 12)
#define KEY_ERR0_INTR_CLR                   (1 << 11)
#define KEY_PRESS_INTR_CLR                  (1 << 10)
#define KEY_RELEASE_INTR_CLR                (1 << 9)
#define SAMPLE_DONE_INTR_CLR                (1 << 8)
#define CHAN_DATA_INTR_CLR_SHIFT            0
#define CHAN_DATA_INTR_CLR_MASK             (0xFF << CHAN_DATA_INTR_CLR_SHIFT)
#define CHAN_DATA_INTR_CLR(n)               BITFIELD_VAL(CHAN_DATA_INTR_CLR, n)

// REG_52
#define WDT_LOAD_INTR                       (1 << 9)
#define PMU_GPIO_INTR2                      (1 << 8)
#define PMU_GPIO_INTR1                      (1 << 7)
#define R_WDT_INTR                          (1 << 6)
#define USB_DTESTO_TX                       (1 << 5)
#define VCORE_ON_DELAY                      (1 << 4)
#define RTC_LOAD                            (1 << 3)
#define WDT_LOAD                            (1 << 2)
#define CORE_GPIO_OUT1                      (1 << 1)
#define POWER_ON                            (1 << 0)

// REG_54
#define RTC_VALUE_15_0_SHIFT                0
#define RTC_VALUE_15_0_MASK                 (0xFFFF << RTC_VALUE_15_0_SHIFT)
#define RTC_VALUE_15_0(n)                   BITFIELD_VAL(RTC_VALUE_15_0, n)

// REG_55
#define RTC_VALUE_31_16_SHIFT               0
#define RTC_VALUE_31_16_MASK                (0xFFFF << RTC_VALUE_31_16_SHIFT)
#define RTC_VALUE_31_16(n)                  BITFIELD_VAL(RTC_VALUE_31_16, n)

// REG_56
#define DATA_CHAN0_SHIFT                    0
#define DATA_CHAN0_MASK                     (0xFFFF << DATA_CHAN0_SHIFT)
#define DATA_CHAN0(n)                       BITFIELD_VAL(DATA_CHAN0, n)

// REG_57
#define DATA_CHAN1_SHIFT                    0
#define DATA_CHAN1_MASK                     (0xFFFF << DATA_CHAN1_SHIFT)
#define DATA_CHAN1(n)                       BITFIELD_VAL(DATA_CHAN1, n)

// REG_58
#define DATA_CHAN2_SHIFT                    0
#define DATA_CHAN2_MASK                     (0xFFFF << DATA_CHAN2_SHIFT)
#define DATA_CHAN2(n)                       BITFIELD_VAL(DATA_CHAN2, n)

// REG_59
#define DATA_CHAN3_SHIFT                    0
#define DATA_CHAN3_MASK                     (0xFFFF << DATA_CHAN3_SHIFT)
#define DATA_CHAN3(n)                       BITFIELD_VAL(DATA_CHAN3, n)

// REG_5A
#define DATA_CHAN4_SHIFT                    0
#define DATA_CHAN4_MASK                     (0xFFFF << DATA_CHAN4_SHIFT)
#define DATA_CHAN4(n)                       BITFIELD_VAL(DATA_CHAN4, n)

// REG_5B
#define DATA_CHAN5_SHIFT                    0
#define DATA_CHAN5_MASK                     (0xFFFF << DATA_CHAN5_SHIFT)
#define DATA_CHAN5(n)                       BITFIELD_VAL(DATA_CHAN5, n)

// REG_5C
#define DATA_CHAN6_SHIFT                    0
#define DATA_CHAN6_MASK                     (0xFFFF << DATA_CHAN6_SHIFT)
#define DATA_CHAN6(n)                       BITFIELD_VAL(DATA_CHAN6, n)

// REG_5D
#define DATA_CHAN7_SHIFT                    0
#define DATA_CHAN7_MASK                     (0xFFFF << DATA_CHAN7_SHIFT)
#define DATA_CHAN7(n)                       BITFIELD_VAL(DATA_CHAN7, n)

// REG_5E
#define POWER_ON_RELEASE                    (1 << 15)
#define POWER_ON_PRESS                      (1 << 14)
#define DIG_LP_EN_VCODEC                    (1 << 11)
#define DIG_PU_VCODEC                       (1 << 10)
#define DIG_LP_EN_VPA                       (1 << 9)
#define DIG_PU_VPA                          (1 << 8)
#define DIG_PU_VRTC_RF                      (1 << 7)
#define DIG_PU_BIAS_LDO                     (1 << 6)
#define DIG_PU_LP_BIAS_LDO                  (1 << 5)
#define INTR_MSKED_CHARGE_SHIFT             3
#define INTR_MSKED_CHARGE_MASK              (0x3 << INTR_MSKED_CHARGE_SHIFT)
#define INTR_MSKED_CHARGE(n)                BITFIELD_VAL(INTR_MSKED_CHARGE, n)
#define AC_ON                               (1 << 2)
#define AC_ON_DET_OUT                       (1 << 1)
#define AC_ON_DET_IN                        (1 << 0)

// REG_5F
#define RC_SLEEP_MODE                       (1 << 15)
#define DIG_PU_SAR                          (1 << 14)
#define DIG_PU_DCDC_VCORE                   (1 << 13)
#define DIG_PU_DCDC_VANA                    (1 << 12)
#define DIG_PU_DCDC_VCODEC                  (1 << 11)
#define DIG_LP_EN_VCORE_LDO                 (1 << 10)
#define DIG_PU_VCORE_LDO                    (1 << 9)
#define DIG_LP_EN_VDCDC                     (1 << 8)
#define DIG_PU_VDCDC_LDO                    (1 << 7)
#define DEEPSLEEP_MODE                      (1 << 6)
#define UVLO_LV                             (1 << 5)
#define DIG_PU_LPO                          (1 << 4)
#define PMU_LDO_ON_SOURCE_SHIFT             1
#define PMU_LDO_ON_SOURCE_MASK              (0x7 << PMU_LDO_ON_SOURCE_SHIFT)
#define PMU_LDO_ON_SOURCE(n)                BITFIELD_VAL(PMU_LDO_ON_SOURCE, n)
#define PMU_LDO_ON_                         (1 << 0)

// REG_60
#define REG_WD_RESETN_INFOR_SHIFT           0
#define REG_WD_RESETN_INFOR_MASK            (0xFFFF << REG_WD_RESETN_INFOR_SHIFT)
#define REG_WD_RESETN_INFOR(n)              BITFIELD_VAL(REG_WD_RESETN_INFOR, n)

// REG_61
#define REG_COMMAND_RST_DIS                 (1 << 15)
#define REG_NOT_RESET_SHIFT                 2
#define REG_NOT_RESET_MASK                  (0x1FFF << REG_NOT_RESET_SHIFT)
#define REG_NOT_RESET(n)                    BITFIELD_VAL(REG_NOT_RESET, n)
#define REG_NOT_RESET_BIT1                  (1 << 1)
#define REG_NOT_RESET_BIT0                  (1 << 0)

#define REG_NOT_RESET_RTC_ENABLED           REG_NOT_RESET_BIT1
#define REG_NOT_RESET_CHIP_PWR_ON           REG_NOT_RESET_BIT0

// REG_62
#define RESETN_A7PLL_REG                    (1 << 11)
#define RESETN_A7PLL_DR                     (1 << 10)
#define RESETN_PSRAM1_REG                   (1 << 9)
#define RESETN_PSRAM1_DR                    (1 << 8)
#define RESETN_PSRAM0_REG                   (1 << 7)
#define RESETN_PSRAM0_DR                    (1 << 6)
#define RESETN_PSRAMPLL_REG                 (1 << 5)
#define RESETN_PSRAMPLL_DR                  (1 << 4)
#define RESETN_BT_REG                       (1 << 3)
#define RESETN_BT_DR                        (1 << 2)
#define RESETN_WIFI_REG                     (1 << 1)
#define RESETN_WIFI_DR                      (1 << 0)

// REG_63
#define REG_BUCK_PRIOR_CHANNEL_SHIFT        14
#define REG_BUCK_PRIOR_CHANNEL_MASK         (0x3 << REG_BUCK_PRIOR_CHANNEL_SHIFT)
#define REG_BUCK_PRIOR_CHANNEL(n)           BITFIELD_VAL(REG_BUCK_PRIOR_CHANNEL, n)
#define REG_BUCK_VCORE_IS_GAIN_DSLEEP_SHIFT 11
#define REG_BUCK_VCORE_IS_GAIN_DSLEEP_MASK  (0x7 << REG_BUCK_VCORE_IS_GAIN_DSLEEP_SHIFT)
#define REG_BUCK_VCORE_IS_GAIN_DSLEEP(n)    BITFIELD_VAL(REG_BUCK_VCORE_IS_GAIN_DSLEEP, n)
#define REG_BUCK_VCORE_IS_GAIN_NORMAL_SHIFT 8
#define REG_BUCK_VCORE_IS_GAIN_NORMAL_MASK  (0x7 << REG_BUCK_VCORE_IS_GAIN_NORMAL_SHIFT)
#define REG_BUCK_VCORE_IS_GAIN_NORMAL(n)    BITFIELD_VAL(REG_BUCK_VCORE_IS_GAIN_NORMAL, n)
#define REG_BUCK_ANA_BURST_MODE_DSLEEP      (1 << 6)
#define REG_VCORE_ANA_I_DELTAV_X2           (1 << 5)
#define REG_VCORE_ANA_REV_BACK              (1 << 4)
#define REG_BUCK_ANA_SENSE_SEL              (1 << 3)
#define REG_BUCK_ANA_BURST_MODE_NORMAL      (1 << 2)
#define REG_BUCK_ANA_VOUT_SEL               (1 << 1)
#define REG_BUCK_ANA_REV_BACK               (1 << 0)

// REG_64
#define REG_BUCK_VCORE_INTERNAL_FREQUENCY_SHIFT 10
#define REG_BUCK_VCORE_INTERNAL_FREQUENCY_MASK (0x7 << REG_BUCK_VCORE_INTERNAL_FREQUENCY_SHIFT)
#define REG_BUCK_VCORE_INTERNAL_FREQUENCY(n) BITFIELD_VAL(REG_BUCK_VCORE_INTERNAL_FREQUENCY, n)
#define REG_BUCK_VCORE_LOW_VOLTAGE          (1 << 9)
#define REG_BUCK_VCORE_DT_BIT               (1 << 8)
#define REG_BUCK_VCORE_PULLDOWN_EN          (1 << 7)
#define REG_BUCK_VCORE_ANTI_RES_DISABLE     (1 << 6)
#define REG_BUCK_VCORE_SYNC_DISABLE         (1 << 3)
#define REG_BUCK_VCORE_SOFT_START_EN        (1 << 2)
#define REG_BUCK_VCORE_RECOVER              (1 << 1)
#define REG_BUCK_VCORE_SLOPE_DOUBLE         (1 << 0)

// REG_65
#define REG_BUCK_VCORE_BURST_MODE_DSLEEP    (1 << 15)
#define REG_BUCK_VCORE_I_DELTAV_X2          (1 << 14)
#define REG_BUCK_VCORE_DELTAV_SHIFT         8
#define REG_BUCK_VCORE_DELTAV_MASK          (0x3F << REG_BUCK_VCORE_DELTAV_SHIFT)
#define REG_BUCK_VCORE_DELTAV(n)            BITFIELD_VAL(REG_BUCK_VCORE_DELTAV, n)
#define REG_BUCK_VCORE_ZCD_CAP_BIT_SHIFT    4
#define REG_BUCK_VCORE_ZCD_CAP_BIT_MASK     (0xF << REG_BUCK_VCORE_ZCD_CAP_BIT_SHIFT)
#define REG_BUCK_VCORE_ZCD_CAP_BIT(n)       BITFIELD_VAL(REG_BUCK_VCORE_ZCD_CAP_BIT, n)
#define REG_BUCK_VCORE_EN_ZCD_CAL           (1 << 3)
#define REG_BUCK_VCORE_SENSE_SEL            (1 << 2)
#define REG_BUCK_VCORE_BURST_MODE_NORMAL    (1 << 1)
#define REG_BUCK_VCORE_VOUT_SEL             (1 << 0)

// REG_66
#define REG_COMMAND_RST_BIT_WIDTH_TOLERANCE_SHIFT 8
#define REG_COMMAND_RST_BIT_WIDTH_TOLERANCE_MASK (0xFF << REG_COMMAND_RST_BIT_WIDTH_TOLERANCE_SHIFT)
#define REG_COMMAND_RST_BIT_WIDTH_TOLERANCE(n) BITFIELD_VAL(REG_COMMAND_RST_BIT_WIDTH_TOLERANCE, n)
#define REG_BUCK_VCORE_LP_ERR               (1 << 6)
#define REG_BUCK_VCORE_LP_VCOMP             (1 << 5)
#define REG_BUCK_VCORE_HALF_BIAS            (1 << 4)
#define REG_BUCK_VCORE_EDGE_CON_SHIFT       0
#define REG_BUCK_VCORE_EDGE_CON_MASK        (0xF << REG_BUCK_VCORE_EDGE_CON_SHIFT)
#define REG_BUCK_VCORE_EDGE_CON(n)          BITFIELD_VAL(REG_BUCK_VCORE_EDGE_CON, n)

// REG_67
#define REG_VSENSE_SEL_VMEM                 (1 << 10)
#define REG_PULLDOWN_VMEM                   (1 << 9)
#define REG_MIC_BIASB_PULLDOWN              (1 << 2)
#define REG_MIC_BIASA_PULLDOWN              (1 << 1)
#define REG_MIC_LP_ENABLE                   (1 << 0)

// REG_68
#define LP_EN_VMEM_LDO_DSLEEP               (1 << 15)
#define LP_EN_VMEM_LDO_DR                   (1 << 14)
#define LP_EN_VMEM_LDO_REG                  (1 << 13)
#define REG_PU_LDO_VMEM_DSLEEP              (1 << 12)
#define REG_PU_LDO_VMEM_DR                  (1 << 11)
#define REG_PU_LDO_VMEM_REG                 (1 << 10)
#define LDO_VMEM_VBIT_NORMAL_SHIFT          5
#define LDO_VMEM_VBIT_NORMAL_MASK           (0x1F << LDO_VMEM_VBIT_NORMAL_SHIFT)
#define LDO_VMEM_VBIT_NORMAL(n)             BITFIELD_VAL(LDO_VMEM_VBIT_NORMAL, n)
#define LDO_VMEM_VBIT_DSLEEP_SHIFT          0
#define LDO_VMEM_VBIT_DSLEEP_MASK           (0x1F << LDO_VMEM_VBIT_DSLEEP_SHIFT)
#define LDO_VMEM_VBIT_DSLEEP(n)             BITFIELD_VAL(LDO_VMEM_VBIT_DSLEEP, n)

// REG_69
#define LP_EN_VPA_LDO_DSLEEP                (1 << 15)
#define LP_EN_VPA_LDO_DR                    (1 << 14)
#define LP_EN_VPA_LDO_REG                   (1 << 13)
#define REG_PU_LDO_VPA_DSLEEP               (1 << 12)
#define REG_PU_LDO_VPA_DR                   (1 << 11)
#define REG_PU_LDO_VPA_REG                  (1 << 10)
#define LDO_VPA_VBIT_NORMAL_SHIFT           5
#define LDO_VPA_VBIT_NORMAL_MASK            (0x1F << LDO_VPA_VBIT_NORMAL_SHIFT)
#define LDO_VPA_VBIT_NORMAL(n)              BITFIELD_VAL(LDO_VPA_VBIT_NORMAL, n)
#define LDO_VPA_VBIT_DSLEEP_SHIFT           0
#define LDO_VPA_VBIT_DSLEEP_MASK            (0x1F << LDO_VPA_VBIT_DSLEEP_SHIFT)
#define LDO_VPA_VBIT_DSLEEP(n)              BITFIELD_VAL(LDO_VPA_VBIT_DSLEEP, n)

// REG_6A
#define REG_TRAN_ENHANCE_VANA_DR            (1 << 11)
#define REG_TRAN_ENHANCE_VANA               (1 << 10)
#define REG_TRAN_ENHANCE_VCODEC_DR          (1 << 9)
#define REG_TRAN_ENHANCE_VCODEC             (1 << 8)
#define REG_BIAS_SEL_LDO                    (1 << 7)
#define LP_MODE_RTC_DR                      (1 << 6)
#define LP_MODE_RTC_REG                     (1 << 5)
#define REG_BUCK_VANA_RAMP_EN               (1 << 4)
#define REG_BUCK_VCORE_RAMP_EN              (1 << 3)
#define REG_BUCK_VCODEC_RAMP_EN             (1 << 2)
#define REG_OCP_VPA                         (1 << 1)
#define REG_PULLDOWN_VPA                    (1 << 0)

// REG_6B
#define POWER_UP_MOD8_CNT_SHIFT             8
#define POWER_UP_MOD8_CNT_MASK              (0xFF << POWER_UP_MOD8_CNT_SHIFT)
#define POWER_UP_MOD8_CNT(n)                BITFIELD_VAL(POWER_UP_MOD8_CNT, n)
#define POWER_UP_MOD7_CNT_SHIFT             0
#define POWER_UP_MOD7_CNT_MASK              (0xFF << POWER_UP_MOD7_CNT_SHIFT)
#define POWER_UP_MOD7_CNT(n)                BITFIELD_VAL(POWER_UP_MOD7_CNT, n)

// REG_6C
#define POWER_UP_MOD9_CNT_SHIFT             0
#define POWER_UP_MOD9_CNT_MASK              (0xFF << POWER_UP_MOD9_CNT_SHIFT)
#define POWER_UP_MOD9_CNT(n)                BITFIELD_VAL(POWER_UP_MOD9_CNT, n)

// REG_6D
#define REG_SAR_RESULT_SEL                  (1 << 15)
#define REG_SAR_ADC_ON                      (1 << 14)
#define SAR_BIT00_WEIGHT_SHIFT              0
#define SAR_BIT00_WEIGHT_MASK               (0x3FFF << SAR_BIT00_WEIGHT_SHIFT)
#define SAR_BIT00_WEIGHT(n)                 BITFIELD_VAL(SAR_BIT00_WEIGHT, n)

// REG_6E
#define SAR_BIT01_WEIGHT_SHIFT              0
#define SAR_BIT01_WEIGHT_MASK               (0x3FFF << SAR_BIT01_WEIGHT_SHIFT)
#define SAR_BIT01_WEIGHT(n)                 BITFIELD_VAL(SAR_BIT01_WEIGHT, n)

// REG_6F
#define SAR_BIT02_WEIGHT_SHIFT              0
#define SAR_BIT02_WEIGHT_MASK               (0x3FFF << SAR_BIT02_WEIGHT_SHIFT)
#define SAR_BIT02_WEIGHT(n)                 BITFIELD_VAL(SAR_BIT02_WEIGHT, n)

// REG_70
#define SAR_BIT03_WEIGHT_SHIFT              0
#define SAR_BIT03_WEIGHT_MASK               (0x3FFF << SAR_BIT03_WEIGHT_SHIFT)
#define SAR_BIT03_WEIGHT(n)                 BITFIELD_VAL(SAR_BIT03_WEIGHT, n)

// REG_71
#define SAR_BIT04_WEIGHT_SHIFT              0
#define SAR_BIT04_WEIGHT_MASK               (0x3FFF << SAR_BIT04_WEIGHT_SHIFT)
#define SAR_BIT04_WEIGHT(n)                 BITFIELD_VAL(SAR_BIT04_WEIGHT, n)

// REG_72
#define SAR_P_BIT05_WEIGHT_SHIFT            0
#define SAR_P_BIT05_WEIGHT_MASK             (0x3FFF << SAR_P_BIT05_WEIGHT_SHIFT)
#define SAR_P_BIT05_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT05_WEIGHT, n)

// REG_73
#define SAR_P_BIT06_WEIGHT_SHIFT            0
#define SAR_P_BIT06_WEIGHT_MASK             (0x3FFF << SAR_P_BIT06_WEIGHT_SHIFT)
#define SAR_P_BIT06_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT06_WEIGHT, n)

// REG_74
#define SAR_P_BIT07_WEIGHT_SHIFT            0
#define SAR_P_BIT07_WEIGHT_MASK             (0x3FFF << SAR_P_BIT07_WEIGHT_SHIFT)
#define SAR_P_BIT07_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT07_WEIGHT, n)

// REG_75
#define SAR_P_BIT08_WEIGHT_SHIFT            0
#define SAR_P_BIT08_WEIGHT_MASK             (0x3FFF << SAR_P_BIT08_WEIGHT_SHIFT)
#define SAR_P_BIT08_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT08_WEIGHT, n)

// REG_76
#define SAR_P_BIT09_WEIGHT_SHIFT            0
#define SAR_P_BIT09_WEIGHT_MASK             (0x3FFF << SAR_P_BIT09_WEIGHT_SHIFT)
#define SAR_P_BIT09_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT09_WEIGHT, n)

// REG_77
#define SAR_P_BIT10_WEIGHT_SHIFT            0
#define SAR_P_BIT10_WEIGHT_MASK             (0x3FFF << SAR_P_BIT10_WEIGHT_SHIFT)
#define SAR_P_BIT10_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT10_WEIGHT, n)

// REG_78
#define SAR_P_BIT11_WEIGHT_SHIFT            0
#define SAR_P_BIT11_WEIGHT_MASK             (0x3FFF << SAR_P_BIT11_WEIGHT_SHIFT)
#define SAR_P_BIT11_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT11_WEIGHT, n)

// REG_79
#define SAR_P_BIT12_WEIGHT_SHIFT            0
#define SAR_P_BIT12_WEIGHT_MASK             (0x3FFF << SAR_P_BIT12_WEIGHT_SHIFT)
#define SAR_P_BIT12_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT12_WEIGHT, n)

// REG_7A
#define SAR_P_BIT13_WEIGHT_SHIFT            0
#define SAR_P_BIT13_WEIGHT_MASK             (0x3FFF << SAR_P_BIT13_WEIGHT_SHIFT)
#define SAR_P_BIT13_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT13_WEIGHT, n)

// REG_7B
#define SAR_P_BIT14_WEIGHT_SHIFT            0
#define SAR_P_BIT14_WEIGHT_MASK             (0x3FFF << SAR_P_BIT14_WEIGHT_SHIFT)
#define SAR_P_BIT14_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT14_WEIGHT, n)

// REG_7C
#define SAR_P_BIT15_WEIGHT_SHIFT            0
#define SAR_P_BIT15_WEIGHT_MASK             (0x3FFF << SAR_P_BIT15_WEIGHT_SHIFT)
#define SAR_P_BIT15_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT15_WEIGHT, n)

// REG_7D
#define SAR_P_BIT16_WEIGHT_SHIFT            0
#define SAR_P_BIT16_WEIGHT_MASK             (0x3FFF << SAR_P_BIT16_WEIGHT_SHIFT)
#define SAR_P_BIT16_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT16_WEIGHT, n)

// REG_7E
#define SAR_P_BIT17_WEIGHT_SHIFT            0
#define SAR_P_BIT17_WEIGHT_MASK             (0x3FFF << SAR_P_BIT17_WEIGHT_SHIFT)
#define SAR_P_BIT17_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT17_WEIGHT, n)

// REG_7F
#define SAR_N_BIT05_WEIGHT_SHIFT            0
#define SAR_N_BIT05_WEIGHT_MASK             (0x3FFF << SAR_N_BIT05_WEIGHT_SHIFT)
#define SAR_N_BIT05_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT05_WEIGHT, n)

// REG_80
#define SAR_N_BIT06_WEIGHT_SHIFT            0
#define SAR_N_BIT06_WEIGHT_MASK             (0x3FFF << SAR_N_BIT06_WEIGHT_SHIFT)
#define SAR_N_BIT06_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT06_WEIGHT, n)

// REG_81
#define SAR_N_BIT07_WEIGHT_SHIFT            0
#define SAR_N_BIT07_WEIGHT_MASK             (0x3FFF << SAR_N_BIT07_WEIGHT_SHIFT)
#define SAR_N_BIT07_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT07_WEIGHT, n)

// REG_82
#define SAR_N_BIT08_WEIGHT_SHIFT            0
#define SAR_N_BIT08_WEIGHT_MASK             (0x3FFF << SAR_N_BIT08_WEIGHT_SHIFT)
#define SAR_N_BIT08_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT08_WEIGHT, n)

// REG_83
#define SAR_N_BIT09_WEIGHT_SHIFT            0
#define SAR_N_BIT09_WEIGHT_MASK             (0x3FFF << SAR_N_BIT09_WEIGHT_SHIFT)
#define SAR_N_BIT09_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT09_WEIGHT, n)

// REG_84
#define SAR_N_BIT10_WEIGHT_SHIFT            0
#define SAR_N_BIT10_WEIGHT_MASK             (0x3FFF << SAR_N_BIT10_WEIGHT_SHIFT)
#define SAR_N_BIT10_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT10_WEIGHT, n)

// REG_85
#define SAR_N_BIT11_WEIGHT_SHIFT            0
#define SAR_N_BIT11_WEIGHT_MASK             (0x3FFF << SAR_N_BIT11_WEIGHT_SHIFT)
#define SAR_N_BIT11_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT11_WEIGHT, n)

// REG_86
#define SAR_N_BIT12_WEIGHT_SHIFT            0
#define SAR_N_BIT12_WEIGHT_MASK             (0x3FFF << SAR_N_BIT12_WEIGHT_SHIFT)
#define SAR_N_BIT12_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT12_WEIGHT, n)

// REG_87
#define SAR_N_BIT13_WEIGHT_SHIFT            0
#define SAR_N_BIT13_WEIGHT_MASK             (0x3FFF << SAR_N_BIT13_WEIGHT_SHIFT)
#define SAR_N_BIT13_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT13_WEIGHT, n)

// REG_88
#define SAR_N_BIT14_WEIGHT_SHIFT            0
#define SAR_N_BIT14_WEIGHT_MASK             (0x3FFF << SAR_N_BIT14_WEIGHT_SHIFT)
#define SAR_N_BIT14_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT14_WEIGHT, n)

// REG_89
#define SAR_N_BIT15_WEIGHT_SHIFT            0
#define SAR_N_BIT15_WEIGHT_MASK             (0x3FFF << SAR_N_BIT15_WEIGHT_SHIFT)
#define SAR_N_BIT15_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT15_WEIGHT, n)

// REG_8A
#define SAR_N_BIT16_WEIGHT_SHIFT            0
#define SAR_N_BIT16_WEIGHT_MASK             (0x3FFF << SAR_N_BIT16_WEIGHT_SHIFT)
#define SAR_N_BIT16_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT16_WEIGHT, n)

// REG_8B
#define SAR_N_BIT17_WEIGHT_SHIFT            0
#define SAR_N_BIT17_WEIGHT_MASK             (0x3FFF << SAR_N_BIT17_WEIGHT_SHIFT)
#define SAR_N_BIT17_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT17_WEIGHT, n)

// REG_8C
#define CLK_SARADC_CODEC_INV                (1 << 14)
#define REG_SAR_ADC_OFFSET_DR               (1 << 13)
#define REG_SAR_OFFSET_DR                   (1 << 12)
#define REG_SAR_WEIGHT_DR                   (1 << 11)
#define REG_SAR_CALI_LSB_SCRN               (1 << 10)
#define REG_SAR_CALI_CNT_SHIFT              6
#define REG_SAR_CALI_CNT_MASK               (0xF << REG_SAR_CALI_CNT_SHIFT)
#define REG_SAR_CALI_CNT(n)                 BITFIELD_VAL(REG_SAR_CALI_CNT, n)
#define REG_SAR_CALI                        (1 << 5)
#define REG_SAR_INIT_CALI_BIT_SHIFT         0
#define REG_SAR_INIT_CALI_BIT_MASK          (0x1F << REG_SAR_INIT_CALI_BIT_SHIFT)
#define REG_SAR_INIT_CALI_BIT(n)            BITFIELD_VAL(REG_SAR_INIT_CALI_BIT, n)

// REG_8D
#define REG_SAR_SAMPLE_IBIT_SHIFT           10
#define REG_SAR_SAMPLE_IBIT_MASK            (0x7 << REG_SAR_SAMPLE_IBIT_SHIFT)
#define REG_SAR_SAMPLE_IBIT(n)              BITFIELD_VAL(REG_SAR_SAMPLE_IBIT, n)
#define REG_SAR_OFFSET_N_SHIFT              5
#define REG_SAR_OFFSET_N_MASK               (0x1F << REG_SAR_OFFSET_N_SHIFT)
#define REG_SAR_OFFSET_N(n)                 BITFIELD_VAL(REG_SAR_OFFSET_N, n)
#define REG_SAR_OFFSET_P_SHIFT              0
#define REG_SAR_OFFSET_P_MASK               (0x1F << REG_SAR_OFFSET_P_SHIFT)
#define REG_SAR_OFFSET_P(n)                 BITFIELD_VAL(REG_SAR_OFFSET_P, n)

// REG_8E
#define REG_SAR_ADC_OFFSET_SHIFT            0
#define REG_SAR_ADC_OFFSET_MASK             (0xFFFF << REG_SAR_ADC_OFFSET_SHIFT)
#define REG_SAR_ADC_OFFSET(n)               BITFIELD_VAL(REG_SAR_ADC_OFFSET, n)

// REG_8F
#define REG_SAR_VCM_CORE_SEL_SHIFT          11
#define REG_SAR_VCM_CORE_SEL_MASK           (0x7 << REG_SAR_VCM_CORE_SEL_SHIFT)
#define REG_SAR_VCM_CORE_SEL(n)             BITFIELD_VAL(REG_SAR_VCM_CORE_SEL, n)
#define REG_SAR_DIFF_EN_IN_REG              (1 << 10)
#define REG_SAR_DIFF_EN_IN_DR               (1 << 9)
#define REG_SAR_SE2DIFF_EN                  (1 << 8)
#define REG_SAR_BUF_EN                      (1 << 7)
#define SAR_CONV_DONE_INV                   (1 << 6)
#define SAR_VOUT_CALIB_INV                  (1 << 5)
#define REG_SAR_CLK_OUT_SEL                 (1 << 4)
#define REG_SAR_OFFSET_CALI_CNT_SHIFT       1
#define REG_SAR_OFFSET_CALI_CNT_MASK        (0x7 << REG_SAR_OFFSET_CALI_CNT_SHIFT)
#define REG_SAR_OFFSET_CALI_CNT(n)          BITFIELD_VAL(REG_SAR_OFFSET_CALI_CNT, n)
#define REG_SAR_OFFSET_CALI                 (1 << 0)

// REG_90
#define REG_SAR_DELAY_CMP                   (1 << 15)
#define REG_SAR_INPUT_BUF_EN                (1 << 14)
#define REG_SAR_EN_PREAMP                   (1 << 13)
#define REG_SAR_DELAY_BIT_SHIFT             11
#define REG_SAR_DELAY_BIT_MASK              (0x3 << REG_SAR_DELAY_BIT_SHIFT)
#define REG_SAR_DELAY_BIT(n)                BITFIELD_VAL(REG_SAR_DELAY_BIT, n)
#define REG_SAR_CLK_MODE                    (1 << 10)
#define REG_SAR_CLK_TRIM_SHIFT              8
#define REG_SAR_CLK_TRIM_MASK               (0x3 << REG_SAR_CLK_TRIM_SHIFT)
#define REG_SAR_CLK_TRIM(n)                 BITFIELD_VAL(REG_SAR_CLK_TRIM, n)
#define REG_SAR_CH_SEL_IN_SHIFT             0
#define REG_SAR_CH_SEL_IN_MASK              (0xFF << REG_SAR_CH_SEL_IN_SHIFT)
#define REG_SAR_CH_SEL_IN(n)                BITFIELD_VAL(REG_SAR_CH_SEL_IN, n)

// REG_91
#define REG_SAR_HIGH_PW_CMP                 (1 << 11)
#define REG_SAR_HIGH_DVDD_EN                (1 << 10)
#define REG_SAR_VREG_OUTCAP_MODE            (1 << 9)
#define REG_SAR_VREG_IBIT_SHIFT             7
#define REG_SAR_VREG_IBIT_MASK              (0x3 << REG_SAR_VREG_IBIT_SHIFT)
#define REG_SAR_VREG_IBIT(n)                BITFIELD_VAL(REG_SAR_VREG_IBIT, n)
#define REG_SAR_THERM_GAIN_IN_SHIFT         5
#define REG_SAR_THERM_GAIN_IN_MASK          (0x3 << REG_SAR_THERM_GAIN_IN_SHIFT)
#define REG_SAR_THERM_GAIN_IN(n)            BITFIELD_VAL(REG_SAR_THERM_GAIN_IN, n)
#define REG_SAR_SE_MODE                     (1 << 4)
#define REG_SAR_PU_NIN_BIAS                 (1 << 3)
#define REG_SAR_PU_NIN_BIAS_DR              (1 << 2)
#define REG_SAR_PU_PIN_BIAS                 (1 << 1)
#define REG_SAR_PU_PIN_BIAS_DR              (1 << 0)

// REG_92
#define REG_SAR_RESET                       (1 << 9)
#define REG_SAR_ADC_RESET                   (1 << 8)
#define REG_SAR_CLK_OUT_DIV_SHIFT           0
#define REG_SAR_CLK_OUT_DIV_MASK            (0xFF << REG_SAR_CLK_OUT_DIV_SHIFT)
#define REG_SAR_CLK_OUT_DIV(n)              BITFIELD_VAL(REG_SAR_CLK_OUT_DIV, n)

// REG_93
#define REG_SAR_PU_START_DLY_CNT_SHIFT      9
#define REG_SAR_PU_START_DLY_CNT_MASK       (0x7F << REG_SAR_PU_START_DLY_CNT_SHIFT)
#define REG_SAR_PU_START_DLY_CNT(n)         BITFIELD_VAL(REG_SAR_PU_START_DLY_CNT, n)
#define REG_SAR_PU_RST_REG                  (1 << 8)
#define REG_SAR_PU_RST_DR                   (1 << 7)
#define REG_SAR_PU_PRECHARGE_REG            (1 << 6)
#define REG_SAR_PU_PRECHARGE_DR             (1 << 5)
#define REG_SAR_PU_REG                      (1 << 4)
#define REG_SAR_PU_DR                       (1 << 3)
#define REG_SAR_PU_VREF_IN                  (1 << 2)
#define REG_SAR_PU_VREF_REG                 (1 << 1)
#define REG_SAR_PU_VREF_DR                  (1 << 0)

// REG_94
#define REG_SAR_PU_RST_START_DLY_CNT_SHIFT  7
#define REG_SAR_PU_RST_START_DLY_CNT_MASK   (0x7F << REG_SAR_PU_RST_START_DLY_CNT_SHIFT)
#define REG_SAR_PU_RST_START_DLY_CNT(n)     BITFIELD_VAL(REG_SAR_PU_RST_START_DLY_CNT, n)
#define REG_SAR_PU_PRECHARGE_START_DLY_CNT_SHIFT 0
#define REG_SAR_PU_PRECHARGE_START_DLY_CNT_MASK (0x7F << REG_SAR_PU_PRECHARGE_START_DLY_CNT_SHIFT)
#define REG_SAR_PU_PRECHARGE_START_DLY_CNT(n) BITFIELD_VAL(REG_SAR_PU_PRECHARGE_START_DLY_CNT, n)

// REG_95
#define REG_SAR_PU_PRECHARGE_LAST_CNT_SHIFT 7
#define REG_SAR_PU_PRECHARGE_LAST_CNT_MASK  (0x1FF << REG_SAR_PU_PRECHARGE_LAST_CNT_SHIFT)
#define REG_SAR_PU_PRECHARGE_LAST_CNT(n)    BITFIELD_VAL(REG_SAR_PU_PRECHARGE_LAST_CNT, n)
#define REG_SAR_PU_RST_LAST_CNT_SHIFT       0
#define REG_SAR_PU_RST_LAST_CNT_MASK        (0x7F << REG_SAR_PU_RST_LAST_CNT_SHIFT)
#define REG_SAR_PU_RST_LAST_CNT(n)          BITFIELD_VAL(REG_SAR_PU_RST_LAST_CNT, n)


// REG_96
#define SAR_P_BIT05_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT05_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT05_WEIGHT_IN_SHIFT)
#define SAR_P_BIT05_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT05_WEIGHT_IN, n)

// REG_97
#define SAR_P_BIT06_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT06_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT06_WEIGHT_IN_SHIFT)
#define SAR_P_BIT06_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT06_WEIGHT_IN, n)

// REG_98
#define SAR_P_BIT07_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT07_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT07_WEIGHT_IN_SHIFT)
#define SAR_P_BIT07_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT07_WEIGHT_IN, n)

// REG_99
#define SAR_P_BIT08_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT08_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT08_WEIGHT_IN_SHIFT)
#define SAR_P_BIT08_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT08_WEIGHT_IN, n)

// REG_9A
#define SAR_P_BIT09_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT09_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT09_WEIGHT_IN_SHIFT)
#define SAR_P_BIT09_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT09_WEIGHT_IN, n)

// REG_9B
#define SAR_P_BIT10_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT10_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT10_WEIGHT_IN_SHIFT)
#define SAR_P_BIT10_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT10_WEIGHT_IN, n)

// REG_9C
#define SAR_P_BIT11_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT11_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT11_WEIGHT_IN_SHIFT)
#define SAR_P_BIT11_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT11_WEIGHT_IN, n)

// REG_9D
#define SAR_P_BIT12_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT12_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT12_WEIGHT_IN_SHIFT)
#define SAR_P_BIT12_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT12_WEIGHT_IN, n)

// REG_9E
#define SAR_P_BIT13_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT13_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT13_WEIGHT_IN_SHIFT)
#define SAR_P_BIT13_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT13_WEIGHT_IN, n)

// REG_9F
#define SAR_P_BIT14_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT14_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT14_WEIGHT_IN_SHIFT)
#define SAR_P_BIT14_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT14_WEIGHT_IN, n)

// REG_A0
#define SAR_P_BIT15_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT15_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT15_WEIGHT_IN_SHIFT)
#define SAR_P_BIT15_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT15_WEIGHT_IN, n)

// REG_A1
#define SAR_P_BIT16_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT16_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT16_WEIGHT_IN_SHIFT)
#define SAR_P_BIT16_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT16_WEIGHT_IN, n)

// REG_A2
#define SAR_P_BIT17_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT17_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT17_WEIGHT_IN_SHIFT)
#define SAR_P_BIT17_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT17_WEIGHT_IN, n)

// REG_A3
#define SAR_N_BIT05_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT05_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT05_WEIGHT_IN_SHIFT)
#define SAR_N_BIT05_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT05_WEIGHT_IN, n)

// REG_A4
#define SAR_N_BIT06_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT06_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT06_WEIGHT_IN_SHIFT)
#define SAR_N_BIT06_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT06_WEIGHT_IN, n)

// REG_A5
#define SAR_N_BIT07_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT07_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT07_WEIGHT_IN_SHIFT)
#define SAR_N_BIT07_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT07_WEIGHT_IN, n)

// REG_A6
#define SAR_N_BIT08_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT08_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT08_WEIGHT_IN_SHIFT)
#define SAR_N_BIT08_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT08_WEIGHT_IN, n)

// REG_A7
#define SAR_N_BIT09_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT09_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT09_WEIGHT_IN_SHIFT)
#define SAR_N_BIT09_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT09_WEIGHT_IN, n)

// REG_A8
#define SAR_N_BIT10_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT10_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT10_WEIGHT_IN_SHIFT)
#define SAR_N_BIT10_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT10_WEIGHT_IN, n)

// REG_A9
#define SAR_N_BIT11_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT11_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT11_WEIGHT_IN_SHIFT)
#define SAR_N_BIT11_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT11_WEIGHT_IN, n)

// REG_AA
#define SAR_N_BIT12_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT12_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT12_WEIGHT_IN_SHIFT)
#define SAR_N_BIT12_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT12_WEIGHT_IN, n)

// REG_AB
#define SAR_N_BIT13_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT13_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT13_WEIGHT_IN_SHIFT)
#define SAR_N_BIT13_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT13_WEIGHT_IN, n)

// REG_AC
#define SAR_N_BIT14_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT14_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT14_WEIGHT_IN_SHIFT)
#define SAR_N_BIT14_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT14_WEIGHT_IN, n)

// REG_AD
#define SAR_N_BIT15_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT15_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT15_WEIGHT_IN_SHIFT)
#define SAR_N_BIT15_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT15_WEIGHT_IN, n)

// REG_AE
#define SAR_N_BIT16_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT16_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT16_WEIGHT_IN_SHIFT)
#define SAR_N_BIT16_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT16_WEIGHT_IN, n)

// REG_AF
#define SAR_N_BIT17_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT17_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT17_WEIGHT_IN_SHIFT)
#define SAR_N_BIT17_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT17_WEIGHT_IN, n)

// REG_B0
#define REG_SAR_OFFSET_N_IN_SHIFT           5
#define REG_SAR_OFFSET_N_IN_MASK            (0x1F << REG_SAR_OFFSET_N_IN_SHIFT)
#define REG_SAR_OFFSET_N_IN(n)              BITFIELD_VAL(REG_SAR_OFFSET_N_IN, n)
#define REG_SAR_OFFSET_P_IN_SHIFT           0
#define REG_SAR_OFFSET_P_IN_MASK            (0x1F << REG_SAR_OFFSET_P_IN_SHIFT)
#define REG_SAR_OFFSET_P_IN(n)              BITFIELD_VAL(REG_SAR_OFFSET_P_IN, n)


// REG_B1
#define REG_SAR_ADC_OFFSET_IN_SHIFT         0
#define REG_SAR_ADC_OFFSET_IN_MASK          (0xFFFF << REG_SAR_ADC_OFFSET_IN_SHIFT)
#define REG_SAR_ADC_OFFSET_IN(n)            BITFIELD_VAL(REG_SAR_ADC_OFFSET_IN, n)

// REG_B2
#define REG_EFUSE_REDUNDANCY_DATA_OUT_SHIFT 2
#define REG_EFUSE_REDUNDANCY_DATA_OUT_MASK  (0xF << REG_EFUSE_REDUNDANCY_DATA_OUT_SHIFT)
#define REG_EFUSE_REDUNDANCY_DATA_OUT(n)    BITFIELD_VAL(REG_EFUSE_REDUNDANCY_DATA_OUT, n)
#define REG_EFUSE_REDUNDANCY_DATA_OUT_DR    (1 << 1)
#define REG_EFUSE_DATA_OUT_DR               (1 << 0)

// REG_B3
#define REG_EFUSE_DATA_OUT_HI_SHIFT         0
#define REG_EFUSE_DATA_OUT_HI_MASK          (0xFFFF << REG_EFUSE_DATA_OUT_HI_SHIFT)
#define REG_EFUSE_DATA_OUT_HI(n)            BITFIELD_VAL(REG_EFUSE_DATA_OUT_HI, n)

// REG_B4
#define REG_EFUSE_DATA_OUT_LO_SHIFT         0
#define REG_EFUSE_DATA_OUT_LO_MASK          (0xFFFF << REG_EFUSE_DATA_OUT_LO_SHIFT)
#define REG_EFUSE_DATA_OUT_LO(n)            BITFIELD_VAL(REG_EFUSE_DATA_OUT_LO, n)

// REG_B5
#define EFUSE_REDUNDANCY_DATA_OUT_SHIFT     0
#define EFUSE_REDUNDANCY_DATA_OUT_MASK      (0xF << EFUSE_REDUNDANCY_DATA_OUT_SHIFT)
#define EFUSE_REDUNDANCY_DATA_OUT(n)        BITFIELD_VAL(EFUSE_REDUNDANCY_DATA_OUT, n)

// REG_B6
#define EFUSE_CHIP_SEL_ENB_DR               (1 << 15)
#define EFUSE_CHIP_SEL_ENB_REG              (1 << 14)
#define EFUSE_STROBE_DR                     (1 << 13)
#define EFUSE_STROBE_REG                    (1 << 12)
#define EFUSE_LOAD_DR                       (1 << 11)
#define EFUSE_LOAD_REG                      (1 << 10)
#define EFUSE_PGM_ENB_DR                    (1 << 9)
#define EFUSE_PGM_ENB_REG                   (1 << 8)
#define EFUSE_PGM_SEL_DR                    (1 << 7)
#define EFUSE_PGM_SEL_REG                   (1 << 6)
#define EFUSE_READ_MODE_DR                  (1 << 5)
#define EFUSE_READ_MODE_REG                 (1 << 4)
#define EFUSE_PWR_DN_ENB_DR                 (1 << 3)
#define EFUSE_PWR_DN_ENB_REG                (1 << 2)
#define EFUSE_REDUNDANCY_EN_DR              (1 << 1)
#define EFUSE_REDUNDANCY_EN_REG             (1 << 0)

// REG_B7
#define REG_EFUSE_SEL                       (1 << 15)
#define REG_EFUSE_ADDRESS_SHIFT             6
#define REG_EFUSE_ADDRESS_MASK              (0x1FF << REG_EFUSE_ADDRESS_SHIFT)
#define REG_EFUSE_ADDRESS(n)                BITFIELD_VAL(REG_EFUSE_ADDRESS, n)
#define REG_EFUSE_STROBE_TRIGGER            (1 << 5)
#define REG_EFUSE_TURN_ON                   (1 << 4)
#define REG_EFUSE_CLK_EN                    (1 << 3)
#define REG_EFUSE_READ_MODE                 (1 << 2)
#define REG_EFUSE_PGM_MODE                  (1 << 1)
#define REG_EFUSE_PGM_READ_SEL              (1 << 0)

// REG_B8
#define REG_EFUSE_TIME_CSB_ADDR_VALUE_SHIFT 10
#define REG_EFUSE_TIME_CSB_ADDR_VALUE_MASK  (0xF << REG_EFUSE_TIME_CSB_ADDR_VALUE_SHIFT)
#define REG_EFUSE_TIME_CSB_ADDR_VALUE(n)    BITFIELD_VAL(REG_EFUSE_TIME_CSB_ADDR_VALUE, n)
#define REG_EFUSE_TIME_PS_CSB_VALUE_SHIFT   6
#define REG_EFUSE_TIME_PS_CSB_VALUE_MASK    (0xF << REG_EFUSE_TIME_PS_CSB_VALUE_SHIFT)
#define REG_EFUSE_TIME_PS_CSB_VALUE(n)      BITFIELD_VAL(REG_EFUSE_TIME_PS_CSB_VALUE, n)
#define REG_EFUSE_TIME_PD_PS_VALUE_SHIFT    0
#define REG_EFUSE_TIME_PD_PS_VALUE_MASK     (0x3F << REG_EFUSE_TIME_PD_PS_VALUE_SHIFT)
#define REG_EFUSE_TIME_PD_PS_VALUE(n)       BITFIELD_VAL(REG_EFUSE_TIME_PD_PS_VALUE, n)


// REG_B9
#define REG_EFUSE_TIME_PGM_STROBING_VALUE_SHIFT 4
#define REG_EFUSE_TIME_PGM_STROBING_VALUE_MASK (0x1FF << REG_EFUSE_TIME_PGM_STROBING_VALUE_SHIFT)
#define REG_EFUSE_TIME_PGM_STROBING_VALUE(n) BITFIELD_VAL(REG_EFUSE_TIME_PGM_STROBING_VALUE, n)
#define REG_EFUSE_TIME_ADDR_STROBE_VALUE_SHIFT 0
#define REG_EFUSE_TIME_ADDR_STROBE_VALUE_MASK (0xF << REG_EFUSE_TIME_ADDR_STROBE_VALUE_SHIFT)
#define REG_EFUSE_TIME_ADDR_STROBE_VALUE(n) BITFIELD_VAL(REG_EFUSE_TIME_ADDR_STROBE_VALUE, n)

// REG_BA
#define REG_EFUSE_TIME_READ_STROBING_VALUE_SHIFT 0
#define REG_EFUSE_TIME_READ_STROBING_VALUE_MASK (0x1FF << REG_EFUSE_TIME_READ_STROBING_VALUE_SHIFT)
#define REG_EFUSE_TIME_READ_STROBING_VALUE(n) BITFIELD_VAL(REG_EFUSE_TIME_READ_STROBING_VALUE, n)

// REG_BB
#define REG_EFUSE_TIME_PS_PD_VALUE_SHIFT    10
#define REG_EFUSE_TIME_PS_PD_VALUE_MASK     (0x3F << REG_EFUSE_TIME_PS_PD_VALUE_SHIFT)
#define REG_EFUSE_TIME_PS_PD_VALUE(n)       BITFIELD_VAL(REG_EFUSE_TIME_PS_PD_VALUE, n)
#define REG_EFUSE_TIME_CSB_PS_VALUE_SHIFT   6
#define REG_EFUSE_TIME_CSB_PS_VALUE_MASK    (0xF << REG_EFUSE_TIME_CSB_PS_VALUE_SHIFT)
#define REG_EFUSE_TIME_CSB_PS_VALUE(n)      BITFIELD_VAL(REG_EFUSE_TIME_CSB_PS_VALUE, n)
#define REG_EFUSE_TIME_STROBE_CSB_VALUE_SHIFT 0
#define REG_EFUSE_TIME_STROBE_CSB_VALUE_MASK (0x3F << REG_EFUSE_TIME_STROBE_CSB_VALUE_SHIFT)
#define REG_EFUSE_TIME_STROBE_CSB_VALUE(n)  BITFIELD_VAL(REG_EFUSE_TIME_STROBE_CSB_VALUE, n)

// REG_BC
#define EFUSE_REDUNDANCY_INFO_ROW_SEL_DR    (1 << 7)
#define EFUSE_REDUNDANCY_INFO_ROW_SEL_REG   (1 << 6)
#define REG_EFUSE_TIME_PD_OFF_VALUE_SHIFT   0
#define REG_EFUSE_TIME_PD_OFF_VALUE_MASK    (0x3F << REG_EFUSE_TIME_PD_OFF_VALUE_SHIFT)
#define REG_EFUSE_TIME_PD_OFF_VALUE(n)      BITFIELD_VAL(REG_EFUSE_TIME_PD_OFF_VALUE, n)

// REG_BD
#define EFUSE_DATA_OUT_HI_SHIFT             0
#define EFUSE_DATA_OUT_HI_MASK              (0xFFFF << EFUSE_DATA_OUT_HI_SHIFT)
#define EFUSE_DATA_OUT_HI(n)                BITFIELD_VAL(EFUSE_DATA_OUT_HI, n)

// REG_BE
#define EFUSE_DATA_OUT_LO_SHIFT             0
#define EFUSE_DATA_OUT_LO_MASK              (0xFFFF << EFUSE_DATA_OUT_LO_SHIFT)
#define EFUSE_DATA_OUT_LO(n)                BITFIELD_VAL(EFUSE_DATA_OUT_LO, n)

// REG_BF
#define REG_BUCK_HPPA_IS_GAIN_NORMAL_SHIFT  13
#define REG_BUCK_HPPA_IS_GAIN_NORMAL_MASK   (0x7 << REG_BUCK_HPPA_IS_GAIN_NORMAL_SHIFT)
#define REG_BUCK_HPPA_IS_GAIN_NORMAL(n)     BITFIELD_VAL(REG_BUCK_HPPA_IS_GAIN_NORMAL, n)
#define REG_BUCK_HPPA_INTERNAL_FREQUENCY_SHIFT 10
#define REG_BUCK_HPPA_INTERNAL_FREQUENCY_MASK (0x7 << REG_BUCK_HPPA_INTERNAL_FREQUENCY_SHIFT)
#define REG_BUCK_HPPA_INTERNAL_FREQUENCY(n) BITFIELD_VAL(REG_BUCK_HPPA_INTERNAL_FREQUENCY, n)
#define REG_BUCK_HPPA_PULLDOWN_EN           (1 << 9)
#define REG_BUCK_HPPA_ANTI_RES_DISABLE      (1 << 8)
#define REG_BUCK_HPPA_BURST_THRESHOLD_SHIFT 3
#define REG_BUCK_HPPA_BURST_THRESHOLD_MASK  (0x1F << REG_BUCK_HPPA_BURST_THRESHOLD_SHIFT)
#define REG_BUCK_HPPA_BURST_THRESHOLD(n)    BITFIELD_VAL(REG_BUCK_HPPA_BURST_THRESHOLD, n)
#define REG_BUCK_HPPA_SYNC_DISABLE          (1 << 2)
#define REG_BUCK_HPPA_SOFT_START_EN         (1 << 1)
#define REG_BUCK_HPPA_SLOPE_DOUBLE          (1 << 0)

// REG_C0
#define REG_BUCK_HPPA_IS_GAIN_SLEEP_SHIFT   13
#define REG_BUCK_HPPA_IS_GAIN_SLEEP_MASK    (0x7 << REG_BUCK_HPPA_IS_GAIN_SLEEP_SHIFT)
#define REG_BUCK_HPPA_IS_GAIN_SLEEP(n)      BITFIELD_VAL(REG_BUCK_HPPA_IS_GAIN_SLEEP, n)
#define REG_BUCK_HPPA_VREF_SEL_SHIFT        11
#define REG_BUCK_HPPA_VREF_SEL_MASK         (0x3 << REG_BUCK_HPPA_VREF_SEL_SHIFT)
#define REG_BUCK_HPPA_VREF_SEL(n)           BITFIELD_VAL(REG_BUCK_HPPA_VREF_SEL, n)
#define REG_BUCK_HPPA_TEST_POWERMOS         (1 << 10)
#define REG_BUCK_HPPA_EN_ZCD_CAL            (1 << 9)
#define REG_BUCK_HPPA_TEST_PMOS             (1 << 8)
#define REG_BUCK_HPPA_HALF_BIAS             (1 << 7)
#define REG_BUCK_HPPA_SLOPE_EN_BURST        (1 << 6)
#define REG_BUCK_HPPA_OFFSET_CURRENT_EN     (1 << 5)
#define REG_BUCK_HPPA_I_DELTAV_X2           (1 << 4)
#define REG_BUCK_HPPA_LOGIC_IR_CLK_EN       (1 << 3)
#define REG_BUCK_HPPA_ENB_PU_DELAY          (1 << 2)
#define REG_BUCK_HPPA_IX2_ERR               (1 << 1)
#define REG_BUCK_HPPA_LP_VCOMP              (1 << 0)

// REG_C1
#define REG_BUCK_HPPA_ZCD_CAP_BIT_SHIFT     12
#define REG_BUCK_HPPA_ZCD_CAP_BIT_MASK      (0xF << REG_BUCK_HPPA_ZCD_CAP_BIT_SHIFT)
#define REG_BUCK_HPPA_ZCD_CAP_BIT(n)        BITFIELD_VAL(REG_BUCK_HPPA_ZCD_CAP_BIT, n)
#define REG_BUCK_HPPA_CAP_BIT_SHIFT         8
#define REG_BUCK_HPPA_CAP_BIT_MASK          (0xF << REG_BUCK_HPPA_CAP_BIT_SHIFT)
#define REG_BUCK_HPPA_CAP_BIT(n)            BITFIELD_VAL(REG_BUCK_HPPA_CAP_BIT, n)
#define REG_BUCK_HPPA_DT_BIT                (1 << 6)
#define REG_BUCK_HPPA_CAL_DELTAV_SHIFT      0
#define REG_BUCK_HPPA_CAL_DELTAV_MASK       (0x3F << REG_BUCK_HPPA_CAL_DELTAV_SHIFT)
#define REG_BUCK_HPPA_CAL_DELTAV(n)         BITFIELD_VAL(REG_BUCK_HPPA_CAL_DELTAV, n)

// REG_C2
#define REG_BUCK_HPPA_EDGE_CON_SHIFT        12
#define REG_BUCK_HPPA_EDGE_CON_MASK         (0xF << REG_BUCK_HPPA_EDGE_CON_SHIFT)
#define REG_BUCK_HPPA_EDGE_CON(n)           BITFIELD_VAL(REG_BUCK_HPPA_EDGE_CON, n)
#define REG_BUCK_HPPA_OFFSET_BIT_SHIFT      7
#define REG_BUCK_HPPA_OFFSET_BIT_MASK       (0x1F << REG_BUCK_HPPA_OFFSET_BIT_SHIFT)
#define REG_BUCK_HPPA_OFFSET_BIT(n)         BITFIELD_VAL(REG_BUCK_HPPA_OFFSET_BIT, n)
#define REG_BUCK_HPPA_HYSTERESIS_BIT_SHIFT  4
#define REG_BUCK_HPPA_HYSTERESIS_BIT_MASK   (0x7 << REG_BUCK_HPPA_HYSTERESIS_BIT_SHIFT)
#define REG_BUCK_HPPA_HYSTERESIS_BIT(n)     BITFIELD_VAL(REG_BUCK_HPPA_HYSTERESIS_BIT, n)
#define REG_BUCK_HPPA_COUNTER_SEL_SHIFT     2
#define REG_BUCK_HPPA_COUNTER_SEL_MASK      (0x3 << REG_BUCK_HPPA_COUNTER_SEL_SHIFT)
#define REG_BUCK_HPPA_COUNTER_SEL(n)        BITFIELD_VAL(REG_BUCK_HPPA_COUNTER_SEL, n)
#define REG_BUCK_HPPA_ULP_MODE_DSLEEP       (1 << 1)
#define REG_BUCK_HPPA_ULP_MODE_NORMAL       (1 << 0)

// REG_C3
#define REG_BUCK_HPPA_VBIT_DSLEEP_SHIFT     8
#define REG_BUCK_HPPA_VBIT_DSLEEP_MASK      (0xFF << REG_BUCK_HPPA_VBIT_DSLEEP_SHIFT)
#define REG_BUCK_HPPA_VBIT_DSLEEP(n)        BITFIELD_VAL(REG_BUCK_HPPA_VBIT_DSLEEP, n)
#define REG_BUCK_HPPA_VBIT_NORMAL_SHIFT     0
#define REG_BUCK_HPPA_VBIT_NORMAL_MASK      (0xFF << REG_BUCK_HPPA_VBIT_NORMAL_SHIFT)
#define REG_BUCK_HPPA_VBIT_NORMAL(n)        BITFIELD_VAL(REG_BUCK_HPPA_VBIT_NORMAL, n)

// REG_C4
#define REG_BUCK_ANA_CAP_BIT_SHIFT          12
#define REG_BUCK_ANA_CAP_BIT_MASK           (0xF << REG_BUCK_ANA_CAP_BIT_SHIFT)
#define REG_BUCK_ANA_CAP_BIT(n)             BITFIELD_VAL(REG_BUCK_ANA_CAP_BIT, n)
#define REG_BUCK_ANA_CAL_DELTAV_SHIFT       6
#define REG_BUCK_ANA_CAL_DELTAV_MASK        (0x3F << REG_BUCK_ANA_CAL_DELTAV_SHIFT)
#define REG_BUCK_ANA_CAL_DELTAV(n)          BITFIELD_VAL(REG_BUCK_ANA_CAL_DELTAV, n)
#define REG_BUCK_ANA_BURST_THRESHOLD_SHIFT  0
#define REG_BUCK_ANA_BURST_THRESHOLD_MASK   (0x3F << REG_BUCK_ANA_BURST_THRESHOLD_SHIFT)
#define REG_BUCK_ANA_BURST_THRESHOLD(n)     BITFIELD_VAL(REG_BUCK_ANA_BURST_THRESHOLD, n)

// REG_C5
#define REG_BUCK_VREF_SEL_SHIFT             10
#define REG_BUCK_VREF_SEL_MASK              (0x3F << REG_BUCK_VREF_SEL_SHIFT)
#define REG_BUCK_VREF_SEL(n)                BITFIELD_VAL(REG_BUCK_VREF_SEL, n)
#define REG_BUCK_ANA_TEST_MODE_EN           (1 << 8)
#define REG_BUCK_ANA_ULP_MODE_DSLEEP        (1 << 7)
#define REG_BUCK_ANA_ULP_MODE_NORMAL        (1 << 6)
#define REG_BUCK_ANA_TEST_POWERMOS          (1 << 5)
#define REG_BUCK_ANA_TEST_PMOS              (1 << 4)
#define REG_BUCK_ANA_SLOPE_EN_BURST         (1 << 3)
#define REG_BUCK_ANA_OFFSET_CURRENT_EN      (1 << 2)
#define REG_BUCK_ANA_LOGIC_IR_CLK_EN        (1 << 1)
#define REG_BUCK_ANA_ENB_PU_DELAY           (1 << 0)

// REG_C6
#define REG_BUCK_HPPA_TEST_MODE_EN          (1 << 5)
#define REG_BUCK_ANA_OFFSET_BIT_SHIFT       0
#define REG_BUCK_ANA_OFFSET_BIT_MASK        (0x1F << REG_BUCK_ANA_OFFSET_BIT_SHIFT)
#define REG_BUCK_ANA_OFFSET_BIT(n)          BITFIELD_VAL(REG_BUCK_ANA_OFFSET_BIT, n)

// REG_C7
#define REG_BUCK_VCORE_IX2_ERR              (1 << 15)
#define REG_BUCK_VCORE_CAP_BIT_SHIFT        11
#define REG_BUCK_VCORE_CAP_BIT_MASK         (0xF << REG_BUCK_VCORE_CAP_BIT_SHIFT)
#define REG_BUCK_VCORE_CAP_BIT(n)           BITFIELD_VAL(REG_BUCK_VCORE_CAP_BIT, n)
#define REG_BUCK_VCORE_CAL_DELTAV_SHIFT     5
#define REG_BUCK_VCORE_CAL_DELTAV_MASK      (0x3F << REG_BUCK_VCORE_CAL_DELTAV_SHIFT)
#define REG_BUCK_VCORE_CAL_DELTAV(n)        BITFIELD_VAL(REG_BUCK_VCORE_CAL_DELTAV, n)
#define REG_BUCK_VCORE_BURST_THRESHOLD_SHIFT 0
#define REG_BUCK_VCORE_BURST_THRESHOLD_MASK (0x1F << REG_BUCK_VCORE_BURST_THRESHOLD_SHIFT)
#define REG_BUCK_VCORE_BURST_THRESHOLD(n)   BITFIELD_VAL(REG_BUCK_VCORE_BURST_THRESHOLD, n)

// REG_C8
#define REG_BUCK_VCORE_VREF_SEL_SHIFT       14
#define REG_BUCK_VCORE_VREF_SEL_MASK        (0x3 << REG_BUCK_VCORE_VREF_SEL_SHIFT)
#define REG_BUCK_VCORE_VREF_SEL(n)          BITFIELD_VAL(REG_BUCK_VCORE_VREF_SEL, n)
#define REG_BUCK_VCORE_HYSTERESIS_BIT_SHIFT 11
#define REG_BUCK_VCORE_HYSTERESIS_BIT_MASK  (0x7 << REG_BUCK_VCORE_HYSTERESIS_BIT_SHIFT)
#define REG_BUCK_VCORE_HYSTERESIS_BIT(n)    BITFIELD_VAL(REG_BUCK_VCORE_HYSTERESIS_BIT, n)
#define REG_BUCK_VCORE_COUNTER_SEL_SHIFT    9
#define REG_BUCK_VCORE_COUNTER_SEL_MASK     (0x3 << REG_BUCK_VCORE_COUNTER_SEL_SHIFT)
#define REG_BUCK_VCORE_COUNTER_SEL(n)       BITFIELD_VAL(REG_BUCK_VCORE_COUNTER_SEL, n)
#define REG_BUCK_VCORE_TEST_MODE_EN         (1 << 8)
#define REG_BUCK_VCORE_ULP_MODE_DSLEEP      (1 << 7)
#define REG_BUCK_VCORE_ULP_MODE_NORMAL      (1 << 6)
#define REG_BUCK_VCORE_TEST_POWERMOS        (1 << 5)
#define REG_BUCK_VCORE_TEST_PMOS            (1 << 4)
#define REG_BUCK_VCORE_SLOPE_EN_BURST       (1 << 3)
#define REG_BUCK_VCORE_OFFSET_CURRENT_EN    (1 << 2)
#define REG_BUCK_VCORE_LOGIC_IR_CLK_EN      (1 << 1)
#define REG_BUCK_VCORE_ENB_PU_DELAY         (1 << 0)

// REG_C9
#define REG_BUCK_VCORE_OFFSET_BIT_SHIFT     0
#define REG_BUCK_VCORE_OFFSET_BIT_MASK      (0x1F << REG_BUCK_VCORE_OFFSET_BIT_SHIFT)
#define REG_BUCK_VCORE_OFFSET_BIT(n)        BITFIELD_VAL(REG_BUCK_VCORE_OFFSET_BIT, n)

// REG_CA
#define DCDC_VCODEC_LP_EN_DSLEEP            (1 << 7)
#define PU_DCDC_VCODEC_DR                   (1 << 6)
#define PU_DCDC_VCODEC_REG                  (1 << 5)
#define DCDC_VCODEC_LP_EN_DR                (1 << 4)
#define DCDC_VCODEC_LP_EN_REG               (1 << 3)
#define PU_DCDC_VCODEC_DSLEEP               (1 << 2)
#define REG_BUCK_VCODEC_BURST_MODE_NORMAL   (1 << 1)
#define REG_BUCK_VCODEC_BURST_MODE_DSLEEP   (1 << 0)

// REG_CA
#define DCDC_HPPA_LP_EN_DSLEEP              DCDC_VCODEC_LP_EN_DSLEEP
#define PU_DCDC_HPPA_DR                     PU_DCDC_VCODEC_DR
#define PU_DCDC_HPPA_REG                    PU_DCDC_VCODEC_REG
#define DCDC_HPPA_LP_EN_DR                  DCDC_VCODEC_LP_EN_DR
#define DCDC_HPPA_LP_EN_REG                 DCDC_VCODEC_LP_EN_REG
#define PU_DCDC_HPPA_DSLEEP                 PU_DCDC_VCODEC_DSLEEP
#define REG_BUCK_HPPA_BURST_MODE_NORMAL     REG_BUCK_VCODEC_BURST_MODE_NORMAL
#define REG_BUCK_HPPA_BURST_MODE_DSLEEP     REG_BUCK_VCODEC_BURST_MODE_DSLEEP

// REG_CB
#define AC_PRESENT_DB_CNT_SHIFT             11
#define AC_PRESENT_DB_CNT_MASK              (0x1F << AC_PRESENT_DB_CNT_SHIFT)
#define AC_PRESENT_DB_CNT(n)                BITFIELD_VAL(AC_PRESENT_DB_CNT, n)
#define AC_PRESENT_INTR_EN                  (1 << 10)
#define AC_PRESENT_N_INTR_MSK               (1 << 9)
#define AC_PRESENT_P_INTR_MSK               (1 << 8)
#define AC_PRESENT_N_INTR_MSKED             (1 << 5)
#define AC_PRESENT_P_INTR_MSKED             (1 << 4)
#define AC_PRESENT_N_INTR                   (1 << 3)
#define AC_PRESENT_P_INTR                   (1 << 2)
#define AC_PRESENT_STATUS                   (1 << 1)
#define AC_PRESENT                          (1 << 0)

#define AC_PRESENT_N_INTR_CLR               (1 << 3)
#define AC_PRESENT_P_INTR_CLR               (1 << 2)

// REG_CE
#define REG_BUCK_ANA_BURST_MODE_RC          (1 << 15)
#define REG_PU_DCDC_ANA_RC_SLP              (1 << 14)
#define REG_PU_LDO_VUSB_RC_SLP              (1 << 13)
#define REG_PU_LDO_VPA_RC_SLP               (1 << 12)
#define REG_PU_LDO_VCODEC_RC_SLP            (1 << 11)
#define REG_PU_LDO_DIG_RC_SLP               (1 << 10)
#define REG_PU_LDO_VANA_RC_SLP              (1 << 9)
#define REG_PU_LP_BIAS_LDO_RC_SLP           (1 << 8)
#define REG_RC_SLEEP_ALLOW                  (1 << 7)
#define REG_RC_MODE2_EN                     (1 << 6)
#define REG_RC_SLEEP_MODE_DR                (1 << 5)
#define REG_RC_SLEEP                        (1 << 4)
#define REG_OSC_MASK0                       (1 << 3)
#define REG_RC_MASK0                        (1 << 2)
#define REG_OSC_MASK1                       (1 << 1)
#define REG_RC_MASK1                        (1 << 0)

// REG_CF
#define REG_BUCK_ANA_IS_GAIN_RC_SHIFT       13
#define REG_BUCK_ANA_IS_GAIN_RC_MASK        (0x7 << REG_BUCK_ANA_IS_GAIN_RC_SHIFT)
#define REG_BUCK_ANA_IS_GAIN_RC(n)          BITFIELD_VAL(REG_BUCK_ANA_IS_GAIN_RC, n)
#define LDO_VCODEC_VBIT_RC_SHIFT            8
#define LDO_VCODEC_VBIT_RC_MASK             (0x1F << LDO_VCODEC_VBIT_RC_SHIFT)
#define LDO_VCODEC_VBIT_RC(n)               BITFIELD_VAL(LDO_VCODEC_VBIT_RC, n)
#define LDO_DIG_VBIT_RC_SHIFT               4
#define LDO_DIG_VBIT_RC_MASK                (0xF << LDO_DIG_VBIT_RC_SHIFT)
#define LDO_DIG_VBIT_RC(n)                  BITFIELD_VAL(LDO_DIG_VBIT_RC, n)
#define LDO_ANA_VBIT_RC_SHIFT               0
#define LDO_ANA_VBIT_RC_MASK                (0xF << LDO_ANA_VBIT_RC_SHIFT)
#define LDO_ANA_VBIT_RC(n)                  BITFIELD_VAL(LDO_ANA_VBIT_RC, n)

// REG_D0
#define REG_PU_LDO_VRTC_RF_RC_SLP           (1 << 7)
#define REG_BUCK_HPPA_ULP_MODE_RC           (1 << 6)
#define REG_BUCK_VCORE_ULP_MODE_RC          (1 << 5)
#define REG_BUCK_ANA_ULP_MODE_RC            (1 << 4)
#define REG_BUCK_HPPA_BURST_MODE_RC         (1 << 3)
#define REG_PU_DCDC_HPPA_RC_SLP             (1 << 2)
#define REG_BUCK_VCORE_BURST_MODE_RC        (1 << 1)
#define REG_PU_DCDC_DIG_RC_SLP              (1 << 0)

// REG_D1
#define REG_BUCK_HPPA_IS_GAIN_RC_SHIFT      13
#define REG_BUCK_HPPA_IS_GAIN_RC_MASK       (0x7 << REG_BUCK_HPPA_IS_GAIN_RC_SHIFT)
#define REG_BUCK_HPPA_IS_GAIN_RC(n)         BITFIELD_VAL(REG_BUCK_HPPA_IS_GAIN_RC, n)
#define REG_BUCK_VCORE_IS_GAIN_RC_SHIFT     10
#define REG_BUCK_VCORE_IS_GAIN_RC_MASK      (0x7 << REG_BUCK_VCORE_IS_GAIN_RC_SHIFT)
#define REG_BUCK_VCORE_IS_GAIN_RC(n)        BITFIELD_VAL(REG_BUCK_VCORE_IS_GAIN_RC, n)
#define LDO_VUSB_VBIT_RC_SHIFT              5
#define LDO_VUSB_VBIT_RC_MASK               (0x1F << LDO_VUSB_VBIT_RC_SHIFT)
#define LDO_VUSB_VBIT_RC(n)                 BITFIELD_VAL(LDO_VUSB_VBIT_RC, n)
#define LDO_VPA_VBIT_RC_SHIFT               0
#define LDO_VPA_VBIT_RC_MASK                (0x1F << LDO_VPA_VBIT_RC_SHIFT)
#define LDO_VPA_VBIT_RC(n)                  BITFIELD_VAL(LDO_VPA_VBIT_RC, n)

// REG_D2
#define BUCK_VCORE_VBIT_RC_SHIFT            8
#define BUCK_VCORE_VBIT_RC_MASK             (0xFF << BUCK_VCORE_VBIT_RC_SHIFT)
#define BUCK_VCORE_VBIT_RC(n)               BITFIELD_VAL(BUCK_VCORE_VBIT_RC, n)
#define BUCK_VANA_VBIT_RC_SHIFT             0
#define BUCK_VANA_VBIT_RC_MASK              (0xFF << BUCK_VANA_VBIT_RC_SHIFT)
#define BUCK_VANA_VBIT_RC(n)                BITFIELD_VAL(BUCK_VANA_VBIT_RC, n)

// REG_D3
#define BUCK_VCODEC_VBIT_RC_SHIFT           0
#define BUCK_VCODEC_VBIT_RC_MASK            (0xFF << BUCK_VCODEC_VBIT_RC_SHIFT)
#define BUCK_VCODEC_VBIT_RC(n)              BITFIELD_VAL(BUCK_VCODEC_VBIT_RC, n)

// REG_D4
#define REG_BUCK_VCORE_EDGE_CON_RX_SHIFT    8
#define REG_BUCK_VCORE_EDGE_CON_RX_MASK     (0xF << REG_BUCK_VCORE_EDGE_CON_RX_SHIFT)
#define REG_BUCK_VCORE_EDGE_CON_RX(n)       BITFIELD_VAL(REG_BUCK_VCORE_EDGE_CON_RX, n)
#define REG_BUCK_ANA_EDGE_CON_RX_SHIFT      4
#define REG_BUCK_ANA_EDGE_CON_RX_MASK       (0xF << REG_BUCK_ANA_EDGE_CON_RX_SHIFT)
#define REG_BUCK_ANA_EDGE_CON_RX(n)         BITFIELD_VAL(REG_BUCK_ANA_EDGE_CON_RX, n)
#define REG_BUCK_HPPA_EDGE_CON_RX_SHIFT     0
#define REG_BUCK_HPPA_EDGE_CON_RX_MASK      (0xF << REG_BUCK_HPPA_EDGE_CON_RX_SHIFT)
#define REG_BUCK_HPPA_EDGE_CON_RX(n)        BITFIELD_VAL(REG_BUCK_HPPA_EDGE_CON_RX, n)

// REG_D5
#define REG_BUCK_PRIOR_CHANNEL_DSLEEP_SHIFT 14
#define REG_BUCK_PRIOR_CHANNEL_DSLEEP_MASK  (0x3 << REG_BUCK_PRIOR_CHANNEL_DSLEEP_SHIFT)
#define REG_BUCK_PRIOR_CHANNEL_DSLEEP(n)    BITFIELD_VAL(REG_BUCK_PRIOR_CHANNEL_DSLEEP, n)
#define REG_BUCK_PRIOR_CHANNEL_NORMAL_SHIFT 12
#define REG_BUCK_PRIOR_CHANNEL_NORMAL_MASK  (0x3 << REG_BUCK_PRIOR_CHANNEL_NORMAL_SHIFT)
#define REG_BUCK_PRIOR_CHANNEL_NORMAL(n)    BITFIELD_VAL(REG_BUCK_PRIOR_CHANNEL_NORMAL, n)
#define REG_BUCK_BIAS_HALF_B                (1 << 11)
#define REG_BUCK_IX2_ERR_NORMAL             (1 << 10)
#define REG_BUCK_IX4_ERR_NORMAL             (1 << 9)
#define REG_BUCK_REV_BACK                   (1 << 8)
#define REG_BUCK_RESERVED_SHIFT             0
#define REG_BUCK_RESERVED_MASK              (0xFF << REG_BUCK_RESERVED_SHIFT)
#define REG_BUCK_RESERVED(n)                BITFIELD_VAL(REG_BUCK_RESERVED, n)

// REG_D6
#define ABORT_SOURCE_SHIFT                  0
#define ABORT_SOURCE_MASK                   (0x7 << ABORT_SOURCE_SHIFT)
#define ABORT_SOURCE(n)                     BITFIELD_VAL(ABORT_SOURCE, n)

// REG_D7
#define REG_GP_RAMP_STEP_SHIFT              8
#define REG_GP_RAMP_STEP_MASK               (0xF << REG_GP_RAMP_STEP_SHIFT)
#define REG_GP_RAMP_STEP(n)                 BITFIELD_VAL(REG_GP_RAMP_STEP, n)
#define REG_VOUT_HV_VCORE_L                 (1 << 6)
#define REG_BYPASS_VCORE_L                  (1 << 5)
#define REG_VCORE_L_RAMP_STEP_SHIFT         0
#define REG_VCORE_L_RAMP_STEP_MASK          (0x1F << REG_VCORE_L_RAMP_STEP_SHIFT)
#define REG_VCORE_L_RAMP_STEP(n)            BITFIELD_VAL(REG_VCORE_L_RAMP_STEP, n)


// REG_D8
#define REG_BUCK_ANA_VREF_SEL_DSLEEP_SHIFT  7
#define REG_BUCK_ANA_VREF_SEL_DSLEEP_MASK   (0x3F << REG_BUCK_ANA_VREF_SEL_DSLEEP_SHIFT)
#define REG_BUCK_ANA_VREF_SEL_DSLEEP(n)     BITFIELD_VAL(REG_BUCK_ANA_VREF_SEL_DSLEEP, n)
#define REG_BUCK_ANA_VREF_SEL_NORMAL_SHIFT  0
#define REG_BUCK_ANA_VREF_SEL_NORMAL_MASK   (0x3F << REG_BUCK_ANA_VREF_SEL_NORMAL_SHIFT)
#define REG_BUCK_ANA_VREF_SEL_NORMAL(n)     BITFIELD_VAL(REG_BUCK_ANA_VREF_SEL_NORMAL, n)

// REG_D9
#define REG_BUCK_EN_SIMO                    (1 << 12)
#define REG_BUCK_IX2_ERR_DSLEEP             (1 << 11)
#define REG_BUCK_IX4_ERR_DSLEEP             (1 << 10)
#define REG_BUCK_IX2_ERR_RC                 (1 << 9)
#define REG_BUCK_IX4_ERR_RC                 (1 << 8)
#define REG_BUCK_PRIOR_CHANNEL_RC_SHIFT     6
#define REG_BUCK_PRIOR_CHANNEL_RC_MASK      (0x3 << REG_BUCK_PRIOR_CHANNEL_RC_SHIFT)
#define REG_BUCK_PRIOR_CHANNEL_RC(n)        BITFIELD_VAL(REG_BUCK_PRIOR_CHANNEL_RC, n)
#define REG_BUCK_ANA_VREF_SEL_RC_SHIFT      0
#define REG_BUCK_ANA_VREF_SEL_RC_MASK       (0x3F << REG_BUCK_ANA_VREF_SEL_RC_SHIFT)
#define REG_BUCK_ANA_VREF_SEL_RC(n)         BITFIELD_VAL(REG_BUCK_ANA_VREF_SEL_RC, n)

// REG_DA
#define REG_PU_VSYS_DIV                     (1 << 9)

// REG_DC
#define REG_VSENSOR_RAMP_EN                 (1 << 12)
#define LDO_VSENSOR_VBIT_RC_SHIFT           0
#define LDO_VSENSOR_VBIT_RC_MASK            (0x3F << LDO_VSENSOR_VBIT_RC_SHIFT)
#define LDO_VSENSOR_VBIT_RC(n)              BITFIELD_VAL(LDO_VSENSOR_VBIT_RC, n)

// REG_DD
#define REG_PULL_DOWN_VSENSOR               (1 << 15)
#define LP_EN_VSENSOR_LDO_REG               (1 << 14)
#define LP_EN_VSENSOR_LDO_DR                (1 << 13)
#define LP_EN_VSENSOR_LDO_DSLEEP            (1 << 12)
#define REG_PU_LDO_VSENSOR_REG              (1 << 11)
#define REG_PU_LDO_VSENSOR_DR               (1 << 10)
#define REG_PU_LDO_VSENSOR_RC_SLP           (1 << 9)
#define REG_PU_LDO_VSENSOR_DSLEEP           (1 << 8)
#define LDO_VSENSOR_VBIT_NORMAL_SHIFT       0
#define LDO_VSENSOR_VBIT_NORMAL_MASK        (0x3F << LDO_VSENSOR_VBIT_NORMAL_SHIFT)
#define LDO_VSENSOR_VBIT_NORMAL(n)          BITFIELD_VAL(LDO_VSENSOR_VBIT_NORMAL, n)

// REG_DE
#define LDO_VSENSOR_VBIT_DSLEEP_SHIFT       0
#define LDO_VSENSOR_VBIT_DSLEEP_MASK        (0x3F << LDO_VSENSOR_VBIT_DSLEEP_SHIFT)
#define LDO_VSENSOR_VBIT_DSLEEP(n)          BITFIELD_VAL(LDO_VSENSOR_VBIT_DSLEEP, n)

// REG_DF
#define REG_GUARD_MODE_SEL                  (1 << 15)
#define POWER_DOWN_INTR_MERGED_MSK          (1 << 14)
#define POWER_DOWN_INTR_MSK                 (1 << 13)
#define HARD_POWER_OFF_INTR_EN              (1 << 12)
#define REG_VRTC_SEL_SHIFT                  8
#define REG_VRTC_SEL_MASK                   (0xF << REG_VRTC_SEL_SHIFT)
#define REG_VRTC_SEL(n)                     BITFIELD_VAL(REG_VRTC_SEL, n)
#define REG_GUARD_RELEASE_SHIFT             4
#define REG_GUARD_RELEASE_MASK              (0xF << REG_GUARD_RELEASE_SHIFT)
#define REG_GUARD_RELEASE(n)                BITFIELD_VAL(REG_GUARD_RELEASE, n)
#define REG_GUARD_HIGH_SHIFT                0
#define REG_GUARD_HIGH_MASK                 (0xF << REG_GUARD_HIGH_SHIFT)
#define REG_GUARD_HIGH(n)                   BITFIELD_VAL(REG_GUARD_HIGH, n)

// REG_E0
#define REG_OSC_NPULSE_EXT_EN               (1 << 14)
#define REG_OSC_GUARD_TIME_SHIFT            8
#define REG_OSC_GUARD_TIME_MASK             (0x3F << REG_OSC_GUARD_TIME_SHIFT)
#define REG_OSC_GUARD_TIME(n)               BITFIELD_VAL(REG_OSC_GUARD_TIME, n)
#define REG_POWER_DOWN_DB_VALUE_SHIFT       0
#define REG_POWER_DOWN_DB_VALUE_MASK        (0xFF << REG_POWER_DOWN_DB_VALUE_SHIFT)
#define REG_POWER_DOWN_DB_VALUE(n)          BITFIELD_VAL(REG_POWER_DOWN_DB_VALUE, n)

// REG_E1
#define REG_BIASB_LPFSEL_DLY_SHIFT          5
#define REG_BIASB_LPFSEL_DLY_MASK           (0x1F << REG_BIASB_LPFSEL_DLY_SHIFT)
#define REG_BIASB_LPFSEL_DLY(n)             BITFIELD_VAL(REG_BIASB_LPFSEL_DLY, n)
#define REG_BIASA_LPFSEL_DLY_SHIFT          0
#define REG_BIASA_LPFSEL_DLY_MASK           (0x1F << REG_BIASA_LPFSEL_DLY_SHIFT)
#define REG_BIASA_LPFSEL_DLY(n)             BITFIELD_VAL(REG_BIASA_LPFSEL_DLY, n)

// REG_E2
#define REG_REFRESH_MODE_SEL                (1 << 15)
#define REG_RC_ASENSOR_EN                   (1 << 14)
#define REG_MIC_BIASB_EN_DR                 (1 << 13)
#define REG_MIC_LDO_EN_DR                   (1 << 12)
#define REG_MIC_BIASB_LPFSEL_SHIFT          9
#define REG_MIC_BIASB_LPFSEL_MASK           (0x7 << REG_MIC_BIASB_LPFSEL_SHIFT)
#define REG_MIC_BIASB_LPFSEL(n)             BITFIELD_VAL(REG_MIC_BIASB_LPFSEL, n)
#define REG_MIC_BIASB_LPFSEL_CHANGE_SHIFT   6
#define REG_MIC_BIASB_LPFSEL_CHANGE_MASK    (0x7 << REG_MIC_BIASB_LPFSEL_CHANGE_SHIFT)
#define REG_MIC_BIASB_LPFSEL_CHANGE(n)      BITFIELD_VAL(REG_MIC_BIASB_LPFSEL_CHANGE, n)
#define REG_MIC_BIASA_LPFSEL_SHIFT          3
#define REG_MIC_BIASA_LPFSEL_MASK           (0x7 << REG_MIC_BIASA_LPFSEL_SHIFT)
#define REG_MIC_BIASA_LPFSEL(n)             BITFIELD_VAL(REG_MIC_BIASA_LPFSEL, n)
#define REG_MIC_BIASA_LPFSEL_CHANGE_SHIFT   0
#define REG_MIC_BIASA_LPFSEL_CHANGE_MASK    (0x7 << REG_MIC_BIASA_LPFSEL_CHANGE_SHIFT)
#define REG_MIC_BIASA_LPFSEL_CHANGE(n)      BITFIELD_VAL(REG_MIC_BIASA_LPFSEL_CHANGE, n)

// REG_E3
#define REG_REFRESH_MODE_EN                 (1 << 15)
#define REG_BG_EN_REFRESH_DR                (1 << 13)
#define REG_BG_EN_REFRESH                   (1 << 12)
#define REG_REFRESH_CYCLE_CNT_SHIFT         0
#define REG_REFRESH_CYCLE_CNT_MASK          (0xFFF << REG_REFRESH_CYCLE_CNT_SHIFT)
#define REG_REFRESH_CYCLE_CNT(n)            BITFIELD_VAL(REG_REFRESH_CYCLE_CNT, n)

// REG_E4
#define REG_VBG_SEL_DLY_TIME_SHIFT          12
#define REG_VBG_SEL_DLY_TIME_MASK           (0xF << REG_VBG_SEL_DLY_TIME_SHIFT)
#define REG_VBG_SEL_DLY_TIME(n)             BITFIELD_VAL(REG_VBG_SEL_DLY_TIME, n)
#define REG_REFRESH_OFF_DLY_CNT_SHIFT       8
#define REG_REFRESH_OFF_DLY_CNT_MASK        (0x3 << REG_REFRESH_OFF_DLY_CNT_SHIFT)
#define REG_REFRESH_OFF_DLY_CNT(n)          BITFIELD_VAL(REG_REFRESH_OFF_DLY_CNT, n)
#define REG_REFRESH_ON_DLY_CNT_SHIFT        5
#define REG_REFRESH_ON_DLY_CNT_MASK         (0x7 << REG_REFRESH_ON_DLY_CNT_SHIFT)
#define REG_REFRESH_ON_DLY_CNT(n)           BITFIELD_VAL(REG_REFRESH_ON_DLY_CNT, n)
#define REG_REFRESH_ON_CNT_SHIFT            0
#define REG_REFRESH_ON_CNT_MASK             (0x1F << REG_REFRESH_ON_CNT_SHIFT)
#define REG_REFRESH_ON_CNT(n)               BITFIELD_VAL(REG_REFRESH_ON_CNT, n)

// REG_E5
#define REG_LP_BIAS_SEL_LDO_REG_SHIFT       12
#define REG_LP_BIAS_SEL_LDO_REG_MASK        (0xF << REG_LP_BIAS_SEL_LDO_REG_SHIFT)
#define REG_LP_BIAS_SEL_LDO_REG(n)          BITFIELD_VAL(REG_LP_BIAS_SEL_LDO_REG, n)
#define REG_LP_BIAS_LDO_VBIT_RC_SHIFT       8
#define REG_LP_BIAS_LDO_VBIT_RC_MASK        (0xF << REG_LP_BIAS_LDO_VBIT_RC_SHIFT)
#define REG_LP_BIAS_LDO_VBIT_RC(n)          BITFIELD_VAL(REG_LP_BIAS_LDO_VBIT_RC, n)
#define REG_LP_BIAS_LDO_VBIT_DSLEEP_SHIFT   4
#define REG_LP_BIAS_LDO_VBIT_DSLEEP_MASK    (0xF << REG_LP_BIAS_LDO_VBIT_DSLEEP_SHIFT)
#define REG_LP_BIAS_LDO_VBIT_DSLEEP(n)      BITFIELD_VAL(REG_LP_BIAS_LDO_VBIT_DSLEEP, n)
#define REG_LP_BIAS_LDO_VBIT_NORMAL_SHIFT   0
#define REG_LP_BIAS_LDO_VBIT_NORMAL_MASK    (0xF << REG_LP_BIAS_LDO_VBIT_NORMAL_SHIFT)
#define REG_LP_BIAS_LDO_VBIT_NORMAL(n)      BITFIELD_VAL(REG_LP_BIAS_LDO_VBIT_NORMAL, n)

// REG_E6
#define REG_SAR_VREG_SEL_SHIFT              8
#define REG_SAR_VREG_SEL_MASK               (0x7F << REG_SAR_VREG_SEL_SHIFT)
#define REG_SAR_VREG_SEL(n)                 BITFIELD_VAL(REG_SAR_VREG_SEL, n)
#define REG_SAR_EN_VREF_CALIB               (1 << 6)
#define REG_SAR_PD_VREF_ADC_SW              (1 << 5)
#define REG_VBUCK_RAMP_STEP_SHIFT           0
#define REG_VBUCK_RAMP_STEP_MASK            (0x1F << REG_VBUCK_RAMP_STEP_SHIFT)
#define REG_VBUCK_RAMP_STEP(n)              BITFIELD_VAL(REG_VBUCK_RAMP_STEP, n)

// REG_E7
#define REG_SAR_OFFSET_BIT_VREF_SHIFT       10
#define REG_SAR_OFFSET_BIT_VREF_MASK        (0x3F << REG_SAR_OFFSET_BIT_VREF_SHIFT)
#define REG_SAR_OFFSET_BIT_VREF(n)          BITFIELD_VAL(REG_SAR_OFFSET_BIT_VREF, n)

#define REG_RFPA_3P3_RES_SHIFT              2
#define REG_RFPA_3P3_RES_MASK               (0xF << REG_RFPA_3P3_RES_SHIFT)
#define REG_RFPA_3P3_RES(n)                 BITFIELD_VAL(REG_RFPA_3P3_RES, n)
#define REG_RFPA_ON_B                       (1 << 1)
#define REG_RFPA_3P3                        (1 << 0)

// REG_EA
#define REG_EN_SW1P8_PSRAM_DR               (1 << 13)
#define REG_EN_SW1P8_PSRAM_DLY              (1 << 12)
#define REG_EN_SW1P8_PSRAM                  (1 << 11)
#define REG_SW_DLY_CNT_DR                   (1 << 10)
#define REG_EN_SW_DLY_SEL_SHIFT             7
#define REG_EN_SW_DLY_SEL_MASK              (0x7 << REG_EN_SW_DLY_SEL_SHIFT)
#define REG_EN_SW_DLY_SEL(n)                BITFIELD_VAL(REG_EN_SW_DLY_SEL, n)
#define REG_EN_SW_DLY_CNT_SHIFT             0
#define REG_EN_SW_DLY_CNT_MASK              (0x7F << REG_EN_SW_DLY_CNT_SHIFT)
#define REG_EN_SW_DLY_CNT(n)                BITFIELD_VAL(REG_EN_SW_DLY_CNT, n)

// REG_EB
#define REG_SW1P8_PSRAM_PULL_DOWN_SHIFT     4
#define REG_SW1P8_PSRAM_PULL_DOWN_MASK      (0x3 << REG_SW1P8_PSRAM_PULL_DOWN_SHIFT)
#define REG_SW1P8_PSRAM_PULL_DOWN(n)        BITFIELD_VAL(REG_SW1P8_PSRAM_PULL_DOWN, n)
#define REG_SW1P8_PSRAM_ISEL_SHIFT          0
#define REG_SW1P8_PSRAM_ISEL_MASK           (0xF << REG_SW1P8_PSRAM_ISEL_SHIFT)
#define REG_SW1P8_PSRAM_ISEL(n)             BITFIELD_VAL(REG_SW1P8_PSRAM_ISEL, n)

// REG_EC
#define REG_VRTC_RAMP_EN                    (1 << 13)
#define REG_POWER_RTC_CNT_SHIFT             8
#define REG_POWER_RTC_CNT_MASK              (0x1F << REG_POWER_RTC_CNT_SHIFT)
#define REG_POWER_RTC_CNT(n)                BITFIELD_VAL(REG_POWER_RTC_CNT, n)
#define REG_VRTC_PWR_DOWN_SHIFT             4
#define REG_VRTC_PWR_DOWN_MASK              (0xF << REG_VRTC_PWR_DOWN_SHIFT)
#define REG_VRTC_PWR_DOWN(n)                BITFIELD_VAL(REG_VRTC_PWR_DOWN, n)
#define REG_VRTC_PWR_UP_SHIFT               0
#define REG_VRTC_PWR_UP_MASK                (0xF << REG_VRTC_PWR_UP_SHIFT)
#define REG_VRTC_PWR_UP(n)                  BITFIELD_VAL(REG_VRTC_PWR_UP, n)

#endif
