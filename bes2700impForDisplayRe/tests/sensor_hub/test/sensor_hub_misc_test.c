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
#ifdef SENSOR_HUB_MISC_TEST
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include <string.h>
#include "hal_trace.h"
#include "hal_timer.h"
#include "sensor_hub_core.h"

POSSIBLY_UNUSED uint32_t core_busy_cnt = 0;
POSSIBLY_UNUSED void core_busy_func1(void *param)
{
    uint32_t *pcnt = (uint32_t *)param;

    *pcnt = *pcnt + 1;
    TRACE(1, "%s: cnt=%d", __func__, *pcnt);
    osDelay(100);
}

POSSIBLY_UNUSED void core_busy_func2(void *param)
{
    uint32_t *pcnt = (uint32_t *)param;

    TRACE(1, "%s: cnt=%d", __func__, *pcnt);
    osDelay(100);

    if (*pcnt > 100) {
        // stop the workload and remove it
        TRACE(1, "%s: cnt=%d, will stopped", __func__, *pcnt);
        sensor_hub_disable_workload(WORKLOAD_ID_0);
        sensor_hub_remove_workload(WORKLOAD_ID_0);
        sensor_hub_disable_workload(WORKLOAD_ID_1);
        sensor_hub_remove_workload(WORKLOAD_ID_1);
    }
}

POSSIBLY_UNUSED void sensor_hub_core_workload_test(void)
{
    TRACE(1, "%s", __func__);
    core_busy_cnt = 0;

    // add new workload and enable it
    sensor_hub_add_workload(WORKLOAD_ID_0, core_busy_func1, (void *)&core_busy_cnt);
    sensor_hub_enable_workload(WORKLOAD_ID_0);
    sensor_hub_add_workload(WORKLOAD_ID_1, core_busy_func2, (void *)&core_busy_cnt);
    sensor_hub_enable_workload(WORKLOAD_ID_1);
}

void sensor_hub_misc_test(void)
{
    sensor_hub_core_workload_test();
}
#endif
