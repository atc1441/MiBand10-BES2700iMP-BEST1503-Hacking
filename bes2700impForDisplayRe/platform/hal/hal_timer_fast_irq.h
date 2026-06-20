/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifndef __HAL_TIMER_FAST_IRQ_H__
#define __HAL_TIMER_FAST_IRQ_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_timer.h"

void hal_fast_timer_setup(enum HAL_TIMER_TYPE_T type, HAL_TIMER_IRQ_HANDLER_T handler);

void hal_fast_timer_start(uint32_t load);

void hal_fast_timer_stop(void);

void hal_fast_timer_pause(void);

void hal_fast_timer_continue(void);

int hal_fast_timer_is_enabled(void);

void hal_fast_timer_reload(uint32_t load);

uint32_t hal_fast_timer_get_raw_value(void);

int hal_fast_timer_irq_active(void);

int hal_fast_timer_irq_pending(void);

uint32_t hal_fast_timer_get_elapsed_time(void);

uint32_t hal_fast_timer_get_load(void);

uint32_t hal_fast_timer_get_count(void);

void hal_fast_timer_bgload(uint32_t load);

#ifdef __cplusplus
}
#endif

#endif
