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
#include "hal_ipc_notify_api.h"
#include "hal_ipc_notify.h"
#include "hal_location.h"
#include "hal_rmt_ipc.h"
#include "hal_sys2bth.h"
#include "hal_trace.h"
#define CMU_HDR_CHECK_SUBSYS
#include CHIP_SPECIFIC_HDR(reg_cmu)
#include CHIP_SPECIFIC_HDR(reg_bthcmu)

#define HAL_SYS_WAKE_LOCK_USER_SYS2BTH      HAL_SYS_WAKE_LOCK_USER_6

#ifdef BTH_AS_MAIN_MCU
#ifdef __IPC_HIFI4_2_BTH_ONLY__
#define SYS_CFG_PTR_LOC                     (HIFI4_MAILBOX_BASE + 0)
#define BTH_CFG_PTR_LOC                     (HIFI4_MAILBOX_BASE + sizeof(struct HAL_RMT_IPC_CFG_T *))
STATIC_ASSERT((2 * sizeof(struct HAL_RMT_IPC_CFG_T *)) <= HIFI4_MAILBOX_SIZE, "HIFI4_MAILBOX_SIZE too small");
#else
#define SYS_CFG_PTR_LOC                     (M55_MAILBOX_BASE + 0)
#define BTH_CFG_PTR_LOC                     (M55_MAILBOX_BASE + sizeof(struct HAL_RMT_IPC_CFG_T *))
STATIC_ASSERT((2 * sizeof(struct HAL_RMT_IPC_CFG_T *)) <= M55_MAILBOX_SIZE, "M55_MAILBOX_SIZE too small");
#endif
#else
#define SYS_CFG_PTR_LOC                     (BTH_MAILBOX_BASE + 0)
#define BTH_CFG_PTR_LOC                     (BTH_MAILBOX_BASE + sizeof(struct HAL_RMT_IPC_CFG_T *))
STATIC_ASSERT((2 * sizeof(struct HAL_RMT_IPC_CFG_T *)) <= BTH_MAILBOX_SIZE, "BTH_MAILBOX_SIZE too small");
#endif

#ifndef __IPC_HIFI4_2_BTH_ONLY__
#define HIFI4_USE_SYS2BTH_1
#endif

#ifdef HIFI4_USE_SYS2BTH_1
#ifdef __IPC_HIFI4_2_BTH_ONLY__
#error "HIFI4_USE_SYS2BTH_1 conflicts with __IPC_HIFI4_2_BTH_ONLY__"
#endif

#define SYS_CFG_PTR_LOC1                    (HIFI4_MAILBOX_BASE + sizeof(struct HAL_RMT_IPC_CFG_T *) * 2)
#ifdef BTH_AS_MAIN_MCU
#define BTH_CFG_PTR_LOC1                    (HIFI4_MAILBOX_BASE + sizeof(struct HAL_RMT_IPC_CFG_T *) * 3)
STATIC_ASSERT((4 * sizeof(struct HAL_RMT_IPC_CFG_T *)) <= HIFI4_MAILBOX_SIZE, "HIFI4_MAILBOX_SIZE too small");
#else
#define BTH_CFG_PTR_LOC1                    (BTH_MAILBOX_BASE + sizeof(struct HAL_RMT_IPC_CFG_T *) * 2)
STATIC_ASSERT((3 * sizeof(struct HAL_RMT_IPC_CFG_T *)) <= BTH_MAILBOX_SIZE, "BTH_MAILBOX_SIZE too small");
#endif
#endif

static struct CMU_T * const cmu = (struct CMU_T *)SYS_CMU_BASE;
static struct BTHCMU_T * const bthcmu = (struct BTHCMU_T *)BTH_CMU_BASE;

static const IRQn_Type rx_irq_id[HAL_SYS2BTH_ID_QTY] = {
    BTH2SYS_DATA_IRQn,
    BTH2SYS_DATA1_IRQn,
};

static const IRQn_Type tx_irq_id[HAL_SYS2BTH_ID_QTY] = {
    SYS2BTH_DONE_IRQn,
    SYS2BTH_DONE1_IRQn,
};

#ifndef __IPC_HIFI4_2_BTH_ONLY__
const
#endif
static struct HAL_RMT_IPC_CFG_T sys2bth_cfg;

SYNC_FLAGS_LOC
static struct HAL_RMT_IPC_CH_CFG_T sys2bth_chan_cfg[HAL_SYS2BTH_ID_QTY];

static bool busy_now = false;

static int hal_sys2bth_peer_irq_set(enum HAL_SYS2BTH_ID_T id, enum HAL_RMT_IPC_IRQ_TYPE_T type)
{
    uint32_t value;

    if (type != HAL_RMT_IPC_IRQ_SEND_IND) {
        return 1;
    }

#ifdef CHIP_SUBSYS_BTH
    if (id == HAL_SYS2BTH_ID_0) {
        value = BTH_CMU_BTH2SYS_DATA_IND_SET;
    } else {
        value = BTH_CMU_BTH2SYS_DATA1_IND_SET;
    }

    bthcmu->BTH2SYS_IRQ_SET = value;
#else
    if (id == HAL_SYS2BTH_ID_0) {
        value = CMU_SYS2BTH_DATA_IND_SET;
    } else {
        value = CMU_SYS2BTH_DATA1_IND_SET;
    }

    cmu->SYS2BTH_IRQ_SET = value;
#endif

    return 0;
}

static int hal_sys2bth_local_irq_clear(enum HAL_SYS2BTH_ID_T id, enum HAL_RMT_IPC_IRQ_TYPE_T type)
{
    uint32_t value;

#ifdef CHIP_SUBSYS_BTH
    if (id == HAL_SYS2BTH_ID_0) {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_SYS2BTH_DATA_IND_CLR;
        } else {
            value = BTH_CMU_SYS2BTH_DATA_DONE_CLR;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_SYS2BTH_DATA1_IND_CLR;
        } else {
            value = BTH_CMU_SYS2BTH_DATA1_DONE_CLR;
        }
    }

    if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
        cmu->SYS2BTH_IRQ_CLR = value;
        // Flush the clear operation immediately
        cmu->SYS2BTH_IRQ_CLR;
    } else {
        bthcmu->BTH2SYS_IRQ_CLR = value;
    }
#else
    if (id == HAL_SYS2BTH_ID_0) {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = BTH_CMU_BTH2SYS_DATA_IND_CLR;
        } else {
            value = CMU_BTH2SYS_DATA_DONE_CLR;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = BTH_CMU_BTH2SYS_DATA1_IND_CLR;
        } else {
            value = CMU_BTH2SYS_DATA1_DONE_CLR;
        }
    }

    if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
        bthcmu->BTH2SYS_IRQ_CLR = value;
        // Flush the clear operation immediately
        bthcmu->BTH2SYS_IRQ_CLR;
    } else {
        cmu->SYS2BTH_IRQ_CLR = value;
        cmu->SYS2BTH_IRQ_CLR;
    }
    __DSB();
#endif

    return 0;
}

static int hal_sys2bth_local_irq_mask_set(enum HAL_SYS2BTH_ID_T id, enum HAL_RMT_IPC_IRQ_TYPE_T type)
{
    uint32_t value;

#ifdef CHIP_SUBSYS_BTH
    if (id == HAL_SYS2BTH_ID_0) {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = BTH_CMU_SYS2BTH_DATA_MSK_SET;
        } else {
            value = BTH_CMU_BTH2SYS_DATA_MSK_SET;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = BTH_CMU_SYS2BTH_DATA1_MSK_SET;
        } else {
            value = BTH_CMU_BTH2SYS_DATA1_MSK_SET;
        }
    }

    bthcmu->BTH2SYS_IRQ_SET = value;
#else
    if (id == HAL_SYS2BTH_ID_0) {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_BTH2SYS_DATA_MSK_SET;
        } else {
            value = CMU_SYS2BTH_DATA_MSK_SET;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_BTH2SYS_DATA1_MSK_SET;
        } else {
            value = CMU_SYS2BTH_DATA1_MSK_SET;
        }
    }

    cmu->SYS2BTH_IRQ_SET = value;
#endif

    return 0;
}

static int hal_sys2bth_local_irq_mask_clear(enum HAL_SYS2BTH_ID_T id, enum HAL_RMT_IPC_IRQ_TYPE_T type)
{
    uint32_t value;

#ifdef CHIP_SUBSYS_BTH
    if (id == HAL_SYS2BTH_ID_0) {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = BTH_CMU_SYS2BTH_DATA_MSK_CLR;
        } else {
            value = BTH_CMU_BTH2SYS_DATA_MSK_CLR;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = BTH_CMU_SYS2BTH_DATA1_MSK_CLR;
        } else {
            value = BTH_CMU_BTH2SYS_DATA1_MSK_CLR;
        }
    }

    bthcmu->BTH2SYS_IRQ_CLR = value;
#else
    if (id == HAL_SYS2BTH_ID_0) {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_BTH2SYS_DATA_MSK_CLR;
        } else {
            value = CMU_SYS2BTH_DATA_MSK_CLR;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_BTH2SYS_DATA1_MSK_CLR;
        } else {
            value = CMU_SYS2BTH_DATA1_MSK_CLR;
        }
    }

    cmu->SYS2BTH_IRQ_CLR = value;
    cmu->SYS2BTH_IRQ_CLR;
    __DSB();
#endif

    return 0;
}

int hal_sys2bth_rx_irq_pending(enum HAL_SYS2BTH_ID_T id)
{
    uint32_t value;

#ifdef CHIP_SUBSYS_BTH
    if (id == HAL_SYS2BTH_ID_0) {
        value = BTH_CMU_SYS2BTH_DATA_INTR;
    } else {
        value = BTH_CMU_SYS2BTH_DATA1_INTR;
    }

    return !!(bthcmu->BTH2SYS_IRQ_SET & value);
#else
    if (id == HAL_SYS2BTH_ID_0) {
        value = CMU_BTH2SYS_DATA_INTR;
    } else {
        value = CMU_BTH2SYS_DATA1_INTR;
    }

    return !!(cmu->SYS2BTH_IRQ_SET & value);
#endif
}

static void hal_sys2bth_irq_init(uint32_t id32)
{
    enum HAL_SYS2BTH_ID_T id = (enum HAL_SYS2BTH_ID_T)id32;

#ifdef CHIP_SUBSYS_BTH
    if (id == HAL_SYS2BTH_ID_0) {
        bthcmu->BTH2SYS_IRQ_CLR = BTH_CMU_SYS2BTH_DATA_DONE_CLR | BTH_CMU_BTH2SYS_DATA_IND_CLR;
    } else {
        bthcmu->BTH2SYS_IRQ_CLR = BTH_CMU_SYS2BTH_DATA1_DONE_CLR | BTH_CMU_BTH2SYS_DATA1_IND_CLR;
    }
#endif

#if !defined(CHIP_SUBSYS_BTH) || (defined(BTH_AS_MAIN_MCU) && defined(BTH_USE_SYS_PERIPH))
    if (id == HAL_SYS2BTH_ID_0) {
        cmu->SYS2BTH_IRQ_CLR = CMU_BTH2SYS_DATA_DONE_CLR | CMU_SYS2BTH_DATA_IND_CLR;
    } else {
        cmu->SYS2BTH_IRQ_CLR = CMU_BTH2SYS_DATA1_DONE_CLR | CMU_SYS2BTH_DATA1_IND_CLR;
    }
    cmu->SYS2BTH_IRQ_CLR;
    __DSB();
#endif

    hal_sys2bth_local_irq_mask_set(id, HAL_RMT_IPC_IRQ_SEND_IND);
    hal_sys2bth_local_irq_mask_set(id, HAL_RMT_IPC_IRQ_RECV_DONE);
}

static void hal_sys2bth_peer_tx_irq_set(uint32_t id32)
{
    enum HAL_SYS2BTH_ID_T id = (enum HAL_SYS2BTH_ID_T)id32;

    hal_sys2bth_peer_irq_set(id, HAL_RMT_IPC_IRQ_SEND_IND);
}

static void hal_sys2bth_local_tx_irq_clear(uint32_t id32)
{
    enum HAL_SYS2BTH_ID_T id = (enum HAL_SYS2BTH_ID_T)id32;

    hal_sys2bth_local_irq_clear(id, HAL_RMT_IPC_IRQ_RECV_DONE);
}

static void hal_sys2bth_rx_irq_suspend(uint32_t id32)
{
    enum HAL_SYS2BTH_ID_T id = (enum HAL_SYS2BTH_ID_T)id32;

    hal_sys2bth_local_irq_mask_clear(id, HAL_RMT_IPC_IRQ_SEND_IND);
}

static void hal_sys2bth_rx_irq_resume(uint32_t id32)
{
    enum HAL_SYS2BTH_ID_T id = (enum HAL_SYS2BTH_ID_T)id32;

    hal_sys2bth_local_irq_mask_set(id, HAL_RMT_IPC_IRQ_SEND_IND);
}

static void hal_sys2bth_rx_done_id32(uint32_t id32)
{
    enum HAL_SYS2BTH_ID_T id = (enum HAL_SYS2BTH_ID_T)id32;

    hal_sys2bth_local_irq_clear(id, HAL_RMT_IPC_IRQ_SEND_IND);
    hal_sys2bth_local_irq_mask_set(id, HAL_RMT_IPC_IRQ_SEND_IND);
}

static int hal_sys2bth_irq_active(uint32_t id32, uint32_t type)
{
    enum HAL_SYS2BTH_ID_T id = (enum HAL_SYS2BTH_ID_T)id32;
    uint32_t value;

#ifdef CHIP_SUBSYS_BTH
    if (id == HAL_SYS2BTH_ID_0) {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = BTH_CMU_SYS2BTH_DATA_INTR_MSK;
        } else {
            value = BTH_CMU_BTH2SYS_DATA_INTR_MSK;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = BTH_CMU_SYS2BTH_DATA1_INTR_MSK;
        } else {
            value = BTH_CMU_BTH2SYS_DATA1_INTR_MSK;
        }
    }

    return !!(bthcmu->BTH2SYS_IRQ_SET & value);
#else
    if (id == HAL_SYS2BTH_ID_0) {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_BTH2SYS_DATA_INTR_MSK;
        } else {
            value = CMU_SYS2BTH_DATA_INTR_MSK;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_BTH2SYS_DATA1_INTR_MSK;
        } else {
            value = CMU_SYS2BTH_DATA1_INTR_MSK;
        }
    }

    return !!(cmu->SYS2BTH_IRQ_SET & value);
#endif
}

static void hal_sys2bth_rx_irq_handler(void)
{
    hal_rmt_ipc_rx_irq_handler(&sys2bth_cfg);
}

static void hal_sys2bth_tx_irq_handler(void)
{
    hal_rmt_ipc_tx_irq_handler(&sys2bth_cfg);
}

#ifdef CORE_SLEEP_POWER_DOWN
static int hal_sys2bth_pm_notif_handler(enum HAL_PM_STATE_T state)
{
    return hal_rmt_ipc_pm_notif_handler(&sys2bth_cfg, state);
}
#endif

#ifndef __IPC_HIFI4_2_BTH_ONLY__
const
#endif
static struct HAL_RMT_IPC_CFG_T sys2bth_cfg = {
#ifdef HIFI4_USE_SYS2BTH_1
    .name = "M55-BTH",
#else
    .name = "SYS2BTH",
#endif
#ifdef CHIP_SUBSYS_BTH
    .peer_cfg_pp = (const struct HAL_RMT_IPC_CFG_T **)SYS_CFG_PTR_LOC,
    .local_cfg_pp = (const struct HAL_RMT_IPC_CFG_T **)BTH_CFG_PTR_LOC,
#else
    .peer_cfg_pp = (const struct HAL_RMT_IPC_CFG_T **)BTH_CFG_PTR_LOC,
    .local_cfg_pp = (const struct HAL_RMT_IPC_CFG_T **)SYS_CFG_PTR_LOC,
#endif

    .irq_init = hal_sys2bth_irq_init,
    .peer_tx_irq_set = hal_sys2bth_peer_tx_irq_set,
    .local_tx_irq_clear = hal_sys2bth_local_tx_irq_clear,
    .rx_irq_suspend = hal_sys2bth_rx_irq_suspend,
    .rx_irq_resume = hal_sys2bth_rx_irq_resume,
    .rx_done = hal_sys2bth_rx_done_id32,
    .irq_active = hal_sys2bth_irq_active,
    .rx_irq_entry = hal_sys2bth_rx_irq_handler,
    .tx_irq_entry = hal_sys2bth_tx_irq_handler,
#ifdef CORE_SLEEP_POWER_DOWN
    .pm_notif_handler = hal_sys2bth_pm_notif_handler,
#endif

    .sys_wake_lock = HAL_SYS_WAKE_LOCK_USER_SYS2BTH,
    .chan_num = HAL_SYS2BTH_ID_QTY,
    .chan_cfg = &sys2bth_chan_cfg[0],
    .rx_irq_id = &rx_irq_id[0],
    .tx_irq_id = &tx_irq_id[0],
    .busy_p = &busy_now,
};

#ifdef HIFI4_USE_SYS2BTH_1
static struct HAL_RMT_IPC_CFG_T sys2bth_cfg1;

static void hal_sys2bth_rx_irq_handler1(void)
{
    hal_rmt_ipc_rx_irq_handler(&sys2bth_cfg1);
}

static void hal_sys2bth_tx_irq_handler1(void)
{
    hal_rmt_ipc_tx_irq_handler(&sys2bth_cfg1);
}

SYNC_FLAGS_LOC
static struct HAL_RMT_IPC_CFG_T sys2bth_cfg1 = {
    .name = "HIFI4-BTH",
#ifdef CHIP_SUBSYS_BTH
    .peer_cfg_pp = (const struct HAL_RMT_IPC_CFG_T **)SYS_CFG_PTR_LOC1,
    .local_cfg_pp = (const struct HAL_RMT_IPC_CFG_T **)BTH_CFG_PTR_LOC1,
#else
    .peer_cfg_pp = (const struct HAL_RMT_IPC_CFG_T **)BTH_CFG_PTR_LOC1,
    .local_cfg_pp = (const struct HAL_RMT_IPC_CFG_T **)SYS_CFG_PTR_LOC1,
#endif

    .irq_init = hal_sys2bth_irq_init,
    .peer_tx_irq_set = hal_sys2bth_peer_tx_irq_set,
    .local_tx_irq_clear = hal_sys2bth_local_tx_irq_clear,
    .rx_irq_suspend = hal_sys2bth_rx_irq_suspend,
    .rx_irq_resume = hal_sys2bth_rx_irq_resume,
    .rx_done = hal_sys2bth_rx_done_id32,
    .irq_active = hal_sys2bth_irq_active,
    .rx_irq_entry = hal_sys2bth_rx_irq_handler1,
    .tx_irq_entry = hal_sys2bth_tx_irq_handler1,

    .sys_wake_lock = HAL_SYS_WAKE_LOCK_USER_SYS2BTH,
    .chan_num = HAL_SYS2BTH_ID_QTY,
    .chan_cfg = &sys2bth_chan_cfg[0],
    .rx_irq_id = &rx_irq_id[0],
    .tx_irq_id = &tx_irq_id[0],
    .busy_p = &busy_now,
};
#endif

int hal_sys2bth_config_get(struct HAL_RMT_IPC_CFG_T *cfg)
{
#ifdef HIFI4_USE_SYS2BTH_1
    *cfg = sys2bth_cfg1;
#elif defined(__IPC_HIFI4_2_BTH_ONLY__)
    *cfg = sys2bth_cfg;
#else
    ASSERT(false, "%s: Not supported", __func__);
#endif
    return 0;
}

int hal_sys2bth_config_set(const struct HAL_RMT_IPC_CFG_T *cfg)
{
#ifdef HIFI4_USE_SYS2BTH_1
    sys2bth_cfg1 = *cfg;
#elif defined(__IPC_HIFI4_2_BTH_ONLY__)
    sys2bth_cfg = *cfg;
#else
    ASSERT(false, "%s: Not supported", __func__);
#endif
    return 0;
}

int hal_sys2bth_open(enum HAL_SYS2BTH_ID_T id, HAL_SYS2BTH_RX_IRQ_HANDLER rxhandler, HAL_SYS2BTH_TX_IRQ_HANDLER txhandler, int rx_flowctrl)
{
    const struct HAL_RMT_IPC_CFG_T *cfg = &sys2bth_cfg;
#ifdef HIFI4_USE_SYS2BTH_1
    if (id == HAL_SYS2BTH_ID_1) {
        cfg = &sys2bth_cfg1;
    }
#endif
    return hal_rmt_ipc_open(cfg, id, rxhandler, txhandler, rx_flowctrl);
}

int hal_sys2bth_close(enum HAL_SYS2BTH_ID_T id)
{
    const struct HAL_RMT_IPC_CFG_T *cfg = &sys2bth_cfg;
#ifdef HIFI4_USE_SYS2BTH_1
    if (id == HAL_SYS2BTH_ID_1) {
        cfg = &sys2bth_cfg1;
    }
#endif
    return hal_rmt_ipc_close(cfg, id);
}

int hal_sys2bth_start_recv(enum HAL_SYS2BTH_ID_T id)
{
    const struct HAL_RMT_IPC_CFG_T *cfg = &sys2bth_cfg;
#ifdef HIFI4_USE_SYS2BTH_1
    if (id == HAL_SYS2BTH_ID_1) {
        cfg = &sys2bth_cfg1;
    }
#endif
    return hal_rmt_ipc_start_recv(cfg, id);
}

int hal_sys2bth_stop_recv(enum HAL_SYS2BTH_ID_T id)
{
    const struct HAL_RMT_IPC_CFG_T *cfg = &sys2bth_cfg;
#ifdef HIFI4_USE_SYS2BTH_1
    if (id == HAL_SYS2BTH_ID_1) {
        cfg = &sys2bth_cfg1;
    }
#endif
    return hal_rmt_ipc_stop_recv(cfg, id);
}

int hal_sys2bth_send_seq(enum HAL_SYS2BTH_ID_T id, const void *data, unsigned int len, unsigned int *seq)
{
    const struct HAL_RMT_IPC_CFG_T *cfg = &sys2bth_cfg;
#ifdef HIFI4_USE_SYS2BTH_1
    if (id == HAL_SYS2BTH_ID_1) {
        cfg = &sys2bth_cfg1;
    }
#endif
    return hal_rmt_ipc_send_seq(cfg, id, data, len, seq);
}

int hal_sys2bth_send(enum HAL_SYS2BTH_ID_T id, const void *data, unsigned int len)
{
    return hal_sys2bth_send_seq(id, data, len, NULL);
}

int hal_sys2bth_rx_done(enum HAL_SYS2BTH_ID_T id)
{
    const struct HAL_RMT_IPC_CFG_T *cfg = &sys2bth_cfg;
#ifdef HIFI4_USE_SYS2BTH_1
    if (id == HAL_SYS2BTH_ID_1) {
        cfg = &sys2bth_cfg1;
    }
#endif
    return hal_rmt_ipc_rx_done(cfg, id);
}

int hal_sys2bth_tx_irq_run(enum HAL_SYS2BTH_ID_T id)
{
    const struct HAL_RMT_IPC_CFG_T *cfg = &sys2bth_cfg;
#ifdef HIFI4_USE_SYS2BTH_1
    if (id == HAL_SYS2BTH_ID_1) {
        cfg = &sys2bth_cfg1;
    }
#endif
    return hal_rmt_ipc_tx_irq_run(cfg, id);
}

int hal_sys2bth_tx_active(enum HAL_SYS2BTH_ID_T id, unsigned int seq)
{
    const struct HAL_RMT_IPC_CFG_T *cfg = &sys2bth_cfg;
#ifdef HIFI4_USE_SYS2BTH_1
    if (id == HAL_SYS2BTH_ID_1) {
        cfg = &sys2bth_cfg1;
    }
#endif
    return hal_rmt_ipc_tx_active(cfg, id, seq);
}

int hal_sys2bth_opened(enum HAL_SYS2BTH_ID_T id)
{
    const struct HAL_RMT_IPC_CFG_T *cfg = &sys2bth_cfg;
#ifdef HIFI4_USE_SYS2BTH_1
    if (id == HAL_SYS2BTH_ID_1) {
        cfg = &sys2bth_cfg1;
    }
#endif
    return hal_rmt_ipc_opened(cfg, id);
}

int hal_sys2bth_set_chan_state(enum HAL_SYS2BTH_ID_T id, int opened)
{
#if defined(CORE_SLEEP_POWER_DOWN) && !defined(CHIP_SUBSYS_BTH) && !defined(BTH_AS_MAIN_MCU)
    // This is M55, and M55 runs as main MCU.
    // HIFI4 chan state can be save here.
    if (id == HAL_SYS2BTH_ID_1) {
        return hal_rmt_ipc_set_chan_state(&sys2bth_cfg, id, opened);
    }
#endif
    return 0;
}

static struct HAL_IPC_NOTIFY_CH_CFG_T sys2bth_ipc_notify_chan_cfg[HAL_SYS2BTH_ID_QTY];
static void hal_sys2bth_ipc_notify_rx_irq_handler(void);
static void hal_sys2bth_ipc_notify_tx_irq_handler(void);

static const struct HAL_IPC_NOTIFY_CFG_T sys2bth_ipc_notify_cfg = {
    .name = "SYS2BTH",
    .core = HAL_IPC_NOTIFY_CORE_SYS_BTH,

    .irq_init = hal_sys2bth_irq_init,
    .peer_tx_irq_set = hal_sys2bth_peer_tx_irq_set,
    .local_tx_irq_clear = hal_sys2bth_local_tx_irq_clear,

    .rx_done = hal_sys2bth_rx_done_id32,
    .irq_active = hal_sys2bth_irq_active,
    .rx_irq_entry = hal_sys2bth_ipc_notify_rx_irq_handler,
    .tx_irq_entry = hal_sys2bth_ipc_notify_tx_irq_handler,
    .chan_num = HAL_SYS2BTH_ID_QTY,
    .chan_cfg = &sys2bth_ipc_notify_chan_cfg[0],

    .rx_irq_id = &rx_irq_id[0],
    .tx_irq_id = &tx_irq_id[0],

};

static void hal_sys2bth_ipc_notify_rx_irq_handler(void)
{
    hal_ipc_notify_internal_rx_irq_handler(&sys2bth_ipc_notify_cfg);
}

static void hal_sys2bth_ipc_notify_tx_irq_handler(void)
{
    hal_ipc_notify_internal_tx_irq_handler(&sys2bth_ipc_notify_cfg);
}

int hal_sys2bth_ipc_notify_open(enum HAL_SYS2BTH_ID_T id, HAL_SYS2BTH_NOTIFY_IRQ_HANDLER rxhandler, HAL_SYS2BTH_NOTIFY_IRQ_HANDLER txhandler)
{
    return hal_ipc_notify_internal_open(&sys2bth_ipc_notify_cfg, id, rxhandler, txhandler);
}

int hal_sys2bth_ipc_notify_start_recv(enum HAL_SYS2BTH_ID_T id)
{
    return hal_ipc_notify_internal_start_recv(&sys2bth_ipc_notify_cfg, id);
}

int hal_sys2bth_ipc_notify_interrupt_core(enum HAL_SYS2BTH_ID_T id)
{
    return hal_ipc_notify_internal_peer_irq_set(&sys2bth_ipc_notify_cfg, (uint32_t)id);
}

int hal_sys2bth_ipc_notify_close(enum HAL_SYS2BTH_ID_T id)
{
    return hal_ipc_notify_internal_close(&sys2bth_ipc_notify_cfg, (uint32_t)id);
}

#endif
