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
#include "plat_addr_map.h"

#if defined(BTH_CMU_BASE) && !defined(CHIP_SUBSYS_SENS)

#include "cmsis_nvic.h"
#include "hal_mcu2btc.h"
#include "hal_rmt_ipc.h"
#include "hal_ipc_notify_api.h"
#include "hal_ipc_notify.h"
#include "hal_trace.h"
#include CHIP_SPECIFIC_HDR(reg_btcmu)
#ifdef CHIP_SUBSYS_BTH
#include CHIP_SPECIFIC_HDR(reg_bthcmu)
#elif defined(CHIP_SUBSYS_SENS)
#include CHIP_SPECIFIC_HDR(reg_senscmu)
#else
#include CHIP_SPECIFIC_HDR(reg_cmu)
#endif


#ifdef CHIP_SUBSYS_SENS
static struct SENSCMU_T * const cmu = (struct SENSCMU_T *)SENS_CMU_BASE;
#elif defined(CHIP_SUBSYS_BTH)
static struct BTHCMU_T * const cmu = (struct BTHCMU_T *)BTH_CMU_BASE;
#else
static struct CMU_T * const cmu = (struct CMU_T *)CMU_BASE;
#endif

#define CHIP_AS_BTH 1

static struct BTCMU_T * const btcmu = (struct BTCMU_T *)BT_CMU_BASE;

static const IRQn_Type rx_irq_id[HAL_MCU2BTC_ID_QTY] = {
    ISDATA_IRQn,
    ISDATA1_IRQn,
};

static const IRQn_Type tx_irq_id[HAL_MCU2BTC_ID_QTY] = {
    ISDONE_IRQn,
    ISDONE1_IRQn,
};

static int hal_mcu2btc_peer_irq_set(enum HAL_MCU2BTC_ID_T id, enum HAL_RMT_IPC_IRQ_TYPE_T type)
{
    uint32_t value;

    if (type != HAL_RMT_IPC_IRQ_SEND_IND) {
        return 1;
    }

#ifdef CHIP_AS_BTH
    if (id == HAL_MCU2BTC_ID_0) {
        value = CMU_MCU2BT_DATA_IND_SET;
    } else {
        value = CMU_MCU2BT_DATA1_IND_SET;
    }

    cmu->ISIRQ_SET = value;
    cmu->ISIRQ_SET;
#else
    if (id == HAL_MCU2BTC_ID_0) {
        value = BT_CMU_BT2MCU_DATA_IND_SET;
    } else {
        value = BT_CMU_BT2MCU_DATA1_IND_SET;
    }

    btcmu->ISIRQ_SET = value;
#endif

    return 0;
}

static int hal_mcu2btc_local_irq_clear(enum HAL_MCU2BTC_ID_T id, enum HAL_RMT_IPC_IRQ_TYPE_T type)
{
    uint32_t value;

#ifdef CHIP_AS_BTH
    if (id == HAL_MCU2BTC_ID_0) {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = BT_CMU_BT2MCU_DATA_IND_CLR;
        } else {
            value = CMU_BT2MCU_DATA_DONE_CLR;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = BT_CMU_BT2MCU_DATA1_IND_CLR;
        } else {
            value = CMU_BT2MCU_DATA1_DONE_CLR;
        }
    }

    if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
        btcmu->ISIRQ_CLR = value;
        // Flush the clear operation immediately
        btcmu->ISIRQ_CLR;
    } else {
        cmu->ISIRQ_CLR = value;
    }
#else
    if (id == HAL_MCU2BTC_ID_0) {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_MCU2BT_DATA_IND_CLR;
        } else {
            value = BT_CMU_MCU2BT_DATA_DONE_CLR;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_MCU2BT_DATA1_IND_CLR;
        } else {
            value = BT_CMU_MCU2BT_DATA1_DONE_CLR;
        }
    }

    if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
        cmu->ISIRQ_CLR = value;
        // Flush the clear operation immediately
        cmu->ISIRQ_CLR;
    } else {
        btcmu->ISIRQ_CLR = value;
    }
#endif

    return 0;
}

static int hal_mcu2btc_local_irq_mask_set(enum HAL_MCU2BTC_ID_T id, enum HAL_RMT_IPC_IRQ_TYPE_T type)
{
    uint32_t value;

#ifdef CHIP_AS_BTH
    if (id == HAL_MCU2BTC_ID_0) {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA_MSK_SET;
        } else {
            value = CMU_MCU2BT_DATA_MSK_SET;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA1_MSK_SET;
        } else {
            value = CMU_MCU2BT_DATA1_MSK_SET;
        }
    }

    cmu->ISIRQ_SET = value;
#else
    if (id == HAL_MCU2BTC_ID_0) {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = BT_CMU_MCU2BT_DATA_MSK_SET;
        } else {
            value = BT_CMU_BT2MCU_DATA_MSK_SET;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = BT_CMU_MCU2BT_DATA1_MSK_SET;
        } else {
            value = BT_CMU_BT2MCU_DATA1_MSK_SET;
        }
    }
    btcmu->BT2MCUIRQ_SET = value;
#endif

    return 0;
}

static int hal_mcu2btc_local_irq_mask_clear(enum HAL_MCU2BTC_ID_T id, enum HAL_RMT_IPC_IRQ_TYPE_T type)
{
    uint32_t value;

#ifdef CHIP_AS_BTH
    if (id == HAL_MCU2BTC_ID_0) {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA_MSK_CLR;
        } else {
            value = CMU_MCU2BT_DATA_MSK_CLR;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA1_MSK_CLR;
        } else {
            value = CMU_MCU2BT_DATA1_MSK_CLR;
        }
    }

    cmu->ISIRQ_CLR = value;
#else
    if (id == HAL_MCU2BTC_ID_0) {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = BT_CMU_MCU2BT_DATA_MSK_CLR;
        } else {
            value = BT_CMU_BT2MCU_DATA_MSK_CLR;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = BT_CMU_MCU2BT_DATA1_MSK_CLR;
        } else {
            value = BT_CMU_BT2MCU_DATA1_MSK_CLR;
        }
    }

    btcmu->BT2MCUIRQ_CLR = value;
#endif

    return 0;
}

static void hal_mcu2btc_irq_init(uint32_t id32)
{
    enum HAL_MCU2BTC_ID_T id = (enum HAL_MCU2BTC_ID_T)id32;

#ifdef CHIP_AS_BTH
    if (id == HAL_MCU2BTC_ID_0) {
        cmu->ISIRQ_CLR = CMU_BT2MCU_DATA_DONE_CLR | CMU_MCU2BT_DATA_IND_CLR;
    } else {
        cmu->ISIRQ_CLR = CMU_BT2MCU_DATA1_DONE_CLR | CMU_MCU2BT_DATA1_IND_CLR;
    }
#else
    if (id == HAL_MCU2BTC_ID_0) {
        btcmu->ISIRQ_CLR = BT_CMU_MCU2BT_DATA_DONE_CLR|BT_CMU_BT2MCU_DATA_IND_CLR;
    } else {
        btcmu->ISIRQ_CLR = BT_CMU_MCU2BT_DATA1_DONE_CLR|BT_CMU_BT2MCU_DATA1_IND_CLR;
    }
#endif

    hal_mcu2btc_local_irq_mask_set(id, HAL_RMT_IPC_IRQ_SEND_IND);
    hal_mcu2btc_local_irq_mask_set(id, HAL_RMT_IPC_IRQ_RECV_DONE);
}

static void hal_mcu2btc_peer_tx_irq_set(uint32_t id32)
{
    enum HAL_MCU2BTC_ID_T id = (enum HAL_MCU2BTC_ID_T)id32;

    hal_mcu2btc_peer_irq_set(id, HAL_RMT_IPC_IRQ_SEND_IND);
}

static void hal_mcu2btc_local_tx_irq_clear(uint32_t id32)
{
    enum HAL_MCU2BTC_ID_T id = (enum HAL_MCU2BTC_ID_T)id32;

    hal_mcu2btc_local_irq_clear(id, HAL_RMT_IPC_IRQ_RECV_DONE);
}

POSSIBLY_UNUSED static void hal_mcu2btc_rx_irq_suspend(uint32_t id32)
{
    enum HAL_MCU2BTC_ID_T id = (enum HAL_MCU2BTC_ID_T)id32;

    hal_mcu2btc_local_irq_mask_clear(id, HAL_RMT_IPC_IRQ_SEND_IND);
}

POSSIBLY_UNUSED static void hal_mcu2btc_rx_irq_resume(uint32_t id32)
{
    enum HAL_MCU2BTC_ID_T id = (enum HAL_MCU2BTC_ID_T)id32;

    hal_mcu2btc_local_irq_mask_set(id, HAL_RMT_IPC_IRQ_SEND_IND);
}

static void hal_mcu2btc_rx_done_id32(uint32_t id32)
{
    enum HAL_MCU2BTC_ID_T id = (enum HAL_MCU2BTC_ID_T)id32;

    hal_mcu2btc_local_irq_clear(id, HAL_RMT_IPC_IRQ_SEND_IND);
    hal_mcu2btc_local_irq_mask_set(id, HAL_RMT_IPC_IRQ_SEND_IND);
}

static int hal_mcu2btc_irq_active(uint32_t id32, uint32_t type)
{
    enum HAL_MCU2BTC_ID_T id = (enum HAL_MCU2BTC_ID_T)id32;
    uint32_t value;

#ifdef CHIP_AS_BTH
    if (id == HAL_MCU2BTC_ID_0) {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA_INTR_MSK;
        } else {
            value = CMU_MCU2BT_DATA_INTR_MSK;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA1_INTR_MSK;
        } else {
            value = CMU_MCU2BT_DATA1_INTR_MSK;
        }
    }

    return !!(cmu->ISIRQ_SET & value);
#else
    if (id == HAL_MCU2BTC_ID_0) {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = BT_CMU_MCU2BT_DATA_INTR_MSK;
        } else {
            value = BT_CMU_BT2MCU_DATA_INTR_MSK;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = BT_CMU_MCU2BT_DATA1_INTR_MSK;
        } else {
            value = BT_CMU_BT2MCU_DATA1_INTR_MSK;
        }
    }

    return !!(btcmu->BT2MCUIRQ_SET & value);
#endif
}


static struct HAL_IPC_NOTIFY_CH_CFG_T sys2bth_ipc_notify_chan_cfg[HAL_MCU2BTC_ID_QTY];
static void hal_mcu2btc_ipc_notify_rx_irq_handler(void);
static void hal_mcu2btc_ipc_notify_tx_irq_handler(void);


static const struct HAL_IPC_NOTIFY_CFG_T mcu2btc_ipc_notify_cfg = {
    .name = "SYS2BTH",
    .core = HAL_IPC_NOTIFY_CORE_BTH_BTC,

    .irq_init = hal_mcu2btc_irq_init,
    .peer_tx_irq_set = hal_mcu2btc_peer_tx_irq_set,
    .local_tx_irq_clear = hal_mcu2btc_local_tx_irq_clear,

    .rx_done = hal_mcu2btc_rx_done_id32,
    .irq_active = hal_mcu2btc_irq_active,
    .rx_irq_entry = hal_mcu2btc_ipc_notify_rx_irq_handler,
    .tx_irq_entry = hal_mcu2btc_ipc_notify_tx_irq_handler,
    .chan_num = HAL_MCU2BTC_ID_QTY,
    .chan_cfg = &sys2bth_ipc_notify_chan_cfg[0],

    .rx_irq_id = &rx_irq_id[0],
    .tx_irq_id = &tx_irq_id[0],

};

static void hal_mcu2btc_ipc_notify_rx_irq_handler(void)
{
    hal_ipc_notify_internal_rx_irq_handler(&mcu2btc_ipc_notify_cfg);
}

static void hal_mcu2btc_ipc_notify_tx_irq_handler(void)
{
    hal_ipc_notify_internal_tx_irq_handler(&mcu2btc_ipc_notify_cfg);
}

int hal_mcu2btc_ipc_notify_open(enum HAL_MCU2BTC_ID_T id, HAL_MCU2BTC_NOTIFY_IRQ_HANDLER rxhandler, HAL_MCU2BTC_NOTIFY_IRQ_HANDLER txhandler)
{
    return hal_ipc_notify_internal_open(&mcu2btc_ipc_notify_cfg, id, rxhandler, txhandler);
}

int hal_mcu2btc_ipc_notify_start_recv(enum HAL_MCU2BTC_ID_T id)
{
    return hal_ipc_notify_internal_start_recv(&mcu2btc_ipc_notify_cfg, id);
}

int hal_mcu2btc_ipc_notify_interrupt_core(enum HAL_MCU2BTC_ID_T id)
{
    return hal_ipc_notify_internal_peer_irq_set(&mcu2btc_ipc_notify_cfg, (uint32_t)id);
}

int hal_mcu2btc_ipc_notify_close(enum HAL_MCU2BTC_ID_T id)
{
    return hal_ipc_notify_internal_close(&mcu2btc_ipc_notify_cfg, (uint32_t)id);
}

#endif

