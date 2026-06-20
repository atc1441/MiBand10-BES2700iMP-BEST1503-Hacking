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
#ifndef SPI_NORFLASH_DRV_H
#define SPI_NORFLASH_DRV_H

#include "plat_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

//---------------------------------------------------------------------------
// API
//---------------------------------------------------------------------------

bool spi_norflash_match_chip(const uint8_t *id, uint32_t len);
int32_t spi_norflash_get_size(uint32_t *total_size, uint32_t *block_size, uint32_t *sector_size, uint32_t *page_size);

int32_t spi_norflash_drv_spi_init(const struct HAL_SPI_CFG_T *spi_cfg);
int32_t spi_norflash_drv_spi_deinit(void);
int32_t spi_norflash_drv_get_id(uint8_t *id, uint32_t len);
int32_t spi_norflash_drv_get_unique_id(uint8_t *id, uint32_t len);
int32_t spi_norflash_drv_read(uint32_t addr, uint8_t *buff, uint32_t len);
int32_t spi_norflash_drv_write(uint32_t addr, uint8_t *buff, uint32_t len);
int32_t spi_norflash_drv_erase(uint32_t addr, uint32_t len);
int32_t spi_norflash_drv_reset(void);

// Error code.
#define SPI_NORFLASH_DRV_RET_OK   0
#define SPI_NORFLASH_DRV_RET_SPI_ERROR 1


#ifdef __cplusplus
}
#endif

#endif /*SPI_NORFLASH_DRV_H*/

