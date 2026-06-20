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
#include "hal_trace.h"
#include "app_anc_assist.h"
#include "anc_assist.h"
#include "app_voice_assist_noise_classify_adapt_anc.h"

static int32_t POSSIBLY_UNUSED noise_classify_status[MAX_FF_CHANNEL_NUM];
static int32_t g_status = 2;

static int32_t _voice_assist_noise_classify_adapt_anc_callback(void *buf, uint32_t len, void *other);

int32_t app_voice_assist_noise_classify_adapt_anc_init(void)
{
    app_anc_assist_register(ANC_ASSIST_USER_NOISE_CLASSIFY_ADAPT_ANC, _voice_assist_noise_classify_adapt_anc_callback);
    return 0;
}

int32_t app_voice_assist_noise_classify_adapt_anc_open(void)
{
    TRACE(0, "[%s] noise adapt anc start stream", __func__);
    app_anc_assist_open(ANC_ASSIST_USER_NOISE_CLASSIFY_ADAPT_ANC);
    return 0;
}


int32_t app_voice_assist_noise_classify_adapt_anc_close(void)
{
    TRACE(0, "[%s] noise adapt anc close stream", __func__);
    app_anc_assist_close(ANC_ASSIST_USER_NOISE_CLASSIFY_ADAPT_ANC);
    return 0;
}


static int32_t _voice_assist_noise_classify_adapt_anc_callback(void * buf, uint32_t len, void *other)
{
    int32_t *res = (int32_t *)buf;
    int32_t tmp_status;
#if defined(FREEMAN_ENABLED_STERO)
    for (uint8_t i = 0, j = 0; i < len; i += 2, j++) {
        noise_classify_status[j] = res[i + 1];
        // TRACE(0, "mic %d: noise status = %d", j, noise_classify_status[j]);
    }
    tmp_status = MAX(noise_classify_status[0], noise_classify_status[1]);
#else
    tmp_status = res[1];
#endif
    if (g_status != tmp_status) {
        g_status = tmp_status;
        if (g_status == NOISE_STATUS_PLANE_ANC) {
            TRACE(4, "NOISE_STATUS_PLANE_ANC");
        } else if (g_status == NOISE_STATUS_TRANSPORT_ANC) {
            TRACE(3, "NOISE_STATUS_TRANSPORT_ANC");
        } else if (g_status == NOISE_STATUS_OUTDOOR_ANC) {
            TRACE(2, "NOISE_STATUS_OUTDOOR_ANC");
        } else if (g_status == NOISE_STATUS_INDOOR_ANC) {
            TRACE(1, "NOISE_STATUS_INDOOR_ANC");
        }
    }
    return 0;
}
