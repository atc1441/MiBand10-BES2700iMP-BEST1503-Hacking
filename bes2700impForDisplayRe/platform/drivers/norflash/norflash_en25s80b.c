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
#include "plat_types.h"
#include "norflash_drv.h"
#include "hal_norflaship.h"
#include "norflash_en25s80b.h"
#include "hal_trace.h"
#include "hal_timer.h"

static void POSSIBLY_UNUSED en25s80b_en25s80b_reset(enum HAL_FLASH_ID_T id)
{
    // ip quad mode
    norflaship_quad_mode(id, 1);
    norflaship_busy_wait(id);

    hal_sys_timer_delay(20);

    // quad reset enable
    norflaship_clear_txfifo(id);
    norflaship_cmd_addr(id, EN25S80B_CMD_QUAD_RESET_ENABLE, 0);
    norflaship_busy_wait(id);

    hal_sys_timer_delay(20);

    // quad reset
    norflaship_clear_txfifo(id);
    norflaship_cmd_addr(id, EN25S80B_CMD_QUAD_RESET, 0);
    norflaship_busy_wait(id);

    hal_sys_timer_delay(20);

    // ip spi mode
    norflaship_quad_mode(id, 0);
    norflaship_hold_pin(id, 0);
    norflaship_wpr_pin(id, 0);
    norflaship_busy_wait(id);

    hal_sys_timer_delay(20);

    // reset enable
    norflaship_clear_txfifo(id);
    norflaship_cmd_addr(id, EN25S80B_CMD_SPI_RESET_ENABLE, 0);
    norflaship_busy_wait(id);

    hal_sys_timer_delay(20);

    // reset
    norflaship_clear_txfifo(id);
    norflaship_cmd_addr(id, EN25S80B_CMD_SPI_RESET, 0);
    norflaship_busy_wait(id);

    hal_sys_timer_delay(20);
}

static void en25s80b_enter_OTP(enum HAL_FLASH_ID_T id)
{
    norflaship_clear_txfifo(id);
    norflaship_cmd_addr(id, EN25S80B_CMD_ENTER_OTP, 0);
    norflash_status_WIP_1_wait(id, 0);
}

static void en25s80b_exit_OTP(enum HAL_FLASH_ID_T id)
{
    norflaship_clear_txfifo(id);
    norflaship_cmd_addr(id, EN25S80B_CMD_EXIT_OTP, 0);
    norflash_status_WIP_1_wait(id, 0);
}

static void en25s80b_write_status_s0_s7(enum HAL_FLASH_ID_T id, enum DRV_NORFLASH_W_STATUS_T type, uint8_t status)
{
    bool volatile_reg = true;

    if (type == DRV_NORFLASH_W_STATUS_LB || type == DRV_NORFLASH_W_STATUS_BP_INIT) {
        volatile_reg = false;
    }
    norflash_write_reg(id, volatile_reg, EN25S80B_CMD_WRITE_STATUS, &status, 1);
}

static int en25s80b_write_status(enum HAL_FLASH_ID_T id, enum DRV_NORFLASH_W_STATUS_T type, uint32_t param)
{
    uint8_t status;

    if (type == DRV_NORFLASH_W_STATUS_QE) {
        en25s80b_enter_OTP(id);

        status = norflash_read_status_s0_s7(id);

        if (param) {
            status |= (EN25S80B_WHDIS_BIT_MASK);
        } else {
            status &= ~(EN25S80B_WHDIS_BIT_MASK);
        }

        en25s80b_write_status_s0_s7(id, type, status);

        en25s80b_exit_OTP(id);

        return 0;
    }

    return 1;
}

const struct NORFLASH_CFG_T en25s80b_cfg = {
    .id = { 0x1C, 0x38, 0x14, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_6_EIGHTH,
            .others = SPEED_RATIO_6_EIGHTH,
        },
    },
    .crm_en_bits = 0xA5,
    .crm_dis_bits = 0xAA,
    .sec_reg_cfg = {
        .s = {
            .enabled = false,
        },
    },
    .total_size = EN25S80B_TOTAL_SIZE,
    .max_speed = 104 * 1000 * 1000,
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                HAL_NORFLASH_OP_MODE_QUAD_IO |
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_ERASE_IN_STD),
    .write_status = en25s80b_write_status,
};

