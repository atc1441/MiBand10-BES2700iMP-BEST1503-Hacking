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
#include "stdio.h"
#include "hal_trace.h"
#include "hal_aud.h"
#include "plat_types.h"
#include "string.h"
#include "audio_test_defs.h"
#include "audio_test_cmd.h"
#include "app_trace_rx.h"
#include "app_key.h"
#include "bluetooth_bt_api.h"
#include "app_media_player.h"
#if defined(ANC_APP)
#include "app_anc.h"
#include "app_anc_utils.h"
#endif

#define AUDIO_TEST_LOG_I(str, ...)      TR_INFO(TR_MOD(TEST), "[AUDIO_TEST]" str, ##__VA_ARGS__)

/**
 * Usage:
 *  1. CMD Format: e.g. [audio_test,anc_switch]
 **/

typedef void (*func_handler_t)(const char *cmd);

typedef struct {
    const char *name;
    func_handler_t handler;
} audio_test_func_t;

extern void app_debug_tool_printf(const char *fmt, ...);

static void audio_test_anc_switch(const char *cmd)
{
    AUDIO_TEST_LOG_I("[%s] cmd len: %d", __func__, strlen(cmd) - 2);

#if defined(ANC_APP)
    if (strlen(cmd) > 2) {
        uint32_t mode = 0;
        sscanf(cmd, "%d", &mode);

        AUDIO_TEST_LOG_I("[%s] mode: %d", __func__, mode);
        ASSERT(mode < APP_ANC_MODE_QTY, "[%s] mode is invalid: %d", __func__, mode);
        app_anc_switch(mode);
    } else {
        app_anc_loop_switch();
    }
#endif
}

#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_CP__) || defined(__VIRTUAL_SURROUND_STEREO__)
extern int32_t audio_process_stereo_surround_onoff(int32_t onoff);
extern int32_t audio_process_stereo_surround_onoff(int32_t onoff);
int open_flag = 0;
int32_t stereo_surround_status = 0;
#include "app_bt_stream.h"
#endif

static void audio_test_virtual_surround_on(const char *cmd){
#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_CP__) || defined(__VIRTUAL_SURROUND_STEREO__)
    open_flag = 1;
    // app_anc_switch(open_flag);
    stereo_surround_status = open_flag;
    audio_process_stereo_surround_onoff(open_flag);
#endif
}

static void audio_test_virtual_surround_off(const char *cmd){
#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_CP__) || defined(__VIRTUAL_SURROUND_STEREO__)
    open_flag = 0;
    // app_anc_switch(open_flag);
    stereo_surround_status = open_flag;
    audio_process_stereo_surround_onoff(open_flag);
#endif
}

static void audio_test_anc_gain_switch(const char *cmd)
{
    AUDIO_TEST_LOG_I("[%s] ...", __func__);

#if defined(ANC_APP)
    static bool flag = false;
    if (flag) {
        app_anc_set_gain_f32(ANC_GAIN_USER_TUNING, PSAP_FEEDFORWARD, 1.0, 1.0);
    } else {
        app_anc_set_gain_f32(ANC_GAIN_USER_TUNING, PSAP_FEEDFORWARD, 0.0, 0.0);
    }

    flag = !flag;
#endif
}

extern int32_t audio_test_stream(void);
static void audio_test_stream_switch(const char *cmd)
{
    AUDIO_TEST_LOG_I("[%s] ...", __func__);

#if defined(AUDIO_TEST_STREAM)
    audio_test_stream();
#endif
}

static void audio_test_karaoke_switch(const char *cmd)
{
    AUDIO_TEST_LOG_I("[%s] ...", __func__);

#ifdef A2DP_KARAOKE
    static bool stream_status = true;
    extern int32_t app_karaoke_start(bool on);
    app_karaoke_start(stream_status);
    stream_status = !stream_status;
#endif
}

static void audio_test_switch_app_mcpp(const char *cmd)
{
    AUDIO_TEST_LOG_I("[%s] %s", __func__, cmd);

#ifdef APP_MCPP_CLI
    extern int32_t audio_test_app_mcpp(const char *cmd);
    audio_test_app_mcpp(cmd);
#else
    AUDIO_TEST_LOG_I("please define APP_MCPP_CLI first!!!");
#endif
}

#if defined(ANC_ASSIST_ENABLED)
#include "app_anc_assist.h"
#endif
static void audio_test_anc_assist_switch(const char *cmd)
{
    AUDIO_TEST_LOG_I("[%s] ...", __func__);
#if defined(ANC_ASSIST_ENABLED)
    static bool flag = true;
    if (flag) {
        app_anc_assist_open(ANC_ASSIST_USER_ANC);
    } else {
        app_anc_assist_close(ANC_ASSIST_USER_ANC);
    }

    flag = !flag;
#endif
}

static void audio_test_assist_noise_switch(const char *cmd)
{
    AUDIO_TEST_LOG_I("[%s] ...", __func__);
#if defined(ANC_ASSIST_ENABLED) && defined(VOICE_ASSIST_NOISE)
    static bool flag = true;
    if (flag) {
        app_anc_assist_open(ANC_ASSIST_USER_NOISE_ADAPT_ANC);
    } else {
        app_anc_assist_close(ANC_ASSIST_USER_NOISE_ADAPT_ANC);
    }

    flag = !flag;
#endif
}

static void audio_test_assist_noise_classify_switch(const char *cmd)
{
    AUDIO_TEST_LOG_I("[%s] ...", __func__);
#if defined(ANC_ASSIST_ENABLED) && defined(VOICE_ASSIST_NOISE_CLASSIFY)
    static bool flag = true;
    if (flag) {
        app_anc_assist_open(ANC_ASSIST_USER_NOISE_CLASSIFY_ADAPT_ANC);
    } else {
        app_anc_assist_close(ANC_ASSIST_USER_NOISE_CLASSIFY_ADAPT_ANC);
    }

    flag = !flag;
#endif
}

#if defined(ANC_ASSIST_ENABLED)
#include "app_voice_assist_ai_voice.h"
#include "audioflinger.h"
#endif

static void audio_test_kws_switch(const char *cmd)
{
    AUDIO_TEST_LOG_I("[%s] ...", __func__);
#if defined(ANC_ASSIST_ENABLED)

    struct AF_STREAM_CONFIG_T kws_stream_cfg;
    kws_stream_cfg.sample_rate    = AUD_SAMPRATE_16000;
    kws_stream_cfg.channel_num    = AUD_CHANNEL_NUM_1;
    kws_stream_cfg.bits           = AUD_BITS_16;
    kws_stream_cfg.device         = AUD_STREAM_USE_INT_CODEC;
    kws_stream_cfg.io_path        = AUD_INPUT_PATH_ASRMIC;
    kws_stream_cfg.vol            = 12;
    kws_stream_cfg.handler        = NULL;
    kws_stream_cfg.data_ptr       = NULL;
    kws_stream_cfg.data_size      = 0;
    TRACE(0,"111");
    static bool flag = true;
    if (flag) {
        // FIXME: Create a stream_cfg
        app_voice_assist_ai_voice_open(ASSIST_AI_VOICE_MODE_KWS, &kws_stream_cfg, NULL);
    } else {
        app_voice_assist_ai_voice_close();
    }

    flag = !flag;
#endif
}

#if defined(ANC_ASSIST_ENABLED)
int32_t app_voice_assist_wd_open(void);
int32_t app_voice_assist_wd_close(void);
#endif
static void audio_test_wd_switch(const char *cmd)
{
    AUDIO_TEST_LOG_I("[%s] ...", __func__);
#if defined(ANC_ASSIST_ENABLED)
    static bool flag = true;
    if (flag) {
        app_voice_assist_wd_open();
    } else {
        app_voice_assist_wd_close();
    }

    flag = !flag;
#endif
}

#if defined(ANC_ASSIST_ENABLED)
int32_t app_voice_assist_optimal_tf_anc_open(void);
int32_t app_voice_assist_optimal_tf_anc_close(void);
#endif

static void audio_test_optimal_tf_switch(const char *cmd)
{
    AUDIO_TEST_LOG_I("[%s] ...", __func__);
#if defined(ANC_ASSIST_ENABLED)
    app_voice_assist_optimal_tf_anc_open();
#endif
}

#if defined(SPEECH_BONE_SENSOR)
#include "speech_bone_sensor.h"
#endif
static void audio_test_bone_sensor_switch(const char *cmd)
{
    AUDIO_TEST_LOG_I("[%s] ...", __func__);
#if defined(SPEECH_BONE_SENSOR)
    static bool flag = true;
    if (flag) {
        speech_bone_sensor_open(16000, 120);
        speech_bone_sensor_start();
    } else {
        speech_bone_sensor_stop();
        speech_bone_sensor_close();
    }

    flag = !flag;
#endif
}

static void audio_test_prompt_switch(const char *cmd)
{
    AUDIO_TEST_LOG_I("[%s] ...", __func__);
#if 1//defined(ANC_ASSIST_ENABLED)
    int prompt_buf[6] = {14, 15, 16, 17, 18, 19};
    static int prompt_cnt = 0;

    AUD_ID_ENUM id_tmp = (AUD_ID_ENUM)prompt_buf[prompt_cnt];
    TRACE(1,"[%s],id = 0x%x",__func__,(int)id_tmp);
    media_PlayAudio(id_tmp, 0);

    prompt_cnt++;

    if(6<= prompt_cnt)
        prompt_cnt = 0;

#endif
}

#if defined(ANC_ASSIST_ENABLED)
int32_t app_voice_assist_pnc_adapt_anc_open(void);
int32_t app_voice_assist_pnc_adapt_anc_close(void);
#endif

static void audio_test_pnc_adapt_switch(const char *cmd)
{
    AUDIO_TEST_LOG_I("[%s] ...", __func__);
#if defined(ANC_ASSIST_ENABLED)
    static bool flag = false;
    if (flag == false) {
        app_voice_assist_pnc_adapt_anc_open();
    } else {
        app_voice_assist_pnc_adapt_anc_close();
    }

    flag = !flag;
#endif
}

#if defined(ANC_ASSIST_ENABLED)
int32_t app_voice_assist_optimal_tf_anc_open(void);
int32_t app_voice_assist_optimal_tf_anc_close(void);
#endif

extern void switch_virtual_filt(uint32_t index);
static void audio_test_lms_vir(const char *cmd)
{
    AUDIO_TEST_LOG_I("[%s] ...", __func__);
    uint32_t ch = 0;
    sscanf(cmd, "%d", &ch);
#ifdef VOICE_ASSIST_FF_FIR_LMS
    switch_virtual_filt(ch);
#endif
}

#ifdef VOICE_ASSIST_FF_FIR_LMS
#include "app_voice_assist_fir_lms.h"
#endif
static void audio_test_fir(const char *cmd)
{
    AUDIO_TEST_LOG_I("[%s] ...", __func__);
#ifdef VOICE_ASSIST_FF_FIR_LMS
    uint32_t mode = 0;
    sscanf(cmd, "%d", &mode);

    AUDIO_TEST_LOG_I("[%s] mode: %d", __func__, mode);
    ASSERT(mode < APP_ANC_MODE_QTY, "[%s] mode is invalid: %d", __func__, mode);

    app_voice_assist_fir_lms_switch(mode);
#endif
}

#ifdef VOICE_ASSIST_ADA_IIR
#include "app_voice_assist_ada_iir.h"
#endif
static void audio_test_ada_iir(const char *cmd)
{
    AUDIO_TEST_LOG_I("[%s] ...", __func__);
#ifdef VOICE_ASSIST_ADA_IIR
    static int cnt = 0;
    switch (cnt)
    {
    case 0:
        app_voice_assist_ada_iir_open();
        cnt = 1;
        break;
    case 1:
        app_voice_assist_ada_iir_close();
        cnt = 0;
    default:
        break;
    }
#endif
}

extern void set_fir_lms_step(uint32_t step);
static void audio_test_set_lms_step(const char *cmd)
{
    AUDIO_TEST_LOG_I("[%s] ...", __func__);
    uint32_t ch = 0;
    sscanf(cmd, "%d", &ch);
#ifdef VOICE_ASSIST_FF_FIR_LMS
    set_fir_lms_step(ch);
#endif
}

extern int32_t bt_sco_chain_bypass_tx_algo(uint32_t sel_ch);
static void audio_test_bypass_tx_algo(const char *cmd)
{
    AUDIO_TEST_LOG_I("[%s] %s", __func__, cmd);

    uint32_t ch = 0;
    sscanf(cmd, "ch=%d", &ch);

    AUDIO_TEST_LOG_I("[%s] ch: %d", __func__, ch);
    bt_sco_chain_bypass_tx_algo(ch);
}

static void audio_test_input_param(const char *cmd)
{
    AUDIO_TEST_LOG_I("[%s] %s", __func__, cmd);

    int32_t val1 = 0;
    int32_t val2 = 0;
    sscanf(cmd, "val1=%d val2=%d", &val1, &val2);

    AUDIO_TEST_LOG_I("[%s] val1: %d, val2: %d", __func__, val1, val2);
}

static void audio_test_call_algo_cfg(const char *cmd)
{
    AUDIO_TEST_LOG_I("[%s] %s", __func__, cmd);

}

static void audio_test_hifi(const char *cmd)
{
    AUDIO_TEST_LOG_I("[%s] %s", __func__, cmd);

#if defined(AUDIO_TEST_HIFI)
    extern int32_t audio_test_hifi_tx_data(void);
    audio_test_hifi_tx_data();
#endif
}

#ifdef __AUDIO_DYNAMIC_BOOST__
extern int switch_cfg(void);
#endif

static void audio_test_dynamic_boost(const char *cmd)
{
    AUDIO_TEST_LOG_I("[%s] ...", __func__);

#ifdef __AUDIO_DYNAMIC_BOOST__
    switch_cfg();
#endif
}

extern void audio_virtual_bass_switch(int val);
static void audio_test_virtual_bass_switch(const char *cmd)
{
#ifdef AUDIO_BASS_ENHANCER
    if (!strcmp(cmd, "on")) {
        audio_virtual_bass_switch(1);
        AUDIO_TEST_LOG_I("[%s]:open", __FUNCTION__);
    } else if (!strcmp(cmd, "off")) {
        audio_virtual_bass_switch(0);
        AUDIO_TEST_LOG_I("[%s]:close", __FUNCTION__);
    } else {
        AUDIO_TEST_LOG_I("invalid cmd: %s", cmd);
        AUDIO_TEST_LOG_I("cmd usage: audio_test:virtual_bass on");
        AUDIO_TEST_LOG_I("cmd usage: audio_test:virtual_bass off");
    }
#endif
}

const audio_test_func_t audio_test_func[]= {
    {"anc_switch",          audio_test_anc_switch},
    {"stream_switch",       audio_test_stream_switch},
    {"karaoke_switch",      audio_test_karaoke_switch},
    {"anc_assist_switch",   audio_test_anc_assist_switch},
    {"kws_switch",          audio_test_kws_switch},
    {"wd_switch",           audio_test_wd_switch},
    {"anc_gain_switch",     audio_test_anc_gain_switch},
    {"bone_sensor_switch",  audio_test_bone_sensor_switch},
    {"prompt_switch",       audio_test_prompt_switch},
    {"bypass_tx_algo",      audio_test_bypass_tx_algo},
    {"input_param",         audio_test_input_param},
    {"call_algo_cfg",       audio_test_call_algo_cfg},
    {"hifi",                audio_test_hifi},
    {"app_mcpp",            audio_test_switch_app_mcpp},
    {"virtual_surround_on",    audio_test_virtual_surround_on},
    {"virtual_surround_off",    audio_test_virtual_surround_off},
    {"dynamic_bass_boost",  audio_test_dynamic_boost},
    {"virtual_bass",        audio_test_virtual_bass_switch},
    {"optimal_tf_switch",   audio_test_optimal_tf_switch},
    {"voice_assist_noise_switch",   audio_test_assist_noise_switch},
    {"voice_assist_noise_classify_switch",   audio_test_assist_noise_classify_switch},
    {"pnc_adapt_switch",    audio_test_pnc_adapt_switch},
    {"lms_virt_switch",     audio_test_lms_vir},
    {"fir_open_switch",     audio_test_fir},
    {"iir_switch",     audio_test_ada_iir},
    {"set_lms_step_switch", audio_test_set_lms_step},
};

static uint32_t audio_test_cmd_callback(uint8_t *buf, uint32_t len)
{
    uint32_t index = 0;
    char *cmd = (char *)buf;
    func_handler_t func_handler = NULL;

    // filter ' ' before function
    for (uint32_t i=0; i<strlen(cmd); i++) {
        if (cmd[i] != ' ') {
            cmd += i;
            break;
        }
    }

    // Separate function and value with ' '
    for (index=0; index<strlen(cmd); index++) {
        if (cmd[index] == ' ' || cmd[index] == '\r' || cmd[index] == '\n') {
            break;
        }
    }

    for (uint8_t i = 0; i < ARRAY_SIZE(audio_test_func); i++) {
        if (strncmp((char *)cmd, audio_test_func[i].name, index) == 0) {
            func_handler = audio_test_func[i].handler;
            break;
        }
    }

    // filter ' ' before value
    for (; index<strlen(cmd); index++) {
        if (cmd[index] != ' ') {
            break;
        }
    }

    if (strncmp(cmd + index, "bin ", strlen("bin ")) == 0) {
        index += strlen("bin ");
    }

    if (func_handler) {
        func_handler(cmd + index);
        AUDIO_TEST_LOG_I("[audio_test] cmd: OK!");
        TRACE(0, "[CMD] res : 0;");
        app_debug_tool_printf("cmd, Process %s", cmd);
    } else {
        AUDIO_TEST_LOG_I("[audio_test] cmd: Can not found cmd: %s", cmd);
        TRACE(0, "[CMD] res : 1; info : Can not found cmd(%s);", cmd);
        app_debug_tool_printf("cmd, Invalid %s", cmd);
    }

    return 0;
}

int32_t audio_test_cmd_init(void)
{
    app_trace_rx_register("audio_test", audio_test_cmd_callback);

#if defined(AUDIO_TEST_HIFI)
    extern int32_t audio_test_hifi_init(void);
    audio_test_hifi_init();
#endif

    return 0;
}
