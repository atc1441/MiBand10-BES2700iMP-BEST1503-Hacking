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
#ifndef __REG_PSRAM_PHY_V4_H__
#define __REG_PSRAM_PHY_V4_H__

#include "plat_types.h"

struct PSRAM_PHY_T {
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
    __IO uint32_t RESERVED_064[0x27];
    __IO uint32_t REG_100;
    __IO uint32_t REG_104;
    __IO uint32_t REG_108;
    __IO uint32_t REG_10C;
    __IO uint32_t REG_110;
    __IO uint32_t REG_114;
};

// reg_000
#define PSRAM_ULP_PHY_CHIP_TYPE                             (1 << 0)
#define PSRAM_ULP_PHY_CHIP_BIT                              (1 << 1)
#define PSRAM_ULP_PHY_MEMORY_WIDTH(n)                       (((n) & 0x3) << 2)
#define PSRAM_ULP_PHY_MEMORY_WIDTH_MASK                     (0x3 << 2)
#define PSRAM_ULP_PHY_MEMORY_WIDTH_SHIFT                    (2)
#define PSRAM_ULP_PHY_FRE_RATIO(n)                          (((n) & 0x3) << 4)
#define PSRAM_ULP_PHY_FRE_RATIO_MASK                        (0x3 << 4)
#define PSRAM_ULP_PHY_FRE_RATIO_SHIFT                       (4)

// reg_004
#define PSRAM_ULP_PHY_CTRL_DELAY(n)                         (((n) & 0x3) << 0)
#define PSRAM_ULP_PHY_CTRL_DELAY_MASK                       (0x3 << 0)
#define PSRAM_ULP_PHY_CTRL_DELAY_SHIFT                      (0)
#define PSRAM_ULP_PHY_RX_DLY_EN                             (1 << 2)
#define PSRAM_ULP_PHY_ALIGN_BYPASS                          (1 << 3)
#define PSRAM_ULP_PHY_PHY_LOOPBACK_EN                       (1 << 4)
#define PSRAM_ULP_PHY_PHY_DUMMY_CYC_EN                      (1 << 5)
#define PSRAM_ULP_PHY_PHY_DLY_AUTO_EN                       (1 << 6)
#define PSRAM_ULP_PHY_SQPI_SAMPLE_SEL(n)                    (((n) & 0xF) << 7)
#define PSRAM_ULP_PHY_SQPI_SAMPLE_SEL_MASK                  (0xF << 7)
#define PSRAM_ULP_PHY_SQPI_SAMPLE_SEL_SHIFT                 (7)
#define PSRAM_ULP_PHY_IDLE_DQ_OEN                           (1 << 11)
#define PSRAM_ULP_PHY_PHY_TX_BYPASS                         (1 << 12)
#define PSRAM_ULP_PHY_ADDR_4BYTES_EN                        (1 << 13)
#define PSRAM_ULP_PHY_IDLE_DQS_OEN                          (1 << 14)
#define PSRAM_ULP_PHY_DQS_DM_MERGE_EN                       (1 << 15)
#define PSRAM_ULP_PHY_REG_WR_WO_DM                          (1 << 16)
#define PSRAM_ULP_PHY_CS_POL                                (1 << 17)

// reg_008
#define PSRAM_ULP_PHY_T_WPST(n)                             (((n) & 0x7) << 0)
#define PSRAM_ULP_PHY_T_WPST_MASK                           (0x7 << 0)
#define PSRAM_ULP_PHY_T_WPST_SHIFT                          (0)

// reg_00c
#define PSRAM_ULP_PHY_T_EXPANDRD(n)                         (((n) & 0x3F) << 0)
#define PSRAM_ULP_PHY_T_EXPANDRD_MASK                       (0x3F << 0)
#define PSRAM_ULP_PHY_T_EXPANDRD_SHIFT                      (0)

// reg_010
#define PSRAM_ULP_PHY_CMD_CONFLICT_CLR                      (1 << 0)

// reg_014
#define PSRAM_ULP_PHY_SQPI_CMD_CONFLICT_CLR                 (1 << 0)

// reg_018
#define PSRAM_ULP_PHY_T_PHYDATA_DLY(n)                      (((n) & 0x3) << 0)
#define PSRAM_ULP_PHY_T_PHYDATA_DLY_MASK                    (0x3 << 0)
#define PSRAM_ULP_PHY_T_PHYDATA_DLY_SHIFT                   (0)

// reg_01c
#define PSRAM_ULP_PHY_CA_TR_EN                              (1 << 0)
#define PSRAM_ULP_PHY_DQS_RDY                               (1 << 1)
#define PSRAM_ULP_PHY_DQS_STS                               (1 << 2)

// reg_020
#define PSRAM_ULP_PHY_SEL_PHY_CLK1X                         (1 << 0)
#define PSRAM_ULP_PHY_CA_SEQ(n)                             (((n) & 0x7) << 1)
#define PSRAM_ULP_PHY_CA_SEQ_MASK                           (0x7 << 1)
#define PSRAM_ULP_PHY_CA_SEQ_SHIFT                          (1)
#define PSRAM_ULP_PHY_RX_SAMP_POS_SEL                       (1 << 4)
#define PSRAM_ULP_PHY_PAD_RSTN                              (1 << 5)
#define PSRAM_ULP_PHY_HYPERBUS_SAMPLE_LAT_EDGE_SEL          (1 << 6)
#define PSRAM_ULP_PHY_ANA_RST                               (1 << 7)
#define PSRAM_ULP_PHY_HYPERBUS_VAR_LAT                      (1 << 8)
#define PSRAM_ULP_PHY_HYPERBUS_CMD_DLY_CNT(n)               (((n) & 0x7) << 9)
#define PSRAM_ULP_PHY_HYPERBUS_CMD_DLY_CNT_MASK             (0x7 << 9)
#define PSRAM_ULP_PHY_HYPERBUS_CMD_DLY_CNT_SHIFT            (9)
#define PSRAM_ULP_PHY_REG_RX_DLY_OF_DESIGN(n)               (((n) & 0xF) << 12)
#define PSRAM_ULP_PHY_REG_RX_DLY_OF_DESIGN_MASK             (0xF << 12)
#define PSRAM_ULP_PHY_REG_RX_DLY_OF_DESIGN_SHIFT            (12)
#define PSRAM_ULP_PHY_CFG_RX_DQS_SEL(n)                     (((n) & 0x3) << 16)
#define PSRAM_ULP_PHY_CFG_RX_DQS_SEL_MASK                   (0x3 << 16)
#define PSRAM_ULP_PHY_CFG_RX_DQS_SEL_SHIFT                  (16)

// reg_040
#define PSRAM_ULP_PHY_PHY_RX_BYPASS                         (1 << 0)
#define PSRAM_ULP_PHY_PHY_SAMP_WITH_CLK                     (1 << 1)
#define PSRAM_ULP_PHY_PHY_RX_BYPASS2                        (1 << 2)
#define PSRAM_ULP_PHY_PHY_SAMP_WITH_CLK2                    (1 << 3)
#define PSRAM_ULP_PHY_REG_PSRAM_LOOPBACK_EN                 (1 << 4)
#define PSRAM_ULP_PHY_REG_PSRAM_LOOPBACK_EN2                (1 << 5)

// reg_044
#define PSRAM_ULP_PHY_CMD_CONFLICT_STS                      (1 << 0)
#define PSRAM_ULP_PHY_PHY_FSM_STATE(n)                      (((n) & 0xF) << 1)
#define PSRAM_ULP_PHY_PHY_FSM_STATE_MASK                    (0xF << 1)
#define PSRAM_ULP_PHY_PHY_FSM_STATE_SHIFT                   (1)
#define PSRAM_ULP_PHY_FIFO_EMPTY_L                          (1 << 5)
#define PSRAM_ULP_PHY_FIFO_EMPTY_H                          (1 << 6)
#define PSRAM_ULP_PHY_SQPI_CMD_CONFLICT_STS                 (1 << 7)
#define PSRAM_ULP_PHY_SQPI_PHY_FSM_STATE(n)                 (((n) & 0xF) << 8)
#define PSRAM_ULP_PHY_SQPI_PHY_FSM_STATE_MASK               (0xF << 8)
#define PSRAM_ULP_PHY_SQPI_PHY_FSM_STATE_SHIFT              (8)

// reg_048
#define PSRAM_ULP_PHY_REG0_PSRAM_DLL(n)                     (((n) & 0xFF) << 0)
#define PSRAM_ULP_PHY_REG0_PSRAM_DLL_MASK                   (0xFF << 0)
#define PSRAM_ULP_PHY_REG0_PSRAM_DLL_SHIFT                  (0)
#define PSRAM_ULP_PHY_REG0_PSRAM_DLL_DLY_INI(n)             (((n) & 0xFF) << 8)
#define PSRAM_ULP_PHY_REG0_PSRAM_DLL_DLY_INI_MASK           (0xFF << 8)
#define PSRAM_ULP_PHY_REG0_PSRAM_DLL_DLY_INI_SHIFT          (8)
#define PSRAM_ULP_PHY_REG0_PSRAM_DLL_PU                     (1 << 16)
#define PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE(n)               (((n) & 0x3) << 17)
#define PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_MASK             (0x3 << 17)
#define PSRAM_ULP_PHY_REG0_PSRAM_DLL_RANGE_SHIFT            (17)
#define PSRAM_ULP_PHY_REG0_PSRAM_DLL_RESETB                 (1 << 19)
#define PSRAM_ULP_PHY_REG0_PSRAM_DLL_SWRC(n)                (((n) & 0x3) << 20)
#define PSRAM_ULP_PHY_REG0_PSRAM_DLL_SWRC_MASK              (0x3 << 20)
#define PSRAM_ULP_PHY_REG0_PSRAM_DLL_SWRC_SHIFT             (20)
#define PSRAM_ULP_PHY_REG0_PSRAM_DLL_CK_RDY                 (1 << 22)

// reg_04c
#define PSRAM_ULP_PHY_REG0_PSRAM_LDO_PRECHARGE              (1 << 0)
#define PSRAM_ULP_PHY_REG0_PSRAM_LDO_PU                     (1 << 1)
#define PSRAM_ULP_PHY_REG0_PSRAM_LDO_RES(n)                 (((n) & 0xF) << 2)
#define PSRAM_ULP_PHY_REG0_PSRAM_LDO_RES_MASK               (0xF << 2)
#define PSRAM_ULP_PHY_REG0_PSRAM_LDO_RES_SHIFT              (2)
#define PSRAM_ULP_PHY_REG0_PSRAM_PU                         (1 << 6)
#define PSRAM_ULP_PHY_REG0_PSRAM_PU2                        (1 << 7)
#define PSRAM_ULP_PHY_REG0_PSRAM_PU_DVDD                    (1 << 8)
#define PSRAM_ULP_PHY_REG0_PSRAM_SWRC(n)                    (((n) & 0x3) << 9)
#define PSRAM_ULP_PHY_REG0_PSRAM_SWRC_MASK                  (0x3 << 9)
#define PSRAM_ULP_PHY_REG0_PSRAM_SWRC_SHIFT                 (9)
#define PSRAM_ULP_PHY_REG0_PSRAM_IEN_DLL_DVDD(n)            (((n) & 0xF) << 11)
#define PSRAM_ULP_PHY_REG0_PSRAM_IEN_DLL_DVDD_MASK          (0xF << 11)
#define PSRAM_ULP_PHY_REG0_PSRAM_IEN_DLL_DVDD_SHIFT         (11)
#define PSRAM_ULP_PHY_REG0_PSRAM_IEN_DVDD(n)                (((n) & 0xF) << 15)
#define PSRAM_ULP_PHY_REG0_PSRAM_IEN_DVDD_MASK              (0xF << 15)
#define PSRAM_ULP_PHY_REG0_PSRAM_IEN_DVDD_SHIFT             (15)
#define PSRAM_ULP_PHY_REG0_PSRAM_ITUNE(n)                   (((n) & 0x7) << 19)
#define PSRAM_ULP_PHY_REG0_PSRAM_ITUNE_MASK                 (0x7 << 19)
#define PSRAM_ULP_PHY_REG0_PSRAM_ITUNE_SHIFT                (19)

// reg_050
#define PSRAM_ULP_PHY_REG0_PSRAM_TXDRV(n)                   (((n) & 0x7) << 0)
#define PSRAM_ULP_PHY_REG0_PSRAM_TXDRV_MASK                 (0x7 << 0)
#define PSRAM_ULP_PHY_REG0_PSRAM_TXDRV_SHIFT                (0)
#define PSRAM_ULP_PHY_REG0_PSRAM_TXDRV_CLK(n)               (((n) & 0x7) << 3)
#define PSRAM_ULP_PHY_REG0_PSRAM_TXDRV_CLK_MASK             (0x7 << 3)
#define PSRAM_ULP_PHY_REG0_PSRAM_TXDRV_CLK_SHIFT            (3)
#define PSRAM_ULP_PHY_REG0_PSRAM_TXDRV_DATA(n)              (((n) & 0x7) << 6)
#define PSRAM_ULP_PHY_REG0_PSRAM_TXDRV_DATA_MASK            (0x7 << 6)
#define PSRAM_ULP_PHY_REG0_PSRAM_TXDRV_DATA_SHIFT           (6)
#define PSRAM_ULP_PHY_REG0_PSRAM_PCK_SEL(n)                 (((n) & 0x3) << 9)
#define PSRAM_ULP_PHY_REG0_PSRAM_PCK_SEL_MASK               (0x3 << 9)
#define PSRAM_ULP_PHY_REG0_PSRAM_PCK_SEL_SHIFT              (9)
#define PSRAM_ULP_PHY_REG0_PSRAM_PSEL_CLK                   (1 << 11)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_BIAS(n)                 (((n) & 0x3) << 12)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_BIAS_MASK               (0x3 << 12)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_BIAS_SHIFT              (12)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_BYPASS                  (1 << 14)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_VREF_TUNE(n)            (((n) & 0xF) << 15)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_VREF_TUNE_MASK          (0xF << 15)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_VREF_TUNE_SHIFT         (15)
#define PSRAM_ULP_PHY_REG0_PSRAM_SAMP_CLK_SELECT            (1 << 19)

// reg_054
#define PSRAM_ULP_PHY_REG0_PSRAM_TXDRV2(n)                  (((n) & 0x7) << 0)
#define PSRAM_ULP_PHY_REG0_PSRAM_TXDRV2_MASK                (0x7 << 0)
#define PSRAM_ULP_PHY_REG0_PSRAM_TXDRV2_SHIFT               (0)
#define PSRAM_ULP_PHY_REG0_PSRAM_TXDRV_CLK2(n)              (((n) & 0x7) << 3)
#define PSRAM_ULP_PHY_REG0_PSRAM_TXDRV_CLK2_MASK            (0x7 << 3)
#define PSRAM_ULP_PHY_REG0_PSRAM_TXDRV_CLK2_SHIFT           (3)
#define PSRAM_ULP_PHY_REG0_PSRAM_TXDRV_DATA2(n)             (((n) & 0x7) << 6)
#define PSRAM_ULP_PHY_REG0_PSRAM_TXDRV_DATA2_MASK           (0x7 << 6)
#define PSRAM_ULP_PHY_REG0_PSRAM_TXDRV_DATA2_SHIFT          (6)
#define PSRAM_ULP_PHY_REG0_PSRAM_PCK_SEL2(n)                (((n) & 0x3) << 9)
#define PSRAM_ULP_PHY_REG0_PSRAM_PCK_SEL2_MASK              (0x3 << 9)
#define PSRAM_ULP_PHY_REG0_PSRAM_PCK_SEL2_SHIFT             (9)
#define PSRAM_ULP_PHY_REG0_PSRAM_PSEL_CLK2                  (1 << 11)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_BIAS2(n)                (((n) & 0x3) << 12)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_BIAS2_MASK              (0x3 << 12)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_BIAS2_SHIFT             (12)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_BYPASS2                 (1 << 14)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_VREF_TUNE2(n)           (((n) & 0xF) << 15)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_VREF_TUNE2_MASK         (0xF << 15)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_VREF_TUNE2_SHIFT        (15)
#define PSRAM_ULP_PHY_REG0_PSRAM_SAMP_CLK_SELECT2           (1 << 19)

// reg_058
#define PSRAM_ULP_PHY_REG0_PSRAM_TX_CEB_DLY(n)              (((n) & 0x1F) << 0)
#define PSRAM_ULP_PHY_REG0_PSRAM_TX_CEB_DLY_MASK            (0x1F << 0)
#define PSRAM_ULP_PHY_REG0_PSRAM_TX_CEB_DLY_SHIFT           (0)
#define PSRAM_ULP_PHY_REG0_PSRAM_TX_CLK_DLY(n)              (((n) & 0x1F) << 5)
#define PSRAM_ULP_PHY_REG0_PSRAM_TX_CLK_DLY_MASK            (0x1F << 5)
#define PSRAM_ULP_PHY_REG0_PSRAM_TX_CLK_DLY_SHIFT           (5)
#define PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY(n)              (((n) & 0x1F) << 10)
#define PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY_MASK            (0x1F << 10)
#define PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY_SHIFT           (10)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_DQS_DLY(n)              (((n) & 0x1F) << 15)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_DQS_DLY_MASK            (0x1F << 15)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_DQS_DLY_SHIFT           (15)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_CLK_DLY(n)              (((n) & 0x1F) << 20)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_CLK_DLY_MASK            (0x1F << 20)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_CLK_DLY_SHIFT           (20)

// reg_05c
#define PSRAM_ULP_PHY_REG0_PSRAM_TX_CEB_DLY2(n)             (((n) & 0x1F) << 0)
#define PSRAM_ULP_PHY_REG0_PSRAM_TX_CEB_DLY2_MASK           (0x1F << 0)
#define PSRAM_ULP_PHY_REG0_PSRAM_TX_CEB_DLY2_SHIFT          (0)
#define PSRAM_ULP_PHY_REG0_PSRAM_TX_CLK_DLY2(n)             (((n) & 0x1F) << 5)
#define PSRAM_ULP_PHY_REG0_PSRAM_TX_CLK_DLY2_MASK           (0x1F << 5)
#define PSRAM_ULP_PHY_REG0_PSRAM_TX_CLK_DLY2_SHIFT          (5)
#define PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY2(n)             (((n) & 0x1F) << 10)
#define PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY2_MASK           (0x1F << 10)
#define PSRAM_ULP_PHY_REG0_PSRAM_TX_DQS_DLY2_SHIFT          (10)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_DQS_DLY2(n)             (((n) & 0x1F) << 15)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_DQS_DLY2_MASK           (0x1F << 15)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_DQS_DLY2_SHIFT          (15)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_CLK_DLY2(n)             (((n) & 0x1F) << 20)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_CLK_DLY2_MASK           (0x1F << 20)
#define PSRAM_ULP_PHY_REG0_PSRAM_RX_CLK_DLY2_SHIFT          (20)

// reg_060
#define PSRAM_ULP_PHY_PSRAM_DLL_DLY_IN(n)                   (((n) & 0x3F) << 0)
#define PSRAM_ULP_PHY_PSRAM_DLL_DLY_IN_MASK                 (0x3F << 0)
#define PSRAM_ULP_PHY_PSRAM_DLL_DLY_IN_SHIFT                (0)
#define PSRAM_ULP_PHY_PSRAM_DLL_LOCK                        (1 << 6)
#define PSRAM_ULP_PHY_PSRAM_DLL_ALL_ZERO                    (1 << 7)
#define PSRAM_ULP_PHY_PSRAM_DLL_ALL_ONE                     (1 << 8)

// reg_100
#define PSRAM_ULP_PHY_REG1_PSRAM_DLL(n)                     (((n) & 0xFF) << 0)
#define PSRAM_ULP_PHY_REG1_PSRAM_DLL_MASK                   (0xFF << 0)
#define PSRAM_ULP_PHY_REG1_PSRAM_DLL_SHIFT                  (0)
#define PSRAM_ULP_PHY_REG1_PSRAM_DLL_DLY_INI(n)             (((n) & 0xFF) << 8)
#define PSRAM_ULP_PHY_REG1_PSRAM_DLL_DLY_INI_MASK           (0xFF << 8)
#define PSRAM_ULP_PHY_REG1_PSRAM_DLL_DLY_INI_SHIFT          (8)
#define PSRAM_ULP_PHY_REG1_PSRAM_DLL_PU                     (1 << 16)
#define PSRAM_ULP_PHY_REG1_PSRAM_DLL_RANGE(n)               (((n) & 0x3) << 17)
#define PSRAM_ULP_PHY_REG1_PSRAM_DLL_RANGE_MASK             (0x3 << 17)
#define PSRAM_ULP_PHY_REG1_PSRAM_DLL_RANGE_SHIFT            (17)
#define PSRAM_ULP_PHY_REG1_PSRAM_DLL_RESETB                 (1 << 19)
#define PSRAM_ULP_PHY_REG1_PSRAM_DLL_SWRC(n)                (((n) & 0x3) << 20)
#define PSRAM_ULP_PHY_REG1_PSRAM_DLL_SWRC_MASK              (0x3 << 20)
#define PSRAM_ULP_PHY_REG1_PSRAM_DLL_SWRC_SHIFT             (20)
#define PSRAM_ULP_PHY_REG1_PSRAM_DLL_CK_RDY                 (1 << 22)

// reg_104
#define PSRAM_ULP_PHY_REG1_PSRAM_LDO_PRECHARGE              (1 << 0)
#define PSRAM_ULP_PHY_REG1_PSRAM_LDO_PU                     (1 << 1)
#define PSRAM_ULP_PHY_REG1_PSRAM_LDO_RES(n)                 (((n) & 0xF) << 2)
#define PSRAM_ULP_PHY_REG1_PSRAM_LDO_RES_MASK               (0xF << 2)
#define PSRAM_ULP_PHY_REG1_PSRAM_LDO_RES_SHIFT              (2)
#define PSRAM_ULP_PHY_REG1_PSRAM_PU                         (1 << 6)
#define PSRAM_ULP_PHY_REG1_PSRAM_PU2                        (1 << 7)
#define PSRAM_ULP_PHY_REG1_PSRAM_PU_DVDD                    (1 << 8)
#define PSRAM_ULP_PHY_REG1_PSRAM_SWRC(n)                    (((n) & 0x3) << 9)
#define PSRAM_ULP_PHY_REG1_PSRAM_SWRC_MASK                  (0x3 << 9)
#define PSRAM_ULP_PHY_REG1_PSRAM_SWRC_SHIFT                 (9)
#define PSRAM_ULP_PHY_REG1_PSRAM_IEN_DLL_DVDD(n)            (((n) & 0xF) << 11)
#define PSRAM_ULP_PHY_REG1_PSRAM_IEN_DLL_DVDD_MASK          (0xF << 11)
#define PSRAM_ULP_PHY_REG1_PSRAM_IEN_DLL_DVDD_SHIFT         (11)
#define PSRAM_ULP_PHY_REG1_PSRAM_IEN_DVDD(n)                (((n) & 0xF) << 15)
#define PSRAM_ULP_PHY_REG1_PSRAM_IEN_DVDD_MASK              (0xF << 15)
#define PSRAM_ULP_PHY_REG1_PSRAM_IEN_DVDD_SHIFT             (15)
#define PSRAM_ULP_PHY_REG1_PSRAM_ITUNE(n)                   (((n) & 0x7) << 19)
#define PSRAM_ULP_PHY_REG1_PSRAM_ITUNE_MASK                 (0x7 << 19)
#define PSRAM_ULP_PHY_REG1_PSRAM_ITUNE_SHIFT                (19)

// reg_108
#define PSRAM_ULP_PHY_REG1_PSRAM_TXDRV(n)                   (((n) & 0x7) << 0)
#define PSRAM_ULP_PHY_REG1_PSRAM_TXDRV_MASK                 (0x7 << 0)
#define PSRAM_ULP_PHY_REG1_PSRAM_TXDRV_SHIFT                (0)
#define PSRAM_ULP_PHY_REG1_PSRAM_TXDRV_CLK(n)               (((n) & 0x7) << 3)
#define PSRAM_ULP_PHY_REG1_PSRAM_TXDRV_CLK_MASK             (0x7 << 3)
#define PSRAM_ULP_PHY_REG1_PSRAM_TXDRV_CLK_SHIFT            (3)
#define PSRAM_ULP_PHY_REG1_PSRAM_TXDRV_DATA(n)              (((n) & 0x7) << 6)
#define PSRAM_ULP_PHY_REG1_PSRAM_TXDRV_DATA_MASK            (0x7 << 6)
#define PSRAM_ULP_PHY_REG1_PSRAM_TXDRV_DATA_SHIFT           (6)
#define PSRAM_ULP_PHY_REG1_PSRAM_PCK_SEL(n)                 (((n) & 0x3) << 9)
#define PSRAM_ULP_PHY_REG1_PSRAM_PCK_SEL_MASK               (0x3 << 9)
#define PSRAM_ULP_PHY_REG1_PSRAM_PCK_SEL_SHIFT              (9)
#define PSRAM_ULP_PHY_REG1_PSRAM_PSEL_CLK                   (1 << 11)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_BIAS(n)                 (((n) & 0x3) << 12)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_BIAS_MASK               (0x3 << 12)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_BIAS_SHIFT              (12)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_BYPASS                  (1 << 14)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_VREF_TUNE(n)            (((n) & 0xF) << 15)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_VREF_TUNE_MASK          (0xF << 15)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_VREF_TUNE_SHIFT         (15)
#define PSRAM_ULP_PHY_REG1_PSRAM_SAMP_CLK_SELECT            (1 << 19)

// reg_10c
#define PSRAM_ULP_PHY_REG1_PSRAM_TXDRV2(n)                  (((n) & 0x7) << 0)
#define PSRAM_ULP_PHY_REG1_PSRAM_TXDRV2_MASK                (0x7 << 0)
#define PSRAM_ULP_PHY_REG1_PSRAM_TXDRV2_SHIFT               (0)
#define PSRAM_ULP_PHY_REG1_PSRAM_TXDRV_CLK2(n)              (((n) & 0x7) << 3)
#define PSRAM_ULP_PHY_REG1_PSRAM_TXDRV_CLK2_MASK            (0x7 << 3)
#define PSRAM_ULP_PHY_REG1_PSRAM_TXDRV_CLK2_SHIFT           (3)
#define PSRAM_ULP_PHY_REG1_PSRAM_TXDRV_DATA2(n)             (((n) & 0x7) << 6)
#define PSRAM_ULP_PHY_REG1_PSRAM_TXDRV_DATA2_MASK           (0x7 << 6)
#define PSRAM_ULP_PHY_REG1_PSRAM_TXDRV_DATA2_SHIFT          (6)
#define PSRAM_ULP_PHY_REG1_PSRAM_PCK_SEL2(n)                (((n) & 0x3) << 9)
#define PSRAM_ULP_PHY_REG1_PSRAM_PCK_SEL2_MASK              (0x3 << 9)
#define PSRAM_ULP_PHY_REG1_PSRAM_PCK_SEL2_SHIFT             (9)
#define PSRAM_ULP_PHY_REG1_PSRAM_PSEL_CLK2                  (1 << 11)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_BIAS2(n)                (((n) & 0x3) << 12)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_BIAS2_MASK              (0x3 << 12)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_BIAS2_SHIFT             (12)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_BYPASS2                 (1 << 14)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_VREF_TUNE2(n)           (((n) & 0xF) << 15)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_VREF_TUNE2_MASK         (0xF << 15)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_VREF_TUNE2_SHIFT        (15)
#define PSRAM_ULP_PHY_REG1_PSRAM_SAMP_CLK_SELECT2           (1 << 19)

// reg_110
#define PSRAM_ULP_PHY_REG1_PSRAM_TX_CEB_DLY(n)              (((n) & 0x1F) << 0)
#define PSRAM_ULP_PHY_REG1_PSRAM_TX_CEB_DLY_MASK            (0x1F << 0)
#define PSRAM_ULP_PHY_REG1_PSRAM_TX_CEB_DLY_SHIFT           (0)
#define PSRAM_ULP_PHY_REG1_PSRAM_TX_CLK_DLY(n)              (((n) & 0x1F) << 5)
#define PSRAM_ULP_PHY_REG1_PSRAM_TX_CLK_DLY_MASK            (0x1F << 5)
#define PSRAM_ULP_PHY_REG1_PSRAM_TX_CLK_DLY_SHIFT           (5)
#define PSRAM_ULP_PHY_REG1_PSRAM_TX_DQS_DLY(n)              (((n) & 0x1F) << 10)
#define PSRAM_ULP_PHY_REG1_PSRAM_TX_DQS_DLY_MASK            (0x1F << 10)
#define PSRAM_ULP_PHY_REG1_PSRAM_TX_DQS_DLY_SHIFT           (10)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_DQS_DLY(n)              (((n) & 0x1F) << 15)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_DQS_DLY_MASK            (0x1F << 15)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_DQS_DLY_SHIFT           (15)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_CLK_DLY(n)              (((n) & 0x1F) << 20)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_CLK_DLY_MASK            (0x1F << 20)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_CLK_DLY_SHIFT           (20)

// reg_114
#define PSRAM_ULP_PHY_REG1_PSRAM_TX_CEB_DLY2(n)             (((n) & 0x1F) << 0)
#define PSRAM_ULP_PHY_REG1_PSRAM_TX_CEB_DLY2_MASK           (0x1F << 0)
#define PSRAM_ULP_PHY_REG1_PSRAM_TX_CEB_DLY2_SHIFT          (0)
#define PSRAM_ULP_PHY_REG1_PSRAM_TX_CLK_DLY2(n)             (((n) & 0x1F) << 5)
#define PSRAM_ULP_PHY_REG1_PSRAM_TX_CLK_DLY2_MASK           (0x1F << 5)
#define PSRAM_ULP_PHY_REG1_PSRAM_TX_CLK_DLY2_SHIFT          (5)
#define PSRAM_ULP_PHY_REG1_PSRAM_TX_DQS_DLY2(n)             (((n) & 0x1F) << 10)
#define PSRAM_ULP_PHY_REG1_PSRAM_TX_DQS_DLY2_MASK           (0x1F << 10)
#define PSRAM_ULP_PHY_REG1_PSRAM_TX_DQS_DLY2_SHIFT          (10)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_DQS_DLY2(n)             (((n) & 0x1F) << 15)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_DQS_DLY2_MASK           (0x1F << 15)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_DQS_DLY2_SHIFT          (15)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_CLK_DLY2(n)             (((n) & 0x1F) << 20)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_CLK_DLY2_MASK           (0x1F << 20)
#define PSRAM_ULP_PHY_REG1_PSRAM_RX_CLK_DLY2_SHIFT          (20)

#endif
