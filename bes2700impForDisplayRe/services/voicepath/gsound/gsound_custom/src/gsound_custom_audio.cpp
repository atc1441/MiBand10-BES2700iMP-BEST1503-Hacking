/***************************************************************************
*
*Copyright 2015-2020 BES.
*All rights reserved. All unpublished rights reserved.
*
*No part of this work may be used or reproduced in any form or by any
*means, or stored in a database or retrieval system, without prior written
*permission of BES.
*
*Use of this work is governed by a license granted by BES.
*This work contains confidential and proprietary information of
*BES. which is protected by copyright, trade secret,
*trademark and other intellectual property rights.
*
****************************************************************************/

/*****************************header include********************************/
#include "hal_timer.h"
#include "hal_aud.h"
#include "gsound_dbg.h"
#include "voice_sbc.h"
#include "app_audio.h"
#include "bluetooth_bt_api.h"
#include "app_bt_media_manager.h"
#include "gsound_target_audio.h"
#include "gsound_custom_audio.h"
#include "gsound_custom_bt.h"
#include "audio_dump.h"
#include "app_ai_voice.h"

#ifdef IBRT
#include "app_ibrt_internal.h"
#include "earbud_ux_api.h"
#include "app_tws_ibrt.h"
#endif

#if defined(GSOUND_HOTWORD_EXTERNAL)
#include "hotword_dsp_multi_bank_api.h"
#include "gsound_custom_hotword_external.h"
#endif
#ifdef GSOUND_M55_ENABLED
#include "mcu_dsp_m55_app_ai.h"
#endif
/************************private macro defination***************************/

#if AI_CAPTURE_CHANNEL_NUM == 2
#define MIC_CHANNEL_COUNT (2)
#else
#define MIC_CHANNEL_COUNT (1)
#endif

#define GSOUND_VOICE_VOLUME         (16)
#define GSOUND_VOICE_SAMPLE_RATE    (AUD_SAMPRATE_16000)

// #define BISTO_DUMP_AUDIO (1)
// #define HW_TRACE_PROCESS_TIME (1)

#define DATA_IN_SIGNAL_ID 0x01

/*********************external function declearation************************/

/************************private type defination****************************/

typedef struct
{
    uint8_t micDtatStreaming : 1;
    uint8_t resetEncoder : 1;
    uint8_t blockReason : 2;
    uint8_t glibRequestData : 1;
    uint8_t reserved : 3;

    uint8_t resetEncoderState;
    uint8_t *inputDataBuf;
    uint32_t inputDataLen;
    uint8_t *encodeDataBuf;
    uint32_t encodeDataLen;

    // used to store the audio settings from gsoundLib
    GSoundTargetAudioInSettings libSettings;

    // used to store the audio interface handler
    const GSoundAudioInterface *handler;
} GSOUND_AUDIO_CONTEXT_T;

/**********************private function declearation************************/
/**
 * @brief set resetEncoder flag
 * NOTE: this function should be called when:
 *       1.initialize the SBC encoder(isReset=true)
 *       2.any update of encoder param(isReset=true)
 *       3.updated encoder param took effect(isReset=false)
 *
 * @param isReset flag value to set
 */
static void _reset_codec(uint8_t isReset);

/**
 * @brief configure the param of voice encoder
 *
 */
static void _config_voice_encoder(void);

/**
 * @brief start capture mic data
 *
 */
static void _start_capture_stream(void);

void _init_voicepath(void);
void _deinit_voicepath(void);
uint32_t _mic_data_handle(uint8_t *ptrBuf, uint32_t length);

/************************private variable defination************************/
static GSOUND_AUDIO_CONTEXT_T gAudioEnv;

static VOICE_SBC_CONFIG_T gSbcEncodeCfg = {
    VOICE_SBC_CHANNEL_COUNT,
    VOICE_SBC_CHANNEL_MODE,
    VOICE_SBC_BIT_POOL,
    VOICE_SBC_SIZE_PER_SAMPLE,
    VOICE_SBC_SAMPLE_RATE,
    VOICE_SBC_NUM_BLOCKS,
    VOICE_SBC_NUM_SUB_BANDS,
    VOICE_SBC_MSBC_FLAG,
    VOICE_SBC_ALLOC_METHOD,
};

/// used to mark the output SBC frame length
uint32_t outputSbcFrameLen = 0;

/// used to mark if someone is handling @see outputSbcFrameLen
bool outputSbcFrameLenHandling = false;

double frameCntPerMs = 0.0;

#ifdef HW_TRACE_PROCESS_TIME
static uint32_t last_process_time;
#endif

uint8_t encodedDataBuf[GSOUND_ENCODER_OUTPUT_BUF_SIZE] = {0,};
uint8_t frameDataBuf[VOICE_SBC_PCM_DATA_SIZE_PER_HANDLE] = {0,};
uint32_t readOffsetInAIVoiceBuf = 0;
osMutexDef(read_offset_mutex);
osMutexId read_offset_mutex_id = NULL;

static osThreadId gsound_audio_tid = NULL;
static void gsound_audio_thread(void const *argument);
#ifndef GSOUND_AUDIO_THREAD_SIZE
#define GSOUND_AUDIO_THREAD_SIZE    (1024*2)
#endif
osThreadDef(gsound_audio_thread, osPriorityAboveNormal, 1, GSOUND_AUDIO_THREAD_SIZE, "gsound_hot_word");

/****************************function defination****************************/
static void _reset_codec(uint8_t isReset)
{
    gAudioEnv.resetEncoder = isReset;
    GLOG_D("reset_codec flag is set to:%d", isReset);
}

static void _config_voice_encoder(void)
{
    voice_sbc_init(&gSbcEncodeCfg);
}

static void read_offset_lock(void)
{
    osMutexWait(read_offset_mutex_id, osWaitForever);
}

static void read_offset_unlock(void)
{
    osMutexRelease(read_offset_mutex_id);
}

static uint32_t _get_read_offset_in_ai_voice_buf(void)
{
    read_offset_lock();
    uint32_t offset = readOffsetInAIVoiceBuf;
    read_offset_unlock();

    return offset;
}

void _update_read_offset_in_ai_voice_buf(uint32_t offset)
{
    uint32_t bufLen = app_ai_voice_get_ai_voice_buf_len();

    read_offset_lock();
    GLOG_V("read_offset_in_ai_voice_buf:%d->%d",
           readOffsetInAIVoiceBuf, offset % bufLen);

    readOffsetInAIVoiceBuf = offset % bufLen;
    read_offset_unlock();
}

void gsound_custom_audio_update_streaming_state(bool on)
{
    GLOG_I("streaming state update:%d->%d", gAudioEnv.micDtatStreaming, on);
    gAudioEnv.micDtatStreaming = on;
}

static void _upstream_data_process()
{
    gsound_custom_audio_trigger_data_process();
}

static void _hw_detect_data_process(uint8_t ai)
{
    gsound_custom_audio_trigger_data_process();
}

POSSIBLY_UNUSED static void _hw_detect_update_cp(bool status)
{
#ifdef GSOUND_M55_ENABLED

#else
    if(status)
    {
        GLOG_I("enable cp");
#ifdef GSOUND_HOTWORD_EXTERNAL
        gsound_custom_hotword_external_start_cp_process();
#endif
    }
    else
    {
        GLOG_I("disable cp");
#ifdef GSOUND_HOTWORD_EXTERNAL
        gsound_custom_hotword_external_stop_cp_process();
#endif
    }
#endif    
}

static int _hw_detect_start(void)
{
    GLOG_I("_hw_detect_start");

    _update_read_offset_in_ai_voice_buf(0);
    app_ai_voice_update_work_mode(AIV_USER_GVA, AIV_WORK_MODE_HW_DETECTING);

#ifdef GSOUND_M55_ENABLED

#else
    _hw_detect_update_cp(true);
#endif    
    return 0;
}

static void _hw_detect_stop(void)
{
    GLOG_I("_hw_detect_stop");

#ifdef GSOUND_M55_ENABLED

#else
    _hw_detect_update_cp(false);
#endif    
}

static void _start_capture_stream(void)
{
    //gsound_custom_audio_update_streaming_state(true);

    /// init read offset in AI voice buffer
    _update_read_offset_in_ai_voice_buf(0);
    /// register AI voice related handlers
    AIV_HANDLER_BUNDLE_T handlerBundle = {
        .upstreamStopped        = _update_read_offset_in_ai_voice_buf,
        .streamStarted          = _hw_detect_start,
        .streamStopped          = _hw_detect_stop,
        .upstreamDataHandler    = _upstream_data_process,
        .hwDetectHandler        = _hw_detect_data_process,
    };
    app_ai_voice_register_ai_handler(AIV_USER_GVA, &handlerBundle);

#ifndef GSOUND_M55_ENABLED
    // update the SBC encoder confiuration
    _reset_codec(true);

#ifdef GSOUND_HOTWORD_EXTERNAL
    gsound_custom_hotword_external_init_buffer();
#endif
    /// init encoder
    _config_voice_encoder();
#endif

#ifdef BISTO_DUMP_AUDIO
    audio_dump_init(VOICE_SBC_PCM_DATA_SIZE_PER_HANDLE / VOICE_SBC_SIZE_PER_SAMPLE, VOICE_SBC_SIZE_PER_SAMPLE, 1);
#endif

#ifdef GSOUND_HOTWORD_EXTERNAL
    /// init cp to start processing data
  //  gsound_custom_hotword_external_start_cp_process();
#endif

    /// the audio functionality is managed by audio manager, so
    /// we start the capture stream here
    int ret = app_ai_voice_stream_control(true, AIV_USER_GVA);
    if(ret >= 0)
    {
        gsound_custom_audio_update_streaming_state(true);
    }

#ifdef HW_TRACE_PROCESS_TIME
    last_process_time = hal_fast_sys_timer_get();
#endif
}

void gsound_custom_audio_init(void)
{
    GLOG_I("%s", __func__);

    memset((uint8_t *)&gAudioEnv, 0, sizeof(gAudioEnv));

    if (NULL == gsound_audio_tid)
    {
        GLOG_I("Create gsound audio thread");
        gsound_audio_tid = osThreadCreate(osThread(gsound_audio_thread), NULL);

        if (NULL == gsound_audio_tid)
        {
            ASSERT(0, "thread create failed");
        }
    }

    if (NULL == read_offset_mutex_id)
    {
        read_offset_mutex_id = osMutexCreate((osMutex(read_offset_mutex)));

        ASSERT(read_offset_mutex_id, "mutex create failed.");
    }
}

uint32_t gsound_custom_audio_get_output_sbc_frame_len(void)
{
    /// wait until outputSbcFrameLen handling done
    while (outputSbcFrameLenHandling)
    {
        hal_sys_timer_delay_us(10);
    };

    return outputSbcFrameLen;
}

double gsound_custom_audio_get_sbc_frame_cnt_per_ms(void)
{
    return frameCntPerMs;
}

void gsound_custom_audio_set_libgsound_request_data_flag(bool flag)
{
    GLOG_I("glib request data flag update:%d->%d", gAudioEnv.glibRequestData, flag);
    gAudioEnv.glibRequestData = flag;
#ifdef GSOUND_M55_ENABLED
    uint8_t request_flag = gAudioEnv.glibRequestData;
    app_dsp_m55_mcu_ai_send_msg(DSP_M55_AI_USER_GSOUND, DSP_M55_AI_GSOUND_CMD_SET_REQ_DATA_FLAG, (void*)&request_flag, sizeof(request_flag));
#endif
}

void gsound_custom_audio_store_audio_in_settings(const void *config)
{
    memcpy((void *)&gAudioEnv.libSettings, (void *)config, sizeof(GSoundTargetAudioInSettings));
    GLOG_D("received audio in settings");
    GLOG_D("bitpool:%d", gAudioEnv.libSettings.sbc_bitpool);
    GLOG_D("enable_sidetone:%d", gAudioEnv.libSettings.enable_sidetone);
    GLOG_D("raw_samples_required:%d", gAudioEnv.libSettings.raw_samples_required);
    GLOG_D("include_sbc_headers:%d", gAudioEnv.libSettings.include_sbc_headers);
}

void gsound_custom_audio_store_lib_interface(const void *handler)
{
    gAudioEnv.handler = (GSoundAudioInterface *)handler;

#ifdef GSOUND_M55_ENABLED
    app_dsp_m55_mcu_ai_send_msg(DSP_M55_AI_USER_GSOUND, DSP_M55_AI_GSOUND_CMD_AUDIO_INTERFACE_INIT, (void*)gAudioEnv.handler, sizeof(gAudioEnv.handler));
#endif
}

uint32_t gsound_custom_audio_get_bitpool(void)
{
    return gAudioEnv.libSettings.sbc_bitpool;
}

void gsound_custom_audio_open_mic(int sbc_bitpool)
{
    GLOG_I("%s bitpool:%d, micDataStreaming:%d",
           __func__,
           sbc_bitpool,
           gAudioEnv.micDtatStreaming);

#ifndef GSOUND_M55_ENABLED
    // check if need to updata bitpool
    if (gSbcEncodeCfg.bitPool != sbc_bitpool)
    {
        GLOG_I("Run-time update gsound bitpool from %d to %d",
               gSbcEncodeCfg.bitPool,
               sbc_bitpool);

        // update the codec info
        gSbcEncodeCfg.bitPool = sbc_bitpool;

        while (STATE_BUSY == gAudioEnv.resetEncoderState)
        {
            hal_sys_timer_delay_us(10);
        }
        gAudioEnv.resetEncoderState = STATE_BUSY;

        // set the reset encoder flag for the next round sbc encoding to reset the encoder env
        _reset_codec(true);
        gAudioEnv.resetEncoderState = STATE_IDLE;
    }
#endif
    if (gAudioEnv.micDtatStreaming)
    {
        GLOG_I("mic data already streaming");
    }
    else
    {
#ifdef IBRT
        if (TWS_UI_SLAVE == app_ibrt_if_get_ui_role())
        {
            GLOG_I("slave cannot open mic.");
            gAudioEnv.blockReason = AUDIO_BLOCK_REASON_ROLE;
        }
        else
#endif
        {
            _start_capture_stream();
        }
    }
#ifdef GSOUND_M55_ENABLED
    app_dsp_m55_mcu_ai_send_msg(DSP_M55_AI_USER_GSOUND, DSP_M55_AI_GSOUND_CMD_OPEN_MIC, &sbc_bitpool, sizeof(sbc_bitpool));
#endif    
}

static uint32_t _audio_encode_mic_data(uint8_t *inputDataBuf,
                                       uint32_t inputDataLen,
                                       uint8_t *encodeDataBuf)
{
    ASSERT(inputDataBuf, "input buf is null!");
    ASSERT(inputDataLen, "input data len is 0");
    ASSERT(encodeDataBuf, "encoded buf is null");

    uint32_t pcmEncodedLen = GSOUND_ENCODER_OUTPUT_BUF_SIZE;
    uint32_t sbcFrameLen = 0;

    while (STATE_BUSY == gAudioEnv.resetEncoderState)
    {
        hal_sys_timer_delay_us(10);
    }

    gAudioEnv.resetEncoderState = STATE_BUSY;
    bool resetEncoder = gAudioEnv.resetEncoder;

    // check if codec need to reset
    if (resetEncoder)
    {
        _config_voice_encoder();

        // clear the reset codec flag
        _reset_codec(false);
    }

    uint32_t encodedDataLen = voice_sbc_encode(inputDataBuf,
                                               inputDataLen,
                                               &pcmEncodedLen,
                                               encodeDataBuf,
                                               resetEncoder);
    ASSERT(GSOUND_ENCODER_OUTPUT_BUF_SIZE >= encodedDataLen,
           "encoded data length %d exceeded buffer length %d.",
           encodedDataLen, GSOUND_ENCODER_OUTPUT_BUF_SIZE);

    gAudioEnv.resetEncoderState = STATE_IDLE;

    sbcFrameLen = voice_sbc_get_frame_len();
    GLOG_V("raw data len:%d, encodedDataLen:%d, sbcFrameLen:%d",
           gAudioEnv.inputDataLen, encodedDataLen, sbcFrameLen);

    /// mark that handling outputSbcFrameLen now
    outputSbcFrameLenHandling = true;
    outputSbcFrameLen = sbcFrameLen;

    /// calculate the frame count
    uint32_t frameCount = encodedDataLen / sbcFrameLen;
    frameCntPerMs = ((double)frameCount) / ((double)VOICE_SBC_HANDLE_INTERVAL_MS);
    GLOG_V("sbc frameCntPerSecond is %d, frameCount:%d",
           (uint32_t)(frameCntPerMs * 1000), frameCount);

    /// remove the SBC frame header if needed
    if (!gAudioEnv.libSettings.include_sbc_headers)
    {
        // trimed the frame header as libgsound required
        // Note: it is assumed that the sbc encoder will
        // return an integer number of frames(no partial frames)
        outputSbcFrameLen -= VOICE_SBC_FRAME_HEADER_LEN;

        uint32_t offsetToFill = 0;
        uint32_t offsetToFetch = VOICE_SBC_FRAME_HEADER_LEN;

        for (uint8_t frame = 0; frame < frameCount; frame++)
        {
            memmove((encodeDataBuf + offsetToFill),
                    (encodeDataBuf + offsetToFetch),
                    outputSbcFrameLen);

            offsetToFill += outputSbcFrameLen;
            offsetToFetch += sbcFrameLen;
        }

        encodedDataLen = offsetToFill;
    }

    /// mark that handling outputSbcFrameLen done
    outputSbcFrameLenHandling = false;

    return encodedDataLen;
}

void gsound_custom_audio_transmit_data_to_libgsound()
{
#ifdef GSOUND_M55_ENABLED
	extern GSOUND_UPSTREAM_DATA_T gsound_upstream_data;
    gAudioEnv.inputDataBuf = gsound_upstream_data.raw_data_ptr;
    gAudioEnv.inputDataLen = gsound_upstream_data.raw_data_len;
    gAudioEnv.encodeDataBuf = gsound_upstream_data.encode_data_ptr;
    gAudioEnv.encodeDataLen = gsound_upstream_data.encode_data_len;
    outputSbcFrameLen = gsound_upstream_data.output_sbc_frame_len;
#endif
    if (gAudioEnv.glibRequestData)
    {
        GLOG_D("gsound process data");

#if defined(GSOUND_HOTWORD_ENABLED)
        uint32_t rawDataLen = 0;
        const uint16_t *rawDataBuf = NULL;

        if (gAudioEnv.libSettings.raw_samples_required)
        {
            rawDataBuf = (const uint16_t *)gAudioEnv.inputDataBuf;
            rawDataLen = gAudioEnv.inputDataLen / VOICE_SBC_SIZE_PER_SAMPLE;
        }

        if (GSOUND_STATUS_OK != gAudioEnv.handler->gsound_target_on_audio_in_ex(rawDataBuf,
                                                                                rawDataLen,
                                                                                gAudioEnv.encodeDataBuf,
                                                                                gAudioEnv.encodeDataLen,
                                                                                outputSbcFrameLen))
        {
            GLOG_W("no room in audio buffer of libgsound");
        }
#else  // #if !defined(GSOUND_HOTWORD_ENABLE)
        gAudioEnv.handler->gsound_target_on_audio_in(gAudioEnv.encodeDataBuf,
                                                     gAudioEnv.encodeDataLen,
                                                     outputSbcFrameLen);
#endif // #if defined(GSOUND_HOTWORD_ENABLE)
    }
    else
    {
        GLOG_W("glib not require data");
    }
}

uint32_t gsound_custom_audio_handle_mic_data(uint8_t *inputBuf,
                                             uint32_t inputLen,
                                             uint8_t *outputBuf)
{
    if ((NULL != inputBuf) && (0 != inputLen) && (NULL != outputBuf))
    {
        gAudioEnv.inputDataBuf = inputBuf;
        gAudioEnv.inputDataLen = inputLen;
        gAudioEnv.encodeDataBuf = outputBuf;

#ifdef HW_TRACE_PROCESS_TIME
        uint32_t start_time = hal_fast_sys_timer_get();
#endif
        // encode mic data into SBC frames
        gAudioEnv.encodeDataLen = _audio_encode_mic_data(inputBuf, inputLen, outputBuf);

#ifdef HW_TRACE_PROCESS_TIME
        TRACE(1,"encode cost : %d",FAST_TICKS_TO_US(hal_fast_sys_timer_get() - start_time));
#endif
#if defined(GSOUND_HOTWORD_EXTERNAL)
        // cache data into hotword buffer
        gsound_custom_hotword_external_process_data(inputBuf,
                                                    inputLen,
                                                    outputBuf,
                                                    gAudioEnv.encodeDataLen,
                                                    gAudioEnv.glibRequestData);
#else
        gsound_custom_audio_transmit_data_to_libgsound();
#endif
#ifdef HW_TRACE_PROCESS_TIME
        TRACE(1,"glib cost : %d",FAST_TICKS_TO_US(hal_fast_sys_timer_get() - start_time));
#endif
    }
    else
    {
        gAudioEnv.encodeDataLen = 0;
        GLOG_W("INVALID input param, inputBuf:%p, inputLen:%d, outputBuf:%p",
               inputBuf,
               inputLen,
               outputBuf);
    }

    return gAudioEnv.encodeDataLen;
}

void gsound_custom_audio_close_mic(void)
{
    GLOG_D("%s", __func__);

    if (!gAudioEnv.micDtatStreaming)
    {
        GLOG_D("%s: not streaming", __func__);

        if (AUDIO_BLOCK_REASON_ROLE == gAudioEnv.blockReason)
        {
            gAudioEnv.blockReason = AUDIO_BLOCK_REASON_NULL;
        }
    }
    else
    {
        // stop the capture steam via the BT stream manager
        app_ai_voice_stream_control(false, AIV_USER_GVA);


#ifdef GSOUND_HOTWORD_EXTERNAL
        gsound_custom_hotword_external_deinit_buffer();
#endif

        gAudioEnv.micDtatStreaming = false;

        if (gAudioEnv.blockReason == AUDIO_BLOCK_REASON_NULL)
        {
            gAudioEnv.libSettings.raw_samples_required = false;
        }
    }
#ifdef GSOUND_M55_ENABLED
    app_dsp_m55_mcu_ai_send_msg(DSP_M55_AI_USER_GSOUND, DSP_M55_AI_GSOUND_CMD_CLOSE_MIC, NULL, 0);
#endif
}

void gsound_custom_audio_in_stream_interrupted_handler(uint8_t reason)
{
    if (gAudioEnv.micDtatStreaming && gAudioEnv.handler)
    {
        gAudioEnv.handler->audio_in_stream_interrupted((GSoundTargetAudioInStreamIntReason)reason);
    }
}

#ifdef IBRT
void gsound_custom_audio_role_switch_complete_handler(uint8_t role)
{
    if ((IBRT_MASTER == role) && (AUDIO_BLOCK_REASON_ROLE == gAudioEnv.blockReason))
    {
        gAudioEnv.blockReason = AUDIO_BLOCK_REASON_NULL;
        gsound_custom_audio_open_mic(gSbcEncodeCfg.bitPool);
    }
    else if (IBRT_SLAVE == role && gAudioEnv.micDtatStreaming)
    {
        gAudioEnv.blockReason = AUDIO_BLOCK_REASON_ROLE;
        gsound_custom_audio_close_mic();
    }
}
#endif

POSSIBLY_UNUSED static void _on_capture_stream_started(void)
{
    GLOG_D("%s", __func__);
}

POSSIBLY_UNUSED static void _on_capture_stream_stopped(void)
{
#ifdef GSOUND_HOTWORD_EXTERNAL
    gsound_custom_hotword_external_stop_cp_process();
#endif
}

void gsound_custom_audio_on_upstream_started(void)
{
    /// stop sniff to optimize the performance
    gsound_custom_bt_stop_sniff();
    /// monopolize the capture stream
    app_ai_voice_request_monopolize_stream(AIV_USER_GVA);
}

void gsound_custom_audio_on_upstream_stopped(void)
{
    /// allow sniff to save power consumption
    //gsound_custom_bt_allow_sniff();

    /// release the capture stream monopolize
    app_ai_voice_request_share_stream(AIV_USER_GVA, _get_read_offset_in_ai_voice_buf());
}

uint32_t _mic_data_handle(uint8_t *ptrBuf, uint32_t length)
{
#ifdef HW_TRACE_PROCESS_TIME
    uint32_t stime;
    uint32_t etime;

    stime = hal_fast_sys_timer_get();
#endif

#ifdef BISTO_DUMP_AUDIO
    audio_dump_add_channel_data(0, (void *)gAudioEnv.inputDataBuf, gAudioEnv.inputDataLen / VOICE_SBC_SIZE_PER_SAMPLE);
#endif

#ifdef GSOUND_M55_ENABLED
    GSOUND_PCM_TO_M55_T gsound_pcm;
    gsound_pcm.in_ptr = ptrBuf;
    gsound_pcm.out_ptr = encodedDataBuf;
    gsound_pcm.length = length;
    app_dsp_m55_mcu_ai_send_msg_with_waiting_rsp(DSP_M55_AI_USER_GSOUND, DSP_M55_AI_GSOUND_CMD_MIC_DATA, &gsound_pcm, sizeof(gsound_pcm));
#else
#ifdef GSOUND_HOTWORD_EXTERNAL
    // inform co-processor to handle data
    gsound_custom_hotword_external_handle_mic_data(ptrBuf, length, encodedDataBuf);
#else
    gsound_custom_audio_handle_mic_data(ptrBuf, length, encodedDataBuf);
#endif
#endif

#ifdef HW_TRACE_PROCESS_TIME
    etime = hal_fast_sys_timer_get();
    TRACE(1, "cp_decode: %5u us in %5u us",
           FAST_TICKS_TO_US(etime - stime),
           FAST_TICKS_TO_US(etime - last_process_time));
    last_process_time = etime;
#endif

#ifdef BISTO_DUMP_AUDIO
    audio_dump_run();
#endif

    return 0;
}

void gsound_custom_audio_trigger_data_process()
{
    uint32_t cachedDataLen = app_ai_voice_get_cached_len_according_to_read_offset(_get_read_offset_in_ai_voice_buf());

    if (cachedDataLen >= VOICE_SBC_PCM_DATA_SIZE_PER_HANDLE)
    {
        osSignalSet(gsound_audio_tid, DATA_IN_SIGNAL_ID);
    }
}

static void gsound_audio_thread(void const *argument)
{
    osEvent evt;
    uint32_t cachedDataLen = 0;

    while (1)
    {
        /// wait for data income signal
        evt = osSignalWait(DATA_IN_SIGNAL_ID, osWaitForever);

        /// signal coming
        if(evt.status == osEventSignal)
        {
            uint32_t read_offset = _get_read_offset_in_ai_voice_buf();
            cachedDataLen = app_ai_voice_get_cached_len_according_to_read_offset(read_offset);
            while (cachedDataLen >= VOICE_SBC_PCM_DATA_SIZE_PER_HANDLE)
            {
                /// fetch data from AI voice PCM data cache buffer
                if(app_ai_voice_get_pcm_data(read_offset, frameDataBuf, VOICE_SBC_PCM_DATA_SIZE_PER_HANDLE))
                {
                    /// process a frame of cached PCM data
                    _mic_data_handle(frameDataBuf, VOICE_SBC_PCM_DATA_SIZE_PER_HANDLE);

                    if (read_offset != _get_read_offset_in_ai_voice_buf())
                    {
                        _update_read_offset_in_ai_voice_buf(0);
                        cachedDataLen = 0;
                        break;
                    }

                    /// update read offset in AI voice PCM data cache buffer
                    _update_read_offset_in_ai_voice_buf(read_offset + VOICE_SBC_PCM_DATA_SIZE_PER_HANDLE);

                    read_offset = _get_read_offset_in_ai_voice_buf();
                    /// update cached data length after read offset updated
                    cachedDataLen = app_ai_voice_get_cached_len_according_to_read_offset(read_offset);
                }
            }
        }
    }
}
