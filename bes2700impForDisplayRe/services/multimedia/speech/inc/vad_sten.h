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
#ifndef __VAD_STEN_H__
#define __VAD_STEN_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

float short_time_pwr_vad(short *mic_buf, uint32_t frame_len);

#ifdef __cplusplus
}
#endif

#endif
