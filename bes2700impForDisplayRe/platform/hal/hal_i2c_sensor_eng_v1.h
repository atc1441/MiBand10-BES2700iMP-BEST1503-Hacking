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
#ifndef __HAL_I2C_SENSOR_ENG_V1_H__
#define __HAL_I2C_SENSOR_ENG_V1_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_i2c.h"
#include "hal_gpio.h"
#include "hal_sensor_eng_v1.h"

typedef void (*HAL_I2C_SENSOR_ENG_HANDLER_V1_T)(enum HAL_I2C_ID_T id, const uint8_t *buf, uint32_t len);

struct HAL_I2C_SENSOR_ENGINE_CONFIG_V1_T {
    enum HAL_SENSOR_ENGINE_ID_T id;
    enum HAL_SENSOR_ENGINE_TRIGGER_T trigger_type;
    enum HAL_GPIO_PIN_T trigger_gpio;
    uint32_t period_us;
    uint16_t target_addr;
    uint8_t *write_buf;
    uint16_t write_txn_len;
    uint8_t *read_buf;
    uint16_t read_txn_len;
    uint16_t txn_cnt;
    uint8_t read_burst_cnt;
    HAL_I2C_SENSOR_ENG_HANDLER_V1_T handler;
};

uint32_t hal_i2c_sensor_engine_start(enum HAL_I2C_ID_T id, const struct HAL_I2C_SENSOR_ENGINE_CONFIG_V1_T *cfg);
uint32_t hal_i2c_sensor_engine_stop(enum HAL_I2C_ID_T id);

#ifdef __cplusplus
}
#endif

#endif
