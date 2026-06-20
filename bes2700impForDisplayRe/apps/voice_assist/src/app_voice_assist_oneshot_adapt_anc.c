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
#ifdef VOICE_ASSIST_ONESHOT_ADAPTIVE_ANC
#include "hal_trace.h"
#include "app_anc_assist.h"
#include "app_voice_assist_oneshot_adapt_anc.h"
#include "anc_select_mode.h"
#include "anc_assist.h"

#include "aud_section.h"
#include "anc_process.h"
#include "arm_math.h"
#include "audio_dump.h"
#include "app_utils.h"
#include "heap_api.h"


// #define ONESHOT_ADAPT_ANC_DUMP

#ifdef ONESHOT_ADAPT_ANC_DUMP
static short tmp_data[120];
#endif

static float local_denoise = 0;
static int32_t g_best_mode_index;
static int32_t g_select_gain = 512;
static int32_t g_function = 0;

// local anc settings for adaptive anc
static const struct_anc_cfg POSSIBLY_UNUSED AncFirCoef_50p7k_mode0;

struct_anc_cfg const * POSSIBLY_UNUSED anc_mode_list[4] = {
    &AncFirCoef_50p7k_mode0,
    &AncFirCoef_50p7k_mode0,
    &AncFirCoef_50p7k_mode0,
    &AncFirCoef_50p7k_mode0,
};

AncSelectModeState * anc_select_mode_st = NULL;
SELECT_MODE_CFG_T anc_select_mode_cfg = {
        .sample_rate = 16000, // sample rate, same as anc_assist_module
        .frame_size = 120, // frame size, same as anc_assist_modult

        .process_debug = 1,  // show debug log for process
        .snr_debug = 0,   // not show debug lof for snr
        .snr_on = 0,  // set snr function not affect procedure

        .wait_period = 50, // wait period for change anc, here is about 30*7.5 ms
        .test_period = 90, // time for whole test, total time is 80*7.5ms

        .mode_test_num = 4, // the upper bound for gain adapt, pnc_mode_list[49]

        .fb_power_thd = 0, // the fb energy lower bound for mode adapt
        .ff_power_thd = 0, // the fb energy upper bound for mode adapt

        .analysis_freq_upper = 800, 
        .analysis_freq_lower = 200,

        .gain_freq_upper = 600,
        .gain_freq_lower = 100,

        .min_db_diff = 0, // the min diff for adaption, avoid the mistaken from in-ear detection
        
};

static int32_t _voice_assist_oneshot_adapt_anc_callback(void *buf, uint32_t len, void *other);
static enum ONESHOT_ADAPTIVE_ANC_STATE oneshot_adaptive_anc_state = ONESHOT_ADAPTIVE_ANC_STATE_RUNNING;
int32_t app_voice_assist_oneshot_adapt_anc_init(void)
{
    oneshot_adaptive_anc_state = ONESHOT_ADAPTIVE_ANC_STATE_RUNNING;
    app_anc_assist_register(ANC_ASSIST_USER_ONESHOT_ADAPT_ANC, _voice_assist_oneshot_adapt_anc_callback);
    
    //set when anc init, not here， just example here
    anc_set_switching_delay(ANC_SWITCHING_DELAY_50ms,ANC_FEEDFORWARD);
    anc_set_switching_delay(ANC_SWITCHING_DELAY_50ms,ANC_FEEDBACK);
    return 0;
}

#define ADAPT_ANC_HEAP_SIZE (37*1024)
static uint8_t heap_buf[ADAPT_ANC_HEAP_SIZE];
int32_t app_voice_assist_oneshot_adapt_anc_open(int32_t mode)     // mode 1: select filter, mode 2: select gain
{
    TRACE(0, "[%s] oneshot adapt anc start stream", __func__);

    app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_208M);

    //create st
    AncSelectModeState * tmp_st = anc_select_mode_create(&anc_select_mode_cfg, mode, heap_buf, ADAPT_ANC_HEAP_SIZE);

    oneshot_adaptive_anc_state = ONESHOT_ADAPTIVE_ANC_STATE_RUNNING;
    app_anc_assist_open(ANC_ASSIST_USER_ONESHOT_ADAPT_ANC);
    local_denoise = 0;
    g_function = mode;
    g_select_gain = 512;

    if(tmp_st != NULL){
        anc_select_mode_st = tmp_st;
    }

    // close fb anc for adaptive anc, it is better not to open it during the init state
    TRACE(2,"[adapt_anc_action create] change mode %d ",0);
    if (g_function == 1) {
        anc_set_cfg(anc_mode_list[0], ANC_FEEDFORWARD, ANC_GAIN_NO_DELAY);
    }
    anc_set_gain(0,0,ANC_FEEDBACK);
#ifdef ONESHOT_ADAPT_ANC_DUMP
    audio_dump_init(120,sizeof(short),2);
#endif
    return 0;
}

int32_t app_voice_assist_oneshot_adapt_anc_close(void)
{
    oneshot_adaptive_anc_state = ONESHOT_ADAPTIVE_ANC_STATE_STOP;
    TRACE(0, "[%s] oneshot adapt anc close stream", __func__);
    anc_set_gain(512, 512, ANC_FEEDBACK);
#ifdef ONESHOT_ADAPT_ANC_DUMP
    audio_dump_deinit();
#endif
    return 0;
}


static int32_t _voice_assist_oneshot_adapt_anc_callback(void * buf, uint32_t len, void *other)
{
    if(oneshot_adaptive_anc_state == ONESHOT_ADAPTIVE_ANC_STATE_STOP){
        TRACE(2,"[%s] stop in callback func with state %d",__func__,oneshot_adaptive_anc_state);
        app_anc_assist_close(ANC_ASSIST_USER_ONESHOT_ADAPT_ANC);
        anc_select_mode_destroy(anc_select_mode_st);
        // app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_32K);
        oneshot_adaptive_anc_state = ONESHOT_ADAPTIVE_ANC_STATE_POST_STOP;
        return 0;
    } else if(oneshot_adaptive_anc_state == ONESHOT_ADAPTIVE_ANC_STATE_POST_STOP){
        TRACE(2,"[%s] stop in callback func with state %d",__func__,oneshot_adaptive_anc_state);
        return 0;
    }
    // deal with input values
    float ** input_data = buf;
    float * ff_data = input_data[0];
    float * fb_data = input_data[1];
    AncAssistRes * assist_res = (AncAssistRes *)other;
    int wind_status = 0; // will not pause algo
    if(assist_res->wind_status > 1){ // check wind status enum, this algo not work when bigger than small wind
        wind_status = 0; // will pause algo
    }

#ifdef ONESHOT_ADAPT_ANC_DUMP
    audio_dump_clear_up();
    for(int i = 0; i< len; i++){
        tmp_data[i] = (short)(ff_data[i]/256);
    }
    audio_dump_add_channel_data(0,tmp_data,len);
    for(int i = 0; i< len; i++){
        tmp_data[i] = (short)(fb_data[i]/256);
    }
    audio_dump_add_channel_data(1,tmp_data,len); 
    audio_dump_run();
#endif    

    for(int i = 0; i< len; i++){
        ff_data[i] = ff_data[i]/ 256;
        fb_data[i] = fb_data[i]/ 256;
    }

    SELECT_MODE_RESULT_T res = anc_select_process(anc_select_mode_st,ff_data,fb_data,len,wind_status);
    // TRACE(2,"res state: current_mode : %d denoise_score: %d best_mode: %d",res.current_mode,(int)(100*res.denoise_score),res.best_mode);
    // TRACE(2,"res state: snr: %d fb: %d ff: %d min_diff: %d",res.snr_stop_flag,res.fb_pwr_stop_flag,res.ff_pwr_stop_flag,res.min_diff_stop_flag);
    if(res.snr_stop_flag == 1 || res.fb_pwr_stop_flag == 1 || res.ff_pwr_stop_flag == 1 || res.min_diff_stop_flag == 1){
        // if you want to stop when you meet wrong condition
        if (g_function == 2) {
            anc_set_gain(512, 512, ANC_FEEDFORWARD);
        }
        app_voice_assist_oneshot_adapt_anc_close();
        return 0;
    } 

    // keep running to the end
    if (res.current_mode > 0 && res.current_mode < anc_select_mode_cfg.mode_test_num && g_function == 1) { 
        TRACE(2,"[adapt_anc_action] change mode %d ", res.current_mode);
        anc_set_cfg(anc_mode_list[res.current_mode], ANC_FEEDFORWARD, ANC_GAIN_NO_DELAY);
    }

    if (res.current_mode == 1  && g_function == 2) {
        local_denoise = res.denoise_score;
        if (res.denoise_score > -20) {
            g_select_gain += 64;
            TRACE(2,"[adapt_gain_action] change gain add 1dB");
        } else {
            g_select_gain += 32;
            TRACE(2,"[adapt_gain_action] change gain add 0.5dB");
        }
        anc_set_gain(g_select_gain, g_select_gain, ANC_FEEDFORWARD);
    } else if (res.current_mode > 1  && g_function == 2) {
        if (local_denoise > res.denoise_score + 5) {
            g_select_gain -= 32;
            TRACE(2,"[adapt_gain_action] change gain minus 0.5dB");
        } else if (local_denoise < res.denoise_score - 5) {
            g_select_gain += 32;
            TRACE(2,"[adapt_gain_action] change gain add 0.5dB");
        }
        if (g_select_gain > 406 && g_select_gain < 682) {
            anc_set_gain(g_select_gain, g_select_gain, ANC_FEEDFORWARD);
            TRACE(2,"[adapt_gain_action] change gain to %d", g_select_gain);
        }
    }

    if (res.best_mode >= 0 && res.normal_stop_flag == 1) {
        if (g_function == 1) {
            TRACE(2,"[adapt_anc_action] test stop, change mode %d", res.best_mode);
            anc_set_cfg(anc_mode_list[res.best_mode], ANC_FEEDFORWARD, ANC_GAIN_NO_DELAY);
            g_best_mode_index = res.best_mode;
        } else {
            TRACE(2, "[adapt_gain_action] test stop, final gain %d", g_select_gain);
        }
        
        app_voice_assist_oneshot_adapt_anc_close();
    }

    return 0;
}
#endif