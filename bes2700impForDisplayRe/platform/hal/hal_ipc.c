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
#ifdef __HAL_IPC_SUPPORT__
#include "plat_addr_map.h"
#include "cmsis_nvic.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_ipc.h"
#include CHIP_SPECIFIC_HDR(hal_ipc_mcu2bt)
#include CHIP_SPECIFIC_HDR(hal_ipc_mcu2sys)
#include CHIP_SPECIFIC_HDR(hal_ipc_mcu2sens)

int hal_ipc_interrupt_core(enum HAL_IPC_CORE_TYPE_T core, enum HAL_IPC_SUB_ID_T id)
{
    switch(core)
    {
        case HAL_IPC_CORE_AP_BT:
            hal_ipc_mcu2bt_peer_irq_set(id, HAL_IPC_IRQ_SEND_IND);
            break;
        case HAL_IPC_CORE_AP_SYS:
            hal_ipc_mcu2sys_peer_irq_set(id, HAL_IPC_IRQ_SEND_IND);
            break;
        case HAL_IPC_CORE_SENSORHUB:
            hal_ipc_mcu2sens_peer_irq_set(id, HAL_IPC_IRQ_SEND_IND);
            break;
        default:
            break;
    }
    return 0;
}

int hal_ipc_open(enum HAL_IPC_CORE_TYPE_T core, enum HAL_IPC_SUB_ID_T id, struct HAL_IPC_IRQ_CB_T irq_cb)
{
    TRACE_HAL_IPC_I("%s core:%d id:%d", __func__, core, id);

    if (core >= HAL_IPC_CORE_QTY) {
        return 1;
    }
    switch(core)
    {
        case HAL_IPC_CORE_AP_BT:
            hal_ipc_mcu2bt_open(NULL, id, irq_cb);
            hal_ipc_mcu2bt_start_recv(id);
            break;
        case HAL_IPC_CORE_AP_SYS:
            hal_ipc_mcu2sys_open(NULL, id, irq_cb);
            hal_ipc_mcu2sys_start_recv(id);
            break;
        case HAL_IPC_CORE_SENSORHUB:
            hal_ipc_mcu2sens_open(NULL, id, irq_cb);
            hal_ipc_mcu2sens_start_recv(id);
            break;
        default:
            break;
    }
    return 0;
}

int hal_ipc_close(enum HAL_IPC_CORE_TYPE_T core, enum HAL_IPC_SUB_ID_T id)
{
    TRACE_HAL_IPC_I("%s core:%d id:%d", __func__, core, id);

    if (core >= HAL_IPC_CORE_QTY) {
        return 1;
    }
    switch(core)
    {
        case HAL_IPC_CORE_AP_BT:
            hal_ipc_mcu2bt_close(id);
            break;
        case HAL_IPC_CORE_AP_SYS:
            hal_ipc_mcu2sys_close(id);
            break;
        case HAL_IPC_CORE_SENSORHUB:
            hal_ipc_mcu2sens_close(id);
            break;
        default:
            break;
    }
    return 0;
}
#endif
