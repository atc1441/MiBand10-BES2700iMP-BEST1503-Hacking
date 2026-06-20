/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifdef __HAL_IPC_RMT_TRACE_SENDER_SUPPORT__
#include "plat_types.h"
#include "cmsis.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include CHIP_SPECIFIC_HDR(hal_ipc_mcu2sys)
#include "hal_ipc_msg.h"
#include "hal_ipc_rmt_trace_sender.h"
#include "string.h"
#include "hal_cmu.h"

#define RMT_TRC_IPC_ID                     HAL_IPC_SUB_ID_1

#ifndef RMT_TRC_BUF_SIZE
#define RMT_TRC_BUF_SIZE                   (1024 * 4)
#endif
#ifndef RMT_TRC_ENTRY_CNT
#define RMT_TRC_ENTRY_CNT                  100
#endif
#ifndef RMT_TRC_INTVL_MS
#define RMT_TRC_INTVL_MS                   0
#endif

#define RMT_TRC_CRASH_WAIT_TIMEOUT_MS          200

#define VAR_MAX_VALUE(a)                    ((1 << (sizeof(a) * 8)) - 1)

STATIC_ASSERT(RMT_TRC_BUF_SIZE < VAR_MAX_VALUE(((struct IPC_RMT_TRC_CTRL_T *)0)->buf_size), "RMT_TRC_BUF_SIZE too large");
STATIC_ASSERT(RMT_TRC_ENTRY_CNT < VAR_MAX_VALUE(((struct IPC_RMT_TRC_CTRL_T *)0)->entry_cnt), "RMT_TRC_ENTRY_CNT too large");
STATIC_ASSERT(RMT_TRC_ENTRY_CNT < VAR_MAX_VALUE(((struct IPC_RMT_TRC_CTRL_T *)0)->entry_rpos), "RMT_TRC_ENTRY_CNT too large");
STATIC_ASSERT(RMT_TRC_ENTRY_CNT < VAR_MAX_VALUE(((struct IPC_RMT_TRC_CTRL_T *)0)->entry_wpos), "RMT_TRC_ENTRY_CNT too large");

static uint8_t trc_buf[RMT_TRC_BUF_SIZE];
static uint16_t trc_entry_list[RMT_TRC_ENTRY_CNT];
static struct IPC_RMT_TRC_CTRL_T ctrl;
static unsigned int last_seq;
static bool in_crash;

static const struct IPC_RMT_TRC_MSG_T trc_msg = {
    .id = IPC_RMT_TRC_MSG_ID_TRACE,
    .param = &ctrl,
};

static struct HAL_IPC_MSG_T *ipc_msg_trc_snd_ctx = NULL;

static void rmt_trc_wait_tx_msg_done(void)
{
    uint32_t time;

    time = hal_sys_timer_get();
    while (hal_ipc_msg_tx_active(ipc_msg_trc_snd_ctx, RMT_TRC_IPC_ID, last_seq) &&
            (hal_sys_timer_get() - time < MS_TO_TICKS(RMT_TRC_CRASH_WAIT_TIMEOUT_MS))) {
        hal_ipc_msg_tx_irq_run(ipc_msg_trc_snd_ctx, RMT_TRC_IPC_ID);
    }
}

static void rmt_trc_send_trace_msg(void)
{
    uint32_t lock;
    int ret = 0;

    lock = int_lock();

#if (RMT_TRC_INTVL_MS > 0)
    if (!in_crash && ipc_msg_trc_snd_ctx->hal_op.irq_active(RMT_TRC_IPC_ID, last_seq) && ctrl.seq_r != ctrl.seq_w) {
        goto _exit;
    }
#endif

    ctrl.seq_w++;
    ret = hal_ipc_msg_send_seq(ipc_msg_trc_snd_ctx, RMT_TRC_IPC_ID, &trc_msg, sizeof(trc_msg), &last_seq);
    if (ret){
        hal_cmu_simu_set_val(ret);
    }

    if (in_crash) {
        rmt_trc_wait_tx_msg_done();
    }

_exit: POSSIBLY_UNUSED;
    int_unlock(lock);
}

#if (RMT_TRC_INTVL_MS > 0)
static HWTIMER_ID msg_timer;

static void msg_timer_handler(void *param)
{
    rmt_trc_send_trace_msg();
}
#endif

static void rmt_trc_notify_handler(enum HAL_TRACE_STATE_T state)
{
    int ret;
    struct IPC_RMT_TRC_MSG_T crash_msg;

    in_crash = true;

    if (state == HAL_TRACE_STATE_CRASH_ASSERT_START) {
        crash_msg.id = IPC_RMT_TRC_MSG_ID_CRASH_ASSERT_START;
    } else if (state == HAL_TRACE_STATE_CRASH_FAULT_START) {
        crash_msg.id = IPC_RMT_TRC_MSG_ID_CRASH_FAULT_START;
    } else {
        crash_msg.id = IPC_RMT_TRC_MSG_ID_CRASH_END;
    }
    crash_msg.param = &ctrl;

#if (RMT_TRC_INTVL_MS > 0)
    hwtimer_stop(msg_timer);
#endif
    rmt_trc_wait_tx_msg_done();

    ctrl.seq_w++;
    ret = hal_ipc_msg_send_seq(ipc_msg_trc_snd_ctx, RMT_TRC_IPC_ID, &crash_msg, sizeof(crash_msg), &last_seq);
    if (ret == 0) {
        rmt_trc_wait_tx_msg_done();
    }
}

static int rmt_trc_tag_handler(char *buf, unsigned int buf_len)
{
    const char tag[] = "[SYS]";
    unsigned int len;

    len = sizeof(tag) - 1;
    if (buf_len > len) {
        memcpy(buf, tag, len);
        return len;
    }

    return 0;
}

static void rmt_trc_output_handler(const unsigned char *buf, unsigned int buf_len)
{
    uint32_t entry_rpos, entry_wpos;
    uint32_t entry_avail;
    uint32_t buf_rpos, buf_wpos;
    uint32_t buf_avail;
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
        goto _tell_mcu;
    }

    buf_rpos = trc_entry_list[entry_rpos];
    buf_wpos = trc_entry_list[entry_wpos];
    if (buf_wpos >= buf_rpos) {
        // Not to wrap buffer -- simplify the atomic trace operation on MCU
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
        goto _tell_mcu;
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

_tell_mcu:
    // Send message to MCU
    if (in_crash) {
        if ((entry_avail < ARRAY_SIZE(trc_entry_list) / 2) || (buf_avail < sizeof(trc_buf) / 2)) {
            rmt_trc_send_trace_msg();
        }
        return;
    }

#if (RMT_TRC_INTVL_MS > 0)
    if ((entry_avail < ARRAY_SIZE(trc_entry_list) / 2) || (buf_avail < sizeof(trc_buf) / 2)) {
        hwtimer_stop(msg_timer);
        rmt_trc_send_trace_msg();
        return;
    }

    if (!hwtimer_active(msg_timer)) {
        hwtimer_start(msg_timer, MS_TO_TICKS(RMT_TRC_INTVL_MS));
    }
#else
    rmt_trc_send_trace_msg();
#endif
}

void hal_ipc_rmt_trace_sender_open(void)
{
    int ret;

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

    ipc_msg_trc_snd_ctx = (struct HAL_IPC_MSG_T *)hal_ipc_msg_ctx_get(HAL_IPC_CORE_AP_SYS);
    if (!ipc_msg_trc_snd_ctx){
        ASSERT(ipc_msg_trc_snd_ctx, "hal_ipc_msg_open ctx null");
        return;
    }

    ipc_msg_trc_snd_ctx->hal_op.peer_irq_set = hal_ipc_mcu2sys_peer_irq_set;
    ipc_msg_trc_snd_ctx->hal_op.local_irq_clear = hal_ipc_mcu2sys_local_irq_clear;
    ipc_msg_trc_snd_ctx->hal_op.local_irq_mask_set = hal_ipc_mcu2sys_local_irq_mask_set;
    ipc_msg_trc_snd_ctx->hal_op.local_irq_mask_clr = hal_ipc_mcu2sys_local_irq_mask_clear;
    ipc_msg_trc_snd_ctx->hal_op.irq_active = hal_ipc_mcu2sys_irq_active;
    ipc_msg_trc_snd_ctx->hal_op.irq_pending = hal_ipc_mcu2sys_rx_irq_pending;
    ipc_msg_trc_snd_ctx->hal_op.start_recv = hal_ipc_mcu2sys_start_recv;
    ipc_msg_trc_snd_ctx->hal_op.stop_recv = hal_ipc_mcu2sys_stop_recv;
    ipc_msg_trc_snd_ctx->hal_op.open = hal_ipc_mcu2sys_open;
    ipc_msg_trc_snd_ctx->hal_op.close = hal_ipc_mcu2sys_close;
    ipc_msg_trc_snd_ctx->recv_msg_list_p = (struct IPC_MSG_T **)*(uint32_t *)IPC_MSG_MCU2SYS_MAILBOX_RX_MSG_PTR_LOC;
    ipc_msg_trc_snd_ctx->send_msg_list_p = (struct IPC_MSG_T **)IPC_MSG_MCU2SYS_MAILBOX_BASE;
    ipc_msg_trc_snd_ctx->rx_irq_handler[RMT_TRC_IPC_ID] = NULL;
    ipc_msg_trc_snd_ctx->tx_irq_handler[RMT_TRC_IPC_ID] = NULL;

    ret = hal_ipc_msg_open(ipc_msg_trc_snd_ctx, RMT_TRC_IPC_ID, false);

    ASSERT(ret == 0, "hal_ipc_msg_open failed: %d", ret);

    hal_trace_global_tag_register(rmt_trc_tag_handler);
    hal_trace_app_register(HAL_TRACE_APP_REG_ID_0, rmt_trc_notify_handler, rmt_trc_output_handler);
}
#endif
