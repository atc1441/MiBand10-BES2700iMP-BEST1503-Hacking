/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifndef __SYSTEM_SUBSYS_H__
#define __SYSTEM_SUBSYS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"
#include "hal_overlay_subsys.h"

#define SUBSYS_IMAGE_DESC_VERSION           0

#define SUBSYS_IMAGE_TYPE_SIMPLE            0
#define SUBSYS_IMAGE_TYPE_SEGMENT           1
#define SUBSYS_IMAGE_TYPE_OVERLAY           2

struct SUBSYS_IMAGE_DESC_T {
    uint16_t version;
    uint8_t  type;
    uint8_t  reserved_003;
    uint32_t image_size;
    uint32_t code_start_offset;
    union {
        uint32_t exec_addr;
        uint32_t seg_map_size;
    } u;
};

struct CODE_SEG_MAP_ITEM_T {
    uint32_t exec_addr;
    uint32_t load_offset;
    uint32_t size;
};

#define SUBSYS_OVERLAY_CNT                  8

struct OVERLAY_INFO_T {
    struct SUBSYS_OVERLAY_EXEC_T exec_info;
    struct SUBSYS_OVERLAY_ITEM_T overlay_seg[SUBSYS_OVERLAY_CNT];
};

//---------------------------------------------------------------------
// Subsys APIs
//---------------------------------------------------------------------
void system_subsys_reset_handler(void);

void system_subsys_init(void);

void system_subsys_term(void);

//---------------------------------------------------------------------
// AP APIs
//---------------------------------------------------------------------
int subsys_loader_check_image(uint32_t addr, uint32_t len, int check_magic);

const struct SUBSYS_IMAGE_DESC_T *subsys_loader_get_image_desc(uint32_t addr);

const struct CODE_SEG_MAP_ITEM_T *subsys_loader_get_seg_map(uint32_t addr);

const struct OVERLAY_INFO_T *subsys_loader_get_overlay_info(uint32_t addr);

#ifdef __cplusplus
}
#endif

#endif
