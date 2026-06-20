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
#ifndef __HAL_DSI_H__
#define __HAL_DSI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"

typedef void (*HAL_DSI_TE_GPIO_IRQ_HANDLER)(void);
typedef void (*HAL_DSI_ULPS_CB_HANDLER)(uint32_t param);

struct HAL_DSI_CFG_T {
    uint32_t active_width;
    uint32_t active_height;//screen size
    uint32_t h_front_porch;
    uint32_t h_back_porch;
    uint32_t v_front_porch;
    uint32_t v_back_porch;
    uint32_t total_width;
    uint32_t total_height;

    // pn video
    uint32_t y_mem_pitch;
    uint32_t uv_mem_pitch;
    uint32_t c_mem_pitch;
    uint32_t image_h_sa;
    uint32_t image_v_sa;
    uint32_t image_width;
    uint32_t image_height;
    uint32_t zm_image_width;
    uint32_t zm_image_height;

    // pn graphic
    uint32_t g_mem_pitch;
    uint32_t graphic_h_sa;
    uint32_t graphic_v_sa;
    uint32_t graphic_width;
    uint32_t graphic_height;
    uint32_t zm_graphic_width;
    uint32_t zm_graphic_height;

    // tv graphic
    uint32_t g_tv_mem_pitch;
    uint32_t tvg_h_sa;
    uint32_t tvg_v_sa;
    uint32_t tvg_width;
    uint32_t tvg_height;
    uint32_t zm_tvg_width;
    uint32_t zm_tvg_height;

    // pn cursor
    uint32_t cursor_h_sa;
    uint32_t cursor_v_sa;
    uint32_t cursor_width;
    uint32_t cursor_height;
    int hwc_color1;
    int hwc_color2;

    //background
    int blankcolor;

    // pn viedo setting
    int cos0;
    int sin0;
    int c_mult_s;
    int saturation;
    int brightness;
    int contrast;

    // pn keys
    int cfg_alpha_y;
    int cfg_ckey_y;
    int cfg_ckey_y1;
    int cfg_ckey_y2;
    int cfg_alpha_u;
    int cfg_ckey_u;
    int cfg_ckey_u1;
    int cfg_ckey_u2;
    int cfg_alpha_v;
    int cfg_ckey_v;
    int cfg_ckey_v1;
    int cfg_ckey_v2;
};

/**
 * @brief hal_dsi_init - init dsi phy
 *
 * @param h_res : panel horizontal resolution
 */
void hal_dsi_init(uint16_t h_res);

/**
 * @brief hal_dsi_reset - reset phy, besause hs mode phy work error,
 *                        lp cmd send has some errors.
 *
 * @param h_res : panel horizontal resolution
 */
void hal_dsi_reset(uint16_t h_res);

/**
 * @brief hal_dsi_send_cmd - send cmd by lp mode
 *
 * @param cmd
 */
void hal_dsi_send_cmd(uint8_t cmd);

/**
 * @brief hal_dsi_read_cmd - read data by lp mode
 *
 * @param cmd : cmd
 * @param read_buf : read buffer
 * @param len : read len
 * @return int
 */
int hal_dsi_read_cmd(uint8_t cmd,uint8_t* read_buf,uint8_t len);

/**
 * @brief hal_dsi_send_cmd_data - send cmd with data by lp mode
 *
 * @param cmd : cmd
 * @param len : send param num
 * @param p0 : param0
 * @param p1 : param1
 * @param p2 : param2
 * @param p3 : param3
 */
void hal_dsi_send_cmd_data(uint8_t cmd, uint32_t len, uint8_t p0,
                           uint8_t p1, uint8_t p2, uint8_t p3);
/**
 * @brief hal_dsi_send_long_array - send cmd with data with array
 *
 * @param len : array len
 * @param data : array data
 */
void hal_dsi_send_long_array(uint32_t len,uint32_t *data);

/**
 * @brief hal_dsi_send_cmd_list - send cmd with data list
 *
 * @param cmd
 * @param para_count
 * @param para_list
 */
void hal_dsi_send_cmd_list(unsigned cmd, unsigned char para_count,
                           unsigned char *para_list);

/**
 * hal_dsi_irqn - get dsi irq number
 */
int hal_dsi_irqn(void);

/**
 * hal_dsi_get_irqstate - get dsi irq state
 */
uint32_t hal_dsi_get_irqstate(void);

/**
 * hal_dsi_teirq_enable - enable dsi te irq
 */
void hal_dsi_teirq_enable(void);

/**
 * hal_dsi_irq_disable - disable dsi all irqs
 */
void hal_dsi_irq_disable(void);

/**
 * hal_dsi_clear_teirq_flag - clear dsi te irq state
 */
void hal_dsi_clear_teirq_flag(void);

/**
 * hal_dsi_irq_is_te - irq is te
 * @regval : dsi ISR value
 */
bool hal_dsi_irq_is_te(uint32_t regval);

/**
 * hal_dsi_irq_is_err - dsi is irq err
 */
bool hal_dsi_irq_is_err(uint32_t regval);

/**
 * hal_dsi_irq_clear_err - clear dsi error flag.
 */
void hal_dsi_irq_clear_err(uint32_t status);

/**
 * hal_dsi_enter_ulps_mode - enter ulps mode
 */
void hal_dsi_enter_ulps_mode();

/**
 * hal_dsi_exit_ulps_mode - exit ulps mode
 */
void hal_dsi_exit_ulps_mode(void);

void hal_dsi_exit_ulps_with_cb(HAL_DSI_ULPS_CB_HANDLER hdlr,void *param);

void hal_dsi_start(void);

void hal_dsi_sleep(void);

void hal_dsi_wakeup(void);

void hal_dsi_te_gpio_irq_register(HAL_DSI_TE_GPIO_IRQ_HANDLER hdlr);

void hal_dsi_te_gpio_irq_enable(void);

void hal_dsi_te_gpio_irq_disable(void);

#ifdef __cplusplus
}
#endif

#endif

