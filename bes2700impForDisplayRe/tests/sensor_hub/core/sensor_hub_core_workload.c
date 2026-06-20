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
#ifdef FULL_WORKLOAD_MODE_ENABLED
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "string.h"
#include "hal_mcu2sens.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "sens_mem.h"
#include "sensor_hub_core_workload.h"

static void sensor_hub_full_workload_thread(const void *arg);
static osThreadId workload_thread_id = NULL;
osThreadDef(sensor_hub_full_workload_thread, osPriorityNormal, 1,
    APP_THREAD_FULL_WORKLOAD_STACK_SIZE, "full_workload_thread");

osMutexId workload_mutex_id = NULL;
osMutexDef(workload_mutex);

#define workload_mutex_lock()   {osMutexWait(workload_mutex_id, osWaitForever);}
#define workload_mutex_unlock() {osMutexRelease(workload_mutex_id);}

typedef struct {
    WORKLOAD_FUNCTION_T func;
    void *param;
} workload_func_t;

static uint32_t workload_enable_map = 0;
static bool isSensorHubInFullWorkloadMode = false;
static workload_func_t workload_list[WORKLOAD_ID_QTY];

static void sensor_hub_init_workload(void)
{
    memset((void *)workload_list, 0, sizeof(workload_func_t)*(ARRAY_SIZE(workload_list)));
    workload_enable_map = 0;
}

int sensor_hub_add_workload(enum WORKLOAD_ID_T id, WORKLOAD_FUNCTION_T func, void *param)
{
    workload_func_t *l = &workload_list[id];

    TRACE(1, "%s: id=%d,func=%x,param=%x", __func__, id, (int)func, (int)param);
    ASSERT(id < WORKLOAD_ID_QTY, "%s: invalid id=%d", __func__, id);

    workload_mutex_lock();

    l->func = func;
    l->param = param;

    workload_mutex_unlock();
    return 0;
}

int sensor_hub_remove_workload(enum WORKLOAD_ID_T id)
{
    workload_func_t *l = &workload_list[id];

    TRACE(1, "%s: id=%d", __func__, id);
    ASSERT(id < WORKLOAD_ID_QTY, "%s: invalid id=%d", __func__, id);

    workload_mutex_lock();
    l->func = NULL;
    l->param = NULL;
    if (workload_enable_map & (1<<id)) {
        workload_enable_map &= ~(1<<id);
    }
    workload_mutex_unlock();
    return 0;
}

void sensor_hub_enable_workload(enum WORKLOAD_ID_T id)
{
    workload_func_t *l = &workload_list[id];

    TRACE(1, "%s: id=%d", __func__, id);
    ASSERT(id < WORKLOAD_ID_QTY, "%s: invalid id=%d", __func__, id);

    workload_mutex_lock();
    if (l->func) {
        workload_enable_map |= (1<<id);
    }
    if (workload_enable_map > 0) {
        sensor_hub_enter_full_workload_mode();
    }
    workload_mutex_unlock();
}

void sensor_hub_disable_workload(enum WORKLOAD_ID_T id)
{
    workload_func_t *l = &workload_list[id];

    TRACE(1, "%s: id=%d", __func__, id);
    ASSERT(id < WORKLOAD_ID_QTY, "%s: invalid id=%d", __func__, id);

    workload_mutex_lock();
    if (l->func) {
        workload_enable_map &= ~(1<<id);
    }
    if (workload_enable_map == 0) {
        sensor_hub_exit_full_workload_mode();
    }
    workload_mutex_unlock();
}

static void sensor_hub_run_workload(void)
{
    uint32_t i;

    workload_mutex_lock();

    for (i = 0; i < ARRAY_SIZE(workload_list); i++) {
        uint32_t lock;

        lock = int_lock();
        uint32_t map = workload_enable_map;
        WORKLOAD_FUNCTION_T func = workload_list[i].func;
        void *param = workload_list[i].param;
        int_unlock(lock);

        if ((func) && (map & (1<<i))) {
            func(param);
        }
    }

    workload_mutex_unlock();
}

void sensor_hub_enter_full_workload_mode(void)
{
    isSensorHubInFullWorkloadMode = true;
}

void sensor_hub_exit_full_workload_mode(void)
{
    isSensorHubInFullWorkloadMode = false;
}

static void sensor_hub_full_workload_thread(const void *arg)
{
    while (1) {
        if (!isSensorHubInFullWorkloadMode) {
            osDelay(APP_THREAD_FULL_WORKLOAD_DLY);
        } else {
            sensor_hub_run_workload();
        }
    }
}

void sensor_hub_init_workload_thread(void)
{
    sensor_hub_init_workload();
    if (workload_mutex_id == NULL) {
        workload_mutex_id = osMutexCreate(osMutex(workload_mutex));
    }
    if (workload_thread_id == NULL) {
        workload_thread_id = osThreadCreate(osThread(sensor_hub_full_workload_thread), NULL);
    }
}
#endif
