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
#ifndef __PMU_BEST1306_H__
#define __PMU_BEST1306_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_cmu.h"

#define ANA_REG(r)                          (((r) & 0xFFF) | 0x1000)
#define RF_REG(r)                           (((r) & 0xFFF) | 0x2000)
#define PMU_REG(r)                          (((r) & 0xFFF) | 0x3000)
#define CHG_REG(r)                          (((r) & 0xFFF) | 0x4000)
#define USBPHY_REG(r)                       (((r) & 0xFFF) | 0x5000)

#define ISPI_PMU_REG(reg)                   (((reg) & 0xFFF) | 0x3000)

#define MAX_VMIC_CH_NUM                     2

enum PMU_EFUSE_PAGE_T {
    PMU_EFUSE_PAGE_SECURITY         = 0x0,
    PMU_EFUSE_PAGE_BOOT             = 0x1,
    PMU_EFUSE_PAGE_FEATURE          = 0x2,
    PMU_EFUSE_PAGE_BATTER_LV        = 0x3,

    PMU_EFUSE_PAGE_BATTER_HV        = 0x4,
    PMU_EFUSE_PAGE_SW_CFG           = 0x5,
    PMU_EFUSE_PAGE_PROD_TEST        = 0x6,
    PMU_EFUSE_PAGE_RESERVED_7       = 0x7,

    PMU_EFUSE_PAGE_DCDC_VOLT_CAL    = 0x8,
    PMU_EFUSE_PAGE_DCDC_VOLT_LP_CAL = 0x9,
    PMU_EFUSE_PAGE_CP_XY_COORDINATE = 0xA,
    PMU_EFUSE_PAGE_CP_WAFER_ID      = 0xB,

    PMU_EFUSE_PAGE_RESERVED_C       = 0xC,
    PMU_EFUSE_PAGE_RESERVED_D       = 0xD,
    PMU_EFUSE_PAGE_RESERVED_E       = 0xE,
    PMU_EFUSE_PAGE_RESERVED_F       = 0xF,

    PMU_EFUSE_PAGE_QTY,
};

enum PMU_IRQ_TYPE_T {
    PMU_IRQ_TYPE_GPADC,
    PMU_IRQ_TYPE_RTC,
    PMU_IRQ_TYPE_CHARGER,
    PMU_IRQ_TYPE_GPIO,
    PMU_IRQ_TYPE_WDT,
    PMU_IRQ_TYPE_PWRKEY,
    PMU_IRQ_TYPE_CHARGER_PRESENT,

    PMU_IRQ_TYPE_QTY
};

enum PMU_PLL_DIV_TYPE_T {
    PMU_PLL_DIV_DIG,
    PMU_PLL_DIV_CODEC,
};

enum PMU_BIG_BANDGAP_USER_T {
    PMU_BIG_BANDGAP_USER_GPADC          = (1 << 0),
};

enum PMU_VSENSOR_USER_T {
    PMU_VSENSOR_USER_SINGLE_WIRE_UART       = (1 << 0),
};

enum PMU_CHARGER_PRESENT_STATUS_T {
    PMU_CHARGER_PRESENT_IN,
    PMU_CHARGER_PRESENT_OUT,
};

union SECURITY_VALUE_T {
    struct {
        unsigned short security_en      :1;
        unsigned short mode             :1;
        unsigned short sig_type         :1;
        unsigned short reserved         :1;
        unsigned short key_id           :2;
        unsigned short vendor_id        :5;
        unsigned short flash_id         :1;
        unsigned short chksum           :4;
    } root;
    struct {
        unsigned short security_en      :1;
        unsigned short mode             :1;
        unsigned short sig_type         :1;
        unsigned short skip_romkey      :1;
        unsigned short key_id           :2;
        unsigned short reg_base         :2;
        unsigned short reg_size         :2;
        unsigned short reg_offset       :1;
        unsigned short flash_id         :1;
        unsigned short chksum           :4;
    } otp;
    unsigned short reg;
};
#define SECURITY_VALUE_T                    SECURITY_VALUE_T

enum HAL_FLASH_ID_T pmu_get_boot_flash_ctrl_id(void);

uint8_t pmu_gpio_setup_irq(enum HAL_GPIO_PIN_T pin, const struct HAL_GPIO_IRQ_CFG_T *cfg);

void pmu_codec_hppa_enable(int enable);

void pmu_codec_mic_bias_set_volt(uint32_t map, uint32_t mv);

void pmu_codec_mic_bias_enable(uint32_t map, int enable);

void pmu_codec_mic_bias_lowpower_mode(uint32_t map, int enable);

int pmu_codec_volt_ramp_up(void);

int pmu_codec_volt_ramp_down(void);

void pmu_pll_div_reset_set(enum HAL_CMU_PLL_T pll);

void pmu_pll_div_reset_clear(enum HAL_CMU_PLL_T pll);

void pmu_pll_div_set(enum HAL_CMU_PLL_T pll, enum PMU_PLL_DIV_TYPE_T type, uint32_t div);

void pmu_pll_freq_reg_set(uint16_t low, uint16_t high, uint16_t high2);

void pmu_pll_codec_clock_enable(bool en);

void pmu_led_set_hiz(enum HAL_GPIO_PIN_T pin);

void pmu_led_uart_enable(enum HAL_IOMUX_PIN_T pin);

void pmu_led_uart_disable(enum HAL_IOMUX_PIN_T pin);

uint32_t pmu_led_set_drv(enum HAL_IOMUX_PIN_T pin, uint32_t val_drv);

void pmu_rf_ana_init(void);

void pmu_big_bandgap_enable(enum PMU_BIG_BANDGAP_USER_T user, int enable);

void pmu_bt_reconn(bool en);

void pmu_get_vbat_calib_value(unsigned short *val_lv, unsigned short *val_hv);

void pmu_ldo_vpa_en(bool en);

void pmu_ldo_vsensor_enable(enum PMU_VSENSOR_USER_T user);

void pmu_ldo_vsensor_disable(enum PMU_VSENSOR_USER_T user);

void pmu_charger_out_poweron_enable(void);

void pmu_charger_out_poweron_disable(void);

void pmu_shipment_mode_enable(bool en);

typedef void (*PMU_NTC_IRQ_HANDLER_T)(uint16_t raw_val, int temperature);

int pmu_volt2temperature(const uint16_t volt);

int pmu_ntc_capture_start(PMU_NTC_IRQ_HANDLER_T cb);

void pmu_ntc_capture_reset(void);

uint16_t pmu_ntc_temperature_reference_get(void);

bool pmu_force_lp_bg_valid_check(void);

typedef void (*PMU_CHARGER_PRESENT_IRQ_HANDLER_T)(enum PMU_CHARGER_PRESENT_STATUS_T status);

void pmu_charger_present_set_irq_handler(PMU_CHARGER_PRESENT_IRQ_HANDLER_T handler);

enum PMU_CHARGER_PRESENT_STATUS_T pmu_charger_presernt_get_status(void);

void bbpll_codec_clock_enable(bool en);

void bbpll_pll_update(uint64_t pll_cfg_val, bool calib);

void bbpll_set_codec_div(uint32_t div);

void bbpll_auto_calib_spd(uint32_t codec_freq, uint32_t div);

bool pmu_boot_first_power_up(void);

void pmu_charger_pattern_reset_enable(bool enable);

int pmu_wdt_reboot(void);

#ifdef __cplusplus
}
#endif

#endif

