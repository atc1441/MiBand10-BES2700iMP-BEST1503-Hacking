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
#ifndef __CSI_REG_H__
#define __CSI_REG_H__

#include "plat_types.h"

struct CSI_REG_T {
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
};

// REG00
#define CSI_R_HS_PRPR_TIME(n)                               (((n) & 0xFF) << 24)
#define CSI_R_HS_PRPR_TIME_MASK                             (0xFF << 24)
#define CSI_R_HS_PRPR_TIME_SHIFT                            (24)
#define CSI_R_PWR_INI_DELAY(n)                              (((n) & 0xFF) << 16)
#define CSI_R_PWR_INI_DELAY_MASK                            (0xFF << 16)
#define CSI_R_PWR_INI_DELAY_SHIFT                           (16)
#define CSI_R_TIMER_LPX(n)                                  (((n) & 0xFF) << 8)
#define CSI_R_TIMER_LPX_MASK                                (0xFF << 8)
#define CSI_R_TIMER_LPX_SHIFT                               (8)
#define CSI_R_FORCE_TIME_LPX                                (1 << 7)
#define CSI_R_LPCD_CYCS                                     (1 << 6)
#define CSI_R_LPCD_DLY                                      (1 << 5)
#define CSI_R_LPCD_EN                                       (1 << 4)
#define CSI_R_DISABLE_TA_FAIL                               (1 << 3)
#define CSI_R_CKLANE_HS_EN                                  (1 << 2)
#define CSI_R_LANE_NUM(n)                                   (((n) & 0x3) << 0)
#define CSI_R_LANE_NUM_MASK                                 (0x3 << 0)
#define CSI_R_LANE_NUM_SHIFT                                (0)

// REG0C
#define CSI_R_USER_DTYPE_BN3(n)                                   (((n) & 0x3) << 30)
#define CSI_R_USER_DTYPE_BN3_MASK                                 (0x3 << 30)
#define CSI_R_USER_DTYPE_BN3_SHIFT                                (30)
#define CSI_R_USER_DTYPE_BN2(n)                                   (((n) & 0x3) << 28)
#define CSI_R_USER_DTYPE_BN2_MASK                                 (0x3 << 28)
#define CSI_R_USER_DTYPE_BN2_SHIFT                                (28)
#define CSI_R_USER_DTYPE_BN1(n)                                   (((n) & 0x3) << 26)
#define CSI_R_USER_DTYPE_BN1_MASK                                 (0x3 << 26)
#define CSI_R_USER_DTYPE_BN1_SHIFT                                (26)
#define CSI_R_USER_DTYPE_BN0(n)                                   (((n) & 0x3) << 24)
#define CSI_R_USER_DTYPE_BN0_MASK                                 (0x3 << 24)
#define CSI_R_USER_DTYPE_BN0_SHIFT                                (24)
#define CSI_R_HDLY1(n)                                   (((n) & 0xFFF) << 12)
#define CSI_R_HDLY1_MASK                                 (0xFFF << 12)
#define CSI_R_HDLY1_SHIFT                                (12)
#define CSI_R_HDLY0(n)                                   (((n) & 0xFFF) << 0)
#define CSI_R_HDLY0_MASK                                 (0xFFF << 0)
#define CSI_R_HDLY0_SHIFT                                (0)

// REG10
#define CSI_R_HDLY3(n)                                   (((n) & 0xFFF) << 12)
#define CSI_R_HDLY3_MASK                                 (0xFFF << 12)
#define CSI_R_HDLY3_SHIFT                                (12)
#define CSI_R_HDLY2(n)                                   (((n) & 0xFFF) << 0)
#define CSI_R_HDLY2_MASK                                 (0xFFF << 0)
#define CSI_R_HDLY2_SHIFT                                (0)

// REG60
#define CSI_BURST_LEN(n)                                   (((n) & 0x1F) << 24)
#define CSI_BURST_LEN_MASK                                 (0x1F << 24)
#define CSI_BURST_LEN_SHIFT                                (24)
#define CSI_IMG_PITCH(n)                                   (((n) & 0x7FFF) << 8)
#define CSI_IMG_PITCH_MASK                                 (0x7FFF << 8)
#define CSI_IMG_PITCH_SHIFT                                (8)
#define CSI_ARST_SYNC_BYPASS_EN                            (1 << 4)
#define CSI_PIX_FMT(n)                                   (((n) & 0x3) << 2)
#define CSI_PIX_FMT_MASK                                 (0x3 << 2)
#define CSI_PIX_FMT_SHIFT                                (2)
#define CSI_WDMA_ADDR_SEL                               (1 << 1)
#define CSI_DMA_ENA                                     (1 << 0)

// REG64
#define CSI_IMG_WIDTH(n)                                   (((n) & 0x1FFF) << 12)
#define CSI_IMG_WIDTH_MASK                                 (0x1FFF << 12)
#define CSI_IMG_WIDTH_SHIFT                                (12)
#define CSI_IMG_HEIGHT(n)                                   (((n) & 0xFFF) << 0)
#define CSI_IMG_HEIGHT_MASK                                 (0xFFF << 0)
#define CSI_IMG_HEIGHT_SHIFT                                (0)

// REG68
#define CSI_DMA_BASE_ADDR(n)                                   (((n) & 0xFFFFFFFF) << 0)
#define CSI_DMA_BASE_ADDR_MASK                                 (0xFFFFFFFF << 0)
#define CSI_DMA_BASE_ADDR_SHIFT                                (0)

// REG6C
#define CSI_REG_DMA_ENA_SYNC_FCLK                               (1 << 21)
#define CSI_REG_DMA_ENA_SYNC_ACLK                               (1 << 20)
#define CSI_DMAC_RST_N_PWR                               (1 << 19)
#define CSI_DMAC_RST_REQ                               (1 << 18)
#define CSI_DMAC_WR_INT_CLK                               (1 << 17)
#define CSI_DMAC_AXI_SEC                               (1 << 16)
#define CSI_DMAC_POSTWR_EN                               (1 << 15)
#define CSI_DMAC_MAX_REQ_NUM(n)                                   (((n) & 0x7) << 12)
#define CSI_DMAC_MAX_REQ_NUM_MASK                                 (0x7 << 12)
#define CSI_DMAC_MAX_REQ_NUM_SHIFT                                (12)
#define CSI_DMAC_ARQOS(n)                                   (((n) & 0xF) << 8)
#define CSI_DMAC_ARQOS_MASK                                 (0xF << 8)
#define CSI_DMAC_ARQOS_SHIFT                                (8)
#define CSI_DMAC_AWQOS(n)                                   (((n) & 0xF) << 4)
#define CSI_DMAC_AWQOS_MASK                                 (0xF << 4)
#define CSI_DMAC_AWQOS_SHIFT                                (4)
#define CSI_DMAC_USER_ID(n)                                   (((n) & 0xF) << 0)
#define CSI_DMAC_USER_ID_MASK                                 (0xF << 0)
#define CSI_DMAC_USER_ID_SHIFT                                (0)

// REG80
#define CSI_DMA_BASE_ADDR1(n)                                   (((n) & 0xFFFFFFFF) << 0)
#define CSI_DMA_BASE_ADDR1_MASK                                 (0xFFFFFFFF << 0)
#define CSI_DMA_BASE_ADDR1_SHIFT                                (0)

#endif /* __CSI_REG_H__ */

