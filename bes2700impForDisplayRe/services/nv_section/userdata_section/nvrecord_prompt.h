/**
 * @file nvrecord_prompt.h
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

#ifndef __NVRECORD_PROMPT_H__
#define __NVRECORD_PROMPT_H__

#ifdef __cplusplus
extern "C"{
#endif

/*****************************header include********************************/
#include "nvrecord_extension.h"

/******************************macro defination*****************************/
#define LANGUAGE_ID_MAX 0xFF
#define LANGUAGE_PACKAGE_SIZE (0x25800) //!< 150K for one language
#define DEFAULT_PROMPT_NUM 1
#define PROMPT_INFO_MAX 40

/******************************type defination******************************/
typedef struct
{
    uint32_t id;
    uint32_t offset;
    uint32_t length;
    uint32_t crc32;
} PROMPT_CONTENT_INFO_T;

typedef struct
{
    uint32_t mainInfo;
    uint32_t version;
    uint32_t contentNum;
    PROMPT_CONTENT_INFO_T info[PROMPT_INFO_MAX];
    uint32_t crc32;
} PROMPT_IMAGE_HEADER_T;

/****************************function declearation**************************/
void nv_record_prompt_rec_init(void);

void nv_record_prompt_parse_file(void *addr, PROMPT_IMAGE_HEADER_T *info);
bool nv_record_prompt_check_crc(uint32_t startAddr, PROMPT_IMAGE_HEADER_T *info);
uint32_t nv_record_prompt_get_language_packet_size(PROMPT_IMAGE_HEADER_T *info);

bool nv_record_prompt_rec_add_new_package(void *pInfo);

uint32_t nv_record_prompt_get_package_addr(uint8_t language, bool addNew, uint32_t newPackageLen);

void nv_record_prompt_get_prompt_info(uint8_t language, uint16_t id, uint8_t **data, uint32_t *length);

uint8_t nv_record_prompt_get_prompt_num(void);
uint8_t nv_record_prompt_get_prompt_language(uint8_t num);
void nv_record_prompt_set_new_language_flag(void);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __NVRECORD_PROMPT_H__ */