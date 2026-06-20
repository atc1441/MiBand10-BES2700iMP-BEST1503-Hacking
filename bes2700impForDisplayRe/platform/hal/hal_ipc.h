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
#ifndef __HAL_IPC_BEST1501SIMU__
#define __HAL_IPC_BEST1501SIMU__



#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"
#include "plat_addr_map.h"


#define TRACE_HAL_IPC_D(str, ...)  hal_trace_dummy(str, ##__VA_ARGS__)
// TR_INFO(TR_ATTR_ARG_NUM(COUNT_ARG_NUM(unused, ##__VA_ARGS__)),     "[HAL][IPC]" str, ##__VA_ARGS__)
#define TRACE_HAL_IPC_I(str, ...) TR_INFO(TR_ATTR_ARG_NUM(COUNT_ARG_NUM(unused, ##__VA_ARGS__)),     "[HAL][IPC]" str, ##__VA_ARGS__)


enum HAL_IPC_CORE_TYPE_T {
    HAL_IPC_CORE_AP_BT,
    HAL_IPC_CORE_AP_SYS,
    HAL_IPC_CORE_BT_SYS,
    HAL_IPC_CORE_SENSORHUB,

    HAL_IPC_CORE_QTY
};

enum HAL_IPC_SUB_ID_T {
    HAL_IPC_SUB_ID_0,
    HAL_IPC_SUB_ID_1,
    HAL_IPC_SUB_ID_QTY
};

enum HAL_IPC_IRQ_TYPE_T {
    HAL_IPC_IRQ_SEND_IND,
    HAL_IPC_IRQ_RECV_DONE,

    HAL_IPC_IRQ_TYPE_QTY
};


typedef void (*HAL_IPC_RX_IRQ_HANDLER)(enum HAL_IPC_CORE_TYPE_T core, enum HAL_IPC_SUB_ID_T id);
typedef void (*HAL_IPC_TX_IRQ_HANDLER)(enum HAL_IPC_CORE_TYPE_T core, enum HAL_IPC_SUB_ID_T id);
typedef void (*HAL_IPC_MANUAL_IRQ_HANDLER)(void *);

struct HAL_IPC_IRQ_CB_T {
    HAL_IPC_MANUAL_IRQ_HANDLER manual_rx_handler;
    HAL_IPC_MANUAL_IRQ_HANDLER manual_tx_handler;
    HAL_IPC_RX_IRQ_HANDLER rx_handler;
    HAL_IPC_TX_IRQ_HANDLER tx_handler;
};

typedef int (*IPC_PEER_IRQ_SET)(enum HAL_IPC_SUB_ID_T, enum HAL_IPC_IRQ_TYPE_T);
typedef int (*IPC_LOCAL_IRQ_CLEAR)(enum HAL_IPC_SUB_ID_T, enum HAL_IPC_IRQ_TYPE_T);
typedef int (*IPC_LOCAL_IRQ_MASK_SET)(enum HAL_IPC_SUB_ID_T, enum HAL_IPC_IRQ_TYPE_T);
typedef int (*IPC_LOCAL_IRQ_MASK_CLR)(enum HAL_IPC_SUB_ID_T, enum HAL_IPC_IRQ_TYPE_T);
typedef int (*IPC_IRQ_ACTIVE)(enum HAL_IPC_SUB_ID_T, enum HAL_IPC_IRQ_TYPE_T);
typedef int (*IPC_RX_IRQ_PENDING)(enum HAL_IPC_SUB_ID_T);
typedef int (*IPC_START_RECV)(enum HAL_IPC_SUB_ID_T);
typedef int (*IPC_STOP_RECV)(enum HAL_IPC_SUB_ID_T);
typedef int (*IPC_OPEN)(void *, enum HAL_IPC_SUB_ID_T, struct HAL_IPC_IRQ_CB_T);
typedef int (*IPC_CLOSE)(enum HAL_IPC_SUB_ID_T);

struct IPC_HALOP_T{
    IPC_PEER_IRQ_SET peer_irq_set;
    IPC_LOCAL_IRQ_CLEAR local_irq_clear;
    IPC_LOCAL_IRQ_MASK_SET local_irq_mask_set;
    IPC_LOCAL_IRQ_MASK_CLR local_irq_mask_clr;
    IPC_IRQ_ACTIVE irq_active;
    IPC_RX_IRQ_PENDING irq_pending;
    IPC_START_RECV start_recv;
    IPC_STOP_RECV stop_recv;
    IPC_OPEN open;
    IPC_CLOSE close;
};

int hal_ipc_interrupt_core(enum HAL_IPC_CORE_TYPE_T core, enum HAL_IPC_SUB_ID_T id);
int hal_ipc_open(enum HAL_IPC_CORE_TYPE_T core, enum HAL_IPC_SUB_ID_T id, struct HAL_IPC_IRQ_CB_T irq_cb);
int hal_ipc_close(enum HAL_IPC_CORE_TYPE_T core, enum HAL_IPC_SUB_ID_T id);

#ifdef __cplusplus
}
#endif

#endif

