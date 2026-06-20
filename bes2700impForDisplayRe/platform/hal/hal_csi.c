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
#include "plat_addr_map.h"
#if defined(CSI_BASE)
#include "plat_types.h"
#include "hal_cmu.h"
#include "hal_csi.h"
#include "reg_csi.h"
#include "mipiphy.h"
#include "hal_trace.h"

#define GC2145MIPI_2LANE

static struct CSI_REG_T * const csi = (struct CSI_REG_T *)CSI_BASE;

void hal_csi_init(void)
{
    hal_cmu_csi_clock_enable();
    hal_cmu_csi_reset_clear();

#ifndef FPGA
    csiphy_open(0);
#endif

    csi->REG_00 = CSI_R_PWR_INI_DELAY(0xc0) | CSI_R_HS_PRPR_TIME(0x8) | CSI_R_CKLANE_HS_EN |
#ifdef GC2145MIPI_2LANE
    CSI_R_LANE_NUM(1)
#else
    CSI_R_LANE_NUM(0)
#endif
    ;
    csi->REG_0C = CSI_R_USER_DTYPE_BN3(0x2) | CSI_R_USER_DTYPE_BN2(0x2) |
        CSI_R_USER_DTYPE_BN1(0x2) | CSI_R_USER_DTYPE_BN0(0x2) |
        CSI_R_HDLY1(0x1A0) | CSI_R_HDLY0(0x30F);
}

void hal_csi_start(enum CSI_PIX_FMT_T fmt, uint32_t width, uint32_t height, uint32_t addr)
{
    uint32_t pitch;
    if (fmt == CSI_PIX_FMT_YUV422) {
        pitch = width * 2;
    } else if (fmt == CSI_PIX_FMT_RBB565) {
        pitch = width * 2;
    } else if (fmt == CSI_PIX_FMT_RGB888) {
        pitch = width * 3;
    } else if (fmt == CSI_PIX_FMT_RGBA888) {
        pitch = width * 4;
    } else {
        ASSERT(0, "unsupport fmt");
    }

    csi->REG_68 = CSI_DMA_BASE_ADDR(addr);
    csi->REG_80 = CSI_DMA_BASE_ADDR(0x50100000);
    csi->REG_7C = 0;

    csi->REG_64 = CSI_IMG_HEIGHT(height) | CSI_IMG_WIDTH(width);
    csi->REG_6C = CSI_DMAC_AWQOS(0x4) | CSI_DMAC_ARQOS(0x4) |
        CSI_DMAC_MAX_REQ_NUM(0x4) | CSI_DMAC_RST_N_PWR |
        CSI_REG_DMA_ENA_SYNC_ACLK | CSI_REG_DMA_ENA_SYNC_FCLK;
    csi->REG_60 = CSI_IMG_PITCH(pitch) | CSI_BURST_LEN(0x8) | CSI_PIX_FMT(fmt) |
        //CSI_WDMA_ADDR_SEL | //ping-pong mode
        CSI_DMA_ENA;
}
void hal_csi_stop(void)
{
    csi->REG_60 &= ~CSI_DMA_ENA;
}

void hal_csi_restart(void)
{
    csi->REG_60 |= CSI_DMA_ENA;
}

#endif /*CSI_BASE*/
