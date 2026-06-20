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
#ifndef __HAL_IR_H__
#define __HAL_IR_H__

#ifdef __cplusplus
extern "C" {
#endif

enum HAL_IR_TX_CLK_DUTY_T {
    HAL_IR_TX_CLK_DUTY_1_1 = 0,
    HAL_IR_TX_CLK_DUTY_1_2,
    HAL_IR_TX_CLK_DUTY_1_3,
    HAL_IR_TX_CLK_DUTY_1_4,
    HAL_IR_TX_CLK_DUTY_1_5,
    HAL_IR_TX_CLK_DUTY_1_6,
};

enum HAL_IR_RX_DEFAULT_LEVEL_T {
    HAL_IR_RX_DEFAULT_LEVEL_LOW = 0,
    HAL_IR_RX_DEFAULT_LEVEL_HIGH,
};

enum HAL_IR_RX_INVERT_T {
    HAL_IR_RX_INVERT_DISABLE = 0,
    HAL_IR_RX_INVERT_ENABLE,
};

enum HAL_IR_MODE_T {
    HAL_IR_MODE_RX = 0,
    HAL_IR_MODE_TX,
};

enum HAL_IR_TX_LEVEL_T {
    HAL_IR_TX_LEVEL_0_LOW_1_HIGH = 0,
    HAL_IR_TX_LEVEL_0_LOW_1_CARRIER,
    HAL_IR_TX_LEVEL_0_HIGH_1_LOW,
    HAL_IR_TX_LEVEL_0_HIGH_1_CARRIER,
};

enum HAL_IR_RX_EDGE_T {
    HAL_IR_RX_RISING_CAPTURE_WITH_MODULATION = 0,
    HAL_IR_RX_FALLING_CAPTURE_WITH_MODULATION,
    HAL_IR_RX_BOTH_CAPTURE_WITHOUT_MODULATION,
};

typedef void (*HAL_IR_INT_HANDLER_T)(const uint8_t *buf, uint32_t len, bool rxtx_complete);

struct HAL_IR_CFG_T {
    uint32_t tx_clk;
    enum HAL_IR_TX_CLK_DUTY_T tx_clk_duty;
    uint32_t rx_clk;
    uint16_t rec_period_single;
    uint16_t rec_duty_single;
    uint8_t  rec_clk_cnt_idle:7;
    uint16_t auto_sleep_cnt;
    enum HAL_IR_RX_DEFAULT_LEVEL_T rx_default_level;
    enum HAL_IR_RX_INVERT_T rx_invert;
    uint8_t use_dma:1;
    enum HAL_IR_MODE_T ir_mode;
    enum HAL_IR_TX_LEVEL_T tx_level;
    enum HAL_IR_RX_EDGE_T rx_edge;
    uint8_t clear_fifo:1;
    uint8_t fifo_ae_level:6;
    uint8_t fifo_af_level:6;
    HAL_IR_INT_HANDLER_T handler;
};

void hal_ir_open(const struct HAL_IR_CFG_T *cfg);
void hal_ir_close(void);

void hal_ir_tx_byte(const uint8_t data);
void hal_ir_tx(const uint8_t *data, uint8_t length);
void hal_ir_rx(uint8_t *data, uint8_t length);

void hal_ir_set_tx_done_mask(void);
void hal_ir_set_rx_done_mask(void);
void hal_ir_get_rx_length(uint8_t *len);
void hal_ir_get_int_status(uint16_t *status);
void hal_ir_get_fifo_status(uint16_t *status);

/* dma related functions */
typedef void (*HAL_IR_DMA_HANDLER_T)(int error);

int hal_ir_dma_rx(uint8_t *data, uint32_t len, HAL_IR_DMA_HANDLER_T handler);
int hal_ir_dma_tx(const uint8_t *data, uint32_t len, HAL_IR_DMA_HANDLER_T handler);
void hal_ir_stop_dma_rx(void);
void hal_ir_stop_dma_tx(void);

#ifdef __cplusplus
}
#endif

#endif
