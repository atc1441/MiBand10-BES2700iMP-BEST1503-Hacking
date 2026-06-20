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

#ifndef __REG_CAPSENSOR_BEST1306_H__
#define __REG_CAPSENSOR_BEST1306_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

struct CAPSENSOR_T {
    __IO uint32_t REG_00;
    __IO uint32_t REG_04;
    __IO uint32_t REG_08;
    __IO uint32_t REG_0C;
    __IO uint32_t REG_10;
    __IO uint32_t REG_14;
    __IO uint32_t REG_18;
    __IO uint32_t REG_1C;
    __IO uint32_t REG_20;
    __IO uint32_t REG_24;
    __IO uint32_t REG_28;
    __IO uint32_t REG_2C;
    __IO uint32_t REG_30;
    __IO uint32_t REG_34;
    __IO uint32_t REG_38;
    __IO uint32_t REG_3C;
    __IO uint32_t REG_40;
    __IO uint32_t REG_44;
    __IO uint32_t REG_48;
    __IO uint32_t REG_4C;
    __IO uint32_t REG_50;
    __IO uint32_t REG_54;
    __IO uint32_t REG_58;
    __IO uint32_t REG_5C;
    __IO uint32_t REG_60;
    __IO uint32_t REG_64;
    __IO uint32_t REG_68;
    __IO uint32_t REG_6C;
    __IO uint32_t REG_70;
    __IO uint32_t REG_74;
    __IO uint32_t REG_78;
    __IO uint32_t REG_7C;
    __IO uint32_t REG_80;
    __IO uint32_t REG_84;
    __IO uint32_t REG_88;
    __IO uint32_t REG_8C;
    __IO uint32_t REG_90;
    __IO uint32_t REG_94;
    __IO uint32_t REG_98;
    __IO uint32_t REG_9C;
    __IO uint32_t REG_A0;
    __IO uint32_t REG_A4;
    __IO uint32_t REG_A8;
    __IO uint32_t REG_AC;
    __IO uint32_t REG_B0;
    __IO uint32_t REG_B4;
    __IO uint32_t REG_B8;
    __IO uint32_t REG_BC;
    __IO uint32_t REG_C0;
    __IO uint32_t REG_C4;
    __IO uint32_t REG_C8;
    __IO uint32_t REG_CC;
    __IO uint32_t REG_D0;
    __IO uint32_t REG_D4;
    __IO uint32_t REG_D8;
    __IO uint32_t REG_DC;
    __IO uint32_t REG_E0;
    __IO uint32_t REG_E4;
    __IO uint32_t REG_E8;
    __IO uint32_t REG_EC;
    __IO uint32_t REG_F0;
    __IO uint32_t REG_F4;
    __IO uint32_t REG_F8;
    __IO uint32_t REG_FC;
    __IO uint32_t REG_100;
    __IO uint32_t REG_104;
    __IO uint32_t REG_108;
    __IO uint32_t REG_10C;
    __IO uint32_t REG_110;
    __IO uint32_t REG_114;
    __IO uint32_t REG_118;
    __IO uint32_t REG_11C;
};

/*0x4008b004*/
// reg_04
#define CAP_FSM_EN                                          (1 << 0)
#define CAP_CDC_SDM_MODE_DR                                 (1 << 1)
#define CAP_CDC_SDM_MODE_REG                                (1 << 2)
#define CAP_CDC_SAR_N_MODE_REG                              (1 << 3)
#define CAP_CDC_SAR_P_MODE_DR                               (1 << 4)
#define CAP_CDC_SAR_P_MODE_REG                              (1 << 5)
#define CAP_CDC_CAL_N_MODE_DR                               (1 << 6)
#define CAP_CDC_CAL_N_MODE_REG                              (1 << 7)
#define CAP_CDC_CAL_P_MODE_DR                               (1 << 8)
#define CAP_CDC_CAL_P_MODE_REG                              (1 << 9)
#define CAP_CDC_P_MODE_DR                                   (1 << 10)
#define CAP_CDC_P_MODE_REG                                  (1 << 11)
#define CAP_CDC_PU_VREF0P7_REG                              (1 << 12)
#define CAP_CDC_PU_LDO_REG                                  (1 << 13)
#define CAP_CDC_PU_CDC_DR                                   (1 << 14)
#define CAP_CDC_PU_CDC_REG                                  (1 << 15)
#define CAP_CDC_PU_N_CHAN_DR                                (1 << 16)
#define CAP_CDC_PU_N_CHAN_REG                               (1 << 17)
#define CAP_CDC_PU_P_CHAN_DR                                (1 << 18)
#define CAP_CDC_PU_P_CHAN_REG                               (1 << 19)
#define CAP_ACTIVE_CH_NUM(n)                                (((n) & 0x7) << 20)
#define CAP_ACTIVE_CH_NUM_MASK                              (0x7 << 20)
#define CAP_ACTIVE_CH_NUM_SHIFT                             (20)
#define CAP_OPERATION_MODE(n)                               (((n) & 0x3) << 23)
#define CAP_OPERATION_MODE_MASK                             (0x3 << 23)
#define CAP_OPERATION_MODE_SHIFT                            (23)
#define CAP_POWER_MODE_DR                                   (1 << 25)
#define CAP_POWER_MODE_REG                                  (1 << 26)
#define CAP_CDC_DLATCH_IBSEL(n)                             (((n) & 0xF) << 27)
#define CAP_CDC_DLATCH_IBSEL_MASK                           (0xF << 27)
#define CAP_CDC_DLATCH_IBSEL_SHIFT                          (27)
#define CAP_CDC_RSTB                                        (1 << 31)

// reg_08
#define CAP_CDC_SAR_P_BIT_IN_DR                             (1 << 0)
#define CAP_CDC_BASELINE_N_BIT_IN_DR                        (1 << 1)
#define CAP_CDC_BASELINE_P_BIT_IN_DR                        (1 << 2)
#define CAP_NDAC_MODE_DR                                    (1 << 3)
#define CAP_NDAC_MODE_REG                                   (1 << 4)
#define CAP_PDAC_MODE_DR                                    (1 << 5)
#define CAP_PDAC_MODE_REG                                   (1 << 6)
#define CAP_CDC_N_MODE_DR                                   (1 << 7)
#define CAP_CDC_N_MODE_REG                                  (1 << 8)
#define CAP_CDC_LDO_VSEL_REG(n)                             (((n) & 0x7) << 9)
#define CAP_CDC_LDO_VSEL_REG_MASK                           (0x7 << 9)
#define CAP_CDC_LDO_VSEL_REG_SHIFT                          (9)
#define CAP_CDC_LDO_ISEL_REG(n)                             (((n) & 0xF) << 12)
#define CAP_CDC_LDO_ISEL_REG_MASK                           (0xF << 12)
#define CAP_CDC_LDO_ISEL_REG_SHIFT                          (12)
#define CAP_CDC_CK_EDGE_SEL                                 (1 << 16)
#define CAP_THRESHOLD_MODE(n)                               (((n) & 0x3) << 17)
#define CAP_THRESHOLD_MODE_MASK                             (0x3 << 17)
#define CAP_THRESHOLD_MODE_SHIFT                            (17)
#define CAP_CDC_PD_IDLE_CHAN(n)                             (((n) & 0xFF) << 19)
#define CAP_CDC_PD_IDLE_CHAN_MASK                           (0xFF << 19)
#define CAP_CDC_PD_IDLE_CHAN_SHIFT                          (19)
#define CAP_CDC_SDM_OP_CO1(n)                               (((n) & 0x7) << 27)
#define CAP_CDC_SDM_OP_CO1_MASK                             (0x7 << 27)
#define CAP_CDC_SDM_OP_CO1_SHIFT                            (27)

// reg_0c
#define CAP_CH_IDX_0_SAMPLE_POINT(n)                        (((n) & 0xFF) << 0)
#define CAP_CH_IDX_0_SAMPLE_POINT_MASK                      (0xFF << 0)
#define CAP_CH_IDX_0_SAMPLE_POINT_SHIFT                     (0)
#define CAP_CH_IDX_1_SAMPLE_POINT(n)                        (((n) & 0xFF) << 8)
#define CAP_CH_IDX_1_SAMPLE_POINT_MASK                      (0xFF << 8)
#define CAP_CH_IDX_1_SAMPLE_POINT_SHIFT                     (8)
#define CAP_CH_IDX_2_SAMPLE_POINT(n)                        (((n) & 0xFF) << 16)
#define CAP_CH_IDX_2_SAMPLE_POINT_MASK                      (0xFF << 16)
#define CAP_CH_IDX_2_SAMPLE_POINT_SHIFT                     (16)
#define CAP_CH_IDX_3_SAMPLE_POINT(n)                        (((n) & 0xFF) << 24)
#define CAP_CH_IDX_3_SAMPLE_POINT_MASK                      (0xFF << 24)
#define CAP_CH_IDX_3_SAMPLE_POINT_SHIFT                     (24)

// reg_10
#define CAP_CH_IDX_4_SAMPLE_POINT(n)                        (((n) & 0xFF) << 0)
#define CAP_CH_IDX_4_SAMPLE_POINT_MASK                      (0xFF << 0)
#define CAP_CH_IDX_4_SAMPLE_POINT_SHIFT                     (0)
#define CAP_CH_IDX_5_SAMPLE_POINT(n)                        (((n) & 0xFF) << 8)
#define CAP_CH_IDX_5_SAMPLE_POINT_MASK                      (0xFF << 8)
#define CAP_CH_IDX_5_SAMPLE_POINT_SHIFT                     (8)
#define CAP_CH_IDX_6_SAMPLE_POINT(n)                        (((n) & 0xFF) << 16)
#define CAP_CH_IDX_6_SAMPLE_POINT_MASK                      (0xFF << 16)
#define CAP_CH_IDX_6_SAMPLE_POINT_SHIFT                     (16)
#define CAP_CH_IDX_7_SAMPLE_POINT(n)                        (((n) & 0xFF) << 24)
#define CAP_CH_IDX_7_SAMPLE_POINT_MASK                      (0xFF << 24)
#define CAP_CH_IDX_7_SAMPLE_POINT_SHIFT                     (24)

// reg_14
#define CAP_CDC_BASELINE_N_OUT_CH0_R_REG(n)                 (((n) & 0xFF) << 0)
#define CAP_CDC_BASELINE_N_OUT_CH0_R_REG_MASK               (0xFF << 0)
#define CAP_CDC_BASELINE_N_OUT_CH0_R_REG_SHIFT              (0)
#define CAP_CDC_BASELINE_N_OUT_CH1_R_REG(n)                 (((n) & 0xFF) << 8)
#define CAP_CDC_BASELINE_N_OUT_CH1_R_REG_MASK               (0xFF << 8)
#define CAP_CDC_BASELINE_N_OUT_CH1_R_REG_SHIFT              (8)
#define CAP_CDC_BASELINE_N_OUT_CH2_R_REG(n)                 (((n) & 0xFF) << 16)
#define CAP_CDC_BASELINE_N_OUT_CH2_R_REG_MASK               (0xFF << 16)
#define CAP_CDC_BASELINE_N_OUT_CH2_R_REG_SHIFT              (16)
#define CAP_CDC_BASELINE_N_OUT_CH3_R_REG(n)                 (((n) & 0xFF) << 24)
#define CAP_CDC_BASELINE_N_OUT_CH3_R_REG_MASK               (0xFF << 24)
#define CAP_CDC_BASELINE_N_OUT_CH3_R_REG_SHIFT              (24)

// reg_18
#define CAP_CDC_BASELINE_N_OUT_CH4_R_REG(n)                 (((n) & 0xFF) << 0)
#define CAP_CDC_BASELINE_N_OUT_CH4_R_REG_MASK               (0xFF << 0)
#define CAP_CDC_BASELINE_N_OUT_CH4_R_REG_SHIFT              (0)
#define CAP_CDC_BASELINE_N_OUT_CH5_R_REG(n)                 (((n) & 0xFF) << 8)
#define CAP_CDC_BASELINE_N_OUT_CH5_R_REG_MASK               (0xFF << 8)
#define CAP_CDC_BASELINE_N_OUT_CH5_R_REG_SHIFT              (8)
#define CAP_CDC_BASELINE_N_OUT_CH6_R_REG(n)                 (((n) & 0xFF) << 16)
#define CAP_CDC_BASELINE_N_OUT_CH6_R_REG_MASK               (0xFF << 16)
#define CAP_CDC_BASELINE_N_OUT_CH6_R_REG_SHIFT              (16)
#define CAP_CDC_BASELINE_N_OUT_CH7_R_REG(n)                 (((n) & 0xFF) << 24)
#define CAP_CDC_BASELINE_N_OUT_CH7_R_REG_MASK               (0xFF << 24)
#define CAP_CDC_BASELINE_N_OUT_CH7_R_REG_SHIFT              (24)

// reg_1c
#define CAP_CDC_BASELINE_P_OUT_CH0_REG(n)                   (((n) & 0x3FF) << 0)
#define CAP_CDC_BASELINE_P_OUT_CH0_REG_MASK                 (0x3FF << 0)
#define CAP_CDC_BASELINE_P_OUT_CH0_REG_SHIFT                (0)
#define CAP_CDC_BASELINE_P_OUT_CH1_REG(n)                   (((n) & 0x3FF) << 10)
#define CAP_CDC_BASELINE_P_OUT_CH1_REG_MASK                 (0x3FF << 10)
#define CAP_CDC_BASELINE_P_OUT_CH1_REG_SHIFT                (10)
#define CAP_CDC_BASELINE_P_OUT_CH2_REG(n)                   (((n) & 0x3FF) << 20)
#define CAP_CDC_BASELINE_P_OUT_CH2_REG_MASK                 (0x3FF << 20)
#define CAP_CDC_BASELINE_P_OUT_CH2_REG_SHIFT                (20)

// reg_20
#define CAP_CDC_BASELINE_P_OUT_CH3_REG(n)                   (((n) & 0x3FF) << 0)
#define CAP_CDC_BASELINE_P_OUT_CH3_REG_MASK                 (0x3FF << 0)
#define CAP_CDC_BASELINE_P_OUT_CH3_REG_SHIFT                (0)
#define CAP_CDC_BASELINE_P_OUT_CH4_REG(n)                   (((n) & 0x3FF) << 10)
#define CAP_CDC_BASELINE_P_OUT_CH4_REG_MASK                 (0x3FF << 10)
#define CAP_CDC_BASELINE_P_OUT_CH4_REG_SHIFT                (10)
#define CAP_CDC_BASELINE_P_OUT_CH5_REG(n)                   (((n) & 0x3FF) << 20)
#define CAP_CDC_BASELINE_P_OUT_CH5_REG_MASK                 (0x3FF << 20)
#define CAP_CDC_BASELINE_P_OUT_CH5_REG_SHIFT                (20)

// reg_24
#define CAP_CDC_BASELINE_P_OUT_CH6_REG(n)                   (((n) & 0x3FF) << 0)
#define CAP_CDC_BASELINE_P_OUT_CH6_REG_MASK                 (0x3FF << 0)
#define CAP_CDC_BASELINE_P_OUT_CH6_REG_SHIFT                (0)
#define CAP_CDC_BASELINE_P_OUT_CH7_REG(n)                   (((n) & 0x3FF) << 10)
#define CAP_CDC_BASELINE_P_OUT_CH7_REG_MASK                 (0x3FF << 10)
#define CAP_CDC_BASELINE_P_OUT_CH7_REG_SHIFT                (10)

// reg_28
#define CAP_CDC_BASELINE_P_OUT_CH0_R(n)                     (((n) & 0x3FF) << 0)
#define CAP_CDC_BASELINE_P_OUT_CH0_R_MASK                   (0x3FF << 0)
#define CAP_CDC_BASELINE_P_OUT_CH0_R_SHIFT                  (0)
#define CAP_CDC_BASELINE_P_OUT_CH1_R(n)                     (((n) & 0x3FF) << 10)
#define CAP_CDC_BASELINE_P_OUT_CH1_R_MASK                   (0x3FF << 10)
#define CAP_CDC_BASELINE_P_OUT_CH1_R_SHIFT                  (10)
#define CAP_CDC_BASELINE_P_OUT_CH2_R(n)                     (((n) & 0x3FF) << 20)
#define CAP_CDC_BASELINE_P_OUT_CH2_R_MASK                   (0x3FF << 20)
#define CAP_CDC_BASELINE_P_OUT_CH2_R_SHIFT                  (20)

// reg_2c
#define CAP_CDC_BASELINE_P_OUT_CH3_R(n)                     (((n) & 0x3FF) << 0)
#define CAP_CDC_BASELINE_P_OUT_CH3_R_MASK                   (0x3FF << 0)
#define CAP_CDC_BASELINE_P_OUT_CH3_R_SHIFT                  (0)
#define CAP_CDC_BASELINE_P_OUT_CH4_R(n)                     (((n) & 0x3FF) << 10)
#define CAP_CDC_BASELINE_P_OUT_CH4_R_MASK                   (0x3FF << 10)
#define CAP_CDC_BASELINE_P_OUT_CH4_R_SHIFT                  (10)
#define CAP_CDC_BASELINE_P_OUT_CH5_R(n)                     (((n) & 0x3FF) << 20)
#define CAP_CDC_BASELINE_P_OUT_CH5_R_MASK                   (0x3FF << 20)
#define CAP_CDC_BASELINE_P_OUT_CH5_R_SHIFT                  (20)

// reg_30
#define CAP_CDC_BASELINE_P_OUT_CH6_R(n)                     (((n) & 0x3FF) << 0)
#define CAP_CDC_BASELINE_P_OUT_CH6_R_MASK                   (0x3FF << 0)
#define CAP_CDC_BASELINE_P_OUT_CH6_R_SHIFT                  (0)
#define CAP_CDC_BASELINE_P_OUT_CH7_R(n)                     (((n) & 0x3FF) << 10)
#define CAP_CDC_BASELINE_P_OUT_CH7_R_MASK                   (0x3FF << 10)
#define CAP_CDC_BASELINE_P_OUT_CH7_R_SHIFT                  (10)

// reg_34
#define CAP_CDC_BASELINE_N_OUT_CH0_REG(n)                   (((n) & 0xFF) << 0)
#define CAP_CDC_BASELINE_N_OUT_CH0_REG_MASK                 (0xFF << 0)
#define CAP_CDC_BASELINE_N_OUT_CH0_REG_SHIFT                (0)
#define CAP_CDC_BASELINE_N_OUT_CH1_REG(n)                   (((n) & 0xFF) << 8)
#define CAP_CDC_BASELINE_N_OUT_CH1_REG_MASK                 (0xFF << 8)
#define CAP_CDC_BASELINE_N_OUT_CH1_REG_SHIFT                (8)
#define CAP_CDC_BASELINE_N_OUT_CH2_REG(n)                   (((n) & 0xFF) << 16)
#define CAP_CDC_BASELINE_N_OUT_CH2_REG_MASK                 (0xFF << 16)
#define CAP_CDC_BASELINE_N_OUT_CH2_REG_SHIFT                (16)
#define CAP_CDC_BASELINE_N_OUT_CH3_REG(n)                   (((n) & 0xFF) << 24)
#define CAP_CDC_BASELINE_N_OUT_CH3_REG_MASK                 (0xFF << 24)
#define CAP_CDC_BASELINE_N_OUT_CH3_REG_SHIFT                (24)

// reg_38
#define CAP_CDC_BASELINE_N_OUT_CH4_REG(n)                   (((n) & 0xFF) << 0)
#define CAP_CDC_BASELINE_N_OUT_CH4_REG_MASK                 (0xFF << 0)
#define CAP_CDC_BASELINE_N_OUT_CH4_REG_SHIFT                (0)
#define CAP_CDC_BASELINE_N_OUT_CH5_REG(n)                   (((n) & 0xFF) << 8)
#define CAP_CDC_BASELINE_N_OUT_CH5_REG_MASK                 (0xFF << 8)
#define CAP_CDC_BASELINE_N_OUT_CH5_REG_SHIFT                (8)
#define CAP_CDC_BASELINE_N_OUT_CH6_REG(n)                   (((n) & 0xFF) << 16)
#define CAP_CDC_BASELINE_N_OUT_CH6_REG_MASK                 (0xFF << 16)
#define CAP_CDC_BASELINE_N_OUT_CH6_REG_SHIFT                (16)
#define CAP_CDC_BASELINE_N_OUT_CH7_REG(n)                   (((n) & 0xFF) << 24)
#define CAP_CDC_BASELINE_N_OUT_CH7_REG_MASK                 (0xFF << 24)
#define CAP_CDC_BASELINE_N_OUT_CH7_REG_SHIFT                (24)

// reg_3c
#define CAP_CDC_BASELINE_N_OUT_CH0_R(n)                     (((n) & 0xFF) << 0)
#define CAP_CDC_BASELINE_N_OUT_CH0_R_MASK                   (0xFF << 0)
#define CAP_CDC_BASELINE_N_OUT_CH0_R_SHIFT                  (0)
#define CAP_CDC_BASELINE_N_OUT_CH1_R(n)                     (((n) & 0xFF) << 8)
#define CAP_CDC_BASELINE_N_OUT_CH1_R_MASK                   (0xFF << 8)
#define CAP_CDC_BASELINE_N_OUT_CH1_R_SHIFT                  (8)
#define CAP_CDC_BASELINE_N_OUT_CH2_R(n)                     (((n) & 0xFF) << 16)
#define CAP_CDC_BASELINE_N_OUT_CH2_R_MASK                   (0xFF << 16)
#define CAP_CDC_BASELINE_N_OUT_CH2_R_SHIFT                  (16)
#define CAP_CDC_BASELINE_N_OUT_CH3_R(n)                     (((n) & 0xFF) << 24)
#define CAP_CDC_BASELINE_N_OUT_CH3_R_MASK                   (0xFF << 24)
#define CAP_CDC_BASELINE_N_OUT_CH3_R_SHIFT                  (24)

// reg_40
#define CAP_CDC_BASELINE_N_OUT_CH4_R(n)                     (((n) & 0xFF) << 0)
#define CAP_CDC_BASELINE_N_OUT_CH4_R_MASK                   (0xFF << 0)
#define CAP_CDC_BASELINE_N_OUT_CH4_R_SHIFT                  (0)
#define CAP_CDC_BASELINE_N_OUT_CH5_R(n)                     (((n) & 0xFF) << 8)
#define CAP_CDC_BASELINE_N_OUT_CH5_R_MASK                   (0xFF << 8)
#define CAP_CDC_BASELINE_N_OUT_CH5_R_SHIFT                  (8)
#define CAP_CDC_BASELINE_N_OUT_CH6_R(n)                     (((n) & 0xFF) << 16)
#define CAP_CDC_BASELINE_N_OUT_CH6_R_MASK                   (0xFF << 16)
#define CAP_CDC_BASELINE_N_OUT_CH6_R_SHIFT                  (16)
#define CAP_CDC_BASELINE_N_OUT_CH7_R(n)                     (((n) & 0xFF) << 24)
#define CAP_CDC_BASELINE_N_OUT_CH7_R_MASK                   (0xFF << 24)
#define CAP_CDC_BASELINE_N_OUT_CH7_R_SHIFT                  (24)

// reg_44
#define CAP_CDC_SAR_P_OUT_CH0_REG(n)                        (((n) & 0x3FF) << 0)
#define CAP_CDC_SAR_P_OUT_CH0_REG_MASK                      (0x3FF << 0)
#define CAP_CDC_SAR_P_OUT_CH0_REG_SHIFT                     (0)
#define CAP_CDC_SAR_P_OUT_CH1_REG(n)                        (((n) & 0x3FF) << 10)
#define CAP_CDC_SAR_P_OUT_CH1_REG_MASK                      (0x3FF << 10)
#define CAP_CDC_SAR_P_OUT_CH1_REG_SHIFT                     (10)
#define CAP_CDC_SAR_P_OUT_CH2_REG(n)                        (((n) & 0x3FF) << 20)
#define CAP_CDC_SAR_P_OUT_CH2_REG_MASK                      (0x3FF << 20)
#define CAP_CDC_SAR_P_OUT_CH2_REG_SHIFT                     (20)

// reg_48
#define CAP_CDC_SAR_P_OUT_CH3_REG(n)                        (((n) & 0x3FF) << 0)
#define CAP_CDC_SAR_P_OUT_CH3_REG_MASK                      (0x3FF << 0)
#define CAP_CDC_SAR_P_OUT_CH3_REG_SHIFT                     (0)
#define CAP_CDC_SAR_P_OUT_CH4_REG(n)                        (((n) & 0x3FF) << 10)
#define CAP_CDC_SAR_P_OUT_CH4_REG_MASK                      (0x3FF << 10)
#define CAP_CDC_SAR_P_OUT_CH4_REG_SHIFT                     (10)
#define CAP_CDC_SAR_P_OUT_CH5_REG(n)                        (((n) & 0x3FF) << 20)
#define CAP_CDC_SAR_P_OUT_CH5_REG_MASK                      (0x3FF << 20)
#define CAP_CDC_SAR_P_OUT_CH5_REG_SHIFT                     (20)

// reg_4c
#define CAP_CDC_SAR_P_OUT_CH6_REG(n)                        (((n) & 0x3FF) << 0)
#define CAP_CDC_SAR_P_OUT_CH6_REG_MASK                      (0x3FF << 0)
#define CAP_CDC_SAR_P_OUT_CH6_REG_SHIFT                     (0)
#define CAP_CDC_SAR_P_OUT_CH7_REG(n)                        (((n) & 0x3FF) << 10)
#define CAP_CDC_SAR_P_OUT_CH7_REG_MASK                      (0x3FF << 10)
#define CAP_CDC_SAR_P_OUT_CH7_REG_SHIFT                     (10)

// reg_50
#define CAP_CDC_SAR_P_OUT_CH0_R(n)                          (((n) & 0x3FF) << 0)
#define CAP_CDC_SAR_P_OUT_CH0_R_MASK                        (0x3FF << 0)
#define CAP_CDC_SAR_P_OUT_CH0_R_SHIFT                       (0)
#define CAP_CDC_SAR_P_OUT_CH1_R(n)                          (((n) & 0x3FF) << 10)
#define CAP_CDC_SAR_P_OUT_CH1_R_MASK                        (0x3FF << 10)
#define CAP_CDC_SAR_P_OUT_CH1_R_SHIFT                       (10)
#define CAP_CDC_SAR_P_OUT_CH2_R(n)                          (((n) & 0x3FF) << 20)
#define CAP_CDC_SAR_P_OUT_CH2_R_MASK                        (0x3FF << 20)
#define CAP_CDC_SAR_P_OUT_CH2_R_SHIFT                       (20)

// reg_54
#define CAP_CDC_SAR_P_OUT_CH3_R(n)                          (((n) & 0x3FF) << 0)
#define CAP_CDC_SAR_P_OUT_CH3_R_MASK                        (0x3FF << 0)
#define CAP_CDC_SAR_P_OUT_CH3_R_SHIFT                       (0)
#define CAP_CDC_SAR_P_OUT_CH4_R(n)                          (((n) & 0x3FF) << 10)
#define CAP_CDC_SAR_P_OUT_CH4_R_MASK                        (0x3FF << 10)
#define CAP_CDC_SAR_P_OUT_CH4_R_SHIFT                       (10)
#define CAP_CDC_SAR_P_OUT_CH5_R(n)                          (((n) & 0x3FF) << 20)
#define CAP_CDC_SAR_P_OUT_CH5_R_MASK                        (0x3FF << 20)
#define CAP_CDC_SAR_P_OUT_CH5_R_SHIFT                       (20)

// reg_58
#define CAP_CDC_SAR_P_OUT_CH6_R(n)                          (((n) & 0x3FF) << 0)
#define CAP_CDC_SAR_P_OUT_CH6_R_MASK                        (0x3FF << 0)
#define CAP_CDC_SAR_P_OUT_CH6_R_SHIFT                       (0)
#define CAP_CDC_SAR_P_OUT_CH7_R(n)                          (((n) & 0x3FF) << 10)
#define CAP_CDC_SAR_P_OUT_CH7_R_MASK                        (0x3FF << 10)
#define CAP_CDC_SAR_P_OUT_CH7_R_SHIFT                       (10)

// reg_5c
#define CAP_CDC_SAR_N_OUT_CH0_REG(n)                        (((n) & 0x3FF) << 0)
#define CAP_CDC_SAR_N_OUT_CH0_REG_MASK                      (0x3FF << 0)
#define CAP_CDC_SAR_N_OUT_CH0_REG_SHIFT                     (0)
#define CAP_CDC_SAR_N_OUT_CH1_REG(n)                        (((n) & 0x3FF) << 10)
#define CAP_CDC_SAR_N_OUT_CH1_REG_MASK                      (0x3FF << 10)
#define CAP_CDC_SAR_N_OUT_CH1_REG_SHIFT                     (10)
#define CAP_CDC_SAR_N_OUT_CH2_REG(n)                        (((n) & 0x3FF) << 20)
#define CAP_CDC_SAR_N_OUT_CH2_REG_MASK                      (0x3FF << 20)
#define CAP_CDC_SAR_N_OUT_CH2_REG_SHIFT                     (20)

// reg_60
#define CAP_CDC_SAR_N_OUT_CH3_REG(n)                        (((n) & 0x3FF) << 0)
#define CAP_CDC_SAR_N_OUT_CH3_REG_MASK                      (0x3FF << 0)
#define CAP_CDC_SAR_N_OUT_CH3_REG_SHIFT                     (0)
#define CAP_CDC_SAR_N_OUT_CH4_REG(n)                        (((n) & 0x3FF) << 10)
#define CAP_CDC_SAR_N_OUT_CH4_REG_MASK                      (0x3FF << 10)
#define CAP_CDC_SAR_N_OUT_CH4_REG_SHIFT                     (10)
#define CAP_CDC_SAR_N_OUT_CH5_REG(n)                        (((n) & 0x3FF) << 20)
#define CAP_CDC_SAR_N_OUT_CH5_REG_MASK                      (0x3FF << 20)
#define CAP_CDC_SAR_N_OUT_CH5_REG_SHIFT                     (20)

// reg_64
#define CAP_CDC_SAR_N_OUT_CH6_REG(n)                        (((n) & 0x3FF) << 0)
#define CAP_CDC_SAR_N_OUT_CH6_REG_MASK                      (0x3FF << 0)
#define CAP_CDC_SAR_N_OUT_CH6_REG_SHIFT                     (0)
#define CAP_CDC_SAR_N_OUT_CH7_REG(n)                        (((n) & 0x3FF) << 10)
#define CAP_CDC_SAR_N_OUT_CH7_REG_MASK                      (0x3FF << 10)
#define CAP_CDC_SAR_N_OUT_CH7_REG_SHIFT                     (10)

// reg_68
#define CAP_LP_SLEEP_TIME(n)                                (((n) & 0x3FFF) << 0)
#define CAP_LP_SLEEP_TIME_MASK                              (0x3FFF << 0)
#define CAP_LP_SLEEP_TIME_SHIFT                             (0)
#define CAP_MODE3_SAR_ADJUST_VALUE(n)                       (((n) & 0x3) << 14)
#define CAP_MODE3_SAR_ADJUST_VALUE_MASK                     (0x3 << 14)
#define CAP_MODE3_SAR_ADJUST_VALUE_SHIFT                    (14)
#define CAP_FIFO_DATA_NUM_TH(n)                             (((n) & 0xFF) << 16)
#define CAP_FIFO_DATA_NUM_TH_MASK                           (0xFF << 16)
#define CAP_FIFO_DATA_NUM_TH_SHIFT                          (16)

// reg_6c
#define CAP_CDC_CAP_PLUS_SEL_CH0_REG(n)                     (((n) & 0x7) << 0)
#define CAP_CDC_CAP_PLUS_SEL_CH0_REG_MASK                   (0x7 << 0)
#define CAP_CDC_CAP_PLUS_SEL_CH0_REG_SHIFT                  (0)
#define CAP_CDC_CAP_PLUS_SEL_CH1_REG(n)                     (((n) & 0x7) << 3)
#define CAP_CDC_CAP_PLUS_SEL_CH1_REG_MASK                   (0x7 << 3)
#define CAP_CDC_CAP_PLUS_SEL_CH1_REG_SHIFT                  (3)
#define CAP_CDC_CAP_PLUS_SEL_CH2_REG(n)                     (((n) & 0x7) << 6)
#define CAP_CDC_CAP_PLUS_SEL_CH2_REG_MASK                   (0x7 << 6)
#define CAP_CDC_CAP_PLUS_SEL_CH2_REG_SHIFT                  (6)
#define CAP_CDC_CAP_PLUS_SEL_CH3_REG(n)                     (((n) & 0x7) << 9)
#define CAP_CDC_CAP_PLUS_SEL_CH3_REG_MASK                   (0x7 << 9)
#define CAP_CDC_CAP_PLUS_SEL_CH3_REG_SHIFT                  (9)
#define CAP_CDC_CAP_PLUS_SEL_CH4_REG(n)                     (((n) & 0x7) << 12)
#define CAP_CDC_CAP_PLUS_SEL_CH4_REG_MASK                   (0x7 << 12)
#define CAP_CDC_CAP_PLUS_SEL_CH4_REG_SHIFT                  (12)
#define CAP_CDC_CAP_PLUS_SEL_CH5_REG(n)                     (((n) & 0x7) << 15)
#define CAP_CDC_CAP_PLUS_SEL_CH5_REG_MASK                   (0x7 << 15)
#define CAP_CDC_CAP_PLUS_SEL_CH5_REG_SHIFT                  (15)
#define CAP_CDC_CAP_PLUS_SEL_CH6_REG(n)                     (((n) & 0x7) << 18)
#define CAP_CDC_CAP_PLUS_SEL_CH6_REG_MASK                   (0x7 << 18)
#define CAP_CDC_CAP_PLUS_SEL_CH6_REG_SHIFT                  (18)
#define CAP_CDC_CAP_PLUS_SEL_CH7_REG(n)                     (((n) & 0x7) << 21)
#define CAP_CDC_CAP_PLUS_SEL_CH7_REG_MASK                   (0x7 << 21)
#define CAP_CDC_CAP_PLUS_SEL_CH7_REG_SHIFT                  (21)
#define CAP_CDC_C2X_CH0_REG                                 (1 << 24)
#define CAP_CDC_C2X_CH1_REG                                 (1 << 25)
#define CAP_CDC_C2X_CH2_REG                                 (1 << 26)
#define CAP_CDC_C2X_CH3_REG                                 (1 << 27)
#define CAP_CDC_C2X_CH4_REG                                 (1 << 28)
#define CAP_CDC_C2X_CH5_REG                                 (1 << 29)
#define CAP_CDC_C2X_CH6_REG                                 (1 << 30)
#define CAP_CDC_C2X_CH7_REG                                 (1 << 31)

// reg_70
#define CAP_CDC_SDM_OP_CIN1_CH0_REG(n)                      (((n) & 0x7) << 0)
#define CAP_CDC_SDM_OP_CIN1_CH0_REG_MASK                    (0x7 << 0)
#define CAP_CDC_SDM_OP_CIN1_CH0_REG_SHIFT                   (0)
#define CAP_CDC_SDM_OP_CIN1_CH1_REG(n)                      (((n) & 0x7) << 3)
#define CAP_CDC_SDM_OP_CIN1_CH1_REG_MASK                    (0x7 << 3)
#define CAP_CDC_SDM_OP_CIN1_CH1_REG_SHIFT                   (3)
#define CAP_CDC_SDM_OP_CIN1_CH2_REG(n)                      (((n) & 0x7) << 6)
#define CAP_CDC_SDM_OP_CIN1_CH2_REG_MASK                    (0x7 << 6)
#define CAP_CDC_SDM_OP_CIN1_CH2_REG_SHIFT                   (6)
#define CAP_CDC_SDM_OP_CIN1_CH3_REG(n)                      (((n) & 0x7) << 9)
#define CAP_CDC_SDM_OP_CIN1_CH3_REG_MASK                    (0x7 << 9)
#define CAP_CDC_SDM_OP_CIN1_CH3_REG_SHIFT                   (9)
#define CAP_CDC_SDM_OP_CIN1_CH4_REG(n)                      (((n) & 0x7) << 12)
#define CAP_CDC_SDM_OP_CIN1_CH4_REG_MASK                    (0x7 << 12)
#define CAP_CDC_SDM_OP_CIN1_CH4_REG_SHIFT                   (12)
#define CAP_CDC_SDM_OP_CIN1_CH5_REG(n)                      (((n) & 0x7) << 15)
#define CAP_CDC_SDM_OP_CIN1_CH5_REG_MASK                    (0x7 << 15)
#define CAP_CDC_SDM_OP_CIN1_CH5_REG_SHIFT                   (15)
#define CAP_CDC_SDM_OP_CIN1_CH6_REG(n)                      (((n) & 0x7) << 18)
#define CAP_CDC_SDM_OP_CIN1_CH6_REG_MASK                    (0x7 << 18)
#define CAP_CDC_SDM_OP_CIN1_CH6_REG_SHIFT                   (18)
#define CAP_CDC_SDM_OP_CIN1_CH7_REG(n)                      (((n) & 0x7) << 21)
#define CAP_CDC_SDM_OP_CIN1_CH7_REG_MASK                    (0x7 << 21)
#define CAP_CDC_SDM_OP_CIN1_CH7_REG_SHIFT                   (21)

// reg_74
#define CAP_MODE1_FILTER_OUT_TH_LOW(n)                      (((n) & 0xFFFFF) << 0)
#define CAP_MODE1_FILTER_OUT_TH_LOW_MASK                    (0xFFFFF << 0)
#define CAP_MODE1_FILTER_OUT_TH_LOW_SHIFT                   (0)
#define CAP_MODE1_SAR_INCREASE_VALUE(n)                     (((n) & 0xF) << 23)
#define CAP_MODE1_SAR_INCREASE_VALUE_MASK                   (0xF << 23)
#define CAP_MODE1_SAR_INCREASE_VALUE_SHIFT                  (23)
#define CAP_MODE1_OVERFLOW_SAR_TH_HI(n)                     (((n) & 0xFF) << 27)
#define CAP_MODE1_OVERFLOW_SAR_TH_HI_MASK                   (0xFF << 27)
#define CAP_MODE1_OVERFLOW_SAR_TH_HI_SHIFT                  (27)

// reg_78
#define CAP_MODE1_FILTER_OUT_TH_HI(n)                       (((n) & 0x1FFFF) << 0)
#define CAP_MODE1_FILTER_OUT_TH_HI_MASK                     (0x1FFFF << 0)
#define CAP_MODE1_FILTER_OUT_TH_HI_SHIFT                    (0)
#define CAP_MODE1_SAR_DECREASE_VALUE(n)                     (((n) & 0xF) << 20)
#define CAP_MODE1_SAR_DECREASE_VALUE_MASK                   (0xF << 20)
#define CAP_MODE1_SAR_DECREASE_VALUE_SHIFT                  (20)
#define CAP_MODE1_OVERFLOW_SAR_TH_LOW(n)                    (((n) & 0xFF) << 24)
#define CAP_MODE1_OVERFLOW_SAR_TH_LOW_MASK                  (0xFF << 24)
#define CAP_MODE1_OVERFLOW_SAR_TH_LOW_SHIFT                 (24)

// reg_7c
#define CAP_FIFO_OUT_LO(n)                                  (((n) & 0xFFFF) << 0)
#define CAP_FIFO_OUT_LO_MASK                                (0xFFFF << 0)
#define CAP_FIFO_OUT_LO_SHIFT                               (0)
#define CAP_FIFO_OUT_HI(n)                                  (((n) & 0x3FFF) << 16)
#define CAP_FIFO_OUT_HI_MASK                                (0x3FFF << 16)
#define CAP_FIFO_OUT_HI_SHIFT                               (16)

// reg_80
#define CAP_CFG_ADC_OUT_PHASE(n)                            (((n) & 0x7) << 0)
#define CAP_CFG_ADC_OUT_PHASE_MASK                          (0x7 << 0)
#define CAP_CFG_ADC_OUT_PHASE_SHIFT                         (0)
#define CAP_CFG_ADC_OUT_DECIM(n)                            (((n) & 0x7) << 3)
#define CAP_CFG_ADC_OUT_DECIM_MASK                          (0x7 << 3)
#define CAP_CFG_ADC_OUT_DECIM_SHIFT                         (3)
#define CAP_CFG_CLK_ADC_INV                                 (1 << 6)
#define CAP_ADC_HBF1_BYPASS                                 (1 << 7)
#define CAP_ADC_HBF2_BYPASS                                 (1 << 8)
#define CAP_DOWN_SEL(n)                                     (((n) & 0x7) << 9)
#define CAP_DOWN_SEL_MASK                                   (0x7 << 9)
#define CAP_DOWN_SEL_SHIFT                                  (9)

// reg_84
#define CAP_HBF2_COEF_01(n)                                 (((n) & 0xFFFF) << 0)
#define CAP_HBF2_COEF_01_MASK                               (0xFFFF << 0)
#define CAP_HBF2_COEF_01_SHIFT                              (0)
#define CAP_HBF2_COEF_02(n)                                 (((n) & 0xFFFF) << 16)
#define CAP_HBF2_COEF_02_MASK                               (0xFFFF << 16)
#define CAP_HBF2_COEF_02_SHIFT                              (16)

// reg_88
#define CAP_HBF2_COEF_03(n)                                 (((n) & 0xFFFF) << 0)
#define CAP_HBF2_COEF_03_MASK                               (0xFFFF << 0)
#define CAP_HBF2_COEF_03_SHIFT                              (0)
#define CAP_HBF2_COEF_04(n)                                 (((n) & 0xFFFF) << 16)
#define CAP_HBF2_COEF_04_MASK                               (0xFFFF << 16)
#define CAP_HBF2_COEF_04_SHIFT                              (16)

// reg_8c
#define CAP_HBF2_COEF_05(n)                                 (((n) & 0xFFFF) << 0)
#define CAP_HBF2_COEF_05_MASK                               (0xFFFF << 0)
#define CAP_HBF2_COEF_05_SHIFT                              (0)
#define CAP_HBF2_COEF_06(n)                                 (((n) & 0xFFFF) << 16)
#define CAP_HBF2_COEF_06_MASK                               (0xFFFF << 16)
#define CAP_HBF2_COEF_06_SHIFT                              (16)

// reg_90
#define CAP_HBF2_COEF_07(n)                                 (((n) & 0xFFFF) << 0)
#define CAP_HBF2_COEF_07_MASK                               (0xFFFF << 0)
#define CAP_HBF2_COEF_07_SHIFT                              (0)
#define CAP_HBF2_COEF_08(n)                                 (((n) & 0xFFFF) << 16)
#define CAP_HBF2_COEF_08_MASK                               (0xFFFF << 16)
#define CAP_HBF2_COEF_08_SHIFT                              (16)

// reg_94
#define CAP_HBF2_COEF_09(n)                                 (((n) & 0xFFFF) << 0)
#define CAP_HBF2_COEF_09_MASK                               (0xFFFF << 0)
#define CAP_HBF2_COEF_09_SHIFT                              (0)

// reg_98
#define CAP_RSVED_CDC(n)                                    (((n) & 0xFFFF) << 0)
#define CAP_RSVED_CDC_MASK                                  (0xFFFF << 0)
#define CAP_RSVED_CDC_SHIFT                                 (0)
#define CAP_SAR_OVERFLOW_DR                                 (1 << 16)
#define CAP_SAR_OVERFLOW_REG                                (1 << 17)
#define CAP_FIFO_SAR_DATA_SEL                               (1 << 18)

// reg_9c
#define CAP_FILTER_TH_FLAG_EN                               (1 << 0)
#define CAP_CH_SCAN_CNT_MATCHED_REG(n)                      (((n) & 0xF) << 1)
#define CAP_CH_SCAN_CNT_MATCHED_REG_MASK                    (0xF << 1)
#define CAP_CH_SCAN_CNT_MATCHED_REG_SHIFT                   (1)
#define CAP_PRESS_INT_TH_VALUE_LO(n)                        (((n) & 0xFFFF) << 5)
#define CAP_PRESS_INT_TH_VALUE_LO_MASK                      (0xFFFF << 5)
#define CAP_PRESS_INT_TH_VALUE_LO_SHIFT                     (5)
#define CAP_PRESS_INT_TH_VALUE_HI(n)                        (((n) & 0x1FF) << 21)
#define CAP_PRESS_INT_TH_VALUE_HI_MASK                      (0x1FF << 21)
#define CAP_PRESS_INT_TH_VALUE_HI_SHIFT                     (21)

// reg_a0
#define CAP_FIFO_IN_LO(n)                                   (((n) & 0xFFFF) << 0)
#define CAP_FIFO_IN_LO_MASK                                 (0xFFFF << 0)
#define CAP_FIFO_IN_LO_SHIFT                                (0)
#define CAP_FIFO_IN_HI(n)                                   (((n) & 0x3FFF) << 16)
#define CAP_FIFO_IN_HI_MASK                                 (0x3FFF << 16)
#define CAP_FIFO_IN_HI_SHIFT                                (16)

// reg_a4
#define CAP_FIFO_CLK_DR                                     (1 << 0)
#define CAP_FIFO_RD_DEBUG_DR                                (1 << 1)
#define CAP_FIFO_RD_DEBUG_REG                               (1 << 2)
#define CAP_FIFO_WR_CLK_DR                                  (1 << 4)
#define CAP_FIFO_RD_DIRECTION_REG                           (1 << 5)
#define CAP_FIFO_RD_START_REG                               (1 << 6)

// reg_a8
#define CAP_CH_CONFIG_IDX_0(n)                              (((n) & 0x7F) << 0)
#define CAP_CH_CONFIG_IDX_0_MASK                            (0x7F << 0)
#define CAP_CH_CONFIG_IDX_0_SHIFT                           (0)
#define CAP_CH_CONFIG_IDX_1(n)                              (((n) & 0x7F) << 7)
#define CAP_CH_CONFIG_IDX_1_MASK                            (0x7F << 7)
#define CAP_CH_CONFIG_IDX_1_SHIFT                           (8)
#define CAP_CH_CONFIG_IDX_2(n)                              (((n) & 0x7F) << 14)
#define CAP_CH_CONFIG_IDX_2_MASK                            (0x7F << 14)
#define CAP_CH_CONFIG_IDX_2_SHIFT                           (16)
#define CAP_CH_CONFIG_IDX_3(n)                              (((n) & 0x7F) << 21)
#define CAP_CH_CONFIG_IDX_3_MASK                            (0x7F << 21)
#define CAP_CH_CONFIG_IDX_3_SHIFT                           (24)

// reg_ac
#define CAP_CH_CONFIG_IDX_4(n)                              (((n) & 0x7F) << 0)
#define CAP_CH_CONFIG_IDX_4_MASK                            (0x7F << 0)
#define CAP_CH_CONFIG_IDX_4_SHIFT                           (0)
#define CAP_CH_CONFIG_IDX_5(n)                              (((n) & 0x7F) << 7)
#define CAP_CH_CONFIG_IDX_5_MASK                            (0x7F << 7)
#define CAP_CH_CONFIG_IDX_5_SHIFT                           (8)
#define CAP_CH_CONFIG_IDX_6(n)                              (((n) & 0x7F) << 14)
#define CAP_CH_CONFIG_IDX_6_MASK                            (0x7F << 14)
#define CAP_CH_CONFIG_IDX_6_SHIFT                           (16)
#define CAP_CH_CONFIG_IDX_7(n)                              (((n) & 0x7F) << 21)
#define CAP_CH_CONFIG_IDX_7_MASK                            (0x7F << 21)
#define CAP_CH_CONFIG_IDX_7_SHIFT                           (24)

// reg_b0
#define CAP_CDC_SDM_CO_COMP(n)                              (((n) & 0x7) << 0)
#define CAP_CDC_SDM_CO_COMP_MASK                            (0x7 << 0)
#define CAP_CDC_SDM_CO_COMP_SHIFT                           (0)
#define CAP_CDC_SDM_CIN_COMP(n)                             (((n) & 0x7) << 3)
#define CAP_CDC_SDM_CIN_COMP_MASK                           (0x7 << 3)
#define CAP_CDC_SDM_CIN_COMP_SHIFT                          (3)
#define CAP_CDC_PU_CHOP                                     (1 << 6)
#define CAP_CDC_SDM_OP_CO3(n)                               (((n) & 0x7) << 7)
#define CAP_CDC_SDM_OP_CO3_MASK                             (0x7 << 7)
#define CAP_CDC_SDM_OP_CO3_SHIFT                            (7)
#define CAP_CDC_SDM_OP_CO2(n)                               (((n) & 0x7) << 10)
#define CAP_CDC_SDM_OP_CO2_MASK                             (0x7 << 10)
#define CAP_CDC_SDM_OP_CO2_SHIFT                            (10)
#define CAP_CDC_SDM_OP_CIN3(n)                              (((n) & 0x7) << 13)
#define CAP_CDC_SDM_OP_CIN3_MASK                            (0x7 << 13)
#define CAP_CDC_SDM_OP_CIN3_SHIFT                           (13)
#define CAP_CDC_SDM_OP_CIN2(n)                              (((n) & 0x7) << 16)
#define CAP_CDC_SDM_OP_CIN2_MASK                            (0x7 << 16)
#define CAP_CDC_SDM_OP_CIN2_SHIFT                           (16)

// reg_b4
#define CAP_THRESHOLD_MODE1_TH(n)                           (((n) & 0x1FF) << 0)
#define CAP_THRESHOLD_MODE1_TH_MASK                         (0x1FF << 0)
#define CAP_THRESHOLD_MODE1_TH_SHIFT                        (0)
#define CAP_THRESHOLD_MODE2_TH(n)                           (((n) & 0x1FFFF) << 12)
#define CAP_THRESHOLD_MODE2_TH_MASK                         (0x1FFFF << 12)
#define CAP_THRESHOLD_MODE2_TH_SHIFT                        (12)

// reg_b8
#define CAP_FILTER_OUT_CMP_REG_LOW(n)                       (((n) & 0xFFFF) << 0)
#define CAP_FILTER_OUT_CMP_REG_LOW_MASK                     (0xFFFF << 0)
#define CAP_FILTER_OUT_CMP_REG_LOW_SHIFT                    (0)
#define CAP_FILTER_OUT_CMP_REG_HI(n)                        (((n) & 0xFFFF) << 16)
#define CAP_FILTER_OUT_CMP_REG_HI_MASK                      (0xFFFF << 16)
#define CAP_FILTER_OUT_CMP_REG_HI_SHIFT                     (16)

// reg_bc
#define CAP_FIFO_REG_WRITE                                  (1 << 0)
#define CAP_FIFO_REG_READ                                   (1 << 1)

// reg_c0
#define CAP_SINGLE_SAR_OUT_WEIGHT_1_HI                      (1 << 0)
#define CAP_SINGLE_SAR_OUT_WEIGHT_2_HI(n)                   (((n) & 0x3) << 1)
#define CAP_SINGLE_SAR_OUT_WEIGHT_2_HI_MASK                 (0x3 << 1)
#define CAP_SINGLE_SAR_OUT_WEIGHT_2_HI_SHIFT                (1)
#define CAP_SINGLE_SAR_OUT_WEIGHT_3_HI(n)                   (((n) & 0x3) << 3)
#define CAP_SINGLE_SAR_OUT_WEIGHT_3_HI_MASK                 (0x3 << 3)
#define CAP_SINGLE_SAR_OUT_WEIGHT_3_HI_SHIFT                (3)
#define CAP_SINGLE_SAR_OUT_WEIGHT_4_HI(n)                   (((n) & 0x7) << 5)
#define CAP_SINGLE_SAR_OUT_WEIGHT_4_HI_MASK                 (0x7 << 5)
#define CAP_SINGLE_SAR_OUT_WEIGHT_4_HI_SHIFT                (5)
#define CAP_SINGLE_SAR_OUT_WEIGHT_5_HI(n)                   (((n) & 0xF) << 8)
#define CAP_SINGLE_SAR_OUT_WEIGHT_5_HI_MASK                 (0xF << 8)
#define CAP_SINGLE_SAR_OUT_WEIGHT_5_HI_SHIFT                (8)
#define CAP_SINGLE_SAR_OUT_WEIGHT_6_HI(n)                   (((n) & 0x1F) << 12)
#define CAP_SINGLE_SAR_OUT_WEIGHT_6_HI_MASK                 (0x1F << 12)
#define CAP_SINGLE_SAR_OUT_WEIGHT_6_HI_SHIFT                (12)
#define CAP_SINGLE_SAR_OUT_WEIGHT_7_HI(n)                   (((n) & 0x1F) << 17)
#define CAP_SINGLE_SAR_OUT_WEIGHT_7_HI_MASK                 (0x1F << 17)
#define CAP_SINGLE_SAR_OUT_WEIGHT_7_HI_SHIFT                (17)
#define CAP_SINGLE_SAR_OUT_WEIGHT_8_HI(n)                   (((n) & 0x3F) << 22)
#define CAP_SINGLE_SAR_OUT_WEIGHT_8_HI_MASK                 (0x3F << 22)
#define CAP_SINGLE_SAR_OUT_WEIGHT_8_HI_SHIFT                (22)

// reg_c4
#define CAP_SINGLE_SAR_OUT_WEIGHT_9_HI(n)                   (((n) & 0x7F) << 0)
#define CAP_SINGLE_SAR_OUT_WEIGHT_9_HI_MASK                 (0x7F << 0)
#define CAP_SINGLE_SAR_OUT_WEIGHT_9_HI_SHIFT                (0)
#define CAP_DIG_DIFF_EN                                     (1 << 7)
#define CAP_DIG_DIFF_TH(n)                                  (((n) & 0x1FF) << 8)
#define CAP_DIG_DIFF_TH_MASK                                (0x1FF << 8)
#define CAP_DIG_DIFF_TH_SHIFT                               (8)

// reg_c8
#define CAP_SINGLE_SAR_OUT_WEIGHT_0_LO(n)                   (((n) & 0xFFFF) << 0)
#define CAP_SINGLE_SAR_OUT_WEIGHT_0_LO_MASK                 (0xFFFF << 0)
#define CAP_SINGLE_SAR_OUT_WEIGHT_0_LO_SHIFT                (0)
#define CAP_SINGLE_SAR_OUT_WEIGHT_1_LO(n)                   (((n) & 0xFFFF) << 16)
#define CAP_SINGLE_SAR_OUT_WEIGHT_1_LO_MASK                 (0xFFFF << 16)
#define CAP_SINGLE_SAR_OUT_WEIGHT_1_LO_SHIFT                (16)

// reg_cc
#define CAP_SINGLE_SAR_OUT_WEIGHT_2_LO(n)                   (((n) & 0xFFFF) << 0)
#define CAP_SINGLE_SAR_OUT_WEIGHT_2_LO_MASK                 (0xFFFF << 0)
#define CAP_SINGLE_SAR_OUT_WEIGHT_2_LO_SHIFT                (0)
#define CAP_SINGLE_SAR_OUT_WEIGHT_3_LO(n)                   (((n) & 0xFFFF) << 16)
#define CAP_SINGLE_SAR_OUT_WEIGHT_3_LO_MASK                 (0xFFFF << 16)
#define CAP_SINGLE_SAR_OUT_WEIGHT_3_LO_SHIFT                (16)

// reg_d0
#define CAP_SINGLE_SAR_OUT_WEIGHT_4_LO(n)                   (((n) & 0xFFFF) << 0)
#define CAP_SINGLE_SAR_OUT_WEIGHT_4_LO_MASK                 (0xFFFF << 0)
#define CAP_SINGLE_SAR_OUT_WEIGHT_4_LO_SHIFT                (0)
#define CAP_SINGLE_SAR_OUT_WEIGHT_5_LO(n)                   (((n) & 0xFFFF) << 16)
#define CAP_SINGLE_SAR_OUT_WEIGHT_5_LO_MASK                 (0xFFFF << 16)
#define CAP_SINGLE_SAR_OUT_WEIGHT_5_LO_SHIFT                (16)

// reg_d4
#define CAP_SINGLE_SAR_OUT_WEIGHT_6_LO(n)                   (((n) & 0xFFFF) << 0)
#define CAP_SINGLE_SAR_OUT_WEIGHT_6_LO_MASK                 (0xFFFF << 0)
#define CAP_SINGLE_SAR_OUT_WEIGHT_6_LO_SHIFT                (0)
#define CAP_SINGLE_SAR_OUT_WEIGHT_7_LO(n)                   (((n) & 0xFFFF) << 16)
#define CAP_SINGLE_SAR_OUT_WEIGHT_7_LO_MASK                 (0xFFFF << 16)
#define CAP_SINGLE_SAR_OUT_WEIGHT_7_LO_SHIFT                (16)

// reg_d8
#define CAP_SINGLE_SAR_OUT_WEIGHT_8_LO(n)                   (((n) & 0xFFFF) << 0)
#define CAP_SINGLE_SAR_OUT_WEIGHT_8_LO_MASK                 (0xFFFF << 0)
#define CAP_SINGLE_SAR_OUT_WEIGHT_8_LO_SHIFT                (0)
#define CAP_SINGLE_SAR_OUT_WEIGHT_9_LO(n)                   (((n) & 0xFFFF) << 16)
#define CAP_SINGLE_SAR_OUT_WEIGHT_9_LO_MASK                 (0xFFFF << 16)
#define CAP_SINGLE_SAR_OUT_WEIGHT_9_LO_SHIFT                (16)

// reg_dc
#define CAP_DIFF_SAR_OUT_WEIGHT_2_HI                        (1 << 0)
#define CAP_DIFF_SAR_OUT_WEIGHT_3_HI                        (1 << 1)
#define CAP_DIFF_SAR_OUT_WEIGHT_4_HI(n)                     (((n) & 0x3) << 2)
#define CAP_DIFF_SAR_OUT_WEIGHT_4_HI_MASK                   (0x3 << 2)
#define CAP_DIFF_SAR_OUT_WEIGHT_4_HI_SHIFT                  (2)
#define CAP_DIFF_SAR_OUT_WEIGHT_5_HI(n)                     (((n) & 0x7) << 4)
#define CAP_DIFF_SAR_OUT_WEIGHT_5_HI_MASK                   (0x7 << 4)
#define CAP_DIFF_SAR_OUT_WEIGHT_5_HI_SHIFT                  (4)
#define CAP_DIFF_SAR_OUT_WEIGHT_6_HI(n)                     (((n) & 0xF) << 7)
#define CAP_DIFF_SAR_OUT_WEIGHT_6_HI_MASK                   (0xF << 7)
#define CAP_DIFF_SAR_OUT_WEIGHT_6_HI_SHIFT                  (7)
#define CAP_DIFF_SAR_OUT_WEIGHT_7_HI(n)                     (((n) & 0xF) << 11)
#define CAP_DIFF_SAR_OUT_WEIGHT_7_HI_MASK                   (0xF << 11)
#define CAP_DIFF_SAR_OUT_WEIGHT_7_HI_SHIFT                  (11)
#define CAP_DIFF_SAR_OUT_WEIGHT_8_HI(n)                     (((n) & 0x1F) << 15)
#define CAP_DIFF_SAR_OUT_WEIGHT_8_HI_MASK                   (0x1F << 15)
#define CAP_DIFF_SAR_OUT_WEIGHT_8_HI_SHIFT                  (15)
#define CAP_DIFF_SAR_OUT_WEIGHT_9_HI(n)                     (((n) & 0x3F) << 20)
#define CAP_DIFF_SAR_OUT_WEIGHT_9_HI_MASK                   (0x3F << 20)
#define CAP_DIFF_SAR_OUT_WEIGHT_9_HI_SHIFT                  (20)

// reg_e0
#define CAP_DIFF_SAR_OUT_WEIGHT_0_LO(n)                     (((n) & 0xFFFF) << 0)
#define CAP_DIFF_SAR_OUT_WEIGHT_0_LO_MASK                   (0xFFFF << 0)
#define CAP_DIFF_SAR_OUT_WEIGHT_0_LO_SHIFT                  (0)
#define CAP_DIFF_SAR_OUT_WEIGHT_1_LO(n)                     (((n) & 0xFFFF) << 16)
#define CAP_DIFF_SAR_OUT_WEIGHT_1_LO_MASK                   (0xFFFF << 16)
#define CAP_DIFF_SAR_OUT_WEIGHT_1_LO_SHIFT                  (16)

// reg_e4
#define CAP_DIFF_SAR_OUT_WEIGHT_2_LO(n)                     (((n) & 0xFFFF) << 0)
#define CAP_DIFF_SAR_OUT_WEIGHT_2_LO_MASK                   (0xFFFF << 0)
#define CAP_DIFF_SAR_OUT_WEIGHT_2_LO_SHIFT                  (0)
#define CAP_DIFF_SAR_OUT_WEIGHT_3_LO(n)                     (((n) & 0xFFFF) << 16)
#define CAP_DIFF_SAR_OUT_WEIGHT_3_LO_MASK                   (0xFFFF << 16)
#define CAP_DIFF_SAR_OUT_WEIGHT_3_LO_SHIFT                  (16)

// reg_e8
#define CAP_DIFF_SAR_OUT_WEIGHT_4_LO(n)                     (((n) & 0xFFFF) << 0)
#define CAP_DIFF_SAR_OUT_WEIGHT_4_LO_MASK                   (0xFFFF << 0)
#define CAP_DIFF_SAR_OUT_WEIGHT_4_LO_SHIFT                  (0)
#define CAP_DIFF_SAR_OUT_WEIGHT_5_LO(n)                     (((n) & 0xFFFF) << 16)
#define CAP_DIFF_SAR_OUT_WEIGHT_5_LO_MASK                   (0xFFFF << 16)
#define CAP_DIFF_SAR_OUT_WEIGHT_5_LO_SHIFT                  (16)

// reg_ec
#define CAP_DIFF_SAR_OUT_WEIGHT_6_LO(n)                     (((n) & 0xFFFF) << 0)
#define CAP_DIFF_SAR_OUT_WEIGHT_6_LO_MASK                   (0xFFFF << 0)
#define CAP_DIFF_SAR_OUT_WEIGHT_6_LO_SHIFT                  (0)
#define CAP_DIFF_SAR_OUT_WEIGHT_7_LO(n)                     (((n) & 0xFFFF) << 16)
#define CAP_DIFF_SAR_OUT_WEIGHT_7_LO_MASK                   (0xFFFF << 16)
#define CAP_DIFF_SAR_OUT_WEIGHT_7_LO_SHIFT                  (16)

// reg_f0
#define CAP_DIFF_SAR_OUT_WEIGHT_8_LO(n)                     (((n) & 0xFFFF) << 0)
#define CAP_DIFF_SAR_OUT_WEIGHT_8_LO_MASK                   (0xFFFF << 0)
#define CAP_DIFF_SAR_OUT_WEIGHT_8_LO_SHIFT                  (0)
#define CAP_DIFF_SAR_OUT_WEIGHT_9_LO(n)                     (((n) & 0xFFFF) << 16)
#define CAP_DIFF_SAR_OUT_WEIGHT_9_LO_MASK                   (0xFFFF << 16)
#define CAP_DIFF_SAR_OUT_WEIGHT_9_LO_SHIFT                  (16)

// reg_f4
#define CAP_CH_SAMPLE_START(n)                              (((n) & 0x7) << 0)
#define CAP_CH_SAMPLE_START_MASK                            (0x7 << 0)
#define CAP_CH_SAMPLE_START_SHIFT                           (0)
#define CAP_CH_SAMPLE_CNT_SEL(n)                            (((n) & 0x7) << 3)
#define CAP_CH_SAMPLE_CNT_SEL_MASK                          (0x7 << 3)
#define CAP_CH_SAMPLE_CNT_SEL_SHIFT                         (3)
#define CAP_CH_SAMPLE_MODE                                  (1 << 6)
#define CAP_FIFO_TAG_EN                                     (1 << 7)
#define CAP_CDC_CAL_BASELINE_DONE_ALL_DR                    (1 << 8)
#define CAP_CDC_CAL_BASELINE_DONE_ALL_REG                   (1 << 9)
#define CAP_CDC_SAR_DONE_ALL_DR                             (1 << 10)
#define CAP_CDC_SAR_DONE_ALL_REG                            (1 << 11)

// reg_f8
#define CAP_NEXT_STATE(n)                                   (((n) & 0x7) << 0)
#define CAP_NEXT_STATE_MASK                                 (0x7 << 0)
#define CAP_NEXT_STATE_SHIFT                                (0)
#define CAP_STATE(n)                                        (((n) & 0x7) << 4)
#define CAP_STATE_MASK                                      (0x7 << 4)
#define CAP_STATE_SHIFT                                     (4)
#define CAP_CH_COUNT(n)                                     (((n) & 0x7) << 8)
#define CAP_CH_COUNT_MASK                                   (0x7 << 8)
#define CAP_CH_COUNT_SHIFT                                  (8)
#define CAP_ALL_FIFO_DATA_VLD_FLAG                          (1 << 12)
#define CAP_ADDR(n)                                         (((n) & 0x7F) << 16)
#define CAP_ADDR_MASK                                       (0x7F << 16)
#define CAP_ADDR_SHIFT                                      (16)

// reg_fc
#define CAP_RD_ADDR_PTR_DEBUG(n)                            (((n) & 0xFF) << 0)
#define CAP_RD_ADDR_PTR_DEBUG_MASK                          (0xFF << 0)
#define CAP_RD_ADDR_PTR_DEBUG_SHIFT                         (0)
#define CAP_CAP_SENSOR_CLK_POL_SEL                          (1 << 8)
#define CAP_FILTER_OUT_OVERFLOW_DR                          (1 << 9)
#define CAP_FILTER_OUT_OVERFLOW_REG                         (1 << 10)
#define CAP_FSM_CLK_DR                                      (1 << 11)
#define CAP_SEL_FSM_CLK_REG                                 (1 << 12)
#define CAP_PU_OSC_CAP_SENSOR_DR                            (1 << 15)
#define CAP_PU_OSC_CAP_SENSOR_REG                           (1 << 16)
#define CAP_CLK_SETTLE_TIME(n)                              (((n) & 0x3FFF) << 18)
#define CAP_CLK_SETTLE_TIME_MASK                            (0x3FFF << 18)
#define CAP_CLK_SETTLE_TIME_SHIFT                           (18)

// reg_100
#define CAP_PU_OSC_INT                                      (1 << 0)
#define CAP_PU_OSC_INT_RAW                                  (1 << 1)
#define CAP_PU_OSC_INT_MASK                                 (1 << 2)
#define CAP_PU_OSC_INT_CLR                                  (1 << 3)
#define CAP_PD_OSC_INT                                      (1 << 4)
#define CAP_PD_OSC_INT_RAW                                  (1 << 5)
#define CAP_PD_OSC_INT_MASK                                 (1 << 6)
#define CAP_PD_OSC_INT_CLR                                  (1 << 7)
#define CAP_CAP_SENSOR_PRESS_INT                            (1 << 8)
#define CAP_CAP_SENSOR_PRESS_INT_RAW                        (1 << 9)
#define CAP_CAP_SENSOR_PRESS_INT_MASK                       (1 << 10)
#define CAP_PRESS_INT_CLR_REG                               (1 << 11)
#define CAP_FP_MODE_RD_INT                                  (1 << 12)
#define CAP_FP_MODE_RD_INT_RAW                              (1 << 13)
#define CAP_FP_MODE_RD_INT_MASK                             (1 << 14)
#define CAP_FP_MODE_RD_INT_CLR_REG                          (1 << 15)
#define CAP_FP_MODE_RD_INT_RAW_EN_REG                       (1 << 16)
#define CAP_SAR_OVERFLOW_INT                                (1 << 17)
#define CAP_SAR_OVERFLOW_INT_RAW                            (1 << 18)
#define CAP_SAR_OVERFLOW_INT_MASK                           (1 << 19)
#define CAP_SAR_OVERFLOW_INT_CLR_REG                        (1 << 20)

// reg_104
#define CAP_RESETN_FILTER_REG_DR                            (1 << 2)
#define CAP_RESETN_FILTER_REG                               (1 << 3)

// reg_108
#define CAP_ADDR_DR                                         (1 << 0)
#define CAP_ADDR_REG(n)                                     (((n) & 0x7F) << 1)
#define CAP_ADDR_REG_MASK                                   (0x7F << 1)
#define CAP_ADDR_REG_SHIFT                                  (1)

// reg_10c
#define CAP_RES_REG_FOR_CAP_DIG(n)                          (((n) & 0xFFFFFFFF) << 0)
#define CAP_RES_REG_FOR_CAP_DIG_MASK                        (0xFFFFFFFF << 0)
#define CAP_RES_REG_FOR_CAP_DIG_SHIFT                       (0)

// reg_110
#define CAP_WR_ADDR_PTR(n)                                  (((n) & 0xFF) << 0)
#define CAP_WR_ADDR_PTR_MASK                                (0xFF << 0)
#define CAP_WR_ADDR_PTR_SHIFT                               (0)
#define CAP_WR_PTR_DR                                       (1 << 8)

// reg_114
#define CAP_RD_ADDR_PTR(n)                                  (((n) & 0xFF) << 0)
#define CAP_RD_ADDR_PTR_MASK                                (0xFF << 0)
#define CAP_RD_ADDR_PTR_SHIFT                               (0)

// reg_118
#define CAP_WR_ADDR_PTR_REG(n)                              (((n) & 0xFF) << 0)
#define CAP_WR_ADDR_PTR_REG_MASK                            (0xFF << 0)
#define CAP_WR_ADDR_PTR_REG_SHIFT                           (0)
#define CAP_WR_PTR_DR                                       (1 << 8)

// reg_11c
#define CAP_RD_ADDR_PTR_REG(n)                              (((n) & 0xFF) << 0)
#define CAP_RD_ADDR_PTR_REG_MASK                            (0xFF << 0)
#define CAP_RD_ADDR_PTR_REG_SHIFT                           (0)
#define CAP_RD_PTR_DR                                       (1 << 8)
/*0x4008b004*/

#ifdef __cplusplus
}
#endif

#endif /*__REG_CAPSENSOR_BEST1306_H__*/
