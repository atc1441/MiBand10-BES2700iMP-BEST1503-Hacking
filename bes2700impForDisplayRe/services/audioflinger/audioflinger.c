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
#include "plat_addr_map.h"
#include "audioflinger.h"
#include "analog.h"
#include "cmsis.h"
#include "codec_tlv32aic32.h"
#ifdef AF_DEVICE_INT_CODEC
#include "codec_int.h"
#endif
#include "hal_btpcm.h"
#include "hal_cmu.h"
#include "hal_codec.h"
#include "hal_dma.h"
#include "hal_i2s.h"
#include "hal_spdif.h"
#include "hal_tdm.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "pmu.h"
#include "string.h"
#include "tgt_hardware.h"
#ifdef DMA_RPC_CLI
#include "stream_dma_rpc.h"
#endif

#include "eq_cfg.h"

#ifdef RTOS
#include "cmsis_os.h"
#else
#include "hal_sleep.h"
#endif

#if defined(AUDIO_OUTPUT_I2S_SW_GAIN)
#include"af_stream_sw_gain.h"
#endif

#if defined(AUDIO_OUTPUT_SW_LIMITER) || defined(AUDIO_OUTPUT_DAC2_SW_LIMITER) \
    || defined(AUDIO_OUTPUT_DAC3_SW_LIMITER)
#include "floatlimiter.h"
#endif

#if defined(CHIP_BEST2001) && defined(FPGA)
#undef FPGA
#endif

#define AF_TRACE_DEBUG()   // TRACE(2,"%s:%d\n", __func__, __LINE__)

//#define AF_STREAM_ID_0_PLAYBACK_FADEOUT

//#define CORRECT_SAMPLE_VALUE

#ifdef FPGA
#define AF_DEVICE_EXT_CODEC
#endif

#ifdef AUDIO_OUTPUT_DC_CALIB
#ifdef CHIP_BEST1000
#define AUDIO_OUTPUT_DC_CALIB_SW
#define AUDIO_OUTPUT_PA_ON_FADE_IN
//#define AUDIO_OUTPUT_PA_OFF_FADE_OUT
#elif (defined(__TWS__) || defined(IBRT)) && !(defined(ANC_APP) || defined(AUDIO_OUTPUT_DAC2) \
                                                || defined(AUDIO_OUTPUT_DAC3))
#define AUDIO_OUTPUT_PA_ON_FADE_IN
#endif // !CHIP_BEST1000 && __TWS__
#elif defined(AUDIO_OUTPUT_DC_CALIB_ANA) && (defined(__TWS__) || defined(IBRT)) \
    && !(defined(ANC_APP) || defined(AUDIO_OUTPUT_DAC2) || defined(AUDIO_OUTPUT_DAC3))
#define AUDIO_OUTPUT_PA_ON_FADE_IN
#endif // !AUDIO_OUTPUT_DC_CALIB && AUDIO_OUTPUT_DC_CALIB_ANA && __TWS__

#if (defined(AUDIO_OUTPUT_PA_ON_FADE_IN) || defined(AUDIO_OUTPUT_PA_OFF_FADE_OUT)) \
    && (defined(AUDIO_OUTPUT_DAC2) || defined(AUDIO_OUTPUT_DAC3))
#error "AUDIO_OUTPUT_PA_ON_FADE_IN or AUDIO_OUTPUT_PA_OFF_FADE_OUT will" \
        "conflict with AUDIO_OUTPUT_DAC2 or AUDIO_OUTPUT_DAC3 "
#endif

#define AF_FADE_OUT_SIGNAL_ID           15
#define AF_FADE_IN_SIGNAL_ID            14

/* config params */
#define AF_CODEC_INST HAL_CODEC_ID_0
#define AF_BTPCM_INST HAL_BTPCM_ID_0

#define AF_CPU_WAKE_USER                HAL_CPU_WAKE_LOCK_USER_AUDIOFLINGER

#define AF_CODEC_RIGHT_CHAN_ATTN        0.968277856 // -0.28 dB

#define AF_CODEC_VOL_UPDATE_STEP        0.00002

#define AF_CODEC_DC_MAX_SCALE           32767
#define AF_CODEC_DC_MAX_SCALE_2         0x7FFFFF // reg[23:0],singned

#define AF_CODEC_DC_STABLE_INTERVAL     MS_TO_TICKS(4)
#ifdef AUDIO_OUTPUT_PA_OFF_FADE_OUT
#define AF_CODEC_PA_RESTART_INTERVAL    MS_TO_TICKS(160)
#else
#define AF_CODEC_PA_RESTART_INTERVAL    MS_TO_TICKS(8)
#endif

// The following might have been defined in tgt_hardware.h
#ifndef AF_CODEC_FADE_IN_MS
#define AF_CODEC_FADE_IN_MS             20
#endif
#ifndef AF_CODEC_FADE_OUT_MS
#define AF_CODEC_FADE_OUT_MS            20
#endif
#define AF_CODEC_FADE_MIN_SAMPLE_CNT    200

#define PP_PINGPANG(v)                  (v == PP_PING ? PP_PANG : PP_PING)

#ifndef AF_STACK_SIZE
#if defined(AUDIO_ANC_FB_ADJ_MC) || defined(BLE_AUDIO_ENABLED)|| defined(AUDIO_ADJ_EQ)
#define AF_STACK_SIZE                   (1024*10)
#elif defined(BLE_WALKIE_TALKIE)
#define AF_STACK_SIZE                   (1024*32)
#else
#define AF_STACK_SIZE                   (1024*4)
#endif
#endif

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP)
#define DYNAMIC_AUDIO_BUFFER_COUNT
#endif

#ifdef DYNAMIC_AUDIO_BUFFER_COUNT
#define MAX_AUDIO_BUFFER_COUNT          16
#define MIN_AUDIO_BUFFER_COUNT          2
#define AUDIO_BUFFER_COUNT              (role->dma_desc_cnt)
#else
#define MAX_AUDIO_BUFFER_COUNT          4
#define AUDIO_BUFFER_COUNT              MAX_AUDIO_BUFFER_COUNT
#if (AUDIO_BUFFER_COUNT & 0x1)
#error "AUDIO_BUFFER_COUNT must be an even number"
#endif
#endif

/* internal use */
enum AF_BOOL_T{
    AF_FALSE = 0,
    AF_TRUE = 1
};

enum AF_RESULT_T{
    AF_RES_SUCCESS = 0,
    AF_RES_FAILD = 1
};

enum AF_DAC_PA_STATE_T {
    AF_DAC_PA_NULL,
    AF_DAC_PA_ON_TRIGGER,
    AF_DAC_PA_ON_SOFT_START,
    AF_DAC_PA_OFF_TRIGGER,
    AF_DAC_PA_OFF_SOFT_END,
    AF_DAC_PA_OFF_SOFT_END_DONE,
    AF_DAC_PA_OFF_DC_START,
};

//status machine
enum AF_STATUS_T{
    AF_STATUS_NULL = 0x00,
    AF_STATUS_OPEN_CLOSE = 0x01,
    AF_STATUS_STREAM_OPEN_CLOSE = 0x02,
    AF_STATUS_STREAM_START_STOP = 0x04,
    AF_STATUS_STREAM_PAUSE_RESTART = 0x08,
    AF_STATUS_MASK = 0x0F,
};

struct af_stream_ctl_t{
    enum AF_PP_T pp_index;      //pingpong operate
    uint8_t pp_cnt;             //use to count the lost signals
    uint8_t status;             //status machine
    enum AUD_STREAM_USE_DEVICE_T use_device;

    uint32_t hdlr_intvl_ticks;
    uint32_t prev_hdlr_time;
    bool first_hdlr_proc;
    bool intvl_check_en;
};

struct af_stream_cfg_t {
    //used inside
    struct af_stream_ctl_t ctl;

    //dma buf parameters, RAM can be alloced in different way
    uint8_t *dma_buf_ptr;
    uint32_t dma_buf_size;

    //store stream cfg parameters
    struct AF_STREAM_CONFIG_T cfg;

    //dma cfg parameters
#ifdef DYNAMIC_AUDIO_BUFFER_COUNT
    uint8_t dma_desc_cnt;
#endif
    struct HAL_DMA_DESC_T dma_desc[MAX_AUDIO_BUFFER_COUNT];
    struct HAL_DMA_CH_CFG_T dma_cfg;
    uint32_t dma_base;

    //callback function
    AF_STREAM_HANDLER_T handler;
};

static struct af_stream_cfg_t af_stream[AUD_STREAM_ID_NUM][AUD_STREAM_NUM];
static uint8_t af_sig_lost_cnt[AUD_STREAM_ID_NUM][AUD_STREAM_NUM];

enum AF_CODEC_PLAY_HDLR_ID_T {
    AF_CODEC_PLAY_HDLR_ID_0,
#ifdef AUDIO_OUTPUT_DAC2
    AF_CODEC_PLAY_HDLR_ID_1,
#endif
#ifdef AUDIO_OUTPUT_DAC3
    AF_CODEC_PLAY_HDLR_ID_2,
#endif
    AF_CODEC_PLAY_HDLR_ID_QTY,
};

static AF_CODEC_PLAYBACK_POST_HANDLER_T codec_play_post_hdlr[AF_CODEC_PLAY_HDLR_ID_QTY];

typedef struct {
    float coefs_b[3];
    float coefs_a[3];
    float history_x[2];
    float history_y[2];
}SW_GAIN_IIR_T;

//#define SW_GAIN_OPTI_MIPS_FOR_ALIGN_4

#ifdef AUDIO_OUTPUT_SW_GAIN
#ifndef AUDIO_OUTPUT_SW_GAIN_BEFORE_DRC
const
#endif
static bool dac1_sw_gain_enabled = true;
volatile static float dac1_saved_output_coef, dac1_algo_gain, custom_eq_peak = 1.f;
#ifdef AUDIO_OUTPUT_SW_LIMITER
static uint8_t FloatLimiterMem[FL_STATE_SIZE];
static FloatLimiterPtr FloatLimiterP;
#else
SW_GAIN_IIR_T sw_gain_iir=
    {
        // .coefs_b={0.00000042780818597736f,   0.00000085561637195472f,   0.00000042780818597736f},
        // .coefs_a={1.00000000000000000000f, -1.99738371810092970000f,   0.99738542933367369000f},
        .coefs_b={0.0000452390567342137f, 0.0001890478113468427f, 0.00009452390567342137f},
        .coefs_a={1.00000000000000000000f, -1.961110637819907f, 0.961488733442601f},
        .history_x={0,0},
        .history_y={0,0},
    };

typedef struct {
    uint32_t samples;
    float step;
    float curr_gain;
    float new_gain;
    bool finished;
} fade_context_t;

static fade_context_t dac1_fade;

void fade_reset(fade_context_t *ctx, uint32_t sample_rate, uint32_t ms)
{
    ctx->samples = (uint32_t)(sample_rate * ms / 1000);
    ctx->step = 0.f;
    ctx->curr_gain = 1.f;
    ctx->new_gain = 1.f;
    ctx->finished = true;
}

void fade_update_gain(fade_context_t *ctx, float gain)
{
    if (ABS(gain - ctx->new_gain) < 1e-4) {
        return;
    }

    ctx->new_gain = gain;
    ctx->step = (ctx->new_gain - ctx->curr_gain) / ctx->samples;
    ctx->finished = false;
}

float fade_smooth_gain(fade_context_t *ctx, float gain)
{
    if (ctx->finished)
        return ctx->curr_gain;

    ctx->curr_gain = ctx->curr_gain + ctx->step;

    if (ctx->step > 0 && ctx->curr_gain > gain) {
        ctx->curr_gain = gain;
        ctx->finished = true;
    } else if (ctx->step < 0 && ctx->curr_gain < gain) {
        ctx->curr_gain = gain;
        ctx->finished = true;
    }

    return ctx->curr_gain;
}

#endif
#endif

#ifdef AUDIO_OUTPUT_DAC2_SW_GAIN
volatile static float dac2_saved_output_coef;
#ifdef AUDIO_OUTPUT_DAC2_SW_LIMITER
static uint8_t Dac2FloatLimiterMem[FL_STATE_SIZE];
static FloatLimiterPtr Dac2FloatLimiterP;
#else
SW_GAIN_IIR_T dac2_sw_gain_iir=
    {
        // .coefs_b={0.00000042780818597736f,   0.00000085561637195472f,   0.00000042780818597736f},
        // .coefs_a={1.00000000000000000000f,  -1.99738371810092970000f,   0.99738542933367369000f},
        .coefs_b={0.0000452390567342137f, 0.0001890478113468427f, 0.00009452390567342137f},
        .coefs_a={1.00000000000000000000f, -1.961110637819907f, 0.961488733442601f},
        .history_x={0,0},
        .history_y={0,0},
    };
#endif
#endif /* AUDIO_OUTPUT_DAC2_SW_GAIN */

#ifdef AUDIO_OUTPUT_DAC3_SW_GAIN
volatile static float dac3_saved_output_coef;
#ifdef AUDIO_OUTPUT_DAC3_SW_LIMITER
static uint8_t Dac3FloatLimiterMem[FL_STATE_SIZE];
static FloatLimiterPtr Dac3FloatLimiterP;
#else
SW_GAIN_IIR_T dac3_sw_gain_iir=
    {
        // .coefs_b={0.00000042780818597736f,   0.00000085561637195472f,   0.00000042780818597736f},
        // .coefs_a={1.00000000000000000000f,  -1.99738371810092970000f,   0.99738542933367369000f},
        .coefs_b={0.0000452390567342137f, 0.0001890478113468427f, 0.00009452390567342137f},
        .coefs_a={1.00000000000000000000f, -1.961110637819907f, 0.961488733442601f},
        .history_x={0,0},
        .history_y={0,0},
    };
#endif
#endif /* AUDIO_OUTPUT_DAC3_SW_GAIN */

#ifdef AUDIO_OUTPUT_DC_CALIB
static bool dac_dc_valid;
#ifdef AUDIO_OUTPUT_DC_CALIB_SW
static int16_t dac_dc[2];
#endif
#endif

#if defined(AUDIO_OUTPUT_PA_ON_FADE_IN) || defined(AUDIO_OUTPUT_PA_OFF_FADE_OUT)
static volatile enum AF_DAC_PA_STATE_T dac_pa_state;
static uint32_t dac_dc_start_time;
static uint32_t dac_pa_stop_time;
#endif

#if defined(AUDIO_OUTPUT_PA_OFF_FADE_OUT) && defined(RTOS)
static osThreadId fade_thread_id;
#ifdef AF_STREAM_ID_0_PLAYBACK_FADEOUT
#error "Cannot enable 2 kinds of fade out codes at the same time"
#endif
#endif

#ifdef RTOS

#ifdef AF_STREAM_ID_0_PLAYBACK_FADEOUT

#define AF_STREAM_ID_0_PLAYBACK_FADEOUT_TIMEOUT (100)

struct af_stream_fade_out_t{
    bool stop_on_process;
    uint8_t stop_process_cnt;
    osThreadId stop_request_tid;
    uint32_t need_fadeout_samples;
    uint32_t need_fadeout_samples_processed;
    float step;
    float weight;
};

static struct af_stream_fade_out_t af_stream_fade_out ={
                                                .stop_on_process = false,
                                                .stop_process_cnt = 0,
                                                .stop_request_tid = NULL,
                                                .need_fadeout_samples = 0,
                                                .need_fadeout_samples_processed = 0,
};
#endif

static osThreadId af_thread_tid = NULL;

static void af_thread(void const *argument);
osThreadDef(af_thread, osPriorityHigh, 1, AF_STACK_SIZE, "audio_flinger");
static int af_default_priority;

osMutexId audioflinger_mutex_id = NULL;
osMutexDef(audioflinger_mutex);

#else // !RTOS

static volatile uint32_t af_flag_lock;

#endif // RTOS

#ifdef AUDIO_RMS_MONITOR_ENABLE
extern uint32_t rms_debug_cnt;
#endif

static AF_IRQ_NOTIFICATION_T irq_notif;

#ifdef CODEC_DSD
static bool af_dsd_enabled;
#endif

#ifdef CODEC_ERROR_HANDLING
static void af_supervisor_init(void);
static void af_ping_supervisor(enum AUD_STREAM_ID_T streamId, enum AUD_STREAM_T streamType);
static void af_update_stream_state(enum AUD_STREAM_ID_T streamId, enum AUD_STREAM_T streamType, bool isOn);
#endif

void af_lock_thread(void)
{
    void * POSSIBLY_UNUSED lr = __builtin_return_address(0);
#ifdef RTOS
    osMutexWait(audioflinger_mutex_id, osWaitForever);
#else
    static void * POSSIBLY_UNUSED locked_lr;
    ASSERT(af_flag_lock == 0, "audioflinger has been locked by %p. LR=%p", (void *)locked_lr, (void *)lr);
    af_flag_lock = 1;
    locked_lr = lr;
#endif
}

#ifdef AUDIO_OUTPUT_DC_AUTO_CALIB
enum AF_CODEC_CALIB_CMD_T af_codec_calib_cmd = CODEC_CALIB_CMD_CLOSE;
#endif

void af_unlock_thread(void)
{
    void * POSSIBLY_UNUSED lr = __builtin_return_address(0);
#ifdef RTOS
    osMutexRelease(audioflinger_mutex_id);
#else
    static void * POSSIBLY_UNUSED unlocked_lr;
    ASSERT(af_flag_lock == 1, "audioflinger not locked before (lastly unlocked by %p). LR=%p", (void *)unlocked_lr, (void *)lr);
    af_flag_lock = 0;
    unlocked_lr = lr;
#endif
}

#if defined(RTOS) && defined(AF_STREAM_ID_0_PLAYBACK_FADEOUT)
int af_stream_fadeout_start(uint32_t sample)
{
    TRACE(2,"%s, fadeout sample: %d", __func__, sample);
    af_lock_thread();
    af_stream_fade_out.stop_process_cnt = 0;
    af_stream_fade_out.need_fadeout_samples = sample;
    af_stream_fade_out.need_fadeout_samples_processed = sample;
    af_stream_fade_out.step = 1.f / (sample - 1);
    af_stream_fade_out.weight = 1.f;
    af_unlock_thread();
    return 0;
}

int af_stream_fadeout_stop(void)
{
    TRACE(2,"%s, stop_process_cnt: %d", __func__, af_stream_fade_out.stop_process_cnt);
    af_lock_thread();
    af_stream_fade_out.stop_process_cnt = 0;
    af_stream_fade_out.stop_on_process = false;
    af_unlock_thread();
    return 0;
}

static uint32_t af_stream_fadeout(uint8_t *buf, uint32_t len, enum AUD_CHANNEL_NUM_T num, enum AUD_BITS_T bits)
{
    uint32_t sample_len = 0;
    uint32_t i = 0;
    uint32_t end;

    if (af_stream_fade_out.need_fadeout_samples_processed == 0) {
        TRACE(4 ,"%s end, fade ch:%d bit:%d weight:%d(/10000)",
            __func__, num, bits, (int32_t)(af_stream_fade_out.weight * 10000));
        memset(buf, 0, len);
        af_stream_fade_out.stop_process_cnt += 1;
        return 0;
    }

    if (bits == AUD_BITS_16){
        sample_len = len/num/2;
    }else if (bits >= AUD_BITS_16){
        sample_len = len/num/4;
    }

    end = MIN(af_stream_fade_out.need_fadeout_samples_processed, sample_len);

    TRACE(2 ,"fade ch:%d bit:%d samples:%d/%d weight:%d(/10000)",
            num, bits, end, sample_len, (int32_t)(af_stream_fade_out.weight * 10000));

    if (bits == AUD_BITS_16) {
        int16_t *sample_p = (int16_t *)buf;
        if (num == AUD_CHANNEL_NUM_1){
            for (i = 0; i < end * num; i += 1){
                sample_p[i + 0] = (int16_t)(sample_p[i + 0]*af_stream_fade_out.weight);
                af_stream_fade_out.weight -= af_stream_fade_out.step;
            }
        }else if (num == AUD_CHANNEL_NUM_2){
            for (i = 0; i < end * num; i += 2){
                sample_p[i + 0] = (int16_t)(sample_p[i + 0]*af_stream_fade_out.weight);
                sample_p[i + 1] = (int16_t)(sample_p[i + 1]*af_stream_fade_out.weight);
                af_stream_fade_out.weight -= af_stream_fade_out.step;
            }
        }else if (num == AUD_CHANNEL_NUM_4){
            for (i = 0; i < end * num; i += 4){
                sample_p[i + 0] = (int16_t)(sample_p[i + 0]*af_stream_fade_out.weight);
                sample_p[i + 1] = (int16_t)(sample_p[i + 1]*af_stream_fade_out.weight);
                sample_p[i + 2] = (int16_t)(sample_p[i + 2]*af_stream_fade_out.weight);
                sample_p[i + 3] = (int16_t)(sample_p[i + 3]*af_stream_fade_out.weight);
                af_stream_fade_out.weight -= af_stream_fade_out.step;
            }
        }else if (num == AUD_CHANNEL_NUM_8){
            for (i = 0; i < end * num; i += 8){
                sample_p[i + 0] = (int16_t)(sample_p[i + 0]*af_stream_fade_out.weight);
                sample_p[i + 1] = (int16_t)(sample_p[i + 1]*af_stream_fade_out.weight);
                sample_p[i + 2] = (int16_t)(sample_p[i + 2]*af_stream_fade_out.weight);
                sample_p[i + 3] = (int16_t)(sample_p[i + 3]*af_stream_fade_out.weight);
                sample_p[i + 4] = (int16_t)(sample_p[i + 4]*af_stream_fade_out.weight);
                sample_p[i + 5] = (int16_t)(sample_p[i + 5]*af_stream_fade_out.weight);
                sample_p[i + 6] = (int16_t)(sample_p[i + 6]*af_stream_fade_out.weight);
                sample_p[i + 7] = (int16_t)(sample_p[i + 7]*af_stream_fade_out.weight);
                af_stream_fade_out.weight -= af_stream_fade_out.step;
            }
        }

        for (; i < sample_len * num; i++) {
            sample_p[i] = 0;
        }
    }else if (bits >= AUD_BITS_16){
        int32_t *sample_p = (int32_t *)buf;
        if (num == AUD_CHANNEL_NUM_1){
            for (i = 0; i < end * num; i += 1){
                sample_p[i + 0] = (int32_t)(sample_p[i + 0]*af_stream_fade_out.weight);
                af_stream_fade_out.weight -= af_stream_fade_out.step;
            }
        }else if (num == AUD_CHANNEL_NUM_2){
            for (i = 0; i < end * num; i += 2){
                sample_p[i + 0] = (int32_t)(sample_p[i + 0]*af_stream_fade_out.weight);
                sample_p[i + 1] = (int32_t)(sample_p[i + 1]*af_stream_fade_out.weight);
                af_stream_fade_out.weight -= af_stream_fade_out.step;
            }
        }else if (num == AUD_CHANNEL_NUM_4){
            for (i = 0; i < end * num; i += 4){
                sample_p[i + 0] = (int32_t)(sample_p[i + 0]*af_stream_fade_out.weight);
                sample_p[i + 1] = (int32_t)(sample_p[i + 1]*af_stream_fade_out.weight);
                sample_p[i + 2] = (int32_t)(sample_p[i + 2]*af_stream_fade_out.weight);
                sample_p[i + 3] = (int32_t)(sample_p[i + 3]*af_stream_fade_out.weight);
                af_stream_fade_out.weight -= af_stream_fade_out.step;
            }
        }else if (num == AUD_CHANNEL_NUM_8){
            for (i = 0; i < end * num; i += 8){
                sample_p[i + 0] = (int32_t)(sample_p[i + 0]*af_stream_fade_out.weight);
                sample_p[i + 1] = (int32_t)(sample_p[i + 1]*af_stream_fade_out.weight);
                sample_p[i + 2] = (int32_t)(sample_p[i + 2]*af_stream_fade_out.weight);
                sample_p[i + 3] = (int32_t)(sample_p[i + 3]*af_stream_fade_out.weight);
                sample_p[i + 4] = (int32_t)(sample_p[i + 4]*af_stream_fade_out.weight);
                sample_p[i + 5] = (int32_t)(sample_p[i + 5]*af_stream_fade_out.weight);
                sample_p[i + 6] = (int32_t)(sample_p[i + 6]*af_stream_fade_out.weight);
                sample_p[i + 7] = (int32_t)(sample_p[i + 7]*af_stream_fade_out.weight);
                af_stream_fade_out.weight -= af_stream_fade_out.step;
            }
        }

        for (; i < sample_len * num; i++) {
            sample_p[i] = 0;
        }
    }

    af_stream_fade_out.need_fadeout_samples_processed -= end;

    return len;
}

void af_stream_fadeout_wait_finish()
{
    af_stream_fade_out.stop_on_process = true;
    af_stream_fade_out.stop_request_tid = osThreadGetId();
    osSignalClear(af_stream_fade_out.stop_request_tid, (1 << AF_FADE_OUT_SIGNAL_ID));
    osSignalWait((1 << AF_FADE_OUT_SIGNAL_ID), AF_STREAM_ID_0_PLAYBACK_FADEOUT_TIMEOUT);
}

void af_stream_fadeout_process(struct af_stream_cfg_t *af_cfg, uint8_t *buf, uint32_t len)
{
    if (af_stream_fade_out.stop_on_process){
        TRACE(5,"fadeout num:%d size:%d len:%d cnt:%d",
				af_cfg->cfg.channel_num, af_cfg->cfg.data_size, len,  af_stream_fade_out.stop_process_cnt);
        uint32_t ret = af_stream_fadeout(buf, len, af_cfg->cfg.channel_num, af_cfg->cfg.bits);
        TRACE(3,"process ret:%d %d %d",
				*(int16_t *)(buf+len-2-2-2), *(int16_t *)(buf+len-2-2), *(int16_t *)(buf+len-2));
        // Wait an extra handler to stop fadeout
        if (ret == 0 && af_stream_fade_out.stop_process_cnt == 2) {
            TRACE(0,"fadeout_process end");
            osSignalSet(af_stream_fade_out.stop_request_tid, (1 << AF_FADE_OUT_SIGNAL_ID));
        }
    }
}

uint32_t af_stream_playback_fadeout(enum AUD_STREAM_ID_T id, uint32_t ms)
{
    ASSERT(ms <= AF_STREAM_ID_0_PLAYBACK_FADEOUT_TIMEOUT, "[%s] ms(%d) should be less than %d",
        __FUNCTION__, ms, AF_STREAM_ID_0_PLAYBACK_FADEOUT_TIMEOUT);

    struct AF_STREAM_CONFIG_T *stream_cfg = NULL;
    enum AF_RESULT_T ret;
    uint32_t fadt_out_samples = 0;
    ret = af_stream_get_cfg(id, AUD_STREAM_PLAYBACK, &stream_cfg, false);
    if (ret == AF_RES_FAILD ){
        ret = AF_RES_FAILD;
        goto exit;
    }

    fadt_out_samples = (uint32_t)(ms * stream_cfg->sample_rate / 1000);
    TRACE(3,"[%s] ms:%d sample:%d bit:%d ch:%d", __func__, ms, (uint32_t)fadt_out_samples,
                                            stream_cfg->bits, stream_cfg->channel_num);

    af_stream_fadeout_start(fadt_out_samples);
    af_stream_fadeout_wait_finish();

exit:
    return ret;
}
#endif // RTOS && AF_STREAM_ID_0_PLAYBACK_FADEOUT

static inline int af_get_role_id_stream(const struct af_stream_cfg_t *role,
                            enum AUD_STREAM_ID_T *rid, enum AUD_STREAM_T *rstream)
{
    enum AUD_STREAM_ID_T id;
    enum AUD_STREAM_T stream;

    for (id = 0; id < AUD_STREAM_ID_NUM; id++) {
        for (stream = 0; stream < AUD_STREAM_NUM; stream++) {
            if (role == &af_stream[id][stream]) {
                *rid = id;
                *rstream = stream;
                return 0;
            }
        }
    }

    return -1;
}

#ifdef AUDIO_OUTPUT_I2S_SW_GAIN
POSSIBLY_UNUSED
static void sw_gain_i2s_set_volume(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, uint32_t vol)
{
    af_stream_sw_gain_set_volume_level(id, stream, vol);
}
#endif

//used by dma irq and af_thread
static inline struct af_stream_cfg_t *af_get_stream_role(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    ASSERT(id < AUD_STREAM_ID_NUM, "[%s] Bad id=%d", __func__, id);
    ASSERT(stream < AUD_STREAM_NUM, "[%s] Bad stream=%d", __func__, stream);

    return &af_stream[id][stream];
}

static void af_set_status(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, enum AF_STATUS_T status)
{
    struct af_stream_cfg_t *role = NULL;

    role = af_get_stream_role(id, stream);

    role->ctl.status |= status;
}

static void af_clear_status(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, enum AF_STATUS_T status)
{
    struct af_stream_cfg_t *role = NULL;

    role = af_get_stream_role(id, stream);

    role->ctl.status &= ~status;
}

//get current stream config parameters
uint32_t af_stream_get_cfg(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream,
                                    struct AF_STREAM_CONFIG_T **cfg, bool needlock)
{
    AF_TRACE_DEBUG();

    struct af_stream_cfg_t *role;
    enum AF_RESULT_T ret;

    if (needlock) {
        af_lock_thread();
    }
    role = af_get_stream_role(id, stream);
    //check stream is open
    if (role->ctl.status & AF_STATUS_STREAM_OPEN_CLOSE) {
        *cfg = &(role->cfg);
        ret = AF_RES_SUCCESS;
    } else {
        ret = AF_RES_FAILD;
    }
    if (needlock) {
        af_unlock_thread();
    }
    return ret;
}

#if 0
static void af_dump_cfg()
{
    struct af_stream_cfg_t *role = NULL;

    TRACE(0,"dump cfg.........start");
    //initial parameter
    for(uint8_t id=0; id< AUD_STREAM_ID_NUM; id++)
    {
        for(uint8_t stream=0; stream < AUD_STREAM_NUM; stream++)
        {
            role = af_get_stream_role((enum AUD_STREAM_ID_T)id, (enum AUD_STREAM_T)stream);

            TRACE(2,"id = %d, stream = %d:", id, stream);
            TRACE(1,"ctl.use_device = %d", role->ctl.use_device);
            TRACE(1,"cfg.device = %d", role->cfg.device);
            TRACE(1,"dma_cfg.ch = %d", role->dma_cfg.ch);
        }
    }
    TRACE(0,"dump cfg.........end");
}
#endif

#ifdef AUDIO_OUTPUT_SW_GAIN
static void af_codec_dac1_output_gain_changed(float coef)
{
    dac1_saved_output_coef = coef;
    TRACE(1, "output_gain_change da1:%08d, final:%08d", (int32_t)(coef * 10000000), (int32_t)(dac1_saved_output_coef * 10000000));
}

void af_codec_dac1_set_algo_gain(float coef)
{
    TRACE(1, "algo_gain da1:%08d", (int32_t)(coef * 10000000));
    dac1_algo_gain = coef;
    fade_update_gain(&dac1_fade, dac1_algo_gain);
}

void af_codec_dac1_set_custom_eq_peak(float peak)
{
    TRACE(1, "eq_master_gain:%08d", (int32_t)(peak * 10000000));
    custom_eq_peak = peak;
}
#endif /* AUDIO_OUTPUT_SW_GAIN */

#ifdef AUDIO_OUTPUT_DAC2_SW_GAIN
static void af_codec_dac2_output_gain_changed(float coef)
{
    TRACE(1, "output_gain_change da2:%08d", (int32_t)(coef * 10000000));
    dac2_saved_output_coef = coef;
}
#endif /* AUDIO_OUTPUT_DAC2_SW_GAIN */

#ifdef AUDIO_OUTPUT_DAC3_SW_GAIN
static void af_codec_dac3_output_gain_changed(float coef)
{
	TRACE(1, "output_gain_change da3:%08d", (int32_t)(coef * 10000000));
    dac3_saved_output_coef = coef;
}
#endif /* AUDIO_OUTPUT_DAC3_SW_GAIN */

#if defined(AUDIO_OUTPUT_SW_GAIN) || defined(AUDIO_OUTPUT_DAC2_SW_GAIN)
/*static inline float af_codec_sw_gain_filter(SW_GAIN_IIR_T *iir, float gain_in)
{
    float gain_out = gain_in*iir->coefs_b[0]
                            +iir->history_x[0]*iir->coefs_b[1]
                            +iir->history_x[1]*iir->coefs_b[2]
                            -iir->history_y[0]*iir->coefs_a[1]
                            -iir->history_y[1]*iir->coefs_a[2];

    iir->history_y[1]=iir->history_y[0];
    iir->history_y[0]=gain_out;
    iir->history_x[1]=iir->history_x[0];
    iir->history_x[0]=gain_in;

    return gain_out;
}*/
static void af_codec_sw_gain_process(uint8_t *buf, uint32_t size, enum AUD_BITS_T bits,
                            enum AUD_CHANNEL_NUM_T chans, SW_GAIN_IIR_T *iir, float coef)
{
    uint32_t i,pcm_len;
    int32_t pcm_out_l;
    int32_t pcm_out_r;

    float output_gain=1.0f;

    //TRACE(1,"af_codec_sw_gain_process:%d",(int32_t)(saved_output_coef*1000));

    float coefs_b0,coefs_b1,coefs_b2;
    float coefs_a1,coefs_a2;
    float history_x0,history_x1;
    float history_y0,history_y1;

    coefs_b0=iir->coefs_b[0];
    coefs_b1=iir->coefs_b[1];
    coefs_b2=iir->coefs_b[2];

    coefs_a1=iir->coefs_a[1];
    coefs_a2=iir->coefs_a[2];

    history_x0=iir->history_x[0];
    history_x1=iir->history_x[1];

    history_y0=iir->history_y[0];
    history_y1=iir->history_y[1];

   //TRACE(1,"chans:%d,bits:%d,size:%d",chans,bits,size);

   // uint32_t lock;

   // lock = int_lock();

   // uint32_t start_ticks = hal_fast_sys_timer_get();

    if(chans==AUD_CHANNEL_NUM_1)
    {
        if (bits <= AUD_BITS_16)
        {
            pcm_len = size / sizeof(int16_t);
            int16_t *pcm_buf = (int16_t *)buf;
            for (i = 0; i < pcm_len;) {
                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(&dac1_fade, dac1_algo_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 16);
                i++;

#ifdef SW_GAIN_OPTI_MIPS_FOR_ALIGN_4
                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(&dac1_fade, dac1_algo_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 16);
                i++;

                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(&dac1_fade, dac1_algo_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 16);
                i++;

                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(&dac1_fade, dac1_algo_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 16);
                i++;
#endif
            }
        }
        else
        {
            pcm_len = size / sizeof(int32_t);
            int32_t *pcm_buf = (int32_t *)buf;
            for (i = 0; i < pcm_len;) {
                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(&dac1_fade, dac1_algo_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 24);
                i++;
#ifdef SW_GAIN_OPTI_MIPS_FOR_ALIGN_4
                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(&dac1_fade, dac1_algo_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 24);
                i++;

                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(&dac1_fade, dac1_algo_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 24);
                i++;

                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(&dac1_fade, dac1_algo_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 24);
                i++;
#endif
            }
        }
    }
    else if(chans == AUD_CHANNEL_NUM_2)
    {
        if (bits <= AUD_BITS_16)
        {
            pcm_len = size / sizeof(int16_t);
            int16_t *pcm_buf = (int16_t *)buf;
            for (i = 0; i < pcm_len;)
            {
                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(&dac1_fade, dac1_algo_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_out_r = (int32_t)(pcm_buf[i+1] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 16);
                pcm_buf[i+1] = __SSAT(pcm_out_r, 16);
                i=i+2;

#ifdef SW_GAIN_OPTI_MIPS_FOR_ALIGN_4
                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(&dac1_fade, dac1_algo_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_out_r = (int32_t)(pcm_buf[i+1] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 16);
                pcm_buf[i+1] = __SSAT(pcm_out_r, 16);
                i=i+2;

                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(&dac1_fade, dac1_algo_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_out_r = (int32_t)(pcm_buf[i+1] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 16);
                pcm_buf[i+1] = __SSAT(pcm_out_r, 16);
                i=i+2;

                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(&dac1_fade, dac1_algo_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_out_r = (int32_t)(pcm_buf[i+1] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 16);
                pcm_buf[i+1] = __SSAT(pcm_out_r, 16);
                i=i+2;
#endif
            }
        }
        else
        {
            pcm_len = size / sizeof(int32_t);
            int32_t *pcm_buf = (int32_t *)buf;
            for (i = 0; i < pcm_len;) {
                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(&dac1_fade, dac1_algo_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_out_r = (int32_t)(pcm_buf[i+1] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 24);
                pcm_buf[i+1] = __SSAT(pcm_out_r, 24);
                i=i+2;
#ifdef SW_GAIN_OPTI_MIPS_FOR_ALIGN_4
                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(&dac1_fade, dac1_algo_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_out_r = (int32_t)(pcm_buf[i+1] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 24);
                pcm_buf[i+1] = __SSAT(pcm_out_r, 24);
                i=i+2;

                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(&dac1_fade, dac1_algo_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_out_r = (int32_t)(pcm_buf[i+1] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 24);
                pcm_buf[i+1] = __SSAT(pcm_out_r, 24);
                i=i+2;

                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(&dac1_fade, dac1_algo_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_out_r = (int32_t)(pcm_buf[i+1] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 24);
                pcm_buf[i+1] = __SSAT(pcm_out_r, 24);
                i=i+2;
#endif
            }
        }
    }

    iir->history_y[1] = history_y1;
    iir->history_y[0] = history_y0;
    iir->history_x[1] = history_x1;
    iir->history_x[0] = history_x0;

    //uint32_t end_ticks = hal_fast_sys_timer_get();

    //int_unlock(lock);
}
#endif /* #if defined(AUDIO_OUTPUT_SW_GAIN) || defined(AUDIO_OUTPUT_DAC2_SW_GAIN) */

#if defined(AUDIO_OUTPUT_SW_GAIN) && defined(AUDIO_OUTPUT_SW_GAIN_BEFORE_DRC)
void af_codec_dac1_sw_gain_process(uint8_t *buf, uint32_t len, enum AUD_BITS_T bits, enum AUD_CHANNEL_NUM_T chans)
{
    af_codec_sw_gain_process(buf,len,bits,chans,&sw_gain_iir,dac1_saved_output_coef);
}

void af_codec_dac1_sw_gain_enable(bool enable)
{
    dac1_sw_gain_enabled = enable;
}
#endif

static uint32_t POSSIBLY_UNUSED af_codec_get_sample_count(uint32_t size, enum AUD_BITS_T bits, enum AUD_CHANNEL_NUM_T chans)
{
    uint32_t div;

    if (bits <= AUD_BITS_16) {
        div = 2;
    } else {
        div = 4;
    }
    div *= chans;
    return size / div;
}

static void POSSIBLY_UNUSED af_zero_mem(void *dst, unsigned int size)
{
    uint32_t *d = dst;
    uint32_t count = size / 4;

    while (count--) {
        *d++ = 0;
    }
}

static bool af_codec_playback_pre_handler(uint8_t *buf, uint32_t len, const struct af_stream_cfg_t *role)
{
    uint32_t POSSIBLY_UNUSED time;

    if ((role->cfg.device == AUD_STREAM_USE_INT_CODEC2)
        || (role->cfg.device == AUD_STREAM_USE_INT_CODEC3)) {
        return false;
    }
    if (0) {

#ifdef AUDIO_OUTPUT_PA_ON_FADE_IN
    } else if (dac_pa_state == AF_DAC_PA_ON_TRIGGER &&
            (time = hal_sys_timer_get()) - dac_dc_start_time >= AF_CODEC_DC_STABLE_INTERVAL &&
            time - dac_pa_stop_time >= AF_CODEC_PA_RESTART_INTERVAL) {
        analog_aud_codec_speaker_enable(true);
        dac_pa_state = AF_DAC_PA_NULL;
#endif // AUDIO_OUTPUT_PA_ON_FADE_IN

#ifdef AUDIO_OUTPUT_PA_OFF_FADE_OUT
    } else if (dac_pa_state == AF_DAC_PA_OFF_TRIGGER) {
        dac_dc_start_time = hal_sys_timer_get();
        dac_pa_state = AF_DAC_PA_OFF_DC_START;
    } else if (dac_pa_state == AF_DAC_PA_OFF_DC_START) {
        time = hal_sys_timer_get();
        if (time - dac_dc_start_time >= AF_CODEC_DC_STABLE_INTERVAL) {
            dac_pa_state = AF_DAC_PA_NULL;
            analog_aud_codec_speaker_enable(false);
            dac_pa_stop_time = time;
#ifdef RTOS
            osSignalSet(fade_thread_id, (1 << AF_FADE_OUT_SIGNAL_ID));
#endif
        }
#endif // AUDIO_OUTPUT_PA_OFF_FADE_OUT

    }

#if defined(AUDIO_OUTPUT_PA_ON_FADE_IN) || defined(AUDIO_OUTPUT_PA_OFF_FADE_OUT)
    if (dac_pa_state == AF_DAC_PA_ON_TRIGGER || dac_pa_state == AF_DAC_PA_OFF_DC_START) {
        af_zero_mem(buf, len);
        return true;
    }
#endif

    return false;
}

#ifdef AUDIO_OUTPUT_DC_CALIB_SW
static void af_codec_playback_sw_dc_calib(uint8_t *buf, uint32_t len, enum AUD_BITS_T bits, enum AUD_CHANNEL_NUM_T chans)
{
    uint32_t cnt;

    if (bits <= AUD_BITS_16) {
        int16_t *ptr16 = (int16_t *)buf;
#ifdef AUDIO_OUTPUT_DC_CALIB_SW
        int16_t dc_l = dac_dc[0];
        int16_t dc_r = dac_dc[1];
#endif
        if (chans == AUD_CHANNEL_NUM_1) {
            cnt = len / sizeof(int16_t);
            while (cnt-- > 0) {
#ifdef AUDIO_OUTPUT_DC_CALIB_SW
                *ptr16 = __SSAT(*ptr16 + dc_l, 16);
#endif
                ptr16++;
            }
        } else {
            cnt = len / sizeof(int16_t) / 2;
            while (cnt-- > 0) {
#ifdef AUDIO_OUTPUT_DC_CALIB_SW
                *ptr16 = __SSAT(*ptr16 + dc_l, 16);
                *(ptr16 + 1) = __SSAT(*(ptr16 + 1) + dc_r, 16);
#endif
                ptr16 += 2;
            }
        }
    } else {
        int32_t *ptr32 = (int32_t *)buf;
        int32_t dac_bits =
#ifdef CHIP_BEST1000
            CODEC_PLAYBACK_BIT_DEPTH;
#else
            24;
#endif
        int32_t val_shift;

        if (dac_bits < 24) {
            val_shift = 24 - dac_bits;
        } else {
            val_shift = 0;
        }

#ifdef AUDIO_OUTPUT_DC_CALIB_SW
        int32_t dc_l = dac_dc[0] << 8;
        int32_t dc_r = dac_dc[1] << 8;
#endif
        if (chans == AUD_CHANNEL_NUM_1) {
            cnt = len / sizeof(int32_t);
            while (cnt-- > 0) {
#ifdef CORRECT_SAMPLE_VALUE
                *ptr32 = ((*ptr32) << (32 - dac_bits)) >> (32 - dac_bits);
#endif
#ifdef AUDIO_OUTPUT_DC_CALIB_SW
                *ptr32 = __SSAT((*ptr32 + dc_l) >> val_shift, dac_bits);
#elif defined(CORRECT_SAMPLE_VALUE)
                *ptr32 = __SSAT(*ptr32 >> val_shift, dac_bits);
#else
                *ptr32 = *ptr32 >> val_shift;
#endif
                ptr32++;
            }
        } else {
            cnt = len / sizeof(int32_t) / 2;
            while (cnt-- > 0) {
#ifdef CORRECT_SAMPLE_VALUE
                *ptr32 = ((*ptr32) << (32 - dac_bits)) >> (32 - dac_bits);
                *(ptr32 + 1) = ((*(ptr32 + 1)) << (32 - dac_bits)) >> (32 - dac_bits);
#endif
#ifdef AUDIO_OUTPUT_DC_CALIB_SW
                *ptr32 = __SSAT((*ptr32 + dc_l) >> val_shift, dac_bits);
                *(ptr32 + 1) = __SSAT((*(ptr32 + 1) + dc_r) >> val_shift, dac_bits);
#elif defined(CORRECT_SAMPLE_VALUE)
                *ptr32 = __SSAT(*ptr32 >> val_shift, dac_bits);
                *(ptr32 + 1) = __SSAT(*(ptr32 + 1) >> val_shift, dac_bits);
#else
                *ptr32 = *ptr32 >> val_shift;
                *(ptr32 + 1) = *(ptr32 + 1) >> val_shift;
#endif
                ptr32 += 2;
            }
        }
    }
}
#endif // AUDIO_OUTPUT_DC_CALIB_SW

static void af_codec_playback_post_handler(uint8_t *buf, uint32_t len, const struct af_stream_cfg_t *role)
{
    POSSIBLY_UNUSED bool sw_gain_process_flag = false;
    POSSIBLY_UNUSED enum AUD_BITS_T bits;
    POSSIBLY_UNUSED enum AUD_CHANNEL_NUM_T chans;
    POSSIBLY_UNUSED uint32_t cnt;
    POSSIBLY_UNUSED enum AF_CODEC_PLAY_HDLR_ID_T id = AF_CODEC_PLAY_HDLR_ID_0;
#if defined(AUDIO_OUTPUT_SW_GAIN) || defined(AUDIO_OUTPUT_DAC2_SW_GAIN)
    POSSIBLY_UNUSED float saved_output_coef;
    POSSIBLY_UNUSED SW_GAIN_IIR_T *gain_iir = NULL;
#if defined(AUDIO_OUTPUT_SW_LIMITER) || defined(AUDIO_OUTPUT_DAC2_SW_LIMITER) \
    || defined(AUDIO_OUTPUT_DAC3_SW_LIMITER)
    POSSIBLY_UNUSED FloatLimiterPtr pFLimiter = NULL;
#endif
#endif

#ifdef AUDIO_OUTPUT_DC_AUTO_CALIB
    if (af_codec_calib_cmd != CODEC_CALIB_CMD_CLOSE) {
        return;
    }
#endif
    bits = role->cfg.bits;
    chans = role->cfg.channel_num;

    if (0) {
#ifdef AUDIO_OUTPUT_DAC2
    } else if (role->cfg.device == AUD_STREAM_USE_INT_CODEC2) {
        id = AF_CODEC_PLAY_HDLR_ID_1;
#endif
#ifdef AUDIO_OUTPUT_DAC3
    } else if (role->cfg.device == AUD_STREAM_USE_INT_CODEC3) {
        id = AF_CODEC_PLAY_HDLR_ID_2;
#endif
    } else {
        id = AF_CODEC_PLAY_HDLR_ID_0;
    }

    if (0) {
#if defined(AUDIO_OUTPUT_SW_GAIN)
    } else if (role->cfg.device == AUD_STREAM_USE_INT_CODEC && dac1_sw_gain_enabled) {
        saved_output_coef = MIN(dac1_saved_output_coef, 1.f / custom_eq_peak);
#if defined(AUDIO_OUTPUT_SW_LIMITER)
        pFLimiter = FloatLimiterP;
#else
        gain_iir = &sw_gain_iir;
#endif
#endif /* AUDIO_OUTPUT_SW_GAIN */
#ifdef AUDIO_OUTPUT_DAC2_SW_GAIN
    } else if (role->cfg.device == AUD_STREAM_USE_INT_CODEC2) {
        saved_output_coef = dac2_saved_output_coef;
#if defined(AUDIO_OUTPUT_DAC2_SW_LIMITER)
        pFLimiter = Dac2FloatLimiterP;
#else
        gain_iir = &dac2_sw_gain_iir;
#endif
#endif /* AUDIO_OUTPUT_DAC2_SW_GAIN */
#ifdef AUDIO_OUTPUT_DAC3_SW_GAIN
    } else if (role->cfg.device == AUD_STREAM_USE_INT_CODEC3) {
        saved_output_coef = dac3_saved_output_coef;
#if defined(AUDIO_OUTPUT_DAC3_SW_LIMITER)
        pFLimiter = Dac3FloatLimiterP;
#else
        gain_iir = &dac3_sw_gain_iir;
#endif
#endif /* AUDIO_OUTPUT_DAC3_SW_GAIN */
#ifdef AUDIO_OUTPUT_I2S_SW_GAIN
    } else if (role->cfg.device == AUD_STREAM_USE_I2S0_MASTER ||
                role->cfg.device == AUD_STREAM_USE_I2S1_MASTER) {
        sw_gain_process_flag = true;
#endif /* AUDIO_OUTPUT_I2S_SW_GAIN */
    }
#if defined(AUDIO_OUTPUT_I2S_SW_GAIN)
    if (sw_gain_process_flag && af_stream_sw_gain_get_enable(id, AUD_STREAM_PLAYBACK)){
        af_stream_sw_gain_process(id, AUD_STREAM_PLAYBACK, buf, len);
    }
#endif

#if defined(AUDIO_OUTPUT_SW_GAIN) || defined(AUDIO_OUTPUT_DAC2_SW_GAIN) \
    || defined(AUDIO_OUTPUT_DAC3_SW_GAIN)
    if (gain_iir != NULL) {
        af_codec_sw_gain_process(buf,len,bits,chans,gain_iir,saved_output_coef);
#if defined(AUDIO_OUTPUT_SW_LIMITER) || defined(AUDIO_OUTPUT_DAC2_SW_LIMITER) \
    || defined(AUDIO_OUTPUT_DAC3_SW_LIMITER)
    } else if (pFLimiter != NULL) {
        if (bits <= AUD_BITS_16) {
            cnt = len / sizeof(int16_t);
        } else {
            cnt = len / sizeof(int32_t);
        }

        ApplyFloatLimiter(pFLimiter, buf, saved_output_coef, cnt);
#endif
    }
#endif

    if (codec_play_post_hdlr[id]) {
#ifdef CODEC_VCM_CHECK
        extern uint8_t vcm_sta;
        if(!vcm_sta)
        {
            vcm_sta = 1;
            analog_vcm_status_check();
            //TRACE(0, "analog_vcm_status_check\n");
            //hal_sys_timer_delay(MS_TO_TICKS(3));
        }
#endif
        codec_play_post_hdlr[id](buf, len, (void *)&role->cfg);
    }

#if defined(AUDIO_OUTPUT_INVERT_RIGHT_CHANNEL)
    if (chans == AUD_CHANNEL_NUM_2) {
        if (bits == AUD_BITS_16) {
            int16_t *buf16 = (int16_t *)buf;
            for (int i = 1; i < len / sizeof(int16_t); i += 2) {
                int32_t tmp = -buf16[i];
                buf16[i] = __SSAT(tmp, 16);
            }
        } else {
            int32_t *buf32 = (int32_t *)buf;
            for (int i = 1; i < len / sizeof(int32_t); i += 2) {
                int32_t tmp = -buf32[i];
                buf32[i] = __SSAT(tmp, 24);
            }
        }
    }
#endif

#if (defined(CHIP_BEST1000) || defined(CHIP_BEST2000)) && defined(AUDIO_OUTPUT_CALIB_GAIN_MISSMATCH)
    // gain must less than 1.0
    float gain = 0.9441f;
    if (chans == AUD_CHANNEL_NUM_2) {
        if (bits == AUD_BITS_16) {
            int16_t *buf16 = (int16_t *)buf;
            for (int i = 1; i < len / sizeof(int16_t); i += 2) {
                int32_t tmp = (int)(buf16[i + 1] * gain);
                buf16[i + 1] = __SSAT(tmp, 16);
            }
        } else {
            int32_t *buf32 = (int32_t *)buf;
            for (int i = 1; i < len / sizeof(int32_t); i += 2) {
                int32_t tmp = (int)(buf32[i + 1] * gain);
                buf32[i + 1] = __SSAT(tmp, 24);
            }
        }
    }
#endif

#if defined(AUDIO_OUTPUT_DC_CALIB_SW)
    af_codec_playback_sw_dc_calib(buf, len, bits, chans);
#elif defined(CHIP_BEST1000)
    if (bits == AUD_BITS_24 || bits == AUD_BITS_32) {
        int32_t *ptr32 = (int32_t *)buf;
        int32_t val_shift;

        if (bits == AUD_BITS_24) {
            val_shift = 24 - CODEC_PLAYBACK_BIT_DEPTH;
        } else {
            val_shift = 32 - CODEC_PLAYBACK_BIT_DEPTH;
        }

        cnt = len / sizeof(int32_t);
        while (cnt-- > 0) {
            *ptr32 >>= val_shift;
            ptr32++;
        }
    }
#endif

#if defined(CHIP_BEST1000) && defined(AUDIO_OUTPUT_GAIN_M60DB_CHECK)
    hal_codec_dac_gain_m60db_check(HAL_CODEC_PERF_TEST_M60DB);
#endif
}

static void _config_handler_interval_check(struct af_stream_cfg_t *role)
{
    uint32_t rate;
    uint32_t samp_size;
    uint32_t us;
    uint32_t ticks;
    uint32_t tmp_samp_cnt;

    if (role->ctl.use_device == AUD_STREAM_USE_DPD_RX) {
        // Not support yet
        role->ctl.intvl_check_en = false;
        return;
    }

    role->ctl.intvl_check_en = true;

    rate = role->cfg.sample_rate;
    if (role->ctl.use_device == AUD_STREAM_USE_BT_PCM) {
        // BT-PCM is not a real PCM stream, but a CVSD or MSBC stream
        rate = AUD_SAMPRATE_8000;
    }

    if (role->cfg.bits == AUD_BITS_24 || role->cfg.bits == AUD_BITS_32) {
        samp_size = 4;
    } else {
        samp_size = 2;
    }

    tmp_samp_cnt = role->cfg.data_size / 2 / samp_size / role->cfg.channel_num;
    us = (uint32_t)((float)(tmp_samp_cnt) * 1000 / ((float)rate / 1000) + 0.5);
#ifdef TIMER1_BASE
    ticks = US_TO_FAST_TICKS(us);
#else
    ticks = US_TO_TICKS(us);
#endif

    role->ctl.hdlr_intvl_ticks = ticks;

#if 0
    enum AUD_STREAM_ID_T id = -1;
    enum AUD_STREAM_T stream = -1;

    af_get_role_id_stream(role, &id, &stream);

    TRACE(0, "[%s] id=%d stream=%d rate=%u samp_size=%d chans=%d size=%u us=%d",
        __func__, id, stream, rate, samp_size, role->cfg.channel_num, role->cfg.data_size, us);
#endif
}

static uint32_t _get_fast_time_stamp(void)
{
#ifdef TIMER1_BASE
    return hal_fast_sys_timer_get();
#else
    return hal_sys_timer_get();
#endif
}

static uint32_t _fast_time_to_us(uint32_t ticks)
{
#ifdef TIMER1_BASE
    return FAST_TICKS_TO_US(ticks);
#else
    return TICKS_TO_US(ticks);
#endif
}

#ifdef ADVANCE_FILL_ENABLED
void af_pre_fill_handler(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, uint8_t pre_fill_seq)
{
    af_lock_thread();
    uint8_t *buf=NULL;
    uint32_t len;
    struct af_stream_cfg_t *role=NULL;

    role = af_get_stream_role(id, stream);
    bool skip_handler = false;
    len = role->dma_buf_size / 2;

    buf = role->dma_buf_ptr + pre_fill_seq * len;
    skip_handler = af_codec_playback_pre_handler(buf, len, role);
    if (!skip_handler) {
        role->handler(buf, len);
    }
    af_codec_playback_post_handler(buf, len, role);
    af_unlock_thread();
}
#endif

static inline void af_thread_stream_handler(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    uint32_t lock;
    struct af_stream_cfg_t *role=NULL;
    uint32_t dma_addr, hw_pos;
    uint8_t *buf=NULL;
    uint32_t len;
    uint32_t pp_cnt;
    bool codec_playback=false;
    bool skip_handler=false;
    enum AF_PP_T pp_index;

    role = af_get_stream_role(id, stream);

    af_lock_thread();

    if (role->handler && (role->ctl.status & AF_STATUS_STREAM_START_STOP)) {
        lock = int_lock();
        pp_cnt = role->ctl.pp_cnt;
        role->ctl.pp_cnt = 0;
        int_unlock(lock);

        af_sig_lost_cnt[id][stream] = pp_cnt - 1;
        if (af_sig_lost_cnt[id][stream]) {
            TRACE(3,"af_thread:WARNING: id=%d stream=%d lost %u signals", id, stream, af_sig_lost_cnt[id][stream]);
        }

        pp_index = role->ctl.pp_index;

        // Get PP index from accurate DMA pos
        dma_addr = af_stream_get_cur_dma_addr(id, stream);
        hw_pos = dma_addr - (uint32_t)role->dma_buf_ptr;
        if (hw_pos > role->dma_buf_size) {
            TRACE(3,"af_thread: Failed to get valid dma addr for id=%d stream=%d: 0x%08x", id, stream, dma_addr);
        }
#ifndef CHIP_BEST1000
        if (role->cfg.chan_sep_buf && role->cfg.channel_num > AUD_CHANNEL_NUM_1) {
            uint32_t chan_size;

            chan_size = role->dma_buf_size / role->cfg.channel_num;

            if (hw_pos <= role->dma_buf_size) {
                hw_pos = hw_pos % chan_size;
                if (hw_pos < chan_size / 2) {
                    pp_index = PP_PANG;
                } else if (hw_pos < chan_size) {
                    pp_index = PP_PING;
                }
            }
            if (pp_index == PP_PING) {
                buf = role->dma_buf_ptr;
            } else {
                buf = role->dma_buf_ptr + chan_size / 2;
            }
        } else
#endif
        {
            if (hw_pos < role->dma_buf_size / 2) {
                pp_index = PP_PANG;
            } else if (hw_pos < role->dma_buf_size) {
                pp_index = PP_PING;
            }
            if (pp_index == PP_PING) {
                buf = role->dma_buf_ptr;
            } else {
                buf = role->dma_buf_ptr + role->dma_buf_size / 2;
            }
        }

        if (role->ctl.intvl_check_en) {
            bool intvl_err = false;
            uint32_t cur_fast_time = _get_fast_time_stamp();

            if (pp_index == role->ctl.pp_index) {
                if (cur_fast_time - role->ctl.prev_hdlr_time >= 2 * role->ctl.hdlr_intvl_ticks) {
                    intvl_err = true;
                }
            } else {
                intvl_err = true;
            }
            if (role->ctl.first_hdlr_proc) {
                role->ctl.first_hdlr_proc = false;
            } else {
                if (intvl_err) {
                    TRACE(3,"af_thread:WARNING: id=%d stream=%d hdlr intvl ERROR: %u us (expecting %u)",
                        id, stream, _fast_time_to_us(cur_fast_time - role->ctl.prev_hdlr_time), _fast_time_to_us(role->ctl.hdlr_intvl_ticks));
                }
            }
            role->ctl.prev_hdlr_time = cur_fast_time;
        }

        role->ctl.pp_index = PP_PINGPANG(pp_index);

        if (stream == AUD_STREAM_PLAYBACK
            && ((role->ctl.use_device == AUD_STREAM_USE_INT_CODEC)
                || (role->ctl.use_device == AUD_STREAM_USE_INT_CODEC2)
                || (role->ctl.use_device == AUD_STREAM_USE_INT_CODEC3)
                || (role->ctl.use_device == AUD_STREAM_USE_I2S0_MASTER)
                || (role->ctl.use_device == AUD_STREAM_USE_I2S1_MASTER))) {
            codec_playback = true;
        } else {
            codec_playback = false;
        }

        skip_handler = false;
        len = role->dma_buf_size / 2;

        if (codec_playback) {
            skip_handler = af_codec_playback_pre_handler(buf, len, role);
        }

        if (!skip_handler) {
            role->handler(buf, len);
        }

        if (codec_playback) {
            af_codec_playback_post_handler(buf, len, role);
        }

#if defined(RTOS) && defined(AF_STREAM_ID_0_PLAYBACK_FADEOUT)
        if (((id == AUD_STREAM_ID_0) || (id == AUD_STREAM_ID_3)) && stream == AUD_STREAM_PLAYBACK
            && (role->ctl.use_device == AUD_STREAM_USE_INT_CODEC || role->ctl.use_device == AUD_STREAM_USE_INT_CODEC2)) {
            af_stream_fadeout_process(role, buf, len);
        }
#endif

#ifdef AUDIO_RMS_MONITOR_ENABLE
        if (codec_playback) {
            if(rms_debug_cnt == 0)
            {
            #ifdef AUDIO_OUTPUT_SW_GAIN
                if (role->cfg.bits <= AUD_BITS_16) {
                    int16_t* rms_buff = (int16_t*)buf;
                    rms_debug_cnt = 0;
                    TRACE(1, "RMS AF:%d coef %d %d %d %d %d %d",(int32_t)(dac1_saved_output_coef * 10000000), rms_buff[0], rms_buff[1], rms_buff[2], rms_buff[3], rms_buff[4], rms_buff[5]);
                }
                else {
                    int32_t* rms_buff = (int32_t*)buf;
                    rms_debug_cnt = 0;
                    TRACE(1, "RMS AF:%d coef %d %d %d %d %d %d",(int32_t)(dac1_saved_output_coef * 10000000), rms_buff[0], rms_buff[1], rms_buff[2], rms_buff[3], rms_buff[4], rms_buff[5]);
                }
            #else
                if (role->cfg.bits <= AUD_BITS_16) {
                    int16_t* rms_buff = (int16_t*)buf;
                    rms_debug_cnt = 0;
                    TRACE(1, "RMS AF: %d %d %d %d %d %d", rms_buff[0], rms_buff[1], rms_buff[2], rms_buff[3], rms_buff[4], rms_buff[5]);
                }
                else {
                    int32_t* rms_buff = (int32_t*)buf;
                    rms_debug_cnt = 0;
                    TRACE(1, "RMS AF: %d %d %d %d %d %d", rms_buff[0], rms_buff[1], rms_buff[2], rms_buff[3], rms_buff[4], rms_buff[5]);
                }
            #endif
            }
        }
#endif

        if (role->ctl.pp_cnt) {
            TRACE(3,"af_thread:WARNING: id=%d stream=%d hdlr ran too long (pp_cnt=%u)", id, stream, role->ctl.pp_cnt);
        }
    }

    af_unlock_thread();
}

#ifdef RTOS
extern void hal_trace_fault_dump_cp_print();
static void af_thread(void const *argument)
{
    osEvent evt;
    uint32_t signals = 0;
    enum AUD_STREAM_ID_T id;
    enum AUD_STREAM_T stream;

    while(1)
    {
        //wait any signal
        hal_trace_fault_dump_cp_print();
        evt = osSignalWait(0x0, osWaitForever);
        signals = evt.value.signals;
//        TRACE(3,"[%s] status = %x, signals = %d", __func__, evt.status, evt.value.signals);

        if(evt.status == osEventSignal)
        {
            for(uint8_t i=0; i<AUD_STREAM_ID_NUM * AUD_STREAM_NUM; i++)
            {
                if(signals & (1 << i))
                {
                    id = (enum AUD_STREAM_ID_T)(i >> 1);
                    stream = (enum AUD_STREAM_T)(i & 1);
                #ifdef CODEC_ERROR_HANDLING
                    af_ping_supervisor(id, stream);
                #endif
                    af_thread_stream_handler(id, stream);

                }
            }
        }
        else
        {
            TRACE(2,"[%s] ERROR: evt.status = %d", __func__, evt.status);
            continue;
        }
    }
}

#else // !RTOS

#include "cmsis.h"
static volatile uint32_t af_flag_open;
static volatile uint32_t af_flag_signal;

static void af_set_flag(volatile uint32_t *flag, uint32_t set)
{
    uint32_t lock;

    lock = int_lock();
    *flag |= set;
    int_unlock(lock);
}

static void af_clear_flag(volatile uint32_t *flag, uint32_t clear)
{
    uint32_t lock;

    lock = int_lock();
    *flag &= ~clear;
    int_unlock(lock);
}

static bool af_flag_active(volatile uint32_t *flag, uint32_t bits)
{
    return !!(*flag & bits);
}

void af_thread(void const *argument)
{
    uint32_t lock;
    uint32_t i;
    enum AUD_STREAM_ID_T id;
    enum AUD_STREAM_T stream;

    if (af_flag_open == 0) {
        return;
    }

    for (i = 0; i < AUD_STREAM_ID_NUM * AUD_STREAM_NUM; i++) {
        if (!af_flag_active(&af_flag_signal, 1 << i)) {
            continue;
        }
        af_clear_flag(&af_flag_signal, 1 << i);

        id = (enum AUD_STREAM_ID_T)(i >> 1);
        stream = (enum AUD_STREAM_T)(i & 1);

#ifdef CODEC_ERROR_HANDLING
        af_ping_supervisor(id, stream);
#endif

        af_thread_stream_handler(id, stream);
    }

    lock = int_lock();
    if (af_flag_signal == 0) {
        hal_cpu_wake_unlock(AF_CPU_WAKE_USER);
    }
    int_unlock(lock);
}

#endif // !RTOS

static void af_dma_irq_handler(uint8_t ch, uint32_t remain_dst_tsize, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    struct af_stream_cfg_t *role = NULL;

    //initial parameter
    for (uint8_t id = 0; id < AUD_STREAM_ID_NUM; id++)
    {
        for (uint8_t stream = 0; stream < AUD_STREAM_NUM; stream++)
        {
            role = af_get_stream_role((enum AUD_STREAM_ID_T)id, (enum AUD_STREAM_T)stream);

            if (role->dma_cfg.ch == ch)
            {
                role->ctl.pp_cnt++;
                //TRACE(4,"[%s] id = %d, stream = %d, ch = %d", __func__, id, stream, ch);
                //TRACE(2,"[%s] PLAYBACK pp_cnt = %d", __func__, role->ctl.pp_cnt);
                if (irq_notif) {
                    irq_notif(id, stream);
                }
#ifdef RTOS
                osSignalSet(af_thread_tid, 0x01 << (id * 2 + stream));
#else
                af_set_flag(&af_flag_signal, 0x01 << (id * 2 + stream));
                hal_cpu_wake_lock(AF_CPU_WAKE_USER);
#endif
                return;
            }
        }
    }

    //invalid dma irq
    ASSERT(0, "[%s] ERROR: channel id = %d", __func__, ch);
}

#ifdef __CODEC_ASYNC_CLOSE__
static void af_codec_async_close(void)
{
    af_lock_thread();
    codec_int_close(CODEC_CLOSE_ASYNC_REAL);
    af_unlock_thread();
}
#endif

void *af_thread_tid_get(void)
{
#ifdef RTOS
    return (void *)af_thread_tid;
#else
    return NULL;
#endif
}

POSSIBLY_UNUSED static void af_set_dac_dc_offset(void)
{
#ifdef AUDIO_OUTPUT_DC_AUTO_CALIB
    int32_t dc_l, dc_r;
    uint32_t max_dc, max_scale = AF_CODEC_DC_MAX_SCALE_2;
#else
    int16_t dc_l, dc_r;
    uint16_t max_dc, max_scale = AF_CODEC_DC_MAX_SCALE;
#endif
    float attn;

#ifdef AUDIO_OUTPUT_DC_AUTO_CALIB
    // Dig DC value comes from calibration cfg structure
    hal_codec_get_dig_dc_calib_value_high_dre_gain(&dc_l, &dc_r);
#else
    // Dig DC value comes from EFUSE
    analog_aud_get_dc_calib_value(&dc_l, &dc_r);
#endif
    if (ABS(dc_l) >= ABS(dc_r)) {
        max_dc = ABS(dc_l);
    } else {
        max_dc = ABS(dc_r);
    }
    ASSERT(max_dc + 1 < max_scale, "Bad dc values: (%d, %d)", dc_l, dc_r);
    if (max_dc) {
        attn = 1 - (float)(max_dc + 1) / max_scale;
    } else {
        attn = 1;
    }
    hal_codec_set_dac_dc_gain_attn(attn);
#ifdef AUDIO_OUTPUT_DC_CALIB_SW
    dac_dc[0] = dc_l;
    dac_dc[1] = dc_r;
#else
    hal_codec_set_dac_dc_offset(dc_l, dc_r);
#endif
}

uint32_t af_open(void)
{
    AF_TRACE_DEBUG();
    struct af_stream_cfg_t *role = NULL;

#ifdef RTOS
    if (audioflinger_mutex_id == NULL)
    {
        audioflinger_mutex_id = osMutexCreate((osMutex(audioflinger_mutex)));
    }
#endif

    af_lock_thread();

#ifdef AUDIO_OUTPUT_DC_CALIB
    if (!dac_dc_valid) {
        af_set_dac_dc_offset();
        dac_dc_valid = true;
    }
#endif

#ifdef AUDIO_OUTPUT_I2S_SW_GAIN
af_stream_sw_gain_init();
#endif

#ifdef AUDIO_OUTPUT_SW_GAIN
    hal_codec_set_sw_output_coef_callback(af_codec_dac1_output_gain_changed);
#endif

#ifdef AUDIO_OUTPUT_DAC2_SW_GAIN
    hal_codec_set_dac2_sw_output_coef_callback(af_codec_dac2_output_gain_changed);
#endif

#ifdef AUDIO_OUTPUT_DAC3_SW_GAIN
    hal_codec_set_dac3_sw_output_coef_callback(af_codec_dac3_output_gain_changed);
#endif

#ifdef __CODEC_ASYNC_CLOSE__
    codec_int_set_close_handler(af_codec_async_close);
#endif

    //initial parameters
    for(uint8_t id=0; id< AUD_STREAM_ID_NUM; id++)
    {
        for(uint8_t stream=0; stream < AUD_STREAM_NUM; stream++)
        {
            role = af_get_stream_role((enum AUD_STREAM_ID_T)id, (enum AUD_STREAM_T)stream);

            if(role->ctl.status == AF_STATUS_NULL)
            {
                role->dma_buf_ptr = NULL;
                role->dma_buf_size = 0;
                role->ctl.status = AF_STATUS_OPEN_CLOSE;
                role->ctl.use_device = AUD_STREAM_USE_DEVICE_NULL;
                role->dma_cfg.ch = HAL_DMA_CHAN_NONE;
            }
            else
            {
                ASSERT(0, "[%s] ERROR: id = %d, stream = %d", __func__, id, stream);
            }
        }
    }

#ifdef RTOS
    if (af_thread_tid == NULL) {
        af_thread_tid = osThreadCreate(osThread(af_thread), NULL);
        af_default_priority = af_get_priority();
        osSignalSet(af_thread_tid, 0x0);
    }
#endif

    af_unlock_thread();

#ifdef CODEC_ERROR_HANDLING
    af_supervisor_init();
#endif
    return AF_RES_SUCCESS;
}

static void af_stream_update_dma_buffer(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, struct af_stream_cfg_t *role, const struct AF_STREAM_CONFIG_T *cfg)
{
    int i;
    enum HAL_DMA_RET_T dma_ret;
    struct HAL_DMA_DESC_T *dma_desc, *next_desc;
    struct HAL_DMA_CH_CFG_T *dma_cfg;
    int irq;
    uint32_t desc_xfer_size;
    uint32_t align;
    uint8_t samp_size;
    enum HAL_DMA_WDITH_T width;
#ifndef CHIP_BEST1000
    bool dma_2d_en;
    uint32_t chan_desc_xfer_size = 0;
#endif

    dma_desc = &role->dma_desc[0];
    dma_cfg = &role->dma_cfg;

#ifdef CODEC_DSD
    if (stream == AUD_STREAM_PLAYBACK && role->cfg.device == AUD_STREAM_USE_INT_CODEC) {
        if (dma_cfg->dst_periph == HAL_AUDMA_DSD_TX) {
            dma_cfg->dst_bsize = HAL_DMA_BSIZE_1;
        } else {
            dma_cfg->dst_bsize = HAL_DMA_BSIZE_4;
        }
    }
#endif

    if(role->cfg.device == AUD_STREAM_USE_BT_PCM)
     {
        dma_cfg->dst_bsize = HAL_DMA_BSIZE_1;
        dma_cfg->src_bsize = HAL_DMA_BSIZE_1;
        dma_cfg->try_burst = 0;
    }

    role->dma_buf_ptr = cfg->data_ptr;
    role->dma_buf_size = cfg->data_size;

    if (cfg->bits == AUD_BITS_24 || cfg->bits == AUD_BITS_32) {
        width = HAL_DMA_WIDTH_WORD;
        samp_size = 4;
    } else if (cfg->bits == AUD_BITS_16) {
        width = HAL_DMA_WIDTH_HALFWORD;
        samp_size = 2;
    } else {
        ASSERT(false, "%s: Invalid stream config bits=%d", __func__, cfg->bits);
    }

#ifdef DYNAMIC_AUDIO_BUFFER_COUNT
    uint32_t desc_cnt;

    desc_cnt = (cfg->data_size / samp_size + HAL_DMA_MAX_DESC_XFER_SIZE - 1) / HAL_DMA_MAX_DESC_XFER_SIZE;
    if (desc_cnt < 2) {
        desc_cnt = 2;
    } else if (desc_cnt & (desc_cnt - 1)) {
        desc_cnt = 1 << (31 - __CLZ(desc_cnt) + 1);
    }
    TRACE(4,"%s: desc_cnt=%u data_size=%u samp_size=%u", __func__, desc_cnt, cfg->data_size, samp_size);
    ASSERT(MIN_AUDIO_BUFFER_COUNT <= desc_cnt && desc_cnt <= MAX_AUDIO_BUFFER_COUNT, "%s: Bad desc_cnt=%u", __func__, desc_cnt);
    role->dma_desc_cnt = desc_cnt;
#endif

    desc_xfer_size = cfg->data_size / AUDIO_BUFFER_COUNT;

#ifndef CHIP_BEST1000
    if (cfg->chan_sep_buf && cfg->channel_num > AUD_CHANNEL_NUM_1) {
        dma_2d_en = true;
    } else {
        dma_2d_en = false;
    }

    if (dma_2d_en) {
        enum HAL_DMA_BSIZE_T bsize;
        uint8_t burst_size;

        chan_desc_xfer_size = desc_xfer_size / cfg->channel_num;

        if (stream == AUD_STREAM_PLAYBACK) {
            bsize = dma_cfg->src_bsize;
        } else {
            bsize = dma_cfg->dst_bsize;
        }
        if (bsize == HAL_DMA_BSIZE_1) {
            burst_size = 1;
        } else if (bsize == HAL_DMA_BSIZE_4) {
            burst_size = 4;
        } else {
            burst_size = 8;
        }
        align = burst_size * samp_size * cfg->channel_num;
        // Ensure word-aligned too
        if (align & 0x1) {
            align *= 4;
        } else if (align & 0x2) {
            align *= 2;
        }
    } else
#endif
    {
        align = 4;
    }
    ASSERT(desc_xfer_size * AUDIO_BUFFER_COUNT == cfg->data_size && (desc_xfer_size % align) == 0,
        "%s: Dma data size is not aligned: data_size=%u AUDIO_BUFFER_COUNT=%u align=%u",
        __func__, cfg->data_size, AUDIO_BUFFER_COUNT, align);

    dma_cfg->dst_width = width;
    dma_cfg->src_width = width;
    dma_cfg->src_tsize = desc_xfer_size / samp_size;

    for (i = 0; i < AUDIO_BUFFER_COUNT; i++) {
        if (i == AUDIO_BUFFER_COUNT - 1) {
            next_desc = &dma_desc[0];
            irq = 1;
        } else {
            next_desc = &dma_desc[i + 1];
            if (i == AUDIO_BUFFER_COUNT / 2 - 1) {
                irq = 1;
            } else {
                irq = 0;
            }
        }

        if (stream == AUD_STREAM_PLAYBACK) {
#ifndef CHIP_BEST1000
            if (dma_2d_en) {
                dma_cfg->src = (uint32_t)(role->dma_buf_ptr + chan_desc_xfer_size * i);
            } else
#endif
            {
                dma_cfg->src = (uint32_t)(role->dma_buf_ptr + desc_xfer_size * i);
            }
        } else {
#ifndef CHIP_BEST1000
            if (dma_2d_en) {
                dma_cfg->dst = (uint32_t)(role->dma_buf_ptr + chan_desc_xfer_size * i);
            } else
#endif
            {
                dma_cfg->dst = (uint32_t)(role->dma_buf_ptr + desc_xfer_size * i);
            }
        }
#ifdef DMA_RPC_CLI
        if (dma_rpcif_enabled(id, stream))
        {
            dma_ret = dma_rpc_init_desc(&dma_desc[i], dma_cfg, next_desc, irq);
        } else
#endif
        {
            dma_ret = hal_audma_init_desc(&dma_desc[i], dma_cfg, next_desc, irq);
        }
        ASSERT(dma_ret == HAL_DMA_OK, "[%s] Failed to init dma desc for stream %d: ret=%d", __func__, stream, dma_ret);
    }

    _config_handler_interval_check(role);
}

// Support memory<-->peripheral
// Note: Do not support peripheral <--> peripheral

uint32_t af_stream_open(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, const struct AF_STREAM_CONFIG_T *cfg)
{
    AF_TRACE_DEBUG();
    struct af_stream_cfg_t *role;
    enum AF_RESULT_T ret;
    enum AUD_STREAM_USE_DEVICE_T device;
    struct HAL_DMA_CH_CFG_T *dma_cfg = NULL;

    role = af_get_stream_role(id, stream);
    TRACE(3,"[%s] id = %d, stream = %d", __func__, id, stream);

    ASSERT(cfg->data_ptr != NULL, "[%s] ERROR: data_ptr == NULL!!!", __func__);
    ASSERT(((uint32_t)cfg->data_ptr) % 4 == 0, "[%s] ERROR: data_ptr(%p) is not align!!!", __func__, cfg->data_ptr);
    ASSERT(cfg->data_size != 0, "[%s] ERROR: data_size == 0!!!", __func__);
#ifndef CHIP_BEST1000
    if (cfg->chan_sep_buf && cfg->channel_num > AUD_CHANNEL_NUM_1) {
        ASSERT(cfg->device == AUD_STREAM_USE_INT_CODEC || cfg->device == AUD_STREAM_USE_INT_CODEC2 ||
               cfg->device == AUD_STREAM_USE_INT_CODEC3 ||
               cfg->device == AUD_STREAM_USE_I2S0_MASTER || cfg->device == AUD_STREAM_USE_I2S0_SLAVE ||
               cfg->device == AUD_STREAM_USE_I2S1_MASTER || cfg->device == AUD_STREAM_USE_I2S1_SLAVE ||
               cfg->device == AUD_STREAM_USE_TDM0_MASTER || cfg->device == AUD_STREAM_USE_TDM0_SLAVE ||
               cfg->device == AUD_STREAM_USE_TDM1_MASTER || cfg->device == AUD_STREAM_USE_TDM1_SLAVE,
                "[%s] ERROR: Unsupport chan_sep_buf for device %d", __func__, cfg->device);
    }
#endif

    ret = AF_RES_FAILD;

    af_lock_thread();

    //check af is open
    if(role->ctl.status != AF_STATUS_OPEN_CLOSE)
    {
        TRACE(2,"[%s] ERROR: status = %d",__func__, role->ctl.status);
        goto _exit;
    }

    role->cfg = *cfg;

    device = cfg->device;
    role->ctl.use_device = device;

    dma_cfg = &role->dma_cfg;
    memset(dma_cfg, 0, sizeof(*dma_cfg));

    if(device == AUD_STREAM_USE_BT_PCM)
    {
        dma_cfg->dst_bsize = HAL_DMA_BSIZE_1;
        dma_cfg->src_bsize = HAL_DMA_BSIZE_1;
        dma_cfg->try_burst = 0;
    }
    else
    {
        dma_cfg->dst_bsize = HAL_DMA_BSIZE_4;

#ifndef CHIP_BEST1000
        // If channel num > 1, burst size should be set to 1 for:
        // 1) all playback streams with 2D DMA enabled; or
        // 2) internal codec capture streams with 2D DMA disabled.
        if (cfg->channel_num > AUD_CHANNEL_NUM_1 &&
            ((stream == AUD_STREAM_PLAYBACK && cfg->chan_sep_buf) ||
             (stream == AUD_STREAM_CAPTURE && !cfg->chan_sep_buf &&
                 (device == AUD_STREAM_USE_INT_CODEC
                     || device == AUD_STREAM_USE_INT_CODEC2)))) {
            dma_cfg->src_bsize = HAL_DMA_BSIZE_1;
        } else
#endif
        {
            dma_cfg->src_bsize = HAL_DMA_BSIZE_4;
        }
        dma_cfg->try_burst = 1;
    }

    dma_cfg->handler = af_dma_irq_handler;

    if (stream == AUD_STREAM_PLAYBACK)
    {
        AF_TRACE_DEBUG();
        dma_cfg->src_periph = (enum HAL_DMA_PERIPH_T)0;
        dma_cfg->type = HAL_DMA_FLOW_M2P_DMA;

        //open device and stream
        if(0)
        {

        }
#ifdef AF_DEVICE_EXT_CODEC
        else if(device == AUD_STREAM_USE_EXT_CODEC)
        {
            AF_TRACE_DEBUG();
            tlv32aic32_open();
            tlv32aic32_stream_open(stream);

            dma_cfg->dst_periph = HAL_AUDMA_I2S0_TX;
        }
#endif
#ifdef AF_DEVICE_I2S
        else if(device == AUD_STREAM_USE_I2S0_MASTER)
        {
            AF_TRACE_DEBUG();
            hal_i2s_open(HAL_I2S_ID_0, stream, HAL_I2S_MODE_MASTER);
            dma_cfg->dst_periph = HAL_AUDMA_I2S0_TX;
#ifdef AUDIO_OUTPUT_I2S_SW_GAIN
            af_stream_sw_gain_open(id, stream, cfg->sample_rate, cfg->bits, cfg->channel_num,
                                    AUDIO_OUTPUT_SW_GAIN_MODE_IIR);
#endif
        }
        else if(device == AUD_STREAM_USE_I2S0_SLAVE)
        {
            AF_TRACE_DEBUG();
            hal_i2s_open(HAL_I2S_ID_0, stream, HAL_I2S_MODE_SLAVE);
            dma_cfg->dst_periph = HAL_AUDMA_I2S0_TX;
        }
#if defined(CHIP_HAS_I2S) && (CHIP_HAS_I2S > 1)
        else if(device == AUD_STREAM_USE_I2S1_MASTER)
        {
            AF_TRACE_DEBUG();
            hal_i2s_open(HAL_I2S_ID_1, stream, HAL_I2S_MODE_MASTER);
            dma_cfg->dst_periph = HAL_AUDMA_I2S1_TX;
#ifdef AUDIO_OUTPUT_I2S_SW_GAIN
            af_stream_sw_gain_open(id, stream, cfg->sample_rate, cfg->bits, cfg->channel_num,
                                    AUDIO_OUTPUT_SW_GAIN_MODE_IIR);
#endif
        }
        else if(device == AUD_STREAM_USE_I2S1_SLAVE)
        {
            AF_TRACE_DEBUG();
            hal_i2s_open(HAL_I2S_ID_1, stream, HAL_I2S_MODE_SLAVE);
            dma_cfg->dst_periph = HAL_AUDMA_I2S1_TX;
        }
#endif
#endif
#ifdef AF_DEVICE_TDM
        else if(device == AUD_STREAM_USE_TDM0_MASTER)
        {
            AF_TRACE_DEBUG();
            hal_tdm_open(HAL_I2S_ID_0, stream, HAL_I2S_MODE_MASTER);
            dma_cfg->dst_periph = HAL_AUDMA_I2S0_TX;
        }
        else if(device == AUD_STREAM_USE_TDM0_SLAVE)
        {
            AF_TRACE_DEBUG();
            hal_tdm_open(HAL_I2S_ID_0, stream, HAL_I2S_MODE_SLAVE);
            dma_cfg->dst_periph = HAL_AUDMA_I2S0_TX;
        }
#if defined(CHIP_HAS_I2S) && (CHIP_HAS_I2S > 1)
        else if(device == AUD_STREAM_USE_TDM1_MASTER)
        {
            AF_TRACE_DEBUG();
            hal_tdm_open(HAL_I2S_ID_1, stream, HAL_I2S_MODE_MASTER);
            dma_cfg->dst_periph = HAL_AUDMA_I2S1_TX;
        }
        else if(device == AUD_STREAM_USE_TDM1_SLAVE)
        {
            AF_TRACE_DEBUG();
            hal_tdm_open(HAL_I2S_ID_1, stream, HAL_I2S_MODE_SLAVE);
            dma_cfg->dst_periph = HAL_AUDMA_I2S1_TX;
        }
#endif
#endif
#ifdef AF_DEVICE_INT_CODEC
        else if(device == AUD_STREAM_USE_INT_CODEC)
        {
            AF_TRACE_DEBUG();
            codec_int_open();
            codec_int_stream_open(stream);

#ifdef AUDIO_OUTPUT_SW_GAIN
#ifdef AUDIO_OUTPUT_SW_LIMITER
            FLOATLIMITER_ERROR limiter_err;

            FloatLimiterP = CreateFloatLimiter(FloatLimiterMem, FL_ATTACK_DEFAULT_MS, FL_RELEASE_DEFAULT_MS, FL_THRESHOLD, MAX_CHANEL, MAX_SAMPLERATE, MAX_SAMPLEBIT);

            limiter_err = SetFloatLimiterNChannels(FloatLimiterP, cfg->channel_num);
            if (limiter_err != FLOATLIMIT_OK) {
                goto _exit;
            }
            limiter_err = SetFloatLimiterSampleRate(FloatLimiterP, cfg->sample_rate);
            if (limiter_err != FLOATLIMIT_OK) {
                goto _exit;
            }
            limiter_err = SetFloatLimiterSampleBit(FloatLimiterP, cfg->bits);
            if (limiter_err != FLOATLIMIT_OK) {
                goto _exit;
            }
#else
            float coefs[6], gain = 0.f, f0 = 30.f, Q = 0.5f;                            //f0 larger means converge faster
            sw_gain_iir.history_x[0]=0.0f;
            sw_gain_iir.history_x[1]=0.0f;
            sw_gain_iir.history_y[0]=0.0f;
            sw_gain_iir.history_y[1]=0.0f;

            iir_lowspass_coefs_generate(gain, f0 / cfg->sample_rate, Q, coefs);
            sw_gain_iir.coefs_b[0] = coefs[3];
            sw_gain_iir.coefs_b[1] = coefs[4];
            sw_gain_iir.coefs_b[2] = coefs[5];
            sw_gain_iir.coefs_a[0] = coefs[0];
            sw_gain_iir.coefs_a[1] = coefs[1];
            sw_gain_iir.coefs_a[2] = coefs[2];

            fade_reset(&dac1_fade, cfg->sample_rate, 100);
            dac1_algo_gain = 1.f;
#endif
#ifdef AUDIO_OUTPUT_SW_GAIN_BEFORE_DRC
            af_codec_dac1_sw_gain_enable(true);
#endif
#endif


#ifdef CODEC_DSD
            if (af_dsd_enabled) {
                dma_cfg->dst_periph = HAL_AUDMA_DSD_TX;
                dma_cfg->dst_bsize = HAL_DMA_BSIZE_1;
            } else
#endif
            {
                dma_cfg->dst_periph = HAL_AUDMA_CODEC_TX;
            }
        }
#endif
#ifdef AUDIO_OUTPUT_DAC2
        else if(device == AUD_STREAM_USE_INT_CODEC2)
        {
            AF_TRACE_DEBUG();
            codec2_int_open();
            codec2_int_stream_open(stream);
            dma_cfg->dst_periph = HAL_AUDMA_CODEC_TX2;
#ifdef AUDIO_OUTPUT_DAC2_SW_GAIN
#ifdef AUDIO_OUTPUT_DAC2_SW_LIMITER
            FLOATLIMITER_ERROR limiter_err;

            Dac2FloatLimiterP = CreateFloatLimiter(Dac2FloatLimiterMem, FL_ATTACK_DEFAULT_MS, FL_RELEASE_DEFAULT_MS, FL_THRESHOLD, MAX_CHANEL, MAX_SAMPLERATE, MAX_SAMPLEBIT);

            limiter_err = SetFloatLimiterNChannels(Dac2FloatLimiterP, cfg->channel_num);
            if (limiter_err != FLOATLIMIT_OK) {
                goto _exit;
            }
            limiter_err = SetFloatLimiterSampleRate(Dac2FloatLimiterP, cfg->sample_rate);
            if (limiter_err != FLOATLIMIT_OK) {
                goto _exit;
            }
            limiter_err = SetFloatLimiterSampleBit(Dac2FloatLimiterP, cfg->bits);
            if (limiter_err != FLOATLIMIT_OK) {
                goto _exit;
            }
#else
            float coefs[6], gain = 0.f, f0 = 30.f, Q = 0.5f;                            //f0 larger means converge faster
            dac2_sw_gain_iir.history_x[0]=0.0f;
            dac2_sw_gain_iir.history_x[1]=0.0f;
            dac2_sw_gain_iir.history_y[0]=0.0f;
            dac2_sw_gain_iir.history_y[1]=0.0f;

            iir_lowspass_coefs_generate(gain, f0 / cfg->sample_rate, Q, coefs);
            dac2_sw_gain_iir.coefs_b[0] = coefs[3];
            dac2_sw_gain_iir.coefs_b[1] = coefs[4];
            dac2_sw_gain_iir.coefs_b[2] = coefs[5];
            dac2_sw_gain_iir.coefs_a[0] = coefs[0];
            dac2_sw_gain_iir.coefs_a[1] = coefs[1];
            dac2_sw_gain_iir.coefs_a[2] = coefs[2];
#endif
#endif /* #ifdef AUDIO_OUTPUT_DAC2_SW_GAIN */
        }
#endif
#ifdef AUDIO_OUTPUT_DAC3
        else if(device == AUD_STREAM_USE_INT_CODEC3)
        {
            AF_TRACE_DEBUG();
            codec3_int_open();
            codec3_int_stream_open(stream);
            dma_cfg->dst_periph = HAL_AUDMA_CODEC_TX3;
#ifdef AUDIO_OUTPUT_DAC3_SW_GAIN
#ifdef AUDIO_OUTPUT_DAC3_SW_LIMITER
            FLOATLIMITER_ERROR limiter_err;

            Dac3FloatLimiterP = CreateFloatLimiter(Dac3FloatLimiterMem, FL_ATTACK_DEFAULT_MS, FL_RELEASE_DEFAULT_MS, FL_THRESHOLD, MAX_CHANEL, MAX_SAMPLERATE, MAX_SAMPLEBIT);

            limiter_err = SetFloatLimiterNChannels(Dac3FloatLimiterP, cfg->channel_num);
            if (limiter_err != FLOATLIMIT_OK) {
                goto _exit;
            }
            limiter_err = SetFloatLimiterSampleRate(Dac3FloatLimiterP, cfg->sample_rate);
            if (limiter_err != FLOATLIMIT_OK) {
                goto _exit;
            }
            limiter_err = SetFloatLimiterSampleBit(Dac3FloatLimiterP, cfg->bits);
            if (limiter_err != FLOATLIMIT_OK) {
                goto _exit;
            }
#else
            float coefs[6], gain = 0.f, f0 = 30.f, Q = 0.5f;                            //f0 larger means converge faster
            dac3_sw_gain_iir.history_x[0]=0.0f;
            dac3_sw_gain_iir.history_x[1]=0.0f;
            dac3_sw_gain_iir.history_y[0]=0.0f;
            dac3_sw_gain_iir.history_y[1]=0.0f;

            iir_lowspass_coefs_generate(gain, f0 / cfg->sample_rate, Q, coefs);
            dac3_sw_gain_iir.coefs_b[0] = coefs[3];
            dac3_sw_gain_iir.coefs_b[1] = coefs[4];
            dac3_sw_gain_iir.coefs_b[2] = coefs[5];
            dac3_sw_gain_iir.coefs_a[0] = coefs[0];
            dac3_sw_gain_iir.coefs_a[1] = coefs[1];
            dac3_sw_gain_iir.coefs_a[2] = coefs[2];
#endif
#endif /* #ifdef AUDIO_OUTPUT_DAC3_SW_GAIN */
        }
#endif
#ifdef AF_DEVICE_SPDIF
        else if(device == AUD_STREAM_USE_SPDIF0)
        {
            AF_TRACE_DEBUG();
            hal_spdif_open(HAL_SPDIF_ID_0, stream);
            dma_cfg->dst_periph = HAL_AUDMA_SPDIF0_TX;
        }
#ifdef SPDIF1_BASE
        else if(device == AUD_STREAM_USE_SPDIF1)
        {
            AF_TRACE_DEBUG();
            hal_spdif_open(HAL_SPDIF_ID_1, stream);
            dma_cfg->dst_periph = HAL_AUDMA_SPDIF1_TX;
        }
#endif
#endif
#ifdef AF_DEVICE_BT_PCM
        else if(device == AUD_STREAM_USE_BT_PCM)
        {
            AF_TRACE_DEBUG();
            hal_btpcm_open(AF_BTPCM_INST, stream);
            dma_cfg->dst_periph = HAL_AUDMA_BTPCM_TX;
        }
#endif
#ifdef AUDIO_ANC_FB_MC
        else if(device == AUD_STREAM_USE_MC)
        {
            AF_TRACE_DEBUG();
            hal_codec_mc_enable();
            dma_cfg->dst_periph = HAL_AUDMA_MC_RX;
        }
#endif
        else
        {
            ASSERT(0, "[%s] ERROR: device %d is not defined!", __func__, device);
        }
#ifdef DMA_RPC_CLI
        if (dma_rpcif_enabled(id, stream))
        {
            dma_cfg->ch = dma_rpc_get_chan(dma_cfg->dst_periph, HAL_DMA_HIGH_PRIO);
            role->dma_base = dma_rpc_get_base_addr(dma_cfg->ch);
        } else
#endif
        {
            dma_cfg->ch = hal_audma_get_chan(dma_cfg->dst_periph, HAL_DMA_HIGH_PRIO);
            role->dma_base = hal_audma_get_base_addr(dma_cfg->ch);
        }
    }
    else
    {
        AF_TRACE_DEBUG();
        dma_cfg->dst_periph = (enum HAL_DMA_PERIPH_T)0;
        dma_cfg->type = HAL_DMA_FLOW_P2M_DMA;

        //open device and stream
        if(0)
        {
        }
#ifdef AF_DEVICE_EXT_CODEC
        else if(device == AUD_STREAM_USE_EXT_CODEC)
        {
            AF_TRACE_DEBUG();
            tlv32aic32_open();
            tlv32aic32_stream_open(stream);

            dma_cfg->src_periph = HAL_AUDMA_I2S0_RX;
        }
#endif
#ifdef AF_DEVICE_I2S
        else if(device == AUD_STREAM_USE_I2S0_MASTER)
        {
            AF_TRACE_DEBUG();
            hal_i2s_open(HAL_I2S_ID_0, stream, HAL_I2S_MODE_MASTER);
            dma_cfg->src_periph = HAL_AUDMA_I2S0_RX;
        }
        else if(device == AUD_STREAM_USE_I2S0_SLAVE)
        {
            AF_TRACE_DEBUG();
            hal_i2s_open(HAL_I2S_ID_0, stream, HAL_I2S_MODE_SLAVE);
            dma_cfg->src_periph = HAL_AUDMA_I2S0_RX;
        }
#if defined(CHIP_HAS_I2S) && (CHIP_HAS_I2S > 1)
        else if(device == AUD_STREAM_USE_I2S1_MASTER)
        {
            AF_TRACE_DEBUG();
            hal_i2s_open(HAL_I2S_ID_1, stream, HAL_I2S_MODE_MASTER);
            dma_cfg->src_periph = HAL_AUDMA_I2S1_RX;
        }
        else if(device == AUD_STREAM_USE_I2S1_SLAVE)
        {
            AF_TRACE_DEBUG();
            hal_i2s_open(HAL_I2S_ID_1, stream, HAL_I2S_MODE_SLAVE);
            dma_cfg->src_periph = HAL_AUDMA_I2S1_RX;
        }
#endif
#endif
#ifdef AF_DEVICE_TDM
        else if(device == AUD_STREAM_USE_TDM0_MASTER)
        {
            AF_TRACE_DEBUG();
            hal_tdm_open(HAL_I2S_ID_0, stream, HAL_I2S_MODE_MASTER);
            dma_cfg->src_periph = HAL_AUDMA_I2S0_RX;
            AF_TRACE_DEBUG();
        }
        else if(device == AUD_STREAM_USE_TDM0_SLAVE)
        {
            AF_TRACE_DEBUG();
            hal_tdm_open(HAL_I2S_ID_0, stream, HAL_I2S_MODE_SLAVE);
            dma_cfg->src_periph = HAL_AUDMA_I2S0_RX;
        }
#if defined(CHIP_HAS_I2S) && (CHIP_HAS_I2S > 1)
        else if(device == AUD_STREAM_USE_TDM1_MASTER)
        {
            AF_TRACE_DEBUG();
            hal_tdm_open(HAL_I2S_ID_1, stream, HAL_I2S_MODE_MASTER);
            dma_cfg->src_periph = HAL_AUDMA_I2S1_RX;
        }
        else if(device == AUD_STREAM_USE_TDM1_SLAVE)
        {
            AF_TRACE_DEBUG();
            hal_tdm_open(HAL_I2S_ID_1, stream, HAL_I2S_MODE_SLAVE);
            dma_cfg->src_periph = HAL_AUDMA_I2S1_RX;
        }
#endif
#endif
#ifdef AF_DEVICE_INT_CODEC
        else if(device == AUD_STREAM_USE_INT_CODEC)
        {
            AF_TRACE_DEBUG();
            codec_int_open();
            codec_int_stream_open(stream);
            dma_cfg->src_periph = HAL_AUDMA_CODEC_RX;
        }
#endif
#ifdef AUDIO_OUTPUT_DAC2
        else if(device == AUD_STREAM_USE_INT_CODEC2)
        {
            AF_TRACE_DEBUG();
            codec2_int_open();
            codec2_int_stream_open(stream);
            dma_cfg->src_periph = HAL_AUDMA_CODEC_RX2;
        }
#endif
#ifdef AF_DEVICE_SPDIF
        else if(device == AUD_STREAM_USE_SPDIF0)
        {
            AF_TRACE_DEBUG();
            hal_spdif_open(HAL_SPDIF_ID_0, stream);
            dma_cfg->src_periph = HAL_AUDMA_SPDIF0_RX;
        }
#ifdef SPDIF1_BASE
        else if(device == AUD_STREAM_USE_SPDIF1)
        {
            AF_TRACE_DEBUG();
            hal_spdif_open(HAL_SPDIF_ID_1, stream);
            dma_cfg->src_periph = HAL_AUDMA_SPDIF1_RX;
        }
#endif
#endif
#ifdef AF_DEVICE_BT_PCM
        else if(device == AUD_STREAM_USE_BT_PCM)
        {
            AF_TRACE_DEBUG();
            hal_btpcm_open(AF_BTPCM_INST, stream);

            dma_cfg->src_periph = HAL_AUDMA_BTPCM_RX;
        }
#endif
#ifdef AF_DEVICE_DPD_RX
        else if(device == AUD_STREAM_USE_DPD_RX)
        {
            AF_TRACE_DEBUG();
            dma_cfg->src_periph = HAL_AUDMA_DPD_RX;
        }
#endif
        else
        {
            ASSERT(0, "[%s] ERROR: device %d is not defined!", __func__, device);
        }

#ifdef DMA_RPC_CLI
        if (dma_rpcif_enabled(id, stream))
        {
            dma_cfg->ch = dma_rpc_get_chan(dma_cfg->src_periph, HAL_DMA_HIGH_PRIO);
            role->dma_base = dma_rpc_get_base_addr(dma_cfg->ch);
        } else
#endif
        {
            dma_cfg->ch = hal_audma_get_chan(dma_cfg->src_periph, HAL_DMA_HIGH_PRIO);
            role->dma_base = hal_audma_get_base_addr(dma_cfg->ch);
        }
    }
    af_stream_update_dma_buffer(id, stream, role, cfg);
    role->handler = cfg->handler;

    af_set_status(id, stream, AF_STATUS_STREAM_OPEN_CLOSE);
#ifndef RTOS
    af_clear_flag(&af_flag_signal, 1 << (id * 2 + stream));
    af_set_flag(&af_flag_open, 1 << (id * 2 + stream));
#endif
#ifdef DMA_RPC_CLI
    if (dma_rpcif_enabled(id, stream))
    {
        dma_rpc_stream_open(id, stream);
    }
#endif

    AF_TRACE_DEBUG();
    ret = AF_RES_SUCCESS;

_exit:

    if (ret == AF_RES_SUCCESS) {
        af_stream_setup(id, stream, &role->cfg);
    }
    af_unlock_thread();

    return ret;
}

//volume, path, sample rate, channel num ...
uint32_t af_stream_setup(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, const struct AF_STREAM_CONFIG_T *cfg)
{
    AF_TRACE_DEBUG();

    struct af_stream_cfg_t *role;
    enum AF_RESULT_T ret;
    enum AUD_STREAM_USE_DEVICE_T device;

    role = af_get_stream_role(id, stream);
    TRACE(3,"[%s] id = %d, stream = %d", __func__, id, stream);

    ret = AF_RES_FAILD;

    af_lock_thread();

    //check stream is open
    if(!(role->ctl.status & AF_STATUS_STREAM_OPEN_CLOSE))
    {
        TRACE(2,"[%s] ERROR: status = %d", __func__, role->ctl.status);
        goto _exit;
    }

    device = role->ctl.use_device;

    if (&role->cfg != cfg) {
        bool update_dma = false;
        bool update_intvl_chk = false;

        if (role->cfg.bits != cfg->bits) {
            TRACE(4,"[%s] Change bits from %d to %d for stream %d", __func__, role->cfg.bits, cfg->bits, stream);
            update_dma = true;
        }
        if (role->cfg.data_ptr != cfg->data_ptr) {
            TRACE(4,"[%s] Change data_ptr from %p to %p for stream %d",
                            __func__, role->cfg.data_ptr, cfg->data_ptr, stream);
            update_dma = true;
        }
        if (role->cfg.data_size != cfg->data_size) {
            TRACE(4,"[%s] Change data_size from %d to %d for stream %d",
                            __func__, role->cfg.data_size, cfg->data_size, stream);
            update_dma = true;
        }
        if (update_dma) {
            // To avoid FIFO corruption, streams must be stopped before changing sample size
            ASSERT((role->ctl.status & AF_STATUS_STREAM_START_STOP) == 0,
                "[%s] ERROR: Update dma while stream %d started", __func__, stream);

            af_stream_update_dma_buffer(id, stream, role, cfg);
        }

        if (role->cfg.sample_rate != cfg->sample_rate) {
            TRACE(4,"[%s] Change sample rate from %d to %d for stream %d",
                    __func__, role->cfg.sample_rate, cfg->sample_rate, stream);

            // To avoid L/R sample misalignment, streams must be stopped before changing sample rate
            ASSERT((role->ctl.status & AF_STATUS_STREAM_START_STOP) == 0,
                "[%s] ERROR: Change sample rate from %d to %d while stream %d started",
                __func__, role->cfg.sample_rate, cfg->sample_rate, stream);
        }

        if (role->cfg.sample_rate != cfg->sample_rate || role->cfg.channel_num != cfg->channel_num) {
            // Changes in data_size and/or bits have been processed in af_stream_update_dma_buffer()
            // Changes in device is forbidden
            update_intvl_chk = true;
        }

        role->cfg = *cfg;

        if (update_intvl_chk) {
            _config_handler_interval_check(role);
        }
    }

    AF_TRACE_DEBUG();
    if(0)
    {
    }
#ifdef AF_DEVICE_EXT_CODEC
    else if(device == AUD_STREAM_USE_EXT_CODEC)
    {
        AF_TRACE_DEBUG();

        struct tlv32aic32_config_t tlv32aic32_cfg;

        memset(&tlv32aic32_cfg, 0, sizeof(tlv32aic32_cfg));
        tlv32aic32_cfg.bits = cfg->bits;
        tlv32aic32_cfg.channel_num = cfg->channel_num;
        tlv32aic32_cfg.channel_map = cfg->channel_map;
        tlv32aic32_cfg.sample_rate = cfg->sample_rate;
        tlv32aic32_cfg.use_dma = AF_TRUE;
        tlv32aic32_cfg.chan_sep_buf = cfg->chan_sep_buf;
        tlv32aic32_cfg.sync_start = cfg->sync_start;
        tlv32aic32_cfg.slot_cycles = cfg->slot_cycles;
        tlv32aic32_stream_setup(stream, &tlv32aic32_cfg);
    }
#endif
#ifdef AF_DEVICE_I2S
    else if(device == AUD_STREAM_USE_I2S0_MASTER || device == AUD_STREAM_USE_I2S0_SLAVE
#if defined(CHIP_HAS_I2S) && (CHIP_HAS_I2S > 1)
            || device == AUD_STREAM_USE_I2S1_MASTER || device == AUD_STREAM_USE_I2S1_SLAVE
#endif
            )
    {
        AF_TRACE_DEBUG();

        struct HAL_I2S_CONFIG_T i2s_cfg;
        enum HAL_I2S_ID_T i2s_id;

        i2s_id = HAL_I2S_ID_0;
#if defined(CHIP_HAS_I2S) && (CHIP_HAS_I2S > 1)
        if(device == AUD_STREAM_USE_I2S1_MASTER || device == AUD_STREAM_USE_I2S1_SLAVE) {
            i2s_id = HAL_I2S_ID_1;
        }
#endif

        memset(&i2s_cfg, 0, sizeof(i2s_cfg));
        i2s_cfg.use_dma = AF_TRUE;
        i2s_cfg.chan_sep_buf = cfg->chan_sep_buf;
        i2s_cfg.sync_start = cfg->sync_start;
        i2s_cfg.cycles = cfg->slot_cycles;
        i2s_cfg.bits = cfg->bits;
        i2s_cfg.channel_num = cfg->channel_num;
        i2s_cfg.channel_map = cfg->channel_map;
        i2s_cfg.sample_rate = cfg->sample_rate;
        i2s_cfg.ext_mclk_freq = cfg->ext_mclk_freq;
        hal_i2s_setup_stream(i2s_id, stream, &i2s_cfg);
#ifdef AUDIO_OUTPUT_I2S_SW_GAIN
        if (device == AUD_STREAM_USE_I2S0_MASTER
#ifdef I2S1_BASE
            || device == AUD_STREAM_USE_I2S1_MASTER
#endif
        ) {
            sw_gain_i2s_set_volume(id, stream, cfg->vol);
        }
#endif
}
#endif

#ifdef AF_DEVICE_TDM
    else if(device == AUD_STREAM_USE_TDM0_MASTER || device == AUD_STREAM_USE_TDM0_SLAVE
#if defined(CHIP_HAS_I2S) && (CHIP_HAS_I2S > 1)
            || device == AUD_STREAM_USE_TDM1_MASTER || device == AUD_STREAM_USE_TDM1_SLAVE
#endif
            )
    {
        AF_TRACE_DEBUG();

        struct HAL_TDM_CONFIG_T tdm_cfg;
        enum HAL_I2S_ID_T i2s_id;

        i2s_id = HAL_I2S_ID_0;
#if defined(CHIP_HAS_I2S) && (CHIP_HAS_I2S > 1)
        if(device == AUD_STREAM_USE_TDM1_MASTER || device == AUD_STREAM_USE_TDM1_SLAVE) {
            i2s_id = HAL_I2S_ID_1;
        }
#endif

        memset(&tdm_cfg, 0, sizeof(tdm_cfg));
        tdm_cfg.mode = (cfg->align == AUD_DATA_ALIGN_I2S) ? HAL_TDM_MODE_FS_ASSERTED_AT_LAST : HAL_TDM_MODE_FS_ASSERTED_AT_FIRST;
        tdm_cfg.edge = (cfg->fs_edge == AUD_FS_FIRST_EDGE_NEG) ? HAL_TDM_FS_EDGE_NEGEDGE : HAL_TDM_FS_EDGE_POSEDGE;
        tdm_cfg.cycles = (cfg->channel_num * cfg->slot_cycles);
        tdm_cfg.fs_cycles = cfg->fs_cycles;
        tdm_cfg.slot_cycles = cfg->slot_cycles;
        tdm_cfg.bits = cfg->bits;
        tdm_cfg.channel_num = cfg->channel_num;
        tdm_cfg.data_offset = (cfg->align == AUD_DATA_ALIGN_RIGHT_JUSTIFIED) ? (cfg->slot_cycles - cfg->bits) : 0;
        tdm_cfg.chan_sep_buf = cfg->chan_sep_buf;
        tdm_cfg.sync_start = cfg->sync_start;
        tdm_cfg.ext_mclk_freq = cfg->ext_mclk_freq;
        TRACE(5,"%s: cycle=%d fs_cycles=%d slot_cycles=%d wait=%d",
            __func__, tdm_cfg.cycles, tdm_cfg.fs_cycles, tdm_cfg.slot_cycles,tdm_cfg.sync_start);
        hal_tdm_setup_stream(i2s_id, stream, cfg->sample_rate, &tdm_cfg);
    }

#endif
#ifdef AF_DEVICE_INT_CODEC
    else if(device == AUD_STREAM_USE_INT_CODEC)
    {
        AF_TRACE_DEBUG();
        struct HAL_CODEC_CONFIG_T codec_cfg;
        memset(&codec_cfg, 0, sizeof(codec_cfg));
        codec_cfg.bits = cfg->bits;
        codec_cfg.sample_rate = cfg->sample_rate;
        codec_cfg.channel_num = cfg->channel_num;
        codec_cfg.channel_map = cfg->channel_map;
        codec_cfg.use_dma = AF_TRUE;
        codec_cfg.vol = cfg->vol;
        codec_cfg.io_path = cfg->io_path;

        AF_TRACE_DEBUG();
        TRACE(2, "[%s] current vol:%d", __func__, cfg->vol);
        codec_int_stream_setup(stream, &codec_cfg);
    }
#endif
#ifdef AUDIO_OUTPUT_DAC2
    else if(device == AUD_STREAM_USE_INT_CODEC2)
    {
        AF_TRACE_DEBUG();
        struct HAL_CODEC_CONFIG_T codec_cfg;
        memset(&codec_cfg, 0, sizeof(codec_cfg));
        codec_cfg.bits = cfg->bits;
        codec_cfg.sample_rate = cfg->sample_rate;
        codec_cfg.channel_num = cfg->channel_num;
        codec_cfg.channel_map = cfg->channel_map;
        codec_cfg.use_dma = AF_TRUE;
        codec_cfg.vol = cfg->vol;
        codec_cfg.io_path = cfg->io_path;

        AF_TRACE_DEBUG();
        TRACE(2, "[%s] current vol:%d", __func__, cfg->vol);
        codec2_int_stream_setup(stream, &codec_cfg);
    }
#endif
#ifdef AUDIO_OUTPUT_DAC3
    else if(device == AUD_STREAM_USE_INT_CODEC3)
    {
        AF_TRACE_DEBUG();
        struct HAL_CODEC_CONFIG_T codec_cfg;
        memset(&codec_cfg, 0, sizeof(codec_cfg));
        codec_cfg.bits = cfg->bits;
        codec_cfg.sample_rate = cfg->sample_rate;
        codec_cfg.channel_num = cfg->channel_num;
        codec_cfg.channel_map = cfg->channel_map;
        codec_cfg.use_dma = AF_TRUE;
        codec_cfg.vol = cfg->vol;
        codec_cfg.io_path = cfg->io_path;

        AF_TRACE_DEBUG();
        TRACE(2, "[%s] current vol:%d", __func__, cfg->vol);
        codec3_int_stream_setup(stream, &codec_cfg);
    }
#endif
#ifdef AF_DEVICE_SPDIF
    else if(device == AUD_STREAM_USE_SPDIF0
#ifdef SPDIF1_BASE
            || device == AUD_STREAM_USE_SPDIF1
#endif
            )
    {
        AF_TRACE_DEBUG();
        struct HAL_SPDIF_CONFIG_T spdif_cfg;
        enum HAL_SPDIF_ID_T spdif_id;

        spdif_id = HAL_SPDIF_ID_0;
#ifdef SPDIF1_BASE
        if (device == AUD_STREAM_USE_SPDIF1) {
            spdif_id = HAL_SPDIF_ID_1;
        }
#endif

        memset(&spdif_cfg, 0, sizeof(spdif_cfg));
        spdif_cfg.use_dma = AF_TRUE;
        spdif_cfg.bits = cfg->bits;
        spdif_cfg.channel_num = cfg->channel_num;
        spdif_cfg.sample_rate = cfg->sample_rate;
        hal_spdif_setup_stream(spdif_id, stream, &spdif_cfg);
    }
#endif
#ifdef AF_DEVICE_BT_PCM
    else if(device == AUD_STREAM_USE_BT_PCM)
    {
        AF_TRACE_DEBUG();
        struct HAL_BTPCM_CONFIG_T btpcm_cfg;

        memset(&btpcm_cfg, 0, sizeof(btpcm_cfg));
        btpcm_cfg.use_dma = AF_TRUE;
        btpcm_cfg.bits = cfg->bits;
        btpcm_cfg.channel_num = cfg->channel_num;
        btpcm_cfg.sample_rate = cfg->sample_rate;
        hal_btpcm_setup_stream(AF_BTPCM_INST, stream, &btpcm_cfg);
    }
#endif
#ifdef AF_DEVICE_DPD_RX
    else if(device == AUD_STREAM_USE_DPD_RX)
    {
        AF_TRACE_DEBUG();
    }
#endif
#ifdef AUDIO_ANC_FB_MC
    else if(device == AUD_STREAM_USE_MC)
    {
        AF_TRACE_DEBUG();
        hal_codec_setup_mc(cfg->channel_num, cfg->bits);
    }
#endif
    else
    {
        ASSERT(0, "[%s] ERROR: device %d is not defined!", __func__, device);
    }

    AF_TRACE_DEBUG();

    ret = AF_RES_SUCCESS;

_exit:
    af_unlock_thread();

    return ret;
}

uint32_t af_stream_mute(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, bool mute)
{
    AF_TRACE_DEBUG();

    struct af_stream_cfg_t *role;
    enum AF_RESULT_T ret;
    POSSIBLY_UNUSED enum AUD_STREAM_USE_DEVICE_T device;

    ret = AF_RES_FAILD;
    role = af_get_stream_role(id, stream);

    af_lock_thread();

    if ((role->ctl.status & AF_STATUS_STREAM_OPEN_CLOSE) == 0) {
        TRACE(2,"[%s] ERROR: status = %d", __func__, role->ctl.status);
        goto _exit;
    }

    device = role->ctl.use_device;

#ifdef AF_DEVICE_INT_CODEC
    if (device == AUD_STREAM_USE_INT_CODEC) {
        codec_int_stream_mute(stream, mute);
        ret = AF_RES_SUCCESS;
#ifdef AUDIO_OUTPUT_DAC2
    } else if (device == AUD_STREAM_USE_INT_CODEC2) {
        codec2_int_stream_mute(stream, mute);
        ret = AF_RES_SUCCESS;
#endif
#ifdef AUDIO_OUTPUT_DAC3
    } else if (device == AUD_STREAM_USE_INT_CODEC3) {
        codec3_int_stream_mute(stream, mute);
        ret = AF_RES_SUCCESS;
#endif
    }
#endif//AF_DEVICE_INT_CODEC

_exit:
    af_unlock_thread();

    return ret;
}

uint32_t af_stream_set_chan_vol(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, enum AUD_CHANNEL_MAP_T ch_map, uint8_t vol)
{
    AF_TRACE_DEBUG();

    struct af_stream_cfg_t *role;
    enum AF_RESULT_T ret;
    POSSIBLY_UNUSED enum AUD_STREAM_USE_DEVICE_T device;

    ret = AF_RES_FAILD;
    role = af_get_stream_role(id, stream);

    af_lock_thread();

    if ((role->ctl.status & AF_STATUS_STREAM_OPEN_CLOSE) == 0) {
        TRACE(2,"[%s] ERROR: status = %d", __func__, role->ctl.status);
        goto _exit;
    }

    device = role->ctl.use_device;

#ifdef AF_DEVICE_INT_CODEC
    if (device == AUD_STREAM_USE_INT_CODEC) {
        codec_int_stream_set_chan_vol(stream, ch_map, vol);
        ret = AF_RES_SUCCESS;
#ifdef AUDIO_OUTPUT_DAC2
    } else if (device == AUD_STREAM_USE_INT_CODEC2) {
        codec2_int_stream_set_chan_vol(stream, ch_map, vol);
        ret = AF_RES_SUCCESS;
#endif
#ifdef AUDIO_OUTPUT_DAC3
    } else if (device == AUD_STREAM_USE_INT_CODEC3) {
        codec3_int_stream_set_chan_vol(stream, ch_map, vol);
        ret = AF_RES_SUCCESS;
#endif
    }
#endif//AF_DEVICE_INT_CODEC

_exit:
    af_unlock_thread();

    return ret;
}

uint32_t af_stream_restore_chan_vol(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    AF_TRACE_DEBUG();

    struct af_stream_cfg_t *role;
    enum AF_RESULT_T ret;
    POSSIBLY_UNUSED enum AUD_STREAM_USE_DEVICE_T device;

    ret = AF_RES_FAILD;
    role = af_get_stream_role(id, stream);

    af_lock_thread();

    if ((role->ctl.status & AF_STATUS_STREAM_OPEN_CLOSE) == 0) {
        TRACE(2,"[%s] ERROR: status = %d", __func__, role->ctl.status);
        goto _exit;
    }

    device = role->ctl.use_device;

#ifdef AF_DEVICE_INT_CODEC
    if (device == AUD_STREAM_USE_INT_CODEC) {
        codec_int_stream_restore_chan_vol(stream);
        ret = AF_RES_SUCCESS;
#ifdef AUDIO_OUTPUT_DAC2
    } else if (device == AUD_STREAM_USE_INT_CODEC2) {
        codec2_int_stream_restore_chan_vol(stream);
        ret = AF_RES_SUCCESS;
#endif
#ifdef AUDIO_OUTPUT_DAC3
    } else if (device == AUD_STREAM_USE_INT_CODEC3) {
        codec3_int_stream_restore_chan_vol(stream);
        ret = AF_RES_SUCCESS;
#endif
    }
#endif

_exit:
    af_unlock_thread();

    return ret;
}

#ifdef CODEC_PLAY_BEFORE_CAPTURE
static struct af_stream_cfg_t *codec_capture_role;

static void af_codec_stream_pre_start(enum AUD_STREAM_T stream)
{
    struct af_stream_cfg_t *role = NULL;

    if (stream == AUD_STREAM_CAPTURE) {
        return;
    }

    for(uint8_t id=0; id< AUD_STREAM_ID_NUM; id++)
    {
        role = af_get_stream_role((enum AUD_STREAM_ID_T)id, AUD_STREAM_CAPTURE);
        if (role->cfg.device == AUD_STREAM_USE_INT_CODEC &&
                role->ctl.status == (AF_STATUS_OPEN_CLOSE | AF_STATUS_STREAM_OPEN_CLOSE | AF_STATUS_STREAM_START_STOP)) {
            hal_audma_stop(role->dma_cfg.ch);
            codec_int_stream_stop(AUD_STREAM_CAPTURE);
            codec_capture_role = role;
            return;
        }
    }
}

static void af_codec_stream_post_start(enum AUD_STREAM_T stream)
{
    if (stream == AUD_STREAM_CAPTURE) {
        return;
    }

    if (codec_capture_role) {
        hal_audma_sg_start(&codec_capture_role->dma_desc[0], &codec_capture_role->dma_cfg);
        codec_int_stream_start(AUD_STREAM_CAPTURE);
        codec_capture_role = NULL;
    }
}

static void af_codec_stream_pre_stop(enum AUD_STREAM_T stream)
{
    struct af_stream_cfg_t *role = NULL;

    if (stream == AUD_STREAM_CAPTURE) {
        return;
    }

    for(uint8_t id=0; id< AUD_STREAM_ID_NUM; id++)
    {
        role = af_get_stream_role((enum AUD_STREAM_ID_T)id, AUD_STREAM_CAPTURE);
        if (role->cfg.device == AUD_STREAM_USE_INT_CODEC &&
                role->ctl.status == (AF_STATUS_OPEN_CLOSE | AF_STATUS_STREAM_OPEN_CLOSE | AF_STATUS_STREAM_START_STOP)) {
            hal_audma_stop(role->dma_cfg.ch);
            codec_int_stream_stop(AUD_STREAM_CAPTURE);
            codec_capture_role = role;
            return;
        }
    }
}

static void af_codec_stream_post_stop(enum AUD_STREAM_T stream)
{
    if (stream == AUD_STREAM_CAPTURE) {
        return;
    }

    if (codec_capture_role) {
        hal_audma_sg_start(&codec_capture_role->dma_desc[0], &codec_capture_role->dma_cfg);
        codec_int_stream_start(AUD_STREAM_CAPTURE);
        codec_capture_role = NULL;
    }
}
#endif

uint32_t af_stream_start(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    struct af_stream_cfg_t *role;
    enum AF_RESULT_T ret;
    enum AUD_STREAM_USE_DEVICE_T device;
    enum HAL_DMA_RET_T dma_ret;

    role = af_get_stream_role(id, stream);
    TRACE(3,"[%s] id = %d, stream = %d", __func__, id, stream);

    af_lock_thread();

    //check stream is open and not start.
    if(role->ctl.status != (AF_STATUS_OPEN_CLOSE | AF_STATUS_STREAM_OPEN_CLOSE))
    {
        TRACE(2,"[%s] ERROR: status = %d",__func__, role->ctl.status);
        ret = AF_RES_FAILD;
        goto _exit;
    }

    device = role->ctl.use_device;

    role->ctl.pp_index = PP_PING;
    role->ctl.pp_cnt = 0;
    role->ctl.prev_hdlr_time = _get_fast_time_stamp();
    role->ctl.first_hdlr_proc = true;

#ifndef RTOS
    af_clear_flag(&af_flag_signal, 1 << (id * 2 + stream));
#endif

    if (device == AUD_STREAM_USE_INT_CODEC && stream == AUD_STREAM_PLAYBACK) {
#ifdef AUDIO_OUTPUT_PA_ON_FADE_IN
        dac_pa_state = AF_DAC_PA_ON_TRIGGER;
        // Has the buffer been zero-ed out?
        af_zero_mem(role->dma_buf_ptr, role->dma_buf_size);
#ifdef AUDIO_OUTPUT_DC_CALIB_SW
        af_codec_playback_sw_dc_calib(role->dma_buf_ptr, role->dma_buf_size, role->cfg.bits, role->cfg.channel_num);
#endif
        dac_dc_start_time = hal_sys_timer_get();
#endif
    }

#ifndef CHIP_BEST1000
    if (role->cfg.chan_sep_buf && role->cfg.channel_num > AUD_CHANNEL_NUM_1) {
        struct HAL_DMA_2D_CFG_T *src, *dst;
        struct HAL_DMA_2D_CFG_T dma_2d_cfg;
        uint8_t burst_size = 1;
        uint32_t chan_xfer_cnt;

        src = NULL;
        dst = NULL;
        if (stream == AUD_STREAM_PLAYBACK) {
            if (role->cfg.channel_num == AUD_CHANNEL_NUM_2) {
                ASSERT(role->dma_cfg.src_bsize == HAL_DMA_BSIZE_1,
                    "Play 2D DMA: Bad src burst size: %d", role->dma_cfg.src_bsize);
                burst_size = 1;
                src = &dma_2d_cfg;
            }
        } else {
            if (role->cfg.channel_num > AUD_CHANNEL_NUM_1) {
                ASSERT(role->dma_cfg.src_bsize == role->dma_cfg.dst_bsize,
                    "Cap 2D DMA: src burst size (%d) != dst (%d)", role->dma_cfg.src_bsize, role->dma_cfg.dst_bsize);
                if (role->dma_cfg.dst_bsize == HAL_DMA_BSIZE_1) {
                    burst_size = 1;
                } else if (role->dma_cfg.dst_bsize == HAL_DMA_BSIZE_4) {
                    burst_size = 4;
                } else if (role->dma_cfg.dst_bsize == HAL_DMA_BSIZE_8) {
                    burst_size = 8;
                } else {
                    ASSERT(false, "Cap 2D DMA: Bad dst burst size: %d", role->dma_cfg.dst_bsize);
                }
                dst = &dma_2d_cfg;
            }
        }

        if (src || dst) {
            chan_xfer_cnt = role->dma_cfg.src_tsize * AUDIO_BUFFER_COUNT / role->cfg.channel_num;
            dma_2d_cfg.xcount = role->cfg.channel_num;
            dma_2d_cfg.xmodify = chan_xfer_cnt - burst_size;
            dma_2d_cfg.ycount = chan_xfer_cnt / burst_size;
            dma_2d_cfg.ymodify = -chan_xfer_cnt * (role->cfg.channel_num - 1);
        }
#ifdef DMA_RPC_CLI
        if (dma_rpcif_enabled(id, stream))
        {
            dma_ret = dma_rpc_sg_2d_start(&role->dma_desc[0], &role->dma_cfg, src, dst);
        } else
#endif
        {
            dma_ret = hal_dma_sg_2d_start(&role->dma_desc[0], &role->dma_cfg, src, dst);
        }
    } else
#endif
    {
#ifdef DMA_RPC_CLI
        if (dma_rpcif_enabled(id, stream))
        {
            dma_ret = dma_rpc_sg_start(&role->dma_desc[0], &role->dma_cfg);
        } else
#endif
        {
            dma_ret = hal_dma_sg_start(&role->dma_desc[0], &role->dma_cfg);
        }
    }
    ASSERT(dma_ret == HAL_DMA_OK, "[%s] Failed to start dma for stream %d: ret=%d", __func__, stream, dma_ret);

    AF_TRACE_DEBUG();
    if(0)
    {
    }
#ifdef AF_DEVICE_EXT_CODEC
    else if(device == AUD_STREAM_USE_EXT_CODEC)
    {
        AF_TRACE_DEBUG();
        tlv32aic32_stream_start(stream);
    }
#endif
#ifdef AF_DEVICE_I2S
    else if(device == AUD_STREAM_USE_I2S0_MASTER || device == AUD_STREAM_USE_I2S0_SLAVE)
    {
        hal_i2s_start_stream(HAL_I2S_ID_0, stream);
    }
#if defined(CHIP_HAS_I2S) && (CHIP_HAS_I2S > 1)
    else if(device == AUD_STREAM_USE_I2S1_MASTER || device == AUD_STREAM_USE_I2S1_SLAVE)
    {
        hal_i2s_start_stream(HAL_I2S_ID_1, stream);
    }
#endif
#endif
#ifdef AF_DEVICE_TDM
    else if(device == AUD_STREAM_USE_TDM0_MASTER || device == AUD_STREAM_USE_TDM0_SLAVE)
    {
        hal_tdm_start_stream(HAL_I2S_ID_0, stream);
    }
#if defined(CHIP_HAS_I2S) && (CHIP_HAS_I2S > 1)
    else if(device == AUD_STREAM_USE_TDM1_MASTER || device == AUD_STREAM_USE_TDM1_SLAVE)
    {
        hal_tdm_start_stream(HAL_I2S_ID_1, stream);
    }
#endif
#endif
#ifdef AF_DEVICE_INT_CODEC
    else if(device == AUD_STREAM_USE_INT_CODEC)
    {
        AF_TRACE_DEBUG();
#ifdef CODEC_PLAY_BEFORE_CAPTURE
        af_codec_stream_pre_start(stream);
#endif
        codec_int_stream_start(stream);
#ifdef CODEC_PLAY_BEFORE_CAPTURE
        af_codec_stream_post_start(stream);
#endif
#ifdef CODEC_ERROR_HANDLING
        af_update_stream_state(id, stream, true);
#endif
    }
#endif
#ifdef AUDIO_OUTPUT_DAC2
    else if(device == AUD_STREAM_USE_INT_CODEC2)
    {
        AF_TRACE_DEBUG();
        codec2_int_stream_start(stream);
#ifdef CODEC_ERROR_HANDLING
        af_update_stream_state(id, stream, true);
#endif
    }
#endif
#ifdef AUDIO_OUTPUT_DAC3
    else if(device == AUD_STREAM_USE_INT_CODEC3)
    {
        AF_TRACE_DEBUG();
        codec3_int_stream_start(stream);
#ifdef CODEC_ERROR_HANDLING
        af_update_stream_state(id, stream, true);
#endif
    }
#endif
#ifdef AF_DEVICE_SPDIF
    else if(device == AUD_STREAM_USE_SPDIF0)
    {
        AF_TRACE_DEBUG();
        hal_spdif_start_stream(HAL_SPDIF_ID_0, stream);
    }
#ifdef SPDIF1_BASE
    else if(device == AUD_STREAM_USE_SPDIF1)
    {
        AF_TRACE_DEBUG();
        hal_spdif_start_stream(HAL_SPDIF_ID_1, stream);
    }
#endif
#endif
#ifdef AF_DEVICE_BT_PCM
    else if(device == AUD_STREAM_USE_BT_PCM)
    {
        AF_TRACE_DEBUG();
        hal_btpcm_start_stream(AF_BTPCM_INST, stream);
    }
#endif
#ifdef AF_DEVICE_DPD_RX
    else if(device == AUD_STREAM_USE_DPD_RX)
    {
        AF_TRACE_DEBUG();
        hal_btpcm_start_stream(AF_BTPCM_INST, stream);
    }
#endif
#ifdef AUDIO_ANC_FB_MC
    else if(device == AUD_STREAM_USE_MC)
    {
        AF_TRACE_DEBUG();
    }
#endif
    else
    {
        ASSERT(0, "[%s] ERROR: device %d is not defined!", __func__, device);
    }

    AF_TRACE_DEBUG();
    af_set_status(id, stream, AF_STATUS_STREAM_START_STOP);
#ifdef DMA_RPC_CLI
    if (dma_rpcif_enabled(id, stream))
    {
        dma_rpc_sync_data((void *)af_stream, sizeof(af_stream));
        dma_rpc_stream_start(id, stream);
    }
#endif

    ret = AF_RES_SUCCESS;

_exit:
    af_unlock_thread();

    return ret;
}

uint32_t af_stream_stop(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    struct af_stream_cfg_t *role;
    enum AF_RESULT_T ret;
    enum AUD_STREAM_USE_DEVICE_T device;

    role = af_get_stream_role(id, stream);
    TRACE(3,"[%s] id = %d, stream = %d", __func__, id, stream);

    af_lock_thread();

    device = role->ctl.use_device;

    if (device == AUD_STREAM_USE_INT_CODEC &&
            role->ctl.status == (AF_STATUS_OPEN_CLOSE | AF_STATUS_STREAM_OPEN_CLOSE | AF_STATUS_STREAM_START_STOP | AF_STATUS_STREAM_PAUSE_RESTART))
    {
        af_clear_status(id, stream, AF_STATUS_STREAM_PAUSE_RESTART);
        goto _pause_stop;
    }

    //check stream is start and not stop
    if (role->ctl.status != (AF_STATUS_OPEN_CLOSE | AF_STATUS_STREAM_OPEN_CLOSE | AF_STATUS_STREAM_START_STOP))
    {
        TRACE(2,"[%s] ERROR: status = %d",__func__, role->ctl.status);
        ret = AF_RES_FAILD;
        goto _exit;
    }

#ifdef CODEC_ERROR_HANDLING
    af_update_stream_state(id, stream, false);
#endif

#if defined(RTOS) && defined(AF_STREAM_ID_0_PLAYBACK_FADEOUT)
    if (((id == AUD_STREAM_ID_0) || (id == AUD_STREAM_ID_3)) && stream == AUD_STREAM_PLAYBACK){
        af_stream_fadeout_stop();
    }
#endif

    if (device == AUD_STREAM_USE_INT_CODEC && stream == AUD_STREAM_PLAYBACK) {
#ifdef AUDIO_OUTPUT_PA_OFF_FADE_OUT
        dac_pa_state = AF_DAC_PA_OFF_TRIGGER;
        af_unlock_thread();
        while (dac_pa_state != AF_DAC_PA_NULL) {
#ifdef RTOS
            osSignalWait((1 << AF_FADE_OUT_SIGNAL_ID), 300);
            osSignalClear(fade_thread_id, (1 << AF_FADE_OUT_SIGNAL_ID));
#else
            af_thread(NULL);
#endif
        }
        af_lock_thread();
#elif defined(AUDIO_OUTPUT_PA_ON_FADE_IN)
        dac_pa_state = AF_DAC_PA_NULL;
        analog_aud_codec_speaker_enable(false);
        dac_pa_stop_time = hal_sys_timer_get();
#endif // !AUDIO_OUTPUT_PA_OFF_FADE_OUT && AUDIO_OUTPUT_PA_ON_FADE_IN
    }

#ifdef DMA_RPC_CLI
    if (dma_rpcif_enabled(id,stream))
    {
        dma_rpc_stop(role->dma_cfg.ch);
    } else
#endif
    {
        hal_audma_stop(role->dma_cfg.ch);
    }

#if defined(RTOS) && defined(AF_STREAM_ID_0_PLAYBACK_FADEOUT)
    if (((id == AUD_STREAM_ID_0) || (id == AUD_STREAM_ID_3)) && stream == AUD_STREAM_PLAYBACK){
        af_stream_fadeout_stop();
    }
#endif

    if(0)
    {
    }
#ifdef AF_DEVICE_EXT_CODEC
    else if(device == AUD_STREAM_USE_EXT_CODEC)
    {
        AF_TRACE_DEBUG();
        tlv32aic32_stream_stop(stream);
    }
#endif
#ifdef AF_DEVICE_I2S
    else if(device == AUD_STREAM_USE_I2S0_MASTER || device == AUD_STREAM_USE_I2S0_SLAVE)
    {
        AF_TRACE_DEBUG();
        hal_i2s_stop_stream(HAL_I2S_ID_0, stream);
    }
#if defined(CHIP_HAS_I2S) && (CHIP_HAS_I2S > 1)
    else if(device == AUD_STREAM_USE_I2S1_MASTER || device == AUD_STREAM_USE_I2S1_SLAVE)
    {
        AF_TRACE_DEBUG();
        hal_i2s_stop_stream(HAL_I2S_ID_1, stream);
    }
#endif
#endif
#ifdef AF_DEVICE_TDM
    else if(device == AUD_STREAM_USE_TDM0_MASTER || device == AUD_STREAM_USE_TDM0_SLAVE)
    {
        AF_TRACE_DEBUG();
        hal_tdm_stop_stream(HAL_I2S_ID_0, stream);
    }
#if defined(CHIP_HAS_I2S) && (CHIP_HAS_I2S > 1)
    else if(device == AUD_STREAM_USE_TDM1_MASTER || device == AUD_STREAM_USE_TDM1_SLAVE)
    {
        AF_TRACE_DEBUG();
        hal_tdm_stop_stream(HAL_I2S_ID_1, stream);
    }
#endif
#endif
#ifdef AF_DEVICE_INT_CODEC
    else if(device == AUD_STREAM_USE_INT_CODEC)
    {
        AF_TRACE_DEBUG();
#ifdef CODEC_PLAY_BEFORE_CAPTURE
        af_codec_stream_pre_stop(stream);
#endif
        codec_int_stream_stop(stream);
#ifdef CODEC_PLAY_BEFORE_CAPTURE
        af_codec_stream_post_stop(stream);
#endif
    }
#endif
#ifdef AUDIO_OUTPUT_DAC2
    else if(device == AUD_STREAM_USE_INT_CODEC2)
    {
        AF_TRACE_DEBUG();
        codec2_int_stream_stop(stream);
    }
#endif
#ifdef AUDIO_OUTPUT_DAC3
    else if(device == AUD_STREAM_USE_INT_CODEC3)
    {
        AF_TRACE_DEBUG();
        codec3_int_stream_stop(stream);
    }
#endif
#ifdef AF_DEVICE_SPDIF
    else if(device == AUD_STREAM_USE_SPDIF0)
    {
        AF_TRACE_DEBUG();
        hal_spdif_stop_stream(HAL_SPDIF_ID_0, stream);
    }
#ifdef SPDIF1_BASE
    else if(device == AUD_STREAM_USE_SPDIF1)
    {
        AF_TRACE_DEBUG();
        hal_spdif_stop_stream(HAL_SPDIF_ID_1, stream);
    }
#endif
#endif
#ifdef AF_DEVICE_BT_PCM
    else if(device == AUD_STREAM_USE_BT_PCM)
    {
        AF_TRACE_DEBUG();
        hal_btpcm_stop_stream(AF_BTPCM_INST, stream);
    }
#endif
#ifdef AF_DEVICE_DPD_RX
    else if(device == AUD_STREAM_USE_DPD_RX)
    {
        AF_TRACE_DEBUG();
        hal_btpcm_stop_stream(AF_BTPCM_INST, stream);
    }
#endif
#ifdef AUDIO_ANC_FB_MC
    else if(device == AUD_STREAM_USE_MC)
    {
        AF_TRACE_DEBUG();
    }
#endif
    else
    {
        ASSERT(0, "[%s] ERROR: device %d is not defined!", __func__, device);
    }

_pause_stop:
    AF_TRACE_DEBUG();
    af_clear_status(id, stream, AF_STATUS_STREAM_START_STOP);

#ifndef RTOS
    af_clear_flag(&af_flag_signal, 1 << (id * 2 + stream));
#endif
#ifdef DMA_RPC_CLI
    if (dma_rpcif_enabled(id, stream))
    {
        dma_rpc_stream_stop(id, stream);
        dma_rpc_sync_data((void *)af_stream, sizeof(af_stream));
    }
#endif

    ret = AF_RES_SUCCESS;

_exit:
    af_unlock_thread();

    return ret;
}

uint32_t af_stream_close(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    struct af_stream_cfg_t *role;
    enum AF_RESULT_T ret;
    enum AUD_STREAM_USE_DEVICE_T device;

    role = af_get_stream_role(id, stream);
    TRACE(3,"[%s] id = %d, stream = %d", __func__, id, stream);

    af_lock_thread();

    //check stream is stop and not close.
    if(role->ctl.status != (AF_STATUS_OPEN_CLOSE | AF_STATUS_STREAM_OPEN_CLOSE))
    {
        TRACE(2,"[%s] ERROR: status = %d",__func__, role->ctl.status);
        ret = AF_RES_FAILD;
        goto _exit;
    }

    device = role->ctl.use_device;

    memset(role->dma_buf_ptr, 0, role->dma_buf_size);
#ifdef DMA_RPC_CLI
    if (dma_rpcif_enabled(id,stream))
    {
        dma_rpc_free_chan(role->dma_cfg.ch);
    } else
#endif
    {
        hal_audma_free_chan(role->dma_cfg.ch);
    }

    //  TODO: more parameter should be set!!!
//    memset(role, 0xff, sizeof(struct af_stream_cfg_t));
    role->handler = NULL;
    role->ctl.pp_index = PP_PING;
    role->ctl.use_device = AUD_STREAM_USE_DEVICE_NULL;
    role->dma_buf_ptr = NULL;
    role->dma_buf_size = 0;

    role->dma_cfg.ch = HAL_DMA_CHAN_NONE;

    if(0)
    {
    }
#ifdef AF_DEVICE_EXT_CODEC
    else if(device == AUD_STREAM_USE_EXT_CODEC)
    {
        AF_TRACE_DEBUG();
        tlv32aic32_stream_close(stream);
    }
#endif
#ifdef AF_DEVICE_I2S
    else if(device == AUD_STREAM_USE_I2S0_MASTER || device == AUD_STREAM_USE_I2S0_SLAVE)
    {
        AF_TRACE_DEBUG();
#ifdef AUDIO_OUTPUT_I2S_SW_GAIN
        if (device == AUD_STREAM_USE_I2S0_MASTER) {
            af_stream_sw_gain_close(id, stream);
        }
#endif
        hal_i2s_close(HAL_I2S_ID_0, stream);
    }
#if defined(CHIP_HAS_I2S) && (CHIP_HAS_I2S > 1)
    else if(device == AUD_STREAM_USE_I2S1_MASTER || device == AUD_STREAM_USE_I2S1_SLAVE)
    {
        AF_TRACE_DEBUG();
#ifdef AUDIO_OUTPUT_I2S_SW_GAIN
        if (device == AUD_STREAM_USE_I2S0_MASTER) {
            af_stream_sw_gain_close(id, stream);
        }
#endif
        hal_i2s_close(HAL_I2S_ID_1, stream);
    }
#endif
#endif
#ifdef AF_DEVICE_TDM
    else if(device == AUD_STREAM_USE_TDM0_MASTER || device == AUD_STREAM_USE_TDM0_SLAVE)
    {
        AF_TRACE_DEBUG();
        hal_tdm_close(HAL_I2S_ID_0, stream);
    }
#if defined(CHIP_HAS_I2S) && (CHIP_HAS_I2S > 1)
    else if(device == AUD_STREAM_USE_TDM1_MASTER || device == AUD_STREAM_USE_TDM1_SLAVE)
    {
        AF_TRACE_DEBUG();
        hal_tdm_close(HAL_I2S_ID_1, stream);
    }
#endif
#endif
#ifdef AF_DEVICE_INT_CODEC
    else if(device == AUD_STREAM_USE_INT_CODEC)
    {
        AF_TRACE_DEBUG();
        codec_int_stream_close(stream);
        codec_int_close(CODEC_CLOSE_NORMAL);
    }
#endif
#ifdef AUDIO_OUTPUT_DAC2
    else if(device == AUD_STREAM_USE_INT_CODEC2)
    {
        AF_TRACE_DEBUG();
        codec2_int_stream_close(stream);
        codec2_int_close(CODEC_CLOSE_NORMAL);
    }
#endif
#ifdef AUDIO_OUTPUT_DAC3
    else if(device == AUD_STREAM_USE_INT_CODEC3)
    {
        AF_TRACE_DEBUG();
        codec3_int_stream_close(stream);
        codec3_int_close(CODEC_CLOSE_NORMAL);
    }
#endif
#ifdef AF_DEVICE_SPDIF
    else if(device == AUD_STREAM_USE_SPDIF0)
    {
        AF_TRACE_DEBUG();
        hal_spdif_close(HAL_SPDIF_ID_0, stream);
    }
#ifdef SPDIF1_BASE
    else if(device == AUD_STREAM_USE_SPDIF1)
    {
        AF_TRACE_DEBUG();
        hal_spdif_close(HAL_SPDIF_ID_1, stream);
    }
#endif
#endif
#ifdef AF_DEVICE_BT_PCM
    else if(device == AUD_STREAM_USE_BT_PCM)
    {
        AF_TRACE_DEBUG();
        hal_btpcm_close(AF_BTPCM_INST, stream);
    }
#endif
#ifdef AF_DEVICE_DPD_RX
    else if(device == AUD_STREAM_USE_DPD_RX)
    {
        AF_TRACE_DEBUG();
    }
#endif
#ifdef AUDIO_ANC_FB_MC
    else if(device == AUD_STREAM_USE_MC)
    {
        AF_TRACE_DEBUG();
        hal_codec_mc_disable();
    }
#endif
    else
    {
        ASSERT(0, "[%s] ERROR: device %d is not defined!", __func__, device);
    }

    AF_TRACE_DEBUG();
    af_clear_status(id, stream, AF_STATUS_STREAM_OPEN_CLOSE);

#ifndef RTOS
    af_clear_flag(&af_flag_open, 1 << (id * 2 + stream));
    if (af_flag_open == 0) {
        hal_cpu_wake_unlock(AF_CPU_WAKE_USER);
    }
#endif
#ifdef DMA_RPC_CLI
    if (dma_rpcif_enabled(id, stream))
    {
        dma_rpc_stream_close(id, stream);
    }
#endif

    ret = AF_RES_SUCCESS;

_exit:
    af_unlock_thread();

    return ret;
}

uint32_t af_close(void)
{
    struct af_stream_cfg_t *role;

    // Avoid blocking shutdown process
    //af_lock_thread();

    for (uint8_t id=0; id < AUD_STREAM_ID_NUM; id++)
    {
        for (uint8_t stream=0; stream < AUD_STREAM_NUM; stream++)
        {
            role = af_get_stream_role((enum AUD_STREAM_ID_T)id, (enum AUD_STREAM_T)stream);
            role->ctl.status = AF_STATUS_NULL;
        }
    }

#ifdef AF_DEVICE_INT_CODEC
    codec_int_close(CODEC_CLOSE_FORCED);
#endif

    //af_unlock_thread();

    return AF_RES_SUCCESS;
}

static uint32_t _get_cur_dma_virtual_src_addr(const struct af_stream_cfg_t *role)
{
    uint32_t remain;
    uint32_t src;
    uint32_t desc_cnt;
    uint32_t desc_size;
    uint8_t idx;
    uint32_t offset;
    uint32_t addr_offset;

    hal_dma_get_cur_src_remain_and_addr(role->dma_cfg.ch, &remain, &src);
    if (role->dma_cfg.src_width == HAL_DMA_WIDTH_HALFWORD) {
        remain *= 2;
    } else if (role->dma_cfg.src_width == HAL_DMA_WIDTH_WORD) {
        remain *= 4;
    }
    if (src < (uint32_t)role->dma_buf_ptr || src >= ((uint32_t)role->dma_buf_ptr + role->dma_buf_size)) {
        return 0;
    }
    desc_cnt = AUDIO_BUFFER_COUNT;
    if (desc_cnt == 0) {
        return 0;
    }
    desc_size = role->dma_buf_size / desc_cnt;
    if (desc_size < remain) {
        return 0;
    }
    offset = desc_size - remain;
    idx = (src - (uint32_t)role->dma_buf_ptr) / desc_size;
    addr_offset = (src - (uint32_t)role->dma_buf_ptr) % desc_size;
    if (offset > addr_offset) {
        if (idx) {
            idx--;
        } else {
            idx = desc_cnt - 1;
        }
    }

    return ((uint32_t)role->dma_buf_ptr + desc_size * idx + offset);
}

uint32_t _get_cur_dma_addr(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, bool virtual)
{
    struct af_stream_cfg_t *role;
    int i;
    uint32_t addr = 0;

    role = af_get_stream_role(id, stream);

    //check stream is start and not stop
    if (role->ctl.status == (AF_STATUS_OPEN_CLOSE | AF_STATUS_STREAM_OPEN_CLOSE | AF_STATUS_STREAM_START_STOP)) {
        if (role->dma_cfg.ch != HAL_DMA_CHAN_NONE) {
            for (i = 0; i < 2; i++) {
                if (stream == AUD_STREAM_PLAYBACK) {
                    if (virtual) {
                        addr = _get_cur_dma_virtual_src_addr(role);
                    } else {
                        addr = hal_audma_get_cur_src_addr(role->dma_cfg.ch);
                    }
                } else {
                    addr = hal_audma_get_cur_dst_addr(role->dma_cfg.ch);
                }
                if (addr) {
                    break;
                }
                // Previous link list item was just finished. Read current DMA address again.
            }
        }
    }

    return addr;
}

int _get_cur_dma_pos(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, bool virtual)
{
    struct af_stream_cfg_t *role;
    uint32_t addr;
    int pos;

    role = af_get_stream_role(id, stream);

    addr = _get_cur_dma_addr(id, stream, virtual);

    pos = addr - (uint32_t)role->dma_buf_ptr;

    if (pos < 0 || pos > role->dma_buf_size) {
        return -1;
    }

#ifndef CHIP_BEST1000
    if (role->cfg.chan_sep_buf && role->cfg.channel_num > AUD_CHANNEL_NUM_1) {
        uint32_t chan_size;
        uint8_t chan_idx;
        uint8_t desc_idx;
        uint16_t chan_desc_offset;
        uint16_t chan_desc_xfer_size;
        uint32_t chan_offset;

        chan_size = role->dma_buf_size / role->cfg.channel_num;
        chan_desc_xfer_size = chan_size / AUDIO_BUFFER_COUNT;

        chan_idx = pos / chan_size;
        chan_offset = pos % chan_size;
        desc_idx = chan_offset / chan_desc_xfer_size;
        chan_desc_offset = chan_offset % chan_desc_xfer_size;

        pos = desc_idx * (role->dma_buf_size / AUDIO_BUFFER_COUNT) + chan_idx * chan_desc_xfer_size + chan_desc_offset;
    }
#endif

    return pos;
}

uint32_t af_stream_get_cur_dma_addr(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    return _get_cur_dma_addr(id, stream, false);
}

int af_stream_get_cur_dma_pos(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    return _get_cur_dma_pos(id, stream, false);
}

uint32_t af_stream_get_cur_dma_virtual_addr(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    return _get_cur_dma_addr(id, stream, true);
}

int af_stream_get_cur_dma_virtual_pos(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    return _get_cur_dma_pos(id, stream, true);
}

int af_stream_buffer_error(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    return af_sig_lost_cnt[id][stream];
}

uint32_t af_stream_dma_tc_irq_enable(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    struct af_stream_cfg_t *role;

    role = af_get_stream_role(id, stream);

    // Check if opened
    if ((role->ctl.status & (AF_STATUS_OPEN_CLOSE | AF_STATUS_STREAM_OPEN_CLOSE)) ==
            (AF_STATUS_OPEN_CLOSE | AF_STATUS_STREAM_OPEN_CLOSE)) {
        hal_dma_tc_irq_enable(role->dma_cfg.ch);
        return AF_RES_SUCCESS;
    }

    return AF_RES_FAILD;
}

uint32_t af_stream_dma_tc_irq_disable(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    struct af_stream_cfg_t *role;

    role = af_get_stream_role(id, stream);

    // Check if opened
    if ((role->ctl.status & (AF_STATUS_OPEN_CLOSE | AF_STATUS_STREAM_OPEN_CLOSE)) ==
            (AF_STATUS_OPEN_CLOSE | AF_STATUS_STREAM_OPEN_CLOSE)) {
        hal_dma_tc_irq_disable(role->dma_cfg.ch);
        return AF_RES_SUCCESS;
    }

    return AF_RES_FAILD;
}

uint32_t af_stream_get_dma_base_addr(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    uint32_t addr = 0;
    struct af_stream_cfg_t *role;

    role = af_get_stream_role(id, stream);

    // Check if opened
    if ((role->ctl.status & (AF_STATUS_OPEN_CLOSE | AF_STATUS_STREAM_OPEN_CLOSE)) ==
            (AF_STATUS_OPEN_CLOSE | AF_STATUS_STREAM_OPEN_CLOSE)) {
        addr = role->dma_base;
    }
    return addr;
}

uint8_t af_stream_get_dma_chan(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    struct af_stream_cfg_t *role;

    role = af_get_stream_role(id, stream);

    // Check if opened
    if ((role->ctl.status & (AF_STATUS_OPEN_CLOSE | AF_STATUS_STREAM_OPEN_CLOSE)) ==
            (AF_STATUS_OPEN_CLOSE | AF_STATUS_STREAM_OPEN_CLOSE)) {
        return role->dma_cfg.ch;
    }

    return HAL_DMA_CHAN_NONE;
}

void af_stream_tune(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, float ratio)
{
    af_codec_tune(stream, ratio);
}

void af_set_irq_notification(AF_IRQ_NOTIFICATION_T notif)
{
    irq_notif = notif;
}

#ifdef RTOS
static int af_priority[AF_USER_NUM];

void af_set_priority(AF_USER_E user, int priority)
{
    uint8_t i = 0;
    int max_priority = 0;
    af_priority[user] = priority;

    for (i=0; i<AF_USER_NUM; i++)
    {
        if (max_priority < af_priority[i])
        {
            max_priority = af_priority[i];
        }
    }

    if (max_priority != af_get_priority())
    {
        osThreadSetPriority(af_thread_tid, max_priority);
    }
}

int af_get_priority(void)
{
    return (int)osThreadGetPriority(af_thread_tid);
}

int af_get_default_priority(void)
{
    return af_default_priority;
}

void af_reset_priority(void)
{
    osThreadSetPriority(af_thread_tid, af_default_priority);
}
#endif

void af_codec_tune_resample_rate(enum AUD_STREAM_T stream, float ratio)
{
    af_lock_thread();

    af_codec_direct_tune_resample_rate(stream, ratio);

    af_unlock_thread();
}

void af_codec_direct_tune_resample_rate(enum AUD_STREAM_T stream, float ratio)
{
    ASSERT(stream < AUD_STREAM_NUM, "[%s] Bad stream=%d", __func__, stream);

    hal_codec_tune_resample_rate(stream, ratio);
}

void af_codec_tune_both_resample_rate(float ratio)
{
    af_lock_thread();

    hal_codec_tune_both_resample_rate(ratio);

    af_unlock_thread();
}

void af_codec_tune_pll(float ratio)
{
    af_lock_thread();

    analog_aud_pll_tune(ratio);

    af_unlock_thread();
}

void af_codec_tune_xtal(float ratio)
{
    af_lock_thread();

    analog_aud_xtal_tune(ratio);

    af_unlock_thread();
}

void af_codec_tune(enum AUD_STREAM_T stream, float ratio)
{
    af_lock_thread();

    af_codec_direct_tune(stream, ratio);

    af_unlock_thread();
}

void af_codec_direct_tune(enum AUD_STREAM_T stream, float ratio)
{
    ASSERT(stream <= AUD_STREAM_NUM, "[%s] Bad stream=%d", __func__, stream);

#ifdef __AUDIO_RESAMPLE__
    if (hal_cmu_get_audio_resample_status()) {
        if (stream < AUD_STREAM_NUM) {
            hal_codec_tune_resample_rate(stream, ratio);
        } else {
            hal_codec_tune_both_resample_rate(ratio);
        }
    } else
#endif
    {
        analog_aud_pll_tune(ratio);
    }
}

#ifdef AUDIO_OUTPUT_DAC2
void af_codec_tune_dac2_resample_rate(enum AUD_STREAM_T stream, float ratio)
{
    af_lock_thread();

    af_codec_direct_tune_dac2_resample_rate(stream, ratio);

    af_unlock_thread();
}

void af_codec_direct_tune_dac2_resample_rate(enum AUD_STREAM_T stream, float ratio)
{
    ASSERT(stream == AUD_STREAM_PLAYBACK, "[%s] Bad stream=%d", __func__, stream);

    hal_codec_tune_dac2_resample_rate(stream, ratio);
}

void af_codec_tune_dac2(enum AUD_STREAM_T stream, float ratio)
{
    af_lock_thread();

    af_codec_direct_tune_dac2(stream, ratio);

    af_unlock_thread();
}

void af_codec_direct_tune_dac2(enum AUD_STREAM_T stream, float ratio)
{
    ASSERT(stream == AUD_STREAM_PLAYBACK, "[%s] Bad stream=%d", __func__, stream);

#ifdef __AUDIO_RESAMPLE__
    if (hal_cmu_get_audio_resample_status()) {
        hal_codec_tune_dac2_resample_rate(stream, ratio);
    } else
#endif
    {
        analog_aud_pll_tune(ratio);
    }
}
#endif

void af_codec_set_perf_test_power(int type)
{
    af_lock_thread();

    hal_codec_dac_gain_m60db_check((enum HAL_CODEC_PERF_TEST_POWER_T)type);

    af_unlock_thread();
}

void af_codec_set_noise_reduction(bool enable)
{
    af_lock_thread();

    hal_codec_set_noise_reduction(enable);

    af_unlock_thread();
}

void af_codec_bt_trigger_config(bool en, AF_CODEC_BT_TRIGGER_CALLBACK callback)
{
    if (en)
    {
        hal_codec_set_bt_trigger_callback(callback);
        hal_codec_bt_trigger_start();
    }
    else
    {
        hal_codec_bt_trigger_stop();
    }
}

void af_codec_set_bt_sync_source(enum AUD_STREAM_T stream, uint32_t src)
{
#ifndef CHIP_BEST1000
    af_lock_thread();
    if (stream == AUD_STREAM_PLAYBACK) {
        hal_codec_set_dac_bt_sync_source(src);
    } else {
        hal_codec_set_adc_bt_sync_source(src);
    }
    af_unlock_thread();
#endif
}

void af_codec_sync_config(enum AUD_STREAM_T stream, enum AF_CODEC_SYNC_TYPE_T type, bool enable)
{
#ifndef CHIP_BEST1000
    af_lock_thread();

    if (stream == AUD_STREAM_PLAYBACK) {
        if (enable) {
            hal_codec_sync_dac_enable((enum HAL_CODEC_SYNC_TYPE_T)type);
        } else {
            hal_codec_sync_dac_disable();
        }
    } else {
        if (enable) {
            hal_codec_sync_adc_enable((enum HAL_CODEC_SYNC_TYPE_T)type);
        } else {
            hal_codec_sync_adc_disable();
        }
    }

    af_unlock_thread();
#endif
}

void af_codec_sync_resample_rate_config(enum AUD_STREAM_T stream, enum AF_CODEC_SYNC_TYPE_T type, bool enable)
{
#ifndef CHIP_BEST1000
    af_lock_thread();

    if (stream == AUD_STREAM_PLAYBACK) {
        if (enable) {
            hal_codec_sync_dac_resample_rate_enable((enum HAL_CODEC_SYNC_TYPE_T)type);
        } else {
            hal_codec_sync_dac_resample_rate_disable();
        }
    } else {
        if (enable) {
            hal_codec_sync_adc_resample_rate_enable((enum HAL_CODEC_SYNC_TYPE_T)type);
        } else {
            hal_codec_sync_adc_resample_rate_disable();
        }
    }

    af_unlock_thread();
#endif
}

void af_codec_sync_gain_config(enum AUD_STREAM_T stream, enum AF_CODEC_SYNC_TYPE_T type, bool enable)
{
#ifndef CHIP_BEST1000
    af_lock_thread();

    if (stream == AUD_STREAM_PLAYBACK) {
        if (enable) {
            hal_codec_sync_dac_gain_enable((enum HAL_CODEC_SYNC_TYPE_T)type);
        } else {
            hal_codec_sync_dac_gain_disable();
        }
    } else {
        if (enable) {
            hal_codec_sync_adc_gain_enable((enum HAL_CODEC_SYNC_TYPE_T)type);
        } else {
            hal_codec_sync_adc_gain_disable();
        }
    }

    af_unlock_thread();
#endif
}

#ifdef AUDIO_OUTPUT_DAC2
void af_codec_set_dac2_bt_sync_source(enum AUD_STREAM_T stream, uint32_t src)
{
    af_lock_thread();

    if (stream == AUD_STREAM_PLAYBACK) {
        hal_codec_set_dac2_bt_sync_source(src);
    }

    af_unlock_thread();
}

void af_codec_sync_dac2_config(enum AUD_STREAM_T stream, enum AF_CODEC_SYNC_TYPE_T type, bool enable)
{
    af_lock_thread();

    if (stream == AUD_STREAM_PLAYBACK) {
        if (enable) {
            hal_codec_sync_dac2_enable((enum HAL_CODEC_SYNC_TYPE_T)type);
        } else {
            hal_codec_sync_dac2_disable();
        }
    }

    af_unlock_thread();
}

void af_codec_sync_dac2_resample_rate_config(enum AUD_STREAM_T stream, enum AF_CODEC_SYNC_TYPE_T type, bool enable)
{
    af_lock_thread();

    if (stream == AUD_STREAM_PLAYBACK) {
        if (enable) {
            hal_codec_sync_dac2_resample_rate_enable((enum HAL_CODEC_SYNC_TYPE_T)type);
        } else {
            hal_codec_sync_dac2_resample_rate_disable();
        }
    }

    af_unlock_thread();
}

void af_codec_sync_dac2_gain_config(enum AUD_STREAM_T stream, enum AF_CODEC_SYNC_TYPE_T type, bool enable)
{
    af_lock_thread();

    if (stream == AUD_STREAM_PLAYBACK) {
        if (enable) {
            hal_codec_sync_dac2_gain_enable((enum HAL_CODEC_SYNC_TYPE_T)type);
        } else {
            hal_codec_sync_dac2_gain_disable();
        }
    }

    af_unlock_thread();
}
#endif

void af_codec_set_device_bt_sync_source(enum AUD_STREAM_USE_DEVICE_T device,
                                        enum AUD_STREAM_T stream, uint32_t src)
{
    if (device == AUD_STREAM_USE_INT_CODEC) {
        af_codec_set_bt_sync_source(stream, src);
    }
#ifdef AUDIO_OUTPUT_DAC2
    else if (device == AUD_STREAM_USE_INT_CODEC2) {
        af_codec_set_dac2_bt_sync_source(stream, src);
    }
#endif
}

void af_codec_sync_device_config(enum AUD_STREAM_USE_DEVICE_T device,
                enum AUD_STREAM_T stream, enum AF_CODEC_SYNC_TYPE_T type, bool enable)
{
    if (device == AUD_STREAM_USE_INT_CODEC) {
        af_codec_sync_config(stream, type, enable);
    }
#ifdef AUDIO_OUTPUT_DAC2
    else if (device == AUD_STREAM_USE_INT_CODEC2) {
        af_codec_sync_dac2_config(stream, type, enable);
    }
#endif
}

void af_codec_swap_output(bool swap)
{
    af_lock_thread();

    hal_codec_swap_output(swap);

    af_unlock_thread();
}

void af_codec_set_playback_post_handler(AF_CODEC_PLAYBACK_POST_HANDLER_T hdlr)
{
    codec_play_post_hdlr[AF_CODEC_PLAY_HDLR_ID_0] = hdlr;
}

#ifdef AUDIO_OUTPUT_DAC2
void af_codec_set_playback2_post_handler(AF_CODEC_PLAYBACK_POST_HANDLER_T hdlr)
{
    codec_play_post_hdlr[AF_CODEC_PLAY_HDLR_ID_1] = hdlr;
}
#endif

void af_codec_min_phase_mode_enable(enum AUD_STREAM_T stream)
{
    af_lock_thread();

    hal_codec_min_phase_mode_enable(stream);

    af_unlock_thread();
}

void af_codec_min_phase_mode_disable(enum AUD_STREAM_T stream)
{
    af_lock_thread();

    hal_codec_min_phase_mode_disable(stream);

    af_unlock_thread();
}

int af_anc_open(enum ANC_TYPE_T type, enum AUD_SAMPRATE_T play_rate,
                    enum AUD_SAMPRATE_T capture_rate, AF_ANC_HANDLER handler)
{
    FUNC_ENTRY_TRACE();

    af_lock_thread();

#ifdef AF_DEVICE_INT_CODEC
    codec_anc_open(type, play_rate, capture_rate, handler);
#endif

    af_unlock_thread();

    return AF_RES_SUCCESS;
}

int af_anc_close(enum ANC_TYPE_T type)
{
    FUNC_ENTRY_TRACE();

    af_lock_thread();

#ifdef AF_DEVICE_INT_CODEC
    codec_anc_close(type);
#endif

    af_unlock_thread();

    return AF_RES_SUCCESS;
}

#ifdef VOICE_DETECTOR_EN
int af_vad_open_simplified(bool init, struct AUD_VAD_SIMP_CFG_T *cfg)
{
    struct AUD_VAD_CONFIG_T c;

    c.type        = cfg->type;
    c.sample_rate = cfg->sample_rate;
    c.bits        = cfg->bits;
    c.handler     = cfg->handler;
    c.adc_gain    = cfg->adc_gain;
    c.frame_len   = cfg->frame_len;
    c.sth         = cfg->sound_thresh;
    c.mvad        = cfg->frame_thresh;
    c.psd_th[0]   = VAD_DEF_PSD_TH0;
    c.psd_th[1]   = VAD_DEF_PSD_TH1;
    c.udc         = VAD_DEF_UDC;
    c.upre        = VAD_DEF_UPRE;
    c.dig_mode    = VAD_DEF_DIG_MODE;
    c.pre_gain    = VAD_DEF_PRE_GAIN;
    c.dc_bypass   = VAD_DEF_DC_BYPASS;
    c.frame_th[0] = VAD_DEF_FRAME_TH0;
    c.frame_th[1] = VAD_DEF_FRAME_TH0;
    c.frame_th[2] = VAD_DEF_FRAME_TH0;
    c.range[0]    = VAD_DEF_RANGE0;
    c.range[1]    = VAD_DEF_RANGE1;
    c.range[2]    = VAD_DEF_RANGE2;
    c.range[3]    = VAD_DEF_RANGE3;

    return af_vad_open(&c);
}

int af_vad_open(const struct AUD_VAD_CONFIG_T *cfg)
{
    FUNC_ENTRY_TRACE();

    af_lock_thread();

    codec_vad_open(cfg);

    af_unlock_thread();

    return AF_RES_SUCCESS;
}

int af_vad_close(void)
{
    FUNC_ENTRY_TRACE();

    af_lock_thread();

    codec_vad_close();

    af_unlock_thread();

    return AF_RES_SUCCESS;
}

int af_vad_start(void)
{
    FUNC_ENTRY_TRACE();

    af_lock_thread();

    hal_codec_vad_start();

    af_unlock_thread();

    return AF_RES_SUCCESS;
}

int af_vad_stop(void)
{
    FUNC_ENTRY_TRACE();

    af_lock_thread();

    hal_codec_vad_stop();

    af_unlock_thread();

    return AF_RES_SUCCESS;
}

uint32_t af_vad_get_data(uint8_t *buf, uint32_t len)
{
    return hal_codec_vad_recv_data(buf, len);
}

void af_vad_get_data_info(struct CODEC_VAD_BUF_INFO_T * vad_buf_info)
{
    hal_codec_get_vad_data_info(vad_buf_info);
}

#endif

#ifdef CODEC_DSD
void af_dsd_enable(void)
{
    enum AUD_STREAM_ID_T id;
    struct af_stream_cfg_t *role;
    bool opened = false;

    af_lock_thread();

    for (id = AUD_STREAM_ID_0; id < AUD_STREAM_ID_NUM; id++) {
        role = af_get_stream_role(id, AUD_STREAM_PLAYBACK);
        if (role->ctl.status & AF_STATUS_STREAM_OPEN_CLOSE) {
            if (role->cfg.device == AUD_STREAM_USE_INT_CODEC && role->dma_cfg.dst_periph == HAL_AUDMA_CODEC_TX) {
                ASSERT(role->ctl.status == (AF_STATUS_OPEN_CLOSE | AF_STATUS_STREAM_OPEN_CLOSE),
                    "Bad stream status when DSD enabled: 0x%X", role->ctl.status);
                role->dma_cfg.dst_periph = HAL_AUDMA_DSD_TX;
                af_stream_update_dma_buffer(id, AUD_STREAM_PLAYBACK, role, &role->cfg);
                opened = true;
                break;
            }
        }
    }

    hal_codec_dsd_enable();
    af_dsd_enabled = true;

    af_unlock_thread();

    if (opened) {
        // Enable DSD sample rate handling
        af_stream_setup(id, AUD_STREAM_PLAYBACK, &role->cfg);
    }
}

void af_dsd_disable(void)
{
    enum AUD_STREAM_ID_T id;
    struct af_stream_cfg_t *role;
    bool opened = false;

    af_lock_thread();

    for (id = AUD_STREAM_ID_0; id < AUD_STREAM_ID_NUM; id++) {
        role = af_get_stream_role(id, AUD_STREAM_PLAYBACK);
        if (role->ctl.status & AF_STATUS_STREAM_OPEN_CLOSE) {
            if (role->cfg.device == AUD_STREAM_USE_INT_CODEC && role->dma_cfg.dst_periph == HAL_AUDMA_DSD_TX) {
                ASSERT(role->ctl.status == (AF_STATUS_OPEN_CLOSE | AF_STATUS_STREAM_OPEN_CLOSE),
                    "Bad stream status when DSD disabled: 0x%X", role->ctl.status);
                role->dma_cfg.dst_periph = HAL_AUDMA_CODEC_TX;
                af_stream_update_dma_buffer(id, AUD_STREAM_PLAYBACK, role, &role->cfg);
                opened = true;
                break;
            }
        }
    }

    hal_codec_dsd_disable();
    af_dsd_enabled = false;

    af_unlock_thread();

    if (opened) {
        // Disable DSD sample rate handling
        af_stream_setup(id, AUD_STREAM_PLAYBACK, &role->cfg);
    }
}
#endif

#if defined(AF_DEVICE_I2S) || defined(AF_DEVICE_TDM)
void af_i2s_sync_config(enum AUD_STREAM_T stream, enum AF_I2S_SYNC_TYPE_T type, bool enable)
{
#if defined(HW_I2S_TDM_TRIGGER)
    af_lock_thread();

    if (enable) {
        hal_i2s_clk_sync_enable((enum HAL_I2S_SYNC_TYPE_T)type);
    } else {
        hal_i2s_clk_sync_disable();
    }

    af_unlock_thread();
#endif
}

#ifdef I2S_RESAMPLE
int af_i2s_resample_enable(enum AUD_STREAM_USE_DEVICE_T dev)
{
    enum HAL_I2S_ID_T id;

    if (dev == AUD_STREAM_USE_I2S0_MASTER) {
        id = HAL_I2S_ID_0;
#if defined(CHIP_HAS_I2S) && (CHIP_HAS_I2S > 1)
    } else if (dev == AUD_STREAM_USE_I2S1_MASTER) {
        id = HAL_I2S_ID_1;
#endif
    } else {
        return 1;
    }
    hal_i2s_resample_enable(id);
    return 0;
}

int af_i2s_resample_disable(enum AUD_STREAM_USE_DEVICE_T dev)
{
    enum HAL_I2S_ID_T id;

    if (dev == AUD_STREAM_USE_I2S0_MASTER) {
        id = HAL_I2S_ID_0;
#if defined(CHIP_HAS_I2S) && (CHIP_HAS_I2S > 1)
    } else if (dev == AUD_STREAM_USE_I2S1_MASTER) {
        id = HAL_I2S_ID_1;
#endif
    } else {
        return 1;
    }
    hal_i2s_resample_disable(id);
    return 0;
}
#endif
#endif

#ifdef AF_DEVICE_TDM
#ifdef I2S_RESAMPLE
int af_tdm_resample_enable(enum AUD_STREAM_USE_DEVICE_T dev)
{
    enum HAL_I2S_ID_T id;

    if (dev == AUD_STREAM_USE_TDM0_MASTER) {
        id = HAL_I2S_ID_0;
#if defined(CHIP_HAS_I2S) && (CHIP_HAS_I2S > 1)
    } else if (dev == AUD_STREAM_USE_TDM1_MASTER) {
        id = HAL_I2S_ID_1;
#endif
    } else {
        return 1;
    }
    hal_i2s_resample_enable(id);
    return 0;
}

int af_tdm_resample_disable(enum AUD_STREAM_USE_DEVICE_T dev)
{
    enum HAL_I2S_ID_T id;

    if (dev == AUD_STREAM_USE_TDM0_MASTER) {
        id = HAL_I2S_ID_0;
#if defined(CHIP_HAS_I2S) && (CHIP_HAS_I2S > 1)
    } else if (dev == AUD_STREAM_USE_TDM1_MASTER) {
        id = HAL_I2S_ID_1;
#endif
    } else {
        return 1;
    }
    hal_i2s_resample_disable(id);
    return 0;
}
#endif
#endif

#ifndef DAC_DC_CALIB_MIC_CHAN_MAP
#define DAC_DC_CALIB_MIC_CHAN_MAP (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1)
//#define DAC_DC_CALIB_MIC_CHAN_MAP (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1)
#endif

#ifdef AUDIO_OUTPUT_DC_AUTO_CALIB
//#define DEBUG_DAC_DC_AUTO_CALIB

#ifdef DEBUG_DAC_DC_AUTO_CALIB
#define DAC_DC_CALIB_GET_DC_CNT         8
#else
#define DAC_DC_CALIB_GET_DC_CNT         2
#endif
#define DAC_DC_CALIB_SAMP_RATE          AUD_SAMPRATE_48000
#ifdef  AUDIO_OUTPUT_SHORT_CALIB_TIME
#define DAC_DC_CALIB_BUF_SAMP_CNT       (DAC_DC_CALIB_SAMP_RATE * 16 / 1000)
#else
#define DAC_DC_CALIB_BUF_SAMP_CNT       (DAC_DC_CALIB_SAMP_RATE * 64 / 1000)
#endif
#define DAC_DC_CALIB_AVG_SAMP_CNT       (DAC_DC_CALIB_BUF_SAMP_CNT / 2 * DAC_DC_CALIB_GET_DC_CNT)

#define DAC_GAIN_CALIB_AUTO_CNT         4
#define DAC_DC_CALIB_AUTO_CNT           100
#define SET_LARGE_ANA_DC_CNT            20

// About +-50mV
#define DAC_DC_CALIB_MAX_ABS            0x50000
// About +- 13 mV
#define ADC_DC_CALIB_MAX_ABS            80000

#define FINAL_DAC_DC_CALIB_MAX_ABS      (0x3FFFF)

//#define AUDIO_OUTPUT_DEBUG_ANA_ADC

#ifdef DEBUG_DAC_DC_AUTO_CALIB
static volatile uint32_t dc_calib_loop_cnt;
#endif
static volatile int64_t dc_calib_sum_l;
static volatile int64_t dc_calib_sum_r;
static volatile uint32_t dc_calib_samp_cnt;
static volatile bool dc_calib_first_irq;

static uint32_t codec_dac_calc_dc_handler(uint8_t *buf, uint32_t len)
{
    int64_t dc_l, dc_r;
    uint32_t i;
    int32_t *samp;
    uint32_t samp_cnt;
//    uint32_t time = TICKS_TO_MS(hal_sys_timer_get());

//    TRACE(1, "[%d] CODEC_CAP: %s, buf=%x, len=%d", time, __func__, (int)buf, len);

    if (dc_calib_first_irq) {
        dc_calib_first_irq = false;
        return 0;
    }

    samp = (int32_t *)buf;
    samp_cnt = len / 4 / 2;

#ifdef DEBUG_DAC_DC_AUTO_CALIB
    int32_t min_l, max_l, min_r, max_r;
    int32_t val;

    min_l = samp[0];
    max_l = min_l;
    min_r = samp[1];
    max_r = min_r;
    for (i = 1; i < samp_cnt; i++) {
        val = samp[i * 2];
        if (min_l > val) {
            min_l = val;
        } else if (max_l < val) {
            max_l = val;
        }
        val = samp[i * 2 + 1];
        if (min_r > val) {
            min_r = val;
        } else if (max_r < val) {
            max_r = val;
        }
    }
#endif

    dc_l = 0;
    dc_r = 0;
    for (i = 0; i < samp_cnt; i++) {
        dc_l += (int64_t)samp[i * 2];
        dc_r += (int64_t)samp[i * 2 + 1];
    }

    dc_calib_sum_l += (int64_t)dc_l;
    dc_calib_sum_r += (int64_t)dc_r;
    dc_calib_samp_cnt += i;

#ifdef DEBUG_DAC_DC_AUTO_CALIB
    dc_calib_loop_cnt++;
    if (dc_calib_loop_cnt > DAC_DC_CALIB_GET_DC_CNT) {
        return 0;
    }

    dc_l /= (int)i;
    dc_r /= (int)i;

    TRACE(1, "[%4u] DC_L=%6d (%6d/%6d) DC_R=%6d (%6d/%6d)", dc_calib_loop_cnt, dc_l, min_l, max_l, dc_r, min_r, max_r);
#endif

    return 0;
}

static void get_codec_dac_dc(int32_t *dc_l, int32_t *dc_r)
{
    uint32_t lock;

    lock = int_lock();
    dc_calib_first_irq = true;
    dc_calib_sum_l = 0;
    dc_calib_sum_r = 0;
    dc_calib_samp_cnt = 0;
#ifdef DEBUG_DAC_DC_AUTO_CALIB
    dc_calib_loop_cnt = 0;
#endif
    int_unlock(lock);

    while (dc_calib_samp_cnt < DAC_DC_CALIB_AVG_SAMP_CNT
#ifdef DEBUG_DAC_DC_AUTO_CALIB
            || dc_calib_loop_cnt < DAC_DC_CALIB_GET_DC_CNT
#endif
            ) {
#ifdef RTOS
        osStatus_t r;
        r = osDelay(2);
        if (r != osOK) {
            TRACE(1, "osDelay failed %d", (int)r);
        } else {
//            TRACE(1, "dc_calib_samp_cnt=%d", dc_calib_samp_cnt);
        }
#else
        af_thread(NULL);
#endif
    }

    lock = int_lock();
    *dc_l = (int32_t)(dc_calib_sum_l / (int64_t)dc_calib_samp_cnt);
    *dc_r = (int32_t)(dc_calib_sum_r / (int64_t)dc_calib_samp_cnt);
    int_unlock(lock);

    TRACE(1, "Dac dc_l=%d dc_r=%d cnt=%u", *dc_l, *dc_r, dc_calib_samp_cnt);
}
#endif

static volatile uint8_t cal_dac_ana_gain = 0x11;
static volatile uint8_t cal_dac_ini_ana_gain = 0;
static volatile uint8_t cal_dac_gain_offset = 2;
static volatile uint8_t cal_dac_chan_enable = 3;
static volatile uint16_t cal_dac_ana_dc_l = 0;
static volatile uint16_t cal_dac_ana_dc_r = 0;
static volatile int32_t cal_dac_dig_dc_l = 0;
static volatile int32_t cal_dac_dig_dc_r = 0;
static volatile int32_t cal_dac_out_dc_l = 0;
static volatile bool cal_dac_out_enable = false;
static volatile bool cal_stream_is_opened = false;

void af_codec_calib_dac_chan_enable(bool en_l, bool en_r)
{
    if (en_l)
        cal_dac_chan_enable |= (1<<0);
    else
        cal_dac_chan_enable &= ~(1<<0);

    if (en_r)
        cal_dac_chan_enable |= (1<<1);
    else
        cal_dac_chan_enable &= ~(1<<1);
}

void af_codec_calib_param_setup(enum AF_CODEC_CALIB_PARAM_T type, uint32_t p0, uint32_t p1, uint32_t p2)
{
    switch (type) {
    case DAC_PARAM_ANA_GAIN:
        cal_dac_ana_gain     = (uint8_t)p0;
        cal_dac_ini_ana_gain = (uint8_t)p1;
        cal_dac_gain_offset  = (uint8_t)p2;
        break;
    case DAC_PARAM_ANA_DC:
        cal_dac_ana_dc_l = (uint16_t)p0;
        cal_dac_ana_dc_r = (uint16_t)p1;
        break;
    case DAC_PARAM_DIG_DC:
        cal_dac_dig_dc_l = (int32_t)p0;
        cal_dac_dig_dc_r = (int32_t)p1;
        break;
    case DAC_PARAM_OUT_DC:
        cal_dac_out_dc_l = (int32_t)p0;
        break;
    default:
        break;
    }
}

#ifdef AUDIO_OUTPUT_DC_AUTO_CALIB
static uint32_t codec_dac_output_dc_handler(uint8_t *buf, uint32_t len)
{
    if (cal_dac_out_enable) {
        int16_t *p = (int16_t *)buf;
        uint32_t n = len / sizeof(int16_t), i;
        int16_t outval = (int16_t)cal_dac_out_dc_l;

        for (i = 0; i < n; i++) {
            p[i] = outval;
        }
//        TRACE("DC OUTPUT: 0x%x, len=%d", outval, len);
    }
    return 0;
}
#endif

#ifdef AUDIO_OUTPUT_DIG_DC_DEEP_CALIB
int codec_calib_dac_dig_dc(uint32_t ch_l_en, uint32_t ch_r_en,
    int32_t tgt_dc_l, int32_t tgt_dc_r,
    uint32_t *comp_dc_l, uint32_t *comp_dc_r)
{
#define DC_CALIB_DEBUG
#define DC_DET_GATE    (10)
#define DC_DET_GATE_ERR (100)
#define DC_REG_MSB_POS (18)
#define DC_REG_EXT(v)  (((v)<<(31-(DC_REG_MSB_POS)))>>(31-(DC_REG_MSB_POS)))
    int i;
    int success = 0;
    int ch_l_done = 0, ch_r_done = 0;
    int32_t dc_l = 0, dc_r = 0;
    int32_t det_l = 0, det_r = 0;
    int32_t regval_l = 0, regval_r = 0;
    uint32_t comp_val_l = 0, comp_val_r = 0;
    uint32_t det_gate_l = DC_DET_GATE;
    uint32_t det_gate_r = DC_DET_GATE;

    if ((!ch_l_en) && (!ch_r_en)) {
        return 0;
    }
    TRACE(1, "CALIB: TGT_DC: tgt_dc_l=%d, tgt_dc_r=%d", tgt_dc_l, tgt_dc_r);

    analog_aud_dac_dc_auto_calib_set_mode(ANA_DAC_DC_CALIB_MODE_DAC_TO_ADC);
    osDelay(10);

    get_codec_dac_dc(&dc_l, &dc_r);
    det_l = tgt_dc_l - dc_l;
    det_r = tgt_dc_r - dc_r;

    for (i = DC_REG_MSB_POS; i >= 0;) {
#ifdef DC_CALIB_DEBUG
        TRACE(1, "[%d]:dc_l=%d,tgt_dc_l=%d,det_l=%d",i,dc_l,tgt_dc_l,det_l);
        TRACE(1, "[%d]:dc_r=%d,tgt_dc_r=%d,det_r=%d",i,dc_r,tgt_dc_r,det_r);
#endif
        if (ch_l_en) {
            if (!ch_l_done) {
                if (ABS(det_l) < det_gate_l) {
                    ch_l_done = 1;
                    TRACE(1, "[%d]: DONE: comp_val_l=%x", i, comp_val_l);
                }
                if (!ch_l_done) {
                    if (det_l < 0) {
                        comp_val_l |= (1<<i);
                    } else {
                        if (i < DC_REG_MSB_POS) {
                            comp_val_l &= ~(1<<(i+1));
                            comp_val_l |= (1<<i);
                        } else {
                            comp_val_l &= ~(1<<DC_REG_MSB_POS);
                        }
                    }
                    regval_l = (int32_t)comp_val_l;
                    regval_l = DC_REG_EXT(regval_l);
                }
            }
#ifdef DC_CALIB_DEBUG
            TRACE(1, "[%d]:comp_val_l=%x,regval_l=%x",i,comp_val_l,regval_l);
#endif
        }
        if (ch_r_en) {
            if (!ch_r_done) {
                if (ABS(det_r) < det_gate_r) {
                    ch_r_done = 1;
                    TRACE(1, "[%d]: DONE: comp_val_r=%x", i, comp_val_r);
                }
                if (!ch_r_done) {
                    if (det_r < 0) {
                        comp_val_r |= (1<<i);
                    } else {
                        if (i < DC_REG_MSB_POS) {
                            comp_val_r &= ~(1<<(i+1));
                            comp_val_r |= (1<<i);
                        } else {
                            comp_val_r &= ~(1<<DC_REG_MSB_POS);
                        }
                    }
                    regval_r = (int32_t)comp_val_r;
                    regval_r = DC_REG_EXT(regval_r);
                }
            }
#ifdef DC_CALIB_DEBUG
            TRACE(1, "[%d]:comp_val_r=%x,regval_r=%x",i,comp_val_r,regval_r);
#endif
        }
        if (ch_l_en && ch_r_en) {
            if (ch_l_done && ch_r_done) {
                TRACE(1, "DAC CH L/R DC calib done");
                success = 1;
                break;
            }
        } else if (ch_l_en) {
            if (ch_l_done) {
                TRACE(1, "DAC CH L DC calib done");
                success = 1;
                break;
            }
        } else if (ch_r_en) {
            if (ch_r_done) {
                TRACE(1, "DAC CH R DC calib done");
                success = 1;
                break;
            }
        }
        hal_codec_dac_dc_offset_enable(regval_l, regval_r);
        osDelay(5);

        get_codec_dac_dc(&dc_l, &dc_r);
        det_l = dc_l - tgt_dc_l;
        det_r = dc_r - tgt_dc_r;
        i--;
#ifdef DC_CALIB_DEBUG
        TRACE(1,"-----------------------");
#endif
    }
    // DC CALIB CHECK
    if (!success) {
        if (ch_l_en && !ch_l_done) {
            if (ABS(det_l) > DC_DET_GATE_ERR) {
                TRACE(false, " ** L DAC CALIB FAIL det_l=%d **", det_l);
            } else {
                ch_l_done = 1;
            }
        }
        if (ch_r_en && !ch_r_done) {
            if (ABS(det_r) > DC_DET_GATE_ERR) {
                TRACE(false, " ** R DAC CALIB FAIL det_l=%d **", det_r);
            } else {
                ch_r_done = 1;
            }
        }
        success = (ch_l_en ? ch_l_done : 1) && (ch_r_en ? ch_r_done : 1);
        TRACE(1, "DAC DC CALIB CHECK success = %d", success);
    }

    TRACE(1, "FINAL DIG DC: regval_l=%x, regval_r=%x", regval_l, regval_r);
    if (comp_dc_l) {
        *comp_dc_l = regval_l;
    }
    if (comp_dc_r) {
        *comp_dc_r = regval_r;
    }
    return success;
}
#endif /* AUDIO_OUTPUT_DIG_DC_DEEP_CALIB */

#define DAC_DC_CALIB_SPK_CHAN_MAP (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1)

int af_codec_calib_dac_dc(enum AF_CODEC_CALIB_CMD_T calib_cmd,
    struct AF_CODEC_CALIB_CFG_T *cfg)
{
#ifdef AUDIO_OUTPUT_DC_AUTO_CALIB
    const uint32_t dc_avg_samp_cnt = DAC_DC_CALIB_BUF_SAMP_CNT;
    uint32_t ret = 0;
    struct AF_STREAM_CONFIG_T stream_cfg;
    uint8_t *play_buf, *cap_buf;
    uint32_t play_buf_len, cap_buf_len;
    int i;
    int32_t dc_target_l, dc_target_r;
    int32_t dc_l, dc_r;
    int32_t comp_l = 0, comp_r = 0;
    int32_t comp_old_l = 0, comp_old_r = 0;
    uint32_t val_l = 0, val_r = 0;
    int32_t diff_l, diff_r;
    int16_t dc_step_l = 0, dc_step_r = 0;
    uint32_t en_l, en_r;
    uint8_t *buf = cfg->buf;
    uint32_t len = cfg->len;
#ifdef AUDIO_OUTPUT_DC_CALIB_ANA
    int16_t dc_step_old_l = 0, dc_step_old_r = 0;
#else
    uint32_t comp_gate_l = 0, comp_gate_r = 0;
#endif

    uint32_t time = hal_fast_sys_timer_get();

    en_l = (cal_dac_chan_enable >> 0)&0x1;
    en_r = (cal_dac_chan_enable >> 1)&0x1;

    af_codec_calib_cmd = calib_cmd;
    // open stream for dac calibration
    if (calib_cmd == CODEC_CALIB_CMD_OPEN) {
        if (cal_stream_is_opened) {
            TRACE(1, "dac calib stream already opened");
            goto _end;
        }
        TRACE(1, "\nOPEN CALIB STREAM\n");

        play_buf = (uint8_t *)(((uint32_t)buf + 0x3) & ~0x3);
        play_buf_len = DAC_DC_CALIB_SAMP_RATE / 1000 * 2 * 2 * 10; //48K,16bit,2ch,10 frame
        cap_buf = play_buf + play_buf_len;
        cap_buf_len = dc_avg_samp_cnt * 2 * 4; //48K, 24bit, 2ch, 4 frame

        TRACE(1, "playback stream: play_buf=%x, play_buf_len=%d", (int)play_buf, play_buf_len);
        TRACE(1, "capture  stream: cap_buf=%x,  cap_buf_len=%d",  (int)cap_buf,  cap_buf_len);

        ASSERT(cap_buf + cap_buf_len <= buf + len, "buf too small: len=%u but needs %u", len, cap_buf + cap_buf_len - buf);

        for (i = 0; i < (play_buf_len + cap_buf_len) / 4; i++) {
            *((volatile uint32_t *)play_buf + i) = 0;
        }

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.bits        = AUD_BITS_16;
        stream_cfg.channel_num = AUD_CHANNEL_NUM_2;
        stream_cfg.channel_map = DAC_DC_CALIB_SPK_CHAN_MAP;
        stream_cfg.sample_rate = DAC_DC_CALIB_SAMP_RATE;
        stream_cfg.device      = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.vol         = 0;
        stream_cfg.io_path     = AUD_IO_PATH_NULL;
        stream_cfg.handler     = codec_dac_output_dc_handler;
        stream_cfg.data_ptr    = play_buf;
        stream_cfg.data_size   = play_buf_len;
        TRACE(1, "PLAY_BUF:%x, LEN=%d", (uint32_t)play_buf, play_buf_len);

        ret = af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);
        ASSERT(ret == 0, "af_stream_open playback failed: %d", ret);

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.bits        = AUD_BITS_24;
        stream_cfg.channel_num = AUD_CHANNEL_NUM_2;
        stream_cfg.channel_map = DAC_DC_CALIB_MIC_CHAN_MAP;
        stream_cfg.sample_rate = DAC_DC_CALIB_SAMP_RATE;
        stream_cfg.device      = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.vol         = 0;
        stream_cfg.io_path     = AUD_INPUT_PATH_MAINMIC;
        stream_cfg.handler     = codec_dac_calc_dc_handler;
        stream_cfg.data_ptr    = cap_buf;
        stream_cfg.data_size   = cap_buf_len;
        TRACE(1, "CAP_BUF:%x, LEN=%d", (uint32_t)cap_buf, cap_buf_len);

        ret = af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);
        ASSERT(ret == 0, "af_stream_open playback failed: %d", ret);

#if 1
        hal_codec_dac_dc_auto_calib_enable();
        analog_aud_dac_dc_auto_calib_enable();
        osDelay(200);
#endif
        ret = af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        ASSERT(ret == 0, "af_stream_start playback failed: %d", ret);
        ret = af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        ASSERT(ret == 0, "af_stream_start playback failed: %d", ret);

        hal_codec_dac_dc_auto_calib_enable();
        analog_aud_dac_dc_auto_calib_enable();
        osDelay(200);

        cal_stream_is_opened = true;
        goto _end;
    }

    if (calib_cmd == CODEC_CALIB_CMD_CLOSE) {
        TRACE(1, "\nCLOSE CALIB STREAM\n");
        ret = af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        ASSERT(ret == 0, "af_stream_stop capture failed: %d", ret);

        ret = af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        ASSERT(ret == 0, "af_stream_stop playback failed: %d", ret);

        analog_aud_dac_dc_auto_calib_disable();
        hal_codec_dac_dc_auto_calib_disable();

        ret = af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        ASSERT(ret == 0, "af_stream_close capture failed: %d", ret);

        ret = af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        ASSERT(ret == 0, "af_stream_close playback failed: %d", ret);

        codec_int_stream_reset_state(AUD_STREAM_CAPTURE);
        codec_int_stream_reset_state(AUD_STREAM_PLAYBACK);

        TRACE(1, "\nDC auto calib consumes %u ms\n", FAST_TICKS_TO_MS(hal_fast_sys_timer_get() - time));

        cal_stream_is_opened = false;

        af_set_dac_dc_offset();
        goto _end;
    }

    ASSERT(cal_stream_is_opened == true, "calib stream is not opend");

    TRACE(1, "\nADC only\n");
    analog_aud_dac_dc_auto_calib_set_mode(ANA_DAC_DC_CALIB_MODE_ADC_ONLY);
#ifdef AUDIO_OUTPUT_DEBUG_ANA_ADC
    osDelay(3000);
#endif
    osDelay(10);
    get_codec_dac_dc(&dc_target_l, &dc_target_r);
#ifdef AUDIO_OUTPUT_USE_FIXED_DC
    analog_aud_dc_calib_get_fixed_comp_value(cal_dac_ana_gain, &dc_l, &dc_r);
    TRACE(1, "FIXED DC: GAIN=%d, L=%d, R=%d", cal_dac_ana_gain, dc_l, dc_r);
    dc_target_l += dc_l;
    dc_target_r += dc_r;
#endif
    /* check ADC DC value */
    if (calib_cmd == CODEC_CALIB_CMD_DIG_DC) {
#ifdef AUDIO_OUTPUT_ASSERT_LARGE_DC
        if (en_l) {
            ASSERT(ABS(dc_target_l) <= ADC_DC_CALIB_MAX_ABS,
                "ADC_DC_L too large: %d", dc_target_l);
        }
        if (en_r) {
            ASSERT(ABS(dc_target_r) <= ADC_DC_CALIB_MAX_ABS,
                "ADC_DC_R too large: %d", dc_target_r);
        }
#elif defined(AUDIO_OUTPUT_ERROR_LARGE_DC)
        if (en_l) {
            if(ABS(dc_target_l) > ADC_DC_CALIB_MAX_ABS) {
                cfg->state = CODEC_CALIB_STATE_ERR_ADC_DC_L;
                goto _end;
            }
        }
        if (en_r) {
            if(ABS(dc_target_r) > ADC_DC_CALIB_MAX_ABS) {
                cfg->state = CODEC_CALIB_STATE_ERR_ADC_DC_R;
                goto _end;
            }
        }
#endif
    }
    TRACE(1, "\nDAC TO ADC\n");
    analog_aud_dac_dc_auto_calib_set_mode(ANA_DAC_DC_CALIB_MODE_DAC_TO_ADC);
#ifdef AUDIO_OUTPUT_DC_CALIB_ANA
    analog_aud_dc_calib_set_value(0, 0);
    dc_step_l = analog_aud_dac_dc_get_step();
    dc_step_r = dc_step_l;
#else
    dc_step_l = 1;
    dc_step_r = dc_step_l;
    if (cal_dac_ana_gain >= 0xF) {
        comp_gate_l = dc_step_l * 10 / 2;//0.163*10=1.63
        comp_gate_r = dc_step_r * 10 / 2;
    } else {
        comp_gate_l = dc_step_l * 50 ;//0.049*50=2.45
        comp_gate_r = dc_step_r * 50 ;
    }
    TRACE(1, "comp_gate_l=%d, comp_gate_r=%d", comp_gate_l, comp_gate_r);
    hal_codec_set_dac_ana_gain(cal_dac_ini_ana_gain, cal_dac_gain_offset);
    hal_codec_dac_dc_offset_enable(0, 0);
    analog_aud_dc_calib_set_value(0, 0);

    if (calib_cmd == CODEC_CALIB_CMD_SET_LARGE_ANA_DC) {
        int16_t ana_dc_l = 0, ana_dc_r = 0;
        uint16_t ana_reg_val_l = 0, ana_reg_val_r = 0;
#ifdef AUDIO_OUTPUT_SET_LARGE_ANA_DC
        bool ok_l = false, ok_r = false;

        TRACE(1,"\nSET LARGE ANA DC\n");
        TRACE(1,"dc_target_l=%d,dc_target_r=%d", dc_target_l, dc_target_r);
        dc_l = dc_r = 0;
        get_codec_dac_dc(&dc_l, &dc_r);
        TRACE(1,"INIT DAC DC dc_l=%d,dc_r=%d", dc_l, dc_r);
        analog_aud_dc_calib_get_large_ana_dc_value(&ana_dc_l, dc_l, dc_target_l, 0, true);
        analog_aud_dc_calib_get_large_ana_dc_value(&ana_dc_r, dc_r, dc_target_r, 1, true);
        TRACE(1, "ANA_DC init: ana_dc_l=%d,ana_dc_r=%d", ana_dc_l, ana_dc_r);
        ana_reg_val_l = analog_aud_dac_dc_diff_to_val(ana_dc_l);
        ana_reg_val_r = analog_aud_dac_dc_diff_to_val(ana_dc_r);
        analog_aud_dc_calib_set_value(ana_reg_val_l, ana_reg_val_r);
        TRACE(1, "ANA_DC init: ana_reg_val_l=0x%x,ana_reg_val_l=0x%x", ana_reg_val_l, ana_reg_val_l);
        analog_aud_dc_calib_enable(true);
        for (i = 0; i < SET_LARGE_ANA_DC_CNT; i++) {
            osDelay(5);
            get_codec_dac_dc(&dc_l, &dc_r);
            if (en_l && !en_r) {
                ok_l = analog_aud_dc_calib_get_large_ana_dc_value(&ana_dc_l, dc_l, dc_target_l, 0, false);
                if (ok_l) {
                    break;
                }
            } else if (en_r && !en_l) {
                ok_r = analog_aud_dc_calib_get_large_ana_dc_value(&ana_dc_r, dc_r, dc_target_r, 1, false);
                if (ok_r) {
                    break;
                }
            } else if (en_l && en_r) {
                ok_l = analog_aud_dc_calib_get_large_ana_dc_value(&ana_dc_l, dc_l, dc_target_l, 0, false);
                ok_r = analog_aud_dc_calib_get_large_ana_dc_value(&ana_dc_r, dc_r, dc_target_r, 1, false);
                if (ok_l && ok_r) {
                    break;
                }
            }
            TRACE(1, "ANA_DC[%d]: ana_dc_l=%d,ana_dc_r=%d,dc_l=%d,dc_r=%d", i, ana_dc_l, ana_dc_r, dc_l, dc_r);
            ana_reg_val_l = analog_aud_dac_dc_diff_to_val(ana_dc_l);
            ana_reg_val_r = analog_aud_dac_dc_diff_to_val(ana_dc_r);
            analog_aud_dc_calib_set_value(ana_reg_val_l, ana_reg_val_r);
            TRACE(1, "ANA_DC[%d]: ana_reg_val_l=0x%x,ana_reg_val_r=0x%x,dc_l=%d,dc_r=%d", i, ana_reg_val_l, ana_reg_val_r, dc_l, dc_r);
        }
#endif /* AUDIO_OUTPUT_SET_LARGE_ANA_DC */
        ana_reg_val_l = analog_aud_dac_dc_diff_to_val(ana_dc_l);
        ana_reg_val_r = analog_aud_dac_dc_diff_to_val(ana_dc_r);
        cfg->ana_dc_l = ana_reg_val_l;
        cfg->ana_dc_r = ana_reg_val_r;
        TRACE(1, "====>Final ana_dc_l=0x%x, ana_dc_r=0x%x",cfg->ana_dc_l, cfg->ana_dc_r);
        goto _end;
    }
#endif

    TRACE(1, "\nSET ANA DC\n");
    TRACE(1, "cal_dac_ana_dc_l=0x%x, cal_dac_ana_dc_r=0x%x", cal_dac_ana_dc_l, cal_dac_ana_dc_r);
    analog_aud_dc_calib_set_value(cal_dac_ana_dc_l, cal_dac_ana_dc_r);
    analog_aud_dc_calib_enable(true);

    if (calib_cmd == CODEC_CALIB_CMD_DIG_GAIN) {
        int32_t dc_out_l = 0, dc_out_r = 0;

        TRACE(1, "\nCALIB DAC GAIN\n");
        TRACE(1, "cal_dac_dig_dc_l=%d, cal_dac_dig_dc_r=%d",cal_dac_dig_dc_l, cal_dac_dig_dc_r);
        hal_codec_dac_dc_offset_enable(cal_dac_dig_dc_l, cal_dac_dig_dc_r);
        hal_codec_set_dig_dac_gain_dr(DAC_DC_CALIB_SPK_CHAN_MAP, 0);
        cal_dac_out_enable = true;
        osDelay(5);
        get_codec_dac_dc(&dc_l, &dc_r);
        for (i = 0; i < DAC_GAIN_CALIB_AUTO_CNT;i++) {
            get_codec_dac_dc(&dc_l, &dc_r);
            dc_out_l += (dc_l - dc_target_l);
            dc_out_r += (dc_r - dc_target_r);
        }
        cal_dac_out_enable = false;
        dc_out_l /= DAC_GAIN_CALIB_AUTO_CNT;
        dc_out_r /= DAC_GAIN_CALIB_AUTO_CNT;
        cfg->out_dc_l = dc_out_l;
        cfg->out_dc_r = dc_out_r;
        TRACE(1, "====>Final dc_out_l=%d, dc_out_r=%d", dc_out_l, dc_out_r);
        goto _end;
    }

    if (calib_cmd == CODEC_CALIB_CMD_GET_CUR_DC) {
        get_codec_dac_dc(&dc_l, &dc_r);
        osDelay(5);
        cfg->dig_dc_l = dc_target_l;
        cfg->dig_dc_r = dc_target_r;
        cfg->out_dc_l = dc_l;
        cfg->out_dc_r = dc_r;
        TRACE(1, "====>GET_CUR_DC: tgt_l=%d, tgt_r=%d, dc_l=%d, dc_r=%d",
            dc_target_l, dc_target_r, dc_l, dc_r);
        goto _end;
    }

    TRACE(1, "\nCALIB DIG DC\n");
    val_l = val_r = 0;
    diff_l = diff_r = 0;
    comp_old_l = comp_old_r = 0;
#ifdef AUDIO_OUTPUT_DEBUG_ANA_ADC
    do {
        osDelay(5);
        get_codec_dac_dc(&dc_l, &dc_r);
        TRACE(1, "dc_tgt_l=%d, dc_tgt_r=%d", dc_target_l, dc_target_r);
    } while (1);
#endif
#ifdef AUDIO_OUTPUT_DIG_DC_DEEP_CALIB
    ret = codec_calib_dac_dig_dc(en_l, en_r, dc_target_l, dc_target_r, &val_l, &val_r);
    if (!ret) {
        cfg->state = CODEC_CALIB_STATE_ERR_DAC_DC_L | CODEC_CALIB_STATE_ERR_DAC_DC_R;
    }
    (void)diff_l;
    (void)diff_r;
    (void)comp_old_l;
    (void)comp_old_r;
    (void)comp_l;
    (void)comp_r;
#else
    for (i = 0; i < DAC_DC_CALIB_AUTO_CNT; i++) {
        if (cal_dac_ana_gain >= 0xF) {
            osDelay(5);
        } else {
            osDelay(5);
        }
        get_codec_dac_dc(&dc_l, &dc_r);

        if (en_l) {
            comp_l = dc_target_l - dc_l;
        }
        if (en_r) {
            comp_r = dc_target_r - dc_r;
        }
        TRACE(1, "[%u] comp_l=%d comp_r=%d", i, comp_l, comp_r);

#ifdef AUDIO_OUTPUT_DC_CALIB_ANA
        if (i == 1) {
            if (en_l) {
                dc_step_old_l = dc_step_l;
                dc_step_l -= diff_l ? comp_l / diff_l : 0;
            }
            if (en_r) {
                dc_step_old_r = dc_step_r;
                dc_step_r -= diff_r ? comp_r / diff_r : 0;
            }
            TRACE(1, "Update dc_step_l=%d->%d dc_step_r=%d->%d", dc_step_old_l, dc_step_l, dc_step_old_r, dc_step_r);
        }
#endif

        // DC noise is larger than one dc step unit ...
        if (en_l && !en_r) {
            if ((ABS(comp_l) <= dc_step_l)
#ifndef AUDIO_OUTPUT_DC_CALIB_ANA
                || (ABS(comp_l) <= comp_gate_l)
#endif
                ) {
                TRACE(1, "break: comp_l=%d, dc_step_l=%d", comp_l, dc_step_l);
                break;
            }
        } else if (en_l && en_r) {
            if ((ABS(comp_l) < dc_step_l && ABS(comp_r) < dc_step_r)
#ifndef AUDIO_OUTPUT_DC_CALIB_ANA
                || (ABS(comp_l) <= comp_gate_l && ABS(comp_r) <= comp_gate_r)
#endif
                ) {
                TRACE(1, "break: comp_l=%d, dc_step_l=%d", comp_l, dc_step_l);
                TRACE(1, "break: comp_r=%d, dc_step_r=%d", comp_r, dc_step_r);
                break;
            }
        } else if (!en_l && en_r) {
            if ((ABS(comp_r) <= dc_step_r)
#ifndef AUDIO_OUTPUT_DC_CALIB_ANA
                || ABS(comp_r) <= comp_gate_r
#endif
                ) {
                TRACE(1, "break: comp_r=%d, dc_step_r=%d", comp_r, dc_step_r);
                break;
            }
        }
        if (i >= 2) {
            if (en_l) {
                if (ABS(comp_l) > 5 * ABS(comp_old_l) && ABS(comp_l) > 5 * dc_step_l) {
                    comp_l = dc_step_l * ((comp_l > 0) ? 1 : -1);
                    TRACE(1, "[%u] ERROR: comp_l too large -- reset to %d", i, comp_l);
                }
            }
            if (en_r) {
                if (ABS(comp_r) > 5 * ABS(comp_old_r) && ABS(comp_r) > 5 * dc_step_r) {
                    comp_r = dc_step_r * ((comp_r > 0) ? 1 : -1);
                    TRACE(1, "[%u] ERROR: comp_r too large -- reset to %d", i, comp_r);
                }
            }
        }
        if (en_l) {
            ASSERT(dc_step_l != 0, "[%u] zero dc_step_l", i);
            comp_old_l = comp_l;
#ifdef AUDIO_OUTPUT_DC_CALIB_ANA
            diff_l += (comp_l + (dc_step_l / 2) * (comp_l > 0 ? 1 : -1)) / dc_step_l;
#else
            diff_l += comp_l;
#endif
            ASSERT(ABS(comp_l) <= DAC_DC_CALIB_MAX_ABS, "[%u] comp_l too large: %d", i, comp_l);
            ASSERT(ABS(diff_l) <= DAC_DC_CALIB_MAX_ABS, "[%u] diff_l too large: %d", i, diff_l);
        } else {
            diff_l = comp_l = 0;
        }
        if (en_r) {
            ASSERT(dc_step_r != 0, "[%u] zero dc_step_r", i);
            comp_old_r = comp_r;
#ifdef AUDIO_OUTPUT_DC_CALIB_ANA
            diff_r += (comp_r + (dc_step_r / 2) * (comp_r > 0 ? 1 : -1)) / dc_step_r;
#else
            diff_r += comp_r;
#endif
            ASSERT(ABS(comp_r) <= DAC_DC_CALIB_MAX_ABS, "[%u] comp_r too large: %d", i, comp_r);
            ASSERT(ABS(diff_r) <= DAC_DC_CALIB_MAX_ABS, "[%u] diff_r too large: %d", i, diff_r);
        } else {
            diff_r = comp_r = 0;
        }
#ifdef AUDIO_OUTPUT_DC_CALIB_ANA
        val_l = analog_aud_dac_dc_diff_to_val(diff_l);
        val_r = analog_aud_dac_dc_diff_to_val(diff_r);
        analog_aud_dc_calib_set_value(val_l, val_r);
#else
#if defined(CHIP_BEST2300A)
        val_l = -diff_l;
        val_r = -diff_r;
#else
        val_l = diff_l;
        val_r = diff_r;
#endif
        hal_codec_dac_dc_offset_enable(val_l, val_r);
#endif
        TRACE(1, "[%u] val_l=0x%08X val_r=0x%08X diff_l=%d diff_r=%d", i, val_l, val_r, diff_l, diff_r);
    }
#endif /* AUDIO_OUTPUT_DIG_DC_DEEP_CALIB */
    if (calib_cmd == CODEC_CALIB_CMD_DIG_DC) {
        cfg->dig_dc_l = val_l;
        cfg->dig_dc_r = val_r;
#ifdef AUDIO_OUTPUT_ASSERT_LARGE_DC
        if (en_l) {
            ASSERT(ABS(diff_l) <= FINAL_DAC_DC_CALIB_MAX_ABS, "final DAC_DC_L too large: %d", val_l);
        }
        if (en_r) {
            ASSERT(ABS(diff_r) <= FINAL_DAC_DC_CALIB_MAX_ABS, "final DAC_DC_R too large: %d", val_r);
        }
#elif defined(AUDIO_OUTPUT_ERROR_LARGE_DC)
        if (en_l) {
            if (ABS(diff_l) > FINAL_DAC_DC_CALIB_MAX_ABS) {
                cfg->state = CODEC_CALIB_STATE_ERR_DAC_DC_L;
            }
        }
        if (en_r) {
            if (ABS(diff_r) > FINAL_DAC_DC_CALIB_MAX_ABS) {
                cfg->state = CODEC_CALIB_STATE_ERR_DAC_DC_R;
            }
        }
#endif
    }
    TRACE(1, "Final analog_gain: %d", cal_dac_ana_gain);
    TRACE(1, "Final comp_l=%d, comp_r=%d", comp_l, comp_r);
    TRACE(1, "Final diff_l=%d, diff_r=%d", diff_l, diff_r);
    TRACE(1, "Final val_l  =0x%08X val_r  =0x%08X", val_l, val_r);
#ifdef AUDIO_OUTPUT_DC_CALIB_ANA
    TRACE(1, "Final efuse_l=0x%04X efuse_r=0x%04X", analog_aud_dc_calib_val_to_efuse(val_l), analog_aud_dc_calib_val_to_efuse(val_r));
#endif

    TRACE(1, "\nGetting final values consumes %u ms\n", FAST_TICKS_TO_MS(hal_fast_sys_timer_get() - time));

    //hal_codec_set_dac_dc_offset(val_l, val_r);
    //hal_codec_dac_dc_offset_enable(val_l, val_r);
    //while (1) {}

_end:
    return 0;

#else
    cfg->dig_dc_l = 0;
    cfg->dig_dc_r = 0;
    cfg->ana_dc_l = 0;
    cfg->ana_dc_r = 0;
    cfg->out_dc_l = 0;
    cfg->out_dc_r = 0;
    return 1;
#endif
}

#ifdef CODEC_ERROR_HANDLING
static bool is_audio_stream_on[AUD_STREAM_ID_NUM][AUD_STREAM_NUM];
static uint8_t af_supervisor_period_cnt_since_last_dma_irq[AUD_STREAM_ID_NUM][AUD_STREAM_NUM];
static uint8_t alive_audio_stream_count = 0;
static af_enter_abnormal_state_callback_func_t af_enter_abnormal_state_cb_handler = NULL;

static void af_supervisor_timer_cb(void const *n);
osTimerDef(AF_SUPERVISOR_TIMER, af_supervisor_timer_cb);
osTimerId af_supervisor_timer_id = NULL;

static void af_supervisor_init(void)
{
    alive_audio_stream_count = 0;
    memset(is_audio_stream_on, 0, sizeof(is_audio_stream_on));
    memset(af_supervisor_period_cnt_since_last_dma_irq, 0, sizeof(af_supervisor_period_cnt_since_last_dma_irq));
    af_supervisor_timer_id = osTimerCreate(osTimer(AF_SUPERVISOR_TIMER), osTimerPeriodic, NULL);
}

static void af_ping_supervisor(enum AUD_STREAM_ID_T streamId, enum AUD_STREAM_T streamType)
{
    af_supervisor_period_cnt_since_last_dma_irq[streamId][streamType] = 0;
}


void af_register_enter_abnormal_state_callback(af_enter_abnormal_state_callback_func_t func)
{
    af_enter_abnormal_state_cb_handler = func;
}

static void af_supervisor_timer_cb(void const *n)
{
    for (uint8_t id = 0;id < AUD_STREAM_ID_NUM; id++)
    {
        for (uint8_t type = 0; type < AUD_STREAM_NUM; type++)
        {
            if (is_audio_stream_on[id][type])
            {
                af_supervisor_period_cnt_since_last_dma_irq[id][type]++;
                if (af_supervisor_period_cnt_since_last_dma_irq[id][type] >=
                    AF_SUPERVISOR_TIMOUT_PERIOD_CNT_SINCE_LAST_DMA_IRQ)
                {
                    if (af_enter_abnormal_state_cb_handler != NULL)
                    {
                        af_enter_abnormal_state_cb_handler(id, type);
                    }
                }
            }
        }
    }
}

static void af_update_stream_state(enum AUD_STREAM_ID_T streamId, enum AUD_STREAM_T streamType, bool isOn)
{
    if (is_audio_stream_on[streamId][streamType] ^ isOn)
    {
        is_audio_stream_on[streamId][streamType] = isOn;
        af_supervisor_period_cnt_since_last_dma_irq[streamId][streamType] = 0;

        uint8_t formerAliveStreamCount = alive_audio_stream_count;

        if (isOn)
        {
            if (alive_audio_stream_count < (AUD_STREAM_ID_NUM * AUD_STREAM_NUM))
            {
                alive_audio_stream_count++;

                if (0 == formerAliveStreamCount)
                {
                    osTimerStart(af_supervisor_timer_id, AF_SUPERVISOR_PERIOD_IN_MS);
                }
            }
        }
        else
        {
            if (alive_audio_stream_count > 0)
            {
                alive_audio_stream_count--;
                if (0 == alive_audio_stream_count)
                {
                    osTimerStop(af_supervisor_timer_id);
                }
            }
        }
    }
}
#if 0
void af_codec_monitor_get_info(uint32_t *volume)
{
    *volume = audio_mon_volume;
}

void af_codec_monitor_get_info_interrupt(uint32_t (*irq_table)[2])
{
    for (int i = 0; i < AUD_STREAM_ID_NUM; i++)    {
        irq_table[i][0] = audio_mon_dma_irq[i][0];
        irq_table[i][1] = audio_mon_dma_irq[i][1];
    }
}

float af_codec_monitor_get_coef(void)
{
    return output_coef;
}
#endif
#endif
