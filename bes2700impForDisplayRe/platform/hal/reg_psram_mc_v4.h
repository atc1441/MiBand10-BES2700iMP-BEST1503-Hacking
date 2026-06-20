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
#ifndef __REG_PSRAM_MC_V4_H__
#define __REG_PSRAM_MC_V4_H__

#include "plat_types.h"

struct PSRAM_MC_T {
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
    __IO uint32_t REG_068;
    __IO uint32_t REG_06C;
    __IO uint32_t REG_070;
    __IO uint32_t REG_074;
    __IO uint32_t REG_078;
    __IO uint32_t REG_07C;
    __IO uint32_t REG_080;
    __IO uint32_t REG_084;
    __IO uint32_t REG_088;
    __IO uint32_t REG_08C;
    __IO uint32_t REG_090;
    __IO uint32_t REG_094;
    __IO uint32_t REG_098;
    __IO uint32_t REG_09C;
    __IO uint32_t REG_0A0;
    __IO uint32_t REG_0A4;
    __IO uint32_t REG_0A8;
    __IO uint32_t REG_0AC;
    __IO uint32_t REG_0B0;
    __IO uint32_t REG_0B4;
    __IO uint32_t REG_0B8;
    __IO uint32_t REG_0BC;
    __IO uint32_t REG_0C0;
    __IO uint32_t REG_0C4;
    __IO uint32_t REG_RESERVED_0C0[0x1E];
    __IO uint32_t REG_140;
    __IO uint32_t REG_144;
    __IO uint32_t REG_148;
    __IO uint32_t REG_14C;
    __IO uint32_t REG_150;
    __IO uint32_t REG_154;
    __IO uint32_t REG_158;
    __IO uint32_t REG_15C;
    __IO uint32_t REG_160;
    __IO uint32_t REG_RESERVED_164[7];
    __IO uint32_t REG_180;
    __IO uint32_t REG_184;
    __IO uint32_t REG_188;
    __IO uint32_t REG_18C;
    __IO uint32_t REG_190;
    __IO uint32_t REG_194;
    __IO uint32_t REG_198;
    __IO uint32_t REG_19C;
    __IO uint32_t REG_RESERVED_198[0x18];
    __IO uint32_t REG_200;
    __IO uint32_t REG_RESERVED_204[0x7F];
    __IO uint32_t REG_400;
    __IO uint32_t REG_404;
    __IO uint32_t REG_RESERVED_408[0xE];
    __IO uint32_t REG_440;
    __IO uint32_t REG_444;
    __IO uint32_t REG_448;
    __IO uint32_t REG_44C;
    __IO uint32_t REG_450;
    __IO uint32_t REG_454;
    __IO uint32_t REG_458;
    __IO uint32_t REG_45C;
    __IO uint32_t REG_460;
    __IO uint32_t REG_464;
    __IO uint32_t REG_468;
    __IO uint32_t REG_46C;
    __IO uint32_t REG_470;
    __IO uint32_t REG_474;
    __IO uint32_t REG_478;
    __IO uint32_t REG_47C;
    __IO uint32_t REG_480;
    __IO uint32_t REG_484;
    __IO uint32_t REG_488;
    __IO uint32_t REG_48C;
    __IO uint32_t REG_490;
    __IO uint32_t REG_494;
    __IO uint32_t REG_498;
    __IO uint32_t REG_49C;
};

// reg_000
#define PSRAM_ULP_MC_CHIP_BIT                               (1 << 0)
#define PSRAM_ULP_MC_CHIP_TYPE                              (1 << 1)
#define PSRAM_ULP_MC_CHIP_X16                               (1 << 2)
#define PSRAM_ULP_MC_CHIP_CA_PATTERN(n)                     (((n) & 0x7) << 3)
#define PSRAM_ULP_MC_CHIP_CA_PATTERN_MASK                   (0x7 << 3)
#define PSRAM_ULP_MC_CHIP_CA_PATTERN_SHIFT                  (3)
#define PSRAM_ULP_MC_CHIP_SWITCH                            (1 << 6)
#define PSRAM_ULP_MC_SQPI_MODE                              (1 << 7)
#define PSRAM_ULP_MC_QPI_MODE                               (1 << 8)
#define PSRAM_ULP_MC_OPI_MODE                               (1 << 9)
#define PSRAM_ULP_MC_DTR_MODE(n)                            (((n) & 0x7) << 10)
#define PSRAM_ULP_MC_DTR_MODE_MASK                          (0x7 << 10)
#define PSRAM_ULP_MC_DTR_MODE_SHIFT                         (10)
#define PSRAM_ULP_MC_DQS_MODE                               (1 << 13)
#define PSRAM_ULP_MC_LATENCY_TYPE                           (1 << 14)
#define PSRAM_ULP_MC_SQPI_CMD_SEQ_DEFINE_EN                 (1 << 15)
#define PSRAM_ULP_MC_HYPERBUS                               (1 << 16)

// reg_004
#define PSRAM_ULP_MC_MGR_CMD(n)                             (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_MGR_CMD_MASK                           (0xFF << 0)
#define PSRAM_ULP_MC_MGR_CMD_SHIFT                          (0)

// reg_008
#define PSRAM_ULP_MC_MGR_ADDR(n)                            (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_MGR_ADDR_MASK                          (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_MGR_ADDR_SHIFT                         (0)

// reg_00c
#define PSRAM_ULP_MC_MGR_LEN(n)                             (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_MGR_LEN_MASK                           (0xFF << 0)
#define PSRAM_ULP_MC_MGR_LEN_SHIFT                          (0)
#define PSRAM_ULP_MC_MGR_LEN_BYPASS                         (1 << 8)

// reg_010
#define PSRAM_ULP_MC_MGR_WSTRB(n)                           (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_MGR_WSTRB_MASK                         (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_MGR_WSTRB_SHIFT                        (0)

// reg_014
#define PSRAM_ULP_MC_MGR_TX_FIFO(n)                         (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_MGR_TX_FIFO_MASK                       (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_MGR_TX_FIFO_SHIFT                      (0)

// reg_018
#define PSRAM_ULP_MC_MGR_RX_FIFO(n)                         (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_MGR_RX_FIFO_MASK                       (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_MGR_RX_FIFO_SHIFT                      (0)

// reg_01c
#define PSRAM_ULP_MC_MGR_TX_FIFO_CLR                        (1 << 0)
#define PSRAM_ULP_MC_MGR_RX_FIFO_CLR                        (1 << 1)

// reg_020
#define PSRAM_ULP_MC_REFRESH_MODE                           (1 << 0)
#define PSRAM_ULP_MC_BURST_REFRESH_EN                       (1 << 1)

// reg_024
#define PSRAM_ULP_MC_ENTRY_SLEEP_IDLE                       (1 << 0)
#define PSRAM_ULP_MC_ENTRY_SELF_REFRESH_IDLE                (1 << 1)
#define PSRAM_ULP_MC_STOP_CLK_IDLE                          (1 << 2)
#define PSRAM_ULP_MC_AUTOWAKEUP_EN                          (1 << 3)
#define PSRAM_ULP_MC_RES_7_4_REG24(n)                       (((n) & 0xF) << 4)
#define PSRAM_ULP_MC_RES_7_4_REG24_MASK                     (0xF << 4)
#define PSRAM_ULP_MC_RES_7_4_REG24_SHIFT                    (4)
#define PSRAM_ULP_MC_PD_MR(n)                               (((n) & 0xFF) << 8)
#define PSRAM_ULP_MC_PD_MR_MASK                             (0xFF << 8)
#define PSRAM_ULP_MC_PD_MR_SHIFT                            (8)
#define PSRAM_ULP_MC_PD_CMD(n)                              (((n) & 0xFF) << 16)
#define PSRAM_ULP_MC_PD_CMD_MASK                            (0xFF << 16)
#define PSRAM_ULP_MC_PD_CMD_SHIFT                           (16)
#define PSRAM_ULP_MC_SELF_REFRESH_LP_EN                     (1 << 24)

// reg_028
#define PSRAM_ULP_MC_WRITE_LATENCY_0(n)                     (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_WRITE_LATENCY_0_MASK                   (0xFF << 0)
#define PSRAM_ULP_MC_WRITE_LATENCY_0_SHIFT                  (0)

// reg_02c
#define PSRAM_ULP_MC_READ_LATENCY_0(n)                      (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_READ_LATENCY_0_MASK                    (0xFF << 0)
#define PSRAM_ULP_MC_READ_LATENCY_0_SHIFT                   (0)

// reg_030
#define PSRAM_ULP_MC_MEMORY_WIDTH(n)                        (((n) & 0x3) << 0)
#define PSRAM_ULP_MC_MEMORY_WIDTH_MASK                      (0x3 << 0)
#define PSRAM_ULP_MC_MEMORY_WIDTH_SHIFT                     (0)
#define PSRAM_ULP_MC_MEM_SIZE(n)                            (((n) & 0x7) << 2)
#define PSRAM_ULP_MC_MEM_SIZE_MASK                          (0x7 << 2)
#define PSRAM_ULP_MC_MEM_SIZE_SHIFT                         (2)
#define PSRAM_ULP_MC_DUAL_ENTRY_MODE                        (1 << 5)
#define PSRAM_ULP_MC_DIS_OUT_OF_ORDER                       (1 << 6)
#define PSRAM_ULP_MC_PREFETCH_16N                           (1 << 7)

// reg_034
#define PSRAM_ULP_MC_BURST_LENGTH_0(n)                      (((n) & 0x7) << 0)
#define PSRAM_ULP_MC_BURST_LENGTH_0_MASK                    (0x7 << 0)
#define PSRAM_ULP_MC_BURST_LENGTH_0_SHIFT                   (0)
#define PSRAM_ULP_MC_AUTO_SWITCH_RD_LEN                     (1 << 3)
#define PSRAM_ULP_MC_PAGE_BOUNDARY(n)                       (((n) & 0x3) << 4)
#define PSRAM_ULP_MC_PAGE_BOUNDARY_MASK                     (0x3 << 4)
#define PSRAM_ULP_MC_PAGE_BOUNDARY_SHIFT                    (4)
#define PSRAM_ULP_MC_CMD_ENTER_QUAD(n)                      (((n) & 0xFF) << 6)
#define PSRAM_ULP_MC_CMD_ENTER_QUAD_MASK                    (0xFF << 6)
#define PSRAM_ULP_MC_CMD_ENTER_QUAD_SHIFT                   (6)
#define PSRAM_ULP_MC_CMD_EXIT_QUAD(n)                       (((n) & 0xFF) << 14)
#define PSRAM_ULP_MC_CMD_EXIT_QUAD_MASK                     (0xFF << 14)
#define PSRAM_ULP_MC_CMD_EXIT_QUAD_SHIFT                    (14)

// reg_038
#define PSRAM_ULP_MC_BUS_WIDTH                              (1 << 0)

// reg_03c
#define PSRAM_ULP_MC_HIGH_PRI_LEVEL(n)                      (((n) & 0x1F) << 0)
#define PSRAM_ULP_MC_HIGH_PRI_LEVEL_MASK                    (0x1F << 0)
#define PSRAM_ULP_MC_HIGH_PRI_LEVEL_SHIFT                   (0)

// reg_040
#define PSRAM_ULP_MC_CP_WRAP_EN_0                           (1 << 0)
#define PSRAM_ULP_MC_AUTO_PRECHARGE                         (1 << 1)
#define PSRAM_ULP_MC_WRAP_CRT_RET_EN                        (1 << 2)
#define PSRAM_ULP_MC_CROSS_BOUNDARY_EN                      (1 << 3)
#define PSRAM_ULP_MC_TOGGLE_BURST_LEN_EN                    (1 << 4)
#define PSRAM_ULP_MC_TOGGLE_BURST_LEN_SET(n)                (((n) & 0x7) << 5)
#define PSRAM_ULP_MC_TOGGLE_BURST_LEN_SET_MASK              (0x7 << 5)
#define PSRAM_ULP_MC_TOGGLE_BURST_LEN_SET_SHIFT             (5)
#define PSRAM_ULP_MC_TOGGLE_BURST_LEN_CMD(n)                (((n) & 0xFF) << 8)
#define PSRAM_ULP_MC_TOGGLE_BURST_LEN_CMD_MASK              (0xFF << 8)
#define PSRAM_ULP_MC_TOGGLE_BURST_LEN_CMD_SHIFT             (8)
#define PSRAM_ULP_MC_TOGGLE_WRAP_EN                         (1 << 16)
#define PSRAM_ULP_MC_TOGGLE_WRAP_SET                        (1 << 17)

// reg_044
#define PSRAM_ULP_MC_WB_DRAIN                               (1 << 0)
#define PSRAM_ULP_MC_WB_INVALID                             (1 << 1)
#define PSRAM_ULP_MC_RB_INVALID                             (1 << 2)
#define PSRAM_ULP_MC_SNP_DISABLE                            (1 << 3)
#define PSRAM_ULP_MC_BUFFERABLE_WB_EN                       (1 << 4)
#define PSRAM_ULP_MC_RD_CANCEL_EN                           (1 << 5)

// reg_048
#define PSRAM_ULP_MC_FRE_RATIO(n)                           (((n) & 0x3) << 0)
#define PSRAM_ULP_MC_FRE_RATIO_MASK                         (0x3 << 0)
#define PSRAM_ULP_MC_FRE_RATIO_SHIFT                        (0)

// reg_04c
#define PSRAM_ULP_MC_T_REFI_0(n)                            (((n) & 0xFFFF) << 0)
#define PSRAM_ULP_MC_T_REFI_0_MASK                          (0xFFFF << 0)
#define PSRAM_ULP_MC_T_REFI_0_SHIFT                         (0)
#define PSRAM_ULP_MC_NUM_OF_BURST_RFS(n)                    (((n) & 0xFFFF) << 16)
#define PSRAM_ULP_MC_NUM_OF_BURST_RFS_MASK                  (0xFFFF << 16)
#define PSRAM_ULP_MC_NUM_OF_BURST_RFS_SHIFT                 (16)

// reg_050
#define PSRAM_ULP_MC_T_RC_0(n)                              (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_T_RC_0_MASK                            (0xFF << 0)
#define PSRAM_ULP_MC_T_RC_0_SHIFT                           (0)

// reg_054
#define PSRAM_ULP_MC_T_RFC_0(n)                             (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_T_RFC_0_MASK                           (0xFF << 0)
#define PSRAM_ULP_MC_T_RFC_0_SHIFT                          (0)

// reg_058
#define PSRAM_ULP_MC_T_CPHR_0(n)                            (((n) & 0x3F) << 0)
#define PSRAM_ULP_MC_T_CPHR_0_MASK                          (0x3F << 0)
#define PSRAM_ULP_MC_T_CPHR_0_SHIFT                         (0)

// reg_05c
#define PSRAM_ULP_MC_T_CPHR_AP_0(n)                         (((n) & 0x3F) << 0)
#define PSRAM_ULP_MC_T_CPHR_AP_0_MASK                       (0x3F << 0)
#define PSRAM_ULP_MC_T_CPHR_AP_0_SHIFT                      (0)

// reg_060
#define PSRAM_ULP_MC_T_CPHW_0(n)                            (((n) & 0x3F) << 0)
#define PSRAM_ULP_MC_T_CPHW_0_MASK                          (0x3F << 0)
#define PSRAM_ULP_MC_T_CPHW_0_SHIFT                         (0)

// reg_064
#define PSRAM_ULP_MC_T_CPHW_AP_0(n)                         (((n) & 0x3F) << 0)
#define PSRAM_ULP_MC_T_CPHW_AP_0_MASK                       (0x3F << 0)
#define PSRAM_ULP_MC_T_CPHW_AP_0_SHIFT                      (0)

// reg_068
#define PSRAM_ULP_MC_T_MRR_0(n)                             (((n) & 0x3F) << 0)
#define PSRAM_ULP_MC_T_MRR_0_MASK                           (0x3F << 0)
#define PSRAM_ULP_MC_T_MRR_0_SHIFT                          (0)

// reg_06c
#define PSRAM_ULP_MC_T_MRS_0(n)                             (((n) & 0x3F) << 0)
#define PSRAM_ULP_MC_T_MRS_0_MASK                           (0x3F << 0)
#define PSRAM_ULP_MC_T_MRS_0_SHIFT                          (0)

// reg_070
#define PSRAM_ULP_MC_T_CEM_0(n)                             (((n) & 0xFFFF) << 0)
#define PSRAM_ULP_MC_T_CEM_0_MASK                           (0xFFFF << 0)
#define PSRAM_ULP_MC_T_CEM_0_SHIFT                          (0)

// reg_074
#define PSRAM_ULP_MC_T_RST_0(n)                             (((n) & 0xFFFF) << 0)
#define PSRAM_ULP_MC_T_RST_0_MASK                           (0xFFFF << 0)
#define PSRAM_ULP_MC_T_RST_0_SHIFT                          (0)

// reg_078
#define PSRAM_ULP_MC_T_SRF_0(n)                             (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_T_SRF_0_MASK                           (0xFF << 0)
#define PSRAM_ULP_MC_T_SRF_0_SHIFT                          (0)

// reg_07c
#define PSRAM_ULP_MC_T_XSR_0(n)                             (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_T_XSR_0_MASK                           (0xFF << 0)
#define PSRAM_ULP_MC_T_XSR_0_SHIFT                          (0)

// reg_080
#define PSRAM_ULP_MC_T_HS_0(n)                              (((n) & 0xFFFF) << 0)
#define PSRAM_ULP_MC_T_HS_0_MASK                            (0xFFFF << 0)
#define PSRAM_ULP_MC_T_HS_0_SHIFT                           (0)

// reg_084
#define PSRAM_ULP_MC_T_XPHS_0(n)                            (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_T_XPHS_0_MASK                          (0xFF << 0)
#define PSRAM_ULP_MC_T_XPHS_0_SHIFT                         (0)

// reg_088
#define PSRAM_ULP_MC_T_XHS_0(n)                             (((n) & 0xFFFFF) << 0)
#define PSRAM_ULP_MC_T_XHS_0_MASK                           (0xFFFFF << 0)
#define PSRAM_ULP_MC_T_XHS_0_SHIFT                          (0)

// reg_08c
#define PSRAM_ULP_MC_T_ZQCAL_0(n)                           (((n) & 0xFFFFF) << 0)
#define PSRAM_ULP_MC_T_ZQCAL_0_MASK                         (0xFFFFF << 0)
#define PSRAM_ULP_MC_T_ZQCAL_0_SHIFT                        (0)

// reg_090
#define PSRAM_ULP_MC_T_ZQCRST_0(n)                          (((n) & 0xFFFFF) << 0)
#define PSRAM_ULP_MC_T_ZQCRST_0_MASK                        (0xFFFFF << 0)
#define PSRAM_ULP_MC_T_ZQCRST_0_SHIFT                       (0)

// reg_094
#define PSRAM_ULP_MC_T_XCKD_0(n)                            (((n) & 0x3F) << 0)
#define PSRAM_ULP_MC_T_XCKD_0_MASK                          (0x3F << 0)
#define PSRAM_ULP_MC_T_XCKD_0_SHIFT                         (0)

// reg_098
#define PSRAM_ULP_MC_T_ECKD_0(n)                            (((n) & 0x3F) << 0)
#define PSRAM_ULP_MC_T_ECKD_0_MASK                          (0x3F << 0)
#define PSRAM_ULP_MC_T_ECKD_0_SHIFT                         (0)

// reg_09c
#define PSRAM_ULP_MC_WR_DMY_CYC(n)                          (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_WR_DMY_CYC_MASK                        (0xFF << 0)
#define PSRAM_ULP_MC_WR_DMY_CYC_SHIFT                       (0)
#define PSRAM_ULP_MC_CMD_DMY_CYC(n)                         (((n) & 0xFF) << 8)
#define PSRAM_ULP_MC_CMD_DMY_CYC_MASK                       (0xFF << 8)
#define PSRAM_ULP_MC_CMD_DMY_CYC_SHIFT                      (8)
#define PSRAM_ULP_MC_STOP_CLK_IN_NOP                        (1 << 0)

// reg_0a0
#define PSRAM_ULP_MC_NOP_DMY_CYC(n)                         (((n) & 0xFF) << 1)
#define PSRAM_ULP_MC_NOP_DMY_CYC_MASK                       (0xFF << 1)
#define PSRAM_ULP_MC_NOP_DMY_CYC_SHIFT                      (1)
#define PSRAM_ULP_MC_STOP_CLK_IN_TCPH                       (1 << 9)

// reg_0a4
#define PSRAM_ULP_MC_QUEUE_IDLE_CYCLE(n)                    (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_QUEUE_IDLE_CYCLE_MASK                  (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_QUEUE_IDLE_CYCLE_SHIFT                 (0)

// reg_0a8
#define PSRAM_ULP_MC_T_EXPANDRD(n)                          (((n) & 0x3F) << 0)
#define PSRAM_ULP_MC_T_EXPANDRD_MASK                        (0x3F << 0)
#define PSRAM_ULP_MC_T_EXPANDRD_SHIFT                       (0)

// reg_0ac
#define PSRAM_ULP_MC_RX_SYNC_BYPASS                         (1 << 0)

// reg_0b0
#define PSRAM_ULP_MC_MGR_FIFO_TEST_EN                       (1 << 0)

// reg_0b4
#define PSRAM_ULP_MC_T_ZQCAS_0(n)                           (((n) & 0xFFFFF) << 0)
#define PSRAM_ULP_MC_T_ZQCAS_0_MASK                         (0xFFFFF << 0)
#define PSRAM_ULP_MC_T_ZQCAS_0_SHIFT                        (0)

// reg_0b8
#define PSRAM_ULP_MC_T_NEW_HOLD(n)                          (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_T_NEW_HOLD_MASK                        (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_T_NEW_HOLD_SHIFT                       (0)

// reg_0bc
#define PSRAM_ULP_MC_NEW_CMD_OP(n)                          (((n) & 0x7) << 0)
#define PSRAM_ULP_MC_NEW_CMD_OP_MASK                        (0x7 << 0)
#define PSRAM_ULP_MC_NEW_CMD_OP_SHIFT                       (0)

// reg_0c0
#define PSRAM_ULP_MC_T_REF_WIN_0(n)                         (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_T_REF_WIN_0_MASK                       (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_T_REF_WIN_0_SHIFT                      (0)

// reg_0c4
#define PSRAM_ULP_MC_T_CSCKE(n)                             (((n) & 0xFFFF) << 0)
#define PSRAM_ULP_MC_T_CSCKE_MASK                           (0xFFFF << 0)
#define PSRAM_ULP_MC_T_CSCKE_SHIFT                          (0)
#define PSRAM_ULP_MC_T_CSCKEH(n)                            (((n) & 0xFFFF) << 16)
#define PSRAM_ULP_MC_T_CSCKEH_MASK                          (0xFFFF << 16)
#define PSRAM_ULP_MC_T_CSCKEH_SHIFT                         (16)

// reg_0c8
#define PSRAM_ULP_MC_CFG_RD_CONTI_RANGE(n)                  (((n) & 0x7) << 0)
#define PSRAM_ULP_MC_CFG_RD_CONTI_RANGE_MASK                (0x7 << 0)
#define PSRAM_ULP_MC_CFG_RD_CONTI_RANGE_SHIFT               (0)
#define PSRAM_ULP_MC_CFG_RD_NSEQ_NUM(n)                     (((n) & 0xF) << 3)
#define PSRAM_ULP_MC_CFG_RD_NSEQ_NUM_MASK                   (0xF << 3)
#define PSRAM_ULP_MC_CFG_RD_NSEQ_NUM_SHIFT                  (3)
#define PSRAM_ULP_MC_CFG_RD_SEQ_NUM(n)                      (((n) & 0xF) << 7)
#define PSRAM_ULP_MC_CFG_RD_SEQ_NUM_MASK                    (0xF << 7)
#define PSRAM_ULP_MC_CFG_RD_SEQ_NUM_SHIFT                   (7)
#define PSRAM_ULP_MC_CFG_RD_NACC_NUM(n)                     (((n) & 0xFF) << 11)
#define PSRAM_ULP_MC_CFG_RD_NACC_NUM_MASK                   (0xFF << 11)
#define PSRAM_ULP_MC_CFG_RD_NACC_NUM_SHIFT                  (11)
#define PSRAM_ULP_MC_CFG_RD_ACC_NUM(n)                      (((n) & 0xFF) << 19)
#define PSRAM_ULP_MC_CFG_RD_ACC_NUM_MASK                    (0xFF << 19)
#define PSRAM_ULP_MC_CFG_RD_ACC_NUM_SHIFT                   (19)
#define PSRAM_ULP_MC_CFG_RD_CASE_SEL(n)                     (((n) & 0x3) << 27)
#define PSRAM_ULP_MC_CFG_RD_CASE_SEL_MASK                   (0x3 << 27)
#define PSRAM_ULP_MC_CFG_RD_CASE_SEL_SHIFT                  (27)

// reg_0cc
#define PSRAM_ULP_MC_CFG_RD_TRANS_NUM(n)                    (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_CFG_RD_TRANS_NUM_MASK                  (0xFF << 0)
#define PSRAM_ULP_MC_CFG_RD_TRANS_NUM_SHIFT                 (0)
#define PSRAM_ULP_MC_CFG_RD_DIFF_NUM(n)                     (((n) & 0xFF) << 8)
#define PSRAM_ULP_MC_CFG_RD_DIFF_NUM_MASK                   (0xFF << 8)
#define PSRAM_ULP_MC_CFG_RD_DIFF_NUM_SHIFT                  (8)
#define PSRAM_ULP_MC_CFG_RD_CASE_MASK(n)                    (((n) & 0x7) << 16)
#define PSRAM_ULP_MC_CFG_RD_CASE_MASK_MASK                  (0x7 << 16)
#define PSRAM_ULP_MC_CFG_RD_CASE_MASK_SHIFT                 (16)

// reg_0d0
#define PSRAM_ULP_MC_DYN_SWITCH_EN                          (1 << 0)
#define PSRAM_ULP_MC_DYN_SWITCH_MR_0(n)                     (((n) & 0xFF) << 1)
#define PSRAM_ULP_MC_DYN_SWITCH_MR_0_MASK                   (0xFF << 1)
#define PSRAM_ULP_MC_DYN_SWITCH_MR_0_SHIFT                  (1)
#define PSRAM_ULP_MC_DYN_SWITCH_MD_0(n)                     (((n) & 0xFF) << 9)
#define PSRAM_ULP_MC_DYN_SWITCH_MD_0_MASK                   (0xFF << 9)
#define PSRAM_ULP_MC_DYN_SWITCH_MD_0_SHIFT                  (9)

// reg_0d4
#define PSRAM_ULP_MC_DYN_SWITCH_MR_1(n)                     (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_DYN_SWITCH_MR_1_MASK                   (0xFF << 0)
#define PSRAM_ULP_MC_DYN_SWITCH_MR_1_SHIFT                  (0)
#define PSRAM_ULP_MC_DYN_SWITCH_MD_1(n)                     (((n) & 0xFF) << 8)
#define PSRAM_ULP_MC_DYN_SWITCH_MD_1_MASK                   (0xFF << 8)
#define PSRAM_ULP_MC_DYN_SWITCH_MD_1_SHIFT                  (8)

// reg_140
#define PSRAM_ULP_MC_CMD_TABLE_ARRAY_RD(n)                  (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_CMD_TABLE_ARRAY_RD_MASK                (0xFF << 0)
#define PSRAM_ULP_MC_CMD_TABLE_ARRAY_RD_SHIFT               (0)

// reg_144
#define PSRAM_ULP_MC_CMD_TABLE_ARRAY_WR(n)                  (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_CMD_TABLE_ARRAY_WR_MASK                (0xFF << 0)
#define PSRAM_ULP_MC_CMD_TABLE_ARRAY_WR_SHIFT               (0)

// reg_148
#define PSRAM_ULP_MC_CMD_TABLE_REG_RD(n)                    (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_CMD_TABLE_REG_RD_MASK                  (0xFF << 0)
#define PSRAM_ULP_MC_CMD_TABLE_REG_RD_SHIFT                 (0)

// reg_14c
#define PSRAM_ULP_MC_CMD_TABLE_REG_WR(n)                    (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_CMD_TABLE_REG_WR_MASK                  (0xFF << 0)
#define PSRAM_ULP_MC_CMD_TABLE_REG_WR_SHIFT                 (0)

// reg_150
#define PSRAM_ULP_MC_CMD_TABLE_AUTO_REFR(n)                 (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_CMD_TABLE_AUTO_REFR_MASK               (0xFF << 0)
#define PSRAM_ULP_MC_CMD_TABLE_AUTO_REFR_SHIFT              (0)

// reg_154
#define PSRAM_ULP_MC_CMD_TABLE_SELF_REFR(n)                 (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_CMD_TABLE_SELF_REFR_MASK               (0xFF << 0)
#define PSRAM_ULP_MC_CMD_TABLE_SELF_REFR_SHIFT              (0)

// reg_158
#define PSRAM_ULP_MC_CMD_TABLE_HSLP_ENTRY(n)                (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_CMD_TABLE_HSLP_ENTRY_MASK              (0xFF << 0)
#define PSRAM_ULP_MC_CMD_TABLE_HSLP_ENTRY_SHIFT             (0)

// reg_15c
#define PSRAM_ULP_MC_CMD_TABLE_GLBRST(n)                    (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_CMD_TABLE_GLBRST_MASK                  (0xFF << 0)
#define PSRAM_ULP_MC_CMD_TABLE_GLBRST_SHIFT                 (0)

// reg_160
#define PSRAM_ULP_MC_CMD_TABLE_NOP(n)                       (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_CMD_TABLE_NOP_MASK                     (0xFF << 0)
#define PSRAM_ULP_MC_CMD_TABLE_NOP_SHIFT                    (0)

// reg_164
#define PSRAM_ULP_MC_CMD_TABLE_ORDER(n)                     (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_CMD_TABLE_ORDER_MASK                   (0xFF << 0)
#define PSRAM_ULP_MC_CMD_TABLE_ORDER_SHIFT                  (0)

// reg_168
#define PSRAM_ULP_MC_CMD_SEQ_DEFINE0(n)                     (((n) & 0x3FFFFFFF) << 0)
#define PSRAM_ULP_MC_CMD_SEQ_DEFINE0_MASK                   (0x3FFFFFFF << 0)
#define PSRAM_ULP_MC_CMD_SEQ_DEFINE0_SHIFT                  (0)

// reg_16c
#define PSRAM_ULP_MC_CMD_SEQ_DEFINE1(n)                     (((n) & 0x3FF) << 0)
#define PSRAM_ULP_MC_CMD_SEQ_DEFINE1_MASK                   (0x3FF << 0)
#define PSRAM_ULP_MC_CMD_SEQ_DEFINE1_SHIFT                  (0)

// reg_180
#define PSRAM_ULP_MC_CA_MAP_BIT0(n)                         (((n) & 0x1F) << 0)
#define PSRAM_ULP_MC_CA_MAP_BIT0_MASK                       (0x1F << 0)
#define PSRAM_ULP_MC_CA_MAP_BIT0_SHIFT                      (0)
#define PSRAM_ULP_MC_CA_MAP_BIT1(n)                         (((n) & 0x1F) << 5)
#define PSRAM_ULP_MC_CA_MAP_BIT1_MASK                       (0x1F << 5)
#define PSRAM_ULP_MC_CA_MAP_BIT1_SHIFT                      (5)
#define PSRAM_ULP_MC_CA_MAP_BIT2(n)                         (((n) & 0x1F) << 10)
#define PSRAM_ULP_MC_CA_MAP_BIT2_MASK                       (0x1F << 10)
#define PSRAM_ULP_MC_CA_MAP_BIT2_SHIFT                      (10)
#define PSRAM_ULP_MC_CA_MAP_BIT3(n)                         (((n) & 0x1F) << 15)
#define PSRAM_ULP_MC_CA_MAP_BIT3_MASK                       (0x1F << 15)
#define PSRAM_ULP_MC_CA_MAP_BIT3_SHIFT                      (15)
#define PSRAM_ULP_MC_CA_MAP_BIT4(n)                         (((n) & 0x1F) << 20)
#define PSRAM_ULP_MC_CA_MAP_BIT4_MASK                       (0x1F << 20)
#define PSRAM_ULP_MC_CA_MAP_BIT4_SHIFT                      (20)
#define PSRAM_ULP_MC_CA_MAP_BIT5(n)                         (((n) & 0x1F) << 25)
#define PSRAM_ULP_MC_CA_MAP_BIT5_MASK                       (0x1F << 25)
#define PSRAM_ULP_MC_CA_MAP_BIT5_SHIFT                      (25)

// reg_184
#define PSRAM_ULP_MC_CA_MAP_BIT6(n)                         (((n) & 0x1F) << 0)
#define PSRAM_ULP_MC_CA_MAP_BIT6_MASK                       (0x1F << 0)
#define PSRAM_ULP_MC_CA_MAP_BIT6_SHIFT                      (0)
#define PSRAM_ULP_MC_CA_MAP_BIT7(n)                         (((n) & 0x1F) << 5)
#define PSRAM_ULP_MC_CA_MAP_BIT7_MASK                       (0x1F << 5)
#define PSRAM_ULP_MC_CA_MAP_BIT7_SHIFT                      (5)
#define PSRAM_ULP_MC_CA_MAP_BIT8(n)                         (((n) & 0x1F) << 10)
#define PSRAM_ULP_MC_CA_MAP_BIT8_MASK                       (0x1F << 10)
#define PSRAM_ULP_MC_CA_MAP_BIT8_SHIFT                      (10)
#define PSRAM_ULP_MC_CA_MAP_BIT9(n)                         (((n) & 0x1F) << 15)
#define PSRAM_ULP_MC_CA_MAP_BIT9_MASK                       (0x1F << 15)
#define PSRAM_ULP_MC_CA_MAP_BIT9_SHIFT                      (15)
#define PSRAM_ULP_MC_CA_MAP_BIT10(n)                        (((n) & 0x1F) << 20)
#define PSRAM_ULP_MC_CA_MAP_BIT10_MASK                      (0x1F << 20)
#define PSRAM_ULP_MC_CA_MAP_BIT10_SHIFT                     (20)
#define PSRAM_ULP_MC_CA_MAP_BIT11(n)                        (((n) & 0x1F) << 25)
#define PSRAM_ULP_MC_CA_MAP_BIT11_MASK                      (0x1F << 25)
#define PSRAM_ULP_MC_CA_MAP_BIT11_SHIFT                     (25)

// reg_188
#define PSRAM_ULP_MC_CA_MAP_BIT12(n)                        (((n) & 0x1F) << 0)
#define PSRAM_ULP_MC_CA_MAP_BIT12_MASK                      (0x1F << 0)
#define PSRAM_ULP_MC_CA_MAP_BIT12_SHIFT                     (0)
#define PSRAM_ULP_MC_CA_MAP_BIT13(n)                        (((n) & 0x1F) << 5)
#define PSRAM_ULP_MC_CA_MAP_BIT13_MASK                      (0x1F << 5)
#define PSRAM_ULP_MC_CA_MAP_BIT13_SHIFT                     (5)
#define PSRAM_ULP_MC_CA_MAP_BIT14(n)                        (((n) & 0x1F) << 10)
#define PSRAM_ULP_MC_CA_MAP_BIT14_MASK                      (0x1F << 10)
#define PSRAM_ULP_MC_CA_MAP_BIT14_SHIFT                     (10)
#define PSRAM_ULP_MC_CA_MAP_BIT15(n)                        (((n) & 0x1F) << 15)
#define PSRAM_ULP_MC_CA_MAP_BIT15_MASK                      (0x1F << 15)
#define PSRAM_ULP_MC_CA_MAP_BIT15_SHIFT                     (15)
#define PSRAM_ULP_MC_CA_MAP_BIT16(n)                        (((n) & 0x1F) << 20)
#define PSRAM_ULP_MC_CA_MAP_BIT16_MASK                      (0x1F << 20)
#define PSRAM_ULP_MC_CA_MAP_BIT16_SHIFT                     (20)
#define PSRAM_ULP_MC_CA_MAP_BIT17(n)                        (((n) & 0x1F) << 25)
#define PSRAM_ULP_MC_CA_MAP_BIT17_MASK                      (0x1F << 25)
#define PSRAM_ULP_MC_CA_MAP_BIT17_SHIFT                     (25)

// reg_18c
#define PSRAM_ULP_MC_CA_MAP_BIT18(n)                        (((n) & 0x1F) << 0)
#define PSRAM_ULP_MC_CA_MAP_BIT18_MASK                      (0x1F << 0)
#define PSRAM_ULP_MC_CA_MAP_BIT18_SHIFT                     (0)
#define PSRAM_ULP_MC_CA_MAP_BIT19(n)                        (((n) & 0x1F) << 5)
#define PSRAM_ULP_MC_CA_MAP_BIT19_MASK                      (0x1F << 5)
#define PSRAM_ULP_MC_CA_MAP_BIT19_SHIFT                     (5)
#define PSRAM_ULP_MC_CA_MAP_BIT20(n)                        (((n) & 0x1F) << 10)
#define PSRAM_ULP_MC_CA_MAP_BIT20_MASK                      (0x1F << 10)
#define PSRAM_ULP_MC_CA_MAP_BIT20_SHIFT                     (10)
#define PSRAM_ULP_MC_CA_MAP_BIT21(n)                        (((n) & 0x1F) << 15)
#define PSRAM_ULP_MC_CA_MAP_BIT21_MASK                      (0x1F << 15)
#define PSRAM_ULP_MC_CA_MAP_BIT21_SHIFT                     (15)
#define PSRAM_ULP_MC_CA_MAP_BIT22(n)                        (((n) & 0x1F) << 20)
#define PSRAM_ULP_MC_CA_MAP_BIT22_MASK                      (0x1F << 20)
#define PSRAM_ULP_MC_CA_MAP_BIT22_SHIFT                     (20)
#define PSRAM_ULP_MC_CA_MAP_BIT23(n)                        (((n) & 0x1F) << 25)
#define PSRAM_ULP_MC_CA_MAP_BIT23_MASK                      (0x1F << 25)
#define PSRAM_ULP_MC_CA_MAP_BIT23_SHIFT                     (25)

// reg_190
#define PSRAM_ULP_MC_CA_MAP_BIT24(n)                        (((n) & 0x1F) << 0)
#define PSRAM_ULP_MC_CA_MAP_BIT24_MASK                      (0x1F << 0)
#define PSRAM_ULP_MC_CA_MAP_BIT24_SHIFT                     (0)
#define PSRAM_ULP_MC_CA_MAP_BIT25(n)                        (((n) & 0x1F) << 5)
#define PSRAM_ULP_MC_CA_MAP_BIT25_MASK                      (0x1F << 5)
#define PSRAM_ULP_MC_CA_MAP_BIT25_SHIFT                     (5)
#define PSRAM_ULP_MC_CA_MAP_BIT26(n)                        (((n) & 0x1F) << 10)
#define PSRAM_ULP_MC_CA_MAP_BIT26_MASK                      (0x1F << 10)
#define PSRAM_ULP_MC_CA_MAP_BIT26_SHIFT                     (10)
#define PSRAM_ULP_MC_CA_MAP_BIT27(n)                        (((n) & 0x1F) << 15)
#define PSRAM_ULP_MC_CA_MAP_BIT27_MASK                      (0x1F << 15)
#define PSRAM_ULP_MC_CA_MAP_BIT27_SHIFT                     (15)
#define PSRAM_ULP_MC_CA_MAP_BIT28(n)                        (((n) & 0x1F) << 20)
#define PSRAM_ULP_MC_CA_MAP_BIT28_MASK                      (0x1F << 20)
#define PSRAM_ULP_MC_CA_MAP_BIT28_SHIFT                     (20)
#define PSRAM_ULP_MC_CA_MAP_BIT29(n)                        (((n) & 0x1F) << 25)
#define PSRAM_ULP_MC_CA_MAP_BIT29_MASK                      (0x1F << 25)
#define PSRAM_ULP_MC_CA_MAP_BIT29_SHIFT                     (25)

// reg_194
#define PSRAM_ULP_MC_CA_MAP_BIT30(n)                        (((n) & 0x1F) << 0)
#define PSRAM_ULP_MC_CA_MAP_BIT30_MASK                      (0x1F << 0)
#define PSRAM_ULP_MC_CA_MAP_BIT30_SHIFT                     (0)
#define PSRAM_ULP_MC_CA_MAP_BIT31(n)                        (((n) & 0x1F) << 5)
#define PSRAM_ULP_MC_CA_MAP_BIT31_MASK                      (0x1F << 5)
#define PSRAM_ULP_MC_CA_MAP_BIT31_SHIFT                     (5)
#define PSRAM_ULP_MC_CA_MAP_BIT32(n)                        (((n) & 0x1F) << 10)
#define PSRAM_ULP_MC_CA_MAP_BIT32_MASK                      (0x1F << 10)
#define PSRAM_ULP_MC_CA_MAP_BIT32_SHIFT                     (10)

// reg_198
#define PSRAM_ULP_MC_RESERVED_0(n)                          (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_RESERVED_0_MASK                        (0xFF << 0)
#define PSRAM_ULP_MC_RESERVED_0_SHIFT                       (0)
#define PSRAM_ULP_MC_AUTO_PD_ENTER_EN                       (1 << 0)

// reg_19c
#define PSRAM_ULP_MC_SW_BUS_HALT_EN                         (1 << 0)
#define PSRAM_ULP_MC_SW_BUS_HALT_STS                        (1 << 1)

// reg_200
#define PSRAM_ULP_MC_DYN_FREQ_SET_POINT                     (1 << 0)
#define PSRAM_ULP_MC_DYN_FREQ_MR_VLD(n)                     (((n) & 0xFF) << 1)
#define PSRAM_ULP_MC_DYN_FREQ_MR_VLD_MASK                   (0xFF << 1)
#define PSRAM_ULP_MC_DYN_FREQ_MR_VLD_SHIFT                  (1)
#define PSRAM_ULP_MC_DYN_FREQ_STS                           (1 << 9)

// reg_204
#define PSRAM_ULP_MC_DYN_FREQ_MR0_0(n)                      (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_DYN_FREQ_MR0_0_MASK                    (0xFF << 0)
#define PSRAM_ULP_MC_DYN_FREQ_MR0_0_SHIFT                   (0)
#define PSRAM_ULP_MC_DYN_FREQ_MR1_0(n)                      (((n) & 0xFF) << 8)
#define PSRAM_ULP_MC_DYN_FREQ_MR1_0_MASK                    (0xFF << 8)
#define PSRAM_ULP_MC_DYN_FREQ_MR1_0_SHIFT                   (8)
#define PSRAM_ULP_MC_DYN_FREQ_MR2_0(n)                      (((n) & 0xFF) << 16)
#define PSRAM_ULP_MC_DYN_FREQ_MR2_0_MASK                    (0xFF << 16)
#define PSRAM_ULP_MC_DYN_FREQ_MR2_0_SHIFT                   (16)
#define PSRAM_ULP_MC_DYN_FREQ_MR3_0(n)                      (((n) & 0xFF) << 24)
#define PSRAM_ULP_MC_DYN_FREQ_MR3_0_MASK                    (0xFF << 24)
#define PSRAM_ULP_MC_DYN_FREQ_MR3_0_SHIFT                   (24)

// reg_208
#define PSRAM_ULP_MC_DYN_FREQ_MR4_0(n)                      (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_DYN_FREQ_MR4_0_MASK                    (0xFF << 0)
#define PSRAM_ULP_MC_DYN_FREQ_MR4_0_SHIFT                   (0)
#define PSRAM_ULP_MC_DYN_FREQ_MR5_0(n)                      (((n) & 0xFF) << 8)
#define PSRAM_ULP_MC_DYN_FREQ_MR5_0_MASK                    (0xFF << 8)
#define PSRAM_ULP_MC_DYN_FREQ_MR5_0_SHIFT                   (8)
#define PSRAM_ULP_MC_DYN_FREQ_MR6_0(n)                      (((n) & 0xFF) << 16)
#define PSRAM_ULP_MC_DYN_FREQ_MR6_0_MASK                    (0xFF << 16)
#define PSRAM_ULP_MC_DYN_FREQ_MR6_0_SHIFT                   (16)
#define PSRAM_ULP_MC_DYN_FREQ_MR7_0(n)                      (((n) & 0xFF) << 24)
#define PSRAM_ULP_MC_DYN_FREQ_MR7_0_MASK                    (0xFF << 24)
#define PSRAM_ULP_MC_DYN_FREQ_MR7_0_SHIFT                   (24)

// reg_20c
#define PSRAM_ULP_MC_DYN_FREQ_MD0_0(n)                      (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_DYN_FREQ_MD0_0_MASK                    (0xFF << 0)
#define PSRAM_ULP_MC_DYN_FREQ_MD0_0_SHIFT                   (0)
#define PSRAM_ULP_MC_DYN_FREQ_MD1_0(n)                      (((n) & 0xFF) << 8)
#define PSRAM_ULP_MC_DYN_FREQ_MD1_0_MASK                    (0xFF << 8)
#define PSRAM_ULP_MC_DYN_FREQ_MD1_0_SHIFT                   (8)
#define PSRAM_ULP_MC_DYN_FREQ_MD2_0(n)                      (((n) & 0xFF) << 16)
#define PSRAM_ULP_MC_DYN_FREQ_MD2_0_MASK                    (0xFF << 16)
#define PSRAM_ULP_MC_DYN_FREQ_MD2_0_SHIFT                   (16)
#define PSRAM_ULP_MC_DYN_FREQ_MD3_0(n)                      (((n) & 0xFF) << 24)
#define PSRAM_ULP_MC_DYN_FREQ_MD3_0_MASK                    (0xFF << 24)
#define PSRAM_ULP_MC_DYN_FREQ_MD3_0_SHIFT                   (24)

// reg_210
#define PSRAM_ULP_MC_DYN_FREQ_MD4_0(n)                      (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_DYN_FREQ_MD4_0_MASK                    (0xFF << 0)
#define PSRAM_ULP_MC_DYN_FREQ_MD4_0_SHIFT                   (0)
#define PSRAM_ULP_MC_DYN_FREQ_MD5_0(n)                      (((n) & 0xFF) << 8)
#define PSRAM_ULP_MC_DYN_FREQ_MD5_0_MASK                    (0xFF << 8)
#define PSRAM_ULP_MC_DYN_FREQ_MD5_0_SHIFT                   (8)
#define PSRAM_ULP_MC_DYN_FREQ_MD6_0(n)                      (((n) & 0xFF) << 16)
#define PSRAM_ULP_MC_DYN_FREQ_MD6_0_MASK                    (0xFF << 16)
#define PSRAM_ULP_MC_DYN_FREQ_MD6_0_SHIFT                   (16)
#define PSRAM_ULP_MC_DYN_FREQ_MD7_0(n)                      (((n) & 0xFF) << 24)
#define PSRAM_ULP_MC_DYN_FREQ_MD7_0_MASK                    (0xFF << 24)
#define PSRAM_ULP_MC_DYN_FREQ_MD7_0_SHIFT                   (24)

// reg_214
#define PSRAM_ULP_MC_DYN_FREQ_MR0_1(n)                      (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_DYN_FREQ_MR0_1_MASK                    (0xFF << 0)
#define PSRAM_ULP_MC_DYN_FREQ_MR0_1_SHIFT                   (0)
#define PSRAM_ULP_MC_DYN_FREQ_MR1_1(n)                      (((n) & 0xFF) << 8)
#define PSRAM_ULP_MC_DYN_FREQ_MR1_1_MASK                    (0xFF << 8)
#define PSRAM_ULP_MC_DYN_FREQ_MR1_1_SHIFT                   (8)
#define PSRAM_ULP_MC_DYN_FREQ_MR2_1(n)                      (((n) & 0xFF) << 16)
#define PSRAM_ULP_MC_DYN_FREQ_MR2_1_MASK                    (0xFF << 16)
#define PSRAM_ULP_MC_DYN_FREQ_MR2_1_SHIFT                   (16)
#define PSRAM_ULP_MC_DYN_FREQ_MR3_1(n)                      (((n) & 0xFF) << 24)
#define PSRAM_ULP_MC_DYN_FREQ_MR3_1_MASK                    (0xFF << 24)
#define PSRAM_ULP_MC_DYN_FREQ_MR3_1_SHIFT                   (24)

// reg_218
#define PSRAM_ULP_MC_DYN_FREQ_MR4_1(n)                      (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_DYN_FREQ_MR4_1_MASK                    (0xFF << 0)
#define PSRAM_ULP_MC_DYN_FREQ_MR4_1_SHIFT                   (0)
#define PSRAM_ULP_MC_DYN_FREQ_MR5_1(n)                      (((n) & 0xFF) << 8)
#define PSRAM_ULP_MC_DYN_FREQ_MR5_1_MASK                    (0xFF << 8)
#define PSRAM_ULP_MC_DYN_FREQ_MR5_1_SHIFT                   (8)
#define PSRAM_ULP_MC_DYN_FREQ_MR6_1(n)                      (((n) & 0xFF) << 16)
#define PSRAM_ULP_MC_DYN_FREQ_MR6_1_MASK                    (0xFF << 16)
#define PSRAM_ULP_MC_DYN_FREQ_MR6_1_SHIFT                   (16)
#define PSRAM_ULP_MC_DYN_FREQ_MR7_1(n)                      (((n) & 0xFF) << 24)
#define PSRAM_ULP_MC_DYN_FREQ_MR7_1_MASK                    (0xFF << 24)
#define PSRAM_ULP_MC_DYN_FREQ_MR7_1_SHIFT                   (24)

// reg_21c
#define PSRAM_ULP_MC_DYN_FREQ_MD0_1(n)                      (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_DYN_FREQ_MD0_1_MASK                    (0xFF << 0)
#define PSRAM_ULP_MC_DYN_FREQ_MD0_1_SHIFT                   (0)
#define PSRAM_ULP_MC_DYN_FREQ_MD1_1(n)                      (((n) & 0xFF) << 8)
#define PSRAM_ULP_MC_DYN_FREQ_MD1_1_MASK                    (0xFF << 8)
#define PSRAM_ULP_MC_DYN_FREQ_MD1_1_SHIFT                   (8)
#define PSRAM_ULP_MC_DYN_FREQ_MD2_1(n)                      (((n) & 0xFF) << 16)
#define PSRAM_ULP_MC_DYN_FREQ_MD2_1_MASK                    (0xFF << 16)
#define PSRAM_ULP_MC_DYN_FREQ_MD2_1_SHIFT                   (16)
#define PSRAM_ULP_MC_DYN_FREQ_MD3_1(n)                      (((n) & 0xFF) << 24)
#define PSRAM_ULP_MC_DYN_FREQ_MD3_1_MASK                    (0xFF << 24)
#define PSRAM_ULP_MC_DYN_FREQ_MD3_1_SHIFT                   (24)

// reg_220
#define PSRAM_ULP_MC_DYN_FREQ_MD4_1(n)                      (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_DYN_FREQ_MD4_1_MASK                    (0xFF << 0)
#define PSRAM_ULP_MC_DYN_FREQ_MD4_1_SHIFT                   (0)
#define PSRAM_ULP_MC_DYN_FREQ_MD5_1(n)                      (((n) & 0xFF) << 8)
#define PSRAM_ULP_MC_DYN_FREQ_MD5_1_MASK                    (0xFF << 8)
#define PSRAM_ULP_MC_DYN_FREQ_MD5_1_SHIFT                   (8)
#define PSRAM_ULP_MC_DYN_FREQ_MD6_1(n)                      (((n) & 0xFF) << 16)
#define PSRAM_ULP_MC_DYN_FREQ_MD6_1_MASK                    (0xFF << 16)
#define PSRAM_ULP_MC_DYN_FREQ_MD6_1_SHIFT                   (16)
#define PSRAM_ULP_MC_DYN_FREQ_MD7_1(n)                      (((n) & 0xFF) << 24)
#define PSRAM_ULP_MC_DYN_FREQ_MD7_1_MASK                    (0xFF << 24)
#define PSRAM_ULP_MC_DYN_FREQ_MD7_1_SHIFT                   (24)

// reg_224
#define PSRAM_ULP_MC_WRITE_LATENCY_1(n)                     (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_WRITE_LATENCY_1_MASK                   (0xFF << 0)
#define PSRAM_ULP_MC_WRITE_LATENCY_1_SHIFT                  (0)
#define PSRAM_ULP_MC_READ_LATENCY_1(n)                      (((n) & 0xFF) << 8)
#define PSRAM_ULP_MC_READ_LATENCY_1_MASK                    (0xFF << 8)
#define PSRAM_ULP_MC_READ_LATENCY_1_SHIFT                   (8)

// reg_228
#define PSRAM_ULP_MC_CP_WRAP_EN_1                           (1 << 0)
#define PSRAM_ULP_MC_BURST_LENGTH_1(n)                      (((n) & 0x7) << 1)
#define PSRAM_ULP_MC_BURST_LENGTH_1_MASK                    (0x7 << 1)
#define PSRAM_ULP_MC_BURST_LENGTH_1_SHIFT                   (1)

// reg_22c
#define PSRAM_ULP_MC_T_REFI_1(n)                            (((n) & 0xFFFF) << 0)
#define PSRAM_ULP_MC_T_REFI_1_MASK                          (0xFFFF << 0)
#define PSRAM_ULP_MC_T_REFI_1_SHIFT                         (0)

// reg_230
#define PSRAM_ULP_MC_T_RC_1(n)                              (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_T_RC_1_MASK                            (0xFF << 0)
#define PSRAM_ULP_MC_T_RC_1_SHIFT                           (0)

// reg_234
#define PSRAM_ULP_MC_T_RFC_1(n)                             (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_T_RFC_1_MASK                           (0xFF << 0)
#define PSRAM_ULP_MC_T_RFC_1_SHIFT                          (0)

// reg_238
#define PSRAM_ULP_MC_T_CPHR_1(n)                            (((n) & 0x3F) << 0)
#define PSRAM_ULP_MC_T_CPHR_1_MASK                          (0x3F << 0)
#define PSRAM_ULP_MC_T_CPHR_1_SHIFT                         (0)

// reg_23c
#define PSRAM_ULP_MC_T_CPHR_AP_1(n)                         (((n) & 0x3F) << 0)
#define PSRAM_ULP_MC_T_CPHR_AP_1_MASK                       (0x3F << 0)
#define PSRAM_ULP_MC_T_CPHR_AP_1_SHIFT                      (0)

// reg_240
#define PSRAM_ULP_MC_T_CPHW_1(n)                            (((n) & 0x3F) << 0)
#define PSRAM_ULP_MC_T_CPHW_1_MASK                          (0x3F << 0)
#define PSRAM_ULP_MC_T_CPHW_1_SHIFT                         (0)

// reg_244
#define PSRAM_ULP_MC_T_CPHW_AP_1(n)                         (((n) & 0x3F) << 0)
#define PSRAM_ULP_MC_T_CPHW_AP_1_MASK                       (0x3F << 0)
#define PSRAM_ULP_MC_T_CPHW_AP_1_SHIFT                      (0)

// reg_248
#define PSRAM_ULP_MC_T_MRR_1(n)                             (((n) & 0x3F) << 0)
#define PSRAM_ULP_MC_T_MRR_1_MASK                           (0x3F << 0)
#define PSRAM_ULP_MC_T_MRR_1_SHIFT                          (0)

// reg_24c
#define PSRAM_ULP_MC_T_MRS_1(n)                             (((n) & 0x3F) << 0)
#define PSRAM_ULP_MC_T_MRS_1_MASK                           (0x3F << 0)
#define PSRAM_ULP_MC_T_MRS_1_SHIFT                          (0)

// reg_250
#define PSRAM_ULP_MC_T_CEM_1(n)                             (((n) & 0xFFFF) << 0)
#define PSRAM_ULP_MC_T_CEM_1_MASK                           (0xFFFF << 0)
#define PSRAM_ULP_MC_T_CEM_1_SHIFT                          (0)

// reg_254
#define PSRAM_ULP_MC_T_RST_1(n)                             (((n) & 0xFFFF) << 0)
#define PSRAM_ULP_MC_T_RST_1_MASK                           (0xFFFF << 0)
#define PSRAM_ULP_MC_T_RST_1_SHIFT                          (0)

// reg_258
#define PSRAM_ULP_MC_T_SRF_1(n)                             (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_T_SRF_1_MASK                           (0xFF << 0)
#define PSRAM_ULP_MC_T_SRF_1_SHIFT                          (0)

// reg_25c
#define PSRAM_ULP_MC_T_XSR_1(n)                             (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_T_XSR_1_MASK                           (0xFF << 0)
#define PSRAM_ULP_MC_T_XSR_1_SHIFT                          (0)

// reg_260
#define PSRAM_ULP_MC_T_HS_1(n)                              (((n) & 0xFFFF) << 0)
#define PSRAM_ULP_MC_T_HS_1_MASK                            (0xFFFF << 0)
#define PSRAM_ULP_MC_T_HS_1_SHIFT                           (0)

// reg_264
#define PSRAM_ULP_MC_T_XPHS_1(n)                            (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_T_XPHS_1_MASK                          (0xFF << 0)
#define PSRAM_ULP_MC_T_XPHS_1_SHIFT                         (0)

// reg_268
#define PSRAM_ULP_MC_T_XHS_1(n)                             (((n) & 0xFFFFF) << 0)
#define PSRAM_ULP_MC_T_XHS_1_MASK                           (0xFFFFF << 0)
#define PSRAM_ULP_MC_T_XHS_1_SHIFT                          (0)

// reg_26c
#define PSRAM_ULP_MC_T_ZQCAL_1(n)                           (((n) & 0xFFFFF) << 0)
#define PSRAM_ULP_MC_T_ZQCAL_1_MASK                         (0xFFFFF << 0)
#define PSRAM_ULP_MC_T_ZQCAL_1_SHIFT                        (0)

// reg_270
#define PSRAM_ULP_MC_T_ZQCRST_1(n)                          (((n) & 0xFFFFF) << 0)
#define PSRAM_ULP_MC_T_ZQCRST_1_MASK                        (0xFFFFF << 0)
#define PSRAM_ULP_MC_T_ZQCRST_1_SHIFT                       (0)

// reg_274
#define PSRAM_ULP_MC_T_XCKD_1(n)                            (((n) & 0x3F) << 0)
#define PSRAM_ULP_MC_T_XCKD_1_MASK                          (0x3F << 0)
#define PSRAM_ULP_MC_T_XCKD_1_SHIFT                         (0)

// reg_278
#define PSRAM_ULP_MC_T_ECKD_1(n)                            (((n) & 0x3F) << 0)
#define PSRAM_ULP_MC_T_ECKD_1_MASK                          (0x3F << 0)
#define PSRAM_ULP_MC_T_ECKD_1_SHIFT                         (0)

// reg_27c
#define PSRAM_ULP_MC_T_ZQCAS_1(n)                           (((n) & 0xFFFFF) << 0)
#define PSRAM_ULP_MC_T_ZQCAS_1_MASK                         (0xFFFFF << 0)
#define PSRAM_ULP_MC_T_ZQCAS_1_SHIFT                        (0)

// reg_280
#define PSRAM_ULP_MC_T_REF_WIN_1(n)                         (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_T_REF_WIN_1_MASK                       (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_T_REF_WIN_1_SHIFT                      (0)

// reg_284
#define PSRAM_ULP_MC_T_CSCKE_1(n)                           (((n) & 0xFFFF) << 0)
#define PSRAM_ULP_MC_T_CSCKE_1_MASK                         (0xFFFF << 0)
#define PSRAM_ULP_MC_T_CSCKE_1_SHIFT                        (0)
#define PSRAM_ULP_MC_T_CSCKEH_1(n)                          (((n) & 0xFFFF) << 16)
#define PSRAM_ULP_MC_T_CSCKEH_1_MASK                        (0xFFFF << 16)
#define PSRAM_ULP_MC_T_CSCKEH_1_SHIFT                       (16)

// reg_400
#define PSRAM_ULP_MC_INIT_COMPLETE                          (1 << 0)

// reg_404
#define PSRAM_ULP_MC_BUSY                                   (1 << 0)
#define PSRAM_ULP_MC_MGR_RXFIFO_R_EMPTY                     (1 << 1)
#define PSRAM_ULP_MC_MGR_RXFIFO_FULL_CNT(n)                 (((n) & 0xF) << 2)
#define PSRAM_ULP_MC_MGR_RXFIFO_FULL_CNT_MASK               (0xF << 2)
#define PSRAM_ULP_MC_MGR_RXFIFO_FULL_CNT_SHIFT              (2)
#define PSRAM_ULP_MC_MGR_TXFIFO_W_FULL                      (1 << 6)
#define PSRAM_ULP_MC_MGR_TXFIFO_EMPTY_CNT(n)                (((n) & 0xF) << 7)
#define PSRAM_ULP_MC_MGR_TXFIFO_EMPTY_CNT_MASK              (0xF << 7)
#define PSRAM_ULP_MC_MGR_TXFIFO_EMPTY_CNT_SHIFT             (7)
#define PSRAM_ULP_MC_WB_FILL_LEVEL(n)                       (((n) & 0x1F) << 11)
#define PSRAM_ULP_MC_WB_FILL_LEVEL_MASK                     (0x1F << 11)
#define PSRAM_ULP_MC_WB_FILL_LEVEL_SHIFT                    (11)
#define PSRAM_ULP_MC_CP_FSM_STATE(n)                        (((n) & 0xF) << 16)
#define PSRAM_ULP_MC_CP_FSM_STATE_MASK                      (0xF << 16)
#define PSRAM_ULP_MC_CP_FSM_STATE_SHIFT                     (16)
#define PSRAM_ULP_MC_RD_FSM_0(n)                            (((n) & 0x3) << 20)
#define PSRAM_ULP_MC_RD_FSM_0_MASK                          (0x3 << 20)
#define PSRAM_ULP_MC_RD_FSM_0_SHIFT                         (20)
#define PSRAM_ULP_MC_RD_FSM_1(n)                            (((n) & 0x3) << 22)
#define PSRAM_ULP_MC_RD_FSM_1_MASK                          (0x3 << 22)
#define PSRAM_ULP_MC_RD_FSM_1_SHIFT                         (22)

// reg_440
#define PSRAM_ULP_MC_PMU_MONITOR_START                      (1 << 0)
#define PSRAM_ULP_MC_PMU_MONITOR_END                        (1 << 1)
#define PSRAM_ULP_MC_PMU_MONITOR_STS                        (1 << 2)

// reg_444
#define PSRAM_ULP_MC_PMU_TOL_MON_CLK_CYCLE0(n)              (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_PMU_TOL_MON_CLK_CYCLE0_MASK            (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_PMU_TOL_MON_CLK_CYCLE0_SHIFT           (0)

// reg_448
#define PSRAM_ULP_MC_PMU_TOL_MON_CLK_CYCLE1(n)              (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_PMU_TOL_MON_CLK_CYCLE1_MASK            (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_PMU_TOL_MON_CLK_CYCLE1_SHIFT           (0)

// reg_44c
#define PSRAM_ULP_MC_PMU_TOL_WR_DATA_BYTES0(n)              (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_PMU_TOL_WR_DATA_BYTES0_MASK            (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_PMU_TOL_WR_DATA_BYTES0_SHIFT           (0)

// reg_450
#define PSRAM_ULP_MC_PMU_TOL_WR_DATA_BYTES1(n)              (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_PMU_TOL_WR_DATA_BYTES1_MASK            (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_PMU_TOL_WR_DATA_BYTES1_SHIFT           (0)

// reg_454
#define PSRAM_ULP_MC_PMU_TOL_RD_DATA_BYTES0(n)              (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_PMU_TOL_RD_DATA_BYTES0_MASK            (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_PMU_TOL_RD_DATA_BYTES0_SHIFT           (0)

// reg_458
#define PSRAM_ULP_MC_PMU_TOL_RD_DATA_BYTES1(n)              (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_PMU_TOL_RD_DATA_BYTES1_MASK            (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_PMU_TOL_RD_DATA_BYTES1_SHIFT           (0)

// reg_45c
#define PSRAM_ULP_MC_PMU_TOL_RD_ACC_LATENCY0(n)             (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_PMU_TOL_RD_ACC_LATENCY0_MASK           (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_PMU_TOL_RD_ACC_LATENCY0_SHIFT          (0)

// reg_460
#define PSRAM_ULP_MC_PMU_TOL_RD_ACC_LATENCY1(n)             (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_PMU_TOL_RD_ACC_LATENCY1_MASK           (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_PMU_TOL_RD_ACC_LATENCY1_SHIFT          (0)

// reg_464
#define PSRAM_ULP_MC_PMU_TOL_RD_ACC_NUM0(n)                 (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_PMU_TOL_RD_ACC_NUM0_MASK               (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_PMU_TOL_RD_ACC_NUM0_SHIFT              (0)

// reg_468
#define PSRAM_ULP_MC_PMU_TOL_RD_ACC_NUM1(n)                 (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_PMU_TOL_RD_ACC_NUM1_MASK               (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_PMU_TOL_RD_ACC_NUM1_SHIFT              (0)

// reg_46c
#define PSRAM_ULP_MC_PMU_MAX_RD_ACC_LATENCY(n)              (((n) & 0xFFFF) << 0)
#define PSRAM_ULP_MC_PMU_MAX_RD_ACC_LATENCY_MASK            (0xFFFF << 0)
#define PSRAM_ULP_MC_PMU_MAX_RD_ACC_LATENCY_SHIFT           (0)

// reg_470
#define PSRAM_ULP_MC_PMU_TOL_WR_ACC_NUM_TO_DIE(n)           (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_PMU_TOL_WR_ACC_NUM_TO_DIE_MASK         (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_PMU_TOL_WR_ACC_NUM_TO_DIE_SHIFT        (0)

// reg_474
#define PSRAM_ULP_MC_PMU_TOL_WR_CYCS_IN_MCLK_0(n)           (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_PMU_TOL_WR_CYCS_IN_MCLK_0_MASK         (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_PMU_TOL_WR_CYCS_IN_MCLK_0_SHIFT        (0)

// reg_478
#define PSRAM_ULP_MC_PMU_TOL_WR_CYCS_IN_MCLK_1(n)           (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_PMU_TOL_WR_CYCS_IN_MCLK_1_MASK         (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_PMU_TOL_WR_CYCS_IN_MCLK_1_SHIFT        (0)

// reg_47c
#define PSRAM_ULP_MC_PMU_TOL_RD_ACC_NUM_TO_DIE(n)           (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_PMU_TOL_RD_ACC_NUM_TO_DIE_MASK         (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_PMU_TOL_RD_ACC_NUM_TO_DIE_SHIFT        (0)

// reg_480
#define PSRAM_ULP_MC_PMU_TOL_RD_CYCS_IN_MCLK_0(n)           (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_PMU_TOL_RD_CYCS_IN_MCLK_0_MASK         (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_PMU_TOL_RD_CYCS_IN_MCLK_0_SHIFT        (0)

// reg_484
#define PSRAM_ULP_MC_PMU_TOL_RD_CYCS_IN_MCLK_1(n)           (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_ULP_MC_PMU_TOL_RD_CYCS_IN_MCLK_1_MASK         (0xFFFFFFFF << 0)
#define PSRAM_ULP_MC_PMU_TOL_RD_CYCS_IN_MCLK_1_SHIFT        (0)

// reg_490
#define PSRAM_ULP_MC_DBG_READING_ADDR_0(n)                  (((n) & 0x3FFFFFF) << 0)
#define PSRAM_ULP_MC_DBG_READING_ADDR_0_MASK                (0x3FFFFFF << 0)
#define PSRAM_ULP_MC_DBG_READING_ADDR_0_SHIFT               (0)

// reg_494
#define PSRAM_ULP_MC_DBG_FETCH_VLD_0(n)                     (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_DBG_FETCH_VLD_0_MASK                   (0xFF << 0)
#define PSRAM_ULP_MC_DBG_FETCH_VLD_0_SHIFT                  (0)
#define PSRAM_ULP_MC_DBG_SPLIT_0(n)                         (((n) & 0x3) << 8)
#define PSRAM_ULP_MC_DBG_SPLIT_0_MASK                       (0x3 << 8)
#define PSRAM_ULP_MC_DBG_SPLIT_0_SHIFT                      (8)
#define PSRAM_ULP_MC_DBG_REQ_ST_0(n)                        (((n) & 0x7) << 10)
#define PSRAM_ULP_MC_DBG_REQ_ST_0_MASK                      (0x7 << 10)
#define PSRAM_ULP_MC_DBG_REQ_ST_0_SHIFT                     (10)
#define PSRAM_ULP_MC_DBG_RB_VLD_0(n)                        (((n) & 0xFF) << 13)
#define PSRAM_ULP_MC_DBG_RB_VLD_0_MASK                      (0xFF << 13)
#define PSRAM_ULP_MC_DBG_RB_VLD_0_SHIFT                     (13)
#define PSRAM_ULP_MC_DBG_RD_BUSY_0                          (1 << 21)

// reg_498
#define PSRAM_ULP_MC_DBG_READING_ADDR_1(n)                  (((n) & 0x3FFFFFF) << 0)
#define PSRAM_ULP_MC_DBG_READING_ADDR_1_MASK                (0x3FFFFFF << 0)
#define PSRAM_ULP_MC_DBG_READING_ADDR_1_SHIFT               (0)

// reg_49c
#define PSRAM_ULP_MC_DBG_FETCH_VLD_1(n)                     (((n) & 0xFF) << 0)
#define PSRAM_ULP_MC_DBG_FETCH_VLD_1_MASK                   (0xFF << 0)
#define PSRAM_ULP_MC_DBG_FETCH_VLD_1_SHIFT                  (0)
#define PSRAM_ULP_MC_DBG_SPLIT_1(n)                         (((n) & 0x3) << 8)
#define PSRAM_ULP_MC_DBG_SPLIT_1_MASK                       (0x3 << 8)
#define PSRAM_ULP_MC_DBG_SPLIT_1_SHIFT                      (8)
#define PSRAM_ULP_MC_DBG_REQ_ST_1(n)                        (((n) & 0x7) << 10)
#define PSRAM_ULP_MC_DBG_REQ_ST_1_MASK                      (0x7 << 10)
#define PSRAM_ULP_MC_DBG_REQ_ST_1_SHIFT                     (10)
#define PSRAM_ULP_MC_DBG_RB_VLD_1(n)                        (((n) & 0xFF) << 13)
#define PSRAM_ULP_MC_DBG_RB_VLD_1_MASK                      (0xFF << 13)
#define PSRAM_ULP_MC_DBG_RB_VLD_1_SHIFT                     (13)
#define PSRAM_ULP_MC_DBG_RD_BUSY_1                          (1 << 21)

// reg_4c0
#define PSRAM_ULP_MC_DBG_HRSTN_W_SET                        (1 << 0)
#define PSRAM_ULP_MC_DBG_HRSTN_R_SET                        (1 << 1)
#define PSRAM_ULP_MC_DBG_HRSTN_O_SET                        (1 << 2)
#define PSRAM_ULP_MC_DBG_MRSTN_SET                          (1 << 3)

// reg_4c4
#define PSRAM_ULP_MC_DBG_HRSTN_W_CLR                        (1 << 0)
#define PSRAM_ULP_MC_DBG_HRSTN_R_CLR                        (1 << 1)
#define PSRAM_ULP_MC_DBG_HRSTN_O_CLR                        (1 << 2)
#define PSRAM_ULP_MC_DBG_MRSTN_CLR                          (1 << 3)

// reg_4d4
#define PSRAM_ULP_MC_WRITE_UNLOCK_CLR                       (1 << 0)

// reg_4d8
#define PSRAM_ULP_MC_WRITE_UNLOCK_SET                       (1 << 0)

#endif
