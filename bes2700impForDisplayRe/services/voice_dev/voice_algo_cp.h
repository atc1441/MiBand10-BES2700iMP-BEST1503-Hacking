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
#ifndef __VOICE_ALGO_CP_H__
#define __VOICE_ALGO_CP_H__

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t voice_algo_cp_open(uint32_t frame_len, uint32_t mic_channel_num, uint32_t ref_channel_num);

int32_t voice_algo_cp_close(void);

int32_t voice_algo_cp_process(uint8_t *buf_ptr[], uint32_t frame_len);

#ifdef __cplusplus
}
#endif

#endif

