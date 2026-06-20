/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifndef __HAL_SDMMC_H__
#define __HAL_SDMMC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"
#include "plat_addr_map.h"

enum HAL_SDMMC_ID_T {
    HAL_SDMMC_ID_0 = 0,
#ifdef SDMMC1_BASE
    HAL_SDMMC_ID_1,
#endif

    HAL_SDMMC_ID_NUM,
};

enum HAL_SDMMC_ERR {
    HAL_SDMMC_ERR_NONE              = 0,
    HAL_SDMMC_RESET_FAIL            = 1,
    HAL_SDMMC_DMA_IN_USE            = 2,
    HAL_SDMMC_COMM_TIMEOUT          = 3,
    HAL_SDMMC_NOT_DATA_MSG          = 4,    /* EBADMSG, Not a data message */
    HAL_SDMMC_NO_SUCH_DEVICE        = 5,    /* ENODEV, No such device */
    HAL_SDMMC_INVALID_SYS_CALL      = 6,
    HAL_SDMMC_WAIT_DAT0_TIMEOUT     = 7,
    HAL_SDMMC_IO_ERR                = 8,    /* EIO, I/O error */
    HAL_SDMMC_COMM_ERR              = 9,    /* ECOMM, Communication error on send*/
    HAL_SDMMC_OUT_OF_MEMORY         = 10,   /* ENOMEM, Out of memory */
    HAL_SDMMC_NO_MEDIUM_FOUND       = 11,   /* ENOMEDIUM, No medium found */
    HAL_SDMMC_MEDIUM_TYPE_ERR       = 12,   /* EMEDIUMTYPE, Wrong medium type*/
    HAL_SDMMC_OP_NOT_PERMITTED      = 13,   /* EPERM, Operation not permitted */
    HAL_SDMMC_OP_NOT_SUPPORTED      = 14,   /* ENOTSUPP, Operation is not supported */
    HAL_SDMMC_OP_NOT_SUPPORTED_EP   = 15,   /* EOPNOTSUPP, Operation not supported on transport endpoint */
    HAL_SDMMC_RESPONSE_ERR          = 16,
    HAL_SDMMC_RESPONSE_BUSY         = 17,
    HAL_SDMMC_RESPONSE_TIMEOUT      = 18,
    HAL_SDMMC_CMD_SEND_TIMEOUT      = 19,
    HAL_SDMMC_CMD_START_TIMEOUT1    = 20,
    HAL_SDMMC_CMD_START_TIMEOUT2    = 21,
    HAL_SDMMC_CMD_START_TIMEOUT3    = 22,
    HAL_SDMMC_CMD_START_TIMEOUT4    = 23,
    HAL_SDMMC_CMD_START_TIMEOUT5    = 24,
    HAL_SDMMC_INVALID_PARAMETER     = 25,
};

enum HAL_SDMMC_HOST_ERR {
    HAL_SDMMC_HOST_RESPONSE_ERR                 = 100,
    HAL_SDMMC_HOST_RESPONSE_CRC_ERR             = 101,
    HAL_SDMMC_HOST_DATA_CRC_ERR                 = 102,
    HAL_SDMMC_HOST_RESPONSE_TIMEOUT_BAR         = 103,
    HAL_SDMMC_HOST_DATA_READ_TIMEOUT_BDS        = 104,
    HAL_SDMMC_HOST_STARVATION_TIMEOUT           = 105,
    HAL_SDMMC_HOST_FIFO_ERR                     = 106,
    HAL_SDMMC_HOST_HARDWARE_LOCKED_WRITE_ERR    = 107,
    HAL_SDMMC_HOST_START_BIT_ERR                = 108,
    HAL_SDMMC_HOST_READ_END_BIT_ERR_WRITE_NOCRC = 109,
};

struct HAL_SDMMC_CB_T {
    void (*hal_sdmmc_host_error)(enum HAL_SDMMC_HOST_ERR error);
    void (*hal_sdmmc_txrx_done)(void);
};

typedef void (*HAL_SDMMC_DELAY_FUNC)(uint32_t ms);
HAL_SDMMC_DELAY_FUNC hal_sdmmc_set_delay_func(HAL_SDMMC_DELAY_FUNC new_func);

int hal_sdmmc_open(enum HAL_SDMMC_ID_T id);
int hal_sdmmc_open_ext(enum HAL_SDMMC_ID_T id, struct HAL_SDMMC_CB_T *callback);
void hal_sdmmc_close(enum HAL_SDMMC_ID_T id);

uint32_t hal_sdmmc_read_blocks(enum HAL_SDMMC_ID_T id, uint32_t start_block, uint32_t block_count, uint8_t *dest);
uint32_t hal_sdmmc_write_blocks(enum HAL_SDMMC_ID_T id, uint32_t start_block, uint32_t block_count, uint8_t *src);

//uint32_t hal_sdmmc_read_blocks_dma(enum HAL_SDMMC_ID_T id, uint32_t start_block, uint32_t block_count, uint8_t *dest);
//uint32_t hal_sdmmc_write_blocks_dma(enum HAL_SDMMC_ID_T id, uint32_t start_block, uint32_t block_count, uint8_t *src);
//uint32_t hal_sdmmc_read_blocks_polling(enum HAL_SDMMC_ID_T id, uint32_t start_block, uint32_t block_count, uint8_t *dest);
//uint32_t hal_sdmmc_write_blocks_polling(enum HAL_SDMMC_ID_T id, uint32_t start_block, uint32_t block_count, uint8_t *src);

int hal_sdmmc_get_status(enum HAL_SDMMC_ID_T id);
uint32_t hal_sdmmc_get_bus_speed(enum HAL_SDMMC_ID_T id);

void hal_sdmmc_dump(enum HAL_SDMMC_ID_T id);
void hal_sdmmc_info(enum HAL_SDMMC_ID_T id, uint32_t *sector_count, uint32_t *sector_size);


#ifdef __cplusplus
}
#endif

#endif

