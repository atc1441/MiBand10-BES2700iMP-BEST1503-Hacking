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
#ifndef __HAL_IPC_NOTIFY__
#define __HAL_IPC_NOTIFY__


#ifdef __cplusplus
extern "C" {
#endif

#include "cmsis.h"

#define TRACE_HAL_IPC_NOTIFY_D(str, ...) TR_INFO(TR_ATTR_ARG_NUM(COUNT_ARG_NUM(unused, ##__VA_ARGS__)),     "[HAL][IPC]" str, ##__VA_ARGS__)
//hal_trace_dummy(str, ##__VA_ARGS__)
#define TRACE_HAL_IPC_NOTIFY_I(str, ...) TR_INFO(TR_ATTR_ARG_NUM(COUNT_ARG_NUM(unused, ##__VA_ARGS__)),     "[HAL][IPC]" str, ##__VA_ARGS__)

enum HAL_IPC_NOTIFY_IRQ_TYPE_T {
    HAL_IPC_NOTIFY_IRQ_SEND_IND,
    HAL_IPC_NOTIFY_IRQ_RECV_DONE,

    HAL_IPC_NOTIFY_IRQ_TYPE_QTY
};

typedef void (*HAL_IPC_NOTIFY_IRQ_ENTRY)(void);
typedef void (*HAL_IPC_NOTIFY_IRQ_INIT)(uint32_t id);
typedef void (*HAL_IPC_NOTIFY_PEER_TX_IRQ_SET)(uint32_t id);
typedef void (*HAL_IPC_NOTIFY_LOCAL_TX_IRQ_CLEAR)(uint32_t id);
typedef void (*HAL_IPC_NOTIFY_RX_DONE)(uint32_t id);
typedef int (*HAL_IPC_NOTIFY_IRQ_ACTIVE)(uint32_t id, uint32_t type);
typedef void (*HAL_IPC_NOTIFY_RX_IRQ_HANDLER)(uint32_t core, uint32_t id);

struct HAL_IPC_NOTIFY_CH_CFG_T {
    HAL_IPC_NOTIFY_RX_IRQ_HANDLER rx_irq_handler;
    HAL_IPC_NOTIFY_RX_IRQ_HANDLER tx_irq_handler;
    bool chan_opened;
};

struct HAL_IPC_NOTIFY_CFG_T {
    const char * name;
    enum HAL_IPC_NOTIFY_CORE_TYPE_T core;
    HAL_IPC_NOTIFY_IRQ_INIT irq_init;
    HAL_IPC_NOTIFY_PEER_TX_IRQ_SET peer_tx_irq_set;
    HAL_IPC_NOTIFY_LOCAL_TX_IRQ_CLEAR local_tx_irq_clear;
    HAL_IPC_NOTIFY_RX_DONE rx_done;
    HAL_IPC_NOTIFY_IRQ_ACTIVE irq_active;
    HAL_IPC_NOTIFY_IRQ_ENTRY rx_irq_entry;
    HAL_IPC_NOTIFY_IRQ_ENTRY tx_irq_entry;
    uint8_t chan_num;
    struct HAL_IPC_NOTIFY_CH_CFG_T *chan_cfg;
    const IRQn_Type * rx_irq_id;
    const IRQn_Type * tx_irq_id;
};

void hal_ipc_notify_internal_rx_irq_handler(const struct HAL_IPC_NOTIFY_CFG_T *cfg);
void hal_ipc_notify_internal_tx_irq_handler(const struct HAL_IPC_NOTIFY_CFG_T *cfg);
int hal_ipc_notify_internal_open(const struct HAL_IPC_NOTIFY_CFG_T *cfg, uint32_t id, HAL_IPC_NOTIFY_RX_IRQ_HANDLER rxhandler, HAL_IPC_NOTIFY_RX_IRQ_HANDLER txhandler);
int hal_ipc_notify_internal_start_recv(const struct HAL_IPC_NOTIFY_CFG_T *cfg, uint32_t id);
int hal_ipc_notify_internal_peer_irq_set(const struct HAL_IPC_NOTIFY_CFG_T *cfg, uint32_t id);
int hal_ipc_notify_internal_close(const struct HAL_IPC_NOTIFY_CFG_T *cfg, uint32_t id);
int hal_ipc_notify_internal_opened(const struct HAL_IPC_NOTIFY_CFG_T *cfg, uint32_t id);


#ifdef __cplusplus
}
#endif

#endif

