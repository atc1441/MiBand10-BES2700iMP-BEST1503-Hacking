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
#include "cmsis.h"
#include "hal_sleep.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_psram.h"
//#include "hwtimer_list.h"
#include "stdlib.h"
#include "hal_sysfreq.h"

#ifdef RTOS
#include "cmsis_os.h"

static osThreadId main_thread_tid = NULL;


extern int gpu_test(void);

static void gpu_test_thread(void const *argument)
{
    gpu_test();
    while(1){

        TRACE(3,"gpu test thread\n");
        osDelay(1000);
    }
}

osThreadDef(gpu_test_thread, osPriorityHigh, 1, 4012, "gpu_test_thread");

static int start_gpu_test(void)
{
    osThreadId thread_tid;
    thread_tid = osThreadCreate(osThread(gpu_test_thread), NULL);

    if (thread_tid == NULL)  {
        TRACE(0,"Failed to Create gpu test thread\n");
        return 0;
    }
    return 0;
}

extern int fb_lcdc_test(void);

static void lcdc_test_thread(void const *argument)
{
    fb_lcdc_test();
    while(1){
        TRACE(3,"lcdc test thread\n");
        osDelay(1000);
    }
}

osThreadDef(lcdc_test_thread, osPriorityHigh, 1, 4012, "lcdc_test_thread");

static int start_lcdc_test(void)
{
    osThreadId thread_tid;
    thread_tid = osThreadCreate(osThread(lcdc_test_thread), NULL);

    if (thread_tid == NULL)  {
        TRACE(0,"Failed to Create lcdc test thread\n");
        return 0;
    }
    return 0;
}


extern  int fb_dma2d_test(void);

static void dma2d_test_thread(void const *argument)
{
    fb_dma2d_test();
    while(1){
        TRACE(3,"dma2d test thread\n");
        osDelay(2000);
    }
}

osThreadDef(dma2d_test_thread, osPriorityHigh, 1, 4012, "dma2d_test_thread");

static int start_dma2d_test(void)
{
    osThreadId thread_tid;
    thread_tid = osThreadCreate(osThread(dma2d_test_thread), NULL);

    if (thread_tid == NULL)  {
        TRACE(0,"Failed to Create dma2d test thread\n");
        return 0;
    }
    return 0;
}

static int gra_task_started = 0;
int gra_test_main(void)
{
    if (gra_task_started) return 0;
    gra_task_started = 1;

    main_thread_tid = osThreadGetId();
    hal_psram_init();
  //  hal_sysfreq_req(HAL_SYSFREQ_USER_APP_8, HAL_CMU_FREQ_104M);
    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_8, HAL_CMU_FREQ_78M);

    //start_gpu_test();
    start_lcdc_test();
    osDelay(100);
    start_dma2d_test();
    while(1){
        TRACE(3, "%s %d ", __func__, __LINE__);
        osDelay(1000);
    }

    return 0;
}

#endif
