/***************************************************************************
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
 ***************************************************************************/
#include "stdio.h"
#include "string.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "app_thread.h"
#include "app_utils.h"
#include "heap_api.h"
#include "pmu.h"
#include "hal_sdmmc.h"
#include "sdmmc_msd.h"



#ifdef MSD_MODE

volatile uint8_t wait_usb_msd_configured = 0;

// ---- msd
#include "cqueue.h"
#include "usb_msd.h"

// direct io or cache io
#define MSD_DIRECT_IO_WR 1
#define MSD_DIRECT_IO_RD 1

// cache io impl
#define CACHE_IO_THREAD_STACK_SIZE (4096)
static osThreadId cache_io_thread_id;
static void cache_io_thread(void const *argument);
osThreadDef(cache_io_thread, osPriorityNormal, 1, CACHE_IO_THREAD_STACK_SIZE, "cache_io_thread");

#define CACHE_IO_QUEUE_BUF_SIZE (8192) // double comm buffer at least for perf
static CQueue  cache_io_queue;
//static unsigned char cache_io_queue_buf[CACHE_IO_QUEUE_BUF_SIZE];
osMutexId cache_io_queue_lock;
osMutexDef(cache_io_queue_lock);
osSemaphoreDef(cache_io_queue_space_signal);
osSemaphoreId cache_io_queue_space_signal;
osSemaphoreDef(cache_io_queue_signal);
osSemaphoreId cache_io_queue_signal;

#define LOCK_CACHE_IO_QUEUE() \
    osMutexWait(cache_io_queue_lock, osWaitForever)

#define UNLOCK_CACHE_IO_QUEUE() \
    osMutexRelease(cache_io_queue_lock)

#define WAIT_CACHE_IO_QUEUE_DATA() \
    osSemaphoreWait(cache_io_queue_signal, osWaitForever)

#define SIGNAL_CACHE_IO_QUEUE_DATA() \
    osSemaphoreRelease(cache_io_queue_signal)

#define WAIT_CACHE_IO_QUEUE_SPACE() \
    osSemaphoreWait(cache_io_queue_space_signal, osWaitForever)

#define SIGNAL_CACHE_IO_QUEUE_SPACE() \
    osSemaphoreRelease(cache_io_queue_space_signal)

#define CACHE_IO_TEMP_BUF_SIZE (8192) // comm buffer at least for perf
static unsigned char *cache_io_temp_buf = NULL;//[CACHE_IO_TEMP_BUF_SIZE];

#define CACHE_IO_MAILBOX_SIZE (64)
typedef struct {
    uint32_t rw;
    uint32_t start_block;
    uint32_t block_count;
} CACHE_IO_MESSAGE_BLOCK;

enum {
    CACHE_IO_IDLE = 0,
    CACHE_IO_RD,
    CACHE_IO_WR,
};

osMailQDef (cache_io_mailbox, CACHE_IO_MAILBOX_SIZE, CACHE_IO_MESSAGE_BLOCK);
static osMailQId cache_io_mailbox = NULL;
volatile static uint8_t cache_io_mailbox_cnt = 0;

volatile static uint8_t cache_io_current_do  = CACHE_IO_IDLE;
osSemaphoreDef(cache_io_rw_signal);
osSemaphoreId cache_io_rw_signal;

#define WAIT_CACHE_IO_RW() \
    osSemaphoreWait(cache_io_rw_signal, osWaitForever)

#define SIGNAL_CACHE_IO_RW() \
    osSemaphoreRelease(cache_io_rw_signal)

#if !defined(MSD_DIRECT_IO_RD)
static void wait_cache_io_read_done(void)
{
    if (cache_io_current_do == CACHE_IO_RD) {
        WAIT_CACHE_IO_RW();
        SIGNAL_CACHE_IO_RW();
    }
}
#endif

#if !defined(MSD_DIRECT_IO_WR)
static void wait_cache_io_write_done(void)
{
    if (cache_io_current_do == CACHE_IO_WR) {
        WAIT_CACHE_IO_RW();
        SIGNAL_CACHE_IO_RW();
    }
}
#endif

static int cache_io_mailbox_init(void)
{
    cache_io_mailbox = osMailCreate(osMailQ(cache_io_mailbox), NULL);
    if (cache_io_mailbox == NULL)  {
        TRACE(1, "Failed to Create cache_io_mailbox\n");
        return -1;
    }
    cache_io_mailbox_cnt = 0;
    return 0;
}

int cache_io_mailbox_put(CACHE_IO_MESSAGE_BLOCK* msg_src)
{
    osStatus status;

    CACHE_IO_MESSAGE_BLOCK *msg_p = NULL;

    msg_p = (CACHE_IO_MESSAGE_BLOCK*)osMailAlloc(cache_io_mailbox, 0);
    if(!msg_p) {
        TRACE(1, "%s , osMailAlloc fail\n", __func__);
        return -1;
    }
    ASSERT(msg_p, "osMailAlloc error");

    msg_p->rw  = msg_src->rw;
    msg_p->start_block = msg_src->start_block;
    msg_p->block_count = msg_src->block_count;

    status = osMailPut(cache_io_mailbox, msg_p);
    if (osOK == status)
        cache_io_mailbox_cnt++;
    return (int)status;
}

int cache_io_mailbox_free(CACHE_IO_MESSAGE_BLOCK* msg_p)
{
    osStatus status;

    status = osMailFree(cache_io_mailbox, msg_p);
    if (osOK == status)
        cache_io_mailbox_cnt--;

    return (int)status;
}

int cache_io_mailbox_get(CACHE_IO_MESSAGE_BLOCK** msg_p)
{
    osEvent evt;

    evt = osMailGet(cache_io_mailbox, osWaitForever);

    if (evt.status == osEventMail) {
        *msg_p = (CACHE_IO_MESSAGE_BLOCK *)evt.value.p;
        return 0;
    }
    return -1;
}

static void _cache_io_read(uint32_t start_block, uint32_t block_count)
{
    uint32_t len = 0;
    uint32_t blk_count, block_size;

    hal_sdmmc_info(HAL_SDMMC_ID_0, &blk_count, &block_size);

    hal_sdmmc_read_blocks(HAL_SDMMC_ID_0, start_block, block_count, cache_io_temp_buf);

wait_space:
    LOCK_CACHE_IO_QUEUE();
    len = AvailableOfCQueue(&cache_io_queue);
    UNLOCK_CACHE_IO_QUEUE();

    // wait enough data
    if (len < block_size*block_count) {
        WAIT_CACHE_IO_QUEUE_SPACE();
        goto wait_space;
    }

    LOCK_CACHE_IO_QUEUE();
    EnCQueue(&cache_io_queue, cache_io_temp_buf, block_count*block_size);
    UNLOCK_CACHE_IO_QUEUE();

    SIGNAL_CACHE_IO_QUEUE_DATA();
}

static void _cache_io_write(uint32_t start_block, uint32_t block_count)
{
    uint32_t len = 0;
    uint32_t blk_count, block_size;

    hal_sdmmc_info(HAL_SDMMC_ID_0, &blk_count, &block_size);

wait_data:
    LOCK_CACHE_IO_QUEUE();
    len = LengthOfCQueue(&cache_io_queue);
    UNLOCK_CACHE_IO_QUEUE();

    // wait enough data
    if (len < block_size*block_count) {
        TRACE(1, "wait data: len %d,block_size %d,block_count %d",len,block_size,block_count);
        WAIT_CACHE_IO_QUEUE_DATA();
        goto wait_data;
    }

    LOCK_CACHE_IO_QUEUE();
    DeCQueue(&cache_io_queue, cache_io_temp_buf, block_count*block_size);
    UNLOCK_CACHE_IO_QUEUE();

    SIGNAL_CACHE_IO_QUEUE_SPACE();
    len = hal_sdmmc_write_blocks(HAL_SDMMC_ID_0, start_block, block_count, cache_io_temp_buf);
    TRACE(1, "W BLOCK %d,blksize:%d ,ret %d",block_count,block_size,len);
}


void cache_io_read(uint8_t *data, uint32_t start_block, uint32_t block_count)
{
    int len = 0;
    uint32_t blk_count, block_size;
    CACHE_IO_MESSAGE_BLOCK msg_p;

    hal_sdmmc_info(HAL_SDMMC_ID_0, &blk_count, &block_size);

#if !defined(MSD_DIRECT_IO_WR)
    // wait write done
    wait_cache_io_write_done();
#endif

    msg_p.rw    = CACHE_IO_RD;
    msg_p.start_block = start_block;
    msg_p.block_count = block_count;
    // commit req
    cache_io_mailbox_put(&msg_p);

wait_data:
    LOCK_CACHE_IO_QUEUE();
    len = LengthOfCQueue(&cache_io_queue);
    UNLOCK_CACHE_IO_QUEUE();

    // wait enough data
    if ((uint32_t)len < block_size*block_count) {
        WAIT_CACHE_IO_QUEUE_DATA();
        goto wait_data;
    }

    LOCK_CACHE_IO_QUEUE();
    DeCQueue(&cache_io_queue, data, block_count*block_size);
    UNLOCK_CACHE_IO_QUEUE();

    SIGNAL_CACHE_IO_QUEUE_SPACE();
}

void cache_io_write(uint8_t *data, uint32_t start_block, uint32_t block_count)
{
    int len = 0;
    uint32_t blk_count, blk_size;
    CACHE_IO_MESSAGE_BLOCK msg_p;

    hal_sdmmc_info(HAL_SDMMC_ID_0, &blk_count, &blk_size);

#if !defined(MSD_DIRECT_IO_RD)
    // wait read done
    wait_cache_io_read_done();
#endif
    TRACE(1, "%s start_block:%d,blockcnt:%d ",__func__,start_block,block_count);
wait_space:
    LOCK_CACHE_IO_QUEUE();
    len = AvailableOfCQueue(&cache_io_queue);
    UNLOCK_CACHE_IO_QUEUE();

    // wait enough data
    if ((uint32_t)len < blk_size*block_count) {
        TRACE(1, "wait space :len %d,blk_size:%d,block_count %d",len,blk_size,block_count);
        WAIT_CACHE_IO_QUEUE_SPACE();
        goto wait_space;
    }

    LOCK_CACHE_IO_QUEUE();
    EnCQueue(&cache_io_queue, data, block_count*blk_size);
    UNLOCK_CACHE_IO_QUEUE();

    msg_p.rw    = CACHE_IO_WR;
    msg_p.start_block = start_block;
    msg_p.block_count = block_count;
    // commit req
    cache_io_mailbox_put(&msg_p);

    SIGNAL_CACHE_IO_QUEUE_DATA();
}
// cache io impl end


void read_block(uint8_t *data, uint32_t start_block, uint32_t block_count)
{
#if defined(MSD_DIRECT_IO_RD)
#if !defined(MSD_DIRECT_IO_WR)
    wait_cache_io_write_done();
#endif
    hal_sdmmc_read_blocks(HAL_SDMMC_ID_0, start_block, block_count, data);
#else
    cache_io_read(data, start_block, block_count);
#endif
}

void write_block(uint8_t *data, uint32_t start_block, uint32_t block_count)
{
#if defined(MSD_DIRECT_IO_WR)
#if !defined(MSD_DIRECT_IO_RD)
    wait_cache_io_read_done();
#endif
    hal_sdmmc_write_blocks(HAL_SDMMC_ID_0, start_block, block_count, data);
#else
    cache_io_write(data, start_block, block_count);
#endif
}

void trig_msd_start(void)
{
    wait_usb_msd_configured = 0;
}

void device_status(uint32_t state, uint32_t param)
{
    TRACE(1, "%s:%d:%d\n", __func__, __LINE__,state);
    if (state == USB_MSD_CONFIGURED) {
        trig_msd_start();
    }
}


void sd_hotplug_detect_open(void (* cb)(uint8_t)) {
    //hal_sdmmc_init_vars();
}


static void cache_io_thread(void const *argument)
{
    int ret=0;
    uint32_t sector_count, sector_size;
    CACHE_IO_MESSAGE_BLOCK *msg_p = NULL;

    // usb msd
    pmu_usb_config(PMU_USB_CONFIG_TYPE_DEVICE);

    wait_usb_msd_configured = 1;

    // app_sysfreq_req(APP_SYSFREQ_USER_RB_CTL, APP_SYSFREQ_104M);
    app_sysfreq_req(APP_SYSFREQ_USER_APP_15, APP_SYSFREQ_104M);

    // hal_cmu_pll_enable(HAL_CMU_PLL_USB, HAL_CMU_PLL_USER_SWITCH);
    hal_cmu_pll_enable(HAL_CMU_PLL_USB, HAL_CMU_PLL_USER_USB);

//    do {
        // hal_sdmmc_close(HAL_SDMMC_ID_0);
        hal_sdmmc_set_delay_func((HAL_SDMMC_DELAY_FUNC)osDelay);

        ret = hal_sdmmc_open(HAL_SDMMC_ID_0);
        TRACE(1, "!!!!!!!!!!!            RST     HAL SDMMC :%d!!!!!!!!!!1",ret);
        // if(usb_init_cnt-- < 0)
            // app_reset();
//    } while( 0!= ret);

    hal_sdmmc_info(HAL_SDMMC_ID_0, &sector_count, &sector_size);

    TRACE(1, "sdcard sector_count %d, sector_size %d\n", sector_count, sector_size);

    TRACE(1, "pmu config done\n");
    ret = usb_msd_open(read_block, write_block, device_status, USB_MSD_API_NONBLOCKING, sector_count, sector_size);

    TRACE(1, "usb_msd_open ret %d\n", ret);
    osDelay(100);

    //why while??
    while (wait_usb_msd_configured == 1) {
        osDelay(500);
    }

    osDelay(3000);

    usb_msd_start();

    while (1) {
        cache_io_mailbox_get(&msg_p);
        // app_sysfreq_req(APP_SYSFREQ_USER_RB_CTL, APP_SYSFREQ_208M);
        // hal_sdmmc_set_dma_tid();

        if (msg_p->rw == CACHE_IO_RD) {
            if (cache_io_current_do == CACHE_IO_IDLE) {
                WAIT_CACHE_IO_RW();
            }
            cache_io_current_do = CACHE_IO_RD;
        } else {
            if (cache_io_current_do == CACHE_IO_IDLE) {
                WAIT_CACHE_IO_RW();
            }
            cache_io_current_do = CACHE_IO_WR;
        }

        switch(cache_io_current_do) {
            case CACHE_IO_IDLE:
                break;
            case CACHE_IO_RD:
                _cache_io_read(msg_p->start_block, msg_p->block_count);
                break;
            case CACHE_IO_WR:
                _cache_io_write(msg_p->start_block, msg_p->block_count);
                break;
        }

        cache_io_mailbox_free(msg_p);

        if (cache_io_mailbox_cnt == 0) {
            cache_io_current_do = CACHE_IO_IDLE;
            SIGNAL_CACHE_IO_RW();
        }

       // app_sysfreq_req(APP_SYSFREQ_USER_RB_CTL, APP_SYSFREQ_32K);
    }
}

int cache_io_init(void)
{
    uint8_t *queue_buf = 0;
#if defined(SLAVE_USE_OPUS) || defined(MASTER_USE_OPUS) || defined(ALL_USE_OPUS)
    app_audio_mempool_init(app_audio_get_basebuf_ptr(APP_MEM_BACK_AUDIO), app_audio_get_basebuf_size(APP_MEM_BACK_AUDIO));
#else
    syspool_init();
#endif

    syspool_get_buff(&queue_buf, CACHE_IO_QUEUE_BUF_SIZE);
    InitCQueue(&cache_io_queue, CACHE_IO_QUEUE_BUF_SIZE, queue_buf);

    syspool_get_buff(&cache_io_temp_buf, CACHE_IO_TEMP_BUF_SIZE);

    if (cache_io_mailbox_init())
    {
        TRACE(1, "Failed to init mailbox \n");
        return -1;
    }

    cache_io_rw_signal = osSemaphoreCreate(osSemaphore(cache_io_rw_signal), 1);
    if (cache_io_rw_signal == NULL) {
        TRACE(1, "Failed to Create rw signal\n");
        return -1;
    }

    cache_io_queue_signal = osSemaphoreCreate(osSemaphore(cache_io_queue_signal), 1);
    if (cache_io_queue_signal == NULL) {
        TRACE(1, "Failed to Create queue signal\n");
        return -1;
    }

    cache_io_queue_space_signal = osSemaphoreCreate(osSemaphore(cache_io_queue_space_signal), 1);
    if (cache_io_queue_space_signal == NULL) {
        TRACE(1, "Failed to Create queue signal\n");
        return -1;
    }

    cache_io_queue_lock = osMutexCreate(osMutex(cache_io_queue_lock));
    if (cache_io_queue_lock == NULL) {
        TRACE(1, "Failed to Create queue signal\n");
        return -1;
    }

    cache_io_thread_id = osThreadCreate(osThread(cache_io_thread), NULL);
    if (cache_io_thread_id == NULL)  {
        TRACE(1, "Failed to Create cache io thread\n");
        return -1;
    }

    return 0;
}


bool usbstorage_init(void )
{
    /*
        emmc
        rst sdio d2 p04   this is not compatible with chip, if need this signal,we should control it.
        cmd ssd cmd p15
        clk ssd clk p14
        d0  ssd d0  p16
        d1  ssd d1  p17
        d2  ssd d2  p20
        d3  ssd d3  p21
    */
    //app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_208M);
    // sd card

    // this code not necessary
    // app_overlay_select(APP_OVERLAY_ID_7);

    // this code not necessary
    // app_key_open(false);

    // this code not necessary
    // usbdld_key_init();

    cache_io_init();
    TRACE(1, "cache io init  done\n");

    return true;
}

#endif
