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
#ifndef __ASSIST_ULTRASOUND_H__
#define __ASSIST_ULTRASOUND_H__

#include <stdbool.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void assist_ultrasound_reset(void);

bool assist_ultrasound_process(float *fb_mic, float *ref, uint32_t frame_len);

#ifdef __cplusplus
}
#endif

#endif
