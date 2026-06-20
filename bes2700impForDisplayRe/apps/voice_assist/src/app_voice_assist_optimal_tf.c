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
#include "app_voice_assist_optimal_tf.h"
#include "anc_optimal_tf.h"
#include "anc_assist.h"
#include "app_anc.h"
#include "audio_dump.h"
#include "heap_api.h"
#include "app_utils.h"
#include "hal_timer.h"
#include "hal_codec.h"

WEAK void hal_codec_set_echo_rate(enum HAL_CODEC_ECHO_RATE_T rate){

};

#define OPTIMAL_TF_SAMPLE_RATE (32000)

#if OPTIMAL_TF_SAMPLE_RATE == 48000
#define FILTER_LENGTH (360)
#elif OPTIMAL_TF_SAMPLE_RATE == 32000
#define FILTER_LENGTH (240)
#else
#define FILTER_LENGTH (120)
#endif

#define STAGE_NUM (10)
const static float faet_thd[STAGE_NUM-1] = {-10.75, -9.6, -8.45, -7.3, -6.15, -5.0, -3.85, -2.7, -1.55};

float Sz_estimate[FILTER_LENGTH] = {1.0,0.0};
// float Sz_estimate[FILTER_LENGTH] = {
// #include "static_Sz//hp_bus_Sz.txt"
// };
float Pz_estimate[FILTER_LENGTH] = {1.0,0.0};

#include "anc_process.h"
extern const struct_anc_cfg * anc_coef_list_50p7k[12];

typedef struct
{
    anc_optimal_tf_inst *anc_inst;
    int32_t mode;
    ANC_OPTIMAL_TF_STAGE last_stage;
    ANC_OPTIMAL_TF_STAGE stage;
    uint32_t frame_count;
    uint32_t pnc_total_frame;
    uint32_t switch_total_frame;
    uint32_t anc_total_frame;
} voice_assist_optimal_tf_inst;

static voice_assist_optimal_tf_inst ctx;

#if( OPTIMAL_TF_SAMPLE_RATE == 32000)
static void dump_Sz_Pz(float *Sz, float *Pz)
{
    int16_t dump_data[240] = {0};
    for (uint32_t i = 0; i < 240; i++) {
        dump_data[i] = 0x5554;
    }
    audio_dump_add_channel_data(0, dump_data, 240);

    for (uint32_t j = 0; j < 240; j++) {
        dump_data[j] = (int16_t)(*Sz++ * 32768 / 4);
    }
    audio_dump_add_channel_data(1, dump_data, 240);

    for (uint32_t j = 0; j < 240; j++) {
        dump_data[j] = (int16_t)(*Pz++ * 32768 / 4);
    }
    audio_dump_add_channel_data(2, dump_data, 240);
    audio_dump_run();
}
#else
static void dump_Sz_Pz(float *Sz, float *Pz)
{
    int16_t dump_data[120] = {0};
    for (uint32_t i = 0; i < 120; i++) {
        dump_data[i] = 0x5554;
    }
    audio_dump_add_channel_data(0, dump_data, 120);

    for (uint32_t j = 0; j < 120; j++) {
        dump_data[j] = (int16_t)(*Sz++ * 32768 / 4);
    }
    audio_dump_add_channel_data(1, dump_data, 120);

    for (uint32_t j = 0; j < 120; j++) {
        dump_data[j] = (int16_t)(*Pz++ * 32768 / 4);
    }
    audio_dump_add_channel_data(2, dump_data, 120);
}
#endif


voice_assist_optimal_tf_inst *voice_assist_get_ctx(void)
{
    return &ctx;
}

static int32_t _voice_assist_optimal_tf_anc_callback(void *buf, uint32_t len, void *other);

int32_t app_voice_assist_optimal_tf_anc_init(void)
{
    app_anc_assist_register(ANC_ASSIST_USER_OPTIMAL_TF_ANC, _voice_assist_optimal_tf_anc_callback);

    return 0;
}

int32_t app_voice_assist_optimal_tf_anc_open(int32_t mode)
{
    voice_assist_optimal_tf_inst *ctx = voice_assist_get_ctx();

    app_sysfreq_req(APP_SYSFREQ_USER_OPTIMAL_TF, APP_SYSFREQ_208M);

    syspool_init();

    med_heap_init(syspool_start_addr(), syspool_total_size());

    ctx->anc_inst = anc_optimal_tf_create(OPTIMAL_TF_SAMPLE_RATE, FILTER_LENGTH, 1, default_allocator());
    ctx->mode = mode;

    if (ctx->mode == 0) {
        // first run
        ctx->stage = ANC_OPTIMAL_TF_STAGE_IDLE;
    } else {
        anc_optimal_tf_set_Pz(ctx->anc_inst, Pz_estimate, ARRAY_SIZE(Pz_estimate));
        ctx->stage = ANC_OPTIMAL_TF_STAGE_WAITING_ANC_ON;
    }

    ctx->frame_count = 0;
    ctx->pnc_total_frame = 500 * 2 / 15;
    ctx->switch_total_frame = 700 * 2 / 15;
    ctx->anc_total_frame = 500 * 2 / 15;

    app_anc_assist_open(ANC_ASSIST_USER_OPTIMAL_TF_ANC);

    return 0;
}

int32_t app_voice_assist_optimal_tf_anc_close(void)
{
    voice_assist_optimal_tf_inst *ctx = voice_assist_get_ctx();

    anc_optimal_tf_get_Sz(ctx->anc_inst , Sz_estimate);
    anc_optimal_tf_get_Pz(ctx->anc_inst , Pz_estimate);
    dump_Sz_Pz(Sz_estimate, Pz_estimate);

    anc_optimal_tf_destroy(ctx->anc_inst);

    app_anc_assist_close(ANC_ASSIST_USER_OPTIMAL_TF_ANC);
    app_sysfreq_req(APP_SYSFREQ_USER_OPTIMAL_TF, APP_SYSFREQ_32K);

    return 0;
}

int32_t app_voice_assist_optimal_tf_choose_mode(void)
{
    voice_assist_optimal_tf_inst *ctx = voice_assist_get_ctx();
    int32_t best_mode = STAGE_NUM;

    float feat = anc_optimal_tf_get_TF_feature(ctx->anc_inst);

    for (int32_t i = 1; i < STAGE_NUM; i++){
        if(feat < faet_thd[i-1]){
            best_mode = i;
            break;
        }
    }
    // float dist = 0.0;
    // float dist_min = 100.0;
    // for (int32_t i = 0; i < STAGE_NUM; i++){
    //     dist = ABS(feat - faet_thd[i]);
    //     if (dist_min > dist){
    //         dist_min = dist;
    //         best_mode = i + 1;
    //     }
    // }

    TRACE(2, "[%s] ---------feat = %de-2 best_mode = mode %d--------", __FUNCTION__, (int)(feat * 100), best_mode);
    // if (best_mode != 1)
        // anc_set_cfg(anc_coef_list_50p7k[best_mode-1], ANC_TALKTHRU, ANC_GAIN_NO_DELAY);
    // app_anc_switch(best_mode);
    return 0;
}

static char *stage_desc[ANC_OPTIMAL_TF_STAGE_NUM] = {
    "IDLE",
    "PNC",
    "WAITING ANC ON",
    "ANC",
    "WAITING ANC OFF",
};

static int32_t _voice_assist_optimal_tf_anc_callback(void * buf, uint32_t len, void *other)
{
    float **input_data = buf;
    float *ff_data = input_data[0];  // error
    float *fb_data = input_data[1];  // error
    float *ref_data = input_data[2];

    voice_assist_optimal_tf_inst *ctx = voice_assist_get_ctx();

    // TRACE(0, "[%s] len = %d", __FUNCTION__, len);

    anc_optimal_tf_process(ctx->anc_inst, ff_data, fb_data, ref_data, len, ctx->stage);

    static POSSIBLY_UNUSED enum HAL_CODEC_ECHO_PATH_T path_def = HAL_CODEC_ECHO_PATH_QTY;

    ctx->frame_count += 1;

    if (ctx->stage == ANC_OPTIMAL_TF_STAGE_IDLE) {
        if (ctx->frame_count  < 15){
            return 0;
        }
        ctx->stage = ANC_OPTIMAL_TF_STAGE_PNC;
        ctx->frame_count = 0;
        path_def = hal_codec_get_echo_path();
        hal_codec_set_echo_rate(HAL_CODEC_ECHO_RATE_384K);
#if defined(CODEC_ECHO_PATH_VER) && (CODEC_ECHO_PATH_VER >= 2)
        hal_codec_set_echo_path(HAL_CODEC_ECHO_PATH_HBF4_DATA_IN);
#else
        hal_codec_set_echo_path(HAL_CODEC_ECHO_PATH_ALL);
#endif
    } else if (ctx->stage == ANC_OPTIMAL_TF_STAGE_PNC) {
        if (ctx->frame_count == ctx->pnc_total_frame) {
            app_anc_switch(APP_ANC_MODE1);
            ctx->stage = ANC_OPTIMAL_TF_STAGE_WAITING_ANC_ON;
            ctx->frame_count = 0;
        }
    } else if (ctx->stage == ANC_OPTIMAL_TF_STAGE_WAITING_ANC_ON) {
        if (ctx->frame_count == ctx->switch_total_frame) {
            ctx->stage = ANC_OPTIMAL_TF_STAGE_ANC;
            ctx->frame_count = 0;
        }
    } else if (ctx->stage == ANC_OPTIMAL_TF_STAGE_ANC) {
        if (ctx->frame_count == ctx->anc_total_frame) {
            ctx->stage = ANC_OPTIMAL_TF_STAGE_WAITING_ANC_OFF;
            if (ctx->mode == 0) {
                app_voice_assist_optimal_tf_choose_mode();
            }
            app_voice_assist_optimal_tf_anc_close();
            hal_codec_set_echo_path(path_def);
        }
    }

    if (ctx->frame_count == 0) {
        TRACE(2, "[%s] switch to stage %s", __FUNCTION__, stage_desc[ctx->stage]);
    }

    return 0;
}
