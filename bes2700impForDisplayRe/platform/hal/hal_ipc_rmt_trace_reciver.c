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
#ifdef __HAL_IPC_RMT_TRACE_RECIVER_SUPPORT__
#include "plat_types.h"
#include "hal_ipc_rmt_trace_reciver.h"
#include "hal_ipc_msg.h"
#include "hal_trace.h"

#define RMT_TRC_IPC_ID                     HAL_IPC_SUB_ID_1

struct HAL_IPC_RMT_TRACE_T{
    bool first_msg;
    const uint8_t *buf_start;
    const uint16_t *entry_start;
};

static struct HAL_IPC_RMT_TRACE_T ipc_rmt_trace_ctx[HAL_IPC_CORE_QTY] ;
static struct HAL_IPC_MSG_T *ipc_msg_trc_rcv_ctx[HAL_IPC_CORE_QTY] = {NULL,};

static void trace_msg_handler(enum HAL_IPC_CORE_TYPE_T core, struct IPC_RMT_TRC_CTRL_T *ctrl)
{
    uint32_t entry_rpos, entry_wpos;
    uint32_t trace_start, trace_end;
    uint16_t discard_cnt_w;
    uint16_t discard_cnt;
    uint32_t len;
    int proc_len;

    if (ipc_rmt_trace_ctx[core].first_msg) {
        ipc_rmt_trace_ctx[core].first_msg = false;
        ipc_rmt_trace_ctx[core].buf_start = ctrl->buf_start;
        ipc_rmt_trace_ctx[core].entry_start = ctrl->entry_start;
    }

    ctrl->seq_r = ctrl->seq_w;

    discard_cnt_w = ctrl->discard_cnt_w;
    discard_cnt = discard_cnt_w - ctrl->discard_cnt_r;
    ctrl->discard_cnt_r = discard_cnt_w;

    if (discard_cnt) {
        TRACE(0, "*** WARNING: SENSOR TRACE LOST %u", discard_cnt);
    }

    entry_rpos = ctrl->entry_rpos;
    entry_wpos = ctrl->entry_wpos;

    trace_start = ipc_rmt_trace_ctx[core].entry_start[entry_rpos];

    while (entry_rpos != entry_wpos) {
        entry_rpos++;
        if (entry_rpos >= ctrl->entry_cnt) {
            entry_rpos = 0;
        }
        trace_end = ipc_rmt_trace_ctx[core].entry_start[entry_rpos];
        if (trace_end <= trace_start) {
            // Trace starts from buffer head
            // Sens trace will never wrap buffer so that one invokation of TRACE_OUTPUT can process one trace.
            trace_start = 0;
        }
        len = trace_end - trace_start;
        if (len) {
            proc_len = TRACE_OUTPUT(ipc_rmt_trace_ctx[core].buf_start + trace_start, len);
            if (proc_len == 0) {
                break;
            }
        }
        trace_start = trace_end;
    }

    ctrl->entry_rpos = entry_rpos;
}

static unsigned int core_mcu2sys_rx_handler(const void *data, unsigned int len)
{
    const struct IPC_RMT_TRC_MSG_T *trc_msg;
    struct IPC_RMT_TRC_CTRL_T *ctrl;

    ASSERT(data, "%s: data ptr null", __func__);
    ASSERT(len == sizeof(*trc_msg), "%s: Bad msg len %u (expecting %u", __func__, len, sizeof(*trc_msg));

    trc_msg = (const struct IPC_RMT_TRC_MSG_T *)data;
    switch (trc_msg->id) {
    case IPC_RMT_TRC_MSG_ID_CRASH_ASSERT_START:
    case IPC_RMT_TRC_MSG_ID_CRASH_FAULT_START:
    case IPC_RMT_TRC_MSG_ID_CRASH_END:
        TRACE_FLUSH();
        // FALL THROUGH
    case IPC_RMT_TRC_MSG_ID_TRACE:
        ctrl = (struct IPC_RMT_TRC_CTRL_T *)trc_msg->param;
        trace_msg_handler(HAL_IPC_CORE_AP_SYS, ctrl);
        break;
    default:
        ASSERT(false, "%s: Bad trc msg id: %d", __func__, trc_msg->id)
    }

    return len;
}

void hal_ipc_rmt_trace_reciver_open(enum HAL_IPC_CORE_TYPE_T core)
{
    int ret;
    TR_INFO(0, "%s", __FUNCTION__);

    ipc_rmt_trace_ctx[core].first_msg = true;
    ipc_rmt_trace_ctx[core].buf_start = NULL;
    ipc_rmt_trace_ctx[core].entry_start = NULL;

    ipc_msg_trc_rcv_ctx[core] = (struct HAL_IPC_MSG_T *)hal_ipc_msg_ctx_get(core);
    if (!ipc_msg_trc_rcv_ctx[core]){
        ASSERT(ipc_msg_trc_rcv_ctx[core], "hal_ipc_msg_open ctx null");
        return;
    }
    switch (core) {
        case HAL_IPC_CORE_AP_BT:
            break;
        case HAL_IPC_CORE_AP_SYS:
            ipc_msg_trc_rcv_ctx[core]->hal_op.peer_irq_set = hal_ipc_mcu2sys_peer_irq_set;
            ipc_msg_trc_rcv_ctx[core]->hal_op.local_irq_clear = hal_ipc_mcu2sys_local_irq_clear;
            ipc_msg_trc_rcv_ctx[core]->hal_op.local_irq_mask_set = hal_ipc_mcu2sys_local_irq_mask_set;
            ipc_msg_trc_rcv_ctx[core]->hal_op.local_irq_mask_clr = hal_ipc_mcu2sys_local_irq_mask_clear;
            ipc_msg_trc_rcv_ctx[core]->hal_op.irq_active = hal_ipc_mcu2sys_irq_active;
            ipc_msg_trc_rcv_ctx[core]->hal_op.irq_pending = hal_ipc_mcu2sys_rx_irq_pending;
            ipc_msg_trc_rcv_ctx[core]->hal_op.start_recv = hal_ipc_mcu2sys_start_recv;
            ipc_msg_trc_rcv_ctx[core]->hal_op.stop_recv = hal_ipc_mcu2sys_stop_recv;
            ipc_msg_trc_rcv_ctx[core]->hal_op.open = hal_ipc_mcu2sys_open;
            ipc_msg_trc_rcv_ctx[core]->hal_op.close = hal_ipc_mcu2sys_close;
            ipc_msg_trc_rcv_ctx[core]->peer_recv_msg_ptr = IPC_MSG_MCU2SYS_MAILBOX_RX_MSG_PTR_LOC;
            ipc_msg_trc_rcv_ctx[core]->recv_msg_list_p = (struct IPC_MSG_T **)IPC_MSG_MCU2SYS_MAILBOX_BASE;
            ipc_msg_trc_rcv_ctx[core]->rx_irq_handler[RMT_TRC_IPC_ID] = core_mcu2sys_rx_handler;
            ipc_msg_trc_rcv_ctx[core]->tx_irq_handler[RMT_TRC_IPC_ID] = NULL;
            break;
        case HAL_IPC_CORE_SENSORHUB:
            break;
        default:
            ASSERT(false, "%s: Bad core:%d", __func__, core)
    }
    ret = hal_ipc_msg_open(ipc_msg_trc_rcv_ctx[core], RMT_TRC_IPC_ID, false);
    ASSERT(ret == 0, "hal_ipc_msg_open failed: %d", ret);
    ret = hal_ipc_msg_start_recv(ipc_msg_trc_rcv_ctx[core], RMT_TRC_IPC_ID);
    ASSERT(ret == 0, "hal_ipc_msg_start_recv failed: %d", ret);
}
#endif
