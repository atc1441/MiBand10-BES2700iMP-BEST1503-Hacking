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
#include "hal_cmu.h"
#include "hal_norflaship.h"
#include "norflash_gd25q32c.h"
#include "norflash_cfg.h"
#include "norflash_drv.h"
#include "spi_norflash_drv.h"
#include "hal_spi_norflash.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "string.h"

#ifdef SPI_NORFLASH_DRV_DEBUG
    #define SPIFLASH_TRACE(n, s, ...)    TRACE(n, s, ##__VA_ARGS__)
#else
    #define SPIFLASH_TRACE(n, s, ...)    TRACE_DUMMY(n, s, ##__VA_ARGS__)
#endif

#define SPI_CMD_SIZE            32
#define SPI_BUFFER_SIZE         256
#define SPI_WORD_ALIGN(x,y)     (((x+y-1)/y)*y)
#define TO_LITTLE_ENDIAN(val)   (((val & 0xff) << 24) | ((val & 0xff00) << 8) | \
                                 ((val & 0xff0000) >> 8) | ((val & 0xff000000) >> 24))

#ifdef SPI_NORFLASH_USING_SPILCD
    #define SPI_DMA_SEND  hal_spilcd_dma_send
    #define SPI_DMA_RECV  hal_spilcd_dma_recv
    #define SPI_ENABLE_AND_SEND  hal_spilcd_enable_and_send
    #define SPI_ENABLE_AND_RECV  hal_spilcd_enable_and_recv
    #define SPI_OPEN  hal_spilcd_open
    extern void hal_iomux_set_spilcd(void);
    #define IOMUX_SET_SPI hal_iomux_set_spilcd
#else
    #define SPI_DMA_SEND  hal_spi_dma_send
    #define SPI_DMA_RECV  hal_spi_dma_recv
    #define SPI_ENABLE_AND_SEND  hal_spi_enable_and_send
    #define SPI_ENABLE_AND_RECV  hal_spi_enable_and_recv
    #define SPI_OPEN  hal_spi_open
    extern void hal_iomux_set_spi(void);
    #define IOMUX_SET_SPI hal_iomux_set_spi
#endif

#ifdef FLASH_ERASE_PAGE_ENABLE
    #define PUYA_FLASH_CMD_PAGE_ERASE   0x81
#endif
#define INVALID_SPIFLASH_IDX            0xFF

static uint8_t spi_send_buffer[SPI_CMD_SIZE + SPI_BUFFER_SIZE];
static uint8_t spi_recv_buffer[SPI_CMD_SIZE + SPI_BUFFER_SIZE];


static struct HAL_SPI_CTRL_T _spi_ctrl_8bit;
static uint8_t spi_norflash_idx = INVALID_SPIFLASH_IDX;
static const struct NORFLASH_CFG_T  **spi_norflash_list = NULL;
static uint32_t spi_list_len = 0;

bool spi_norflash_match_chip(const uint8_t *id, uint32_t len)
{
    const uint8_t *cmp_id;
    const struct NORFLASH_CFG_T  *tmp_list;

    norflash_get_flash_list(&tmp_list, &spi_list_len);
    spi_norflash_list = (const struct NORFLASH_CFG_T **)tmp_list;

    if (len == NORFLASH_ID_LEN) {
        for (spi_norflash_idx = 0; spi_norflash_idx < spi_list_len; spi_norflash_idx++) {
            cmp_id = spi_norflash_list[spi_norflash_idx]->id;
            if (id[0] == cmp_id[0] && id[1] == cmp_id[1] && id[2] == cmp_id[2]) {
                return true;
            }
        }
    }

    return false;
}

int32_t spi_norflash_get_size(uint32_t *total_size, uint32_t *block_size, uint32_t *sector_size, uint32_t *page_size)
{
    if (total_size) {
        *total_size  = spi_norflash_list[spi_norflash_idx]->total_size;
    }
    if (block_size) {
        *block_size  = NORFLASH_BLOCK_SIZE_32K;
    }
    if (sector_size) {
        *sector_size = NORFLASH_SECTOR_SIZE;
    }
    if (page_size) {
        *page_size   = NORFLASH_PAGE_SIZE;
    }

    return 0;
}

static void _drv_delay(uint32_t us)
{
#ifdef CHIP_BEST1000
    hal_sys_timer_delay(US_TO_TICKS(us));
#else
    hal_sys_timer_delay_us(us);
#endif
}

static void _drv_to_little_endian(uint8_t *out, uint8_t *in, uint32_t len)
{
    uint32_t *pout;
    uint32_t *pin;

    ASSERT((len / 4) * 4 == len, "%s: No word alignment! len = %d", __func__, len);

    pin = (uint32_t *)in;
    pout = (uint32_t *)out;

    while (len > 0) {
        *pout = TO_LITTLE_ENDIAN(*pin);
        pin ++;
        pout ++;
        len -= 4;
    }
}

static int32_t _spi_send_word(uint8_t *cmd, uint32_t cmd_len, uint8_t *buff, uint32_t buff_len)
{
    int32_t ret;
    uint32_t len;
    uint8_t *res_buff = (uint8_t *)spi_recv_buffer;
    uint8_t *dst_buff = (uint8_t *)spi_send_buffer;

    ASSERT((buff_len + cmd_len <= SPI_CMD_SIZE + SPI_BUFFER_SIZE),
           "%s: length too large, cmd_len = %d,buff_len = %d",
           __func__, cmd_len, buff_len);
    ASSERT((buff == NULL && buff_len == 0)
           || (buff != NULL && buff_len > 0),
           "%s: buff is null,but buff_len = %d",
           __func__, buff_len);

    memcpy(res_buff, cmd, cmd_len);
    len = cmd_len;
    if (buff_len > 0) {
        memcpy(res_buff + cmd_len, buff, buff_len);
        len += buff_len;
    }
    len = SPI_WORD_ALIGN(len, 4);
    _drv_to_little_endian(dst_buff, res_buff, len);

    ret = SPI_DMA_SEND(dst_buff, len, NULL);
    if (ret) {
        SPIFLASH_TRACE(1, "%s, spi dma send failed! ret = %d",
                       __func__, ret);
    }

    return ret;
}

static int32_t _spi_recv_word(uint8_t *cmd, uint32_t cmd_len, uint8_t *buff, uint32_t buff_len)
{
    int32_t ret;
    uint32_t len;
    uint8_t cmd_buff[SPI_CMD_SIZE];

    ASSERT((buff_len + cmd_len <= SPI_CMD_SIZE + SPI_BUFFER_SIZE),
           "%s: length too large, cmd_len = %d,buff_len = %d",
           __func__, cmd_len, buff_len);
    ASSERT(buff != NULL, "%s: buff is null", __func__);

    memset(cmd_buff, 0, SPI_CMD_SIZE);
    memcpy(cmd_buff, cmd, cmd_len);
    _drv_to_little_endian(spi_send_buffer, cmd_buff, SPI_WORD_ALIGN(cmd_len, 4));
    len = SPI_WORD_ALIGN((buff_len + cmd_len), 4);

    ret = SPI_DMA_RECV(spi_send_buffer, spi_recv_buffer, len, NULL);
    if (ret) {
        SPIFLASH_TRACE(1, "%s, spi dma recv failed! ret = %d",
                       __func__, ret);
    }
    _drv_to_little_endian(buff, spi_recv_buffer + cmd_len, buff_len);

    return ret;
}

static int32_t _spi_send_byte(uint8_t *cmd, uint32_t cmd_len, uint8_t *buff, uint32_t buff_len)
{
    int32_t ret;
    uint32_t len;

    ASSERT((buff_len + cmd_len <= SPI_CMD_SIZE + SPI_BUFFER_SIZE),
           "%s: length too large, cmd_len = %d,buff_len = %d",
           __func__, cmd_len, buff_len);
    ASSERT((buff == NULL && buff_len == 0)
           || (buff != NULL && buff_len > 0),
           "%s: buff is null,but buff_len = %d",
           __func__, buff_len);

    memcpy(spi_send_buffer, cmd, cmd_len);
    len = cmd_len;
    if (buff_len > 0) {
        memcpy(spi_send_buffer + cmd_len, buff, buff_len);
        len += buff_len;
    }

    ret = SPI_ENABLE_AND_SEND(&_spi_ctrl_8bit, spi_send_buffer, len);
    if (ret) {
        SPIFLASH_TRACE(1, "%s, spi enable and send failed! ret = %d",
                       __func__, ret);
    }

    return ret;
}

static int32_t _spi_recv_byte(uint8_t *cmd, uint32_t cmd_len, uint8_t *buff, uint32_t buff_len)
{
    int32_t ret;
    uint32_t len;

    ASSERT((buff_len + cmd_len <= SPI_CMD_SIZE + SPI_BUFFER_SIZE),
           "%s: length too large, cmd_len = %d,buff_len = %d",
           __func__, cmd_len, buff_len);

    ASSERT(buff != NULL,
           "%s: buff is null or buff_len = %d",
           __func__, buff_len);

    len = cmd_len + buff_len;
    memset(spi_send_buffer, 0, len);
    memcpy(spi_send_buffer, cmd, cmd_len);

    ret = SPI_ENABLE_AND_RECV(&_spi_ctrl_8bit, spi_send_buffer, spi_recv_buffer, len);
    if (ret) {
        SPIFLASH_TRACE(1, "%s, spi enable and recv failed! ret = %d",
                       __func__, ret);
    }
    memcpy(buff, spi_recv_buffer + cmd_len, buff_len);

    return ret;
}

static int _drv_busy_wait(void)
{
    int ret;
    uint8_t cmd = GD25Q32C_CMD_READ_STATUS_S0_S7;
    uint8_t status = 0;

    while (1) {
        ret = _spi_recv_byte(&cmd, sizeof(cmd), &status, sizeof(status));
        if (ret) {
            SPIFLASH_TRACE(1, "%s: _spi_recv_byte failed,ret = %d", __func__, ret);
            break;
        }
        if (!(status & GD25Q32C_WIP_BIT_MASK)) {
            return 0;
        }
    }
    return -1;
}

static int32_t _write_enable(void)
{
    int32_t ret;
    uint8_t cmd = GD25Q32C_CMD_WRITE_ENABLE;

    ret = _spi_send_byte(&cmd, sizeof(cmd), NULL, 0);
    if (ret != 0) {
        SPIFLASH_TRACE(1, "%s: _spi_send_byte failed,ret = %d", __func__, ret);
    }
    return ret;
}

static int32_t _drv_reset_enable(void)
{
    int32_t ret;
    uint8_t cmd = GD25Q32C_CMD_ENABLE_RESET;

    ret = _spi_send_byte(&cmd, sizeof(cmd), NULL, 0);
    if (ret != 0) {
        SPIFLASH_TRACE(1, "%s: _spi_send_byte failed,ret = %d", __func__, ret);
    }
    _drv_busy_wait();
    return ret;
}

static int32_t _drv_reset(void)
{
    int32_t ret;
    uint8_t cmd = GD25Q32C_CMD_RESET;

    ret = _spi_send_byte(&cmd, sizeof(cmd), NULL, 0);
    if (ret != 0) {
        SPIFLASH_TRACE(1, "%s: _spi_send_byte failed,ret = %d", __func__, ret);
    }
    _drv_busy_wait();
    return ret;
}

//------------------------------------------------------------------
// spi_norflash_drv function
//------------------------------------------------------------------

int32_t spi_norflash_drv_spi_init(const struct HAL_SPI_CFG_T *spi_cfg)
{
    int32_t ret;
    struct HAL_SPI_CFG_T spi_cfg_8bit;

    spi_cfg_8bit = *spi_cfg;
    spi_cfg_8bit.dma_rx = false;
    spi_cfg_8bit.dma_tx = false;
    spi_cfg_8bit.tx_bits = 8;
    spi_cfg_8bit.rx_bits = 8;

    SPIFLASH_TRACE(0, "  ");
    SPIFLASH_TRACE(1, "spi_cfg_8bit.clk_delay_half:%d", spi_cfg_8bit.clk_delay_half);
    SPIFLASH_TRACE(1, "spi_cfg_8bit.clk_polarity  :%d", spi_cfg_8bit.clk_polarity);
    SPIFLASH_TRACE(1, "spi_cfg_8bit.slave         :%d", spi_cfg_8bit.slave);
    SPIFLASH_TRACE(1, "spi_cfg_8bit.dma_rx        :%d", spi_cfg_8bit.dma_rx);
    SPIFLASH_TRACE(1, "spi_cfg_8bit.dma_tx        :%d", spi_cfg_8bit.dma_tx);
    SPIFLASH_TRACE(1, "spi_cfg_8bit.rx_sep_line   :%d", spi_cfg_8bit.rx_sep_line);
    SPIFLASH_TRACE(1, "spi_cfg_8bit.cs            :%d", spi_cfg_8bit.cs);
    SPIFLASH_TRACE(1, "spi_cfg_8bit.rate          :%d", spi_cfg_8bit.rate);
    SPIFLASH_TRACE(1, "spi_cfg_8bit.rx_bits       :%d", spi_cfg_8bit.rx_bits);
    SPIFLASH_TRACE(1, "spi_cfg_8bit.tx_bits       :%d", spi_cfg_8bit.tx_bits);
    SPIFLASH_TRACE(1, "spi_cfg_8bit.rx_frame_bits :%d", spi_cfg_8bit.rx_frame_bits);
    SPIFLASH_TRACE(0, "  ");

    IOMUX_SET_SPI();
    ret = SPI_OPEN(spi_cfg);
    if (ret) {
        SPIFLASH_TRACE(1, "%s, spi open failed! ret = %d", __func__, ret);
    }

    hal_spi_init_ctrl(&spi_cfg_8bit, &_spi_ctrl_8bit);//Use additional SPI configuration

    return ret;
}

int32_t spi_norflash_drv_spi_deinit(void)
{
    return 0;
}

int32_t spi_norflash_drv_get_id(uint8_t *id, uint32_t len)
{
    int32_t ret;
    uint8_t cmd = GD25Q32C_CMD_ID;

    _drv_busy_wait();
    ret = _spi_recv_byte(&cmd, sizeof(cmd), id, len);
    if (ret != 0) {
        SPIFLASH_TRACE(1, "%s: _spi_recv_byte failed,ret = %d", __func__, ret);
        return SPI_NORFLASH_DRV_RET_SPI_ERROR;
    }

    return SPI_NORFLASH_DRV_RET_OK;
}

#ifdef FLASH_UNIQUE_ID
int32_t spi_norflash_drv_get_unique_id(uint8_t *id, uint32_t len)
{
    int32_t ret;
    uint8_t cmd[5] = {GD25Q32C_CMD_UNIQUE_ID, 0,};

    _drv_busy_wait();
    ret = _spi_recv_byte(cmd, sizeof(cmd), id, len);
    if (ret != 0) {
        SPIFLASH_TRACE(1, "%s: _spi_recv_byte failed,ret = %d", __func__, ret);
        return SPI_NORFLASH_DRV_RET_SPI_ERROR;
    }

    return SPI_NORFLASH_DRV_RET_OK;
}
#endif

int32_t spi_norflash_drv_read(uint32_t addr, uint8_t *buff, uint32_t len)
{
    int32_t ret;
    uint32_t tmp_len;
    uint32_t cur_addr;
    uint32_t remain_len;
    uint8_t *pbuff;
    uint8_t cmd[4];

    _drv_busy_wait();

    cur_addr = addr;
    pbuff = buff;
    remain_len = len;
    do {
        cmd[0] = GD25Q32C_CMD_STANDARD_READ;
        cmd[1] = ((cur_addr >> 16) & 0xff);
        cmd[2] = ((cur_addr >> 8) & 0xff);
        cmd[3] = (cur_addr & 0xff);
        tmp_len = remain_len <= SPI_BUFFER_SIZE ? remain_len : SPI_BUFFER_SIZE;
        ret = _spi_recv_word(cmd, sizeof(cmd), pbuff, tmp_len);
        if (ret) {
            SPIFLASH_TRACE(1, "%s: _spi_recv_word failed!ret = %d", __func__, ret);
            return SPI_NORFLASH_DRV_RET_SPI_ERROR;
        }
        cur_addr += tmp_len;
        remain_len -= tmp_len;
        pbuff += tmp_len;
    } while (remain_len);

    return SPI_NORFLASH_DRV_RET_OK;
}

int32_t spi_norflash_drv_write(uint32_t addr, uint8_t *buff, uint32_t len)
{
    int32_t ret;
    uint32_t tmp_len;
    uint32_t cur_addr;
    uint32_t remain_len;
    uint8_t *pbuff;
    uint8_t cmd[4];

    cur_addr = addr;
    pbuff = buff;
    remain_len = len;
    _drv_busy_wait();
    do {
        _write_enable();

        cmd[0] = GD25Q32C_CMD_PAGE_PROGRAM;
        cmd[1] = ((cur_addr >> 16) & 0xff);
        cmd[2] = ((cur_addr >> 8) & 0xff);
        cmd[3] = (cur_addr & 0xff);

        tmp_len = remain_len <= SPI_BUFFER_SIZE ? remain_len : SPI_BUFFER_SIZE;
        ret = _spi_send_word(cmd, sizeof(cmd), pbuff, tmp_len);
        if (ret) {
            SPIFLASH_TRACE(1, "%s: _spi_send_word failed!ret = %d", __func__, ret);
            return SPI_NORFLASH_DRV_RET_SPI_ERROR;
        }
        remain_len -= tmp_len;
        pbuff += tmp_len;
        cur_addr += tmp_len;

        _drv_busy_wait();
    } while (remain_len);

    return SPI_NORFLASH_DRV_RET_OK;
}

int32_t spi_norflash_drv_erase(uint32_t addr, uint32_t len)
{
    int32_t ret;
    uint32_t erase_len;
    uint8_t cmd[4];
    uint8_t erase_type;
    uint32_t cur_addr;
    uint32_t remain_len;

    cur_addr = addr;
    remain_len = len;
    _drv_busy_wait();

    do {
        erase_len = 0;
#ifdef FLASH_ERASE_PAGE_ENABLE
        if (cur_addr & (NORFLASH_SECTOR_SIZE - 1)) {
            if (remain_len >= NORFLASH_PAGE_SIZE) {
                erase_len = NORFLASH_PAGE_SIZE;
            }
        } else
#endif
            if (cur_addr & (NORFLASH_BLOCK_SIZE_32K - 1)) {
                if (remain_len >= NORFLASH_SECTOR_SIZE) {
                    erase_len = NORFLASH_SECTOR_SIZE;
                } else {
#ifdef FLASH_ERASE_PAGE_ENABLE
                    if (remain_len >= NORFLASH_PAGE_SIZE) {
                        erase_len = NORFLASH_PAGE_SIZE;
                    }
#else
                    erase_len = 0;
#endif
                }
            } else if (cur_addr & ((NORFLASH_BLOCK_SIZE_32K * 2) - 1)) {
                if (remain_len >= NORFLASH_BLOCK_SIZE_32K) {
                    erase_len = NORFLASH_BLOCK_SIZE_32K;
                } else if (remain_len >= NORFLASH_SECTOR_SIZE) {
                    erase_len = NORFLASH_SECTOR_SIZE;
                } else {
#ifdef FLASH_ERASE_PAGE_ENABLE
                    if (remain_len >= NORFLASH_PAGE_SIZE) {
                        erase_len = NORFLASH_PAGE_SIZE;
                    }
#endif
                }
            } else {
                if (remain_len >= (NORFLASH_BLOCK_SIZE_32K * 2)) {
                    erase_len = (NORFLASH_BLOCK_SIZE_32K * 2);
                } else if (remain_len >= NORFLASH_BLOCK_SIZE_32K) {
                    erase_len = NORFLASH_BLOCK_SIZE_32K;
                } else if (remain_len >= NORFLASH_SECTOR_SIZE) {
                    erase_len = NORFLASH_SECTOR_SIZE;
                } else {
#ifdef FLASH_ERASE_PAGE_ENABLE
                    if (remain_len >= NORFLASH_PAGE_SIZE) {
                        erase_len = NORFLASH_PAGE_SIZE;
                    }
#endif
                }
            }

        if (erase_len == (NORFLASH_BLOCK_SIZE_64K)) {
            erase_type = GD25Q32C_CMD_BLOCK_ERASE_64K;
        } else if (erase_len == NORFLASH_BLOCK_SIZE_32K) {
            erase_type = GD25Q32C_CMD_BLOCK_ERASE_32K;
        } else if (erase_len == NORFLASH_SECTOR_SIZE) {
            erase_type = GD25Q32C_CMD_SECTOR_ERASE;
        }
#ifdef FLASH_ERASE_PAGE_ENABLE
        else if (erase_len == NORFLASH_PAGE_SIZE) {
            erase_type = PUYA_FLASH_CMD_PAGE_ERASE;
        }
#endif
        else {
            ASSERT(0, "%s: erase_len = 0", __func__);
        }

        _write_enable();

        cmd[0] = erase_type;
        cmd[1] = ((cur_addr >> 16) & 0xff);
        cmd[2] = ((cur_addr >> 8) & 0xff);
        cmd[3] = (cur_addr & 0xff);

        ret = _spi_send_word(cmd, sizeof(cmd), NULL, 0);
        if (ret) {
            SPIFLASH_TRACE(1, "%s: _spi_send_word failed!.ret = %d", __func__, ret);
            return SPI_NORFLASH_DRV_RET_SPI_ERROR;
        }

        cur_addr += erase_len;
        remain_len -= erase_len;

        _drv_busy_wait();
    } while (remain_len);

    return SPI_NORFLASH_DRV_RET_OK;
}

int32_t spi_norflash_drv_reset(void)
{
    int32_t ret;

    _drv_busy_wait();

    ret = _drv_reset_enable();
    if (ret) {
        return SPI_NORFLASH_DRV_RET_SPI_ERROR;
    }
    _drv_delay(40);
    ret = _drv_reset();
    if (ret) {
        return SPI_NORFLASH_DRV_RET_SPI_ERROR;
    }
    _drv_delay(40);
    spi_norflash_idx = 0;
    return SPI_NORFLASH_DRV_RET_OK;
}

