/**
 * @file nvrecord_prompt.c
 * @author BES AI team
 * @version 0.1
 * @date 2020-07-15
 * 
 * @copyright Copyright (c) 2015-2020 BES Technic.
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
 */

/*****************************header include********************************/
#include "cmsis.h"
#include "nv_section_dbg.h"
#include "nvrecord_prompt.h"
#include "crc32_c.h"

/*********************external function declearation************************/

/************************private macro defination***************************/

/************************private type defination****************************/

/************************extern function declearation***********************/
extern uint32_t __prompt_start[];
extern uint32_t __prompt_end[];

#ifdef PROMPT_EMBEDED
extern unsigned char __en_prompt_package[];
#define DEFAULT_PROMPT_ADDR __en_prompt_package
#else
#define DEFAULT_PROMPT_ADDR __prompt_start
#endif

#define PROMPT_IMAGE_HEADER_SIZE 16
#define PROMPT_CONTENT_INFO_SIZE 12

/**********************private function declearation************************/

/************************private variable defination************************/
static NV_PROMPT_INFO_T *nvrecord_prompt_p = NULL;


PACKAGE_NODE_T defaultPromptInfo[DEFAULT_PROMPT_NUM] = {
    {0x01, (uint32_t)DEFAULT_PROMPT_ADDR, LANGUAGE_PACKAGE_SIZE}, //!< ID = 1 means this is an English package
};

/****************************function defination****************************/
static uint32_t _read_uint32_little_endian_and_increment(const uint8_t **read_ptr)
{
    uint8_t i = 0;
    uint32_t ret = 0;
    for (i = 0; i < 4; i++)
    {
        ret |= (**read_ptr << (8 * i));
        (*read_ptr)++;
    }
    return ret;
}

void nv_record_prompt_parse_file(void *addr, PROMPT_IMAGE_HEADER_T *info)
{
    ASSERT(addr, "invalid file address");
    ASSERT(info, "invalid info pointer");

    const uint8_t *pRead = (const uint8_t *)addr;

    info->mainInfo = _read_uint32_little_endian_and_increment(&pRead);
    LOG_I("main-info:0x%x", info->mainInfo);

    info->version = _read_uint32_little_endian_and_increment(&pRead);
    LOG_I("version:0x%x", info->version);

    info->contentNum = _read_uint32_little_endian_and_increment(&pRead);
    LOG_I("content number:0x%x", info->contentNum);
    ASSERT(PROMPT_INFO_MAX >= info->contentNum, "content number exceed %d", PROMPT_INFO_MAX);

    for (uint8_t i = 0; i < info->contentNum; i++)
    {
        info->info[i].id = _read_uint32_little_endian_and_increment(&pRead);
        LOG_I("content %d id:0x%x", i, info->info[i].id);

        info->info[i].offset = _read_uint32_little_endian_and_increment(&pRead);
        LOG_I("content %d offset:0x%x", i, info->info[i].offset);

        info->info[i].length = _read_uint32_little_endian_and_increment(&pRead);
        LOG_I("content %d len:0x%x", i, info->info[i].length);

        info->info[i].crc32 = _read_uint32_little_endian_and_increment(&pRead);
        LOG_I("content %d crc32:0x%x", i, info->info[i].crc32);
    }

    info->crc32 =  _read_uint32_little_endian_and_increment(&pRead);
    LOG_I("crc32:0x%x", info->crc32);
}

bool nv_record_prompt_check_crc(uint32_t startAddr, PROMPT_IMAGE_HEADER_T *info)
{
    ASSERT(startAddr, "invalid file address");
    ASSERT(info, "invalid info pointer");

    uint32_t crc32 = 0;

    for (uint8_t i = 0; i < info->contentNum; i++)
    {
        crc32 = crc32_c(crc32, (uint8_t*)(startAddr + info->info[i].offset), info->info[i].length);
        TRACE(4,"[%d] crc:0x%x offset:%d len:%d",i, crc32, info->info[i].offset, info->info[i].length);
    }
    LOG_I("%s flash crc:0x%x calcute crc 0x%x", __func__, info->crc32, crc32);
    return crc32 == info->crc32 ? true : false;
}

uint32_t nv_record_prompt_get_language_packet_size(PROMPT_IMAGE_HEADER_T *info)
{
    ASSERT(info, "invalid info pointer");

    uint32_t size = 0; 

    size = PROMPT_IMAGE_HEADER_SIZE + PROMPT_CONTENT_INFO_SIZE * info->contentNum;
    for (uint8_t i = 0; i < info->contentNum; i++)
    {
        size += info->info[i].length;
    }
    return size;
}

extern bool app_prompt_apply(void);

void nv_record_prompt_rec_init(void)
{
    uint32_t flush = 0;
    PROMPT_IMAGE_HEADER_T imageInfo;
    PACKAGE_NODE_T nodeInfo;
    if (NULL == nvrecord_prompt_p)
    {
        nvrecord_prompt_p = &(nvrecord_extension_p->prompt_info);
    }

    if(nvrecord_prompt_p->newFlag)
    {
        flush = 1;
        //copy new packet.
#if defined(PROMPT_IN_FLASH)&&defined(BES_OTA)
        app_prompt_apply();
#endif
        uint32_t lock = nv_record_pre_write_operation();
        nvrecord_prompt_p->newFlag = 0;
        nv_record_post_write_operation(lock);
    }

    if(!nvrecord_prompt_p->num)
    {
        for(uint8_t i = 0; i < LOCAL_PACKAGE_MAX; i++)
        {
            uint32_t startAddr = (uint32_t)__prompt_start+i*LANGUAGE_PACKAGE_SIZE;
            nv_record_prompt_parse_file((void*)startAddr, &imageInfo);
            //if(nv_record_prompt_check_crc(startAddr, &imageInfo))
            {
                //language patcker check pass, add to prompt struct.
                nodeInfo.startAddr = startAddr;
                nodeInfo.id = imageInfo.mainInfo;
                nodeInfo.len = nv_record_prompt_get_language_packet_size(&imageInfo);
                nv_record_prompt_rec_add_new_package((void*)&nodeInfo);
            }
        }
        if(!nvrecord_prompt_p->num)
        {
            //flash has no language packet, register default
            for (uint8_t i = 0; i < DEFAULT_PROMPT_NUM; i++)
            {
                LOG_I("%s set default language, language: %d prompt_package_start:0x%x", \
                    __func__, defaultPromptInfo[i].id, defaultPromptInfo[i].startAddr);
                nv_record_prompt_rec_add_new_package((void *)&defaultPromptInfo[i]);
            }
        }
        flush = 1;
    }
    if(flush)
    {
        //flush userdata
        nv_record_update_runtime_userdata();
        nv_record_execute_async_flush();
    }
}

void nv_record_rec_get_ptr(void **ptr)
{
    *ptr = nvrecord_prompt_p;
}

bool nv_record_prompt_rec_add_new_package(void *pInfo)
{
    bool ret = true;
    bool found = false;

    PACKAGE_NODE_T *info = (PACKAGE_NODE_T *)pInfo;

    if (info)
    {
        uint32_t lock = nv_record_pre_write_operation();

        for (uint8_t i = 0; i < nvrecord_prompt_p->num; i++)
        {
             if (nvrecord_prompt_p->packageInfo[i].id == info->id)
            {
                LOG_I("info->startAddr:%x", info->startAddr);

                ///.update the start address
                if (info->startAddr)
                {
                    nvrecord_prompt_p->packageInfo[i].startAddr = info->startAddr;
                }

                LOG_I("package already exist in user section, id:%d, startAddr:%x",
                      nvrecord_prompt_p->packageInfo[i].id,
                      nvrecord_prompt_p->packageInfo[i].startAddr);
                found = true;
                break;
            }
        }

        LOG_I("current supported package cnt:%d", nvrecord_prompt_p->num);

        /// brand new model insert
        if (!found)
        {
            memcpy(&nvrecord_prompt_p->packageInfo[nvrecord_prompt_p->num],
                   info,
                   sizeof(PACKAGE_NODE_T));

            LOG_I("new added package:%x, startAddr:%x, len:%x", info->id, info->startAddr, info->len);
            nvrecord_prompt_p->num++;
        }
        nv_record_post_write_operation(lock);

        /// new model file incoming, update flash info
        if (!found)
        {
            /// flush the hotword info to flash
            nv_record_update_runtime_userdata();
            nv_record_execute_async_flush();
        }

        LOG_I("current supported package Cnt:%d", nvrecord_prompt_p->num);
    }
    else
    {
        LOG_W("NULL pointer received in %s", __func__);
        ret = false;
    }

    return ret;
}

uint32_t nv_record_prompt_get_package_addr(uint8_t language, bool addNew, uint32_t newPackageLen)
{
    uint32_t addr = 0;
    uint8_t index = 0xFF;

    LOG_I("%s revceived language ID: %d", __func__, language);
    ASSERT(nvrecord_prompt_p, "nvrecord_prompt_p is null");
    ASSERT(nvrecord_prompt_p->num, "no local supported package");

    for (uint8_t i = 0; i < ARRAY_SIZE(nvrecord_prompt_p->packageInfo); i++)
    {
        if (nvrecord_prompt_p->packageInfo[i].id == language)
        {
            addr = nvrecord_prompt_p->packageInfo[i].startAddr;
            index = i;
            LOG_I("found saved package, index:%d", index);
            break;
        }
    }

    if (addNew)
    {
        if (0xFF == index)
        {
            if (nvrecord_prompt_p->num >= LOCAL_PACKAGE_MAX) //!< no free slot
            {
                /// update the address
                addr = nvrecord_prompt_p->packageInfo[0].startAddr;

                uint32_t lock = nv_record_pre_write_operation(); //!< disable the MPU for info update
                uint32_t intLock = int_lock();                   //!< lock the global interrupt

                /// overwrite the eldest package
                for (uint8_t i = 0; i < (LOCAL_PACKAGE_MAX - 1); i++)
                {
                    nvrecord_prompt_p->packageInfo[i] = nvrecord_prompt_p->packageInfo[i + 1];
                }

                /// minus the supported package
                nvrecord_prompt_p->num--;

                int_unlock(intLock); //!< unlock the global interrupt

                nv_record_post_write_operation(lock); //!< enable the MPU

                /// flush the hotword info to flash
                nv_record_update_runtime_userdata();
                nv_record_execute_async_flush();
            }
            else //!< there is free slot(s)
            {
                /// update the address
                if(nvrecord_prompt_p->num > 0)
                {
                    addr = nvrecord_prompt_p->packageInfo[nvrecord_prompt_p->num - 1].startAddr;
                }
                else
                {
                    LOG_I("%s error: nvrecord_prompt_p->num = 0", __func__);
                }
                    
                LOG_I("start_addr:%x, prompt_start:%x", addr, (uint32_t)__prompt_start);

                /// address belongs to embeded one
                if ((addr & 0xFFFFFF) < ((uint32_t)__prompt_start & 0xFFFFFF))
                {
                    addr = (uint32_t)__prompt_start;
                }
                else
                {
                    addr += LANGUAGE_PACKAGE_SIZE;
                }

                LOG_I("address is 0x%x", addr);
            }
        }
        else
        {
            /// skip the address of embeded prompt
            if (addr < (uint32_t)__prompt_start)
            {
                addr = (uint32_t)__prompt_start;
            }
        }
    }

    /// check if the address is legal
    // ASSERT((addr + newPackageLen) <= (uint32_t)__prompt_end, "new package length:%d", newPackageLen);

    return addr;
}

void nv_record_prompt_get_prompt_info(uint8_t language, uint16_t id, uint8_t **data, uint32_t *length)
{
    bool found = false;
    PROMPT_IMAGE_HEADER_T info;
    uint32_t packageAddr = nv_record_prompt_get_package_addr(language, false, 0);
    if(!packageAddr)
    {
        *length = 0;
        *data = NULL;
        LOG_I("%s language %d not found", __func__, language);
        return;
    }

    nv_record_prompt_parse_file((void *)packageAddr, &info);

    for (uint8_t i = 0; i < info.contentNum; i++)
    {
        if (id == info.info[i].id)
        {
            found = true;
            *data = (uint8_t*)(packageAddr + info.info[i].offset);
            *length = info.info[i].length;
            break;
        }
    }

    if (found)
    {
        LOG_I("prompt addr: %p", *data);
        LOG_I("prompt length: %d", *length);
    }
    else
    {
        ASSERT(0, "language %d prompt %d not found", language, id);
    }
}

uint8_t nv_record_prompt_get_prompt_num(void)
{
    return nvrecord_prompt_p->num;
}

uint8_t nv_record_prompt_get_prompt_language(uint8_t num)
{
    if((num > 0) && (num <= nvrecord_prompt_p->num))
    {
        return nvrecord_prompt_p->packageInfo[num-1].id;
    }
    else
    {
        return 0;
    }
}

void nv_record_prompt_set_new_language_flag(void)
{
    uint32_t lock = nv_record_pre_write_operation();
    nvrecord_prompt_p->newFlag = 1;
    nv_record_post_write_operation(lock);
    /// flush the hotword info to flash
    nv_record_update_runtime_userdata();
    nv_record_execute_async_flush();
}
