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

#if defined(HIFI4_ITCM_SIZE) && \
    !defined(CHIP_SUBSYS_SENS) && !defined(CHIP_SUBSYS_BTH)

#include "cmsis_nvic.h"
#include "hal_mcu2dsp.h"
#include "hal_rmt_ipc.h"
#include "hal_ipc_notify_api.h"
#include "hal_ipc_notify.h"
#include "hal_trace.h"
#include CHIP_SPECIFIC_HDR(reg_cmu)

#define HAL_SYS_WAKE_LOCK_USER_MCU2DSP      HAL_SYS_WAKE_LOCK_USER_7

static uint32_t mailbox[2];

#define DSP_MAILBOX_BASE                    ((uint32_t)&mailbox[0])
#define DSP_MAILBOX_SIZE                    (sizeof(mailbox))

#define MCU_CFG_PTR_LOC                     (DSP_MAILBOX_BASE + 0)
#define DSP_CFG_PTR_LOC                     (DSP_MAILBOX_BASE + sizeof(struct HAL_RMT_IPC_CFG_T *))

STATIC_ASSERT((2 * sizeof(struct HAL_RMT_IPC_CFG_T *)) <= DSP_MAILBOX_SIZE, "DSP_MAILBOX_SIZE too small");

static struct CMU_T * const cmu = (struct CMU_T *)CMU_BASE;

static const IRQn_Type rx_irq_id[HAL_MCU2DSP_ID_QTY] = {
    DSP2MCU_DATA_IRQn,
    DSP2MCU_DATA1_IRQn,
};

static const IRQn_Type tx_irq_id[HAL_MCU2DSP_ID_QTY] = {
    MCU2DSP_DONE_IRQn,
    MCU2DSP_DONE1_IRQn,
};

static struct HAL_RMT_IPC_CFG_T mcu2dsp_cfg;

#ifndef CHIP_SUBSYS_DSP
static HAL_MCU2DSP_SEND_CALLBACK send_cb;
#endif

static struct HAL_RMT_IPC_CH_CFG_T mcu2dsp_chan_cfg[HAL_MCU2DSP_ID_QTY];

static bool busy_now;

static int hal_mcu2dsp_peer_irq_set(enum HAL_MCU2DSP_ID_T id, enum HAL_RMT_IPC_IRQ_TYPE_T type)
{
    uint32_t value;

#ifdef CHIP_SUBSYS_DSP
    if (id == HAL_MCU2DSP_ID_0) {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_DSP2M55_DATA_IND_SET;
        } else {
            value = CMU_M552DSP_DATA_DONE_SET;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_DSP2M55_DATA1_IND_SET;
        } else {
            value = CMU_M552DSP_DATA1_DONE_SET;
        }
    }

    cmu->DSP2M55_IRQ_SET = value;
#else
    if (id == HAL_MCU2DSP_ID_0) {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_M552DSP_DATA_IND_SET;
        } else {
            value = CMU_DSP2M55_DATA_DONE_SET;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_M552DSP_DATA1_IND_SET;
        } else {
            value = CMU_DSP2M55_DATA1_DONE_SET;
        }
    }

    cmu->M552DSP_IRQ_SET = value;
#endif

    return 0;
}

static int hal_mcu2dsp_local_irq_clear(enum HAL_MCU2DSP_ID_T id, enum HAL_RMT_IPC_IRQ_TYPE_T type)
{
    uint32_t value;

#ifdef CHIP_SUBSYS_DSP
    if (id == HAL_MCU2DSP_ID_0) {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_M552DSP_DATA_IND_CLR;
        } else {
            value = CMU_DSP2M55_DATA_DONE_CLR;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_M552DSP_DATA1_IND_CLR;
        } else {
            value = CMU_DSP2M55_DATA1_DONE_CLR;
        }
    }

    cmu->M552DSP_IRQ_CLR = value;
#else
    if (id == HAL_MCU2DSP_ID_0) {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_DSP2M55_DATA_IND_CLR;
        } else {
            value = CMU_M552DSP_DATA_DONE_CLR;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_DSP2M55_DATA1_IND_CLR;
        } else {
            value = CMU_M552DSP_DATA1_DONE_CLR;
        }
    }

    cmu->DSP2M55_IRQ_CLR = value;
#endif

    cmu->DSP2M55_IRQ_CLR;
    __DSB();

    return 0;
}

int hal_mcu2dsp_rx_irq_pending(enum HAL_MCU2DSP_ID_T id)
{
    uint32_t value;

#ifdef CHIP_SUBSYS_DSP
    if (id == HAL_MCU2DSP_ID_0) {
        value = CMU_M552DSP_DATA_IND_SET;
    } else {
        value = CMU_M552DSP_DATA1_IND_SET;
    }

    return !!(cmu->M552DSP_IRQ_SET & value);
#else
    if (id == HAL_MCU2DSP_ID_0) {
        value = CMU_DSP2M55_DATA_IND_SET;
    } else {
        value = CMU_DSP2M55_DATA1_IND_SET;
    }

    return !!(cmu->DSP2M55_IRQ_SET & value);
#endif
}

static void hal_mcu2dsp_irq_init(uint32_t id)
{
    if (id == HAL_MCU2DSP_ID_0) {
        cmu->M552DSP_IRQ_CLR = CMU_DSP2M55_DATA_DONE_CLR | CMU_M552DSP_DATA_IND_CLR;
        cmu->DSP2M55_IRQ_CLR = CMU_M552DSP_DATA_DONE_CLR | CMU_DSP2M55_DATA_IND_CLR;
    } else {
        cmu->M552DSP_IRQ_CLR = CMU_DSP2M55_DATA1_DONE_CLR | CMU_M552DSP_DATA1_IND_CLR;
        cmu->DSP2M55_IRQ_CLR = CMU_M552DSP_DATA1_DONE_CLR | CMU_DSP2M55_DATA1_IND_CLR;
    }

    cmu->DSP2M55_IRQ_CLR;
    __DSB();
}

static void hal_mcu2dsp_peer_tx_irq_set(uint32_t id32)
{
    enum HAL_MCU2DSP_ID_T id = (enum HAL_MCU2DSP_ID_T)id32;

    hal_mcu2dsp_peer_irq_set(id, HAL_RMT_IPC_IRQ_SEND_IND);
}

static void hal_mcu2dsp_local_tx_irq_clear(uint32_t id32)
{
    enum HAL_MCU2DSP_ID_T id = (enum HAL_MCU2DSP_ID_T)id32;

    hal_mcu2dsp_local_irq_clear(id, HAL_RMT_IPC_IRQ_RECV_DONE);
}

static void hal_mcu2dsp_rx_irq_suspend(uint32_t id32)
{
    enum HAL_MCU2DSP_ID_T id = (enum HAL_MCU2DSP_ID_T)id32;

    hal_mcu2dsp_local_irq_clear(id, HAL_RMT_IPC_IRQ_SEND_IND);
}

static void hal_mcu2dsp_rx_irq_resume(uint32_t id32)
{
    enum HAL_MCU2DSP_ID_T id = (enum HAL_MCU2DSP_ID_T)id32;

    hal_mcu2dsp_peer_irq_set(id, HAL_RMT_IPC_IRQ_SEND_IND);
}

static void hal_mcu2dsp_rx_done_id32(uint32_t id32)
{
    enum HAL_MCU2DSP_ID_T id = (enum HAL_MCU2DSP_ID_T)id32;

    hal_mcu2dsp_peer_irq_set(id, HAL_RMT_IPC_IRQ_RECV_DONE);
}

static int hal_mcu2dsp_irq_active(uint32_t id, uint32_t type)
{
    uint32_t value;

#ifdef CHIP_SUBSYS_DSP
    if (id == HAL_MCU2DSP_ID_0) {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_M552DSP_DATA_IND_SET;
        } else {
            value = CMU_DSP2M55_DATA_DONE_SET;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_M552DSP_DATA1_IND_SET;
        } else {
            value = CMU_DSP2M55_DATA1_DONE_SET;
        }
    }

    return !!(cmu->M552DSP_IRQ_SET & value);
#else
    if (id == HAL_MCU2DSP_ID_0) {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_DSP2M55_DATA_IND_SET;
        } else {
            value = CMU_M552DSP_DATA_DONE_SET;
        }
    } else {
        if (type == HAL_RMT_IPC_IRQ_SEND_IND) {
            value = CMU_DSP2M55_DATA1_IND_SET;
        } else {
            value = CMU_M552DSP_DATA1_DONE_SET;
        }
    }

    return !!(cmu->DSP2M55_IRQ_SET & value);
#endif
}

static void hal_mcu2dsp_rx_irq_handler(void)
{
    hal_rmt_ipc_rx_irq_handler(&mcu2dsp_cfg);
}

static void hal_mcu2dsp_tx_irq_handler(void)
{
    hal_rmt_ipc_tx_irq_handler(&mcu2dsp_cfg);
}

static struct HAL_RMT_IPC_CFG_T mcu2dsp_cfg = {
    .name = "MCU2DSP",
#ifdef CHIP_SUBSYS_DSP
    .peer_cfg_pp = (const struct HAL_RMT_IPC_CFG_T **)MCU_CFG_PTR_LOC,
    .local_cfg_pp = (const struct HAL_RMT_IPC_CFG_T **)DSP_CFG_PTR_LOC,
#else
    .peer_cfg_pp = (const struct HAL_RMT_IPC_CFG_T **)DSP_CFG_PTR_LOC,
    .local_cfg_pp = (const struct HAL_RMT_IPC_CFG_T **)MCU_CFG_PTR_LOC,
#endif

    .irq_init = hal_mcu2dsp_irq_init,
    .peer_tx_irq_set = hal_mcu2dsp_peer_tx_irq_set,
    .local_tx_irq_clear = hal_mcu2dsp_local_tx_irq_clear,
    .rx_irq_suspend = hal_mcu2dsp_rx_irq_suspend,
    .rx_irq_resume = hal_mcu2dsp_rx_irq_resume,
    .rx_done = hal_mcu2dsp_rx_done_id32,
    .irq_active = hal_mcu2dsp_irq_active,
    .rx_irq_entry = hal_mcu2dsp_rx_irq_handler,
    .tx_irq_entry = hal_mcu2dsp_tx_irq_handler,
#ifdef CORE_SLEEP_POWER_DOWN
    .pm_notif_handler = NULL,
#endif

    .sys_wake_lock = HAL_SYS_WAKE_LOCK_USER_MCU2DSP,
    .chan_num = HAL_MCU2DSP_ID_QTY,
    .chan_cfg = &mcu2dsp_chan_cfg[0],
    .rx_irq_id = &rx_irq_id[0],
    .tx_irq_id = &tx_irq_id[0],
    .busy_p = &busy_now,
};

int hal_mcu2dsp_config_get(struct HAL_RMT_IPC_CFG_T *cfg)
{
    *cfg = mcu2dsp_cfg;
    return 0;
}

int hal_mcu2dsp_config_set(const struct HAL_RMT_IPC_CFG_T *cfg)
{
    mcu2dsp_cfg = *cfg;
    return 0;
}

#ifndef CHIP_SUBSYS_DSP
int hal_mcu2dsp_register_send_callback(HAL_MCU2DSP_SEND_CALLBACK cb)
{
    send_cb = cb;
    return 0;
}
#endif

int hal_mcu2dsp_open(enum HAL_MCU2DSP_ID_T id, HAL_MCU2DSP_RX_IRQ_HANDLER rxhandler, HAL_MCU2DSP_TX_IRQ_HANDLER txhandler, int rx_flowctrl)
{
    return hal_rmt_ipc_open(&mcu2dsp_cfg, id, rxhandler, txhandler, rx_flowctrl);
}

int hal_mcu2dsp_close(enum HAL_MCU2DSP_ID_T id)
{
    return hal_rmt_ipc_close(&mcu2dsp_cfg, id);
}

int hal_mcu2dsp_start_recv(enum HAL_MCU2DSP_ID_T id)
{
    return hal_rmt_ipc_start_recv(&mcu2dsp_cfg, id);
}

int hal_mcu2dsp_stop_recv(enum HAL_MCU2DSP_ID_T id)
{
    return hal_rmt_ipc_stop_recv(&mcu2dsp_cfg, id);
}

int hal_mcu2dsp_send_seq(enum HAL_MCU2DSP_ID_T id, const void *data, unsigned int len, unsigned int *seq)
{
    return hal_rmt_ipc_send_seq(&mcu2dsp_cfg, id, data, len, seq);
}

int hal_mcu2dsp_send(enum HAL_MCU2DSP_ID_T id, const void *data, unsigned int len)
{
    int ret;

    ret = hal_mcu2dsp_send_seq(id, data, len, NULL);
#ifndef CHIP_SUBSYS_DSP
    if (send_cb) {
        send_cb(id);
    }
#endif
    return ret;
}

int hal_mcu2dsp_rx_done(enum HAL_MCU2DSP_ID_T id)
{
    return hal_rmt_ipc_rx_done(&mcu2dsp_cfg, id);
}

int hal_mcu2dsp_tx_irq_run(enum HAL_MCU2DSP_ID_T id)
{
    return hal_rmt_ipc_tx_irq_run(&mcu2dsp_cfg, id);
}

int hal_mcu2dsp_tx_active(enum HAL_MCU2DSP_ID_T id, unsigned int seq)
{
    return hal_rmt_ipc_tx_active(&mcu2dsp_cfg, id, seq);
}

int hal_mcu2dsp_opened(enum HAL_MCU2DSP_ID_T id)
{
    return hal_rmt_ipc_opened(&mcu2dsp_cfg, id);
}

static struct HAL_IPC_NOTIFY_CH_CFG_T mcu2dsp_ipc_notify_chan_cfg[HAL_MCU2DSP_ID_QTY];
static void hal_mcu2dsp_ipc_notify_rx_irq_handler(void);
static void hal_mcu2dsp_ipc_notify_tx_irq_handler(void);

static const struct HAL_IPC_NOTIFY_CFG_T mcu2dsp_ipc_notify_cfg = {
    .name = "MCU2DSP",
    .core = HAL_IPC_NOTIFY_CORE_SYS_DSP,

    .irq_init = hal_mcu2dsp_irq_init,
    .peer_tx_irq_set = hal_mcu2dsp_peer_tx_irq_set,
    .local_tx_irq_clear = hal_mcu2dsp_local_tx_irq_clear,

    .rx_done = hal_mcu2dsp_rx_done_id32,
    .irq_active = hal_mcu2dsp_irq_active,
    .rx_irq_entry = hal_mcu2dsp_ipc_notify_rx_irq_handler,
    .tx_irq_entry = hal_mcu2dsp_ipc_notify_tx_irq_handler,
    .chan_num = HAL_MCU2DSP_ID_QTY,
    .chan_cfg = &mcu2dsp_ipc_notify_chan_cfg[0],

    .rx_irq_id = &rx_irq_id[0],
    .tx_irq_id = &tx_irq_id[0],

};

static void hal_mcu2dsp_ipc_notify_rx_irq_handler(void)
{
    hal_ipc_notify_internal_rx_irq_handler(&mcu2dsp_ipc_notify_cfg);
}

static void hal_mcu2dsp_ipc_notify_tx_irq_handler(void)
{
    hal_ipc_notify_internal_tx_irq_handler(&mcu2dsp_ipc_notify_cfg);
}

int hal_mcu2dsp_ipc_notify_open(enum HAL_MCU2DSP_ID_T id, HAL_MCU2DSP_NOTIFY_IRQ_HANDLER rxhandler, HAL_MCU2DSP_NOTIFY_IRQ_HANDLER txhandler)
{
    return hal_ipc_notify_internal_open(&mcu2dsp_ipc_notify_cfg, id, rxhandler, txhandler);
}

int hal_mcu2dsp_ipc_notify_start_recv(enum HAL_MCU2DSP_ID_T id)
{
    return hal_ipc_notify_internal_start_recv(&mcu2dsp_ipc_notify_cfg, id);
}

int hal_mcu2dsp_ipc_notify_interrupt_core(enum HAL_MCU2DSP_ID_T id)
{
    return hal_ipc_notify_internal_peer_irq_set(&mcu2dsp_ipc_notify_cfg, (uint32_t)id);
}

int hal_mcu2dsp_ipc_notify_close(enum HAL_MCU2DSP_ID_T id)
{
    return hal_ipc_notify_internal_close(&mcu2dsp_ipc_notify_cfg, (uint32_t)id);
}

#endif

