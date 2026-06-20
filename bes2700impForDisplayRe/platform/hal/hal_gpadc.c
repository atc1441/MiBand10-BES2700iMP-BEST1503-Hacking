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
#include "plat_addr_map.h"

#if !defined(GPADC_CHIP_SPECIFIC) && !defined(CHIP_SUBSYS_SENS)

#include "plat_types.h"
#include "cmsis_nvic.h"
#include "hal_analogif.h"
#include "hal_gpadc.h"
#include "hal_sleep.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "pmu.h"
#include "tgt_hardware.h"

#if (CFG_HW_ADCKEY_NUMBER > 0)
#include "hwtimer_list.h"
#define GPADC_START_WORKAROUND_TIMEOUT
#else
#define GPADC_START_WORKAROUND_INTERVAL
#ifndef WORKAROUND_ATP_VAL
#define WORKAROUND_ATP_VAL                  HAL_GPADC_ATP_125US
#endif
#endif

#define GPADC_INTERVAL_MODE_WORKAROUND

#define HAL_GPADC_TRACE(n, s, ...)          //TRACE(n, s, ##__VA_ARGS__)

#define gpadc_reg_read(reg,val)             pmu_read(reg,val)
#define gpadc_reg_write(reg,val)            pmu_write(reg,val)

// #define VBAT_DIV_ALWAYS_ON
#define VBAT_DIV_STABLE_TIME_US             50

#ifdef GPADC_DYNAMIC_DATA_BITS
#ifdef GPADC_VALUE_BITS
#error "GPADC_DYNAMIC_DATA_BITS set while GPADC_VALUE_BITS also set"
#endif
#define GPADC_VALUE_BITS                    gpadc_data_bits
#elif !defined(GPADC_VALUE_BITS)
#define GPADC_VALUE_BITS                    10
#endif

#if 0
#elif (GPADC_CTRL_VER == 1)

enum GPADC_REG_T {
    GPADC_REG_VBAT_EN = 0x45,
    GPADC_REG_INTVL_EN = 0x60,
    GPADC_REG_INTVL_VAL = 0x64,
    GPADC_REG_START = 0x65,
    GPADC_REG_CH_EN = 0x65,
    GPADC_REG_INT_MASK = 0x67,
    GPADC_REG_INT_EN = 0x68,
    GPADC_REG_INT_RAW_STS = 0x69,
    GPADC_REG_INT_MSKED_STS = 0x6A,
    GPADC_REG_INT_CLR = 0x6A,
    GPADC_REG_CH0_DATA = 0x78,
};

// GPADC_REG_VBAT_EN
#define REG_PU_VBAT_DIV                     (1 << 0)

// GPADC_REG_INTVL_EN
#define GPADC_INTERVAL_MODE                 (1 << 12)

// GPADC_REG_START
#define KEY_START                           (1 << 9)
#define GPADC_START                         (1 << 8)

// GPADC_REG_CH_EN
#define CHAN_EN_REG_SHIFT                   0
#define CHAN_EN_REG_MASK                    (0xFF << CHAN_EN_REG_SHIFT)
#define CHAN_EN_REG(n)                      BITFIELD_VAL(CHAN_EN_REG, n)

// GPADC_REG_INT_MASK
#define KEY_ERR1_INTR_MSK                   (1 << 12)
#define KEY_ERR0_INTR_MSK                   (1 << 11)
#define KEY_PRESS_INTR_MSK                  (1 << 10)
#define KEY_RELEASE_INTR_MSK                (1 << 9)
#define SAMPLE_DONE_INTR_MSK                (1 << 8)
#define CHAN_DATA_INTR_MSK_SHIFT            0
#define CHAN_DATA_INTR_MSK_MASK             (0xFF << CHAN_DATA_INTR_MSK_SHIFT)
#define CHAN_DATA_INTR_MSK(n)               BITFIELD_VAL(CHAN_DATA_INTR_MSK, n)

// GPADC_REG_INT_EN
#define KEY_ERR1_INTR_EN                    (1 << 12)
#define KEY_ERR0_INTR_EN                    (1 << 11)
#define KEY_PRESS_INTR_EN                   (1 << 10)
#define KEY_RELEASE_INTR_EN                 (1 << 9)
#define SAMPLE_DONE_INTR_EN                 (1 << 8)
#define CHAN_DATA_INTR_EN_SHIFT             0
#define CHAN_DATA_INTR_EN_MASK              (0xFF << CHAN_DATA_INTR_EN_SHIFT)
#define CHAN_DATA_INTR_EN(n)                BITFIELD_VAL(CHAN_DATA_INTR_EN, n)

// GPADC_REG_INT_RAW_STS
#define KEY_ERR1_INTR                       (1 << 12)
#define KEY_ERR0_INTR                       (1 << 11)
#define KEY_PRESS_INTR                      (1 << 10)
#define KEY_RELEASE_INTR                    (1 << 9)
#define SAMPLE_PERIOD_DONE_INTR             (1 << 8)
#define CHAN_DATA_VALID_INTR_SHIFT          0
#define CHAN_DATA_VALID_INTR_MASK           (0xFF << CHAN_DATA_VALID_INTR_SHIFT)
#define CHAN_DATA_VALID_INTR(n)             BITFIELD_VAL(CHAN_DATA_VALID_INTR, n)

// GPADC_REG_INT_MSKED_STS
#define KEY_ERR1_INTR_MSKED                 (1 << 12)
#define KEY_ERR0_INTR_MSKED                 (1 << 11)
#define KEY_PRESS_INTR_MSKED                (1 << 10)
#define KEY_RELEASE_INTR_MSKED              (1 << 9)
#define SAMPLE_DONE_INTR_MSKED              (1 << 8)
#define CHAN_DATA_INTR_MSKED_SHIFT          0
#define CHAN_DATA_INTR_MSKED_MASK           (0xFF << CHAN_DATA_INTR_MSKED_SHIFT)
#define CHAN_DATA_INTR_MSKED(n)             BITFIELD_VAL(CHAN_DATA_INTR_MSKED, n)

// GPADC_REG_INT_CLR
#define KEY_ERR1_INTR_CLR                   (1 << 12)
#define KEY_ERR0_INTR_CLR                   (1 << 11)
#define KEY_PRESS_INTR_CLR                  (1 << 10)
#define KEY_RELEASE_INTR_CLR                (1 << 9)
#define SAMPLE_DONE_INTR_CLR                (1 << 8)
#define CHAN_DATA_INTR_CLR_SHIFT            0
#define CHAN_DATA_INTR_CLR_MASK             (0xFF << CHAN_DATA_INTR_CLR_SHIFT)
#define CHAN_DATA_INTR_CLR(n)               BITFIELD_VAL(CHAN_DATA_INTR_CLR, n)

// GPADC_REG_CH0_DATA
#define DATA_CHAN0_SHIFT                    0
#define DATA_CHAN0_MASK                     (((1 << GPADC_VALUE_BITS) - 1) << DATA_CHAN0_SHIFT)
#define DATA_CHAN0(n)                       BITFIELD_VAL(DATA_CHAN0, n)

#elif (GPADC_CTRL_VER >= 2)

#if (GPADC_CTRL_VER == 2) || (GPADC_CTRL_VER >= 4)

enum GPADC_REG_T {
    GPADC_REG_VBAT_EN = 0x02,
    GPADC_REG_INTVL_EN = 0x1F,
    GPADC_REG_INTVL_VAL = 0x23,
    GPADC_REG_START = 0x4F,
    GPADC_REG_CH_EN = 0x24,
    GPADC_REG_INT_MASK = 0x26,
    GPADC_REG_INT_EN = 0x27,
#if (GPADC_CTRL_VER == 2)
    GPADC_REG_INT_RAW_STS = 0x50,
    GPADC_REG_INT_MSKED_STS = 0x51,
    GPADC_REG_INT_CLR = 0x51,
#elif (GPADC_CTRL_VER >= 4)
    GPADC_REG_INT_RAW_STS = 0x52,
    GPADC_REG_INT_MSKED_STS = 0x53,
#if (GPADC_CTRL_VER == 4)
    GPADC_REG_INT_CLR = 0x51,
#else
    GPADC_REG_INT_CLR = 0x52,
#endif
#else
#error "Bad GPADC_CTRL_VER"
#endif
    GPADC_REG_CH0_DATA = 0x56,
};

#elif (GPADC_CTRL_VER == 3)

enum GPADC_REG_T {
    GPADC_REG_VBAT_EN = 0x02,
    GPADC_REG_INTVL_EN = 0x18,
    GPADC_REG_INTVL_VAL = 0x1C,
    GPADC_REG_START = 0x4F,
    GPADC_REG_CH_EN = 0x1D,
    GPADC_REG_INT_MASK = 0x1F,
    GPADC_REG_INT_EN = 0x20,
    GPADC_REG_INT_RAW_STS = 0x50,
    GPADC_REG_INT_MSKED_STS = 0x51,
    GPADC_REG_INT_CLR = 0x51,
    GPADC_REG_CH0_DATA = 0x56,
};

#else
#error "Bad GPADC_CTRL_VER"
#endif

// GPADC_REG_VBAT_EN
#define REG_PU_VBAT_DIV                     (1 << 15)

// GPADC_REG_INTVL_EN
#define GPADC_INTERVAL_MODE                 (1 << 12)

// GPADC_REG_START
#define GPADC_START                         (1 << 5)
#define KEY_START                           (1 << 4)

// GPADC_REG_CH_EN
#define CHAN_EN_REG_SHIFT                   0
#define CHAN_EN_REG_MASK                    (0xFF << CHAN_EN_REG_SHIFT)
#define CHAN_EN_REG(n)                      BITFIELD_VAL(CHAN_EN_REG, n)

// GPADC_REG_INT_MASK
#define KEY_ERR1_INTR_MSK                   (1 << 12)
#define KEY_ERR0_INTR_MSK                   (1 << 11)
#define KEY_PRESS_INTR_MSK                  (1 << 10)
#define KEY_RELEASE_INTR_MSK                (1 << 9)
#define SAMPLE_DONE_INTR_MSK                (1 << 8)
#define CHAN_DATA_INTR_MSK_SHIFT            0
#define CHAN_DATA_INTR_MSK_MASK             (0xFF << CHAN_DATA_INTR_MSK_SHIFT)
#define CHAN_DATA_INTR_MSK(n)               BITFIELD_VAL(CHAN_DATA_INTR_MSK, n)

// GPADC_REG_INT_EN
#define KEY_ERR1_INTR_EN                    (1 << 12)
#define KEY_ERR0_INTR_EN                    (1 << 11)
#define KEY_PRESS_INTR_EN                   (1 << 10)
#define KEY_RELEASE_INTR_EN                 (1 << 9)
#define SAMPLE_DONE_INTR_EN                 (1 << 8)
#define CHAN_DATA_INTR_EN_SHIFT             0
#define CHAN_DATA_INTR_EN_MASK              (0xFF << CHAN_DATA_INTR_EN_SHIFT)
#define CHAN_DATA_INTR_EN(n)                BITFIELD_VAL(CHAN_DATA_INTR_EN, n)

// GPADC_REG_INT_RAW_STS
#define KEY_ERR1_INTR                       (1 << 12)
#define KEY_ERR0_INTR                       (1 << 11)
#define KEY_PRESS_INTR                      (1 << 10)
#define KEY_RELEASE_INTR                    (1 << 9)
#define SAMPLE_PERIOD_DONE_INTR             (1 << 8)
#define CHAN_DATA_VALID_INTR_SHIFT          0
#define CHAN_DATA_VALID_INTR_MASK           (0xFF << CHAN_DATA_VALID_INTR_SHIFT)
#define CHAN_DATA_VALID_INTR(n)             BITFIELD_VAL(CHAN_DATA_VALID_INTR, n)

// GPADC_REG_INT_MSKED_STS
#define KEY_ERR1_INTR_MSKED                 (1 << 12)
#define KEY_ERR0_INTR_MSKED                 (1 << 11)
#define KEY_PRESS_INTR_MSKED                (1 << 10)
#define KEY_RELEASE_INTR_MSKED              (1 << 9)
#define SAMPLE_DONE_INTR_MSKED              (1 << 8)
#define CHAN_DATA_INTR_MSKED_SHIFT          0
#define CHAN_DATA_INTR_MSKED_MASK           (0xFF << CHAN_DATA_INTR_MSKED_SHIFT)
#define CHAN_DATA_INTR_MSKED(n)             BITFIELD_VAL(CHAN_DATA_INTR_MSKED, n)

// GPADC_REG_INT_CLR
#define KEY_ERR1_INTR_CLR                   (1 << 12)
#define KEY_ERR0_INTR_CLR                   (1 << 11)
#define KEY_PRESS_INTR_CLR                  (1 << 10)
#define KEY_RELEASE_INTR_CLR                (1 << 9)
#define SAMPLE_DONE_INTR_CLR                (1 << 8)
#define CHAN_DATA_INTR_CLR_SHIFT            0
#define CHAN_DATA_INTR_CLR_MASK             (0xFF << CHAN_DATA_INTR_CLR_SHIFT)
#define CHAN_DATA_INTR_CLR(n)               BITFIELD_VAL(CHAN_DATA_INTR_CLR, n)

// GPADC_REG_CH0_DATA
#define DATA_CHAN0_SHIFT                    0
#define DATA_CHAN0_MASK                     (((1 << GPADC_VALUE_BITS) - 1) << DATA_CHAN0_SHIFT)
#define DATA_CHAN0(n)                       BITFIELD_VAL(DATA_CHAN0, n)

#else
#error "Bad GPADC_CTRL_VER"
#endif

#define HAL_GPADC_STS_UPDATE_TIMEOUT        (MS_TO_TICKS(2))

#define HAL_GPADC_NORM_CHAN_MASK            ((1 << HAL_GPADC_CHAN_ADCKEY) - 1)
#define HAL_GPADC_ALL_CHAN_MASK             ((1 << HAL_GPADC_CHAN_QTY) - 1)

#if !defined(GPADC_DYNAMIC_DATA_BITS) && (GPADC_VALUE_BITS < 10 || GPADC_VALUE_BITS > 16)
#error "GPADC value bits not in range"
#endif

// Battery voltage = gpadc voltage * 4
// Range 0~2V
#ifndef GPADC_CALIB_CH_DIV
#define GPADC_CALIB_CH_DIV                  4
#endif
#define HAL_GPADC_MVOLT_A                   (3200 / GPADC_CALIB_CH_DIV)
#define HAL_GPADC_MVOLT_B                   (4200 / GPADC_CALIB_CH_DIV)
#if defined(GPADC_CALIB_DEFAULT_A) || defined(GPADC_CALIB_DEFAULT_A2) || defined(GPADC_CALIB_DEFAULT_B)
#define HAL_GPADC_CALIB_DEFAULT_A           GPADC_CALIB_DEFAULT_A
#define HAL_GPADC_CALIB_DEFAULT_A2          GPADC_CALIB_DEFAULT_A2
#define HAL_GPADC_CALIB_DEFAULT_B           GPADC_CALIB_DEFAULT_B
#else
#if (GPADC_VALUE_BITS == 16)
#define HAL_GPADC_CALIB_DEFAULT_A           (0x3FF6 * 4 / GPADC_CALIB_CH_DIV)
#define HAL_GPADC_CALIB_DEFAULT_A2          (0x4A42 * 4 / GPADC_CALIB_CH_DIV)
#define HAL_GPADC_CALIB_DEFAULT_B           (0x5422 * 4 / GPADC_CALIB_CH_DIV)
#else
#define HAL_GPADC_CALIB_DEFAULT_A           ((428 << (GPADC_VALUE_BITS - 10)) * 4 / GPADC_CALIB_CH_DIV)
#define HAL_GPADC_CALIB_DEFAULT_A2          ((496 << (GPADC_VALUE_BITS - 10)) * 4 / GPADC_CALIB_CH_DIV)
#define HAL_GPADC_CALIB_DEFAULT_B           ((565 << (GPADC_VALUE_BITS - 10)) * 4 / GPADC_CALIB_CH_DIV)
#endif
#endif

#ifdef GPADC_WORKAROUND_1805
#define HAL_GPADC_CALIB_DEFAULT_A_1805      0x06FB
#define HAL_GPADC_CALIB_DEFAULT_B_1805      0x0930
#endif

#define HAL_GPADC_MVOLT_A_EXT               400
#define HAL_GPADC_MVOLT_A2_EXT              900
#define HAL_GPADC_MVOLT_B_EXT               1400

#if defined(__FPU_USED) && (__FPU_USED == 1)
typedef float ADC_COEF_T;
#define ADC_CALC_FACTOR                     1
#else
typedef int32_t ADC_COEF_T;
#define ADC_CALC_FACTOR                     1000
#endif

#ifdef GPADC_DYNAMIC_DATA_BITS
static uint8_t gpadc_data_bits;
#endif

static ADC_COEF_T g_adcSlope = 0;
static ADC_COEF_T g_adcIntcpt = 0;
static bool gpadc_irq_enabled = false;
static bool adckey_irq_enabled = false;
static bool irq_enabled = false;
static bool g_adcCalibrated = false;
static HAL_GPADC_EVENT_CB_T gpadc_event_cb[HAL_GPADC_CHAN_QTY];
static enum HAL_GPADC_ATP_T gpadc_atp[HAL_GPADC_CHAN_QTY];
static uint16_t gpadc_chan_en;
static uint16_t gpadc_irq_mask;

#ifdef VBAT_DIV_ALWAYS_ON
static bool vbat_div_enabled = false;
#endif

#ifdef GPADC_START_WORKAROUND_TIMEOUT
static bool timer_running;
static HWTIMER_ID gpadc_timer;
static uint32_t chan_start_time[HAL_GPADC_CHAN_QTY];
static const uint32_t gpadc_timeout_ticks = MS_TO_TICKS(3);
#endif

#if defined(GPADC_HAS_EXT_SLOPE_CAL) || defined(GPADC_HAS_EXT_SLOPE_CAL2)
static bool g_adcCalibrated_ext = false;
static ADC_COEF_T g_adcSlope_ext = 0;
static ADC_COEF_T g_adcIntcpt_ext = 0;
#ifdef GPADC_HAS_EXT_SLOPE_CAL2
static ADC_COEF_T g_adcSlope_ext2 = 0;
static ADC_COEF_T g_adcIntcpt_ext2 = 0;
// medium voltage
static ADC_COEF_T g_adcVal_mv;
#endif
#endif

#ifdef GPADC_INTERVAL_MODE_WORKAROUND
static bool gpadc_raw_data_valid[HAL_GPADC_CHAN_QTY - 1];
static bool gpadc_interval_mode_en = false;
#endif

#ifdef GPADC_START_WORKAROUND_TIMEOUT
static void gpadc_timer_handler(void *param)
{
    unsigned short val;
    unsigned short mask;
    enum E_HWTIMER_T ret;

    if ((gpadc_irq_mask & CHAN_DATA_INTR_MSK(HAL_GPADC_ALL_CHAN_MASK)) == 0) {
        timer_running = false;
        return;
    }

    if (gpadc_irq_mask & CHAN_DATA_INTR_MSK(1 << HAL_GPADC_CHAN_ADCKEY)) {
        mask = KEY_START;
    } else {
        mask = GPADC_START;
    }
    gpadc_reg_read(GPADC_REG_START, &val);
    val |= mask;
    gpadc_reg_write(GPADC_REG_START, val);

    ret = hwtimer_start(gpadc_timer, gpadc_timeout_ticks);
    ASSERT(ret == E_HWTIMER_OK, "Failed to start gpadc timer: %d", ret);
}
#endif

static uint16_t hal_gpadc_get_all_masked_irq(void)
{
    uint16_t all;

    all = KEY_ERR1_INTR_MSKED | KEY_ERR0_INTR_MSKED | KEY_PRESS_INTR_MSKED | KEY_RELEASE_INTR_MSKED |
        SAMPLE_DONE_INTR_MSKED | CHAN_DATA_INTR_MSKED_MASK;
    return all;
}

static uint16_t hal_gpadc_get_cur_masked_irq(void)
{
#if (KEY_ERR1_INTR_MSKED != KEY_ERR1_INTR_MSK) || \
        (KEY_ERR0_INTR_MSKED != KEY_ERR0_INTR_MSK) || \
        (KEY_PRESS_INTR_MSKED != KEY_PRESS_INTR_MSK) || \
        (KEY_RELEASE_INTR_MSKED != KEY_RELEASE_INTR_MSK) || \
        (SAMPLE_DONE_INTR_MSKED != SAMPLE_DONE_INTR_MSK) || \
        (CHAN_DATA_INTR_MSKED_MASK != CHAN_DATA_INTR_MSK_MASK)
#error "GPADC IRQ MASKED STS != IRQ MASK"
#endif

    uint16_t all = hal_gpadc_get_all_masked_irq();
    return (all & gpadc_irq_mask);
}

int hal_gpadc_masked_irq_valid(uint16_t irq)
{
    uint16_t masked = hal_gpadc_get_cur_masked_irq();
    irq &= masked;
    return (irq ? true: false);
}

uint16_t hal_gpadc_filter_out_unmasked_irq(uint16_t irq)
{
    uint16_t all = hal_gpadc_get_all_masked_irq();
    uint16_t masked = hal_gpadc_get_cur_masked_irq();
    irq &= ((~all) | masked);
    return irq;
}

POSSIBLY_UNUSED
static enum HAL_GPADC_ATP_T hal_gpadc_get_min_atp(void)
{
    enum HAL_GPADC_CHAN_T ch;
    enum HAL_GPADC_ATP_T atp = HAL_GPADC_ATP_NULL;

    for (ch = HAL_GPADC_CHAN_0; ch < HAL_GPADC_CHAN_QTY; ch++) {
        if (gpadc_atp[ch] != HAL_GPADC_ATP_NULL) {
            if (atp == HAL_GPADC_ATP_NULL ||
                    (uint32_t)gpadc_atp[ch] < (uint32_t)atp) {
                atp = gpadc_atp[ch];
            }
        }
    }

    return atp;
}

static void hal_gpadc_update_atp(void)
{
    enum HAL_GPADC_ATP_T atp;
    uint16_t val;

#ifdef GPADC_START_WORKAROUND_INTERVAL
    atp = WORKAROUND_ATP_VAL;
#else
    atp = hal_gpadc_get_min_atp();
#endif

    if (atp == HAL_GPADC_ATP_NULL || atp == HAL_GPADC_ATP_ONESHOT) {
#ifdef GPADC_INTERVAL_MODE_WORKAROUND
        gpadc_interval_mode_en = false;
#endif
        gpadc_reg_read(GPADC_REG_INTVL_EN, &val);
        val &= ~GPADC_INTERVAL_MODE;
        gpadc_reg_write(GPADC_REG_INTVL_EN, val);
    } else {
#ifdef GPADC_INTERVAL_MODE_WORKAROUND
        gpadc_interval_mode_en = true;
#endif
        gpadc_reg_read(GPADC_REG_INTVL_EN, &val);
        val |= GPADC_INTERVAL_MODE;
        gpadc_reg_write(GPADC_REG_INTVL_EN, val);
        val = atp * 1000 / 1024;
        gpadc_reg_write(GPADC_REG_INTVL_VAL, val);
    }
}

static int hal_gpadc_adc2volt_calib(void)
{
    ADC_COEF_T y1, y2, x1, x2;
    unsigned short efuse_a = 0;
    unsigned short efuse_b = 0;

    if (!g_adcCalibrated)
    {
        y1 = (ADC_COEF_T)HAL_GPADC_MVOLT_A;
        y2 = (ADC_COEF_T)HAL_GPADC_MVOLT_B;

#ifdef GPADC_CUSTOM_CALIB_VAL
        pmu_get_vbat_calib_value(&efuse_a, &efuse_b);
#else
        pmu_get_efuse(PMU_EFUSE_PAGE_BATTER_LV, &efuse_a);
        pmu_get_efuse(PMU_EFUSE_PAGE_BATTER_HV, &efuse_b);
#endif

        x1 = (ADC_COEF_T)(efuse_a > 0 ? efuse_a : HAL_GPADC_CALIB_DEFAULT_A);
        x2 = (ADC_COEF_T)(efuse_b > 0 ? efuse_b : HAL_GPADC_CALIB_DEFAULT_B);
        g_adcSlope = (y2 - y1) * ADC_CALC_FACTOR / (x2 - x1);
        g_adcIntcpt = ((y1 * x2) - (x1 * y2)) / ((x2 - x1));
        g_adcCalibrated = true;

        TRACE(7,"%s efuse:%d/%d LV=%d, HV=%d, Slope:%d Intcpt:%d", __func__,
            efuse_a, efuse_b, (int32_t)x1, (int32_t)x2, (int32_t)g_adcSlope, (int32_t)g_adcIntcpt);
    }

    return 0;
}

#if defined(GPADC_HAS_EXT_SLOPE_CAL) || defined(GPADC_HAS_EXT_SLOPE_CAL2)
static int hal_gpadc_adc2volt_calib_ext(void)
{
#ifdef GPADC_HAS_EXT_SLOPE_CAL2
    ADC_COEF_T y1, y2, y3, x1, x2, x3;
    unsigned short efuse_a_ext = 0;
    unsigned short efuse_a2_ext = 0;
    unsigned short efuse_b_ext = 0;

    if (!g_adcCalibrated_ext) {
        y1 = (ADC_COEF_T)HAL_GPADC_MVOLT_A_EXT;
        y2 = (ADC_COEF_T)HAL_GPADC_MVOLT_A2_EXT;
        y3 = (ADC_COEF_T)HAL_GPADC_MVOLT_B_EXT;

#ifdef GPADC_CUSTOM_CALIB_VAL
        pmu_get_ext_gpadc_calib2_value(&efuse_a_ext, &efuse_a2_ext, &efuse_b_ext);
#else
        pmu_get_efuse(PMU_EFUSE_PAGE_GPADC_LV, &efuse_a_ext);
        pmu_get_efuse(PMU_EFUSE_PAGE_GPADC_MV, &efuse_a2_ext);
        pmu_get_efuse(PMU_EFUSE_PAGE_GPADC_HV, &efuse_b_ext);
#endif

        if (efuse_a_ext == 0 || efuse_a2_ext == 0 || efuse_b_ext == 0) {
            hal_gpadc_adc2volt_calib();
            g_adcSlope_ext = g_adcSlope;
            g_adcSlope_ext2 = g_adcSlope_ext;
            g_adcIntcpt_ext = g_adcIntcpt;
            g_adcIntcpt_ext2 = g_adcIntcpt_ext;

            TRACE(0, "Use ch1 calibration values");
        } else {
            x1 = (ADC_COEF_T)(efuse_a_ext > 0 ? efuse_a_ext : HAL_GPADC_CALIB_DEFAULT_A);
            x2 = (ADC_COEF_T)(efuse_a2_ext > 0 ? efuse_a2_ext : HAL_GPADC_CALIB_DEFAULT_A2);
            x3 = (ADC_COEF_T)(efuse_b_ext > 0 ? efuse_b_ext : HAL_GPADC_CALIB_DEFAULT_B);
            g_adcVal_mv = x2;

            g_adcSlope_ext = (y2 - y1) * ADC_CALC_FACTOR / (x2 - x1);
            g_adcSlope_ext2 = (y3 - y2) * ADC_CALC_FACTOR / (x3 - x2);
            g_adcIntcpt_ext = ((y1 * x2) - (x1 * y2)) / ((x2 - x1));
            g_adcIntcpt_ext2 = ((y2 * x3) - (x2 * y3)) / ((x3 - x2));

            TRACE(7,"%s efuse:%d/%d LV=%d, MV=%d, Slope:%d Intcpt:%d", __func__,
                efuse_a_ext, efuse_a2_ext, (int32_t)x1, (int32_t)x2, (int32_t)g_adcSlope_ext, (int32_t)g_adcIntcpt_ext);
            TRACE(7,"%s efuse:%d/%d MV=%d, HV=%d, Slope2:%d Intcpt2:%d", __func__,
                efuse_a2_ext, efuse_b_ext, (int32_t)x2, (int32_t)x3, (int32_t)g_adcSlope_ext2, (int32_t)g_adcIntcpt_ext2);
        }
        g_adcCalibrated_ext = true;
    }
#else // GPADC_HAS_EXT_SLOPE_CAL
    ADC_COEF_T y1, y2, x1, x2;
    unsigned short efuse_a_ext = 0;
    unsigned short efuse_b_ext = 0;

    if (!g_adcCalibrated_ext) {
        y1 = (ADC_COEF_T)HAL_GPADC_MVOLT_A_EXT;
        y2 = (ADC_COEF_T)HAL_GPADC_MVOLT_B_EXT;

#ifdef GPADC_CUSTOM_CALIB_VAL
        pmu_get_ext_gpadc_calib_value(&efuse_a_ext, &efuse_b_ext);
#else
        pmu_get_efuse(PMU_EFUSE_PAGE_GPADC_LV, &efuse_a_ext);
        pmu_get_efuse(PMU_EFUSE_PAGE_GPADC_HV, &efuse_b_ext);
#endif

        if (efuse_a_ext == 0 || efuse_b_ext == 0) {
            hal_gpadc_adc2volt_calib();
            g_adcSlope_ext = g_adcSlope;
            g_adcIntcpt_ext = g_adcIntcpt;

            TRACE(0, "Use ch1 calibration values");
        } else {
            x1 = (ADC_COEF_T)(efuse_a_ext > 0 ? efuse_a_ext : HAL_GPADC_CALIB_DEFAULT_A);
            x2 = (ADC_COEF_T)(efuse_b_ext > 0 ? efuse_b_ext : HAL_GPADC_CALIB_DEFAULT_B);

            g_adcSlope_ext = (y2 - y1) * ADC_CALC_FACTOR / (x2 - x1);
            g_adcIntcpt_ext = ((y1 * x2) - (x1 * y2)) / ((x2 - x1));

            TRACE(7,"%s efuse:%d/%d LV=%d, HV=%d, Slope:%d Intcpt:%d", __func__,
                efuse_a_ext, efuse_b_ext, (int32_t)x1, (int32_t)x2, (int32_t)g_adcSlope_ext, (int32_t)g_adcIntcpt_ext);
        }
        g_adcCalibrated_ext = true;
    }
#endif
    return 0;
}

HAL_GPADC_MV_T hal_gpadc_adc2volt_ext(uint16_t gpadcVal, enum HAL_GPADC_CHAN_T channel)
{
    int32_t voltage;

    if ((channel == HAL_GPADC_CHAN_BATTERY) || (channel == HAL_GPADC_CHAN_5) || (channel == HAL_GPADC_CHAN_6)) {
        return hal_gpadc_adc2volt(gpadcVal);
    }

    hal_gpadc_adc2volt_calib_ext();

    if (gpadcVal == HAL_GPADC_BAD_VALUE) {
        return HAL_GPADC_BAD_VALUE;
    } else {
        ADC_COEF_T slope = g_adcSlope_ext;
        ADC_COEF_T intcpt = g_adcIntcpt_ext;
#ifdef GPADC_HAS_EXT_SLOPE_CAL2
        if (gpadcVal > g_adcVal_mv && g_adcVal_mv != 0) {
            slope = g_adcSlope_ext2;
            intcpt = g_adcIntcpt_ext2;
        }
#endif
        voltage = (int32_t)(((slope * gpadcVal) / ADC_CALC_FACTOR) + intcpt);

        return (voltage < 0) ? 0 : voltage;
    }
}
#endif

HAL_GPADC_MV_T hal_gpadc_adc2volt(uint16_t gpadcVal)
{
    int32_t voltage;

    hal_gpadc_adc2volt_calib();
    if (gpadcVal == HAL_GPADC_BAD_VALUE)
    {
        // Bad values from the GPADC are still Bad Values
        // for the voltage-speaking user.
        return HAL_GPADC_BAD_VALUE;
    }
    else
    {
        voltage = (int32_t)(((g_adcSlope * gpadcVal) / ADC_CALC_FACTOR) + (g_adcIntcpt));

        return (voltage < 0) ? 0 : voltage;
    }
}

static void hal_gpadc_enable_vbat_div(int enable)
{
    unsigned short val;

#ifdef GPADC_HAS_VSYS_DIV
    int ret;

    ret = pmu_gpadc_div_ctrl(HAL_GPADC_CHAN_BATTERY, enable);
    if (ret == 0) {
        return;
    }
#endif

    gpadc_reg_read(GPADC_REG_VBAT_EN, &val);
    if (enable) {
        val |= REG_PU_VBAT_DIV;
    } else {
        val &= ~REG_PU_VBAT_DIV;
    }
    gpadc_reg_write(GPADC_REG_VBAT_EN, val);
}

static void hal_gpadc_clear_chan_irq(unsigned short irq)
{
#ifdef GPADC_CUSTOM_CHAN_IRQ_CLEAR
    int ret;

    ret = pmu_gpadc_clear_chan_irq(irq);
    if (ret == 0) {
        return;
    }
#endif

    gpadc_reg_write(GPADC_REG_INT_CLR, irq);
}

static void hal_gpadc_enable_chan(enum HAL_GPADC_CHAN_T ch, int enable)
{
#if (GPADC_CTRL_VER >= 2)
    uint16_t val;

    if (enable) {
        if (gpadc_chan_en == 0) {
            hal_sys_wake_lock(HAL_SYS_WAKE_LOCK_USER_GPADC);
        }
        gpadc_chan_en |= (1 << ch);
    } else {
        gpadc_chan_en &= ~(1 << ch);
        if (gpadc_chan_en == 0) {
            hal_sys_wake_unlock(HAL_SYS_WAKE_LOCK_USER_GPADC);
        }
    }

    if (ch == HAL_GPADC_CHAN_ADCKEY) {
        // ADCKEY CHAN is controlled by KEY_START
        return;
    }

#ifdef GPADC_CUSTOM_CHAN_ENABLE
    int ret;

    ret = pmu_gpadc_enable_chan(ch, enable);
    if (ret == 0) {
        return;
    }
#endif

    gpadc_reg_read(GPADC_REG_CH_EN, &val);
    if (enable) {
        val |= CHAN_EN_REG(1 << ch);
    } else {
        val &= ~CHAN_EN_REG(1 << ch);
    }
    gpadc_reg_write(GPADC_REG_CH_EN, val);
#endif
}

#ifdef PMU_IRQ_UNIFIED
#define GPADC_IRQ_HDLR_PARAM            uint16_t irq_status
#else
#define GPADC_IRQ_HDLR_PARAM            void
#endif
static void hal_gpadc_irq_handler(GPADC_IRQ_HDLR_PARAM)
{
    uint32_t lock;
    enum HAL_GPADC_CHAN_T ch;
    unsigned short read_val;
    uint16_t adc_val;
    HAL_GPADC_MV_T volt;
    uint16_t irq_checked;

#ifdef PMU_IRQ_UNIFIED
    irq_status &= hal_gpadc_get_cur_masked_irq();
#else
    unsigned short irq_status;

    gpadc_reg_read(GPADC_REG_INT_MSKED_STS, &irq_status);
    irq_status &= hal_gpadc_get_cur_masked_irq();
    hal_gpadc_clear_chan_irq(irq_status);
#endif

    if (irq_status & CHAN_DATA_INTR_MSKED(HAL_GPADC_NORM_CHAN_MASK)) {
        for (ch = HAL_GPADC_CHAN_0; ch < HAL_GPADC_CHAN_ADCKEY; ch++) {
            if (irq_status & CHAN_DATA_INTR_MSKED(1 << ch)) {
                switch (ch) {
                case HAL_GPADC_CHAN_BATTERY:
                case HAL_GPADC_CHAN_0:
                case HAL_GPADC_CHAN_2:
                case HAL_GPADC_CHAN_3:
                case HAL_GPADC_CHAN_4:
                case HAL_GPADC_CHAN_5:
                case HAL_GPADC_CHAN_6:
                    gpadc_reg_read(GPADC_REG_CH0_DATA + ch, &adc_val);
                    adc_val = GET_BITFIELD(adc_val, DATA_CHAN0);
#ifdef GPADC_INTERVAL_MODE_WORKAROUND
                    // Wait next GPADC irq.
                    if (gpadc_interval_mode_en && !gpadc_raw_data_valid[ch] && adc_val == 0) {
                        gpadc_raw_data_valid[ch] = true;
                        return;
                    }
                    gpadc_raw_data_valid[ch] = false;
#endif

#if defined(GPADC_HAS_EXT_SLOPE_CAL) || defined(GPADC_HAS_EXT_SLOPE_CAL2)
                    volt = hal_gpadc_adc2volt_ext(adc_val, ch);
#else
                    volt = hal_gpadc_adc2volt(adc_val);
#endif

                    if (gpadc_event_cb[ch]) {
                        gpadc_event_cb[ch](adc_val, volt);
                    }
                    if (gpadc_atp[ch] == HAL_GPADC_ATP_NULL || gpadc_atp[ch] == HAL_GPADC_ATP_ONESHOT) {
                        lock = int_lock();

#ifndef VBAT_DIV_ALWAYS_ON
                        if (ch == HAL_GPADC_CHAN_BATTERY) {
                            hal_gpadc_enable_vbat_div(false);
                        }
#endif

#ifdef GPADC_HAS_VSYS_DIV
                        if (ch != HAL_GPADC_CHAN_BATTERY) {
                            pmu_gpadc_div_ctrl(ch, false);
                        }
#endif

                        // Int mask
                        gpadc_reg_read(GPADC_REG_INT_MASK, &read_val);
                        read_val &= ~CHAN_DATA_INTR_MSK(1 << ch);
                        gpadc_reg_write(GPADC_REG_INT_MASK, read_val);
                        gpadc_irq_mask = read_val;

                        // Int enable
                        gpadc_reg_read(GPADC_REG_INT_EN, &read_val);
                        read_val &= ~CHAN_DATA_INTR_EN(1 << ch);
                        gpadc_reg_write(GPADC_REG_INT_EN, read_val);

                        // Channel enable
                        hal_gpadc_enable_chan(ch, false);

                        int_unlock(lock);
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }

    // Disable GPADC (GPADC_START will be cleared automatically unless in interval mode)
    lock = int_lock();
    if ((gpadc_chan_en & HAL_GPADC_NORM_CHAN_MASK) == 0) {
        gpadc_reg_read(GPADC_REG_START, &read_val);
        read_val &= ~GPADC_START;
        gpadc_reg_write(GPADC_REG_START, read_val);
#ifdef DCDC_CLOCK_CONTROL
        hal_cmu_dcdc_clock_disable(HAL_CMU_DCDC_CLOCK_USER_GPADC);
#endif
#ifdef GPADC_SAR_INPUT_BUF_CTRL
        pmu_sar_input_buf_enable(false);
#endif
    }
    int_unlock(lock);

    irq_checked = CHAN_DATA_INTR_MSKED(1 << HAL_GPADC_CHAN_ADCKEY) |
            KEY_RELEASE_INTR_MSKED | KEY_PRESS_INTR_MSKED |
            KEY_ERR0_INTR_MSKED | KEY_ERR1_INTR_MSKED;
    if (irq_status & irq_checked) {
        if (gpadc_event_cb[HAL_GPADC_CHAN_ADCKEY]) {
            enum HAL_ADCKEY_IRQ_STATUS_T adckey_irq;

            adckey_irq = 0;
            if (irq_status & KEY_RELEASE_INTR_MSKED) {
                adckey_irq |= HAL_ADCKEY_RELEASED;
            }
            if (irq_status & KEY_PRESS_INTR_MSKED) {
                adckey_irq |= HAL_ADCKEY_PRESSED;
            }
            if (irq_status & KEY_ERR0_INTR_MSKED) {
                adckey_irq |= HAL_ADCKEY_ERR0;
            }
            if (irq_status & KEY_ERR1_INTR_MSKED) {
                adckey_irq |= HAL_ADCKEY_ERR1;
            }

            if (irq_status & CHAN_DATA_INTR_MSKED(1 << HAL_GPADC_CHAN_ADCKEY)) {
                adckey_irq |= HAL_ADCKEY_ADC_VALID;

                lock = int_lock();

                // Int mask
                gpadc_reg_read(GPADC_REG_INT_MASK, &read_val);
                read_val &= ~CHAN_DATA_INTR_MSK(1 << HAL_GPADC_CHAN_ADCKEY);
                gpadc_reg_write(GPADC_REG_INT_MASK, read_val);
                gpadc_irq_mask = read_val;

                // Int enable
                gpadc_reg_read(GPADC_REG_INT_EN, &read_val);
                read_val &= ~CHAN_DATA_INTR_EN(1 << HAL_GPADC_CHAN_ADCKEY);
                gpadc_reg_write(GPADC_REG_INT_EN, read_val);

                hal_gpadc_enable_chan(HAL_GPADC_CHAN_ADCKEY, false);

                int_unlock(lock);

                // No voltage conversion
                gpadc_reg_read(GPADC_REG_CH0_DATA + HAL_GPADC_CHAN_ADCKEY, &adc_val);
                adc_val = GET_BITFIELD(adc_val, DATA_CHAN0);
            } else {
                adc_val = HAL_GPADC_BAD_VALUE;
            }

            ((HAL_ADCKEY_EVENT_CB_T)gpadc_event_cb[HAL_GPADC_CHAN_ADCKEY])(adckey_irq, adc_val);
        }
    }

#ifdef GPADC_START_WORKAROUND_TIMEOUT
    if (timer_running && (gpadc_irq_mask & CHAN_DATA_INTR_MSK(HAL_GPADC_ALL_CHAN_MASK)) == 0) {
        timer_running = false;
        hwtimer_stop(gpadc_timer);
    }
#endif
}

bool hal_gpadc_get_volt(enum HAL_GPADC_CHAN_T ch, HAL_GPADC_MV_T *volt)
{
    bool ret = false;
    unsigned short read_val;

    if (ch >= HAL_GPADC_CHAN_QTY || (gpadc_chan_en & (1 << ch)) == 0) {
        return ret;
    }

    gpadc_reg_read(GPADC_REG_INT_RAW_STS, &read_val);

    if (read_val & CHAN_DATA_VALID_INTR(1 << ch)) {
        // Clear the channel valid status
        hal_gpadc_clear_chan_irq(CHAN_DATA_INTR_CLR(1 << ch));

        gpadc_reg_read(GPADC_REG_CH0_DATA + ch, &read_val);
        read_val = GET_BITFIELD(read_val, DATA_CHAN0);
#ifdef GPADC_INTERVAL_MODE_WORKAROUND
        if (gpadc_interval_mode_en && !gpadc_raw_data_valid[ch] && read_val == 0) {
            gpadc_raw_data_valid[ch] = true;
            return ret;
        }
        gpadc_raw_data_valid[ch] = false;
#endif

#if defined(GPADC_HAS_EXT_SLOPE_CAL) || defined(GPADC_HAS_EXT_SLOPE_CAL2)
        *volt = hal_gpadc_adc2volt_ext(read_val, ch);
#else
        *volt = hal_gpadc_adc2volt(read_val);
#endif

        ret = true;
    }

#ifdef GPADC_START_WORKAROUND_TIMEOUT
    uint32_t cur_time = hal_sys_timer_get();

    if (cur_time - chan_start_time[ch] > gpadc_timeout_ticks) {
        uint16_t val;
        uint16_t mask;
        uint32_t lock;

        if (ch == HAL_GPADC_CHAN_ADCKEY) {
            mask = KEY_START;
        } else {
            mask = GPADC_START;
        }

        lock = int_lock();
        gpadc_reg_read(GPADC_REG_START, &val);
        if ((val & mask) == 0) {
            val |= mask;
            gpadc_reg_write(GPADC_REG_START, val);
            chan_start_time[ch] = cur_time;
        }
        int_unlock(lock);
   }
#endif

    return ret;
}

bool hal_gpadc_get_volt_blocked(enum HAL_GPADC_CHAN_T ch, HAL_GPADC_MV_T *volt)
{
    bool ret = false;
    uint32_t start;

    if (ch >= HAL_GPADC_CHAN_QTY || (gpadc_chan_en & (1 << ch)) == 0) {
        return ret;
    }

    start = hal_sys_timer_get();
    do {
        ret = hal_gpadc_get_volt(ch, volt);
    } while(!ret && ((hal_sys_timer_get() - start) < HAL_GPADC_STS_UPDATE_TIMEOUT));

    return ret;
}

static void hal_gpadc_irq_control(void)
{
    if (gpadc_irq_enabled || adckey_irq_enabled) {
        if (!irq_enabled) {
            irq_enabled = true;
#ifdef PMU_IRQ_UNIFIED
            pmu_set_irq_unified_handler(PMU_IRQ_TYPE_GPADC, hal_gpadc_irq_handler);
#else
            NVIC_SetVector(GPADC_IRQn, (uint32_t)hal_gpadc_irq_handler);
            NVIC_SetPriority(GPADC_IRQn, IRQ_PRIORITY_NORMAL);
            NVIC_ClearPendingIRQ(GPADC_IRQn);
            NVIC_EnableIRQ(GPADC_IRQn);
#endif
        }
    } else {
        if (irq_enabled) {
            irq_enabled = false;
#ifdef PMU_IRQ_UNIFIED
            pmu_set_irq_unified_handler(PMU_IRQ_TYPE_GPADC, NULL);
#else
            NVIC_DisableIRQ(GPADC_IRQn);
#endif
        }
    }
}

int hal_gpadc_open(enum HAL_GPADC_CHAN_T channel, enum HAL_GPADC_ATP_T atp, HAL_GPADC_EVENT_CB_T cb)
{
    uint32_t lock;
    unsigned short val;
    unsigned short reg_start_mask;
    bool cfg_vbat_div;

#ifdef GPADC_DYNAMIC_DATA_BITS
    if (gpadc_data_bits == 0) {
        gpadc_data_bits = pmu_get_gpadc_data_bits();
        ASSERT(10 <= gpadc_data_bits && gpadc_data_bits <= 16, "GPADC value bits not in range: %u", gpadc_data_bits);
    }
#endif
#if (CFG_HW_ADCKEY_NUMBER > 0)
    ASSERT(atp == HAL_GPADC_ATP_NULL || atp == HAL_GPADC_ATP_ONESHOT,
        "GPADC cannot work in INTERVAL mode when ADCKEY in use");
#endif
#ifdef GPADC_START_WORKAROUND_INTERVAL
#if (CFG_HW_ADCKEY_NUMBER > 0)
#error "GPADC WORKAROUND INTERVAL cannot work when ADCKEY in use"
#endif
    ASSERT(atp == HAL_GPADC_ATP_NULL || atp == WORKAROUND_ATP_VAL || atp == HAL_GPADC_ATP_ONESHOT,
        "Bad atp=%d in GPADC WORKAROUND INTERVAL", atp);
#endif

    if (channel >= HAL_GPADC_CHAN_QTY || (gpadc_chan_en & (1 << channel))) {
        return -1;
    }

    // NOTE: ADCKEY callback is not set here, but in hal_adckey_set_irq_handler()
    if (channel != HAL_GPADC_CHAN_ADCKEY) {
#ifdef GPADC_SAR_INPUT_BUF_CTRL
        if (gpadc_chan_en == 0) {
            pmu_sar_input_buf_enable(true);
        }
#endif
        gpadc_event_cb[channel] = cb;
        gpadc_atp[channel] = atp;
    }

    switch (channel) {
        case HAL_GPADC_CHAN_BATTERY:
            // Enable vbat div
            cfg_vbat_div = true;
            lock = int_lock();
#ifdef VBAT_DIV_ALWAYS_ON
            if (vbat_div_enabled) {
                cfg_vbat_div = false;
            } else {
                vbat_div_enabled = true;
            }
#endif
            if (cfg_vbat_div) {
                hal_gpadc_enable_vbat_div(true);
            }
            int_unlock(lock);
            if (cfg_vbat_div) {
                hal_sys_timer_delay_us(VBAT_DIV_STABLE_TIME_US);
            }
            // FALLTHROUGH
        case HAL_GPADC_CHAN_0:
        case HAL_GPADC_CHAN_2:
        case HAL_GPADC_CHAN_3:
        case HAL_GPADC_CHAN_4:
        case HAL_GPADC_CHAN_5:
        case HAL_GPADC_CHAN_6:
        case HAL_GPADC_CHAN_ADCKEY:
            lock = int_lock();

#ifdef DCDC_CLOCK_CONTROL
            if (channel != HAL_GPADC_CHAN_ADCKEY) {
                hal_cmu_dcdc_clock_enable(HAL_CMU_DCDC_CLOCK_USER_GPADC);
            }
#endif

#ifdef GPADC_CHAN_3_BOND_LED2
            if (channel == HAL_GPADC_CHAN_3) {
                pmu_led_set_hiz(HAL_GPIO_PIN_LED2);
            }
#endif

#ifdef GPADC_HAS_VSYS_DIV
            if (channel != HAL_GPADC_CHAN_BATTERY) {
                pmu_gpadc_div_ctrl(channel, true);
                hal_sys_timer_delay_us(20);
            }
#endif

            // Int mask
            if (channel == HAL_GPADC_CHAN_ADCKEY || gpadc_event_cb[channel]) {
                // 1) Always enable ADCKEY mask
                // 2) Enable mask if handler is not null
                gpadc_reg_read(GPADC_REG_INT_MASK, &val);
                val |= CHAN_DATA_INTR_MSK(1 << channel);
                gpadc_reg_write(GPADC_REG_INT_MASK, val);
                gpadc_irq_mask = val;
                gpadc_irq_enabled = true;
                hal_gpadc_irq_control();
            }

            // Int enable
            gpadc_reg_read(GPADC_REG_INT_EN, &val);
            val |= CHAN_DATA_INTR_EN(1 << channel);
            gpadc_reg_write(GPADC_REG_INT_EN, val);

            // Clear the channel valid status
            hal_gpadc_clear_chan_irq(CHAN_DATA_INTR_CLR(1 << channel));

            // Channel enable
            if (channel == HAL_GPADC_CHAN_ADCKEY) {
                reg_start_mask = KEY_START;
            } else {
                hal_gpadc_update_atp();
#ifdef GPADC_INTERVAL_MODE_WORKAROUND
                if (gpadc_interval_mode_en) {
                    gpadc_raw_data_valid[channel] = false;
                }
#endif
                reg_start_mask = GPADC_START;
#if (GPADC_CTRL_VER == 1)
                reg_start_mask |= CHAN_EN_REG(1 << channel);
#endif
            }
            hal_gpadc_enable_chan(channel, true);

            // GPADC enable
            gpadc_reg_read(GPADC_REG_START, &val);
            val |= reg_start_mask;
            gpadc_reg_write(GPADC_REG_START, val);

#ifdef GPADC_START_WORKAROUND_TIMEOUT
            chan_start_time[channel] = hal_sys_timer_get();

            if (!timer_running && (channel == HAL_GPADC_CHAN_ADCKEY || gpadc_event_cb[channel])) {
                enum E_HWTIMER_T ret;

                if (!gpadc_timer) {
                    gpadc_timer = hwtimer_alloc(gpadc_timer_handler, NULL);
                    ASSERT(gpadc_timer, "Failed to alloc gpadc timer");
                }

                timer_running = true;
                ret = hwtimer_start(gpadc_timer, gpadc_timeout_ticks);
                ASSERT(ret == E_HWTIMER_OK, "Failed to start gpadc timer: %d", ret);
            }
#endif

            int_unlock(lock);
            break;
        default:
            break;
    }

    return 0;
}

int hal_gpadc_close(enum HAL_GPADC_CHAN_T channel)
{
    uint32_t lock;
    unsigned short val;
    unsigned short chan_int_en;
    unsigned short reg_start;

    if (channel >= HAL_GPADC_CHAN_QTY || (gpadc_chan_en & (1 << channel)) == 0) {
        return -1;
    }

    if (channel != HAL_GPADC_CHAN_ADCKEY) {
        gpadc_atp[channel] = HAL_GPADC_ATP_NULL;
    }

    switch (channel) {
        case HAL_GPADC_CHAN_BATTERY:
        case HAL_GPADC_CHAN_0:
        case HAL_GPADC_CHAN_2:
        case HAL_GPADC_CHAN_3:
        case HAL_GPADC_CHAN_4:
        case HAL_GPADC_CHAN_5:
        case HAL_GPADC_CHAN_6:
        case HAL_GPADC_CHAN_ADCKEY:
            lock = int_lock();

#ifndef VBAT_DIV_ALWAYS_ON
            if (channel == HAL_GPADC_CHAN_BATTERY) {
                hal_gpadc_enable_vbat_div(false);
            }
#endif

#ifdef GPADC_HAS_VSYS_DIV
            if (channel != HAL_GPADC_CHAN_BATTERY) {
                pmu_gpadc_div_ctrl(channel, false);
            }
#endif

            // Int mask
            gpadc_reg_read(GPADC_REG_INT_MASK, &val);
            val &= ~CHAN_DATA_INTR_MSK(1 << channel);
            gpadc_reg_write(GPADC_REG_INT_MASK, val);
            gpadc_irq_mask = val;

            // Int enable
            gpadc_reg_read(GPADC_REG_INT_EN, &chan_int_en);
            chan_int_en &= ~CHAN_DATA_INTR_EN(1 << channel);
            gpadc_reg_write(GPADC_REG_INT_EN, chan_int_en);

            // Channel enable
            hal_gpadc_enable_chan(channel, false);
            gpadc_reg_read(GPADC_REG_START, &reg_start);
            if (channel == HAL_GPADC_CHAN_ADCKEY) {
                reg_start &= ~KEY_START;
            } else {
#if (GPADC_CTRL_VER == 1)
                reg_start &= ~CHAN_EN_REG(1 << channel);
#endif
                if (gpadc_chan_en & HAL_GPADC_NORM_CHAN_MASK) {
                    hal_gpadc_update_atp();
                } else {
                    reg_start &= ~GPADC_START;
                }
            }
            gpadc_reg_write(GPADC_REG_START, reg_start);

#if defined(DCDC_CLOCK_CONTROL) || defined(GPADC_SAR_INPUT_BUF_CTRL)
            if ((reg_start & GPADC_START) == 0) {
#ifdef DCDC_CLOCK_CONTROL
                hal_cmu_dcdc_clock_disable(HAL_CMU_DCDC_CLOCK_USER_GPADC);
#endif
#ifdef GPADC_SAR_INPUT_BUF_CTRL
                pmu_sar_input_buf_enable(false);
#endif
            }
#endif

            if ((gpadc_irq_mask & CHAN_DATA_INTR_MSK(HAL_GPADC_ALL_CHAN_MASK)) == 0) {
                gpadc_irq_enabled = false;
                hal_gpadc_irq_control();
#ifdef GPADC_START_WORKAROUND_TIMEOUT
                if (timer_running) {
                    timer_running = false;
                    hwtimer_stop(gpadc_timer);
                }
#endif
            }

            int_unlock(lock);
            break;
        default:
            break;
    }

    return 0;
}

void hal_gpadc_sleep(void)
{
#ifdef VBAT_DIV_ALWAYS_ON
    if (vbat_div_enabled) {
        hal_gpadc_enable_vbat_div(false);
    }
#endif
}

void hal_gpadc_wakeup(void)
{
#ifdef VBAT_DIV_ALWAYS_ON
    if (vbat_div_enabled) {
        hal_gpadc_enable_vbat_div(true);
    }
#endif
}

void hal_adckey_set_irq_handler(HAL_ADCKEY_EVENT_CB_T cb)
{
    gpadc_event_cb[HAL_GPADC_CHAN_ADCKEY] = (HAL_GPADC_EVENT_CB_T)cb;
}

int hal_adckey_set_irq(enum HAL_ADCKEY_IRQ_T type)
{
    uint32_t lock;
    uint16_t val;
    uint16_t set_mask;
    uint16_t clr_mask;
    uint16_t set_en;
    uint16_t clr_en;

    set_mask = 0;
    clr_mask = 0;
    set_en = 0;
    clr_en = 0;
    if (type == HAL_ADCKEY_IRQ_NONE) {
        clr_mask = KEY_RELEASE_INTR_MSK | KEY_PRESS_INTR_MSK | KEY_ERR0_INTR_MSK | KEY_ERR1_INTR_MSK;
        clr_en = KEY_RELEASE_INTR_EN | KEY_PRESS_INTR_EN | KEY_ERR0_INTR_EN | KEY_ERR1_INTR_EN;
        adckey_irq_enabled = false;
    } else if (type == HAL_ADCKEY_IRQ_PRESSED) {
        set_mask = KEY_PRESS_INTR_MSK | KEY_ERR0_INTR_MSK | KEY_ERR1_INTR_MSK;
        clr_mask = KEY_RELEASE_INTR_MSK;
        set_en = KEY_PRESS_INTR_EN | KEY_ERR0_INTR_EN | KEY_ERR1_INTR_EN;
        clr_en = KEY_RELEASE_INTR_EN;
        adckey_irq_enabled = true;
    } else if (type == HAL_ADCKEY_IRQ_RELEASED) {
        set_mask = KEY_RELEASE_INTR_MSK | KEY_ERR0_INTR_MSK | KEY_ERR1_INTR_MSK;
        clr_mask = KEY_PRESS_INTR_MSK;
        set_en = KEY_RELEASE_INTR_EN | KEY_ERR0_INTR_EN | KEY_ERR1_INTR_EN;
        clr_en = KEY_PRESS_INTR_EN;
        adckey_irq_enabled = true;
    } else if (type == HAL_ADCKEY_IRQ_BOTH) {
        set_mask = KEY_RELEASE_INTR_MSK | KEY_PRESS_INTR_MSK | KEY_ERR0_INTR_MSK | KEY_ERR1_INTR_MSK;
        set_en = KEY_RELEASE_INTR_EN | KEY_PRESS_INTR_EN | KEY_ERR0_INTR_EN | KEY_ERR1_INTR_EN;
        adckey_irq_enabled = true;
    } else {
        return 1;
    }

#ifdef DCDC_CLOCK_CONTROL
    if (adckey_irq_enabled) {
        hal_cmu_dcdc_clock_enable(HAL_CMU_DCDC_CLOCK_USER_ADCKEY);
    }
    // Never disable adckey clock
#endif

    lock = int_lock();

    gpadc_reg_read(GPADC_REG_INT_MASK, &val);
    val &= ~clr_mask;
    val |= set_mask;
    gpadc_reg_write(GPADC_REG_INT_MASK, val);
    gpadc_irq_mask = val;

    gpadc_reg_read(GPADC_REG_INT_EN, &val);
    val &= ~clr_en;
    val |= set_en;
    gpadc_reg_write(GPADC_REG_INT_EN, val);

    hal_gpadc_irq_control();

    int_unlock(lock);

    return 0;
}

#endif // !GPADC_CHIP_SPECIFIC

