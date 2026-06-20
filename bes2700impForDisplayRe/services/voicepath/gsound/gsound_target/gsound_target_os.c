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
#include <stdbool.h>
#include <stdint.h>

#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "customparam_section.h"
#include "gsound_target.h"

//
// Target OS functions.
//
static int mutex_index = 0;

osMutexDef(mutexes_0);
osMutexDef(mutexes_1);
osMutexDef(mutexes_2);

static int semaphore_index = 0;

osSemaphoreDef(semaphores_0);
osSemaphoreDef(semaphores_1);

static void mq_thread(const void *arg);
#ifndef GSOUND_MQ_THREAD_SIZE
#define GSOUND_MQ_THREAD_SIZE  (2048)
#endif
osThreadDef(mq_thread, osPriorityAboveNormal, 1, GSOUND_MQ_THREAD_SIZE, "mq_thread");
static osThreadId gsound_mq_thread_id;

GSoundStatus GSoundTargetOsMutexReserve(GSoundTargetMutexHandle *id)
{
    ASSERT(mutex_index < GSOUND_TARGET_OS_MUTEX_MAX,
           "Too many mutexes reserved.");

    switch (mutex_index)
    {
    case 0:
        *id = (GSoundTargetMutexHandle)osMutexCreate((osMutex(mutexes_0)));
        break;
    case 1:
        *id = (GSoundTargetMutexHandle)osMutexCreate((osMutex(mutexes_1)));
        break;
    case 2:
        *id = (GSoundTargetMutexHandle)osMutexCreate((osMutex(mutexes_2)));
        break;
    default:
        break;
    }

    mutex_index += 1;

    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetOsMutexWait(GSoundTargetMutexHandle id)
{
    osMutexWait((osMutexId)id, osWaitForever);

    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetOsMutexRelease(GSoundTargetMutexHandle id)
{
    osMutexRelease((osMutexId)id);

    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetOsSemaphoreReserve(GSoundTargetSemaphoreHandle *id,
                                            uint32_t max_count,
                                            uint32_t initial_count)
{
    ASSERT(semaphore_index < GSOUND_TARGET_OS_SEMAPHORE_MAX,
           "Too many semaphores reserved.");

    switch (semaphore_index)
    {
    case 0:
        *id = (GSoundTargetSemaphoreHandle)osSemaphoreCreate((osSemaphore(semaphores_0)), 0);
        break;
    case 1:
        *id = (GSoundTargetSemaphoreHandle)osSemaphoreCreate((osSemaphore(semaphores_1)), 0);
        break;
    default:
        break;
    }

    semaphore_index += 1;

    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetOsSemaphoreWait(GSoundTargetSemaphoreHandle id)
{
    int32_t tokens = osSemaphoreWait((osSemaphoreId)id, osWaitForever);

    return tokens >= 0 ? GSOUND_STATUS_OK : GSOUND_STATUS_ERROR;
}

GSoundStatus GSoundTargetOsSemaphorePost(GSoundTargetSemaphoreHandle id)
{
    osStatus status = osSemaphoreRelease((osSemaphoreId)id);

    return status == osOK ? GSOUND_STATUS_OK : GSOUND_STATUS_ERROR;
}

static void mq_thread(const void *arg)
{
    ((GSoundTargetOsThreadEntry)arg)();
}

bool GSoundTargetOsMqActive(void)
{
    return osThreadGetId() == gsound_mq_thread_id;
}

GSoundStatus GSoundTargetOsThreadInit(GSoundTargetOsThreadEntry entry)
{
    gsound_mq_thread_id = osThreadCreate(osThread(mq_thread), entry);

    return GSOUND_STATUS_OK;
}

uint32_t GSoundTargetOsIntLock(void)
{
    return int_lock();
}

void GSoundTargetOsIntRevert(uint32_t int_status)
{
    int_unlock(int_status);
}
