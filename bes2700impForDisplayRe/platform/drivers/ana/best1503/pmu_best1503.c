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
#include "pmu.h"
#include CHIP_SPECIFIC_HDR(reg_pmu)
#include "analog.h"
#include "cmsis.h"
#include "cmsis_nvic.h"
#include "hal_aud.h"
#include "hal_bootmode.h"
#include "hal_cache.h"
#include "hal_chipid.h"
#include "hal_cmu.h"
#include "hal_location.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "patch.h"
#include "tgt_hardware.h"
#include CHIP_SPECIFIC_HDR(charger)
#include CHIP_SPECIFIC_HDR(reg_charger)

#define PMU_DCDC_SIMO_ENABLE
#define PMU_EFUSE_INIT_IN_BOOT
#define CALIB_POWER_OFF_LPO
// #define PMU_EFUSE_CFG_EMA_VCORE

#ifdef PMU_DCDC_SIMO_ENABLE
#if defined(LDO_MODE) ^ defined(HPPA_LDO_ON)
#error "Invalid config! DCDC SIMO is enabled so VCORE VANA VCODEC should use same power mode!"
#endif
#endif

#ifndef BBPLL_FREQ_MHZ
#define BBPLL_FREQ_MHZ                      384
#endif

#ifdef PMU_IRQ_UNIFIED
#define PMU_IRQ_HDLR_PARAM              uint16_t irq_status
#else
#define PMU_IRQ_HDLR_PARAM              void
#endif

#define PMU_NTC_CTRL_ENABLE

#ifndef PMU_DCDC_PWM
#error "Invalid config! Need define PMU_DCDC_PWM"
#endif

#define ana_read(reg,val)               hal_analogif_reg_read(ANA_REG(reg),val)
#define ana_write(reg,val)              hal_analogif_reg_write(ANA_REG(reg),val)
#define rf_read(reg,val)                hal_analogif_reg_read(RF_REG(reg),val)
#define rf_write(reg,val)               hal_analogif_reg_write(RF_REG(reg),val)
#define chg_read(reg,val)               hal_analogif_reg_read(CHG_REG(reg),val)
#define chg_write(reg,val)              hal_analogif_reg_write(CHG_REG(reg),val)

// LDO soft start interval is about 1000 us
#define PMU_LDO_PU_STABLE_TIME_US       1800
#define PMU_LDO_PULLDOWN_STABLE_TIME_US 1000
#define PMU_DCDC_PU_STABLE_TIME_US      100
#define PMU_VCODEC_STABLE_TIME_US       10
#define PMU_VANA_STABLE_TIME_US         10
#define PMU_VCORE_STABLE_TIME_US        10
#define PMU_BIG_BG_STABLE_TIME_US       200

#ifdef __PMU_VIO_DYNAMIC_CTRL_MODE__
#define IO_VOLT_ACTIVE_NORMAL           PMU_IO_2_6V
#else
#define IO_VOLT_ACTIVE_NORMAL           PMU_IO_2_6V
#endif
#ifdef DIGMIC_HIGH_VOLT
#define IO_VOLT_ACTIVE_RISE             PMU_IO_3_0V
#else
#define IO_VOLT_ACTIVE_RISE             PMU_IO_2_8V
#endif
#define IO_VOLT_SLEEP                   PMU_IO_2_6V

#define PMU_DCDC_ANA_1_8V               0x2C
#define PMU_DCDC_ANA_1_7V               0x24
#define PMU_DCDC_ANA_1_6V               0x1C
#define PMU_DCDC_ANA_1_5V               0x14
#define PMU_DCDC_ANA_1_4V               0xC
#define PMU_DCDC_ANA_1_35V              0x8
#define PMU_DCDC_ANA_1_3V               0x4
#define PMU_DCDC_ANA_1_25V              0x0

#define PMU_DCDC_DIG_1_1V               0x40
#define PMU_DCDC_DIG_1_05V              0x39
#define PMU_DCDC_DIG_1_0V               0x33
#define PMU_DCDC_DIG_0_95V              0x2C
#define PMU_DCDC_DIG_0_9V               0x26
#define PMU_DCDC_DIG_0_85V              0x1F
#define PMU_DCDC_DIG_0_8V               0x18
#define PMU_DCDC_DIG_0_785V             0x16
#define PMU_DCDC_DIG_0_77V              0x14
#define PMU_DCDC_DIG_0_75V              0x12
#define PMU_DCDC_DIG_0_7V               0xC
#define PMU_DCDC_DIG_0_65V              0x6
#define PMU_DCDC_DIG_DEFAULT            0x18 /* 0.8V */

#define PMU_VDIG_1_35V                  0xF
#define PMU_VDIG_1_3V                   0xE
#define PMU_VDIG_1_25V                  0xD
#define PMU_VDIG_1_2V                   0xC
#define PMU_VDIG_1_15V                  0xB
#define PMU_VDIG_1_1V                   0xA
#define PMU_VDIG_1_05V                  0x9
#define PMU_VDIG_1_0V                   0x8
#define PMU_VDIG_0_95V                  0x7
#define PMU_VDIG_0_9V                   0x6
#define PMU_VDIG_0_85V                  0x5
#define PMU_VDIG_0_8V                   0x4
#define PMU_VDIG_0_75V                  0x3
#define PMU_VDIG_0_7V                   0x2
#define PMU_VDIG_0_65V                  0x1
#define PMU_VDIG_0_6V                   0x0
#define PMU_VDIG_MAX                    PMU_VDIG_1_2V
#define PMU_VDIG_DEFAULT                0x4 /* 0.8V */

#define PMU_LDO_SENSOR_3_3V             0x2A
#define PMU_LDO_SENSOR_3_2V             0x28
#define PMU_LDO_SENSOR_3_1V             0x26
#define PMU_LDO_SENSOR_3_0V             0x24
#define PMU_LDO_SENSOR_2_9V             0x22
#define PMU_LDO_SENSOR_2_8V             0x20
#define PMU_LDO_SENSOR_2_7V             0x1E
#define PMU_LDO_SENSOR_2_6V             0x1C
#define PMU_LDO_SENSOR_2_5V             0x1A
#define PMU_LDO_SENSOR_2_4V             0x18
#define PMU_LDO_SENSOR_2_3V             0x16
#define PMU_LDO_SENSOR_2_2V             0x14
#define PMU_LDO_SENSOR_2_1V             0x12
#define PMU_LDO_SENSOR_2_0V             0x10
#define PMU_LDO_SENSOR_1_9V             0xE
#define PMU_LDO_SENSOR_1_8V             0xC
#define PMU_LDO_SENSOR_1_7V             0xA
#define PMU_LDO_SENSOR_1_6V             0x8

#ifdef PMU_HIGH_VPA
#define PMU_LDO_PA_MAX                  PMU_LDO_PA_3_3V
#define PMU_LDO_PA_3_3V                 0xC
#define PMU_LDO_PA_3_2V                 0xB
#else
#define PMU_LDO_PA_MAX                  PMU_LDO_PA_3_1V
#endif

#define PMU_LDO_PA_3_1V                 0xA
#define PMU_LDO_PA_3_0V                 0x9
#define PMU_LDO_PA_2_9V                 0x8
#define PMU_LDO_PA_2_8V                 0x7
#define PMU_LDO_PA_2_7V                 0x6
#define PMU_LDO_PA_2_6V                 0x5
#define PMU_LDO_PA_2_5V                 0x4
#define PMU_LDO_PA_2_4V                 0x3
#define PMU_LDO_PA_2_3V                 0x2
#define PMU_LDO_PA_2_2V                 0x1
#define PMU_LDO_PA_2_1V                 0x0

#define PMU_VMEM_2_8V                   0x10
#define PMU_VMEM_2_0V                   0x8
#define PMU_VMEM_1_9V                   0x7
#define PMU_VMEM_1_8V                   0x6

#define PMU_VMIC_2_8V                   0x30
#define PMU_VMIC_1_6V                   0x13

#define PMU_VMIC_RES_3_3V               0xF
#define PMU_VMIC_RES_2_8V               0xA

#define PMU_CODEC_2_8V                  0x16
#define PMU_CODEC_2_7V                  0xF
#define PMU_CODEC_2_6V                  0xE
#define PMU_CODEC_2_5V                  0xD
#define PMU_CODEC_2_4V                  0xC
#define PMU_CODEC_2_3V                  0xB
#define PMU_CODEC_2_2V                  0xA
#define PMU_CODEC_2_1V                  0x9
#define PMU_CODEC_2_0V                  0x8
#define PMU_CODEC_1_9V                  0x7
#define PMU_CODEC_1_8V                  0x6
#define PMU_CODEC_1_7V                  0x5
#define PMU_CODEC_1_6V                  0x4
#define PMU_CODEC_1_5V                  0x3

#define PMU_USB_3_3V                    0xC
#define PMU_USB_3_2V                    0xB
#define PMU_USB_3_1V                    0xA
#define PMU_USB_3_0V                    0x9
#define PMU_USB_2_9V                    0x8
#define PMU_USB_2_8V                    0x7
#define PMU_USB_2_7V                    0x6
#define PMU_USB_2_6V                    0x5
#define PMU_USB_2_5V                    0x4
#define PMU_USB_2_4V                    0x3

#define PMU_DCDC_HPPA_2_1V              0x38
#define PMU_DCDC_HPPA_2_0V              0x34
#define PMU_DCDC_HPPA_1_95V             0x30
#define PMU_DCDC_HPPA_1_9V              0x2D
#define PMU_DCDC_HPPA_1_8V              0x26
#define PMU_DCDC_HPPA_1_75V             0x23
#define PMU_DCDC_HPPA_1_7V              0x20
#define PMU_DCDC_HPPA_1_65V             0x1C
#define PMU_DCDC_HPPA_1_6V              0x19
#define PMU_DCDC_HPPA_1_5V              0x12
#define PMU_DCDC_HPPA_1_45V             0xE
#define PMU_DCDC_HPPA_1_4V              0xB
#define PMU_DCDC_HPPA_1_35V             0x7
#define PMU_DCDC_HPPA_1_3V              0x4

#define MAX_BUCK_VANA_BIT_VAL           (BUCK_VANA_BIT_NORMAL_MASK >> BUCK_VANA_BIT_NORMAL_SHIFT)
#define MAX_BUCK_VCORE_BIT_VAL          (BUCK_VCORE_BIT_NORMAL_MASK >> BUCK_VCORE_BIT_NORMAL_SHIFT)
#define MAX_BUCK_VHPPA_BIT_VAL          (REG_BUCK_HPPA_VBIT_NORMAL_MASK >> REG_BUCK_HPPA_VBIT_NORMAL_SHIFT)

#define INTR_MSKED_CHARGE_OUT           INTR_MSKED_CHARGE(1 << 1)
#define INTR_MSKED_CHARGE_IN            INTR_MSKED_CHARGE(1 << 0)

#define GPADC_VSYS_CALIB_VAL_A_DEFAULT                  16320
#define GPADC_VSYS_CALIB_VAL_B_DEFAULT                  21480

#define GPADC_CALIB_VAL_SLIM_A_BASE                     12000
#define GPADC_CALIB_VAL_SLIM_B_BASE                     16200

#define GPADC_CALIB_VAL_SLIM_A_LP_BASE                  12000
#define GPADC_CALIB_VAL_SLIM_B_LP_BASE                  3700

#define GPADC_CALIB_VAL_SLIM_COMP_SHIFT                 0
#define GPADC_CALIB_VAL_SLIM_COMP_MASK                  (0x7FF << GPADC_CALIB_VAL_SLIM_COMP_SHIFT)
#define GPADC_CALIB_VAL_SLIM_COMP(n)                    BITFIELD_VAL(GPADC_CALIB_VAL_SLIM_COMP, n)
#define GPADC_CALIB_VAL_SLIM_COMP_LP_LV_SHIFT           0
#define GPADC_CALIB_VAL_SLIM_COMP_LP_LV_MASK            (0xFFF << GPADC_CALIB_VAL_SLIM_COMP_LP_LV_SHIFT)
#define GPADC_CALIB_VAL_SLIM_COMP_LP_LV(n)              BITFIELD_VAL(GPADC_CALIB_VAL_SLIM_COMP_LP_LV, n)
#define GPADC_CALIB_VAL_SLIM_COMP_LP_SHIFT              11
#define GPADC_CALIB_VAL_SLIM_COMP_LP_MASK               (0x1F << GPADC_CALIB_VAL_SLIM_COMP_LP_SHIFT)
#define GPADC_CALIB_VAL_SLIM_COMP_LP(n)                 BITFIELD_VAL(GPADC_CALIB_VAL_SLIM_COMP_LP, n)

// PMU_EFUSE_PAGE_SW_CFG 0x5
#define PMU_EFUSE_5_TEMPERATURE_REFERENCE_SHIFT         0
#define PMU_EFUSE_5_TEMPERATURE_REFERENCE_MASK          (0xFFF << PMU_EFUSE_5_TEMPERATURE_REFERENCE_SHIFT)
#define PMU_EFUSE_5_TEMPERATURE_REFERENCE(n)            BITFIELD_VAL(PMU_EFUSE_5_TEMPERATURE_REFERENCE, n)
#define PMU_EFUSE_5_VCORE_MIN_SHIFT                     12
#define PMU_EFUSE_5_VCORE_MIN_MASK                      (0x7 << PMU_EFUSE_5_VCORE_MIN_SHIFT)
#define PMU_EFUSE_5_VCORE_MIN(n)                        BITFIELD_VAL(PMU_EFUSE_5_VCORE_MIN, n)
#define PMU_EFUSE_5_GPADC_CALIB_CH6_VAL_SLIM            (1 << 15)

// PMU_EFUSE_PAGE_RESERVED_7
#define PMU_EFUSE_7_EMA_CFG_MSB                         (1 << 13)
#define PMU_EFUSE_7_NTC_REF_CALIB_NORMAL_BG_FLAG        (1 << 15)

// PMU_EFUSE_PAGE_CP_XY_COORDINATE 0xA
#define PMU_EFUSE_CHIP_WAFER_COORDINATE_X_SHIFT         0
#define PMU_EFUSE_CHIP_WAFER_COORDINATE_X_MASK          (0x7F << PMU_EFUSE_CHIP_WAFER_COORDINATE_X_SHIFT)
#define PMU_EFUSE_CHIP_WAFER_COORDINATE_X(n)            BITFIELD_VAL(PMU_EFUSE_CHIP_WAFER_COORDINATE_X, n)
#define PMU_EFUSE_CHIP_WAFER_COORDINATE_Y_SHIFT         7
#define PMU_EFUSE_CHIP_WAFER_COORDINATE_Y_MASK          (0x7F << PMU_EFUSE_CHIP_WAFER_COORDINATE_Y_SHIFT)
#define PMU_EFUSE_CHIP_WAFER_COORDINATE_Y(n)            BITFIELD_VAL(PMU_EFUSE_CHIP_WAFER_COORDINATE_Y, n)

// PMU_EFUSE_PAGE_CP_WAFER_ID 0xB
#define PMU_EFUSE_BURST_THRESHOLD_CAL_SHIFT             5
#define PMU_EFUSE_BURST_THRESHOLD_CAL_MASK              (0x3F << PMU_EFUSE_BURST_THRESHOLD_CAL_SHIFT)
#define PMU_EFUSE_BURST_THRESHOLD_CAL(n)                BITFIELD_VAL(PMU_EFUSE_BURST_THRESHOLD_CAL, n)
#define PMU_EFUSE_CHIP_WAFER_ID_SHIFT                   0
#define PMU_EFUSE_CHIP_WAFER_ID_MASK                    (0x1F << PMU_EFUSE_CHIP_WAFER_ID_SHIFT)
#define PMU_EFUSE_CHIP_WAFER_ID(n)                      BITFIELD_VAL(PMU_EFUSE_CHIP_WAFER_ID, n)

// PMU_EFUSE_PAGE_RESERVED_C
#define PMU_EFUSE_C_VANA_DCDC_COMP_FIX_RF_SHIFT         1
#define PMU_EFUSE_C_VANA_DCDC_COMP_FIX_RF_MASK          (0x7 << PMU_EFUSE_C_VANA_DCDC_COMP_FIX_RF_SHIFT)
#define PMU_EFUSE_C_VANA_DCDC_COMP_FIX_RF(n)            BITFIELD_VAL(PMU_EFUSE_C_VANA_DCDC_COMP_FIX_RF, n)
#define PMU_EFUSE_C_EMA_CFG_LSB                         (1 << 13)
#define PMU_EFUSE_C_FLASH_VDD_TYPE_SHIFT                14
#define PMU_EFUSE_C_FLASH_VDD_TYPE_MASK                 (0x3 << PMU_EFUSE_C_FLASH_VDD_TYPE_SHIFT)
#define PMU_EFUSE_C_FLASH_VDD_TYPE(n)                   BITFIELD_VAL(PMU_EFUSE_C_FLASH_VDD_TYPE, n)

// PMU_EFUSE_PAGE_RESERVED_D
#define PMU_EFUSE_D_DCDC3_COMP_LP_LOW_TGT_SHIFT         12
#define PMU_EFUSE_D_DCDC3_COMP_LP_LOW_TGT_MASK          (0x7 << PMU_EFUSE_D_DCDC3_COMP_LP_LOW_TGT_SHIFT)
#define PMU_EFUSE_D_DCDC3_COMP_LP_LOW_TGT(n)            BITFIELD_VAL(PMU_EFUSE_D_DCDC3_COMP_LP_LOW_TGT, n)
#define PMU_EFUSE_D_DCDC3_COMP_LP_LOW_TGT_SIGN          (1 << 15)

// RF_REG_F5
#define REG_BT_BBPLL_DIVN_CODEC_SHIFT                   0
#define REG_BT_BBPLL_DIVN_CODEC_MASK                    (0x1F << REG_BT_BBPLL_DIVN_CODEC_SHIFT)
#define REG_BT_BBPLL_DIVN_CODEC(n)                      BITFIELD_VAL(REG_BT_BBPLL_DIVN_CODEC, n)
#define REG_RXCALI_REVERSE                              (1 << 5)
#define REG_SIG_LOSS_SHIFT                              6
#define REG_SIG_LOSS_MASK                               (0xF << REG_SIG_LOSS_SHIFT)
#define REG_SIG_LOSS(n)                                 BITFIELD_VAL(REG_SIG_LOSS, n)
#define REG_BT_IPTAT_ISEL_SHIFT                         10
#define REG_BT_IPTAT_ISEL_MASK                          (0x3F << REG_BT_IPTAT_ISEL_SHIFT)
#define REG_BT_IPTAT_ISEL(n)                            BITFIELD_VAL(REG_BT_IPTAT_ISEL, n)

// RF_REG_146
#define REG_LP_BBPLL_INT_DEC_SEL_SHIFT                  3
#define REG_LP_BBPLL_INT_DEC_SEL_MASK                   (0x7 << REG_LP_BBPLL_INT_DEC_SEL_SHIFT)
#define REG_LP_BBPLL_INT_DEC_SEL(n)                     BITFIELD_VAL(REG_LP_BBPLL_INT_DEC_SEL, n)

// RF_REG_148
#define REG_LP_BBPLL_SDM_FREQWORD_15_0_SHIFT            0
#define REG_LP_BBPLL_SDM_FREQWORD_15_0_MASK             (0xFFFF << REG_LP_BBPLL_SDM_FREQWORD_15_0_SHIFT)
#define REG_LP_BBPLL_SDM_FREQWORD_15_0(n)               BITFIELD_VAL(REG_LP_BBPLL_SDM_FREQWORD_15_0, n)

// RF_REG_149
#define REG_LP_BBPLL_SDM_FREQWORD_31_16_SHIFT           0
#define REG_LP_BBPLL_SDM_FREQWORD_31_16_MASK            (0xFFFF << REG_LP_BBPLL_SDM_FREQWORD_31_16_SHIFT)
#define REG_LP_BBPLL_SDM_FREQWORD_31_16(n)              BITFIELD_VAL(REG_LP_BBPLL_SDM_FREQWORD_31_16, n)

// RF_REG_14A
#define REG_LP_BBPLL_SDM_FREQWORD_34_32_SHIFT           0
#define REG_LP_BBPLL_SDM_FREQWORD_34_32_MASK            (0x7 << REG_LP_BBPLL_SDM_FREQWORD_34_32_SHIFT)
#define REG_LP_BBPLL_SDM_FREQWORD_34_32(n)              BITFIELD_VAL(REG_LP_BBPLL_SDM_FREQWORD_34_32, n)

// RF_REG_14C
#define REG_LP_BBPLL_FREQ_EN_COMB                       (1 << 0)
#define REG_LP_BBPLL_TRI_STEP_SEL_SHIFT                 1
#define REG_LP_BBPLL_TRI_STEP_SEL_MASK                  (0x3 << REG_LP_BBPLL_TRI_STEP_SEL_SHIFT)
#define REG_LP_BBPLL_TRI_STEP_SEL(n)                    BITFIELD_VAL(REG_LP_BBPLL_TRI_STEP_SEL, n)
#define REG_LP_BBPLL_TRI_FREQ_SEL_SHIFT                 3
#define REG_LP_BBPLL_TRI_FREQ_SEL_MASK                  (0x3 << REG_LP_BBPLL_TRI_FREQ_SEL_SHIFT)
#define REG_LP_BBPLL_TRI_FREQ_SEL(n)                    BITFIELD_VAL(REG_LP_BBPLL_TRI_FREQ_SEL, n)
#define REG_LP_BBPLL_TRI_FREQ_OFST_SEL_SHIFT            5
#define REG_LP_BBPLL_TRI_FREQ_OFST_SEL_MASK             (0x7 << REG_LP_BBPLL_TRI_FREQ_OFST_SEL_SHIFT)
#define REG_LP_BBPLL_TRI_FREQ_OFST_SEL(n)               BITFIELD_VAL(REG_LP_BBPLL_TRI_FREQ_OFST_SEL, n)
#define REG_LP_BBPLL_SSC_ENABLE                         (1 << 8)
#define REG_LP_BBPLL_ON_DR                              (1 << 9)
#define REG_LP_BBPLL_ON                                 (1 << 10)
#define REG_LP_BBPLL_LDO_PU_DR                          (1 << 11)
#define REG_LP_BBPLL_LDO_PU                             (1 << 12)
#define REG_LP_BBPLL_LDO_PRE_CHARGE_DR                  (1 << 13)
#define REG_LP_BBPLL_LDO_PRE_CHARGE                     (1 << 14)
#define REG_LP_BBPLL_PU_CP_DR                           (1 << 15)

// RF_REG_159
#define REG_XTAL_BUF_RC_OTHER_SHIFT                     0
#define REG_XTAL_BUF_RC_OTHER_MASK                      (0xF << REG_XTAL_BUF_RC_OTHER_SHIFT)
#define REG_XTAL_BUF_RC_OTHER(n)                        BITFIELD_VAL(REG_XTAL_BUF_RC_OTHER, n)
#define REG_XTAL_X2X4_RC_OUT_OTHER_SHIFT                4
#define REG_XTAL_X2X4_RC_OUT_OTHER_MASK                 (0xF << REG_XTAL_X2X4_RC_OUT_OTHER_SHIFT)
#define REG_XTAL_X2X4_RC_OUT_OTHER(n)                   BITFIELD_VAL(REG_XTAL_X2X4_RC_OUT_OTHER, n)
#define REG_XTAL_LDO_VTUNE_OTHER_SHIFT                  8
#define REG_XTAL_LDO_VTUNE_OTHER_MASK                   (0x7 << REG_XTAL_LDO_VTUNE_OTHER_SHIFT)
#define REG_XTAL_LDO_VTUNE_OTHER(n)                     BITFIELD_VAL(REG_XTAL_LDO_VTUNE_OTHER, n)
#define REG_XTAL_IC_RES_SHIFT                           11
#define REG_XTAL_IC_RES_MASK                            (0x7 << REG_XTAL_IC_RES_SHIFT)
#define REG_XTAL_IC_RES(n)                              BITFIELD_VAL(REG_XTAL_IC_RES, n)

// RF_REG_15A
#define REG_XTAL_FDATA1_DR                              (1 << 8)
#define REG_XTAL_FDATA1_SHIFT                           9
#define REG_XTAL_FDATA1_MASK                            (0x1F << REG_XTAL_FDATA1_SHIFT)
#define REG_XTAL_FDATA1(n)                              BITFIELD_VAL(REG_XTAL_FDATA1, n)
#define REG_XTAL_FDATA2_DR                              (1 << 14)

// RF_REG_15B
#define REG_XTAL_FDATA2_SHIFT                           0
#define REG_XTAL_FDATA2_MASK                            (0x1F << REG_XTAL_FDATA2_SHIFT)
#define REG_XTAL_FDATA2(n)                              BITFIELD_VAL(REG_XTAL_FDATA2, n)

// RF_REG_168
#define REG_XTAL_FX4_CAP1_SHIFT                         13
#define REG_XTAL_FX4_CAP1_MASK                          (0x3 << REG_XTAL_FX4_CAP1_SHIFT)
#define REG_XTAL_FX4_CAP1(n)                            BITFIELD_VAL(REG_XTAL_FX4_CAP1, n)

// RF_REG_169
#define REG_XTAL_FX4_CAP2_SHIFT                         14
#define REG_XTAL_FX4_CAP2_MASK                          (0x3 << REG_XTAL_FX4_CAP2_SHIFT)
#define REG_XTAL_FX4_CAP2(n)                            BITFIELD_VAL(REG_XTAL_FX4_CAP2, n)

// RF_REG_16E
#define REG_LP_BBPLL_VCO_SPD_SHIFT                      9
#define REG_LP_BBPLL_VCO_SPD_MASK                       (0x7 << REG_LP_BBPLL_VCO_SPD_SHIFT)
#define REG_LP_BBPLL_VCO_SPD(n)                         BITFIELD_VAL(REG_LP_BBPLL_VCO_SPD, n)


// RF_REG_170
#define REG_LP_BBPLL_EN_CLK_USB                         (1 << 0)
#define REG_LP_BBPLL_EN_CLK_CODEC                       (1 << 1)
#define REG_LP_BBPLL_DIVN_AD_SHIFT                      2
#define REG_LP_BBPLL_DIVN_AD_MASK                       (0x1F << REG_LP_BBPLL_DIVN_AD_SHIFT)
#define REG_LP_BBPLL_DIVN_AD(n)                         BITFIELD_VAL(REG_LP_BBPLL_DIVN_AD, n)
#define REG_LP_BBPLL_DIG_CLK_FREQ_SEL                   (1 << 7)
#define REG_LP_BBPLL_EN_CLK_DIG                         (1 << 8)
#define REG_LP_BBPLL_EN_CLK_BBADC                       (1 << 9)
#define REG_LP_BBPLL_TST_BUF_DRV_SEL_SHIFT              10
#define REG_LP_BBPLL_TST_BUF_DRV_SEL_MASK               (0x3 << REG_LP_BBPLL_TST_BUF_DRV_SEL_SHIFT)
#define REG_LP_BBPLL_TST_BUF_DRV_SEL(n)                 BITFIELD_VAL(REG_LP_BBPLL_TST_BUF_DRV_SEL, n)
#define REG_LP_BBPLL_TST_BUF_PWR_SEL_SHIFT              12
#define REG_LP_BBPLL_TST_BUF_PWR_SEL_MASK               (0x3 << REG_LP_BBPLL_TST_BUF_PWR_SEL_SHIFT)
#define REG_LP_BBPLL_TST_BUF_PWR_SEL(n)                 BITFIELD_VAL(REG_LP_BBPLL_TST_BUF_PWR_SEL, n)

// CHG_REG_TEMP_BG_CFG
#define REG_BG_CORE_EN                                  (1 << 0)
#define REG_BG_CORE_EN_DR                               (1 << 1)
#define REG_BG_REF_GEN_EN                               (1 << 2)
#define REG_BG_REF_GEN_EN_DR                            (1 << 3)
#define REG_BG_EN_OTP                                   (1 << 4)
#define REG_BG_EN_OTP_DR                                (1 << 5)

#define PMU_EFUSE_TEMPSENSOR_BASE                       22000
#define PMU_EFUSE_TEMPSENSOR_DEFAULT                    23782

#define PMU_NTC_RAW_DATA_TO_ONE_CENTIGRADE              80
#define PMU_NTC_IRQ_TIMEOUT_MS                          5

#ifdef CALIB_POWER_OFF_LPO
#define PMU_CONFIG_SYSTICK_HZ_NOMINAL                   (16000)
#define PMU_MAX_CALIB_SYSTICK_HZ                        (PMU_CONFIG_SYSTICK_HZ_NOMINAL * 2)
#define PMU_MIN_CALIB_TICKS                             (30 * (PMU_CONFIG_SYSTICK_HZ_NOMINAL / 1000))
#define PMU_MAX_CALIB_TICKS                             (30 * PMU_CONFIG_SYSTICK_HZ_NOMINAL)
#ifdef FAST_TIMER_COMPENSATE
#define PMU_FAST_TICK_RATIO_NUM                         3
#endif
#endif

enum CHG_RF_ANA_REG_T {
    ANA_REG_00                 = 0x00,

    RF_REG_00                  = 0x00,
    RF_REG_F5                  = 0xF5,
    RF_REG_146                 = 0x146,
    RF_REG_148                 = 0x148,
    RF_REG_149                 = 0x149,
    RF_REG_14A                 = 0x14A,
    RF_REG_14C                 = 0x14C,
    RF_REG_159                 = 0x159,
    RF_REG_15A                 = 0x15A,
    RF_REG_15B                 = 0x15B,
    RF_REG_168                 = 0x168,
    RF_REG_169                 = 0x169,
    RF_REG_16E                 = 0x16E,
    RF_REG_170                 = 0x170,
};

enum PMU_VCORE_REQ_T {
    PMU_VCORE_FLASH_WRITE_ENABLED   = (1 << 0),
    PMU_VCORE_FLASH_FREQ_HIGH       = (1 << 1),
    PMU_VCORE_USB_HS_ENABLED        = (1 << 2),
    PMU_VCORE_SYS_FREQ_MEDIUM_LOW   = (1 << 3),
    PMU_VCORE_SYS_FREQ_MEDIUM       = (1 << 4),
    PMU_VCORE_SYS_FREQ_HIGH         = (1 << 5),
    PMU_VCORE_RS_FREQ_HIGH          = (1 << 6),
    PMU_VCORE_RS_ADC_FREQ_HIGH      = (1 << 7),
    PMU_VCORE_IIR_FREQ_HIGH         = (1 << 8),
    PMU_VCORE_IIR_EQ_FREQ_HIGH      = (1 << 9),
    PMU_VCORE_FIR_FREQ_HIGH         = (1 << 10),
    PMU_VCORE_PMU_RESET             = (1 << 11),
    PMU_VCORE_NTC_COOL              = (1 << 12),
    PMU_VCORE_NTC_COLD              = (1 << 13),
    PMU_VCORE_NTC_CHILLY            = (1 << 14),
    PMU_VCORE_NTC_FREEZING          = (1 << 15),
};

enum PMU_VUSB_REQ_T {
    PMU_VUSB_REQ_INIT               = (1 << 0),
    PMU_VUSB_REQ_USB                = (1 << 1),
    PMU_VUSB_REQ_LBRT               = (1 << 2),
};

union BOOT_SETTINGS_T {
    struct {
        unsigned short usb_dld_dis      :1;
        unsigned short uart_dld_en      :1;
        unsigned short uart_trace_en    :1;
        unsigned short pll_dis          :1;
        unsigned short uart_baud_div2   :1;
        unsigned short sec_freq_div2    :1;
        unsigned short crystal_freq     :2;
        unsigned short timeout_div      :2;
        unsigned short uart_connected   :1;
        unsigned short uart_1p8v        :1;
        unsigned short chksum           :4;
    };
    unsigned short reg;
};

enum PMU_MODUAL_T {
    PMU_ANA,
    PMU_DIG,
    PMU_SENSOR,
    PMU_CODEC,
    PMU_PA,

    PMU_MODULE_QTY,
};

struct PMU_MODULE_CFG_T {
    unsigned short manual_bit;
    unsigned short ldo_en;
    unsigned short lp_en_dr;
    unsigned short lp_en;
    unsigned short dsleep_mode;
    unsigned short dsleep_v;
    unsigned short dsleep_v_shift;
    unsigned short normal_v;
    unsigned short normal_v_shift;
};

#define PMU_MOD_CFG_VAL(m)              { \
    REG_PU_LDO_V##m##_DR, REG_PU_LDO_V##m##_REG, \
    LP_EN_V##m##_LDO_DR, LP_EN_V##m##_LDO_REG, \
    REG_PU_LDO_V##m##_DSLEEP, \
    LDO_V##m##_VBIT_DSLEEP_MASK, LDO_V##m##_VBIT_DSLEEP_SHIFT,\
    LDO_V##m##_VBIT_NORMAL_MASK, LDO_V##m##_VBIT_NORMAL_SHIFT }

static const struct PMU_MODULE_CFG_T pmu_module_cfg[] = {
    PMU_MOD_CFG_VAL(ANA),
    PMU_MOD_CFG_VAL(DIG),
    PMU_MOD_CFG_VAL(SENSOR),
    PMU_MOD_CFG_VAL(CODEC),
    PMU_MOD_CFG_VAL(PA),
};

#define OPT_TYPE                        const

static OPT_TYPE POSSIBLY_UNUSED bool vcodec_off =
#ifdef VCODEC_OFF
    true;
#else
    false;
#endif

static OPT_TYPE POSSIBLY_UNUSED uint16_t vcodec_mv = (uint16_t)(VCODEC_VOLT * 1000);
static OPT_TYPE POSSIBLY_UNUSED uint16_t vhppa_mv = (uint16_t)(VHPPA_VOLT * 1000);

static OPT_TYPE uint8_t ana_act_dcdc = PMU_DCDC_ANA_1_3V;
static OPT_TYPE uint8_t ana_lp_dcdc = ana_act_dcdc;

static OPT_TYPE uint8_t dig_lp_dcdc = PMU_DCDC_DIG_0_65V;
static OPT_TYPE uint8_t dig_lp_ldo = PMU_VDIG_0_65V;

static enum PMU_POWER_MODE_T BOOT_DATA_LOC pmu_power_mode = PMU_POWER_MODE_NONE;

static enum PMU_VCORE_REQ_T BOOT_BSS_LOC pmu_vcore_req;

static PMU_CHARGER_IRQ_HANDLER_T charger_irq_handler;

#ifdef CHG_PRESENT_ENABLE
static PMU_CHARGER_PRESENT_IRQ_HANDLER_T charger_present_irq_handler;
static void pmu_charger_present_init(void);
#endif

#ifdef PMU_IRQ_UNIFIED
static bool gpio_irq_en[2];
static HAL_GPIO_PIN_IRQ_HANDLER gpio_irq_handler[2];

static PMU_WDT_IRQ_HANDLER_T wdt_irq_handler;

static PMU_IRQ_UNIFIED_HANDLER_T pmu_irq_hdlrs[PMU_IRQ_TYPE_QTY];
#endif

static uint16_t SRAM_BSS_DEF(dcdc_ramp_map);

// Move all the data/bss invovled in pmu_open() to .sram_data/.sram_bss,
// so that pmu_open() can be called at the end of BootInit(),
// for data/bss is initialized after BootInit().

static uint8_t BOOT_DATA_LOC bbpll_codec_div = 32;

static uint32_t BOOT_BSS_LOC pmu_metal_id;

static uint32_t BOOT_BSS_LOC mcu_metal_id;

static uint16_t wdt_irq_timer;
static uint16_t wdt_reset_timer;

static bool shipment_en = false;

static bool gpadc_calib_vbat_slim = false;

static enum PMU_VSENSOR_USER_T vsensor_map;

static uint16_t ntc_raw = 0;
static int ntc_temperature = 0;
static bool ntc_irq_busy = false;
static PMU_NTC_IRQ_HANDLER_T pmu_ntc_cb;

#if defined(MCU_HIGH_PERFORMANCE_MODE)
static const uint16_t high_perf_freq_mhz =
#if defined(MTEST_ENABLED) && defined(MTEST_CLK_MHZ)
    MTEST_CLK_MHZ;
#else
    300;
#endif
static bool high_perf_on;
#endif

#ifdef PMU_FORCE_LP_MODE
static bool pmu_force_lp_bg_valid;
static bool pmu_force_lp_bg_init_done = false;
static enum PMU_BIG_BANDGAP_USER_T big_bandgap_user_map;
static void pmu_bandgap_cfg_reset(void);
#endif

static enum PMU_BOOT_CAUSE_T BOOT_BSS_LOC pmu_boot_reason;

static bool BOOT_BSS_LOC pmu_boot_first_pwr_up = false;

#if defined(PMU_EFUSE_CFG_EMA_VCORE) && defined(PMU_SAFE_VOLT_MIN)
static uint8_t BOOT_BSS_LOC pmu_ema_cfg;
#endif

#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
#ifdef ARM_CMSE
static bool BOOT_BSS_LOC pmu_ns_running;
#endif
#ifndef HPPA_LDO_ON
static void pmu_hppa_dcdc_to_ldo(void);
#endif

#if !defined(ROM_BUILD) && defined(PMU_EFUSE_INIT_IN_BOOT)
static bool BOOT_BSS_LOC efuse_buf_valid;
static unsigned short pmu_efuse_page_arrays[PMU_EFUSE_PAGE_QTY];
#endif
#endif

#if defined(_AUTO_TEST_)
static bool at_skip_shutdown = false;

void pmu_at_skip_shutdown(bool enable)
{
    at_skip_shutdown = enable;
}
#endif

#ifdef RTC_ENABLE
struct PMU_RTC_CTX_T {
    bool enabled;
    bool alarm_set;
    uint32_t alarm_val;
};

static struct PMU_RTC_CTX_T BOOT_BSS_LOC rtc_ctx;

static PMU_RTC_IRQ_HANDLER_T rtc_irq_handler;

static void BOOT_TEXT_SRAM_LOC pmu_rtc_save_context(void)
{
    if (pmu_rtc_enabled()) {
        rtc_ctx.enabled = true;
        if (pmu_rtc_alarm_status_set()) {
            rtc_ctx.alarm_set = true;
            rtc_ctx.alarm_val = pmu_rtc_get_alarm();
        }
    } else {
        rtc_ctx.enabled = false;
    }
}

static void pmu_rtc_restore_context(void)
{
    uint32_t rtc_val;

    if (rtc_ctx.enabled) {
        pmu_rtc_enable();
        if (rtc_ctx.alarm_set) {
            rtc_val = pmu_rtc_get();
            if (rtc_val - rtc_ctx.alarm_val <= 1 || rtc_ctx.alarm_val - rtc_val < 5) {
                rtc_ctx.alarm_val = rtc_val + 5;
            }
            pmu_rtc_set_alarm(rtc_ctx.alarm_val);
        }
    }
}

#ifdef CALIB_POWER_OFF_LPO
static void pmu_rtc_clk_div_save(void);
static void pmu_rtc_clk_div_reload(void);
static float pmu_sys_tick_hz = PMU_CONFIG_SYSTICK_HZ_NOMINAL;
static uint32_t pmu_slow_val;
static uint32_t pmu_fast_val;
static uint32_t pmu_rtc_div_pwroff;
#endif
#endif

#ifdef PMU_DCDC_CALIB
union VOLT_COMP_T {
    struct VOLT_COMP_FIELD_T {
        uint16_t dcdc1_v: 5; //bit[4:0]: 0 ~ 31
        uint16_t dcdc1_f: 1; //bit[5]  : 1: negative, 0: positive;
        uint16_t dcdc2_v: 4; //bit[9:6]: 0 ~ 15
        uint16_t dcdc2_f: 1; //bit[10] :
        uint16_t dcdc3_v: 4; //bit[14:11]: 0 ~ 15
        uint16_t dcdc3_f: 1; //bit[15]
    } f;
    uint16_t v;
};

static int8_t pmu_dcdc_dig_comp = 0;
static int8_t pmu_dcdc_ana_comp = 0;
static int8_t pmu_dcdc_hppa_comp = 0;

static int8_t pmu_dig_lp_comp  = 0;
static int8_t pmu_ana_lp_comp  = 0;
static int8_t pmu_hppa_lp_comp = 0;

static int8_t pmu_dcdc_safe_volt_vana = 0;

#ifdef PMU_SAFE_VOLT_MIN
static int8_t pmu_dcdc_safe_volt_vcore = 0;

struct PMU_DCDC_VCORE_VMIN_MAP_T {
    uint16_t vcore_vmin_efuse_val; //bit[14:12] 0~7
    int8_t vcore_vmin_dcdc_comp;
};

static const struct PMU_DCDC_VCORE_VMIN_MAP_T pmu_vcore_vmin_map[] = {
    {0x0,   0},
    {0x1,  -6},
    {0x2,  -5},
    {0x3,  -4},
    {0x4,  -2},
    {0x5,  -1},
    {0x6,   0},
    {0x7,   2},
};
#endif

static void pmu_get_dcdc_safe_volt_min(void)
{
    unsigned short val;

    TRACE(0, "%s", __func__);

#ifdef PMU_SAFE_VOLT_MIN
    int i;

#ifdef PMU_EFUSE_CFG_EMA_VCORE
    if (pmu_ema_cfg != 0xff)
#endif
    {
        pmu_get_efuse(PMU_EFUSE_PAGE_SW_CFG, &val);
        val = GET_BITFIELD(val, PMU_EFUSE_5_VCORE_MIN);
        for (i = 0; i < ARRAY_SIZE(pmu_vcore_vmin_map); i++) {
            if (pmu_vcore_vmin_map[i].vcore_vmin_efuse_val == val) {
                pmu_dcdc_safe_volt_vcore = pmu_vcore_vmin_map[i].vcore_vmin_dcdc_comp;
                break;
            }
        }

        TRACE(0, "safe VCORE=%d vmin_efuse_val=0x%x", pmu_dcdc_safe_volt_vcore, pmu_vcore_vmin_map[i].vcore_vmin_efuse_val);
    }
#endif

    pmu_get_efuse(PMU_EFUSE_PAGE_RESERVED_C, &val);
    pmu_dcdc_safe_volt_vana = GET_BITFIELD(val, PMU_EFUSE_C_VANA_DCDC_COMP_FIX_RF) << 1;

    TRACE(0, "safe VANA=%d", pmu_dcdc_safe_volt_vana);
}

static POSSIBLY_UNUSED bool pmu_dcdc3_calib_low_target_voltage_enable(void)
{
    uint16_t val;
    bool low_tgt_volt = false;

    pmu_get_efuse(PMU_EFUSE_PAGE_RESERVED_C, &val);
    val = GET_BITFIELD(val, PMU_EFUSE_C_FLASH_VDD_TYPE);

    // 1.2V flash
    if (val == 1) {
        low_tgt_volt = true;
    }

    return low_tgt_volt;
}

static POSSIBLY_UNUSED int8_t pmu_dcdc3_calib_low_target_voltage_comp_get(void)
{
    uint16_t val;
    int8_t dcdc3_comp;

    pmu_get_efuse(PMU_EFUSE_PAGE_RESERVED_D, &val);
    dcdc3_comp = GET_BITFIELD(val, PMU_EFUSE_D_DCDC3_COMP_LP_LOW_TGT);

    if (val & PMU_EFUSE_D_DCDC3_COMP_LP_LOW_TGT_SIGN) {
        dcdc3_comp = -dcdc3_comp;
    }

    return dcdc3_comp;
}

static POSSIBLY_UNUSED void pmu_get_dcdc_calib_value(void)
{
    union VOLT_COMP_T cv;

    TRACE(0, "%s", __func__);

    pmu_get_efuse(PMU_EFUSE_PAGE_DCDC_VOLT_CAL, &cv.v);
    if (cv.f.dcdc1_f) { //digital
        pmu_dcdc_dig_comp = -(int8_t)(cv.f.dcdc1_v);
    } else {
        pmu_dcdc_dig_comp = (int8_t)(cv.f.dcdc1_v);
    }
    if (cv.f.dcdc2_f) { //ana
        pmu_dcdc_ana_comp = -(int8_t)(cv.f.dcdc2_v);
    } else {
        pmu_dcdc_ana_comp = (int8_t)(cv.f.dcdc2_v);
    }
    if (cv.f.dcdc3_f) { //hppa
        pmu_dcdc_hppa_comp = -(int8_t)(cv.f.dcdc3_v);
    } else {
        pmu_dcdc_hppa_comp = (int8_t)(cv.f.dcdc3_v);
    }

    pmu_get_efuse(PMU_EFUSE_PAGE_DCDC_VOLT_LP_CAL, &cv.v);
    if (cv.f.dcdc1_f) { //digital
        pmu_dig_lp_comp = -(int8_t)(cv.f.dcdc1_v);
    } else {
        pmu_dig_lp_comp = (int8_t)(cv.f.dcdc1_v);
    }
    if (cv.f.dcdc2_f) { //ana
        pmu_ana_lp_comp = -(int8_t)(cv.f.dcdc2_v);
    } else {
        pmu_ana_lp_comp = (int8_t)(cv.f.dcdc2_v);
    }
#ifdef PMU_FORCE_LP_MODE
    if (pmu_dcdc3_calib_low_target_voltage_enable() && vcodec_mv < 1600) {
        if (pmu_force_lp_bg_valid) {
            TRACE(0, "VCODEC low target voltage calib!");
            pmu_hppa_lp_comp = pmu_dcdc3_calib_low_target_voltage_comp_get();
        } else {
            if (cv.f.dcdc3_f) { //hppa
                pmu_hppa_lp_comp = -(int8_t)(cv.f.dcdc3_v);
            } else {
                pmu_hppa_lp_comp = (int8_t)(cv.f.dcdc3_v);
            }
        }
    } else
#endif
    {
        if (cv.f.dcdc3_f) { //hppa
            pmu_hppa_lp_comp = -(int8_t)(cv.f.dcdc3_v);
        } else {
            pmu_hppa_lp_comp = (int8_t)(cv.f.dcdc3_v);
        }
    }

#ifdef FORCE_BIG_BANDGAP
    pmu_dig_lp_comp  = pmu_dcdc_dig_comp;
    pmu_ana_lp_comp  = pmu_dcdc_ana_comp;
    pmu_hppa_lp_comp = pmu_dcdc_hppa_comp;
#elif defined(PMU_FORCE_LP_MODE)
    if (pmu_force_lp_bg_valid) {
        pmu_dcdc_dig_comp  = pmu_dig_lp_comp;
        pmu_dcdc_ana_comp  = pmu_ana_lp_comp;
        pmu_dcdc_hppa_comp = pmu_hppa_lp_comp;
    }
#endif

    pmu_get_dcdc_safe_volt_min();
    pmu_dcdc_ana_comp += pmu_dcdc_safe_volt_vana;
#ifdef PMU_SAFE_VOLT_MIN
    pmu_dcdc_dig_comp += pmu_dcdc_safe_volt_vcore;
#endif

    TRACE(0, "normal VCORE-M=%d VANA=%d VCODEC=%d", pmu_dcdc_dig_comp, pmu_dcdc_ana_comp, pmu_dcdc_hppa_comp);
    TRACE(0, "lowpwr VCORE-M=%d VANA=%d VCODEC=%d", pmu_dig_lp_comp, pmu_ana_lp_comp, pmu_hppa_lp_comp);
}

static POSSIBLY_UNUSED unsigned short pmu_reg_val_add(unsigned short val, int delta, unsigned short max)
{
    int result = val + delta;

    if (result > max) {
        result = max;
    } else if (result < 0) {
        result = 0;
    }

    return (unsigned short)result;
}
#endif

uint32_t BOOT_TEXT_FLASH_LOC read_hw_metal_id(void)
{
    uint16_t val;
    uint32_t metal_id;

    pmu_read(PMU_REG_METAL_ID, &val);
    pmu_metal_id = GET_BITFIELD(val, REVID);

    metal_id = hal_cmu_get_aon_revision_id();
    mcu_metal_id = metal_id;
#ifndef ARM_CMSE
    charger_read_hw_metal_id();
#endif
    return metal_id;
}

static void BOOT_TEXT_FLASH_LOC pmu_boot_cause_init(void)
{
    uint16_t val = 0;
    union HAL_HW_BOOTMODE_T hw_bm;

    hw_bm = hal_hw_bootmode_get();

    if (hw_bm.watchdog) {
        pmu_boot_reason = PMU_BOOT_CAUSE_DIG_WDT;
    } else if (hw_bm.global) {
        pmu_boot_reason = PMU_BOOT_CAUSE_DIG_REBOOT;
    } else {
        pmu_read(PMU_REG_HW_STATUS_5F, &val);
        val = GET_BITFIELD(val, PMU_LDO_ON_SOURCE);
        if (val == 1) {
            pmu_boot_reason = PMU_BOOT_CAUSE_POWER_KEY;
        } else if (val == 2) {
            pmu_boot_reason = PMU_BOOT_CAUSE_RTC;
        } else if (val == 3) {
            pmu_boot_reason = PMU_BOOT_CAUSE_AC_IN;
        } else if (val == 4) {
            pmu_boot_reason = PMU_BOOT_CAUSE_AC_OUT;
        } else {
            pmu_boot_reason = PMU_BOOT_CAUSE_NULL;
        }
    }

    pmu_read(PMU_REG_COMMAND_NOT_RESET, &val);
    if ((val & REG_NOT_RESET_CHIP_PWR_ON) == 0) {
        pmu_boot_first_pwr_up = true;
    }
#if !(defined(OTA_BOOT_IMAGE) || defined(ARM_CMSE))
    val |= REG_NOT_RESET_CHIP_PWR_ON;
    pmu_write(PMU_REG_COMMAND_NOT_RESET, val);
#endif
}

enum PMU_BOOT_CAUSE_T pmu_boot_cause_get(void)
{
    return pmu_boot_reason;
}

bool pmu_boot_first_power_up(void)
{
    return pmu_boot_first_pwr_up;
}

void BOOT_TEXT_FLASH_LOC pmu_boot_init(void)
{
    uint16_t val;

#ifndef ARM_CMSE
    charger_boot_init();
#endif
#if !defined(PROGRAMMER) && defined(EFUSE_READ_WORKAROUND)
    if ((hal_sw_bootmode_get() & HAL_SW_BOOTMODE_FORCE_USB_DLD) == 0 &&
            hal_iomux_uart0_connected()) {
        hal_sw_bootmode_set(HAL_SW_BOOTMODE_FORCE_USB_DLD);
        patch_open(0);
        patch_code_enable_id(0, 0x20e94, 0x22012201);
        patch_code_enable_id(1, 0x20e9c, 0xbd10b002);
        hal_cmu_reset_pulse(HAL_CMU_MOD_H_MCU);
    }
#endif

    pmu_boot_cause_init();

#ifdef RTC_ENABLE
    // RTC will be restored in pmu_open()
    pmu_rtc_save_context();
#endif

#ifdef __WATCHER_DOG_RESET__
    pmu_wdt_save_context();
#endif

#if !defined(FPGA) && !defined(PROGRAMMER)
    pmu_charger_save_context();
#endif

    // Reset PMU (to recover from a possible insane state, e.g., ESD reset)
    pmu_write(PMU_REG_METAL_ID, 0xCAFE);
    pmu_write(PMU_REG_METAL_ID, 0x5FEE);
    hal_sys_timer_delay(US_TO_TICKS(500));

    pmu_read(PMU_REG_CHARGER_CFG, &val);
    val |= REG_CHARGE_INTR_EN;
    pmu_write(PMU_REG_CHARGER_CFG, val);

#ifdef __WATCHER_DOG_RESET__
    pmu_wdt_restore_context();
#else
    pmu_wdt_stop();
#endif

    pmu_rf_ana_init();
}

static unsigned int NOINLINE BOOT_TEXT_SRAM_LOC pmu_count_zeros(unsigned int val, unsigned int bits)
{
    int cnt = 0;
    int i;

    for (i = 0; i < bits; i++) {
        if ((val & (1 << i)) == 0) {
            cnt++;
        }
    }

    return cnt;
}

void pmu_pll_freq_reg_set(uint16_t low, uint16_t high, uint16_t high2)
{
    int ret;
    uint16_t val;

    ret = rf_write(RF_REG_148, low);
    if (ret) {
        return;
    }

    ret = rf_write(RF_REG_149, high);
    if (ret) {
        return;
    }

    ret = rf_write(RF_REG_14A, high2);
    if (ret) {
        return;
    }

    ret = rf_read(RF_REG_14C, &val);
    if (ret) {
        return;
    }
    val |= REG_LP_BBPLL_FREQ_EN_COMB;
    ret = rf_write(RF_REG_14C, val);
    if (ret) {
        return;
    }

    // Delay at least for 7us
    hal_sys_timer_delay(US_TO_TICKS(100));
}

void BOOT_TEXT_FLASH_LOC bbpll_freq_pll_config(uint32_t freq)
{
    uint64_t PLL_cfg_val;
    uint16_t v[3];
    uint32_t crystal = hal_cmu_get_crystal_freq();

    PLL_cfg_val = ((uint64_t)(1 << 28) * freq + crystal / 2) / crystal;
    v[0] = PLL_cfg_val & 0xFFFF;
    v[1] = (PLL_cfg_val >> 16) & 0xFFFF;
    v[2] = (PLL_cfg_val >> 32) & 0xFFFF;

    pmu_pll_freq_reg_set(v[0], v[1], v[2]);
}

void bbpll_codec_clock_enable(bool en)
{
    static bool codec_clk_en = false;
    uint16_t val;

    if (en == codec_clk_en) {
        return;
    }
    rf_read(RF_REG_170, &val);
    if (en) {
        val |= REG_LP_BBPLL_EN_CLK_CODEC;
    } else {
        val &= ~(REG_LP_BBPLL_EN_CLK_CODEC);
    }
    rf_write(RF_REG_170, val);
    codec_clk_en = en;
}

void bbpll_pll_update(uint64_t pll_cfg_val, bool calib)
{
    uint16_t v[3];
    uint16_t val;
    //bool sys_is_bbpll = false;
    //bool flash_is_bbpll = false;
    //uint32_t lock = 0;

    v[0] = pll_cfg_val & 0xFFFF;
    v[1] = (pll_cfg_val >> 16) & 0xFFFF;
    v[2] = (pll_cfg_val >> 32) & 0xFFFF;

    rf_write(RF_REG_148, v[0]);
    rf_write(RF_REG_149, v[1]);
    rf_read(RF_REG_14A, &val);
    val = SET_BITFIELD(val, REG_LP_BBPLL_SDM_FREQWORD_34_32, v[2]);
    rf_write(RF_REG_14A, val);

    rf_read(RF_REG_14C, &val);
    val |= REG_LP_BBPLL_FREQ_EN_COMB;
    rf_write(RF_REG_14C, val);
}

void bbpll_set_codec_div(uint32_t div)
{
    uint16_t val;

    rf_read(RF_REG_170, &val);
    val = SET_BITFIELD(val, REG_LP_BBPLL_DIVN_AD, div);
    rf_write(RF_REG_170, val);
}

void bbpll_set_spd(uint32_t spd)
{
    uint16_t val;
    rf_read(RF_REG_16E, &val);
    val = SET_BITFIELD(val, REG_LP_BBPLL_VCO_SPD, spd);
    rf_write(RF_REG_16E, val);
}

void bbpll_auto_calib_spd(uint32_t codec_freq, uint32_t div)
{
    const uint32_t sys_freq_div = 4;
    const uint32_t f_med = codec_freq * div / sys_freq_div
#ifndef AUD_PLL_DOUBLE
        / 2
#endif
        ;
    const uint32_t calc_ms = 50;
    const float deviation = (float)f_med / hal_cmu_get_crystal_freq() * (1.0f / CONFIG_FAST_SYSTICK_HZ) / (calc_ms / 1000.0f);
    const uint32_t f_min = f_med - (uint32_t)(f_med * deviation * 1.6f);
    const uint32_t f_max = f_med + (uint32_t)(f_med * deviation * 1.2f);
    uint32_t f;
    uint32_t k = 0;
    uint32_t valid_cnt = 0;
    uint32_t valid_start = 0;
    uint32_t valid_end = 0;
    uint32_t cur_cnt = 0;
    uint32_t sel;
    uint32_t lock;

    lock = int_lock();

    const enum HAL_CMU_FREQ_T org_freq = hal_sysfreq_get_hw_freq();
    // Switch to 52M to avoid using PLL
    hal_cmu_sys_set_freq(HAL_CMU_FREQ_52M);
    // Enable PLL
#ifdef ULTRA_LOW_POWER
    hal_cmu_low_freq_mode_disable(org_freq, HAL_CMU_FREQ_208M);
#endif
    // Inc vcore voltage
    pmu_sys_freq_config(HAL_CMU_FREQ_208M);

    analog_aud_freq_pll_config(codec_freq, div);
    TRACE(0,"codec_freq = %d, div = %d", codec_freq, div);
    TRACE(0,"f_med = %d, deviation = %f", f_med, (double)deviation);
    hal_cmu_force_sys_pll_div(sys_freq_div);

    for (k = 0; k < 8; k++) {
        // Switch to 52M to avoid using PLL
        hal_cmu_sys_set_freq(HAL_CMU_FREQ_52M);

        bbpll_set_spd(k);

        // Switch to PLL freq
        hal_cmu_sys_set_freq(HAL_CMU_FREQ_208M);

        f = hal_sys_timer_calc_cpu_freq(calc_ms, 1);
        if (f_min < f && f < f_max) {
            cur_cnt++;
        } else {
            if (cur_cnt) {
                if (cur_cnt > valid_cnt) {
                    valid_cnt = cur_cnt;
                    valid_end = k - 1;
                }
                cur_cnt = 0;
            }
        }
        TRACE(0, "[%c%2u] f=%u min=%u max=%u", cur_cnt ? '+' : '-', k, f, f_min, f_max);
    }

    if (cur_cnt) {
        if (cur_cnt > valid_cnt) {
            valid_cnt = cur_cnt;
            valid_end = k - 1;
        }
    }
    if (valid_cnt) {
        valid_start = valid_end + 1 - valid_cnt;
        sel = (valid_start + valid_end) / 2;
        TRACE(0, "Valid spd=[%2u, %2u] cnt=%2u sel=%2u/0x%02X", valid_start, valid_end, valid_cnt, sel, sel);
    } else {
        sel = 0x0;
        TRACE(0, "BAD: no valid spd");
    }
    bbpll_set_spd(sel);
    // Restore the sys freq
    hal_cmu_force_sys_pll_div(0);
    hal_cmu_sys_set_freq(org_freq);
    // Restore PLL status
#ifdef ULTRA_LOW_POWER
    hal_cmu_low_freq_mode_enable(HAL_CMU_FREQ_208M, org_freq);
#endif
    // Restore vcore voltage
    pmu_sys_freq_config(org_freq);

    int_unlock(lock);
}

void BOOT_TEXT_FLASH_LOC pmu_rf_ana_init(void)
{
    int ret;
    uint16_t val;
    union BOOT_SETTINGS_T boot;

    ret = pmu_get_efuse(PMU_EFUSE_PAGE_BOOT, &boot.reg);
    if (ret) {
        boot.reg = 0;
    } else {
        if (pmu_count_zeros(boot.reg, 12) != boot.chksum) {
            boot.reg = 0;
        }
    }
    hal_cmu_set_crystal_freq_index(boot.crystal_freq);
    // Update ISPI cfg
    ret = hal_analogif_open();
    if (ret) {
        SAFE_PROGRAM_STOP();
    }

#if !defined(FPGA) && !defined(PROGRAMMER) && !defined(MCU_HIGH_PERFORMANCE_MODE)
    if (hal_cmu_get_crystal_freq() != hal_cmu_get_default_crystal_freq()) {
        // Update bbpll freq after resetting RF and getting crystal freq
        bbpll_freq_pll_config(384000000);
    }
#endif

#if defined(PMU_FULL_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
    // Reset RF
    rf_write(RF_REG_00, 0xCAFE);
    rf_write(RF_REG_00, 0x5FEE);
    hal_sys_timer_delay(US_TO_TICKS(500));

    // Reset ANA
    ana_write(ANA_REG_00, 0xCAFE);
    ana_write(ANA_REG_00, 0x5FEE);
    hal_sys_timer_delay(US_TO_TICKS(500));
#endif

    // modify 26M_X2 And 26M_X4 config
    rf_read(RF_REG_159, &val);
    val = SET_BITFIELD(val, REG_XTAL_X2X4_RC_OUT_OTHER, 0xf) | SET_BITFIELD(val, REG_XTAL_LDO_VTUNE_OTHER, 0x4);
    rf_write(RF_REG_159, val);

    // modify 26M_X2 config
    rf_read(RF_REG_15A, &val);
    val = SET_BITFIELD(val, REG_XTAL_FDATA1, 0x7) | REG_XTAL_FDATA2_DR | REG_XTAL_FDATA1_DR;
    rf_write(RF_REG_15A, val);
    rf_read(RF_REG_15B, &val);
    val = SET_BITFIELD(val, REG_XTAL_FDATA2, 0x1);
    rf_write(RF_REG_15B, val);

#ifdef ANA_26M_X4_ENABLE
    //rf_write(0xBF, 0x0400);
    // modify 26M_X4 config
    rf_read(RF_REG_168, &val);
    val = SET_BITFIELD(val, REG_XTAL_FX4_CAP1, 0x1);
    rf_write(RF_REG_168, val);
    rf_read(RF_REG_169, &val);
    val = SET_BITFIELD(val, REG_XTAL_FX4_CAP2, 0x1);
    rf_write(RF_REG_169, val);
#endif

    rf_read(RF_REG_16E, &val);
    val = SET_BITFIELD(val, REG_LP_BBPLL_VCO_SPD, 0x4);
    rf_write(RF_REG_16E, val);

    rf_read(RF_REG_146, &val);
    val = SET_BITFIELD(val, REG_LP_BBPLL_INT_DEC_SEL, 0);
    rf_write(RF_REG_146, val);

#if defined(PMU_EFUSE_CFG_EMA_VCORE) && defined(PMU_SAFE_VOLT_MIN)
    uint16_t val2;
    uint16_t val3;

    pmu_get_efuse(PMU_EFUSE_PAGE_RESERVED_7, &val);
    val = (val & PMU_EFUSE_7_EMA_CFG_MSB) >> 13;

    pmu_get_efuse(PMU_EFUSE_PAGE_RESERVED_C, &val2);
    val2 = (val2 & PMU_EFUSE_C_EMA_CFG_LSB) >> 13;

    pmu_get_efuse(PMU_EFUSE_PAGE_SW_CFG, &val3);
    val3 = GET_BITFIELD(val3, PMU_EFUSE_5_VCORE_MIN);

    // bit4:     page_0x7, bit13
    // bit3:     page_0xc, bit13
    // bit[2:0]: page_0x5, bti[14:12]
    val = (val << 4) | (val2 << 3) | val3;
    if (val == 0xc || val == 0x14 || val == 0x8 || val == 0x10) {
        pmu_ema_cfg = val;
    } else {
        // Invalid
        pmu_ema_cfg = 0xff;
    }

    if (pmu_ema_cfg == 0x14 || pmu_ema_cfg == 0x10) {
        hal_cmu_set_mcu_ram_timing(2);
    }
#endif
}

void BOOT_TEXT_SRAM_LOC pmu_pll_div_reset_set(enum HAL_CMU_PLL_T pll)
{
    uint32_t lock;
    uint16_t val;

    lock = int_lock();
    if (pll == HAL_CMU_PLL_AUD) {
        rf_read(RF_REG_F5, &val);
        val |= REG_BT_BBPLL_DIVN_CODEC_MASK;
        rf_write(RF_REG_F5, val);
    }
    int_unlock(lock);
}

void BOOT_TEXT_SRAM_LOC pmu_pll_div_reset_clear(enum HAL_CMU_PLL_T pll)
{
    uint32_t lock;
    uint16_t val;

    lock = int_lock();
    if (pll == HAL_CMU_PLL_AUD) {
        rf_read(RF_REG_F5, &val);
        val = SET_BITFIELD(val, REG_BT_BBPLL_DIVN_CODEC, bbpll_codec_div);
        rf_write(RF_REG_F5, val);
    }
    int_unlock(lock);
}

void pmu_pll_div_set(enum HAL_CMU_PLL_T pll, enum PMU_PLL_DIV_TYPE_T type, uint32_t div)
{
    uint32_t lock;
    uint16_t val;

    if (type != PMU_PLL_DIV_CODEC) {
        return;
    }

    lock = int_lock();
    if (pll == HAL_CMU_PLL_AUD) {
        if (div != bbpll_codec_div) {
            bbpll_codec_div = div;
            rf_read(RF_REG_F5, &val);
            val |= REG_BT_BBPLL_DIVN_CODEC_MASK;
            rf_write(RF_REG_F5, val);
            if (div != (REG_BT_BBPLL_DIVN_CODEC_MASK >> REG_BT_BBPLL_DIVN_CODEC_SHIFT)) {
                val = SET_BITFIELD(val, REG_BT_BBPLL_DIVN_CODEC, bbpll_codec_div);
                rf_write(RF_REG_F5, val);
            }
        }
    }
    int_unlock(lock);
}

int pmu_get_security_value(union SECURITY_VALUE_T *val)
{
    int ret;

    val->reg = 0;

    ret = pmu_get_efuse(PMU_EFUSE_PAGE_SECURITY, &val->reg);
    if (ret) {
        // Error
        return ret;
    }

    return 0;
}

enum HAL_FLASH_ID_T pmu_get_boot_flash_ctrl_id(void)
{
#ifdef FLASH1_CTRL_BASE
    union SECURITY_VALUE_T security;

    pmu_get_security_value(&security);

    return (security.root.flash_id) ? HAL_FLASH_ID_1 : HAL_FLASH_ID_0;
#else
    return HAL_FLASH_ID_0;
#endif
}

static int BOOT_TEXT_SRAM_LOC pmu_get_efuse_phy(enum PMU_EFUSE_PAGE_T page, unsigned short *efuse)
{
    int ret;
    unsigned short val;
    unsigned short tmp[2];

#ifdef EFUSE_READ_WORKAROUND
    *efuse = 0;
    return 0;
#endif

    //hal_cmu_pmu_fast_clock_enable();

    // Enable CLK_EN
    val = REG_EFUSE_CLK_EN;
    ret = pmu_write(PMU_REG_EFUSE_CTRL, val);
    if (ret) {
        goto _exit;
    }

    // Enable TURN_ON
    val |= REG_EFUSE_TURN_ON;
    ret = pmu_write(PMU_REG_EFUSE_CTRL, val);
    if (ret) {
        goto _exit;
    }

    // Write Address
#ifdef PMU_EFUSE_NO_REDUNDANCY
    val |= REG_EFUSE_ADDRESS(page / 2);
#else
    val |= REG_EFUSE_ADDRESS(page); //redundancy
#endif
    ret = pmu_write(PMU_REG_EFUSE_CTRL, val);
    if (ret) {
        goto _exit;
    }

    // Set Strobe Trigger = 1
    val |= REG_EFUSE_STROBE_TRIGGER;
    ret = pmu_write(PMU_REG_EFUSE_CTRL, val);
    if (ret) {
        goto _exit;
    }

    // set Strobe Trigger = 0
    val &= ~REG_EFUSE_STROBE_TRIGGER;
    ret = pmu_write(PMU_REG_EFUSE_CTRL, val);
    if (ret) {
        goto _exit;
    }

    // Read Efuse High 16 bits
    ret = pmu_read(PMU_REG_EFUSE_DATA_LOW, &tmp[0]);
    if (ret) {
        goto _exit;
    }

    // Read Efuse Low 16 bits
    ret = pmu_read(PMU_REG_EFUSE_DATA_HIGH, &tmp[1]);
    if (ret) {
        goto _exit;
    }
#ifdef PMU_EFUSE_NO_REDUNDANCY
    *efuse = tmp[page % 2];
#else
    *efuse = (tmp[0] | tmp[1]); //redundancy
#endif

    _exit:
    // Disable TURN_ON
    val &= ~(REG_EFUSE_TURN_ON | REG_EFUSE_ADDRESS_MASK);
    ret = pmu_write(PMU_REG_EFUSE_CTRL, val);

    // Disable CLK_EN
    val &= ~REG_EFUSE_CLK_EN;
    ret = pmu_write(PMU_REG_EFUSE_CTRL, val);

    //hal_cmu_pmu_fast_clock_disable();

    return ret;
}

#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
#if !defined(ROM_BUILD) && defined(PMU_EFUSE_INIT_IN_BOOT)
static void BOOT_TEXT_FLASH_LOC pmu_efuse_init(void)
{
    uint32_t i;

    for (i = 0; i < PMU_EFUSE_PAGE_QTY; i++) {
        pmu_get_efuse_phy(i, &pmu_efuse_page_arrays[i]);
    }

    efuse_buf_valid = true;
}
#endif
#endif

int BOOT_TEXT_FLASH_LOC pmu_get_efuse(enum PMU_EFUSE_PAGE_T page, unsigned short *efuse)
{
#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
#if !defined(ROM_BUILD) && defined(PMU_EFUSE_INIT_IN_BOOT)
    if (efuse_buf_valid) {
        *efuse = pmu_efuse_page_arrays[page];
        return 0;
    }
#endif
#endif
    return pmu_get_efuse_phy(page, efuse);
}

#ifdef PMU_DCDC_SIMO_ENABLE
static void pmu_dcdc_simo_enable(bool en)
{
    uint16_t val;

    pmu_read(PMU_REG_DCDC_SIMO_IX_CFG, &val);
    if (en) {
        val |= REG_BUCK_EN_SIMO;
    } else {
        val &= ~REG_BUCK_EN_SIMO;
    }
    pmu_write(PMU_REG_DCDC_SIMO_IX_CFG, val);

    if (en) {
        hal_sys_timer_delay_us(PMU_DCDC_PU_STABLE_TIME_US);
    }
}
#endif

static void pmu_sys_ctrl(bool shutdown)
{
    uint16_t val;
    uint32_t lock = int_lock();

    PMU_INFO_TRACE_IMM(0, "Start pmu %s", shutdown ? "shutdown" : "reboot");

#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
#if defined(MCU_HIGH_PERFORMANCE_MODE)
    // Default vcore might not be high enough to support high performance mode
    pmu_high_performance_mode_enable(false);
#endif
    hal_cmu_sys_set_freq(HAL_CMU_FREQ_26M);
    pmu_vcore_req |= PMU_VCORE_PMU_RESET;
#if !defined(HPPA_LDO_ON) && !defined(PMU_DCDC_SIMO_ENABLE)
    pmu_hppa_dcdc_to_ldo();
#endif
    pmu_mode_change(PMU_POWER_MODE_LDO);
    hal_sys_timer_delay(MS_TO_TICKS(1));
#ifdef PMU_DCDC_PWM
    hal_cmu_dcdc_clock_disable(HAL_CMU_DCDC_CLOCK_USER_DCDC_SYNC);
#endif
#endif

#ifdef RTC_ENABLE
    pmu_rtc_save_context();
#ifdef CALIB_POWER_OFF_LPO
    if (shutdown) {
        pmu_rtc_clk_div_save();
    }
#endif
#endif

#ifdef PMU_FORCE_LP_MODE
    pmu_bandgap_cfg_reset();
#endif

    // Reset PMU
    pmu_write(PMU_REG_METAL_ID, 0xCAFE);
    pmu_write(PMU_REG_METAL_ID, 0x5FEE);
    hal_sys_timer_delay(US_TO_TICKS(500));

    pmu_read(PMU_REG_CHARGER_CFG, &val);
    val |= REG_CHARGE_INTR_EN;
    pmu_write(PMU_REG_CHARGER_CFG, val);

#ifdef RTC_ENABLE
    pmu_rtc_restore_context();
#ifdef CALIB_POWER_OFF_LPO
    if (shutdown) {
        pmu_rtc_clk_div_reload();
    }
#endif
#endif

    // Reset Charger
    if (hal_get_chip_metal_id() > HAL_CHIP_METAL_ID_0) {
        chg_write(CHG_REG_METAL_ID, 0xCAFE);
        chg_write(CHG_REG_METAL_ID, 0x5FEE);
        hal_sys_timer_delay(US_TO_TICKS(500));

        // ramp for CC satrtup
        chg_read(CHG_REG_CC_PRE_CV_CFG, &val);
        val &= ~REG_CHARGER_ICC_SEL_DR;
        chg_write(CHG_REG_CC_PRE_CV_CFG, val);
    }

    if (shutdown) {
#if defined(_AUTO_TEST_)
        if (at_skip_shutdown) {
            hal_cmu_sys_reboot();
            return;
        }
#endif

#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
        if (
#if defined(RTC_ENABLE)
            rtc_ctx.enabled ||
#endif
#if defined(CHG_OUT_PWRON)
            // ac_on will en lpo
            true ||
#endif
            false) {
            pmu_wdt_stop();
        } else {
            pmu_wdt_config(3*1000, 3*1000);
            pmu_wdt_start();
        }
#endif

        pmu_read(PMU_REG_USB_CFG_3F,&val);
#ifdef CHG_OUT_PWRON
        val |= AC_OUT_LDO_ON_EN;
#else
        val &= ~AC_OUT_LDO_ON_EN;
#endif
        pmu_write(PMU_REG_USB_CFG_3F, val);

        // Power off
        PMU_INFO_TRACE_IMM(0, "shipment_en=%d", shipment_en);
        if (shipment_en) {
            // Only AC_ON and chg-pattern can power on.
            charger_ship_mode_enable();
        }

#ifdef CHG_FORCE_LP_PWROFF
        // Can't charge
        if (hal_get_chip_metal_id() > HAL_CHIP_METAL_ID_0) {
            chg_read(CHG_REG_02, &val);
            val |= ACIN_PP_LDO_LP_EN;
            chg_write(CHG_REG_02, val);

            chg_read(CHG_REG_TEMP_BG_CFG, &val);
            val &= ~REG_BG_EN_OTP;
            chg_write(CHG_REG_TEMP_BG_CFG, val);

            val &= ~REG_BG_REF_GEN_EN;
            chg_write(CHG_REG_TEMP_BG_CFG, val);

            val &= ~REG_BG_CORE_EN;
            chg_write(CHG_REG_TEMP_BG_CFG, val);

            hal_sys_timer_delay(MS_TO_TICKS(1));
        }
#endif

        pmu_read(PMU_REG_POWER_OFF,&val);
        val |= SOFT_POWER_OFF;
        for (int i = 0; i < 100; i++) {
            pmu_write(PMU_REG_POWER_OFF,val);
            hal_sys_timer_delay(MS_TO_TICKS(5));
        }

        hal_sys_timer_delay(MS_TO_TICKS(50));

        //can't reach here
        PMU_INFO_TRACE_IMM(0, "\nError: pmu shutdown failed!\n");
        hal_sys_timer_delay(MS_TO_TICKS(5));

        pmu_read(PMU_REG_POWER_OFF, &val);
        if (val & SOFT_POWER_OFF) {
            PMU_INFO_TRACE_IMM(0, "\nError: SOFT_POWER_OFF=1!\n");
        }
        pmu_wdt_reboot();
        while (1) {}
    } else {
#if defined(PMU_FULL_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
        // CAUTION:
        // 1) Never reset RF because system or flash might be using X2/X4, which are off by default
        // 2) Never reset RF/ANA because system or flash might be using PLL, and the reset might cause clock glitch
        // TODO:
        // Restore BBPLL settings in RF
#endif
    }

    hal_cmu_sys_reboot();

    int_unlock(lock);
}

void pmu_shutdown(void)
{
    pmu_sys_ctrl(true);
}

void pmu_reboot(void)
{
    pmu_sys_ctrl(false);
}

static inline uint16_t pmu_get_module_addr(enum PMU_MODUAL_T module)
{
    if (module == PMU_CODEC) {
        return PMU_REG_CODEC_CFG;
    } else if (module == PMU_PA) {
        return PMU_REG_PA_CFG;
    } else if (module == PMU_SENSOR) {
        return PMU_REG_SENSOR_CFG;
    } else {
        return module + PMU_REG_MODULE_START;
    }
}

void pmu_module_config(enum PMU_MODUAL_T module,unsigned short is_manual,unsigned short ldo_on,unsigned short lp_mode,unsigned short dpmode)
{
    unsigned short val;
    unsigned short module_address;
    const struct PMU_MODULE_CFG_T *module_cfg_p = &pmu_module_cfg[module];

    module_address = pmu_get_module_addr(module);

    pmu_read(module_address, &val);
    if (is_manual) {
        val |= module_cfg_p->manual_bit;
    } else {
        val &= ~module_cfg_p->manual_bit;
    }
    if (ldo_on) {
        val |= module_cfg_p->ldo_en;
    } else {
        val &= ~module_cfg_p->ldo_en;
    }

#ifdef PMU_FORCE_LP_MODE
    if (pmu_force_lp_bg_valid) {
        if (pmu_power_mode != PMU_POWER_MODE_NONE) {
            val |= module_cfg_p->lp_en_dr;
            if (ldo_on) {
                val |= module_cfg_p->lp_en;
            } else {
                val &= ~module_cfg_p->lp_en;
            }
        }
    } else {
        if (lp_mode) {
            val &= ~module_cfg_p->lp_en_dr;
        } else {
            val = (val & ~module_cfg_p->lp_en) | module_cfg_p->lp_en_dr;
        }
    }
#else
    if (lp_mode) {
        val &= ~module_cfg_p->lp_en_dr;
    } else {
        val = (val & ~module_cfg_p->lp_en) | module_cfg_p->lp_en_dr;
    }
#endif

    if (dpmode) {
        val |= module_cfg_p->dsleep_mode;
    } else {
        val &= ~module_cfg_p->dsleep_mode;
    }
    pmu_write(module_address, val);
}

#ifdef PMU_FORCE_LP_MODE
void pmu_module_force_lp_config(void)
{
    uint16_t val;
    uint16_t module_address;
    const struct PMU_MODULE_CFG_T *module_cfg_p;
    enum PMU_MODUAL_T module;
    bool ldo_on;

    if (pmu_force_lp_bg_valid) {
        for (module = 0; module < PMU_MODULE_QTY; module++) {
            module_cfg_p = &pmu_module_cfg[module];
            module_address = pmu_get_module_addr(module);

            pmu_read(module_address, &val);
            ldo_on = !!(val & module_cfg_p->ldo_en);
            val |= module_cfg_p->lp_en_dr;
            if (ldo_on) {
                val |= module_cfg_p->lp_en;
            } else {
                val &= ~module_cfg_p->lp_en;
            }
            pmu_write(module_address, val);
        }
    }
}
#endif

void pmu_module_set_volt(unsigned char module, unsigned short sleep_v,unsigned short normal_v)
{
    unsigned short val;
    unsigned short module_address;
    const struct PMU_MODULE_CFG_T *module_cfg_p = &pmu_module_cfg[module];

   if (module == PMU_SENSOR) {
        pmu_read(PMU_REG_SENSOR_CFG, &val);
        val &= ~module_cfg_p->normal_v;
        val |= (normal_v << module_cfg_p->normal_v_shift) & module_cfg_p->normal_v;
        pmu_write(PMU_REG_SENSOR_CFG, val);
        pmu_read(PMU_REG_GP_SENS_VOLT_SLEEP, &val);
        val &= ~module_cfg_p->dsleep_v;
        val |= (sleep_v << module_cfg_p->dsleep_v_shift) & module_cfg_p->dsleep_v;
        pmu_write(PMU_REG_GP_SENS_VOLT_SLEEP, val);

        return;
    }

    module_address = pmu_get_module_addr(module);

    if (module == PMU_PA) {
        if (sleep_v > PMU_LDO_PA_MAX) {
            sleep_v = PMU_LDO_PA_MAX;
        }
        if (normal_v > PMU_LDO_PA_MAX) {
            normal_v = PMU_LDO_PA_MAX;
        }
    }

    pmu_read(module_address, &val);
    val &= ~module_cfg_p->normal_v;
    val |= (normal_v << module_cfg_p->normal_v_shift) & module_cfg_p->normal_v;
    val &= ~module_cfg_p->dsleep_v;
    val |= (sleep_v << module_cfg_p->dsleep_v_shift) & module_cfg_p->dsleep_v;
    pmu_write(module_address, val);
}

int pmu_module_get_volt(unsigned char module, unsigned short *sleep_vp,unsigned short *normal_vp)
{
    unsigned short val;
    unsigned short module_address;
    const struct PMU_MODULE_CFG_T *module_cfg_p = &pmu_module_cfg[module];

    if (module == PMU_SENSOR) {
        pmu_read(PMU_REG_SENSOR_CFG, &val);
        if (normal_vp) {
            *normal_vp = (val & module_cfg_p->normal_v) >> module_cfg_p->normal_v_shift;
        }
        pmu_read(PMU_REG_GP_SENS_VOLT_SLEEP, &val);
        if (sleep_vp) {
            *sleep_vp = (val & module_cfg_p->dsleep_v) >> module_cfg_p->dsleep_v_shift;
        }

        return 0;
    }

    module_address = pmu_get_module_addr(module);

    pmu_read(module_address, &val);
    if (normal_vp) {
        *normal_vp = (val & module_cfg_p->normal_v) >> module_cfg_p->normal_v_shift;
    }
    if (sleep_vp) {
        *sleep_vp = (val & module_cfg_p->dsleep_v) >> module_cfg_p->dsleep_v_shift;
    }

    return 0;
}

static void pmu_module_ramp_volt(unsigned char module, unsigned short sleep_v, unsigned short normal_v)
{
    uint16_t old_normal_v;
    uint16_t old_sleep_v;

    pmu_module_get_volt(module, &old_sleep_v, &old_normal_v);

    if (old_normal_v < normal_v) {
        while (old_normal_v++ < normal_v) {
            pmu_module_set_volt(module, sleep_v, old_normal_v);
        }
    } else if (old_normal_v != normal_v || old_sleep_v != sleep_v) {
        pmu_module_set_volt(module, sleep_v, normal_v);
    }
}

static void pmu_dcdc_ana_get_volt(unsigned short *normal_vp, unsigned short *dsleep_vp)
{
    unsigned short val;

    pmu_read(PMU_REG_DCDC_ANA_VOLT, &val);
    if (normal_vp) {
        *normal_vp = GET_BITFIELD(val, BUCK_VANA_BIT_NORMAL);
    }
    if (dsleep_vp) {
        *dsleep_vp = GET_BITFIELD(val, BUCK_VANA_BIT_DSLEEP);
    }

#ifdef PMU_DCDC_CALIB
    if (normal_vp) {
        *normal_vp = pmu_reg_val_add(*normal_vp, -pmu_dcdc_ana_comp, MAX_BUCK_VANA_BIT_VAL);
    }
    if (dsleep_vp) {
        *dsleep_vp = pmu_reg_val_add(*dsleep_vp, -pmu_ana_lp_comp, MAX_BUCK_VANA_BIT_VAL);
    }
#endif
}

static void pmu_dcdc_ana_set_volt(unsigned short normal_v,unsigned short dsleep_v)
{
    unsigned short val;

#ifdef PMU_DCDC_CALIB
    normal_v = pmu_reg_val_add(normal_v, pmu_dcdc_ana_comp, MAX_BUCK_VANA_BIT_VAL);
    dsleep_v = pmu_reg_val_add(dsleep_v, pmu_ana_lp_comp, MAX_BUCK_VANA_BIT_VAL);
#endif

    pmu_read(PMU_REG_DCDC_ANA_VOLT, &val);
    val = SET_BITFIELD(val, BUCK_VANA_BIT_DSLEEP, dsleep_v);
    val = SET_BITFIELD(val, BUCK_VANA_BIT_NORMAL, normal_v);
    pmu_write(PMU_REG_DCDC_ANA_VOLT, val);

    pmu_read(PMU_REG_DIG_ANA_VOLT_RC, &val);
    val = SET_BITFIELD(val, BUCK_VANA_VBIT_RC, normal_v);
    pmu_write(PMU_REG_DIG_ANA_VOLT_RC, val);

    if (normal_v > dsleep_v) {
        dcdc_ramp_map |= REG_BUCK_VANA_RAMP_EN;
    } else {
        dcdc_ramp_map &= ~REG_BUCK_VANA_RAMP_EN;
    }
}

static void pmu_ana_set_volt(enum PMU_POWER_MODE_T mode)
{
    uint16_t old_act_dcdc;
    uint16_t old_lp_dcdc;
    uint16_t new_act_dcdc;

    new_act_dcdc = ana_act_dcdc;

    if (mode == PMU_POWER_MODE_ANA_DCDC || mode == PMU_POWER_MODE_DIG_DCDC) {
        pmu_dcdc_ana_get_volt(&old_act_dcdc, &old_lp_dcdc);
        if (old_act_dcdc < new_act_dcdc) {
            while (old_act_dcdc++ < new_act_dcdc) {
                pmu_dcdc_ana_set_volt(old_act_dcdc, ana_lp_dcdc);
            }
            hal_sys_timer_delay_us(PMU_VANA_STABLE_TIME_US);
        } else if (old_act_dcdc != new_act_dcdc || old_lp_dcdc != ana_lp_dcdc) {
            pmu_dcdc_ana_set_volt(new_act_dcdc, ana_lp_dcdc);
        }
    } else {
        pmu_dcdc_ana_set_volt(new_act_dcdc, ana_lp_dcdc);
    }
}

static void pmu_dcdc_dig_get_volt(unsigned short *normal_vp, unsigned short *dsleep_vp)
{
    unsigned short val;

    pmu_read(PMU_REG_DCDC_DIG_VOLT, &val);
    if (normal_vp) {
        *normal_vp = GET_BITFIELD(val, BUCK_VCORE_BIT_NORMAL);
    }
    if (dsleep_vp) {
        *dsleep_vp = GET_BITFIELD(val, BUCK_VCORE_BIT_DSLEEP);
    }

#ifdef PMU_DCDC_CALIB
    if (normal_vp) {
        *normal_vp = pmu_reg_val_add(*normal_vp, -pmu_dcdc_dig_comp, MAX_BUCK_VCORE_BIT_VAL);
    }
    if (dsleep_vp) {
        *dsleep_vp = pmu_reg_val_add(*dsleep_vp, -pmu_dig_lp_comp, MAX_BUCK_VCORE_BIT_VAL);
    }
#endif
}

static void pmu_dcdc_dig_set_volt(unsigned short normal_v,unsigned short dsleep_v)
{
    unsigned short val;

#ifdef PMU_DCDC_CALIB
    normal_v = pmu_reg_val_add(normal_v, pmu_dcdc_dig_comp, MAX_BUCK_VCORE_BIT_VAL);
    dsleep_v = pmu_reg_val_add(dsleep_v, pmu_dig_lp_comp, MAX_BUCK_VCORE_BIT_VAL);
#endif

    pmu_read(PMU_REG_DCDC_DIG_VOLT, &val);
    val = SET_BITFIELD(val, BUCK_VCORE_BIT_DSLEEP, dsleep_v);
    val = SET_BITFIELD(val, BUCK_VCORE_BIT_NORMAL, normal_v);
    pmu_write(PMU_REG_DCDC_DIG_VOLT, val);


    pmu_read(PMU_REG_DIG_ANA_VOLT_RC, &val);
    val = SET_BITFIELD(val, BUCK_VCORE_VBIT_RC, normal_v);
    pmu_write(PMU_REG_DIG_ANA_VOLT_RC, val);

    if (normal_v > dsleep_v) {
        dcdc_ramp_map |= REG_BUCK_VCORE_RAMP_EN;
    } else {
        dcdc_ramp_map &= ~REG_BUCK_VCORE_RAMP_EN;
    }
}

static void BOOT_TEXT_SRAM_LOC pmu_dig_get_target_volt(uint16_t *ldo, uint16_t *dcdc)
{
    uint16_t ldo_volt;
    uint16_t dcdc_volt;

    if (0) {
    } else if (pmu_vcore_req & PMU_VCORE_PMU_RESET) {
        ldo_volt = PMU_VDIG_DEFAULT;
        dcdc_volt = PMU_DCDC_DIG_DEFAULT;
#if defined(MCU_HIGH_PERFORMANCE_MODE)
    } else if (pmu_vcore_req & (PMU_VCORE_SYS_FREQ_HIGH)) {
        if (high_perf_freq_mhz <= 260) {
            ldo_volt = PMU_VDIG_1_05V;
            dcdc_volt = PMU_DCDC_DIG_1_05V;
        } else if (high_perf_freq_mhz <= 300) {
            ldo_volt = PMU_VDIG_1_1V;
            dcdc_volt = PMU_DCDC_DIG_1_1V;
        } else {
            ldo_volt = PMU_VDIG_1_2V;
            dcdc_volt = PMU_DCDC_DIG_1_1V;
        }
#endif
#ifdef PMU_SAFE_VOLT_MIN
        dcdc_volt -= pmu_dcdc_safe_volt_vcore;
#endif
    } else if (pmu_vcore_req & (PMU_VCORE_RS_FREQ_HIGH | PMU_VCORE_SYS_FREQ_MEDIUM
        | PMU_VCORE_IIR_EQ_FREQ_HIGH | PMU_VCORE_IIR_FREQ_HIGH)) {
        ldo_volt = PMU_VDIG_0_9V;
        dcdc_volt = PMU_DCDC_DIG_0_9V;
#ifdef PMU_SAFE_VOLT_MIN
        dcdc_volt -= pmu_dcdc_safe_volt_vcore;
#endif
    } else if (pmu_vcore_req & PMU_VCORE_SYS_FREQ_MEDIUM_LOW) {
        ldo_volt = PMU_VDIG_0_8V;
#if (BBPLL_FREQ_MHZ > 384) && !defined(OSC_26M_X4_AUD2BB)
        dcdc_volt = PMU_DCDC_DIG_0_785V;
#else
        dcdc_volt = PMU_DCDC_DIG_0_77V;
#endif
    } else {
        // Common cases
        ldo_volt = PMU_VDIG_0_75V;
        dcdc_volt = PMU_DCDC_DIG_0_75V;
    }

#if defined(PMU_EFUSE_CFG_EMA_VCORE) && defined(PMU_SAFE_VOLT_MIN)
    if (pmu_ema_cfg == 0xff) {
        ldo_volt = ldo_volt > PMU_VDIG_0_8V ? ldo_volt : PMU_VDIG_0_8V;
        dcdc_volt = dcdc_volt > PMU_DCDC_DIG_0_8V ? dcdc_volt : PMU_DCDC_DIG_0_8V;
    }
#endif

#ifdef PMU_NTC_CTRL_ENABLE
    if (!(pmu_vcore_req & (PMU_VCORE_RS_FREQ_HIGH | PMU_VCORE_SYS_FREQ_MEDIUM | PMU_VDIG_DEFAULT))) {
        if (pmu_vcore_req & PMU_VCORE_NTC_FREEZING) {
            ldo_volt += 2;
            dcdc_volt += 10;
        } else if (pmu_vcore_req & (PMU_VCORE_NTC_CHILLY | PMU_VCORE_NTC_COLD | PMU_VCORE_NTC_COOL)) {
            ldo_volt += 1;
            if (pmu_vcore_req & PMU_VCORE_NTC_CHILLY) {
                dcdc_volt += 6;
            } else if (pmu_vcore_req & PMU_VCORE_NTC_COLD) {
                dcdc_volt += 4;
            } else {
                dcdc_volt += 2;
            }
        }
    }
#endif

#if (defined(PROGRAMMER) || defined(__BES_OTA_MODE__)) && !defined(PMU_FULL_INIT)
    // Try to keep the same vcore voltage as ROM (hardware default)
    if (ldo_volt < PMU_VDIG_0_9V) {
        ldo_volt = PMU_VDIG_0_9V;
    }
    if (dcdc_volt < PMU_DCDC_DIG_0_9V) {
        dcdc_volt = PMU_DCDC_DIG_0_9V;
    }
#endif

#ifdef DIG_DCDC_MODE
    if (ldo_volt < PMU_VDIG_DEFAULT) {
        ldo_volt = PMU_VDIG_DEFAULT;
    }
#endif

    if (ldo) {
        *ldo = ldo_volt;
    }
    if (dcdc) {
        *dcdc = dcdc_volt;
    }
}

static void pmu_dig_set_volt(enum PMU_POWER_MODE_T mode)
{
    uint32_t lock;
    uint16_t dcdc_volt, old_act_dcdc, old_lp_dcdc;
    uint16_t ldo_volt, old_act_ldo, old_lp_ldo;
    bool volt_inc = false;

    lock = int_lock();

    if (mode == PMU_POWER_MODE_NONE) {
        mode = pmu_power_mode;
    }

    pmu_dig_get_target_volt(&ldo_volt, &dcdc_volt);

    pmu_module_get_volt(PMU_DIG, &old_lp_ldo, &old_act_ldo);
    pmu_dcdc_dig_get_volt(&old_act_dcdc, &old_lp_dcdc);

    if (mode == PMU_POWER_MODE_DIG_DCDC) {
        if (old_act_dcdc < dcdc_volt) {
            volt_inc = true;
            while (old_act_dcdc++ < dcdc_volt) {
                pmu_dcdc_dig_set_volt(old_act_dcdc, dig_lp_dcdc);
            }
        } else if (old_act_dcdc != dcdc_volt || old_lp_dcdc != dig_lp_dcdc) {
            pmu_dcdc_dig_set_volt(dcdc_volt, dig_lp_dcdc);
        }
        // Update the voltage of the other mode
        pmu_module_set_volt(PMU_DIG, dig_lp_ldo, ldo_volt);
    } else {
        if (old_act_ldo < ldo_volt) {
            volt_inc = true;
        }
        pmu_module_ramp_volt(PMU_DIG, dig_lp_ldo, ldo_volt);
        // Update the voltage of the other mode
        pmu_dcdc_dig_set_volt(dcdc_volt, dig_lp_dcdc);
    }

    if (volt_inc) {
        hal_sys_timer_delay_us(PMU_VCORE_STABLE_TIME_US);
    }

    int_unlock(lock);
}

static void pmu_ana_reserved_en(bool en)
{
    uint16_t val;

    pmu_read(PMU_REG_RESERVED_ANA, &val);
    if (en) {
        val |= RESERVED_ANA_EN;
    } else {
        val &= ~RESERVED_ANA_EN;
    }
    pmu_write(PMU_REG_RESERVED_ANA, val);
}

static void pmu_ldo_mode_en(void)
{
    unsigned short POSSIBLY_UNUSED val;

    // Enable vana ldo
    pmu_module_config(PMU_ANA,PMU_AUTO_MODE,PMU_LDO_ON,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_ON);
    // Enable vcore ldo
    pmu_module_config(PMU_DIG,PMU_AUTO_MODE,PMU_LDO_ON,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_ON);

    pmu_ana_reserved_en(true);

    hal_sys_timer_delay_us(PMU_LDO_PU_STABLE_TIME_US);

#ifndef PMU_DCDC_SIMO_ENABLE
    // Disable vana dcdc and vcore dcdc
    pmu_read(PMU_REG_DCDC_EN, &val);
    val = val & TEST_MODE_MASK;
    pmu_write(PMU_REG_DCDC_EN, val);
#endif
}

static void pmu_dcdc_ana_mode_en(void)
{
#ifndef PMU_DCDC_SIMO_ENABLE
    unsigned short val;

    if (pmu_power_mode == PMU_POWER_MODE_DIG_DCDC) {
        // Enable vcore ldo
        pmu_module_config(PMU_DIG,PMU_AUTO_MODE,PMU_LDO_ON,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_ON);

        hal_sys_timer_delay_us(PMU_LDO_PU_STABLE_TIME_US);

        // Disable vcore dcdc
        pmu_read(PMU_REG_DCDC_EN, &val);
        val &= ~(REG_BUCK_CC_MODE | PU_DCDC_DIG_DR | PU_DCDC_DIG_REG);
        pmu_write(PMU_REG_DCDC_EN, val);
    } else {
        // Enable vana dcdc
        pmu_read(PMU_REG_DCDC_EN, &val);
        val |= DCDC_ANA_LP_EN_DSLEEP | PU_DCDC_ANA_DSLEEP;
        val |= PU_DCDC_ANA_DR | PU_DCDC_ANA_REG;
        val &= ~REG_BUCK_CC_MODE;
        pmu_write(PMU_REG_DCDC_EN, val);

        hal_sys_timer_delay_us(PMU_DCDC_PU_STABLE_TIME_US);

        // Disable vana ldo
        pmu_module_config(PMU_ANA,PMU_MANUAL_MODE,PMU_LDO_OFF,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_OFF);
        hal_sys_timer_delay(MS_TO_TICKS(5));

        pmu_ana_reserved_en(false);
    }
#endif
}

static void pmu_dcdc_dual_mode_en(void)
{
#ifndef PMU_DCDC_SIMO_ENABLE
    unsigned short val;

    // Enable vana dcdc and vcore dcdc
    pmu_read(PMU_REG_DCDC_EN, &val);
    val |= REG_BUCK_CC_MODE;
    val |= PU_DCDC_ANA_DR | PU_DCDC_ANA_REG;
    val |= PU_DCDC_DIG_DR | PU_DCDC_DIG_REG;
    val |= DCDC_ANA_LP_EN_DSLEEP | PU_DCDC_ANA_DSLEEP;
    val |= DCDC_DIG_LP_EN_DSLEEP | PU_DCDC_DIG_DSLEEP;
    pmu_write(PMU_REG_DCDC_EN, val);

    hal_sys_timer_delay_us(PMU_DCDC_PU_STABLE_TIME_US);
#endif

    // Disable vana ldo
    pmu_module_config(PMU_ANA, PMU_MANUAL_MODE, PMU_LDO_OFF, PMU_LP_MODE_ON, PMU_DSLEEP_MODE_OFF);
    // Disable vcore ldo
    pmu_module_config(PMU_DIG, PMU_MANUAL_MODE, PMU_LDO_OFF, PMU_LP_MODE_ON, PMU_DSLEEP_MODE_OFF);

    pmu_ana_reserved_en(false);
}

void pmu_mode_change(enum PMU_POWER_MODE_T mode)
{
    uint32_t lock;

#ifdef PMU_DCDC_SIMO_ENABLE
    ASSERT(mode != PMU_POWER_MODE_ANA_DCDC, "PMU_DCDC_SIMO_ENABLE don't support PMU_POWER_MODE_ANA_DCDC!");
#endif

    if (pmu_power_mode == mode || mode == PMU_POWER_MODE_NONE) {
        return;
    }

    lock = int_lock();

    pmu_ana_set_volt(pmu_power_mode);
    pmu_dig_set_volt(pmu_power_mode);

    if (mode == PMU_POWER_MODE_ANA_DCDC) {
        pmu_dcdc_ana_mode_en();
    } else if (mode == PMU_POWER_MODE_DIG_DCDC) {
#ifdef PMU_DCDC_SIMO_ENABLE
        // LDO_VCORE and LDO_VANA will be closed in pmu_dcdc_dual_mode_en()
        pmu_dcdc_simo_enable(true);
        pmu_module_config(PMU_CODEC, PMU_MANUAL_MODE, PMU_LDO_OFF, PMU_LP_MODE_ON, PMU_DSLEEP_MODE_OFF);
#endif
        pmu_dcdc_dual_mode_en();
    } else if (mode == PMU_POWER_MODE_LDO) {
#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
#if defined(PMU_DCDC_SIMO_ENABLE) && !defined(HPPA_LDO_ON)
        pmu_hppa_dcdc_to_ldo();
#endif
#endif
        pmu_ldo_mode_en();
#ifdef PMU_DCDC_SIMO_ENABLE
        pmu_dcdc_simo_enable(false);
#endif
    }

    pmu_power_mode = mode;

    int_unlock(lock);
}

void pmu_sleep_en(unsigned char sleep_en)
{
    unsigned short val;

    pmu_read(PMU_REG_SLEEP_CFG, &val);
    if (sleep_en) {
        val |= SLEEP_ALLOW;
    } else {
        val &= ~SLEEP_ALLOW;
    }
    pmu_write(PMU_REG_SLEEP_CFG, val);

    pmu_read(PMU_REG_RC_SLEEP_CFG, &val);
    if (sleep_en) {
        val |= REG_RC_SLEEP_ALLOW;
    } else {
        val &= ~REG_RC_SLEEP_ALLOW;
    }
    pmu_write(PMU_REG_RC_SLEEP_CFG, val);
}

#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
POSSIBLY_UNUSED
static uint32_t pmu_vcodec_mv_to_val(uint16_t mv)
{
    switch(mv) {
    case 1500:
        return PMU_CODEC_1_5V;
    case 1600:
        return PMU_CODEC_1_6V;
    case 1660:
    case 1700:
        return PMU_CODEC_1_7V;
    case 1900:
        return PMU_CODEC_1_9V;
    case 2000:
        return PMU_CODEC_2_0V;
    case 2500:
        return PMU_CODEC_2_5V;
    case 2800:
        return PMU_CODEC_2_8V;
    default:
        return PMU_CODEC_1_8V;
    }
}

#ifndef HPPA_LDO_ON
POSSIBLY_UNUSED
static uint32_t pmu_dcdc_hppa_mv_to_val(uint16_t mv)
{
    uint32_t val;

    if (mv == 1300) {
        val = PMU_DCDC_HPPA_1_3V;
    } else if (mv == 1350) {
        val = PMU_DCDC_HPPA_1_35V;
    } else if (mv == 1400) {
        val = PMU_DCDC_HPPA_1_4V;
    } else if (mv == 1450) {
        val = PMU_DCDC_HPPA_1_45V;
    } else if (mv == 1500) {
        val = PMU_DCDC_HPPA_1_5V;
    } else if (mv == 1600) {
        val = PMU_DCDC_HPPA_1_6V;
    } else if (mv == 1650) {
        val = PMU_DCDC_HPPA_1_65V;
    } else if (mv == 1700) {
        val = PMU_DCDC_HPPA_1_7V;
    } else if (mv == 1900) {
        val = PMU_DCDC_HPPA_1_9V;
    } else if (mv >= 1950) {
        val = PMU_DCDC_HPPA_1_95V;
    } else {
        val = PMU_DCDC_HPPA_1_8V;
    }

    return val;
}

POSSIBLY_UNUSED
static void pmu_dcdc_hppa_get_volt(unsigned short *normal_vp, unsigned short *dsleep_vp)
{
    unsigned short val;

    pmu_read(PMU_REG_DCDC_HPPA_VOLT, &val);
    if (normal_vp) {
        *normal_vp = GET_BITFIELD(val, REG_BUCK_HPPA_VBIT_NORMAL);
    }
    if (dsleep_vp) {
        *dsleep_vp = GET_BITFIELD(val, REG_BUCK_HPPA_VBIT_DSLEEP);
    }

#ifdef PMU_DCDC_CALIB
    if (normal_vp) {
        *normal_vp = pmu_reg_val_add(*normal_vp, -pmu_dcdc_hppa_comp, MAX_BUCK_VHPPA_BIT_VAL);
    }
    if (dsleep_vp) {
        *dsleep_vp = pmu_reg_val_add(*dsleep_vp, -pmu_hppa_lp_comp, MAX_BUCK_VHPPA_BIT_VAL);
    }
#endif
}

POSSIBLY_UNUSED
static void pmu_dcdc_hppa_set_volt(unsigned short normal_v,unsigned short dsleep_v)
{
    unsigned short val;

#ifdef PMU_DCDC_CALIB
    normal_v = pmu_reg_val_add(normal_v, pmu_dcdc_hppa_comp, MAX_BUCK_VHPPA_BIT_VAL);
    dsleep_v = pmu_reg_val_add(dsleep_v, pmu_hppa_lp_comp, MAX_BUCK_VHPPA_BIT_VAL);
#endif

    pmu_read(PMU_REG_DCDC_HPPA_VOLT, &val);
    val = SET_BITFIELD(val, REG_BUCK_HPPA_VBIT_DSLEEP, dsleep_v);
    val = SET_BITFIELD(val, REG_BUCK_HPPA_VBIT_NORMAL, normal_v);
    pmu_write(PMU_REG_DCDC_HPPA_VOLT, val);

    pmu_read(PMU_REG_VCODEC_VOLT_RC, &val);
    val = SET_BITFIELD(val, BUCK_VCODEC_VBIT_RC, dsleep_v);
    pmu_write(PMU_REG_VCODEC_VOLT_RC, val);

    if (normal_v > dsleep_v) {
        dcdc_ramp_map |= REG_BUCK_VCODEC_RAMP_EN;
    } else {
        dcdc_ramp_map &= ~REG_BUCK_VCODEC_RAMP_EN;
    }
}

static void pmu_dcdc_hppa_en(int enable)
{
#ifndef PMU_DCDC_SIMO_ENABLE
    uint16_t val;

    pmu_read(PMU_REG_DCDC_HPPA_EN, &val);
    if (enable) {
        val |= DCDC_HPPA_LP_EN_DSLEEP | PU_DCDC_HPPA_DR| PU_DCDC_HPPA_REG | PU_DCDC_HPPA_DSLEEP |
            REG_BUCK_HPPA_BURST_MODE_NORMAL | REG_BUCK_HPPA_BURST_MODE_DSLEEP;
    } else {
        val &= ~(DCDC_HPPA_LP_EN_DSLEEP | PU_DCDC_HPPA_DR| PU_DCDC_HPPA_REG | PU_DCDC_HPPA_DSLEEP |
            REG_BUCK_HPPA_BURST_MODE_NORMAL | REG_BUCK_HPPA_BURST_MODE_DSLEEP);
    }
    pmu_write(PMU_REG_DCDC_HPPA_EN, val);
#endif
}

static void pmu_hppa_dcdc_to_ldo(void)
{
    pmu_module_set_volt(PMU_CODEC, PMU_CODEC_1_8V, PMU_CODEC_1_8V);
    pmu_module_config(PMU_CODEC, PMU_MANUAL_MODE, PMU_LDO_ON, PMU_LP_MODE_ON, PMU_DSLEEP_MODE_OFF);
    hal_sys_timer_delay_us(PMU_LDO_PU_STABLE_TIME_US);
    pmu_dcdc_hppa_en(false);
}
#endif

static void BOOT_TEXT_FLASH_LOC pmu_dig_init_volt(void)
{
    uint16_t ldo_volt;
    uint16_t val;

    pmu_dig_get_target_volt(&ldo_volt, NULL);

    pmu_read(PMU_REG_DIG_CFG, &val);
    if (GET_BITFIELD(val, LDO_DIG_VBIT_NORMAL) < ldo_volt) {
        val = SET_BITFIELD(val, LDO_DIG_VBIT_NORMAL, ldo_volt);
        pmu_write(PMU_REG_DIG_CFG, val);
    }
}

int pmu_codec_volt_ramp_up(void)
{
    return 0;
}

int pmu_codec_volt_ramp_down(void)
{
    return 0;
}

#ifdef PMU_NTC_CTRL_ENABLE
static void pmu_ntc_temperature_config(int temperature)
{
    uint32_t lock;
    enum PMU_VCORE_REQ_T old_req;
    bool update = false;

    lock = int_lock();
    old_req = pmu_vcore_req;
    pmu_vcore_req &= ~(PMU_VCORE_NTC_COOL | PMU_VCORE_NTC_COLD | PMU_VCORE_NTC_CHILLY | PMU_VCORE_NTC_FREEZING);
    if (temperature <= 18) {
        if (temperature >= 10) {
             pmu_vcore_req |= PMU_VCORE_NTC_COOL;
        } else if (temperature >= 0) {
             pmu_vcore_req |= PMU_VCORE_NTC_COLD;
        } else if (temperature >= -10) {
             pmu_vcore_req |= PMU_VCORE_NTC_CHILLY;
        } else {
             pmu_vcore_req |= PMU_VCORE_NTC_FREEZING;
        }
    }
    if (old_req != pmu_vcore_req) {
        update = true;
    }
    int_unlock(lock);

    if (!update) {
        // Nothing changes
        return;
    }

    if (pmu_power_mode == PMU_POWER_MODE_DIG_DCDC) {
        pmu_dig_set_volt(PMU_POWER_MODE_NONE);
    }
}

static int pmu_raw2temperature(const uint16_t raw)
{
    uint16_t raw_cal = raw;
    static uint16_t ntc_tmep_ref = 0;
    int temp_centigrade = 25;

    if (ntc_tmep_ref == 0) {
        ntc_tmep_ref = pmu_ntc_temperature_reference_get();
    }

    if (pmu_power_mode != PMU_POWER_MODE_DIG_DCDC) {
        // The current of LDO is higher than that of DCDC, so the chip temperature will also be higher.
        // Need compensate an offset here
        raw_cal -= 28;
    }

    raw_cal = (raw_cal + PMU_NTC_RAW_DATA_TO_ONE_CENTIGRADE / 2);

    if (raw_cal >= ntc_tmep_ref) {
        temp_centigrade += ((raw_cal - ntc_tmep_ref) / PMU_NTC_RAW_DATA_TO_ONE_CENTIGRADE);
    } else {
        temp_centigrade -= ((ntc_tmep_ref - raw_cal) / PMU_NTC_RAW_DATA_TO_ONE_CENTIGRADE);
    }

    return temp_centigrade;
}

static void pmu_vcore_ntc_check(void)
{
    uint32_t s_time;
    uint16_t val;
    uint16_t val_bg_bak;
    uint16_t ntc_ref = pmu_ntc_temperature_reference_get();
    int temp;
    bool ntc_ref_normal_bg = true;

    // Save bandgap cfg.
    pmu_read(PMU_REG_BIAS_CFG, &val_bg_bak);

    pmu_get_efuse(PMU_EFUSE_PAGE_RESERVED_C, &val);
    val = GET_BITFIELD(val, PMU_EFUSE_C_FLASH_VDD_TYPE);

    // Use the same bandgap cfg as for factory NTC calibration.
    if (val != 0) {
        pmu_get_efuse(PMU_EFUSE_PAGE_RESERVED_7, &val);
        ntc_ref_normal_bg = !!(val & PMU_EFUSE_7_NTC_REF_CALIB_NORMAL_BG_FLAG);

        if (!ntc_ref_normal_bg) {
            //TRACE(0, "%s NTC use lp ref!", __func__);
            pmu_read(PMU_REG_BIAS_CFG, &val);
            val = (val & ~BG_VBG_SEL_REG) | BG_VBG_SEL_DR;
            pmu_write(PMU_REG_BIAS_CFG, val);
            hal_sys_timer_delay(MS_TO_TICKS(1));
        }
    }

    pmu_ntc_capture_start(NULL);

    s_time = hal_sys_timer_get();

    while (ntc_irq_busy && TICKS_TO_MS(hal_sys_timer_get() - s_time) < PMU_NTC_IRQ_TIMEOUT_MS);

    if (ntc_irq_busy) {
        pmu_ntc_capture_reset();
        TRACE(0, "%s NTC irq error!", __func__);
    } else {
        if (ntc_raw < ntc_ref) {
            temp = pmu_raw2temperature(ntc_raw);
            pmu_ntc_temperature_config(temp);
            TRACE(0, "%s temperature: %dC", __func__, temp);
        }
    }

    if (!ntc_ref_normal_bg) {
        pmu_write(PMU_REG_BIAS_CFG, val_bg_bak);
        hal_sys_timer_delay(MS_TO_TICKS(1));
    }
}
#endif
#endif

#if defined(ARM_CMSE) || defined(ARM_CMNS)
uint32_t TZ_pmu_get_vcore_req_S(void);

#ifdef ARM_CMSE
CMSE_API
uint32_t TZ_pmu_get_vcore_req_S(void)
{
#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
    pmu_ns_running = true;
#endif
    return pmu_vcore_req;
}
#endif
#endif

int BOOT_TEXT_FLASH_LOC pmu_open(void)
{
#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))

    uint16_t val;
    uint16_t POSSIBLY_UNUSED val_efuse;
    enum PMU_POWER_MODE_T mode;
    uint32_t codec_bbpll_freq;

    ASSERT(!vcodec_off || vcodec_mv == vhppa_mv,
        "Invalid vcodec/vhppa cfg: vcodec_off=%d vcodec_mv=%u vhppa_mv=%u", vcodec_off, vcodec_mv, vhppa_mv);
    ASSERT(vcodec_mv == 1300 || vcodec_mv == 1350 || vcodec_mv == 1400 || vcodec_mv == 1450 || vcodec_mv == 1500 || vcodec_mv == 1650 ||
            vcodec_mv == 1700 || vcodec_mv == 1800 || vcodec_mv == 1900 || vcodec_mv == 1950, "Invalid vcodec cfg: vcodec_mv=%u", vcodec_mv);
    ASSERT(vhppa_mv == 1300 || vhppa_mv == 1350 || vhppa_mv == 1400 || vhppa_mv == 1450 || vhppa_mv == 1500 || vhppa_mv == 1650 ||
            vhppa_mv == 1700 || vhppa_mv == 1800 || vhppa_mv == 1900 || vhppa_mv == 1950, "Invalid vhppa cfg: vhppa_mv=%u", vhppa_mv);
    TRACE(0, "%s vcodec_mv:%d vhppa_mv:%d", __func__, vcodec_mv, vhppa_mv);

#ifdef ARM_CMNS
    pmu_vcore_req = TZ_pmu_get_vcore_req_S();
#endif

#if defined(PMU_EFUSE_CFG_EMA_VCORE) && defined(PMU_SAFE_VOLT_MIN)
    TRACE(0, "pmu_ema_cfg:0x%x", pmu_ema_cfg);
#endif

#ifdef PMU_WAFER_PRINT
    pmu_get_efuse(PMU_EFUSE_PAGE_CP_WAFER_ID, &val);
    TRACE(0, "wafer_ctx_print: wafer_ID=%d", GET_BITFIELD(val, PMU_EFUSE_CHIP_WAFER_ID));

    pmu_get_efuse(PMU_EFUSE_PAGE_CP_XY_COORDINATE, &val);
    TRACE(0, "wafer_ctx_print: wafer_X=%d, wafer_Y=%d",
        GET_BITFIELD(val, PMU_EFUSE_CHIP_WAFER_COORDINATE_X), GET_BITFIELD(val, PMU_EFUSE_CHIP_WAFER_COORDINATE_Y));
#endif

#if !defined(ROM_BUILD) && defined(PMU_EFUSE_INIT_IN_BOOT)
    pmu_efuse_init();
#endif

    // Disable and clear all PMU irqs by default
    pmu_write(PMU_REG_INT_MASK, 0);
    pmu_write(PMU_REG_INT_EN, 0);
    // PMU irqs cannot be cleared by PMU soft reset
    pmu_read(PMU_REG_CHARGER_STATUS, &val);
    pmu_write(PMU_REG_CHARGER_STATUS, val);
    pmu_read(PMU_REG_INT_STATUS, &val);
    pmu_write(PMU_REG_INT_CLR, val);

    // Allow PMU to sleep when power key is pressed
    pmu_read(PMU_REG_POWER_KEY_CFG, &val);
    val &= ~POWERKEY_WAKEUP_OSC_EN;
    pmu_write(PMU_REG_POWER_KEY_CFG, val);

    // Increase big bandgap startup time (stable time)
    pmu_read(PMU_REG_CRYSTAL_CFG, &val);
    val = SET_BITFIELD(val, REG_VCORE_SSTIME_MODE, 2);
    pmu_write(PMU_REG_CRYSTAL_CFG, val);

#ifdef PMU_FORCE_LP_MODE
    pmu_force_lp_bg_valid_check();
#else
    pmu_read(PMU_REG_BIAS_CFG, &val);
#ifdef FORCE_BIG_BANDGAP
    val |= PU_BIAS_LDO_DR | BG_VBG_SEL_DR | BG_CONSTANT_GM_BIAS_DR | BG_CORE_EN_DR | BG_VTOI_EN_DR;
#else
    // Allow low power bandgap
    val &= ~BG_VBG_SEL_DR;
#endif
    pmu_write(PMU_REG_BIAS_CFG, val);
#endif

#ifdef DIG_DCDC_MODE
    // Decrease voltage drop when ldo switch to dcdc
    pmu_read(PMU_REG_BIAS_CFG, &val);
    val = (val & ~BG_NOTCH_EN_REG) | BG_NOTCH_EN_DR;
    pmu_write(PMU_REG_BIAS_CFG, val);

    pmu_read(PMU_REG_BIAS_CFG2, &val);
    val &= ~BG_NOTCH_LPF_LOW_BW;
    pmu_write(PMU_REG_BIAS_CFG2, val);

    pmu_read(PMU_REG_CODEC_CFG, &val);
    val |= LP_EN_VCODEC_LDO_DR | LP_EN_VCODEC_LDO_REG;
    pmu_write(PMU_REG_CODEC_CFG, val);

    pmu_read(PMU_REG_ANA_CFG, &val);
    val |= LP_EN_VANA_LDO_DR | LP_EN_VANA_LDO_REG;
    pmu_write(PMU_REG_ANA_CFG, val);

    pmu_read(PMU_REG_DIG_CFG, &val);
    val |= LP_EN_VCORE_LDO_DR | LP_EN_VCORE_LDO_REG;
    pmu_write(PMU_REG_DIG_CFG, val);
#endif

    // Enable GPADC
    pmu_read(PMU_REG_SAR_EN, &val);
    val |= REG_SAR_BUF_EN;
    pmu_write(PMU_REG_SAR_EN, val);

    pmu_read(PMU_REG_SAR_GPADC_CFG, &val);
    val |= REG_CLK_GPADC_EN;
    pmu_write(PMU_REG_SAR_GPADC_CFG, val);

    pmu_read(PMU_REG_SAR_VREF_CFG, &val);
    val = SET_BITFIELD(val, REG_SAR_VREG_SEL, 0x2D);
    pmu_write(PMU_REG_SAR_VREF_CFG, val);

    pmu_read(PMU_REG_LPO_CFG, &val);
    val = SET_BITFIELD(val, REG_LPO_ITRIM, 0x3);
    val = SET_BITFIELD(val, REG_LPO_KTRIM, 0x0);
    pmu_write(PMU_REG_LPO_CFG, val);

#ifdef PMU_IRQ_UNIFIED
    pmu_read(PMU_REG_WDT_CFG, &val);
    val |= POWERON_DETECT_EN | MERGE_INTR;
    pmu_write(PMU_REG_WDT_CFG, val);
#endif

#ifdef PMU_DCDC_CALIB
    pmu_get_dcdc_calib_value();
#endif

    pmu_get_efuse(PMU_EFUSE_PAGE_SW_CFG, &val);
    gpadc_calib_vbat_slim = !!(val & PMU_EFUSE_5_GPADC_CALIB_CH6_VAL_SLIM);
    TRACE(0, "gpadc_calib_vbat_slim=%d", gpadc_calib_vbat_slim);

#ifndef NO_SLEEP
    pmu_sleep_en(1);  //enable sleep
#endif

#ifdef DIG_DCDC_MODE
    // dcdc configuration
    pmu_read(PMU_REG_DCDC_RAMP_EN, &val);
    val &= ~(REG_BUCK_VANA_RAMP_EN | REG_BUCK_VCORE_RAMP_EN | REG_BUCK_VCODEC_RAMP_EN);
    pmu_write(PMU_REG_DCDC_RAMP_EN, val);

    if (hal_get_chip_metal_id() > HAL_CHIP_METAL_ID_0) {
        // Optimize deep sleep current by burst threshold calibration.
        pmu_get_efuse(PMU_EFUSE_PAGE_CP_WAFER_ID, &val_efuse);
        val_efuse = GET_BITFIELD(val_efuse, PMU_EFUSE_BURST_THRESHOLD_CAL);
#ifdef PMU_DCDC_PWM
        val_efuse = val_efuse > 0 ? val_efuse : 0x3A;
#else
        val_efuse = val_efuse > 0 ? val_efuse : 0x3C;
#endif
        pmu_read(PMU_REG_DCDC_CFG_C4, &val);
        val = SET_BITFIELD(val, REG_BUCK_ANA_BURST_THRESHOLD, val_efuse);
        pmu_write(PMU_REG_DCDC_CFG_C4, val);
    }

    // Enhance current for ireverse_vcomp
    pmu_read(PMU_REG_BUCK_CFG_D5, &val);
    val |= (1 << 3);
    pmu_write(PMU_REG_BUCK_CFG_D5, val);

    pmu_read(PMU_REG_DCDC_ANA_DIG_CFG_63, &val);
    val = SET_BITFIELD(val, REG_BUCK_PRIOR_CHANNEL, 0x2);
    val |= REG_BUCK_ANA_BURST_MODE_DSLEEP;
#ifdef PMU_DCDC_PWM
    val &= ~REG_BUCK_ANA_BURST_MODE_NORMAL;
#else
    val |= REG_BUCK_ANA_BURST_MODE_NORMAL;
#endif
    pmu_write(PMU_REG_DCDC_ANA_DIG_CFG_63, val);

    pmu_read(PMU_REG_DCDC_ANA_CFG_49, &val);
    val = SET_BITFIELD(val, REG_BUCK_LP_VCOMP2, 0x1);
    val = (val & ~REG_BUCK_ANA_LP_VCOMP) | REG_BUCK_SLOPE_HALF_ENB;
    pmu_write(PMU_REG_DCDC_ANA_CFG_49, val);

    pmu_read(PMU_REG_DCDC_V2I_BIT_CFG, &val);
    val = SET_BITFIELD(val, REG_BUCK_I2V_BIT2, 0x1);
    val = SET_BITFIELD(val, REG_BUCK_I2V_BIT1, 0x1);
    pmu_write(PMU_REG_DCDC_V2I_BIT_CFG, val);

    pmu_read(PMU_REG_DCDC_ANA_CFG_15, &val);
    val |= REG_BUCK_ANA_SLOPE_DOUBLE;
#ifdef PMU_DCDC_PWM
    val &= ~REG_BUCK_ANA_SYNC_DISABLE;
    hal_cmu_dcdc_clock_enable(HAL_CMU_DCDC_CLOCK_USER_DCDC_SYNC);
#else
    val |= REG_BUCK_ANA_SYNC_DISABLE;
#endif
    val = SET_BITFIELD(val, REG_BUCK_PDRV_BIT, 0x2);
    pmu_write(PMU_REG_DCDC_ANA_CFG_15, val);

    pmu_read(PMU_REG_AVDD_EN, &val);
    val = SET_BITFIELD(val, REG_BUCK_CC_ILIMIT, 0xC);
    pmu_write(PMU_REG_AVDD_EN, val);

    pmu_read(PMU_REG_RC_SLEEP_CFG, &val);
    val |= REG_BUCK_ANA_BURST_MODE_RC;
    pmu_write(PMU_REG_RC_SLEEP_CFG, val);

    // normal/dlseep volt ramp step
    pmu_read(PMU_REG_SAR_VREF_CFG, &val);
    val = SET_BITFIELD(val, REG_VBUCK_RAMP_STEP, 0x1);
    pmu_write(PMU_REG_SAR_VREF_CFG, val);
#endif

#ifndef HPPA_LDO_ON
    // enable vcodec dcdc
    val = pmu_dcdc_hppa_mv_to_val(vhppa_mv);
    pmu_dcdc_hppa_set_volt(val, val);
#ifndef PMU_DCDC_SIMO_ENABLE
    pmu_dcdc_hppa_en(true);
    hal_sys_timer_delay_us(PMU_DCDC_PU_STABLE_TIME_US);
    //disable vcodec ldo
    pmu_module_config(PMU_CODEC, PMU_MANUAL_MODE, PMU_LDO_OFF, PMU_LP_MODE_ON, PMU_DSLEEP_MODE_OFF);
#endif
#endif

#ifdef DIG_DCDC_MODE
    mode = PMU_POWER_MODE_DIG_DCDC;
#elif defined(ANA_DCDC_MODE)
#if defined(PMU_DCDC_SIMO_ENABLE)
#error "Invalid config! DCDC SIMO is enabled so chip don't support ANA_DCDC_MODE"
#endif
    mode = PMU_POWER_MODE_ANA_DCDC;
#else // LDO_MODE
    mode = PMU_POWER_MODE_LDO;
#endif

    pmu_codec_mic_bias_set_volt(AUD_VMIC_MAP_VMIC1 | AUD_VMIC_MAP_VMIC2, 1800);

#ifdef PMU_NTC_CTRL_ENABLE
    pmu_vcore_ntc_check();
#endif

    pmu_mode_change(mode);

    pmu_module_config(PMU_SENSOR, PMU_MANUAL_MODE, PMU_LDO_OFF, PMU_LP_MODE_ON, PMU_DSLEEP_MODE_OFF);

#ifdef PMU_FORCE_LP_MODE
    if (hal_get_chip_metal_id() == HAL_CHIP_METAL_ID_0) {
        ASSERT(0, "Don't support force lp bandgap for metal 0 chips!");
    }

    if (pmu_force_lp_bg_valid) {
        // Force LP mode
        pmu_module_force_lp_config();
        pmu_codec_mic_bias_lowpower_mode(AUD_VMIC_MAP_VMIC1 | AUD_VMIC_MAP_VMIC2, true);
        hal_sys_timer_delay(MS_TO_TICKS(1));

        // Reduce LDO voltage ripple
        pmu_read(PMU_REG_AVDD_EN, &val);
        val = SET_BITFIELD(val, REG_LP_BIAS_SEL_LDO, 0);
        pmu_write(PMU_REG_AVDD_EN, val);

        // Switch to little bandgap
        pmu_read(PMU_REG_BIAS_CFG, &val);
        val = (val & ~BG_VBG_SEL_REG) | BG_VBG_SEL_DR;
        pmu_write(PMU_REG_BIAS_CFG, val);
        hal_sys_timer_delay(MS_TO_TICKS(1));

        if (big_bandgap_user_map == 0) {
            // Disable big bandgap
            val = (val & ~(PU_BIAS_LDO_REG | BG_CONSTANT_GM_BIAS_REG | BG_CORE_EN_REG | BG_VTOI_EN_REG)) |
                PU_BIAS_LDO_DR | BG_CONSTANT_GM_BIAS_DR | BG_CORE_EN_DR | BG_VTOI_EN_DR;
            pmu_write(PMU_REG_BIAS_CFG, val);
        }

        pmu_force_lp_bg_init_done = true;
    } else {
        pmu_read(PMU_REG_BIAS_CFG, &val);
        val &= ~BG_VBG_SEL_DR;
        pmu_write(PMU_REG_BIAS_CFG, val);
    }
#endif

    pmu_read(PMU_REG_LED_CFG_IO2, &val);
    val &= ~(REG_LED_IO2_PDEN | REG_LED_IO2_PUEN);
    pmu_write(PMU_REG_LED_CFG_IO2, val);

    pmu_read(PMU_REG_LED_CFG_IO1, &val);
    val &= ~(REG_LED_IO1_PDEN | REG_LED_IO1_PUEN);
    pmu_write(PMU_REG_LED_CFG_IO1, val);

#ifdef RTC_ENABLE
    pmu_rtc_restore_context();
#endif

#if defined(MCU_HIGH_PERFORMANCE_MODE)
    codec_bbpll_freq = high_perf_freq_mhz * 1000000 / CODEC_PLL_DIV;
#else
    codec_bbpll_freq = BBPLL_FREQ_MHZ * 1000000 / CODEC_PLL_DIV;
#endif
    bbpll_auto_calib_spd(codec_bbpll_freq, CODEC_PLL_DIV);

#if defined(MCU_HIGH_PERFORMANCE_MODE)
    pmu_high_performance_mode_enable(true);
#endif

#ifndef ARM_CMSE
    charger_charge_open();
    if (charger_package_type_get() == CHARGER_PACKAGE_TYPE_SPLIT_ACIN_UARTCOM) {
        pmu_ldo_vsensor_enable(PMU_VSENSOR_USER_SINGLE_WIRE_UART);
    }
#endif

#ifdef ACIN_PATTERN_DISABLE
    pmu_charger_pattern_reset_enable(false);
#else
    pmu_charger_pattern_reset_enable(true);
#endif

#ifdef CHG_PRESENT_ENABLE
    pmu_charger_present_init();
#endif

    TRACE(0, "pmu_boot_cause_get=0x%x first_pwron=%d", pmu_boot_cause_get(), pmu_boot_first_power_up());
#endif

    return 0;
}

#ifdef PMU_FORCE_LP_MODE
static void pmu_bandgap_cfg_reset(void)
{
    uint16_t val;

    if (pmu_force_lp_bg_valid) {
        pmu_read(PMU_REG_BIAS_CFG, &val);
        val |= PU_BIAS_LDO_DR | BG_CONSTANT_GM_BIAS_DR | BG_CORE_EN_DR | BG_VTOI_EN_DR | BG_NOTCH_EN_DR | BG_VBG_SEL_DR |
            PU_BIAS_LDO_REG | BG_CONSTANT_GM_BIAS_REG | BG_CORE_EN_REG | BG_VTOI_EN_REG | BG_NOTCH_EN_REG;
        pmu_write(PMU_REG_BIAS_CFG, val);
        hal_sys_timer_delay_us(PMU_BIG_BG_STABLE_TIME_US);

        val |= BG_VBG_SEL_REG;
        pmu_write(PMU_REG_BIAS_CFG, val);
    }
}
#endif

void pmu_big_bandgap_enable(enum PMU_BIG_BANDGAP_USER_T user, int enable)
{
#ifdef PMU_FORCE_LP_MODE
    uint16_t val;
    bool update = false;

    if (pmu_force_lp_bg_valid && pmu_force_lp_bg_init_done) {
        if (enable) {
            if (big_bandgap_user_map == 0) {
                update = true;
            }
            big_bandgap_user_map |= user;
        } else {
            if (big_bandgap_user_map & user) {
                big_bandgap_user_map &= ~user;
                if (big_bandgap_user_map == 0) {
                    update = true;
                }
            }
        }

        if (!update) {
            return;
        }

        pmu_read(PMU_REG_BIAS_CFG, &val);
        val |= BG_CONSTANT_GM_BIAS_DR | BG_CORE_EN_DR | BG_VTOI_EN_DR;
        if (enable) {
            val |= BG_CONSTANT_GM_BIAS_REG | BG_CORE_EN_REG | BG_VTOI_EN_REG;
        } else {
            val &= ~(BG_CONSTANT_GM_BIAS_REG | BG_CORE_EN_REG | BG_VTOI_EN_REG);
        }
        pmu_write(PMU_REG_BIAS_CFG, val);
        if (enable) {
            hal_sys_timer_delay_us(PMU_BIG_BG_STABLE_TIME_US);
        }
    }
#endif
}

void pmu_sleep(void)
{
    uint16_t val;

    if (dcdc_ramp_map) {
        // Enable DCDC ramp
        pmu_read(PMU_REG_DCDC_RAMP_EN, &val);
        val |= dcdc_ramp_map;
        pmu_write(PMU_REG_DCDC_RAMP_EN, val);
    }
}

void pmu_wakeup(void)
{
    uint16_t val;

    if (dcdc_ramp_map) {
        // Disable DCDC ramp
        pmu_read(PMU_REG_DCDC_RAMP_EN, &val);
        val &= ~dcdc_ramp_map;
        pmu_write(PMU_REG_DCDC_RAMP_EN, val);
    }
}

void pmu_codec_config(int enable)
{
#ifdef HPPA_LDO_ON
    if (!vcodec_off) {
        if (enable) {
            pmu_module_config(PMU_CODEC,PMU_MANUAL_MODE,PMU_LDO_ON,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_OFF);
        } else {
            pmu_module_config(PMU_CODEC,PMU_MANUAL_MODE,PMU_LDO_OFF,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_OFF);
        }
    }
#endif
}

void pmu_codec_hppa_enable(int enable)
{
}

void pmu_codec_mic_bias_set_volt(uint32_t map, uint32_t mv)
{
    uint16_t val;
    int i;
    enum PMU_REG_T bias_reg;
    uint8_t volt = 0;
    uint8_t res = 0;
    uint8_t step = 0;

    step = 40;

    if (mv <= 1600) {
        volt = PMU_VMIC_1_6V;
    } else {
        volt = PMU_VMIC_1_6V + (mv - 1600) / step;
        if (volt > PMU_VMIC_2_8V) {
            volt = PMU_VMIC_2_8V;
        }
    }

    if (vcodec_mv < 1800) {
        volt += 3;
    }

    if (volt > MAX_VMIC_BIAS_VSEL_BIT_VAL) {
        volt = MAX_VMIC_BIAS_VSEL_BIT_VAL;
    }

    res = PMU_VMIC_RES_2_8V;

    pmu_read(PMU_REG_MIC_BIAS_A, &val);
    val = (val & ~(REG_MIC_LDO_RES_MASK)) | REG_MIC_LDO_RES(res);
    pmu_write(PMU_REG_MIC_BIAS_A, val);

    for (i = 0; i < MAX_VMIC_CH_NUM; i++) {
        if ((map & (AUD_VMIC_MAP_VMIC1 << i)) == 0) {
            continue;
        }
        bias_reg = PMU_REG_MIC_BIAS_A + i;
        pmu_read(bias_reg, &val);
        val = (val & ~(REG_MIC_BIASA_VSEL_MASK)) | REG_MIC_BIASA_VSEL(volt);
        pmu_write(bias_reg, val);
    }
}

void pmu_codec_mic_bias_enable(uint32_t map, int enable)
{
    int i;
    enum PMU_REG_T bias_reg;
    uint16_t val;
    uint16_t val_temp = 0;
    static bool reg_mic_ldo_en = false;
    static int need_ldo_on_cnt = 0;

    for (i = 0; i < MAX_VMIC_CH_NUM; i++) {
        if ((map & (AUD_VMIC_MAP_VMIC1 << i)) == 0) {
            continue;
        }
        bias_reg = PMU_REG_MIC_BIAS_A + i;
        pmu_read(bias_reg, &val);
        if (enable){
            val |= REG_MIC_BIASA_EN;
            need_ldo_on_cnt++;
        } else {
            val &= ~REG_MIC_BIASA_EN;
            need_ldo_on_cnt--;
        }
        pmu_write(bias_reg, val);
    }

    if (!reg_mic_ldo_en && need_ldo_on_cnt) {
        pmu_read(PMU_REG_MIC_BIAS_B, &val);
        val |= REG_MIC_LDO_EN;
        pmu_write(PMU_REG_MIC_BIAS_B, val);
        reg_mic_ldo_en = true;
    } else if (need_ldo_on_cnt == 0) {
        pmu_read(PMU_REG_MIC_BIAS_B, &val);
        val &= ~REG_MIC_LDO_EN;
        pmu_write(PMU_REG_MIC_BIAS_B, val);
        reg_mic_ldo_en = false;
    }

    if (!enable) {
        pmu_read(PMU_REG_MIC_PULL_DOWN, &val);
        val_temp = val;
        if (map & AUD_VMIC_MAP_VMIC1) {
            val_temp |= REG_MIC_BIASA_PULLDOWN;
        }
        if (map & AUD_VMIC_MAP_VMIC2) {
            val_temp |= REG_MIC_BIASB_PULLDOWN;
        }
        pmu_write(PMU_REG_MIC_PULL_DOWN, val_temp);
        hal_sys_timer_delay_us(PMU_LDO_PULLDOWN_STABLE_TIME_US);
        // need clear pull_down
        pmu_write(PMU_REG_MIC_PULL_DOWN, val);
    }
}

void pmu_codec_mic_bias_lowpower_mode(uint32_t map, int enable)
{
    uint16_t val;

    if (map & (AUD_VMIC_MAP_VMIC1 | AUD_VMIC_MAP_VMIC2)) {
        pmu_read(PMU_REG_MIC_PULL_DOWN, &val);
        if (enable) {
            val |= REG_MIC_LP_ENABLE;
        } else {
            val &= ~REG_MIC_LP_ENABLE;
        }
        pmu_write(PMU_REG_MIC_PULL_DOWN, val);
    }
}

SRAM_TEXT_LOC void pmu_flash_write_config(void)
{
#ifdef FLASH_WRITE_AT_HIGH_VCORE
    uint32_t lock;

    if (pmu_vcore_req & PMU_VCORE_FLASH_WRITE_ENABLED) {
        return;
    }

    lock = int_lock();
    pmu_vcore_req |= PMU_VCORE_FLASH_WRITE_ENABLED;
    int_unlock(lock);

    pmu_dig_set_volt(PMU_POWER_MODE_NONE);
#endif
}

SRAM_TEXT_LOC void pmu_flash_read_config(void)
{
#ifdef FLASH_WRITE_AT_HIGH_VCORE
    uint32_t lock;

    if ((pmu_vcore_req & PMU_VCORE_FLASH_WRITE_ENABLED) == 0) {
        return;
    }

    lock = int_lock();
    pmu_vcore_req &= ~PMU_VCORE_FLASH_WRITE_ENABLED;
    int_unlock(lock);

    pmu_dig_set_volt(PMU_POWER_MODE_NONE);
#endif
}

void BOOT_TEXT_FLASH_LOC pmu_flash_freq_config(uint32_t freq)
{
#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
    uint32_t lock;

    lock = int_lock();
    if (freq > 52000000) {
        // The real max freq is 120M
        //pmu_vcore_req |= PMU_VCORE_FLASH_FREQ_HIGH;
    } else {
        pmu_vcore_req &= ~PMU_VCORE_FLASH_FREQ_HIGH;
    }
    int_unlock(lock);

    if (pmu_power_mode == PMU_POWER_MODE_NONE) {
        // PMU not init yet
        pmu_dig_init_volt();
        return;
    }

    pmu_dig_set_volt(PMU_POWER_MODE_NONE);
#endif
}

void BOOT_TEXT_FLASH_LOC pmu_psram_freq_config(uint32_t freq)
{
}

void pmu_anc_config(int enable)
{
}

void pmu_fir_high_speed_config(int enable)
{
}

void pmu_iir_freq_config(uint32_t freq)
{
    uint32_t lock;

    lock = int_lock();
    if (freq >= 60000000) {
        pmu_vcore_req |= PMU_VCORE_IIR_FREQ_HIGH;
    } else {
        pmu_vcore_req &= ~PMU_VCORE_IIR_FREQ_HIGH;
    }
    int_unlock(lock);

    pmu_dig_set_volt(PMU_POWER_MODE_NONE);
}

void pmu_iir_eq_freq_config(uint32_t freq)
{
    uint32_t lock;

    lock = int_lock();
    if (freq >= 96000000) {
        pmu_vcore_req |= PMU_VCORE_IIR_EQ_FREQ_HIGH;
    } else {
        pmu_vcore_req &= ~PMU_VCORE_IIR_EQ_FREQ_HIGH;
    }
    int_unlock(lock);

    pmu_dig_set_volt(PMU_POWER_MODE_NONE);
}

void pmu_rs_freq_config(uint32_t freq)
{
    uint32_t lock;

    lock = int_lock();
    if (freq >= 60000000) {
        pmu_vcore_req |= PMU_VCORE_RS_FREQ_HIGH;
    } else {
        pmu_vcore_req &= ~PMU_VCORE_RS_FREQ_HIGH;
    }
    int_unlock(lock);

    pmu_dig_set_volt(PMU_POWER_MODE_NONE);
}

void BOOT_TEXT_SRAM_LOC pmu_sys_freq_config(enum HAL_CMU_FREQ_T freq)
{
#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
#if defined(MCU_HIGH_PERFORMANCE_MODE) || defined(ULTRA_LOW_POWER) || !defined(OSC_26M_X4_AUD2BB)
    uint32_t lock;
    enum PMU_VCORE_REQ_T old_req;
    bool update = false;

#ifdef ARM_CMSE
    ASSERT(!pmu_ns_running, "pmu_sys_freq_config: NS is running");
#endif

    lock = int_lock();
    old_req = pmu_vcore_req;
    pmu_vcore_req &= ~(PMU_VCORE_SYS_FREQ_HIGH | PMU_VCORE_SYS_FREQ_MEDIUM | PMU_VCORE_SYS_FREQ_MEDIUM_LOW);
#if defined(MCU_HIGH_PERFORMANCE_MODE)
    if (freq > HAL_CMU_FREQ_104M) {
        if (high_perf_on) {
            // The real freq is 350M
            pmu_vcore_req |= PMU_VCORE_SYS_FREQ_HIGH;
        } else {
            pmu_vcore_req |= PMU_VCORE_SYS_FREQ_MEDIUM;
        }
    } else {
#ifndef OSC_26M_X4_AUD2BB
        if (freq == HAL_CMU_FREQ_104M) {
            // The real freq is 200M
            pmu_vcore_req |= PMU_VCORE_SYS_FREQ_MEDIUM;
        }
#endif
    }
#else
    if (freq > HAL_CMU_FREQ_104M) {
        pmu_vcore_req |= PMU_VCORE_SYS_FREQ_MEDIUM;
    } else if (freq == HAL_CMU_FREQ_104M) {
        pmu_vcore_req |= PMU_VCORE_SYS_FREQ_MEDIUM_LOW;
    }
#endif
    if (old_req != pmu_vcore_req) {
        update = true;
    }
    int_unlock(lock);

    if (!update) {
        // Nothing changes
        return;
    }

    if (pmu_power_mode == PMU_POWER_MODE_NONE) {
        // PMU not init yet
        pmu_dig_init_volt();
        return;
    }

    pmu_dig_set_volt(PMU_POWER_MODE_NONE);
#endif
#endif
}

void pmu_high_performance_mode_enable(bool enable)
{
#if defined(MCU_HIGH_PERFORMANCE_MODE)
    uint16_t val;

    if (high_perf_on == enable) {
        return;
    }
    high_perf_on = enable;

    if (!enable) {
        if (high_perf_freq_mhz > 250) {
            // Switch to 52M to avoid using PLL
            hal_cmu_sys_set_freq(HAL_CMU_FREQ_52M);
            // Restore the default div
            rf_read(RF_REG_170, &val);
            val |= REG_LP_BBPLL_DIG_CLK_FREQ_SEL; // div=2 (bit7=1)
            rf_write(RF_REG_170, val);
            // Restore the sys freq
            hal_cmu_sys_set_freq(hal_sysfreq_get_hw_freq());
        }
        // Restore the default PLL freq (384M)
        bbpll_freq_pll_config(192 * 1000000 * 2);
    }

    pmu_sys_freq_config(hal_sysfreq_get_hw_freq());

    if (enable) {
        uint32_t pll_freq;

        // Change freq first, and then change divider.
        // Otherwise there will be an instant very high freq sent to digital domain.

        if (high_perf_freq_mhz <= 250) {
            pll_freq = high_perf_freq_mhz * 1000000 * 2;
        } else {
            pll_freq = high_perf_freq_mhz * 1000000;
        }
        bbpll_freq_pll_config(pll_freq);

        if (high_perf_freq_mhz > 250) {
            // Switch to 52M to avoid using PLL
            hal_cmu_sys_set_freq(HAL_CMU_FREQ_52M);
            rf_read(RF_REG_170, &val);
            val &= ~REG_LP_BBPLL_DIG_CLK_FREQ_SEL; // div=1 (bit7=0)
            rf_write(RF_REG_170, val);
            // Restore the sys freq
            hal_cmu_sys_set_freq(hal_sysfreq_get_hw_freq());
        }
    }
#endif
}

void pmu_usb_config(enum PMU_USB_CONFIG_TYPE_T type)
{

}

#if !defined(FPGA) && !defined(PROGRAMMER)
struct PMU_CHG_CTX_T {
    uint16_t pmu_chg_status;
};

struct PMU_CHG_CTX_T BOOT_BSS_LOC pmu_chg_ctx;

void BOOT_TEXT_SRAM_LOC pmu_charger_save_context(void)
{
    pmu_read(PMU_REG_CHARGER_STATUS, &pmu_chg_ctx.pmu_chg_status);
}

enum PMU_POWER_ON_CAUSE_T pmu_charger_poweron_status(void)
{
    enum PMU_POWER_ON_CAUSE_T pmu_power_on_cause = PMU_POWER_ON_CAUSE_NONE;

    if (pmu_chg_ctx.pmu_chg_status & AC_ON_DET_OUT){
        pmu_power_on_cause = PMU_POWER_ON_CAUSE_CHARGER_ACOFF;
    }else if (pmu_chg_ctx.pmu_chg_status & AC_ON){
        pmu_power_on_cause = PMU_POWER_ON_CAUSE_CHARGER_ACON;
    }
    return pmu_power_on_cause;
}
#endif

void pmu_charger_init(void)
{
    unsigned short readval_cfg;
    uint32_t lock;

    lock = int_lock();
    pmu_read(PMU_REG_CHARGER_CFG, &readval_cfg);
    readval_cfg &= ~(REG_CHARGE_OUT_INTR_MSK | REG_CHARGE_IN_INTR_MSK |
        REG_AC_ON_OUT_EN | REG_AC_ON_IN_EN);
    pmu_write(PMU_REG_CHARGER_CFG ,readval_cfg);
    int_unlock(lock);

    hal_sys_timer_delay(MS_TO_TICKS(1));

    lock = int_lock();
    pmu_read(PMU_REG_CHARGER_CFG, &readval_cfg);
    readval_cfg |= REG_AC_ON_OUT_EN | REG_AC_ON_IN_EN;
    readval_cfg = SET_BITFIELD(readval_cfg, REG_AC_ON_DB_VALUE, 8);
    pmu_write(PMU_REG_CHARGER_CFG ,readval_cfg);
    int_unlock(lock);
}

static void pmu_charger_irq_handler(PMU_IRQ_HDLR_PARAM)
{
    enum PMU_CHARGER_STATUS_T status = PMU_CHARGER_UNKNOWN;
    unsigned short readval;

#ifdef PMU_IRQ_UNIFIED
    readval = irq_status;
#else
    uint32_t lock;

    lock = int_lock();
    pmu_read(PMU_REG_CHARGER_STATUS, &readval);
    pmu_write(PMU_REG_CHARGER_STATUS, readval);
    int_unlock(lock);
#endif
    PMU_DEBUG_TRACE(3,"%s REG_%02X=0x%04X", __func__, PMU_REG_CHARGER_STATUS, readval);

    if ((readval & (INTR_MSKED_CHARGE_IN | INTR_MSKED_CHARGE_OUT)) == 0){
        PMU_DEBUG_TRACE(1,"%s SKIP", __func__);
        return;
    } else if ((readval & (INTR_MSKED_CHARGE_IN | INTR_MSKED_CHARGE_OUT)) ==
            (INTR_MSKED_CHARGE_IN | INTR_MSKED_CHARGE_OUT)) {
        PMU_DEBUG_TRACE(1,"%s DITHERING", __func__);
        hal_sys_timer_delay(2);
    } else {
        PMU_DEBUG_TRACE(1,"%s NORMAL", __func__);
    }

    status = pmu_charger_get_status();

    if (charger_irq_handler) {
        charger_irq_handler(status);
    }
}

void pmu_charger_set_irq_handler(PMU_CHARGER_IRQ_HANDLER_T handler)
{
    uint32_t lock;
    uint16_t val;

    charger_irq_handler = handler;

    lock = int_lock();
    pmu_read(PMU_REG_CHARGER_CFG, &val);
    if (handler) {
        val |= REG_CHARGE_IN_INTR_MSK | REG_CHARGE_OUT_INTR_MSK;
    } else {
        val &= ~(REG_CHARGE_IN_INTR_MSK | REG_CHARGE_OUT_INTR_MSK);
    }
    pmu_write(PMU_REG_CHARGER_CFG, val);

#ifdef PMU_IRQ_UNIFIED
   pmu_set_irq_unified_handler(PMU_IRQ_TYPE_CHARGER, handler ? pmu_charger_irq_handler : NULL);
#else
    if (handler) {
        NVIC_SetVector(CHARGER_IRQn, (uint32_t)pmu_charger_irq_handler);
        NVIC_SetPriority(CHARGER_IRQn, IRQ_PRIORITY_NORMAL);
        NVIC_ClearPendingIRQ(CHARGER_IRQn);
        NVIC_EnableIRQ(CHARGER_IRQn);
    } else {
        NVIC_DisableIRQ(CHARGER_IRQn);
    }
#endif
    int_unlock(lock);
}

void pmu_charger_plugin_config(void)
{
}

void pmu_charger_plugout_config(void)
{
}

enum PMU_CHARGER_STATUS_T pmu_charger_get_status(void)
{
    unsigned short readval;
    enum PMU_CHARGER_STATUS_T status;

    pmu_read(PMU_REG_CHARGER_STATUS, &readval);
    if (readval & AC_ON)
        status = PMU_CHARGER_PLUGIN;
    else
        status = PMU_CHARGER_PLUGOUT;

    return status;
}

#ifdef CHG_PRESENT_ENABLE
static void pmu_charger_present_init(void)
{
    unsigned short readval_cfg;
    uint32_t lock;

    lock = int_lock();
    pmu_read(PMU_REG_AC_PRESENT_CFG, &readval_cfg);
    readval_cfg &= ~(AC_PRESENT_N_INTR_MSK | AC_PRESENT_P_INTR_MSK | AC_PRESENT_INTR_EN);
    pmu_write(PMU_REG_AC_PRESENT_CFG, readval_cfg);
    int_unlock(lock);

    hal_sys_timer_delay(MS_TO_TICKS(1));

    lock = int_lock();
    pmu_read(PMU_REG_AC_PRESENT_CFG, &readval_cfg);
    readval_cfg |= AC_PRESENT_INTR_EN;
    readval_cfg = SET_BITFIELD(readval_cfg, AC_PRESENT_DB_CNT, 8);
    pmu_write(PMU_REG_AC_PRESENT_CFG, readval_cfg);
    int_unlock(lock);
}

static void pmu_charger_present_irq_handler(PMU_IRQ_HDLR_PARAM)
{
    enum PMU_CHARGER_PRESENT_STATUS_T status = PMU_CHARGER_PRESENT_IN;
    unsigned short readval;

#ifdef PMU_IRQ_UNIFIED
    readval = irq_status;
#else
    uint32_t lock;

    lock = int_lock();
    pmu_read(PMU_REG_AC_PRESENT_CFG, &readval);
    pmu_write(PMU_REG_AC_PRESENT_CFG, readval);
    int_unlock(lock);
#endif
    PMU_DEBUG_TRACE(3,"%s REG_%02X=0x%04X", __func__, PMU_REG_AC_PRESENT_CFG, readval);

    if ((readval & (AC_PRESENT_P_INTR_MSKED | AC_PRESENT_N_INTR_MSKED)) == 0){
        PMU_DEBUG_TRACE(1,"%s SKIP", __func__);
        return;
    } else if ((readval & (AC_PRESENT_P_INTR_MSKED | AC_PRESENT_N_INTR_MSKED)) ==
            (AC_PRESENT_P_INTR_MSKED | AC_PRESENT_N_INTR_MSKED)) {
        PMU_DEBUG_TRACE(1,"%s DITHERING", __func__);
        hal_sys_timer_delay(2);
    } else {
        PMU_DEBUG_TRACE(1,"%s NORMAL", __func__);
    }

    status = pmu_charger_presernt_get_status();

    if (charger_present_irq_handler) {
        charger_present_irq_handler(status);
    }
}

void pmu_charger_present_set_irq_handler(PMU_CHARGER_PRESENT_IRQ_HANDLER_T handler)
{
    uint32_t lock;
    uint16_t val;

    charger_present_irq_handler = handler;

    lock = int_lock();
    pmu_read(PMU_REG_AC_PRESENT_CFG, &val);
    if (handler) {
        val |= AC_PRESENT_P_INTR_MSK | AC_PRESENT_N_INTR_MSK;
    } else {
        val &= ~(AC_PRESENT_P_INTR_MSK | AC_PRESENT_N_INTR_MSK);
    }
    pmu_write(PMU_REG_AC_PRESENT_CFG, val);

#ifdef PMU_IRQ_UNIFIED
   pmu_set_irq_unified_handler(PMU_IRQ_TYPE_CHARGER_PRESENT, handler ? pmu_charger_present_irq_handler : NULL);
#else
#ifdef RTC_ENABLE
#error "RTC and CHARGER_PRESENT used same IRQ number, please define PMU_IRQ_UNIFIED."
#endif
    if (handler) {
        NVIC_SetVector(RTC_IRQn, (uint32_t)pmu_charger_present_irq_handler);
        NVIC_SetPriority(RTC_IRQn, IRQ_PRIORITY_NORMAL);
        NVIC_ClearPendingIRQ(RTC_IRQn);
        NVIC_EnableIRQ(RTC_IRQn);
    } else {
        NVIC_DisableIRQ(RTC_IRQn);
    }
#endif
    int_unlock(lock);
}

enum PMU_CHARGER_PRESENT_STATUS_T pmu_charger_presernt_get_status(void)
{
    unsigned short readval;
    enum PMU_CHARGER_PRESENT_STATUS_T status = PMU_CHARGER_PRESENT_OUT;

    pmu_read(PMU_REG_AC_PRESENT_CFG, &readval);
    if (readval & AC_PRESENT_STATUS) {
        status = PMU_CHARGER_PRESENT_IN;
    }

    return status;
}
#endif

#ifdef RTC_ENABLE
void pmu_rtc_enable(void)
{
    uint16_t readval;
    uint32_t lock;

#ifdef SIMU
    // Set RTC counter to 1KHz
    pmu_write(PMU_REG_RTC_DIV_1HZ, 32 - 2);
#else
    // Set RTC counter to 1Hz
    pmu_write(PMU_REG_RTC_DIV_1HZ, CONFIG_SYSTICK_HZ * 2 - 2);
#endif

    lock = int_lock();
    pmu_read(PMU_REG_POWER_KEY_CFG, &readval);
    readval |= RTC_POWER_ON_EN | PU_LPO_DR | PU_LPO_REG;
    pmu_write(PMU_REG_POWER_KEY_CFG, readval);

    pmu_read(PMU_REG_COMMAND_NOT_RESET, &readval);
    readval |= REG_NOT_RESET_RTC_ENABLED;
    pmu_write(PMU_REG_COMMAND_NOT_RESET, readval);
    int_unlock(lock);
}

void pmu_rtc_disable(void)
{
    uint16_t readval;
    uint32_t lock;

    pmu_rtc_clear_alarm();

    lock = int_lock();
    pmu_read(PMU_REG_POWER_KEY_CFG, &readval);
    readval &= ~(RTC_POWER_ON_EN | PU_LPO_DR);
    pmu_write(PMU_REG_POWER_KEY_CFG, readval);

    pmu_read(PMU_REG_COMMAND_NOT_RESET, &readval);
    readval &= ~REG_NOT_RESET_RTC_ENABLED;
    pmu_write(PMU_REG_COMMAND_NOT_RESET, readval);
    int_unlock(lock);
}

int BOOT_TEXT_SRAM_LOC pmu_rtc_enabled(void)
{
    uint16_t readval;

    pmu_read(PMU_REG_COMMAND_NOT_RESET, &readval);

    return !!(readval & REG_NOT_RESET_RTC_ENABLED);
}

void pmu_rtc_set(uint32_t seconds)
{
    uint16_t high, low;

    // Need 3 seconds to load a new value
    seconds += 3;

    high = seconds >> 16;
    low = seconds & 0xFFFF;

    pmu_write(PMU_REG_RTC_LOAD_LOW, low);
    pmu_write(PMU_REG_RTC_LOAD_HIGH, high);
}

uint32_t pmu_rtc_get(void)
{
    uint16_t high, low, high2;

    pmu_read(PMU_REG_RTC_VAL_HIGH, &high);
    pmu_read(PMU_REG_RTC_VAL_LOW, &low);
    // Handle counter wrap
    pmu_read(PMU_REG_RTC_VAL_HIGH, &high2);
    if (high != high2) {
        high = high2;
        pmu_read(PMU_REG_RTC_VAL_LOW, &low);
    }

    return (high << 16) | low;
}

void pmu_rtc_set_alarm(uint32_t seconds)
{
    uint16_t readval;
    uint16_t high, low;
    uint32_t lock;

    // Need 1 second to raise the interrupt
    if (seconds > 0) {
        seconds -= 1;
    }

    high = seconds >> 16;
    low = seconds & 0xFFFF;

    pmu_write(PMU_REG_INT_CLR, RTC_INT_CLR_1);

    pmu_write(PMU_REG_RTC_MATCH1_LOW, low);
    pmu_write(PMU_REG_RTC_MATCH1_HIGH, high);

    lock = int_lock();
    pmu_read(PMU_REG_INT_EN, &readval);
    readval |= RTC_INT_EN_1;
    pmu_write(PMU_REG_INT_EN, readval);
    int_unlock(lock);
}

uint32_t BOOT_TEXT_SRAM_LOC pmu_rtc_get_alarm(void)
{
    uint16_t high, low;

    pmu_read(PMU_REG_RTC_MATCH1_LOW, &low);
    pmu_read(PMU_REG_RTC_MATCH1_HIGH, &high);

    // Compensate the alarm offset
    return (uint32_t)((high << 16) | low) + 1;
}

void pmu_rtc_clear_alarm(void)
{
    uint16_t readval;
    uint32_t lock;

    lock = int_lock();
    pmu_read(PMU_REG_INT_EN, &readval);
    readval &= ~RTC_INT_EN_1;
    pmu_write(PMU_REG_INT_EN, readval);
    int_unlock(lock);

    pmu_write(PMU_REG_INT_CLR, RTC_INT_CLR_1);
}

int BOOT_TEXT_SRAM_LOC pmu_rtc_alarm_status_set(void)
{
    uint16_t readval;

    pmu_read(PMU_REG_INT_EN, &readval);

    return !!(readval & RTC_INT_EN_1);
}

int pmu_rtc_alarm_alerted()
{
    uint16_t readval;

    pmu_read(PMU_REG_INT_STATUS, &readval);

    return !!(readval & RTC_INT_1);
}

static void pmu_rtc_irq_handler(PMU_IRQ_HDLR_PARAM)
{
    uint32_t seconds;
    bool alerted;

#ifdef PMU_IRQ_UNIFIED
    alerted = !!(irq_status & RTC_INT_1);
#else
    alerted = pmu_rtc_alarm_alerted();
#endif

    if (alerted) {
        pmu_rtc_clear_alarm();

        if (rtc_irq_handler) {
            seconds = pmu_rtc_get();
            rtc_irq_handler(seconds);
        }
    }
}

void pmu_rtc_set_irq_handler(PMU_RTC_IRQ_HANDLER_T handler)
{
    uint16_t readval;
    uint32_t lock;

    rtc_irq_handler = handler;

    lock = int_lock();
    pmu_read(PMU_REG_INT_MASK, &readval);
    if (handler) {
        readval |= RTC_INT1_MSK;
    } else {
        readval &= ~RTC_INT1_MSK;
    }
    pmu_write(PMU_REG_INT_MASK, readval);

#ifdef PMU_IRQ_UNIFIED
    pmu_set_irq_unified_handler(PMU_IRQ_TYPE_RTC, handler ? pmu_rtc_irq_handler : NULL);
#else
    if (handler) {
        NVIC_SetVector(RTC_IRQn, (uint32_t)pmu_rtc_irq_handler);
        NVIC_SetPriority(RTC_IRQn, IRQ_PRIORITY_NORMAL);
        NVIC_ClearPendingIRQ(RTC_IRQn);
        NVIC_EnableIRQ(RTC_IRQn);
    } else {
        NVIC_DisableIRQ(RTC_IRQn);
    }
#endif
    int_unlock(lock);
}

#ifdef CALIB_POWER_OFF_LPO
static void SRAM_TEXT_LOC pmu_sys_timer_calib_start(void)
{
    uint32_t lock;
    uint32_t slow;
    uint32_t fast;

    lock = int_lock();
    slow = hal_sys_timer_get();
    while (hal_sys_timer_get() == slow);
    __ISB();
    fast = hal_fast_sys_timer_get();
    int_unlock(lock);

    pmu_slow_val = slow + 1;
    pmu_fast_val = fast;
}

static int SRAM_TEXT_LOC pmu_sys_timer_calib_end(void)
{
    uint32_t lock;
    uint32_t slow;
    uint32_t fast;
    uint32_t slow_diff;

    lock = int_lock();
    slow = hal_sys_timer_get();
    while (hal_sys_timer_get() == slow);
    __ISB();
    fast = hal_fast_sys_timer_get();
    int_unlock(lock);

    slow += 1;
    slow_diff = slow - pmu_slow_val;

    // Avoid computation error
    if (slow_diff < PMU_MIN_CALIB_TICKS) {
        return 1;
    }
    // Avoid fast tick overflow
    if (slow_diff > PMU_MAX_CALIB_TICKS) {
        return 2;
    }

    pmu_sys_tick_hz = (float)CONFIG_FAST_SYSTICK_HZ / (fast - pmu_fast_val) * slow_diff;

    if (pmu_sys_tick_hz > PMU_MAX_CALIB_SYSTICK_HZ) {
        pmu_sys_tick_hz = PMU_MAX_CALIB_SYSTICK_HZ;
    }

    return 0;
}

static void SRAM_TEXT_LOC pmu_sys_timer_calib(void)
{
    pmu_sys_timer_calib_start();
    hal_sys_timer_delay(PMU_MIN_CALIB_TICKS);
    pmu_sys_timer_calib_end();
}

static uint32_t pmu_sys_timer_systick_hz(void)
{
    return (uint32_t)(pmu_sys_tick_hz + 0.5);
}

static void pmu_rtc_clk_div_save(void)
{
    uint16_t val;

    // resume lpo settings
    pmu_read(PMU_REG_LPO_CFG, &val);
    val = SET_BITFIELD(val, REG_LPO_ITRIM, 0x1);
    val = SET_BITFIELD(val, REG_LPO_KTRIM, 0x26);
    pmu_write(PMU_REG_LPO_CFG, val);

    // Set lp mode
    pmu_read(PMU_REG_DCDC_RAMP_EN, &val);
    val |= LP_MODE_RTC_REG | LP_MODE_RTC_DR;
    pmu_write(PMU_REG_DCDC_RAMP_EN, val);

    pmu_sys_timer_calib();
    pmu_rtc_div_pwroff = (pmu_sys_timer_systick_hz() << 1) - 2;

    // Resume lp mode
    val &= ~(LP_MODE_RTC_REG | LP_MODE_RTC_DR);
    pmu_write(PMU_REG_DCDC_RAMP_EN, val);
    hal_sys_timer_delay(MS_TO_TICKS(1));
}

static void pmu_rtc_clk_div_reload(void)
{
    pmu_write(PMU_REG_RTC_DIV_1HZ, pmu_rtc_div_pwroff);
}
#endif
#endif

#ifdef PMU_IRQ_UNIFIED
static void pmu_general_irq_handler(void)
{
    uint32_t lock;
    uint16_t val;
    bool pwrkey, charger, gpadc, rtc, gpio, wdt, charger_pre;

    pwrkey = false;
    charger = false;
    gpadc = false;
    rtc = false;
    gpio = false;
    wdt = false;
    charger_pre = false;

    lock = int_lock();
    pmu_read(PMU_REG_CHARGER_STATUS, &val);
    if (val & (POWER_ON_RELEASE | POWER_ON_PRESS)) {
        pwrkey = true;
    }
    if (val & (INTR_MSKED_CHARGE_IN | INTR_MSKED_CHARGE_OUT)) {
        charger = true;
    }
    if (pwrkey || charger) {
        pmu_write(PMU_REG_CHARGER_STATUS, val);
    }
    int_unlock(lock);

    if (pwrkey) {
        if (pmu_irq_hdlrs[PMU_IRQ_TYPE_PWRKEY]) {
            pmu_irq_hdlrs[PMU_IRQ_TYPE_PWRKEY](val);
        }
    }
    if (charger) {
        if (pmu_irq_hdlrs[PMU_IRQ_TYPE_CHARGER]) {
            pmu_irq_hdlrs[PMU_IRQ_TYPE_CHARGER](val);
        }
    }

    lock = int_lock();
    pmu_read(PMU_REG_INT_MSKED_STATUS, &val);
    if (val & (KEY_ERR1_INTR_MSKED | KEY_ERR0_INTR_MSKED | KEY_PRESS_INTR_MSKED | KEY_RELEASE_INTR_MSKED |
            SAMPLE_DONE_INTR_MSKED | CHAN_DATA_INTR_MSKED_MASK)) {
        gpadc = true;
    }
    if (val & (RTC_INT1_MSKED | RTC_INT0_MSKED)) {
        rtc = true;
    }
    if (gpadc || rtc) {
        pmu_write(PMU_REG_INT_CLR, val);
    }
    int_unlock(lock);

    if (gpadc) {
        if (pmu_irq_hdlrs[PMU_IRQ_TYPE_GPADC]) {
            pmu_irq_hdlrs[PMU_IRQ_TYPE_GPADC](val);
        }
    }
    if (rtc) {
        if (pmu_irq_hdlrs[PMU_IRQ_TYPE_RTC]) {
            pmu_irq_hdlrs[PMU_IRQ_TYPE_RTC](val);
        }
    }

    lock = int_lock();
    pmu_read(PMU_REG_LED_IO_IN, &val);
    if (val & (PMU_GPIO_INTR_MSKED1 | PMU_GPIO_INTR_MSKED2)) {
        gpio = true;
    }
    if (val & WDT_INTR_MSKED) {
        wdt = true;
    }
    if (gpio) {
        pmu_write(PMU_REG_LED_IO_IN, val);
    }
    if (wdt) {
        // No need to clear
    }
    int_unlock(lock);

    if (gpio) {
        if (pmu_irq_hdlrs[PMU_IRQ_TYPE_GPIO]) {
            pmu_irq_hdlrs[PMU_IRQ_TYPE_GPIO](val);
        }
    }
    if (wdt) {
        if (pmu_irq_hdlrs[PMU_IRQ_TYPE_WDT]) {
            pmu_irq_hdlrs[PMU_IRQ_TYPE_WDT](val);
        }
    }

    lock = int_lock();
    pmu_read(PMU_REG_AC_PRESENT_CFG, &val);
    if (val & (AC_PRESENT_N_INTR_MSKED | AC_PRESENT_P_INTR_MSKED)) {
        charger_pre = true;
    }
    if (charger_pre) {
        pmu_write(PMU_REG_AC_PRESENT_CFG, val);
    }
    int_unlock(lock);

    if (charger_pre) {
        if (pmu_irq_hdlrs[PMU_IRQ_TYPE_CHARGER_PRESENT]) {
            pmu_irq_hdlrs[PMU_IRQ_TYPE_CHARGER_PRESENT](val);
        }
    }
}

int pmu_set_irq_unified_handler(enum PMU_IRQ_TYPE_T type, PMU_IRQ_UNIFIED_HANDLER_T hdlr)
{
    bool update;
    uint32_t lock;
    int i;

    if (type >= PMU_IRQ_TYPE_QTY) {
        return 1;
    }

    enum PMU_REG_T reg;
    uint16_t val;
    uint16_t mask;

    if (type == PMU_IRQ_TYPE_GPADC) {
        reg = PMU_REG_USB_CFG2;
        mask = GPADC_INTR_MERGED_MSK;
    } else if (type == PMU_IRQ_TYPE_RTC) {
        reg = PMU_REG_USB_CFG2;
        mask = RTC_INTR_TMP_MERGED_MSK;
    } else if (type == PMU_IRQ_TYPE_CHARGER) {
        reg = PMU_REG_USB_CFG2;
        mask = CHARGE_INTR_MERGED_MSK;
    } else if (type == PMU_IRQ_TYPE_GPIO) {
        reg = PMU_REG_USB_CFG3;
        mask = PMU_GPIO_INTR_MSKED1_MERGED_MSK | PMU_GPIO_INTR_MSKED2_MERGED_MSK;
    } else if (type == PMU_IRQ_TYPE_WDT) {
        reg = PMU_REG_USB_CFG3;
        mask = WDT_INTR_MSKED_MERGED_MSK;
    } else if (type == PMU_IRQ_TYPE_CHARGER_PRESENT) {
    } else if (type == PMU_IRQ_TYPE_PWRKEY) {
        reg = PMU_REG_USB_CFG3;
        mask = POWER_ON_INTR_MERGED_MSK;
    } else {
        return 2;
    }

    update = false;

    lock = int_lock();

    for (i = 0; i < PMU_IRQ_TYPE_QTY; i++) {
        if (pmu_irq_hdlrs[i]) {
            break;
        }
    }

    pmu_irq_hdlrs[type] = hdlr;

    if (type != PMU_IRQ_TYPE_CHARGER_PRESENT) {
        pmu_read(reg, &val);
        if (hdlr) {
            val |= mask;
        } else {
            val &= ~mask;
        }
        pmu_write(reg, val);
    }

    if (hdlr) {
        update = (i >= PMU_IRQ_TYPE_QTY);
    } else {
        if (i == type) {
            for (; i < PMU_IRQ_TYPE_QTY; i++) {
                if (pmu_irq_hdlrs[i]) {
                    break;
                }
            }
            update = (i >= PMU_IRQ_TYPE_QTY);
        }
    }

    if (update) {
        if (hdlr) {
            NVIC_SetVector(RTC_IRQn, (uint32_t)pmu_general_irq_handler);
            NVIC_SetPriority(RTC_IRQn, IRQ_PRIORITY_NORMAL);
            NVIC_ClearPendingIRQ(RTC_IRQn);
            NVIC_EnableIRQ(RTC_IRQn);
        } else {
            NVIC_DisableIRQ(RTC_IRQn);
        }
    }

    int_unlock(lock);

    return 0;
}
#endif

enum HAL_PWRKEY_IRQ_T pmu_pwrkey_irq_value_to_state(uint16_t irq_status)
{
    enum HAL_PWRKEY_IRQ_T state = HAL_PWRKEY_IRQ_NONE;

    if (irq_status & POWER_ON_PRESS) {
        state |= HAL_PWRKEY_IRQ_FALLING_EDGE;
    }

    if (irq_status & POWER_ON_RELEASE) {
        state |= HAL_PWRKEY_IRQ_RISING_EDGE;
    }

    return state;
}

void pmu_viorise_req(enum PMU_VIORISE_REQ_USER_T user, bool rise)
{

}

int pmu_debug_config_ana(uint16_t volt)
{
    return 0;
}

int pmu_debug_config_codec(uint16_t volt)
{
    return 0;
}

int pmu_debug_config_vcrystal(bool on)
{
    return 0;
}

int pmu_debug_config_audio_output(bool diff)
{
    return 0;
}

void pmu_debug_reliability_test(int stage)
{
    uint16_t volt;

    if (stage == 0) {
        volt = PMU_DCDC_ANA_1_25V;
    } else {
        volt = PMU_DCDC_ANA_1_3V;
    }
    pmu_dcdc_ana_set_volt(volt, ana_lp_dcdc);
}

void pmu_led_set_hiz(enum HAL_GPIO_PIN_T pin)
{
    uint16_t val;
    uint32_t lock;

    if (pin == HAL_GPIO_PIN_LED1 || pin == HAL_GPIO_PIN_LED2) {
        lock = int_lock();
        if (pin == HAL_GPIO_PIN_LED1) {
            pmu_read(PMU_REG_LED_CFG_IO1, &val);
            val = (val | REG_LED_IO1_PU | REG_LED_IO1_OENB_PRE) & ~(REG_LED_IO1_PUEN | REG_LED_IO1_PDEN | REG_LED_IO1_RX_EN);
            pmu_write(PMU_REG_LED_CFG_IO1, val);
        } else {
            pmu_read(PMU_REG_LED_CFG_IO2, &val);
            val = (val | REG_LED_IO2_PU | REG_LED_IO2_OENB_PRE) & ~(REG_LED_IO2_PUEN | REG_LED_IO2_PDEN | REG_LED_IO2_RX_EN);
            pmu_write(PMU_REG_LED_CFG_IO2, val);
        }
        int_unlock(lock);
    }
}

void pmu_led_set_direction(enum HAL_GPIO_PIN_T pin, enum HAL_GPIO_DIR_T dir)
{
    uint16_t val;
    uint32_t lock;

    if (pin == HAL_GPIO_PIN_LED1 || pin == HAL_GPIO_PIN_LED2) {
        lock = int_lock();
        if (pin == HAL_GPIO_PIN_LED1) {
            pmu_read(PMU_REG_LED_CFG_IO1, &val);
            val |= REG_LED_IO1_PU;
            if (dir == HAL_GPIO_DIR_IN) {
                val |= REG_LED_IO1_OENB_PRE | REG_LED_IO1_RX_EN;
            } else {
                val &= ~(REG_LED_IO1_OENB_PRE | REG_LED_IO1_RX_EN);
            }
            pmu_write(PMU_REG_LED_CFG_IO1, val);
        } else {
            pmu_read(PMU_REG_LED_CFG_IO2, &val);
            val |= REG_LED_IO2_PU;
            if (dir == HAL_GPIO_DIR_IN) {
                val |= REG_LED_IO2_OENB_PRE | REG_LED_IO2_RX_EN;
            } else {
                val &= ~(REG_LED_IO2_OENB_PRE | REG_LED_IO2_RX_EN);
            }
            pmu_write(PMU_REG_LED_CFG_IO2, val);
        }
        int_unlock(lock);
    }
}

enum HAL_GPIO_DIR_T pmu_led_get_direction(enum HAL_GPIO_PIN_T pin)
{
    uint16_t val;

    if (pin == HAL_GPIO_PIN_LED1 || pin == HAL_GPIO_PIN_LED2) {
        if (pin == HAL_GPIO_PIN_LED1) {
            pmu_read(PMU_REG_LED_CFG_IO1, &val);
            return (val & REG_LED_IO1_OENB_PRE) ? HAL_GPIO_DIR_IN : HAL_GPIO_DIR_OUT;
        } else {
            pmu_read(PMU_REG_LED_CFG_IO2, &val);
            return (val & REG_LED_IO2_OENB_PRE) ? HAL_GPIO_DIR_IN : HAL_GPIO_DIR_OUT;
        }
    } else {
        return HAL_GPIO_DIR_IN;
    }
}

void pmu_led_set_voltage_domains(enum HAL_IOMUX_PIN_T pin, enum HAL_IOMUX_PIN_VOLTAGE_DOMAINS_T volt)
{
    enum PMU_LED_VOLT_T {
        PMU_LED_VOLT_VBAT,
        PMU_LED_VOLT_VMEM,
        PMU_LED_VOLT_VIO,
    };
    enum PMU_LED_VOLT_T sel;
    uint16_t val;
    uint32_t lock;

    if (pin == HAL_IOMUX_PIN_LED1 || pin == HAL_IOMUX_PIN_LED2) {
        if (volt == HAL_IOMUX_PIN_VOLTAGE_VIO) {
            sel = PMU_LED_VOLT_VIO;
        } else if (volt == HAL_IOMUX_PIN_VOLTAGE_MEM) {
            sel = PMU_LED_VOLT_VMEM;
        } else {
            sel = PMU_LED_VOLT_VBAT;
        }

        lock = int_lock();
        if (pin == HAL_IOMUX_PIN_LED1) {
            pmu_read(PMU_REG_LED_CFG_IO1, &val);
            val = SET_BITFIELD(val, REG_LED_IO1_SEL, sel);
            pmu_write(PMU_REG_LED_CFG_IO1, val);
        } else {
            pmu_read(PMU_REG_LED_CFG_IO2, &val);
            val = SET_BITFIELD(val, REG_LED_IO2_SEL, sel);
            pmu_write(PMU_REG_LED_CFG_IO2, val);
        }
        int_unlock(lock);
    }
}

void pmu_led_set_pull_select(enum HAL_IOMUX_PIN_T pin, enum HAL_IOMUX_PIN_PULL_SELECT_T pull_sel)
{
    uint16_t val;
    uint32_t lock;

    if (pin == HAL_IOMUX_PIN_LED1 || pin == HAL_IOMUX_PIN_LED2) {
        lock = int_lock();
        if (pin == HAL_IOMUX_PIN_LED1) {
            pmu_read(PMU_REG_LED_CFG_IO1, &val);
            val &= ~(REG_LED_IO1_PDEN | REG_LED_IO1_PUEN);
            if (pull_sel == HAL_IOMUX_PIN_PULLUP_ENABLE) {
                val |= REG_LED_IO1_PUEN;
            } else if (pull_sel == HAL_IOMUX_PIN_PULLDOWN_ENABLE) {
                val |= REG_LED_IO1_PDEN;
            }
            pmu_write(PMU_REG_LED_CFG_IO1, val);
        } else {
            pmu_read(PMU_REG_LED_CFG_IO2, &val);
            val &= ~(REG_LED_IO2_PDEN | REG_LED_IO2_PUEN);
            if (pull_sel == HAL_IOMUX_PIN_PULLUP_ENABLE) {
                val |= REG_LED_IO2_PUEN;
            } else if (pull_sel == HAL_IOMUX_PIN_PULLDOWN_ENABLE) {
                val |= REG_LED_IO2_PDEN;
            }
            pmu_write(PMU_REG_LED_CFG_IO2, val);
        }
        int_unlock(lock);
    }
}

void pmu_led_set_value(enum HAL_GPIO_PIN_T pin, int data)
{
    uint32_t lock;
    uint16_t val;

    if (pin == HAL_GPIO_PIN_LED1 || pin == HAL_GPIO_PIN_LED2) {
        lock = int_lock();
        if (pin == HAL_GPIO_PIN_LED1) {
#ifdef PMU_LED_VIA_PWM
            pmu_write(PMU_REG_PWM2_TOGGLE, 0xFFFF);
            pmu_read(PMU_REG_PWM2_BR_EN, &val);
            val &= ~REG_PWM2_BR_EN;
            val |= PWM_SELECT_EN;
            if (data) {
                val &= ~PWM_SELECT_INV;
            } else {
                val |= PWM_SELECT_INV;
            }
            pmu_write(PMU_REG_PWM2_BR_EN, val);
#else
            pmu_read(PMU_REG_PWM2_EN, &val);
            if (data) {
                val |= REG_LED0_OUT;
            } else {
                val &= ~REG_LED0_OUT;
            }
            pmu_write(PMU_REG_PWM2_EN, val);
#endif
        }else {
#ifdef PMU_LED_VIA_PWM
            pmu_write(PMU_REG_PWMB_TOGGLE, 0xFFFF);
            pmu_read(PMU_REG_PWMB_BR_EN, &val);
            val &= ~REG_PWMB_BR_EN;
            val |= PWMB_SELECT_EN;
            if (data) {
                val &= ~PWMB_SELECT_INV;
            } else {
                val |= PWMB_SELECT_INV;
            }
            pmu_write(PMU_REG_PWMB_BR_EN, val);
#else
            pmu_read(PMU_REG_PWMB_EN, &val);
            if (data) {
                val |= REG_LEDB_OUT;
            } else {
                val &= ~REG_LEDB_OUT;
            }
            pmu_write(PMU_REG_PWMB_EN, val);
#endif
        }
        int_unlock(lock);
    }
}

int pmu_led_get_value(enum HAL_GPIO_PIN_T pin)
{
    uint16_t val;
    int data = 0;

    if (pin == HAL_GPIO_PIN_LED1 || pin == HAL_GPIO_PIN_LED2) {
        pmu_read(PMU_REG_LED_IO_IN, &val);
        if (pin == HAL_GPIO_PIN_LED1) {
            data = LED_IO1_IN_DB;
        } else {
            data = LED_IO2_IN_DB;
        }
        data &= val;
    }

    return !!data;
}

void pmu_led_uart_enable(enum HAL_IOMUX_PIN_T pin)
{
    uint16_t val;

    if (pin == HAL_IOMUX_PIN_LED1) {
        pmu_read(PMU_REG_UART1_CFG, &val);
        val &= ~(REG_UART_LEDA_SEL | REG_PMU_UART_DR1);
        val |=  REG_GPIO_I_SEL;
        pmu_write(PMU_REG_UART1_CFG, val);

        pmu_read(PMU_REG_UART2_CFG, &val);
        val |= REG_PMU_UART_DR2;
        pmu_write(PMU_REG_UART2_CFG, val);
    } else {
        pmu_read(PMU_REG_UART1_CFG, &val);
        val &= ~ REG_GPIO_I_SEL;
        val |= REG_PMU_UART_DR1;
        pmu_write(PMU_REG_UART1_CFG, val);

        pmu_read(PMU_REG_UART2_CFG, &val);
        val &= ~(REG_UART_LEDB_SEL | REG_PMU_UART_DR2);
        pmu_write(PMU_REG_UART2_CFG, val);
    }
}

void pmu_led_uart_disable(enum HAL_IOMUX_PIN_T pin)
{
    uint16_t val;
    if (pin == HAL_IOMUX_PIN_LED1) {
        pmu_read(PMU_REG_UART1_CFG, &val);
        val |= (REG_UART_LEDA_SEL | REG_PMU_UART_DR1);
        val &=  ~REG_GPIO_I_SEL;
        pmu_write(PMU_REG_UART1_CFG, val);

        pmu_read(PMU_REG_UART2_CFG, &val);
        val &= ~REG_PMU_UART_DR2;
        pmu_write(PMU_REG_UART2_CFG, val);
    } else {
        pmu_read(PMU_REG_UART1_CFG, &val);
        val |= REG_GPIO_I_SEL;
        val &= ~REG_PMU_UART_DR1;
        pmu_write(PMU_REG_UART1_CFG, val);

        pmu_read(PMU_REG_UART2_CFG, &val);
        val |= (REG_UART_LEDB_SEL | REG_PMU_UART_DR2);
        pmu_write(PMU_REG_UART2_CFG, val);
    }
}

uint32_t pmu_led_set_drv(enum HAL_IOMUX_PIN_T pin, uint32_t val_drv)
{
    uint16_t val;
    uint32_t lock;

    if (pin >= HAL_IOMUX_PIN_LED_NUM || pin <= HAL_IOMUX_PIN_NUM) {
        return 1;
    }

    if (val_drv > 3) {
        return 2;
    }

    if (pin == HAL_IOMUX_PIN_LED1 || pin == HAL_IOMUX_PIN_LED2) {
        lock = int_lock();
        if (pin == HAL_IOMUX_PIN_LED1) {
            pmu_read(PMU_REG_LED_CFG_IO1, &val);
            val = SET_BITFIELD(val, REG_LED_IO1_IBIT, val_drv);
            pmu_write(PMU_REG_LED_CFG_IO1, val);
        } else {
            pmu_read(PMU_REG_LED_CFG_IO2, &val);
            val = SET_BITFIELD(val, REG_LED_IO2_IBIT, val_drv);
            pmu_write(PMU_REG_LED_CFG_IO2, val);
        }
        int_unlock(lock);
    }

    return 0;
}

void pmu_led_breathing_enable(enum HAL_IOMUX_PIN_T pin, const struct PMU_LED_BR_CFG_T *cfg)
{
    uint32_t st1;
    uint32_t st2;
    uint32_t subcnt_data;
    uint8_t tg;
    uint16_t val;
    uint32_t lock;
    if (pin == HAL_IOMUX_PIN_LED1 || pin == HAL_IOMUX_PIN_LED2) {
        st1 = MS_TO_TICKS(cfg->off_time_ms);
        if (st1 > 0xFFFF) {
            st1 = 0xFFFF;
        }
        st2 = MS_TO_TICKS(cfg->on_time_ms);
        if (st2 > 0xFFFF) {
            st2 = 0xFFFF;
        }
        subcnt_data = MS_TO_TICKS(cfg->fade_time_ms);
        subcnt_data = integer_sqrt_nearest(subcnt_data);
        if (subcnt_data > (SUBCNT_DATA2_MASK >> SUBCNT_DATA2_SHIFT)) {
            subcnt_data = (SUBCNT_DATA2_MASK >> SUBCNT_DATA2_SHIFT);
        }
        // TODO: Keep compatible with digital PWM module (can be removed after 2500)
        if (subcnt_data > 0xFE) {
            subcnt_data = 0xFE;
        }
        tg = 1;

        pmu_led_set_direction((enum HAL_GPIO_PIN_T)pin, HAL_GPIO_DIR_OUT);
        pmu_led_set_pull_select(pin, HAL_IOMUX_PIN_NOPULL);
        pmu_led_set_voltage_domains(pin, HAL_IOMUX_PIN_VOLTAGE_VBAT);

        lock = int_lock();
        if (pin == HAL_IOMUX_PIN_LED1) {
            pmu_write(PMU_REG_PWM2_TOGGLE, st2);
            pmu_write(PMU_REG_PWM2_ST1, st1);
            val = SUBCNT_DATA2(subcnt_data) | TG_SUBCNT_D2_ST(tg);
            pmu_write(PMU_REG_PWM2_EN, val);
            pmu_read(PMU_REG_PWM2_BR_EN, &val);
            val = (val & ~REG_CLK_PWM_DIV_MASK) | REG_CLK_PWM_DIV(0) |
                REG_PWM_CLK_EN | REG_PWM2_BR_EN | PWM_SELECT_EN;
            pmu_write(PMU_REG_PWM2_BR_EN, val);
        } else {
            pmu_write(PMU_REG_PWMB_TOGGLE, st2);
            pmu_write(PMU_REG_PWMB_ST1, st1);
            val = SUBCNT_DATAB(subcnt_data) | TG_SUBCNT_DB_ST(tg);
            pmu_write(PMU_REG_PWMB_EN, val);
            pmu_read(PMU_REG_PWMB_BR_EN, &val);
            val = (val & ~REG_CLK_PWMB_DIV_MASK) | REG_CLK_PWMB_DIV(0) |
                REG_PWMB_CLK_EN | REG_PWMB_BR_EN | PWMB_SELECT_EN;
            pmu_write(PMU_REG_PWMB_BR_EN, val);
        }
        int_unlock(lock);
    }
}

void pmu_led_breathing_disable(enum HAL_IOMUX_PIN_T pin)
{
    uint16_t val;
    uint32_t lock;
    if (pin == HAL_IOMUX_PIN_LED1 || pin == HAL_IOMUX_PIN_LED2) {
        lock = int_lock();
        if (pin == HAL_IOMUX_PIN_LED1) {
            pmu_read(PMU_REG_PWM2_BR_EN, &val);
            val &= ~(REG_PWM_CLK_EN | REG_PWM2_BR_EN | PWM_SELECT_EN);
            pmu_write(PMU_REG_PWM2_BR_EN, val);
        } else {
            pmu_read(PMU_REG_PWMB_BR_EN, &val);
            val &= ~(REG_PWMB_CLK_EN | REG_PWMB_BR_EN | PWMB_SELECT_EN);
            pmu_write(PMU_REG_PWMB_BR_EN, val);
        }
        int_unlock(lock);
        pmu_led_set_direction((enum HAL_GPIO_PIN_T)pin, HAL_GPIO_DIR_IN);
        pmu_led_set_pull_select(pin, HAL_IOMUX_PIN_PULLUP_ENABLE);
    }
}

#ifdef PMU_IRQ_UNIFIED
static void pmu_gpio_irq_handler(uint16_t irq_status)
{
    if (irq_status & PMU_GPIO_INTR_MSKED1) {
        if (gpio_irq_handler[0]) {
            gpio_irq_handler[0](HAL_GPIO_PIN_LED1);
        }
    }
    if (irq_status & PMU_GPIO_INTR_MSKED2) {
        if (gpio_irq_handler[1]) {
            gpio_irq_handler[1](HAL_GPIO_PIN_LED2);
        }
    }
}

uint8_t pmu_gpio_setup_irq(enum HAL_GPIO_PIN_T pin, const struct HAL_GPIO_IRQ_CFG_T *cfg)
{
    uint32_t lock;
    uint16_t val;
    bool old_en;

    if (pin != HAL_GPIO_PIN_LED1 && pin != HAL_GPIO_PIN_LED2) {
        return 1;
    }

    lock = int_lock();

    old_en = (gpio_irq_en[0] || gpio_irq_en[1]);

    if (pin == HAL_GPIO_PIN_LED1) {
        gpio_irq_en[0] = cfg->irq_enable;
        gpio_irq_handler[0] = cfg->irq_handler;

        if (cfg->irq_enable) {
        }

        pmu_read(PMU_REG_UART1_CFG, &val);
        if (cfg->irq_enable) {
            val |= REG_PMU_GPIO_INTR_MSK1 | REG_PMU_GPIO_INTR_EN1;
            if (cfg->irq_debounce) {
                val &= ~PMU_DB_BYPASS1;
            } else {
                val |= PMU_DB_BYPASS1;
            }
        } else {
            val &= ~(REG_PMU_GPIO_INTR_MSK1 | REG_PMU_GPIO_INTR_EN1);
        }
        pmu_write(PMU_REG_UART1_CFG, val);
    } else {
        gpio_irq_en[1] = cfg->irq_enable;
        gpio_irq_handler[1] = cfg->irq_handler;

        pmu_read(PMU_REG_UART2_CFG, &val);
        if (cfg->irq_enable) {
            val |= REG_PMU_GPIO_INTR_MSK2 | REG_PMU_GPIO_INTR_EN2;
            if (cfg->irq_debounce) {
                val &= ~PMU_DB_BYPASS2;
            } else {
                val |= PMU_DB_BYPASS2;
            }
        } else {
            val &= ~(REG_PMU_GPIO_INTR_MSK1 | REG_PMU_GPIO_INTR_EN1);
        }
        pmu_write(PMU_REG_UART2_CFG, val);
    }

    if (cfg->irq_enable) {
        uint16_t type;
        uint16_t pol;

        type = (pin == HAL_GPIO_PIN_LED1) ? REG_EDGE_INTR_SEL1 : REG_EDGE_INTR_SEL2;
        pol = (pin == HAL_GPIO_PIN_LED1) ? REG_POS_INTR_SEL1 : REG_POS_INTR_SEL2;
        pmu_read(PMU_REG_MIC_BIAS_C, &val);
        if (cfg->irq_type == HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE) {
            val |= type;
            if (cfg->irq_polarity == HAL_GPIO_IRQ_POLARITY_LOW_FALLING) {
                val &= ~pol;
            } else {
                val |= pol;
            }
        } else {
            val &= ~type;
        }
        pmu_write(PMU_REG_MIC_BIAS_C, val);

        if (cfg->irq_type != HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE) {
            if (pin == HAL_GPIO_PIN_LED1) {
                pmu_read(PMU_REG_WDT_INT_CFG, &val);
                if (cfg->irq_polarity == HAL_GPIO_IRQ_POLARITY_LOW_FALLING) {
                    val |= REG_LOW_LEVEL_INTR_SEL1;
                } else {
                    val &= ~REG_LOW_LEVEL_INTR_SEL1;
                }
                pmu_write(PMU_REG_WDT_INT_CFG, val);
            } else {
                pmu_read(PMU_REG_UART2_CFG, &val);
                if (cfg->irq_polarity == HAL_GPIO_IRQ_POLARITY_LOW_FALLING) {
                    val |= REG_LOW_LEVEL_INTR_SEL2;
                } else {
                    val &= ~REG_LOW_LEVEL_INTR_SEL2;
                }
                pmu_write(PMU_REG_UART2_CFG, val);
            }
        }
    }

    if (old_en != cfg->irq_enable) {
        pmu_set_irq_unified_handler(PMU_IRQ_TYPE_GPIO, cfg->irq_enable ? pmu_gpio_irq_handler : NULL);
    }

    int_unlock(lock);

    return 0;
}

static void pmu_wdt_irq_handler(uint16_t irq_status)
{
    if (wdt_irq_handler) {
        wdt_irq_handler();
    }
}

void pmu_wdt_set_irq_handler(PMU_WDT_IRQ_HANDLER_T handler)
{
    uint32_t lock;
    uint16_t val;

    lock = int_lock();

    wdt_irq_handler = handler;

    pmu_read(PMU_REG_WDT_INT_CFG, &val);
    if (handler) {
        val |= REG_WDT_INTR_EN | REG_WDT_INTR_MSK;
    } else {
        val &= ~(REG_WDT_INTR_EN | REG_WDT_INTR_MSK);
    }
    pmu_write(PMU_REG_WDT_INT_CFG, val);

    pmu_set_irq_unified_handler(PMU_IRQ_TYPE_WDT, handler ? pmu_wdt_irq_handler : NULL);

    int_unlock(lock);
}
#else
uint8_t pmu_gpio_setup_irq(enum HAL_GPIO_PIN_T pin, const struct HAL_GPIO_IRQ_CFG_T *cfg)
{
    ASSERT(false, "PMU_IRQ_UNIFIED must defined to use PMU GPIO IRQ");
    return 1;
}

void pmu_wdt_set_irq_handler(PMU_WDT_IRQ_HANDLER_T handler)
{
}
#endif

#ifdef __WATCHER_DOG_RESET__
struct PMU_WDT_CTX_T {
    bool enabled;
    uint16_t wdt_irq_timer;
    uint16_t wdt_reset_timer;
    uint16_t wdt_cfg;
};

static struct PMU_WDT_CTX_T BOOT_BSS_LOC wdt_ctx;

void BOOT_TEXT_SRAM_LOC pmu_wdt_save_context(void)
{
    uint16_t wdt_cfg = 0, wdt_timer = 0;
    pmu_read(PMU_REG_WDT_CFG, &wdt_cfg);
    if (wdt_cfg & (REG_WDT_RESET_EN | REG_WDT_EN)){
        wdt_ctx.enabled = true;
        wdt_ctx.wdt_cfg = wdt_cfg;
        pmu_read(PMU_REG_WDT_IRQ_TIMER, &wdt_timer);
        wdt_ctx.wdt_irq_timer = wdt_timer;
        pmu_read(PMU_REG_WDT_RESET_TIMER, &wdt_timer);
        wdt_ctx.wdt_reset_timer = wdt_timer;
    }
}

void BOOT_TEXT_SRAM_LOC pmu_wdt_restore_context(void)
{
    if (wdt_ctx.enabled) {
        pmu_write(PMU_REG_WDT_IRQ_TIMER, wdt_ctx.wdt_irq_timer);
        pmu_write(PMU_REG_WDT_RESET_TIMER, wdt_ctx.wdt_reset_timer);
        pmu_write(PMU_REG_WDT_CFG, wdt_ctx.wdt_cfg);
    }
}
#endif

int pmu_wdt_config(uint32_t irq_ms, uint32_t reset_ms)
{
    if (irq_ms > 0xFFFF) {
        return 1;
    }
    if (reset_ms > 0xFFFF) {
        return 1;
    }
    wdt_irq_timer = irq_ms;
    wdt_reset_timer = reset_ms;

    pmu_write(PMU_REG_WDT_IRQ_TIMER, wdt_irq_timer);
    pmu_write(PMU_REG_WDT_RESET_TIMER, wdt_reset_timer);

    return 0;
}

void pmu_wdt_start(void)
{
    uint16_t val;

    if (wdt_irq_timer == 0 && wdt_reset_timer == 0) {
        return;
    }

    pmu_read(PMU_REG_WDT_CFG, &val);
    val |= (REG_WDT_RESET_EN | REG_WDT_EN);
    pmu_write(PMU_REG_WDT_CFG, val);
}

#ifndef __WATCHER_DOG_RESET__
BOOT_TEXT_SRAM_LOC
#endif
void pmu_wdt_stop(void)
{
    uint16_t val;

    pmu_read(PMU_REG_WDT_CFG, &val);
    val &= ~(REG_WDT_RESET_EN | REG_WDT_EN);
    pmu_write(PMU_REG_WDT_CFG, val);

    pmu_read(PMU_REG_LED_IO_IN, &val);
    val |= REG_WDT_INTR_CLR;
    pmu_write(PMU_REG_LED_IO_IN, val);
}

void pmu_wdt_feed(void)
{
    if (wdt_irq_timer == 0 && wdt_reset_timer == 0) {
        return;
    }

    pmu_write(PMU_REG_WDT_IRQ_TIMER, wdt_irq_timer);
    pmu_write(PMU_REG_WDT_RESET_TIMER, wdt_reset_timer);
}

int pmu_wdt_reboot(void)
{
    uint32_t lock;
    uint16_t val;
    uint16_t val_wdt_cfg;
    const uint16_t wdt_time_ms = 4;

    lock = int_lock();

    // Disable wdt by defalut
    pmu_read(PMU_REG_WDT_CFG, &val_wdt_cfg);
    val_wdt_cfg &= ~(REG_WDT_RESET_EN | REG_WDT_EN);
    pmu_write(PMU_REG_WDT_CFG, val_wdt_cfg);

    // Clear intr by default
    val = REG_WDT_INTR_CLR;
    pmu_write(PMU_REG_LED_IO_IN, val);

    pmu_write(PMU_REG_WDT_IRQ_TIMER, wdt_time_ms / 2);
    pmu_write(PMU_REG_WDT_RESET_TIMER, wdt_time_ms / 2);

    pmu_read(PMU_REG_WDT_INT_CFG, &val);
    val |= REG_WDT_INTR_EN | REG_WDT_INTR_MSK;
    pmu_write(PMU_REG_WDT_INT_CFG, val);

    val_wdt_cfg |= (REG_WDT_RESET_EN | REG_WDT_EN);
    pmu_write(PMU_REG_WDT_CFG, val_wdt_cfg);
    hal_sys_timer_delay(MS_TO_TICKS(wdt_time_ms << 1));

    PMU_INFO_TRACE_IMM(0, "\nError: pmu wdt reboot failed!\n");

    int_unlock(lock);

    return 0;
}

void pmu_ntc_capture_enable(void)
{
    uint16_t val;

    pmu_read(PMU_REG_AVDD_EN, &val);
    val |= REG_PU_AVDD25_ANA;
    pmu_write(PMU_REG_AVDD_EN, val);

    pmu_read(PMU_REG_SAR_CLK_CFG, &val);
    val |= REG_SAR_INPUT_BUF_EN;
    pmu_write(PMU_REG_SAR_CLK_CFG, val);

    pmu_read(PMU_REG_SAR_PU_CFG, &val);
    val |= REG_SAR_PU_VREF_IN;
    pmu_write(PMU_REG_SAR_PU_CFG, val);

    pmu_read(PMU_REG_IPTAT_CORE_I_CFG, &val);
    val |= DIG_IPTAT_CORE_EN;
    pmu_write(PMU_REG_IPTAT_CORE_I_CFG, val);
}

void pmu_ntc_capture_disable(void)
{
    uint16_t val;

    pmu_read(PMU_REG_AVDD_EN, &val);
    val &= ~REG_PU_AVDD25_ANA;
    pmu_write(PMU_REG_AVDD_EN, val);

    pmu_read(PMU_REG_SAR_CLK_CFG, &val);
    val &= ~REG_SAR_INPUT_BUF_EN;
    pmu_write(PMU_REG_SAR_CLK_CFG, val);

    pmu_read(PMU_REG_SAR_PU_CFG, &val);
    val &= ~REG_SAR_PU_VREF_IN;
    pmu_write(PMU_REG_SAR_PU_CFG, val);

    pmu_read(PMU_REG_IPTAT_CORE_I_CFG, &val);
    val &= ~DIG_IPTAT_CORE_EN;
    pmu_write(PMU_REG_IPTAT_CORE_I_CFG, val);
}

void pmu_bt_reconn(bool en)
{
    uint16_t pmu_val = 0;

    pmu_read(0x38, &pmu_val);
    //TRACE(1,"origin: PMU_38=0x%x", pmu_val);
    pmu_val &= 0xfff0;      //clear [3:0]

    if (en) {
        pmu_val |= 0xf;
    }
    pmu_write(0x38, pmu_val);

    //pmu_read(0x38, &pmu_val);
    //TRACE(1,"final: PMU_38=0x%x", pmu_val);
}

static int pmu_vsys_div_ctrl(int enable)
{
    uint32_t lock;
    uint16_t val;

    lock = int_lock();
    pmu_read(PMU_REG_PU_VSYS_DIV, &val);
    if (enable) {
        val |= REG_PU_VSYS_DIV;
    } else {
        val &= ~REG_PU_VSYS_DIV;
    }
    pmu_write(PMU_REG_PU_VSYS_DIV, val);
    int_unlock(lock);

    return 0;
}

int pmu_gpadc_div_ctrl(enum HAL_GPADC_CHAN_T channel, int enable)
{
    int ret = 0;

    if (channel == HAL_GPADC_CHAN_BATTERY) {
        ret = pmu_vsys_div_ctrl(enable);
    } else if(channel == HAL_GPADC_CHAN_6) {
        ret = charger_gpadc_vbat_div_ctrl(enable);
    }

    return ret;
}

void pmu_get_vbat_calib_value(unsigned short *val_lv, unsigned short *val_hv)
{
    if (gpadc_calib_vbat_slim) {
#ifdef PMU_FORCE_LP_MODE
        uint16_t val_hv_low;
        uint16_t val_hv_high;

        if (pmu_force_lp_bg_valid) {
            pmu_get_efuse(PMU_EFUSE_PAGE_RESERVED_D, val_lv);
            *val_lv = GET_BITFIELD(*val_lv, GPADC_CALIB_VAL_SLIM_COMP_LP_LV);
            *val_lv += GPADC_CALIB_VAL_SLIM_A_LP_BASE;

            pmu_get_efuse(PMU_EFUSE_PAGE_BATTER_LV, &val_hv_low);
            pmu_get_efuse(PMU_EFUSE_PAGE_BATTER_HV, &val_hv_high);
            val_hv_low = GET_BITFIELD(val_hv_low, GPADC_CALIB_VAL_SLIM_COMP_LP);
            val_hv_high = GET_BITFIELD(val_hv_high, GPADC_CALIB_VAL_SLIM_COMP_LP);
            *val_hv = (val_hv_high << 5 | val_hv_low) + *val_lv + GPADC_CALIB_VAL_SLIM_B_LP_BASE;
        } else {
            pmu_get_efuse(PMU_EFUSE_PAGE_BATTER_LV, val_lv);
            pmu_get_efuse(PMU_EFUSE_PAGE_BATTER_HV, val_hv);
            *val_lv = GET_BITFIELD(*val_lv, GPADC_CALIB_VAL_SLIM_COMP);
            *val_hv = GET_BITFIELD(*val_hv, GPADC_CALIB_VAL_SLIM_COMP);
            *val_lv += GPADC_CALIB_VAL_SLIM_A_BASE;
            *val_hv += GPADC_CALIB_VAL_SLIM_B_BASE;
        }
#else
        pmu_get_efuse(PMU_EFUSE_PAGE_BATTER_LV, val_lv);
        pmu_get_efuse(PMU_EFUSE_PAGE_BATTER_HV, val_hv);
        *val_lv = GET_BITFIELD(*val_lv, GPADC_CALIB_VAL_SLIM_COMP);
        *val_hv = GET_BITFIELD(*val_hv, GPADC_CALIB_VAL_SLIM_COMP);
        *val_lv += GPADC_CALIB_VAL_SLIM_A_BASE;
        *val_hv += GPADC_CALIB_VAL_SLIM_B_BASE;
#endif
    } else {
        pmu_get_efuse(PMU_EFUSE_PAGE_BATTER_LV, val_lv);
        pmu_get_efuse(PMU_EFUSE_PAGE_BATTER_HV, val_hv);
        *val_lv = *val_lv > 0 ? *val_lv : GPADC_VSYS_CALIB_VAL_A_DEFAULT;
        *val_hv = *val_hv > 0 ? *val_hv : GPADC_VSYS_CALIB_VAL_B_DEFAULT;
    }
}

void pmu_ldo_vpa_en(bool en)
{
    if (en) {
        pmu_module_config(PMU_PA,PMU_MANUAL_MODE,PMU_LDO_ON,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_ON);
#ifdef PMU_HIGH_VPA
        pmu_module_ramp_volt(PMU_PA, PMU_LDO_PA_3_3V, PMU_LDO_PA_3_3V);
#else
        pmu_module_ramp_volt(PMU_PA, PMU_LDO_PA_3_1V, PMU_LDO_PA_3_1V);
#endif
        hal_sys_timer_delay_us(PMU_LDO_PU_STABLE_TIME_US);
    } else {
        pmu_module_config(PMU_PA,PMU_MANUAL_MODE,PMU_LDO_OFF,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_ON);
    }
}

void pmu_ldo_vsensor_enable(enum PMU_VSENSOR_USER_T user)
{
    uint32_t lock;

    lock = int_lock();
    if (vsensor_map == 0) {
        pmu_module_set_volt(PMU_SENSOR, PMU_LDO_SENSOR_3_3V, PMU_LDO_SENSOR_3_3V);
        pmu_module_config(PMU_SENSOR, PMU_MANUAL_MODE, PMU_LDO_ON, PMU_LP_MODE_ON, PMU_DSLEEP_MODE_ON);
    }
    vsensor_map |= user;
    int_unlock(lock);
}

void pmu_ldo_vsensor_disable(enum PMU_VSENSOR_USER_T user)
{
    uint32_t lock;

    if (charger_package_type_get() == CHARGER_PACKAGE_TYPE_SPLIT_ACIN_UARTCOM) {
        // VSENSOR always on.
        return;
    }

    lock = int_lock();
    if (vsensor_map == user) {
        pmu_module_config(PMU_SENSOR, PMU_MANUAL_MODE, PMU_LDO_OFF, PMU_LP_MODE_ON, PMU_DSLEEP_MODE_OFF);
    }
    vsensor_map &= ~user;
    int_unlock(lock);
}

void pmu_shipment_mode_enable(bool en)
{
    shipment_en = en;
}

int pmu_volt2temperature(const uint16_t volt)
{
    const int Kvtherm_T = 380;
    static uint16_t ntc_tmep_ref = 0;
    static uint16_t ntc_tmep_ref2volt = 0;
    int temp_centigrade = 0;

    if (ntc_tmep_ref == 0) {
        ntc_tmep_ref = pmu_ntc_temperature_reference_get();
        ntc_tmep_ref2volt = hal_gpadc_adc2volt(ntc_tmep_ref);
    }

    temp_centigrade = (int)(((volt - ntc_tmep_ref2volt) * 100) / Kvtherm_T + 25);

    return temp_centigrade;
}

static void pmu_ntc_irq_handler(uint16_t raw, uint16_t volt)
{
    pmu_ntc_capture_disable();

    ntc_temperature = pmu_volt2temperature(volt);
    ntc_raw = raw;

    if (pmu_ntc_cb) {
        pmu_ntc_cb(ntc_raw, ntc_temperature);
    }

    ntc_irq_busy = false;
}

int pmu_ntc_capture_start(PMU_NTC_IRQ_HANDLER_T cb)
{
    int nRet = 0;
    uint32_t lock = 0;

    lock = int_lock();
    if (ntc_irq_busy) {
        nRet = -1;
        goto exit;
    }
    pmu_ntc_cb = cb;
    ntc_irq_busy = true;
    pmu_ntc_capture_enable();
    hal_gpadc_open(HAL_GPADC_CHAN_0, HAL_GPADC_ATP_ONESHOT, pmu_ntc_irq_handler);
exit:
    int_unlock(lock);

    return nRet;
}

void pmu_ntc_capture_reset(void)
{
    uint32_t lock;

    lock = int_lock();
    hal_gpadc_close(HAL_GPADC_CHAN_0);
    pmu_ntc_capture_disable();
    pmu_ntc_cb = NULL;
    ntc_irq_busy = false;
    int_unlock(lock);
}

uint16_t pmu_ntc_temperature_reference_get(void)
{
    static uint16_t efuse_temperature_raw = 0;

    if (!efuse_temperature_raw) {
        uint16_t efuse_val;

        pmu_get_efuse(PMU_EFUSE_PAGE_SW_CFG, &efuse_val);
        efuse_val = GET_BITFIELD(efuse_val, PMU_EFUSE_5_TEMPERATURE_REFERENCE);
        if (efuse_val) {
            efuse_temperature_raw = efuse_val + PMU_EFUSE_TEMPSENSOR_BASE;
        } else {
            efuse_temperature_raw = PMU_EFUSE_TEMPSENSOR_DEFAULT;
        }
        TRACE(0, "%s efuse_val=0x%x efuse_temperature_raw=%d", __func__, efuse_val, efuse_temperature_raw);
    }

    return efuse_temperature_raw;
}

bool pmu_force_lp_bg_valid_check(void)
{
#ifdef PMU_FORCE_LP_MODE
    static bool checked = false;
    uint16_t val;

    if (!checked) {
        pmu_get_efuse(PMU_EFUSE_PAGE_RESERVED_D, &val);
        val = GET_BITFIELD(val, GPADC_CALIB_VAL_SLIM_COMP_LP_LV);
        if (val == 0) {
            pmu_force_lp_bg_valid = false;
        } else {
            pmu_force_lp_bg_valid = true;
        }
        checked = true;
        TRACE(0, "%s valid=%d", __func__, pmu_force_lp_bg_valid);
    }

    return pmu_force_lp_bg_valid;
#else
    return false;
#endif
}

void pmu_power_key_hw_reset_enable(uint8_t seconds)
{
    uint16_t val;
    uint32_t lock;
    const uint8_t sample_clk_hz = 4;

    seconds *= sample_clk_hz;
    if (seconds > 60) {
        seconds = 60;
    }

#ifdef SIMU
    pmu_write(PMU_REG_DIV_HW_RESET_CFG, 32 / sample_clk_hz - 2);
#else
    pmu_write(PMU_REG_DIV_HW_RESET_CFG, CONFIG_SYSTICK_HZ / sample_clk_hz * 2 - 2);
#endif

    lock = int_lock();
    pmu_read(PMU_REG_WDT_CFG, &val);
    val = SET_BITFIELD(val, REG_HW_RESET_TIME, seconds) | REG_HW_RESET_EN;
    pmu_write(PMU_REG_WDT_CFG, val);
    int_unlock(lock);
}

void pmu_charger_pattern_reset_enable(bool enable)
{
    uint16_t val;

    pmu_read(PMU_REG_COMMAND_NOT_RESET, &val);
    if (enable) {
        val &= ~REG_COMMAND_RST_DIS;
    } else {
        val |= REG_COMMAND_RST_DIS;
    }
    pmu_write(PMU_REG_COMMAND_NOT_RESET, val);
}
