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
#if (CHIP_GPIO_VER >= 2)

#include "plat_addr_map.h"
#include "cmsis_nvic.h"
#include "hal_gpio.h"
#ifdef PSC_GPIO_IRQ_CTRL
#include "hal_psc.h"
#endif
#include "hal_timer.h"
#include "hal_trace.h"
#ifdef PMU_HAS_LED_PIN
#include "pmu.h"
#endif
#include "reg_gpio_v2.h"

#define HAL_GPIO_PIN_NUM_EACH_BANK          (32)

#define HAL_GPIO_BANK_NUM                   ARRAY_SIZE(gpio_bank)

#if defined(HAL_GPIO_BANK0_PIN_NUM) && (HAL_GPIO_BANK0_PIN_NUM > HAL_GPIO_PIN_NUM_EACH_BANK)
#error "Bad HAL_GPIO_BANK0_PIN_NUM"
#endif
#ifdef GPIO1_BASE
#if defined(HAL_GPIO_BANK1_PIN_NUM) && (HAL_GPIO_BANK1_PIN_NUM > HAL_GPIO_PIN_NUM_EACH_BANK)
#error "Bad HAL_GPIO_BANK1_PIN_NUM"
#endif
#endif
#ifdef GPIO2_BASE
#if defined(HAL_GPIO_BANK2_PIN_NUM) && (HAL_GPIO_BANK2_PIN_NUM > HAL_GPIO_PIN_NUM_EACH_BANK)
#error "Bad HAL_GPIO_BANK2_PIN_NUM"
#endif
#endif

static struct GPIO_BANK_T * const gpio_bank[] = {
    (struct GPIO_BANK_T *)GPIO_BASE,
#ifdef GPIO1_BASE
    (struct GPIO_BANK_T *)GPIO1_BASE,
#endif
#ifdef GPIO2_BASE
    (struct GPIO_BANK_T *)GPIO2_BASE,
#endif
};

static const uint8_t gpio_num[HAL_GPIO_BANK_NUM] = {
#ifdef HAL_GPIO_BANK0_PIN_NUM
    HAL_GPIO_BANK0_PIN_NUM,
#else
    HAL_GPIO_PIN_NUM_EACH_BANK,
#endif
#ifdef GPIO1_BASE
#ifdef HAL_GPIO_BANK1_PIN_NUM
    HAL_GPIO_BANK1_PIN_NUM,
#else
    HAL_GPIO_PIN_NUM_EACH_BANK,
#endif
#endif
#ifdef GPIO2_BASE
#ifdef HAL_GPIO_BANK2_PIN_NUM
    HAL_GPIO_BANK2_PIN_NUM,
#else
    HAL_GPIO_PIN_NUM_EACH_BANK,
#endif
#endif
};

static HAL_GPIO_PIN_IRQ_HANDLER gpio_irq_handler[HAL_GPIO_PIN_NUM];

static const char * const err_bad_pin = "Bad GPIO pin %u (should < %u)";

static uint32_t hal_gpio_pin_to_bank(enum HAL_GPIO_PIN_T pin)
{
    uint32_t p = pin;

    for (uint32_t i = 0; i < HAL_GPIO_BANK_NUM; i++) {
        if (p < gpio_num[i]) {
            ASSERT(gpio_bank[i], "%s: Bad bank %u for gpio pin: %u", __func__, i, pin);
            return i;
        }
        p -= gpio_num[i];
    }
    ASSERT(false, "%s: Bad gpio pin: %u", __func__, pin);
    return 0;
}

static uint32_t hal_gpio_pin_to_offset(enum HAL_GPIO_PIN_T pin)
{
    uint32_t p = pin;

    for (uint32_t i = 0; i < HAL_GPIO_BANK_NUM; i++) {
        if (p < gpio_num[i]) {
            return p;
        }
        p -= gpio_num[i];
    }
    ASSERT(false, "%s: Bad gpio pin: %u", __func__, pin);
    return 0;
}

static enum HAL_GPIO_PIN_T hal_gpio_bank_offset_to_pin(uint32_t bank, uint32_t offset)
{
    uint32_t p;

    ASSERT(bank < HAL_GPIO_BANK_NUM, "%s: Bad bank: %u", __func__, bank);
    ASSERT(offset < gpio_num[bank], "%s: Bad offset @ bank %u: %u (should < %u)", __func__, bank, offset, gpio_num[bank]);

    p = 0;
    for (uint32_t i = 0; i < bank; i++) {
        p += gpio_num[i];
    }
    return (enum HAL_GPIO_PIN_T)(p + offset);
}

enum HAL_GPIO_DIR_T hal_gpio_pin_get_dir(enum HAL_GPIO_PIN_T pin)
{
    int pin_offset = 0;
    int bank = 0;
    enum HAL_GPIO_DIR_T dir = 0;

    enum HAL_GPIO_PIN_T max_pin = HAL_GPIO_PIN_NUM;
#ifdef PMU_HAS_LED_PIN
    max_pin = HAL_GPIO_PIN_LED_NUM;
#endif
#ifdef PMU_HAS_GPIO_PIN
    max_pin = HAL_GPIO_PIN_PMU_NUM;
#endif
    ASSERT(pin < max_pin, err_bad_pin, pin, max_pin);

    if (0) {
#ifdef PMU_HAS_LED_PIN
    } else if ((HAL_GPIO_PIN_LED1 <= pin) && (pin < HAL_GPIO_PIN_LED_NUM)) {
        return pmu_led_get_direction(pin);
#endif
#ifdef PMU_HAS_GPIO_PIN
    } else if (pin >= HAL_GPIO_PIN_PMU_P0_0 && pin < HAL_GPIO_PIN_PMU_NUM) {
        return pmu_gpio_pin_get_dir(pin);
#endif
    } else {
        bank = hal_gpio_pin_to_bank(pin);
        pin_offset = hal_gpio_pin_to_offset(pin);

        if (gpio_bank[bank]->GPIO_DDR_SET & (0x1 << pin_offset)) {
            dir = HAL_GPIO_DIR_OUT;
        } else {
            dir = HAL_GPIO_DIR_IN;
        }
    }

    return dir;
}

void hal_gpio_pin_set_dir(enum HAL_GPIO_PIN_T pin, enum HAL_GPIO_DIR_T dir, uint8_t val_for_out)
{
    int pin_offset = 0;
    int bank = 0;
    enum HAL_GPIO_PIN_T max_pin = HAL_GPIO_PIN_NUM;
#ifdef PMU_HAS_LED_PIN
    max_pin = HAL_GPIO_PIN_LED_NUM;
#endif
#ifdef PMU_HAS_GPIO_PIN
    max_pin = HAL_GPIO_PIN_PMU_NUM;
#endif
    ASSERT(pin < max_pin, err_bad_pin, pin, max_pin);

    if(dir == HAL_GPIO_DIR_OUT) {
        if(val_for_out) {
            hal_gpio_pin_set(pin);
        } else {
            hal_gpio_pin_clr(pin);
        }
    }

    if (0) {
#ifdef PMU_HAS_LED_PIN
    } else if ((HAL_GPIO_PIN_LED1 <= pin) && (pin < HAL_GPIO_PIN_LED_NUM)) {
        pmu_led_set_direction(pin, dir);
#endif
#ifdef PMU_HAS_GPIO_PIN
    } else if (pin >= HAL_GPIO_PIN_PMU_P0_0 && pin < HAL_GPIO_PIN_PMU_NUM) {
        pmu_gpio_pin_set_dir(pin, dir, val_for_out);
#endif
    } else {
        bank = hal_gpio_pin_to_bank(pin);
        pin_offset = hal_gpio_pin_to_offset(pin);

        if(dir == HAL_GPIO_DIR_OUT) {
            gpio_bank[bank]->GPIO_DDR_SET = (0x1 << pin_offset);
        } else {
            gpio_bank[bank]->GPIO_DDR_CLR = (0x1 << pin_offset);
        }
    }
}

void hal_gpio_pin_set(enum HAL_GPIO_PIN_T pin)
{
    int pin_offset = 0;
    int bank = 0;

    enum HAL_GPIO_PIN_T max_pin = HAL_GPIO_PIN_NUM;
#ifdef PMU_HAS_LED_PIN
    max_pin = HAL_GPIO_PIN_LED_NUM;
#endif
#ifdef PMU_HAS_GPIO_PIN
    max_pin = HAL_GPIO_PIN_PMU_NUM;
#endif
    ASSERT(pin < max_pin, err_bad_pin, pin, max_pin);

    if (0) {
#ifdef PMU_HAS_LED_PIN
    } else if ((HAL_GPIO_PIN_LED1 <= pin) && (pin < HAL_GPIO_PIN_LED_NUM)) {
        pmu_led_set_value(pin, 1);
#endif
#ifdef PMU_HAS_GPIO_PIN
    } else if (pin >= HAL_GPIO_PIN_PMU_P0_0 && pin < HAL_GPIO_PIN_PMU_NUM) {
        pmu_gpio_pin_set(pin);
#endif
    } else {
        bank = hal_gpio_pin_to_bank(pin);
        pin_offset = hal_gpio_pin_to_offset(pin);

        gpio_bank[bank]->GPIO_DR_SET = (0x1 << pin_offset);
    }
}

void hal_gpio_pin_clr(enum HAL_GPIO_PIN_T pin)
{
    int pin_offset = 0;
    int bank = 0;

    enum HAL_GPIO_PIN_T max_pin = HAL_GPIO_PIN_NUM;
#ifdef PMU_HAS_LED_PIN
    max_pin = HAL_GPIO_PIN_LED_NUM;
#endif
#ifdef PMU_HAS_GPIO_PIN
    max_pin = HAL_GPIO_PIN_PMU_NUM;
#endif
    ASSERT(pin < max_pin, err_bad_pin, pin, max_pin);

    if (0) {
#ifdef PMU_HAS_LED_PIN
    } else if ((HAL_GPIO_PIN_LED1 <= pin) && (pin < HAL_GPIO_PIN_LED_NUM)) {
        pmu_led_set_value(pin, 0);
#endif
#ifdef PMU_HAS_GPIO_PIN
    } else if (pin >= HAL_GPIO_PIN_PMU_P0_0 && pin < HAL_GPIO_PIN_PMU_NUM) {
        pmu_gpio_pin_clr(pin);
#endif
    } else {
        bank = hal_gpio_pin_to_bank(pin);
        pin_offset = hal_gpio_pin_to_offset(pin);

        gpio_bank[bank]->GPIO_DR_CLR = (0x1 << pin_offset);
    }
}

uint8_t hal_gpio_pin_get_val(enum HAL_GPIO_PIN_T pin)
{
    int pin_offset = 0;
    int bank = 0;

    enum HAL_GPIO_PIN_T max_pin = HAL_GPIO_PIN_NUM;
#ifdef PMU_HAS_LED_PIN
    max_pin = HAL_GPIO_PIN_LED_NUM;
#endif
#ifdef PMU_HAS_GPIO_PIN
    max_pin = HAL_GPIO_PIN_PMU_NUM;
#endif
    ASSERT(pin < max_pin, err_bad_pin, pin, max_pin);

    if (0) {
#ifdef PMU_HAS_LED_PIN
    } else if ((HAL_GPIO_PIN_LED1 <= pin) && (pin < HAL_GPIO_PIN_LED_NUM)) {
        return pmu_led_get_value(pin);
#endif
#ifdef PMU_HAS_GPIO_PIN
    } else if (pin >= HAL_GPIO_PIN_PMU_P0_0 && pin < HAL_GPIO_PIN_PMU_NUM) {
        return pmu_gpio_pin_get_input_val(pin);
#endif
    } else {
        bank = hal_gpio_pin_to_bank(pin);
        pin_offset = hal_gpio_pin_to_offset(pin);

        return !!(gpio_bank[bank]->GPIO_EXT_PORT & (0x1 << pin_offset));
    }
}

uint8_t hal_gpio_pin_get_out_val(enum HAL_GPIO_PIN_T pin)
{
    int pin_offset = 0;
    int bank = 0;

    enum HAL_GPIO_PIN_T max_pin = HAL_GPIO_PIN_NUM;
#ifdef PMU_HAS_LED_PIN
    max_pin = HAL_GPIO_PIN_LED_NUM;
#endif
#ifdef PMU_HAS_GPIO_PIN
    max_pin = HAL_GPIO_PIN_PMU_NUM;
#endif
    ASSERT(pin < max_pin, err_bad_pin, pin, max_pin);

    if (0) {
#ifdef PMU_HAS_GPIO_PIN
    } else if (pin >= HAL_GPIO_PIN_PMU_P0_0 && pin < HAL_GPIO_PIN_PMU_NUM) {
        return pmu_gpio_pin_get_output_val(pin);
#endif
    } else {
        bank = hal_gpio_pin_to_bank(pin);
        pin_offset = hal_gpio_pin_to_offset(pin);

        return !!(gpio_bank[bank]->GPIO_DR_SET  & (0x1 << pin_offset));
    }
}

static void _hal_gpio_irq_handler(void)
{
    uint32_t raw_status[HAL_GPIO_BANK_NUM];
    uint32_t bank = 0, pin_offset = 0;
    uint32_t cnt;
    enum HAL_GPIO_PIN_T pin;

#ifdef PSC_GPIO_IRQ_CTRL
    cnt = hal_psc_gpio_irq_get_status(raw_status, ARRAY_SIZE(raw_status));
#endif
    for (bank = cnt; bank < HAL_GPIO_BANK_NUM; bank++) {
        raw_status[bank] = gpio_bank[bank]->GPIO_RAW_INTSTATUS;
    }

    for (bank = 0; bank < HAL_GPIO_BANK_NUM; bank++) {
        gpio_bank[bank]->GPIO_PORTA_EOI = raw_status[bank];

        while (raw_status[bank]) {
            pin_offset = get_lsb_pos(raw_status[bank]);
            pin = hal_gpio_bank_offset_to_pin(bank, pin_offset);
            if (gpio_irq_handler[pin]) {
                gpio_irq_handler[pin](pin);
            }
            raw_status[bank] &= ~(0x1 << pin_offset);
        }
    }
}

uint8_t hal_gpio_setup_irq(enum HAL_GPIO_PIN_T pin, const struct HAL_GPIO_IRQ_CFG_T *cfg)
{
    int pin_offset = 0;
    int bank = 0;

    enum HAL_GPIO_PIN_T max_pin = HAL_GPIO_PIN_NUM;
#if defined(PMU_HAS_LED_PIN) && defined(PMU_HAS_LED_IRQ)
    max_pin = HAL_GPIO_PIN_LED_NUM;
#endif
#if defined(PMU_HAS_GPIO_PIN) && defined(PMU_HAS_GPIO_IRQ)
    max_pin = HAL_GPIO_PIN_PMU_NUM;
#endif
    ASSERT(pin < max_pin, err_bad_pin, pin, max_pin);

    if (0) {
#ifdef PMU_HAS_LED_PIN
    } else if ((HAL_GPIO_PIN_LED1 <= pin) && (pin < HAL_GPIO_PIN_LED_NUM)) {
#ifdef PMU_HAS_LED_IRQ
        return pmu_gpio_setup_irq(pin, cfg);
#else
        return 1;
#endif
#endif
#ifdef PMU_HAS_GPIO_PIN
    } else if (pin >= HAL_GPIO_PIN_PMU_P0_0 && pin < HAL_GPIO_PIN_PMU_NUM) {
#ifdef PMU_HAS_GPIO_IRQ
        return pmu_gpio_setup_irq(pin, cfg);
#else
        return 1;
#endif
#endif
    } else {
        bank = hal_gpio_pin_to_bank(pin);
        pin_offset = hal_gpio_pin_to_offset(pin);

        if (cfg->irq_enable) {
            gpio_bank[bank]->GPIO_INTMASK_SET = (0x1 << pin_offset);

            ASSERT(cfg->irq_polarity != HAL_GPIO_IRQ_POLARITY_EDGE_BOTH,\
                "%s: Not support edge both: pin=%u", __func__, pin);

            if (cfg->irq_type == HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE) {
                gpio_bank[bank]->GPIO_INTTYPE_LEVEL_SET = (0x1 << pin_offset);
            } else {
                gpio_bank[bank]->GPIO_INTTYPE_LEVEL_CLR = (0x1 << pin_offset);
            }

            if (cfg->irq_polarity == HAL_GPIO_IRQ_POLARITY_HIGH_RISING) {
                gpio_bank[bank]->GPIO_INT_POLARITY_SET = (0x1 << pin_offset);
            } else {
                gpio_bank[bank]->GPIO_INT_POLARITY_CLR = (0x1 << pin_offset);
            }
            // If debounce is disabled, 4 APB clocks are needed before enabling INT
            // to avoid the (first) fake edge IRQ

            if (cfg->irq_debounce) {
#ifdef GPIO_FIRST_EDGE_IRQ_WORKAROUND
                if (cfg->irq_type == HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE) {
                    // Make sure the debounce input sees the newest value (in 32K clock)
                    hal_sys_timer_delay_us(50);
                }
#endif
                gpio_bank[bank]->GPIO_DEBOUNCE_SET = (0x1 << pin_offset);
            } else {
                gpio_bank[bank]->GPIO_DEBOUNCE_CLR = (0x1 << pin_offset);
            }

            gpio_irq_handler[pin] = cfg->irq_handler;

            NVIC_SetVector(GPIO_IRQn, (uint32_t)_hal_gpio_irq_handler);
            NVIC_SetPriority(GPIO_IRQn, IRQ_PRIORITY_NORMAL);
            NVIC_EnableIRQ(GPIO_IRQn);
#ifdef PSC_GPIO_IRQ_CTRL
            hal_psc_gpio_irq_enable(pin);
#endif

            gpio_bank[bank]->GPIO_PORTA_EOI = (0x1 << pin_offset);
            gpio_bank[bank]->GPIO_INTMASK_CLR = (0x1 << pin_offset);
            gpio_bank[bank]->GPIO_INTEN_SET = (0x1 << pin_offset);
        } else {
#ifdef PSC_GPIO_IRQ_CTRL
            hal_psc_gpio_irq_disable(pin);
#endif

            gpio_bank[bank]->GPIO_INTMASK_SET = (0x1 << pin_offset);
            gpio_bank[bank]->GPIO_INTEN_CLR = (0x1 << pin_offset);
            gpio_irq_handler[pin] = NULL;
        }
    }

    return 0;
}

#endif

