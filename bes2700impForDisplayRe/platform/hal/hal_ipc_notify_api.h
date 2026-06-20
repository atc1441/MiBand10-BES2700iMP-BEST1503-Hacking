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
#ifndef __HAL_IPC_NOTIFY_API__
#define __HAL_IPC_NOTIFY_API__

#ifdef __cplusplus
extern "C" {
#endif

#include "cmsis.h"
#include "stdbool.h"

enum HAL_IPC_NOTIFY_CORE_TYPE_T {
    HAL_IPC_NOTIFY_CORE_SYS_BTH,
    HAL_IPC_NOTIFY_CORE_SYS_BTC,
    HAL_IPC_NOTIFY_CORE_SYS_SENS,
    HAL_IPC_NOTIFY_CORE_SYS_DSP,
    HAL_IPC_NOTIFY_CORE_BTH_BTC,
    HAL_IPC_NOTIFY_CORE_BTH_SENS,

    HAL_IPC_NOTIFY_CORE_QTY
};

enum HAL_IPC_NOTIFY_SUB_ID_T {
    HAL_IPC_NOTIFY_SUB_ID_0,
    HAL_IPC_NOTIFY_SUB_ID_1,
    HAL_IPC_NOTIFY_SUB_ID_QTY
};

typedef void (*HAL_IPC_NOTIFY_IRQ_HANDLER)(uint32_t core, uint32_t id);

struct HAL_IPC_NOTIFY_IRQ_CB_T {
    HAL_IPC_NOTIFY_IRQ_HANDLER rx_handler;
    HAL_IPC_NOTIFY_IRQ_HANDLER tx_handler;
};

int hal_ipc_notify_interrupt_core(enum HAL_IPC_NOTIFY_CORE_TYPE_T core, enum HAL_IPC_NOTIFY_SUB_ID_T id);
int hal_ipc_notify_open(enum HAL_IPC_NOTIFY_CORE_TYPE_T core, enum HAL_IPC_NOTIFY_SUB_ID_T id, struct HAL_IPC_NOTIFY_IRQ_CB_T irq_cb);
int hal_ipc_notify_close(enum HAL_IPC_NOTIFY_CORE_TYPE_T core, enum HAL_IPC_NOTIFY_SUB_ID_T id);

#ifdef __cplusplus
}
#endif

#endif

