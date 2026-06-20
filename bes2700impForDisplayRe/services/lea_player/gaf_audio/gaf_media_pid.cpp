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
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#if BLE_AUDIO_ENABLED
#include "bluetooth_bt_api.h"
#include "app_bt_func.h"
#include "app_utils.h"
#include "audio_dump.h"
#include "audioflinger.h"
#include "cqueue.h"
#include "hal_aud.h"
#include "hal_trace.h"

#include "gaf_media_pid.h"

// Max allowed total tune ratio (5000ppm)
#define MAX_TOTAL_TUNE_RATIO        0.005000

void gaf_media_pid_init(gaf_media_pid_t* pStreamPid)
{
    memset(pStreamPid, 0, sizeof(gaf_media_pid_t));
    pStreamPid->Kp = 0.000018;
    pStreamPid->Ki = 0.000005;
    pStreamPid->Kd = 0.000008;
}

void gaf_media_pid_adjust(uint8_t streamType, gaf_media_pid_t* pStreamPid, int32_t diff_time)
{
    float result_P, result_I, result_D;

    pStreamPid->diff_time0 = diff_time;
    result_P = pStreamPid->Kp * (float)(pStreamPid->diff_time0 - pStreamPid->diff_time1);
    result_I = pStreamPid->Ki * (float)pStreamPid->diff_time0;
    result_D = pStreamPid->Kd * (float)(pStreamPid->diff_time0 - 2 * pStreamPid->diff_time1 + pStreamPid->diff_time2);
    pStreamPid->result += (result_P + result_I + result_D);
    if (pStreamPid->result > MAX_TOTAL_TUNE_RATIO)
    {
        pStreamPid->result = MAX_TOTAL_TUNE_RATIO;
    }
    else if (pStreamPid->result < -MAX_TOTAL_TUNE_RATIO)
    {
        pStreamPid->result = -MAX_TOTAL_TUNE_RATIO;
    }

    pStreamPid->diff_time2 = pStreamPid->diff_time1;
    pStreamPid->diff_time1 = pStreamPid->diff_time0;

    af_codec_tune((enum AUD_STREAM_T)streamType, pStreamPid->result);
}

void gaf_media_pid_update_threshold(gaf_media_pid_t* pStreamPid, int32_t threshold_us)
{
    pStreamPid->gap_threshold_us = threshold_us;
}
#endif

/// @} APP
