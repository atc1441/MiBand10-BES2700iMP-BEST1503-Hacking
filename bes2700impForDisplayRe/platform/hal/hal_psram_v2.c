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
#if defined(CHIP_HAS_PSRAM) && \
    (CHIP_PSRAM_CTRL_VER == 2 || CHIP_PSRAM_CTRL_VER == 3 || CHIP_PSRAM_CTRL_VER == 5)

#include "plat_types.h"
#include "plat_addr_map.h"
#include "cmsis.h"
#include "hal_location.h"
#include "hal_psram.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_cache.h"
#include "hal_chipid.h"
#include "pmu.h"
#include "reg_psram_mc_v2.h"
#include "reg_psram_phy_v2.h"
#include "string.h"

#define PSRAM_RESET
//#define PSRAM_DUAL_8BIT
#ifdef CHIP_BEST2003
    #ifndef PSRAM_APS3208K
        #define PSRAM_WRAP_ENABLE
    #endif
#endif

#ifndef PSRAM_PAGE_SIZE
#ifdef PSRAM_XCCELA_MODE
// APS128xxN APS256xxN APS512xxN
#define PSRAM_PAGE_SIZE                     2048
#else /* PSRAM_XCCELA_MODE */
// APS3208K APS6408L
#define PSRAM_PAGE_SIZE                     1024
#endif /* PSRAM_XCCELA_MODE */
#endif

#ifdef PSRAM_DEBUG
    #define PSRAM_TRACE TRACE_IMM
#else
    #define PSRAM_TRACE(...)
#endif

#ifdef PSRAM_DEBUG
    #define PSRAM_TRACENOCRLF_NOTS REL_TRACE_NOCRLF_NOTS
#else
    #define PSRAM_TRACENOCRLF_NOTS(...)
#endif

#ifdef CHIP_BEST1502X
#define SLEEP_WITH_DIGPHY
#endif

#define TX_FIFO_DEPTH                       8
#define RX_FIFO_DEPTH                       8

// MR0
#define MR0_DRIVE_STR_SHIFT                 0
#define MR0_DRIVE_STR_MASK                  (0x3 << MR0_DRIVE_STR_SHIFT)
#define MR0_DRIVE_STR(n)                    BITFIELD_VAL(MR0_DRIVE_STR, n)
#define MR0_READ_LATENCY_SHIFT              2
#define MR0_READ_LATENCY_MASK               (0x7 << MR0_READ_LATENCY_SHIFT)
#define MR0_READ_LATENCY(n)                 BITFIELD_VAL(MR0_READ_LATENCY, n)
#define MR0_LT                              (1 << 5)
#define MR0_FIXED_00_SHIFT                  6
#define MR0_FIXED_00_MASK                   (0x3 << MR0_FIXED_00_SHIFT)
#define MR0_FIXED_00(n)                     BITFIELD_VAL(MR0_FIXED_00, n)

// MR1
#define MR1_VENDOR_ID_SHIFT                 0
#define MR1_VENDOR_ID_MASK                  (0x1F << MR1_VENDOR_ID_SHIFT)
#define MR1_VENDOR_ID(n)                    BITFIELD_VAL(MR1_VENDOR_ID, n)
#define MR1_DENSITY_SHIFT                   5
#define MR1_DENSITY_MASK                    (0x3 << MR1_DENSITY_SHIFT)
#define MR1_DENSITY(n)                      BITFIELD_VAL(MR1_DENSITY, n)
#define MR1_ULP                             (1 << 7)

// MR2
#define MR2_VENDOR_ID_SHIFT                 0
#define MR2_VENDOR_ID_MASK                  (0x7 << MR2_VENDOR_ID_SHIFT)
#define MR2_VENDOR_ID(n)                    BITFIELD_VAL(MR2_VENDOR_ID, n)
#define MR2_DEV_ID_SHIFT                    3
#define MR2_DEV_ID_MASK                     (0x3 << MR2_DEV_ID_SHIFT)
#define MR2_DEV_ID(n)                       BITFIELD_VAL(MR2_DEV_ID, n)
#define MR2_RSVD                            (1 << 5)
#define MR2_FIXED_1                         (1 << 6)
#define MR2_GB                              (1 << 7)

// MR4
#define MR4_PASR_SHIFT                      0
#define MR4_PASR_MASK                       (0x7 << MR4_PASR_SHIFT)
#define MR4_PASR(n)                         BITFIELD_VAL(MR4_PASR, n)
#define MR4_RF                              (1 << 3)
#define MR4_FIXED_0                         (1 << 4)
#define MR4_WRITE_LATENCY_SHIFT             5
#define MR4_WRITE_LATENCY_MASK              (0x7 << MR4_WRITE_LATENCY_SHIFT)
#define MR4_WRITE_LATENCY(n)                BITFIELD_VAL(MR4_WRITE_LATENCY, n)

// MR6
#define MR6_RSVD_SHIFT                      0
#define MR6_RSVD_MASK                       (0xF << MR6_RSVD_SHIFT)
#define MR6_RSVD(n)                         BITFIELD_VAL(MR6_RSVD, n)
#define MR6_HALF_SLEEP_SHIFT                4
#define MR6_HALF_SLEEP_MASK                 (0xF << MR6_HALF_SLEEP_SHIFT)
#define MR6_HALF_SLEEP(n)                   BITFIELD_VAL(MR6_HALF_SLEEP, n)

// MR8
#define MR8_BL_SHIFT                        0
#define MR8_BL_MASK                         (0x3 << MR8_BL_SHIFT)
#define MR8_BL(n)                           BITFIELD_VAL(MR8_BL, n)
#define MR8_BT                              (1 << 2)
#define MR8_FIXED_0                         (1 << 3)
#define MR8_RSVD_SHIFT                      4
#define MR8_RSVD_MASK                       (0x7 << MR8_RSVD_SHIFT)
#define MR8_RSVD(n)                         BITFIELD_VAL(MR8_RSVD, n)
#define MR8_FIXED_00                        (1 << 7)

enum PSRAM_CMD_T {
    PSRAM_CMD_SYNC_READ     = 0x00,
    PSRAM_CMD_SYNC_WRITE    = 0x80,
    PSRAM_CMD_4BYTE_READ    = 0x3F,
    PSRAM_CMD_4BYTE_WRITE   = 0xBF,
    PSRAM_CMD_REG_READ      = 0x40,
    PSRAM_CMD_REG_WRITE     = 0xC0,
    PSRAM_CMD_GLOBAL_RESET  = 0xFF,
};

enum CP_FSM_STATE_T {
    CP_FSM_STATE_SELF_REFRESH   = 1,
    CP_FSM_STATE_PD             = 2,
    CP_FSM_STATE_READY          = 4,
};

enum MEMIF_CMD_T {
    MEMIF_NO_CMD        = 0x00,
    MEMIF_WRITE         = 0x01,
    MEMIF_READ          = 0x02,
    MEMIF_MRS           = 0x05,
    MEMIF_MRR           = 0x06,
    MEMIF_REF           = 0x08,
    MEMIF_SREF          = 0x09,
    MEMIF_PD            = 0x10,
    MEMIF_NOP           = 0x20,
    MEMIF_RST           = 0xFF,
    MEMIF_ZQCL          = 0x85,
    MEMIF_ZQCS          = 0x45,
    MEMIF_ZQCRST        = 0x25,
    MEMIF_START_CLOCK   = 0x40,
    MEMIF_STOP_CLOCK    = 0x80,
    MEMIF_NEW_CMD       = 0x7F,
};

static struct PSRAM_MC_T *const psram_mc = (struct PSRAM_MC_T *)PSRAM_CTRL_BASE;
static struct PSRAM_PHY_T *const psram_phy = (struct PSRAM_PHY_T *)(PSRAM_CTRL_BASE + 0x8000);

#ifdef FPGA
    static const uint32_t psram_cfg_clk = 20 * 1000 * 1000;
    static const uint32_t psram_run_clk = 20 * 1000 * 1000;
#else /*FPGA*/
    #if (PSRAM_SPEED != 0)
        static const uint32_t psram_cfg_clk = 48 * 1000 * 1000;
        static const uint32_t psram_run_clk = PSRAM_SPEED * 1000 * 1000;
    #else
        #error "invalid PSRAM_SPEED"
    #endif
#endif /*FPGA*/

static void psram_chip_timing_config(uint32_t clk, bool psram_first);

int hal_psramip_mc_busy(void)
{
    return !!(psram_mc->REG_404 & PSRAM_ULP_MC_BUSY);
}

static int hal_psramip_wb_busy(void)
{
    return !!(psram_mc->REG_404 & PSRAM_ULP_MC_WB_FILL_LEVEL_MASK);
}

int hal_psramip_mc_in_sleep(void)
{
    return GET_BITFIELD(psram_mc->REG_404, PSRAM_ULP_MC_CP_FSM_STATE) == CP_FSM_STATE_PD;
}

int hal_psramip_rx_fifo_empty(void)
{
    return !!(psram_mc->REG_404 & PSRAM_ULP_MC_MGR_RXFIFO_R_EMPTY);
}

int hal_psramip_tx_fifo_full(void)
{
    return !!(psram_mc->REG_404 & PSRAM_ULP_MC_MGR_TXFIFO_W_FULL);
}

uint32_t hal_psramip_get_rx_fifo_len(void)
{
    return GET_BITFIELD(psram_mc->REG_404, PSRAM_ULP_MC_MGR_RXFIFO_FULL_CNT);
}

uint32_t hal_psramip_get_tx_fifo_free_len(void)
{
    return GET_BITFIELD(psram_mc->REG_404, PSRAM_ULP_MC_MGR_TXFIFO_EMPTY_CNT);
}

void hal_psramip_mc_busy_wait(void)
{
    while (hal_psramip_mc_busy());
}

void hal_psramip_wb_busy_wait(void)
{
    while (hal_psramip_wb_busy());
}

void hal_psramip_flush_tx_fifo(void)
{
    hal_psramip_mc_busy_wait();
    psram_mc->REG_01C = PSRAM_ULP_MC_MGR_TX_FIFO_CLR;
    hal_psramip_mc_busy_wait();
}

void hal_psramip_flush_rx_fifo(void)
{
    hal_psramip_mc_busy_wait();
    psram_mc->REG_01C = PSRAM_ULP_MC_MGR_RX_FIFO_CLR;
    hal_psramip_mc_busy_wait();
}

void hal_psramip_flush_all_fifo(void)
{
    hal_psramip_mc_busy_wait();
    psram_mc->REG_01C = PSRAM_ULP_MC_MGR_TX_FIFO_CLR | PSRAM_ULP_MC_MGR_RX_FIFO_CLR;
    hal_psramip_mc_busy_wait();
}

void hal_psramip_xfer_addr_len(uint32_t addr, uint32_t len)
{
    psram_mc->REG_008 = addr;
    psram_mc->REG_00C = len;
}

void hal_psramip_write_fifo(uint32_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        psram_mc->REG_014 = *data++;
    }
}

void hal_psramip_read_fifo(uint32_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        *data++ = psram_mc->REG_018;
    }
}

void hal_psramip_set_reg_data_mask(void)
{
#ifdef PSRAM_DUAL_8BIT
    psram_mc->REG_010 = 0xFC;
#else
    psram_mc->REG_010 = 0xFE;
#endif
}

void hal_psramip_set_mem_data_mask(void)
{
    psram_mc->REG_010 = 0;
}

void hal_psramip_set_cmd(enum MEMIF_CMD_T cmd)
{
    psram_mc->REG_004 = cmd;
}

POSSIBLY_UNUSED void psram_read_reg(uint32_t reg, uint32_t *val)
{
    hal_psramip_flush_all_fifo();
    hal_psramip_xfer_addr_len(reg, 1);
    hal_psramip_set_cmd(MEMIF_MRR);
    while (hal_psramip_rx_fifo_empty());
    hal_psramip_read_fifo(val, 1);
}

static void psram_send_cmd_reg(enum MEMIF_CMD_T cmd, uint32_t reg, uint32_t val)
{
#ifdef PSRAM_DUAL_8BIT
    val &= 0xFF;
    val |= (val << 8);
#endif
    hal_psramip_flush_all_fifo();
    //hal_psramip_set_reg_data_mask();
    hal_psramip_write_fifo(&val, 1);
    hal_psramip_xfer_addr_len(reg, 1);
    hal_psramip_set_cmd(cmd);
    while (hal_psramip_get_tx_fifo_free_len() != TX_FIFO_DEPTH);
    hal_psramip_mc_busy_wait();
    //hal_psramip_set_mem_data_mask();
}

static void psram_write_reg(uint32_t reg, uint32_t val)
{
    psram_send_cmd_reg(MEMIF_MRS, reg, val);
}

static void psram_single_cmd(enum MEMIF_CMD_T cmd)
{
    hal_psramip_flush_all_fifo();
    hal_psramip_set_cmd(cmd);
    hal_psramip_mc_busy_wait();
}

static POSSIBLY_UNUSED void psram_reset(void)
{
    psram_single_cmd(MEMIF_RST);
#ifdef PSRAM_APS3208K
    hal_sys_timer_delay_us(1);
    psram_single_cmd(MEMIF_RST);
#endif
}

static void psram_set_timing(uint32_t clk)
{
    uint32_t reg;
    uint32_t val;
#ifdef PSRAM_XCCELA_MODE
    reg = 8;
#ifdef PSRAM_WRAP_ENABLE
    // Wrap 32
    val = MR8_BL(1);
#else
    // Wrap 1k
    val = MR8_BL(0x3);
#endif
    psram_write_reg(reg, val);

    reg = 0;
    if (clk <= 66000000) {
        val = 0;
    } else if (clk <= 109000000) {
        val = 1;
    } else if (clk <= 133000000) {
        val = 2;
    } else if (clk <= 166000000) {
        val = 3;
    } else {
        val = 4;
    }
    // Latency type: Variable
    val = MR0_DRIVE_STR(3) | MR0_READ_LATENCY(val);
    psram_write_reg(reg, val);

    reg = 4;
    if (clk <= 66000000) {
        val = 0;
    } else if (clk <= 109000000) {
        val = 4;
    } else if (clk <= 133000000) {
        val = 2;
    } else if (clk <= 166000000) {
        val = 6;
    } else {
        val = 1;
    }
    //Always 4x Refresh
    val = MR4_PASR(0) | MR4_WRITE_LATENCY(val);
    psram_write_reg(reg, val);
#else /*PSRAM_XCCELA_MODE*/
#ifndef PSRAM_APS3208K
    reg = 8;
#ifdef PSRAM_WRAP_ENABLE
    // Wrap 32
    val = MR8_BL(1);
#else
    // Wrap 1k
    val = MR8_BL(0x3);
#endif
    psram_write_reg(reg, val);
#endif /* PSRAM_APS3208K */
    reg = 0;
    if (clk <= 66000000) {
        val = 2;
    } else if (clk <= 109000000) {
        val = 3;
    } else if (clk <= 133000000) {
        val = 4;
    } else if (clk <= 166000000) {
        val = 5;
    } else {
        val = 6;
    }
    // Latency type: Variable
    val = MR0_DRIVE_STR(3) | MR0_READ_LATENCY(val);
    psram_write_reg(reg, val);

    reg = 4;
    if (clk <= 166000000) {
        val = 0;
    } else {
        val = 4;
    }
    //Fast Refresh,
    val = MR4_PASR(0) | MR4_WRITE_LATENCY(val);
    psram_write_reg(reg, val);
#endif /*PSRAM_XCCELA_MODE*/
}

static uint32_t hal_psram_phy_read_reg(__I uint32_t *addr)
{
#if defined(CHIP_BEST2003) || defined(CHIP_BEST2006)
    *addr;
#endif
    return *addr;
}

static void hal_psram_phy_dll_config(uint32_t clk)
{
    uint32_t phy_clk;
    uint32_t range;
    uint32_t val;

#if CHIP_PSRAM_CTRL_VER >= 5
    uint32_t t_trans, t_precharge, t_pu, t_lock;
    /* warning: cycle count of controller clock. for 1502: controller clock freq = die clock freq */
#if (defined(CHIP_BEST1306P) || defined(CHIP_BEST1502P) || defined(CHIP_BEST1502X) || defined(CHIP_BEST1503))
    // t_trans > 30ns
    t_trans = (clk / 1000000 * 30 + (1000 - 1)) / 1000;
    // t_precharge > 15us
    t_precharge = clk / 1000000 * 15 + 1;
    psram_phy->REG_060 = PSRAM_ULP_PHY_REG_T_TRANS(t_trans) | PSRAM_ULP_PHY_REG_T_PRECHARGE(t_precharge);
    // t_pu > 400ns
    t_pu = (clk / 1000000 * 400 + (1000 - 1)) / 1000;
    // t_lock > 800ns
    t_lock = (clk / 1000000 * 800 + (1000 - 1)) / 1000;
    psram_phy->REG_064 = PSRAM_ULP_PHY_REG_T_PU(t_pu) | PSRAM_ULP_PHY_REG_T_LOCK(t_lock);
#else
#error "WARNING: modify this reg for every CHIP"
#endif
#endif

    val = hal_psram_phy_read_reg(&psram_phy->REG_050);
    val &= ~PSRAM_ULP_PHY_REG_DLL_RESETB | PSRAM_ULP_PHY_REG_DLL_CK_RDY;
    psram_phy->REG_050 = val;
    phy_clk = clk;
    if (phy_clk <= 100000000 / 2) {
        range = 3;
    } else if (phy_clk <= 150000000 / 2) {
        range = 2;
    } else if (phy_clk <= 300000000 / 2) {
        range = 1;
    } else {
        range = 1;
    }
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_DLL_RANGE, range);
    psram_phy->REG_050 = val;
    hal_sys_timer_delay_us(100);
    val |= PSRAM_ULP_PHY_REG_DLL_RESETB | PSRAM_ULP_PHY_REG_DLL_CK_RDY;
    psram_phy->REG_050 = val;
    hal_sys_timer_delay_us(100);
}

static void hal_psram_phy_init(uint32_t clk)
{
    uint32_t val;
    val = hal_psram_phy_read_reg(&psram_phy->REG_048);
    val |= PSRAM_ULP_PHY_REG_LDO_PU | PSRAM_ULP_PHY_REG_LDO_PRECHARGE;
    psram_phy->REG_048 = val;
    hal_sys_timer_delay_us(100);

    val &= ~PSRAM_ULP_PHY_REG_LDO_PRECHARGE;
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_IEN1, 0xd);
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_IEN2, 0x7);
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_VTUNE, 0x2);
#if CHIP_PSRAM_CTRL_VER >= 3
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_RES, 0x4);
#endif
    psram_phy->REG_048 = val;
    hal_sys_timer_delay_us(100);

#if CHIP_PSRAM_CTRL_VER >= 3
#ifdef CHIP_BEST2003
    // chip 2006
    if (hal_get_chip_metal_id() >= HAL_CHIP_METAL_ID_8) {
        psram_mc->REG_200 |= PSRAM_ULP_PHY_REG_CEB_PU;
    }
#endif
#endif
    val = hal_psram_phy_read_reg(&psram_phy->REG_04C);
    val |= PSRAM_ULP_PHY_REG_PSRAM_PU;
    //val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_SWRC, 0x3);
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_TXDRV, 0x3);
    psram_phy->REG_04C = val;
    hal_sys_timer_delay_us(100);

    val = hal_psram_phy_read_reg(&psram_phy->REG_050);
    val |= PSRAM_ULP_PHY_REG_DLL_PU;
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_DLL_SWRC, 0x0);
    val &= ~PSRAM_ULP_PHY_REG_BYPASS_DECIMATION;
    psram_phy->REG_050 = val;
    hal_sys_timer_delay_us(100);

    val |= PSRAM_ULP_PHY_REG_DLL_RESETB;
    psram_phy->REG_050 = val;
    hal_sys_timer_delay_us(100);

    hal_psram_phy_dll_config(clk);
    hal_sys_timer_delay_us(500);
}

static void hal_psram_digphy_init()
{
#if CHIP_PSRAM_CTRL_VER >= 3
    uint32_t val;
    val = hal_psram_phy_read_reg(&psram_phy->REG_004);
#ifdef PSRAM_XCCELA_MODE
    val |= PSRAM_ULP_PHY_ADDR_4BYTES_EN | PSRAM_ULP_PHY_DQS_DM_MERGE_EN;
    val &= ~PSRAM_ULP_PHY_PHY_DUMMY_CYC_EN;
#endif
    val |= PSRAM_ULP_PHY_CTRL_DELAY(1);
    val &= ~PSRAM_ULP_PHY_PHY_DLY_AUTO_EN;
    psram_phy->REG_004 = val;
#endif
}

POSSIBLY_UNUSED void hal_psram_phy_sleep()
{
#if CHIP_PSRAM_CTRL_VER >= 5 && defined(SLEEP_WITH_DIGPHY)
    /* use dig phy because ana phy cs go low after ldo off */
    psram_phy->REG_040 &= ~PSRAM_ULP_PHY_ANA_PHY_LP_CTRL_DR;
    psram_phy->REG_040 |= PSRAM_ULP_PHY_ANA_PHY_BYPASS;
    hal_psramip_mc_busy_wait();
#else
    uint32_t val;

    val = hal_psram_phy_read_reg(&psram_phy->REG_04C);
    val &= ~PSRAM_ULP_PHY_REG_PSRAM_PU;
    psram_phy->REG_04C = val;

    val = hal_psram_phy_read_reg(&psram_phy->REG_050);
    val &= ~PSRAM_ULP_PHY_REG_DLL_PU;
    psram_phy->REG_050 = val;

    val = hal_psram_phy_read_reg(&psram_phy->REG_048);
    val &= ~PSRAM_ULP_PHY_REG_LDO_PU;
    psram_phy->REG_048 = val;
#endif
}

POSSIBLY_UNUSED static void hal_psram_phy_wakeup()
{
    uint32_t val;
#if CHIP_PSRAM_CTRL_VER >= 5 && defined(SLEEP_WITH_DIGPHY)
    /* use dig phy because ana phy cs go low after ldo off */
    psram_phy->REG_040 &= ~PSRAM_ULP_PHY_ANA_PHY_BYPASS;
    hal_sys_timer_delay_us(20);
    hal_psramip_mc_busy_wait();
    psram_phy->REG_040 |= PSRAM_ULP_PHY_ANA_PHY_LP_CTRL_DR;
#else
    val = hal_psram_phy_read_reg(&psram_phy->REG_048);
    val |= PSRAM_ULP_PHY_REG_LDO_PU | PSRAM_ULP_PHY_REG_LDO_PRECHARGE;
    psram_phy->REG_048 = val;
    hal_sys_timer_delay_us(10);

    val &= ~PSRAM_ULP_PHY_REG_LDO_PRECHARGE;
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_IEN1, 0xd);
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_IEN2, 0x7);
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_VTUNE, 0x2);
    psram_phy->REG_048 = val;

    val = hal_psram_phy_read_reg(&psram_phy->REG_04C);
    val |= PSRAM_ULP_PHY_REG_PSRAM_PU;
    //val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_SWRC, 0x3);
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_PSRAM_TXDRV, 0x3);
    psram_phy->REG_04C = val;

    val = hal_psram_phy_read_reg(&psram_phy->REG_050);
    val |= PSRAM_ULP_PHY_REG_DLL_PU;
    //val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_DLL_SWRC, 0x3);
    psram_phy->REG_050 = val;
    hal_sys_timer_delay_us(2);

    val |= PSRAM_ULP_PHY_REG_DLL_RESETB;
    psram_phy->REG_050 = val;
    hal_sys_timer_delay_us(20);
#endif

    do {
        val = hal_psram_phy_read_reg(&psram_phy->REG_058);
    } while ((val & PSRAM_ULP_PHY_DLL_LOCK) == 0);
}
static void hal_psram_mc_set_timing(uint32_t clk, bool init)
{
    uint32_t val;
#ifdef PSRAM_XCCELA_MODE
    if (init) {
        val = PSRAM_ULP_MC_WRITE_LATENCY(1);
    } else {
        if (clk <= 66000000) {
            val = PSRAM_ULP_MC_WRITE_LATENCY(3);
        } else if (clk <= 109000000) {
            val = PSRAM_ULP_MC_WRITE_LATENCY(4);
        } else if (clk <= 133000000) {
            val = PSRAM_ULP_MC_WRITE_LATENCY(5);
        } else if (clk <= 166000000) {
            val = PSRAM_ULP_MC_WRITE_LATENCY(6);
        } else {
            val = PSRAM_ULP_MC_WRITE_LATENCY(7);
        }
    }
    psram_mc->REG_028 = val;
    if (clk <= 66000000) {
        val = PSRAM_ULP_MC_READ_LATENCY(3);
    } else if (clk <= 109000000) {
        val = PSRAM_ULP_MC_READ_LATENCY(4);
    } else if (clk <= 133000000) {
        val = PSRAM_ULP_MC_READ_LATENCY(5);
    } else if (clk <= 166000000) {
        val = PSRAM_ULP_MC_READ_LATENCY(6);
    } else {
        val = PSRAM_ULP_MC_READ_LATENCY(7);
    }
    psram_mc->REG_02C = val;
    // tCPH >= 24 ns
    val = (clk / 1000000 * 24 + (1000 - 1)) / 1000;
    psram_mc->REG_058 = PSRAM_ULP_MC_T_CPHR(val);
    psram_mc->REG_060 = PSRAM_ULP_MC_T_CPHW(val);
    psram_mc->REG_068 = PSRAM_ULP_MC_T_MRR(val);
    psram_mc->REG_06C = PSRAM_ULP_MC_T_MRS(val);
#else /*PSRAM_XCCELA_MODE*/
    if (clk <= 166000000) {
        val = PSRAM_ULP_MC_WRITE_LATENCY(0);
    } else {
        val = PSRAM_ULP_MC_WRITE_LATENCY(2);
    }
    psram_mc->REG_028 = val;
    if (clk <= 66000000) {
        val = PSRAM_ULP_MC_READ_LATENCY(2);
    } else if (clk <= 109000000) {
        val = PSRAM_ULP_MC_READ_LATENCY(3);
    } else if (clk <= 133000000) {
        val = PSRAM_ULP_MC_READ_LATENCY(4);
    } else if (clk <= 166000000) {
        val = PSRAM_ULP_MC_READ_LATENCY(5);
    } else {
        val = PSRAM_ULP_MC_READ_LATENCY(6);
    }
    psram_mc->REG_02C = val;
    // tCPH
    val = 2;
    psram_mc->REG_058 = PSRAM_ULP_MC_T_CPHR(val);
    psram_mc->REG_068 = PSRAM_ULP_MC_T_MRR(val);
    val = 6;
    psram_mc->REG_060 = PSRAM_ULP_MC_T_CPHW(val);
#if CHIP_PSRAM_CTRL_VER == 2
    val += 1;
#endif
    psram_mc->REG_06C = PSRAM_ULP_MC_T_MRS(val);
#endif /*PSRAM_XCCELA_MODE*/
    // tRC >= 55 ns
    val = (clk / 1000000 * 55 + (1000 - 1)) / 1000;
    psram_mc->REG_050 = PSRAM_ULP_MC_T_RC(val);
    // tCEM <= 2.5 us
    val = clk / 1000000 * 25 / 10;
    psram_mc->REG_070 = PSRAM_ULP_MC_T_CEM(val);
    // tRST >= 2 us
    val = clk / 1000000 * 2 + 1;
    psram_mc->REG_074 = PSRAM_ULP_MC_T_RST(val);
    // tHS >= 2 us
    val = clk / 1000000 * 2 + 1;
    psram_mc->REG_080 = PSRAM_ULP_MC_T_HS(val);
    // tXPHS in [60 ns, 2 us]
    val = (clk / 1000000 * 60 + (1000 - 1)) / 1000;
    psram_mc->REG_084 = PSRAM_ULP_MC_T_XPHS(val);
    // tXHS >= 100 us
    val = clk / 1000000 * 100 + 1;
    psram_mc->REG_088 = PSRAM_ULP_MC_T_XHS(val);
    psram_mc->REG_09C = PSRAM_ULP_MC_WR_DMY_CYC(1);
    // NOP dummy cycles, same as tXPHS in [60 ns, 2 us]
    val = (clk / 1000000 * 60 + (1000 - 1)) / 1000;
    psram_mc->REG_0A0 = PSRAM_ULP_MC_STOP_CLK_IN_NOP |
#ifdef PSRAM_XCCELA_MODE
                        PSRAM_ULP_MC_STOP_CLK_IN_TCPH |
#endif
                        PSRAM_ULP_MC_NOP_DMY_CYC(val);
    psram_mc->REG_0A4 = PSRAM_ULP_MC_QUEUE_IDLE_CYCLE(5000);
}

static void hal_psram_phy_wait_lock()
{
#ifndef PROGRAMMER
    uint32_t val;
    uint32_t val_50;
    uint32_t count = 0;
    val = hal_psram_phy_read_reg(&psram_phy->REG_048);
    val_50 = hal_psram_phy_read_reg(&psram_phy->REG_050);
#endif
    while (1) {
        uint32_t val_58;
        val_58 = hal_psram_phy_read_reg(&psram_phy->REG_058);
        if ((val_58 & PSRAM_ULP_PHY_DLL_LOCK))
            break;
#ifndef PROGRAMMER
        if (count++ > 20) {
            if (GET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_VTUNE) == 0x2) {
                val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_VTUNE, 0x1);
                psram_phy->REG_048 = val;
            } else if (GET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_VTUNE) == 0x1) {
                val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_LDO_VTUNE, 0x2);
                psram_phy->REG_048 = val;
                if (GET_BITFIELD(val_50, PSRAM_ULP_PHY_REG_DLL_RANGE) == 0x1) {
                    val_50 = SET_BITFIELD(val_50, PSRAM_ULP_PHY_REG_DLL_RANGE, 0x2);
                    psram_phy->REG_050 = val_50;
                }
            }
            count = 0;
        }
#endif
        hal_sys_timer_delay_us(100);
    }
}

POSSIBLY_UNUSED
static void hal_psram_init_calib(void)
{
    uint32_t val;
    uint32_t delay;

    hal_psram_phy_wait_lock();
    val = hal_psram_phy_read_reg(&psram_phy->REG_058);
    delay = GET_BITFIELD(val, PSRAM_ULP_PHY_DLL_DLY_IN);
    //ASSERT(delay < (PSRAM_ULP_PHY_DLL_DLY_IN_MASK >> PSRAM_ULP_PHY_DLL_DLY_IN_SHIFT),
    //    "%s: Bad DLL_DLY_IN=0x%X reg=0x%08X", __func__, delay, psram_phy->REG_058);

    delay /= 2;
    psram_phy->REG_054 = PSRAM_ULP_PHY_REG_PSRAM_TX_CEB_DLY(delay) | PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DLY(delay) |
                         PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY(delay) | PSRAM_ULP_PHY_REG_PSRAM_RX_DQS_DLY(delay);
    hal_sys_timer_delay_us(100);
}

static void hal_psram_mc_init(uint32_t clk)
{
    uint32_t val = 0;
#if CHIP_PSRAM_CTRL_VER >= 3
#if defined(PSRAM_DUAL_8BIT)
    val |= PSRAM_ULP_MC_CHIP_BIT;
#elif defined(PSRAM_X16_MODE)
    val |= PSRAM_ULP_MC_CHIP_X16;
#endif
#ifdef PSRAM_SQPI_MODE
    val |= PSRAM_ULP_MC_SQPI_MODE;
    val |= (PSRAM_ULP_PHY_IDLE_DQ_OEN | PSRAM_ULP_PHY_IDLE_DQS_OEN);
#else /* PSRAM_SQPI_MODE */
    val |= PSRAM_ULP_MC_OPI_MODE;
    val = SET_BITFIELD(val, PSRAM_ULP_MC_CHIP_CA_PATTERN, 0x1);
#endif /* PSRAM_SQPI_MODE */
#else /* CHIP_PSRAM_CTRL_VER == 2 */
#if defined(PSRAM_DUAL_8BIT)
    val |= PSRAM_ULP_MC_CHIP_BIT;
#endif
#endif /* CHIP_PSRAM_CTRL_VER >= 3 */
    psram_mc->REG_000 = val;
    psram_mc->REG_020 = 0;
    psram_mc->REG_024 =
#if CHIP_PSRAM_CTRL_VER >= 3
        //PSRAM_ULP_MC_ENTRY_SLEEP_IDLE |
#endif
        PSRAM_ULP_MC_AUTOWAKEUP_EN |
        PSRAM_ULP_MC_PD_MR(6) | PSRAM_ULP_MC_PD_CMD(0xF0);
#ifdef PSRAM_WRAP_ENABLE
    // Burst len: 32 bytes, page: 1K
    val = PSRAM_ULP_MC_BURST_LENGTH(1);
#else
    if (PSRAM_PAGE_SIZE == 512) {
        val = PSRAM_ULP_MC_BURST_LENGTH(7);
    } else if (PSRAM_PAGE_SIZE == 1024) {
        val = PSRAM_ULP_MC_BURST_LENGTH(4);
    } else if (PSRAM_PAGE_SIZE == 2048) {
        val = PSRAM_ULP_MC_BURST_LENGTH(5);
    } else {
        // 4K
        val = PSRAM_ULP_MC_BURST_LENGTH(6);
    }
#endif /* PSRAM_WRAP_ENABLE */
    if (PSRAM_PAGE_SIZE == 512) {
        val |= PSRAM_ULP_MC_PAGE_BOUNDARY(3);
    } else if (PSRAM_PAGE_SIZE == 1024) {
        val |= PSRAM_ULP_MC_PAGE_BOUNDARY(0);
    } else if (PSRAM_PAGE_SIZE == 2048) {
        val |= PSRAM_ULP_MC_PAGE_BOUNDARY(1);
    } else {
        // 4K
        val |= PSRAM_ULP_MC_PAGE_BOUNDARY(2);
    }
    psram_mc->REG_034 = val;
    // AHB bus width: 32 bits
    psram_mc->REG_038 = 0;
    // Write buffer level with high priority: 0~7
#if CHIP_PSRAM_CTRL_VER == 2
    psram_mc->REG_03C = PSRAM_ULP_MC_HIGH_PRI_LEVEL(1);
#else
    psram_mc->REG_03C = PSRAM_ULP_MC_HIGH_PRI_LEVEL(7);
#endif
#ifdef PSRAM_WRAP_ENABLE
    psram_mc->REG_040 = PSRAM_ULP_MC_CP_WRAP_EN;
#else
    psram_mc->REG_040 = PSRAM_ULP_MC_WRAP_CRT_RET_EN;
#endif
    psram_mc->REG_044 = 0;
    psram_mc->REG_048 = 0;

    hal_psramip_set_reg_data_mask();
    hal_sys_timer_delay_us(100);

    hal_psram_mc_set_timing(clk, true);
    hal_sys_timer_delay_us(100);

    psram_mc->REG_400 = PSRAM_ULP_MC_INIT_COMPLETE;
    hal_sys_timer_delay_us(100);
}

void hal_psram_sleep(void)
{
    hal_psramip_mc_busy_wait();
    if (!hal_psramip_mc_in_sleep()) {
#if CHIP_PSRAM_CTRL_VER >= 3
        //psram_mc->REG_024 &= ~PSRAM_ULP_MC_ENTRY_SLEEP_IDLE;
#endif
        hal_psramip_mc_busy_wait();
#if CHIP_PSRAM_CTRL_VER >= 3
#ifdef PSRAM_XCCELA_MODE
        int val = psram_mc->REG_028;
        psram_mc->REG_028 = PSRAM_ULP_MC_WRITE_LATENCY(1);
        __DSB();
        hal_psramip_mc_busy_wait();
#endif /* PSRAM_XCCELA_MODE */
        psram_write_reg(0x6, 0xF0);
        hal_psramip_mc_busy_wait();
#ifdef PSRAM_XCCELA_MODE
        psram_mc->REG_028 = val;
        hal_psramip_mc_busy_wait();
#endif /* PSRAM_XCCELA_MODE */
#else
        hal_psramip_set_cmd(MEMIF_PD);
        hal_psramip_mc_busy_wait();
#endif
    }
    hal_psram_phy_sleep();
}

void hal_psram_wakeup(void)
{
    hal_psram_phy_wakeup();
    hal_psramip_set_cmd(MEMIF_NOP);
    hal_psramip_mc_busy_wait();
#if CHIP_PSRAM_CTRL_VER >= 3
    //psram_mc->REG_024 |= PSRAM_ULP_MC_ENTRY_SLEEP_IDLE;
#endif
#if CHIP_PSRAM_CTRL_VER >= 5 && defined(SLEEP_WITH_DIGPHY)
    hal_sys_timer_delay_us(100);
#endif
}

static void psram_chip_timing_config(uint32_t clk, bool update_psram_first)
{
    enum HAL_CMU_FREQ_T freq;

    if (clk <= 52000000) {
        freq = HAL_CMU_FREQ_104M;
    } else if (clk <= 104000000) {
        freq = HAL_CMU_FREQ_208M;
    } else {
#ifdef HAL_CMU_FREQ_390M
        freq = HAL_CMU_FREQ_390M;
#else
        freq = HAL_CMU_FREQ_208M;
#endif
    }

    if (update_psram_first) {
        psram_set_timing(clk);
    }

    hal_cmu_mem_set_freq(freq);
    hal_sys_timer_delay_us(500);
    hal_psram_phy_dll_config(clk);

#if !defined(FPGA) && !defined(SIMU)
    hal_psram_init_calib();
#endif
    hal_psram_mc_set_timing(clk, false);
    if (!update_psram_first) {
        psram_set_timing(clk);
    }
    hal_sys_timer_delay_us(100);
}
void hal_psram_snoop_enable()
{
#if CHIP_PSRAM_CTRL_VER >= 3
    psram_mc->REG_044 &= ~PSRAM_ULP_MC_SNP_DISABLE;
#endif
}
void hal_psram_snoop_disable()
{
    psram_mc->REG_044 |= PSRAM_ULP_MC_SNP_DISABLE;
}

void hal_psram_write_buffer_invalidate(void)
{
    uint32_t level;
    uint32_t start = hal_sys_timer_get();

    __DSB();
    do {
        level = GET_BITFIELD(psram_mc->REG_404, PSRAM_ULP_MC_WB_FILL_LEVEL);
        if (level == 0) {
            break;
        }
    } while (hal_sys_timer_get() - start < MS_TO_TICKS(1));

    ASSERT(level == 0, "%s: Drain timeout: level=%u status=0x%08X", __func__, level, psram_mc->REG_404);

    psram_mc->REG_044 |= PSRAM_ULP_MC_WB_INVALID;
    __DSB();
}

static void hal_psram_phy_restore_range(uint32_t range)
{
    uint32_t val;

    val = hal_psram_phy_read_reg(&psram_phy->REG_050);
    val &= ~PSRAM_ULP_PHY_REG_DLL_RESETB | PSRAM_ULP_PHY_REG_DLL_CK_RDY;
    psram_phy->REG_050 = val;
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_DLL_RANGE, range);
    psram_phy->REG_050 = val;
    val |= PSRAM_ULP_PHY_REG_DLL_RESETB | PSRAM_ULP_PHY_REG_DLL_CK_RDY;
    psram_phy->REG_050 = val;
}

static void psram_mc_reset(uint32_t range)
{
    hal_cmu_reset_set(HAL_CMU_MOD_H_PSRAM);
    hal_cmu_reset_set(HAL_CMU_MOD_O_PSRAM);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_PSRAM);
    hal_cmu_reset_clear(HAL_CMU_MOD_O_PSRAM);

    hal_cmu_mem_set_freq(HAL_CMU_FREQ_104M);
    hal_psram_phy_init(psram_cfg_clk);
    hal_sys_timer_delay_us(30);
    hal_psram_mc_init(psram_cfg_clk);
#if !defined(FPGA) && !defined(SIMU)
    hal_psram_init_calib();
#endif
    psram_reset();
    psram_chip_timing_config(psram_run_clk, true);
    hal_psram_phy_restore_range(range);
#if !defined(FPGA) && !defined(SIMU)
    hal_psram_init_calib();
#endif

    hal_psram_snoop_disable();
}
POSSIBLY_UNUSED static bool psramphy_check_tx_ceb_valid(uint32_t range)
{
    int retry = 2;
    while (retry--) {
        int count = 0;
        hal_psramip_flush_all_fifo();
        hal_psramip_xfer_addr_len(0x100000, 8);
        hal_psramip_set_cmd(MEMIF_READ);
        while (hal_psramip_rx_fifo_empty()) {
            if (count++ > 10) {
                //PSRAM_TRACE(1, "%s, rx fifo empty, return false", __FUNCTION__);
                psram_mc_reset(range);
                hal_psram_snoop_disable();
                return false;
            }
        }
    }
    return true;
}
POSSIBLY_UNUSED static bool psramphy_check_tx_clk_valid(uint32_t range)
{
    uint32_t time;
    uint32_t val;

    time = hal_fast_sys_timer_get();
    psram_mc->REG_01C = PSRAM_ULP_MC_MGR_TX_FIFO_CLR | PSRAM_ULP_MC_MGR_RX_FIFO_CLR;
    hal_psramip_xfer_addr_len(0xAA55AA55, 1);
    hal_psramip_set_cmd(MEMIF_READ);
    while (hal_psramip_rx_fifo_empty()) {
        if (FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time) > 5) {
            val = hal_psram_phy_read_reg(&psram_phy->REG_004);
            val |= PSRAM_ULP_PHY_PHY_LOOPBACK_EN;
            psram_phy->REG_004 = val;
            while (hal_psramip_mc_busy());
            val &= ~PSRAM_ULP_PHY_PHY_LOOPBACK_EN;
            psram_phy->REG_004 = val;
            return false;
        }
    }
    return true;
}
static bool psramphy_check_write_valid()
{
    uint32_t i;
    uint32_t val, val0, val1;
    uint32_t val2, val3;
    volatile uint32_t *psram_base = (volatile uint32_t *)PSRAM_NC_BASE;
    volatile uint32_t *psram_base1 = (volatile uint32_t *)(PSRAM_NC_BASE + 0x100000);

    for (i = 0; i < 0x100; ++i) {
        *(psram_base + i) = 0xffffffff;
        *(psram_base1 + i) = 0xffffffff;
    }
    for (i = 0; i < 0x100; ++i) {
        val = i & 0xFF;
        val = val | (val << 8) | (val << 16) | (val << 24);
        val0 = (val & 0x00FF00FF) | ((~val) & 0xFF00FF00);
        val1 = (val & 0xFF0000FF) | ((~val) & 0x00FFFF00);
        *(psram_base + i) = val0;
        *(psram_base1 + i) = val1;
    }

    hal_psramip_wb_busy_wait();
    hal_psramip_mc_busy_wait();
    for (i = 0; i < 0x100; ++i) {
        val = i & 0xFF;
        val = val | (val << 8) | (val << 16) | (val << 24);
        val0 = (val & 0x00FF00FF) | ((~val) & 0xFF00FF00);
        val1 = (val & 0xFF0000FF) | ((~val) & 0x00FFFF00);
        val2 = *(psram_base + i);
        val3 = *(psram_base1 + i);
        if (val2 != val0) {
            //PSRAM_TRACE(3, "%s, i:%d, 0x%x, 0x%x", __FUNCTION__, i, val2, val0);
            return false;
        }
        if (val3 != val1) {
            //PSRAM_TRACE(3, "%s, i:%d, 0x%x, 0x%x", __FUNCTION__, i, val3, val1);
            return false;
        }
    }
    return true;
}

static void hal_psram_calib_range(uint32_t range)
{
    #define BUFFER_SIZE  0x20
    uint32_t val;
    uint32_t delay;
    POSSIBLY_UNUSED uint8_t tx_dqs, rx_dqs, tx_ceb, tx_clk;
    uint8_t inc_delay, volume;
    POSSIBLY_UNUSED int8_t left_bound;
    POSSIBLY_UNUSED int8_t right_bound;
    bool cali_valid[BUFFER_SIZE][BUFFER_SIZE];
    uint8_t cali_value[BUFFER_SIZE][BUFFER_SIZE];
    POSSIBLY_UNUSED bool valid_array[BUFFER_SIZE];

#ifdef PSRAM_WINDOW_TEST
    static int8_t last_left_bound = 0;
    static int8_t last_right_bound = 0;
    uint8_t dqs_point = 0;
    uint8_t dqs_len = 0;
    static uint8_t last_dqs_point = 0;
    static uint8_t last_dqs_len = 0;
    POSSIBLY_UNUSED static uint8_t cali_data[BUFFER_SIZE][BUFFER_SIZE];
#endif

    ASSERT(range <= (PSRAM_ULP_PHY_REG_DLL_RANGE_MASK >> PSRAM_ULP_PHY_REG_DLL_RANGE_SHIFT), "ERROR, bad ana phy range:%d", range);

    val = hal_psram_phy_read_reg(&psram_phy->REG_050);
    val &= ~(PSRAM_ULP_PHY_REG_DLL_RESETB | PSRAM_ULP_PHY_REG_DLL_CK_RDY);
    psram_phy->REG_050 = val;
    val = SET_BITFIELD(val, PSRAM_ULP_PHY_REG_DLL_RANGE, range);
    psram_phy->REG_050 = val;
    hal_sys_timer_delay_us(100);
    val |= (PSRAM_ULP_PHY_REG_DLL_RESETB | PSRAM_ULP_PHY_REG_DLL_CK_RDY);
    psram_phy->REG_050 = val;

    hal_sys_timer_delay_us(100);
    hal_sys_timer_delay_us(100);

    hal_psram_phy_wait_lock();

    val = hal_psram_phy_read_reg(&psram_phy->REG_058);
    if ((val & PSRAM_ULP_PHY_DLL_ALL_ONE)) {
        PSRAM_TRACE(2, "%s: all one, increase range=%d", __func__, range + 1);
        return hal_psram_calib_range(range + 1);
    }

    delay = GET_BITFIELD(val, PSRAM_ULP_PHY_DLL_DLY_IN);
    PSRAM_TRACE(4, "%s, range:%d, T/4 = 0x%x(psram_phy->REG_058:0x%x)", __func__, range, delay / 2, val);
#ifdef PSRAM_WINDOW_TEST
    TRACE_IMM(TR_ATTR_NO_LF | TR_ATTR_NO_TS | TR_ATTR_NO_ID, ", T/4=0x%02x", delay / 2);
#endif
    if (delay > (PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY_MASK >> PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY_SHIFT) && range < 3) {
        PSRAM_TRACE(2, "%s: bad delay (T/2 > 0x1f). increase range=%d", __func__, range + 1);
        return hal_psram_calib_range(range + 1);
    }

    inc_delay = delay / 8;
    if (inc_delay == 0)
        inc_delay = 1;

#if CHIP_PSRAM_CTRL_VER >= 3
    tx_ceb = delay / 2;
    tx_clk = delay / 2 + 2;
#else /*CHIP_PSRAM_CTRL_VER == 2*/
    tx_ceb = delay * 5 / 6; // t/4 + t/6
    tx_clk = (delay * 11 + 8) / 16;// t/4 + t/8 - t/32
#endif

    volume = (PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY_MASK >> PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY_SHIFT) / inc_delay;
    //PSRAM_TRACE(2, "volume:%d, inc_delay:%d", volume, inc_delay);

#if 0
    //calibrate tx_ceb
    PSRAM_TRACE(0, "cali tx_ceb");
    left_bound = -1;
    right_bound = -1;
    memset(valid_array, 0, sizeof(valid_array));
    for (tx_ceb = 0; tx_ceb <= volume; tx_ceb++) {
        bool valid;

        psram_phy->REG_054 = PSRAM_ULP_PHY_REG_PSRAM_TX_CEB_DLY(tx_ceb * inc_delay) | PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DLY(delay / 2) |
                             PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY(delay / 2) | PSRAM_ULP_PHY_REG_PSRAM_RX_DQS_DLY(delay / 2);

        valid = psramphy_check_tx_ceb_valid(range);
        PSRAM_TRACE(3, "%s, tx_ceb:0x%x, valid:%d", __FUNCTION__, (tx_ceb * inc_delay), valid);
        valid_array[tx_ceb] = valid;
        if (tx_ceb == 0) {
            if (valid) {
                left_bound = tx_ceb;
                PSRAM_TRACE(1, "left_bound:0x%x", left_bound * inc_delay);
            }
        } else if (tx_ceb == volume) {
            if (valid) {
                right_bound = tx_ceb;//find right bound
                PSRAM_TRACE(1, "right_bound:0x%x", right_bound * inc_delay);
            } else {
                if (valid_array[tx_ceb - 1]) {
                    right_bound = tx_ceb - 1; //find right bound
                    PSRAM_TRACE(1, "right_bound:0x%x", right_bound * inc_delay);
                }
            }
        } else {
            if (valid && !valid_array[tx_ceb - 1]) {
                left_bound = tx_ceb;//find left bound
                PSRAM_TRACE(1, "left_bound:0x%x", left_bound * inc_delay);
            } else if (!valid && valid_array[tx_ceb - 1]) {
                right_bound = tx_ceb - 1; //find right bound
                PSRAM_TRACE(1, "right_bound:0x%x", right_bound * inc_delay);
                break;
            }
        }
    }
    if (left_bound != -1 && right_bound != -1) { // && (right_bound-left_bound>1)) {
        tx_ceb = (left_bound + right_bound) / 2;
        PSRAM_TRACE(3, "cali tx_ceb done, tx_ceb:0x%x(0x%x-0x%x)", tx_ceb * inc_delay, left_bound * inc_delay, right_bound * inc_delay);
    } else {
        ASSERT(false, "ERROR, tx_ceb need do more cali");
    }
    tx_ceb *= inc_delay;
#endif
#if 0
    //calibrate tx_clk
    PSRAM_TRACE(0, "cali tx_clk");
    left_bound = -1;
    right_bound = -1;
    memset(valid_array, 0, sizeof(valid_array));
    for (tx_clk = 0; tx_clk <= volume; tx_clk++) {
        bool valid;

        psram_phy->REG_054 = PSRAM_ULP_PHY_REG_PSRAM_TX_CEB_DLY(tx_ceb) | PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DLY(tx_clk * inc_delay) |
                             PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY(delay / 2) | PSRAM_ULP_PHY_REG_PSRAM_RX_DQS_DLY(delay / 2);
        valid = psramphy_check_tx_ceb_valid(range);
        //PSRAM_TRACE(3, "%s, tx_clk:0x%x, valid:%d", __FUNCTION__, (tx_clk*inc_delay), valid);
        valid_array[tx_clk] = valid;
        if (tx_clk == 0) {
            if (valid) {
                left_bound = tx_clk;
                PSRAM_TRACE(1, "left_bound:0x%x", left_bound * inc_delay);
            }
        } else if (tx_clk == volume) {
            if (valid) {
                right_bound = tx_clk;//find right bound
                PSRAM_TRACE(1, "right_bound:0x%x", right_bound * inc_delay);
            } else {
                if (valid_array[tx_clk - 1]) {
                    right_bound = tx_clk - 1; //find right bound
                    PSRAM_TRACE(1, "right_bound:0x%x", right_bound * inc_delay);
                }
            }
        } else {
            if (valid && !valid_array[tx_clk - 1]) {
                left_bound = tx_clk;//find left bound
                PSRAM_TRACE(1, "left_bound:0x%x", left_bound * inc_delay);
            } else if (!valid && valid_array[tx_clk - 1]) {
                right_bound = tx_clk - 1; //find right bound
                PSRAM_TRACE(1, "right_bound:0x%x", right_bound * inc_delay);
                break;
            }
        }
    }
    if (left_bound != -1 && right_bound != -1) { // && (right_bound-left_bound>1)) {
        tx_clk = (left_bound + right_bound) / 2;
        PSRAM_TRACE(3, "cali tx_clk done, tx_clk:0x%x(0x%x-0x%x)", tx_clk * inc_delay, left_bound * inc_delay, right_bound * inc_delay);
    } else {
        ASSERT(false, "ERROR, tx_clk need do more cali");
    }
    tx_clk *= inc_delay;
#endif

#ifdef PSRAM_WINDOW_TEST    //new added,Use when testing window,2021-08-28
    volume = (PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY_MASK >> PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY_SHIFT) / inc_delay;
    PSRAM_TRACE(2, "volume:%d, inc_delay:%d", volume, inc_delay);

    //calibrate tx_clk
    PSRAM_TRACE(0, "cali tx_clk");
    left_bound = -1;
    right_bound = -1;
    memset(valid_array, 0, sizeof(valid_array));
    for (tx_clk = 1; tx_clk <= volume; tx_clk++) {
        bool valid;
        tx_ceb = tx_clk - 1;
        psram_phy->REG_054 = PSRAM_ULP_PHY_REG_PSRAM_TX_CEB_DLY(tx_ceb * inc_delay) | PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DLY(tx_clk * inc_delay) |
                             PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY(delay / 2) | PSRAM_ULP_PHY_REG_PSRAM_RX_DQS_DLY(delay / 2);
        valid = psramphy_check_tx_clk_valid(range);
        //PSRAM_TRACE(3, "tx_clk:0x%x, valid:%d", (tx_clk*inc_delay), valid);
        valid_array[tx_clk] = valid;
        if (tx_clk == 1) {
            if (valid) {
                left_bound = tx_clk;
                PSRAM_TRACE(1, "left_bound:0x%x", left_bound * inc_delay);
            }
        } else if (tx_clk == volume) {
            if (valid) {
                right_bound = tx_clk;//find right bound
                PSRAM_TRACE(1, "right_bound:0x%x", right_bound * inc_delay);
            } else {
                if (valid_array[tx_clk - 1]) {
                    right_bound = tx_clk - 1; //find right bound
                    PSRAM_TRACE(1, "right_bound:0x%x", right_bound * inc_delay);
                }
            }
        } else {
            if (valid && !valid_array[tx_clk - 1]) {
                left_bound = tx_clk;//find left bound
                PSRAM_TRACE(1, "left_bound:0x%x", left_bound * inc_delay);
            } else if (!valid && valid_array[tx_clk - 1]) {
                right_bound = tx_clk - 1; //find right bound
                PSRAM_TRACE(1, "right_bound:0x%x", right_bound * inc_delay);
                break;
            }
        }
    }
    if (left_bound != -1 && right_bound != -1) {// && (right_bound-left_bound>1)) {
        tx_clk = (left_bound + right_bound) / 2;
        PSRAM_TRACE(3, "cali tx_clk done, tx_clk:0x%x(0x%x-0x%x)", tx_clk * inc_delay, left_bound * inc_delay, right_bound * inc_delay);

#ifdef PSRAM_WINDOW_TEST
        if ((last_left_bound != left_bound) || (last_right_bound != right_bound)) {
            last_left_bound = left_bound;
            last_right_bound = right_bound;
            TRACE_IMM(TR_ATTR_NO_LF | TR_ATTR_NO_TS | TR_ATTR_NO_ID, ", tx_clk:0x%02x(0x%02x-0x%02x)", tx_clk * inc_delay, left_bound * inc_delay, right_bound * inc_delay);
        } else {
            TRACE_IMM(TR_ATTR_NO_LF | TR_ATTR_NO_TS | TR_ATTR_NO_ID, ", **********************");
        }
#endif
    } else {
        ASSERT(false, "ERROR, tx_clk need do more cali");
    }
    tx_ceb = (tx_clk - 1) * inc_delay;
    tx_clk = tx_clk * inc_delay;
#endif

    PSRAM_TRACE(2, "tx_ceb:0x%x, tx_clk:0x%x", tx_ceb, tx_clk);

    volume = MIN(delay, (PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY_MASK >> PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY_SHIFT)) / inc_delay;
    PSRAM_TRACE(2, "volume:%d, inc_delay:%d", volume, inc_delay);

    //calibrate tx_dqs and rx_dqs
    uint8_t all_valid = 1;
    memset(cali_valid, 0, sizeof(cali_valid));
    for (tx_dqs = 0; tx_dqs <= volume; tx_dqs++) {
        for (rx_dqs = 0; rx_dqs <= volume; rx_dqs++) {
#ifdef PSRAM_XCCELA_MODE
            psram_phy->REG_054 = PSRAM_ULP_PHY_REG_PSRAM_TX_CEB_DLY(tx_ceb) | PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DLY(tx_dqs * inc_delay) |
                                 PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY(0) | PSRAM_ULP_PHY_REG_PSRAM_RX_DQS_DLY(rx_dqs * inc_delay);
#else
            psram_phy->REG_054 = PSRAM_ULP_PHY_REG_PSRAM_TX_CEB_DLY(tx_ceb) | PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DLY(tx_clk) |
                                 PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY(tx_dqs * inc_delay) | PSRAM_ULP_PHY_REG_PSRAM_RX_DQS_DLY(rx_dqs * inc_delay);
#endif
            cali_valid[tx_dqs][rx_dqs] = psramphy_check_write_valid();
            //PSRAM_TRACE(3, "tx_dqs:0x%x, rx_dqs:0x%x, valid:%d", tx_dqs*inc_delay, rx_dqs*inc_delay, cali_valid[tx_dqs][rx_dqs]);
            if (cali_valid[tx_dqs][rx_dqs] == false)
                all_valid = 0;
        }
    }

    if (all_valid && range < (PSRAM_ULP_PHY_DLL_DLY_IN_MASK >> PSRAM_ULP_PHY_DLL_DLY_IN_SHIFT)) {
        //PSRAM_TRACE(2, "%s: all valid increase range=%d", __func__, range+1);
        //return hal_psram_calib_range(range+1);
    }

    memset(cali_value, 0, sizeof(cali_value));
    PSRAM_TRACENOCRLF_NOTS(0, "\r\n\r\n ---------------------------------------------------------------------- \r\n");
    PSRAM_TRACENOCRLF_NOTS(0, "    rx_dqs");
    for (tx_dqs = 0; tx_dqs <= volume; tx_dqs++) {
        PSRAM_TRACENOCRLF_NOTS(1, " %2d ", tx_dqs * inc_delay);
    }
    PSRAM_TRACENOCRLF_NOTS(0, "\r\n");
    for (tx_dqs = 0; tx_dqs <= volume; tx_dqs++) {
        PSRAM_TRACENOCRLF_NOTS(1, "tx_dqs:%2d ", tx_dqs * inc_delay);
        for (rx_dqs = 0; rx_dqs <= volume; rx_dqs++) {
            PSRAM_TRACENOCRLF_NOTS(1, "  %d ", cali_valid[tx_dqs][rx_dqs]);
            if (cali_valid[tx_dqs][rx_dqs]) {
                uint8_t len_from_zero;
                int8_t p;
                p = tx_dqs;
                while (p >= 0) {
                    if (cali_valid[p][rx_dqs] == false)
                        break;
                    p--;
                }
                len_from_zero = tx_dqs - p;
                cali_value[tx_dqs][rx_dqs] = len_from_zero;

                p = tx_dqs;
                while (p <= volume) {
                    if (cali_valid[p][rx_dqs] == false)
                        break;
                    p++;
                }
                len_from_zero = p - tx_dqs;
                cali_value[tx_dqs][rx_dqs] = MIN(cali_value[tx_dqs][rx_dqs], len_from_zero);

                p = rx_dqs;
                while (p >= 0) {
                    if (cali_valid[tx_dqs][p] == false)
                        break;
                    p--;
                }
                len_from_zero = rx_dqs - p;
                cali_value[tx_dqs][rx_dqs] = MIN(cali_value[tx_dqs][rx_dqs], len_from_zero);

                p = rx_dqs;
                while (p <= volume) {
                    if (cali_valid[tx_dqs][p] == false)
                        break;
                    p++;
                }
                len_from_zero = p - rx_dqs;
                cali_value[tx_dqs][rx_dqs] = MIN(cali_value[tx_dqs][rx_dqs], len_from_zero);
            }
        }
        PSRAM_TRACENOCRLF_NOTS(0, "\r\n");
    }
    PSRAM_TRACENOCRLF_NOTS(0, " -------------------------------------------------------------------------- \r\n");

#ifdef PSRAM_WINDOW_TEST
    //Processing window data
    uint8_t cp_volume = volume + 1;
    uint8_t enable_clear_circle = 0;
    uint8_t enable_print_window = 0;
    uint8_t rows;

#if 1   //Window inner bag 0 test,2021-08-30
    #define HW_READ_REG(_Reg)           (*((volatile uint32_t*)(_Reg)))
    #define HW_WRITE_REG(_Reg,_Value)   (*((volatile uint32_t*)(_Reg)) = (uint32_t)(_Value))
    if (HW_READ_REG(0x400000F4) == 0xCAFE) {
        HW_WRITE_REG(0x400000F4, 0x9A55);
        cali_valid[4][4] = 0;//Central point clear
    }
#endif

    for (tx_dqs = 0; tx_dqs < cp_volume; tx_dqs++) {
        for (rx_dqs = 0; rx_dqs < cp_volume; rx_dqs++) {
            cali_data[tx_dqs][rx_dqs] = cali_valid[tx_dqs][rx_dqs] + '0';
        }
    }

    if (cp_volume % 2) {
        rows = cp_volume / 2;
    } else {
        rows = cp_volume / 2 - 1;
    }
    for (tx_dqs = 0; tx_dqs < rows; tx_dqs++) {
        //Check the starting line
        for (rx_dqs = tx_dqs; rx_dqs < cp_volume - tx_dqs; rx_dqs++) {
            if (cali_data[tx_dqs][rx_dqs] == '0') {
                break;
            }
        }

        if (rx_dqs >= cp_volume - tx_dqs) {
            //Check the end of the line
            for (rx_dqs = tx_dqs; rx_dqs < cp_volume - tx_dqs; rx_dqs++) {
                if (cali_data[volume - tx_dqs][rx_dqs] == '0') {
                    break;
                }
            }
            if (rx_dqs >= cp_volume - tx_dqs) {
                //Check the starting column
                for (rx_dqs = tx_dqs; rx_dqs < cp_volume - tx_dqs; rx_dqs++) {
                    if (cali_data[rx_dqs][tx_dqs] == '0') {
                        break;
                    }
                }
                if (rx_dqs >= cp_volume - tx_dqs) {
                    //Check the end of the column
                    for (rx_dqs = tx_dqs; rx_dqs < cp_volume - tx_dqs; rx_dqs++) {
                        if (cali_data[rx_dqs][volume - tx_dqs] == '0') {
                            break;
                        }
                    }
                    if (rx_dqs >= cp_volume - tx_dqs) {
                        enable_clear_circle = 0;
                    } else {
                        enable_clear_circle = 1;
                    }
                } else {
                    enable_clear_circle = 1;
                }
            } else {
                enable_clear_circle = 1;
            }
        } else {
            enable_clear_circle = 1;
        }

        //The current line has 0, directly clear the current circle
        if (enable_clear_circle) {
            enable_clear_circle = 0;

            //clear the starting line
            for (rx_dqs = tx_dqs; rx_dqs < cp_volume - tx_dqs; rx_dqs++) {
                cali_data[tx_dqs][rx_dqs] = ' ';
            }

            //clear the end of the line
            for (rx_dqs = tx_dqs; rx_dqs < cp_volume - tx_dqs; rx_dqs++) {
                cali_data[volume - tx_dqs][rx_dqs] = ' ';
            }

            //clear the starting column
            for (rx_dqs = tx_dqs + 1; rx_dqs < cp_volume - tx_dqs - 1; rx_dqs++) {
                cali_data[rx_dqs][tx_dqs] = ' ';
            }

            //clear the end of the column
            for (rx_dqs = tx_dqs + 1; rx_dqs < cp_volume - tx_dqs - 1; rx_dqs++) {
                cali_data[rx_dqs][volume - tx_dqs] = ' ';
            }
        } else {
            //Check if there is 0 in the middle of the ring
            uint8_t i;

            enable_print_window = 0;
            for (i = tx_dqs; i < cp_volume - tx_dqs * 2 + tx_dqs; i++) {
                //Check the remaining rows
                for (rx_dqs = tx_dqs; rx_dqs < cp_volume - tx_dqs * 2 + tx_dqs; rx_dqs++) {
                    if (cali_data[i][rx_dqs] == '0') {
                        break;
                    }
                }
                if (rx_dqs < cp_volume - tx_dqs * 2 + tx_dqs) {
                    enable_print_window = 1;
                    break;
                }
            }
            break;
        }
    }

    //Only the middle window is left, print it out forcibly
    if (tx_dqs >= rows) {
        enable_print_window = 1;
    }

    dqs_point = tx_dqs;
    dqs_len = cp_volume - tx_dqs * 2;

    if (last_dqs_point != dqs_point) {
        last_dqs_point = dqs_point;

        TRACE_IMM(TR_ATTR_NO_LF | TR_ATTR_NO_TS | TR_ATTR_NO_ID, ", rx_dqs:(%02d,%02d)~(%02d,%02d)",
                  dqs_point * inc_delay, dqs_point * inc_delay,
                  (dqs_point + dqs_len - 1)*inc_delay, dqs_point * inc_delay);
    } else {
        TRACE_IMM(TR_ATTR_NO_LF | TR_ATTR_NO_TS | TR_ATTR_NO_ID, ", ++++++++++++++++++++++");
    }

    if (last_dqs_len != dqs_len) {
        last_dqs_len = dqs_len;

        if (enable_print_window) {
            TRACE_IMM(TR_ATTR_NO_LF | TR_ATTR_NO_TS | TR_ATTR_NO_ID, ", tx_dqs:(%02d,%02d)~(%02d,%02d), abnormal",
                      dqs_point * inc_delay, dqs_point * inc_delay,
                      dqs_point * inc_delay, (dqs_point + dqs_len - 1)*inc_delay);
        } else {
            TRACE_IMM(TR_ATTR_NO_LF | TR_ATTR_NO_TS | TR_ATTR_NO_ID, ", tx_dqs:(%02d,%02d)~(%02d,%02d), normal",
                      dqs_point * inc_delay, dqs_point * inc_delay,
                      dqs_point * inc_delay, (dqs_point + dqs_len - 1)*inc_delay);
        }
    } else {
        if (enable_print_window) {
            TRACE_IMM(TR_ATTR_NO_LF | TR_ATTR_NO_TS | TR_ATTR_NO_ID, ", ----------------------, abnormal");
        } else {
            TRACE_IMM(TR_ATTR_NO_LF | TR_ATTR_NO_TS | TR_ATTR_NO_ID, ", ----------------------, ######");
        }
    }
    TRACE_IMM(TR_ATTR_NO_TS | TR_ATTR_NO_ID, ", vol:%02d, delay:%02d", volume, inc_delay);

    if (enable_print_window) {
        enable_print_window = 0;

        //Print original window data
        REL_TRACE_NOTS(0, " ");
        REL_TRACE_NOCRLF_NOTS(0, "   rx_dqs ");
        for (tx_dqs = 0; tx_dqs <= volume; tx_dqs++) {
            REL_TRACE_NOCRLF_NOTS(1, " %2d ", tx_dqs * inc_delay);
        }
        REL_TRACE_NOCRLF_NOTS(0, "\r\n");
        for (tx_dqs = 0; tx_dqs <= volume; tx_dqs++) {
            REL_TRACE_NOCRLF_NOTS(1, "tx_dqs:%2d ", tx_dqs * inc_delay);
            for (rx_dqs = 0; rx_dqs <= volume; rx_dqs++) {
                REL_TRACE_NOCRLF_NOTS(1, "  %d ", cali_valid[tx_dqs][rx_dqs]);
            }
            REL_TRACE_NOCRLF_NOTS(0, "\r\n");
        }
        REL_TRACE_NOTS(0, " ");

        //Print new window data
        REL_TRACE_NOCRLF_NOTS(0, "   rx_dqs ");
        for (tx_dqs = 0; tx_dqs <= volume; tx_dqs++) {
            REL_TRACE_NOCRLF_NOTS(1, " %2d ", tx_dqs * inc_delay);
        }
        REL_TRACE_NOCRLF_NOTS(0, "\r\n");
        for (tx_dqs = 0; tx_dqs <= volume; tx_dqs++) {
            REL_TRACE_NOCRLF_NOTS(1, "tx_dqs:%2d ", tx_dqs * inc_delay);
            for (rx_dqs = 0; rx_dqs <= volume; rx_dqs++) {
                REL_TRACE_NOCRLF_NOTS(1, "  %c ", cali_data[tx_dqs][rx_dqs]);
            }
            REL_TRACE_NOCRLF_NOTS(0, "\r\n");
        }
        REL_TRACE_NOTS(0, " ");
    }
#endif

#if 0
    //Print cali_value window data
    PSRAM_TRACENOCRLF_NOTS(0, "\r\n\r\n ---------------------------------------------------------------------- \r\n");
    PSRAM_TRACENOCRLF_NOTS(0, "    rx_dqs");
    for (tx_dqs = 0; tx_dqs <= volume; tx_dqs++) {
        PSRAM_TRACENOCRLF_NOTS(1, " %2d ", tx_dqs * inc_delay);
    }
    PSRAM_TRACENOCRLF_NOTS(0, "\r\n");
    for (tx_dqs = 0; tx_dqs <= volume; tx_dqs++) {
        PSRAM_TRACENOCRLF_NOTS(1, "tx_dqs:%2d ", tx_dqs * inc_delay);
        for (rx_dqs = 0; rx_dqs <= volume; rx_dqs++) {
            PSRAM_TRACENOCRLF_NOTS(1, "  %d ", cali_value[tx_dqs][rx_dqs]);
        }
        PSRAM_TRACENOCRLF_NOTS(0, "\r\n");
    }
    PSRAM_TRACENOCRLF_NOTS(0, " -------------------------------------------------------------------------- \r\n");
#endif

    uint32_t position = 0;
    uint8_t max_value = 0;
    for (tx_dqs = 0; tx_dqs <= volume; tx_dqs++) {
        for (rx_dqs = 0; rx_dqs <= volume; rx_dqs++) {
            if (cali_value[tx_dqs][rx_dqs] > max_value) {
                max_value = cali_value[tx_dqs][rx_dqs];
                position = tx_dqs * (volume + 1) + rx_dqs;
            }
        }
    }
    PSRAM_TRACENOCRLF_NOTS(1, "position:%d\r\n", position);
    tx_dqs = position / (volume + 1) * inc_delay;
    rx_dqs = (position % (volume + 1)) * inc_delay;
    PSRAM_TRACENOCRLF_NOTS(2, "most optimal position. tx_dqs:%d, rx_dqs:%d\r\n", tx_dqs, rx_dqs);
#ifdef PSRAM_XCCELA_MODE
    psram_phy->REG_054 = PSRAM_ULP_PHY_REG_PSRAM_TX_CEB_DLY(tx_ceb) | PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DLY(tx_dqs) |
                         PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY(0) | PSRAM_ULP_PHY_REG_PSRAM_RX_DQS_DLY(rx_dqs);
#else
    psram_phy->REG_054 = PSRAM_ULP_PHY_REG_PSRAM_TX_CEB_DLY(tx_ceb) | PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DLY(tx_clk) |
                         PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY(tx_dqs) | PSRAM_ULP_PHY_REG_PSRAM_RX_DQS_DLY(rx_dqs);
#endif

#if 0
    volume = (PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY_MASK >> PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY_SHIFT) / inc_delay;
    PSRAM_TRACE(2, "volume:%d, inc_delay:%d", volume, inc_delay);

    //calibrate tx_clk
    PSRAM_TRACE(0, "cali tx_clk");
    left_bound = -1;
    right_bound = -1;
    memset(valid_array, 0, sizeof(valid_array));
    for (tx_clk = 1; tx_clk <= volume; tx_clk++) {
        bool valid;
        //tx_ceb = tx_clk;
        psram_phy->REG_054 = PSRAM_ULP_PHY_REG_PSRAM_TX_CEB_DLY(tx_ceb * inc_delay) | PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DLY(tx_clk * inc_delay) |
                             PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY(delay / 2) | PSRAM_ULP_PHY_REG_PSRAM_RX_DQS_DLY(delay / 2);
        valid = psramphy_check_write_valid(range);
        PSRAM_TRACE(3, "tx_clk:0x%x, valid:%d", (tx_clk * inc_delay), valid);
        valid_array[tx_clk] = valid;
        if (tx_clk == 1) {
            if (valid) {
                left_bound = tx_clk;
                PSRAM_TRACE(1, "left_bound:0x%x", left_bound * inc_delay);
            }
        } else if (tx_clk == volume) {
            if (valid) {
                right_bound = tx_clk;//find right bound
                PSRAM_TRACE(1, "right_bound:0x%x", right_bound * inc_delay);
            } else {
                if (valid_array[tx_clk - 1]) {
                    right_bound = tx_clk - 1; //find right bound
                    PSRAM_TRACE(1, "right_bound:0x%x", right_bound * inc_delay);
                }
            }
        } else {
            if (valid && !valid_array[tx_clk - 1]) {
                left_bound = tx_clk;//find left bound
                PSRAM_TRACE(1, "left_bound:0x%x", left_bound * inc_delay);
            } else if (!valid && valid_array[tx_clk - 1]) {
                right_bound = tx_clk - 1; //find right bound
                PSRAM_TRACE(1, "right_bound:0x%x", right_bound * inc_delay);
                break;
            }
        }
    }
    if (left_bound != -1 && right_bound != -1) {// && (right_bound-left_bound>1)) {
        tx_clk = (left_bound + right_bound) / 2;
        PSRAM_TRACE(3, "cali tx_clk done, tx_clk:0x%x(0x%x-0x%x)", tx_clk * inc_delay, left_bound * inc_delay, right_bound * inc_delay);
    } else {
        ASSERT(false, "ERROR, tx_clk need do more cali");
    }
    //tx_ceb = tx_clk * inc_delay;
    tx_clk = tx_clk * inc_delay;
#endif
}
void hal_psram_calib(uint32_t clk)
{
    uint32_t phy_clk;
    uint32_t range;
    PSRAM_TRACE(2, "%s, speed:%d", __func__, clk);
    phy_clk = clk;
    if (phy_clk <= 100000000 / 2) {
        range = 3;
    } else if (phy_clk <= 150000000 / 2) {
        range = 2;
    } else if (phy_clk <= 300000000 / 2) {
        range = 1;
    } else {
        range = 1;
    }
#ifdef PSRAM_WINDOW_TEST
    TRACE_IMM(TR_ATTR_NO_LF, "clk:%d, range:%d", clk, range);
#endif
    hal_psram_calib_range(range);
}

void hal_psram_init(void)
{
    pmu_psram_freq_config(psram_run_clk);

    hal_cache_wrap_enable(HAL_CACHE_ID_I_CACHE);
    hal_cache_wrap_enable(HAL_CACHE_ID_D_CACHE);

    hal_cmu_mem_set_freq(HAL_CMU_FREQ_104M);
    hal_cmu_clock_enable(HAL_CMU_MOD_O_PSRAM);
    hal_cmu_clock_enable(HAL_CMU_MOD_H_PSRAM);
    hal_cmu_reset_clear(HAL_CMU_MOD_O_PSRAM);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_PSRAM);

#ifndef FPGA
    hal_psram_phy_init(psram_cfg_clk);
    hal_sys_timer_delay_us(100);
#endif
    hal_psram_digphy_init();
    hal_psram_mc_init(psram_cfg_clk);
#if !defined(FPGA) && !defined(SIMU)
    hal_psram_init_calib();
#endif

#ifdef PSRAM_RESET
    psram_reset();
    psram_chip_timing_config(psram_run_clk, true);
#else
    uint32_t reg;
    uint32_t val;

    reg = 4;
    psram_read_reg(reg, &val);
    if (val & MR4_WL) {
        psram_chip_timing_config(psram_run_clk, false);
    } else {
        psram_chip_timing_config(psram_run_clk, true);
    }
#endif

    hal_psram_snoop_disable();
#if !defined(FPGA) && !defined(SIMU)
    hal_psram_calib(psram_run_clk);
#endif
    hal_psram_snoop_enable();
}

#endif

