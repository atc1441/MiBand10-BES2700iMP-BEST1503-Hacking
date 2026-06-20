/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#ifndef __TZ_OTA_H__
#define __TZ_OTA_H__

#ifdef __cplusplus
extern "C" {
#endif


void cmse_ota_start(void);
//段校验
void cmse_ota_set_segment_crc(uint32_t crc_origin);
uint32_t cmse_ota_get_segment_crc(void);
//整个bin校验 
uint32_t cmse_ota_get_whole_crc(void);
//写入bin，先擦除，再写入
//首先对数据进行crc，然后进行解密，之后调用se_flash函数写数据
uint32_t cmse_ota_write_packet(uint32_t offset, uint8_t *buf, uint32_t len);
//apply，ota完成，更新boot info
//接口中首先判断cmse bin的升级状态，校验不通过，返回false
//拼接boot info字段，更新flash boot info区域
uint32_t cmse_ota_apply(uint32_t ns_offset, uint32_t ns_length, uint32_t ns_crc);
uint32_t cmse_ota_decypt_handler_test(void);

#ifdef __cplusplus
}
#endif

#endif