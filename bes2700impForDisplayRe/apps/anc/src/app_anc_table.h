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
#ifndef __APP_ANC_TABLE_H__
#define __APP_ANC_TABLE_H__

#include "plat_types.h"
#include "app_anc.h"

typedef enum {
    ANC_CUSTOM_MODE_OFF = 0,
    ANC_CUSTOM_MODE_ANC,
    ANC_CUSTOM_MODE_FIR_ANC, // ff fir 2048
    ANC_CUSTOM_MODE_TT,
    ANC_CUSTOM_MODE_FIR_TT, // ff fir 512 + tt fir 512
    ANC_CUSTOM_MODE_ADAPTIVE,
    ANC_CUSTOM_MODE_NUM,
} anc_custom_mode_t;

typedef struct {
    uint32_t ff;
    uint32_t fb;
    uint32_t psap;
    uint32_t spk_calib;
    uint32_t tt;
    uint32_t psap_sw;
    uint32_t custom;
} app_anc_coef_index_cfg_t;

#define ANC_INVALID_COEF_INDEX      (0xff)

#ifdef __cplusplus
extern "C" {
#endif

int32_t app_anc_table_init(void);
int32_t app_anc_table_deinit(void);
app_anc_coef_index_cfg_t app_anc_table_get_cfg(app_anc_mode_t mode);
uint32_t app_anc_table_get_types(app_anc_mode_t mode);
uint32_t app_anc_table_get_custom_mode(app_anc_mode_t mode);

#ifdef __cplusplus
}
#endif

#endif
