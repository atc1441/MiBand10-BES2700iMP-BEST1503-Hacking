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
#ifndef __SENSOR_HUB_CORE_WORKLOAD_H__
#define __SENSOR_HUB_CORE_WORKLOAD_H__

#ifdef __cplusplus
extern "C" {
#endif

enum WORKLOAD_ID_T {
    WORKLOAD_ID_0,
    WORKLOAD_ID_1,
    WORKLOAD_ID_2,
    WORKLOAD_ID_3,

    WORKLOAD_ID_QTY,
};

typedef void (*WORKLOAD_FUNCTION_T)(void *param);

void sensor_hub_init_workload_thread(void);
void sensor_hub_enter_full_workload_mode(void);
void sensor_hub_exit_full_workload_mode(void);
int sensor_hub_add_workload(enum WORKLOAD_ID_T id, WORKLOAD_FUNCTION_T func, void *param);
int sensor_hub_remove_workload(enum WORKLOAD_ID_T id);
void sensor_hub_enable_workload(enum WORKLOAD_ID_T id);
void sensor_hub_disable_workload(enum WORKLOAD_ID_T id);

#ifdef __cplusplus
}
#endif

#endif

