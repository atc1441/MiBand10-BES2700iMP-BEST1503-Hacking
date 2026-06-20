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
#include "cmsis.h"
#include "hal_trace.h"
#include "main_entry.h"
#include "mpu_cfg.h"
#include <stdio.h>

#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "simu_test.h"


#ifdef RTOS
uint32_t OS_Tick_GetCount (void);
void SystemCoreClockUpdate();

void app_main (void *argument)
{
    printf(" task %s\n", __func__);
    simu_os_test();
}

static POSSIBLY_UNUSED void simu_start_os(void)
{
    SystemCoreClockUpdate();
    // ...
    osKernelInitialize();                 // Initialize CMSIS-RTOS
    osThreadId_t app_main_id = osThreadNew(app_main, NULL, NULL);    // Create application main thread

    if (osKernelGetState() == osKernelReady) {
        osKernelStart();                    // Start thread execution
    }
    printf(" task sleep %d\n", OS_Tick_GetCount());
    if (!app_main_id)
        printf(" task create wrong\n");
    else
        printf(" task %p sleep\n", app_main_id);

    osDelay(10000);
    printf(" task sleep ok\n");
    /* printf(" task sleep ok %d\n", OS_Tick_GetCount()); */
}
#endif


int MAIN_ENTRY(void)
{
    simu_semihost_test();

#ifdef RTOS
    simu_start_os();
#endif
    while (1) {
        uint32_t cnt = osKernelGetTickCount();
        printf("i am main? %d\n", cnt);
        osDelay(10000);
        cnt = osKernelGetTickCount();
        printf("now %d\n", cnt);
    }

    SAFE_PROGRAM_STOP();
    return 0;
}

