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
#ifndef __AUDIO_PROCESS_VOL_H__
#define __AUDIO_PROCESS_VOL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

int32_t audio_process_vol_init(uint32_t sample_rate, uint32_t bits, uint32_t ch, float gain);
int32_t audio_process_vol_run(int32_t *pcm_buf, uint32_t frame_len);

/**
 * This API use fixed slope method.
 * ms is the time smooth from 0.0 to 1.0
 * e.g.
 *   1. If want to smooth 1.0 to 0.6 during 300ms, need set ms as following:
 *   audio_process_vol_start(0.6, (uint32_t)(300 / (1.0-0.6)));
 *
 *   2. If want to smooth 0.1 to 0.6 during 300ms, need set ms as following:
 *   audio_process_vol_start(0.6, (uint32_t)(300 / (0.6-0.1)));
 */
int32_t audio_process_vol_start(float gain, uint32_t ms);
int32_t audio_process_vol_start_impl(float gain, uint32_t ms);
int32_t audio_process_vol_start_sync(float gain, uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif