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
#ifndef __ANC_ASSIST_RESAMPLE_H__
#define __ANC_ASSIST_RESAMPLE_H__

#include "anc_assist_defs.h"
#include "plat_types.h"
#include "custom_allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t anc_assist_resample_init(uint32_t sample_rate, uint32_t frame_len, custom_allocator *allocator);
int32_t anc_assist_resample_deinit(void);
int32_t anc_assist_resample_process(float *pcm_buf, uint32_t buf_frame_len, uint32_t valid_frame_len);

#ifdef __cplusplus
}
#endif

#endif