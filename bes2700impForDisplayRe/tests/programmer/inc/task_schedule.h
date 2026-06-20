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
#ifndef __TASK_SCHEDULE_H__
#define __TASK_SCHEDULE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"

#ifdef RTOS
#ifndef NO_SIMPLE_TASK_SWITCH
#define NO_SIMPLE_TASK_SWITCH
#endif
#endif

#if defined(PROGRAMMER) && !defined(NO_SIMPLE_TASK_SWITCH)
#define SIMPLE_TASK_SWITCH
#endif

//-------------------------------------------------------
// Types

enum TASK_STATE_T {
    TASK_STATE_NONE,
    TASK_STATE_ACTIVE,
    TASK_STATE_PENDING,
    TASK_STATE_SLEEPING,
};

typedef void (*TASK_ENTRY_T)(void);

//-------------------------------------------------------
// Functions

#ifdef SIMPLE_TASK_SWITCH

#define TASK_SCHEDULE                   task_yield()

int task_setup(uint8_t id, uint32_t sp, uint32_t sp_limit, enum TASK_STATE_T state, TASK_ENTRY_T entry);

bool task_yield(void);

bool task_sleep(void);

bool task_wakeup(uint8_t id);

#else // !SIMPLE_TASK_SWITCH

#define TASK_SCHEDULE                   true

static inline int task_setup(uint8_t id, uint32_t sp, uint32_t sp_limit, enum TASK_STATE_T state, TASK_ENTRY_T entry)
{
    return 0;
}

static inline bool task_yield(void)
{
    return true;
}

static inline bool task_sleep(void)
{
    return true;
}

static inline bool task_wakeup(uint8_t id)
{
    return true;
}

#endif // !SIMPLE_TASK_SWITCH

#ifdef __cplusplus
}
#endif

#endif

