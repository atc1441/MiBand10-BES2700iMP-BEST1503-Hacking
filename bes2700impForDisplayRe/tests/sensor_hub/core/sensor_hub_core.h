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
#ifndef __SENSOR_HUB_CORE_H__
#define __SENSOR_HUB_CORE_H__
#ifdef FULL_WORKLOAD_MODE_ENABLED
#include "sensor_hub_core_workload.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define SENSOR_HUB_BUFFER_LOCATION __attribute__((used, section(".buffer_section")))
#if (SENS_FIR_LMS_SIZE > 0)
#define FIR_LMS_TEXT_LOCATION __attribute__((used, section(".fir_lms_text_section")))
#define FIR_LMS_DATA_LOCATION __attribute__((used, section(".fir_lms_data_section")))
#define FIR_LMS_BSS_LOCATION __attribute__((used, section(".fir_lms_bss_section")))
#else
#define FIR_LMS_TEXT_LOCATION
#define FIR_LMS_DATA_LOCATION
#define FIR_LMS_BSS_LOCATION
#endif

typedef unsigned int (*sensor_hub_core_rx_irq_handler_t)(const void*, unsigned int);
typedef void (*sensor_hub_core_tx_done_irq_handler_t)(const void*, unsigned int);

void sensor_hub_core_register_rx_irq_handler(sensor_hub_core_rx_irq_handler_t irqHandler);
void sensor_hub_core_register_tx_done_irq_handler(sensor_hub_core_tx_done_irq_handler_t irqHandler);

#ifdef __cplusplus
}
#endif

#endif

