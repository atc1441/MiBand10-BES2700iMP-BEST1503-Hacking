/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#include "gaf_codec_cc_common.h"
#include "cmsis.h"
#include "heap_api.h"
#include "app_gaf_dbg.h"

#define GAF_SEQ_GAP_THRESHOULD_TO_RETRIGGER 32

#ifdef GAF_CODEC_CROSS_CORE

extern heap_handle_t gaf_decode_m55_heap_handle;
GAF_AUDIO_M55_DEINIT_T gaf_m55_deinit_status;
bool is_support_ble_audio_mobile_m55_decode = false;
bool is_playback_state = false;
bool is_capture_state  = false;

void gaf_audio_stream_set_playback_state(bool enable)
{
    is_playback_state = enable;
}

void gaf_audio_stream_set_capture_state(bool enable)
{
    is_capture_state = enable;
    return;
}

/**
 ****************************************************************************************
 * @brief It is used to adjust the media bth freqency, when open m55 core.
 *
 * @param[in] frame_size                 Frame size
 * @param[in] channel                    The number of channel
 * @param[in] frame_ms                   The during time of one frame
 * @param[in] sample_rate                Sample freqency
 * @param[in] _base_freq_                base freqency
 *
 * @param[out] gaf_audio_bth_base_freq       return frequency value
 ****************************************************************************************
 */
enum APP_SYSFREQ_FREQ_T gaf_audio_media_decoder_adjust_bth_freq(uint16_t frame_size, uint8_t channel,
    uint8_t frame_ms, uint32_t sample_rate, enum APP_SYSFREQ_FREQ_T _base_freq_)
{
    enum APP_SYSFREQ_FREQ_T gaf_audio_bth_base_freq = _base_freq_;
    uint32_t lc3_bitrate = frame_size * 80000 / frame_ms;

    if (44100 == sample_rate){
        lc3_bitrate = lc3_bitrate * 44100 / 48000;
    }
    LOG_I("BTH_LC3_bitrate:%d", lc3_bitrate);

    /// media scence
    if ((16000 == lc3_bitrate) && (8000 == sample_rate)){
        gaf_audio_bth_base_freq = APP_SYSFREQ_12M;
    }
    else if ((16000 == lc3_bitrate) && (16000 == sample_rate)){
        gaf_audio_bth_base_freq = APP_SYSFREQ_12M;
    }
    else if ((32000 == lc3_bitrate) && (16000 == sample_rate)){
        gaf_audio_bth_base_freq = APP_SYSFREQ_12M;
    }
    else if ((32000 == lc3_bitrate) && (32000 == sample_rate)){
        gaf_audio_bth_base_freq = APP_SYSFREQ_12M;
    }
    else if ((48000 == lc3_bitrate) && (32000 == sample_rate)){
        gaf_audio_bth_base_freq = APP_SYSFREQ_12M;
    }
    else if ((64000 == lc3_bitrate) && (32000 == sample_rate)){
        gaf_audio_bth_base_freq = APP_SYSFREQ_12M;
    }
    else if ((88200 == lc3_bitrate) && (44100 == sample_rate)){
        gaf_audio_bth_base_freq = APP_SYSFREQ_15M;
    }
    else if ((96000 == lc3_bitrate) && (48000 == sample_rate)){
        gaf_audio_bth_base_freq = APP_SYSFREQ_15M;
    }
    else if ((96000 < lc3_bitrate) && (48000 < sample_rate))
    {
        gaf_audio_bth_base_freq = APP_SYSFREQ_30M;
    }
    return gaf_audio_bth_base_freq;
}

/**
 ****************************************************************************************
 * @brief It is used to adjust the flexible bth freqency, when open m55 core.
 *
 * @param[in] frame_size                 Frame size
 * @param[in] channel                    The number of channel
 * @param[in] frame_ms                   The during time of one frame
 * @param[in] sample_rate                Sample freqency
 * @param[in] _base_freq_                base freqency
 *
 * @param[out] gaf_audio_bth_base_freq       return frequency value
 ****************************************************************************************
 */
enum APP_SYSFREQ_FREQ_T gaf_audio_flexible_adjust_bth_freq(uint16_t frame_size, uint8_t channel,
    uint8_t frame_ms, uint32_t sample_rate, enum APP_SYSFREQ_FREQ_T _base_freq_, bool is_playback, bool is_capture)
{
    enum APP_SYSFREQ_FREQ_T gaf_audio_base_freq = _base_freq_;
    uint32_t lc3_bitrate = frame_size * 80000 / frame_ms;

    if (44100 == sample_rate){
        lc3_bitrate = lc3_bitrate * 44100 / 48000;
    }
    LOG_I("LC3_bitrate:%d", lc3_bitrate);

    /// call scence
    if ((true == is_playback) &&(true == is_capture)){
        if ((16000 == lc3_bitrate) && (8000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_26M;
        }
        else if ((16000 == lc3_bitrate) && (16000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_26M;
        }
        else if ((32000 == lc3_bitrate) && (16000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_30M;
        }
        else if ((32000 == lc3_bitrate) && (32000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_48M;
        }
        else if ((48000 == lc3_bitrate) && (32000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_48M;
        }
        else if ((64000 == lc3_bitrate) && (32000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_48M;
        }
        else if ((88200 == lc3_bitrate) && (44100 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_48M;
        }
        else if ((96000 == lc3_bitrate) && (48000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_48M;
        }
        else if ((96000 < lc3_bitrate) && (48000 < sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_48M;
        }
    }
    /// ai or live scence
    else if ((false == is_playback) &&(true == is_capture)){
            if ((16000 == lc3_bitrate) && (8000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_30M;
        }
        else if ((16000 == lc3_bitrate) && (16000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_30M;
        }
        else if ((32000 == lc3_bitrate) && (16000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_30M;
        }
        else if ((32000 == lc3_bitrate) && (32000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_48M;
        }
        else if ((48000 == lc3_bitrate) && (32000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_48M;
        }
        else if ((64000 == lc3_bitrate) && (32000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_48M;
        }
        else if ((88200 == lc3_bitrate) && (44100 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_48M;
        }
        else if ((96000 == lc3_bitrate) && (48000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_48M;
        }
        else if ((96000 < lc3_bitrate) && (48000 < sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_48M;
        }
    }
    else if((true == is_playback) &&(false == is_capture)){
        if ((16000 == lc3_bitrate) && (8000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_30M;
        }
        else if ((16000 == lc3_bitrate) && (16000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_30M;
        }
        else if ((32000 == lc3_bitrate) && (16000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_30M;
        }
        else if ((32000 == lc3_bitrate) && (32000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_48M;
        }
        else if ((48000 == lc3_bitrate) && (32000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_48M;
        }
        else if ((64000 == lc3_bitrate) && (32000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_48M;
        }
        else if ((88200 == lc3_bitrate) && (44100 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_48M;
        }
        else if ((96000 == lc3_bitrate) && (48000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_48M;
        }
        else if ((96000 < lc3_bitrate) && (48000 < sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_48M;
        }
    }

    TRACE(0, "%s,  [freq] %d", __func__, gaf_audio_base_freq);
    return gaf_audio_base_freq;
}

/**
 ****************************************************************************************
 * @brief It is used to adjust the m55 decode freqency
 *
 * @param[in] frame_size                 Frame size
 * @param[in] channel                    The number of channel
 * @param[in] frame_ms                   The during time of one frame
 * @param[in] sample_rate                Sample freqency
 * @param[in] _base_freq_                base freqency
 *
 * @param[out] gaf_audio_base_freq       return frequency value
 ****************************************************************************************
 */
enum APP_SYSFREQ_FREQ_T gaf_audio_stream_decoder_adjust_m55_freq(uint16_t frame_size, uint8_t channel,
    uint8_t frame_ms, uint32_t sample_rate, enum APP_SYSFREQ_FREQ_T _base_freq_,
    bool is_playback, bool is_capture)
{
    enum APP_SYSFREQ_FREQ_T gaf_audio_base_freq = _base_freq_;
    uint32_t lc3_bitrate = frame_size * 80000 / frame_ms;

    if (44100 == sample_rate){
        lc3_bitrate = lc3_bitrate * 44100 / 48000;
    }
    LOG_I("LC3_bitrate:%d", lc3_bitrate);

    /// media scence
    /// m55 core cpu freq min value is 15M
    if ((true == is_playback) &&(false == is_capture)){
        if ((16000 == lc3_bitrate) && (8000 == sample_rate)){
#ifdef HELIUM_LC3_CODEC_ENABLED
            gaf_audio_base_freq = APP_SYSFREQ_15M;
#else
            gaf_audio_base_freq = APP_SYSFREQ_26M;
#endif
        }
        else if ((16000 == lc3_bitrate) && (16000 == sample_rate)){
#ifdef HELIUM_LC3_CODEC_ENABLED
            gaf_audio_base_freq = APP_SYSFREQ_15M;
#else
            gaf_audio_base_freq = APP_SYSFREQ_26M;
#endif
        }
        else if ((32000 == lc3_bitrate) && (16000 == sample_rate)){
#ifdef HELIUM_LC3_CODEC_ENABLED
            gaf_audio_base_freq = APP_SYSFREQ_15M;
#else
            gaf_audio_base_freq = APP_SYSFREQ_26M;
#endif
        }
        else if ((32000 == lc3_bitrate) && (32000 == sample_rate)){
#ifdef HELIUM_LC3_CODEC_ENABLED
            gaf_audio_base_freq = APP_SYSFREQ_15M;
#else
            gaf_audio_base_freq = APP_SYSFREQ_26M;
#endif
        }
        else if ((48000 == lc3_bitrate) && (32000 == sample_rate)){
#ifdef HELIUM_LC3_CODEC_ENABLED
            gaf_audio_base_freq = APP_SYSFREQ_15M;
#else
            gaf_audio_base_freq = APP_SYSFREQ_26M;
#endif
        }
        else if ((64000 == lc3_bitrate) && (32000 == sample_rate)){
#ifdef HELIUM_LC3_CODEC_ENABLED
            gaf_audio_base_freq = APP_SYSFREQ_15M;
#else
            gaf_audio_base_freq = APP_SYSFREQ_26M;
#endif
        }
        else if ((88200 == lc3_bitrate) && (44100 == sample_rate)){
#ifdef HELIUM_LC3_CODEC_ENABLED
            gaf_audio_base_freq = APP_SYSFREQ_15M;
#else
            gaf_audio_base_freq = APP_SYSFREQ_26M;
#endif
        }
        else if ((96000 == lc3_bitrate) && (48000 == sample_rate)){
#ifdef HELIUM_LC3_CODEC_ENABLED
            gaf_audio_base_freq = APP_SYSFREQ_15M;
#else
            gaf_audio_base_freq = APP_SYSFREQ_26M;
#endif
        }
        else if ((96000 < lc3_bitrate) && (48000 < sample_rate))
        {
#ifdef HELIUM_LC3_CODEC_ENABLED
            gaf_audio_base_freq = APP_SYSFREQ_30M;
#else
            gaf_audio_base_freq = APP_SYSFREQ_52M;
#endif
        }
    }
    /// call scence
    else if ((true == is_playback) &&(true == is_capture)){
        if ((16000 == lc3_bitrate) && (8000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_15M;
        }
        else if ((16000 == lc3_bitrate) && (16000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_15M;
        }
        else if ((32000 == lc3_bitrate) && (16000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_26M;
        }
        else if ((32000 == lc3_bitrate) && (32000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_26M;
        }
        else if ((48000 == lc3_bitrate) && (32000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_30M;
        }
        else if ((64000 == lc3_bitrate) && (32000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_30M;
        }
        else if ((88200 == lc3_bitrate) && (44100 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_48M;
        }
        else if ((96000 == lc3_bitrate) && (48000 == sample_rate)){
            gaf_audio_base_freq = APP_SYSFREQ_48M;
        }
        else if ((96000 < lc3_bitrate) && (48000 < sample_rate))
        {
            gaf_audio_base_freq = APP_SYSFREQ_52M;
        }
    }
    return gaf_audio_base_freq;
}

/**
 ****************************************************************************************
 * @brief It is used to adjust the m55 encoder freqency
 *
 * @param[in] frame_size                 Frame size
 * @param[in] channel                    The number of channel
 * @param[in] frame_ms                   The during time of one frame
 * @param[in] sample_rate                Sample freqency
 * @param[in] _base_freq_                base freqency
 *
 * @param[out] gaf_audio_m55_base_freq   return frequency value
 ****************************************************************************************
 */
enum APP_SYSFREQ_FREQ_T gaf_audio_stream_encoder_adjust_m55_freq(uint16_t frame_size, uint8_t channel,
    uint8_t frame_ms, uint32_t sample_rate, enum APP_SYSFREQ_FREQ_T _base_freq_,
    bool is_playback, bool is_capture)
{
    enum APP_SYSFREQ_FREQ_T gaf_audio_m55_base_freq = _base_freq_;
    uint32_t lc3_bitrate = frame_size * 80000 / frame_ms;

    if (44100 == sample_rate){
        lc3_bitrate = lc3_bitrate * 44100 / 48000;
    }
    LOG_I("LC3_bitrate:%d", lc3_bitrate);

    if (1 == channel){
        if ((16000 == lc3_bitrate) && (8000 == sample_rate)){
            gaf_audio_m55_base_freq = APP_SYSFREQ_30M;
        }
        else if ((16000 == lc3_bitrate) && (16000 == sample_rate)){
            gaf_audio_m55_base_freq = APP_SYSFREQ_30M;
        }
        else if ((32000 == lc3_bitrate) && (16000 == sample_rate)){
            gaf_audio_m55_base_freq = APP_SYSFREQ_52M;
        }
        else if ((32000 == lc3_bitrate) && (32000 == sample_rate)){
            gaf_audio_m55_base_freq = APP_SYSFREQ_52M;
        }
        else if ((48000 == lc3_bitrate) && (32000 == sample_rate)){
            gaf_audio_m55_base_freq = APP_SYSFREQ_78M;
        }
        else if ((64000 == lc3_bitrate) && (32000 == sample_rate)){
            gaf_audio_m55_base_freq = APP_SYSFREQ_78M;
        }
        else if ((88200 == lc3_bitrate) && (44100 == sample_rate)){
            gaf_audio_m55_base_freq = APP_SYSFREQ_78M;
        }
        else if ((96000 == lc3_bitrate) && (48000 == sample_rate)){
            gaf_audio_m55_base_freq = APP_SYSFREQ_78M;
        }
        else if ((96000 < lc3_bitrate) && (48000 < sample_rate))
        {
            gaf_audio_m55_base_freq = APP_SYSFREQ_104M;
        }
    }//mobile role
    else if (2 == channel){
        if ((32000 == lc3_bitrate) && (8000 == sample_rate)){
            gaf_audio_m55_base_freq = APP_SYSFREQ_78M;
        }
        else if ((32000 == lc3_bitrate) && (16000 == sample_rate)){
            gaf_audio_m55_base_freq = APP_SYSFREQ_78M;
        }
        else if ((64000 == lc3_bitrate) && (16000 == sample_rate)){
            gaf_audio_m55_base_freq = APP_SYSFREQ_78M;
        }
        else if ((64000 == lc3_bitrate) && (32000 == sample_rate)){
            gaf_audio_m55_base_freq = APP_SYSFREQ_104M;
        }
        else if ((96000 == lc3_bitrate) && (32000 == sample_rate)){
            gaf_audio_m55_base_freq = APP_SYSFREQ_104M;
        }
        else if ((128000 == lc3_bitrate) && (32000 == sample_rate)){
            gaf_audio_m55_base_freq = APP_SYSFREQ_104M;
        }
        else if ((176400 == lc3_bitrate) && (44100 == sample_rate)){
            gaf_audio_m55_base_freq = APP_SYSFREQ_104M;
        }
        else if ((192000 == lc3_bitrate) && (48000 == sample_rate)){
            gaf_audio_m55_base_freq = APP_SYSFREQ_104M;
        }
        else if ((192000 < lc3_bitrate) && (48000 < sample_rate))
        {
            gaf_audio_m55_base_freq = APP_SYSFREQ_144M;
        }
    }

    return gaf_audio_m55_base_freq;
}

uint16_t gaf_decoder_cc_check_seq_num(uint16_t actualseq, uint16_t expectedseq)
{
    uint16_t checkResult = 0;
    if (expectedseq == actualseq)
    {
        return GAF_DECODER_SEQ_CHECK_RESULT_MATCH;
    }
    else
    {
        if (ABS((int32_t)expectedseq - (int32_t)actualseq) > GAF_SEQ_GAP_THRESHOULD_TO_RETRIGGER)
        {
            checkResult |= GAF_DECODER_SEQ_CHECK_RESULT_DIFF_EXCEEDS_LIMIT;
        }

        if (expectedseq != actualseq)
        {
            if (expectedseq > 0)
            {
                if (actualseq > expectedseq)
                {
                    checkResult |= GAF_DECODER_SEQ_CHECK_RESULT_LATER_THAN_EXPECTED;
                }
                else
                {
                    checkResult |= GAF_DECODER_SEQ_CHECK_RESULT_EARLIER_THAN_EXPECTED;
                }
            }
            else
            {
                // roll back case
                if (actualseq > 0xFF00)
                {
                    checkResult |= GAF_DECODER_SEQ_CHECK_RESULT_EARLIER_THAN_EXPECTED;
                }
                else
                {
                    checkResult |= GAF_DECODER_SEQ_CHECK_RESULT_LATER_THAN_EXPECTED;
                }
            }
       }
    }
    return checkResult;
}
#endif /// GAF_CODEC_CROSS_CORE