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

#ifndef __HAL_CAPSENSOR_BEST1306_H__
#define __HAL_CAPSENSOR_BEST1306_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "capsensor_driver.h"

void capsensor_setup_cfg(struct CAPSENSOR_CFG_T * cap_cfg);
void hal_capsensor_irq_enable(void);
void hal_capsensor_irq_disable(void);

int capsensor_get_raw_data(struct capsensor_sample_data *sample, int num);
void hal_capsensor_fp_mode_clear_irq(void);
void hal_capsensor_suspend_pu_osc_dr(void);
void hal_capsensor_start_pu_osc_reg(void);
void hal_capsensor_fp_mode_set_mask(void);
void hal_capsensor_fp_mode_clr_mask(void);
void hal_capsensor_set_sdm_init_flag(bool m_sdm_init_flag);
void hal_capsensor_sw_control_sar_en(void);
void hal_capsensor_baseline_sar_reg_read(uint32_t* sar_value, uint32_t* baseline_value_p, uint32_t* baseline_value_n);
void hal_capsensor_baseline_sar_dr(uint32_t* sar_value, uint32_t* baseline_value_p, uint32_t* baseline_value_n);
int hal_capsensor_judge_machine_state(void);

#ifdef __cplusplus
}
#endif

#endif /*__HAL_CAPSENSOR_BEST1306_H__*/
