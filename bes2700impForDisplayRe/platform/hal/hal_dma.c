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
#include "plat_addr_map.h"
#include "cmsis_nvic.h"
#include "hal_cache.h"
#include "hal_chipid.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_location.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "reg_dma.h"

// To be compatible with old chip definitions
#ifdef AUDMA_BASE
#define DMA0_BASE                           AUDMA_BASE
#define DMA0_IRQn                           AUDMA_IRQn
#define dma0_fifo_addr                      audma_fifo_addr
#define dma0_fifo_periph                    audma_fifo_periph
#define dma0_fifo_periph_remap              audma_fifo_periph_remap
#define DMA0_CHAN_START                     AUDMA_CHAN_START
#define DMA0_CHAN_NUM                       AUDMA_CHAN_NUM
#endif

#ifdef GPDMA_BASE
#define DMA1_BASE                           GPDMA_BASE
#define DMA1_IRQn                           GPDMA_IRQn
#define dma1_fifo_addr                      gpdma_fifo_addr
#define dma1_fifo_periph                    gpdma_fifo_periph
#define dma1_fifo_periph_remap              gpdma_fifo_periph_remap
#define DMA1_CHAN_START                     GPDMA_CHAN_START
#define DMA1_CHAN_NUM                       GPDMA_CHAN_NUM
#endif

#ifdef DMA0_BASE

#if (defined(CHIP_BEST1000) || defined(CHIP_BEST2000) || defined(CHIP_BEST2300)) && !(defined(ROM_BUILD) || defined(PROGRAMMER))
#define DMA_REMAP
#endif

#if (defined(CHIP_BEST1501SIMU) && !defined(__FPGA_1501P__)) || defined(CHIP_BEST1600SIMU)
enum HAL_DMA_INST_T {
    HAL_DMA_INST_0 = 1,
    HAL_DMA_INST_1 = HAL_DMA_INST_0,
    HAL_DMA_INST_QTY
};
#else
enum HAL_DMA_INST_T {
    HAL_DMA_INST_0 = 0,
#ifdef DMA1_BASE
    HAL_DMA_INST_1,
#endif
#ifdef DMA2_BASE
    HAL_DMA_INST_2,
#endif

    HAL_DMA_INST_QTY
};
#endif

struct HAL_DMA_FIFO_ADDR_T {
    uint32_t count;
    const uint32_t *addr;
};

struct HAL_DMA_FIFO_PERIPH_T {
    uint32_t count;
    const enum HAL_DMA_PERIPH_T *periph;
#ifdef DMA_REMAP
    const enum HAL_DMA_PERIPH_T *periph_remap;
#endif
};

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#include CHIP_SPECIFIC_HDR(hal_dmacfg)

static struct DMA_T * const dma[HAL_DMA_INST_QTY] = {
    (struct DMA_T *)DMA0_BASE,
#ifdef DMA1_BASE
    (struct DMA_T *)DMA1_BASE,
#endif
#ifdef DMA2_BASE
    (struct DMA_T *)DMA2_BASE,
#endif
};

static const int16_t irq_type[HAL_DMA_INST_QTY] = {
    DMA0_IRQn,
#ifdef DMA1_BASE
    DMA1_IRQn,
#endif
#ifdef DMA2_BASE
    DMA2_IRQn,
#endif
};

static void hal_dma0_irq_handler(void);
#ifdef DMA1_BASE
static void hal_dma1_irq_handler(void);
#endif
#ifdef DMA2_BASE
static void hal_dma2_irq_handler(void);
#endif

static const uint32_t irq_entry[HAL_DMA_INST_QTY] = {
    (uint32_t)hal_dma0_irq_handler,
#ifdef DMA1_BASE
    (uint32_t)hal_dma1_irq_handler,
#endif
#ifdef DMA2_BASE
    (uint32_t)hal_dma2_irq_handler,
#endif
};

static const struct HAL_DMA_FIFO_ADDR_T fifo_addr[HAL_DMA_INST_QTY] = {
    {
        .count = ARRAY_SIZE(dma0_fifo_addr),
        .addr = dma0_fifo_addr,
    },
#ifdef DMA1_BASE
    {
        .count = ARRAY_SIZE(dma1_fifo_addr),
        .addr = dma1_fifo_addr,
    },
#endif
#ifdef DMA2_BASE
    {
        .count = ARRAY_SIZE(dma2_fifo_addr),
        .addr = dma2_fifo_addr,
    },
#endif
};

static const struct HAL_DMA_FIFO_PERIPH_T fifo_periph[HAL_DMA_INST_QTY] = {
    {
        .count = ARRAY_SIZE(dma0_fifo_periph),
        .periph = dma0_fifo_periph,
#ifdef DMA_REMAP
        .periph_remap = dma0_fifo_periph_remap,
#endif
    },
#ifdef DMA1_BASE
    {
        .count = ARRAY_SIZE(dma1_fifo_periph),
        .periph = dma1_fifo_periph,
#ifdef DMA_REMAP
        .periph_remap = dma1_fifo_periph_remap,
#endif
    },
#endif
#ifdef DMA2_BASE
    {
        .count = ARRAY_SIZE(dma2_fifo_periph),
        .periph = dma2_fifo_periph,
#ifdef DMA_REMAP
        .periph_remap = dma2_fifo_periph_remap,
#endif
    },
#endif
};

static const uint8_t chan_start[HAL_DMA_INST_QTY] = {
    DMA0_CHAN_START,
#ifdef DMA1_BASE
    DMA1_CHAN_START,
#endif
#ifdef DMA2_BASE
    DMA2_CHAN_START,
#endif
};

static const uint8_t chan_num[HAL_DMA_INST_QTY] = {
    DMA0_CHAN_NUM,
#ifdef DMA1_BASE
    DMA1_CHAN_NUM,
#endif
#ifdef DMA2_BASE
    DMA2_CHAN_NUM,
#endif
};

/* Channel array to monitor free channel */
static bool chan_enabled[HAL_DMA_INST_QTY][DMA_NUMBER_CHANNELS];

static HAL_DMA_IRQ_HANDLER_T handler[HAL_DMA_INST_QTY][DMA_NUMBER_CHANNELS];

#ifdef DMA_REMAP
static uint32_t periph_remap_bitmap[HAL_DMA_INST_QTY];
#endif

#ifdef CORE_SLEEP_POWER_DOWN
static uint32_t saved_dma_regs[HAL_DMA_INST_QTY];
#endif

static const char * const err_invalid_inst = "Invalid DMA inst: %u";
static const char * const err_invalid_chan = "Invalid DMA%u chan: %u";

static bool dma_opened = false;

static HAL_DMA_DELAY_FUNC dma_delay = NULL;

#ifdef DMA_DEBUG
#ifdef DMA2_BASE && (DMA2_CHAN_NUM > DMA0_CHAN_NUM)
#define MAX_CHAN_NUM02                      DMA2_CHAN_NUM
#else
#define MAX_CHAN_NUM02                      DMA0_CHAN_NUM
#endif
#ifdef DMA1_BASE && (DMA1_CHAN_NUM > MAX_CHAN_NUM02)
#define MAX_CHAN_NUM_PER_INST               DMA1_CHAN_NUM
#else
#define MAX_CHAN_NUM_PER_INST               MAX_CHAN_NUM02
#endif
static void *chan_user[HAL_DMA_INST_QTY][MAX_CHAN_NUM_PER_INST];
static uint32_t debug_user_map[HAL_DMA_INST_QTY];
STATIC_ASSERT(sizeof(debug_user_map[0]) * 8 >= MAX_CHAN_NUM_PER_INST, "debug_user_map size too small");
#endif

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/
static void hal_dma_delay(uint32_t ms)
{
    if (dma_delay && !in_isr()) {
        dma_delay(ms);
    } else {
        hal_sys_timer_delay(MS_TO_TICKS(ms));
    }
}

static inline uint8_t generate_chan(enum HAL_DMA_INST_T inst, uint8_t hwch)
{
    return ((inst << 4) | (hwch & 0xF));
}

static inline enum HAL_DMA_INST_T get_inst_from_chan(uint8_t ch)
{
    return (enum HAL_DMA_INST_T)(ch >> 4);
}

static inline uint8_t get_hwch_from_chan(uint8_t ch)
{
    return (ch & 0xF);
}

static inline int get_index_from_periph(enum HAL_DMA_PERIPH_T periph, enum HAL_DMA_INST_T *pinst, uint8_t *pidx)
{
    enum HAL_DMA_INST_T inst;
    int i;

    for (inst = 0; inst < HAL_DMA_INST_QTY; inst++) {
        for (i = 0; i < fifo_periph[inst].count; i++) {
            if (fifo_periph[inst].periph[i] == periph) {
                *pinst = inst;
                *pidx = i;
                return 0;
            }
        }
    }

    return 1;
}

#ifdef DMA_REMAP
static inline int get_remap_index_from_periph(enum HAL_DMA_INST_T inst, enum HAL_DMA_PERIPH_T periph, uint8_t *pidx)
{
    int i;

    if (fifo_periph[inst].periph_remap == NULL) {
        return 1;
    }

    for (i = 0; i < fifo_periph[inst].count; i++) {
        if (fifo_periph[inst].periph_remap[i] == periph) {
            *pidx = i;
            return 0;
        }
    }

    return 2;
}
#endif

static inline uint32_t hal_dma_get_periph_addr(enum HAL_DMA_PERIPH_T periph)
{
    int ret;
    enum HAL_DMA_INST_T inst;
    uint8_t index;

    ret = get_index_from_periph(periph, &inst, &index);
    if (ret) {
        return 0;
    }

    return fifo_addr[inst].addr[index];
}

/* Initialize the DMA */
static void hal_dma_open_inst(enum HAL_DMA_INST_T inst)
{
    uint8_t i;

    /* Reset all channel configuration register */
    for (i = 0; i < DMA_NUMBER_CHANNELS; i++) {
        dma[inst]->CH[i].CONFIG = 0;
    }

    /* Clear all DMA interrupt and error flag */
    dma[inst]->INTTCCLR = ~0UL;
    dma[inst]->INTERRCLR = ~0UL;

    dma[inst]->DMACONFIG = (dma[inst]->DMACONFIG & ~(DMA_DMACONFIG_AHB1_BIGENDIAN |
        DMA_DMACONFIG_AHB2_BIGENDIAN | DMA_DMACONFIG_CLK_EN_MASK | DMA_DMACONFIG_CLK_MANUAL)) |
        DMA_DMACONFIG_EN | DMA_DMACONFIG_CROSS_1KB_EN |
#ifdef CHIP_BEST3001
        DMA_DMACONFIG_TC_IRQ_EN_MASK |
#endif
        0;

#ifdef CHIP_BEST1400
    dma[inst]->DMACONFIG &= ~DMA_DMACONFIG_TC_IRQ_EN_MASK;
#endif

    /* Reset all channels are free */
    for (i = 0; i < DMA_NUMBER_CHANNELS; i++) {
        chan_enabled[inst][i] = false;
    }

    NVIC_SetVector(irq_type[inst], irq_entry[inst]);
    if (inst == HAL_DMA_INST_0) {
        NVIC_SetPriority(irq_type[inst], IRQ_PRIORITY_ABOVENORMAL);
    } else {
        NVIC_SetPriority(irq_type[inst], IRQ_PRIORITY_NORMAL);
    }
    NVIC_ClearPendingIRQ(irq_type[inst]);
    NVIC_EnableIRQ(irq_type[inst]);
}

/* Shutdown the DMA */
static void hal_dma_close_inst(enum HAL_DMA_INST_T inst)
{
    NVIC_DisableIRQ(irq_type[inst]);
    dma[inst]->DMACONFIG = 0;
}

static bool hal_dma_chan_busy_inst(enum HAL_DMA_INST_T inst, uint8_t hwch)
{
    return !!(dma[inst]->ENBLDCHNS & DMA_STAT_CHAN(hwch));
}

static void hal_dma_handle_chan_irq(enum HAL_DMA_INST_T inst, uint8_t hwch)
{
    uint32_t remains;
    struct HAL_DMA_DESC_T *lli;
    bool tcint, errint;

    /* Check counter terminal status */
    tcint = !!(dma[inst]->INTTCSTAT & DMA_STAT_CHAN(hwch));
    /* Check error terminal status */
    errint = !!(dma[inst]->INTERRSTAT & DMA_STAT_CHAN(hwch));

    if (tcint || errint) {
        if (tcint) {
            /* Clear terminate counter Interrupt pending */
            dma[inst]->INTTCCLR = DMA_STAT_CHAN(hwch);
        }
        if (errint) {
            /* Clear error counter Interrupt pending */
            dma[inst]->INTERRCLR = DMA_STAT_CHAN(hwch);
        }

        if (handler[inst][hwch]) {
            remains = GET_BITFIELD(dma[inst]->CH[hwch].CONTROL, DMA_CONTROL_TRANSFERSIZE);
            lli = (struct HAL_DMA_DESC_T *)dma[inst]->CH[hwch].LLI;
            handler[inst][hwch](generate_chan(inst, hwch), remains, errint, lli);
        }
    }
}

static void hal_dma_irq_handler(enum HAL_DMA_INST_T inst)
{
    uint8_t hwch;

    for (hwch = 0; hwch < DMA_NUMBER_CHANNELS; hwch++) {
        if ((dma[inst]->INTSTAT & DMA_STAT_CHAN(hwch)) == 0) {
            continue;
        }
        hal_dma_handle_chan_irq(inst, hwch);
    }
}

static void hal_dma0_irq_handler(void)
{
    hal_dma_irq_handler(HAL_DMA_INST_0);
}

#ifdef DMA1_BASE
static void hal_dma1_irq_handler(void)
{
    hal_dma_irq_handler(HAL_DMA_INST_1);
}
#endif

#ifdef DMA2_BASE
static void hal_dma2_irq_handler(void)
{
    hal_dma_irq_handler(HAL_DMA_INST_2);
}
#endif

static enum HAL_DMA_RET_T hal_dma_init_control(uint32_t *ctrl,
                                               const struct HAL_DMA_CH_CFG_T *cfg,
                                               int tc_irq)
{
    uint32_t addr_inc;
    enum HAL_DMA_FLOW_CONTROL_T type;

    if (cfg->src_tsize > HAL_DMA_MAX_DESC_XFER_SIZE) {
        return HAL_DMA_ERR;
    }

#ifdef CHIP_BEST1000
    type = cfg->type;
#else
    type = cfg->type & ~HAL_DMA_FLOW_FLAG_MASK;
#endif

    switch (type) {
        case HAL_DMA_FLOW_M2M_DMA:
            addr_inc = DMA_CONTROL_SI | DMA_CONTROL_DI;
            break;
        case HAL_DMA_FLOW_M2P_DMA:
        case HAL_DMA_FLOW_M2P_PERIPH:
            addr_inc = DMA_CONTROL_SI;
            break;
        case HAL_DMA_FLOW_P2M_DMA:
        case HAL_DMA_FLOW_P2M_PERIPH:
            addr_inc = DMA_CONTROL_DI;
            break;
        case HAL_DMA_FLOW_P2P_DMA:
        case HAL_DMA_FLOW_P2P_DSTPERIPH:
        case HAL_DMA_FLOW_P2P_SRCPERIPH:
            addr_inc = 0;
            break;
        default:
            return HAL_DMA_ERR;
    }

#ifndef CHIP_BEST1000
    if (cfg->type & HAL_DMA_FLOW_FLAG_SI) {
        addr_inc |= DMA_CONTROL_SI;
    }
    if (cfg->type & HAL_DMA_FLOW_FLAG_DI) {
        addr_inc |= DMA_CONTROL_DI;
    }
#endif

    *ctrl = DMA_CONTROL_TRANSFERSIZE(cfg->src_tsize) |
            DMA_CONTROL_SBSIZE(cfg->src_bsize) |
            DMA_CONTROL_DBSIZE(cfg->dst_bsize) |
            DMA_CONTROL_SWIDTH(cfg->src_width) |
            DMA_CONTROL_DWIDTH(cfg->dst_width) |
            (tc_irq ? DMA_CONTROL_TC_IRQ : 0) |
            addr_inc;

    return HAL_DMA_OK;
}

/*****************************************************************************
 * Generic Public functions
 ****************************************************************************/

enum HAL_DMA_RET_T hal_dma_init_desc(struct HAL_DMA_DESC_T *desc,
                                     const struct HAL_DMA_CH_CFG_T *cfg,
                                     const struct HAL_DMA_DESC_T *next,
                                     int tc_irq)
{
    uint32_t ctrl;
    enum HAL_DMA_RET_T ret;
    enum HAL_DMA_FLOW_CONTROL_T type;

    ret = hal_dma_init_control(&ctrl, cfg, tc_irq);
    if (ret != HAL_DMA_OK) {
        return ret;
    }

#ifdef CHIP_BEST1000
    type = cfg->type;
#else
    type = cfg->type & ~HAL_DMA_FLOW_FLAG_MASK;
#endif

    if (type == HAL_DMA_FLOW_M2M_DMA || type == HAL_DMA_FLOW_M2P_DMA ||
            type == HAL_DMA_FLOW_M2P_PERIPH) {
        desc->src = cfg->src;
    } else {
        desc->src = hal_dma_get_periph_addr(cfg->src_periph);
    }
    if (type == HAL_DMA_FLOW_M2M_DMA || type == HAL_DMA_FLOW_P2M_DMA ||
            type == HAL_DMA_FLOW_P2M_PERIPH) {
        desc->dst = cfg->dst;
    } else {
        desc->dst = hal_dma_get_periph_addr(cfg->dst_periph);
    }
    desc->lli = (uint32_t)next;
    desc->ctrl = ctrl;

    return HAL_DMA_OK;
}

enum HAL_DMA_RET_T hal_dma_sg_2d_start(const struct HAL_DMA_DESC_T *desc,
                                       const struct HAL_DMA_CH_CFG_T *cfg,
                                       const struct HAL_DMA_2D_CFG_T *src_2d,
                                       const struct HAL_DMA_2D_CFG_T *dst_2d)
{
    enum HAL_DMA_INST_T inst;
    uint8_t hwch;
    enum HAL_DMA_FLOW_CONTROL_T type;
    uint8_t src_periph, dst_periph;
    enum HAL_DMA_INST_T periph_inst;
    int ret;
    uint32_t irq_mask, try_burst;
    uint32_t lock;

    inst = get_inst_from_chan(cfg->ch);
    hwch = get_hwch_from_chan(cfg->ch);

    ASSERT(inst < HAL_DMA_INST_QTY, err_invalid_inst, inst);
    ASSERT(hwch < DMA_NUMBER_CHANNELS, err_invalid_chan, inst, hwch);

#ifdef CHIP_BEST1000
    type = cfg->type;
#else
    type = cfg->type & ~HAL_DMA_FLOW_FLAG_MASK;
#endif

    if (type == HAL_DMA_FLOW_M2M_DMA || type == HAL_DMA_FLOW_M2P_DMA ||
            type == HAL_DMA_FLOW_M2P_PERIPH) {
        src_periph = 0;
    } else {
        ret = get_index_from_periph(cfg->src_periph, &periph_inst, &src_periph);
        ASSERT(ret == 0, "Failed to get src periph: %d", cfg->src_periph);
#ifdef DMA_REMAP
        if (periph_remap_bitmap[periph_inst] & (1 << src_periph)) {
            periph_inst ^= 1;
            ret = get_remap_index_from_periph(periph_inst, cfg->src_periph, &src_periph);
            ASSERT(ret == 0, "Failed to get remap src periph: %d", cfg->src_periph);
        }
#endif
        ASSERT(inst == periph_inst, "Mismatch inst in chan=0x%02x and src periph %d", cfg->ch, cfg->src_periph);
    }
    if (type == HAL_DMA_FLOW_M2M_DMA || type == HAL_DMA_FLOW_P2M_DMA ||
            type == HAL_DMA_FLOW_P2M_PERIPH) {
        dst_periph = 0;
    } else {
        ret = get_index_from_periph(cfg->dst_periph, &periph_inst, &dst_periph);
        ASSERT(ret == 0, "Failed to get dst periph: %d", cfg->dst_periph);
#ifdef DMA_REMAP
        if (periph_remap_bitmap[periph_inst] & (1 << dst_periph)) {
            periph_inst ^= 1;
            ret = get_remap_index_from_periph(periph_inst, cfg->dst_periph, &dst_periph);
            ASSERT(ret == 0, "Failed to get remap dst periph: %d", cfg->dst_periph);
        }
#endif
        ASSERT(inst == periph_inst, "Mismatch inst in chan=0x%02x and dst periph %d", cfg->ch, cfg->dst_periph);
    }

    if (!chan_enabled[inst][hwch]) {
        // Not acquired
        return HAL_DMA_ERR;
    }
    if (hal_dma_chan_busy_inst(inst, hwch)) {
        // Busy
        return HAL_DMA_ERR;
    }

    if (cfg->handler == NULL) {
        irq_mask = 0;
    } else {
        irq_mask = DMA_CONFIG_ERR_IRQMASK | DMA_CONFIG_TC_IRQMASK;
        handler[inst][hwch] = cfg->handler;
    }

    try_burst = cfg->try_burst ? DMA_CONFIG_TRY_BURST : 0;

    /* Reset the Interrupt status */
    dma[inst]->INTTCCLR = DMA_STAT_CHAN(hwch);
    dma[inst]->INTERRCLR = DMA_STAT_CHAN(hwch);

    dma[inst]->CH[hwch].SRCADDR = desc->src;
    dma[inst]->CH[hwch].DSTADDR = desc->dst;
    dma[inst]->CH[hwch].LLI = desc->lli;
    dma[inst]->CH[hwch].CONTROL = desc->ctrl;
    dma[inst]->CH[hwch].CONFIG = DMA_CONFIG_SRCPERIPH(src_periph) |
                                 DMA_CONFIG_DSTPERIPH(dst_periph) |
                                 DMA_CONFIG_TRANSFERTYPE(type) |
                                 irq_mask |
                                 try_burst;

#ifndef CHIP_BEST1000
    if (src_2d) {
        dma[inst]->_2D[hwch].SRCX = DMA_2D_MODIFY(src_2d->xmodify) | DMA_2D_COUNT(src_2d->xcount);
        dma[inst]->_2D[hwch].SRCY = DMA_2D_MODIFY(src_2d->ymodify) | DMA_2D_COUNT(src_2d->ycount);
        dma[inst]->_2D[hwch].CTRL |= DMA_2D_CTRL_SRC_EN;
    } else {
        dma[inst]->_2D[hwch].CTRL &= ~DMA_2D_CTRL_SRC_EN;
    }
    if (dst_2d) {
        dma[inst]->_2D[hwch].DSTX = DMA_2D_MODIFY(dst_2d->xmodify) | DMA_2D_COUNT(dst_2d->xcount);
        dma[inst]->_2D[hwch].DSTY = DMA_2D_MODIFY(dst_2d->ymodify) | DMA_2D_COUNT(dst_2d->ycount);
        dma[inst]->_2D[hwch].CTRL |= DMA_2D_CTRL_DST_EN;
    } else {
        dma[inst]->_2D[hwch].CTRL &= ~DMA_2D_CTRL_DST_EN;
    }
#endif

    lock = int_lock();
    if (cfg->start_cb) {
        cfg->start_cb(cfg->ch);
    }
    dma[inst]->CH[hwch].CONFIG |= DMA_CONFIG_EN;
    int_unlock(lock);

    return HAL_DMA_OK;
}

enum HAL_DMA_RET_T hal_dma_sg_start(const struct HAL_DMA_DESC_T *desc,
                                    const struct HAL_DMA_CH_CFG_T *cfg)
{
    return hal_dma_sg_2d_start(desc, cfg, NULL, NULL);
}

enum HAL_DMA_RET_T hal_dma_start(const struct HAL_DMA_CH_CFG_T *cfg)
{
    struct HAL_DMA_DESC_T desc;
    enum HAL_DMA_RET_T ret;

    ret = hal_dma_init_desc(&desc, cfg, NULL, 1);
    if (ret != HAL_DMA_OK) {
        return ret;
    }

    ret = hal_dma_sg_start(&desc, cfg);
    if (ret != HAL_DMA_OK) {
        return ret;
    }

    return HAL_DMA_OK;
}

void hal_dma_cancel(uint8_t ch)
{
    enum HAL_DMA_INST_T inst;
    uint8_t hwch;

    inst = get_inst_from_chan(ch);
    hwch = get_hwch_from_chan(ch);

    ASSERT(inst < HAL_DMA_INST_QTY, err_invalid_inst, inst);
    ASSERT(hwch < DMA_NUMBER_CHANNELS, err_invalid_chan, inst, hwch);

    dma[inst]->CH[hwch].CONFIG &= ~DMA_CONFIG_EN;
    dma[inst]->INTTCCLR = DMA_STAT_CHAN(hwch);
    dma[inst]->INTERRCLR = DMA_STAT_CHAN(hwch);
}

void hal_dma_stop(uint8_t ch)
{
    enum HAL_DMA_INST_T inst;
    uint8_t hwch;
    uint8_t retry = 0;
    const uint8_t max_retry = 10;

    inst = get_inst_from_chan(ch);
    hwch = get_hwch_from_chan(ch);

    ASSERT(inst < HAL_DMA_INST_QTY, err_invalid_inst, inst);
    ASSERT(hwch < DMA_NUMBER_CHANNELS, err_invalid_chan, inst, hwch);

    dma[inst]->CH[hwch].CONFIG |= DMA_CONFIG_HALT;
#if 1
    while ((dma[inst]->CH[hwch].CONFIG & DMA_CONFIG_ACTIVE) && (++retry < max_retry)) {
        if (retry <= 3) {
            hal_sys_timer_delay_us(10);
        } else {
            hal_dma_delay(1);
        }
    }
#else
    while (dma[inst]->CH[hwch].CONFIG & DMA_CONFIG_ACTIVE);
#endif

    hal_dma_cancel(ch);
}

uint32_t hal_dma_get_base_addr(uint8_t ch)
{
    enum HAL_DMA_INST_T inst;
    uint32_t addr;

    inst = get_inst_from_chan(ch);
    ASSERT(inst < HAL_DMA_INST_QTY, err_invalid_inst, inst);

    addr = (uint32_t)(dma[inst]);
    return addr;
}

uint8_t hal_dma_get_chan(enum HAL_DMA_PERIPH_T periph, enum HAL_DMA_GET_CHAN_T policy)
{
    enum HAL_DMA_INST_T inst;
    int ret;
    uint8_t i, hwch;
    uint8_t got = HAL_DMA_CHAN_NONE;
    uint32_t lock;
    uint32_t start;
    uint32_t end;

    ASSERT(policy == HAL_DMA_HIGH_PRIO || policy == HAL_DMA_LOW_PRIO || policy == HAL_DMA_LOW_PRIO_ONLY,
        "Invalid DMA policy: %d", policy);

    if (periph == HAL_DMA_PERIPH_NULL) {
        return got;
    }

    if (HAL_DMA0_MEM <= periph && periph <= HAL_DMA2_MEM) {
        if (0) {
#ifdef DMA1_BASE
        } else if (periph == HAL_DMA1_MEM) {
            inst = HAL_DMA_INST_1;
#endif
#ifdef DMA2_BASE
        } else if (periph == HAL_DMA2_MEM) {
            inst = HAL_DMA_INST_2;
#endif
        } else {
            inst = HAL_DMA_INST_0;
        }
    } else {
        ret = get_index_from_periph(periph, &inst, &i);
        ASSERT(ret == 0, "Invalid DMA periph: %d", periph);
#ifdef DMA_REMAP
        if (periph_remap_bitmap[inst] & (1 << i)) {
            inst ^= 1;
        }
#endif
    }

    start = chan_start[inst];
    end = start + chan_num[inst];

    lock = int_lock();
    for (i = start; i < end; i++) {
        if (policy == HAL_DMA_HIGH_PRIO) {
            hwch = i;
        } else {
            hwch = end - 1 - i;
            if (policy == HAL_DMA_LOW_PRIO_ONLY && (hwch + 2) < end) {
                break;
            }
        }

        if (!chan_enabled[inst][hwch] && !hal_dma_chan_busy_inst(inst, hwch)) {
            chan_enabled[inst][hwch] = true;
            got = generate_chan(inst, hwch);
            dma[inst]->DMACONFIG |= DMA_DMACONFIG_CLK_EN(1 << hwch);
            break;
        }
    }
    int_unlock(lock);

#ifdef DMA_DEBUG
    if (got != HAL_DMA_CHAN_NONE) {
        chan_user[inst][hwch] = __builtin_return_address(0);
    }
#endif

    return got;
}

void hal_dma_free_chan(uint8_t ch)
{
    enum HAL_DMA_INST_T inst;
    uint8_t hwch;
    uint32_t lock;

    inst = get_inst_from_chan(ch);
    hwch = get_hwch_from_chan(ch);

    ASSERT(inst < HAL_DMA_INST_QTY, err_invalid_inst, inst);
    ASSERT(hwch < DMA_NUMBER_CHANNELS, err_invalid_chan, inst, hwch);
    ASSERT(chan_enabled[inst][hwch], "DMA chan not enabled: inst=%u hwch=%u", inst, hwch);

    hal_dma_cancel(ch);

    lock = int_lock();
    chan_enabled[inst][hwch] = false;
    dma[inst]->DMACONFIG &= ~DMA_DMACONFIG_CLK_EN(1 << hwch);
    int_unlock(lock);
}

uint32_t hal_dma_get_cur_src_addr(uint8_t ch)
{
    enum HAL_DMA_INST_T inst;
    uint8_t hwch;

    inst = get_inst_from_chan(ch);
    hwch = get_hwch_from_chan(ch);

    ASSERT(inst < HAL_DMA_INST_QTY, err_invalid_inst, inst);
    ASSERT(hwch < DMA_NUMBER_CHANNELS, err_invalid_chan, inst, hwch);

    return dma[inst]->CH[hwch].SRCADDR;
}

uint32_t hal_dma_get_cur_dst_addr(uint8_t ch)
{
    enum HAL_DMA_INST_T inst;
    uint8_t hwch;

    inst = get_inst_from_chan(ch);
    hwch = get_hwch_from_chan(ch);

    ASSERT(inst < HAL_DMA_INST_QTY, err_invalid_inst, inst);
    ASSERT(hwch < DMA_NUMBER_CHANNELS, err_invalid_chan, inst, hwch);

    return dma[inst]->CH[hwch].DSTADDR;
}

void SRAM_TEXT_LOC hal_dma_get_cur_src_remain_and_addr(uint8_t ch, uint32_t *remain, uint32_t *src)
{
    enum HAL_DMA_INST_T inst;
    uint8_t hwch;
    uint32_t lock;
    uint32_t size[2];
    uint32_t srcaddr[2];

    inst = get_inst_from_chan(ch);
    hwch = get_hwch_from_chan(ch);

    ASSERT(inst < HAL_DMA_INST_QTY, err_invalid_inst, inst);
    ASSERT(hwch < DMA_NUMBER_CHANNELS, err_invalid_chan, inst, hwch);

    lock = int_lock();
    size[0] = dma[inst]->CH[hwch].CONTROL;
    srcaddr[0] = dma[inst]->CH[hwch].SRCADDR;
    size[1] = dma[inst]->CH[hwch].CONTROL;
    srcaddr[1] = dma[inst]->CH[hwch].SRCADDR;
    int_unlock(lock);

    size[0] = GET_BITFIELD(size[0], DMA_CONTROL_TRANSFERSIZE);
    size[1] = GET_BITFIELD(size[1], DMA_CONTROL_TRANSFERSIZE);

    if (size[0] == size[1]) {
        *remain = size[0];
        *src = srcaddr[0];
    } else {
        *remain = size[1];
        *src = srcaddr[1];
    }

    return;
}

void hal_dma_get_irq_list(const int16_t **irq, uint32_t *cnt)
{
    if (irq && cnt) {
        *irq = irq_type;
        *cnt = ARRAY_SIZE(irq_type);
    }
}

enum HAL_DMA_RET_T hal_dma_irq_run_chan(uint8_t ch)
{
    enum HAL_DMA_INST_T inst;
    uint8_t hwch;

    inst = get_inst_from_chan(ch);
    hwch = get_hwch_from_chan(ch);

    ASSERT(inst < HAL_DMA_INST_QTY, err_invalid_inst, inst);
    ASSERT(hwch < DMA_NUMBER_CHANNELS, err_invalid_chan, inst, hwch);

    if ((dma[inst]->INTSTAT & DMA_STAT_CHAN(hwch)) == 0) {
        return HAL_DMA_ERR;
    }

    hal_dma_handle_chan_irq(inst, hwch);

    return HAL_DMA_OK;
}

bool hal_dma_chan_busy(uint8_t ch)
{
    enum HAL_DMA_INST_T inst;
    uint8_t hwch;

    inst = get_inst_from_chan(ch);
    hwch = get_hwch_from_chan(ch);

    ASSERT(inst < HAL_DMA_INST_QTY, err_invalid_inst, inst);
    ASSERT(hwch < DMA_NUMBER_CHANNELS, err_invalid_chan, inst, hwch);

    return hal_dma_chan_busy_inst(inst, hwch);
}

bool hal_dma_busy(void)
{
    enum HAL_DMA_INST_T inst;
    int hwch;
    uint32_t start;
    uint32_t end;

    for (inst = HAL_DMA_INST_0; inst < HAL_DMA_INST_QTY; inst++) {
        start = chan_start[inst];
        end = start + chan_num[inst];
        for (hwch = start; hwch < end; hwch++) {
            if (hal_dma_chan_busy_inst(inst, hwch)) {
                return true;
            }
        }
    }

    return false;
}

uint32_t hal_dma_get_sg_remain_size(uint8_t ch)
{
    enum HAL_DMA_INST_T inst;
    uint8_t hwch;
    uint32_t remains;
    const struct HAL_DMA_DESC_T *desc, *first;

    inst = get_inst_from_chan(ch);
    hwch = get_hwch_from_chan(ch);

    ASSERT(inst < HAL_DMA_INST_QTY, err_invalid_inst, inst);
    ASSERT(hwch < DMA_NUMBER_CHANNELS, err_invalid_chan, inst, hwch);

    remains = GET_BITFIELD(dma[inst]->CH[hwch].CONTROL, DMA_CONTROL_TRANSFERSIZE);
    first = (const struct HAL_DMA_DESC_T *)dma[inst]->CH[hwch].LLI;
    desc = first;
    while (desc) {
        remains += GET_BITFIELD(desc->ctrl, DMA_CONTROL_TRANSFERSIZE);
        desc = (const struct HAL_DMA_DESC_T *)desc->lli;
        if (desc == first) {
            break;
        }
    }

    return remains;
}

void hal_dma_tc_irq_enable(uint8_t ch)
{
#if !(defined(CHIP_BEST1000) || defined(CHIP_BEST2000) || defined(CHIP_BEST3001) || defined(CHIP_BEST3005))
    enum HAL_DMA_INST_T inst;
    uint8_t hwch;
    uint32_t lock;

    inst = get_inst_from_chan(ch);
    hwch = get_hwch_from_chan(ch);

    ASSERT(inst < HAL_DMA_INST_QTY, err_invalid_inst, inst);
    ASSERT(hwch < DMA_NUMBER_CHANNELS, err_invalid_chan, inst, hwch);

    lock = int_lock();
#ifdef CHIP_BEST2300
    if (inst == HAL_DMA_INST_0) {
        hal_cmu_dma_tc_irq_set_chan(hwch);
    }
#else
    dma[inst]->DMACONFIG |= DMA_DMACONFIG_TC_IRQ_EN(1 << hwch);
#endif
    int_unlock(lock);
#endif
}

void hal_dma_tc_irq_disable(uint8_t ch)
{
#if !(defined(CHIP_BEST1000) || defined(CHIP_BEST2000) || defined(CHIP_BEST3001) || defined(CHIP_BEST3005))
    enum HAL_DMA_INST_T inst;
    uint8_t hwch;
    uint32_t lock;

    inst = get_inst_from_chan(ch);
    hwch = get_hwch_from_chan(ch);

    ASSERT(inst < HAL_DMA_INST_QTY, err_invalid_inst, inst);
    ASSERT(hwch < DMA_NUMBER_CHANNELS, err_invalid_chan, inst, hwch);

    lock = int_lock();
#ifdef CHIP_BEST2300
    if (inst == HAL_DMA_INST_0) {
        hal_cmu_dma_tc_irq_clear_chan(hwch);
    }
#else
    dma[inst]->DMACONFIG &= ~DMA_DMACONFIG_TC_IRQ_EN(1 << hwch);
#endif
    int_unlock(lock);
#endif
}

void hal_dma_set_burst_addr_inc(uint8_t ch, const struct HAL_DMA_BURST_ADDR_INC_T *inc)
{
#if !(defined(CHIP_BEST1000) || defined(CHIP_BEST2000) || defined(CHIP_BEST3001) || defined(CHIP_BEST3005))
    enum HAL_DMA_INST_T inst;
    uint8_t hwch;
    uint32_t lock;

    inst = get_inst_from_chan(ch);
    hwch = get_hwch_from_chan(ch);

    ASSERT(inst < HAL_DMA_INST_QTY, err_invalid_inst, inst);
    ASSERT(hwch < DMA_NUMBER_CHANNELS, err_invalid_chan, inst, hwch);

    lock = int_lock();
    if (inc && inc->src_inc_en) {
        dma[inst]->CH[hwch].CONTROL |= DMA_CONTROL_BURST_SI;
        dma[inst]->_2D[hwch].SRC_INC = SET_BITFIELD(dma[inst]->_2D[hwch].SRC_INC, DMA_BURST_SRC_INC_VAL, inc->src_inc_val);
    } else {
        dma[inst]->CH[hwch].CONTROL &= ~DMA_CONTROL_BURST_SI;
    }
    if (inc && inc->dst_inc_en) {
        dma[inst]->CH[hwch].CONTROL |= DMA_CONTROL_BURST_DI;
        dma[inst]->_2D[hwch].DST_INC = SET_BITFIELD(dma[inst]->_2D[hwch].DST_INC, DMA_BURST_DST_INC_VAL, inc->dst_inc_val);
    } else {
        dma[inst]->CH[hwch].CONTROL &= ~DMA_CONTROL_BURST_DI;
    }
    int_unlock(lock);
#endif
}

void hal_dma_clear_burst_addr_inc(uint8_t ch)
{
    hal_dma_set_burst_addr_inc(ch, NULL);
}

void hal_dma_set_desc_burst_addr_inc(struct HAL_DMA_DESC_T *desc, const struct HAL_DMA_BURST_ADDR_INC_T *inc)
{
#if !(defined(CHIP_BEST1000) || defined(CHIP_BEST2000) || defined(CHIP_BEST3001) || defined(CHIP_BEST3005))
    if (inc && inc->src_inc_en) {
        desc->ctrl |= DMA_CONTROL_BURST_SI;
    } else {
        desc->ctrl &= ~DMA_CONTROL_BURST_SI;
    }
    if (inc && inc->dst_inc_en) {
        desc->ctrl |= DMA_CONTROL_BURST_DI;
    } else {
        desc->ctrl &= ~DMA_CONTROL_BURST_DI;
    }
#endif
}

void hal_dma_clear_desc_burst_addr_inc(struct HAL_DMA_DESC_T *desc)
{
    hal_dma_set_desc_burst_addr_inc(desc, NULL);
}

#ifdef DMA_REMAP
void hal_dma_remap_periph(enum HAL_DMA_PERIPH_T periph, int enable)
{
    enum HAL_DMA_INST_T inst1, inst2;
    uint8_t index1, index2;
    int ret;
    uint32_t lock;

    // Tag the periph
    ret = get_index_from_periph(periph, &inst1, &index1);
    ASSERT(ret == 0, "Invalid DMA periph for remap1: %d", periph);

    // Tag the peer periph
    inst2 = inst1 ^ 1;
    ret = get_remap_index_from_periph(inst2, periph, &index2);
    ASSERT(ret == 0, "Invalid DMA periph for remap2: %d", periph);

    lock = int_lock();
    if (enable) {
        periph_remap_bitmap[inst1] |= (1 << index1);
        periph_remap_bitmap[inst2] |= (1 << index2);
#ifndef CHIP_BEST1000
        // For best2000, index1 == index2
        hal_cmu_dma_swap_enable(index1);
#endif
    } else {
        periph_remap_bitmap[inst1] &= ~(1 << index1);
        periph_remap_bitmap[inst2] &= ~(1 << index2);
#ifndef CHIP_BEST1000
        hal_cmu_dma_swap_disable(index1);
#endif
    }
    int_unlock(lock);
}
#endif

void hal_dma_open(void)
{
    enum HAL_DMA_INST_T inst;

    if (dma_opened) {
        return;
    }

    for (inst = HAL_DMA_INST_0; inst < HAL_DMA_INST_QTY; inst++) {
        hal_dma_open_inst(inst);
    }

#ifdef DMA_REMAP
#ifdef CHIP_BEST1000
    if (hal_get_chip_metal_id() >= HAL_CHIP_METAL_ID_4) {
        hal_dma_remap_periph(HAL_AUDMA_I2S0_RX, 1);
        hal_dma_remap_periph(HAL_AUDMA_I2S0_TX, 1);
        hal_dma_remap_periph(HAL_AUDMA_SPDIF0_RX, 1);
        hal_dma_remap_periph(HAL_AUDMA_SPDIF0_TX, 1);
    }
#endif
#endif

    dma_opened = true;
}

void hal_dma_close(void)
{
    enum HAL_DMA_INST_T inst;

    if (!dma_opened) {
        return;
    }

    for (inst = 0; inst < HAL_DMA_INST_QTY; inst++) {
        hal_dma_close_inst(inst);
    }

    dma_opened = false;
}

#ifdef CORE_SLEEP_POWER_DOWN
void hal_dma_sleep(void)
{
    enum HAL_DMA_INST_T inst;

    for (inst = 0; inst < HAL_DMA_INST_QTY; inst++) {
        saved_dma_regs[inst] = dma[inst]->DMACONFIG;
    }
}

void hal_dma_wakeup(void)
{
    enum HAL_DMA_INST_T inst;

    for (inst = 0; inst < HAL_DMA_INST_QTY; inst++) {
        dma[inst]->DMACONFIG = saved_dma_regs[inst];
    }
}
#endif

HAL_DMA_DELAY_FUNC hal_dma_set_delay_func(HAL_DMA_DELAY_FUNC new_func)
{
    HAL_DMA_DELAY_FUNC old_func = dma_delay;
    dma_delay = new_func;
    return old_func;
}

void hal_dma_record_busy_chan(void)
{
#ifdef DMA_DEBUG
    enum HAL_DMA_INST_T inst;
    int hwch;

    for (inst = HAL_DMA_INST_0; inst < HAL_DMA_INST_QTY; inst++) {
        debug_user_map[inst] = 0;
        for (hwch = chan_start[inst]; hwch < chan_num[inst]; hwch++) {
            if (hal_dma_chan_busy_inst(inst, hwch)) {
                debug_user_map[inst] |= (1 << hwch);
            }
        }
    }
#endif
}

void hal_dma_sync_for_cpu(const void *buf, uint32_t len)
{
    hal_cache_invalidate(HAL_CACHE_ID_D_CACHE, (uint32_t)buf, len);
}

void hal_dma_sync_for_device(const void *buf, uint32_t len)
{
    hal_cache_sync(HAL_CACHE_ID_D_CACHE, (uint32_t)buf, len);
}

void hal_dma_print_busy_chan(void)
{
#ifdef DMA_DEBUG
    enum HAL_DMA_INST_T inst;
    int hwch;

    for (inst = HAL_DMA_INST_0; inst < HAL_DMA_INST_QTY; inst++) {
        for (hwch = chan_start[inst]; hwch < chan_num[inst]; hwch++) {
            if (debug_user_map[inst] & (1 << hwch)) {
                TRACE(0, "*** DMA CHAN BUSY: %u-%u %8X", inst, hwch, (uint32_t)chan_user[inst][hwch]);
            }
        }
    }
#endif
}

#else// !DMA0_BASE

void hal_dma_open(void)
{
}

void hal_dma_close(void)
{
}

void hal_dma_sleep(void)
{
}

void hal_dma_wakeup(void)
{
}

bool hal_dma_chan_busy(uint8_t ch)
{
    return false;
}

bool hal_dma_busy(void)
{
    return false;
}

void hal_dma_record_busy_chan(void)
{
}

void hal_dma_print_busy_chan(void)
{
}

#endif
