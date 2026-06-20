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
/**
 * Usage:
 *  1. This module is related with UI.
 **/
#include "hal_trace.h"
#include "anc_assist_tws_sync.h"
#include "anc_assist_anc.h"
#if defined(IBRT)
#include "app_ibrt_internal.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_ibrt_customif_cmd.h"
#include "app_tws_ctrl_thread.h"
#include "besaud_api.h"

#define _TWS_SYNC_INFO_ENABLED
#define _SYNC_ARBITRATION_MECHANISM_ENABLED
#endif

// 100 is the scale factor which is related with precision
#define _EQ_FLOAT(a, b)         ((uint32_t)((a) * 100) == (uint32_t)((b) * 100) ? true : false)

#if defined(_TWS_SYNC_INFO_ENABLED)
#define SYNC_INTERVAL_TIME_MS   (1000)
#define SYNC_MONITOR_TIME_MS    (10000)

#define _TWS_ROLE_MASTER        BTIF_BCR_MASTER
#define _TWS_ROLE_SLAVE         BTIF_BCR_SLAVE
#define _TWS_ROLE_UNKNOWN       (0xFF)

typedef struct {
    uint32_t    trigger_cnt;            // For trigger operation
    float       ff_gain_coef;
    float       fb_gain_coef;
    uint32_t    anc_curve_index;
    uint32_t    fir_flag;
    uint32_t    tws_role;
} anc_assist_sync_info_t;

#define SYNC_INFO_INIT(info)            \
{                                       \
    info.trigger_cnt = 0;               \
    info.ff_gain_coef = 1.0;            \
    info.fb_gain_coef = 1.0;            \
    info.anc_curve_index = 0;           \
    info.fir_flag = 0;                  \
    info.tws_role = _TWS_ROLE_UNKNOWN;  \
}

static anc_assist_sync_info_t g_curr_info;
static anc_assist_sync_info_t g_opt_info;
static anc_assist_sync_info_t g_local_info;
static anc_assist_sync_info_t g_remote_info;
static anc_assist_sync_info_t g_remote_info_new;
static bool g_remote_info_new_flag  = false;
static bool g_opt_info_cache_flag   = false;

// Use heartbeat to instead of timer or thread.
#define HEARTBEAT_CNT_INVALID       (0x0FFFFFFF)
static uint32_t g_heartbeat_cnt     = HEARTBEAT_CNT_INVALID;
static uint32_t g_heartbeat_cnt_threshold   = 100;

static uint32_t g_monitor_sync_cnt = 0;
static uint32_t g_monitor_sync_cnt_threshold = 1000;

// Lock
osMutexId _tws_sync_mutex_id = NULL;
osMutexDef(_tws_sync_mutex);

int32_t anc_assist_tws_reset_curr_info(void)
{
    TRACE(0, "%s ...", __func__);

    g_curr_info.ff_gain_coef = 1.0;
    g_curr_info.fb_gain_coef = 1.0;
    g_curr_info.anc_curve_index = 0;
    g_curr_info.fir_flag = 0;

    return 0;
}

static int32_t anc_assist_status_change_impl(anc_assist_sync_info_t *info)
{
#if defined(ANC_APP)
    if (_EQ_FLOAT(g_curr_info.ff_gain_coef, info->ff_gain_coef) == false) {
        anc_assist_anc_set_gain_coef_impl(ANC_GAIN_USER_ANC_ASSIST_SYNC, info->ff_gain_coef, info->ff_gain_coef, ANC_FEEDFORWARD);
#ifdef AUDIO_ANC_TT_HW
        anc_assist_anc_set_gain_coef_impl(ANC_GAIN_USER_ANC_ASSIST_SYNC, info->ff_gain_coef, info->ff_gain_coef, ANC_TALKTHRU);
#endif
        g_curr_info.ff_gain_coef = info->ff_gain_coef;
    }

    if (_EQ_FLOAT(g_curr_info.fb_gain_coef, info->fb_gain_coef) == false) {
        anc_assist_anc_set_gain_coef_impl(ANC_GAIN_USER_ANC_ASSIST_SYNC, info->fb_gain_coef, info->fb_gain_coef, ANC_FEEDBACK);
        g_curr_info.fb_gain_coef = info->fb_gain_coef;
    }

    if (g_curr_info.anc_curve_index != info->anc_curve_index) {
        anc_assist_anc_switch_curve_impl(info->anc_curve_index);
        g_curr_info.anc_curve_index = info->anc_curve_index;
    }

    if (g_curr_info.fir_flag != info->fir_flag) {
        anc_assist_anc_switch_fir_flag_impl(info->fir_flag);
        g_curr_info.fir_flag = info->fir_flag;
    }
#endif

    return 0;
}

static void _tws_sync_create_lock(void)
{
    if (_tws_sync_mutex_id == NULL) {
        _tws_sync_mutex_id = osMutexCreate((osMutex(_tws_sync_mutex)));
    }
}

static void _tws_sync_lock(void)
{
    osMutexWait(_tws_sync_mutex_id, osWaitForever);
}

static void _tws_sync_unlock(void)
{
    osMutexRelease(_tws_sync_mutex_id);
}

static bool _check_tws_connected(void)
{
    return app_tws_ibrt_tws_link_connected();
}

static uint32_t _get_tws_role(void)
{
    return app_tws_ibrt_get_local_tws_role();
}

static int32_t _send_sync_info(anc_assist_sync_info_t *info)
{
#if defined(ANC_ASSIST_ENABLED)
    if (_check_tws_connected()) {
        tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_ANC_ASSIST_STATUS, (uint8_t *)info, sizeof(anc_assist_sync_info_t));
    }
#endif

    return 0;
}

int32_t anc_assist_send_sync_info(void)
{
    TRACE(0, "%s ...", __func__);

    anc_assist_sync_info_t sync_info;
    sync_info.trigger_cnt = 1;
    sync_info.ff_gain_coef = g_curr_info.ff_gain_coef;
    sync_info.fb_gain_coef = g_curr_info.fb_gain_coef;
    sync_info.anc_curve_index = g_curr_info.anc_curve_index;
    sync_info.fir_flag = g_curr_info.fir_flag;
    sync_info.tws_role = _TWS_ROLE_SLAVE;

    if (_get_tws_role() == _TWS_ROLE_SLAVE) {
        TRACE(0, "%s _TWS_ROLE_SLAVE", __func__);
        _send_sync_info(&sync_info);
    } else {
        TRACE(0, "%s _TWS_ROLE_MASTER", __func__);
    }

    return 0;
}

#if defined(_SYNC_ARBITRATION_MECHANISM_ENABLED)
WEAK uint32_t anc_ff_fir_lms_sync_fir_flag(uint32_t local_fir_flag, uint32_t remote_fir_flag)
{
    return 0;
}

static bool anc_assist_tws_arbitration_mechanism(anc_assist_sync_info_t info)
{
    anc_assist_sync_info_t ret_info = g_curr_info;
    bool sync_flag = false;

    // WIND FF: Follow strong wind (small gain)
    // NOISE FB: Follow big noise (big gain)
    // FIR FLAG: AND
    anc_assist_sync_info_t *local_info = NULL;
    anc_assist_sync_info_t *remote_info = NULL;

    if (info.tws_role == _TWS_ROLE_MASTER) {
        local_info = &info;
        remote_info = &g_remote_info;
    } else if (info.tws_role == _TWS_ROLE_SLAVE) {
        local_info = &g_local_info;
        remote_info = &info;
    } else {
        ASSERT(0, "[%s] Invalid role: %d", __func__, info.tws_role);
    }

    ret_info.ff_gain_coef = MIN(local_info->ff_gain_coef, remote_info->ff_gain_coef);
    ret_info.fb_gain_coef = MAX(local_info->fb_gain_coef, remote_info->fb_gain_coef);
    ret_info.fir_flag = anc_ff_fir_lms_sync_fir_flag(local_info->fir_flag, remote_info->fir_flag);

    TRACE(8, "[%s] opt role: %d, Gain(x100) FF: (%d, %d) --> %d", __func__, info.tws_role,
        (int32_t)(g_local_info.ff_gain_coef * 100), (int32_t)(g_remote_info.ff_gain_coef * 100), (int32_t)(ret_info.ff_gain_coef * 100));

    TRACE(8, "[%s] FB: (%d, %d) --> %d, FIR: (%d, %d) --> %d", __func__,
        (int32_t)(g_local_info.fb_gain_coef * 100), (int32_t)(g_remote_info.fb_gain_coef * 100), (int32_t)(ret_info.fb_gain_coef * 100),
        g_local_info.fir_flag, g_remote_info.fir_flag, ret_info.fir_flag);

    if ((_EQ_FLOAT(g_curr_info.ff_gain_coef, ret_info.ff_gain_coef) == false) ||
        (_EQ_FLOAT(g_curr_info.fb_gain_coef, ret_info.fb_gain_coef) == false) ||
        g_curr_info.fir_flag != ret_info.fir_flag) {

        sync_flag = true;
    }

    if (sync_flag) {
        TRACE(1, "[%s] Sync change status!", __func__);
        ret_info.tws_role = _TWS_ROLE_MASTER;
        _send_sync_info(&ret_info);
        anc_assist_status_change_impl(&ret_info);
    }

    return sync_flag;
}
#endif

int32_t anc_assist_tws_sync_status_change(uint8_t *buf, uint32_t len)
{
    ASSERT(len == sizeof(anc_assist_sync_info_t), "[%s] len(%d) != sizeof(anc_assist_sync_info_t)(%d)", __func__, len, sizeof(anc_assist_sync_info_t));

    anc_assist_sync_info_t info = *((anc_assist_sync_info_t *)buf);

    // TRACE(2, "[%s] gain_coef: %d", __func__, (uint32_t)(info.ff_gain_coef * 100));

#if defined(_SYNC_ARBITRATION_MECHANISM_ENABLED)
    TRACE(3, "[%s] Local role: %d, sync role: %d", __func__, _get_tws_role(), info.tws_role);
    if ((_get_tws_role() == _TWS_ROLE_MASTER) && (info.tws_role == _TWS_ROLE_SLAVE)) {
        _tws_sync_lock();
        g_remote_info_new = info;
        g_remote_info_new_flag = true;
        _tws_sync_unlock();
    } else if ((_get_tws_role() == _TWS_ROLE_SLAVE) && (info.tws_role == _TWS_ROLE_MASTER)) {
        anc_assist_status_change_impl(&info);
    } else if ((_get_tws_role() == _TWS_ROLE_MASTER) && (info.tws_role == _TWS_ROLE_MASTER)) {
        ASSERT(0, "[%s] WARNING: Both buds are master", __func__);
    } else if ((_get_tws_role() == _TWS_ROLE_SLAVE) && (info.tws_role == _TWS_ROLE_SLAVE)) {
        ASSERT(0, "[%s] WARNING: Both buds are slave", __func__);
    } else {
        ASSERT(0, "[%s] Invalid roles: %d, %d", __func__, _get_tws_role(), info.tws_role);
    }
#else
    anc_assist_status_change_impl(&info);
#endif

    return 0;
}

int32_t anc_assist_tws_sync_info_impl(anc_assist_sync_info_t info)
{
#if defined(_SYNC_ARBITRATION_MECHANISM_ENABLED)
    uint32_t local_role = _get_tws_role();
    if ((local_role == _TWS_ROLE_MASTER) && _check_tws_connected()) {
        anc_assist_tws_arbitration_mechanism(info);
    } else if ((local_role == _TWS_ROLE_SLAVE) && _check_tws_connected()) {
        info.tws_role = _TWS_ROLE_SLAVE;
        _send_sync_info(&info);
    } else {
        anc_assist_status_change_impl(&info);
    }
#else
    _send_sync_info(&info);
    anc_assist_status_change_impl(&info);
#endif

    g_opt_info_cache_flag = false;
    g_heartbeat_cnt = 0;

    return 0;
}

int32_t anc_assist_tws_sync_info(anc_assist_sync_info_t info)
{
    // TRACE(2, "[%s] ff_gain_coef: %d", __func__, (uint32_t)(info.ff_gain_coef * 100));

    info.tws_role = _get_tws_role();
    g_local_info = info;
    if (g_heartbeat_cnt < g_heartbeat_cnt_threshold) {
        g_opt_info_cache_flag = true;
        TRACE(1, "[%s] The sending interval is too short! Cache sync info.", __func__);
        return 1;
    } else {
        anc_assist_tws_sync_info_impl(info);
        return 0;
    }
}
#endif


int32_t anc_assist_tws_force_sync_info(anc_assist_sync_info_t info)
{
    // TRACE(2, "[%s] ff_gain_coef: %d", __func__, (uint32_t)(info.ff_gain_coef * 100));

    info.tws_role = _get_tws_role();
    g_local_info = info;

    anc_assist_tws_sync_info_impl(info);
    return 0;
}

int32_t anc_assist_tws_sync_set_anc_gain_coef(uint32_t *ff_gain_changed, anc_assist_algo_id_t *ff_id, float *ff_gain_coef, uint8_t ff_ch_num,
                                              uint32_t *fb_gain_changed, anc_assist_algo_id_t *fb_id, float *fb_gain_coef, uint8_t fb_ch_num)
{
#if defined(_TWS_SYNC_INFO_ENABLED)
    bool tws_sync = false;
#endif

    // Diff algo set different anc channel
    if (any_of_algo_id(fb_id, fb_ch_num, ANC_ASSIST_ALGO_ID_NOISE) ||
        any_of_algo_id(ff_id, ff_ch_num, ANC_ASSIST_ALGO_ID_WIND)) {
        // Both Master and Slave can make a decision 
#if defined(_TWS_SYNC_INFO_ENABLED)
        tws_sync = true;
#endif
    }

#if MAX_FF_CHANNEL_NUM == 1
    if (ff_gain_changed[0]) {
#if defined(_TWS_SYNC_INFO_ENABLED)
        if (tws_sync) {
            g_opt_info.ff_gain_coef = ff_gain_coef[0];
        }
        else
#endif
        {
            anc_assist_anc_set_gain_coef_impl(ANC_GAIN_USER_ANC_ASSIST_LOCAL, ff_gain_coef[0], ff_gain_coef[0], ANC_FEEDFORWARD);
#ifdef AUDIO_ANC_TT_HW
            anc_assist_anc_set_gain_coef_impl(ANC_GAIN_USER_ANC_ASSIST_LOCAL, ff_gain_coef[0], ff_gain_coef[0], ANC_TALKTHRU);
#endif
        }
    }
#elif MAX_FF_CHANNEL_NUM == 2
    if (ff_gain_changed[0] || ff_gain_changed[1]) {
#if defined(_TWS_SYNC_INFO_ENABLED)
        if (tws_sync) {
            g_opt_info.ff_gain_coef = ff_gain_coef[0];
        }
        else
#endif
        {
            anc_assist_anc_set_gain_coef_impl(ANC_GAIN_USER_ANC_ASSIST_LOCAL, ff_gain_coef[0], ff_gain_coef[1], ANC_FEEDFORWARD);
#ifdef AUDIO_ANC_TT_HW
            anc_assist_anc_set_gain_coef_impl(ANC_GAIN_USER_ANC_ASSIST_LOCAL, ff_gain_coef[0], ff_gain_coef[1], ANC_TALKTHRU);
#endif
        }
    }
#endif

#if MAX_FF_CHANNEL_NUM == 1
    if (fb_gain_changed[0]) {
#if defined(_TWS_SYNC_INFO_ENABLED)
        if (tws_sync) {
            g_opt_info.fb_gain_coef = fb_gain_coef[0];
        }
        else
#endif
        {
            anc_assist_anc_set_gain_coef_impl(ANC_GAIN_USER_ANC_ASSIST_LOCAL, fb_gain_coef[0], fb_gain_coef[0], ANC_FEEDBACK);
        }
    }
#elif MAX_FB_CHANNEL_NUM == 2
    if (fb_gain_changed[0] || fb_gain_changed[1]) {
#if defined(_TWS_SYNC_INFO_ENABLED)
        if (tws_sync) {
            g_opt_info.fb_gain_coef = fb_gain_coef[0];
        }
        else
#endif
        {
            anc_assist_anc_set_gain_coef_impl(ANC_GAIN_USER_ANC_ASSIST_LOCAL, fb_gain_coef[0], fb_gain_coef[1], ANC_FEEDBACK);
        }
    }
#endif

#if defined(_TWS_SYNC_INFO_ENABLED)
    if (tws_sync) {
        anc_assist_tws_sync_info(g_opt_info);
    }
#endif

    return 0;
}

int32_t anc_assist_tws_sync_set_anc_curve(anc_assist_algo_id_t id, uint32_t index)
{
    TRACE(3, "[%s] Switch curve: id = 0x%x, index = %d", __func__, id, index);

#if defined(_TWS_SYNC_INFO_ENABLED)
    if(bes_bt_tws_besaud_is_connected())
    {
        if (_get_tws_role() == _TWS_ROLE_SLAVE)
        {
            TRACE(1, "[%s] SLAVE has not access to change anc curve!", __func__);
        } 
        else
        {    // _TWS_ROLE_MASTER or _TWS_ROLE_UNKNOWN
            g_opt_info.anc_curve_index = index;
            anc_assist_tws_sync_info(g_opt_info);
        }
    }
#else
    anc_assist_anc_switch_curve_impl(index);
#endif

    return 0;
}

int32_t anc_assist_tws_sync_set_fir_flag(anc_assist_algo_id_t id, uint32_t flag)
{
    TRACE(3, "[%s] id = 0x%x, flag = %d", __FUNCTION__, id, flag);

#if defined(_TWS_SYNC_INFO_ENABLED)
    if (0/*_get_tws_role() == _TWS_ROLE_SLAVE*/) {
        TRACE(1, "[%s] SLAVE has not access to change fir flag!", __FUNCTION__);
    } else { // _TWS_ROLE_MASTER or _TWS_ROLE_UNKNOWN
        g_opt_info.fir_flag = flag;
        anc_assist_tws_force_sync_info(g_opt_info);
    }
#else
    anc_assist_anc_switch_fir_flag_impl(flag);
#endif

    return 0;
}

int32_t anc_assist_tws_sync_init(float frame_ms)
{
#if defined(_TWS_SYNC_INFO_ENABLED)
    SYNC_INFO_INIT(g_curr_info);
    SYNC_INFO_INIT(g_opt_info);
    SYNC_INFO_INIT(g_local_info);
    SYNC_INFO_INIT(g_remote_info);
    SYNC_INFO_INIT(g_remote_info_new);

    g_curr_info.tws_role        = _TWS_ROLE_MASTER; // Just used to sync by master
    g_remote_info.tws_role      = _TWS_ROLE_SLAVE;
    g_remote_info_new.tws_role  = _TWS_ROLE_SLAVE;

    g_remote_info_new_flag  = false;
    g_opt_info_cache_flag   = false;

    g_heartbeat_cnt = HEARTBEAT_CNT_INVALID;
    g_heartbeat_cnt_threshold = (uint32_t)(SYNC_INTERVAL_TIME_MS / frame_ms);

    g_monitor_sync_cnt = 0;
    g_monitor_sync_cnt_threshold = (uint32_t)(SYNC_MONITOR_TIME_MS / frame_ms);

    _tws_sync_create_lock();

    // TRACE(0, "[%s] Heartbeat counter threshold: %d", __func__, g_heartbeat_cnt_threshold);
#endif

    return 0;
}

int32_t anc_assist_tws_sync_heartbeat(void)
{
#if defined(_TWS_SYNC_INFO_ENABLED)
    bool sync_info_flag = false;
    bool monitor_sync_flag = true;

    _tws_sync_lock();
    if (g_remote_info_new_flag) {
        g_remote_info = g_remote_info_new;
        sync_info_flag = true;
        g_remote_info_new_flag = false;
    }
    _tws_sync_unlock();

    if (sync_info_flag) {
        if ((g_remote_info.trigger_cnt == 1) && (_get_tws_role() == _TWS_ROLE_MASTER)) {
            TRACE(0, "need update g_monitor_sync_cnt !!!");
            if (g_monitor_sync_cnt <  (uint32_t)(g_monitor_sync_cnt_threshold*0.8)) {
                g_monitor_sync_cnt =  (uint32_t)(g_monitor_sync_cnt_threshold*0.8);
            }
            g_remote_info.trigger_cnt = 0;
        } else {
            anc_assist_tws_sync_info_impl(g_remote_info);
            monitor_sync_flag = false;
        }
    }

    if (g_heartbeat_cnt != HEARTBEAT_CNT_INVALID) {
        g_heartbeat_cnt++;

        if (g_heartbeat_cnt > g_heartbeat_cnt_threshold) {
            if (g_opt_info_cache_flag) {
                anc_assist_tws_sync_info_impl(g_local_info);
                monitor_sync_flag = false;
            } else {
                g_heartbeat_cnt = HEARTBEAT_CNT_INVALID;
            }
        }
    }

    if (g_monitor_sync_cnt++ >= g_monitor_sync_cnt_threshold) {
        if (_get_tws_role() == _TWS_ROLE_MASTER) {
            TRACE(2, "[%s] Monitor sync: %d", __func__, monitor_sync_flag);
            if (monitor_sync_flag) {
                _send_sync_info(&g_curr_info);
            }
        }
        g_monitor_sync_cnt = 0;
    }
#endif

    return 0;
}