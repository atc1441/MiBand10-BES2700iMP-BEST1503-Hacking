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
#include "hal_pwm.h"
#include "reg_pwm.h"
#include "cmsis.h"
#include "hal_cmu.h"
#include "hal_timer.h"
#include "hal_trace.h"

#ifdef PWM_BASE

#define PWM_SLOW_CLOCK                  (CONFIG_SYSTICK_HZ)
#define PWM_FAST_CLOCK                  (hal_cmu_get_crystal_freq() / 2)
#define PWM_MAX_VALUE                   0xFFFF

// Max allowed PWM freqency error in percentage
#define PWM_MAX_FREQ_ERR_PCT            5

static struct PWM_T * const pwm[] = {
    (struct PWM_T *)PWM_BASE,
#ifdef PWM1_BASE
    (struct PWM_T *)PWM1_BASE,
#endif
};

static const enum HAL_CMU_MOD_ID_T pwm_o_mod[] = {
    HAL_CMU_MOD_O_PWM0,
#ifdef PWM1_BASE
    HAL_CMU_MOD_O_PWM4,
#endif
};

static const enum HAL_CMU_MOD_ID_T pwm_p_mod[] = {
    HAL_CMU_MOD_P_PWM,
#ifdef PWM1_BASE
    HAL_CMU_MOD_P_PWM1,
#endif
};

static uint8_t pwm_map;
STATIC_ASSERT(sizeof(pwm_map) * 8 >= HAL_PWM_ID_QTY, "pwm_map size too small");

int hal_pwm_enable(enum HAL_PWM_ID_T id, const struct HAL_PWM_CFG_T *cfg)
{
    uint32_t mod_freq;
    uint32_t load;
    uint32_t toggle;
    uint32_t lock;
    uint8_t ratio;
    uint8_t index;
    uint8_t offset;

    if (id >= HAL_PWM_ID_QTY) {
        return 1;
    }
    if (cfg->ratio > 100) {
        return 2;
    }

    if (cfg->inv && (cfg->ratio == 0 || cfg->ratio == 100)) {
        ratio = 100 - cfg->ratio;
    } else {
        ratio = cfg->ratio;
    }

#ifdef PWM_TRY_SLOW_CLOCK
    mod_freq = PWM_SLOW_CLOCK;
#else
    if (cfg->sleep_on) {
        mod_freq = PWM_SLOW_CLOCK;
    } else {
        mod_freq = PWM_FAST_CLOCK;
    }
#endif

    if (ratio == 100) {
        load = PWM_MAX_VALUE;
        toggle = PWM_MAX_VALUE;
    } else if (ratio == 0) {
        load = 0;
        toggle = 0;
    } else {
        load = mod_freq / cfg->freq;
        toggle = load * ratio / 100;
        if (toggle == 0) {
            toggle = 1;
        }
#ifdef PWM_TRY_SLOW_CLOCK
        // Check PWM frequency error in percentage
        if (!cfg->sleep_on && ABS((int)(toggle * 100 - load * ratio)) > load * PWM_MAX_FREQ_ERR_PCT) {
            mod_freq = PWM_FAST_CLOCK;
            load = mod_freq / cfg->freq;
            toggle = load * ratio / 100;
        }
#endif
        load = PWM_MAX_VALUE + 1 - load;
        toggle = PWM_MAX_VALUE - toggle;
    }

#ifdef PWM1_BASE
    if (id < HAL_PWM1_ID_0) {
        index = 0;
        offset = id - HAL_PWM_ID_0;
    } else {
        index = 1;
        offset = id - HAL_PWM1_ID_0;
    }
#else
    index = 0;
    offset = id - HAL_PWM_ID_0;
#endif

    lock = int_lock();

    if ((pwm_map & (1 << id)) == 0) {
        pwm_map |= (1 << id);
        hal_cmu_clock_enable(pwm_o_mod[index] + offset);
        hal_cmu_clock_enable(pwm_p_mod[index]);
        hal_cmu_reset_clear(pwm_o_mod[index] + offset);
        hal_cmu_reset_clear(pwm_p_mod[index]);
    } else {
        pwm[index]->EN &= ~(1 << offset);
    }

    if (ratio == 0) {
        // Output 0 when disabled
        goto _exit;
    }

    hal_cmu_pwm_set_freq(id, (mod_freq == PWM_FAST_CLOCK) ? mod_freq : 0);

    if (offset == 0) {
        pwm[index]->LOAD01 = SET_BITFIELD(pwm[index]->LOAD01, PWM_LOAD01_0, load);
        pwm[index]->TOGGLE01 = SET_BITFIELD(pwm[index]->TOGGLE01, PWM_TOGGLE01_0, toggle);
    } else if (offset == 1) {
        pwm[index]->LOAD01 = SET_BITFIELD(pwm[index]->LOAD01, PWM_LOAD01_1, load);
        pwm[index]->TOGGLE01 = SET_BITFIELD(pwm[index]->TOGGLE01, PWM_TOGGLE01_1, toggle);
    } else if (offset == 2) {
        pwm[index]->LOAD23 = SET_BITFIELD(pwm[index]->LOAD23, PWM_LOAD23_2, load);
        pwm[index]->TOGGLE23 = SET_BITFIELD(pwm[index]->TOGGLE23, PWM_TOGGLE23_2, toggle);
    } else {
        pwm[index]->LOAD23 = SET_BITFIELD(pwm[index]->LOAD23, PWM_LOAD23_3, load);
        pwm[index]->TOGGLE23 = SET_BITFIELD(pwm[index]->TOGGLE23, PWM_TOGGLE23_3, toggle);
    }

    if (cfg->inv) {
        pwm[index]->INV |= (1 << offset);
    } else {
        pwm[index]->INV &= ~(1 << offset);
    }

    pwm[index]->EN |= (1 << offset);

_exit:
    int_unlock(lock);

    return 0;
}

int hal_pwm_disable(enum HAL_PWM_ID_T id)
{
    uint8_t index;
    uint8_t offset;
    uint32_t lock;
    uint32_t mask;

    if (id >= HAL_PWM_ID_QTY) {
        return 1;
    }

#ifdef PWM1_BASE
    if (id < HAL_PWM1_ID_0) {
        index = 0;
        offset = id - HAL_PWM_ID_0;
        mask = (1 << HAL_PWM1_ID_0) - 1;
    } else {
        index = 1;
        offset = id - HAL_PWM1_ID_0;
        mask = (1 << HAL_PWM_ID_QTY) - (1 << HAL_PWM1_ID_0);
    }
#else
    index = 0;
    offset = id - HAL_PWM_ID_0;
    mask = (1 << HAL_PWM_ID_QTY) - 1;
#endif

    lock = int_lock();

    if ((pwm_map & (1 << id)) == 0) {
        goto _exit;
    }
    pwm_map &= ~(1 << id);

    pwm[index]->EN &= ~(1 << offset);
    hal_cmu_reset_set(pwm_o_mod[index] + offset);
    hal_cmu_clock_disable(pwm_o_mod[index] + offset);
    if ((pwm_map & mask) == 0) {
        hal_cmu_reset_set(pwm_p_mod[index]);
        hal_cmu_clock_disable(pwm_p_mod[index]);
    }

_exit:
    int_unlock(lock);

    return 0;
}

int hal_pwm_breathing_led_enable(enum HAL_PWM_ID_T id, const struct HAL_PWM_BR_CFG_T *cfg)
{
    uint8_t index;
    uint8_t offset;
    uint32_t lock;
    uint32_t st1;
    uint32_t st2;
    uint32_t subcnt_data;
    uint8_t tg;

    if (id >= HAL_PWM_ID_QTY) {
        return 1;
    }

#ifdef PWM1_BASE
    if (id < HAL_PWM1_ID_0) {
        index = 0;
        offset = id - HAL_PWM_ID_0;
    } else {
        index = 1;
        offset = id - HAL_PWM1_ID_0;
    }
#else
    index = 0;
    offset = id - HAL_PWM_ID_0;
#endif

#if !(defined(CHIP_BEST2003))
    if (offset != 2 && offset != 3) {
        return 2;
    }
#endif

    st1 = MS_TO_TICKS(cfg->off_time_ms);
    if (st1 > (REG_PWM2_ST1_MASK >> REG_PWM2_ST1_SHIFT)) {
        st1 = (REG_PWM2_ST1_MASK >> REG_PWM2_ST1_SHIFT);
    }
    st2 = MS_TO_TICKS(cfg->on_time_ms);
    if (st2 > (PWM_TOGGLE23_2_MASK >> PWM_TOGGLE23_2_SHIFT)) {
        st2 = (PWM_TOGGLE23_2_MASK >> PWM_TOGGLE23_2_SHIFT);
    }
    subcnt_data = MS_TO_TICKS(cfg->fade_time_ms);
    subcnt_data = integer_sqrt_nearest(subcnt_data);
    if (subcnt_data > (SUBCNT_DATA2_MASK >> SUBCNT_DATA2_SHIFT)) {
        subcnt_data = (SUBCNT_DATA2_MASK >> SUBCNT_DATA2_SHIFT);
    }
    // TODO: HW will be fixed from 2500
    if (subcnt_data > 0xFE) {
        subcnt_data = 0xFE;
    }
    tg = 1;

    lock = int_lock();

    if (hal_cmu_reset_get_status(pwm_o_mod[index] + offset) == HAL_CMU_RST_SET) {
        hal_cmu_clock_enable(pwm_o_mod[index] + offset);
        hal_cmu_clock_enable(pwm_p_mod[index]);
        hal_cmu_reset_clear(pwm_o_mod[index] + offset);
        hal_cmu_reset_clear(pwm_p_mod[index]);
    } else {
        pwm[index]->EN &= ~(1 << offset);
    }

    hal_cmu_pwm_set_freq(id, 0);//default frequency:32K

#if !(defined(CHIP_BEST2003))
    if (offset == 2) {
        pwm[index]->ST1_23 = SET_BITFIELD(pwm[index]->ST1_23, REG_PWM2_ST1, st1);
        pwm[index]->TOGGLE23 = SET_BITFIELD(pwm[index]->TOGGLE23, PWM_TOGGLE23_2, st2);
        pwm[index]->TWINKLE23 = (pwm[index]->TWINKLE23 & ~(SUBCNT_DATA2_MASK | TG_SUBCNT_D2_ST_MASK)) |
            SUBCNT_DATA2(subcnt_data) | TG_SUBCNT_D2_ST(tg) | REG_PWM2_BR_EN;
    } else {
        pwm[index]->ST1_23 = SET_BITFIELD(pwm[index]->ST1_23, REG_PWM3_ST1, st1);
        pwm[index]->TOGGLE23 = SET_BITFIELD(pwm[index]->TOGGLE23, PWM_TOGGLE23_3, st2);
        pwm[index]->TWINKLE23 = (pwm[index]->TWINKLE23 & ~(SUBCNT_DATA3_MASK | TG_SUBCNT_D3_ST_MASK)) |
            SUBCNT_DATA3(subcnt_data) | TG_SUBCNT_D3_ST(tg) | REG_PWM3_BR_EN;
    }
#else
    if (offset == 3) {
        pwm[index]->ST23 = SET_BITFIELD(pwm[index]->ST23, REG_PWM3_ST1, st1);
        pwm[index]->TOGGLE23 = SET_BITFIELD(pwm[index]->TOGGLE23, PWM_TOGGLE23_3, st2);
        pwm[index]->TWINKLE23 = (pwm[index]->TWINKLE23 & ~(SUBCNT_DATA3_MASK | TG_SUBCNT_D3_ST_MASK)) |
            SUBCNT_DATA3(subcnt_data) | TG_SUBCNT_D3_ST(tg) | REG_PWM3_BR_EN;
    } else if (offset == 2) {
        pwm[index]->ST23 = SET_BITFIELD(pwm[index]->ST23, REG_PWM2_ST1, st1);
        pwm[index]->TOGGLE23 = SET_BITFIELD(pwm[index]->TOGGLE23, PWM_TOGGLE23_2, st2);
        pwm[index]->TWINKLE23 = (pwm[index]->TWINKLE23 & ~(SUBCNT_DATA2_MASK | TG_SUBCNT_D2_ST_MASK)) |
            SUBCNT_DATA2(subcnt_data) | TG_SUBCNT_D2_ST(tg) | REG_PWM2_BR_EN;
    } else if (offset == 1) {
        pwm[index]->ST01 = SET_BITFIELD(pwm[index]->ST01, REG_PWM1_ST1, st1);
        pwm[index]->TOGGLE01 = SET_BITFIELD(pwm[index]->TOGGLE01, PWM_TOGGLE01_1, st2);
        pwm[index]->TWINKLE01 = (pwm[index]->TWINKLE01 & ~(SUBCNT_DATA1_MASK | TG_SUBCNT_D1_ST_MASK)) |
            SUBCNT_DATA1(subcnt_data) | TG_SUBCNT_D1_ST(tg) | REG_PWM1_BR_EN;
    } else {
        pwm[index]->ST01 = SET_BITFIELD(pwm[index]->ST01, REG_PWM0_ST1, st1);
        pwm[index]->TOGGLE01 = SET_BITFIELD(pwm[index]->TOGGLE01, PWM_TOGGLE01_0, st2);
        pwm[index]->TWINKLE01 = (pwm[index]->TWINKLE01 & ~(SUBCNT_DATA0_MASK | TG_SUBCNT_D0_ST_MASK)) |
            SUBCNT_DATA0(subcnt_data) | TG_SUBCNT_D0_ST(tg) | REG_PWM0_BR_EN;
    }
#endif

    pwm[index]->EN |= (1 << offset);

    int_unlock(lock);

    return 0;
}

int hal_pwm_breathing_led_disable(enum HAL_PWM_ID_T id)
{
    uint8_t index;
    uint8_t offset;
    uint32_t lock;

    if (id >= HAL_PWM_ID_QTY) {
        return 1;
    }

#ifdef PWM1_BASE
    if (id < HAL_PWM1_ID_0) {
        index = 0;
        offset = id - HAL_PWM_ID_0;
    } else {
        index = 1;
        offset = id - HAL_PWM1_ID_0;
    }
#else
    index = 0;
    offset = id - HAL_PWM_ID_0;
#endif

    if (offset != 2 && offset != 3) {
        return 2;
    }

    lock = int_lock();

    if (hal_cmu_reset_get_status(pwm_o_mod[index] + offset) == HAL_CMU_RST_SET) {
        goto _exit;
    }

    if (offset == 2) {
        pwm[index]->TWINKLE23 &= ~REG_PWM2_BR_EN;
    } else {
        pwm[index]->TWINKLE23 &= ~REG_PWM3_BR_EN;
    }

    hal_pwm_disable(id);

_exit:
    int_unlock(lock);

    return 0;
}

#endif
