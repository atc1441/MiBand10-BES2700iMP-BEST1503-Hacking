/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifndef __HAL_TRNG_H__
#define __HAL_TRNG_H__

#include "plat_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_TRNG_DATA_LEN                   24

enum HAL_TRNG_RND_SRC_SEL
{
    HAL_TRNG_RND_SRC_SHORTEST = 0,
    HAL_TRNG_RND_SRC_SHORT,
    HAL_TRNG_RND_SRC_LONG,
    HAL_TRNG_RND_SRC_LONGEST,
};
typedef unsigned int HAL_TRNG_RND_SRC_SEL_ENUM;

struct HAL_TRNG_CFG_T
{
    uint32_t                    sample_cntr1; // must not be less than 0x11
    HAL_TRNG_RND_SRC_SEL_ENUM   rnd_src_sel;
};

typedef void (*HAL_TRNG_RUN_CB_T)(const uint32_t *buf, uint32_t len, uint32_t error);

int hal_trng_open(const struct HAL_TRNG_CFG_T *cfg);

void hal_trng_close(void);

/*
 * Attention: User random api should add lock around this function, and check the return value
 * If this function return error or timeout, you can increase sample_cntr1
 * TRNG module is stopped in the end of this function
 * Don't close TRNG module between twice hal_get_trngdata, or the result is NOT TRUE RNADOM
 * The power consumption of trng module is about 30uA@3.8V at best2003 after stopping
 */
int hal_get_trngdata(uint8_t *buf, uint32_t buf_len);

int hal_trng_run(HAL_TRNG_RUN_CB_T cb);

void hal_trng_cancel(void);

#ifdef __cplusplus
}
#endif

#endif //__HAL_TRNG_H__
