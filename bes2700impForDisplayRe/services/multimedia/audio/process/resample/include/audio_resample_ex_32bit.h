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
#ifndef __AUDIO_RESAMPLER_EX_32BITH__
#define __AUDIO_RESAMPLER_EX_32BITH__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"
#include "hal_aud.h"
#include "audio_resample_ex.h"


uint32_t audio_resample_ex_get_buffer_size_32bit(enum AUD_CHANNEL_NUM_T chans, enum AUD_BITS_T bits, uint8_t phase_coef_num);
enum RESAMPLE_STATUS_T audio_resample_ex_open_32bit(const struct RESAMPLE_CFG_T *cfg, RESAMPLE_ID *id_ptr);
enum RESAMPLE_STATUS_T audio_resample_ex_run_32bit(RESAMPLE_ID id, const struct RESAMPLE_IO_BUF_T *io, uint32_t *in_size_ptr, uint32_t *out_size_ptr);
void audio_resample_ex_close_32bit(RESAMPLE_ID id);
void audio_resample_ex_flush_32bit(RESAMPLE_ID id);

enum RESAMPLE_STATUS_T audio_resample_ex_set_ratio_step_32bit(RESAMPLE_ID id, float ratio_step);
enum RESAMPLE_STATUS_T audio_resample_ex_get_ratio_step_32bit(RESAMPLE_ID id, float *ratio_step);

#ifdef __cplusplus
}
#endif

#endif
