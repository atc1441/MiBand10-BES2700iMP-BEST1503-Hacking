/**
 *
 * @copyright Copyright (c) 2015-2021 BES Technic.
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
 */

/*****************************header include********************************/
#if BLE_AUDIO_ENABLED
#include "bluetooth_bt_api.h"
#include "hal_dma.h"
#include "hal_trace.h"
#include "hal_codec.h"
#include "app_trace_rx.h"
#include "app_bt_func.h"
#include "app_utils.h"
#include "plat_types.h"
#include "cqueue.h"
#include "heap_api.h"
#include "audio_dump.h"
#include "audioflinger.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_interface.h"
#include "app_audio.h"
#include "app_overlay.h"
#include "list.h"
#include "lc3_process.h"
#include "besbt_string.h"

#include "gaf_media_sync.h"
#include "gaf_media_common.h"
#include "gaf_codec_lc3.h"
#include "gaf_media_stream.h"
#include "gaf_stream_process.h"
#include "gaf_stream_dbg.h"
#include "app_audio_active_device_manager.h"
#include "app_bt_media_manager.h"

#include "nvrecord_ble.h"
#include "app_anc.h"
#include "app_anc_assist.h"
#include "anc_assist_mic.h"
#include "anc_assist_resample.h"
#if defined(VOICE_ASSIST_FF_FIR_LMS)
#include "app_voice_assist_fir_lms.h"
#endif

#include "rwble_config.h"
#include "bes_aob_api.h"

#include "app_bt_sync.h"

#ifdef GAF_CODEC_CROSS_CORE
#include "app_dsp_m55.h"
#include "mcu_dsp_m55_app.h"
#include "gaf_codec_cc_common.h"
#include "gaf_codec_cc_bth.h"
#endif

#ifdef CODEC_DAC_MULTI_VOLUME_TABLE
#include "hal_codec.h"
#endif

#ifdef SPEECH_BONE_SENSOR
#include "speech_bone_sensor.h"
#define SPEECH_BS_SINGLE_CHANNEL    (SPEECH_BS_CHANNEL_Z)
static bool g_capture_vpu_enabled = false;
#ifdef GAF_ENCODER_CROSS_CORE_USE_M55
static int32_t *g_capture_combo_vpu_pcm_buf = NULL;
#endif
#endif

#ifdef AOB_LATENCY_TEST_MODE
#include "app_ble_usb_latency_test.h"
#endif

#ifdef GAF_DSP
#include "dsp_loader.h"
#endif

#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
#include "audio_prompt_sbc.h"
#endif

#ifdef DYNAMIC_SET_PB_TIME
#include "gaf_media_dync_buffer.h"
#endif

// #define GAF_STREAM_PLAYBACK_AUDIO_DUMP      (16)
#define STREAM_PROCESS_ENABLE

/*********************external function declaration*************************/

/************************private type defination****************************/
#ifndef BT_AUDIO_CACHE_2_UNCACHE
#define BT_AUDIO_CACHE_2_UNCACHE(addr) \
    ((unsigned char *)((unsigned int)addr & ~(0x04000000)))
#endif

#define APP_BAP_MAX_ASCS_NB_ASE_CFG               (APP_BAP_MAX_ASCS_NB_ASE_CHAR * BLE_CONNECTION_MAX)
#define APP_BAP_MAX_RECORD_CNT                    (APP_BAP_MAX_ASCS_NB_ASE_CFG + 1)

/**********************private function declaration*************************/

/************************private variable defination************************/
#ifdef AOB_UC_TEST
uint8_t buds_freq;
#endif
/****************************GAF CUSTOM*****************************/
static const GAF_STREAM_COMMON_CUSTOM_DATA_HANDLER_FUNC_T *gaf_uc_srv_custom_data_callback = NULL;

#ifdef HAL_CMU_FREQ_15M
static enum APP_SYSFREQ_FREQ_T gaf_decoder_flexible_bth_base_freq  = APP_SYSFREQ_30M;
#else
static enum APP_SYSFREQ_FREQ_T gaf_decoder_flexible_bth_base_freq  = APP_SYSFREQ_52M;
#endif
static GAF_AUDIO_STREAM_ENV_T gaf_audio_stream_env[GAF_MAXIMUM_CONNECTION_COUNT];
static GAF_MEDIA_DWELLING_INFO_T gaf_cis_media_dwelling_info[GAF_MAXIMUM_CONNECTION_COUNT];
static uint16_t is_gaf_audio_startedStreamTypes[APP_BAP_MAX_RECORD_CNT];
static uint32_t POSSIBLY_UNUSED g_capture_ch_num = 4;
static uint32_t POSSIBLY_UNUSED g_capture_frame_len = 16 * 10;
static uint32_t POSSIBLY_UNUSED g_capture_sample_rate = 16000;
static uint32_t POSSIBLY_UNUSED g_capture_sample_bytes = sizeof(int32_t);
static GAF_STREAM_PLAYBACK_STATUS_E gaf_stream_playback_status = GAF_STREAM_PLAYBACK_STATUS_IDLE;

#ifdef ANC_ASSIST_LE_AUDIO_SUPPORT
#define RESAMPLE_HEAP_BUFF_SIZE         (1024 * 24)
#define ANC_ASSIST_TEMP_MIC_CH          (0) /* 0: MIC1_MAIN, 1: MIC2_SUB */
static uint8_t *g_anc_assist_interval_buf = NULL;
static uint8_t *g_anc_assist_resample_buf = NULL;
static uint32_t g_anc_assist_resample_buf_used = 0;
#endif

#ifdef GAF_STREAM_PLAYBACK_AUDIO_DUMP
static uint32_t g_stream_pcm_dump_frame_len = 0;
static uint32_t g_stream_pcm_dump_channel_num = 0;
static uint32_t g_stream_pcm_dump_sample_bytes = 0;
#endif

#ifdef BINAURAL_RECORD_ADC_HIGH_SAMPLE_RATE
static uint32_t g_bin_record_sample_rate_div = BINAURAL_RECORD_ADC_HIGH_SAMPLE_RATE / 48000;
uint32_t gaf_stream_get_real_capture_sample_rate(void)
{
    return g_capture_sample_rate;
}
#endif

osMutexDef(gaf_playback_status_mutex_0);
#if defined (BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT)
osMutexDef(gaf_playback_status_mutex_1);
#endif

GAF_AUDIO_STREAM_ENV_T* pLocalStreamEnvPtr;
#ifdef GAF_CODEC_CROSS_CORE
osMutexDef(gaf_m55_encoder_buffer_mutex);
#else
osMutexDef(gaf_encoder_buffer_mutex);
#endif

/*
 * GAF_AUDIO_STREAM_CONTEXT_TYPE_MIN           = 0
 * GAF_AUDIO_STREAM_CONTEXT_TYPE_MEDIA         = GAF_AUDIO_STREAM_CONTEXT_TYPE_MIN,
 * GAF_AUDIO_STREAM_CONTEXT_TYPE_CALL,
 * GAF_AUDIO_STREAM_CONTEXT_TYPE_AI,
 * GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE,
*/
const static GAF_MEDIA_STREAM_TYPE_OPERATION_RULE_T gaf_cis_stream_type_op_rule[GAF_AUDIO_CONTEXT_NUM_MAX] =
{
    { GAF_AUDIO_STREAM_TYPE_PLAYBACK, GAF_AUDIO_TRIGGER_BY_PLAYBACK_STREAM, 1, 0},
#ifdef AOB_MOBILE_ENABLED
    { GAF_AUDIO_STREAM_TYPE_PLAYBACK|GAF_AUDIO_STREAM_TYPE_CAPTURE, GAF_AUDIO_TRIGGER_BY_PLAYBACK_STREAM, 1, 1},
    { GAF_AUDIO_STREAM_TYPE_CAPTURE, GAF_AUDIO_TRIGGER_BY_CAPTURE_STREAM, 0, 2},
#endif
    { GAF_AUDIO_STREAM_TYPE_FLEXIBLE, GAF_AUDIO_TRIGGER_BY_ISOLATE_STREAM, 1, 1},
};

#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
static uint32_t ble_audio_playback_sample_rate = AUD_SAMPRATE_48000;
static uint8_t* promptTmpSourcePcmDataBuf = NULL;
static uint8_t* promptTmpTargetPcmDataBuf = NULL;
static uint8_t* promptPcmDataBuf = NULL;
static uint8_t* promptResamplerBuf = NULL;
#endif

/****************************function defination****************************/
static void gaf_audio_add_ase_into_playback_ase_list(uint8_t con_lid, uint8_t ase_lid)
{
    LOG_I("set con_lid %d playback ase lid %d", con_lid, ase_lid);

    uint8_t idx = 0;

    if (GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT ==
        gaf_stream_common_get_ase_idx_in_ase_lid_list(
            &gaf_cis_media_dwelling_info[con_lid].playback_ase_id[0], ase_lid))
    {
        idx = gaf_stream_common_get_valid_idx_in_ase_lid_list(
                &gaf_cis_media_dwelling_info[con_lid].playback_ase_id[0]);
        if (idx == GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT)
        {
            LOG_E("no more space for adding playback ase lid: %d into list!!!", ase_lid);
        }
        else
        {
            gaf_cis_media_dwelling_info[con_lid].playback_ase_id[idx] = ase_lid;
        }
    }
    else
    {
        LOG_E("ase_lid: %d already in playback ase list!!!", ase_lid);
    }
}

static void gaf_audio_remove_ase_from_playback_ase_list(uint8_t con_lid, uint8_t ase_lid)
{
    LOG_I("remove con_lid %d playback ase lid %d", con_lid, ase_lid);

    uint8_t idx = gaf_stream_common_get_ase_idx_in_ase_lid_list(
            &gaf_cis_media_dwelling_info[con_lid].playback_ase_id[0], ase_lid);
    if (idx == GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT)
    {
        LOG_E("ase lid: %d is not in the list!!!", ase_lid);
        return;
    }

    gaf_cis_media_dwelling_info[con_lid].playback_ase_id[idx] = GAF_INVALID_ASE_INDEX;
}

static void gaf_audio_add_ase_into_capture_ase_list(uint8_t con_lid, uint8_t ase_lid)
{
    LOG_I("set con_lid %d capture ase lid %d", con_lid, ase_lid);

    uint8_t idx = 0;

    if (GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT ==
        gaf_stream_common_get_ase_idx_in_ase_lid_list(
            &gaf_cis_media_dwelling_info[con_lid].capture_ase_id[0], ase_lid))
    {
        idx = gaf_stream_common_get_valid_idx_in_ase_lid_list(
                &gaf_cis_media_dwelling_info[con_lid].capture_ase_id[0]);
        if (idx == GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT)
        {
            LOG_E("no more space for adding capture ase lid: %d into list!!!", ase_lid);
        }
        else
        {
            gaf_cis_media_dwelling_info[con_lid].capture_ase_id[idx] = ase_lid;
        }
    }
    else
    {
        LOG_E("ase_lid: %d already in capture ase list!!!", ase_lid);
    }
}

static void gaf_audio_remove_ase_from_capture_ase_list(uint8_t con_lid, uint8_t ase_lid)
{
    LOG_I("remove con_lid %d capture ase lid %d", con_lid, ase_lid);

    uint8_t idx = gaf_stream_common_get_ase_idx_in_ase_lid_list(
            &gaf_cis_media_dwelling_info[con_lid].capture_ase_id[0], ase_lid);

    if (idx == GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT)
    {
        LOG_E("ase lid: %d is not in the list!!!", ase_lid);
        return;
    }

    gaf_cis_media_dwelling_info[con_lid].capture_ase_id[idx] = GAF_INVALID_ASE_INDEX;
}

static uint8_t* gaf_audio_get_playback_ase_index_list(uint8_t con_lid)
{
    LOG_D("get playback con_lid %d", con_lid);
    return &gaf_cis_media_dwelling_info[con_lid].playback_ase_id[0];
}

static uint8_t* gaf_audio_get_capture_ase_index_list(uint8_t con_lid)
{
    LOG_D("get capture con_lid %d", con_lid);
    return &gaf_cis_media_dwelling_info[con_lid].capture_ase_id[0];
}

static void gaf_audio_clear_playback_ase_index_list(uint8_t con_lid)
{
    LOG_D("clear playback ase list con_lid %d", con_lid);
    memset_s(&gaf_cis_media_dwelling_info[con_lid].playback_ase_id[0],
            GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT*sizeof(uint8_t), GAF_INVALID_ASE_INDEX,
            GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT*sizeof(uint8_t));
}

static void gaf_audio_clear_capture_ase_index_list(uint8_t con_lid)
{
    LOG_D("clear playback ase list con_lid %d", con_lid);
    memset_s(&gaf_cis_media_dwelling_info[con_lid].capture_ase_id[0],
            GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT*sizeof(uint8_t), GAF_INVALID_ASE_INDEX,
            GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT*sizeof(uint8_t));
}

static void gaf_audio_dwelling_info_list_init(void)
{
    memset_s((void *)&gaf_cis_media_dwelling_info, sizeof(gaf_cis_media_dwelling_info),
             0, sizeof(gaf_cis_media_dwelling_info));
    memset_s(is_gaf_audio_startedStreamTypes, sizeof(is_gaf_audio_startedStreamTypes),
             0, sizeof(is_gaf_audio_startedStreamTypes));
}

static GAF_AUDIO_STREAM_ENV_T* gaf_audio_get_stream_from_channel_index(uint8_t channel)
{
    for (uint32_t con_lid = 0; con_lid < GAF_MAXIMUM_CONNECTION_COUNT; con_lid++)
    {
        for (uint8_t i = 0; i < GAF_AUDIO_ASE_TOTAL_COUNT; i++)
        {
            if ((gaf_audio_stream_env[con_lid].stream_info.playbackInfo.aseChInfo[i].iso_channel_hdl == channel) ||
                (gaf_audio_stream_env[con_lid].stream_info.captureInfo.aseChInfo[i].iso_channel_hdl == channel))
            {
                return &gaf_audio_stream_env[con_lid];
            }
        }
    }

    return NULL;
}

#ifdef IS_BLE_AUDIO_DEBUG_INFO_COLLECTOR_ENABLED
GAF_AUDIO_STREAM_ENV_T* gaf_audio_get_stream_env_from_con_lid(uint8_t con_lid)
{
    for (uint32_t type = GAF_AUDIO_STREAM_CONTEXT_TYPE_MEDIA; type < GAF_AUDIO_CONTEXT_NUM_MAX; type++)
    {
        if (gaf_audio_stream_env[type].stream_info.con_lid == con_lid)
        {
            return &gaf_audio_stream_env[type];
        }
    }

    LOG_W("(d%d)%s fail invaild", con_lid, __func__);
    return NULL;
}

static gaf_stream_dma_irq_cb capture_dma_irq_info_record_handler = NULL;
void gaf_stream_capture_register_dma_irq_cb(gaf_stream_dma_irq_cb func)
{
    capture_dma_irq_info_record_handler = func;
}
#endif

void gaf_stream_pre_fill_handler(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, uint8_t pre_fill_seq)
{
    LOG_I("start of pre fill");
    app_sysfreq_req(APP_SYSFREQ_USER_STREAM_BOOST, APP_SYSFREQ_208M);
    af_pre_fill_handler(id, stream, pre_fill_seq);
    app_sysfreq_req(APP_SYSFREQ_USER_STREAM_BOOST, APP_SYSFREQ_32K);
    LOG_I("end of pre fill");
}

static void gaf_stream_configure_playback_trigger(GAF_AUDIO_STREAM_ENV_T* pStreamEnv, uint8_t list_idx)
{
    gaf_stream_buff_list_t *list = &pStreamEnv->stream_context.playback_buff_list[list_idx].buff_list;
    gaf_media_data_t *decoder_frame_read = NULL;
    uint32_t current_bt_time = 0;
    uint32_t trigger_bt_time = 0;

    do
    {
        list->node = gaf_list_begin(list);
        if (NULL == list->node)
        {
            break;
        }

        decoder_frame_read = (gaf_media_data_t *)gaf_list_node(list);

        if (GAF_ISO_PKT_STATUS_VALID == decoder_frame_read->pkt_status)
        {
            current_bt_time = gaf_media_sync_get_curr_time();
            LOG_I("%s time_stamp %u current us %u seq 0x%x", __func__,
                decoder_frame_read->time_stamp, current_bt_time, decoder_frame_read->pkt_seq_nb);

            // time-stamp + present deay - one dma empty run
            trigger_bt_time = decoder_frame_read->time_stamp +
                (uint32_t)(pStreamEnv->stream_info.playbackInfo.presDelayUs) -
                pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs;

            LOG_I("calculated trigger ticks %u, presDelay %u", trigger_bt_time, pStreamEnv->stream_info.playbackInfo.presDelayUs);

            int32_t bt_time_diff = 0;
            bt_time_diff = GAF_AUDIO_CLK_32_BIT_DIFF(current_bt_time, trigger_bt_time);
            if ((bt_time_diff > GAF_MARGIN_BETWEEN_TRIGGER_TIME_AND_CURRENT_TIME_US) &&
                ((current_bt_time < trigger_bt_time) || // normal case
                (bt_time_diff > 0xFF00)))    // roll back case
            {
                pStreamEnv->stream_context.lastestPlaybackSeqNum[list_idx] = decoder_frame_read->pkt_seq_nb - 1;

                gaf_stream_set_prefill_status(true);
#ifdef ADVANCE_FILL_ENABLED
                pStreamEnv->stream_context.lastestPlaybackSeqNum[list_idx]++;
                gaf_stream_pre_fill_handler(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK,PP_PANG);
#endif
                gaf_stream_set_prefill_status(false);

                LOG_I("Starting playback[%d] seq num 0x%x", list_idx,
                      pStreamEnv->stream_context.lastestPlaybackSeqNum[list_idx] + 1);
                /// We just want to set lastestPlaybackSeqNum but not to reset trigger time
                if (pStreamEnv->stream_context.playback_stream_state < GAF_PLAYBACK_STREAM_START_TRIGGERING)
                {
                    gaf_stream_common_set_playback_trigger_time(pStreamEnv, trigger_bt_time);
                }
                break;
            }
            else
            {
                LOG_I("time_stamp pass");
                gaf_list_remove(list, decoder_frame_read);
            }
        }
    } while(1);
}

POSSIBLY_UNUSED static void gaf_media_stop_all_iso_dp_rx(GAF_AUDIO_STREAM_ENV_T* pStreamEnv)
{
    for (uint8_t i = 0; i < GAF_AUDIO_ASE_TOTAL_COUNT; i++)
    {
        if (GAF_AUDIO_INVALID_ISO_CHANNEL != pStreamEnv->stream_info.playbackInfo.aseChInfo[i].iso_channel_hdl)
        {
            bes_ble_bap_dp_rx_iso_stop(pStreamEnv->stream_info.playbackInfo.aseChInfo[i].cis_hdl);
        }
    }
}

POSSIBLY_UNUSED static void gaf_media_stop_all_iso_dp_tx(GAF_AUDIO_STREAM_ENV_T* pStreamEnv)
{
    for (uint8_t i = 0; i < GAF_AUDIO_ASE_TOTAL_COUNT; i++)
    {
        if (GAF_AUDIO_INVALID_ISO_CHANNEL != pStreamEnv->stream_info.captureInfo.aseChInfo[i].iso_channel_hdl)
        {
            bes_ble_bap_dp_tx_iso_stop(pStreamEnv->stream_info.captureInfo.aseChInfo[i].cis_hdl);
        }
    }
}

static void gaf_stream_start_trigger_playback(
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv, gaf_media_data_t* decoder_frame_info, uint8_t ase_chan_lid)
{
    if (((GAF_ISO_PKT_STATUS_VALID == decoder_frame_info->pkt_status) &&
        (decoder_frame_info->data_len > 0)) &&
        (pStreamEnv->stream_context.playback_stream_state < GAF_PLAYBACK_STREAM_START_TRIGGERING))
    {
        if (((GAF_AUDIO_TRIGGER_BY_PLAYBACK_STREAM ==
              gaf_cis_stream_type_op_rule[pStreamEnv->stream_info.contextType].trigger_stream_type) ||
             (GAF_AUDIO_TRIGGER_BY_ISOLATE_STREAM ==
              gaf_cis_stream_type_op_rule[pStreamEnv->stream_info.contextType].trigger_stream_type))
            && (GAF_PLAYBACK_STREAM_INITIALIZED == pStreamEnv->stream_context.playback_stream_state))
        {
            gaf_stream_configure_playback_trigger(pStreamEnv, ase_chan_lid);
        }
        else
        {
            LOG_W("[ISO SDU RECV] current stream context type = %d, state = %d",
                        pStreamEnv->stream_info.contextType,
                        pStreamEnv->stream_context.playback_stream_state);
        }
    }
#if defined (BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT)
    /// If we'd like to recv multi chan, set seq trigger but do not set dma trigger again for this channel
    else if ((pStreamEnv->stream_context.playback_stream_state == GAF_PLAYBACK_STREAM_START_TRIGGERING) &&
             ((GAF_ISO_PKT_STATUS_VALID == decoder_frame_info->pkt_status) && (decoder_frame_info->data_len > 0)) &&
             (pStreamEnv->stream_context.lastestPlaybackSeqNum[ase_chan_lid] == GAF_AUDIO_INVALID_SEQ_NUMBER))
    {
        gaf_stream_configure_playback_trigger(pStreamEnv, ase_chan_lid);
    }
#endif
}

bool gaf_stream_store_packet(uint16_t conhdl, gaf_media_data_t *pkt)
{
     // map to gaf stream context
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = NULL;
    uint8_t channel = BLE_ISOHDL_TO_ACTID(conhdl);
    pStreamEnv = gaf_audio_get_stream_from_channel_index(channel);

    uint8_t ase_chan_lid = GAF_AUDIO_ASE_TOTAL_COUNT;
    gaf_media_data_t decoder_frame_info;
    gaf_media_data_t* storedFramePointer = NULL;

    /// Check for stream env info for app unsetup ASE
    if (!pStreamEnv)
    {
        return false;
    }

    if (pStreamEnv->stream_context.playback_stream_state == GAF_PLAYBACK_STREAM_IDLE)
    {
        return false;
    }

#ifdef GAF_CODEC_CROSS_CORE
    pLocalStreamEnvPtr = pStreamEnv;
#endif

#if defined(BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT)
    ase_chan_lid = gaf_media_common_get_ase_chan_lid_from_iso_channel(pStreamEnv, BES_BLE_GAF_DIRECTION_SINK, channel);
    if (ase_chan_lid >= GAF_AUDIO_ASE_TOTAL_COUNT)
    {
        LOG_E("can not get ase chan lid by cis handle: 0x%x", conhdl);
        return false;
    }
#else
    ase_chan_lid = GAF_AUDIO_DFT_PLAYBACK_LIST_IDX;
#endif


#ifdef DYNAMIC_SET_PB_TIME
        if (gaf_dync_is_interference_start())
        {
            pkt->pkt_status = GAF_ISO_PKT_STATUS_INVALID;
        }
#endif
        if ((pStreamEnv->stream_context.playback_stream_state >= GAF_PLAYBACK_STREAM_START_TRIGGERING)
            || ((GAF_ISO_PKT_STATUS_VALID == pkt->pkt_status)
            && (pkt->data_len > 0)))
        {
        #ifdef GAF_CODEC_CROSS_CORE
            if (gaf_m55_deinit_status.playback_deinit == true)
        #else
            if (false)
        #endif
            {
               return false;
            }
            else
            {
                /// gaf custom, may be a watch point to check some pattern in encoded packet
                if (gaf_uc_srv_custom_data_callback->encoded_packet_recv_cb)
                {
                    gaf_uc_srv_custom_data_callback->encoded_packet_recv_cb(pkt);
                }
#ifdef DYNAMIC_SET_PB_TIME  // Adjust the timestamp dynamically
                gaf_dync_set_timestamp_by_variable_ft(pkt,&pStreamEnv->stream_info.playbackInfo.dync_buffer,
                    pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs);
#endif
                storedFramePointer = gaf_stream_common_store_received_packet(pStreamEnv, ase_chan_lid, pkt);
            }
        }

        if (storedFramePointer == NULL)
        {
           return false;
        }
        decoder_frame_info = *pkt;
        gaf_stream_start_trigger_playback(pStreamEnv, &decoder_frame_info, ase_chan_lid);

        return true;
}

static void gaf_stream_receive_data(uint16_t conhdl, GAF_ISO_PKT_STATUS_E pkt_status)
{
    /// map to gaf stream context
    bool result;
    gaf_media_data_t *p_decoder_frame = NULL;

    while ((p_decoder_frame = (gaf_media_data_t *)bes_ble_bap_dp_itf_get_rx_data(conhdl, NULL)))
    {
        result = gaf_stream_store_packet(conhdl, p_decoder_frame);
        if (!result)
        {
             gaf_stream_data_free(p_decoder_frame);
        }
    }
}

static int gaf_audio_flexible_playback_stream_start_handler(void* _pStreamEnv)
{
    LOG_I("%s start", __func__);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    uint8_t rt_vol = bes_ble_arc_vol_get_real_time_volume(pStreamEnv->stream_info.con_lid);
    uint8_t vol = bes_ble_arc_convert_le_vol_to_local_vol(rt_vol);

    if (GAF_PLAYBACK_STREAM_IDLE == pStreamEnv->stream_context.playback_stream_state)
    {
        pLocalStreamEnvPtr = pStreamEnv;

#ifdef BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT
        app_sysfreq_req(APP_SYSFREQ_USER_AOB_PLAYBACK, APP_SYSFREQ_208M);
#else
        // According to the bitrate and sample_rate to choose best frequecy to set
        int32_t sample_rate   = pStreamEnv->stream_info.playbackInfo.sample_rate;
        uint8_t channel       = pStreamEnv->stream_info.playbackInfo.num_channels;
        uint8_t frame_dms     = (uint8_t)(pStreamEnv->stream_info.playbackInfo.frame_ms * 10);
        uint16_t frame_size   = pStreamEnv->stream_info.playbackInfo.encoded_frame_size * channel;
        enum APP_SYSFREQ_FREQ_T gaf_call_base_freq = gaf_decoder_flexible_bth_base_freq;

#ifdef GAF_CODEC_CROSS_CORE
        gaf_decoder_flexible_bth_base_freq = gaf_audio_flexible_adjust_bth_freq(frame_size,
                                            channel, frame_dms, sample_rate, gaf_call_base_freq,
                                            is_playback_state, is_capture_state);
#else
        gaf_decoder_flexible_bth_base_freq = gaf_audio_flexible_decoder_adjust_bth_freq_no_m55(frame_size,
                                            channel, frame_dms, sample_rate, gaf_call_base_freq);
#endif

        app_sysfreq_req(APP_SYSFREQ_USER_AOB_PLAYBACK, gaf_decoder_flexible_bth_base_freq);
#endif

        pStreamEnv->func_list->stream_func_list.playback_init_stream_buf_func(pStreamEnv);
#ifdef GAF_CODEC_CROSS_CORE
        if (pStreamEnv->stream_context.playback_retrigger_onprocess)
        {
            gaf_stream_common_update_playback_stream_state(pStreamEnv, GAF_PLAYBACK_STREAM_INITIALIZED);
        }
        else
        {
            gaf_stream_common_update_playback_stream_state(pStreamEnv, GAF_PLAYBACK_STREAM_WAITING_M55_INIT);
        }
#endif
        pStreamEnv->func_list->decoder_func_list->decoder_init_func(pStreamEnv,
        gaf_cis_stream_type_op_rule[pStreamEnv->stream_info.contextType].playback_ase_count);

        struct AF_STREAM_CONFIG_T stream_cfg;
        af_set_priority(AF_USER_AOB_PLAYBACK, osPriorityHigh);

#ifdef CODEC_DAC_MULTI_VOLUME_TABLE
        if (gaf_stream_process_context_is_call(pStreamEnv->stream_info.bap_contextType)) {
            hal_codec_set_dac_volume_table(codec_dac_hfp_vol, ARRAY_SIZE(codec_dac_hfp_vol));
        } else {
            hal_codec_set_dac_volume_table(codec_dac_a2dp_vol, ARRAY_SIZE(codec_dac_a2dp_vol));
        }
#endif
        // playback stream
        memset_s((void *)&stream_cfg, sizeof(struct AF_STREAM_CONFIG_T),
                0, sizeof(struct AF_STREAM_CONFIG_T));
        stream_cfg.bits         = (enum AUD_BITS_T)(pStreamEnv->stream_info.playbackInfo.bits_depth);
        stream_cfg.channel_num  = (enum AUD_CHANNEL_NUM_T)(pStreamEnv->stream_info.playbackInfo.num_channels);

        stream_cfg.io_path      = AUD_OUTPUT_PATH_SPEAKER;
#ifdef PSAP_SW_USE_DAC1
        stream_cfg.device       = AUD_STREAM_USE_INT_CODEC3;
#else
        stream_cfg.device       = AUD_STREAM_USE_INT_CODEC;
#endif
        stream_cfg.sample_rate  = (enum AUD_SAMPRATE_T)pStreamEnv->stream_info.playbackInfo.sample_rate;
#ifdef BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT
        if (stream_cfg.channel_num == AUD_CHANNEL_NUM_2)
        {
            stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)( AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1);
        }
#endif
        stream_cfg.vol          = vol;
        stream_cfg.data_size    = (uint32_t)(2 * pStreamEnv->stream_info.playbackInfo.dmaChunkSize);
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(pStreamEnv->stream_info.playbackInfo.dmaBufPtr);
        stream_cfg.handler = pStreamEnv->func_list->stream_func_list.playback_dma_irq_handler_func;

        af_codec_tune(AUD_STREAM_PLAYBACK, 0);
        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);

#ifdef STREAM_PROCESS_ENABLE
        gaf_stream_process_playback_open(pStreamEnv->stream_info.bap_contextType, &stream_cfg);
#endif

#if defined(SPA_AUDIO_ENABLE)
        //spatial_audio_app_init_if(sample_rate, sample_bits, hw_ch_num, frame_size, (void*)app_audio_mempool_get_buff, app_ibrt_if_get_ui_role(), ((ibrt_ctrl_t *)app_tws_ibrt_get_bt_ctrl_ctx())->audio_chnl_sel);
        spatial_audio_app_init_if(stream_cfg.sample_rate,stream_cfg.bits,stream_cfg.channel_num,stream_cfg.data_size/stream_cfg.channel_num/(stream_cfg.bits <= AUD_BITS_16 ? 2 : 4)/2,(void*)app_audio_mempool_get_buff, app_ibrt_if_get_ui_role(), ((ibrt_ctrl_t *)app_tws_ibrt_get_bt_ctrl_ctx())->audio_chnl_sel);
#endif

#ifdef GAF_STREAM_PLAYBACK_AUDIO_DUMP
        if (stream_cfg.bits == 16) {
            g_stream_pcm_dump_sample_bytes = sizeof(int16_t);
        } else {
            g_stream_pcm_dump_sample_bytes = sizeof(int32_t);
        }
        g_stream_pcm_dump_channel_num = stream_cfg.channel_num;
        g_stream_pcm_dump_frame_len = stream_cfg.data_size / 2 / g_stream_pcm_dump_sample_bytes / g_stream_pcm_dump_channel_num;
#if GAF_STREAM_PLAYBACK_AUDIO_DUMP == 16
        audio_dump_init(g_stream_pcm_dump_frame_len, sizeof(int16_t), 1);
#else
        audio_dump_init(g_stream_pcm_dump_frame_len, g_stream_pcm_dump_sample_bytes, 1);
#endif
#endif

        pStreamEnv->stream_context.playbackTriggerChannel = app_bt_sync_get_available_trigger_channel(APP_BT_SYNC_OP_MAX, APP_BT_SYNC_POLICY_DEFAULT);

        gaf_media_prepare_playback_trigger(pStreamEnv->stream_context.playbackTriggerChannel);

        gaf_media_pid_init(&(pStreamEnv->stream_context.playback_pid_env));

        bes_ble_bap_dp_itf_data_come_callback_register((void *)gaf_stream_receive_data);

#ifndef GAF_CODEC_CROSS_CORE
        gaf_stream_common_update_playback_stream_state(pStreamEnv, GAF_PLAYBACK_STREAM_INITIALIZED);
#endif

        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);

#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
        audio_prompt_stop_playing();

        audio_prompt_buffer_config(MIX_WITH_BLE_AUDIO_FLEXIBLE_STREAMING,
                                   stream_cfg.channel_num,
                                   stream_cfg.bits,
                                   promptTmpSourcePcmDataBuf,
                                   promptTmpTargetPcmDataBuf,
                                   promptPcmDataBuf,
                                   AUDIO_PROMPT_PCM_BUFFER_SIZE,
                                   promptResamplerBuf,
                                   AUDIO_PROMPT_BUF_SIZE_FOR_RESAMPLER);
#endif

        LOG_D("%s end", __func__);

        return 0;
    }

    return -1;
}

#ifdef ANC_ASSIST_LE_AUDIO_SUPPORT
static uint32_t gaf_audio_anc_resample_heap_free_size()
{
    return RESAMPLE_HEAP_BUFF_SIZE - g_anc_assist_resample_buf_used;
}

static void gaf_audio_anc_resample_heap_init()
{
    g_anc_assist_resample_buf_used = 0;
    memset_s(g_anc_assist_resample_buf, RESAMPLE_HEAP_BUFF_SIZE, 0, RESAMPLE_HEAP_BUFF_SIZE);
}

static int gaf_audio_anc_resample_get_heap_buff(uint8_t **buff, uint32_t size)
{
    uint32_t buff_size_free;

    buff_size_free = gaf_audio_anc_resample_heap_free_size();

    if (size % 4){
        size = size + (4 - size % 4);
    }
    ASSERT (size <= buff_size_free, "anc resample pool in shortage! To allocate size %d but free size %d.",
        size, buff_size_free);
    *buff = g_anc_assist_resample_buf + g_anc_assist_resample_buf_used;
    g_anc_assist_resample_buf_used += size;
    LOG_I("[%s] ptr=%p size=%u free=%u", __func__, *buff, size, buff_size_free);
    return buff_size_free;
}

void *gaf_anc_assist_resample_malloc(size_t size)
{
    uint8_t *pBuf = NULL;

    if (size % 4) {
        size = size + (4 - size % 4);
    }

    gaf_audio_anc_resample_get_heap_buff(&pBuf, size);

    return (void *)pBuf;
}

void *gaf_anc_assist_resample_calloc(size_t nitems, size_t len)
{
    uint8_t *pBuf = NULL;
    int size;

    size = nitems * len;

    if (size % 4) {
        size = size + (4 - size % 4);
    }

    gaf_audio_anc_resample_get_heap_buff(&pBuf, size);

    memset((uint8_t*)pBuf, 0x0, size);

    return (void *)pBuf;
}

void gaf_anc_assist_resample_free_ext_buf(void *mem_ptr)
{

}

static POSSIBLY_UNUSED custom_allocator gaf_resample_allocator = {
    .malloc = gaf_anc_assist_resample_malloc,
    .calloc = gaf_anc_assist_resample_calloc,
    .free = gaf_anc_assist_resample_free_ext_buf,
};
#endif

static void gaf_audio_flexible_common_buf_init(GAF_AUDIO_STREAM_ENV_T * pStreamEnv, GAF_STREAM_TYPE_E stream_type)
{
    uint32_t audioCacheHeapSize = 0;
    uint8_t* heapBufStartAddr = NULL;

    if ((GAF_PLAYBACK_STREAM_IDLE == pStreamEnv->stream_context.playback_stream_state) &&
        (GAF_CAPTURE_STREAM_IDLE == pStreamEnv->stream_context.capture_stream_state))
    {
        pStreamEnv->stream_info.playbackInfo.storedDmaBufPtr = NULL;
        pStreamEnv->stream_info.captureInfo.storedDmaBufPtr = NULL;
#if defined(SPEECH_BONE_SENSOR) && defined(GAF_ENCODER_CROSS_CORE_USE_M55)
        g_capture_combo_vpu_pcm_buf = NULL;
#endif
#ifdef ANC_ASSIST_LE_AUDIO_SUPPORT
        g_anc_assist_interval_buf = NULL;
        g_anc_assist_resample_buf = NULL;
#endif

#ifndef AOB_CODEC_CP
        lc3_alloc_data_free();
#endif
        if (gaf_stream_process_context_is_call(pStreamEnv->stream_info.bap_contextType)) {
            app_overlay_select(APP_OVERLAY_SPEECH_ALGO);
        }

        if (gaf_stream_process_context_is_binaural_record(pStreamEnv->stream_info.bap_contextType))
        {
            app_audio_mempool_init();
        }
        else
        {
            app_audio_mempool_init_with_specific_size(app_audio_mempool_size());
        }

        if (GAF_STREAM_PLAYBACK == stream_type)
        {
            audioCacheHeapSize = 2*(pStreamEnv->stream_info.playbackInfo.maxCachedEncodedAudioPacketCount*
                pStreamEnv->stream_info.playbackInfo.maxEncodedAudioPacketSize);
        }
        else
        {
            audioCacheHeapSize = 2*(pStreamEnv->stream_info.captureInfo.maxCachedEncodedAudioPacketCount*
                pStreamEnv->stream_info.captureInfo.maxEncodedAudioPacketSize);
        }

        app_audio_mempool_get_buff(&heapBufStartAddr, audioCacheHeapSize);
        gaf_stream_heap_init(heapBufStartAddr, audioCacheHeapSize);

#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
        app_audio_mempool_get_buff(&promptTmpSourcePcmDataBuf, AUDIO_PROMPT_SOURCE_PCM_BUFFER_SIZE);
        app_audio_mempool_get_buff(&promptTmpTargetPcmDataBuf, AUDIO_PROMPT_TARGET_PCM_BUFFER_SIZE);
        app_audio_mempool_get_buff(&promptPcmDataBuf, AUDIO_PROMPT_PCM_BUFFER_SIZE);
        app_audio_mempool_get_buff(&promptResamplerBuf, AUDIO_PROMPT_BUF_SIZE_FOR_RESAMPLER);
#endif

    }

    if (GAF_STREAM_PLAYBACK == stream_type)
    {
#ifdef STREAM_PROCESS_ENABLE
        uint32_t stream_process_buf_size = gaf_stream_process_need_playback_buf_size(pStreamEnv->stream_info.bap_contextType);
        if (stream_process_buf_size != 0) {
            gaf_stream_process_set_playback_buf((uint8_t *)app_audio_mempool_calloc(stream_process_buf_size, sizeof(int8_t)), stream_process_buf_size);
        }
#endif
        if (NULL == pStreamEnv->stream_info.playbackInfo.storedDmaBufPtr)
        {
            app_audio_mempool_get_buff(&(pStreamEnv->stream_info.playbackInfo.storedDmaBufPtr),
                            pStreamEnv->stream_info.playbackInfo.dmaChunkSize*2);
        }
        pStreamEnv->stream_info.playbackInfo.dmaBufPtr = pStreamEnv->stream_info.playbackInfo.storedDmaBufPtr;
    }
    else
    {
#ifdef STREAM_PROCESS_ENABLE
        uint32_t stream_process_buf_size = gaf_stream_process_need_capture_buf_size(pStreamEnv->stream_info.bap_contextType);
        if (stream_process_buf_size != 0) {
            gaf_stream_process_set_capture_buf((uint8_t *)app_audio_mempool_calloc(stream_process_buf_size, sizeof(int8_t)), stream_process_buf_size);
        }
#endif

        GAF_AUDIO_STREAM_COMMON_INFO_T *pCommonInfo = &pStreamEnv->stream_info.captureInfo;
        g_capture_frame_len = (uint32_t)(g_capture_sample_rate * pCommonInfo->frame_ms / 1000);

        if (gaf_stream_process_context_is_binaural_record(pStreamEnv->stream_info.bap_contextType)) {
            g_capture_ch_num = 1;
#ifdef BINAURAL_RECORD_ADC_HIGH_SAMPLE_RATE
            ASSERT((g_capture_sample_rate == 32000) || (g_capture_sample_rate == 48000),"[%s] Don't support this sample rate: %d", __func__, g_capture_sample_rate);
            g_bin_record_sample_rate_div = BINAURAL_RECORD_ADC_HIGH_SAMPLE_RATE / g_capture_sample_rate;
            pCommonInfo->dmaChunkSize *= g_bin_record_sample_rate_div;
#endif
        }
        else
        {
            g_capture_ch_num = hal_codec_get_input_path_chan_num(AUD_INPUT_PATH_MAINMIC);
            pCommonInfo->dmaChunkSize *= g_capture_ch_num;
#if defined(SPEECH_BONE_SENSOR) && defined(GAF_ENCODER_CROSS_CORE_USE_M55)
            if ((g_capture_combo_vpu_pcm_buf == NULL) && g_capture_vpu_enabled)
            {
                g_capture_combo_vpu_pcm_buf = (int32_t *)app_audio_mempool_calloc(g_capture_frame_len * (g_capture_ch_num + 1), sizeof(int32_t));
            }
#endif
        }
#ifdef ANC_ASSIST_LE_AUDIO_SUPPORT
        if (g_anc_assist_interval_buf == NULL)
        {
            app_audio_mempool_get_buff(&g_anc_assist_interval_buf, g_capture_frame_len * g_capture_ch_num * sizeof(int32_t));
            if (g_capture_sample_rate != AUD_SAMPRATE_16000)
            {
                app_audio_mempool_get_buff(&g_anc_assist_resample_buf, RESAMPLE_HEAP_BUFF_SIZE);
            }
        }
#endif


        if (NULL == pCommonInfo->storedDmaBufPtr)
        {
            app_audio_mempool_get_buff(&(pCommonInfo->storedDmaBufPtr), pCommonInfo->dmaChunkSize*2);
        }
        pCommonInfo->dmaBufPtr = pCommonInfo->storedDmaBufPtr;
    }
}

static void gaf_audio_flexible_playback_buf_init(void* _pStreamEnv)
{
    LOG_D("%s start", __func__);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    gaf_audio_flexible_common_buf_init(pStreamEnv, GAF_STREAM_PLAYBACK);

    gaf_list_new(&pStreamEnv->stream_context.playback_buff_list[0].buff_list,
                    (osMutex(gaf_playback_status_mutex_0)),
                    (list_free_cb)gaf_stream_data_free,
                    (list_mempool_zmalloc)gaf_stream_heap_cmalloc,
                    (list_mempool_free)gaf_stream_heap_free);
#if defined (BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT)
    gaf_list_new(&pStreamEnv->stream_context.playback_buff_list[1].buff_list,
                    (osMutex(gaf_playback_status_mutex_1)),
                    (list_free_cb)gaf_stream_data_free,
                    (list_mempool_zmalloc)gaf_stream_heap_cmalloc,
                    (list_mempool_free)gaf_stream_heap_free);
#endif

    pStreamEnv->func_list->decoder_func_list->decoder_init_buf_func(pStreamEnv,
        gaf_cis_stream_type_op_rule[pStreamEnv->stream_info.contextType].playback_ase_count);

    LOG_D("%s end", __func__);
}

static int gaf_playback_status_mutex_init(void **mutex)
{
    if (mutex[0] == NULL){
        mutex[0] = osMutexCreate((osMutex(gaf_playback_status_mutex_0)));
    }
#if defined (BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT)
    if (mutex[1] == NULL){
        mutex[1] = osMutexCreate((osMutex(gaf_playback_status_mutex_1)));
    }
#endif
    return 0;
}

static int gaf_stream_set_playback_status(uint8_t con_lid,
                                          GAF_STREAM_PLAYBACK_STATUS_E playback_status)
{
    void *mutex = gaf_audio_stream_env[con_lid].stream_info.gaf_playback_status_mutex;

    gaf_playback_status_mutex_lock(mutex);
    gaf_stream_playback_status = playback_status;
    gaf_playback_status_mutex_unlock(mutex);

    return 0;
}

GAF_STREAM_PLAYBACK_STATUS_E gaf_stream_get_playback_status(uint8_t con_lid)
{
    void *mutex = gaf_audio_stream_env[con_lid].stream_info.gaf_playback_status_mutex;
    GAF_STREAM_PLAYBACK_STATUS_E playback_status;

    gaf_playback_status_mutex_lock(mutex);
    playback_status = gaf_stream_playback_status;
    gaf_playback_status_mutex_unlock(mutex);

    return playback_status;
}

void gaf_stream_dump_dma_trigger_status()
{
    uint8_t context_id = 0;
    uint8_t len = 0;
    char state_str[100];
    for (context_id = 0; context_id < GAF_MAXIMUM_CONNECTION_COUNT; context_id++)
    {
        GAF_PLAYBACK_STREAM_STATE_E trigger_status = gaf_audio_stream_env[context_id].stream_context.playback_stream_state;
        len += sprintf(state_str + len, "context %d state %d ", context_id, trigger_status);
    }
    LOG_I("gaf tri: %s", state_str);
}

static int gaf_stream_playback_stop(uint8_t con_lid)
{
    LOG_D("%s",__func__);
    int cnt = 50;

    do {
        if (gaf_stream_get_playback_status(con_lid) == GAF_STREAM_PLAYBACK_STATUS_IDLE)
        {
            LOG_I("[STOP] PLAYBACK_STATUS_IDLE cnt:%d", cnt);
            break;
        }
        else
        {
            osDelay(1);
        }
    } while(--cnt > 0);

    return 0;
}

static int gaf_audio_flexible_playback_stream_stop_handler(void* _pStreamEnv)
{
    LOG_D("%s start", __func__);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    if (GAF_PLAYBACK_STREAM_IDLE != pStreamEnv->stream_context.playback_stream_state)
    {
        pLocalStreamEnvPtr = NULL;
        bes_ble_bap_dp_itf_data_come_callback_deregister();
        gaf_stream_common_update_playback_stream_state(pStreamEnv, GAF_PLAYBACK_STREAM_IDLE);
        gaf_stream_playback_stop(pStreamEnv->stream_info.con_lid);

#ifdef AF_STREAM_ID_0_PLAYBACK_FADEOUT
        af_stream_playback_fadeout(AUD_STREAM_ID_0, 10);
#endif

        uint8_t POSSIBLY_UNUSED adma_ch = HAL_DMA_CHAN_NONE;
        uint32_t dma_base;
        // sink
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

#ifdef STREAM_PROCESS_ENABLE
        gaf_stream_process_playback_close();
#endif

        pStreamEnv->func_list->decoder_func_list->decoder_deinit_func();

        gaf_stream_common_clr_trigger(pStreamEnv->stream_context.playbackTriggerChannel);

        af_codec_set_bt_sync_source(AUD_STREAM_PLAYBACK, 0);

        af_set_priority(AF_USER_AOB_PLAYBACK, osPriorityAboveNormal);
        pStreamEnv->func_list->stream_func_list.playback_deinit_stream_buf_func(pStreamEnv);

#ifdef GAF_ENCODER_CROSS_CORE_USE_M55
        app_overlay_unloadall();
#endif

#ifdef DYNAMIC_SET_PB_TIME
       gaf_dync_buffer_info_reset(&pStreamEnv->stream_info.playbackInfo.dync_buffer);
#endif

#ifdef ANC_ASSIST_LE_AUDIO_SUPPORT
        app_anc_assist_set_mode(ANC_ASSIST_MODE_STANDALONE);
#endif
        app_sysfreq_req(APP_SYSFREQ_USER_AOB_PLAYBACK, APP_SYSFREQ_32K);
    
        LOG_D("%s end", __func__);
        return 0;
    }

    return -1;
}

static bool gaf_stream_is_any_downstream_iso_created(uint8_t con_lid)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_audio_stream_env[con_lid];

    for (uint8_t i = 0; i < GAF_AUDIO_ASE_TOTAL_COUNT; i++)
    {
        if (GAF_AUDIO_INVALID_ISO_CHANNEL != pStreamEnv->stream_info.playbackInfo.aseChInfo[i].iso_channel_hdl)
        {
            return true;
        }
    }

    return false;
}

#if defined (BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT)
static bool gaf_stream_flexible_is_skip_prefill_handler(GAF_AUDIO_STREAM_ENV_T* pStreamEnv)
{
    uint8_t ase_idx = 0;
    /// Check is there need wait for second iso data to do prefill
    if (gaf_stream_get_prefill_status())
    {
        uint8_t set_trigger_times = 0;
        for (ase_idx = 0; ase_idx < (GAF_AUDIO_ASE_TOTAL_COUNT - 1); ase_idx++)
        {
            if (GAF_AUDIO_INVALID_ISO_CHANNEL == pStreamEnv->stream_info.playbackInfo.aseChInfo[ase_idx].iso_channel_hdl)
            {
                break;
            }

            if (pStreamEnv->stream_context.lastestPlaybackSeqNum[ase_idx] != GAF_AUDIO_INVALID_SEQ_NUMBER)
            {
                set_trigger_times++;
            }
        }
        /// If (ase idx > set_trigger_times) means two iso connected but just one channel has recved data
        if (ase_idx > set_trigger_times)
        {
            return true;
        }
    }

    return false;
}
#endif

static uint32_t _gaf_stream_flexible_playback_dma_irq_handler(uint8_t con_lid, uint8_t* ptrBuf, uint32_t length)
{
    LOG_D("%s buffer len = %d", __func__,length);
#ifdef AOB_LATENCY_TEST_MODE
    app_ble_usb_latency_playback_signal();
#endif
    GAF_AUDIO_STREAM_ENV_T *pStreamEnv = &gaf_audio_stream_env[con_lid];

    if(pStreamEnv == NULL)
    {
        LOG_D("%s pStreamEnv is NULL", __func__);
        return 0;
    }

    if (!gaf_stream_is_any_downstream_iso_created(pStreamEnv->stream_info.con_lid))
    {
        return length;
    }

#if defined (BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT)
    /// Check is here any neccesary to do prefill, maybe need wait for two channel ready
    if (gaf_stream_flexible_is_skip_prefill_handler(pStreamEnv))
    {
        return length;
    }
#endif

    uint32_t btclk; //hal slot -- 312.5us
    uint16_t btcnt; //hal  microsecond -- 0.5 us
    uint32_t dmaIrqHappeningTimeUs = 0;
    uint8_t adma_ch = HAL_DMA_CHAN_NONE;
    uint32_t dma_base;

    gaf_stream_set_playback_status(con_lid, GAF_STREAM_PLAYBACK_STATUS_BUSY);
    adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    if (adma_ch != HAL_DMA_CHAN_NONE)
    {
        bt_drv_reg_op_dma_tc_clkcnt_get_by_ch(&btclk, &btcnt, adma_ch&0xFF, dma_base);
        dmaIrqHappeningTimeUs = bt_syn_ble_bt_time_to_bts(btclk, btcnt);
    }

    // it's possible the multiple DMA irq triggered message accumulates,
    // so the acuiqred dmaIrqHappeningTimeUs is the last dma irq, which has
    // been handled. For this case, ignore this dma irq message handling
    if (!gaf_stream_get_prefill_status())
    {
        if ((!pStreamEnv) ||
            (GAF_PLAYBACK_STREAM_START_TRIGGERING > pStreamEnv->stream_context.playback_stream_state) ||
            ((GAF_PLAYBACK_STREAM_STREAMING_TRIGGERED == pStreamEnv->stream_context.playback_stream_state) &&
            (dmaIrqHappeningTimeUs == pStreamEnv->stream_context.lastPlaybackDmaIrqTimeUs)))
        {
            memset_s(ptrBuf, length, 0, length);
            gaf_stream_set_playback_status(con_lid, GAF_STREAM_PLAYBACK_STATUS_IDLE);
            return length;
        }

#if defined (BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT)
        gaf_stream_common_update_multi_channel_expect_seq_and_time(pStreamEnv, dmaIrqHappeningTimeUs);
#else
        gaf_stream_common_updated_expeceted_playback_seq_and_time(pStreamEnv, GAF_AUDIO_DFT_PLAYBACK_LIST_IDX, dmaIrqHappeningTimeUs);
#endif

        if (GAF_PLAYBACK_STREAM_STREAMING_TRIGGERED != pStreamEnv->stream_context.playback_stream_state)
        {
            gaf_stream_common_update_playback_stream_state(pStreamEnv,  GAF_PLAYBACK_STREAM_STREAMING_TRIGGERED);
            gaf_stream_common_clr_trigger(pStreamEnv->stream_context.playbackTriggerChannel);
            LOG_I("%s Update playback 0 seq to [0x%x]", __func__,
                pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX]);
#if defined (BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT)
            if (pStreamEnv->stream_context.lastestPlaybackSeqNum[1] != GAF_AUDIO_INVALID_SEQ_NUMBER)
            {
               LOG_I("%s Update playback 1 seq to [0x%x]", __func__, pStreamEnv->stream_context.lastestPlaybackSeqNum[1]);
            }
#endif
        }
    }

#ifdef GAF_DECODER_CROSS_CORE_USE_M55
    int ret = 0;
    bool isSuccessful = gaf_decoder_core_fetch_pcm_data(pStreamEnv,
        pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX], ptrBuf, length, dmaIrqHappeningTimeUs);
    if (!isSuccessful)
    {
        memset_s(ptrBuf, length, 0, length);
        ret = 1;
    }
#else
    int ret = 0;
    int32_t diff_bt_time = 0;
    gaf_media_data_t *decoder_frame_p = NULL;

#if defined (BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT)
    decoder_frame_p = gaf_stream_common_get_combined_packet_from_multi_channels(pStreamEnv, dmaIrqHappeningTimeUs);
#else
    decoder_frame_p = gaf_stream_common_get_packet(pStreamEnv, GAF_AUDIO_DFT_PLAYBACK_LIST_IDX, dmaIrqHappeningTimeUs);
#endif

    if (!gaf_stream_get_prefill_status())
    {
        diff_bt_time = GAF_AUDIO_CLK_32_BIT_DIFF(
            decoder_frame_p->time_stamp, dmaIrqHappeningTimeUs) - pStreamEnv->stream_info.playbackInfo.presDelayUs;
#ifdef ADVANCE_FILL_ENABLED
        diff_bt_time += pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs;
#endif
        gaf_media_pid_adjust(AUD_STREAM_PLAYBACK, &(pStreamEnv->stream_context.playback_pid_env), diff_bt_time);
    }

#ifdef DYNAMIC_SET_PB_TIME
    gaf_dync_monitor_recv_pkt_status(&pStreamEnv->stream_info.playbackInfo.dync_buffer, decoder_frame_p);
#endif
    ret = pStreamEnv->func_list->decoder_func_list->decoder_decode_frame_func
            (decoder_frame_p->isPLC, decoder_frame_p->data_len, decoder_frame_p->sdu_data, &pStreamEnv->stream_context.codec_alg_context[0], ptrBuf);

    LOG_I("decoded seq 0x%02x expected play time %u local time %u diff %d dec ret %d",
            decoder_frame_p->pkt_seq_nb,decoder_frame_p->time_stamp, dmaIrqHappeningTimeUs, diff_bt_time, ret);

    /// gaf custom, may be a watch point to check some pattern in decoded raw data
    if (gaf_uc_srv_custom_data_callback->decoded_raw_data_cb)
    {
        gaf_uc_srv_custom_data_callback->decoded_raw_data_cb(decoder_frame_p, ptrBuf, length);
    }

    gaf_stream_data_free(decoder_frame_p);
#endif

    if (LC3_API_OK == ret)
    {
#ifdef STREAM_PROCESS_ENABLE
        length = gaf_stream_process_playback_run(ptrBuf, length);
#endif
    }
    else
    {
        //LOG_E("[le][err]%dms,%p,%d->%p,%d",dmaIrqHappeningTimeUs/1000,decoder_frame_p->data,decoder_frame_p->data_len,ptrBuf,length);
        memset_s(ptrBuf, length, 0, length);
    }
    gaf_stream_set_playback_status(con_lid, GAF_STREAM_PLAYBACK_STATUS_IDLE);

#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
    audio_prompt_processing_handler(0, length, ptrBuf);
#endif

    LOG_D("%s end", __func__);

    return 0;
}

static uint32_t gaf_stream_flexible_playback_idx_0_dma_irq_handler(uint8_t* ptrBuf, uint32_t length)
{
    return _gaf_stream_flexible_playback_dma_irq_handler(0, ptrBuf, length);
}

static uint32_t gaf_stream_flexible_playback_idx_1_dma_irq_handler(uint8_t* ptrBuf, uint32_t length)
{
    return _gaf_stream_flexible_playback_dma_irq_handler(1, ptrBuf, length);
}

static uint32_t gaf_stream_flexible_playback_idx_2_dma_irq_handler(uint8_t* ptrBuf, uint32_t length)
{
    return _gaf_stream_flexible_playback_dma_irq_handler(2, ptrBuf, length);
}

/// TODO: maybe need more

static void gaf_audio_flexible_playback_buf_deinit(void* _pStreamEnv)
{
    LOG_D("%s start", __func__);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    pStreamEnv->stream_info.playbackInfo.dmaBufPtr = NULL;
    gaf_list_free(&pStreamEnv->stream_context.playback_buff_list[0].buff_list);
#if defined (BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT)
    gaf_list_free(&pStreamEnv->stream_context.playback_buff_list[1].buff_list);
#endif

    pStreamEnv->func_list->decoder_func_list->decoder_deinit_buf_func(pStreamEnv,
        gaf_cis_stream_type_op_rule[pStreamEnv->stream_info.contextType].playback_ase_count);
    LOG_I("[%s] syspool free size: %d/%d", __func__, syspool_free_size(), syspool_total_size());

    LOG_D("%s end", __func__);
}

static int gaf_audio_flexible_capture_stream_start_handler(void* _pStreamEnv)
{
    LOG_D("%s start", __func__);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    uint8_t vol = hal_codec_get_default_dac_volume_index();

    if (GAF_CAPTURE_STREAM_IDLE == pStreamEnv->stream_context.capture_stream_state)
    {
#ifdef __BIXBY
        app_bixby_on_suspend_handle();
#endif
        // According to the bitrate and sample_rate to choose best frequecy to set
        int32_t sample_rate   = pStreamEnv->stream_info.captureInfo.sample_rate;
        uint8_t channel       = pStreamEnv->stream_info.captureInfo.num_channels;
        uint8_t frame_dms     = (uint8_t)(pStreamEnv->stream_info.captureInfo.frame_ms * 10);
        uint16_t frame_size   = pStreamEnv->stream_info.captureInfo.encoded_frame_size * channel;
        enum APP_SYSFREQ_FREQ_T gaf_bth_call_base_freq = gaf_decoder_flexible_bth_base_freq;

        LOG_D("%s frame_size %d frame_dms %d channel %d", __func__,frame_size,frame_dms, channel);

#ifdef GAF_CODEC_CROSS_CORE
        gaf_decoder_flexible_bth_base_freq = gaf_audio_flexible_adjust_bth_freq(frame_size,
                                            channel, frame_dms, sample_rate, gaf_bth_call_base_freq,
                                            is_playback_state, is_capture_state);
        // use higher cpu frequency for binaural recording case to avoid missing feeding
        // data to btc
        if (gaf_stream_process_context_is_binaural_record(pStreamEnv->stream_info.bap_contextType))
        {
#ifdef BINAURAL_RECORD_PROCESS
            gaf_decoder_flexible_bth_base_freq = APP_SYSFREQ_104M;
#else
            gaf_decoder_flexible_bth_base_freq = APP_SYSFREQ_72M;
#endif
        }
        app_sysfreq_req(APP_SYSFREQ_USER_AOB_CAPTURE, gaf_decoder_flexible_bth_base_freq);
#else
        gaf_decoder_flexible_bth_base_freq = gaf_audio_stream_encoder_adjust_bth_freq_no_m55(frame_size,
                                            channel, frame_dms, sample_rate, gaf_bth_call_base_freq);
        app_sysfreq_req(APP_SYSFREQ_USER_AOB_CAPTURE, gaf_decoder_flexible_bth_base_freq);
#endif

        // FIXME: It's a workaround method
        if (gaf_stream_process_context_is_call(pStreamEnv->stream_info.bap_contextType)) {
            // Set sysfreq depend on algos
#ifdef GAF_CODEC_CROSS_CORE
            app_sysfreq_req(APP_SYSFREQ_USER_SPEECH_ALGO, APP_SYSFREQ_32K);
#else
            // app_sysfreq_req(APP_SYSFREQ_USER_SPEECH_ALGO, APP_SYSFREQ_104M);
            app_sysfreq_req(APP_SYSFREQ_USER_AOB_CAPTURE, APP_SYSFREQ_104M);
#endif
        }

#ifdef ANC_ASSIST_LE_AUDIO_SUPPORT
#if defined(VOICE_ASSIST_FF_FIR_LMS)
        app_voice_assist_fir_lms_enable_fir(false);
#endif
        app_anc_assist_set_mode(ANC_ASSIST_MODE_LE_CALL);
#endif

#ifdef SPEECH_BONE_SENSOR
        if ((gaf_stream_process_context_is_call(pStreamEnv->stream_info.bap_contextType)) && (g_capture_sample_rate == AUD_SAMPRATE_16000)) {
            LOG_I("[%s] Enable VPU", __func__);
            g_capture_vpu_enabled = true;
        } else {
            LOG_I("[%s] Disable VPU", __func__);
            g_capture_vpu_enabled = false;
        }
#endif

        pStreamEnv->func_list->stream_func_list.capture_init_stream_buf_func(pStreamEnv);

        /**
         * Call after ANC_ASSIST switch mode, because ANC_ASSIST perhaps open VPU.
         * Make sure power on VPU in here.
         * If use VMIC for VPU and shared with MIC, you must move open VPU after af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
         */
#ifdef SPEECH_BONE_SENSOR
        if (g_capture_vpu_enabled) {
            speech_bone_sensor_open(g_capture_sample_rate, g_capture_frame_len);
// #ifdef VPU_STREAM_SYNC_TUNING
//         bt_sco_i2s_sync_tuning_reset();
// #endif

// #ifdef HW_I2S_TDM_TRIGGER
//         af_i2s_sync_config(AUD_STREAM_CAPTURE, AF_I2S_SYNC_TYPE_BT, true);
// #else
//         ASSERT(0, "Need to enable HW_I2S_TDM_TRIGGER");
// #endif
        }
#endif

#ifdef GAF_ENCODER_CROSS_CORE_USE_M55
        pStreamEnv->stream_info.tx_algo_cfg.channel_num = g_capture_ch_num;
#ifdef SPEECH_BONE_SENSOR
        if (g_capture_vpu_enabled) {
            pStreamEnv->stream_info.tx_algo_cfg.channel_num++;
        }
#endif
        pStreamEnv->stream_info.tx_algo_cfg.frame_len = g_capture_frame_len;
        pStreamEnv->stream_info.tx_algo_cfg.sample_rate = g_capture_sample_rate;
        pStreamEnv->stream_info.tx_algo_cfg.bits = pStreamEnv->stream_info.captureInfo.bits_depth;
#ifdef ANC_APP
        pStreamEnv->stream_info.tx_algo_cfg.anc_mode = app_anc_get_curr_mode();
#endif
        if (gaf_stream_process_context_is_call(pStreamEnv->stream_info.bap_contextType)) {
            pStreamEnv->stream_info.tx_algo_cfg.bypass = false;
            app_overlay_subsys_select(APP_OVERLAY_M55, APP_OVERLAY_SUBSYS_SPEECH_ALGO);
        } else {
            pStreamEnv->stream_info.tx_algo_cfg.bypass = true;
        }
#endif

        pStreamEnv->func_list->encoder_func_list->encoder_init_func(pStreamEnv);

        struct AF_STREAM_CONFIG_T stream_cfg;

        af_set_priority(AF_USER_AOB_CAPTURE, osPriorityHigh);
        // capture stream
        memset_s((void *)&stream_cfg, sizeof(struct AF_STREAM_CONFIG_T),
               0, sizeof(struct AF_STREAM_CONFIG_T));
        stream_cfg.bits         = (enum AUD_BITS_T)(pStreamEnv->stream_info.captureInfo.bits_depth);
        stream_cfg.sample_rate  = (enum AUD_SAMPRATE_T)g_capture_sample_rate;
        stream_cfg.channel_num  = (enum AUD_CHANNEL_NUM_T)g_capture_ch_num;
        stream_cfg.device       = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.io_path      = AUD_INPUT_PATH_MAINMIC;

        if (gaf_stream_process_context_is_binaural_record(pStreamEnv->stream_info.bap_contextType))
        {
#ifdef BINAURAL_RECORD_ADC_HIGH_SAMPLE_RATE
            ASSERT((stream_cfg.sample_rate == 32000) || (stream_cfg.sample_rate == 48000),
                    "[%s] Don't support this sample rate: %d", __func__, stream_cfg.sample_rate);
            stream_cfg.sample_rate  = (enum AUD_SAMPRATE_T)BINAURAL_RECORD_ADC_HIGH_SAMPLE_RATE;
#endif
            stream_cfg.channel_map  = (enum AUD_CHANNEL_MAP_T)(AUD_CHANNEL_MAP_CH0);
        } else {
#ifdef ANC_ASSIST_LE_AUDIO_SUPPORT
            ASSERT(stream_cfg.bits == AUD_BITS_24, "[%s] bits(%d) != AUD_BITS_24", __func__, stream_cfg.bits);
            ASSERT(stream_cfg.sample_rate % AUD_SAMPRATE_16000 == 0, "[%s] sample_rate(%d) is invalid", __func__, stream_cfg.sample_rate);
            app_anc_assist_set_capture_info(g_capture_frame_len);
            if (g_capture_sample_rate != AUD_SAMPRATE_16000) {
                gaf_audio_anc_resample_heap_init();
                anc_assist_resample_init(g_capture_sample_rate, g_capture_frame_len, &gaf_resample_allocator);
            }
#elif defined(BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT)
            stream_cfg.io_path      = AUD_INPUT_PATH_MAINMIC;
            stream_cfg.channel_num  = (enum AUD_CHANNEL_NUM_T)(pStreamEnv->stream_info.captureInfo.num_channels);
            if (stream_cfg.channel_num == AUD_CHANNEL_NUM_2)
            {
                stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)( AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1);
            }
#endif
        }

        g_capture_ch_num        = stream_cfg.channel_num;

        stream_cfg.vol          = vol;
        stream_cfg.data_size    = (uint32_t)(2 * pStreamEnv->stream_info.captureInfo.dmaChunkSize);
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(pStreamEnv->stream_info.captureInfo.dmaBufPtr);
        stream_cfg.handler = pStreamEnv->func_list->stream_func_list.capture_dma_irq_handler_func;

#ifdef PSAP_SW_USE_DAC1
        af_codec_replace_echo_ref0_init(&stream_cfg, &g_capture_stream_channel_map, &g_capture_stream_channel_num);
#endif

        LOG_I("[%s] global frame_len: %d, ch_num: %d", __func__, g_capture_frame_len, g_capture_ch_num);

        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);

#ifdef STREAM_PROCESS_ENABLE
        gaf_stream_process_capture_open(pStreamEnv->stream_info.bap_contextType, &stream_cfg);
#endif
        gaf_stream_process_capture_upsampling_init(&pStreamEnv->stream_info, &stream_cfg);
        uint32_t stream_process_upsampling_buf_size = gaf_stream_process_need_upsampling_buf_size(pStreamEnv->stream_info.bap_contextType);
        if (stream_process_upsampling_buf_size != 0) {
            gaf_stream_process_set_upsampling_buf((uint8_t *)app_audio_mempool_calloc(stream_process_upsampling_buf_size, sizeof(int8_t)),
                                                  stream_process_upsampling_buf_size);
        }

        pStreamEnv->stream_context.isUsSinceLatestAnchorPointConfigured = false;
        pStreamEnv->stream_context.captureTriggerChannel = app_bt_sync_get_available_trigger_channel(APP_BT_SYNC_OP_MAX, APP_BT_SYNC_POLICY_DEFAULT);

        gaf_media_prepare_capture_trigger(pStreamEnv->stream_context.captureTriggerChannel);

        gaf_media_pid_init(&(pStreamEnv->stream_context.capture_pid_env));
        gaf_media_pid_update_threshold(&(pStreamEnv->stream_context.capture_pid_env),
            pStreamEnv->stream_info.captureInfo.dmaChunkIntervalUs/2);

        gaf_stream_common_update_capture_stream_state(pStreamEnv, GAF_CAPTURE_STREAM_INITIALIZED);
        gaf_stream_common_start_sync_capture(pStreamEnv);

        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);

#ifdef SPEECH_BONE_SENSOR
        if (g_capture_vpu_enabled) {
            speech_bone_sensor_start();
        }
#endif

        LOG_D("%s end", __func__);
        return 0;
    }

    return -1;
}

static void gaf_audio_flexible_capture_buf_init(void* _pStreamEnv)
{
    LOG_D("%s start", __func__);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    gaf_audio_flexible_common_buf_init(pStreamEnv, GAF_STREAM_CAPTURE);

#ifdef GAF_CODEC_CROSS_CORE
    gaf_list_new(&pStreamEnv->stream_context.m55_capture_buff_list.buff_list,
                    (osMutex(gaf_m55_encoder_buffer_mutex)),
                    (list_free_cb)gaf_m55_stream_encoder_data_free,
                    (list_mempool_zmalloc)gaf_stream_heap_cmalloc,
                    (list_mempool_free)gaf_m55_stream_encoder_heap_free);
#else
    gaf_list_new(&pStreamEnv->stream_context.capture_buff_list,
                    (osMutex(gaf_encoder_buffer_mutex)),
                    (list_free_cb)gaf_stream_data_free,
                    (list_mempool_zmalloc)gaf_stream_heap_cmalloc,
                    (list_mempool_free)gaf_stream_heap_free);
#endif

    pStreamEnv->func_list->encoder_func_list->encoder_init_buf_func(pStreamEnv);
    LOG_D("%s end", __func__);
}

static int gaf_audio_flexible_capture_stream_stop_handler(void* _pStreamEnv)
{
    LOG_D("%s start", __func__);
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

#ifdef SPEECH_BONE_SENSOR
        if (g_capture_vpu_enabled) {
            speech_bone_sensor_stop();
            speech_bone_sensor_close();
// #ifdef VPU_STREAM_SYNC_TUNING
//         bt_sco_i2s_sync_tuning_reset();
// #endif
        }
        g_capture_vpu_enabled = false;
#endif

        gaf_stream_process_capture_upsampling_deinit();
#ifdef STREAM_PROCESS_ENABLE
        gaf_stream_process_capture_close();
#endif

        gaf_stream_common_clr_trigger(pStreamEnv->stream_context.captureTriggerChannel);

        af_codec_set_bt_sync_source(AUD_STREAM_CAPTURE, 0);

        app_sysfreq_req(APP_SYSFREQ_USER_AOB_CAPTURE, APP_SYSFREQ_32K);
        af_set_priority(AF_USER_AOB_CAPTURE, osPriorityAboveNormal);

        pStreamEnv->func_list->encoder_func_list->encoder_deinit_func(pStreamEnv);

        pStreamEnv->func_list->stream_func_list.capture_deinit_stream_buf_func(pStreamEnv);

#ifdef GAF_ENCODER_CROSS_CORE_USE_M55
        app_overlay_subsys_unloadall(APP_OVERLAY_M55);
#else
        app_overlay_unloadall();
#endif

#ifdef ANC_ASSIST_LE_AUDIO_SUPPORT
        if (g_capture_sample_rate != AUD_SAMPRATE_16000) {
            anc_assist_resample_deinit();
        }
        app_anc_assist_set_mode(ANC_ASSIST_MODE_STANDALONE);
#if defined(VOICE_ASSIST_FF_FIR_LMS)
        app_voice_assist_fir_lms_enable_fir(true);
#endif
#endif
        LOG_D("%s end", __func__);
        return 0;
    }

    return -1;
}

static void gaf_stream_capture_dma_irq_handler_send(void* pStreamEnv_, void *payload,
    uint32_t payload_size, uint32_t ref_time)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T*) pStreamEnv_;

    uint8_t ase_count = GAF_AUDIO_ASE_TOTAL_COUNT;
    uint8_t channel_shift = 0;
    uint8_t audio_allocation_cnt = 0;
    uint16_t payload_len_per_channel = 0;

    for (uint8_t i = 0; i < ase_count; i++)
    {
        if (GAF_AUDIO_INVALID_ISO_CHANNEL == pStreamEnv->stream_info.captureInfo.aseChInfo[i].iso_channel_hdl)
        {
            continue;
        }

        channel_shift = 0;
        audio_allocation_cnt = bes_ble_audio_get_location_fs_l_r_cnt(pStreamEnv->stream_info.captureInfo.aseChInfo[i].allocation_bf);
        // Config palyload len  for AUD_CHANNEL_NUM_2
        payload_len_per_channel = payload_size / pStreamEnv->stream_info.captureInfo.num_channels;
        // Check is there use two ASE
        if (audio_allocation_cnt == AUD_CHANNEL_NUM_1)
        {
            if ((pStreamEnv->stream_info.captureInfo.aseChInfo[i].allocation_bf &
                (BES_BLE_LOC_SIDE_RIGHT | BES_BLE_LOC_FRONT_RIGHT)) &&
                pStreamEnv->stream_info.captureInfo.num_channels == AUD_CHANNEL_NUM_2)
            {
                // shift to right channel
                channel_shift += payload_len_per_channel;
            }
        }
        else// if (audio_allocation_cnt == AUD_CHANNEL_NUM_2)
        {
            payload_len_per_channel = payload_size;
            channel_shift = 0;
        }

        /// gaf custom, may be a watch point to put some pattern in encoded packet to be send
        if (gaf_uc_srv_custom_data_callback->encoded_packet_send_cb)
        {
            gaf_uc_srv_custom_data_callback->encoded_packet_send_cb((uint8_t *)payload + channel_shift, payload_len_per_channel);
        }

        LOG_D("channel_shift: %d, payload_len_per_channel: %d", channel_shift, payload_len_per_channel);
        bes_ble_bap_iso_dp_send_data(pStreamEnv->stream_info.captureInfo.aseChInfo[i].cis_hdl,
                                pStreamEnv->stream_context.latestCaptureSeqNum,
                                (uint8_t *)payload + channel_shift, payload_len_per_channel,
                                ref_time);
    }
}

static uint8_t *gaf_stream_captured_data_pre_processing_handler(GAF_AUDIO_STREAM_ENV_T* pStreamEnv, uint8_t* ptrBuf, uint32_t* pLength)
{
    uint32_t length = *pLength;
    int32_t POSSIBLY_UNUSED *pcm_buf = (int32_t *)ptrBuf;
    uint32_t POSSIBLY_UNUSED pcm_len = length / sizeof(int32_t);
    uint32_t POSSIBLY_UNUSED frame_len = pcm_len / g_capture_ch_num;

#ifdef BINAURAL_RECORD_ADC_HIGH_SAMPLE_RATE
    if (gaf_stream_process_context_is_binaural_record(pStreamEnv->stream_info.bap_contextType)) {
        frame_len /= g_bin_record_sample_rate_div;
        for (uint32_t i=0; i<frame_len; i++) {
            for (uint32_t ch=0; ch<g_capture_ch_num; ch++) {    //BES intentional code. g_capture_ch_num will never be 0.
                pcm_buf[g_capture_ch_num * i + ch] = pcm_buf[g_capture_ch_num * g_bin_record_sample_rate_div * i + ch];
            }
        }
        length /= g_bin_record_sample_rate_div;
        pcm_len /= g_bin_record_sample_rate_div;
    }
#endif

#ifdef ANC_ASSIST_LE_AUDIO_SUPPORT
    if (!gaf_stream_process_context_is_binaural_record(pStreamEnv->stream_info.bap_contextType)) {
        uint32_t anc_assist_len = length / (g_capture_sample_rate / AUD_SAMPRATE_16000);
        float *temp_anc = (float *)g_anc_assist_interval_buf;

        if(app_anc_assist_is_runing()) {
            for (uint8_t ch = 0; ch < g_capture_ch_num; ch++) { //BES intentional code. g_capture_ch_num will never be 0.
                for (uint32_t i = 0; i < (length / g_capture_ch_num / sizeof(float)); i++) {
                    temp_anc[ch * (length / g_capture_ch_num / sizeof(float)) + i] = (float)pcm_buf[i * g_capture_ch_num + ch];
                }
            }
            anc_assist_resample_process(temp_anc, (length / g_capture_ch_num / sizeof(float)), (length / g_capture_ch_num / sizeof(float)));
            //BES intentional code. g_capture_ch_num will never be 0.
            app_anc_assist_process_interval(temp_anc, anc_assist_len);
        }
    }
#endif

// NOTE: Add gain in algo process flow
#if 0
    // Add 18 dB gain for MIC
    if (pStreamEnv->stream_info.captureInfo.bits_depth == 16) {
        int16_t *pcm_buf = (int16_t *)ptrBuf;
        uint32_t pcm_len = length / sizeof(int16_t);
        for (uint32_t i=0; i<pcm_len; i++) {
            pcm_buf[i] = __SSAT(((int32_t)(pcm_buf[i])) << 3, 16);
        }
    } else if (pStreamEnv->stream_info.captureInfo.bits_depth == 24) {
        int32_t *pcm_buf = (int32_t *)ptrBuf;
        uint32_t pcm_len = length / sizeof(int32_t);
        for (uint32_t i=0; i<pcm_len; i++) {
            pcm_buf[i] = __SSAT(pcm_buf[i] << 3, 24);
        }
    } else {
        ASSERT(0, "[%s] bits(%d) is invalid", __func__, pStreamEnv->stream_info.captureInfo.bits_depth);
    }
#endif

#ifdef STREAM_PROCESS_ENABLE
    length = gaf_stream_process_capture_run(ptrBuf, length);
#endif
    length = gaf_stream_process_capture_upsampling_run(ptrBuf, length);
    uint8_t *capture_upsampling_buf = gaf_stream_process_get_upsampling_buf();
    if (capture_upsampling_buf){
        ptrBuf = capture_upsampling_buf;
    }

#if defined(SPEECH_BONE_SENSOR) && defined(GAF_ENCODER_CROSS_CORE_USE_M55)
    if (g_capture_vpu_enabled) {
        speech_bone_sensor_get_data(g_capture_combo_vpu_pcm_buf, frame_len, SPEECH_BS_SINGLE_CHANNEL, 24);
        for (int32_t i=frame_len-1; i>=0; i--) {
            g_capture_combo_vpu_pcm_buf[i * (g_capture_ch_num + 1) + g_capture_ch_num] = g_capture_combo_vpu_pcm_buf[i];
        }

        for (uint32_t ch=0; ch<g_capture_ch_num; ch++) {
            for (uint32_t i=0; i<frame_len; i++) {
                g_capture_combo_vpu_pcm_buf[i * (g_capture_ch_num + 1) + ch] = pcm_buf[i * g_capture_ch_num + ch];
            }
        }

        ptrBuf = (uint8_t *)g_capture_combo_vpu_pcm_buf;
        length = (length / g_capture_ch_num) * (g_capture_ch_num + 1);
    }
#endif

    *pLength = length;

    return ptrBuf;
}


static bool gaf_stream_is_any_upstream_iso_created(uint8_t con_lid)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_audio_stream_env[con_lid];

    for (uint8_t i = 0; i < GAF_AUDIO_ASE_TOTAL_COUNT; i++)
    {
        if (GAF_AUDIO_INVALID_ISO_CHANNEL != pStreamEnv->stream_info.captureInfo.aseChInfo[i].iso_channel_hdl)
        {
            return true;
        }
    }

    return false;
}

static uint32_t _gaf_stream_flexible_capture_dma_irq_handler(uint8_t con_lid, uint8_t* ptrBuf, uint32_t length)
{
    LOG_D("%s start,length = %d", __func__,length);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = NULL;

    pStreamEnv = &gaf_audio_stream_env[con_lid];

    if(pStreamEnv == NULL)
    {
        LOG_D("%s pStreamEnv is NULL", __func__);
        return 0;
    }

    uint8_t muted = bes_ble_arc_get_mic_state(pStreamEnv->stream_info.con_lid);
    if (muted) {
        memset((uint8_t*)ptrBuf, 0x0, length);
    }

    if (!gaf_stream_is_any_upstream_iso_created(pStreamEnv->stream_info.con_lid))
    {
        return length;
    }

#ifdef GAF_CODEC_CROSS_CORE
    pLocalCaptureStreamEnvPtr = pStreamEnv;
#endif

    uint32_t dmaIrqHappeningTimeUs = 0;
    uint8_t adma_ch = HAL_DMA_CHAN_NONE;
    uint32_t dma_base;
    uint32_t btclk; //hal slot -- 312.5us
    uint16_t btcnt; //hal microsecond -- 0.5 us

    adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    if (adma_ch != HAL_DMA_CHAN_NONE)
    {
        bt_drv_reg_op_dma_tc_clkcnt_get_by_ch(&btclk, &btcnt,adma_ch&0xFF, dma_base);
        dmaIrqHappeningTimeUs = bt_syn_ble_bt_time_to_bts(btclk, btcnt);
    }

    if ((!pStreamEnv) ||
        (GAF_CAPTURE_STREAM_START_TRIGGERING > pStreamEnv->stream_context.capture_stream_state)) {
        memset_s(ptrBuf, length, 0x00, length);
        return length;
    } else if (GAF_CAPTURE_STREAM_START_TRIGGERING == pStreamEnv->stream_context.capture_stream_state) {
        gaf_stream_common_update_capture_stream_state(pStreamEnv, GAF_CAPTURE_STREAM_STREAMING_TRIGGERED);
        gaf_stream_common_clr_trigger(pStreamEnv->stream_context.captureTriggerChannel);
        uint32_t latest_iso_bt_time = gaf_media_common_get_latest_tx_iso_evt_timestamp(pStreamEnv);
        if (!pStreamEnv->stream_context.isUsSinceLatestAnchorPointConfigured)
        {
            pStreamEnv->stream_context.isUsSinceLatestAnchorPointConfigured = true;
            pStreamEnv->stream_context.usSinceLatestAnchorPoint = GAF_AUDIO_CLK_32_BIT_DIFF(dmaIrqHappeningTimeUs, latest_iso_bt_time);
            LOG_I("initial usSinceAnch %d", pStreamEnv->stream_context.usSinceLatestAnchorPoint);
            LOG_I("anch time %d dma irq time %d", latest_iso_bt_time, dmaIrqHappeningTimeUs);
            gaf_stream_common_sync_us_since_latest_anchor_point(pStreamEnv);
        }
    }

    // it's possible the multiple DMA irq triggered message accumulates,
    // so the acuiqred dmaIrqHappeningTimeUs is the last dma irq, which has
    // been handled. For this case, ignore this dma irq message handling
    if ((GAF_CAPTURE_STREAM_STREAMING_TRIGGERED ==
        pStreamEnv->stream_context.capture_stream_state) &&
        (dmaIrqHappeningTimeUs ==
        pStreamEnv->stream_context.lastCaptureDmaIrqTimeUs))
    {
        LOG_W("accumulated irq messages happen!");
        return length;
    }

    gaf_stream_common_capture_timestamp_checker(pStreamEnv, dmaIrqHappeningTimeUs);

    LOG_D("%s length %d encoded_len %d filled timestamp %u", __func__, length,
        pStreamEnv->stream_info.captureInfo.encoded_frame_size,
        dmaIrqHappeningTimeUs);

    /// gaf custom, may be a watch point to put some pattern in pcm raw data to be encode
    if (gaf_uc_srv_custom_data_callback->raw_pcm_data_cb)
    {
        gaf_uc_srv_custom_data_callback->raw_pcm_data_cb(ptrBuf, length);
    }
#ifdef IS_BLE_AUDIO_DEBUG_INFO_COLLECTOR_ENABLED
    if (NULL != capture_dma_irq_info_record_handler)
    {
        capture_dma_irq_info_record_handler(pStreamEnv->stream_info.con_lid, dmaIrqHappeningTimeUs,
                                            pStreamEnv->stream_context.latestCaptureSeqNum);
    }
#endif
#ifdef GAF_ENCODER_CROSS_CORE_USE_M55
    /// store the pcm data into capture buff list,
    /// and if bth putted the pcm data into list, it will be send m55 instantly.
    /// if bth could not putted into the list, and it will loop to take the over steps,
    /// so to it is successed.
    do {
peek_again:
        if (gaf_m55_deinit_status.capture_deinit == true)
        {
            break;
        }
        ptrBuf = gaf_stream_captured_data_pre_processing_handler(pStreamEnv, ptrBuf, &length);

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
            POSSIBLY_UNUSED int32_t channelNum = pStreamEnv->stream_info.captureInfo.num_channels;
            output_buf = (uint8_t *)gaf_stream_heap_cmalloc(lc3_encoded_frame_len);

            // bth fetch encoded data
            gaf_encoder_core_fetch_encoded_data(pStreamEnv, output_buf, lc3_encoded_frame_len, dmaIrqHappeningTimeUs);
            LOG_D("lc3_encoded_frame_len %d ", lc3_encoded_frame_len);
            // bth send out encoded data
            gaf_stream_capture_dma_irq_handler_send(pStreamEnv, output_buf, lc3_encoded_frame_len, dmaIrqHappeningTimeUs);
            gaf_stream_heap_free(output_buf);
        }
    } while(0);
#else
    ptrBuf = gaf_stream_captured_data_pre_processing_handler(pStreamEnv, ptrBuf, &length);

    pStreamEnv->func_list->encoder_func_list->encoder_encode_frame_func(pStreamEnv, dmaIrqHappeningTimeUs,
        length, ptrBuf, &pStreamEnv->stream_context.codec_alg_context[0],
        &gaf_stream_capture_dma_irq_handler_send);
#endif
    LOG_D("%s end", __func__);
    return length;
}

static uint32_t gaf_stream_flexible_capture_idx_0_dma_irq_handler(uint8_t* ptrBuf, uint32_t length)
{
    return _gaf_stream_flexible_capture_dma_irq_handler(0, ptrBuf, length);
}

static uint32_t gaf_stream_flexible_capture_idx_1_dma_irq_handler(uint8_t* ptrBuf, uint32_t length)
{
    return _gaf_stream_flexible_capture_dma_irq_handler(1, ptrBuf, length);
}

static uint32_t gaf_stream_flexible_capture_idx_2_dma_irq_handler(uint8_t* ptrBuf, uint32_t length)
{
    return _gaf_stream_flexible_capture_dma_irq_handler(2, ptrBuf, length);
}

static void gaf_audio_flexible_capture_buf_deinit(void* _pStreamEnv)
{
    LOG_D("%s start", __func__);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    pStreamEnv->stream_info.captureInfo.dmaBufPtr = NULL;
#ifdef GAF_CODEC_CROSS_CORE
    gaf_list_free(&pStreamEnv->stream_context.m55_capture_buff_list.buff_list);
#else
    gaf_list_free(&pStreamEnv->stream_context.capture_buff_list);
#endif

    pStreamEnv->func_list->encoder_func_list->encoder_deinit_buf_func(pStreamEnv);
    LOG_I("[%s] syspool free size: %d/%d", __func__, syspool_free_size(), syspool_total_size());
    LOG_D("%s end", __func__);
}

static GAF_AUDIO_FUNC_LIST_T gaf_audio_flexible_stream_func_list[GAF_MAXIMUM_CONNECTION_COUNT] =
{
    {
        {
            .playback_dma_irq_handler_func = gaf_stream_flexible_playback_idx_0_dma_irq_handler,
            .capture_dma_irq_handler_func = gaf_stream_flexible_capture_idx_0_dma_irq_handler,

            .playback_start_stream_func = gaf_audio_flexible_playback_stream_start_handler,
            .playback_init_stream_buf_func = gaf_audio_flexible_playback_buf_init,
            .playback_stop_stream_func = gaf_audio_flexible_playback_stream_stop_handler,
            .playback_deinit_stream_buf_func = gaf_audio_flexible_playback_buf_deinit,

            .capture_start_stream_func = gaf_audio_flexible_capture_stream_start_handler,
            .capture_init_stream_buf_func = gaf_audio_flexible_capture_buf_init,
            .capture_stop_stream_func = gaf_audio_flexible_capture_stream_stop_handler,
            .capture_deinit_stream_buf_func = gaf_audio_flexible_capture_buf_deinit,
        }
    },

    {
        {
            .playback_dma_irq_handler_func = gaf_stream_flexible_playback_idx_1_dma_irq_handler,
            .capture_dma_irq_handler_func = gaf_stream_flexible_capture_idx_1_dma_irq_handler,

            .playback_start_stream_func = gaf_audio_flexible_playback_stream_start_handler,
            .playback_init_stream_buf_func = gaf_audio_flexible_playback_buf_init,
            .playback_stop_stream_func = gaf_audio_flexible_playback_stream_stop_handler,
            .playback_deinit_stream_buf_func = gaf_audio_flexible_playback_buf_deinit,

            .capture_start_stream_func = gaf_audio_flexible_capture_stream_start_handler,
            .capture_init_stream_buf_func = gaf_audio_flexible_capture_buf_init,
            .capture_stop_stream_func = gaf_audio_flexible_capture_stream_stop_handler,
            .capture_deinit_stream_buf_func = gaf_audio_flexible_capture_buf_deinit,
        }
    },

    {
        {
            .playback_dma_irq_handler_func = gaf_stream_flexible_playback_idx_2_dma_irq_handler,
            .capture_dma_irq_handler_func = gaf_stream_flexible_capture_idx_2_dma_irq_handler,

            .playback_start_stream_func = gaf_audio_flexible_playback_stream_start_handler,
            .playback_init_stream_buf_func = gaf_audio_flexible_playback_buf_init,
            .playback_stop_stream_func = gaf_audio_flexible_playback_stream_stop_handler,
            .playback_deinit_stream_buf_func = gaf_audio_flexible_playback_buf_deinit,

            .capture_start_stream_func = gaf_audio_flexible_capture_stream_start_handler,
            .capture_init_stream_buf_func = gaf_audio_flexible_capture_buf_init,
            .capture_stop_stream_func = gaf_audio_flexible_capture_stream_stop_handler,
            .capture_deinit_stream_buf_func = gaf_audio_flexible_capture_buf_deinit,
        }
    },
};

static GAF_AUDIO_STREAM_ENV_T* gaf_audio_get_stream_env_from_ase(uint8_t ase_lid)
{
    const bes_ble_bap_ascs_ase_t *p_bap_ase_info = bes_ble_get_ascs_ase_info(ase_lid);

    return &gaf_audio_stream_env[p_bap_ase_info->con_lid];
}

void gaf_audio_update_stream_iso_hdl(uint8_t ase_lid)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = gaf_audio_get_stream_env_from_ase(ase_lid);
    if (!pStreamEnv)
    {
        LOG_E("[UPDATE ISO HDL] ase lid %d with no stream info now, please check!!!", ase_lid);
        return;
    }

    const bes_ble_bap_ascs_ase_t *p_bap_ase_info = bes_ble_get_ascs_ase_info(ase_lid);
    uint8_t aseChnInfoIdx = p_bap_ase_info->ase_lid % GAF_AUDIO_ASE_TOTAL_COUNT;
    GAF_AUDIO_STREAM_COMMON_INFO_T* pCommonInfo;
    if (BES_BLE_GAF_DIRECTION_SINK == p_bap_ase_info->direction)
    {
        pCommonInfo = &(pStreamEnv->stream_info.playbackInfo);
    }
    else
    {
        pCommonInfo = &(pStreamEnv->stream_info.captureInfo);
    }

    pCommonInfo->aseChInfo[aseChnInfoIdx].iso_channel_hdl = BLE_ISOHDL_TO_ACTID(p_bap_ase_info->cis_hdl);
    pCommonInfo->aseChInfo[aseChnInfoIdx].cis_hdl = p_bap_ase_info->cis_hdl;
    LOG_I("[UPDATE ISO HDL] ase lid %d cis hdl %d con_lid %d channel %d",
        ase_lid, pCommonInfo->aseChInfo[aseChnInfoIdx].cis_hdl,
        p_bap_ase_info->con_lid, pCommonInfo->aseChInfo[aseChnInfoIdx].iso_channel_hdl);

    /// CIS connected and check for capture trigger set
    if (BES_BLE_GAF_DIRECTION_SRC == p_bap_ase_info->direction &&
        pStreamEnv->stream_context.capture_stream_state == GAF_CAPTURE_STREAM_INITIALIZED)
    {
        gaf_stream_common_start_sync_capture(pStreamEnv);
    }
}

POSSIBLY_UNUSED static GAF_AUDIO_STREAM_ENV_T* gaf_audio_refresh_stream_info_from_ase(uint8_t ase_lid)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = gaf_audio_get_stream_env_from_ase(ase_lid);
    if (!pStreamEnv)
    {
        LOG_W("ase is not ready yet for refresh stream info from");
        return NULL;
    }
    const bes_ble_bap_ascs_ase_t *p_bap_ase_info = bes_ble_get_ascs_ase_info(ase_lid);
    GAF_AUDIO_STREAM_COMMON_INFO_T* pCommonInfo;
    uint8_t audio_alloc_l_r_cnt = 0;
    uint8_t aseChnInfoIdx = p_bap_ase_info->ase_lid % GAF_AUDIO_ASE_TOTAL_COUNT;
    uint8_t loc_supp_audio_location = 0;

    pStreamEnv->stream_info.bap_contextType = p_bap_ase_info->p_metadata->param.context_bf;
    LOG_I("[%s] TYPE bap: %d, app: %d", __func__, pStreamEnv->stream_info.bap_contextType, pStreamEnv->stream_info.contextType);

    if (BES_BLE_GAF_DIRECTION_SINK == p_bap_ase_info->direction)
    {
        loc_supp_audio_location = bes_ble_bap_capa_get_location_bf(BES_BLE_GAF_DIRECTION_SINK);
        pCommonInfo = &(pStreamEnv->stream_info.playbackInfo);
    }
    else
    {
        loc_supp_audio_location = bes_ble_bap_capa_get_location_bf(BES_BLE_GAF_DIRECTION_SRC);
        pCommonInfo = &(pStreamEnv->stream_info.captureInfo);
    }
    /// Check for this stream's playback or capture info is refreshed
    if (pCommonInfo->aseChInfo[aseChnInfoIdx].allocation_bf != 0 && pCommonInfo->frame_ms != 0)
    {
        LOG_I("already been refreshed %s stream info, allocation 0x%x via ase lid: %d",
              (p_bap_ase_info->direction == BES_BLE_GAF_DIRECTION_SINK ? "playback" : "capture"),
              pCommonInfo->aseChInfo[aseChnInfoIdx].allocation_bf,
              ase_lid);
        return pStreamEnv;
    }
    /// update stream common info
    pCommonInfo->num_channels = bes_ble_audio_get_location_fs_l_r_cnt(loc_supp_audio_location);
    pCommonInfo->cigSyncDelayUs = p_bap_ase_info->cig_sync_delay;
    pCommonInfo->isoIntervalUs = p_bap_ase_info->iso_interval_us;
    pCommonInfo->bnM2S = p_bap_ase_info->bn_m2s;
    pCommonInfo->bnS2M = p_bap_ase_info->bn_s2m;
    pCommonInfo->bits_depth = GAF_AUDIO_STREAM_BIT_NUM;
    /// get custom set presDelay first
    pCommonInfo->presDelayUs = gaf_stream_common_get_custom_presdelay_us();
    if (pCommonInfo->presDelayUs == 0)
    {
        pCommonInfo->presDelayUs = p_bap_ase_info->qos_cfg.pres_delay_us > GAF_AUDIO_CONTROLLER_2_HOST_LATENCY_US ?
                                   p_bap_ase_info->qos_cfg.pres_delay_us : GAF_AUDIO_CONTROLLER_2_HOST_LATENCY_US;
    }

    pCommonInfo->aseChInfo[aseChnInfoIdx].iso_channel_hdl = BLE_ISOHDL_TO_ACTID(p_bap_ase_info->cis_hdl);
    pCommonInfo->aseChInfo[aseChnInfoIdx].cis_hdl = p_bap_ase_info->cis_hdl;
    pCommonInfo->aseChInfo[aseChnInfoIdx].allocation_bf = p_bap_ase_info->p_cfg->param.location_bf;

    audio_alloc_l_r_cnt = bes_ble_audio_get_location_fs_l_r_cnt(p_bap_ase_info->p_cfg->param.location_bf);
    /// multi channel allocation or no specified allocation check
    if ((audio_alloc_l_r_cnt >= AUD_CHANNEL_NUM_2 && p_bap_ase_info->qos_cfg.max_sdu_size ==
         p_bap_ase_info->p_cfg->param.frame_octet * p_bap_ase_info->p_cfg->param.frames_sdu) ||
        /// absence of audio allocation, value == 0
        (audio_alloc_l_r_cnt == 0) ||
        /// Sanity check
        (audio_alloc_l_r_cnt > pCommonInfo->num_channels))
    {
        if (BES_BLE_AUDIO_TWS_SLAVE == bes_ble_audio_get_tws_nv_role())
        {
            pCommonInfo->aseChInfo[aseChnInfoIdx].allocation_bf = BES_BLE_LOC_FRONT_LEFT;
        }
        else
        {
            pCommonInfo->aseChInfo[aseChnInfoIdx].allocation_bf = BES_BLE_LOC_FRONT_RIGHT;
        }

        LOG_W("use mono channel with allocation: 0x%x",
              pCommonInfo->aseChInfo[aseChnInfoIdx].allocation_bf);
    }

    LOG_I("ase_id:%d direction:%d codec_id.codec_id:%d presDelayus:%u cigDelay:%u", ase_lid,
        p_bap_ase_info->direction, p_bap_ase_info->codec_id.codec_id[0], pCommonInfo->presDelayUs,
        pCommonInfo->cigSyncDelayUs);

    switch (p_bap_ase_info->codec_id.codec_id[0])
    {
        case BES_BLE_GAF_CODEC_TYPE_LC3:
        {
            AOB_BAP_CFG_T* p_lc3_cfg = (AOB_BAP_CFG_T *)p_bap_ase_info->p_cfg;
            pCommonInfo->frame_ms =
                gaf_stream_common_frame_duration_parse(p_lc3_cfg->param.frame_dur);
            pCommonInfo->sample_rate =
                gaf_stream_common_sample_freq_parse(p_lc3_cfg->param.sampling_freq);
            pCommonInfo->encoded_frame_size = p_lc3_cfg->param.frame_octet;
            pCommonInfo->maxCachedEncodedAudioPacketCount = GAF_AUDIO_MEDIA_DATA_PACKET_NUM_LIMITER;
            pCommonInfo->maxEncodedAudioPacketSize = gaf_audio_lc3_encoder_get_max_frame_size();
            pCommonInfo->dmaChunkIntervalUs = (uint32_t)(pCommonInfo->frame_ms*1000);

            g_capture_sample_bytes = (pCommonInfo->bits_depth / 8);
#ifdef __BLE_AUDIO_24BIT__
            g_capture_sample_bytes = sizeof(int32_t);
#endif
            if (BES_BLE_GAF_DIRECTION_SRC == p_bap_ase_info->direction) {
                g_capture_sample_rate = pCommonInfo->sample_rate;

#ifdef LEA_CALL_FIX_ADC_SAMPLE_RATE
                if ((gaf_stream_process_context_is_call(p_bap_ase_info->p_metadata->param.context_bf)) &&
                    (pCommonInfo->sample_rate == AUD_SAMPRATE_32000)) {
                    g_capture_sample_rate = AUD_SAMPRATE_16000;
                }
#endif
                pCommonInfo->dmaChunkSize =
                    (uint32_t)((g_capture_sample_rate *
                    g_capture_sample_bytes *
                    (pCommonInfo->dmaChunkIntervalUs) *
                    pCommonInfo->num_channels) / (1000 * 1000));
            } else {
                pCommonInfo->dmaChunkSize =
                    (uint32_t)((pCommonInfo->sample_rate *
                    g_capture_sample_bytes *
                    (pCommonInfo->dmaChunkIntervalUs) *
                    pCommonInfo->num_channels) / (1000 * 1000));
            }
            gaf_audio_lc3_update_codec_func_list(pStreamEnv);
            break;
        }
#ifdef LC3PLUS_SUPPORT
        case BES_BLE_GAF_CODEC_TYPE_VENDOR:
        {
            AOB_BAP_CFG_T* p_lc3plus_cfg = p_bap_ase_info->p_cfg;
            pCommonInfo->frame_ms =
                gaf_stream_common_frame_duration_parse(p_lc3plus_cfg->param.frame_dur);
            pCommonInfo->sample_rate =
                gaf_stream_common_sample_freq_parse(p_lc3plus_cfg->param.sampling_freq);
            pCommonInfo->maxCachedEncodedAudioPacketCount = GAF_AUDIO_MEDIA_DATA_PACKET_NUM_LIMITER;
            pCommonInfo->maxEncodedAudioPacketSize = gaf_audio_lc3_encoder_get_max_frame_size();
            /// 2.5ms * GAF_AUDIO_MEDIA_DATA_PACKET_NUM_LIMITER <= BAP Presdelay
            if (p_lc3plus_cfg->param.frame_dur == BES_BLE_GAF_BAP_FRAME_DURATION_2_5MS)
            {
                pCommonInfo->maxCachedEncodedAudioPacketCount = GAF_AUDIO_MEDIA_DATA_PACKET_NUM_LIMITER*2;
                pCommonInfo->maxEncodedAudioPacketSize = gaf_audio_lc3_encoder_get_max_frame_size()/2;
            }
            pCommonInfo->encoded_frame_size = p_lc3plus_cfg->param.frame_octet;
            pCommonInfo->dmaChunkIntervalUs = (uint32_t)(pCommonInfo->frame_ms*1000);
            pCommonInfo->bits_depth = AUD_BITS_24;

            g_capture_sample_bytes = sizeof(int32_t);
            if (BES_BLE_GAF_DIRECTION_SRC == p_bap_ase_info->direction) {
                g_capture_sample_rate = pCommonInfo->sample_rate;

#ifdef LEA_CALL_FIX_ADC_SAMPLE_RATE
                if ((gaf_stream_process_context_is_call(p_bap_ase_info->p_metadata->param.context_bf)) &&
                    (pCommonInfo->sample_rate == AUD_SAMPRATE_32000)) {
                    g_capture_sample_rate = AUD_SAMPRATE_16000;
                }
#endif
                pCommonInfo->dmaChunkSize =
                    (uint32_t)((g_capture_sample_rate *
                    g_capture_sample_bytes *
                    (pCommonInfo->dmaChunkIntervalUs) *
                    pCommonInfo->num_channels) / (1000 * 1000));
            } else {
                pCommonInfo->dmaChunkSize =
                    (uint32_t)((pCommonInfo->sample_rate *
                    g_capture_sample_bytes *
                    (pCommonInfo->dmaChunkIntervalUs) *
                    pCommonInfo->num_channels) / (1000 * 1000));
            }

            gaf_audio_lc3plus_update_codec_func_list(pStreamEnv);
            break;
        }
#endif
        default:
            ASSERT(false, "unknown codec type!");
            return NULL;
    }

    LOG_I("isoIntervalUs:%d presDelayus:%u cigDelay:%u, bn m2s:%u s2m:%u appCtxType:%d",
         p_bap_ase_info->iso_interval_us, p_bap_ase_info->qos_cfg.pres_delay_us,
        p_bap_ase_info->cig_sync_delay, pCommonInfo->bnM2S, pCommonInfo->bnS2M, pStreamEnv->stream_info.contextType);
    LOG_I("ase lid %d cis hdl %d con_lid %d channel %d", ase_lid, pCommonInfo->aseChInfo[aseChnInfoIdx].cis_hdl,
        p_bap_ase_info->con_lid, pCommonInfo->aseChInfo[aseChnInfoIdx].iso_channel_hdl);
    LOG_I("frame len %d us, sample rate %d dma chunk time %d us dma chunk size %d",
        (uint32_t)(pCommonInfo->frame_ms*1000), pCommonInfo->sample_rate,
        pCommonInfo->dmaChunkIntervalUs, pCommonInfo->dmaChunkSize);
    LOG_I("num of channel = %d", pCommonInfo->num_channels);
    LOG_I("allocation: 0x%x", pCommonInfo->aseChInfo[aseChnInfoIdx].allocation_bf);
    LOG_I("context: %s",
        gaf_stream_common_print_context(p_bap_ase_info->p_metadata->param.context_bf));
    LOG_I("codec: %s", gaf_stream_common_print_code_type(p_bap_ase_info->codec_id.codec_id[0]));

    return pStreamEnv;
}

#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
uint32_t gaf_get_ble_audio_playback_sample_rate(void)
{
    return ble_audio_playback_sample_rate;
}

void gaf_set_ble_audio_playback_sample_rate(uint32_t sample_rate)
{
    ble_audio_playback_sample_rate = sample_rate;
}
#endif

bool gaf_audio_is_playback_stream_on(void)
{
    GAF_AUDIO_STREAM_ENV_T *pStreamEnv = NULL;
    GAF_MEDIA_DWELLING_INFO_T* pDewellingInfo = NULL;
    for (uint8_t con_lid = 0; con_lid < GAF_MAXIMUM_CONNECTION_COUNT; con_lid++)
    {
        pStreamEnv = &gaf_audio_stream_env[con_lid];

        if (pStreamEnv->stream_context.capture_stream_state != GAF_CAPTURE_STREAM_IDLE ||
            pStreamEnv->stream_context.playback_stream_state != GAF_PLAYBACK_STREAM_IDLE)
        {
            pDewellingInfo = &gaf_cis_media_dwelling_info[con_lid];

            if (GAF_AUDIO_STREAM_TYPE_PLAYBACK & pDewellingInfo->startedStreamTypes ||
                GAF_AUDIO_STREAM_TYPE_FLEXIBLE & pDewellingInfo->startedStreamTypes)
            {
                return true;
            }
        }
    }

    return false;
}

gaf_stream_context_state_t gaf_audio_update_stream_state_info_from_ase(
                            GAF_AUDIO_UPDATE_STREAM_INFO_PURPOSE_E purpose, uint8_t ase_lid)
{
    const bes_ble_bap_ascs_ase_t *p_bap_ase_info = bes_ble_get_ascs_ase_info(ase_lid);

    LOG_I("update stream ase %d purpose %d", ase_lid, purpose);

    GAF_MEDIA_DWELLING_INFO_T* pDewellingInfo =
        &gaf_cis_media_dwelling_info[p_bap_ase_info->con_lid];

    if (BES_BLE_GAF_DIRECTION_SINK == p_bap_ase_info->direction)
    {
        if (GAF_AUDIO_UPDATE_STREAM_INFO_TO_START == purpose)
        {
            pDewellingInfo->startedStreamTypes |= GAF_AUDIO_STREAM_TYPE_PLAYBACK;
        }
        else
        {
            pDewellingInfo->startedStreamTypes &= (~GAF_AUDIO_STREAM_TYPE_PLAYBACK);
            is_gaf_audio_startedStreamTypes[ase_lid] = GAF_AUDIO_STREAM_NON_START;
        }
    }
    else
    {
        if (GAF_AUDIO_UPDATE_STREAM_INFO_TO_START == purpose)
        {
            pDewellingInfo->startedStreamTypes |= GAF_AUDIO_STREAM_TYPE_CAPTURE;
        }
        else
        {
            pDewellingInfo->startedStreamTypes &= (~GAF_AUDIO_STREAM_TYPE_CAPTURE);
            is_gaf_audio_startedStreamTypes[ase_lid] = GAF_AUDIO_STREAM_NON_START;
        }
    }

    LOG_I("direction %d started stream types updated to %d", p_bap_ase_info->direction,
        pDewellingInfo->startedStreamTypes);

    if (GAF_AUDIO_UPDATE_STREAM_INFO_TO_START == purpose)
    {
        if (BES_BLE_GAF_DIRECTION_SINK == p_bap_ase_info->direction)
        {
            gaf_audio_add_ase_into_playback_ase_list(p_bap_ase_info->con_lid, ase_lid);
        }
        else
        {
            gaf_audio_add_ase_into_capture_ase_list(p_bap_ase_info->con_lid, ase_lid);
        }

        // any direction of stream can start the context
        return APP_GAF_CONTEXT_STREAM_STARTED;
    }
    else
    {
        if (BES_BLE_GAF_DIRECTION_SINK == p_bap_ase_info->direction)
        {
            gaf_audio_remove_ase_from_playback_ase_list(p_bap_ase_info->con_lid, ase_lid);
        }
        else
        {
            gaf_audio_remove_ase_from_capture_ase_list(p_bap_ase_info->con_lid, ase_lid);
        }

        if (0 == pDewellingInfo->startedStreamTypes)
        {
            return APP_GAF_CONTEXT_ALL_STREAMS_STOPPED;
        }
        else
        {
            if (pDewellingInfo->startedStreamTypes == GAF_AUDIO_STREAM_TYPE_PLAYBACK)
            {
                return APP_GAF_CONTEXT_CAPTURE_STREAMS_STOPPED;
            }
            else
            {
                return APP_GAF_CONTEXT_PLAYBACK_STREAMS_STOPPED;
            }
        }
    }
}

#ifdef BLE_USB_AUDIO_SUPPORT
uint8_t gaf_audio_get_stream_started_type(uint8_t con_lid)
{
    return gaf_cis_media_dwelling_info[con_lid].startedStreamTypes;
}
#endif

gaf_stream_context_state_t gaf_audio_stream_update_and_start_handler(uint8_t ase_lid)
{
     gaf_stream_context_state_t updatedContextStreamState =
        gaf_audio_update_stream_state_info_from_ase(GAF_AUDIO_UPDATE_STREAM_INFO_TO_START, ase_lid);

    if (APP_GAF_CONTEXT_STREAM_STARTED == updatedContextStreamState)
    {
#ifdef GAF_CODEC_CROSS_CORE
        memset_s(&gaf_m55_deinit_status, sizeof(gaf_m55_deinit_status),
               0x0, sizeof(gaf_m55_deinit_status));
#endif
    }

    return updatedContextStreamState;
}

static GAF_AUDIO_STREAM_ENV_T* _gaf_refresh_all_stream_info_by_con_lid(uint8_t con_lid)
{
    uint8_t *playback_ase_lid_list = gaf_audio_get_playback_ase_index_list(con_lid);
    uint8_t *capture_ase_lid_list = gaf_audio_get_capture_ase_index_list(con_lid);
    uint8_t idx = 0;
    GAF_AUDIO_STREAM_ENV_T* pPlaybackStreamEnv = NULL;
    GAF_AUDIO_STREAM_ENV_T* pCaptureStreamEnv = NULL;
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = NULL;

    for (idx = 0; idx < GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT; idx++)
    {
        if (GAF_INVALID_ASE_INDEX != capture_ase_lid_list[idx])
        {
            pStreamEnv = gaf_audio_refresh_stream_info_from_ase(capture_ase_lid_list[idx]);
            pCaptureStreamEnv = (pStreamEnv == NULL) ?
                                        pCaptureStreamEnv : pStreamEnv;
            // No break for update all un refresh stream info
        }
    }

    for (idx = 0; idx < GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT; idx++)
    {
        if (GAF_INVALID_ASE_INDEX != playback_ase_lid_list[idx])
        {
            pStreamEnv = gaf_audio_refresh_stream_info_from_ase(playback_ase_lid_list[idx]);
            pPlaybackStreamEnv = (pStreamEnv == NULL) ?
                                        pPlaybackStreamEnv : pStreamEnv;
            // No break for update all un refresh stream info
        }
    }

    if (pPlaybackStreamEnv && pCaptureStreamEnv)
    {
        ASSERT(pPlaybackStreamEnv == pCaptureStreamEnv, "Same context should use the same env!");
    }

    if (pPlaybackStreamEnv)
    {
        pPlaybackStreamEnv->stream_info.con_lid = con_lid;
        return pPlaybackStreamEnv;
    }
    else if (pCaptureStreamEnv)
    {
        pCaptureStreamEnv->stream_info.con_lid = con_lid;
        return pCaptureStreamEnv;
    }
    else
    {
        return NULL;
    }
}

void gaf_audio_stream_start(uint8_t con_lid)
{
    LOG_I("%s start,con_lid = 0x%x", __func__,con_lid);

#ifdef GAF_DSP
    dsp_open();
#endif

    GAF_AUDIO_STREAM_ENV_T* pStreamEnv =  _gaf_refresh_all_stream_info_by_con_lid(con_lid);

    if (pStreamEnv)
    {
        if ((GAF_CAPTURE_STREAM_IDLE == pStreamEnv->stream_context.capture_stream_state) &&
            (GAF_PLAYBACK_STREAM_IDLE == pStreamEnv->stream_context.playback_stream_state))
        {
            bt_drv_i2v_disable_sleep_for_bt_access();
            bt_adapter_write_sleep_enable(0);
            gaf_stream_register_running_stream_ref(con_lid, pStreamEnv);
        }

#ifdef CODEC_VCM_CHECK
        extern uint8_t vcm_sta;
        vcm_sta = 0;
#endif

        GAF_MEDIA_DWELLING_INFO_T* pDewellingInfo = &gaf_cis_media_dwelling_info[con_lid];

#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
        gaf_set_ble_audio_playback_sample_rate(pStreamEnv->stream_info.playbackInfo.sample_rate);
#endif
        if ((pDewellingInfo->startedStreamTypes & GAF_AUDIO_STREAM_TYPE_PLAYBACK) &&
            (GAF_PLAYBACK_STREAM_IDLE == pStreamEnv->stream_context.playback_stream_state))
        {
            uint8_t playback_ase_lid = GAF_INVALID_ASE_INDEX;
            uint8_t *playback_ase_lid_list = gaf_audio_get_playback_ase_index_list(con_lid);
            for (uint8_t ase_lid = 0; ase_lid < GAF_AUDIO_ASE_TOTAL_COUNT - 1; ase_lid++)
            {
                if (GAF_INVALID_ASE_INDEX != playback_ase_lid_list[ase_lid])
                {
                    playback_ase_lid = playback_ase_lid_list[ase_lid];
                    break;
                }
            }

            ASSERT(playback_ase_lid != GAF_INVALID_ASE_INDEX, "invalid playback ase lid");

#ifdef  GAF_CODEC_CROSS_CORE
            if (is_gaf_audio_startedStreamTypes[playback_ase_lid] == GAF_AUDIO_STREAM_NON_START)
            {
                gaf_audio_stream_set_playback_state(true);
#else
            {
#endif
                is_gaf_audio_startedStreamTypes[playback_ase_lid] = GAF_AUDIO_STREAM_START_FIRST_CNT;
                pStreamEnv->func_list->stream_func_list.playback_start_stream_func(pStreamEnv);
            }
        }

        if ((pDewellingInfo->startedStreamTypes & GAF_AUDIO_STREAM_TYPE_CAPTURE) &&
            (GAF_CAPTURE_STREAM_IDLE == pStreamEnv->stream_context.capture_stream_state))
        {
            uint8_t capture_ase_lid = GAF_INVALID_ASE_INDEX;
            uint8_t *capture_ase_lid_list = gaf_audio_get_capture_ase_index_list(con_lid);
            for (uint8_t ase_lid = 0; ase_lid < GAF_AUDIO_ASE_TOTAL_COUNT - 1; ase_lid++)
            {
                if (GAF_INVALID_ASE_INDEX != capture_ase_lid_list[ase_lid])
                {
                    capture_ase_lid = capture_ase_lid_list[ase_lid];
                    break;
                }
            }

            ASSERT(capture_ase_lid != GAF_INVALID_ASE_INDEX, "invalid capture ase lid");

#ifdef  GAF_CODEC_CROSS_CORE
            if (is_gaf_audio_startedStreamTypes[capture_ase_lid] == GAF_AUDIO_STREAM_NON_START)
            {
                gaf_audio_stream_set_capture_state(true);
#else
            {
#endif
                is_gaf_audio_startedStreamTypes[capture_ase_lid] = GAF_AUDIO_STREAM_START_FIRST_CNT;
                pStreamEnv->func_list->stream_func_list.capture_start_stream_func(pStreamEnv);
            }
        }
    }

    LOG_I("%s end", __func__);
}

#ifdef GAF_CODEC_CROSS_CORE
/**
 ****************************************************************************************
 * @brief BTH core set deinit parameters status (global varible),
 *        before bth send deinit signal to m55.
 *
 * @param[in] con_id                       ASE Connection index
 * @param[in] context_type                 APP Layer Audio context Type
 *
 * @param[out] NONE                         return NONE
 ****************************************************************************************
 */
static void gaf_audio_stream_bth_set_playback_deinit_status(uint8_t con_id, uint32_t context_type)
{
    gaf_m55_deinit_status.con_lid      = con_id;
    gaf_m55_deinit_status.context_type = context_type;
    gaf_m55_deinit_status.is_bis       = false;
    gaf_m55_deinit_status.is_bis_src   = false;
    gaf_m55_deinit_status.is_mobile_role = false;

    if (GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE == context_type)
    {
        uint8_t playback_ase_lid = GAF_INVALID_ASE_INDEX;
        uint8_t *playback_ase_lid_list = gaf_audio_get_playback_ase_index_list(con_id);
        for (uint8_t ase_lid = 0; ase_lid < GAF_AUDIO_ASE_TOTAL_COUNT - 1; ase_lid++)
        {
            if (GAF_INVALID_ASE_INDEX != playback_ase_lid_list[ase_lid])
            {
                playback_ase_lid = playback_ase_lid_list[ase_lid];
                break;
            }
        }
        is_gaf_audio_startedStreamTypes[playback_ase_lid] = GAF_AUDIO_STREAM_NON_START;
        gaf_audio_stream_set_playback_state(false);
        gaf_m55_deinit_status.playback_deinit = true;
     }
    return ;
}

/**
 ****************************************************************************************
 * @brief BTH core set deinit parameters status (global varible),
 *        before bth send deinit signal to m55.
 *
 * @param[in] con_id                       ASE Connection index
 * @param[in] context_type                 APP Layer Audio context Type
 *
 * @param[out] NONE                         return NONE
 ****************************************************************************************
 */
static void gaf_audio_stream_bth_set_capture_deinit_status(uint8_t con_id, uint32_t context_type)
{
    gaf_m55_deinit_status.con_lid      = con_id;
    gaf_m55_deinit_status.context_type = context_type;
    gaf_m55_deinit_status.is_bis       = false;
    gaf_m55_deinit_status.is_bis_src   = false;
    gaf_m55_deinit_status.is_mobile_role = false;

    if (GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE == context_type)
    {
        gaf_audio_stream_set_capture_state(false);
        gaf_m55_deinit_status.capture_deinit = true;
     }
    return ;
}

/**
 ****************************************************************************************
 * @brief BTH core send deinit signal to m55.
 *
 * @param[in] NONE                       NONE
 * @param[in] NONE                       NONE
 *
 * @param[out] NONE                      NONE
 ****************************************************************************************
 */
static void gaf_audio_stream_playback_bth_send_deinit_to_m55(GAF_AUDIO_STREAM_ENV_T* pStreamEnv)
{
    if (pStreamEnv->stream_context.playback_retrigger_onprocess)
    {
        return;
    }
    GAF_AUDIO_M55_DEINIT_T p_deinit_req;
    p_deinit_req.con_lid         = gaf_m55_deinit_status.con_lid;
    p_deinit_req.context_type    = gaf_m55_deinit_status.context_type;
    p_deinit_req.is_bis          = gaf_m55_deinit_status.is_bis;
    p_deinit_req.is_bis_src      = gaf_m55_deinit_status.is_bis_src;
    p_deinit_req.capture_deinit  = gaf_m55_deinit_status.capture_deinit;
    p_deinit_req.playback_deinit = gaf_m55_deinit_status.playback_deinit;
    p_deinit_req.is_mobile_role  = gaf_m55_deinit_status.is_mobile_role;

    if (!gaf_m55_deinit_status.playback_deinit_sent){
        app_dsp_m55_bridge_send_cmd(
            CROSS_CORE_TASK_CMD_GAF_DECODE_DEINIT_WAITING_RSP,
            (uint8_t*)&p_deinit_req,
            sizeof(GAF_AUDIO_M55_DEINIT_T));
        gaf_m55_deinit_status.playback_deinit_sent = true;
    }

    return ;
}

/**
 ****************************************************************************************
 * @brief BTH core send deinit signal to m55.
 *
 * @param[in] NONE                       NONE
 * @param[in] NONE                       NONE
 *
 * @param[out] NONE                      NONE
 ****************************************************************************************
 */
static void gaf_audio_stream_capture_bth_send_deinit_to_m55(GAF_AUDIO_STREAM_ENV_T* pStreamEnv)
{
    if (pStreamEnv->stream_context.playback_retrigger_onprocess)
    {
        return;
    }
    GAF_AUDIO_M55_DEINIT_T p_deinit_req;
    p_deinit_req.con_lid         = gaf_m55_deinit_status.con_lid;
    p_deinit_req.context_type    = gaf_m55_deinit_status.context_type;
    p_deinit_req.is_bis          = gaf_m55_deinit_status.is_bis;
    p_deinit_req.is_bis_src      = gaf_m55_deinit_status.is_bis_src;
    p_deinit_req.capture_deinit  = gaf_m55_deinit_status.capture_deinit;
    p_deinit_req.playback_deinit = gaf_m55_deinit_status.playback_deinit;
    p_deinit_req.is_mobile_role  = gaf_m55_deinit_status.is_mobile_role;

    if (!gaf_m55_deinit_status.capture_deinit_sent){
        app_dsp_m55_bridge_send_cmd(
            CROSS_CORE_TASK_CMD_GAF_ENCODE_DEINIT_WAITING_RSP, \
            (uint8_t*)&p_deinit_req, \
            sizeof(GAF_AUDIO_M55_DEINIT_T));

        gaf_m55_deinit_status.capture_deinit_sent = true;
    }
    return ;
}

/**
 ****************************************************************************************
 * @brief When bth received the deocder deinit signal from m55, bth will deinit m55 core
 *
 * @param[in] con_lid                       connection index
 * @param[in] context_type                 ASE context type
 *
 * @param[out] NONE                      NONE
 ****************************************************************************************
 */
void _gaf_audio_bth_received_decoder_deinit_signal_from_m55(uint8_t con_lid, uint32_t context_type)
{
    LOG_D("%s", __func__);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_audio_stream_env[con_lid];
    if (pStreamEnv)
    {
        if (GAF_PLAYBACK_STREAM_IDLE == pStreamEnv->stream_context.playback_stream_state)
            app_dsp_m55_deinit(APP_DSP_M55_USER_AUDIO_DECODER);
    }
    return ;
}

/**
 ****************************************************************************************
 * @brief When bth received the encoder deinit signal from m55, bth will deinit m55 core
 *
 * @param[in] con_lid                       connection index
 * @param[in] context_type                 ASE context type
 *
 * @param[out] NONE                      NONE
 ****************************************************************************************
 */
void _gaf_audio_bth_received_encoder_deinit_signal_from_m55(uint8_t con_lid, uint32_t context_type)
{
    LOG_D("%s", __func__);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_audio_stream_env[con_lid];
    if (pStreamEnv){
        if (GAF_CAPTURE_STREAM_IDLE == pStreamEnv->stream_context.capture_stream_state)
            app_dsp_m55_deinit(APP_DSP_M55_USER_AUDIO_ENCODER);
    }
    return ;
}
#endif

static void gaf_audio_stream_stop_generic(bool isCheckServiceState, uint8_t con_lid)
{
    LOG_D("%s", __func__);

    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_audio_stream_env[con_lid];

#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
    uint8_t pendingStopOp;
    if (isCheckServiceState)
    {
        pendingStopOp = PENDING_TO_STOP_BLE_AUDIO_SINGLE_STREAMING;
    }
    else
    {
        pendingStopOp = PENDING_TO_STOP_BLE_AUDIO_STREAMING;
    }

    bool ret = audio_prompt_check_on_stopping_ble_audio_stream(pendingStopOp,
                    con_lid, con_lid, (void*)gaf_audio_stream_stop_generic);
    if (!ret)
    {
        TRACE(0,"Pending stop BLE_AUDIO_STREAM");
        return;
    }
#endif

    /// Check stream state
    if (pStreamEnv &&
        (GAF_CAPTURE_STREAM_IDLE != pStreamEnv->stream_context.capture_stream_state ||
         GAF_PLAYBACK_STREAM_IDLE != pStreamEnv->stream_context.playback_stream_state))
    {
        GAF_MEDIA_DWELLING_INFO_T* pDewellingInfo = &gaf_cis_media_dwelling_info[con_lid];

        if (isCheckServiceState)
        {
            if ((0 == (pDewellingInfo->startedStreamTypes&GAF_AUDIO_STREAM_TYPE_PLAYBACK)) &&
                (GAF_PLAYBACK_STREAM_IDLE != pStreamEnv->stream_context.playback_stream_state))
            {
#ifdef GAF_CODEC_CROSS_CORE
                gaf_audio_stream_bth_set_playback_deinit_status(con_lid, GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE);
#endif
                pStreamEnv->func_list->stream_func_list.playback_stop_stream_func(pStreamEnv);
                for (uint8_t i = 0; i < GAF_AUDIO_ASE_TOTAL_COUNT; i++)
                {
                    pStreamEnv->stream_info.playbackInfo.aseChInfo[i].iso_channel_hdl = GAF_AUDIO_INVALID_ISO_CHANNEL;
                    pStreamEnv->stream_info.playbackInfo.aseChInfo[i].allocation_bf = 0;
                    pStreamEnv->stream_context.lastestPlaybackSeqNum[i] = GAF_AUDIO_INVALID_SEQ_NUMBER;
                }
#ifdef GAF_CODEC_CROSS_CORE
                gaf_audio_stream_playback_bth_send_deinit_to_m55(pStreamEnv);
#endif
            }

            if ((0 == (pDewellingInfo->startedStreamTypes&GAF_AUDIO_STREAM_TYPE_CAPTURE)) &&
                (GAF_CAPTURE_STREAM_IDLE != pStreamEnv->stream_context.capture_stream_state))
            {
#ifdef GAF_CODEC_CROSS_CORE
                gaf_audio_stream_bth_set_capture_deinit_status(con_lid, GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE);
#endif
                pStreamEnv->func_list->stream_func_list.capture_stop_stream_func(pStreamEnv);
                for (uint8_t i = 0; i < GAF_AUDIO_ASE_TOTAL_COUNT; i++)
                {
                    pStreamEnv->stream_info.captureInfo.aseChInfo[i].iso_channel_hdl = GAF_AUDIO_INVALID_ISO_CHANNEL;
                    pStreamEnv->stream_info.captureInfo.aseChInfo[i].allocation_bf = 0;
                }
#ifdef GAF_CODEC_CROSS_CORE
                gaf_audio_stream_capture_bth_send_deinit_to_m55(pStreamEnv);
#endif
            }
        }
        else
        {
#ifdef GAF_CODEC_CROSS_CORE
                gaf_audio_stream_bth_set_playback_deinit_status(con_lid, GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE);
                gaf_audio_stream_bth_set_capture_deinit_status(con_lid, GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE);
#endif
                pStreamEnv->func_list->stream_func_list.playback_stop_stream_func(pStreamEnv);
                for (uint8_t i = 0; i < GAF_AUDIO_ASE_TOTAL_COUNT; i++)
                {
                    pStreamEnv->stream_info.playbackInfo.aseChInfo[i].iso_channel_hdl = GAF_AUDIO_INVALID_ISO_CHANNEL;
                    pStreamEnv->stream_info.playbackInfo.aseChInfo[i].allocation_bf = 0;
                    pStreamEnv->stream_context.lastestPlaybackSeqNum[i] = GAF_AUDIO_INVALID_SEQ_NUMBER;
                }


                pStreamEnv->func_list->stream_func_list.capture_stop_stream_func(pStreamEnv);
                for (uint8_t i = 0; i < GAF_AUDIO_ASE_TOTAL_COUNT; i++)
                {
                    pStreamEnv->stream_info.captureInfo.aseChInfo[i].iso_channel_hdl = GAF_AUDIO_INVALID_ISO_CHANNEL;
                    pStreamEnv->stream_info.captureInfo.aseChInfo[i].allocation_bf = 0;
                }
#ifdef GAF_CODEC_CROSS_CORE
                if (true == gaf_m55_deinit_status.playback_deinit){
                    gaf_audio_stream_playback_bth_send_deinit_to_m55(pStreamEnv);
                }

                if (true == gaf_m55_deinit_status.capture_deinit){
                    gaf_audio_stream_capture_bth_send_deinit_to_m55(pStreamEnv);
                }
#endif
        }

        /// Should be transfered to IDLE by previous procedure, avoid multi call
        if ((GAF_CAPTURE_STREAM_IDLE == pStreamEnv->stream_context.capture_stream_state) &&
            (GAF_PLAYBACK_STREAM_IDLE == pStreamEnv->stream_context.playback_stream_state))
        {
            gaf_stream_register_running_stream_ref(con_lid, NULL);
            bt_drv_i2v_enable_sleep_for_bt_access();
            bt_adapter_write_sleep_enable(1);
        }
    }
    else
    {
        LOG_W("%s no ase_lid stream to stop, may be already stopped", __func__);
    }
#ifdef GAF_DSP
    dsp_close();
#endif
}

void gaf_audio_stream_stop(uint8_t con_lid)
{
    gaf_audio_stream_stop_generic(false, con_lid);
}

void gaf_audio_stream_refresh_and_stop(uint8_t con_lid)
{
    gaf_audio_stream_stop_generic(true, con_lid);
}

static void gaf_audio_steam_retrigger_handler(void* stream_env)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)stream_env;

    GAF_MEDIA_DWELLING_INFO_T gaf_cis_media_dwelling_info_rec;
    uint16_t is_gaf_audio_startedStreamTypes_rec[APP_BAP_MAX_RECORD_CNT];
    // Record this
    memcpy_s(is_gaf_audio_startedStreamTypes_rec, sizeof(is_gaf_audio_startedStreamTypes),
                is_gaf_audio_startedStreamTypes, sizeof(is_gaf_audio_startedStreamTypes));
    memcpy_s(&gaf_cis_media_dwelling_info_rec, sizeof(GAF_MEDIA_DWELLING_INFO_T),
                &gaf_cis_media_dwelling_info[pStreamEnv->stream_info.con_lid],
                sizeof(GAF_MEDIA_DWELLING_INFO_T));
    // Clear this
    gaf_audio_dwelling_info_list_init();
    // Memset to invalid ase lid
    gaf_audio_clear_playback_ase_index_list(pStreamEnv->stream_info.con_lid);
    gaf_audio_clear_capture_ase_index_list(pStreamEnv->stream_info.con_lid);
    // Stop this must in this thread
    gaf_audio_stream_stop_generic(false, pStreamEnv->stream_info.con_lid);
    // Recovery this
    memcpy_s(is_gaf_audio_startedStreamTypes, sizeof(is_gaf_audio_startedStreamTypes),
                is_gaf_audio_startedStreamTypes_rec, sizeof(is_gaf_audio_startedStreamTypes));
    memcpy_s(&gaf_cis_media_dwelling_info[pStreamEnv->stream_info.con_lid],
                sizeof(GAF_MEDIA_DWELLING_INFO_T),
                &gaf_cis_media_dwelling_info_rec, sizeof(GAF_MEDIA_DWELLING_INFO_T));
    // Start this in other thread is ok
    gaf_audio_stream_start(pStreamEnv->stream_info.con_lid);
}

void gaf_audio_stream_register_func_list(GAF_AUDIO_STREAM_ENV_T* pStreamEnv, GAF_AUDIO_FUNC_LIST_T* funcList)
{
    pStreamEnv->func_list = funcList;
}

void gaf_audio_stream_init(void)
{
    /// GAF CUSTOM CALLBACK PTR
    POSSIBLY_UNUSED GAF_AUDIO_STREAM_COMMON_INFO_T *playback_info = NULL;
    gaf_uc_srv_custom_data_callback = gaf_stream_common_get_custom_data_handler(GAF_STREAM_USER_CASE_UC_SRV);
    /// Sanity check
    ASSERT(gaf_uc_srv_custom_data_callback, "Invalid custom data callback, user case %d", GAF_STREAM_USER_CASE_UC_SRV);

    gaf_audio_dwelling_info_list_init();

    for (uint8_t con_lid = 0; con_lid < GAF_MAXIMUM_CONNECTION_COUNT; con_lid++)
    {
        GAF_AUDIO_STREAM_ENV_T *pStreamEnv = &gaf_audio_stream_env[con_lid];
        memset_s((uint8_t *)pStreamEnv,sizeof(GAF_AUDIO_STREAM_ENV_T),  0, sizeof(GAF_AUDIO_STREAM_ENV_T));
        playback_info = &pStreamEnv->stream_info.playbackInfo;
        pStreamEnv->stream_context.playback_stream_state = GAF_PLAYBACK_STREAM_IDLE;
        pStreamEnv->stream_context.capture_stream_state = GAF_CAPTURE_STREAM_IDLE;
        pStreamEnv->stream_info.playbackInfo.presDelayUs = GAF_AUDIO_CONTROLLER_2_HOST_LATENCY_US;
        pStreamEnv->stream_info.captureInfo.presDelayUs = GAF_AUDIO_CONTROLLER_2_HOST_LATENCY_US;
        pStreamEnv->stream_context.playback_trigger_supervisor_timer_id = NULL;
        pStreamEnv->stream_context.capture_trigger_supervisor_timer_id = NULL;
        pStreamEnv->stream_context.playback_retrigger_onprocess = false;
        pStreamEnv->stream_info.is_mobile = false;
        pStreamEnv->stream_info.contextType = GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE;
        gaf_stream_common_register_func_list(pStreamEnv, &gaf_audio_flexible_stream_func_list[con_lid]);
        gaf_playback_status_mutex_init(pStreamEnv->stream_info.gaf_playback_status_mutex);
        gaf_audio_clear_playback_ase_index_list(con_lid);
        gaf_audio_clear_capture_ase_index_list(con_lid);

        for (uint8_t i = 0; i < GAF_AUDIO_ASE_TOTAL_COUNT; i++)
        {
            pStreamEnv->stream_info.playbackInfo.aseChInfo[i].iso_channel_hdl = GAF_AUDIO_INVALID_ISO_CHANNEL;
            pStreamEnv->stream_info.captureInfo.aseChInfo[i].iso_channel_hdl = GAF_AUDIO_INVALID_ISO_CHANNEL;
            pStreamEnv->stream_info.playbackInfo.aseChInfo[i].allocation_bf = 0;
            pStreamEnv->stream_info.captureInfo.aseChInfo[i].allocation_bf = 0;
            pStreamEnv->stream_context.lastestPlaybackSeqNum[i] = GAF_AUDIO_INVALID_SEQ_NUMBER;
        }
#ifdef DYNAMIC_SET_PB_TIME
        gaf_dync_buffer_init(&playback_info->dync_buffer);
#endif
    }

    gaf_stream_register_retrigger_callback(gaf_audio_steam_retrigger_handler);
}
#endif
