/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifndef __STREAM_MCPS_H__
#define __STREAM_MCPS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Call this function when open stream
 *
 * @param name Module name
 * @param sys_freq Set sys freq
 *      1. If sys_freq is not 0, just use this vaule to calculate MCPS. So make sure the sys_freq is correctly.
 *      2. If ysys_freq is 0, stream_mcps_run_post will get sys_freq in real time. This will cause pop noise.
 * @param frame_ms Stream frame time
 * @param interval_ms Print statistical MCPS info every interval_ms
 * @return
 */
int32_t stream_mcps_start(const char *name, uint32_t sys_freq, float frame_ms, float interval_ms);

/**
 * @brief Call this function when close stream
 *
 * @param name Same name with stream_mcps_start
 * @return
 */
int32_t stream_mcps_stop(const char *name);

/**
 * @brief Call this function before algo
 *
 * @param name Same name with stream_mcps_start
 * @return
 */
int32_t stream_mcps_run_pre(const char *name);

/**
 * @brief Call this function after algo
 *
 * @param name Same name with stream_mcps_start
 * @return
 */
int32_t stream_mcps_run_post(const char *name);

#ifdef __cplusplus
}
#endif

#endif
