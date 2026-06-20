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
#ifndef __HAL_TIMER_USER_IRQ_H__
#define __HAL_TIMER_USER_IRQ_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_timer.h"

void hal_user_timer0_setup(enum HAL_TIMER_TYPE_T type, HAL_TIMER_IRQ_HANDLER_T handler);

void hal_user_timer0_start(uint32_t load);

void hal_user_timer0_stop(void);

void hal_user_timer0_continue(void);

int hal_user_timer0_is_enabled(void);

void hal_user_timer0_reload(uint32_t load);

uint32_t hal_user_timer0_get_raw_value(void);

int hal_user_timer0_irq_active(void);

int hal_user_timer0_irq_pending(void);

uint32_t hal_user_timer0_get_elapsed_time(void);

void hal_user_timer1_setup(enum HAL_TIMER_TYPE_T type, HAL_TIMER_IRQ_HANDLER_T handler);

void hal_user_timer1_start(uint32_t load);

void hal_user_timer1_stop(void);

void hal_user_timer1_continue(void);

int hal_user_timer1_is_enabled(void);

void hal_user_timer1_reload(uint32_t load);

uint32_t hal_user_timer1_get_raw_value(void);

int hal_user_timer1_irq_active(void);

int hal_user_timer1_irq_pending(void);

uint32_t hal_user_timer1_get_elapsed_time(void);

#ifdef __cplusplus
}
#endif

#endif
