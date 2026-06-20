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
 *  2021-02-22     tanwenchen    New document.
 ****************************************************************************/
#include "plat_addr_map.h"

#ifdef IRDA_BASE

#include "cmsis_nvic.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "reg_ir.h"
#include "hal_ir.h"

#ifndef FPGA
#include "hal_iomux.h"
#endif

#ifndef IR_DONOT_USE_DMA
#include "hal_dma.h"
#include "string.h"
#endif

#ifdef RTOS
#include "cmsis_os.h"
#endif

#define HAL_IR_DEBUG

#define IR_CLK 24000000 //default,come form osc
static struct IR_T * const ir = (struct IR_T *)IRDA_BASE;

static HAL_IR_INT_HANDLER_T ir_handler;
static enum HAL_IR_MODE_T rx_tx_flag;

void hal_ir_get_int_status(uint16_t *status)
{
    *status = ir->INT_STATUS;
}

void hal_ir_get_fifo_status(uint16_t *status)
{
    *status = ir->FIFO_STATUS;
}

void hal_ir_get_rx_length(uint8_t *len)
{
    *len = ir->FIFO_STATUS & 0x3F;
}

bool hal_ir_check_rx_busy(void)
{
    return((ir->INT_STATUS & IR_RX_AUTO_DONE_STATUS) ? false : true);
}

bool hal_ir_check_tx_busy(void)
{
    return((ir->INT_STATUS & IR_TX_AUTO_DONE_STATUS) ? false : true);
}

void hal_ir_set_tx_done_mask(void)
{
    ir->INT_MASK = IR_TX_AUTO_DONE_MSK;
}

void hal_ir_set_rx_done_mask(void)
{
    ir->INT_MASK = IR_RX_AUTO_DONE_MSK;
}

static void hal_ir_irq_handler(void)
{
    uint16_t status;
    hal_ir_get_int_status(&status);

#ifdef HAL_IR_DEBUG
    uint16_t fifostatus;
    hal_ir_get_fifo_status(&fifostatus);
    TRACE(1,"ir irq status=0x%X,tx/rx fifo level=0x%X/0x%X", status, (fifostatus)>>8, (fifostatus)&0x3F);
#endif

    if(rx_tx_flag == HAL_IR_MODE_TX)
    {
#ifdef IR_DONOT_USE_DMA
        if((status & IR_TX_FIFO_EMPTY_STATUS)&&(!(status & IR_TX_AUTO_DONE_STATUS)))
        {
            if(ir_handler)
            {
                ir_handler(NULL,0,false);
            }
        }
#endif
        if(status & IR_TX_AUTO_DONE_STATUS)
        {
            if(ir_handler)
            {
                ir_handler(NULL,0,true);
            }
        }
    }
    else
    {
#ifdef IR_DONOT_USE_DMA
        if((status & IR_RX_FIFO_ALMOST_FULL_STATUS)&&(!(status & IR_RX_AUTO_DONE_STATUS)))
        {
            if(ir_handler)
            {
                ir_handler(NULL,0,false);
            }
        }
#endif
        if(status & IR_RX_AUTO_DONE_STATUS)
        {
            if(ir_handler)
            {
                ir_handler(NULL,0,true);
            }
        }
    }
    ir->INT_CLR = 0xFFFF;
    ir->INT_CLR = 0xFFFF;
}

static void hal_ir_irq_enable(void)
{
    NVIC_SetVector(IR_IRQn, (uint32_t)hal_ir_irq_handler);
    NVIC_SetPriority(IR_IRQn, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(IR_IRQn);
    NVIC_EnableIRQ(IR_IRQn);
}

static void hal_ir_irq_disable(void)
{
    NVIC_DisableIRQ(IR_IRQn);
}

void hal_ir_open(const struct HAL_IR_CFG_T *cfg)
{
    uint16_t config=0;

    ASSERT(cfg, "%s:ir cfg cannot be NULL,line=%d",__func__,__LINE__);

    hal_cmu_clock_enable(HAL_CMU_MOD_P_IR);
    hal_cmu_clock_enable(HAL_CMU_MOD_Q_IR);
    hal_cmu_reset_clear(HAL_CMU_MOD_P_IR);
    hal_cmu_reset_clear(HAL_CMU_MOD_Q_IR);

#ifndef FPGA
    hal_iomux_set_ir();
#endif

    if(cfg->ir_mode == HAL_IR_MODE_RX)
    {
        rx_tx_flag = HAL_IR_MODE_RX;
        ir->RX_CLK_DIV = (uint16_t)(IR_CLK / cfg->rx_clk - 1);
        ir->REC_PERIOD_SINGLE = (uint32_t)cfg->rec_duty_single | cfg->rec_period_single;
        ir->REC_CLK_CNT_IDLE = cfg->rec_clk_cnt_idle;
        if(cfg->use_dma)
        {
            ir->INT_MASK = IR_RX_AUTO_DONE_MSK;
        }
        else
        {
            ir->INT_MASK = IR_RX_AUTO_DONE_MSK | IR_RX_FIFO_ALMOST_FULL_MSK;
        }

        config = (cfg->rx_default_level ? IR_RX_DEFAULT_HIGH : 0)   |
                 (cfg->rx_invert ? IR_RX_INVERT : 0)                |
                 (cfg->use_dma ? IR_DMA_MODE : 0)                   |
                 (cfg->ir_mode ? IR_IR_MODE : 0)                    |
                 (IR_RX_EDGE(cfg->rx_edge))                         |
                 (cfg->clear_fifo ? IR_CLR_FIFO : 0);
    }
    else if(cfg->ir_mode == HAL_IR_MODE_TX)
    {
        rx_tx_flag = HAL_IR_MODE_TX;
        ir->TX_CLK_DIV = (uint16_t)(IR_CLK / cfg->tx_clk - 1);
        switch(cfg->tx_clk_duty)
        {
            case HAL_IR_TX_CLK_DUTY_1_1:
            {
                ir->TX_CLK_DUTY = (uint16_t)(IR_CLK / cfg->tx_clk / 2);
                break;
            }
            case HAL_IR_TX_CLK_DUTY_1_2:
            {
                ir->TX_CLK_DUTY = (uint16_t)(IR_CLK / cfg->tx_clk / 3);
                break;
            }
            case HAL_IR_TX_CLK_DUTY_1_3:
            {
                ir->TX_CLK_DUTY = (uint16_t)(IR_CLK / cfg->tx_clk / 4);
                break;
            }
            case HAL_IR_TX_CLK_DUTY_1_4:
            {
                ir->TX_CLK_DUTY = (uint16_t)(IR_CLK / cfg->tx_clk / 5);
                break;
            }
            case HAL_IR_TX_CLK_DUTY_1_5:
            {
                ir->TX_CLK_DUTY = (uint16_t)(IR_CLK / cfg->tx_clk / 6);
                break;
            }
            case HAL_IR_TX_CLK_DUTY_1_6:
            {
                ir->TX_CLK_DUTY = (uint16_t)(IR_CLK / cfg->tx_clk / 7);
                break;
            }
            default:
            {
                ASSERT(0, "%s:ir tx_clk_duty error,line=%d",__func__,__LINE__);
                break;
            }
        }

        if(cfg->use_dma)
        {
            ir->INT_MASK = IR_TX_AUTO_DONE_MSK;
        }
        else
        {
            ir->INT_MASK = IR_TX_AUTO_DONE_MSK | IR_TX_FIFO_EMPTY_MSK;
        }

        config = (cfg->use_dma ? IR_DMA_MODE : 0)                   |
                 (cfg->ir_mode ? IR_IR_MODE : 0)                    |
                 (IR_TX_LEVEL(cfg->tx_level))                       |
                 (cfg->clear_fifo ? IR_CLR_FIFO : 0);
    }
    else
    {
        ASSERT(0, "%s:ir tx/rx mode error,line=%d",__func__,__LINE__);
    }
#ifdef IR_DONOT_USE_DMA
    if (cfg->use_dma)
    {
        ASSERT(0, "%s:using DMA when IR_DONOT_USE_DMA is enabled,line=%d",__func__,__LINE__);
    }
#endif

    //common config
    ir_handler = cfg->handler;
    ir->CONFIG = 1;//clear fifo
    ir->CONFIG = config;
    ir->AUTO_SLEEP_CNT = cfg->auto_sleep_cnt;
    ir->FIFO_AE_LEVEL = cfg->fifo_ae_level;
    ir->FIFO_AF_LEVEL = cfg->fifo_af_level;
    ir->INT_CLR = 0xFFFF;
    ir->EN = (cfg->ir_mode == HAL_IR_MODE_RX) ? IR_IR_EN_RX : IR_IR_EN_TX;

#ifndef IR_DONOT_USE_DMA
    hal_ir_stop_dma_rx();
    hal_ir_stop_dma_tx();
#endif
    hal_ir_irq_enable();
}

void hal_ir_close(void)
{
    uint8_t time = 0;
    if(rx_tx_flag == HAL_IR_MODE_RX)
    {
        while(hal_ir_check_rx_busy() == true)
        {
#ifdef RTOS
            osDelay(2);
#else
            hal_sys_timer_delay(MS_TO_TICKS(2));
#endif
            time++;
            if(time >=5)//10ms timeout
            {
                ASSERT(false, "%s:ir rx close timeout,line=%d",__func__,__LINE__);
            }
        }
    }
    else
    {
        while(hal_ir_check_tx_busy() == true)
        {
#ifdef RTOS
            osDelay(2);
#else
            hal_sys_timer_delay(MS_TO_TICKS(2));
#endif
            time++;
            if(time >=5)//10ms timeout
            {
                ASSERT(false, "%s:ir tx close timeout,line=%d",__func__,__LINE__);
            }
        }
    }

    hal_ir_irq_disable();
#ifndef IR_DONOT_USE_DMA
    if(rx_tx_flag == HAL_IR_MODE_RX)
    {
        hal_ir_stop_dma_rx();
    }
    else
    {
        hal_ir_stop_dma_tx();
    }
#endif
    ir->INT_CLR = 0xFFFF;
    ir->EN = 0;
}

void hal_ir_tx(const uint8_t *data, uint8_t length)
{
    ASSERT(length < 32, "%s:The length of the data sent exceeds the depth of fifo(max=32bytes):%d,line=%d",__func__,length,__LINE__);

    for(uint8_t i=0;i<length;i++)
    {
        if(data[i])
        {
            ir->TX_DATA = data[i]-1;
        }
    }
}

void hal_ir_tx_byte(const uint8_t data)
{
    if(data)
    {
        ir->TX_DATA = data-1;
    }
}

void hal_ir_rx(uint8_t *data, uint8_t length)
{
    ASSERT(length < 32, "%s:The length of the received data exceeds the depth of fifo(max=32bytes):%d,line=%d",__func__,length,__LINE__);

    for(uint8_t i=0;i<length;i++)
    {
        data[i] = ir->RX_DATA;
    }
}

/*************************dma related functions,2021-02-26*********************/
#ifndef IR_DONOT_USE_DMA

#define HAL_IR_ID       0
#define HAL_IR_ID_QTY   1

static bool in_use[HAL_IR_ID_QTY] = { false, };
static HAL_IR_DMA_HANDLER_T ir_txdma_handler[HAL_IR_ID_QTY];
static HAL_IR_DMA_HANDLER_T ir_rxdma_handler[HAL_IR_ID_QTY];
static uint8_t ir_txdma_chan[HAL_IR_ID_QTY] = {HAL_DMA_CHAN_NONE};
static uint8_t ir_rxdma_chan[HAL_IR_ID_QTY] = {HAL_DMA_CHAN_NONE};

static void hal_ir_rxdma_handler(uint8_t chan, uint32_t remains, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    uint8_t id;
    uint32_t lock;

    id = HAL_IR_ID;
    lock = int_lock();
    ir_rxdma_chan[id] = HAL_DMA_CHAN_NONE;
    int_unlock(lock);

    hal_gpdma_free_chan(chan);
    clear_bool_flag(&in_use[id]);
    if (ir_rxdma_handler[id])
    {
        ir_rxdma_handler[id](error);
    }
}

static int hal_ir_dma_recv(uint8_t id, uint8_t *data, uint32_t len, HAL_IR_DMA_HANDLER_T handler)
{
    uint8_t cnt;
    uint32_t lock;
    enum HAL_DMA_RET_T ret;
    struct HAL_DMA_CH_CFG_T dma_cfg;
    enum HAL_DMA_WDITH_T dma_width;
    enum HAL_DMA_PERIPH_T src_periph;

    ASSERT((ir->CONFIG & IR_DMA_MODE), "%s:DMA not configured on IR,line=%d",__func__,__LINE__);
    ir_rxdma_handler[id] = handler;

    src_periph = HAL_GPDMA_IR_RX;
    dma_width = HAL_DMA_WIDTH_BYTE;
    if(dma_width == HAL_DMA_WIDTH_BYTE)
    {
        cnt = 1;
    }
    else
    {
        ASSERT(false, "%s:DMA data width error,line=%d",__func__,__LINE__);
    }

    lock = int_lock();
    if (ir_rxdma_chan[id] != HAL_DMA_CHAN_NONE)
    {
        int_unlock(lock);
        return -1;
    }
    ir_rxdma_chan[id] = hal_gpdma_get_chan(src_periph, HAL_DMA_HIGH_PRIO);
    if (ir_rxdma_chan[id] == HAL_DMA_CHAN_NONE)
    {
        int_unlock(lock);
        return -2;
    }
    int_unlock(lock);

    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.ch = ir_rxdma_chan[id];
    dma_cfg.dst = (uint32_t)data;
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_16;
    //dma_cfg.dst_periph = HAL_GPDMA_PERIPH_QTY; // useless
    dma_cfg.dst_width = dma_width;
    dma_cfg.handler = handler ? hal_ir_rxdma_handler : NULL;
    dma_cfg.src = 0; // useless
    dma_cfg.src_periph = src_periph;
    dma_cfg.src_bsize = HAL_DMA_BSIZE_16;
    dma_cfg.src_tsize = len / cnt;
    dma_cfg.src_width = dma_width;
    dma_cfg.try_burst = 0;
    dma_cfg.type = HAL_DMA_FLOW_P2M_DMA;

    ret = hal_gpdma_start(&dma_cfg);
    if (ret != HAL_DMA_OK)
    {
        return -3;
    }

    if (handler == NULL)
    {
        while ((ir->CONFIG & IR_IR_MODE) && hal_gpdma_chan_busy(ir_rxdma_chan[id]));
        hal_gpdma_free_chan(ir_rxdma_chan[id]);
        ir_rxdma_chan[id] = HAL_DMA_CHAN_NONE;
    }

    return 0;
}

void hal_ir_stop_dma_rx(void)
{
    uint8_t id;
    uint32_t lock;
    uint8_t rx_chan;

    id = HAL_IR_ID;
    lock = int_lock();
    rx_chan = ir_rxdma_chan[id];
    ir_rxdma_chan[id] = HAL_DMA_CHAN_NONE;
    int_unlock(lock);

    if (rx_chan == HAL_DMA_CHAN_NONE)
    {
        return;
    }

    if (rx_chan != HAL_DMA_CHAN_NONE)
    {
        hal_gpdma_cancel(rx_chan);
        hal_gpdma_free_chan(rx_chan);
    }
    clear_bool_flag(&in_use[id]);
}

static void hal_ir_txdma_handler(uint8_t chan, uint32_t remains, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    uint8_t id;
    uint32_t lock;

    id = HAL_IR_ID;
    lock = int_lock();
    ir_txdma_chan[id] = HAL_DMA_CHAN_NONE;
    int_unlock(lock);

    hal_gpdma_free_chan(chan);
    clear_bool_flag(&in_use[id]);
    if (ir_txdma_handler[id])
    {
        ir_txdma_handler[id](error);
    }
}

static int hal_ir_dma_send(uint8_t id, const uint8_t *data, uint32_t len, HAL_IR_DMA_HANDLER_T handler)
{
    uint8_t cnt=0;
    uint32_t lock;
    enum HAL_DMA_RET_T ret;
    struct HAL_DMA_CH_CFG_T dma_cfg;
    enum HAL_DMA_WDITH_T dma_width;
    enum HAL_DMA_PERIPH_T dst_periph;

    ASSERT((ir->CONFIG & IR_DMA_MODE), "%s:DMA not configured on IR,line=%d",__func__,__LINE__);
    ir_txdma_handler[id] = handler;

    dst_periph = HAL_GPDMA_IR_TX;
    dma_width = HAL_DMA_WIDTH_BYTE;
    if(dma_width == HAL_DMA_WIDTH_BYTE)
    {
        cnt = 1;
    }
    else
    {
        ASSERT(false, "%s:DMA data width error,line=%d",__func__,__LINE__);
    }

    lock = int_lock();
    if (ir_txdma_chan[id] != HAL_DMA_CHAN_NONE)
    {
        int_unlock(lock);
        return -1;
    }
    ir_txdma_chan[id] = hal_gpdma_get_chan(dst_periph, HAL_DMA_HIGH_PRIO);
    if (ir_txdma_chan[id] == HAL_DMA_CHAN_NONE)
    {
        int_unlock(lock);
        return -2;
    }
    int_unlock(lock);

    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.ch = ir_txdma_chan[id];
    dma_cfg.dst = 0; // useless
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_16;
    dma_cfg.dst_periph = dst_periph;
    dma_cfg.dst_width = dma_width;
    dma_cfg.handler = handler ? hal_ir_txdma_handler : NULL;
    dma_cfg.src = (uint32_t)data;
    dma_cfg.src_bsize = HAL_DMA_BSIZE_16;
    //dma_cfg.src_periph = HAL_GPDMA_PERIPH_QTY; // useless
    dma_cfg.src_tsize = len / cnt;
    dma_cfg.src_width = dma_width;
    dma_cfg.try_burst = 0;
    dma_cfg.type = HAL_DMA_FLOW_M2P_DMA;

    ret = hal_gpdma_start(&dma_cfg);
    if (ret != HAL_DMA_OK)
    {
        return -3;
    }

    if (handler == NULL)
    {
        while ((ir->CONFIG & IR_IR_MODE) && hal_gpdma_chan_busy(ir_txdma_chan[id]));
        hal_gpdma_free_chan(ir_txdma_chan[id]);
        ir_txdma_chan[id] = HAL_DMA_CHAN_NONE;
    }

    return 0;
}

void hal_ir_stop_dma_tx(void)
{
    uint8_t id;
    uint32_t lock;
    uint8_t tx_chan;

    id = HAL_IR_ID;
    lock = int_lock();
    tx_chan = ir_txdma_chan[id];
    ir_txdma_chan[id] = HAL_DMA_CHAN_NONE;
    int_unlock(lock);

    if (tx_chan != HAL_DMA_CHAN_NONE)
    {
        hal_gpdma_cancel(tx_chan);
        hal_gpdma_free_chan(tx_chan);
    }

    clear_bool_flag(&in_use[id]);
}

int hal_ir_dma_rx(uint8_t *data, uint32_t len, HAL_IR_DMA_HANDLER_T handler)
{
    int ret;
    if (set_bool_flag(&in_use[HAL_IR_ID]))
    {
        return -31;
    }

    ASSERT(rx_tx_flag == HAL_IR_MODE_RX, "%s:The current infrared transceiver mode is wrong,line=%d",__func__,__LINE__);

    ret = hal_ir_dma_recv(HAL_IR_ID, data, len, handler);
    if (ret || handler == NULL)
    {
        clear_bool_flag(&in_use[HAL_IR_ID]);
    }

    return ret;
}

int hal_ir_dma_tx(const uint8_t *data, uint32_t len, HAL_IR_DMA_HANDLER_T handler)
{
    int ret;
    if (set_bool_flag(&in_use[HAL_IR_ID]))
    {
        return -31;
    }

    ASSERT(rx_tx_flag == HAL_IR_MODE_TX, "%s:The current infrared transceiver mode is wrong,line=%d",__func__,__LINE__);

    ret = hal_ir_dma_send(HAL_IR_ID, data, len, handler);
    if (ret || handler == NULL)
    {
        clear_bool_flag(&in_use[HAL_IR_ID]);
    }

    return ret;
}
#endif

#endif

