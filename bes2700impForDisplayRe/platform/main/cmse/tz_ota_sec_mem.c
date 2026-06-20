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
#if defined(OTA_TZ_ENABLE) && !defined(SPA_AUDIO_SEC)

#include <stdlib.h>
#include <string.h>
#include "hal_trace.h"
#include "hal_timer.h"
#include "string.h"

#include "heap_api.h"

#if !defined(_WIN32)
#if !(defined(ARM_CMSE) && defined(CMSE_RAM_RAMX_LEND_NSE))
static heap_handle_t sec_mem_heap;
#endif
static unsigned int  sec_mem_total_size = 0;
static unsigned char *sec_mem_total_start_addr = NULL;
static unsigned int sec_mem_total_used = 0;

#if defined(ARM_CMSE) && defined(CMSE_RAM_RAMX_LEND_NSE)
#include "cmsis.h"
#include "hal_norflash.h"

#define SEC_MEM_BUFFER_SIZE (1024*32)

__attribute((section(".cmse_ram_lend_bss_sram_section")))
static volatile unsigned int sec_mem_buffer[SEC_MEM_BUFFER_SIZE / sizeof(int)] ;
#endif

#endif

void sec_mem_total_init(void)
{
#if !defined(_WIN32)
#if defined(ARM_CMSE) && defined(CMSE_RAM_RAMX_LEND_NSE)
    sec_mem_total_size = SEC_MEM_BUFFER_SIZE;
    sec_mem_total_start_addr = (unsigned char *)&sec_mem_buffer[0];
    sec_mem_total_used = 0;
#else
    syspool_init();
    sec_mem_total_size = syspool_total_size();
    sec_mem_total_start_addr = syspool_start_addr();
    sec_mem_total_used = 0;
#endif
#endif
}

unsigned int sec_mem_total_avalable(void)
{
#if !defined(_WIN32)
    return (sec_mem_total_size - sec_mem_total_used);
#else
    return 0;
#endif
}

unsigned int sec_mem_total_size_get(void)
{
    return sec_mem_total_size;
}

unsigned int sec_mem_buf_alloc(unsigned char **buff, unsigned int size)
{
#if !defined(_WIN32)
#if defined(ARM_CMSE) && defined(CMSE_RAM_RAMX_LEND_NSE)
    *buff = sec_mem_total_start_addr+sec_mem_total_used;
#else
    syspool_get_buff(buff, size);
#endif
    sec_mem_total_used += size;
    return size;
#else
    return 0;
#endif
}

void sec_mem_heap_init(unsigned int size)
{
#if !defined(_WIN32)
#if defined(ARM_CMSE) && defined(CMSE_RAM_RAMX_LEND_NSE)
#else
    unsigned char *heap_addr = NULL;
    unsigned int heap_size = 0;

    if (sec_mem_total_avalable() < size) {
        TRACE(1,"*** WARNING:%s: pool too small: %u (< %u)", __func__, heap_size, size);
        ASSERT(0,"%s size = %d %d %d",__func__,sec_mem_total_avalable(),sec_mem_total_size_get(),size);
    } else {
        heap_size = size;
    }

    sec_mem_buf_alloc(&heap_addr, heap_size);

    ASSERT(heap_addr, "%s: Failed to get buffer pool: size=%u", __func__, heap_size);

    if (heap_addr) {
        sec_mem_heap = heap_register(heap_addr, heap_size);
        ASSERT(sec_mem_heap, "%s: Failed to register heap: ptr=%p size=%u", __func__, heap_addr, heap_size);
    }

    sec_mem_total_used += size;
#endif
#endif
}

void * sec_mem_heap_malloc(unsigned int size) 
{
#if !defined(_WIN32)
#if defined(ARM_CMSE) && defined(CMSE_RAM_RAMX_LEND_NSE)
    unsigned char *buff;
    sec_mem_buf_alloc(&buff, size);
    sec_mem_total_used += size;
    return (void *)buff;
#else
    void *ptr = NULL;
    ptr = heap_malloc(sec_mem_heap, size);
    ASSERT(ptr, "%s size:%d", __func__, size);
    return ptr;
#endif
#else
    return (void*)malloc(size);
#endif
}

void *sec_mem_heap_calloc(unsigned int  size)
{
#if !defined(_WIN32)
#if defined(ARM_CMSE) && defined(CMSE_RAM_RAMX_LEND_NSE)
    unsigned char *buff;
    sec_mem_buf_alloc(&buff, size);
    ASSERT(buff, "%s size:%d", __func__, size);
    memset(buff, 0, size);
    sec_mem_total_used += size;
    return (void *)buff;
#else
    void *ptr = NULL;
    ptr = heap_malloc(sec_mem_heap, size);
    ASSERT(ptr, "%s size:%d", __func__, size);
    memset(ptr, 0, size);

    return ptr;
#endif
#else
    return (void*)calloc(1, size);
#endif
}

void * sec_mem_heap_realloc(void *rmem, unsigned int  newsize)
{
#if !defined(_WIN32)
#if defined(ARM_CMSE) && defined(CMSE_RAM_RAMX_LEND_NSE)
    unsigned char *buff;
    sec_mem_buf_alloc(&buff, newsize);
    ASSERT(buff, "%s size:%d", __func__, newsize);
    memset(buff, 0, newsize);
    sec_mem_total_used += newsize;
    return (void *)buff;
#else
    void *ptr = NULL;
    ptr = heap_realloc(sec_mem_heap, rmem, newsize);
    ASSERT(ptr, "%s rmem:%p size:%d", __func__, rmem,newsize);
    return ptr;
#endif
#else
    return (void*)realloc(rmem, newsize);
#endif
}

void sec_mem_heap_free(void *rmem)
{
#if !defined(_WIN32)
#if defined(ARM_CMSE) && defined(CMSE_RAM_RAMX_LEND_NSE)
#else
    ASSERT(rmem, "%s rmem:%p", __func__, rmem);
    heap_free(sec_mem_heap, rmem);
    rmem = NULL;
#endif
#else
    if(rmem){
        free(rmem);
    }
#endif
}

#if defined(ARM_CMSE) && defined(CMSE_RAM_RAMX_LEND_NSE)
int cmse_flash_load_into_sram(uint32_t flash_start,uint32_t flash_end,uint32_t sram_start , uint32_t sram_end,uint8_t * cache_buf,uint32_t cache_buf_len)
{
    uint32_t i = 0;
    uint32_t crypt_iter_op_in_len = 0;
    uint32_t crypt_iter_op_size = 0;
    uint32_t crypt_iter_op_remain = 0;
    uint32_t infile_len = flash_end - flash_start;

    unsigned char *outbuf;

    char * op_file_pos = (char *)flash_start;

    outbuf = cache_buf;

    if(outbuf == NULL){
        ASSERT(0,"%s buf error",__func__);
    }

    customer_load_sram_ram_text_data_init_offset_reset();

    crypt_iter_op_size = cache_buf_len;
    crypt_iter_op_remain = flash_end - flash_start;

    for(i = 0; i < infile_len; i+= crypt_iter_op_in_len){

        crypt_iter_op_in_len = crypt_iter_op_size;

        if(crypt_iter_op_remain < cache_buf_len){
            crypt_iter_op_in_len = crypt_iter_op_remain;
        }
        sec_flash_read(HAL_FLASH_ID_0,(uint32_t)op_file_pos,outbuf,crypt_iter_op_in_len);

        DUMP8("%02x ",outbuf,(crypt_iter_op_in_len > 16)?(16):(crypt_iter_op_in_len));
        hal_sys_timer_delay_us(100);

        op_file_pos += crypt_iter_op_in_len;

        customer_load_sram_ram_text_memcpy_with_offset((uint32_t*)outbuf,(uint32_t*)(outbuf + crypt_iter_op_in_len),crypt_iter_op_in_len/sizeof(int),(uint32_t*)sram_start,(uint32_t*)sram_end);
        crypt_iter_op_remain -= crypt_iter_op_in_len;
    }


    return 0;
}

int cmse_flash_load_into_sram_if(uint32_t flash_start,uint32_t flash_end,uint32_t sram_start , uint32_t sram_end)
{
    unsigned char * buf;
    unsigned int len;

    sec_mem_total_init();

    len = 1024*4;

    sec_mem_buf_alloc(&buf,len);

    return cmse_flash_load_into_sram(flash_start,flash_end,sram_start,sram_end,buf,len);
}
#endif

#endif
