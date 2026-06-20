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
#ifndef __APP_ANC_H__
#define __APP_ANC_H__

#include "plat_types.h"
#include "hal_aud.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    APP_ANC_MODE_OFF = 0,
    APP_ANC_MODE1,
    APP_ANC_MODE2,
    APP_ANC_MODE3,
    APP_ANC_MODE4,
    APP_ANC_MODE5,
    APP_ANC_MODE6,
    APP_ANC_MODE7,
    APP_ANC_MODE8,
    APP_ANC_MODE9,
    APP_ANC_MODE10,
    APP_ANC_MODE_QTY,
} app_anc_mode_t;

int32_t app_anc_init(void);
int32_t app_anc_deinit(void);

bool app_anc_work_status(void);
app_anc_mode_t app_anc_get_curr_mode(void);
uint32_t app_anc_get_curr_types(void);

// Switch mode with sync
int32_t app_anc_switch(app_anc_mode_t mode);
// Just switch local mode
int32_t app_anc_switch_locally(app_anc_mode_t mode);

// UI: Switch mode one by one
int32_t app_anc_loop_switch(void);

int32_t app_anc_thread_set_gain(enum ANC_TYPE_T type, float gain_l, float gain_r);

int32_t app_anc_enable_assist(bool en);
bool app_anc_assist_is_enable(void);

#ifdef __cplusplus
}
#endif

#endif
