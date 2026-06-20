/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifndef __ANC_ASSIST_THREAD_H__
#define __ANC_ASSIST_THREAD_H__

#include "plat_types.h"
#include "stdint.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t anc_assist_thread_open();
int32_t anc_assist_thread_close(void);

int32_t anc_assist_thread_capture_process(float *in_buf[], uint32_t frame_len);

#ifdef __cplusplus
}
#endif

#endif