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
#ifdef VOICE_ASSIST_FF_FIR_LMS
#include "hal_trace.h"
#include "app_anc_assist.h"
#include "anc_assist.h"
#include "anc_assist_anc.h"
#include "anc_process.h"
#include "audio_dump.h"
#include "app_utils.h"
#include "ae_math.h"
#include "hal_timer.h"
#include "app_anc.h"
#include "app_anc_table.h"
#include "cmsis.h"
#include "anc_ff_fir_lms.h"
#include "anc_mc_fir_lms.h"
#include "anc_fir_lms_aec.h"
#include "anc_fir_lms_eq.h"
#include "event_detection.h"
#include "tt_noise_reduction.h"
#include "feedback_canceller.h"
#include "hal_codec.h"
#include "cmsis_os.h"
#include "app_voice_assist_fir_lms_cfg.h"
#include "app_voice_assist_optimal_tf.h"
#include "app_voice_assist_fir_lms.h"
#include "anc_fir_coeff_config.h"
#include "app_voice_assist_fir_lms_thread.h"
#include "app_voice_assist_event_detection_thread.h"
#include "sensor_hub_core.h"
#if defined(APP_MCPP_CLI)
#include "app_mcpp_comm.h"
#include "app_mcpp.h"
#include "mcpp_server.h"
#endif

// #define TT_NOISE_REDUCTION
// #define TT_AFC

// #define VOICE_FIR_LMS_DUMP_REMOTE
// #define VOICE_FIR_LMS_DUMP_LOCAL
#if defined(VOICE_FIR_LMS_DUMP_LOCAL) || defined(VOICE_FIR_LMS_DUMP_REMOTE)
static short tmp_data[FIR_BLOCK_SIZE];
#endif

extern const struct_anc_cfg * anc_coef_list_50p7k[];

#define HEAP_BUFF_SIZE (156 * 1024 * FIR_CHANNEL_NUM ) // fir_lms heap:157K for 2048 ,used:142k

#include "ext_heap.h"
#if defined(APP_MCPP_CLI)
#if (FIR_CHANNEL_NUM == 2)
static  uint8_t SENSOR_HUB_BUFFER_LOCATION fir_ext_heap_event[HEAP_BUFF_EVENT_SIZE];
static  uint8_t fir_ext_heap[HEAP_BUFF_SIZE];
#else
static ALIGNED(4) uint8_t fir_ext_heap_event[HEAP_BUFF_EVENT_SIZE];
static ALIGNED(4) uint8_t SENSOR_HUB_BUFFER_LOCATION fir_ext_heap[HEAP_BUFF_SIZE];
#endif
#else
static ALIGNED(4) uint8_t fir_ext_heap_event[HEAP_BUFF_EVENT_SIZE];
static ALIGNED(4) uint8_t fir_ext_heap[HEAP_BUFF_SIZE];
#endif

static EventDetectionState *event_st = NULL;
#ifdef MC_FIR_LMS_ENABLED
static ANCMCFirLmsSt *mc_fir_st[FIR_CHANNEL_NUM] = {NULL, };
#endif
static ANCFFFirLmsSt *fir_st[FIR_CHANNEL_NUM]= {NULL, };
#ifdef FIR_LMS_AEC_ENABLED
static ANCFFFIRlmsAecSt *aec_st = NULL;
#endif

#ifdef __NuttX__
#if (FIR_CHANNEL_NUM == 2)
// stereo
#define FREQ_FIR_LOW       (APP_SYSFREQ_104M)
#define FREQ_FIR_HIGH      (APP_SYSFREQ_208M)
#else
#if defined(VOICE_ASSIST_FF_FIR_LMS_MULTI_THREAD)
// tws dual core
#define FREQ_FIR_LOW       (APP_SYSFREQ_52M)
#define FREQ_FIR_HIGH      (APP_SYSFREQ_104M)
#else
// tws single core
#define FREQ_FIR_LOW       (APP_SYSFREQ_104M)
#define FREQ_FIR_HIGH      (APP_SYSFREQ_208M)
#endif
#endif
typedef enum {
    SET_FREQ_NONE = 0,
    SET_FREQ_FIR_LOW,
    SET_FREQ_FIR_HIGH,
    SET_FREQ_TT_LOW,
    SET_FREQ_TT_HIGH,
    SET_FREQ_NUM,
} set_freq_enum_t;
static set_freq_enum_t _set_freq_state = SET_FREQ_NONE;

typedef enum {
    SENSOR_CPU0 = 0,
    SENSOR_CPU1 = 1,
    SENSOR_CPU_QTY
} _SENSOR_CPU_T;

static int _thread_switch_to_cpu(osThreadId thread_id, _SENSOR_CPU_T cpu_id)
{
    int thread;
    int ret = 0;
    osThreadDef_t *p_thread_def = NULL;

    p_thread_def = (osThreadDef_t *)thread_id;
    thread = p_thread_def->thread;
    if (thread < 0) {
        TRACE(1, "thread:%d error please check!!!!!\n", thread);
        return 2;
    }

    cpu_set_t cpu_set = 1 << 0;
    cpu_set_t cpu_set_curr;
    if (SENSOR_CPU1 == cpu_id) {
        cpu_set = 1 << 1;
    }
    nxsched_get_affinity(thread, sizeof(cpu_set_curr), &cpu_set_curr);
    if (cpu_set != cpu_set_curr) {
        sched_setaffinity(thread, sizeof(cpu_set), &cpu_set);
        TRACE(2, "thread:%d switch to cpu:%d", thread, cpu_id);
    } else {
        ret = 1;
    }

    return ret;
}

static osMutexId _set_freq_mutex_id = NULL;
osMutexDef(_set_freq_mutex);

static void _set_freq_create_lock(void)
{
    if (_set_freq_mutex_id == NULL) {
        _set_freq_mutex_id = osMutexCreate((osMutex(_set_freq_mutex)));
    }
}

static void _set_freq_destroy_lock(void)
{
    if (_set_freq_mutex_id != NULL) {
        osMutexRelease((osMutex(_set_freq_mutex)));
        _set_freq_mutex_id = NULL;
    }
}

static void _set_freq_lock(void)
{
    osMutexWait(_set_freq_mutex_id, osWaitForever);
}

static void _set_freq_unlock(void)
{
    osMutexRelease(_set_freq_mutex_id);
}
#endif

#ifdef TT_NOISE_REDUCTION
static TtNsState *ns_st = NULL;
static int32_t _voice_assist_tt_noise_reduction_handler(float *coeffs, uint32_t num);
#endif

#ifdef TT_AFC
static AfcState *afc_st = NULL;
static int32_t _voice_assist_afc_handler(float *coeffs, uint32_t num);
#endif

static struct_anc_fir_cfg fir_stream_cfg;
void app_voice_assist_fir_lms_set_fir_HW_work_mode(int mode)
{
    if (mode == ANC_CUSTOM_MODE_ANC) {
        fir_stream_cfg.anc_fir_cfg_ff_l.fir_bypass_flag = 0;
        fir_stream_cfg.anc_fir_cfg_ff_l.fir_len = 0;
        fir_stream_cfg.anc_fir_cfg_ff_l.fir_step = 12;
        arm_fill_q31(0, &fir_stream_cfg.anc_fir_cfg_ff_l.fir_coef[0], AUD_COEF_LEN);
        fir_stream_cfg.anc_fir_cfg_ff_l.fir_coef[0] = 0 * 32768 * 256;

        fir_stream_cfg.anc_fir_cfg_tt_l.fir_bypass_flag = 0;
        fir_stream_cfg.anc_fir_cfg_tt_l.fir_len = 0;
        fir_stream_cfg.anc_fir_cfg_tt_l.fir_step = 12;
        arm_fill_q31(0, &fir_stream_cfg.anc_fir_cfg_tt_l.fir_coef[0], AUD_COEF_LEN);
        fir_stream_cfg.anc_fir_cfg_tt_l.fir_coef[0] = 0 * 32768 * 256;

        anc_fir_open(&fir_stream_cfg, ANC_FEEDFORWARD);
        anc_fir_open(&fir_stream_cfg, ANC_TALKTHRU);

        anc_tt_pass_mc_enable(false);
        anc_dehowl_enable(false);
    }else if (mode == ANC_CUSTOM_MODE_FIR_ANC) {
        fir_stream_cfg.anc_fir_cfg_ff_l.fir_bypass_flag = 0;
        fir_stream_cfg.anc_fir_cfg_ff_l.fir_len = LOCAL_FIR_LEN / 12;
        fir_stream_cfg.anc_fir_cfg_ff_l.fir_step = 12;
        arm_fill_q31(0, &fir_stream_cfg.anc_fir_cfg_ff_l.fir_coef[0], AUD_COEF_LEN);
        fir_stream_cfg.anc_fir_cfg_ff_l.fir_coef[0] = 0 * 32768 * 256;

        fir_stream_cfg.anc_fir_cfg_tt_l.fir_bypass_flag = 0;
        fir_stream_cfg.anc_fir_cfg_tt_l.fir_len = 0;
        fir_stream_cfg.anc_fir_cfg_tt_l.fir_step = 12;
        arm_fill_q31(0, &fir_stream_cfg.anc_fir_cfg_tt_l.fir_coef[0], AUD_COEF_LEN);
        fir_stream_cfg.anc_fir_cfg_tt_l.fir_coef[0] = 0 * 32768 * 256;

#if (FIR_CHANNEL_NUM == 2)
        fir_stream_cfg.anc_fir_cfg_ff_r.fir_bypass_flag = 0;
        fir_stream_cfg.anc_fir_cfg_ff_r.fir_len = LOCAL_FIR_LEN / 12;
        fir_stream_cfg.anc_fir_cfg_ff_r.fir_step = 12;
        arm_fill_q31(0, &fir_stream_cfg.anc_fir_cfg_ff_r.fir_coef[0], AUD_COEF_LEN);
        fir_stream_cfg.anc_fir_cfg_ff_r.fir_coef[0] = 0 * 32768 * 256;

        anc_fir_open(&fir_stream_cfg, ANC_FEEDFORWARD);
#else
        anc_fir_open(&fir_stream_cfg, ANC_FEEDFORWARD);
        anc_fir_open(&fir_stream_cfg, ANC_TALKTHRU);
#endif

        anc_tt_pass_mc_enable(false);
        anc_dehowl_enable(false);

        anc_set_fir_cfg(&fir_stream_cfg, ANC_FEEDFORWARD);
    }else if (mode == ANC_CUSTOM_MODE_TT) {
        fir_stream_cfg.anc_fir_cfg_ff_l.fir_bypass_flag = 0;
        fir_stream_cfg.anc_fir_cfg_ff_l.fir_len = 0;
        fir_stream_cfg.anc_fir_cfg_ff_l.fir_step = 12;
        arm_fill_q31(0, &fir_stream_cfg.anc_fir_cfg_ff_l.fir_coef[0], AUD_COEF_LEN);
        fir_stream_cfg.anc_fir_cfg_ff_l.fir_coef[0] = 0 * 32768 * 256;

        fir_stream_cfg.anc_fir_cfg_tt_l.fir_bypass_flag = 0;
        fir_stream_cfg.anc_fir_cfg_tt_l.fir_len = 0;
        fir_stream_cfg.anc_fir_cfg_tt_l.fir_step = 12;
        arm_fill_q31(0, &fir_stream_cfg.anc_fir_cfg_tt_l.fir_coef[0], AUD_COEF_LEN);
        fir_stream_cfg.anc_fir_cfg_tt_l.fir_coef[0] = 0 * 32768 * 256;

        anc_fir_open(&fir_stream_cfg, ANC_FEEDFORWARD);
        anc_fir_open(&fir_stream_cfg, ANC_TALKTHRU);

        anc_tt_pass_mc_enable(true);
        anc_dehowl_enable(false);
    }else if (mode == ANC_CUSTOM_MODE_FIR_TT) {
        fir_stream_cfg.anc_fir_cfg_ff_l.fir_bypass_flag = 0;
        fir_stream_cfg.anc_fir_cfg_ff_l.fir_len = 40;
        fir_stream_cfg.anc_fir_cfg_ff_l.fir_step = 12;
        arm_fill_q31(0, &fir_stream_cfg.anc_fir_cfg_ff_l.fir_coef[0], AUD_COEF_LEN);
        fir_stream_cfg.anc_fir_cfg_ff_l.fir_coef[0] = 0 * 32768 * 256;

        fir_stream_cfg.anc_fir_cfg_tt_l.fir_bypass_flag = 0;
        fir_stream_cfg.anc_fir_cfg_tt_l.fir_len = 40;
        fir_stream_cfg.anc_fir_cfg_tt_l.fir_step = 12;
        arm_fill_q31(0, &fir_stream_cfg.anc_fir_cfg_tt_l.fir_coef[0], AUD_COEF_LEN);
        fir_stream_cfg.anc_fir_cfg_tt_l.fir_coef[0] = 0.5 * 32768 * 256;

        anc_fir_open(&fir_stream_cfg, ANC_FEEDFORWARD);
        anc_fir_open(&fir_stream_cfg, ANC_TALKTHRU);

        anc_tt_pass_mc_enable(true);
        anc_dehowl_enable(true);

        anc_set_fir_cfg(&fir_stream_cfg, ANC_FEEDFORWARD);
        anc_set_fir_cfg(&fir_stream_cfg, ANC_TALKTHRU);
    }else if (mode == ANC_CUSTOM_MODE_ADAPTIVE) {
        fir_stream_cfg.anc_fir_cfg_ff_l.fir_bypass_flag = 0;
        fir_stream_cfg.anc_fir_cfg_ff_l.fir_len = 0;
        fir_stream_cfg.anc_fir_cfg_ff_l.fir_step = 12;
        arm_fill_q31(0, &fir_stream_cfg.anc_fir_cfg_ff_l.fir_coef[0], AUD_COEF_LEN);
        fir_stream_cfg.anc_fir_cfg_ff_l.fir_coef[0] = 0 * 32768 * 256;

        fir_stream_cfg.anc_fir_cfg_tt_l.fir_bypass_flag = 0;
        fir_stream_cfg.anc_fir_cfg_tt_l.fir_len = 0;
        fir_stream_cfg.anc_fir_cfg_tt_l.fir_step = 12;
        arm_fill_q31(0, &fir_stream_cfg.anc_fir_cfg_tt_l.fir_coef[0], AUD_COEF_LEN);
        fir_stream_cfg.anc_fir_cfg_tt_l.fir_coef[0] = 0 * 32768 * 256;

        anc_fir_open(&fir_stream_cfg, ANC_FEEDFORWARD);
        anc_fir_open(&fir_stream_cfg, ANC_TALKTHRU);

        anc_tt_pass_mc_enable(true);
        anc_dehowl_enable(false);
    }
}

#if !defined(FIR_ADAPT_ANC_M55) && !defined(APP_MCPP_CLI)
static struct_anc_fir_cfg fir_cache_cfg = {
    .anc_fir_cfg_ff_l.fir_bypass_flag = 0,
    .anc_fir_cfg_ff_l.fir_len = LOCAL_FIR_LEN,
    .anc_fir_cfg_ff_l.fir_coef[0] = 0 * 32768 * 256,
    .anc_fir_cfg_mc_l.fir_bypass_flag = 0,
    .anc_fir_cfg_mc_l.fir_len = LOCAL_FIR_LEN,
    .anc_fir_cfg_mc_l.fir_coef[0] = 0.5 * 32768 * 256,
};

static int stop_flag = 0;
int32_t *fir_coeff_cache = NULL;
int32_t *fir_coeff_cache2 = NULL;
int32_t *mc_fir_coeff_cache = NULL;
#endif

/*
lijiayi1         -11.599780
zhoujiayi1       -11.944938        -11.7

menglinkui2       -10.130689
zhouruolin2       -10.970635
lvyuyan2         -10.493400      -10.5

lixiaojuan2       -8.845212
wangqinsheng2     -8.763390
zhuzhihao2        -9.058938     -8.9
*/
#define STAGE_NUM (3)
const static float faet_thd[STAGE_NUM] = {-11.7, -10.5, -8.9};

static EventDetectionRes event_res;
typedef struct
{
    ANCFFFirLmsSt *anc_inst[FIR_CHANNEL_NUM];
    ANC_FF_FIR_LMS_STAGE last_stage;
    ANC_FF_FIR_LMS_STAGE stage;
    uint32_t frame_count;
    uint32_t pnc_total_frame;
    uint32_t switch_total_frame;
    uint32_t anc_total_frame;
    uint32_t switch_mode_frame;
    uint32_t adapt_iir_frame;
    int32_t mips_needed;
    app_anc_mode_t mode;
    uint32_t ctrl;
    FIR_LMS_CALIB_GAIN calib_gain[FIR_CHANNEL_NUM];
    bool fir_enable;
    bool fir_status;
    float fb_gain;
    int32_t tt_algo_status;
} voice_assist_fir_lms_inst;

static voice_assist_fir_lms_inst ctx =
{
    .fir_enable = true,
    .fir_status = true,
};

static voice_assist_fir_lms_inst *voice_assist_get_ctx(void)
{
    return &ctx;
}

typedef enum
{
    ANC_FF_FIR_LMS_CMD_SET_CFG = 0,
    ANC_FF_FIR_LMS_CMD_SET_FIR_CFG,
    ANC_FF_FIR_LMS_CMD_SWITCH_MODE,
    ANC_FF_FIR_LMS_CMD_RETURN_RES,
    ANC_FF_FIR_LMS_CMD_SET_IIR,
} ANC_FF_FIR_LMS_CMD;

typedef enum
{
    ANC_FF_FIR_LMS_CTRL_SET_FF_FIR_OFF = 0,
    ANC_FF_FIR_LMS_CTRL_SET_FF_FIR_ON,
    ANC_FF_FIR_LMS_CTRL_SET_ANC_MODE,
    ANC_FF_FIR_LMS_CTRL_START_FIR,
    ANC_FF_FIR_LMS_CTRL_STOP_FIR,
    ANC_FF_FIR_LMS_CTRL_SET_FIR_FLAG,
    ANC_FF_FIR_LMS_CTRL_SET_CALIB_GAIN,
    ANC_FF_FIR_LMS_CTRL_SET_FB_GAIN,
    ANC_FF_FIR_LMS_CTRL_SET_TT_NS_CFG,
    ANC_FF_FIR_LMS_CTRL_SET_TT_ALGO_STATUS,
    ANC_FF_FIR_LMS_CTRL_NUM,
} ANC_FF_FIR_LMS_CTRL;

typedef struct
{
    enum ANC_TYPE_T type;
    aud_fir_item cfg;
} voice_assist_anc_fir_cfg;

typedef struct
{
    app_anc_mode_t mode;
    enum ANC_TYPE_T type;
    ANC_GAIN_TIME gain_delay;
} voice_assist_anc_cfg;

#define MAX_ADAPT_IIR_BAND (2)

typedef struct
{
    enum ANC_TYPE_T type;
    uint32_t index;
    int32_t coeffs[6*MAX_ADAPT_IIR_BAND];
    int32_t coeffs_len;
} voice_assist_anc_iir_cfg;

int32_t fir_lms_anc_set_cfg(app_anc_mode_t mode, enum ANC_TYPE_T anc_type, ANC_GAIN_TIME anc_gain_delay);

WEAK int32_t event_detection_set_fir_flag(EventDetectionState* st, uint32_t flag)
{
    return 0;
}

int32_t app_voice_assist_fir_lms_choose_mode(void)
{
    voice_assist_fir_lms_inst *ctx = voice_assist_get_ctx();
    int32_t best_mode = STAGE_NUM;

    float feat = anc_ff_fir_lms_get_TF_feature(ctx->anc_inst[0]);

    float ff_calib_gain_db = LIN2DB(ctx->calib_gain[0].ff_gain/512.0);
    feat -= ff_calib_gain_db;
    // for (int32_t i = 1; i < STAGE_NUM; i++){
    //     if(feat < faet_thd[i-1]){
    //         best_mode = i;
    //         break;
    //     }
    // }
    // best_mode = best_mode + 6; //20 mode, starting at 6
    float dist = 0.0;
    float dist_min = 100.0;
    for (int32_t i = 0; i < STAGE_NUM; i++){
        dist = ABS(feat - faet_thd[i]);
        if (dist_min > dist){
            dist_min = dist;
            best_mode = i + 7;
        }
    }

    TRACE(2, "[%s] ff_calib=%ddB feat = %de-2 best_mode = mode %d", __FUNCTION__,(int)ff_calib_gain_db, (int)(feat * 100), best_mode);
    if (best_mode != 1){
        fir_lms_anc_set_cfg(best_mode, ANC_TALKTHRU, ANC_GAIN_NO_DELAY);
    }
    // fir_lms_app_anc_switch(best_mode);
    return 0;
}

static struct_anc_cfg iir_cfg;
static void app_voice_assist_init_iir_cfg(const struct_anc_cfg *cfg)
{
    TRACE(0,"[%s]...",__func__);
    memcpy(&iir_cfg, cfg, sizeof(struct_anc_cfg));
}

static void app_voice_assist_set_iir_coeffs(enum ANC_TYPE_T anc_type, uint32_t index, int32_t *coeffs, int32_t len)
{
    if (anc_type == ANC_FEEDFORWARD) {
        memcpy(&iir_cfg.anc_cfg_ff_l.iir_coef[index], coeffs, len * sizeof(int32_t));
    } if (anc_type == ANC_FEEDBACK) {
        memcpy(&iir_cfg.anc_cfg_fb_l.iir_coef[index], coeffs, len * sizeof(int32_t));
    } else if (anc_type == ANC_TALKTHRU) {
        memcpy(&iir_cfg.anc_cfg_tt_l.iir_coef[index], coeffs, len * sizeof(int32_t));
    }
    anc_set_cfg(&iir_cfg, anc_type, 0);
    // TRACE(0,"coeffs[0]=0x%x coeffs[2]=0x%x coeffs[4]=0x%x len=%d",coeffs[0],coeffs[2],coeffs[4],len);
}

static POSSIBLY_UNUSED int32_t _assist_anc_dsp_result_callback(void *buf, uint32_t len, void *other, uint32_t sub_cmd)
{
    // TRACE(0,"[%s] get data is %d and %d", __func__, ((struct_anc_fir_cfg *)buf)->anc_fir_cfg_ff_l.fir_len, sub_cmd);
    // TRACE(0, "[%s] sub_cmd = %d", __FUNCTION__, sub_cmd);

    switch (sub_cmd) {
    case ANC_FF_FIR_LMS_CMD_SET_CFG:
    {
        voice_assist_anc_cfg *iir_cfg = (voice_assist_anc_cfg *)buf;
        if (iir_cfg->mode > APP_ANC_MODE_OFF && iir_cfg->mode < APP_ANC_MODE_QTY) {
            anc_set_cfg(anc_coef_list_50p7k[iir_cfg->mode], iir_cfg->type, iir_cfg->gain_delay);
        } else {
            TRACE(0, "[%s] invalid mode %d", __FUNCTION__, iir_cfg->mode);
        }
        break;
    }
    case ANC_FF_FIR_LMS_CMD_SET_IIR:
    {
        voice_assist_anc_iir_cfg *iir_cfg = (voice_assist_anc_iir_cfg *)buf;
        app_voice_assist_set_iir_coeffs(iir_cfg->type, iir_cfg->index, iir_cfg->coeffs, iir_cfg->coeffs_len / sizeof(int32_t));
        break;
    }
    case ANC_FF_FIR_LMS_CMD_SWITCH_MODE:
    {
        app_anc_mode_t *mode = (app_anc_mode_t *)buf;
        app_anc_switch_locally(*mode);
        break;
    }
    case ANC_FF_FIR_LMS_CMD_RETURN_RES:
    {
        uint32_t ff_ch_num = MAX_FF_CHANNEL_NUM, fb_ch_num = MAX_FB_CHANNEL_NUM;
        EventDetectionRes *res = (EventDetectionRes *)buf;
        if (any_of_u32(res->ff_gain_changed, ff_ch_num, ANC_ASSIST_ALGO_STATUS_CHANGED) ||
            any_of_u32(res->fb_gain_changed, fb_ch_num, ANC_ASSIST_ALGO_STATUS_CHANGED)) {
            anc_assist_anc_set_gain_coef(res->ff_gain_changed, res->ff_gain_id, res->ff_gain, ff_ch_num, res->fb_gain_changed, res->fb_gain_id, res->fb_gain, fb_ch_num);
        }

        // TODO: deal with stereo headphone
        if (res->curve_changed[0]) {
            anc_assist_anc_switch_curve(res->curve_id[0], res->curve_index[0]);
        }

        if (res->fir_flag_changed) {
            anc_assist_anc_switch_fir_flag(res->fir_flag_id, res->fir_flag); //to do
        }

        if (res->noise_status[0] == NOISE_STATUS_STRONG_ANC) {
            TRACE(4, "[noise_status]change to strong");
        } else if (res->noise_status[0] == NOISE_STATUS_MIDDLE_ANC) {
            TRACE(3, "[noise_status]change to middle");
        } else if (res->noise_status[0] == NOISE_STATUS_LOWER_ANC) {
            TRACE(2, "[noise_status]change to low");
        } else if (res->noise_status[0] == NOISE_STATUS_QUIET_ANC) {
            TRACE(1, "[noise_status]change to quiet");
        } else {
            TRACE(1, "[noise_status]change to quiet extremely");
        }

        break;
    }
    default:
        TRACE(0, "[%s] cmd  = %d,is invalid", __FUNCTION__, sub_cmd);
        break;
    }

    return 0;
}

static POSSIBLY_UNUSED int32_t app_voice_assist_fir_lms_ctrl(anc_assist_user_t user, uint32_t ctrl, uint8_t *buf, uint32_t len)
{
#if defined(APP_MCPP_CLI)
	return app_mcpp_capture_algo_ctl(APP_MCPP_USER_VOICE_ASSIST_FIR_LMS, ctrl, buf, len, APP_MCPP_SYNC);
#elif defined(FIR_ADAPT_ANC_M55)
    return app_anc_assist_ctrl(user, ctrl, buf, len);
#else
    return voice_assist_fir_lms_set_cfg(ctrl, buf, len);
#endif
}

static POSSIBLY_UNUSED int32_t app_voice_assist_fir_lms_ctrl_async(anc_assist_user_t user, uint32_t ctrl, uint8_t *buf, uint32_t len)
{
#if defined(APP_MCPP_CLI)
	return app_mcpp_capture_algo_ctl(APP_MCPP_USER_VOICE_ASSIST_FIR_LMS, ctrl, buf, len, APP_MCPP_NOTIFY);
#elif defined(FIR_ADAPT_ANC_M55)
    return app_anc_assist_ctrl(user, ctrl, buf, len);
#else
    return voice_assist_fir_lms_set_cfg(ctrl, buf, len);
#endif
}

#if defined(APP_MCPP_CLI)
static void app_voice_assist_fir_lms_receive_data_handler(uint8_t* ptr, short len)
{
    app_voice_assist_core_interact_data_t *receive_data = (app_voice_assist_core_interact_data_t *)ptr;
    //TRACE(2, "<receive data>assist_user:%d len:%d", receive_data->user, receive_data->len);
    _assist_anc_dsp_result_callback(receive_data->buf, receive_data->len,
                                                                NULL, receive_data->sub_cmd);
}
#endif

static int32_t _voice_assist_fir_lms_open_mic_callback(void)
{
    TRACE(0, "[%s]", __FUNCTION__);
#if(FIR_CHANNEL_NUM == 1)
#if defined(CODEC_ECHO_PATH_VER) && (CODEC_ECHO_PATH_VER >= 2)
    //ref_l:music data;ref_r:music+anc
    hal_codec_set_echo_ch1(HAL_CODEC_ECHO_CHAN_DAC_DATA_DBG_OUT_L); //on the one channel mode, need to use another echo ch
    hal_codec_set_echo_ch1_rate(HAL_CODEC_ECHO_RATE_384K);
    hal_codec_set_echo1_path(HAL_CODEC_ECHO_PATH_HBF4_DATA_IN);
#else
    //ref_l:music+anc data;ref_r:music
    hal_codec_set_echo_path(HAL_CODEC_ECHO_PATH_ALL);
    hal_codec_set_echo1_path(HAL_CODEC_ECHO_PATH_DAC1_DAC2_PSAP);
#endif
#elif (FIR_CHANNEL_NUM == 2)
    hal_codec_set_echo_path(HAL_CODEC_ECHO_PATH_ALL);
    hal_codec_set_echo1_path(HAL_CODEC_ECHO_PATH_ALL);
#endif
    TRACE(0, "[%s]", __FUNCTION__);
    // anc_adc_data_select(ANC_ADC_ADC_ADD_MC); //fb DAM data

    return 0;
}

static int32_t _voice_assist_fir_lms_callback(void *buf, uint32_t len, void *other);

int32_t app_voice_assist_fir_lms_init(void)
{
#if defined(FIR_ADAPT_ANC_M55)
    app_anc_assist_result_register(ANC_ASSIST_USER_FIR_LMS, _assist_anc_dsp_result_callback);
#endif
    app_anc_assist_register(ANC_ASSIST_USER_FIR_LMS, _voice_assist_fir_lms_callback);

    return 0;
}

int32_t app_voice_assist_fir_lms_reset(void)
{
#if defined(APP_MCPP_CLI) || defined(FIR_ADAPT_ANC_M55)
    // TODO: Add app_voice_assist_fir_lms_ctrl(ANC_ASSIST_USER_FIR_LMS);
#else
    for (uint32_t i = 0; i < FIR_CHANNEL_NUM; i++) {
        anc_ff_fir_lms_reset(fir_st[i], 0, AUD_CHANNEL_MAP_CH0 << i);
    }
#endif
    return 0;
}

void set_fixed_fir_filter(void)
{
#if defined(APP_MCPP_CLI) || defined(FIR_ADAPT_ANC_M55)
    // TODO: Add fir_lms_mcpp_capture_ctrl(ANC_ASSIST_ALGO_ID_FIR_LMS);
#elif defined(FREEMAN_ENABLED_STERO)
    for (uint32_t i = 0; i < FIR_CHANNEL_NUM; i++) {
        anc_ff_fir_lms_reset(fir_st[i], 0, AUD_CHANNEL_MAP_CH0 << i);
    }
#else
    for (uint32_t i = 0; i < FIR_CHANNEL_NUM; i++) {
        anc_ff_fir_lms_reset(fir_st[i], 1, AUD_CHANNEL_MAP_CH0 << i);
    }
#endif
}

int32_t app_voice_assist_fir_lms_enable_fir(bool enable)
{
    voice_assist_fir_lms_inst *ctx = voice_assist_get_ctx();

    ctx->fir_enable = enable;

    app_voice_assist_fir_lms_set_fir_status(ctx->fir_status);

    return 0;
}

int32_t app_voice_assist_fir_lms_set_fir_status(bool enable)
{
    voice_assist_fir_lms_inst *ctx = voice_assist_get_ctx();

    ctx->fir_status = enable;

    if (ctx->fir_enable & ctx->fir_status) {
        app_voice_assist_fir_lms_ctrl(ANC_ASSIST_USER_FIR_LMS, ANC_FF_FIR_LMS_CTRL_SET_FF_FIR_ON, NULL, 0);
    } else {
        app_voice_assist_fir_lms_ctrl(ANC_ASSIST_USER_FIR_LMS, ANC_FF_FIR_LMS_CTRL_SET_FF_FIR_OFF, NULL, 0);
    }

    return 0;
}

int32_t app_voice_assist_fir_lms_set_tt_algo_status(int32_t enable)
{
    app_voice_assist_fir_lms_ctrl(ANC_ASSIST_USER_FIR_LMS, ANC_FF_FIR_LMS_CTRL_SET_TT_ALGO_STATUS, (uint8_t *)&enable, sizeof(int32_t));

    return 0;
}

int32_t app_voice_assist_fir_lms_set_anc_mode(app_anc_mode_t mode)
{
    app_voice_assist_fir_lms_ctrl(ANC_ASSIST_USER_FIR_LMS, ANC_FF_FIR_LMS_CTRL_SET_ANC_MODE, &mode, sizeof(mode));

    return 0;
}

int32_t app_voice_assist_fir_lms_set_fir_flag(uint32_t fir_flag)
{
    app_voice_assist_fir_lms_ctrl_async(ANC_ASSIST_USER_FIR_LMS, ANC_FF_FIR_LMS_CTRL_SET_FIR_FLAG, (uint8_t *)&fir_flag, sizeof(fir_flag));

    return 0;
}

int32_t app_voice_assist_fir_lms_set_fb_gain(float gain)
{
    app_voice_assist_fir_lms_ctrl(ANC_ASSIST_USER_FIR_LMS, ANC_FF_FIR_LMS_CTRL_SET_FB_GAIN, (uint8_t *)&gain, sizeof(gain));

    return 0;
}

int32_t app_voice_assist_fir_lms_set_tt_ns_cfg(void *cfg)
{
    app_voice_assist_fir_lms_ctrl(ANC_ASSIST_USER_FIR_LMS, ANC_FF_FIR_LMS_CTRL_SET_TT_NS_CFG, (uint8_t *)cfg, sizeof(TtNsConfig));

    return 0;
}

int32_t app_voice_assist_fir_lms_start_fir(void)
{
    app_voice_assist_fir_lms_ctrl(ANC_ASSIST_USER_FIR_LMS, ANC_FF_FIR_LMS_CTRL_START_FIR, NULL, 0);

    return 0;
}

int32_t app_voice_assist_fir_lms_stop_fir(void)
{
    app_voice_assist_fir_lms_ctrl(ANC_ASSIST_USER_FIR_LMS, ANC_FF_FIR_LMS_CTRL_STOP_FIR, NULL, 0);

    return 0;
}

#if defined(APP_MCPP_CLI) || defined(FIR_ADAPT_ANC_M55)
int32_t _voice_assist_fir_lms_return_res_handler(EventDetectionRes *res);
#endif

int32_t fir_lms_return_res_handler(EventDetectionRes *res)
{
#if defined(APP_MCPP_CLI) || defined(FIR_ADAPT_ANC_M55)
    return _voice_assist_fir_lms_return_res_handler(res);
#else
    return _assist_anc_dsp_result_callback(res, sizeof(EventDetectionRes), NULL, ANC_FF_FIR_LMS_CMD_RETURN_RES);
#endif
}

#if defined(APP_MCPP_CLI) || defined(FIR_ADAPT_ANC_M55)
int32_t _voice_assist_anc_set_cfg_handler(app_anc_mode_t mode, enum ANC_TYPE_T anc_type, ANC_GAIN_TIME anc_gain_delay);
#endif

int32_t fir_lms_anc_set_cfg(app_anc_mode_t mode, enum ANC_TYPE_T anc_type, ANC_GAIN_TIME anc_gain_delay)
{
    if (mode <= APP_ANC_MODE_OFF || mode >= APP_ANC_MODE_QTY) {
        TRACE(0, "[%s] invalid mode %d", __FUNCTION__, mode);
        return -1;
    }

#if defined(APP_MCPP_CLI) || defined(FIR_ADAPT_ANC_M55)
    return _voice_assist_anc_set_cfg_handler(mode, anc_type, anc_gain_delay);
#else
    return anc_set_cfg(anc_coef_list_50p7k[mode - 1], anc_type, anc_gain_delay);
#endif
}

#if defined(APP_MCPP_CLI) || defined(FIR_ADAPT_ANC_M55)
int32_t _voice_assist_app_anc_switch_handler(app_anc_mode_t mode);
#endif

FIR_LMS_TEXT_LOCATION
int32_t fir_lms_app_anc_switch(app_anc_mode_t mode)
{
#if defined(APP_MCPP_CLI) || defined(FIR_ADAPT_ANC_M55)
    return _voice_assist_app_anc_switch_handler(mode);
#else
    return app_anc_switch_locally(mode);
#endif
}

#if defined(APP_MCPP_CLI) || defined(FIR_ADAPT_ANC_M55)
int32_t _voice_assist_set_iir_handler(enum ANC_TYPE_T anc_type, uint32_t index, uint8_t *coeffs, int32_t len);
#endif

FIR_LMS_TEXT_LOCATION
int32_t fir_lms_anc_set_iir(enum ANC_TYPE_T anc_type, uint32_t index, int32_t *coeffs, int32_t len)
{
#if defined(APP_MCPP_CLI) || defined(FIR_ADAPT_ANC_M55)
    ASSERT(len % 6 == 0, "[%s] Each stage must contains 6 coeffs", __FUNCTION__);
    ASSERT(len / 6 <= MAX_ADAPT_IIR_BAND, "[%s] max support 2 band", __FUNCTION__);
    return _voice_assist_set_iir_handler(anc_type, index, (uint8_t *)coeffs, len * sizeof(int32_t));
#else
    app_voice_assist_set_iir_coeffs(anc_type, index, coeffs, len);
    return 0;
#endif
}

FIR_LMS_TEXT_LOCATION
int32_t dsp_set_anc_fir_cfg(struct_anc_fir_cfg *cfg, enum ANC_TYPE_T anc_type, enum AUD_CHANNEL_MAP_T map)
{
#if defined(APP_MCPP_CLI)
    if (ANC_FEEDFORWARD == anc_type){
        if (map == AUD_CHANNEL_MAP_CH0) {
            anc_fir_set_coeff_q23(ANC_FIR_CHANNEL_FF_L, cfg->anc_fir_cfg_ff_l.fir_coef, cfg->anc_fir_cfg_ff_l.fir_len);
        } else if (map == AUD_CHANNEL_MAP_CH1) {
            anc_fir_set_coeff_q23(ANC_FIR_CHANNEL_FF_R, cfg->anc_fir_cfg_ff_l.fir_coef, cfg->anc_fir_cfg_ff_l.fir_len);
        } else {
            ASSERT(0, "[%s] invalid channel map %d", __FUNCTION__, map);
        }
    }else if (ANC_MUSICCANCLE == anc_type){
        anc_fir_set_coeff_q23(ANC_FIR_CHANNEL_MC_L, cfg->anc_fir_cfg_mc_l.fir_coef, cfg->anc_fir_cfg_mc_l.fir_len);
    }else if (ANC_TALKTHRU == anc_type){
        anc_fir_set_coeff_q23(ANC_FIR_CHANNEL_TT_L, cfg->anc_fir_cfg_tt_l.fir_coef, cfg->anc_fir_cfg_tt_l.fir_len);
    }
#elif defined(FIR_ADAPT_ANC_M55)
    static voice_assist_anc_fir_cfg fir_cfg;

    uint8_t user = ANC_ASSIST_USER_FIR_LMS;
    uint8_t cmd = ANC_FF_FIR_LMS_CMD_SET_FIR_CFG;

    fir_cfg.type = anc_type;
    memcpy(&fir_cfg.cfg, cfg, sizeof(struct_anc_fir_cfg));
    anc_assist_dsp_send_result_to_bth_via_fifo(user, (uint8_t *)&fir_cfg, sizeof(voice_assist_anc_fir_cfg), (uint8_t)cmd);
#else
    anc_set_fir_cfg(cfg, anc_type);
#endif
    return 0;
}

WEAK int32_t app_anc_get_calib_gain(FIR_LMS_CALIB_GAIN *calib_gain)
{
    for (uint32_t i = 0; i < FIR_CHANNEL_NUM; i++) {
        calib_gain[i].ff_gain = 512;
        calib_gain[i].fb_gain = 512;
        calib_gain[i].mc_gain = 512;
    }
    TRACE(0, "[%s] Warning: please impl this function", __FUNCTION__);

    return 0;
}

int32_t app_voice_assist_fir_lms_open(app_anc_mode_t mode)
{
    TRACE(0, "[%s] fir lms start stream", __func__);

#if defined(APP_MCPP_CLI)
    voice_assist_fir_lms_inst *ctx = voice_assist_get_ctx();
#endif

    app_anc_assist_open_mic_register(ANC_ASSIST_USER_FIR_LMS, _voice_assist_fir_lms_open_mic_callback);

    static FIR_LMS_CALIB_GAIN calib_gain[FIR_CHANNEL_NUM];
    app_anc_get_calib_gain(calib_gain);

#if !defined(APP_MCPP_CLI) && !defined(FIR_ADAPT_ANC_M55)
    stop_flag = 0;
    voice_assist_fir_lms_open(calib_gain);
#else
    app_sysfreq_req(APP_SYSFREQ_USER_FIR_LMS, APP_SYSFREQ_52M);
#endif

#ifdef FIR_LMS_AEC_ENABLED
    aec_st = anc_fir_lms_aec_init(FIR_SAMPLE_RATE, FIR_BLOCK_SIZE);
#endif

#if defined(APP_MCPP_CLI)
    APP_MCPP_CFG_T dsp_cfg;
    memset(&dsp_cfg, 0, sizeof(APP_MCPP_CFG_T));
    dsp_cfg.capture.stream_enable = true;
    dsp_cfg.capture.sample_rate = FIR_SAMPLE_RATE;
    dsp_cfg.capture.sample_bytes = 4;
    dsp_cfg.capture.algo_frame_len = FIR_BLOCK_SIZE;
    dsp_cfg.capture.params[1] = app_mcpp_get_cap_channel_map(false, false, false);
    dsp_cfg.capture.channel_num = MIC_INDEX_QTY;
    dsp_cfg.capture.delay = FIR_BLOCK_SIZE * 3;
    dsp_cfg.capture.core_server = APP_MCPP_CORE_SENS;
    dsp_cfg.capture.params[2] = (int)&calib_gain;

    app_mcpp_open(APP_MCPP_USER_VOICE_ASSIST_FIR_LMS, &dsp_cfg);
    app_mcpp_capture_async_ctl_cb_register(APP_MCPP_USER_VOICE_ASSIST_FIR_LMS, app_voice_assist_fir_lms_receive_data_handler);
    if (ctx->fir_enable) {
        app_voice_assist_fir_lms_ctrl(ANC_ASSIST_USER_FIR_LMS, ANC_FF_FIR_LMS_CTRL_SET_FF_FIR_ON, NULL, 0);
    } else {
        app_voice_assist_fir_lms_ctrl(ANC_ASSIST_USER_FIR_LMS, ANC_FF_FIR_LMS_CTRL_SET_FF_FIR_OFF, NULL, 0);
    }
    app_voice_assist_fir_lms_set_anc_mode(mode);
#endif

    anc_fir_lms_eq_init(FIR_SAMPLE_RATE, FIR_BLOCK_SIZE);
    app_anc_assist_open(ANC_ASSIST_USER_FIR_LMS);

    //fir_coeff_cache = fir_lms_coeff_cache(fir_st);
    //mc_fir_coeff_cache = mc_fir_lms_coeff_cache(fir_st);

#ifdef VOICE_FIR_LMS_DUMP_LOCAL
    audio_dump_init(FIR_BLOCK_SIZE, sizeof(short), 3);
#endif

    // close fb anc for adaptive anc, it is better not to open it during the init state
    // anc_set_gain(0, 0, ANC_FEEDBACK);

#if defined(FIR_ADAPT_ANC_M55)
    if (ctx->fir_enable) {
        app_anc_assist_ctrl(ANC_ASSIST_USER_FIR_LMS, ANC_FF_FIR_LMS_CTRL_SET_FF_FIR_ON, NULL, 0);
    } else {
        app_anc_assist_ctrl(ANC_ASSIST_USER_FIR_LMS, ANC_FF_FIR_LMS_CTRL_SET_FF_FIR_OFF, NULL, 0);
    }
    app_voice_assist_fir_lms_set_anc_mode(mode);
    app_anc_assist_ctrl(ANC_ASSIST_USER_FIR_LMS, ANC_FF_FIR_LMS_CTRL_SET_CALIB_GAIN, (uint8_t *)&calib_gain, sizeof(calib_gain));
    app_sysfreq_req(APP_SYSFREQ_USER_FIR_LMS, APP_SYSFREQ_26M);
#endif

    app_voice_assist_init_iir_cfg(anc_coef_list_50p7k[0]);

    return 0;
}

int32_t app_voice_assist_fir_lms_switch(app_anc_mode_t mode)
{
    voice_assist_fir_lms_inst *ctx = voice_assist_get_ctx();

    if (ctx->mode != mode) {
        if (ctx->mode == APP_ANC_MODE_OFF) {
            app_voice_assist_fir_lms_open(mode);
        } else if (mode == APP_ANC_MODE_OFF) {
            app_voice_assist_fir_lms_close();
            app_anc_switch_locally(APP_ANC_MODE_OFF);
        } else {
            app_anc_switch_locally(mode);
        }

        ctx->mode = mode;
    }

    return 0;
}

extern uint8_t is_a2dp_or_sco_on_starting(void);
int32_t app_anc_assist_fir_lms_open(app_anc_mode_t mode)
{
    TRACE(3, "[%s]  Starting:%d ", __func__, is_a2dp_or_sco_on_starting());
    app_voice_assist_fir_lms_open(mode);

    return 0;
}

int32_t app_anc_assist_fir_lms_close(void)
{
    app_voice_assist_fir_lms_close();
    return 0;
}

int32_t app_voice_assist_fir_lms_close(void)
{
    TRACE(0, "[%s] fir lms close stream", __func__);
    app_anc_assist_open_mic_register(ANC_ASSIST_USER_FIR_LMS, NULL);

    app_anc_assist_close(ANC_ASSIST_USER_FIR_LMS);

#if defined(APP_MCPP_CLI)
    app_mcpp_close(APP_MCPP_USER_VOICE_ASSIST_FIR_LMS);
#elif defined(FIR_ADAPT_ANC_M55)
#else
    voice_assist_fir_lms_close();
#endif

#ifdef FIR_LMS_AEC_ENABLED
    anc_fir_lms_aec_destroy(aec_st);
#endif

    app_sysfreq_req(APP_SYSFREQ_USER_FIR_LMS, APP_SYSFREQ_32K);

#ifdef VOICE_FIR_LMS_DUMP_LOCAL
    audio_dump_deinit();
#endif
    return 0;
}

static POSSIBLY_UNUSED inline bool any_of_lms_state(ANCFFFirLmsSt **status, uint32_t len, void *expected_status)
{
	for (uint32_t i = 0; i < len; i++) {
		if (status[i] == expected_status)
			return true;
	}

	return false;
}

int voice_assist_fir_lms_update_cfg(voice_assist_fir_lms_inst *ctx);
int32_t _voice_assist_fir_lms_return_res_handler(EventDetectionRes *res);

int32_t voice_assist_event_detection_process(process_frame_data_t *buf, EventController* event_control)
{
    if (event_st != NULL) {
// uint32_t start_time = hal_fast_sys_timer_get();
        voice_assist_fir_lms_inst *ctx = voice_assist_get_ctx();
        if (ctx->stage > ANC_FF_FIR_LMS_STAGE_SKIP) {
            event_detection_process(event_st, buf, FIR_BLOCK_SIZE, &event_res, event_control);
            // TRACE(0,"------------event_st--------%d us",FAST_TICKS_TO_US(hal_fast_sys_timer_get() - start_time));
        // static uint32_t cnt2 = 0;
        // TRACE(0,"[event_detection_done] cnt = %d", cnt2++);
            if (any_of_u32(event_res.ff_gain_changed, MAX_FF_CHANNEL_NUM, ANC_ASSIST_ALGO_STATUS_CHANGED) ||
                any_of_u32(event_res.fb_gain_changed, MAX_FB_CHANNEL_NUM, ANC_ASSIST_ALGO_STATUS_CHANGED) ||
                any_of_u32(event_res.curve_changed, MAX_FB_CHANNEL_NUM, ANC_ASSIST_ALGO_STATUS_CHANGED) ||
                event_res.fir_flag_changed) {
                // TRACE(1, "fir res is change and send to bth");
                fir_lms_return_res_handler(&event_res);

            }
        }
    }
#if defined(APP_MCPP_CLI) && defined(VOICE_ASSIST_FF_FIR_LMS_MULTI_THREAD)
    app_voice_assist_fir_lms_thread_process(buf, event_control);
#endif
    return 0;
}

#ifdef MC_FIR_LMS_ENABLED
int32_t voice_assist_anc_mc_fir_lms_process(process_frame_data_t *buf, EventController* event_control)
{
    ANC_MC_FIR_LMS_STATUS mc_res = ANC_MC_FIR_LMS_STATE_NO_CHANGED;
    float update_mc_fir[50];
    for (uint32_t i = 0; i < FIR_CHANNEL_NUM; i++) {
        if (mc_fir_st[i] != NULL){
            float *fb_data = buf->fb_mic[i];
            float *ref_data_before = buf->ref[2*i];
            // uint32_t start_time1 = hal_fast_sys_timer_get();
            mc_res = anc_mc_fir_lms_process(mc_fir_st[i],ref_data_before, fb_data, FIR_BLOCK_SIZE, event_control, update_mc_fir, AUD_CHANNEL_MAP_CH0 << i);
            // TRACE(0,"------------mc_fir_st--------%d us",FAST_TICKS_TO_US(hal_fast_sys_timer_get() - start_time1));
            if (ANC_MC_FIR_LMS_STATE_CHANGED_TO_CONVERGENT == mc_res) {
                anc_ff_fir_lms_update_mc_fir(fir_st[i], update_mc_fir);
            }
        }
    }

    return 0;
}
#endif

FIR_LMS_TEXT_LOCATION
int32_t voice_assist_anc_ff_fir_lms_process(process_frame_data_t *buf, EventController* event_control)
{
    if (any_of_lms_state(fir_st, FIR_CHANNEL_NUM, NULL))
        return 0;

    ANC_FF_FIR_LMS_RES res[FIR_CHANNEL_NUM] = {0,};
    voice_assist_fir_lms_inst *ctx = voice_assist_get_ctx();

    voice_assist_fir_lms_update_cfg(ctx);

// uint32_t start_time2 = hal_fast_sys_timer_get();
    for (uint32_t i = 0; i < FIR_CHANNEL_NUM; i++) {
        float* mic_data[3] = {buf->ff_mic[i], buf->fb_mic[i], buf->ref[i]};
        res[i] = anc_ff_fir_lms_process(fir_st[i], mic_data, FIR_BLOCK_SIZE, ctx->stage, event_control, AUD_CHANNEL_MAP_CH0 << i);
    }
// TRACE(0,"------------ff_fir--------%d us",FAST_TICKS_TO_US(hal_fast_sys_timer_get() - start_time2));
#if defined(APP_MCPP_CLI)
    // mcpp_srv_ff_fir_process_done();
#endif

    ctx->frame_count += 1;

    if (ctx->stage == ANC_FF_FIR_LMS_STAGE_IDLE) {
        fir_lms_app_anc_switch(ctx->mode);
        ctx->stage = ANC_FF_FIR_LMS_STAGE_SKIP;
        ctx->frame_count = 0;
    } else if (ctx->stage == ANC_FF_FIR_LMS_STAGE_SKIP){
        if (ctx->frame_count == 15){
            ctx->stage = ANC_FF_FIR_LMS_STAGE_PNC;
            ctx->frame_count = 0;
        }
    } else if (ctx->stage == ANC_FF_FIR_LMS_STAGE_PNC) {
        if (res[0] == ANC_FF_FIR_LMS_RES_PZ_ERR || ctx->frame_count == ctx->pnc_total_frame) {
            ctx->stage = ANC_FF_FIR_LMS_STAGE_WAITING_ANC_ON;
            ctx->frame_count = 0;
        }
    } else if (ctx->stage == ANC_FF_FIR_LMS_STAGE_WAITING_ANC_ON) {
        if (ctx->frame_count == ctx->switch_total_frame) {
#if defined(APP_MCPP_CLI)
            anc_fir_coeff_config_init();
#endif
            ctx->stage = ANC_FF_FIR_LMS_STAGE_ANC;
            ctx->frame_count = 0;
        }
    } else if (ctx->stage == ANC_FF_FIR_LMS_STAGE_ANC) {
        if (app_anc_table_get_custom_mode(ctx->mode) == ANC_CUSTOM_MODE_FIR_ANC) {
            if (res[0] == ANC_FF_FIR_LMS_RES_SZ_ERR) {
                ctx->stage = ANC_FF_FIR_LMS_STAGE_WAITING_ADAPTIVE_ANC_ON;
                ctx->frame_count = 0;
            } else if (ctx->frame_count == ctx->anc_total_frame) {
                ctx->stage = ANC_FF_FIR_LMS_STAGE_WAITING_ADAPTIVE_ANC_ON;
                app_voice_assist_fir_lms_choose_mode();
                ctx->frame_count = 0;
            }
        } else { // normal anc mode
            if (res[0] == ANC_FF_FIR_LMS_RES_SZ_ERR || ctx->frame_count == ctx->anc_total_frame) {
                ctx->stage = ANC_FF_FIR_LMS_STAGE_NORMAL_ANC;
                ctx->frame_count = 0;
            }
        }
        if (ctx->frame_count == 0) {
            event_detection_set_anc_on_flag(event_st, true);
        }
    } else if (ctx->stage == ANC_FF_FIR_LMS_STAGE_WAITING_ADAPTIVE_ANC_ON) {
        if (ctx->frame_count == ctx->switch_mode_frame) {
            ctx->stage = ANC_FF_FIR_LMS_STAGE_ADAPTIVE_IIR;
            ctx->frame_count = 0;
        }
    } else if (ctx->stage == ANC_FF_FIR_LMS_STAGE_ADAPTIVE_IIR) {
        if (ctx->frame_count == ctx->adapt_iir_frame) {
            ctx->stage = ANC_FF_FIR_LMS_STAGE_ADAPTIVE_ANC;
            ctx->frame_count = 0;
        }
    }

    if (ctx->last_stage != ctx->stage) {
        TRACE(2, "[%s] switch stage %s to stage %s", __FUNCTION__, stage_desc[ctx->last_stage], stage_desc[ctx->stage]);
        ctx->last_stage = ctx->stage;
    }

    if (ctx->stage == ANC_FF_FIR_LMS_STAGE_NORMAL_ANC &&
        ctx->tt_algo_status == 1 &&
        app_anc_table_get_custom_mode(ctx->mode) == ANC_CUSTOM_MODE_FIR_TT) {
#if defined(TT_AFC) || defined(TT_NOISE_REDUCTION)
        POSSIBLY_UNUSED float *ff = mic_data[0];
        POSSIBLY_UNUSED float *ref_before =mic_data[3];
        POSSIBLY_UNUSED uint32_t lock, start_time, end_time;
#endif

#ifdef TT_AFC
        // lock = int_lock();
        // start_time = hal_fast_sys_timer_get();
        afc_process(afc_st, ff, ref_before, ff, FIR_BLOCK_SIZE, _voice_assist_afc_handler);
        // end_time = hal_fast_sys_timer_get();
        // int_unlock(lock);
        // TRACE(0,"-----afc process---------------%d us, cpu freq: %d",FAST_TICKS_TO_US(end_time - start_time), hal_sysfreq_get());
#endif

#ifdef TT_NOISE_REDUCTION
        // lock = int_lock();
        // start_time = hal_fast_sys_timer_get();
        tt_ns_process(ns_st, ff, FIR_BLOCK_SIZE, _voice_assist_tt_noise_reduction_handler);
        // end_time = hal_fast_sys_timer_get();
        // int_unlock(lock);
        // TRACE(0,"-----ns process---------------%d us, cpu freq: %d",FAST_TICKS_TO_US(end_time - start_time), hal_sysfreq_get());
#endif
    }
    return 0;
}

#if (FIR_SAMPLE_RATE == 32000)
static int16_t dump_data[240];
static POSSIBLY_UNUSED void dump_fir_coeff(int32_t *fir_coeff_cache, uint32_t len)
{
    int32_t *tmp_p = fir_coeff_cache;
    for (uint32_t i = 0; i < 240; i++) {
        dump_data[i] = 0x5555;
    }
    audio_dump_add_channel_data(0, dump_data, 240);

    for (uint32_t j = 0; j < 240; j++) {
        dump_data[j] = *tmp_p++ >> 8;
    }
    audio_dump_add_channel_data(1, dump_data, 240);

    for (uint32_t j = 0; j < 232; j++) {
        dump_data[j] = *tmp_p++ >> 8;
    }
    audio_dump_add_channel_data(2, dump_data, 232);

    audio_dump_run();
}
#elif (FIR_SAMPLE_RATE == 16000)
static int16_t dump_data[120];

static void dump_fir_coeff(int32_t *fir_coeff_cache, uint32_t len)
{
    int32_t *tmp_p = fir_coeff_cache;
    for (uint32_t i = 0; i < 8; i++) {
        dump_data[i] = 0x5555;
    }

    for (uint32_t j = 8; j < FIR_BLOCK_SIZE; j++) {
        dump_data[j] = *tmp_p++ >> 8;
    }
    audio_dump_add_channel_data(0, dump_data, FIR_BLOCK_SIZE);
    for (uint32_t j = 0; j < FIR_BLOCK_SIZE; j++) {
        dump_data[j] = *tmp_p++ >> 8;
    }
    audio_dump_add_channel_data(1, dump_data, FIR_BLOCK_SIZE);

    audio_dump_run();
}
#endif

int voice_assist_fir_lms_process(process_frame_data_t *process_frame_data);

static int32_t _voice_assist_fir_lms_callback(void * buf, uint32_t len, void *other)
{
#ifdef VOICE_FIR_LMS_DUMP_LOCAL
    float ** input_data_tmp = buf;
    audio_dump_clear_up();
    for(int32_t i = 0; i < FIR_BLOCK_SIZE; i++) {
        tmp_data[i] = (int32_t)input_data_tmp[1][i] >> 5;
    }
    audio_dump_add_channel_data(0, tmp_data, FIR_BLOCK_SIZE);

    for(int32_t i = 0; i < FIR_BLOCK_SIZE; i++) {
        tmp_data[i] = (int32_t)input_data_tmp[2][i] >> 5;
    }
    audio_dump_add_channel_data(1, tmp_data, FIR_BLOCK_SIZE);

    for(int32_t i = 0; i < FIR_BLOCK_SIZE; i++) {
        tmp_data[i] = (int32_t)input_data_tmp[4][i] >> 5;
    }
    audio_dump_add_channel_data(2, tmp_data, FIR_BLOCK_SIZE);
    audio_dump_run();
#endif

    float **input_data = buf;

    anc_fir_lms_eq_process_f32(input_data, FIR_BLOCK_SIZE);

#if defined(APP_MCPP_CLI)
#ifdef FIR_LMS_AEC_ENABLED
    float *fb_mic = input_data[1];
    float *ref_before = input_data[3];
    // uint32_t start_time = hal_fast_sys_timer_get();
    anc_fir_lms_aec_process_f32(aec_st, fb_mic, ref_before, fb_mic);
    // TRACE(0,"--------------------%d us",FAST_TICKS_TO_US(hal_fast_sys_timer_get() - start_time));
    // TRACE(0, "[%s] Sys freq: %d", __func__, hal_sys_timer_calc_cpu_freq(5, 0));
#endif
    static float _temp_buf[MIC_INDEX_QTY * FIR_BLOCK_SIZE];
    for (uint32_t j = 0; j < MIC_INDEX_QTY; j++) {
        memcpy(_temp_buf + j * FIR_BLOCK_SIZE, input_data[j], FIR_BLOCK_SIZE * sizeof(uint32_t));
    }

    APP_MCPP_CAP_PCM_T pcm_cfg;
    memset(&pcm_cfg, 0, sizeof(pcm_cfg));
    pcm_cfg.in = _temp_buf;
    pcm_cfg.frame_len = len;
    pcm_cfg.out = _temp_buf;

    app_mcpp_capture_process(APP_MCPP_USER_VOICE_ASSIST_FIR_LMS, &pcm_cfg);

#elif defined(FIR_ADAPT_ANC_M55)
#else
    if (stop_flag == 0) {
        int loop_cnt = 1;
        int offset = 0;

        ASSERT(((len % FIR_BLOCK_SIZE) == 0), "[%s] the 120 data_len is error ", __func__);
        loop_cnt = len / FIR_BLOCK_SIZE;
        for (int i = 0; i < loop_cnt; i++) {
            float *ff_data[MAX_FF_CHANNEL_NUM] = {};
            float *fb_data[MAX_FB_CHANNEL_NUM] = {};
            float *talk_data[MAX_TALK_CHANNEL_NUM] = {};
            float *ref_data[MAX_REF_CHANNEL_NUM] = {};
            float *vpu_data = NULL;
            uint32_t j = 0;
            for (uint32_t i = 0; i < MAX_FF_CHANNEL_NUM; i++){
                ff_data[i] = input_data[j] + offset;
                j++;
            }
            for (uint32_t i = 0; i < MAX_FB_CHANNEL_NUM; i++){
                fb_data[i] = input_data[j] + offset;
                j++;
            }
            for (uint32_t i = 0; i < MAX_TALK_CHANNEL_NUM; i++){
                talk_data[i] = input_data[j] + offset;
                j++;;
            }
            for (uint32_t i = 0; i < MAX_REF_CHANNEL_NUM; i++){
                ref_data[i] = input_data[j] + offset;
                j++;
            }
        #if defined(ANC_ASSIST_VPU)
            vpu_data = input_data[j];
        #endif


            // TRACE(0, "[%s] fir lms process...", __func__);
            process_frame_data_t process_frame_data;

            process_frame_data.ff_mic = ff_data;
            process_frame_data.ff_ch_num = MAX_FF_CHANNEL_NUM;
            process_frame_data.fb_mic = fb_data;
            process_frame_data.fb_ch_num = MAX_FB_CHANNEL_NUM;
            process_frame_data.talk_mic = talk_data;
            process_frame_data.talk_ch_num = MAX_TALK_CHANNEL_NUM;
            process_frame_data.ref = ref_data;
            process_frame_data.ref_ch_num = MAX_REF_CHANNEL_NUM;
            process_frame_data.vpu_mic = vpu_data;
            process_frame_data.frame_len = FIR_BLOCK_SIZE;
            voice_assist_fir_lms_process(&process_frame_data);

            offset += FIR_BLOCK_SIZE;
        }
    } else {
        return 0;
    }
#endif
    return 0;
}

#if !defined(APP_MCPP_CLI) && !defined(FIR_ADAPT_ANC_M55)
void set_fir_cache(void)
{
    for (uint32_t i = 0; i < LOCAL_FIR_LEN; i++)
        fir_cache_cfg.anc_fir_cfg_ff_l.fir_coef[i]=fir_coeff_cache[i];
    anc_set_fir_cfg(&fir_cache_cfg, ANC_FEEDFORWARD);
}
#ifdef MC_FIR_LMS_ENABLED
void set_mc_fir_cache(void)
{
    for (uint32_t i = 0; i < LOCAL_FIR_LEN; i++)
        fir_cache_cfg.anc_fir_cfg_mc_l.fir_coef[i]=fir_coeff_cache[i];
    anc_set_fir_cfg(&fir_cache_cfg, ANC_MUSICCANCLE);
}
#endif
#endif

#if defined(FIR_ADAPT_ANC_M55) || defined(APP_MCPP_CLI)
// static int32_t *fir_coeff_cache = NULL;
static app_voice_assist_core_interact_data_t interact_data;

int32_t _voice_assist_fir_lms_return_res_handler(EventDetectionRes *res)
{
    uint8_t user = ANC_ASSIST_USER_FIR_LMS;
    uint8_t cmd = ANC_FF_FIR_LMS_CMD_RETURN_RES;
#if defined(APP_MCPP_CLI)
    interact_data.user = user;
    interact_data.sub_cmd = cmd;

    memcpy(interact_data.buf, res, sizeof(EventDetectionRes));
    interact_data.len = sizeof(EventDetectionRes);

    mcpp_capture_ctl_cmd_send_handler_done(APP_MCPP_USER_VOICE_ASSIST_FIR_LMS, &interact_data, sizeof(interact_data));
#elif defined(FIR_ADAPT_ANC_M55)
    anc_assist_dsp_send_result_to_bth(user, (uint8_t *)res, sizeof(EventDetectionRes), (uint8_t)cmd);
#endif
    return 0;
}

int32_t _voice_assist_anc_set_cfg_handler(app_anc_mode_t mode, enum ANC_TYPE_T anc_type, ANC_GAIN_TIME anc_gain_delay)
{
    static voice_assist_anc_cfg iir_cfg;

    uint8_t user = ANC_ASSIST_USER_FIR_LMS;
    uint8_t cmd = ANC_FF_FIR_LMS_CMD_SET_CFG;

    iir_cfg.mode = mode;
    iir_cfg.type = anc_type;
    iir_cfg.gain_delay = anc_gain_delay;

    // TRACE(0, "[%s] sub_cmd = %d", __FUNCTION__, cmd);
#if defined(APP_MCPP_CLI)
    interact_data.user = user;
    interact_data.sub_cmd = cmd;

    memcpy(interact_data.buf, &iir_cfg, sizeof(voice_assist_anc_cfg));
    interact_data.len = sizeof(voice_assist_anc_cfg);

    mcpp_capture_ctl_cmd_send_handler_done(APP_MCPP_USER_VOICE_ASSIST_FIR_LMS, &interact_data, sizeof(interact_data));
#elif defined(FIR_ADAPT_ANC_M55)
    anc_assist_dsp_send_result_to_bth(user, (uint8_t *)&iir_cfg, sizeof(voice_assist_anc_cfg), (uint8_t)cmd);
#endif
    return 0;
}

FIR_LMS_TEXT_LOCATION
int32_t _voice_assist_app_anc_switch_handler(app_anc_mode_t mode)
{
    anc_assist_user_t user = ANC_ASSIST_USER_FIR_LMS;
    uint32_t cmd = ANC_FF_FIR_LMS_CMD_SWITCH_MODE;

#if defined(APP_MCPP_CLI)
    interact_data.user = user;
    interact_data.sub_cmd = cmd;

    memcpy(interact_data.buf, &mode, sizeof(app_anc_mode_t));
    interact_data.len = sizeof(app_anc_mode_t);

    // TRACE(0, "[%s] sub_cmd = %d, mode = %d", __FUNCTION__, interact_data.sub_cmd, mode);
    mcpp_capture_ctl_cmd_send_handler_done(APP_MCPP_USER_VOICE_ASSIST_FIR_LMS, &interact_data, sizeof(interact_data));
#elif defined(FIR_ADAPT_ANC_M55)
    anc_assist_dsp_send_result_to_bth(user, (uint8_t *)&mode, sizeof(app_anc_mode_t), (uint8_t)cmd);
#endif

    return 0;
}

int32_t _voice_assist_set_iir_handler(enum ANC_TYPE_T anc_type, uint32_t index, uint8_t *coeffs, int32_t len)
{
    static voice_assist_anc_iir_cfg iir_cfg;

    uint8_t user = ANC_ASSIST_USER_FIR_LMS;
    uint8_t cmd = ANC_FF_FIR_LMS_CMD_SET_IIR;

    iir_cfg.type = anc_type;
    iir_cfg.index = index;
    memcpy(iir_cfg.coeffs, coeffs, len);
    iir_cfg.coeffs_len = len;

#if defined(APP_MCPP_CLI)
    interact_data.user = user;
    interact_data.sub_cmd = cmd;

    memcpy(interact_data.buf, &iir_cfg, sizeof(voice_assist_anc_iir_cfg));
    interact_data.len = sizeof(voice_assist_anc_iir_cfg);

    mcpp_capture_ctl_cmd_send_handler_done(APP_MCPP_USER_VOICE_ASSIST_FIR_LMS, &interact_data, sizeof(interact_data));
#elif defined(FIR_ADAPT_ANC_M55)
    anc_assist_dsp_send_result_to_bth(user, (uint8_t *)&iir_cfg, sizeof(voice_assist_anc_iir_cfg), (uint8_t)cmd);
#endif
    return 0;
}
#endif

#if defined(TT_NOISE_REDUCTION)
static int32_t _voice_assist_tt_noise_reduction_handler(float *coeffs, uint32_t num)
{
#ifdef VOICE_FIR_LMS_DUMP_REMOTE
    audio_dump_add_channel_data_f32(2, coeffs, num, -14);
#endif

    // fir default gain 0.5
    arm_scale_f32(coeffs, 0.5f, coeffs, num);

    anc_fir_set_coeff(ANC_FIR_CHANNEL_TT_L, coeffs, num);

    return 0;
}
#endif

#if defined(TT_AFC)
static int32_t _voice_assist_afc_handler(float *coeffs, uint32_t num)
{
#ifdef VOICE_FIR_LMS_DUMP_REMOTE
    audio_dump_add_channel_data_f32(2, coeffs, num, -14);
#endif

    anc_fir_set_coeff(ANC_FIR_CHANNEL_FF_L, coeffs, num);

    return 0;
}
#endif

static void _voice_assist_set_freq(int freq, int line);

int voice_assist_fir_lms_open(void *calib_gain_buf)
{
    static FIR_LMS_CALIB_GAIN calib_gain[FIR_CHANNEL_NUM];
    memcpy(&calib_gain, calib_gain_buf, sizeof(FIR_LMS_CALIB_GAIN)*FIR_CHANNEL_NUM);
    voice_assist_fir_lms_inst *ctx = voice_assist_get_ctx();

    POSSIBLY_UNUSED uint16_t frame_size = FIR_BLOCK_SIZE;
    POSSIBLY_UNUSED uint16_t blocks = 1;

    ext_heap_init(fir_ext_heap);
    ext_heap_init_event(fir_ext_heap_event);

    anc_ff_fir_lms_register_set_fir_freq_handler(_voice_assist_set_freq);
#ifdef __NuttX__
    _set_freq_state = SET_FREQ_NONE;
    _set_freq_destroy_lock();
    _set_freq_create_lock();
#endif

#ifdef FIR_LMS_AEC_ENABLED
    aec_st = anc_fir_lms_aec_init(FIR_SAMPLE_RATE, FIR_BLOCK_SIZE);
#endif

    TRACE(1,"0000 %d",ext_heap_get_used_buff_size());

#ifdef TT_NOISE_REDUCTION
    TtNsConfig ns_cfg;
    ns_cfg.alpha = 0.99;
    ns_cfg.denoise_dB = -12;
    for (uint32_t i = 0; i < ARRAY_SIZE(ns_cfg.noise_psd_dB); i++) {
        ns_cfg.noise_psd_dB[i] = -107 + 48;
    }
    ns_st = tt_ns_create(FIR_SAMPLE_RATE, TT_NS_TAPS, &ns_cfg, &ext_allocator);
    TRACE(1,"3333 %d",ext_heap_get_used_buff_size());// 5884 = 6K
#endif

#ifdef TT_AFC
    AfcConfig afc_cfg = {
        .delay = 0,
        .taps = 32,
    };
    afc_st = afc_create(FIR_SAMPLE_RATE, FIR_BLOCK_SIZE, &afc_cfg, &ext_allocator);
    TRACE(1,"4444 %d",ext_heap_get_used_buff_size());// 6164 - 5884 = 280
#endif

    event_st = event_detection_create(EVENT_DETECTION_SAMPLE_RATE, EVENT_DETECTION_FRAME_SIZE, &event_cfg, &ext_allocator_event,&event_res);
    TRACE(1,"1111 %d",ext_heap_event_get_used_buff_size());//33644 = 33K

#if defined(APP_MCPP_CLI) && defined(VOICE_ASSIST_FF_FIR_LMS_MULTI_THREAD)
    event_detection_thread_init();
    fir_lms_thread_init();
#endif

    for (uint32_t i = 0; i < FIR_CHANNEL_NUM; i++) {
#ifdef MC_FIR_LMS_ENABLED
        mc_fir_st[i] = anc_mc_fir_lms_create(FIR_SAMPLE_RATE, FIR_BLOCK_SIZE, &mc_cfg, &ext_allocator, &calib_gain[i]);
        TRACE(1,"2222 %d",ext_heap_get_used_buff_size());//52548 - 35232 = 17k
#endif
        fir_st[i] = anc_ff_fir_lms_create(FIR_SAMPLE_RATE, FIR_BLOCK_SIZE, &cfg ,&ext_allocator, &calib_gain[i]);//144604
        anc_ff_fir_lms_set_ff_fir_status(fir_st[i], event_st, true);
    // fir_coeff_cache = fir_lms_coeff_cache(fir_st);
        TRACE(1,"5555 %d",ext_heap_get_used_buff_size());// 158400 - 6164 = 152236(149k)
    }

    for (uint32_t i = 0; i < FIR_CHANNEL_NUM; i++) {
        ctx->anc_inst[i] = fir_st[i];
    }
    ctx->last_stage = ANC_FF_FIR_LMS_STAGE_IDLE;
    ctx->stage = ANC_FF_FIR_LMS_STAGE_IDLE;
    ctx->frame_count = 0;
    ctx->pnc_total_frame = 500 * 2 / 15;
    ctx->switch_total_frame = 700 * 2 / 15;
    ctx->anc_total_frame = 500 * 2 / 15;
    ctx->switch_mode_frame = 50 * 2 / 15;
    ctx->adapt_iir_frame = 1; // 1000 * 2 / 15;
    ctx->mips_needed = 40;
    ctx->ctrl = 0;
    memcpy(&ctx->calib_gain, calib_gain, sizeof(FIR_LMS_CALIB_GAIN)*FIR_CHANNEL_NUM);
#ifdef VOICE_FIR_LMS_DUMP_REMOTE
    audio_dump_init(FIR_BLOCK_SIZE, sizeof(short), 3);
#endif
    return 0;
}

void voice_assist_ff_fir_lms_close(void)
{
    voice_assist_fir_lms_inst *ctx = voice_assist_get_ctx();
    TRACE(0,"[%s]...",__func__);
    for (uint32_t i = 0; i < FIR_CHANNEL_NUM; i++) {
        anc_ff_fir_lms_destroy(fir_st[i]);
        fir_st[i] = NULL;
        ctx->anc_inst[i] = NULL;
    }
}

#ifdef MC_FIR_LMS_ENABLED
void voice_assist_mc_fir_lms_close(void)
{
    TRACE(0,"[%s]...",__func__);
    for (uint32_t i = 0; i < FIR_CHANNEL_NUM; i++) {
        anc_mc_fir_lms_destroy(mc_fir_st[i]);
        mc_fir_st[i] = NULL;
    }
}
#endif

void voice_assist_event_detection_close(void)
{
    TRACE(0,"[%s]...",__func__);
    event_detection_destroy(event_st);
    event_st = NULL;
}

int voice_assist_fir_lms_close(void)
{
    TRACE(0, "[%s] fir lms deinit...", __func__);
#ifdef FIR_LMS_AEC_ENABLED
    anc_fir_lms_aec_destroy(aec_st);
#endif

#ifdef TT_NOISE_REDUCTION
    tt_ns_destroy(ns_st);
#endif

#ifdef TT_AFC
    afc_destroy(afc_st);
#endif

#if defined(APP_MCPP_CLI) && defined(VOICE_ASSIST_FF_FIR_LMS_MULTI_THREAD)
    app_voice_assist_event_detection_thread_close();
    app_voice_assist_fir_lms_thread_close();
#else
    voice_assist_event_detection_close();
    voice_assist_ff_fir_lms_close();
#ifdef MC_FIR_LMS_ENABLED
    voice_assist_mc_fir_lms_close();
#endif
#endif

#if defined(APP_MCPP_CLI)
    anc_fir_coeff_config_deinit();
#endif

    app_sysfreq_req(APP_SYSFREQ_USER_FIR_LMS, APP_SYSFREQ_32K);
    ext_heap_deinit();
    ext_heap_deinit_event();
    return 0;
}

int voice_assist_fir_lms_reset(void)
{
    TRACE(0, "[%s] fir lms reset...", __func__);
    for (uint32_t i = 0; i < FIR_CHANNEL_NUM; i++) {
        anc_ff_fir_lms_reset(fir_st[i], 0, AUD_CHANNEL_MAP_CH0 << i);
    }
    return 0;
}

int voice_assist_fir_lms_get_freq(void)
{
    voice_assist_fir_lms_inst *ctx = voice_assist_get_ctx();

    return ctx->mips_needed;
}

/* same as the define in anc_ff_fir_lms.c */
#define ADAPTIVE_MC_MIPS (22)
#define ADAPTIVE_FF_MIPS (70)

static void _voice_assist_set_freq(int freq, int line)
{
    #if defined(APP_MCPP_CLI)
#if defined(__NuttX__)
#if defined(VOICE_ASSIST_FF_FIR_LMS_MULTI_THREAD)
    osThreadId aec_thread_id = mcpp_srv_get_thread_id();
    // fir lms or tt ns
    osThreadId fir_lms_thread_id = fir_lms_get_thread_id();
    osThreadId event_detection_thread_id = event_detection_get_thread_id();
#endif
    _set_freq_lock();
    voice_assist_fir_lms_inst *ctx = voice_assist_get_ctx();
    uint32_t mode = app_anc_table_get_custom_mode(ctx->mode);
    if (mode == ANC_CUSTOM_MODE_FIR_ANC) {
        if (ADAPTIVE_MC_MIPS == freq) {
            if (SET_FREQ_FIR_LOW == _set_freq_state) {
                _set_freq_unlock();
                return;
            }
#if defined(VOICE_ASSIST_FF_FIR_LMS_MULTI_THREAD)
            /* aec first:priority:event must > aec */
            if (0 == _thread_switch_to_cpu(aec_thread_id, SENSOR_CPU1)) {
                osDelay(50);
            }
            /* fir_lms second:priority:fir_lms > aec */
            osThreadSetPriority(event_detection_thread_id, osPriorityNormal);
            _thread_switch_to_cpu(fir_lms_thread_id, SENSOR_CPU0);
#endif
            hal_sysfreq_req(HAL_SYSFREQ_USER_DSP, FREQ_FIR_LOW);
            _set_freq_state = SET_FREQ_FIR_LOW;
        } else if (ADAPTIVE_FF_MIPS == freq) {
            if (SET_FREQ_FIR_HIGH == _set_freq_state) {
                _set_freq_unlock();
                return;
            }
            hal_sysfreq_req(HAL_SYSFREQ_USER_DSP, FREQ_FIR_HIGH);

#if defined(VOICE_ASSIST_FF_FIR_LMS_MULTI_THREAD)
            /* fir_lms first:priority:fir_lms > aec */
            _thread_switch_to_cpu(fir_lms_thread_id, SENSOR_CPU1);
            /* event second:priority:event must > aec */
            osThreadSetPriority(event_detection_thread_id, osPriorityAboveNormal);
            osDelay(10);
            _thread_switch_to_cpu(aec_thread_id, SENSOR_CPU0);
#endif
            _set_freq_state = SET_FREQ_FIR_HIGH;
        }
    } else {
        if (ADAPTIVE_MC_MIPS == freq) {
            if (SET_FREQ_TT_LOW == _set_freq_state) {
                _set_freq_unlock();
                return;
            }

#if defined(VOICE_ASSIST_FF_FIR_LMS_MULTI_THREAD)
            _thread_switch_to_cpu(fir_lms_thread_id, SENSOR_CPU1);
            /* event second:priority:event must > aec */
            osThreadSetPriority(event_detection_thread_id, osPriorityAboveNormal);
            osDelay(1);
            _thread_switch_to_cpu(aec_thread_id, SENSOR_CPU0);
#endif

            hal_sysfreq_req(HAL_SYSFREQ_USER_DSP, FREQ_FIR_LOW);
            _set_freq_state = SET_FREQ_TT_LOW;
        } else if (ADAPTIVE_FF_MIPS == freq) {
            if (SET_FREQ_TT_HIGH == _set_freq_state) {
                _set_freq_unlock();
                return;
            }
            hal_sysfreq_req(HAL_SYSFREQ_USER_DSP, FREQ_FIR_HIGH);

#if defined(VOICE_ASSIST_FF_FIR_LMS_MULTI_THREAD)
            _thread_switch_to_cpu(fir_lms_thread_id, SENSOR_CPU1);
            /* event second:priority:event must > aec */
            osThreadSetPriority(event_detection_thread_id, osPriorityAboveNormal);
            osDelay(1);
            _thread_switch_to_cpu(aec_thread_id, SENSOR_CPU0);
#endif
            _set_freq_state = SET_FREQ_TT_HIGH;
        }
    }
    _set_freq_unlock();
    TRACE(4, "[%s]:mode:%d,%d,%d", __func__, mode, freq, line);
#endif
#else
    voice_assist_fir_lms_inst *ctx = voice_assist_get_ctx();

    if (ctx->mips_needed != freq) {
        ctx->mips_needed = freq;
#if defined(FIR_ADAPT_ANC_M55)
        anc_assist_frequency_update();
#endif
    }
#endif
}

int voice_assist_fir_lms_set_cfg(uint32_t ctrl, uint8_t *tgt_cfg, uint32_t ptr_len)
{
    TRACE(0, "[%s] ctrl %d", __func__, ctrl);

    switch (ctrl) {
        case ANC_FF_FIR_LMS_CTRL_SET_ANC_MODE:
        {
            voice_assist_fir_lms_inst *ctx = voice_assist_get_ctx();
            app_anc_mode_t *mode = (app_anc_mode_t *)tgt_cfg;
            ctx->mode = *mode;
            TRACE(0, "[%s] mode = %d", __FUNCTION__, ctx->mode);
            if (ctx->stage != ANC_FF_FIR_LMS_STAGE_IDLE) {
                if (ANC_FF_FIR_LMS_STAGE_NORMAL_ANC > ctx->stage
                    && ANC_FF_FIR_LMS_STAGE_SKIP < ctx->stage) {
                    _voice_assist_set_freq(ADAPTIVE_FF_MIPS, __LINE__);
                } else {
                    _voice_assist_set_freq(ADAPTIVE_MC_MIPS, __LINE__);
                }
            }
#if defined(APP_MCPP_CLI)
            //if switch anc mode, init fir again.
            anc_fir_coeff_config_init();
#endif
            break;
        }
        case ANC_FF_FIR_LMS_CTRL_SET_FIR_FLAG:
        {
            uint32_t *flag = (uint32_t *)tgt_cfg;
            event_detection_set_fir_flag(event_st, *flag);
            break;
        }
        case ANC_FF_FIR_LMS_CTRL_SET_CALIB_GAIN:
        {
            voice_assist_fir_lms_inst *ctx = voice_assist_get_ctx();
            memcpy(&ctx->calib_gain, tgt_cfg, sizeof(FIR_LMS_CALIB_GAIN) * FIR_CHANNEL_NUM);
            ctx->ctrl |= (0x1 << ctrl);
            break;
        }
        case ANC_FF_FIR_LMS_CTRL_SET_FB_GAIN:
        {
            voice_assist_fir_lms_inst *ctx = voice_assist_get_ctx();
            float *gain = (float *)tgt_cfg;
            ctx->fb_gain = *gain;
            ctx->ctrl |= (0x1 << ctrl);
            break;
        }
        case ANC_FF_FIR_LMS_CTRL_SET_TT_NS_CFG:
        {
#if defined(TT_NOISE_REDUCTION)
            tt_ns_set_cfg(ns_st, (TtNsConfig *)tgt_cfg);
#endif
            break;
        }
        case ANC_FF_FIR_LMS_CTRL_SET_TT_ALGO_STATUS:
        {
#if defined(TT_NOISE_REDUCTION)
            voice_assist_fir_lms_inst *ctx = voice_assist_get_ctx();
            ctx->tt_algo_status = *((int32_t *)tgt_cfg);
#endif
            break;
        }
        default:
        {
            voice_assist_fir_lms_inst *ctx = voice_assist_get_ctx();
            ctx->ctrl |= (0x1 << ctrl);
            TRACE(0, "[%s] receive ctrl %d", __FUNCTION__, ctrl);
            break;
        }
    }

    return 0;
}

FIR_LMS_TEXT_LOCATION
int voice_assist_fir_lms_update_cfg(voice_assist_fir_lms_inst *ctx)
{
    for (uint32_t i = 0; i < ANC_FF_FIR_LMS_CTRL_NUM; i++) {
        if (ctx->ctrl & (0x1 << i)) {
            TRACE(0, "[%s] process ctrl %d", __FUNCTION__, i);

            switch (i) {
                case ANC_FF_FIR_LMS_CTRL_SET_FF_FIR_ON:
                    for (uint32_t i = 0; i < FIR_CHANNEL_NUM; i++) {
                        anc_ff_fir_lms_set_ff_fir_status(fir_st[i], event_st,true);
#ifdef MC_FIR_LMS_ENABLED
                        anc_ff_fir_lms_set_mc_fir_status(mc_fir_st[i], true);
#endif
                    }
                    break;
                case ANC_FF_FIR_LMS_CTRL_SET_FF_FIR_OFF:
                    for (uint32_t i = 0; i < FIR_CHANNEL_NUM; i++) {
                        anc_ff_fir_lms_set_ff_fir_status(fir_st[i], event_st, false);
#ifdef MC_FIR_LMS_ENABLED
                        anc_ff_fir_lms_set_mc_fir_status(mc_fir_st[i], false);
#endif
                    }
                    break;
                case ANC_FF_FIR_LMS_CTRL_START_FIR:
                {
                    for (uint32_t i = 0; i < FIR_CHANNEL_NUM; i++) {
                        anc_ff_fir_lms_reset_ff_FIR(fir_st[i], AUD_CHANNEL_MAP_CH0 << i);
#ifdef MC_FIR_LMS_ENABLED
                        anc_ff_fir_lms_reset_mc_FIR(mc_fir_st[i], AUD_CHANNEL_MAP_CH0 << i);
#endif
                    }
                    ctx->stage = ANC_FF_FIR_LMS_STAGE_WAITING_ADAPTIVE_ANC_ON;
                    ctx->frame_count = 0;
                    break;
                }
                case ANC_FF_FIR_LMS_CTRL_SET_CALIB_GAIN:
                {
                    for (uint32_t i = 0; i < FIR_CHANNEL_NUM; i++) {
                        anc_ff_fir_lms_set_ff_fir_calib_gain(fir_st[i], &ctx->calib_gain[i]);
#ifdef MC_FIR_LMS_ENABLED
                        anc_ff_fir_lms_set_mc_fir_calib_gain(mc_fir_st[i], &ctx->calib_gain[i]);
#endif
                     }
                    break;
                }
                case ANC_FF_FIR_LMS_CTRL_SET_FB_GAIN:
                {
                    for (uint32_t i = 0; i < FIR_CHANNEL_NUM; i++) {
                        anc_ff_fir_lms_update_FB_gain(fir_st[i], ctx->fb_gain);
                    }
                    break;
                }
                case ANC_FF_FIR_LMS_CTRL_STOP_FIR:
                {
                    // if reset ANC_FEEDFORWARD which will cause pop noise (fir coeff change abruptly)
#ifdef MC_FIR_LMS_ENABLED
                    for(uint32_t i = 0; i < FIR_CHANNEL_NUM; i++){
                        anc_ff_fir_lms_reset_mc_FIR(mc_fir_st, AUD_CHANNEL_MAP_CH0 << i);
                    }
#endif
                    ctx->stage = ANC_FF_FIR_LMS_STAGE_NORMAL_ANC;
                    ctx->frame_count = 0;
                    break;
                }
                default:
                    TRACE(0, "[%s] invalid ctrl %d", __FUNCTION__, i);
                    break;
            }
            ctx->ctrl &= ~(0x1 << i);
        }
    }

    return 0;
}

int voice_assist_fir_lms_process(process_frame_data_t *process_frame_data)
{
#ifdef VOICE_FIR_LMS_DUMP_REMOTE
        audio_dump_clear_up();
        for(int32_t i = 0; i < FIR_BLOCK_SIZE; i++) {
            tmp_data[i] = (int32_t)process_frame_data->ff_mic[1][i] >> 5;
        }
        audio_dump_add_channel_data(0, tmp_data, FIR_BLOCK_SIZE);

        for(int32_t i = 0; i < FIR_BLOCK_SIZE; i++) {
            tmp_data[i] = (int32_t)process_frame_data->fb_mic[1][i] >> 5;
        }
        audio_dump_add_channel_data(1, tmp_data, FIR_BLOCK_SIZE);

        for(int32_t i = 0; i < FIR_BLOCK_SIZE; i++) {
            tmp_data[i] = (int32_t)process_frame_data->ref[1][i] >> 5;
        }
        audio_dump_add_channel_data(2, tmp_data, FIR_BLOCK_SIZE);
        audio_dump_run();
#endif

        // float *mic_data[6] = {ff_mic_buf[0], fb_mic_buf[0], talk_mic_buf[0],ref_mic_buf[0], ref_mic_buf[1], process_frame_data->vpu_mic + offset,};
        // TRACE(0, "[%s] fir lms process...", __func__);
#ifdef FIR_LMS_AEC_ENABLED
    voice_assist_fir_lms_inst *ctx = voice_assist_get_ctx();
    if (app_anc_table_get_custom_mode(ctx->mode) == ANC_CUSTOM_MODE_FIR_ANC) {
        // uint32_t start_time = hal_fast_sys_timer_get();
        anc_fir_lms_aec_process_f32(aec_st, process_frame_data->fb_mic[0], process_frame_data->ref[0], process_frame_data->fb_mic[0]);
        // TRACE(0,"-----aec process---------------%d us",FAST_TICKS_TO_US(hal_fast_sys_timer_get() - start_time));
    }
#endif

    EventController event_control;
#if defined(APP_MCPP_CLI) && defined(VOICE_ASSIST_FF_FIR_LMS_MULTI_THREAD)
    app_voice_assist_event_detection_thread_process(process_frame_data, &event_control);
#else
    voice_assist_event_detection_process(process_frame_data, &event_control);
#endif
        // event_control.update_Pz = 1;
        // event_control.update_Sz = 1;
        // event_control.update_MC = 0;
        // event_control.update_W = 1;
        // event_control.fir_flag = 1;
        // event_control.fir_open_again = 0;

        //mc fir ff fir
#if defined(APP_MCPP_CLI) && defined(VOICE_ASSIST_FF_FIR_LMS_MULTI_THREAD)
        // app_voice_assist_fir_lms_thread_process(process_frame_data, &event_control);
        // voice_assist_anc_ff_fir_lms_process(process_frame_data, &event_control);
#else
#ifdef MC_FIR_LMS_ENABLED
        voice_assist_anc_mc_fir_lms_process(process_frame_data, &event_control);
#endif
        voice_assist_anc_ff_fir_lms_process(process_frame_data, &event_control);
#endif


    return 0;
}


#if defined(FIR_ADAPT_ANC_M55)
#include "app_voice_assist_dsp.h"
app_voice_assist_dsp_t voice_assist_fir_lms_m55 = {
    .open          = voice_assist_fir_lms_open,
    .close         = voice_assist_fir_lms_close,
    .reset         = voice_assist_fir_lms_reset,
    .ctrl          = voice_assist_fir_lms_set_cfg,
    .process       = voice_assist_fir_lms_process,
    .get_freq      = voice_assist_fir_lms_get_freq,
};

int32_t assist_fir_lms_dsp_init(void)
{
    TRACE(1, "[%s]fir lms m55 init", __func__);
    anc_assist_dsp_register(ANC_ASSIST_USER_FIR_LMS, &voice_assist_fir_lms_m55, ANC_ASSIST_USER_FS_32K);
    return 0;
}
#endif
#endif