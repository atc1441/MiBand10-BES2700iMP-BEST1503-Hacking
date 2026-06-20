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
#include "hal_trace.h"
#include "app_anc_assist.h"
#include "anc_assist.h"
#include "anc_process.h"
#include "app_voice_assist_noise_adapt_anc.h"
#include "app_anc_utils.h"

 static int32_t _voice_assist_noise_adapt_anc_callback(void *buf, uint32_t len, void *other);

 int32_t app_voice_assist_noise_adapt_anc_init(void)
 {
     app_anc_assist_register(ANC_ASSIST_USER_NOISE_ADAPT_ANC, _voice_assist_noise_adapt_anc_callback);
     return 0;
 }

int32_t app_voice_assist_noise_adapt_anc_open(void)
{
    TRACE(0, "[%s] noise adapt anc start stream", __func__);
    app_anc_assist_open(ANC_ASSIST_USER_NOISE_ADAPT_ANC);
    app_anc_set_gain_f32(ANC_GAIN_USER_APP_ANC, ANC_FEEDBACK, 1, 1);
    app_anc_set_gain_f32(ANC_GAIN_USER_ANC_ASSIST_SYNC, ANC_FEEDBACK, 1, 1);
    return 0;
}


int32_t app_voice_assist_noise_adapt_anc_close(void)
{
    TRACE(0, "[%s] noise adapt anc close stream", __func__);
    app_anc_assist_close(ANC_ASSIST_USER_NOISE_ADAPT_ANC);
    app_anc_set_gain_f32(ANC_GAIN_USER_ANC_ASSIST_SYNC, ANC_FEEDBACK, 1, 1);
    return 0;
}

static int32_t ambient_cnt = 0;

static int32_t _voice_assist_noise_adapt_anc_callback(void * buf, uint32_t len, void *other)
{
    int32_t *res = (int32_t *)buf;
    noise_status_t noise_status = res[1];

    if (res[2] >= 150) {
        ambient_cnt++;
        if (ambient_cnt >= 10) {
            ambient_cnt = 0;
            TRACE(0, "The ambient noise is greater than 50dB!");
        }
    } else {
        ambient_cnt = 0;
    }
    
    // need to do tws sync
    // app_anc_set_gain_f32 0.5 == total gain -6dB, app_anc_set_gain_f32 0.25 == total gain -12dB
    if (res[0]) {
        if (noise_status == NOISE_STATUS_STRONG_ANC) {
        TRACE(4, "4444444444444444444444444444");
        } else if (noise_status == NOISE_STATUS_MIDDLE_ANC) {
            TRACE(3, "3333333333333333333333333333");
        } else if (noise_status == NOISE_STATUS_LOWER_ANC) {
            TRACE(2, "2222222222222222222222222222");
        } else if (noise_status == NOISE_STATUS_QUIET_ANC)  {
            TRACE(1, "1111111111111111111111111111");
        } else {
            TRACE(1, "0000000000000000000000000000");
        }
    }
    
    return 0;
}
