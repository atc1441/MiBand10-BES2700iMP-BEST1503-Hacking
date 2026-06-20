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
#include "plat_types.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "cmsis_os.h"
#include "anc_process.h"
#include "app_anc_utils.h"

#define ANC_FADE_MS         (10)
#define ANC_FADE_CNT        (512)

#if !defined(ANC_IIR_LINEAR_SMOOTH_MODE)
static void anc_set_fade_gain(uint32_t types, float gain)
{
    enum ANC_TYPE_T type = ANC_NOTYPE;

    for (uint32_t index=0; index<ANC_TYPE_NUM; index++) {
        type = types & (0x01 << index);
        if (type) {
            app_anc_set_gain_f32(ANC_GAIN_USER_APP_ANC_FADE, type, gain, gain);
        }
    }
}

static void anc_fadein_impl(uint32_t types)
{
    float gain = 0.0;
    float gain_step = 1.0 / ANC_FADE_CNT;
    uint32_t delay_us = (ANC_FADE_MS * 1000) / ANC_FADE_CNT;

    TRACE(0, "[%s] types: 0x%x", __func__, types);

    // Reset all user gain to 1.0
    for (uint32_t index=0; index<ANC_TYPE_NUM; index++) {
        app_anc_reset_gain(0x01 << index, 1.0, 1.0);
    }

    for (uint32_t cnt=0; cnt<ANC_FADE_CNT; cnt++) {
        gain += gain_step;
        anc_set_fade_gain(types, gain);
        hal_sys_timer_delay_us(delay_us);
    }

    anc_set_fade_gain(types, 1.0);

    TRACE(1, "[%s] OK", __func__);
}

static void anc_fadeout_impl(uint32_t types)
{
    float gain = 1.0;
    float gain_step = 1.0 / ANC_FADE_CNT;
    uint32_t delay_us = (ANC_FADE_MS * 1000) / ANC_FADE_CNT;

    TRACE(0, "[%s] types: 0x%x", __func__, types);

    for (uint32_t cnt=0; cnt<ANC_FADE_CNT; cnt++) {
        gain -= gain_step;
        anc_set_fade_gain(types, gain);
        hal_sys_timer_delay_us(delay_us);
    }

    anc_set_fade_gain(types, 0.0);

    TRACE(1, "[%s] OK", __func__);
}
#endif

int32_t app_anc_fade_init(void)
{
    TRACE(0, "[%s] ...", __func__);

    return 0;
}

int32_t app_anc_fade_deinit(void)
{
    TRACE(0, "[%s] ...", __func__);

    return 0;
}

int32_t app_anc_fadein(uint32_t types)
{
    TRACE(0, "[%s] types: 0x%0x, APP_ANC_FADE_IN ...", __func__, types);

#if defined(ANC_IIR_LINEAR_SMOOTH_MODE)
    int i = 0;
    while (types) {
        if (types & 0x1) {
            app_anc_enable_gain(1 << i);
        }
        i++;
        types >>= 1;
    }
#else
#if defined(PSAP_APP)
    if (types & PSAP_FEEDFORWARD) {
        app_anc_enable_gain(PSAP_FEEDFORWARD);
        types &= ~PSAP_FEEDFORWARD;
    }
#endif

    anc_fadein_impl(types);
#endif

    return 0;
}

int32_t app_anc_fadeout(uint32_t types)
{
    TRACE(0, "[%s] types: 0x%0x, APP_ANC_FADE_OUT ...", __func__, types);

#if defined(ANC_IIR_LINEAR_SMOOTH_MODE)
    int i = 0;
    while (types) {
        if (types & 0x1) {
            app_anc_disable_gain(1 << i);
        }
        i++;
        types >>= 1;
    }
#else
#if defined(PSAP_APP)
    if (types & PSAP_FEEDFORWARD) {
        app_anc_disable_gain(PSAP_FEEDFORWARD);
        types &= ~PSAP_FEEDFORWARD;
    }
#endif

    anc_fadeout_impl(types);
#endif

    return 0;
}
