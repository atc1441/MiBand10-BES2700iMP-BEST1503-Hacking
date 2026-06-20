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
#ifndef __REG_DSI_H__
#define __REG_DSI_H__

#include "plat_types.h"

struct DSI_REG_T {
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
    __IO uint32_t REG_RES[5];
    __IO uint32_t REG_060;
    __IO uint32_t REG_064;
    __IO uint32_t REG_068;
    __IO uint32_t REG_06C;
    __IO uint32_t REG_070;
    __IO uint32_t REG_074;
};

// reg_00
#define DSI_R_LANE_NUM(n)                                   (((n) & 0x3) << 0)
#define DSI_R_LANE_NUM_MASK                                 (0x3 << 0)
#define DSI_R_LANE_NUM_SHIFT                                (0)
#define DSI_R_LPCD_EN                                       (1 << 2)
#define DSI_R_LPCD_DLY(n)                                   (((n) & 0x3) << 3)
#define DSI_R_LPCD_DLY_MASK                                 (0x3 << 3)
#define DSI_R_LPCD_DLY_SHIFT                                (3)
#define DSI_R_HSA_LP                                        (1 << 5)
#define DSI_R_HSE_EN                                        (1 << 6)
#define DSI_R_HBP_LP                                        (1 << 7)
#define DSI_R_HFP_LP                                        (1 << 8)
#define DSI_R_EOTP_EN                                       (1 << 9)
#define DSI_R_CLANE_LP_EN                                   (1 << 10)
#define DSI_R_VIDEO_MODE                                    (1 << 11)
#define DSI_R_T_BTA(n)                                      (((n) & 0xF) << 12)
#define DSI_R_T_BTA_MASK                                    (0xF << 12)
#define DSI_R_T_BTA_SHIFT                                   (12)
#define DSI_R_T_LPX(n)                                      (((n) & 0xF) << 16)
#define DSI_R_T_LPX_MASK                                    (0xF << 16)
#define DSI_R_T_LPX_SHIFT                                   (16)
#define DSI_R_CLK_T_LPX(n)                                  (((n) & 0xF) << 20)
#define DSI_R_CLK_T_LPX_MASK                                (0xF << 20)
#define DSI_R_CLK_T_LPX_SHIFT                               (20)
#define DSI_R_CLK_PRE(n)                                    (((n) & 0x3) << 24)
#define DSI_R_CLK_PRE_MASK                                  (0x3 << 24)
#define DSI_R_CLK_PRE_SHIFT                                 (24)
#define DSI_R_CLK_POST(n)                                   (((n) & 0x3F) << 26)
#define DSI_R_CLK_POST_MASK                                 (0x3F << 26)
#define DSI_R_CLK_POST_SHIFT                                (26)

// reg_04
#define DSI_R_HS_EXIT_TIME(n)                               (((n) & 0x3F) << 0)
#define DSI_R_HS_EXIT_TIME_MASK                             (0x3F << 0)
#define DSI_R_HS_EXIT_TIME_SHIFT                            (0)
#define DSI_R_HS_PRPR_TIME(n)                               (((n) & 0xF) << 6)
#define DSI_R_HS_PRPR_TIME_MASK                             (0xF << 6)
#define DSI_R_HS_PRPR_TIME_SHIFT                            (6)
#define DSI_R_HS_ZERO_TIME(n)                               (((n) & 0x3F) << 10)
#define DSI_R_HS_ZERO_TIME_MASK                             (0x3F << 10)
#define DSI_R_HS_ZERO_TIME_SHIFT                            (10)
#define DSI_R_HS_TRAIL_TIME(n)                              (((n) & 0x3F) << 16)
#define DSI_R_HS_TRAIL_TIME_MASK                            (0x3F << 16)
#define DSI_R_HS_TRAIL_TIME_SHIFT                           (16)
#define DSI_R_T_WAKEUP(n)                                   (((n) & 0x3FF) << 22)
#define DSI_R_T_WAKEUP_MASK                                 (0x3FF << 22)
#define DSI_R_T_WAKEUP_SHIFT                                (22)

// reg_08
#define DSI_R_CLK_EXIT_TIME(n)                              (((n) & 0x3F) << 0)
#define DSI_R_CLK_EXIT_TIME_MASK                            (0x3F << 0)
#define DSI_R_CLK_EXIT_TIME_SHIFT                           (0)
#define DSI_R_CLK_PRPR_TIME(n)                              (((n) & 0xF) << 6)
#define DSI_R_CLK_PRPR_TIME_MASK                            (0xF << 6)
#define DSI_R_CLK_PRPR_TIME_SHIFT                           (6)
#define DSI_R_CLK_ZERO_TIME(n)                              (((n) & 0x3F) << 10)
#define DSI_R_CLK_ZERO_TIME_MASK                            (0x3F << 10)
#define DSI_R_CLK_ZERO_TIME_SHIFT                           (10)
#define DSI_R_CLK_TRAIL_TIME(n)                             (((n) & 0x3F) << 16)
#define DSI_R_CLK_TRAIL_TIME_MASK                           (0x3F << 16)
#define DSI_R_CLK_TRAIL_TIME_SHIFT                          (16)
#define DSI_R_CLK_T_WAKEUP(n)                               (((n) & 0x3FF) << 22)
#define DSI_R_CLK_T_WAKEUP_MASK                             (0x3FF << 22)
#define DSI_R_CLK_T_WAKEUP_SHIFT                            (22)

// reg_0c
#define DSI_R_DTYPE(n)                                      (((n) & 0x3F) << 0)
#define DSI_R_DTYPE_MASK                                    (0x3F << 0)
#define DSI_R_DTYPE_SHIFT                                   (0)
#define DSI_R_VC_CH_ID(n)                                   (((n) & 0x3) << 6)
#define DSI_R_VC_CH_ID_MASK                                 (0x3 << 6)
#define DSI_R_VC_CH_ID_SHIFT                                (6)
#define DSI_R_VIDEO_PACKET_LENTH(n)                         (((n) & 0xFFFF) << 8)
#define DSI_R_VIDEO_PACKET_LENTH_MASK                       (0xFFFF << 8)
#define DSI_R_VIDEO_PACKET_LENTH_SHIFT                      (8)
#define DSI_R_INPUT_TYPE(n)                                 (((n) & 0x3) << 24)
#define DSI_R_INPUT_TYPE_MASK                               (0x3 << 24)
#define DSI_R_INPUT_TYPE_SHIFT                              (24)
#define DSI_R_DLANE_AD_TIME(n)                              (((n) & 0x3F) << 26)
#define DSI_R_DLANE_AD_TIME_MASK                            (0x3F << 26)
#define DSI_R_DLANE_AD_TIME_SHIFT                           (26)

// reg_10
#define DSI_LPRX_DATA(n)                                    (((n) & 0xFF) << 0)
#define DSI_LPRX_DATA_MASK                                  (0xFF << 0)
#define DSI_LPRX_DATA_SHIFT                                 (0)
#define DSI_LPRX_SIZE(n)                                    (((n) & 0x1F) << 8)
#define DSI_LPRX_SIZE_MASK                                  (0x1F << 8)
#define DSI_LPRX_SIZE_SHIFT                                 (8)
#define DSI_HS_CMD_RD_EN                                    (1 << 14)
#define DSI_LPRX_RD_EN                                      (1 << 15)
#define DSI_CMD_DONE_FLAG                                   (1 << 16)
#define DSI_BTA_DONE_FLAG                                   (1 << 17)
#define DSI_BTA_FAIL_FLAG                                   (1 << 18)
#define DSI_LP_RX_DONE_FLAG                                 (1 << 19)
#define DSI_RX_ERR_FLAG                                     (1 << 20)
#define DSI_LPCD_FLAG                                       (1 << 21)
#define DSI_RX_TIMEOUT_FLAG                                 (1 << 22)
#define DSI_TE_INTR                                         (1 << 23)
#define DSI_INTR_MASK(n)                                    (((n) & 0xFF) << 24)
#define DSI_INTR_MASK_MASK                                  (0xFF << 24)
#define DSI_INTR_MASK_SHIFT                                 (24)
#define DSI_FLAG_MASK(n)                                    (((n) & 0xFF) << 16)
#define DSI_FLAG_MASK_MASK                                  (0xFF << 16)
#define DSI_FLAG_MASK_SHIFT                                 (16)

// reg_14
#define DSI_CMD_TYPE(n)                                     (((n) & 0x7) << 0)
#define DSI_CMD_TYPE_MASK                                   (0x7 << 0)
#define DSI_CMD_TYPE_SHIFT                                  (0)
#define DSI_FLUSH_CMD_PRE                                   (1 << 3)
#define DSI_LP_CMD_VALID                                    (1 << 4)
#define DSI_HS_CMD_VALID                                    (1 << 5)
#define DSI_BTA_VALID                                       (1 << 6)
#define DSI_ESC_CMD(n)                                      (((n) & 0xFF) << 8)
#define DSI_ESC_CMD_MASK                                    (0xFF << 8)
#define DSI_ESC_CMD_SHIFT                                   (8)
#define DSI_R_SOFT_CMD_LENGTH(n)                            (((n) & 0xFFF) << 16)
#define DSI_R_SOFT_CMD_LENGTH_MASK                          (0xFFF << 16)
#define DSI_R_SOFT_CMD_LENGTH_SHIFT                         (16)

// reg_30
#define DSI_OV_FLAG                                         (1 << 0)
#define DSI_UN_FLAG                                         (1 << 1)
#define DSI_CTRL_STX(n)                                     (((n) & 0x1F) << 8)
#define DSI_CTRL_STX_MASK                                   (0x1F << 8)
#define DSI_CTRL_STX_SHIFT                                  (8)
#define DSI_CTRL_ST(n)                                      (((n) & 0xF) << 13)
#define DSI_CTRL_ST_MASK                                    (0xF << 13)
#define DSI_CTRL_ST_SHIFT                                   (13)
#define DSI_HS_CTRL_ST(n)                                   (((n) & 0xF) << 17)
#define DSI_HS_CTRL_ST_MASK                                 (0xF << 17)
#define DSI_HS_CTRL_ST_SHIFT                                (17)

// reg_40
#define DSI_R_HTOTAL(n)                                     (((n) & 0xFFF) << 0)
#define DSI_R_HTOTAL_MASK                                   (0xFFF << 0)
#define DSI_R_HTOTAL_SHIFT                                  (0)
#define DSI_R_HSTART(n)                                     (((n) & 0xFF) << 12)
#define DSI_R_HSTART_MASK                                   (0xFF << 12)
#define DSI_R_HSTART_SHIFT                                  (12)
#define DSI_R_HWIDTH(n)                                     (((n) & 0xFFF) << 20)
#define DSI_R_HWIDTH_MASK                                   (0xFFF << 20)
#define DSI_R_HWIDTH_SHIFT                                  (20)

// reg_44
#define DSI_R_VTOTAL(n)                                     (((n) & 0xFFF) << 0)
#define DSI_R_VTOTAL_MASK                                   (0xFFF << 0)
#define DSI_R_VTOTAL_SHIFT                                  (0)
#define DSI_R_VSTART(n)                                     (((n) & 0xFF) << 12)
#define DSI_R_VSTART_MASK                                   (0xFF << 12)
#define DSI_R_VSTART_SHIFT                                  (12)
#define DSI_R_VHEIGHT(n)                                    (((n) & 0xFFF) << 20)
#define DSI_R_VHEIGHT_MASK                                  (0xFFF << 20)
#define DSI_R_VHEIGHT_SHIFT                                 (20)

// reg_48
#define DSI_R_CL_DATA_SEL                                   (1 << 0)
#define DSI_R_TE_EDGE_SEL                                   (1 << 1)
#define DSI_R_VIDEO_SEL                                     (1 << 2)
#define DSI_R_VIDEO_BIST_EN                                 (1 << 3)
#define DSI_R_VIDEO_BIST_PATTERN(n)                         (((n) & 0x7) << 4)
#define DSI_R_VIDEO_BIST_PATTERN_MASK                       (0x7 << 4)
#define DSI_R_VIDEO_BIST_PATTERN_SHIFT                      (4)
#define DSI_R_COLOR_BAR_WIDTH(n)                            (((n) & 0xFFF) << 8)
#define DSI_R_COLOR_BAR_WIDTH_MASK                          (0xFFF << 8)
#define DSI_R_COLOR_BAR_WIDTH_SHIFT                         (8)
#define DSI_R_HSYNC_DLY_NUM(n)                              (((n) & 0xFFF) << 20)
#define DSI_R_HSYNC_DLY_NUM_MASK                            (0xFFF << 20)
#define DSI_R_HSYNC_DLY_NUM_SHIFT                           (20)

//2003 reg_60
#define DSI_REG_CK_LB_HS(n)                                 (((n) & 0xFF) << 0)
#define DSI_REG_CK_LB_HS_MASK                               (0xFF << 0)
#define DSI_REG_CK_LB_HS_SHIFT                              (0)
#define DSI_REG_CK_LB_ZERO(n)                               (((n) & 0xFF) << 8)
#define DSI_REG_CK_LB_ZERO_MASK                             (0xFF << 8)
#define DSI_REG_CK_LB_ZERO_SHIFT                            (8)
#define DSI_REG_CK_LB_PRPR(n)                               (((n) & 0xFF) << 16)
#define DSI_REG_CK_LB_PRPR_MASK                             (0xFF << 16)
#define DSI_REG_CK_LB_PRPR_SHIFT                            (16)
#define DSI_REG_CK_LB_RQST(n)                               (((n) & 0xFF) << 24)
#define DSI_REG_CK_LB_RQST_MASK                             (0xFF << 24)
#define DSI_REG_CK_LB_RQST_SHIFT                            (24)

//1600 REG060
#define REG_EN_LPRX0_DSI                                    (1 << 0)
#define REG_CLK_SWRC_DSI_SHIFT                              1
#define REG_CLK_SWRC_DSI_MASK                               (0x7 << REG_CLK_SWRC_DSI_SHIFT)
#define REG_CLK_SWRC_DSI(n)                                 BITFIELD_VAL(REG_CLK_SWRC_DSI, n)
#define REG_CLKGEN_PU_DSI                                   (1 << 4)
#define REG_RX_SWAP_PN_DSI                                  (1 << 5)
#define REG_SWAP_CKX1_DSI                                   (1 << 6)
#define REG_SWAP_CKX8_DSI                                   (1 << 7)
#define REG_PRECHARGE_LDO_DSI                               (1 << 8)
#define REG_PU_CH_DSI                                       (1 << 9)
#define REG_PU_LDO_DSI                                      (1 << 10)
#define REG_DSI_PHY_RSTN                                    (1 << 11)
#define REG_MASK_LP_LDO                                     (1 << 12)
#define REG_MASK_LP_RSTN                                    (1 << 13)
#define REG_CKX8_EN                                         (1 << 14)

//1600 REG064
#define REG_LDO0P4_RES_SHIFT                                0
#define REG_LDO0P4_RES_MASK                                 (0xF << REG_LDO0P4_RES_SHIFT)
#define REG_LDO0P4_RES(n)                                   BITFIELD_VAL(REG_LDO0P4_RES, n)
#define REG_LDO0P8_RES_SHIFT                                4
#ifdef CHIP_BEST1502X
#define REG_LDO0P8_RES_MASK                                 (0xF << REG_LDO0P8_RES_SHIFT)
#else
#define REG_LDO0P8_RES_MASK                                 (0x7 << REG_LDO0P8_RES_SHIFT)
#endif
#define REG_LDO0P8_RES(n)                                   BITFIELD_VAL(REG_LDO0P8_RES, n)
#define REG_LDO1P2_RES_SHIFT                                8
#ifdef CHIP_BEST1502X
#define REG_LDO1P2_RES_MASK                                 (0xF << REG_LDO1P2_RES_SHIFT)
#else
#define REG_LDO1P2_RES_MASK                                 (0x7 << REG_LDO1P2_RES_SHIFT)
#endif
#define REG_LDO1P2_RES(n)                                   BITFIELD_VAL(REG_LDO1P2_RES, n)
#define REG_VB_LPCD_H_SHIFT                                 12
#define REG_VB_LPCD_H_MASK                                  (0x7 << REG_VB_LPCD_H_SHIFT)
#define REG_VB_LPCD_H(n)                                    BITFIELD_VAL(REG_VB_LPCD_H, n)
#define REG_VB_LPCD_L_SHIFT                                 15
#define REG_VB_LPCD_L_MASK                                  (0x7 << REG_VB_LPCD_L_SHIFT)
#define REG_VB_LPCD_L(n)                                    BITFIELD_VAL(REG_VB_LPCD_L, n)
#define REG_VB_LPRX_H_SHIFT                                 18
#define REG_VB_LPRX_H_MASK                                  (0x7 << REG_VB_LPRX_H_SHIFT)
#define REG_VB_LPRX_H(n)                                    BITFIELD_VAL(REG_VB_LPRX_H, n)
#define REG_VB_LPRX_L_SHIFT                                 21
#define REG_VB_LPRX_L_MASK                                  (0x7 << REG_VB_LPRX_L_SHIFT)
#define REG_VB_LPRX_L(n)                                    BITFIELD_VAL(REG_VB_LPRX_L, n)

// reg_64
#define DSI_REG_DATA_LB_ZERO(n)                             (((n) & 0xFF) << 0)
#define DSI_REG_DATA_LB_ZERO_MASK                           (0xFF << 0)
#define DSI_REG_DATA_LB_ZERO_SHIFT                          (0)
#define DSI_REG_DATA_LB_PRPR(n)                             (((n) & 0xFF) << 8)
#define DSI_REG_DATA_LB_PRPR_MASK                           (0xFF << 8)
#define DSI_REG_DATA_LB_PRPR_SHIFT                          (8)
#define DSI_REG_DATA_LB_RQST(n)                             (((n) & 0xFF) << 16)
#define DSI_REG_DATA_LB_RQST_MASK                           (0xFF << 16)
#define DSI_REG_DATA_LB_RQST_SHIFT                          (16)
#define DSI_REG_CK_LB_DATA_SEL                              (1 << 24)
#define DSI_REG_LOOPBACK_TEST                               (1 << 25)

//REG068
#define REG_CH0_EN_DSI                                      (1 << 0)
#define REG_CH0_DLY_TUNE_DSI_SHIFT                          1
#define REG_CH0_DLY_TUNE_DSI_MASK                           (0x7 << REG_CH0_DLY_TUNE_DSI_SHIFT)
#define REG_CH0_DLY_TUNE_DSI(n)                             BITFIELD_VAL(REG_CH0_DLY_TUNE_DSI, n)
#define REG_CH0_HSTX_DRV_DSI_SHIFT                          4
#define REG_CH0_HSTX_DRV_DSI_MASK                           (0xF << REG_CH0_HSTX_DRV_DSI_SHIFT)
#define REG_CH0_HSTX_DRV_DSI(n)                             BITFIELD_VAL(REG_CH0_HSTX_DRV_DSI, n)
#define REG_CH0_LPTX_DRV_DSI_SHIFT                          8
#define REG_CH0_LPTX_DRV_DSI_MASK                           (0x7 << REG_CH0_LPTX_DRV_DSI_SHIFT)
#define REG_CH0_LPTX_DRV_DSI(n)                             BITFIELD_VAL(REG_CH0_LPTX_DRV_DSI, n)
#define REG_CH0_PULL_DN_DSI                                 (1 << 11)
#define REG_CH0_SWAP_PN_DSI                                 (1 << 12)
#define RESERVED_DSI_11_9_SHIFT                             13
#define RESERVED_DSI_11_9_MASK                              (0x7 << RESERVED_DSI_11_9_SHIFT)
#define RESERVED_DSI_11_9(n)                                BITFIELD_VAL(RESERVED_DSI_11_9, n)

//REG06C
#define REG_CH1_EN_DSI                                      (1 << 0)
#define REG_CH1_DLY_TUNE_DSI_SHIFT                          1
#define REG_CH1_DLY_TUNE_DSI_MASK                           (0x7 << REG_CH1_DLY_TUNE_DSI_SHIFT)
#define REG_CH1_DLY_TUNE_DSI(n)                             BITFIELD_VAL(REG_CH1_DLY_TUNE_DSI, n)
#define REG_CH1_HSTX_DRV_DSI_SHIFT                          4
#define REG_CH1_HSTX_DRV_DSI_MASK                           (0xF << REG_CH1_HSTX_DRV_DSI_SHIFT)
#define REG_CH1_HSTX_DRV_DSI(n)                             BITFIELD_VAL(REG_CH1_HSTX_DRV_DSI, n)
#define REG_CH1_LPTX_DRV_DSI_SHIFT                          8
#define REG_CH1_LPTX_DRV_DSI_MASK                           (0x7 << REG_CH1_LPTX_DRV_DSI_SHIFT)
#define REG_CH1_LPTX_DRV_DSI(n)                             BITFIELD_VAL(REG_CH1_LPTX_DRV_DSI, n)
#define REG_CH1_PULL_DN_DSI                                 (1 << 11)
#define REG_CH1_SWAP_PN_DSI                                 (1 << 12)
#define RESERVED_DSI_14_12_SHIFT                            13
#define RESERVED_DSI_14_12_MASK                             (0x7 << RESERVED_DSI_14_12_SHIFT)
#define RESERVED_DSI_14_12(n)                               BITFIELD_VAL(RESERVED_DSI_14_12, n)

//REG070
#define REG_CHCK_EN_DSI                                     (1 << 0)
#define REG_CHCK_DLY_TUNE_DSI_SHIFT                         1
#define REG_CHCK_DLY_TUNE_DSI_MASK                          (0x7 << REG_CHCK_DLY_TUNE_DSI_SHIFT)
#define REG_CHCK_DLY_TUNE_DSI(n)                            BITFIELD_VAL(REG_CHCK_DLY_TUNE_DSI, n)
#define REG_CHCK_HSTX_DRV_DSI_SHIFT                         4
#define REG_CHCK_HSTX_DRV_DSI_MASK                          (0xF << REG_CHCK_HSTX_DRV_DSI_SHIFT)
#define REG_CHCK_HSTX_DRV_DSI(n)                            BITFIELD_VAL(REG_CHCK_HSTX_DRV_DSI, n)
#define REG_CHCK_LPTX_DRV_DSI_SHIFT                         8
#define REG_CHCK_LPTX_DRV_DSI_MASK                          (0x7 << REG_CHCK_LPTX_DRV_DSI_SHIFT)
#define REG_CHCK_LPTX_DRV_DSI(n)                            BITFIELD_VAL(REG_CHCK_LPTX_DRV_DSI, n)
#define REG_CHCK_PULL_DN_DSI                                (1 << 11)
#define REG_CHCK_SWAP_PN_DSI                                (1 << 12)
#define REG_PRECHARGE_LDO_CNT_SHIFT                         13
#define REG_PRECHARGE_LDO_CNT_MASK                          (0x3ff << REG_PRECHARGE_LDO_CNT_SHIFT)
#define REG_PRECHARGE_LDO_CNT(n)                            BITFIELD_VAL(REG_PRECHARGE_LDO_CNT,n)

// reg_74
#define DSI_R_HSW(n)                                        (((n) & 0x7F) << 0)
#define DSI_R_HSW_MASK                                      (0x7F << 0)
#define DSI_R_HSW_SHIFT                                     (0)
#define DSI_R_VSW(n)                                        (((n) & 0x7F) << 7)
#define DSI_R_VSW_MASK                                      (0x7F << 7)
#define DSI_R_VSW_SHIFT                                     (7)
#define DSI_R_HS_TRAIL_TIME2(n)                             (((n) & 0x3F) << 16)
#define DSI_R_HS_TRAIL_TIME2_MASK                           (0x3F << 16)
#define DSI_R_HS_TRAIL_TIME2_SHIFT                          (16)

#endif

