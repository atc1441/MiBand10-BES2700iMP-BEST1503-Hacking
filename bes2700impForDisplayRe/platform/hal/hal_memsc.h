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
#ifndef __HAL_MEMSC_H__
#define __HAL_MEMSC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"

enum HAL_MEMSC_ID_T {
    HAL_MEMSC_ID_0,
    HAL_MEMSC_ID_1,
    HAL_MEMSC_ID_2,
    HAL_MEMSC_ID_3,

    HAL_MEMSC_ID_QTY
};

#define HAL_MEMSC_ID_TRACE              HAL_MEMSC_ID_0
#define HAL_MEMSC_ID_TRNG               HAL_MEMSC_ID_1
#if defined(__NuttX__)
#define HAL_MEMSC_ID_SPINLOCK           HAL_MEMSC_ID_2
#endif

enum HAL_MEMSC_AON_ID_T {
    HAL_MEMSC_AON_ID_0,
    HAL_MEMSC_AON_ID_1,
    HAL_MEMSC_AON_ID_2,
    HAL_MEMSC_AON_ID_3,

    HAL_MEMSC_AON_ID_QTY
};

int hal_memsc_lock(enum HAL_MEMSC_ID_T id);

void hal_memsc_unlock(enum HAL_MEMSC_ID_T id);

bool hal_memsc_avail(enum HAL_MEMSC_ID_T id);

int hal_memsc_aon_lock(enum HAL_MEMSC_AON_ID_T id);

void hal_memsc_aon_unlock(enum HAL_MEMSC_AON_ID_T id);

bool hal_memsc_aon_avail(enum HAL_MEMSC_AON_ID_T id);

#ifdef __cplusplus
}
#endif

#endif

