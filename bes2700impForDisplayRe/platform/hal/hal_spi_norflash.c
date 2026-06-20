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
#include "plat_types.h"

#include "hal_spi.h"
#include "spi_norflash_drv.h"
#include "hal_spi_norflash.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "string.h"

#ifdef SPI_NORFLASH_DEBUG
    #define HAL_SPI_NORFLASH_TRACE(n, s, ...)    TRACE(n, s, ##__VA_ARGS__)
#else
    #define HAL_SPI_NORFLASH_TRACE(n, s, ...)    TRACE_DUMMY(n, s, ##__VA_ARGS__)
#endif

static const struct HAL_SPI_CFG_T _spi_cfg_default = {
    .rate = 24 * 1000000,
    .clk_delay_half = true,
    .clk_polarity = true,
    .slave = false,
    .dma_rx = true,
    .dma_tx = true,
    .rx_sep_line = true,
    .cs = 0,
    .tx_bits = 32,
    .rx_bits = 32,
    .rx_frame_bits = 0,
};

struct HAL_SPI_NORFLASH_CONTEXT {
    bool opened;
    uint8_t device_id[HAL_SPI_NORFLASH_DEVICE_ID_LEN];
#ifdef FLASH_UNIQUE_ID
    uint8_t unique_id[HAL_SPI_NORFLASH_UNIQUE_ID_LEN + HAL_SPI_NORFLASH_CP_ID_LEN];
#endif
    uint32_t total_size;
    uint32_t block_size;
    uint32_t sector_size;
    uint32_t page_size;
    enum HAL_SPI_NORFLASH_RET_T open_state;
};

static struct HAL_SPI_NORFLASH_CONTEXT spi_norflash_ctx[HAL_SPI_NORFLASH_ID_NUM];

static const struct HAL_SPI_NORFLASH_CONFIG_T spi_norflash_cfg = {
    .mode = HAL_SPI_NORFLASH_OP_MODE_STAND_SPI,
};

enum HAL_SPI_NORFLASH_RET_T _spi_norflash_open(enum HAL_SPI_NORFLASH_ID_T id, const struct HAL_SPI_NORFLASH_CONFIG_T *cfg, uint32_t spi_rate)
{
    enum HAL_SPI_NORFLASH_RET_T ret = HAL_SPI_NORFLASH_OK;
    int32_t result;
    bool matched;
    uint8_t flash_id[HAL_SPI_NORFLASH_DEVICE_ID_LEN];
    uint32_t total_size = 0;
    uint32_t block_size = 0;
    uint32_t sector_size = 0;
    uint32_t page_size = 0;
    struct HAL_SPI_CFG_T spi_cfg;

    HAL_SPI_NORFLASH_TRACE(2, "%s:%d", __func__, __LINE__);
    ASSERT(id < HAL_SPI_NORFLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(!spi_norflash_ctx[id].opened, "%s: flash is opened!", __func__);

    spi_cfg = _spi_cfg_default;
    spi_cfg.rate = spi_rate;

    result = spi_norflash_drv_spi_init(&spi_cfg);
    if (result) {
        HAL_SPI_NORFLASH_TRACE(1, "%s: spi_norflash_drv_spi_init failed! result = %d", __func__, result);
        return HAL_SPI_NORFLASH_SPI_INIT_ERROR;
    }

    result = spi_norflash_drv_get_id(flash_id, sizeof(flash_id));
    if (result) {
        HAL_SPI_NORFLASH_TRACE(1, "%s: spi_norflash_drv_get_id failed! ret = %d", __func__, result);
        return HAL_SPI_NORFLASH_GET_ID_FAIL;
    }

    matched = spi_norflash_match_chip(flash_id, HAL_SPI_NORFLASH_DEVICE_ID_LEN);
    if (matched) {
        result = spi_norflash_get_size(&total_size, &block_size, &sector_size, &page_size);
        if (result == 0) {
            memcpy(spi_norflash_ctx[id].device_id, flash_id, HAL_SPI_NORFLASH_DEVICE_ID_LEN);
            spi_norflash_ctx[id].total_size = total_size;
            spi_norflash_ctx[id].block_size = block_size;
            spi_norflash_ctx[id].sector_size = sector_size;
            spi_norflash_ctx[id].page_size = page_size;
            spi_norflash_ctx[id].opened = true;
            ret = HAL_SPI_NORFLASH_OK;
        } else {
            HAL_SPI_NORFLASH_TRACE(1, "%s: spi_norflash_get_size failed! result = %d", __func__, result);
            ret = HAL_SPI_NORFLASH_BAD_IDX;
        }
    } else {
        HAL_SPI_NORFLASH_TRACE(1, "%s: spi_norflash_match_chip false! ", __func__);
        ret = HAL_SPI_NORFLASH_BAD_ID;
    }

#ifdef FLASH_UNIQUE_ID
    spi_norflash_drv_get_unique_id(spi_norflash_ctx[id].unique_id, sizeof(spi_norflash_ctx[id].unique_id));
#endif

    spi_norflash_ctx[id].open_state = ret;

    return ret;
}

//---------------------------------------------------------------------
// SPI Flash Apis
//---------------------------------------------------------------------
enum HAL_SPI_NORFLASH_RET_T hal_spi_norflash_init(void)
{
    enum HAL_SPI_NORFLASH_RET_T ret;

    HAL_SPI_NORFLASH_TRACE(2, "%s:%d", __func__, __LINE__);
    ret = hal_spi_norflash_open(HAL_SPI_NORFLASH_ID_0, &spi_norflash_cfg, _spi_cfg_default.rate);
    if (ret != HAL_SPI_NORFLASH_OK) {
        HAL_SPI_NORFLASH_TRACE(1, "%s: hal_spi_norflash_open failed! ret = %d", __func__, ret);
    }

    return ret;
}

enum HAL_SPI_NORFLASH_RET_T hal_spi_norflash_deinit(void)
{
    hal_spi_norflash_close(HAL_SPI_NORFLASH_ID_0);
    return HAL_SPI_NORFLASH_OK;
}

enum HAL_SPI_NORFLASH_RET_T hal_spi_norflash_open(enum HAL_SPI_NORFLASH_ID_T id, const struct HAL_SPI_NORFLASH_CONFIG_T *cfg, uint32_t spi_rate)
{
    return _spi_norflash_open(id, cfg, spi_rate);
}

enum HAL_SPI_NORFLASH_RET_T hal_spi_norflash_close(enum HAL_SPI_NORFLASH_ID_T id)
{
    HAL_SPI_NORFLASH_TRACE(2, "%s:%d", __func__, __LINE__);
    ASSERT(id < HAL_SPI_NORFLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_norflash_ctx[id].opened, "%s: flash not opened!", __func__);

    memset(&spi_norflash_ctx[id], 0, sizeof(spi_norflash_ctx[id]));
    spi_norflash_drv_spi_deinit();

    return HAL_SPI_NORFLASH_OK;
}

enum HAL_SPI_NORFLASH_RET_T hal_spi_norflash_get_id(enum HAL_SPI_NORFLASH_ID_T id, uint8_t *value, uint32_t len)
{
    int32_t ret;

    HAL_SPI_NORFLASH_TRACE(2, "%s:%d", __func__, __LINE__);
    ASSERT(id < HAL_SPI_NORFLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_norflash_ctx[id].opened, "%s: flash not opened!", __func__);

    ret = spi_norflash_drv_get_id(value, len);
    if (ret) {
        HAL_SPI_NORFLASH_TRACE(1, "%s: spi_norflash_drv_get_id failed! ret = %d.", __func__, ret);
        return HAL_SPI_NORFLASH_ERR;
    }

    return HAL_SPI_NORFLASH_OK;
}

#ifdef FLASH_UNIQUE_ID
enum HAL_SPI_NORFLASH_RET_T hal_spi_norflash_get_unique_id(enum HAL_SPI_NORFLASH_ID_T id, uint8_t *value, uint32_t len)
{
    ASSERT(spi_norflash_ctx[id].opened, "%s: spi norflash not opened", __func__);

    len = MIN(len, sizeof(spi_norflash_ctx[id].unique_id));

    memcpy(value, spi_norflash_ctx[id].unique_id, len);

    return HAL_SPI_NORFLASH_OK;
}
#endif

enum HAL_SPI_NORFLASH_RET_T hal_spi_norflash_get_size(enum HAL_SPI_NORFLASH_ID_T id, uint32_t *total_size, uint32_t *block_size, uint32_t *sector_size, uint32_t *page_size)
{
    HAL_SPI_NORFLASH_TRACE(2, "%s:%d", __func__, __LINE__);
    ASSERT(id < HAL_SPI_NORFLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_norflash_ctx[id].opened, "%s: spi norflash not opened", __func__);

    *total_size = spi_norflash_ctx[id].total_size;
    *block_size = spi_norflash_ctx[id].block_size;
    *sector_size = spi_norflash_ctx[id].sector_size;
    *page_size = spi_norflash_ctx[id].page_size;

    return HAL_SPI_NORFLASH_OK;
}

enum HAL_SPI_NORFLASH_RET_T hal_spi_norflash_read(enum HAL_SPI_NORFLASH_ID_T id, uint32_t addr, uint8_t *buff, uint32_t len)
{
    int32_t ret;

    HAL_SPI_NORFLASH_TRACE(2, "%s:%d", __func__, __LINE__);
    ASSERT(id < HAL_SPI_NORFLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_norflash_ctx[id].opened, "%s: spi norflash not opened", __func__);

    if (addr + len > spi_norflash_ctx[id].total_size) {
        HAL_SPI_NORFLASH_TRACE(1, "%s: addr error! add = 0x%x,len = 0x%x.", __func__, addr, len);
        return HAL_SPI_NORFLASH_BAD_ADDR;
    }

    if (buff == NULL) {
        HAL_SPI_NORFLASH_TRACE(1, "%s: buff is null!", __func__);
        return HAL_SPI_NORFLASH_BUFF_IS_NULL;
    }

    if (len == 0) {
        HAL_SPI_NORFLASH_TRACE(1, "%s: len = 0!", __func__);
        return HAL_SPI_NORFLASH_BAD_LEN;
    }

    ret = spi_norflash_drv_read(addr, buff, len);
    if (ret) {
        HAL_SPI_NORFLASH_TRACE(1, "%s: spi_norflash_drv_read failed! ret = %d", __func__, ret);
        return HAL_SPI_NORFLASH_READ_ERROR;
    }

    return HAL_SPI_NORFLASH_OK;
}

enum HAL_SPI_NORFLASH_RET_T hal_spi_norflash_write(enum HAL_SPI_NORFLASH_ID_T id, uint32_t addr, uint8_t *buff, uint32_t len)
{
    int32_t ret;

    HAL_SPI_NORFLASH_TRACE(2, "%s:%d", __func__, __LINE__);
    ASSERT(id < HAL_SPI_NORFLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_norflash_ctx[id].opened, "%s: spi norflash not opened", __func__);

    if (addr + len > spi_norflash_ctx[id].total_size) {
        HAL_SPI_NORFLASH_TRACE(1, "%s: addr error! add = 0x%x,len = 0x%x.", __func__, addr, len);
        return HAL_SPI_NORFLASH_BAD_ADDR;
    }

    if (buff == NULL) {
        HAL_SPI_NORFLASH_TRACE(1, "%s: buff is null!", __func__);
        return HAL_SPI_NORFLASH_BUFF_IS_NULL;
    }

    if (len == 0) {
        HAL_SPI_NORFLASH_TRACE(1, "%s: len = 0!", __func__);
        return HAL_SPI_NORFLASH_BAD_LEN;
    }

    ret = spi_norflash_drv_write(addr, buff, len);
    if (ret) {
        HAL_SPI_NORFLASH_TRACE(1, "%s: spi_norflash_drv_write failed! ret = %d", __func__, ret);
        return HAL_SPI_NORFLASH_WRITE_ERROR;
    }

    return HAL_SPI_NORFLASH_OK;
}

enum HAL_SPI_NORFLASH_RET_T hal_spi_norflash_erase(enum HAL_SPI_NORFLASH_ID_T id, uint32_t addr, uint32_t len)
{
    int32_t ret;

    HAL_SPI_NORFLASH_TRACE(2, "%s:%d", __func__, __LINE__);
    ASSERT(id < HAL_SPI_NORFLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_norflash_ctx[id].opened, "%s: spi norflash not opened", __func__);

    if (addr + len > spi_norflash_ctx[id].total_size) {
        HAL_SPI_NORFLASH_TRACE(1, "%s: addr error! add = 0x%x,len = 0x%x.", __func__, addr, len);
        return HAL_SPI_NORFLASH_BAD_ADDR;
    }

    if (len == 0) {
        HAL_SPI_NORFLASH_TRACE(1, "%s: len = 0!", __func__);
        return HAL_SPI_NORFLASH_BAD_LEN;
    }
#ifdef FLASH_ERASE_PAGE_ENABLE
    if (addr & (spi_norflash_ctx[id].page_size - 1)) {
        HAL_SPI_NORFLASH_TRACE(1, "%s: addr no alignment.addr = 0x%x!", __func__, addr);
        return HAL_SPI_NORFLASH_BAD_ADDR;
    }

    if (len & (spi_norflash_ctx[id].page_size - 1)) {
        HAL_SPI_NORFLASH_TRACE(1, "%s: len no alignment.addr = 0x%x!", __func__, len);
        return HAL_SPI_NORFLASH_BAD_ADDR;
    }
#else
    if (addr & (spi_norflash_ctx[id].sector_size - 1)) {
        HAL_SPI_NORFLASH_TRACE(1, "%s: addr no alignment.addr = 0x%x!", __func__, addr);
        return HAL_SPI_NORFLASH_BAD_ADDR;
    }

    if (len & (spi_norflash_ctx[id].sector_size - 1)) {
        HAL_SPI_NORFLASH_TRACE(1, "%s: len no alignment.addr = 0x%x!", __func__, len);
        return HAL_SPI_NORFLASH_BAD_ADDR;
    }
#endif

    ret = spi_norflash_drv_erase(addr, len);
    if (ret) {
        HAL_SPI_NORFLASH_TRACE(1, "%s: spi_norflash_drv_erase failed! ret = %d", __func__, ret);
        return HAL_SPI_NORFLASH_ERASE_ERROR;
    }

    return HAL_SPI_NORFLASH_OK;
}

enum HAL_SPI_NORFLASH_RET_T hal_spi_norflash_reset(enum HAL_SPI_NORFLASH_ID_T id)
{
    int ret;

    HAL_SPI_NORFLASH_TRACE(2, "%s:%d", __func__, __LINE__);
    ret = spi_norflash_drv_reset();
    if (ret) {
        HAL_SPI_NORFLASH_TRACE(1, "%s: spi_norflash_drv_reset failed! ret = %d", __func__, ret);
        return HAL_SPI_NORFLASH_RESET_ERROR;
    }

    return HAL_SPI_NORFLASH_OK;
}

