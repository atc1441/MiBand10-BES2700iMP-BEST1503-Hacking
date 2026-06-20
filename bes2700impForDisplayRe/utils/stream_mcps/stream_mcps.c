/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#include "stream_mcps.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "string.h"

#define _USER_QTY       (5)

#define _CHECK_USER_INDEX() if (index >= _USER_QTY) { \
                                TRACE(0, "[%s] Can not find user: %s", __func__, name); \
                                return -1; \
                            }

typedef struct {
    const char *name;
    uint32_t frame_us;
    uint32_t mcps_min;
    uint32_t mcps_max;
    uint32_t mcps_sum;
    uint32_t cnt;
    uint32_t interval_cnt;
    uint32_t sys_freq_running;
    uint32_t sys_freq;
    uint32_t ticks_pre;
} user_st_t;

static const char *NULL_STR = "NULL";
static user_st_t g_user_st[_USER_QTY];

static uint32_t get_user_index(const char *name)
{
    for (uint32_t i=0; i<_USER_QTY; i++) {
        // TRACE(0, "[%s] [%d] %s vs %s", __func__, i, g_user_st[i].name, name);
        if (strcmp(g_user_st[i].name, name) == 0) {
            return i;
        }
    }

    return _USER_QTY;
}

int32_t stream_mcps_start(const char *name, uint32_t sys_freq, float frame_ms, float interval_ms)
{
    ASSERT(name != NULL, "[%s] name is NULL", __func__);

    uint32_t index = 0;
    user_st_t *user_st = NULL;

    // Init once
    if (g_user_st[0].name == NULL) {
        for (uint32_t i=0; i<_USER_QTY; i++) {
            g_user_st[i].name = NULL_STR;
        }
    }

    index = get_user_index(name);

    if (index < _USER_QTY) {
        TRACE(0, "[%s] Find history user[%d]: %s", __func__, index, name);
    } else {
        index = get_user_index(NULL_STR);
        if (index < _USER_QTY) {
            TRACE(0, "[%s] Alloc new user[%d]: %s", __func__, index, name);
        } else {
            TRACE(0, "[%s] Can not alloc user: %s. You can increase _USER_QTY", __func__, name);
            return -1;
        }
    }

    _CHECK_USER_INDEX();
    user_st = &g_user_st[index];

    user_st->name = name;
    user_st->frame_us = (uint32_t)(frame_ms * 1000);

    user_st->cnt = 0;
    user_st->interval_cnt = (uint32_t)(interval_ms / frame_ms);

    user_st->mcps_min = 0x7FFFFFFF;
    user_st->mcps_max = 0;
    user_st->mcps_sum = 0;

    user_st->sys_freq_running = 0;
    user_st->sys_freq = sys_freq;

    user_st->ticks_pre = 0;

    return 0;
}

int32_t stream_mcps_stop(const char *name)
{
    ASSERT(name != NULL, "[%s] name is NULL", __func__);

    uint32_t index = get_user_index(name);
    _CHECK_USER_INDEX();
    user_st_t *user_st = &g_user_st[index];

    user_st->name = NULL_STR;

    return 0;
}

int32_t stream_mcps_run_pre(const char *name)
{
    ASSERT(name != NULL, "[%s] name is NULL", __func__);

    uint32_t index = get_user_index(name);
    _CHECK_USER_INDEX();
    user_st_t *user_st = &g_user_st[index];

    user_st->ticks_pre = hal_fast_sys_timer_get();

    return 0;
}

int32_t stream_mcps_run_post(const char *name)
{
    ASSERT(name != NULL, "[%s] name is NULL", __func__);

    uint32_t ticks_post = hal_fast_sys_timer_get();

    uint32_t index = get_user_index(name);
    _CHECK_USER_INDEX();
    user_st_t *user_st  = &g_user_st[index];
    uint32_t sys_freq   = user_st->sys_freq;
    uint32_t ticks_pre  = user_st->ticks_pre;

    // TRACE(0, "[%s] %d", __func__, ticks_post - ticks_pre);

    if (sys_freq == 0) {
        sys_freq = hal_sys_timer_calc_cpu_freq(5, 0) / 1000000;
        if (user_st->sys_freq_running != sys_freq) {
            TRACE(0, "[%s] Old freq: %dM; New freq: %dM", __func__, user_st->sys_freq_running, sys_freq);
        }
        user_st->sys_freq_running = sys_freq;
    }

    ASSERT(user_st->frame_us != 0, "[%s] frame_us = 0", __func__);
    uint32_t mcps = sys_freq *  FAST_TICKS_TO_US(ticks_post - ticks_pre) / user_st->frame_us;

    if (mcps < user_st->mcps_min) {
        user_st->mcps_min = mcps;
    }
    if (mcps > user_st->mcps_max) {
        user_st->mcps_max = mcps;
    }

    user_st->mcps_sum += mcps;

    if (user_st->cnt++ >= user_st->interval_cnt) {

        TRACE(0, "[%s] %s Freq: %dM, frame: %dus, MCPS: avg=%d, min=%d, max=%d", __func__, \
            user_st->name, sys_freq, user_st->frame_us, \
            user_st->mcps_sum / user_st->interval_cnt, user_st->mcps_min, user_st->mcps_max);

        user_st->mcps_min = 0x7FFFFFFF;
        user_st->mcps_max = 0;
        user_st->mcps_sum = 0;
        user_st->cnt = 0;
    }

    return 0;
}