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
#ifndef __VOICE_ALGO_H__
#define __VOICE_ALGO_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define VOICE_ALGO_GET_FRAME_LEN            (0)
#define VOICE_ALGO_SET_FRAME_LEN            (1)

int32_t voice_algo_init(void);
int32_t voice_algo_open(uint32_t algos);
int32_t voice_algo_close(void);
int32_t voice_algo_start(uint32_t algos);
int32_t voice_algo_stop(uint32_t algos);
int32_t voice_algo_process(uint8_t *buf_ptr[], uint32_t frame_len);

int32_t voice_algo_ctl(uint32_t ctl, void *ptr);

#ifdef __cplusplus
}
#endif

#endif
