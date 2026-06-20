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
#ifndef __HAL_PSRAM_H__
#define __HAL_PSRAM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

enum HAL_PSRAM_ID_T {
    HAL_PSRAM_ID_0 = 0,
    HAL_PSRAM_ID_NUM,
};

enum HAL_PSRAM_CALIB_ITEM_T {
    PSRAM_CALIB_RX_DQS,
    PSRAM_CALIB_TX_DQS,
    PSRAM_CALIB_TX_CLK,
    PSRAM_CALIB_TX_CEB,
};

struct HAL_PSRAM_DQS_BOUNDARY_T {
    uint8_t l;
    uint8_t r;
    bool is_legal;
};

struct HAL_PSRAM_WINDOW_1D_ANALYSIS_T {
    uint8_t range;
    uint8_t t_div_2;
    uint8_t *raw_data;
    enum HAL_PSRAM_CALIB_ITEM_T item;
    struct HAL_PSRAM_DQS_BOUNDARY_T *boundary;
};

struct HAL_PSRAM_RECTANGLE_T{
    uint8_t x_start;
    uint8_t y_start;
    uint8_t x_end;
    uint8_t y_end;
};

typedef void (*HAL_PSRAM_WINDOW_2D_CB_T)(void * const, uint8_t, uint8_t, int, const struct HAL_PSRAM_RECTANGLE_T *);
typedef void (*HAL_PSRAM_WINDOW_1D_CB_T)(struct HAL_PSRAM_WINDOW_1D_ANALYSIS_T *);

#define PSRAM_WINDOW_SIZE    (0x20)

void hal_psram_sleep(void);
void hal_psram_wakeup(void);
void hal_psram_init(void);
void hal_psram_phy_sleep(void);
void hal_psram_snoop_enable(void);
void hal_psram_snoop_disable(void);
void hal_psram_write_buffer_invalidate(void);
void hal_psram_calib(uint32_t clk);
void hal_psram_window_1d_cb_register(HAL_PSRAM_WINDOW_1D_CB_T func);
void hal_psram_window_2d_cb_register(HAL_PSRAM_WINDOW_2D_CB_T func);
uint32_t hal_psram_ca_calib_result(void);

#ifdef __cplusplus
}
#endif

#endif

