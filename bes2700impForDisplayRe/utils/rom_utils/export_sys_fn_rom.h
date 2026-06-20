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
#ifndef __EXPORT_SYS_FN_ROM_H__
#define __EXPORT_SYS_FN_ROM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stddef.h"
#include "stdint.h"
#include "stdarg.h"
#include "stdbool.h"

typedef int (*__HAL_TRACE_PRINTF)(uint32_t attr, const char *fmt, va_list ap);
typedef int (*__HAL_TRACE_DUMP)(const char *fmt, unsigned int size,  unsigned int count, const void *buffer);
typedef int (*__HAL_TRACE_OUTPUT)(const unsigned char *buf, unsigned int buf_len);

typedef uint32_t (*__HAL_SYS_TIMER_GET)(void);
typedef uint32_t (*__HAL_SYS_TIMER_GET_MAX)(void);

typedef int (*__INT_LOCK)();
typedef void (*__INT_UNLOCK)();

typedef osThreadId (*__OS_THREAD_CREATE)(const osThreadDef_t *thread_def, void *argument);

typedef osStatus (*__OS_DELAY)(uint32_t millisec);

typedef osStatus_t (*__OS_MUTEX_ACQUIRE)(osMutexId_t mutex_id, uint32_t timeout);
typedef osMutexId (*__OS_MUTEX_CREATE)(const osMutexDef_t *mutex_def);
typedef osStatus_t (*__OS_MUTEX_RELEASE)(osMutexId_t mutex_id);

typedef osSemaphoreId (*__OS_SEMAPHORE_CREATE)(const osSemaphoreDef_t *semaphore_def, int32_t count);
typedef int32_t (*__OS_SEMAPHORE_WAIT)(osSemaphoreId semaphore_id, uint32_t millisec);

typedef osTimerId (*__OS_TIMER_CREATE)(const osTimerDef_t *timer_def, os_timer_type type, void *argument);
typedef osStatus_t (*__OS_TIMER_START)(osTimerId_t timer_id, uint32_t ticks);
typedef uint32_t (*__OS_TIMER_IS_RUNNING)(osTimerId_t timer_id);
typedef osStatus_t (*__OS_TIMER_STOP)(osTimerId_t timer_id);
typedef osStatus_t (*__OS_TIMER_DEL)(osTimerId_t timer_id);

typedef osMessageQId (*__OS_MSG_CREATE)(const osMessageQDef_t *queue_def, osThreadId thread_id);
typedef uint32_t (*__OS_MSG_GET_SPACE)(osMessageQId queue_id);
typedef osEvent (*__OS_MSG_GET)(osMessageQId queue_id, uint32_t millisec);
typedef osStatus (*__OS_MSG_PUT)(osMessageQId queue_id, uint32_t info, uint32_t millisec);

typedef bool (*__OSIF_INIT)(void);
typedef void (*__OSIF_STOP_HARDWARE)(void);
typedef void (*__OSIF_RESUME_HARDWARE)(void);
typedef void (*__OSIF_LOCK_STACK)(void);
typedef void (*__OSIF_UNLOCK_STACK)(void);
typedef void (*__OSIF_NOTIFY_EVM)(void);
typedef uint8_t (*__OSIF_LOCK_IS_EXIST)(void);

typedef int32_t (*__OS_SIGNAL_SET)(osThreadId thread_id, int32_t signals);
typedef osEvent (*__OS_SIGNAL_WAIT)(int32_t signals, uint32_t millisec);


typedef void (*__BT_DRV_DIGITAL_CONFIG_FOR_BLE_ADV)(bool en);
typedef void (*BT_DRV_LOAD_SLEEP_CONFIG)(uint8_t* config);

typedef int (*__APP_SYSFREQ_REQ)(enum APP_SYSFREQ_USER_T user, enum APP_SYSFREQ_FREQ_T freq);

struct EXPORT_SYS_FN_ROM_T {
    void *reserved;

    __HAL_TRACE_DUMP hal_trace_dump;
    __HAL_TRACE_PRINTF hal_trace_printf;
    __HAL_TRACE_OUTPUT hal_trace_output;

    __HAL_SYS_TIMER_GET hal_sys_timer_get;
    __HAL_SYS_TIMER_GET_MAX hal_sys_timer_get_max;

    __INT_LOCK    int_lock;
    __INT_UNLOCK  int_unlock;

    __OS_THREAD_CREATE osThreadCreate;
    __OS_DELAY         osDelay;

    __OS_MUTEX_ACQUIRE osMutexAcquire;
    __OS_MUTEX_CREATE  osMutexCreate;
    __OS_MUTEX_RELEASE osMutexRelease;

    __OS_SEMAPHORE_CREATE osSemaphoreCreate;
    __OS_SEMAPHORE_WAIT osSemaphoreWait;
    __OS_TIMER_CREATE  osTimerCreate;
    __OS_TIMER_IS_RUNNING osTimerIsRunning;
    __OS_TIMER_START   osTimerStart;
    __OS_TIMER_STOP    osTimerStop;
    __OS_TIMER_DEL     osTimerDelete;

    __OS_MSG_CREATE    osMessageCreate;
    __OS_MSG_GET_SPACE osMessageGetSpace;
    __OS_MSG_GET       osMessageGet;
    __OS_MSG_PUT       osMessagePut;

    __OS_SIGNAL_SET   osSignalSet;
    __OS_SIGNAL_WAIT  osSignalWait;

    __OSIF_INIT            osif_init;
    __OSIF_STOP_HARDWARE   osif_stop_hardware;
    __OSIF_RESUME_HARDWARE osif_resume_hardware;
    __OSIF_LOCK_STACK      osif_lock_stack;
    __OSIF_UNLOCK_STACK    osif_unlock_stack;
    __OSIF_NOTIFY_EVM      osif_notify_evm;
    __OSIF_LOCK_IS_EXIST   osif_lock_is_exist;

    __BT_DRV_DIGITAL_CONFIG_FOR_BLE_ADV  bt_drv_digital_config_for_ble_adv;
    BT_DRV_LOAD_SLEEP_CONFIG             btdrv_load_sleep_config;

    __APP_SYSFREQ_REQ  app_sysfreq_req;
};

void export_register_sys_fn(const struct EXPORT_SYS_FN_ROM_T* fn);

#ifdef __cplusplus
}
#endif

#endif /* __EXPORT_NV_FN_ROM_H__ */

