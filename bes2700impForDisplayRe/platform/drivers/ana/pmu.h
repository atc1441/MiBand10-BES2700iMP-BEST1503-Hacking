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
#ifndef __PMU_H__
#define __PMU_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "hal_analogif.h"
#include "hal_cmu.h"
#include "hal_gpio.h"
#include "hal_gpadc.h"
#include "plat_addr_map.h"
#include CHIP_SPECIFIC_HDR(pmu)

#define PMU_DEBUG_TRACE(n, s, ...)          TR_DUMMY(n, s, ##__VA_ARGS__)
#define PMU_INFO_TRACE_IMM(n, s, ...)       TR_INFO((n) | TR_ATTR_IMM, s, ##__VA_ARGS__)
#define PMU_INFO_TRACE(n, s, ...)           TR_INFO(n, s, ##__VA_ARGS__)

#ifndef ISPI_PMU_REG
#define ISPI_PMU_REG(reg)                   (reg)
#endif
#define pmu_read(reg, val)                  hal_analogif_reg_read(ISPI_PMU_REG(reg), val)
#define pmu_write(reg, val)                 hal_analogif_reg_write(ISPI_PMU_REG(reg), val)

#define  PMU_MANUAL_MODE                    1
#define  PMU_AUTO_MODE                      0
#define  PMU_LDO_ON                         1
#define  PMU_LDO_OFF                        0
#define  PMU_LP_MODE_ON                     1
#define  PMU_LP_MODE_OFF                    0
#define  PMU_DSLEEP_MODE_ON                 1
#define  PMU_DSLEEP_MODE_OFF                0

#ifndef RTC_CALENDAR_YEAR_REF
#define RTC_CALENDAR_YEAR_REF               2000
#endif

#ifndef SECURITY_VALUE_T
union SECURITY_VALUE_T {
    struct {
        unsigned short security_en      :1;
        unsigned short key_id           :3;
        unsigned short key_chksum       :2;
        unsigned short vendor_id        :6;
        unsigned short vendor_chksum    :3;
        unsigned short chksum           :1;
    };
    unsigned short reg;
};
#endif

enum PMU_CHARGER_STATUS_T {
    PMU_CHARGER_PLUGIN,
    PMU_CHARGER_PLUGOUT,
    PMU_CHARGER_UNKNOWN,
};

enum PMU_POWER_ON_CAUSE_T {
    PMU_POWER_ON_CAUSE_NONE,
    PMU_POWER_ON_CAUSE_POWER_KEY,
    PMU_POWER_ON_CAUSE_RTC,
    PMU_POWER_ON_CAUSE_CHARGER_ACON,
    PMU_POWER_ON_CAUSE_CHARGER_ACOFF,
};

enum PMU_USB_PIN_CHK_STATUS_T {
    PMU_USB_PIN_CHK_NONE,
    // Chip acts as host
    PMU_USB_PIN_CHK_DEV_CONN,
    // Chip acts as host
    PMU_USB_PIN_CHK_DEV_DISCONN,
    // Chip acts as device
    PMU_USB_PIN_CHK_HOST_RESUME,

    PMU_USB_PIN_CHK_STATUS_QTY
};

enum PMU_USB_CONFIG_TYPE_T {
    PMU_USB_CONFIG_TYPE_NONE,
    PMU_USB_CONFIG_TYPE_DEVICE,
    PMU_USB_CONFIG_TYPE_HOST,
};

enum PMU_POWER_MODE_T {
    PMU_POWER_MODE_NONE,
    PMU_POWER_MODE_LDO,
    PMU_POWER_MODE_ANA_DCDC,
    PMU_POWER_MODE_DIG_DCDC,
};

enum PMU_VIORISE_REQ_USER_T {
   PMU_VIORISE_REQ_USER_PWL0,
   PMU_VIORISE_REQ_USER_PWL1,
   PMU_VIORISE_REQ_USER_FLASH,
   PMU_VIORISE_REQ_USER_CHARGER,

   PMU_VIORISE_REQ_USER_QTY
};

enum PMU_BOOT_CAUSE_T {
    PMU_BOOT_CAUSE_NULL                 = 0,
    PMU_BOOT_CAUSE_POWER_KEY            = (1 << 0),
    PMU_BOOT_CAUSE_DIG_WDT              = (1 << 1),
    PMU_BOOT_CAUSE_DIG_REBOOT           = (1 << 2),
    PMU_BOOT_CAUSE_AC_IN                = (1 << 3),
    PMU_BOOT_CAUSE_AC_OUT               = (1 << 4),
    PMU_BOOT_CAUSE_RTC                  = (1 << 5),
    PMU_BOOT_CAUSE_PMU_WDT              = (1 << 6),
    PMU_BOOT_CAUSE_RESET_KEY            = (1 << 7),
    PMU_BOOT_CAUSE_LONG_PRESS           = (1 << 8),
    PMU_BOOT_CAUSE_VCHG_COMMAND         = (1 << 9),
    PMU_BOOT_CAUSE_VCORE_LOW            = (1 << 10),
    PMU_BOOT_CAUSE_VBAT_OVP             = (1 << 11),
};

struct PMU_LED_BR_CFG_T {
    uint16_t off_time_ms;
    uint16_t on_time_ms;
    uint16_t fade_time_ms;
};

typedef void (*PMU_USB_PIN_CHK_CALLBACK)(enum PMU_USB_PIN_CHK_STATUS_T status);

typedef void (*PMU_CHARGER_IRQ_HANDLER_T)(enum PMU_CHARGER_STATUS_T status);

typedef void (*PMU_WDT_IRQ_HANDLER_T)(void);

typedef void (*PMU_CAP_IRQ_HANDLER_T)(void);

typedef void (*PMU_IRQ_UNIFIED_HANDLER_T)(uint16_t irq_status);

void pmu_boot_init(void);

int pmu_open(void);

void pmu_sleep(void);

void pmu_wakeup(void);

void pmu_mode_change(enum PMU_POWER_MODE_T mode);

int pmu_get_security_value(union SECURITY_VALUE_T *val);

void pmu_shutdown(void);

void pmu_reboot(void);

int pmu_get_efuse(enum PMU_EFUSE_PAGE_T page, unsigned short *efuse);

void bbpll_freq_pll_config(uint32_t freq);

void pmu_codec_config(int enable);

void pmu_anc_config(int enable);

void pmu_usb_config(enum PMU_USB_CONFIG_TYPE_T type);

void pmu_sleep_en(unsigned char sleep_en);

void pmu_flash_write_config(void);

void pmu_flash_read_config(void);

void pmu_flash_freq_config(uint32_t freq);

void pmu_psram_freq_config(uint32_t freq);

void pmu_psramuhs_freq_config(uint32_t freq);

void pmu_fir_high_speed_config(int enable);

void pmu_fir_freq_config(uint32_t freq);

void pmu_iir_freq_config(uint32_t freq);

void pmu_iir_eq_freq_config(uint32_t freq);

void pmu_rs_freq_config(uint32_t freq);

void pmu_rs_adc_freq_config(uint32_t freq);

void pmu_sys_freq_config(enum HAL_CMU_FREQ_T freq);

void pmu_high_performance_mode_enable(bool enable);

void pmu_charger_init(void);

void pmu_charger_set_irq_handler(PMU_CHARGER_IRQ_HANDLER_T handler);

void pmu_charger_plugin_config(void);

void pmu_charger_plugout_config(void);

enum PMU_CHARGER_STATUS_T pmu_charger_get_status(void);

void pmu_charger_out_poweron_enable(void);

void pmu_charger_out_poweron_disable(void);

int pmu_usb_config_pin_status_check(enum PMU_USB_PIN_CHK_STATUS_T status, PMU_USB_PIN_CHK_CALLBACK callback, int enable);

void pmu_usb_enable_pin_status_check(void);

void pmu_usb_disable_pin_status_check(void);

void pmu_usb_get_pin_status(int *dp, int *dm);

void pmu_codec_mic_bias_set_volt(uint32_t map, uint32_t mv);

void pmu_viorise_req(enum PMU_VIORISE_REQ_USER_T user, bool rise);

enum PMU_POWER_ON_CAUSE_T pmu_get_power_on_cause(void);

int pmu_debug_config_ana(uint16_t volt);

int pmu_debug_config_codec(uint16_t volt);

int pmu_debug_config_audio_output(bool diff);

int pmu_debug_config_vcrystal(bool on);

void pmu_debug_reliability_test(int stage);

void pmu_at_skip_shutdown(bool enable);

void pmu_led_set_direction(enum HAL_GPIO_PIN_T pin, enum HAL_GPIO_DIR_T dir);

enum HAL_GPIO_DIR_T pmu_led_get_direction(enum HAL_GPIO_PIN_T pin);

void pmu_led_set_voltage_domains(enum HAL_IOMUX_PIN_T pin, enum HAL_IOMUX_PIN_VOLTAGE_DOMAINS_T volt);

void pmu_led_set_pull_select(enum HAL_IOMUX_PIN_T pin, enum HAL_IOMUX_PIN_PULL_SELECT_T pull_sel);

void pmu_led_set_value(enum HAL_GPIO_PIN_T pin, int data);

int pmu_led_get_value(enum HAL_GPIO_PIN_T pin);

void pmu_led_breathing_enable(enum HAL_IOMUX_PIN_T pin, const struct PMU_LED_BR_CFG_T *cfg);

void pmu_led_breathing_disable(enum HAL_IOMUX_PIN_T pin);

void pmu_wdt_set_irq_handler(PMU_WDT_IRQ_HANDLER_T handler);

int pmu_wdt_config(uint32_t irq_ms, uint32_t reset_ms);

void pmu_wdt_start(void);

void pmu_wdt_stop(void);

void pmu_wdt_feed(void);

void pmu_ntc_capture_enable(void);

void pmu_ntc_capture_disable(void);

int pmu_set_irq_unified_handler(enum PMU_IRQ_TYPE_T type, PMU_IRQ_UNIFIED_HANDLER_T hdlr);

enum HAL_PWRKEY_IRQ_T pmu_pwrkey_irq_value_to_state(uint16_t irq_status);

int pmu_gpadc_div_ctrl(enum HAL_GPADC_CHAN_T channel, int enable);

enum PMU_BOOT_CAUSE_T pmu_boot_cause_get(void);

bool pmu_boot_first_power_up(void);

void pmu_power_key_hw_reset_enable(uint8_t seconds);

void pmu_power_key_hw_reset_disable(void);

//=========================================================================
// APIs for pmu rtc
//=========================================================================
#ifdef RTC_ENABLE
typedef void (*PMU_RTC_IRQ_HANDLER_T)(uint32_t seconds);

void pmu_rtc_enable(void);

void pmu_rtc_disable(void);

int pmu_rtc_enabled(void);

void pmu_rtc_set(uint32_t seconds);

uint32_t pmu_rtc_get(void);

void pmu_rtc_set_alarm(uint32_t seconds);

uint32_t pmu_rtc_get_alarm(void);

void pmu_rtc_clear_alarm(void);

int pmu_rtc_alarm_status_set(void);

int pmu_rtc_alarm_alerted();

void pmu_rtc_set_irq_handler(PMU_RTC_IRQ_HANDLER_T handler);

#elif defined(RTC_CALENDAR)

typedef void (*PMU_RTC_CALENDAR_IRQ_HANDLER_T)(struct RTC_CALENDAR_FORMAT_T *calendar);

int pmu_rtc_calendar_alarm_irq_handler_set(PMU_RTC_CALENDAR_IRQ_HANDLER_T handler);

int pmu_rtc_calendar_alarm_clear(void);

int pmu_rtc_calendar_alarm_set(struct RTC_CALENDAR_FORMAT_T *alarm_set);

int pmu_rtc_calendar_alarm_get(struct RTC_CALENDAR_FORMAT_T *alarm_get);

int pmu_rtc_calendar_set(struct RTC_CALENDAR_FORMAT_T *calendar_set);

int pmu_rtc_calendar_get(struct RTC_CALENDAR_FORMAT_T *calendar_get);

int pmu_rtc_calendar_pwron_enable(bool en);
#endif

//=========================================================================
// APIs for internal use only
//=========================================================================

void pmu_wdt_save_context(void);

void pmu_wdt_restore_context(void);

void pmu_charger_save_context(void);

void pmu_charger_shutdown_config(void);

int pmu_get_gpadc_data_bits(void);

void pmu_bt_rcosc_cal(bool first_boot);

void pmu_capsensor_close(void);

void pmu_capsensor_open(uint8_t clk_cap_div);

int pmu_capsensor_set_irq_handler(PMU_CAP_IRQ_HANDLER_T handler);

void pmu_reg_val_print(void);

#ifdef __cplusplus
}
#endif

#endif
