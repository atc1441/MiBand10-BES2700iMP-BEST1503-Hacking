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
#ifndef __HAL_SENSOR_ENG_V2_H__
#define __HAL_SENSOR_ENG_V2_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_gpio.h"

enum HAL_SENSOR_ENGINE_MASTER_T {
    HAL_SENSOR_ENGINE_MASTER_0 = 0,
    HAL_SENSOR_ENGINE_MASTER_1,
    HAL_SENSOR_ENGINE_MASTER_2,
    HAL_SENSOR_ENGINE_MASTER_3,

    HAL_SENSOR_ENGINE_MASTER_QTY,
};

enum HAL_SENSOR_ENGINE_ID_T {
    HAL_SENSOR_ENGINE_ID_0 = 0,
    HAL_SENSOR_ENGINE_ID_1,
    HAL_SENSOR_ENGINE_ID_2,
    HAL_SENSOR_ENGINE_ID_3,
    HAL_SENSOR_ENGINE_ID_4,
    HAL_SENSOR_ENGINE_ID_5,
    HAL_SENSOR_ENGINE_ID_6,
    HAL_SENSOR_ENGINE_ID_7,

    HAL_SENSOR_ENGINE_ID_QTY,
};

enum HAL_SENSOR_ENGINE_TRIGGER_T {
    HAL_SENSOR_ENGINE_TRIGGER_GPIO = 0,
    HAL_SENSOR_ENGINE_TRIGGER_TIMER,

    HAL_SENSOR_ENGINE_TRIGGER_QTY,
};

enum HAL_SENSOR_ENGINE_DEVICE_T {
    HAL_SENSOR_ENGINE_DEVICE_I2C0 = 0,
    HAL_SENSOR_ENGINE_DEVICE_I2C1 = 1,
    HAL_SENSOR_ENGINE_DEVICE_I2C2 = 2,
    HAL_SENSOR_ENGINE_DEVICE_I2C3 = 3,
    HAL_SENSOR_ENGINE_DEVICE_I2C4 = 4,
    HAL_SENSOR_ENGINE_DEVICE_SPI  = 7,

    HAL_SENSOR_ENGINE_DEVICE_QTY,
};

enum HAL_SENSOR_ENGINE_DATA_WIDTH_T {
    HAL_SENSOR_ENGINE_DATA_BYTE = 0,
    HAL_SENSOR_ENGINE_DATA_HALF_WORD,
    HAL_SENSOR_ENGINE_DATA_WORD,

    HAL_SENSOR_ENGINE_DATA_WORD_QTY,
};

enum HAL_SENSOR_ENGINE_SPI_CS_T {
    HAL_SENSOR_ENGINE_SPI_CS00 = 0,
    HAL_SENSOR_ENGINE_SPI_CS01,
    HAL_SENSOR_ENGINE_SPI_CS02,
    HAL_SENSOR_ENGINE_SPI_CS03,
    HAL_SENSOR_ENGINE_SPI_CS04,
    HAL_SENSOR_ENGINE_SPI_CS05,
    HAL_SENSOR_ENGINE_SPI_CS06,
    HAL_SENSOR_ENGINE_SPI_CS07,
    HAL_SENSOR_ENGINE_SPI_CS08,
    HAL_SENSOR_ENGINE_SPI_CS09,
    HAL_SENSOR_ENGINE_SPI_CS10,
    HAL_SENSOR_ENGINE_SPI_CS11,
    HAL_SENSOR_ENGINE_SPI_CS12,
    HAL_SENSOR_ENGINE_SPI_CS13,
    HAL_SENSOR_ENGINE_SPI_CS14,
    HAL_SENSOR_ENGINE_SPI_CS15,

    HAL_SENSOR_ENGINE_SPI_CS_QTY,
};

enum HAL_SENSOR_ENGINE_SPI_RXDS_T {
    HAL_SENSOR_ENGINE_SPI_RXDS_0 = 0,
    HAL_SENSOR_ENGINE_SPI_RXDS_1,

    HAL_SENSOR_ENGINE_SPI_RXDS_QTY,
};

enum HAL_SENSOR_ENGINE_READ_WRITE_T {
    HAL_SENSOR_ENGINE_READ = 0,
    HAL_SENSOR_ENGINE_WRITE,

    HAL_SENSOR_ENGINE_READ_WRITE_QTY,
};

enum HAL_SENSOR_ENGINE_CODEC_SEL_T {
    HAL_SENSOR_ENGINE_CODEC_SEL_SENSOR0 = 0,
    HAL_SENSOR_ENGINE_CODEC_SEL_SENSOR1,
    HAL_SENSOR_ENGINE_CODEC_SEL_SENSOR2,
    HAL_SENSOR_ENGINE_CODEC_SEL_SENSOR3,
    HAL_SENSOR_ENGINE_CODEC_SEL_SENSOR4,
    HAL_SENSOR_ENGINE_CODEC_SEL_SENSOR5,
    HAL_SENSOR_ENGINE_CODEC_SEL_SENSOR6,
    HAL_SENSOR_ENGINE_CODEC_SEL_SENSOR7,

    HAL_SENSOR_ENGINE_CODEC_SEL_QTY,
};

enum HAL_SENSOR_ENGINE_STATUS_T {
    HAL_SENSOR_ENGINE_IDLE      = 1 << 0,
    HAL_SENSOR_ENGINE_WAKEUP_S0 = 1 << 1,
    HAL_SENSOR_ENGINE_WAKEUP_S1 = 1 << 2,
    HAL_SENSOR_ENGINE_WAKEUP_S2 = 1 << 3,
    HAL_SENSOR_ENGINE_WAKEUP_S3 = 1 << 4,
    HAL_SENSOR_ENGINE_WAKEUP_S4 = 1 << 5,
    HAL_SENSOR_ENGINE_WAKEUP_S5 = 1 << 6,
    HAL_SENSOR_ENGINE_WAKEUP_S6 = 1 << 7,
    HAL_SENSOR_ENGINE_WAKEUP_S7 = 1 << 8,
};

typedef void (*HAL_SENSOR_ENG_HANDLER_T)(enum HAL_SENSOR_ENGINE_MASTER_T master,
        enum HAL_SENSOR_ENGINE_ID_T id, enum HAL_SENSOR_ENGINE_DEVICE_T device, const uint8_t *buf,
        uint32_t len, enum HAL_SENSOR_ENGINE_READ_WRITE_T read_write);

struct HAL_SENSOR_ENGINE_CFG_T {
    enum HAL_SENSOR_ENGINE_ID_T id;                     //Common variable
    enum HAL_SENSOR_ENGINE_DEVICE_T device;             //Common variable
    enum HAL_SENSOR_ENGINE_TRIGGER_T trigger_type;      //Common variable
    enum HAL_GPIO_PIN_T trigger_gpio;                   //only gpio trigger use
    uint32_t timer_period_ms;                           //only timer trigger use
    uint8_t interrupt_count: 4;                         //Common variable
    enum HAL_SENSOR_ENGINE_CODEC_SEL_T data_to_vad;     //Common variable

    uint8_t i2c_device_address;                         //only i2c device use
    enum HAL_SENSOR_ENGINE_SPI_CS_T spi_device_cs;      //only spi device use
    enum HAL_SENSOR_ENGINE_SPI_RXDS_T spi_device_rxds;  //only spi device use
    uint8_t spi_id;                                     //only spi device use
    enum HAL_SENSOR_ENGINE_READ_WRITE_T read_write;     //Common variable
    enum HAL_SENSOR_ENGINE_DATA_WIDTH_T tx_data_width;  //Common variable
    enum HAL_SENSOR_ENGINE_DATA_WIDTH_T rx_data_width;  //Common variable
    void *tx_buffer_addr;                               //Common variable
    void *rx_buffer_addr;                               //Common variable
    uint32_t tx_buffer_size;                            //Common variable
    uint32_t rx_buffer_size;                            //Common variable
    uint32_t tx_transfer_size;                          //Common variable
    uint32_t rx_transfer_size;                          //Common variable
    HAL_SENSOR_ENG_HANDLER_T handler;                   //Common variable
};

int hal_sensor_engine_cfg(enum HAL_SENSOR_ENGINE_MASTER_T master, const struct HAL_SENSOR_ENGINE_CFG_T *cfg);
int hal_sensor_engine_open(enum HAL_SENSOR_ENGINE_MASTER_T master);
int hal_sensor_engine_close(enum HAL_SENSOR_ENGINE_MASTER_T master);
int hal_sensor_engine_open_sensor(enum HAL_SENSOR_ENGINE_MASTER_T master, enum HAL_SENSOR_ENGINE_ID_T id);
int hal_sensor_engine_close_sensor(enum HAL_SENSOR_ENGINE_MASTER_T master, enum HAL_SENSOR_ENGINE_ID_T id);
int hal_sensor_engine_enable(enum HAL_SENSOR_ENGINE_MASTER_T master);
int hal_sensor_engine_disable(enum HAL_SENSOR_ENGINE_MASTER_T master);

#ifdef __cplusplus
}
#endif

#endif

