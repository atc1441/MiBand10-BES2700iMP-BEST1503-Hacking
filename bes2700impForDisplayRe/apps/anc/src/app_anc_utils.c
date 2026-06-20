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
#include "tgt_hardware.h"
#include "hal_aud.h"
#include "cmsis.h"
#include "app_anc_utils.h"
#include "app_anc_sync.h"
#include "anc_process.h"
#if defined(PSAP_APP)
#include "psap_process.h"
#endif

// ******************** COMMON ********************
enum ANC_CHANNEL_ID {
    ANC_CHANNEL_LEFT = 0,
    ANC_CHANNEL_RIGHT,
    ANC_CHANNEL_NUM
};

uint32_t anc_type_to_index(enum ANC_TYPE_T type)
{
    uint32_t index = 0;
    uint32_t num = 0;

    for (uint32_t i=0; i<ANC_TYPE_NUM; i++) {
        if (type & (0x01 << i)) {
            index = i;
            num++;
        }
    }

    ASSERT(num == 1, "[%s] type(0x%x) is invalid", __func__, type);

    return index;
}

// ******************** GAIN ********************
//  When switch anc mode, reset g_anc_gain
static float g_anc_gain[ANC_GAIN_USER_QTY][ANC_TYPE_NUM][ANC_CHANNEL_NUM];

//  When switch anc mode, don't reset g_anc_global_gain
static float g_anc_global_gain[ANC_TYPE_NUM][ANC_CHANNEL_NUM];

int32_t app_anc_utils_init(void)
{
    for (uint32_t i=0; i<ANC_TYPE_NUM; i++) {
        g_anc_global_gain[i][ANC_CHANNEL_LEFT] = 1.0;
        g_anc_global_gain[i][ANC_CHANNEL_RIGHT] = 1.0;
    }

    return 0;
}

int32_t app_anc_reset_gain(enum ANC_TYPE_T type, float gain_l, float gain_r)
{
    TRACE(0, "[%s] type: %d, gain_l: %d, gain_r: %d", __func__, anc_type_to_index(type), (int32_t)(gain_l * 100), (int32_t)(gain_r * 100));

    uint32_t lock = int_lock();
    uint32_t index = anc_type_to_index(type);
    for (uint32_t user=0; user<ANC_GAIN_USER_QTY; user++) {
        g_anc_gain[user][index][ANC_CHANNEL_LEFT] = gain_l;
        g_anc_gain[user][index][ANC_CHANNEL_RIGHT] = gain_r;
    }
    int_unlock(lock);

    return 0;
}

int32_t app_anc_dump_gain(void)
{
    TRACE(0, "#################### %s ####################", __func__);
    for (uint32_t index=0; index<ANC_TYPE_NUM; index++) {
        TRACE(0, "-------------------- Type: %d --------------------", index);
        for (uint32_t user=0; user<ANC_GAIN_USER_QTY; user++) {
            TRACE(0, "%d: %d,\t%d", user, (int32_t)(g_anc_gain[user][index][ANC_CHANNEL_LEFT] * 100), (int32_t)(g_anc_gain[user][index][ANC_CHANNEL_RIGHT] * 100));
        }
    }
    TRACE(0, "#################### END ####################");

    return 0;
}

int32_t app_anc_get_total_gain_f32(enum ANC_TYPE_T type, float *gain_l, float *gain_r)
{
    uint32_t lock = int_lock();
    *gain_l = 1.0;
    *gain_r = 1.0;
    
    uint32_t index = anc_type_to_index(type);
    for (uint32_t user=0; user<ANC_GAIN_USER_QTY; user++) {
        *gain_l *= g_anc_gain[user][index][ANC_CHANNEL_LEFT];
        *gain_r *= g_anc_gain[user][index][ANC_CHANNEL_RIGHT];
    }

    *gain_l *= g_anc_global_gain[index][ANC_CHANNEL_LEFT];
    *gain_r *= g_anc_global_gain[index][ANC_CHANNEL_RIGHT];
    int_unlock(lock);

    return 0;
}

static int32_t app_anc_update_gain_f32(enum ANC_TYPE_T type)
{
    float gain_l = 1.0;
    float gain_r = 1.0;

    app_anc_get_total_gain_f32(type, &gain_l, &gain_r);

    if (0) {
#if defined(PSAP_APP)
    } else if (type == PSAP_FEEDFORWARD) {
        psap_set_total_gain_f32(gain_l, gain_r);
#endif
    } else {
        anc_set_gain_f32(gain_l, gain_r, type);
    }

    // app_anc_dump_gain();

    return 0;
}

int32_t app_anc_enable_gain(enum ANC_TYPE_T type)
{
    app_anc_reset_gain(type, 1.0, 1.0);
    app_anc_update_gain_f32(type);

    return 0;
}

int32_t app_anc_disable_gain(enum ANC_TYPE_T type)
{
    app_anc_reset_gain(type, 0.0, 0.0);
    app_anc_update_gain_f32(type);

    return 0;
}

int32_t app_anc_set_gain_f32(anc_gain_user_t user, enum ANC_TYPE_T type, float gain_l, float gain_r)
{
    uint32_t lock = int_lock();
    uint32_t index = anc_type_to_index(type);
    g_anc_gain[user][index][ANC_CHANNEL_LEFT] = gain_l;
    g_anc_gain[user][index][ANC_CHANNEL_RIGHT] = gain_r;
    int_unlock(lock);

    app_anc_update_gain_f32(type);

    return 0;
}

int32_t app_anc_get_gain_f32(anc_gain_user_t user, enum ANC_TYPE_T type, float *gain_l, float *gain_r)
{
    uint32_t lock = int_lock();
    uint32_t index = anc_type_to_index(type);
    *gain_l = g_anc_gain[user][index][ANC_CHANNEL_LEFT];
    *gain_r = g_anc_gain[user][index][ANC_CHANNEL_RIGHT];
    int_unlock(lock);

    return 0;
}

int32_t app_anc_set_global_gain_f32(enum ANC_TYPE_T type, float gain_l, float gain_r)
{
    TRACE(3, "[%s] type: 0x%x, gain(x100): %d, %d", __func__, type, (int32_t)(gain_l * 100), (int32_t)(gain_r * 100));

    uint32_t lock = int_lock();
    uint32_t index = anc_type_to_index(type);
    g_anc_global_gain[index][ANC_CHANNEL_LEFT] = gain_l;
    g_anc_global_gain[index][ANC_CHANNEL_RIGHT] = gain_r;
    int_unlock(lock);

    app_anc_update_gain_f32(type);

    return 0;
}

int32_t app_anc_get_global_gain_f32(enum ANC_TYPE_T type, float *gain_l, float *gain_r)
{
    uint32_t lock = int_lock();
    uint32_t index = anc_type_to_index(type);
    *gain_l = g_anc_global_gain[index][ANC_CHANNEL_LEFT];
    *gain_r = g_anc_global_gain[index][ANC_CHANNEL_RIGHT];
    int_unlock(lock);

    return 0;
}
// ******************** SYNC ********************
int32_t app_anc_sync_set_psap_bands_gain(float gain_l, float gain_r, uint32_t index_start, uint32_t index_end)
{
    app_anc_sync_psap_bands_gain(gain_l, gain_r, index_start, index_end);
#if defined(PSAP_APP)
    psap_set_bands_same_gain_f32(gain_l, gain_r, index_start, index_end);
#endif

    return 0;
}