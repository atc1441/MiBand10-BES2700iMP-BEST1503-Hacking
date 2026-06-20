
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

#ifndef _GPU_PORT_H
#define _GPU_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

//#include "plat_types.h"

#ifdef GRAPHICDRV_TIME_STATIS
#include "hal_timer.h"
#include "hal_trace.h"


#define _TEST_REP(...)  TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)

#define TC_INIT  volatile uint32_t time_s; \
                 volatile uint32_t time_e;
#define TC_START time_s = hal_fast_sys_timer_get();
#define TC_END   time_e = hal_fast_sys_timer_get();

#define _TSTR(s) #s

#define TC_REP(s) do{ \
                      _TEST_REP(_TSTR(s)":%d us", FAST_TICKS_TO_US(time_e - time_s)); \
                    }while(0)

#else

#define TC_INIT
#define TC_START
#define TC_END
#define TC_REP(s)

#endif

/**
 * gpu_init - main function to setup gpu
 *
 */
void gpu_init(void);

/**
 * gpu_memory_setup - setup gpu subsys memory requirement
 *
 */
void gpu_memory_setup(void);

/**
 * gpu_malloc - memory allocat method interface
 * @size : size required
 */
void *gpu_malloc(uint32_t size);


/**
 * gpu_free - memory allocat method interface
 * @mem :  pointer to memory allocated by gpu_malloc
 */
void gpu_free(void *mem);

/**
 * gpu_hw_reset - reset gpu hardware
 *
 */
void gpu_hw_reset(void);


/**
 * vg_lite_hal_get_intr_status - return gpu irq status
 *
 */
int vg_lite_hal_get_intr_status(void);

/**
 * vg_lite_hal_clear_intr_status - clear gpu irq status
 *
 */
void vg_lite_hal_clear_intr_status(void);


/*
 * set_gpu_done_interrupt_handler - setup gpu irq handler
 *
 */
void set_gpu_done_interrupt_handler(void);

/**
 * cpu_cache_flush - flush cpu cache table
 *
 * called before invalid gpu cache
 *
 */
void cpu_cache_flush(uint32_t start, uint32_t length);

/**
 * gpu_cache_invalid - invalid gpu cache table
 *
 * called  when start GPU to process cmd buffer
 *
 */
void gpu_cache_invalid(void);

/**
 * cpu_gpu_data_cache_invalid - invalid cpu cache table
 *
 * called after GPU process done
 *
 */
void cpu_gpu_data_cache_invalid(uint32_t start, uint32_t length);

void gpu_wait_done(void);

void gpu_post_done(void);

#ifdef __cplusplus
}
#endif

#endif //_GPU_PORT_H
