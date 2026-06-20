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
#ifndef __APP_ANC_SYNC_H__
#define __APP_ANC_SYNC_H__

#include "plat_types.h"

typedef enum {
    APP_ANC_SYNC_MODE,
    APP_ANC_SYNC_PSAP_BANDS_GAIN,
    APP_ANC_SYNC_PSAP_BANDS_GAIN_ARRAY,
    APP_ANC_SYNC_QTY
} app_anc_sync_t;

#ifdef __cplusplus
extern "C" {
#endif

int32_t app_anc_sync_init(void);

int32_t app_anc_sync_send(uint8_t *buf, uint32_t len);
int32_t app_anc_sync_mode(uint32_t mode);
int32_t app_anc_sync_psap_bands_gain(float gain_l, float gain_r, uint32_t index_start, uint32_t index_end);
int32_t app_anc_sync_psap_bands_gain_array(float *gain);

int32_t app_anc_tws_sync_change(uint8_t *buf, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif
