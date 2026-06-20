/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifndef SPI_NORFLASH_HAL_H
#define SPI_NORFLASH_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"
#include "hal_cmu.h"

#define HAL_SPI_NORFLASH_DEVICE_ID_LEN          3
#define HAL_SPI_NORFLASH_UNIQUE_ID_LEN          16
#define HAL_SPI_NORFLASH_CP_ID_LEN              2

enum HAL_SPI_NORFLASH_ID_T {
    HAL_SPI_NORFLASH_ID_0 = 0,
    HAL_SPI_NORFLASH_ID_NUM,
};

enum HAL_SPI_NORFLASH_RET_T {
    HAL_SPI_NORFLASH_OK,
    HAL_SPI_NORFLASH_ERR,
    HAL_SPI_NORFLASH_GET_ID_FAIL,
    HAL_SPI_NORFLASH_BAD_ID,
    HAL_SPI_NORFLASH_BAD_IDX,
    HAL_SPI_NORFLASH_BAD_ADDR,
    HAL_SPI_NORFLASH_BAD_LEN,
    HAL_SPI_NORFLASH_BUFF_IS_NULL,
    HAL_SPI_NORFLASH_NOT_OPENED,
    HAL_SPI_NORFLASH_CFG_NULL,
    HAL_SPI_NORFLASH_SPI_INIT_ERROR,
    HAL_SPI_NORFLASH_SPI_SEND_ERROR,
    HAL_SPI_NORFLASH_SPI_RECV_ERROR,
    HAL_SPI_NORFLASH_SEMAPHORE_ERROR,
    HAL_SPI_NORFLASH_CHECK_ID_ERROR,
    HAL_SPI_NORFLASH_READ_ERROR,
    HAL_SPI_NORFLASH_WRITE_ERROR,
    HAL_SPI_NORFLASH_ERASE_ERROR,
    HAL_SPI_NORFLASH_RESET_ERROR,
};

enum HAL_SPI_NORFLASH_OP_MODE {
    // (1) basic read mode
    // standard spi mode
    HAL_SPI_NORFLASH_OP_MODE_STAND_SPI      = (1 << 0),
    // fast spi mode.
    HAL_SPI_NORFLASH_OP_MODE_FAST_SPI       = (1 << 1),

    HAL_SPI_NORFLASH_OP_MODE_RESERVED       = 0xFFFFFFFF,
};

struct HAL_SPI_NORFLASH_CONFIG_T {
    enum HAL_SPI_NORFLASH_OP_MODE mode;
};

/* hal api */
enum HAL_SPI_NORFLASH_RET_T hal_spi_norflash_init(void);
enum HAL_SPI_NORFLASH_RET_T hal_spi_norflash_deinit(void);
enum HAL_SPI_NORFLASH_RET_T hal_spi_norflash_open(enum HAL_SPI_NORFLASH_ID_T id, const struct HAL_SPI_NORFLASH_CONFIG_T *cfg, uint32_t spi_rate);
enum HAL_SPI_NORFLASH_RET_T hal_spi_norflash_close(enum HAL_SPI_NORFLASH_ID_T id);
enum HAL_SPI_NORFLASH_RET_T hal_spi_norflash_get_id(enum HAL_SPI_NORFLASH_ID_T id, uint8_t *value, uint32_t len);
enum HAL_SPI_NORFLASH_RET_T hal_spi_norflash_get_unique_id(enum HAL_SPI_NORFLASH_ID_T id, uint8_t *value, uint32_t len);
enum HAL_SPI_NORFLASH_RET_T hal_spi_norflash_get_size(enum HAL_SPI_NORFLASH_ID_T id, uint32_t *total_size, uint32_t *block_size, uint32_t *sector_size, uint32_t *page_size);
enum HAL_SPI_NORFLASH_RET_T hal_spi_norflash_read(enum HAL_SPI_NORFLASH_ID_T id, uint32_t addr, uint8_t *buff, uint32_t len);
enum HAL_SPI_NORFLASH_RET_T hal_spi_norflash_write(enum HAL_SPI_NORFLASH_ID_T id, uint32_t addr, uint8_t *buff, uint32_t len);
enum HAL_SPI_NORFLASH_RET_T hal_spi_norflash_erase(enum HAL_SPI_NORFLASH_ID_T id, uint32_t addr, uint32_t len);
enum HAL_SPI_NORFLASH_RET_T hal_spi_norflash_reset(enum HAL_SPI_NORFLASH_ID_T id);
#ifdef __cplusplus
}
#endif

#endif /* SPI_NORFLASH_HAL_H */

