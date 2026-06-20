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

#ifndef __NVRECORD_COMBO_H__
#define __NVRECORD_COMBO_H__

#ifdef __cplusplus
extern "C"{
#endif

#ifdef COMBO_CUSBIN_IN_FLASH
/*****************************header include********************************/
#include "nvrecord_extension.h"

/******************************macro defination*****************************/

/******************************type defination******************************/

/****************************function declearation**************************/
void nv_record_combo_bin_rec_init(void);
void nv_record_combo_bin_clear_info(void);
bool nv_record_combo_bin_get_info(uint32_t id, uint32_t* offset, uint32_t* length);
void nv_record_combo_bin_update_info(void* combo_bin_info,uint32_t crc);
void* nv_record_combo_bin_rec_get_ptr(void);
void* nv_record_combo_bin_info_get_ptr(void);
uint16_t nv_record_combo_bin_get_hearlen(void);
uint32_t nv_record_combo_bin_get_content_num(void);
uint32_t nv_record_combo_bin_get_whole_crc32(void);
uint32_t nv_record_combo_bin_get_datalen(uint8_t id);
uint32_t nv_record_combo_bin_get_fw_id(uint8_t id);
uint32_t nv_record_combo_bin_get_id_crc32(uint8_t id);
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __NVRECORD_COMBO_H__ */