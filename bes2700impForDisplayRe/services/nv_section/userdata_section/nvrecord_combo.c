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
#ifdef COMBO_CUSBIN_IN_FLASH
#include "cmsis.h"
#include "nv_section_dbg.h"
#include "nvrecord_combo.h"

/*********************external function declearation************************/

/************************private macro defination***************************/

/************************private type defination****************************/

/************************extern function declearation***********************/

/**********************private function declearation************************/

/************************private variable defination************************/
static NV_COMBO_IMAGE_INFO_T *nvrecord_combo_p = NULL;
/****************************function defination****************************/
void nv_record_combo_bin_rec_init(void)
{
    if (NULL == nvrecord_combo_p)
    {
        nvrecord_combo_p = &(nvrecord_extension_p->combo_bin_info);
    }
}

void* nv_record_combo_bin_rec_get_ptr(void)
{
    return nvrecord_combo_p;
}

void* nv_record_combo_bin_info_get_ptr(void)
{
    return nvrecord_combo_p->info;
}


void nv_record_combo_bin_update_info(void* combo_bin_info,uint32_t crc)
{
    bool update = false;
    uint32_t lock = 0;
    NV_COMBO_IMAGE_INFO_T* pcombo_src =(NV_COMBO_IMAGE_INFO_T*)combo_bin_info;
    NV_COMBO_IMAGE_INFO_T *pcombo_dst= nvrecord_combo_p;

    LOG_I("%s", __func__);

    lock = nv_record_pre_write_operation();
    pcombo_src->crc32 = crc;

    if ((pcombo_src->mainInfo != pcombo_dst->mainInfo) ||\
        (pcombo_src->version != pcombo_dst->version) || \
        (pcombo_src->contentNum != pcombo_dst->contentNum) ||\
        (pcombo_src->crc32 != pcombo_dst->crc32)||\
        (memcmp(pcombo_dst->info,pcombo_src->info,(pcombo_src->contentNum*sizeof(COMBO_CONTENT_INFO_T)))))
    {
        memcpy(pcombo_dst,pcombo_src,3*sizeof(uint32_t));
        memcpy(pcombo_dst->info,pcombo_src->info,(pcombo_src->contentNum*sizeof(COMBO_CONTENT_INFO_T)));
        pcombo_dst->crc32 = pcombo_src->crc32;
        LOG_I("%s %x ", __func__,pcombo_dst->crc32);
        update = true;
    }

    nv_record_post_write_operation(lock);

    if (update)
    {
        LOG_I("will update info into flash.");
        nv_record_update_runtime_userdata();
        nv_record_flash_flush();
    }
}

bool nv_record_combo_bin_get_info(uint32_t id, uint32_t* offset, uint32_t* length)
{
    uint8_t i = 0;
    bool ret = false;
    NV_COMBO_IMAGE_INFO_T *pcombo= nvrecord_combo_p;

    for(i = 0; i < pcombo->contentNum; i++)
    {
        if (pcombo->info[i].id == id)
        {
            *offset = pcombo->info[i].offset;
            *length = pcombo->info[i].length;
            ret = true;
            break;
        }
    }

    return ret;
}

void nv_record_combo_bin_clear_info(void)
{
    uint32_t lock = 0;
    NV_COMBO_IMAGE_INFO_T *pcombo= nvrecord_combo_p;
    lock = nv_record_pre_write_operation();
    memset(pcombo,0x00,sizeof(NV_COMBO_IMAGE_INFO_T));
    nv_record_post_write_operation(lock);
    LOG_I("will update info into flash.");
    nv_record_update_runtime_userdata();
    nv_record_flash_flush();
}

uint16_t nv_record_combo_bin_get_hearlen(void)
{
    NV_COMBO_IMAGE_INFO_T *pcombo= nvrecord_combo_p;
    return pcombo->info[0].offset;
}

uint32_t nv_record_combo_bin_get_content_num(void)
{
    NV_COMBO_IMAGE_INFO_T *pcombo= nvrecord_combo_p;
    return pcombo->contentNum;
}

uint32_t nv_record_combo_bin_get_datalen(uint8_t id)
{
    NV_COMBO_IMAGE_INFO_T *pcombo= nvrecord_combo_p;
    return pcombo->info[id -1].length;
}

uint32_t nv_record_combo_bin_get_whole_crc32(void)
{
    NV_COMBO_IMAGE_INFO_T *pcombo= nvrecord_combo_p;
    return pcombo->crc32;
}

uint32_t nv_record_combo_bin_get_fw_id(uint8_t id)
{
    NV_COMBO_IMAGE_INFO_T *pcombo= nvrecord_combo_p;
    return pcombo->info[id -1].id;
}

uint32_t nv_record_combo_bin_get_id_crc32(uint8_t id)
{
    NV_COMBO_IMAGE_INFO_T *pcombo= nvrecord_combo_p;
    return pcombo->info[id -1].crc32;
}
#endif
