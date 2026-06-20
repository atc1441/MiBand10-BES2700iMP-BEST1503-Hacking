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
#ifndef DRV_NORFLASH_H
#define DRV_NORFLASH_H

#include "plat_types.h"
#include "hal_norflash.h"
#include "hal_cmu.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef FLASH_LOCK_MEM_READ
#if (CHIP_FLASH_CTRL_VER <= 5)
#error "FLASH_LOCK_MEM_READ can be enabled only if CHIP_FLASH_CTRL_VER >= 6"
#elif !defined(PROGRAMMER)
#define FLASH_MEM_READ_BUS_LOCK
#endif
#endif

#if !(defined(ROM_BUILD) || (defined(PROGRAMMER) && !defined(PROGRAMMER_INFLASH)) || \
        defined(NANDFLASH_BUILD) || defined(SUBSYS_FLASH_BOOT))
#ifndef FLASH_CALIB_MAGIC_WORD
#define FLASH_CALIB_MAGIC_WORD
#endif
#ifndef FLASH_CALIB_SEQ_PATTERN
#define FLASH_CALIB_SEQ_PATTERN
#endif
#endif

#if !(defined(PROGRAMMER) && !defined(PROGRAMMER_INFLASH))
#if defined(FLASH_CALIB_MAGIC_WORD) && !defined(FLASH_INIT_CALIB_MAGIC_WORD)
#define FLASH_INIT_CALIB_MAGIC_WORD
#endif
#if defined(FLASH_CALIB_SEQ_PATTERN) && !defined(FLASH_INIT_CALIB_SEQ_PATTERN)
#define FLASH_INIT_CALIB_SEQ_PATTERN
#endif
#endif

// The following chips need to open macro NORFLASH_CHECK_SFDP_ENABLE:
// BEST2003, BEST1600, BEST1502X
#if defined(CHIP_BEST2003) || defined(CHIP_BEST1600) || defined(CHIP_BEST1502X)
#define NORFLASH_CHECK_SFDP_ENABLE
#endif

#define NORFLASH_ID_LEN                     3

#define NORFLASH_SFDP_MAX_CHECK_LEN         4

enum DRV_NORFLASH_ERASE_T {
    DRV_NORFLASH_ERASE_PAGE,
    DRV_NORFLASH_ERASE_SECTOR,
    DRV_NORFLASH_ERASE_BLOCK_32K,
    DRV_NORFLASH_ERASE_BLOCK_64K,
    DRV_NORFLASH_ERASE_CHIP,
};

enum DRV_NORFLASH_W_STATUS_T {
    DRV_NORFLASH_W_STATUS_INIT,
    DRV_NORFLASH_W_STATUS_QE,
    DRV_NORFLASH_W_STATUS_LB,
    DRV_NORFLASH_W_STATUS_BP_INIT,
    DRV_NORFLASH_W_STATUS_BP,

    DRV_NORFLASH_W_STATUS_QTY,
};

enum SPEED_RATIO_T {
    SPEED_RATIO_1_EIGHTH,
    SPEED_RATIO_2_EIGHTH,
    SPEED_RATIO_3_EIGHTH,
    SPEED_RATIO_4_EIGHTH,
    SPEED_RATIO_5_EIGHTH,
    SPEED_RATIO_6_EIGHTH,
    SPEED_RATIO_7_EIGHTH,
    SPEED_RATIO_8_EIGHTH,
};

enum DRV_NORFLASH_CALIB_T {
    DRV_NORFLASH_CALIB_FLASH_ID,
    DRV_NORFLASH_CALIB_MAGIC_WORD,
    DRV_NORFLASH_CALIB_SEQ_PATTERN,

    DRV_NORFLASH_CALIB_QTY,
};

union DRV_NORFLASH_SPEED_RATIO_T {
    struct {
        uint8_t std_read    :3;
        uint8_t others      :3;
        uint8_t reserved    :2;
    } s;
    uint8_t v;
};

union DRV_NORFLASH_SEC_REG_CFG_T {
    struct {
        uint16_t enabled    :1;
        uint16_t base       :2;
        uint16_t size       :2;
        uint16_t offset     :1;
        uint16_t cnt        :2;
        uint16_t pp         :2;
        uint16_t lb         :2;
        uint16_t reserved   :4;
    } s;
    uint16_t v;
};

union DRV_NORFLASH_DTR_CFG_T {
    struct {
        uint8_t speed_ratio :3;
        uint8_t dummy_cycles:5;
    } s;
    uint8_t v;
};

typedef int (*NORFLASH_WRITE_STATUS_CB)(enum HAL_FLASH_ID_T id, enum DRV_NORFLASH_W_STATUS_T type, uint32_t param);

struct NORFLASH_SFDP_CFG_T {
    uint8_t param[NORFLASH_SFDP_MAX_CHECK_LEN];
    union DRV_NORFLASH_SPEED_RATIO_T speed_ratio;
    union DRV_NORFLASH_DTR_CFG_T dtr_quad_cfg;
    uint32_t max_speed;
};

struct NORFLASH_SFDP_LIST_T {
    uint8_t id[NORFLASH_ID_LEN];
    uint8_t cfg_cnt;
    uint8_t sfdp_check_offset;
    uint8_t sfdp_check_len;
    struct NORFLASH_SFDP_CFG_T *cfg;
};

struct NORFLASH_CFG_T {
    uint8_t id[NORFLASH_ID_LEN];
    union DRV_NORFLASH_SPEED_RATIO_T speed_ratio;
    union DRV_NORFLASH_DTR_CFG_T dtr_quad_cfg;
    uint8_t crm_en_bits;
    uint8_t crm_dis_bits;
    uint16_t block_protect_mask;
    union DRV_NORFLASH_SEC_REG_CFG_T sec_reg_cfg;
    uint32_t total_size;
    uint32_t max_speed;
    uint32_t mode;
    NORFLASH_WRITE_STATUS_CB write_status;
};

uint8_t norflash_read_status_s0_s7(enum HAL_FLASH_ID_T id);

void norflash_dual_chip_read_status_s0_s7(enum HAL_FLASH_ID_T id, uint8_t *val0, uint8_t *val1);

uint8_t norflash_read_status_s8_s15(enum HAL_FLASH_ID_T id);

void norflash_dual_chip_read_status_s8_s15(enum HAL_FLASH_ID_T id, uint8_t *val0, uint8_t *val1);

void norflash_status_WEL_0_wait(enum HAL_FLASH_ID_T id);

enum HAL_NORFLASH_RET_T norflash_status_WIP_1_wait(enum HAL_FLASH_ID_T id, int suspend);

uint32_t norflash_get_supported_mode(enum HAL_FLASH_ID_T id);

uint32_t norflash_get_current_mode(enum HAL_FLASH_ID_T id);

union DRV_NORFLASH_SEC_REG_CFG_T norflash_get_security_register_config(enum HAL_FLASH_ID_T id);

uint32_t norflash_get_block_protect_mask(enum HAL_FLASH_ID_T id);

void norflash_set_dual_chip_mode(enum HAL_FLASH_ID_T id, int flash, int sec_reg, int tx_copy);

void norflash_reset(enum HAL_FLASH_ID_T id);

int norflash_get_total_size(enum HAL_FLASH_ID_T id, uint32_t *total_size);

int norflash_get_page_size(enum HAL_FLASH_ID_T id, uint32_t *page_size);

int norflash_set_mode(enum HAL_FLASH_ID_T id, uint32_t op);

int norflash_pre_operation_suspend(enum HAL_FLASH_ID_T id, int suspend);

int norflash_pre_operation(enum HAL_FLASH_ID_T id);

int norflash_post_operation_suspend(enum HAL_FLASH_ID_T id, int suspend, int skip_mode_change);

int norflash_post_operation(enum HAL_FLASH_ID_T id);

int norflash_read_reg(enum HAL_FLASH_ID_T id, uint8_t cmd, uint8_t *val, uint32_t len);

int norflash_read_reg_ex(enum HAL_FLASH_ID_T id, uint8_t cmd, uint8_t *param, uint32_t param_len, uint8_t *val, uint32_t len);

int norflash_dual_chip_read_reg_ex(enum HAL_FLASH_ID_T id, uint8_t cmd, uint8_t *param, uint32_t param_len, uint8_t *val0, uint8_t *val1, uint32_t len);

int norflash_write_reg(enum HAL_FLASH_ID_T id, bool volatile_reg, uint8_t cmd, const uint8_t *val, uint32_t len);

int norflash_init_sample_delay_by_div(enum HAL_FLASH_ID_T id, uint32_t div);

void norflash_set_sample_delay_index(enum HAL_FLASH_ID_T id, uint32_t index);

uint32_t norflash_get_sample_delay_index(enum HAL_FLASH_ID_T id);

void norflash_set_rclk_delay(enum HAL_FLASH_ID_T id, const struct HAL_NORFLASH_TIMING_CONFIG_T *timing);

void norflash_get_rclk_delay(enum HAL_FLASH_ID_T id, struct HAL_NORFLASH_TIMING_CONFIG_T *timing);

void norflash_calib_get_seq_pattern(const uint32_t **seq, uint32_t *word_cnt);

int norflash_sample_delay_calib(enum HAL_FLASH_ID_T id, enum DRV_NORFLASH_CALIB_T type);

void norflash_show_calib_result(enum HAL_FLASH_ID_T id);

int norflash_init_div(enum HAL_FLASH_ID_T id, const struct HAL_NORFLASH_CONFIG_T *cfg);

int norflash_match_chip(enum HAL_FLASH_ID_T id, const uint8_t *dev_id, uint32_t len);

void norflash_match_sfdp(enum HAL_FLASH_ID_T id, const uint8_t *dev_id, uint8_t len);

void norflash_show_sfdp_check_param(enum HAL_FLASH_ID_T id);

void norflash_get_flash_list(const struct NORFLASH_CFG_T **list, uint32_t *len);

int norflash_get_id(enum HAL_FLASH_ID_T id, uint8_t *value, uint32_t len);

int norflash_dual_chip_get_id(enum HAL_FLASH_ID_T id, uint8_t *value0, uint8_t *value1, uint32_t len);

int norflash_get_unique_id(enum HAL_FLASH_ID_T id, uint8_t *value, uint32_t len);

int norflash_dual_chip_get_unique_id(enum HAL_FLASH_ID_T id, uint8_t *value0, uint8_t *value1, uint32_t len);

int norflash_read_sfdp_reg(enum HAL_FLASH_ID_T id, uint8_t addr, uint8_t *value, uint32_t len);

int norflash_dual_chip_read_sfdp_reg(enum HAL_FLASH_ID_T id, uint8_t addr, uint8_t *value0, uint8_t *value1, uint32_t len);

void norflash_enter_4byteaddr_mode(enum HAL_FLASH_ID_T id);

void norflash_exit_4byteaddr_mode(enum HAL_FLASH_ID_T id);

enum HAL_NORFLASH_RET_T norflash_erase(enum HAL_FLASH_ID_T id, uint32_t start_address, enum DRV_NORFLASH_ERASE_T type, int suspend);

enum HAL_NORFLASH_RET_T norflash_erase_resume(enum HAL_FLASH_ID_T id, int suspend);

enum HAL_NORFLASH_RET_T norflash_write(enum HAL_FLASH_ID_T id, uint32_t start_address, const uint8_t *buffer, uint32_t len, int suspend);

enum HAL_NORFLASH_RET_T norflash_write_resume(enum HAL_FLASH_ID_T id, int suspend);

int norflash_suspend_check_irq(enum HAL_FLASH_ID_T id, uint32_t irq_num, int enable);

int norflash_suspend_check_flash_read(enum HAL_FLASH_ID_T id, int enable);

int norflash_read(enum HAL_FLASH_ID_T id, uint32_t start_address, uint8_t *buffer, uint32_t len);

void norflash_force_sleep(enum HAL_FLASH_ID_T id);

void norflash_force_wakeup(enum HAL_FLASH_ID_T id);

void norflash_sleep(enum HAL_FLASH_ID_T id);

void norflash_wakeup(enum HAL_FLASH_ID_T id);

void norflash_read_status(enum HAL_FLASH_ID_T id, uint16_t *status);

void norflash_dual_chip_read_status(enum HAL_FLASH_ID_T id, uint16_t *status0, uint16_t *status1);

int norflash_init_status(enum HAL_FLASH_ID_T id, uint32_t status);

int norflash_init_block_protection(enum HAL_FLASH_ID_T id, uint32_t bp);

int norflash_set_block_protection(enum HAL_FLASH_ID_T id, uint32_t bp);

int norflash_security_register_get_lock_status(enum HAL_FLASH_ID_T id, uint32_t id_map, uint32_t *p_locked);

int norflash_security_register_lock(enum HAL_FLASH_ID_T id, uint32_t id_map);

enum HAL_NORFLASH_RET_T norflash_security_register_erase(enum HAL_FLASH_ID_T id, uint32_t start_address);

enum HAL_NORFLASH_RET_T norflash_security_register_write(enum HAL_FLASH_ID_T id, uint32_t start_address, const uint8_t *buffer, uint32_t len);

int norflash_security_register_read(enum HAL_FLASH_ID_T id, uint32_t start_address, uint8_t *buffer, uint32_t len);

uint32_t norflash_security_register_enable_read(enum HAL_FLASH_ID_T id);

void norflash_security_register_disable_read(enum HAL_FLASH_ID_T id, uint32_t mode);

#ifdef __cplusplus
}
#endif

#endif /* DRV_NORFLASH_H */
