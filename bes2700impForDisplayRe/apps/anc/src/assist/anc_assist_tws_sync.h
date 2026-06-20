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
#ifndef __ANC_ASSIST_TWS_SYNC_H__
#define __ANC_ASSIST_TWS_SYNC_H__

#include "anc_assist_defs.h"
#include "plat_types.h"
#include "hal_aud.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t anc_assist_tws_sync_init(float frame_ms);

int32_t anc_assist_tws_sync_set_anc_gain_coef(uint32_t *ff_gain_changed, anc_assist_algo_id_t *ff_id, float *ff_gain_coef, uint8_t ff_ch_num,
                                              uint32_t *fb_gain_changed, anc_assist_algo_id_t *fb_id, float *fb_gain_coef, uint8_t fb_ch_num);
int32_t anc_assist_tws_sync_set_anc_curve(anc_assist_algo_id_t id, uint32_t index);

int32_t anc_assist_tws_sync_set_fir_flag(anc_assist_algo_id_t id, uint32_t flag);

int32_t anc_assist_tws_sync_heartbeat(void);

int32_t anc_assist_tws_reset_curr_info(void);

#ifdef __cplusplus
}
#endif

#endif