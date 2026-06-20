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
/**
 ****************************************************************************************
 * @addtogroup GAF_MEDIA_PID
 * @{
 ****************************************************************************************
 */

#ifndef GAF_MEDIA_PID_H_
#define GAF_MEDIA_PID_H_


typedef struct
{
    int32_t diff_time0;
    int32_t diff_time1;
    int32_t diff_time2;
    float Kp;
    float Ki;
    float Kd;
    float result;
    int32_t gap_threshold_us;
} gaf_media_pid_t;

#ifdef __cplusplus
extern "C" {
#endif

#define GAF_MEDIA_PID_ABS(value)                  ((value) > 0?(value):(-value))

void gaf_media_pid_init(gaf_media_pid_t* pStreamPid);
void gaf_media_pid_adjust(uint8_t streamType, gaf_media_pid_t* pStreamPid, int32_t diff_time);
void gaf_media_pid_update_threshold(gaf_media_pid_t* pStreamPid, int32_t threshold_us);

#ifdef __cplusplus
}
#endif

#endif // GAF_MEDIA_PID_H_

/// @} GAF_MEDIA_PID
