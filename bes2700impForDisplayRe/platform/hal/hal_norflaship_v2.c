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
#if (CHIP_FLASH_CTRL_VER >= 2)

#include "plat_types.h"
#include "plat_addr_map.h"

#ifdef FLASH_CTRL_BASE

#include "cmsis.h"
#include "hal_location.h"
#include "hal_norflaship.h"
#include "hal_timer.h"
#include "reg_norflaship_v2.h"
#ifdef PROGRAMMER
#include "task_schedule.h"
#else
#define TASK_SCHEDULE                           true
#endif

//====================================================================================
// Flash IP Operations
//====================================================================================

/*
 Supported Command List (Based on GD25Q32C):

 parameter WRSR1     = 8'h01 ;
 parameter PP        = 8'h02 ;
 parameter READ      = 8'h03 ;
 parameter WRDI      = 8'h04 ;
 parameter RDSR1     = 8'h05 ;
 parameter WREN      = 8'h06 ;
 parameter FREAD     = 8'h0B ;
 parameter WRSR3     = 8'h11 ;
 parameter RDSR3     = 8'h15 ;
 parameter SE        = 8'h20 ;
 parameter WRSR2     = 8'h31 ;
 parameter QPP       = 8'h32 ;
 parameter RDSR2     = 8'h35 ;
 parameter DOR       = 8'h3B ;
 parameter PSR       = 8'h42 ;
 parameter ESR       = 8'h44 ;
 parameter RSR       = 8'h48 ;
 parameter WEVSR     = 8'h50 ;
 parameter BE32      = 8'h52 ;
 parameter RSFDP     = 8'h5A ;
 parameter CE        = 8'h60 ;
 parameter RSTE      = 8'h66 ;
 parameter QOR       = 8'h6B ;
 parameter PES       = 8'h75 ;
 parameter SBW       = 8'h77 ;
 parameter PER       = 8'h7A ;
 parameter REMS      = 8'h90 ;
 parameter DREAD_ID  = 8'h92 ;
 parameter QREAD_ID  = 8'h94 ;
 parameter RST       = 8'h99 ;
 parameter RDID      = 8'h9F ;
 parameter HPM       = 8'hA3 ;
 parameter RDI       = 8'hAB ;
 parameter DP        = 8'hB9 ;
 parameter DIOR      = 8'hBB ;
 parameter BE64      = 8'hD8 ;
 parameter QIOWR     = 8'hE7 ;
 parameter QIOR      = 8'hEB ;
 parameter FPP       = 8'hF2 ;
*/

static struct NORFLASH_CTRL_T * const norflash[HAL_FLASH_ID_NUM] = {
    (struct NORFLASH_CTRL_T *)FLASH_CTRL_BASE,
#ifdef FLASH1_CTRL_BASE
    (struct NORFLASH_CTRL_T *)FLASH1_CTRL_BASE,
#endif
#ifdef FLASH2_CTRL_BASE
    (struct NORFLASH_CTRL_T *)FLASH2_CTRL_BASE,
#endif
};

BOOT_RODATA_FLASH_LOC
static struct NORFLASH_CTRL_T * const norflash_boot[HAL_FLASH_ID_NUM] = {
    (struct NORFLASH_CTRL_T *)FLASH_CTRL_BASE,
#ifdef FLASH1_CTRL_BASE
    (struct NORFLASH_CTRL_T *)FLASH1_CTRL_BASE,
#endif
#ifdef FLASH2_CTRL_BASE
    (struct NORFLASH_CTRL_T *)FLASH2_CTRL_BASE,
#endif
};

#if 0
static const uint8_t line_mode[8] = {
    NEW_CMD_LINE_1X, NEW_CMD_LINE_1X, NEW_CMD_LINE_2X, NEW_CMD_LINE_1X,
    NEW_CMD_LINE_4X, NEW_CMD_LINE_1X, NEW_CMD_LINE_1X, NEW_CMD_LINE_1X,
};
#endif

#ifdef CORE_SLEEP_POWER_DOWN
static struct NORFLASH_CTRL_T norflash_regs[HAL_FLASH_ID_NUM];
#endif

uint8_t norflaship_continuous_read_mode_bit(enum HAL_FLASH_ID_T id, uint8_t mode_bit)
{
    norflaship_busy_wait(id);
    norflash[id]->REG_004 = SET_BITFIELD(norflash[id]->REG_004, REG_004_MODEBIT, mode_bit);
    return 0;
}
uint8_t norflaship_continuous_read_off(enum HAL_FLASH_ID_T id)
{
    norflaship_busy_wait(id);
    norflash[id]->REG_004 &= ~REG_004_CONTINUOUS_MODE;
    return 0;
}
uint8_t norflaship_continuous_read_on(enum HAL_FLASH_ID_T id)
{
    norflaship_busy_wait(id);
    norflash[id]->REG_004 |= REG_004_CONTINUOUS_MODE;
    return 0;
}
static uint8_t norflaship_read_txfifo_empty_count(enum HAL_FLASH_ID_T id)
{
    return GET_BITFIELD(norflash[id]->REG_00C, REG_00C_TXFIFO_EMPCNT);
}
void norflaship_write_txfifo(enum HAL_FLASH_ID_T id, const uint8_t *val, uint32_t len)
{
    norflaship_blksize(id, len);
    while (norflaship_write_txfifo_cont(id, val, len) > 0);
}
uint32_t norflaship_write_txfifo_start(enum HAL_FLASH_ID_T id, const uint8_t *val, uint32_t len)
{
    norflaship_blksize(id, len);
    return norflaship_write_txfifo_cont(id, val, len);
}
uint32_t norflaship_write_txfifo_cont(enum HAL_FLASH_ID_T id, const uint8_t *val, uint32_t len)
{
    uint32_t avail = norflaship_read_txfifo_empty_count(id);
    while (avail > 0 && len > 0) {
#if 0
        if (len >= 4 && ((uint32_t)val & 3) == 0) {
            norflash[id]->REG_008.TXWORD = *(uint32_t *)val;
            val += 4;
            len -= 4;
        } else if (len >= 2 && ((uint32_t)val & 1) == 0) {
            norflash[id]->REG_008.TXHALFWORD = *(uint16_t *)val;
            val += 2;
            len -= 2;
        } else
#endif
        {
            norflash[id]->REG_008.TXBYTE = *(uint8_t *)val;
            val += 1;
            len -= 1;
        }
        avail -= 1;
    }
    return len;
}
#if (CHIP_FLASH_CTRL_VER >= 3) && defined(UNALIGNED_ACCESS)
uint32_t norflaship_write4_txfifo(enum HAL_FLASH_ID_T id, const uint8_t *val, uint32_t len)
{
    norflaship_blksize(id, len);
    return norflaship_write4_txfifo_cont(id, val, len);
}
uint32_t norflaship_write4_txfifo_cont(enum HAL_FLASH_ID_T id, const uint8_t *val, uint32_t len)
{
    const uint32_t *p32 = (uint32_t *)val;
    uint32_t avail = norflaship_read_txfifo_empty_count(id);
    while (avail > 0 && len >= 4) {
        norflash[id]->REG_008.TXWORD = *p32++;
        avail -= 1;
        len -= 4;
    }
    val = (const uint8_t *)p32;
    while (avail > 0 && len > 0) {
        norflash[id]->REG_008.TXBYTE = *val++;
        avail--;
        len--;
    }
    return len;
}
#endif
uint8_t norflaship_read_rxfifo_count(enum HAL_FLASH_ID_T id)
{
    return GET_BITFIELD(norflash[id]->REG_00C, REG_00C_RXFIFO_COUNT);
}
uint8_t norflaship_read_rxfifo(enum HAL_FLASH_ID_T id)
{
    return norflash[id]->REG_010;
}
void norflaship_blksize(enum HAL_FLASH_ID_T id, uint32_t blksize)
{
    norflash[id]->REG_004 = SET_BITFIELD(norflash[id]->REG_004, REG_004_BLOCK_SIZE, blksize);
}
void norflaship_cmd_addr(enum HAL_FLASH_ID_T id, uint8_t cmd, uint32_t address)
{
    norflaship_busy_wait(id);
    norflash[id]->REG_004 &= ~REG_004_NEW_CMD_EN;
    norflash[id]->REG_000 = REG_000_CMD(cmd) | REG_000_ADDR(address);
}
void norflaship_ext_tx_cmd(enum HAL_FLASH_ID_T id, uint8_t cmd, uint32_t tx_len)
{
    norflaship_busy_wait(id);
    norflash[id]->REG_004 = SET_BITFIELD(norflash[id]->REG_004, REG_004_BLOCK_SIZE, tx_len) | REG_004_NEW_CMD_EN;
    norflash[id]->REG_000 = REG_000_CMD(cmd);
}
void norflaship_ext_tx_cmd_4x(enum HAL_FLASH_ID_T id, uint8_t cmd, uint32_t tx_len)
{
    norflaship_busy_wait(id);
    norflash[id]->REG_004 = SET_BITFIELD(norflash[id]->REG_004, REG_004_BLOCK_SIZE, tx_len) | REG_004_NEW_CMD_EN;
    norflash[id]->REG_000 = REG_000_NEW_CMD_TX_LINE(NEW_CMD_LINE_4X) | REG_000_CMD(cmd);
}
void norflaship_ext_rx_cmd(enum HAL_FLASH_ID_T id, uint8_t cmd, uint32_t tx_len, uint32_t rx_len)
{
    norflaship_busy_wait(id);
    norflash[id]->REG_004 = SET_BITFIELD(norflash[id]->REG_004, REG_004_BLOCK_SIZE, tx_len) | REG_004_NEW_CMD_EN;
    norflash[id]->REG_000 = REG_000_CMD(cmd) | REG_000_NEW_CMD_RX_EN | REG_000_NEW_CMD_RX_LEN(rx_len);
}
void norflaship_ext_rx_cmd_4x(enum HAL_FLASH_ID_T id, uint8_t cmd, uint32_t tx_len, uint32_t rx_len)
{
    norflaship_busy_wait(id);
    norflash[id]->REG_004 = SET_BITFIELD(norflash[id]->REG_004, REG_004_BLOCK_SIZE, tx_len) | REG_004_NEW_CMD_EN;
    norflash[id]->REG_000 = REG_000_NEW_CMD_RX_LINE(NEW_CMD_LINE_4X) | REG_000_NEW_CMD_TX_LINE(NEW_CMD_LINE_4X) |
        REG_000_CMD(cmd) | REG_000_NEW_CMD_RX_EN | REG_000_NEW_CMD_RX_LEN(rx_len);
}
void norflaship_ext_rx_cmd_rx_4x(enum HAL_FLASH_ID_T id, uint8_t cmd, uint32_t tx_len, uint32_t rx_len)
{
    norflaship_busy_wait(id);
    norflash[id]->REG_004 = SET_BITFIELD(norflash[id]->REG_004, REG_004_BLOCK_SIZE, tx_len) | REG_004_NEW_CMD_EN;
    norflash[id]->REG_000 = REG_000_NEW_CMD_RX_LINE(NEW_CMD_LINE_4X) |
        REG_000_CMD(cmd) | REG_000_NEW_CMD_RX_EN | REG_000_NEW_CMD_RX_LEN(rx_len);
}
void norflaship_cmd_done(enum HAL_FLASH_ID_T id)
{
    norflaship_busy_wait(id);
    norflash[id]->REG_004 &= ~REG_004_NEW_CMD_EN;
}
void norflaship_rxfifo_count_wait(enum HAL_FLASH_ID_T id, uint8_t cnt)
{
    while ((norflaship_read_rxfifo_count(id) < cnt) && TASK_SCHEDULE);
}
void norflaship_rxfifo_empty_wait(enum HAL_FLASH_ID_T id)
{
    while ((norflash[id]->REG_00C & REG_00C_RXFIFO_EMPTY) && TASK_SCHEDULE);
}
int norflaship_txfifo_is_full(enum HAL_FLASH_ID_T id)
{
    return !!(norflash[id]->REG_00C & REG_00C_TXFIFO_FULL);
}
void norflaship_busy_wait(enum HAL_FLASH_ID_T id)
{
#ifdef ROM_BUILD
#define NORFLASH_ROM_OPER_TIMEOUT           MS_TO_TICKS(5)
    uint32_t time;

    time = hal_sys_timer_get();
    while ((norflash[id]->REG_00C & REG_00C_BUSY) && TASK_SCHEDULE &&
            (hal_sys_timer_get() - time) < NORFLASH_ROM_OPER_TIMEOUT);
#else
    while ((norflash[id]->REG_00C & REG_00C_BUSY) && TASK_SCHEDULE);
#endif
}
int norflaship_is_busy(enum HAL_FLASH_ID_T id)
{
    return !!(norflash[id]->REG_00C & REG_00C_BUSY);
}
void norflaship_clear_fifos(enum HAL_FLASH_ID_T id)
{
    norflaship_busy_wait(id);
    norflash[id]->REG_018 = REG_018_TXFIFOCLR | REG_018_RXFIFOCLR;
    norflaship_busy_wait(id);
}
void norflaship_clear_rxfifo(enum HAL_FLASH_ID_T id)
{
    norflaship_busy_wait(id);
    norflash[id]->REG_018 = REG_018_RXFIFOCLR;
    norflaship_busy_wait(id);
}
void norflaship_clear_txfifo(enum HAL_FLASH_ID_T id)
{
    norflaship_busy_wait(id);
    norflash[id]->REG_018 = REG_018_TXFIFOCLR;
    norflaship_busy_wait(id);
}
void norflaship_div(enum HAL_FLASH_ID_T id, uint32_t div)
{
    norflaship_busy_wait(id);
#ifdef CHIP_BEST2300P
    norflash[id]->REG_014 = SET_BITFIELD(norflash[id]->REG_014, REG_014_CLKDIV, div) | EXTRA_TCHSH_EN_O;
#else
    norflash[id]->REG_014 = SET_BITFIELD(norflash[id]->REG_014, REG_014_CLKDIV, div);
#endif
}
uint32_t norflaship_get_div(enum HAL_FLASH_ID_T id)
{
    return GET_BITFIELD(norflash[id]->REG_014, REG_014_CLKDIV);
}
uint32_t norflaship_get_samdly(enum HAL_FLASH_ID_T id)
{
    return GET_BITFIELD(norflash[id]->REG_014, REG_014_SAMPLESEL);
}
void norflaship_samdly(enum HAL_FLASH_ID_T id, uint32_t v)
{
    norflaship_busy_wait(id);
#ifdef CHIP_BEST2300P
    norflash[id]->REG_014 = SET_BITFIELD(norflash[id]->REG_014, REG_014_SAMPLESEL, v) | EXTRA_TCHSH_EN_O;
#else
    norflash[id]->REG_014 = SET_BITFIELD(norflash[id]->REG_014, REG_014_SAMPLESEL, v);
#endif
}
void norflaship_dual_mode(enum HAL_FLASH_ID_T id, uint32_t v)
{
    norflaship_busy_wait(id);
#ifdef CHIP_BEST2300P
    if (v) {
        norflash[id]->REG_014 |= REG_014_RAM_DUALMODE | EXTRA_TCHSH_EN_O;
    } else {
        norflash[id]->REG_014 = (norflash[id]->REG_014 & ~REG_014_RAM_DUALMODE) | EXTRA_TCHSH_EN_O;
    }
#else
    if (v) {
        norflash[id]->REG_014 |= REG_014_RAM_DUALMODE;
    } else {
        norflash[id]->REG_014 &= ~REG_014_RAM_DUALMODE;
    }
#endif
}
void norflaship_hold_pin(enum HAL_FLASH_ID_T id, uint32_t v)
{
#ifdef CHIP_BEST2300P
    if (v) {
        norflash[id]->REG_014 |= REG_014_HOLDPIN | EXTRA_TCHSH_EN_O;
    } else {
        norflash[id]->REG_014 = (norflash[id]->REG_014 & ~REG_014_HOLDPIN) | EXTRA_TCHSH_EN_O;
    }
#else
    if (v) {
        norflash[id]->REG_014 |= REG_014_HOLDPIN;
    } else {
        norflash[id]->REG_014 &= ~REG_014_HOLDPIN;
    }
#endif
}
void norflaship_wpr_pin(enum HAL_FLASH_ID_T id, uint32_t v)
{
#ifdef CHIP_BEST2300P
    if (v) {
        norflash[id]->REG_014 |= REG_014_WPROPIN | EXTRA_TCHSH_EN_O;
    } else {
        norflash[id]->REG_014 = (norflash[id]->REG_014 & ~REG_014_WPROPIN) | EXTRA_TCHSH_EN_O;
    }
#else
    if (v) {
        norflash[id]->REG_014 |= REG_014_WPROPIN;
    } else {
        norflash[id]->REG_014 &= ~REG_014_WPROPIN;
    }
#endif
}
void norflaship_reset_pin(enum HAL_FLASH_ID_T id, uint32_t v)
{
    if (v) {
        norflash[id]->REG_014 |= REG_014_RESETPIN;
    } else {
        norflash[id]->REG_014 &= ~REG_014_RESETPIN;
    }
}
void norflaship_quad_mode(enum HAL_FLASH_ID_T id, uint32_t v)
{
    norflaship_busy_wait(id);
#ifdef CHIP_BEST2300P
    if (v) {
        norflash[id]->REG_014 |= REG_014_RAM_QUADMODE | EXTRA_TCHSH_EN_O;
    } else {
        norflash[id]->REG_014 = (norflash[id]->REG_014 & ~REG_014_RAM_QUADMODE) | EXTRA_TCHSH_EN_O;
    }
#else
    if (v) {
        norflash[id]->REG_014 |= REG_014_RAM_QUADMODE;
    } else {
        norflash[id]->REG_014 &= ~REG_014_RAM_QUADMODE;
    }
#endif
}
void norflaship_4byteaddr_mode(enum HAL_FLASH_ID_T id, uint32_t v)
{
#ifdef CHIP_BEST2300P
    if (v) {
        norflash[id]->REG_014 |= REG_014_FOUR_BYTE_ADDR_EN | EXTRA_TCHSH_EN_O;
    } else {
        norflash[id]->REG_014 = (norflash[id]->REG_014 & ~REG_014_FOUR_BYTE_ADDR_EN) | EXTRA_TCHSH_EN_O;
    }
#else
    if (v) {
        norflash[id]->REG_014 |= REG_014_FOUR_BYTE_ADDR_EN;
    } else {
        norflash[id]->REG_014 &= ~REG_014_FOUR_BYTE_ADDR_EN;
    }
#endif
}
void norflaship_ext_addr(enum HAL_FLASH_ID_T id, uint32_t v)
{
    norflash[id]->REG_030 = REG_030_ADDR_31_24(v >> 24);
}
void norflaship_nand_sel(enum HAL_FLASH_ID_T id, uint32_t v)
{
    if (v) {
        norflash[id]->REG_01C |= REG_01C_NAND_SEL;
    } else {
        norflash[id]->REG_01C &= ~REG_01C_NAND_SEL;
    }
}
void norflaship_dmactrl_tx_en(enum HAL_FLASH_ID_T id, uint32_t v)
{
    if (v) {
        norflash[id]->REG_01C |= REG_01C_DMACTRL_TX_EN;
    } else {
        norflash[id]->REG_01C &= ~REG_01C_DMACTRL_TX_EN;
    }
}
void norflaship_dmactrl_rx_en(enum HAL_FLASH_ID_T id, uint32_t v)
{
    if (v) {
        norflash[id]->REG_01C |= REG_01C_DMACTRL_RX_EN;
    } else {
        norflash[id]->REG_01C &= ~REG_01C_DMACTRL_RX_EN;
    }
}
void norflaship_dma_rx_threshold(enum HAL_FLASH_ID_T id, uint32_t v)
{
    norflash[id]->REG_01C = SET_BITFIELD(norflash[id]->REG_01C, REG_01C_RX_THRESHOLD, v);
}
void norflaship_dma_tx_threshold(enum HAL_FLASH_ID_T id, uint32_t v)
{
    norflash[id]->REG_01C = SET_BITFIELD(norflash[id]->REG_01C, REG_01C_TX_THRESHOLD, v);
}
void norflaship_dma_rx_size(enum HAL_FLASH_ID_T id, uint32_t v)
{
    norflash[id]->REG_01C = SET_BITFIELD(norflash[id]->REG_01C, REG_01C_DMA_RX_SIZE, v);
}
void norflaship_ruen(enum HAL_FLASH_ID_T id, uint32_t v)
{
    norflash[id]->REG_034 = SET_BITFIELD(norflash[id]->REG_034, REG_034_SPI_RUEN, v);
}
void norflaship_rden(enum HAL_FLASH_ID_T id, uint32_t v)
{
    norflash[id]->REG_034 = SET_BITFIELD(norflash[id]->REG_034, REG_034_SPI_RDEN, v);
}
void norflaship_dualiocmd(enum HAL_FLASH_ID_T id, uint32_t v)
{
    norflash[id]->REG_020 = SET_BITFIELD(norflash[id]->REG_020, REG_020_DUALCMD, v);
}
void norflaship_rdcmd(enum HAL_FLASH_ID_T id, uint32_t v)
{
    norflash[id]->REG_020 = SET_BITFIELD(norflash[id]->REG_020, REG_020_READCMD, v);
}
void norflaship_frdcmd(enum HAL_FLASH_ID_T id, uint32_t v)
{
    norflash[id]->REG_020 = SET_BITFIELD(norflash[id]->REG_020, REG_020_FREADCMD, v);
}
void norflaship_qrdcmd(enum HAL_FLASH_ID_T id, uint32_t v)
{
    norflash[id]->REG_020 = SET_BITFIELD(norflash[id]->REG_020, REG_020_QUADCMD, v);
}
uint32_t norflaship_get_rdcmd(enum HAL_FLASH_ID_T id)
{
    return GET_BITFIELD(norflash[id]->REG_020, REG_020_READCMD);
}
void norflaship_set_idle_io_dir(enum HAL_FLASH_ID_T id, uint32_t v)
{
#ifdef NORFLASHIP_HAS_IDLE_IO_CTRL
    norflash[id]->REG_034 = SET_BITFIELD(norflash[id]->REG_034, REG_034_SPI_IOEN, v);
#endif
}

void norflaship_sleep(enum HAL_FLASH_ID_T id)
{
#ifndef NORFLASHIP_HAS_IDLE_IO_CTRL
    norflash[id]->REG_034 |= REG_034_SPI_IORES(1 << 2);
#endif
#ifdef CORE_SLEEP_POWER_DOWN
    volatile uint32_t *src, *dst;
    int i;

    src = (volatile uint32_t *)norflash[id];
    dst = (volatile uint32_t *)&norflash_regs;
    for (i = 1; i < sizeof(struct NORFLASH_CTRL_T) / 4; i++) {
       dst[i] = src[i];
    }
#endif
}

void norflaship_wakeup(enum HAL_FLASH_ID_T id)
{
#if CORE_SLEEP_POWER_DOWN
    volatile uint32_t *src, *dst;
    int i;

    src = (volatile uint32_t *)&norflash_regs;
    dst = (volatile uint32_t *)norflash[id];
    for (i = 1; i < sizeof(struct NORFLASH_CTRL_T) / 4; i++) {
        if (&dst[i] == &norflash[id]->REG_008.TXWORD) {
            continue;
        }
#ifdef CHIP_BEST1501
        if (&dst[i] == &norflash[id]->REG_02C) {
            continue;
        }
#endif
        dst[i] = src[i];
    }
#endif
#ifndef NORFLASHIP_HAS_IDLE_IO_CTRL
    norflash[id]->REG_034 &= ~REG_034_SPI_IORES(1 << 2);
#endif
}

void norflaship_dec_index(enum HAL_FLASH_ID_T id, uint32_t idx)
{
#ifdef NORFLASHIP_HAS_SECURITY
    norflash[id]->REG_058 = SET_BITFIELD(norflash[id]->REG_058, REG_058_IDX, idx);
#endif
}

void norflaship_dec_saddr(enum HAL_FLASH_ID_T id, uint32_t addr)
{
#ifdef NORFLASHIP_HAS_SECURITY
    norflash[id]->REG_060 =  ((addr & 0x000000ff) << 24)
                        |((addr & 0x0000ff00) << 8)
                        |((addr & 0x00ff0000) >> 8)
                        |((addr & 0xff000000) >> 24);
#endif
}

void norflaship_dec_eaddr(enum HAL_FLASH_ID_T id, uint32_t addr)
{
#ifdef NORFLASHIP_HAS_SECURITY
    norflash[id]->REG_064 =  ((addr & 0x000000ff) << 24)
                        |((addr & 0x0000ff00) << 8)
                        |((addr & 0x00ff0000) >> 8)
                        |((addr & 0xff000000) >> 24);
#endif
}

void norflaship_dec_enable(enum HAL_FLASH_ID_T id)
{
#ifdef NORFLASHIP_HAS_SECURITY
    norflash[id]->REG_06C |= REG_06C_DEC_ENABLE;
#endif
}

void norflaship_dec_disable(enum HAL_FLASH_ID_T id)
{
#ifdef NORFLASHIP_HAS_SECURITY
    norflash[id]->REG_06C &= ~REG_06C_DEC_ENABLE;
#endif
}

void norflaship_man_wrap_width(enum HAL_FLASH_ID_T id, uint32_t width)
{
#ifdef NORFLASHIP_HAS_SECURITY
    uint32_t bits = 0;

    if (width == 8)
        bits = 0;
    else if (width == 16)
        bits = 1;
    else if (width == 32)
        bits = 2;
    else if (width == 64)
        bits = 3;
    else
        bits = 2;
    norflash[id]->REG_038 = SET_BITFIELD(norflash[id]->REG_038, REG_038_MAN_WRAP_BITS, bits);
#endif
}

void norflaship_man_wrap_enable(enum HAL_FLASH_ID_T id)
{
#ifdef NORFLASHIP_HAS_SECURITY
    norflash[id]->REG_038 |= REG_038_MAN_WRAP_ENABLE;
#endif
}

void norflaship_man_wrap_disable(enum HAL_FLASH_ID_T id)
{
#ifdef NORFLASHIP_HAS_SECURITY
    norflash[id]->REG_038 &= ~REG_038_MAN_WRAP_ENABLE;
#endif
}

void norflaship_auto_wrap_cmd(enum HAL_FLASH_ID_T id, uint32_t cmd)
{
#ifdef NORFLASHIP_HAS_SECURITY
    norflash[id]->REG_038 = SET_BITFIELD(norflash[id]->REG_038, REG_038_AUTO_WRAP_CMD, cmd);
#endif
}

void norflaship_man_mode_enable(enum HAL_FLASH_ID_T id)
{
#ifdef NORFLASHIP_HAS_SECURITY
    norflash[id]->REG_038 |= REG_038_WRAP_MODE_SEL;
#endif
}

void norflaship_man_mode_disable(enum HAL_FLASH_ID_T id)
{
#ifdef NORFLASHIP_HAS_SECURITY
    norflash[id]->REG_038 &= ~REG_038_WRAP_MODE_SEL;
#endif
}

__STATIC_FORCEINLINE
void _reset_remap(struct NORFLASH_CTRL_T *ctrl)
{
#ifdef NORFLASHIP_HAS_REMAP
    ctrl->REG_0B0 &= ~(REG_0B0_GLB_REMAP_EN | REG_0B0_ADDR0_REMAP_EN | REG_0B0_ADDR1_REMAP_EN | REG_0B0_ADDR2_REMAP_EN | REG_0B0_ADDR3_REMAP_EN);
#endif
}

void norflaship_reset_remap(enum HAL_FLASH_ID_T id)
{
    _reset_remap(norflash[id]);
}

BOOT_TEXT_FLASH_LOC
void norflaship_boot_reset_remap(enum HAL_FLASH_ID_T id)
{
    _reset_remap(norflash_boot[id]);
}

__STATIC_FORCEINLINE
int _config_remap_section(struct NORFLASH_CTRL_T *ctrl, uint32_t index, uint32_t addr, uint32_t len, uint32_t new_addr)
{
#ifdef NORFLASHIP_HAS_REMAP
    __IO uint32_t *start, *end, *to;

    if (index >= 4) {
        return 1;
    }
    if (len == 0) {
        ctrl->REG_0B0 &= ~(REG_0B0_ADDR0_REMAP_EN << index);
        return 0;
    }
    if (addr & (NORFLASH_SECTOR_SIZE - 1)) {
        return 2;
    }
    if (new_addr & (NORFLASH_SECTOR_SIZE - 1)) {
        return 3;
    }
    if (len < NORFLASH_SECTOR_SIZE) {
        return 4;
    }

#ifdef CHIP_BEST2300P
    if (len & (len - 1)) {
        return 5;
    }

    uint32_t len_idx;

    len_idx = __CLZ(__RBIT(len)) - 9;

    ctrl->REG_0A0 = (ctrl->REG_0A0 & ~(REG_0A0_LEN_WIDTH0_MASK << (index * REG_0A0_LEN_WIDTH1_SHIFT))) |
        (REG_0A0_LEN_WIDTH0(len_idx) << (index * REG_0A0_LEN_WIDTH1_SHIFT));
#else
    if (len & (NORFLASH_SECTOR_SIZE - 1)) {
        return 5;
    }
#endif

    start = &ctrl->REG_070 + index;
    end = &ctrl->REG_080 + index;
    to = &ctrl->REG_090 + index;

    *start = addr;
    *end = addr + len - 1;
    *to = new_addr;

#ifdef CHIP_BEST2300P
    ctrl->REG_0B0 |= (REG_0B0_ADDR0_REMAP_EN << index);
#endif

    return 0;
#else
    return -1;
#endif
}

int norflaship_config_remap_section(enum HAL_FLASH_ID_T id, uint32_t index, uint32_t addr, uint32_t len, uint32_t new_addr)
{
    return _config_remap_section(norflash[id], index, addr, len, new_addr);
}

BOOT_TEXT_FLASH_LOC
int norflaship_boot_config_remap_section(enum HAL_FLASH_ID_T id, uint32_t index, uint32_t addr, uint32_t len, uint32_t new_addr)
{
    return _config_remap_section(norflash_boot[id], index, addr, len, new_addr);
}

__STATIC_FORCEINLINE
void _enable_remap(struct NORFLASH_CTRL_T *ctrl, uint32_t index)
{
#ifdef NORFLASHIP_HAS_REMAP
#ifdef CHIP_BEST2300P
    ctrl->REG_0B0 |= REG_0B0_GLB_REMAP_EN;
#else
    ctrl->REG_0B0 |= REG_0B0_GLB_REMAP_EN | (REG_0B0_ADDR0_REMAP_EN << index);
#endif
#endif
}

void norflaship_enable_remap(enum HAL_FLASH_ID_T id, uint32_t index)
{
    _enable_remap(norflash[id], index);
}

BOOT_TEXT_FLASH_LOC
void norflaship_boot_enable_remap(enum HAL_FLASH_ID_T id, uint32_t index)
{
    _enable_remap(norflash_boot[id], index);
}

__STATIC_FORCEINLINE
void _disable_remap(struct NORFLASH_CTRL_T *ctrl, uint32_t index)
{
#ifdef NORFLASHIP_HAS_REMAP
#ifdef CHIP_BEST2300P
    ctrl->REG_0B0 &= ~REG_0B0_GLB_REMAP_EN;
#else
    ctrl->REG_0B0 &= ~(REG_0B0_ADDR0_REMAP_EN << index);
#endif
#endif
}

void norflaship_disable_remap(enum HAL_FLASH_ID_T id, uint32_t index)
{
    _disable_remap(norflash[id], index);
}

BOOT_TEXT_FLASH_LOC
void norflaship_boot_disable_remap(enum HAL_FLASH_ID_T id, uint32_t index)
{
    _disable_remap(norflash_boot[id], index);
}

__STATIC_FORCEINLINE
int _get_remap_status(struct NORFLASH_CTRL_T *ctrl, uint32_t index)
{
#ifdef NORFLASHIP_HAS_REMAP
#ifdef CHIP_BEST2300P
    return (ctrl->REG_0B0 & REG_0B0_GLB_REMAP_EN) ? true : false;
#else
    return (ctrl->REG_0B0 & (REG_0B0_ADDR0_REMAP_EN << index)) ? true : false;
#endif
#else
    return false;
#endif
}

int norflaship_get_remap_status(enum HAL_FLASH_ID_T id, uint32_t index)
{
    return _get_remap_status(norflash[id], index);
}

BOOT_TEXT_FLASH_LOC
int norflaship_boot_get_remap_status(enum HAL_FLASH_ID_T id, uint32_t index)
{
    return _get_remap_status(norflash_boot[id], index);
}

#if (CHIP_FLASH_CTRL_VER >= 3)
void norflaship_qpi_mode(enum HAL_FLASH_ID_T id, uint32_t v)
{
    norflaship_busy_wait(id);
    if (v) {
        norflash[id]->REG_040 |= REG_040_QPI_MODE;
    } else {
        norflash[id]->REG_040 &= ~REG_040_QPI_MODE;
    }
}

void norflaship_dtr_mode(enum HAL_FLASH_ID_T id, uint32_t v)
{
    norflaship_busy_wait(id);
#if (CHIP_FLASH_CTRL_VER == 3)
    if (v) {
        norflash[id]->REG_040 |= REG_040_DTR_MODE;
    } else {
        norflash[id]->REG_040 &= ~REG_040_DTR_MODE;
    }
#elif (CHIP_FLASH_CTRL_VER >= 4)
    if (v) {
        norflash[id]->REG_040 |= (REG_040_DTR_ADDR_MODE | REG_040_DTR_OTHER_MODE);
    } else {
        norflash[id]->REG_040 &= ~(REG_040_DTR_ADDR_MODE | REG_040_DTR_OTHER_MODE);
    }
#endif
}

void norflaship_dummy_qior(enum HAL_FLASH_ID_T id, uint32_t v)
{
    norflash[id]->REG_108 = REG_108_DUMMY_QIOR(v);
}

void norflaship_dummy_dtr4rd(enum HAL_FLASH_ID_T id, uint32_t v)
{
    norflash[id]->REG_12C = REG_12C_DUMMY_DTR4RD(v);
}

void norflaship_dummy_others(enum HAL_FLASH_ID_T id, uint32_t v)
{
    norflash[id]->REG_150 = REG_150_DUMMY_OTHERS(v);
}
#endif

#if (CHIP_FLASH_CTRL_VER >= 5)
void norflaship_nand_addr_byte_num(enum HAL_FLASH_ID_T id, uint32_t v)
{
    norflash[id]->REG_044 = REG_044_NAND_ADDR_BYTE_NUM(v);
}

void norflaship_dummy_n_rcqio(enum HAL_FLASH_ID_T id, uint32_t v)
{
    norflash[id]->REG_164 = REG_164_DUMMY_N_RCQIO(v);
}
#endif

void norflaship_fetch_disable(enum HAL_FLASH_ID_T id)
{
    norflash[id]->REG_02C &= ~REG_02C_FETCH_EN;
}

#if (CHIP_FLASH_CTRL_VER >= 3)
void norflaship_cacheable_enable(enum HAL_FLASH_ID_T id)
{
    uint32_t val;

    val = REG_02C_CACHEABLE_EN;
#if (CHIP_FLASH_CTRL_VER <= 6)
    // CSN_PIN_REG cannot be read and should be set to its default value
    val |= REG_02C_CSN_PIN_REG_MASK;
#endif
    norflash[id]->REG_02C |= val;
}
#endif

#if (CHIP_FLASH_CTRL_VER >= 4)
uint32_t norflaship_mem_read_bus_lock(enum HAL_FLASH_ID_T id)
{
    uint32_t status;

    status = (norflash[id]->REG_034 & REG_034_MEM_READ_BUS_LOCK);
    if (status == 0) {
        norflash[id]->REG_034 |= REG_034_MEM_READ_BUS_LOCK;
#if (CHIP_FLASH_CTRL_VER <= 5)
        // A race condition still exists, and is fixed in version 6
#endif
        norflaship_busy_wait(id);
    }
    return status;
}

void norflaship_mem_read_bus_unlock(enum HAL_FLASH_ID_T id, uint32_t status)
{
    if (status == 0) {
        norflaship_busy_wait(id);
#if (CHIP_FLASH_CTRL_VER <= 5)
        if (norflash[id]->REG_03C & REG_03C_MEM_READ_IN_HOLD) {
            norflash[id]->REG_034 |= REG_034_MEM_READ_CONT_EN;
            norflash[id]->REG_014 = norflash[id]->REG_014;
            while ((norflash[id]->REG_03C & REG_03C_MEM_READ_BUS_IDLE) == 0);
            norflaship_busy_wait(id);
            norflash[id]->REG_034 &= ~(REG_034_MEM_READ_CONT_EN | REG_034_MEM_READ_BUS_LOCK);
        } else
#endif
        {
            norflash[id]->REG_034 &= ~REG_034_MEM_READ_BUS_LOCK;
        }
    }
}

int norflaship_mem_read_pending(enum HAL_FLASH_ID_T id)
{
    return !!(norflash[id]->REG_03C & REG_03C_MEM_READ_IN_HOLD);
}
#endif

#if (CHIP_FLASH_CTRL_VER >= 5)
void norflaship_cs_switch(enum HAL_FLASH_ID_T id, uint32_t v)
{
    if (v) {
        norflash[id]->REG_200 |= REG_200_REG_CS_SWITCH;
    } else {
        norflash[id]->REG_200 &= ~REG_200_REG_CS_SWITCH;
    }
}

void norflaship_cs0_cap_size(enum HAL_FLASH_ID_T id, uint32_t size)
{
    uint32_t v;

    if (size == 4 * 1024 * 1024) {
        v = 0;
    } else if (size == 8 * 1024 * 1024) {
        v = 1;
    } else if (size == 16 * 1024 * 1024) {
        v = 2;
    } else if (size == 32 * 1024 * 1024) {
        v = 3;
    } else {
        // size == 64M
        v = 4;
    }
    norflash[id]->REG_200 = SET_BITFIELD(norflash[id]->REG_200, REG_200_REG_CS0_CAP_POI, v);
}

void norflaship_cs0_manual_sel_en(enum HAL_FLASH_ID_T id, uint32_t v)
{
    if (v) {
        norflash[id]->REG_200 |= REG_200_REG_CS_MASK_EN(1 << 0);
    } else {
        norflash[id]->REG_200 &= ~REG_200_REG_CS_MASK_EN(1 << 0);
    }
}

void norflaship_cs1_manual_sel_en(enum HAL_FLASH_ID_T id, uint32_t v)
{
    if (v) {
        norflash[id]->REG_200 |= REG_200_REG_CS_MASK_EN(1 << 1);
    } else {
        norflash[id]->REG_200 &= ~REG_200_REG_CS_MASK_EN(1 << 1);
    }
}

void norflaship_cs_manual(enum HAL_FLASH_ID_T id, uint32_t v)
{
    if (v) {
        norflash[id]->REG_200 |= REG_200_REG_CS_MASK_MANUAL;
    } else {
        norflash[id]->REG_200 &= ~REG_200_REG_CS_MASK_MANUAL;
    }
}

void norflaship_tx_data_copy(enum HAL_FLASH_ID_T id, uint32_t v)
{
    if (v) {
        norflash[id]->REG_204 |= REG_204_TX_DATA_COPY;
    } else {
        norflash[id]->REG_204 &= ~REG_204_TX_DATA_COPY;
    }
}

void norflaship_rx_data_reorder(enum HAL_FLASH_ID_T id, uint32_t v)
{
    if (v) {
        norflash[id]->REG_204 |= REG_204_RX_DATA_REORDER;
    } else {
        norflash[id]->REG_204 &= ~REG_204_RX_DATA_REORDER;
    }
}

void norflaship_dual_chip_mode(enum HAL_FLASH_ID_T id, uint32_t v)
{
    norflaship_busy_wait(id);
    if (v) {
        norflash[id]->REG_204 |= REG_204_DUAL_CHIP_MODE;
    } else {
        norflash[id]->REG_204 &= ~REG_204_DUAL_CHIP_MODE;
    }
}

int norflaship_get_dual_chip_mode(enum HAL_FLASH_ID_T id)
{
    return !!(norflash[id]->REG_204 & REG_204_DUAL_CHIP_MODE);
}

void norflaship_cfg_rx_fifo_width(enum HAL_FLASH_ID_T id, uint32_t size)
{
    uint32_t v;

    if (size == 1) {
        v = 0;
    } else if (size == 2) {
        v = 1;
    } else {
        // size == 4
        v = 2;
    }
    norflash[id]->REG_204 = SET_BITFIELD(norflash[id]->REG_204, REG_204_CFG_RXSIZE, v);
}

void norflaship_cs1_reset_pin(enum HAL_FLASH_ID_T id, uint32_t v)
{
    if (v) {
        norflash[id]->REG_204 |= REG_204_RESET_PIN1;
    } else {
        norflash[id]->REG_204 &= ~REG_204_RESET_PIN1;
    }
}

void norflaship_cs1_wpr_pin(enum HAL_FLASH_ID_T id, uint32_t v)
{
    if (v) {
        norflash[id]->REG_204 |= REG_204_WPROTECT_PIN1;
    } else {
        norflash[id]->REG_204 &= ~REG_204_WPROTECT_PIN1;
    }
}

void norflaship_cs1_hold_pin(enum HAL_FLASH_ID_T id, uint32_t v)
{
    if (v) {
        norflash[id]->REG_204 |= REG_204_HOLD_PIN1;
    } else {
        norflash[id]->REG_204 &= ~REG_204_HOLD_PIN1;
    }
}

void norflaship_cs1_div(enum HAL_FLASH_ID_T id, uint32_t div)
{
    norflaship_busy_wait(id);
    norflash[id]->REG_204 = SET_BITFIELD(norflash[id]->REG_204, REG_204_CLK_DIVIDER1, div);
}

uint32_t norflaship_cs1_get_div(enum HAL_FLASH_ID_T id)
{
    return GET_BITFIELD(norflash[id]->REG_204, REG_204_CLK_DIVIDER1);
}

void norflaship_boundary_split(enum HAL_FLASH_ID_T id, uint32_t v)
{
    if (v) {
        norflash[id]->REG_204 |= REG_204_BOUNDARY_SPLIT_ENABLE;
    } else {
        norflash[id]->REG_204 &= ~REG_204_BOUNDARY_SPLIT_ENABLE;
    }
}

uint32_t norflaship_cs1_get_samdly(enum HAL_FLASH_ID_T id)
{
    return GET_BITFIELD(norflash[id]->REG_204, REG_204_SAMPLESEL1);
}

void norflaship_cs1_samdly(enum HAL_FLASH_ID_T id, uint32_t v)
{
    norflaship_busy_wait(id);
    norflash[id]->REG_204 = SET_BITFIELD(norflash[id]->REG_204, REG_204_SAMPLESEL1, v);
}

void norflaship_cs1_4byteaddr_mode(enum HAL_FLASH_ID_T id, uint32_t v)
{
    if (v) {
        norflash[id]->REG_204 |= REG_204_FOUR_BYTE_ADDR_EN1;
    } else {
        norflash[id]->REG_204 &= ~REG_204_FOUR_BYTE_ADDR_EN1;
    }
}

#if (CHIP_FLASH_CTRL_VER >= 7)
void norflaship_fetch_64_bytes(enum HAL_FLASH_ID_T id, uint32_t v)
{
    if (v) {
        norflash[id]->REG_204 |= REG_204_FETCH_64B;
    } else {
        norflash[id]->REG_204 &= ~REG_204_FETCH_64B;
    }
}
#endif

void norflaship_write_lock(enum HAL_FLASH_ID_T id)
{
    norflash[id]->REG_220 = 0xCAFE0000;
}

void norflaship_write_unlock(enum HAL_FLASH_ID_T id)
{
#if (CHIP_FLASH_CTRL_VER >= 7)
    norflash[id]->REG_220 = 0xCAFE0001;
#else
    norflash[id]->REG_224 = 0xCAFE0001;
#endif
}
#endif

#if (CHIP_FLASH_CTRL_VER >= 6)
/**
 * norflaship_set_transsize_control -  select control modes of how to stop flash write process
 *  @use_tx_blksize : 0, use_tx_blksize; 1, stop with trans_cmp_en bit of TRANSSIZECTRL register
 */
void norflaship_set_transsize_control(enum HAL_FLASH_ID_T id, bool use_tx_blksize)
{
    if (use_tx_blksize) {
       norflash[id]->REG_048 &= ~REG_048_TRANS_SIZE_CTRL;
       norflash[id]->REG_048 |= REG_048_TRANS_CMP_EN;
    } else {
       norflash[id]->REG_048 |= REG_048_TRANS_SIZE_CTRL;
    }
}

/**
 * norflaship_set_transcmp - set transcmp stop flag in dma completed handler
 *  clear transcmp stop flag before start to write flash
*/
void norflaship_set_transcmp(enum HAL_FLASH_ID_T id, bool completed)
{
    if (completed) {
       while (!(norflash[id]->REG_00C & REG_00C_TXFIFO_EMPTY));
       norflash[id]->REG_048 |= REG_048_TRANS_CMP_EN;
    } else {
       norflash[id]->REG_048 &= ~REG_048_TRANS_CMP_EN;
    }
}
#endif

#if (CHIP_FLASH_CTRL_VER >= 7)
void norflaship_clock_gating_enable(enum HAL_FLASH_ID_T id)
{
    norflash[id]->REG_014 &= ~(REG_014_MODE_SCLK_ACCESS | REG_014_MODE_HCLK_ACCESS | REG_014_MODE_HCLK_REG_ACCESS);
}

void norflaship_clock_gating_disable(enum HAL_FLASH_ID_T id)
{
    norflash[id]->REG_014 |= REG_014_MODE_SCLK_ACCESS | REG_014_MODE_HCLK_ACCESS | REG_014_MODE_HCLK_REG_ACCESS;
}

void norflaship_set_rclk_dly(enum HAL_FLASH_ID_T id, uint32_t map, uint32_t delay)
{
    uint32_t mask = 0;
    uint32_t val = 0;

    if (map & (1 << 0)) {
        mask |= REG_380_RCLK_DLY_0_MASK;
        val |= REG_380_RCLK_DLY_0(delay);
    }
    if (map & (1 << 1)) {
        mask |= REG_380_RCLK_DLY_1_MASK;
        val |= REG_380_RCLK_DLY_1(delay);
    }
    if (map & (1 << 2)) {
        mask |= REG_380_RCLK_DLY_2_MASK;
        val |= REG_380_RCLK_DLY_2(delay);
    }
    if (map & (1 << 3)) {
        mask |= REG_380_RCLK_DLY_3_MASK;
        val |= REG_380_RCLK_DLY_3(delay);
    }

    norflash[id]->REG_380 = (norflash[id]->REG_380 & ~mask) | val;
}

uint32_t norflaship_get_rclk_dly(enum HAL_FLASH_ID_T id, uint32_t map)
{
    uint32_t delay = 0;
    uint32_t val = norflash[id]->REG_380;

    if (map & (1 << 0)) {
        delay = GET_BITFIELD(val ,REG_380_RCLK_DLY_0);
    } else if (map & (1 << 1)) {
        delay = GET_BITFIELD(val ,REG_380_RCLK_DLY_1);
    } else if (map & (1 << 2)) {
        delay = GET_BITFIELD(val ,REG_380_RCLK_DLY_2);
    } else if (map & (1 << 3)) {
        delay = GET_BITFIELD(val ,REG_380_RCLK_DLY_3);
    }

    return delay;
}

void norflaship_rclk_dly_enable(enum HAL_FLASH_ID_T id, uint32_t map)
{
    norflash[id]->REG_380 &= ~REG_380_DLY_LINE_BYPASS(map);
}

void norflaship_rclk_dly_disable(enum HAL_FLASH_ID_T id, uint32_t map)
{
    norflash[id]->REG_380 |= REG_380_DLY_LINE_BYPASS(map);
}

uint32_t norflaship_get_rclk_dly_status(enum HAL_FLASH_ID_T id)
{
    uint32_t map;
    uint32_t bypass;

    bypass = GET_BITFIELD(norflash[id]->REG_380, REG_380_DLY_LINE_BYPASS);
    map = (~bypass) & (REG_380_DLY_LINE_BYPASS_MASK >> REG_380_DLY_LINE_BYPASS_SHIFT);
    return map;
}

void norflaship_set_rxfifo_ctrl(enum HAL_FLASH_ID_T id, uint32_t val)
{
    norflash[id]->REG_20C = REG_20C_RXFIFO_CTRL(val);
}
#endif

#endif // FLASH_CTRL_BASE

#endif // (CHIP_FLASH_CTRL_VER >= 2)
