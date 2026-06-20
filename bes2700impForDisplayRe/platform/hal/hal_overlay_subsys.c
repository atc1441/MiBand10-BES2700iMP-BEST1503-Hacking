
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
#include "plat_types.h"
#include "hal_overlay.h"
#include "hal_sysfreq.h"
#include "hal_cache.h"
#include "hal_trace.h"
#include "cmsis.h"
#ifdef __ARMCC_VERSION
#include "link_sym_armclang.h"
#endif
#include "hal_overlay_subsys.h"

//TODO:only support m55 currently, hifi4 will be later
#ifndef NO_OVERLAY

static uint32_t *__overlay_text_start__;

static uint32_t *__overlay_data_start__;


#define OVERLAY_IS_FREE  0
#define OVERLAY_IS_USED  1

static bool segment_state[HAL_OVERLAY_ID_QTY];

static enum HAL_OVERLAY_ID_T cur_overlay_id = HAL_OVERLAY_ID_QTY;

static uint32_t * text_load_start[HAL_OVERLAY_ID_QTY];

static uint32_t * text_load_stop[HAL_OVERLAY_ID_QTY];

static uint32_t * data_load_start[HAL_OVERLAY_ID_QTY];

static uint32_t * data_load_stop[HAL_OVERLAY_ID_QTY];

enum HAL_OVERLAY_RET_T hal_overlay_subsys_set_text_exec_addr(
                    enum OVERLAY_SUBSYS_T chip_id, uint32_t text_vma,
                    uint32_t text_size, uint32_t data_vma, uint32_t data_size)
{
    enum HAL_OVERLAY_RET_T ret;

    ret = HAL_OVERLAY_RET_OK;

  __overlay_text_start__ = (uint32_t *)text_vma;
  __overlay_data_start__ = (uint32_t *)data_vma;

  return ret;
}

enum HAL_OVERLAY_RET_T hal_overlay_subsys_set_text_addr(enum OVERLAY_SUBSYS_T chip_id,
                        enum HAL_OVERLAY_ID_T id, uint32_t start, uint32_t stop)
{
    enum HAL_OVERLAY_RET_T ret;

    CHECK_OVERLAY_ID(id);
    ret = HAL_OVERLAY_RET_OK;

    text_load_start[id] = (uint32_t *)start;
    text_load_stop[id] = (uint32_t *)stop;

    return ret;
}

enum HAL_OVERLAY_RET_T hal_overlay_subsys_set_data_addr(enum OVERLAY_SUBSYS_T chip_id,
                        enum HAL_OVERLAY_ID_T id, uint32_t start, uint32_t stop)
{
    enum HAL_OVERLAY_RET_T ret;

    CHECK_OVERLAY_ID(id);
    ret = HAL_OVERLAY_RET_OK;

    data_load_start[id] = (uint32_t *)start;
    data_load_stop[id] =  (uint32_t *)stop;

    return ret;
}

/*must called by lock get */
static void invalid_overlay_cache(enum HAL_OVERLAY_ID_T id)
{
    return;
}

static bool hal_overlay_subsys_is_used(void);
static void hal_overlay_subsys_acquire(enum HAL_OVERLAY_ID_T id);
static void hal_overlay_subsys_release(enum HAL_OVERLAY_ID_T id);

enum HAL_OVERLAY_RET_T hal_overlay_subsys_load(enum OVERLAY_SUBSYS_T chip_id,
                                                enum HAL_OVERLAY_ID_T id)
{
    enum HAL_OVERLAY_RET_T ret;

    CHECK_OVERLAY_ID(id);
    ret = HAL_OVERLAY_RET_OK;

    uint32_t lock;
    uint32_t *dst, *src;

    if (hal_overlay_subsys_is_used()) {
        ASSERT(0, "overlay %d is in use", id);
        return HAL_OVERLAY_RET_IN_USE;
    }

    hal_sysfreq_req(HAL_SYSFREQ_USER_OVERLAY_SUBSYS, HAL_CMU_FREQ_52M);
    lock = int_lock();

    if (cur_overlay_id == HAL_OVERLAY_ID_QTY) {
        cur_overlay_id = HAL_OVERLAY_ID_IN_CFG;
    } else if (cur_overlay_id == HAL_OVERLAY_ID_IN_CFG) {
        ret = HAL_OVERLAY_RET_IN_CFG;
    } else {
        ret = HAL_OVERLAY_RET_IN_USE;
    }

    if (ret != HAL_OVERLAY_RET_OK) {
        goto _exit;
    }

    hal_overlay_subsys_acquire(id);

    for (dst = __overlay_text_start__, src = text_load_start[id];
            src < text_load_stop[id];
            dst++, src++) {
        *dst = *src;
    }

    for (dst = __overlay_data_start__, src = data_load_start[id];
            src < data_load_stop[id];
            dst++, src++) {
        *dst = *src;
    }

    cur_overlay_id = id;
    segment_state[id] = OVERLAY_IS_USED;

_exit:
    int_unlock(lock);
    hal_sysfreq_req(HAL_SYSFREQ_USER_OVERLAY_SUBSYS, HAL_CMU_FREQ_32K);

    return ret;
}

enum HAL_OVERLAY_RET_T hal_overlay_subsys_unload(enum OVERLAY_SUBSYS_T chip_id,
                                                enum HAL_OVERLAY_ID_T id)
{
    enum HAL_OVERLAY_RET_T ret;

    CHECK_OVERLAY_ID(id);
    ret = HAL_OVERLAY_RET_OK;

    uint32_t lock;

    lock = int_lock();
    if (cur_overlay_id == id) {
        cur_overlay_id = HAL_OVERLAY_ID_QTY;
    } else if (cur_overlay_id == HAL_OVERLAY_ID_IN_CFG) {
        ret = HAL_OVERLAY_RET_IN_CFG;
    } else {
        ret = HAL_OVERLAY_RET_IN_USE;
    }
    hal_overlay_subsys_release(id);
    int_unlock(lock);

    return ret;
}

/*
 * acquire one overlay segment
 */
void hal_overlay_subsys_acquire(enum HAL_OVERLAY_ID_T id)
{
    uint32_t lock;

    CHECK_OVERLAY_ID(id);
    segment_state[id] = OVERLAY_IS_USED;

    lock = int_lock();
    invalid_overlay_cache(id);
    int_unlock(lock);
    return;
}

/*
 * release one overlay segment
 */
void hal_overlay_subsys_release(enum HAL_OVERLAY_ID_T id)
{
    CHECK_OVERLAY_ID(id);
    segment_state[id] = OVERLAY_IS_FREE;
    return;
}

/*
 * check if any overlay segment is used
 */
bool hal_overlay_subsys_is_used(void)
{
    int i;
    uint32_t lock;

    lock = int_lock();
    for (i = 0; i < HAL_OVERLAY_ID_QTY; i++) {
        // if any segment is in use, the overlay is used;
        if (segment_state[i] == OVERLAY_IS_USED ) {
            int_unlock(lock);
            return true;
        }
    }
    int_unlock(lock);
    return false;
}

#endif

