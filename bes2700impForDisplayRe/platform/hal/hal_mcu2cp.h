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
#ifndef __HAL_MCU2CP_H__
#define __HAL_MCU2CP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_rmt_ipc.h"

enum HAL_MCU2CP_ID_T {
    HAL_MCU2CP_ID_0,
    HAL_MCU2CP_ID_1,

    HAL_MCU2CP_ID_QTY
};

typedef HAL_RMT_IPC_RX_IRQ_HANDLER HAL_MCU2CP_RX_IRQ_HANDLER;

typedef HAL_RMT_IPC_TX_IRQ_HANDLER HAL_MCU2CP_TX_IRQ_HANDLER;

#if defined(CP_AS_SUBSYS) || defined(CHIP_ROLE_CP)

#include "hal_mcu2cp_subsys.h"

#else

int hal_mcu2cp_open_mcu(enum HAL_MCU2CP_ID_T id,
                        HAL_MCU2CP_RX_IRQ_HANDLER rxhandler, HAL_MCU2CP_TX_IRQ_HANDLER txhandler, int rx_flowctrl);
int hal_mcu2cp_open_cp (enum HAL_MCU2CP_ID_T id,
                        HAL_MCU2CP_RX_IRQ_HANDLER rxhandler, HAL_MCU2CP_TX_IRQ_HANDLER txhandler, int rx_flowctrl);

int hal_mcu2cp_close_mcu(enum HAL_MCU2CP_ID_T id);
int hal_mcu2cp_close_cp (enum HAL_MCU2CP_ID_T id);

int hal_mcu2cp_start_recv_mcu(enum HAL_MCU2CP_ID_T id);
int hal_mcu2cp_start_recv_cp (enum HAL_MCU2CP_ID_T id);

int hal_mcu2cp_stop_recv_mcu(enum HAL_MCU2CP_ID_T id);
int hal_mcu2cp_stop_recv_cp (enum HAL_MCU2CP_ID_T id);

int hal_mcu2cp_send_mcu(enum HAL_MCU2CP_ID_T id,
                    const unsigned char *data, unsigned int len);
int hal_mcu2cp_send_cp (enum HAL_MCU2CP_ID_T id,
                    const unsigned char *data, unsigned int len);

void hal_mcu2cp_rx_done_mcu(enum HAL_MCU2CP_ID_T id);
void hal_mcu2cp_rx_done_cp (enum HAL_MCU2CP_ID_T id);

int hal_mcu2cp_opened_mcu(enum HAL_MCU2CP_ID_T id);
int hal_mcu2cp_opened_cp(enum HAL_MCU2CP_ID_T id);

int hal_mcu2cp_local_irq_pending_mcu(enum HAL_MCU2CP_ID_T id);
int hal_mcu2cp_local_irq_pending_cp(enum HAL_MCU2CP_ID_T id);

void hal_mcu2cp_poll_tx_irq_mcu(enum HAL_MCU2CP_ID_T id);
void hal_mcu2cp_poll_tx_irq_cp(enum HAL_MCU2CP_ID_T id);
int hal_mcu2cp_force_flush_pending_mcu(enum HAL_MCU2CP_ID_T id);
int hal_mcu2cp_force_flush_pending_cp(enum HAL_MCU2CP_ID_T id);

#endif // !(CP_AS_SUBSYS || CHIP_ROLE_CP)

#ifdef __cplusplus
}
#endif

#endif

