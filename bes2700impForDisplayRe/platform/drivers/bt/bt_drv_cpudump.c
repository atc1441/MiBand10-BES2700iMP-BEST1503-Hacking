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
#include "plat_addr_map.h"

#ifdef BTC_CPUDUMP_BASE

#include "hal_cmu.h"
#include "bt_drv_cpudump.h"
#include "hal_timer.h"

#define BTC_CPUDUMP_EN_REG_OFFSET      0x2000
#define BTC_CPUDUMP_DISABLE            (0 << 0)
#define BTC_CPUDUMP_ENABLE             (1 << 0)
#define BTC_CPUDUMP_CTRL_REG_OFFSET    0x2004
#define BTC_CPUDUMP_CTRL_STOP          (1 << 0)
#define BTC_CPUDUMP_CTRL_START         (1 << 1)


#define btc_cpudump_read32(b,a) \
     (*(volatile uint32_t *)(b+a))

#define btc_cpudump_write32(v,b,a) \
     ((*(volatile uint32_t *)(b+a)) = v)

void btdrv_cpudump_clk_enable(void)
{
    //hal_cmu_clock_enable(HAL_CMU_MOD_X_CPUDUMP);
    //hal_cmu_reset_clear(HAL_CMU_MOD_X_CPUDUMP);
}

void btdrv_cpudump_clk_disable(void)
{
    //hal_cmu_reset_set(HAL_CMU_MOD_X_CPUDUMP);
    //hal_cmu_clock_disable(HAL_CMU_MOD_X_CPUDUMP);
}

void btdrv_cpudump_enable(void)
{
    uint32_t val;

    val = BTC_CPUDUMP_ENABLE;
    btc_cpudump_write32(val, BTC_CPUDUMP_BASE, BTC_CPUDUMP_EN_REG_OFFSET);
    val = BTC_CPUDUMP_CTRL_START;
    btc_cpudump_write32(val, BTC_CPUDUMP_BASE, BTC_CPUDUMP_CTRL_REG_OFFSET);
}

void btdrv_cpudump_disable(void)
{
    uint32_t val;

    val = BTC_CPUDUMP_CTRL_STOP;
    btc_cpudump_write32(val, BTC_CPUDUMP_BASE, BTC_CPUDUMP_CTRL_REG_OFFSET);
    hal_sys_timer_delay(MS_TO_TICKS(1));
    val = BTC_CPUDUMP_DISABLE;
    btc_cpudump_write32(val, BTC_CPUDUMP_BASE, BTC_CPUDUMP_EN_REG_OFFSET);
}
#endif
