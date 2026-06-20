
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
#ifndef __HAL_OVERLAY_SUBSYS_H__
#define __HAL_OVERLAY_SUBSYS_H__


#include "hal_overlay.h"

#ifdef __cplusplus
extern "C" {
#endif

enum OVERLAY_SUBSYS_T {
    OVERLAY_M55,
    OVERLAY_HIFI4,
};

struct SUBSYS_OVERLAY_EXEC_T {
    uint32_t text_start_addr;
    uint32_t text_end_addr;
    uint32_t data_start_addr;
    uint32_t data_end_addr;
};

struct SUBSYS_OVERLAY_ITEM_T {
    uint32_t text_offset;
    uint32_t text_size;
    uint32_t data_offset;
    uint32_t data_size;
};

enum HAL_OVERLAY_RET_T hal_overlay_subsys_set_text_exec_addr(
            enum OVERLAY_SUBSYS_T chip_id, uint32_t text_vma,
            uint32_t text_size, uint32_t data_vma, uint32_t data_size);

enum HAL_OVERLAY_RET_T hal_overlay_subsys_set_text_addr(enum OVERLAY_SUBSYS_T chip_id,
            enum HAL_OVERLAY_ID_T id, uint32_t start, uint32_t stop);

enum HAL_OVERLAY_RET_T hal_overlay_subsys_set_data_addr(enum OVERLAY_SUBSYS_T chip_id,
            enum HAL_OVERLAY_ID_T id, uint32_t start, uint32_t stop);

enum HAL_OVERLAY_RET_T hal_overlay_subsys_load(enum OVERLAY_SUBSYS_T chip_id,
                                                enum HAL_OVERLAY_ID_T id);

enum HAL_OVERLAY_RET_T hal_overlay_subsys_unload(enum OVERLAY_SUBSYS_T chip_id,
                                                enum HAL_OVERLAY_ID_T id);
#ifdef __cplusplus
}
#endif

#endif /* __HAL_OVERLAY_SUBSYS_H__ */
