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
#ifndef NORFLASH_HAL_H
#define NORFLASH_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"
#include "plat_addr_map.h"
#include "hal_cmu.h"

// 64M Bytes
#ifndef HAL_NORFLASH_ADDR_MASK
#define HAL_NORFLASH_ADDR_MASK              0x03FFFFFF
#endif

#define HAL_NORFLASH_DEVICE_ID_LEN          3

#define HAL_NORFLASH_CP_ID_LEN              2
#define HAL_NORFLASH_UNIQUE_ID_LEN          16

#ifdef FLASH_DUAL_CHIP
#define FLASH_IS_DUAL_CHIP(id)              (FLASH_DUAL_CHIP & (0x1 << id))
#define NORFLASH_4BYTE_ADDR_SIZE(id)        (FLASH_IS_DUAL_CHIP(id) ? (1 << 25) : (1 << 24))
#else
#define NORFLASH_4BYTE_ADDR_SIZE(id)        (1 << 24)
#endif

#define FLASH_SECTOR_SIZE_IN_BYTES          4096
#define FLASH_BLOCK_SIZE_IN_BYTES           (32*1024)

#define NORFLASH_PUYA_ID_PREFIX             0x85

#define NORFLASH_SK_ID_PREFIX               0x25

#define NORFLASH_XTS_ID_PREFIX              0x0B

#define NORFLASH_XMC_ID_PREFIX              0x20

#define NORFLASH_ZBIT_ID_PREFIX             0x5E

#define NORFLASH_WB_ID_PREFIX               0xEF

#define NORFLASH_GD_ID_PREFIX               0xC8

#define NORFLASH_EN_ID_PREFIX               0x1C

#define NORFLASH_XT_ID_PREFIX               0x0B

enum HAL_NORFLASH_RET_T {
    HAL_NORFLASH_OK                 = 0,
    HAL_NORFLASH_SUSPENDED          = 1,
    HAL_NORFLASH_ERR                = 2,
    HAL_NORFLASH_BAD_ID             = 3,
    HAL_NORFLASH_BAD_DIV            = 4,
    HAL_NORFLASH_BAD_CALIB_ID       = 5,
    HAL_NORFLASH_BAD_CALIB_MAGIC    = 6,
    HAL_NORFLASH_BAD_CALIB_SEQ      = 7,
    HAL_NORFLASH_BAD_CFG            = 8,
    HAL_NORFLASH_BAD_MODE           = 9,
    HAL_NORFLASH_BAD_OP             = 10,
    HAL_NORFLASH_BAD_ADDR           = 11,
    HAL_NORFLASH_BAD_LEN            = 12,
    HAL_NORFLASH_BAD_REMAP_ID       = 13,
    HAL_NORFLASH_BAD_REMAP_OFFSET   = 14,
    HAL_NORFLASH_BAD_ERASE_TYPE     = 15,
    HAL_NORFLASH_BAD_NS_ADDR        = 16,
    HAL_NORFLASH_BAD_PRIVILEGE      = 17,
    HAL_NORFLASH_NOT_OPENED         = 18,
    HAL_NORFLASH_CFG_NULL           = 19,
    HAL_NORFLASH_TIMING_NULL        = 20,
    HAL_NORFLASH_BUFFER_NULL        = 21,
};

enum HAL_NORFLASH_SPEED {
    HAL_NORFLASH_SPEED_13M  = 13000000,
    HAL_NORFLASH_SPEED_26M  = 26000000,
    HAL_NORFLASH_SPEED_52M  = 52000000,
    HAL_NORFLASH_SPEED_78M  = 78000000,
    HAL_NORFLASH_SPEED_104M = 104000000,
    HAL_NORFLASH_SPEED_156M = 156000000,
    HAL_NORFLASH_SPEED_208M = 208000000,
};

enum HAL_NORFLASH_REMAP_ID_T {
    HAL_NORFLASH_REMAP_ID_0,
    HAL_NORFLASH_REMAP_ID_1,
    HAL_NORFLASH_REMAP_ID_2,
    HAL_NORFLASH_REMAP_ID_3,

    HAL_NORFLASH_REMAP_ID_QTY,
};

enum HAL_NORFLASH_OP_MODE {
    // Different groups can be used together, different flash-device may support different option(s)

    // (1) basic read mode
    // standard spi mode
    HAL_NORFLASH_OP_MODE_STAND_SPI              = (1 << 0),
    // fast spi mode
    HAL_NORFLASH_OP_MODE_FAST_SPI               = (1 << 1),
    // dual mode
    HAL_NORFLASH_OP_MODE_DUAL_OUTPUT            = (1 << 2),
    // dual mode
    HAL_NORFLASH_OP_MODE_DUAL_IO                = (1 << 3),
    // quad mode
    HAL_NORFLASH_OP_MODE_QUAD_OUTPUT            = (1 << 4),
    // quad mode
    HAL_NORFLASH_OP_MODE_QUAD_IO                = (1 << 5),
    // dtr mode
    HAL_NORFLASH_OP_MODE_DTR                    = (1 << 6),

    // (2) extend read mode
    // read accelerate (no cmd bettween read operation) :
    // may need Dual or Quad Mode
    HAL_NORFLASH_OP_MODE_CONTINUOUS_READ        = (1 << 12),
    // read high performance mode
    HAL_NORFLASH_OP_MODE_HIGH_PERFORMANCE       = (1 << 13),
    // read wrap mode
    HAL_NORFLASH_OP_MODE_READ_WRAP              = (1 << 14),

    // (3) program mode.
    // page program mode
    HAL_NORFLASH_OP_MODE_PAGE_PROGRAM           = (1 << 16),
    // dual program mode
    HAL_NORFLASH_OP_MODE_DUAL_PAGE_PROGRAM      = (1 << 17),
    // quad program mode
    HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM      = (1 << 18),
    // erase in standard spi mode
    HAL_NORFLASH_OP_MODE_ERASE_IN_STD           = (1 << 19),
    // page size 1K
    HAL_NORFLASH_OP_MODE_PAGE_SIZE_1K           = (1 << 20),

    // (4) advanced features
    // suspend and resume
    HAL_NORFLASH_OP_MODE_SUSPEND                = (1 << 24),

    // (5) others
    // check sfdp mode
    HAL_NORFLASH_OP_MODE_CHECK_SFDP             = (1 << 28),
    // Quad id dummy 8
    HAL_NORFLASH_OP_MODE_QUAD_IO_DUMMY_8        = (1 << 29),

    HAL_NORFLASH_OP_MODE_RESERVED               = ~0UL,
};

enum HAL_NORFLASH_OPT_T {
    HAL_NORFLASH_OPT_CALIB_FLASH_ID             = (1 << 0),
    HAL_NORFLASH_OPT_CALIB_MAGIC_WORD           = (1 << 1),
    HAL_NORFLASH_OPT_CALIB_SEQ_PATTERN          = (1 << 2),
    HAL_NORFLASH_OPT_DUAL_CHIP                  = (1 << 3),
    HAL_NORFLASH_OPT_SEC_REG_DUAL_CHIP          = (1 << 4),

    HAL_NORFLASH_OPT_RESERVED                   = ~0UL,
};

struct HAL_NORFLASH_CONFIG_T {
    uint32_t source_clk;
    uint32_t speed;
    enum HAL_NORFLASH_OP_MODE mode;
    enum HAL_NORFLASH_OPT_T opt;
};

struct HAL_NORFLASH_TIMING_CONFIG_T {
    uint8_t timing_idx;
    uint8_t delay_map;
    uint8_t delay_val;
};

#ifdef ARM_CMNS

//#define hal_norflash_get_init_config hal_norflash_get_init_config_se
#define hal_norflash_get_init_config_ex hal_norflash_get_init_config_ex_se
#define hal_norflash_init hal_norflash_init_se
#define hal_norflash_open hal_norflash_open_se
#define hal_norflash_reopen hal_norflash_reopen_se
#define hal_norflash_apply_config hal_norflash_apply_config_se
#define hal_norflash_get_timing_config hal_norflash_get_timing_config_se
#define hal_norflash_get_id hal_norflash_get_id_se
#define hal_norflash_get_flash_total_size hal_norflash_get_flash_total_size_se
#define hal_norflash_get_size hal_norflash_get_size_se
#define hal_norflash_get_boundary hal_norflash_get_boundary_se
#define hal_norflash_show_calib_result hal_norflash_show_calib_result_se
#define hal_norflash_show_sfdp_check_param hal_norflash_show_sfdp_check_param_se
#define hal_norflash_erase hal_norflash_erase_se
#define hal_norflash_write hal_norflash_write_se
#define hal_norflash_read hal_norflash_read_se
#define hal_norflash_force_wakeup hal_norflash_force_wakeup_se
#define hal_norflash_sleep hal_norflash_sleep_se
#define hal_norflash_wakeup hal_norflash_wakeup_se
#define hal_norflash_opened hal_norflash_opened_se
#define hal_norflash_get_open_state hal_norflash_get_open_state_se
#define hal_norflash_write_resume hal_norflash_write_resume_se
#define hal_norflash_erase_resume hal_norflash_erase_resume_se
#define hal_norflash_write_suspend hal_norflash_write_suspend_se
#define hal_norflash_erase_suspend hal_norflash_erase_suspend_se
#define hal_norflash_suspend_check_irq hal_norflash_suspend_check_irq_se
#define hal_norflash_suspend_check_flash_read hal_norflash_suspend_check_flash_read_se
#define hal_norflash_enable_remap hal_norflash_enable_remap_se
#define hal_norflash_disable_remap hal_norflash_disable_remap_se
#define hal_norflash_config_remap hal_norflash_config_remap_se
#define hal_norflash_get_remap_status hal_norflash_get_remap_status_se
#define hal_norflash_get_unique_id hal_norflash_get_unique_id_se
#define hal_norflash_read_sfdp_reg hal_norflash_read_sfdp_reg_se
#define hal_norflash_dual_chip_read_sfdp_reg hal_norflash_dual_chip_read_sfdp_reg_se
#define hal_norflash_enable_protection hal_norflash_enable_protection_se
#define hal_norflash_disable_protection hal_norflash_disable_protection_se
#define hal_norflash_set_protection hal_norflash_set_protection_se
#define hal_norflash_security_register_lock hal_norflash_security_register_lock_se
#define hal_norflash_security_register_read hal_norflash_security_register_read_se
#define hal_norflash_set_dual_chip_mode hal_norflash_set_dual_chip_mode_se
#define hal_norflash_get_dual_chip_mode hal_norflash_get_dual_chip_mode_se
#define hal_norflash_show_id_state hal_norflash_show_id_state_se
#define hal_norflash_addr_to_id hal_norflash_addr_to_id_se
#define hal_norflash_get_addr_offset hal_norflash_get_addr_offset_se
#define hal_norflash_dec_enable hal_norflash_dec_enable_sec
#define hal_norflash_dec_disable hal_norflash_dec_disable_sec

#else /* !ARM_CMNS */

void hal_norflash_set_boot_freq(enum HAL_CMU_FREQ_T freq);
const struct HAL_NORFLASH_CONFIG_T *hal_norflash_get_init_config(void);
enum HAL_NORFLASH_RET_T hal_norflash_get_init_config_ex(struct HAL_NORFLASH_CONFIG_T *cfg);
enum HAL_NORFLASH_RET_T hal_norflash_init(enum HAL_FLASH_ID_T id);
enum HAL_NORFLASH_RET_T hal_norflash_open(enum HAL_FLASH_ID_T id, const struct HAL_NORFLASH_CONFIG_T *cfg);
enum HAL_NORFLASH_RET_T hal_norflash_reopen(enum HAL_FLASH_ID_T id, const struct HAL_NORFLASH_CONFIG_T *cfg);
enum HAL_NORFLASH_RET_T hal_norflash_apply_config(enum HAL_FLASH_ID_T id, const struct HAL_NORFLASH_CONFIG_T *cfg, const struct HAL_NORFLASH_TIMING_CONFIG_T *timing);
enum HAL_NORFLASH_RET_T hal_norflash_get_timing_config(enum HAL_FLASH_ID_T id, struct HAL_NORFLASH_TIMING_CONFIG_T *timing);
void hal_norflash_show_calib_result(enum HAL_FLASH_ID_T id);
void hal_norflash_show_sfdp_check_param(enum HAL_FLASH_ID_T id);
enum HAL_CMU_FREQ_T hal_norflash_clk_to_cmu_freq(uint32_t clk);
enum HAL_NORFLASH_RET_T hal_norflash_get_size(enum HAL_FLASH_ID_T id, uint32_t *total_size, uint32_t *block_size, uint32_t *sector_size, uint32_t *page_size);
enum HAL_NORFLASH_RET_T hal_norflash_get_boundary(enum HAL_FLASH_ID_T id, uint32_t address, uint32_t* block_boundary, uint32_t* sector_boundary);
enum HAL_NORFLASH_RET_T hal_norflash_get_id(enum HAL_FLASH_ID_T id, uint8_t *value, uint32_t len);
enum HAL_NORFLASH_RET_T hal_norflash_dual_chip_get_id(enum HAL_FLASH_ID_T id, uint8_t *value0, uint8_t *value1, uint32_t len);
enum HAL_NORFLASH_RET_T hal_norflash_get_unique_id(enum HAL_FLASH_ID_T id, uint8_t *value, uint32_t len);
enum HAL_NORFLASH_RET_T hal_norflash_dual_chip_get_unique_id(enum HAL_FLASH_ID_T id, uint8_t *value0, uint8_t *value1, uint32_t len);
enum HAL_NORFLASH_RET_T hal_norflash_read_sfdp_reg(enum HAL_FLASH_ID_T id, uint8_t addr, uint8_t *value, uint32_t len);
enum HAL_NORFLASH_RET_T hal_norflash_dual_chip_read_sfdp_reg(enum HAL_FLASH_ID_T id, uint8_t addr, uint8_t *value0, uint8_t *value1, uint32_t len);
enum HAL_NORFLASH_RET_T hal_norflash_enable_protection(enum HAL_FLASH_ID_T id);
enum HAL_NORFLASH_RET_T hal_norflash_disable_protection(enum HAL_FLASH_ID_T id);
enum HAL_NORFLASH_RET_T hal_norflash_set_protection(enum HAL_FLASH_ID_T id, uint32_t bp);
enum HAL_NORFLASH_RET_T hal_norflash_erase_chip(enum HAL_FLASH_ID_T id);
enum HAL_NORFLASH_RET_T hal_norflash_erase_suspend(enum HAL_FLASH_ID_T id, uint32_t start_address, uint32_t len, int suspend);
enum HAL_NORFLASH_RET_T hal_norflash_erase(enum HAL_FLASH_ID_T id, uint32_t start_address, uint32_t len);
enum HAL_NORFLASH_RET_T hal_norflash_erase_resume(enum HAL_FLASH_ID_T id, int suspend);
enum HAL_NORFLASH_RET_T hal_norflash_write_suspend(enum HAL_FLASH_ID_T id, uint32_t start_address, const uint8_t *buffer, uint32_t len, int suspend);
enum HAL_NORFLASH_RET_T hal_norflash_write(enum HAL_FLASH_ID_T id, uint32_t start_address, const uint8_t *buffer, uint32_t len);
enum HAL_NORFLASH_RET_T hal_norflash_write_resume(enum HAL_FLASH_ID_T id, int suspend);
enum HAL_NORFLASH_RET_T hal_norflash_suspend_check_irq(enum HAL_FLASH_ID_T id, uint32_t irq_num, int enable);
enum HAL_NORFLASH_RET_T hal_norflash_suspend_check_flash_read(enum HAL_FLASH_ID_T id, int enable);
enum HAL_NORFLASH_RET_T hal_norflash_read(enum HAL_FLASH_ID_T id, uint32_t start_address, uint8_t *buffer, uint32_t len);
enum HAL_NORFLASH_RET_T hal_norflash_close(enum HAL_FLASH_ID_T id);
void hal_norflash_force_sleep(enum HAL_FLASH_ID_T id);
void hal_norflash_force_wakeup(enum HAL_FLASH_ID_T id);
void hal_norflash_sleep(enum HAL_FLASH_ID_T id);
void hal_norflash_wakeup(enum HAL_FLASH_ID_T id);
int hal_norflash_busy(enum HAL_FLASH_ID_T id);
uint32_t hal_norflash_mem_read_bus_lock(enum HAL_FLASH_ID_T id);
void hal_norflash_mem_read_bus_unlock(enum HAL_FLASH_ID_T id, uint32_t status);
enum HAL_NORFLASH_RET_T hal_norflash_set_dual_chip_mode(enum HAL_FLASH_ID_T id, int dual_flash, int dual_sec_reg);
enum HAL_NORFLASH_RET_T hal_norflash_get_dual_chip_mode(enum HAL_FLASH_ID_T id, int *dual_flash, int *dual_sec_reg);
uint32_t hal_norflash_get_flash_total_size(enum HAL_FLASH_ID_T id);
int hal_norflash_opened(enum HAL_FLASH_ID_T id);
enum HAL_NORFLASH_RET_T hal_norflash_get_open_state(enum HAL_FLASH_ID_T id);
int hal_norflash_security_register_is_locked(enum HAL_FLASH_ID_T id, uint32_t start_address, uint32_t len);
enum HAL_NORFLASH_RET_T hal_norflash_security_register_lock(enum HAL_FLASH_ID_T id, uint32_t start_address, uint32_t len);
enum HAL_NORFLASH_RET_T hal_norflash_security_register_erase(enum HAL_FLASH_ID_T id, uint32_t start_address, uint32_t len);
enum HAL_NORFLASH_RET_T hal_norflash_security_register_write(enum HAL_FLASH_ID_T id, uint32_t start_address, const uint8_t *buffer, uint32_t len);
enum HAL_NORFLASH_RET_T hal_norflash_security_register_read(enum HAL_FLASH_ID_T id, uint32_t start_address, uint8_t *buffer, uint32_t len);
void hal_norflash_reset_remap(enum HAL_FLASH_ID_T id);
enum HAL_NORFLASH_RET_T hal_norflash_config_remap(enum HAL_FLASH_ID_T id, enum HAL_NORFLASH_REMAP_ID_T remap_id, uint32_t addr, uint32_t len, uint32_t offset);
enum HAL_NORFLASH_RET_T hal_norflash_enable_remap(enum HAL_FLASH_ID_T id, enum HAL_NORFLASH_REMAP_ID_T remap_id);
enum HAL_NORFLASH_RET_T hal_norflash_disable_remap(enum HAL_FLASH_ID_T id, enum HAL_NORFLASH_REMAP_ID_T remap_id);
int hal_norflash_get_remap_status(enum HAL_FLASH_ID_T id, enum HAL_NORFLASH_REMAP_ID_T remap_id);
void hal_norflash_boot_reset_remap(enum HAL_FLASH_ID_T id);
enum HAL_NORFLASH_RET_T hal_norflash_boot_config_remap(enum HAL_FLASH_ID_T id, enum HAL_NORFLASH_REMAP_ID_T remap_id, uint32_t addr, uint32_t len, uint32_t offset);
enum HAL_NORFLASH_RET_T hal_norflash_boot_disable_remap(enum HAL_FLASH_ID_T id, enum HAL_NORFLASH_REMAP_ID_T remap_id);
enum HAL_NORFLASH_RET_T hal_norflash_boot_enable_remap(enum HAL_FLASH_ID_T id, enum HAL_NORFLASH_REMAP_ID_T remap_id);
int hal_norflash_boot_get_remap_status(enum HAL_FLASH_ID_T id, enum HAL_NORFLASH_REMAP_ID_T remap_id);
enum HAL_FLASH_ID_T hal_norflash_addr_to_id(uint32_t addr);
uint32_t hal_norflash_get_addr_offset(uint32_t addr);
void hal_norflash_show_id_state(enum HAL_FLASH_ID_T id, int assert_on_error);

#endif /* !ARM_CMNS */

#if defined(ARM_CMSE) || defined(ARM_CMNS)

//const struct HAL_NORFLASH_CONFIG_T *hal_norflash_get_init_config_se(void);
enum HAL_NORFLASH_RET_T hal_norflash_get_init_config_ex_se(struct HAL_NORFLASH_CONFIG_T *cfg);
enum HAL_NORFLASH_RET_T hal_norflash_init_se(enum HAL_FLASH_ID_T id);
enum HAL_NORFLASH_RET_T hal_norflash_open_se(enum HAL_FLASH_ID_T id, const struct HAL_NORFLASH_CONFIG_T *cfg);
enum HAL_NORFLASH_RET_T hal_norflash_reopen_se(enum HAL_FLASH_ID_T id, const struct HAL_NORFLASH_CONFIG_T *cfg);
enum HAL_NORFLASH_RET_T hal_norflash_apply_config_se(enum HAL_FLASH_ID_T id, const struct HAL_NORFLASH_CONFIG_T *cfg, const struct HAL_NORFLASH_TIMING_CONFIG_T *timing);
enum HAL_NORFLASH_RET_T hal_norflash_get_timing_config_se(enum HAL_FLASH_ID_T id, struct HAL_NORFLASH_TIMING_CONFIG_T *timing);
enum HAL_NORFLASH_RET_T hal_norflash_get_id_se(enum HAL_FLASH_ID_T id, uint8_t *value, uint32_t len);
uint32_t hal_norflash_get_flash_total_size_se(enum HAL_FLASH_ID_T id);
// 'cmse_nonsecure_entry' functions only allow 4 arguments
enum HAL_NORFLASH_RET_T hal_norflash_get_size_se0(uint32_t *total_size, uint32_t *block_size, uint32_t *sector_size, uint32_t *page_size);
enum HAL_NORFLASH_RET_T hal_norflash_get_size_se1(uint32_t *total_size, uint32_t *block_size, uint32_t *sector_size, uint32_t *page_size);
enum HAL_NORFLASH_RET_T hal_norflash_get_size_se2(uint32_t *total_size, uint32_t *block_size, uint32_t *sector_size, uint32_t *page_size);
static inline enum HAL_NORFLASH_RET_T hal_norflash_get_size_se(enum HAL_FLASH_ID_T id, uint32_t *total_size,
    uint32_t *block_size, uint32_t *sector_size, uint32_t *page_size)
{
    if (id == HAL_FLASH_ID_0)
        return hal_norflash_get_size_se0(total_size, block_size, sector_size, page_size);
#ifdef FLASH1_CTRL_BASE
    else if (id == HAL_FLASH_ID_1)
        return hal_norflash_get_size_se1(total_size, block_size, sector_size, page_size);
#endif
#ifdef FLASH2_CTRL_BASE
    else if (id == HAL_FLASH_ID_2)
        return hal_norflash_get_size_se2(total_size, block_size, sector_size, page_size);
#endif
    else
        return HAL_NORFLASH_BAD_ID;
}
enum HAL_NORFLASH_RET_T hal_norflash_get_boundary_se(enum HAL_FLASH_ID_T id, uint32_t address, uint32_t* block_boundary, uint32_t* sector_boundary);
void hal_norflash_show_calib_result_se(enum HAL_FLASH_ID_T id);
enum HAL_NORFLASH_RET_T hal_norflash_erase_se(enum HAL_FLASH_ID_T id, uint32_t start_address, uint32_t len);
enum HAL_NORFLASH_RET_T hal_norflash_write_se(enum HAL_FLASH_ID_T id, uint32_t start_address, const uint8_t *buffer, uint32_t len);
enum HAL_NORFLASH_RET_T hal_norflash_read_se(enum HAL_FLASH_ID_T id, uint32_t start_address, uint8_t *buffer, uint32_t len);
void hal_norflash_sleep_se(enum HAL_FLASH_ID_T id);
void hal_norflash_wakeup_se(enum HAL_FLASH_ID_T id);
int hal_norflash_opened_se(enum HAL_FLASH_ID_T id);
enum HAL_NORFLASH_RET_T hal_norflash_get_open_state_se(enum HAL_FLASH_ID_T id);
enum HAL_NORFLASH_RET_T hal_norflash_write_resume_se(enum HAL_FLASH_ID_T id, int suspend);
enum HAL_NORFLASH_RET_T hal_norflash_erase_resume_se(enum HAL_FLASH_ID_T id, int suspend);
enum HAL_NORFLASH_RET_T hal_norflash_suspend_check_irq_se(enum HAL_FLASH_ID_T id, uint32_t irq_num, int enable);
enum HAL_NORFLASH_RET_T hal_norflash_suspend_check_flash_read_se(enum HAL_FLASH_ID_T id, int enable);
enum HAL_NORFLASH_RET_T hal_norflash_enable_remap_se(enum HAL_FLASH_ID_T id, enum HAL_NORFLASH_REMAP_ID_T remap_id);
enum HAL_NORFLASH_RET_T hal_norflash_disable_remap_se(enum HAL_FLASH_ID_T id, enum HAL_NORFLASH_REMAP_ID_T remap_id);
// 'cmse_nonsecure_entry' functions only allow 4 arguments
enum HAL_NORFLASH_RET_T hal_norflash_config_remap_se0(enum HAL_NORFLASH_REMAP_ID_T remap_id, uint32_t addr, uint32_t len, uint32_t offset);
enum HAL_NORFLASH_RET_T hal_norflash_config_remap_se1(enum HAL_NORFLASH_REMAP_ID_T remap_id, uint32_t addr, uint32_t len, uint32_t offset);
enum HAL_NORFLASH_RET_T hal_norflash_config_remap_se2(enum HAL_NORFLASH_REMAP_ID_T remap_id, uint32_t addr, uint32_t len, uint32_t offset);
static inline enum HAL_NORFLASH_RET_T hal_norflash_config_remap_se(enum HAL_FLASH_ID_T id, enum HAL_NORFLASH_REMAP_ID_T remap_id, uint32_t addr, uint32_t len, uint32_t offset)
{
    if (id == HAL_FLASH_ID_0)
        return hal_norflash_config_remap_se0(remap_id, addr, len, offset);
#ifdef FLASH1_CTRL_BASE
    else if (id == HAL_FLASH_ID_1)
        return hal_norflash_config_remap_se1(remap_id, addr, len, offset);
#endif
#ifdef FLASH2_CTRL_BASE
    else if (id == HAL_FLASH_ID_2)
        return hal_norflash_config_remap_se2(remap_id, addr, len, offset);
#endif
    else
        return HAL_NORFLASH_BAD_ID;
}
int hal_norflash_get_remap_status_se(enum HAL_FLASH_ID_T id, enum HAL_NORFLASH_REMAP_ID_T remap_id);
// 'cmse_nonsecure_entry' functions only allow 4 arguments
enum HAL_NORFLASH_RET_T hal_norflash_write_suspend_se0(uint32_t start_address, const uint8_t *buffer, uint32_t len, int suspend);
enum HAL_NORFLASH_RET_T hal_norflash_write_suspend_se1(uint32_t start_address, const uint8_t *buffer, uint32_t len, int suspend);
enum HAL_NORFLASH_RET_T hal_norflash_write_suspend_se2(uint32_t start_address, const uint8_t *buffer, uint32_t len, int suspend);
static inline enum HAL_NORFLASH_RET_T hal_norflash_write_suspend_se(enum HAL_FLASH_ID_T id, uint32_t start_address, const uint8_t *buffer, uint32_t len, int suspend)
{
    if (id == HAL_FLASH_ID_0)
        return hal_norflash_write_suspend_se0(start_address, buffer, len, suspend);
#ifdef FLASH1_CTRL_BASE
    else if (id == HAL_FLASH_ID_1)
        return hal_norflash_write_suspend_se1(start_address, buffer, len, suspend);
#endif
#ifdef FLASH2_CTRL_BASE
    else if (id == HAL_FLASH_ID_2)
        return hal_norflash_write_suspend_se2(start_address, buffer, len, suspend);
#endif
    else
        return HAL_NORFLASH_BAD_ID;
}
enum HAL_NORFLASH_RET_T hal_norflash_get_unique_id_se(enum HAL_FLASH_ID_T id, uint8_t *value, uint32_t len);
enum HAL_NORFLASH_RET_T hal_norflash_read_sfdp_reg_se(enum HAL_FLASH_ID_T id, uint8_t addr, uint8_t *value, uint32_t len);
// 'cmse_nonsecure_entry' functions only allow 4 arguments
enum HAL_NORFLASH_RET_T hal_norflash_dual_chip_read_sfdp_reg_se0(uint8_t addr, uint8_t *value0, uint8_t *value1, uint32_t len);
enum HAL_NORFLASH_RET_T hal_norflash_dual_chip_read_sfdp_reg_se1(uint8_t addr, uint8_t *value0, uint8_t *value1, uint32_t len);
enum HAL_NORFLASH_RET_T hal_norflash_dual_chip_read_sfdp_reg_se2(uint8_t addr, uint8_t *value0, uint8_t *value1, uint32_t len);
static inline enum HAL_NORFLASH_RET_T hal_norflash_dual_chip_read_sfdp_reg_se(enum HAL_FLASH_ID_T id, uint8_t addr, uint8_t *value0, uint8_t *value1, uint32_t len)
{
    if (id == HAL_FLASH_ID_0)
        return hal_norflash_dual_chip_read_sfdp_reg_se0(addr, value0, value1, len);
#ifdef FLASH1_CTRL_BASE
    else if (id == HAL_FLASH_ID_1)
        return hal_norflash_dual_chip_read_sfdp_reg_se1(addr, value0, value1, len);
#endif
#ifdef FLASH2_CTRL_BASE
    else if (id == HAL_FLASH_ID_2)
        return hal_norflash_dual_chip_read_sfdp_reg_se2(addr, value0, value1, len);
#endif
    else
        return HAL_NORFLASH_BAD_ID;
}
enum HAL_NORFLASH_RET_T hal_norflash_enable_protection_se(enum HAL_FLASH_ID_T id);
enum HAL_NORFLASH_RET_T hal_norflash_disable_protection_se(enum HAL_FLASH_ID_T id);
enum HAL_NORFLASH_RET_T hal_norflash_set_protection_se(enum HAL_FLASH_ID_T id, uint32_t bp);
enum HAL_NORFLASH_RET_T hal_norflash_security_register_lock_se(enum HAL_FLASH_ID_T id, uint32_t start_address, uint32_t len);
enum HAL_NORFLASH_RET_T hal_norflash_security_register_read_se(enum HAL_FLASH_ID_T id, uint32_t start_address, uint8_t *buffer, uint32_t len);
enum HAL_NORFLASH_RET_T hal_norflash_erase_suspend_se(enum HAL_FLASH_ID_T id, uint32_t start_address, uint32_t len, int suspend);
enum HAL_NORFLASH_RET_T hal_norflash_set_dual_chip_mode_se(enum HAL_FLASH_ID_T id, int dual_flash, int dual_sec_reg);
enum HAL_NORFLASH_RET_T hal_norflash_get_dual_chip_mode_se(enum HAL_FLASH_ID_T id, int *dual_flash, int *dual_sec_reg);
void hal_norflash_show_id_state_se(enum HAL_FLASH_ID_T id, int assert_on_error);
enum HAL_FLASH_ID_T hal_norflash_addr_to_id_se(uint32_t addr);
uint32_t hal_norflash_get_addr_offset_se(uint32_t addr);

#endif /* ARM_CMSE || ARM_CMNS */

#ifdef __cplusplus
}
#endif

#endif /* NORFLASH_HAL_H */
