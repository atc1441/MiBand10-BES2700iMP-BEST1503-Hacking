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
#include "plat_addr_map.h"
#include "cmsis.h"
#include "hal_location.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hwtimer_list.h"
#include "rmt_trace_client.h"
#include "stdio.h"
#include "string.h"
#ifdef USE_TRACE_ID
#include "hal_trace_id.h"
#endif

#ifndef RMT_TRC_BUF_SIZE
#define RMT_TRC_BUF_SIZE                    (1024 * 4)
#endif
#ifndef RMT_TRC_ENTRY_CNT
#define RMT_TRC_ENTRY_CNT                   100
#endif

/* CONFIG_USE_SLOW_TIMER=y hwtimer_* unsupported */
#ifdef __NuttX__
#define RMT_TRC_INTVL_MS                    0
#endif

#ifndef RMT_TRC_INTVL_MS
#define RMT_TRC_INTVL_MS                    100
#endif

#define RMT_CRASH_WAIT_TIMEOUT_MS           200

#define VAR_MAX_VALUE(a)                    ((1 << (sizeof(a) * 8)) - 1)

STATIC_ASSERT(RMT_TRC_BUF_SIZE < VAR_MAX_VALUE(((struct RMT_TRC_CTRL_T *)0)->buf_size), "RMT_TRC_BUF_SIZE too large");
STATIC_ASSERT(RMT_TRC_ENTRY_CNT < VAR_MAX_VALUE(((struct RMT_TRC_CTRL_T *)0)->entry_cnt), "RMT_TRC_ENTRY_CNT too large");
STATIC_ASSERT(RMT_TRC_ENTRY_CNT < VAR_MAX_VALUE(((struct RMT_TRC_CTRL_T *)0)->entry_rpos), "RMT_TRC_ENTRY_CNT too large");
STATIC_ASSERT(RMT_TRC_ENTRY_CNT < VAR_MAX_VALUE(((struct RMT_TRC_CTRL_T *)0)->entry_wpos), "RMT_TRC_ENTRY_CNT too large");

SYNC_FLAGS_LOC
static uint8_t trc_buf[RMT_TRC_BUF_SIZE];
SYNC_FLAGS_LOC
static uint16_t trc_entry_list[RMT_TRC_ENTRY_CNT];
SYNC_FLAGS_LOC
static struct RMT_TRC_CTRL_T ctrl;
static unsigned int last_seq;
static bool in_crash;

#ifdef NO_SUBSYS_FLASH_ADDR_ACCESS
SRAM_RODATA_LOC
#endif
static const struct RMT_TRC_MSG_T trc_msg = {
    .msg_magic = RMT_TRC_MSG_MAGIC_WORD,
    .id = RMT_TRC_MSG_ID_TRACE,
    .param = &ctrl,
};

static char trc_tag[16];
static uint8_t trc_tag_len;

static RMT_TRC_SEND_CB send_cb;
static RMT_TRC_TX_ACTIVE_CB tx_active_cb;
static RMT_TRC_TX_IRQ_RUN_CB tx_irq_run_cb;

static void rmt_wait_tx_msg_done(void)
{
    uint32_t time;

    time = hal_sys_timer_get();
    while (tx_active_cb(last_seq) &&
            (hal_sys_timer_get() - time < MS_TO_TICKS(RMT_CRASH_WAIT_TIMEOUT_MS))) {
        tx_irq_run_cb();
    }
}

static void rmt_send_trace_msg(void)
{
    uint32_t lock;

    lock = int_lock();

#if (RMT_TRC_INTVL_MS > 0)
    if (!in_crash && tx_active_cb(last_seq) && ctrl.seq_r != ctrl.seq_w) {
        goto _exit;
    }
#endif

    ctrl.seq_w++;
    send_cb(&trc_msg, sizeof(trc_msg), &last_seq);

    if (in_crash) {
        rmt_wait_tx_msg_done();
    }

_exit: POSSIBLY_UNUSED;
    int_unlock(lock);
}

#if (RMT_TRC_INTVL_MS > 0)
static HWTIMER_ID msg_timer;

static void msg_timer_handler(void *param)
{
    rmt_send_trace_msg();
}
#endif

static void rmt_trace_notify_handler(enum HAL_TRACE_STATE_T state)
{
    int ret;
    struct RMT_TRC_MSG_T crash_msg;

    in_crash = true;

    memset(&crash_msg, 0, sizeof(crash_msg));
    crash_msg.msg_magic = RMT_TRC_MSG_MAGIC_WORD;
    if (state == HAL_TRACE_STATE_CRASH_ASSERT_START) {
        crash_msg.id = RMT_TRC_MSG_ID_CRASH_ASSERT_START;
    } else if (state == HAL_TRACE_STATE_CRASH_FAULT_START) {
        crash_msg.id = RMT_TRC_MSG_ID_CRASH_FAULT_START;
    } else {
        crash_msg.id = RMT_TRC_MSG_ID_CRASH_END;
    }
    crash_msg.param = &ctrl;

#if (RMT_TRC_INTVL_MS > 0)
    hwtimer_stop(msg_timer);
#endif
    rmt_wait_tx_msg_done();

    ctrl.seq_w++;
    ret = send_cb(&crash_msg, sizeof(crash_msg), &last_seq);
    if (ret == 0) {
        rmt_wait_tx_msg_done();
    }
}

static int rmt_global_tag_handler(char *buf, unsigned int buf_len)
{
#ifdef USE_TRACE_ID
    uint32_t id = hal_trace_get_id(buf);
    hal_trace_set_id(buf, SET_BITFIELD(id, USER_ID_CPUID, trc_tag[0] + get_cpu_id()));
    return id;
#else
    unsigned int len;

    if (buf_len && trc_tag_len) {
        len = (buf_len <= trc_tag_len) ? buf_len : trc_tag_len;
        memcpy(buf, trc_tag, len);
        return len;
    }

    return 0;
#endif
}

static void rmt_trace_output_handler(const unsigned char *buf, unsigned int buf_len)
{
    uint32_t entry_rpos, entry_wpos;
    uint32_t entry_avail = 0;
    uint32_t buf_rpos, buf_wpos;
    uint32_t buf_avail = 0;
    uint32_t len;

    if (buf_len == 0) {
        return;
    }

    entry_rpos = ctrl.entry_rpos;
    entry_wpos = ctrl.entry_wpos;

    if (entry_wpos >= entry_rpos) {
        entry_avail = ARRAY_SIZE(trc_entry_list) - (entry_wpos - entry_rpos) - 1;
    } else {
        entry_avail = (entry_rpos - entry_wpos) - 1;
    }

    if (entry_avail < 1) {
        ctrl.discard_cnt_w++;
        goto _tell_server;
    }

    buf_rpos = trc_entry_list[entry_rpos];
    buf_wpos = trc_entry_list[entry_wpos];
    if (buf_wpos >= buf_rpos) {
        // Not to wrap buffer -- simplify the atomic trace operation on server
        buf_avail = sizeof(trc_buf) - buf_wpos;
        if (buf_avail < buf_len && buf_rpos) {
            buf_wpos = 0;
            buf_avail = buf_rpos;
        }
    } else {
        buf_avail = (buf_rpos - buf_wpos);
    }

    if (buf_avail < buf_len) {
        ctrl.discard_cnt_w++;
        goto _tell_server;
    }

    len = buf_len;
    memcpy(trc_buf + buf_wpos, buf, len);
    buf_wpos += buf_len;

    entry_wpos++;
    if (entry_wpos >= ARRAY_SIZE(trc_entry_list)) {
        entry_wpos -= ARRAY_SIZE(trc_entry_list);
    }

    trc_entry_list[entry_wpos] = buf_wpos;
    ctrl.entry_wpos = entry_wpos;

_tell_server:
    // Send message to SERVER
    if (in_crash) {
        if ((entry_avail < ARRAY_SIZE(trc_entry_list) / 2) || (buf_avail < sizeof(trc_buf) / 2)) {
            rmt_send_trace_msg();
        }
        return;
    }

#if (RMT_TRC_INTVL_MS > 0)
    if ((entry_avail < ARRAY_SIZE(trc_entry_list) / 2) || (buf_avail < sizeof(trc_buf) / 2)) {
        hwtimer_stop(msg_timer);
        rmt_send_trace_msg();
        return;
    }

    if (!hwtimer_active(msg_timer)) {
        hwtimer_start(msg_timer, MS_TO_TICKS(RMT_TRC_INTVL_MS));
    }
#else
    rmt_send_trace_msg();
#endif
}

int rmt_trace_client_open(const struct RMT_TRC_CLIENT_CFG_T *cfg)
{
    if (!cfg) {
        return 1;
    }
    if (!cfg->tag) {
        return 2;
    }
    if (!cfg->send_cb) {
        return 3;
    }
    if (!cfg->tx_active_cb) {
        return 4;
    }
    if (!cfg->tx_irq_run_cb) {
        return 5;
    }

    int i = 0;
    while (i < sizeof(trc_tag) && cfg->tag[i]) {
        trc_tag[i] = cfg->tag[i];
        i++;
    }
    trc_tag_len = i;
    send_cb = cfg->send_cb;
    tx_active_cb = cfg->tx_active_cb;
    tx_irq_run_cb = cfg->tx_irq_run_cb;

    ctrl.ctrl_magic = RMT_TRC_CTRL_MAGIC_WORD;
    ctrl.buf_start = trc_buf;
    ctrl.entry_start = trc_entry_list;
    ctrl.buf_size = sizeof(trc_buf);
    ctrl.entry_cnt = ARRAY_SIZE(trc_entry_list);
    ctrl.entry_rpos = 0;
    ctrl.entry_wpos = 0;
    ctrl.discard_cnt_r = 0;
    ctrl.discard_cnt_w = 0;

    trc_entry_list[ctrl.entry_rpos] = 0;
    trc_entry_list[ctrl.entry_wpos] = 0;

    last_seq = 0;
    in_crash = false;

#if (RMT_TRC_INTVL_MS > 0)
    msg_timer = hwtimer_alloc(msg_timer_handler, NULL);
    ASSERT(msg_timer, "%s: Failed to alloc msg_timer", __func__);
#endif

    hal_trace_global_tag_register(rmt_global_tag_handler);
    hal_trace_app_register(HAL_TRACE_APP_REG_ID_0, rmt_trace_notify_handler, rmt_trace_output_handler);

    return 0;
}

unsigned int rmt_trace_client_msg_handler(const void *data, unsigned int len)
{
    const struct RMT_TRC_MSG_T *trc_msg;

#ifdef RMT_TRC_IN_MSG_CHAN
    if (!rmt_trace_check_trace(data, len)) {
        return 0;
    }
#endif

    ASSERT(data, "%s: data ptr null", __func__);
    ASSERT(len == sizeof(*trc_msg), "%s: Bad msg len %u (expecting %u", __func__, len, sizeof(*trc_msg));

    trc_msg = (const struct RMT_TRC_MSG_T *)data;
    ASSERT(trc_msg->id == RMT_TRC_MSG_ID_SERVER_DUMP, "%s: Bad msg id %d (expecting %d)", __func__, trc_msg->id, RMT_TRC_MSG_ID_SERVER_DUMP);

    // Just make the ram wakeup
    while (1) {
        __WFI();
    }

    return len;
}
