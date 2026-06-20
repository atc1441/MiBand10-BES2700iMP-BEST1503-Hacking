/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#include "plat_addr_map.h"
#if defined(DSI_BASE) && defined(LCDC_BASE)
#include "plat_types.h"
#include "cmsis.h"
#include "cmsis_nvic.h"
#include "hal_cmu.h"
#include "hal_dsi.h"
#include "hal_gpio.h"
#include "hal_iomux.h"
#include "hal_lcdc.h"
#include "hal_psc.h"
#include "hal_timer.h"
#include "hwtimer_list.h"
#include "hal_trace.h"
#include "reg_dsi.h"
#if !defined(__NuttX__)
#include "graphic_conf.h"
#endif

#ifdef CSI_BASE
#define CHIP_HAS_MIPIPHY
#endif

#ifdef CHIP_HAS_MIPIPHY
#include "mipiphy.h"
#endif

static struct DSI_REG_T * const dsi = (struct DSI_REG_T *)DSI_BASE;

enum DSI_CMD_TYPE_T {
    CLN_ENT_LP = 0,
    CLN_EXIT_LP = 1,
    CLN_ENT_ULPS = 2,
    CLN_EXIT_ULPS = 3,
    DLN_LPT = 4,
    DLN_HST = 5,
    DLN_LPT_BTA = 6,
    DLN_HST_BTA = 7,
};

enum DSI_CMD_T {
    DSC_SHORT_WRITE = 0x05,
    DSC_SHORT_WRITE_1PAR = 0x15,
    DSC_LONG_WRITE = 0x39
};

static bool ulps_enter = false;

static HAL_DSI_TE_GPIO_IRQ_HANDLER te_gpio_hdlr;
static HAL_DSI_ULPS_CB_HANDLER ulps_exit_hdlr;
static HWTIMER_ID ulps_exit_timer = NULL;

static enum HAL_GPIO_PIN_T te_gpio_pin;

/**
 * @brief hal_dsi_lpbusy_timeout - wait lpcmd status busy bit with timout
 *
 * @param ticks : wait time
 * @return int : 0 success, -1 timeout
 */
__attribute__((unused)) static int hal_dsi_lpbusy_timeout(uint32_t ticks)
{
    uint32_t start = hal_sys_timer_get();

    while (hal_sys_timer_get() - start < ticks) {
        if (!(dsi->REG_014 & (1 << 4))) {
            return 0;
        }
    }

    return -1;
}

#if !defined(CHIP_HAS_MIPIPHY)
/**
 * @brief hal_dsiphy_open - open dsi phy
 *
 */
static void hal_dsiphy_open(void)
{
    dsi->REG_070 |= REG_CHCK_EN_DSI;
    dsi->REG_070 = SET_BITFIELD(dsi->REG_070, REG_CHCK_DLY_TUNE_DSI, 0x4);
    dsi->REG_070 = SET_BITFIELD(dsi->REG_070, REG_CHCK_HSTX_DRV_DSI, 0xF);
    dsi->REG_070 = SET_BITFIELD(dsi->REG_070, REG_CHCK_LPTX_DRV_DSI, 0x4);

#if defined(CONFIG_SWAP_DSI_CLK_PN)
    dsi->REG_070 |= REG_CHCK_SWAP_PN_DSI;
#endif
    //dsi->REG_070 |= REG_PRECHARGE_LDO_CNT(0xf);
#if defined(CONFIG_SWAP_DSI_LANE0_PN)
    dsi->REG_068 |= REG_CH0_SWAP_PN_DSI;
#endif

#if defined(CONFIG_SWAP_DSI_LANE1_PN)
    dsi->REG_06C |= REG_CH1_SWAP_PN_DSI;
#endif
#if 0
    dsi->REG_060 = 0x7e1F;
    dsi->REG_064 = 0x924258;
#else
    dsi->REG_064 = SET_BITFIELD(dsi->REG_064, REG_LDO0P4_RES, 0x8);
    dsi->REG_064 = SET_BITFIELD(dsi->REG_064, REG_LDO0P8_RES, 0x5);
    dsi->REG_064 = SET_BITFIELD(dsi->REG_064, REG_LDO1P2_RES, 0x2);
    dsi->REG_064 = SET_BITFIELD(dsi->REG_064, REG_VB_LPCD_H, 0x4);
    dsi->REG_064 = SET_BITFIELD(dsi->REG_064, REG_VB_LPCD_L, 0x2);
    dsi->REG_064 = SET_BITFIELD(dsi->REG_064, REG_VB_LPRX_H, 0x4);
    dsi->REG_064 = SET_BITFIELD(dsi->REG_064, REG_VB_LPRX_L, 0x4);

    dsi->REG_060 |= REG_EN_LPRX0_DSI;
    dsi->REG_060 = SET_BITFIELD(dsi->REG_060, REG_CLK_SWRC_DSI, 0x7);
    dsi->REG_060 |= REG_CLKGEN_PU_DSI;
    dsi->REG_060 |= REG_PU_CH_DSI;
    dsi->REG_060 |= REG_PU_LDO_DSI;
    dsi->REG_060 |= REG_DSI_PHY_RSTN;
    dsi->REG_060 |= REG_MASK_LP_LDO;
    dsi->REG_060 |= REG_MASK_LP_RSTN;
    dsi->REG_060 |= REG_CKX8_EN;
#endif
    dsi->REG_060 |= REG_PRECHARGE_LDO_DSI;
//    hal_sys_timer_delay(MS_TO_TICKS(1));
    dsi->REG_060 &= ~REG_PRECHARGE_LDO_DSI;
}

static void hal_dsiphy_sleep(void)
{
    dsi->REG_060 &= ~REG_PU_LDO_DSI;
}

static void hal_dsiphy_wakeup(void)
{
    dsi->REG_060 |= REG_PU_LDO_DSI;
    hal_sys_timer_delay_us(100);
}
#endif

/**
 * @brief hal_dsi_read_cmd - read data by lp mode
 * packet format:
 * ShortPacket: DI DATA0 DATA1 ECC
 * LongPacket:  DI WC0   WC1   ECC DATA[0]-DATA[WC-1] CS0 CS1
 * e.g. read id: hal_dsi_read_cmd(0x04, buf, 3)
 * @param cmd : cmd
 * @param read_buf : read buffer
 * @param len : read len
 * @return int
 */
int hal_dsi_read_cmd(uint8_t cmd, uint8_t *recv_buf, uint8_t len)
{
    volatile uint32_t temp;
    uint8_t fifo_len;
    uint8_t buf[32];
    uint8_t buf_len = 0;
    uint32_t time;

    dsi->REG_010 = (dsi->REG_010 & ~DSI_FLAG_MASK_MASK) | DSI_LP_RX_DONE_FLAG;

    dsi->REG_018 = (len << 8) | 0x37;
    dsi->REG_014 =  (5 << 16) | (0xe1 << 8) | DLN_LPT;
    hal_sys_timer_delay_us(20);
    dsi->REG_018 = (cmd << 8)| 0x06;
    dsi->REG_014 = (5 << 16) | (0xe1 << 8) | DLN_LPT_BTA;
    hal_sys_timer_delay_us(10);

    time = hal_sys_timer_get();
    while (1) {
        temp = dsi->REG_010;
        if (temp & DSI_LPRX_SIZE_MASK) {
            buf[buf_len ++] = temp;
        } else if (temp & (DSI_RX_ERR_FLAG | DSI_RX_TIMEOUT_FLAG)) {
            dsi->REG_010 = (dsi->REG_010 & ~DSI_FLAG_MASK_MASK) | DSI_RX_ERR_FLAG | DSI_RX_TIMEOUT_FLAG;
            TRACE(0, "rx error:0x%x", temp);
            return -1;
        } else if ((temp & DSI_LP_RX_DONE_FLAG) && (temp & DSI_LPRX_SIZE_MASK) == 0) {
            //TRACE(0, "rx done:0x%x", temp);
            break;
        } else if (hal_sys_timer_get() - time > MS_TO_TICKS(1000)) {
            TRACE(0, "rx timerout:0x%x", temp);
            return -2;
        }
        hal_sys_timer_delay_us(1);
    }

    fifo_len = GET_BITFIELD(temp, DSI_LPRX_SIZE);
    if (fifo_len > 1) {
        TRACE(0, "warning: fifo is not empty");
        while (fifo_len > 1) {
            temp = dsi->REG_010;
            fifo_len --;
        }
    }

    if (buf_len >= 2 && buf[1] == 0x02) {
        TRACE(0, "Acknowledge error report");
        return -1;
    } else if (len == 1 && buf_len >= 3 && buf[1] == 0x21) { //short packet
        recv_buf[0] = buf[2];
        return 1;
    } else if (len == 2 && buf_len >= 4 && buf[1] == 0x22) { //short packet
        recv_buf[0] = buf[2];
        recv_buf[1] = buf[3];
        return 2;
    } else if (buf_len >= 10 && buf[1] == 0x1c) { //long packet
        uint16_t word_cnt = (buf[3] >> 8) | buf[2];
        uint16_t i;
        if (word_cnt < len) {
            TRACE(0, "warning: word_cnt:%d < len:%d", word_cnt, len);
            len = word_cnt;
        }
        for (i = 0; i < len; ++i)
            recv_buf[i] = buf[5 + i];
        return len;
    } else {
        TRACE(0, "error, buf_len:%d, len:%d", buf_len, len);
        DUMP8("%02x ", buf, buf_len);
        return -3;
    }

    return 0;
}

/**
 * @brief hal_dsi_send_cmd - send cmd by lp mode
 *
 * @param cmd
 */
void hal_dsi_send_cmd(uint8_t cmd)
{
    dsi->REG_018 = (cmd << 8) | DSC_SHORT_WRITE;
    dsi->REG_014 = (5 << 16) | (0xe1 << 8) | DLN_LPT;

    hal_dsi_lpbusy_timeout(US_TO_TICKS(30));
}

/**
 * @brief hal_dsi_send_cmd_data - send cmd with data by lp mode
 *
 * @param cmd : cmd
 * @param len : send param num
 * @param p0 : param0
 * @param p1 : param1
 * @param p2 : param2
 * @param p3 : param3
 */
void hal_dsi_send_cmd_data(uint8_t cmd, uint32_t len, uint8_t p0,
                           uint8_t p1, uint8_t p2, uint8_t p3)
{
    if (len == 1) {
        dsi->REG_018 = (p0 << 16) | (cmd << 8) | DSC_SHORT_WRITE_1PAR;
        dsi->REG_014 = (5 << 16) | (0xe1 << 8) | DLN_LPT;
    } else if (len == 2) {
        dsi->REG_018 = ((len+1) << 8) | DSC_LONG_WRITE;
        dsi->REG_01C = cmd |
                       (p0 << 8) |
                       (p1 << 16);
        dsi->REG_014 = ((len+1+4+3) << 16) | (0xe1 << 8) | DLN_LPT;
    } else if (len == 3) {
        dsi->REG_018 = ((len+1) << 8) | DSC_LONG_WRITE;
        dsi->REG_01C = cmd |
                       (p0 << 8) |
                       (p1 << 16) |
                       (p2 << 24);
        dsi->REG_014 = ((len+1+4+3) << 16) | (0xe1 << 8) | DLN_LPT;
    } else if (len == 4) {
        dsi->REG_018 = ((len+1) << 8) | DSC_LONG_WRITE;
        dsi->REG_01C = cmd |
                       (p0 << 8) |
                       (p1 << 16) |
                       (p2 << 24);
        dsi->REG_020 = p3;
        dsi->REG_014 = ((len+1+4+3) << 16) | (0xe1 << 8) | DLN_LPT;
    }

    hal_dsi_lpbusy_timeout(US_TO_TICKS(len * 20 + 10));
}

/**
 * @brief hal_dsi_send_long_array - send cmd with data with array
 *
 * @param len : array len
 * @param data : array data
 */
void hal_dsi_send_long_array(uint32_t len, uint32_t *data)
{
    int i;
    dsi->REG_018 = (len << 8) | DSC_LONG_WRITE;
    for(i=0;i<len/4;i++)
        dsi->REG_01C = data[i];
    int remain = len%4;
    if (remain == 1)
        dsi->REG_020 = data[i];
    else if (remain == 2)
        dsi->REG_024 = data[i];
    else if (remain == 3)
        dsi->REG_028 = data[i];
    dsi->REG_014 = ((len+4+3) << 16) | (0xe1 << 8) | DLN_LPT;

    hal_dsi_lpbusy_timeout(US_TO_TICKS(len * 20 + 10));
}

/**
 * @brief hal_dsi_send_cmd_list - send cmd with data list
 *
 * @param cmd
 * @param para_count
 * @param para_list
 */
void hal_dsi_send_cmd_list(unsigned cmd, unsigned char para_count,
                           unsigned char *para_list)
{
    unsigned int item[16]={0};
    unsigned char dsi_cmd = (unsigned char)cmd;
    unsigned char dc;
    int index = 0;
    if (para_count + 1 > 60)
            return;

    if (para_count == 0) {
            hal_dsi_send_cmd(dsi_cmd);
    } else if (para_count == 1) {
            hal_dsi_send_cmd_data(dsi_cmd, 1, para_list[0], 0x0, 0x0, 0x0);
    } else if (para_count == 2) {
            hal_dsi_send_cmd_data(dsi_cmd, 2, para_list[0], para_list[1], 0x0, 0x0);
    } else if (para_count == 3) {
            hal_dsi_send_cmd_data(dsi_cmd, 3, para_list[0], para_list[1], para_list[2], 0x0);
    } else if (para_count == 4) {
            hal_dsi_send_cmd_data(dsi_cmd, 4, para_list[0], para_list[1], para_list[2], para_list[3]);
    } else {
        while (1) {
            if (index == para_count + 1){
                break;
            }
            if (index == 0)
                dc = cmd;
            else
                dc = para_list[index - 1];
            item[index / 4 ] |= (dc << (8 * (index % 4)));
            ++index;
        }
        hal_dsi_send_long_array(index,(uint32_t *)item);
    }
}


/**
 * @brief hal_dsi_reset - reset phy, besause hs mode phy work error,
 *                        lp cmd send has some errors.
 *
 * @param h_res : panel horizontal resolution
 */
void hal_dsi_reset(uint16_t h_res)
{
    /* reset dsi phy */
    hal_cmu_dsi_phy_reset_set();
    hal_cmu_dsi_phy_reset_clear();

#if defined(CHIP_HAS_MIPIPHY)
    dsiphy_open(0);
#else
    hal_dsiphy_open();
#endif

    //DSI init-1
    dsi->REG_000 = DSI_R_LPCD_DLY(1) | DSI_R_LANE_NUM(0) |
                   DSI_R_HSA_LP | DSI_R_HSE_EN | DSI_R_HFP_LP |
                   DSI_R_EOTP_EN | DSI_R_T_LPX(0x7) | DSI_R_CLK_T_LPX(0x3) |
                   DSI_R_CLK_PRE(0x5) | DSI_R_CLK_POST(0x7);
#if defined(CONFIG_DSI_2LANE_MODE)
    dsi->REG_000 |= DSI_R_LANE_NUM(0x1);
#endif
#if defined(CONFIG_DSI_VIDEO_MODE)
    dsi->REG_000 |= DSI_R_VIDEO_MODE;
    dsi->REG_000 &=  ~(DSI_R_EOTP_EN |DSI_R_HSA_LP|DSI_R_HBP_LP);
    dsi->REG_048 &= ~(DSI_R_VIDEO_BIST_EN);
#endif

    dsi->REG_004 = DSI_R_HS_EXIT_TIME(0xb) | DSI_R_HS_PRPR_TIME(0x4) |
                   DSI_R_HS_ZERO_TIME(0xb) | DSI_R_HS_TRAIL_TIME(0xa) |
                   DSI_R_T_WAKEUP(0x207);
    dsi->REG_008 = DSI_R_CLK_EXIT_TIME(0xb) | DSI_R_CLK_PRPR_TIME(0x8) |
                   DSI_R_CLK_ZERO_TIME(0x1f) | DSI_R_CLK_TRAIL_TIME(0x8) |
                   DSI_R_CLK_T_WAKEUP(0x66);
    dsi->REG_00C = DSI_R_DTYPE(0x39) | DSI_R_VC_CH_ID(0x0) |
                   DSI_R_INPUT_TYPE(0x0) | DSI_R_DLANE_AD_TIME(0x12);

    dsi->REG_048 &= ~DSI_R_COLOR_BAR_WIDTH_MASK;
    dsi->REG_048 |= DSI_R_COLOR_BAR_WIDTH(0x100);

    dsi->REG_048 &= ~DSI_R_HSYNC_DLY_NUM_MASK;

    dsi->REG_00C &= ~DSI_R_VIDEO_PACKET_LENTH_MASK;

#if defined(CONFIG_DSIPHY_OUTFMT_RGB888)
    dsi->REG_00C |= DSI_R_INPUT_TYPE(0x0) |
                    DSI_R_VIDEO_PACKET_LENTH(h_res*3+6+1);
    dsi->REG_048 |= DSI_R_HSYNC_DLY_NUM(0x120);
#elif defined(CONFIG_DSIPHY_OUTFMT_RGB565)
    dsi->REG_00C |= DSI_R_INPUT_TYPE(0x1) |
                    DSI_R_VIDEO_PACKET_LENTH(h_res*2+6+1);
    dsi->REG_048 |= DSI_R_HSYNC_DLY_NUM(0x120);
#elif defined(CONFIG_DSIPHY_OUTFMT_LOOSELYRGB666)

#elif defined(CONFIG_DSIPHY_OUTFMT_RGB666)

#elif defined(CONFIG_DSIPHY_OUTFMT_RGB101010)

#elif defined(CONFIG_DSIPHY_OUTFMT_YCbCr24)

#elif defined(CONFIG_DSIPHY_OUTFMT_YCbCr20)

#else
    dsi->REG_00C |= DSI_R_INPUT_TYPE(0x0) |
                    DSI_R_VIDEO_PACKET_LENTH(h_res*3+6+1);
    dsi->REG_048 |= DSI_R_HSYNC_DLY_NUM(0x120);
#endif
#if defined(CONFIG_DSI_VIDEO_MODE)
    dsi->REG_00C &= ~DSI_R_DTYPE_MASK;
    dsi->REG_00C |= DSI_R_DTYPE(0x3e);
    dsi->REG_00C &= ~DSI_R_VIDEO_PACKET_LENTH_MASK;
    dsi->REG_00C |= DSI_R_VIDEO_PACKET_LENTH(h_res*3+6);
    dsi->REG_048 &= ~DSI_R_HSYNC_DLY_NUM_MASK;
    dsi->REG_048 |= DSI_R_HSYNC_DLY_NUM(0x4b4);
    dsi->REG_048 &= ~DSI_R_COLOR_BAR_WIDTH_MASK;
#endif

    //MASK(disable) all irq
    dsi->REG_010 = (dsi->REG_010 & ~(DSI_INTR_MASK_MASK | DSI_HS_CMD_RD_EN | DSI_LPRX_RD_EN)) //disalbe all irq
        | DSI_FLAG_MASK_MASK; //clear irq status
    dsi->REG_010 |= DSI_LPRX_RD_EN;// | (1<<31);//for DCS read cmd

#ifdef CONFIG_LCDC_TE_IRQ_ENABLE
    /* enable te irq */
    hal_dsi_teirq_enable();
#endif
}

/**
 * @brief hal_dsi_init - init dsi phy
 *
 * @param h_res : panel horizontal resolution
 */
void hal_dsi_init(uint16_t h_res)
{
    hal_iomux_set_dsi_te();

    hal_cmu_lcdc_clock_enable(0);
    hal_cmu_dsi_clock_enable();

#if defined(CHIP_HAS_MIPIPHY)
    dsiphy_open(0);
    hal_cmu_dsi_phy_reset_set();
    hal_cmu_dsi_phy_reset_clear();
#else
    hal_dsiphy_open();
#endif

    //DSI init-1
    dsi->REG_000 = DSI_R_LPCD_DLY(1) | DSI_R_LANE_NUM(0) |
                   DSI_R_HSA_LP | DSI_R_HSE_EN | DSI_R_HFP_LP |
                   DSI_R_EOTP_EN | DSI_R_T_LPX(0x7) | DSI_R_CLK_T_LPX(0x3) |
                   DSI_R_CLK_PRE(0x5) | DSI_R_CLK_POST(0x7);
#if defined(CONFIG_DSI_2LANE_MODE)
    dsi->REG_000 |= DSI_R_LANE_NUM(0x1);
#endif
#if defined(CONFIG_DSI_VIDEO_MODE)
    dsi->REG_000 |= DSI_R_VIDEO_MODE;
    dsi->REG_000 &=  ~(DSI_R_EOTP_EN |DSI_R_HSA_LP|DSI_R_HBP_LP);
    dsi->REG_048 &= ~(DSI_R_VIDEO_BIST_EN);
#endif

    dsi->REG_004 = DSI_R_HS_EXIT_TIME(0xb) | DSI_R_HS_PRPR_TIME(0x4) |
                   DSI_R_HS_ZERO_TIME(0xb) | DSI_R_HS_TRAIL_TIME(0xa) |
                   DSI_R_T_WAKEUP(0x207);
    dsi->REG_008 = DSI_R_CLK_EXIT_TIME(0xb) | DSI_R_CLK_PRPR_TIME(0x8) |
                   DSI_R_CLK_ZERO_TIME(0x1f) | DSI_R_CLK_TRAIL_TIME(0x8) |
                   DSI_R_CLK_T_WAKEUP(0x66);
    dsi->REG_00C = DSI_R_DTYPE(0x39) | DSI_R_VC_CH_ID(0x0) |
                   DSI_R_INPUT_TYPE(0x0) | DSI_R_DLANE_AD_TIME(0x12);

    dsi->REG_048 &= ~DSI_R_COLOR_BAR_WIDTH_MASK;
    dsi->REG_048 |= DSI_R_COLOR_BAR_WIDTH(0x100);

    dsi->REG_048 &= ~DSI_R_HSYNC_DLY_NUM_MASK;

    dsi->REG_00C &= ~DSI_R_VIDEO_PACKET_LENTH_MASK;

#if defined(CONFIG_DSIPHY_OUTFMT_RGB888)
    dsi->REG_00C |= DSI_R_INPUT_TYPE(0x0) |
                    DSI_R_VIDEO_PACKET_LENTH(h_res*3+6+1);
    dsi->REG_048 |= DSI_R_HSYNC_DLY_NUM(0x120);
#elif defined(CONFIG_DSIPHY_OUTFMT_RGB565)
    dsi->REG_00C |= DSI_R_INPUT_TYPE(0x1) |
                    DSI_R_VIDEO_PACKET_LENTH(h_res*2+6+1);
    dsi->REG_048 |= DSI_R_HSYNC_DLY_NUM(0x120);
#elif defined(CONFIG_DSIPHY_OUTFMT_LOOSELYRGB666)

#elif defined(CONFIG_DSIPHY_OUTFMT_RGB666)

#elif defined(CONFIG_DSIPHY_OUTFMT_RGB101010)

#elif defined(CONFIG_DSIPHY_OUTFMT_YCbCr24)

#elif defined(CONFIG_DSIPHY_OUTFMT_YCbCr20)

#else
    dsi->REG_00C |= DSI_R_INPUT_TYPE(0x0) |
                    DSI_R_VIDEO_PACKET_LENTH(h_res*3+6+1);
    dsi->REG_048 |= DSI_R_HSYNC_DLY_NUM(0x120);
#endif
#if defined(CONFIG_DSI_VIDEO_MODE)
    dsi->REG_00C &= ~DSI_R_DTYPE_MASK;
    dsi->REG_00C |= DSI_R_DTYPE(0x3e);
    dsi->REG_00C &= ~DSI_R_VIDEO_PACKET_LENTH_MASK;
    dsi->REG_00C |= DSI_R_VIDEO_PACKET_LENTH(h_res*3+6);
    dsi->REG_048 &= ~DSI_R_HSYNC_DLY_NUM_MASK;
    dsi->REG_048 |= DSI_R_HSYNC_DLY_NUM(0x4b4);
    dsi->REG_048 &= ~DSI_R_COLOR_BAR_WIDTH_MASK;
#endif


    hal_sys_timer_delay(MS_TO_TICKS(2));

    //MASK(disable) all irq
    dsi->REG_010 = (dsi->REG_010 & ~(DSI_INTR_MASK_MASK | DSI_HS_CMD_RD_EN | DSI_LPRX_RD_EN)) //disalbe all irq
        | DSI_FLAG_MASK_MASK; //clear irq status
    dsi->REG_010 |= DSI_LPRX_RD_EN;// | (1<<31);//for DCS read cmd

    hal_sys_timer_delay(MS_TO_TICKS(10));
}

WEAK void hal_dsi_start(void)
{
}

/**
 * hal_dsi_irqn - get dsi irq number
 */
int hal_dsi_irqn(void)
{
    return DSI_IRQn;
}

/**
 * hal_dsi_get_irqstate - get dsi irq state
 */
uint32_t hal_dsi_get_irqstate(void)
{
    return dsi->REG_010;
}

/**
 * hal_dsi_clear_teirq_flag - clear dsi te irq state
 */
void hal_dsi_clear_teirq_flag(void)
{
    /* write 1 to clear irq flag */
    dsi->REG_010 = (dsi->REG_010 & ~DSI_FLAG_MASK_MASK) | DSI_TE_INTR;
}

/**
 * hal_dsi_irq_disable - disable dsi all irqs
 */
void hal_dsi_irq_disable(void)
{
    dsi->REG_010 = (dsi->REG_010 & ~(DSI_INTR_MASK_MASK | DSI_HS_CMD_RD_EN /*| DSI_LPRX_RD_EN*/)) //disalbe all irq
        | DSI_FLAG_MASK_MASK; //clear irq status
}

/**
 * hal_dsi_teirq_enable - enable dsi te irq
 */
void hal_dsi_teirq_enable(void)
{
    dsi->REG_048 |= DSI_R_TE_EDGE_SEL;
    dsi->REG_010 |= DSI_INTR_MASK(DSI_TE_INTR>>DSI_FLAG_MASK_SHIFT);
}

/**
 * hal_dsi_irq_is_te - irq is te
 * @regval : dsi ISR value
 */
bool hal_dsi_irq_is_te(uint32_t regval)
{
   return !!(regval & DSI_TE_INTR);
}

/**
 * hal_dsi_irq_is_err - dsi is irq err
 */
bool hal_dsi_irq_is_err(uint32_t regval)
{
    return !!(regval & (DSI_RX_TIMEOUT_FLAG |
                        DSI_RX_ERR_FLAG |
                        DSI_BTA_FAIL_FLAG));
}

/**
 * hal_dsi_irq_clear_err - clear dsi error flag.
 */
void hal_dsi_irq_clear_err(uint32_t status)
{
    dsi->REG_010 = (dsi->REG_010 & ~DSI_FLAG_MASK_MASK) |
                               DSI_RX_TIMEOUT_FLAG |
                               DSI_RX_ERR_FLAG |
                               DSI_BTA_FAIL_FLAG;
}

/**
 * hal_dsi_enter_ulps_mode - enter ulps mode
 */
void hal_dsi_enter_ulps_mode(void)
{
    uint32_t time = hal_sys_timer_get();

    if (!ulps_enter) {
        //TRACE(0,"hal_dsi_enter_ulps_mode");
        hal_sys_timer_delay_us(200);
        dsi->REG_010 = (dsi->REG_010 & ~DSI_FLAG_MASK_MASK) | DSI_CMD_DONE_FLAG;
        dsi->REG_014 = 0x0005e100;//dsi clk lane lp
        hal_sys_timer_delay_us(1);
        dsi->REG_014 = 0x00001e02;//clk lane enter ulps mode
        dsi->REG_014 = 0x00011e04;//data lane enter ulps mode
        while ((dsi->REG_010 & DSI_CMD_DONE_FLAG) == 0 &&
                hal_sys_timer_get() - time < MS_TO_TICKS(1)) {}
        ulps_enter = true;
    }
}

static void ulps_exit_handler(void *param)
{
    ulps_enter = false;
    if (ulps_exit_hdlr != NULL)
        ulps_exit_hdlr((uint32_t)param);
    ulps_exit_hdlr = NULL;
}

void hal_dsi_exit_ulps_with_cb(HAL_DSI_ULPS_CB_HANDLER hdlr,void *param)
{
    if (ulps_enter) {
        //TRACE(0,"hal_dsi_exit_ulps_mode");
        dsi->REG_014 = 0x00001e04;//data lane exit ulps mode
        dsi->REG_014 = 0x00001e03;//clk lane exit ulps mode
        // T_wakeup >= 1ms
        if (hdlr != NULL) {
            ulps_exit_hdlr = hdlr;
            if (ulps_exit_timer == NULL)
                ulps_exit_timer = hwtimer_alloc(ulps_exit_handler, param);
            hwtimer_update_then_start(ulps_exit_timer,ulps_exit_handler,param,US_TO_HWTICKS(1200));
        } else {
            hal_sys_timer_delay_us(1500);
            ulps_enter = false;
        }
    }
}

/**
 * hal_dsi_exit_ulps_mode - exit ulps mode
 */
void hal_dsi_exit_ulps_mode(void)
{
    if (ulps_enter) {
        //TRACE(0,"hal_dsi_exit_ulps_mode");
        dsi->REG_014 = 0x00001e04;//data lane exit ulps mode
        dsi->REG_014 = 0x00001e03;//clk lane exit ulps mode
        // T_wakeup >= 1ms
        hal_sys_timer_delay_us(1500);
        ulps_enter = false;
    }
}

void hal_dsi_sleep(void)
{
#if defined(CHIP_HAS_MIPIPHY)
    dsiphy_sleep();
#else
    hal_dsiphy_sleep();
#endif
    hal_cmu_dsi_sleep();
}

void hal_dsi_wakeup(void)
{
    hal_cmu_dsi_wakeup();
#if defined(CHIP_HAS_MIPIPHY)
    dsiphy_wakeup();
#else
    hal_dsiphy_wakeup();
#endif
}

void hal_dsi_te_gpio_irq_register(HAL_DSI_TE_GPIO_IRQ_HANDLER hdlr)
{
    te_gpio_hdlr = hdlr;
}

static void hal_dsi_te_gpio_irq_handler(enum HAL_GPIO_PIN_T pin)
{
    ASSERT(pin == te_gpio_pin, "%s: Bad te gpio: %u (expecting %u)", __func__, pin, te_gpio_pin);

    if (te_gpio_hdlr) {
        te_gpio_hdlr();
    }
}

void hal_dsi_te_gpio_irq_enable(void)
{
    static const struct HAL_GPIO_IRQ_CFG_T gpiocfg = {
        .irq_enable = true,
        .irq_debounce = false,
        .irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING,
        .irq_handler = hal_dsi_te_gpio_irq_handler,
        .irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE,
    };

    te_gpio_pin = (enum HAL_GPIO_PIN_T)hal_iomux_get_dsi_te_pin();

    hal_gpio_pin_set_dir(te_gpio_pin, HAL_GPIO_DIR_IN, 0);
    hal_gpio_setup_irq(te_gpio_pin, &gpiocfg);
}

void hal_dsi_te_gpio_irq_disable(void)
{
    static const struct HAL_GPIO_IRQ_CFG_T gpiocfg = {
        .irq_enable = false,
        .irq_debounce = false,
        .irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING,
        .irq_handler = NULL,
        .irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE,
    };

    hal_gpio_setup_irq(te_gpio_pin, &gpiocfg);
}

#endif
