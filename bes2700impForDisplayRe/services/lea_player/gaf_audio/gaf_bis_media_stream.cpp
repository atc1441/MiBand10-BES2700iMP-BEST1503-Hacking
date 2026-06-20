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
/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#if BLE_AUDIO_ENABLED
#include "bluetooth_bt_api.h"
#include "app_bt_func.h"
#include "app_utils.h"
#include "audio_dump.h"
#include "audioflinger.h"
#include "cqueue.h"
#include "hal_dma.h"
#include "hal_aud.h"
#include "hal_trace.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_interface.h"
#include "app_audio.h"

/// TODO:include path
#include "rwble_config.h"

#include "gaf_stream_dbg.h"
#include "gaf_media_pid.h"
#include "gaf_media_sync.h"
#include "gaf_bis_media_stream.h"
#include "gaf_codec_lc3.h"

#include "app_overlay.h"

#include "ble_audio_earphone_info.h"

#include "app_bt_sync.h"

#include "bes_aob_api.h"

#ifdef DSP_HIFI4
#include "mcu_dsp_m55_app.h"
#endif
#ifdef GAF_CODEC_CROSS_CORE
#include "mcu_dsp_m55_app.h"
#include "app_dsp_m55.h"
#include "gaf_codec_cc_common.h"
#include "gaf_codec_cc_bth.h"
#endif

/************************private macro defination***************************/
//#define BAP_DUMP_AUDIO_DATA
//#define BAP_CALCULATE_CODEC_MIPS

#define BT_AUDIO_CACHE_2_UNCACHE(addr) \
    ((unsigned char *)((unsigned int)addr & ~(0x04000000)))

#define LC3_BPS                             AUD_BITS_16
#define LC3_FRAME_MS                        10

//audio
#define LC3_AUDIO_CHANNEL_NUM               AUD_CHANNEL_NUM_1

//local source audio play
#define BIS_AUDIO_CHANNEL_NUM               AUD_CHANNEL_NUM_2
#define BIS_SAMPLE_RATE                     AUD_SAMPRATE_48000

#define TIME_CALCULATE_REVERSAL_THRESHOLD   0x7F000000
#define BIS_AUDIO_DEF_PRES_DLAY_US          40000   //Presentation Delay
#define BIS_AUDIO_PLAY_INTERVAL_US          10000   //af DMA irq interval
#define LC3_ALGORITH_CODEC_DELAY_US         2500    //lc3 encode decode delay
#define BIS_DIFF_ANCHOR_US                  4000    //current time diff to bis send anchor


/************************private type defination****************************/

/************************extern function declearation***********************/

/**********************private function declearation************************/

/************************private variable defination************************/
osMutexDef(gaf_bis_src_decoded_buffer_mutex);
osMutexDef(gaf_bis_src_encoded_buffer_mutex);
osMutexDef(gaf_decoder_buffer_mutex);
osMutexDef(gaf_decoder_buffer_mutex_1);
#ifdef GAF_CODEC_CROSS_CORE
osMutexDef(gaf_m55_encoder_buffer_mutex);
#endif
extern struct hci_le_create_big_cmp_evt big_created_info;
static gaf_bis_src_param_t bis_src_parameter = {0};
uint32_t expected_play_time = 0;

GAF_AUDIO_STREAM_ENV_T* pLocalBisStreamEnvPtr = NULL;
GAF_AUDIO_STREAM_ENV_T gaf_bis_src_audio_stream_env;
GAF_AUDIO_STREAM_ENV_T gaf_bis_audio_stream_env;

const static GAF_MEDIA_STREAM_TYPE_OPERATION_RULE_T gaf_bis_sink_stream_types_op_rule =
{
    GAF_AUDIO_STREAM_TYPE_PLAYBACK,
    GAF_AUDIO_TRIGGER_BY_PLAYBACK_STREAM,
    1,
    0,
};

const static GAF_MEDIA_STREAM_TYPE_OPERATION_RULE_T gaf_bis_src_stream_types_op_rule =
{
    GAF_AUDIO_STREAM_TYPE_PLAYBACK | GAF_AUDIO_STREAM_TYPE_CAPTURE,
    GAF_AUDIO_TRIGGER_BY_CAPTURE_STREAM,
    1,
    1,
};

#ifdef GAF_CODEC_CROSS_CORE

/**
 ****************************************************************************************
 * @brief When bth bis send the deinit signal to m55 before bth deinit m55 core
 *
 * @param[in] NONE                 NONE
 * @param[in] NONE                 NONE
 *
 * @param[out] NONE                NONE
 ****************************************************************************************
 */
static void gaf_bis_src_audio_send_deinit_signal_to_m55(void)
{
    GAF_AUDIO_M55_DEINIT_T p_deinit_req;
    p_deinit_req.con_lid         = gaf_m55_deinit_status.con_lid;
    p_deinit_req.context_type    = gaf_m55_deinit_status.context_type;
    p_deinit_req.is_bis          = gaf_m55_deinit_status.is_bis;
    p_deinit_req.is_bis_src      = gaf_m55_deinit_status.is_bis_src;
    p_deinit_req.capture_deinit  = gaf_m55_deinit_status.capture_deinit;
    p_deinit_req.playback_deinit = gaf_m55_deinit_status.playback_deinit;
    p_deinit_req.is_mobile_role  = gaf_m55_deinit_status.is_mobile_role;

    if (true == gaf_m55_deinit_status.playback_deinit){
        app_dsp_m55_bridge_send_cmd(
            CROSS_CORE_TASK_CMD_GAF_DECODE_DEINIT_WAITING_RSP,
            (uint8_t*)&p_deinit_req,
            sizeof(GAF_AUDIO_M55_DEINIT_T));
    }

    if (true == gaf_m55_deinit_status.capture_deinit){
        app_dsp_m55_bridge_send_cmd(
            CROSS_CORE_TASK_CMD_GAF_ENCODE_DEINIT_WAITING_RSP, \
            (uint8_t*)&p_deinit_req, \
            sizeof(GAF_AUDIO_M55_DEINIT_T));
    }

    return ;
}
#endif

static GAF_MEDIA_DWELLING_INFO_T gaf_bis_src_media_dwelling_info;
/****************************function defination****************************/
static uint8_t *gaf_bis_src_media_data_get_packets(uint32_t len)
{
    gaf_stream_buff_list_t *list = &gaf_bis_src_audio_stream_env.stream_context.playback_buff_list[0].buff_list;
    uint8_t *pcm_buff = NULL;
    uint8_t *read_pcm_buff = NULL;

    while (gaf_list_length(list))
    {
        list->node = gaf_list_begin(list);
        pcm_buff = (uint8_t *)gaf_list_node(list);
        break;
    }

    read_pcm_buff = (uint8_t *)gaf_stream_heap_malloc(len);
    if (pcm_buff)
    {
        memcpy(read_pcm_buff, pcm_buff, len);
        gaf_list_remove(list, pcm_buff);
    }

    return read_pcm_buff;
}

POSSIBLY_UNUSED static int gaf_bis_src_store_pcm_buffer(uint8_t *buffer, uint32_t len)
{
    gaf_stream_buff_list_t *list = &gaf_bis_src_audio_stream_env.stream_context.playback_buff_list[0].buff_list;
    int nRet = 0;

    LOG_D("%s, data_len %d list len %d ", __func__, len, gaf_list_length(list));
    if (gaf_list_length(list) < 5)
    {
        uint8_t *pcm_buffer = (uint8_t *)gaf_stream_heap_malloc(len);

        if (len)
        {
            memcpy(pcm_buffer, buffer, len);
        }
        gaf_list_append(list, pcm_buffer);
    }
    else
    {
        LOG_I("%s list full current list_len:%d data_len:%d", __func__, gaf_list_length(list), len);
        nRet = -1;
    }

    return nRet;
}

static void gaf_bis_src_audio_media_buf_init(void* _pStreamEnv)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    uint8_t* heapBufStartAddr = NULL;

#ifndef AOB_CODEC_CP
    lc3_alloc_data_free();
#endif
    app_audio_mempool_init_with_specific_size(app_audio_mempool_size());

    uint32_t audioCacheHeapSize = pStreamEnv->stream_info.playbackInfo.maxCachedEncodedAudioPacketCount*
        pStreamEnv->stream_info.playbackInfo.maxEncodedAudioPacketSize;
    audioCacheHeapSize += 2 * (pStreamEnv->stream_info.captureInfo.maxCachedEncodedAudioPacketCount*
        pStreamEnv->stream_info.captureInfo.maxEncodedAudioPacketSize);
    app_audio_mempool_get_buff(&heapBufStartAddr, audioCacheHeapSize);
    gaf_stream_heap_init(heapBufStartAddr, audioCacheHeapSize);

    app_audio_mempool_get_buff(&(pStreamEnv->stream_info.playbackInfo.dmaBufPtr),
        pStreamEnv->stream_info.playbackInfo.dmaChunkSize*2);

    gaf_list_new(&pStreamEnv->stream_context.playback_buff_list[0].buff_list,
                    (osMutex(gaf_bis_src_decoded_buffer_mutex)),
                    gaf_stream_heap_free,
                    gaf_stream_heap_cmalloc,
                    gaf_stream_heap_free);

    pStreamEnv->func_list->decoder_func_list->decoder_init_buf_func(pStreamEnv,
        gaf_bis_src_stream_types_op_rule.playback_ase_count);

    app_audio_mempool_get_buff(&(pStreamEnv->stream_info.captureInfo.dmaBufPtr),
        pStreamEnv->stream_info.captureInfo.dmaChunkSize*2);

#ifdef GAF_CODEC_CROSS_CORE
    gaf_list_new(&pStreamEnv->stream_context.m55_capture_buff_list.buff_list,
                    (osMutex(gaf_m55_encoder_buffer_mutex)),
                    gaf_m55_stream_encoder_data_free,
                    gaf_stream_heap_cmalloc,
                    gaf_m55_stream_encoder_heap_free);
#else
    gaf_list_new(&pStreamEnv->stream_context.capture_buff_list,
                    (osMutex(gaf_bis_src_encoded_buffer_mutex)),
                    gaf_stream_data_free,
                    gaf_stream_heap_cmalloc,
                    gaf_stream_heap_free);
#endif

    pStreamEnv->func_list->encoder_func_list->encoder_init_buf_func(pStreamEnv);

    gaf_bis_src_media_dwelling_info.playback_ase_id[0] = GAF_INVALID_ASE_INDEX;
    gaf_bis_src_media_dwelling_info.capture_ase_id[0] = GAF_INVALID_ASE_INDEX;
    gaf_bis_src_media_dwelling_info.startedStreamTypes = 0;
}

static int gaf_bis_src_audio_media_stream_start_handler(void* _pStreamEnv)
{
    uint32_t trigger_bt_time = 0,current_bt_time = 0,bis_anchor_time = 0,diff_time = 0;
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
#ifdef GAF_ENCODER_CROSS_CORE_USE_M55
    pLocalCaptureBisStreamEnvPtr = pStreamEnv;
#endif
    if (GAF_CAPTURE_STREAM_IDLE == pStreamEnv->stream_context.capture_stream_state)
    {
        app_sysfreq_req(APP_SYSFREQ_USER_AOB, APP_SYSFREQ_208M);
        af_set_priority(AF_USER_AI, osPriorityHigh);

        struct AF_STREAM_CONFIG_T stream_cfg;

        // capture stream
        memset((void *)&stream_cfg, 0, sizeof(struct AF_STREAM_CONFIG_T));
        stream_cfg.bits         = (enum AUD_BITS_T)(pStreamEnv->stream_info.captureInfo.bits_depth);
        stream_cfg.channel_num  = (enum AUD_CHANNEL_NUM_T)(pStreamEnv->stream_info.captureInfo.num_channels);
        if (stream_cfg.channel_num == AUD_CHANNEL_NUM_2)
        {
            stream_cfg.channel_map  = (enum AUD_CHANNEL_MAP_T)(AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1);
        }
        else if (stream_cfg.channel_num != AUD_CHANNEL_NUM_1)
        {
            LOG_E("Invalid bis src channel num = %d", stream_cfg.channel_num);
        }
        stream_cfg.io_path      = AUD_INPUT_PATH_MAINMIC;
        stream_cfg.device       = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.sample_rate  = (enum AUD_SAMPRATE_T)pStreamEnv->stream_info.captureInfo.sample_rate;

        // TODO: get vol from VCC via ase_lid
        stream_cfg.vol          = TGT_ADC_VOL_LEVEL_7;

        stream_cfg.data_size    = (uint32_t)(2 * pStreamEnv->stream_info.captureInfo.dmaChunkSize);

        pStreamEnv->func_list->stream_func_list.init_stream_buf_func(pStreamEnv);

        pStreamEnv->func_list->encoder_func_list->encoder_init_func(pStreamEnv);

        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(pStreamEnv->stream_info.captureInfo.dmaBufPtr);

        stream_cfg.handler = pStreamEnv->func_list->stream_func_list.capture_dma_irq_handler_func;

        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);

        // playback stream
        memset((void *)&stream_cfg, 0, sizeof(struct AF_STREAM_CONFIG_T));
        stream_cfg.bits         = (enum AUD_BITS_T)(pStreamEnv->stream_info.playbackInfo.bits_depth);
        stream_cfg.channel_num  = (enum AUD_CHANNEL_NUM_T)(pStreamEnv->stream_info.playbackInfo.num_channels);

        stream_cfg.io_path      = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.device       = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.sample_rate  = (enum AUD_SAMPRATE_T)pStreamEnv->stream_info.playbackInfo.sample_rate;

        // TODO: get vol from VCC via ase_lid
        stream_cfg.vol          = TGT_VOLUME_LEVEL_7;

        stream_cfg.data_size    = (uint32_t)(2 * pStreamEnv->stream_info.playbackInfo.dmaChunkSize);

        pStreamEnv->func_list->decoder_func_list->decoder_init_func(pStreamEnv,
            gaf_bis_src_stream_types_op_rule.playback_ase_count);

        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(pStreamEnv->stream_info.playbackInfo.dmaBufPtr);

        stream_cfg.handler = pStreamEnv->func_list->stream_func_list.playback_dma_irq_handler_func;

        af_codec_tune(AUD_STREAM_PLAYBACK, 0);
        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);

        // trigger set-up
        pStreamEnv->stream_context.playbackTriggerChannel = app_bt_sync_get_available_trigger_channel(APP_BT_SYNC_OP_MAX, APP_BT_SYNC_POLICY_DEFAULT);

        gaf_media_prepare_playback_trigger(pStreamEnv->stream_context.playbackTriggerChannel);

        af_codec_sync_device_config(AUD_STREAM_USE_INT_CODEC, AUD_STREAM_PLAYBACK, AF_CODEC_SYNC_TYPE_BT, false);
        af_codec_sync_device_config(AUD_STREAM_USE_INT_CODEC, AUD_STREAM_PLAYBACK, AF_CODEC_SYNC_TYPE_BT, true);
        af_codec_set_device_bt_sync_source(AUD_STREAM_USE_INT_CODEC, AUD_STREAM_PLAYBACK, pStreamEnv->stream_context.playbackTriggerChannel);

        gaf_media_pid_init(&(pStreamEnv->stream_context.playback_pid_env));

        gaf_stream_common_update_playback_stream_state(pStreamEnv, GAF_PLAYBACK_STREAM_INITIALIZED);

        pStreamEnv->stream_context.captureTriggerChannel = app_bt_sync_get_available_trigger_channel(APP_BT_SYNC_OP_MAX, APP_BT_SYNC_POLICY_DEFAULT);

        gaf_media_prepare_capture_trigger(pStreamEnv->stream_context.captureTriggerChannel);
        gaf_media_pid_init(&(pStreamEnv->stream_context.capture_pid_env));
        gaf_media_pid_update_threshold(&(pStreamEnv->stream_context.capture_pid_env),
            pStreamEnv->stream_info.captureInfo.dmaChunkIntervalUs/2);

        gaf_stream_common_update_capture_stream_state(pStreamEnv, GAF_CAPTURE_STREAM_INITIALIZED);

        if (GAF_AUDIO_TRIGGER_BY_CAPTURE_STREAM ==
            gaf_bis_src_stream_types_op_rule.trigger_stream_type)
        {
            current_bt_time = gaf_media_sync_get_curr_time();
#ifndef WIFI_DONGLE
            bis_anchor_time = btdrv_reg_op_big_anchor_timestamp(bis_src_parameter.conhdl&0xFF);
#endif
            LOG_I("current_bt_time %d bis_anchor_time %d big_sync_delay %d",current_bt_time, bis_anchor_time,bis_src_parameter.big_sync_delay);

            if(current_bt_time >= bis_anchor_time)
            {
                diff_time = bis_anchor_time + pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs - current_bt_time;
                if(diff_time >= BIS_DIFF_ANCHOR_US){
                    if(bis_anchor_time + bis_src_parameter.big_sync_delay > current_bt_time){
                        current_bt_time = bis_anchor_time + bis_src_parameter.big_sync_delay;
                    }
                }
                else{
                    current_bt_time = bis_anchor_time + pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs;
                    osDelay(diff_time/1000);
                }
            }
            else
            {
                diff_time = bis_anchor_time - current_bt_time;
                if(diff_time >= BIS_DIFF_ANCHOR_US){
                    if(bis_anchor_time + bis_src_parameter.big_sync_delay - pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs > current_bt_time){
                        current_bt_time = bis_anchor_time + bis_src_parameter.big_sync_delay - pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs;
                    }
                }
                else{
                    current_bt_time = bis_anchor_time;
                    osDelay(diff_time/1000);
                }
            }

            trigger_bt_time = current_bt_time + bis_src_parameter.big_trans_latency -
                pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs + LC3_ALGORITH_CODEC_DELAY_US;
            expected_play_time = trigger_bt_time + pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs;
            gaf_stream_common_set_capture_trigger_time(pStreamEnv, trigger_bt_time);
        }

        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);

        return 0;
    }

    return -1;
}

static uint32_t gaf_bis_src_playback_dma_irq_handler(uint8_t *buf, uint32_t len)
{
    uint32_t af_dma_time = 0;
    uint8_t adma_ch = HAL_DMA_CHAN_NONE;
    uint32_t dma_base;
    int32_t diff_bt_time = 0;
    //float ratio = 0;
    uint32_t btclk; //hal slot -- 312.5us
    uint16_t btcnt; //hal  microsecond -- 0.5 us
    uint64_t revlersal_expected_time = 1,revlersal_current_time = 1;
    POSSIBLY_UNUSED uint8_t* read_buf = NULL;
    memset(buf, 0, len);

    //GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_bis_src_audio_stream_env[GAF_AUDIO_STREAM_CONTEXT_TYPE_MEDIA];

    adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    if (adma_ch != HAL_DMA_CHAN_NONE)
    {
        bt_drv_reg_op_dma_tc_clkcnt_get_by_ch(&btclk, &btcnt, adma_ch&0xFF, dma_base);
        af_dma_time = bt_syn_ble_bt_time_to_bts(btclk, btcnt);
    }

    diff_bt_time = af_dma_time - expected_play_time;

    if(abs(diff_bt_time) > TIME_CALCULATE_REVERSAL_THRESHOLD)
    {
        if(af_dma_time > expected_play_time)
        {
            revlersal_expected_time <<= 32;
            revlersal_expected_time += expected_play_time;
            revlersal_current_time = af_dma_time;
            diff_bt_time = revlersal_current_time - revlersal_expected_time;
        }
        else
        {
            revlersal_current_time <<= 32;
            revlersal_expected_time = expected_play_time;
            revlersal_current_time += af_dma_time;
            diff_bt_time = revlersal_current_time - revlersal_expected_time;
        }
        LOG_I("WARNIN:time has revleral expected_play_time %d af_dma_time %d diff_bt_time %d ",expected_play_time, af_dma_time,diff_bt_time);
    }

    //ratio = gaf_media_pid_adjust(&(pStreamEnv->stream_context.playback_pid_env), diff_bt_time);

    //af_codec_tune(AUD_STREAM_PLAYBACK, ratio);
    expected_play_time += BIS_AUDIO_PLAY_INTERVAL_US;

    read_buf = gaf_bis_src_media_data_get_packets(len);
    memcpy(buf, read_buf, len);
    gaf_stream_heap_free(read_buf);
    LOG_I("%s length %d", __func__, len);

    return 0;
}

static int gaf_bis_src_audio_media_stream_stop_handler(void* _pStreamEnv)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    if (GAF_CAPTURE_STREAM_IDLE != pStreamEnv->stream_context.capture_stream_state)
    {
        uint8_t POSSIBLY_UNUSED adma_ch = HAL_DMA_CHAN_NONE;
        uint32_t dma_base;
        // source
        gaf_stream_common_update_capture_stream_state(pStreamEnv, GAF_CAPTURE_STREAM_IDLE);
        af_stream_dma_tc_irq_disable(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        if(adma_ch != HAL_DMA_CHAN_NONE)
        {
            bt_drv_reg_op_disable_dma_tc(adma_ch&0xFF, dma_base);
        }

        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);

        // sink
        gaf_stream_common_update_playback_stream_state(pStreamEnv, GAF_PLAYBACK_STREAM_IDLE);
        af_stream_dma_tc_irq_disable(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        if(adma_ch != HAL_DMA_CHAN_NONE)
        {
            bt_drv_reg_op_disable_dma_tc(adma_ch&0xFF, dma_base);
        }

        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_codec_tune(AUD_STREAM_PLAYBACK, 0);

        pStreamEnv->func_list->decoder_func_list->decoder_deinit_func();

        gaf_stream_common_clr_trigger(pStreamEnv->stream_context.playbackTriggerChannel);
        gaf_stream_common_clr_trigger(pStreamEnv->stream_context.captureTriggerChannel);

        app_sysfreq_req(APP_SYSFREQ_USER_AOB, APP_SYSFREQ_32K);
        af_set_priority(AF_USER_AI, osPriorityAboveNormal);

        pStreamEnv->func_list->encoder_func_list->encoder_deinit_func(pStreamEnv);
        pStreamEnv->func_list->stream_func_list.deinit_stream_buf_func(pStreamEnv);

        return 0;
    }

    return -1;
}

static void gaf_bis_src_capture_dma_irq_handler_send(void* pStreamEnv_,void *payload,
    uint32_t payload_size, uint32_t ref_time)
{
#ifdef AOB_MOBILE_ENABLED
    uint8_t *output_buf[2];
    output_buf[0] = (uint8_t*)payload;
    output_buf[1] = NULL;
#ifndef WIFI_DONGLE
    bes_ble_bis_src_send_iso_data_to_all_channel(output_buf, payload_size, ref_time);
#endif
#endif
}

static uint32_t gaf_bis_src_capture_dma_irq_handler(uint8_t* ptrBuf, uint32_t length)
{
    uint32_t dmaIrqHappeningTimeUs = 0;
    uint32_t btclk; //hal slot -- 312.5us
    uint16_t btcnt; //hal  microsecond -- 0.5 us
    uint8_t adma_ch = HAL_DMA_CHAN_NONE;
    uint32_t dma_base;

    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_bis_src_audio_stream_env;

    if ((!pStreamEnv) ||
        (GAF_CAPTURE_STREAM_START_TRIGGERING > pStreamEnv->stream_context.capture_stream_state)) {
        memset(ptrBuf, 0x00, length);
        return length;
    } else if (GAF_CAPTURE_STREAM_START_TRIGGERING == pStreamEnv->stream_context.capture_stream_state) {
        gaf_stream_common_update_capture_stream_state(pStreamEnv, GAF_CAPTURE_STREAM_STREAMING_TRIGGERED);
        gaf_stream_common_clr_trigger(pStreamEnv->stream_context.captureTriggerChannel);
    }

    adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    if (adma_ch != HAL_DMA_CHAN_NONE)
    {
        bt_drv_reg_op_dma_tc_clkcnt_get_by_ch(&btclk, &btcnt, adma_ch&0xFF, dma_base);
        dmaIrqHappeningTimeUs = bt_syn_ble_bt_time_to_bts(btclk, btcnt);
    }

    // it's possible the multiple DMA irq triggered message accumulates,
    // so the acuiqred dmaIrqHappeningTimeUs is the last dma irq, which has
    // been handled. For this case, ignore this dma irq message handling
    if ((GAF_CAPTURE_STREAM_STREAMING_TRIGGERED ==
        pStreamEnv->stream_context.capture_stream_state) &&
        (dmaIrqHappeningTimeUs ==
        pStreamEnv->stream_context.lastCaptureDmaIrqTimeUs))
    {
        LOG_W("accumulated irq %u messages happen!", dmaIrqHappeningTimeUs);
        return length;
    }

    //gaf_bis_src_store_pcm_buffer(ptrBuf, length);
    gaf_stream_common_capture_timestamp_checker(pStreamEnv, dmaIrqHappeningTimeUs);

    dmaIrqHappeningTimeUs += (uint32_t)pStreamEnv->stream_info.captureInfo.dmaChunkIntervalUs;
    LOG_I("length %d encoded_len %d filled timestamp %d", length,
        pStreamEnv->stream_info.captureInfo.encoded_frame_size,
        dmaIrqHappeningTimeUs);

#ifdef GAF_ENCODER_CROSS_CORE_USE_M55
    do{
peek_again:
        if (gaf_m55_deinit_status.capture_deinit == true)
        {
            break;
        }
        bool is_accessed = false;
        is_accessed = gaf_stream_common_store_received_pcm_packet((void *)pStreamEnv, dmaIrqHappeningTimeUs, ptrBuf, length);

        if (pStreamEnv->stream_context.isUpStreamingStarted)
        {
            uint8_t capture_list_length = 0;
            capture_list_length = gaf_list_length(&pStreamEnv->stream_context.m55_capture_buff_list.buff_list);
            if ((false == is_accessed) && (capture_list_length > GAF_ENCODER_PCM_DATA_BUFF_LIST_MAX_LENGTH)) {
                goto peek_again;
            }

            // malloc output_buf to cached encoded data
            uint8_t *output_buf = NULL;
            uint32_t lc3_encoded_frame_len = (uint32_t)(pStreamEnv->stream_info.captureInfo.encoded_frame_size);
            output_buf = (uint8_t *)gaf_stream_heap_cmalloc(lc3_encoded_frame_len);

            // bth fetch encoded data
            gaf_encoder_core_fetch_encoded_data(pStreamEnv, output_buf, lc3_encoded_frame_len, dmaIrqHappeningTimeUs);

            // bth send out encoded data
            gaf_bis_src_capture_dma_irq_handler_send(pStreamEnv, output_buf, lc3_encoded_frame_len, dmaIrqHappeningTimeUs);

            gaf_stream_heap_free(output_buf);
        }
    } while(0);
#else

    pStreamEnv->func_list->encoder_func_list->encoder_encode_frame_func(pStreamEnv, dmaIrqHappeningTimeUs,
        length, ptrBuf, &pStreamEnv->stream_context.codec_alg_context[0],
        &gaf_bis_src_capture_dma_irq_handler_send);
#endif
    return 0;
}

static void gaf_bis_src_audio_media_buf_deinit(void* _pStreamEnv)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    pStreamEnv->stream_info.captureInfo.dmaBufPtr = NULL;
#ifdef GAF_CODEC_CROSS_CORE
    gaf_list_free(&pStreamEnv->stream_context.m55_capture_buff_list.buff_list);
#else
    gaf_list_free(&pStreamEnv->stream_context.capture_buff_list);
#endif

    pStreamEnv->stream_info.playbackInfo.dmaBufPtr = NULL;
    gaf_list_free(&pStreamEnv->stream_context.playback_buff_list[0].buff_list);

    pStreamEnv->func_list->decoder_func_list->decoder_deinit_buf_func(pStreamEnv,
        gaf_bis_src_stream_types_op_rule.playback_ase_count);
    pStreamEnv->func_list->encoder_func_list->encoder_deinit_buf_func(pStreamEnv);
}

static GAF_AUDIO_FUNC_LIST_T gaf_bis_src_audio_media_stream_func_list =
{
    {
        .start_stream_func = gaf_bis_src_audio_media_stream_start_handler,
        .init_stream_buf_func = gaf_bis_src_audio_media_buf_init,
        .stop_stream_func = gaf_bis_src_audio_media_stream_stop_handler,
        .playback_dma_irq_handler_func = gaf_bis_src_playback_dma_irq_handler,
        .capture_dma_irq_handler_func = gaf_bis_src_capture_dma_irq_handler,
        .deinit_stream_buf_func = gaf_bis_src_audio_media_buf_deinit,
    },
};

static GAF_AUDIO_STREAM_ENV_T *gaf_bis_src_audio_stream_update_stream_env_from_grp_info(uint8_t grp_lid)
{
#ifdef AOB_MOBILE_ENABLED
    uint16_t bis_hdl = bes_ble_bis_src_get_bis_hdl_by_big_idx(grp_lid);

    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_bis_src_audio_stream_env;
    GAF_AUDIO_STREAM_COMMON_INFO_T* pCommonInfo;
    pCommonInfo = &(pStreamEnv->stream_info.captureInfo);

    pCommonInfo->bits_depth = LC3_BPS;

    const AOB_BAP_CFG_T* p_lc3_cfg = bes_ble_bis_src_get_codec_cfg_by_big_idx(grp_lid);
    pCommonInfo->aseChInfo[0].allocation_bf = p_lc3_cfg->param.location_bf;
    pCommonInfo->aseChInfo[0].iso_channel_hdl = BLE_BISHDL_TO_ACTID(bis_hdl);
    pCommonInfo->aseChInfo[0].cis_hdl = bis_hdl;

    pCommonInfo->num_channels = bes_ble_audio_get_location_fs_l_r_cnt(p_lc3_cfg->param.location_bf);

    const AOB_CODEC_ID_T *p_codec_id = bes_ble_bis_src_get_codec_id_by_big_idx(grp_lid, 0);

    switch (p_codec_id->codec_id[0])
    {
        case AOB_CODEC_TYPE_LC3:
        {
            pCommonInfo->frame_ms =
                gaf_stream_common_frame_duration_parse(p_lc3_cfg->param.frame_dur);
            pCommonInfo->sample_rate =
                gaf_stream_common_sample_freq_parse(p_lc3_cfg->param.sampling_freq);
            pCommonInfo->encoded_frame_size = p_lc3_cfg->param.frame_octet;
            pCommonInfo->maxCachedEncodedAudioPacketCount = GAF_AUDIO_MEDIA_DATA_PACKET_NUM_LIMITER;
            pCommonInfo->maxEncodedAudioPacketSize = gaf_audio_lc3_encoder_get_max_frame_size();
            pCommonInfo->dmaChunkIntervalUs = (uint32_t)(pCommonInfo->frame_ms*1000);
            pCommonInfo->dmaChunkSize =
                (uint32_t)((pCommonInfo->sample_rate *
                (pCommonInfo->bits_depth / 8) *
                (pCommonInfo->dmaChunkIntervalUs) *
                pCommonInfo->num_channels) / (1000 * 1000));
            memcpy(&(pStreamEnv->stream_info.playbackInfo), pCommonInfo, sizeof(GAF_AUDIO_STREAM_COMMON_INFO_T));
            gaf_audio_lc3_update_codec_func_list(pStreamEnv);
            break;
        }
        default:
            ASSERT(false, "unknown codec type!");
            return NULL;
    }

    LOG_I("frame len %d us, sample rate %d dma chunk time %d us dma chunk size %d",
            (uint32_t)(pCommonInfo->frame_ms*1000), pCommonInfo->sample_rate,
            pCommonInfo->dmaChunkIntervalUs, pCommonInfo->dmaChunkSize);
    LOG_I("num of channel = %d", pCommonInfo->num_channels);
    LOG_I("allocation: 0x%x", pCommonInfo->aseChInfo[0].allocation_bf);

    return pStreamEnv;
#else
    return NULL;
#endif /// AOB_MOBILE_ENABLED
}

static void _gaf_bis_src_audio_stream_start_handler(uint32_t grp_param)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = NULL;

    memcpy(&bis_src_parameter, (void *)grp_param, sizeof(gaf_bis_src_param_t));
    pStreamEnv= gaf_bis_src_audio_stream_update_stream_env_from_grp_info(bis_src_parameter.grp_lid);
    if (pStreamEnv)
    {
        pStreamEnv->func_list->stream_func_list.start_stream_func(pStreamEnv);
    }
}

void gaf_bis_src_audio_stream_start_handler(gaf_bis_src_param_t *src_para)
{
    LOG_I("%s", __func__);

    bt_adapter_write_sleep_enable(0);
    app_bt_start_custom_function_in_bt_thread((uint32_t)src_para,
                                              0,
                                              (uint32_t)_gaf_bis_src_audio_stream_start_handler);
}

static void _gaf_bis_src_audio_stream_stop_handler(uint8_t grp_lid)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = gaf_bis_src_audio_stream_update_stream_env_from_grp_info(grp_lid);

    if (pStreamEnv)
    {
        pStreamEnv->func_list->stream_func_list.stop_stream_func(pStreamEnv);
        bt_adapter_write_sleep_enable(1);
#ifdef GAF_CODEC_CROSS_CORE
        /// for bis src, the conlid and context_type is not needed,
        /// so it could be setted 0
        gaf_m55_deinit_status.con_lid         = 0;
        gaf_m55_deinit_status.context_type    = 0;
        gaf_m55_deinit_status.is_bis          = true;
        gaf_m55_deinit_status.is_bis_src      = true;
        gaf_m55_deinit_status.playback_deinit = true;
        gaf_m55_deinit_status.capture_deinit  = true;
        gaf_m55_deinit_status.is_mobile_role  = false;

        gaf_bis_src_audio_send_deinit_signal_to_m55();
#endif

    }
}

void gaf_bis_src_audio_stream_stop_handler(uint8_t grp_lid)
{
    LOG_I("%s", __func__);
    app_bt_start_custom_function_in_bt_thread((uint32_t)grp_lid,
                                              0,
                                              (uint32_t)_gaf_bis_src_audio_stream_stop_handler);
}

void gaf_bis_src_audio_stream_init(void)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_bis_src_audio_stream_env;
    memset((uint8_t *)pStreamEnv, 0, sizeof(GAF_AUDIO_STREAM_ENV_T));
    pStreamEnv->stream_context.playback_stream_state = GAF_PLAYBACK_STREAM_IDLE;
    pStreamEnv->stream_context.capture_stream_state = GAF_CAPTURE_STREAM_IDLE;
    pStreamEnv->stream_info.playbackInfo.aseChInfo[0].iso_channel_hdl = GAF_AUDIO_INVALID_ISO_CHANNEL;
    pStreamEnv->stream_info.captureInfo.aseChInfo[0].iso_channel_hdl = GAF_AUDIO_INVALID_ISO_CHANNEL;
    pStreamEnv->stream_info.playbackInfo.presDelayUs = GAF_AUDIO_CONTROLLER_2_HOST_LATENCY_US;
    pStreamEnv->stream_info.captureInfo.presDelayUs = GAF_AUDIO_CONTROLLER_2_HOST_LATENCY_US;

    GAF_AUDIO_STREAM_INFO_T* pStreamInfo;

    pStreamInfo = &(gaf_bis_src_audio_stream_env.stream_info);
    pStreamInfo->contextType = GAF_AUDIO_STREAM_CONTEXT_TYPE_MEDIA;
    gaf_stream_common_register_func_list(&gaf_bis_src_audio_stream_env,
        &gaf_bis_src_audio_media_stream_func_list);
}

#ifdef GAF_CODEC_CROSS_CORE
/**
 ****************************************************************************************
 * @brief When bth bis received the decdoer deinit signal from m55, bth will deinit m55 core
 *
 * @param[in] con_id                       connection index
 * @param[in] context_type                 ASE context type
 *
 * @param[out] NONE                      NONE
 ****************************************************************************************
 */
void _gaf_audio_bis_bth_decoder_received_deinit_signal_from_m55(uint8_t con_id, uint32_t context_type)
{
    LOG_D("gaf_audio_bth_bis_received_deinit_signal_from_m55");
    app_dsp_m55_deinit(APP_DSP_M55_USER_AUDIO_DECODER);
    return ;
}

/**
 ****************************************************************************************
 * @brief When bth bis received the encoder deinit signal from m55, bth will deinit m55 core
 *
 * @param[in] con_id                       connection index
 * @param[in] context_type                 ASE context type
 *
 * @param[out] NONE                      NONE
 ****************************************************************************************
 */
void _gaf_audio_bis_bth_encoder_received_deinit_signal_from_m55(uint8_t con_id, uint32_t context_type)
{
    LOG_D("gaf_audio_bth_bis_received_deinit_signal_from_m55");
    app_dsp_m55_deinit(APP_DSP_M55_USER_AUDIO_ENCODER);
    return ;
}
#endif

//bis sink audio steam
static gaf_media_data_t *gaf_bis_stream_select_data_store(GAF_AUDIO_STREAM_ENV_T* pStreamEnv, gaf_media_data_t *p_decoder_frame)
{
    uint8_t list_idx = 0xFF;
    uint8_t  select_ch_map = 0;
    uint16_t blocks_size = 0;
    uint16_t store_ch_num = 0;
    uint16_t frame_size = pStreamEnv->stream_info.playbackInfo.encoded_frame_size;
    GAF_AUDIO_STREAM_COMMON_INFO_T* pCommonInfo = &(pStreamEnv->stream_info.playbackInfo);
    gaf_media_data_t *frame_p = NULL;
    gaf_media_data_t* storedFramePointer = NULL;
    /// Which stream does the query packet belong to
    for (int i=0; i<2; i++)
    {
        if (pCommonInfo->bisChInfo[i].iso_channel_hdl == p_decoder_frame->conhdl)
        {
            list_idx = i;
            select_ch_map = pCommonInfo->bisChInfo[i].select_ch_map;
            blocks_size   = pCommonInfo->bisChInfo[i].blocks_size;
            break;
        }
    }

    if (list_idx == 0xFF)
    {
        return NULL;
    }

    // Filter and split audio data
    if (pStreamEnv->stream_info.playbackInfo.sdu_num > 1)
    {
        for(int i=0; i < pStreamEnv->stream_info.playbackInfo.sdu_num; i++)
        {
            /// If there are multiple SDU packages, the SDU package is split
            frame_p = (gaf_media_data_t *)gaf_stream_heap_malloc(sizeof(gaf_media_data_t));
            frame_p->time_stamp = (uint32_t)(pStreamEnv->stream_info.playbackInfo.frame_ms *1000) * i
                                  + p_decoder_frame->time_stamp;
            frame_p->pkt_seq_nb = p_decoder_frame->pkt_seq_nb * pStreamEnv->stream_info.playbackInfo.sdu_num +i;
            frame_p->data_len   = (p_decoder_frame->data_len)?blocks_size:p_decoder_frame->data_len;
            frame_p->origin_buffer = NULL;
            frame_p->sdu_data      = NULL;
            frame_p->pkt_status      = p_decoder_frame->pkt_status;
            frame_p->cisChannel      = p_decoder_frame->cisChannel;
            frame_p->conhdl          = p_decoder_frame->conhdl;

            if (frame_p->data_len)
            {
                frame_p->origin_buffer = (uint8_t *)gaf_stream_heap_malloc(blocks_size);
                frame_p->sdu_data      = frame_p->origin_buffer;
                memcpy(frame_p->sdu_data, p_decoder_frame->sdu_data + (i * blocks_size), blocks_size);
                /// Select one channel data when multiple channels are available
                for (int i=0; ((select_ch_map!=0) && ((i*frame_size) < frame_p->data_len)); ++i)
                {
                    if (select_ch_map & (1UL<<i))
                    {
                        if (store_ch_num != i)
                        {
                            memcpy(frame_p->sdu_data+(frame_size * store_ch_num),
                                   frame_p->sdu_data+(frame_size * i), frame_size);
                        }
                        store_ch_num++;
                    }
                    select_ch_map &= ~(1UL<<i);
                }
                frame_p->data_len = frame_size*store_ch_num;
            }

            /// store data packet
            storedFramePointer = gaf_stream_common_store_received_packet(pStreamEnv,
                                                                         list_idx,
                                                                         frame_p);
            if (!storedFramePointer)
            {
                break;
            }

            if (i == 0)
            {
                storedFramePointer = frame_p;
            }
        }
        gaf_stream_data_free(p_decoder_frame);
    }
    else
    {
        /// Select one channel data when multiple channels are available
        if (p_decoder_frame->data_len)
        {
            for (int i=0; ((select_ch_map!=0) && ((i*frame_size) < p_decoder_frame->data_len)); ++i)
            {
                if (select_ch_map & (1UL<<i))
                {
                    if (store_ch_num != i)
                    {
                        memcpy(p_decoder_frame->sdu_data+(frame_size * store_ch_num),
                               p_decoder_frame->sdu_data+(frame_size * i), frame_size);
                    }
                    store_ch_num++;
                }
                select_ch_map &= ~(1UL<<i);
            }
            p_decoder_frame->data_len = frame_size*store_ch_num;
        }

        storedFramePointer = gaf_stream_common_store_received_packet(pStreamEnv,
                                                                     list_idx,
                                                                     p_decoder_frame);
    }

    return storedFramePointer;
}

static void gaf_bis_stream_receive_data(uint16_t conhdl, GAF_ISO_PKT_STATUS_E pkt_status)
{
    // map to gaf stream context
    // TODO: get the correct stream context based on active stream type.
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_bis_audio_stream_env;

    uint32_t current_bt_time = 0;
    uint32_t trigger_bt_time = 0;
    gaf_media_data_t decoder_frame_info;
    gaf_media_data_t *p_decoder_frame = NULL;
    gaf_media_data_t* storedFramePointer = NULL;

    while ((p_decoder_frame = (gaf_media_data_t *)bes_ble_bap_dp_itf_get_rx_data(conhdl, NULL)))
    {
        ASSERT(p_decoder_frame->data_len <= pStreamEnv->stream_info.playbackInfo.maxEncodedAudioPacketSize,
            "%s len %d %d, channel:%d, playbackInfo:%p", __func__, p_decoder_frame->data_len,
            pStreamEnv->stream_info.playbackInfo.maxEncodedAudioPacketSize, conhdl, &(pStreamEnv->stream_info.playbackInfo));

        if ((pStreamEnv->stream_context.playback_stream_state >=
            GAF_PLAYBACK_STREAM_START_TRIGGERING) ||
            ((GAF_ISO_PKT_STATUS_VALID == p_decoder_frame->pkt_status) &&
            (p_decoder_frame->data_len > 0)))
        {
        #ifdef GAF_CODEC_CROSS_CORE
            if (gaf_m55_deinit_status.playback_deinit == true)
            {
                gaf_stream_data_free(p_decoder_frame);
                break;
            }
        #endif
            storedFramePointer =gaf_bis_stream_select_data_store(pStreamEnv, p_decoder_frame);
        }

        if (storedFramePointer == NULL)
        {
            decoder_frame_info = *p_decoder_frame;
            gaf_stream_data_free(p_decoder_frame);
        }
        else
        {
            decoder_frame_info = *storedFramePointer;
        }
        if (pStreamEnv->stream_context.playback_stream_state < GAF_PLAYBACK_STREAM_START_TRIGGERING)
        {
            if (GAF_AUDIO_TRIGGER_BY_PLAYBACK_STREAM ==
                gaf_bis_sink_stream_types_op_rule.trigger_stream_type)
            {
                if ((GAF_PLAYBACK_STREAM_INITIALIZED == pStreamEnv->stream_context.playback_stream_state)
                    && (GAF_ISO_PKT_STATUS_VALID == decoder_frame_info.pkt_status)
                    && (decoder_frame_info.data_len > 0))
                {
                    for (int i=0; i <2; ++i)
                    {
                        if(conhdl == pStreamEnv->stream_info.playbackInfo.bisChInfo[i].iso_channel_hdl)
                        {
                            pStreamEnv->stream_info.playbackInfo.trigger_stream_lid = i;
                        }
                    }
                    current_bt_time = gaf_media_sync_get_curr_time();

                    LOG_I("%s expected play us %u current us %u seq 0x%x", __func__,
                        decoder_frame_info.time_stamp, current_bt_time, decoder_frame_info.pkt_seq_nb);

                    trigger_bt_time = decoder_frame_info.time_stamp + pStreamEnv->stream_info.playbackInfo.presDelayUs -
                        (uint32_t)(pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs);
                    LOG_I("calculated trigger ticks %u, %u ,%u, %u",current_bt_time, decoder_frame_info.time_stamp,
                        trigger_bt_time,pStreamEnv->stream_info.playbackInfo.presDelayUs);
                    if (current_bt_time < trigger_bt_time)
                    {
                        LOG_I("Starting playback seq num 0x%x", decoder_frame_info.pkt_seq_nb);
                        pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX] = decoder_frame_info.pkt_seq_nb;
                        gaf_stream_common_set_playback_trigger_time(pStreamEnv, trigger_bt_time);
                    }
                    else
                    {
                        LOG_I("time_stamp error");
                        if (storedFramePointer)
                        {
                            gaf_list_remove(&pStreamEnv->stream_context.playback_buff_list[0].buff_list,
                                storedFramePointer);
                        }
                    }
                }
            }
        }
    }
}

static int gaf_bis_audio_media_stream_start_handler(void* _pStreamEnv)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    if (GAF_PLAYBACK_STREAM_IDLE == pStreamEnv->stream_context.playback_stream_state)
    {
        pLocalBisStreamEnvPtr = pStreamEnv;
        // TODO: shall use reasonable cpu frequency
        app_sysfreq_req(APP_SYSFREQ_USER_AOB, APP_SYSFREQ_104M);
        af_set_priority(AF_USER_AI, osPriorityHigh);

        struct AF_STREAM_CONFIG_T stream_cfg;

        memset((void *)&stream_cfg, 0, sizeof(struct AF_STREAM_CONFIG_T));
        stream_cfg.bits         = (enum AUD_BITS_T)(pStreamEnv->stream_info.playbackInfo.bits_depth);
        stream_cfg.channel_num  = (enum AUD_CHANNEL_NUM_T)(pStreamEnv->stream_info.playbackInfo.num_channels);
        if (stream_cfg.channel_num == AUD_CHANNEL_NUM_2)
        {
            stream_cfg.channel_map  = (enum AUD_CHANNEL_MAP_T)(AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1);
        }
        else if (stream_cfg.channel_num != AUD_CHANNEL_NUM_1)
        {
            LOG_E("Invalid bis media channel num = %d", stream_cfg.channel_num);
        }
        stream_cfg.io_path      = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.device       = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.sample_rate  = (enum AUD_SAMPRATE_T)pStreamEnv->stream_info.playbackInfo.sample_rate;

        stream_cfg.vol          = pStreamEnv->stream_context.tgt_vol;

        stream_cfg.data_size    = (uint32_t)(2 * pStreamEnv->stream_info.playbackInfo.dmaChunkSize);

        pStreamEnv->func_list->stream_func_list.init_stream_buf_func(pStreamEnv);
#ifdef GAF_CODEC_CROSS_CORE
        gaf_stream_common_update_playback_stream_state(pStreamEnv, GAF_PLAYBACK_STREAM_WAITING_M55_INIT);
#endif
        pStreamEnv->func_list->decoder_func_list->decoder_init_func(pStreamEnv,
            gaf_bis_sink_stream_types_op_rule.playback_ase_count);

        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(pStreamEnv->stream_info.playbackInfo.dmaBufPtr);

        stream_cfg.handler = pStreamEnv->func_list->stream_func_list.playback_dma_irq_handler_func;

#ifdef PLAYBACK_USE_I2S
        stream_cfg.io_path = AUD_IO_PATH_NULL;
        stream_cfg.device = AUD_STREAM_USE_I2S0_MASTER;
        stream_cfg.channel_num = AUD_CHANNEL_NUM_2;
#endif
        af_codec_tune(AUD_STREAM_PLAYBACK, 0);
        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);

#ifdef PLAYBACK_USE_I2S
        hal_cmu_audio_resample_disable();
        pStreamEnv->stream_context.playbackTriggerChannel = 0;
        gaf_media_prepare_playback_trigger(pStreamEnv->stream_context.playbackTriggerChannel);
        af_i2s_sync_config(AUD_STREAM_ID_0,AUD_STREAM_PLAYBACK, AF_I2S_SYNC_TYPE_BT, false);
        af_i2s_sync_config(AUD_STREAM_ID_0,AUD_STREAM_PLAYBACK, AF_I2S_SYNC_TYPE_BT, true);
#else
        pStreamEnv->stream_context.playbackTriggerChannel = app_bt_sync_get_available_trigger_channel(APP_BT_SYNC_OP_MAX, APP_BT_SYNC_POLICY_DEFAULT);
        gaf_media_prepare_playback_trigger(pStreamEnv->stream_context.playbackTriggerChannel);
#endif
        // put PID env into stream context
        gaf_media_pid_init(&(pStreamEnv->stream_context.playback_pid_env));

        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
#ifndef GAF_CODEC_CROSS_CORE
        gaf_stream_common_update_playback_stream_state(pStreamEnv, GAF_PLAYBACK_STREAM_INITIALIZED);
#endif
        bes_ble_bap_dp_itf_data_come_callback_register((void *)gaf_bis_stream_receive_data);

        return 0;
    }

    return -1;
}

static void gaf_bis_audio_media_playback_buf_init(void* _pStreamEnv)
{
    uint8_t* heapBufStartAddr = NULL;
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    GAF_AUDIO_STREAM_COMMON_INFO_T* pCommonInfo = &pStreamEnv->stream_info.playbackInfo;

#ifndef AOB_CODEC_CP
    lc3_alloc_data_free();
#endif
    app_audio_mempool_init_with_specific_size(app_audio_mempool_size());

    uint32_t audioCacheHeapSize = pStreamEnv->stream_info.playbackInfo.maxCachedEncodedAudioPacketCount*
        pStreamEnv->stream_info.playbackInfo.maxEncodedAudioPacketSize;

    app_audio_mempool_get_buff(&heapBufStartAddr, audioCacheHeapSize);
    gaf_stream_heap_init(heapBufStartAddr, audioCacheHeapSize);
    app_audio_mempool_get_buff(&(pStreamEnv->stream_info.playbackInfo.dmaBufPtr),
        pStreamEnv->stream_info.playbackInfo.dmaChunkSize*2);

    gaf_list_new(&pStreamEnv->stream_context.playback_buff_list[0].buff_list,
                (osMutex(gaf_decoder_buffer_mutex)),
                gaf_stream_data_free,
                gaf_stream_heap_cmalloc,
                gaf_stream_heap_free);

    if (pCommonInfo->bisChInfo[1].iso_channel_hdl != 0)
    {
        gaf_list_new(&pStreamEnv->stream_context.playback_buff_list[1].buff_list,
                    (osMutex(gaf_decoder_buffer_mutex_1)),
                    gaf_stream_data_free,
                    gaf_stream_heap_cmalloc,
                    gaf_stream_heap_free);
       app_audio_mempool_get_buff(&(pStreamEnv->stream_info.playbackInfo.decode_buf),
       pStreamEnv->stream_info.playbackInfo.num_channels * pStreamEnv->stream_info.playbackInfo.encoded_frame_size);
    }

    pStreamEnv->func_list->decoder_func_list->decoder_init_buf_func(pStreamEnv,
        gaf_bis_sink_stream_types_op_rule.playback_ase_count);
}

static int gaf_bis_audio_media_stream_stop_handler(void* _pStreamEnv)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    if (GAF_PLAYBACK_STREAM_IDLE != pStreamEnv->stream_context.playback_stream_state)
    {
        pLocalBisStreamEnvPtr = NULL;
        uint8_t POSSIBLY_UNUSED adma_ch = HAL_DMA_CHAN_NONE;
        uint32_t dma_base;
        bes_ble_bap_dp_itf_data_come_callback_deregister();
        gaf_stream_common_update_playback_stream_state(pStreamEnv, GAF_PLAYBACK_STREAM_IDLE);
        af_stream_dma_tc_irq_disable(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        if(adma_ch != HAL_DMA_CHAN_NONE)
        {
            bt_drv_reg_op_disable_dma_tc(adma_ch&0xFF, dma_base);
        }

        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_codec_tune(AUD_STREAM_PLAYBACK, 0);

        gaf_stream_common_clr_trigger(pStreamEnv->stream_context.playbackTriggerChannel);

#ifdef PLAYBACK_USE_I2S
        hal_cmu_audio_resample_enable();
#endif

        app_sysfreq_req(APP_SYSFREQ_USER_AOB, APP_SYSFREQ_32K);
        af_set_priority(AF_USER_AI, osPriorityAboveNormal);

        pStreamEnv->func_list->decoder_func_list->decoder_deinit_func();
        pStreamEnv->func_list->stream_func_list.deinit_stream_buf_func(pStreamEnv);

        return 0;
    }

    return -1;

}

static uint32_t gaf_bis_stream_media_dma_irq_handler(uint8_t *buf, uint32_t len)
{
    uint32_t btclk; //hal slot -- 312.5us
    uint16_t btcnt; //hal  microsecond -- 0.5 us
    uint32_t dmaIrqHappeningTimeUs = 0;
    uint8_t adma_ch = HAL_DMA_CHAN_NONE;
    uint32_t dma_base;
    adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    if (adma_ch != HAL_DMA_CHAN_NONE)
    {
        bt_drv_reg_op_dma_tc_clkcnt_get_by_ch(&btclk, &btcnt, adma_ch&0xFF, dma_base);
        dmaIrqHappeningTimeUs = bt_syn_ble_bt_time_to_bts(btclk, btcnt);
    }

    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_bis_audio_stream_env;

    // it's possible the multiple DMA irq triggered message accumulates,
    // so the acuiqred dmaIrqHappeningTimeUs is the last dma irq, which has
    // been handled. For this case, ignore this dma irq message handling
    if ((!pStreamEnv) ||
        (GAF_PLAYBACK_STREAM_START_TRIGGERING > pStreamEnv->stream_context.playback_stream_state) ||
        ((GAF_PLAYBACK_STREAM_STREAMING_TRIGGERED == pStreamEnv->stream_context.playback_stream_state) &&
        (dmaIrqHappeningTimeUs == pStreamEnv->stream_context.lastPlaybackDmaIrqTimeUs)))
    {
        memset(buf, 0, len);
        return len;
    }

    gaf_stream_common_updated_expeceted_playback_seq_and_time(pStreamEnv,
    pStreamEnv->stream_info.playbackInfo.trigger_stream_lid, dmaIrqHappeningTimeUs);

    if (GAF_PLAYBACK_STREAM_STREAMING_TRIGGERED !=
        pStreamEnv->stream_context.playback_stream_state)
    {
        gaf_stream_common_update_playback_stream_state(pStreamEnv,
            GAF_PLAYBACK_STREAM_STREAMING_TRIGGERED);
        gaf_stream_common_clr_trigger(pStreamEnv->stream_context.playbackTriggerChannel);
        pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX]--;
        LOG_D("Trigger ticks %d Update playback seq to %d",
            dmaIrqHappeningTimeUs,
            pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX]);
    }

#ifdef GAF_DECODER_CROSS_CORE_USE_M55
    if (is_support_ble_audio_mobile_m55_decode == false)
    {
            bool ret = gaf_bis_decoder_core_fetch_pcm_data(pStreamEnv, pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX],
                buf, len, dmaIrqHappeningTimeUs);
            if (false == ret)
            {
                memset(buf, 0, len);
            }
    }
#else
    int32_t diff_bt_time = 0;
    uint8_t *decode_buf_temp     = NULL;
    uint32_t decode_buf_temp_len = 0;
    uint32_t expected_buf_len = pStreamEnv->stream_info.playbackInfo.encoded_frame_size*pStreamEnv->stream_info.playbackInfo.num_channels;

#ifdef ADVANCE_FILL_ENABLED
    // Broadcast sink prefill is not ready yet
    dmaIrqHappeningTimeUs -= pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs;
#endif

    uint64_t revlersal_expected_time = 1;
    uint64_t revlersal_current_time = 1;
    gaf_media_data_t *decoder_frame_p = NULL;
    gaf_media_data_t *decoder_frame_p1 = NULL;

    decoder_frame_p = gaf_stream_common_get_packet(pStreamEnv,
        pStreamEnv->stream_info.playbackInfo.trigger_stream_lid, dmaIrqHappeningTimeUs);
    if (pStreamEnv->stream_info.playbackInfo.bisChInfo[1].iso_channel_hdl)
    {
        gaf_stream_buff_list_t *list = NULL;

        if (pStreamEnv->stream_info.playbackInfo.trigger_stream_lid)
        {
            list = &pStreamEnv->stream_context.playback_buff_list[0].buff_list;
        }
        else
        {
            list = &pStreamEnv->stream_context.playback_buff_list[1].buff_list;
        }
        list->node = gaf_list_begin(list);
        if (NULL != list->node)
        {
            decoder_frame_p1 =  (gaf_media_data_t *)gaf_list_node(list);
            gaf_list_only_remove_node(list, decoder_frame_p1);
        }
    }

#ifdef ADVANCE_FILL_ENABLED
    dmaIrqHappeningTimeUs += pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs;
#endif
    diff_bt_time = GAF_AUDIO_CLK_32_BIT_DIFF(decoder_frame_p->time_stamp, dmaIrqHappeningTimeUs) - pStreamEnv->stream_info.playbackInfo.presDelayUs;

    if(abs(diff_bt_time) > TIME_CALCULATE_REVERSAL_THRESHOLD)
    {
        decoder_frame_p->time_stamp += pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs;
        dmaIrqHappeningTimeUs += pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs;
        if(dmaIrqHappeningTimeUs > decoder_frame_p->time_stamp)
        {
            revlersal_expected_time <<= 32;
            revlersal_expected_time += decoder_frame_p->time_stamp;
            revlersal_current_time = dmaIrqHappeningTimeUs;
            diff_bt_time = revlersal_current_time - revlersal_expected_time;
        }
        else
        {
            revlersal_current_time <<= 32;
            revlersal_expected_time = decoder_frame_p->time_stamp;
            revlersal_current_time += dmaIrqHappeningTimeUs;
            diff_bt_time = revlersal_current_time - revlersal_expected_time;
        }
        LOG_I("WARNIN:time has revleral time_stamp %d dmaIrqHappeningTimeUs %d diff_bt_time %d ", decoder_frame_p->time_stamp, dmaIrqHappeningTimeUs,diff_bt_time);
    }

    gaf_media_pid_adjust(AUD_STREAM_PLAYBACK, &(pStreamEnv->stream_context.playback_pid_env), diff_bt_time);

    if (decoder_frame_p1)
    {
        decode_buf_temp = pStreamEnv->stream_info.playbackInfo.decode_buf;
        memcpy(decode_buf_temp, decoder_frame_p->sdu_data, decoder_frame_p->data_len);
        decode_buf_temp_len = decoder_frame_p->data_len;
        if (!decoder_frame_p->isPLC)
        {
            memcpy(decode_buf_temp+decoder_frame_p->data_len, decoder_frame_p1->sdu_data, decoder_frame_p1->data_len);
            decode_buf_temp_len += decoder_frame_p1->data_len;
        }

        if (decode_buf_temp_len < expected_buf_len)
        {
            decoder_frame_p->isPLC = true;
            decode_buf_temp_len = expected_buf_len;
        }
    }
    else
    {
        decode_buf_temp = decoder_frame_p->sdu_data;
        decode_buf_temp_len = decoder_frame_p->data_len;
    }

    if (decode_buf_temp)
    {
        int ret = pStreamEnv->func_list->decoder_func_list->decoder_decode_frame_func
                (decoder_frame_p->isPLC, decode_buf_temp_len, decode_buf_temp, &pStreamEnv->stream_context.codec_alg_context[0], buf);
        LOG_I("seq 0x%02x expected play time %u local time %u dt_time %d dec ret %d", decoder_frame_p->pkt_seq_nb,
                    decoder_frame_p->time_stamp, dmaIrqHappeningTimeUs, diff_bt_time, ret);
    }
    else
    {
        memset(buf, 0, len);
    }
    gaf_stream_data_free(decoder_frame_p);
    if (decoder_frame_p1)
    {
        gaf_stream_data_free(decoder_frame_p1);
    }
#endif

    return 0;
}

static void gaf_bis_audio_media_playback_buf_deinit(void* _pStreamEnv)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    pStreamEnv->stream_info.playbackInfo.dmaBufPtr = NULL;
    gaf_list_free(&pStreamEnv->stream_context.playback_buff_list[0].buff_list);
    if (pStreamEnv->stream_info.playbackInfo.bisChInfo[1].iso_channel_hdl != 0)
    {
        gaf_list_free(&pStreamEnv->stream_context.playback_buff_list[1].buff_list);
    }
    pStreamEnv->func_list->decoder_func_list->decoder_deinit_buf_func(pStreamEnv,
        gaf_bis_sink_stream_types_op_rule.playback_ase_count);
}

static GAF_AUDIO_FUNC_LIST_T gaf_audio_media_stream_func_list =
{
    {
        .start_stream_func = gaf_bis_audio_media_stream_start_handler,
        .init_stream_buf_func = gaf_bis_audio_media_playback_buf_init,
        .stop_stream_func = gaf_bis_audio_media_stream_stop_handler,
        .playback_dma_irq_handler_func = gaf_bis_stream_media_dma_irq_handler,
        .deinit_stream_buf_func = gaf_bis_audio_media_playback_buf_deinit,
    },
};

static GAF_AUDIO_STREAM_ENV_T *gaf_bis_audio_stream_update_stream_env_from_grp_info(uint8_t stream_lid)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_bis_audio_stream_env;
    GAF_AUDIO_STREAM_COMMON_INFO_T* pCommonInfo = &(pStreamEnv->stream_info.playbackInfo);

    AOB_BIS_GROUP_INFO_T *aob_bis_group_info = ble_audio_earphone_info_get_bis_group_info();
    if (NULL == aob_bis_group_info)
    {
        ASSERT(0, "%s bc sink can not find earbud info", __func__);
    }

    if(pStreamEnv->stream_context.playback_stream_state == GAF_PLAYBACK_STREAM_IDLE)
    {
        pCommonInfo->bisChInfo[0].iso_channel_hdl = aob_bis_group_info->play_stream_info[0].bis_hdl;
        LOG_I("[%s][%d][0]: bis_chn = %d", __FUNCTION__, __LINE__, pCommonInfo->bisChInfo[0].iso_channel_hdl);
        pCommonInfo->bisChInfo[0].select_ch_map = aob_bis_group_info->play_stream_info[0].select_ch_bf;
        pCommonInfo->bisChInfo[0].blocks_size = aob_bis_group_info->play_stream_info[0].blocks_size;
        if (aob_bis_group_info->play_stream_num == 2)
        {
            pCommonInfo->bisChInfo[1].iso_channel_hdl = aob_bis_group_info->play_stream_info[1].bis_hdl;
            LOG_I("[%s][%d][1]: bis_chn = %d", __FUNCTION__, __LINE__, pCommonInfo->bisChInfo[1].iso_channel_hdl);
            pCommonInfo->bisChInfo[1].select_ch_map     = aob_bis_group_info->play_stream_info[1].select_ch_bf;
            pCommonInfo->bisChInfo[1].blocks_size = aob_bis_group_info->play_stream_info[1].blocks_size;
        }
        else
        {
            pCommonInfo->bisChInfo[1].iso_channel_hdl   = 0;
            pCommonInfo->bisChInfo[1].select_ch_map     = 0;
            pCommonInfo->bisChInfo[1].blocks_size = 0;
        }

        LOG_I("[%s][%d]: %d, %d, %d", __FUNCTION__, __LINE__, aob_bis_group_info->play_stream_num,
            pCommonInfo->bisChInfo[0].select_ch_map, pCommonInfo->bisChInfo[1].select_ch_map);
        pStreamEnv->stream_info.playbackInfo.num_channels = aob_bis_group_info->play_ch_num;
        pStreamEnv->stream_info.playbackInfo.bits_depth = LC3_BPS;
        pStreamEnv->stream_info.playbackInfo.presDelayUs = aob_bis_group_info->pres_delay?
            aob_bis_group_info->pres_delay : BIS_AUDIO_DEF_PRES_DLAY_US;

        switch (aob_bis_group_info->play_stream_info[0].stream_info.codec_id.codec_id[0])
        {
            case AOB_CODEC_TYPE_LC3:
            {
                AOB_BAP_CFG_T* p_lc3_cfg = (AOB_BAP_CFG_T*)&aob_bis_group_info->play_stream_info[0].stream_info.cfg_param;
                pCommonInfo->frame_ms =
                    gaf_stream_common_frame_duration_parse(p_lc3_cfg->param.frame_dur);
                pCommonInfo->sample_rate =
                    gaf_stream_common_sample_freq_parse(p_lc3_cfg->param.sampling_freq);
                pCommonInfo->encoded_frame_size = p_lc3_cfg->param.frame_octet;
                pCommonInfo->sdu_num            = p_lc3_cfg->param.frames_sdu ? p_lc3_cfg->param.frames_sdu : 1;
                pCommonInfo->maxCachedEncodedAudioPacketCount = GAF_AUDIO_MEDIA_DATA_PACKET_NUM_LIMITER;
                pCommonInfo->maxEncodedAudioPacketSize = gaf_audio_lc3_encoder_get_max_frame_size();
                pCommonInfo->dmaChunkIntervalUs = (uint32_t)(pCommonInfo->frame_ms*1000);
                pCommonInfo->dmaChunkSize =
                    (uint32_t)((pCommonInfo->sample_rate*
                    (pCommonInfo->bits_depth/8)*
                    (pCommonInfo->dmaChunkIntervalUs)*
                    pCommonInfo->num_channels)/1000/1000);
                gaf_audio_lc3_update_codec_func_list(pStreamEnv);
                break;
            }
            default:
                ASSERT(false, "unknown codec type!");
                return NULL;
        }
    }

    return pStreamEnv;
}

#ifdef GAF_CODEC_CROSS_CORE
/**
 ****************************************************************************************
 * @brief When bth bis send the deinit signal to m55 before bth deinit m55 core
 *
 * @param[in] NONE                 NONE
 * @param[in] NONE                 NONE
 *
 * @param[out] NONE                NONE
 ****************************************************************************************
 */
static void gaf_bis_audio_send_deinit_signal_to_m55(void)
{
    GAF_AUDIO_M55_DEINIT_T p_deinit_req;
    p_deinit_req.con_lid         = gaf_m55_deinit_status.con_lid;
    p_deinit_req.context_type    = gaf_m55_deinit_status.context_type;
    p_deinit_req.is_bis          = gaf_m55_deinit_status.is_bis;
    p_deinit_req.is_bis_src      = gaf_m55_deinit_status.is_bis_src;
    p_deinit_req.capture_deinit  = gaf_m55_deinit_status.capture_deinit;
    p_deinit_req.playback_deinit = gaf_m55_deinit_status.playback_deinit;
    p_deinit_req.is_mobile_role  = gaf_m55_deinit_status.is_mobile_role;

    if (true == gaf_m55_deinit_status.playback_deinit){
        app_dsp_m55_bridge_send_cmd(
            CROSS_CORE_TASK_CMD_GAF_DECODE_DEINIT_WAITING_RSP,
            (uint8_t*)&p_deinit_req,
            sizeof(GAF_AUDIO_M55_DEINIT_T));
    }
    return ;
}
#endif

static void _gaf_bis_audio_stream_stop_handler(uint8_t stream_lid)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_bis_audio_stream_env;

    if (pStreamEnv &&
        GAF_PLAYBACK_STREAM_IDLE != pStreamEnv->stream_context.playback_stream_state)
    {
        pStreamEnv->func_list->stream_func_list.stop_stream_func(pStreamEnv);

        bt_adapter_write_sleep_enable(1);
#ifdef GAF_CODEC_CROSS_CORE
        gaf_m55_deinit_status.con_lid         = 0;
        gaf_m55_deinit_status.context_type    = 0;
        gaf_m55_deinit_status.is_bis          = true;
        gaf_m55_deinit_status.is_bis_src      = false;
        gaf_m55_deinit_status.playback_deinit = true;
        gaf_m55_deinit_status.capture_deinit  = false;
        gaf_m55_deinit_status.is_mobile_role  = false;

        gaf_bis_audio_send_deinit_signal_to_m55();
#endif
    }
#ifdef DSP_SMF
    dsp_close();
#endif
}

void gaf_bis_audio_stream_stop_handler(uint8_t stream_lid)
{
    LOG_I("%s", __func__);
    _gaf_bis_audio_stream_stop_handler(stream_lid);
}

static void _gaf_bis_audio_stream_start_handler(uint8_t stream_lid)
{
#ifdef DSP_SMF
    dsp_open();
    osDelay(150);
#endif
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = gaf_bis_audio_stream_update_stream_env_from_grp_info(stream_lid);

    if (pStreamEnv)
    {
        bt_adapter_write_sleep_enable(0);
        pStreamEnv->func_list->stream_func_list.start_stream_func(pStreamEnv);
    }
}

void gaf_bis_audio_stream_start_handler(uint8_t stream_lid)
{
    // TODO: triggered by bt media manager
    LOG_I("%s, stream_lid=%d", __func__, stream_lid);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_bis_audio_stream_env;
    if(pStreamEnv->stream_context.playback_stream_state != GAF_PLAYBACK_STREAM_IDLE)
    {
        LOG_I("%s, stream started", __func__);
        return;
    }
    app_bt_start_custom_function_in_bt_thread((uint32_t)stream_lid,
                                              0,
                                              (uint32_t)_gaf_bis_audio_stream_start_handler);
}

void gaf_bis_stream_dump_dma_trigger_status()
{
    uint8_t context_id = 0;
    char state_str[30];
    GAF_PLAYBACK_STREAM_STATE_E trigger_status = gaf_bis_audio_stream_env.stream_context.playback_stream_state;
    sprintf(state_str, "context %d state %d ", context_id, trigger_status);
    LOG_I("gaf tri: %s", state_str);
}

GAF_PLAYBACK_STREAM_STATE_E gaf_bis_stream_get_playback_stream_state(uint8_t con_lid)
{
    if (con_lid >= GAF_MAXIMUM_CONNECTION_COUNT)
    {
        return GAF_PLAYBACK_STREAM_IDLE;
    }

    return gaf_bis_audio_stream_env.stream_context.playback_stream_state;
}

void gaf_bis_audio_stream_set_stream_volume(int8_t vol)
{
    struct AF_STREAM_CONFIG_T *stream_cfg = NULL;

    if (vol > TGT_VOLUME_LEVEL_MAX)
    {
        vol = TGT_VOLUME_LEVEL_MAX;
    }

    if (vol < TGT_VOLUME_LEVEL_MUTE)
    {
        vol = TGT_VOLUME_LEVEL_MUTE;
    }

    gaf_bis_audio_stream_env.stream_context.tgt_vol = vol;

    LOG_I("%s tgt vol = %d", __func__, vol);

    if (gaf_bis_audio_stream_env.stream_context.playback_stream_state ==
        GAF_PLAYBACK_STREAM_IDLE)
    {
        return;
    }

    if ((af_stream_get_cfg(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg, false) == 0) &&
        stream_cfg != NULL)
    {
        stream_cfg->vol = vol;
        af_stream_setup(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, stream_cfg);
    }
}

void gaf_bis_audio_stream_init(void)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_bis_audio_stream_env;
    memset((uint8_t *)pStreamEnv, 0, sizeof(GAF_AUDIO_STREAM_ENV_T));
    pStreamEnv->stream_context.playback_stream_state = GAF_PLAYBACK_STREAM_IDLE;
    pStreamEnv->stream_context.capture_stream_state = GAF_CAPTURE_STREAM_IDLE;
    pStreamEnv->stream_context.tgt_vol = TGT_VOLUME_LEVEL_7;
    pStreamEnv->stream_info.playbackInfo.aseChInfo[0].iso_channel_hdl = GAF_AUDIO_INVALID_ISO_CHANNEL;
    pStreamEnv->stream_info.captureInfo.aseChInfo[0].iso_channel_hdl = GAF_AUDIO_INVALID_ISO_CHANNEL;
    pStreamEnv->stream_info.playbackInfo.presDelayUs = GAF_AUDIO_CONTROLLER_2_HOST_LATENCY_US;

    GAF_AUDIO_STREAM_INFO_T* pStreamInfo;

    pStreamInfo = &(gaf_bis_audio_stream_env.stream_info);
    pStreamInfo->contextType = GAF_AUDIO_STREAM_CONTEXT_TYPE_MEDIA;
    gaf_stream_common_register_func_list(&gaf_bis_audio_stream_env,
        &gaf_audio_media_stream_func_list);
}

/// @} APP
#endif
