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

/**
 * DOC: gpu driver porting layer
 *
 * should adjust this impliment by product host platform
 *
 */


#include <stdio.h>
#include <string.h>
#include "cmsis_nvic.h"
#include "hal_uart.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_psc.h"
#include "hal_cache.h"
#include "hal_psram.h"
#include "hal_location.h"
#include "heap_api.h"
#include "gpu_port.h"
#include "gpu_conf.h"
#include "vg_lite.h"
#include "vg_lite_kernel.h"
#include "vg_lite_hw.h"
#include "vg_lite_hal.h"
#include "vg_lite_platform.h"
#include "hal_sysfreq.h"

#ifdef RTOS
//#define WITH_OS
#endif


#ifdef WITH_OS
#include "semaphore.h"
#endif

//#define SIM_GPU

#ifndef ALIGN_UP
#  define ALIGN_UP(num, align) (((num) + ((align) - 1)) & ~((align) - 1))
#endif

#ifdef SIM_GPU
#define LCDC_INFO(...)   //TR_INFO( TR_MOD(TEST), "//"  __VA_ARGS__)
#define TRACE(...)
#else
#define LCDC_INFO(...)  TR_INFO( TR_MOD(TEST), "//"  __VA_ARGS__)
#endif
/**
 * GPU cache devices id
 *
 */

enum {
   PECRD,   /* Pixel Engine Color Read (frame buffer read) */
   PECWR,   /* Pixel Engine Color Write (frame buffer write) */
   TSRD,    /* Tessellation Read */
   TSWR,    /* Tessellation Write */
   RESV0,   /* Reserved */
   FERD,    /* Front End (DMA) Read */
   VGRD,    /* Vector Graphics Read */
   PTRD,    /* Page Table (MMU) Read */

   RESV1,   /* Reserved */
   RESV2,   /* Reserved */
   IMGRD,   /* Imager Read */
};


/**
 * GPU cache control Register
 *
 */
#define GPU_CACHE_CNFSELCR  0x268
#define GPU_CACHE_DEVICEWRMODR  0x26c //deviceid 0 - 7
#define GPU_CACHE_DEVICEWRMOD01R  0x270 //deviceid 8 - 15
#define GPU_CACHE_DEVICERDMODR  0x278 //deviceid 0 - 7
#define GPU_CACHE_DEVICERDMOD01R  0x27c //deviceid 0 - 15


#define L2CC_CACHE_ENA    (1 << 0)
#define L2CC_CACHE_ENACR  0x100
#define L2CC_CACHE_SYNCCR 0x730
#define L2CC_CACHE_PREFETCHCR 0xf60 
#define L2CC_CACHE_PREFETCH_DBLF_ENA    (1 << 30)
#define L2CC_CACHE_PREFETCH_INCDBLF_ENA    (1 << 23)
#define L2CC_CACHE_INVALIDCR 0x77c

/**
 * GPU device write cache attributes(WR/RA/C/B)
 *
 */
#define  DEVICE_WRCATTR  0x06  //Outer Write-through, no allocate on write

/**
 * GPU cache device read mode
 *
 */
#define  DEVICE_RDCATTR  0x03  //Outer no allocate


#define  GPU_DEVICE_CATTR(attr, id)  ((uint32_t)attr  << (id * 4))

#define GPU_DONE_IRQn     GPU_IRQn

#define GPU_HEAP_SIZE    (16 << 10)


#ifdef GPU_USE_PSRAM
#define GPU_CONTIGUOUS_SIZE       (1024*1024)
#else
#define GPU_CONTIGUOUS_SIZE       (1024*1024*3)
#endif

/**
 * Private data
 *
 */
static heap_handle_t heap;
static uint32_t gpu_contiguous_size = 0;

static uint32_t gpu_heap_mem[GPU_HEAP_SIZE/4];

//static heap_handle_t heap;

#if _BAREMETAL
static volatile int intr_status = 0;

#ifdef WITH_OS
static sem_t  gpu_done_sem ;
#endif
#endif
/**
 * gpu_contiguous_mem
 *
 * continue memory space used by gpu driver to allocate fb/ts/cb buffer,
 *
 * NOTE: should configured by system fb memory policy
 *
 */


#ifdef GPU_USE_PSRAM_TEST
#ifdef GPU_CPU_PSRAM_PARALL_TEST
static volatile char *gpu_contiguous_mem = (char *)(PSRAM_NC_BASE + 0x200000);
#else
//static volatile char *gpu_contiguous_mem = (char *)(PSRAM_NC_BASE);
//static volatile char *gpu_contiguous_mem = (char *)(PSRAM_BASE);
char gpu_contiguous_mem[GPU_CONTIGUOUS_SIZE+64]PSRAM_BSS_LOC ;
#endif
#else
#ifdef GPU_USE_PSRAM
static volatile char *gpu_contiguous_mem;
// static volatile char *gpu_contiguous_mem = (char *)(0x38400000);
#else
static volatile char gpu_contiguous_mem[GPU_CONTIGUOUS_SIZE];
#endif
#endif

/**
 * Public functions
 *
 */

void gpu_init()
{
    gpu_hw_reset();
    gpu_memory_setup();
    /* Turn on the hardware */
    vg_lite_init(480, 480);
    set_gpu_done_interrupt_handler();
}

/**
 * gpu_memory_setup - setup gpu subsys memory requirement
 *
 */
void gpu_memory_setup(void)
{
#if _BAREMETAL
#ifdef WITH_OS
    gpu_done_sem = SEM_INITIALIZER((&gpu_done_sem),0);
#endif
#endif
    if (gpu_contiguous_size == 0) {
        heap = heap_register(gpu_heap_mem, GPU_HEAP_SIZE);
        ASSERT(heap != NULL, "%s", __func__);
        gpu_contiguous_size = GPU_CONTIGUOUS_SIZE;
#ifdef GPU_USE_PSRAM
        gpu_contiguous_mem = malloc(GPU_CONTIGUOUS_SIZE);

        /* align to 32 bytes */
        if ((uint32_t)gpu_contiguous_mem & 0x1F) {
            gpu_contiguous_mem = ALIGN_UP((uint32_t)gpu_contiguous_mem, 32);
            gpu_contiguous_size -= 32;
        }
#endif
        vg_lite_init_mem(GPU_BASE, 0, gpu_contiguous_mem, gpu_contiguous_size);
    }
}

/**
 * gpu_malloc - memory allocat method interface
 * @size : size required
 */
void *gpu_malloc(uint32_t size)
{
    return heap_malloc(heap, size );
}

/**
 * gpu_free - memory allocat method interface
 * @mem :  pointer to memory allocated by gpu_malloc
 */
void gpu_free(void *mem)
{
    heap_free(heap, mem);
}

extern void vg_lite_hal_poke(uint32_t address, uint32_t data);
void gpu_cache_invalid(void);

//#define GPU_L2CC_ENABLE

uint32_t gpu_l2cc_hal_peek(uint32_t address)
{
    #ifdef GPU_L2CC_ENABLE
    /* Read data from the GPU L2CC register. */
    return *(volatile  uint32_t *)(SYS_L2CC_BASE + address);
    #else
    return 0;
    #endif
}

void gpu_l2cc_hal_poke(uint32_t address, uint32_t data)
{
    /* Write data to the GPU L2CC register. */
    #ifdef GPU_L2CC_ENABLE
    *(volatile uint32_t *)(SYS_L2CC_BASE + address) = data;
    #endif
}
/**
 * gpu_cache_config -  configurate gpu cache
 *
 * call by vg_lite_init
 */
void gpu_cache_config(void)
{
   #ifdef GPU_L2CC_ENABLE
   static uint32_t l2cc_configured = 0;
   if (l2cc_configured) return;
   l2cc_configured = true;
   TRACE(3, "%s ", __func__);
    uint32_t rvalue = 0;
   gpu_l2cc_hal_poke(L2CC_CACHE_INVALIDCR, 0xf);

   rvalue = gpu_l2cc_hal_peek(L2CC_CACHE_INVALIDCR);

   while(rvalue & 0xf){
       rvalue = gpu_l2cc_hal_peek(L2CC_CACHE_INVALIDCR);
   }
   //Enable L2CC
   gpu_l2cc_hal_poke(L2CC_CACHE_ENACR, L2CC_CACHE_ENA);

   //Set prefetch contorl register, double linefill and incr double linefill enable
   rvalue = gpu_l2cc_hal_peek(L2CC_CACHE_PREFETCHCR);
   rvalue |= L2CC_CACHE_PREFETCH_DBLF_ENA;
   rvalue |= L2CC_CACHE_PREFETCH_INCDBLF_ENA;
   gpu_l2cc_hal_poke(L2CC_CACHE_PREFETCHCR, rvalue);
   #endif
}

void gpu_l2cc_invlid_all(void)
{
   #ifdef GPU_L2CC_ENABLE
    uint32_t rvalue = 0;
   //invalid all entries in L2CC 
   gpu_l2cc_hal_poke(L2CC_CACHE_INVALIDCR, 0xffff);

   rvalue = gpu_l2cc_hal_peek(L2CC_CACHE_INVALIDCR);

   while(rvalue & 0xf){
       rvalue = gpu_l2cc_hal_peek(L2CC_CACHE_INVALIDCR);
   }

   #endif
}


void gpu_hw_clk_reset(void)
{
    /*reset gpu clk */
    hal_cmu_gpu_clock_enable();
}

/**
 * gpu_hw_reset - reset gpu hardware
 * registers rest
 */

void gpu_hw_reset(void)
{
    /*set gpu power clk on*/
    hal_psc_display_enable(HAL_PSC_DISPLAY_MOD_GPU);
    hal_cmu_gpu_clock_enable();
}





#if _BAREMETAL

/**
 * vg_lite_hal_get_intr_status - return gpu irq status
 *
 */
int vg_lite_hal_get_intr_status(void)
{
   return intr_status;
}

/**
 * vg_lite_hal_clear_intr_status - clear gpu irq status
 *
 */
void vg_lite_hal_clear_intr_status(void)
{
   intr_status = 0;
}

/**
 * gpu_wait_done - suspend gpu hold task
 *
 */
void gpu_wait_done(void)
{
    int int_status = 0;
#ifdef  WITH_OS
    sem_wait(&gpu_done_sem);
#else
    while (int_status==0){
        int_status = vg_lite_hal_get_intr_status();
    }
#endif
}

/**
 * gpu_post_done - wakeup waiting task
 *
 */
void gpu_post_done(void)
{
#ifdef  WITH_OS
    sem_post(&gpu_done_sem);
#else
    intr_status = 1;
#endif
}

#endif


/**
 * vg_lite_bus_error_handler - to be notified when a bus
 * error event occurs.
 *
 */
void vg_lite_bus_error_handler()
{
     gpu_hw_clk_reset();
     return;
}


/**
 * gpu_done_irq_handler - irq handler
 *
 */

extern void vg_lite_IRQHandler(void);

static void gpu_done_irq_handler(void)
{
    //TRACE(3,"%s", __func__);
    #if _BAREMETAL
        #ifdef GPU_IRQ_TEST
        set_soft_break_point();
        #endif
        vg_lite_hal_peek(VG_LITE_INTR_STATUS);
        gpu_post_done();
    #else
        vg_lite_IRQHandler();
    #endif
}

/**
 * set_gpu_done_interrupt_handler - setup gpu irq handler
 *
 */
void set_gpu_done_interrupt_handler(void)
{
    NVIC_SetVector(GPU_DONE_IRQn, (uint32_t)gpu_done_irq_handler);
    NVIC_SetPriority(GPU_DONE_IRQn, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(GPU_DONE_IRQn);
    NVIC_EnableIRQ(GPU_DONE_IRQn);
}

/**
 * cpu_cache_flush - flush  cpu  cache table
 *
 * called before invalid gpu cache
 *
 */

POSSIBLY_UNUSED void cpu_cache_flush(uint32_t start, uint32_t length)
{
    if (length < 0x8000){
        hal_cache_sync(HAL_CACHE_ID_D_CACHE, start, length);
    }else{
        hal_cache_sync(HAL_CACHE_ID_D_CACHE, start, 0x8000);
    }
}


extern void vg_lite_hal_poke(uint32_t address, uint32_t data);


/**
 * gpu_cache_invalid - invalid gpu cache table
 *
 * called  when start GPU to process cmd buffer
 * NOTE: driver only sync/flush the internal used buffers, application
 * should flush the source buffers and invalid the dest buffers properly        
 */
void vg_lite_get_cmdbuf_info(uint32_t *start, uint32_t *bytes);
extern void * vg_lite_get_tsbuffer(void);
extern uint32_t vg_lite_get_tsbuffer_size(void);
void * vg_lite_get_rtbuffer(void);
uint32_t vg_lite_get_rtbuffer_size(void);

void gpu_cache_invalid(void)
{
    uint32_t start_cb;
    uint32_t start_ts;
    //uint32_t start_rt;
    uint32_t bytes_cb;
    uint32_t bytes_ts;
    //uint32_t bytes_rt;

    uint32_t size = 0;
    vg_lite_get_cmdbuf_info(&start_cb, &bytes_cb);
    size += bytes_cb;
    bytes_ts = vg_lite_get_tsbuffer_size();
    size += bytes_ts;
    //bytes_rt = vg_lite_get_rtbuffer_size();
    //size += bytes_rt;
    if (size > 0x8000){
        hal_cache_sync_all(HAL_CACHE_ID_D_CACHE);
        return;
    }else{
        start_ts = (uint32_t)vg_lite_get_tsbuffer();
        //start_rt = vg_lite_get_rtbuffer();
        cpu_cache_flush(start_cb, bytes_cb);
        cpu_cache_flush(start_ts, bytes_ts);
        //cpu_cache_flush(start_rt, bytes_rt);
    }
    //cpu_cache_flush(gpu_contiguous_mem, GPU_CONTIGUOUS_SIZE);
}


/**
 * cpu_gpu_data_cache_invalid - invalid cpu cache table
 *
 * called after GPU process done
 *
 */

void cpu_gpu_data_cache_invalid(uint32_t start, uint32_t length)
{
    if (length > 0x8000){
        hal_cache_invalidate(HAL_CACHE_ID_D_CACHE,start, 0x8000);
    }else{
        hal_cache_invalidate(HAL_CACHE_ID_D_CACHE,start, length);
    }
}


#ifdef MEASURE_GPU_WORKTIME_BYGPIO

 static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_gpio[] = {
        {HAL_IOMUX_PIN_P4_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
 };

 static void gpu_gpio_setup(void)
 {
    hal_iomux_init(pinmux_gpio, ARRAY_SIZE(pinmux_gpio));
    hal_gpio_pin_set_dir(HAL_IOMUX_PIN_P4_2, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_clr(HAL_IOMUX_PIN_P4_2);
 }

void gpu_set_start_flag(void)
{
    hal_gpio_pin_set(HAL_IOMUX_PIN_P4_2);
}

void gpu_set_end_flag(void)
{
    hal_gpio_pin_clr(HAL_IOMUX_PIN_P4_2);
}

#endif
