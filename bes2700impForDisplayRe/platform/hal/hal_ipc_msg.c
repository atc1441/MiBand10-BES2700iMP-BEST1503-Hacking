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
#ifdef __HAL_IPC_SUPPORT__
#include "plat_addr_map.h"
#include "cmsis_nvic.h"
#include "hal_location.h"
#include "hal_ipc_msg.h"
#include "hal_sleep.h"
#include "hal_timer.h"
#include "hal_trace.h"

#define IPC_MSG_VERBOSE_TRACE 0
// ((1 << 0) | (1 << 1))


#define HAL_SYS_WAKE_LOCK_USER_MCU2SENS     HAL_SYS_WAKE_LOCK_USER_5

STATIC_ASSERT(sizeof(struct IPC_MSG_T *) * HAL_IPC_SUB_ID_QTY +
    sizeof(const struct IPC_MSG_T **) <= SENS_MAILBOX_SIZE, "IPC_MSG_SIZE too small");

static struct HAL_IPC_MSG_T hal_ipc_msg_pool[HAL_IPC_CORE_QTY] = {0,};

static void hal_ipc_msg_busy(struct HAL_IPC_MSG_T *ipc_msg_ctx, enum HAL_IPC_SUB_ID_T id, bool busy)
{
    int i;
    bool new_state;

    if (ipc_msg_ctx->chan_busy[id] == busy) {
        return;
    }

    ipc_msg_ctx->chan_busy[id] = busy;

    if (ipc_msg_ctx->busy_now == busy) {
        return;
    }

    if (busy) {
        hal_sys_wake_lock(HAL_SYS_WAKE_LOCK_USER_MCU2SENS);
        ipc_msg_ctx->busy_now = true;
    } else {
        new_state = false;
        for (i = 0; i < HAL_IPC_SUB_ID_QTY; i++) {
            if (ipc_msg_ctx->chan_busy[i]) {
                new_state = true;
                break;
            }
        }
        if (!new_state) {
            hal_sys_wake_unlock(HAL_SYS_WAKE_LOCK_USER_MCU2SENS);
            ipc_msg_ctx->busy_now = false;
        }
    }
}


static void hal_ipc_msg_rx_irq_handler(void * ctx)
{
    int id;
    const struct IPC_MSG_T *msg_ptr;
    unsigned int processed;
    struct HAL_IPC_MSG_T *ipc_msg_ctx = ctx;

    for (id = HAL_IPC_SUB_ID_0; id < HAL_IPC_SUB_ID_QTY; id++) {
        if (!ipc_msg_ctx->chan_opened[id]){
            continue;
        }
        if (ipc_msg_ctx->hal_op.irq_active(id, HAL_IPC_IRQ_SEND_IND)) {
            ipc_msg_ctx->hal_op.local_irq_mask_clr(id, HAL_IPC_IRQ_SEND_IND);
            if (ipc_msg_ctx->recv_pending_head[id].data) {
                // Previous unprocessed message
                msg_ptr = &ipc_msg_ctx->recv_pending_head[id];
            } else {
                // New message
                msg_ptr = ipc_msg_ctx->recv_msg_list_p[id];
            }
            while (msg_ptr) {
#if (IPC_MSG_VERBOSE_TRACE & (1 << 0))
                TR_INFO(0, "IPC-MSG-RX:%d: msg=%p len=%u data=%p content=0x%08X 0x%08X",
                    id, msg_ptr, msg_ptr->len, msg_ptr->data, msg_ptr->data ? *(uint32_t *)msg_ptr->data : 0,
                    msg_ptr->data ? *(uint32_t *)(msg_ptr->data + 4) : 0);
#endif
                if (ipc_msg_ctx->rx_irq_handler[id]) {
                    processed = ipc_msg_ctx->rx_irq_handler[id](msg_ptr->data, msg_ptr->len);
                    // Check if flow control needed
                    if (processed < msg_ptr->len) {
                        ipc_msg_ctx->recv_pending_head[id].next = msg_ptr->next;
                        ipc_msg_ctx->recv_pending_head[id].len = msg_ptr->len - processed;
                        ipc_msg_ctx->recv_pending_head[id].data = msg_ptr->data + processed;
                        break;
                    }
                } else {
                    // Error
                    ASSERT(false, "IPC-MSG-RX: Handler missing");
                    break;
                }
                msg_ptr = msg_ptr->next;
            }

            if (msg_ptr == NULL) {
                if (!ipc_msg_ctx->need_flow_ctrl[id]){
                    hal_ipc_msg_rx_done(ipc_msg_ctx, id);
                }
                ipc_msg_ctx->recv_pending_head[id].data = NULL;
            }
        }
    }
}

static void hal_ipc_msg_tx_irq_handler_by_id(void * ctx, enum HAL_IPC_SUB_ID_T id)
{
    struct IPC_MSG_T *msg_ptr;
    struct HAL_IPC_MSG_T *ipc_msg_ctx = ctx;

    if (ipc_msg_ctx->hal_op.irq_active(id, HAL_IPC_IRQ_RECV_DONE)) {
        ipc_msg_ctx->hal_op.local_irq_clear(id, HAL_IPC_IRQ_RECV_DONE);

        msg_ptr = ipc_msg_ctx->send_msg_list_p[id];
        while (msg_ptr) {
            if (ipc_msg_ctx->tx_irq_handler[id]) {
                ipc_msg_ctx->tx_irq_handler[id](msg_ptr->data, msg_ptr->len);
            };
            CONTAINER_OF(msg_ptr, struct IPC_SEND_RECORD_T, msg)->in_use = false;
            msg_ptr = msg_ptr->next;
        }

        if (ipc_msg_ctx->send_pending_list_p[id]) {
            ipc_msg_ctx->send_msg_list_p[id] = ipc_msg_ctx->send_pending_list_p[id];
            ipc_msg_ctx->send_pending_list_p[id] = NULL;
            ipc_msg_ctx->hal_op.peer_irq_set(id, HAL_IPC_IRQ_SEND_IND);
        } else {
            ipc_msg_ctx->send_msg_list_p[id] = NULL;
            // Allow sleep
            hal_ipc_msg_busy(ipc_msg_ctx, id, false);
        }
    }
}

static void hal_ipc_msg_tx_irq_handler(void * ctx)
{
    int id;
    struct HAL_IPC_MSG_T *ipc_msg_ctx = ctx;

    for (id = HAL_IPC_SUB_ID_0; id < HAL_IPC_SUB_ID_QTY; id++) {
        if (!ipc_msg_ctx->chan_opened[id]){
            continue;
        }
        hal_ipc_msg_tx_irq_handler_by_id(ipc_msg_ctx, id);
    }
}

void *hal_ipc_msg_ctx_get(enum HAL_IPC_CORE_TYPE_T core)
{
    if (core >= HAL_IPC_CORE_QTY) {
        return NULL;
    }
    return (void *)&hal_ipc_msg_pool[core];
}

int hal_ipc_msg_open(struct HAL_IPC_MSG_T *ipc_msg_ctx, enum HAL_IPC_SUB_ID_T id, bool rx_flowctrl)
{
    struct HAL_IPC_IRQ_CB_T irq_cb;
    int i = 0;

    if (id >= HAL_IPC_SUB_ID_QTY) {
        return 1;
    }

    if (!ipc_msg_ctx->chan_opened[id]) {
        ipc_msg_ctx->send_msg_list_p[id] = NULL;
        ipc_msg_ctx->send_pending_list_p[id] = NULL;
        ipc_msg_ctx->recv_pending_head[id].data = NULL;
        for (i = 0; i < HAL_IPC_MAX_SEND_RECORD_COUNT; i++) {
            ipc_msg_ctx->send_msgs[id][i].in_use = false;
        }
        ipc_msg_ctx->need_flow_ctrl[id] = rx_flowctrl;
        if (ipc_msg_ctx->peer_recv_msg_ptr){
            *(uint32_t *)ipc_msg_ctx->peer_recv_msg_ptr = (uint32_t)&(ipc_msg_ctx->send_msg_list_p[0]);
        }
        TR_INFO(0, "%s:%d: rxPtr=%p txPtr=%p flowCtrl=%d", __func__, id, &ipc_msg_ctx->recv_msg_list_p[id], &ipc_msg_ctx->send_msg_list_p[id], ipc_msg_ctx->need_flow_ctrl[id]);
        irq_cb.manual_rx_handler = hal_ipc_msg_rx_irq_handler;
        irq_cb.manual_tx_handler = hal_ipc_msg_tx_irq_handler;
        irq_cb.rx_handler = NULL;
        irq_cb.tx_handler = NULL;
        ipc_msg_ctx->hal_op.open((void *)ipc_msg_ctx, id, irq_cb);
    } else {
        ASSERT(ipc_msg_ctx->need_flow_ctrl[id] == rx_flowctrl, "IPC-MSG-OPEN: rx_flowctrl=%d (should be %d)", rx_flowctrl, ipc_msg_ctx->need_flow_ctrl[id]);
        return 3;
    }

    ipc_msg_ctx->chan_opened[id] = true;

    return 0;
}

int hal_ipc_msg_close(struct HAL_IPC_MSG_T *ipc_msg_ctx, enum HAL_IPC_SUB_ID_T id)
{
    if (id >= HAL_IPC_SUB_ID_QTY) {
        return 1;
    }

    ipc_msg_ctx->chan_opened[id] = false;
    ipc_msg_ctx->rx_irq_handler[id] = NULL;
    ipc_msg_ctx->tx_irq_handler[id] = NULL;

    ipc_msg_ctx->hal_op.close(id);

    ipc_msg_ctx->send_msg_list_p[id] = NULL;
    ipc_msg_ctx->send_pending_list_p[id] = NULL;
    ipc_msg_ctx->recv_pending_head[id].data = NULL;
    ipc_msg_ctx->need_flow_ctrl[id] = false;

    return 0;
}

int hal_ipc_msg_start_recv(struct HAL_IPC_MSG_T *ipc_msg_ctx, enum HAL_IPC_SUB_ID_T id)
{
    if (id >= HAL_IPC_SUB_ID_QTY) {
        return 1;
    }

    ipc_msg_ctx->hal_op.start_recv(id);

    // Check if there is any previous unprocessed message
    if (ipc_msg_ctx->recv_pending_head[id].data) {
        ipc_msg_ctx->hal_op.local_irq_mask_set(id, HAL_IPC_IRQ_SEND_IND);
    }

    return 0;
}

int hal_ipc_msg_stop_recv(struct HAL_IPC_MSG_T *ipc_msg_ctx, enum HAL_IPC_SUB_ID_T id)
{
    if (id >= HAL_IPC_SUB_ID_QTY) {
        return 1;
    }

    ipc_msg_ctx->hal_op.start_recv(id);

    return 0;
}

int hal_ipc_msg_send_seq(struct HAL_IPC_MSG_T *ipc_msg_ctx, enum HAL_IPC_SUB_ID_T id, const void *data, unsigned int len, unsigned int *seq)
{
    uint32_t lock;
    int ret;
    struct IPC_SEND_RECORD_T *record;
    struct IPC_MSG_T *next;
    int i;

    if (id >= HAL_IPC_SUB_ID_QTY) {
        return 1;
    }
    if (!ipc_msg_ctx->chan_opened[id]) {
        return 2;
    }

    ret = -1;
    record = &ipc_msg_ctx->send_msgs[id][0];

    lock = int_lock();
    for (i = 0; i < HAL_IPC_MAX_SEND_RECORD_COUNT; i++) {
        if (record->in_use) {
            record++;
            continue;
        }
        record->in_use = true;
        record->msg.next = NULL;
        record->msg.len = len;
        record->msg.data = data;
#if (IPC_MSG_VERBOSE_TRACE & (1 << 1))
        TR_INFO(0, "IPC-MSG-TX:%d: msg=%p len=%u data=%p content=0x%08X 0x%08X",
            id, record, len, data, data ? *(uint32_t *)data : 0, data ? *(uint32_t *)(data + 4) : 0);
#endif
        if (ipc_msg_ctx->send_msg_list_p[id] == NULL) {
            ipc_msg_ctx->send_msg_list_p[id] = &record->msg;
            ipc_msg_ctx->hal_op.peer_irq_set(id, HAL_IPC_IRQ_SEND_IND);
        } else if (ipc_msg_ctx->send_pending_list_p[id] == NULL) {
            ipc_msg_ctx->send_pending_list_p[id] = &record->msg;
        } else {
            next = ipc_msg_ctx->send_pending_list_p[id];
            while (next->next) {
                next = next->next;
            }
            next->next = &record->msg;
        }
        ret = 0;
        if (seq) {
            *seq = i;
        }
        // Prohibit sleep here
        hal_ipc_msg_busy(ipc_msg_ctx, id, true);
        break;
    }
    int_unlock(lock);

    return ret;
}

int hal_ipc_msg_send(struct HAL_IPC_MSG_T *ipc_msg_ctx, enum HAL_IPC_SUB_ID_T id, const void *data, unsigned int len)
{
    return hal_ipc_msg_send_seq(ipc_msg_ctx, id, data, len, NULL);
}

void hal_ipc_msg_rx_done(struct HAL_IPC_MSG_T *ipc_msg_ctx, enum HAL_IPC_SUB_ID_T id)
{
    ipc_msg_ctx->hal_op.local_irq_clear(id, HAL_IPC_IRQ_SEND_IND);
    ipc_msg_ctx->hal_op.local_irq_mask_set(id, HAL_IPC_IRQ_SEND_IND);
}

void hal_ipc_msg_tx_irq_run(struct HAL_IPC_MSG_T *ipc_msg_ctx, enum HAL_IPC_SUB_ID_T id)
{
    hal_ipc_msg_tx_irq_handler_by_id(ipc_msg_ctx, id);
}

int hal_ipc_msg_tx_active(struct HAL_IPC_MSG_T *ipc_msg_ctx, enum HAL_IPC_SUB_ID_T id, unsigned int seq)
{
    if (seq < ARRAY_SIZE(ipc_msg_ctx->send_msgs[0])) {
        return ipc_msg_ctx->send_msgs[id][seq].in_use;
    }
    return false;
}

int hal_ipc_msg_opened(struct HAL_IPC_MSG_T *ipc_msg_ctx, enum HAL_IPC_SUB_ID_T id)
{
    return !!ipc_msg_ctx->chan_opened[id];
}
#endif

