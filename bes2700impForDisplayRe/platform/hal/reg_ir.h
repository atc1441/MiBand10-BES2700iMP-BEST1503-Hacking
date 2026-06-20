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
#ifndef __REG_IR_BEST2003_H__
#define __REG_IR_BEST2003_H__

#include "plat_types.h"

struct IR_T {
    __I  uint32_t RX_DATA;              //0x00
    __IO uint32_t TX_DATA;              //0x04
    __IO uint32_t TX_CLK_DIV;           //0x08
    __IO uint32_t TX_CLK_DUTY;          //0x0C
    __IO uint32_t RX_CLK_DIV;           //0x10
    __IO uint32_t REC_PERIOD_SINGLE;    //0x14
    __IO uint32_t REC_CLK_CNT_IDLE;     //0x18
    __IO uint32_t AUTO_SLEEP_CNT;       //0x1C
    __IO uint32_t CONFIG;               //0x20
    __IO uint32_t EN;                   //0x24
    __IO uint32_t INT_MASK;             //0x28
    __IO uint32_t INT_CLR;              //0x2C
    __I  uint32_t INT_STATUS;           //0x30
    __I  uint32_t TXRX_STATUS;          //0x34
    __I  uint32_t FIFO_STATUS;          //0x38
    __IO uint32_t FIFO_AE_LEVEL;        //0x3C
    __IO uint32_t FIFO_AF_LEVEL;        //0x40
};

// reg_000
#define IR_RX_DATA(n)                                       (((n) & 0xFF) << 0)
#define IR_RX_DATA_MASK                                     (0xFF << 0)
#define IR_RX_DATA_SHIFT                                    (0)

// reg_004
#define IR_TX_DATA(n)                                       (((n) & 0xFF) << 0)
#define IR_TX_DATA_MASK                                     (0xFF << 0)
#define IR_TX_DATA_SHIFT                                    (0)

// reg_008
#define IR_TX_CLK_DIV(n)                                    (((n) & 0xFFFF) << 0)
#define IR_TX_CLK_DIV_MASK                                  (0xFFFF << 0)
#define IR_TX_CLK_DIV_SHIFT                                 (0)

// reg_00c
#define IR_TX_CLK_DUTY(n)                                   (((n) & 0xFFFF) << 0)
#define IR_TX_CLK_DUTY_MASK                                 (0xFFFF << 0)
#define IR_TX_CLK_DUTY_SHIFT                                (0)

// reg_010
#define IR_RX_CLK_DIV(n)                                    (((n) & 0xFFFF) << 0)
#define IR_RX_CLK_DIV_MASK                                  (0xFFFF << 0)
#define IR_RX_CLK_DIV_SHIFT                                 (0)

// reg_014
#define IR_REC_PERIOD_SINGLE(n)                             (((n) & 0xFFFF) << 0)
#define IR_REC_PERIOD_SINGLE_MASK                           (0xFFFF << 0)
#define IR_REC_PERIOD_SINGLE_SHIFT                          (0)
#define IR_REC_DUTY_SINGLE(n)                               (((n) & 0xFFFF) << 16)
#define IR_REC_DUTY_SINGLE_MASK                             (0xFFFF << 16)
#define IR_REC_DUTY_SINGLE_SHIFT                            (16)

// reg_018
#define IR_REC_CLK_CNT_IDLE(n)                              (((n) & 0x7F) << 0)
#define IR_REC_CLK_CNT_IDLE_MASK                            (0x7F << 0)
#define IR_REC_CLK_CNT_IDLE_SHIFT                           (0)

// reg_01c
#define IR_AUTO_SLEEP_CNT(n)                                (((n) & 0xFFFF) << 0)
#define IR_AUTO_SLEEP_CNT_MASK                              (0xFFFF << 0)
#define IR_AUTO_SLEEP_CNT_SHIFT                             (0)

// reg_020
#define IR_CLR_FIFO                                         (1 << 0)
#define IR_RX_EDGE(n)                                       (((n) & 0x3) << 1)
#define IR_RX_EDGE_MASK                                     (0x3 << 1)
#define IR_RX_EDGE_SHIFT                                    (1)
#define IR_TX_LEVEL(n)                                      (((n) & 0x3) << 3)
#define IR_TX_LEVEL_MASK                                    (0x3 << 3)
#define IR_TX_LEVEL_SHIFT                                   (3)
#define IR_IR_MODE                                          (1 << 5)
#define IR_DMA_MODE                                         (1 << 6)
#define IR_RX_INVERT                                        (1 << 7)
#define IR_RX_DEFAULT_HIGH                                  (1 << 8)

// reg_024
#define IR_IR_EN_TX                                         (1 << 0)
#define IR_IR_EN_RX                                         (1 << 1)

// reg_028
#define IR_TX_AUTO_DONE_MSK                                 (1 << 0)
#define IR_TX_ERR_MSK                                       (1 << 1)
#define IR_RX_AUTO_DONE_MSK                                 (1 << 2)
#define IR_RX_ERR_MSK                                       (1 << 3)
#define IR_RX_FIFO_UNDERFLOW_MSK                            (1 << 4)
#define IR_RX_FIFO_EMPTY_MSK                                (1 << 5)
#define IR_RX_FIFO_ALMOST_EMPTY_MSK                         (1 << 6)
#define IR_RX_FIFO_ALMOST_FULL_MSK                          (1 << 7)
#define IR_RX_FIFO_FULL_MSK                                 (1 << 8)
#define IR_RX_FIFO_OVERFULL_MSK                             (1 << 9)
#define IR_TX_FIFO_UNDERFLOW_MSK                            (1 << 10)
#define IR_TX_FIFO_EMPTY_MSK                                (1 << 11)
#define IR_TX_FIFO_ALMOST_EMPTY_MSK                         (1 << 12)
#define IR_TX_FIFO_ALMOST_FULL_MSK                          (1 << 13)
#define IR_TX_FIFO_FULL_MSK                                 (1 << 14)
#define IR_TX_FIFO_OVERFULL_MSK                             (1 << 15)

// reg_02c
#define IR_TX_AUTO_DONE_CLR                                 (1 << 0)
#define IR_TX_ERR_CLR                                       (1 << 1)
#define IR_RX_AUTO_DONE_CLR                                 (1 << 2)
#define IR_RX_ERR_CLR                                       (1 << 3)
#define IR_RX_FIFO_UNDERFLOW_CLR                            (1 << 4)
#define IR_RX_FIFO_EMPTY_CLR                                (1 << 5)
#define IR_RX_FIFO_ALMOST_EMPTY_CLR                         (1 << 6)
#define IR_RX_FIFO_ALMOST_FULL_CLR                          (1 << 7)
#define IR_RX_FIFO_FULL_CLR                                 (1 << 8)
#define IR_RX_FIFO_OVERFULL_CLR                             (1 << 9)
#define IR_TX_FIFO_UNDERFLOW_CLR                            (1 << 10)
#define IR_TX_FIFO_EMPTY_CLR                                (1 << 11)
#define IR_TX_FIFO_ALMOST_EMPTY_CLR                         (1 << 12)
#define IR_TX_FIFO_ALMOST_FULL_CLR                          (1 << 13)
#define IR_TX_FIFO_FULL_CLR                                 (1 << 14)
#define IR_TX_FIFO_OVERFULL_CLR                             (1 << 15)

// reg_030
#define IR_TX_AUTO_DONE_STATUS                              (1 << 0)
#define IR_TX_ERR_STATUS                                    (1 << 1)
#define IR_RX_AUTO_DONE_STATUS                              (1 << 2)
#define IR_RX_ERR_STATUS                                    (1 << 3)
#define IR_RX_FIFO_UNDERFLOW_STATUS                         (1 << 4)
#define IR_RX_FIFO_EMPTY_STATUS                             (1 << 5)
#define IR_RX_FIFO_ALMOST_EMPTY_STATUS                      (1 << 6)
#define IR_RX_FIFO_ALMOST_FULL_STATUS                       (1 << 7)
#define IR_RX_FIFO_FULL_STATUS                              (1 << 8)
#define IR_RX_FIFO_OVERFULL_STATUS                          (1 << 9)
#define IR_TX_FIFO_UNDERFLOW_STATUS                         (1 << 10)
#define IR_TX_FIFO_EMPTY_STATUS                             (1 << 11)
#define IR_TX_FIFO_ALMOST_EMPTY_STATUS                      (1 << 12)
#define IR_TX_FIFO_ALMOST_FULL_STATUS                       (1 << 13)
#define IR_TX_FIFO_FULL_STATUS                              (1 << 14)
#define IR_TX_FIFO_OVERFULL_STATUS                          (1 << 15)

// reg_034
#define IR_RX_STATUS(n)                                     (((n) & 0x7) << 0)
#define IR_RX_STATUS_MASK                                   (0x7 << 0)
#define IR_RX_STATUS_SHIFT                                  (0)
#define IR_RESERVED_034_3                                   (1 << 3)
#define IR_TX_STATUS(n)                                     (((n) & 0x7) << 4)
#define IR_TX_STATUS_MASK                                   (0x7 << 4)
#define IR_TX_STATUS_SHIFT                                  (4)
#define IR_ZERO_IS_HIGH                                     (1 << 7)

// reg_038
#define IR_RX_FIFO_LEVEL(n)                                 (((n) & 0x3F) << 0)
#define IR_RX_FIFO_LEVEL_MASK                               (0x3F << 0)
#define IR_RX_FIFO_LEVEL_SHIFT                              (0)
#define IR_RESERVED_038_6(n)                                (((n) & 0x3) << 6)
#define IR_RESERVED_038_6_MASK                              (0x3 << 6)
#define IR_RESERVED_038_6_SHIFT                             (6)
#define IR_TX_FIFO_LEVEL(n)                                 (((n) & 0x3F) << 8)
#define IR_TX_FIFO_LEVEL_MASK                               (0x3F << 8)
#define IR_TX_FIFO_LEVEL_SHIFT                              (8)

// reg_03c
#define IR_FIFO_AE_LEVEL(n)                                 (((n) & 0x3F) << 0)
#define IR_FIFO_AE_LEVEL_MASK                               (0x3F << 0)
#define IR_FIFO_AE_LEVEL_SHIFT                              (0)

// reg_040
#define IR_FIFO_AF_LEVEL(n)                                 (((n) & 0x3F) << 0)
#define IR_FIFO_AF_LEVEL_MASK                               (0x3F << 0)
#define IR_FIFO_AF_LEVEL_SHIFT                              (0)

#endif
