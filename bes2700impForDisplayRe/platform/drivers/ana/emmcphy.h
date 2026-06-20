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
#ifndef __EMMCPHY_H__
#define __EMMCPHY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"
#include "plat_addr_map.h"
#include CHIP_SPECIFIC_HDR(emmcphy)
#include "hal_analogif.h"
#include "hal_phyif.h"

#define ISPI_EMMCPHY_REG(reg)            EMMC_REG(reg)

#define emmcphy_read(reg, val)           hal_analogif_reg_read(ISPI_EMMCPHY_REG(reg), val)
#define emmcphy_write(reg, val)          hal_analogif_reg_write(ISPI_EMMCPHY_REG(reg), val)

void emmcphy_open(void);

void emmcphy_close(void);

void emmcphy_hs_host_mode_enable(void);

void emmcphy_hs_host_mode_disable(void);

void emmcphy_sleep(void);

void emmcphy_wakeup(void);

void emmcphy_pin_pull_up_enable(void);

void emmcphy_pin_pull_up_disable(void);

void emmcphy_pin_set_io_drv(uint8_t drv);

void emmcphy_clk_sel_pck(void);

void emmcphy_clk_sel_pck_delay(void);

void emmcphy_clk_sel_ds_delay(void);

void emmcphy_tuning_correct(void);

void emmcphy_pck_reverse(void);

void emmcphy_rd_tune_dly_cali(uint8_t cali_val);

#ifdef __cplusplus
}
#endif

#endif

