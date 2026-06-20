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
#ifndef __SENSOR_HUB_VAD_ADAPTER_TC_H__
#define __SENSOR_HUB_VAD_ADAPTER_TC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "sens_msg.h"

/*
 * This source file is used to implement some test cases for VAD Adapter application.
 * And these cases maybe used to test VAD's power consumption or others.
 * There is a command handler app_vad_adapter_test_handler() which is used to handle
 * all tests command comes from the core layer.
 */

uint32_t vad_adpt_test_cap_stream_handler(uint8_t *buf, uint32_t len);

int app_vad_adapter_test_handler(enum APP_VAD_ADPT_ID_T id, uint32_t scmd, uint32_t arg1, uint32_t arg2);

#ifdef __cplusplus
}
#endif

#endif
