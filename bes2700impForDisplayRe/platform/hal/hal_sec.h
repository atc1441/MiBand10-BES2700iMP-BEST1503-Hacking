/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifndef __HAL_SEC_H__
#define __HAL_SEC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"
#include "plat_addr_map.h"

#ifdef SEC_CTRL_BASE

enum HAL_SEC_TYPE_T {
    HAL_SEC_TYPE_S,
    HAL_SEC_TYPE_NS,
};

#include CHIP_SPECIFIC_HDR(hal_sec)

void hal_sec_init(void);

void hal_sec_sleep(void);

void hal_sec_wakeup(void);

#endif

void hal_sec_get_spy_base_list(uint32_t **base, uint32_t *cnt);

#ifdef __cplusplus
}
#endif

#endif

