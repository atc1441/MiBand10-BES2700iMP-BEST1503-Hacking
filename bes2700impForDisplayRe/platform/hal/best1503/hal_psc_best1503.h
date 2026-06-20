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
#ifndef __HAL_PSC_BEST1306_H__
#define __HAL_PSC_BEST1306_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "hal_gpio.h"

enum HAL_PSC_DISPLAY_MOD_T {
    HAL_PSC_DISPLAY_MOD_IMGDMA = 0,
    HAL_PSC_DISPLAY_MOD_GPU,
    HAL_PSC_DISPLAY_MOD_LCDC,
    HAL_PSC_DISPLAY_MOD_QSPI,
    HAL_PSC_DISPLAY_MOD_DSI,

    HAL_PSC_DISPLAY_MOD_QTY,
    HAL_PSC_DISPLAY_MOD_ALL = HAL_PSC_DISPLAY_MOD_QTY,
};

void hal_psc_display_enable(enum HAL_PSC_DISPLAY_MOD_T mod);

void hal_psc_display_disable(enum HAL_PSC_DISPLAY_MOD_T mod);

void hal_psc_gpio_irq_enable(enum HAL_GPIO_PIN_T pin);

void hal_psc_gpio_irq_disable(enum HAL_GPIO_PIN_T pin);

uint32_t hal_psc_gpio_irq_get_status(uint32_t *status, uint32_t cnt);

uint32_t hal_psc_get_power_loop_cycle_cnt(void);

#ifdef __cplusplus
}
#endif

#endif

