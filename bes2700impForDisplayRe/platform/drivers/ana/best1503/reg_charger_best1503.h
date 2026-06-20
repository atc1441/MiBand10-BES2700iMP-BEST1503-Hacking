/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifndef __REG_CHARGER_BEST1306_H__
#define __REG_CHARGER_BEST1306_H__

#include "plat_types.h"

enum CHG_REG_T {
    CHG_REG_METAL_ID                    = 0x00,
    CHG_REG_VBAT_DIV_CFG                = 0x01,
    CHG_REG_02                          = 0x02,
    CHG_REG_AC_IN_PP_CFG                = 0x03,
    CHG_REG_TEMP_BG_CFG                 = 0x04,
    CHG_REG_VRECHG_ITERM_CFG            = 0x05,
    CHG_REG_CV_CFG_07                   = 0x07,
    CHG_REG_CHG_FUNC_CFG                = 0x08,
    CHG_REG_CC_PRE_CV_CFG               = 0x09,
    CHG_REG_TRIM_VREF_CFG               = 0x13,
    CHG_REG_EFUSE_INIT_DONE_CFG         = 0x24,
    CHG_PATTERN0_RST_CHARGER_CFG        = 0x30,
    CHG_PATTERN1_SHIPMODE_ON_CFG        = 0x32,
    CHG_PATTERN2_SHIPMODE_OFF_CFG       = 0x34,
    CHG_PATTERN3_SINGLE_UART_ON_CFG     = 0x36,
    CHG_PATTERN4_SINGLE_UART_OFF_CFG    = 0x38,
    CHG_REG_AC_ON_DET_INTR_CFG          = 0x40,
    CHG_REG_EFUSE_INIT_INTR_CFG         = 0x50,
    CHG_REG_ICC_HALF_INTF_PMU           = 0x58,
    CHG_REG_SHIP_MODE_VRTC_CFG          = 0x59,
    CHG_REG_STANDBY_SHIP_MODE           = 0x5A,
    CHG_REG_FAST_TRI_TIMEOUT_INTR_CFG   = 0x60,
    CHG_REG_TEMP_STATUS_INTR_CFG        = 0x63,
    CHG_REG_CHG_DONE_INTR_CFG           = 0x64,
    CHG_REG_OTP_ACIN_VSYS_SC_INTR_CFG   = 0x65,
    CHG_REG_VSYS_LOW_INTR_CFG           = 0x66,
    CHG_REG_VSYS_LOW_DB_CNT_CFG         = 0x67,
    CHG_REG_VSYS_SC_DB_CNT_CFG          = 0x69,
    CHG_REG_ITERM_POS_DB_CNT_CFG        = 0x6D,
    CHG_REG_CHG_STATUS1                 = 0x7B,
    CHG_REG_CHG_STATUS2                 = 0x7C,
    CHG_REG_CHG_STATUS3                 = 0x7D,
    CHG_REG_CHG_INTR1                   = 0x80,
    CHG_REG_CHG_INTR2                   = 0x81,
};

// REG_00
#define REVID_CHARGER_SHIFT                             0
#define REVID_CHARGER_MASK                              (0xF << REVID_CHARGER_SHIFT)
#define REVID_CHARGER(n)                                BITFIELD_VAL(REVID_CHARGER, n)

// REG_01
#define LPO_FTRIM_SHIFT                                 0
#define LPO_FTRIM_MASK                                  (0xF << LPO_FTRIM_SHIFT)
#define LPO_FTRIM(n)                                    BITFIELD_VAL(LPO_FTRIM, n)
#define LPO_KTRIM_SHIFT                                 4
#define LPO_KTRIM_MASK                                  (0x7 << LPO_KTRIM_SHIFT)
#define LPO_KTRIM(n)                                    BITFIELD_VAL(LPO_KTRIM, n)
#define PU_VBAT_DIV                                     (1 << 11)
#define EN_VBAT_DIV_ADC                                 (1 << 12)
#define UVLO_SEL_SHIFT                                  13
#define UVLO_SEL_MASK                                   (0x3 << UVLO_SEL_SHIFT)
#define UVLO_SEL(n)                                     BITFIELD_VAL(UVLO_SEL, n)

// REG_02
#define ACIN_PP_ILIMIT_SEL_SHIFT                        1
#define ACIN_PP_ILIMIT_SEL_MASK                         (0x3 << ACIN_PP_ILIMIT_SEL_SHIFT)
#define ACIN_PP_ILIMIT_SEL(n)                           BITFIELD_VAL(ACIN_PP_ILIMIT_SEL, n)
#define ACIN_PP_P_EDGE_CON_SHIFT                        3
#define ACIN_PP_P_EDGE_CON_MASK                         (0x7 << ACIN_PP_P_EDGE_CON_SHIFT)
#define ACIN_PP_P_EDGE_CON(n)                           BITFIELD_VAL(ACIN_PP_P_EDGE_CON, n)
#define ACIN_PP_N_EDGE_CON_SHIFT                        6
#define ACIN_PP_N_EDGE_CON_MASK                         (0x7 << ACIN_PP_N_EDGE_CON_SHIFT)
#define ACIN_PP_N_EDGE_CON(n)                           BITFIELD_VAL(ACIN_PP_N_EDGE_CON, n)
#define ACIN_PP_LDO_VBIT_SHIFT                          9
#define ACIN_PP_LDO_VBIT_MASK                           (0x1F << ACIN_PP_LDO_VBIT_SHIFT)
#define ACIN_PP_LDO_VBIT(n)                             BITFIELD_VAL(ACIN_PP_LDO_VBIT, n)
#define ACIN_PP_LDO_BYPASS                              (1 << 14)
#define ACIN_PP_LDO_LP_EN                               (1 << 15)

// REG_03
#define REG_ACIN_PP_EN                                  (1 << 0)
#define REG_ACIN_PP_EN_DR                               (1 << 1)
#define REG_ACIN_PP_VDET_EN                             (1 << 2)
#define REG_ACIN_PP_VDET_EN_DR                          (1 << 3)
#define REG_ACIN_PP_IDET_EN                             (1 << 4)
#define REG_ACIN_PP_IDET_EN_DR                          (1 << 5)
#define REG_EN_ACIN_OV_DET                              (1 << 6)
#define REG_EN_ACIN_OV_DET_DR                           (1 << 7)
#define REG_EN_VSYS_SC_DET                              (1 << 8)
#define REG_EN_VSYS_SC_DET_DR                           (1 << 9)
#define ACIN_PP_CONSTANT_VSYS                           (1 << 10)
#define REG_ACIN_PP_ILIMIT_EN                           (1 << 11)
#define REG_ACIN_PP_ILIMIT_EN_DR                        (1 << 12)
#define REG_CHARGER_RECHARGE_EN                         (1 << 13)
#define REG_CHARGER_RECHARGE_EN_DR                      (1 << 14)

// REG_04
#define REG_BG_CORE_EN                                  (1 << 0)
#define REG_BG_CORE_EN_DR                               (1 << 1)
#define REG_BG_REF_GEN_EN                               (1 << 2)
#define REG_BG_REF_GEN_EN_DR                            (1 << 3)
#define REG_BG_EN_OTP                                   (1 << 4)
#define REG_BG_EN_OTP_DR                                (1 << 5)
#define BG_OTP_SEL_SHIFT                                6
#define BG_OTP_SEL_MASK                                 (0x3 << BG_OTP_SEL_SHIFT)
#define BG_OTP_SEL(n)                                   BITFIELD_VAL(BG_OTP_SEL, n)
#define REG_CHARGER_TEMP_COLD_EN                        (1 << 8)
#define REG_CHARGER_TEMP_COLD_EN_DR                     (1 << 9)
#define REG_CHARGER_TEMP_COOL_EN                        (1 << 10)
#define REG_CHARGER_TEMP_COOL_EN_DR                     (1 << 11)
#define REG_CHARGER_TEMP_WARM_EN                        (1 << 12)
#define REG_CHARGER_TEMP_WARM_EN_DR                     (1 << 13)
#define REG_CHARGER_TEMP_HOT_EN                         (1 << 14)
#define REG_CHARGER_TEMP_HOT_EN_DR                      (1 << 15)

// REG_05
#define CHARGER_ITERM_SEL_SHIFT                         1
#define CHARGER_ITERM_SEL_MASK                          (0xF << CHARGER_ITERM_SEL_SHIFT)
#define CHARGER_ITERM_SEL(n)                            BITFIELD_VAL(CHARGER_ITERM_SEL, n)
#define CHARGER_VRECHARGE_SEL_SHIFT                     5
#define CHARGER_VRECHARGE_SEL_MASK                      (0x3 << CHARGER_VRECHARGE_SEL_SHIFT)
#define CHARGER_VRECHARGE_SEL(n)                        BITFIELD_VAL(CHARGER_VRECHARGE_SEL, n)
#define CHARGER_VREF_PRECHARGE_SEL_SHIFT                7
#define CHARGER_VREF_PRECHARGE_SEL_MASK                 (0x3 << CHARGER_VREF_PRECHARGE_SEL_SHIFT)
#define CHARGER_VREF_PRECHARGE_SEL(n)                   BITFIELD_VAL(CHARGER_VREF_PRECHARGE_SEL, n)
#define CHARGER_POWERMOS_P_EDGE_CON_SHIFT               9
#define CHARGER_POWERMOS_P_EDGE_CON_MASK                (0x7 << CHARGER_POWERMOS_P_EDGE_CON_SHIFT)
#define CHARGER_POWERMOS_P_EDGE_CON(n)                  BITFIELD_VAL(CHARGER_POWERMOS_P_EDGE_CON, n)
#define CHARGER_POWERMOS_N_EDGE_CON_SHIFT               12
#define CHARGER_POWERMOS_N_EDGE_CON_MASK                (0x7 << CHARGER_POWERMOS_N_EDGE_CON_SHIFT)
#define CHARGER_POWERMOS_N_EDGE_CON(n)                  BITFIELD_VAL(CHARGER_POWERMOS_N_EDGE_CON, n)

// REG_06
#define CHARGER_CC_COMP_CAP_SHIFT                       0
#define CHARGER_CC_COMP_CAP_MASK                        (0x7 << CHARGER_CC_COMP_CAP_SHIFT)
#define CHARGER_CC_COMP_CAP(n)                          BITFIELD_VAL(CHARGER_CC_COMP_CAP, n)
#define CHARGER_CC_COMP_RES_SHIFT                       3
#define CHARGER_CC_COMP_RES_MASK                        (0x7 << CHARGER_CC_COMP_RES_SHIFT)
#define CHARGER_CC_COMP_RES(n)                          BITFIELD_VAL(CHARGER_CC_COMP_RES, n)
#define CHARGER_CC_GM_SEL_SHIFT                         6
#define CHARGER_CC_GM_SEL_MASK                          (0x7 << CHARGER_CC_GM_SEL_SHIFT)
#define CHARGER_CC_GM_SEL(n)                            BITFIELD_VAL(CHARGER_CC_GM_SEL, n)

// REG_07
#define CHARGER_CV_COMP_CAP_SHIFT                       0
#define CHARGER_CV_COMP_CAP_MASK                        (0x7 << CHARGER_CV_COMP_CAP_SHIFT)
#define CHARGER_CV_COMP_CAP(n)                          BITFIELD_VAL(CHARGER_CV_COMP_CAP, n)
#define CHARGER_CV_COMP_RES_SHIFT                       3
#define CHARGER_CV_COMP_RES_MASK                        (0x7 << CHARGER_CV_COMP_RES_SHIFT)
#define CHARGER_CV_COMP_RES(n)                          BITFIELD_VAL(CHARGER_CV_COMP_RES, n)
#define CHARGER_CV_GM_SEL_SHIFT                         6
#define CHARGER_CV_GM_SEL_MASK                          (0x7 << CHARGER_CV_GM_SEL_SHIFT)
#define CHARGER_CV_GM_SEL(n)                            BITFIELD_VAL(CHARGER_CV_GM_SEL, n)

// REG_08
#define REG_CHARGER_CHARGE_EN                           (1 << 0)
#define REG_CHARGER_CHARGE_EN_DR                        (1 << 1)
#define REG_CHARGER_TRICKLE_CHARGE_EN                   (1 << 2)
#define REG_CHARGER_TRICKLE_CHARGE_EN_DR                (1 << 3)
#define REG_CHARGER_CC_CV_CHARGE_EN                     (1 << 4)
#define REG_CHARGER_CC_CV_CHARGE_EN_DR                  (1 << 5)
#define REG_CHARGER_NTC_EN                              (1 << 6)
#define REG_CHARGER_NTC_EN_DR                           (1 << 7)
#define REG_CHARGER_PRECHARGE_EN                        (1 << 8)
#define REG_CHARGER_PRECHARGE_EN_DR                     (1 << 9)
#define REG_CHARGER_ICHARGE_RAMP_CNT_SHIFT              10
#define REG_CHARGER_ICHARGE_RAMP_CNT_MASK               (0x3F << REG_CHARGER_ICHARGE_RAMP_CNT_SHIFT)
#define REG_CHARGER_ICHARGE_RAMP_CNT(n)                 BITFIELD_VAL(REG_CHARGER_ICHARGE_RAMP_CNT, n)

// REG_09
#define REG_CHARGER_VOREG_BIT_SHIFT                     0
#define REG_CHARGER_VOREG_BIT_MASK                      (0x1F << REG_CHARGER_VOREG_BIT_SHIFT)
#define REG_CHARGER_VOREG_BIT(n)                        BITFIELD_VAL(REG_CHARGER_VOREG_BIT, n)
#define REG_CHARGER_VOREG_BIT_DR                        (1 << 5)
#define REG_CHARGER_IPRE_SEL_SHIFT                      6
#define REG_CHARGER_IPRE_SEL_MASK                       (0xF << REG_CHARGER_IPRE_SEL_SHIFT)
#define REG_CHARGER_IPRE_SEL(n)                         BITFIELD_VAL(REG_CHARGER_IPRE_SEL, n)
#define REG_CHARGER_ICC_SEL_SHIFT                       10
#define REG_CHARGER_ICC_SEL_MASK                        (0xF << REG_CHARGER_ICC_SEL_SHIFT)
#define REG_CHARGER_ICC_SEL(n)                          BITFIELD_VAL(REG_CHARGER_ICC_SEL, n)
#define REG_CHARGER_ICC_SEL_DR                          (1 << 14)

// REG_0A
#define REG_CLK_32K_SEL                                 (1 << 0)
#define REG_CLK_32K_SEL_DR                              (1 << 1)
#define CHARGER_TEST_SEL_SHIFT                          2
#define CHARGER_TEST_SEL_MASK                           (0x7 << CHARGER_TEST_SEL_SHIFT)
#define CHARGER_TEST_SEL(n)                             BITFIELD_VAL(CHARGER_TEST_SEL, n)
#define REG_CHG_TEST_MODE                               (1 << 5)

// REG_10
#define REG_TRIM_BGTEMP_SHIFT                           0
#define REG_TRIM_BGTEMP_MASK                            (0xF << REG_TRIM_BGTEMP_SHIFT)
#define REG_TRIM_BGTEMP(n)                              BITFIELD_VAL(REG_TRIM_BGTEMP, n)
#define REG_TRIM_BGTEMP_DR                              (1 << 4)
#define REG_TRIM_VBG_SHIFT                              5
#define REG_TRIM_VBG_MASK                               (0x3F << REG_TRIM_VBG_SHIFT)
#define REG_TRIM_VBG(n)                                 BITFIELD_VAL(REG_TRIM_VBG, n)
#define REG_TRIM_VBG_DR                                 (1 << 11)

// REG_11
#define REG_TRIM_NTC_RES_SHIFT                          0
#define REG_TRIM_NTC_RES_MASK                           (0xF << REG_TRIM_NTC_RES_SHIFT)
#define REG_TRIM_NTC_RES(n)                             BITFIELD_VAL(REG_TRIM_NTC_RES, n)
#define REG_TRIM_NTC_RES_DR                             (1 << 4)
#define REG_TRIM_IREF_RES_SHIFT                         5
#define REG_TRIM_IREF_RES_MASK                          (0xF << REG_TRIM_IREF_RES_SHIFT)
#define REG_TRIM_IREF_RES(n)                            BITFIELD_VAL(REG_TRIM_IREF_RES, n)
#define REG_TRIM_IREF_RES_DR                            (1 << 9)

// REG_12
#define REG_TRIM_ICHARGE_RES_SHIFT                      0
#define REG_TRIM_ICHARGE_RES_MASK                       (0x7 << REG_TRIM_ICHARGE_RES_SHIFT)
#define REG_TRIM_ICHARGE_RES(n)                         BITFIELD_VAL(REG_TRIM_ICHARGE_RES, n)
#define REG_TRIM_ICHARGE_RES_DR                         (1 << 3)
#define REG_TRIM_ITERM_RES_SHIFT                        4
#define REG_TRIM_ITERM_RES_MASK                         (0x7 << REG_TRIM_ITERM_RES_SHIFT)
#define REG_TRIM_ITERM_RES(n)                           BITFIELD_VAL(REG_TRIM_ITERM_RES, n)
#define REG_TRIM_ITERM_RES_DR                           (1 << 7)
#define REG_TRIM_ACIN_PP_RES_SHIFT                      8
#define REG_TRIM_ACIN_PP_RES_MASK                       (0x7 << REG_TRIM_ACIN_PP_RES_SHIFT)
#define REG_TRIM_ACIN_PP_RES(n)                         BITFIELD_VAL(REG_TRIM_ACIN_PP_RES, n)
#define REG_TRIM_ACIN_PP_RES_DR                         (1 << 11)

// REG_13
#define REG_TRIM_VREF_PRE_SHIFT                         0
#define REG_TRIM_VREF_PRE_MASK                          (0x7 << REG_TRIM_VREF_PRE_SHIFT)
#define REG_TRIM_VREF_PRE(n)                            BITFIELD_VAL(REG_TRIM_VREF_PRE, n)
#define REG_TRIM_VREF_PRE_DR                            (1 << 3)
#define REG_TRIM_VREF_CC_SHIFT                          4
#define REG_TRIM_VREF_CC_MASK                           (0x7 << REG_TRIM_VREF_CC_SHIFT)
#define REG_TRIM_VREF_CC(n)                             BITFIELD_VAL(REG_TRIM_VREF_CC, n)
#define REG_TRIM_VREF_CC_DR                             (1 << 7)
#define REG_TRIM_VOREG_SHIFT                            8
#define REG_TRIM_VOREG_MASK                             (0x1F << REG_TRIM_VOREG_SHIFT)
#define REG_TRIM_VOREG(n)                               BITFIELD_VAL(REG_TRIM_VOREG, n)
#define REG_TRIM_VOREG_DR                               (1 << 13)

// REG_20
#define EFUSE_TRIM_BGTEMP_SHIFT                         0
#define EFUSE_TRIM_BGTEMP_MASK                          (0xF << EFUSE_TRIM_BGTEMP_SHIFT)
#define EFUSE_TRIM_BGTEMP(n)                            BITFIELD_VAL(EFUSE_TRIM_BGTEMP, n)
#define EFUSE_TRIM_BGTEMP_DR                            (1 << 4)
#define EFUSE_TRIM_VBG_SHIFT                            5
#define EFUSE_TRIM_VBG_MASK                             (0x3F << EFUSE_TRIM_VBG_SHIFT)
#define EFUSE_TRIM_VBG(n)                               BITFIELD_VAL(EFUSE_TRIM_VBG, n)
#define EFUSE_TRIM_VBG_DR                               (1 << 11)

// REG_21
#define EFUSE_TRIM_NTC_RES_SHIFT                        0
#define EFUSE_TRIM_NTC_RES_MASK                         (0xF << EFUSE_TRIM_NTC_RES_SHIFT)
#define EFUSE_TRIM_NTC_RES(n)                           BITFIELD_VAL(EFUSE_TRIM_NTC_RES, n)
#define EFUSE_TRIM_NTC_RES_DR                           (1 << 4)
#define EFUSE_TRIM_IREF_RES_SHIFT                       5
#define EFUSE_TRIM_IREF_RES_MASK                        (0xF << EFUSE_TRIM_IREF_RES_SHIFT)
#define EFUSE_TRIM_IREF_RES(n)                          BITFIELD_VAL(EFUSE_TRIM_IREF_RES, n)
#define EFUSE_TRIM_IREF_RES_DR                          (1 << 9)

// REG_22
#define EFUSE_TRIM_ICHARGE_RES_SHIFT                    0
#define EFUSE_TRIM_ICHARGE_RES_MASK                     (0x7 << EFUSE_TRIM_ICHARGE_RES_SHIFT)
#define EFUSE_TRIM_ICHARGE_RES(n)                       BITFIELD_VAL(EFUSE_TRIM_ICHARGE_RES, n)
#define EFUSE_TRIM_ICHARGE_RES_DR                       (1 << 3)
#define EFUSE_TRIM_ITERM_RES_SHIFT                      4
#define EFUSE_TRIM_ITERM_RES_MASK                       (0x7 << EFUSE_TRIM_ITERM_RES_SHIFT)
#define EFUSE_TRIM_ITERM_RES(n)                         BITFIELD_VAL(EFUSE_TRIM_ITERM_RES, n)
#define EFUSE_TRIM_ITERM_RES_DR                         (1 << 7)
#define EFUSE_TRIM_ACIN_PP_RES_SHIFT                    8
#define EFUSE_TRIM_ACIN_PP_RES_MASK                     (0x7 << EFUSE_TRIM_ACIN_PP_RES_SHIFT)
#define EFUSE_TRIM_ACIN_PP_RES(n)                       BITFIELD_VAL(EFUSE_TRIM_ACIN_PP_RES, n)
#define EFUSE_TRIM_ACIN_PP_RES_DR                       (1 << 11)

// REG_23
#define EFUSE_TRIM_VREF_PRE_SHIFT                       0
#define EFUSE_TRIM_VREF_PRE_MASK                        (0x7 << EFUSE_TRIM_VREF_PRE_SHIFT)
#define EFUSE_TRIM_VREF_PRE(n)                          BITFIELD_VAL(EFUSE_TRIM_VREF_PRE, n)
#define EFUSE_TRIM_VREF_PRE_DR                          (1 << 3)
#define EFUSE_TRIM_VREF_CC_SHIFT                        4
#define EFUSE_TRIM_VREF_CC_MASK                         (0x7 << EFUSE_TRIM_VREF_CC_SHIFT)
#define EFUSE_TRIM_VREF_CC(n)                           BITFIELD_VAL(EFUSE_TRIM_VREF_CC, n)
#define EFUSE_TRIM_VREF_CC_DR                           (1 << 7)
#define EFUSE_TRIM_VOREG_SHIFT                          8
#define EFUSE_TRIM_VOREG_MASK                           (0x1F << EFUSE_TRIM_VOREG_SHIFT)
#define EFUSE_TRIM_VOREG(n)                             BITFIELD_VAL(EFUSE_TRIM_VOREG, n)
#define EFUSE_TRIM_VOREG_DR                             (1 << 13)

// REG_24
#define REG_EFUSE_INIT_DONE                             (1 << 0)
#define REG_EFUSE_INIT_DONE_DR                          (1 << 1)
#define REG_EFUSE_INIT_DONE_TRIGGER_EN                  (1 << 2)
#define REG_EFUSE_INIT_DONE_TRIGGER                     (1 << 3)

// REG_30
#define REG_COMMAND_0_ENABLE                            (1 << 0)
#define REG_COMMAND_0_BIT_WIDTH_TOLERANCE_SHIFT         1
#define REG_COMMAND_0_BIT_WIDTH_TOLERANCE_MASK          (0xFF << REG_COMMAND_0_BIT_WIDTH_TOLERANCE_SHIFT)
#define REG_COMMAND_0_BIT_WIDTH_TOLERANCE(n)            BITFIELD_VAL(REG_COMMAND_0_BIT_WIDTH_TOLERANCE, n)

// REG_31
#define REG_COMMAND_0_TIME_SHIFT                        0
#define REG_COMMAND_0_TIME_MASK                         0xFFF << REG_COMMAND_0_TIME_SHIFT)
#define REG_COMMAND_0_TIME(n)                           BITFIELD_VAL(REG_COMMAND_0_TIME, n)

// REG_32
#define REG_COMMAND_1_ENABLE                            (1 << 0)
#define REG_COMMAND_1_BIT_WIDTH_TOLERANCE_SHIFT         1
#define REG_COMMAND_1_BIT_WIDTH_TOLERANCE_MASK          (0xFF << REG_COMMAND_1_BIT_WIDTH_TOLERANCE_SHIFT)
#define REG_COMMAND_1_BIT_WIDTH_TOLERANCE(n)            BITFIELD_VAL(REG_COMMAND_1_BIT_WIDTH_TOLERANCE, n)
#define REG_COMMAND_1_TIME_SHIFT                        9
#define REG_COMMAND_1_TIME_MASK                         (0x3F << REG_COMMAND_1_TIME_SHIFT)
#define REG_COMMAND_1_TIME(n)                           BITFIELD_VAL(REG_COMMAND_1_TIME, n)

// REG_34
#define REG_COMMAND_2_ENABLE                            (1 << 0)
#define REG_COMMAND_2_BIT_WIDTH_TOLERANCE_SHIFT         1
#define REG_COMMAND_2_BIT_WIDTH_TOLERANCE_MASK          (0xFF << REG_COMMAND_2_BIT_WIDTH_TOLERANCE_SHIFT)
#define REG_COMMAND_2_BIT_WIDTH_TOLERANCE(n)            BITFIELD_VAL(REG_COMMAND_2_BIT_WIDTH_TOLERANCE, n)
#define REG_COMMAND_2_TIME_SHIFT                        9
#define REG_COMMAND_2_TIME_MASK                         (0x3F << REG_COMMAND_2_TIME_SHIFT)
#define REG_COMMAND_2_TIME(n)                           BITFIELD_VAL(REG_COMMAND_2_TIME, n)

// REG_36
#define REG_COMMAND_3_ENABLE                            (1 << 0)
#define REG_COMMAND_3_BIT_WIDTH_TOLERANCE_SHIFT         1
#define REG_COMMAND_3_BIT_WIDTH_TOLERANCE_MASK          (0xFF << REG_COMMAND_3_BIT_WIDTH_TOLERANCE_SHIFT)
#define REG_COMMAND_3_BIT_WIDTH_TOLERANCE(n)            BITFIELD_VAL(REG_COMMAND_3_BIT_WIDTH_TOLERANCE, n)
#define REG_COMMAND_3_TIME_SHIFT                        9
#define REG_COMMAND_3_TIME_MASK                         (0x3F << REG_COMMAND_3_TIME_SHIFT)
#define REG_COMMAND_3_TIME(n)                           BITFIELD_VAL(REG_COMMAND_3_TIME, n)

// REG_38
#define REG_COMMAND_4_ENABLE                            (1 << 0)
#define REG_COMMAND_4_BIT_WIDTH_TOLERANCE_SHIFT         1
#define REG_COMMAND_4_BIT_WIDTH_TOLERANCE_MASK          (0xFF << REG_COMMAND_4_BIT_WIDTH_TOLERANCE_SHIFT)
#define REG_COMMAND_4_BIT_WIDTH_TOLERANCE(n)            BITFIELD_VAL(REG_COMMAND_4_BIT_WIDTH_TOLERANCE, n)
#define REG_COMMAND_4_TIME_SHIFT                        9
#define REG_COMMAND_4_TIME_MASK                         (0x3F << REG_COMMAND_4_TIME_SHIFT)
#define REG_COMMAND_4_TIME(n)                           BITFIELD_VAL(REG_COMMAND_4_TIME, n)

// REG_3A
#define REG_RESETN_CHIP_DB_CNT_SHIFT                    0
#define REG_RESETN_CHIP_DB_CNT_MASK                     (0xF << REG_RESETN_CHIP_DB_CNT_SHIFT)
#define REG_RESETN_CHIP_DB_CNT(n)                       BITFIELD_VAL(REG_RESETN_CHIP_DB_CNT, n)
#define REG_RESETN_CHIP_DB_EN                           (1 << 4)

// REG_3B
#define REG_CLK_LPO_STABLE_CNT_SHIFT                    0
#define REG_CLK_LPO_STABLE_CNT_MASK                     (0xF << REG_CLK_LPO_STABLE_CNT_SHIFT)
#define REG_CLK_LPO_STABLE_CNT(n)                       BITFIELD_VAL(REG_CLK_LPO_STABLE_CNT, n)
#define REG_CLK_32K_PMU_STABLE_CNT_SHIFT                4
#define REG_CLK_32K_PMU_STABLE_CNT_MASK                 (0xF << REG_CLK_32K_PMU_STABLE_CNT_SHIFT)
#define REG_CLK_32K_PMU_STABLE_CNT(n)                   BITFIELD_VAL(REG_CLK_32K_PMU_STABLE_CNT, n)

// REG_40
#define REG_AC_ON_DET_IN_INTR_RAW                       (1 << 0)
#define REG_AC_ON_DET_IN_INTR_EN                        (1 << 1)
#define REG_AC_ON_DET_IN_INTR_MASK                      (1 << 2)
#define REG_AC_ON_DET_IN_INTR_CLR                       (1 << 3)
#define REG_AC_ON_DET_OUT_INTR_RAW                      (1 << 8)
#define REG_AC_ON_DET_OUT_INTR_EN                       (1 << 9)
#define REG_AC_ON_DET_OUT_INTR_MASK                     (1 << 10)
#define REG_AC_ON_DET_OUT_INTR_CLR                      (1 << 11)

#define REG_CHARGE_AC_ON_DET_IN_INTR_RAW                REG_AC_ON_DET_IN_INTR_RAW
#define REG_CHARGE_AC_ON_DET_IN_INTR_EN                 REG_AC_ON_DET_IN_INTR_EN
#define REG_CHARGE_AC_ON_DET_IN_INTR_MASK               REG_AC_ON_DET_IN_INTR_MASK
#define REG_CHARGE_AC_ON_DET_IN_INTR_CLR                REG_AC_ON_DET_IN_INTR_CLR
#define REG_CHARGE_AC_ON_DET_OUT_INTR_RAW               REG_AC_ON_DET_OUT_INTR_RAW
#define REG_CHARGE_AC_ON_DET_OUT_INTR_EN                REG_AC_ON_DET_OUT_INTR_EN
#define REG_CHARGE_AC_ON_DET_OUT_INTR_MASK              REG_AC_ON_DET_OUT_INTR_MASK
#define REG_CHARGE_AC_ON_DET_OUT_INTR_CLR               REG_AC_ON_DET_OUT_INTR_CLR

// REG_41
#define REG_AC_ON_EN                                    (1 << 0)
#define REG_AC_ON_POWEROFF_EN                           (1 << 1)

// REG_42
#define REG_AC_ON_POS_DB_CNT_SHIFT                      0
#define REG_AC_ON_POS_DB_CNT_MASK                       (0x7F << REG_AC_ON_POS_DB_CNT_SHIFT)
#define REG_AC_ON_POS_DB_CNT(n)                         BITFIELD_VAL(REG_AC_ON_POS_DB_CNT, n)
#define REG_AC_ON_NEG_DB_CNT_SHIFT                      7
#define REG_AC_ON_NEG_DB_CNT_MASK                       (0x7F << REG_AC_ON_NEG_DB_CNT_SHIFT)
#define REG_AC_ON_NEG_DB_CNT(n)                         BITFIELD_VAL(REG_AC_ON_NEG_DB_CNT, n)

// REG_43
#define REG_SW_POWER_OFF                                (1 << 0)
#define REG_SW_POWER_OFF_CNT_SHIFT                      1
#define REG_SW_POWER_OFF_CNT_MASK                       (0xF << REG_SW_POWER_OFF_CNT_SHIFT)
#define REG_SW_POWER_OFF_CNT(n)                         BITFIELD_VAL(REG_SW_POWER_OFF_CNT, n)
#define REG_LPO_OFF_CNT_SHIFT                           5
#define REG_LPO_OFF_CNT_MASK                            (0xFF << REG_LPO_OFF_CNT_SHIFT)
#define REG_LPO_OFF_CNT(n)                              BITFIELD_VAL(REG_LPO_OFF_CNT, n)
#define REG_PU_LPO                                      (1 << 13)
#define REG_PU_LPO_DR                                   (1 << 14)

// REG_46
#define REG_EFUSE_TIMEOUT_CNT_SHIFT                     0
#define REG_EFUSE_TIMEOUT_CNT_MASK                      (0x1FFF << REG_EFUSE_TIMEOUT_CNT_SHIFT)
#define REG_EFUSE_TIMEOUT_CNT(n)                        BITFIELD_VAL(REG_EFUSE_TIMEOUT_CNT, n)

// REG_47
#define REG_PU_CORE_CNT_SHIFT                           0
#define REG_PU_CORE_CNT_MASK                            (0x1FFF << REG_PU_CORE_CNT_SHIFT)
#define REG_PU_CORE_CNT(n)                              BITFIELD_VAL(REG_PU_CORE_CNT, n)

// REG_48
#define REG_PU_EN_DET_CNT_SHIFT                         0
#define REG_PU_EN_DET_CNT_MASK                          (0x1FFF << REG_PU_EN_DET_CNT_SHIFT)
#define REG_PU_EN_DET_CNT(n)                            BITFIELD_VAL(REG_PU_EN_DET_CNT, n)

// REG_49
#define REG_PU_VDET_CNT_SHIFT                           0
#define REG_PU_VDET_CNT_MASK                            (0x1FFF << REG_PU_VDET_CNT_SHIFT)
#define REG_PU_VDET_CNT(n)                              BITFIELD_VAL(REG_PU_VDET_CNT, n)

// REG_4D
#define REG_PU_CHARGE_CNT_SHIFT                         0
#define REG_PU_CHARGE_CNT_MASK                          (0x1FFF << REG_PU_CHARGE_CNT_SHIFT)
#define REG_PU_CHARGE_CNT(n)                               BITFIELD_VAL(REG_PU_CHARGE_CNT, n)

// REG_4E
#define REG_PP_LDO_SOFT_START_CNT_SHIFT                 0
#define REG_PP_LDO_SOFT_START_CNT_MASK                  (0xFF << REG_PP_LDO_SOFT_START_CNT_SHIFT)
#define REG_PP_LDO_SOFT_START_CNT(n)                    BITFIELD_VAL(REG_PP_LDO_SOFT_START_CNT, n)

// REG_4F
#define REG_OTP_EN                                      (1 << 0)
#define REG_ACIN_CL_EN                                  (1 << 1)
#define REG_ST_INIT_OK                                  (1 << 2)
#define REG_VH_CHRG_SUB3V_BUF_EN                        (1 << 3)
#define REG_VH_CHRG_SUB3V_BUF_EN_DR                     (1 << 4)
#define REG_MAX_SUB3V_BUF_EN                            (1 << 5)
#define REG_MAX_SUB3V_BUF_EN_DR                         (1 << 6)
#define REG_SUB3V_CNT_SHIFT                             7
#define REG_SUB3V_CNT_MASK                              (0x3F << REG_SUB3V_CNT_SHIFT)
#define REG_SUB3V_CNT(n)                                BITFIELD_VAL(REG_SUB3V_CNT, n)
#define REG_VSYS_SC_EN                                  (1 << 13)
#define REG_ACIN_OV_EN                                  (1 << 14)
#define REG_VSYS_LOW_EN                                 (1 << 15)

// REG_50
#define REG_EFUSE_INIT_TIMEOUT_INTR_RAW                 (1 << 0)
#define REG_EFUSE_INIT_TIMEOUT_INTR_EN                  (1 << 1)
#define REG_EFUSE_INIT_TIMEOUT_INTR_MASK                (1 << 2)
#define REG_EFUSE_INIT_TIMEOUT_INTR_CLR                 (1 << 3)
#define REG_EFUSE_INIT_DONE_INTR_RAW                    (1 << 4)
#define REG_EFUSE_INIT_DONE_INTR_EN                     (1 << 5)
#define REG_EFUSE_INIT_DONE_INTR_MASK                   (1 << 6)
#define REG_EFUSE_INIT_DONE_INTR_CLR                    (1 << 7)

// REG_58
#define REG_EN_BAT_ISINK250U                            (1 << 0)
#define REG_EN_BAT_ISINK250U_DR                         (1 << 1)
#define REG_PWDN_INTF_PMU                               (1 << 2)
#define REG_PWDN_INTF_PMU_DR                            (1 << 3)
#define REG_NTC_HALF_ICC                                (1 << 4)
#define REG_NTC_HALF_ICC_DR                             (1 << 5)
#define REG_ACIN_PP_LDO_PULL_DOWN                       (1 << 6)
#define REG_ACIN_PP_LDO_PULL_DOWN_DR                    (1 << 7)
#define REG_PWDN_INTF_PMU_CNT_SHIFT                     8
#define REG_PWDN_INTF_PMU_CNT_MASK                      (0x3F << REG_PWDN_INTF_PMU_CNT_SHIFT)
#define REG_PWDN_INTF_PMU_CNT(n)                        BITFIELD_VAL(REG_PWDN_INTF_PMU_CNT, n)

// REG_59
#define VRTC0P8_SEL_SHIFT                               0
#define VRTC0P8_SEL_MASK                                (0xF << VRTC0P8_SEL_SHIFT)
#define VRTC0P8_SEL(n)                                  BITFIELD_VAL(VRTC0P8_SEL, n)
#define REG_SHIP_MODE_EN                                (1 << 5)
#define REG_SHIP_MODE_EN_DR                             (1 << 6)

// REG_5A
#define REG_STANDBY_SHIP_MODE_EN                        (1 << 0)
#define REG_STANDBY_SHIP_MODE                           (1 << 1)
#define REG_UART_STANDBY_MODE_EN                        (1 << 2)
#define REG_UART_STANDBY_MODE                           (1 << 3)
#define REG_DONE_SW_RECHARGE_EN                         (1 << 4)
#define REG_DONE_SW_RECHARGE                            (1 << 5)
#define REG_FAULT_SW_RECHARGE_EN                        (1 << 6)
#define REG_FAULT_SW_RECHARGE                           (1 << 7)
#define REG_FAULT2DONE_EN                               (1 << 8)
#define REG_FAULT2DONE                                  (1 << 9)

// REG_5D
#define REG_CHARGE_OFF_CNT_SHIFT                        0
#define REG_CHARGE_OFF_CNT_MASK                         (0xFF << REG_CHARGE_OFF_CNT_SHIFT)
#define REG_CHARGE_OFF_CNT(n)                           BITFIELD_VAL(REG_CHARGE_OFF_CNT, n)

// REG_60
#define REG_CHARGE_TRIPRE_TIMEOUT_INTR_RAW              (1 << 0)
#define REG_CHARGE_TRIPRE_TIMEOUT_INTR_EN               (1 << 1)
#define REG_CHARGE_TRIPRE_TIMEOUT_INTR_MASK             (1 << 2)
#define REG_CHARGE_TRIPRE_TIMEOUT_INTR_CLR              (1 << 3)
#define REG_CHARGE_FAST_TIMEOUT_INTR_RAW                (1 << 4)
#define REG_CHARGE_FAST_TIMEOUT_INTR_EN                 (1 << 5)
#define REG_CHARGE_FAST_TIMEOUT_INTR_MASK               (1 << 6)
#define REG_CHARGE_FAST_TIMEOUT_INTR_CLR                (1 << 7)

// REG_61
#define REG_CHARGE_TRIPRE_TIMEOUT_CNT_SHIFT             0
#define REG_CHARGE_TRIPRE_TIMEOUT_CNT_MASK              (0x3FFF << REG_CHARGE_TRIPRE_TIMEOUT_CNT_SHIFT)
#define REG_CHARGE_TRIPRE_TIMEOUT_CNT(n)                BITFIELD_VAL(REG_CHARGE_TRIPRE_TIMEOUT_CNT, n)

// REG_62
#define REG_CHARGE_FAST_TIMEOUT_CNT_SHIFT               0
#define REG_CHARGE_FAST_TIMEOUT_CNT_MASK                (0x3FFF << REG_CHARGE_FAST_TIMEOUT_CNT_SHIFT)
#define REG_CHARGE_FAST_TIMEOUT_CNT(n)                  BITFIELD_VAL(REG_CHARGE_FAST_TIMEOUT_CNT, n)

// REG_63
#define REG_CHARGE_TEMP_COLD_INTR_RAW                   (1 << 0)
#define REG_CHARGE_TEMP_COLD_INTR_EN                    (1 << 1)
#define REG_CHARGE_TEMP_COLD_INTR_MASK                  (1 << 2)
#define REG_CHARGE_TEMP_COLD_INTR_CLR                   (1 << 3)
#define REG_CHARGE_TEMP_HOT_INTR_RAW                    (1 << 4)
#define REG_CHARGE_TEMP_HOT_INTR_EN                     (1 << 5)
#define REG_CHARGE_TEMP_HOT_INTR_MASK                   (1 << 6)
#define REG_CHARGE_TEMP_HOT_INTR_CLR                    (1 << 7)
#define REG_CHARGE_TEMP_COOL_INTR_RAW                   (1 << 8)
#define REG_CHARGE_TEMP_COOL_INTR_EN                    (1 << 9)
#define REG_CHARGE_TEMP_COOL_INTR_MASK                  (1 << 10)
#define REG_CHARGE_TEMP_COOL_INTR_CLR                   (1 << 11)
#define REG_CHARGE_TEMP_WARM_INTR_RAW                   (1 << 12)
#define REG_CHARGE_TEMP_WARM_INTR_EN                    (1 << 13)
#define REG_CHARGE_TEMP_WARM_INTR_MASK                  (1 << 14)
#define REG_CHARGE_TEMP_WARM_INTR_CLR                   (1 << 15)

// REG_64
#define REG_CHARGE_DONE_INTR_RAW                        (1 << 0)
#define REG_CHARGE_DONE_INTR_EN                         (1 << 1)
#define REG_CHARGE_DONE_INTR_MASK                       (1 << 2)
#define REG_CHARGE_DONE_INTR_CLR                        (1 << 3)

// REG_65
#define REG_VSYS_SC_INTR_RAW                            (1 << 0)
#define REG_VSYS_SC_INTR_EN                             (1 << 1)
#define REG_VSYS_SC_INTR_MASK                           (1 << 2)
#define REG_VSYS_SC_INTR_CLR                            (1 << 3)
#define REG_ACIN_OV_INTR_RAW                            (1 << 4)
#define REG_ACIN_OV_INTR_EN                             (1 << 5)
#define REG_ACIN_OV_INTR_MASK                           (1 << 6)
#define REG_ACIN_OV_INTR_CLR                            (1 << 7)
#define REG_ACIN_CL_INTR_RAW                            (1 << 8)
#define REG_ACIN_CL_INTR_EN                             (1 << 9)
#define REG_ACIN_CL_INTR_MASK                           (1 << 10)
#define REG_ACIN_CL_INTR_CLR                            (1 << 11)
#define REG_OTP_INTR_RAW                                (1 << 12)
#define REG_OTP_INTR_EN                                 (1 << 13)
#define REG_OTP_INTR_MASK                               (1 << 14)
#define REG_OTP_INTR_CLR                                (1 << 15)

#define REG_CHARGE_VSYS_SC_INTR_RAW                     REG_VSYS_SC_INTR_RAW
#define REG_CHARGE_VSYS_SC_INTR_EN                      REG_VSYS_SC_INTR_EN
#define REG_CHARGE_VSYS_SC_INTR_MASK                    REG_VSYS_SC_INTR_MASK
#define REG_CHARGE_VSYS_SC_INTR_CLR                     REG_VSYS_SC_INTR_CLR
#define REG_CHARGE_ACIN_OV_INTR_RAW                     REG_ACIN_OV_INTR_RAW
#define REG_CHARGE_ACIN_OV_INTR_EN                      REG_ACIN_OV_INTR_EN
#define REG_CHARGE_ACIN_OV_INTR_MASK                    REG_ACIN_OV_INTR_MASK
#define REG_CHARGE_ACIN_OV_INTR_CLR                     REG_ACIN_OV_INTR_CLR
#define REG_CHARGE_ACIN_CL_INTR_RAW                     REG_ACIN_CL_INTR_RAW
#define REG_CHARGE_ACIN_CL_INTR_EN                      REG_ACIN_CL_INTR_EN
#define REG_CHARGE_ACIN_CL_INTR_MASK                    REG_ACIN_CL_INTR_MASK
#define REG_CHARGE_ACIN_CL_INTR_CLR                     REG_ACIN_CL_INTR_CLR
#define REG_CHARGE_OTP_INTR_RAW                         REG_OTP_INTR_RAW
#define REG_CHARGE_OTP_INTR_EN                          REG_OTP_INTR_EN
#define REG_CHARGE_OTP_INTR_MASK                        REG_OTP_INTR_MASK
#define REG_CHARGE_OTP_INTR_CLR                         REG_OTP_INTR_CLR

// REG_66
#define REG_VSYS_LOW_INTR_RAW                           (1 << 0)
#define REG_VSYS_LOW_INTR_EN                            (1 << 1)
#define REG_VSYS_LOW_INTR_MASK                          (1 << 2)
#define REG_VSYS_LOW_INTR_CLR                           (1 << 3)

#define REG_CHARGE_VSYS_LOW_INTR_RAW                    REG_VSYS_LOW_INTR_RAW
#define REG_CHARGE_VSYS_LOW_INTR_EN                     REG_VSYS_LOW_INTR_EN
#define REG_CHARGE_VSYS_LOW_INTR_MASK                   REG_VSYS_LOW_INTR_MASK
#define REG_CHARGE_VSYS_LOW_INTR_CLR                    REG_VSYS_LOW_INTR_CLR

// REG_67
#define REG_VSYS_LOW_POS_DB_CNT_SHIFT                   0
#define REG_VSYS_LOW_POS_DB_CNT_MASK                    (0x7F << REG_VSYS_LOW_POS_DB_CNT_SHIFT)
#define REG_VSYS_LOW_POS_DB_CNT(n)                      BITFIELD_VAL(REG_VSYS_LOW_POS_DB_CNT, n)
#define REG_VSYS_LOW_NEG_DB_CNT_SHIFT                   7
#define REG_VSYS_LOW_NEG_DB_CNT_MASK                    (0xF << REG_VSYS_LOW_NEG_DB_CNT_SHIFT)
#define REG_VSYS_LOW_NEG_DB_CNT(n)                      BITFIELD_VAL(REG_VSYS_LOW_NEG_DB_CNT, n)

// REG_68
#define REG_ACIN_CL_POS_DB_CNT_SHIFT                    0
#define REG_ACIN_CL_POS_DB_CNT_MASK                     (0xFF << REG_ACIN_CL_POS_DB_CNT_SHIFT)
#define REG_ACIN_CL_POS_DB_CNT(n)                       BITFIELD_VAL(REG_ACIN_CL_POS_DB_CNT, n)
#define REG_ACIN_CL_NEG_DB_CNT_SHIFT                    8
#define REG_ACIN_CL_NEG_DB_CNT_MASK                     (0x1F << REG_ACIN_CL_NEG_DB_CNT_SHIFT)
#define REG_ACIN_CL_NEG_DB_CNT(n)                       BITFIELD_VAL(REG_ACIN_CL_NEG_DB_CNT, n)

// REG_69
#define REG_VSYS_SC_POS_DB_CNT_SHIFT                    0
#define REG_VSYS_SC_POS_DB_CNT_MASK                     (0x1F << REG_VSYS_SC_POS_DB_CNT_SHIFT)
#define REG_VSYS_SC_POS_DB_CNT(n)                       BITFIELD_VAL(REG_VSYS_SC_POS_DB_CNT, n)
#define REG_VSYS_SC_NEG_DB_CNT_SHIFT                    5
#define REG_VSYS_SC_NEG_DB_CNT_MASK                     (0x1F << REG_VSYS_SC_NEG_DB_CNT_SHIFT)
#define REG_VSYS_SC_NEG_DB_CNT(n)                       BITFIELD_VAL(REG_VSYS_SC_NEG_DB_CNT, n)

// REG_6A
#define REG_VBAT_LOW_POS_DB_CNT_SHIFT                   0
#define REG_VBAT_LOW_POS_DB_CNT_MASK                    (0x7F << REG_VBAT_LOW_POS_DB_CNT_SHIFT)
#define REG_VBAT_LOW_POS_DB_CNT(n)                      BITFIELD_VAL(REG_VBAT_LOW_POS_DB_CNT, n)
#define REG_VBAT_LOW_NEG_DB_CNT_SHIFT                   7
#define REG_VBAT_LOW_NEG_DB_CNT_MASK                    (0xF << REG_VBAT_LOW_NEG_DB_CNT_SHIFT)
#define REG_VBAT_LOW_NEG_DB_CNT(n)                      BITFIELD_VAL(REG_VBAT_LOW_NEG_DB_CNT, n)

// REG_6B
#define REG_PRE2CC_POS_DB_CNT_SHIFT                     0
#define REG_PRE2CC_POS_DB_CNT_MASK                      (0xF << REG_PRE2CC_POS_DB_CNT_SHIFT)
#define REG_PRE2CC_POS_DB_CNT(n)                        BITFIELD_VAL(REG_PRE2CC_POS_DB_CNT, n)
#define REG_PRE2CC_NEG_DB_CNT_SHIFT                     4
#define REG_PRE2CC_NEG_DB_CNT_MASK                      (0x7F << REG_PRE2CC_NEG_DB_CNT_SHIFT)
#define REG_PRE2CC_NEG_DB_CNT(n)                        BITFIELD_VAL(REG_PRE2CC_NEG_DB_CNT, n)

// REG_6C
#define REG_RECHARGE_POS_DB_CNT_SHIFT                   0
#define REG_RECHARGE_POS_DB_CNT_MASK                    (0x3FF << REG_RECHARGE_POS_DB_CNT_SHIFT)
#define REG_RECHARGE_POS_DB_CNT(n)                      BITFIELD_VAL(REG_RECHARGE_POS_DB_CNT, n)
#define REG_RECHARGE_NEG_DB_CNT                         (1 << 10)

// REG_6D
#define REG_ITERM_POS_DB_CNT_SHIFT                      0
#define REG_ITERM_POS_DB_CNT_MASK                       (0x1FFF << REG_ITERM_POS_DB_CNT_SHIFT)
#define REG_ITERM_POS_DB_CNT(n)                         BITFIELD_VAL(REG_ITERM_POS_DB_CNT, n)
#define REG_ITERM_NEG_DB_CNT                            (1 << 13)

// REG_6E
#define REG_TEMP_COLD_POS_DB_CNT_SHIFT                  0
#define REG_TEMP_COLD_POS_DB_CNT_MASK                   (0x7F << REG_TEMP_COLD_POS_DB_CNT_SHIFT)
#define REG_TEMP_COLD_POS_DB_CNT(n)                     BITFIELD_VAL(REG_TEMP_COLD_POS_DB_CNT, n)
#define REG_TEMP_COLD_NEG_DB_CNT_SHIFT                  7
#define REG_TEMP_COLD_NEG_DB_CNT_MASK                   (0x7F << REG_TEMP_COLD_NEG_DB_CNT_SHIFT)
#define REG_TEMP_COLD_NEG_DB_CNT(n)                     BITFIELD_VAL(REG_TEMP_COLD_NEG_DB_CNT, n)

// REG_6F
#define REG_TEMP_COOL_POS_DB_CNT_SHIFT                  0
#define REG_TEMP_COOL_POS_DB_CNT_MASK                   (0x7F << REG_TEMP_COOL_POS_DB_CNT_SHIFT)
#define REG_TEMP_COOL_POS_DB_CNT(n)                     BITFIELD_VAL(REG_TEMP_COOL_POS_DB_CNT, n)
#define REG_TEMP_COOL_NEG_DB_CNT_SHIFT                  7
#define REG_TEMP_COOL_NEG_DB_CNT_MASK                   (0x7F << REG_TEMP_COOL_NEG_DB_CNT_SHIFT)
#define REG_TEMP_COOL_NEG_DB_CNT(n)                     BITFIELD_VAL(REG_TEMP_COOL_NEG_DB_CNT, n)

// REG_70
#define REG_TEMP_WARM_POS_DB_CNT_SHIFT                  0
#define REG_TEMP_WARM_POS_DB_CNT_MASK                   (0x7F << REG_TEMP_WARM_POS_DB_CNT_SHIFT)
#define REG_TEMP_WARM_POS_DB_CNT(n)                     BITFIELD_VAL(REG_TEMP_WARM_POS_DB_CNT, n)
#define REG_TEMP_WARM_NEG_DB_CNT_SHIFT                  7
#define REG_TEMP_WARM_NEG_DB_CNT_MASK                   (0x7F << REG_TEMP_WARM_NEG_DB_CNT_SHIFT)
#define REG_TEMP_WARM_NEG_DB_CNT(n)                     BITFIELD_VAL(REG_TEMP_WARM_NEG_DB_CNT, n)

// REG_71
#define REG_TEMP_HOT_POS_DB_CNT_SHIFT                   0
#define REG_TEMP_HOT_POS_DB_CNT_MASK                    (0x7F << REG_TEMP_HOT_POS_DB_CNT_SHIFT)
#define REG_TEMP_HOT_POS_DB_CNT(n)                      BITFIELD_VAL(REG_TEMP_HOT_POS_DB_CNT, n)
#define REG_TEMP_HOT_NEG_DB_CNT_SHIFT                   7
#define REG_TEMP_HOT_NEG_DB_CNT_MASK                    (0x7F << REG_TEMP_HOT_NEG_DB_CNT_SHIFT)
#define REG_TEMP_HOT_NEG_DB_CNT(n)                      BITFIELD_VAL(REG_TEMP_HOT_NEG_DB_CNT, n)

// REG_72
#define REG_CV_MODE_POS_DB_CNT_SHIFT                    0
#define REG_CV_MODE_POS_DB_CNT_MASK                     (0x7F << REG_CV_MODE_POS_DB_CNT_SHIFT)
#define REG_CV_MODE_POS_DB_CNT(n)                       BITFIELD_VAL(REG_CV_MODE_POS_DB_CNT, n)
#define REG_CV_MODE_NEG_DB_CNT_SHIFT                    7
#define REG_CV_MODE_NEG_DB_CNT_MASK                     (0x7F << REG_CV_MODE_NEG_DB_CNT_SHIFT)
#define REG_CV_MODE_NEG_DB_CNT(n)                       BITFIELD_VAL(REG_CV_MODE_NEG_DB_CNT, n)

// REG_73
#define REG_OTP_POS_DB_CNT_SHIFT                        0
#define REG_OTP_POS_DB_CNT_MASK                         (0x7F << REG_OTP_POS_DB_CNT_SHIFT)
#define REG_OTP_POS_DB_CNT(n)                           BITFIELD_VAL(REG_OTP_POS_DB_CNT, n)
#define REG_OTP_NEG_DB_CNT                              (1 << 7)

// REG_74
#define REG_ACIN_OV_POS_DB_CNT                          (1 << 0)
#define REG_ACIN_OV_NEG_DB_CNT_SHIFT                    1
#define REG_ACIN_OV_NEG_DB_CNT_MASK                     (0x1F << REG_ACIN_OV_NEG_DB_CNT_SHIFT)
#define REG_ACIN_OV_NEG_DB_CNT(n)                       BITFIELD_VAL(REG_ACIN_OV_NEG_DB_CNT, n)

// REG_75
#define REG_ACIN_OK_POS_DB_CNT_SHIFT                    0
#define REG_ACIN_OK_POS_DB_CNT_MASK                     (0x7F << REG_ACIN_OK_POS_DB_CNT_SHIFT)
#define REG_ACIN_OK_POS_DB_CNT(n)                       BITFIELD_VAL(REG_ACIN_OK_POS_DB_CNT, n)
#define REG_ACIN_OK_NEG_DB_CNT_SHIFT                    7
#define REG_ACIN_OK_NEG_DB_CNT_MASK                     (0x7F << REG_ACIN_OK_NEG_DB_CNT_SHIFT)
#define REG_ACIN_OK_NEG_DB_CNT(n)                       BITFIELD_VAL(REG_ACIN_OK_NEG_DB_CNT, n)

// REG_76
#define RESERVED_DIG_15_0_SHIFT                         0
#define RESERVED_DIG_15_0_MASK                          (0xFFFF << RESERVED_DIG_15_0_SHIFT)
#define RESERVED_DIG_15_0(n)                            BITFIELD_VAL(RESERVED_DIG_15_0, n)

// REG_77
#define RESERVED_DIG_31_16_SHIFT                        0
#define RESERVED_DIG_31_16_MASK                         (0xFFFF << RESERVED_DIG_31_16_SHIFT)
#define RESERVED_DIG_31_16(n)                           BITFIELD_VAL(RESERVED_DIG_31_16, n)
// REG_78
#define RESERVED_ANA_8_0_SHIFT                          0
#define RESERVED_ANA_8_0_MASK                           (0x1FF << RESERVED_ANA_8_0_SHIFT)
#define RESERVED_ANA_8_0(n)                             BITFIELD_VAL(RESERVED_ANA_8_0, n)

// REG_79
#define RESERVED_ANA_17_9_SHIFT                         0
#define RESERVED_ANA_17_9_MASK                          (0x1FF << RESERVED_ANA_17_9_SHIFT)
#define RESERVED_ANA_17_9(n)                            BITFIELD_VAL(RESERVED_ANA_17_9, n)

// REG_7A
#define REG_CHG_CHIP_ADDR_I2C_SHIFT                     0
#define REG_CHG_CHIP_ADDR_I2C_MASK                      (0x7F << REG_CHG_CHIP_ADDR_I2C_SHIFT)
#define REG_CHG_CHIP_ADDR_I2C(n)                        BITFIELD_VAL(REG_CHG_CHIP_ADDR_I2C, n)

// REG_7B
#define REG_CHG_AC_ON                                   (1 << 0)
#define REG_CHG_AC_ON_DB                                (1 << 1)
#define PRE2CC                                          (1 << 2)
#define PRE2CC_DB                                       (1 << 3)
#define ITERM                                           (1 << 4)
#define ITERM_DB                                        (1 << 5)
#define RECHARGE                                        (1 << 6)
#define RECHARGE_DB                                     (1 << 7)
#define CV_MODE                                         (1 << 8)
#define CV_MODE_DB                                      (1 << 9)
#define TEMP_COLD                                       (1 << 10)
#define TEMP_COLD_DB                                    (1 << 11)
#define TEMP_COOL                                       (1 << 12)
#define TEMP_COOL_DB                                    (1 << 13)
#define TEMP_WARM                                       (1 << 14)
#define TEMP_WARM_DB                                    (1 << 15)

// REG_7C
#define TEMP_HOT                                        (1 << 0)
#define TEMP_HOT_DB                                     (1 << 1)
#define ACIN_OK                                         (1 << 2)
#define ACIN_OK_DB                                      (1 << 3)
#define ACIN_CL                                         (1 << 4)
#define ACIN_CL_DB                                      (1 << 5)
#define ACIN_OV                                         (1 << 6)
#define ACIN_OV_DB                                      (1 << 7)
#define VBAT_LOW                                        (1 << 8)
#define VBAT_LOW_DB                                     (1 << 9)
#define OTP                                             (1 << 10)
#define OTP_DB                                          (1 << 11)
#define VSYS_SC                                         (1 << 12)
#define VSYS_SC_DB                                      (1 << 13)
#define VSYS_LOW                                        (1 << 14)
#define VSYS_LOW_DB                                     (1 << 15)

// REG_7D
#define ACINGTVBAT                                      (1 << 0)
#define BAT_TEMP_OK                                     (1 << 1)
#define POWER_UP_STATE_SHIFT                            5
#define POWER_UP_STATE_MASK                             (0x7 << POWER_UP_STATE_SHIFT)
#define POWER_UP_STATE(n)                               BITFIELD_VAL(POWER_UP_STATE, n)
#define MAIN_STATE_SHIFT                                8
#define MAIN_STATE_MASK                                 (0x3 << MAIN_STATE_SHIFT)
#define MAIN_STATE(n)                                   BITFIELD_VAL(MAIN_STATE, n)
#define CHARGE_STATE_SHIFT                              10
#define CHARGE_STATE_MASK                               (0x7 << CHARGE_STATE_SHIFT)
#define CHARGE_STATE(n)                                 BITFIELD_VAL(CHARGE_STATE, n)

// REG_80
#define CHRG_INTR                                       (1 << 0)
#define AC_ON_DET_IN_INTR                               (1 << 1)
#define AC_ON_DET_OUT_INTR                              (1 << 2)
#define EFUSE_INIT_TIMEOUT_INTR                         (1 << 3)
#define EFUSE_INIT_DONE_INTR                            (1 << 4)
#define CHARGE_TRIPRE_TIMEOUT_INTR                      (1 << 5)
#define CHARGE_FAST_TIMEOUT_INTR                        (1 << 6)
#define CHARGE_TEMP_COLD_INTR                           (1 << 7)
#define CHARGE_TEMP_HOT_INTR                            (1 << 8)
#define CHARGE_TEMP_COOL_INTR                           (1 << 9)
#define CHARGE_TEMP_WARM_INTR                           (1 << 10)
#define CHARGE_DONE_INTR                                (1 << 11)

#define IRQ1_STATUS_SHIFT                               0
#define IRQ1_STATUS_MASK                                (0xFFF << IRQ1_STATUS_SHIFT)
#define IRQ1_STATUS(n)                                  BITFIELD_VAL(IRQ1_STATUS, n)
#define IRQ1_NUM                                        12

// REG_81
#define VSYS_SC_INTR                                    (1 << 0)
#define ACIN_OV_INTR                                    (1 << 1)
#define ACIN_CL_INTR                                    (1 << 2)
#define OTP_INTR                                        (1 << 3)
#define VSYS_LOW_INTR                                   (1 << 4)

#define IRQ2_STATUS_SHIFT                               0
#define IRQ2_STATUS_MASK                                (0x1F << IRQ2_STATUS_SHIFT)
#define IRQ2_STATUS(n)                                  BITFIELD_VAL(IRQ2_STATUS, n)

#endif
