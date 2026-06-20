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
#ifndef __MIPIPHY_H__
#define __MIPIPHY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_addr_map.h"
#include CHIP_SPECIFIC_HDR(mipiphy)
#include "hal_phyif.h"

#ifdef SPIPHY_MIPI_REG
#define SPI_MIPI_REG(reg)           SPIPHY_MIPI_REG(reg)
#else
#define SPI_MIPI_REG(reg)           (reg)
#endif

#define mipiphy_read(reg, val)      hal_phyif_reg_read(SPI_MIPI_REG(reg), val)
#define mipiphy_write(reg, val)     hal_phyif_reg_write(SPI_MIPI_REG(reg), val)

void dsiphy_open(uint32_t clk);

void dsiphy_sleep(void);

void dsiphy_wakeup(void);

void csiphy_open(uint32_t clk);

void csiphy_sleep(void);

void csiphy_wakeup(void);

#ifdef __cplusplus
}
#endif

#endif

