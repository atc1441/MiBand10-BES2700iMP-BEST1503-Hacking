/**
 * @file ota_config.h
 * @author BES AI team
 * @version 0.1
 * @date 2020-06-20
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

/**
 * NOTE: This file is used to initialize the BES OTA related configurations
 * 
 */

#ifndef __OTA_CONFIG_H__
#define __OTA_CONFIG_H__

#ifdef __cplusplus
extern "C"{
#endif

/*****************************header include********************************/
#include "nvrecord_extension.h"

/******************************macro defination*****************************/

/******************************type defination******************************/
#ifdef COMBO_CUSBIN_IN_FLASH
typedef COMBO_CONTENT_INFO_T CONTENT_INFO_T;
typedef NV_COMBO_IMAGE_INFO_T CUSTOM_IMAGE_HEADER_T;
uint32_t _combo_bin_get_addr_by_id(uint32_t id);
uint32_t _combo_bin_get_max_size(uint32_t id);
#endif


/****************************function declearation**************************/
void bes_ota_init_handler(void);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __OTA_CONFIG_H__ */