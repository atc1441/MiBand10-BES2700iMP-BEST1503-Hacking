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
#ifndef __HAL_NORFLASHIP_H__
#define __HAL_NORFLASHIP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"
#include "hal_cmu.h"

#define NORFLASH_PAGE_SIZE                      0x100
#define NORFLASH_PAGE_SIZE_1K                   0x400
#define NORFLASH_SECTOR_SIZE                    0x1000
#define NORFLASH_BLOCK_SIZE_32K                 0x8000
#define NORFLASH_BLOCK_SIZE_64K                 0x10000

#if (CHIP_FLASH_CTRL_VER <= 1)
#define NORFLASHIP_RXFIFO_SIZE                  8
#define NORFLASHIP_TXFIFO_SIZE                  256
#else // CHIP_FLASH_CTRL_VER >= 2
#if (CHIP_FLASH_CTRL_VER <= 6)
#define NORFLASHIP_RXFIFO_SIZE                  8
#define NORFLASHIP_TXFIFO_SIZE                  16
#else
#define NORFLASHIP_RXFIFO_SIZE                  16
#define NORFLASHIP_TXFIFO_SIZE                  16
#endif

#define NORFLASHIP_MAX_RCLK_DLY_VAL             16

#if !defined(CHIP_BEST2300)
#define NORFLASHIP_HAS_SECURITY
#endif

#if (defined(CHIP_BEST2300) || defined(CHIP_BEST1400))
#define NORFLASHIP_REMAP_NUM                    0
#else
#define NORFLASHIP_HAS_IDLE_IO_CTRL
#define NORFLASHIP_HAS_REMAP
#define NORFLASHIP_REMAP_NUM                    4
#endif
#endif

uint8_t norflaship_continuous_read_mode_bit(enum HAL_FLASH_ID_T id, uint8_t mode_bit);

uint8_t norflaship_continuous_read_off(enum HAL_FLASH_ID_T id);

uint8_t norflaship_continuous_read_on(enum HAL_FLASH_ID_T id);

void norflaship_write_txfifo(enum HAL_FLASH_ID_T id, const uint8_t *val, uint32_t len);

#if (CHIP_FLASH_CTRL_VER <= 1)
uint32_t norflaship_v1_write_txfifo_safe(const uint8_t *val, uint32_t len);

uint32_t norflaship_v1_write_txfifo_all(const uint8_t *val, uint32_t len);
#else
uint32_t norflaship_write_txfifo_start(enum HAL_FLASH_ID_T id, const uint8_t *val, uint32_t len);

uint32_t norflaship_write_txfifo_cont(enum HAL_FLASH_ID_T id, const uint8_t *val, uint32_t len);
#endif

#if (CHIP_FLASH_CTRL_VER >= 3) && defined(UNALIGNED_ACCESS)
uint32_t norflaship_write4_txfifo(enum HAL_FLASH_ID_T id, const uint8_t *val, uint32_t len);

uint32_t norflaship_write4_txfifo_cont(enum HAL_FLASH_ID_T id, const uint8_t *val, uint32_t len);
#endif

uint8_t norflaship_read_rxfifo_count(enum HAL_FLASH_ID_T id);

uint8_t norflaship_read_rxfifo(enum HAL_FLASH_ID_T id);

void norflaship_blksize(enum HAL_FLASH_ID_T id, uint32_t blksize);

void norflaship_cmd_addr(enum HAL_FLASH_ID_T id, uint8_t cmd, uint32_t address);

void norflaship_ext_tx_cmd(enum HAL_FLASH_ID_T id, uint8_t cmd, uint32_t tx_len);

void norflaship_ext_tx_cmd_4x(enum HAL_FLASH_ID_T id, uint8_t cmd, uint32_t tx_len);

void norflaship_ext_rx_cmd(enum HAL_FLASH_ID_T id, uint8_t cmd, uint32_t tx_len, uint32_t rx_len);

void norflaship_ext_rx_cmd_4x(enum HAL_FLASH_ID_T id, uint8_t cmd, uint32_t tx_len, uint32_t rx_len);

void norflaship_ext_rx_cmd_rx_4x(enum HAL_FLASH_ID_T id, uint8_t cmd, uint32_t tx_len, uint32_t rx_len);

void norflaship_cmd_done(enum HAL_FLASH_ID_T id);

void norflaship_rxfifo_count_wait(enum HAL_FLASH_ID_T id, uint8_t cnt);

void norflaship_rxfifo_empty_wait(enum HAL_FLASH_ID_T id);

void norflaship_busy_wait(enum HAL_FLASH_ID_T id);

int norflaship_is_busy(enum HAL_FLASH_ID_T id);

void norflaship_clear_fifos(enum HAL_FLASH_ID_T id);

void norflaship_clear_rxfifo(enum HAL_FLASH_ID_T id);

void norflaship_clear_txfifo(enum HAL_FLASH_ID_T id);

void norflaship_div(enum HAL_FLASH_ID_T id, uint32_t div);

uint32_t norflaship_get_div(enum HAL_FLASH_ID_T id);

uint32_t norflaship_get_pos_neg(enum HAL_FLASH_ID_T id);

void norflaship_pos_neg(enum HAL_FLASH_ID_T id, uint32_t v);

uint32_t norflaship_get_neg_phase(enum HAL_FLASH_ID_T id);

void norflaship_neg_phase(enum HAL_FLASH_ID_T id, uint32_t v);

uint32_t norflaship_get_samdly(enum HAL_FLASH_ID_T id);

void norflaship_samdly(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_dual_mode(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_hold_pin(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_wpr_pin(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_reset_pin(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_quad_mode(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_dummyclc(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_dummyclcen(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_4byteaddr_mode(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_ext_addr(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_nand_sel(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_dmactrl_tx_en(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_dmactrl_rx_en(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_dma_rx_threshold(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_dma_tx_threshold(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_dma_rx_size(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_ruen(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_rden(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_dualiocmd(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_rdcmd(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_frdcmd(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_qrdcmd(enum HAL_FLASH_ID_T id, uint32_t v);

uint32_t norflaship_get_rdcmd(enum HAL_FLASH_ID_T id);

void norflaship_set_idle_io_dir(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_sleep(enum HAL_FLASH_ID_T id);

void norflaship_wakeup(enum HAL_FLASH_ID_T id);

void norflaship_dec_index(enum HAL_FLASH_ID_T id, uint32_t idx);

void norflaship_dec_saddr(enum HAL_FLASH_ID_T id, uint32_t addr);

void norflaship_dec_eaddr(enum HAL_FLASH_ID_T id, uint32_t addr);

void norflaship_dec_enable(enum HAL_FLASH_ID_T id);

void norflaship_dec_disable(enum HAL_FLASH_ID_T id);

void norflaship_man_wrap_width(enum HAL_FLASH_ID_T id, uint32_t width);

void norflaship_man_wrap_enable(enum HAL_FLASH_ID_T id);

void norflaship_man_wrap_disable(enum HAL_FLASH_ID_T id);

void norflaship_auto_wrap_cmd(enum HAL_FLASH_ID_T id, uint32_t cmd);

void norflaship_man_mode_enable(enum HAL_FLASH_ID_T id);

void norflaship_man_mode_disable(enum HAL_FLASH_ID_T id);

void norflaship_reset_remap(enum HAL_FLASH_ID_T id);

int norflaship_config_remap_section(enum HAL_FLASH_ID_T id, uint32_t index, uint32_t addr, uint32_t len, uint32_t new_addr);

void norflaship_enable_remap(enum HAL_FLASH_ID_T id, uint32_t index);

void norflaship_disable_remap(enum HAL_FLASH_ID_T id, uint32_t index);

int norflaship_get_remap_status(enum HAL_FLASH_ID_T id, uint32_t index);

void norflaship_boot_reset_remap(enum HAL_FLASH_ID_T id);

int norflaship_boot_config_remap_section(enum HAL_FLASH_ID_T id, uint32_t index, uint32_t addr, uint32_t len, uint32_t new_addr);

void norflaship_boot_enable_remap(enum HAL_FLASH_ID_T id, uint32_t index);

void norflaship_boot_disable_remap(enum HAL_FLASH_ID_T id, uint32_t index);

int norflaship_boot_get_remap_status(enum HAL_FLASH_ID_T id, uint32_t index);

void norflaship_qpi_mode(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_dtr_mode(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_dummy_qior(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_dummy_dtr4rd(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_dummy_others(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_dummy_n_rcqio(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_nand_addr_byte_num(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_fetch_disable(enum HAL_FLASH_ID_T id);

void norflaship_cacheable_enable(enum HAL_FLASH_ID_T id);

uint32_t norflaship_mem_read_bus_lock(enum HAL_FLASH_ID_T id);

void norflaship_mem_read_bus_unlock(enum HAL_FLASH_ID_T id, uint32_t status);

int norflaship_mem_read_pending(enum HAL_FLASH_ID_T id);

void norflaship_cs_switch(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_cs0_cap_size(enum HAL_FLASH_ID_T id, uint32_t size);

void norflaship_cs0_manual_sel_en(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_cs1_manual_sel_en(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_cs_manual(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_tx_data_copy(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_rx_data_reorder(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_dual_chip_mode(enum HAL_FLASH_ID_T id, uint32_t v);

int norflaship_get_dual_chip_mode(enum HAL_FLASH_ID_T id);

void norflaship_cfg_rx_fifo_width(enum HAL_FLASH_ID_T id, uint32_t size);

void norflaship_cs1_reset_pin(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_cs1_wpr_pin(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_cs1_hold_pin(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_cs1_div(enum HAL_FLASH_ID_T id, uint32_t div);

uint32_t norflaship_cs1_get_div(enum HAL_FLASH_ID_T id);

void norflaship_boundary_split(enum HAL_FLASH_ID_T id, uint32_t v);

uint32_t norflaship_cs1_get_samdly(enum HAL_FLASH_ID_T id);

void norflaship_cs1_samdly(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_cs1_4byteaddr_mode(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_fetch_64_bytes(enum HAL_FLASH_ID_T id, uint32_t v);

void norflaship_write_lock(enum HAL_FLASH_ID_T id);

void norflaship_write_unlock(enum HAL_FLASH_ID_T id);

void norflaship_set_transsize_control(enum HAL_FLASH_ID_T id, bool use_tx_blksize);

void norflaship_set_transcmp(enum HAL_FLASH_ID_T id, bool completed);

void norflaship_clock_gating_enable(enum HAL_FLASH_ID_T id);

void norflaship_clock_gating_disable(enum HAL_FLASH_ID_T id);

void norflaship_set_rclk_dly(enum HAL_FLASH_ID_T id, uint32_t map, uint32_t delay);

uint32_t norflaship_get_rclk_dly(enum HAL_FLASH_ID_T id, uint32_t map);

void norflaship_rclk_dly_enable(enum HAL_FLASH_ID_T id, uint32_t map);

void norflaship_rclk_dly_disable(enum HAL_FLASH_ID_T id, uint32_t map);

uint32_t norflaship_get_rclk_dly_status(enum HAL_FLASH_ID_T id);

void norflaship_set_rxfifo_ctrl(enum HAL_FLASH_ID_T id, uint32_t val);

#ifdef __cplusplus
}
#endif

#endif
