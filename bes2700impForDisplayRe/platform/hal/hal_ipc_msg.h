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
#ifndef __HAL_IPC_MSG_H__
#define __HAL_IPC_MSG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include CHIP_SPECIFIC_HDR(hal_ipc_mcu2sys)

#include "hal_cmu.h"
#include "stdbool.h"
#include "hal_ipc.h"

#define HAL_IPC_MAX_SEND_RECORD_COUNT               3

#define IPC_MSG_MCU2SYS_MAILBOX_BASE (0x20541000)
#define IPC_MSG_MCU2SYS_MAILBOX_RX_MSG_PTR_LOC (IPC_MSG_MCU2SYS_MAILBOX_BASE + sizeof(struct IPC_MSG_T *) * HAL_IPC_SUB_ID_QTY)

typedef unsigned int (*HAL_IPC_MSG_RX_IRQ_HANDLER)(const void *data, unsigned int len);
typedef void (*HAL_IPC_MSG_TX_IRQ_HANDLER)(const void *data, unsigned int len);

struct IPC_MSG_T {
    struct IPC_MSG_T *next;        // pointer to next element in the list
    unsigned int len;                       // message data length in bytes
    const void *data;                       // pointer to the message data
};

struct IPC_SEND_RECORD_T {
    struct IPC_MSG_T msg;
    bool in_use;
};

struct HAL_IPC_MSG_T{
    uint32_t peer_recv_msg_ptr;
    struct IPC_MSG_T ** recv_msg_list_p;
#ifdef CHIP_SUBSYS_SENS
    struct IPC_MSG_T ** send_msg_list_p;
#else
    struct IPC_MSG_T * send_msg_list_p[HAL_IPC_SUB_ID_QTY];
#endif
    struct IPC_MSG_T * send_pending_list_p[HAL_IPC_SUB_ID_QTY];
    struct IPC_MSG_T recv_pending_head[HAL_IPC_SUB_ID_QTY];
    struct IPC_SEND_RECORD_T send_msgs[HAL_IPC_SUB_ID_QTY][HAL_IPC_MAX_SEND_RECORD_COUNT];
    HAL_IPC_MSG_RX_IRQ_HANDLER rx_irq_handler[HAL_IPC_SUB_ID_QTY];
    HAL_IPC_MSG_TX_IRQ_HANDLER tx_irq_handler[HAL_IPC_SUB_ID_QTY];
    bool chan_opened[HAL_IPC_SUB_ID_QTY];
    bool need_flow_ctrl[HAL_IPC_SUB_ID_QTY];
    bool chan_busy[HAL_IPC_SUB_ID_QTY];
    bool busy_now;
    struct IPC_HALOP_T hal_op; 
};

void *hal_ipc_msg_ctx_get(enum HAL_IPC_CORE_TYPE_T core);

int hal_ipc_msg_open(struct HAL_IPC_MSG_T *ipc_msg_ctx, enum HAL_IPC_SUB_ID_T id, bool rx_flowctrl);

int hal_ipc_msg_close(struct HAL_IPC_MSG_T *ipc_msg_ctx, enum HAL_IPC_SUB_ID_T id);

int hal_ipc_msg_start_recv(struct HAL_IPC_MSG_T *ipc_msg_ctx, enum HAL_IPC_SUB_ID_T id);

int hal_ipc_msg_stop_recv(struct HAL_IPC_MSG_T *ipc_msg_ctx, enum HAL_IPC_SUB_ID_T id);

int hal_ipc_msg_send_seq(struct HAL_IPC_MSG_T *ipc_msg_ctx, enum HAL_IPC_SUB_ID_T id, const void *data, unsigned int len, unsigned int *seq);

int hal_ipc_msg_send(struct HAL_IPC_MSG_T *ipc_msg_ctx, enum HAL_IPC_SUB_ID_T id, const void *data, unsigned int len);

void hal_ipc_msg_rx_done(struct HAL_IPC_MSG_T *ipc_msg_ctx, enum HAL_IPC_SUB_ID_T id);

void hal_ipc_msg_tx_irq_run(struct HAL_IPC_MSG_T *ipc_msg_ctx, enum HAL_IPC_SUB_ID_T id);

int hal_ipc_msg_tx_active(struct HAL_IPC_MSG_T *ipc_msg_ctx, enum HAL_IPC_SUB_ID_T id, unsigned int seq);

int hal_ipc_msg_opened(struct HAL_IPC_MSG_T *ipc_msg_ctx, enum HAL_IPC_SUB_ID_T id);


#ifdef __cplusplus
}
#endif

#endif

