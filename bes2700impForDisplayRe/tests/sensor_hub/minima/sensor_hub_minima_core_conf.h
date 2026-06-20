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
#ifndef __SENSOR_HUB_MINIMA_CORE_CONF_H__
#define __SENSOR_HUB_MINIMA_CORE_CONF_H__
#include "hal_sysfreq.h"
#include "hal_cmu.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t user;
    uint8_t freq;
    uint8_t volt_op; //voltage operation point
} minima_freq_volt_op_list_t;

#define LAST_VOLT_OP_INI_VAL IDLE
#define CUR_VOLT_OP_INI_VAL  IDLE

static minima_freq_volt_op_list_t freq_volt_op_list[] = {
    {HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_6P5M, DVFS_OP_FREQ_6P5M},
    {HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_13M,  DVFS_OP_FREQ_13M},
    {HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_26M,  DVFS_OP_FREQ_26M},
    {HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_52M,  DVFS_OP_FREQ_52M},
    {HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_78M,  DVFS_OP_FREQ_78M},
    {HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_104M, DVFS_OP_FREQ_104M},
};

#ifdef __cplusplus
}
#endif

#endif /* __SENSOR_HUB_MINIMA_CONFIG_H__ */
