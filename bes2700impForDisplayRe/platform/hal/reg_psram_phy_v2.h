/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifndef __REG_PSRAM_PHY_V2_H__
#define __REG_PSRAM_PHY_V2_H__

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
    __IO uint32_t REG_064;
};

// reg_00
#define PSRAM_ULP_PHY_CHIP_TYPE                  (1 << 0)
#define PSRAM_ULP_PHY_CHIP_BIT                   (1 << 1)
#define PSRAM_ULP_PHY_MEMORY_WIDTH(n)            (((n) & 0x3) << 2)
#define PSRAM_ULP_PHY_MEMORY_WIDTH_MASK          (0x3 << 2)
#define PSRAM_ULP_PHY_MEMORY_WIDTH_SHIFT         (2)
#define PSRAM_ULP_PHY_FRE_RATIO(n)               (((n) & 0x3) << 4)
#define PSRAM_ULP_PHY_FRE_RATIO_MASK             (0x3 << 4)
#define PSRAM_ULP_PHY_FRE_RATIO_SHIFT            (4)

// reg_04
#define PSRAM_ULP_PHY_CTRL_DELAY(n)              (((n) & 0x3) << 0)
#define PSRAM_ULP_PHY_CTRL_DELAY_MASK            (0x3 << 0)
#define PSRAM_ULP_PHY_CTRL_DELAY_SHIFT           (0)
#define PSRAM_ULP_PHY_RX_DLY_EN                  (1 << 2)
#define PSRAM_ULP_PHY_ALIGN_BYPASS               (1 << 3)
#define PSRAM_ULP_PHY_PHY_LOOPBACK_EN            (1 << 4)
#define PSRAM_ULP_PHY_PHY_DUMMY_CYC_EN           (1 << 5)
#define PSRAM_ULP_PHY_PHY_DLY_AUTO_EN            (1 << 6)
#define PSRAM_ULP_PHY_SQPI_SAMPLE_SEL(n)         (((n) & 0xF) << 7)
#define PSRAM_ULP_PHY_SQPI_SAMPLE_SEL_MASK       (0xF << 7)
#define PSRAM_ULP_PHY_SQPI_SAMPLE_SEL_SHIFT      (7)
#define PSRAM_ULP_PHY_IDLE_DQ_OEN                (1 << 11)
#define PSRAM_ULP_PHY_PHY_TX_BYPASS              (1 << 12)
#define PSRAM_ULP_PHY_ADDR_4BYTES_EN             (1 << 13)
#define PSRAM_ULP_PHY_IDLE_DQS_OEN               (1 << 14)
#define PSRAM_ULP_PHY_DQS_DM_MERGE_EN            (1 << 15)
#define PSRAM_ULP_PHY_REG_WR_WO_DM               (1 << 16)

// reg_08
#define PSRAM_ULP_PHY_T_WPST(n)                  (((n) & 0x7) << 0)
#define PSRAM_ULP_PHY_T_WPST_MASK                (0x7 << 0)
#define PSRAM_ULP_PHY_T_WPST_SHIFT               (0)

// reg_00c
#define PSRAM_ULP_PHY_T_EXPANDRD(n)              (((n) & 0x3F) << 0)
#define PSRAM_ULP_PHY_T_EXPANDRD_MASK            (0x3F << 0)
#define PSRAM_ULP_PHY_T_EXPANDRD_SHIFT           (0)

// reg_00

// reg_10
#define PSRAM_ULP_PHY_CMD_CONFLICT_CLR           (1 << 0)

// reg_14
#define PSRAM_ULP_PHY_SQPI_CMD_CONFLICT_CLR      (1 << 0)

// reg_40
#define PSRAM_ULP_PHY_PHY_RX_BYPASS              (1 << 0)
#define PSRAM_ULP_PHY_PHY_SAMP_WITH_CLK          (1 << 1)
#define PSRAM_ULP_PHY_ANA_PHY_BYPASS             (1 << 2)
#define PSRAM_ULP_PHY_ANA_PHY_LP_CTRL_DR         (1 << 3)

// reg_44
#define PSRAM_ULP_PHY_CMD_CONFLICT_STS           (1 << 0)
#define PSRAM_ULP_PHY_PHY_FSM_STATE(n)           (((n) & 0xF) << 1)
#define PSRAM_ULP_PHY_PHY_FSM_STATE_MASK         (0xF << 1)
#define PSRAM_ULP_PHY_PHY_FSM_STATE_SHIFT        (1)
#define PSRAM_ULP_PHY_SQPI_CMD_CONFLICT_STS      (1 << 5)
#define PSRAM_ULP_PHY_SQPI_PHY_FSM_STATE(n)      (((n) & 0xF) << 6)
#define PSRAM_ULP_PHY_SQPI_PHY_FSM_STATE_MASK    (0xF << 6)
#define PSRAM_ULP_PHY_SQPI_PHY_FSM_STATE_SHIFT   (6)
#define PSRAM_ULP_PHY_ANA_PHY_BYPASS_STS         (1 << 10)

// reg_48
#define PSRAM_ULP_PHY_REG_LDO_PU                 (1 << 0)
#define PSRAM_ULP_PHY_REG_LDO_PRECHARGE          (1 << 1)
#define PSRAM_ULP_PHY_REG_LDO_IEN1(n)            (((n) & 0xF) << 2)
#define PSRAM_ULP_PHY_REG_LDO_IEN1_MASK          (0xF << 2)
#define PSRAM_ULP_PHY_REG_LDO_IEN1_SHIFT         (2)
#define PSRAM_ULP_PHY_REG_LDO_IEN2(n)            (((n) & 0xF) << 6)
#define PSRAM_ULP_PHY_REG_LDO_IEN2_MASK          (0xF << 6)
#define PSRAM_ULP_PHY_REG_LDO_IEN2_SHIFT         (6)
#define PSRAM_ULP_PHY_REG_LDO_VTUNE(n)           (((n) & 0x7) << 10)
#define PSRAM_ULP_PHY_REG_LDO_VTUNE_MASK         (0x7 << 10)
#define PSRAM_ULP_PHY_REG_LDO_VTUNE_SHIFT        (10)
#define PSRAM_ULP_PHY_REG_RESERVED(n)            (((n) & 0x7) << 13)
#define PSRAM_ULP_PHY_REG_RESERVED_MASK          (0x7 << 13)
#define PSRAM_ULP_PHY_REG_RESERVED_SHIFT         (13)
#define PSRAM_ULP_PHY_REG_LDO_IDLL(n)            (((n) & 0xF) << 16)
#define PSRAM_ULP_PHY_REG_LDO_IDLL_MASK          (0xF << 16)
#define PSRAM_ULP_PHY_REG_LDO_IDLL_SHIFT         (16)
#define PSRAM_ULP_PHY_REG_LDO_IPSRAM(n)          (((n) & 0xF) << 20)
#define PSRAM_ULP_PHY_REG_LDO_IPSRAM_MASK        (0xF << 20)
#define PSRAM_ULP_PHY_REG_LDO_IPSRAM_SHIFT       (20)
#define PSRAM_ULP_PHY_REG_LDO_IPTAT_RES(n)       (((n) & 0xF) << 24)
#define PSRAM_ULP_PHY_REG_LDO_IPTAT_RES_MASK     (0xF << 24)
#define PSRAM_ULP_PHY_REG_LDO_IPTAT_RES_SHIFT    (24)
#define PSRAM_ULP_PHY_REG_LDO_RES(n)             (((n) & 0xF) << 28)
#define PSRAM_ULP_PHY_REG_LDO_RES_MASK           (0xF << 28)
#define PSRAM_ULP_PHY_REG_LDO_RES_SHIFT          (28)

// reg_4c
#define PSRAM_ULP_PHY_REG_PSRAM_PU               (1 << 0)
#define PSRAM_ULP_PHY_REG_PSRAM_SWRC(n)          (((n) & 0x3) << 1)
#define PSRAM_ULP_PHY_REG_PSRAM_SWRC_MASK        (0x3 << 1)
#define PSRAM_ULP_PHY_REG_PSRAM_SWRC_SHIFT       (1)
#define PSRAM_ULP_PHY_REG_PSRAM_TXDRV(n)         (((n) & 0x7) << 3)
#define PSRAM_ULP_PHY_REG_PSRAM_TXDRV_MASK       (0x7 << 3)
#define PSRAM_ULP_PHY_REG_PSRAM_TXDRV_SHIFT      (3)
#define PSRAM_ULP_PHY_REG_PSRAM_LOOPBACK_EN      (1 << 6)

// reg_50
#define PSRAM_ULP_PHY_REG_DLL_PU                 (1 << 0)
#define PSRAM_ULP_PHY_REG_DLL_SWRC(n)            (((n) & 0x3) << 1)
#define PSRAM_ULP_PHY_REG_DLL_SWRC_MASK          (0x3 << 1)
#define PSRAM_ULP_PHY_REG_DLL_SWRC_SHIFT         (1)
#define PSRAM_ULP_PHY_REG_DLL_RANGE(n)           (((n) & 0x3) << 3)
#define PSRAM_ULP_PHY_REG_DLL_RANGE_MASK         (0x3 << 3)
#define PSRAM_ULP_PHY_REG_DLL_RANGE_SHIFT        (3)
#define PSRAM_ULP_PHY_REG_DLL_DLY_INI(n)         (((n) & 0xFF) << 5)
#define PSRAM_ULP_PHY_REG_DLL_DLY_INI_MASK       (0xFF << 5)
#define PSRAM_ULP_PHY_REG_DLL_DLY_INI_SHIFT      (5)
#define PSRAM_ULP_PHY_REG_DLL(n)                 (((n) & 0xFF) << 13)
#define PSRAM_ULP_PHY_REG_DLL_MASK               (0xFF << 13)
#define PSRAM_ULP_PHY_REG_DLL_SHIFT              (13)
#define PSRAM_ULP_PHY_REG_BYPASS_DECIMATION      (1 << 16)
#define PSRAM_ULP_PHY_REG_DLL_RESETB             (1 << 21)
#define PSRAM_ULP_PHY_REG_DLL_CK_RDY             (1 << 22)

// reg_54
#define PSRAM_ULP_PHY_REG_PSRAM_TX_CEB_DLY(n)    (((n) & 0x1F) << 0)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_CEB_DLY_MASK  (0x1F << 0)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_CEB_DLY_SHIFT (0)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DLY(n)    (((n) & 0x1F) << 5)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DLY_MASK  (0x1F << 5)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DLY_SHIFT (5)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY(n)    (((n) & 0x1F) << 10)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY_MASK  (0x1F << 10)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY_SHIFT (10)
#define PSRAM_ULP_PHY_REG_PSRAM_RX_DQS_DLY(n)    (((n) & 0x1F) << 15)
#define PSRAM_ULP_PHY_REG_PSRAM_RX_DQS_DLY_MASK  (0x1F << 15)
#define PSRAM_ULP_PHY_REG_PSRAM_RX_DQS_DLY_SHIFT (15)
#define PSRAM_ULP_PHY_REG_PSRAM_RX_CLK_DLY(n)    (((n) & 0x1F) << 20)
#define PSRAM_ULP_PHY_REG_PSRAM_RX_CLK_DLY_MASK  (0x1F << 20)
#define PSRAM_ULP_PHY_REG_PSRAM_RX_CLK_DLY_SHIFT (20)

// reg_58
#define PSRAM_ULP_PHY_DLL_DLY_IN(n)              (((n) & 0x3F) << 0)
#define PSRAM_ULP_PHY_DLL_DLY_IN_MASK            (0x3F << 0)
#define PSRAM_ULP_PHY_DLL_DLY_IN_SHIFT           (0)
#define PSRAM_ULP_PHY_DLL_LOCK                   (1 << 6)
#define PSRAM_ULP_PHY_DLL_ALL_ZERO               (1 << 7)
#define PSRAM_ULP_PHY_DLL_ALL_ONE                (1 << 8)

// reg_05c
#define PSRAM_ULP_PHY_REG_PSRAM1_TX_CEB_DLY(n)    (((n) & 0x1F) << 0)
#define PSRAM_ULP_PHY_REG_PSRAM1_TX_CEB_DLY_MASK  (0x1F << 0)
#define PSRAM_ULP_PHY_REG_PSRAM1_TX_CEB_DLY_SHIFT (0)
#define PSRAM_ULP_PHY_REG_PSRAM1_TX_CLK_DLY(n)    (((n) & 0x1F) << 5)
#define PSRAM_ULP_PHY_REG_PSRAM1_TX_CLK_DLY_MASK  (0x1F << 5)
#define PSRAM_ULP_PHY_REG_PSRAM1_TX_CLK_DLY_SHIFT (5)
#define PSRAM_ULP_PHY_REG_PSRAM1_TX_DQS_DLY(n)    (((n) & 0x1F) << 10)
#define PSRAM_ULP_PHY_REG_PSRAM1_TX_DQS_DLY_MASK  (0x1F << 10)
#define PSRAM_ULP_PHY_REG_PSRAM1_TX_DQS_DLY_SHIFT (10)
#define PSRAM_ULP_PHY_REG_PSRAM1_RX_DQS_DLY(n)    (((n) & 0x1F) << 15)
#define PSRAM_ULP_PHY_REG_PSRAM1_RX_DQS_DLY_MASK  (0x1F << 15)
#define PSRAM_ULP_PHY_REG_PSRAM1_RX_DQS_DLY_SHIFT (15)
#define PSRAM_ULP_PHY_REG_PSRAM1_RX_CLK_DLY(n)    (((n) & 0x1F) << 20)
#define PSRAM_ULP_PHY_REG_PSRAM1_RX_CLK_DLY_MASK  (0x1F << 20)
#define PSRAM_ULP_PHY_REG_PSRAM1_RX_CLK_DLY_SHIFT (20)

// reg_060
#define PSRAM_ULP_PHY_REG_T_TRANS(n)              (((n) & 0xFFFF) << 0)
#define PSRAM_ULP_PHY_REG_T_TRANS_MASK            (0xFFFF << 0)
#define PSRAM_ULP_PHY_REG_T_TRANS_SHIFT           (0)
#define PSRAM_ULP_PHY_REG_T_PRECHARGE(n)          (((n) & 0xFFFF) << 16)
#define PSRAM_ULP_PHY_REG_T_PRECHARGE_MASK        (0xFFFF << 16)
#define PSRAM_ULP_PHY_REG_T_PRECHARGE_SHIFT       (16)

// reg_064
#define PSRAM_ULP_PHY_REG_T_PU(n)                 (((n) & 0xFFFF) << 0)
#define PSRAM_ULP_PHY_REG_T_PU_MASK               (0xFFFF << 0)
#define PSRAM_ULP_PHY_REG_T_PU_SHIFT              (0)
#define PSRAM_ULP_PHY_REG_T_LOCK(n)               (((n) & 0xFFFF) << 16)
#define PSRAM_ULP_PHY_REG_T_LOCK_MASK             (0xFFFF << 16)
#define PSRAM_ULP_PHY_REG_T_LOCK_SHIFT            (16)

#endif
