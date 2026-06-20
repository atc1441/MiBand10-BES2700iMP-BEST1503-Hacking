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
#include "log_section.h"



extern void nv_record_btdevicerecord_crash_dump(void);
extern void nv_record_blerec_crash_dump(void);
extern uint32_t __log_dump_start;
extern uint32_t __log_dump_end;

extern uint32_t __crash_dump_start;
extern uint32_t __crash_dump_end;
extern const char  sys_build_info[];

#define CRASH_DUMP_BLK_LEN           0x14000
#define CRASH_DUMP_BLK_LOG_OFFSET    0x10000
#define CRASH_INFO_BLK_NUM           2
#define CRASH_UPLOAD_OK              1

typedef struct{
    uint8_t crash_id;
    uint8_t mobile_type;
    uint8_t update_flag;
    uint8_t reseved;
    uint32_t trace_len;
    uint32_t crash_len;
}CRASH_STRUCT_T;

typedef struct{
    uint32_t blk_num;
    CRASH_STRUCT_T crash_info_blk[CRASH_INFO_BLK_NUM];
}CRASH_INFO_T;


static uint8_t record_crash_id = 0;
static uint8_t record_role = 0;
static uint8_t record_type = 0;
static uint8_t cur_blk_id = 0;
static CRASH_INFO_T local_crash_info;
static CRASH_INFO_T peer_crash_info;
static bool crash_dump_is_init = false;
static bool is_suspend_log_record = false;
static uint32_t total_len = 0;
static uint32_t assert_info_offset = 0;
static uint32_t cur_blk_start_addr = 0;
static uint32_t crash_dump_flash_len = 0;
static uint32_t crash_dump_w_seqnum = 0;
static uint32_t crash_dump_total_len = 0;
static uint32_t crash_dump_flash_offs = 0;
static CRASH_DATA_BUFFER crash_data_buffer;
static DATA_BUFFER data_buffer_list[LOG_DUMP_SECTOR_BUFFER_COUNT];
static const uint32_t crash_dump_flash_start_addr = (uint32_t)&__log_dump_start;
static const uint32_t crash_dump_flash_end_addr =  (uint32_t)&__crash_dump_end;
static const uint32_t crash_dump_flash_info_start_addr =  (uint32_t)&__crash_dump_start;
static const uint32_t crash_dump_flash_blk0_start_addr = (uint32_t)(crash_dump_flash_start_addr + CRASH_DUMP_BLK_LEN);
static const uint32_t crash_dump_flash_blk1_start_addr = (uint32_t)(crash_dump_flash_start_addr);


void crash_dump_init_buffer(void)
{
    crash_data_buffer.offset = 0;
    syspool_init_specific_size(CRASH_DUMP_SECTOR_SIZE*4);
    syspool_get_buff(&(crash_data_buffer.buffer), CRASH_DUMP_SECTOR_SIZE*4);
    CRASH_DUMP_TRACE(1,"buffer addr %p",crash_data_buffer.buffer);
}

void crash_dump_clear_buffer(void)
{
    memset(crash_data_buffer.buffer,0x00,CRASH_DUMP_SECTOR_SIZE*4);
    crash_data_buffer.offset = 0;
}

void crash_dump_env(void)
{
#ifdef NEW_NV_RECORD_ENABLED
    nv_record_btdevicerecord_crash_dump();
    nv_record_blerec_crash_dump();
#endif
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

void crash_dump_trace_output_handler(const unsigned char *buf, unsigned int buf_len)
{
    uint32_t write_len = 0;
    uint32_t written_len = 0;;
    uint32_t remain_len = 0;;
    LOG_DUMP_HEADER log_header;
    DATA_BUFFER *data_buff;

    data_buff = &data_buffer_list[crash_dump_w_seqnum];
    remain_len = buf_len;
    written_len = 0;
    do{
        if(data_buff->offset == 0)
        {
            memset((uint8_t*)&log_header,0,sizeof(log_header));
            log_header.magic = DUMP_LOG_MAGIC;
            log_header.version = DUMP_LOG_VERSION;
            log_header.seqnum = 0;
            log_header.reseved[0] = '\0';
            log_header.reseved[1] = '\0';
            log_header.reseved[2] = '\n';

            memcpy(data_buff->buffer + data_buff->offset,
                    (uint8_t*)&log_header,
                    sizeof(LOG_DUMP_HEADER));

            data_buff->offset += sizeof(LOG_DUMP_HEADER);
        }

        if(data_buff->offset + remain_len > LOG_DUMP_SECTOR_SIZE)
        {
            write_len = (LOG_DUMP_SECTOR_SIZE - data_buff->offset);
        }
        else
        {
            write_len = remain_len;
        }

        if(write_len > 0)
        {
            memcpy(data_buff->buffer + data_buff->offset,
                    buf + written_len,
                    write_len);
            data_buff->offset += write_len;
            written_len += write_len;
        }

        remain_len -= write_len;

        if(data_buff->offset == LOG_DUMP_SECTOR_SIZE)
        {
            crash_dump_w_seqnum = (crash_dump_w_seqnum + 1 == LOG_DUMP_SECTOR_BUFFER_COUNT)? \
                               0 : crash_dump_w_seqnum + 1;

            data_buff = &data_buffer_list[crash_dump_w_seqnum];
            memset(data_buff,0x00,sizeof(DATA_BUFFER));
        }
    }while(remain_len > 0);
}

void crash_dump_clac_trace_total_len(void)
{
    uint8_t i = 0;
    total_len = 0;

    for(i = 0; i<LOG_DUMP_SECTOR_BUFFER_COUNT;i++)
    {
        if(data_buffer_list[i].offset <= sizeof(LOG_DUMP_HEADER))
        {
            data_buffer_list[i].offset = 0;
        }
        total_len += (data_buffer_list[i].offset - sizeof(LOG_DUMP_HEADER));
    }

    CRASH_DUMP_TRACE(1,"ZYH clac_trace_log %d",total_len);
}

void crash_dump_write_trace_to_flash(void)
{
    uint8_t i = 0;
    DATA_BUFFER *data_buff;
    uint32_t dump_w_seqnum = 0;
    uint32_t dump_w_offset = 0;

    dump_w_seqnum = ((crash_dump_w_seqnum + 1 == LOG_DUMP_SECTOR_BUFFER_COUNT)? 0:(crash_dump_w_seqnum + 1));
    data_buff = &data_buffer_list[dump_w_seqnum];

    if((data_buff->offset == 0)||(dump_w_seqnum == 0))
    {
        for(i = 0; i<=crash_dump_w_seqnum;i++)
        {
            crash_dump_write(cur_blk_start_addr+dump_w_offset,data_buffer_list[i].buffer,LOG_DUMP_SECTOR_SIZE);
            dump_w_offset += LOG_DUMP_SECTOR_SIZE;
        }
    }
    else if(data_buff->offset != 0)
    {
        for(i = dump_w_seqnum; i< LOG_DUMP_SECTOR_BUFFER_COUNT; i++)
        {
            crash_dump_write(cur_blk_start_addr+dump_w_offset,data_buffer_list[i].buffer,LOG_DUMP_SECTOR_SIZE);
            dump_w_offset += LOG_DUMP_SECTOR_SIZE;
        }

        for(i = 0; i<= crash_dump_w_seqnum; i++)
        {
            crash_dump_write(cur_blk_start_addr+dump_w_offset,data_buffer_list[i].buffer,LOG_DUMP_SECTOR_SIZE);
            dump_w_offset += LOG_DUMP_SECTOR_SIZE;
        }
    }
}

void crash_dump_get_info(void)
{
    uint8_t i = 0;
    uint8_t cur_id = 0;
    uint8_t rd_offset = 0;
    uint32_t tmp_addr = 0;
    CRASH_DUMP_HEADER_T log_head;

    enum NORFLASH_API_RET_T ret;

    crash_dump_init_buffer();

    CRASH_DUMP_TRACE(4,"ZYH get info");

    ret = norflash_sync_read(NORFLASH_API_MODULE_ID_CRASH_DUMP,
                             crash_dump_flash_info_start_addr,
                             crash_data_buffer.buffer,
                             CRASH_DUMP_SECTOR_SIZE);

    if(ret != NORFLASH_API_OK)
    {
        CRASH_DUMP_TRACE(4,"__CRASH_DUMP:%s: norflash_sync_read,addr = 0x%x,len = 0x%x,ret = %d.",
                        __func__,crash_dump_flash_info_start_addr,(crash_dump_flash_end_addr - crash_dump_flash_start_addr),ret);
        return ;
    }

    memset((uint8_t*)(&local_crash_info),0x00,sizeof(CRASH_INFO_T));
    if(!((CRASH_DUMP_MAGIC_CODE == *((uint32_t*)(crash_data_buffer.buffer)))&&
        (CRASH_DUMP_VERSION == *((uint32_t*)(crash_data_buffer.buffer+4)))))
    {
        CRASH_DUMP_TRACE(0,"read fail , get info");
        return;
    }

    memcpy(&log_head,crash_data_buffer.buffer,sizeof(CRASH_DUMP_HEADER_T));
    rd_offset += sizeof(CRASH_DUMP_HEADER_T);

    memcpy((uint8_t*)(&local_crash_info), \
           (uint8_t*)(crash_data_buffer.buffer + rd_offset),\
           sizeof(CRASH_INFO_T));

    for(i = 0;i<local_crash_info.blk_num;i++)
    {
        cur_id = local_crash_info.crash_info_blk[i].crash_id;
        CRASH_DUMP_TRACE(1,"read has been done, id %d",cur_id);/*error*/
        if(cur_id == 1)
        {
            tmp_addr = crash_dump_flash_blk0_start_addr;
        }
        else if(cur_id == 2)
        {
            tmp_addr = crash_dump_flash_blk1_start_addr;
        }
        crash_dump_clear_buffer();
        ret = norflash_sync_read(NORFLASH_API_MODULE_ID_CRASH_DUMP,
                                  tmp_addr,
                                  crash_data_buffer.buffer,
                                  CRASH_DUMP_SECTOR_SIZE);
        CRASH_DUMP_TRACE(2,"read has been done, read error %x %x",*((uint32_t*)(crash_data_buffer.buffer)),*((uint32_t*)(crash_data_buffer.buffer+4)));/*error*/
        if(!((DUMP_LOG_MAGIC == *((uint32_t*)(crash_data_buffer.buffer)))&&
            (DUMP_LOG_VERSION == *((uint32_t*)(crash_data_buffer.buffer+4)))))
        {

            local_crash_info.crash_info_blk[i].trace_len = 0;
            local_crash_info.crash_info_blk[i].crash_len = 0;
        }
    }
}

uint8_t* crash_dump_get_local_crash_info(uint16_t* len)
{
    *len = sizeof(CRASH_INFO_T);
    return (uint8_t*)(&local_crash_info);
}

uint32_t crash_dump_slave_get_totoal_len(uint8_t id)
{
    return (local_crash_info.crash_info_blk[id -1].trace_len + local_crash_info.crash_info_blk[id - 1].crash_len);
}

void crash_dump_record_peer_crash_info(uint8_t**peer_info,uint8_t length)
{
    if(length != sizeof(CRASH_INFO_T))
    {
        CRASH_DUMP_TRACE(1,"%s:length is invaild",__func__);
        return;
    }
    CRASH_DUMP_TRACE(2,"ZYH: %p %d %d",*peer_info,length,sizeof(CRASH_INFO_T));
    memset((uint8_t*)(&peer_crash_info),0x00,sizeof(CRASH_INFO_T));
    memcpy((uint8_t*)(&peer_crash_info),(uint8_t*)(*peer_info),sizeof(CRASH_INFO_T));
}
/*need modify*/
void crash_dump_generate_crash_dump_rsp(uint8_t**rsp,uint8_t* len)
{
    uint8_t i = 0;
    uint8_t*rsp_ptr = *rsp;
    uint8_t*head_ptr = rsp_ptr;
    uint8_t wt_offset = 0;
    uint8_t total_len = 0;

    *len = 0;

    /* genernate local response part */
    rsp_ptr[0] = 0x00;

    if(local_crash_info.blk_num)
    {
        head_ptr = rsp_ptr + 1;
        rsp_ptr += 2;

        for(i = 0; i< local_crash_info.blk_num; i++)
        {
            if(local_crash_info.crash_info_blk[i].crash_id != 0)
            {
               *(rsp_ptr++) = local_crash_info.crash_info_blk[i].crash_id;
               *(rsp_ptr++) = local_crash_info.crash_info_blk[i].mobile_type;
               *(rsp_ptr++) = local_crash_info.crash_info_blk[i].update_flag;
               *(rsp_ptr++) = local_crash_info.crash_info_blk[i].reseved;
               wt_offset += 4;
            }
        }

        *head_ptr = wt_offset;
        total_len += (wt_offset + 2);
    }
    else
    {
        rsp_ptr[1] = 0x00;
        total_len += 2;
        rsp_ptr += 2;
    }

    /* genernate peer response part*/
    *rsp_ptr = 0x01;

    if(peer_crash_info.blk_num)
    {
        head_ptr = rsp_ptr + 1;
        rsp_ptr += 2;
        wt_offset = 0;

        for(i = 0; i< peer_crash_info.blk_num; i++)
        {
            if(peer_crash_info.crash_info_blk[i].crash_id != 0)
            {
                *(rsp_ptr++) = peer_crash_info.crash_info_blk[i].crash_id;
                *(rsp_ptr++) = peer_crash_info.crash_info_blk[i].mobile_type;
                *(rsp_ptr++) = peer_crash_info.crash_info_blk[i].update_flag;
                *(rsp_ptr++) = peer_crash_info.crash_info_blk[i].reseved;
                wt_offset += 4;
            }
        }

        *head_ptr = wt_offset;
        total_len += (wt_offset + 2);
    }
    else
    {
        *(rsp_ptr+1) = 0;
        total_len += 2;
    }

    *len = total_len;
}

uint32_t crash_dump_check_param_is_vaild(uint8_t crash_id,uint8_t role)
{
    uint8_t i = 0;
    uint32_t length = 0x00;
    CRASH_INFO_T* info = NULL;

    if(0x01 == role)
    {
        /*tws role is slave*/
        info = &peer_crash_info;
    }
    else
    {
        /*tws role is master or unkown*/
        info = &local_crash_info;
    }

    for(i = 0; i < info->blk_num ; i++)
    {
        TRACE(2,"[ZYH] id %d %d",info->crash_info_blk[i].crash_id,crash_id);
        if(crash_id == info->crash_info_blk[i].crash_id)
        {
            TRACE(2,"[ZYH] id %d %d ",info->crash_info_blk[i].crash_id,crash_id);
            length = info->crash_info_blk[i].trace_len + info->crash_info_blk[i].crash_len;
            TRACE(2,"[ZYH] length %d %d",info->crash_info_blk[i].trace_len,info->crash_info_blk[i].crash_len);
            break;
        }
    }
    if(length)
    {
        record_crash_id = crash_id;
        record_role = role;
    }

    return length;
}

void crash_dump_set_readtype(uint8_t type)
{
    record_type = type;
}

void crash_dump_get_crash_id(uint8_t* crash_id,uint8_t* role,uint8_t* type)
{
    *crash_id = record_crash_id;
//    *role = record_role;
    *type =  record_type;
}

int32_t crash_dump_get_assert_content(uint8_t** ptr,uint32_t* bytenum, uint8_t id,uint8_t type)
{
    uint8_t i = 0;
    enum NORFLASH_API_RET_T ret;
    uint32_t tmp_start_addr = 0;
    uint32_t length = 0;

    if(id == 1)
    {
        tmp_start_addr = crash_dump_flash_blk0_start_addr;
    }
    else if(id == 2)
    {
        tmp_start_addr = crash_dump_flash_blk1_start_addr;
    }

    crash_dump_init_buffer();

    CRASH_DUMP_TRACE(2,"%s : type %d id %d offset %d",__func__,type, id,assert_info_offset);

    if(!type)
    {
        /* read  assert information*/
        ret = norflash_sync_read(NORFLASH_API_MODULE_ID_CRASH_DUMP,
                                  tmp_start_addr+CRASH_DUMP_BLK_LOG_OFFSET,
                                  crash_data_buffer.buffer,
                                  4*CRASH_DUMP_SECTOR_SIZE);
        length = 4*CRASH_DUMP_SECTOR_SIZE;
    }
    else
    {
        for(i = 0; i<4; i++)
        {
            ret = norflash_sync_read(NORFLASH_API_MODULE_ID_CRASH_DUMP,
                                  tmp_start_addr+assert_info_offset+sizeof(LOG_DUMP_HEADER),
                                  crash_data_buffer.buffer + i*(CRASH_DUMP_SECTOR_SIZE - sizeof(LOG_DUMP_HEADER)),
                                  CRASH_DUMP_SECTOR_SIZE - sizeof(LOG_DUMP_HEADER));
            assert_info_offset += CRASH_DUMP_SECTOR_SIZE;
        }

        length = 4*(CRASH_DUMP_SECTOR_SIZE - sizeof(LOG_DUMP_HEADER));
    }

    if(ret != NORFLASH_API_OK)
    {
        CRASH_DUMP_TRACE(4,"__CRASH_DUMP:%s: norflash_sync_read,addr = 0x%x,len = 0x%x,ret = %d.",
                        __func__,crash_dump_flash_start_addr,(crash_dump_flash_end_addr - crash_dump_flash_start_addr),ret);
        return ret;
    }

    *ptr = crash_data_buffer.buffer;
    *bytenum = length;
    CRASH_DUMP_TRACE(3, "[ZYH]%s %p %d",__func__,*ptr,*bytenum);

    return 0;
}

uint8_t crash_dump_erase_crash_info(void)
{
    enum NORFLASH_API_RET_T ret;

    ret = norflash_api_erase(NORFLASH_API_MODULE_ID_CRASH_DUMP,
                             crash_dump_flash_start_addr,
                             crash_dump_flash_end_addr - crash_dump_flash_start_addr,
                             false);

    if(ret != NORFLASH_API_OK)
    {
        CRASH_DUMP_TRACE(3,"__CRASH_DUMP:%s: norflash_api_erase failed! addr = 0x%x,ret = %d.",
                        __func__,(crash_dump_flash_start_addr),ret);
        return (int32_t)ret;
    }
    return NORFLASH_API_OK;
}

void crash_dump_reset_assert_offset(void)
{
    assert_info_offset = 0;
}


uint8_t crash_dump_update_crash_info(uint32_t trace_len, uint32_t crash_len)
{
    int ret = 0;
    uint8_t i = 0;
    uint8_t rd_offset = 0;
    CRASH_DUMP_HEADER_T log_header;
    CRASH_DATA_BUFFER *data_buff = &crash_data_buffer;

    /*get crash information from flash*/
    ret = norflash_sync_read(NORFLASH_API_MODULE_ID_CRASH_DUMP,
                              crash_dump_flash_info_start_addr,
                              crash_data_buffer.buffer,
                              CRASH_DUMP_SECTOR_SIZE);
    if((ret == NORFLASH_API_OK)&& \
        (CRASH_DUMP_MAGIC_CODE == *((uint32_t*)(crash_data_buffer.buffer)))&& \
        (CRASH_DUMP_VERSION == *((uint32_t*)(crash_data_buffer.buffer+4))))
    {
        rd_offset += sizeof(CRASH_DUMP_HEADER_T);
        memcpy((uint8_t*)(&local_crash_info),(uint8_t*)(crash_data_buffer.buffer + rd_offset),sizeof(CRASH_INFO_T));
    }
    else
    {
        memset((uint8_t*)(&local_crash_info),0x00,sizeof(CRASH_INFO_T));
    }

    crash_dump_clear_buffer();
    memset((uint8_t*)&log_header,0,sizeof(log_header));
    log_header.magic = CRASH_DUMP_MAGIC_CODE;
    log_header.version = CRASH_DUMP_VERSION;
    log_header.seqnum = 0;
    log_header.reseved[0] = '\0';
    log_header.reseved[1] = '\0';
    log_header.reseved[2] = '\0';
    log_header.reseved[3] = '\n';
    memcpy(data_buff->buffer + data_buff->offset,
                   (uint8_t*)&log_header,sizeof(CRASH_DUMP_HEADER_T));
    data_buff->offset += sizeof(CRASH_DUMP_HEADER_T);

    /*get idle data blk*/
    if(local_crash_info.blk_num >= CRASH_INFO_BLK_NUM)
    {
        for(i = 0; i< CRASH_INFO_BLK_NUM; i++)
        {
            if(CRASH_UPLOAD_OK == local_crash_info.crash_info_blk[i].update_flag)
            {
                memset((uint8_t*)(&(local_crash_info.crash_info_blk[i])),\
                        0x00, sizeof(CRASH_STRUCT_T));
                local_crash_info.crash_info_blk[i].crash_id = i+1;
                local_crash_info.crash_info_blk[i].trace_len = trace_len;
                local_crash_info.crash_info_blk[i].crash_len = crash_len;
                i = i+1;
                goto update_exit;
            }
        }

        if(i == CRASH_INFO_BLK_NUM)
        {
            memset((uint8_t*)(&(local_crash_info.crash_info_blk[0])),\
                   0x00, sizeof(CRASH_STRUCT_T));
            local_crash_info.crash_info_blk[0].crash_id = 1;
            local_crash_info.crash_info_blk[0].trace_len = trace_len;
            local_crash_info.crash_info_blk[0].crash_len = crash_len;
            i = 1;
            goto update_exit;
        }
    }
    else
    {
        local_crash_info.blk_num++;
        i = local_crash_info.blk_num-1;
        memset((uint8_t*)(&(local_crash_info.crash_info_blk[i])),0x00, sizeof(CRASH_STRUCT_T));
        local_crash_info.crash_info_blk[i].crash_id = i+1;
        local_crash_info.crash_info_blk[i].trace_len = trace_len;
        local_crash_info.crash_info_blk[i].crash_len = crash_len;
        i = i+1;
        goto update_exit;
    }
update_exit:
    memcpy(data_buff->buffer + data_buff->offset,
          (uint8_t*)(&local_crash_info),sizeof(CRASH_INFO_T));

    data_buff->offset += sizeof(CRASH_INFO_T);
    crash_dump_write(crash_dump_flash_info_start_addr,data_buff->buffer,0x1000);
    return i;
}

static void _crash_dump_output(const unsigned char *buf, unsigned int buf_len)
{
    uint32_t uint_len;
    uint32_t written_len;
    uint32_t remain_len;
    CRASH_DUMP_HEADER_T log_header;
    CRASH_DATA_BUFFER *data_buff;
    static bool write_header_flag = false;

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
        if ((data_buff->offset == 0)&&(write_header_flag == false))
        {
            memset((uint8_t*)&log_header,0,sizeof(log_header));
            log_header.magic = CRASH_DUMP_MAGIC_CODE;
            log_header.version = CRASH_DUMP_VERSION;
            log_header.seqnum = 0;
            log_header.reseved[0] = '\0';
            log_header.reseved[1] = '\0';
            log_header.reseved[2] = '\0';
            log_header.reseved[3] = '\n';
            memcpy(data_buff->buffer + data_buff->offset,
                   (uint8_t*)&log_header,sizeof(CRASH_DUMP_HEADER_T));

            data_buff->offset += sizeof(CRASH_DUMP_HEADER_T);

            memcpy(data_buff->buffer + data_buff->offset,sys_build_info,strlen(sys_build_info)+1);
            data_buff->offset += strlen(sys_build_info)+1;

            write_header_flag = true;
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
            crash_dump_write(cur_blk_start_addr+ CRASH_DUMP_BLK_LOG_OFFSET+ crash_dump_flash_offs,
                             data_buff->buffer,data_buff->offset);

            crash_dump_flash_offs = crash_dump_flash_offs + data_buff->offset >= crash_dump_flash_len ?\
                                    0 : crash_dump_flash_offs + data_buff->offset;
            data_buff->offset = 0;
        }
        remain_len -= uint_len;
    }while(remain_len > 0);
}


void crash_dump_record_output(const unsigned char *buf, unsigned int buf_len)
{
    if(is_suspend_log_record)
    {
        //CRASH_DUMP_TRACE(0,"crash_dump_record_output");
        _crash_dump_output(buf,buf_len);
    }
    else
    {
        //CRASH_DUMP_TRACE(0,"crash_dump_record_output trace");
        crash_dump_trace_output_handler(buf,buf_len);
    }
}

void crash_dump_record_notify_handler(enum HAL_TRACE_STATE_T state)
{
    uint32_t lock;
    CRASH_DATA_BUFFER *data_buff;

    CRASH_DUMP_TRACE(2,"ZYH %s: %d", __func__,state);

    if ((state == HAL_TRACE_STATE_CRASH_ASSERT_START)||\
        (state == HAL_TRACE_STATE_CRASH_FAULT_START))
    {
        norflash_api_flush_enable(NORFLASH_API_USER_CP);
        crash_dump_init_buffer();
        crash_dump_clac_trace_total_len();
        cur_blk_id = crash_dump_update_crash_info(total_len,0x4000);
        if(cur_blk_id == 1)
        {
            cur_blk_start_addr = crash_dump_flash_blk0_start_addr;
        }
        else if(cur_blk_id == 2)
        {
            cur_blk_start_addr = crash_dump_flash_blk1_start_addr;
        }

        crash_dump_write_trace_to_flash();
        is_suspend_log_record = true;
        crash_dump_clear_buffer();
		crash_dump_env();
        assert_info_offset = 0;
    }
    else if(HAL_TRACE_STATE_CRASH_END == state)
    {
        lock = int_lock_global();
        data_buff = &crash_data_buffer;
        crash_dump_write(cur_blk_start_addr + CRASH_DUMP_BLK_LOG_OFFSET + crash_dump_flash_offs,
                                 data_buff->buffer,data_buff->offset);

        int_unlock_global(lock);
        CRASH_DUMP_TRACE(2,"__CRASH_DUMP END:%s: len = %d.",__func__,data_buff->offset);
    }
}


void crash_dump_init(void)
{
    uint32_t block_size = 0;
    uint32_t sector_size = 0;
    uint32_t page_size = 0;
    uint32_t buffer_len = 0;

    enum NORFLASH_API_RET_T result;
    enum HAL_FLASH_ID_T flash_id;

    flash_id = norflash_api_get_dev_id_by_addr(crash_dump_flash_start_addr);
    hal_norflash_get_size(flash_id, NULL, &block_size, &sector_size, &page_size);
    buffer_len = CRASH_DUMP_NORFALSH_BUFFER_LEN;
    CRASH_DUMP_TRACE(4,"__CRASH_DUMP:%s: crash_dump_start = 0x%x, crash_dump_end = 0x%x, buff_len = 0x%x sector size %d.",
                    __func__,
                    crash_dump_flash_start_addr,
                    crash_dump_flash_end_addr,
                    buffer_len,
                    sector_size);

    result = norflash_api_register(
                    NORFLASH_API_MODULE_ID_CRASH_DUMP,
                    flash_id,
                    crash_dump_flash_start_addr,
                    crash_dump_flash_end_addr - crash_dump_flash_start_addr,
                    block_size,
                    sector_size,
                    page_size,
                    buffer_len,
                    NULL);

    if(result != NORFLASH_API_OK)
    {
        CRASH_DUMP_TRACE(1,"__CRASH_DUMP:%s: norflash_api_register ok.", __func__);
    }

    hal_trace_app_register(HAL_TRACE_APP_REG_ID_0, crash_dump_record_notify_handler,crash_dump_record_output);
    memset((uint8_t*)(&peer_crash_info),0x00,sizeof(CRASH_INFO_T));
    memset((uint8_t*)(&local_crash_info),0x00,sizeof(CRASH_INFO_T));
    memset((uint8_t*)(data_buffer_list),0x00,sizeof(DATA_BUFFER)*LOG_DUMP_SECTOR_BUFFER_COUNT);
    crash_dump_flash_len = crash_dump_flash_end_addr - crash_dump_flash_start_addr;
    crash_dump_flash_offs = 0;
    crash_data_buffer.offset = 0;
    crash_data_buffer.buffer = NULL;
    crash_dump_is_init = true;
    is_suspend_log_record = false;
    crash_dump_w_seqnum = 0;
    crash_dump_total_len = 0;
}
