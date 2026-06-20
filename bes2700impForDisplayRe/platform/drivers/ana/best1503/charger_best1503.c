/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#include "charger.h"
#include "pmu.h"
#include CHIP_SPECIFIC_HDR(reg_charger)
#include "analog.h"
#include "cmsis.h"
#include "cmsis_nvic.h"
#include "hal_aud.h"
#include "hal_bootmode.h"
#include "hal_cache.h"
#include "hal_chipid.h"
#include "hal_cmu.h"
#include "hal_location.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "patch.h"
#include "tgt_hardware.h"

// PMU_EFUSE_PAGE_RESERVED_C
#define PMU_EFUSE_PAGE_C_CHG_PKG_TYPE_SHIFT            7
#define PMU_EFUSE_PAGE_C_CHG_PKG_TYPE_MASK             (0x3 << PMU_EFUSE_PAGE_C_CHG_PKG_TYPE_SHIFT)
#define PMU_EFUSE_PAGE_C_CHG_PKG_TYPE(n)               BITFIELD_VAL(PMU_EFUSE_PAGE_C_CHG_PKG_TYPE, n)

#define PMU_EFUSE_PAGE_C_TRIM_VREF_CC_CAL_SHIFT        4
#define PMU_EFUSE_PAGE_C_TRIM_VREF_CC_CAL_MASK         (0x3 << PMU_EFUSE_PAGE_C_TRIM_VREF_CC_CAL_SHIFT)
#define PMU_EFUSE_PAGE_C_TRIM_VREF_CC_CAL(n)           BITFIELD_VAL(PMU_EFUSE_PAGE_C_TRIM_VREF_CC_CAL, n)

// PMU_EFUSE_PAGE_RESERVED_F
#define PMU_EFUSE_PAGE_F_CHG_CALIB_FLAG                (1 << 15)

#define CHG_IRQ_MODULE_ENABLE                          1
#define CHG_IRQ_MODULE_DISABLE                         0

enum CHG_IRQ_MODULE_E {
    CHG_CHARGE_INTR     = 0,
    CHG_AC_ON_DET_IN    = 1,
    CHG_AC_ON_DET_OUT   = 2,
    CHG_TRIPRE_TIMEOUT  = 5,
    CHG_FAST_TIMEOUT    = 6,
    CHG_TEMP_COLD       = 7,
    CHG_TEMP_HOT        = 8,
    CHG_TEMP_COOL       = 9,
    CHG_TEMP_WARM       = 10,
    CHG_DONE            = 11,
    CHG_ACIN_OV         = 13,
    CHG_ACIN_CL         = 14,
    CHG_OTP             = 15,
    CHG_VSYS_LOW        = 16,

    CHG_IRQ_MODULE_QTY,
};

struct CHG_IRQ_MODULE_RAW_MAP_T {
    enum CHG_IRQ_MODULE_E module;
    uint32_t raw;
};

struct CHG_IRQ_MODULE_CFG_T {
    unsigned short intr_clr;
    unsigned short intr_mask;
    unsigned short intr_en;
    unsigned short intr_raw;
};

#define CHG_IRQ_MOD_CFG_VAL(m)              { \
    REG_##m##_INTR_CLR, REG_##m##_INTR_MASK, \
    REG_##m##_INTR_EN, REG_##m##_INTR_RAW }

static struct CHG_IRQ_MODULE_CFG_T chg_irq_module_cfg[CHG_IRQ_MODULE_QTY];

static CHARGER_CHARGE_IRQ_HANDLER_T chg_irq_handler;

static const struct CHG_IRQ_MODULE_RAW_MAP_T chg_irq_module_raw_map[] = {
    {CHG_AC_ON_DET_IN,   CHARGER_IRQ_CAUSE_AC_ON_DET_IN                     },
    {CHG_AC_ON_DET_OUT,  CHARGER_IRQ_CAUSE_AC_ON_DET_OUT                    },
    {CHG_TRIPRE_TIMEOUT, CHARGER_IRQ_CAUSE_CHARGE_TRIPRE_TIMEOUT            },
    {CHG_FAST_TIMEOUT,   CHARGER_IRQ_CAUSE_CHARGE_FAST_TIMEOUT              },
    {CHG_TEMP_COLD,      CHARGER_IRQ_CAUSE_CHARGE_TEMP_COLD_LESS_THEN_0     },
    {CHG_TEMP_HOT,       CHARGER_IRQ_CAUSE_CHARGE_TEMP_HOT_HIGHER_THEN_60   },
    {CHG_TEMP_COOL,      CHARGER_IRQ_CAUSE_CHARGE_TEMP_COOL_LESS_THEN_10    },
    {CHG_TEMP_WARM,      CHARGER_IRQ_CAUSE_CHARGE_TEMP_WARM_HIGHER_THEN_45  },
    {CHG_DONE,           CHARGER_IRQ_CAUSE_CHARGE_DONE                      },
    {CHG_ACIN_OV,        CHARGER_IRQ_CAUSE_ACIN_OV                          },
    {CHG_ACIN_CL,        CHARGER_IRQ_CAUSE_ACIN_CL                          },
    {CHG_OTP,            CHARGER_IRQ_CAUSE_OTP                              },
    {CHG_VSYS_LOW,       CHARGER_IRQ_CAUSE_VSYS_LOW                         },
};

static enum CHARGER_PACKAGE_TYPE_E BOOT_BSS_LOC chg_pkg_type;

static enum HAL_CHIP_METAL_ID_T BOOT_BSS_LOC chg_metal_id;

// For metal0 chips.
static uint8_t chg_icc0_trim_vref_cc_val_calib;
static uint8_t chg_icc0_trim_vref_cc_val_default;

#ifdef CHG_FORCE_LOW_CV_CURRENT
static bool chg_half_icc_busy = false;

static void charger_charge_rechage_enable(bool enable);
#endif

static bool BOOT_BSS_LOC chg_opened = false;

// Charger 1620 drivers begin.
static struct CHARGER_CHARGE_MODULE_CFG_T charger_1620_mod_cfg = {
    .prechg_current = 0,
    .cc_current = CHARGER_CHARGE_CONSTANT_CURRENT_100MA,
    .stop_current = 0,
    .cv_volt = 0,
    .rechg_volt = 0,
    .ntc_det_en = false,
    .rechg_en = true,
    .chg_en = true,
};

static const struct HAL_IOMUX_PIN_FUNCTION_MAP charger_1620_detect_pin[1] = {
    {HAL_IOMUX_PIN_P3_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
};

static const struct HAL_IOMUX_PIN_FUNCTION_MAP charger_1620_current_pin[1] = {
    {HAL_IOMUX_PIN_P1_0, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
};

static const struct HAL_IOMUX_PIN_FUNCTION_MAP charger_1620_enable_pin[1] = {
    {HAL_IOMUX_PIN_P0_6, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
};

static void charger_1620_charge_enable(void)
{
    uint32_t lock;

    ASSERT(chg_pkg_type == CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    if (charger_1620_enable_pin[0].pin != HAL_IOMUX_PIN_NUM) {
        lock = int_lock();
        hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)charger_1620_enable_pin[0].pin);
        int_unlock(lock);
    }
}

static void charger_1620_charge_disable(void)
{
    uint32_t lock;

    ASSERT(chg_pkg_type == CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    if (charger_1620_enable_pin[0].pin != HAL_IOMUX_PIN_NUM) {
        lock = int_lock();
        hal_gpio_pin_set((enum HAL_GPIO_PIN_T)charger_1620_enable_pin[0].pin);
        int_unlock(lock);
    }
}

static void charger_1620_charge_high_current_set(void)
{
    uint32_t lock;

    ASSERT(chg_pkg_type == CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    if (charger_1620_current_pin[0].pin != HAL_IOMUX_PIN_NUM) {
        lock = int_lock();
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)charger_1620_current_pin[0].pin, HAL_GPIO_DIR_OUT, 0);
        int_unlock(lock);
    }
}

static void charger_1620_charge_low_current_set(void)
{
    uint32_t lock;

    ASSERT(chg_pkg_type == CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    if (charger_1620_current_pin[0].pin != HAL_IOMUX_PIN_NUM) {
        lock = int_lock();
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)charger_1620_current_pin[0].pin, HAL_GPIO_DIR_IN, 0);
        int_unlock(lock);
    }
}

static int charger_1620_charge_module_cfg_set(const struct CHARGER_CHARGE_MODULE_CFG_T *cfg)
{
    uint32_t lock;

    ASSERT(chg_pkg_type == CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    lock = int_lock();
    if (cfg->chg_en) {
        charger_1620_charge_enable();
    } else {
        charger_1620_charge_disable();
    }
    charger_1620_mod_cfg.chg_en = cfg->chg_en;

    if (cfg->cc_current >= CHARGER_CHARGE_CONSTANT_CURRENT_100MA) {
        charger_1620_charge_high_current_set();
    } else {
        charger_1620_charge_low_current_set();
    }
    charger_1620_mod_cfg.cc_current = cfg->cc_current;
    int_unlock(lock);

    return 0;
}

static void charger_1620_charge_module_cfg_get(struct CHARGER_CHARGE_MODULE_CFG_T *cfg)
{
    uint32_t lock;

    ASSERT(chg_pkg_type == CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    lock = int_lock();
    cfg->prechg_current = charger_1620_mod_cfg.prechg_current;
    cfg->stop_current = charger_1620_mod_cfg.stop_current;
    cfg->cc_current = charger_1620_mod_cfg.cc_current;
    cfg->chg_en = charger_1620_mod_cfg.chg_en;
    cfg->ntc_det_en = charger_1620_mod_cfg.ntc_det_en;
    cfg->cv_volt = charger_1620_mod_cfg.cv_volt;
    cfg->rechg_volt = charger_1620_mod_cfg.rechg_volt;
    cfg->rechg_en = cfg->chg_en;
    int_unlock(lock);
}

static enum CHARGER_ACIN_STATUS_TYPE_E charger_1620_acin_status_get(void)
{
    uint32_t lock;
    enum CHARGER_ACIN_STATUS_TYPE_E status = CHARGER_ACIN_STATUS_PLUGOUT;

    ASSERT(chg_pkg_type == CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    lock = int_lock();
    if (hal_gpio_pin_get_val(HAL_GPIO_PIN_LED2)) {
        status = CHARGER_ACIN_STATUS_PLUGIN;
    }
    int_unlock(lock);

    return status;
}

static void charger_1620_charge_open(void)
{
    uint32_t lock;

    ASSERT(chg_pkg_type == CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    lock = int_lock();
    if (charger_1620_detect_pin[0].pin != HAL_IOMUX_PIN_NUM) {
        hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&charger_1620_detect_pin[0], ARRAY_SIZE(charger_1620_detect_pin));
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)charger_1620_detect_pin[0].pin, HAL_GPIO_DIR_IN, 0);
    }

    if (charger_1620_current_pin[0].pin != HAL_IOMUX_PIN_NUM) {
        hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&charger_1620_current_pin[0], ARRAY_SIZE(charger_1620_current_pin));
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)charger_1620_current_pin[0].pin, HAL_GPIO_DIR_IN, 0);
    }

    if (charger_1620_enable_pin[0].pin != HAL_IOMUX_PIN_NUM) {
        hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&charger_1620_enable_pin[0], ARRAY_SIZE(charger_1620_enable_pin));
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)charger_1620_enable_pin[0].pin, HAL_GPIO_DIR_OUT, 0);
        charger_1620_charge_module_cfg_set(&charger_1620_mod_cfg);
    }
    int_unlock(lock);
}
// Charger 1620 drivers end.

static void charger_charge_icc0_current_calib_resume(void)
{
    uint16_t val;

    ASSERT(chg_pkg_type != CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    if (chg_metal_id == HAL_CHIP_METAL_ID_0) {
        chg_read(CHG_REG_TRIM_VREF_CFG, &val);
        val = SET_BITFIELD(val, REG_TRIM_VREF_CC, chg_icc0_trim_vref_cc_val_default);
        chg_write(CHG_REG_TRIM_VREF_CFG, val);
    }
}

static void charger_charge_icc0_current_calib_set(void)
{
    uint16_t val;

    ASSERT(chg_pkg_type != CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    if (chg_metal_id == HAL_CHIP_METAL_ID_0) {
        chg_read(CHG_REG_TRIM_VREF_CFG, &val);
        val = SET_BITFIELD(val, REG_TRIM_VREF_CC, chg_icc0_trim_vref_cc_val_calib);
        chg_write(CHG_REG_TRIM_VREF_CFG, val);
    }
}

static void charger_charge_icc0_current_calib_get(void)
{
    uint16_t val;

    ASSERT(chg_pkg_type != CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    if (chg_metal_id == HAL_CHIP_METAL_ID_0) {
        chg_read(CHG_REG_TRIM_VREF_CFG, &val);
        chg_icc0_trim_vref_cc_val_default = GET_BITFIELD(val, REG_TRIM_VREF_CC);

        pmu_get_efuse(PMU_EFUSE_PAGE_RESERVED_C, &val);
        chg_icc0_trim_vref_cc_val_calib = chg_icc0_trim_vref_cc_val_default | GET_BITFIELD(val, PMU_EFUSE_PAGE_C_TRIM_VREF_CC_CAL);

        TRACE(0, "%s chg_icc0_trim_vref_cc_val default/calib:0x%x/0x%x", __func__, chg_icc0_trim_vref_cc_val_default, chg_icc0_trim_vref_cc_val_calib);
    }
}

#ifdef CHG_FORCE_LOW_CV_CURRENT
static void charger_charge_cv_force_low_iterm_config_set(void)
{
    uint32_t lock;
    uint16_t val;
    uint16_t val_rechg_en;
    uint16_t val_icc_half;

    ASSERT(chg_pkg_type != CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    lock = int_lock();
    chg_read(CHG_REG_AC_IN_PP_CFG, &val_rechg_en);
    chg_read(CHG_REG_ICC_HALF_INTF_PMU, &val_icc_half);
    TRACE(0, "%s val_rechg_en=0x%04x val_icc_half=0x%04x", __func__, val_rechg_en, val_icc_half);
    if (val_rechg_en & REG_CHARGER_RECHARGE_EN) {
        TRACE(0, "OFF->DONE");
        val_rechg_en &= ~REG_CHARGER_RECHARGE_EN;
        chg_write(CHG_REG_AC_IN_PP_CFG, val_rechg_en);

        val_icc_half &= ~REG_NTC_HALF_ICC_DR;
        chg_write(CHG_REG_ICC_HALF_INTF_PMU, val_icc_half);

        chg_half_icc_busy = false;
    } else {
        TRACE(0, "FAST->DONE");
        val_icc_half |= REG_NTC_HALF_ICC_DR | REG_NTC_HALF_ICC;
        chg_write(CHG_REG_ICC_HALF_INTF_PMU, val_icc_half);

        val_rechg_en |= REG_CHARGER_RECHARGE_EN;
        chg_write(CHG_REG_AC_IN_PP_CFG, val_rechg_en);

        chg_read(CHG_REG_STANDBY_SHIP_MODE, &val);
        val |= REG_DONE_SW_RECHARGE;
        chg_write(CHG_REG_STANDBY_SHIP_MODE, val);

        //  Avoid half icc be cleared by charger_charge_half_icc_current_disable().
        chg_half_icc_busy = true;
    }
    int_unlock(lock);
}
#endif

static void charger_charge_irq_module_init(void)
{
    static bool inited = false;

    ASSERT(chg_pkg_type != CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    if (inited) {
        return;
    }
    inited = !inited;

    chg_irq_module_cfg[CHG_AC_ON_DET_IN] = (struct CHG_IRQ_MODULE_CFG_T)CHG_IRQ_MOD_CFG_VAL(CHARGE_AC_ON_DET_IN);
    chg_irq_module_cfg[CHG_AC_ON_DET_OUT] = (struct CHG_IRQ_MODULE_CFG_T)CHG_IRQ_MOD_CFG_VAL(CHARGE_AC_ON_DET_OUT);
    chg_irq_module_cfg[CHG_TRIPRE_TIMEOUT] = (struct CHG_IRQ_MODULE_CFG_T)CHG_IRQ_MOD_CFG_VAL(CHARGE_TRIPRE_TIMEOUT);
    chg_irq_module_cfg[CHG_FAST_TIMEOUT] = (struct CHG_IRQ_MODULE_CFG_T)CHG_IRQ_MOD_CFG_VAL(CHARGE_FAST_TIMEOUT);
    chg_irq_module_cfg[CHG_TEMP_COLD] = (struct CHG_IRQ_MODULE_CFG_T)CHG_IRQ_MOD_CFG_VAL(CHARGE_TEMP_COLD);
    chg_irq_module_cfg[CHG_TEMP_HOT] = (struct CHG_IRQ_MODULE_CFG_T)CHG_IRQ_MOD_CFG_VAL(CHARGE_TEMP_HOT);
    chg_irq_module_cfg[CHG_TEMP_COOL] = (struct CHG_IRQ_MODULE_CFG_T)CHG_IRQ_MOD_CFG_VAL(CHARGE_TEMP_COOL);
    chg_irq_module_cfg[CHG_TEMP_WARM] = (struct CHG_IRQ_MODULE_CFG_T)CHG_IRQ_MOD_CFG_VAL(CHARGE_TEMP_WARM);
    chg_irq_module_cfg[CHG_DONE] = (struct CHG_IRQ_MODULE_CFG_T)CHG_IRQ_MOD_CFG_VAL(CHARGE_DONE);
    chg_irq_module_cfg[CHG_ACIN_OV] = (struct CHG_IRQ_MODULE_CFG_T)CHG_IRQ_MOD_CFG_VAL(CHARGE_ACIN_OV);
    chg_irq_module_cfg[CHG_ACIN_CL] = (struct CHG_IRQ_MODULE_CFG_T)CHG_IRQ_MOD_CFG_VAL(CHARGE_ACIN_CL);
    chg_irq_module_cfg[CHG_OTP] = (struct CHG_IRQ_MODULE_CFG_T)CHG_IRQ_MOD_CFG_VAL(CHARGE_OTP);
    chg_irq_module_cfg[CHG_VSYS_LOW] = (struct CHG_IRQ_MODULE_CFG_T)CHG_IRQ_MOD_CFG_VAL(CHARGE_VSYS_LOW);
}

static inline uint16_t charger_charge_irq_module_addr_get(enum CHG_IRQ_MODULE_E module)
{
    ASSERT(chg_pkg_type != CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    if (module == CHG_AC_ON_DET_IN || module == CHG_AC_ON_DET_OUT) {
        return CHG_REG_AC_ON_DET_INTR_CFG;
    } else if (module == CHG_TRIPRE_TIMEOUT || module == CHG_FAST_TIMEOUT) {
        return CHG_REG_FAST_TRI_TIMEOUT_INTR_CFG;
    } else if (module >= CHG_TEMP_COLD && module <= CHG_TEMP_WARM) {
        return CHG_REG_TEMP_STATUS_INTR_CFG;
    } else if (module == CHG_DONE) {
        return CHG_REG_CHG_DONE_INTR_CFG;
    } else if (module == CHG_VSYS_LOW) {
        return CHG_REG_VSYS_LOW_INTR_CFG;
    } else {
        return CHG_REG_OTP_ACIN_VSYS_SC_INTR_CFG;
    }
}

static void charger_charge_irq_module_clr(enum CHG_IRQ_MODULE_E module)
{
    unsigned short val;
    unsigned short module_address;
    const struct CHG_IRQ_MODULE_CFG_T *module_cfg_p = &chg_irq_module_cfg[module];

    ASSERT(chg_pkg_type != CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    module_address = charger_charge_irq_module_addr_get(module);

    chg_read(module_address, &val);
    val |= module_cfg_p->intr_clr;
    chg_write(module_address, val);
}

static void charger_charge_irq_module_cfg(enum CHG_IRQ_MODULE_E module, int irq_en)
{
    unsigned short val;
    unsigned short module_address;
    const struct CHG_IRQ_MODULE_CFG_T *module_cfg_p = &chg_irq_module_cfg[module];

    ASSERT(chg_pkg_type != CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    module_address = charger_charge_irq_module_addr_get(module);

    chg_read(module_address, &val);
    if (irq_en) {
        val |= module_cfg_p->intr_en | module_cfg_p->intr_mask;
    } else {
        val &= ~(module_cfg_p->intr_en | module_cfg_p->intr_mask);
    }
    chg_write(module_address, val);
}

static void charger_charge_irq_handler(void)
{
    enum CHG_IRQ_MODULE_E irq_module = CHG_CHARGE_INTR;
    uint32_t lock;
    uint32_t irq_raw_total;
    uint16_t irq1_raw;
    uint16_t irq2_raw;

    ASSERT(chg_pkg_type != CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    lock = int_lock();
    chg_read(CHG_REG_CHG_INTR1, &irq1_raw);
    chg_read(CHG_REG_CHG_INTR2, &irq2_raw);

    irq1_raw &= ~CHRG_INTR;
    irq1_raw = GET_BITFIELD(irq1_raw, IRQ1_STATUS);
    irq2_raw = GET_BITFIELD(irq2_raw, IRQ2_STATUS);
    irq_raw_total = irq1_raw | irq2_raw << IRQ1_NUM;
    //TRACE(0, "%s irq_raw_total=0x%08x", __func__, irq_raw_total);

    for (int i = 0; i < ARRAY_SIZE(chg_irq_module_raw_map); i++) {
        if (!!(chg_irq_module_raw_map[i].raw & irq_raw_total)) {
            irq_module = chg_irq_module_raw_map[i].module;
            charger_charge_irq_module_clr(irq_module);
            //TRACE(0, "%s irq_module=%d", __func__, irq_module);
        }
    }
    int_unlock(lock);

#ifdef CHG_FORCE_LOW_CV_CURRENT
    if ((irq_raw_total & (CHARGER_IRQ_CAUSE_AC_ON_DET_IN | CHARGER_IRQ_CAUSE_AC_ON_DET_OUT)) && chg_half_icc_busy) {
        // Reset force low CV cunrrent status.
        chg_half_icc_busy = false;
        charger_charge_half_icc_current_disable();
        charger_charge_rechage_enable(false);
    }
    if (irq_raw_total & CHARGER_IRQ_CAUSE_CHARGE_DONE) {
        charger_charge_cv_force_low_iterm_config_set();
    }
#endif

    if (chg_irq_handler) {
        chg_irq_handler(irq_raw_total);
    }
}

static void charger_charge_pre_current_set(uint16_t pre_current_new, enum CHARGER_CHARGE_STATUE_E status)
{
    uint16_t val_read;
    uint16_t val;

    ASSERT(chg_pkg_type != CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    chg_read(CHG_REG_CC_PRE_CV_CFG, &val_read);
    val = GET_BITFIELD(val_read, REG_CHARGER_IPRE_SEL);
    if ((status == CHARGER_CHARGE_STATUS_TRICKLE || status == CHARGER_CHARGE_STATUS_PRE) && val < pre_current_new) {
        while (val++ < pre_current_new) {
            val_read = SET_BITFIELD(val_read, REG_CHARGER_IPRE_SEL, val);
            chg_write(CHG_REG_CC_PRE_CV_CFG, val_read);
        }
    } else {
        val_read = SET_BITFIELD(val_read, REG_CHARGER_IPRE_SEL, pre_current_new);
        chg_write(CHG_REG_CC_PRE_CV_CFG, val_read);
    }
}

static void charger_charge_cc_current_set(uint16_t cc_current_new, enum CHARGER_CHARGE_STATUE_E status)
{
    static uint16_t cc_current_old = CHARGER_CHARGE_CONSTANT_CURRENT_60MA;
    uint16_t val_read;
    uint16_t val;
    bool update = false;

    ASSERT(chg_pkg_type != CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    if (chg_metal_id == HAL_CHIP_METAL_ID_0) {
        if (cc_current_old != cc_current_new) {
            update = true;
        }
        if (chg_icc0_trim_vref_cc_val_default != chg_icc0_trim_vref_cc_val_calib && update) {
            if (cc_current_new == CHARGER_CHARGE_CONSTANT_CURRENT_20MA) {
                charger_charge_icc0_current_calib_set();
            } else {
                charger_charge_icc0_current_calib_resume();
            }
        }
        cc_current_old = cc_current_new;
    }

    chg_read(CHG_REG_CC_PRE_CV_CFG, &val_read);
    val = GET_BITFIELD(val_read, REG_CHARGER_ICC_SEL);
    if ((status == CHARGER_CHARGE_STATUS_PRE || status == CHARGER_CHARGE_STATUS_FAST) && val < cc_current_new) {
        while (val++ < cc_current_new) {
            val_read = SET_BITFIELD(val_read, REG_CHARGER_ICC_SEL, val);
            chg_write(CHG_REG_CC_PRE_CV_CFG, val_read);
        }
    } else {
        val_read = SET_BITFIELD(val_read, REG_CHARGER_ICC_SEL, cc_current_new);
        chg_write(CHG_REG_CC_PRE_CV_CFG, val_read);
    }
}

static void charger_charge_stop_current_set(uint16_t stop_current_new, enum CHARGER_CHARGE_STATUE_E status)
{
    uint16_t val_read;
    uint16_t val;

    ASSERT(chg_pkg_type != CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    chg_read(CHG_REG_VRECHG_ITERM_CFG, &val_read);
    val = GET_BITFIELD(val_read, CHARGER_ITERM_SEL);
    if ((status == CHARGER_CHARGE_STATUS_FAST || status == CHARGER_CHARGE_STATUS_DONE) && val < stop_current_new) {
        while (val++ < stop_current_new) {
            val_read = SET_BITFIELD(val_read, CHARGER_ITERM_SEL, val);
            chg_write(CHG_REG_VRECHG_ITERM_CFG, val_read);
        }
    } else {
        val_read = SET_BITFIELD(val_read, CHARGER_ITERM_SEL, stop_current_new);
        chg_write(CHG_REG_VRECHG_ITERM_CFG, val_read);
    }
}

static void charger_charge_cv_voltage_set(uint16_t cv_volt_new, enum CHARGER_CHARGE_STATUE_E status)
{
    uint16_t val_read;
    uint16_t val;

    ASSERT(chg_pkg_type != CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    chg_read(CHG_REG_CC_PRE_CV_CFG, &val_read);
    val = GET_BITFIELD(val_read, REG_CHARGER_VOREG_BIT);
    if ((status == CHARGER_CHARGE_STATUS_FAST || status == CHARGER_CHARGE_STATUS_DONE) && val < cv_volt_new) {
        while (val++ < cv_volt_new) {
            val_read = SET_BITFIELD(val_read, REG_CHARGER_VOREG_BIT, val);
            chg_write(CHG_REG_CC_PRE_CV_CFG, val_read);
        }
    } else {
        val_read = SET_BITFIELD(val_read, REG_CHARGER_VOREG_BIT, cv_volt_new);
        chg_write(CHG_REG_CC_PRE_CV_CFG, val_read);
    }
}

static void charger_charge_recharge_voltage_set(uint16_t rechg_volt)
{
    uint16_t val;

    ASSERT(chg_pkg_type != CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    chg_read(CHG_REG_VRECHG_ITERM_CFG, &val);
    val= SET_BITFIELD(val, CHARGER_VRECHARGE_SEL, rechg_volt);
    chg_write(CHG_REG_VRECHG_ITERM_CFG, val);
}

static void charger_charge_ntc_enable(bool enable)
{
    uint16_t val;

    ASSERT(chg_pkg_type != CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    chg_read(CHG_REG_CHG_FUNC_CFG, &val);
    if (enable) {
        val |= REG_CHARGER_NTC_EN;
    } else {
        val &= ~REG_CHARGER_NTC_EN;
    }
    chg_write(CHG_REG_CHG_FUNC_CFG, val);
}

static void charger_charge_rechage_enable(bool enable)
{
    uint16_t val;

    ASSERT(chg_pkg_type != CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    chg_read(CHG_REG_AC_IN_PP_CFG, &val);
    if (enable) {
        val |= REG_CHARGER_RECHARGE_EN;
    } else {
        val &= ~REG_CHARGER_RECHARGE_EN;
    }
    chg_write(CHG_REG_AC_IN_PP_CFG, val);
}

#ifdef ACIN_PATTERN_DISABLE
static void charger_charge_pattern_disable(void)
{
    uint16_t val;

    chg_read(CHG_PATTERN0_RST_CHARGER_CFG, &val);
    val &= ~REG_COMMAND_0_ENABLE;
    chg_write(CHG_PATTERN0_RST_CHARGER_CFG, val);

    chg_read(CHG_PATTERN1_SHIPMODE_ON_CFG, &val);
    val &= ~REG_COMMAND_1_ENABLE;
    chg_write(CHG_PATTERN1_SHIPMODE_ON_CFG, val);

    chg_read(CHG_PATTERN2_SHIPMODE_OFF_CFG, &val);
    val &= ~REG_COMMAND_2_ENABLE;
    chg_write(CHG_PATTERN2_SHIPMODE_OFF_CFG, val);

    chg_read(CHG_PATTERN3_SINGLE_UART_ON_CFG, &val);
    val &= ~REG_COMMAND_3_ENABLE;
    chg_write(CHG_PATTERN3_SINGLE_UART_ON_CFG, val);

    chg_read(CHG_PATTERN4_SINGLE_UART_OFF_CFG, &val);
    val &= ~REG_COMMAND_4_ENABLE;
    chg_write(CHG_PATTERN4_SINGLE_UART_OFF_CFG, val);
}
#endif

static void BOOT_TEXT_FLASH_LOC charger_charge_low_vrtc_enable(bool enable)
{
    uint16_t val;
    uint16_t val_vrtc;

    if (chg_metal_id == HAL_CHIP_METAL_ID_1) {
        chg_read(CHG_REG_SHIP_MODE_VRTC_CFG, &val);
        if (enable) {
            val= SET_BITFIELD(val, VRTC0P8_SEL, 0x9);
            chg_write(CHG_REG_SHIP_MODE_VRTC_CFG, val);
        } else {
            val_vrtc = GET_BITFIELD(val, VRTC0P8_SEL);
            while (val_vrtc++ < 0xE) {
                val= SET_BITFIELD(val, VRTC0P8_SEL, val_vrtc);
                chg_write(CHG_REG_SHIP_MODE_VRTC_CFG, val);
            }
            hal_sys_timer_delay_us(10);
        }
    }
}

void charger_charge_irq_handler_set(CHARGER_CHARGE_IRQ_HANDLER_T handler)
{
    uint32_t lock;

    ASSERT(chg_pkg_type != CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    if (handler) {
        lock = int_lock();
        chg_irq_handler = handler;
        int_unlock(lock);
    }
}

enum CHARGER_CHARGE_STATUE_E charger_charge_status_get(void)
{
    uint32_t lock;
    uint16_t val;

    ASSERT(chg_pkg_type != CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    lock = int_lock();
    chg_read(CHG_REG_CHG_STATUS3, &val);
    val= GET_BITFIELD(val, CHARGE_STATE);
    int_unlock(lock);

    return (enum CHARGER_CHARGE_STATUE_E)val;
}

void charger_charge_module_cfg_get(struct CHARGER_CHARGE_MODULE_CFG_T *cfg)
{
    uint32_t lock;
    uint16_t val;

    if (chg_pkg_type == CHARGER_PACKAGE_TYPE_SIP_1620) {
        charger_1620_charge_module_cfg_get(cfg);
        return;
    }

    lock = int_lock();
    chg_read(CHG_REG_CC_PRE_CV_CFG, &val);
    cfg->cv_volt = GET_BITFIELD(val, REG_CHARGER_VOREG_BIT);
    cfg->prechg_current = GET_BITFIELD(val, REG_CHARGER_IPRE_SEL);
    cfg->cc_current = GET_BITFIELD(val, REG_CHARGER_ICC_SEL);

    chg_read(CHG_REG_VRECHG_ITERM_CFG, &val);
    cfg->stop_current = GET_BITFIELD(val, CHARGER_ITERM_SEL);
    cfg->rechg_volt= GET_BITFIELD(val, CHARGER_VRECHARGE_SEL);

    chg_read(CHG_REG_CHG_FUNC_CFG, &val);
    cfg->ntc_det_en = !!(val & REG_CHARGER_NTC_EN);
    cfg->chg_en = !!(val & REG_CHARGER_CHARGE_EN);

    chg_read(CHG_REG_AC_IN_PP_CFG, &val);
    cfg->rechg_en = !!(val & REG_CHARGER_RECHARGE_EN);
    int_unlock(lock);
}

int charger_charge_module_cfg_set(struct CHARGER_CHARGE_MODULE_CFG_T *cfg)
{
    enum CHARGER_CHARGE_STATUE_E status;
    uint32_t lock;

    if (chg_pkg_type == CHARGER_PACKAGE_TYPE_SIP_1620) {
        charger_1620_charge_module_cfg_set(cfg);
        return 0;
    }

    if (cfg->prechg_current >= CHARGER_CHARGE_PRECHARGE_CURRENT_QTY) {
        return 1;
    }

    if (cfg->cc_current >= CHARGER_CHARGE_CONSTANT_CURRENT_QTY) {
        return 2;
    }

    if (cfg->stop_current >= CHARGER_CHARGE_STOP_CURRENT_QTY) {
        return 3;
    }

    if (cfg->cv_volt >= CHARGER_CHARGING_CONSTANT_VOLTAGE_QTY) {
        return 4;
    }

    if (cfg->rechg_volt >= CHARGER_CHARGE_RECHARGE_VOLTAGE_QTY) {
        return 5;
    }

    lock = int_lock();
    status = charger_charge_status_get();

    if (chg_metal_id == HAL_CHIP_METAL_ID_0) {
        if (cfg->cc_current > CHARGER_CHARGE_CONSTANT_CURRENT_40MA && cfg->cc_current < CHARGER_CHARGE_CONSTANT_CURRENT_300MA) {
            // Remap cc curent level.
            cfg->cc_current -= 1;
        }
    }

    charger_charge_pre_current_set(cfg->prechg_current, status);
    charger_charge_cc_current_set(cfg->cc_current, status);
    charger_charge_stop_current_set(cfg->stop_current, status);
    charger_charge_cv_voltage_set(cfg->cv_volt, status);
    charger_charge_recharge_voltage_set(cfg->rechg_volt);
    charger_charge_ntc_enable(cfg->ntc_det_en);
#ifndef CHG_FORCE_LOW_CV_CURRENT
    // Set rechg_en by charger irq handler only.
    charger_charge_rechage_enable(cfg->rechg_en);
#endif
    if (cfg->chg_en) {
        charger_charge_enable();
    } else {
        charger_charge_disable();
    }
    int_unlock(lock);

    return 0;
}

void charger_charge_open(void)
{
    uint32_t lock;
    uint16_t val;
    uint16_t val_efuse;

    ASSERT(chg_pkg_type != CHARGER_PACKAGE_TYPE_QTY, "%s Invalid charger package type!", __func__);

    if (chg_opened) {
        return;
    }

    chg_opened = true;

    TRACE(0, "%s chg_pkg_type=%d chg_metal_id=%d", __func__, chg_pkg_type, chg_metal_id);
    if (chg_pkg_type == CHARGER_PACKAGE_TYPE_SIP_1620) {
        charger_1620_charge_open();
        return;
    }

    lock = int_lock();
    // Disable and clear all CHG irqs by default
    // CHG irqs cannot be cleared by CHG soft reset
    val = REG_AC_ON_DET_IN_INTR_CLR | REG_AC_ON_DET_OUT_INTR_CLR;
    chg_write(CHG_REG_AC_ON_DET_INTR_CFG, val);

    val = REG_EFUSE_INIT_TIMEOUT_INTR_CLR | REG_EFUSE_INIT_DONE_INTR_CLR;
    chg_write(CHG_REG_EFUSE_INIT_INTR_CFG, val);

    val = REG_CHARGE_TRIPRE_TIMEOUT_INTR_CLR | REG_CHARGE_FAST_TIMEOUT_INTR_CLR;
    chg_write(CHG_REG_FAST_TRI_TIMEOUT_INTR_CFG, val);

    val = REG_CHARGE_TEMP_COLD_INTR_CLR | REG_CHARGE_TEMP_HOT_INTR_CLR | REG_CHARGE_TEMP_COOL_INTR_CLR | REG_CHARGE_TEMP_WARM_INTR_CLR;
    chg_write(CHG_REG_TEMP_STATUS_INTR_CFG, val);

    val = REG_CHARGE_DONE_INTR_CLR;
    chg_write(CHG_REG_CHG_DONE_INTR_CFG, val);

    val = REG_VSYS_SC_INTR_CLR | REG_ACIN_OV_INTR_CLR | REG_ACIN_CL_INTR_CLR | REG_OTP_INTR_CLR;
    chg_write(CHG_REG_OTP_ACIN_VSYS_SC_INTR_CFG, val);

    val = REG_VSYS_LOW_INTR_CLR;
    chg_write(CHG_REG_VSYS_LOW_INTR_CFG, val);

    pmu_get_efuse(PMU_EFUSE_PAGE_RESERVED_F, &val_efuse);
    TRACE(0, "%s charger calibration flag=%d", __func__, !!(val_efuse & PMU_EFUSE_PAGE_F_CHG_CALIB_FLAG));

    charger_charge_icc0_current_calib_get();

    // Improve CV stop current stability.
    chg_read(CHG_REG_CV_CFG_07, &val);
    val= SET_BITFIELD(val, CHARGER_CV_GM_SEL, 0x4);
    val= SET_BITFIELD(val, CHARGER_CV_COMP_RES, 0x0);
    val= SET_BITFIELD(val, CHARGER_CV_COMP_CAP, 0x7);
    chg_write(CHG_REG_CV_CFG_07, val);

    chg_read(CHG_REG_ITERM_POS_DB_CNT_CFG, &val);
    val= SET_BITFIELD(val, REG_ITERM_POS_DB_CNT, 0x1FFF);
    chg_write(CHG_REG_ITERM_POS_DB_CNT_CFG, val);

    chg_read(CHG_REG_VSYS_LOW_DB_CNT_CFG, &val);
    val= SET_BITFIELD(val, REG_VSYS_LOW_POS_DB_CNT, 0x0);
    chg_write(CHG_REG_VSYS_LOW_DB_CNT_CFG, val);

    chg_read(CHG_REG_VSYS_SC_DB_CNT_CFG, &val);
    val= SET_BITFIELD(val, REG_VSYS_SC_POS_DB_CNT, 0x0);
    chg_write(CHG_REG_VSYS_SC_DB_CNT_CFG, val);

    chg_read(CHG_REG_VRECHG_ITERM_CFG, &val);
    val= SET_BITFIELD(val, CHARGER_POWERMOS_N_EDGE_CON, 0x7);
    chg_write(CHG_REG_VRECHG_ITERM_CFG, val);

#ifdef CHG_FORCE_LOW_CV_CURRENT
    charger_charge_rechage_enable(false);
#endif

#ifdef ACIN_PATTERN_DISABLE
    charger_charge_pattern_disable();
#endif

    if (hal_get_chip_metal_id() > HAL_CHIP_METAL_ID_0) {
        chg_read(CHG_REG_CC_PRE_CV_CFG, &val);
        val |= REG_CHARGER_ICC_SEL_DR;
        chg_write(CHG_REG_CC_PRE_CV_CFG, val);
    }

    charger_charge_irq_module_init();
    charger_charge_irq_module_cfg(CHG_AC_ON_DET_IN, CHG_IRQ_MODULE_ENABLE);
    charger_charge_irq_module_cfg(CHG_AC_ON_DET_OUT, CHG_IRQ_MODULE_ENABLE);
    charger_charge_irq_module_cfg(CHG_TRIPRE_TIMEOUT, CHG_IRQ_MODULE_ENABLE);
    charger_charge_irq_module_cfg(CHG_FAST_TIMEOUT, CHG_IRQ_MODULE_ENABLE);
    charger_charge_irq_module_cfg(CHG_TEMP_COLD, CHG_IRQ_MODULE_ENABLE);
    charger_charge_irq_module_cfg(CHG_TEMP_HOT, CHG_IRQ_MODULE_ENABLE);
    charger_charge_irq_module_cfg(CHG_TEMP_COOL, CHG_IRQ_MODULE_ENABLE);
    charger_charge_irq_module_cfg(CHG_TEMP_WARM, CHG_IRQ_MODULE_ENABLE);
    charger_charge_irq_module_cfg(CHG_DONE, CHG_IRQ_MODULE_ENABLE);
    charger_charge_irq_module_cfg(CHG_ACIN_OV, CHG_IRQ_MODULE_ENABLE);
    charger_charge_irq_module_cfg(CHG_ACIN_CL, CHG_IRQ_MODULE_ENABLE);
    charger_charge_irq_module_cfg(CHG_OTP, CHG_IRQ_MODULE_ENABLE);
    charger_charge_irq_module_cfg(CHG_VSYS_LOW, CHG_IRQ_MODULE_ENABLE);

    NVIC_SetVector(CHARGER_IRQn, (uint32_t)charger_charge_irq_handler);
    NVIC_SetPriority(CHARGER_IRQn, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(CHARGER_IRQn);
    NVIC_EnableIRQ(CHARGER_IRQn);
    int_unlock(lock);
}

void charger_charge_enable(void)
{
    uint32_t lock;
    uint16_t val;

    if (chg_pkg_type == CHARGER_PACKAGE_TYPE_SIP_1620) {
        charger_1620_charge_enable();
        return;
    }

    lock = int_lock();
    chg_read(CHG_REG_CHG_FUNC_CFG, &val);
    val |= REG_CHARGER_CHARGE_EN;
    chg_write(CHG_REG_CHG_FUNC_CFG, val);
    int_unlock(lock);
}

void charger_charge_disable(void)
{
    uint32_t lock;
    uint16_t val;

    if (chg_pkg_type == CHARGER_PACKAGE_TYPE_SIP_1620) {
        charger_1620_charge_disable();
        return;
    }

    lock = int_lock();
    chg_read(CHG_REG_CHG_FUNC_CFG, &val);
    val &= ~REG_CHARGER_CHARGE_EN;
    chg_write(CHG_REG_CHG_FUNC_CFG, val);
    int_unlock(lock);
}

void charger_ship_mode_enable(void)
{
    uint32_t lock;

    ASSERT(chg_pkg_type != CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    lock = int_lock();
    if (chg_metal_id == HAL_CHIP_METAL_ID_1) {
        charger_charge_low_vrtc_enable(true);
    } else {
#ifndef CHG_OUT_PWRON
        uint16_t val;

        chg_read(CHG_REG_STANDBY_SHIP_MODE, &val);
        val |= REG_STANDBY_SHIP_MODE;
        for (int i = 0; i < 100; i++) {
            chg_write(CHG_REG_STANDBY_SHIP_MODE, val);
            hal_sys_timer_delay(MS_TO_TICKS(5));
        }
#endif
    }
    int_unlock(lock);
}

int charger_gpadc_vbat_div_ctrl(int enable)
{
    uint32_t lock;
    uint16_t val;

    ASSERT(chg_pkg_type != CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    lock = int_lock();
    chg_read(CHG_REG_VBAT_DIV_CFG, &val);
    if (enable) {
        val |= PU_VBAT_DIV;
    } else {
        val &= ~PU_VBAT_DIV;
    }
    chg_write(CHG_REG_VBAT_DIV_CFG, val);
    int_unlock(lock);

    return 0;
}

enum CHARGER_PACKAGE_TYPE_E BOOT_TEXT_SRAM_LOC charger_package_type_get(void)
{
    return chg_pkg_type;
}

enum CHARGER_ACIN_STATUS_TYPE_E charger_acin_status_get(void)
{
    enum CHARGER_ACIN_STATUS_TYPE_E status = CHARGER_ACIN_STATUS_PLUGOUT;
    uint16_t val;

    if (chg_pkg_type == CHARGER_PACKAGE_TYPE_SIP_1620) {
        charger_1620_acin_status_get();
    } else {
        chg_read(CHG_REG_CHG_STATUS1, &val);
        if (!!(val & REG_CHG_AC_ON_DB)) {
            // db_time=16ms
            status = CHARGER_ACIN_STATUS_PLUGIN;
        }
    }

    return status;
}

void BOOT_TEXT_FLASH_LOC charger_read_hw_metal_id(void)
{
    uint16_t val_efuse;

    chg_metal_id = hal_cmu_get_aon_revision_id();

    if (chg_metal_id >= HAL_CHIP_METAL_ID_1) {
        chg_pkg_type = CHARGER_PACKAGE_TYPE_METAL_V1;
    } else {
        pmu_get_efuse(PMU_EFUSE_PAGE_RESERVED_C, &val_efuse);
        val_efuse = GET_BITFIELD(val_efuse, PMU_EFUSE_PAGE_C_CHG_PKG_TYPE);
        if (val_efuse == 0x0) {
            chg_pkg_type = CHARGER_PACKAGE_TYPE_METAL_V0;
        } else if (val_efuse == 0x1) {
            chg_pkg_type = CHARGER_PACKAGE_TYPE_SPLIT_ACIN_UARTCOM;
        } else if (val_efuse == 0x2) {
            chg_pkg_type = CHARGER_PACKAGE_TYPE_SIP_1620;
        }
    }
}

void BOOT_TEXT_FLASH_LOC charger_boot_init(void)
{
    uint16_t val;

#ifdef CHG_FORCE_LP_PWROFF
    if (hal_get_chip_metal_id() > HAL_CHIP_METAL_ID_0) {
        chg_read(CHG_REG_TEMP_BG_CFG, &val);
        val |= REG_BG_CORE_EN;
        chg_write(CHG_REG_TEMP_BG_CFG, val);

        val |= REG_BG_REF_GEN_EN;
        chg_write(CHG_REG_TEMP_BG_CFG, val);

        hal_sys_timer_delay(MS_TO_TICKS(1));

        val |= REG_BG_EN_OTP;
        chg_write(CHG_REG_TEMP_BG_CFG, val);

        chg_read(CHG_REG_02, &val);
        val &= ~ACIN_PP_LDO_LP_EN;
        chg_write(CHG_REG_02, val);
    }
#endif

    // Resume VRTC at first.
    charger_charge_low_vrtc_enable(false);

    // Reset Charger
    if (hal_get_chip_metal_id() > HAL_CHIP_METAL_ID_0) {
        chg_write(CHG_REG_METAL_ID, 0xCAFE);
        chg_write(CHG_REG_METAL_ID, 0x5FEE);
        hal_sys_timer_delay(US_TO_TICKS(500));

        // ramp for CC satrtup
        chg_read(CHG_REG_CC_PRE_CV_CFG, &val);
        val &= ~REG_CHARGER_ICC_SEL_DR;
        chg_write(CHG_REG_CC_PRE_CV_CFG, val);
    }

    if (chg_pkg_type == CHARGER_PACKAGE_TYPE_METAL_V0 || chg_pkg_type == CHARGER_PACKAGE_TYPE_SPLIT_ACIN_UARTCOM) {
        val = REG_EFUSE_INIT_DONE | REG_EFUSE_INIT_DONE_DR | REG_EFUSE_INIT_DONE_TRIGGER_EN;
        chg_write(CHG_REG_EFUSE_INIT_DONE_CFG, val);
    }
}

void charger_charge_half_icc_current_enable(void)
{
    uint32_t lock;
    uint16_t val;

    ASSERT(chg_pkg_type != CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    if (chg_metal_id > HAL_CHIP_METAL_ID_0
#ifdef CHG_FORCE_LOW_CV_CURRENT
        && !chg_half_icc_busy
#endif
        )
    {
        lock = int_lock();
        chg_read(CHG_REG_ICC_HALF_INTF_PMU, &val);
        val |= REG_NTC_HALF_ICC;
        val |= REG_NTC_HALF_ICC_DR;
        chg_write(CHG_REG_ICC_HALF_INTF_PMU, val);
        int_unlock(lock);
    }
}

void charger_charge_half_icc_current_disable(void)
{
    uint32_t lock;
    uint16_t val;

    ASSERT(chg_pkg_type != CHARGER_PACKAGE_TYPE_SIP_1620, "%s Invalid charger package type!", __func__);

    if (chg_metal_id > HAL_CHIP_METAL_ID_0
#ifdef CHG_FORCE_LOW_CV_CURRENT
        && !chg_half_icc_busy
#endif
        )
    {
        lock = int_lock();
        chg_read(CHG_REG_ICC_HALF_INTF_PMU, &val);
        val |= REG_NTC_HALF_ICC;
        val &= ~REG_NTC_HALF_ICC_DR;
        chg_write(CHG_REG_ICC_HALF_INTF_PMU, val);
        int_unlock(lock);
    }
}
