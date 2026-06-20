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
// Standard C Included Files
#include "cmsis.h"
#include "plat_types.h"
#include <string.h>
#include "heap_api.h"
#include "hal_location.h"
#include "sbc_api.h"
#include "sbc_error_code.h"
#include "a2dp_decoder_internal.h"
#include "hal_timer.h"
#include "cmsis_os.h"
#ifdef IBRT
#include "app_tws_ibrt_audio_analysis.h"
#endif

#if defined(A2DP_SBC_PLC_ENABLED)
#include "sbcplc.h"
static float *cos_buf = NULL;
#define SBC_SMOOTH_LENGTH   128
static struct PLC_State * sbc_plc_state0;
static struct PLC_State * sbc_plc_state1;
extern int a2dp_audio_sysfreq_boost_start(uint32_t boost_cnt);
#endif

//#define A2DP_SBC_PLC_CALC_MIPS

#ifndef SBC_MTU_LIMITER
#define SBC_MTU_LIMITER (250) /*must <= 332*/
#endif
#define SBC_PCMLEN_DEFAULT (512)

#define SBC_LIST_SAMPLES (128)

#define SBC_PLC_FRAME_THRESHOLD (10)

static A2DP_AUDIO_CONTEXT_T *a2dp_audio_context_p = NULL;
extern const A2DP_AUDIO_DECODER_T a2dp_audio_sbc_decoder_config;

typedef struct  {
    sbc_decoder_t *sbc_decoder;
    pcm_frame_t *pcm_data;
} a2dp_audio_sbc_decoder_t;

typedef struct {
    uint16_t sequenceNumber;
    uint32_t timestamp;
    uint16_t curSubSequenceNumber;
    uint16_t totalSubSequenceNumber;
    uint8_t *sbc_buffer;
    uint32_t sbc_buffer_len;
} a2dp_audio_sbc_decoder_frame_t;

int a2dp_audio_sbc_channel_select(A2DP_AUDIO_CHANNEL_SELECT_E chnl_sel);

static a2dp_audio_sbc_decoder_t a2dp_audio_sbc_decoder;
static sbc_decoder_t *a2dp_audio_sbc_decoder_preparse = NULL;


static A2DP_AUDIO_DECODER_LASTFRAME_INFO_T a2dp_audio_sbc_lastframe_info;

static uint16_t sbc_mtu_limiter = SBC_MTU_LIMITER;

static bool sbc_chnl_mode_mono = false;
#if !defined(BT_DONT_PLAY_MUTE_WHEN_A2DP_STUCK_PATCH)
static btif_media_header_t sbc_decoder_last_valid_frame = {0,};
static bool sbc_decoder_last_valid_frame_ready = false;
static int a2dp_audio_sbc_header_parser_init(void);
#endif
static void *a2dp_audio_sbc_subframe_malloc(uint32_t sbc_len)
{
    a2dp_audio_sbc_decoder_frame_t *sbc_decoder_frame_p = NULL;
    uint8_t *sbc_buffer = NULL;

    sbc_buffer = (uint8_t *)a2dp_audio_heap_malloc(sbc_len);
    sbc_decoder_frame_p = (a2dp_audio_sbc_decoder_frame_t *)a2dp_audio_heap_malloc(sizeof(a2dp_audio_sbc_decoder_frame_t));
    sbc_decoder_frame_p->sbc_buffer = sbc_buffer;
    sbc_decoder_frame_p->sbc_buffer_len = sbc_len;
    return (void *)sbc_decoder_frame_p;
}

static void a2dp_audio_sbc_subframe_free(void *packet)
{
    a2dp_audio_sbc_decoder_frame_t *sbc_decoder_frame_p = (a2dp_audio_sbc_decoder_frame_t *)packet;
    a2dp_audio_heap_free(sbc_decoder_frame_p->sbc_buffer);
    a2dp_audio_heap_free(sbc_decoder_frame_p);
}

static void sbc_codec_init(void)
{
    sbc_decoder_open(a2dp_audio_sbc_decoder.sbc_decoder);
    a2dp_audio_sbc_decoder.pcm_data->pcm_data = NULL;
    a2dp_audio_sbc_decoder.pcm_data->valid_size = 0;
}

#ifdef A2DP_CP_ACCEL
struct A2DP_CP_SBC_IN_FRM_INFO_T {
    uint16_t sequenceNumber;
    uint32_t timestamp;
    uint16_t curSubSequenceNumber;
    uint16_t totalSubSequenceNumber;
    A2DP_AUDIO_CHANNEL_SELECT_E chnl_sel;
};

struct A2DP_CP_SBC_OUT_FRM_INFO_T {
    struct A2DP_CP_SBC_IN_FRM_INFO_T in_info;
    uint16_t frame_samples;
    uint16_t decoded_frames;
    uint16_t frame_idx;
    uint16_t pcm_len;
};

static bool cp_codec_reset;
extern "C" uint32_t get_in_cp_frame_cnt(void);
extern "C" uint32_t get_out_cp_frame_cnt(void);
extern "C" uint32_t get_in_cp_free_cnt(void);
extern "C" unsigned int set_cp_reset_flag(uint8_t evt);

/* int a2dp_cp_sbc_cp_decode(void); */

extern uint32_t app_bt_stream_get_dma_buffer_samples(void);

static int TEXT_SBC_LOC a2dp_cp_sbc_after_cache_underflow(void)
{
#ifdef A2DP_CP_ACCEL
    // cp_codec_reset = true;
#endif
    return 0;
}

static int a2dp_cp_sbc_mcu_decode(uint8_t *buffer, uint32_t buffer_bytes)
{
    a2dp_audio_sbc_decoder_frame_t *sbc_decoder_frame = NULL;
    list_node_t *node = NULL;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    int ret, dec_ret;
    struct A2DP_CP_SBC_IN_FRM_INFO_T in_info;
    struct A2DP_CP_SBC_OUT_FRM_INFO_T *p_out_info = NULL;
    uint8_t *out = NULL;
    uint32_t out_len;
    uint32_t out_frame_len;
    uint32_t cp_buffer_frames_max = 0;
    uint32_t check_sum = 0;
    cp_buffer_frames_max = app_bt_stream_get_dma_buffer_samples()/2;
    if (cp_buffer_frames_max %(a2dp_audio_sbc_lastframe_info.frame_samples) ){
        cp_buffer_frames_max =  cp_buffer_frames_max /(a2dp_audio_sbc_lastframe_info.frame_samples) +1  ;
    }else{
        cp_buffer_frames_max =  cp_buffer_frames_max /(a2dp_audio_sbc_lastframe_info.frame_samples) ;
    }

    out_frame_len = sizeof(*p_out_info) + buffer_bytes;
    ret = a2dp_cp_decoder_init(out_frame_len, cp_buffer_frames_max * 2);
    if (ret){
        TRACE_A2DP_DECODER_W("[SBC][INIT] cp_decoder_init() failed: ret=%d", ret);
        set_cp_reset_flag(true);
        return A2DP_DECODER_DECODE_ERROR;
    }

    while ((node = a2dp_audio_list_begin(list)) != NULL) {
        sbc_decoder_frame = (a2dp_audio_sbc_decoder_frame_t *)a2dp_audio_list_node(node);

        in_info.sequenceNumber = sbc_decoder_frame->sequenceNumber;
        in_info.timestamp = sbc_decoder_frame->timestamp;
        in_info.curSubSequenceNumber = sbc_decoder_frame->curSubSequenceNumber;
        in_info.totalSubSequenceNumber = sbc_decoder_frame->totalSubSequenceNumber;
        in_info.chnl_sel = a2dp_audio_context_p->chnl_sel;
        ret = a2dp_cp_put_in_frame(&in_info, sizeof(in_info), sbc_decoder_frame->sbc_buffer, sbc_decoder_frame->sbc_buffer_len);
        if (ret) {
            TRACE_A2DP_DECODER_D("[MCU][SBC] piff !!!!!!ret: %d ", ret);
            break;
        }
        check_sum = a2dp_audio_decoder_internal_check_sum_generate(
                        sbc_decoder_frame->sbc_buffer, sbc_decoder_frame->sbc_buffer_len);

        a2dp_audio_list_remove(list, sbc_decoder_frame);
    }
    ret = a2dp_cp_get_full_out_frame((void **)&out, &out_len);
    if (ret) {
        if (!get_in_cp_frame_cnt()){
            TRACE_A2DP_DECODER_I("[MCU][SBC] cp cache underflow list:%d in_cp:%d",a2dp_audio_list_length(list), get_in_cp_frame_cnt());
            return A2DP_DECODER_CACHE_UNDERFLOW_ERROR;
        }
        if (!a2dp_audio_sysfreq_boost_running()){
            a2dp_audio_sysfreq_boost_start(1);
        }

        uint8_t delayCount = 0;
        do
        {
            osDelay(2);
            ret = a2dp_cp_get_full_out_frame((void **)&out, &out_len);
            if (ret) {
                if (delayCount >= 3)
                {
                    TRACE_A2DP_DECODER_I("[MCU][SBC] cp cache underflow list:%d in_cp:%d",a2dp_audio_list_length(list), get_in_cp_frame_cnt());
                    a2dp_cp_sbc_after_cache_underflow();
                    return A2DP_DECODER_CACHE_UNDERFLOW_ERROR;
                }
            }
            else
            {
                break;
            }

            delayCount++;
        } while (1);

    }

    if (out_len == 0) {
        TRACE_A2DP_DECODER_I("[MCU][SBC]  olz!!!%d ",__LINE__);
        memset(buffer, 0, buffer_bytes);
        a2dp_cp_consume_full_out_frame();
        return A2DP_DECODER_NO_ERROR;
    }
    if(out_len != out_frame_len){
         TRACE_A2DP_DECODER_I("[MCU][SBC] Bad out len %u (should be %u)", out_len, out_frame_len);
         set_cp_reset_flag(true);
         return A2DP_DECODER_DECODE_ERROR;
    }
    p_out_info = (struct A2DP_CP_SBC_OUT_FRM_INFO_T *)out;
    if (p_out_info->pcm_len) {
        a2dp_audio_sbc_lastframe_info.chnl_sel = p_out_info->in_info.chnl_sel;
        a2dp_audio_sbc_lastframe_info.sequenceNumber = p_out_info->in_info.sequenceNumber;
        a2dp_audio_sbc_lastframe_info.timestamp = p_out_info->in_info.timestamp;
        a2dp_audio_sbc_lastframe_info.curSubSequenceNumber = p_out_info->in_info.curSubSequenceNumber;
        a2dp_audio_sbc_lastframe_info.totalSubSequenceNumber = p_out_info->in_info.totalSubSequenceNumber;
        a2dp_audio_sbc_lastframe_info.frame_samples = p_out_info->frame_samples;
        a2dp_audio_sbc_lastframe_info.decoded_frames += p_out_info->decoded_frames;
        a2dp_audio_sbc_lastframe_info.undecode_frames =
            a2dp_audio_list_length(list) + a2dp_cp_get_in_frame_cnt_by_index(p_out_info->frame_idx) - 1;
        a2dp_audio_sbc_lastframe_info.check_sum= check_sum?check_sum:a2dp_audio_sbc_lastframe_info.check_sum;
        a2dp_audio_decoder_internal_lastframe_info_set(&a2dp_audio_sbc_lastframe_info);
    }

    if (p_out_info->pcm_len == buffer_bytes) {
        memcpy(buffer, p_out_info + 1, p_out_info->pcm_len);
        dec_ret = A2DP_DECODER_NO_ERROR;
    } else {
        TRACE_A2DP_DECODER_I("[MCU][SBC] line:%d cp decoder error  !!!!!!", __LINE__);
        set_cp_reset_flag(true);
        return A2DP_DECODER_DECODE_ERROR;
    }

    ret = a2dp_cp_consume_full_out_frame();
    if(ret){
        TRACE_A2DP_DECODER_I("[MCU][SBC] cp_consume_full_out_frame failed: ret=%d",  ret);
        set_cp_reset_flag(true);
        return A2DP_DECODER_DECODE_ERROR;
    }
    return dec_ret;
}


#ifdef __CP_EXCEPTION_TEST__
static bool  _cp_assert = false;
 int cp_assert_sbc(void)
{
    _cp_assert = true;
    return 0;
}
#endif

#if defined(A2DP_SBC_PLC_CALC_MIPS)
uint32_t cp_decode_ticks;
uint32_t cp_decode_ticks_pre;
uint32_t cp_decode_us;
uint32_t bad_start_ticks;
uint32_t bad_end_ticks;
uint32_t bad_used_us;
#endif

#if defined(__VIRTUAL_SURROUND_CP__)
#include "stereo_process.h"
extern "C" StereoSurroundState *stereo_surround_st;
#endif
TEXT_SBC_LOC
static int a2dp_cp_sbc_cp_decode(void)
{
#if defined(A2DP_SBC_PLC_CALC_MIPS)
    cp_decode_ticks = hal_fast_sys_timer_get();
    cp_decode_us = FAST_TICKS_TO_US(cp_decode_ticks - cp_decode_ticks_pre);
    TRACE(0,"[%s] cp decode period = %d", __func__, cp_decode_us);
    cp_decode_ticks_pre = cp_decode_ticks;
#endif
    int ret = 0;
    enum CP_EMPTY_OUT_FRM_T out_frm_st;
    uint8_t *out = NULL;
    uint32_t out_len = 0;
    uint8_t *dec_start = NULL;
    uint32_t dec_len = 0;
    struct A2DP_CP_SBC_IN_FRM_INFO_T *p_in_info = NULL;
    struct A2DP_CP_SBC_OUT_FRM_INFO_T *p_out_info = NULL;
    uint8_t *in_buf = NULL;
    uint32_t in_len = 0;
    sbc_decoder_t *sbc_decoder = NULL;
    pcm_frame_t *pcm_data = NULL;
    int decoder_err = 0;
    int error = 0;
#if defined(A2DP_SBC_PLC_ENABLED)
    static int frame_len = SBC_PCMLEN_DEFAULT;
#endif
    if (cp_codec_reset) {
        cp_codec_reset = false;
        sbc_codec_init();
    }

#ifdef  __CP_EXCEPTION_TEST__
        if (_cp_assert){
        _cp_assert = false;
        *(int*) 0 = 1;
             //ASSERT_A2DP_DECODER(0, "ASSERT_A2DP_DECODER  %s %d", __func__, __LINE__);
        }
#endif
    sbc_decoder = a2dp_audio_sbc_decoder.sbc_decoder;
    pcm_data = a2dp_audio_sbc_decoder.pcm_data;
    out_frm_st = a2dp_cp_get_emtpy_out_frame((void **)&out, &out_len);
    if (out_frm_st != CP_EMPTY_OUT_FRM_OK && out_frm_st != CP_EMPTY_OUT_FRM_WORKING) {
        return 1;
    }
    ASSERT_A2DP_DECODER(out_len > sizeof(*p_out_info), "[CP][SBC] Bad out_len %u (should > %u)", out_len, sizeof(*p_out_info));

    p_out_info = (struct A2DP_CP_SBC_OUT_FRM_INFO_T *)out;
    if (out_frm_st == CP_EMPTY_OUT_FRM_OK) {
        p_out_info->pcm_len = 0;
        p_out_info->decoded_frames = 0;
    }
    ASSERT_A2DP_DECODER(out_len > sizeof(*p_out_info) + p_out_info->pcm_len, "[CP][SBC] Bad out_len %u (should > %u + %u)", out_len, sizeof(*p_out_info), p_out_info->pcm_len);

    dec_start = (uint8_t *)(p_out_info + 1) + p_out_info->pcm_len;
    dec_len = out_len - (dec_start - (uint8_t *)out);
#if defined(__VIRTUAL_SURROUND_CP__)
    uint8_t *surround_start = (uint8_t *)(p_out_info + 1) + p_out_info->pcm_len;
#endif
    pcm_data->pcm_data = (int16_t *)dec_start;
    pcm_data->valid_size = 0;
    pcm_data->buffer_size = dec_len;
    error = 0;

    sbc_decoder_channel_select_e chnl_sel = (sbc_decoder_channel_select_e)a2dp_audio_context_p->chnl_sel;
    if(sbc_chnl_mode_mono){
        chnl_sel = SBC_DECODER_CHANNEL_SELECT_LCHNL;
    }
    sbc_decoder_set(sbc_decoder, SBC_SELECT_CHANNEL_MODE, &chnl_sel);
    while (pcm_data->valid_size < dec_len && error == 0) {
        ret = a2dp_cp_get_in_frame((void **)&in_buf, &in_len);
        if (ret == 0) {
            ASSERT_A2DP_DECODER(in_len > sizeof(*p_in_info), "%s: Bad in_len %u (should > %u)", __func__, in_len, sizeof(*p_in_info));

            p_in_info = (struct A2DP_CP_SBC_IN_FRM_INFO_T *)in_buf;
            in_buf += sizeof(*p_in_info);
            in_len -= sizeof(*p_in_info);

#if defined(A2DP_SBC_PLC_ENABLED)
            int16_t *decoded_buf = (int16_t *)&pcm_data->pcm_data[pcm_data->valid_size/2];
            uint32_t decoded_offset = pcm_data->valid_size;
            int ch_num = sbc_chnl_mode_mono ? 1 : 2;
#endif

#if defined(A2DP_SBC_PLC_ENABLED)
            if (p_in_info->timestamp != UINT32_MAX)
#else
            if (1)
#endif
            {
                sbc_frame_t sbc_data;
                sbc_data.sbc_data = in_buf,
                sbc_data.buffer_size = in_len;
                sbc_data.valid_size = in_len;
                a2dp_audio_sbc_channel_select(p_in_info->chnl_sel);
                decoder_err = sbc_decoder_process_frame(sbc_decoder, &sbc_data, pcm_data);
                switch (decoder_err)
                {
                    case BT_STS_SUCCESS:
                    case BT_STS_CONTINUE:
#if defined(A2DP_SBC_PLC_ENABLED)
                        if (pcm_data->valid_size > decoded_offset) {
                            if(chnl_sel == 0) {
                                a2dp_plc_good_frame(sbc_plc_state0, decoded_buf, decoded_buf, cos_buf, SBC_SMOOTH_LENGTH, 2, 0);
                                a2dp_plc_good_frame(sbc_plc_state1, decoded_buf, decoded_buf, cos_buf, SBC_SMOOTH_LENGTH, 2, 1);
                            } else {
                                a2dp_plc_good_frame(sbc_plc_state0, decoded_buf, decoded_buf, cos_buf, SBC_SMOOTH_LENGTH, ch_num, chnl_sel - 2);
                            }
                            // TRACE(0,"[%s] PLC good frame len %d %d", __func__, pcm_data->valid_size, decoded_offset);
                        }
#endif
                        break;
                    case BT_STS_NO_RESOURCES:
                        error = 1;
                        ASSERT_A2DP_DECODER(0, "sbc_decode BT_STS_NO_RESOURCES pcm has no more buffer, i think can't reach here");
                        break;
                    case BT_STS_FAILED:
                    default:
                        error = 1;
                        sbc_codec_init();
                        break;
                }
            }
#if defined(A2DP_SBC_PLC_ENABLED)
            else
            {
            #if defined(A2DP_SBC_PLC_CALC_MIPS)
                bad_start_ticks = hal_fast_sys_timer_get();
            #endif
            if(chnl_sel == 0) {
                a2dp_plc_bad_frame(sbc_plc_state0, decoded_buf, decoded_buf, cos_buf, SBC_SMOOTH_LENGTH, 2, 0);
                a2dp_plc_bad_frame(sbc_plc_state1, decoded_buf, decoded_buf, cos_buf, SBC_SMOOTH_LENGTH, 2, 1);
            } else {
                a2dp_plc_bad_frame(sbc_plc_state0, decoded_buf, decoded_buf, cos_buf, SBC_SMOOTH_LENGTH, ch_num, chnl_sel - 2);
            }
            #if defined(A2DP_SBC_PLC_CALC_MIPS)
                bad_end_ticks = hal_fast_sys_timer_get();
                bad_used_us = FAST_TICKS_TO_US(bad_end_ticks - bad_start_ticks);
                TRACE(0,"[%s] plc bad frame period = %d", __func__, bad_used_us);
            #endif
                TRACE(0,"[%s] PLC bad frame len %d", __func__, frame_len);
                pcm_data->valid_size += frame_len;
            }
#endif
            sbc_stream_info_t info;
            sbc_decoder_get_stream_info(sbc_decoder, &info);
            memmove(&p_out_info->in_info, p_in_info, sizeof(*p_in_info));
            p_out_info->decoded_frames++;
            p_out_info->frame_samples = info.pcm_samples;
#if defined(A2DP_SBC_PLC_ENABLED)
            frame_len = info.pcm_samples * info.num_channels;
#endif
            p_out_info->frame_idx = a2dp_cp_get_in_frame_index();

            ret = a2dp_cp_consume_in_frame();
            ASSERT_A2DP_DECODER(ret == 0, "%s: a2dp_cp_consume_in_frame() failed: ret=%d", __func__, ret);
        }
        else
        {
            p_out_info->pcm_len += pcm_data->valid_size;
            return 4;
        }
    }

    p_out_info->pcm_len += pcm_data->valid_size;

    if (error || out_len <= sizeof(*p_out_info) + p_out_info->pcm_len) {
#if defined(__VIRTUAL_SURROUND_CP__)
        if(stereo_surround_st != NULL){
            stereo_surround_preprocess_cp(stereo_surround_st,surround_start, pcm_data->valid_size/sizeof(int16_t));
        }
#endif

        ret = a2dp_cp_consume_emtpy_out_frame();
        ASSERT_A2DP_DECODER(ret == 0, "%s: a2dp_cp_consume_emtpy_out_frame() failed: ret=%d", __func__, ret);
    }

    return error;
}
#endif

static int a2dp_audio_sbc_list_checker(void)
{
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    a2dp_audio_sbc_decoder_frame_t *sbc_decoder_frame = NULL;
    int cnt = 0;

    do {
        sbc_decoder_frame = (a2dp_audio_sbc_decoder_frame_t *)a2dp_audio_sbc_subframe_malloc(SBC_LIST_SAMPLES);
        if (sbc_decoder_frame){
            a2dp_audio_list_append(list, sbc_decoder_frame);
        }
        cnt++;
    }while(sbc_decoder_frame && cnt < SBC_MTU_LIMITER);

    do {
        node = a2dp_audio_list_begin(list);
        if (node){
            sbc_decoder_frame = (a2dp_audio_sbc_decoder_frame_t *)a2dp_audio_list_node(node);
            a2dp_audio_list_remove(list, sbc_decoder_frame);
        }
    }while(node);

    TRACE_A2DP_DECODER_I("[SBC][INIT] cnt:%d list:%d", cnt, a2dp_audio_list_length(list));

    return 0;
}

int a2dp_audio_sbc_init(A2DP_AUDIO_OUTPUT_CONFIG_T *config, void *context)
{
    TRACE_A2DP_DECODER_I("[SBC][INIT]");

    a2dp_audio_context_p = (A2DP_AUDIO_CONTEXT_T *)context;
#if !defined(BT_DONT_PLAY_MUTE_WHEN_A2DP_STUCK_PATCH)
    a2dp_audio_sbc_header_parser_init();
#endif
    memset(&a2dp_audio_sbc_lastframe_info, 0, sizeof(A2DP_AUDIO_DECODER_LASTFRAME_INFO_T));
    a2dp_audio_sbc_lastframe_info.chnl_sel = a2dp_audio_context_p->chnl_sel;
    a2dp_audio_sbc_lastframe_info.stream_info = *config;
    a2dp_audio_sbc_lastframe_info.frame_samples= SBC_LIST_SAMPLES;
    a2dp_audio_sbc_lastframe_info.list_samples = SBC_LIST_SAMPLES;
    a2dp_audio_decoder_internal_lastframe_info_set(&a2dp_audio_sbc_lastframe_info);

    ASSERT_A2DP_DECODER(a2dp_audio_context_p->dest_packet_mut < SBC_MTU_LIMITER,
            "%s MTU OVERFLOW:%u/%u", __func__, a2dp_audio_context_p->dest_packet_mut, SBC_MTU_LIMITER);

    a2dp_audio_sbc_decoder.sbc_decoder = (sbc_decoder_t *)
#if defined(A2DP_CP_ACCEL)&&!defined(UNIFY_HEAP_ENABLED)
        a2dp_cp_heap_malloc(sizeof(sbc_decoder_t));
#else
        a2dp_audio_heap_malloc(sizeof(sbc_decoder_t));
#endif
    a2dp_audio_sbc_decoder.pcm_data = (pcm_frame_t *)a2dp_audio_heap_malloc(sizeof(pcm_frame_t));
    a2dp_audio_sbc_decoder_preparse = (sbc_decoder_t *)a2dp_audio_heap_malloc(sizeof(sbc_decoder_t));
#ifdef A2DP_CP_ACCEL
    int ret;
    cp_codec_reset = true;
    ret = a2dp_cp_init(a2dp_cp_sbc_cp_decode, CP_PROC_DELAY_2_FRAMES);
    ASSERT_A2DP_DECODER(ret == 0, "%s: a2dp_cp_init() failed: ret=%d", __func__, ret);
#else
    sbc_codec_init();
#endif
    a2dp_audio_sbc_list_checker();
    sbc_chnl_mode_mono = false;
#if defined(A2DP_SBC_PLC_ENABLED)
    cos_buf = (float *)a2dp_audio_heap_malloc((SBC_SMOOTH_LENGTH*4)*sizeof(float));
    cos_generate(cos_buf, SBC_SMOOTH_LENGTH*4, SBC_SMOOTH_LENGTH);
    sbc_plc_state0 = (struct PLC_State*)a2dp_audio_heap_malloc(sizeof(struct PLC_State));
    a2dp_plc_init(sbc_plc_state0, A2DP_PLC_CODEC_TYPE_SBC);
    if(a2dp_audio_context_p->chnl_sel == 0) {
        sbc_plc_state1 = (struct PLC_State*)a2dp_audio_heap_malloc(sizeof(struct PLC_State));
        a2dp_plc_init(sbc_plc_state0, A2DP_PLC_CODEC_TYPE_SBC);
    }
#endif

    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_sbc_deinit(void)
{
#ifdef A2DP_CP_ACCEL
    a2dp_cp_deinit();
#endif
    a2dp_audio_heap_free(a2dp_audio_sbc_decoder_preparse);
    a2dp_audio_heap_free(a2dp_audio_sbc_decoder.pcm_data);
#if defined(A2DP_CP_ACCEL)&&!defined(UNIFY_HEAP_ENABLED)
    a2dp_cp_heap_free(a2dp_audio_sbc_decoder.sbc_decoder);
#else
    a2dp_audio_heap_free(a2dp_audio_sbc_decoder.sbc_decoder);
#endif
    a2dp_audio_sbc_decoder_preparse = NULL;
    memset(&a2dp_audio_sbc_decoder, 0, sizeof(a2dp_audio_sbc_decoder));

#if defined(A2DP_SBC_PLC_ENABLED)
    a2dp_audio_heap_free(cos_buf);
    a2dp_audio_heap_free(sbc_plc_state0);
    if(a2dp_audio_context_p->chnl_sel == 0) {
        a2dp_audio_heap_free(sbc_plc_state1);
    }
#endif
    TRACE_A2DP_DECODER_I("[SBC][DEINIT]");

    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_sbc_mcu_decode_frame(uint8_t *buffer, uint32_t buffer_bytes)
{
    int ret = SBC_OK;
    sbc_decoder_t *sbc_decoder = NULL;
    pcm_frame_t *pcm_data = NULL;

    uint16_t frame_pcmbyte = 0;
    uint16_t pcm_output_byte = 0;
    bool cache_underflow = false;

    sbc_decoder = a2dp_audio_sbc_decoder.sbc_decoder;
    pcm_data = a2dp_audio_sbc_decoder.pcm_data;

    a2dp_audio_sbc_decoder_frame_t *sbc_decoder_frame = NULL;
    list_node_t *node = NULL;

    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;

    pcm_data->pcm_data = (int16_t *)buffer;
    pcm_data->valid_size = 0;
    pcm_data->buffer_size = buffer_bytes;

    TRACE_A2DP_DECODER_D("[MCU][SBC] size:%d", a2dp_audio_list_length(list));
    for (pcm_output_byte = 0; pcm_output_byte<buffer_bytes; pcm_output_byte += frame_pcmbyte){
        node = a2dp_audio_list_begin(list);

        if (node){
            sbc_decoder_frame = (a2dp_audio_sbc_decoder_frame_t *)a2dp_audio_list_node(node);
            sbc_frame_t sbc_data;
            sbc_data.sbc_data = sbc_decoder_frame->sbc_buffer,
            sbc_data.buffer_size = sbc_decoder_frame->sbc_buffer_len;
            sbc_data.valid_size = sbc_decoder_frame->sbc_buffer_len;
            a2dp_audio_sbc_channel_select(a2dp_audio_context_p->chnl_sel);
            ret = sbc_decoder_process_frame(sbc_decoder, &sbc_data, pcm_data);
            int used = sbc_data.buffer_size - sbc_data.valid_size;
            TRACE_A2DP_DECODER_D("[MCU][SBC] seq:%d/%d/%d len:%d ret:%d used:%d",
                                  sbc_decoder_frame->curSubSequenceNumber,
                                  sbc_decoder_frame->totalSubSequenceNumber,
                                  sbc_decoder_frame->sequenceNumber,
                                  sbc_decoder_frame->sbc_buffer_len,
                                  ret,
                                  used);
            sbc_stream_info_t info;
            sbc_decoder_get_stream_info(sbc_decoder, &info);
            frame_pcmbyte = info.pcm_samples * 2;
            a2dp_audio_sbc_lastframe_info.chnl_sel = a2dp_audio_context_p->chnl_sel;
            a2dp_audio_sbc_lastframe_info.sequenceNumber = sbc_decoder_frame->sequenceNumber;
            a2dp_audio_sbc_lastframe_info.timestamp = sbc_decoder_frame->timestamp;
            a2dp_audio_sbc_lastframe_info.curSubSequenceNumber = sbc_decoder_frame->curSubSequenceNumber;
            a2dp_audio_sbc_lastframe_info.totalSubSequenceNumber = sbc_decoder_frame->totalSubSequenceNumber;
            a2dp_audio_sbc_lastframe_info.frame_samples = info.pcm_samples;
            a2dp_audio_sbc_lastframe_info.decoded_frames++;
            a2dp_audio_sbc_lastframe_info.undecode_frames = a2dp_audio_list_length(list)-1;
            a2dp_audio_sbc_lastframe_info.check_sum = a2dp_audio_decoder_internal_check_sum_generate(
                                                            sbc_decoder_frame->sbc_buffer,
                                                            sbc_decoder_frame->sbc_buffer_len);
            a2dp_audio_decoder_internal_lastframe_info_set(&a2dp_audio_sbc_lastframe_info);
            a2dp_audio_list_remove(list, sbc_decoder_frame);
            switch (ret)
            {
                case BT_STS_SUCCESS:
                    if (pcm_data->valid_size != buffer_bytes){
                        TRACE_A2DP_DECODER_W("[MCU][SBC] WARNING pcm buff mismatch %d/%d", pcm_data->valid_size, buffer_bytes);
                    }
                    if (pcm_data->valid_size == buffer_bytes){
                        if (pcm_output_byte+frame_pcmbyte != buffer_bytes){
                            TRACE_A2DP_DECODER_W("[MCU][SBC]WARNING loop not break %d/%d frame_pcm:%d", pcm_output_byte, buffer_bytes, frame_pcmbyte);
                            goto exit;
                        }
                    }
                    break;
                case BT_STS_CONTINUE:
                    continue;
                    break;
                case BT_STS_NO_RESOURCES:
                    ASSERT_A2DP_DECODER(0, "sbc_decode BT_STS_NO_RESOURCES pcm has no more buffer, i think can't reach here");
                    break;
                case BT_STS_FAILED:
                default:
                    sbc_codec_init();
                    goto exit;
            }
        }else{
            TRACE_A2DP_DECODER_W("[MCU][SBC] A2DP PACKET CACHE UNDERFLOW");
            ret = BT_STS_FAILED;
            cache_underflow = true;
            goto exit;
        }

    }
exit:
    if (cache_underflow){
        TRACE_A2DP_DECODER_W("[MCU][SBC] A2DP PACKET CACHE UNDERFLOW need add some process");
        a2dp_audio_sbc_lastframe_info.undecode_frames = 0;
        a2dp_audio_sbc_lastframe_info.check_sum = 0;
        a2dp_audio_decoder_internal_lastframe_info_set(&a2dp_audio_sbc_lastframe_info);
        ret = A2DP_DECODER_CACHE_UNDERFLOW_ERROR;
    }
    return ret;
}

int a2dp_audio_sbc_decode_frame(uint8_t *buffer, uint32_t buffer_bytes)
{
    int nRet = 0;
    if (sbc_chnl_mode_mono){
        int i = 0;
        int16_t *src = NULL,*dest = NULL;
#ifdef A2DP_CP_ACCEL
        nRet = a2dp_cp_sbc_mcu_decode(buffer, buffer_bytes/2);
#else
        nRet = a2dp_audio_sbc_mcu_decode_frame(buffer, buffer_bytes/2);
#endif
        i = buffer_bytes / 2;
        dest = (int16_t *)buffer + i - 1;
        i = i / 2;
        src = (int16_t *)buffer + i - 1;
        for (; i>=0; i--){
            *dest = *src;
            dest--;
            *dest = *src;
            dest--;
            src--;
        }
    }else{
#ifdef A2DP_CP_ACCEL
        nRet = a2dp_cp_sbc_mcu_decode(buffer, buffer_bytes);
#else
        nRet = a2dp_audio_sbc_mcu_decode_frame(buffer, buffer_bytes);
#endif
    }
    return  nRet;
}

int a2dp_audio_sbc_preparse_packet(btif_media_header_t * header, uint8_t *buffer, uint32_t buffer_bytes)
{
    uint16_t bytes_parsed = 0;
    uint32_t frame_num = 0;
    uint8_t *parser_p = buffer;

    frame_num = *parser_p;
    parser_p++;
    buffer_bytes--;

    // TODO: Remove the following sbc init and decode codes. They might conflict with the calls
    //       during CP process. CP process is triggered by audioflinger PCM callback.

    sbc_stream_info_t info;
    memset(&info, 0, sizeof(sbc_stream_info_t));
    if (*parser_p != 0x9c){
        TRACE_A2DP_DECODER_I("[SBC][PRE] ERROR SBC FRAME !!! frame_num:%d", frame_num);
        DUMP8("%02x ", parser_p, 12);
    }else{
        sbc_frame_t sbc_data;
        sbc_data.sbc_data = parser_p;
        sbc_data.buffer_size = buffer_bytes;
        sbc_data.valid_size = buffer_bytes;
        sbc_decoder_parser_frame(a2dp_audio_sbc_decoder_preparse, &sbc_data);
        bytes_parsed = sbc_data.buffer_size - sbc_data.valid_size;
        sbc_decoder_get_stream_info(a2dp_audio_sbc_decoder_preparse, &info);
        TRACE_A2DP_DECODER_I("[SBC][PRE] seq:%d tStmp:%08x smpR:%d ch:%d len:%d",
                             header->sequenceNumber,
                             header->timestamp,
                             info.sample_rate,
                             info.channel_mode,
                             info.pcm_samples);
        TRACE_A2DP_DECODER_I("[SBC][PRE] frmN:%d par:%d/%d", frame_num, buffer_bytes, bytes_parsed);

        a2dp_audio_sbc_lastframe_info.sequenceNumber = header->sequenceNumber;
        a2dp_audio_sbc_lastframe_info.timestamp = header->timestamp;
        a2dp_audio_sbc_lastframe_info.curSubSequenceNumber = 0;
        a2dp_audio_sbc_lastframe_info.totalSubSequenceNumber = frame_num;
        a2dp_audio_sbc_lastframe_info.frame_samples = info.pcm_samples;
        a2dp_audio_sbc_lastframe_info.list_samples = SBC_LIST_SAMPLES;
        a2dp_audio_sbc_lastframe_info.decoded_frames = 0;
        a2dp_audio_sbc_lastframe_info.undecode_frames = 0;
        a2dp_audio_decoder_internal_lastframe_info_set(&a2dp_audio_sbc_lastframe_info);
        if (info.channel_mode == SBC_CHANNEL_MODE_MONO){
            sbc_chnl_mode_mono = true;
        }
    }
    return A2DP_DECODER_NO_ERROR;
}

#if !defined(BT_DONT_PLAY_MUTE_WHEN_A2DP_STUCK_PATCH)
static int a2dp_audio_sbc_header_parser_init(void)
{
    sbc_decoder_last_valid_frame_ready = false;
    memset(&sbc_decoder_last_valid_frame, 0, sizeof(sbc_decoder_last_valid_frame));
    return 0;
}

static int a2dp_audio_sbc_packet_recover_save_last(btif_media_header_t *sbc_decoder_frame)
{
    sbc_decoder_last_valid_frame = *sbc_decoder_frame;
    sbc_decoder_last_valid_frame_ready = true;
    return 0;
}

static int a2dp_audio_sbc_packet_recover_find_missing(btif_media_header_t *sbc_decoder_frame, uint8_t frame_cnt)
{
    uint16_t diff_seq = 0;
    uint32_t diff_timestamp = 0;
    uint32_t need_recover_pkt = 0;

    if (!sbc_decoder_last_valid_frame_ready){
        return need_recover_pkt;
    }

    diff_seq = a2dp_audio_get_passed(sbc_decoder_frame->sequenceNumber, sbc_decoder_last_valid_frame.sequenceNumber, UINT16_MAX);
    diff_timestamp = a2dp_audio_get_passed(sbc_decoder_frame->timestamp, sbc_decoder_last_valid_frame.timestamp, UINT32_MAX);
    if (diff_seq > 1){
        if (diff_timestamp%SBC_LIST_SAMPLES == 0){
            need_recover_pkt = diff_timestamp/SBC_LIST_SAMPLES;
            if (need_recover_pkt > frame_cnt){
                need_recover_pkt -= frame_cnt;
            }
        }else{
            diff_seq--;
            need_recover_pkt = diff_seq * frame_cnt;
        }
#if defined(A2DP_SBC_PLC_ENABLED)
        a2dp_audio_sysfreq_boost_start(diff_seq*3);
#endif
        TRACE_A2DP_DECODER_W("[SBC][INPUT][PLC] seq:%d/%d stmp:%d/%d", sbc_decoder_frame->sequenceNumber, sbc_decoder_last_valid_frame.sequenceNumber,
                                                                                  sbc_decoder_frame->timestamp, sbc_decoder_last_valid_frame.timestamp);
        TRACE_A2DP_DECODER_W("[SBC][INPUT][PLC] diff_seq:%d diff_stmp:%d missing:%d", diff_seq, diff_timestamp, need_recover_pkt);
    }

    return need_recover_pkt;
}

static int a2dp_audio_sbc_packet_recover_proc(btif_media_header_t *sbc_decoder_frame, a2dp_audio_sbc_decoder_frame_t *sbc_raw_frame, uint8_t frame_cnt)
{
    int nRet = A2DP_DECODER_NO_ERROR;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    int missing_pkt_cnt = 0;
    missing_pkt_cnt = a2dp_audio_sbc_packet_recover_find_missing(sbc_decoder_frame, frame_cnt);
    if (missing_pkt_cnt && sbc_raw_frame &&
        (a2dp_audio_list_length(list)+missing_pkt_cnt) < sbc_mtu_limiter){
        for (uint8_t i =0; i<missing_pkt_cnt; i++){
            a2dp_audio_sbc_decoder_frame_t *frame_p = (a2dp_audio_sbc_decoder_frame_t *)a2dp_audio_sbc_subframe_malloc(sbc_raw_frame->sbc_buffer_len);
            if (!frame_p){
                nRet = A2DP_DECODER_MEMORY_ERROR;
                goto exit;
            }
            frame_p->sequenceNumber = UINT16_MAX;
            frame_p->timestamp = UINT32_MAX;
            memcpy(frame_p->sbc_buffer, sbc_raw_frame->sbc_buffer, sbc_raw_frame->sbc_buffer_len);
            frame_p->sbc_buffer_len = sbc_raw_frame->sbc_buffer_len;
            a2dp_audio_list_append(list, frame_p);
        }
    }
exit:
    return nRet;
}

#else
int a2dp_audio_sbc_packet_adjust(a2dp_audio_sbc_decoder_frame_t *sbc_decoder_frame, uint32_t frame_num, uint32_t buffer_bytes)
{
    int nRet = A2DP_DECODER_NO_ERROR;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    a2dp_audio_sbc_decoder_frame_t *frame_p2 = NULL;
    int8_t refill_subframes = 0;

#ifdef IBRT
    refill_subframes = app_tws_ibrt_audio_analysis_get_refill_frames();
    app_tws_ibrt_audio_analysis_update_refill_frames(-refill_subframes);
#endif

    if((refill_subframes != 0)&&(a2dp_audio_context_p->adjust_frame_cnt_after_no_cache ==0))
    {
        a2dp_audio_context_p->adjust_frame_cnt_after_no_cache = refill_subframes;
    }
    if(a2dp_audio_context_p->adjust_frame_cnt_after_no_cache < 0)
    {
        if(a2dp_audio_list_length(list) > 0)
        {
            frame_p2 = (a2dp_audio_sbc_decoder_frame_t *)a2dp_audio_list_front(list);
            if(frame_p2)
            {
                if(a2dp_audio_list_remove(list, frame_p2))
                {
                    a2dp_audio_context_p->adjust_frame_cnt_after_no_cache++;
                    TRACE_A2DP_DECODER_W("[SBC][INPUT][ADJUST] a2dp_audio_context_p->adjust_frame_cnt_after_no_cache=%d\n",a2dp_audio_context_p->adjust_frame_cnt_after_no_cache);
                }
            }
            else
            {
                TRACE_A2DP_DECODER_W("[SBC][INPUT][ADJUST] try next time to a2dp_audio_list_remove front of list !");
            }
        }
    }
    else if(a2dp_audio_context_p->adjust_frame_cnt_after_no_cache > 0)
    {
        if (a2dp_audio_list_length(list) + frame_num < sbc_mtu_limiter)
        {
            list_node_t *node = a2dp_audio_list_begin(list);
            if(node != NULL)
            {
                frame_p2 = (a2dp_audio_sbc_decoder_frame_t *)a2dp_audio_sbc_subframe_malloc(buffer_bytes);
                if(frame_p2 != NULL)
                {
                    frame_p2->sequenceNumber = UINT16_MAX;
                    frame_p2->timestamp = UINT32_MAX;
                    frame_p2->curSubSequenceNumber = sbc_decoder_frame->curSubSequenceNumber;
                    frame_p2->totalSubSequenceNumber = sbc_decoder_frame->totalSubSequenceNumber;
                    memcpy(frame_p2->sbc_buffer, sbc_decoder_frame->sbc_buffer, buffer_bytes);
                    frame_p2->sbc_buffer_len = buffer_bytes;
                    if(a2dp_audio_list_prepend(list, frame_p2))
                    {
                        a2dp_audio_context_p->adjust_frame_cnt_after_no_cache--;
                        TRACE_A2DP_DECODER_W("[SBC][INPUT][ADJUST] a2dp_audio_context.adjust_frame_cnt_after_no_cache=%d\n",
                            a2dp_audio_context_p->adjust_frame_cnt_after_no_cache);
                    }
                }
            }
        }
        else
        {
            nRet = A2DP_DECODER_SYNC_ERROR;
        }
    }
    return nRet;
}
#endif

#define FRAME_LIST_MAX (20)
static int a2dp_audio_sbc_store_packet(btif_media_header_t * header, uint8_t *buffer, uint32_t buffer_bytes)
{
    int nRet = A2DP_DECODER_NO_ERROR;

    uint32_t frame_cnt = 0;
    uint32_t frame_num = 0;
    uint32_t frame_len = 0;
    uint8_t *parser_p = buffer;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    uint16_t bytes_parsed = 0;
    a2dp_audio_sbc_decoder_frame_t *frame_list[FRAME_LIST_MAX] = {0,};
    uint8_t frame_list_idx = 0;
    bool find_err = false;
    uint32_t i = 0;

    frame_num = *parser_p;

    if (!frame_num){
        TRACE_A2DP_DECODER_W("[SBC][INPUT] ERROR SBC FRAME !!! frame_num:%d", frame_num);
        DUMP8("%02x ", parser_p, 12);
        return A2DP_DECODER_DECODE_ERROR;
    }

    parser_p++;
    buffer_bytes--;
    frame_len = buffer_bytes/frame_num;

    if ((a2dp_audio_list_length(list)+frame_num) < sbc_mtu_limiter){
        for (i=0; i<buffer_bytes; i+=bytes_parsed, frame_cnt++){
            bytes_parsed = 0;
            if (*(parser_p+i) == 0x9c){
                sbc_frame_t sbc_data;
                sbc_data.sbc_data = parser_p+i;
                sbc_data.buffer_size = buffer_bytes;
                sbc_data.valid_size = buffer_bytes;
                int ret = sbc_decoder_parser_frame(a2dp_audio_sbc_decoder_preparse, &sbc_data);
                bytes_parsed = sbc_data.buffer_size - sbc_data.valid_size;
                if (ret != SBC_OK){
                    bytes_parsed = frame_len;
                    TRACE_A2DP_DECODER_W("[SBC][INPUT] ERROR SBC FRAME PARSER !!!");
                    DUMP8("%02x ", parser_p+i, 12);
                    find_err = true;
                    break;
                }
                a2dp_audio_sbc_decoder_frame_t *frame_p = (a2dp_audio_sbc_decoder_frame_t *)a2dp_audio_sbc_subframe_malloc(bytes_parsed);
                if (!frame_p){
                    nRet = A2DP_DECODER_MEMORY_ERROR;
                    find_err = true;
                    break;
                }
                frame_p->sequenceNumber = header->sequenceNumber;
                frame_p->timestamp = header->timestamp;
                frame_p->curSubSequenceNumber = frame_cnt;
                frame_p->totalSubSequenceNumber = frame_num;
                memcpy(frame_p->sbc_buffer, (parser_p+i), bytes_parsed);
                frame_p->sbc_buffer_len = bytes_parsed;
#if defined(BT_DONT_PLAY_MUTE_WHEN_A2DP_STUCK_PATCH)
                nRet = a2dp_audio_sbc_packet_adjust(frame_p,frame_num,bytes_parsed);
                if(nRet == A2DP_DECODER_SYNC_ERROR)
                {
                    return nRet;
                }
#endif
                frame_list[frame_list_idx++] = frame_p;
                if (frame_list_idx >= FRAME_LIST_MAX){
                    find_err = true;
                    break;
                }
            }else{
                TRACE_A2DP_DECODER_W("[SBC][INPUT] ERROR SBC FRAME !!!");
                DUMP8("%02x ", parser_p+i, 12);
                find_err = true;
                break;
            }
        }
        if (find_err){
            TRACE_A2DP_DECODER_W("[SBC][INPUT] FIND ERR !!!");
            for (i = 0; i<frame_list_idx; i++){
                a2dp_audio_sbc_subframe_free(frame_list[i]);
            }
            nRet = A2DP_DECODER_DECODE_ERROR;
        }else{
#if !defined(BT_DONT_PLAY_MUTE_WHEN_A2DP_STUCK_PATCH)
            a2dp_audio_sbc_packet_recover_proc(header, frame_list[0], frame_num);
            a2dp_audio_sbc_packet_recover_save_last(header);
#endif
            for (i = 0; i<frame_list_idx; i++){
                a2dp_audio_list_append(list, frame_list[i]);
            }
            nRet = A2DP_DECODER_NO_ERROR;
        }
    }else{
        TRACE_A2DP_DECODER_W("[SBC][INPUT] OVERFLOW list:%d,want_insert=%d,limit=%d", a2dp_audio_list_length(list), frame_num, sbc_mtu_limiter);
        nRet = A2DP_DECODER_MTU_LIMTER_ERROR;
    }

    return nRet;
}

int a2dp_audio_sbc_discards_packet(uint32_t packets)
{
    int nRet = A2DP_DECODER_MEMORY_ERROR;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    a2dp_audio_sbc_decoder_frame_t *sbc_decoder_frame = NULL;
    uint16_t totalSubSequenceNumber;
    uint8_t j = 0;

#ifdef A2DP_CP_ACCEL
    a2dp_cp_reset_frame();
#endif

    node = a2dp_audio_list_begin(list);
    if (!node){
        goto exit;
    }
    sbc_decoder_frame = (a2dp_audio_sbc_decoder_frame_t *)a2dp_audio_list_node(node);

    for (j=0; j<a2dp_audio_list_length(list); j++){
        node = a2dp_audio_list_begin(list);
        if (!node){
            goto exit;
        }
        sbc_decoder_frame = (a2dp_audio_sbc_decoder_frame_t *)a2dp_audio_list_node(node);
        if (sbc_decoder_frame->curSubSequenceNumber != 0){
            a2dp_audio_list_remove(list, sbc_decoder_frame);
        }else{
            break;
        }
    }

    node = a2dp_audio_list_begin(list);
    if (!node){
        goto exit;
    }
    sbc_decoder_frame = (a2dp_audio_sbc_decoder_frame_t *)a2dp_audio_list_node(node);
    ASSERT_A2DP_DECODER(sbc_decoder_frame->curSubSequenceNumber == 0, "sbc_discards_packet not align curSubSequenceNumber:%d",
                                                                          sbc_decoder_frame->curSubSequenceNumber);

    totalSubSequenceNumber = sbc_decoder_frame->totalSubSequenceNumber;

    if (packets <= a2dp_audio_list_length(list)/totalSubSequenceNumber){
        for (uint8_t i=0; i<packets; i++){
            for (j=0; j<totalSubSequenceNumber; j++){
                node = a2dp_audio_list_begin(list);
                if (!node){
                    goto exit;
                }
                sbc_decoder_frame = (a2dp_audio_sbc_decoder_frame_t *)a2dp_audio_list_node(node);
                a2dp_audio_list_remove(list, sbc_decoder_frame);
            }
        }
        nRet = A2DP_DECODER_NO_ERROR;
    }
exit:
    TRACE_A2DP_DECODER_I("[SBC][DISCARDS] packets:%d nRet:%d", packets, nRet);
    return nRet;
}

int a2dp_audio_sbc_headframe_info_get(A2DP_AUDIO_HEADFRAME_INFO_T *headframe_info)
{
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    a2dp_audio_sbc_decoder_frame_t *sbc_decoder_frame = NULL;

    if (a2dp_audio_list_length(list) && ((node = a2dp_audio_list_begin(list)) != NULL)){
        sbc_decoder_frame = (a2dp_audio_sbc_decoder_frame_t *)a2dp_audio_list_node(node);
        headframe_info->sequenceNumber         = sbc_decoder_frame->sequenceNumber;
        headframe_info->timestamp              = sbc_decoder_frame->timestamp;
        headframe_info->curSubSequenceNumber   = sbc_decoder_frame->curSubSequenceNumber;
        headframe_info->totalSubSequenceNumber = sbc_decoder_frame->totalSubSequenceNumber;
    }else{
        memset(headframe_info, 0, sizeof(A2DP_AUDIO_HEADFRAME_INFO_T));
    }

    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_sbc_info_get(void *info)
{
    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_sbc_synchronize_packet(A2DP_AUDIO_SYNCFRAME_INFO_T *sync_info, uint32_t mask)
{
    int nRet = A2DP_DECODER_SYNC_ERROR;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    int list_len;
    a2dp_audio_sbc_decoder_frame_t *sbc_decoder_frame = NULL;

#ifdef A2DP_CP_ACCEL
    a2dp_cp_reset_frame();
#endif

    list_len = a2dp_audio_list_length(list);

    for (uint16_t i=0; i<list_len; i++){
        node = a2dp_audio_list_begin(list);
        if (node){
            sbc_decoder_frame = (a2dp_audio_sbc_decoder_frame_t *)a2dp_audio_list_node(node);
            if  (A2DP_AUDIO_SYNCFRAME_CHK(sbc_decoder_frame->sequenceNumber         == sync_info->sequenceNumber,        A2DP_AUDIO_SYNCFRAME_MASK_SEQ,        mask)&&
                 A2DP_AUDIO_SYNCFRAME_CHK(sbc_decoder_frame->curSubSequenceNumber   == sync_info->curSubSequenceNumber,  A2DP_AUDIO_SYNCFRAME_MASK_CURRSUBSEQ, mask)&&
                 A2DP_AUDIO_SYNCFRAME_CHK(sbc_decoder_frame->totalSubSequenceNumber == sync_info->totalSubSequenceNumber,A2DP_AUDIO_SYNCFRAME_MASK_TOTALSUBSEQ,mask)){
                nRet = A2DP_DECODER_NO_ERROR;
                break;
            }
            a2dp_audio_list_remove(list, sbc_decoder_frame);
        }
    }

    node = a2dp_audio_list_begin(list);
    if (node){
        sbc_decoder_frame = (a2dp_audio_sbc_decoder_frame_t *)a2dp_audio_list_node(node);
        TRACE_A2DP_DECODER_I("[MCU][SYNC][SBC] sync pkt nRet:%d SEQ:%d timestamp:%d %d/%d",
                                                      nRet, sbc_decoder_frame->sequenceNumber, sbc_decoder_frame->timestamp,
                                                      sbc_decoder_frame->curSubSequenceNumber, sbc_decoder_frame->totalSubSequenceNumber);
    }else{
        TRACE_A2DP_DECODER_I("[MCU][SYNC][SBC] sync pkt nRet:%d", nRet);
    }

    return nRet;
}

int a2dp_audio_sbc_synchronize_dest_packet_mut(uint16_t packet_mut)
{
    list_node_t *node = NULL;
    uint32_t list_len = 0;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    a2dp_audio_sbc_decoder_frame_t *sbc_decoder_frame = NULL;

    list_len = a2dp_audio_list_length(list);
    if (list_len > packet_mut){
        do{
            node = a2dp_audio_list_begin(list);
            if (node){
                sbc_decoder_frame = (a2dp_audio_sbc_decoder_frame_t *)a2dp_audio_list_node(node);
                a2dp_audio_list_remove(list, sbc_decoder_frame);
            }
        }while(a2dp_audio_list_length(list) > packet_mut);
    }

    TRACE_A2DP_DECODER_I("[MCU][SYNC][SBC] dest pkt list:%d", a2dp_audio_list_length(list));

    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_sbc_convert_list_to_samples(uint32_t *samples)
{
    uint32_t list_len = 0;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;

    list_len = a2dp_audio_list_length(list);
    *samples = SBC_LIST_SAMPLES*list_len;

    TRACE_A2DP_DECODER_I("[MCU][SBC] list:%d samples:%d", list_len, *samples);

    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_sbc_discards_samples(uint32_t samples)
{
    int nRet = A2DP_DECODER_SYNC_ERROR;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    a2dp_audio_sbc_decoder_frame_t *sbc_decoder_frame = NULL;
    list_node_t *node = NULL;
    int need_remove_list = 0;
    uint32_t list_samples = 0;
    ASSERT_A2DP_DECODER(!(samples%SBC_LIST_SAMPLES), "%s samples err:%d", __func__, samples);

    a2dp_audio_sbc_convert_list_to_samples(&list_samples);
    if (list_samples >= samples){
        need_remove_list = samples/SBC_LIST_SAMPLES;
        for (int i=0; i<need_remove_list; i++){
            node = a2dp_audio_list_begin(list);
            if (node){
                sbc_decoder_frame = (a2dp_audio_sbc_decoder_frame_t *)a2dp_audio_list_node(node);
                a2dp_audio_list_remove(list, sbc_decoder_frame);
            }
        }
        nRet = A2DP_DECODER_NO_ERROR;
    }

    return nRet;
}

int a2dp_audio_sbc_channel_select(A2DP_AUDIO_CHANNEL_SELECT_E chnl_sel)
{
    sbc_decoder_channel_select_e sbc_decoder_channel_select = SBC_DECODER_CHANNEL_SELECT_STEREO;
    if(sbc_chnl_mode_mono){
        sbc_decoder_channel_select = SBC_DECODER_CHANNEL_SELECT_LCHNL;
    }else{
        switch(chnl_sel)
        {
            case A2DP_AUDIO_CHANNEL_SELECT_STEREO:
                sbc_decoder_channel_select = SBC_DECODER_CHANNEL_SELECT_STEREO;
                break;
            case A2DP_AUDIO_CHANNEL_SELECT_LRMERGE:
                sbc_decoder_channel_select = SBC_DECODER_CHANNEL_SELECT_LRMERGE;
                break;
            case A2DP_AUDIO_CHANNEL_SELECT_LCHNL:
                sbc_decoder_channel_select = SBC_DECODER_CHANNEL_SELECT_LCHNL;
               break;
            case A2DP_AUDIO_CHANNEL_SELECT_RCHNL:
                sbc_decoder_channel_select = SBC_DECODER_CHANNEL_SELECT_RCHNL;
            default:
                break;
        }
    }
    sbc_decoder_set(a2dp_audio_sbc_decoder.sbc_decoder, SBC_SELECT_CHANNEL_MODE, &sbc_decoder_channel_select);
    return A2DP_DECODER_NO_ERROR;
}

extern const A2DP_AUDIO_DECODER_T a2dp_audio_sbc_decoder_config = {
    {44100, 2, 16},
    1,{0},
    a2dp_audio_sbc_init,
    a2dp_audio_sbc_deinit,
    a2dp_audio_sbc_decode_frame,
    a2dp_audio_sbc_preparse_packet,
    a2dp_audio_sbc_store_packet,
    a2dp_audio_sbc_discards_packet,
    a2dp_audio_sbc_synchronize_packet,
    a2dp_audio_sbc_synchronize_dest_packet_mut,
    a2dp_audio_sbc_convert_list_to_samples,
    a2dp_audio_sbc_discards_samples,
    a2dp_audio_sbc_headframe_info_get,
    a2dp_audio_sbc_info_get,
    a2dp_audio_sbc_subframe_free,
    a2dp_audio_sbc_channel_select,
};

