/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifdef AOB_MOBILE_ENABLED
#ifdef BLE_USB_AUDIO_SUPPORT
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_dma.h"
#include "hal_trace.h"
#include "app_trace_rx.h"
#include "bluetooth_bt_api.h"
#include "app_bt_func.h"
#include "app_utils.h"
#include "plat_types.h"
#include "audioflinger.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_interface.h"
#include "app_audio.h"
#include "app_ble_usb_audio.h"
#include "app_ble_usb_stream_stm.h"
#include "usb_audio_app.h"
#include "gaf_mobile_media_stream.h"
#include "aob_media_api.h"
#include "aob_mgr_gaf_evt.h"
#include "ble_audio_dbg.h"
#include "gaf_media_sync.h"
#include "gaf_media_pid.h"
#include "lc3_process.h"
#include "gaf_codec_lc3.h"
#include "app_bap_data_path_itf.h"
#include "gaf_media_common.h"
#include "ble_audio_ase_stm.h"
#include "app_bt_sync.h"
#include "rwble_config.h"

extern GAF_AUDIO_STREAM_CONTEXT_TYPE_E g_cur_context_type ;
extern GAF_AUDIO_STREAM_ENV_T gaf_mobile_audio_stream_env;
extern "C" void app_usbaudio_entry(void);
extern "C" uint32_t btdrv_reg_op_cig_anchor_timestamp(uint8_t link_id);

static void gaf_mobile_usb_start_debounce_handler(void const *param);
osTimerDef (GAF_USB_CAPTURE_START_DEBOUNCE, gaf_mobile_usb_start_debounce_handler);
static osTimerId gaf_mobile_usb_start_debounce_timer = NULL;

//TODO: need a better way to replace USB_CAPTURE_START_DEBOUNCE_TIME_MS
#define USB_CAPTURE_START_DEBOUNCE_TIME_MS          1000

static void usb_ble_audio_update_timing_test_handler(void)
{
    AOB_BAP_CIG_PARAM_T cig_param;
    cig_param.c2p_bn = 1,
    cig_param.c2p_nse = 2;
    cig_param.p2c_bn = 1,
    cig_param.p2c_nse = 2,
    cig_param.frame_cnt_per_sdu = 1,

#ifdef BLE_USB_AUDIO_SUPPORT
    cig_param.c2p_ft = 2,
    cig_param.p2c_ft = 2,
#else
    cig_param.c2p_ft = 3,
    cig_param.p2c_ft = 3,
#endif

#ifdef AOB_LOW_LATENCY_MODE
    cig_param.iso_interval_1_25ms = 4,  // 5ms = 4 * 1.25ms
#else
#ifdef BLE_AUDIO_FRAME_DUR_7_5MS
    cig_param.iso_interval_1_25ms = 6,  // 7.5ms = 6 * 1.25ms
#else
    cig_param.iso_interval_1_25ms = 8,  // 5ms = 8 * 1.25ms
#endif
#endif

    TRACE(0, "Update USB AUDIO CIG param:");
    TRACE(0, "m2s_bn %d", cig_param.c2p_bn);
    TRACE(0, "m2s_nse %d", cig_param.c2p_nse);
    TRACE(0, "m2s_ft %d", cig_param.c2p_ft);
    TRACE(0, "s2m_bn %d", cig_param.p2c_bn);
    TRACE(0, "s2m_nse %d", cig_param.p2c_nse);
    TRACE(0, "s2m_ft %d", cig_param.p2c_ft);
    TRACE(0, "frame_cnt_per_sdu %d", cig_param.frame_cnt_per_sdu);
    TRACE(0, "iso_interval %d", cig_param.iso_interval_1_25ms);

    bes_ble_bap_ascc_prepare_cig_param(&cig_param);
}

extern uint8_t aob_media_mobile_get_cur_streaming_ase_lid(uint8_t con_lid, \
    AOB_MGR_DIRECTION_E direction);
extern void aob_media_mobile_release_stream(uint8_t ase_id);

void app_ble_usb_media_ascc_release_stream_test(void)
{
    TRACE(1,"%s",__func__);
    aob_media_mobile_release_stream(0);
    aob_media_mobile_release_stream(1);
}

void app_ble_usb_media_ascc_start_stream_test(void)
{
    TRACE(1,"%s",__func__);

    // Default cis count in cig
    bes_ble_bap_ascc_set_cis_count_in_cig(bes_ble_gap_connection_count());

#ifdef AOB_LOW_LATENCY_MODE
    bes_ble_bap_ascc_set_sdu_interval(5000, 5000);
#endif /* AOB_LOW_LATENCY_MODE */

#ifdef BLE_AUDIO_FRAME_DUR_7_5MS
    bes_ble_bap_ascc_set_sdu_interval(7500, 7500);
#endif

    AOB_MEDIA_ASE_CFG_INFO_T ase_to_start =
    {
        APP_GAF_BAP_SAMPLING_FREQ_48000HZ, 120, APP_GAF_DIRECTION_SINK, AOB_CODEC_ID_LC3,  APP_GAF_BAP_CONTEXT_TYPE_MEDIA
    };

    for (uint32_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++) {
        aob_media_mobile_start_stream(&ase_to_start, i, 1);
    }
}

AOB_MGR_STREAM_STATE_E app_ble_usb_media_get_cur_ase_state(void)
{
    uint8_t ase_lid = aob_media_mobile_get_cur_streaming_ase_lid(0, AOP_MGR_DIRECTION_SINK);
    return aob_media_mobile_get_cur_ase_state(ase_lid);
}

static void gaf_media_usb_prepare_playback_trigger(uint8_t trigger_channel)
{
    LOG_D("%s start trigger_channel %d", __func__, trigger_channel);

    uint8_t adma_ch = HAL_DMA_CHAN_NONE;
    uint32_t dma_base;
    af_stream_dma_tc_irq_enable(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK);
    adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK);
    dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK);
    if(adma_ch != HAL_DMA_CHAN_NONE)
    {
        bt_drv_reg_op_enable_dma_tc(adma_ch&0xFF, dma_base);
    }
    else
    {
        LOG_I("error adma_ch HAL_DMA_CHAN_NONE %d", adma_ch);
    }
    btdrv_syn_clr_trigger(trigger_channel);
    af_codec_sync_config(AUD_STREAM_PLAYBACK, AF_CODEC_SYNC_TYPE_BT, false);
    af_codec_sync_config(AUD_STREAM_PLAYBACK, AF_CODEC_SYNC_TYPE_BT, true);

    LOG_D("%s end", __func__);
}

static void gaf_media_usb_prepare_capture_trigger(uint8_t trigger_channel)
{
    LOG_D("%s start", __func__);
    uint8_t adma_ch = 0;
    uint32_t dma_base;
    btdrv_syn_clr_trigger(trigger_channel);

    af_stream_dma_tc_irq_enable(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
    adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
    dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
    if(adma_ch != HAL_DMA_CHAN_NONE)
    {
        bt_drv_reg_op_enable_dma_tc(adma_ch&0xFF, dma_base);
    }
    af_codec_sync_config(AUD_STREAM_CAPTURE, AF_CODEC_SYNC_TYPE_BT, false);
    af_codec_sync_config(AUD_STREAM_CAPTURE, AF_CODEC_SYNC_TYPE_BT, true);
    LOG_D("%s end", __func__);
}

static uint32_t gaf_mobile_calculate_trigger_time(GAF_AUDIO_STREAM_ENV_T* pStreamEnv)
{
    uint32_t trigger_bt_time = gaf_media_sync_get_curr_time() +
                               GAF_AUDIO_CAPTURE_TRIGGER_DELAY_IN_US;
    return trigger_bt_time;
}

static void* gaf_mobile_audio_get_media_stream_env(void)
{
    return &gaf_mobile_audio_stream_env;
}

void gap_mobile_start_usb_audio_receiving_dma(void)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv =
        (GAF_AUDIO_STREAM_ENV_T*)gaf_mobile_audio_get_media_stream_env();

    if (pStreamEnv == NULL)
    {
         LOG_E("%s, pStreamEnv is NULL", __func__);
         return;
    }

    LOG_I("%s", __func__);

    gaf_media_pid_init(&(pStreamEnv->stream_context.playback_pid_env));

    gaf_media_usb_prepare_playback_trigger(pStreamEnv->stream_context.playbackTriggerChannel);

    uint32_t trigger_bt_time = gaf_mobile_calculate_trigger_time(pStreamEnv);

    gaf_stream_common_set_capture_trigger_time_generic(pStreamEnv, AUD_STREAM_PLAYBACK,
                                                       trigger_bt_time);
}

void gap_mobile_start_usb_audio_transmission_dma(void)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv =
        (GAF_AUDIO_STREAM_ENV_T*)gaf_mobile_audio_get_media_stream_env();

    if (pStreamEnv == NULL)
    {
         LOG_E("%s, pStreamEnv is NULL", __func__);
         return;
    }
    LOG_I("%s", __func__);

    gaf_media_pid_init(&(pStreamEnv->stream_context.capture_pid_env));
    gaf_media_pid_update_threshold(&(pStreamEnv->stream_context.capture_pid_env),
        (1000000/pStreamEnv->stream_info.playbackInfo.sample_rate) + 1);
    app_bap_dp_itf_data_come_callback_register((void *)gaf_mobile_audio_receive_data);
    gaf_media_usb_prepare_capture_trigger(pStreamEnv->stream_context.captureTriggerChannel);
}

static void gaf_usb_mobile_audio_process_pcm_data_send(void *pStreamEnv_,void *payload_,
    uint32_t payload_size, uint32_t ref_time)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T* )pStreamEnv_;
#if defined(BLE_AUDIO_USE_TWO_CHANNEL_SINK_FOR_DONGLE) || defined(BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT)
    uint32_t payload_len_per_channel = payload_size;
#else
    uint32_t payload_len_per_channel = payload_size/pStreamEnv->stream_info.captureInfo.num_channels;
#endif
    uint8_t ase_count = gaf_cis_mobile_stream_type_op_rule.capture_ase_count;
    uint8_t *payload = (uint8_t *)payload_;
    for (uint8_t i = 0; i < ase_count; i++) {
        bes_ble_bap_iso_dp_send_data(pStreamEnv->stream_info.captureInfo.aseChInfo[i].ase_handle,
                                pStreamEnv->stream_context.latestCaptureSeqNum,
                                payload, payload_len_per_channel,
                                ref_time);
#ifndef BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT
        payload+=payload_len_per_channel;
#endif
    }
}

static void gaf_usb_mobile_audio_process_pcm_data(GAF_AUDIO_STREAM_ENV_T *_pStreamEnv, uint8_t *ptrBuf, uint32_t length)
{
    uint32_t dmaIrqHappeningTimeUs = 0;
    uint8_t adma_ch = HAL_DMA_CHAN_NONE;
    uint32_t dma_base;
    uint32_t btclk; //hal slot -- 312.5us
    uint16_t btcnt; //hal microsecond -- 0.5 us
    GAF_AUDIO_STREAM_INFO_T stream_info = _pStreamEnv->stream_info;
    GAF_AUDIO_STREAM_CONTEXT_T stream_context = _pStreamEnv->stream_context;

    adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK);
    dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK);
    if (adma_ch != HAL_DMA_CHAN_NONE)
    {
        bt_drv_reg_op_dma_tc_clkcnt_get_by_ch(&btclk, &btcnt,adma_ch&0xFF, dma_base);
        dmaIrqHappeningTimeUs = bt_syn_ble_bt_time_to_bts(btclk, btcnt);
    }

    // it's possible the multiple DMA irq triggered message accumulates,
    // so the acuiqred dmaIrqHappeningTimeUs is the last dma irq, which has
    // been handled. For this case, ignore this dma irq message handling
    if ((GAF_CAPTURE_STREAM_STREAMING_TRIGGERED ==stream_context.capture_stream_state) &&
        (dmaIrqHappeningTimeUs ==stream_context.lastCaptureDmaIrqTimeUs))
    {
        LOG_W("accumulated irq messages happen!");
        return;
    }

    if (GAF_CAPTURE_STREAM_STREAMING_TRIGGERED != stream_context.capture_stream_state)
    {
        uint32_t expectedDmaIrqHappeningTimeUs =
            _pStreamEnv->stream_context.lastCaptureDmaIrqTimeUs +
            (uint32_t)_pStreamEnv->stream_context.captureAverageDmaChunkIntervalUs;

        int32_t gapUs = GAF_AUDIO_CLK_32_BIT_DIFF(dmaIrqHappeningTimeUs, expectedDmaIrqHappeningTimeUs);

        int32_t gapUs_abs = GAF_AUDIO_ABS(gapUs);

        if ((gapUs > 0) && (gapUs_abs > (int32_t)_pStreamEnv->stream_info.captureInfo.dmaChunkIntervalUs/2)) {
            LOG_I("%s, gapUs = %d, dmaChunkIntervalUs = %d", __func__, gapUs, _pStreamEnv->stream_info.captureInfo.dmaChunkIntervalUs);
            return;
        }
        gaf_stream_common_update_capture_stream_state(_pStreamEnv,
                                                     GAF_CAPTURE_STREAM_STREAMING_TRIGGERED);
        gaf_stream_common_clr_trigger(stream_context.playbackTriggerChannel);
        uint32_t latest_iso_bt_time = btdrv_reg_op_cig_anchor_timestamp(
            BLE_ISOHDL_TO_ACTID(_pStreamEnv->stream_info.captureInfo.aseChInfo[0].iso_channel_hdl));
        _pStreamEnv->stream_context.usSinceLatestAnchorPoint = GAF_AUDIO_CLK_32_BIT_DIFF(dmaIrqHappeningTimeUs, latest_iso_bt_time);

    }

    gaf_stream_common_playback_timestamp_checker(_pStreamEnv, dmaIrqHappeningTimeUs);

    dmaIrqHappeningTimeUs += (uint32_t)stream_info.captureInfo.dmaChunkIntervalUs;
    LOG_D("length %d encoded_len %d filled timestamp %u", length,
        _pStreamEnv->stream_info.captureInfo.encoded_frame_size,
        dmaIrqHappeningTimeUs);

    _pStreamEnv->func_list->encoder_func_list->encoder_encode_frame_func(_pStreamEnv, dmaIrqHappeningTimeUs,
        length, ptrBuf,&_pStreamEnv->stream_context.codec_alg_context[0],
        &gaf_usb_mobile_audio_process_pcm_data_send);


}

static void gaf_mobile_usb_processing_received_data(uint8_t* ptrBuf, uint32_t length)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T*)gaf_mobile_audio_get_media_stream_env();

    if (pStreamEnv && (pStreamEnv->stream_context.capture_stream_state > GAF_CAPTURE_STREAM_INITIALIZED)) {
        gaf_usb_mobile_audio_process_pcm_data(pStreamEnv, ptrBuf, length);
    }
}

static void gaf_usb_mobile_audio_process_encoded_data(GAF_AUDIO_STREAM_ENV_T *pStreamEnv, uint8_t *ptrBuf, uint32_t length)
{
    LOG_D("%s start, len = %d", __func__,length);
    uint32_t btclk; //hal slot -- 312.5us
    uint16_t btcnt; //hal  microsecond -- 0.5 us
    uint32_t dmaIrqHappeningTimeUs = 0;
    uint8_t adma_ch = HAL_DMA_CHAN_NONE;
    uint32_t dma_base;
    uint32_t len = 0;

    adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
    dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
    len = length;
    if (adma_ch != HAL_DMA_CHAN_NONE)
    {
        bt_drv_reg_op_dma_tc_clkcnt_get_by_ch(&btclk, &btcnt, adma_ch&0xFF, dma_base);
        dmaIrqHappeningTimeUs = bt_syn_ble_bt_time_to_bts(btclk, btcnt);
    }

    // it's possible the multiple DMA irq triggered message accumulates,
    // so the acuiqred dmaIrqHappeningTimeUs is the last dma irq, which has
    // been handled. For this case, ignore this dma irq message handling
    if ((GAF_PLAYBACK_STREAM_STREAMING_TRIGGERED ==
    pStreamEnv->stream_context.playback_stream_state) &&
    (dmaIrqHappeningTimeUs ==
    pStreamEnv->stream_context.lastPlaybackDmaIrqTimeUs))
    {
        memset(ptrBuf, 0, length);
        return;
    }

    gaf_stream_common_updated_expeceted_playback_seq_and_time(pStreamEnv, GAF_AUDIO_DFT_PLAYBACK_LIST_IDX, dmaIrqHappeningTimeUs);

    if (GAF_PLAYBACK_STREAM_START_TRIGGERING ==
        pStreamEnv->stream_context.playback_stream_state)
    {
        gaf_stream_common_update_playback_stream_state(pStreamEnv,
            GAF_PLAYBACK_STREAM_STREAMING_TRIGGERED);
        gaf_stream_common_clr_trigger(pStreamEnv->stream_context.captureTriggerChannel);
#ifdef GAF_DECODER_CROSS_CORE_USE_M55
        if (true == is_support_ble_audio_mobile_m55_decode)
        {
            pStreamEnv->stream_context.lastestPlaybackSeqNumR--;
            pStreamEnv->stream_context.lastestPlaybackSeqNumL--;
        }
        else
        {
            pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX]--;
        }
#else
            pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX]--;
#endif

        LOG_I("Update playback seq to 0x%x", pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX]);
    }

    bool isOneChannelNoData = false;
    uint8_t ase_count = gaf_cis_mobile_stream_type_op_rule.playback_ase_count;
    GAF_AUDIO_STREAM_COMMON_INFO_T playbackInfo = pStreamEnv->stream_info.playbackInfo;
    uint8_t num_channels = playbackInfo.num_channels;
    uint32_t sample_cnt;

#ifdef __BLE_AUDIO_24BIT__
    if (num_channels == 1)
    {
        sample_cnt = (len/sizeof(int32_t))/2;
    }
    else if (num_channels == 2)
    {
        sample_cnt = (len/sizeof(int32_t));
    }
    else if (num_channels == 4)
    {
        sample_cnt = (len/sizeof(int32_t))*2;
    }
    else
    {
        ASSERT(0, "%s:%d,num_channels err:%d", __func__, __LINE__, num_channels)
    }
    int32_t pcm_buf[ase_count][sample_cnt];
#else
    if (num_channels == 1)
    {
        sample_cnt = (len/sizeof(int16_t))/2;
    }
    else if (num_channels == 2)
    {
        sample_cnt = (len/sizeof(int16_t));
    }
    //todo: need to verrity lc3 codec 4ch
    else if (num_channels == 4)
    {
        sample_cnt = (len/sizeof(int16_t))*2;
    }
    else
    {
        ASSERT(0, "%s:%d,num_channels err:%d", __func__, __LINE__, num_channels)
    }
    int16_t pcm_buf[ase_count][sample_cnt];
#endif
    memset(pcm_buf, 0, sizeof(pcm_buf));

    LOG_I("%s contextType %d ase_count %d,sample_cnt %d,num_channel %d",
        __func__ ,pStreamEnv->stream_info.contextType, ase_count,sample_cnt,num_channels);

    for (uint8_t i = 0; i < ase_count; i++) {
        if (playbackInfo.aseChInfo[i].iso_channel_hdl == GAF_AUDIO_INVALID_ISO_CHANNEL) {
             continue;
        }

        int ret = 0;
        POSSIBLY_UNUSED int32_t diff_bt_time = 0;
        gaf_media_data_t *decoder_frame_p = NULL;
        GAF_AUDIO_STREAM_COMMON_INFO_T playback_Info = pStreamEnv->stream_info.playbackInfo;
        decoder_frame_p = gaf_mobile_audio_get_packet(pStreamEnv, dmaIrqHappeningTimeUs,
                playback_Info.aseChInfo[i].iso_channel_hdl);

        LOG_I("%s %d %d %d length %d", __func__, i, decoder_frame_p->data_len,
                playback_Info.aseChInfo[i].iso_channel_hdl, length);

        ret = pStreamEnv->func_list->decoder_func_list->decoder_decode_frame_func
                (pStreamEnv, decoder_frame_p->data_len, decoder_frame_p->sdu_data,
                &pStreamEnv->stream_context.codec_alg_context[i], pcm_buf[i]);

        LOG_D("dec ret %d ", ret);
        if (i == 0)
        {
            diff_bt_time = GAF_AUDIO_CLK_32_BIT_DIFF(decoder_frame_p->time_stamp +
                                                    (GAF_AUDIO_FIXED_MOBILE_SIDE_PRESENT_LATENCY_US),
                                                    dmaIrqHappeningTimeUs);
            gaf_media_pid_adjust(AUD_STREAM_CAPTURE, &(pStreamEnv->stream_context.playback_pid_env),
                                            diff_bt_time);
            LOG_D("index %d Decoded seq 0x%02x expected play time %u local time %u diff %d", i, decoder_frame_p->pkt_seq_nb,
                    decoder_frame_p->time_stamp, dmaIrqHappeningTimeUs, diff_bt_time);
        }
        gaf_stream_data_free(decoder_frame_p);
        }
#if defined(AUDIO_STREAM_TEST_AUDIO_DUMP)
    audio_dump_add_channel_data_from_multi_channels_32bit_to_16bit(0 , pcm_buf[0],sample_cnt, 2, 0, 8);
    audio_dump_add_channel_data_from_multi_channels_32bit_to_16bit(1 , pcm_buf[1],sample_cnt, 2, 1, 8);
    audio_dump_run();
#endif
    // Merge stream data
    if (!isOneChannelNoData) {
#if BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT == 1
        memcpy((void *)ptrBuf, (void *)pcm_buf[0], len);
#else
#ifdef __BLE_AUDIO_24BIT__
        uint16_t i = 0;

        if (num_channels = 1)
        {
            for (uint32_t samples = 0; samples < sample_cnt; samples++)
            {
                ((int32_t *)ptrBuf)[i++] = (int32_t)pcm_buf[0][samples];
                ((int32_t *)ptrBuf)[i++] = (int32_t)pcm_buf[1][samples];
            }
        }
        else
        {
            for (uint32_t samples = 0; samples < sample_cnt; samples++)
            {
                ((int32_t *)ptrBuf)[samples] =
                    (int32_t)(((int32_t)pcm_buf[0][samples] + (int32_t)pcm_buf[1][samples])/2);
            }
        }
#else
        uint16_t i = 0;
        if (num_channels == 1)
        {
            for (uint32_t samples = 0; samples < sample_cnt; samples++)
            {
                ((int16_t *)ptrBuf)[i++] = (int16_t)pcm_buf[0][samples];
                ((int16_t *)ptrBuf)[i++] = (int16_t)pcm_buf[1][samples];
            }
        }
        else
        {
            for (uint32_t samples = 0; samples < sample_cnt; samples++)
            {
                ((int16_t *)ptrBuf)[samples] =
                    (int16_t)(((int16_t)pcm_buf[0][samples] + (int16_t)pcm_buf[1][samples])/2);
            }
        }
        //LOG_I("dump source record pcm data:");
        //DUMP8("%02x ",ptrBuf, 16);
#endif
#endif
    } else {
       memset(ptrBuf, 0, length);
    }
}

static void gaf_mobile_usb_feed_data(uint8_t* ptrBuf, uint32_t length)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T*)gaf_mobile_audio_get_media_stream_env();
    if (pStreamEnv && (pStreamEnv->stream_context.playback_stream_state > GAF_PLAYBACK_STREAM_INITIALIZED)) {
        gaf_usb_mobile_audio_process_encoded_data(pStreamEnv, ptrBuf, length);
    }
}

void gaf_mobile_usb_dma_playback_stop(void)
{
    uint8_t POSSIBLY_UNUSED adma_ch = HAL_DMA_CHAN_NONE;
    uint32_t dma_base;
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T*)gaf_mobile_audio_get_media_stream_env();

    if (pStreamEnv) {
        af_stream_dma_tc_irq_disable(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK);
        adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK);
        dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK);
        if (adma_ch != HAL_DMA_CHAN_NONE)
        {
            bt_drv_reg_op_disable_dma_tc(adma_ch&0xFF, dma_base);
        }

        btdrv_syn_clr_trigger(pStreamEnv->stream_context.playbackTriggerChannel);

        if (pStreamEnv->stream_context.capture_stream_state > GAF_CAPTURE_STREAM_INITIALIZED) {
            gaf_stream_common_update_capture_stream_state(pStreamEnv, GAF_CAPTURE_STREAM_INITIALIZED);
        }
    }
}


void gaf_mobile_usb_dma_capture_stop(void)
{
    uint8_t POSSIBLY_UNUSED adma_ch = HAL_DMA_CHAN_NONE;
    uint32_t dma_base;
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T*)gaf_mobile_audio_get_media_stream_env();

    if (pStreamEnv) {
        af_stream_dma_tc_irq_disable(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
        adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
        dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
        if (adma_ch != HAL_DMA_CHAN_NONE)
        {
            bt_drv_reg_op_disable_dma_tc(adma_ch&0xFF, dma_base);
        }
        btdrv_syn_clr_trigger(pStreamEnv->stream_context.captureTriggerChannel);

        app_bap_dp_itf_data_come_callback_deregister();

        if (pStreamEnv->stream_context.playback_stream_state > GAF_PLAYBACK_STREAM_INITIALIZED) {
            gaf_stream_common_update_playback_stream_state(pStreamEnv, GAF_PLAYBACK_STREAM_INITIALIZED);
        }
    }
}

static bool app_ble_is_usb_playback_on = false;
static bool app_ble_is_usb_capture_on = false;

static void gaf_mobile_usb_set_sysfreq(void)
{
    if (app_ble_is_usb_capture_on && app_ble_is_usb_playback_on)
    {
        app_sysfreq_req(APP_SYSFREQ_USER_AOB, APP_SYSFREQ_208M);
    }
    else if (app_ble_is_usb_capture_on && !app_ble_is_usb_playback_on)
    {
        app_sysfreq_req(APP_SYSFREQ_USER_AOB, APP_SYSFREQ_208M);
    }
    else if (!app_ble_is_usb_capture_on && app_ble_is_usb_playback_on)
    {
        app_sysfreq_req(APP_SYSFREQ_USER_AOB, APP_SYSFREQ_208M);
    }
    else
    {
        app_sysfreq_req(APP_SYSFREQ_USER_AOB, APP_SYSFREQ_32K);
        af_set_priority(AF_USER_AI, osPriorityAboveNormal);
    }
}

static void gaf_mobile_usb_playback_vol_changed(uint32_t level)
{
    for (uint32_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++)
    {
        uint8_t ase_lid = app_bap_uc_cli_get_ase_lid(i, APP_GAF_BAP_UC_ASE_STATE_STREAMING, APP_GAF_DIRECTION_SINK);
        if (ase_lid != 0xFF)
        {
            aob_mobile_vol_set_abs(i, level);
        }
    }
}

static void gaf_mobile_usb_start_debounce_handler(void const *param)
{
    if (!app_ble_is_usb_capture_on) {
        app_ble_is_usb_capture_on = true;
        gaf_mobile_usb_set_sysfreq();
        ble_usb_stream_send_message(ble_usb_get_stream_sm(), USB_REQ_STREAM_CAPTURE_START, 0, 0);
    }
}

static void gaf_mobile_usb_playback_start(void)
{
    app_ble_is_usb_playback_on = true;
    gaf_mobile_usb_set_sysfreq();
    ble_usb_stream_send_message(ble_usb_get_stream_sm(), USB_REQ_STREAM_PLAYBACK_START, 0, 0);
}

static void gaf_mobile_usb_playback_stop(void)
{
    app_ble_is_usb_playback_on = false;
    gaf_mobile_usb_set_sysfreq();
    gaf_mobile_usb_dma_playback_stop();
    ble_usb_stream_send_message(ble_usb_get_stream_sm(), USB_REQ_STREAM_PLAYBACK_STOP, 0, 0);
}

static void gaf_mobile_usb_capture_start(void)
{
    LOG_D("%s", __func__);
    osTimerStart(gaf_mobile_usb_start_debounce_timer, USB_CAPTURE_START_DEBOUNCE_TIME_MS);
}

static void gaf_mobile_usb_capture_stop(void)
{
    LOG_D("%s", __func__);
    osTimerStop(gaf_mobile_usb_start_debounce_timer);
    if (app_ble_is_usb_capture_on) {
        app_ble_is_usb_capture_on = false;
        gaf_mobile_usb_set_sysfreq();
        gaf_mobile_usb_dma_capture_stop();
        ble_usb_stream_send_message(ble_usb_get_stream_sm(), USB_REQ_STREAM_CAPTURE_STOP, 0, 0);
    }
}

static const USB_AUDIO_SOURCE_EVENT_CALLBACK_T ble_usb_audio_stream_func_cb_list =
{
    .data_playback_cb = gaf_mobile_usb_processing_received_data,
    .data_capture_cb = gaf_mobile_usb_feed_data,
    .data_prep_cb = NULL,

    .playback_start_cb = gaf_mobile_usb_playback_start,
    .playback_stop_cb = gaf_mobile_usb_playback_stop,

    .capture_start_cb = gaf_mobile_usb_capture_start,
    .capture_stop_cb = gaf_mobile_usb_capture_stop,

    .playback_vol_change_cb = gaf_mobile_usb_playback_vol_changed,
};

void app_ble_usb_audio_init(void)
{
    ble_usb_stream_stm_init();

    usb_ble_audio_update_timing_test_handler();

    // Default cis count in cig
    bes_ble_bap_ascc_set_cis_count_in_cig(bes_ble_gap_connection_count());

#ifdef AOB_LOW_LATENCY_MODE
    bes_ble_bap_ascc_set_sdu_interval(5000, 5000);
#endif /* AOB_LOW_LATENCY_MODE */

#ifdef BLE_AUDIO_FRAME_DUR_7_5MS
    bes_ble_bap_ascc_set_sdu_interval(7500, 7500);
#endif

    usb_audio_source_config_init(&ble_usb_audio_stream_func_cb_list);

    usb_stream_stm_t* usb_stream_stm_ptr = ble_usb_get_stream_sm();
    if (NULL != usb_stream_stm_ptr) {
        ble_usb_stream_stm_startup(usb_stream_stm_ptr);
    }

    if (gaf_mobile_usb_start_debounce_timer == NULL) {
        gaf_mobile_usb_start_debounce_timer = osTimerCreate (osTimer(GAF_USB_CAPTURE_START_DEBOUNCE), osTimerOnce, NULL);
    }
}

int gaf_mobile_usb_audio_media_stream_start_handler(void* _pStreamEnv)
{
    LOG_I("%s", __func__);

    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    if (GAF_CAPTURE_STREAM_IDLE == pStreamEnv->stream_context.capture_stream_state)
    {
        af_set_priority(AF_USER_AI, osPriorityHigh);

        pStreamEnv->func_list->stream_func_list.capture_init_stream_buf_func(pStreamEnv);

        pStreamEnv->func_list->encoder_func_list->encoder_init_func(pStreamEnv);

        pStreamEnv->stream_context.captureTriggerChannel = app_bt_sync_get_available_trigger_channel(APP_BT_SYNC_OP_MAX, APP_BT_SYNC_POLICY_DEFAULT);

        LOG_D("%s captureTriggerChannel %d", __func__,pStreamEnv->stream_context.captureTriggerChannel);

        // capture pcm data from usb audio
        gaf_stream_common_update_capture_stream_state(pStreamEnv, GAF_CAPTURE_STREAM_INITIALIZED);

        //gap_mobile_start_usb_audio_receiving_dma();
        ble_usb_stream_send_message(ble_usb_get_stream_sm(), EVT_PLAYBACK_STREAM_STARTED, 0, 0);
        LOG_I("%s end", __func__);

        return 0;
    }

    return -1;
}


int gaf_mobile_usb_audio_media_stream_stop_handler(void* _pStreamEnv)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    pStreamEnv->func_list->encoder_func_list->encoder_deinit_func(pStreamEnv);
    pStreamEnv->func_list->stream_func_list.capture_deinit_stream_buf_func(pStreamEnv);

    app_bt_sync_release_trigger_channel(pStreamEnv->stream_context.captureTriggerChannel);

    gaf_stream_common_update_capture_stream_state(pStreamEnv, GAF_CAPTURE_STREAM_IDLE);
#ifdef GAF_CODEC_CROSS_CORE
#ifdef DSP_M55
    app_dsp_m55_deinit();
#endif
#endif
    af_set_priority(AF_USER_AI, osPriorityAboveNormal);
    return 0;
}

int gaf_mobile_usb_audio_capture_start_handler(void* _pStreamEnv)
{
    LOG_I("%s", __func__);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    uint8_t ase_count = gaf_cis_mobile_stream_type_op_rule.playback_ase_count;

    af_set_priority(AF_USER_AI, osPriorityHigh);

    pStreamEnv->func_list->stream_func_list.playback_init_stream_buf_func(pStreamEnv);

    pStreamEnv->func_list->decoder_func_list->decoder_init_func(pStreamEnv, ase_count);

    pStreamEnv->stream_context.playbackTriggerChannel = app_bt_sync_get_available_trigger_channel(APP_BT_SYNC_OP_MAX, APP_BT_SYNC_POLICY_DEFAULT);

    gaf_stream_common_update_playback_stream_state(pStreamEnv, GAF_PLAYBACK_STREAM_INITIALIZED);

    //gap_mobile_start_usb_audio_transmission_dma();
    ble_usb_stream_send_message(ble_usb_get_stream_sm(), EVT_CAPTURE_STREAM_STARTED, 0, 0);

    return 0;
}

int gaf_mobile_usb_audio_capture_stop_handler(void* _pStreamEnv)
{
    LOG_I("%s", __func__);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    pStreamEnv->func_list->decoder_func_list->decoder_deinit_func();
    pStreamEnv->func_list->stream_func_list.playback_deinit_stream_buf_func(pStreamEnv);
    gaf_stream_common_update_playback_stream_state(pStreamEnv, GAF_PLAYBACK_STREAM_IDLE);

    app_bt_sync_release_trigger_channel(pStreamEnv->stream_context.playbackTriggerChannel);
    af_set_priority(AF_USER_AI, osPriorityAboveNormal);
    return 0;
}

#endif
#endif
