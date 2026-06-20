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
#ifndef __TZ_FLASH_H__
#define __TZ_FLASH_H__

#ifdef __cplusplus
extern "C" {
#endif

#define FLASH_BP_MAP_LEN_MAX   64

typedef struct{
    uint32_t bp;
    uint32_t start_addr;
    uint32_t end_addr;
}FLASH_BP_MAP_T;


int sec_flash_init(enum HAL_FLASH_ID_T id);
int sec_flash_erase(enum HAL_FLASH_ID_T id, uint32_t addr, uint32_t len, uint32_t suspend);
int sec_flash_program(enum HAL_FLASH_ID_T id,
                       uint32_t addr,
                       uint8_t *buffer,
                       uint32_t len,
                       uint32_t suspend);
int sec_flash_read(enum HAL_FLASH_ID_T id,
                    uint32_t addr,
                    uint8_t *buffer,
                    uint32_t len);

 int sec_flash_remap_config(enum HAL_FLASH_ID_T id,
                          enum HAL_NORFLASH_REMAP_ID_T remap_id,
                          uint32_t addr,
                          uint32_t len,
                          uint32_t remap_offset);

bool sec_flash_is_remap_enabled(void);

bool sec_flash_check_signature(enum HAL_FLASH_ID_T id, uint32_t addr, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif