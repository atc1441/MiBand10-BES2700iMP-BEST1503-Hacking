/***************************************************************************
 *
 * Copyright 2022-2023 BES.
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
#ifndef CHIP_SUBSYS_SENS
#include "capsensor_driver_best1306.h"
#include "analog.h"
#include "touch_wear_core.h"
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_capsensor_best1306.h"
#include "tgt_hardware_capsensor.h"
#ifdef CAPSENSOR_IZC_PACKAGE
#include "hal_gpio.h"
#include "hal_chipid.h"
#endif
#include "hal_trace.h"

void capsensor_driver_init(void)
{
    analog_capsensor_init();

    cap_sensor_core_thread_init();
}

uint8_t capsensor_ch_num_get(void)
{
    return CAP_CHNUM;
}

uint8_t capsensor_conversion_num_get(void)
{
    return CAP_REPNUM;
}

uint16_t capsensor_samp_fs_get(void)
{
    return CAP_SAMP_FS;
}

uint8_t capsensor_ch_map_get(void)
{
    return (CAP_CH0 | CAP_CH1 | CAP_CH2 | CAP_CH3 | CAP_CH4 | CAP_CH5 | CAP_CH6);
}

void capsensor_gpio_init(void)
{
#ifdef CAPSENSOR_IZC_PACKAGE
    uint8_t cap_ch_map = capsensor_ch_map_get();

    if(cap_ch_map & CAP_CH3) {
        hal_iomux_set_analog_io(HAL_IOMUX_PIN_P0_0);
    }

    if(cap_ch_map & CAP_CH4) {
        if(hal_get_chip_metal_id() >= HAL_CHIP_METAL_ID_2) {
            hal_iomux_set_analog_io(HAL_IOMUX_PIN_P0_4);
        } else {
            const struct HAL_IOMUX_PIN_FUNCTION_MAP iomux_tport[] = {
                {HAL_IOMUX_PIN_P0_4, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL}
            };
            hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&iomux_tport[0], 1);
        };
    }

    if(cap_ch_map & CAP_CH5) {
        hal_iomux_set_analog_io(HAL_IOMUX_PIN_P0_1);
    }
#endif
}

/***************************************************************************
 * @brief suspend capsensor functions.
 *
 ***************************************************************************/
void capsensor_suspend(void)
{
    uint32_t lock;

    lock = int_lock();
    hal_capsensor_fp_mode_set_mask();

    hal_cmu_capsensor_suspend();
    hal_capsensor_suspend_pu_osc_dr();
    int_unlock(lock);
}

/***************************************************************************
 * @brief start capsensor functions.
 *
 ***************************************************************************/
void capsensor_start(void)
{
    uint32_t lock;

    lock = int_lock();
    hal_cmu_capsensor_start();
    hal_capsensor_start_pu_osc_reg();

    hal_capsensor_fp_mode_clear_irq();
    hal_capsensor_fp_mode_clr_mask();
    int_unlock(lock);
}

void capsensor_close(void)
{
    hal_cmu_capsensor_close();
}

void capsensor_open(void)
{
    hal_cmu_capsensor_open();

    analog_capsensor_init();
    capsensor_drv_start();
}

int capsensor_reopen(void)
{
    int ret = 0;

    analog_capsensor_init();
    capsensor_drv_start();
    osDelay(10);
    ret = hal_capsensor_judge_machine_state();
    TRACE(0, "capsensor_reopen:%d, !!!", ret);
    return ret;
}

void capsensor_judge_machine_state(void)
{
    int ret1 = 0;
    int ret2 = 0;

    osDelay(10);
    ret1 = hal_capsensor_judge_machine_state();
    if (ret1) {
        TRACE(0, "capsensor_machine_state_abnormal:%d, need reopen!!!", ret1);

        ret2 = capsensor_reopen();
        if (ret2) {
            TRACE(0, "hal_capsensor_reopen failed:%d", ret2);
            return;
        } else {
            TRACE(0, "hal_capsensor_reopen successed:%d", ret2);
        }
    }
}

void capsensor_set_sdm_init_flag(bool m_sdm_init_flag)
{
    hal_capsensor_set_sdm_init_flag(m_sdm_init_flag);
}

void capsensor_baseline_sar_dr(uint32_t* sar_value, uint32_t* baseline_value_p, uint32_t* baseline_value_n)
{
    hal_capsensor_baseline_sar_dr(sar_value, baseline_value_p, baseline_value_n);
}

void capsensor_baseline_sar_reg_read(uint32_t* sar_value, uint32_t* baseline_value_p, uint32_t* baseline_value_n)
{
    hal_capsensor_baseline_sar_reg_read(sar_value, baseline_value_p, baseline_value_n);
}

#endif
