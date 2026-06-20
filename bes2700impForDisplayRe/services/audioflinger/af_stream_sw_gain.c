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
#include "af_stream_sw_gain.h"
#include "cmsis.h"
#include "hal_trace.h"
#include "hal_codec.h"
#include "string.h"
#include "eq_cfg.h"

// #define SW_GAIN_OPTI_MIPS_FOR_ALIGN_4

#define MAX_DAC_DBVAL                       (50)
#define ZERODB_DAC_DBVAL                    (0)
#define MIN_DAC_DBVAL                       (-99)

typedef struct {
    float coefs_b[3];
    float coefs_a[3];
    float history_x[2];
    float history_y[2];
} SW_GAIN_IIR_T;

typedef struct {
    uint32_t sample_rate;
    uint32_t samples;
    float step;
    float curr_gain;
    float new_gain;
    float tar_gain;
    bool finished;
} fade_context_t;

typedef struct {
    enum AUD_BITS_T        bits;
    enum AUD_SAMPRATE_T    sample_rate;
    enum AUD_CHANNEL_NUM_T chans;
    bool            sw_gain_enabled;
    int32_t         gain_mode;      // 0 iir, 1 linear
    SW_GAIN_IIR_T   gain_iir;
    fade_context_t  gain_linear;
    fade_context_t  second_gain;
    volatile float  output_coef;
} AF_STREAM_SW_GAIN_CONTEXT_T;

#define _USER_QTY                (4)
static int8_t g_sw_gain_ctx_index[AUD_STREAM_ID_NUM][AUD_STREAM_NUM] = {{0}};
static AF_STREAM_SW_GAIN_CONTEXT_T g_sw_gain_ctx[_USER_QTY];

static int32_t get_sw_gain_ctx_index(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    return (int32_t)(g_sw_gain_ctx_index[id][stream] - 1);
}

static POSSIBLY_UNUSED
void reset_sw_gain_ctx_index(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    g_sw_gain_ctx_index[id][stream] = 0;
    return;
}

static POSSIBLY_UNUSED
void set_sw_gain_ctx_index(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, int8_t index)
{
    g_sw_gain_ctx_index[id][stream] = index + 1;
    return;
}

static bool check_sw_gain_ctx_index_used(int8_t index)
{
    for (uint32_t id = 0; id < AUD_STREAM_ID_NUM; id++) {
        for (uint32_t stream = 0; stream < AUD_STREAM_NUM; stream++){
            if (index == get_sw_gain_ctx_index(id, stream)) {
                return true;
            }
        }
    }

    return false;
}

static int32_t get_sw_gain_new_index(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    int32_t index = 0;
    for (index = 0; index < _USER_QTY; index++) {
        if (!check_sw_gain_ctx_index_used(index)) {
            TRACE(0, "[%s] Alloc new index[%d], id: %d, stream: %d", __func__, index, id, stream);
            set_sw_gain_ctx_index(id, stream, index);
            return index;
        }
    }

    TRACE(0, "[%s] Can not alloc index, id: %d, stream: %d. You can increase _USER_QTY", __func__, id, stream);
    return -1;

}

static AF_STREAM_SW_GAIN_CONTEXT_T *get_sw_gain_ctx(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    int32_t index = 0;

    index = get_sw_gain_ctx_index(id, stream);

    if (index >= 0 && index < _USER_QTY) {
        return &g_sw_gain_ctx[index];
    } else {
        return NULL;
    }
}

const static SW_GAIN_IIR_T g_sw_gain_iir = {
    .coefs_b={0.0000452390567342137f, 0.0001890478113468427f, 0.00009452390567342137f},
    .coefs_a={1.00000000000000000000f, -1.961110637819907f, 0.961488733442601f},
    .history_x={0,0},
    .history_y={0,0},
};

static void fade_reset_to_zero(fade_context_t *ctx, uint32_t sample_rate, uint32_t ms)
{
    ctx->sample_rate = sample_rate;
    ctx->samples = (uint32_t)(sample_rate * ms / 1000);
    ctx->step = 0.f;
    ctx->curr_gain = 0.f;
    ctx->new_gain = 0.f;
    ctx->tar_gain = 0.f;
    ctx->finished = true;
}

static void fade_reset_to_one(fade_context_t *ctx, uint32_t sample_rate, uint32_t ms)
{
    ctx->sample_rate = sample_rate;
    ctx->samples = (uint32_t)(sample_rate * ms / 1000);
    ctx->step = 0.f;
    ctx->curr_gain = 1.f;
    ctx->new_gain = 1.f;
    ctx->tar_gain = 1.f;
    ctx->finished = true;
}

static void fade_set_new_gain(fade_context_t *ctx, float gain)
{
    if (ABS(gain - ctx->new_gain) < 1e-4) {
        return;
    }
    ctx->new_gain = gain;
}

static void fade_set_smooth_time(fade_context_t *ctx, uint32_t ms)
{
    ctx->samples = (uint32_t)(ctx->sample_rate * ms / 1000);
}

static float fade_get_gain(fade_context_t *ctx)
{
    return ctx->new_gain;
}

static float fade_smooth_gain(fade_context_t *ctx)
{
    if (ctx->new_gain != ctx->tar_gain) {
        ctx->tar_gain = ctx->new_gain;
        ctx->step = (ctx->tar_gain - ctx->curr_gain) / ctx->samples;
        ctx->finished = false;
    }

    if (ctx->finished)
        return ctx->curr_gain;

    ctx->curr_gain = ctx->curr_gain + ctx->step;

    if ((ctx->step > 0 && ctx->curr_gain > ctx->tar_gain) ||
            (ctx->step < 0 && ctx->curr_gain < ctx->tar_gain)) {
        ctx->curr_gain = ctx->tar_gain;
        ctx->finished = true;
    }

    return ctx->curr_gain;
}

static POSSIBLY_UNUSED
bool fade_smooth_is_finished(fade_context_t *ctx)
{
    return ctx->finished;
}

int32_t af_stream_sw_gain_init(void)
{
    return 0;
}

static void af_stream_sw_gain_iir_process(uint8_t *buf, uint32_t size, enum AUD_BITS_T bits, enum AUD_CHANNEL_NUM_T chans,
                                            SW_GAIN_IIR_T *iir, float coef, fade_context_t *second_gain)
{
    uint32_t i,pcm_len;
    int32_t pcm_out_l;
    int32_t pcm_out_r;

    float output_gain=1.0f;

    //TRACE(1,"af_stream_sw_gain_iir_process:%d",(int32_t)(saved_output_coef*1000));

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

    if (chans==AUD_CHANNEL_NUM_1) {
        if (bits <= AUD_BITS_16) {
            pcm_len = size / sizeof(int16_t);
            int16_t *pcm_buf = (int16_t *)buf;
            for (i = 0; i < pcm_len;) {
                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(second_gain);
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

                output_gain *= fade_smooth_gain(second_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 16);
                i++;

                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(second_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 16);
                i++;

                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(second_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 16);
                i++;
#endif
            }
        } else {
            pcm_len = size / sizeof(int32_t);
            int32_t *pcm_buf = (int32_t *)buf;
            for (i = 0; i < pcm_len;) {
                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(second_gain);
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

                output_gain *= fade_smooth_gain(second_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 24);
                i++;

                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(second_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 24);
                i++;

                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(second_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 24);
                i++;
#endif
            }
        }
    } else if (chans == AUD_CHANNEL_NUM_2) {
        if (bits <= AUD_BITS_16) {
            pcm_len = size / sizeof(int16_t);
            int16_t *pcm_buf = (int16_t *)buf;
            for (i = 0; i < pcm_len;) {
                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(second_gain);
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

                output_gain *= fade_smooth_gain(second_gain);
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

                output_gain *= fade_smooth_gain(second_gain);
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

                output_gain *= fade_smooth_gain(second_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_out_r = (int32_t)(pcm_buf[i+1] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 16);
                pcm_buf[i+1] = __SSAT(pcm_out_r, 16);
                i=i+2;
#endif
            }
        } else {
            pcm_len = size / sizeof(int32_t);
            int32_t *pcm_buf = (int32_t *)buf;
            for (i = 0; i < pcm_len;) {
                output_gain = coef * coefs_b0 + history_x0 * coefs_b1 + history_x1 * coefs_b2
                              - history_y0 * coefs_a1 - history_y1 * coefs_a2;

                history_y1 = history_y0;
                history_y0 = output_gain;
                history_x1 = history_x0;
                history_x0 = coef;

                output_gain *= fade_smooth_gain(second_gain);
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

                output_gain *= fade_smooth_gain(second_gain);
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

                output_gain *= fade_smooth_gain(second_gain);
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

                output_gain *= fade_smooth_gain(second_gain);
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

static void af_stream_sw_gain_linear_process(uint8_t *buf, uint32_t size, enum AUD_BITS_T bits, enum AUD_CHANNEL_NUM_T chans,
                                            fade_context_t *linear, fade_context_t *second_gain)
{
    uint32_t i,pcm_len;
    int32_t pcm_out_l;
    int32_t pcm_out_r;

    float output_gain=1.0f;

    if (chans==AUD_CHANNEL_NUM_1) {
        if (bits <= AUD_BITS_16) {
            pcm_len = size / sizeof(int16_t);
            int16_t *pcm_buf = (int16_t *)buf;
            for (i = 0; i < pcm_len;) {
                output_gain = fade_smooth_gain(linear);

                output_gain *= fade_smooth_gain(second_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 16);
                i++;
            }
        } else {
            pcm_len = size / sizeof(int32_t);
            int32_t *pcm_buf = (int32_t *)buf;
            for (i = 0; i < pcm_len;) {
                output_gain = fade_smooth_gain(linear);

                output_gain *= fade_smooth_gain(second_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 24);
                i++;
            }
        }
    } else if (chans == AUD_CHANNEL_NUM_2) {
        if (bits <= AUD_BITS_16) {
            pcm_len = size / sizeof(int16_t);
            int16_t *pcm_buf = (int16_t *)buf;
            for (i = 0; i < pcm_len;) {
                output_gain = fade_smooth_gain(linear);

                output_gain *= fade_smooth_gain(second_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_out_r = (int32_t)(pcm_buf[i+1] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 16);
                pcm_buf[i+1] = __SSAT(pcm_out_r, 16);
                i=i+2;
            }
        } else {
            pcm_len = size / sizeof(int32_t);
            int32_t *pcm_buf = (int32_t *)buf;
            for (i = 0; i < pcm_len;) {
                output_gain = fade_smooth_gain(linear);

                output_gain *= fade_smooth_gain(second_gain);
                pcm_out_l = (int32_t)(pcm_buf[i] * output_gain);
                pcm_out_r = (int32_t)(pcm_buf[i+1] * output_gain);
                pcm_buf[i] = __SSAT(pcm_out_l, 24);
                pcm_buf[i+1] = __SSAT(pcm_out_r, 24);
                i=i+2;
            }
        }
    }
}

void af_stream_sw_gain_process(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, uint8_t *buf, uint32_t len)
{
    AF_STREAM_SW_GAIN_CONTEXT_T *sw_gain_ctx = NULL;

    sw_gain_ctx = get_sw_gain_ctx(id, stream);
    if (sw_gain_ctx == NULL) {
        return;
    }

    uint32_t bits = sw_gain_ctx->bits;
    uint32_t chans = sw_gain_ctx->chans;
    fade_context_t *second_gain = &sw_gain_ctx->second_gain;

    if (sw_gain_ctx->gain_mode == AUDIO_OUTPUT_SW_GAIN_MODE_IIR) {
        af_stream_sw_gain_iir_process(buf, len, bits, chans, &sw_gain_ctx->gain_iir, sw_gain_ctx->output_coef, second_gain);
    } else if (sw_gain_ctx->gain_mode == AUDIO_OUTPUT_SW_GAIN_MODE_LINEAR) {
        af_stream_sw_gain_linear_process(buf, len, bits, chans, &sw_gain_ctx->gain_linear, second_gain);
    }
}

static int32_t af_stream_sw_gain_iir_open(AF_STREAM_SW_GAIN_CONTEXT_T *gain_ctx)
{
    ASSERT(gain_ctx != NULL, "[%s]:af software gain is NULL", __func__);
    uint32_t sample_rate = gain_ctx->sample_rate;
    SW_GAIN_IIR_T *sw_gain_iir = &gain_ctx->gain_iir;

    float coefs[6], gain = 0.f, f0 = 30.f, Q = 0.5f;                            //f0 larger means converge faster
    sw_gain_iir->history_x[0]=0.0f;
    sw_gain_iir->history_x[1]=0.0f;
    sw_gain_iir->history_y[0]=0.0f;
    sw_gain_iir->history_y[1]=0.0f;

    iir_lowspass_coefs_generate(gain, f0 / sample_rate, Q, coefs);
    sw_gain_iir->coefs_b[0] = coefs[3];
    sw_gain_iir->coefs_b[1] = coefs[4];
    sw_gain_iir->coefs_b[2] = coefs[5];
    sw_gain_iir->coefs_a[0] = coefs[0];
    sw_gain_iir->coefs_a[1] = coefs[1];
    sw_gain_iir->coefs_a[2] = coefs[2];

    return 0;
}

static int32_t af_stream_sw_gain_linear_open(AF_STREAM_SW_GAIN_CONTEXT_T *gain_ctx)
{
    ASSERT(gain_ctx != NULL, "[%s]:af software gain is NULL", __func__);
    uint32_t sample_rate = gain_ctx->sample_rate;
    fade_reset_to_zero(&gain_ctx->gain_linear, sample_rate, 1000);

    return 0;
}

int32_t af_stream_sw_gain_open(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream,
                                enum AUD_SAMPRATE_T sample_rate, enum AUD_BITS_T bits,
                                enum AUD_CHANNEL_NUM_T chans, int32_t gain_mode)
{
    AF_STREAM_SW_GAIN_CONTEXT_T *sw_gain_ctx = NULL;

    sw_gain_ctx = get_sw_gain_ctx(id, stream);

    if (sw_gain_ctx == NULL) {
        uint32_t index = get_sw_gain_new_index(id, stream);
        if (index < 0) {
            return -1;
        }
        sw_gain_ctx = &g_sw_gain_ctx[index];
    }


    sw_gain_ctx->bits = bits;
    sw_gain_ctx->chans = chans;
    sw_gain_ctx->sample_rate = sample_rate;
    sw_gain_ctx->sw_gain_enabled = true;
    sw_gain_ctx->gain_mode = gain_mode;

    if (sw_gain_ctx->gain_mode == AUDIO_OUTPUT_SW_GAIN_MODE_IIR) {
        sw_gain_ctx->gain_iir = g_sw_gain_iir;
        af_stream_sw_gain_iir_open(sw_gain_ctx);
    } else if (sw_gain_ctx->gain_mode == AUDIO_OUTPUT_SW_GAIN_MODE_LINEAR) {
        af_stream_sw_gain_linear_open(sw_gain_ctx);
    }

    fade_reset_to_one(&sw_gain_ctx->second_gain, sample_rate, 100);

    return 0;
}

int32_t af_stream_sw_gain_close(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    // To avoid multiple requests for different indexes, the module is not closed by default.
    // reset_sw_gain_ctx_index(id, stream);

    return 0;
}

void af_stream_sw_gain_set_gain_coef(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, float coef)
{
    TRACE(0, "[%08d]:volume_level_to_float!", (int32_t)(coef * 10000000));  //vol=2,3,4,5,6
    AF_STREAM_SW_GAIN_CONTEXT_T *sw_gain_ctx = get_sw_gain_ctx(id, stream);
    if (sw_gain_ctx == NULL) {
        return;
    }

    TRACE(4, "[%s] id: %d, stream: %d, coef: %08d", __func__, id, stream, (int32_t)(coef * 10000000));
    if (sw_gain_ctx->gain_mode == AUDIO_OUTPUT_SW_GAIN_MODE_IIR) {
        sw_gain_ctx->output_coef = coef;
    } else if (sw_gain_ctx->gain_mode == AUDIO_OUTPUT_SW_GAIN_MODE_LINEAR) {
        fade_set_new_gain(&sw_gain_ctx->gain_linear, coef);
    }
}

static float volume_level_to_float(uint32_t vol)
{
    float coef = 0.0;
    const struct CODEC_DAC_VOL_T *dac_vol = hal_codec_get_dac_volume(vol);
    int32_t dac_vol_db = dac_vol->sdac_volume;

    if (dac_vol_db == ZERODB_DAC_DBVAL) {
        coef = 1.0;
    } else if (dac_vol_db <= MIN_DAC_DBVAL) {
        coef = 0.0;
    } else {
        if (dac_vol_db > MAX_DAC_DBVAL) {
            dac_vol_db = MAX_DAC_DBVAL;
        }
        coef = db_to_float(dac_vol_db);
    }

    return coef;
}

void af_stream_sw_gain_set_volume_level(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, uint32_t vol)
{
    af_stream_sw_gain_set_gain_coef(id, stream, volume_level_to_float(vol));
}

void af_stream_sw_gain_set_smooth_time(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, uint32_t time_ms)
{
    AF_STREAM_SW_GAIN_CONTEXT_T *sw_gain_ctx = get_sw_gain_ctx(id, stream);
    if (sw_gain_ctx == NULL) {
        return;
    }

    if (sw_gain_ctx->gain_mode == AUDIO_OUTPUT_SW_GAIN_MODE_LINEAR) {
        fade_set_smooth_time(&sw_gain_ctx->gain_linear, time_ms);
    } else {
        TRACE(0, "[%s]:iir mode not support set smooth time!", __func__);
    }
}

float af_stream_sw_gain_get_gain_coef(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    AF_STREAM_SW_GAIN_CONTEXT_T *sw_gain_ctx = get_sw_gain_ctx(id, stream);
    if (sw_gain_ctx == NULL) {
        return 0;
    }

    float coef = 0.0;
    if (sw_gain_ctx->gain_mode == AUDIO_OUTPUT_SW_GAIN_MODE_IIR) {
        coef = sw_gain_ctx->output_coef;
    } else if (sw_gain_ctx->gain_mode == AUDIO_OUTPUT_SW_GAIN_MODE_LINEAR) {
        coef = fade_get_gain(&sw_gain_ctx->gain_linear);
    }

    return coef;
}

void af_stream_sw_gain_enable(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, bool enable)
{
    AF_STREAM_SW_GAIN_CONTEXT_T *sw_gain_ctx = get_sw_gain_ctx(id, stream);
    if (sw_gain_ctx == NULL) {
        return;
    }

    sw_gain_ctx->sw_gain_enabled = enable;
}

bool af_stream_sw_gain_get_enable(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    AF_STREAM_SW_GAIN_CONTEXT_T *sw_gain_ctx = get_sw_gain_ctx(id, stream);
    if (sw_gain_ctx == NULL) {
        return false;
    }

    return sw_gain_ctx->sw_gain_enabled;
}

void af_stream_sw_gain_set_second_coef(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, float coef)
{
    AF_STREAM_SW_GAIN_CONTEXT_T *sw_gain_ctx = get_sw_gain_ctx(id, stream);
    if (sw_gain_ctx == NULL) {
        return;
    }

    TRACE(3, "[%s] id: %d, stream: %d, coef: %08d", __func__, id, stream, (int32_t)(coef * 10000000));
    fade_set_new_gain(&sw_gain_ctx->second_gain, coef);
}