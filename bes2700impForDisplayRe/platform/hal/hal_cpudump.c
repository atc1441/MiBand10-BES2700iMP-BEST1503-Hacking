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

#ifdef CPUDUMP_BASE

#include "hal_cmu.h"
#include "hal_cpudump.h"
#include "hal_timer.h"

#define CPUDUMP_EN_REG_OFFSET      0x2000
#define CPUDUMP_DISABLE            (0 << 0)
#define CPUDUMP_ENABLE             (1 << 0)
#define CPUDUMP_LR_ENABLE          (1 << 1)
#define CPUDUMP_CTRL_REG_OFFSET    0x2004
#define CPUDUMP_CTRL_STOP          (1 << 0)
#define CPUDUMP_CTRL_START         (1 << 1)


#define cpudump_read32(b,a) \
     (*(volatile uint32_t *)(b+a))

#define cpudump_write32(v,b,a) \
     ((*(volatile uint32_t *)(b+a)) = v)
#ifndef CHIP_SUBSYS_SENS
void hal_cpudump_clk_enable(void)
{
/* sensor do not support cpudump now */
#ifndef CHIP_SUBSYS_SENS
    hal_cmu_clock_enable(HAL_CMU_MOD_X_CPUDUMP);
    hal_cmu_reset_clear(HAL_CMU_MOD_X_CPUDUMP);
#endif
}

void hal_cpudump_clk_disable(void)
{
#ifndef CHIP_SUBSYS_SENS
    hal_cmu_reset_set(HAL_CMU_MOD_X_CPUDUMP);
    hal_cmu_clock_disable(HAL_CMU_MOD_X_CPUDUMP);
#endif
}
#endif
void hal_cpudump_enable(void)
{
    uint32_t val;

    val = CPUDUMP_ENABLE;
#ifdef CPU_PC_DUMP_LR
    val |= CPUDUMP_LR_ENABLE;
#endif
    cpudump_write32(val, CPUDUMP_BASE, CPUDUMP_EN_REG_OFFSET);
    val = CPUDUMP_CTRL_START;
    cpudump_write32(val, CPUDUMP_BASE, CPUDUMP_CTRL_REG_OFFSET);
}

void hal_cpudump_disable(void)
{
    uint32_t val;

    val = CPUDUMP_CTRL_STOP;
    cpudump_write32(val, CPUDUMP_BASE, CPUDUMP_CTRL_REG_OFFSET);
    hal_sys_timer_delay(MS_TO_TICKS(1));
    val = CPUDUMP_DISABLE;
    cpudump_write32(val, CPUDUMP_BASE, CPUDUMP_EN_REG_OFFSET);
}

uint32_t hal_cpudump_get_last_pc_addr(void)
{
    return hal_cmu_cpudump_get_last_pc_addr();
}
#endif
