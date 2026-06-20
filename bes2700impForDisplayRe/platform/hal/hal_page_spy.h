/***************************************************************************
 *
 * Copyright 2020-2022 BES.
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
#ifndef __HAL_PAGE_SPY_H__
#define __HAL_PAGE_SPY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

enum HAL_PAGE_SPY_FLAG_T {
    HAL_PAGE_SPY_FLAG_NONE      = 0,
    HAL_PAGE_SPY_FLAG_READ      = (1 << 0),
    HAL_PAGE_SPY_FLAG_WRITE     = (1 << 1),
    HAL_PAGE_SPY_FLAG_IRQ       = (1 << 2),
};

struct HAL_PAGE_SPY_HIT_INFO_T {
    uint32_t addr;
    uint32_t len;
    uint16_t master_id;
    enum HAL_PAGE_SPY_FLAG_T flags;
};

typedef int (*HAL_PAGE_SPY_IRQ_HANDLER_T)(const struct HAL_PAGE_SPY_HIT_INFO_T *info);

void hal_page_spy_set_irq_handler(HAL_PAGE_SPY_IRQ_HANDLER_T hdlr);

// CAUTION:
// The hitting address and length must be fully included in the configured start address and length.
// Partly overlapped address access will NOT be caught.
// Future chips will support partly overlapped address match.
int hal_page_spy_set(uint32_t spy_base, uint32_t id, uint32_t start_addr, uint32_t len, enum HAL_PAGE_SPY_FLAG_T flags);

int hal_page_spy_clear(uint32_t spy_base, uint32_t id);

int hal_page_spy_hit(uint32_t spy_base, uint32_t id);

int hal_page_spy_get_hit_info(uint32_t spy_base, uint32_t id, struct HAL_PAGE_SPY_HIT_INFO_T *info);

#ifdef __cplusplus
}
#endif

#endif

