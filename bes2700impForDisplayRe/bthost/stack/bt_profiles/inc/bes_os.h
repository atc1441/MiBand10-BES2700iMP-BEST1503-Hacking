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
#include <stdio.h>
#include <stdbool.h>
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "cmsis.h"
#include "hal_trace.h"
#include "hal_timer.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define OS_CRITICAL_METHOD      0
#define OS_ENTER_CRITICAL()     uint32_t os_lock = bt_int_lock()
#define OS_EXIT_CRITICAL()      bt_int_unlock(os_lock)
#define OSTimeDly(a)            osDelay((a)*2)

void osapi_init(void);
void osapi_stop_hardware(void);
void osapi_resume_hardware(void);
void osapi_lock_stack(void);
void osapi_unlock_stack(void);
void osapi_notify_evm(void);
int osapi_lock_is_exist(void);

void Plt_TimerWrapperInit(void);
void Plt_TimerWrapperStart(unsigned int ms);
void Plt_TimerWrapperStop(void);
void Plt_UNLockTimerWrapper(void);
void Plt_LockTimerWrapper(void);
unsigned int Plt_GetTicks(void);
unsigned int Plt_GetTicksMax(void);

void OS_LockStack_Info_Print(void);

#if defined(__cplusplus)
}
#endif