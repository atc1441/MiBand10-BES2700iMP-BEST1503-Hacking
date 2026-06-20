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
#ifndef __HAL_SYS2BTH_H__
#define __HAL_SYS2BTH_H__

#include "hal_rmt_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

enum HAL_SYS2BTH_ID_T {
    HAL_SYS2BTH_ID_0,
    HAL_SYS2BTH_ID_1,

    HAL_SYS2BTH_ID_QTY
};

typedef unsigned int (*HAL_SYS2BTH_RX_IRQ_HANDLER)(const void *data, unsigned int len);

typedef void (*HAL_SYS2BTH_TX_IRQ_HANDLER)(const void *data, unsigned int len);

typedef void (*HAL_SYS2BTH_NOTIFY_IRQ_HANDLER)(uint32_t core, uint32_t id);

int hal_sys2bth_config_get(struct HAL_RMT_IPC_CFG_T *cfg);

int hal_sys2bth_config_set(const struct HAL_RMT_IPC_CFG_T *cfg);

int hal_sys2bth_open(enum HAL_SYS2BTH_ID_T id, HAL_SYS2BTH_RX_IRQ_HANDLER rxhandler, HAL_SYS2BTH_TX_IRQ_HANDLER txhandler, int rx_flowctrl);

int hal_sys2bth_close(enum HAL_SYS2BTH_ID_T id);

int hal_sys2bth_start_recv(enum HAL_SYS2BTH_ID_T id);

int hal_sys2bth_stop_recv(enum HAL_SYS2BTH_ID_T id);

int hal_sys2bth_send_seq(enum HAL_SYS2BTH_ID_T id, const void *data, unsigned int len, unsigned int *seq);

int hal_sys2bth_send(enum HAL_SYS2BTH_ID_T id, const void *data, unsigned int len);

int hal_sys2bth_rx_done(enum HAL_SYS2BTH_ID_T id);

int hal_sys2bth_tx_irq_run(enum HAL_SYS2BTH_ID_T id);

int hal_sys2bth_tx_active(enum HAL_SYS2BTH_ID_T id, unsigned int seq);

int hal_sys2bth_opened(enum HAL_SYS2BTH_ID_T id);

int hal_sys2bth_set_chan_state(enum HAL_SYS2BTH_ID_T id, int opened);

int hal_sys2bth_rx_irq_pending(enum HAL_SYS2BTH_ID_T id);

int hal_sys2bth_ipc_notify_open(enum HAL_SYS2BTH_ID_T id, HAL_SYS2BTH_NOTIFY_IRQ_HANDLER rxhandler, HAL_SYS2BTH_NOTIFY_IRQ_HANDLER txhandler);

int hal_sys2bth_ipc_notify_start_recv(enum HAL_SYS2BTH_ID_T id);

int hal_sys2bth_ipc_notify_interrupt_core(enum HAL_SYS2BTH_ID_T id);

int hal_sys2bth_ipc_notify_close(enum HAL_SYS2BTH_ID_T id);


#ifdef __cplusplus
}
#endif

#endif
