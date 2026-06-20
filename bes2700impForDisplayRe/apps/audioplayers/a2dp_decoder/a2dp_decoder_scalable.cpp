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
#include "hal_timer.h"
#include "cmsis.h"
#include "plat_types.h"
#include <string.h>
#include "heap_api.h"
#include "hal_location.h"
#include "a2dp_decoder_internal.h"
#include "app_audio.h"
#if defined(A2DP_SCALABLE_ON)
#include "heap_api.h"
#include "ssc.h"
#include "cmsis_os.h"
#ifdef IBRT
#include "app_ibrt_internal.h"
#include "app_tws_ibrt_audio_analysis.h"
#include "earbud_ux_api.h"
#endif
#define SCALABLE_MTU_LIMITER (28)
#define SCALABLE_MEMPOOL_SIZE  1024

#define SBM_DEBUG_ENABLEDx
#define SBM_ENHANCED_DEBUG_ENABLED
#define DEBUG_CALC_MCPS
#ifdef DEBUG_CALC_MCPS
unsigned int cp_peak_dec_time = 0;
#endif
unsigned int sbm_operation_fast = 0;
unsigned int sbm_error_fast = 0;
unsigned int sbm_operation_slow = 0;
unsigned int sbm_error_slow = 0;

unsigned int frame_cnt_audio = 0;
unsigned int Frame_sequence_mismatch_Cnt;
unsigned int PLC_Frame;
unsigned int FrameCnt;
typedef void *HANDLE_DECODER;
static A2DP_AUDIO_CONTEXT_T *a2dp_audio_context_p = NULL;
static A2DP_AUDIO_OUTPUT_CONFIG_T output_config;
#ifndef A2DP_DECODER_CROSS_CORE

static unsigned char *scalable_decoder_place = NULL;
#endif
static unsigned char *scalable_decoder_temp_buf = NULL;
static short ss_pcm_buff[SCALABLE_MAX_FRAME_SIZE*4];
static int scalable_uhq_flag __attribute__((unused)) = 0;

static HANDLE_DECODER scalableDec_handle = NULL;
static uint32_t currentTmpBufOffset = 0;
static uint32_t currentTmpLeftBytes = 0;

static A2DP_AUDIO_DECODER_LASTFRAME_INFO_T lastframe_info;
#ifndef A2DP_DECODER_CROSS_CORE
static void sbm_operation_env_reset(void);
#endif
static bool sbm_operation_pre_decoding_handler(uint32_t currentSeqNum);

// macro to enable scalable PLC
#define SCALABLE_PLC_ENABLED

#define SCALABLE_UNDERFLOW_PLC_TOLERANCE_TIMES  1
static uint16_t scalable_last_received_a2dp_packet_size = 0;
static uint8_t* scalable_last_received_a2dp_packet_buf_ptr = NULL;
static uint8_t scalable_underflow_plc_count = 0;

// if the gap between two received a2dp packet seq numer is bigger than this value(normal case is 1),
// don't do PLC but directly retrigger the a2dp streaming
#define SCALABLE_NON_CONTINUOUS_SEQ_NUM_TOLERANCE_GAP 3
static uint16_t lastStoredScalablePacketSeqNum = 0;
static bool isFirstScalablePacketReceived = false;
static uint16_t scalable_plc_a2dp_packet_size = 0;
static uint8_t* scalable_plc_a2dp_packet_buf_ptr = NULL;

#ifdef IBRT
uint32_t app_tws_ibrt_audio_analysis_tick_get(void);
uint32_t app_tws_ibrt_audio_analysis_interval_get(void);
extern "C" bool app_tws_ibrt_tws_link_connected(void);
#endif

#ifdef A2DP_DECODER_CROSS_CORE_USE_M55
#include "app_dsp_m55.h"
void audio_audio_scalable_get_sync_custom_data(uint8_t* custom_data);
SBM_STATUS_E a2dp_decoder_bth_get_cc_sbm_status(void);
#endif

extern "C" uint32_t get_in_cp_frame_cnt(void);
extern "C" uint32_t set_cp_reset_flag(uint8_t evt);

typedef struct {
    A2DP_COMMON_MEDIA_FRAME_HEADER_T header;
} a2dp_audio_scalable_decoder_frame_t;

static void ss_to_24bit_buf(int32_t * out, int32_t * in, int size)
{
    memcpy((uint8_t *)out, (uint8_t *)in, sizeof(int32_t)*size);
}

static void a2dp_audio_scalable_decoder_init(void)
{
#ifndef A2DP_DECODER_CROSS_CORE
    if (scalableDec_handle == NULL) {
        scalableDec_handle = scalable_decoder_place;
#ifdef DEBUG_CALC_MCPS
		cp_peak_dec_time = 0;
#endif
        frame_cnt_audio = 0;
        Frame_sequence_mismatch_Cnt = 0;
        PLC_Frame = 0;
        FrameCnt = 0;
        sbm_operation_fast = 0;
        sbm_error_fast = 0;
        sbm_operation_slow = 0;
        sbm_error_slow = 0;
        TRACE(0,"[SSC]ssc_decoder_init channels = %d, sample_rate=%d", output_config.num_channels, output_config.sample_rate);
        ssc_decoder_init(scalableDec_handle, output_config.num_channels, output_config.sample_rate, true, output_config.bits_depth);
        sbm_operation_env_reset();
    }
#endif
}

static int scalableDecoder_Close(HANDLE_DECODER handle)
{
    if (handle) {
    a2dp_audio_heap_free(handle);
    a2dp_audio_heap_free(scalable_decoder_temp_buf);
    }

    return 0;
}

static void a2dp_audio_scalable_decoder_deinit(void)
{
    if (scalableDec_handle) {
    scalableDecoder_Close(scalableDec_handle);
    scalableDec_handle = NULL;
    }
}

static void a2dp_audio_scalable_decoder_reinit(void)
{
    TRACE(0,"[SSC]a2dp_audio_scalable_decoder_reinit()");
    if (scalableDec_handle) {
    a2dp_audio_scalable_decoder_deinit();
    }
    a2dp_audio_scalable_decoder_init();
}

static bool is_valid_frame(a2dp_audio_scalable_decoder_frame_t * decoder_frame_p)
{
    uint32_t hw_tmp, len, bitrate_bps, frame_len, frame_size;

    int sampling_rate = 44100;
    unsigned char *input_buf = decoder_frame_p->header.ptrData;

    if ((0xFF != input_buf[0]) || (0xE0 != (input_buf[1] & 0xF0)))
    {
        TRACE_A2DP_DECODER_E("invalid scalable a2dp frame, sync words not right.");
        return false;
    }

    if (decoder_frame_p->header.dataLen < SCALABLE_HEAD_SIZE) {
        TRACE_A2DP_DECODER_E("invalid scalable a2dp frame, length < SCALABLE_HEAD_SIZE !!!!!!!");
        return false;
    }

    scalable_uhq_flag = 0;

    switch ((input_buf[3]&0xf7)) {
        case 0xF0:
            bitrate_bps = 88000;
            break;
        case 0xF1:
            bitrate_bps = 96000;
            break;
        case 0xF2:
            bitrate_bps = 128000;
            break;
        case 0xF3:
            bitrate_bps = 192000;
            break;
        case 0xF4:
            bitrate_bps = 229000;
            break;
        case 0xF5:
            scalable_uhq_flag = 1;
            bitrate_bps = 328000;
            sampling_rate = 96000;
            break;
        default:
            bitrate_bps = 192000;
            break;
    }

    frame_size = SCALABLE_FRAME_SIZE;

    len = bitrate_bps * frame_size / sampling_rate / 8;
    if (scalable_uhq_flag == 0) {
    hw_tmp = (len * 3) >> 7;
    len = hw_tmp + len;
    len = len + ((len & 1) ^ 1);
    } else {
    len = 369;      //744/2-4+1
    }
    TRACE_A2DP_DECODER_D
    ("scalable a2dp frame, length:%d bitrate:%d sampling_rate:%d",
     decoder_frame_p->header.dataLen, bitrate_bps, sampling_rate);
    frame_len = SCALABLE_HEAD_SIZE + len - 1;

    if (decoder_frame_p->header.dataLen < frame_len) {
    TRACE_A2DP_DECODER_E
        ("invalid scalable a2dp frame, length:%d  <  %d !!!!!!!", decoder_frame_p->header.dataLen, frame_len);
    return false;
    }
    return true;

}


#ifdef A2DP_CP_ACCEL
struct A2DP_CP_scalable_IN_FRM_INFO_T {
    uint16_t sequenceNumber;
    uint32_t timestamp;
#ifdef SCALABLE_PLC_ENABLED
    uint8_t isPLC;
#endif
};

struct A2DP_CP_scalable_OUT_FRM_INFO_T {
    struct A2DP_CP_scalable_IN_FRM_INFO_T in_info;
    uint16_t frame_samples;
    uint16_t decoded_frames;
    uint16_t frame_idx;
    uint16_t pcm_len;
    uint16_t fetchOffset;
};

static bool cp_codec_reset;
extern uint32_t app_bt_stream_get_dma_buffer_samples(void);

static int TEXT_AAC_LOC a2dp_cp_scalable_lc_after_cache_underflow(void)
{
#ifdef A2DP_CP_ACCEL
    cp_codec_reset = true;
#endif
    return 0;
}

TEXT_SSC_LOC static int a2dp_cp_scalable_mcu_decode(uint8_t * buffer, uint32_t buffer_bytes)
{
    a2dp_audio_scalable_decoder_frame_t *decoder_frame_p = NULL;
    list_node_t *node = NULL;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    int ret, dec_ret;
    struct A2DP_CP_scalable_IN_FRM_INFO_T in_info;
    struct A2DP_CP_scalable_OUT_FRM_INFO_T *p_out_info;
    uint8_t *out;
    uint32_t out_len;
    uint32_t out_frame_len;
    uint32_t cp_buffer_frames_max = 0;

    cp_buffer_frames_max = app_bt_stream_get_dma_buffer_samples()/2;
    if (cp_buffer_frames_max %(lastframe_info.frame_samples) ){
        cp_buffer_frames_max =  cp_buffer_frames_max /(lastframe_info.frame_samples) +1;
    }else{
        cp_buffer_frames_max =  cp_buffer_frames_max /(lastframe_info.frame_samples);
    }

    // prepare for the maximum possible frame length
    out_frame_len = sizeof(*p_out_info) + SCALABLE_MAX_FRAME_SIZE*4;

    ret = a2dp_cp_decoder_init(out_frame_len, cp_buffer_frames_max * 2);
    if (ret){
        TRACE_A2DP_DECODER_W("[MCU][SCALABLE] cp_decoder_init failed: ret=%d", ret);
        set_cp_reset_flag(true);
        return A2DP_DECODER_DECODE_ERROR;
    }

#if defined(BT_DONT_PLAY_MUTE_WHEN_A2DP_STUCK_PATCH)
    int8_t refill_subframes = app_tws_ibrt_audio_analysis_get_refill_frames();

    if (refill_subframes < 0)
    {
        // leave at leaset one entry in the audio list
        while (a2dp_audio_list_length(list) > 1)
        {
            a2dp_audio_list_remove(list, list_front(list));
            refill_subframes = app_tws_ibrt_audio_analysis_update_refill_frames(1);
            if (refill_subframes >= 0)
            {
                break;
            }
        }
    }
    else if (refill_subframes > 0)
    {
        if ((node = a2dp_audio_list_begin(list)) != NULL) {
            decoder_frame_p = (a2dp_audio_scalable_decoder_frame_t *)
            a2dp_audio_list_node(node);

            if (false == is_valid_frame(decoder_frame_p)) {
                TRACE_A2DP_DECODER_D("[MCU][SCALABLE] invalid frame!");
                set_cp_reset_flag(true);
                return A2DP_DECODER_DECODE_ERROR;
            }

            in_info.sequenceNumber = decoder_frame_p->header.sequenceNumber;
            in_info.timestamp = decoder_frame_p->header.timestamp;
#ifdef SCALABLE_PLC_ENABLED
            in_info.isPLC = true;
            Frame_sequence_mismatch_Cnt++;
            TRACE(0, "Frame sequence mismatch!!!!");
#endif
            TRACE(0, "seq %d time stamp %d", decoder_frame_p->header.sequenceNumber,
                decoder_frame_p->header.timestamp);

            while (1)
            {
                if (in_info.sequenceNumber > 0)
                {
                    in_info.sequenceNumber--;
                }
                else
                {
                    in_info.sequenceNumber = 0xFFFF;
                }

                ret = a2dp_cp_put_in_frame(&in_info, sizeof(in_info), decoder_frame_p->header.ptrData, decoder_frame_p->header.dataLen);
                if (ret) {
                    TRACE_A2DP_DECODER_D("[MCU][SCALABLE] piff !!!!!!ret: %d ", ret);
                    break;
                }

                refill_subframes = app_tws_ibrt_audio_analysis_update_refill_frames(-1);
                if (refill_subframes <= 0)
                {
                    break;
                }
            }
        }
    }

    uint8_t* a2dp_data_ptr;
    uint32_t a2dp_data_len;
    while ((node = a2dp_audio_list_begin(list)) != NULL) {
        decoder_frame_p = (a2dp_audio_scalable_decoder_frame_t *)
        a2dp_audio_list_node(node);

#ifdef SBM_DEBUG_ENABLED
        TRACE(0, "lastSequenceNum %d", lastframe_info.lastPopedSeqNum);
#endif

        if (false == is_valid_frame(decoder_frame_p)) {
            TRACE_A2DP_DECODER_E("[MCU][SCALABLE] invalid frame at %d actual seq %d!",
                lastframe_info.lastPopedSeqNum, decoder_frame_p->header.sequenceNumber);
        #ifdef SCALABLE_PLC_ENABLED
            if (scalable_plc_a2dp_packet_size > 0)
            {
                decoder_frame_p->header.sequenceNumber = lastframe_info.lastPopedSeqNum;
                decoder_frame_p->header.timestamp = lastframe_info.lastPopedTimeStamp;
                a2dp_data_ptr = scalable_plc_a2dp_packet_buf_ptr;
                a2dp_data_len = scalable_plc_a2dp_packet_size;
                in_info.isPLC = true;
            }
            else
            {
                set_cp_reset_flag(true);
                return A2DP_DECODER_DECODE_ERROR;
            }
        #else
            set_cp_reset_flag(true);
            return A2DP_DECODER_DECODE_ERROR;
        #endif
        }
        else
        {
            a2dp_data_ptr = decoder_frame_p->header.ptrData;
            a2dp_data_len = decoder_frame_p->header.dataLen;
        #ifdef SCALABLE_PLC_ENABLED
            in_info.isPLC = false;
        #endif
        }

        in_info.sequenceNumber = decoder_frame_p->header.sequenceNumber;
        in_info.timestamp = decoder_frame_p->header.timestamp;

        ret = a2dp_cp_put_in_frame(&in_info, sizeof(in_info),
                                a2dp_data_ptr, a2dp_data_len);
        if (ret) {
            TRACE_A2DP_DECODER_D("[MCU][SCALABLE] piff !!!!!!ret: %d ", ret);
            break;
        }

        lastframe_info.lastPopedSeqNum++;
        lastframe_info.lastPopedTimeStamp++;

        a2dp_audio_list_remove(list, decoder_frame_p);
    }
#else
    while ((node = a2dp_audio_list_begin(list)) != NULL) {
        decoder_frame_p = (a2dp_audio_scalable_decoder_frame_t *)
        a2dp_audio_list_node(node);

        if (false == is_valid_frame(decoder_frame_p)) {
            TRACE_A2DP_DECODER_D("[MCU][SCALABLE] invalid frame!");
            set_cp_reset_flag(true);
            return A2DP_DECODER_DECODE_ERROR;
        }

        in_info.sequenceNumber = decoder_frame_p->header.sequenceNumber;
        in_info.timestamp = decoder_frame_p->header.timestamp;
        in_info.isPLC = false;

        ret = a2dp_cp_put_in_frame(&in_info, sizeof(in_info), decoder_frame_p->header.ptrData, decoder_frame_p->header.dataLen);
        if (ret) {
            TRACE_A2DP_DECODER_D("[MCU][SCALABLE] piff !!!!!!ret: %d ", ret);
            break;
        }

        a2dp_audio_list_remove(list, decoder_frame_p);
    }
#endif

    bool isFirstFetch = true;
    do
    {
        ret = a2dp_cp_get_full_out_frame((void **)&out, &out_len);
        if (isFirstFetch)
        {
            if (ret) {
                if (!get_in_cp_frame_cnt()){
#ifdef SCALABLE_PLC_ENABLED
                    // give chance for PLC
                    if (scalable_last_received_a2dp_packet_size > 0)
                    {
                        if (scalable_underflow_plc_count < SCALABLE_UNDERFLOW_PLC_TOLERANCE_TIMES)
                        {
                            in_info.sequenceNumber = lastframe_info.sequenceNumber+1;
                            in_info.timestamp = lastframe_info.timestamp+1;
                            in_info.isPLC = true;

                            ret = a2dp_cp_put_in_frame(&in_info, sizeof(in_info),
                                scalable_last_received_a2dp_packet_buf_ptr, scalable_last_received_a2dp_packet_size);

                            if (0 == ret)
                            {
                                TRACE(0, "scalable underflow plc %d %d", in_info.sequenceNumber,
                                    in_info.timestamp);
                                scalable_underflow_plc_count++;
                            }
                        }
                    }
#endif
                    if (ret)
                    {
                        scalable_underflow_plc_count = 0;
                        TRACE_A2DP_DECODER_I("[MCU][SCALABLE] cp cache underflow list:%d in_cp:%d",a2dp_audio_list_length(list), get_in_cp_frame_cnt());
                        return A2DP_DECODER_CACHE_UNDERFLOW_ERROR;
                    }
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
                        if (delayCount >= 8)
                        {
                            TRACE_A2DP_DECODER_I("[MCU][SCALABLE] cp cache underflow list:%d in_cp:%d",a2dp_audio_list_length(list), get_in_cp_frame_cnt());
                            a2dp_cp_scalable_lc_after_cache_underflow();
                            if (get_in_cp_frame_cnt()) {
                                return A2DP_DECODER_CACHE_UNDERFLOW_ERROR_SYS_BUSY;
                            }
                            return A2DP_DECODER_CACHE_UNDERFLOW_ERROR;
                        }
                    }
                    else
                    {
                        //TRACE(0, "=%d", (delayCount+1)*2);
                        break;
                    }

                    delayCount++;
                } while (1);
            }
            else
            {
                // reset plc count
                scalable_underflow_plc_count = 0;
            }
        }
        else
        {
            if (ret && (get_in_cp_frame_cnt() > 0))
            {
                if (!a2dp_audio_sysfreq_boost_running()){
                    a2dp_audio_sysfreq_boost_start(1);
                }

                uint8_t delayCount = 0;
                do
                {
                    osDelay(2);
                    ret = a2dp_cp_get_full_out_frame((void **)&out, &out_len);
                    if (ret) {
                        if (delayCount >= 8)
                        {
                            TRACE_A2DP_DECODER_I("[MCU][SCALABLE] cp cache underflow list:%d in_cp:%d",a2dp_audio_list_length(list), get_in_cp_frame_cnt());
                            a2dp_cp_scalable_lc_after_cache_underflow();
                            if (get_in_cp_frame_cnt()) {
                                return A2DP_DECODER_CACHE_UNDERFLOW_ERROR_SYS_BUSY;
                            }
                            return A2DP_DECODER_CACHE_UNDERFLOW_ERROR;
                        }
                    }
                    else
                    {
                        //TRACE(0, "=%d", (delayCount+1)*2);
                        break;
                    }

                    delayCount++;
                } while (1);
            }
        }

        isFirstFetch = false;

        if (out_len == 0) {
            memset(buffer, 0, buffer_bytes);
            a2dp_cp_consume_full_out_frame();
            TRACE_A2DP_DECODER_I("[MCU][SCALABLE] olz!!!%d ", __LINE__);
            return A2DP_DECODER_NO_ERROR;
        }

        p_out_info = (struct A2DP_CP_scalable_OUT_FRM_INFO_T *)out;
        if (p_out_info->pcm_len && (0 == p_out_info->fetchOffset)) {
        #ifdef SBM_DEBUG_ENABLED
            TRACE(0, "SBM 0x%x seq %d", (uint32_t)p_out_info, p_out_info->in_info.sequenceNumber);
        #endif
            lastframe_info.sequenceNumber = p_out_info->in_info.sequenceNumber;
            lastframe_info.timestamp = p_out_info->in_info.timestamp;
            lastframe_info.curSubSequenceNumber = 0;
            lastframe_info.totalSubSequenceNumber = 0;
            lastframe_info.frame_samples = p_out_info->frame_samples;
            lastframe_info.decoded_frames += p_out_info->decoded_frames;
            lastframe_info.undecode_frames =
                a2dp_audio_list_length(list) + a2dp_cp_get_in_frame_cnt_by_index(p_out_info->frame_idx) - 1;
            a2dp_audio_decoder_internal_lastframe_info_set(&lastframe_info);
        }

        uint32_t leftPcmData = p_out_info->pcm_len - p_out_info->fetchOffset;
#ifdef SBM_DEBUG_ENABLED
        TRACE(0, "SBM info 0x%x %d-%d-%d", (uint32_t)p_out_info, p_out_info->pcm_len, p_out_info->fetchOffset,
            buffer_bytes);
#endif
        if (leftPcmData > buffer_bytes)
        {
            memcpy(buffer, (uint8_t *)((uint32_t)(p_out_info + 1) + p_out_info->fetchOffset),
                buffer_bytes);
            p_out_info->fetchOffset += buffer_bytes;
            dec_ret = A2DP_DECODER_NO_ERROR;
#ifdef SBM_DEBUG_ENABLED
            TRACE(0, "SBM execute 0x%x - %d", (uint32_t)p_out_info, p_out_info->fetchOffset);
#endif
            break;
        }
        else
        {
            memcpy(buffer, (uint8_t *)((uint32_t)(p_out_info + 1) + p_out_info->fetchOffset),
                leftPcmData);
            p_out_info->fetchOffset += leftPcmData;
            buffer += leftPcmData;
            buffer_bytes -= leftPcmData;
        }
#ifdef SBM_DEBUG_ENABLED
        TRACE(0, "SBM free 0x%x buffer_bytes %d", (uint32_t)p_out_info, buffer_bytes);
#endif
        ret = a2dp_cp_consume_full_out_frame();
        if (ret){

            TRACE_A2DP_DECODER_I("[MCU][SCALABLE] cp consume_full_out_frame() failed: ret=%d", ret);
            set_cp_reset_flag(true);
            return A2DP_DECODER_DECODE_ERROR;
        }
        else
        {
            if (0 == buffer_bytes)
            {
                dec_ret = A2DP_DECODER_NO_ERROR;
                break;
            }
        }

    } while (1);

    return dec_ret;
}

TEXT_SSC_LOC int a2dp_cp_scalable_cp_decode(void)
{
    int ret;
    enum CP_EMPTY_OUT_FRM_T out_frm_st;
    uint8_t *out;
    uint32_t out_len;
    uint8_t *dec_start;
    struct A2DP_CP_scalable_IN_FRM_INFO_T *p_in_info;
    struct A2DP_CP_scalable_OUT_FRM_INFO_T *p_out_info;
    uint8_t *in_buf;
    uint32_t in_len;
    uint32_t dec_sum;
    int error, output_samples = 0,output_byte = 0;

#ifdef DEBUG_CALC_MCPS
    uint32_t stime;
    uint32_t etime;
    uint32_t ptime;
#endif

    if (cp_codec_reset)
    {
        TRACE(0, "[SSC]a2dp_cp_scalable_cp_decode() - cp_codec_reset");
        cp_codec_reset      = false;
        a2dp_audio_scalable_decoder_init();
    }

    out_frm_st = a2dp_cp_get_emtpy_out_frame((void **)&out, &out_len);

    if (out_frm_st != CP_EMPTY_OUT_FRM_OK && out_frm_st != CP_EMPTY_OUT_FRM_WORKING)
    {
        return 1;
    }

    ASSERT(out_len > sizeof(*p_out_info), "%s: Bad out_len %u (should > %u)", __func__, out_len, sizeof(*p_out_info));

    p_out_info = (struct A2DP_CP_scalable_OUT_FRM_INFO_T *)out;
    if (out_frm_st == CP_EMPTY_OUT_FRM_OK)
    {
        p_out_info->pcm_len = 0;
        p_out_info->fetchOffset = 0;
        p_out_info->decoded_frames = 0;
    }

    ASSERT(out_len > sizeof(*p_out_info) + p_out_info->pcm_len,
       "%s: Bad out_len %u (should > %u + %u)", __func__, out_len, sizeof(*p_out_info), p_out_info->pcm_len);

    dec_start = (uint8_t *) (p_out_info + 1) + p_out_info->pcm_len;
    if (!scalableDec_handle) {
        TRACE(0,"scalable_decode not ready");
        return 3;
    }

    dec_sum = 0;
    error = 0;

    ret = a2dp_cp_get_in_frame((void * *) &in_buf, &in_len);

    if (ret)
    {
        return 4;
    }

    ASSERT(in_len > sizeof(*p_in_info), "%s: Bad in_len %u (should > %u)", __func__, in_len, sizeof(*p_in_info));
    p_in_info = (struct A2DP_CP_scalable_IN_FRM_INFO_T *)in_buf;
    in_buf += sizeof(*p_in_info);
    in_len -= sizeof(*p_in_info);

    /* decode one SSC frame */
    bool isNeedToRetrigger = sbm_operation_pre_decoding_handler(p_in_info->sequenceNumber);
    if (isNeedToRetrigger)
    {
        return 4;
    }

#ifdef SCALABLE_PLC_ENABLED
    if (p_in_info->isPLC)
    {
        PLC_Frame++;
    }
#endif

#ifdef DEBUG_CALC_MCPS
    stime = hal_fast_sys_timer_get();
#endif

#ifdef SCALABLE_PLC_ENABLED
    output_samples = ssc_decode(scalableDec_handle,(const unsigned char*)in_buf, (unsigned char*)ss_pcm_buff, SCALABLE_FRAME_SIZE, p_in_info->isPLC, 2);
#else
    output_samples = ssc_decode(scalableDec_handle,(const unsigned char*)in_buf, (unsigned char*)ss_pcm_buff, SCALABLE_FRAME_SIZE, 0, 2);
#endif

#ifdef DEBUG_CALC_MCPS
    etime = hal_fast_sys_timer_get();
    ptime = FAST_TICKS_TO_US(etime - stime);
    if(FrameCnt > 50)
    {
        if(ptime > cp_peak_dec_time)
            cp_peak_dec_time = ptime;
    }
    //TRACE(0, "ssc audio excution cycle(avg) : %5u -- %5u", ptime, cp_peak_dec_time);
#endif

    frame_cnt_audio++;
    FrameCnt++;
    if(frame_cnt_audio % 133 == 0)
    {
#ifdef DEBUG_CALC_MCPS
        TRACE(0, "[SSC]ssc audio excution us(Peak) : %5u", cp_peak_dec_time);
#endif
        TRACE(0, "[SSC]ssc audio DBG_MSG Total Frame : %d, PLC Frame : %d, L/R Sequence mismatch : %d", FrameCnt, PLC_Frame, Frame_sequence_mismatch_Cnt);
        TRACE(0, "[SSC]sbm operation_fast : %d, error data fast = %d", sbm_operation_fast, sbm_error_fast);
        TRACE(0, "[SSC]sbm operation_slow : %d, error data slow = %d", sbm_operation_slow, sbm_error_slow);
        frame_cnt_audio = 0;
    }

#ifdef SBM_DEBUG_ENABLED
    TRACE(0, "out:%d", output_samples);
#endif

    if (0 == output_samples)
    {
        a2dp_audio_scalable_decoder_reinit();
        TRACE(0, "scalable_decode reinin codec \n");
        error = A2DP_DECODER_DECODE_ERROR;
        goto exit;
    }

    ss_to_24bit_buf((int32_t *) (dec_start+dec_sum), (int32_t *) ss_pcm_buff, output_samples);
    output_byte = output_samples * 4;
    dec_sum += output_byte;
exit:
    memcpy(&p_out_info->in_info, (uint8_t*)p_in_info, sizeof(*p_in_info));
    p_out_info->decoded_frames++;
    p_out_info->frame_samples = output_samples;
    p_out_info->frame_idx = a2dp_cp_get_in_frame_index();

    ret = a2dp_cp_consume_in_frame();
    ASSERT(ret == 0, "%s: a2dp_cp_consume_in_frame() failed: ret=%d", __func__, ret);

    p_out_info->pcm_len += dec_sum;

#ifdef SBM_DEBUG_ENABLED
    TRACE(0, "SBM add pcm 0x%x %d", (uint32_t)p_out_info, p_out_info->pcm_len);
#endif
    ret = a2dp_cp_consume_emtpy_out_frame();
    ASSERT(ret == 0, "%s: a2dp_cp_consume_emtpy_out_frame() failed: ret=%d", __func__, ret);

    return error;
}
#endif

static int a2dp_audio_scalable_init(A2DP_AUDIO_OUTPUT_CONFIG_T * config, void *context)
{
    TRACE_A2DP_DECODER_D("%s", __func__);

    TRACE(0,"\n\nA2DP SSC-LC INIT\n");

    a2dp_audio_context_p = (A2DP_AUDIO_CONTEXT_T *) context;

    memcpy(&output_config, config, sizeof(A2DP_AUDIO_OUTPUT_CONFIG_T));

    memset(&lastframe_info, 0, sizeof(A2DP_AUDIO_DECODER_LASTFRAME_INFO_T));
    lastframe_info.stream_info = output_config;
    lastframe_info.frame_samples = SCALABLE_FRAME_SIZE;
    lastframe_info.list_samples = SCALABLE_FRAME_SIZE;
    a2dp_audio_decoder_internal_lastframe_info_set(&lastframe_info);

    scalable_underflow_plc_count = 0;
    scalable_last_received_a2dp_packet_size = 0;

    scalable_plc_a2dp_packet_size = 0;
    isFirstScalablePacketReceived = false;

    ASSERT(a2dp_audio_context_p->dest_packet_mut < SCALABLE_MTU_LIMITER,
       "%s MTU OVERFLOW:%u/%u", __func__, a2dp_audio_context_p->dest_packet_mut, SCALABLE_MTU_LIMITER);
#ifndef A2DP_DECODER_CROSS_CORE
    int decoder_size;

    decoder_size = ssc_decoder_get_size(output_config.num_channels, output_config.sample_rate); //todo:  get size with codec capability

    TRACE(0, "decoder size %d", decoder_size);

    scalable_decoder_place = (unsigned char *)a2dp_audio_heap_malloc(decoder_size);
    ASSERT_A2DP_DECODER(scalable_decoder_place, "no memory resource for scalable_decoder_place");

    scalable_decoder_temp_buf = (unsigned char *)a2dp_audio_heap_malloc(SCALABLE_MAX_FRAME_SIZE * 16);
    ASSERT_A2DP_DECODER(scalable_decoder_temp_buf, "no memory resource for scalable_decoder_temp_buf");

#ifdef A2DP_CP_ACCEL
    int ret;
    cp_codec_reset = true;
    ret = a2dp_cp_init(a2dp_cp_scalable_cp_decode, CP_PROC_DELAY_1_FRAME);
    ASSERT(ret == 0, "%s: a2dp_cp_init() failed: ret=%d", __func__, ret);
#else
    a2dp_audio_scalable_decoder_init();
#endif
#endif // #ifndef A2DP_DECODER_CROSS_CORE
    return A2DP_DECODER_NO_ERROR;
}

static int a2dp_audio_scalable_deinit(void)
{
#ifdef A2DP_CP_ACCEL
    a2dp_cp_deinit();
#endif

    a2dp_audio_scalable_decoder_deinit();

    TRACE(0,"\n\nA2DP   SCALABLE  DEINIT\n");

    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_scalable_mcu_decode_frame(uint8_t * buffer, uint32_t buffer_bytes)
{
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    a2dp_audio_scalable_decoder_frame_t *decoder_frame_p = NULL;
    int ret = A2DP_DECODER_NO_ERROR;

    bool cache_underflow = false;
    int output_byte = 0, output_samples = 0;
    uint8_t *output = buffer;

    if (buffer_bytes < (SCALABLE_FRAME_SIZE * output_config.num_channels * output_config.bits_depth / 8)) {
        TRACE(1,"scalable_decode pcm_len = %d \n", buffer_bytes);
        return A2DP_DECODER_NO_ERROR;
    }

    if (!scalableDec_handle) {
        TRACE(0,"scalable_decode not ready");
        return A2DP_DECODER_NO_ERROR;
    }

    if (currentTmpLeftBytes > 0)
    {
    #ifdef SBM_DEBUG_ENABLED
        TRACE(0, "SBM 1:%d %d %d", buffer_bytes, currentTmpLeftBytes,
            currentTmpBufOffset);
    #endif
        if (currentTmpLeftBytes < buffer_bytes)
        {
            memcpy(output, ((uint8_t *)ss_pcm_buff)+currentTmpBufOffset, currentTmpLeftBytes);
            output += currentTmpLeftBytes;
            currentTmpLeftBytes = 0;
            currentTmpBufOffset = 0;
        }
        else
        {
            memcpy(output, ((uint8_t *)ss_pcm_buff)+currentTmpBufOffset, buffer_bytes);
            output += buffer_bytes;
            currentTmpLeftBytes -= buffer_bytes;
            currentTmpBufOffset += buffer_bytes;
            return A2DP_DECODER_NO_ERROR;
        }
    }

    while (output < buffer + buffer_bytes) {
        node = a2dp_audio_list_begin(list);
        if (!node)
        {
            TRACE(0,"scalable_decode cache underflow");
            cache_underflow = true;
            goto exit;
        }
        else
        {

            decoder_frame_p = (a2dp_audio_scalable_decoder_frame_t *)
                a2dp_audio_list_node(node);

            if (false == is_valid_frame(decoder_frame_p)) {
                TRACE_A2DP_DECODER_E("%s %d invalid a2dp frame", __func__, __LINE__);
                ret = A2DP_DECODER_DECODE_ERROR;
                goto exit;
            }

            bool isNeedToRetrigger = sbm_operation_pre_decoding_handler(decoder_frame_p->header.sequenceNumber);
            if (isNeedToRetrigger)
            {
                ret = A2DP_DECODER_CACHE_UNDERFLOW_ERROR;
                goto exit;
            }
            /* decode one SSC frame */
            output_samples =
                ssc_decode(scalableDec_handle,
                   decoder_frame_p->header.ptrData, (uint8_t *)ss_pcm_buff, SCALABLE_FRAME_SIZE,0,2);
        #ifdef SBM_DEBUG_ENABLED
            TRACE
            (0, "scalable_decode seq:%d len:%d output_samples:%d buffer_bytes: %d",
             decoder_frame_p->header.sequenceNumber, decoder_frame_p->header.dataLen, output_samples, buffer_bytes);
        #endif
            if (0 == output_samples) {
                TRACE(0,"scalable_decode failed  !!!!!!");
                //if  failed reopen it again
                a2dp_audio_scalable_decoder_reinit();
                TRACE(0,"scalable_decode reinin codec \n");
                ret = A2DP_DECODER_DECODE_ERROR;
                goto exit;
            }

            output_byte = output_samples * 4;

            uint32_t costedBytes = 0;
            if ((output+output_byte) <= (buffer + buffer_bytes))
            {
                costedBytes = output_byte;
            }
            else
            {
                costedBytes = (buffer + buffer_bytes)-output;
            }

            ss_to_24bit_buf((int32_t *) output, (int32_t *) ss_pcm_buff, costedBytes/sizeof(int32_t));
            currentTmpLeftBytes = output_byte - costedBytes;
            currentTmpBufOffset = costedBytes;
        #ifdef SBM_DEBUG_ENABLED
            TRACE(0, "SBM 2:%d %d %d", currentTmpLeftBytes, currentTmpBufOffset,
                costedBytes);
        #endif
            output += costedBytes;

            lastframe_info.sequenceNumber = decoder_frame_p->header.sequenceNumber;
            lastframe_info.timestamp = decoder_frame_p->header.timestamp;
            lastframe_info.curSubSequenceNumber = 0;
            lastframe_info.totalSubSequenceNumber = 0;
            lastframe_info.frame_samples = SCALABLE_FRAME_SIZE;
            lastframe_info.decoded_frames++;
            lastframe_info.undecode_frames = a2dp_audio_list_length(list) - 1;
            a2dp_audio_decoder_internal_lastframe_info_set(&lastframe_info);
            a2dp_audio_list_remove(list, decoder_frame_p);
        }
    }

  exit:
    if (cache_underflow) {
        lastframe_info.undecode_frames = 0;
        a2dp_audio_decoder_internal_lastframe_info_set(&lastframe_info);
        ret = A2DP_DECODER_CACHE_UNDERFLOW_ERROR;
    }
    //TRACE(0,"abd");
    return ret;
}

static int a2dp_audio_scalable_decode_frame(uint8_t * buffer, uint32_t buffer_bytes)
{
#ifndef A2DP_DECODER_CROSS_CORE
#ifdef A2DP_CP_ACCEL
    return a2dp_cp_scalable_mcu_decode(buffer, buffer_bytes);
#else
    return a2dp_audio_scalable_mcu_decode_frame(buffer, buffer_bytes);
#endif
#else
    return A2DP_DECODER_NO_ERROR;
#endif // #ifndef A2DP_DECODER_CROSS_CORE
}

static int a2dp_audio_scalable_preparse_packet(btif_media_header_t * header, uint8_t * buffer, uint32_t buffer_bytes)
{
    lastframe_info.sequenceNumber = header->sequenceNumber;
    lastframe_info.timestamp = header->timestamp;
    lastframe_info.curSubSequenceNumber = 0;
    lastframe_info.totalSubSequenceNumber = 0;
    lastframe_info.frame_samples = SCALABLE_FRAME_SIZE;
    lastframe_info.list_samples = SCALABLE_FRAME_SIZE;
    lastframe_info.decoded_frames = 0;
    lastframe_info.undecode_frames = 0;
    a2dp_audio_decoder_internal_lastframe_info_set(&lastframe_info);

    TRACE(3,"%s seq:%d timestamp:%08x", __func__, header->sequenceNumber, header->timestamp);

    return A2DP_DECODER_NO_ERROR;
}

static void *a2dp_audio_scalable_frame_malloc(uint32_t packet_len)
{
    a2dp_audio_scalable_decoder_frame_t *decoder_frame_p = NULL;
    uint8_t *buffer = NULL;

    buffer = (uint8_t *) a2dp_audio_heap_malloc(SCALABLE_READBUF_SIZE);
    decoder_frame_p = (a2dp_audio_scalable_decoder_frame_t *)
    a2dp_audio_heap_malloc(sizeof(a2dp_audio_scalable_decoder_frame_t));
    decoder_frame_p->header.ptrData = buffer;
    decoder_frame_p->header.dataLen = packet_len;
    decoder_frame_p->header.totalSubSequenceNumber = 0;
    decoder_frame_p->header.curSubSequenceNumber = 0;
    return (void *)decoder_frame_p;
}

static void a2dp_audio_scalable_free(void *packet)
{
    a2dp_audio_scalable_decoder_frame_t *decoder_frame_p = (a2dp_audio_scalable_decoder_frame_t *) packet;
    a2dp_audio_heap_free(decoder_frame_p->header.ptrData);
    a2dp_audio_heap_free(decoder_frame_p);
}

static int a2dp_audio_scalable_store_packet(btif_media_header_t * header, uint8_t * buffer, uint32_t buffer_bytes)
{
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    int nRet = A2DP_DECODER_NO_ERROR;

    if (a2dp_audio_list_length(list) < SCALABLE_MTU_LIMITER) {

    if (buffer_bytes > SCALABLE_READBUF_SIZE)
    {
        TRACE(0, "%s buffer_bytes(%d) > SCALABLE_READBUF_SIZE(%d), Limit it to SCALABLE_READBUF_SIZE",
                                                        __func__, buffer_bytes, SCALABLE_READBUF_SIZE);
        buffer_bytes = SCALABLE_READBUF_SIZE;
    }
    a2dp_audio_scalable_decoder_frame_t *decoder_frame_p = (a2dp_audio_scalable_decoder_frame_t *)
        a2dp_audio_scalable_frame_malloc(buffer_bytes);
#ifdef SBM_DEBUG_ENABLED
    TRACE(0, "%s seq:%d len:%d", __func__, header->sequenceNumber, buffer_bytes);
#endif
    decoder_frame_p->header.sequenceNumber = header->sequenceNumber;
    decoder_frame_p->header.timestamp = header->timestamp;
    memcpy(decoder_frame_p->header.ptrData, buffer, buffer_bytes);
    decoder_frame_p->header.dataLen = buffer_bytes;

    if (false == is_valid_frame(decoder_frame_p))
    {
        TRACE_A2DP_DECODER_E("%s %d invalid a2dp frame", __func__, __LINE__);
        scalable_plc_a2dp_packet_size = scalable_last_received_a2dp_packet_size;
        scalable_plc_a2dp_packet_buf_ptr = scalable_last_received_a2dp_packet_buf_ptr;
    }
    else
    {
        if (isFirstScalablePacketReceived)
        {
            // check whether the sequence number is consective
            uint32_t seqNumGap;
            if (decoder_frame_p->header.sequenceNumber > lastStoredScalablePacketSeqNum)
            {
                seqNumGap = decoder_frame_p->header.sequenceNumber-lastStoredScalablePacketSeqNum;
            }
            else
            {
                seqNumGap = 0x10000+decoder_frame_p->header.sequenceNumber-lastStoredScalablePacketSeqNum;
            }

            if (seqNumGap > SCALABLE_NON_CONTINUOUS_SEQ_NUM_TOLERANCE_GAP)
            {
                TRACE(0, "Scalable seq num gap %d, do retrigger", seqNumGap);
                a2dp_audio_scalable_free(decoder_frame_p);
                return A2DP_DECODER_NON_CONSECTIVE_SEQ;
            }
            else if (seqNumGap > 1)
            {
                TRACE(0, "Scalable seq num gap %d, activate PLC!", seqNumGap);
                // append missing packets
                uint32_t filledUpCount = 0;
                uint16_t filledSeqNum = lastStoredScalablePacketSeqNum;

                TRACE(0, "insert invalid frame for following PLC handling");
                while (filledUpCount < (seqNumGap-1))
                {
                    // reserve one slot for the received data packet
                    if (a2dp_audio_list_length(list) >= (SCALABLE_MTU_LIMITER-1))
                    {
                        TRACE(2,"%s list full current len:%d", __func__, a2dp_audio_list_length(list));
                        a2dp_audio_scalable_free(decoder_frame_p);
                        return A2DP_DECODER_NON_CONSECTIVE_SEQ;
                    }
                    filledSeqNum++;
                    filledUpCount++;
                    a2dp_audio_scalable_decoder_frame_t *padding_decoder_frame_p = (a2dp_audio_scalable_decoder_frame_t *)
                        a2dp_audio_scalable_frame_malloc(buffer_bytes);
                    TRACE(0, "insert seq %d", filledSeqNum);
                    padding_decoder_frame_p->header.sequenceNumber = filledSeqNum;
                    padding_decoder_frame_p->header.timestamp = filledSeqNum;
                    // invalid data length to let it processed by invalid packet PLC handling
                    padding_decoder_frame_p->header.dataLen = 0;
                    a2dp_audio_list_append(list, padding_decoder_frame_p);
                }

                lastStoredScalablePacketSeqNum = filledSeqNum;
                scalable_plc_a2dp_packet_size = scalable_last_received_a2dp_packet_size;
                scalable_plc_a2dp_packet_buf_ptr = scalable_last_received_a2dp_packet_buf_ptr;
            }
        }
        else
        {
            isFirstScalablePacketReceived = true;
        }

        lastStoredScalablePacketSeqNum = decoder_frame_p->header.sequenceNumber;
        scalable_last_received_a2dp_packet_size = buffer_bytes;
        scalable_last_received_a2dp_packet_buf_ptr = decoder_frame_p->header.ptrData;
    }
    a2dp_audio_list_append(list, decoder_frame_p);
    a2dp_audio_list_end(list);
    nRet = A2DP_DECODER_NO_ERROR;
    } else {
        TRACE(2,"%s list full current len:%d", __func__, a2dp_audio_list_length(list));
        nRet = A2DP_DECODER_MTU_LIMTER_ERROR;
    }

    return nRet;
}

static int a2dp_audio_scalable_discards_packet(uint32_t packets)
{
    int nRet = A2DP_DECODER_MEMORY_ERROR;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    a2dp_audio_scalable_decoder_frame_t *decoder_frame_p = NULL;

#ifdef A2DP_CP_ACCEL
    a2dp_cp_reset_frame();
#endif

    if (packets <= a2dp_audio_list_length(list)) {
    for (uint8_t i = 0; i < packets; i++) {
        if ((node = a2dp_audio_list_begin(list)) != NULL){
            decoder_frame_p = (a2dp_audio_scalable_decoder_frame_t *)
            a2dp_audio_list_node(node);
            a2dp_audio_list_remove(list, decoder_frame_p);
        }
    }
    nRet = A2DP_DECODER_NO_ERROR;
    }

    TRACE(3,"%s packets:%d nRet:%d", __func__, packets, nRet);
    return nRet;
}

#define AUDIO_LATENCY_CHUNKER_GAP_TO_TRIGGER_TUNING 1

#define SBM_FAST_SPEED 67380 // 840 2.76%
#define SBM_SLOW_SPEED 63730 // 888 2.76%
#define SBM_NUM_NORMAL 100

#define SBM_MIN_APPLY_SEQUENCE_GAP  20

static SBM_OPERATION_ENV_T sbm_operation_env;
#ifdef A2DP_DECODER_CROSS_CORE_USE_M55
SBM_STATUS_E sbm_status = SBM_IN_NORMAL_SPEED;
#endif
#define SAME_STATUS_CHANCE  10
static uint8_t sameStatusCount = 0;
static SBM_OPERATION_E pendingOperation;
static SBM_STATUS_E lastSbmStatus = SBM_IN_NORMAL_SPEED;

static bool sbm_operation_pre_decoding_handler(uint32_t currentSeqNum)
{
    bool isNeedToRetrigger = false;

    uint32_t lock = int_lock();

    if (sbm_operation_env.isToProcess)
    {
        // check for the SBM parameter validity
		if(sbm_operation_env.sbmOperationToDo != SBM_USE_NORMAL_SPEED)
		{
			if(ssc_SBM_getstatus() != 0)
			{
				TRACE(0, "[SSC] while SBM, request SBM again");
			}
            switch(sbm_operation_env.chunkOffset)
            {
                case 4:
                case 5:
                case 7:
                case 9:
                case 11:
                    //normal operation!!
                    break;
                default :
                    if(sbm_operation_env.sbmOperationToDo == 1)
                    {
                        if(sbm_error_fast != sbm_operation_env.chunkOffset)
                        {
                            sbm_operation_fast++;
                        }
                        sbm_error_fast = sbm_operation_env.chunkOffset;
                    }
                    else if (sbm_operation_env.sbmOperationToDo == -1)
                    {
                        if(sbm_error_slow != sbm_operation_env.chunkOffset)
                        {
                            sbm_operation_slow++;
                        }
                        sbm_error_slow = sbm_operation_env.chunkOffset;
                    }
                    TRACE(0, "[SSC] SBM direction = %d, Offset = %d",sbm_operation_env.sbmOperationToDo, sbm_operation_env.chunkOffset);
                    break;
            }
		}
        if (currentSeqNum == sbm_operation_env.sequenceNumToApplyNewSpeed)
        {
            ssc_sbm_interface(sbm_operation_env.sbmOperationToDo,
                sbm_operation_env.chunkOffset);
        }
        else if (currentSeqNum > sbm_operation_env.sequenceNumToApplyNewSpeed)
        {
            if (!(sbm_operation_env.isSequenceNumToApplyRollBack))
            {
                TRACE(0, "SBM: miss the applying speed sequence number!!!");
                isNeedToRetrigger = true;
            }
            else
            {
                ssc_sbm_interface(SBM_USE_NORMAL_SPEED, 0);
                int_unlock(lock);
                return false;
            }
        }
        else
        {
            ssc_sbm_interface(SBM_USE_NORMAL_SPEED, 0);
            int_unlock(lock);
            return false;
        }

        TRACE(0, "SBM start updating.");
        TRACE(0, "SBM: expected seq %d current seq %d",
            sbm_operation_env.sequenceNumToApplyNewSpeed, currentSeqNum);
        sbm_operation_env.isToProcess = false;
    }
    else
    {
        ssc_sbm_interface(SBM_USE_NORMAL_SPEED, 0);
    }
    int_unlock(lock);

    return isNeedToRetrigger;
}
#ifndef A2DP_DECODER_CROSS_CORE
static void sbm_operation_env_reset(void)
{
    currentTmpBufOffset = 0;
    currentTmpLeftBytes = 0;

    uint32_t lock = int_lock();
    memset((uint8_t *)&sbm_operation_env, 0, sizeof(sbm_operation_env));
    sbm_operation_env.sbmOperationToDo = SBM_USE_NORMAL_SPEED;
    int_unlock(lock);

    sameStatusCount = 0;
    pendingOperation = SBM_USE_NORMAL_SPEED;

    // play speed setting for slow and fast speed
    // when normal or game mode triggered and vice versa. Default +-2.76%
    ssc_set_SBMspeed_test(SBM_SLOW_SPEED, SBM_FAST_SPEED, SBM_NUM_NORMAL);
}
#endif
static void sbm_push_operation_request(SBM_OPERATION_E sbmOp, uint8_t chunkOffset, uint32_t seqNumToApplyNewSpeed)
{
    uint32_t lock = int_lock();

    // send out at the right next audio playback info sync
    sbm_operation_env.isToProcess = true;
    sbm_operation_env.sbmOperationToDo = sbmOp;
    sbm_operation_env.chunkOffset = chunkOffset;
    sbm_operation_env.sequenceNumToApplyNewSpeed = (uint16_t)seqNumToApplyNewSpeed;
    if (seqNumToApplyNewSpeed > 0xFFFF)
    {
        sbm_operation_env.isSequenceNumToApplyRollBack = true;
    }
    else
    {
        sbm_operation_env.isSequenceNumToApplyRollBack = false;
    }

#ifdef A2DP_DECODER_CROSS_CORE
    SBM_OPERATION_ENV_T sbm_cc_env;
    audio_audio_scalable_get_sync_custom_data((uint8_t *)&sbm_cc_env);
    app_dsp_m55_bridge_send_cmd(
                         CROSS_CORE_TASK_CMD_A2DP_SCALABLE_SBM_TO_OFF_BTH, \
                         (uint8_t *)&sbm_cc_env, sizeof(SBM_OPERATION_ENV_T));
#endif

    int_unlock(lock);

    TRACE(0, "Push sbm op %d chunk offset %d", sbmOp, chunkOffset);
#ifdef A2DP_DECODER_CROSS_CORE
    A2DP_AUDIO_LASTFRAME_INFO_T a2dp_audio_lastframe_info;
    a2dp_audio_lastframe_info_get(&a2dp_audio_lastframe_info);
    TRACE(0, "Cur seqnum %d SBM shall be processed at seq %d",
        a2dp_audio_lastframe_info.sequenceNumber, seqNumToApplyNewSpeed);
#else
    TRACE(0, "Cur seqnum %d SBM shall be processed at seq %d",
        lastframe_info.sequenceNumber, seqNumToApplyNewSpeed);
#endif
}

const uint8_t valid_sbm_offset[] = {4, 5, 7, 9, 11};
static bool sbm_operation_get_expected_speed(float averageChunkCnt, uint16_t expectedChunkCnt,
    SBM_OPERATION_E* pOperation, uint8_t* pChunkGap)
{
#ifdef SBM_ENHANCED_DEBUG_ENABLED
    TRACE(0, "SBM, avr %d/1000 tgt %d/1000",
            (uint32_t)(averageChunkCnt*1000), (uint32_t)(expectedChunkCnt*1000));
#endif

    bool isNeedToUpdateSpeed = false;

    float chunkGap = abs(averageChunkCnt - expectedChunkCnt);
    uint8_t figuredOutOffset = (uint8_t)chunkGap;
    if (figuredOutOffset < valid_sbm_offset[0])
    {
        // if the offset is smaller than the least tunable offset,
        // let the SDK do the play speed tuning
        sbm_clear_manual_adjustment_on_going_flag();
        return false;
    }

    uint8_t index;
    for (index = 0;index < sizeof(valid_sbm_offset);index++)
    {
        if (figuredOutOffset <= valid_sbm_offset[index])
        {
            figuredOutOffset = valid_sbm_offset[index];
            break;
        }
    }

    if (sizeof(valid_sbm_offset) == index)
    {
        figuredOutOffset = valid_sbm_offset[sizeof(valid_sbm_offset)-1];
    }

    TRACE(0, "SBM offset %d", figuredOutOffset);

    *pChunkGap = figuredOutOffset;

    if ((averageChunkCnt - expectedChunkCnt) >= AUDIO_LATENCY_CHUNKER_GAP_TO_TRIGGER_TUNING)
    {
        *pOperation = SBM_USE_FASTER_SPEED;
        isNeedToUpdateSpeed = true;
    }
    else if ((expectedChunkCnt - averageChunkCnt) >= AUDIO_LATENCY_CHUNKER_GAP_TO_TRIGGER_TUNING)
    {
        *pOperation = SBM_USE_SLOWER_SPEED;
        isNeedToUpdateSpeed = true;
    }

    if (isNeedToUpdateSpeed)
    {
        if (0 == sameStatusCount)
        {
            sameStatusCount++;
            pendingOperation = *pOperation;
        }
        else
        {
            if (pendingOperation == *pOperation)
            {
                sameStatusCount++;
            }
            else
            {
                sameStatusCount = 0;
                pendingOperation = *pOperation;
            }
        }
    }
    else
    {
        sameStatusCount = 0;
    }

    if (sameStatusCount < SAME_STATUS_CHANCE)
    {
        isNeedToUpdateSpeed = false;
    }

    return isNeedToUpdateSpeed;
}

bool a2dp_audio_scalable_latency_custom_tune(uint8_t device_id, float averageChunkCnt, uint16_t expectedChunkCnt)
{
#ifdef IS_DISABLE_A2DP_ALGORITHM_PLAYBACK_SPEED_TUNING
    return false;
#endif

    if (!sbm_is_algorithm_adjustment_enabled())
    {
        return false;
    }

    // only allow manual adjustment
    if (!sbm_is_manual_adjustment_on_going())
    {
        // shall be done by SDK
        return false;
    }

#ifdef A2DP_DECODER_CROSS_CORE
    SBM_STATUS_E currentSbmStatus = a2dp_decoder_bth_get_cc_sbm_status();
#else
    SBM_STATUS_E currentSbmStatus = (SBM_STATUS_E)ssc_SBM_getstatus();
#endif

#ifdef SBM_ENHANCED_DEBUG_ENABLED
    TRACE(0, "SBM, cur status %d last status %d isToProcess %d",
        currentSbmStatus, lastSbmStatus, sbm_operation_env.isToProcess);
#endif
    if ((SBM_IN_NORMAL_SPEED != lastSbmStatus) &&
        (SBM_IN_NORMAL_SPEED == currentSbmStatus))
    {
        sbm_target_jitter_buffer_updating_completed();
    }

    lastSbmStatus = currentSbmStatus;

    if (!(sbm_operation_env.isToProcess))
    {
        uint8_t chunkGap;
        SBM_OPERATION_E operation = SBM_USE_NORMAL_SPEED;

        if (SBM_IN_NORMAL_SPEED == currentSbmStatus)
        {
            if (false == sbm_operation_get_expected_speed(averageChunkCnt, expectedChunkCnt,
                &operation, &chunkGap))
            {
                return true;
            }
        }
        else
        {
            return true;
        }

#ifdef SBM_ENHANCED_DEBUG_ENABLED
        TRACE(0, "SBM cur op %d push op %d offset %d",
            currentSbmStatus, operation, (uint8_t)chunkGap);
#endif

        uint32_t seqNumToBeApplied;
#ifdef A2DP_DECODER_CROSS_CORE
        A2DP_AUDIO_LASTFRAME_INFO_T a2dp_audio_lastframe_info;
        a2dp_audio_lastframe_info_get(&a2dp_audio_lastframe_info);
#endif
#if defined(IBRT)
        if (app_tws_ibrt_tws_link_connected())
        {
            // only for master to trigger the tuning procedure
            if (app_ibrt_if_is_ui_slave())
            {
                return true;
            }

            uint32_t currentSyncTicks = app_tws_ibrt_audio_analysis_tick_get();
            uint32_t syncTicksInternal = app_tws_ibrt_audio_analysis_interval_get();

            uint32_t syncTicksToApplyNewSpeed =
                    (((currentSyncTicks+syncTicksInternal-1)/syncTicksInternal) + 1)
                    *syncTicksInternal;
#ifdef A2DP_DECODER_CROSS_CORE
            seqNumToBeApplied = (a2dp_audio_lastframe_info.sequenceNumber +
                (syncTicksToApplyNewSpeed-currentSyncTicks));
#else
            seqNumToBeApplied = (lastframe_info.sequenceNumber +
                (syncTicksToApplyNewSpeed-currentSyncTicks));
#endif
        }
        else
        {
#ifdef A2DP_DECODER_CROSS_CORE
            seqNumToBeApplied = a2dp_audio_lastframe_info.sequenceNumber + SBM_MIN_APPLY_SEQUENCE_GAP;
#else
            seqNumToBeApplied = lastframe_info.sequenceNumber + SBM_MIN_APPLY_SEQUENCE_GAP;
#endif
        }
#else
#ifdef A2DP_DECODER_CROSS_CORE_USE_M55
        seqNumToBeApplied = a2dp_audio_lastframe_info.sequenceNumber + SBM_MIN_APPLY_SEQUENCE_GAP;
#else
        seqNumToBeApplied = lastframe_info.sequenceNumber + SBM_MIN_APPLY_SEQUENCE_GAP;
#endif
#endif

        sbm_bump_cpu_freq_during_speed_tuning();

        sbm_push_operation_request(operation, (uint8_t)chunkGap, seqNumToBeApplied);

    #if defined(IBRT)
        if (app_tws_ibrt_tws_link_connected())
        {
            app_ibrt_request_peer_custom_play_speed_tuning();
        }
    #endif
    }

    return true;
}

void audio_audio_scalable_get_sync_custom_data(uint8_t* custom_data)
{
    uint32_t lock = int_lock();
    memcpy(custom_data, (uint8_t *)&sbm_operation_env, sizeof(sbm_operation_env));
    int_unlock(lock);
}

void audio_audio_scalable_push_sync_custom_data(uint8_t* custom_data)
{
    uint32_t lock = int_lock();
    memcpy((uint8_t *)&sbm_operation_env, custom_data, sizeof(sbm_operation_env));
    int_unlock(lock);
}

static int a2dp_audio_scalable_headframe_info_get(A2DP_AUDIO_HEADFRAME_INFO_T* headframe_info)
{
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    a2dp_audio_scalable_decoder_frame_t *decoder_frame_p = NULL;

    if (a2dp_audio_list_length(list)){
        if ((node = a2dp_audio_list_begin(list)) != NULL){
            decoder_frame_p = (a2dp_audio_scalable_decoder_frame_t *)a2dp_audio_list_node(node);
            headframe_info->sequenceNumber = decoder_frame_p->header.sequenceNumber;
            headframe_info->timestamp = decoder_frame_p->header.timestamp;
            headframe_info->curSubSequenceNumber = 0;
            headframe_info->totalSubSequenceNumber = 0;
        }
    }else{
        memset(headframe_info, 0, sizeof(A2DP_AUDIO_HEADFRAME_INFO_T));
    }

    return A2DP_DECODER_NO_ERROR;
}

static int a2dp_audio_scalable_info_get(void *info)
{
    return A2DP_DECODER_NO_ERROR;
}

static int a2dp_audio_scalable_synchronize_packet(A2DP_AUDIO_SYNCFRAME_INFO_T * sync_info, uint32_t mask)
{
    int nRet = A2DP_DECODER_SYNC_ERROR;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    int list_len;
    a2dp_audio_scalable_decoder_frame_t *decoder_frame_p = NULL;

#ifdef A2DP_CP_ACCEL
    a2dp_cp_reset_frame();
#endif

    list_len = a2dp_audio_list_length(list);

    for (uint16_t i = 0; i < list_len; i++) {
        if ((node = a2dp_audio_list_begin(list)) != NULL){
            decoder_frame_p = (a2dp_audio_scalable_decoder_frame_t *)
                a2dp_audio_list_node(node);
            if (A2DP_AUDIO_SYNCFRAME_CHK(decoder_frame_p->header.sequenceNumber == sync_info->sequenceNumber, A2DP_AUDIO_SYNCFRAME_MASK_SEQ,       mask)&&
                A2DP_AUDIO_SYNCFRAME_CHK(decoder_frame_p->header.timestamp      == sync_info->timestamp,      A2DP_AUDIO_SYNCFRAME_MASK_TIMESTAMP, mask)) {
                nRet = A2DP_DECODER_NO_ERROR;
                break;
            }
            a2dp_audio_list_remove(list, decoder_frame_p);
        }
    }

    node = a2dp_audio_list_begin(list);
    if (node) {
        decoder_frame_p = (a2dp_audio_scalable_decoder_frame_t *)
            a2dp_audio_list_node(node);
        TRACE(4,"%s nRet:%d SEQ:%d timestamp:%d", __func__, nRet,
            decoder_frame_p->header.sequenceNumber, decoder_frame_p->header.timestamp);
    } else {
        TRACE(2,"%s nRet:%d", __func__, nRet);
    }

    return nRet;
}

static int a2dp_audio_scalable_synchronize_dest_packet_mut(uint16_t packet_mut)
{
    list_node_t *node = NULL;
    uint32_t list_len = 0;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    a2dp_audio_scalable_decoder_frame_t *decoder_frame_p = NULL;

    list_len = a2dp_audio_list_length(list);
    if (list_len > packet_mut) {
    do {
        node = a2dp_audio_list_begin(list);
        if (node)
        {
            decoder_frame_p = (a2dp_audio_scalable_decoder_frame_t *)
            a2dp_audio_list_node(node);
            a2dp_audio_list_remove(list, decoder_frame_p);
        }
    } while (a2dp_audio_list_length(list) > packet_mut);
    }

    TRACE(2,"%s list:%d", __func__, a2dp_audio_list_length(list));

    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_scalable_convert_list_to_samples(uint32_t *samples)
{
    uint32_t list_len = 0;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;

    list_len = a2dp_audio_list_length(list);
    *samples = SCALABLE_FRAME_SIZE*list_len;

    TRACE(3, "%s list:%d samples:%d", __func__, list_len, *samples);

    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_scalable_discards_samples(uint32_t samples)
{
    int nRet = A2DP_DECODER_SYNC_ERROR;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    a2dp_audio_scalable_decoder_frame_t *scalable_decoder_frame = NULL;
    list_node_t *node = NULL;
    int need_remove_list = 0;
    uint32_t list_samples = 0;
    ASSERT(!(samples%SCALABLE_FRAME_SIZE), "%s samples err:%d", __func__, samples);

    a2dp_audio_scalable_convert_list_to_samples(&list_samples);
    if (list_samples >= samples){
        need_remove_list = samples/SCALABLE_FRAME_SIZE;
        for (int i = 0; i < need_remove_list; i++){
            node = a2dp_audio_list_begin(list);
            if (node)
            {
                scalable_decoder_frame = (a2dp_audio_scalable_decoder_frame_t *)a2dp_audio_list_node(node);
                a2dp_audio_list_remove(list, scalable_decoder_frame);
            }
        }
        nRet = A2DP_DECODER_NO_ERROR;
    }

    return nRet;
}
extern const A2DP_AUDIO_DECODER_T a2dp_audio_scalable_decoder_config = {
    {44100, 2, 16},
    1,{0},
    a2dp_audio_scalable_init,
    a2dp_audio_scalable_deinit,
    a2dp_audio_scalable_decode_frame,
    a2dp_audio_scalable_preparse_packet,
    a2dp_audio_scalable_store_packet,
    a2dp_audio_scalable_discards_packet,
    a2dp_audio_scalable_synchronize_packet,
    a2dp_audio_scalable_synchronize_dest_packet_mut,
    a2dp_audio_scalable_convert_list_to_samples,
    a2dp_audio_scalable_discards_samples,
    a2dp_audio_scalable_headframe_info_get,
    a2dp_audio_scalable_info_get,
    a2dp_audio_scalable_free,
    NULL,
    a2dp_audio_scalable_latency_custom_tune,
    audio_audio_scalable_get_sync_custom_data,
    audio_audio_scalable_push_sync_custom_data,
};
#else
A2DP_AUDIO_DECODER_T a2dp_audio_scalable_decoder_config = { 0, };
#endif
