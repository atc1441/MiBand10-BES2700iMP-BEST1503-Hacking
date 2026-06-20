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
#ifndef __VOICE_NOISE_EST_H__
#define __VOICE_NOISE_EST_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

int32_t voice_noise_est_open(uint32_t frame_len, uint32_t bits);

int32_t voice_noise_est_close(void);

int32_t voice_noise_est_process(int16_t *pcm_buf, int32_t pcm_len, float *out_buf);

#ifdef __cplusplus
}
#endif

#endif
