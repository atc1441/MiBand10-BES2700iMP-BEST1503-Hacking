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
#include <stdio.h>
#include <string.h>
#include "crash_dump_section.h"
#include <string.h>
#include "hal_timer.h"
#include "hal_norflash.h"
#include "norflash_api.h"
#include "cmsis.h"
#include "hal_cache.h"
#include "hal_sleep.h"
#include "pmu.h"
#include "heap_api.h"

extern uint32_t __crash_dump_start;
extern uint32_t __crash_dump_end;
extern void syspool_init_specific_size(uint32_t size);

static const uint32_t crash_dump_flash_start_addr = (uint32_t)&__crash_dump_start;
static const uint32_t crash_dump_flash_end_addr =  (uint32_t)&__crash_dump_end;
static uint32_t crash_dump_flash_len;
static CRASH_DATA_BUFFER crash_data_buffer;
static uint32_t crash_dump_w_seqnum = 0;
static uint32_t crash_dump_flash_offs = 0;
static uint32_t crash_dump_cur_dump_seqnum;
static uint32_t crash_dump_total_len = 0;
static bool crash_dump_is_init = false;
static uint8_t crash_dump_is_happend = 0;
static uint32_t crash_dump_type;
HAL_TRACE_APP_NOTIFY_T crash_dump_notify_cb = NULL;
HAL_TRACE_APP_OUTPUT_T crash_dump_output_cb = NULL;
HAL_TRACE_APP_OUTPUT_T crash_dump_fault_cb = NULL;
static enum NORFLASH_API_RET_T _flash_api_read(uint32_t addr, uint8_t* buffer, uint32_t len)
{
    return norflash_api_read(NORFLASH_API_MODULE_ID_CRASH_DUMP,
                addr,
                buffer,
                len);
}


static int32_t _crash_dump_get_seqnum(uint32_t * dump_seqnum, uint32_t *sector_seqnum)
{
    uint32_t i;
    uint32_t count;
    static enum NORFLASH_API_RET_T result;
    CRASH_DUMP_HEADER_T crash_dump_header;
    uint32_t max_dump_seqnum = 0;
    uint32_t max_sector_seqnum = 0;
    bool is_existed = false;

    count = (crash_dump_flash_end_addr-crash_dump_flash_start_addr)/CRASH_DUMP_BUFFER_LEN;
    for(i = 0; i < count; i++)
    {
        result = _flash_api_read(
                    crash_dump_flash_start_addr + i*CRASH_DUMP_BUFFER_LEN,
                    (uint8_t*)&crash_dump_header,
                    sizeof(CRASH_DUMP_HEADER_T));
        if(result == NORFLASH_API_OK)
        {
            if(crash_dump_header.magic == CRASH_DUMP_MAGIC_CODE
               && crash_dump_header.version == CRASH_DUMP_VERSION
              )
            {
                is_existed = true;
                if(crash_dump_header.seqnum > max_dump_seqnum)
                {
                    max_dump_seqnum = crash_dump_header.seqnum;
                    max_sector_seqnum = i;
                }
            }
        }
        else
        {
            ASSERT(0,"_crash_dump_get_seqnum: _flash_api_read failed!result = %d.",result);
        }
    }
    if(is_existed)
    {
        *dump_seqnum = max_dump_seqnum + 1;
        *sector_seqnum = max_sector_seqnum + 1 >= count ? 0: max_sector_seqnum + 1;
    }
    else
    {
        *dump_seqnum = 0;
        *sector_seqnum = 0;
    }
    return 0;
}
static void _crash_dump_notify(enum HAL_TRACE_STATE_T state)
{
    CRASH_DATA_BUFFER *data_buff;

    if(!crash_dump_is_init)
    {
        return;
    }
    if(state == HAL_TRACE_STATE_CRASH_ASSERT_START)
    {
        crash_dump_type = CRASH_DUMP_ASSERT_CODE;
    }
    else if(state == HAL_TRACE_STATE_CRASH_FAULT_START)
    {
        crash_dump_type = CRASH_DUMP_EXCEPTION_CODE;
    }
    CRASH_DUMP_TRACE(2,"__CRASH_DUMP:%s: state = %d.",__func__,state);
    CRASH_DUMP_TRACE(3,"__CRASH_DUMP:%s: start_addr = 0x%x,end_addr = 0x%x.",
                __func__,crash_dump_flash_start_addr,crash_dump_flash_end_addr);
    CRASH_DUMP_TRACE(3,"__CRASH_DUMP:%s: dump_seqnum = 0x%x,flash_offset = 0x%x.",
                __func__,crash_dump_cur_dump_seqnum,crash_dump_flash_offs);

    if(HAL_TRACE_STATE_CRASH_END == state)
    {
        data_buff = &crash_data_buffer;
        crash_dump_write(crash_dump_flash_start_addr + crash_dump_flash_offs,
                                 data_buff->buffer,data_buff->offset);
    }
}

static void _crash_dump_fault(const unsigned char *buf, unsigned int buf_len)
{
    uint32_t uint_len;
    uint32_t written_len;
    uint32_t remain_len;
    CRASH_DUMP_HEADER_T log_header;
    CRASH_DATA_BUFFER *data_buff;

    if(!crash_dump_is_init)
    {
        return;
    }

    if(strstr((char*)buf,CRASH_DUMP_PREFIX) != NULL)
    {
        return;
    }

    data_buff = &crash_data_buffer;
    remain_len = buf_len;
    written_len = 0;
    do{
        if(data_buff->offset ==  0)
        {
            memset((uint8_t*)&log_header,0,sizeof(log_header));
            log_header.magic = CRASH_DUMP_MAGIC_CODE;
            log_header.version = CRASH_DUMP_VERSION;
            log_header.seqnum = crash_dump_cur_dump_seqnum;
            log_header.reseved[0] = '\0';
            log_header.reseved[1] = '\0';
            log_header.reseved[2] = '\0';
            log_header.reseved[3] = '\n';
            memcpy(data_buff->buffer + data_buff->offset,
                    (uint8_t*)&log_header,
                    sizeof(log_header));

            data_buff->offset += sizeof(log_header);
            crash_dump_cur_dump_seqnum ++;
        }

        if(data_buff->offset + remain_len > CRASH_DUMP_SECTOR_SIZE)
        {
            uint_len = (CRASH_DUMP_SECTOR_SIZE - data_buff->offset);
        }
        else
        {
            uint_len = remain_len;
        }
        if(uint_len > 0)
        {
            memcpy(data_buff->buffer + data_buff->offset,
                    buf + written_len,
                    uint_len);
            data_buff->offset += uint_len;
            written_len += uint_len;
            crash_dump_total_len += uint_len;
        }

        if(data_buff->offset == CRASH_DUMP_SECTOR_SIZE)
        {
            crash_dump_write(crash_dump_flash_start_addr + crash_dump_flash_offs,
                             data_buff->buffer,data_buff->offset);

            crash_dump_flash_offs = crash_dump_flash_offs + data_buff->offset >= crash_dump_flash_len ?\
                                    0 : crash_dump_flash_offs + data_buff->offset;
            data_buff->offset = 0;
        }
        remain_len -= uint_len;
    }while(remain_len > 0);
}

void crash_dump_init(void)
{
    uint32_t block_size = 0;
    uint32_t sector_size = 0;
    uint32_t page_size = 0;
    uint32_t buffer_len = 0;
    uint32_t dump_seqnum = 0;
    uint32_t sector_seqnum = 0;
    enum NORFLASH_API_RET_T result;
    enum HAL_FLASH_ID_T  flash_id;

    flash_id = norflash_api_get_dev_id_by_addr(crash_dump_flash_start_addr);
    hal_norflash_get_size(flash_id, NULL, &block_size, &sector_size, &page_size);
    buffer_len = CRASH_DUMP_NORFALSH_BUFFER_LEN;
    CRASH_DUMP_TRACE(4,"__CRASH_DUMP:%s: crash_dump_start = 0x%x, crash_dump_end = 0x%x, buff_len = 0x%x.",
                    __func__,
                    crash_dump_flash_start_addr,
                    crash_dump_flash_end_addr,
                    buffer_len);

    result = norflash_api_register(
                    NORFLASH_API_MODULE_ID_CRASH_DUMP,
                    flash_id,
                    crash_dump_flash_start_addr,
                    crash_dump_flash_end_addr - crash_dump_flash_start_addr,
                    block_size,
                    sector_size,
                    page_size,
                    buffer_len,
                    NULL
                    );

    if(result == NORFLASH_API_OK)
    {
        CRASH_DUMP_TRACE(1,"__CRASH_DUMP:%s: norflash_api_register ok.", __func__);
    }
    else
    {
        CRASH_DUMP_TRACE(2,"__CRASH_DUMP:%s: norflash_api_register failed,result = %d", __func__, result);
        return;
    }

    crash_dump_notify_cb = _crash_dump_notify;
    crash_dump_output_cb = NULL;
    crash_dump_fault_cb = _crash_dump_fault;

    hal_trace_app_custom_register(HAL_TRACE_APP_REG_ID_1, crash_dump_notify_handler, NULL, crash_dump_fault_handler);
    _crash_dump_get_seqnum(&dump_seqnum,&sector_seqnum);
    crash_dump_cur_dump_seqnum = dump_seqnum;
    crash_dump_flash_len = crash_dump_flash_end_addr - crash_dump_flash_start_addr;
    crash_dump_flash_offs = sector_seqnum*CRASH_DUMP_BUFFER_LEN;
    crash_data_buffer.offset = 0;
    crash_dump_w_seqnum = 0;
    crash_dump_total_len = 0;
    crash_dump_is_init = true;
}

void crash_dump_init_buffer(void)
{
  return;
}

int32_t crash_dump_read(uint32_t addr,uint8_t* ptr,uint32_t len)
{
    enum NORFLASH_API_RET_T ret;

    ret = norflash_sync_read(NORFLASH_API_MODULE_ID_CRASH_DUMP,
                addr,
                ptr,
                len);
    if(ret != NORFLASH_API_OK)
    {
        CRASH_DUMP_TRACE(4,"__CRASH_DUMP:%s: norflash_sync_read,addr = 0x%x,len = 0x%x,ret = %d.",
                        __func__,addr,len,ret);
        return ret;
    }
    return 0;
}

int32_t crash_dump_write(uint32_t addr,uint8_t* ptr,uint32_t len)
{
    enum NORFLASH_API_RET_T ret;

    if(CRASH_LOG_ALIGN(addr,CRASH_DUMP_SECTOR_SIZE) != addr)
    {
        CRASH_DUMP_TRACE(2,"__CRASH_DUMP:%s: addr not aligned! addr = 0x%x.",__func__,addr);
        return (int32_t)NORFLASH_API_BAD_ADDR;
    }
    ret = norflash_api_erase(NORFLASH_API_MODULE_ID_CRASH_DUMP,
                        addr,
                        CRASH_DUMP_SECTOR_SIZE,
                        false);
    if(ret != NORFLASH_API_OK)
    {
        CRASH_DUMP_TRACE(3,"__CRASH_DUMP:%s: norflash_api_erase failed! addr = 0x%x,ret = %d.",
                        __func__,addr,ret);
        return (int32_t)ret;
    }

    ret = norflash_api_write(NORFLASH_API_MODULE_ID_CRASH_DUMP,
                addr,
                ptr,
                len,
                false);
    if(ret != NORFLASH_API_OK)
    {
        CRASH_DUMP_TRACE(4,"__CRASH_DUMP:%s: norflash_api_write,addr = 0x%x,len = 0x%x,ret = %d.",
                        __func__,addr,len,ret);
        return (int32_t)ret;
    }
    return 0;
}

void crash_dump_notify_handler(enum HAL_TRACE_STATE_T state)
{
    if ((state == HAL_TRACE_STATE_CRASH_ASSERT_START)
            || (state == HAL_TRACE_STATE_CRASH_FAULT_START)) {
        norflash_api_flush_enable_all();
        crash_dump_init_buffer();
    }

    if(crash_dump_notify_cb)
    {
        crash_dump_notify_cb(state);
    }
}

void crash_dump_fault_handler(const unsigned char *buf, unsigned int buf_len)
{
    if(crash_dump_fault_cb)
    {
        crash_dump_fault_cb(buf,buf_len);
    }
}

void crash_dump_set_flag(uint8_t is_happend)
{
    crash_dump_is_happend = is_happend;
}

void crash_dump_register(HAL_TRACE_APP_NOTIFY_T notify_cb,HAL_TRACE_APP_OUTPUT_T crash_output_cb,HAL_TRACE_APP_OUTPUT_T crash_fault_cb)
{
    crash_dump_notify_cb = notify_cb;
    crash_dump_output_cb = crash_output_cb;
    crash_dump_fault_cb = crash_fault_cb;
}

CRASH_DATA_BUFFER* crash_dump_get_buffer(void)
{
    return &crash_data_buffer;
}

uint32_t crash_dump_get_type(void)
{
    return crash_dump_type;
}


